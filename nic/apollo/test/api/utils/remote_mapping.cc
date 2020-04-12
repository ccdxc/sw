//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/api/utils.hpp"
#include "nic/apollo/api/include/pds_if.hpp"
#include "nic/apollo/test/api/utils/batch.hpp"
#include "nic/apollo/test/api/utils/remote_mapping.hpp"

namespace test {
namespace api {

//----------------------------------------------------------------------------
// Remote Mapping feeder class routines
//----------------------------------------------------------------------------

void
remote_mapping_feeder::init(pds_obj_key_t vpc, pds_obj_key_t subnet,
                            std::string vnic_ip_cidr_str, uint64_t vnic_mac,
                            pds_encap_type_t encap_type, uint32_t encap_val,
                            pds_nh_type_t nh_type, uint32_t nh_id,
                            uint32_t num_teps, uint32_t num_vnic_per_tep,
                            pds_mapping_type_t map_type) {

    ip_prefix_t vnic_ip_pfx;

    test::extract_ip_pfx(vnic_ip_cidr_str.c_str(), &vnic_ip_pfx);

    memset(&spec, 0, sizeof(spec));
    spec.skey.type = map_type;
    if (map_type == PDS_MAPPING_TYPE_L3) {
        spec.skey.vpc = vpc;
        spec.skey.ip_addr = vnic_ip_pfx.addr;
    } else {
        spec.skey.subnet = subnet;
        MAC_UINT64_TO_ADDR(spec.skey.mac_addr, vnic_mac);
    }
    spec.subnet = subnet;
    MAC_UINT64_TO_ADDR(spec.vnic_mac, vnic_mac);
    spec.nh_type = nh_type;
    if (nh_type == PDS_NH_TYPE_OVERLAY)
        spec.tep = int2pdsobjkey(nh_id);
    else
        spec.nh_group = int2pdsobjkey(nh_id);
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


    this->num_teps = num_teps;
    this->num_vnic_per_tep = num_vnic_per_tep;
    this->curr_tep_vnic_cnt = 0;
    this->num_obj = num_teps * num_vnic_per_tep;
}

remote_mapping_feeder::remote_mapping_feeder(const remote_mapping_feeder& feeder) {
    memcpy(&this->spec, &feeder.spec, sizeof(pds_remote_mapping_spec_t));
    num_obj = feeder.num_obj;
    num_vnic_per_tep = feeder.num_vnic_per_tep;
    num_teps = feeder.num_teps;
    curr_tep_vnic_cnt = feeder.curr_tep_vnic_cnt;
}

void
remote_mapping_feeder::iter_next(int width) {
    uint64_t mac;

    mac = MAC_TO_UINT64(spec.vnic_mac);
    mac += width;
    MAC_UINT64_TO_ADDR(spec.vnic_mac, mac);
    if (spec.skey.type == PDS_MAPPING_TYPE_L3) {
        test::increment_ip_addr(&spec.skey.ip_addr, width);
    } else {
        MAC_UINT64_TO_ADDR(spec.skey.mac_addr, mac)
    }
    key_build(&spec.key);
    //increment_encap(&fabric_encap, width);
    curr_tep_vnic_cnt++;

    if (curr_tep_vnic_cnt == num_vnic_per_tep) {
        spec.tep = int2pdsobjkey(pdsobjkey2int(spec.tep) + width);;
        curr_tep_vnic_cnt = 0;
    }
    cur_iter_pos++;
}

void
remote_mapping_feeder::key_build(pds_obj_key_t *key) const {
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
remote_mapping_feeder::spec_build(pds_remote_mapping_spec_t *spec) const {
    memcpy(spec, &this->spec, sizeof(pds_remote_mapping_spec_t));
}

bool
remote_mapping_feeder::key_compare(const pds_obj_key_t *key) const {
    if (this->spec.key != *key)
        return false;
    return true;
}

bool
remote_mapping_feeder::spec_compare(const pds_remote_mapping_spec_t *spec) const {
    if (this->spec.skey.type != spec->skey.type)
        return false;

    if (this->spec.skey.type == PDS_MAPPING_TYPE_L3)
        return ((this->spec.skey.vpc == spec->skey.vpc) &&
                IPADDR_EQ(&this->spec.skey.ip_addr, &spec->skey.ip_addr));

    // L2 key type
    return ((this->spec.skey.subnet == spec->skey.subnet) &&
            (MAC_TO_UINT64(this->spec.skey.mac_addr) ==
             MAC_TO_UINT64(spec->skey.mac_addr)));

    // skipping comparing subnet id and tep/nh_group hw id since the
    // hw id's retured via read are different from those configured.

    if (!test::pdsencap_isequal(&this->spec.fabric_encap, &spec->fabric_encap))
        return false;

    if (this->spec.nh_type != spec->nh_type)
        return false;

    if (MAC_TO_UINT64(this->spec.vnic_mac) != MAC_TO_UINT64(spec->vnic_mac))
        return false;

    return true;
}

bool
remote_mapping_feeder::status_compare(
    const pds_mapping_status_t *status1,
    const pds_mapping_status_t *status2) const {
    return true;
}

void
remote_mapping_feeder::update_spec(uint32_t width) {
    // next hop/TEP id is a good candidate to create update feeder
    uint64_t nh_id = pdsobjkey2int(spec.tep);
    nh_id += width;
    if (nh_id > PDS_MAX_TEP)
        nh_id -= PDS_MAX_TEP;
    spec.tep = int2pdsobjkey(nh_id);
}

//----------------------------------------------------------------------------
// Remote mapping CRUD helper routines
//----------------------------------------------------------------------------

void
rmap_create (remote_mapping_feeder& feeder)
{
    pds_batch_ctxt_t bctxt = batch_start();

    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_create<remote_mapping_feeder>(bctxt, feeder)));
    batch_commit(bctxt);
}

void
rmap_read (remote_mapping_feeder& feeder, sdk_ret_t exp_result)
{
    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_read<remote_mapping_feeder>(feeder, exp_result)));
}

static void
rmap_attr_update (remote_mapping_feeder& feeder,
                  pds_remote_mapping_spec_t *spec, uint64_t chg_bmap)
{
    if (bit_isset(chg_bmap, RMAP_ATTR_SUBNET)) {
    }
    if (bit_isset(chg_bmap, RMAP_ATTR_FAB_ENCAP)) {
    }
    if (bit_isset(chg_bmap, RMAP_ATTR_NH_TYPE)) {
    }
    if (bit_isset(chg_bmap, RMAP_ATTR_VNIC_MAC)) {
    }
    if (bit_isset(chg_bmap, RMAP_ATTR_TAGS)) {
    }
    if (bit_isset(chg_bmap, RMAP_ATTR_PROVIDER_IP)) {
    }
}

void
rmap_update (remote_mapping_feeder& feeder, pds_remote_mapping_spec_t *spec,
             uint64_t chg_bmap, sdk_ret_t exp_result)
{
    pds_batch_ctxt_t bctxt = batch_start();

    rmap_attr_update(feeder, spec, chg_bmap);
    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_update<remote_mapping_feeder>(bctxt, feeder)));

    // if expected result is err, batch commit should fail
    if (exp_result == SDK_RET_ERR)
        batch_commit_fail(bctxt);
    else
        batch_commit(bctxt);
}

void
rmap_delete (remote_mapping_feeder& feeder)
{
    pds_batch_ctxt_t bctxt = batch_start();

    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_delete<remote_mapping_feeder>(bctxt, feeder)));
    batch_commit(bctxt);
}

}    // namespace api
}    // namespace test
