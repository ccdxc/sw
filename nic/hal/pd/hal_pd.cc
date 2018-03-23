#include <string>
#include <dlfcn.h>
#include "nic/include/hal.hpp"
#include "nic/include/hal_pd.hpp"
#include "nic/include/asic_pd.hpp"
#include "nic/hal/pd/pd_api.hpp"
#include "nic/hal/pd/pd_api_c.h"

namespace hal {

extern bool gl_super_user;

namespace pd {

pd_call_t    *g_pd_calls;

#define PD_SYMBOL_LOAD(PD_FUNC_ID, NAME)                                       \
{                                                                              \
    g_pd_calls[PD_FUNC_ID].NAME =                                              \
        (NAME ## _t)dlsym(hal_cfg->pd_so, #NAME);                              \
    dlsym_error = dlerror();                                                   \
    if (dlsym_error) {                                                         \
        HAL_TRACE_DEBUG("Failed to load symbol from PD lib {}:{}", #NAME,      \
                        dlsym_error);                                          \
        g_pd_calls[PD_FUNC_ID].NAME =                                          \
            (NAME ## _t)dlsym(hal_cfg->pd_stub_so, #NAME);                     \
        dlsym_error = dlerror();                                               \
        if (dlsym_error) {                                                     \
            HAL_TRACE_ERR("Failed to load symbol from PD stub lib {}:{}",      \
                          #NAME, dlsym_error);                                 \
            HAL_ASSERT(0);                                                     \
        }                                                                      \
    }                                                                          \
}

//------------------------------------------------------------------------------
// TODO: Sample Expansion of above macro. Remove this once the code is stable
//
//   g_pd_calls[PD_FUNC_ID_VRF_CREATE].pd_vrf_create = 
//       (pd_vrf_create_t)dlsym(g_hal_state->pd_so(), 
//                              "pd_vrf_create");
//   dlsym_error = dlerror();
//   if (dlsym_error) {
//       HAL_TRACE_ERR("{}: cannot load symbol from PD LIB {}: {}", 
//                     __FUNCTION__, "pd_vrf_create", dlsym_error);
//       g_pd_calls[PD_FUNC_ID_VRF_CREATE].pd_vrf_create = 
//           (pd_vrf_create_t)dlsym(g_hal_state->pd_stub_so(), 
//                                  "pd_vrf_create");
//       dlsym_error = dlerror();
//       if (dlsym_error) {
//           HAL_TRACE_ERR("{}: cannot load symbol from PD STUB LIB {}: {}", 
//                         __FUNCTION__, "pd_vrf_create", dlsym_error);
//           HAL_ASSERT(0);
//       }
//   }
//------------------------------------------------------------------------------
hal_ret_t
hal_pd_load_symbols (hal_cfg_t *hal_cfg)
{
    hal_ret_t       ret = HAL_RET_OK;
    const char*     dlsym_error = NULL;

    g_pd_calls = (pd_call_t *)HAL_CALLOC(HAL_MEM_ALLOC_PD_CALLS, 
                                       PD_FUNC_ID_MAX * sizeof(pd_call_t));

    // init pd calls
    PD_SYMBOL_LOAD(PD_FUNC_ID_MEM_INIT, pd_mem_init);
    PD_SYMBOL_LOAD(PD_FUNC_ID_MEM_INIT_PHASE2, pd_mem_init_phase2);
    PD_SYMBOL_LOAD(PD_FUNC_ID_PGM_DEF_ENTRIES, pd_pgm_def_entries);
    PD_SYMBOL_LOAD(PD_FUNC_ID_PGM_DEF_P4PLUS_ENTRIES, pd_pgm_def_p4plus_entries);

    // vrf pd calls
    PD_SYMBOL_LOAD(PD_FUNC_ID_VRF_CREATE, pd_vrf_create);
    PD_SYMBOL_LOAD(PD_FUNC_ID_VRF_DELETE, pd_vrf_delete);
    PD_SYMBOL_LOAD(PD_FUNC_ID_VRF_UPDATE, pd_vrf_update);
    PD_SYMBOL_LOAD(PD_FUNC_ID_VRF_MEM_FREE, pd_vrf_mem_free);
    PD_SYMBOL_LOAD(PD_FUNC_ID_VRF_MAKE_CLONE, pd_vrf_make_clone);

    // l2seg pd calls
    PD_SYMBOL_LOAD(PD_FUNC_ID_L2SEG_CREATE, pd_l2seg_create);
    PD_SYMBOL_LOAD(PD_FUNC_ID_L2SEG_DELETE, pd_l2seg_delete);
    PD_SYMBOL_LOAD(PD_FUNC_ID_L2SEG_UPDATE, pd_l2seg_update);
    PD_SYMBOL_LOAD(PD_FUNC_ID_L2SEG_MEM_FREE, pd_l2seg_mem_free);
    PD_SYMBOL_LOAD(PD_FUNC_ID_L2SEG_MAKE_CLONE, pd_l2seg_make_clone);
    PD_SYMBOL_LOAD(PD_FUNC_ID_FIND_L2SEG_BY_HWID, pd_find_l2seg_by_hwid);

    // misc apis for vrf and l2seg
    PD_SYMBOL_LOAD(PD_FUNC_ID_GET_OBJ_FROM_FLOW_LKPID, pd_get_object_from_flow_lkupid);
    PD_SYMBOL_LOAD(PD_FUNC_ID_L2SEG_GET_FLOW_LKPID, pd_l2seg_get_flow_lkupid);
    PD_SYMBOL_LOAD(PD_FUNC_ID_VRF_GET_FLOW_LKPID, pd_vrf_get_lookup_id);
    PD_SYMBOL_LOAD(PD_FUNC_ID_L2SEG_GET_FRCPU_VLANID, pd_l2seg_get_fromcpu_vlanid);
    PD_SYMBOL_LOAD(PD_FUNC_ID_VRF_GET_FRCPU_VLANID, pd_vrf_get_fromcpu_vlanid);

    // nwsec profile pd calls
    PD_SYMBOL_LOAD(PD_FUNC_ID_NWSEC_PROF_CREATE, pd_nwsec_profile_create);
    PD_SYMBOL_LOAD(PD_FUNC_ID_NWSEC_PROF_DELETE, pd_nwsec_profile_delete);
    PD_SYMBOL_LOAD(PD_FUNC_ID_NWSEC_PROF_UPDATE, pd_nwsec_profile_update);
    PD_SYMBOL_LOAD(PD_FUNC_ID_NWSEC_PROF_MEM_FREE, pd_nwsec_profile_mem_free);
    PD_SYMBOL_LOAD(PD_FUNC_ID_NWSEC_PROF_MAKE_CLONE, pd_nwsec_profile_make_clone);

    // dos policy pd calls
    PD_SYMBOL_LOAD(PD_FUNC_ID_DOS_POLICY_CREATE, pd_dos_policy_create);
    PD_SYMBOL_LOAD(PD_FUNC_ID_DOS_POLICY_DELETE, pd_dos_policy_delete);
    PD_SYMBOL_LOAD(PD_FUNC_ID_DOS_POLICY_UPDATE, pd_dos_policy_update);
    PD_SYMBOL_LOAD(PD_FUNC_ID_DOS_POLICY_MEM_FREE, pd_dos_policy_mem_free);
    PD_SYMBOL_LOAD(PD_FUNC_ID_DOS_POLICY_MAKE_CLONE, pd_dos_policy_make_clone);

    // lif pd calls
    PD_SYMBOL_LOAD(PD_FUNC_ID_LIF_CREATE, pd_lif_create);
    PD_SYMBOL_LOAD(PD_FUNC_ID_LIF_DELETE, pd_lif_delete);
    PD_SYMBOL_LOAD(PD_FUNC_ID_LIF_UPDATE, pd_lif_update);
    PD_SYMBOL_LOAD(PD_FUNC_ID_LIF_MEM_FREE, pd_lif_mem_free);
    PD_SYMBOL_LOAD(PD_FUNC_ID_LIF_MAKE_CLONE, pd_lif_make_clone);
    PD_SYMBOL_LOAD(PD_FUNC_ID_LIF_GET, pd_lif_get);

    // if pd calls
    PD_SYMBOL_LOAD(PD_FUNC_ID_IF_CREATE, pd_if_create);
    PD_SYMBOL_LOAD(PD_FUNC_ID_IF_DELETE, pd_if_delete);
    PD_SYMBOL_LOAD(PD_FUNC_ID_IF_UPDATE, pd_if_update);
    PD_SYMBOL_LOAD(PD_FUNC_ID_IF_MEM_FREE, pd_if_mem_free);
    PD_SYMBOL_LOAD(PD_FUNC_ID_IF_MAKE_CLONE, pd_if_make_clone);
    PD_SYMBOL_LOAD(PD_FUNC_ID_IF_NWSEC_UPDATE, pd_if_nwsec_update);
    PD_SYMBOL_LOAD(PD_FUNC_ID_IF_LIF_UPDATE, pd_if_lif_update);
    PD_SYMBOL_LOAD(PD_FUNC_ID_IF_GET, pd_if_get);

    // ep pd calls
    PD_SYMBOL_LOAD(PD_FUNC_ID_EP_CREATE, pd_ep_create);
    PD_SYMBOL_LOAD(PD_FUNC_ID_EP_DELETE, pd_ep_delete);
    PD_SYMBOL_LOAD(PD_FUNC_ID_EP_UPDATE, pd_ep_update);
    PD_SYMBOL_LOAD(PD_FUNC_ID_EP_MEM_FREE, pd_ep_mem_free);
    PD_SYMBOL_LOAD(PD_FUNC_ID_EP_MAKE_CLONE, pd_ep_make_clone);

    // session pd calls
    PD_SYMBOL_LOAD(PD_FUNC_ID_SESSION_CREATE, pd_session_create);
    PD_SYMBOL_LOAD(PD_FUNC_ID_SESSION_DELETE, pd_session_delete);
    PD_SYMBOL_LOAD(PD_FUNC_ID_SESSION_UPDATE, pd_session_update);
    PD_SYMBOL_LOAD(PD_FUNC_ID_SESSION_GET, pd_session_get);

    // tlscb pd calls
    PD_SYMBOL_LOAD(PD_FUNC_ID_TLSCB_CREATE, pd_tlscb_create);
    PD_SYMBOL_LOAD(PD_FUNC_ID_TLSCB_DELETE, pd_tlscb_delete);
    PD_SYMBOL_LOAD(PD_FUNC_ID_TLSCB_UPDATE, pd_tlscb_update);
    PD_SYMBOL_LOAD(PD_FUNC_ID_TLSCB_GET, pd_tlscb_get);

    // tcpcb pd calls
    PD_SYMBOL_LOAD(PD_FUNC_ID_TCPCB_CREATE, pd_tcpcb_create);
    PD_SYMBOL_LOAD(PD_FUNC_ID_TCPCB_DELETE, pd_tcpcb_delete);
    PD_SYMBOL_LOAD(PD_FUNC_ID_TCPCB_UPDATE, pd_tcpcb_update);
    PD_SYMBOL_LOAD(PD_FUNC_ID_TCPCB_GET, pd_tcpcb_get);

    // ipseccb pd calls
    PD_SYMBOL_LOAD(PD_FUNC_ID_IPSECCB_CREATE, pd_ipseccb_create);
    PD_SYMBOL_LOAD(PD_FUNC_ID_IPSECCB_DELETE, pd_ipseccb_delete);
    PD_SYMBOL_LOAD(PD_FUNC_ID_IPSECCB_UPDATE, pd_ipseccb_update);
    PD_SYMBOL_LOAD(PD_FUNC_ID_IPSECCB_GET, pd_ipseccb_get);

    // ipseccb_decrypt pd calls
    PD_SYMBOL_LOAD(PD_FUNC_ID_IPSECCB_DECRYPT_CREATE, pd_ipseccb_decrypt_create);
    PD_SYMBOL_LOAD(PD_FUNC_ID_IPSECCB_DECRYPT_DELETE, pd_ipseccb_decrypt_delete);
    PD_SYMBOL_LOAD(PD_FUNC_ID_IPSECCB_DECRYPT_UPDATE, pd_ipseccb_decrypt_update);
    PD_SYMBOL_LOAD(PD_FUNC_ID_IPSECCB_DECRYPT_GET, pd_ipseccb_decrypt_get);

    // l4lb
    PD_SYMBOL_LOAD(PD_FUNC_ID_L4LB_CREATE, pd_l4lb_create);

    // cpucb
    PD_SYMBOL_LOAD(PD_FUNC_ID_CPUCB_CREATE, pd_cpucb_create);
    PD_SYMBOL_LOAD(PD_FUNC_ID_CPUCB_DELETE, pd_cpucb_delete);
    PD_SYMBOL_LOAD(PD_FUNC_ID_CPUCB_UPDATE, pd_cpucb_update);
    PD_SYMBOL_LOAD(PD_FUNC_ID_CPUCB_GET, pd_cpucb_get);

    // rawrcb
    PD_SYMBOL_LOAD(PD_FUNC_ID_RAWRCB_CREATE, pd_rawrcb_create);
    PD_SYMBOL_LOAD(PD_FUNC_ID_RAWRCB_DELETE, pd_rawrcb_delete);
    PD_SYMBOL_LOAD(PD_FUNC_ID_RAWRCB_UPDATE, pd_rawrcb_update);
    PD_SYMBOL_LOAD(PD_FUNC_ID_RAWRCB_GET, pd_rawrcb_get);


    // rawccb
    PD_SYMBOL_LOAD(PD_FUNC_ID_RAWCCB_CREATE, pd_rawccb_create);
    PD_SYMBOL_LOAD(PD_FUNC_ID_RAWCCB_DELETE, pd_rawccb_delete);
    PD_SYMBOL_LOAD(PD_FUNC_ID_RAWCCB_UPDATE, pd_rawccb_update);
    PD_SYMBOL_LOAD(PD_FUNC_ID_RAWCCB_GET, pd_rawccb_get);

    // proxyrcb
    PD_SYMBOL_LOAD(PD_FUNC_ID_PROXYRCB_CREATE, pd_proxyrcb_create);
    PD_SYMBOL_LOAD(PD_FUNC_ID_PROXYRCB_DELETE, pd_proxyrcb_delete);
    PD_SYMBOL_LOAD(PD_FUNC_ID_PROXYRCB_UPDATE, pd_proxyrcb_update);
    PD_SYMBOL_LOAD(PD_FUNC_ID_PROXYRCB_GET, pd_proxyrcb_get);

    // proxyccb
    PD_SYMBOL_LOAD(PD_FUNC_ID_PROXYCCB_CREATE, pd_proxyccb_create);
    PD_SYMBOL_LOAD(PD_FUNC_ID_PROXYCCB_DELETE, pd_proxyccb_delete);
    PD_SYMBOL_LOAD(PD_FUNC_ID_PROXYCCB_UPDATE, pd_proxyccb_update);
    PD_SYMBOL_LOAD(PD_FUNC_ID_PROXYCCB_GET, pd_proxyccb_get);

    // qos class
    PD_SYMBOL_LOAD(PD_FUNC_ID_QOS_CLASS_CREATE, pd_qos_class_create);
    PD_SYMBOL_LOAD(PD_FUNC_ID_QOS_CLASS_DELETE, pd_qos_class_delete);
    PD_SYMBOL_LOAD(PD_FUNC_ID_QOS_CLASS_UPDATE, pd_qos_class_update);
    PD_SYMBOL_LOAD(PD_FUNC_ID_QOS_CLASS_MAKE_CLONE, pd_qos_class_make_clone);
    PD_SYMBOL_LOAD(PD_FUNC_ID_QOS_CLASS_MEM_FREE, pd_qos_class_mem_free);

    // copp
    PD_SYMBOL_LOAD(PD_FUNC_ID_COPP_CREATE, pd_copp_create);
    PD_SYMBOL_LOAD(PD_FUNC_ID_COPP_DELETE, pd_copp_delete);
    PD_SYMBOL_LOAD(PD_FUNC_ID_COPP_UPDATE, pd_copp_update);
    PD_SYMBOL_LOAD(PD_FUNC_ID_COPP_MAKE_CLONE, pd_copp_make_clone);
    PD_SYMBOL_LOAD(PD_FUNC_ID_COPP_MEM_FREE, pd_copp_mem_free);

    // acl
    PD_SYMBOL_LOAD(PD_FUNC_ID_ACL_CREATE, pd_acl_create);
    PD_SYMBOL_LOAD(PD_FUNC_ID_ACL_DELETE, pd_acl_delete);
    PD_SYMBOL_LOAD(PD_FUNC_ID_ACL_UPDATE, pd_acl_update);
    PD_SYMBOL_LOAD(PD_FUNC_ID_ACL_MEM_FREE, pd_acl_mem_free);
    PD_SYMBOL_LOAD(PD_FUNC_ID_ACL_MAKE_CLONE, pd_acl_make_clone);

    // wring
    PD_SYMBOL_LOAD(PD_FUNC_ID_WRING_CREATE, pd_wring_create);
    // PD_SYMBOL_LOAD(PD_FUNC_ID_WRING_DELETE, pd_wring_delete);
    // PD_SYMBOL_LOAD(PD_FUNC_ID_WRING_UPDATE, pd_wring_update);
    PD_SYMBOL_LOAD(PD_FUNC_ID_WRING_GET_ENTRY, pd_wring_get_entry);
    PD_SYMBOL_LOAD(PD_FUNC_ID_WRING_GET_META, pd_wring_get_meta);
    PD_SYMBOL_LOAD(PD_FUNC_ID_WRING_SET_META, pd_wring_set_meta);

    // mirror session
    PD_SYMBOL_LOAD(PD_FUNC_ID_MIRROR_SESSION_CREATE, pd_mirror_session_create);
    PD_SYMBOL_LOAD(PD_FUNC_ID_MIRROR_SESSION_DELETE, pd_mirror_session_delete);
    PD_SYMBOL_LOAD(PD_FUNC_ID_MIRROR_SESSION_GET, pd_mirror_session_get);

    // collector
    PD_SYMBOL_LOAD(PD_FUNC_ID_COLLECTOR_CREATE, pd_collector_create);

    // mc entry
    PD_SYMBOL_LOAD(PD_FUNC_ID_MC_ENTRY_CREATE, pd_mc_entry_create);
    PD_SYMBOL_LOAD(PD_FUNC_ID_MC_ENTRY_DELETE, pd_mc_entry_delete);
    // PD_SYMBOL_LOAD(PD_FUNC_ID_MC_ENTRY_UPDATE, pd_mc_entry_update);

    // flow get
    PD_SYMBOL_LOAD(PD_FUNC_ID_FLOW_GET, pd_flow_get);

    // l2seg-uplink
    PD_SYMBOL_LOAD(PD_FUNC_ID_ADD_L2SEG_UPLINK, pd_add_l2seg_uplink);
    PD_SYMBOL_LOAD(PD_FUNC_ID_DEL_L2SEG_UPLINK, pd_del_l2seg_uplink);

    // debug cli
    PD_SYMBOL_LOAD(PD_FUNC_ID_DEBUG_CLI_READ, pd_debug_cli_read);
    PD_SYMBOL_LOAD(PD_FUNC_ID_DEBUG_CLI_WRITE, pd_debug_cli_write);
    PD_SYMBOL_LOAD(PD_FUNC_ID_MPU_TRACE_ENABLE, pd_mpu_trace_enable);

    // apis
    PD_SYMBOL_LOAD(PD_FUNC_ID_IF_GET_HW_LIF_ID, pd_if_get_hw_lif_id);
    PD_SYMBOL_LOAD(PD_FUNC_ID_IF_GET_LPORT_ID, pd_if_get_lport_id);
    PD_SYMBOL_LOAD(PD_FUNC_ID_IF_GET_TM_OPORT, pd_if_get_tm_oport);

    // twice nat
    PD_SYMBOL_LOAD(PD_FUNC_ID_RWENTRY_FIND_OR_ALLOC, pd_rw_entry_find_or_alloc);
    PD_SYMBOL_LOAD(PD_FUNC_ID_TWICE_NAT_ADD, pd_twice_nat_add);
    PD_SYMBOL_LOAD(PD_FUNC_ID_TWICE_NAT_DEL, pd_twice_nat_del);

    // qos
    PD_SYMBOL_LOAD(PD_FUNC_ID_GET_QOS_CLASSID, pd_qos_class_get_qos_class_id);
    PD_SYMBOL_LOAD(PD_FUNC_ID_QOS_GET_ADMIN_COS, pd_qos_class_get_admin_cos);

    // aol
    PD_SYMBOL_LOAD(PD_FUNC_ID_DESC_AOL_GET, pd_descriptor_aol_get);

    // crypto
    PD_SYMBOL_LOAD(PD_FUNC_ID_CRYPTO_ALLOC_KEY, pd_crypto_alloc_key);
    PD_SYMBOL_LOAD(PD_FUNC_ID_CRYPTO_FREE_KEY, pd_crypto_free_key);
    PD_SYMBOL_LOAD(PD_FUNC_ID_CRYPTO_WRITE_KEY, pd_crypto_write_key);
    PD_SYMBOL_LOAD(PD_FUNC_ID_CRYPTO_READ_KEY, pd_crypto_read_key);
    PD_SYMBOL_LOAD(PD_FUNC_ID_CRYPTO_ASYM_ALLOC_KEY, pd_crypto_asym_alloc_key);
    PD_SYMBOL_LOAD(PD_FUNC_ID_CRYPTO_ASYM_FREE_KEY, pd_crypto_asym_free_key);
    PD_SYMBOL_LOAD(PD_FUNC_ID_CRYPTO_ASYM_WRITE_KEY, pd_crypto_asym_write_key);
    PD_SYMBOL_LOAD(PD_FUNC_ID_CRYPTO_ASYM_READ_KEY, pd_crypto_asym_read_key);

    // barco
    PD_SYMBOL_LOAD(PD_FUNC_ID_OPAQUE_TAG_ADDR, pd_get_opaque_tag_addr);

    // stats
    PD_SYMBOL_LOAD(PD_FUNC_ID_DROP_STATS_GET, pd_drop_stats_get);
    PD_SYMBOL_LOAD(PD_FUNC_ID_TABLE_STATS_GET, pd_table_stats_get);
    PD_SYMBOL_LOAD(PD_FUNC_ID_SYSTEM_DROP_ACTION, pd_system_drop_action);

    // oifl
    PD_SYMBOL_LOAD(PD_FUNC_ID_OIFL_CREATE, pd_oif_list_create);
    PD_SYMBOL_LOAD(PD_FUNC_ID_OIFL_CREATE_BLOCK, pd_oif_list_create_block);
    PD_SYMBOL_LOAD(PD_FUNC_ID_OIFL_DELETE, pd_oif_list_delete);
    PD_SYMBOL_LOAD(PD_FUNC_ID_OIFL_DELETE_BLOCK, pd_oif_list_delete_block);
    PD_SYMBOL_LOAD(PD_FUNC_ID_OIFL_ADD_OIF, pd_oif_list_add_oif);
    PD_SYMBOL_LOAD(PD_FUNC_ID_OIFL_ADD_QP_OIF, pd_oif_list_add_qp_oif);
    PD_SYMBOL_LOAD(PD_FUNC_ID_OIFL_REM_OIF, pd_oif_list_remove_oif);
    PD_SYMBOL_LOAD(PD_FUNC_ID_OIFL_IS_MEMBER, pd_oif_list_is_member);
    PD_SYMBOL_LOAD(PD_FUNC_ID_GET_NUM_OIFS, pd_oif_list_get_num_oifs);
    PD_SYMBOL_LOAD(PD_FUNC_ID_GET_OIF_ARRAY, pd_oif_list_get_oif_array);
    PD_SYMBOL_LOAD(PD_FUNC_ID_SET_HONOR_ING, pd_oif_list_set_honor_ingress);
    PD_SYMBOL_LOAD(PD_FUNC_ID_CLR_HONOR_ING, pd_oif_list_clr_honor_ingress);

    // tnnl if
    PD_SYMBOL_LOAD(PD_FUNC_ID_TNNL_IF_GET_RW_IDX, pd_tunnelif_get_rw_idx);

    // cpu
    PD_SYMBOL_LOAD(PD_FUNC_ID_CPU_ALLOC_INIT, pd_cpupkt_ctxt_alloc_init);
    PD_SYMBOL_LOAD(PD_FUNC_ID_CPU_REG_RXQ, pd_cpupkt_register_rx_queue);
    PD_SYMBOL_LOAD(PD_FUNC_ID_CPU_REG_TXQ, pd_cpupkt_register_tx_queue);
    PD_SYMBOL_LOAD(PD_FUNC_ID_CPU_UNREG_TXQ, pd_cpupkt_unregister_tx_queue);
    PD_SYMBOL_LOAD(PD_FUNC_ID_CPU_POLL_REC, pd_cpupkt_poll_receive);
    PD_SYMBOL_LOAD(PD_FUNC_ID_CPU_FREE, pd_cpupkt_free);
    PD_SYMBOL_LOAD(PD_FUNC_ID_CPU_SEND, pd_cpupkt_send);
    PD_SYMBOL_LOAD(PD_FUNC_ID_CPU_PAGE_ALLOC, pd_cpupkt_page_alloc);
    PD_SYMBOL_LOAD(PD_FUNC_ID_CPU_DESCR_ALLOC, pd_cpupkt_descr_alloc);
    PD_SYMBOL_LOAD(PD_FUNC_ID_PGM_SEND_RING_DBELL, pd_cpupkt_program_send_ring_doorbell);

    // rdma
    PD_SYMBOL_LOAD(PD_FUNC_ID_RXDMA_TABLE_ADD, pd_rxdma_table_entry_add);
    PD_SYMBOL_LOAD(PD_FUNC_ID_TXDMA_TABLE_ADD, pd_txdma_table_entry_add);

    // lif
    PD_SYMBOL_LOAD(PD_FUNC_ID_LIF_GET_LPORTID, pd_lif_get_lport_id);

    // p4pt
    PD_SYMBOL_LOAD(PD_FUNC_ID_P4PT_INIT, p4pt_pd_init);

    // eth
    PD_SYMBOL_LOAD(PD_FUNC_ID_RSS_PARAMS_TABLE_ADD, pd_rss_params_table_entry_add);
    PD_SYMBOL_LOAD(PD_FUNC_ID_RSS_INDIR_TABLE_ADD, pd_rss_indir_table_entry_add);

    // asic
    PD_SYMBOL_LOAD(PD_FUNC_ID_ASIC_INIT, pd_asic_init);

    // capri
    PD_SYMBOL_LOAD(PD_FUNC_ID_GET_START_OFFSET, pd_get_start_offset);
    PD_SYMBOL_LOAD(PD_FUNC_ID_GET_REG_SIZE, pd_get_size_kb);
    PD_SYMBOL_LOAD(PD_FUNC_ID_PUSH_QSTATE, pd_push_qstate_to_capri);
    PD_SYMBOL_LOAD(PD_FUNC_ID_CLEAR_QSTATE, pd_clear_qstate);
    PD_SYMBOL_LOAD(PD_FUNC_ID_READ_QSTATE, pd_read_qstate);
    PD_SYMBOL_LOAD(PD_FUNC_ID_WRITE_QSTATE, pd_write_qstate);
    PD_SYMBOL_LOAD(PD_FUNC_ID_GET_PC_OFFSET, pd_get_pc_offset);
    PD_SYMBOL_LOAD(PD_FUNC_ID_HBM_READ, pd_capri_hbm_read_mem);
    PD_SYMBOL_LOAD(PD_FUNC_ID_HBM_WRITE, pd_capri_hbm_write_mem);
    PD_SYMBOL_LOAD(PD_FUNC_ID_PROG_LBL_TO_OFFSET, pd_capri_program_label_to_offset);
    PD_SYMBOL_LOAD(PD_FUNC_ID_PXB_CFG_LIF_BDF, pd_capri_pxb_cfg_lif_bdf);
    PD_SYMBOL_LOAD(PD_FUNC_ID_PROG_TO_BASE_ADDR, pd_capri_program_to_base_addr);
    PD_SYMBOL_LOAD(PD_FUNC_ID_BARCO_ASYM_REQ_DSC_GET, pd_capri_barco_asym_req_descr_get);
    PD_SYMBOL_LOAD(PD_FUNC_ID_BARCO_SYM_REQ_DSC_GET, pd_capri_barco_symm_req_descr_get);
    PD_SYMBOL_LOAD(PD_FUNC_ID_BARCO_RING_META_GET, pd_capri_barco_ring_meta_get);
    PD_SYMBOL_LOAD(PD_FUNC_ID_BARCO_ASYM_ECC_MUL_P256, pd_capri_barco_asym_ecc_point_mul_p256);
    PD_SYMBOL_LOAD(PD_FUNC_ID_BARCO_ASYM_ECDSA_P256_SIG_GEN, pd_capri_barco_asym_ecdsa_p256_sig_gen);
    PD_SYMBOL_LOAD(PD_FUNC_ID_BARCO_ASYM_ECDSA_P256_SIG_VER, pd_capri_barco_asym_ecdsa_p256_sig_verify);
    PD_SYMBOL_LOAD(PD_FUNC_ID_BARCO_ASYM_RSA2K_ENCRYPT, pd_capri_barco_asym_rsa2k_encrypt);
    PD_SYMBOL_LOAD(PD_FUNC_ID_BARCO_ASYM_RSA2K_DECRYPT, pd_capri_barco_asym_rsa2k_decrypt);
    PD_SYMBOL_LOAD(PD_FUNC_ID_BARCO_ASYM_RSA2K_CRT_DECRYPT, pd_capri_barco_asym_rsa2k_crt_decrypt);
    PD_SYMBOL_LOAD(PD_FUNC_ID_BARCO_ASYM_RSA2K_SIG_GEN, pd_capri_barco_asym_rsa2k_sig_gen);
    PD_SYMBOL_LOAD(PD_FUNC_ID_BARCO_ASYM_RSA2K_SIG_VERIFY, pd_capri_barco_asym_rsa2k_sig_verify);
    PD_SYMBOL_LOAD(PD_FUNC_ID_BARCO_SYM_HASH_PROC_REQ, pd_capri_barco_sym_hash_process_request);

    // hw clock to sw clock conversion api
    PD_SYMBOL_LOAD(PD_FUNC_ID_CONV_HW_CLOCK_TO_SW_CLOCK, pd_conv_hw_clock_to_sw_clock);
    PD_SYMBOL_LOAD(PD_FUNC_ID_CONV_SW_CLOCK_TO_HW_CLOCK, pd_conv_sw_clock_to_hw_clock);
    PD_SYMBOL_LOAD(PD_FUNC_ID_CLOCK_DELTA_COMP, pd_clock_delta_comp);

    // gft
    PD_SYMBOL_LOAD(PD_FUNC_ID_GFT_EXACT_MATCH_PROFILE_CREATE,
                   pd_gft_exact_match_profile_create);
    PD_SYMBOL_LOAD(PD_FUNC_ID_GFT_HDR_TRANSPOSITION_PROFILE_CREATE,
                   pd_gft_hdr_group_xposition_profile_create);
    PD_SYMBOL_LOAD(PD_FUNC_ID_GFT_EXACT_MATCH_FLOW_ENTRY_CREATE,
                   pd_gft_exact_match_flow_entry_create);

    // slab
    PD_SYMBOL_LOAD(PD_FUNC_ID_GET_SLAB, pd_get_slab);

    // swphv
    PD_SYMBOL_LOAD(PD_FUNC_ID_SWPHV_INJECT, pd_swphv_inject);
    PD_SYMBOL_LOAD(PD_FUNC_ID_SWPHV_GET_STATE, pd_swphv_get_state);

    return ret;
}

#define PD_SYMBOL_CALL(PD_FUNC_ID, NAME)                                      \
{                                                                             \
    if (pd_func_id == PD_FUNC_ID) {                                           \
        ret = g_pd_calls[pd_func_id].NAME((NAME ## _args_t *)args);           \
        return ret;                                                           \
    }                                                                         \
}

#define PD_SYMBOL_ARGS_CALL(PD_FUNC_ID, NAME, ARGS)                           \
{                                                                             \
    if (pd_func_id == PD_FUNC_ID) {                                           \
        ret = g_pd_calls[pd_func_id].NAME((ARGS ## _args_t *)args);           \
        return ret;                                                           \
    }                                                                         \
}

hal_ret_t
hal_pd_call (pd_func_id_t pd_func_id, void *args) 
{
    hal_ret_t       ret = HAL_RET_OK;

    // init pd calls
    PD_SYMBOL_CALL(PD_FUNC_ID_MEM_INIT, pd_mem_init);
    PD_SYMBOL_CALL(PD_FUNC_ID_MEM_INIT_PHASE2, pd_mem_init_phase2);
    PD_SYMBOL_CALL(PD_FUNC_ID_PGM_DEF_ENTRIES, pd_pgm_def_entries);
    PD_SYMBOL_CALL(PD_FUNC_ID_PGM_DEF_P4PLUS_ENTRIES, pd_pgm_def_p4plus_entries);

    // vrf pd calls
    PD_SYMBOL_CALL(PD_FUNC_ID_VRF_CREATE, pd_vrf_create);
    PD_SYMBOL_CALL(PD_FUNC_ID_VRF_DELETE, pd_vrf_delete);
    PD_SYMBOL_CALL(PD_FUNC_ID_VRF_UPDATE, pd_vrf_update);
    PD_SYMBOL_CALL(PD_FUNC_ID_VRF_MEM_FREE, pd_vrf_mem_free);
    PD_SYMBOL_CALL(PD_FUNC_ID_VRF_MAKE_CLONE, pd_vrf_make_clone);

    // l2seg pd calls
    PD_SYMBOL_CALL(PD_FUNC_ID_L2SEG_CREATE, pd_l2seg_create);
    PD_SYMBOL_CALL(PD_FUNC_ID_L2SEG_DELETE, pd_l2seg_delete);
    PD_SYMBOL_CALL(PD_FUNC_ID_L2SEG_UPDATE, pd_l2seg_update);
    PD_SYMBOL_CALL(PD_FUNC_ID_L2SEG_MEM_FREE, pd_l2seg_mem_free);
    PD_SYMBOL_CALL(PD_FUNC_ID_L2SEG_MAKE_CLONE, pd_l2seg_make_clone);
    PD_SYMBOL_CALL(PD_FUNC_ID_FIND_L2SEG_BY_HWID, pd_find_l2seg_by_hwid);

    PD_SYMBOL_CALL(PD_FUNC_ID_GET_OBJ_FROM_FLOW_LKPID, pd_get_object_from_flow_lkupid);
    PD_SYMBOL_CALL(PD_FUNC_ID_L2SEG_GET_FLOW_LKPID, pd_l2seg_get_flow_lkupid);
    PD_SYMBOL_CALL(PD_FUNC_ID_VRF_GET_FLOW_LKPID, pd_vrf_get_lookup_id);
    PD_SYMBOL_CALL(PD_FUNC_ID_L2SEG_GET_FRCPU_VLANID, pd_l2seg_get_fromcpu_vlanid);
    PD_SYMBOL_CALL(PD_FUNC_ID_VRF_GET_FRCPU_VLANID, pd_vrf_get_fromcpu_vlanid);

    // nwsec_profile pd calls
    PD_SYMBOL_CALL(PD_FUNC_ID_NWSEC_PROF_CREATE, pd_nwsec_profile_create);
    PD_SYMBOL_CALL(PD_FUNC_ID_NWSEC_PROF_DELETE, pd_nwsec_profile_delete);
    PD_SYMBOL_CALL(PD_FUNC_ID_NWSEC_PROF_UPDATE, pd_nwsec_profile_update);
    PD_SYMBOL_CALL(PD_FUNC_ID_NWSEC_PROF_MEM_FREE, pd_nwsec_profile_mem_free);
    PD_SYMBOL_CALL(PD_FUNC_ID_NWSEC_PROF_MAKE_CLONE, pd_nwsec_profile_make_clone);

    // dos policy pd calls
    PD_SYMBOL_CALL(PD_FUNC_ID_DOS_POLICY_CREATE, pd_dos_policy_create);
    PD_SYMBOL_CALL(PD_FUNC_ID_DOS_POLICY_DELETE, pd_dos_policy_delete);
    PD_SYMBOL_CALL(PD_FUNC_ID_DOS_POLICY_UPDATE, pd_dos_policy_update);
    PD_SYMBOL_CALL(PD_FUNC_ID_DOS_POLICY_MEM_FREE, pd_dos_policy_mem_free);
    PD_SYMBOL_CALL(PD_FUNC_ID_DOS_POLICY_MAKE_CLONE, pd_dos_policy_make_clone);

    // lif pd calls
    PD_SYMBOL_CALL(PD_FUNC_ID_LIF_CREATE, pd_lif_create);
    PD_SYMBOL_CALL(PD_FUNC_ID_LIF_DELETE, pd_lif_delete);
    PD_SYMBOL_CALL(PD_FUNC_ID_LIF_UPDATE, pd_lif_update);
    PD_SYMBOL_CALL(PD_FUNC_ID_LIF_MEM_FREE, pd_lif_mem_free);
    PD_SYMBOL_CALL(PD_FUNC_ID_LIF_MAKE_CLONE, pd_lif_make_clone);
    PD_SYMBOL_CALL(PD_FUNC_ID_LIF_GET, pd_lif_get);

    // if pd calls
    PD_SYMBOL_CALL(PD_FUNC_ID_IF_CREATE, pd_if_create);
    PD_SYMBOL_CALL(PD_FUNC_ID_IF_DELETE, pd_if_delete);
    PD_SYMBOL_CALL(PD_FUNC_ID_IF_UPDATE, pd_if_update);
    PD_SYMBOL_CALL(PD_FUNC_ID_IF_MEM_FREE, pd_if_mem_free);
    PD_SYMBOL_CALL(PD_FUNC_ID_IF_MAKE_CLONE, pd_if_make_clone);
    PD_SYMBOL_CALL(PD_FUNC_ID_IF_NWSEC_UPDATE, pd_if_nwsec_update);
    PD_SYMBOL_CALL(PD_FUNC_ID_IF_LIF_UPDATE, pd_if_lif_update);
    PD_SYMBOL_CALL(PD_FUNC_ID_IF_GET, pd_if_get);

    // ep pd calls
    PD_SYMBOL_CALL(PD_FUNC_ID_EP_CREATE, pd_ep_create);
    PD_SYMBOL_CALL(PD_FUNC_ID_EP_DELETE, pd_ep_delete);
    PD_SYMBOL_CALL(PD_FUNC_ID_EP_UPDATE, pd_ep_update);
    PD_SYMBOL_CALL(PD_FUNC_ID_EP_MEM_FREE, pd_ep_mem_free);
    PD_SYMBOL_CALL(PD_FUNC_ID_EP_MAKE_CLONE, pd_ep_make_clone);

    // session pd calls
    PD_SYMBOL_CALL(PD_FUNC_ID_SESSION_CREATE, pd_session_create);
    PD_SYMBOL_CALL(PD_FUNC_ID_SESSION_DELETE, pd_session_delete);
    PD_SYMBOL_CALL(PD_FUNC_ID_SESSION_UPDATE, pd_session_update);
    PD_SYMBOL_CALL(PD_FUNC_ID_SESSION_GET, pd_session_get);

    // tlscb pd calls
    PD_SYMBOL_CALL(PD_FUNC_ID_TLSCB_CREATE, pd_tlscb_create);
    PD_SYMBOL_CALL(PD_FUNC_ID_TLSCB_DELETE, pd_tlscb_delete);
    PD_SYMBOL_CALL(PD_FUNC_ID_TLSCB_UPDATE, pd_tlscb_update);
    PD_SYMBOL_CALL(PD_FUNC_ID_TLSCB_GET, pd_tlscb_get);

    // tcpcb pd calls
    PD_SYMBOL_CALL(PD_FUNC_ID_TCPCB_CREATE, pd_tcpcb_create);
    PD_SYMBOL_CALL(PD_FUNC_ID_TCPCB_DELETE, pd_tcpcb_delete);
    PD_SYMBOL_CALL(PD_FUNC_ID_TCPCB_UPDATE, pd_tcpcb_update);
    PD_SYMBOL_CALL(PD_FUNC_ID_TCPCB_GET, pd_tcpcb_get);

    // ipseccb pd calls
    PD_SYMBOL_CALL(PD_FUNC_ID_IPSECCB_CREATE, pd_ipseccb_create);
    PD_SYMBOL_CALL(PD_FUNC_ID_IPSECCB_DELETE, pd_ipseccb_delete);
    PD_SYMBOL_CALL(PD_FUNC_ID_IPSECCB_UPDATE, pd_ipseccb_update);
    PD_SYMBOL_CALL(PD_FUNC_ID_IPSECCB_GET, pd_ipseccb_get);

    // ipseccb_decrypt pd calls
    PD_SYMBOL_CALL(PD_FUNC_ID_IPSECCB_DECRYPT_CREATE, pd_ipseccb_decrypt_create);
    PD_SYMBOL_CALL(PD_FUNC_ID_IPSECCB_DECRYPT_DELETE, pd_ipseccb_decrypt_delete);
    PD_SYMBOL_CALL(PD_FUNC_ID_IPSECCB_DECRYPT_UPDATE, pd_ipseccb_decrypt_update);
    PD_SYMBOL_CALL(PD_FUNC_ID_IPSECCB_DECRYPT_GET, pd_ipseccb_decrypt_get);

    // l4lb
    PD_SYMBOL_CALL(PD_FUNC_ID_L4LB_CREATE, pd_l4lb_create);

    // cpucb
    PD_SYMBOL_CALL(PD_FUNC_ID_CPUCB_CREATE, pd_cpucb_create);
    PD_SYMBOL_CALL(PD_FUNC_ID_CPUCB_DELETE, pd_cpucb_delete);
    PD_SYMBOL_CALL(PD_FUNC_ID_CPUCB_UPDATE, pd_cpucb_update);
    PD_SYMBOL_CALL(PD_FUNC_ID_CPUCB_GET, pd_cpucb_get);

    // rawrcb
    PD_SYMBOL_CALL(PD_FUNC_ID_RAWRCB_CREATE, pd_rawrcb_create);
    PD_SYMBOL_CALL(PD_FUNC_ID_RAWRCB_DELETE, pd_rawrcb_delete);
    PD_SYMBOL_CALL(PD_FUNC_ID_RAWRCB_UPDATE, pd_rawrcb_update);
    PD_SYMBOL_CALL(PD_FUNC_ID_RAWRCB_GET, pd_rawrcb_get);


    // rawccb
    PD_SYMBOL_CALL(PD_FUNC_ID_RAWCCB_CREATE, pd_rawccb_create);
    PD_SYMBOL_CALL(PD_FUNC_ID_RAWCCB_DELETE, pd_rawccb_delete);
    PD_SYMBOL_CALL(PD_FUNC_ID_RAWCCB_UPDATE, pd_rawccb_update);
    PD_SYMBOL_CALL(PD_FUNC_ID_RAWCCB_GET, pd_rawccb_get);

    // proxyrcb
    PD_SYMBOL_CALL(PD_FUNC_ID_PROXYRCB_CREATE, pd_proxyrcb_create);
    PD_SYMBOL_CALL(PD_FUNC_ID_PROXYRCB_DELETE, pd_proxyrcb_delete);
    PD_SYMBOL_CALL(PD_FUNC_ID_PROXYRCB_UPDATE, pd_proxyrcb_update);
    PD_SYMBOL_CALL(PD_FUNC_ID_PROXYRCB_GET, pd_proxyrcb_get);

    // proxyccb
    PD_SYMBOL_CALL(PD_FUNC_ID_PROXYCCB_CREATE, pd_proxyccb_create);
    PD_SYMBOL_CALL(PD_FUNC_ID_PROXYCCB_DELETE, pd_proxyccb_delete);
    PD_SYMBOL_CALL(PD_FUNC_ID_PROXYCCB_UPDATE, pd_proxyccb_update);
    PD_SYMBOL_CALL(PD_FUNC_ID_PROXYCCB_GET, pd_proxyccb_get);

    // qos class
    PD_SYMBOL_CALL(PD_FUNC_ID_QOS_CLASS_CREATE, pd_qos_class_create);
    PD_SYMBOL_CALL(PD_FUNC_ID_QOS_CLASS_DELETE, pd_qos_class_delete);
    PD_SYMBOL_CALL(PD_FUNC_ID_QOS_CLASS_UPDATE, pd_qos_class_update);
    PD_SYMBOL_CALL(PD_FUNC_ID_QOS_CLASS_MAKE_CLONE, pd_qos_class_make_clone);
    PD_SYMBOL_CALL(PD_FUNC_ID_QOS_CLASS_MEM_FREE, pd_qos_class_mem_free);

    // copp
    PD_SYMBOL_CALL(PD_FUNC_ID_COPP_CREATE, pd_copp_create);
    PD_SYMBOL_CALL(PD_FUNC_ID_COPP_DELETE, pd_copp_delete);
    PD_SYMBOL_CALL(PD_FUNC_ID_COPP_UPDATE, pd_copp_update);
    PD_SYMBOL_CALL(PD_FUNC_ID_COPP_MAKE_CLONE, pd_copp_make_clone);
    PD_SYMBOL_CALL(PD_FUNC_ID_COPP_MEM_FREE, pd_copp_mem_free);

    // acl
    PD_SYMBOL_CALL(PD_FUNC_ID_ACL_CREATE, pd_acl_create);
    PD_SYMBOL_CALL(PD_FUNC_ID_ACL_DELETE, pd_acl_delete);
    PD_SYMBOL_CALL(PD_FUNC_ID_ACL_UPDATE, pd_acl_update);
    PD_SYMBOL_CALL(PD_FUNC_ID_ACL_MEM_FREE, pd_acl_mem_free);
    PD_SYMBOL_CALL(PD_FUNC_ID_ACL_MAKE_CLONE, pd_acl_make_clone);

    // wring
    PD_SYMBOL_CALL(PD_FUNC_ID_WRING_CREATE, pd_wring_create);
    // PD_SYMBOL_CALL(PD_FUNC_ID_WRING_DELETE, pd_wring_delete);
    // PD_SYMBOL_CALL(PD_FUNC_ID_WRING_UPDATE, pd_wring_update);
    PD_SYMBOL_CALL(PD_FUNC_ID_WRING_GET_ENTRY, pd_wring_get_entry);
    PD_SYMBOL_CALL(PD_FUNC_ID_WRING_GET_META, pd_wring_get_meta);
    PD_SYMBOL_CALL(PD_FUNC_ID_WRING_SET_META, pd_wring_set_meta);

    // mirror session
    PD_SYMBOL_CALL(PD_FUNC_ID_MIRROR_SESSION_CREATE, pd_mirror_session_create);
    PD_SYMBOL_CALL(PD_FUNC_ID_MIRROR_SESSION_DELETE, pd_mirror_session_delete);
    PD_SYMBOL_CALL(PD_FUNC_ID_MIRROR_SESSION_GET, pd_mirror_session_get);

    // collector
    PD_SYMBOL_CALL(PD_FUNC_ID_COLLECTOR_CREATE, pd_collector_create);

    // mc entry
    PD_SYMBOL_CALL(PD_FUNC_ID_MC_ENTRY_CREATE, pd_mc_entry_create);
    PD_SYMBOL_CALL(PD_FUNC_ID_MC_ENTRY_DELETE, pd_mc_entry_delete);
    // PD_SYMBOL_CALL(PD_FUNC_ID_MC_ENTRY_UPDATE, pd_mc_entry_update);

    // flow get
    PD_SYMBOL_CALL(PD_FUNC_ID_FLOW_GET, pd_flow_get);

    // l2seg-uplink
    PD_SYMBOL_CALL(PD_FUNC_ID_ADD_L2SEG_UPLINK, pd_add_l2seg_uplink);
    PD_SYMBOL_CALL(PD_FUNC_ID_DEL_L2SEG_UPLINK, pd_del_l2seg_uplink);

    // debug cli
    PD_SYMBOL_CALL(PD_FUNC_ID_DEBUG_CLI_READ, pd_debug_cli_read);
    PD_SYMBOL_CALL(PD_FUNC_ID_DEBUG_CLI_WRITE, pd_debug_cli_write);
    PD_SYMBOL_CALL(PD_FUNC_ID_MPU_TRACE_ENABLE, pd_mpu_trace_enable);

    // apis
    PD_SYMBOL_CALL(PD_FUNC_ID_IF_GET_HW_LIF_ID, pd_if_get_hw_lif_id);
    PD_SYMBOL_CALL(PD_FUNC_ID_IF_GET_LPORT_ID, pd_if_get_lport_id);
    PD_SYMBOL_CALL(PD_FUNC_ID_IF_GET_TM_OPORT, pd_if_get_tm_oport);

    // twice nat
    PD_SYMBOL_CALL(PD_FUNC_ID_RWENTRY_FIND_OR_ALLOC, pd_rw_entry_find_or_alloc);
    PD_SYMBOL_CALL(PD_FUNC_ID_TWICE_NAT_ADD, pd_twice_nat_add);
    PD_SYMBOL_CALL(PD_FUNC_ID_TWICE_NAT_DEL, pd_twice_nat_del);

    // qos
    PD_SYMBOL_CALL(PD_FUNC_ID_GET_QOS_CLASSID, pd_qos_class_get_qos_class_id);
    PD_SYMBOL_CALL(PD_FUNC_ID_QOS_GET_ADMIN_COS, pd_qos_class_get_admin_cos);

    // aol
    PD_SYMBOL_CALL(PD_FUNC_ID_DESC_AOL_GET, pd_descriptor_aol_get);

    // crypto
    PD_SYMBOL_CALL(PD_FUNC_ID_CRYPTO_ALLOC_KEY, pd_crypto_alloc_key);
    PD_SYMBOL_CALL(PD_FUNC_ID_CRYPTO_FREE_KEY, pd_crypto_free_key);
    PD_SYMBOL_CALL(PD_FUNC_ID_CRYPTO_WRITE_KEY, pd_crypto_write_key);
    PD_SYMBOL_CALL(PD_FUNC_ID_CRYPTO_READ_KEY, pd_crypto_read_key);
    PD_SYMBOL_CALL(PD_FUNC_ID_CRYPTO_ASYM_ALLOC_KEY, pd_crypto_asym_alloc_key);
    PD_SYMBOL_CALL(PD_FUNC_ID_CRYPTO_ASYM_FREE_KEY, pd_crypto_asym_free_key);
    PD_SYMBOL_CALL(PD_FUNC_ID_CRYPTO_ASYM_WRITE_KEY, pd_crypto_asym_write_key);
    PD_SYMBOL_CALL(PD_FUNC_ID_CRYPTO_ASYM_READ_KEY, pd_crypto_asym_read_key);

    // barco
    PD_SYMBOL_CALL(PD_FUNC_ID_OPAQUE_TAG_ADDR, pd_get_opaque_tag_addr);

    // stats
    PD_SYMBOL_CALL(PD_FUNC_ID_DROP_STATS_GET, pd_drop_stats_get);
    PD_SYMBOL_CALL(PD_FUNC_ID_TABLE_STATS_GET, pd_table_stats_get);
    PD_SYMBOL_CALL(PD_FUNC_ID_SYSTEM_DROP_ACTION, pd_system_drop_action);

    // oifl
    PD_SYMBOL_CALL(PD_FUNC_ID_OIFL_CREATE, pd_oif_list_create);
    PD_SYMBOL_CALL(PD_FUNC_ID_OIFL_CREATE_BLOCK, pd_oif_list_create_block);
    PD_SYMBOL_CALL(PD_FUNC_ID_OIFL_DELETE, pd_oif_list_delete);
    PD_SYMBOL_CALL(PD_FUNC_ID_OIFL_DELETE_BLOCK, pd_oif_list_delete_block);
    PD_SYMBOL_CALL(PD_FUNC_ID_OIFL_ADD_OIF, pd_oif_list_add_oif);
    PD_SYMBOL_CALL(PD_FUNC_ID_OIFL_ADD_QP_OIF, pd_oif_list_add_qp_oif);
    PD_SYMBOL_CALL(PD_FUNC_ID_OIFL_REM_OIF, pd_oif_list_remove_oif);
    PD_SYMBOL_CALL(PD_FUNC_ID_OIFL_IS_MEMBER, pd_oif_list_is_member);
    PD_SYMBOL_CALL(PD_FUNC_ID_GET_NUM_OIFS, pd_oif_list_get_num_oifs);
    PD_SYMBOL_CALL(PD_FUNC_ID_GET_OIF_ARRAY, pd_oif_list_get_oif_array);
    PD_SYMBOL_CALL(PD_FUNC_ID_SET_HONOR_ING, pd_oif_list_set_honor_ingress);
    PD_SYMBOL_CALL(PD_FUNC_ID_CLR_HONOR_ING, pd_oif_list_clr_honor_ingress);

    // tunnel if
    PD_SYMBOL_CALL(PD_FUNC_ID_TNNL_IF_GET_RW_IDX, pd_tunnelif_get_rw_idx);

    // cpu
    PD_SYMBOL_CALL(PD_FUNC_ID_CPU_ALLOC_INIT, pd_cpupkt_ctxt_alloc_init);
    PD_SYMBOL_CALL(PD_FUNC_ID_CPU_REG_RXQ, pd_cpupkt_register_rx_queue);
    PD_SYMBOL_CALL(PD_FUNC_ID_CPU_REG_TXQ, pd_cpupkt_register_tx_queue);
    PD_SYMBOL_CALL(PD_FUNC_ID_CPU_UNREG_TXQ, pd_cpupkt_unregister_tx_queue);
    PD_SYMBOL_CALL(PD_FUNC_ID_CPU_POLL_REC, pd_cpupkt_poll_receive);
    PD_SYMBOL_CALL(PD_FUNC_ID_CPU_FREE, pd_cpupkt_free);
    PD_SYMBOL_CALL(PD_FUNC_ID_CPU_SEND, pd_cpupkt_send);
    PD_SYMBOL_CALL(PD_FUNC_ID_CPU_PAGE_ALLOC, pd_cpupkt_page_alloc);
    PD_SYMBOL_CALL(PD_FUNC_ID_CPU_DESCR_ALLOC, pd_cpupkt_descr_alloc);
    PD_SYMBOL_CALL(PD_FUNC_ID_PGM_SEND_RING_DBELL, pd_cpupkt_program_send_ring_doorbell);

    // rdma
    PD_SYMBOL_CALL(PD_FUNC_ID_RXDMA_TABLE_ADD, pd_rxdma_table_entry_add);
    PD_SYMBOL_CALL(PD_FUNC_ID_TXDMA_TABLE_ADD, pd_txdma_table_entry_add);

    // lif
    PD_SYMBOL_CALL(PD_FUNC_ID_LIF_GET_LPORTID, pd_lif_get_lport_id);

    // p4pt
    PD_SYMBOL_CALL(PD_FUNC_ID_P4PT_INIT, p4pt_pd_init);

    // eth
    PD_SYMBOL_CALL(PD_FUNC_ID_RSS_PARAMS_TABLE_ADD, pd_rss_params_table_entry_add);
    PD_SYMBOL_CALL(PD_FUNC_ID_RSS_INDIR_TABLE_ADD, pd_rss_indir_table_entry_add);

    // asic
    PD_SYMBOL_CALL(PD_FUNC_ID_ASIC_INIT, pd_asic_init);

    // capri
    PD_SYMBOL_CALL(PD_FUNC_ID_GET_START_OFFSET, pd_get_start_offset);
    PD_SYMBOL_CALL(PD_FUNC_ID_GET_REG_SIZE, pd_get_size_kb);
    PD_SYMBOL_CALL(PD_FUNC_ID_PUSH_QSTATE, pd_push_qstate_to_capri);
    PD_SYMBOL_CALL(PD_FUNC_ID_CLEAR_QSTATE, pd_clear_qstate);
    PD_SYMBOL_CALL(PD_FUNC_ID_READ_QSTATE, pd_read_qstate);
    PD_SYMBOL_CALL(PD_FUNC_ID_WRITE_QSTATE, pd_write_qstate);
    PD_SYMBOL_CALL(PD_FUNC_ID_GET_PC_OFFSET, pd_get_pc_offset);
    PD_SYMBOL_CALL(PD_FUNC_ID_HBM_READ, pd_capri_hbm_read_mem);
    PD_SYMBOL_CALL(PD_FUNC_ID_HBM_WRITE, pd_capri_hbm_write_mem);
    PD_SYMBOL_CALL(PD_FUNC_ID_PROG_LBL_TO_OFFSET, pd_capri_program_label_to_offset);
    PD_SYMBOL_CALL(PD_FUNC_ID_PXB_CFG_LIF_BDF, pd_capri_pxb_cfg_lif_bdf);
    PD_SYMBOL_CALL(PD_FUNC_ID_PROG_TO_BASE_ADDR, pd_capri_program_to_base_addr);
    PD_SYMBOL_CALL(PD_FUNC_ID_BARCO_ASYM_REQ_DSC_GET, pd_capri_barco_asym_req_descr_get);
    PD_SYMBOL_CALL(PD_FUNC_ID_BARCO_SYM_REQ_DSC_GET, pd_capri_barco_symm_req_descr_get);
    PD_SYMBOL_CALL(PD_FUNC_ID_BARCO_RING_META_GET, pd_capri_barco_ring_meta_get);
    PD_SYMBOL_CALL(PD_FUNC_ID_BARCO_ASYM_ECC_MUL_P256, pd_capri_barco_asym_ecc_point_mul_p256);
    PD_SYMBOL_CALL(PD_FUNC_ID_BARCO_ASYM_ECDSA_P256_SIG_GEN, pd_capri_barco_asym_ecdsa_p256_sig_gen);
    PD_SYMBOL_CALL(PD_FUNC_ID_BARCO_ASYM_ECDSA_P256_SIG_VER, pd_capri_barco_asym_ecdsa_p256_sig_verify);
    PD_SYMBOL_CALL(PD_FUNC_ID_BARCO_ASYM_RSA2K_ENCRYPT, pd_capri_barco_asym_rsa2k_encrypt);
    PD_SYMBOL_CALL(PD_FUNC_ID_BARCO_ASYM_RSA2K_DECRYPT, pd_capri_barco_asym_rsa2k_decrypt);
    PD_SYMBOL_CALL(PD_FUNC_ID_BARCO_ASYM_RSA2K_CRT_DECRYPT, pd_capri_barco_asym_rsa2k_crt_decrypt);
    PD_SYMBOL_CALL(PD_FUNC_ID_BARCO_ASYM_RSA2K_SIG_GEN, pd_capri_barco_asym_rsa2k_sig_gen);
    PD_SYMBOL_CALL(PD_FUNC_ID_BARCO_ASYM_RSA2K_SIG_VERIFY, pd_capri_barco_asym_rsa2k_sig_verify);
    PD_SYMBOL_CALL(PD_FUNC_ID_BARCO_SYM_HASH_PROC_REQ, pd_capri_barco_sym_hash_process_request);


    //clock
    PD_SYMBOL_CALL(PD_FUNC_ID_CONV_HW_CLOCK_TO_SW_CLOCK, pd_conv_hw_clock_to_sw_clock);
    PD_SYMBOL_CALL(PD_FUNC_ID_CONV_SW_CLOCK_TO_HW_CLOCK, pd_conv_sw_clock_to_hw_clock);
    PD_SYMBOL_CALL(PD_FUNC_ID_CLOCK_DELTA_COMP, pd_clock_delta_comp);

    // gft
    PD_SYMBOL_ARGS_CALL(PD_FUNC_ID_GFT_EXACT_MATCH_PROFILE_CREATE,
                        pd_gft_exact_match_profile_create,
                        pd_gft_exact_match_profile);
    PD_SYMBOL_ARGS_CALL(PD_FUNC_ID_GFT_HDR_TRANSPOSITION_PROFILE_CREATE,
                        pd_gft_hdr_group_xposition_profile_create,
                        pd_gft_hdr_group_xposition_profile);
    PD_SYMBOL_ARGS_CALL(PD_FUNC_ID_GFT_EXACT_MATCH_FLOW_ENTRY_CREATE,
                        pd_gft_exact_match_flow_entry_create,
                        pd_gft_exact_match_flow_entry);

    // slab
    PD_SYMBOL_CALL(PD_FUNC_ID_GET_SLAB, pd_get_slab);

    // swphv
    PD_SYMBOL_CALL(PD_FUNC_ID_SWPHV_INJECT, pd_swphv_inject);
    PD_SYMBOL_CALL(PD_FUNC_ID_SWPHV_GET_STATE, pd_swphv_get_state);

    HAL_ASSERT(0);
    return ret;
}

hal_ret_t
hal_pd_libopen (hal_cfg_t *hal_cfg)
{
    hal_ret_t   ret         = HAL_RET_OK;
    std::string feature_set = std::string(hal_cfg->feature_set);
    std::string feature_pd_stub = "pd_stub";
    std::string pdlib_path, pdlib_stub_path;

    pdlib_path = hal_cfg->cfg_path + "/" + feature_set + "/";
    pdlib_stub_path = hal_cfg->cfg_path + "/" + feature_pd_stub + "/";
    pdlib_path += "lib" + feature_set + ".so";
    pdlib_stub_path += "lib" + feature_pd_stub + ".so";

    HAL_TRACE_DEBUG("Loading pd lib: {}", pdlib_path);

    // with deepbind, its taking the symbol the PD is taking from PI
    // void *so = dlopen(pdlib_path.c_str(), RTLD_NOW|RTLD_GLOBAL|RTLD_DEEPBIND);
    hal_cfg->pd_so = dlopen(pdlib_path.c_str(), RTLD_NOW|RTLD_GLOBAL);
    if (!hal_cfg->pd_so) {
        HAL_TRACE_ERR("dlopen failed {}:{}", pdlib_path, dlerror());
        HAL_ASSERT(0);
    }

    // open PD stub library
    HAL_TRACE_DEBUG("Loading pd stub lib: {}", pdlib_stub_path);
    hal_cfg->pd_stub_so = dlopen(pdlib_stub_path.c_str(), RTLD_NOW|RTLD_GLOBAL);
    if (!hal_cfg->pd_stub_so) {
        HAL_TRACE_ERR("{} dlopen failed {}", pdlib_stub_path, dlerror());
        HAL_ASSERT(0);
    }

    return ret;
}

//------------------------------------------------------------------------------
// PD init routine to
// - start USD thread that inits the ASIC, which will then start ASIC RW thread
// TODO: for now we direcly spawn ASIC RW thread from here !!
//------------------------------------------------------------------------------
hal_ret_t
hal_pd_init (hal_cfg_t *hal_cfg)
{
    hal_ret_t                           ret;
    pd_mem_init_args_t                  mem_init_args;
    pd_mem_init_phase2_args_t           ph2_args;
    pd_pgm_def_entries_args_t           pgm_def_args;
    pd_pgm_def_p4plus_entries_args_t    pgm_p4p_args;
    pd_clock_delta_comp_args_t          clock_args;

    HAL_ASSERT(hal_cfg != NULL);

    // open pd libs
    ret = hal_pd_libopen(hal_cfg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("HAL PD lib open failed, err : {}", ret);
        goto cleanup;
    }

    // load pd symbols
    ret = hal_pd_load_symbols(hal_cfg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("HAL PD lib load symbols failed, err : {}", ret);
        goto cleanup;
    }

    mem_init_args.cfg_path = hal_cfg->cfg_path.c_str();
    ret = hal_pd_call(PD_FUNC_ID_MEM_INIT, (void *)&mem_init_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("HAL PD init failed, err : {}", ret);
        goto cleanup;
    }

    // start the asic-rw thread
    HAL_TRACE_DEBUG("Starting asic-rw thread ...");
    g_hal_threads[HAL_THREAD_ID_ASIC_RW] =
        thread::factory(
                std::string("asic-rw").c_str(),
                HAL_THREAD_ID_ASIC_RW,
                sdk::lib::THREAD_ROLE_CONTROL,
                0x0 /* use all control cores */,
                hal::pd::asic_rw_start,
                sched_get_priority_max(SCHED_RR),
                gl_super_user ? SCHED_RR : SCHED_OTHER,
                true);
    HAL_ABORT(g_hal_threads[HAL_THREAD_ID_ASIC_RW] != NULL);

    // set custom data
    g_hal_threads[HAL_THREAD_ID_ASIC_RW]->set_data(hal_cfg);

    // invoke with thread instance reference
    g_hal_threads[HAL_THREAD_ID_ASIC_RW]->start(
                            g_hal_threads[HAL_THREAD_ID_ASIC_RW]);

    HAL_TRACE_DEBUG("Waiting for asic-rw thread to be ready ...");
    // wait for ASIC RW thread to be ready before initializing table entries
    while (!is_asic_rw_ready()) {
        pthread_yield();
    }

    ph2_args.cfg_path = mem_init_args.cfg_path;
    ph2_args.hal_cfg = hal_cfg;
    ret = hal_pd_call(PD_FUNC_ID_MEM_INIT_PHASE2, (void *)&ph2_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("HAL PD init failed, err : {}", ret);
        goto cleanup;
    }

    pgm_def_args.hal_cfg = hal_cfg;
    ret = hal_pd_call(PD_FUNC_ID_PGM_DEF_ENTRIES, (void *)&pgm_def_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("HAL Programming default entries, err : {}", ret);
        goto cleanup;
    }

    pgm_p4p_args.hal_cfg = hal_cfg;
    ret = hal_pd_call(PD_FUNC_ID_PGM_DEF_P4PLUS_ENTRIES, (void *)&pgm_p4p_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("HAL Programming default p4plus entries failed, err : {}", ret);
        goto cleanup;
    }

    HAL_ABORT(hal_pd_call(PD_FUNC_ID_CLOCK_DELTA_COMP, (void *)&clock_args) == HAL_RET_OK);

    return HAL_RET_OK;

cleanup:

    return ret;
}

extern "C" int 
pd_tls_asym_ecdsa_p256_sig_verify (uint8_t *p, uint8_t *n,
        uint8_t *xg, uint8_t *yg, uint8_t *a, uint8_t *b, uint8_t *xq,
        uint8_t *yq, uint8_t *r, uint8_t *s, uint8_t *h)
{
    hal_ret_t ret = HAL_RET_OK;
    pd_capri_barco_asym_ecdsa_p256_sig_verify_args_t args = {0};

    args.p = p;
    args.n = n;
    args.xg = xg;
    args.yg = yg;
    args.a = a;
    args.b = b;
    args.xq = xq;
    args.yq = yq;
    args.r = r;
    args.s = s;
    args.h = h;

    ret = hal_pd_call(PD_FUNC_ID_BARCO_ASYM_ECDSA_P256_SIG_VER, (void*)&args);
    if (ret != HAL_RET_OK) {
        return -1;
    }
    return 1;
}

extern "C" int 
pd_tls_asym_rsa2k_encrypt(uint8_t *n, uint8_t *e, uint8_t *m,  uint8_t *c)
{
    hal_ret_t ret = HAL_RET_OK;
    pd_capri_barco_asym_rsa2k_encrypt_args_t args = {0};
    args.n = n;
    args.e = e;
    args.m = m;
    args.c = c;

    ret = hal_pd_call(PD_FUNC_ID_BARCO_ASYM_RSA2K_ENCRYPT, (void*)&args);
    if (ret != HAL_RET_OK) {
        return -1;
    }
    return 1;
}

extern "C" int 
pd_tls_asym_rsa2k_sig_gen(uint8_t *n, uint8_t *d, uint8_t *h,  uint8_t *s)
{
    hal_ret_t ret = HAL_RET_OK;
    pd_capri_barco_asym_rsa2k_sig_gen_args_t args = {0};
    args.n = n;
    args.d = d;
    args.h = h;
    args.s = s;

    ret = hal_pd_call(PD_FUNC_ID_BARCO_ASYM_RSA2K_SIG_GEN, (void*)&args);
    if (ret != HAL_RET_OK) {
        return -1;
    }
    return 1;
}

}    // namespace pd
}    // namespace hal
