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
/// How to send different upgrade events (_stage_id_t) to list of
/// interested services. events can be send either 'serial'  or
/// 'parallel' order to services.
typedef enum event_sequence_t {
    PARALLEL = 0,
    SERIAL   = 1,
} event_sequence_t;

/// \brief    A service is identified by a name(example: pdsagent)
class svc_t {
public:
    svc_t(std::string name="");
    ~svc_t(void);
    std::string name(void);
    std::string thread_name(void);
    void        set_thread_name(std::string name);
    uint32_t    thread_id(void);
    void    set_thread_id(uint32_t id);
    svc_t& operator = (const svc_t &obj) ;
    void dispatch_event(upg_stage_t event);

private:
    std::string  name_;
    uint32_t     thread_id_;
    std::string  thread_name_;
};

/// \brief a container for a ordered list of interested services
typedef boost::container::vector<svc_t> svc_sequence_t;

/// \brief    A global service configuration
/// \remark
///  This is kinda default policy holder for all services and pds
/// upgrade logic. It holds the behaviour of event sequencing and
/// service order, a default behavior which guides upgrade orchestrator
/// how each interested service should receive event whether in a serial
/// order as specified in svc_sequence_t or in parallel.
///
/// This default behaviour can be altered when in each stage specifies
/// their custom policy by defining svc_sequence_t and event_sequence_t.
/// This customization has to be done from pds upgrade json file and a
/// new header needs to be generated.
class services_t {
public:
    event_sequence_t event_sequence(void);
    svc_sequence_t   svc_sequence(void);
    void             set_event_sequence(event_sequence_t type);
    void             add_svc(svc_t& service);
    svc_t&           svc_by_name(std::string& name);
    void             dispatch_event(upg_stage_t event);
private:
    event_sequence_t event_sequence_;
    svc_sequence_t   svc_sequence_;
};

static services_t fsm_services;
}
#endif    // __UPGRADE_FSM_SVC_HPP_
