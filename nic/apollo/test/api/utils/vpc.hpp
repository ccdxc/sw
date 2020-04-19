//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_API_UTILS_VPC_HPP__
#define __TEST_API_UTILS_VPC_HPP__

#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/test/api/utils/api_base.hpp"
#include "nic/apollo/test/api/utils/feeder.hpp"

namespace test {
namespace api {

// Export variables
extern pds_obj_key_t k_vpc_key;

enum vpc_attrs {
    VPC_ATTR_TYPE         =  bit(0),
    VPC_ATTR_V4_PREFIX    =  bit(1),
    VPC_ATTR_V6_PREFIX    =  bit(2),
    VPC_ATTR_VR_MAC       =  bit(3),
    VPC_ATTR_FAB_ENCAP    =  bit(4),
    VPC_ATTR_V4_RTTBL     =  bit(5),
    VPC_ATTR_V6_RTTBL     =  bit(6),
    VPC_ATTR_NAT46_PREFIX =  bit(7),
    VPC_ATTR_TOS          =  bit(8),
};

// VPC test feeder class
class vpc_feeder : public feeder {
public:
    pds_vpc_spec_t spec;

    // Constructor
    vpc_feeder() { };

    // Initialize feeder with the base set of values
    void init(pds_obj_key_t key, pds_vpc_type_t type, std::string v4_cidr_str,
              int num_vpc = 1, std::string vr_mac_str = "00:00:de:ad:be:ef",
              std::string v6_cidr_str = "1000::1/64",
              std::string fabric_encap_str = "VxLAN 9999",
              pds_obj_key_t v4_rttbl = int2pdsobjkey(4000),
              pds_obj_key_t v6_rttbl = int2pdsobjkey(6000), uint8_t tos = 0);

    // Iterate helper routines
    void iter_next(int width = 1);

    // Build routines
    void key_build(pds_obj_key_t *key) const;
    void spec_build(pds_vpc_spec_t *spec) const;

    // Compare routines
    bool key_compare(const pds_obj_key_t *key) const;
    bool spec_compare(const pds_vpc_spec_t *spec) const;
    bool status_compare(const pds_vpc_status_t *status1,
                        const pds_vpc_status_t *status2) const;
};

// Dump prototypes
inline std::ostream&
operator<<(std::ostream& os, const pds_vpc_spec_t *spec) {
    os << spec->key.str()
       << " type: " << spec->type
       << " v4 cidr: " << ipv4pfx2str(&spec->v4_prefix)
       << " v6 cidr: " << ippfx2str(&spec->v6_prefix)
       << " vr mac: " << macaddr2str(spec->vr_mac)
       << " fabric encap: " << pds_encap2str(&spec->fabric_encap)
       << " v4 rt table: " << spec->v4_route_table.str()
       << " v6 rt table: " << spec->v6_route_table.str()
       << " nat46 pfx: " << ippfx2str(&spec->nat46_prefix)
       << " tos: " << (int)spec->tos;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_vpc_status_t *status) {
    os << " HW id: " << status->hw_id;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_vpc_info_t *obj) {
    os << "VPC info => "
       << &obj->spec << &obj->status << std::endl;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const vpc_feeder& obj) {
    os << "VPC feeder => " << &obj.spec << " ";
    return os;
}

// CRUD prototypes
API_CREATE(vpc);
API_READ(vpc);
API_UPDATE(vpc);
API_DELETE(vpc);

// VPC crud helper prototypes
void vpc_create(vpc_feeder& feeder);
void vpc_read(vpc_feeder& feeder, sdk_ret_t exp_result = SDK_RET_OK);
void vpc_update(vpc_feeder& feeder, pds_vpc_spec_t *spec,
                uint64_t chg_bmap, sdk_ret_t exp_result = SDK_RET_OK);
void vpc_delete(vpc_feeder& feeder);

// Misc function prototypes
void sample_vpc_setup(pds_batch_ctxt_t bctxt, pds_vpc_type_t type);
void sample_vpc_setup_validate(pds_vpc_type_t type);
void sample_vpc_teardown(pds_batch_ctxt_t bctxt, pds_vpc_type_t type);
void sample1_vpc_setup(pds_batch_ctxt_t bctxt, pds_vpc_type_t type);
void sample1_vpc_setup_validate(pds_vpc_type_t type);
void sample1_vpc_teardown(pds_batch_ctxt_t bctxt, pds_vpc_type_t type);

}    // namespace api
}    // namespace test

#endif    // __TEST_API_UTILS_VPC_HPP__
