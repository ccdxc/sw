#pragma once

#include "nic/include/base.hpp"
#include "nic/include/fte_ctx.hpp"
#include <tins/tins.h>

namespace fte {

// FTE pipeline progress action codes
//  continue - continue the pipeline
//  end - end processing
//  finish - finish processing current flow
//  restart - restart the processing from the begining
//            (may select a diffrent pipeline)
#define FTE_PIPELINE_ACTION_ENTRIES(ENTRY)       \
    ENTRY(PIPELINE_CONTINUE, 0, "continue")      \
    ENTRY(PIPELINE_END,      1, "end")           \
    ENTRY(PIPELINE_FINISH,   2, "finish")       \
    ENTRY(PIPELINE_RESTART,  3, "restart")

DEFINE_ENUM(pipeline_action_t, FTE_PIPELINE_ACTION_ENTRIES)
#undef FTE_PIPELINE_ACTION_ENTRIES

typedef std::function<pipeline_action_t(ctx_t &ctx)> exec_handler_t;
typedef void (*feature_state_init_t)(void *state); // Init calback for feature specific state
typedef pipeline_action_t (*session_delete_handler_t)(ctx_t &ctx); // Session delete callback
typedef pipeline_action_t (*session_get_handler_t)(ctx_t &ctx); // Session get callback
typedef pipeline_action_t (*session_update_handler_t)(ctx_t &ctx); // Session update callback

typedef struct feature_info_s {
    // feature speicific per ctx state (this is not persisited
    // across packets, only valid during processing of one packet)
    size_t                           state_size;
    feature_state_init_t             state_init_fn;
    session_delete_handler_t         sess_del_cb;
    session_get_handler_t            sess_get_cb;
    session_update_handler_t         sess_upd_cb;

    // TODO(goli) need to define these
    // grpc_session_create_handler_t
    // grpc_session_get_handler_t
    // session_update_handler_t
} feature_info_t;

hal_ret_t register_feature(const std::string& name,
                           const exec_handler_t &exec_handler,
                           const feature_info_t &feature_info = {});

hal_ret_t unregister_feature(const std::string& name);

// Selects a pipeline and invokes features in that pipeline
// Doesn't update P4 tables and write packet to send queue
hal_ret_t execute_pipeline(ctx_t &ctx);

// for unit test code only
void unregister_features_and_pipelines();

// grpc session request
hal_ret_t session_create(SessionSpec& spec, SessionResponse *rsp);
hal_ret_t session_delete(SessionDeleteRequest& spec, SessionDeleteResponse *rsp);
hal_ret_t session_get(hal::session_t *session, SessionGetResponse *response);

// sync session request - Create or Update
hal_ret_t sync_session(fte_session_args_t args);

// Session delete request
hal_ret_t session_delete_in_fte(hal_handle_t session_handle, bool force_delete=false);
hal_ret_t session_delete(hal::session_t *session, bool force_delete=false);
hal_ret_t session_delete_async(hal::session_t *session, bool force_delete=false);
hal_ret_t session_update_async(hal::session_t *session, uint64_t bitmap);
hal_ret_t session_update_in_fte(hal_handle_t session_handle, uint64_t bitmap);

// Find if a feature is enabled on a session or not
bool session_is_feature_enabled(hal::session_t *session, const char *feature);

// Get logger instance for the current thread
ipc_logger *get_current_ipc_logger_inst(void);

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

// API to increment per-instance stats for fte errors
void incr_inst_fte_error (hal_ret_t rc);

// API to fetch per-instance FTE stats
fte_stats_t fte_stats_get (uint8_t fte_id, bool clear_on_read=false);


// API to fetch per-instance FTE txrxstats
fte_txrx_stats_t fte_txrx_stats_get (uint8_t fte_id, bool clear_on_read=false);

// API to bypass packet processing in FTE
void set_bypass_fte (uint8_t fte_id, bool bypass_fte=false);

// API to increment per-instance stats for freed TX resources
void incr_inst_freed_tx_stats(void);

//Compute CPS
void fte_inst_compute_cps(void);

hal_ret_t fte_softq_stats_get(uint8_t fte_id, int &stat);
// Set max session per fte
void set_fte_max_sessions(uint8_t fte_id, uint64_t max_session);

// Set Quisce on FTE
void fte_set_quiesce(uint8_t fte_id, bool quiesce);

hal_ret_t fte_inject_eth_pkt(const lifqid_t &lifq, hal_handle_t src_ifh, hal_handle_t src_l2segh,
                              std::vector<Tins::EthernetII> &pkts, bool add_padding=false);

} // namespace fte
