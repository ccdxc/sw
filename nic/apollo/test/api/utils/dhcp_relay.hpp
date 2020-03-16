//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_API_UTILS_DHCP_RELAY_HPP__
#define __TEST_API_UTILS_DHCP_RELAY_HPP__

#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/test/api/utils/api_base.hpp"
#include "nic/apollo/test/api/utils/feeder.hpp"

namespace test {
namespace api {

// DHCP_RELAY test feeder class
class dhcp_policy_feeder : public feeder {
public:
    pds_dhcp_policy_spec_t spec;

    // Constructor
    dhcp_policy_feeder() { };
    dhcp_policy_feeder(const dhcp_policy_feeder& feeder) {
        memcpy(&this->spec, &feeder.spec, sizeof(pds_dhcp_policy_spec_t));
        this->num_obj = feeder.num_obj;
    }

    // Initialize feeder with the base set of values
    void init(pds_obj_key_t key, std::string agent_ip, std::string server_ip,
              pds_obj_key_t vpc, int num_obj = 1);

    // Iterate helper routines
    void iter_next(int width = 1) { cur_iter_pos++; }

    bool read_unsupported(void) const {
        return true;
    }

    // Build routines
    void key_build(pds_obj_key_t *key) const;
    void spec_build(pds_dhcp_policy_spec_t *spec) const;

    // Compare routines
    bool key_compare(const pds_obj_key_t *key) const;
    bool spec_compare(const pds_dhcp_policy_spec_t *spec) const;
    bool status_compare(const pds_dhcp_policy_status_t *status1,
                        const pds_dhcp_policy_status_t *status2) const;
};

// Dump prototypes
inline std::ostream&
operator<<(std::ostream& os, const pds_dhcp_policy_spec_t *spec) {
    os << " key: " << std::string(spec->key.str())
       << " agentip: " << ipaddr2str(&spec->relay_spec.agent_ip)
       << " serverip: " << ipaddr2str(&spec->relay_spec.server_ip)
       << " type: " << spec->type
       << " vpc id: " << std::string(spec->relay_spec.vpc.str());
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_tep_status_t *status) {
    os << "Status";
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_dhcp_policy_info_t *obj) {
    os << "DHCP Relay info =>"
       << &obj->spec
       << &obj->status
       << std::endl;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const dhcp_policy_feeder& obj) {
    os << "DHCP Relay feeder =>" << &obj.spec << " ";
    return os;
}


// CRUD prototypes
API_CREATE(dhcp_policy);
API_READ(dhcp_policy);
API_UPDATE(dhcp_policy);
API_DELETE(dhcp_policy);

}    // namespace api
}    // namespace test

#endif    // __TEST_API_UTILS_DHCP_RELAY_HPP__
