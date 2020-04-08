// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <string>
#include <dlfcn.h>
#include "nic/include/hal.hpp"
#include "nic/include/hal_pd.hpp"
#include "nic/include/asic_pd.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/sdk/asic/rw/asicrw.hpp"
#include "nic/hal/pd/pd_api.hpp"
#include "nic/hal/pd/pd_api_c.h"
#include "nic/sdk/lib/pal/pal.hpp"

using namespace sdk::asic::pd;

namespace hal {
namespace pd {

pd_func_t   *g_pd_funcs;
void        *g_pd_so;
void        *g_pd_stub_so;

hal_ret_t
hal_pd_stub_assert (pd_func_args_t *args)
{
    SDK_ASSERT(FALSE);
    return HAL_RET_OK;
}

#define PD_FUNC_LOAD(PD_FUNC_ID, NAME)                                         \
{                                                                              \
    g_pd_funcs[PD_FUNC_ID] =                                                   \
        (pd_func_t)dlsym(RTLD_DEFAULT, #NAME);                                 \
    dlsym_error = dlerror();                                                   \
    if (dlsym_error) {                                                         \
        HAL_TRACE_DEBUG("Failed to load symbol from PD lib {}:{}", #NAME,      \
                        dlsym_error);                                          \
        g_pd_funcs[PD_FUNC_ID] = hal_pd_stub_assert;                           \
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
//           SDK_ASSERT(0);
//       }
//   }
//------------------------------------------------------------------------------
static inline hal_ret_t
hal_pd_load_symbols (void)
{
    hal_ret_t       ret = HAL_RET_OK;
    const char*     dlsym_error = NULL;

    g_pd_funcs = (pd_func_t *)HAL_CALLOC(HAL_MEM_ALLOC_PD_FUNCS,
                                       PD_FUNC_ID_MAX * sizeof(pd_func_t));

    // init pd calls
    PD_FUNC_LOAD(PD_FUNC_ID_MEM_INIT, pd_mem_init);
    PD_FUNC_LOAD(PD_FUNC_ID_MEM_INIT_PHASE2, pd_mem_init_phase2);
    PD_FUNC_LOAD(PD_FUNC_ID_PGM_DEF_ENTRIES, pd_pgm_def_entries);
    PD_FUNC_LOAD(PD_FUNC_ID_PGM_DEF_P4PLUS_ENTRIES, pd_pgm_def_p4plus_entries);

    // vrf pd calls
    PD_FUNC_LOAD(PD_FUNC_ID_VRF_CREATE, pd_vrf_create);
    PD_FUNC_LOAD(PD_FUNC_ID_VRF_RESTORE, pd_vrf_restore);
    PD_FUNC_LOAD(PD_FUNC_ID_VRF_DELETE, pd_vrf_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_VRF_UPDATE, pd_vrf_update);
    PD_FUNC_LOAD(PD_FUNC_ID_VRF_MEM_FREE, pd_vrf_mem_free);
    PD_FUNC_LOAD(PD_FUNC_ID_VRF_MAKE_CLONE, pd_vrf_make_clone);
    PD_FUNC_LOAD(PD_FUNC_ID_VRF_GET, pd_vrf_get);

    // l2seg pd calls
    PD_FUNC_LOAD(PD_FUNC_ID_L2SEG_CREATE, pd_l2seg_create);
    PD_FUNC_LOAD(PD_FUNC_ID_L2SEG_RESTORE, pd_l2seg_restore);
    PD_FUNC_LOAD(PD_FUNC_ID_L2SEG_DELETE, pd_l2seg_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_L2SEG_UPDATE, pd_l2seg_update);
    PD_FUNC_LOAD(PD_FUNC_ID_L2SEG_MEM_FREE, pd_l2seg_mem_free);
    PD_FUNC_LOAD(PD_FUNC_ID_L2SEG_MAKE_CLONE, pd_l2seg_make_clone);
    PD_FUNC_LOAD(PD_FUNC_ID_L2SEG_GET, pd_l2seg_get);
    PD_FUNC_LOAD(PD_FUNC_ID_L2SEG_PIN_UPLINK_CHANGE, pd_l2seg_update_pinned_uplink);
    PD_FUNC_LOAD(PD_FUNC_ID_TEL_L2SEG_UPDATE, pd_tel_l2seg_update);

    // misc apis for vrf and l2seg
    PD_FUNC_LOAD(PD_FUNC_ID_GET_OBJ_FROM_FLOW_LKPID, pd_get_object_from_flow_lkupid);
    PD_FUNC_LOAD(PD_FUNC_ID_L2SEG_GET_FLOW_LKPID, pd_l2seg_get_flow_lkupid);
    PD_FUNC_LOAD(PD_FUNC_ID_VRF_GET_FLOW_LKPID, pd_vrf_get_lookup_id);
    PD_FUNC_LOAD(PD_FUNC_ID_L2SEG_GET_FRCPU_VLANID, pd_l2seg_get_fromcpu_vlanid);
    PD_FUNC_LOAD(PD_FUNC_ID_VRF_GET_FRCPU_VLANID, pd_vrf_get_fromcpu_vlanid);

    // nwsec profile pd calls
    PD_FUNC_LOAD(PD_FUNC_ID_NWSEC_PROF_CREATE, pd_nwsec_profile_create);
    PD_FUNC_LOAD(PD_FUNC_ID_NWSEC_PROF_DELETE, pd_nwsec_profile_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_NWSEC_PROF_UPDATE, pd_nwsec_profile_update);
    PD_FUNC_LOAD(PD_FUNC_ID_NWSEC_PROF_MEM_FREE, pd_nwsec_profile_mem_free);
    PD_FUNC_LOAD(PD_FUNC_ID_NWSEC_PROF_MAKE_CLONE, pd_nwsec_profile_make_clone);
    PD_FUNC_LOAD(PD_FUNC_ID_NWSEC_PROF_RESTORE, pd_nwsec_profile_restore);
    PD_FUNC_LOAD(PD_FUNC_ID_NWSEC_PROF_GET, pd_nwsec_profile_get);

    // dos policy pd calls
    PD_FUNC_LOAD(PD_FUNC_ID_DOS_POLICY_CREATE, pd_dos_policy_create);
    PD_FUNC_LOAD(PD_FUNC_ID_DOS_POLICY_DELETE, pd_dos_policy_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_DOS_POLICY_UPDATE, pd_dos_policy_update);
    PD_FUNC_LOAD(PD_FUNC_ID_DOS_POLICY_MEM_FREE, pd_dos_policy_mem_free);
    PD_FUNC_LOAD(PD_FUNC_ID_DOS_POLICY_MAKE_CLONE, pd_dos_policy_make_clone);

    // lif pd calls
    PD_FUNC_LOAD(PD_FUNC_ID_LIF_CREATE, pd_lif_create);
    PD_FUNC_LOAD(PD_FUNC_ID_LIF_DELETE, pd_lif_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_LIF_UPDATE, pd_lif_update);
    PD_FUNC_LOAD(PD_FUNC_ID_LIF_MEM_FREE, pd_lif_mem_free);
    PD_FUNC_LOAD(PD_FUNC_ID_LIF_MAKE_CLONE, pd_lif_make_clone);
    PD_FUNC_LOAD(PD_FUNC_ID_LIF_GET, pd_lif_get);
    PD_FUNC_LOAD(PD_FUNC_ID_LIF_STATS_GET, pd_lif_stats_get);
    PD_FUNC_LOAD(PD_FUNC_ID_LIF_SCHED_CONTROL, pd_lif_sched_control);

    // if pd calls
    PD_FUNC_LOAD(PD_FUNC_ID_IF_CREATE, pd_if_create);
    PD_FUNC_LOAD(PD_FUNC_ID_IF_DELETE, pd_if_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_IF_UPDATE, pd_if_update);
    PD_FUNC_LOAD(PD_FUNC_ID_IF_MEM_FREE, pd_if_mem_free);
    PD_FUNC_LOAD(PD_FUNC_ID_IF_MAKE_CLONE, pd_if_make_clone);
    PD_FUNC_LOAD(PD_FUNC_ID_IF_NWSEC_UPDATE, pd_if_nwsec_update);
    PD_FUNC_LOAD(PD_FUNC_ID_IF_LIF_UPDATE, pd_if_lif_update);
    PD_FUNC_LOAD(PD_FUNC_ID_IF_GET, pd_if_get);
    PD_FUNC_LOAD(PD_FUNC_ID_IF_RESTORE, pd_if_restore);
    PD_FUNC_LOAD(PD_FUNC_ID_IF_INP_MAC_VLAN_PGM, pd_if_inp_mac_vlan_pgm);
    PD_FUNC_LOAD(PD_FUNC_ID_IF_INP_PROP_PGM, pd_if_inp_prop_pgm);

    // ep pd calls
    PD_FUNC_LOAD(PD_FUNC_ID_EP_CREATE, pd_ep_create);
    PD_FUNC_LOAD(PD_FUNC_ID_EP_DELETE, pd_ep_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_EP_UPDATE, pd_ep_update);
    PD_FUNC_LOAD(PD_FUNC_ID_EP_IF_UPDATE, pd_ep_if_update);
    PD_FUNC_LOAD(PD_FUNC_ID_EP_MEM_FREE, pd_ep_mem_free);
    PD_FUNC_LOAD(PD_FUNC_ID_EP_MAKE_CLONE, pd_ep_make_clone);
    PD_FUNC_LOAD(PD_FUNC_ID_EP_GET, pd_ep_get);
    PD_FUNC_LOAD(PD_FUNC_ID_EP_RESTORE, pd_ep_restore);
    PD_FUNC_LOAD(PD_FUNC_ID_EP_IPSG_CHANGE, pd_ep_ipsg_change);
    PD_FUNC_LOAD(PD_FUNC_ID_EP_QUIESCE, pd_ep_quiesce);
    PD_FUNC_LOAD(PD_FUNC_ID_EP_NORMALIZATION, pd_ep_normalization);

    // session pd calls
    PD_FUNC_LOAD(PD_FUNC_ID_SESSION_CREATE, pd_session_create);
    PD_FUNC_LOAD(PD_FUNC_ID_SESSION_DELETE, pd_session_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_SESSION_UPDATE, pd_session_update);
    PD_FUNC_LOAD(PD_FUNC_ID_SESSION_GET, pd_session_get);
    PD_FUNC_LOAD(PD_FUNC_ID_SESSION_GET_FOR_AGE_THREAD,
                 pd_session_get_for_age_thread);
    PD_FUNC_LOAD(PD_FUNC_ID_BYPASS_FLOWID_GET, pd_get_cpu_bypass_flowid);
    PD_FUNC_LOAD(PD_FUNC_ID_FLOW_HASH_GET, pd_flow_hash_get);

    // tlscb pd calls
    PD_FUNC_LOAD(PD_FUNC_ID_TLSCB_CREATE, pd_tlscb_create);
    PD_FUNC_LOAD(PD_FUNC_ID_TLSCB_DELETE, pd_tlscb_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_TLSCB_UPDATE, pd_tlscb_update);
    PD_FUNC_LOAD(PD_FUNC_ID_TLSCB_GET, pd_tlscb_get);

    // tlscb pd calls
    PD_FUNC_LOAD(PD_FUNC_ID_TLS_PROXY_CB_CREATE, pd_tls_proxy_cb_create);
    PD_FUNC_LOAD(PD_FUNC_ID_TLS_PROXY_CB_DELETE, pd_tls_proxy_cb_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_TLS_PROXY_CB_UPDATE, pd_tls_proxy_cb_update);
    PD_FUNC_LOAD(PD_FUNC_ID_TLS_PROXY_CB_GET, pd_tls_proxy_cb_get);

    // tcpcb pd calls
    PD_FUNC_LOAD(PD_FUNC_ID_TCPCB_CREATE, pd_tcpcb_create);
    PD_FUNC_LOAD(PD_FUNC_ID_TCPCB_DELETE, pd_tcpcb_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_TCPCB_UPDATE, pd_tcpcb_update);
    PD_FUNC_LOAD(PD_FUNC_ID_TCPCB_GET, pd_tcpcb_get);

    // tcp_proxy_cb pd calls
    PD_FUNC_LOAD(PD_FUNC_ID_TCP_PROXY_CB_CREATE, pd_tcp_proxy_cb_create);
    PD_FUNC_LOAD(PD_FUNC_ID_TCP_PROXY_CB_DELETE, pd_tcp_proxy_cb_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_TCP_PROXY_CB_UPDATE, pd_tcp_proxy_cb_update);
    PD_FUNC_LOAD(PD_FUNC_ID_TCP_PROXY_CB_GET, pd_tcp_proxy_cb_get);

    // ipseccb pd calls
    PD_FUNC_LOAD(PD_FUNC_ID_IPSECCB_CREATE, pd_ipseccb_create);
    PD_FUNC_LOAD(PD_FUNC_ID_IPSECCB_DELETE, pd_ipseccb_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_IPSECCB_UPDATE, pd_ipseccb_update);
    PD_FUNC_LOAD(PD_FUNC_ID_IPSECCB_GET, pd_ipseccb_get);

    // ipseccb_decrypt pd calls
    PD_FUNC_LOAD(PD_FUNC_ID_IPSECCB_DECRYPT_CREATE, pd_ipseccb_decrypt_create);
    PD_FUNC_LOAD(PD_FUNC_ID_IPSECCB_DECRYPT_DELETE, pd_ipseccb_decrypt_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_IPSECCB_DECRYPT_UPDATE, pd_ipseccb_decrypt_update);
    PD_FUNC_LOAD(PD_FUNC_ID_IPSECCB_DECRYPT_GET, pd_ipseccb_decrypt_get);

    // ipsec_sa_encrypt pd calls
    PD_FUNC_LOAD(PD_FUNC_ID_IPSEC_ENCRYPT_CREATE, pd_ipsec_encrypt_create);
    PD_FUNC_LOAD(PD_FUNC_ID_IPSEC_ENCRYPT_DELETE, pd_ipsec_encrypt_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_IPSEC_ENCRYPT_UPDATE, pd_ipsec_encrypt_update);
    PD_FUNC_LOAD(PD_FUNC_ID_IPSEC_ENCRYPT_GET, pd_ipsec_encrypt_get);
    PD_FUNC_LOAD(PD_FUNC_ID_IPSEC_GLOBAL_STATS_GET, pd_ipsec_global_stats_get);

    // ipsec_sa_decrypt pd calls
    PD_FUNC_LOAD(PD_FUNC_ID_IPSEC_DECRYPT_CREATE, pd_ipsec_decrypt_create);
    PD_FUNC_LOAD(PD_FUNC_ID_IPSEC_DECRYPT_DELETE, pd_ipsec_decrypt_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_IPSEC_DECRYPT_UPDATE, pd_ipsec_decrypt_update);
    PD_FUNC_LOAD(PD_FUNC_ID_IPSEC_DECRYPT_GET, pd_ipsec_decrypt_get);

    // l4lb
    PD_FUNC_LOAD(PD_FUNC_ID_L4LB_CREATE, pd_l4lb_create);

    // cpucb
    PD_FUNC_LOAD(PD_FUNC_ID_CPUCB_CREATE, pd_cpucb_create);
    PD_FUNC_LOAD(PD_FUNC_ID_CPUCB_DELETE, pd_cpucb_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_CPUCB_UPDATE, pd_cpucb_update);
    PD_FUNC_LOAD(PD_FUNC_ID_CPUCB_GET, pd_cpucb_get);

    // rawrcb
    PD_FUNC_LOAD(PD_FUNC_ID_RAWRCB_CREATE, pd_rawrcb_create);
    PD_FUNC_LOAD(PD_FUNC_ID_RAWRCB_DELETE, pd_rawrcb_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_RAWRCB_UPDATE, pd_rawrcb_update);
    PD_FUNC_LOAD(PD_FUNC_ID_RAWRCB_GET, pd_rawrcb_get);


    // rawccb
    PD_FUNC_LOAD(PD_FUNC_ID_RAWCCB_CREATE, pd_rawccb_create);
    PD_FUNC_LOAD(PD_FUNC_ID_RAWCCB_DELETE, pd_rawccb_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_RAWCCB_UPDATE, pd_rawccb_update);
    PD_FUNC_LOAD(PD_FUNC_ID_RAWCCB_GET, pd_rawccb_get);

    // proxyrcb
    PD_FUNC_LOAD(PD_FUNC_ID_PROXYRCB_CREATE, pd_proxyrcb_create);
    PD_FUNC_LOAD(PD_FUNC_ID_PROXYRCB_DELETE, pd_proxyrcb_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_PROXYRCB_UPDATE, pd_proxyrcb_update);
    PD_FUNC_LOAD(PD_FUNC_ID_PROXYRCB_GET, pd_proxyrcb_get);

    // proxyccb
    PD_FUNC_LOAD(PD_FUNC_ID_PROXYCCB_CREATE, pd_proxyccb_create);
    PD_FUNC_LOAD(PD_FUNC_ID_PROXYCCB_DELETE, pd_proxyccb_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_PROXYCCB_UPDATE, pd_proxyccb_update);
    PD_FUNC_LOAD(PD_FUNC_ID_PROXYCCB_GET, pd_proxyccb_get);

    // qos class
    PD_FUNC_LOAD(PD_FUNC_ID_QOS_CLASS_CREATE, pd_qos_class_create);
    PD_FUNC_LOAD(PD_FUNC_ID_QOS_CLASS_RESTORE, pd_qos_class_restore);
    PD_FUNC_LOAD(PD_FUNC_ID_QOS_CLASS_DELETE, pd_qos_class_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_QOS_CLASS_UPDATE, pd_qos_class_update);
    PD_FUNC_LOAD(PD_FUNC_ID_QOS_CLASS_MAKE_CLONE, pd_qos_class_make_clone);
    PD_FUNC_LOAD(PD_FUNC_ID_QOS_CLASS_MEM_FREE, pd_qos_class_mem_free);
    PD_FUNC_LOAD(PD_FUNC_ID_QOS_CLASS_GET, pd_qos_class_get);
    PD_FUNC_LOAD(PD_FUNC_ID_QOS_CLASS_PERIODIC_STATS_UPDATE,
                   pd_qos_class_periodic_stats_update);
    PD_FUNC_LOAD(PD_FUNC_ID_QOS_CLASS_SET_GLOBAL_PAUSE_TYPE,
                   pd_qos_class_set_global_pause_type);
    PD_FUNC_LOAD(PD_FUNC_ID_QOS_CLASS_INIT_TC_TO_IQ_MAP,
                   pd_qos_class_init_tc_to_iq_map);
    PD_FUNC_LOAD(PD_FUNC_ID_QOS_SWM_QUEUE_INIT,
                   pd_qos_swm_queue_init);
    PD_FUNC_LOAD(PD_FUNC_ID_QOS_SWM_QUEUE_DEINIT,
                   pd_qos_swm_queue_deinit);

    // copp
    PD_FUNC_LOAD(PD_FUNC_ID_COPP_CREATE, pd_copp_create);
    PD_FUNC_LOAD(PD_FUNC_ID_COPP_RESTORE, pd_copp_restore);
    PD_FUNC_LOAD(PD_FUNC_ID_COPP_DELETE, pd_copp_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_COPP_UPDATE, pd_copp_update);
    PD_FUNC_LOAD(PD_FUNC_ID_COPP_MAKE_CLONE, pd_copp_make_clone);
    PD_FUNC_LOAD(PD_FUNC_ID_COPP_MEM_FREE, pd_copp_mem_free);
    PD_FUNC_LOAD(PD_FUNC_ID_COPP_GET, pd_copp_get);

    // acl
    PD_FUNC_LOAD(PD_FUNC_ID_ACL_CREATE, pd_acl_create);
    PD_FUNC_LOAD(PD_FUNC_ID_ACL_RESTORE, pd_acl_restore);
    PD_FUNC_LOAD(PD_FUNC_ID_ACL_DELETE, pd_acl_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_ACL_UPDATE, pd_acl_update);
    PD_FUNC_LOAD(PD_FUNC_ID_ACL_MEM_FREE, pd_acl_mem_free);
    PD_FUNC_LOAD(PD_FUNC_ID_ACL_MAKE_CLONE, pd_acl_make_clone);
    PD_FUNC_LOAD(PD_FUNC_ID_ACL_GET, pd_acl_get);

    // wring
    PD_FUNC_LOAD(PD_FUNC_ID_WRING_CREATE, pd_wring_create);
    // PD_FUNC_LOAD(PD_FUNC_ID_WRING_DELETE, pd_wring_delete);
    // PD_FUNC_LOAD(PD_FUNC_ID_WRING_UPDATE, pd_wring_update);
    PD_FUNC_LOAD(PD_FUNC_ID_WRING_GET_ENTRY, pd_wring_get_entry);
    PD_FUNC_LOAD(PD_FUNC_ID_WRING_GET_META, pd_wring_get_meta);
    PD_FUNC_LOAD(PD_FUNC_ID_WRING_SET_META, pd_wring_set_meta);
    PD_FUNC_LOAD(PD_FUNC_ID_WRING_GET_BASE_ADDR, pd_wring_get_base_addr);

    // mirror session
    PD_FUNC_LOAD(PD_FUNC_ID_MIRROR_SESSION_CREATE, pd_mirror_session_create);
    PD_FUNC_LOAD(PD_FUNC_ID_MIRROR_SESSION_UPDATE, pd_mirror_session_update);
    PD_FUNC_LOAD(PD_FUNC_ID_MIRROR_SESSION_DELETE, pd_mirror_session_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_MIRROR_SESSION_GET_HW_ID, pd_mirror_session_get_hw_id);

    // drop monitoring rule
    PD_FUNC_LOAD(PD_FUNC_ID_DROP_MONITOR_RULE_CREATE, pd_drop_monitor_rule_create);
    PD_FUNC_LOAD(PD_FUNC_ID_DROP_MONITOR_RULE_DELETE, pd_drop_monitor_rule_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_DROP_MONITOR_RULE_GET, pd_drop_monitor_rule_get);

    // uplink erspan
    PD_FUNC_LOAD(PD_FUNC_ID_UPLINK_ERSPAN_ENABLE, pd_uplink_erspan_enable);
    PD_FUNC_LOAD(PD_FUNC_ID_UPLINK_ERSPAN_DISABLE, pd_uplink_erspan_disable);

    // collector
    PD_FUNC_LOAD(PD_FUNC_ID_COLLECTOR_CREATE, pd_collector_create);
    PD_FUNC_LOAD(PD_FUNC_ID_COLLECTOR_DELETE, pd_collector_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_COLLECTOR_GET, pd_collector_get);
    PD_FUNC_LOAD(PD_FUNC_ID_COLL_EP_UPDATE, pd_collector_ep_update);

    // mc entry
    PD_FUNC_LOAD(PD_FUNC_ID_MC_ENTRY_GET,    pd_mc_entry_get);
    PD_FUNC_LOAD(PD_FUNC_ID_MC_ENTRY_CREATE, pd_mc_entry_create);
    PD_FUNC_LOAD(PD_FUNC_ID_MC_ENTRY_DELETE, pd_mc_entry_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_MC_ENTRY_UPDATE, pd_mc_entry_update);

    // flow get
    PD_FUNC_LOAD(PD_FUNC_ID_FLOW_GET, pd_flow_get);

    // l2seg-uplink
    PD_FUNC_LOAD(PD_FUNC_ID_ADD_L2SEG_UPLINK, pd_add_l2seg_uplink);
    PD_FUNC_LOAD(PD_FUNC_ID_DEL_L2SEG_UPLINK, pd_del_l2seg_uplink);

    // debug cli
    PD_FUNC_LOAD(PD_FUNC_ID_DEBUG_CLI_READ, pd_debug_cli_read);
    PD_FUNC_LOAD(PD_FUNC_ID_DEBUG_CLI_WRITE, pd_debug_cli_write);
    PD_FUNC_LOAD(PD_FUNC_ID_TABLE_METADATA_GET, pd_table_metadata_get);
    PD_FUNC_LOAD(PD_FUNC_ID_TABLE_GET, pd_table_get);

    // apis
    PD_FUNC_LOAD(PD_FUNC_ID_IF_GET_HW_LIF_ID, pd_if_get_hw_lif_id);
    PD_FUNC_LOAD(PD_FUNC_ID_IF_GET_LPORT_ID, pd_if_get_lport_id);
    PD_FUNC_LOAD(PD_FUNC_ID_IF_GET_TM_OPORT, pd_if_get_tm_oport);

    //Rwentry
    PD_FUNC_LOAD(PD_FUNC_ID_RWENTRY_FIND_OR_ALLOC, pd_rw_entry_find_or_alloc);

    // twice nat
    PD_FUNC_LOAD(PD_FUNC_ID_TWICE_NAT_ADD, pd_twice_nat_add);
    PD_FUNC_LOAD(PD_FUNC_ID_TWICE_NAT_DEL, pd_twice_nat_del);

    // qos
    PD_FUNC_LOAD(PD_FUNC_ID_GET_QOS_CLASSID, pd_qos_class_get_qos_class_id);
    PD_FUNC_LOAD(PD_FUNC_ID_QOS_GET_ADMIN_COS, pd_qos_class_get_admin_cos);

    // aol
    PD_FUNC_LOAD(PD_FUNC_ID_DESC_AOL_GET, pd_descriptor_aol_get);

    // crypto
    PD_FUNC_LOAD(PD_FUNC_ID_CRYPTO_ALLOC_KEY, pd_crypto_alloc_key);
    PD_FUNC_LOAD(PD_FUNC_ID_CRYPTO_ALLOC_KEY_WITHID, pd_crypto_alloc_key_withid);
    PD_FUNC_LOAD(PD_FUNC_ID_CRYPTO_FREE_KEY, pd_crypto_free_key);
    PD_FUNC_LOAD(PD_FUNC_ID_CRYPTO_WRITE_KEY, pd_crypto_write_key);
    PD_FUNC_LOAD(PD_FUNC_ID_CRYPTO_READ_KEY, pd_crypto_read_key);
    PD_FUNC_LOAD(PD_FUNC_ID_CRYPTO_ASYM_ALLOC_KEY, pd_crypto_asym_alloc_key);
    PD_FUNC_LOAD(PD_FUNC_ID_CRYPTO_ASYM_FREE_KEY, pd_crypto_asym_free_key);
    PD_FUNC_LOAD(PD_FUNC_ID_CRYPTO_ASYM_WRITE_KEY, pd_crypto_asym_write_key);
    PD_FUNC_LOAD(PD_FUNC_ID_CRYPTO_ASYM_READ_KEY, pd_crypto_asym_read_key);

    // barco
    PD_FUNC_LOAD(PD_FUNC_ID_OPAQUE_TAG_ADDR, pd_get_opaque_tag_addr);

    // stats
    PD_FUNC_LOAD(PD_FUNC_ID_DROP_STATS_GET, pd_drop_stats_get);
    PD_FUNC_LOAD(PD_FUNC_ID_DROP_STATS_CLEAR, pd_drop_stats_clear);
    PD_FUNC_LOAD(PD_FUNC_ID_EGRESS_DROP_STATS_GET, pd_egress_drop_stats_get);
    PD_FUNC_LOAD(PD_FUNC_ID_EGRESS_DROP_STATS_CLEAR, pd_egress_drop_stats_clear);
    PD_FUNC_LOAD(PD_FUNC_ID_TABLE_STATS_GET, pd_table_stats_get);
    PD_FUNC_LOAD(PD_FUNC_ID_SYSTEM_DROP_STATS_GET, pd_system_drop_stats_get);

    // oifl
    PD_FUNC_LOAD(PD_FUNC_ID_OIFL_GET, pd_oif_list_get);
    PD_FUNC_LOAD(PD_FUNC_ID_OIFL_CREATE, pd_oif_list_create);
    PD_FUNC_LOAD(PD_FUNC_ID_OIFL_CREATE_BLOCK, pd_oif_list_create_block);
    PD_FUNC_LOAD(PD_FUNC_ID_OIFL_DELETE, pd_oif_list_delete);
    PD_FUNC_LOAD(PD_FUNC_ID_OIFL_DELETE_BLOCK, pd_oif_list_delete_block);
    PD_FUNC_LOAD(PD_FUNC_ID_OIFL_ATTACH, pd_oif_list_attach);
    PD_FUNC_LOAD(PD_FUNC_ID_OIFL_DETACH, pd_oif_list_detach);
    PD_FUNC_LOAD(PD_FUNC_ID_OIFL_ADD_OIF, pd_oif_list_add_oif);
    PD_FUNC_LOAD(PD_FUNC_ID_OIFL_ADD_QP_OIF, pd_oif_list_add_qp_oif);
    PD_FUNC_LOAD(PD_FUNC_ID_OIFL_REM_OIF, pd_oif_list_remove_oif);
    PD_FUNC_LOAD(PD_FUNC_ID_OIFL_IS_MEMBER, pd_oif_list_is_member);
    PD_FUNC_LOAD(PD_FUNC_ID_GET_NUM_OIFS, pd_oif_list_get_num_oifs);
    PD_FUNC_LOAD(PD_FUNC_ID_SET_HONOR_ING, pd_oif_list_set_honor_ingress);
    PD_FUNC_LOAD(PD_FUNC_ID_CLR_HONOR_ING, pd_oif_list_clr_honor_ingress);

    // tnnl if
    PD_FUNC_LOAD(PD_FUNC_ID_TNNL_IF_GET_RW_IDX, pd_tunnelif_get_rw_idx);
    PD_FUNC_LOAD(PD_FUNC_ID_TUNNEL_IF_RTEP_UPDATE, pd_tunnel_if_update_rtep);

    // tcp
    PD_FUNC_LOAD(PD_FUNC_ID_TCP_RINGS_CTXT_INIT, pd_tcp_rings_ctxt_init);
    PD_FUNC_LOAD(PD_FUNC_ID_TCP_RINGS_REGISTER, pd_tcp_rings_register);
    PD_FUNC_LOAD(PD_FUNC_ID_TCP_RINGS_POLL, pd_tcp_rings_poll);

    // cpu
    PD_FUNC_LOAD(PD_FUNC_ID_CPU_ALLOC_INIT, pd_cpupkt_ctxt_alloc_init);
    PD_FUNC_LOAD(PD_FUNC_ID_CPU_REG_RXQ, pd_cpupkt_register_rx_queue);
    PD_FUNC_LOAD(PD_FUNC_ID_CPU_REG_TXQ, pd_cpupkt_register_tx_queue);
    PD_FUNC_LOAD(PD_FUNC_ID_CPU_UNREG_TXQ, pd_cpupkt_unregister_tx_queue);
    PD_FUNC_LOAD(PD_FUNC_ID_CPU_POLL_RECV, pd_cpupkt_poll_receive);
    PD_FUNC_LOAD(PD_FUNC_ID_CPU_POLL_RECV_NEW, pd_cpupkt_poll_receive_new);
    PD_FUNC_LOAD(PD_FUNC_ID_CPU_FREE_PKT_RES, pd_cpupkt_free_pkt_resources);
    PD_FUNC_LOAD(PD_FUNC_ID_CPU_SEND, pd_cpupkt_send);
    PD_FUNC_LOAD(PD_FUNC_ID_CPU_SEND_NEW, pd_cpupkt_send_new);
    PD_FUNC_LOAD(PD_FUNC_ID_CPU_PAGE_ALLOC, pd_cpupkt_page_alloc);
    PD_FUNC_LOAD(PD_FUNC_ID_CPU_DESCR_ALLOC, pd_cpupkt_descr_alloc);
    PD_FUNC_LOAD(PD_FUNC_ID_PGM_SEND_RING_DBELL, pd_cpupkt_program_send_ring_doorbell);
    PD_FUNC_LOAD(PD_FUNC_ID_CPU_GET_GLOBAL, pd_cpupkt_get_global);

    // rdma
    PD_FUNC_LOAD(PD_FUNC_ID_RXDMA_TABLE_ADD, pd_rxdma_table_entry_add);
    PD_FUNC_LOAD(PD_FUNC_ID_TXDMA_TABLE_ADD, pd_txdma_table_entry_add);

    // lif
    PD_FUNC_LOAD(PD_FUNC_ID_LIF_GET_LPORTID, pd_lif_get_lport_id);

    // p4pt
    PD_FUNC_LOAD(PD_FUNC_ID_P4PT_INIT, p4pt_pd_init);

    // eth
    PD_FUNC_LOAD(PD_FUNC_ID_RSS_PARAMS_TABLE_ADD, pd_rss_params_table_entry_add);
    PD_FUNC_LOAD(PD_FUNC_ID_RSS_INDIR_TABLE_ADD, pd_rss_indir_table_entry_add);

    // asic
    PD_FUNC_LOAD(PD_FUNC_ID_ASIC_INIT, pd_asic_init);

    PD_FUNC_LOAD(PD_FUNC_ID_TABLE_PROPERTIES_GET, pd_table_properties_get);

    // capri
    PD_FUNC_LOAD(PD_FUNC_ID_HBM_READ, pd_hbm_read_mem);
    PD_FUNC_LOAD(PD_FUNC_ID_HBM_WRITE, pd_hbm_write_mem);
    PD_FUNC_LOAD(PD_FUNC_ID_PROG_LBL_TO_OFFSET, pd_program_label_to_offset);
    PD_FUNC_LOAD(PD_FUNC_ID_PROG_TO_BASE_ADDR, pd_program_to_base_addr);

    // hw clock to sw clock conversion api
    PD_FUNC_LOAD(PD_FUNC_ID_CONV_HW_CLOCK_TO_SW_CLOCK, pd_conv_hw_clock_to_sw_clock);
    PD_FUNC_LOAD(PD_FUNC_ID_CONV_SW_CLOCK_TO_HW_CLOCK, pd_conv_sw_clock_to_hw_clock);
    PD_FUNC_LOAD(PD_FUNC_ID_CLOCK_DELTA_COMP, pd_clock_delta_comp);
    PD_FUNC_LOAD(PD_FUNC_ID_CLOCK_DETAIL_GET, pd_clock_detail_get);
    PD_FUNC_LOAD(PD_FUNC_ID_CLOCK_TRIGGER_SYNC, pd_clock_trigger_sync);
    PD_FUNC_LOAD(PD_FUNC_ID_SET_CLOCK_MULTIPLIER, pd_set_clock_multiplier);

    // packet buffer
    PD_FUNC_LOAD(PD_FUNC_ID_PACKET_BUFFER_UPDATE, pd_packet_buffer_update);
    PD_FUNC_LOAD(PD_FUNC_ID_SPAN_THRESHOLD_UPDATE, pd_span_threshold_update);

    // gft
    PD_FUNC_LOAD(PD_FUNC_ID_GFT_EXACT_MATCH_PROFILE_CREATE,
                   pd_gft_exact_match_profile_create);
    PD_FUNC_LOAD(PD_FUNC_ID_GFT_HDR_TRANSPOSITION_PROFILE_CREATE,
                   pd_gft_hdr_group_xposition_profile_create);
    PD_FUNC_LOAD(PD_FUNC_ID_GFT_EXACT_MATCH_FLOW_ENTRY_CREATE,
                   pd_gft_exact_match_flow_entry_create);

    // slab
    PD_FUNC_LOAD(PD_FUNC_ID_GET_SLAB, pd_get_slab);

    // swphv
    PD_FUNC_LOAD(PD_FUNC_ID_SWPHV_INJECT, pd_swphv_inject);
    PD_FUNC_LOAD(PD_FUNC_ID_SWPHV_GET_STATE, pd_swphv_get_state);

    PD_FUNC_LOAD(PD_FUNC_ID_UPG_TABLE_RESET, pd_system_upgrade_table_reset);

    // pb
    PD_FUNC_LOAD(PD_FUNC_ID_PB_STATS_GET, pd_pb_stats_get);
    PD_FUNC_LOAD(PD_FUNC_ID_PB_STATS_CLEAR, pd_pb_stats_clear);

    // fte_span pd calls
    PD_FUNC_LOAD(PD_FUNC_ID_FTE_SPAN_GET, pd_fte_span_get);
    PD_FUNC_LOAD(PD_FUNC_ID_FTE_SPAN_CREATE, pd_fte_span_create);
    PD_FUNC_LOAD(PD_FUNC_ID_FTE_SPAN_UPDATE, pd_fte_span_update);
    PD_FUNC_LOAD(PD_FUNC_ID_FTE_SPAN_MEM_FREE, pd_fte_span_mem_free);
    PD_FUNC_LOAD(PD_FUNC_ID_FTE_SPAN_MAKE_CLONE, pd_fte_span_make_clone);
    // PD_FUNC_LOAD(PD_FUNC_ID_FTE_SPAN_GET, pd_fte_span_get);

    // tcp global stats
    PD_FUNC_LOAD(PD_FUNC_ID_TCP_GLOBAL_STATS_GET, pd_tcp_global_stats_get);

    // snake tests
    PD_FUNC_LOAD(PD_FUNC_ID_SNAKE_TEST_CREATE, pd_snake_test_create);
    PD_FUNC_LOAD(PD_FUNC_ID_SNAKE_TEST_DELETE, pd_snake_test_delete);

    // nvme sesscb pd calls
    PD_FUNC_LOAD(PD_FUNC_ID_NVME_SESSCB_CREATE, pd_nvme_sesscb_create);
    //PD_FUNC_LOAD(PD_FUNC_ID_NVME_SESSCB_DELETE, pd_nvme_sesscb_delete);
    //PD_FUNC_LOAD(PD_FUNC_ID_NVME_SESSCB_UPDATE, pd_nvme_sesscb_update);
    //PD_FUNC_LOAD(PD_FUNC_ID_NVME_SESSCB_GET, pd_nvme_sesscb_get);

    // nvme global pd calls
    PD_FUNC_LOAD(PD_FUNC_ID_NVME_GLOBAL_CREATE, pd_nvme_global_create);
    //PD_FUNC_LOAD(PD_FUNC_ID_NVME_GLOBAL_GET, pd_nvme_global_get);

    // nvme ns pd calls
    PD_FUNC_LOAD(PD_FUNC_ID_NVME_NS_CREATE, pd_nvme_ns_create);
    //PD_FUNC_LOAD(PD_FUNC_ID_NVME_NS_DELETE, pd_nvme_ns_delete);
    //PD_FUNC_LOAD(PD_FUNC_ID_NVME_NS_UPDATE, pd_nvme_ns_update);
    //PD_FUNC_LOAD(PD_FUNC_ID_NVME_NS_GET, pd_nvme_ns_get);

    // nvme sq pd calls
    PD_FUNC_LOAD(PD_FUNC_ID_NVME_SQ_CREATE, pd_nvme_sq_create);
    //PD_FUNC_LOAD(PD_FUNC_ID_NVME_SQ_DELETE, pd_nvme_sq_delete);
    //PD_FUNC_LOAD(PD_FUNC_ID_NVME_SQ_UPDATE, pd_nvme_sq_update);
    //PD_FUNC_LOAD(PD_FUNC_ID_NVME_SQ_GET, pd_nvme_sq_get);

    // nvme cq pd calls
    PD_FUNC_LOAD(PD_FUNC_ID_NVME_CQ_CREATE, pd_nvme_cq_create);
    //PD_FUNC_LOAD(PD_FUNC_ID_NVME_CQ_DELETE, pd_nvme_cq_delete);
    //PD_FUNC_LOAD(PD_FUNC_ID_NVME_CQ_UPDATE, pd_nvme_cq_update);
    //PD_FUNC_LOAD(PD_FUNC_ID_NVME_CQ_GET, pd_nvme_cq_get);

    return ret;
}

hal_ret_t
hal_pd_call (pd_func_id_t pd_func_id, pd_func_args_t *args)
{
    hal_ret_t    ret;

    ret = g_pd_funcs[pd_func_id](args);
    return ret;
}

static inline hal_ret_t
hal_pd_libopen (hal_cfg_t *hal_cfg)
{
    hal_ret_t   ret         = HAL_RET_OK;
    std::string feature_set = std::string(hal_cfg->feature_set);
    std::string feature_pd_stub = "pd_stub";
    std::string pdlib_path, pdlib_stub_path;

    //pdlib_path = hal_cfg->cfg_path + "/" + feature_set + "/";
    //pdlib_stub_path = hal_cfg->cfg_path + "/" + feature_pd_stub + "/";
    pdlib_path = "libpd_" + feature_set + ".so";
    pdlib_stub_path = "lib" + feature_pd_stub + ".so";

    HAL_TRACE_DEBUG("Using pd lib: {}", pdlib_path);
    // with deepbind, its taking the symbol the PD is taking from PI
    // void *so = dlopen(pdlib_path.c_str(), RTLD_NOW|RTLD_GLOBAL|RTLD_DEEPBIND);
    g_pd_so = dlopen(pdlib_path.c_str(), RTLD_NOW|RTLD_GLOBAL|RTLD_NOLOAD);
    if (!g_pd_so) {
        HAL_TRACE_DEBUG("Loading pd lib: {}", pdlib_path);
        g_pd_so = dlopen(pdlib_path.c_str(), RTLD_NOW|RTLD_GLOBAL);
        if (!g_pd_so) {
            HAL_TRACE_ERR("dlopen failed {}: {}", pdlib_path, dlerror());
            SDK_ASSERT(0);
        }
    }

    // open PD stub library
    HAL_TRACE_DEBUG("Using pd stub lib: {}", pdlib_stub_path);
    g_pd_stub_so = dlopen(pdlib_stub_path.c_str(), RTLD_NOW|RTLD_GLOBAL|RTLD_NOLOAD);
    if (!g_pd_stub_so) {
        HAL_TRACE_DEBUG("Loading pd stub lib: {}", pdlib_stub_path);
        g_pd_stub_so = dlopen(pdlib_stub_path.c_str(), RTLD_NOW|RTLD_GLOBAL);
        if (!g_pd_stub_so) {
            HAL_TRACE_ERR("{} dlopen failed {}", pdlib_stub_path, dlerror());
            SDK_ASSERT(0);
        }
    }

    return ret;
}

// load PD modules and all necessary symbols
static inline hal_ret_t
hal_pd_module_init (hal_cfg_t *hal_cfg)
{
    hal_ret_t    ret;

    // open pd libs
    ret = hal_pd_libopen(hal_cfg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("HAL PD lib open failed, err : {}", ret);
        return ret;
    }

    // load pd symbols
    ret = hal_pd_load_symbols();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("HAL PD lib load symbols failed, err : {}", ret);
        return ret;
    }

    return HAL_RET_OK;
}

hal_ret_t
hal_pd_clock_delta_comp_init (hal_cfg_t *hal_cfg)
{
    hal_ret_t                     ret;
    pd_clock_delta_comp_args_t    clock_args;
    pd_func_args_t                pd_func_args = { 0 };

    pd_func_args.pd_clock_delta_comp = &clock_args;
    ret = hal_pd_call(PD_FUNC_ID_CLOCK_DELTA_COMP, &pd_func_args);

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
    pal_ret_t                           palrv;
    hal_ret_t                           ret;
    pd_mem_init_args_t                  mem_init_args;
    pd_mem_init_phase2_args_t           ph2_args;
    pd_pgm_def_entries_args_t           pgm_def_args;
    pd_pgm_def_p4plus_entries_args_t    pgm_p4p_args;
    pd_func_args_t                      pd_func_args = { 0 };
    sdk::lib::thread                    *hal_thread;
    asic_cfg_t                          asic_cfg;
    pd_asic_init_args_t                 args;

    SDK_ASSERT(hal_cfg != NULL);
    hal_pd_module_init(hal_cfg);

    mem_init_args.cfg_path = hal_cfg->cfg_path.c_str();
    mem_init_args.hal_cfg = hal_cfg;
    pd_func_args.pd_mem_init = &mem_init_args;
    ret = hal_pd_call(PD_FUNC_ID_MEM_INIT, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("HAL PD init failed, err : {}", ret);
        goto cleanup;
    }

    // initialize PAL
    palrv = sdk::lib::pal_init(hal_cfg->platform);
    SDK_ASSERT_GOTO(IS_PAL_API_SUCCESS(palrv), cleanup);

    // do asic initialization
    asic_cfg.default_config_dir = hal_cfg->default_config_dir;
    asic_cfg.admin_cos = 1;
    asic_cfg.cfg_path = hal_cfg->cfg_path;
    asic_cfg.catalog = hal_cfg->catalog;
    asic_cfg.mempartition = hal_cfg->mempartition;
    asic_cfg.num_rings = 0;
    asic_cfg.ring_meta = NULL;
    asic_cfg.platform = hal_cfg->platform;
    asic_cfg.device_profile = hal_cfg->device_cfg.device_profile;
    args.cfg = &asic_cfg;
    pd_func_args.pd_asic_init = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_ASIC_INIT, &pd_func_args);
    SDK_ASSERT_GOTO((ret == HAL_RET_OK), cleanup);

    ph2_args.cfg_path = mem_init_args.cfg_path;
    ph2_args.hal_cfg = hal_cfg;
    pd_func_args.pd_mem_init_phase2 = &ph2_args;
    ret = hal_pd_call(PD_FUNC_ID_MEM_INIT_PHASE2, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("HAL PD init failed, err : {}", ret);
        goto cleanup;
    }

    pgm_def_args.hal_cfg = hal_cfg;
    pd_func_args.pd_pgm_def_entries = &pgm_def_args;
    ret = hal_pd_call(PD_FUNC_ID_PGM_DEF_ENTRIES, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("HAL Programming default entries, err : {}", ret);
        goto cleanup;
    }

    pgm_p4p_args.hal_cfg = hal_cfg;
    pd_func_args.pd_pgm_def_p4plus_entries = &pgm_p4p_args;
    ret = hal_pd_call(PD_FUNC_ID_PGM_DEF_P4PLUS_ENTRIES, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("HAL Programming default p4plus entries failed, err : {}", ret);
        goto cleanup;
    }

    // start the asic-rw thread
    HAL_TRACE_DEBUG("Starting asic-rw thread ...");
    hal_thread =
        hal_thread_create(std::string("asicrw").c_str(),
            HAL_THREAD_ID_ASIC_RW,
            sdk::lib::THREAD_ROLE_CONTROL,
            0x0,    // use all control cores
            sdk::asic::asicrw_start,
            sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            hal_cfg);
    SDK_ASSERT_TRACE_RETURN((hal_thread != NULL), HAL_RET_ERR,
                            "asicrw thread creation failure");
    hal_thread->start(hal_thread);

    HAL_TRACE_DEBUG("Waiting for asic-rw thread to be ready ...");
    // wait for ASIC RW thread to be ready before initializing table entries
    while (!sdk::asic::is_asicrw_ready()) {
        pthread_yield();
    }

    return HAL_RET_OK;

cleanup:

    return ret;
}

extern "C" int
pd_tls_asym_ecdsa_p256_sig_gen (int32_t key_idx, uint8_t *p, uint8_t *n,
                                uint8_t *xg, uint8_t *yg,
                                uint8_t *a, uint8_t *b,
                                uint8_t *da, uint8_t *k,
                                uint8_t *h, uint8_t *r,
                                uint8_t *s, bool async,
                                const uint8_t *unique_key)
{
    sdk_ret_t sdk_ret;
    hal_ret_t ret = HAL_RET_OK;

    /* WIP-WIP: originally 'n' was used for 'p'*/
    sdk_ret = asicpd_barco_asym_ecdsa_p256_sig_gen(key_idx, p, n, xg, yg, a, b,
                                                   da, k, h, r, s, async,
                                                   unique_key);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if(ret != HAL_RET_OK) {
        return -1;
    }
    return 1;
}

extern "C" int
pd_tls_asym_ecdsa_p256_sig_gen_param (void *ctx,
                                      const PSE_EC_SIGN_PARAM *param)
{
    return pd_tls_asym_ecdsa_p256_sig_gen(param->key_idx, NULL, NULL, NULL,
                                          NULL, NULL, NULL, NULL,
                                          param->k_random, param->hash_input,
                                          param->r_output, param->s_output,
                                          param->async, param->caller_unique_id);
}

extern "C" int
pd_tls_asym_ecdsa_p256_sig_verify (uint8_t *p, uint8_t *n,
                                   uint8_t *xg, uint8_t *yg, uint8_t *a,
                                   uint8_t *b, uint8_t *xq, uint8_t *yq,
                                   uint8_t *r, uint8_t *s, uint8_t *h,
                                   bool async, const uint8_t *unique_key)
{
    sdk_ret_t sdk_ret;
    hal_ret_t ret = HAL_RET_OK;

    sdk_ret = asicpd_barco_asym_ecdsa_p256_sig_verify(p, n, xg, yg, a, b,
                                                      xq, yq, r, s, h,
                                                      async, unique_key);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        return -1;
    }
    return 1;
}

extern "C" int
pd_tls_asym_ecdsa_p256_sig_verify_param(void *ctx,
                                        const PSE_EC_VERIFY_PARAM *param)
{
    return pd_tls_asym_ecdsa_p256_sig_verify(param->p, param->n, param->xg, param->yg,
                                             param->a, param->b, param->xq, param->yq,
                                             param->r, param->s, param->hash_input,
                                             param->async, param->caller_unique_id);
}

extern "C" int
pd_tls_asym_rsa2k_encrypt(uint8_t *n, uint8_t *e, uint8_t *m,  uint8_t *c,
                          bool async, const uint8_t *unique_key)
{
    sdk_ret_t sdk_ret = SDK_RET_OK;
    hal_ret_t ret = HAL_RET_OK;

    sdk_ret = asicpd_barco_asym_rsa2k_encrypt(n, e, m, c, async, unique_key);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    if (ret != HAL_RET_OK) {
        return -1;
    }
    return 1;
}

extern "C" int
pd_tls_asym_rsa_encrypt (uint16_t key_size, uint8_t *n, uint8_t *e,
                         uint8_t *m,  uint8_t *c, bool async,
                         const uint8_t *unique_key)
{
    sdk_ret_t sdk_ret = SDK_RET_OK;
    hal_ret_t ret = HAL_RET_OK;

    sdk_ret = asicpd_barco_asym_rsa_encrypt(key_size, n, e, m, c, async, unique_key);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    if (ret != HAL_RET_OK) {
        return -1;
    }
    return 1;
}

extern "C" int
pd_tls_asym_rsa_encrypt_param (void *ctx, const PSE_RSA_ENCRYPT_PARAM *param)
{
    return pd_tls_asym_rsa_encrypt(param->key_size, param->n,
                                   param->e, param->plain_input,
                                   param->ciphered_output, param->async,
                                   param->caller_unique_id);
}

extern "C" int
pd_tls_asym_rsa_sig_gen (uint16_t key_size, int32_t key_idx, uint8_t *n,
                         uint8_t *d, uint8_t *h,  uint8_t *s,
                         bool async, const uint8_t *unique_key)
{
    return (asicpd_barco_asym_rsa_sig_gen(key_size, key_idx, n, d, h, s,
                                          async, unique_key) ==
            SDK_RET_OK ? 1 : -1);
}

extern "C" int
pd_tls_asym_rsa_sig_gen_param (void *ctx, const PSE_RSA_SIGN_PARAM *param)
{
    return pd_tls_asym_rsa_sig_gen(param->key_size, param->key_idx,
                                   param->n, param->d, param->hash_input,
                                   param->sig_output,  param->async,
                                   param->caller_unique_id);
}

extern "C" int
pd_tls_asym_rsa2k_crt_decrypt (int32_t key_idx, uint8_t *p, uint8_t *q,
                               uint8_t *dp, uint8_t *dq, uint8_t *qinv,
                               uint8_t *c, uint8_t *m, bool async,
                               const uint8_t *unique_key)
{
    sdk_ret_t sdk_ret = SDK_RET_OK;
    hal_ret_t ret = HAL_RET_OK;

    sdk_ret = asicpd_barco_asym_rsa2k_crt_decrypt(key_idx, p, q, dp, dq, qinv, c, m, async, unique_key);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    if (ret != HAL_RET_OK) {
        return -1;
    }
    return 1;
}

extern "C" int
pd_tls_asym_rsa_decrypt_param (void *ctx, const PSE_RSA_DECRYPT_PARAM *param)
{
    return pd_tls_asym_rsa2k_crt_decrypt(param->key_idx, param->p, param->q,
                                         param->dp, param->dq, param->qinv,
                                         param->ciphered_input,
                                         param->plain_output,
                                         param->async, param->caller_unique_id);
}

hal_ret_t
pd_barco_asym_rsa2k_sig_gen (pd_func_args_t *pd_func_args)
{
    pd_barco_asym_rsa2k_sig_gen_args_t *args =
        pd_func_args->pd_barco_asym_rsa2k_sig_gen;

    return (asicpd_barco_asym_rsa2k_sig_gen(args->key_idx, args->n, args->d,
                                            args->h, args->s, args->async_en,
                                            args->unique_key) ==
            SDK_RET_OK ? HAL_RET_OK : HAL_RET_ERR);
}

hal_ret_t
pd_mpu_trace_enable (pd_func_args_t *pd_func_args)
{
    pd_mpu_trace_enable_args_t *args = pd_func_args->pd_mpu_trace_enable;
    sdk_ret_t sdk_ret = SDK_RET_OK;

    switch (args->pipeline_type) {
    case MPU_TRACE_PIPELINE_P4_INGRESS:
        sdk_ret = asicpd_p4_ingress_mpu_trace_enable(args->stage_id,
                                                     args->mpu,
                                                     args->mpu_trace_info.enable,
                                                     args->mpu_trace_info.trace_enable,
                                                     args->mpu_trace_info.phv_debug,
                                                     args->mpu_trace_info.phv_error,
                                                     args->mpu_trace_info.watch_pc,
                                                     args->mpu_trace_info.base_addr,
                                                     args->mpu_trace_info.table_key,
                                                     args->mpu_trace_info.instructions,
                                                     args->mpu_trace_info.wrap,
                                                     args->mpu_trace_info.reset,
                                                     args->mpu_trace_info.buf_size);
        break;

    case MPU_TRACE_PIPELINE_P4_EGRESS:
        sdk_ret = asicpd_p4_egress_mpu_trace_enable(args->stage_id,
                                                    args->mpu,
                                                    args->mpu_trace_info.enable,
                                                    args->mpu_trace_info.trace_enable,
                                                    args->mpu_trace_info.phv_debug,
                                                    args->mpu_trace_info.phv_error,
                                                    args->mpu_trace_info.watch_pc,
                                                    args->mpu_trace_info.base_addr,
                                                    args->mpu_trace_info.table_key,
                                                    args->mpu_trace_info.instructions,
                                                    args->mpu_trace_info.wrap,
                                                    args->mpu_trace_info.reset,
                                                    args->mpu_trace_info.buf_size);
        break;

    case MPU_TRACE_PIPELINE_P4P_RXDMA:
        sdk_ret = asicpd_p4p_rxdma_mpu_trace_enable(args->stage_id,
                                                    args->mpu,
                                                    args->mpu_trace_info.enable,
                                                    args->mpu_trace_info.trace_enable,
                                                    args->mpu_trace_info.phv_debug,
                                                    args->mpu_trace_info.phv_error,
                                                    args->mpu_trace_info.watch_pc,
                                                    args->mpu_trace_info.base_addr,
                                                    args->mpu_trace_info.table_key,
                                                    args->mpu_trace_info.instructions,
                                                    args->mpu_trace_info.wrap,
                                                    args->mpu_trace_info.reset,
                                                    args->mpu_trace_info.buf_size);
        break;

    case MPU_TRACE_PIPELINE_P4P_TXDMA:
        sdk_ret = asicpd_p4p_txdma_mpu_trace_enable(args->stage_id,
                                                    args->mpu,
                                                    args->mpu_trace_info.enable,
                                                    args->mpu_trace_info.trace_enable,
                                                    args->mpu_trace_info.phv_debug,
                                                    args->mpu_trace_info.phv_error,
                                                    args->mpu_trace_info.watch_pc,
                                                    args->mpu_trace_info.base_addr,
                                                    args->mpu_trace_info.table_key,
                                                    args->mpu_trace_info.instructions,
                                                    args->mpu_trace_info.wrap,
                                                    args->mpu_trace_info.reset,
                                                    args->mpu_trace_info.buf_size);

        break;

    default:
        return HAL_RET_ERR;
    }

    return hal_sdk_ret_to_hal_ret(sdk_ret);
}

extern "C" hal_ret_t
pd_reg_write (pd_func_args_t *pd_func_args)
{
    pd_reg_write_args_t *args = pd_func_args->pd_reg_write;
    sdk_ret_t sdk_ret = SDK_RET_OK;

    switch (args->register_id) {
    case pd_reg_write_type_t::DPP_INT_CREDIT:
        sdk_ret = asicpd_dpp_int_credit(args->instance, args->value);
        break;
    default:
        return HAL_RET_ERR;
    }

    return hal_sdk_ret_to_hal_ret(sdk_ret);
}

}    // namespace pd
}    // namespace hal
