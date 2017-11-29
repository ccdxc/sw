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
hal_ret_t add_feature(const std::string& name);
hal_ret_t register_feature(const std::string& name,
                           const exec_handler_t &exec_handler);
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

// FTE pkt loop (infinite loop)
void fte_start(uint8_t fte_id);

// Send a packet on ASQ
// ***Should be called from FTE thread***
hal_ret_t fte_asq_send(hal::pd::cpu_to_p4plus_header_t* cpu_header,
                       hal::pd::p4plus_to_p4_header_t* p4plus_header,
                       uint8_t* pkt, size_t pkt_len,
                       uint16_t dest_lif = hal::SERVICE_LIF_CPU,
                       uint8_t  qtype = CPU_ASQ_QTYPE,
                       uint32_t qid = CPU_ASQ_QID,
                       uint8_t  ring_number = CPU_SCHED_RING_ASQ);

// Asynchronouly executes the fn in the specified fte thread,
// If the softq is full, it blocks until a slot is empty.
// ***Should be called from non FTE thread***
typedef void (*softq_fn_t)(void *data);
hal_ret_t fte_softq_enqueue(uint8_t fte_id, softq_fn_t fn, void *data);

// FTE Init routine
hal_ret_t init();

} // namespace fte
