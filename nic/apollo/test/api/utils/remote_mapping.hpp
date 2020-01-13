//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_API_UTILS_REMOTE_MAPPING_HPP__
#define __TEST_API_UTILS_REMOTE_MAPPING_HPP__

#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/test/api/utils/api_base.hpp"
#include "nic/apollo/test/api/utils/feeder.hpp"
#include "nic/apollo/test/api/utils/vnic.hpp"

#define PDS_MAX_TEP 1023
#define PDS_MAX_TEP_VNIC 1024

namespace test {
namespace api {

// Local mapping feeder class
class remote_mapping_feeder : public feeder {
public:
    // spec fields
    pds_mapping_type_t map_type;
    pds_obj_key_t vpc;
    pds_obj_key_t subnet;
    pds_encap_t fabric_encap;
    uint64_t vnic_mac_u64;
    ip_prefix_t vnic_ip_pfx;
    pds_nh_type_t nh_type;
    uint32_t nh_id;         //tep id or nexthop grpup id

    // feeder cfg and state
    uint32_t num_teps;
    uint32_t num_vnic_per_tep;
    uint32_t curr_tep_vnic_cnt;

    // constructor
    remote_mapping_feeder() { };

    remote_mapping_feeder(const remote_mapping_feeder& feeder) {
        this->map_type = feeder.map_type;
        this->vpc = feeder.vpc;
        this->subnet = feeder.subnet;
        this->fabric_encap = feeder.fabric_encap;
        this->nh_type = feeder.nh_type;
        this->nh_id = nh_id;
        this->vnic_mac_u64 = feeder.vnic_mac_u64;
        this->vnic_ip_pfx = feeder.vnic_ip_pfx;
        this->nh_type = feeder.nh_type;
        this->nh_id = feeder.nh_id;

        this->num_teps = feeder.num_teps;
        this->num_vnic_per_tep = feeder.num_vnic_per_tep;
        this->curr_tep_vnic_cnt = feeder.curr_tep_vnic_cnt;

        this->num_obj = feeder.num_obj;
    }

    // initialize feeder with base set of values
    void init(pds_obj_key_t vpc = int2pdsobjkey(1),
              pds_obj_key_t subnet = int2pdsobjkey(1),
              std::string vnic_ip_str= "0.0.0.0/0",
              uint64_t vnic_mac = 0x00030b020a02,
              pds_encap_type_t encap_type = PDS_ENCAP_TYPE_MPLSoUDP,
              uint32_t encap_val = 1,
              pds_nh_type_t nh_type = PDS_NH_TYPE_OVERLAY,
              uint32_t nh_id = 1,
              uint32_t num_teps = PDS_MAX_TEP,
              uint32_t num_vnic_per_tep = PDS_MAX_TEP_VNIC,
              pds_mapping_type_t map_type = PDS_MAPPING_TYPE_L3);

    // iterate helper
    void iter_next(int width = 1);

    // build routines
    void key_build(pds_mapping_key_t *key) const;
    void spec_build(pds_remote_mapping_spec_t *spec) const;

    // compare routines
    bool key_compare(const pds_mapping_key_t *key) const;
    bool spec_compare(const pds_remote_mapping_spec_t *spec) const;

    // create update feeder by changing a spec field
    void update_spec(uint32_t width);
};

// dump prototypes
inline std::ostream&
operator<<(std::ostream& os, const remote_mapping_feeder& obj) {
    os << "remote mapping feeder =>"
        << " vpc: " << obj.vpc.str()
        << " subnet_id: " << obj.subnet.id
        << " vnic_ip: " << ipaddr2str(&obj.vnic_ip_pfx.addr)
        << " vnic_mac: " << mac2str(obj.vnic_mac_u64)
        << " fabric encap: " << pds_encap2str(&obj.fabric_encap)
        << " next hop type" << obj.nh_type
        << " next hop/tep id" << obj.nh_id;
    return os;
}

// CRUD prototypes
// mapping exposes two internal APIs for local/remote but both
// take the same key.
using pds_remote_mapping_key_t = pds_mapping_key_t;

API_CREATE(remote_mapping);
API_READ_TMP(remote_mapping);
API_UPDATE(remote_mapping);
API_DELETE_TMP(remote_mapping);

}    // namespace api
}    // namespace test

#endif // __TEST_API_UTILS_REMOTE_MAPPING_HPP__

