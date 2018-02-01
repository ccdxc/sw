// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/cpupkt_api.hpp"

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

hal_ret_t
pd_tunnelif_get_rw_idx (pd_tunnelif_get_rw_idx_args_t *args)
{
    return HAL_RET_OK;
}

hal_ret_t
lif_get_lport_id (pd_lif_get_lport_id_args_t *args)
{
    return HAL_RET_OK;
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
pd_tcpcb_create (pd_tcpcb_args_t *tcpcb)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_tcpcb_update (pd_tcpcb_args_t *tcpcb)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_tcpcb_delete(pd_tcpcb_args_t *tcpcb)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_tcpcb_get(pd_tcpcb_args_t *tcpcb)
{
    return HAL_RET_OK;
}

hal_ret_t pd_ipseccb_create(pd_ipseccb_args_t *ipseccb)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_ipseccb_update(pd_ipseccb_args_t *ipseccb)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_ipseccb_delete(pd_ipseccb_args_t *ipseccb)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_ipseccb_get(pd_ipseccb_args_t *ipseccb)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_ipseccb_decrypt_create(pd_ipseccb_args_t *ipseccb)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_ipseccb_decrypt_update(pd_ipseccb_args_t *ipseccb)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_ipseccb_decrypt_delete(pd_ipseccb_args_t *ipseccb)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_ipseccb_decrypt_get(pd_ipseccb_args_t *ipseccb)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_l4lb_create(pd_l4lb_args_t *pd_l4lb_args)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_cpucb_create(pd_cpucb_args_t *cpucb)
{
    return HAL_RET_OK;
}

hal_ret_t pd_cpucb_update(pd_cpucb_args_t *cpucb)
{
    return HAL_RET_OK;
}
hal_ret_t pd_cpucb_delete(pd_cpucb_args_t *cpucb)
{
    return HAL_RET_OK;
}
hal_ret_t pd_cpucb_get(pd_cpucb_args_t *cpucb)
{
    return HAL_RET_OK;
}
hal_ret_t pd_rw_entry_find_or_alloc(pd_rw_entry_args_t *rw_key,
                                    uint32_t *rw_idx)
{
    return HAL_RET_OK;
}

hal_ret_t pd_twice_nat_add(pd_twice_nat_entry_args_t *args,
                           uint32_t *twice_nat_idx)
{
    return HAL_RET_OK;
}

hal_ret_t pd_twice_nat_del(pd_twice_nat_entry_args_t *args)
{
    return HAL_RET_OK;
}

hal_ret_t pd_rawrcb_create(pd_rawrcb_args_t *rawrcb)
{
    return HAL_RET_OK;
}

hal_ret_t pd_rawrcb_update(pd_rawrcb_args_t *rawrcb)
{
    return HAL_RET_OK;
}

hal_ret_t pd_rawrcb_delete(pd_rawrcb_args_t *rawrcb,
                           bool retain_in_db)
{
    return HAL_RET_OK;
}

hal_ret_t pd_rawrcb_get(pd_rawrcb_args_t *rawrcb)
{
    return HAL_RET_OK;
}

hal_ret_t pd_rawccb_create(pd_rawccb_args_t *rawccb)
{
    return HAL_RET_OK;
}

hal_ret_t pd_rawccb_update(pd_rawccb_args_t *rawccb)
{
    return HAL_RET_OK;
}

hal_ret_t pd_rawccb_delete(pd_rawccb_args_t *rawccb,
                           bool retain_in_db)
{
    return HAL_RET_OK;
}

hal_ret_t pd_rawccb_get(pd_rawccb_args_t *rawccb)
{
    return HAL_RET_OK;
}

hal_ret_t pd_proxyrcb_create(pd_proxyrcb_args_t *proxyrcb)
{
    return HAL_RET_OK;
}

hal_ret_t pd_proxyrcb_update(pd_proxyrcb_args_t *proxyrcb)
{
    return HAL_RET_OK;
}

hal_ret_t pd_proxyrcb_delete(pd_proxyrcb_args_t *proxyrcb,
                             bool retain_in_db)
{
    return HAL_RET_OK;
}

hal_ret_t pd_proxyrcb_get(pd_proxyrcb_args_t *proxyrcb)
{
    return HAL_RET_OK;
}

hal_ret_t pd_proxyccb_create(pd_proxyccb_args_t *proxyccb)
{
    return HAL_RET_OK;
}

hal_ret_t pd_proxyccb_update(pd_proxyccb_args_t *proxyccb)
{
    return HAL_RET_OK;
}

hal_ret_t pd_proxyccb_delete(pd_proxyccb_args_t *proxyccb,
                             bool retain_in_db)
{
    return HAL_RET_OK;
}

hal_ret_t pd_proxyccb_get(pd_proxyccb_args_t *proxyccb)
{
    return HAL_RET_OK;
}

hal_ret_t pd_qos_class_create(pd_qos_class_args_t *qos_class)
{
    return HAL_RET_OK;
}

hal_ret_t pd_qos_class_delete(pd_qos_class_args_t *qos_class)
{
    return HAL_RET_OK;
}

hal_ret_t qos_class_get_qos_class_id(qos_class_t *qos_class,
                                     if_t *dest_if,
                                     uint32_t *qos_class_id)
{
    return HAL_RET_OK;
}

uint32_t qos_class_get_admin_cos (void)
{
    return HAL_RET_OK;
}

hal_ret_t pd_copp_create(pd_copp_args_t *copp)
{
    return HAL_RET_OK;
}

hal_ret_t pd_copp_delete(pd_copp_args_t *copp)
{
    return HAL_RET_OK;
}

hal_ret_t pd_acl_create(pd_acl_args_t *acl)
{
    return HAL_RET_OK;
}

hal_ret_t pd_acl_update(pd_acl_args_t *acl)
{
    return HAL_RET_OK;
}

hal_ret_t pd_acl_delete(pd_acl_args_t *acl)
{
    return HAL_RET_OK;
}

hal_ret_t pd_acl_mem_free(pd_acl_args_t *acl)
{
    return HAL_RET_OK;
}

hal_ret_t pd_acl_make_clone(acl_t *acl, acl_t *clone)
{
    return HAL_RET_OK;
}

hal_ret_t pd_wring_create(pd_wring_args_t *wring)
{
    return HAL_RET_OK;
}

hal_ret_t pd_wring_update(pd_wring_args_t *wring)
{
    return HAL_RET_OK;
}

hal_ret_t pd_wring_delete(pd_wring_args_t *wring)
{
    return HAL_RET_OK;
}

hal_ret_t pd_wring_get_entry(pd_wring_args_t *wring)
{
    return HAL_RET_OK;
}

hal_ret_t pd_wring_get_meta (pd_wring_args_t *wring)
{
    return HAL_RET_OK;
}

hal_ret_t pd_wring_set_meta (pd_wring_args_t *wring)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_descriptor_aol_get(pd_descr_aol_t *src, pd_descr_aol_t *dst)
{
    return HAL_RET_OK;
}

hal_ret_t pd_mirror_session_create(pd_mirror_session_args_t *args)
{
    return HAL_RET_OK;
}

hal_ret_t pd_mirror_session_get(pd_mirror_session_args_t *args)
{
    return HAL_RET_OK;
}

hal_ret_t pd_mirror_session_delete(pd_mirror_session_args_t *args)
{
    return HAL_RET_OK;
}

hal_ret_t pd_collector_create(collector_config_t *cfg)
{
    return HAL_RET_OK;
}

hal_ret_t pd_crypto_alloc_key(int32_t *key_idx)
{
    return HAL_RET_OK;
}

hal_ret_t pd_crypto_free_key(int32_t key_idx)
{
    return HAL_RET_OK;
}

hal_ret_t pd_crypto_write_key(int32_t key_idx, crypto_key_t *key)
{
    return HAL_RET_OK;
}

hal_ret_t pd_crypto_read_key(int32_t key_idx, crypto_key_t *key)
{
    return HAL_RET_OK;
}

hal_ret_t pd_crypto_asym_alloc_key(int32_t *key_idx)
{
    return HAL_RET_OK;
}

hal_ret_t pd_crypto_asym_free_key(int32_t key_idx)
{
    return HAL_RET_OK;
}

hal_ret_t pd_crypto_asym_write_key(int32_t key_idx, crypto_asym_key_t *key)
{
    return HAL_RET_OK;
}

hal_ret_t pd_crypto_asym_read_key(int32_t key_idx, crypto_asym_key_t *key)
{
    return HAL_RET_OK;
}

#if 0
hal_ret_t get_opaque_tag_addr(types::BarcoRings ring_type, uint64_t* addr)
{
    return HAL_RET_OK;
}
#endif

hal_ret_t pd_drop_stats_get(pd_system_args_t *pd_sys_args)
{
    return HAL_RET_OK;
}

hal_ret_t pd_table_stats_get(pd_system_args_t *pd_sys_args)
{
    return HAL_RET_OK;
}

hal_ret_t oif_list_create(oif_list_id_t *list)
{
    return HAL_RET_OK;
}

hal_ret_t oif_list_create_block(oif_list_id_t *list, uint32_t size)
{
    return HAL_RET_OK;
}

hal_ret_t oif_list_delete(oif_list_id_t list)
{
    return HAL_RET_OK;
}

hal_ret_t oif_list_delete_block(oif_list_id_t list, uint32_t size)
{
    return HAL_RET_OK;
}

hal_ret_t oif_list_add_oif(oif_list_id_t list, oif_t *oif)
{
    return HAL_RET_OK;
}

hal_ret_t oif_list_add_qp_oif(oif_list_id_t list, oif_t *oif)
{
    return HAL_RET_OK;
}

hal_ret_t oif_list_remove_oif(oif_list_id_t list, oif_t *oif)
{
    return HAL_RET_OK;
}

hal_ret_t oif_list_is_member(oif_list_id_t list, oif_t *oif)
{
    return HAL_RET_OK;
}

hal_ret_t oif_list_get_num_oifs(oif_list_id_t list, uint32_t &num_oifs)
{
    return HAL_RET_OK;
}

hal_ret_t oif_list_get_oif_array(oif_list_id_t list,
                                 uint32_t &num_oifs, oif_t *oifs)
{
    return HAL_RET_OK;
}

hal_ret_t oif_list_set_honor_ingress(oif_list_id_t list)
{
    return HAL_RET_OK;
}

hal_ret_t pd_mc_entry_create(pd_mc_entry_args_t *mc_entry_args)
{
    return HAL_RET_OK;
}

hal_ret_t pd_mc_entry_update(pd_mc_entry_args_t *mc_entry_args)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_mc_entry_delete (pd_mc_entry_args_t *mc_entry_args)
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

cpupkt_ctxt_t *
cpupkt_ctxt_alloc_init (void)
{
    return NULL;
}

hal_ret_t
cpupkt_register_rx_queue (cpupkt_ctxt_t* ctxt, types::WRingType type, uint32_t queue_id)
{
    return HAL_RET_OK;
}

hal_ret_t
cpupkt_register_tx_queue (cpupkt_ctxt_t* ctxt, types::WRingType type, uint32_t queue_id)
{
    return HAL_RET_OK;
}

// receive
hal_ret_t
cpupkt_poll_receive (cpupkt_ctxt_t* ctxt,
                     p4_to_p4plus_cpu_pkt_t** flow_miss_hdr,
                     uint8_t** data, size_t* data_len)
{
    return HAL_RET_OK;
}

// transmit pkt
hal_ret_t
cpupkt_send (cpupkt_ctxt_t *ctxt, types::WRingType type,
             uint32_t queue_id, cpu_to_p4plus_header_t *cpu_header,
             p4plus_to_p4_header_t* p4_header, uint8_t *data, size_t data_len,
             uint16_t dest_lif, uint8_t  qtype, uint32_t qid,
             uint8_t  ring_number) 
{
    return HAL_RET_OK;
}

hal_ret_t
cpupkt_program_send_ring_doorbell (uint16_t dest_lif, uint8_t  qtype,
                                  uint32_t qid, uint8_t  ring_number)
{
    return HAL_RET_OK;
}

}   // namespace pd
}   // namespace hal

