// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SDK_LINKMGR_TYPES_HPP__
#define __SDK_LINKMGR_TYPES_HPP__

#include <atomic>
#include "include/sdk/types.hpp"
#include "include/sdk/base.hpp"

namespace sdk {
namespace linkmgr {

#define CONTROL_CORE_ID          0
#define LINKMGR_CONTROL_Q_SIZE   128
#define LINKMGR_LINK_POLL_TIME   1000  // 1000 msecs = 1 sec
#define MAX_LINK_BRINGUP_TIMEOUT 5000  // 5000 msecs = 5 secs

#define MXP_BASE_HAPS           0x01d00000
#define MXP_INST_STRIDE_HAPS    0x00100000
#define MXP_PORT_STRIDE_HAPS    0x2000
#define TEMAC_BASE_OFFSET_HAPS  0x1000

#define PHY_RESET_OFFSET_HAPS    0x4
#define SGMII_RESET_OFFSET_HAPS  0x2000
#define TEMAC_RESET_OFFSET_HAPS  0x2008
#define DATAPATH_RESET_OFFSET    0x200c

#define MDIO_SETUP_OFFSET_HAPS   0x500
#define MDIO_CTRL_OFFSET_HAPS    0x504
#define MDIO_DATA_WR_OFFSET_HAPS 0x508
#define MDIO_DATA_RD_OFFSET_HAPS 0x50c

#define SDK_LINKMGR_CALLOC(var, ID, type, ...)  {  \
    void  *mem   = NULL;                           \
    mem = SDK_CALLOC(ID, sizeof(type));            \
    SDK_ABORT(mem != NULL);                        \
    var = new (mem) type(__VA_ARGS__);             \
}

typedef enum linkmgr_opn_e {
    LINKMGR_OPERATION_PORT_ENABLE,
    LINKMGR_OPERATION_PORT_DISABLE,
    LINKMGR_OPERATION_PORT_BRINGUP_TIMER,
    LINKMGR_OPERATION_PORT_DEBOUNCE_TIMER,
    LINKMGR_OPERATION_PORT_LINK_POLL_TIMER,
    LINKMGR_OPERATION_XCVR_POLL_TIMER,
} linkmgr_opn_t;

typedef struct linkmgr_entry_data_s {
    void  *ctxt;   // data passed by caller
    void  *timer;  // timer data passed by timer callbacks
} linkmgr_entry_data_t;

//------------------------------------------------------------------------------
// linkmgr thread operation entry.
// one such entry is added to the queue for every operation
//------------------------------------------------------------------------------
typedef struct linkmgr_entry_ {
    uint8_t              opn;     // operation requested to perform
    bool                 done;    // TRUE if thread performed operation
    sdk_ret_t            status;  // result status of operation requested
    linkmgr_entry_data_t data;    // data passed by caller
} linkmgr_entry_t;

//------------------------------------------------------------------------------
// linkmgr thread maintains one queue per thread to serve
// operations requested by other thread, thus avoiding locking altogether
//------------------------------------------------------------------------------
typedef struct linkmgr_queue_s {
    uint32_t         nentries;                        // no. of entries in the queue
    uint16_t         pindx;                           // producer index
    uint16_t         cindx;                           // consumer index
    linkmgr_entry_t  entries[LINKMGR_CONTROL_Q_SIZE]; // entries
} linkmgr_queue_t;

//------------------------------------------------------------------------------
// linkmgr thread uses this to post / awake the linkgmr ctrl thread
//------------------------------------------------------------------------------
typedef struct linkmgr_sync_s {
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    int             post;
    bool            waiting;
} linkmgr_sync_t;



}    // namespace linkmgr
}    // namespace sdk

#endif    // __SDK_LINKMGR_TYPES_HPP__
