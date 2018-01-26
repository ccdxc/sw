// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#include "nic/include/base.h"
#include "nic/include/eth.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/acl.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/gen/proto/hal/acl.pb.h"

namespace hal {

// Acl
// ----------------------------------------------------------------------------
// hash table acl_type => ht_entry
//  - Get key from entry
// ----------------------------------------------------------------------------
void *
acl_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t *ht_entry;
    acl_t                    *acl = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    acl = find_acl_by_handle(ht_entry->handle_id);
    return (void *)&(acl->key);
}

// ----------------------------------------------------------------------------
// hash table acl_type => entry - compute hash
// ----------------------------------------------------------------------------
uint32_t
acl_compute_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(acl_key_t)) % ht_size;
}

// ----------------------------------------------------------------------------
// hash table acl_type => entry - compare function
// ----------------------------------------------------------------------------
bool
acl_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));

    if (!memcmp(key1, key2, sizeof(acl_key_t))) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// insert a acl to db
//------------------------------------------------------------------------------
static inline hal_ret_t
acl_add_to_db (acl_t *acl, hal_handle_t handle)
{
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("pi-acl:{}:adding to acl hash table",
                    __func__);
    // allocate an entry to establish mapping from acl-id to its handle
    entry =
        (hal_handle_id_ht_entry_t *)g_hal_state->
        hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    entry->handle_id = handle;
    sdk_ret = g_hal_state->acl_ht()->insert_with_key(&acl->key,
                                                     entry, &entry->ht_ctxt);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("pi-acl:{}:failed to add key to handle mapping, "
                      "err : {}", __func__, ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    // TODO: Check if this is the right place
    acl->hal_handle = handle;

    return ret;
}

//------------------------------------------------------------------------------
// delete a acl from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
acl_del_from_db (acl_t *acl)
{
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("pi-acl:{}:removing from hash table", __func__);

    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)g_hal_state->acl_ht()->remove(&acl->key);

    if (entry) {
        // free up
         hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }

    return HAL_RET_OK;
}

static hal_ret_t
acl_free (acl_t *acl, bool free_pd)
{
    hal_ret_t         ret = HAL_RET_OK;
    pd::pd_acl_args_t pd_acl_args = { 0 };
    if (!acl) {
        return HAL_RET_OK;
    }
    if (free_pd) {
        pd::pd_acl_args_init(&pd_acl_args);
        pd_acl_args.acl = acl;
        ret = pd::pd_acl_mem_free(&pd_acl_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pi-acl:{}:failed to delete acl pd, err : {}",
                          __func__, ret);
            return ret;
        }
    }
    HAL_SPINLOCK_DESTROY(&acl->slock);
    hal::delay_delete_to_slab(HAL_SLAB_ACL, acl);
    return ret;
}


//-----------------------------------------------------------------------------
// Print acl spec
//-----------------------------------------------------------------------------
hal_ret_t
acl_spec_print (AclSpec& spec)
{
    hal_ret_t           ret = HAL_RET_OK;
    fmt::MemoryWriter   buf;

    buf.write("Acl Spec: ");
    if (spec.has_key_or_handle()) {
        auto kh = spec.key_or_handle();
        if (kh.key_or_handle_case() == AclKeyHandle::kAclId) {
            buf.write("acl_id:{}, ", kh.acl_id());
        } else if (kh.key_or_handle_case() == AclKeyHandle::kAclHandle) {
            buf.write("acl_hdl:{}, ", kh.acl_handle());
        }
    } else {
        buf.write("acl_id_hdl:NULL, ");
    }

    buf.write("priority:{}, ",spec.priority());
    if (spec.has_match()) {
        const acl::AclSelector &sel = spec.match();

        if (sel.has_src_if_key_handle()) {
            auto src_if_kh = sel.src_if_key_handle();
            if (src_if_kh.key_or_handle_case() == kh::InterfaceKeyHandle::kInterfaceId) {
                buf.write("src_if_id:{}, ", src_if_kh.interface_id());
            } else {
                buf.write("src_if_handle:{}, ", src_if_kh.if_handle());
            }
        }

        if (sel.has_dst_if_key_handle()) {
            auto dst_if_kh = sel.dst_if_key_handle();
            if (dst_if_kh.key_or_handle_case() == kh::InterfaceKeyHandle::kInterfaceId) {
                buf.write("dst_if_id:{}, ", dst_if_kh.interface_id());
            } else {
                buf.write("dst_if_handle:{}, ", dst_if_kh.if_handle());
            }
        }

        if (sel.has_vrf_key_handle()) {
            auto vrf_kh = sel.vrf_key_handle();
            if (vrf_kh.key_or_handle_case() == kh::VrfKeyHandle::kVrfId) {
                buf.write("vrf_id:{}, ", vrf_kh.vrf_id());
            } else {
                buf.write("vrf_handle:{}, ",vrf_kh.vrf_handle());
            }
        }

        if (sel.has_l2segment_key_handle()) {
            auto l2seg_kh = sel.l2segment_key_handle();
            if (l2seg_kh.key_or_handle_case() == kh::L2SegmentKeyHandle::kSegmentId) {
                buf.write("l2seg_id:{}, ", l2seg_kh.segment_id());
            } else {
                buf.write("l2seg_handle:{}, ", l2seg_kh.l2segment_handle());
            }
        }

        if (sel.has_eth_selector()) {
            buf.write("ethtype:{0:x}/{1:x}, ",
                      sel.eth_selector().eth_type_mask(),
                      sel.eth_selector().eth_type());

            mac_addr_t mac_sa, mac_sa_mask;
            MAC_UINT64_TO_ADDR(mac_sa, sel.eth_selector().src_mac());
            MAC_UINT64_TO_ADDR(mac_sa_mask, sel.eth_selector().src_mac_mask());
            buf.write("mac_sa:{}/{}, ", macaddr2str(mac_sa_mask), macaddr2str(mac_sa));

            mac_addr_t mac_da, mac_da_mask;
            MAC_UINT64_TO_ADDR(mac_da, sel.eth_selector().dst_mac());
            MAC_UINT64_TO_ADDR(mac_da_mask, sel.eth_selector().dst_mac_mask());
            buf.write("mac_da:{}/{}, ", macaddr2str(mac_da_mask), macaddr2str(mac_da));
        }

        // TODO Add other fields
        if (sel.has_ip_selector()) {
            if (sel.ip_selector().has_src_prefix()) {
                ip_prefix_t src_pfx;
                ip_pfx_spec_to_pfx_spec(&src_pfx, sel.ip_selector().src_prefix());
                buf.write("src {}, ", ippfx2str(&src_pfx));
            }
            if (sel.ip_selector().has_dst_prefix()) {
                ip_prefix_t dst_pfx;
                ip_pfx_spec_to_pfx_spec(&dst_pfx, sel.ip_selector().dst_prefix());
                buf.write("dst {}, ", ippfx2str(&dst_pfx));
            }

            const acl::IPSelector& ip_sel = sel.ip_selector();
            switch(ip_sel.l4_selectors_case()) {
                case acl::IPSelector::kIpProtocol:
                    buf.write("proto {}, ", ip_sel.ip_protocol());
                    break;
                case acl::IPSelector::kIcmpSelector:
                    buf.write("icmp code {}/{}, ", 
                              ip_sel.icmp_selector().icmp_code_mask(),
                              ip_sel.icmp_selector().icmp_code());

                    buf.write("icmp type {}/{}, ",
                              ip_sel.icmp_selector().icmp_type_mask(),
                              ip_sel.icmp_selector().icmp_type());
                    break;
                case acl::IPSelector::kUdpSelector:
                    buf.write("udp ");
                    if (ip_sel.udp_selector().has_src_port_range()) {
                        buf.write("sport {}-{}, ",
                                  ip_sel.udp_selector().src_port_range().port_low(),
                                  ip_sel.udp_selector().src_port_range().port_high());
                    }
                    if (ip_sel.udp_selector().has_dst_port_range()) {
                        buf.write("dport {}-{}, ",
                                  ip_sel.udp_selector().dst_port_range().port_low(),
                                  ip_sel.udp_selector().dst_port_range().port_high());
                    }
                    break;
                case acl::IPSelector::kTcpSelector:
                    buf.write("tcp ");
                    if (ip_sel.tcp_selector().has_src_port_range()) {
                        buf.write("sport {}-{}, ",
                                  ip_sel.tcp_selector().src_port_range().port_low(),
                                  ip_sel.tcp_selector().src_port_range().port_high());
                    }
                    if (ip_sel.tcp_selector().has_dst_port_range()) {
                        buf.write("dport {}-{}, ",
                                  ip_sel.tcp_selector().dst_port_range().port_low(),
                                  ip_sel.tcp_selector().dst_port_range().port_high());
                    }
                    break;
                default:
                    break;
            }
        }

        if (sel.has_internal_key()) {
            buf.write("flow_miss {}, outer_dst_mac {}, "
                      "ip_options {}, ip_frag {}, "
                      "tunnel_terminate {}, direction {}, ",
                      sel.internal_key().flow_miss(),
                      sel.internal_key().outer_dst_mac(),
                      sel.internal_key().ip_options(),
                      sel.internal_key().ip_frag(),
                      sel.internal_key().tunnel_terminate(),
                      sel.internal_key().direction());
        }

        if (sel.has_internal_mask()) {
            buf.write("flow_miss_mask {}, outer_dst_mac_mask {}, "
                      "ip_options_mask {}, ip_frag_mask {}, "
                      "tunnel_terminate_mask {}, direction_mask {}, ",
                      sel.internal_mask().flow_miss(),
                      sel.internal_mask().outer_dst_mac(),
                      sel.internal_mask().ip_options(),
                      sel.internal_mask().ip_frag(),
                      sel.internal_mask().tunnel_terminate(),
                      sel.internal_mask().direction());
        }
    }


    if (spec.has_action()) {
        buf.write("Action {} ", spec.action().action());
        if (spec.action().has_redirect_if_key_handle()) {
            auto redirect_if_kh = spec.action().redirect_if_key_handle();
            if (redirect_if_kh.key_or_handle_case() == kh::InterfaceKeyHandle::kInterfaceId) {
                buf.write("redirect_if_id:{}, ", redirect_if_kh.interface_id());
            } else {
                buf.write("redirect_if_handle:{}, ", redirect_if_kh.if_handle());
            }
        }
    }

    HAL_TRACE_DEBUG(buf.c_str());
    return ret;
}


//------------------------------------------------------------------------------
// PD Call to allocate PD resources and HW programming
//------------------------------------------------------------------------------
hal_ret_t
acl_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t         ret = HAL_RET_OK;
    pd::pd_acl_args_t pd_acl_args = { 0 };
    dllist_ctxt_t     *lnode = NULL;
    dhl_entry_t       *dhl_entry = NULL;
    acl_t             *acl = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-acl:{}: invalid cfg_ctxt", __func__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    acl = (acl_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("pi-acl:{}:create add CB {}",
                    __func__, acl->key);

    // PD Call to allocate PD resources and HW programming
    pd::pd_acl_args_init(&pd_acl_args);
    pd_acl_args.acl = acl;
    ret = pd::pd_acl_create(&pd_acl_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-acl:{}:failed to create acl pd, err : {}",
                      __func__, ret);
    }

end:
    return ret;
}


//------------------------------------------------------------------------------
// 1. Update PI DBs as acl_create_add_cb() was a success
//      a. Add to acl id hash table
//------------------------------------------------------------------------------
hal_ret_t
acl_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t     ret = HAL_RET_OK;
    dllist_ctxt_t *lnode = NULL;
    dhl_entry_t   *dhl_entry = NULL;
    acl_t        *acl = NULL;
    hal_handle_t  hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-acl:{}:invalid cfg_ctxt", __func__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    acl = (acl_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("pi-acl:{}:create commit CB {}",
                    __func__, acl->key);

    // Add to DB
    ret = acl_add_to_db (acl, hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-acl:{}:unable to add acl:{} to DB",
                      __func__, acl->key);
        goto end;
    }

    HAL_TRACE_ERR("pi-acl:{}:added acl:{} to DB",
                  __func__, acl->key);

    // TODO: Increment the ref counts of dependent objects

end:
    return ret;
}

//------------------------------------------------------------------------------
// acl_create_add_cb was a failure
// 1. call delete to PD
//      a. Deprogram HW
//      b. Clean up resources
//      c. Free PD object
// 2. Remove object from hal_handle id based hash table in infra
// 3. Free PI acl
//------------------------------------------------------------------------------
hal_ret_t
acl_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t          ret = HAL_RET_OK;
    pd::pd_acl_args_t pd_acl_args = { 0 };
    dllist_ctxt_t      *lnode = NULL;
    dhl_entry_t        *dhl_entry = NULL;
    acl_t             *acl = NULL;
    hal_handle_t       hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-acl:{}:invalid cfg_ctxt", __func__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    acl = (acl_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("pi-acl:{}:create abort CB {}", __func__);

    // 1. delete call to PD
    if (acl->pd) {
        pd::pd_acl_args_init(&pd_acl_args);
        pd_acl_args.acl = acl;
        ret = pd::pd_acl_delete(&pd_acl_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pi-acl:{}:failed to delete acl pd, err : {}",
                          __func__, ret);
        }
    }

    // 2. remove object from hal_handle id based hash table in infra
    hal_handle_free(hal_handle);

    // 3. Free PI acl
    acl_free(acl, false);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// Dummy create cleanup callback
// ----------------------------------------------------------------------------
hal_ret_t
acl_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t   ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// Converts hal_ret_t to API status
//------------------------------------------------------------------------------
hal_ret_t
acl_prepare_rsp (AclResponse *rsp, hal_ret_t ret,
                       hal_handle_t hal_handle)
{
    if (ret == HAL_RET_OK) {
        rsp->mutable_status()->set_acl_handle(hal_handle);
    }

    rsp->set_api_status(hal_prepare_rsp(ret));

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// validate an incoming acl create request
//------------------------------------------------------------------------------
static hal_ret_t
validate_acl_create (AclSpec& spec)
{
    acl_t        *acl = NULL;
    acl_id_t     acl_id;

    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("pi-acl:{}: ACL id not provided",
                      __func__);
        return HAL_RET_INVALID_ARG;
    }

    if (spec.key_or_handle().key_or_handle_case() !=
        acl::AclKeyHandle::kAclId) {
        HAL_TRACE_ERR("pi-acl:{}: ACL id not provided",
                      __func__);
        return HAL_RET_INVALID_ARG;
    }

    acl_id = spec.key_or_handle().acl_id();
    acl = find_acl_by_id(acl_id);
    if (acl) {
        HAL_TRACE_ERR("pi-acl:{} ACL with id {} already created",
                      __func__, acl_id);
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
key_mask_to_l4port_range(uint16_t key, uint16_t mask,
                         uint16_t *port_start_p, uint16_t *port_end_p)
{
    hal_ret_t ret = HAL_RET_OK;
    uint16_t port_start, port_end;

    if (!mask) {
        port_start = 0;
        port_end = 0xffff;
    } else if (~mask & (~mask+1)) {
        // Mask should have all bits together in the MSB
        HAL_TRACE_ERR("pi-acl:{}: Not a valid port mask {#x}",
                      __func__, mask);
        ret = HAL_RET_INVALID_ARG;
        port_start = 0;
        port_end = 0;
    } else {
        port_start = key & mask;
        port_end = port_start | ~mask;
    }

    *port_start_p = port_start;
    *port_end_p = port_end;
    return ret;
}

static hal_ret_t
extract_ip_common (acl_type_e acl_type,
                   acl_ip_match_spec_t *ip_key,
                   acl_ip_match_spec_t *ip_mask,
                   const acl::IPSelector& ip_sel)
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
                    HAL_TRACE_ERR("pi-acl:{}: UDP sport range {} to {} cannot be supported"
                                  " Only maskable ranges are supported",
                                  __func__, port_start, port_end);
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
                    HAL_TRACE_ERR("pi-acl:{}: UDP dport range {} to {} cannot be supported"
                                  " Only maskable ranges are supported",
                                  __func__, port_start, port_end);
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
                    HAL_TRACE_ERR("pi-acl:{}: TCP sport range {} to {} cannot be supported"
                                  " Only maskable ranges are supported",
                                  __func__, port_start, port_end);
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
                    HAL_TRACE_ERR("pi-acl:{}: TCP dport range {} to {} cannot be supported"
                                  " Only maskable ranges are supported",
                                  __func__, port_start, port_end);
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

static hal_ret_t 
populate_ip_common (acl_ip_match_spec_t *ip_key,
                    acl_ip_match_spec_t *ip_mask,
                    acl::IPSelector *ip_sel)
{
    hal_ret_t ret = HAL_RET_OK;
    uint16_t port_start, port_end;
    uint8_t  tcp_flags_mask = 0;
    uint8_t  tcp_flags = 0;

    if (!ip_mask->ip_proto) {
        return HAL_RET_OK;
    }
    switch(ip_key->ip_proto) {
        case IP_PROTO_ICMPV6:
        case IP_PROTO_ICMP:
            ip_sel->mutable_icmp_selector()->set_icmp_code(ip_key->u.icmp.icmp_code);
            ip_sel->mutable_icmp_selector()->set_icmp_code_mask(ip_mask->u.icmp.icmp_code);

            ip_sel->mutable_icmp_selector()->set_icmp_type(ip_key->u.icmp.icmp_type);
            ip_sel->mutable_icmp_selector()->set_icmp_type_mask(ip_mask->u.icmp.icmp_type);
            break;
        case IP_PROTO_UDP:
            if (ip_mask->u.udp.sport) {
                ret = key_mask_to_l4port_range(ip_key->u.udp.sport, 
                                               ip_mask->u.udp.sport,
                                               &port_start,
                                               &port_end);
                ip_sel->mutable_udp_selector()->mutable_src_port_range()->set_port_low(port_start);
                ip_sel->mutable_udp_selector()->mutable_src_port_range()->set_port_high(port_end);
            }
            if (ip_mask->u.udp.dport) {
                ret = key_mask_to_l4port_range(ip_key->u.udp.dport, 
                                               ip_mask->u.udp.dport,
                                               &port_start,
                                               &port_end);
                ip_sel->mutable_udp_selector()->mutable_dst_port_range()->set_port_low(port_start);
                ip_sel->mutable_udp_selector()->mutable_dst_port_range()->set_port_high(port_end);
            }
            break;
        case IP_PROTO_TCP:
            if (ip_mask->u.tcp.sport) {
                ret = key_mask_to_l4port_range(ip_key->u.tcp.sport, 
                                               ip_mask->u.tcp.sport,
                                               &port_start,
                                               &port_end);
                ip_sel->mutable_tcp_selector()->mutable_src_port_range()->set_port_low(port_start);
                ip_sel->mutable_tcp_selector()->mutable_src_port_range()->set_port_high(port_end);
            }
            if (ip_mask->u.tcp.dport) {
                ret = key_mask_to_l4port_range(ip_key->u.tcp.dport, 
                                               ip_mask->u.tcp.dport,
                                               &port_start,
                                               &port_end);
                ip_sel->mutable_tcp_selector()->mutable_dst_port_range()->set_port_low(port_start);
                ip_sel->mutable_tcp_selector()->mutable_dst_port_range()->set_port_high(port_end);
            }

            if (tcp_flags_mask & TCP_FLAG_SYN) {
                if (tcp_flags & TCP_FLAG_SYN) {
                    ip_sel->mutable_tcp_selector()->set_tcp_syn_set(true);
                } else {
                    ip_sel->mutable_tcp_selector()->set_tcp_syn_clear(true);
                }
            }
            if (tcp_flags_mask & TCP_FLAG_ACK) {
                if (tcp_flags & TCP_FLAG_ACK) {
                    ip_sel->mutable_tcp_selector()->set_tcp_ack_set(true);
                } else {
                    ip_sel->mutable_tcp_selector()->set_tcp_ack_clear(true);
                }
            }
            if (tcp_flags_mask & TCP_FLAG_FIN) {
                if (tcp_flags & TCP_FLAG_FIN) {
                    ip_sel->mutable_tcp_selector()->set_tcp_fin_set(true);
                } else {
                    ip_sel->mutable_tcp_selector()->set_tcp_fin_clear(true);
                }
            }
            if (tcp_flags_mask & TCP_FLAG_RST) {
                if (tcp_flags & TCP_FLAG_RST) {
                    ip_sel->mutable_tcp_selector()->set_tcp_rst_set(true);
                } else {
                    ip_sel->mutable_tcp_selector()->set_tcp_rst_clear(true);
                }
            }
            if (tcp_flags_mask & TCP_FLAG_URG) {
                if (tcp_flags & TCP_FLAG_URG) {
                    ip_sel->mutable_tcp_selector()->set_tcp_urg_set(true);
                } else {
                    ip_sel->mutable_tcp_selector()->set_tcp_urg_clear(true);
                }
            }
            break;
        default:
            // TODO Fixme
            //ip_sel->set_ip_protocol(ip_key->ip_proto);
            break;
    }
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

#ifdef ACL_DOL_TEST_ONLY
// Added for internal dol test use only to get the drop reason defines
// TODO: REMOVE
#include "nic/p4/nw/include/defines.h"
static uint64_t
drop_reason_to_define (const acl::DropReason drop_reason)
{
    switch(drop_reason) {
        case acl::INPUT_MAPPING__DROP:
            return DROP_INPUT_MAPPING;
        case acl::INPUT_MAPPING_DEJAVU__DROP:
            return DROP_INPUT_MAPPING_DEJAVU;
        case acl::FLOW_HIT__DROP:
            return DROP_FLOW_HIT;
        case acl::FLOW_MISS__DROP:
            return DROP_FLOW_MISS;
        case acl::IPSG__DROP:
            return DROP_IPSG;
        case acl::INGRESS_POLICER__DROP:
            return DROP_INGRESS_POLICER;
        case acl::RX_POLICER__DROP:
            return DROP_RX_POLICER;
        case acl::NACL__DROP:
            return DROP_NACL;
        case acl::MALFORMED_PKT__DROP:
            return DROP_MALFORMED_PKT;
        case acl::PING_OF_DEATH__DROP:
            return DROP_PING_OF_DEATH;
        case acl::FRAGMENT_TOO_SMALL__DROP:
            return DROP_FRAGMENT_TOO_SMALL;
        case acl::IP_NORMALIZATION__DROP:
            return DROP_IP_NORMALIZATION;
        case acl::TCP_NORMALIZATION__DROP:
            return DROP_TCP_NORMALIZATION;
        case acl::TCP_NON_SYN_FIRST_PKT__DROP:
            return DROP_TCP_NON_SYN_FIRST_PKT;
        case acl::ICMP_NORMALIZATION__DROP:
            return DROP_ICMP_NORMALIZATION;
        case acl::ICMP_SRC_QUENCH_MSG__DROP:
            return DROP_ICMP_SRC_QUENCH_MSG;
        case acl::ICMP_REDIRECT_MSG__DROP:
            return DROP_ICMP_REDIRECT_MSG;
        case acl::ICMP_INFO_REQ_MSG__DROP:
            return DROP_ICMP_INFO_REQ_MSG;
        case acl::ICMP_ADDR_REQ_MSG__DROP:
            return DROP_ICMP_ADDR_REQ_MSG;
        case acl::ICMP_TRACEROUTE_MSG__DROP:
            return DROP_ICMP_TRACEROUTE_MSG;
        case acl::ICMP_RSVD_TYPE_MSG__DROP:
            return DROP_ICMP_RSVD_TYPE_MSG;
        case acl::INPUT_PROPERTIES_MISS__DROP:
            return DROP_INPUT_PROPERTIES_MISS;
        case acl::TCP_OUT_OF_WINDOW__DROP:
            return DROP_TCP_OUT_OF_WINDOW;
        case acl::TCP_SPLIT_HANDSHAKE__DROP:
            return DROP_TCP_SPLIT_HANDSHAKE;
        case acl::TCP_WIN_ZERO_DROP__DROP:
            return DROP_TCP_WIN_ZERO_DROP;
        case acl::TCP_ACK_ERR__DROP:
            return DROP_TCP_ACK_ERR;
        case acl::TCP_DATA_AFTER_FIN__DROP:
            return DROP_TCP_DATA_AFTER_FIN;
        case acl::TCP_NON_RST_PKT_AFTER_RST__DROP:
            return DROP_TCP_NON_RST_PKT_AFTER_RST;
        case acl::TCP_INVALID_RESPONDER_FIRST_PKT__DROP:
            return DROP_TCP_INVALID_RESPONDER_FIRST_PKT;
        case acl::TCP_UNEXPECTED_PKT__DROP:
            return DROP_TCP_UNEXPECTED_PKT;
        default:
            return 0;
    }
    return 0;
}

#endif

static hal_ret_t
extract_match_spec (acl_match_spec_t *ms,
                    const acl::AclSelector &sel)
{
    hal_ret_t            ret = HAL_RET_OK;
    vrf_t             *vrf = NULL;
    vrf_id_t          vrf_id;
    hal_handle_t         vrf_handle = 0;
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
#ifdef ACL_DOL_TEST_ONLY
    int                  i;
#endif

    eth_key = &ms->key.eth;
    eth_mask = &ms->mask.eth;
    ip_key = &ms->key.ip;
    ip_mask = &ms->mask.ip;

    if (sel.has_src_if_key_handle()) {
        ms->src_if_match = true;

        auto src_if_kh = sel.src_if_key_handle();
        if (src_if_kh.key_or_handle_case() == kh::InterfaceKeyHandle::kInterfaceId) {
            src_if_id = src_if_kh.interface_id();
            src_if = find_if_by_id(src_if_id);
        } else {
            src_if_handle = src_if_kh.if_handle();
            src_if = find_if_by_handle(src_if_handle);
        }

        if(src_if == NULL) {
            HAL_TRACE_ERR("pi-acl:{}: Source interface not found",
                          __func__);
            ret = HAL_RET_IF_NOT_FOUND;
            goto end;
        } else {
            ms->src_if_handle = src_if->hal_handle;
        }
    }

    if (sel.has_dst_if_key_handle()) {
        ms->dest_if_match = true;

        auto dest_if_kh = sel.dst_if_key_handle();
        if (dest_if_kh.key_or_handle_case() == kh::InterfaceKeyHandle::kInterfaceId) {
            dest_if_id = dest_if_kh.interface_id();
            dest_if = find_if_by_id(dest_if_id);
        } else {
            dest_if_handle = dest_if_kh.if_handle();
            dest_if = find_if_by_handle(dest_if_handle);
        }

        if(dest_if == NULL) {
            HAL_TRACE_ERR("pi-acl:{}: Destination interface not found",
                          __func__);
            ret = HAL_RET_IF_NOT_FOUND;
            goto end;
        } else {
            ms->dest_if_handle = dest_if->hal_handle;
        }
    }

    if (sel.seg_selector_case() == acl::AclSelector::kVrfKeyHandle) {
        ms->vrf_match = true;

        auto vrf_kh = sel.vrf_key_handle();
        if (vrf_kh.key_or_handle_case() == kh::VrfKeyHandle::kVrfId) {
            vrf_id = vrf_kh.vrf_id();
            vrf = vrf_lookup_by_id(vrf_id);
        } else {
            vrf_handle = vrf_kh.vrf_handle();
            vrf = vrf_lookup_by_handle(vrf_handle);
        }

        if(vrf == NULL) {
            HAL_TRACE_ERR("pi-acl:{}: Vrf not found",
                          __func__);
            ret = HAL_RET_VRF_NOT_FOUND;
            goto end;
        } else {
            ms->vrf_handle = vrf->hal_handle;
        }
    } else if (sel.seg_selector_case() == acl::AclSelector::kL2SegmentKeyHandle) {
        ms->l2seg_match = true;

        auto l2seg_kh = sel.l2segment_key_handle();
        if (l2seg_kh.key_or_handle_case() == kh::L2SegmentKeyHandle::kSegmentId) {
            l2seg_id = l2seg_kh.segment_id();
            l2seg = find_l2seg_by_id(l2seg_id);
        } else {
            l2seg_handle = l2seg_kh.l2segment_handle();
            l2seg = l2seg_lookup_by_handle(l2seg_handle);
        }

        if(l2seg == NULL) {
            HAL_TRACE_ERR("pi-acl:{}: L2 segment not found",
                          __func__);
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
                                    sel.ip_selector());
            break;
        case ACL_TYPE_IPv4:
            if (sel.ip_selector().has_src_prefix()) {
                ip_addr_spec_to_ip_addr(&ip_key->sip, 
                                        sel.ip_selector().src_prefix().address());
                // Convert prefix len to mask
                ip_mask->sip.addr.v4_addr = ipv4_prefix_len_to_mask(
                    sel.ip_selector().src_prefix().prefix_len());
            }

            if (sel.ip_selector().has_dst_prefix()) {
                ip_addr_spec_to_ip_addr(&ip_key->dip, 
                                        sel.ip_selector().dst_prefix().address());
                // Convert prefix len to mask
                ip_mask->dip.addr.v4_addr = ipv4_prefix_len_to_mask(
                    sel.ip_selector().dst_prefix().prefix_len());
            }

            ret = extract_ip_common(ms->acl_type, ip_key, ip_mask,
                                    sel.ip_selector());
            break;
        case ACL_TYPE_IPv6:
            if (sel.ip_selector().has_src_prefix()) {
                ip_addr_spec_to_ip_addr(&ip_key->sip, 
                                        sel.ip_selector().src_prefix().address());
                // Convert prefix len to mask
                ipv6_prefix_len_to_mask(&ip_mask->sip.addr.v6_addr,
                                        sel.ip_selector().src_prefix().prefix_len());
            }

            if (sel.ip_selector().has_dst_prefix()) {
                ip_addr_spec_to_ip_addr(&ip_key->dip, 
                                        sel.ip_selector().dst_prefix().address());
                // Convert prefix len to mask
                ipv6_prefix_len_to_mask(&ip_mask->dip.addr.v6_addr,
                                        sel.ip_selector().dst_prefix().prefix_len());
            }

            ret = extract_ip_common(ms->acl_type, ip_key, ip_mask,
                                    sel.ip_selector());
            break;
        case ACL_TYPE_INVALID:
            HAL_TRACE_ERR("pi-acl:{}: ACL Type invalid",
                          __func__);
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
        HAL_TRACE_ERR("pi-acl:{}: ACL Internal selector key/mask not specified",
                      __func__);
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
        for (i = 0; i < sel.internal_key().drop_reason_size(); i++) {
            ms->int_key.drop_reason |=
                1ull << drop_reason_to_define(sel.internal_key().drop_reason(i));
        }
        for (i = 0; i < sel.internal_mask().drop_reason_size(); i++) {
            ms->int_mask.drop_reason |=
                1ull << drop_reason_to_define(sel.internal_mask().drop_reason(i));
        }
        MAC_UINT64_TO_ADDR(ms->int_key.outer_mac_da, sel.internal_key().outer_dst_mac());
        MAC_UINT64_TO_ADDR(ms->int_mask.outer_mac_da, sel.internal_mask().outer_dst_mac());
    }
#endif
end:
    return ret;
}

static hal_ret_t
populate_match_spec (acl_match_spec_t *ms,
                    acl::AclSelector *sel)
{
    hal_ret_t   ret = HAL_RET_OK;
    ip_prefix_t ip_pfx;
    acl_eth_match_spec_t *eth_key;
    acl_eth_match_spec_t *eth_mask;
    acl_ip_match_spec_t  *ip_key;
    acl_ip_match_spec_t  *ip_mask;

    eth_key = &ms->key.eth;
    eth_mask = &ms->mask.eth;
    ip_key = &ms->key.ip;
    ip_mask = &ms->mask.ip;

    if (ms->src_if_match) {
        sel->mutable_src_if_key_handle()->set_if_handle(ms->src_if_handle);
    }
    if (ms->dest_if_match) {
        sel->mutable_dst_if_key_handle()->set_if_handle(ms->dest_if_handle);
    }

    if (ms->vrf_match) {
        sel->mutable_vrf_key_handle()->set_vrf_handle(ms->vrf_handle);
    } 

    if (ms->l2seg_match) {
        sel->mutable_l2segment_key_handle()->set_l2segment_handle(ms->l2seg_handle);
    }

    switch (ms->acl_type) {
        case ACL_TYPE_NONE:
            break;
        case ACL_TYPE_ETH:
            sel->mutable_eth_selector()->set_eth_type(eth_key->ether_type);
            sel->mutable_eth_selector()->set_eth_type_mask(eth_mask->ether_type);

            sel->mutable_eth_selector()->set_src_mac(MAC_TO_UINT64(eth_key->mac_sa));
            sel->mutable_eth_selector()->set_src_mac_mask(MAC_TO_UINT64(eth_mask->mac_sa));

            sel->mutable_eth_selector()->set_dst_mac(MAC_TO_UINT64(eth_key->mac_da));
            sel->mutable_eth_selector()->set_dst_mac_mask(MAC_TO_UINT64(eth_mask->mac_da));
            break;
        case ACL_TYPE_IP:
            sel->mutable_ip_selector()->set_ip_af(types::IP_AF_NONE);
            ret = populate_ip_common(ip_key, ip_mask, sel->mutable_ip_selector());
            break;
        case ACL_TYPE_IPv4:
            sel->mutable_ip_selector()->set_ip_af(types::IP_AF_INET);

            ip_pfx.len = ipv4_mask_to_prefix_len(ip_mask->sip.addr.v4_addr);
            ip_pfx.addr = ip_key->sip;
            if (ip_pfx.len) {
                ip_pfx_to_spec(sel->mutable_ip_selector()->mutable_src_prefix(), &ip_pfx);
            }

            ip_pfx.len = ipv4_mask_to_prefix_len(ip_mask->dip.addr.v4_addr);
            ip_pfx.addr = ip_key->dip;
            if (ip_pfx.len) {
                ip_pfx_to_spec(sel->mutable_ip_selector()->mutable_dst_prefix(), &ip_pfx);
            }

            ret = populate_ip_common(ip_key, ip_mask, sel->mutable_ip_selector());
            break;
        case ACL_TYPE_IPv6:
            sel->mutable_ip_selector()->set_ip_af(types::IP_AF_INET6);

            ip_pfx.len = ipv6_mask_to_prefix_len(&ip_mask->sip.addr.v6_addr);
            ip_pfx.addr = ip_key->sip;
            if (ip_pfx.len) {
                ip_pfx_to_spec(sel->mutable_ip_selector()->mutable_src_prefix(), &ip_pfx);
            }

            ip_pfx.len = ipv6_mask_to_prefix_len(&ip_mask->dip.addr.v6_addr);
            ip_pfx.addr = ip_key->dip;
            if (ip_pfx.len) {
                ip_pfx_to_spec(sel->mutable_ip_selector()->mutable_dst_prefix(), &ip_pfx);
            }

            ret = populate_ip_common(ip_key, ip_mask, sel->mutable_ip_selector());
            break;
        case ACL_TYPE_INVALID:
            break;
    }
    return ret;
}

// extract mirror sessions specified in spec into ingress and egress bitmaps
//------------------------------------------------------------------------------
static hal_ret_t
extract_mirror_sessions (const acl::AclActionInfo &ainfo, uint8_t *ingress, uint8_t *egress)
{
    int i;
    *ingress = 0;
    *egress = 0;
    for (i = 0; i < ainfo.ing_mirror_sessions_size(); ++i) {
        uint32_t id = ainfo.ing_mirror_sessions(i).session_id();
        if (id > 7) {
            return HAL_RET_INVALID_ARG;
        }
        *ingress = *ingress | (1 << id);
        HAL_TRACE_DEBUG("  Adding ingress session {}", id);
    }
    for (i = 0; i < ainfo.egr_mirror_sessions_size(); ++i) {
        uint32_t id = ainfo.egr_mirror_sessions(i).session_id();
        if (id > 7) {
            return HAL_RET_INVALID_ARG;
        }
        *egress = *egress | (1 << id);
        HAL_TRACE_DEBUG("  Adding egress session {}", id);
    }
    return HAL_RET_OK;
}

static hal_ret_t
extract_action_spec (acl_action_spec_t *as,
                    const acl::AclActionInfo &ainfo)
{
    hal_ret_t    ret = HAL_RET_OK;
    if_t         *redirect_if = NULL;
    if_id_t      redirect_if_id;
    hal_handle_t redirect_if_handle = 0;
    uint8_t      ingress, egress;
#ifdef ACL_DOL_TEST_ONLY
    // Internal fields for use only with DOL/testing infra
    // For production builds this needs to be removed
    // TODO: REMOVE
    hal::pd::pd_rw_entry_args_t rw_key{};
#endif

    as->action = ainfo.action();
    ret = extract_mirror_sessions(ainfo, &ingress, &egress);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-acl:{}: Error extracting mirror sessions",
                      __func__);
        goto end;
    }
    as->ing_mirror_en = ingress ? true : false;
    as->ing_mirror_session = ingress;
    as->egr_mirror_en = egress ? true : false;
    as->egr_mirror_session = egress;
    as->copp_policer_handle = ainfo.copp_policer_handle();

    if (ainfo.has_redirect_if_key_handle()) {
        if (as->action != acl::ACL_ACTION_REDIRECT) {
            HAL_TRACE_ERR("pi-acl:{}: Redirect interface specified with action {} ",
                          __func__, as->action);
            ret = HAL_RET_INVALID_ARG;
            goto end;
        }

        auto redirect_if_kh = ainfo.redirect_if_key_handle();
        if (redirect_if_kh.key_or_handle_case() ==
            kh::InterfaceKeyHandle::kInterfaceId) {
            redirect_if_id = redirect_if_kh.interface_id();
            redirect_if = find_if_by_id(redirect_if_id);
        } else {
            redirect_if_handle = redirect_if_kh.if_handle();
            redirect_if = find_if_by_handle(redirect_if_handle);
        }

        if(redirect_if == NULL) {
            HAL_TRACE_ERR("pi-acl:{}: Redirect interface not found",
                          __func__);
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
            HAL_TRACE_ERR("pi-acl:{}: Redirect action fields specified for "
                          "non-redirect action {}",
                          __func__, as->action);
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
            HAL_TRACE_ERR("pi-acl:{}: Unable to find/alloc rw entry",
                          __func__);
            goto end;
        }
    }
#endif
end:
    return ret;
}

static hal_ret_t
populate_action_spec (acl_action_spec_t *as,
                      acl::AclActionInfo *ainfo)
{
    ainfo->set_action(as->action);
    ainfo->set_copp_policer_handle(as->copp_policer_handle);
    ainfo->mutable_redirect_if_key_handle()->set_if_handle(as->redirect_if_handle);
#ifdef ACL_DOL_TEST_ONLY
    // Internal fields for use only with DOL/testing infra
    // For production builds this needs to be removed
    ainfo->mutable_internal_actions()->set_mac_sa_rewrite_en(as->int_as.mac_sa_rewrite);
    ainfo->mutable_internal_actions()->set_mac_da_rewrite_en(as->int_as.mac_da_rewrite);
    ainfo->mutable_internal_actions()->set_ttl_dec_en(as->int_as.ttl_dec);
    ainfo->mutable_internal_actions()->mutable_encap_info()->set_encap_value(as->int_as.tnnl_vnid);
    // TODO rewrite actions
#endif
    return HAL_RET_OK;
}

static hal_ret_t
acl_populate_from_spec (acl_t *acl, AclSpec& spec)
{
    hal_ret_t ret = HAL_RET_OK;
    // save the configs from the spec
    if (spec.has_key_or_handle() && 
        (spec.key_or_handle().key_or_handle_case() == AclKeyHandle::kAclId)) {
        acl->key.acl_id = spec.key_or_handle().acl_id();
    }

    acl->priority = spec.priority();
    ret = extract_match_spec(&acl->match_spec, spec.match());
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-acl:{}: Acl create request match parsing failed."
                      " Err: {}",
                      __func__, ret);
        goto end;
    }

    ret = extract_action_spec(&acl->action_spec, spec.action());
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-acl:{}: Acl create request action parsing failed."
                      " Err: {}",
                      __func__, ret);
        goto end;
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// process a acl create request
//------------------------------------------------------------------------------
hal_ret_t
acl_create (AclSpec& spec, AclResponse *rsp)
{
    hal_ret_t     ret = HAL_RET_OK;
    acl_t         *acl = NULL;
    dhl_entry_t   dhl_entry = { 0 };
    cfg_op_ctxt_t cfg_ctxt = { 0 };

    hal_api_trace(" API Begin: acl create");
    // dump spec
    acl_spec_print(spec);

    ret = validate_acl_create(spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-acl:{}: Validation failed ret {}", __func__, ret);
        goto end;
    }

    HAL_TRACE_DEBUG("pi-acl:{}: acl create for acl-id {} ",
                    __func__,
                    spec.key_or_handle().acl_id());

    // instantiate acl
    acl = acl_alloc_init();
    if (acl == NULL) {
        HAL_TRACE_ERR("pi-acl:{}:unable to allocate handle/memory ret: {}",
                      __func__, ret);
        ret = HAL_RET_OOM;
        goto end;
    }

    // initialize the acl record
    // populate from the spec
    ret = acl_populate_from_spec(acl, spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-acl:{}: error in populating acl from spec",
                      __func__);
        goto end;
    }

    // allocate hal handle id
    acl->hal_handle = hal_handle_alloc(HAL_OBJ_ID_ACL);
    if (acl->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("pi-acl:{}: failed to alloc handle",
                      __func__);
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    // form ctxt and call infra add

    dhl_entry.handle = acl->hal_handle;
    dhl_entry.obj = acl;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(acl->hal_handle, &cfg_ctxt,
                             acl_create_add_cb,
                             acl_create_commit_cb,
                             acl_create_abort_cb,
                             acl_create_cleanup_cb);

    if (ret != HAL_RET_OK) {
        // acl was freed during abort, pointer not valid anymore
        acl = NULL;
    }
end:
    if (ret != HAL_RET_OK) {
        if (acl) {
            acl_free(acl, true);
            acl = NULL;
        }
    }

    acl_prepare_rsp(rsp, ret, acl ? acl->hal_handle : HAL_HANDLE_INVALID);
    hal_api_trace(" API End: acl create");
    return ret;
}

//------------------------------------------------------------------------------
// validate acl update request
//------------------------------------------------------------------------------
hal_ret_t
validate_acl_update (AclSpec& spec, AclResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("pi-acl:{}:spec has no key or handle", __func__);
        ret =  HAL_RET_INVALID_ARG;
    }

    return ret;
}

//------------------------------------------------------------------------------
// This is the first call back infra does for update.
// 1. PD Call to update PD
//------------------------------------------------------------------------------
hal_ret_t
acl_update_upd_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t             ret = HAL_RET_OK;
    pd::pd_acl_args_t     pd_acl_args = { 0 };
    dllist_ctxt_t         *lnode = NULL;
    dhl_entry_t           *dhl_entry = NULL;
    acl_t                 *acl_clone = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-acl{}:invalid cfg_ctxt", __func__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    acl_clone = (acl_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("pi-acl:{}:update upd CB {}",
                    __func__, acl_clone->key);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_acl_args_init(&pd_acl_args);
    pd_acl_args.acl = acl_clone;
    ret = pd::pd_acl_update(&pd_acl_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-acl:{}:failed to update acl pd, err : {}",
                      __func__, ret);
    }

end:
    return ret;
}


//------------------------------------------------------------------------------
// Make a clone
// - Both PI and PD objects cloned.
//------------------------------------------------------------------------------
hal_ret_t
acl_make_clone (acl_t *acl, acl_t **acl_clone_p, AclSpec& spec)
{
    hal_ret_t ret = HAL_RET_OK;
    acl_t *acl_clone;

    *acl_clone_p = acl_alloc_init();
    acl_clone = *acl_clone_p;

    acl_clone->key = acl->key;
    acl_clone->hal_handle = acl->hal_handle;
    acl_clone->pd = NULL;

    pd::pd_acl_make_clone(acl, acl_clone);

    // Update with the new spec
    ret = acl_populate_from_spec(acl_clone, spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-acl:{}: error in populating acl from spec",
                      __func__);
        goto end;
    }

end:
    if (ret != HAL_RET_OK) {
        if (*acl_clone_p) {
            acl_free(*acl_clone_p, true);
            *acl_clone_p = NULL;
        }
    }
    return ret;
}

//------------------------------------------------------------------------------
// After all hw programming is done
//  1. Free original PI & PD acl.
// Note: Infra make clone as original by replacing original pointer by clone.
//------------------------------------------------------------------------------
hal_ret_t
acl_update_commit_cb(cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t             ret = HAL_RET_OK;
    dllist_ctxt_t         *lnode = NULL;
    dhl_entry_t           *dhl_entry = NULL;
    acl_t                 *acl = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-acl{}:invalid cfg_ctxt", __func__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    acl = (acl_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("pi-acl:{}:update commit CB {}",
                    __func__, acl->key);

    // Free PI.
    acl_free(acl, true);
end:
    return ret;
}

//------------------------------------------------------------------------------
// Update didnt go through.
//  1. Kill the clones
//------------------------------------------------------------------------------
hal_ret_t
acl_update_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t         ret = HAL_RET_OK;
    dllist_ctxt_t     *lnode = NULL;
    dhl_entry_t       *dhl_entry = NULL;
    acl_t             *acl_clone = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-acl{}:invalid cfg_ctxt", __func__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    acl_clone = (acl_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("pi-acl:{}:update abort CB {}",
                    __func__, acl_clone->key);

    // Free Clone
    acl_free(acl_clone, true);
end:

    return ret;
}

hal_ret_t
acl_update_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a acl update request
//------------------------------------------------------------------------------
hal_ret_t
acl_update (AclSpec& spec, AclResponse *rsp)
{
    hal_ret_t             ret = HAL_RET_OK;
    acl_t                 *acl = NULL;
    cfg_op_ctxt_t         cfg_ctxt = { 0 };
    dhl_entry_t           dhl_entry = { 0 };
    const AclKeyHandle    &kh = spec.key_or_handle();
    acl_t                 *acl_clone = NULL;

    hal_api_trace(" API Begin: acl update");

    // dump spec
    acl_spec_print(spec);

    // validate the request message
    ret = validate_acl_update(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-acl:{}:acl delete validation failed, ret : {}",
                      __func__, ret);
        goto end;
    }

    acl = acl_lookup_by_key_or_handle(kh);
    if (acl == NULL) {
        HAL_TRACE_ERR("pi-acl:{}:failed to find acl, id {}, handle {}",
                      __func__, kh.acl_id(), kh.acl_handle());
        ret = HAL_RET_ACL_NOT_FOUND;
        goto end;
    }
    HAL_TRACE_DEBUG("pi-acl:{}:update acl {}", __func__,
                    acl->key);

    acl_make_clone(acl, (acl_t **)&dhl_entry.cloned_obj, spec);

    acl_clone = (acl_t *)dhl_entry.cloned_obj;
    if (acl->priority != acl_clone->priority) {
        HAL_TRACE_ERR("pi-acl:{}: priority updates are currently not supported",
                      __func__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // form ctxt and call infra update object
    dhl_entry.handle = acl->hal_handle;
    dhl_entry.obj = acl;
    cfg_ctxt.app_ctxt = NULL;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_upd_obj(acl->hal_handle, &cfg_ctxt,
                             acl_update_upd_cb,
                             acl_update_commit_cb,
                             acl_update_abort_cb,
                             acl_update_cleanup_cb);

    if (ret != HAL_RET_OK) {
        // acl_clone was freed during abort, pointer not valid anymore
        acl_clone = NULL;
    }
end:
    if (ret != HAL_RET_OK) {
        if (acl_clone) {
            acl_free(acl_clone, true);
            acl_clone = NULL;
        }
    }
    acl_prepare_rsp(rsp, ret,
                       acl ? acl->hal_handle : HAL_HANDLE_INVALID);
    hal_api_trace(" API End: acl update ");
    return ret;
}

//------------------------------------------------------------------------------
// process a acl get request
//------------------------------------------------------------------------------
hal_ret_t
acl_get (AclGetRequest& req, AclGetResponse *rsp)
{
    acl_t        *acl;
    AclSpec      *spec;
    hal_ret_t    ret = HAL_RET_OK;

    hal_api_trace(" API Begin: acl get ");
    // key-handle field must be set
    if (!req.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    acl = acl_lookup_by_key_or_handle(req.key_or_handle());
    if (acl == NULL) {
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_ACL_NOT_FOUND;
    }

    // fill config spec of this acl
    spec = rsp->mutable_spec();
    spec->mutable_key_or_handle()->set_acl_id(acl->key.acl_id);
    spec->set_priority(acl->priority);

    ret = populate_match_spec(&acl->match_spec, spec->mutable_match());
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-acl:{}: Error populating match spec for acl {} ret {}",
                      __func__, acl->key, ret);
    }

    ret = populate_action_spec(&acl->action_spec, spec->mutable_action());
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-acl:{}: Error populating action spec for acl {} ret {}",
                      __func__, acl->key, ret);
    }

    // fill operational state of this acl
    rsp->mutable_status()->set_acl_handle(acl->hal_handle);

    // fill stats of this acl
    rsp->set_api_status(types::API_STATUS_OK);
    hal_api_trace(" API End: acl get ");
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// validate acl delete request
//------------------------------------------------------------------------------
hal_ret_t
validate_acl_delete_req (AclDeleteRequest& req, AclDeleteResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    // key-handle field must be set
    if (!req.has_key_or_handle()) {
        HAL_TRACE_ERR("pi-acl:{}:spec has no key or handle", __func__);
        ret =  HAL_RET_INVALID_ARG;
    }

    return ret;
}

//------------------------------------------------------------------------------
// 1. PD Call to delete PD and free up resources and deprogram HW
//------------------------------------------------------------------------------
hal_ret_t
acl_delete_del_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret         = HAL_RET_OK;
    pd::pd_acl_args_t           pd_acl_args = { 0 };
    dllist_ctxt_t               *lnode      = NULL;
    dhl_entry_t                 *dhl_entry  = NULL;
    acl_t                       *acl        = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-acl:{}:invalid cfg_ctxt", __func__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // TODO: Check the dependency ref count for the acl.
    //       If its non zero, fail the delete.


    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    acl = (acl_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("pi-acl:{}:delete del CB {} handle {}",
                    __func__, acl->key, acl->hal_handle);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_acl_args_init(&pd_acl_args);
    pd_acl_args.acl = acl;
    ret = pd::pd_acl_delete(&pd_acl_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-acl:{}:failed to delete acl pd, err : {}",
                      __func__, ret);
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// Update PI DBs as acl_delete_del_cb() was a succcess
//      a. Delete from acl id hash table
//      b. Remove object from handle id based hash table
//      c. Free PI acl
//------------------------------------------------------------------------------
hal_ret_t
acl_delete_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t       ret = HAL_RET_OK;
    dllist_ctxt_t   *lnode = NULL;
    dhl_entry_t     *dhl_entry = NULL;
    acl_t           *acl = NULL;
    hal_handle_t    hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-acl:{}:invalid cfg_ctxt", __func__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    acl = (acl_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("pi-acl:{}:delete commit CB {} handle {}",
                    __func__, acl->key, acl->hal_handle);

    // a. Remove from acl id hash table
    ret = acl_del_from_db(acl);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-acl:{}:failed to del acl {} from db, err : {}",
                      __func__, acl->key, ret);
        goto end;
    }

    // b. Remove object from handle id based hash table
    hal_handle_free(hal_handle);

    // c. Free PI acl
    acl_free(acl, false);

end:
    return ret;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
acl_delete_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
acl_delete_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a acl delete request
//------------------------------------------------------------------------------
hal_ret_t
acl_delete (AclDeleteRequest& req, AclDeleteResponse *rsp)
{
    hal_ret_t          ret = HAL_RET_OK;
    acl_t              *acl = NULL;
    cfg_op_ctxt_t      cfg_ctxt = { 0 };
    dhl_entry_t        dhl_entry = { 0 };
    const AclKeyHandle &kh = req.key_or_handle();

    hal_api_trace(" API Begin: acl delete ");

    // validate the request message
    ret = validate_acl_delete_req(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-acl:{}:acl delete validation failed, ret : {}",
                      __func__, ret);
        goto end;
    }

    acl = acl_lookup_by_key_or_handle(kh);
    if (acl == NULL) {
        HAL_TRACE_ERR("pi-acl:{}:failed to find acl, id {}, handle {}",
                      __func__, kh.acl_id(), kh.acl_handle());
        ret = HAL_RET_ACL_NOT_FOUND;
        goto end;
    }

    HAL_TRACE_DEBUG("pi-acl:{}:deleting acl {} handle {}",
                    __func__, acl->key, acl->hal_handle);

    // form ctxt and call infra add
    dhl_entry.handle = acl->hal_handle;
    dhl_entry.obj = acl;
    cfg_ctxt.app_ctxt = NULL;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_del_obj(acl->hal_handle, &cfg_ctxt,
                             acl_delete_del_cb,
                             acl_delete_commit_cb,
                             acl_delete_abort_cb,
                             acl_delete_cleanup_cb);

end:
    rsp->set_api_status(hal_prepare_rsp(ret));
    hal_api_trace(" API End: acl delete ");
    return ret;
}
}    // namespace hal
