//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "gen/hal/include/hal_api_stats.hpp"
#include "nic/hal/plugins/cfg/aclqos/acl.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "gen/proto/acl.pb.h"
#include "nic/hal/src/utils/if_utils.hpp"
#include <google/protobuf/util/json_util.h>

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

    SDK_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    acl = find_acl_by_handle(ht_entry->handle_id);
    return (void *)&(acl->key);
}

// ----------------------------------------------------------------------------
// hash table acl_type key size
// ----------------------------------------------------------------------------
uint32_t
acl_key_size ()
{
    return sizeof(acl_key_t);
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

    HAL_TRACE_DEBUG("adding to acl hash table");
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
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("failed to add key to handle mapping, "
                      "err : {}", ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }

    // TODO: check if this is the right place
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

    HAL_TRACE_DEBUG("removing from hash table");

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
    pd::pd_acl_mem_free_args_t pd_acl_args = { 0 };
    pd::pd_func_args_t          pd_func_args = {0};
    if (!acl) {
        return HAL_RET_OK;
    }
    if (free_pd) {
        pd::pd_acl_mem_free_args_init(&pd_acl_args);
        pd_acl_args.acl = acl;
        pd_func_args.pd_acl_mem_free = &pd_acl_args;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_ACL_MEM_FREE, &pd_func_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("failed to delete acl pd, err : {}",
                          ret);
            return ret;
        }
    }
    SDK_SPINLOCK_DESTROY(&acl->slock);
    hal::delay_delete_to_slab(HAL_SLAB_ACL, acl);
    return ret;
}


//-----------------------------------------------------------------------------
// Print acl spec
//-----------------------------------------------------------------------------
static hal_ret_t
acl_spec_dump (AclSpec& spec)
{
    std::string buf;
    google::protobuf::util::JsonPrintOptions options;

    if (hal::utils::hal_trace_level() < ::utils::trace_debug)  {
        return HAL_RET_OK;
    }

    options.add_whitespace = false;
    options.preserve_proto_field_names = true;
    google::protobuf::util::MessageToJsonString(spec, &buf, options);
    HAL_TRACE_DEBUG("Acl Spec: {}", buf);
    return HAL_RET_OK;
}

static hal_ret_t
acl_add_refs (acl_t *acl)
{
    hal_ret_t ret = HAL_RET_OK;
    acl_match_spec_t *ms = &acl->match_spec;
    acl_action_spec_t *as = &acl->action_spec;

    if (ms->vrf_match) {
        ret = vrf_add_acl(vrf_lookup_by_handle(ms->vrf_handle), acl);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to add ref to vrf ret {}", ret);
            goto end;
        }
    }

    if (ms->src_if_match) {
        ret = if_add_acl(find_if_by_handle(ms->src_if_handle), acl,
                         IF_ACL_REF_TYPE_SRC);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to add ref to src if ret {}", ret);
            goto end;
        }
    }

    if (ms->dest_if_match) {
        ret = if_add_acl(find_if_by_handle(ms->dest_if_handle), acl,
                         IF_ACL_REF_TYPE_DEST);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to add ref to dest if ret {}", ret);
            goto end;
        }
    }

    if (ms->l2seg_match) {
        ret = l2seg_add_acl(l2seg_lookup_by_handle(ms->l2seg_handle), acl);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to add ref to l2seg ret {}", ret);
            goto end;
        }
    }

    if (as->redirect_if_handle != HAL_HANDLE_INVALID) {
        ret = if_add_acl(find_if_by_handle(as->redirect_if_handle), acl,
                         IF_ACL_REF_TYPE_REDIRECT);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to add ref to redirect if ret {}", ret);
            goto end;
        }
    }

    if (as->copp_handle != HAL_HANDLE_INVALID) {
        ret = copp_add_acl(find_copp_by_handle(as->copp_handle), acl);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to add ref to copp ret {}", ret);
            goto end;
        }
    }

end:
    return ret;
}

static hal_ret_t
acl_rem_refs (acl_t *acl)
{
    hal_ret_t ret = HAL_RET_OK;
    acl_match_spec_t *ms = &acl->match_spec;
    acl_action_spec_t *as = &acl->action_spec;

    if (ms->vrf_match) {
        ret = vrf_del_acl(vrf_lookup_by_handle(ms->vrf_handle), acl);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to del ref from vrf ret {}", ret);
            goto end;
        }
    }

    if (ms->src_if_match) {
        ret = if_del_acl(find_if_by_handle(ms->src_if_handle), acl,
                         IF_ACL_REF_TYPE_SRC);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to del ref to src if ret {}", ret);
            goto end;
        }
    }

    if (ms->dest_if_match) {
        ret = if_del_acl(find_if_by_handle(ms->dest_if_handle), acl,
                         IF_ACL_REF_TYPE_DEST);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to del ref to dest if ret {}", ret);
            goto end;
        }
    }

    if (ms->l2seg_match) {
        ret = l2seg_del_acl(l2seg_lookup_by_handle(ms->l2seg_handle), acl);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to del ref from l2seg ret {}", ret);
            goto end;
        }
    }

    if (as->redirect_if_handle != HAL_HANDLE_INVALID) {
        ret = if_del_acl(find_if_by_handle(as->redirect_if_handle), acl,
                         IF_ACL_REF_TYPE_REDIRECT);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to del ref to redirect if ret {}", ret);
            goto end;
        }
    }

    if (as->copp_handle != HAL_HANDLE_INVALID) {
        ret = copp_del_acl(find_copp_by_handle(as->copp_handle), acl);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to del ref to copp ret {}", ret);
            goto end;
        }
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// PD Call to allocate PD resources and HW programming
//------------------------------------------------------------------------------
hal_ret_t
acl_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t         ret = HAL_RET_OK;
    pd::pd_acl_create_args_t pd_acl_args = { 0 };
    dllist_ctxt_t     *lnode = NULL;
    dhl_entry_t       *dhl_entry = NULL;
    acl_t             *acl = NULL;
    pd::pd_func_args_t  pd_func_args = {0};

    SDK_ASSERT(cfg_ctxt != NULL);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    acl = (acl_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("create add cb {}",
                    acl->key);

    // PD Call to allocate PD resources and HW programming
    pd::pd_acl_create_args_init(&pd_acl_args);
    pd_acl_args.acl = acl;
    pd_func_args.pd_acl_create = &pd_acl_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_ACL_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to create acl pd, err : {}",
                      ret);
    }

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

    SDK_ASSERT(cfg_ctxt != NULL);

    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    acl = (acl_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("create commit cb {}",
                    acl->key);

    // Add to DB
    ret = acl_add_to_db (acl, hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to add acl:{} to DB",
                      acl->key);
        goto end;
    }

    HAL_TRACE_DEBUG("added acl:{} to DB",
                    acl->key);

    ret = acl_add_refs(acl);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add references for acl {} ret {}",
                      acl->key, ret);
        goto end;
    }

end:
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("commit cbs can't fail: ret : {}",
                      ret);
        SDK_ASSERT(0);
    }
    return ret;
}

//------------------------------------------------------------------------------
// helper function to cleanup all the acl related state during abort operation
// when create failed
//------------------------------------------------------------------------------
static hal_ret_t
acl_create_abort_cleanup (acl_t *acl, hal_handle_t hal_handle)
{
    hal_ret_t                   ret;
    pd::pd_acl_delete_args_t    pd_acl_args = { 0 };
    pd::pd_func_args_t          pd_func_args = {0};

    // 1. delete call to PD
    if (acl->pd) {
        pd::pd_acl_delete_args_init(&pd_acl_args);
        pd_acl_args.acl = acl;
        pd_func_args.pd_acl_delete = &pd_acl_args;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_ACL_DELETE, &pd_func_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to delete acl {} pd, err : {}", acl->key, ret);
        }
    }

    // 2. remove object from hal_handle id based hash table in infra
    hal_handle_free(hal_handle);

    // 3. free acl
    acl_free(acl, false);

    return HAL_RET_OK;
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
    hal_ret_t     ret = HAL_RET_OK;
    dllist_ctxt_t *lnode = NULL;
    dhl_entry_t   *dhl_entry = NULL;
    acl_t         *acl = NULL;
    hal_handle_t  hal_handle = 0;

    SDK_ASSERT(cfg_ctxt != NULL);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    acl = (acl_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("Acl {} create abort cb", acl->key);
    ret = acl_create_abort_cleanup(acl, hal_handle);

    return ret;
}

// ----------------------------------------------------------------------------
// Dummy create cleanup callback
// ----------------------------------------------------------------------------
hal_ret_t
acl_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
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
        HAL_TRACE_ERR("ACL id not provided");
        return HAL_RET_INVALID_ARG;
    }

    if (spec.key_or_handle().key_or_handle_case() !=
        kh::AclKeyHandle::kAclId) {
        HAL_TRACE_ERR("ACL id not provided");
        return HAL_RET_INVALID_ARG;
    }

    acl_id = spec.key_or_handle().acl_id();
    acl = find_acl_by_id(acl_id);
    if (acl) {
        HAL_TRACE_ERR("ACL with id {} already created",
                      acl_id);
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
    } else if ((uint16_t)(~mask & (~mask+1))) {
        // Mask should have all bits together in the MSB
        HAL_TRACE_ERR("Not a valid port mask {:#x}",
                      mask);
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
                HAL_TRACE_ERR("UDP sport range {} to {} cannot be supported"
                              " Only maskable ranges are supported",
                              port_start, port_end);
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
                HAL_TRACE_ERR("UDP dport range {} to {} cannot be supported"
                              " Only maskable ranges are supported",
                              port_start, port_end);
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
                HAL_TRACE_ERR("TCP sport range {} to {} cannot be supported"
                              " Only maskable ranges are supported",
                              port_start, port_end);
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
                HAL_TRACE_ERR("TCP dport range {} to {} cannot be supported"
                              " Only maskable ranges are supported",
                              port_start, port_end);
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

        tcp_flags_mask = ip_mask->u.tcp.tcp_flags;
        tcp_flags = ip_key->u.tcp.tcp_flags;
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

static hal_ret_t
extract_match_spec (acl_match_spec_t *ms,
                    const acl::AclSelector &sel)
{
    hal_ret_t            ret = HAL_RET_OK;
    vrf_t                *vrf = NULL;
    if_t                 *src_if = NULL;
    if_t                 *dest_if = NULL;
    l2seg_t              *l2seg = NULL;
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
        src_if = if_lookup_key_or_handle(sel.src_if_key_handle());

        if(src_if == NULL) {
            HAL_TRACE_ERR("Source interface not found");
            ret = HAL_RET_IF_NOT_FOUND;
            goto end;
        } else {
            ms->src_if_handle = src_if->hal_handle;
        }
    }

    if (sel.has_dst_if_key_handle()) {
        ms->dest_if_match = true;
        dest_if = if_lookup_key_or_handle(sel.dst_if_key_handle());

        if(dest_if == NULL) {
            HAL_TRACE_ERR("Destination interface not found");
            ret = HAL_RET_IF_NOT_FOUND;
            goto end;
        } else {
            ms->dest_if_handle = dest_if->hal_handle;
        }
    }

    if (sel.seg_selector_case() == acl::AclSelector::kVrfKeyHandle) {
        ms->vrf_match = true;
        vrf = vrf_lookup_key_or_handle(sel.vrf_key_handle());

        if(vrf == NULL) {
            HAL_TRACE_ERR("Vrf not found");
            ret = HAL_RET_VRF_NOT_FOUND;
            goto end;
        } else {
            ms->vrf_handle = vrf->hal_handle;
        }
    } else if (sel.seg_selector_case() == acl::AclSelector::kL2SegmentKeyHandle) {
        ms->l2seg_match = true;
        l2seg = l2seg_lookup_key_or_handle(sel.l2segment_key_handle());

        if(l2seg == NULL) {
            HAL_TRACE_ERR("L2 segment not found");
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
        HAL_TRACE_ERR("ACL Type invalid");
        ret = HAL_RET_INVALID_ARG;
        break;
    }

    if (ret != HAL_RET_OK) {
        goto end;
    }

    ms->int_key.ep_learn_en = sel.internal_key().ep_learn_en();    
    ms->int_mask.ep_learn_en = sel.internal_mask().ep_learn_en();    

#ifdef ACL_DOL_TEST_ONLY
    // Key of internal fields for use only with DOL/testing infra
    // For production builds this needs to be removed
    // TODO: REMOVE
    ms->int_key.direction = sel.internal_key().direction();
    ms->int_mask.direction = sel.internal_mask().direction();
    ms->int_key.from_cpu = sel.internal_key().from_cpu();
    ms->int_mask.from_cpu = sel.internal_mask().from_cpu();
    ms->int_key.flow_miss = sel.internal_key().flow_miss();
    ms->int_mask.flow_miss = sel.internal_mask().flow_miss();
    ms->int_key.ip_options = sel.internal_key().ip_options();
    ms->int_mask.ip_options = sel.internal_mask().ip_options();
    ms->int_key.ip_frag = sel.internal_key().ip_frag();
    ms->int_mask.ip_frag = sel.internal_mask().ip_frag();
    ms->int_key.no_drop = sel.internal_key().no_drop();
    ms->int_mask.no_drop = sel.internal_mask().no_drop();
    drop_reason_spec_to_codes(sel.internal_key().drop_reasons(), &ms->int_key.drop_reasons);
    drop_reason_spec_to_codes(sel.internal_mask().drop_reasons(), &ms->int_mask.drop_reasons);
    MAC_UINT64_TO_ADDR(ms->int_key.outer_mac_da, sel.internal_key().outer_dst_mac());
    MAC_UINT64_TO_ADDR(ms->int_mask.outer_mac_da, sel.internal_mask().outer_dst_mac());
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
        sel->mutable_src_if_key_handle()->set_interface_id(find_if_id_from_hal_handle(ms->src_if_handle));
    }
    if (ms->dest_if_match) {
        sel->mutable_dst_if_key_handle()->set_interface_id(find_if_id_from_hal_handle(ms->dest_if_handle));
    }

    if (ms->vrf_match) {
        sel->mutable_vrf_key_handle()->set_vrf_id(((vrf_t *)hal_handle_get_obj(ms->vrf_handle))->vrf_id);
    }

    if (ms->l2seg_match) {
        sel->mutable_l2segment_key_handle()->set_segment_id(((l2seg_t *)hal_handle_get_obj(ms->l2seg_handle))->seg_id);
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
        uint32_t id = ainfo.ing_mirror_sessions(i).mirrorsession_id();
        if (id > 7) {
            return HAL_RET_INVALID_ARG;
        }
        *ingress = *ingress | (1 << id);
        HAL_TRACE_DEBUG("  Adding ingress session {}", id);
    }
    for (i = 0; i < ainfo.egr_mirror_sessions_size(); ++i) {
        uint32_t id = ainfo.egr_mirror_sessions(i).mirrorsession_id();
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
    uint8_t      ingress, egress;
    bool         copp_needed = false;
    copp_t       *copp = NULL;
#ifdef ACL_DOL_TEST_ONLY
    // Internal fields for use only with DOL/testing infra
    // For production builds this needs to be removed
    // TODO: REMOVE
    hal::pd::pd_rw_entry_args_t rw_key{};
#endif

    as->action = ainfo.action();
    ret = extract_mirror_sessions(ainfo, &ingress, &egress);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error extracting mirror sessions");
        goto end;
    }
    as->ing_mirror_en = ingress ? true : false;
    as->ing_mirror_session = ingress;
    as->egr_mirror_en = egress ? true : false;
    as->egr_mirror_session = egress;
    as->egress_drop = ainfo.egress_drop();

    if (ainfo.has_redirect_if_key_handle()) {
        if (as->action != acl::ACL_ACTION_REDIRECT) {
            HAL_TRACE_ERR("Redirect interface specified with action {} ",
                          as->action);
            ret = HAL_RET_INVALID_ARG;
            goto end;
        }

        redirect_if = if_lookup_key_or_handle(ainfo.redirect_if_key_handle());

        if(redirect_if == NULL) {
            HAL_TRACE_ERR("Redirect interface not found");
            ret = HAL_RET_IF_NOT_FOUND;
            goto end;
        } else {
            as->redirect_if_handle = redirect_if->hal_handle;
        }
    } else if (as->action == acl::ACL_ACTION_REDIRECT) {
        HAL_TRACE_ERR("Redirect interface not specified ");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    if ((as->action == acl::ACL_ACTION_LOG) ||
        ((as->action == acl::ACL_ACTION_REDIRECT) &&
         (redirect_if->if_type == intf::IF_TYPE_CPU))) {
        copp_needed = true;
    }

    if (ainfo.has_copp_key_handle()) {
        if (!copp_needed) {
            HAL_TRACE_ERR("Copp specified with non cpu redirect/log");
            ret = HAL_RET_INVALID_ARG;
            goto end;
        }
        copp = find_copp_by_key_handle(ainfo.copp_key_handle());
        if (!copp) {
            HAL_TRACE_ERR("Copp not found");
            ret = HAL_RET_COPP_NOT_FOUND;
            goto end;
        }
        as->copp_handle = copp->hal_handle;
    } else {
        if (copp_needed) {
            HAL_TRACE_ERR("Copp not specified with cpu redirect/log");
            ret = HAL_RET_INVALID_ARG;
            goto end;
        }
        as->copp_handle = HAL_HANDLE_INVALID;
    }


#ifdef ACL_DOL_TEST_ONLY
    // Internal fields for use only with DOL/testing infra
    // For production builds this needs to be removed
    // TODO: REMOVE
    if (ainfo.has_internal_actions()) {
        if ((as->action == acl::ACL_ACTION_DENY) &&
            (ainfo.internal_actions().has_drop_reason())) {
            drop_reason_spec_to_codes(ainfo.internal_actions().drop_reason(), &as->int_as.drop_reason);
        } else {
            if (as->action != acl::ACL_ACTION_REDIRECT) {
                HAL_TRACE_ERR("Redirect action fields specified for "
                        "non-redirect action {}",
                        as->action);
                ret = HAL_RET_INVALID_ARG;
                goto end;
            }
            as->int_as.mac_sa_rewrite = ainfo.internal_actions().mac_sa_rewrite_en();
            as->int_as.mac_da_rewrite = ainfo.internal_actions().mac_da_rewrite_en();
            as->int_as.ttl_dec = ainfo.internal_actions().ttl_dec_en();
            as->int_as.qid = ainfo.internal_actions().qid();
            as->int_as.qid_en = ainfo.internal_actions().qid_valid();

            if (ainfo.internal_actions().has_encap_info()) {
                as->int_as.tnnl_vnid = ainfo.internal_actions().encap_info().encap_value();
            }

            if (as->int_as.mac_sa_rewrite) {
                MAC_UINT64_TO_ADDR(rw_key.mac_sa, ainfo.internal_actions().mac_sa());
            }

            if (as->int_as.mac_da_rewrite) {
                MAC_UINT64_TO_ADDR(rw_key.mac_da, ainfo.internal_actions().mac_da());
            }

            pd::pd_rw_entry_find_or_alloc_args_t r_args;
            pd::pd_func_args_t pd_func_args = {0};
            rw_key.rw_act = REWRITE_REWRITE_ID;
            r_args.args = &rw_key;
            r_args.rw_idx = &as->int_as.rw_idx;
            pd_func_args.pd_rw_entry_find_or_alloc = &r_args;
            ret = pd::hal_pd_call(pd::PD_FUNC_ID_RWENTRY_FIND_OR_ALLOC, &pd_func_args);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Unable to find/alloc rw entry");
                goto end;
            }
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
    if (as->copp_handle != HAL_HANDLE_INVALID) {
        ainfo->mutable_copp_key_handle()->set_copp_handle(as->copp_handle);
    }
    if (as->redirect_if_handle != HAL_HANDLE_INVALID) {
        ainfo->mutable_redirect_if_key_handle()->set_interface_id(find_if_id_from_hal_handle(as->redirect_if_handle));
    }
#ifdef ACL_DOL_TEST_ONLY
    if (as->redirect_if_handle != HAL_HANDLE_INVALID) {
        // Internal fields for use only with DOL/testing infra
        // For production builds this needs to be removed
        ainfo->mutable_internal_actions()->set_mac_sa_rewrite_en(as->int_as.mac_sa_rewrite);
        ainfo->mutable_internal_actions()->set_mac_da_rewrite_en(as->int_as.mac_da_rewrite);
        ainfo->mutable_internal_actions()->set_ttl_dec_en(as->int_as.ttl_dec);
        ainfo->mutable_internal_actions()->set_qid(as->int_as.qid);
        ainfo->mutable_internal_actions()->set_qid_valid(as->int_as.qid_en);
        ainfo->mutable_internal_actions()->mutable_encap_info()->set_encap_value(as->int_as.tnnl_vnid);
        // TODO rewrite actions
    }
#endif
    return HAL_RET_OK;
}

static hal_ret_t
acl_init_from_spec (acl_t *acl, const AclSpec& spec)
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
        HAL_TRACE_ERR("Acl create request match parsing failed."
                      " Err: {}",
                      ret);
        goto end;
    }

    ret = extract_action_spec(&acl->action_spec, spec.action());
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Acl create request action parsing failed."
                      " Err: {}",
                      ret);
        goto end;
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// initialize a acl's oper status from its status object
//------------------------------------------------------------------------------
static hal_ret_t
acl_init_from_status (acl_t *acl, const AclStatus& status)
{
    acl->hal_handle = status.acl_handle();
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// initialize a acl's oper stats from its stats object
//------------------------------------------------------------------------------
static hal_ret_t
acl_init_from_stats (acl_t *acl, const AclStats& stats)
{
    // Only PD ACL has stats
    return HAL_RET_OK;
}

// Function to fill AclGetResponse
static hal_ret_t
acl_process_get (acl_t *acl, AclGetResponse *rsp)
{
    AclSpec               *spec;
    hal_ret_t             ret = HAL_RET_OK;
    pd::pd_acl_get_args_t args   = {0};
    pd::pd_func_args_t    pd_func_args = {0};

    // fill config spec of this acl
    spec = rsp->mutable_spec();
    spec->mutable_key_or_handle()->set_acl_id(acl->key.acl_id);
    spec->set_priority(acl->priority);

    ret = populate_match_spec(&acl->match_spec, spec->mutable_match());
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error populating match spec for acl {} ret {}",
                      acl->key, ret);
    }

    ret = populate_action_spec(&acl->action_spec, spec->mutable_action());
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error populating action spec for acl {} ret {}",
                      acl->key, ret);
    }

    // fill operational state of this acl
    rsp->mutable_status()->set_acl_handle(acl->hal_handle);

    // fill stats of this acl - intialize with 0, PD will set it
    rsp->mutable_stats()->set_num_packets(0);

    rsp->set_api_status(types::API_STATUS_OK);

    // Getting PD information
    args.acl = acl;
    args.rsp = rsp;
    pd_func_args.pd_acl_get = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_ACL_GET, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to do PD get for acl : {}. ret : {}",
                      acl->key, ret);
    }

    return ret;
}

// Callback function to retrieve ACLs
static bool
acl_get_ht_cb(void *ht_entry, void *ctxt)
{
    hal_handle_id_ht_entry_t *entry     = (hal_handle_id_ht_entry_t *)ht_entry;
    AclGetResponseMsg        *rsp       = (AclGetResponseMsg *)ctxt;
    AclGetResponse           *response  = rsp->add_response();
    acl_t                    *acl    = NULL;

    acl = (acl_t *)find_acl_by_handle(entry->handle_id);
    acl_process_get(acl, response);

    // Always return false here, so that we walk through all hash table
    // entries.
    return false;
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

    HAL_TRACE_DEBUG("Rcvd acl create for acl-id {} ",
                    spec.key_or_handle().acl_id());

    // dump spec
    acl_spec_dump(spec);

    ret = validate_acl_create(spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Validation failed ret {}", ret);
        goto end;
    }

    // instantiate acl
    acl = acl_alloc_init();
    if (acl == NULL) {
        HAL_TRACE_ERR("unable to allocate handle/memory ret: {}",
                      ret);
        ret = HAL_RET_OOM;
        goto end;
    }

    // initialize the acl record
    // populate from the spec
    ret = acl_init_from_spec(acl, spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("error in populating acl from spec");
        goto end;
    }

    // allocate hal handle id
    acl->hal_handle = hal_handle_alloc(HAL_OBJ_ID_ACL);
    if (acl->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("failed to alloc handle");
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
            // PD wouldn't have been allocated if we're coming here
            // PD gets allocated in create_add_cb and if it failed,
            // create_abort_cb would free everything
            acl_free(acl, true);
            acl = NULL;
        }
        HAL_API_STATS_INC (HAL_API_ACL_CREATE_FAIL);
    } else {
        HAL_API_STATS_INC (HAL_API_ACL_CREATE_SUCCESS);
    }

    acl_prepare_rsp(rsp, ret, acl ? acl->hal_handle : HAL_HANDLE_INVALID);
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
        HAL_TRACE_ERR("spec has no key or handle");
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
    pd::pd_acl_update_args_t     pd_acl_args = { 0 };
    dllist_ctxt_t         *lnode = NULL;
    dhl_entry_t           *dhl_entry = NULL;
    acl_t                 *acl_clone = NULL;
    pd::pd_func_args_t    pd_func_args = {0};

    SDK_ASSERT(cfg_ctxt != NULL);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    acl_clone = (acl_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update upd cb {}",
                    acl_clone->key);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_acl_update_args_init(&pd_acl_args);
    pd_acl_args.acl = acl_clone;
    pd_func_args.pd_acl_update = &pd_acl_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_ACL_UPDATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to update acl pd, err : {}",
                      ret);
    }

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
    pd::pd_acl_make_clone_args_t args;
    acl_t *acl_clone;
    pd::pd_func_args_t pd_func_args = {0};

    *acl_clone_p = acl_alloc_init();
    acl_clone = *acl_clone_p;

    acl_clone->key = acl->key;
    acl_clone->hal_handle = acl->hal_handle;
    acl_clone->pd = NULL;

    args.acl = acl;
    args.clone = *acl_clone_p;
    pd_func_args.pd_acl_make_clone = &args;
    pd::hal_pd_call(pd::PD_FUNC_ID_ACL_MAKE_CLONE, &pd_func_args);

    // Update with the new spec
    ret = acl_init_from_spec(acl_clone, spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("error in populating acl from spec");
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
acl_update_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t             ret = HAL_RET_OK;
    dllist_ctxt_t         *lnode = NULL;
    dhl_entry_t           *dhl_entry = NULL;
    acl_t                 *acl = NULL;
    acl_t                 *acl_clone = NULL;

    SDK_ASSERT(cfg_ctxt != NULL);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    acl = (acl_t *)dhl_entry->obj;
    acl_clone = (acl_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update commit cb {}",
                    acl->key);

    ret = acl_rem_refs(acl);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to remove references for acl {} ret {}",
                      acl->key, ret);
        goto end;
    }

    ret = acl_add_refs(acl_clone);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add references for acl_clone {} ret {}",
                      acl_clone->key, ret);
        goto end;
    }

    // Free PI.
    acl_free(acl, true);
end:
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("commit cbs can't fail: ret : {}",
                      ret);
        SDK_ASSERT(0);
    }
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

    SDK_ASSERT(cfg_ctxt != NULL);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    acl_clone = (acl_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update abort cb {}",
                    acl_clone->key);

    // Free Clone
    acl_free(acl_clone, true);

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

    HAL_TRACE_DEBUG("Rcvd ACL update");

    // dump spec
    acl_spec_dump(spec);

    // validate the request message
    ret = validate_acl_update(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("acl delete validation failed, ret : {}",
                      ret);
        goto end;
    }

    acl = acl_lookup_by_key_or_handle(kh);
    if (acl == NULL) {
        HAL_TRACE_ERR("failed to find acl, id {}, handle {}",
                      kh.acl_id(), kh.acl_handle());
        ret = HAL_RET_ACL_NOT_FOUND;
        goto end;
    }
    HAL_TRACE_DEBUG("update acl {}", acl->key);

    ret = acl_make_clone(acl, (acl_t **)&dhl_entry.cloned_obj, spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error making acl clone acl {} ret {}", acl->key, ret);
        goto end;
    }

    acl_clone = (acl_t *)dhl_entry.cloned_obj;

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
        HAL_API_STATS_INC (HAL_API_ACL_UPDATE_FAIL);
    } else {
        HAL_API_STATS_INC (HAL_API_ACL_UPDATE_SUCCESS);
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
acl_get (AclGetRequest& req, AclGetResponseMsg *rsp)
{
    acl_t          *acl;
    AclGetResponse *response;

    hal_api_trace(" API Begin: acl get ");
    // key-handle field must be set
    if (!req.has_key_or_handle()) {
        g_hal_state->acl_ht()->walk(acl_get_ht_cb, rsp);
        hal_api_trace(" API End: acl get ");
        HAL_API_STATS_INC (HAL_API_ACL_GET_SUCCESS);
        return HAL_RET_OK;
    }

    response = rsp->add_response();

    acl = acl_lookup_by_key_or_handle(req.key_or_handle());
    if (acl == NULL) {
        response->set_api_status(types::API_STATUS_NOT_FOUND);
        HAL_API_STATS_INC (HAL_API_ACL_GET_FAIL);
        return HAL_RET_ACL_NOT_FOUND;
    }

    acl_process_get(acl, response);

    hal_api_trace(" API End: acl get ");
    HAL_API_STATS_INC (HAL_API_ACL_GET_SUCCESS);
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
        HAL_TRACE_ERR("spec has no key or handle");
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
    pd::pd_acl_delete_args_t           pd_acl_args = { 0 };
    dllist_ctxt_t               *lnode      = NULL;
    dhl_entry_t                 *dhl_entry  = NULL;
    acl_t                       *acl        = NULL;
    pd::pd_func_args_t          pd_func_args = {0};

    SDK_ASSERT(cfg_ctxt != NULL);

    // TODO: Check the dependency ref count for the acl.
    //       If its non zero, fail the delete.


    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    acl = (acl_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("delete del cb {} handle {}",
                    acl->key, acl->hal_handle);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_acl_delete_args_init(&pd_acl_args);
    pd_acl_args.acl = acl;
    pd_func_args.pd_acl_delete = &pd_acl_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_ACL_DELETE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to delete acl pd, err : {}",
                      ret);
    }

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

    SDK_ASSERT(cfg_ctxt != NULL);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    acl = (acl_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("delete commit cb {} handle {}",
                    acl->key, acl->hal_handle);

    ret = acl_rem_refs(acl);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to remove references for acl {} ret {}",
                      acl->key, ret);
        goto end;
    }

    // a. Remove from acl id hash table
    ret = acl_del_from_db(acl);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to del acl {} from db, err : {}",
                      acl->key, ret);
        goto end;
    }

    // b. Remove object from handle id based hash table
    hal_handle_free(hal_handle);

    // c. Free PI acl
    acl_free(acl, false);

end:

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("commit cbs can't fail: ret : {}",
                      ret);
        SDK_ASSERT(0);
    }
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
        HAL_TRACE_ERR("acl delete validation failed, ret : {}",
                      ret);
        goto end;
    }

    acl = acl_lookup_by_key_or_handle(kh);
    if (acl == NULL) {
        HAL_TRACE_ERR("failed to find acl, id {}, handle {}",
                      kh.acl_id(), kh.acl_handle());
        ret = HAL_RET_ACL_NOT_FOUND;
        goto end;
    }

    HAL_TRACE_DEBUG("deleting acl {} handle {}",
                    acl->key, acl->hal_handle);

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

    if (ret == HAL_RET_OK) {
        HAL_API_STATS_INC (HAL_API_ACL_DELETE_SUCCESS);
    } else {
        HAL_API_STATS_INC (HAL_API_ACL_DELETE_FAIL);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));
    hal_api_trace(" API End: acl delete ");
    return ret;
}

//-----------------------------------------------------------------------------
// given a acl, marshall it for persisting the acl state (spec, status, stats)
//
// obj points to acl object i.e., acl_t
// mem is the memory buffer to serialize the state into
// len is the length of the buffer provided
// mlen is to be filled by this function with marshalled state length
//-----------------------------------------------------------------------------
hal_ret_t
acl_store_cb (void *obj, uint8_t *mem, uint32_t len, uint32_t *mlen)
{
    AclGetResponse acl_info;
    uint32_t       serialized_state_sz;
    acl_t          *acl = (acl_t *)obj;

    SDK_ASSERT((acl != NULL) && (mlen != NULL));
    *mlen = 0;

    // get all information about this acl (includes spec, status & stats)
    acl_process_get(acl, &acl_info);
    serialized_state_sz = acl_info.ByteSizeLong();
    if (serialized_state_sz > len) {
        HAL_TRACE_ERR("Failed to marshall ACL {}, not enough room, "
                      "required size {}, available size {}",
                      acl->key, serialized_state_sz, len);
        return HAL_RET_OOM;
    }

    // serialize all the state
    if (acl_info.SerializeToArray(mem, serialized_state_sz) == false) {
        HAL_TRACE_ERR("Failed to serialize acl {}", acl->key);
        return HAL_RET_OOM;
    }
    *mlen = serialized_state_sz;
    HAL_TRACE_DEBUG("Marshalled acl {}, len {}",
                    acl->key, serialized_state_sz);
    return HAL_RET_OK;
}

static hal_ret_t
acl_restore_add (acl_t *acl, const AclGetResponse& acl_info)
{
    hal_ret_t                 ret;
    pd::pd_acl_restore_args_t pd_acl_args = { 0 };
    pd::pd_func_args_t        pd_func_args = {0};

    // restore pd state
    pd::pd_acl_restore_args_init(&pd_acl_args);
    pd_acl_args.acl = acl;
    pd_acl_args.acl_status = &acl_info.status();
    pd_func_args.pd_acl_restore = &pd_acl_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_ACL_RESTORE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to restore acl {} pd, err : {}",
                      acl->key, ret);
    }
    return ret;
}

static hal_ret_t
acl_restore_commit (acl_t *acl, const AclGetResponse& acl_info)
{
    hal_ret_t          ret;

    HAL_TRACE_DEBUG("Committing acl {} restore", acl->key);

    ret = acl_add_to_db(acl, acl->hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to restore acl {} to db, err : {}",
                      acl->key, ret);
    }
    return ret;
}

static hal_ret_t
acl_restore_abort (acl_t *acl, const AclGetResponse& acl_info)
{
    HAL_TRACE_ERR("Aborting acl {} restore", acl->key);
    acl_create_abort_cleanup(acl, acl->hal_handle);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// acl's restore cb.
//  - restores acl to the PI and PD state before the upgrade
//------------------------------------------------------------------------------
uint32_t
acl_restore_cb (void *obj, uint32_t len)
{
    hal_ret_t      ret;
    AclGetResponse acl_info;
    acl_t          *acl;

    // de-serialize the object
    if (acl_info.ParseFromArray(obj, len) == false) {
        HAL_TRACE_ERR("Failed to de-serialize a serialized acl obj");
        SDK_ASSERT(0);
        return 0;
    }

    // allocate ACL obj from slab
    acl = acl_alloc_init();
    if (acl == NULL) {
        HAL_TRACE_ERR("Failed to alloc/init acl");
        return 0;
    }

    // initialize acl attrs from its spec
    ret = acl_init_from_spec(acl, acl_info.spec());
    if (ret != HAL_RET_OK) {
        goto end;
    }
    acl_init_from_status(acl, acl_info.status());
    acl_init_from_stats(acl, acl_info.stats());

    // repopulate handle db
    hal_handle_insert(HAL_OBJ_ID_ACL, acl->hal_handle, (void *)acl);

    ret = acl_restore_add(acl, acl_info);
    if (ret != HAL_RET_OK) {
        acl_restore_abort(acl, acl_info);
        acl = NULL;
        goto end;
    }
    acl_restore_commit(acl, acl_info);
end:
    if (ret != HAL_RET_OK) {
        if (acl) {
            // PD wouldn't have been allocated if we're coming here
            // PD gets allocated in restore_add and if it failed,
            // restore_abort would free everything
            acl_free(acl, true);
            acl = NULL;
        }
    }
    return 0;    // TODO: fix me
}

}    // namespace hal
