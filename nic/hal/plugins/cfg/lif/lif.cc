//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <google/protobuf/util/json_util.h>
#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "gen/proto/l2segment.pb.h"
#include "nic/hal/plugins/cfg/nw/filter.hpp"
#include "nic/hal/src/utils/utils.hpp"
#include "nic/hal/plugins/cfg/mcast/oif_list_api.hpp"
#include "nic/hal/plugins/cfg/aclqos/acl_api.hpp"
#include "nic/hal/src/utils/if_utils.hpp"
#include "nic/hal/plugins/cfg/nvme/nvme.hpp"
#include "nic/hal/plugins/cfg/rdma/rdma.hpp"
#ifdef __x86_64__
#include "nic/hal/src/internal/cpucb.hpp"
#endif
#include "nic/hal/plugins/cfg/lif/lif.hpp"
#include "nic/hal/src/internal/eth.hpp"
#include "nic/hal/plugins/cfg/aclqos/qos.hpp"

using hal::pd::pd_if_create_args_t;
using hal::pd::pd_if_lif_update_args_t;

namespace hal {

static void
lif_process_get (lif_t *lif, LifGetResponse *rsp)
{
    hal_ret_t ret;
    LifSpec  *spec     = NULL;
    uint64_t hw_lif_id = lif_hw_lif_id_get(lif);
    pd::pd_lif_stats_get_args_t args   = {0};
    pd::pd_func_args_t          pd_func_args = {0};

    // fill in the config spec of this lif.
    spec = rsp->mutable_spec();
    spec->mutable_key_or_handle()->set_lif_id(lif->lif_id);
    spec->set_name(lif->name);
    spec->set_type(lif->type);
    spec->set_admin_status(lif->admin_status);
    spec->set_enable_rdma(lif->enable_rdma);
    spec->set_rdma_max_keys(lif->rdma_max_keys);
    spec->set_rdma_max_pt_entries(lif->rdma_max_pt_entries);
    spec->set_enable_nvme(lif->enable_nvme);
    spec->set_nvme_max_ns(lif->nvme_max_ns);
    spec->set_nvme_max_sess(lif->nvme_max_sess);
    spec->set_rdma_sniff_en(lif->rdma_sniff_en);

    spec->set_vlan_strip_en(lif->vlan_strip_en);
    spec->set_vlan_insert_en(lif->vlan_insert_en);
    if (lif->pinned_uplink != HAL_HANDLE_INVALID) {
        if_t *hal_if = find_if_by_handle(lif->pinned_uplink);
        spec->mutable_pinned_uplink_if_key_handle()->set_interface_id(hal_if->if_id);
    }
    qos_policer_to_spec(&lif->qos_info.rx_policer, spec->mutable_rx_policer());
    qos_policer_to_spec(&lif->qos_info.tx_policer, spec->mutable_tx_policer());
    spec->mutable_rss()->set_type(lif->rss.type);
    spec->mutable_rss()->set_key(lif->rss.key, sizeof(lif->rss.key));
    spec->mutable_rss()->set_indir(lif->rss.indir, sizeof(lif->rss.indir));

    spec->mutable_packet_filter()->set_receive_promiscuous(lif->packet_filters.receive_promiscuous);
    spec->mutable_packet_filter()->set_receive_broadcast(lif->packet_filters.receive_broadcast);
    spec->mutable_packet_filter()->set_receive_all_multicast(lif->packet_filters.receive_all_multicast);

    rsp->mutable_status()->set_hw_lif_id(hw_lif_id);
    rsp->mutable_status()->set_lif_handle(lif->hal_handle);
    rsp->mutable_status()->set_lif_status(lif->admin_status);

    // Return LifQstate addresses
    intf::LifQState *entry;
    for (int i = 0; i < intf::LifQPurpose_MAX; i++) {
        const auto &ent = lif->qinfo[i];
        entry = rsp->add_qstate();
        entry->set_type_num(ent.type);
        // entry->set_addr(lif_manager()->GetLIFQStateAddr(hw_lif_id, ent.type, 0));
        entry->set_addr(lif_manager()->get_lif_qstate_addr(hw_lif_id, ent.type, 0));
    }

    if (lif->lif_id == HAL_LIF_CPU) {
#ifdef __x86_64__
        ret = cpucb_get_stats(lif->lif_id, rsp);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_DEBUG("cpucb pd stats get failed");
        }
#endif
    } else {

        // Getting PD information
        args.lif = lif;
        args.rsp = rsp;
        pd_func_args.pd_lif_stats_get = &args;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_LIF_STATS_GET, &pd_func_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Unable to do PD stats get for lif: {}. ret : {}",
                          lif->lif_id, ret);
        }
    }

    rsp->set_api_status(types::API_STATUS_OK);
    proto_msg_dump(*rsp);
}

//-----------------------------------------------------------------------------
// API to send LIF updates to Agent
//-----------------------------------------------------------------------------
static void
hal_stream_lif_updates (void *timer, uint32_t timer_id, void *ctxt) {
    uint64_t lif_id = (uint64_t)ctxt;

    lif_t *lif = find_lif_by_id((uint32_t)lif_id);
    if (lif == NULL) {
        HAL_TRACE_ERR("Lif not found for id: {}", ((uint32_t)lif_id));
        return;
    }

    auto walk_cb = [](uint32_t event_id, void *entry, void *ctxt) {
        grpc::ServerWriter<EventResponse> *stream =
                 (grpc::ServerWriter<EventResponse> *)ctxt;
        lif_t          *lif  = (lif_t *)entry;
        EventResponse   evtresponse;

        evtresponse.set_event_id(::event::EVENT_ID_LIF_ADD_UPDATE);
        lif_process_get(lif, evtresponse.mutable_lif_event());
        stream->Write(evtresponse);
        return true;
    };

    g_hal_state->event_mgr()->notify_event(::event::EVENT_ID_LIF_ADD_UPDATE, lif, walk_cb);
}

void *
lif_id_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t *ht_entry = NULL;
    lif_t                    *lif      = NULL;

    SDK_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    lif = (lif_t *)hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(lif->lif_id);
}

uint32_t
lif_id_key_size ()
{
    return sizeof(lif_id_t);
}

// allocate a LIF instance
static inline lif_t *
lif_alloc (void)
{
    lif_t    *lif;

    lif = (lif_t *)g_hal_state->lif_slab()->alloc();
    if (lif == NULL) {
        return NULL;
    }
    return lif;
}

// initialize a LIF instance
static inline lif_t *
lif_init (lif_t *lif)
{
    if (!lif) {
        return NULL;
    }
    SDK_SPINLOCK_INIT(&lif->slock, PTHREAD_PROCESS_SHARED);

    // initialize the operational state
    lif->hal_handle    = HAL_HANDLE_INVALID;
    lif->pinned_uplink = HAL_HANDLE_INVALID;
    lif->filter_list = block_list::factory(sizeof(hal_handle_t));

    // initialize meta information
    sdk::lib::dllist_reset(&lif->if_list_head);

    return lif;
}

// allocate and initialize a interface instance
static inline lif_t *
lif_alloc_init (void)
{
    return lif_init(lif_alloc());
}

static inline hal_ret_t
lif_free (lif_t *lif)
{
    SDK_SPINLOCK_DESTROY(&lif->slock);
    hal::delay_delete_to_slab(HAL_SLAB_LIF, lif);
    return HAL_RET_OK;
}

// anti lif_alloc_init
hal_ret_t
lif_cleanup (lif_t *lif)
{
    if (lif->filter_list) {
        block_list::destroy(lif->filter_list);
    }

    lif_free(lif);

    return HAL_RET_OK;
}

lif_t *
find_lif_by_id (lif_id_t lif_id)
{
    hal_handle_id_ht_entry_t    *entry;
    lif_t                       *lif;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->lif_id_ht()->lookup(&lif_id);
    if (entry && (entry->handle_id != HAL_HANDLE_INVALID)) {
        // check for object type
        SDK_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() ==
                   HAL_OBJ_ID_LIF);
        lif = (lif_t *)hal_handle_get_obj(entry->handle_id);
        return lif;
    }
    return NULL;
}

uint64_t
lif_hw_lif_id_get (lif_t *lif)
{
    hal_ret_t ret = HAL_RET_OK;
    pd::pd_func_args_t          pd_func_args = {0};

    pd::pd_lif_get_args_t hwlifid_args;
    memset(&hwlifid_args, 0, sizeof(pd::pd_lif_get_args_t));

    hwlifid_args.lif = lif;
    pd_func_args.pd_lif_get = &hwlifid_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_LIF_GET, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get hw_lif_id for lif {}", lif->lif_id);
    }

    return hwlifid_args.hw_lif_id;
}


static bool
lif_get_from_hw_lif_id_ht_cb (void *ht_entry, void *ctxt)
{
    hal_handle_id_ht_entry_t        *entry = (hal_handle_id_ht_entry_t *)ht_entry;
    lif_get_from_hw_lif_id_ctxt_t   *lif_ctxt = (lif_get_from_hw_lif_id_ctxt_t *)ctxt;
    lif_t                           *lif             = NULL;

    lif = (lif_t *)hal_handle_get_obj(entry->handle_id);
    uint32_t hw_lif_id = lif_hw_lif_id_get(lif);
    if (lif_ctxt->hw_lif_id == hw_lif_id) {
        lif_ctxt->lif = lif;
        return true;    // returning true stops the walk
    }

    // returning false, walks through.
    return false;
}

lif_t *
find_lif_by_hw_lif_id (uint32_t hw_lif_id)
{
    lif_get_from_hw_lif_id_ctxt_t lif_ctxt = {0};

    lif_ctxt.hw_lif_id = hw_lif_id;
    g_hal_state->lif_id_ht()->walk(lif_get_from_hw_lif_id_ht_cb, &lif_ctxt);

    return lif_ctxt.lif;
}

lif_t *
find_lif_by_handle (hal_handle_t handle)
{
    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }
    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_DEBUG("{}:failed to find object with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_LIF) {
        HAL_TRACE_DEBUG("{}:failed to find lif with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    return (lif_t *)hal_handle->obj();
}

static bool
lif_get_from_name_ht_cb (void *ht_entry, void *ctxt)
{
    hal_handle_id_ht_entry_t    *entry = (hal_handle_id_ht_entry_t *)ht_entry;
    lif_get_from_name_ctxt_t    *lif_ctxt = (lif_get_from_name_ctxt_t*)ctxt;
    lif_t                       *lif             = NULL;

    lif = (lif_t *)hal_handle_get_obj(entry->handle_id);
    if (!strncmp(lif_ctxt->name, lif->name, LIF_NAME_LEN)) {
        lif_ctxt->lif = lif;
        return true;    // returning true stops the walk
    }

    // returning false, walks through.
    return false;
}

lif_t *
find_lif_by_name (char *name)
{
    lif_get_from_name_ctxt_t lif_ctxt = {0};

    lif_ctxt.name = name;
    g_hal_state->lif_id_ht()->walk(lif_get_from_name_ht_cb, &lif_ctxt);

    return lif_ctxt.lif;
}

//------------------------------------------------------------------------------
// insert this lif in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
lif_add_to_db (lif_t *lif, hal_handle_t handle)
{
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    hal_handle_id_ht_entry_t    *entry = NULL;

    HAL_TRACE_DEBUG("{}:adding to lif id hash table",
                    __FUNCTION__);
    // allocate an entry to establish mapping from vrf id to its handle
    entry =
        (hal_handle_id_ht_entry_t *)g_hal_state->
        hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from vrf id to its handle
    entry->handle_id = handle;
    sdk_ret = g_hal_state->lif_id_ht()->insert_with_key(&lif->lif_id,
                                                        entry, &entry->ht_ctxt);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("{}:failed to add lif id to handle mapping, "
                      "err : {}", __FUNCTION__, ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }

    // TODO: Check if this is the right place
    lif->hal_handle = handle;

    return ret;
}

//------------------------------------------------------------------------------
// delete a lif from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
lif_del_from_db (lif_t *lif)
{
    hal_handle_id_ht_entry_t *entry;

    HAL_TRACE_DEBUG("{}:removing from lif id hash table", __FUNCTION__);
    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)g_hal_state->lif_id_ht()->
        remove(&lif->lif_id);

    // free up
    hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// init lif specific queue state, if any
//------------------------------------------------------------------------------
hal_ret_t
lif_qstate_map_init (LifSpec& spec, uint32_t hw_lif_id, lif_t *lif, bool dont_zero_qstate_mem)
{
    lif_qstate_t qstate;
    uint32_t qcount = 0;

    memset(&qstate, 0, sizeof(lif_qstate_t));

    qstate.lif_id = hw_lif_id;

    for (int i = 0; i < spec.lif_qstate_map_size(); i++) {
        const auto &ent = spec.lif_qstate_map(i);
        if (ent.type_num() >= 8) {
            HAL_TRACE_ERR("Invalid type num in LifSpec : {}", ent.type_num());
            return HAL_RET_INVALID_ARG;
        }
        if (ent.size() > 7 || ent.entries() > 24) {
            HAL_TRACE_ERR("Invalid entry in LifSpec : size={} entries={}",
                          ent.size(), ent.entries());
            return HAL_RET_INVALID_ARG;
        }

        if (ent.purpose() > intf::LifQPurpose_MAX) {
            HAL_TRACE_ERR("Invalid entry in LifSpec : purpose={}", ent.purpose());
            return HAL_RET_INVALID_ARG;
        }

        qstate.type[ent.type_num()].qtype_info.size = ent.size();
        qstate.type[ent.type_num()].qtype_info.entries = ent.entries();

        // Set both cosA,cosB to admin_cos(cosA) value for admin-qtype.
        if (ent.purpose() != intf::LIF_QUEUE_PURPOSE_ADMIN ||
                (hw_lif_id >= HAL_LIF_ID_SVC_LIF_MIN && hw_lif_id <= HAL_LIF_ID_SVC_LIF_MAX)) {
            qstate.type[ent.type_num()].qtype_info.cosA = (lif->qos_info.coses & 0x0f);
            qstate.type[ent.type_num()].qtype_info.cosB = (lif->qos_info.coses & 0xf0) >> 4;
        } else {
            qstate.type[ent.type_num()].qtype_info.cosA =
                qstate.type[ent.type_num()].qtype_info.cosB = (lif->qos_info.coses & 0x0f);
        }

        lif->qinfo[ent.purpose()].type = ent.type_num();
        lif->qinfo[ent.purpose()].size = (uint16_t)pow(2, ent.size());
        lif->qinfo[ent.purpose()].num_queues = pow(2, ent.entries());
        qcount += lif->qinfo[ent.purpose()].num_queues;

        HAL_TRACE_DEBUG("type: {}, entries: {}, size: {}, log entries: {}, size: {}",
                        ent.type_num(),
                        ent.entries(), ent.size(), lif->qinfo[ent.purpose()].num_queues,
                        lif->qinfo[ent.purpose()].size);
    }

    HAL_TRACE_VERBOSE("Lif lif_id: {}, Qcount: {}",
                    lif->lif_id, qcount);
    lif->qcount = qcount;


    // cosB (default cos) will be the hint_cos for the lif.
    qstate.hint_cos = (lif->qos_info.coses & 0xf0) >> 4;
    // Program lif
    if (lif->qstate_pgm_in_hal) {
        // Reserve lif id
        lif_manager()->reserve_id(qstate.lif_id, 1);

        // Init the lif
        lif_manager()->init(&qstate);

        if (!dont_zero_qstate_mem) {
            // Zero out qstate
            lif_manager()->clear_qstate(qstate.lif_id);
        }

        // Program qstate map
        lif_manager()->enable(qstate.lif_id);

    } else {
        HAL_TRACE_DEBUG("Skipped initing lifqstate");
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// init lif specific queue state, if any
//------------------------------------------------------------------------------
hal_ret_t
lif_qstate_init (LifSpec& spec, uint32_t hw_lif_id, lif_t *lif)
{
    std::unique_ptr<uint8_t[]> buf;
    hal_ret_t ret = HAL_RET_OK;
    sdk_ret_t sret = SDK_RET_OK;

    for (int i = 0; i < spec.lif_qstate_size(); i++) {
        const auto &req = spec.lif_qstate(i);

        uint8_t *state = (uint8_t *)req.queue_state().c_str();
        if (req.has_label()) {
            uint8_t off;
#if 0
            int ret = lif_manager()->GetPCOffset(req.label().handle().c_str(),
                req.label().prog_name().c_str(),
                req.label().label().c_str(), &off);
#endif
            int ret = hal_get_pc_offset(req.label().prog_name().c_str(),
                                    req.label().label().c_str(), &off);
            if (ret < 0) {
                ret = HAL_RET_ERR;
                goto end;
            }
            if (req.queue_state().size() != 0) {
                buf.reset(new uint8_t[req.queue_state().size()]);
                bcopy(req.queue_state().c_str(), buf.get(), req.queue_state().size());
                buf.get()[0] = off;
                state = buf.get();
            } else {
                HAL_TRACE_DEBUG("qstate size{}", req.queue_state().size());
            }
        }

        sret = lif_manager()->write_qstate(hw_lif_id, req.type_num(),
                                           req.qid(), state,
                                           req.queue_state().size());
        if (sret != SDK_RET_OK) {
            HAL_TRACE_ERR("Failed to set LIFQState : {}", sret);
            ret = HAL_RET_ERR;
            goto end;
        }
#if 0
        int ret = lif_manager()->WriteQState(hw_lif_id, req.type_num(),
                                             req.qid(), state,
                                             req.queue_state().size());
        if (ret < 0) {
            HAL_TRACE_ERR("Failed to set LIFQState : {}", ret);
            ret = HAL_RET_ERR;
            goto end;
        }
#endif
    }
end:
    return ret;
}

static hal_ret_t
lif_fwd_create (LifSpec& spec, LifResponse *rsp, lif_t *lif,
                lif_hal_info_t *lif_hal_info)
{
    hal_ret_t                ret = HAL_RET_OK;
    pd::pd_lif_create_args_t pd_lif_args;
    pd::pd_func_args_t          pd_func_args = {0};

    HAL_TRACE_VERBOSE("{}: P4 Processing for lif id {}", __FUNCTION__,
                    spec.key_or_handle().lif_id());

    // allocate all PD resources and finish programming, if any
    pd::pd_lif_create_args_init(&pd_lif_args);
    pd_lif_args.lif = lif;
    if (lif_hal_info) {
        pd_lif_args.with_hw_lif_id = lif_hal_info->with_hw_lif_id;
        pd_lif_args.hw_lif_id = lif_hal_info->hw_lif_id;
    }
    pd_func_args.pd_lif_create = &pd_lif_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_LIF_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD lif create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto end;
    }

end:

    return ret;
}

hal_ret_t
validate_lif_create(LifSpec& spec, LifResponse *rsp)
{
    hal_ret_t ret = HAL_RET_OK;

    // make sure key-handle field is set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("{}: Lif didn't have key or handle. Err: {}",
                      __FUNCTION__, HAL_RET_INVALID_ARG);
        rsp->set_api_status(types::API_STATUS_LIF_ID_INVALID);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // check if lif id is set in key-handle
    if (spec.key_or_handle().key_or_handle_case() !=
            kh::LifKeyHandle::kLifId) {
        HAL_TRACE_ERR("{}: Lif didn't have Id. Err: {}",
                      __FUNCTION__, HAL_RET_INVALID_ARG);
        rsp->set_api_status(types::API_STATUS_LIF_ID_INVALID);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // check if lif exists already, and reject if one is found
    if (find_lif_by_id(spec.key_or_handle().lif_id())) {
        HAL_TRACE_ERR("{}:failed to create a lif, "
                      "lif {} exists already", __FUNCTION__,
                      spec.key_or_handle().lif_id());
        rsp->set_api_status(types::API_STATUS_EXISTS_ALREADY);
        return HAL_RET_ENTRY_EXISTS;
    }

    // Check if RSS key is correct size
    if (spec.rss().type() &&
        (spec.rss().key().size() != ETH_RSS_KEY_LENGTH ||
         spec.rss().indir().size() !=  ETH_RSS_INDIR_LENGTH)) {
        HAL_TRACE_ERR("{}: Invalid Args: key size {} indir size {}",
                      __FUNCTION__,
                      spec.rss().key().size(),
                      spec.rss().indir().size());
        return HAL_RET_INVALID_ARG;
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// PD Call to allocate PD resources and HW programming
//------------------------------------------------------------------------------
hal_ret_t
lif_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                  ret            = HAL_RET_OK;
    sdk_ret_t                  sret           = SDK_RET_OK;
    dllist_ctxt_t              *lnode         = NULL;
    dhl_entry_t                *dhl_entry     = NULL;
    LifSpec                    *spec          = NULL;
    LifResponse                *rsp           = NULL;
    lif_t                      *lif           = NULL;
    lif_hal_info_t             *lif_hal_info, lif_info;
    lif_create_app_ctxt_t      *app_ctxt      = NULL;
    uint32_t                   hw_lif_id      = 0;
    bool                       dont_zero_qstate_mem = false;
    std::unique_ptr<uint8_t[]> buf;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}: invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode     = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt  = (lif_create_app_ctxt_t *)cfg_ctxt->app_ctxt;

    lif          = (lif_t *)dhl_entry->obj;
    lif_hal_info = app_ctxt->lif_info;
    spec         = app_ctxt->spec;
    rsp          = app_ctxt->rsp;

    HAL_TRACE_DEBUG("{}:create add cb {}",
                    __FUNCTION__, lif->lif_id);

    // For SWM lif, no need of qstate
    if (lif->type == types::LIF_TYPE_SWM) {
        if (lif_hal_info && lif_hal_info->with_hw_lif_id) {
            lif_info.with_hw_lif_id = 1;
            lif_info.hw_lif_id = lif_hal_info->hw_lif_id;
        } else {
            // Only for MBT
            // DOL
            sret = lif_manager()->alloc_id(&hw_lif_id, 1);
            if (sret != SDK_RET_OK) {
                HAL_TRACE_ERR("Failed to allocate lif, hw_lif_id : {}", hw_lif_id);
                ret = HAL_RET_NO_RESOURCE;
                goto end;
            }
            lif_info.with_hw_lif_id = 1;
            lif_info.hw_lif_id = hw_lif_id;
        }
    } else {
        /*
         * Who creates LIFs:
         * 1. DOL
         *   - Qstate programming has to be done in HAL.qstate_pgm_in_hal:true
         * 2. NIC Mgr (In HW & HAPs)
         *    NIC Mgr allocates hw_lif_id and does qstate programming.
         *    Only for those lifs, qstate_pgm_in_hal:false.
         * 3. HAL
         *    Service LIFs & CPU LIF are created inside HAL, so qsate programmins
         *    is also done in HAL. qstate_pgm_in_hal:true
         *    Service LIFs are reserved during init
         */
        // allocate a hw lif id
        if (lif_hal_info && lif_hal_info->with_hw_lif_id) {
            hw_lif_id = lif_hal_info->hw_lif_id;
            // Check that only service lifs are already allocated
            if (hw_lif_id >= HAL_LIF_CPU && hw_lif_id <= HAL_LIF_ID_SVC_LIF_MAX) {
                lif->qstate_pgm_in_hal = true;
            } else {
                // Nicmgr LIFs
                sret = lif_manager()->reserve_id(hw_lif_id, 1);
                if (sret != SDK_RET_OK) {
                    HAL_TRACE_ERR("Failed to mark lif allocated, info hw_lif_id : {} "
                                  "hw_lif_id: {}", lif_hal_info->hw_lif_id, hw_lif_id);
                    ret = HAL_RET_ERR;
                    goto end;
                }
            }
            memcpy(&lif_info, lif_hal_info, sizeof(lif_info));
            dont_zero_qstate_mem = lif_hal_info->dont_zero_qstate_mem;
        } else {
            // DOL
            sret = lif_manager()->alloc_id(&hw_lif_id, 1);
            if (sret != SDK_RET_OK) {
                HAL_TRACE_ERR("Failed to allocate lif, hw_lif_id : {}", hw_lif_id);
                ret = HAL_RET_NO_RESOURCE;
                goto end;
            }
            lif->qstate_pgm_in_hal = true;
            lif_info.with_hw_lif_id = 1;
            lif_info.hw_lif_id = hw_lif_id;
        }
        app_ctxt->hw_lif_id = hw_lif_id;

        if (spec->lif_qstate_map_size()) {
            // init queue state map
            ret = lif_qstate_map_init(*spec, hw_lif_id, lif, dont_zero_qstate_mem);
            if (ret != HAL_RET_OK) {
                goto end;
            }
            if (spec->lif_qstate_size()) {
                // init queues
                ret = lif_qstate_init(*spec, hw_lif_id, lif);
                if (ret != HAL_RET_OK) {
                    HAL_TRACE_ERR("Failed to do lif qstate: ret: {}", ret);
                    goto end;
                }
            }
            lif->qstate_init_done = true;
        }
    }

    // create the lif now
    ret = lif_fwd_create(*spec, rsp, lif, &lif_info);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("P4 Lif create failure, err : {}", ret);
        ret = HAL_RET_ERR;
        goto end;
    }

    // For NICmgr created lifs, hal doesn't have to do this init.
    if (!is_platform_type_haps() && !is_platform_type_hw() &&
        g_hal_cfg.qemu != true) {
        // For rdma enabled Lifs, call RDMA specific init (allocates KT, PT, etc)
        if (lif->enable_rdma) {
            ret = rdma_lif_init(*spec, hw_lif_id);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("P4 Lif create failure, err : {}", ret);
                ret = HAL_RET_ERR;
                goto end;
            }
        }
    }

    // For NICmgr created lifs, hal doesn't have to do this init.
    if (!is_platform_type_haps() && !is_platform_type_hw() &&
        g_hal_cfg.qemu != true) {
        // For nvme enabled Lifs, call RDMA specific init (allocates KT, PT, etc)
        if (lif->enable_nvme) {
            ret = nvme_lif_init(*spec, hw_lif_id);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("NVME Lif create failure, err : {}", ret);
                ret = HAL_RET_ERR;
                goto end;
            }
        }
    }

    if (lif->type == types::LIF_TYPE_SWM) {
        if (lif_bcast_filter_any_set(lif)) {
            lif_bcast_filter_install(lif);
        }
        if (lif_mcast_filter_any_set(lif)) {
            lif_mcast_filter_install(lif);
        }
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// 1. Update PI DBs as lif_create_add_cb() was a success
//      a. Add to lif id hash table
//------------------------------------------------------------------------------
hal_ret_t
lif_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t     ret        = HAL_RET_OK;
    dllist_ctxt_t *lnode     = NULL;
    dhl_entry_t   *dhl_entry = NULL;
    lif_t         *lif       = NULL;
    hal_handle_t  hal_handle = 0;
    qos_class_t   *rx_qos_class;
    qos_class_t   *tx_qos_class;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // assumption is there is only one element in the list
    lnode     = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    lif        = (lif_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("{}:create commit cb {}",
                    __FUNCTION__, lif->lif_id);

    // 1. a. Add to lif id hash table
    ret = lif_add_to_db(lif, hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to add lif {} to db, err : {}",
                __FUNCTION__, lif->lif_id, ret);
        goto end;
    }

    // TODO: Increment the ref counts of dependent objects
    rx_qos_class = find_qos_class_by_handle(lif->qos_info.rx_qos_class_handle);
    if (rx_qos_class) {
        ret = qos_class_add_lif_rx(rx_qos_class, lif);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to add rx qos class ref. from lif");
            goto end;
        }
    }

    tx_qos_class = find_qos_class_by_handle(lif->qos_info.tx_qos_class_handle);
    if (tx_qos_class) {
        ret = qos_class_add_lif_tx(tx_qos_class, lif);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to add tx qos class ref. from lif");
            goto end;
        }
    }

    // Set mnic internal management
    if (lif->type == types::LIF_TYPE_MNIC_INTERNAL_MANAGEMENT) {
        HAL_TRACE_DEBUG("Setting MNIC internal management id to: {}",
                        lif->lif_id);
        g_hal_state->set_mnic_internal_mgmt_lif_id(lif->lif_id);
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// lif_create_add_cb was a failure
// 1. call delete to PD
//      a. Deprogram HW
//      b. Clean up resources
//      c. Free PD object
// 2. Remove object from hal_handle id based hash table in infra
// 3. Free PI lif
//------------------------------------------------------------------------------
hal_ret_t
lif_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                ret         = HAL_RET_OK;
    dllist_ctxt_t            *lnode      = NULL;
    dhl_entry_t              *dhl_entry  = NULL;
    lif_t                    *lif        = NULL;
    hal_handle_t             hal_handle  = 0;
    pd::pd_lif_delete_args_t pd_lif_args = { 0 };
    pd::pd_func_args_t          pd_func_args = {0};

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode     = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    lif        = (lif_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("{}:create abort cb {}",
                    __FUNCTION__, lif->lif_id);

    // 1. delete call to PD
    if (lif->pd_lif) {
        pd::pd_lif_delete_args_init(&pd_lif_args);
        pd_lif_args.lif = lif;
        pd_func_args.pd_lif_delete = &pd_lif_args;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_LIF_DELETE, &pd_func_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("{}:failed to delete lif pd, err : {}",
                          __FUNCTION__, ret);
        }
    }

    // 2. remove object from hal_handle id based hash table in infra
    hal_handle_free(hal_handle);

    // 3. Free PI lif
    lif_cleanup(lif);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// Dummy create cleanup callback
// ----------------------------------------------------------------------------
hal_ret_t
lif_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// Converts hal_ret_t to API status
//------------------------------------------------------------------------------
hal_ret_t
lif_prepare_rsp (LifResponse *rsp, hal_ret_t ret, hal_handle_t hal_handle)
{
    if (ret == HAL_RET_OK) {
        rsp->mutable_status()->set_lif_handle(hal_handle);
    }

    rsp->set_api_status(hal_prepare_rsp(ret));

    return HAL_RET_OK;
}

hal_ret_t
lif_populate_filters (lif_t *lif, LifSpec& spec)
{
    lif->bcast_filter.arp = spec.bcast_pkt_filter().arp();
    lif->bcast_filter.dhcp_client = spec.bcast_pkt_filter().dhcp_client();
    lif->bcast_filter.dhcp_server = spec.bcast_pkt_filter().dhcp_server();
    lif->bcast_filter.netbios = spec.bcast_pkt_filter().netbios();

    lif->mcast_filter.ipv6_neigh_adv = spec.mcast_pkt_filter().ipv6_neigh_adv();
    lif->mcast_filter.ipv6_router_adv = spec.mcast_pkt_filter().ipv6_router_adv();
    lif->mcast_filter.dhcpv6_relay = spec.mcast_pkt_filter().dhcpv6_relay();
    lif->mcast_filter.dhcpv6_mcast = spec.mcast_pkt_filter().dhcpv6_mcast();
    lif->mcast_filter.ipv6_mld = spec.mcast_pkt_filter().ipv6_mld();
    lif->mcast_filter.ipv6_neigh_sol = spec.mcast_pkt_filter().ipv6_neigh_sol();
}

bool
lif_bcast_filter_any_set (lif_t *lif) {
    return (lif->bcast_filter.arp ||
            lif->bcast_filter.dhcp_client ||
            lif->bcast_filter.dhcp_server ||
            lif->bcast_filter.netbios);
}

bool
lif_mcast_filter_any_set (lif_t *lif) {
    return (lif->mcast_filter.ipv6_neigh_adv ||
            lif->mcast_filter.ipv6_router_adv ||
            lif->mcast_filter.dhcpv6_relay ||
            lif->mcast_filter.dhcpv6_mcast ||
            lif->mcast_filter.ipv6_mld ||
            lif->mcast_filter.ipv6_neigh_sol);
}

hal_ret_t
lif_bcast_filter_install (lif_t *lif) {
    if (lif->bcast_filter.arp) {
        acl_install_bcast_arp();
    }
    if (lif->bcast_filter.dhcp_client) {
        acl_install_bcast_dhcp_client();
    }
    if (lif->bcast_filter.dhcp_server) {
        acl_install_bcast_dhcp_server();
    }
    if (lif->bcast_filter.netbios) {
        acl_install_bcast_netbios();
    }
    acl_install_bcast_all();

    return HAL_RET_OK;
}

hal_ret_t
lif_bcast_filter_uninstall (lif_t *lif) {
    if (lif->bcast_filter.arp) {
        acl_uninstall_bcast_arp();
    }
    if (lif->bcast_filter.dhcp_client) {
        acl_uninstall_bcast_dhcp_client();
    }
    if (lif->bcast_filter.dhcp_server) {
        acl_uninstall_bcast_dhcp_server();
    }
    if (lif->bcast_filter.netbios) {
        acl_uninstall_bcast_netbios();
    }
    acl_uninstall_bcast_all();

    return HAL_RET_OK;
}

hal_ret_t
lif_mcast_filter_install (lif_t *lif) {
    if (lif->mcast_filter.ipv6_neigh_adv) {
        acl_install_mcast_ipv6_nadv();
    }
    if (lif->mcast_filter.ipv6_router_adv) {
        acl_install_mcast_ipv6_radv();
    }
    if (lif->mcast_filter.dhcpv6_relay) {
        acl_install_mcast_dhcpv6_relay();
    } 
    if (lif->mcast_filter.dhcpv6_mcast) {
        acl_install_mcast_dhcpv6_mcast();
    }
    if (lif->mcast_filter.ipv6_mld) {
        acl_install_mcast_mld();
    }
    if (lif->mcast_filter.ipv6_neigh_sol) {
        acl_install_mcast_ipv6_nsol();
    }
    acl_install_mcast_all();

    return HAL_RET_OK;
}

hal_ret_t
lif_mcast_filter_uninstall (lif_t *lif) {
    if (lif->mcast_filter.ipv6_neigh_adv) {
        acl_uninstall_mcast_ipv6_nadv();
    }
    if (lif->mcast_filter.ipv6_router_adv) {
        acl_uninstall_mcast_ipv6_radv();
    }
    if (lif->mcast_filter.dhcpv6_relay) {
        acl_uninstall_mcast_dhcpv6_relay();
    } 
    if (lif->mcast_filter.dhcpv6_mcast) {
        acl_uninstall_mcast_dhcpv6_mcast();
    }
    if (lif->mcast_filter.ipv6_mld) {
        acl_uninstall_mcast_mld();
    }
    if (lif->mcast_filter.ipv6_neigh_sol) {
        acl_uninstall_mcast_ipv6_nsol();
    }
    acl_uninstall_mcast_all();

    return HAL_RET_OK;
}
//------------------------------------------------------------------------------
// process lif create
//------------------------------------------------------------------------------
hal_ret_t
lif_create (LifSpec& spec, LifResponse *rsp, lif_hal_info_t *lif_hal_info)
{
    hal_ret_t                  ret        = HAL_RET_OK;
    lif_t                      *lif       = NULL;
    uint32_t                   hw_lif_id  = 0;
    std::unique_ptr<uint8_t[]> buf;
    hal_handle_t               hal_handle = HAL_HANDLE_INVALID;
    lif_create_app_ctxt_t      app_ctxt   = { 0 };
    dhl_entry_t                dhl_entry  = { 0 };
    cfg_op_ctxt_t              cfg_ctxt   = { 0 };
    qos_class_t                *qos_class, *control_qos_class;
    uint32_t                   cosA = 0, cosB = 0, control_cos = 0;
    pd::pd_qos_class_get_admin_cos_args_t args = {0};
    pd::pd_func_args_t          pd_func_args = {0};
    lif_hal_info_t             proto_hal_info = {0};
    pd::pd_qos_class_get_qos_class_id_args_t q_args;
    void                      *lifupd_timer = NULL;

    proto_msg_dump(spec);

    // validate the request message
    ret = validate_lif_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:validation failed, ret : {}",
                __FUNCTION__, ret);
        return ret;
    }

    // allocate and initialize interface instance
    lif = lif_alloc_init();
    if (lif == NULL) {
        HAL_TRACE_ERR("{}:out of memory. err: {}",
                      __FUNCTION__, HAL_RET_OOM);
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        return HAL_RET_OOM;
    }
    auto uplink_if = if_lookup_key_or_handle(spec.pinned_uplink_if_key_handle());
    auto oob_uplink_if = if_lookup_key_or_handle(spec.swm_oob());

    // consume the config
    lif->lif_id              = spec.key_or_handle().lif_id();
    if (spec.name().size() + 1 > LIF_NAME_LEN) {
        HAL_TRACE_ERR("LIF's name: {} len:{} is greater than static len.",
                      spec.name(), spec.name().size() + 1);
    }
    strcpy(lif->name, spec.name().c_str());
    lif->type                = spec.type();
    lif->admin_status        = spec.admin_status();
    lif->hal_handle          = hal_alloc_handle();
    lif->vlan_strip_en       = spec.vlan_strip_en();
    lif->vlan_insert_en      = spec.vlan_insert_en();
    lif->is_management       = spec.is_management();
    lif->rdma_sniff_en       = spec.rdma_sniff_en();
    lif->pinned_uplink       = uplink_if ? uplink_if->hal_handle :
                               HAL_HANDLE_INVALID;
    lif->oob_uplink          = oob_uplink_if ? oob_uplink_if->hal_handle :
                               HAL_HANDLE_INVALID;
    lif->packet_filters.receive_broadcast = spec.packet_filter().
                                            receive_broadcast();
    lif->packet_filters.receive_promiscuous = spec.packet_filter().
                                              receive_promiscuous();
    lif->packet_filters.receive_all_multicast = spec.packet_filter().
                                                receive_all_multicast();
    lif_populate_filters(lif, spec);

    // RSS configuration
    lif->rss.type = spec.rss().type();
    memcpy(&lif->rss.key, (uint8_t *)spec.rss().key().c_str(),
           sizeof(lif->rss.key));
    memcpy(&lif->rss.indir, (uint8_t *)spec.rss().indir().c_str(),
           sizeof(lif->rss.indir));

    //lif->allmulti = spec.allmulti();
    lif->enable_rdma = spec.enable_rdma();
    lif->rdma_max_keys = spec.rdma_max_keys();
    lif->rdma_max_pt_entries = spec.rdma_max_pt_entries();

    lif->enable_nvme = spec.enable_nvme();
    lif->nvme_max_ns = spec.nvme_max_ns();
    lif->nvme_max_sess = spec.nvme_max_sess();

    if (spec.has_rx_policer()) {
        qos_policer_update_from_spec(spec.rx_policer(), &lif->qos_info.rx_policer);
    }

    if (spec.has_tx_policer()) {
        qos_policer_update_from_spec(spec.tx_policer(), &lif->qos_info.tx_policer);
    }

    if (spec.has_tx_qos_class()) {
        ret = find_qos_cos_info_from_spec(spec.tx_qos_class(), lif->pinned_uplink, &cosB);
        if (ret != HAL_RET_OK) {
            lif_prepare_rsp(rsp, ret, HAL_HANDLE_INVALID);
            return ret;
        }
        qos_class = find_qos_class_by_key_handle(spec.tx_qos_class());
        lif->qos_info.tx_qos_class_handle = qos_class ? qos_class->hal_handle : HAL_HANDLE_INVALID;
    }

    if (lif->type != types::LIF_TYPE_SWM) {
        // cosA of a LIF will always be the ADMIN-COS.
        pd_func_args.pd_qos_class_get_admin_cos = &args;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_QOS_GET_ADMIN_COS, &pd_func_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_DEBUG("pi-lif:{}:failed to fetch admin cos of lif {}, err : {}",
                            __FUNCTION__, lif->lif_id, ret);
        }

        // Fetch cos for control-qos-class
        if ((control_qos_class = find_qos_class_by_group(hal::QOS_GROUP_CONTROL)) != NULL) {
            q_args.qos_class= control_qos_class;
            q_args.dest_if = uplink_if;
            q_args.qos_class_id = &control_cos;
            pd_func_args.pd_qos_class_get_qos_class_id = &q_args;
            ret = pd::hal_pd_call(pd::PD_FUNC_ID_GET_QOS_CLASSID, &pd_func_args);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Error deriving qos-class-id for Qos class "
                              "{} ret {}",
                              control_qos_class->key, ret);
                return ret;
            }
        } else {
            HAL_TRACE_DEBUG("pi-lif:{}:failed to fetch control cos of lif {}, err : {}",
                            __FUNCTION__, lif->lif_id, ret);
        }

        cosA = args.cos;

        HAL_TRACE_DEBUG("cosA: {}, cosB: {}, control_cos: {}", cosA, cosB, control_cos);

        lif->qos_info.cos_bmp =  0xffff; //Burn scheduler bit for all coses for each (lif,queue).
        lif->qos_info.coses   =  (cosA & 0x0f) | ((cosB << 4) & 0xf0);

        if (spec.has_rx_qos_class()) {
            qos_class = find_qos_class_by_key_handle(spec.rx_qos_class());
            lif->qos_info.rx_qos_class_handle = qos_class ? qos_class->hal_handle : HAL_HANDLE_INVALID;
        }
    }

    // allocate hal handle id
    hal_handle = hal_handle_alloc(HAL_OBJ_ID_LIF);
    if (hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("{}:failed to alloc handle {}",
                      __FUNCTION__, lif->lif_id);
        rsp->set_api_status(types::API_STATUS_HANDLE_INVALID);
        lif_cleanup(lif);
        return HAL_RET_HANDLE_INVALID;
    }

    // Take hal_info from proto only if its not passed to this function
    if (!lif_hal_info) {
        if (spec.hw_lif_id() != 0) {
            HAL_TRACE_DEBUG("Creating lif with hw_lif_id passed as: {}",
                            spec.hw_lif_id());
            proto_hal_info.with_hw_lif_id = true;
            proto_hal_info.hw_lif_id = spec.hw_lif_id();
            proto_hal_info.dont_zero_qstate_mem = false;    // default is false.
            lif_hal_info = &proto_hal_info;
        }
    }

    // form ctxt and call infra add
    app_ctxt.lif_info = lif_hal_info;
    app_ctxt.spec     = &spec;
    app_ctxt.rsp      = rsp;
    dhl_entry.handle  = hal_handle;
    dhl_entry.obj     = lif;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(hal_handle, &cfg_ctxt,
                             lif_create_add_cb,
                             lif_create_commit_cb,
                             lif_create_abort_cb,
                             lif_create_cleanup_cb);

    if (ret != HAL_RET_OK && lif != NULL) {
        HAL_TRACE_ERR("error in creating lif: {} ", lif->lif_id);
        // if there is an error, lif will be freed in abort cb
        lif_prepare_rsp(rsp, ret, HAL_HANDLE_INVALID);
        return ret;
    }

    hw_lif_id = app_ctxt.hw_lif_id; // populated in lif_create_add_cb
    lif_prepare_rsp(rsp, ret, hal_handle);
    rsp->mutable_status()->set_hw_lif_id(hw_lif_id);
    rsp->mutable_status()->set_lif_status(lif->admin_status);

    // Return LifQstate addresses
    intf::LifQState *entry;
    for (int i = 0; i < spec.lif_qstate_map_size(); i++) {
        const auto &ent = spec.lif_qstate_map(i);
        entry = rsp->add_qstate();
        entry->set_type_num(ent.type_num());
        // entry->set_addr(lif_manager()->GetLIFQStateAddr(hw_lif_id, ent.type_num(), 0));
        entry->set_addr(lif_manager()->get_lif_qstate_addr(hw_lif_id, ent.type_num(), 0));
    }

#ifdef __x86_64__
    // Return LIF RDMA data for RDMA enabled lifs
    if (lif->enable_rdma) {
        rsp->set_rdma_data_valid(true);
        rsp->mutable_rdma_data()->set_pt_base_addr(rdma_lif_pt_base_addr(hw_lif_id));
        rsp->mutable_rdma_data()->set_kt_base_addr(rdma_lif_kt_base_addr(hw_lif_id));
        rsp->mutable_rdma_data()->set_dcqcn_profile_base_addr(rdma_lif_dcqcn_profile_base_addr(hw_lif_id));
        rsp->mutable_rdma_data()->set_at_base_addr(rdma_lif_at_base_addr(hw_lif_id));
        rsp->mutable_rdma_data()->set_barmap_base_addr(rdma_lif_barmap_base_addr(hw_lif_id));
    }
#endif

    // Add to map of lif name and PI ID
    g_hal_state->lif_name_id_map_insert(lif->name, lif->lif_id);

    if (lif->type != types::LIF_TYPE_NONE) {
        // Send updates to Agent
        lifupd_timer = sdk::lib::timer_schedule(HAL_TIMER_ID_STREAM_LIF_UPDATE,
                                        LIF_STREAM_TIMER,
                                        (reinterpret_cast<void *>(lif->lif_id)),
                                        hal_stream_lif_updates, false);

        if (!lifupd_timer) {
            HAL_TRACE_ERR("Failed to schedule the timer for the ep");
            ret = HAL_RET_ERR;
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
// validate lif update request
//------------------------------------------------------------------------------
hal_ret_t
validate_lif_update (LifSpec& spec, LifResponse *rsp)
{
    hal_ret_t ret = HAL_RET_OK;

    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("{}:spec has no key or handle", __FUNCTION__);
        ret =  HAL_RET_INVALID_ARG;
    }

    // Check if RSS key is correct size
    if (spec.rss().type() &&
        (spec.rss().key().size() != ETH_RSS_KEY_LENGTH ||
         spec.rss().indir().size() != ETH_RSS_INDIR_LENGTH)) {
        HAL_TRACE_ERR("{}: Invalid Args: key size {} indir size {}",
                      __FUNCTION__,
                      spec.rss().key().size(),
                      spec.rss().indir().size());
        return HAL_RET_INVALID_ARG;
    }

    return ret;
}

hal_ret_t
lif_bcast_filter_update (lif_t *lif, lif_update_app_ctxt_t *lif_upd)
{
    hal_ret_t                   ret = HAL_RET_OK;
    hal_handle_id_list_entry_t  *entry = NULL;
    dllist_ctxt_t               *curr  = NULL, *next = NULL;
    if_t                        *hal_if = NULL;

    dllist_for_each_safe(curr, next, &lif->if_list_head) {
        entry = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
        hal_if = find_if_by_handle(entry->handle_id);
        if (!hal_if) {
            HAL_TRACE_ERR("{}:unable to find if with handle:{}",
                          __FUNCTION__, entry->handle_id);
            continue;
        }

        ret = if_update_classic_oif_lists(hal_if, lif_upd);
    }
    return ret;
}
//------------------------------------------------------------------------------
// This is the first call back infra does for update.
// 1. PD Call to update PD
// 2. Update Other objects to update with new nwsec profile
//------------------------------------------------------------------------------
hal_ret_t
lif_update_upd_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t             ret        = HAL_RET_OK;
    pd::pd_lif_update_args_t args       = { 0 };
    pd::pd_func_args_t    pd_func_args = {0};
    dllist_ctxt_t         *lnode     = NULL;
    dhl_entry_t           *dhl_entry = NULL;
    lif_t                 *lif       = NULL;
    lif_t                 *lif_clone = NULL;
    lif_update_app_ctxt_t *app_ctxt  = NULL;
    uint32_t              hw_lif_id;
    LifSpec               *spec = NULL;
    bool                  bcast_old = false, bcast_new = false;
    bool                  mcast_old = false, mcast_new = false;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-lif{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode     = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt  = (lif_update_app_ctxt_t *)cfg_ctxt->app_ctxt;
    spec = app_ctxt->spec;

    lif = (lif_t *)dhl_entry->obj;
    lif_make_clone(lif, (lif_t **)&dhl_entry->cloned_obj, *spec);
    lif_clone = (lif_t *)dhl_entry->cloned_obj;

    // Rule of thumb is if anything which clone has to be updated doesnt result
    // in change of original, it can be done here.

    // Update PI clone
    if (app_ctxt->vlan_strip_en_changed) {
        lif_clone->vlan_strip_en = spec->vlan_strip_en();
    }
    if (app_ctxt->rdma_sniff_en_changed) {
        lif_clone->rdma_sniff_en = spec->rdma_sniff_en();
    }
    if (app_ctxt->vlan_insert_en_changed) {
        lif_clone->vlan_insert_en = app_ctxt->vlan_insert_en;
    }
    if (app_ctxt->pinned_uplink_changed) {
        lif_clone->pinned_uplink = app_ctxt->new_pinned_uplink;
    }

    if (app_ctxt->pkt_filter_prom_changed) {
        lif_clone->packet_filters.receive_promiscuous =
            app_ctxt->receive_promiscous;
    }

    if (app_ctxt->pkt_filter_bcast_changed) {
        lif_clone->packet_filters.receive_broadcast =
            app_ctxt->receive_broadcast;
    }

    if (app_ctxt->pkt_filter_allmc_changed) {
        lif_clone->packet_filters.receive_all_multicast =
            app_ctxt->receive_all_multicast;
    }

    if (app_ctxt->pkt_filter_bcast_changed ||
        app_ctxt->pkt_filter_allmc_changed ||
        app_ctxt->pkt_filter_prom_changed) {
        lif_bcast_filter_update(lif, app_ctxt);
    }

    if (app_ctxt->rss_config_changed) {
        lif_clone->rss.type = spec->rss().type();
        memcpy(&lif_clone->rss.key, (uint8_t *)spec->rss().key().c_str(),
               sizeof(lif_clone->rss.key));
        memcpy(&lif_clone->rss.indir, (uint8_t *)spec->rss().indir().c_str(),
               sizeof(lif_clone->rss.indir));
    }

    if (app_ctxt->name_changed) {
        strcpy(lif_clone->name, spec->name().c_str());
    }

    if (app_ctxt->status_changed) {
        lif_clone->admin_status = spec->admin_status();
    }

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_lif_update_args_init(&args);
    args.lif                   = lif;
    args.lif_clone             = lif_clone;
    args.vlan_strip_en_changed = app_ctxt->vlan_strip_en_changed;
    args.vlan_strip_en         = app_ctxt->vlan_strip_en;
    args.qstate_map_init_set   = app_ctxt->qstate_map_init_set;
    args.rx_policer_changed    = app_ctxt->rx_policer_changed;
    args.tx_policer_changed    = app_ctxt->tx_policer_changed;
    args.pkt_filter_prom_changed = app_ctxt->pkt_filter_prom_changed;
    args.receive_promiscous    = app_ctxt->receive_promiscous;
    args.rdma_sniff_en_changed = app_ctxt->rdma_sniff_en_changed;
    args.rdma_sniff_en         = app_ctxt->rdma_sniff_en;

    hw_lif_id = lif_hw_lif_id_get(lif);

    if (app_ctxt->qstate_map_init_set) {
        // init queue state map
        ret = lif_qstate_map_init(*(app_ctxt->spec), hw_lif_id, lif, false);
        if (ret != HAL_RET_OK) {
            goto end;
        }
        if (app_ctxt->spec->lif_qstate_size()) {
            // init queues
            ret = lif_qstate_init(*(app_ctxt->spec), hw_lif_id, lif);
            if (ret != HAL_RET_OK) {
                goto end;
            }
        }
        lif->qstate_init_done = true;
    }
    args.rss_config_changed = app_ctxt->rss_config_changed;

    pd_func_args.pd_lif_update = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_LIF_UPDATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to delete lif pd, err : {}",
                      __FUNCTION__, ret);
    }

    if (app_ctxt->vlan_strip_en_changed || app_ctxt->vlan_insert_en_changed
        || app_ctxt->pinned_uplink_changed) {
        // Triggers reprogramming of output mapping table for enicifs and uplinks
        ret = lif_update_trigger_if(lif, app_ctxt->vlan_strip_en_changed,
                                    app_ctxt->vlan_strip_en,
                                    app_ctxt->vlan_insert_en_changed,
                                    app_ctxt->vlan_insert_en,
                                    app_ctxt->pinned_uplink_changed,
                                    app_ctxt->new_pinned_uplink);
    }

    lif_populate_filters(lif_clone, *spec);
    if (app_ctxt->bcast_filters_changed) {
        bcast_old = lif_bcast_filter_any_set(lif);
        bcast_new = lif_bcast_filter_any_set(lif_clone);
        if (bcast_old != bcast_new) {
            if (bcast_new) {
                lif_bcast_filter_install(lif_clone);
            } else {
                lif_bcast_filter_uninstall(lif);
            }
        } else {
            if (bcast_new) {
                lif_bcast_filter_uninstall(lif);
                lif_bcast_filter_install(lif_clone);
            }
        }
    }
    if (app_ctxt->mcast_filters_changed) {
        mcast_old = lif_mcast_filter_any_set(lif);
        mcast_new = lif_mcast_filter_any_set(lif_clone);
        if (mcast_old != mcast_new) {
            if (mcast_new) {
                lif_mcast_filter_install(lif_clone);
            } else {
                lif_mcast_filter_uninstall(lif);
            }
        } else {
            if (mcast_new) {
                lif_mcast_filter_uninstall(lif);
                lif_mcast_filter_install(lif_clone);
            }
        }
    }
end:
    return ret;
}

//------------------------------------------------------------------------------
// Make a clone
// - Both PI and PD objects cloned.
//------------------------------------------------------------------------------
hal_ret_t
lif_make_clone (lif_t *lif, lif_t **lif_clone, LifSpec& spec)
{
    pd::pd_lif_make_clone_args_t args;
    pd::pd_func_args_t          pd_func_args = {0};

    // Just alloc, no need to init. We dont want new block lists
    *lif_clone = lif_alloc();
    memcpy(*lif_clone, lif, sizeof(lif_t));

    // After clone always reset lists
    dllist_reset(&(*lif_clone)->if_list_head);

    args.lif = lif;
    args.clone = *lif_clone;
    // pd::pd_lif_make_clone(lif, *lif_clone);
    pd_func_args.pd_lif_make_clone = &args;
    pd::hal_pd_call(pd::PD_FUNC_ID_LIF_MAKE_CLONE, &pd_func_args);


    // Update the clone with the new values
    if (spec.has_rx_policer()) {
        qos_policer_update_from_spec(spec.rx_policer(), &(*lif_clone)->qos_info.rx_policer);
    }

    if (spec.has_tx_policer()) {
        qos_policer_update_from_spec(spec.tx_policer(), &(*lif_clone)->qos_info.tx_policer);
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// After all hw programming is done
//  1. Free original PI & PD lif.
// Note: Infra make clone as original by replacing original pointer by clone.
//------------------------------------------------------------------------------
hal_ret_t
lif_update_commit_cb(cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t             ret         = HAL_RET_OK;
    pd::pd_lif_mem_free_args_t     pd_lif_args = { 0 };
    dllist_ctxt_t         *lnode      = NULL;
    dhl_entry_t           *dhl_entry  = NULL;
    lif_t                 *lif        = NULL, *lif_clone = NULL;
    pd::pd_func_args_t          pd_func_args = {0};
    lif_update_app_ctxt_t *app_ctxt   = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode     = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt  = (lif_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    lif       = (lif_t *)dhl_entry->obj;
    lif_clone = (lif_t *)dhl_entry->cloned_obj;

#if 0
    // Update PI clone
    if (app_ctxt->vlan_strip_en_changed) {
        lif_clone->vlan_strip_en = app_ctxt->vlan_strip_en;
    }
    if (app_ctxt->vlan_insert_en_changed) {
        lif_clone->vlan_insert_en = app_ctxt->vlan_insert_en;
    }
    if (app_ctxt->pinned_uplink_changed) {
        lif_clone->pinned_uplink = app_ctxt->new_pinned_uplink;
    }
#endif

    dllist_move(&lif_clone->if_list_head, &lif->if_list_head);

    HAL_TRACE_DEBUG("Printing IFs from clone:");
    lif_print_ifs(lif_clone);

    if (app_ctxt->name_changed) {
        g_hal_state->lif_name_id_map_delete(lif->name);
        g_hal_state->lif_name_id_map_insert(lif_clone->name, lif_clone->lif_id);
    }

    // TBD trigger a LIF GET here
    if (app_ctxt->status_changed) {
        lif_clone->admin_status = lif->admin_status;
    }

    // Free PD
    pd::pd_lif_mem_free_args_init(&pd_lif_args);
    pd_lif_args.lif = lif;
    pd_func_args.pd_lif_mem_free = &pd_lif_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_LIF_MEM_FREE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to delete lif pd, err : {}", ret);
    }

    // Free PI
    lif_free(lif);
end:
    return ret;
}

//------------------------------------------------------------------------------
// Update didnt go through.
//  1. Kill the clones
//------------------------------------------------------------------------------
hal_ret_t
lif_update_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret         = HAL_RET_OK;
    pd::pd_lif_mem_free_args_t  pd_lif_args = { 0 };
    pd::pd_func_args_t          pd_func_args = {0};
    dllist_ctxt_t               *lnode      = NULL;
    dhl_entry_t                 *dhl_entry  = NULL;
    lif_t                       *lif        = NULL;
    // lif_update_app_ctxt_t *app_ctxt   = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-lif{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode       = cfg_ctxt->dhl.next;
    dhl_entry   = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    // app_ctxt = (lif_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    lif = (lif_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("{}:update commit cb {}",
                    __FUNCTION__, lif->lif_id);

    // Free PD
    pd::pd_lif_mem_free_args_init(&pd_lif_args);
    pd_lif_args.lif = lif;
    pd_func_args.pd_lif_mem_free = &pd_lif_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_LIF_MEM_FREE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to delete lif pd, err : {}",
                      __FUNCTION__, ret);
    }

    // Free PI
    lif_free(lif);
end:

    return ret;
}

hal_ret_t
lif_update_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Lookup lif from key or handle
//------------------------------------------------------------------------------
lif_t *
lif_lookup_key_or_handle (const LifKeyHandle& kh)
{
    lif_t *lif = NULL;

    if (kh.key_or_handle_case() == LifKeyHandle::kLifId) {
        lif = find_lif_by_id(kh.lif_id());
    } else if (kh.key_or_handle_case() == LifKeyHandle::kLifHandle) {
        lif = find_lif_by_handle(kh.lif_handle());
    }

    return lif;
}

//------------------------------------------------------------------------------
// Handle update. Checks what changed
//------------------------------------------------------------------------------
static inline hal_ret_t
lif_handle_update (lif_update_app_ctxt_t *app_ctxt, lif_t *lif)
{
    hal_ret_t ret   = HAL_RET_OK;
    LifSpec   *spec = app_ctxt->spec;
    int cmp_key, cmp_indir;
    policer_t new_rx_policer = { POLICER_TYPE_PPS };
    policer_t new_tx_policer = { POLICER_TYPE_PPS };

    app_ctxt->new_pinned_uplink = HAL_HANDLE_INVALID;

    // Handle vlan_strip_en change
    if (lif->vlan_strip_en != spec->vlan_strip_en()) {
        HAL_TRACE_DEBUG("{}:vlan_strip_en changed {} => {}",
                        __FUNCTION__, lif->vlan_strip_en, spec->vlan_strip_en());
        app_ctxt->vlan_strip_en_changed = true;
        app_ctxt->vlan_strip_en = spec->vlan_strip_en();
    }

    // Handle vlan_insert_en change
    if (lif->vlan_insert_en != spec->vlan_insert_en()) {
        HAL_TRACE_DEBUG("{}:vlan_insert_en changed {} => {}",
                        __FUNCTION__, lif->vlan_insert_en, spec->vlan_insert_en());
        app_ctxt->vlan_insert_en_changed = true;
        app_ctxt->vlan_insert_en = spec->vlan_insert_en();
    }
    auto uplink_if = if_lookup_key_or_handle(spec->pinned_uplink_if_key_handle());
    auto uplink_if_handle = uplink_if ? uplink_if->hal_handle : HAL_HANDLE_INVALID;

    // Handle pinned uplink change
    if (lif->pinned_uplink != uplink_if_handle) {
        HAL_TRACE_DEBUG("{}:pinned uplink hdl changed {} => {}",
                        __FUNCTION__, lif->pinned_uplink, uplink_if_handle);
        app_ctxt->pinned_uplink_changed = true;
        app_ctxt->new_pinned_uplink     = uplink_if_handle;
    }

    if (spec->lif_qstate_map_size() && !lif->qstate_init_done) {
        app_ctxt->qstate_map_init_set = true;
    }

    cmp_key = memcmp(lif->rss.key, (uint8_t*)spec->rss().key().c_str(),
                 sizeof(lif->rss.key));
    cmp_indir = memcmp(lif->rss.indir, (uint8_t*)spec->rss().indir().c_str(),
                 sizeof(lif->rss.indir));
    if (lif->rss.type != spec->rss().type() || cmp_key || cmp_indir) {
        app_ctxt->rss_config_changed = true;
        HAL_TRACE_DEBUG("{}:rss configuration changed", __FUNCTION__);
    }

    if (spec->has_rx_policer()) {
        qos_policer_update_from_spec(spec->rx_policer(), &new_rx_policer);
    }
    if (!qos_policer_spec_same(&lif->qos_info.rx_policer, &new_rx_policer)) {
        app_ctxt->rx_policer_changed = true;
        HAL_TRACE_DEBUG("{}: rx policer configuration changed", __FUNCTION__);
    }

    if (spec->has_tx_policer()) {
        qos_policer_update_from_spec(spec->tx_policer(), &new_tx_policer);
    }
    if (!qos_policer_spec_same(&lif->qos_info.tx_policer, &new_tx_policer)) {
        app_ctxt->tx_policer_changed = true;
        HAL_TRACE_DEBUG("{}: tx policer configuration changed", __FUNCTION__);
    }

    if (lif->packet_filters.receive_promiscuous !=
        spec->packet_filter().receive_promiscuous()) {
        HAL_TRACE_DEBUG("lif prom change: {} => {}",
                        lif->packet_filters.receive_promiscuous,
                        spec->packet_filter().receive_promiscuous());
        app_ctxt->pkt_filter_prom_changed = true;
        app_ctxt->receive_promiscous = spec->
            packet_filter().receive_promiscuous();
    }

    if (lif->packet_filters.receive_broadcast !=
        spec->packet_filter().receive_broadcast()) {
        HAL_TRACE_DEBUG("lif bcast change: {} => {}",
                        lif->packet_filters.receive_broadcast,
                        spec->packet_filter().receive_broadcast());
        app_ctxt->pkt_filter_bcast_changed = true;
        app_ctxt->receive_broadcast = spec->
            packet_filter().receive_broadcast();
    }

    if (lif->packet_filters.receive_all_multicast !=
        spec->packet_filter().receive_all_multicast()) {
        HAL_TRACE_DEBUG("lif all_mc change: {} => {}",
                        lif->packet_filters.receive_all_multicast,
                        spec->packet_filter().receive_all_multicast());
        app_ctxt->pkt_filter_allmc_changed = true;
        app_ctxt->receive_all_multicast = spec->
            packet_filter().receive_all_multicast();
    }

    if (strcmp(lif->name, spec->name().c_str())) {
        HAL_TRACE_DEBUG("lif name change: {} => {}",
                        lif->name, spec->name().c_str());
        app_ctxt->name_changed = true;
    }

    if (lif->admin_status != spec->admin_status()) {
        app_ctxt->status_changed = true;
    }

    if (lif->bcast_filter.arp != spec->bcast_pkt_filter().arp() ||
        lif->bcast_filter.dhcp_client != spec->bcast_pkt_filter().dhcp_client() ||
        lif->bcast_filter.dhcp_server != spec->bcast_pkt_filter().dhcp_server() ||
        lif->bcast_filter.netbios != spec->bcast_pkt_filter().netbios()) {
        app_ctxt->bcast_filters_changed = true;
    }
    if (lif->mcast_filter.ipv6_neigh_adv != spec->mcast_pkt_filter().ipv6_neigh_adv() ||
        lif->mcast_filter.ipv6_router_adv != spec->mcast_pkt_filter().ipv6_router_adv() ||
        lif->mcast_filter.dhcpv6_relay != spec->mcast_pkt_filter().dhcpv6_relay() ||
        lif->mcast_filter.dhcpv6_mcast != spec->mcast_pkt_filter().dhcpv6_mcast() ||
        lif->mcast_filter.ipv6_mld != spec->mcast_pkt_filter().ipv6_mld() ||
        lif->mcast_filter.ipv6_neigh_sol != spec->mcast_pkt_filter().ipv6_neigh_sol()) {
        app_ctxt->mcast_filters_changed = true;
    }

    if (lif->rdma_sniff_en != spec->rdma_sniff_en()) {
        HAL_TRACE_DEBUG("lif rdma_sniff_en change: {} => {}",
                        lif->rdma_sniff_en,
                        spec->rdma_sniff_en());
        app_ctxt->rdma_sniff_en_changed = true;
        app_ctxt->rdma_sniff_en = spec->rdma_sniff_en();
    }

    return ret;
}

//------------------------------------------------------------------------------
// process a lif update request
//------------------------------------------------------------------------------
hal_ret_t
lif_update (LifSpec& spec, LifResponse *rsp)
{
    hal_ret_t             ret          = HAL_RET_OK;
    lif_t                 *lif         = NULL;
    cfg_op_ctxt_t         cfg_ctxt     = { 0 };
    dhl_entry_t           dhl_entry    = { 0 };
    const                 LifKeyHandle &kh = spec.key_or_handle();
    lif_update_app_ctxt_t app_ctxt     = { 0 };
    hal_handle_t          hal_handle   = 0;
    uint64_t              hw_lif_id    = 0;
    void                 *lifupd_timer = NULL;

    hal_api_trace(" API Begin: Lif Update ");
    proto_msg_dump(spec);

    // validate the request message
    ret = validate_lif_update(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:lif delete validation failed, ret : {}",
                      __FUNCTION__, ret);
        goto end;
    }

    lif = lif_lookup_key_or_handle(kh);
    if (lif == NULL) {
        HAL_TRACE_ERR("{}:failed to find lif, id {}, handle {}",
                      __FUNCTION__, kh.lif_id(), kh.lif_handle());
        ret = HAL_RET_LIF_NOT_FOUND;
        goto end;
    }
    HAL_TRACE_DEBUG("{}:update lif {}", __FUNCTION__,
                    lif->lif_id);

    hal_handle = lif->hal_handle;

    // Check for any change, if there is none goto end.
    app_ctxt.spec = &spec;
    app_ctxt.rsp  = rsp;
    ret = lif_handle_update(&app_ctxt, lif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:lif check update failed, ret : {}",
                      __FUNCTION__, ret);
        goto end;
    }

#if 0
    // Promiscuous flag is supposed to be used only in classic mode.
    // Used to update number of prom lifs on l2segs.
    // In non-classic mode, prom flag is not being used.
    if (!is_forwarding_mode_classic_nic() &&
        app_ctxt.pkt_filter_prom_changed &&
        !(lif->type == types::LIF_TYPE_HOST_MANAGEMENT ||
          lif->type == types::LIF_TYPE_MNIC_INTERNAL_MANAGEMENT ||
          lif->type == types::LIF_TYPE_MNIC_OOB_MANAGEMENT)) {
        HAL_TRACE_ERR("lif's promiscous filter can't be changed in mode {}",
                      g_hal_state->forwarding_mode());
        // ret = HAL_RET_INVALID_ARG;
        // goto end;
        // Silently suppressing prom mode change
        app_ctxt.pkt_filter_prom_changed = false;
    }
#endif

    if (!(app_ctxt.vlan_strip_en_changed ||
          app_ctxt.vlan_insert_en_changed ||
          app_ctxt.qstate_map_init_set ||
          app_ctxt.rss_config_changed ||
          app_ctxt.rx_policer_changed ||
          app_ctxt.tx_policer_changed ||
          app_ctxt.pkt_filter_prom_changed ||
          app_ctxt.pkt_filter_bcast_changed ||
          app_ctxt.pkt_filter_allmc_changed ||
          app_ctxt.pinned_uplink_changed ||
          app_ctxt.name_changed ||
          app_ctxt.status_changed ||
          app_ctxt.rdma_sniff_en_changed)) {
        HAL_TRACE_DEBUG("{}:no change in lif update: noop", __FUNCTION__);
        goto end;
    }

    /*
     * Scenario:
     * EP Moved to this lif:
     * - Take cfg db read lock. This should prevent lif update to
     *   create the clone
     * Lif Update from Nicmgr:
     * - When we clone here without write lock, FTE thread which is
     *   processing EP move takes read lock and gets the LIF. It will
     *   update the lif with ENIC in its if_list. The clone will not
     *   have this new ENIC and after the end of update we lost
     *   the ENIC in if_list which is there in lif but not in its clone.
     */
    // lif_make_clone(lif, (lif_t **)&dhl_entry.cloned_obj, spec);

    // form ctxt and call infra update object
    dhl_entry.handle  = lif->hal_handle;
    dhl_entry.obj     = lif;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_upd_obj(lif->hal_handle, &cfg_ctxt,
                             lif_update_upd_cb,
                             lif_update_commit_cb,
                             lif_update_abort_cb,
                             lif_update_cleanup_cb);

end:
    lif_prepare_rsp(rsp, ret, hal_handle);

    if (ret == HAL_RET_OK) {
       hw_lif_id = lif_hw_lif_id_get(lif);
       rsp->mutable_status()->set_hw_lif_id(hw_lif_id);
     }

    if (lif->type == types::LIF_TYPE_NONE) {
        // Send updates to Agent
        lifupd_timer = sdk::lib::timer_schedule(HAL_TIMER_ID_STREAM_LIF_UPDATE,
                                        LIF_STREAM_TIMER,
                                        (reinterpret_cast<void *>(lif->lif_id)),
                                        hal_stream_lif_updates, false);

        if (!lifupd_timer) {
            HAL_TRACE_ERR("Failed to schedule the timer for LIF updates: {:p}", (void *)lifupd_timer);
            ret = HAL_RET_ERR;
        }
    }

    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}

//------------------------------------------------------------------------------
// validate lif delete request
//------------------------------------------------------------------------------
hal_ret_t
validate_lif_delete_req (LifDeleteRequest& req, LifDeleteResponse *rsp)
{
    hal_ret_t ret = HAL_RET_OK;

    // key-handle field must be set
    if (!req.has_key_or_handle()) {
        HAL_TRACE_ERR("{}:spec has no key or handle", __FUNCTION__);
        ret =  HAL_RET_INVALID_ARG;
    }

    return ret;
}

//------------------------------------------------------------------------------
// validate lif delete request
//------------------------------------------------------------------------------
hal_ret_t
validate_lif_delete (lif_t *lif)
{
    hal_ret_t ret = HAL_RET_OK;

    // check for no presence of back references
    if (dllist_count(&lif->if_list_head)) {
        ret = HAL_RET_OBJECT_IN_USE;
        HAL_TRACE_ERR("{}:ifs still referring:", __FUNCTION__);
        hal_print_handles_list(&lif->if_list_head);
        goto end;
    }

    // check for no presence of filters
    if (lif->filter_list->num_elems()) {
        ret = HAL_RET_OBJECT_IN_USE;
        HAL_TRACE_ERR("LIF delete failure, filters still referring :");
        hal_print_handles_block_list(lif->filter_list);
        goto end;
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// 1. PD Call to delete PD and free up resources and deprogram HW
//------------------------------------------------------------------------------
hal_ret_t
lif_delete_del_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t               ret           = HAL_RET_OK;
    sdk_ret_t               sret          = SDK_RET_OK;
    dllist_ctxt_t           *lnode        = NULL;
    dhl_entry_t             *dhl_entry    = NULL;
    lif_t                   *lif          = NULL;
    pd::pd_lif_delete_args_t pd_lif_args  = { 0 };
    pd::pd_func_args_t       pd_func_args = {0};
    uint64_t                 hw_lif_id    = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // TODO: Check the dependency ref count for the lif.
    //       If its non zero, fail the delete.
    //       - Enic's referring to this lif

    lnode     = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    lif = (lif_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("{}:delete del cb {}",
                    __FUNCTION__, lif->lif_id);

    hw_lif_id = lif_hw_lif_id_get(lif);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_lif_delete_args_init(&pd_lif_args);
    pd_lif_args.lif = lif;
    pd_func_args.pd_lif_delete = &pd_lif_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_LIF_DELETE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to delete lif pd, err : {}",
                      __FUNCTION__, ret);
    }

    // Free up lif id for Nicmgr and DOL LIFs
    if (hw_lif_id < HAL_LIF_CPU || hw_lif_id > HAL_LIF_ID_SVC_LIF_MAX) {
        sret = lif_manager()->free_id(hw_lif_id, 1);
        if (sret != SDK_RET_OK) {
            HAL_TRACE_ERR("Failed to free hw_lif_id: {}", hw_lif_id);
            ret = HAL_RET_ERR;
            goto end;
        }
    }

    if (lif->type == types::LIF_TYPE_SWM) {
        lif_bcast_filter_uninstall(lif);
        lif_mcast_filter_uninstall(lif);
    }

    // P4+ delete code for the lif
    // RDMA delete code for the lif

end:
    return ret;
}

//------------------------------------------------------------------------------
// Update PI DBs as lif_delete_del_cb() was a succcess
//      a. Delete from lif id hash table
//      b. Remove object from handle id based hash table
//      c. Free PI lif
//------------------------------------------------------------------------------
hal_ret_t
lif_delete_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t     ret        = HAL_RET_OK;
    dllist_ctxt_t *lnode     = NULL;
    dhl_entry_t   *dhl_entry = NULL;
    lif_t         *lif       = NULL;
    hal_handle_t  hal_handle = 0;
    qos_class_t   *rx_qos_class;
    qos_class_t   *tx_qos_class;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode      = cfg_ctxt->dhl.next;
    dhl_entry  = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    lif        = (lif_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("{}:delete commit cb {}",
                    __FUNCTION__, lif->lif_id);

    rx_qos_class = find_qos_class_by_handle(lif->qos_info.rx_qos_class_handle);
    if (rx_qos_class) {
        ret = qos_class_del_lif_rx(rx_qos_class, lif);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to del rx qos class ref. from lif");
            goto end;
        }
    }

    tx_qos_class = find_qos_class_by_handle(lif->qos_info.tx_qos_class_handle);
    if (tx_qos_class) {
        ret = qos_class_del_lif_tx(tx_qos_class, lif);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to del tx qos class ref. from lif");
            goto end;
        }
    }

    // a. Remove from lif id hash table
    ret = lif_del_from_db(lif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to del lif {} from db, err : {}",
                      __FUNCTION__, lif->lif_id, ret);
        goto end;
    }

    // b. Remove object from handle id based hash table
    hal_handle_free(hal_handle);

    // Remove lif name and lif id from map
    g_hal_state->lif_name_id_map_delete(lif->name);

    // c. Free PI lif
    lif_cleanup(lif);

    // TODO: Decrement the ref counts of dependent objects

end:
    return ret;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
lif_delete_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
lif_delete_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a lif delete request
//------------------------------------------------------------------------------
hal_ret_t
lif_delete (LifDeleteRequest& req, LifDeleteResponse *rsp)
{
    hal_ret_t     ret          = HAL_RET_OK;
    lif_t         *lif         = NULL;
    cfg_op_ctxt_t cfg_ctxt     = { 0 };
    dhl_entry_t   dhl_entry    = { 0 };
    const         LifKeyHandle &kh = req .key_or_handle ();

    hal_api_trace(" API Begin: Lif Delete ");
	proto_msg_dump(req);

    // validate the request message
    ret = validate_lif_delete_req(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:lif delete validation failed, ret : {}",
                      __FUNCTION__, ret);
        goto end;
    }

    lif = lif_lookup_key_or_handle(kh);
    if (lif == NULL) {
        HAL_TRACE_ERR("{}:failed to find lif, id {}, handle {}",
                      __FUNCTION__, kh.lif_id(), kh.lif_handle());
        ret = HAL_RET_LIF_NOT_FOUND;
        goto end;
    }
    HAL_TRACE_DEBUG("{}:deleting lif {}",
                    __FUNCTION__, lif->lif_id);

    // validate if there no objects referring this sec. profile
    ret = validate_lif_delete(lif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:lif delete validation failed, "
                      "ret : {}",
                      __FUNCTION__, ret);
        goto end;
    }

    // form ctxt and call infra add
    dhl_entry.handle  = lif->hal_handle;
    dhl_entry.obj     = lif;
    cfg_ctxt.app_ctxt = NULL;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_del_obj(lif->hal_handle, &cfg_ctxt,
                             lif_delete_del_cb,
                             lif_delete_commit_cb,
                             lif_delete_abort_cb,
                             lif_delete_cleanup_cb);

end:
    rsp->set_api_status(hal_prepare_rsp(ret));
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}

// TODO: following APIs need to be cleaned up.. they dont use types.APIStatus
//       and give out return codes that user has no way to interpret !!!
void
LifGetQState (const intf::QStateGetReq &req, intf::QStateGetResp *rsp)
{
    std::unique_ptr<char[]> buf(new char[kMaxQStateSize]);
    uint32_t size_to_copy = std::min(kMaxQStateSize, req.ret_data_size());
#if 0
    int64_t ret = lif_manager()->GetLIFQStateAddr(
        req.lif_handle(), req.type_num(), req.qid());
#endif
    int64_t ret = lif_manager()->get_lif_qstate_addr(req.lif_handle(),
                                                     req.type_num(), req.qid());

    if (ret < 0) {
        rsp->set_error_code(0 - (int)ret);
        return;
    }
    rsp->set_q_addr((uint64_t)ret);
    ret = lif_manager()->read_qstate(req.lif_handle(), req.type_num(),
                                     req.qid(), (uint8_t *)buf.get(),
                                     kMaxQStateSize);
#if 0
    int ret2 = lif_manager()->ReadQState(req.lif_handle(), req.type_num(),
                                         req.qid(), (uint8_t *)buf.get(),
                                         kMaxQStateSize);
    if (ret2 < 0) {
        rsp->set_error_code(0 - ret2);
        return;
    }
#endif
    rsp->set_error_code(0);
    if (size_to_copy == 0)
        return;
    rsp->set_queue_state(std::string(buf.get(), size_to_copy));
}

void
LifSetQState (const intf::QStateSetReq &req, intf::QStateSetResp *rsp)
{
    if (req.queue_state().size() == 0) {
        rsp->set_error_code(EINVAL);
        return;
    }
    std::unique_ptr<uint8_t[]> buf;
    uint8_t *state = (uint8_t *)req.queue_state().c_str();
    if (req.has_label()) {
        uint8_t off;
        int ret = hal_get_pc_offset(req.label().prog_name().c_str(),
                                    req.label().label().c_str(), &off);
#if 0
        int ret = lif_manager()->GetPCOffset(req.label().handle().c_str(),
            req.label().prog_name().c_str(),
            req.label().label().c_str(), &off);
#endif
        if (ret < 0) {
            rsp->set_error_code(0 - ret);
            return;
        }
        buf.reset(new uint8_t[req.queue_state().size()]);
        bcopy(req.queue_state().c_str(), buf.get(), req.queue_state().size());
        buf.get()[0] = off;
        state = buf.get();
    }

    sdk_ret_t ret = lif_manager()->write_qstate(req.lif_handle(), req.type_num(),
                                                req.qid(), state,
                                                req.queue_state().size());
    if (ret != SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to write qstate: ret: {}", ret);
    }

#if 0
    int ret = lif_manager()->WriteQState(req.lif_handle(), req.type_num(),
                                         req.qid(), state,
                                         req.queue_state().size());
    rsp->set_error_code(0 - ret);
#endif
    rsp->set_error_code(0 - ret);
}

static bool
lif_get_ht_cb (void *ht_entry, void *ctxt)
{
    hal_handle_id_ht_entry_t *entry = (hal_handle_id_ht_entry_t *)ht_entry;
    LifGetResponseMsg *rsp          = (LifGetResponseMsg *)ctxt;
    LifGetResponse *response        = rsp->add_response();
    lif_t          *lif             = NULL;

    lif = (lif_t *)hal_handle_get_obj(entry->handle_id);
    lif_process_get(lif, response);

    // Always return false here, so that we walk through all hash table
    // entries.
    return false;
}

hal_ret_t
lif_get (LifGetRequest& req, LifGetResponseMsg *rsp)
{
    lif_t   *lif;

    if (!req.has_key_or_handle()) {
        g_hal_state->lif_id_ht()->walk(lif_get_ht_cb, rsp);
    } else {
        auto response = rsp->add_response();
        lif = lif_lookup_key_or_handle(req.key_or_handle());
        if (!lif) {
            response->set_api_status(types::API_STATUS_NOT_FOUND);
            return HAL_RET_INVALID_ARG;
        }
        lif_process_get(lif, response);
    }
    fflush(stdout);

    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// Adds If into lif list
//-----------------------------------------------------------------------------
hal_ret_t
lif_add_if (lif_t *lif, if_t *hal_if)
{
    hal_ret_t                  ret    = HAL_RET_OK;
    hal_handle_id_list_entry_t *entry = NULL;

    if (lif == NULL || hal_if == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // Allocate the entry
    entry = (hal_handle_id_list_entry_t *)g_hal_state->
        hal_handle_id_list_entry_slab()->alloc();
    if (entry == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }
    entry->handle_id = hal_if->hal_handle;

    lif_lock(lif, __FILENAME__, __LINE__, __func__);      // lock
    // Insert into the list
    sdk::lib::dllist_add(&lif->if_list_head, &entry->dllist_ctxt);
    lif_unlock(lif, __FILENAME__, __LINE__, __func__);    // unlock

end:

    HAL_TRACE_DEBUG("pi-if:{}: add lif => if, ids: {} => {}, "
                    "hdls: {} => {}, ret:{}",
                    __FUNCTION__, lif->lif_id, hal_if->if_id,
                    lif->hal_handle, hal_if->hal_handle, ret);
    return ret;
}

//-----------------------------------------------------------------------------
// Remove If from lif list
//-----------------------------------------------------------------------------
hal_ret_t
lif_del_if (lif_t *lif, if_t *hal_if)
{
    hal_ret_t                  ret    = HAL_RET_IF_NOT_FOUND;
    hal_handle_id_list_entry_t *entry = NULL;
    dllist_ctxt_t              *curr  = NULL, *next = NULL;

    if (!lif || !hal_if) {
        HAL_TRACE_WARN("lif:{:#x}, hal_if:{:#x} is NULL.",
                       (uint64_t)lif, (uint64_t)hal_if);
        goto end;
    }

    lif_lock(lif, __FILENAME__, __LINE__, __func__);      // lock
    dllist_for_each_safe(curr, next, &lif->if_list_head) {
        entry = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
        if (entry->handle_id == hal_if->hal_handle) {
            // Remove from list
            sdk::lib::dllist_del(&entry->dllist_ctxt);
            // Free the entry
            hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_LIST_ENTRY, entry);
            ret = HAL_RET_OK;
        }
    }
    lif_unlock(lif, __FILENAME__, __LINE__, __func__);    // unlock

    HAL_TRACE_DEBUG("pi-if:{}: del lif =/=> if, {} =/=> {}, ret:{}",
                    __FUNCTION__, lif->lif_id, hal_if->if_id, ret);

end:
    return ret;
}

//------------------------------------------------------------------------------
// lif propagate egress en
//------------------------------------------------------------------------------
hal_ret_t
lif_handle_egress_en (lif_t *lif, filter_key_t *key, bool egress_en)
{
    hal_ret_t                   ret = HAL_RET_OK;
    if_t                        *hal_if = NULL;
    bool                        update_enic = false;
    dllist_ctxt_t               *lnode  = NULL;
    hal_handle_id_list_entry_t  *entry  = NULL;

    // Walk enicifs
    dllist_for_each(lnode, &lif->if_list_head) {
        entry = dllist_entry(lnode, hal_handle_id_list_entry_t, dllist_ctxt);
        hal_if = find_if_by_handle(entry->handle_id);
        if (!hal_if) {
            HAL_TRACE_ERR("{}:unable to find if with handle:{}",
                          __FUNCTION__, entry->handle_id);
            continue;
        }
        update_enic = false;

        // Check if ENIC has a hit with (mac,vlan) or ((mac, *) and (*, vlan))
        filter_check_enic_with_filter(key, lif, hal_if, egress_en, &update_enic);

        if (update_enic) {
            ret = enicif_update_egress_en(hal_if, egress_en);
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
// handling lif update in corresponding IFs
//------------------------------------------------------------------------------
hal_ret_t
lif_update_trigger_if (lif_t *lif,
                       bool vlan_strip_en_changed,
                       bool vlan_strip_en,
                       bool vlan_insert_en_changed,
                       bool vlan_insert_en,
                       bool pinned_uplink_changed,
                       hal_handle_t pinned_uplink)
{
    hal_ret_t                  ret     = HAL_RET_OK;
    dllist_ctxt_t              *lnode  = NULL;
    hal_handle_id_list_entry_t *entry  = NULL;
    if_t                       *hal_if = NULL;
    pd::pd_if_lif_update_args_t   args    = { 0 };

    if (lif == NULL) {
        return ret;
    }

    HAL_TRACE_DEBUG("{}:lif_id: {}",
                    __FUNCTION__, lif->lif_id);

    pd::pd_if_lif_update_args_init(&args);
    // Walk enicifs
    dllist_for_each(lnode, &lif->if_list_head) {
        entry = dllist_entry(lnode, hal_handle_id_list_entry_t, dllist_ctxt);
        hal_if = find_if_by_handle(entry->handle_id);
        if (!hal_if) {
            HAL_TRACE_ERR("{}:unable to find if with handle:{}",
                          __FUNCTION__, entry->handle_id);
            continue;
        }
        args.intf = hal_if;
        args.lif = lif;
        args.vlan_strip_en_changed = vlan_strip_en_changed;
        args.vlan_strip_en = vlan_strip_en;
        args.vlan_insert_en_changed = vlan_insert_en_changed;
        args.vlan_insert_en = vlan_insert_en;
        args.pinned_uplink_changed = pinned_uplink_changed;
        args.pinned_uplink = pinned_uplink;
        if_handle_lif_update(&args);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// Callback to disable TX scheduler for a LIF
//-----------------------------------------------------------------------------
static bool
lif_sched_ht_cb (void *ht_entry, void *ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    hal_handle_id_ht_entry_t    *entry = (hal_handle_id_ht_entry_t *)ht_entry;
    lif_sched_control_cb_ctxt_t *cb_ctxt = (lif_sched_control_cb_ctxt_t *)ctxt;
    lif_t                       *lif = NULL;
    pd::pd_func_args_t          pd_func_args = {0};
    pd::pd_lif_sched_control_args_t lif_sched = {0};

    lif = (lif_t *)hal_handle_get_obj(entry->handle_id);

    lif_sched.lif = lif;
    lif_sched.en = cb_ctxt->en;
    pd_func_args.pd_lif_sched_control = &lif_sched;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_LIF_SCHED_CONTROL,
                          &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to disable TX scheduler for lif {}. err: {}",
                      lif->lif_id, ret);
        goto end;
    }

end:
    // False: Don't stop the traversal
    return false;
}

//-----------------------------------------------------------------------------
// Disable TX scheduler for all LIFs
//-----------------------------------------------------------------------------
hal_ret_t
lif_disable_tx_scheduler (void)
{
    lif_sched_control_cb_ctxt_t ctxt;
    ctxt.en = false;
    g_hal_state->lif_id_ht()->walk(lif_sched_ht_cb, &ctxt);

    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// adds filter into lif list
//-----------------------------------------------------------------------------
hal_ret_t
lif_add_filter (lif_t *lif, filter_t *filter)
{
    hal_ret_t                   ret = HAL_RET_OK;

    if (lif == NULL || filter == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lif_lock(lif, __FILENAME__, __LINE__, __func__);      // lock
    ret = lif->filter_list->insert(&filter->hal_handle);
    lif_unlock(lif, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed to add filter {} to lif {}",
                        filter_keyhandle_to_str(filter), lif->lif_id);
        goto end;
    }
    HAL_TRACE_DEBUG("Added filter {} to lif {}",
                    filter_keyhandle_to_str(filter),
                    lif->lif_id);

end:
    return ret;
}

//-----------------------------------------------------------------------------
// remove filter from lif list
//-----------------------------------------------------------------------------
hal_ret_t
lif_del_filter (lif_t *lif, filter_t *filter)
{
    hal_ret_t                   ret = HAL_RET_OK;

    if (lif == NULL || filter == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lif_lock(lif, __FILENAME__, __LINE__, __func__);      // lock
    ret = lif->filter_list->remove(&filter->hal_handle);
    lif_unlock(lif, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to remove filter {} from from lif {}, err : {}",
                       filter_keyhandle_to_str(filter), lif->lif_id, ret);
        goto end;
    }
    HAL_TRACE_DEBUG("Deleted filter {} from lif {}",
                    filter_keyhandle_to_str(filter),
                    lif->lif_id);

end:
    return ret;
}

//-----------------------------------------------------------------------------
// Prints Enicifs mapped to the lif
//-----------------------------------------------------------------------------
void
lif_print_ifs(lif_t *lif)
{
    dllist_ctxt_t              *lnode  = NULL;
    hal_handle_id_list_entry_t *entry  = NULL;
    if_t                       *hal_if = NULL;

    dllist_for_each(lnode, &(lif->if_list_head)) {
        entry = dllist_entry(lnode, hal_handle_id_list_entry_t, dllist_ctxt);
        hal_if = find_if_by_handle(entry->handle_id);
        HAL_TRACE_DEBUG("if: {}", hal_if->if_id);
    }
}

//-----------------------------------------------------------------------------
// Print PI lif
//-----------------------------------------------------------------------------
void
lif_print(lif_t *lif)
{
    fmt::MemoryWriter          buf;
    dllist_ctxt_t              *lnode  = NULL;
    hal_handle_id_list_entry_t *entry  = NULL;
    if_t                       *hal_if = NULL;

    if (!lif) {
        return;
    }

    buf.write("lif id:{}, vlan_strip_en:{}, vlan_insert_en:{}, "
              "pinned_uplink:{}, enable_rdma:{}, enable_nvme:{}, "
              "hal_handle:{}, num_enics:{} ",
              lif->lif_id, lif->vlan_strip_en, lif->vlan_insert_en,
              lif->pinned_uplink, lif->enable_rdma, lif->enable_nvme,
              lif->hal_handle,
              dllist_count(&(lif->if_list_head)));

    if (dllist_count(&(lif->if_list_head))) {
        buf.write("Enics\n");
        buf.write("-----\n");
        dllist_for_each(lnode, &(lif->if_list_head)) {
            entry = dllist_entry(lnode, hal_handle_id_list_entry_t, dllist_ctxt);
            hal_if = find_if_by_handle(entry->handle_id);
            buf.write("if_id: {}, ", hal_if->if_id);
        }
    }
    HAL_TRACE_DEBUG("{}", buf.c_str());
}

//------------------------------------------------------------------------------
// PI lif to str
//------------------------------------------------------------------------------
const char *
lif_keyhandle_to_str (lif_t *lif)
{
    static thread_local char       lif_str[4][50];
    static thread_local uint8_t    lif_str_next = 0;
    char                           *buf;

    buf = lif_str[lif_str_next++ & 0x3];
    memset(buf, 0, 50);
    if (lif) {
        snprintf(buf, 50, "lif(id: %u, handle: %lu)",
                 lif->lif_id, lif->hal_handle);
    }
    return buf;
}

//------------------------------------------------------------------------------
// Prints LIF's keyhandle
//------------------------------------------------------------------------------
const char *
lif_spec_keyhandle_to_str (const LifKeyHandle& key_handle)
{
	static thread_local char       lif_str[4][50];
	static thread_local uint8_t    lif_str_next = 0;
	char                           *buf;

	buf = lif_str[lif_str_next++ & 0x3];
	memset(buf, 0, 50);

    if (key_handle.key_or_handle_case() == LifKeyHandle::kLifId) {
		snprintf(buf, 50, "lif_id: %lu", key_handle.lif_id());
    }
    if (key_handle.key_or_handle_case() == LifKeyHandle::kLifHandle) {
		snprintf(buf, 50, "lif_handle: 0x%lx", key_handle.lif_handle());
    }

	return buf;
}

// TODO: Eventually may have to be moved to SDK
 int32_t
 hal_get_pc_offset(const char *prog_name, const char *label,
                   uint8_t *offset)
 {
      mem_addr_t off;

      off = prog_info()->symbol_address((char *)prog_name, (char *)label);
      if (off == SDK_INVALID_HBM_ADDRESS)
          return -ENOENT;
      // 64 byte alignment check
      if ((off & 0x3F) != 0) {
          return -EIO;
      }
      // offset can be max 14 bits
      if (off > 0x3FC0) {
          return -EIO;
      }
      *offset = (uint8_t) (off >> 6);
      return 0;
 }

}    // namespace hal
