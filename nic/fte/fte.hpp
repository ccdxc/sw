#pragma once

#include "nic/include/base.h"
#include "nic/include/fte_ctx.hpp"

namespace fte {

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

typedef std::function<pipeline_action_t(ctx_t &ctx)> exec_handler_t;
typedef void (*feature_state_init_t)(void *state); // Init calback for feature specific state
typedef pipeline_action_t (*session_delete_handler_t)(ctx_t &ctx); // Session delete callback

typedef struct feature_info_s {
    // feature speicific per ctx state (this is not persisited
    // across packets, only valid during processing of one packet)
    size_t                    state_size; 
    feature_state_init_t      state_init_fn;
    session_delete_handler_t  sess_del_cb;

    // TODO(goli) need to define these
    // grpc_session_create_handler_t
    // grpc_session_get_handler_t
    // session_update_handler_t
} feature_info_t;

hal_ret_t add_feature(const std::string& name);
hal_ret_t register_feature(const std::string& name,
                           const exec_handler_t &exec_handler,
                           const feature_info_t &feature_info = {});

hal_ret_t unregister_feature(const std::string& name);

//  FTE Pipeline 
hal_ret_t register_pipeline(const std::string& name, const lifqid_t &lifq,
                            const std::vector<std::string> &features_outbound,
                            const std::vector<std::string> &features_inbound = {},
                            const lifqid_t &lifq_mask = lifqid_t{0x7FF, 0x7, 0xFFFFFF});

// Selects a pipeline and invokes features in that pipeline
// Doesn't update P4 tables and write packet to send queue
hal_ret_t execute_pipeline(ctx_t &ctx);

// for unit test code only
void unregister_features_and_pipelines();

// grpc session request
hal_ret_t session_create(SessionSpec& spec, SessionResponse *rsp);
hal_ret_t session_delete(SessionDeleteRequest& spec, SessionDeleteResponse *rsp);
hal_ret_t session_delete(hal::session_t *session, bool force_delete=false);

// FTE pkt loop (infinite loop)
void fte_start(uint8_t fte_id);

// diable all fte threads
void disable_fte();

// Send a packet on ASQ
// ***Should be called from FTE thread***
hal_ret_t fte_asq_send(hal::pd::cpu_to_p4plus_header_t* cpu_header,
                       hal::pd::p4plus_to_p4_header_t* p4plus_header,
                       uint8_t* pkt, size_t pkt_len);
                       
// Get fte_id
// ***Should be called from FTE thread***
uint8_t fte_id();

// Asynchronouly executes the fn in the specified fte thread,
// If the softq is full, it blocks until a slot is empty.
// ***Should be called from non FTE thread***
typedef void (*softq_fn_t)(void *data);
hal_ret_t fte_softq_enqueue(uint8_t fte_id, softq_fn_t fn, void *data);

// Executes the fn in the specified fte thread and blocks until the
// the function is executed by the fte thread.
// ***Should be called from non FTE thread***
hal_ret_t fte_execute(uint8_t fte_id, softq_fn_t fn, void *data);

// FTE Init routine
hal_ret_t init();

} // namespace fte
