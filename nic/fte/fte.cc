#include <sys/queue.h>

#include "nic/fte/fte.hpp"
#include "nic/fte/fte_flow.hpp"
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "nic/p4/nw/include/defines.h"
#include "nic/include/cpupkt_api.hpp"

namespace fte {

// global instance of all FTE data
fte_db  *g_fte_db;

/*-----------------------------------------------------
    Begin FTE DB Constructor/Destructor APIs
------------------------------------------------------*/

hal_ret_t
fte_db::init(void)
{
    // initialize tenant related data structures
    alg_flow_key_ht_ = hal::utils::ht::factory(FTE_MAX_ALG_KEYS,
                                               alg_flow_get_key_func,
                                               alg_flow_compute_hash_func,
                                               alg_flow_compare_key_func);
    HAL_ASSERT_RETURN((alg_flow_key_ht_ != NULL), HAL_RET_ERR);

    return HAL_RET_OK;
}

fte_db::~fte_db(void)
{
    alg_flow_key_ht_ ? delete alg_flow_key_ht_ : HAL_NOP;
}

fte_db *
fte_db::factory(void)
{
    void         *mem;
    fte_db       *db;

    mem = HAL_CALLOC(HAL_MEM_ALLOC_INFRA, sizeof(fte_db));
    HAL_ABORT(mem != NULL);
    db = new (mem) fte_db();
    if (db->init() == HAL_RET_ERR) {
        db->~fte_db();
        HAL_FREE(HAL_MEM_ALLOC_INFRA, mem);
            return NULL;
    }

    return db;
}

/*-----------------------------------------------------
    End FTE DB Constructor/Destructor APIs
------------------------------------------------------*/

// FTE features
typedef struct feature_s feature_t;
struct feature_s {
    STAILQ_ENTRY(feature_s) entries;
    feature_id_t id;
    std::string name;
    exec_handler_t exec_handler;
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
                           const exec_handler_t &exec_handler)
{
    feature_t *feature;

    HAL_TRACE_DEBUG("fte::{}: id={}, name={}", __FUNCTION__, fid, name);

    if (!exec_handler) {
        HAL_TRACE_ERR("fte: skipping invalid feature id={} name={} - null exec_handler",
                      fid, name);
        return HAL_RET_INVALID_ARG;
    }

    if ((feature = feature_lookup_(fid)) != nullptr) {
        HAL_TRACE_ERR("fte: skipping duplicate feature id={} name={} old-name={}",
                      fid, name, feature->name);
        return HAL_RET_DUP_INS_FAIL;
    }

    feature = feature_alloc_();
    feature->id = fid;
    feature->name = name;
    feature->exec_handler = exec_handler;

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
    uint16_t num_features_outbound;
    uint16_t num_features_inbound;
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
pipeline_invoke_exec_(pipeline_t *pipeline, ctx_t &ctx, uint8_t start, uint8_t end)
{
    pipeline_action_t rc;

    HAL_TRACE_DEBUG("Invoking pipeline with start: {} end: {}", start, end);

    for (int i = start; i < end; i++) {
        feature_t *feature = pipeline->features[i];

        ctx.set_feature_name(feature->name.c_str());
        ctx.set_feature_status(HAL_RET_OK);
        rc = feature->exec_handler(ctx);
        HAL_TRACE_DEBUG("fte:exec_handler feature={} pipeline={} action={}", feature->name,
                        pipeline->name, rc);

        if (rc != PIPELINE_CONTINUE) {
            break;
        }
    }

    ctx.set_feature_name("");
    return rc;
}

hal_ret_t
register_pipeline(const std::string& name, const lifqid_t& lifq,
                  feature_id_t features_outbound[], uint16_t num_features_outbound,
                  feature_id_t features_inbound[], uint16_t num_features_inbound,
                  const lifqid_t& lifq_mask)
{
    pipeline_t *pipeline;

    HAL_TRACE_DEBUG("fte::{}: name={} lifq={}", __FUNCTION__, name, lifq);
    if ((pipeline = pipeline_lookup_(lifq)) != nullptr) {
        HAL_TRACE_ERR("fte: skipping duplicate pipline {} lifq={} old-name={}",
                      name, lifq, pipeline->name);
        return HAL_RET_DUP_INS_FAIL;
    }

    pipeline = pipeline_alloc_(num_features_outbound+num_features_inbound);
    pipeline->lifq = lifq;
    pipeline->lifq_mask = lifq_mask;
    pipeline->name = name;

    // Initialize feature blocks
    pipeline->num_features_outbound =num_features_outbound;
    for (int i = 0; i < num_features_outbound; i++) {
        feature_t *feature = feature_lookup_(features_outbound[i]);
        if (!feature) {
            HAL_TRACE_ERR("fte: unknown feature-id {} in outbound pipeline {} - skipping",
                          features_outbound[i], name);
            HAL_FREE(pipeline_t, pipeline);
            return HAL_RET_INVALID_ARG;
        }
        HAL_TRACE_DEBUG("fte: outbound pipeline feature {}/{}", name, feature->name);
        pipeline->features[i] = feature;
    }

    pipeline->num_features_inbound =num_features_inbound;
    for (int i = 0; i < num_features_inbound; i++) {
        feature_t *feature = feature_lookup_(features_inbound[i]);
        if (!feature) {
            HAL_TRACE_ERR("fte: unknown feature-id {} in inbound pipeline {} - skipping",
                          features_inbound[i], name);
            HAL_FREE(pipeline_t, pipeline);
            return HAL_RET_INVALID_ARG;
        }
        HAL_TRACE_DEBUG("fte: inbound pipeline feature {}/{}", name, feature->name);
        pipeline->features[i+num_features_outbound] = feature;
    }

    // Add to global pipline list
    pipeline_add_(pipeline);

    return HAL_RET_OK;
}

hal_ret_t
execute_pipeline(ctx_t &ctx)
{
    uint8_t iflow_start, iflow_end, rflow_start, rflow_end;
    pipeline_action_t rc;

    do {
        pipeline_t *pipeline = pipeline_lookup_(ctx.arm_lifq());
        if (!pipeline) {
            HAL_TRACE_ERR("fte: pipeline not registered for lifq {} - ignoring packet",
                          ctx.arm_lifq());
            return HAL_RET_INVALID_ARG;
        }
        
        HAL_TRACE_DEBUG("fte: executing pipeline {} lifq={} dir={}",
                        pipeline->name, pipeline->lifq, ctx.direction());

        iflow_start = 0;
        iflow_end = rflow_start = pipeline->num_features_outbound;
        rflow_end = pipeline->num_features_outbound + pipeline->num_features_inbound;
        // Flip the feature list for network initiated
        if (pipeline->num_features_inbound > 0 && ctx.direction() == FLOW_DIR_FROM_UPLINK) {
            rflow_start = 0;
            rflow_end = iflow_start = pipeline->num_features_outbound;
            iflow_end = pipeline->num_features_outbound + pipeline->num_features_inbound;
        }

        if (ctx.role() == hal::FLOW_ROLE_RESPONDER) {
            // Invoke all responder feature handlers if we are processing Rflow
            rc = pipeline_invoke_exec_(pipeline, ctx, rflow_start, rflow_end); 
        } else {
            // Invoke all initiator feature handlers
            rc = pipeline_invoke_exec_(pipeline, ctx, iflow_start, iflow_end);
        }
       
        // If we are processing Iflow and Rflow in the same
        // context swap the flow objects and invoke pipeline 
        if (ctx.role() == hal::FLOW_ROLE_INITIATOR && \
            rc == PIPELINE_CONTINUE && ctx.valid_rflow()) {
            //Swap the derived flow objects 
            //for the reverse flow
            ctx.swap_flow_objs();

            ctx.set_role(hal::FLOW_ROLE_RESPONDER);
            rc = pipeline_invoke_exec_(pipeline, ctx, rflow_start, rflow_end);

            //Swap back before GFT update happens
            ctx.swap_flow_objs();
        }
    } while (rc == PIPELINE_RESTART);

    return ctx.feature_status();
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

/*------------------------------------------------------------
    FTE Init API to initialize FTE top-level data structure
--------------------------------------------------------------*/

hal_ret_t init(void) 
{
    g_fte_db = fte_db::factory();
    HAL_ASSERT_RETURN((g_fte_db != NULL), HAL_RET_ERR);

    return HAL_RET_OK;
}

} // namespace fte
