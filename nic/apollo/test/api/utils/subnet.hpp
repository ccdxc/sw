//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_API_UTILS_SUBNET_HPP__
#define __TEST_API_UTILS_SUBNET_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/test/api/utils/api_base.hpp"
#include "nic/apollo/test/api/utils/feeder.hpp"

namespace test {
namespace api {

enum subnet_attrs {
    SUBNET_ATTR_VPC        =  bit(0),
    SUBNET_ATTR_V4_PREFIX  =  bit(1),
    SUBNET_ATTR_V6_PREFIX  =  bit(2),
    SUBNET_ATTR_V4_VRIP    =  bit(3),
    SUBNET_ATTR_V6_VRIP    =  bit(4),
    SUBNET_ATTR_VR_MAC     =  bit(5),
    SUBNET_ATTR_V4_RTTBL   =  bit(6),
    SUBNET_ATTR_V6_RTTBL   =  bit(7),
    SUBNET_ATTR_V4_INGPOL  =  bit(8),
    SUBNET_ATTR_V6_INGPOL  =  bit(9),
    SUBNET_ATTR_V4_EGRPOL  =  bit(10),
    SUBNET_ATTR_V6_EGRPOL  =  bit(11),
    SUBNET_ATTR_FAB_ENCAP  =  bit(12),
    SUBNET_ATTR_HOST_IF    =  bit(13),
    SUBNET_ATTR_DHCP_POL   =  bit(14),
    SUBNET_ATTR_TOS        =  bit(15),
};

#define SUBNET_ATTR_POL SUBNET_ATTR_V4_INGPOL | SUBNET_ATTR_V6_INGPOL | \
                        SUBNET_ATTR_V4_EGRPOL | SUBNET_ATTR_V6_EGRPOL

// Subnet test feeder class
class subnet_feeder : public feeder {
public:
    pds_subnet_spec_t spec;

    // Constructor
    subnet_feeder() { };
    subnet_feeder(pds_subnet_spec_t *spec) {
        memcpy(&this->spec, spec, sizeof(*spec));
    }
    subnet_feeder(const subnet_feeder& feeder);

    // Initialize feeder with the base set of values
    void init(pds_obj_key_t key, pds_obj_key_t vpc_key,
              std::string cidr_str, std::string vrmac_str,
              int num_subnet = 1, int num_policies = 1,
              int start_policy_index = 0);

    // Iterate helper routines
    void iter_next(int width = 1);

    // Build routines
    void key_build(pds_obj_key_t *key) const;
    void spec_build(pds_subnet_spec_t *spec) const;

    // Compare routines
    bool key_compare(const pds_obj_key_t *key) const;
    bool spec_compare(const pds_subnet_spec_t *spec) const;
    bool status_compare(const pds_subnet_status_t *status1,
                        const pds_subnet_status_t *status2) const;
};

inline std::string
policy_str_get(uint8_t num_policies, const pds_obj_key_t *policy)
{
    std::string policy_str;

    for (uint8_t i = 0; i < num_policies; i++) {
        policy_str.append(policy[i].str());
        policy_str.append(", ");
    }
    return policy_str;
}

// Dump prototypes
inline std::ostream&
operator<<(std::ostream& os, const pds_subnet_spec_t *spec) {
    os << &spec->key
        << " vpc: " << spec->vpc.str()
        << " cidr_str: " << ipv4pfx2str(&spec->v4_prefix)
        << " vr_ip: " << ipv4addr2str(spec->v4_vr_ip)
        << " vr_mac: " << macaddr2str(spec->vr_mac)
        << " v4_rt: " << spec->v4_route_table.str()
        << " v6_rt: " << spec->v6_route_table.str()
        << " ing_v4_num_policies: " << spec->num_ing_v4_policy
        << " ing_v6_num_policies: " << spec->num_ing_v6_policy
        << " egr_v4_num_policies: " << spec->num_egr_v4_policy
        << " egr_v6_num_policies: " << spec->num_egr_v6_policy
        << " v4_ing_pol: " << policy_str_get(spec->num_ing_v4_policy, spec->ing_v4_policy)
        << " v6_ing_pol: " << policy_str_get(spec->num_ing_v6_policy, spec->ing_v6_policy)
        << " v4_egr_pol: " << policy_str_get(spec->num_egr_v4_policy, spec->egr_v4_policy)
        << " v6_egr_pol: " << policy_str_get(spec->num_egr_v6_policy, spec->egr_v6_policy)
        << " fabric encap: " << pds_encap2str(&spec->fabric_encap);
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_subnet_info_t *obj) {
    os << "Subnet info =>" << &obj->spec << std::endl;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const subnet_feeder& obj) {
    os << "Subnet feeder =>" << &obj.spec << " ";
    return os;
}

// CRUD prototypes
API_CREATE(subnet);
API_READ(subnet);
API_UPDATE(subnet);
API_DELETE(subnet);

// Export variables
extern pds_obj_key_t k_subnet_key;

// Subnet crud helper prototypes
void subnet_create(subnet_feeder& feeder);
void subnet_read(subnet_feeder& feeder, sdk_ret_t exp_result = SDK_RET_OK);
void subnet_update(subnet_feeder& feeder, pds_subnet_spec_t *spec,
                   uint64_t chg_bmap, sdk_ret_t exp_result = SDK_RET_OK);
void subnet_delete(subnet_feeder& feeder);

// Misc function prototypes
void sample_subnet_setup(pds_batch_ctxt_t bctxt);
void sample_subnet_teardown(pds_batch_ctxt_t bctxt);
void sample1_subnet_setup(pds_batch_ctxt_t bctxt);
void sample1_subnet_teardown(pds_batch_ctxt_t bctxt);
void subnet_spec_policy_fill(pds_subnet_spec_t *spec, uint8_t num_policies,
                             uint8_t start_policy_index);
void subnet_spec_route_table_fill(pds_subnet_spec_t *spec,
                                  uint16_t start_index);

}    // namespace api
}    // namespace test

#endif    // __TEST_API_UTILS_SUBNET_HPP__
