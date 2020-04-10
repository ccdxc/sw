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

void
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


void
vnic_feeder_encap_next (pds_encap_t *encap, int width)
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

void
fill_policy_ids (pds_obj_key_t *pol_arr,
                 uint32_t start_key, uint8_t num_policy)
{
    for (int i = 0; i < num_policy; i++) {
        pol_arr[i] = int2pdsobjkey(start_key++);
    }
}

void
vnic_spec_policy_fill (pds_vnic_spec_t *spec, uint8_t num_policies,
                       uint8_t start_policy_index)
{
    uint8_t policy_start = TEST_POLICY_ID_BASE + start_policy_index;

    spec->num_ing_v4_policy = num_policies;
    spec->num_ing_v6_policy = num_policies;
    spec->num_egr_v4_policy = num_policies;
    spec->num_egr_v6_policy = num_policies;
    for (uint8_t i = 0; i < num_policies; i++) {
        spec->ing_v4_policy[i] = int2pdsobjkey(policy_start + i + 1);
        spec->ing_v6_policy[i] = int2pdsobjkey(policy_start + i + 6);
        spec->egr_v4_policy[i] = int2pdsobjkey(policy_start + i + 11);
        spec->egr_v6_policy[i] = int2pdsobjkey(policy_start + i + 16);
    }
}

void
vnic_feeder::init(pds_obj_key_t key, pds_obj_key_t subnet,
                  uint32_t num_vnic, uint64_t mac,
                  pds_encap_type_t vnic_encap_type,
                  pds_encap_type_t fabric_encap_type,
                  bool binding_checks_en, bool configure_policy,
                  uint8_t tx_mirror_session_bmap,
                  uint8_t rx_mirror_session_bmap,
                  uint8_t num_policies_per_vnic, uint8_t start_policy_index) {
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
        spec.num_ing_v4_policy = num_policies_per_vnic;
        fill_policy_ids(spec.ing_v4_policy,
                        TEST_POLICY_ID_BASE + start_policy_index + 1,
                        num_policies_per_vnic);
        spec.num_ing_v6_policy = num_policies_per_vnic;
        fill_policy_ids(spec.ing_v6_policy,
                        TEST_POLICY_ID_BASE + start_policy_index + 6,
                        num_policies_per_vnic);
        spec.num_egr_v4_policy = num_policies_per_vnic;
        fill_policy_ids(spec.egr_v4_policy,
                        TEST_POLICY_ID_BASE + start_policy_index + 11,
                        num_policies_per_vnic);
        spec.num_egr_v6_policy = num_policies_per_vnic;
        fill_policy_ids(spec.egr_v6_policy,
                        TEST_POLICY_ID_BASE + start_policy_index + 16,
                        num_policies_per_vnic);
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
       strcpy(feeder.spec.hostname, spec->hostname);
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_SUBNET)) {
        feeder.spec.subnet = spec->subnet;
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_VNIC_ENCAP)) {
        feeder.spec.vnic_encap = spec->vnic_encap;
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_FAB_ENCAP)) {
        feeder.spec.fabric_encap = spec->fabric_encap;
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_MAC_ADDR)) {
        memcpy(&feeder.spec.mac_addr, &spec->mac_addr, sizeof(spec->mac_addr));
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_BINDING_CHECKS_EN)) {
        feeder.spec.binding_checks_en = spec->binding_checks_en;
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_TX_MIRROR)) {
        feeder.spec.tx_mirror_session_bmap = spec->tx_mirror_session_bmap;
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_RX_MIRROR)) {
        feeder.spec.rx_mirror_session_bmap = spec->rx_mirror_session_bmap;
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_V4_METER)) {
        feeder.spec.v4_meter = spec->v4_meter;
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_V6_METER)) {
        feeder.spec.v6_meter = spec->v6_meter;
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_SWITCH_VNIC)) {
        feeder.spec.switch_vnic = spec->switch_vnic;
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_V4_INGPOL)) {
        feeder.spec.num_ing_v4_policy = spec->num_ing_v4_policy;
        memcpy(&feeder.spec.ing_v4_policy, &spec->ing_v4_policy,
               sizeof(spec->ing_v4_policy[0]) * spec->num_ing_v4_policy);
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_V6_INGPOL)) {
        feeder.spec.num_ing_v6_policy = spec->num_ing_v6_policy;
        memcpy(&feeder.spec.ing_v6_policy, &spec->ing_v6_policy,
               sizeof(spec->ing_v6_policy[0]) * spec->num_ing_v6_policy);
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_V4_EGRPOL)) {
        feeder.spec.num_egr_v4_policy = spec->num_egr_v4_policy;
        memcpy(&feeder.spec.egr_v4_policy, &spec->egr_v4_policy,
               sizeof(spec->egr_v4_policy[0]) * spec->num_egr_v4_policy);
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_V6_EGRPOL)) {
        feeder.spec.num_egr_v6_policy = spec->num_egr_v6_policy;
        memcpy(&feeder.spec.egr_v6_policy, &spec->egr_v6_policy,
               sizeof(spec->egr_v6_policy[0]) * spec->num_egr_v6_policy);
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_HOST_IF)) {
        feeder.spec.host_if = spec->host_if;
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_TX_POLICER)) {
        feeder.spec.tx_policer = spec->tx_policer;
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_RX_POLICER)) {
        feeder.spec.rx_policer = spec->rx_policer;
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_PRIMARY)) {
        feeder.spec.primary = spec->primary;
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_MAX_SESSIONS)) {
        feeder.spec.max_sessions = spec->max_sessions;
    }
    if (bit_isset(chg_bmap, VNIC_ATTR_FLOW_LEARN_EN)) {
        feeder.spec.flow_learn_en = spec->flow_learn_en;
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
                       PDS_ENCAP_TYPE_MPLSoUDP, true, true, 0, 0, 5, 0);
    many_create(bctxt, k_vnic_feeder);
}

void sample_vnic_teardown(pds_batch_ctxt_t bctxt) {
    // setup and teardown parameters should be in sync
    k_vnic_feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), k_max_vnic,
                       k_feeder_mac, PDS_ENCAP_TYPE_DOT1Q,
                       PDS_ENCAP_TYPE_MPLSoUDP, true, true, 0, 0, 5, 0);
    many_delete(bctxt, k_vnic_feeder);
}

}    // namespace api
}    // namespace test
