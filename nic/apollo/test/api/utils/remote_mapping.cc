//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/api/include/pds_if.hpp"
#include "nic/apollo/api/utils.hpp"
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

    this->map_type = map_type;
    this->vpc = vpc;
    this->subnet = subnet;
    extract_ip_pfx(vnic_ip_cidr_str.c_str(), &this->vnic_ip_pfx);
    key_build(&this->key);
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
    this->nh_type = nh_type;
    this->nh_id = nh_id;

    this->num_teps = num_teps;
    this->num_vnic_per_tep = num_vnic_per_tep;
    this->curr_tep_vnic_cnt = 0;
    this->num_obj = num_teps * num_vnic_per_tep;
}

void
remote_mapping_feeder::iter_next(int width) {
    vnic_mac_u64 += width;
    test::increment_ip_addr(&vnic_ip_pfx.addr, width);
    key_build(&this->key);
    //increment_encap(&fabric_encap, width);
    curr_tep_vnic_cnt++;

    if (curr_tep_vnic_cnt == num_vnic_per_tep) {
        nh_id += width;
        curr_tep_vnic_cnt = 0;
    }
    cur_iter_pos++;
}

void
remote_mapping_feeder::key_build(pds_obj_key_t *key) const {
    // TODO: can we encode vpc, IP or subnet, MAC here ?
    //       won't work with IPv6
    //       how about storing based uuid and increment ?
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
remote_mapping_feeder::spec_build(pds_remote_mapping_spec_t *spec) const {
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
    spec->subnet = subnet;
    spec->fabric_encap = fabric_encap;
    MAC_UINT64_TO_ADDR(spec->vnic_mac, vnic_mac_u64);
    spec->nh_type = nh_type;
    if (nh_type == PDS_NH_TYPE_OVERLAY)
        spec->tep = int2pdsobjkey(nh_id);
    else
        spec->nh_group = int2pdsobjkey(nh_id);
}

bool
remote_mapping_feeder::key_compare(const pds_obj_key_t *key) const {
    if (this->key != *key)
        return false;
    return true;
}

bool
remote_mapping_feeder::spec_compare(const pds_remote_mapping_spec_t *spec) const {
    if (map_type != spec->skey.type)
        return false;

    if (map_type == PDS_MAPPING_TYPE_L3)
        return ((vpc == spec->skey.vpc) &&
                IPADDR_EQ(&vnic_ip_pfx.addr, &spec->skey.ip_addr));

    // L2 key type
    return ((subnet == spec->skey.subnet) &&
            (vnic_mac_u64 == MAC_TO_UINT64(spec->skey.mac_addr)));

    // skipping comparing subnet id and tep/nh_group hw id since the
    // hw id's retured via read are different from those configured.

    if (!test::pdsencap_isequal(&this->fabric_encap, &spec->fabric_encap))
        return false;

    if (this->nh_type != spec->nh_type)
        return false;

    if (this->vnic_mac_u64 != MAC_TO_UINT64(spec->vnic_mac))
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
    nh_id += width;
    if (nh_id > PDS_MAX_TEP)
        nh_id -= PDS_MAX_TEP;
}

}    // namespace api
}    // namespace test
