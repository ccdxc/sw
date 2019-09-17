//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/apollo/api/encap_utils.hpp"
#include "nic/apollo/test/utils/nexthop.hpp"
#include "nic/apollo/test/utils/utils.hpp"

namespace api_test {

//----------------------------------------------------------------------------
// NEXTHOP feeder class routines
//----------------------------------------------------------------------------

void
nexthop_feeder::init(std::string ip_str, uint64_t mac, uint32_t num_obj,
                     pds_nexthop_id_t id, pds_nh_type_t type, uint16_t vlan,
                     pds_vpc_id_t vpc_id) {
    this->id = id;
    this->type = type;
    extract_ip_addr(ip_str.c_str(), &this->ip);
    this->mac = mac;
    this->vlan = vlan;
    this->vpc_id = vpc_id;
    this->num_obj = num_obj;
}

void
nexthop_feeder::iter_next(int width) {
    ip.addr.v4_addr += width;
    mac += width;
    vlan += width;
    id += width;
    cur_iter_pos++;
}

void
nexthop_feeder::key_build(pds_nexthop_key_t *key) const {
    memset(key, 0, sizeof(pds_nexthop_key_t));
    key->id = this->id;
}

void
nexthop_feeder::spec_build(pds_nexthop_spec_t *spec) const {
    memset(spec, 0, sizeof(pds_nexthop_spec_t));
    this->key_build(&spec->key);

    spec->type = this->type;
    spec->vpc.id = this->vpc_id;
    spec->ip = this->ip;
    spec->vlan = this->vlan;
    MAC_UINT64_TO_ADDR(spec->mac, this->mac);
}

bool
nexthop_feeder::key_compare(const pds_nexthop_key_t *key) const {
    return (this->id == key->id);
}

bool
nexthop_feeder::spec_compare(const pds_nexthop_spec_t *spec) const {
    ip_addr_t nh_ip = this->ip, spec_nh_ip = spec->ip;

    // validate NH type
    if (this->type != spec->type)
        return sdk::SDK_RET_ERR;

    // nothing much to check for blackhole nh
    if (this->type == PDS_NH_TYPE_BLACKHOLE)
        return sdk::SDK_RET_OK;

    // validate NH vlan
    if (this->vlan != spec->vlan)
        return sdk::SDK_RET_ERR;

    // validate NH MAC
    if (this->mac != MAC_TO_UINT64(spec->mac))
        return sdk::SDK_RET_ERR;

    if (this->type != PDS_NH_TYPE_IP) {
        // validate NH vpc
        if (this->vpc_id != spec->vpc.id)
            return sdk::SDK_RET_ERR;

        // validate NH ip
        if (!IPADDR_EQ(&nh_ip, &spec_nh_ip))
            return sdk::SDK_RET_ERR;
    }

    return true;
}

//----------------------------------------------------------------------------
// Misc routines
//----------------------------------------------------------------------------

// do not modify these sample values as rest of system is sync with these
static nexthop_feeder k_nexthop_feeder;

void sample_nexthop_setup(void) {
    // setup and teardown parameters should be in sync
    k_nexthop_feeder.init("30.30.30.1");
    many_create(k_nexthop_feeder);
}

void sample_nexthop_teardown(void) {
    k_nexthop_feeder.init("30.30.30.1");
    many_delete(k_nexthop_feeder);
}

}    // namespace api_test
