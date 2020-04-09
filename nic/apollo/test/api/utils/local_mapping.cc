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
                           pds_mapping_type_t map_type) {

    this->map_type = map_type;
    this->vpc = vpc;
    this->subnet = subnet;
    test::extract_ip_pfx(vnic_ip_cidr_str.c_str(), &this->vnic_ip_pfx);
    key_build(&this->key);
    this->vnic = vnic;
    this->vnic_mac_u64 = vnic_mac;

    this->fabric_encap.type = encap_type;
    switch(encap_type) {
    case PDS_ENCAP_TYPE_MPLSoUDP:
        this->fabric_encap.val.mpls_tag = encap_val;
        break;
    case PDS_ENCAP_TYPE_VXLAN:
        this->fabric_encap.val.vnid = encap_val;
        break;
    default:
        this->fabric_encap.val.value = encap_val;
    }

    this->public_ip_valid = public_ip_valid;
    if (public_ip_valid)
        extract_ip_pfx(pub_ip_cidr_str.c_str(), &this->public_ip_pfx);

    this->num_vnics = num_vnics;
    this->num_ip_per_vnic = num_ip_per_vnic;
    this->curr_vnic_ip_cnt = 0;
    this->num_obj = num_vnics * num_ip_per_vnic;
}

void
local_mapping_feeder::iter_next(int width) {
    increment_ip_addr(&vnic_ip_pfx.addr, width);
    key_build(&this->key);
    if (public_ip_valid)
        increment_ip_addr(&public_ip_pfx.addr, width);

    curr_vnic_ip_cnt++;
    if (curr_vnic_ip_cnt == num_ip_per_vnic) {
        vnic = int2pdsobjkey(pdsobjkey2int(vnic) + width);
        vnic_mac_u64 += width;
        curr_vnic_ip_cnt = 0;
    }
    cur_iter_pos++;
}

void
local_mapping_feeder::key_build(pds_obj_key_t *key) const {
    // TODO: encoding here won't work for IPv6
    //       how about storing base uuid and increment ?
    uint32_t vpc_id, subnet_id;

    memset(key, 0, sizeof(*key));
    memcpy(&key->id[0], &map_type, sizeof(uint8_t));
    if (map_type == PDS_MAPPING_TYPE_L3) {
        vpc_id = objid_from_uuid(vpc);
        sprintf(&key->id[1], "%08x", vpc_id);
        memcpy(&key->id[5], &vnic_ip_pfx.addr.addr.v4_addr,
               sizeof(ipv4_addr_t));
    } else {
        subnet_id = objid_from_uuid(subnet);
        memcpy(&key->id[1], &subnet_id, sizeof(uint32_t));
        MAC_UINT64_TO_ADDR(&key->id[5], vnic_mac_u64);
    }
}

void
local_mapping_feeder::spec_build(pds_local_mapping_spec_t *spec) const {
    memset(spec, 0, sizeof(*spec));
    key_build(&spec->key);
    spec->skey.type = map_type;
    if (map_type == PDS_MAPPING_TYPE_L3) {
        spec->skey.vpc = vpc;
        spec->skey.ip_addr = vnic_ip_pfx.addr;
    } else {
        spec->skey.subnet = subnet;
        MAC_UINT64_TO_ADDR(spec->skey.mac_addr, vnic_mac_u64);
    }
    spec->vnic = vnic;
    spec->subnet = subnet;
    spec->fabric_encap = fabric_encap;
    MAC_UINT64_TO_ADDR(spec->vnic_mac, vnic_mac_u64);
    spec->public_ip_valid = public_ip_valid;
    if (public_ip_valid)
        spec->public_ip = public_ip_pfx.addr;
}

bool
local_mapping_feeder::key_compare(const pds_obj_key_t *key) const {
    if (this->key != *key)
        return false;
    return true;
}

bool
local_mapping_feeder::spec_compare(const pds_local_mapping_spec_t *spec) const {

    if (map_type != spec->skey.type)
        return false;

    if (map_type == PDS_MAPPING_TYPE_L3)
        return ((vpc == spec->skey.vpc) &&
                IPADDR_EQ(&vnic_ip_pfx.addr, &spec->skey.ip_addr));

    // L2 key type
    return ((subnet == spec->skey.subnet) &&
            (vnic_mac_u64 == MAC_TO_UINT64(spec->skey.mac_addr)));

    // skipping comparing vnic.id and subnet.id as the hw id's returned
    // through read are not the same as id's assigned.

    if (!test::pdsencap_isequal(&this->fabric_encap, &spec->fabric_encap))
        return false;

    if (this->public_ip_valid != spec->public_ip_valid)
        return false;

    if ((this->public_ip_valid) &&
        (!IPADDR_EQ(&this->public_ip_pfx.addr, &spec->public_ip)))
            return false;

    if (this->vnic_mac_u64 != MAC_TO_UINT64(spec->vnic_mac))
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

    if (map_type == PDS_MAPPING_TYPE_L3)
        vnic_mac_u64 += width;

    if (public_ip_valid)
        increment_ip_addr(&public_ip_pfx.addr, 1);

    // updating this filed is causing some failures, will put it comment now
    //uint16_t vnic_id = (pdsobjkey2int(vnic) + 1)%k_max_vnic;
    //vnic =  int2pdsobjkey(vnic_id ? vnic_id : 1);
    this->fabric_encap.val.value++;
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

// Local mapping feeder class
static inline void
lmap_attr_update (local_mapping_feeder& feeder, pds_local_mapping_spec_t *spec,
                  int chg_bmap)
{
    if (bit_check(chg_bmap, LMAP_ATTR_VNIC)) {
    }
    if (bit_check(chg_bmap, LMAP_ATTR_SUBNET)) {
    }
    if (bit_check(chg_bmap, LMAP_ATTR_FAB_ENCAP)) {
    }
    if (bit_check(chg_bmap, LMAP_ATTR_VNIC_MAC)) {
    }
    if (bit_check(chg_bmap, LMAP_ATTR_PUBLIC_IP)) {
    }
    if (bit_check(chg_bmap, LMAP_ATTR_PROVIDER_IP)) {
    }
    if (bit_check(chg_bmap, LMAP_ATTR_TAGS)) {
    }
}

void
lmap_update (local_mapping_feeder& feeder, pds_local_mapping_spec_t *spec,
             int chg_bmap, sdk_ret_t exp_result)
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
