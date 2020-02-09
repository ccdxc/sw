//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_API_UTILS_LOCAL_MAPPING_HPP__
#define __TEST_API_UTILS_LOCAL_MAPPING_HPP__

#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/test/api/utils/api_base.hpp"
#include "nic/apollo/test/api/utils/feeder.hpp"
#include "nic/apollo/test/api/utils/vnic.hpp"

#define PDS_MAX_VNIC_IP 33

namespace test {
namespace api {

// Local mapping feeder class
class local_mapping_feeder : public feeder {
public:
    // spec fields
    pds_mapping_type_t map_type;
    pds_obj_key_t vpc;
    pds_obj_key_t subnet;
    pds_encap_t fabric_encap;
    uint64_t vnic_mac_u64;
    ip_prefix_t vnic_ip_pfx;
    pds_obj_key_t vnic;
    bool public_ip_valid;
    ip_prefix_t public_ip_pfx;

    // feeder cfg and state
    uint32_t num_vnics;
    uint32_t num_ip_per_vnic;
    uint32_t curr_vnic_ip_cnt;

    // constructor
    local_mapping_feeder() { };

    local_mapping_feeder(const local_mapping_feeder& feeder) {
        this->map_type = feeder.map_type;
        this->vpc = feeder.vpc;
        this->subnet = feeder.subnet;
        this->fabric_encap = feeder.fabric_encap;
        this->vnic_mac_u64 = feeder.vnic_mac_u64;
        this->vnic_ip_pfx = feeder.vnic_ip_pfx;
        this->vnic = feeder.vnic;
        this->public_ip_valid = feeder.public_ip_valid;
        this->public_ip_pfx = feeder.public_ip_pfx;
        this->num_vnics = feeder.num_vnics;
        this->num_ip_per_vnic = feeder.num_ip_per_vnic;
        this->curr_vnic_ip_cnt = feeder.curr_vnic_ip_cnt;

        this->num_obj = feeder.num_obj;
    }

    // initialize feeder with base set of values
    void init(pds_obj_key_t vpc = int2pdsobjkey(1),
              pds_obj_key_t subnet = int2pdsobjkey(1),
              std::string vnic_ip_str= "0.0.0.0/0",
              uint64_t vnic_mac = 0x00030b020a02,
              pds_encap_type_t encap_type = PDS_ENCAP_TYPE_MPLSoUDP,
              uint32_t encap_val = 1,
              pds_obj_key_t vnic = int2pdsobjkey(1),
              bool is_public_ip_valid = 1,
              std::string pub_ip_str = "0.0.0.0/0",
              uint32_t num_vnics = k_max_vnic,
              uint32_t num_ip_per_vnic = PDS_MAX_VNIC_IP,
              pds_mapping_type_t map_type = PDS_MAPPING_TYPE_L3);

    // iterate helper
    void iter_next(int width = 1);

    // build routines
    void key_build(pds_mapping_key_t *key) const;
    void spec_build(pds_local_mapping_spec_t *spec) const;

    // compare routines
    bool key_compare(const pds_mapping_key_t *key) const;
    bool spec_compare(const pds_local_mapping_spec_t *spec) const;
    bool status_compare(const pds_mapping_status_t *status1,
                        const pds_mapping_status_t *status2) const;

    // create update feeder by changing a spec field
    void update_spec(uint32_t width);
};

// dump prototypes
inline std::ostream&
operator<<(std::ostream& os, const local_mapping_feeder& obj) {
    os << "local mapping feeder =>"
        << " vpc: " << obj.vpc.str()
        << " subnet: " << obj.subnet.str()
        << " vnic_ip: " << ipaddr2str(&obj.vnic_ip_pfx.addr)
        << " vnic: " << obj.vnic.str()
        << " vnic_mac: " << mac2str(obj.vnic_mac_u64)
        << " encap_type: " << pds_encap2str(&obj.fabric_encap)
        << " public_ip_valid: " << obj.public_ip_valid
        << " public_ip: " << ipaddr2str(&obj.public_ip_pfx.addr);
    return os;
}

// CRUD prototypes
// mapping exposes two internal APIs for local/remote but both
// take the same key.
using pds_local_mapping_key_t = pds_mapping_key_t;

API_CREATE(local_mapping);
API_READ_TMP(local_mapping);
API_UPDATE(local_mapping);
API_DELETE_TMP(local_mapping);

}    // namespace api
}    // namespace test

#endif // __TEST_API_UTILS_LOCAL_MAPPING_HPP__

