// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_CFG_HPP__
#define __HAL_CFG_HPP__

#include "nic/include/base.h"
#include "sdk/list.hpp"

using sdk::lib::dllist_ctxt_t;
namespace hal {

enum {
    HAL_THREAD_ID_CFG        = 0,
    HAL_THREAD_ID_PERIODIC   = 1,
    HAL_THREAD_ID_FTE_MIN    = 2,
    HAL_THREAD_ID_FTE_MAX    = 4,
    HAL_THREAD_ID_ASIC_RW    = 5,
    HAL_THREAD_ID_MAX        = 6,
};

#define HAL_MAX_NAME_STR  16
const uint16_t MAX_FTE_THREADS =
               HAL_THREAD_ID_FTE_MAX - HAL_THREAD_ID_FTE_MIN + 1;

typedef enum hal_platform_mode_s {
    HAL_PLATFORM_MODE_NONE,
    HAL_PLATFORM_MODE_SIM,
    HAL_PLATFORM_MODE_HW,
    HAL_PLATFORM_MODE_HAPS,
    HAL_PLATFORM_MODE_RTL,
} hal_platform_mode_t;

typedef enum hal_feature_set_s {
    HAL_FEATURE_SET_NONE,
    HAL_FEATURE_SET_IRIS,
    HAL_FEATURE_SET_GFT,
} hal_feature_set_t;

typedef struct hal_cfg_s {
    hal_platform_mode_t  platform_mode;
    char                 asic_name[HAL_MAX_NAME_STR];
    std::string          grpc_port;
    std::string          loader_info_file;
    char                 feature_set[HAL_MAX_NAME_STR];
    hal_feature_set_t    features;
    std::string          forwarding_mode;
    bool                 async_en;
    std::string          default_config_dir;
    uint16_t             num_control_threads;
    uint16_t             num_data_threads;
    uint64_t             control_cores_mask;
    uint64_t             data_cores_mask;
} hal_cfg_t;

//------------------------------------------------------------------------------
// HAL config operations
//------------------------------------------------------------------------------
typedef enum cfg_op_e {
    CFG_OP_NONE,
    CFG_OP_READ,
    CFG_OP_WRITE,
} cfg_op_t;

//------------------------------------------------------------------------------
// Wrappers on HAL config db oeprations by APIs
//------------------------------------------------------------------------------
hal_ret_t hal_cfg_db_open(cfg_op_t op);
hal_ret_t hal_cfg_db_close(void);

//------------------------------------------------------------------------------
// HAL config db APIs store some context in the cfg_db_ctxt_t
// NOTE: this context is per thread, not for the whole process
//------------------------------------------------------------------------------
typedef struct cfg_db_ctxt_s {
    bool                   cfg_db_open_;  // true if cfg db is opened
    cfg_op_t               cfg_op_;       // operation for which db is opened
    cfg_version_t          rversion_;     // read version to use for lookups
    cfg_version_t          wversion_;     // write version to commit db changes with
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
    HAL_OBJ_ID_NONE,
    HAL_OBJ_ID_VRF,
    HAL_OBJ_ID_SECURITY_PROFILE,
    HAL_OBJ_ID_DOS_POLICY,
    HAL_OBJ_ID_L2SEG,
    HAL_OBJ_ID_NETWORK,
    HAL_OBJ_ID_INTERFACE,
    HAL_OBJ_ID_LIF,
    HAL_OBJ_ID_ENDPOINT,
    HAL_OBJ_ID_SESSION,
    HAL_OBJ_ID_ACL,
    HAL_OBJ_ID_SECURITY_GROUP,
    HAL_OBJ_ID_SECURITY_POLICY,
    HAL_OBJ_ID_PORT,
    HAL_OBJ_ID_MC_ENTRY,
    HAL_OBJ_ID_QOS_CLASS,
    HAL_OBJ_ID_COPP,
    HAL_OBJ_ID_GFT_EXACT_MATCH_PROFILE,
    HAL_OBJ_ID_GFT_HDR_TRANSPOSITION_PROFILE,
    HAL_OBJ_ID_GFT_EXACT_MATCH_FLOW_ENTRY,

    HAL_OBJ_ID_MAX
} hal_obj_id_t;

}    // namespace hal

#endif    // __HAL_CFG_HPP__

