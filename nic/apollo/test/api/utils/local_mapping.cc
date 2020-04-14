//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/api/include/pds_if.hpp"
#include "nic/apollo/test/api/utils/batch.hpp"
#include "nic/apollo/test/api/utils/local_mapping.hpp"
#include "nic/apollo/test/api/utils/vnic.hpp"

namespace test {
namespace api {

//----------------------------------------------------------------------------
// Local Mapping feeder class routines
//----------------------------------------------------------------------------

void
local_mapping_feeder::init(pds_obj_key_t vpc, pds_obj_key_t subnet,
                           std::string vnic_ip_cidr_str, uint64_t vnic_mac,
                           pds_encap_type_t encap_type, uint32_t encap_val,
                           pds_obj_key_t vnic, bool public_ip_valid,
                           std::string pub_ip_cidr_str, uint32_t num_vnics,
                           uint32_t num_ip_per_vnic,
                           pds_mapping_type_t map_type,
                           int num_tags) {
    ip_prefix_t vnic_ip_pfx;
    ip_prefix_t public_ip_pfx;

    test::extract_ip_pfx(vnic_ip_cidr_str.c_str(), &vnic_ip_pfx);
    test::extract_ip_pfx(pub_ip_cidr_str.c_str(), &public_ip_pfx);

    memset(&spec, 0, sizeof(pds_local_mapping_spec_t));
    spec.skey.type = map_type;
    if (map_type == PDS_MAPPING_TYPE_L3) {
        spec.skey.vpc = vpc;
        spec.skey.ip_addr = vnic_ip_pfx.addr;
    } else {
        spec.skey.subnet = subnet;
        MAC_UINT64_TO_ADDR(spec.skey.mac_addr, vnic_mac);
    }
    spec.vnic = vnic;
    spec.subnet = subnet;
    MAC_UINT64_TO_ADDR(spec.vnic_mac, vnic_mac);
    spec.public_ip_valid = public_ip_valid;
    if (public_ip_valid)
        spec.public_ip = public_ip_pfx.addr;

    // enable after tag support
#if 0
    spec.num_tags = num_tags;
    for (int i=0; i<num_tags; i++) {
        spec.tags[i] = i+1;
    }
#endif

    this->spec.fabric_encap.type = encap_type;
    switch(encap_type) {
    case PDS_ENCAP_TYPE_MPLSoUDP:
        this->spec.fabric_encap.val.mpls_tag = encap_val;
        break;
    case PDS_ENCAP_TYPE_VXLAN:
        this->spec.fabric_encap.val.vnid = encap_val;
        break;
    default:
        this->spec.fabric_encap.val.value = encap_val;
    }
    key_build(&spec.key);

    this->num_vnics = num_vnics;
    this->num_ip_per_vnic = num_ip_per_vnic;
    this->curr_vnic_ip_cnt = 0;
    this->num_obj = num_vnics * num_ip_per_vnic;
}

local_mapping_feeder::local_mapping_feeder(const local_mapping_feeder& feeder) {
    memcpy(&this->spec, &feeder.spec, sizeof(pds_local_mapping_spec_t));
    num_obj = feeder.num_obj;
    num_vnics = feeder.num_vnics;
    num_ip_per_vnic = feeder.num_ip_per_vnic;
    curr_vnic_ip_cnt = feeder.curr_vnic_ip_cnt;
}

void
local_mapping_feeder::iter_next(int width) {
    uint64_t mac;
    if (spec.skey.type == PDS_MAPPING_TYPE_L3) {
        increment_ip_addr(&spec.skey.ip_addr, width);
    }
    key_build(&this->spec.key);
    if (spec.public_ip_valid)
        increment_ip_addr(&spec.public_ip, width);

    mac = MAC_TO_UINT64(spec.skey.mac_addr);
    mac += width;
    MAC_UINT64_TO_ADDR(spec.vnic_mac, mac);

    curr_vnic_ip_cnt++;
    if (curr_vnic_ip_cnt == num_ip_per_vnic) {
        spec.vnic = int2pdsobjkey(pdsobjkey2int(spec.vnic) + width);
        if (spec.skey.type != PDS_MAPPING_TYPE_L3) {
            MAC_UINT64_TO_ADDR(spec.skey.mac_addr, mac);
        }
        curr_vnic_ip_cnt = 0;
    }
    cur_iter_pos++;
}

void
local_mapping_feeder::key_build(pds_obj_key_t *key) const {
    // TODO: encoding here won't work for IPv6
    //       how about storing base uuid and increment ?
    uint32_t vpc_id, subnet_id;
    uint64_t mac;

    memset(key, 0, sizeof(*key));
    memcpy(&key->id[0], &spec.skey.type, sizeof(uint8_t));
    if (spec.skey.type == PDS_MAPPING_TYPE_L3) {
        vpc_id = objid_from_uuid(spec.skey.vpc);
        sprintf(&key->id[1], "%08x", vpc_id);
        memcpy(&key->id[5], &spec.skey.ip_addr.addr.v4_addr,
               sizeof(ipv4_addr_t));
    } else {
        subnet_id = objid_from_uuid(spec.subnet);
        memcpy(&key->id[1], &subnet_id, sizeof(uint32_t));
        mac = MAC_TO_UINT64(spec.skey.mac_addr);
        MAC_UINT64_TO_ADDR(&key->id[5], mac);
    }
}

void
local_mapping_feeder::spec_build(pds_local_mapping_spec_t *spec) const {
    memcpy(spec, &this->spec, sizeof(pds_local_mapping_spec_t));
}

bool
local_mapping_feeder::key_compare(const pds_obj_key_t *key) const {
    if (this->spec.key != *key)
        return false;
    return true;
}

bool
local_mapping_feeder::spec_compare(const pds_local_mapping_spec_t *spec) const {

    if (this->spec.skey.type != spec->skey.type)
        return false;

    if (this->spec.skey.type == PDS_MAPPING_TYPE_L3)
        return ((this->spec.skey.vpc == spec->skey.vpc) &&
                IPADDR_EQ(&this->spec.skey.ip_addr, &spec->skey.ip_addr));

    // L2 key type
    return ((this->spec.skey.subnet == spec->skey.subnet) &&
            (MAC_TO_UINT64(this->spec.skey.mac_addr) ==
            MAC_TO_UINT64(spec->skey.mac_addr)));

    // skipping comparing vnic.id and subnet.id as the hw id's returned
    // through read are not the same as id's assigned.

    if (!test::pdsencap_isequal(&this->spec.fabric_encap, &spec->fabric_encap))
        return false;

    if (this->spec.public_ip_valid != spec->public_ip_valid)
        return false;

    if ((this->spec.public_ip_valid) &&
        (!IPADDR_EQ(&this->spec.public_ip, &spec->public_ip)))
            return false;

    if (MAC_TO_UINT64(this->spec.vnic_mac) != MAC_TO_UINT64(spec->vnic_mac))
        return false;

    return true;
}

bool
local_mapping_feeder::status_compare(const pds_mapping_status_t *status1,
                                     const pds_mapping_status_t *status2) const {
    return true;
}

void
local_mapping_feeder::update_spec(uint32_t width) {
    // if key is L3, increment MAC
    // TODO: if key if L2 there is no non-key spec field to update.
    //       Revisit when L2 support is added.

    uint64_t mac;
    if (spec.skey.type == PDS_MAPPING_TYPE_L3) {
        mac = MAC_TO_UINT64(this->spec.vnic_mac);
        mac = mac + width;
        MAC_UINT64_TO_ADDR(this->spec.vnic_mac, mac);
    }
    if (spec.public_ip_valid)
        increment_ip_addr(&spec.public_ip, 1);

    // updating this filed is causing some failures, will put it comment now
    //uint16_t vnic_id = (pdsobjkey2int(vnic) + 1)%k_max_vnic;
    //vnic =  int2pdsobjkey(vnic_id ? vnic_id : 1);
    this->spec.fabric_encap.val.value++;
}

//----------------------------------------------------------------------------
// Local mapping CRUD helper routines
//----------------------------------------------------------------------------

void
lmap_create (local_mapping_feeder& feeder)
{
    pds_batch_ctxt_t bctxt = batch_start();

    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_create<local_mapping_feeder>(bctxt, feeder)));
    batch_commit(bctxt);
}

void
lmap_read (local_mapping_feeder& feeder, sdk_ret_t exp_result)
{
    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_read<local_mapping_feeder>(feeder, exp_result)));
}

static void
lmap_attr_update (local_mapping_feeder& feeder, pds_local_mapping_spec_t *spec,
                  uint64_t chg_bmap)
{
    if (bit_isset(chg_bmap, LMAP_ATTR_VNIC)) {
       feeder.spec.vnic = spec->vnic;
    }
    if (bit_isset(chg_bmap, LMAP_ATTR_SUBNET)) {
        feeder.spec.subnet = spec->subnet;
    }
    if (bit_isset(chg_bmap, LMAP_ATTR_FAB_ENCAP)) {
        feeder.spec.fabric_encap = spec->fabric_encap;
    }
    if (bit_isset(chg_bmap, LMAP_ATTR_VNIC_MAC)) {
        memcpy(&feeder.spec.vnic_mac, &spec->vnic_mac, sizeof(spec->vnic_mac));
    }
    if (bit_isset(chg_bmap, LMAP_ATTR_PUBLIC_IP)) {
        feeder.spec.public_ip = spec->public_ip;
        feeder.spec.public_ip_valid = spec->public_ip_valid;
    }
    if (bit_isset(chg_bmap, LMAP_ATTR_PROVIDER_IP)) {
        feeder.spec.provider_ip = spec->provider_ip;
        feeder.spec.provider_ip_valid = spec->provider_ip_valid;
    }
    if (bit_isset(chg_bmap, LMAP_ATTR_TAGS)) {
        feeder.spec.num_tags = spec->num_tags;
        memcpy(&feeder.spec.tags, spec->tags,
               sizeof(spec->tags[0]) * spec->num_tags);
    }
}

void
lmap_update (local_mapping_feeder& feeder, pds_local_mapping_spec_t *spec,
             uint64_t chg_bmap, sdk_ret_t exp_result)
{
    pds_batch_ctxt_t bctxt = batch_start();

    lmap_attr_update(feeder, spec, chg_bmap);
    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_update<local_mapping_feeder>(bctxt, feeder)));

    // if expected result is err, batch commit should fail
    if (exp_result == SDK_RET_ERR)
        batch_commit_fail(bctxt);
    else
        batch_commit(bctxt);
}

void
lmap_delete (local_mapping_feeder& feeder)
{
    pds_batch_ctxt_t bctxt = batch_start();

    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_delete<local_mapping_feeder>(bctxt, feeder)));
    batch_commit(bctxt);
}

}    // namespace api
}    // namespace test
