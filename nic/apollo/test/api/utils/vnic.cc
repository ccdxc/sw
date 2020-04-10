//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/apollo/api/utils.hpp"
#include "nic/apollo/test/api/utils/batch.hpp"
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

static void
increment_num (uint8_t *num)
{
    (*num)++;
    if (*num > PDS_MAX_VNIC_POLICY) {
        *num = 0;
    }
}

static void
fill_policy_ids (pds_obj_key_t *pol_arr,
                 uint32_t start_key, uint8_t num_policy)
{
    for (int i = 0; i < num_policy; i++) {
        pol_arr[i] = int2pdsobjkey(start_key++);
    }
}

void
vnic_feeder::init(pds_obj_key_t key, pds_obj_key_t subnet,
                  uint32_t num_vnic, uint64_t mac,
                  pds_encap_type_t vnic_encap_type,
                  pds_encap_type_t fabric_encap_type,
                  bool binding_checks_en, bool configure_policy,
                  uint8_t tx_mirror_session_bmap,
                  uint8_t rx_mirror_session_bmap) {
    static uint8_t num_policy = 0;
    uint64_t mac_64;
    //static uint32_t lif_id = HOST_LIF_ID_MIN;
    num_obj = num_vnic;
    memset(&spec, 0, sizeof(pds_vnic_spec_t));
    spec.key = key;
    spec.subnet = subnet;
    vnic_feeder_encap_init(pdsobjkey2int(key), vnic_encap_type,
                           &spec.vnic_encap);
    vnic_feeder_encap_init(pdsobjkey2int(key), fabric_encap_type,
                           &spec.fabric_encap);

    mac_64 = mac | (pdsobjkey2int(key) << 24);
    MAC_UINT64_TO_ADDR(spec.mac_addr, mac_64);
    spec.binding_checks_en = binding_checks_en;
    spec.tx_mirror_session_bmap = tx_mirror_session_bmap;
    spec.rx_mirror_session_bmap = rx_mirror_session_bmap;

#if 0
    // TODO: this should be under platform == HW check
    spec->host_ifindex = LIF_IFINDEX(lif_id++);
    if (lif_id > HOST_LIF_ID_MAX) {
        lif_id = HOST_LIF_ID_MIN;
    }
#endif

    if (configure_policy) {
        increment_num(&num_policy);
        spec.num_ing_v4_policy = num_policy;
        fill_policy_ids(spec.ing_v4_policy, TEST_POLICY_ID_BASE + 1, num_policy);
        increment_num(&num_policy);
        spec.num_ing_v6_policy = num_policy;
        fill_policy_ids(spec.ing_v6_policy, TEST_POLICY_ID_BASE + 5, num_policy);
        increment_num(&num_policy);
        spec.num_egr_v4_policy = num_policy;
        fill_policy_ids(spec.egr_v4_policy, TEST_POLICY_ID_BASE + 10, num_policy);
        increment_num(&num_policy);
        spec.num_egr_v6_policy = num_policy;
        fill_policy_ids(spec.egr_v6_policy, TEST_POLICY_ID_BASE + 15, num_policy);
    }
}

vnic_feeder::vnic_feeder(const vnic_feeder& feeder) {
    memcpy(&this->spec, &feeder.spec, sizeof(pds_vnic_spec_t));
    num_obj = feeder.num_obj;
}

void
vnic_feeder::iter_next(int width) {
    uint64_t mac_u64;
    spec.key = int2pdsobjkey(pdsobjkey2int(spec.key) + width);
    vnic_feeder_encap_next(&spec.vnic_encap);
    if (apollo()) {
        vnic_feeder_encap_next(&spec.fabric_encap);
        spec.tx_mirror_session_bmap += width;
        spec.rx_mirror_session_bmap += width;
    }
    mac_u64 = MAC_TO_UINT64(spec.mac_addr);
    mac_u64+= width;

    MAC_UINT64_TO_ADDR(spec.mac_addr, mac_u64);
    cur_iter_pos++;
}

void
vnic_feeder::key_build(pds_obj_key_t *key) const {
    *key = this->spec.key;
}

void
vnic_feeder::spec_build(pds_vnic_spec_t *spec) const {
    memcpy(spec, &this->spec, sizeof(pds_vnic_spec_t));
}

bool
vnic_feeder::key_compare(const pds_obj_key_t *key) const {
    if (this->spec.key != *key)
        return false;
    return true;
}

bool
vnic_feeder::spec_compare(const pds_vnic_spec_t *spec) const {
    mac_addr_t mac = {0};

    if (!test::pdsencap_isequal(&this->spec.vnic_encap, &spec->vnic_encap))
        return false;

    if (apollo()) {
        if (!test::pdsencap_isequal(&this->spec.fabric_encap, &spec->fabric_encap))
            return false;

        if (this->spec.binding_checks_en != spec->binding_checks_en)
            return false;

        if (this->spec.tx_mirror_session_bmap != spec->tx_mirror_session_bmap)
            return false;

        if (this->spec.rx_mirror_session_bmap != spec->rx_mirror_session_bmap)
            return false;
    }

    if (memcmp(&this->spec.mac_addr, &spec->mac_addr, sizeof(mac)))
        return false;

    return true;
}

bool
vnic_feeder::status_compare(const pds_vnic_status_t *status1,
                            const pds_vnic_status_t *status2) const {
    return true;
}

//----------------------------------------------------------------------------
// VNIC CRUD helper routines
//----------------------------------------------------------------------------

void
vnic_create (vnic_feeder& feeder)
{
    pds_batch_ctxt_t bctxt = batch_start();

    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_create<vnic_feeder>(bctxt, feeder)));
    batch_commit(bctxt);
}

void
vnic_read (vnic_feeder& feeder, sdk_ret_t exp_result)
{
    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_read<vnic_feeder>(feeder, exp_result)));
}

static void
vnic_attr_update (vnic_feeder& feeder, pds_vnic_spec_t *spec, uint64_t chg_bmap)
{
    if (bit_isset(chg_bmap, VNIC_ATTR_HOSTNAME)) {
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_SUBNET)) {
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_VNIC_ENCAP)) {
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_FAB_ENCAP)) {
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_MAC_ADDR)) {
    }
    if (bit_isset(chg_bmap, VNIC__ATTR_BINDING_CHECKS_EN)) {
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_TX_MIRROR)) {
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_RX_MIRROR)) {
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_V4_METER)) {
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_V6_METER)) {
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_SWITCH_VNIC)) {
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_V4_INGPOL)) {
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_V6_INGPOL)) {
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_V4_EGRPOL)) {
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_V6_EGRPOL)) {
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_HOST_IF)) {
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_TX_POLICER)) {
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_RX_POLICER)) {
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_PRIMARY)) {
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_MAX_SESSIONS)) {
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_FLOW_LEARN_EN)) {
    }
}

void
vnic_update (vnic_feeder& feeder, pds_vnic_spec_t *spec,
             uint64_t chg_bmap, sdk_ret_t exp_result)
{
    pds_batch_ctxt_t bctxt = batch_start();

    vnic_attr_update(feeder, spec, chg_bmap);
    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_update<vnic_feeder>(bctxt, feeder)));

    // if expected result is err, batch commit should fail
    if (exp_result == SDK_RET_ERR)
        batch_commit_fail(bctxt);
    else
        batch_commit(bctxt);
}

void
vnic_delete (vnic_feeder& feeder)
{
    pds_batch_ctxt_t bctxt = batch_start();

    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_delete<vnic_feeder>(bctxt, feeder)));
    batch_commit(bctxt);
}

//----------------------------------------------------------------------------
// Misc routines
//----------------------------------------------------------------------------


static vnic_feeder k_vnic_feeder;

void sample_vnic_setup(pds_batch_ctxt_t bctxt) {
    // setup and teardown parameters should be in sync
    k_vnic_feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), k_max_vnic,
                       k_feeder_mac, PDS_ENCAP_TYPE_DOT1Q,
                       PDS_ENCAP_TYPE_MPLSoUDP, true, true, 0, 0);
    many_create(bctxt, k_vnic_feeder);
}

void sample_vnic_teardown(pds_batch_ctxt_t bctxt) {
    // setup and teardown parameters should be in sync
    k_vnic_feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), k_max_vnic,
                       k_feeder_mac, PDS_ENCAP_TYPE_DOT1Q,
                       PDS_ENCAP_TYPE_MPLSoUDP, true, true, 0, 0);
    many_delete(bctxt, k_vnic_feeder);
}

}    // namespace api
}    // namespace test
