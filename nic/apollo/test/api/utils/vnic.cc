//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/apollo/api/utils.hpp"
#include "nic/apollo/test/api/utils/vnic.hpp"

#define HOST_LIF_ID_MIN 71
#define HOST_LIF_ID_MAX 78

namespace test {
namespace api {

const uint64_t k_feeder_mac = 0x101000000000;
// artemis - one is reserved, hence max is MAX_VNIC - 1
const uint32_t k_max_vnic = ::apollo() ? 64 : PDS_MAX_VNIC - 1;

static inline void
vnic_feeder_encap_init (uint32_t id, pds_encap_type_t encap_type,
                        pds_encap_t *encap)
{
    encap->type = encap_type;
    // update encap value to seed base
    switch (encap_type) {
    case PDS_ENCAP_TYPE_DOT1Q:
        encap->val.vlan_tag = id;
        break;
    case PDS_ENCAP_TYPE_QINQ:
        encap->val.qinq_tag.c_tag = id;
        encap->val.qinq_tag.s_tag = id + 4096;
        break;
    case PDS_ENCAP_TYPE_MPLSoUDP:
        encap->val.mpls_tag = id;
        break;
    case PDS_ENCAP_TYPE_VXLAN:
        encap->val.vnid = id;
        break;
    default:
        encap->val.value = id;
        break;
    }
}


static inline void
vnic_feeder_encap_next (pds_encap_t *encap, int width = 1)
{
    switch (encap->type) {
    case PDS_ENCAP_TYPE_DOT1Q:
        encap->val.vlan_tag += width;
        break;
    case PDS_ENCAP_TYPE_QINQ:
        encap->val.qinq_tag.c_tag += width;
        encap->val.qinq_tag.s_tag += width;
        break;
    case PDS_ENCAP_TYPE_MPLSoUDP:
        encap->val.mpls_tag += width;
        break;
    case PDS_ENCAP_TYPE_VXLAN:
        encap->val.vnid += width;
        break;
    default:
        encap->val.value += width;
        break;
    }
}

//----------------------------------------------------------------------------
// VNIC feeder class routines
//----------------------------------------------------------------------------

void
vnic_feeder::init(pds_obj_key_t key, uint32_t num_vnic, uint64_t mac,
                  pds_encap_type_t vnic_encap_type,
                  pds_encap_type_t fabric_encap_type,
                  bool binding_checks_en, bool configure_policy) {
    this->key = key;
    this->vpc = int2pdsobjkey(1);
    this->subnet = int2pdsobjkey(1);
    this->mac_u64 = mac | (pdsobjkey2int(key) << 24);
    vnic_feeder_encap_init(pdsobjkey2int(key), vnic_encap_type, &vnic_encap);
    vnic_feeder_encap_init(pdsobjkey2int(key), fabric_encap_type,
                           &fabric_encap);
    this->binding_checks_en = binding_checks_en;
    this->tx_mirror_session_bmap = 0;
    this->rx_mirror_session_bmap = 0;
    this->configure_policy = configure_policy;
    num_obj = num_vnic;
}

void
vnic_feeder::iter_next(int width) {
    key = int2pdsobjkey(pdsobjkey2int(key) + width);
    vnic_feeder_encap_next(&vnic_encap);
    if (apollo()) {
        vnic_feeder_encap_next(&fabric_encap);
        tx_mirror_session_bmap += width;
        rx_mirror_session_bmap += width;
    }
    mac_u64 += width;
    cur_iter_pos++;
}

void
vnic_feeder::key_build(pds_obj_key_t *key) const {
    *key = this->key;
}

static void
fill_policy_ids (pds_obj_key_t *pol_arr,
                 uint32_t start_key, uint8_t num_policy)
{
    for (int i = 0; i < num_policy; i++) {
        pol_arr[i] = int2pdsobjkey(start_key++);
    }
}

static void
increment_num (uint8_t *num)
{
    (*num)++;
    if (*num > PDS_MAX_VNIC_POLICY) {
        *num = 0;
    }
}

void
vnic_feeder::spec_build(pds_vnic_spec_t *spec) const {
    static uint8_t num_policy = 0;
    //static uint32_t lif_id = HOST_LIF_ID_MIN;

    memset(spec, 0, sizeof(pds_vnic_spec_t));
    this->key_build(&spec->key);

    spec->subnet = subnet;
    spec->vnic_encap = vnic_encap;
    spec->fabric_encap = fabric_encap;
    MAC_UINT64_TO_ADDR(spec->mac_addr, mac_u64);
    spec->binding_checks_en = binding_checks_en;
    spec->tx_mirror_session_bmap = tx_mirror_session_bmap;
    spec->rx_mirror_session_bmap = rx_mirror_session_bmap;

#if 0
    // TODO: this should be under platform == HW check
    spec->host_ifindex = LIF_IFINDEX(lif_id++);
    if (lif_id > HOST_LIF_ID_MAX) {
        lif_id = HOST_LIF_ID_MIN;
    }
#endif

    if (this->configure_policy) {
        increment_num(&num_policy);
        spec->num_ing_v4_policy = num_policy;
        fill_policy_ids(spec->ing_v4_policy, TEST_POLICY_ID_BASE + 1, num_policy);
        increment_num(&num_policy);
        spec->num_ing_v6_policy = num_policy;
        fill_policy_ids(spec->ing_v6_policy, TEST_POLICY_ID_BASE + 5, num_policy);
        increment_num(&num_policy);
        spec->num_egr_v4_policy = num_policy;
        fill_policy_ids(spec->egr_v4_policy, TEST_POLICY_ID_BASE + 10, num_policy);
        increment_num(&num_policy);
        spec->num_egr_v6_policy = num_policy;
        fill_policy_ids(spec->egr_v6_policy, TEST_POLICY_ID_BASE + 15, num_policy);
    }
}

bool
vnic_feeder::key_compare(const pds_obj_key_t *key) const {
    if (this->key != *key)
        return false;
    return true;
}

bool
vnic_feeder::spec_compare(const pds_vnic_spec_t *spec) const {
    mac_addr_t mac = {0};

    if (!test::pdsencap_isequal(&vnic_encap, &spec->vnic_encap))
        return false;

    if (apollo()) {
        if (!test::pdsencap_isequal(&fabric_encap, &spec->fabric_encap))
            return false;

        if (binding_checks_en != spec->binding_checks_en)
            return false;

        if (tx_mirror_session_bmap != spec->tx_mirror_session_bmap)
            return false;

        if (rx_mirror_session_bmap != spec->rx_mirror_session_bmap)
            return false;
    }

    MAC_UINT64_TO_ADDR(mac, mac_u64);
    if (memcmp(mac, &spec->mac_addr, sizeof(mac)))
        return false;

    return true;
}

bool
vnic_feeder::status_compare(const pds_vnic_status_t *status1,
                            const pds_vnic_status_t *status2) const {
    return true;
}

static vnic_feeder k_vnic_feeder;

void sample_vnic_setup(pds_batch_ctxt_t bctxt) {
    // setup and teardown parameters should be in sync
    k_vnic_feeder.init(int2pdsobjkey(1));
    many_create(bctxt, k_vnic_feeder);
}

void sample_vnic_teardown(pds_batch_ctxt_t bctxt) {
    // setup and teardown parameters should be in sync
    k_vnic_feeder.init(int2pdsobjkey(1));
    many_delete(bctxt, k_vnic_feeder);
}

}    // namespace api
}    // namespace test
