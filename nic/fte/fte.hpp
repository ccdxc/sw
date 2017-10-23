#pragma once

#include "nic/include/base.h"
#include "nic/include/fte_ctx.hpp"

namespace fte {

// FTE features
typedef uint16_t feature_id_t;

const feature_id_t FTE_FEATURE_NULL          = 0;
const feature_id_t FTE_FEATURE_STAGE_MARKER  = 1;
const feature_id_t FTE_FEATURE_FWDING        = 2;
const feature_id_t FTE_FEATURE_TUNNEL        = 3;
const feature_id_t FTE_FEATURE_DFW           = 4;
const feature_id_t FTE_FEATURE_LB            = 5;
const feature_id_t FTE_FEATURE_TCP_PROXY     = 6;
const feature_id_t FTE_FEATURE_QOS           = 7;
const feature_id_t FTE_FEATURE_DOL_TEST_ONLY = 8;
const feature_id_t FTE_FEATURE_TLS_PROXY     = 9;
const feature_id_t FTE_FEATURE_IPSEC         = 10;
const feature_id_t FTE_FEATURE_ALG           = 11;

// FTE pipeline progress action codes
//  continue - continue the pipeline
//  end - end processing
//  restart - restart the processing from the begining
//            (may select a diffrent pipeline)
#define FTE_PIPELINE_ACTION_ENTRIES(ENTRY)       \
    ENTRY(PIPELINE_CONTINUE, 0, "continue")      \
    ENTRY(PIPELINE_END,      1, "end")           \
    ENTRY(PIPELINE_RESTART,  2, "restart")

DEFINE_ENUM(pipeline_action_t, FTE_PIPELINE_ACTION_ENTRIES)
#undef FTE_PIPELINE_ACTION_ENTRIES

hal_ret_t build_wildcard_key(hal::flow_key_t& key);
void *lookup_alg_db(ctx_t *ctx);
void insert_alg_entry(alg_entry_t *entry);
void *remove_alg_entry(hal::flow_key_t key);
hal_ret_t update_alg_entry(hal::flow_key_t key, void *new_entry, size_t sz);
const void *lookup_alg_entry(hal::flow_key_t key);

typedef std::function<pipeline_action_t(ctx_t &ctx)> exec_handler_t;
hal_ret_t register_feature(const feature_id_t& fid,
                           const std::string& name,
                           const exec_handler_t &exec_handler);

//  FTE Pipeline 
hal_ret_t register_pipeline(const std::string& name, const lifqid_t &lifq,
                            feature_id_t features_outbound[],
                            uint16_t num_features_outbound,
                            feature_id_t features_inbound[] = {},
                            uint16_t num_features_inbound = 0,
                            const lifqid_t &lifq_mask = lifqid_t{0x7FF, 0x7, 0xFFFFFF});

// Selects a pipeline and invokes features in that pipeline
// Doesn't update P4 tables and write packet to send queue
hal_ret_t execute_pipeline(ctx_t &ctx);

// for unit test code only
void unregister_features_and_pipelines();

// grpc session request
hal_ret_t session_create(SessionSpec& spec, SessionResponse *rsp);

// FTE pkt loop (infinite loop)
void pkt_loop(uint8_t fte_id);

// FTE Init routine
hal_ret_t init();

} // namespace fte
