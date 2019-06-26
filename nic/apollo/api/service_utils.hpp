//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains service mapping object utility routines
///
//----------------------------------------------------------------------------

#ifndef __API_SVC_MAPPING_UTILS_HPP__
#define __API_SVC_MAPPING_UTILS_HPP__

#include <iostream>
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/apollo/api/include/pds_service.hpp"

inline std::ostream&
operator<<(std::ostream& os, const pds_svc_mapping_key_t *key) {
    os << " vpc: " << key->vpc.id
       << " vip: " << ipaddr2str(&key->vip)
       << " svc port: " << key->svc_port;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_svc_mapping_spec_t *spec) {
    os << &spec->key
       << " backend vpc: " << spec->vpc.id
       << " backend ip: " << ipaddr2str(&spec->backend_ip)
       << " backend svc port " << spec->svc_port
       << " backend provider ip " << ipaddr2str(&spec->backend_provider_ip);
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_svc_mapping_info_t *obj) {
    os << "Svc mapping info =>"
       << &obj->spec
       << std::endl;
    return os;
}

#endif    // __API_SVC_MAPPING_UTILS_HPP__
