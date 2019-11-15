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
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/api/include/pds_service.hpp"

inline std::ostream&
operator<<(std::ostream& os, const pds_svc_mapping_key_t *key) {
    os << " vpc: " << key->vpc.id
       << " backend ip: " << key->backend_ip
       << " backend port: " << key->backend_port;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_svc_mapping_spec_t *spec) {
    os << &spec->key
       << " vip: " << spec->vip
       << " svc port " << spec->svc_port
       << " backend provider ip " << spec->backend_provider_ip;
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
