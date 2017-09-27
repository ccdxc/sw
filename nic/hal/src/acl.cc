#include "nic/include/base.h"
#include "nic/include/eth.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/acl.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/proto/hal/acl.pb.h"

namespace hal {

void *
acl_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((acl_t *)entry)->acl_id);
}

uint32_t
acl_compute_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(acl_id_t)) % ht_size;
}

bool
acl_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(acl_id_t *)key1 == *(acl_id_t *)key2) {
        return true;
    }
    return false;
}

void *
acl_get_handle_key_func(void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((acl_t *)entry)->hal_handle);
}

uint32_t
acl_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
acl_compare_handle_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(hal_handle_t *)key1 == *(hal_handle_t *)key2) {
        return true;
    }
    return false;
}

// allocate a Acl instance
static inline acl_t *
acl_alloc (void)
{
    acl_t    *acl;

    acl = (acl_t *)g_hal_state->acl_slab()->alloc();
    if (acl == NULL) {
        return NULL;
    }
    return acl;
}

// initialize a Acl instance
static inline acl_t *
acl_init (acl_t *acl)
{
    if (!acl) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&acl->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state

    // initialize meta information
    acl->ht_ctxt.reset();
    acl->hal_handle_ht_ctxt.reset();

    return acl;
}

// allocate and initialize a acl instance
static inline acl_t *
acl_alloc_init (void)
{
    return acl_init(acl_alloc());
}

static inline hal_ret_t
acl_free (acl_t **acl_)
{
    acl_t *acl = *acl_;
    if (!acl) {
        return HAL_RET_OK;
    }
    HAL_SPINLOCK_DESTROY(&acl->slock);
    g_hal_state->acl_slab()->free(acl);
    acl_ = NULL;
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// insert this acl in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_acl_to_db (acl_t *acl)
{
    g_hal_state->acl_id_ht()->insert(acl, &acl->ht_ctxt);
    g_hal_state->acl_hal_handle_ht()->insert(acl, &acl->hal_handle_ht_ctxt);
    return HAL_RET_OK;
}

static inline hal_ret_t
remove_acl_from_db (acl_t *acl)
{
    g_hal_state->acl_id_ht()->remove(&acl->acl_id);
    g_hal_state->acl_hal_handle_ht()->remove(&acl->hal_handle);
    return HAL_RET_OK;
}

static hal_ret_t
validate_acl_create (AclSpec& spec,
                     AclResponse *rsp)
{
    acl_t        *acl = NULL;
    acl_id_t     acl_id;

    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("PI-ACL:{}: ACL id not provided",
                      __func__);
        rsp->set_api_status(types::API_STATUS_ACL_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    if (spec.key_or_handle().key_or_handle_case() !=
        acl::AclKeyHandle::kAclId) {
        HAL_TRACE_ERR("PI-ACL:{}: ACL id not provided",
                      __func__);
        rsp->set_api_status(types::API_STATUS_ACL_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    acl_id = spec.key_or_handle().acl_id();
    acl = find_acl_by_id(acl_id);
    if (acl) {
        HAL_TRACE_ERR("PI-ACL:{} ACL with id {} already created", 
                      __func__, acl_id);
        rsp->set_api_status(types::API_STATUS_EXISTS_ALREADY);
        return HAL_RET_ENTRY_EXISTS;
    }
    return HAL_RET_OK;
}

static hal_ret_t
validate_l4port_range(uint16_t port_start, uint16_t port_end, 
                      uint16_t *key_p, uint16_t *mask_p)
{
    uint16_t key, mask;

    if (port_end < port_start) {
        return HAL_RET_INVALID_ARG;
    }

    /* The range of ports should be maskable */
    mask = ~(port_start^port_end);
    key = port_start & mask;

    if ((key < port_start) ||
        (((key | ~mask) & 0xffff) > port_end)) {
        return HAL_RET_INVALID_ARG;
    }

    *key_p = key;
    *mask_p = mask;

    return HAL_RET_OK;
}

static hal_ret_t
extract_ip_common (acl_type_e acl_type,
                   acl_ip_match_spec_t *ip_key,
                   acl_ip_match_spec_t *ip_mask,
                   const acl::IPSelector& ip_sel,
                   AclResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;
    uint8_t     tcp_flags_mask = 0;
    uint8_t     tcp_flags = 0;
    uint16_t    port_start, port_end;

    switch(ip_sel.l4_selectors_case()) {
        case acl::IPSelector::kIpProtocol:
            ip_key->ip_proto = ip_sel.ip_protocol();
            ip_mask->ip_proto = 0xff;
            break;
        case acl::IPSelector::kIcmpSelector:
            if (acl_type == ACL_TYPE_IPv6) {
                ip_key->ip_proto = IP_PROTO_ICMPV6;
                ip_mask->ip_proto = 0xff;
            } else {
                ip_key->ip_proto = IP_PROTO_ICMP;
                ip_mask->ip_proto = 0xff;
            }

            ip_key->u.icmp.icmp_code = ip_sel.icmp_selector().icmp_code();
            ip_mask->u.icmp.icmp_code = ip_sel.icmp_selector().icmp_code_mask();

            ip_key->u.icmp.icmp_type = ip_sel.icmp_selector().icmp_type();
            ip_mask->u.icmp.icmp_type = ip_sel.icmp_selector().icmp_type_mask();
            break;
        case acl::IPSelector::kUdpSelector:
            ip_key->ip_proto = IP_PROTO_UDP;
            ip_mask->ip_proto = 0xff;

            if (ip_sel.udp_selector().has_src_port_range()) {
                port_start = ip_sel.udp_selector().src_port_range().port_low();
                port_end = ip_sel.udp_selector().src_port_range().port_high();

                ret = validate_l4port_range(port_start, port_end, 
                                            &ip_key->u.udp.sport,
                                            &ip_mask->u.udp.sport);
                if (ret != HAL_RET_OK) {
                    HAL_TRACE_ERR("PI-ACL:{}: UDP sport range {} to {} cannot be supported"
                                  " Only maskable ranges are supported",
                                  __func__, port_start, port_end);
                    rsp->set_api_status(types::API_STATUS_INVALID_ARG);
                    ret = HAL_RET_INVALID_ARG;
                    goto end;
                }
            }
            if (ip_sel.udp_selector().has_dst_port_range()) {
                port_start = ip_sel.udp_selector().dst_port_range().port_low();
                port_end = ip_sel.udp_selector().dst_port_range().port_high();

                ret = validate_l4port_range(port_start, port_end, 
                                            &ip_key->u.udp.dport,
                                            &ip_mask->u.udp.dport);
                if (ret != HAL_RET_OK) {
                    HAL_TRACE_ERR("PI-ACL:{}: UDP dport range {} to {} cannot be supported"
                                  " Only maskable ranges are supported",
                                  __func__, port_start, port_end);
                    rsp->set_api_status(types::API_STATUS_INVALID_ARG);
                    ret = HAL_RET_INVALID_ARG;
                    goto end;
                }
            }
            break;
        case acl::IPSelector::kTcpSelector:
            ip_key->ip_proto = IP_PROTO_TCP;
            ip_mask->ip_proto = 0xff;

            if (ip_sel.tcp_selector().has_src_port_range()) {

                port_start = ip_sel.tcp_selector().src_port_range().port_low();
                port_end = ip_sel.tcp_selector().src_port_range().port_high();

                ret = validate_l4port_range(port_start, port_end, 
                                            &ip_key->u.tcp.sport,
                                            &ip_mask->u.tcp.sport);
                if (ret != HAL_RET_OK) {
                    HAL_TRACE_ERR("PI-ACL:{}: TCP sport range {} to {} cannot be supported"
                                  " Only maskable ranges are supported",
                                  __func__, port_start, port_end);
                    rsp->set_api_status(types::API_STATUS_INVALID_ARG);
                    ret = HAL_RET_INVALID_ARG;
                    goto end;
                }
                
            }
            if (ip_sel.tcp_selector().has_dst_port_range()) {
                port_start = ip_sel.tcp_selector().dst_port_range().port_low();
                port_end = ip_sel.tcp_selector().dst_port_range().port_high();

                ret = validate_l4port_range(port_start, port_end, 
                                            &ip_key->u.tcp.dport,
                                            &ip_mask->u.tcp.dport);
                if (ret != HAL_RET_OK) {
                    HAL_TRACE_ERR("PI-ACL:{}: TCP dport range {} to {} cannot be supported"
                                  " Only maskable ranges are supported",
                                  __func__, port_start, port_end);
                    rsp->set_api_status(types::API_STATUS_INVALID_ARG);
                    ret = HAL_RET_INVALID_ARG;
                    goto end;
                }
            }

            switch(ip_sel.tcp_selector().tcp_syn_case()) {
                case acl::TCPSelector::kTcpSynSet:
                    tcp_flags |= TCP_FLAG_SYN;
                    tcp_flags_mask |= TCP_FLAG_SYN;
                    break;
                case acl::TCPSelector::kTcpSynClear:
                    tcp_flags_mask |= TCP_FLAG_SYN;
                    break;
                case acl::TCPSelector::TCP_SYN_NOT_SET:
                    break;
            }

            switch(ip_sel.tcp_selector().tcp_ack_case()) {
                case acl::TCPSelector::kTcpAckSet:
                    tcp_flags |= TCP_FLAG_ACK;
                    tcp_flags_mask |= TCP_FLAG_ACK;
                    break;
                case acl::TCPSelector::kTcpAckClear:
                    tcp_flags_mask |= TCP_FLAG_ACK;
                    break;
                case acl::TCPSelector::TCP_ACK_NOT_SET:
                    break;
            }

            switch(ip_sel.tcp_selector().tcp_fin_case()) {
                case acl::TCPSelector::kTcpFinSet:
                    tcp_flags |= TCP_FLAG_FIN;
                    tcp_flags_mask |= TCP_FLAG_FIN;
                    break;
                case acl::TCPSelector::kTcpFinClear:
                    tcp_flags_mask |= TCP_FLAG_FIN;
                    break;
                case acl::TCPSelector::TCP_FIN_NOT_SET:
                    break;
            }

            switch(ip_sel.tcp_selector().tcp_rst_case()) {
                case acl::TCPSelector::kTcpRstSet:
                    tcp_flags |= TCP_FLAG_RST;
                    tcp_flags_mask |= TCP_FLAG_RST;
                    break;
                case acl::TCPSelector::kTcpRstClear:
                    tcp_flags_mask |= TCP_FLAG_RST;
                    break;
                case acl::TCPSelector::TCP_RST_NOT_SET:
                    break;
            }

            switch(ip_sel.tcp_selector().tcp_urg_case()) {
                case acl::TCPSelector::kTcpUrgSet:
                    tcp_flags |= TCP_FLAG_URG;
                    tcp_flags_mask |= TCP_FLAG_URG;
                    break;
                case acl::TCPSelector::kTcpUrgClear:
                    tcp_flags_mask |= TCP_FLAG_URG;
                    break;
                case acl::TCPSelector::TCP_URG_NOT_SET:
                    break;
            }

            ip_key->u.tcp.tcp_flags = tcp_flags;
            ip_mask->u.tcp.tcp_flags = tcp_flags_mask;

            break;
        default:
            break;
    }
end:
    return ret;
}

static acl_type_e
get_acl_type (const acl::AclSelector &sel)
{
    acl_type_e acl_type           = ACL_TYPE_INVALID;
    types::IPAddressFamily src_af = types::IP_AF_NONE;
    types::IPAddressFamily dst_af = types::IP_AF_NONE;
    types::IPAddressFamily ip_af  = types::IP_AF_NONE;

    switch(sel.pkt_selector_case()) {
        case acl::AclSelector::kEthSelector:
            acl_type = ACL_TYPE_ETH;
            break;
        case acl::AclSelector::kIpSelector:

            ip_af = sel.ip_selector().ip_af();

            if (sel.ip_selector().has_src_prefix()) {
                src_af = sel.ip_selector().src_prefix().address().ip_af();
            }
            if (sel.ip_selector().has_dst_prefix()) {
                dst_af = sel.ip_selector().dst_prefix().address().ip_af();
            }

            if (ip_af == types::IP_AF_NONE) {
                if ((src_af != types::IP_AF_NONE) ||
                    (dst_af != types::IP_AF_NONE)) {
                    acl_type = ACL_TYPE_INVALID;
                } else {
                    acl_type = ACL_TYPE_IP;
                }
            } else if (ip_af == types::IP_AF_INET) {
                if ((src_af == types::IP_AF_INET6) ||
                    (dst_af == types::IP_AF_INET6)) {
                    acl_type = ACL_TYPE_INVALID;
                } else {
                    acl_type = ACL_TYPE_IPv4;
                }
            } else if (ip_af == types::IP_AF_INET6) {
                if ((src_af == types::IP_AF_INET) ||
                    (dst_af == types::IP_AF_INET)) {
                    acl_type = ACL_TYPE_INVALID;
                } else {
                    acl_type = ACL_TYPE_IPv6;
                }
            }
            break;
        default:
            acl_type = ACL_TYPE_NONE;
            break;
    }
    return acl_type;
}

static hal_ret_t
extract_match_spec (acl_match_spec_t *ms,
                    const acl::AclSelector &sel,
                    AclResponse *rsp)
{
    hal_ret_t            ret = HAL_RET_OK;
    tenant_t             *tenant = NULL;
    tenant_id_t          tenant_id;
    hal_handle_t         tenant_handle = 0;
    if_t                 *src_if = NULL;
    if_id_t              src_if_id;
    hal_handle_t         src_if_handle = 0;
    if_t                 *dest_if = NULL;
    if_id_t              dest_if_id;
    hal_handle_t         dest_if_handle = 0;
    l2seg_t              *l2seg = NULL;
    l2seg_id_t           l2seg_id;
    hal_handle_t         l2seg_handle = 0;
    acl_eth_match_spec_t *eth_key;
    acl_eth_match_spec_t *eth_mask;
    acl_ip_match_spec_t  *ip_key;
    acl_ip_match_spec_t  *ip_mask;

    eth_key = &ms->key.eth;
    eth_mask = &ms->mask.eth;
    ip_key = &ms->key.ip;
    ip_mask = &ms->mask.ip;

    if (sel.has_src_if_key_handle()) {
        ms->src_if_match = true;

        auto src_if_kh = sel.src_if_key_handle();
        if (src_if_kh.key_or_handle_case() == intf::InterfaceKeyHandle::kInterfaceId) {
            src_if_id = src_if_kh.interface_id();
            src_if = find_if_by_id(src_if_id);
        } else {
            src_if_handle = src_if_kh.if_handle();
            src_if = find_if_by_handle(src_if_handle);
        }

        if(src_if == NULL) {
            HAL_TRACE_ERR("PI-ACL:{}: Source interface not found",
                          __func__);
            rsp->set_api_status(types::API_STATUS_INTERFACE_NOT_FOUND);
            ret = HAL_RET_IF_NOT_FOUND;
            goto end;
        } else {
            ms->src_if_handle = src_if->hal_handle;
        }
    }

    if (sel.has_dst_if_key_handle()) {
        ms->dest_if_match = true;

        auto dest_if_kh = sel.dst_if_key_handle();
        if (dest_if_kh.key_or_handle_case() == intf::InterfaceKeyHandle::kInterfaceId) {
            dest_if_id = dest_if_kh.interface_id();
            dest_if = find_if_by_id(dest_if_id);
        } else {
            dest_if_handle = dest_if_kh.if_handle();
            dest_if = find_if_by_handle(dest_if_handle);
        }

        if(dest_if == NULL) {
            HAL_TRACE_ERR("PI-ACL:{}: Destination interface not found",
                          __func__);
            rsp->set_api_status(types::API_STATUS_INTERFACE_NOT_FOUND);
            ret = HAL_RET_IF_NOT_FOUND;
            goto end;
        } else {
            ms->dest_if_handle = dest_if->hal_handle;
        }
    }

    if (sel.seg_selector_case() == acl::AclSelector::kTenantKeyHandle) {
        ms->tenant_match = true;

        auto tenant_kh = sel.tenant_key_handle();
        if (tenant_kh.key_or_handle_case() == tenant::TenantKeyHandle::kTenantId) {
            tenant_id = tenant_kh.tenant_id();
            tenant = tenant_lookup_by_id(tenant_id);
        } else {
            tenant_handle = tenant_kh.tenant_handle();
            tenant = tenant_lookup_by_handle(tenant_handle);
        }

        if(tenant == NULL) {
            HAL_TRACE_ERR("PI-ACL:{}: Tenant not found",
                          __func__);
            rsp->set_api_status(types::API_STATUS_TENANT_NOT_FOUND);
            ret = HAL_RET_TENANT_NOT_FOUND;
            goto end;
        } else {
            ms->tenant_handle = tenant->hal_handle;
        }
    } else if (sel.seg_selector_case() == acl::AclSelector::kL2SegmentKeyHandle) {
        ms->l2seg_match = true;

        auto l2seg_kh = sel.l2segment_key_handle();
        if (l2seg_kh.key_or_handle_case() == l2segment::L2SegmentKeyHandle::kSegmentId) {
            l2seg_id = l2seg_kh.segment_id();
            l2seg = find_l2seg_by_id(l2seg_id);
        } else {
            l2seg_handle = l2seg_kh.l2segment_handle();
            l2seg = find_l2seg_by_handle(l2seg_handle);
        }

        if(l2seg == NULL) {
            HAL_TRACE_ERR("PI-ACL:{}: L2 segment not found",
                          __func__);
            rsp->set_api_status(types::API_STATUS_L2_SEGMENT_NOT_FOUND);
            ret = HAL_RET_L2SEG_NOT_FOUND;
            goto end;
        } else {
            ms->l2seg_handle = l2seg->hal_handle;
        }
    }

    // Figure out the acl-type
    ms->acl_type = get_acl_type(sel);

    switch(ms->acl_type) {
        case ACL_TYPE_NONE:
            // Nothing to do
            break;
        case ACL_TYPE_ETH:
            eth_key->ether_type = sel.eth_selector().eth_type();
            eth_mask->ether_type = sel.eth_selector().eth_type_mask(); 

            MAC_UINT64_TO_ADDR(eth_key->mac_sa, sel.eth_selector().src_mac());
            MAC_UINT64_TO_ADDR(eth_mask->mac_sa, sel.eth_selector().src_mac_mask());

            MAC_UINT64_TO_ADDR(eth_key->mac_da, sel.eth_selector().dst_mac());
            MAC_UINT64_TO_ADDR(eth_mask->mac_da, sel.eth_selector().dst_mac_mask());
            break;
        case ACL_TYPE_IP:
            ret = extract_ip_common(ms->acl_type, ip_key, ip_mask, 
                                    sel.ip_selector(), rsp);
            break;
        case ACL_TYPE_IPv4:
            if (sel.ip_selector().has_src_prefix()) {
                ip_key->sip.addr.v4_addr = sel.ip_selector().src_prefix().address().v4_addr();
                // Convert prefix len to mask
                ip_mask->sip.addr.v4_addr = ipv4_prefix_len_to_mask(
                    sel.ip_selector().src_prefix().prefix_len());
            }

            if (sel.ip_selector().has_dst_prefix()) {
                ip_key->dip.addr.v4_addr = sel.ip_selector().dst_prefix().address().v4_addr();
                // Convert prefix len to mask
                ip_mask->dip.addr.v4_addr = ipv4_prefix_len_to_mask(
                    sel.ip_selector().dst_prefix().prefix_len());
            }

            ret = extract_ip_common(ms->acl_type, ip_key, ip_mask, 
                                    sel.ip_selector(), rsp);
            break;
        case ACL_TYPE_IPv6:
            if (sel.ip_selector().has_src_prefix()) {
                memcpy(ip_key->sip.addr.v6_addr.addr8,
                       sel.ip_selector().src_prefix().address().v6_addr().c_str(),
                       IP6_ADDR8_LEN);
                // Convert prefix len to mask
                ipv6_prefix_len_to_mask(&ip_mask->sip.addr.v6_addr,
                                        sel.ip_selector().src_prefix().prefix_len());
            }

            if (sel.ip_selector().has_dst_prefix()) {
                memcpy(ip_key->dip.addr.v6_addr.addr8,
                       sel.ip_selector().dst_prefix().address().v6_addr().c_str(),
                       IP6_ADDR8_LEN);
                // Convert prefix len to mask
                ipv6_prefix_len_to_mask(&ip_mask->dip.addr.v6_addr,
                                        sel.ip_selector().dst_prefix().prefix_len());
            }

            ret = extract_ip_common(ms->acl_type, ip_key, ip_mask, 
                                    sel.ip_selector(), rsp);
            break;
        case ACL_TYPE_INVALID:
            HAL_TRACE_ERR("PI-ACL:{}: ACL Type invalid",
                          __func__);
            rsp->set_api_status(types::API_STATUS_INVALID_ARG);
            ret = HAL_RET_INVALID_ARG;
            break;
    }

    if (ret != HAL_RET_OK) {
        goto end;
    }

#ifdef ACL_DOL_TEST_ONLY
    // Key of internal fields for use only with DOL/testing infra
    // For production builds this needs to be removed
    // TODO: REMOVE
    if (sel.has_internal_key() != sel.has_internal_mask()) {
        HAL_TRACE_ERR("PI-ACL:{}: ACL Internal selector key/mask not specified",
                      __func__);
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    if (sel.has_internal_key()) {
        ms->int_key.direction = sel.internal_key().direction();
        ms->int_mask.direction = sel.internal_mask().direction();
        ms->int_key.flow_miss = sel.internal_key().flow_miss();
        ms->int_mask.flow_miss = sel.internal_mask().flow_miss();
        ms->int_key.ip_options = sel.internal_key().ip_options();
        ms->int_mask.ip_options = sel.internal_mask().ip_options();
        ms->int_key.ip_frag = sel.internal_key().ip_frag();
        ms->int_mask.ip_frag = sel.internal_mask().ip_frag();
        ms->int_key.drop_reason = sel.internal_key().drop_reason();
        ms->int_mask.drop_reason = sel.internal_mask().drop_reason();
        MAC_UINT64_TO_ADDR(ms->int_key.outer_mac_da, sel.internal_key().outer_dst_mac());
        MAC_UINT64_TO_ADDR(ms->int_mask.outer_mac_da, sel.internal_mask().outer_dst_mac());
    }
#endif
end:
    return ret;
}

static hal_ret_t
extract_action_spec (acl_action_spec_t *as,
                    const acl::AclActionInfo &ainfo,
                    AclResponse *rsp)
{
    hal_ret_t    ret = HAL_RET_OK;
    if_t         *redirect_if = NULL;
    if_id_t      redirect_if_id;
    hal_handle_t redirect_if_handle = 0;
#ifdef ACL_DOL_TEST_ONLY
    // Internal fields for use only with DOL/testing infra
    // For production builds this needs to be removed
    // TODO: REMOVE
    hal::pd::pd_rw_entry_args_t rw_key{};
#endif

    as->action = ainfo.action();
    as->ing_mirror_en = ainfo.ing_mirror_en();
    as->ing_mirror_session_handle = ainfo.ing_mirror_session_handle();
    as->egr_mirror_en = ainfo.egr_mirror_en();;
    as->egr_mirror_session_handle = ainfo.egr_mirror_session_handle();
    as->copp_policer_handle = ainfo.copp_policer_handle();

    if (ainfo.has_redirect_if_key_handle()) {
        if (as->action != acl::ACL_ACTION_REDIRECT) {
            HAL_TRACE_ERR("PI-ACL:{}: Redirect interface specified with action {} ",
                          __func__, as->action);
            rsp->set_api_status(types::API_STATUS_INVALID_ARG);
            ret = HAL_RET_INVALID_ARG;
            goto end;
        }

        auto redirect_if_kh = ainfo.redirect_if_key_handle();
        if (redirect_if_kh.key_or_handle_case() == 
            intf::InterfaceKeyHandle::kInterfaceId) {
            redirect_if_id = redirect_if_kh.interface_id();
            redirect_if = find_if_by_id(redirect_if_id);
        } else {
            redirect_if_handle = redirect_if_kh.if_handle();
            redirect_if = find_if_by_handle(redirect_if_handle);
        }

        if(redirect_if == NULL) {
            HAL_TRACE_ERR("PI-ACL:{}: Redirect interface not found",
                          __func__);
            rsp->set_api_status(types::API_STATUS_INTERFACE_NOT_FOUND);
            ret = HAL_RET_IF_NOT_FOUND;
            goto end;
        } else {
            as->redirect_if_handle = redirect_if->hal_handle;
        }
    }

#ifdef ACL_DOL_TEST_ONLY
    // Internal fields for use only with DOL/testing infra
    // For production builds this needs to be removed
    // TODO: REMOVE
    if (ainfo.has_internal_actions()) {
        if (as->action != acl::ACL_ACTION_REDIRECT) {
            HAL_TRACE_ERR("PI-ACL:{}: Redirect action fields specified for "
                          "non-redirect action {}",
                          __func__, as->action);
            rsp->set_api_status(types::API_STATUS_INVALID_ARG);
            ret = HAL_RET_INVALID_ARG;
            goto end;
        }
        as->int_as.mac_sa_rewrite = ainfo.internal_actions().mac_sa_rewrite_en();
        as->int_as.mac_da_rewrite = ainfo.internal_actions().mac_da_rewrite_en();
        as->int_as.ttl_dec = ainfo.internal_actions().ttl_dec_en();

        if (ainfo.internal_actions().has_encap_info()) {
            as->int_as.tnnl_vnid = ainfo.internal_actions().encap_info().encap_value();
        }

        if (as->int_as.mac_sa_rewrite) {
            MAC_UINT64_TO_ADDR(rw_key.mac_sa, ainfo.internal_actions().mac_sa());
        }

        if (as->int_as.mac_da_rewrite) {
            MAC_UINT64_TO_ADDR(rw_key.mac_da, ainfo.internal_actions().mac_da());
        }

        rw_key.rw_act = REWRITE_REWRITE_ID;
        ret = hal::pd::pd_rw_entry_find_or_alloc(&rw_key, &as->int_as.rw_idx);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("PI-ACL:{}: Unable to find/alloc rw entry",
                          __func__);
            rsp->set_api_status(types::API_STATUS_OUT_OF_RESOURCE);
            goto end;
        }
    }
#endif
end:
    return ret;
}

hal_ret_t
acl_create (AclSpec& spec,
            AclResponse *rsp)
{
    hal_ret_t         ret = HAL_RET_OK;
    acl_t             *acl = NULL;
    pd::pd_acl_args_t pd_acl_args;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-ACL:{}: Acl create ", __func__);

    ret = validate_acl_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-ACL:{}: Acl create request validation failed."
                      " Err: {}",
                      __func__, HAL_RET_INVALID_ARG);
        goto end;
    }

    HAL_TRACE_DEBUG("PI-ACL:{}: Acl create for id {}",
                    __func__, spec.key_or_handle().acl_id());

    // allocate a acl instance
    acl = acl_alloc_init();
    if (acl == NULL) {
        HAL_TRACE_ERR("PI-ACL:{}: Out of Memory. Err: {}",
                      __func__, HAL_RET_OOM);
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        ret = HAL_RET_OOM;
        goto end;
    }

    // save the configs from the spec
    acl->acl_id = spec.key_or_handle().acl_id();
    acl->hal_handle = hal_alloc_handle();

    acl->priority = spec.priority();
    ret = extract_match_spec(&acl->match_spec, spec.match(), rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-ACL:{}: Acl create request match parsing failed."
                      " Err: {}",
                      __func__, ret);
        goto end;
    }

    ret = extract_action_spec(&acl->action_spec, spec.action(), rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-ACL:{}: Acl create request action parsing failed."
                      " Err: {}",
                      __func__, ret);
        goto end;
    }

    // allocate PD resources and program hardware
    pd::pd_acl_args_init(&pd_acl_args);
    pd_acl_args.acl = acl;
    ret = pd::pd_acl_create(&pd_acl_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD acl create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto end;
    }

    // add this acl to the db
    add_acl_to_db(acl);
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_status()->mutable_acl_handle()->set_handle(acl->hal_handle);

end:

    if (ret != HAL_RET_OK && acl != NULL) {
        acl_free(&acl);
    }
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}

hal_ret_t
acl_update (AclSpec& spec,
            AclResponse *rsp)
{
    return HAL_RET_OK;
}

static acl_t*
validate_acl_delete (acl::AclDeleteRequest& req,
                     acl::AclDeleteResponse* rsp)
{
    acl_t        *acl = NULL;
    acl_id_t     acl_id;
    hal_handle_t acl_handle;

    if (!req.has_key_or_handle()) {
        HAL_TRACE_ERR("PI-ACL:{}: Delete request is empty", __func__);
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return NULL;
    }

    auto acl_kh = req.key_or_handle();
    if (acl_kh.key_or_handle_case() == acl::AclKeyHandle::kAclId) {
        acl_id = acl_kh.acl_id();
        acl = find_acl_by_id(acl_id);
    } else {
        acl_handle = acl_kh.acl_handle().handle();
        acl = find_acl_by_handle(acl_handle);
    }

    if (!acl) {
        HAL_TRACE_ERR("PI-ACL:{}: ACL not found", __func__);
        rsp->set_api_status(types::API_STATUS_ACL_NOT_FOUND);
        return NULL;
    }

    return acl;
}

hal_ret_t 
acl_delete (acl::AclDeleteRequest& req,
            acl::AclDeleteResponse* rsp)
{
    hal_ret_t         ret = HAL_RET_OK;
    acl_t             *acl;
    pd::pd_acl_args_t pd_acl_args;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-ACL:{}: Acl delete ", __func__);

    acl = validate_acl_delete(req, rsp);
    if (!acl) {
        HAL_TRACE_ERR("PI-ACL:{}: Acl delete request validation failed. Err {}",
                      __func__, HAL_RET_INVALID_ARG);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    HAL_TRACE_DEBUG("PI-ACL:{}: Acl delete for id {} handle {}",
                    __func__, acl->acl_id, acl->hal_handle);

    // delete PD resources and deprogram hardware
    pd::pd_acl_args_init(&pd_acl_args);
    pd_acl_args.acl = acl;
    ret = pd::pd_acl_delete(&pd_acl_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD acl delete failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto end;
    }

    // Cleanup the PI state and free the ACL
    remove_acl_from_db(acl);
    acl_free(&acl);

    rsp->set_api_status(types::API_STATUS_OK);
end:
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}
}    // namespace hal
