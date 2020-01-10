//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/test/api/utils/subnet.hpp"
#include "nic/apollo/test/api/utils/vpc.hpp"

namespace test {
namespace api {

//----------------------------------------------------------------------------
// Subnet feeder class routines
//----------------------------------------------------------------------------

void
subnet_feeder::init(pds_subnet_key_t key, pds_vpc_key_t vpc_key,
                    std::string cidr_str, std::string vrmac_str,
                    int num_subnet) {
    ip_prefix_t pfx = {0};

    memset(&spec, 0, sizeof(pds_subnet_spec_t));
    spec.key = key;
    spec.vpc = vpc_key;
    str2ipv4pfx((char *)cidr_str.c_str(), &pfx);
    spec.v4_prefix.len = pfx.len;
    spec.v4_prefix.v4_addr = pfx.addr.addr.v4_addr;
    mac_str_to_addr((char *)vrmac_str.c_str(), spec.vr_mac);
    spec.v4_route_table.id = key.id;
    spec.v6_route_table.id = key.id + 1024; // Unique id, 1-1024 reserved
                                            // for IPv4 rt table
    // TODO: fix for multiple policies
    spec.num_ing_v4_policy = 1;
    spec.ing_v4_policy[0].id = 1;
    spec.num_ing_v6_policy = 1;
    spec.ing_v6_policy[0].id = 6;
    spec.num_egr_v4_policy = 1;
    spec.egr_v4_policy[0].id = 11;
    spec.num_egr_v6_policy = 1;
    spec.egr_v6_policy[0].id = 16;
    spec.fabric_encap.val.vnid = key.id + 512;
    spec.fabric_encap.type = PDS_ENCAP_TYPE_VXLAN;

    num_obj = num_subnet;
}

subnet_feeder::subnet_feeder(const subnet_feeder& feeder) {
    memcpy(&this->spec, &feeder.spec, sizeof(pds_subnet_spec_t));
    num_obj = feeder.num_obj;
}

void
subnet_feeder::iter_next(int width) {
    spec.key.id += width;
    spec.v4_prefix.v4_addr += (1 << spec.v4_prefix.len);
    spec.v4_route_table.id += width;
    spec.v6_route_table.id += width;
    spec.fabric_encap.val.vnid += width;

    cur_iter_pos++;
}

void
subnet_feeder::key_build(pds_subnet_key_t *key) const {
    memset(key, 0, sizeof(pds_subnet_key_t));
    key->id = spec.key.id;
}

void
subnet_feeder::spec_build(pds_subnet_spec_t *spec) const {
    memcpy(spec, &this->spec, sizeof(pds_subnet_spec_t));
}

bool
subnet_feeder::key_compare(const pds_subnet_key_t *key) const {
    return (memcmp(key, &this->spec.key, sizeof(pds_subnet_key_t)) == 0);
}

// TODO: fix for multiple policies
bool
subnet_feeder::spec_compare(const pds_subnet_spec_t *spec) const {
    if (spec->vpc != this->spec.vpc) {
        return false;
    }

    if (spec->v4_route_table.id != this->spec.v4_route_table.id) {
        return false;
    }

    if (spec->v6_route_table.id != this->spec.v6_route_table.id) {
        return false;
    }

    if (spec->ing_v4_policy[0].id != this->spec.ing_v4_policy[0].id) {
        return false;
    }

    if (spec->ing_v6_policy[0].id != this->spec.ing_v6_policy[0].id) {
        return false;
    }

    if (spec->egr_v4_policy[0].id != this->spec.egr_v4_policy[0].id) {
        return false;
    }

    if (spec->egr_v6_policy[0].id != this->spec.egr_v6_policy[0].id) {
        return false;
    }

    if (memcmp(&spec->vr_mac, this->spec.vr_mac, sizeof(mac_addr_t))) {
        return false;
    }

    if (memcmp(&spec->fabric_encap, &this->spec.fabric_encap,
               sizeof(pds_encap_t))) {
        return false;
    }
    return true;
}

//----------------------------------------------------------------------------
// Misc routines
//----------------------------------------------------------------------------

// do not modify these sample values as rest of system is sync with these
pds_subnet_key_t k_subnet_key = {.id = 1};
static subnet_feeder k_subnet_feeder;

void sample_subnet_setup(pds_batch_ctxt_t bctxt) {
    // setup and teardown parameters should be in sync
    k_subnet_feeder.init(k_subnet_key, k_vpc_key, "10.1.0.0/16",
                         "00:02:01:00:00:01");
    create(bctxt, k_subnet_feeder);
}

void sample_subnet_teardown(pds_batch_ctxt_t bctxt) {
    k_subnet_feeder.init(k_subnet_key, k_vpc_key, "10.1.0.0/16",
                         "00:02:01:00:00:01");
    del(bctxt, k_subnet_feeder);
}

void sample1_subnet_setup(pds_batch_ctxt_t bctxt) {
    // setup and teardown parameters should be in sync
    k_subnet_feeder.init(k_subnet_key, k_vpc_key, "10.0.0.0/8",
                         "00:02:01:00:00:01");
    create(bctxt, k_subnet_feeder);
}

void sample1_subnet_teardown(pds_batch_ctxt_t bctxt) {
    k_subnet_feeder.init(k_subnet_key, k_vpc_key, "10.0.0.0/8",
                         "00:02:01:00:00:01");
    del(bctxt, k_subnet_feeder);
}

}    // namespace api
}    // namespace test
