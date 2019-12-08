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
                     pds_nexthop_id_t id, pds_nh_type_t type, uint16_t vlan,
                     pds_vpc_id_t vpc_id, pds_if_id_t if_id,
                     pds_tep_id_t tep_id) {
    this->spec.key.id = id;
    this->spec.type = type;
    this->num_obj = num_obj;
    if (type == PDS_NH_TYPE_IP) {
        test::extract_ip_addr(ip_str.c_str(), &this->spec.ip);
        this->spec.vlan = vlan;
        this->spec.vpc.id = vpc_id;
        MAC_UINT64_TO_ADDR(this->spec.mac, mac);
    } else if (type == PDS_NH_TYPE_UNDERLAY) {
        this->spec.l3_if.id = L3_IFINDEX(if_id);
        MAC_UINT64_TO_ADDR(this->spec.underlay_mac, mac);
    } else if (type == PDS_NH_TYPE_OVERLAY) {
        this->spec.tep.id = tep_id;
    }
}

void
nexthop_feeder::iter_next(int width) {
    spec.key.id += width;
    cur_iter_pos++;
    if (spec.type == PDS_NH_TYPE_IP) {
        spec.ip.addr.v4_addr += width;
        spec.vlan += width;
        test::increment_mac_addr(spec.mac, width);
    } else if (spec.type == PDS_NH_TYPE_UNDERLAY) {
        // spec.l3_if.id += width;
        test::increment_mac_addr(spec.underlay_mac, width);
    } else if (spec.type == PDS_NH_TYPE_OVERLAY) {
        spec.tep.id += width;
    }
}

void
nexthop_feeder::key_build(pds_nexthop_key_t *key) const {
    memset(key, 0, sizeof(pds_nexthop_key_t));
    key->id = this->spec.key.id;
}

void
nexthop_feeder::spec_build(pds_nexthop_spec_t *spec) const {
    memset(spec, 0, sizeof(pds_nexthop_spec_t));
    this->key_build(&spec->key);

    spec->type = this->spec.type;
    if (this->spec.type == PDS_NH_TYPE_IP) {
        spec->ip = this->spec.ip;
        spec->vpc.id = this->spec.vpc.id;
        spec->vlan = this->spec.vlan;
        MAC_UINT64_TO_ADDR(spec->mac, MAC_TO_UINT64(this->spec.mac));
    } else if (this->spec.type == PDS_NH_TYPE_UNDERLAY) {
        spec->l3_if.id = this->spec.l3_if.id;
        MAC_UINT64_TO_ADDR(spec->underlay_mac,
                           MAC_TO_UINT64(this->spec.underlay_mac));
    } else if (this->spec.type == PDS_NH_TYPE_OVERLAY) {
        spec->tep.id = this->spec.tep.id;
    }
}

bool
nexthop_feeder::key_compare(const pds_nexthop_key_t *key) const {
    return (this->spec.key.id == key->id);
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
        if (this->spec.vpc.id != spec->vpc.id)
            return false;
        // validate NH ip
        if (!IPADDR_EQ(&this->spec.ip, &spec->ip))
            return false;

    } else if (this->spec.type == PDS_NH_TYPE_UNDERLAY) {

        if (MAC_TO_UINT64(this->spec.underlay_mac) !=
                          MAC_TO_UINT64(spec->underlay_mac))
            return false;
        if (this->spec.l3_if.id != spec->l3_if.id)
            ; // TODO return false;

    } else if (this->spec.type == PDS_NH_TYPE_OVERLAY) {

        if (this->spec.tep.id != spec->tep.id)
            return false;

    }

    return true;
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
    k_unh_feeder.init("", 0x1, k_max_nh, 1, PDS_NH_TYPE_UNDERLAY);
    many_create(bctxt, k_unh_feeder);
}

void sample_underlay_nexthop_teardown(pds_batch_ctxt_t bctxt) {
    k_unh_feeder.init("", 0x1, k_max_nh, 1, PDS_NH_TYPE_UNDERLAY);
    many_delete(bctxt, k_unh_feeder);
}

void sample_nexthop_teardown(pds_batch_ctxt_t bctxt) {
    k_nexthop_feeder.init("30.30.30.1");
    many_delete(bctxt, k_nexthop_feeder);
}

}    // namespace api
}    // namespace test
