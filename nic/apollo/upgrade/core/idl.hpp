// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// common definitions for core upgrade data structures
///
//----------------------------------------------------------------------------

#ifndef __PDS_UPGMGR_IDL_HPP__
#define __PDS_UPGMGR_IDL_HPP__

#include <string>

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
    class idl_stage_t {
    public:
        idl_stage_t(std::string stage, std::string timeout,
                    std::string svcs,std::string evt_seq,
                    std::string pre_hooks,std::string post_hooks);
        idl_stage_t(void);
        ~idl_stage_t(void);
        std::string& stage(void);
        std::string& svc_rsp_timeout(void);
        std::string& svc_sequence(void);
        std::string& event_sequence(void);
        std::string& pre_hook_scripts(void);
        std::string& post_hook_scripts(void);
    private:
        std::string stage_;
        std::string svc_rsp_timeout_;
        std::string svc_sequence_;
        std::string event_sequence_;
        std::string pre_hook_scripts_;
        std::string post_hook_scripts_;
    };

}
#endif    // __PDS_UPGMGR_IDL_HPP__
