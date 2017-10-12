#include "nic/hal/src/tenant.hpp"
#include "nic/hal/pd/iris/tenant_pd.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/acl_pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/acl_api.hpp"
#include "nic/gen/iris/include/p4pd.h"
#include "nic/p4/nw/include/defines.h"
#include "nic/hal/pd/iris/if_pd_utils.hpp"
#include "nic/hal/src/l2segment.hpp"
#include "nic/hal/pd/iris/l2seg_pd.hpp"
#include "nic/include/l2segment_api.hpp"
#include "nic/include/eth.h"

namespace hal {
namespace pd {
// ----------------------------------------------------------------------------
// Allocate Acl Instance
// ----------------------------------------------------------------------------
static inline pd_acl_t *
acl_pd_alloc (void)
{
    pd_acl_t    *acl;

    acl = (pd_acl_t *)g_hal_state_pd->acl_pd_slab()->alloc();
    if (acl == NULL) {
        return NULL;
    }
    return acl;
}

// ----------------------------------------------------------------------------
// Initialize Acl PD instance
// ----------------------------------------------------------------------------
static inline pd_acl_t *
acl_pd_init (pd_acl_t *acl)
{
    // Nothing to do currently
    if (!acl) {
        return NULL;
    }

    // Set here if you want to initialize any fields

    return acl;
}

// ----------------------------------------------------------------------------
// Allocate and Initialize Acl PD Instance
// ----------------------------------------------------------------------------
static inline pd_acl_t *
acl_pd_alloc_init (void)
{
    return acl_pd_init(acl_pd_alloc());
}

// ----------------------------------------------------------------------------
// Freeing Acl PD
// ----------------------------------------------------------------------------
static hal_ret_t
acl_pd_free (pd_acl_t **pd_acl_)
{
    pd_acl_t *pd_acl = *pd_acl_;
    if (pd_acl) {
        g_hal_state_pd->acl_pd_slab()->free(pd_acl);
    }
    *pd_acl_ = NULL;
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
static void 
link_pi_pd(pd_acl_t *pd_acl, acl_t *pi_acl)
{
    pd_acl->pi_acl = pi_acl;
    acl_set_pd_acl(pi_acl, pd_acl);
}

// ----------------------------------------------------------------------------
// Un-Linking PI <-> PD
// ----------------------------------------------------------------------------
static void 
unlink_pi_pd(pd_acl_t *pd_acl, acl_t *pi_acl)
{
    pd_acl->pi_acl = NULL;
    acl_set_pd_acl(pi_acl, NULL);
}

// ----------------------------------------------------------------------------
// Allocate resources for PD Acl
// ----------------------------------------------------------------------------
static hal_ret_t 
acl_pd_alloc_res(pd_acl_t *pd_acl)
{
    hal_ret_t            ret = HAL_RET_OK;

    // Allocate any hardware resources

    return ret;
}

// ----------------------------------------------------------------------------
// Deallocate resources for Acl 
// ----------------------------------------------------------------------------
static void
acl_pd_dealloc_res (pd_acl_t *pd_acl)
{
    acl_t *acl = (acl_t*)pd_acl->pi_acl;

    if (!acl) {
        return;
    }

    // Deallocate any hardware resources 
}

static void
populate_ip_common (nacl_swkey_t *key, nacl_swkey_mask_t *mask,
                    acl_ip_match_spec_t *ip_key, acl_ip_match_spec_t *ip_mask)
{
    key->flow_lkp_metadata_lkp_proto = ip_key->ip_proto;
    mask->flow_lkp_metadata_lkp_proto_mask = ip_mask->ip_proto;

    switch(ip_key->ip_proto) {
        case IP_PROTO_ICMP:
        case IP_PROTO_ICMPV6:
            key->flow_lkp_metadata_lkp_sport = 
                (ip_key->u.icmp.icmp_type << 8) | ip_key->u.icmp.icmp_code;
            mask->flow_lkp_metadata_lkp_sport_mask = 
                (ip_mask->u.icmp.icmp_type << 8) | ip_mask->u.icmp.icmp_code;
            break;
        case IP_PROTO_TCP:
            key->flow_lkp_metadata_lkp_sport = ip_key->u.tcp.sport;
            mask->flow_lkp_metadata_lkp_sport_mask = ip_mask->u.tcp.sport;

            key->flow_lkp_metadata_lkp_dport = ip_key->u.tcp.dport;
            mask->flow_lkp_metadata_lkp_dport_mask = ip_mask->u.tcp.dport;

            key->tcp_flags = ip_key->u.tcp.tcp_flags;
            mask->tcp_flags_mask = ip_mask->u.tcp.tcp_flags;
            break;
        case IP_PROTO_UDP:
            key->flow_lkp_metadata_lkp_sport = ip_key->u.udp.sport;
            mask->flow_lkp_metadata_lkp_sport_mask = ip_mask->u.udp.sport;

            key->flow_lkp_metadata_lkp_dport = ip_key->u.udp.dport;
            mask->flow_lkp_metadata_lkp_dport_mask = ip_mask->u.udp.dport;
            break;
        default:
            break;
    }
}

static void
populate_permit_actions (nacl_actiondata *data, acl_action_spec_t *as)
{
    // TODO Get the index from copp policer handle and mirror session
    // handles
    data->nacl_action_u.nacl_nacl_permit.policer_index = 
        as->copp_policer_handle;
    data->nacl_action_u.nacl_nacl_permit.ingress_mirror_en = 
        as->ing_mirror_en;
    data->nacl_action_u.nacl_nacl_permit.egress_mirror_en = 
        as->egr_mirror_en;
    data->nacl_action_u.nacl_nacl_permit.ingress_mirror_session_id = 
        as->ing_mirror_session_handle;
    data->nacl_action_u.nacl_nacl_permit.egress_mirror_session_id = 
        as->egr_mirror_session_handle;

    data->nacl_action_u.nacl_nacl_permit.ingress_mirror_session_id = 
        as->ing_mirror_session;
    data->nacl_action_u.nacl_nacl_permit.egress_mirror_session_id = 
        as->egr_mirror_session;
    // TODO Populate these values
    data->nacl_action_u.nacl_nacl_permit.qid_en = 0;
    data->nacl_action_u.nacl_nacl_permit.qid = 0;
    data->nacl_action_u.nacl_nacl_permit.egress_policer_en = 0;
    data->nacl_action_u.nacl_nacl_permit.egress_policer_index = 0;
}

static hal_ret_t
acl_pd_pgm_acl_tbl (pd_acl_t *pd_acl)
{
    hal_ret_t            ret = HAL_RET_OK;
    acl_tcam             *acl_tbl = NULL;
    nacl_swkey_t         key;
    nacl_swkey_mask_t    mask;
    nacl_actiondata      data;
    acl_match_spec_t     *ms = NULL;
    acl_action_spec_t    *as = NULL;
    acl_eth_match_spec_t *eth_key;
    acl_eth_match_spec_t *eth_mask;
    acl_ip_match_spec_t  *ip_key;
    acl_ip_match_spec_t  *ip_mask;
    acl_t                *pi_acl = (acl_t *)pd_acl->pi_acl;
    tenant_t             *tenant = NULL;
    l2seg_t              *l2seg = NULL;
    if_t                 *redirect_if = NULL;
    uint16_t             l2seg_mask = 0;
    uint16_t             ten_mask = 0;
    uint8_t              ten_shift = 0;

    ms = acl_get_match_spec(pi_acl);
    as = acl_get_action_spec(pi_acl);

    eth_key = &ms->key.eth;
    eth_mask = &ms->mask.eth;
    ip_key = &ms->key.ip;
    ip_mask = &ms->mask.ip;

    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    // Populate the data
    switch(as->action) {
        case acl::ACL_ACTION_LOG:
            data.actionid = NACL_NACL_PERMIT_ID;
            data.nacl_action_u.nacl_nacl_permit.log_en = 1;
            populate_permit_actions(&data, as);
            break;
        case acl::ACL_ACTION_REDIRECT:
            data.actionid = NACL_NACL_PERMIT_ID;

            redirect_if = find_if_by_handle(as->redirect_if_handle);
            data.nacl_action_u.nacl_nacl_permit.dst_lport_en = 1;
            data.nacl_action_u.nacl_nacl_permit.dst_lport = if_get_lport_id(redirect_if);

            if (if_is_cpu_if(redirect_if)) {
                // If going to CPU, do not do any rewrites on packet. So set
                // the rewrite indexes to 0 (nop)
                data.nacl_action_u.nacl_nacl_permit.force_flow_hit = 1;
                data.nacl_action_u.nacl_nacl_permit.rewrite_en = 1;
                data.nacl_action_u.nacl_nacl_permit.rewrite_index = 0;
                data.nacl_action_u.nacl_nacl_permit.rewrite_flags = 0;
                data.nacl_action_u.nacl_nacl_permit.tunnel_rewrite_en = 1;
                data.nacl_action_u.nacl_nacl_permit.tunnel_rewrite_index = 0;
                data.nacl_action_u.nacl_nacl_permit.tunnel_vnid = 0;
                data.nacl_action_u.nacl_nacl_permit.tunnel_originate = 0;
            } else {
                // TODO: Figure out how to get these values
                data.nacl_action_u.nacl_nacl_permit.force_flow_hit = 0;
                data.nacl_action_u.nacl_nacl_permit.rewrite_en = 0;
                data.nacl_action_u.nacl_nacl_permit.rewrite_index = 0;
                data.nacl_action_u.nacl_nacl_permit.rewrite_flags = 0;
                data.nacl_action_u.nacl_nacl_permit.tunnel_rewrite_en = 0;
                data.nacl_action_u.nacl_nacl_permit.tunnel_rewrite_index = 0;
                data.nacl_action_u.nacl_nacl_permit.tunnel_vnid = 0;
                data.nacl_action_u.nacl_nacl_permit.tunnel_originate = 0;
#ifdef ACL_DOL_TEST_ONLY
                data.nacl_action_u.nacl_nacl_permit.force_flow_hit = 1;
                data.nacl_action_u.nacl_nacl_permit.rewrite_en = 1;
                data.nacl_action_u.nacl_nacl_permit.rewrite_index = 
                    as->int_as.rw_idx;
                data.nacl_action_u.nacl_nacl_permit.rewrite_flags =
                    (as->int_as.mac_sa_rewrite ? REWRITE_FLAGS_MAC_SA : 0) |
                    (as->int_as.mac_da_rewrite ? REWRITE_FLAGS_MAC_DA : 0) |
                    (as->int_as.ttl_dec ? REWRITE_FLAGS_TTL_DEC : 0);

                data.nacl_action_u.nacl_nacl_permit.tunnel_rewrite_en = 1;
                if (if_is_tunnel_if(redirect_if)) {
                    data.nacl_action_u.nacl_nacl_permit.tunnel_rewrite_index = 
                        (pd_tunnelif_get_rw_idx((pd_tunnelif_t *)redirect_if->pd_if));
                    data.nacl_action_u.nacl_nacl_permit.tunnel_vnid = 
                        as->int_as.tnnl_vnid;
                    data.nacl_action_u.nacl_nacl_permit.tunnel_originate = 1;
                } else {
                    // support only non-native segments for DOL testing
                    data.nacl_action_u.nacl_nacl_permit.tunnel_vnid = 
                        as->int_as.tnnl_vnid;
                    data.nacl_action_u.nacl_nacl_permit.tunnel_rewrite_index = 
                        g_hal_state_pd->tnnl_rwr_tbl_encap_vlan_idx();
                }
#endif
            }

            populate_permit_actions(&data, as);
            break;
        case acl::ACL_ACTION_PERMIT:
            data.actionid = NACL_NACL_PERMIT_ID;
            populate_permit_actions(&data, as);
            break;
        case acl::ACL_ACTION_DENY:
            data.actionid = NACL_NACL_DENY_ID;
            break;
        default:
            data.actionid = NACL_NOP_ID;
            break;
    }

    // Populate the Key and Mask
    key.entry_inactive_nacl = 0;
    mask.entry_inactive_nacl_mask = 0x1;

    // Match only when from_cpu is 0
    key.control_metadata_from_cpu = 0;
    mask.control_metadata_from_cpu_mask = 1;

    pd_get_l2seg_ten_masks(&l2seg_mask, &ten_mask, &ten_shift);
    if (ms->tenant_match) {
        tenant = tenant_lookup_by_handle(ms->tenant_handle);
        key.flow_lkp_metadata_lkp_vrf = 
            ((pd_tenant_t *)(tenant->pd))->ten_hw_id << ten_shift;
        mask.flow_lkp_metadata_lkp_vrf_mask = ten_mask;
    } else if (ms->l2seg_match) {
        l2seg = find_l2seg_by_handle(ms->l2seg_handle);
        key.flow_lkp_metadata_lkp_vrf = 
            ((pd_l2seg_t *)(l2seg->pd))->l2seg_ten_hw_id;
        mask.flow_lkp_metadata_lkp_vrf_mask = ten_mask | l2seg_mask;
    }

    if (ms->src_if_match) {
        key.control_metadata_src_lport = 
            if_get_lport_id(find_if_by_handle(ms->src_if_handle));
        mask.control_metadata_src_lport_mask = 
            ~(mask.control_metadata_src_lport_mask & 0);
    }

    if (ms->dest_if_match) {
        key.control_metadata_dst_lport = 
            if_get_lport_id(find_if_by_handle(ms->dest_if_handle));
        mask.control_metadata_dst_lport_mask = 
            ~(mask.control_metadata_dst_lport_mask & 0);
    }

    switch(ms->acl_type) {
        case ACL_TYPE_NONE:
            // Nothing to do
            break;
        case ACL_TYPE_ETH:
            // Mac address needs to be in little-endian format while
            // passing to p4pd. So convert to uint64 and do a memcpy
            key.flow_lkp_metadata_lkp_type = FLOW_KEY_LOOKUP_TYPE_MAC;
            mask.flow_lkp_metadata_lkp_type_mask = 
                ~(mask.flow_lkp_metadata_lkp_type_mask & 0);

            memcpy(key.flow_lkp_metadata_lkp_src, eth_key->mac_sa, sizeof(mac_addr_t));
            memrev(key.flow_lkp_metadata_lkp_src, sizeof(mac_addr_t));
            memcpy(mask.flow_lkp_metadata_lkp_src_mask, eth_mask->mac_sa, sizeof(mac_addr_t));
            memrev(mask.flow_lkp_metadata_lkp_src_mask, sizeof(mac_addr_t));

            memcpy(key.flow_lkp_metadata_lkp_dst, eth_key->mac_da, sizeof(mac_addr_t));
            memrev(key.flow_lkp_metadata_lkp_dst, sizeof(mac_addr_t));
            memcpy(mask.flow_lkp_metadata_lkp_dst_mask, eth_mask->mac_da, sizeof(mac_addr_t));
            memrev(mask.flow_lkp_metadata_lkp_dst_mask, sizeof(mac_addr_t));

            key.flow_lkp_metadata_lkp_sport = eth_key->ether_type;
            mask.flow_lkp_metadata_lkp_sport_mask = eth_mask->ether_type;
            break;
        case ACL_TYPE_IP:
            key.flow_lkp_metadata_lkp_type = FLOW_KEY_LOOKUP_TYPE_IPV4 &
                                                FLOW_KEY_LOOKUP_TYPE_IPV6;
            mask.flow_lkp_metadata_lkp_type_mask = ~(FLOW_KEY_LOOKUP_TYPE_IPV4 ^ 
                                                    FLOW_KEY_LOOKUP_TYPE_IPV6);
            populate_ip_common(&key, &mask, ip_key, ip_mask);
            break;
        case ACL_TYPE_IPv4:
            key.flow_lkp_metadata_lkp_type = FLOW_KEY_LOOKUP_TYPE_IPV4;
            mask.flow_lkp_metadata_lkp_type_mask = 
                ~(mask.flow_lkp_metadata_lkp_type_mask & 0);

            memcpy(key.flow_lkp_metadata_lkp_src, 
                   &ip_key->sip.addr.v4_addr, 
                   sizeof(ipv4_addr_t));
            memcpy(mask.flow_lkp_metadata_lkp_src_mask, 
                   &ip_mask->sip.addr.v4_addr, 
                   sizeof(ipv4_addr_t));

            memcpy(key.flow_lkp_metadata_lkp_dst, 
                   &ip_key->dip.addr.v4_addr, 
                   sizeof(ipv4_addr_t));
            memcpy(mask.flow_lkp_metadata_lkp_dst_mask, 
                   &ip_mask->dip.addr.v4_addr, 
                   sizeof(ipv4_addr_t));

            populate_ip_common(&key, &mask, ip_key, ip_mask);
            break;
        case ACL_TYPE_IPv6:
            key.flow_lkp_metadata_lkp_type = FLOW_KEY_LOOKUP_TYPE_IPV6;
            mask.flow_lkp_metadata_lkp_type_mask = 
                ~(mask.flow_lkp_metadata_lkp_type_mask & 0);

            memcpy(key.flow_lkp_metadata_lkp_src, 
                   ip_key->sip.addr.v6_addr.addr8, 
                   IP6_ADDR8_LEN);
            memrev(key.flow_lkp_metadata_lkp_src, sizeof(key.flow_lkp_metadata_lkp_src));
            memcpy(mask.flow_lkp_metadata_lkp_src_mask, 
                   ip_mask->sip.addr.v6_addr.addr8, 
                   IP6_ADDR8_LEN);
            memrev(mask.flow_lkp_metadata_lkp_src_mask, sizeof(mask.flow_lkp_metadata_lkp_src_mask));

            memcpy(key.flow_lkp_metadata_lkp_dst, 
                   ip_key->dip.addr.v6_addr.addr8, 
                   IP6_ADDR8_LEN);
            memrev(key.flow_lkp_metadata_lkp_dst, sizeof(key.flow_lkp_metadata_lkp_dst));
            memcpy(mask.flow_lkp_metadata_lkp_dst_mask, 
                   ip_mask->dip.addr.v6_addr.addr8, 
                   IP6_ADDR8_LEN);
            memrev(mask.flow_lkp_metadata_lkp_dst_mask, sizeof(mask.flow_lkp_metadata_lkp_dst_mask));

            populate_ip_common(&key, &mask, ip_key, ip_mask);
            break;
        case ACL_TYPE_INVALID:
            return HAL_RET_ERR;
    }
    // TODO:Populate the below values
    key.control_metadata_flow_miss_ingress = 0;
    mask.control_metadata_flow_miss_ingress_mask = 0;
    key.ethernet_dstAddr[0] = 0;
    mask.ethernet_dstAddr_mask[0] = 0;
    key.l3_metadata_ip_option_seen = 0;
    mask.l3_metadata_ip_option_seen_mask = 0;
    key.l3_metadata_ip_frag = 0;
    mask.l3_metadata_ip_frag_mask = 0;
    key.control_metadata_drop_reason = 0;
    mask.control_metadata_drop_reason_mask = 0;
    key.flow_lkp_metadata_lkp_dir = 0;
    mask.flow_lkp_metadata_lkp_dir_mask = 0;

#ifdef ACL_DOL_TEST_ONLY
    // Key of internal fields for use only with DOL/testing infra
    // For production builds this needs to be removed
    // TODO: REMOVE
    key.control_metadata_flow_miss_ingress = ms->int_key.flow_miss;
    mask.control_metadata_flow_miss_ingress_mask = ms->int_mask.flow_miss;
    memcpy(key.ethernet_dstAddr, ms->int_key.outer_mac_da, sizeof(mac_addr_t));
    memrev(key.ethernet_dstAddr, sizeof(mac_addr_t));
    memcpy(mask.ethernet_dstAddr_mask, ms->int_mask.outer_mac_da, sizeof(mac_addr_t));
    memrev(mask.ethernet_dstAddr_mask, sizeof(mac_addr_t));
    key.l3_metadata_ip_option_seen = ms->int_key.ip_options;
    mask.l3_metadata_ip_option_seen_mask = ms->int_mask.ip_options;
    key.l3_metadata_ip_frag = ms->int_key.ip_frag;
    mask.l3_metadata_ip_frag_mask = ms->int_mask.ip_frag;
    key.control_metadata_drop_reason = ms->int_key.drop_reason;
    mask.control_metadata_drop_reason_mask = ms->int_mask.drop_reason;
    key.flow_lkp_metadata_lkp_dir = ms->int_key.direction;
    mask.flow_lkp_metadata_lkp_dir_mask = ms->int_mask.direction;
#endif

    acl_tbl = g_hal_state_pd->acl_table();
    HAL_ASSERT_RETURN((acl_tbl != NULL), HAL_RET_ERR);

    // Insert the entry
    ret = acl_tbl->insert(&key, &mask, &data, acl_get_priority(pi_acl), &pd_acl->handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-ACL::{}: Unable to program for nacl: {}",
                __FUNCTION__, acl_get_acl_id((acl_t *)pd_acl->pi_acl));
    } else {
        HAL_TRACE_DEBUG("PD-ACL::{}: Programmed for nacl: {}",
                __FUNCTION__, acl_get_acl_id((acl_t *)pd_acl->pi_acl));
    }

    return ret;
}

static hal_ret_t 
acl_pd_cleanup_acl_tbl (pd_acl_t *pd_acl)
{
    hal_ret_t ret = HAL_RET_OK;
    acl_tcam  *acl_tbl = NULL;
    
    acl_tbl = g_hal_state_pd->acl_table();
    HAL_ASSERT_RETURN((acl_tbl != NULL), HAL_RET_ERR);

    ret = acl_tbl->remove(pd_acl->handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-ACL::{}: Unable to cleanup for acl: {}",
                __FUNCTION__, acl_get_acl_id((acl_t *)pd_acl->pi_acl));
    } else {
        HAL_TRACE_DEBUG("PD-ACL::{}: Programmed cleanup acl: {}",
                __FUNCTION__, acl_get_acl_id((acl_t *)pd_acl->pi_acl));
    }

    return ret;
}

static hal_ret_t
acl_pd_deprogram_hw (pd_acl_t *pd_acl)
{
    hal_ret_t   ret = HAL_RET_OK;
    ret = acl_pd_cleanup_acl_tbl(pd_acl);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
static hal_ret_t
acl_pd_program_hw(pd_acl_t *pd_acl)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = acl_pd_pgm_acl_tbl(pd_acl);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Delete a PD ACL and remove from hardware
// ----------------------------------------------------------------------------
hal_ret_t
pd_acl_delete (pd_acl_args_t *args) 
{
    hal_ret_t ret;
    pd_acl_t  *pd_acl;

    HAL_TRACE_DEBUG("PD-ACL::{}: Deleting pd state for acl: {}", 
            __func__, acl_get_acl_id(args->acl));

    pd_acl = (pd_acl_t *)hal::acl_get_pd_acl(args->acl);

    // Program HW
    ret = acl_pd_deprogram_hw(pd_acl);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-ACL::{}: Error in deprogramming hardware for acl id {} "
                      " Err {}",
                      __func__, acl_get_acl_id(args->acl));
        return ret;
    }

    acl_pd_dealloc_res(pd_acl);
    unlink_pi_pd(pd_acl, args->acl);
    acl_pd_free(&pd_acl);

    return ret;
}

// ----------------------------------------------------------------------------
// Create a PD ACL and add it to hardware
// ----------------------------------------------------------------------------
hal_ret_t
pd_acl_create (pd_acl_args_t *args) 
{
    hal_ret_t ret;
    pd_acl_t  *pd_acl;

    HAL_TRACE_DEBUG("PD-ACL::{}: Creating pd state for acl: {}", 
            __func__, acl_get_acl_id(args->acl));

    // Create acl PD
    pd_acl = acl_pd_alloc_init();
    if (pd_acl == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    link_pi_pd(pd_acl, args->acl);

    // Allocate Resources
    ret = acl_pd_alloc_res(pd_acl);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("PD-ACL::{}: Unable to alloc. resources for acl: {}",
                __func__, acl_get_acl_id(args->acl));
        goto end;
    }

    // Program HW
    ret = acl_pd_program_hw(pd_acl);

end:
    if (ret != HAL_RET_OK) {
        acl_pd_dealloc_res(pd_acl);
        unlink_pi_pd(pd_acl, args->acl);
        acl_pd_free(&pd_acl);
    }
    return ret;
}

}    // namespace pd
}    // namespace hal
