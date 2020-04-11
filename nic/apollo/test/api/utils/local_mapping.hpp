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

enum lmap_attrs {
    LMAP_ATTR_VNIC        =  bit(0),
    LMAP_ATTR_SUBNET      =  bit(1),
    LMAP_ATTR_FAB_ENCAP   =  bit(2),
    LMAP_ATTR_VNIC_MAC    =  bit(3),
    LMAP_ATTR_PUBLIC_IP   =  bit(4),
    LMAP_ATTR_PROVIDER_IP =  bit(5),
    LMAP_ATTR_TAGS        =  bit(6),
};

// Local mapping feeder class
class local_mapping_feeder : public feeder {
public:
    // spec fields
    pds_local_mapping_spec_t spec;

    // feeder cfg and state
    uint32_t num_vnics;
    uint32_t num_ip_per_vnic;
    uint32_t curr_vnic_ip_cnt;

    // constructor
    local_mapping_feeder() { };

    local_mapping_feeder(const local_mapping_feeder& feeder);

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
    void key_build(pds_obj_key_t *key) const;
    void spec_build(pds_local_mapping_spec_t *spec) const;

    // compare routines
    bool key_compare(const pds_obj_key_t *key) const;
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
        << " vpc: " << obj.spec.skey.vpc.str()
        << " subnet: " << obj.spec.subnet.str()
        << " vnic_ip: " << ipaddr2str(&obj.spec.skey.ip_addr)
        << " vnic: " << obj.spec.vnic.str()
        << " vnic_mac: " << macaddr2str(obj.spec.vnic_mac)
        << " encap_type: " << pds_encap2str(&obj.spec.fabric_encap)
        << " public_ip_valid: " << obj.spec.public_ip_valid
        << " public_ip: " << ipaddr2str(&obj.spec.public_ip);
    return os;
}

// CRUD prototypes
// mapping exposes two internal APIs for local/remote but both
// take the same key.
using pds_local_mapping_key_t = pds_mapping_key_t;

API_CREATE(local_mapping);
API_READ(local_mapping);
API_UPDATE(local_mapping);
API_DELETE(local_mapping);

// Local mapping crud helper prototypes
void lmap_create(local_mapping_feeder& feeder);
void lmap_read(local_mapping_feeder& feeder, sdk_ret_t exp_result = SDK_RET_OK);
void lmap_update(local_mapping_feeder& feeder, pds_local_mapping_spec_t *spec,
                 uint64_t chg_bmap, sdk_ret_t exp_result = SDK_RET_OK);
void lmap_delete(local_mapping_feeder& feeder);

}    // namespace api
}    // namespace test

#endif // __TEST_API_UTILS_LOCAL_MAPPING_HPP__

