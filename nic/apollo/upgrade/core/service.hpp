// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// common definitions for core upgrade data structures
///
//----------------------------------------------------------------------------

#ifndef __UPGRADE_FSM_SVC_HPP__
#define __UPGRADE_FSM_SVC_HPP__

#include <string>
#include <boost/unordered_map.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/assign/list_of.hpp>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/upgrade/include/upgrade.hpp"
#include "nic/apollo/upgrade/core/ipc/ipc.hpp"

namespace upg {
/// \brief Core upgrade acceptable service response
/// \remark
/// upgrade orchestration logic can accept only of these response from
/// a service(which getting being upgraded). Internal stage change
/// happens base on this response code.
typedef enum svc_rsp_code_t {
    SVC_RSP_OK   = 0,
    SVC_RSP_FAIL = 1,
    SVC_RSP_CRIT = 2,
    SVC_RSP_NONE = 3
} svc_rsp_code_t;

/// \brief
/// \remark
/// How to send different upgrade events (upg_stage_t) to list of
/// interested services. events can be send either 'serial'  or
/// 'parallel' order to services.
typedef enum event_sequence_t {
    PARALLEL = 0,
    SERIAL   = 1,
} event_sequence_t;

/// \brief    A service is identified by a name(example: pdsagent)
class upg_svc {
public:
    upg_svc(std::string name="") : name_(name) { };

    ~upg_svc(void){};

    std::string name(void) const {
        return name_;
    };

    uint32_t ipc_id(void) const {
        return ipc_id_;
    };

    void set_ipc_id(const uint32_t id) {
        ipc_id_= id;
    };

    upg_svc& operator = (const upg_svc &obj) ;
    void dispatch_event(ipc::ipc_svc_dom_id_t dom,
                        upg_stage_t event) const ;

private:
    std::string  name_;
    uint32_t     ipc_id_;
};

/// \brief a container for a ordered list of interested services
typedef boost::container::vector<std::string> svc_sequence_list;

/// \brief a container for a ordered list of interested services
typedef boost::unordered_map<std::string, upg_svc> upg_svc_map;

}
#endif    // __UPGRADE_FSM_SVC_HPP_
