#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/nw/interface.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/gen/proto/hal/l2segment.pb.h"
#include "nic/hal/src/lif/lif_manager.hpp"
#include "nic/hal/src/utils/utils.hpp"
#include "nic/include/oif_list_api.hpp"
#include "nic/hal/src/utils/if_utils.hpp"
#include "nic/hal/src/internal/rdma.hpp"
#include "nic/hal/src/lif/lif.hpp"
#include "nic/hal/src/lif/eth.hpp"
#include "nic/hal/src/aclqos/qos.hpp"

using hal::pd::pd_if_create_args_t;
using hal::pd::pd_if_lif_update_args_t;

namespace hal {

void *
lif_id_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t *ht_entry = NULL;
    lif_t                    *lif      = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    lif = (lif_t *)hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(lif->lif_id);
}

uint32_t
lif_id_compute_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(lif_id_t)) % ht_size;
}

bool
lif_id_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(lif_id_t *)key1 == *(lif_id_t *)key2) {
        return true;
    }
    return false;
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
    HAL_SPINLOCK_INIT(&lif->slock, PTHREAD_PROCESS_SHARED);

    // initialize the operational state
    lif->hal_handle    = HAL_HANDLE_INVALID;
    lif->pinned_uplink = HAL_HANDLE_INVALID;

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
    HAL_SPINLOCK_DESTROY(&lif->slock);
    hal::delay_delete_to_slab(HAL_SLAB_LIF, lif);
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
        HAL_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() ==
                   HAL_OBJ_ID_LIF);
        lif = (lif_t *)hal_handle_get_obj(entry->handle_id);
        return lif;
    }
    return NULL;
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

static inline uint64_t
lif_hw_lif_id_get (lif_t *lif)
{
    hal_ret_t ret = HAL_RET_OK;

    pd::pd_lif_get_args_t hwlifid_args;
    memset(&hwlifid_args, 0, sizeof(pd::pd_lif_get_args_t));

    hwlifid_args.lif = lif;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_LIF_GET, (void *)&hwlifid_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get hw_lif_id for lif {}", lif->lif_id);
    }

    return hwlifid_args.hw_lif_id;
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
    LIFQStateParams qs_params = { 0 };
    int32_t         ec        = 0;

    for (int i = 0; i < spec.lif_qstate_map_size(); i++) {
        const auto &ent = spec.lif_qstate_map(i);
        if (ent.type_num() >= kNumQTypes) {
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

        qs_params.type[ent.type_num()].size    = ent.size();
        qs_params.type[ent.type_num()].entries = ent.entries();

        // Set both cosA,cosB to admin_cos(cosA) value for admin-qtype.
        if (ent.purpose() != intf::LIF_QUEUE_PURPOSE_ADMIN) {
            qs_params.type[ent.type_num()].cosA    = (lif->qos_info.coses & 0x0f);
            qs_params.type[ent.type_num()].cosB    = (lif->qos_info.coses & 0xf0) >> 4;
        } else {
            qs_params.type[ent.type_num()].cosA = qs_params.type[ent.type_num()].cosB = (lif->qos_info.coses & 0x0f);
        }

        lif->qinfo[ent.purpose()].type = ent.type_num();
        lif->qinfo[ent.purpose()].size = (uint16_t)pow(2, ent.size());
        lif->qinfo[ent.purpose()].num_queues = (uint16_t)pow(2, ent.entries());
    }

    qs_params.dont_zero_memory = dont_zero_qstate_mem;
    // make sure that when you are creating with hw_lif_id the lif is alloced
    // already, otherwise this call may return an error
    if ((ec = g_lif_manager->InitLIFQState(hw_lif_id, &qs_params)) < 0) {
        HAL_TRACE_ERR("Failed to initialize LIFQState: err_code : {}", ec);
        return HAL_RET_INVALID_ARG;
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

    for (int i = 0; i < spec.lif_qstate_size(); i++) {
        const auto &req = spec.lif_qstate(i);

        uint8_t *state = (uint8_t *)req.queue_state().c_str();
        if (req.has_label()) {
            uint8_t off;
            int ret = g_lif_manager->GetPCOffset(req.label().handle().c_str(),
                req.label().prog_name().c_str(),
                req.label().label().c_str(), &off);
            if (ret < 0) {
                ret = HAL_RET_ERR;
                goto end;
            }
            buf.reset(new uint8_t[req.queue_state().size()]);
            bcopy(req.queue_state().c_str(), buf.get(), req.queue_state().size());
            buf.get()[0] = off;
            state = buf.get();
        }

        int ret = g_lif_manager->WriteQState(hw_lif_id, req.type_num(),
                                             req.qid(), state,
                                             req.queue_state().size());
        if (ret < 0) {
            HAL_TRACE_ERR("Failed to set LIFQState : {}", ret);
            ret = HAL_RET_ERR;
            goto end;
        }
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

    HAL_TRACE_DEBUG("{}: P4 Processing for lif id {}", __FUNCTION__,
                    spec.key_or_handle().lif_id());

    // allocate all PD resources and finish programming, if any
    pd::pd_lif_create_args_init(&pd_lif_args);
    pd_lif_args.lif = lif;
    if (lif_hal_info) {
        pd_lif_args.with_hw_lif_id = lif_hal_info->with_hw_lif_id;
        pd_lif_args.hw_lif_id = lif_hal_info->hw_lif_id;
    }
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_LIF_CREATE, (void *)&pd_lif_args);
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
    if (spec.rss().enable() && spec.rss().key().size() > ETH_RSS_KEY_LENGTH) {
        HAL_TRACE_ERR("{}: Invalid key size {}",
                      __FUNCTION__, spec.rss().key().size());
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

    // allocate a hw lif id
    if (lif_hal_info && lif_hal_info->with_hw_lif_id) {
        hw_lif_id = lif_hal_info->hw_lif_id;
        // make sure hw_lif_id is already allocated.
        LIFQState *qstate = g_lif_manager->GetLIFQState(hw_lif_id);
        if (qstate == nullptr) {
            ret = HAL_RET_INVALID_ARG;
            goto end;
        }
        memcpy(&lif_info, lif_hal_info, sizeof(lif_info));
        dont_zero_qstate_mem = lif_hal_info->dont_zero_qstate_mem;
    } else {
        hw_lif_id = g_lif_manager->LIFRangeAlloc(-1, 1);
        if (((int32_t)hw_lif_id) < 0) {
            HAL_TRACE_ERR("Failed to allocate lif, hw_lif_id : {}", hw_lif_id);
            ret = HAL_RET_NO_RESOURCE;
            goto end;
        }
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
                goto end;
            }
        }
        lif->qstate_init_done = true;
    }

    // create the lif now
    ret = lif_fwd_create(*spec, rsp, lif, &lif_info);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("P4 Lif create failure, err : {}", ret);
        ret = HAL_RET_ERR;
        goto end;
    }

    // For rdma enabled Lifs, call RDMA specific init (allocates KT, PT, etc)
    if (lif->enable_rdma) {
        ret = rdma_lif_init(*spec, hw_lif_id);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("P4 Lif create failure, err : {}", ret);
            ret = HAL_RET_ERR;
            goto end;
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
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_LIF_DELETE, (void *)&pd_lif_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("{}:failed to delete lif pd, err : {}",
                          __FUNCTION__, ret);
        }
    }

    // 2. remove object from hal_handle id based hash table in infra
    hal_handle_free(hal_handle);

    // 3. Free PI lif
    lif_free(lif);
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
    qos_class_t                *qos_class;
    uint32_t                   cosA = 0, cosB = 0;
    pd::pd_qos_class_get_admin_cos_args_t args = {0};


    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("{}:lif create for id {}", __FUNCTION__,
                    spec.key_or_handle().lif_id());

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

    // consume the config
    lif->lif_id              = spec.key_or_handle().lif_id();
    lif->admin_status        = spec.admin_status();
    lif->hal_handle          = hal_alloc_handle();
    lif->vlan_strip_en       = spec.vlan_strip_en();
    lif->vlan_insert_en      = spec.vlan_insert_en();
    lif->pinned_uplink       = uplink_if ? uplink_if->hal_handle :
                               HAL_HANDLE_INVALID;
    lif->packet_filters.receive_broadcast = spec.packet_filter().
                                            receive_broadcast();
    lif->packet_filters.receive_promiscuous = spec.packet_filter().
                                              receive_promiscuous();
    lif->packet_filters.receive_all_multicast = spec.packet_filter().
                                                receive_all_multicast();
    // RSS configuration
    lif->rss.enable = spec.rss().enable();
    lif->rss.type = spec.rss().type();
    memcpy(&lif->rss.key, (uint8_t *)spec.rss().key().c_str(),
           sizeof(lif->rss.key));

    //lif->allmulti = spec.allmulti();
    lif->enable_rdma = spec.enable_rdma();
    lif->rdma_max_keys = spec.rdma_max_keys();
    lif->rdma_max_pt_entries = spec.rdma_max_pt_entries();

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

    // cosA of a LIF will always be the ADMIN-COS.
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_QOS_GET_ADMIN_COS, (void *)&args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-lif:{}:failed to fetch admin cos of lif {}, err : {}",
                      __FUNCTION__, lif->lif_id, ret);
    }
    cosA = args.cos;

    lif->qos_info.cos_bmp =  ((1 << cosA) | (1 << cosB));
    lif->qos_info.coses   =  (cosA & 0x0f) | ((cosB << 4) & 0xf0);

    if (spec.has_rx_qos_class()) {
        qos_class = find_qos_class_by_key_handle(spec.rx_qos_class());
        lif->qos_info.rx_qos_class_handle = qos_class ? qos_class->hal_handle : HAL_HANDLE_INVALID;
    }

    // allocate hal handle id
    hal_handle = hal_handle_alloc(HAL_OBJ_ID_LIF);
    if (hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("{}:failed to alloc handle {}",
                      __FUNCTION__, lif->lif_id);
        rsp->set_api_status(types::API_STATUS_HANDLE_INVALID);
        lif_free(lif);
        return HAL_RET_HANDLE_INVALID;
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
        HAL_TRACE_ERR("{}:error in creating lif with id ", __FUNCTION__, lif->lif_id);
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
        entry->set_addr(g_lif_manager->GetLIFQStateAddr(hw_lif_id, ent.type_num(), 0));
    }

    // Return LIF RDMA data for RDMA enabled lifs
    if (lif->enable_rdma) {
        rsp->set_rdma_data_valid(true);
        rsp->mutable_rdma_data()->set_pt_base_addr(rdma_lif_pt_base_addr(hw_lif_id));
        rsp->mutable_rdma_data()->set_kt_base_addr(rdma_lif_kt_base_addr(hw_lif_id));
    }

    HAL_TRACE_DEBUG("Lif Created:");
    HAL_TRACE_DEBUG("------------");
    lif_print(lif);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
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
    if (spec.rss().enable() && spec.rss().key().size() > ETH_RSS_KEY_LENGTH) {
        HAL_TRACE_ERR("{}: Invalid key size {}",
                      __FUNCTION__, spec.rss().key().size());
        return HAL_RET_INVALID_ARG;
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
    dllist_ctxt_t         *lnode     = NULL;
    dhl_entry_t           *dhl_entry = NULL;
    lif_t                 *lif       = NULL;
    lif_t                 *lif_clone = NULL;
    lif_update_app_ctxt_t *app_ctxt  = NULL;
    uint32_t              hw_lif_id;
    LifSpec                     *spec = NULL;

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
    lif_clone = (lif_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("{}:update upd cb {}",
                    __FUNCTION__, lif->lif_id);

    // Rule of thumb is if anything which clone has to be updated doesnt result
    // in change of original, it can be done here.

    // Update PI clone
    if (app_ctxt->vlan_strip_en_changed) {
        lif_clone->vlan_strip_en = spec->vlan_strip_en();
    }
    if (app_ctxt->vlan_insert_en_changed) {
        lif_clone->vlan_insert_en = app_ctxt->vlan_insert_en;
    }
    if (app_ctxt->pinned_uplink_changed) {
        lif_clone->pinned_uplink = app_ctxt->new_pinned_uplink;
    }

    if (app_ctxt->rss_config_changed) {
        lif_clone->rss.enable = spec->rss().enable();
        lif_clone->rss.type = spec->rss().type();
        memcpy(&lif_clone->rss.key, (uint8_t *)spec->rss().key().c_str(),
               sizeof(lif_clone->rss.key));
    }

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_lif_update_args_init(&args);
    args.lif                   = lif_clone;
    args.vlan_strip_en_changed = app_ctxt->vlan_strip_en_changed;
    args.vlan_strip_en         = app_ctxt->vlan_strip_en;
    args.qstate_map_init_set   = app_ctxt->qstate_map_init_set;
    args.rx_policer_changed    = app_ctxt->rx_policer_changed;
    args.tx_policer_changed    = app_ctxt->tx_policer_changed;

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

    ret = pd::hal_pd_call(pd::PD_FUNC_ID_LIF_UPDATE, (void *)&args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to delete lif pd, err : {}",
                      __FUNCTION__, ret);
    }

    if (app_ctxt->vlan_strip_en_changed || app_ctxt->vlan_insert_en_changed) {
        // Triggers reprogramming of output mapping table for enicifs and uplinks
        ret = lif_update_trigger_if(lif, app_ctxt->vlan_strip_en_changed,
                                    app_ctxt->vlan_strip_en,
                                    app_ctxt->vlan_insert_en_changed,
                                    app_ctxt->vlan_insert_en);
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

    *lif_clone = lif_alloc_init();
    memcpy(*lif_clone, lif, sizeof(lif_t));

    // After clone always reset lists
    dllist_reset(&(*lif_clone)->if_list_head);

    args.lif = lif;
    args.clone = *lif_clone;
    // pd::pd_lif_make_clone(lif, *lif_clone);
    pd::hal_pd_call(pd::PD_FUNC_ID_LIF_MAKE_CLONE, (void *)&args);


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
    // lif_update_app_ctxt_t *app_ctxt   = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-lif{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode     = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    // app_ctxt  = (lif_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    lif       = (lif_t *)dhl_entry->obj;
    lif_clone = (lif_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("{}:update commit cb {}",
                    __FUNCTION__, lif->lif_id);

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

    // Free PD
    pd::pd_lif_mem_free_args_init(&pd_lif_args);
    pd_lif_args.lif = lif;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_LIF_MEM_FREE, (void *)&pd_lif_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to delete lif pd, err : {}",
                      __FUNCTION__, ret);
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
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_LIF_MEM_FREE, (void *)&pd_lif_args);
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
    int cmp;
    policer_t new_rx_policer = {0};
    policer_t new_tx_policer = {0};

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

    cmp = memcmp(lif->rss.key, (uint8_t*)spec->rss().key().c_str(),
                 sizeof(lif->rss.key));
    if (lif->rss.enable != spec->rss().enable() ||
        lif->rss.type != spec->rss().type() ||
        cmp) {
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
    const                 LifKeyHandle &kh = spec .key_or_handle ();
    lif_update_app_ctxt_t app_ctxt     = { 0 };
    hal_handle_t          hal_handle   = 0;
    uint64_t              hw_lif_id    = 0;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");

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

    if (!(app_ctxt.vlan_strip_en_changed ||
          app_ctxt.vlan_insert_en_changed ||
          app_ctxt.qstate_map_init_set ||
          app_ctxt.rss_config_changed ||
          app_ctxt.rx_policer_changed ||
          app_ctxt.tx_policer_changed)) {
        HAL_TRACE_ERR("{}:no change in lif update: noop", __FUNCTION__);
        goto end;
    }

    lif_make_clone(lif, (lif_t **)&dhl_entry.cloned_obj, spec);

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

    hw_lif_id = lif_hw_lif_id_get(lif);
    rsp->mutable_status()->set_hw_lif_id(hw_lif_id);

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
    }

    return ret;
}

//------------------------------------------------------------------------------
// 1. PD Call to delete PD and free up resources and deprogram HW
//------------------------------------------------------------------------------
hal_ret_t
lif_delete_del_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t               ret          = HAL_RET_OK;
    dllist_ctxt_t           *lnode       = NULL;
    dhl_entry_t             *dhl_entry   = NULL;
    lif_t                   *lif         = NULL;
    pd::pd_lif_delete_args_t pd_lif_args = { 0 };

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

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_lif_delete_args_init(&pd_lif_args);
    pd_lif_args.lif = lif;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_LIF_DELETE, (void *)&pd_lif_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to delete lif pd, err : {}",
                      __FUNCTION__, ret);
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

    // c. Free PI lif
    lif_free(lif);

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

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");

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
    int64_t ret = g_lif_manager->GetLIFQStateAddr(
        req.lif_handle(), req.type_num(), req.qid());

    if (ret < 0) {
        rsp->set_error_code(0 - (int)ret);
        return;
    }
    rsp->set_q_addr((uint64_t)ret);
    int ret2 = g_lif_manager->ReadQState(req.lif_handle(), req.type_num(),
                                         req.qid(), (uint8_t *)buf.get(),
                                         kMaxQStateSize);
    if (ret2 < 0) {
        rsp->set_error_code(0 - ret2);
        return;
    }
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
        int ret = g_lif_manager->GetPCOffset(req.label().handle().c_str(),
            req.label().prog_name().c_str(),
            req.label().label().c_str(), &off);
        if (ret < 0) {
            rsp->set_error_code(0 - ret);
            return;
        }
        buf.reset(new uint8_t[req.queue_state().size()]);
        bcopy(req.queue_state().c_str(), buf.get(), req.queue_state().size());
        buf.get()[0] = off;
        state = buf.get();
    }

    int ret = g_lif_manager->WriteQState(req.lif_handle(), req.type_num(),
                                         req.qid(), state,
                                         req.queue_state().size());
    rsp->set_error_code(0 - ret);
}

static void
lif_process_get (lif_t *lif, LifGetResponse *rsp)
{
    LifSpec  *spec     = NULL;
    uint64_t hw_lif_id = lif_hw_lif_id_get(lif);

    // fill in the config spec of this lif.
    spec = rsp->mutable_spec();
    spec->mutable_key_or_handle()->set_lif_id(lif->lif_id);
    spec->set_admin_status(lif->admin_status);
    spec->set_enable_rdma(lif->enable_rdma);
    spec->set_rdma_max_keys(lif->rdma_max_keys);
    spec->set_rdma_max_pt_entries(lif->rdma_max_pt_entries);

    rsp->mutable_status()->set_hw_lif_id(hw_lif_id);

    rsp->mutable_status()->set_lif_handle(lif->hal_handle);
    HAL_TRACE_ERR("lif_handle: {}", lif->hal_handle);
    rsp->set_api_status(types::API_STATUS_OK);
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
    HAL_TRACE_ERR("lif_get ---------------------");
    printf("%s: Trying to get lif =================", __FUNCTION__);

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

    HAL_TRACE_DEBUG("pi-if:{}: add lif =/=> if, {} =/=> {}, ret:{}",
                    __FUNCTION__, lif->lif_id, hal_if->if_id, ret);
    return ret;
}

//------------------------------------------------------------------------------
// handling vlan strip en update
//------------------------------------------------------------------------------
hal_ret_t
lif_update_trigger_if (lif_t *lif,
                       bool vlan_strip_en_changed,
                       bool vlan_strip_en,
                       bool vlan_insert_en_changed,
                       bool vlan_insert_en)
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
        if_handle_lif_update(&args);
    }

    // TODO: Handle pinned uplink change. Trigger updates to enic.

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
        HAL_TRACE_DEBUG("{}: if: {}",
                __FUNCTION__, hal_if->if_id);
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
              "pinned_uplink:{}, enable_rdma:{}, hal_handle:{}, num_enics:{} ",
              lif->lif_id, lif->vlan_strip_en, lif->vlan_insert_en,
              lif->pinned_uplink, lif->enable_rdma, lif->hal_handle,
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

//-----------------------------------------------------------------------------
// Print lif spec
//-----------------------------------------------------------------------------
static hal_ret_t
lif_spec_dump (LifSpec& spec)
{
    hal_ret_t           ret = HAL_RET_OK;
    fmt::MemoryWriter   buf;

    buf.write("Lif Spec: ");
    if (spec.has_key_or_handle()) {
        auto kh = spec.key_or_handle();
        if (kh.key_or_handle_case() == LifKeyHandle::kLifId) {
            buf.write("lif_id:{}, ", kh.lif_id());
        } else if (kh.key_or_handle_case() == LifKeyHandle::kLifHandle) {
            buf.write("lif_hdl:{}, ", kh.lif_handle());
        }
    } else {
        buf.write("lif_id_hdl:NULL, ");
    }

    if (spec.has_pinned_uplink_if_key_handle()) {
        auto kh = spec.pinned_uplink_if_key_handle();
        if (kh.key_or_handle_case() == kh::InterfaceKeyHandle::kInterfaceId) {
            buf.write("pinned_uplinkif_id:{}, ", kh.interface_id());
        } else if (kh.key_or_handle_case() == kh::InterfaceKeyHandle::kIfHandle) {
            buf.write("pinned_uplinkif_hdl:{}, ", kh.if_handle());
        }
    } else {
        buf.write("sec_pro_id_hdl:NULL, ");
    }

    buf.write("vlan_strip_en:{}, vlan_insert_en:{}, ",
              spec.vlan_strip_en(), spec.vlan_insert_en());

    buf.write("packet_filters:rec_bcast:{}, rec_prom:{}, rec_mcast:{}",
              spec.packet_filter().receive_broadcast(),
              spec.packet_filter().receive_promiscuous(),
              spec.packet_filter().receive_all_multicast());

    HAL_TRACE_DEBUG("{}", buf.c_str());
    return ret;
}



}    // namespace hal
