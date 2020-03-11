// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// common definitions for core upgrade data structures
///
//----------------------------------------------------------------------------

#ifndef __UPGRADE_FSM_STAGE_HPP__
#define __UPGRADE_FSM_STAGE_HPP__

#include <string>
#include <boost/unordered_map.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/assign/list_of.hpp>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/upgrade/core/service.hpp"
#include "nic/apollo/upgrade/core/idl.hpp"

#define DEFAULT_RSP_TIMEOUT 200
#define STAGE_COUNT         11

namespace upg {

/// \brief Core upgrade stages
/// \remark
/// upgrade orchestration will happen to start from the compatcheck stage
/// (STAGE_ID_COMPAT_CHECK) to the exit stage (STAGE_ID_EXIT). Internal stage change
/// will happen base on predefined logic from a json file.
typedef enum stage_id_t {
    STAGE_ID_COMPAT_CHECK = 0,
    STAGE_ID_START = 1,
    STAGE_ID_PREPARE = 2,
    STAGE_ID_BACKUP = 3,
    STAGE_ID_SWITCHOVER = 4,
    STAGE_ID_VERIFY = 5,
    STAGE_ID_FINISH = 6,
    STAGE_ID_ABORT = 7,
    STAGE_ID_ROLLBACK = 8,
    STAGE_ID_CRITICAL = 9,
    STAGE_ID_EXIT = 10
} stage_id_t ;

/// \brief function call back type
/// \remark
/// Type of the registered callback , whether to call the address
/// before executing the stage or after executing the stage.
typedef enum stage_callback_t {
    PRE_STAGE  = 0,
    POST_STAGE = 1
} stage_callback_t;

/// \brief        Exported API to register application function callback
/// \param[in]    cb_type       Enum to differentiate pre or post callback
/// \param[in]    cb_stage      Enum to identify stage
/// \param[in]    callback      callback function
/// \return       status to callback registration
extern sdk_ret_t upg_register_stage_callback(stage_callback_t cb_type,
                                             stage_id_t cb_stage,
                                             void *(*callback)(void*),
                                             void* arg);

/// \brief  smallest data structure to decide what would be the next
/// stage
/// \remark
///  stage transition happens based on the current stage and responses
///  from all the services. This data structure helps building lookup
///  table for stage transition.
class stage_transition_t {
public:
    stage_transition_t(void);
    ~stage_transition_t(void);
    stage_transition_t(stage_id_t curr, svc_rsp_code_t rsp,
                       stage_id_t next);
    stage_id_t     from(void);
    void           set_from(stage_id_t id);
    stage_id_t     to(void);
    void           set_to(stage_id_t id);
    svc_rsp_code_t svc_rsp_code(void);
    void           set_svc_rsp_code(svc_rsp_code_t id);
private:
    stage_id_t     form_;
    svc_rsp_code_t svc_rsp_code_;
    stage_id_t     to_;
};

/// \brief a container for a list of stages
// typedef class stage_transition_t stage_transition_t;
typedef boost::container::vector<stage_transition_t>  transition_t;
/// \brief A lookup table
/// \remark
/// A hash map to identify the next stage based on current stage id and
/// the response from the stage. current stage id and response is
/// converted to string and that is used as a key to find the next stage
/// id.
typedef boost::unordered_map<std::string, stage_id_t> stage_map_t;

/// \brief    A path to a script name.
/// \remark
/// an absolute/relative path of a filename in the target system.
/// This file may be instructed to be called either pre hooks or a post
/// hook of any stage.
class script_t {
public:
    script_t(std::string path="");
    ~script_t(void);
    std::string path(void);
private:
    std::string path_;
};

typedef boost::container::vector<script_t> scripts_t;

/// \brief  The container of all internal upgrade data structures
/// \remark
/// A single source of truth to decide what has to be done in any stage
/// . This holds the response timeout, event sequence order, service
/// order, pre hooks, post hooks, also pre and post function callback
/// to the application.
class stage_t {
public:
    stage_t(void);
    stage_t(time_t svc_rsp_timeout, svc_sequence_t svc_seq,
            event_sequence_t evt_seq,
            transition_t transitions, scripts_t pre_sc,
            scripts_t post_sc);
    ~stage_t(void);

    time_t&           svc_rsp_timeout(void);
    svc_sequence_t&   svc_sequence(void);
    event_sequence_t& event_sequence(void);
    transition_t&     transitions(void);
    scripts_t&        pre_hook_scripts(void);
    scripts_t&        post_hook_scripts(void);
private:
    time_t           svc_rsp_timeout_;
    svc_sequence_t   svc_sequence_;
    event_sequence_t event_sequence_;
    transition_t     transitions_;
    scripts_t        pre_hook_scripts_;
    scripts_t        post_hook_scripts_;
    // app_callback_t     _pre_callback;
};

/// \brief A lookup container for stage id and object
typedef boost::unordered_map< stage_id_t, stage_t> stages_t;

static stages_t     fsm_stages;
static stage_map_t  fsm_lookup_tbl;

static const char *fsm_stageid_to_name [] =  {
    [STAGE_ID_COMPAT_CHECK] = "compatcheck",
    [STAGE_ID_START]        = "start",
    [STAGE_ID_PREPARE]      = "prepare",
    [STAGE_ID_BACKUP]       = "backup",
    [STAGE_ID_SWITCHOVER]   = "switchover",
    [STAGE_ID_VERIFY]       = "verify",
    [STAGE_ID_FINISH]       = "finish",
    [STAGE_ID_ABORT]        = "abort",
    [STAGE_ID_ROLLBACK]     = "rollback",
    [STAGE_ID_CRITICAL]     = "critical",
    [STAGE_ID_EXIT]         = "exit"
};

}
#endif    // __UPGRADE_FSM_STAGE_HPP___
