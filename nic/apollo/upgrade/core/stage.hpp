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
#include "nic/apollo/upgrade/include/pds_upgmgr.hpp"
#include "nic/apollo/upgrade/core/service.hpp"
#include "nic/apollo/upgrade/core/idl.hpp"

#define DEFAULT_RSP_TIMEOUT 200
#define STAGE_COUNT         11

namespace upg {
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
    typedef boost::unordered_map<std::string, stage_id_t> name_to_id_t;
    typedef boost::unordered_map<stage_id_t, std::string> id_to_name_t;

    static stages_t     fsm_stages;
    static name_to_id_t fsm_stagename_to_id;
    static id_to_name_t fsm_stageid_to_name;
    static stage_map_t   fsm_lookup_tbl;
}
#endif    // __UPGRADE_FSM_STAGE_HPP___
