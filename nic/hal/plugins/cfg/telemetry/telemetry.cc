//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "gen/proto/telemetry.pb.h"
#include "gen/proto/types.pb.h"
#include "nic/hal/plugins/cfg/telemetry/telemetry.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "nic/include/fte.hpp"
#include "nic/hal/src/utils/if_utils.hpp"

#define TELEMETRY_FREE_RSP_RET_TRACE(free_fn, obj, rsp, ret, args...) \
    {                                                                 \
        if (likely(obj != NULL)) {                                    \
            free_fn(obj);                                             \
        }                                                             \
        rsp->set_api_status(hal_prepare_rsp(ret));                    \
        HAL_TRACE_ERR(args);                                          \
        return ret;                                                   \
    }                                                                 \


using hal::pd::pd_mirror_session_create_args_t;
using hal::pd::pd_mirror_session_update_args_t;
using hal::pd::pd_mirror_session_delete_args_t;
using hal::pd::pd_mirror_session_get_hw_id_args_t;
using hal::mirror_session_t;
using hal::pd::pd_flow_monitor_rule_create_args_t;
using hal::pd::pd_flow_monitor_rule_delete_args_t;
using hal::pd::pd_flow_monitor_rule_get_args_t;
using hal::flow_monitor_rule_t;
using hal::pd::pd_drop_monitor_rule_create_args_t;
using hal::pd::pd_drop_monitor_rule_delete_args_t;
using hal::pd::pd_drop_monitor_rule_get_args_t;
using hal::drop_monitor_rule_t;

namespace hal {

// Global structs
int telemetry_collector_id_db[HAL_MAX_TELEMETRY_COLLECTORS] = {-1};
int flow_monitor_rule_id_db[MAX_FLOW_MONITOR_RULES] = {-1};
acl::acl_config_t   flowmon_rule_config_glbl = { };

void *
mirror_session_get_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((mirror_session_t *)entry)->sw_id);
}

uint32_t
mirror_session_key_size(void)
{
    return sizeof(mirror_session_id_t);
}

void *
flowmon_rules_get_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((flow_monitor_rule_t *)entry)->rule_id);
}

uint32_t
flowmon_rules_key_size ()
{
    return sizeof(uint32_t);
}

hal_ret_t
telemetry_eval_sessions (void)
{
    hal_ret_t ret = HAL_RET_OK;
    hal::cfg_op_t op;

    op = hal_cfg_db_get_mode();
    // Close the config db to avoid any deadlocks with FTE
    hal::hal_cfg_db_close();

    auto walk_func = [](void *entry, void *ctxt) {
        hal::session_t  *session = (session_t *)entry;
        dllist_ctxt_t   *list_head = (dllist_ctxt_t *) ctxt;

        if (!session->is_ipfix_flow) {
            hal_handle_id_list_entry_t *list_entry = (hal_handle_id_list_entry_t *)g_hal_state->
                    hal_handle_id_list_entry_slab()->alloc();
            if (list_entry == NULL) {
                HAL_TRACE_ERR("Out of memory - skipping delete session {}", session->hal_handle);
                return false;
            }

            list_entry->handle_id = session->hal_handle;
            dllist_add(list_head, &list_entry->dllist_ctxt);
        }
        return false;
    };

    // build list of session_ids
    dllist_ctxt_t session_list;
    dllist_reset(&session_list);

    HAL_TRACE_DEBUG("Calling walk func");
    g_hal_state->session_hal_handle_ht()->walk_safe(walk_func, &session_list);

    HAL_TRACE_DEBUG("Update sessions, count {}", dllist_count(&session_list));
    ret = session_update_list(&session_list, true, 
                              (1 << fte::feature_id("pensando.io/telemetry:telemetry")));
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Session update list failed {}", ret);
    }

    // Re-open the db
    hal_cfg_db_open(op);
    return ret;
}

bool
telemetry_is_export_configured (void)
{
    for (int i = 0; i < HAL_MAX_TELEMETRY_COLLECTORS; i++) {
        if (telemetry_collector_id_db[i] != -1) {
            return true;
        }
    }
    return false;
}

static inline int
telemetry_collector_get_id (int spec_id)
{
    for (int i = 0; i < HAL_MAX_TELEMETRY_COLLECTORS; i++) {
        if (telemetry_collector_id_db[i] == spec_id) {
            return i;
        }
    }
    return -1;
}

static inline int
telemetry_flow_monitor_rule_get_id (int spec_id)
{
    for (int i = 0; i < MAX_FLOW_MONITOR_RULES; i++) {
        if (flow_monitor_rule_id_db[i] == spec_id) {
            return i;
        }
    }
    return -1;
}

hal_ret_t
mirror_session_update (MirrorSessionSpec &spec, MirrorSessionResponse *rsp)
{
    return HAL_RET_OK;
}

hal_ret_t
mirror_session_update_pd (mirror_session_t *session,
                          if_t *dest_if, if_t *tnnl_if, ep_t *rtep_ep)
{
    hal_ret_t                       ret = HAL_RET_OK;
    pd::pd_func_args_t              pd_func_args = {0};
    pd_mirror_session_update_args_t args = {0};

    args.session = session;
    args.dst_if = dest_if;
    args.rtep_ep = rtep_ep;
    args.tunnel_if = tnnl_if;
    pd_func_args.pd_mirror_session_update = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_MIRROR_SESSION_UPDATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Update failed for session id {}, ret {}",
                       session->sw_id, ret);
    } else {
        HAL_TRACE_DEBUG("Update Succeeded for session id {}", session->sw_id);
    }
    return ret;
}

#if 0
hal_ret_t
mirror_session_update_ifs (mirror_session_t *session,
                           bool dst_if_change, if_t *dest_if,
                           bool tunnel_if_change, if_t *tunnel_if)
{
    hal_ret_t                       ret = HAL_RET_OK;
    pd::pd_func_args_t              pd_func_args = {0};
    pd_mirror_session_update_args_t args = {0};

#if 0
    dest_if = find_if_by_handle(g_hal_state->inb_bond_active_uplink());
    tunnel_if = find_tnnlif_by_dst_ip(intf::IF_TUNNEL_ENCAP_TYPE_GRE,
                                      &session->mirror_destination_u.er_span_dest.ip_da);
#endif

    if (dst_if_change) {
        args.dst_if_change = true;
        args.dst_if = dest_if;
    }
    if (tunnel_if_change) {
        args.tunnel_if_change = true;
        args.tunnel_if = tunnel_if;
    }

    args.session = session;
    pd_func_args.pd_mirror_session_update = &args;
    // Update mirror session
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_MIRROR_SESSION_UPDATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Update failed for session id {}, ret {}",
                       session->sw_id, ret);
    } else {
        HAL_TRACE_DEBUG("Update Succeeded for session id {}", session->sw_id);
    }
    return ret;
}

static if_t *
telemetry_mirror_pick_dest_if (if_t *dest_if)
{
    // No change to dest_if for sim mode
    if (hal::is_platform_type_sim()) {
        return dest_if;
    }
    if (dest_if->is_oob_management) {
        // Nothing to be done for oob intf
        return dest_if;
    }

    if (dest_if->if_type == intf::IF_TYPE_UPLINK) {
        HAL_TRACE_DEBUG("Choosing active dest-if");
        if_t *new_dest_if = find_if_by_handle(g_hal_state->inb_bond_active_uplink());
        if (new_dest_if) {
            return new_dest_if;
            HAL_TRACE_DEBUG("Active bond dest-if id {}", dest_if->if_id);
        } else {
            HAL_TRACE_DEBUG("Did not find an active uplink!");
        }
    }

    return dest_if;
}
#endif

hal_ret_t
telemetry_mirror_session_handle_repin ()
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = mirror_session_change(NULL, false, NULL, false, NULL, false, NULL);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to repin mirror sessions: ret: {}", ret);
        goto end;
    }

end:
    return ret;

#if 0
    auto ms_ht = g_hal_state->mirror_session_ht();

    auto walk_func = [](void *entry, void *ctxt) {
        mirror_session_t *session = (mirror_session_t *)entry;
        SDK_ASSERT(session != NULL);
        if_t *old_if = session->dest_if;
        if_t *new_dif = telemetry_mirror_pick_dest_if(old_if);
        if (old_if == new_dif) {
            return false;
        }
        session->dest_if = new_dif;
        auto ret = mirror_session_update_ifs(session, true, new_dif,
                                             false, NULL);
        if (ret != HAL_RET_OK) {
            // we have failed to update the newuplink, restore the old dest if.
            session->dest_if = old_if;
        }
        return false;
    };

    auto sdk_ret = ms_ht->walk_safe(walk_func, NULL);
    if (sdk_ret != SDK_RET_OK) {
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        HAL_TRACE_ERR("Mirror session repin failed {}", ret);
        return ret;
    }

    return ret;
#endif
}

static if_t *
telemetry_mirror_pick_dest_if ()
{
    if_t *dst_if = NULL;

    HAL_TRACE_DEBUG("Choosing active dest-if");
    if (hal::is_platform_type_sim()) {
        dst_if = telemetry_get_active_uplink(); // Only for gtest
    } else {
        dst_if = find_if_by_handle(g_hal_state->inb_bond_active_uplink());
    }
    HAL_TRACE_DEBUG("Active bond dest-if id {}",
                    dst_if ? if_keyhandle_to_str(dst_if) : "NULL");

    return dst_if;
#if 0
    // No change to dest_if for sim mode
    if (hal::is_platform_type_sim()) {
        return dest_if;
    }
    if (dest_if->is_oob_management) {
        // Nothing to be done for oob intf
        return dest_if;
    }

    if (dest_if->if_type == intf::IF_TYPE_UPLINK) {
        HAL_TRACE_DEBUG("Choosing active dest-if");
        if_t *new_dest_if = find_if_by_handle(g_hal_state->inb_bond_active_uplink());
        if (new_dest_if) {
            dest_if = new_dest_if;
            HAL_TRACE_DEBUG("Active bond dest-if id {}", dest_if->if_id);
        } else {
            HAL_TRACE_DEBUG("Did not find an active uplink!");
        }
    }

    return dest_if;
#endif
}

hal_ret_t
mirror_session_create (MirrorSessionSpec &spec, MirrorSessionResponse *rsp)
{
    pd_mirror_session_create_args_t args;
    pd_mirror_session_delete_args_t del_args;
    pd::pd_func_args_t pd_func_args = {0};
    mirror_session_t *session = NULL;
    mirror_session_id_t sw_id;
    kh::InterfaceKeyHandle ifid;
    hal_ret_t ret;
    auto ms_ht = g_hal_state->mirror_session_ht();
    if_t *id, *dest_if;

    HAL_TRACE_INFO("Create Mirror session ID {}, snaplen {}",
                   spec.key_or_handle().mirrorsession_id(), spec.snaplen());
    mirrorsession_spec_dump(spec);

    // check if mirror session already exists
    sw_id = spec.key_or_handle().mirrorsession_id();
    session = (mirror_session_t*)ms_ht->lookup(&sw_id);
    if (session != NULL) {
        TELEMETRY_FREE_RSP_RET_TRACE(mirror_session_free, NULL, rsp, HAL_RET_ENTRY_EXISTS,
                                     "Mirror session ID {} already exist",
                                     sw_id);
    } else if (ms_ht->num_entries() >= MAX_MIRROR_SESSION_DEST) {
        TELEMETRY_FREE_RSP_RET_TRACE(mirror_session_free, NULL, rsp, HAL_RET_TABLE_FULL,
                                     "Mirror session table full. scale: {}",
                                     MAX_MIRROR_SESSION_DEST);
    } else {
        session = mirror_session_alloc();
        SDK_ASSERT(session != NULL);
    }

    // Eventually the CREATE API will differ from the Update API in the way it
    // is enabled. In a create invocation, the session is created only after all
    // the flows using a previous incarnation of the mirror session have been
    // cleanedup (i.e. mirror session removed by the periodic thread). Update is
    // treated as an incremental update.
    session->sw_id = spec.key_or_handle().mirrorsession_id();
    session->truncate_len = spec.snaplen();
    switch (spec.destination_case()) {
    case MirrorSessionSpec::kLocalSpanIf: {
        HAL_TRACE_DEBUG("Local Span IF is true");
        ifid = spec.local_span_if();
        id = if_lookup_key_or_handle(ifid);
        if (id == NULL) {
            TELEMETRY_FREE_RSP_RET_TRACE(mirror_session_free, session,
                                         rsp, HAL_RET_INVALID_ARG,
                                         "Local Span IF {} does not exist",
                                         ifid.interface_id());
        }
        session->dest_if = id;
        session->type = hal::MIRROR_DEST_LOCAL;
        break;
    }
    case MirrorSessionSpec::kRspanSpec: {
        HAL_TRACE_DEBUG("RSpan IF is true");
        auto rspan = spec.rspan_spec();
        ifid = rspan.intf();
        session->dest_if = if_lookup_key_or_handle(ifid);
        auto encap = rspan.rspan_encap();
        if (encap.encap_type() == types::ENCAP_TYPE_DOT1Q) {
            session->mirror_destination_u.r_span_dest.vlan = encap.encap_value();
        }
        session->type = hal::MIRROR_DEST_RSPAN;
        break;
    }
    case MirrorSessionSpec::kErspanSpec: {
        HAL_TRACE_DEBUG("ERSpan IF is true");
        auto erspan = spec.erspan_spec();
        session->mirror_destination_u.er_span_dest.vrf_id =
            spec.vrf_key_handle().vrf_id();
        ip_addr_t *src_addr = &session->mirror_destination_u.
            er_span_dest.ip_sa;
        ip_addr_t *dst_addr = &session->mirror_destination_u.
            er_span_dest.ip_da;
        session->mirror_destination_u.er_span_dest.ip_type = dst_addr->af;
        ip_addr_spec_to_ip_addr(src_addr, erspan.src_ip());
        ip_addr_spec_to_ip_addr(dst_addr, erspan.dest_ip());
        ep_t *ep;
        switch (dst_addr->af) {
        case IP_AF_IPV4:
            ep = find_ep_by_v4_key(spec.vrf_key_handle().vrf_id(), dst_addr->addr.v4_addr);
            break;
        case IP_AF_IPV6:
            ep = find_ep_by_v6_key(spec.vrf_key_handle().vrf_id(), dst_addr);
            break;
        default:
            TELEMETRY_FREE_RSP_RET_TRACE(mirror_session_free, session,
                                         rsp, HAL_RET_INVALID_ARG,
                                         "Unknown ERSPAN dest AF {}",
                                         dst_addr->af);
        }
#if 0
        if (ep) {
            // Known EP
            dest_if = find_if_by_handle(ep->if_handle);
            if (dest_if == NULL) {
                TELEMETRY_FREE_RSP_RET_TRACE(mirror_session_free, session,
                                             rsp, HAL_RET_INVALID_ARG,
                                             "Could not find if ERSPAN dest {}",
                                             ipaddr2str(dst_addr));
            }
            HAL_TRACE_DEBUG("Collector EP Dest IF type {}, op_status {}, id {}",
                            dest_if->if_type, dest_if->if_op_status,
                            dest_if->if_id);
        }
#endif
        dest_if = telemetry_mirror_pick_dest_if();
        auto ift = find_tnnlif_by_dst_ip(intf::IF_TUNNEL_ENCAP_TYPE_GRE,
                                         dst_addr);
        session->type = hal::MIRROR_DEST_ERSPAN;
        session->dest_if = dest_if;
        args.tunnel_if = ift;
        args.dst_if = dest_if;
        args.rtep_ep = ep;
        break;
    }
    default: {
        TELEMETRY_FREE_RSP_RET_TRACE(mirror_session_free, session,
                                     rsp, HAL_RET_INVALID_ARG,
                                     "Unknown session type{}",
                                     spec.destination_case());
    }
    }
    args.session = session;
    pd_func_args.pd_mirror_session_create = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_MIRROR_SESSION_CREATE,
                          &pd_func_args);
    if (ret != HAL_RET_OK) {
        TELEMETRY_FREE_RSP_RET_TRACE(mirror_session_free, session,
                                     rsp, ret, "Create failed {}", ret);
    } else {
        HAL_TRACE_INFO("Create Succeeded {}", session->sw_id);
    }

    session->mirror_session_ht_ctxt.reset();
    auto sdk_ret = ms_ht->insert(session, &session->mirror_session_ht_ctxt);
    if (sdk_ret != SDK_RET_OK) {
        // delete the mirror session
        del_args.session = session;
        pd_func_args.pd_mirror_session_delete = &del_args;
        pd::hal_pd_call(pd::PD_FUNC_ID_MIRROR_SESSION_DELETE, &pd_func_args);
        TELEMETRY_FREE_RSP_RET_TRACE(mirror_session_free, session,
                                     rsp, HAL_RET_ERR, "Mirror session ht insert failed {}",
                                     HAL_RET_ERR);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));
    rsp->mutable_status()->set_handle(args.hw_id);
    return ret;
}

static hal_ret_t
mirror_session_get_hw_id_helper (mirror_session_t *session,
                                 mirror_session_id_t *hw_id)
{
    pd_mirror_session_get_hw_id_args_t args;
    pd::pd_func_args_t pd_func_args = {0};

    pd_mirror_session_get_hw_id_args_init(&args);
    args.session = session;
    pd_func_args.pd_mirror_session_get_hw_id = &args;
    auto ret =   pd::hal_pd_call(pd::PD_FUNC_ID_MIRROR_SESSION_GET_HW_ID,
                                 &pd_func_args);
    *hw_id = args.hw_id;
    return ret;
}

static bool
mirror_session_fill_rsp (void *entry, void *ctxt)
{
    if (unlikely(entry == NULL || ctxt == NULL)) {
        HAL_TRACE_ERR("Invalid argument to mirror session fill");
        return false;
    }

    mirror_session_t *session = (mirror_session_t *)entry;
    MirrorSessionGetResponseMsg *rsp = (MirrorSessionGetResponseMsg *)ctxt;
    mirror_session_id_t hw_id;

    auto response = rsp->add_response();
    response->set_api_status(types::API_STATUS_OK);
    response->mutable_spec()->mutable_key_or_handle()->\
        set_mirrorsession_id(session->sw_id);
    auto ret = mirror_session_get_hw_id_helper(session, &hw_id);
    SDK_ASSERT(ret == HAL_RET_OK);
    response->mutable_status()->set_handle(hw_id);
    response->mutable_spec()->set_snaplen(session->truncate_len);

    switch (session->type) {
    case hal::MIRROR_DEST_ERSPAN:
        response->mutable_spec()->mutable_erspan_spec()->mutable_src_ip()->\
            set_v4_addr(session->mirror_destination_u.er_span_dest.ip_sa.addr.v4_addr);
        response->mutable_spec()->mutable_erspan_spec()->mutable_src_ip()->\
            set_ip_af(types::IPAddressFamily::IP_AF_INET);

        response->mutable_spec()->mutable_erspan_spec()->mutable_dest_ip()->\
            set_v4_addr(session->mirror_destination_u.er_span_dest.ip_da.addr.v4_addr);
        response->mutable_spec()->mutable_erspan_spec()->mutable_dest_ip()->\
            set_ip_af(types::IPAddressFamily::IP_AF_INET);
        break;
    default:
        break;
    }

    HAL_TRACE_VERBOSE("Added Mirror Session ID {} in get response",
                    session->sw_id);
    return false;
}

hal_ret_t
mirror_session_get (MirrorSessionGetRequest &req, MirrorSessionGetResponseMsg *rsp)
{
    mirror_session_t *session = NULL;
    hal_ret_t ret = HAL_RET_OK;
    auto ms_ht = g_hal_state->mirror_session_ht();

    if (!req.has_key_or_handle()) {
        HAL_TRACE_INFO("Getting all Mirror Sessions");
        if (ms_ht->num_entries() == 0) {
            return HAL_RET_OK;
        }
        auto sdk_ret = ms_ht->walk_safe(mirror_session_fill_rsp, rsp);
        if (sdk_ret != SDK_RET_OK) {
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            HAL_TRACE_ERR("Mirror session walk failed {}", ret);
            return ret;
        }
    } else {
        mirror_session_id_t sw_id = req.key_or_handle().mirrorsession_id();
        HAL_TRACE_INFO("Getting Mirror Session ID {}", sw_id);
        session = (mirror_session_t*)ms_ht->lookup(&sw_id);
        if (session == NULL) {
            HAL_TRACE_ERR("Mirror Session ID {} does not exists", sw_id);
            return HAL_RET_ENTRY_NOT_FOUND;
        }
        mirror_session_fill_rsp(session, rsp);
    }
    return ret;
}

hal_ret_t
mirror_session_get_hw_id (mirror_session_id_t sw_id,
                          mirror_session_id_t *hw_id)
{
    mirror_session_t *session = NULL;
    auto ms_ht = g_hal_state->mirror_session_ht();

    HAL_TRACE_DEBUG("Get hw id for Mirror session ID {} ", sw_id);

    // check if mirror session exists
    session = (mirror_session_t*)ms_ht->lookup(&sw_id);
    if (session == NULL) {
        HAL_TRACE_ERR("Mirror Session ID {} not found", sw_id);
        return HAL_RET_ENTRY_NOT_FOUND;
    }

    auto ret = mirror_session_get_hw_id_helper(session, hw_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get hw id for Mirror session ID {}",
                      sw_id);
        return ret;
    } else {
        HAL_TRACE_DEBUG("Got hw id {} successfully", *hw_id);
    }
    return ret;
}

hal_ret_t
mirror_session_delete (MirrorSessionDeleteRequest &req, MirrorSessionDeleteResponse *rsp)
{
    pd_mirror_session_delete_args_t args;
    pd::pd_func_args_t pd_func_args = {0};
    mirror_session_t *session;
    hal_ret_t ret;
    mirror_session_id_t sw_id;
    auto ms_ht = g_hal_state->mirror_session_ht();

    // check if mirror session already exists
    sw_id = req.key_or_handle().mirrorsession_id();
    HAL_TRACE_INFO("Delete Mirror Session ID {}", sw_id);

    session = (mirror_session_t*)ms_ht->lookup(&sw_id);
    if (session == NULL) {
        HAL_TRACE_ERR("Mirror session ID {} not found.", sw_id);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_ENTRY_NOT_FOUND;
    }

    // PD call to delete mirror session
    args.session = session;
    pd_func_args.pd_mirror_session_delete = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_MIRROR_SESSION_DELETE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD API failed {}", ret);
        rsp->set_api_status(hal_prepare_rsp(ret));
        return ret;
    }

    session = (mirror_session_t*)ms_ht->remove(&sw_id);
    SDK_ASSERT(session != NULL);
    mirror_session_free(session);
    rsp->set_api_status(hal_prepare_rsp(ret));
    rsp->mutable_key_or_handle()->set_mirrorsession_id(sw_id);
    return ret;
}

/*
 * ip: NULL: Change all sessions
 */
hal_ret_t
mirror_session_change (ip_addr_t *ip,
                       bool tnnl_if_valid, if_t *tnnl_if,
                       bool dest_if_valid, if_t *dest_if,
                       bool rtep_ep_valid, ep_t *rtep_ep)
{
    mirror_session_change_ctxt_t ctxt;

    if (dest_if_valid) {
        ctxt.dest_if = dest_if;
    } else {
        ctxt.dest_if = telemetry_mirror_pick_dest_if();
    }

    ctxt.ip = ip;
    ctxt.tnnl_if_valid = tnnl_if_valid;
    ctxt.tnnl_if = tnnl_if;
    ctxt.rtep_ep_valid = rtep_ep_valid;
    ctxt.rtep_ep = rtep_ep;

    auto walk_cb = [](void *ht_entry, void *ctxt) {
        hal_ret_t ret = HAL_RET_OK;
        mirror_session_t *session = (mirror_session_t *)ht_entry;
        mirror_session_change_ctxt_t *ctx = (mirror_session_change_ctxt_t *)ctxt;
        if_t *tnnl_if = NULL;
        ep_t *rtep_ep = NULL;
        HAL_TRACE_DEBUG("Processing mirror sesssion: {} with IP: {}, for tunnel tnnl_ip: {}",
                        session->sw_id,
                        ipaddr2str(&session->mirror_destination_u.er_span_dest.ip_da),
                        ctx->ip ? ipaddr2str(ctx->ip) : "ALL");

        if (!ctx->ip || !memcmp(ctx->ip, &session->mirror_destination_u.er_span_dest.ip_da,
                                sizeof(ip_addr_t))) {
            if (ctx->tnnl_if_valid) {
                tnnl_if = ctx->tnnl_if;
            } else {
                tnnl_if = find_tnnlif_by_dst_ip(intf::IF_TUNNEL_ENCAP_TYPE_GRE,
                                                        &session->mirror_destination_u.er_span_dest.ip_da);
            }

            if (ctx->rtep_ep_valid) {
                rtep_ep = ctx->rtep_ep;
            } else {
                rtep_ep = find_ep_by_v4_key(session->mirror_destination_u.er_span_dest.vrf_id,
                                            session->mirror_destination_u.er_span_dest.ip_da.addr.v4_addr);
            }
            ret = mirror_session_update_pd(session,
                                           ctx->dest_if,
                                           tnnl_if, rtep_ep);

        }
        return false;
    };

    g_hal_state->mirror_session_ht()->walk_safe(walk_cb, &ctxt);
    return HAL_RET_OK;
}

#if 0
hal_ret_t
mirror_session_if_change (ip_addr_t *ip,
                          bool tunnel_if_change,
                          if_t *tunnel_if,
                          bool dest_if_change,
                          if_t *dest_if,
                          bool rtep_ep_change,
                          bool rtep_ep_exists)
{
    hal_ret_t ret = HAL_RET_OK;
    mirror_session_if_change_ctxt_t ctxt;

    ctxt.ip = ip;
    ctxt.tunnel_if_change = tunnel_if_change;
    ctxt.tunnel_if = tunnel_if;
    ctxt.dest_if_change = dest_if_change;
    ctxt.dest_if = dest_if;
    ctxt.rtep_ep_change = rtep_ep_change;
    ctxt.rtep_ep_exists = rtep_ep_exists;

    if (rtep_ep_change && rtep_ep_exists) {
        ctxt.dest_if_change = true;
        ctxt.dest_if = telemetry_mirror_pick_dest_if();
    }

    auto walk_cb = [](void *ht_entry, void *ctxt) {
        hal_ret_t ret = HAL_RET_OK;
        mirror_session_t *session = (mirror_session_t *)ht_entry;
        mirror_session_if_change_ctxt_t *ctx = (mirror_session_if_change_ctxt_t *)ctxt;
        HAL_TRACE_DEBUG("Processing mirror sesssion: {} with IP: {}, for tunnel tnnl_ip: {}",
                        session->sw_id,
                        ipaddr2str(&session->mirror_destination_u.er_span_dest.ip_da),
                        ipaddr2str(ctx->ip));

        if (!memcmp(ctx->ip, &session->mirror_destination_u.er_span_dest.ip_da,
                    sizeof(ip_addr_t))) {
            if (ctx->rtep_ep_change) {
                session->rtep_ep_exists = ctx->rtep_ep_exists;
            }
            ret = mirror_session_update_ifs(session,
                                            ctx->dest_if_change,
                                            ctx->dest_if,
                                            ctx->tunnel_if_change,
                                            ctx->tunnel_if);
            return true;
        }
        return false;
    };

    g_hal_state->mirror_session_ht()->walk_safe(walk_cb, &ctxt);

    return ret;
}
#endif

hal_ret_t
collector_create (CollectorSpec &spec, CollectorResponse *rsp)
{
    collector_config_t cfg;
    pd::pd_collector_create_args_t args;
    pd::pd_func_args_t pd_func_args = {0};
    hal_ret_t ret = HAL_RET_OK;
    mac_addr_t *mac = NULL;
    mac_addr_t smac;
    uint64_t mgmt_mac = 0;
    uint32_t id;
    encap_t  encap;
    // if_t     *dest_if = NULL, *ndest_if = NULL;

    collector_spec_dump(spec);
    // Get free collector id
    sdk_ret_t sret = g_hal_state->telemetry_collectors_bmp()->first_free(&id);
    if (sret != SDK_RET_OK) {
        HAL_TRACE_ERR("Unable to allocate collector! ret: {}", sret);
        rsp->set_api_status(types::API_STATUS_OUT_OF_RESOURCE);
        return HAL_RET_NO_RESOURCE;
    }
    if (id >= HAL_MAX_TELEMETRY_COLLECTORS) {
        HAL_TRACE_ERR("Id is out of bounds. id {}", id);
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }
    HAL_TRACE_DEBUG("ExportID {} allocated id {}",
                        spec.key_or_handle().collector_id(), id);
    // Stash the collector id
    telemetry_collector_id_db[id] = spec.key_or_handle().collector_id();
    g_hal_state->telemetry_collectors_bmp()->set(id);
    
    cfg.collector_id = id;
    ip_addr_spec_to_ip_addr(&cfg.src_ip, spec.src_ip());
    ip_addr_spec_to_ip_addr(&cfg.dst_ip, spec.dest_ip());
    auto ep = find_ep_by_v4_key(spec.vrf_key_handle().vrf_id(), cfg.dst_ip.addr.v4_addr);
    if (!ep) {
        HAL_TRACE_DEBUG("Unable to find ep for ip: {}",
                        ipaddr2str(&cfg.dst_ip));
    }
#if 0
    if (ep == NULL) {
        HAL_TRACE_ERR("PI-Collector: Unknown endpoint {} : {}",
            spec.vrf_key_handle().vrf_id(), ipaddr2str(&cfg.dst_ip));
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    if (!dest_if->is_oob_management &&
        endpoint_is_remote(ep)) {
        ndest_if = telemetry_get_active_bond_uplink();
        if (ndest_if) {
            HAL_TRACE_DEBUG("New Dest IF type {}, op_status {}, id {}",
                             ndest_if->if_type, ndest_if->if_op_status,
                             ndest_if->if_id);
            if (dest_if != ndest_if) {
                // Update EP's if handle
                ep->if_handle = ndest_if->hal_handle;
                // Update the if to ep backptr also
                if_del_ep(dest_if, ep);
                if_add_ep(ndest_if, ep);
            }
        } else {
            HAL_TRACE_ERR("Failed to get active bond0 uplink");
        }
    }
    /* MAC DA */
    mac = ep_get_mac_addr(ep);
    memcpy(cfg.dest_mac, mac, sizeof(mac_addr_t));
#endif

    cfg.template_id = spec.template_id();
    cfg.export_intvl = spec.export_interval();
    switch (spec.format()) {
        case telemetry::ExportFormat::IPFIX:
            cfg.format = EXPORT_FORMAT_IPFIX;
            break;
        case telemetry::ExportFormat::NETFLOWV9:
            HAL_TRACE_ERR("PI-Collector: Netflow-v9 format type is not supported {}",
                           spec.format());
            rsp->set_api_status(types::API_STATUS_INVALID_ARG);
            ret = HAL_RET_INVALID_ARG;
            goto cleanup;
        default:
            HAL_TRACE_ERR("PI-Collector: Unknown format type {}", spec.format());
            rsp->set_api_status(types::API_STATUS_INVALID_ARG);
            ret = HAL_RET_INVALID_ARG;
            goto cleanup;
    }
    cfg.protocol = spec.protocol();
    cfg.dport = spec.dest_port();
    cfg.l2seg = l2seg_lookup_key_or_handle(spec.l2seg_key_handle());
    if (cfg.l2seg == NULL) {
        HAL_TRACE_ERR("PI-Collector: Could not retrieve L2 segment");
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    /* MAC SA. Use mac from device.conf only if it is set. Else derive the smac via ep l2seg */
    mgmt_mac = g_hal_state->mgmt_if_mac();
    if (mgmt_mac == 0) {
        mac = ep_get_rmac(ep, cfg.l2seg);
        memcpy(cfg.src_mac, mac, sizeof(mac_addr_t));
    } else {
        MAC_UINT64_TO_ADDR(smac, mgmt_mac);
        memcpy(cfg.src_mac, smac, sizeof(mac_addr_t));
    }

    /* Encap comes from the l2seg */
    encap = l2seg_get_wire_encap(cfg.l2seg);
    if (encap.type == types::ENCAP_TYPE_DOT1Q) {
        cfg.vlan = encap.val;
        HAL_TRACE_DEBUG("PI-Collector: Encap vlan {}", cfg.vlan);
    } else {
        HAL_TRACE_ERR("PI-Collector: Unsupport Encap {}", encap.type);
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }

    args.cfg = &cfg;
    args.ep = ep;
    pd_func_args.pd_collector_create = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_COLLECTOR_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        rsp->set_api_status(types::API_STATUS_OK);
        HAL_TRACE_ERR("PI-Collector: PD API failed {}", ret);
        goto cleanup;
    }
    HAL_TRACE_DEBUG("SUCCESS: CollectorID {}, dest {}, source {},  port {}",
            cfg.collector_id, ipaddr2str(&cfg.dst_ip),
            ipaddr2str(&cfg.src_ip), cfg.dport);
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_status()->set_handle(spec.key_or_handle().collector_id());

    return HAL_RET_OK;

cleanup:
    telemetry_collector_id_db[id] = -1;
    return ret;
}

hal_ret_t
collector_update (CollectorSpec &spec, CollectorResponse *rsp)
{
    return HAL_RET_OK;
}

hal_ret_t
collector_delete (CollectorDeleteRequest &req, CollectorDeleteResponse *rsp)
{
    hal_ret_t                       ret;
    collector_config_t              cfg;
    pd::pd_func_args_t              pd_func_args = {0};
    pd::pd_collector_delete_args_t  args;

    HAL_TRACE_DEBUG("Collector ID {}",
            req.key_or_handle().collector_id());
    int id = telemetry_collector_get_id(req.key_or_handle().collector_id());
    if (id < 0) {
        HAL_TRACE_ERR("Collector not found for id {}", req.key_or_handle().collector_id());
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_INVALID_ARG;
    }
    args.cfg = &cfg;
    pd_func_args.pd_collector_delete = &args;

    args.cfg->collector_id = id;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_COLLECTOR_DELETE, &pd_func_args);
    if (ret == HAL_RET_OK) {
        rsp->set_api_status(types::API_STATUS_OK);
        rsp->mutable_key_or_handle()->set_collector_id(args.cfg->collector_id);
        g_hal_state->telemetry_collectors_bmp()->clear(id);
        telemetry_collector_id_db[id] = -1;
    } else {
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
    }
    return ret;
}

static hal_ret_t
collector_process_get (CollectorGetRequest &req, CollectorGetResponse *response,
                       pd::pd_collector_get_args_t *args)
{
    hal_ret_t ret = HAL_RET_OK;
    pd::pd_func_args_t pd_func_args = {0};

    pd_func_args.pd_collector_get = args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_COLLECTOR_GET, &pd_func_args);
    if (ret == HAL_RET_OK) {
        response->set_api_status(types::API_STATUS_OK);
        response->mutable_spec()->mutable_key_or_handle()->set_collector_id(args->cfg->collector_id);
        ip_addr_to_spec(response->mutable_spec()->mutable_src_ip(), &args->cfg->src_ip);
        ip_addr_to_spec(response->mutable_spec()->mutable_dest_ip(), &args->cfg->dst_ip);
        response->mutable_spec()->set_dest_port(args->cfg->dport);
        response->mutable_spec()->set_template_id(args->cfg->template_id);
        response->mutable_spec()->set_export_interval(args->cfg->export_intvl);
        response->mutable_spec()->mutable_encap()->set_encap_type(types::encapType::ENCAP_TYPE_GRE);
        response->mutable_spec()->mutable_encap()->set_encap_value(args->cfg->vlan);
        response->mutable_spec()->set_protocol(types::IPProtocol::IPPROTO_UDP);
        response->mutable_stats()->set_num_exported_bytes(args->stats->num_export_bytes);
        response->mutable_stats()->set_num_exported_packets(args->stats->num_export_packets);
        response->mutable_stats()->set_num_exported_records_nonip(args->stats->num_export_records_nonip);
        response->mutable_stats()->set_num_exported_records_ipv4(args->stats->num_export_records_ipv4);
        response->mutable_stats()->set_num_exported_records_ipv6(args->stats->num_export_records_ipv6);
    } else {
        response->set_api_status(types::API_STATUS_INVALID_ARG);
    }
    response->set_api_status(types::API_STATUS_OK);
    return ret;
}

hal_ret_t
collector_ep_update (ip_addr_t *ip, ep_t *ep)
{
    hal_ret_t                           ret = HAL_RET_OK;
    pd::pd_func_args_t                  pd_func_args = {0};
    pd::pd_collector_ep_update_args_t   upd_args;

    upd_args.ip = ip;
    upd_args.ep = ep;
    pd_func_args.pd_collector_ep_update = &upd_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_COLL_EP_UPDATE,
                          &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Unable to update IP for collector: {}",
                        ipaddr2str(ip));
        goto end;
    }

end:
    return ret;
}

hal_ret_t
collector_get (CollectorGetRequest &req, CollectorGetResponseMsg *rsp)
{
    hal_ret_t                   ret = HAL_RET_OK;
    collector_config_t          cfg;
    pd::pd_collector_get_args_t args;

    args.cfg = &cfg;
    args.stats = NULL;
    if (!req.has_key_or_handle()) {
        /* Iterate over all collectors */
        for (int i = 0; i < HAL_MAX_TELEMETRY_COLLECTORS; i++) {
            if (telemetry_collector_id_db[i] > 0) {
                auto response = rsp->add_response();
                memset(&cfg, 0, sizeof(collector_config_t));
                args.cfg->collector_id = i;
                ret = collector_process_get(req, response, &args);
            }
        }
    } else {
        auto response = rsp->add_response();
        memset(&cfg, 0, sizeof(collector_config_t));
        args.cfg->collector_id = req.key_or_handle().collector_id();
        ret = collector_process_get(req, response, &args);
    }
    return ret;
}

hal_ret_t
flow_monitor_acl_ctx_create()
{
    // ACL ctx is created per vrf when we get a new rule create
    return HAL_RET_OK;
}

hal_ret_t get_flowmon_action (FlowMonitorRuleSpec &spec,
                              telemetry::RuleAction *action)
{
    if (spec.has_action()) {
        if (spec.action().action(0) == telemetry::MIRROR_TO_CPU) {
            return (HAL_RET_INVALID_ARG);
        }
        *action = spec.action().action(0);
        /*
        telemetry::MIRROR
        telemetry::MIRROR_TO_CPU
        telemetry::COLLECT_FLOW_STATS;
        */
    } else {
        return (HAL_RET_INVALID_ARG);
    }
    return HAL_RET_OK;
}

static hal_ret_t
populate_flow_monitor_rule (FlowMonitorRuleSpec &spec,
                            flow_monitor_rule_t *rule)
{
    hal_ret_t   ret = HAL_RET_OK;

    // Populate the rule_match structure
    ret = rule_match_spec_extract(spec.match(), &rule->rule_match);
    if (ret != HAL_RET_OK) {
        rule_match_cleanup(&rule->rule_match);
        HAL_TRACE_ERR("Failed to retrieve rule_match");
        return ret;
    }
    rule->action.num_mirror_dest = rule->action.num_collector = 0;
    if (spec.has_action()) {
        if ((spec.action().action(0) == telemetry::MIRROR) ||
            (spec.action().action(0) == telemetry::MIRROR_TO_CPU)) {
            HAL_TRACE_DEBUG("Action: {}", spec.action().action(0));
            /* Mirror action */
            int n = spec.action().ms_key_handle_size();
            HAL_TRACE_DEBUG("Num mirror dest: {}", n);
            if (n > MAX_MIRROR_SESSION_DEST) {
                HAL_TRACE_ERR("Num mirror destinations {} greater than max {}",
                                                    n, MAX_MIRROR_SESSION_DEST);
                return HAL_RET_INVALID_ARG;
            }
            uint32_t hw_id = 0;
            uint32_t sw_id = 0;
            for (int i = 0; i < n; i++) {
                sw_id = spec.action().ms_key_handle(i).mirrorsession_id();
                ret = mirror_session_get_hw_id(sw_id, &hw_id);
                if (ret != HAL_RET_OK) {
                    HAL_TRACE_ERR("Could not find the hw id for Mirror session ID {} ", sw_id);
                    return HAL_RET_INVALID_ARG;
                }
                rule->action.mirror_destinations_sw_id[i] = sw_id;
                rule->action.mirror_destinations[i] = hw_id;
                HAL_TRACE_DEBUG("Mirror Destinations[{}] hw: {} sw: {}", i,
                                rule->action.mirror_destinations[i],
                                rule->action.mirror_destinations_sw_id[i]);
            }
            rule->action.num_mirror_dest = n;
            n = spec.action().action_size();
            if (n != 0) {
                // Only one action for mirroring
                rule->action.mirror_to_cpu = (spec.action().action(0) ==
                                           telemetry::MIRROR_TO_CPU) ? true : false;
                HAL_TRACE_DEBUG("Mirror to cpu: {}", rule->action.mirror_to_cpu);
            }
        }
        if (spec.action().action(0) == telemetry::COLLECT_FLOW_STATS) {
            /* Netflow or IPFIX action */
            if (spec.action().agg_scheme_size() > 0) {
                if (spec.action().agg_scheme(0) != telemetry::NONE) {
                    /* Aggregation scheme not supported */
                    HAL_TRACE_ERR("Aggregation is not supported {}", spec.action().agg_scheme(0));
                    return HAL_RET_INVALID_ARG;
                }
            }
            /* Get collector info */
            int n = spec.collector_key_handle_size();
            if (n == 0) {
                /* No collectors configured! */
                HAL_TRACE_ERR("Action is collect, but no collectors specified");
                return HAL_RET_INVALID_ARG;
            }
            if (n > MAX_COLLECTORS_PER_FLOW) {
                HAL_TRACE_ERR("Only {} collectors allowed per FlowMon rule",
                               MAX_COLLECTORS_PER_FLOW);
                return HAL_RET_INVALID_ARG;
            }
            HAL_TRACE_DEBUG("Collect action: num_collectors {}", n);
            rule->action.num_collector = n;
            for (int i = 0; i < n; i++) {
                int id = telemetry_collector_get_id(spec.collector_key_handle(i).collector_id());
                if (id < 0) {
                    HAL_TRACE_ERR("Invalid collector! id {}",
                            spec.collector_key_handle(i).collector_id());
                    return HAL_RET_INVALID_ARG;
                }
                rule->action.collectors[i] = id;
                HAL_TRACE_DEBUG("Collector[{}]: {}", i, rule->action.collectors[i]);
            }
        }
    }
    return ret;
}

hal_ret_t
flow_monitor_rule_create (FlowMonitorRuleSpec &spec, FlowMonitorRuleResponse *rsp)
{
    uint64_t            vrf_id;
    bool                mirror_action = false;
    rule_key_t          rule_id = (~0);
    hal_ret_t           ret = HAL_RET_OK;
    flow_monitor_rule_t *rule = NULL;
    const acl_ctx_t     *flowmon_acl_ctx = NULL;
    telemetry::RuleAction action;
    uint32_t            id;
    sdk_ret_t           sret = SDK_RET_OK;
    bool                new_ctx = false;
    const acl_ctx_t    *flowmon_acl_ctx_p = NULL;

    vrf_id = spec.vrf_key_handle().vrf_id();
    if (vrf_id == HAL_VRF_ID_INVALID) {
        rsp->set_api_status(types::API_STATUS_VRF_ID_INVALID);
        HAL_TRACE_ERR("Invalid vrf {}", spec.vrf_key_handle().vrf_id());
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }
    // Get free id
    sret = g_hal_state->telemetry_flowmon_bmp()->first_free(&id);
    if (sret != SDK_RET_OK) {
        HAL_TRACE_ERR("Unable to allocate rule-id! ret: {}", sret);
        rsp->set_api_status(types::API_STATUS_OUT_OF_RESOURCE);
        return HAL_RET_NO_RESOURCE;
    }
    rule_id = id;
    HAL_TRACE_DEBUG("Allocated ruleid {} spec_id {}", rule_id,
                            spec.key_or_handle().flowmonitorrule_id());
    if (rule_id >= MAX_FLOW_MONITOR_RULES) {
        HAL_TRACE_ERR("Id is out of bounds. id {}", rule_id);
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }
    // Stash the rule id
    flow_monitor_rule_id_db[rule_id] = spec.key_or_handle().flowmonitorrule_id();
    g_hal_state->telemetry_flowmon_bmp()->set(rule_id);

    rule = flow_monitor_rule_alloc_init();
    rule->vrf_id = vrf_id;
    rule->rule_id = rule_id;
    rule->flowmon_rules_ht_ctxt.reset();
    g_hal_state->flowmon_rules_ht()->insert(rule,
                                             &rule->flowmon_rules_ht_ctxt);
    ret = get_flowmon_action(spec, &action);
    if (ret != HAL_RET_OK) {
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        HAL_TRACE_ERR("Invalid action, ruleid {}", spec.key_or_handle().flowmonitorrule_id());
        ret = HAL_RET_INVALID_ARG;
        goto end;
    } else {
        mirror_action = (action == telemetry::MIRROR);
    }
    flowmon_acl_ctx = acl::acl_get(flowmon_acl_ctx_name(rule->vrf_id, mirror_action));
    if (!flowmon_acl_ctx) {
        /* Create a new acl context */
        flowmon_acl_ctx = hal::rule_lib_init(flowmon_acl_ctx_name(rule->vrf_id, mirror_action),
                                             &flowmon_rule_config_glbl);
        HAL_TRACE_DEBUG("Creating new ACL ctx for vrf {} id {} ctx_ptr {:#x}",
                         flowmon_acl_ctx_name(rule->vrf_id, mirror_action),
                         rule->vrf_id, (uint64_t) flowmon_acl_ctx);
        new_ctx = true;
    }
    ret = populate_flow_monitor_rule(spec, rule);
    if (ret != HAL_RET_OK) {
        rsp->set_api_status(types::API_STATUS_ERR);
        goto end;
    }

    flowmon_acl_ctx_p = flowmon_acl_ctx;
    ret = rule_match_rule_add(&flowmon_acl_ctx, &rule->rule_match, rule_id, 0,
                              (void *)&rule->ref_count);
    if (ret != HAL_RET_OK) {
        rsp->set_api_status(types::API_STATUS_ERR);
        HAL_TRACE_ERR("Rule match add failed: ruleid {}", spec.key_or_handle().flowmonitorrule_id());
        goto end;
    }
    rsp->set_api_status(types::API_STATUS_OK);

end:
    if (flowmon_acl_ctx) {
        // Commit the new context if context gets copied due to ref_count
        if ((new_ctx || (flowmon_acl_ctx_p != flowmon_acl_ctx)) && (ret == HAL_RET_OK)) {
            ret = acl::acl_commit(flowmon_acl_ctx);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("ACL commit fail vrf {} id {}",
                           flowmon_acl_ctx_name(vrf_id, mirror_action), vrf_id);
                rsp->set_api_status(types::API_STATUS_ERR);
            }
        }
        // acl_get clones the context, so decrement the
        // ref_count
        acl::acl_deref(flowmon_acl_ctx);
    }
    // Reeval telemetry sessions
    if (ret == HAL_RET_OK) {
        ret = telemetry_eval_sessions();
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("telemetry_eval_sessions failed {}", ret);
        }
    } else {
        // Free any allocated ids
        if (rule_id != (uint64_t) (~0)) {
            g_hal_state->telemetry_flowmon_bmp()->clear(rule_id);
        }
    }
    return ret;
}

hal_ret_t
flow_monitor_rule_update (FlowMonitorRuleSpec &spec, FlowMonitorRuleResponse *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    return ret;
}

hal_ret_t
flow_monitor_rule_delete (FlowMonitorRuleDeleteRequest &req, FlowMonitorRuleDeleteResponse *rsp)
{
    bool                mirror_action = false;
    hal_ret_t           ret = HAL_RET_OK;
    uint64_t            vrf_id;
    int                 id;
    rule_key_t          rule_id = (~0);
    flow_monitor_rule_t *rule = NULL;
    const acl_ctx_t     *flowmon_acl_ctx = NULL;
    const acl_ctx_t    *flowmon_acl_ctx_p = NULL;

    vrf_id = req.vrf_key_handle().vrf_id();
    if (vrf_id == HAL_VRF_ID_INVALID) {
        rsp->set_api_status(types::API_STATUS_VRF_ID_INVALID);
        HAL_TRACE_ERR("vrf {}", req.vrf_key_handle().vrf_id());
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    id = telemetry_flow_monitor_rule_get_id(
                        req.key_or_handle().flowmonitorrule_id());
    rule_id = id;
    if (rule_id < 0) {
        HAL_TRACE_ERR("Rule not found for id {}",
                        req.key_or_handle().flowmonitorrule_id());
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_INVALID_ARG;
    }
    rule = (flow_monitor_rule_t *)g_hal_state->flowmon_rules_ht()->lookup((void *)&rule_id);
    if (!rule) {
        HAL_TRACE_DEBUG("Ruleid {} does not exist", rule_id);
        ret = HAL_RET_OK;
        goto end;
    }
    HAL_TRACE_DEBUG("Ruleid {}", rule_id);
    mirror_action = (rule->action.num_mirror_dest > 0);
    flowmon_acl_ctx = acl::acl_get(flowmon_acl_ctx_name(vrf_id, mirror_action));
    if (!flowmon_acl_ctx) {
        HAL_TRACE_DEBUG("Did not find flowmon acl ctx for vrf_id {}", vrf_id);
        ret = HAL_RET_OK;
        goto end;
    }
    HAL_TRACE_DEBUG("Got ctx_name: {} acl_ctx: {:#x}",
                     flowmon_acl_ctx_name(vrf_id, mirror_action),
                     (uint64_t) flowmon_acl_ctx);

    if (g_hal_state->flowmon_rules_ht()->num_entries() == 1) {
        if (flowmon_acl_ctx) {
            acl_dump(flowmon_acl_ctx, 0x01, [](acl_rule_t *rule) {
                  PRINT_RULE_FIELDS(rule);});
            acl::acl_deref(flowmon_acl_ctx);
        }

        // Knock of the context if we are done cleaning
        // all the rules
        rule_lib_delete(flowmon_acl_ctx_name(vrf_id, mirror_action)); 
    } else {
        flowmon_acl_ctx_p = flowmon_acl_ctx;
        ret = rule_match_rule_del(&flowmon_acl_ctx, &rule->rule_match, rule_id, 0,
                              (void *)&rule->ref_count);
        if (ret != HAL_RET_OK) {
            rsp->set_api_status(types::API_STATUS_ERR);
            HAL_TRACE_ERR("Rule match del failed: ruleid {}", rule_id);
            goto end;
        }
        // Commit the changes if the context has changed
        if (flowmon_acl_ctx_p != flowmon_acl_ctx) {
            ret = acl::acl_commit(flowmon_acl_ctx);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("ACL commit fail vrf {} id {}",
                   flowmon_acl_ctx_name(vrf_id, mirror_action), vrf_id);
                rsp->set_api_status(types::API_STATUS_ERR);
                goto end;
            }
        }

        if (flowmon_acl_ctx) {
            acl_dump(flowmon_acl_ctx, 0x01, [](acl_rule_t *rule) {
                     PRINT_RULE_FIELDS(rule);});
            acl::acl_deref(flowmon_acl_ctx);
        }
    }
    g_hal_state->flowmon_rules_ht()->remove_entry(rule,
                                                 &rule->flowmon_rules_ht_ctxt); 
    flow_monitor_rule_free(rule);
    rsp->set_api_status(types::API_STATUS_OK);
    // Cleanup id bitmap state
    g_hal_state->telemetry_flowmon_bmp()->clear(rule_id);
    flow_monitor_rule_id_db[rule_id] = -1;

end:

    // Reeval telemetry sessions
    if (ret == HAL_RET_OK) {
        ret = telemetry_eval_sessions();
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("telemetry_eval_sessions failed {}", ret);
        }
    }
    return ret;
}

static inline hal_ret_t
flow_monitor_rule_spec_build (flow_monitor_rule_t *rule, FlowMonitorRuleGetResponse *resp) 
{
    hal_ret_t ret = HAL_RET_OK;

    ret = rule_match_spec_build(&rule->rule_match, resp->mutable_spec()->mutable_match());
    if (ret != HAL_RET_OK) {
        resp->set_api_status(types::API_STATUS_ERR);
        return ret;
    }

    resp->mutable_spec()->mutable_key_or_handle()->set_flowmonitorrule_id(rule->rule_id);
    resp->mutable_spec()->mutable_vrf_key_handle()->set_vrf_id(rule->vrf_id);

    HAL_TRACE_DEBUG("Number of collectors: {}", rule->action.num_collector);
    HAL_TRACE_DEBUG("Number of mirrors: {}", rule->action.num_mirror_dest);

    for (uint8_t idx = 0; idx<rule->action.num_collector; idx++) {
         if (!idx)
             resp->mutable_spec()->mutable_action()->add_action(telemetry::COLLECT_FLOW_STATS);
         resp->mutable_spec()->add_collector_key_handle()->set_collector_id(\
              rule->action.collectors[idx]);
    }

    for (uint8_t idx = 0; idx<rule->action.num_mirror_dest; idx++) {
         if (!idx) {
             if (rule->action.mirror_to_cpu) 
                 resp->mutable_spec()->mutable_action()->\
                       add_action(telemetry::MIRROR_TO_CPU);
             else
                 resp->mutable_spec()->mutable_action()->\
                       add_action(telemetry::MIRROR);
         }
         resp->mutable_spec()->mutable_action()->add_ms_key_handle()->\
             set_mirrorsession_id(rule->action.mirror_destinations_sw_id[idx]);
    }  

    return HAL_RET_OK;
}

hal_ret_t
flow_monitor_rule_get (FlowMonitorRuleGetRequest &req, FlowMonitorRuleGetResponseMsg *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    uint32_t       rule_id = 0;
    flow_monitor_rule_t *rule=NULL;

    if (req.has_key_or_handle()) {
        rule_id = telemetry_flow_monitor_rule_get_id(
                        req.key_or_handle().flowmonitorrule_id());
        if (rule_id < 0) {
            HAL_TRACE_ERR("Rule not found for id {}",
                        req.key_or_handle().flowmonitorrule_id());
            return HAL_RET_INVALID_ARG;
        }

        rule = (flow_monitor_rule_t*)g_hal_state->flowmon_rules_ht()->lookup((void *)&rule_id);
        if (rule) {
            return flow_monitor_rule_spec_build(rule, rsp->add_response());
        } else {
            HAL_TRACE_ERR("Rule not found for id {}",
                        req.key_or_handle().flowmonitorrule_id());
            return HAL_RET_INVALID_ARG;
        }
    }

    for (int i = 0; i < MAX_FLOW_MONITOR_RULES; i++) {
        rule_id = flow_monitor_rule_id_db[i];
        if (rule_id > 0) {
            HAL_TRACE_DEBUG("Rule id: {}", rule_id);
            rule = (flow_monitor_rule_t*)g_hal_state->flowmon_rules_ht()->lookup((void *)&i);
            if (rule) {
                HAL_TRACE_DEBUG("Found rule with rule id: {}", rule->rule_id);
                flow_monitor_rule_spec_build(rule, rsp->add_response());
            }
        }
    }
    
    return ret;
}

static void
populate_drop_monitor_rule (DropMonitorRuleSpec &spec,
                            drop_monitor_rule_t *rule)
{
    rule->codes.drop_input_mapping = spec.reasons().drop_input_mapping();
    rule->codes.drop_input_mapping_dejavu = spec.reasons().drop_input_mapping_dejavu();
    rule->codes.drop_flow_hit = spec.reasons().drop_flow_hit();
    rule->codes.drop_flow_miss = spec.reasons().drop_flow_miss();
    rule->codes.drop_ipsg = spec.reasons().drop_ipsg();
    rule->codes.drop_nacl = spec.reasons().drop_nacl();
    rule->codes.drop_malformed_pkt = spec.reasons().drop_malformed_pkt();
    rule->codes.drop_ip_normalization = spec.reasons().drop_ip_normalization();
    rule->codes.drop_tcp_normalization = spec.reasons().drop_tcp_normalization();
    rule->codes.drop_tcp_non_syn_first_pkt = spec.reasons().drop_tcp_non_syn_first_pkt();
    rule->codes.drop_icmp_normalization = spec.reasons().drop_icmp_normalization();
    rule->codes.drop_input_properties_miss = spec.reasons().drop_input_properties_miss();
    rule->codes.drop_tcp_out_of_window = spec.reasons().drop_tcp_out_of_window();
    rule->codes.drop_tcp_split_handshake = spec.reasons().drop_tcp_split_handshake();
    rule->codes.drop_tcp_win_zero_drop = spec.reasons().drop_tcp_win_zero_drop();
    rule->codes.drop_tcp_data_after_fin = spec.reasons().drop_tcp_data_after_fin();
    rule->codes.drop_tcp_non_rst_pkt_after_rst = spec.reasons().drop_tcp_non_rst_pkt_after_rst();
    rule->codes.drop_tcp_invalid_responder_first_pkt = spec.reasons().drop_tcp_invalid_responder_first_pkt();
    rule->codes.drop_tcp_unexpected_pkt = spec.reasons().drop_tcp_unexpected_pkt();
    rule->codes.drop_src_lif_mismatch = spec.reasons().drop_src_lif_mismatch();
    rule->codes.drop_parser_icrc_error = spec.reasons().drop_parser_icrc_error();
    rule->codes.drop_parse_len_error = spec.reasons().drop_parse_len_error();
    rule->codes.drop_hardware_error = spec.reasons().drop_hardware_error();
    return;
}

hal_ret_t
drop_monitor_rule_create (DropMonitorRuleSpec &spec, DropMonitorRuleResponse *rsp)
{
    pd_drop_monitor_rule_create_args_t args = {0};
    pd::pd_func_args_t pd_func_args = {0};
    drop_monitor_rule_t rule = {0};
    hal_ret_t ret = HAL_RET_OK;
    int sess_id;

    dropmonrule_spec_dump(spec);
    populate_drop_monitor_rule(spec, &rule);
    int n = spec.ms_key_handle_size();
    for (int i = 0; i < n; i++) {
        sess_id = spec.ms_key_handle(i).mirrorsession_id();
        if (sess_id >= MAX_MIRROR_SESSION_DEST) {
            ret = HAL_RET_INVALID_ARG;
            HAL_TRACE_ERR("PI-DropMonitor create failed {} mirror_dest_id: {}",
                           ret, sess_id);
            goto end;
        }
        rule.mirror_destinations[sess_id] = true;
    }
    args.rule = &rule;
    pd_func_args.pd_drop_monitor_rule_create = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_DROP_MONITOR_RULE_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-DropMonitor create failed {}", ret);
        goto end;
    } else {
        HAL_TRACE_DEBUG("PI-DropMonitor create succeeded");
    }

end:
    return ret;
}

hal_ret_t
drop_monitor_rule_update (DropMonitorRuleSpec &spec, DropMonitorRuleResponse *rsp)
{
    return HAL_RET_OK;
}

hal_ret_t
drop_monitor_rule_delete (DropMonitorRuleDeleteRequest &req, DropMonitorRuleDeleteResponse *rsp)
{
    pd_drop_monitor_rule_create_args_t args = {0};
    pd::pd_func_args_t pd_func_args = {0};
    drop_monitor_rule_t rule = {0};
    hal_ret_t ret;

    rule.rule_id = req.key_or_handle().dropmonitorrule_id();
    args.rule = &rule;
    pd_func_args.pd_drop_monitor_rule_create = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_DROP_MONITOR_RULE_DELETE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-DropMonitor delete failed {}", ret);
        goto end;
    } else {
        HAL_TRACE_DEBUG("PI-DropMonitor delete succeeded");
    }

end:
    return ret;
}

hal_ret_t
drop_monitor_rule_get (DropMonitorRuleGetRequest &req, DropMonitorRuleGetResponseMsg *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    return ret;
}

}    // namespace hal
