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

enum rmap_attrs {
    RMAP_ATTR_SUBNET      =  bit(0),
    RMAP_ATTR_FAB_ENCAP   =  bit(1),
    RMAP_ATTR_NH_TYPE     =  bit(2),
    RMAP_ATTR_VNIC_MAC    =  bit(3),
    RMAP_ATTR_TAGS        =  bit(4),
    RMAP_ATTR_PROVIDER_IP =  bit(5),
};

// Local mapping feeder class
class remote_mapping_feeder : public feeder {
public:
    // spec fields
    pds_remote_mapping_spec_t spec;

    // feeder cfg and state
    uint32_t num_teps;
    uint32_t num_vnic_per_tep;
    uint32_t curr_tep_vnic_cnt;

    // constructor
    remote_mapping_feeder() { };

    remote_mapping_feeder(const remote_mapping_feeder& feeder);

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
    void key_build(pds_obj_key_t *key) const;
    void spec_build(pds_remote_mapping_spec_t *spec) const;

    // compare routines
    bool key_compare(const pds_obj_key_t *key) const;
    bool spec_compare(const pds_remote_mapping_spec_t *spec) const;
    bool status_compare(const pds_mapping_status_t *status1,
                        const pds_mapping_status_t *status2) const;

    // create update feeder by changing a spec field
    void update_spec(uint32_t width);
};

// dump prototypes
inline std::ostream&
operator<<(std::ostream& os, const remote_mapping_feeder& obj) {
    os << "remote mapping feeder =>"
        << " vpc: " << obj.spec.skey.vpc.str()
        << " subnet: " << obj.spec.subnet.str()
        << " vnic_ip: " << ipaddr2str(&obj.spec.skey.ip_addr)
        << " vnic_mac: " << macaddr2str(obj.spec.vnic_mac)
        << " fabric encap: " << pds_encap2str(&obj.spec.fabric_encap)
        << " nexthop type" << obj.spec.nh_type
        << " nexthop/tep:" << obj.spec.tep.str();
    return os;
}

// CRUD prototypes
// mapping exposes two internal APIs for local/remote but both
// take the same key.
using pds_remote_mapping_key_t = pds_mapping_key_t;

API_CREATE(remote_mapping);
API_READ(remote_mapping);
API_UPDATE(remote_mapping);
API_DELETE(remote_mapping);

// Remote mapping crud helper prototypes
void rmap_create(remote_mapping_feeder& feeder);
void rmap_read(remote_mapping_feeder& feeder,
               sdk_ret_t exp_result = SDK_RET_OK);
void ramp_update(remote_mapping_feeder& feeder, pds_remote_mapping_spec_t *spec,
                 uint64_t chg_bmap, sdk_ret_t exp_result = SDK_RET_OK);
void rmap_delete(remote_mapping_feeder& feeder);

}    // namespace api
}    // namespace test

#endif // __TEST_API_UTILS_REMOTE_MAPPING_HPP__

