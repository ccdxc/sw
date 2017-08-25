#include <sys/queue.h>

#include "fte.hpp"
#include "fte_flow.hpp"

namespace fte {

// FTE features
typedef struct feature_s feature_t;
struct feature_s {
    STAILQ_ENTRY(feature_s) entries;
    feature_id_t id;
    std::string name;
    exec_handler_t exec_handler;
    commit_handler_t commit_handler;
};

static STAILQ_HEAD(feature_list_s_, feature_s) g_feature_list_ =
    STAILQ_HEAD_INITIALIZER(g_feature_list_);

static inline feature_t *feature_alloc_()
{
    return (feature_t *)HAL_CALLOC(feature_t, sizeof(feature_t));
}

static inline void feature_add_(feature_t *feature)
{
    STAILQ_INSERT_TAIL(&g_feature_list_, feature, entries);
}

static inline feature_t *feature_lookup_(feature_id_t fid)
{
    feature_t *feature;
    STAILQ_FOREACH(feature, &g_feature_list_, entries) {
        if (feature->id == fid) {
            return feature;
        }
    }
    return nullptr;
}

hal_ret_t register_feature(const feature_id_t& fid, const std::string& name,
                           const exec_handler_t &exec_handler,
                           const commit_handler_t &commit_handler)
{
    feature_t *feature;

    HAL_TRACE_DEBUG("{}: id={}, name={}", __FUNCTION__, fid, name);

    if (!exec_handler) {
        HAL_TRACE_ERR("skipping invalid feature id={} name={} - null exec_handler",
                      fid, name);
        return HAL_RET_INVALID_ARG;
    }

    if ((feature = feature_lookup_(fid)) != nullptr) {
        HAL_TRACE_ERR("skipping duplicate feature id={} name={} old-name={}",
                      fid, name, feature->name);
        return HAL_RET_DUP_INS_FAIL;
    }

    feature = feature_alloc_();
    feature->id = fid;
    feature->name = name;
    feature->exec_handler = exec_handler;
    feature->commit_handler = commit_handler;

    feature_add_(feature);

    return HAL_RET_OK;
}

// FTE pipelines
// A list is used instead of a map for pipelines. It is most likely faster
// to scan a list as the no.o pipelines will be very small (single digits).
// Also we keep the most frequently used pipelines in the front (flow miss
// pipeline will be the first, with minimal lookup cost)
typedef struct pipeline_s pipeline_t;
struct pipeline_s
{
    STAILQ_ENTRY(pipeline_s) entries;
    lifqid_t lifq;
    lifqid_t lifq_mask;
    std::string name;
    uint16_t num_features;
    feature_t *features[0];
};

static STAILQ_HEAD(pipeline_list_s_, pipeline_s) g_pipeline_list_ =
    STAILQ_HEAD_INITIALIZER(g_pipeline_list_);

static inline pipeline_t *
pipeline_alloc_(uint16_t num_features)
{
    return (pipeline_t *)HAL_CALLOC(pipeline_t, sizeof(pipeline_t) +
                                    num_features * sizeof(feature_t *));
}

static inline void
pipeline_add_(pipeline_t *pipeline)
{
    STAILQ_INSERT_TAIL(&g_pipeline_list_, pipeline, entries);
}

static inline pipeline_t *
pipeline_lookup_(const lifqid_t& lifq)
{
    pipeline_t *pipeline;

    STAILQ_FOREACH(pipeline, &g_pipeline_list_, entries) {
        if (pipeline->lifq.lif == (lifq.lif & pipeline->lifq_mask.lif) &&
            pipeline->lifq.qtype == (lifq.qtype & pipeline->lifq_mask.qtype) &&
            pipeline->lifq.qid == (lifq.qid & pipeline->lifq_mask.qid)) {
            return pipeline;
        }
    }
    return nullptr;
}

static inline pipeline_action_t
pipeline_invoke_exec_(pipeline_t *pipeline, ctx_t &ctx)
{
    pipeline_action_t rc;
    for (int i = 0; i < pipeline->num_features; i++) {
        feature_t *feature = pipeline->features[i];

        rc = feature->exec_handler(ctx);
        HAL_TRACE_DEBUG("exec handler feature={} pipeline={} rc={}", feature->name,
                        pipeline->name, rc);

        if (rc != PIPELINE_CONTINUE) {
            break;
        }
    }

    return rc;
}

hal_ret_t
register_pipeline(const std::string& name, const lifqid_t& lifq,
                  feature_id_t features[], uint16_t num_features,
                  const lifqid_t& lifq_mask)
{
    pipeline_t *pipeline;

    HAL_TRACE_DEBUG("{}: name={} lifq={}", __FUNCTION__, name, lifq);
    if ((pipeline = pipeline_lookup_(lifq)) != nullptr) {
        HAL_TRACE_ERR("skipping duplicate pipline {} lifq={} old-name={}",
                      name, lifq, pipeline->name);
        return HAL_RET_DUP_INS_FAIL;
    }

    pipeline = pipeline_alloc_(num_features);
    pipeline->lifq = lifq;
    pipeline->lifq_mask = lifq_mask;
    pipeline->name = name;
    pipeline->num_features = num_features;

    // Initialize feature blocks
    for (int i = 0; i < num_features; i++) {
        feature_t *feature = feature_lookup_(features[i]);
        if (!feature) {
            HAL_TRACE_ERR("unknown feature-id {} in pipeline {} - skipping",
                          features[i], name);
            HAL_FREE(pipeline_t, pipeline);
            return HAL_RET_INVALID_ARG;
        }
        pipeline->features[i] = feature;
    }

    // Add to global pipline list
    pipeline_add_(pipeline);

    return HAL_RET_OK;
}

static inline hal_ret_t
pipeline_execute_(ctx_t &ctx)
{
    pipeline_t *pipeline = pipeline_lookup_(ctx.arm_lifq());
    if (!pipeline) {
        HAL_TRACE_ERR("pipeline not registered for lifq {} - ignoring packet", ctx.arm_lifq());
        return HAL_RET_INVALID_ARG;
    }

    HAL_TRACE_DEBUG("executing pipeline {} lifq={}", pipeline->name, pipeline->lifq);

    // Invoke all feature handlers
    pipeline_action_t rc = pipeline_invoke_exec_(pipeline, ctx);
    if (rc != PIPELINE_RESTART) {
        return HAL_RET_OK;
    }

    // Reexecute new pipeline (tail recursion)
    return pipeline_execute_(ctx);
}

hal_ret_t execute_pipeline(ctx_t &ctx)
{
    hal_ret_t rc = pipeline_execute_(ctx);

    // TODO - invoke commit handlers
    // need to track commit_handlers, we might have executed
    // multiple pipelines and skipped some features.

    return rc;
}

// for unit test code only
void unregister_features_and_pipelines() {
    while (!STAILQ_EMPTY(&g_pipeline_list_)) {
        pipeline_t *pipeline = STAILQ_FIRST(&g_pipeline_list_);
        STAILQ_REMOVE_HEAD(&g_pipeline_list_, entries);
        HAL_FREE(pipeline_t, pipeline);
    }
    while (!STAILQ_EMPTY(&g_feature_list_)) {
        feature_t *feature = STAILQ_FIRST(&g_feature_list_);
        STAILQ_REMOVE_HEAD(&g_feature_list_, entries);
        HAL_FREE(feature_t, feature);
    }
}

// FTE main pkt loop
void
pkt_loop(hal_ret_t (*rx)(phv_t **phv, uint8_t **pkt, size_t *pkt_len),
         hal_ret_t (*tx)(const phv_t *phv, const uint8_t *pkt, size_t pkt_len))
{
    hal_ret_t rc;
    phv_t *phv;
    uint8_t *pkt;
    size_t pkt_len;
    ctx_t ctx;
    flow_t iflow, rflow, iflow_post, rflow_post;

    while(true) {
        // read the packet
        rc = rx(&phv, &pkt, &pkt_len);
        if (rc != HAL_RET_OK) {
            HAL_TRACE_ERR("fte - arm rx failed, rc={}", rc);
            continue;
        }

        // Init ctx_t
        rc = ctx.init(phv, pkt, pkt_len, &iflow, &rflow, &iflow_post, &rflow_post);
        if (rc != HAL_RET_OK) {
            HAL_TRACE_ERR("fte - failied to init context, rc={}", rc);
            continue;
        }

        // execute the pipeline
        rc = execute_pipeline(ctx);
        if (rc != HAL_RET_OK) {
            HAL_TRACE_ERR("fte - failied to execute pipeline, rc={}", rc);
            continue;
        }

        // update GFT
        rc = ctx.update_gft();
        if (rc != HAL_RET_OK) {
            HAL_TRACE_ERR("fte - failied to updated gft, rc={}", rc);
            continue;
        }

        // write the packet
        if (ctx.pkt()) {
            tx(ctx.phv(), ctx.pkt(), ctx.pkt_len());
        }
    }
}
} // namespace fte
