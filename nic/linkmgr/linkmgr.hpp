// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __LINKMGR_HPP__
#define __LINKMGR_HPP__

#include "nic/include/base.h"

namespace linkmgr {

#define LINKMGR_CONTROL_Q_SIZE                   128
#define LINKMGR_OPERATION_PORT_TIMER             0
#define LINKMGR_OPERATION_PORT_ENABLE            1
#define LINKMGR_OPERATION_PORT_DISABLE           2
#define LINKMGR_THREAD_ID_MAX                    2

//------------------------------------------------------------------------------
// notification by other threads
//------------------------------------------------------------------------------
hal_ret_t
linkmgr_notify (uint8_t operation, void *ctxt);

//------------------------------------------------------------------------------
// linkmgr thread operation entry.
// one such entry is added to the queue for every operation
//------------------------------------------------------------------------------
typedef struct linkmgr_entry_ {
    uint8_t           opn;     // operation requested to perform
    std::atomic<bool> done;    // TRUE if thread performed operation
    hal_ret_t         status;  // result status of operation requested
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

// per producer read/write request queues
extern linkmgr_queue_t g_linkmgr_workq[LINKMGR_THREAD_ID_MAX];

}    // namespace linkmgr

#endif  // __LINKMGR_HPP__
