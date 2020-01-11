//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/apollo/test/api/utils/nexthop.hpp"
#include "nic/apollo/test/api/utils/nexthop_group.hpp"

namespace test {
namespace api {

//----------------------------------------------------------------------------
// NEXTHOP group feeder class routines
//----------------------------------------------------------------------------

static constexpr uint64_t k_mac_addr = 0x010203040506;

static void
nh_spec_fill(pds_nexthop_spec_t *spec, pds_nexthop_key_t key,
             pds_nh_type_t type, mac_addr_t mac)
{
    if (spec == NULL) assert(0);
    spec->key = key;
    spec->type = type;
    switch (type) {
    case PDS_NH_TYPE_IP:
        // TODO
        break;
    case PDS_NH_TYPE_UNDERLAY:
        spec->l3_if.id = k_l3_if_id;
        memcpy(spec->underlay_mac, mac, ETH_ADDR_LEN);
        break;
    case PDS_NH_TYPE_OVERLAY:
        spec->tep = int2pdsobjkey(pdsobjkey2int(key) + 1);
        break;
    default:
        assert(0);
    }
}

static void
nh_groups_fill(pds_nexthop_group_spec_t *spec)
{
    pds_nh_type_t type = PDS_NH_TYPE_NONE;
    mac_addr_t mac;

    MAC_UINT64_TO_ADDR(mac, k_mac_addr);
    if (spec->type == PDS_NHGROUP_TYPE_UNDERLAY_ECMP) {
        type = PDS_NH_TYPE_UNDERLAY;
    }
    else if (spec->type == PDS_NHGROUP_TYPE_OVERLAY_ECMP) {
        type = PDS_NH_TYPE_OVERLAY;
    }
    for (int i = 0; i < spec->num_nexthops; i++) {
        nh_spec_fill(&spec->nexthops[i], int2pdsobjkey(i+1), type, mac);
    }
}

void
nexthop_group_feeder::init(pds_nexthop_group_type_t type,
                           uint8_t num_nexthops,
                           pds_nexthop_group_key_t key,
                           uint32_t num_objs) {
    memset(&spec, 0, sizeof(pds_nexthop_group_spec_t));
    spec.type = type;
    spec.key = key;
    spec.num_nexthops = num_nexthops;
    nh_groups_fill(&spec);
    num_obj = num_objs;
}

nexthop_group_feeder::nexthop_group_feeder(const nexthop_group_feeder& feeder) {
    memcpy(&this->spec, &feeder.spec, sizeof(pds_nexthop_group_spec_t));
    num_obj = feeder.num_obj;
}

void
nexthop_group_feeder::iter_next(int width) {
    spec.key = int2pdsobjkey(pdsobjkey2int(spec.key) + width);
    cur_iter_pos++;
}

void
nexthop_group_feeder::key_build(pds_nexthop_group_key_t *key) const {
    *key = spec.key;
}

void
nexthop_group_feeder::spec_build(pds_nexthop_group_spec_t *nhg_spec) const {
    memcpy(nhg_spec, &this->spec, sizeof(pds_nexthop_group_spec_t));
}

bool
nexthop_group_feeder::key_compare(const pds_nexthop_group_key_t *key) const {
    return (spec.key == *key);
}

bool nh_spec_compare(pds_nexthop_spec_t spec1, pds_nexthop_spec_t spec2)
{
    if (spec1.type != spec2.type)
        return false;

    switch (spec1.type) {
    case PDS_NH_TYPE_UNDERLAY:
        if (spec1.l3_if.id != spec2.l3_if.id)
            ; //return false;
        if (MAC_TO_UINT64(spec1.underlay_mac) !=
                                    MAC_TO_UINT64(spec2.underlay_mac))
            return false;
        break;
    case PDS_NH_TYPE_OVERLAY:
        if (spec1.tep != spec2.tep)
            return false;
        break;
    default:
        assert(0);
        break;
    }

    return true;
}

bool nh_specs_compare(pds_nexthop_group_spec_t *spec1,
                      pds_nexthop_group_spec_t *spec2)
{
    if (spec1->type == PDS_NHGROUP_TYPE_OVERLAY_ECMP) {
        // tep keys not read from hw for overlay ecmp
        return true;
    }
    for (uint8_t i = 0; i < spec1->num_nexthops; i++) {
        if (nh_spec_compare(spec1->nexthops[i], spec2->nexthops[i]) != true) {
            return false;
        }
    }
    return true;
}

bool
nexthop_group_feeder::spec_compare(const pds_nexthop_group_spec_t *spec) const {
    if (this->spec.type != spec->type) {
        return false;
    }

    if (this->spec.num_nexthops != spec->num_nexthops) {
        return false;
    }

    if (nh_specs_compare((pds_nexthop_group_spec_t*)&this->spec,
                         (pds_nexthop_group_spec_t*)spec) != true) {
        return false;
    }

    return true;
}

//----------------------------------------------------------------------------
// Misc routines
//----------------------------------------------------------------------------

// do not modify these sample values as rest of system is sync with these
static nexthop_group_feeder k_nh_group_feeder;

void sample_nexthop_group_setup(pds_batch_ctxt_t bctxt) {
    // setup and teardown parameters should be in sync
    k_nh_group_feeder.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, PDS_MAX_ECMP_NEXTHOP);
    many_create(bctxt, k_nh_group_feeder);
}

void sample_nexthop_group_teardown(pds_batch_ctxt_t bctxt) {
    // setup and teardown parameters should be in sync
    k_nh_group_feeder.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, PDS_MAX_ECMP_NEXTHOP);
    many_delete(bctxt, k_nh_group_feeder);
}

}    // namespace api
}    // namespace test
