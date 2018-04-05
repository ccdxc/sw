#include "nic/include/base.h"
#include "nic/include/hal_lock.hpp"
#include "nic/gen/iris/include/p4pd.h"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/dos/dos_pd.hpp"
#include "nic/hal/pd/iris/nw/vrf_pd.hpp"
#include "nic/hal/pd/iris/nw/if_pd_utils.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/include/dos_api.hpp"

namespace hal {
namespace pd {

//-----------------------------------------------------------------------------
// PD DoS Create
//-----------------------------------------------------------------------------
hal_ret_t
pd_dos_policy_create (pd_dos_policy_create_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;
    pd_dos_policy_t      *pd_dosp;

    HAL_TRACE_DEBUG("{}: creating pd state ",
                    __FUNCTION__);

    // Create dos PD
    pd_dosp = dos_pd_alloc_init();
    if (pd_dosp == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    dos_link_pi_pd(pd_dosp, args->dos_policy);

    // Allocate Resources
    ret = dos_pd_alloc_res(pd_dosp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to alloc. resources");
        goto end;
    }

    // Program HW
    ret = dos_pd_program_hw(pd_dosp, TRUE);

end:
    if (ret != HAL_RET_OK) {
        dos_pd_cleanup(pd_dosp);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD DoS Update
//-----------------------------------------------------------------------------
hal_ret_t
pd_dos_policy_update (pd_dos_policy_update_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;;
    pd_dos_policy_t   *pd_dosp;

    HAL_TRACE_DEBUG("{}: updating pd state ",
                    __FUNCTION__);

    pd_dosp = (pd_dos_policy_t *)args->clone_policy->pd;
    ret = dos_pd_program_hw(pd_dosp, FALSE);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("{}: unable to program hw, ret : {}",
                      __FUNCTION__, ret);
    }

    return ret;

#if 0
    pd_dosp = (pd_dos_policy_t *) args->dos_policy->pd;
    // Cache the PI pointer since the ptr in the
    // args is a local copy
    void *cached_pi_ptr = pd_dosp->pi_dos_policy;

    pd_dosp->pi_dos_policy = (void *) args->dos_policy;
    // Program HW
    ret = dos_pd_program_hw(pd_dosp, FALSE);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("PD-DoS::{}: Unable to program hw, ret : {}",
                      __FUNCTION__, ret);
    }
    // Revert back to the cached PI ptr
    pd_dosp->pi_dos_policy = cached_pi_ptr;
#endif
}

//-----------------------------------------------------------------------------
// PD DoS Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_dos_policy_delete (pd_dos_policy_delete_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_dos_policy_t  *dos_pd;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->dos_policy != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->dos_policy->pd != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("{}:deleting pd state for dos policy handle {}",
                    __FUNCTION__, args->dos_policy->hal_handle);
    dos_pd = (pd_dos_policy_t *)args->dos_policy->pd;

    // deprogram HW
    ret = dos_pd_deprogram_hw(dos_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:unable to deprogram hw", __FUNCTION__);
    }

    // dealloc resources and free
    ret = dos_pd_cleanup(dos_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed pd dos delete",
                      __FUNCTION__);
    }

    return ret;
}

hal_ret_t
dos_pd_program_ddos_src_vf_tcam (uint16_t slport, int actionid,
                                 uint16_t policer_idx,
                                 p4pd_table_id tbl_id, int *idx)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    ddos_src_vf_swkey_t         key;
    ddos_src_vf_swkey_mask_t    mask;
    ddos_src_vf_actiondata      data;
    tcam                        *tcam = NULL;
    uint32_t                    ret_idx;

    tcam = g_hal_state_pd->tcam_table(tbl_id);
    HAL_ASSERT(tcam != NULL);
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    key.entry_inactive_ddos_src_vf = 0;
    key.control_metadata_src_lport = slport;
    mask.entry_inactive_ddos_src_vf_mask = 0xFF;
    mask.control_metadata_src_lport_mask = 0xFFFF;

    data.actionid = actionid;
    data.ddos_src_vf_action_u.ddos_src_vf_ddos_src_vf_hit.ddos_src_vf_base_policer_idx = policer_idx;
    HAL_TRACE_DEBUG("{}: act_id: {} pol_index: {} slport: {}",
                     __FUNCTION__, actionid, policer_idx, slport);
    sdk_ret = tcam->insert(&key, &mask, &data, &ret_idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret == HAL_RET_DUP_INS_FAIL) {
        /* Entry already exists. Can be skipped */
        *idx = -1;
    } else {
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("DDoS src-vf tcam write failure, "
                          "idx : {}, err : {}", ret_idx, ret);
            return ret;
        }
    }
    HAL_TRACE_DEBUG("DDoS src-vf tcam write, "
                    "return-idx : {}, ret: {}", ret_idx, ret);
    *idx = (int) ret_idx;
    return ret;
}

hal_ret_t
dos_pd_program_ddos_service_tcam (ip_addr_t *ip_addr, bool is_icmp,
                                  uint16_t dport, uint8_t proto,
                                  uint16_t vrf, int actionid,
                                  uint16_t policer_idx,
                                  p4pd_table_id tbl_id, int *idx)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    ddos_service_swkey_t        key;
    ddos_service_swkey_mask_t   mask;
    ddos_service_actiondata     data;
    tcam                        *tcam = NULL;
    uint32_t                    ret_idx;

    tcam = g_hal_state_pd->tcam_table(tbl_id);
    HAL_ASSERT(tcam != NULL);
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    if (ip_addr->af == IP_AF_IPV6) {
        memcpy(key.flow_lkp_metadata_lkp_dst, ip_addr->addr.v6_addr.addr8,
               IP6_ADDR8_LEN);
        memrev(key.flow_lkp_metadata_lkp_dst, sizeof(key.flow_lkp_metadata_lkp_dst));
        memset(mask.flow_lkp_metadata_lkp_dst_mask, 0xFF, IP6_ADDR8_LEN);
    } else if (ip_addr->af == IP_AF_IPV4) {
        memcpy(key.flow_lkp_metadata_lkp_dst, &ip_addr->addr.v4_addr,
               sizeof(ipv4_addr_t));
        memset(mask.flow_lkp_metadata_lkp_dst_mask, 0xFF, sizeof(ipv4_addr_t));
    }

    if (is_icmp || (dport != 0)) {
        key.flow_lkp_metadata_lkp_dport = dport;
        mask.flow_lkp_metadata_lkp_dport_mask = 0xFFFF;
    }
    if (proto != 0) {
        key.flow_lkp_metadata_lkp_proto = proto;
        mask.flow_lkp_metadata_lkp_proto_mask = 0xFF;
    }
    /* LSB 12-bits is L2seg hw-id which is masked off */
    uint16_t vrf_mask = 0xFFFF;
    key.flow_lkp_metadata_lkp_vrf = (vrf << 12);
    mask.flow_lkp_metadata_lkp_vrf_mask = (vrf_mask << 12);
    key.entry_inactive_ddos_service = 0;
    mask.entry_inactive_ddos_service_mask = 0xFF;

    data.actionid = actionid;
    data.ddos_service_action_u.ddos_service_ddos_service_hit.ddos_service_base_policer_idx = policer_idx;
    HAL_TRACE_DEBUG("{} ip-addr: {} dport: {} proto: {}"
                    "vrf: {} act_id: {} pol_idx: {}", __FUNCTION__,
                    *ip_addr, dport, proto, vrf, actionid, policer_idx);

    sdk_ret = tcam->insert(&key, &mask, &data, &ret_idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret == HAL_RET_DUP_INS_FAIL) {
        /* Entry already exists. Can be skipped */
        *idx = -1;
    } else {
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("DDoS service tcam write failure, "
                          "idx : {}, err : {}", ret_idx, ret);
            return ret;
        }
    }
    HAL_TRACE_DEBUG("{} ret-idx : {}, ret: {}",
                    __FUNCTION__, ret_idx, ret);
    *idx = (int) ret_idx;
    return ret;
}

hal_ret_t
dos_pd_program_ddos_src_dst_tcam (ip_addr_t *src_ip_addr,
                                  uint8_t src_pfxlen,
                                  ip_addr_t *dst_ip_addr,
                                  uint8_t dst_pfxlen, bool is_icmp,
                                  uint16_t dport, uint8_t proto,
                                  uint16_t vrf, int actionid,
                                  uint16_t policer_idx,
                                  p4pd_table_id tbl_id, int *idx)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    ddos_src_dst_swkey_t        key;
    ddos_src_dst_swkey_mask_t   mask;
    ddos_src_dst_actiondata     data;
    tcam                        *tcam = NULL;
    uint32_t                    ret_idx;
    ipv4_addr_t                 v4_mask = {0};
    ipv6_addr_t                 v6_mask = {0};

    tcam = g_hal_state_pd->tcam_table(tbl_id);
    HAL_ASSERT(tcam != NULL);
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    /* Return if both addresses are not v4/v6 */
    if (!((src_ip_addr->af == IP_AF_IPV4) && (dst_ip_addr->af == IP_AF_IPV4)) &&
       (!(src_ip_addr->af == IP_AF_IPV6) && (dst_ip_addr->af == IP_AF_IPV6))) {
        HAL_TRACE_DEBUG("{} src/dst networks are not same type (v4/v6)",
                        __FUNCTION__);
        return (ret);
    }

    if (src_ip_addr->af == IP_AF_IPV4) {
        memcpy(key.flow_lkp_metadata_lkp_src, &src_ip_addr->addr.v4_addr,
               sizeof(ipv4_addr_t));
        v4_mask = ipv4_prefix_len_to_mask(src_pfxlen);
        memcpy(mask.flow_lkp_metadata_lkp_src_mask, &v4_mask,
               sizeof(ipv4_addr_t));
    } else if (src_ip_addr->af == IP_AF_IPV6) {
        memcpy(key.flow_lkp_metadata_lkp_src, &src_ip_addr->addr.v6_addr,
               IP6_ADDR8_LEN);
        memrev(key.flow_lkp_metadata_lkp_src,
               sizeof(key.flow_lkp_metadata_lkp_src));
        ipv6_prefix_len_to_mask(&v6_mask, src_pfxlen);
        memcpy(mask.flow_lkp_metadata_lkp_src_mask, &src_ip_addr->addr.v6_addr,
               IP6_ADDR8_LEN);
        memrev(mask.flow_lkp_metadata_lkp_src_mask,
               sizeof(mask.flow_lkp_metadata_lkp_src_mask));
    }
    if (dst_ip_addr->af == IP_AF_IPV4) {
        memcpy(key.flow_lkp_metadata_lkp_dst, &dst_ip_addr->addr.v4_addr,
               sizeof(ipv4_addr_t));
        v4_mask = ipv4_prefix_len_to_mask(dst_pfxlen);
        memcpy(mask.flow_lkp_metadata_lkp_dst_mask, &v4_mask,
               sizeof(ipv4_addr_t));
    } else if (dst_ip_addr->af == IP_AF_IPV6) {
        memcpy(key.flow_lkp_metadata_lkp_dst, &dst_ip_addr->addr.v6_addr,
               IP6_ADDR8_LEN);
        memrev(key.flow_lkp_metadata_lkp_dst,
               sizeof(key.flow_lkp_metadata_lkp_dst));
        ipv6_prefix_len_to_mask(&v6_mask, dst_pfxlen);
        memcpy(mask.flow_lkp_metadata_lkp_dst_mask, &dst_ip_addr->addr.v6_addr,
               IP6_ADDR8_LEN);
        memrev(mask.flow_lkp_metadata_lkp_dst_mask,
               sizeof(mask.flow_lkp_metadata_lkp_dst_mask));
    }
    if (is_icmp || (dport != 0)) {
        key.flow_lkp_metadata_lkp_dport = dport;
        mask.flow_lkp_metadata_lkp_dport_mask = 0xFFFF;
    }
    if (proto != 0) {
        key.flow_lkp_metadata_lkp_proto = proto;
        mask.flow_lkp_metadata_lkp_proto_mask = 0xFF;
    }
    /* LSB 12-bits is L2seg hw-id which is masked off */
    uint16_t vrf_mask = 0xFFFF;
    key.flow_lkp_metadata_lkp_vrf = (vrf << 12);
    mask.flow_lkp_metadata_lkp_vrf_mask = (vrf_mask << 12);
    key.entry_inactive_ddos_src_dst = 0;
    mask.entry_inactive_ddos_src_dst_mask = 0xFF;

    data.actionid = actionid;
    data.ddos_src_dst_action_u.ddos_src_dst_ddos_src_dst_hit.ddos_src_dst_base_policer_idx = policer_idx;

    HAL_TRACE_DEBUG("{} src-ip: {} src-pfxlen: {} dst-ip: {} dst-pfxlen: {} "
                    "dport: {} proto: {} vrf: {} act_id: {} pol_idx: {}",
                     __FUNCTION__, *src_ip_addr, src_pfxlen, *dst_ip_addr,
                     dst_pfxlen, dport, proto, vrf, actionid, policer_idx);
    sdk_ret = tcam->insert(&key, &mask, &data, &ret_idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret == HAL_RET_DUP_INS_FAIL) {
        /* Entry already exists. Can be skipped */
        *idx = -1;
    } else {
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("DDoS service tcam write failure, "
                          "idx : {}, err : {}", ret_idx, ret);
            return ret;
        }
    }

    HAL_TRACE_DEBUG("{} ret-idx: {}, ret: {}",
                    __FUNCTION__, ret_idx, ret);
    *idx = (int) ret_idx;
    return ret;
}

#define DDOS_POLICER_ACTION(_arg) d.ddos_service_policer_action_action_u.ddos_service_policer_action_ddos_service_policer_action._arg
hal_ret_t
dos_pd_program_ddos_policer_action (uint8_t actionid, uint8_t saved_color,
                                    uint32_t dropped_pkts, p4pd_table_id tbl_id,
                                    uint32_t *idx)
{
    hal_ret_t                           ret = HAL_RET_OK;
    sdk_ret_t                           sdk_ret;
    directmap                           *dm;
    ddos_service_policer_action_actiondata d = { 0 };

    dm = g_hal_state_pd->dm_table(tbl_id);
    HAL_ASSERT(dm != NULL);

    d.actionid = actionid;
    DDOS_POLICER_ACTION(ddos_service_policer_saved_color) = saved_color;
    DDOS_POLICER_ACTION(ddos_service_policer_dropped_packets) = dropped_pkts;
    sdk_ret = dm->insert(&d, idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("ddos policer action table write failure, "
                      "tbl_id: {} err : {}", tbl_id, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("{} saved_color: {} dropped_pkts: {} idx: {}",
                    __FUNCTION__, saved_color, dropped_pkts, *idx);
    return ret;
}

#define DDOS_POLICER(_arg) d.ddos_service_policer_action_u.ddos_service_policer_execute_ddos_service_policer._arg
hal_ret_t
dos_pd_program_ddos_policer (uint8_t actionid, bool pps,
                             bool color_aware, uint32_t cir, uint32_t cbr,
                             uint32_t pir, uint32_t pbr,
                             p4pd_table_id tbl_id, uint32_t *idx)
{
    hal_ret_t                           ret = HAL_RET_OK;
    sdk_ret_t                           sdk_ret;
    directmap                           *dm;
    ddos_service_policer_actiondata     d = { 0 };
    uint32_t                            tbkt;

    dm = g_hal_state_pd->dm_table(tbl_id);
    HAL_ASSERT(dm != NULL);

    /* TODO: Hardcode number of tokens. Only for model testing
     * Token bucket value should be zero since we want to allow one packet
     * The second packet will get dropped when the token bucket goes negative
     * Token bucket setting will be removed for real hw/emulator
     * */
    tbkt = 0;
    /* For color aware policers the commit token bucket update is color blind
     * and the peak token bucket update is color aware
     * */
    d.actionid = actionid;
    DDOS_POLICER(entry_valid) = 1;
    DDOS_POLICER(pkt_rate) = (pps) ? 1 : 0;
    DDOS_POLICER(color_aware) = 0;
    memcpy(DDOS_POLICER(rate), &cir, sizeof(uint32_t));
    memcpy(DDOS_POLICER(burst), &cbr, sizeof(uint32_t));
    memcpy(DDOS_POLICER(tbkt), &tbkt, sizeof(uint32_t));

    DDOS_POLICER(entry_valid2) = 1;
    DDOS_POLICER(pkt_rate2) = (pps) ? 1 : 0;
    DDOS_POLICER(color_aware2) = 1;
    memcpy(DDOS_POLICER(rate2), &pir, sizeof(uint32_t));
    memcpy(DDOS_POLICER(burst2), &pbr, sizeof(uint32_t));
    memcpy(DDOS_POLICER(tbkt2), &tbkt, sizeof(uint32_t));

    sdk_ret = dm->insert(&d, idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("ddos policer table write failure, tbl_id: {} err : {}",
                       tbl_id, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("{} pps: {} color_aware: {} cir: {} cbr: {}"
                    "pir: {} pbr: {} ret-pol-idx: {}", __FUNCTION__, pps,
                    color_aware, cir, cbr, pir, pbr, *idx);
    return ret;
}

/* Index returned will be the base policer index which must be used to
 * program the ddos tcam table result */
hal_ret_t
dos_pd_program_ddos_policers (dos_policy_prop_t *dospp, uint8_t actionid,
                              uint8_t pol_actionid, p4pd_table_id tbl_id,
                              p4pd_table_id pol_action_tbl_id,
                              uint32_t *base_pol_idx)
{
    uint32_t    ret_idx;
    uint32_t    ret_idx_action;
    hal_ret_t   ret;

    /* Program 4 policers */
    /* DDOS ANY Policer */
    ret = dos_pd_program_ddos_policer(actionid, TRUE, TRUE,
                               dospp->other_flood_limits.protect_pps,
                               dospp->other_flood_limits.protect_burst_pps,
                               dospp->other_flood_limits.restrict_pps,
                               dospp->other_flood_limits.restrict_burst_pps,
                               tbl_id, &ret_idx);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);
    /* DDOS ANY Policer Action */
    ret = dos_pd_program_ddos_policer_action(pol_actionid, 0, 0,
                                             pol_action_tbl_id,
                                             &ret_idx_action);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);
    /*
     * DDoS policers are always programmed in groups of four. The base
     * policer index is used in the ddos tcam action
     */
    *base_pol_idx = ret_idx / 4;

    /* DDOS TCP Policer */
    ret = dos_pd_program_ddos_policer(actionid, TRUE, TRUE,
                               dospp->tcp_syn_flood_limits.protect_pps,
                               dospp->tcp_syn_flood_limits.protect_burst_pps,
                               dospp->tcp_syn_flood_limits.restrict_pps,
                               dospp->tcp_syn_flood_limits.restrict_burst_pps,
                               tbl_id, &ret_idx);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);
    /* DDOS TCP Policer Action */
    ret = dos_pd_program_ddos_policer_action(pol_actionid, 0, 0,
                                             pol_action_tbl_id,
                                             &ret_idx_action);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);

    /* DDOS ICMP Policer */
    ret = dos_pd_program_ddos_policer(actionid, TRUE, TRUE,
                               dospp->icmp_flood_limits.protect_pps,
                               dospp->icmp_flood_limits.protect_burst_pps,
                               dospp->icmp_flood_limits.restrict_pps,
                               dospp->icmp_flood_limits.restrict_burst_pps,
                               tbl_id, &ret_idx);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);
    /* DDOS ICMP Policer Action */
    ret = dos_pd_program_ddos_policer_action(pol_actionid, 0, 0,
                                             pol_action_tbl_id,
                                             &ret_idx_action);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);

    /* DDOS UDP Policer */
    ret = dos_pd_program_ddos_policer(actionid, TRUE, TRUE,
                               dospp->udp_flood_limits.protect_pps,
                               dospp->udp_flood_limits.protect_burst_pps,
                               dospp->udp_flood_limits.restrict_pps,
                               dospp->udp_flood_limits.restrict_burst_pps,
                               tbl_id, &ret_idx);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);
    /* DDOS UDP Policer Action */
    ret = dos_pd_program_ddos_policer_action(pol_actionid, 0, 0,
                                             pol_action_tbl_id,
                                             &ret_idx_action);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);

    return ret;
}

hal_ret_t
dos_pd_program_ddos_src_vf_table (pd_dos_policy_t *pd_dosp, ep_t *ep, if_t *intf)
{
    int             tcam_idx;
    uint32_t        srclport;
    uint32_t        base_pol_idx;
    hal_ret_t       ret;
    dos_policy_t    *dosp;

    srclport = if_get_lport_id(intf);
    dosp = (dos_policy_t *) pd_dosp->pi_dos_policy;
    HAL_ASSERT(dosp != NULL);
    if (!dosp->egr_pol_valid) {
        HAL_TRACE_DEBUG("{}: DoS egress policy is not valid. "
                        "Skip src_vf table programming",
                        __FUNCTION__);
        return HAL_RET_OK;
    }
    /* Program the policers */
    ret = dos_pd_program_ddos_policers(&dosp->egress,
                           DDOS_SRC_VF_POLICER_EXECUTE_DDOS_SRC_VF_POLICER_ID,
                           DDOS_SRC_VF_POLICER_ACTION_DDOS_SRC_VF_POLICER_ACTION_ID,
                           P4TBL_ID_DDOS_SRC_VF_POLICER,
                           P4TBL_ID_DDOS_SRC_VF_POLICER_ACTION, &base_pol_idx);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);

    ret = dos_pd_program_ddos_src_vf_tcam(srclport, DDOS_SRC_VF_DDOS_SRC_VF_HIT_ID,
                                          base_pol_idx,
                                          P4TBL_ID_DDOS_SRC_VF, &tcam_idx);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);
    HAL_TRACE_DEBUG("{}: tcam_index: {}",
                     tcam_idx, __FUNCTION__);
    pd_dosp->ddos_src_vf_hw_id = tcam_idx;
    pd_dosp->ddos_src_vf_pol_hw_id = base_pol_idx;

    return (ret);
}

hal_ret_t
dos_pd_program_ddos_src_dst_table (dos_policy_t *dosp,
                                   dos_policy_prop_t *dosp_prop,
                                   pd_dos_policy_t *pd_dosp,
                                   dos_policy_t *pi_dosp, pd_vrf_t *ten_pd)
{
    int             tcam_idx;
    uint16_t        dport;
    uint32_t        base_pol_idx;
    hal_ret_t       ret = HAL_RET_OK;
    network_t       *nw, *pnw;
    dllist_ctxt_t   *curr, *next;
    dllist_ctxt_t   *nwcurr, *nwnext;
    dllist_ctxt_t   *pnwcurr, *pnwnext;
    dllist_ctxt_t   *nw_list, *pnw_list, *sg_list;
    dos_policy_sg_list_entry_t  *ent;
    hal_handle_id_list_entry_t  *nw_ent = NULL;
    hal_handle_id_list_entry_t  *pnw_ent = NULL;

    /* Program the policers */
    ret = dos_pd_program_ddos_policers(dosp_prop,
                    DDOS_SRC_DST_POLICER_EXECUTE_DDOS_SRC_DST_POLICER_ID,
                    DDOS_SRC_DST_POLICER_ACTION_DDOS_SRC_DST_POLICER_ACTION_ID,
                    P4TBL_ID_DDOS_SRC_DST_POLICER,
                    P4TBL_ID_DDOS_SRC_DST_POLICER_ACTION, &base_pol_idx);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);
    HAL_TRACE_DEBUG("{}: base_policer_index: {}",
                     base_pol_idx, __FUNCTION__);
    sg_list = &dosp->sg_list_head;
    /* Go through all the security groups */
    HAL_TRACE_DEBUG("{}: Iterating through all SG", __FUNCTION__);
    dllist_for_each_safe(curr, next, sg_list) {
        ent = dllist_entry(curr, dos_policy_sg_list_entry_t, dllist_ctxt);
        nw_list = get_nw_list_for_security_group(ent->sg_id);
        HAL_TRACE_DEBUG("{}: SG-id: {}", __FUNCTION__, ent->sg_id);
        HAL_TRACE_DEBUG("{}: Iterating through all NWs", __FUNCTION__);
        /* For each security group, go through the associated Networks */
        dllist_for_each_safe(nwcurr, nwnext, nw_list) {
            nw_ent = dllist_entry(nwcurr, hal_handle_id_list_entry_t, dllist_ctxt);
            nw = find_network_by_handle(nw_ent->handle_id);
            HAL_ASSERT(nw != NULL);
            HAL_TRACE_DEBUG("{}: NW pfx: {}/{}", __FUNCTION__,
                            nw->nw_key.ip_pfx.addr, nw->nw_key.ip_pfx.len);
            /* Get the peer Security Group */
            pnw_list = get_nw_list_for_security_group(dosp_prop->peer_sg_id);
            HAL_TRACE_DEBUG("{}: Peer sg-id: {}", __FUNCTION__,
                            dosp_prop->peer_sg_id);
            HAL_TRACE_DEBUG("{}: Iterating through all peer NWs", __FUNCTION__);
            /* Go through the associated peer networks */
            dllist_for_each_safe(pnwcurr, pnwnext, pnw_list) {
                pnw_ent = dllist_entry(pnwcurr, hal_handle_id_list_entry_t, dllist_ctxt);
                pnw = find_network_by_handle(pnw_ent->handle_id);
                HAL_ASSERT(pnw != NULL);
                HAL_TRACE_DEBUG("{}: Peer-NW pfx: {}/{}", __FUNCTION__,
                                pnw->nw_key.ip_pfx.addr, pnw->nw_key.ip_pfx.len);
                HAL_TRACE_DEBUG("{}: is_icmp: {} type: {} code: {}",
                                __FUNCTION__, dosp_prop->service.is_icmp,
                                dosp_prop->service.icmp_msg_type,
                                dosp_prop->service.icmp_msg_code);
                dport = (dosp_prop->service.is_icmp) ?
                        ((dosp_prop->service.icmp_msg_type << 8) |
                        (dosp_prop->service.icmp_msg_code)) :
                        dosp_prop->service.dport;
                ret = dos_pd_program_ddos_src_dst_tcam(&nw->nw_key.ip_pfx.addr,
                              nw->nw_key.ip_pfx.len, &pnw->nw_key.ip_pfx.addr,
                              pnw->nw_key.ip_pfx.len,
                              dosp_prop->service.is_icmp,
                              dport, dosp_prop->service.ip_proto,
                              ten_pd->vrf_hw_id, DDOS_SRC_DST_DDOS_SRC_DST_HIT_ID,
                              base_pol_idx, P4TBL_ID_DDOS_SRC_DST,
                              &tcam_idx);
                HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);
                HAL_TRACE_DEBUG("{}: tcam_index: {}", tcam_idx,
                                __FUNCTION__);
            }
        }
    }

    return (ret);
}

hal_ret_t
dos_pd_program_ddos_src_dst_policy (pd_dos_policy_t *pd_dosp,
                                   dos_policy_t *pi_dosp,
                                   pd_vrf_t *ten_pd)
{
    hal_ret_t       ret = HAL_RET_OK;
    dos_policy_t    *dosp;

    dosp = (dos_policy_t *) pd_dosp->pi_dos_policy;
    HAL_ASSERT(dosp != NULL);
    /* Program the src-dst subnets if ingress policy is valid */
    HAL_TRACE_DEBUG("{}: ingr_pol_valid: {} ingress.peer_sg_id: {}",
                    __FUNCTION__, dosp->ingr_pol_valid, dosp->ingress.peer_sg_id);
    if (dosp->ingr_pol_valid &&
            (dosp->ingress.peer_sg_id != HAL_NWSEC_INVALID_SG_ID)) {
        ret = dos_pd_program_ddos_src_dst_table (dosp, &dosp->ingress, pd_dosp,
                                                 pi_dosp, ten_pd);
        HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);
    }

    /* Program the src-dst subnets if egress policy is valid */
    HAL_TRACE_DEBUG("{}: egr_pol_valid: {} egress.peer_sg_id: {}",
                    __FUNCTION__, dosp->egr_pol_valid, dosp->egress.peer_sg_id);
    if (dosp->egr_pol_valid &&
            (dosp->egress.peer_sg_id != HAL_NWSEC_INVALID_SG_ID)) {
        ret = dos_pd_program_ddos_src_dst_table (dosp, &dosp->egress, pd_dosp,
                                                 pi_dosp, ten_pd);
        HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);
    }
    return (ret);
}

hal_ret_t
dos_pd_program_ddos_service_table (pd_dos_policy_t *pd_dosp,
                                   ep_t *ep, pd_vrf_t *ten_pd)
{
    int                         tcam_idx;
    uint16_t                    dport;
    uint32_t                    base_pol_idx;
    hal_ret_t                   ret;
    dos_policy_t                *dosp;
    dllist_ctxt_t               *curr, *next, *ip_list;
    ep_ip_entry_t               *pi_ip_ent = NULL;

    dosp = (dos_policy_t *) pd_dosp->pi_dos_policy;
    HAL_ASSERT(dosp != NULL);
    if (!dosp->ingr_pol_valid) {
        HAL_TRACE_DEBUG("{}: DoS ingress policy is not valid. ",
                        "Skip service table programming",
                        __FUNCTION__);
        return HAL_RET_OK;
    }

    /* Program the policers */
    ret = dos_pd_program_ddos_policers(&dosp->ingress,
                           DDOS_SERVICE_POLICER_EXECUTE_DDOS_SERVICE_POLICER_ID,
                           DDOS_SERVICE_POLICER_ACTION_DDOS_SERVICE_POLICER_ACTION_ID,
                           P4TBL_ID_DDOS_SERVICE_POLICER,
                           P4TBL_ID_DDOS_SERVICE_POLICER_ACTION, &base_pol_idx);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);
    HAL_TRACE_DEBUG("{}: service_tbl. base_policer_index: {}",
                     base_pol_idx, __FUNCTION__);
    ip_list = &ep->ip_list_head;
    /* Iterate over all the IP addresses for this endpoint */
    dllist_for_each_safe(curr, next, ip_list) {
        pi_ip_ent = dllist_entry(curr, ep_ip_entry_t, ep_ip_lentry);
        HAL_TRACE_DEBUG("{}: ep_ip_addr: {}",
                         __FUNCTION__, pi_ip_ent->ip_addr);
        HAL_TRACE_DEBUG("{}: is_icmp: {} type: {} code: {}",
                        __FUNCTION__, dosp->ingress.service.is_icmp,
                        dosp->ingress.service.icmp_msg_type,
                        dosp->ingress.service.icmp_msg_code);
        dport = (dosp->ingress.service.is_icmp) ?
                ((dosp->ingress.service.icmp_msg_type << 8) |
                (dosp->ingress.service.icmp_msg_code)) : dosp->ingress.service.dport;
        ret = dos_pd_program_ddos_service_tcam(&pi_ip_ent->ip_addr,
                dosp->ingress.service.is_icmp,
                dport, dosp->ingress.service.ip_proto,
                ten_pd->vrf_hw_id, DDOS_SERVICE_DDOS_SERVICE_HIT_ID,
                base_pol_idx, P4TBL_ID_DDOS_SERVICE, &tcam_idx);
        HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);
    }

    HAL_TRACE_DEBUG("{}: tcam_index: {}",
                     tcam_idx, __FUNCTION__);
    /* One IP addr per EP for now */
    pd_dosp->ddos_service_hw_id = tcam_idx;
    pd_dosp->ddos_service_pol_hw_id = base_pol_idx;

    return (ret);
}

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
hal_ret_t
dos_pd_program_hw (pd_dos_policy_t *pd_dosp, bool create)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dos_policy_t                *dp = NULL;
    dllist_ctxt_t               *curr, *next, *ep_list;
    dllist_ctxt_t               *sg_list;
    dllist_ctxt_t               *epcurr, *epnext;
    dos_policy_sg_list_entry_t  *ent = NULL;
    ep_t                        *ep = NULL;
    hal_handle_id_list_entry_t  *ep_ent = NULL;
    if_t                        *intf = NULL;
    vrf_t                    *ten;
    pd_vrf_t                 *ten_pd;

    dp = (dos_policy_t *) pd_dosp->pi_dos_policy;
    ten = vrf_lookup_by_handle(dp->vrf_handle);
    if (ten == NULL) {
        HAL_TRACE_ERR("{}:unable to find vrf", __FUNCTION__);
        ret = HAL_RET_VRF_NOT_FOUND;
        goto end;
    }
    HAL_ASSERT(ten->pd != NULL);
    ten_pd = (pd_vrf_t *) ten->pd;

    sg_list = &dp->sg_list_head;
    HAL_TRACE_DEBUG("{}: Going through the linked security groups",
                     __FUNCTION__);
    /* Get the security groups linked to this dos policy */
    dllist_for_each_safe(curr, next, sg_list) {
        ent = dllist_entry(curr, dos_policy_sg_list_entry_t, dllist_ctxt);
        HAL_TRACE_DEBUG("{}: SG-id: {}", __FUNCTION__, ent->sg_id);
        /* For each security group, go through the associated EPs */
        ep_list = get_ep_list_for_security_group(ent->sg_id);
        dllist_for_each_safe(epcurr, epnext, ep_list) {
            ep_ent = dllist_entry(epcurr, hal_handle_id_list_entry_t, dllist_ctxt);
            ep = find_ep_by_handle(ep_ent->handle_id);
            HAL_ASSERT(ep != NULL);
            HAL_TRACE_DEBUG("{}: Got EP handle {}", __FUNCTION__,
                             ep_ent->handle_id);
            intf = ep_get_if(ep);
            HAL_ASSERT(intf != NULL);
            HAL_TRACE_DEBUG("{}: Intf-id: {}", __FUNCTION__, intf->if_id);
            /*
             * TODO: Can program a single set of policers for all EPs since the
             * policer rates are the same. Using different policers for DOL.
             * Revisit and fix
             * */
            HAL_TRACE_DEBUG("{}: Program src-vf table", __FUNCTION__);
            /* Program the DDoS Src VF table */
            ret = dos_pd_program_ddos_src_vf_table (pd_dosp, ep, intf);
            HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);
            HAL_TRACE_DEBUG("{}: Program service table", __FUNCTION__);
            /* Program the DDoS service table */
            ret = dos_pd_program_ddos_service_table (pd_dosp, ep, ten_pd);
            HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);
        }
    }

    // Program DDoS src-dst policer
    ret = dos_pd_program_ddos_src_dst_policy (pd_dosp, dp, ten_pd);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);

end:
    return ret;
}

// ----------------------------------------------------------------------------
// DeProgram HW
// ----------------------------------------------------------------------------
hal_ret_t
dos_pd_deprogram_hw (pd_dos_policy_t *dos_pd)
{
    hal_ret_t            ret = HAL_RET_OK;

    // De-program DDoS service table
    // De-program DDoS src_vrf table
    // De-program DDoS src-dst table

    // De-program DDoS service policer
    // De-program DDoS src_vrf policer
    // De-program DDoS src-dst policer
    // De-program Input properties Table

    return ret;
}

// ----------------------------------------------------------------------------
// Allocate resources for PD Uplink if
// ----------------------------------------------------------------------------
hal_ret_t
dos_pd_alloc_res(pd_dos_policy_t *pd_dosp)
{
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// De-Allocate resources for PD Uplink if
// ----------------------------------------------------------------------------
hal_ret_t
dos_pd_dealloc_res(pd_dos_policy_t *pd_dosp)
{
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// PD Cleanup
//  - Release all resources
//  - Delink PI <-> PD
//  - Free PD Vrf
//  Note:
//      - Just free up whatever PD has.
//      - Dont use this inplace of delete. Delete may result in giving callbacks
//        to others.
//-----------------------------------------------------------------------------
hal_ret_t
dos_pd_cleanup(pd_dos_policy_t *dos_pd)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!dos_pd) {
        // Nothing to do
        goto end;
    }

    // Releasing resources
    ret = dos_pd_dealloc_res(dos_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: unable to dealloc res for dos hdl: {}",
                      __FUNCTION__,
                      ((dos_policy_t*)(dos_pd->pi_dos_policy))->hal_handle);
        goto end;
    }

    // Delinking PI<->PD
    dos_delink_pi_pd(dos_pd, (dos_policy_t *)dos_pd->pi_dos_policy);

    // Freeing PD
    dos_pd_free(dos_pd);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
void
dos_link_pi_pd(pd_dos_policy_t *pd_dosp, dos_policy_t *pi_nw)
{
    pd_dosp->pi_dos_policy = pi_nw;
    dos_set_pd_dos(pi_nw, pd_dosp);
}

// ----------------------------------------------------------------------------
// Un-Linking PI <-> PD
// ----------------------------------------------------------------------------
void
dos_delink_pi_pd(pd_dos_policy_t *pd_dosp, dos_policy_t  *pi_nw)
{
    pd_dosp->pi_dos_policy = NULL;
    dos_set_pd_dos(pi_nw, NULL);
}

// ----------------------------------------------------------------------------
// Makes a clone
// ----------------------------------------------------------------------------
hal_ret_t
// pd_dos_policy_make_clone(dos_policy_t *dosp, dos_policy_t *clone)
pd_dos_policy_make_clone(pd_dos_policy_make_clone_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_dos_policy_t     *pd_dosp_clone = NULL;
    dos_policy_t *dosp = args->dos_policy;
    dos_policy_t *clone = args->clone_policy;

    pd_dosp_clone = dos_pd_alloc_init();
    if (pd_dosp_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_dosp_clone, dosp->pd, sizeof(pd_dos_policy_t));

    dos_link_pi_pd(pd_dosp_clone, clone);

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Frees PD memory without indexer free.
// ----------------------------------------------------------------------------
hal_ret_t
pd_dos_policy_mem_free(pd_dos_policy_mem_free_args_t *args)
{
    pd_dos_policy_t       *dos_pd;
    hal_ret_t             ret = HAL_RET_OK;

    dos_pd = (pd_dos_policy_t *)args->dos_policy->pd;
    dos_pd_mem_free(dos_pd);

    return ret;
}

}    // namespace pd
}    // namespace hal
