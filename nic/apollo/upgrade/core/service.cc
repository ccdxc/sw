
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
///  Implementation of upgrade classes and methods
///
//----------------------------------------------------------------------------


#include <iostream>
#include <boost/unordered_map.hpp>
#include <boost/container/vector.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/upgrade/core/service.hpp"
#include "nic/apollo/upgrade/core/ipc/notify_endpoint.hpp"

namespace upg {

svc_t::svc_t(std::string name):name_(name){};
svc_t::~svc_t(void){};

std::string svc_t::name(void) {
    return name_;
};

std::string svc_t::thread_name(void) {
    return thread_name_;
};

uint32_t svc_t::thread_id(void) {
    return thread_id_;
};

void svc_t::set_thread_name(std::string name) {
    thread_name_= name;
};

void svc_t::set_thread_id(uint32_t id) {
    thread_id_= id;
};

svc_t& svc_t::operator = (const svc_t &obj) {
    name_        = obj.name_;
    thread_id_   = obj.thread_id_;
    thread_name_ = obj.thread_name_;
    return *this;
};

void svc_t::dispatch_event(upg_stage_t event) {
    upg_event_msg_t msg;

    msg.stage           = event;
    msg.rsp_status      = UPG_STATUS_OK;
    msg.rsp_thread_id   = thread_id_;

    strncpy(msg.rsp_thread_name, thread_name_.c_str(),
            sizeof(msg.rsp_thread_name));

    send_notification(msg);
};

void services_t::set_event_sequence(event_sequence_t type) {
    event_sequence_ = type;
};

event_sequence_t services_t::event_sequence(void) {
    return event_sequence_ ;
};

void services_t::add_svc(svc_t& service ) {
    svc_sequence_.push_back(service);
};

svc_sequence_t services_t::svc_sequence(void) {
    return svc_sequence_;
};

void services_t::dispatch_event(upg_stage_t event) {
    upg_event_msg_t msg;

    msg.stage           = event;
    msg.rsp_status      = UPG_STATUS_OK;

    broadcast_notification(msg);
};

svc_t& services_t::svc_by_name(std::string& name) {
    for (svc_t& svc: svc_sequence_) {
        if ( ! strcmp(svc.name().c_str(), name.c_str()) ) {
            return svc;
        }
    }
    // TODO: Error
    SDK_ASSERT(0);
};
}
