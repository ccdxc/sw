//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/apollo/test/api/utils/dhcp_relay.hpp"

namespace test {
namespace api {

//----------------------------------------------------------------------------
// DHCP Relay feeder class routines
//----------------------------------------------------------------------------

void
dhcp_relay_feeder::init(pds_obj_key_t key, std::string agent_ip_str,
                        std::string server_ip_str, pds_obj_key_t vpc,
                        int num_objs) {
    this->spec.key = key;
    extract_ip_addr(agent_ip_str.c_str(), &this->spec.agent_ip);
    extract_ip_addr(server_ip_str.c_str(), &this->spec.server_ip);
    this->spec.vpc = vpc;
    num_obj = num_objs;
}

void
dhcp_relay_feeder::spec_build(pds_dhcp_relay_spec_t *spec) const {
    memcpy(spec, &this->spec, sizeof(pds_dhcp_relay_spec_t));
}

void
dhcp_relay_feeder::key_build(pds_obj_key_t *key) const {
    memcpy(key , &this->spec.key, sizeof(pds_obj_key_t));
}

bool
dhcp_relay_feeder::key_compare(const pds_obj_key_t *key) const {
    return (*key == this->spec.key);
}


bool
dhcp_relay_feeder::spec_compare(const pds_dhcp_relay_spec_t *spec) const {
    if (!IPADDR_EQ(&this->spec.server_ip, &spec->server_ip)) {
        return false;
    }
    if (!IPADDR_EQ(&this->spec.agent_ip, &spec->agent_ip)) {
        return false;
    }
    if (this->spec.vpc != spec->vpc) {
        return false;
    }
    return true;
}

bool
dhcp_relay_feeder::status_compare(const pds_dhcp_relay_status_t *status1,
                              const pds_dhcp_relay_status_t *status2) const {
    return true;
}

}    // namespace api
}    // namespace test
