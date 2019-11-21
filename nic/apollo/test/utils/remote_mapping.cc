//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
//#include "nic/apollo/api/encap_utils.hpp"
#include "nic/apollo/api/include/pds_if.hpp"
#include "nic/apollo/api/utils.hpp"
#include "nic/apollo/test/utils/remote_mapping.hpp"
#include "nic/apollo/test/utils/utils.hpp"

namespace api_test {

//----------------------------------------------------------------------------
// Remote Mapping feeder class routines
//----------------------------------------------------------------------------

void
remote_mapping_feeder::init(pds_vpc_id_t vpc_id, pds_subnet_id_t subnet_id,
                           std::string vnic_ip_cidr_str, uint64_t vnic_mac,
                           pds_encap_type_t encap_type, uint32_t encap_val,
                           pds_nh_type_t nh_type, uint32_t nh_id,
                           uint32_t num_teps, uint32_t num_vnic_per_tep,
                           pds_mapping_type_t map_type) {

    this->map_type = map_type;
    this->vpc.id = vpc_id;
    this->subnet.id = subnet_id;
    extract_ip_pfx(vnic_ip_cidr_str.c_str(), &this->vnic_ip_pfx);
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
    increment_ip_addr(&vnic_ip_pfx.addr, width);
    //increment_encap(&fabric_encap, width);
    curr_tep_vnic_cnt++;

    if (curr_tep_vnic_cnt == num_vnic_per_tep) {
        nh_id += width;
        curr_tep_vnic_cnt = 0;
    }
    cur_iter_pos++;
}

void
remote_mapping_feeder::key_build(pds_mapping_key_t *key) const {
    key->type = map_type;
    if (map_type == PDS_MAPPING_TYPE_L3) {
        key->vpc = vpc;
        key->ip_addr = vnic_ip_pfx.addr;
    } else {
        key->subnet = subnet;
        MAC_UINT64_TO_ADDR(key->mac_addr, vnic_mac_u64);
    }
}

void
remote_mapping_feeder::spec_build(pds_remote_mapping_spec_t *spec) const {
    memset(spec, 0, sizeof(*spec));
    key_build(&spec->key);
    spec->subnet = subnet;
    spec->fabric_encap = fabric_encap;
    MAC_UINT64_TO_ADDR(spec->vnic_mac, vnic_mac_u64);
    spec->nh_type = nh_type;
    if (nh_type == PDS_NH_TYPE_OVERLAY)
        spec->tep.id = nh_id;
    else
        spec->nh_group.id = nh_id;
}

bool
remote_mapping_feeder::key_compare(const pds_mapping_key_t *key) const {

    if (map_type != key->type)
        return false;

    if (map_type == PDS_MAPPING_TYPE_L3)
        return ((vpc.id == key->vpc.id) &&
                IPADDR_EQ(&vnic_ip_pfx.addr, &key->ip_addr));

    // L2 key type
    return ((subnet.id == key->subnet.id) &&
            (vnic_mac_u64 == MAC_TO_UINT64(key->mac_addr)));

}

bool
remote_mapping_feeder::spec_compare(const pds_remote_mapping_spec_t *spec) const {

    // skipping comparing subnet id and tep/nh_group hw id since the
    // hw id's retured via read are different from those configured.

    if (!pdsencap_isequal(&this->fabric_encap, &spec->fabric_encap))
        return false;

    if (this->nh_type != spec->nh_type)
        return false;

    if (this->vnic_mac_u64 != MAC_TO_UINT64(spec->vnic_mac))
        return false;

    return true;
}

void
remote_mapping_feeder::update_spec(uint32_t width) {
    // next hop/TEP id is a good candidate to create update feeder
    nh_id += width;
    if (nh_id > PDS_MAX_TEP)
        nh_id -= PDS_MAX_TEP;
}

}   // namespace api_test
