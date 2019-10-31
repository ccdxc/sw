//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/apollo/api/encap_utils.hpp"
#include "nic/apollo/api/include/pds_if.hpp"
#include "nic/apollo/test/utils/nexthop.hpp"
#include "nic/apollo/test/utils/utils.hpp"

namespace api_test {

const pds_nh_type_t k_nh_type = apulu() ? PDS_NH_TYPE_UNDERLAY : PDS_NH_TYPE_IP;

//----------------------------------------------------------------------------
// NEXTHOP feeder class routines
//----------------------------------------------------------------------------

void
nexthop_feeder::init(std::string ip_str, uint64_t mac, uint32_t num_obj,
                     pds_nexthop_id_t id, pds_nh_type_t type, uint16_t vlan,
                     pds_vpc_id_t vpc_id, pds_if_id_t if_id) {
    this->id = id;
    this->type = type;
    this->num_obj = num_obj;
    this->mac = mac;
    if (type == PDS_NH_TYPE_IP) {
        extract_ip_addr(ip_str.c_str(), &this->ip);
        this->vlan = vlan;
        this->vpc_id = vpc_id;
    } else if (type == PDS_NH_TYPE_UNDERLAY) {
        this->if_id = if_id;
    }
}

void
nexthop_feeder::iter_next(int width) {
    id += width;
    mac += width;
    cur_iter_pos++;
    if (type == PDS_NH_TYPE_UNDERLAY) {
        // if_id += width;
    } else if (type == PDS_NH_TYPE_IP) {
        ip.addr.v4_addr += width;
        vlan += width;
    }
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
    if (this->type == PDS_NH_TYPE_UNDERLAY) {
        spec->l3_if.id = this->if_id;
        printf(" NH id %u l3 if id %u mac 0x%x\n", spec->key.id, spec->l3_if.id, this->mac);
        MAC_UINT64_TO_ADDR(spec->underlay_mac, this->mac);
    } else if (this->type == PDS_NH_TYPE_IP) {
        MAC_UINT64_TO_ADDR(spec->mac, this->mac);
        spec->ip = this->ip;
        spec->vpc.id = this->vpc_id;
        spec->vlan = this->vlan;
    }
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

void sample_if_setup(pds_batch_ctxt_t bctxt) {
    pds_if_spec_t spec;
    pds_encap_val_t val;

    spec.key.id = 1;
    spec.type = PDS_IF_TYPE_L3;
    spec.admin_state = PDS_IF_STATE_UP;
    spec.l3_if_info.vpc.id = 1;
    spec.l3_if_info.port_num = 1;
    spec.l3_if_info.encap.type = PDS_ENCAP_TYPE_VXLAN;
    val.vlan_tag = 1;
    val.qinq_tag.c_tag = 1;
    val.qinq_tag.s_tag = 1;
    val.vnid = 1;
    val.mpls_tag = 1;
    val.value = 1;
    memcpy(&val, &spec.l3_if_info.encap.val, sizeof(pds_encap_val_t));
    MAC_UINT64_TO_ADDR(spec.l3_if_info.mac_addr, 0x010203040506);
    extract_ip_pfx("1.2.3.4", &spec.l3_if_info.ip_prefix);

    pds_if_create(&spec, bctxt);
}

void sample_if_teardown(pds_batch_ctxt_t bctxt) {
    pds_if_key_t key;

    key.id = 1;
    pds_if_delete(&key, bctxt);
}

void sample_nexthop_setup(pds_batch_ctxt_t bctxt) {
    // setup and teardown parameters should be in sync
    k_nexthop_feeder.init("30.30.30.1");
    many_create(bctxt, k_nexthop_feeder);
}

void sample_nexthop_teardown(pds_batch_ctxt_t bctxt) {
    k_nexthop_feeder.init("30.30.30.1");
    many_delete(bctxt, k_nexthop_feeder);
}

}    // namespace api_test
