#pragma once

#include <base.h>

namespace fte {

// FTE pipeline progress action codes
//  continue - continue the pipeline
//  end - end processing
//  restart - restart the processing from the begining
//            (may select a diffrent pipeline)
#define FTE_PIPELINE_ACTION_ENTRIES(ENTRY) \
    ENTRY(PIPELINE_CONTINUE, 0, "continue") \
    ENTRY(PIPELINE_END, 1, "end")           \
    ENTRY(PIPELINE_RESTART, 2, "restart")

DEFINE_ENUM(pipeline_action_t, FTE_PIPELINE_ACTION_ENTRIES)
#undef FTE_PIPELINE_ACTION_ENTRIES

// FTE lif qid
typedef struct lifqid_s lifqid_t;
struct lifqid_s {
    uint64_t lif : 11;
    uint64_t qtype: 3;
    uint64_t qid: 24;
};
inline std::ostream& operator<<(std::ostream& os, const lifqid_t& lifq)
{
    return os << fmt::format("{{lif={}, qtype={}, qid={}}}",
                             lifq.lif, lifq.qtype, lifq.qid);
}

// FTE context passed between features in a pipeline
typedef struct ctx_s ctx_t;
struct ctx_s {
    bool         drop;
    lifqid_t arm_lifq;
};

// FTE feature
typedef uint16_t feature_id_t;
typedef std::function<pipeline_action_t(ctx_t &ctx)> exec_handler_t;
typedef std::function<void(ctx_t &ctx)> commit_handler_t;
hal_ret_t register_feature(const feature_id_t& fid,
                           const std::string& name,
                           const exec_handler_t &exec_handler,
                           const commit_handler_t &commit_handler = nullptr);

// FTE Pipeline
hal_ret_t register_pipeline(const std::string& name, const lifqid_t &lifq,
                            feature_id_t features[], uint16_t num_features,
                            const lifqid_t &lifq_mask = lifqid_t{0x7FF, 0x7, 0xFFFFFF});

// Selects a pipeline and invokes features in that pipeline
// Doesn't update P4 tables and write packet to send queue
hal_ret_t execute_pipeline(ctx_t &ctx);

// for unit test code only
void unregister_features_and_pipelines();

} // namespace fte
