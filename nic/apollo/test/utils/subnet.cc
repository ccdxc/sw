//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/test/utils/subnet.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/vpc.hpp"

namespace api_test {

//----------------------------------------------------------------------------
// Subnet feeder class routines
//----------------------------------------------------------------------------

void
subnet_feeder::init(pds_subnet_key_t key, pds_vpc_key_t vpc_key,
                    std::string cidr_str, std::string vrmac_str,
                    int num_subnet) {
    this->key = key;
    this->vpc = vpc_key;
    this->cidr_str = cidr_str;
    this->vr_mac = vrmac_str;
    SDK_ASSERT(str2ipv4pfx((char *)cidr_str.c_str(), &pfx) == 0);
    v4_route_table.id = key.id;
    v6_route_table.id = key.id + 1024; // Unique id, 1-1024 reserved
                                       // for IPv4 rt table
    ing_v4_policy.id = key.id;
    ing_v6_policy.id = key.id + 1024;
    egr_v4_policy.id = key.id + 2048;
    egr_v6_policy.id = key.id + 3072;
    fabric_encap.val.vnid = key.id + 512; 
    fabric_encap.type = PDS_ENCAP_TYPE_VXLAN;

    num_obj = num_subnet;
}

void
subnet_feeder::iter_next(int width) {
    ip_addr_t ipaddr = {0};

    ip_prefix_ip_next(&pfx, &ipaddr);
    memcpy(&pfx.addr, &ipaddr, sizeof(ip_addr_t));
    key.id += width;
    v4_route_table.id += width;
    v6_route_table.id += width;
    ing_v4_policy.id += width;
    ing_v6_policy.id += width;
    egr_v4_policy.id += width;
    egr_v6_policy.id += width;
    fabric_encap.val.vnid += width;

    cur_iter_pos++;
}

void
subnet_feeder::key_build(pds_subnet_key_t *key) const {
    memset(key, 0, sizeof(pds_subnet_key_t));
    key->id = this->key.id;
}

void
subnet_feeder::spec_build(pds_subnet_spec_t *spec) const {
    memset(spec, 0, sizeof(pds_subnet_spec_t));
    this->key_build(&spec->key);

    spec->vpc.id = vpc.id;
    spec->v4_prefix.len = pfx.len;
    spec->v4_prefix.v4_addr = pfx.addr.addr.v4_addr;

    // Set the subnets IP (virtual router interface IP)
    if (!this->vr_ip.empty())
        extract_ipv4_addr(vr_ip.c_str(), &spec->v4_vr_ip);

    // Derive mac address from vr_ip if it has not been configured
    if (this->vr_mac.empty()) {
        MAC_UINT64_TO_ADDR(spec->vr_mac, (uint64_t)spec->v4_vr_ip);
    } else {
        mac_str_to_addr((char *)vr_mac.c_str(), spec->vr_mac);
    }

    spec->v4_route_table.id = v4_route_table.id;
    spec->v6_route_table.id = v6_route_table.id;
    spec->ing_v4_policy.id = ing_v4_policy.id;
    spec->ing_v6_policy.id = ing_v6_policy.id;
    spec->egr_v4_policy.id = egr_v4_policy.id;
    spec->egr_v6_policy.id = egr_v6_policy.id;
    spec->fabric_encap.type = fabric_encap.type;
    spec->fabric_encap.val.vnid = fabric_encap.val.vnid;
}

bool
subnet_feeder::key_compare(const pds_subnet_key_t *key) const {
    return (memcmp(key, &this->key, sizeof(pds_subnet_key_t)) == 0);
}

bool
subnet_feeder::spec_compare(const pds_subnet_spec_t *spec) const {
    if (spec->vpc.id != vpc.id)
        return false;

    if (spec->v4_route_table.id != v4_route_table.id)
        return false;

    if (spec->v6_route_table.id != v6_route_table.id)
        return false;

    if (spec->ing_v4_policy.id != ing_v4_policy.id)
        return false;

    if (spec->ing_v6_policy.id != ing_v6_policy.id)
        return false;

    if (spec->egr_v4_policy.id != egr_v4_policy.id)
        return false;

    if (spec->egr_v6_policy.id != egr_v6_policy.id)
        return false;

    if (!vr_mac.empty()) {
        mac_addr_t vrmac;
        mac_str_to_addr((char *)vr_mac.c_str(), vrmac);
        if (memcmp(&spec->vr_mac, vrmac, sizeof(mac_addr_t)))
            return false;
    }

    if (apulu()) {
        if (memcmp(&spec->fabric_encap, &fabric_encap, sizeof(pds_encap_t))) {
            return false;
        }
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

}    // namespace api_test
