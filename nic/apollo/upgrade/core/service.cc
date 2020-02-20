
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

namespace upg {
    svc_t::svc_t(std::string name):name_(name){};
    svc_t::~svc_t(void){};

    std::string svc_t::name(void) {
        return name_;
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
    }
}
