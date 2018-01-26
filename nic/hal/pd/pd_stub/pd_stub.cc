// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"

namespace hal {
namespace pd {

hal_ret_t
pd_vrf_create (pd_vrf_args_t *vrf)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_vrf_update (pd_vrf_args_t *vrf)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_vrf_delete (pd_vrf_args_t *vrf)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_vrf_mem_free (pd_vrf_args_t *vrf)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_vrf_make_clone (vrf_t *vrf, vrf_t *clone)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_l2seg_create (pd_l2seg_args_t *l2seg)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_l2seg_update (pd_l2seg_args_t *l2seg)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_l2seg_delete (pd_l2seg_args_t *l2seg)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_l2seg_mem_free (pd_l2seg_args_t *l2seg)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_l2seg_make_clone (l2seg_t *l2seg, l2seg_t *clone)
{
    return HAL_RET_OK;
}

l2seg_t *
find_l2seg_by_hwid (l2seg_hw_id_t hwid)
{
    return NULL;
}

hal_ret_t
pd_get_object_from_flow_lkupid (uint32_t flow_lkupid, 
                                hal_obj_id_t *obj_id,
                                void **pi_obj)
{
    return HAL_RET_OK;
}

l2seg_hw_id_t
pd_l2seg_get_flow_lkupid (l2seg_t *l2seg)
{
    return HAL_RET_OK;
}

uint32_t
pd_vrf_get_lookup_id (vrf_t *vrf)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_l2seg_get_fromcpu_vlanid (l2seg_t *l2seg, uint16_t *vid)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_vrf_get_fromcpu_vlanid (vrf_t *vrf, uint16_t *vid)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_nwsec_profile_create (pd_nwsec_profile_args_t *nwsec_profile)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_nwsec_profile_update (pd_nwsec_profile_args_t *nwsec_profile)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_nwsec_profile_delete (pd_nwsec_profile_args_t *nwsec_profile)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_nwsec_profile_mem_free (pd_nwsec_profile_args_t *args)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_nwsec_profile_make_clone (nwsec_profile_t *nwsec, nwsec_profile_t *clone)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_dos_policy_create (pd_dos_policy_args_t *dos_policy)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_dos_policy_update (pd_dos_policy_args_t *dos_policy)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_dos_policy_delete (pd_dos_policy_args_t *dos_policy)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_dos_policy_mem_free (pd_dos_policy_args_t *args)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_dos_policy_make_clone (dos_policy_t *nwsec, dos_policy_t *clone)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_lif_create (pd_lif_args_t *lif)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_lif_update (pd_lif_upd_args_t *lif)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_lif_delete (pd_lif_args_t *lif)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_lif_mem_free (pd_lif_args_t *lif)
{
    return HAL_RET_OK;
}

hal_ret_t 
pd_lif_make_clone (lif_t *lif, lif_t *clone)
{
    return HAL_RET_OK;
}

uint32_t
pd_get_hw_lif_id (lif_t *lif)
{
    return 0;
}

hal_ret_t
pd_if_create (pd_if_args_t *hal_if)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_if_update (pd_if_args_t *hal_if)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_if_delete (pd_if_args_t *hal_if)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_if_nwsec_update (pd_if_nwsec_upd_args_t *args)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_if_lif_update (pd_if_lif_upd_args_t *args)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_if_mem_free(pd_if_args_t *intf)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_if_make_clone (if_t *hal_if, if_t *clone)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_debug_cli_read (uint32_t tableid,
                   uint32_t index,
                   void     *swkey,
                   void     *swkey_mask,
                   void     *actiondata)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_debug_cli_write (uint32_t tableid,
                    uint32_t index,
                    void     *swkey,
                    void     *swkey_mask,
                    void     *actiondata)
{
    return HAL_RET_OK;
}

uint32_t
if_get_hw_lif_id (if_t *pi_if)
{
    return 0;
}

uint32_t
if_get_lport_id (if_t *pi_if)
{
    return 0;
}

uint32_t
if_get_tm_oport (if_t *pi_if)
{
    return 0;
}

uint32_t
ep_pd_get_rw_tbl_idx_from_pi_ep (ep_t *pi_ep, rewrite_actions_en rw_act)
{
    return 0;
}

uint32_t
ep_pd_get_tnnl_rw_tbl_idx_from_pi_ep (ep_t *pi_ep,
                                      tunnel_rewrite_actions_en tnnl_rw_act)
{
    return 0;
}

uint32_t
l4lb_pd_get_rw_tbl_idx_from_pi_l4lb (l4lb_service_entry_t *pi_l4lb, 
                                     rewrite_actions_en rw_act)
{
    return 0;
}

hal_ret_t
pd_ep_create(pd_ep_args_t *pd_ep_args)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_ep_update (pd_ep_upd_args_t *pd_ep_upd_args)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_ep_delete (pd_ep_args_t *pd_ep_args)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_ep_mem_free (pd_ep_args_t *args)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_ep_make_clone (ep_t *ep, ep_t *clone)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_session_create (pd_session_args_t *session)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_session_update (pd_session_args_t *session)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_session_delete(pd_session_args_t *session)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_session_get (pd_session_args_t *args)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_flow_get (session_t *session, flow_t *iflow, flow_state_t *flow_state)
{
    return HAL_RET_OK;
}


hal_ret_t
pd_add_l2seg_uplink (pd_l2seg_uplink_args_t *args)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_del_l2seg_uplink (pd_l2seg_uplink_args_t *args)
{
    return HAL_RET_OK;
}


hal_ret_t
pd_tlscb_create (pd_tlscb_args_t *tlscb)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_tlscb_update (pd_tlscb_args_t *tlscb)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_tlscb_delete (pd_tlscb_args_t *tlscb)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_tlscb_get (pd_tlscb_args_t *tlscb)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_gft_exact_match_profile_create (pd_gft_args_t *gft_args)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_gft_hdr_group_xposition_profile_create (pd_gft_args_t *gft_args)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_gft_exact_match_flow_entry_create (pd_gft_args_t *gft_args)
{
    return HAL_RET_OK;
}

}   // namespace pd
}   // namespace hal

