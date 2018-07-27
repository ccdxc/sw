// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SDK_LINKMGR_TYPES_HPP__
#define __SDK_LINKMGR_TYPES_HPP__

#include <atomic>
#include "sdk/types.hpp"

namespace sdk {
namespace linkmgr {

#define CONTROL_CORE_ID          0
#define LINKMGR_THREAD_ID_MAX    3
#define LINKMGR_CONTROL_Q_SIZE   128
#define LINKMGR_LINK_POLL_TIME   10000 // 10000 msecs = 10 secs
#define MAX_LINK_BRINGUP_TIMEOUT 1000 // 10000 msecs = 10 secs

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

#define LINKMGR_CALLOC(var, ID, type, ...)  {  \
    void  *mem   = NULL;                       \
    mem = SDK_CALLOC(ID, sizeof(type));        \
    SDK_ABORT(mem != NULL);                    \
    var = new (mem) type(__VA_ARGS__);         \
}

enum class port_link_sm_t {
    PORT_LINK_SM_DISABLED,
    PORT_LINK_SM_ENABLED,
    PORT_LINK_SM_SERDES_CFG,
    PORT_LINK_SM_WAIT_SERDES_RDY,
    PORT_LINK_SM_MAC_CFG,
    PORT_LINK_SM_SIGNAL_DETECT,
    PORT_LINK_SM_WAIT_MAC_SYNC,
    PORT_LINK_SM_WAIT_MAC_FAULTS_CLEAR,
    PORT_LINK_SM_UP
};

typedef enum linkmgr_thread_id_e {
    LINKMGR_THREAD_ID_CTRL,
    LINKMGR_THREAD_ID_PERIODIC,
    LINKMGR_THREAD_ID_CFG
} linkmgr_thread_id_t;

typedef enum linkmgr_opn_e {
    LINKMGR_OPERATION_PORT_ENABLE,
    LINKMGR_OPERATION_PORT_DISABLE,
    LINKMGR_OPERATION_PORT_BRINGUP_TIMER,
    LINKMGR_OPERATION_PORT_DEBOUNCE_TIMER,
    LINKMGR_OPERATION_PORT_LINK_POLL_TIMER
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
    std::atomic<bool>    done;    // TRUE if thread performed operation
    sdk_ret_t            status;  // result status of operation requested
    linkmgr_entry_data_t data;    // data passed by caller
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

}    // namespace linkmgr
}    // namespace sdk

#endif    // __SDK_LINKMGR_TYPES_HPP__
