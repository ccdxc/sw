//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/apollo/api/include/pds_if.hpp"
#include "nic/apollo/test/api/utils/nexthop.hpp"

namespace test {
namespace api {

// globals
const pds_nh_type_t k_nh_type = apulu() ? PDS_NH_TYPE_UNDERLAY : PDS_NH_TYPE_IP;
const uint32_t k_max_nh = PDS_MAX_NEXTHOP;

//----------------------------------------------------------------------------
// NEXTHOP feeder class routines
//----------------------------------------------------------------------------

void
nexthop_feeder::init(std::string ip_str, uint64_t mac, uint32_t num_obj,
                     pds_obj_key_t key, pds_nh_type_t type, uint16_t vlan,
                     pds_obj_key_t vpc, pds_obj_key_t l3_intf,
                     pds_obj_key_t tep) {
    this->spec.key = key;
    this->spec.type = type;
    this->num_obj = num_obj;
    if (type == PDS_NH_TYPE_IP) {
        test::extract_ip_addr(ip_str.c_str(), &this->spec.ip);
        this->spec.vlan = vlan;
        this->spec.vpc = vpc;
        MAC_UINT64_TO_ADDR(this->spec.mac, mac);
    } else if (type == PDS_NH_TYPE_UNDERLAY) {
        this->spec.l3_if = l3_intf;
        MAC_UINT64_TO_ADDR(this->spec.underlay_mac, mac);
    } else if (type == PDS_NH_TYPE_OVERLAY) {
        this->spec.tep = tep;
    }
}

void
nexthop_feeder::iter_next(int width) {
    spec.key = int2pdsobjkey(pdsobjkey2int(spec.key) + width);
    cur_iter_pos++;
    if (spec.type == PDS_NH_TYPE_IP) {
        spec.ip.addr.v4_addr += width;
        spec.vlan += width;
        test::increment_mac_addr(spec.mac, width);
    } else if (spec.type == PDS_NH_TYPE_UNDERLAY) {
        // spec.l3_if.id += width;
        test::increment_mac_addr(spec.underlay_mac, width);
    } else if (spec.type == PDS_NH_TYPE_OVERLAY) {
        spec.tep = int2pdsobjkey(pdsobjkey2int(spec.tep) + width);
    }
}

void
nexthop_feeder::key_build(pds_obj_key_t *key) const {
    memset(key, 0, sizeof(pds_obj_key_t));
    *key = this->spec.key;
}

void
nexthop_feeder::spec_build(pds_nexthop_spec_t *spec) const {
    memset(spec, 0, sizeof(pds_nexthop_spec_t));
    this->key_build(&spec->key);

    spec->type = this->spec.type;
    if (this->spec.type == PDS_NH_TYPE_IP) {
        spec->ip = this->spec.ip;
        spec->vpc = this->spec.vpc;
        spec->vlan = this->spec.vlan;
        MAC_UINT64_TO_ADDR(spec->mac, MAC_TO_UINT64(this->spec.mac));
    } else if (this->spec.type == PDS_NH_TYPE_UNDERLAY) {
        spec->l3_if = this->spec.l3_if;
        MAC_UINT64_TO_ADDR(spec->underlay_mac,
                           MAC_TO_UINT64(this->spec.underlay_mac));
    } else if (this->spec.type == PDS_NH_TYPE_OVERLAY) {
        spec->tep = this->spec.tep;
    }
}

bool
nexthop_feeder::key_compare(const pds_obj_key_t *key) const {
    return (this->spec.key == *key);
}

bool
nexthop_feeder::spec_compare(const pds_nexthop_spec_t *spec) const {

    // nothing much to check for blackhole nh
    if (this->spec.type == PDS_NH_TYPE_BLACKHOLE)
        return true;

    // validate NH type
    if (this->spec.type != spec->type)
        return false;

    if (this->spec.type == PDS_NH_TYPE_IP) {
        // validate NH vlan
        if (this->spec.vlan != spec->vlan)
            return false;
        // validate NH MAC
        if (MAC_TO_UINT64(this->spec.mac) != MAC_TO_UINT64(spec->mac))
            return false;
        // validate NH vpc
        if (this->spec.vpc != spec->vpc)
            return false;
        // validate NH ip
        if (!IPADDR_EQ(&this->spec.ip, &spec->ip))
            return false;

    } else if (this->spec.type == PDS_NH_TYPE_UNDERLAY) {
        if (MAC_TO_UINT64(this->spec.underlay_mac) !=
                          MAC_TO_UINT64(spec->underlay_mac))
            return false;
        if (this->spec.l3_if != spec->l3_if)
            ; // TODO return false;

    } else if (this->spec.type == PDS_NH_TYPE_OVERLAY) {

        if (this->spec.tep != spec->tep)
            return false;

    }

    return true;
}

bool
nexthop_feeder::status_compare(const pds_nexthop_status_t *status1,
                               const pds_nexthop_status_t *status2) const {
    return true;
}

//----------------------------------------------------------------------------
// NEXTHOP CRUD helper routines
//----------------------------------------------------------------------------

void
nexthop_create (nexthop_feeder& feeder)
{
    pds_batch_ctxt_t bctxt = batch_start();

    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_create<nexthop_feeder>(bctxt, feeder)));
    batch_commit(bctxt);
}

void
nexthop_read (nexthop_feeder& feeder, sdk_ret_t exp_result)
{
    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_read<nexthop_feeder>(feeder, exp_result)));
}

static void
nexthop_attr_update (nexthop_feeder& feeder, pds_nexthop_spec_t *spec,
                     uint64_t chg_bmap)
{
    if (bit_isset(chg_bmap, NEXTHOP_ATTR_TYPE)) {
        feeder.spec.type = spec->type;
    }
    if (bit_isset(chg_bmap, NEXTHOP_ATTR_NH_INFO_OVERLAY)) {
        feeder.spec.tep = spec->tep;
    }
    if (bit_isset(chg_bmap, NEXTHOP_ATTR_NH_INFO_UNDERLAY)) {
        feeder.spec.l3_if = spec->l3_if;
        memcpy(&feeder.spec.underlay_mac, &spec->underlay_mac,
               sizeof(spec->underlay_mac));
    }
    if (bit_isset(chg_bmap, NEXTHOP_ATTR_NH_INFO_NH_IP)) {
        feeder.spec.vlan = spec->vlan;
        feeder.spec.vpc = spec->vpc;
        feeder.spec.ip = spec->ip;
    }
}

void
nexthop_update (nexthop_feeder& feeder, pds_nexthop_spec_t *spec,
                uint64_t chg_bmap, sdk_ret_t exp_result)
{
    pds_batch_ctxt_t bctxt = batch_start();

    nexthop_attr_update(feeder, spec, chg_bmap);
    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_update<nexthop_feeder>(bctxt, feeder)));

    // if expected result is err, batch commit should fail
    if (exp_result == SDK_RET_ERR)
        batch_commit_fail(bctxt);
    else
        batch_commit(bctxt);
}

void
nexthop_delete (nexthop_feeder& feeder)
{
    pds_batch_ctxt_t bctxt = batch_start();

    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_delete<nexthop_feeder>(bctxt, feeder)));
    batch_commit(bctxt);
}

//----------------------------------------------------------------------------
// Misc routines
//----------------------------------------------------------------------------

// do not modify these sample values as rest of system is sync with these
static nexthop_feeder k_nexthop_feeder, k_unh_feeder;

void sample_nexthop_setup(pds_batch_ctxt_t bctxt) {
    // setup and teardown parameters should be in sync
    k_nexthop_feeder.init("30.30.30.1");
    many_create(bctxt, k_nexthop_feeder);
}

void sample_underlay_nexthop_setup(pds_batch_ctxt_t bctxt) {
    k_unh_feeder.init("", 0x1, k_max_nh, int2pdsobjkey(1),
                      PDS_NH_TYPE_UNDERLAY);
    many_create(bctxt, k_unh_feeder);
}

void sample_underlay_nexthop_teardown(pds_batch_ctxt_t bctxt) {
    k_unh_feeder.init("", 0x1, k_max_nh, int2pdsobjkey(1),
                      PDS_NH_TYPE_UNDERLAY);
    many_delete(bctxt, k_unh_feeder);
}

void sample_nexthop_teardown(pds_batch_ctxt_t bctxt) {
    k_nexthop_feeder.init("30.30.30.1");
    many_delete(bctxt, k_nexthop_feeder);
}

}    // namespace api
}    // namespace test
