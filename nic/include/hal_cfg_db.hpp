// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_CFG_DB_HPP__
#define __HAL_CFG_DB_HPP__

#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/list.hpp"

using sdk::lib::dllist_ctxt_t;

namespace hal {

//------------------------------------------------------------------------------
// HAL config operations
//------------------------------------------------------------------------------
typedef enum cfg_op_e {
    CFG_OP_NONE,
    CFG_OP_READ,
    CFG_OP_WRITE,
} cfg_op_t;

//------------------------------------------------------------------------------
// HAL config db APIs store some context in the cfg_db_ctxt_t
// NOTE: this context is per thread, not for the whole process
//------------------------------------------------------------------------------
typedef struct cfg_db_ctxt_s {
    bool                   cfg_db_open_;  // true if cfg db is opened
    cfg_op_t               cfg_op_;       // operation for which db is opened
    cfg_db_ctxt_s() {
        cfg_db_open_ = false;
        cfg_op_ = CFG_OP_NONE;
    }
} __PACK__ cfg_db_ctxt_t;
extern thread_local cfg_db_ctxt_t t_cfg_db_ctxt;

// dirty handle list entry
typedef struct dhl_entry_s {
    hal_handle_t    handle;         // handle of the object
    void            *obj;           // original object
    void            *cloned_obj;    // cloned object
    dllist_ctxt_t   dllist_ctxt;    // list context
} __PACK__ dhl_entry_t;

// operation (add/del/mdfy etc.) specific context
typedef struct cfg_op_ctxt_s {
    void             *app_ctxt;       // app/module specific context
    dllist_ctxt_t    dhl;             // dirty handle list
} __PACK__ cfg_op_ctxt_t;

typedef hal_ret_t (*hal_cfg_del_cb_t)(void *obj);

// delete callback provided per object
typedef hal_ret_t (*hal_cfg_op_cb_t)(cfg_op_ctxt_t *ctxt);
typedef hal_ret_t (*hal_cfg_commit_cb_t)(cfg_op_ctxt_t *ctxt);
typedef hal_ret_t (*hal_cfg_abort_cb_t)(cfg_op_ctxt_t *ctxt);
typedef hal_ret_t (*hal_cfg_cleanup_cb_t)(cfg_op_ctxt_t *ctxt);

//------------------------------------------------------------------------------
// HAL config object identifiers
//------------------------------------------------------------------------------
typedef enum hal_obj_id_e {
    HAL_OBJ_ID_NONE,                              // NOTE: must be first
    HAL_OBJ_ID_MIN,
    HAL_OBJ_ID_LIF                                = HAL_OBJ_ID_MIN,
    HAL_OBJ_ID_SECURITY_PROFILE,
    HAL_OBJ_ID_NETWORK,
    HAL_OBJ_ID_VRF,
    HAL_OBJ_ID_L2SEG,
    HAL_OBJ_ID_INTERFACE,
    HAL_OBJ_ID_NEXTHOP,
    HAL_OBJ_ID_ROUTE,
    HAL_OBJ_ID_ENDPOINT,
    HAL_OBJ_ID_SESSION,
    HAL_OBJ_ID_SECURITY_GROUP,
    HAL_OBJ_ID_SECURITY_POLICY,
    HAL_OBJ_ID_COPP,
    HAL_OBJ_ID_ACL,
    HAL_OBJ_ID_QOS_CLASS,
    HAL_OBJ_ID_MC_ENTRY,
    HAL_OBJ_ID_DOS_POLICY,
    HAL_OBJ_ID_GFT_EXACT_MATCH_PROFILE,
    HAL_OBJ_ID_GFT_HDR_TRANSPOSITION_PROFILE,
    HAL_OBJ_ID_GFT_EXACT_MATCH_FLOW_ENTRY,
    HAL_OBJ_ID_NAT_POOL,
    HAL_OBJ_ID_NAT_POLICY,
    HAL_OBJ_ID_NAT_MAPPING,
    HAL_OBJ_ID_PORT,                              // TBD: doesn't belong in HAL
    HAL_OBJ_ID_IPSEC_POLICY,
    HAL_OBJ_ID_TCP_PROXY_POLICY,
    HAL_OBJ_ID_FILTER,
    HAL_OBJ_ID_FTE_SPAN,
    HAL_OBJ_ID_MAX                                // NOTE: must be the last
} hal_obj_id_t;

}    // namespace hal

#endif    // __HAL_CFG_DB_HPP__

