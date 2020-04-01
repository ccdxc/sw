// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// common definitions for core upgrade data structures
///
//----------------------------------------------------------------------------

#ifndef __UPGRADE_CORE_IDL_HPP__
#define __UPGRADE_CORE_IDL_HPP__

#include <string>

namespace sdk {
namespace upg {
/// \brief This intermediate data structure holds json string to build
/// stage_t objects.
/// \remark
/// This class is closer to interface definition upgrade json but an
/// exact replica of upgrade internal data structure stage_t. This helps
/// creating internal objects.
/// Strings are delimiter separated, and ':' is used as delimiter.
/// This class helps to avoid complex construction of internal objects.
/// This class helps to implement per stage policy implementation, for
/// example one can override api response timeout for specific stage,
/// one can change the order of services and change the event sending
/// mechanism from parallel to serial.
class idl_upg_stage {
public:
    idl_upg_stage(void) {
        stage_             = "";
        svc_rsp_timeout_   = "";
        svc_sequence_      = "";
        event_sequence_    = "";
        pre_hook_scripts_  = "";
        post_hook_scripts_ = "";
    };

    idl_upg_stage( std::string stage, std::string timeout,
                              std::string svcs, std::string evt_seq,
                              std::string pre_hooks,
                              std::string post_hooks) {
        stage_             = stage;
        svc_rsp_timeout_       = timeout ;
        svc_sequence_      = svcs ;
        event_sequence_    = evt_seq ;
        pre_hook_scripts_  = pre_hooks ;
        post_hook_scripts_ = post_hooks ;
    };
    ~idl_upg_stage(void) {};
    std::string& stage(void) { return stage_; };
    std::string& svc_rsp_timeout(void) { return svc_rsp_timeout_; };
    std::string& svc_sequence(void) { return svc_sequence_; };
    std::string& event_sequence(void) { return event_sequence_; };
    std::string& pre_hook_scripts(void) { return pre_hook_scripts_; };
    std::string& post_hook_scripts(void) { return post_hook_scripts_; };

private:
    std::string stage_;
    std::string svc_rsp_timeout_;
    std::string svc_sequence_;
    std::string event_sequence_;
    std::string pre_hook_scripts_;
    std::string post_hook_scripts_;
};

}   // namespace upg
}   // namespace sdk

#endif    // __UPGRADE_CORE_IDL_HPP__
