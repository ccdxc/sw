// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SDK_LINKMGR_INTERNAL_HPP__
#define __SDK_LINKMGR_INTERNAL_HPP__

#include <atomic>
#include "sdk/base.hpp"
#include "sdk/types.hpp"

namespace sdk {
namespace linkmgr {

#define CONTROL_CORE_ID         0
#define LINKMGR_THREAD_ID_MAX   3
#define LINKMGR_CONTROL_Q_SIZE  128

typedef enum linkmgr_thread_id_e {
    LINKMGR_THREAD_ID_CTRL,
    LINKMGR_THREAD_ID_PERIODIC,
    LINKMGR_THREAD_ID_CFG
} linkmgr_thread_id_t;

typedef enum linkmgr_opn_e {
    LINKMGR_OPERATION_PORT_TIMER,
    LINKMGR_OPERATION_PORT_ENABLE,
    LINKMGR_OPERATION_PORT_DISABLE
} linkmgr_opn_t;

//------------------------------------------------------------------------------
// linkmgr thread operation entry.
// one such entry is added to the queue for every operation
//------------------------------------------------------------------------------
typedef struct linkmgr_entry_ {
    uint8_t           opn;     // operation requested to perform
    std::atomic<bool> done;    // TRUE if thread performed operation
    sdk_ret_t         status;  // result status of operation requested
    void              *data;   // data passed by called
} linkmgr_entry_t;

//------------------------------------------------------------------------------
// linkmgr thread maintains one queue per thread to serve
// operations requested by other thread, thus avoiding locking altogether
//------------------------------------------------------------------------------
typedef struct linkmgr_queue_s {
    std::atomic<uint32_t> nentries;    // no. of entries in the queue
    uint16_t              pindx;       // producer index
    uint16_t              cindx;       // consumer index
    linkmgr_entry_t      entries[LINKMGR_CONTROL_Q_SIZE];    // entries
} linkmgr_queue_t;

sdk_ret_t port_event_timer(void *ctxt);
sdk_ret_t port_event_enable(void *ctxt);
sdk_ret_t port_event_disable(void *ctxt);

sdk_ret_t
linkmgr_notify (uint8_t operation, void *ctxt);

bool
is_linkmgr_ctrl_thread();

}    // namespace linkmgr
}    // namespace sdk

#endif    // __SDK_LINKMGR_INTERNAL_HPP__
