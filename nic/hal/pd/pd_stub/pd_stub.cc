// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/cpupkt_api.hpp"

namespace hal {
namespace pd {

#if 0
#define PD_FUNC_STUB(API)                                               \
hal_ret_t                                                               \
API (API ## _args_t *args)                                              \
{                                                                       \
    SDK_ASSERT(FALSE);                                                  \
    return HAL_RET_OK;                                                  \
}
#endif
#define PD_FUNC_STUB(API)                                               \
hal_ret_t                                                               \
API (pd_func_args_t *args)                                              \
{                                                                       \
    SDK_ASSERT(FALSE);                                                  \
    return HAL_RET_OK;                                                  \
}

// Deprecated. Cleanup
#define PD_FUNC_STUB_ARGS(API, ARGS)                                    \
hal_ret_t                                                               \
API (ARGS ## _args_t *args)                                             \
{                                                                       \
    SDK_ASSERT(FALSE);                                                  \
    return HAL_RET_OK;                                                  \
}

// init pd calls
PD_FUNC_STUB(pd_mem_init);
PD_FUNC_STUB(pd_mem_init_phase2);
PD_FUNC_STUB(pd_pgm_def_entries);
PD_FUNC_STUB(pd_pgm_def_p4plus_entries);

// vrf pd calls
PD_FUNC_STUB(pd_vrf_create);
PD_FUNC_STUB(pd_vrf_restore);
PD_FUNC_STUB(pd_vrf_delete);
PD_FUNC_STUB(pd_vrf_update);
PD_FUNC_STUB(pd_vrf_mem_free);
PD_FUNC_STUB(pd_vrf_make_clone);
PD_FUNC_STUB(pd_vrf_get);

// l2seg pd calls
PD_FUNC_STUB(pd_l2seg_create);
PD_FUNC_STUB(pd_l2seg_restore);
PD_FUNC_STUB(pd_l2seg_update);
PD_FUNC_STUB(pd_l2seg_delete);
PD_FUNC_STUB(pd_l2seg_mem_free);
PD_FUNC_STUB(pd_l2seg_make_clone);
PD_FUNC_STUB(pd_l2seg_get);

// misc apis for vrf and l2seg
PD_FUNC_STUB(pd_get_object_from_flow_lkupid);
PD_FUNC_STUB(pd_l2seg_get_flow_lkupid);
PD_FUNC_STUB(pd_vrf_get_lookup_id);
PD_FUNC_STUB(pd_l2seg_get_fromcpu_vlanid);
PD_FUNC_STUB(pd_vrf_get_fromcpu_vlanid);

// nwsec_profile pd calls
PD_FUNC_STUB(pd_nwsec_profile_create);
PD_FUNC_STUB(pd_nwsec_profile_delete);
PD_FUNC_STUB(pd_nwsec_profile_update);
PD_FUNC_STUB(pd_nwsec_profile_mem_free);
PD_FUNC_STUB(pd_nwsec_profile_make_clone);
PD_FUNC_STUB(pd_nwsec_profile_restore);
PD_FUNC_STUB(pd_nwsec_profile_get);

// dos_policy pd calls
PD_FUNC_STUB(pd_dos_policy_create);
PD_FUNC_STUB(pd_dos_policy_delete);
PD_FUNC_STUB(pd_dos_policy_update);
PD_FUNC_STUB(pd_dos_policy_mem_free);
PD_FUNC_STUB(pd_dos_policy_make_clone);

// lif pd calls
PD_FUNC_STUB(pd_lif_create);
PD_FUNC_STUB(pd_lif_delete);
PD_FUNC_STUB(pd_lif_update);
PD_FUNC_STUB(pd_lif_mem_free);
PD_FUNC_STUB(pd_lif_make_clone);
PD_FUNC_STUB(pd_lif_get);
PD_FUNC_STUB(pd_lif_stats_get);
PD_FUNC_STUB(pd_lif_sched_control);

// if calls
PD_FUNC_STUB(pd_if_create);
PD_FUNC_STUB(pd_if_delete);
PD_FUNC_STUB(pd_if_update);
PD_FUNC_STUB(pd_if_mem_free);
PD_FUNC_STUB(pd_if_make_clone);
PD_FUNC_STUB(pd_if_nwsec_update);
PD_FUNC_STUB(pd_if_lif_update);
PD_FUNC_STUB(pd_if_get);
PD_FUNC_STUB(pd_if_restore);

// ep calls
PD_FUNC_STUB(pd_ep_create);
PD_FUNC_STUB(pd_ep_update);
PD_FUNC_STUB(pd_ep_if_update);
PD_FUNC_STUB(pd_ep_delete);
PD_FUNC_STUB(pd_ep_mem_free);
PD_FUNC_STUB(pd_ep_make_clone);
PD_FUNC_STUB(pd_ep_get);
PD_FUNC_STUB(pd_ep_restore);

// session calls
PD_FUNC_STUB(pd_session_create);
PD_FUNC_STUB(pd_session_update);
PD_FUNC_STUB(pd_session_delete);
PD_FUNC_STUB(pd_session_get);
PD_FUNC_STUB(pd_session_age_reset);
PD_FUNC_STUB(pd_session_get_for_age_thread);
PD_FUNC_STUB(pd_get_cpu_bypass_flowid);
PD_FUNC_STUB(pd_flow_hash_get);

// tlscb calls
PD_FUNC_STUB(pd_tlscb_create);
PD_FUNC_STUB(pd_tlscb_update);
PD_FUNC_STUB(pd_tlscb_delete);
PD_FUNC_STUB(pd_tlscb_get);

// tlscb calls
PD_FUNC_STUB(pd_tls_proxy_cb_create);
PD_FUNC_STUB(pd_tls_proxy_cb_update);
PD_FUNC_STUB(pd_tls_proxy_cb_delete);
PD_FUNC_STUB(pd_tls_proxy_cb_get);

// tcpcb calls
PD_FUNC_STUB(pd_tcpcb_create);
PD_FUNC_STUB(pd_tcpcb_update);
PD_FUNC_STUB(pd_tcpcb_delete);
PD_FUNC_STUB(pd_tcpcb_get);

// tcpcb calls
PD_FUNC_STUB(pd_tcp_proxy_cb_create);
PD_FUNC_STUB(pd_tcp_proxy_cb_update);
PD_FUNC_STUB(pd_tcp_proxy_cb_delete);
PD_FUNC_STUB(pd_tcp_proxy_cb_get);

// ipseccb calls
PD_FUNC_STUB(pd_ipseccb_create);
PD_FUNC_STUB(pd_ipseccb_update);
PD_FUNC_STUB(pd_ipseccb_delete);
PD_FUNC_STUB(pd_ipseccb_get);

// ipseccb_decrypt calls
PD_FUNC_STUB(pd_ipseccb_decrypt_create);
PD_FUNC_STUB(pd_ipseccb_decrypt_update);
PD_FUNC_STUB(pd_ipseccb_decrypt_delete);
PD_FUNC_STUB(pd_ipseccb_decrypt_get);

// ipseccb calls
PD_FUNC_STUB(pd_ipsec_encrypt_create);
PD_FUNC_STUB(pd_ipsec_encrypt_update);
PD_FUNC_STUB(pd_ipsec_encrypt_delete);
PD_FUNC_STUB(pd_ipsec_encrypt_get);
PD_FUNC_STUB(pd_ipsec_global_stats_get);

// ipseccb_decrypt calls
PD_FUNC_STUB(pd_ipsec_decrypt_create);
PD_FUNC_STUB(pd_ipsec_decrypt_update);
PD_FUNC_STUB(pd_ipsec_decrypt_delete);
PD_FUNC_STUB(pd_ipsec_decrypt_get);

// l4lb
PD_FUNC_STUB(pd_l4lb_create);

// cpucb
PD_FUNC_STUB(pd_cpucb_create);
PD_FUNC_STUB(pd_cpucb_update);
PD_FUNC_STUB(pd_cpucb_delete);
PD_FUNC_STUB(pd_cpucb_get);

// rawrcb
PD_FUNC_STUB(pd_rawrcb_create);
PD_FUNC_STUB(pd_rawrcb_update);
PD_FUNC_STUB(pd_rawrcb_delete);
PD_FUNC_STUB(pd_rawrcb_get);

// rawccb
PD_FUNC_STUB(pd_rawccb_create);
PD_FUNC_STUB(pd_rawccb_update);
PD_FUNC_STUB(pd_rawccb_delete);
PD_FUNC_STUB(pd_rawccb_get);

// proxyrcb
PD_FUNC_STUB(pd_proxyrcb_create);
PD_FUNC_STUB(pd_proxyrcb_update);
PD_FUNC_STUB(pd_proxyrcb_delete);
PD_FUNC_STUB(pd_proxyrcb_get);

// proxyccb
PD_FUNC_STUB(pd_proxyccb_create);
PD_FUNC_STUB(pd_proxyccb_update);
PD_FUNC_STUB(pd_proxyccb_delete);
PD_FUNC_STUB(pd_proxyccb_get);

// qos clas
PD_FUNC_STUB(pd_qos_class_create);
PD_FUNC_STUB(pd_qos_class_restore);
PD_FUNC_STUB(pd_qos_class_delete);
PD_FUNC_STUB(pd_qos_class_update);
PD_FUNC_STUB(pd_qos_class_make_clone);
PD_FUNC_STUB(pd_qos_class_mem_free);
PD_FUNC_STUB(pd_qos_class_get);
PD_FUNC_STUB(pd_qos_class_periodic_stats_update);
PD_FUNC_STUB(pd_qos_class_set_global_pause_type);

// copp
PD_FUNC_STUB(pd_copp_create);
PD_FUNC_STUB(pd_copp_restore);
PD_FUNC_STUB(pd_copp_delete);
PD_FUNC_STUB(pd_copp_update);
PD_FUNC_STUB(pd_copp_make_clone);
PD_FUNC_STUB(pd_copp_mem_free);
PD_FUNC_STUB(pd_copp_get);

// acl pd calls
PD_FUNC_STUB(pd_acl_create);
PD_FUNC_STUB(pd_acl_restore);
PD_FUNC_STUB(pd_acl_delete);
PD_FUNC_STUB(pd_acl_update);
PD_FUNC_STUB(pd_acl_mem_free);
PD_FUNC_STUB(pd_acl_make_clone);
PD_FUNC_STUB(pd_acl_get);

// wring
PD_FUNC_STUB(pd_wring_create);
PD_FUNC_STUB(pd_wring_delete);
PD_FUNC_STUB(pd_wring_update);
PD_FUNC_STUB(pd_wring_get_entry);
PD_FUNC_STUB(pd_wring_get_meta);
PD_FUNC_STUB(pd_wring_set_meta);
PD_FUNC_STUB(pd_wring_get_base_addr);

// mirror session
PD_FUNC_STUB(pd_mirror_session_create);
PD_FUNC_STUB(pd_mirror_session_delete);
PD_FUNC_STUB(pd_mirror_session_get);

// drop monitor rule
PD_FUNC_STUB(pd_drop_monitor_rule_create);
PD_FUNC_STUB(pd_drop_monitor_rule_delete);
PD_FUNC_STUB(pd_drop_monitor_rule_get);

// collector
PD_FUNC_STUB(pd_collector_create);
PD_FUNC_STUB(pd_collector_delete);
PD_FUNC_STUB(pd_collector_get);

// mc entry
PD_FUNC_STUB(pd_mc_entry_get);
PD_FUNC_STUB(pd_mc_entry_create);
PD_FUNC_STUB(pd_mc_entry_delete);
PD_FUNC_STUB(pd_mc_entry_update);

// flow get
PD_FUNC_STUB(pd_flow_get);

// l2seg-uplink
PD_FUNC_STUB(pd_add_l2seg_uplink);
PD_FUNC_STUB(pd_del_l2seg_uplink);

// debug cli
PD_FUNC_STUB(pd_debug_cli_read);
PD_FUNC_STUB(pd_debug_cli_write);
PD_FUNC_STUB(pd_mpu_trace_enable);
PD_FUNC_STUB(pd_reg_write);
PD_FUNC_STUB(pd_table_metadata_get);
PD_FUNC_STUB(pd_table_get);

// get apis
PD_FUNC_STUB(pd_if_get_hw_lif_id);
PD_FUNC_STUB(pd_if_get_lport_id);
PD_FUNC_STUB(pd_if_get_tm_oport);

// twice nat
PD_FUNC_STUB(pd_rw_entry_find_or_alloc);
PD_FUNC_STUB(pd_twice_nat_add);
PD_FUNC_STUB(pd_twice_nat_del);

// qos
PD_FUNC_STUB(pd_qos_class_get_qos_class_id);
PD_FUNC_STUB(pd_qos_class_get_admin_cos);

// aol
PD_FUNC_STUB(pd_descriptor_aol_get);

// crypto
PD_FUNC_STUB(pd_crypto_alloc_key);
PD_FUNC_STUB(pd_crypto_free_key);
PD_FUNC_STUB(pd_crypto_write_key);
PD_FUNC_STUB(pd_crypto_read_key);
PD_FUNC_STUB(pd_crypto_asym_alloc_key);
PD_FUNC_STUB(pd_crypto_asym_free_key);
PD_FUNC_STUB(pd_crypto_asym_write_key);
PD_FUNC_STUB(pd_crypto_asym_read_key);

// barco
PD_FUNC_STUB(pd_get_opaque_tag_addr);

// stats
PD_FUNC_STUB(pd_drop_stats_get);
PD_FUNC_STUB(pd_drop_stats_clear);
PD_FUNC_STUB(pd_egress_drop_stats_get);
PD_FUNC_STUB(pd_egress_drop_stats_clear);
PD_FUNC_STUB(pd_table_stats_get);
PD_FUNC_STUB(pd_system_drop_stats_get);

// oifl
PD_FUNC_STUB(pd_oif_list_get);
PD_FUNC_STUB(pd_oif_list_create);
PD_FUNC_STUB(pd_oif_list_create_block);
PD_FUNC_STUB(pd_oif_list_delete);
PD_FUNC_STUB(pd_oif_list_delete_block);
PD_FUNC_STUB(pd_oif_list_attach);
PD_FUNC_STUB(pd_oif_list_detach);
PD_FUNC_STUB(pd_oif_list_add_oif);
PD_FUNC_STUB(pd_oif_list_add_qp_oif);
PD_FUNC_STUB(pd_oif_list_remove_oif);
PD_FUNC_STUB(pd_oif_list_is_member);
PD_FUNC_STUB(pd_oif_list_get_num_oifs);
PD_FUNC_STUB(pd_oif_list_set_honor_ingress);
PD_FUNC_STUB(pd_oif_list_clr_honor_ingress);

// if
PD_FUNC_STUB(pd_tunnelif_get_rw_idx);

// cpu
PD_FUNC_STUB(pd_cpupkt_ctxt_alloc_init);
PD_FUNC_STUB(pd_cpupkt_register_rx_queue);
PD_FUNC_STUB(pd_cpupkt_register_tx_queue);
PD_FUNC_STUB(pd_cpupkt_unregister_tx_queue);
PD_FUNC_STUB(pd_cpupkt_poll_receive);
PD_FUNC_STUB(pd_cpupkt_poll_receive_new);
PD_FUNC_STUB(pd_cpupkt_free);
PD_FUNC_STUB(pd_cpupkt_send);
PD_FUNC_STUB(pd_cpupkt_page_alloc);
PD_FUNC_STUB(pd_cpupkt_descr_alloc);
PD_FUNC_STUB(pd_cpupkt_program_send_ring_doorbell);
PD_FUNC_STUB(pd_cpupkt_get_global);

// rdma
PD_FUNC_STUB(pd_rxdma_table_entry_add);
PD_FUNC_STUB(pd_txdma_table_entry_add);

// lif
PD_FUNC_STUB(pd_lif_get_lport_id);

// p4pt
PD_FUNC_STUB(p4pt_pd_init);

// eth
PD_FUNC_STUB(pd_rss_params_table_entry_add);
PD_FUNC_STUB(pd_rss_indir_table_entry_add);

// asic init
PD_FUNC_STUB(pd_asic_init);

PD_FUNC_STUB(pd_table_properties_get);

// capri
PD_FUNC_STUB(pd_push_qstate_to_capri);
PD_FUNC_STUB(pd_clear_qstate);
PD_FUNC_STUB(pd_read_qstate);
PD_FUNC_STUB(pd_get_pc_offset);
PD_FUNC_STUB(pd_capri_hbm_read_mem);
PD_FUNC_STUB(pd_capri_hbm_write_mem);
PD_FUNC_STUB(pd_capri_program_label_to_offset);
PD_FUNC_STUB(pd_capri_pxb_cfg_lif_bdf);
PD_FUNC_STUB(pd_capri_program_to_base_addr);
PD_FUNC_STUB(pd_capri_barco_asym_req_descr_get);
PD_FUNC_STUB(pd_capri_barco_symm_req_descr_get);
PD_FUNC_STUB(pd_capri_barco_ring_meta_get);
PD_FUNC_STUB(pd_capri_barco_ring_meta_config_get);
PD_FUNC_STUB(pd_capri_barco_asym_ecc_point_mul_p256);
PD_FUNC_STUB(pd_capri_barco_asym_ecdsa_p256_setup_priv_key);
PD_FUNC_STUB(pd_capri_barco_asym_ecdsa_p256_sig_gen);
PD_FUNC_STUB(pd_capri_barco_asym_ecdsa_p256_sig_verify);
PD_FUNC_STUB(pd_capri_barco_asym_rsa2k_encrypt);
PD_FUNC_STUB(pd_capri_barco_asym_rsa_encrypt);
PD_FUNC_STUB(pd_capri_barco_asym_rsa2k_decrypt);
PD_FUNC_STUB(pd_capri_barco_asym_rsa2k_crt_decrypt);
PD_FUNC_STUB(pd_capri_barco_asym_rsa2k_setup_sig_gen_priv_key);
PD_FUNC_STUB(pd_capri_barco_asym_rsa_setup_priv_key);
PD_FUNC_STUB(pd_capri_barco_asym_rsa2k_crt_setup_decrypt_priv_key);
PD_FUNC_STUB(pd_capri_barco_asym_rsa2k_sig_gen);
PD_FUNC_STUB(pd_capri_barco_asym_rsa2k_sig_verify);
PD_FUNC_STUB(pd_capri_barco_asym_add_pend_req);
PD_FUNC_STUB(pd_capri_barco_asym_poll_pend_req);
PD_FUNC_STUB(pd_capri_barco_sym_hash_process_request);

// clock
PD_FUNC_STUB(pd_conv_hw_clock_to_sw_clock);
PD_FUNC_STUB(pd_conv_sw_clock_to_hw_clock);
PD_FUNC_STUB(pd_clock_delta_comp);
PD_FUNC_STUB(pd_get_clock_detail);

// gft
PD_FUNC_STUB(pd_gft_exact_match_profile_create)
PD_FUNC_STUB(pd_gft_hdr_group_xposition_profile_create)
PD_FUNC_STUB(pd_gft_exact_match_flow_entry_create)

// slab
PD_FUNC_STUB(pd_get_slab);

// quiesce
PD_FUNC_STUB(pd_quiesce_start);
PD_FUNC_STUB(pd_quiesce_stop);

// pb
PD_FUNC_STUB(pd_pb_stats_get);
PD_FUNC_STUB(pd_pb_stats_clear);

// accelerator ring group
PD_FUNC_STUB(pd_accel_rgroup_add);
PD_FUNC_STUB(pd_accel_rgroup_del);
PD_FUNC_STUB(pd_accel_rgroup_ring_add);
PD_FUNC_STUB(pd_accel_rgroup_ring_del);
PD_FUNC_STUB(pd_accel_rgroup_reset_set);
PD_FUNC_STUB(pd_accel_rgroup_enable_set);
PD_FUNC_STUB(pd_accel_rgroup_pndx_set);
PD_FUNC_STUB(pd_accel_rgroup_info_get);
PD_FUNC_STUB(pd_accel_rgroup_indices_get);


PD_FUNC_STUB(pd_fte_span_create);
PD_FUNC_STUB(pd_fte_span_update);
PD_FUNC_STUB(pd_fte_span_mem_free);
PD_FUNC_STUB(pd_fte_span_make_clone);
PD_FUNC_STUB(pd_fte_span_get);

PD_FUNC_STUB(pd_system_upgrade_table_reset);
}   // namespace pd
}   // namespace hal

