//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/test/api/utils/vpc.hpp"

namespace test {
namespace api {

//----------------------------------------------------------------------------
// VPC feeder class routines
//----------------------------------------------------------------------------

void
vpc_feeder::init(pds_obj_key_t key, pds_vpc_type_t type,
                 std::string cidr_str, std::string vr_mac,
                 uint32_t num_vpc) {
    this->key = key;
    this->type = type;
    this->cidr_str = cidr_str;
    this->vr_mac = vr_mac;
    this->fabric_encap.type = PDS_ENCAP_TYPE_VXLAN;
    this->fabric_encap.val.vnid = pdsobjkey2int(key) + 9999;
    SDK_ASSERT(str2ipv4pfx((char *)cidr_str.c_str(), &pfx) == 0);
    num_obj = num_vpc;
}

void
vpc_feeder::iter_next(int width) {
    ip_addr_t ipaddr = {0};

    ip_prefix_ip_next(&pfx, &ipaddr);
    memcpy(&pfx.addr, &ipaddr, sizeof(ip_addr_t));
    key = int2pdsobjkey(pdsobjkey2int(key) + width);
    if (artemis() || apulu())
        fabric_encap.val.vnid += width;
    cur_iter_pos++;
}

void
vpc_feeder::key_build(pds_obj_key_t *key) const {
    memset(key, 0, sizeof(pds_obj_key_t));
    *key = this->key;
}

void
vpc_feeder::spec_build(pds_vpc_spec_t *spec) const {
    memset(spec, 0, sizeof(pds_vpc_spec_t));
    this->key_build(&spec->key);

    spec->type = type;
    spec->v4_prefix.len = pfx.len;
    spec->v4_prefix.v4_addr = pfx.addr.addr.v4_addr;
    spec->fabric_encap = fabric_encap;
    mac_str_to_addr((char *)vr_mac.c_str(), spec->vr_mac);
}

bool
vpc_feeder::key_compare(const pds_obj_key_t *key) const {
    return (this->key == *key);
}

bool
vpc_feeder::spec_compare(const pds_vpc_spec_t *spec) const {
    if (spec->type != type)
        return false;

    if (memcmp(&spec->fabric_encap, &fabric_encap, sizeof(pds_encap_t))) {
        return false;
    }

    if (apulu()) {
        if (!vr_mac.empty()) {
            mac_addr_t vrmac;
            mac_str_to_addr((char *)vr_mac.c_str(), vrmac);
            if (memcmp(&spec->vr_mac, vrmac, sizeof(mac_addr_t))) {
                return false;
            }
        }
    }
    return true;
}

//----------------------------------------------------------------------------
// Misc routines
//----------------------------------------------------------------------------

// do not modify these sample values as rest of system is sync with these
pds_obj_key_t k_vpc_key = int2pdsobjkey(1);
static vpc_feeder k_vpc_feeder;

void sample_vpc_setup(pds_batch_ctxt_t bctxt, pds_vpc_type_t type) {
    // setup and teardown parameters should be in sync
    k_vpc_feeder.init(k_vpc_key, type, "10.0.0.0/8", "00:02:01:00:00:01");
    create(bctxt, k_vpc_feeder);
}

void sample_vpc_setup_validate(pds_vpc_type_t type) {
    k_vpc_feeder.init(k_vpc_key, type, "10.0.0.0/8", "00:02:01:00:00:01");
    read(k_vpc_feeder);
}

void sample_vpc_teardown(pds_batch_ctxt_t bctxt, pds_vpc_type_t type) {
    k_vpc_feeder.init(k_vpc_key, type, "10.0.0.0/8", "00:02:01:00:00:01");
    del(bctxt, k_vpc_feeder);
}

void sample1_vpc_setup(pds_batch_ctxt_t bctxt, pds_vpc_type_t type) {
    // setup and teardown parameters should be in sync
    k_vpc_feeder.init(k_vpc_key, type, "10.0.0.0/8",
                      "00:02:01:00:00:01", PDS_MAX_VPC);
    many_create(bctxt, k_vpc_feeder);
}

void sample1_vpc_setup_validate(pds_vpc_type_t type) {
    k_vpc_feeder.init(k_vpc_key, type, "10.0.0.0/8",
                      "00:02:01:00:00:01", PDS_MAX_VPC);
    many_read(k_vpc_feeder);
}

void sample1_vpc_teardown(pds_batch_ctxt_t bctxt, pds_vpc_type_t type) {
    k_vpc_feeder.init(k_vpc_key, type, "10.0.0.0/8",
                      "00:02:01:00:00:01", PDS_MAX_VPC);
    many_delete(bctxt, k_vpc_feeder);
}

}    // namespace api
}    // namespace test
