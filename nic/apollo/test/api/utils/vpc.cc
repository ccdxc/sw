//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/test/api/utils/batch.hpp"
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
    ip_prefix_t pfx;

    memset(&spec, 0, sizeof(pds_vpc_spec_t));
    spec.key = key;
    spec.type = type;
    mac_str_to_addr((char *)vr_mac.c_str(), spec.vr_mac);
    spec.fabric_encap.type = PDS_ENCAP_TYPE_VXLAN;
    spec.fabric_encap.val.value = pdsobjkey2int(key) + 9999;
    SDK_ASSERT(str2ipv4pfx((char *)cidr_str.c_str(), &pfx) == 0);
    spec.v4_prefix.len = pfx.len;
    spec.v4_prefix.v4_addr = pfx.addr.addr.v4_addr;
    num_obj = num_vpc;
}

void
vpc_feeder::iter_next(int width) {
    ip_addr_t ipaddr = {0};
    ip_prefix_t pfx = {0};

    SDK_ASSERT(str2ipv4pfx((char *)ipv4pfx2str(&spec.v4_prefix), &pfx) == 0);
    ip_prefix_ip_next(&pfx, &ipaddr);
    memcpy(&pfx.addr, &ipaddr, sizeof(ip_addr_t));
    spec.key = int2pdsobjkey(pdsobjkey2int(spec.key) + width);
    spec.v4_prefix.len = pfx.len;
    spec.v4_prefix.v4_addr = pfx.addr.addr.v4_addr;
    if (artemis() || apulu())
        spec.fabric_encap.val.value += width;
    cur_iter_pos++;
}

void
vpc_feeder::key_build(pds_obj_key_t *key) const {
    memcpy(key , &spec.key, sizeof(pds_obj_key_t));
}

void
vpc_feeder::spec_build(pds_vpc_spec_t *vpc_spec) const {
    memcpy(vpc_spec, &spec, sizeof(pds_vpc_spec_t));
}

bool
vpc_feeder::key_compare(const pds_obj_key_t *key) const {
    return (memcmp(key, &this->spec.key, sizeof(pds_obj_key_t)) == 0);
}

bool
vpc_feeder::spec_compare(const pds_vpc_spec_t *spec) const {
    if (spec->type != this->spec.type)
        return false;

    if (memcmp(&spec->fabric_encap, &this->spec.fabric_encap, sizeof(pds_encap_t))) {
        return false;
    }

    if (apulu()) {
        if (!this->spec.vr_mac) {
            if (memcmp(&spec->vr_mac, this->spec.vr_mac, sizeof(mac_addr_t))) {
                return false;
            }
        }
    }
    return true;
}

bool
vpc_feeder::status_compare(const pds_vpc_status_t *status1,
                           const pds_vpc_status_t *status2) const {
    return true;
}

//----------------------------------------------------------------------------
// VPC CRUD helper routines
//----------------------------------------------------------------------------

void
vpc_create (vpc_feeder& feeder)
{
    pds_batch_ctxt_t bctxt = batch_start();

    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_create<vpc_feeder>(bctxt, feeder)));
    batch_commit(bctxt);
}

void
vpc_read (vpc_feeder& feeder, sdk_ret_t exp_result)
{
    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_read<vpc_feeder>(feeder, exp_result)));
}

static void
vpc_attr_update (vpc_feeder& feeder, pds_vpc_spec_t *spec, uint64_t chg_bmap)
{
    if (bit_isset(chg_bmap, VPC_ATTR_TYPE)) {
        feeder.spec.type = spec->type;
    }
    if (bit_isset(chg_bmap, VPC_ATTR_V4_PREFIX)) {
        feeder.spec.v4_prefix = spec->v4_prefix;
    }
    if (bit_isset(chg_bmap, VPC_ATTR_V6_PREFIX)) {
        feeder.spec.v6_prefix = spec->v6_prefix;
    }
    if (bit_isset(chg_bmap, VPC_ATTR_VR_MAC)) {
        memcpy(&feeder.spec.vr_mac, &spec->vr_mac, sizeof(spec->vr_mac));
    }
    if (bit_isset(chg_bmap, VPC_ATTR_FAB_ENCAP)) {
        feeder.spec.fabric_encap = spec->fabric_encap;
    }
    if (bit_isset(chg_bmap, VPC_ATTR_V4_RTTBL)) {
        feeder.spec.v4_route_table = spec->v4_route_table;
    }
    if (bit_isset(chg_bmap, VPC_ATTR_V6_RTTBL)) {
        feeder.spec.v6_route_table = spec->v6_route_table;
    }
    if (bit_isset(chg_bmap, VPC_ATTR_NAT46_PREFIX)) {
        feeder.spec.nat46_prefix = spec->nat46_prefix;
    }
    if (bit_isset(chg_bmap, VPC_ATTR_TOS)) {
        feeder.spec.tos = spec->tos;
    }
}

void
vpc_update (vpc_feeder& feeder, pds_vpc_spec_t *spec,
               uint64_t chg_bmap, sdk_ret_t exp_result)
{
    pds_batch_ctxt_t bctxt = batch_start();

    vpc_attr_update(feeder, spec, chg_bmap);
    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_update<vpc_feeder>(bctxt, feeder)));

    // if expected result is err, batch commit should fail
    if (exp_result == SDK_RET_ERR)
        batch_commit_fail(bctxt);
    else
        batch_commit(bctxt);
}

void
vpc_delete (vpc_feeder& feeder)
{
    pds_batch_ctxt_t bctxt = batch_start();

    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_delete<vpc_feeder>(bctxt, feeder)));
    batch_commit(bctxt);
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
