#include <hal_lock.hpp>
#include <hal_state_pd.hpp>
#include <pd_api.hpp>
#include <interface_api.hpp>
#include <l2segment_api.hpp>
#include <interface.pb.h>
#include <if_pd.hpp>
#include <lif_pd.hpp>
#include <enicif_pd.hpp>
#include "l2seg_pd.hpp"
#include "if_pd_utils.hpp"
#include <defines.h>
#include <p4pd_defaults.hpp>


namespace hal {
namespace pd {

// ----------------------------------------------------------------------------
// Enic If Create
// ----------------------------------------------------------------------------
hal_ret_t 
pd_enicif_create(pd_if_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;; 
    pd_enicif_t          *pd_enicif;

    HAL_TRACE_DEBUG("PD-ENICIF::{}: Creating pd state for EnicIf: {}", 
                    __FUNCTION__, if_get_if_id(args->intf));

    // Create Enic If PD
    pd_enicif = pd_enicif_alloc_init();
    if (pd_enicif == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    link_pi_pd(pd_enicif, args->intf);

    // Allocate Resources
    ret = pd_enicif_alloc_res(pd_enicif);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("PD-ENICIF::{}: Unable to alloc. resources for EnicIf: {}",
                      __FUNCTION__, if_get_if_id(args->intf));
        goto end;
    }

    // Program HW
    ret = pd_enicif_program_hw(pd_enicif);

end:
    if (ret != HAL_RET_OK) {
        unlink_pi_pd(pd_enicif, args->intf);
        pd_enicif_free(pd_enicif);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Allocate and Initialize EnicIf PD Instance
// ----------------------------------------------------------------------------
pd_enicif_t *
pd_enicif_alloc_init(void)
{
    return pd_enicif_init(pd_enicif_alloc());
}

// ----------------------------------------------------------------------------
// Allocate EnicIf Instance
// ----------------------------------------------------------------------------
pd_enicif_t *
pd_enicif_alloc (void)
{
    pd_enicif_t    *enicif;

    enicif = (pd_enicif_t *)g_hal_state_pd->enicif_pd_slab()->alloc();
    if (enicif == NULL) {
        return NULL;
    }
    return enicif;
}

// ----------------------------------------------------------------------------
// Initialize EnicIF PD instance
// ----------------------------------------------------------------------------
pd_enicif_t *
pd_enicif_init (pd_enicif_t *enicif)
{
    // Nothing to do currently
    if (!enicif) {
        return NULL;
    }

    // Set here if you want to initialize any fields

    return enicif;
}

// ----------------------------------------------------------------------------
// Allocate resources for PD EnicIf
// ----------------------------------------------------------------------------
hal_ret_t 
pd_enicif_alloc_res(pd_enicif_t *pd_enicif)
{
    hal_ret_t            ret = HAL_RET_OK;
    indexer::status      rs = indexer::SUCCESS;

    // Allocate lport
    rs = g_hal_state_pd->lport_idxr()->alloc((uint32_t *)&pd_enicif->
            enic_lport_id);
    if (rs != indexer::SUCCESS) {
        return HAL_RET_NO_RESOURCE;
    }
    HAL_TRACE_DEBUG("PD-EnicIf:{}: if_id:{} Allocated lport_id:{}", 
                    __FUNCTION__, 
                    if_get_if_id((if_t *)pd_enicif->pi_if),
                    pd_enicif->enic_lport_id);

    return ret;
}

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
hal_ret_t
pd_enicif_program_hw(pd_enicif_t *pd_enicif)
{
    hal_ret_t            ret;
    nwsec_profile_t      *pi_nwsec = NULL;

    pi_nwsec = (nwsec_profile_t *)if_enicif_get_pi_nwsec((if_t *)pd_enicif->pi_if);
    if (pi_nwsec == NULL) {
        HAL_TRACE_DEBUG("{}: No nwsec. Programming default", __FUNCTION__);
    }

    // Program Input Properties Mac Vlan
    ret = pd_enicif_pgm_inp_prop_mac_vlan_tbl(pd_enicif, pi_nwsec);

    // Program Output Mapping 
    ret = pd_enicif_pd_pgm_output_mapping_tbl(pd_enicif);

    return ret;
}

// ----------------------------------------------------------------------------
// Program Output Mapping Table
// ----------------------------------------------------------------------------
#define om_tmoport data.output_mapping_action_u.output_mapping_set_tm_oport
hal_ret_t
pd_enicif_pd_pgm_output_mapping_tbl(pd_enicif_t *pd_enicif)
{
    hal_ret_t                   ret = HAL_RET_OK;
    uint8_t                     tm_oport = 0;
    uint8_t                     p4plus_app_id = 0;
    output_mapping_actiondata   data;
    DirectMap                   *dm_omap = NULL;
    pd_lif_t                    *pd_lif = NULL;

    memset(&data, 0, sizeof(data));

    pd_lif = pd_enicif_get_pd_lif(pd_enicif);

    tm_oport = lif_get_port_num((lif_t *)(pd_lif->pi_lif));
    tm_oport = TM_PORT_DMA;
    p4plus_app_id = P4PLUS_APPTYPE_CLASSIC_NIC;

    data.actionid = OUTPUT_MAPPING_SET_TM_OPORT_ID;
    om_tmoport.nports = 1;
    om_tmoport.egress_port1 = tm_oport;
    om_tmoport.p4plus_app_id = p4plus_app_id;
    om_tmoport.dst_lif = pd_lif->hw_lif_id;
    om_tmoport.rdma_enabled = lif_get_enable_rdma((lif_t *)pd_lif->pi_lif);
    om_tmoport.encap_vlan_id_valid = 1;
    om_tmoport.encap_vlan_id = if_get_encap_vlan((if_t *)pd_enicif->pi_if);

    dm_omap = g_hal_state_pd->dm_table(P4TBL_ID_OUTPUT_MAPPING);
    HAL_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    ret = dm_omap->insert_withid(&data, pd_enicif->enic_lport_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-EnicIf::{}: lif_id:{} Unable to program",
                __FUNCTION__, lif_get_lif_id((lif_t *)pd_lif->pi_lif));
    } else {
        HAL_TRACE_DEBUG("PD-EnicIf::{}: lif_id:{} Success",
                __FUNCTION__, lif_get_lif_id((lif_t *)pd_lif->pi_lif));
    }
    return ret;
}

// ----------------------------------------------------------------------------
// Freeing EnicIF PD
// ----------------------------------------------------------------------------
hal_ret_t
pd_enicif_free (pd_enicif_t *enicif)
{
    g_hal_state_pd->enicif_pd_slab()->free(enicif);
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
link_pi_pd(pd_enicif_t *pd_enicif, if_t *pi_if)
{
    pd_enicif->pi_if = pi_if;
    if_set_pd_if(pi_if, pd_enicif);
}

// ----------------------------------------------------------------------------
// Un-Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
unlink_pi_pd(pd_enicif_t *pd_enicif, if_t *pi_if)
{
    pd_enicif->pi_if = NULL;
    if_set_pd_if(pi_if, NULL);
}

#define inp_prop_mac_vlan_data data.input_properties_mac_vlan_action_u.input_properties_mac_vlan_input_properties_mac_vlan
hal_ret_t
pd_enicif_pgm_inp_prop_mac_vlan_tbl(pd_enicif_t *pd_enicif, nwsec_profile_t *nwsec_prof)
{
    hal_ret_t                                   ret = HAL_RET_OK;
    input_properties_mac_vlan_swkey_t           key;
    input_properties_mac_vlan_swkey_mask_t      mask;
    input_properties_mac_vlan_actiondata        data;
    Tcam                                        *inp_prop_mac_vlan_tbl = NULL;
    mac_addr_t                                  *mac = NULL;
    // uint64_t                                    mac_int = 0;
    // intf::IfEnicType                            enicif_type = intf::IF_ENIC_TYPE_NONE;
    // pd_l2seg_t                                  *pd_l2seg = NULL;
    void                                        *pi_l2seg = NULL;
    types::encapType                            enc_type;

    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    inp_prop_mac_vlan_tbl = g_hal_state_pd->tcam_table(
                            P4TBL_ID_INPUT_PROPERTIES_MAC_VLAN);
    HAL_ASSERT_RETURN((inp_prop_mac_vlan_tbl != NULL), HAL_RET_ERR);

    // enicif_type = if_get_enicif_type((if_t*)pd_enicif->pi_if);

    // 2 Entries. 1. Host Side Entry 2. Uplink Entry
    // Entry 1: Host Side Entry
#if 0
    if (enicif_type == intf::IF_ENIC_TYPE_USEG) {
        key.vlan_tag_valid = 1;
        key.vlan_tag_vid = if_get_useg_vlan((if_t*)pd_enicif->pi_if);
    } else if (enicif_type == intf::IF_ENIC_TYPE_PVLAN) {
        key.vlan_tag_valid = 1;
        key.vlan_tag_vid = if_get_iso_vlan((if_t*)pd_enicif->pi_if);
        mac = if_get_mac_addr((if_t*)pd_enicif->pi_if);
        mac_int = MAC_TO_UINT64(*mac);
        memcpy(key.ethernet_srcAddr, &mac_int, 6);
    } else if (enicif_type == intf::IF_ENIC_TYPE_DIRECT){
        // SRIOV
    }
#endif
    key.entry_inactive_input_mac_vlan = 0;
    key.vlan_tag_valid = 1;
    key.vlan_tag_vid = if_get_encap_vlan((if_t*)pd_enicif->pi_if);
    mac = if_get_mac_addr((if_t*)pd_enicif->pi_if);
    memcpy(key.ethernet_srcAddr, *mac, 6);
    memrev(key.ethernet_srcAddr, 6);

    mask.entry_inactive_input_mac_vlan_mask = 0x1;
    mask.vlan_tag_valid_mask = ~(mask.vlan_tag_valid_mask & 0);
    mask.vlan_tag_vid_mask = ~(mask.vlan_tag_vid_mask & 0);
    memset(mask.ethernet_srcAddr_mask, ~0, sizeof(mask.ethernet_srcAddr_mask));

    pd_enicif_inp_prop_form_data(pd_enicif, nwsec_prof, data, true);
#if 0
    pd_l2seg = (pd_l2seg_t *)if_enicif_get_pd_l2seg((if_t*)pd_enicif->pi_if);
    HAL_ASSERT_RETURN((pd_l2seg != NULL), HAL_RET_ERR);
    data.actionid = INPUT_PROPERTIES_MAC_VLAN_INPUT_PROPERTIES_MAC_VLAN_ID;
    inp_prop_mac_vlan_data.vrf = pd_l2seg->l2seg_ten_hw_id;
    inp_prop_mac_vlan_data.dir = FLOW_DIR_FROM_ENIC;
    // inp_prop_mac_vlan_data.ipsg_enable = if_enicif_get_ipsg_en((if_t *)pd_enicif->pi_if);
    inp_prop_mac_vlan_data.ipsg_enable = nwsec_prof ? nwsec_prof->ipsg_en : 0;
    inp_prop_mac_vlan_data.src_lif_check_en = 0; // Enabled only for Deja-vu entry
    inp_prop_mac_vlan_data.src_lif = 0;
    // inp_prop_mac_vlan_data.l4_profile_idx = pd_enicif_get_l4_prof_idx(pd_enicif);
    inp_prop_mac_vlan_data.l4_profile_idx = nwsec_get_nwsec_prof_hw_id(nwsec_prof);
    inp_prop_mac_vlan_data.src_lport = pd_enicif->enic_lport_id;
#endif

    // TODO: Fill these fields eventually
#if 0
    inp_prop_mac_vlan_data.flow_miss_action    
    inp_prop_mac_vlan_data.flow_miss_idx    
    inp_prop_mac_vlan_data.dscp    
    inp_prop_mac_vlan_data.dst_lif    
    inp_prop_mac_vlan_data.filter    
#endif

    ret = inp_prop_mac_vlan_tbl->insert(&key, &mask, &data, 
                                        &(pd_enicif->inp_prop_mac_vlan_idx_host));
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-ENICIF::{}: Unable to program for Host traffic (EnicIf): {}",
                __FUNCTION__, if_get_if_id((if_t*)pd_enicif->pi_if));
        goto end;
    } else {
        HAL_TRACE_DEBUG("PD-ENICIF::{}: Programmed for Host traffic (EnicIf): {} TcamIdx: {}",
                __FUNCTION__, if_get_if_id((if_t*)pd_enicif->pi_if), 
                pd_enicif->inp_prop_mac_vlan_idx_host);
    }



    // Entry 2: Uplink Entry - Has to be installed only in EndHost Mode
    //          Used to do Deja-vu check. The src_lif will not match the 
    //          lif on uplink if.
    memset(&data, 0, sizeof(data));
    
    // TODO: Set the direction bit in the key. 
    // Handles Encap Vlan vs User Vlan conflicts

    pi_l2seg = if_enicif_get_pi_l2seg((if_t*)pd_enicif->pi_if);
    enc_type = l2seg_get_fab_encap_type((l2seg_t*)pi_l2seg);
    if (enc_type == types::ENCAP_TYPE_DOT1Q) {
        key.vlan_tag_valid = 1;
        key.vlan_tag_vid = l2seg_get_fab_encap_val((l2seg_t *)pi_l2seg);
    } else {
        // TODO: What if fab encap is Tunnel ...
        HAL_TRACE_ERR("PD-ENICIF::{}: FabEncap = VXLAN - NOT IMPLEMENTED",
                      __FUNCTION__);
        return HAL_RET_OK;
    }

    pd_enicif_inp_prop_form_data(pd_enicif, nwsec_prof, data, false);
#if 0
    // Data. Only srclif as this will make the pkt drop
    inp_prop_mac_vlan_data.src_lif_check_en = 1;
    inp_prop_mac_vlan_data.src_lif = if_get_hw_lif_id((if_t*)pd_enicif->pi_if);
#endif

    ret = inp_prop_mac_vlan_tbl->insert(&key, &mask, &data, 
                                        &(pd_enicif->inp_prop_mac_vlan_idx_upl));
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-ENICIF::{}: Unable to program for Uplink traffic (EnicIf): {} ret: {}",
                __FUNCTION__, if_get_if_id((if_t*)pd_enicif->pi_if), ret);
        goto end;
    } else {
        HAL_TRACE_ERR("PD-ENICIF::{}: Programmed for Uplink traffic (EnicIf): {} TcamIdx: {}",
                __FUNCTION__, if_get_if_id((if_t*)pd_enicif->pi_if), 
                pd_enicif->inp_prop_mac_vlan_idx_upl);
    }

end:
    return ret;
}

hal_ret_t
pd_enicif_inp_prop_form_data (pd_enicif_t *pd_enicif,
                              nwsec_profile_t *nwsec_prof,
                              input_properties_mac_vlan_actiondata &data,
                              bool host_entry)
{
    pd_l2seg_t      *pd_l2seg = NULL;
    hal_ret_t       ret = HAL_RET_OK;

    memset(&data, 0, sizeof(data));

    if (host_entry) {
        pd_l2seg = (pd_l2seg_t *)if_enicif_get_pd_l2seg((if_t*)pd_enicif->pi_if);
        HAL_ASSERT_RETURN((pd_l2seg != NULL), HAL_RET_ERR);

        data.actionid = INPUT_PROPERTIES_MAC_VLAN_INPUT_PROPERTIES_MAC_VLAN_ID;
        inp_prop_mac_vlan_data.vrf = pd_l2seg->l2seg_ten_hw_id;
        inp_prop_mac_vlan_data.dir = FLOW_DIR_FROM_ENIC;
        // inp_prop_mac_vlan_data.ipsg_enable = if_enicif_get_ipsg_en((if_t *)pd_enicif->pi_if);
        inp_prop_mac_vlan_data.ipsg_enable = nwsec_prof ? nwsec_prof->ipsg_en : 0;
        inp_prop_mac_vlan_data.src_lif_check_en = 0; // Enabled only for Deja-vu entry
        inp_prop_mac_vlan_data.src_lif = 0;
        // inp_prop_mac_vlan_data.l4_profile_idx = pd_enicif_get_l4_prof_idx(pd_enicif);
        inp_prop_mac_vlan_data.l4_profile_idx = nwsec_prof ? nwsec_get_nwsec_prof_hw_id(nwsec_prof) : L4_PROF_DEFAULT_ENTRY;
        inp_prop_mac_vlan_data.src_lport = pd_enicif->enic_lport_id;
    } else {
        inp_prop_mac_vlan_data.src_lif_check_en = 1;
        inp_prop_mac_vlan_data.src_lif = if_get_hw_lif_id((if_t*)pd_enicif->pi_if);
    }

    return ret;
}

hal_ret_t
pd_enicif_upd_inp_prop_mac_vlan_tbl (pd_enicif_t *pd_enicif, 
                                     nwsec_profile_t *nwsec_prof)
{
    hal_ret_t                                   ret = HAL_RET_OK;
    input_properties_mac_vlan_actiondata        data;
    Tcam                                        *inp_prop_mac_vlan_tbl = NULL;

    inp_prop_mac_vlan_tbl = g_hal_state_pd->tcam_table(
                            P4TBL_ID_INPUT_PROPERTIES_MAC_VLAN);
    HAL_ASSERT_RETURN((inp_prop_mac_vlan_tbl != NULL), HAL_RET_ERR);

    pd_enicif_inp_prop_form_data(pd_enicif, nwsec_prof, data, true);

    ret = inp_prop_mac_vlan_tbl->update(pd_enicif->inp_prop_mac_vlan_idx_host, &data);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-ENICIF::{}: Unable to program for Host traffic (EnicIf): {}",
                __FUNCTION__, if_get_if_id((if_t*)pd_enicif->pi_if));
        goto end;
    } else {
        HAL_TRACE_DEBUG("PD-ENICIF::{}: Programmed for Host traffic (EnicIf): {} TcamIdx: {}",
                __FUNCTION__, if_get_if_id((if_t*)pd_enicif->pi_if), 
                pd_enicif->inp_prop_mac_vlan_idx_host);
    }

    // Generally there is nothing to update for network side entries. So skipping it

end:
    return ret;
}






uint32_t
pd_enicif_get_l4_prof_idx(pd_enicif_t *pd_enicif)
{
    if_t        *pi_if = NULL;
    tenant_t    *pi_tenant = NULL;

    pi_if = (if_t *)pd_enicif->pi_if;
    HAL_ASSERT_RETURN(pi_if != NULL, 0);

    pi_tenant = if_get_pi_tenant(pi_if);
    HAL_ASSERT_RETURN(pi_tenant != NULL, 0);

    return ten_get_nwsec_prof_hw_id(pi_tenant);
}

pd_lif_t *
pd_enicif_get_pd_lif(pd_enicif_t *pd_enicif)
{
    if_t        *pi_if = NULL;
    pd_lif_t    *pd_lif = NULL;
    lif_t       *pi_lif = NULL;

    pi_if = (if_t *)pd_enicif->pi_if;
    HAL_ASSERT_RETURN(pi_if != NULL, 0);

    pi_lif = if_get_lif(pi_if);
    HAL_ASSERT(pi_lif != NULL);

    pd_lif = (pd_lif_t *)lif_get_pd_lif(pi_lif);
    HAL_ASSERT(pi_lif != NULL);

    return pd_lif;
}

}    // namespace pd
}    // namespace hal
