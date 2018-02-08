// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/qos.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/gen/proto/hal/qos.pb.h"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include <google/protobuf/util/json_util.h>

namespace pt = boost::property_tree;

namespace hal {

// ----------------------------------------------------------------------------
// hash table qos_group => ht_entry
//  - Get key from entry
// ----------------------------------------------------------------------------
void *
qos_class_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t    *ht_entry;
    qos_class_t                 *qos_class = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    qos_class = find_qos_class_by_handle(ht_entry->handle_id);
    return (void *)&(qos_class->key);
}

// ----------------------------------------------------------------------------
// hash table qos_group => entry - compute hash
// ----------------------------------------------------------------------------
uint32_t
qos_class_compute_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(qos_class_key_t)) % ht_size;
}

// ----------------------------------------------------------------------------
// hash table qos_group => entry - compare function
// ----------------------------------------------------------------------------
bool
qos_class_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));

    if (!memcmp(key1, key2, sizeof(qos_class_key_t))) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// insert a qos_class to db
//------------------------------------------------------------------------------
static inline hal_ret_t
qos_class_add_to_db (qos_class_t *qos_class, hal_handle_t handle)
{
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    hal_handle_id_ht_entry_t    *entry;
    qos_uplink_cmap_t           *cmap = &qos_class->uplink_cmap;

    HAL_TRACE_DEBUG("{}:adding to qos_class hash table", 
                    __func__);
    // allocate an entry to establish mapping from qos_group to its handle
    entry =
        (hal_handle_id_ht_entry_t *)g_hal_state->
        hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    entry->handle_id = handle;
    sdk_ret = g_hal_state->qos_class_ht()->insert_with_key(&qos_class->key,
                                                   entry, &entry->ht_ctxt);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("{}:failed to add key to handle mapping, "
                      "err : {}", __func__, ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    // Update the global bmps for the cmaps
    if (qos_class_is_user_defined(qos_class)) {
        g_hal_state->qos_cmap_pcp_bmp()->set(cmap->dot1q_pcp);
        for (unsigned i = 0; i < HAL_ARRAY_SIZE(cmap->ip_dscp); i++) {
            if (cmap->ip_dscp[i]) {
                g_hal_state->qos_cmap_dscp_bmp()->set(i);
            }
        }
    }

    // TODO: Check if this is the right place
    qos_class->hal_handle = handle;

    return ret;
}

//------------------------------------------------------------------------------
// delete a qos_class from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
qos_class_del_from_db (qos_class_t *qos_class)
{
    hal_handle_id_ht_entry_t    *entry;
    qos_uplink_cmap_t           *cmap = &qos_class->uplink_cmap;

    HAL_TRACE_DEBUG("{}:removing from hash table", __func__);

    // Update the global bmps for the cmaps
    if (qos_class_is_user_defined(qos_class)) {
        g_hal_state->qos_cmap_pcp_bmp()->clear(cmap->dot1q_pcp);
        for (unsigned i = 0; i < HAL_ARRAY_SIZE(cmap->ip_dscp); i++) {
            if (cmap->ip_dscp[i]) {
                g_hal_state->qos_cmap_dscp_bmp()->clear(i);
            }
        }
    }

    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)g_hal_state->qos_class_ht()->
            remove(&qos_class->key);

    if (entry) {
        // free up
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// validate an incoming qos_class create request
//------------------------------------------------------------------------------
static hal_ret_t
validate_qos_class_create (QosClassSpec& spec, QosClassResponse *rsp)
{
    bool        no_drop = false;
    qos_group_t qos_group;

    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("{}:qos group not set in request", __func__);
        return HAL_RET_INVALID_ARG;
    }

    auto kh = spec.key_or_handle();
    if (kh.key_or_handle_case() != QosClassKeyHandle::kQosGroup) {
        // key-handle field set, but qos-group not provided
        HAL_TRACE_ERR("{}:qos group not set in request", __func__);
        return HAL_RET_INVALID_ARG;
    }

    if (find_qos_class_by_key_handle(kh)) {
        HAL_TRACE_ERR("{}:qos class already exists", __func__);
        return HAL_RET_ENTRY_EXISTS;
    }

    // mtu should be set
    if (!spec.mtu()) {
        HAL_TRACE_ERR("{}:mtu not set in request", __func__);
        return HAL_RET_INVALID_ARG;
    }

    // Buffer configuration should be present
    if (!spec.has_buffer() || !spec.buffer().reserved_mtus()) {
        HAL_TRACE_ERR("{}:buffer not set in request", __func__);
        return HAL_RET_INVALID_ARG;
    }

    if (spec.has_pfc()) {
        no_drop = true;
    }

    // Validate buffering configuration
    if (!no_drop) {
        if (spec.buffer().headroom_mtus() || 
            spec.buffer().xon_threshold() ||
            spec.buffer().xoff_clear_limit()) {
            HAL_TRACE_ERR("{}:No-drop class buffer params set in request for drop class", 
                          __func__);
            return HAL_RET_INVALID_ARG;
        }
    } else {
        if (!spec.buffer().headroom_mtus() || 
            !spec.buffer().xon_threshold() ||
            !spec.buffer().xoff_clear_limit()) {
            HAL_TRACE_ERR("{}:No-drop class buffer params not set in request", 
                          __func__);
            return HAL_RET_INVALID_ARG;
        }
    }

    // Scheduler configuration should be set
    if (!spec.has_sched()) {
        HAL_TRACE_ERR("{}:scheduler not set in request", __func__);
        return HAL_RET_INVALID_ARG;
    }

    qos_group = qos_spec_qos_group_to_qos_group(spec.key_or_handle().qos_group());
    if (!valid_qos_group(qos_group)) {
        HAL_TRACE_ERR("{}: Not valid qos group {}",
                      __func__, spec.key_or_handle().qos_group());
        return HAL_RET_INVALID_ARG;
    }
    // Validate the uplink-class-map
    if (qos_group_is_user_defined(qos_group)) {
        if (!spec.has_uplink_class_map()) {
            HAL_TRACE_ERR("{}:uplink class map not set", __func__);
            return HAL_RET_INVALID_ARG;
        }

        // Do validations to check that the dot1q_pcp and ip_dscp are not 
        // associated with other classes
        if (g_hal_state->qos_cmap_pcp_bmp()->is_set(spec.uplink_class_map().dot1q_pcp())) {
            HAL_TRACE_ERR("{}: Dot1q pcp {} is already in use",
                          __func__, spec.uplink_class_map().dot1q_pcp());
            return HAL_RET_INVALID_ARG;
        }

        for (int i = 0; i < spec.uplink_class_map().ip_dscp_size(); i++) {
            if (g_hal_state->qos_cmap_dscp_bmp()->is_set(spec.uplink_class_map().ip_dscp(i))) {
                HAL_TRACE_ERR("{}: IP dscp {} is already in use",
                              __func__, spec.uplink_class_map().ip_dscp(i));
                return HAL_RET_INVALID_ARG;
            }
        }
    } else if (spec.has_uplink_class_map()) {
        HAL_TRACE_ERR("{}:uplink class map set for internal class", __func__);
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// PD Call to allocate PD resources and HW programming
//------------------------------------------------------------------------------
hal_ret_t
qos_class_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_qos_class_create_args_t     pd_qos_class_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    qos_class_t                 *qos_class = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}: invalid cfg_ctxt", __func__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    qos_class = (qos_class_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("{}:create add CB {}",
                    __func__, qos_class->key);

    // PD Call to allocate PD resources and HW programming
    pd::pd_qos_class_create_args_init(&pd_qos_class_args);
    pd_qos_class_args.qos_class = qos_class;
    // ret = pd::pd_qos_class_create(&pd_qos_class_args);
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_QOS_CLASS_CREATE, (void *)&pd_qos_class_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to create qos_class pd, err : {}", 
                      __func__, ret);
    }

end:
    return ret;
}


//------------------------------------------------------------------------------
// 1. Update PI DBs as qos_class_create_add_cb() was a success
//      a. Add to qos_class id hash table
//------------------------------------------------------------------------------
hal_ret_t
qos_class_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    qos_class_t                 *qos_class = NULL;
    hal_handle_t                hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}:invalid cfg_ctxt", __func__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    qos_class = (qos_class_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("{}:create commit CB {}",
                    __func__, qos_class->key);

    // Add to DB
    ret = qos_class_add_to_db (qos_class, hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:unable to add qos class:{} to DB", 
                      __func__, qos_class->key);
        goto end;
    }

    HAL_TRACE_DEBUG("{}:added qos class:{} to DB", 
                  __func__, qos_class->key);

    // TODO: Increment the ref counts of dependent objects

end:
    return ret;
}

//------------------------------------------------------------------------------
// Qos class Cleanup.
//  - PI Cleanup
//  - Removes the existence of this qos class in HAL
//------------------------------------------------------------------------------
hal_ret_t
qos_class_cleanup(qos_class_t *qos_class)
{
    hal_ret_t       ret = HAL_RET_OK;

    // Remove from DB
    ret = qos_class_del_from_db(qos_class);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:unable to delete qos class from DB", __func__);
        goto end;
    }
    HAL_TRACE_DEBUG("{}:deleted qos class:{} from DB", 
                    __func__, qos_class->key);

    // Free qos class 
    ret = qos_class_free(qos_class);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:unable to free qos class", __func__);
        goto end;
    }

end:
    return ret;
}


//------------------------------------------------------------------------------
// qos_class_create_add_cb was a failure
// 1. call delete to PD
//      a. Deprogram HW
//      b. Clean up resources
//      c. Free PD object
// 2. Remove object from hal_handle id based hash table in infra
// 3. Free PI qos class 
//------------------------------------------------------------------------------
hal_ret_t
qos_class_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t               ret = HAL_RET_OK;
    pd::pd_qos_class_delete_args_t pd_qos_class_args = { 0 };
    dllist_ctxt_t           *lnode = NULL;
    dhl_entry_t             *dhl_entry = NULL;
    qos_class_t             *qos_class = NULL;
    hal_handle_t            hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}:invalid cfg_ctxt", __func__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    qos_class = (qos_class_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("{}:create abort CB {}", __func__);

    // 1. delete call to PD
    if (qos_class->pd) {
        pd::pd_qos_class_delete_args_init(&pd_qos_class_args);
        pd_qos_class_args.qos_class = qos_class;
        // ret = pd::pd_qos_class_delete(&pd_qos_class_args);
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_QOS_CLASS_DELETE, (void *)&pd_qos_class_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("{}:failed to delete qos_class pd, err : {}", 
                          __func__, ret);
        }
    }

    // 2. remove object from hal_handle id based hash table in infra
    hal_handle_free(hal_handle);

    // 3. Free PI qos class
    qos_class_cleanup(qos_class);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// Dummy create cleanup callback
// ----------------------------------------------------------------------------
hal_ret_t
qos_class_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t   ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// Converts hal_ret_t to API status
//------------------------------------------------------------------------------
hal_ret_t
qos_class_prepare_rsp (QosClassResponse *rsp, hal_ret_t ret, 
                       hal_handle_t hal_handle)
{
    if (ret == HAL_RET_OK) {
        rsp->mutable_status()->set_qos_class_handle(hal_handle);
    }

    rsp->set_api_status(hal_prepare_rsp(ret));

    return HAL_RET_OK;
}

static hal_ret_t 
update_buffer_params (QosClassSpec& spec, qos_class_t *qos_class)
{
    qos_buf_t   *buffer = &qos_class->buffer;
    bool        no_drop = false;

    if (!spec.has_buffer()) {
        return HAL_RET_INVALID_ARG;
    }

    if (spec.has_pfc()) {
        no_drop = true;
        qos_class->pfc_cos = spec.pfc().pfc_cos();
        if (qos_class->pfc_cos >= HAL_MAX_PFC_COS_VALS) {
            HAL_TRACE_ERR("{}: Invalid pfc cos value {}", 
                          __func__, qos_class->pfc_cos);
            return HAL_RET_INVALID_ARG;
        }
    }

    buffer->reserved_mtus = spec.buffer().reserved_mtus();
    if (no_drop) {
        buffer->headroom_mtus =  spec.buffer().headroom_mtus();
        buffer->xon_threshold =  spec.buffer().xon_threshold();
        buffer->xoff_clear_limit =  spec.buffer().xoff_clear_limit();
    }

    qos_class->no_drop = no_drop;
    return HAL_RET_OK;
}

static hal_ret_t 
update_sched_params (QosClassSpec& spec, qos_class_t *qos_class)
{
    qos_sched_t *sched = &qos_class->sched;
    if (!spec.has_sched()) {
        return HAL_RET_INVALID_ARG;
    }

    if (spec.sched().has_dwrr()) {
        sched->type = QOS_SCHED_TYPE_DWRR;
        sched->dwrr.bw = spec.sched().dwrr().bw_percentage();
    } else if (spec.sched().has_strict()) {
        sched->type = QOS_SCHED_TYPE_STRICT;
        sched->strict.bps = spec.sched().strict().bps();
    } else {
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

static hal_ret_t
update_cmap_params (QosClassSpec& spec, qos_class_t *qos_class)
{
    qos_uplink_cmap_t *cmap = &qos_class->uplink_cmap;
     

    if (!qos_class_is_user_defined(qos_class)) {
        return HAL_RET_OK;
    }

    if (!spec.has_uplink_class_map()) {
        HAL_TRACE_ERR("{}: Invalid class map specified", 
                      __func__);
        return HAL_RET_INVALID_ARG;
    }

    cmap->dot1q_pcp = spec.uplink_class_map().dot1q_pcp();
    for (int i = 0; i < spec.uplink_class_map().ip_dscp_size(); i++) {
        cmap->ip_dscp[spec.uplink_class_map().ip_dscp(i)] = true;
    }

    return HAL_RET_OK;
}

static hal_ret_t
update_marking_params (QosClassSpec& spec, qos_class_t *qos_class)
{
    qos_marking_action_t *marking = &qos_class->marking;

    if (spec.has_marking()) {
        marking->pcp_rewrite_en = spec.marking().dot1q_pcp_rewrite_en();
        marking->pcp = spec.marking().dot1q_pcp();
        marking->dscp_rewrite_en = spec.marking().ip_dscp_rewrite_en();
        marking->dscp = spec.marking().ip_dscp();
    }

    return HAL_RET_OK;
}

static hal_ret_t
qos_class_populate_from_spec (qos_class_t *qos_class, QosClassSpec& spec)
{
    hal_ret_t ret;

    qos_class->key.qos_group = 
        qos_spec_qos_group_to_qos_group(spec.key_or_handle().qos_group());
    if (!valid_qos_group(qos_class->key.qos_group)) {
        return HAL_RET_INVALID_ARG;
    }

    qos_class->mtu = spec.mtu();

    ret = update_buffer_params(spec, qos_class);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    ret = update_sched_params(spec, qos_class);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    ret = update_cmap_params(spec, qos_class);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    ret = update_marking_params(spec, qos_class);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a qos_class create request
//------------------------------------------------------------------------------
hal_ret_t
qos_class_create (QosClassSpec& spec, QosClassResponse *rsp)
{
    hal_ret_t                   ret = HAL_RET_OK;
    qos_class_t                 *qos_class = NULL;
    dhl_entry_t                 dhl_entry = { 0 };
    cfg_op_ctxt_t               cfg_ctxt = { 0 };

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("{}: qos_class create ", __func__);

    ret = validate_qos_class_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: Validation failed ret {}", __func__, ret);
        goto end;
    }

    HAL_TRACE_DEBUG("{}: qos_class create for qos-group {} ", 
                    __func__, 
                    qos_spec_qos_group_to_qos_group(spec.key_or_handle().qos_group()));

    // instantiate qos class 
    qos_class = qos_class_alloc_init();
    if (qos_class == NULL) {
        HAL_TRACE_ERR("{}:unable to allocate handle/memory ret: {}",
                      __func__, ret);
        ret = HAL_RET_OOM;
        goto end;
    }

    // initialize the qos class record
    HAL_SPINLOCK_INIT(&qos_class->slock, PTHREAD_PROCESS_PRIVATE);

    // populate from the spec
    ret = qos_class_populate_from_spec(qos_class, spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: error in populating qos-class from spec",
                      __func__);
        goto end;
    }

    // allocate hal handle id
    qos_class->hal_handle = hal_handle_alloc(HAL_OBJ_ID_QOS_CLASS);
    if (qos_class->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("{}: failed to alloc handle", 
                      __func__);
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    // form ctxt and call infra add
    // app_ctxt.tenant = tenant;

    dhl_entry.handle = qos_class->hal_handle;
    dhl_entry.obj = qos_class;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(qos_class->hal_handle, &cfg_ctxt, 
                             qos_class_create_add_cb,
                             qos_class_create_commit_cb,
                             qos_class_create_abort_cb, 
                             qos_class_create_cleanup_cb);

    if (ret != HAL_RET_OK) {
        // qos-class was freed during abort, pointer not valid anymore
        qos_class = NULL;
    }
end:
    if (ret != HAL_RET_OK) {
        if (qos_class) {
            qos_class_free(qos_class);
            qos_class = NULL;
        }
    }

    qos_class_prepare_rsp(rsp, ret, qos_class ? qos_class->hal_handle : HAL_HANDLE_INVALID);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}


//------------------------------------------------------------------------------
// Fetch qos-cos-info for tx-scheduler.
//------------------------------------------------------------------------------

hal_ret_t
find_qos_cos_info_from_spec(QosClassKeyHandle kh, hal_handle_t pinned_uplink,
                            uint32_t *cos)
{
    if_t                    *pinned_uplink_if;
    hal_ret_t               ret = HAL_RET_OK;
    qos_class_t             *qos_class;
    pd::pd_qos_class_get_qos_class_id_args_t q_args;

    pinned_uplink_if = find_if_by_handle(pinned_uplink);

    qos_class = find_qos_class_by_key_handle(kh);
    if (qos_class == NULL) {
        HAL_TRACE_ERR("Qos-class does not exist");
        return HAL_RET_QOS_CLASS_NOT_FOUND;
    }
    // Fetch cos from qos-class
    q_args.qos_class= qos_class;
    q_args.dest_if = pinned_uplink_if;
    q_args.qos_class_id = cos;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_GET_QOS_CLASSID, (void *)&q_args);

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error deriving qos-class-id for Qos class "
                      "{} ret {}",
                      qos_class->key, ret);
        return ret;
    }
    return ret;
}

// Copp
// ----------------------------------------------------------------------------
// hash table copp_type => ht_entry
//  - Get key from entry
// ----------------------------------------------------------------------------
void *
copp_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t *ht_entry;
    copp_t                   *copp = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    copp = find_copp_by_handle(ht_entry->handle_id);
    return (void *)&(copp->key);
}

// ----------------------------------------------------------------------------
// hash table copp_type => entry - compute hash
// ----------------------------------------------------------------------------
uint32_t
copp_compute_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(copp_key_t)) % ht_size;
}

// ----------------------------------------------------------------------------
// hash table copp_type => entry - compare function
// ----------------------------------------------------------------------------
bool
copp_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));

    if (!memcmp(key1, key2, sizeof(copp_key_t))) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// insert a copp to db
//------------------------------------------------------------------------------
static inline hal_ret_t
copp_add_to_db (copp_t *copp, hal_handle_t handle)
{
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("{}:adding to copp hash table", 
                    __func__);
    // allocate an entry to establish mapping from copp_type to its handle
    entry =
        (hal_handle_id_ht_entry_t *)g_hal_state->
        hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    entry->handle_id = handle;
    sdk_ret = g_hal_state->copp_ht()->insert_with_key(&copp->key,
                                                   entry, &entry->ht_ctxt);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("{}:failed to add key to handle mapping, "
                      "err : {}", __func__, ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    // TODO: Check if this is the right place
    copp->hal_handle = handle;

    return ret;
}

//------------------------------------------------------------------------------
// delete a copp from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
copp_del_from_db (copp_t *copp)
{
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("{}:removing from hash table", __func__);

    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)g_hal_state->copp_ht()->
            remove(&copp->key);

    if (entry) {
        // free up
         hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// validate an incoming copp create request
//------------------------------------------------------------------------------
static hal_ret_t
validate_copp_create (CoppSpec& spec)
{
    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("{}:copp-type not set in request", __func__);
        return HAL_RET_INVALID_ARG;
    }

    auto kh = spec.key_or_handle();
    if (kh.key_or_handle_case() != CoppKeyHandle::kCoppType) {
        // key-handle field set, but copp-type not provided
        HAL_TRACE_ERR("{}:copp-type not set in request", __func__);
        return HAL_RET_INVALID_ARG;
    }

    if (find_copp_by_key_handle(kh)) {
        HAL_TRACE_ERR("{}:copp already exists", __func__);
        return HAL_RET_ENTRY_EXISTS;
    }

    // Copp policer rate cannot be zero
    if (!spec.has_policer() || !spec.policer().bps_rate()) {
        HAL_TRACE_ERR("{}:policer spec not set in request", __func__);
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// PD Call to allocate PD resources and HW programming
//------------------------------------------------------------------------------
hal_ret_t
copp_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t          ret = HAL_RET_OK;
    pd::pd_copp_create_args_t pd_copp_args = { 0 };
    dllist_ctxt_t      *lnode = NULL;
    dhl_entry_t        *dhl_entry = NULL;
    copp_t             *copp = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}: invalid cfg_ctxt", __func__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    copp = (copp_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("{}:create add CB {}",
                    __func__, copp->key);

    // PD Call to allocate PD resources and HW programming
    pd::pd_copp_create_args_init(&pd_copp_args);
    pd_copp_args.copp = copp;
    // ret = pd::pd_copp_create(&pd_copp_args);
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_COPP_CREATE, (void *)&pd_copp_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to create copp pd, err : {}", 
                      __func__, ret);
    }

end:
    return ret;
}


//------------------------------------------------------------------------------
// 1. Update PI DBs as copp_create_add_cb() was a success
//      a. Add to copp id hash table
//------------------------------------------------------------------------------
hal_ret_t
copp_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t     ret = HAL_RET_OK;
    dllist_ctxt_t *lnode = NULL;
    dhl_entry_t   *dhl_entry = NULL;
    copp_t        *copp = NULL;
    hal_handle_t  hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}:invalid cfg_ctxt", __func__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    copp = (copp_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("{}:create commit CB {}",
                    __func__, copp->key);

    // Add to DB
    ret = copp_add_to_db (copp, hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:unable to add copp:{} to DB", 
                      __func__, copp->key);
        goto end;
    }

    HAL_TRACE_DEBUG("{}:added copp:{} to DB", 
                  __func__, copp->key);

    // TODO: Increment the ref counts of dependent objects

end:
    return ret;
}

//------------------------------------------------------------------------------
// Copp Cleanup.
//  - PI Cleanup
//  - Removes the existence of this copp in HAL
//------------------------------------------------------------------------------
hal_ret_t
copp_cleanup(copp_t *copp)
{
    hal_ret_t       ret = HAL_RET_OK;

    // Remove from DB
    ret = copp_del_from_db(copp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:unable to delete copp from DB", __func__);
        goto end;
    }
    HAL_TRACE_DEBUG("{}:deleted copp:{} from DB", 
                    __func__, copp->key);

    // Free copp 
    ret = copp_free(copp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:unable to free copp", __func__);
        goto end;
    }

end:
    return ret;
}


//------------------------------------------------------------------------------
// copp_create_add_cb was a failure
// 1. call delete to PD
//      a. Deprogram HW
//      b. Clean up resources
//      c. Free PD object
// 2. Remove object from hal_handle id based hash table in infra
// 3. Free PI copp 
//------------------------------------------------------------------------------
hal_ret_t
copp_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t          ret = HAL_RET_OK;
    pd::pd_copp_delete_args_t pd_copp_args = { 0 };
    dllist_ctxt_t      *lnode = NULL;
    dhl_entry_t        *dhl_entry = NULL;
    copp_t             *copp = NULL;
    hal_handle_t       hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}:invalid cfg_ctxt", __func__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    copp = (copp_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("{}:create abort CB {}", __func__);

    // 1. delete call to PD
    if (copp->pd) {
        pd::pd_copp_delete_args_init(&pd_copp_args);
        pd_copp_args.copp = copp;
        // ret = pd::pd_copp_delete(&pd_copp_args);
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_COPP_DELETE, (void *)&pd_copp_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("{}:failed to delete copp pd, err : {}", 
                          __func__, ret);
        }
    }

    // 2. remove object from hal_handle id based hash table in infra
    hal_handle_free(hal_handle);

    // 3. Free PI copp
    copp_cleanup(copp);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// Dummy create cleanup callback
// ----------------------------------------------------------------------------
hal_ret_t
copp_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t   ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// Converts hal_ret_t to API status
//------------------------------------------------------------------------------
hal_ret_t
copp_prepare_rsp (CoppResponse *rsp, hal_ret_t ret, 
                       hal_handle_t hal_handle)
{
    if (ret == HAL_RET_OK) {
        rsp->mutable_status()->set_copp_handle(hal_handle);
    }

    rsp->set_api_status(hal_prepare_rsp(ret));

    return HAL_RET_OK;
}

static hal_ret_t
copp_populate_from_spec (copp_t *copp, CoppSpec& spec)
{
    copp->key.copp_type = 
        copp_spec_copp_type_to_copp_type(spec.key_or_handle().copp_type());
    qos_policer_update_from_spec(spec.policer(), &copp->policer);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a copp create request
//------------------------------------------------------------------------------
hal_ret_t
copp_create (CoppSpec& spec, CoppResponse *rsp)
{
    hal_ret_t     ret = HAL_RET_OK;
    copp_t        *copp = NULL;
    dhl_entry_t   dhl_entry = { 0 };
    cfg_op_ctxt_t cfg_ctxt = { 0 };

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("{}: copp create ", __func__);

    ret = validate_copp_create(spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: Validation failed ret {}", __func__, ret);
        goto end;
    }

    HAL_TRACE_DEBUG("{}: copp create for copp-type {} ", 
                    __func__, 
                    copp_spec_copp_type_to_copp_type(spec.key_or_handle().copp_type()));

    // instantiate copp 
    copp = copp_alloc_init();
    if (copp == NULL) {
        HAL_TRACE_ERR("{}:unable to allocate handle/memory ret: {}",
                      __func__, ret);
        ret = HAL_RET_OOM;
        goto end;
    }

    // initialize the copp record
    HAL_SPINLOCK_INIT(&copp->slock, PTHREAD_PROCESS_PRIVATE);

    // populate from the spec
    ret = copp_populate_from_spec(copp, spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: error in populating copp from spec",
                      __func__);
        goto end;
    }

    // allocate hal handle id
    copp->hal_handle = hal_handle_alloc(HAL_OBJ_ID_COPP);
    if (copp->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("{}: failed to alloc handle", 
                      __func__);
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    // form ctxt and call infra add
    // app_ctxt.tenant = tenant;

    dhl_entry.handle = copp->hal_handle;
    dhl_entry.obj = copp;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(copp->hal_handle, &cfg_ctxt, 
                             copp_create_add_cb,
                             copp_create_commit_cb,
                             copp_create_abort_cb, 
                             copp_create_cleanup_cb);

    if (ret != HAL_RET_OK) {
        // copp was freed during abort, pointer not valid anymore
        copp = NULL;
    }
end:
    if (ret != HAL_RET_OK) {
        if (copp) {
            copp_free(copp);
            copp = NULL;
        }
    }

    copp_prepare_rsp(rsp, ret, copp ? copp->hal_handle : HAL_HANDLE_INVALID);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}

hal_ret_t
hal_qos_init (void)
{
    hal_ret_t   ret = HAL_RET_OK;

    pt::ptree &qos_config_tree = g_hal_state->catalog()->catalog_qos_config_tree();

    auto qos_class_configs = qos_config_tree.get_child_optional("configs.qos_class");
    auto copp_configs = qos_config_tree.get_child_optional("configs.copp");

    hal_cfg_db_open(CFG_OP_WRITE);

    if (qos_class_configs) {
        // Create qos-classes
        for (auto &qos_class_config : *qos_class_configs) {
            std::stringstream ss;
            QosClassSpec qos_class_spec;
            QosClassResponse qos_class_rsp;

            write_json(ss, qos_class_config.second);

            google::protobuf::util::JsonStringToMessage(ss.str(), &qos_class_spec);
            ret = qos_class_create(qos_class_spec, &qos_class_rsp); 
            if (qos_class_rsp.api_status() != types::API_STATUS_OK) {
                HAL_TRACE_ERR("{}: Error  creating qos class ret: {}",
                              __func__, ret);
                goto end;
            }
        }
    }

    if (copp_configs) {
        // Create copp objects
        for (auto &copp_config : *copp_configs) {
            std::stringstream ss;
            CoppSpec copp_spec;
            CoppResponse copp_rsp;

            write_json(ss, copp_config.second);

            google::protobuf::util::JsonStringToMessage(ss.str(), &copp_spec);
            ret = copp_create(copp_spec, &copp_rsp); 
            if (copp_rsp.api_status() != types::API_STATUS_OK) {
                HAL_TRACE_ERR("{}: Error  creating copp ret: {}",
                              __func__, ret);
                goto end;
            }
        }
    }

end:
    hal_cfg_db_close();
    return ret;

}

}    // namespace hal
