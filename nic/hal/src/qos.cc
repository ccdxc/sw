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

static inline void
qos_class_cmap_db_add (qos_class_t *qos_class)
{
    qos_uplink_cmap_t           *cmap = &qos_class->uplink_cmap;
    // Update the global bmps for the cmaps
    if (qos_class_is_user_defined(qos_class)) {
        g_hal_state->qos_cmap_pcp_bmp()->set(cmap->dot1q_pcp);
        for (unsigned i = 0; i < HAL_ARRAY_SIZE(cmap->ip_dscp); i++) {
            if (cmap->ip_dscp[i]) {
                g_hal_state->qos_cmap_dscp_bmp()->set(i);
            }
        }
    }
}

static inline void
qos_class_cmap_db_delete (qos_class_t *qos_class)
{
    qos_uplink_cmap_t           *cmap = &qos_class->uplink_cmap;
    // Update the global bmps for the cmaps
    if (qos_class_is_user_defined(qos_class)) {
        g_hal_state->qos_cmap_pcp_bmp()->clear(cmap->dot1q_pcp);
        for (unsigned i = 0; i < HAL_ARRAY_SIZE(cmap->ip_dscp); i++) {
            if (cmap->ip_dscp[i]) {
                g_hal_state->qos_cmap_dscp_bmp()->clear(i);
            }
        }
    }
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

    HAL_TRACE_DEBUG("adding to qos_class hash table");
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
        HAL_TRACE_ERR("failed to add key to handle mapping, "
                      "err : {}", ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    qos_class_cmap_db_add(qos_class);

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

    HAL_TRACE_DEBUG("removing from hash table");

    // Update the global bmps for the cmaps
    qos_class_cmap_db_delete(qos_class);
    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)g_hal_state->qos_class_ht()->
        remove(&qos_class->key);

    if (entry) {
        // free up
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }

    return HAL_RET_OK;
}

static inline hal_ret_t
qos_class_update_db (qos_class_t *qos_class, qos_class_t *qos_class_clone)
{
    qos_class_cmap_db_delete(qos_class);
    qos_class_cmap_db_add(qos_class_clone);
    return HAL_RET_OK;
}

static void
qos_class_get_fill_rsp (qos::QosClassGetResponse *rsp,
                        qos_class_t *qos_class)
{
    QosClassSpec    *spec;

    spec = rsp->mutable_spec();
    spec->mutable_key_or_handle()->set_qos_class_handle(qos_class->hal_handle);
    spec->set_mtu(qos_class->mtu);
    spec->mutable_buffer()->set_xon_threshold(qos_class->buffer.xon_threshold);
    spec->mutable_buffer()->set_xoff_clear_limit(qos_class->buffer.xoff_clear_limit);
    spec->mutable_pfc()->set_pfc_cos(qos_class->pfc_cos);
    if (qos_class->sched.type == QOS_SCHED_TYPE_DWRR) {
        spec->mutable_sched()->mutable_dwrr()->set_bw_percentage(qos_class->sched.dwrr.bw);
    } else {
        spec->mutable_sched()->mutable_strict()->set_bps(qos_class->sched.strict.bps);
    }
    for (uint32_t i = 0; i < HAL_ARRAY_SIZE(qos_class->uplink_cmap.ip_dscp); i ++) {
        if (qos_class->uplink_cmap.ip_dscp[i]) {
            spec->mutable_uplink_class_map()->add_ip_dscp(i);
        }
    }
    spec->mutable_uplink_class_map()->set_dot1q_pcp(qos_class->uplink_cmap.dot1q_pcp);
    spec->mutable_marking()->set_dot1q_pcp(qos_class->marking.pcp);
    spec->mutable_marking()->set_dot1q_pcp_rewrite_en(qos_class->marking.pcp_rewrite_en);
    spec->mutable_marking()->set_ip_dscp(qos_class->marking.dscp);
    spec->mutable_marking()->set_ip_dscp_rewrite_en(qos_class->marking.dscp_rewrite_en);
}

static bool
qos_class_get_ht_cb(void *ht_entry, void *ctxt)
{
    hal_handle_id_ht_entry_t    *entry      = (hal_handle_id_ht_entry_t *)ht_entry;
    qos::QosClassGetResponseMsg *rsp        = (QosClassGetResponseMsg *)ctxt;
    qos::QosClassGetResponse    *response   = rsp->add_response();
    qos_class_t                 *qos_class  = NULL;

    qos_class = (qos_class_t *)find_qos_class_by_handle(entry->handle_id);
    qos_class_get_fill_rsp(response, qos_class);

    // Always return false here, so that we walk through all hash table
    // entries.
    return false;
}

hal_ret_t
qos_class_get (qos::QosClassGetRequest& req,
               qos::QosClassGetResponseMsg *rsp)
{
    qos_class_t                 *qos_class;
    qos::QosClassGetResponse    *response;

    if (!req.has_key_or_handle()) {
        g_hal_state->qos_class_ht()->walk(qos_class_get_ht_cb, rsp);
        return HAL_RET_OK;
    }

    auto kh = req.key_or_handle();
    response = rsp->add_response();
    qos_class = find_qos_class_by_key_handle(kh);
    if (!qos_class) {
        return HAL_RET_INVALID_ARG;
    }

    qos_class_get_fill_rsp(response, qos_class);

    return HAL_RET_OK;
}

static hal_ret_t
qos_class_free (qos_class_t *qos_class, bool free_pd)
{
    hal_ret_t         ret = HAL_RET_OK;
    pd::pd_qos_class_mem_free_args_t pd_qos_class_args = { 0 };
    if (!qos_class) {
        return HAL_RET_OK;
    }
    if (free_pd) {
        pd::pd_qos_class_mem_free_args_init(&pd_qos_class_args);
        pd_qos_class_args.qos_class = qos_class;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_QOS_CLASS_MEM_FREE, (void *)&pd_qos_class_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("failed to delete qos_class pd, err : {}",
                          ret);
            return ret;
        }
    }
    HAL_SPINLOCK_DESTROY(&qos_class->slock);
    hal::delay_delete_to_slab(HAL_SLAB_QOS_CLASS, qos_class);
    return ret;
}

//-----------------------------------------------------------------------------
// Print qos_class spec
//-----------------------------------------------------------------------------
static hal_ret_t
qos_class_spec_print (QosClassSpec& spec)
{
    fmt::MemoryWriter   buf;

    buf.write("QosClass Spec: ");
    if (spec.has_key_or_handle()) {
        auto kh = spec.key_or_handle();
        if (kh.key_or_handle_case() == QosClassKeyHandle::kQosGroup) {
            buf.write("qos_group:{}, ", kh.qos_group());
        } else if (kh.key_or_handle_case() == QosClassKeyHandle::kQosClassHandle) {
            buf.write("qos_hdl:{}, ", kh.qos_class_handle());
        }
    } else {
        buf.write("qos_class_key_hdl:NULL, ");
    }

    buf.write("MTU:{}, ", spec.mtu());
    if (spec.has_buffer()) {
        buf.write("xon_threshold: {}, xoff_clear_limit: {}, ",
                  spec.buffer().xon_threshold(),
                  spec.buffer().xoff_clear_limit());
    }

    if (spec.has_pfc()) {
        buf.write("pfc_cos: {}, ",
                  spec.pfc().pfc_cos());
    }

    if (spec.has_sched()) {
        if (spec.sched().has_dwrr()) {
            buf.write("sched dwrr percentage: {}, ",
                      spec.sched().dwrr().bw_percentage());
        } else if (spec.sched().has_strict()) {
            buf.write("sched strict bps: {}, ",
                      spec.sched().strict().bps());
        }
    }

    if (spec.has_uplink_class_map()) {
        buf.write("dot1q_pcp: {}, ",
                  spec.uplink_class_map().dot1q_pcp());
        buf.write("ip_dscp: [");
        for (int i = 0; i < spec.uplink_class_map().ip_dscp_size(); i++) {
            buf.write("{} ", spec.uplink_class_map().ip_dscp(i));
        }
        buf.write("], ");
    }

    if (spec.has_marking()) {
        buf.write("dot1q_pcp_rewrite_en:{}, dot1q_pcp:{}, ",
                  spec.marking().dot1q_pcp_rewrite_en(),
                  spec.marking().dot1q_pcp());
        buf.write("ip_dscp_rewrite_en:{}, ip_dscp:{}, ",
                  spec.marking().ip_dscp_rewrite_en(),
                  spec.marking().ip_dscp());
    }

    HAL_TRACE_DEBUG("{}", buf.c_str());
    return HAL_RET_OK;
}

static hal_ret_t
validate_qos_class_spec (QosClassSpec& spec, qos_group_t qos_group)
{
    bool        no_drop = false;

    // mtu should be set
    if (!spec.mtu()) {
        HAL_TRACE_ERR("mtu not set in request");
        return HAL_RET_INVALID_ARG;
    }

    if (spec.has_pfc()) {
        no_drop = true;
    }

    // Validate buffering configuration
    if (!no_drop) {
        if (spec.has_buffer() ||
            spec.buffer().xon_threshold() ||
            spec.buffer().xoff_clear_limit()) {
            HAL_TRACE_ERR("No-drop class buffer params set in request for drop class");
            return HAL_RET_INVALID_ARG;
        }
    } else {
        if (!spec.has_buffer() ||
            !spec.buffer().xon_threshold() ||
            !spec.buffer().xoff_clear_limit()) {
            HAL_TRACE_ERR("No-drop class buffer params not set in request");
            return HAL_RET_INVALID_ARG;
        }
    }

    // Scheduler configuration should be set
    if (!spec.has_sched()) {
        HAL_TRACE_ERR("scheduler not set in request");
        return HAL_RET_INVALID_ARG;
    }

    if (!valid_qos_group(qos_group)) {
        HAL_TRACE_ERR("Not valid qos group {}",
                      spec.key_or_handle().qos_group());
        return HAL_RET_INVALID_ARG;
    }
    // Validate the uplink-class-map
    if (qos_group_is_user_defined(qos_group)) {
        if (!spec.has_uplink_class_map()) {
            HAL_TRACE_ERR("uplink class map not set");
            return HAL_RET_INVALID_ARG;
        }
    } else if (spec.has_uplink_class_map()) {
        HAL_TRACE_ERR("uplink class map set for internal class");
        return HAL_RET_INVALID_ARG;
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// validate an incoming qos_class create request
//------------------------------------------------------------------------------
static hal_ret_t
validate_qos_class_create (QosClassSpec& spec, QosClassResponse *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    qos_group_t qos_group;

    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("qos group not set in request");
        return HAL_RET_INVALID_ARG;
    }

    auto kh = spec.key_or_handle();
    if (kh.key_or_handle_case() != QosClassKeyHandle::kQosGroup) {
        // key-handle field set, but qos-group not provided
        HAL_TRACE_ERR("qos group not set in request");
        return HAL_RET_INVALID_ARG;
    }

    if (find_qos_class_by_key_handle(kh)) {
        HAL_TRACE_ERR("qos class already exists");
        return HAL_RET_ENTRY_EXISTS;
    }

    qos_group = qos_spec_qos_group_to_qos_group(spec.key_or_handle().qos_group());
    ret = validate_qos_class_spec(spec, qos_group);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("qos_class spec validation failed ret {}", ret);
        return ret;
    }

    // Validate the uplink-class-map
    if (qos_group_is_user_defined(qos_group)) {
        // Do validations to check that the dot1q_pcp and ip_dscp are not
        // associated with other classes
        if (g_hal_state->qos_cmap_pcp_bmp()->is_set(spec.uplink_class_map().dot1q_pcp())) {
            HAL_TRACE_ERR("Dot1q pcp {} is already in use",
                          spec.uplink_class_map().dot1q_pcp());
            return HAL_RET_INVALID_ARG;
        }

        for (int i = 0; i < spec.uplink_class_map().ip_dscp_size(); i++) {
            if (g_hal_state->qos_cmap_dscp_bmp()->is_set(spec.uplink_class_map().ip_dscp(i))) {
                HAL_TRACE_ERR("IP dscp {} is already in use",
                              spec.uplink_class_map().ip_dscp(i));
                return HAL_RET_INVALID_ARG;
            }
        }
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
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    qos_class = (qos_class_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("create add CB {}",
                    qos_class->key);

    // PD Call to allocate PD resources and HW programming
    pd::pd_qos_class_create_args_init(&pd_qos_class_args);
    pd_qos_class_args.qos_class = qos_class;
    // ret = pd::pd_qos_class_create(&pd_qos_class_args);
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_QOS_CLASS_CREATE, (void *)&pd_qos_class_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to create qos_class pd, err : {}",
                      ret);
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
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    qos_class = (qos_class_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("create commit CB {}",
                    qos_class->key);

    // Add to DB
    ret = qos_class_add_to_db (qos_class, hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to add qos class:{} to DB",
                      qos_class->key);
        goto end;
    }

    HAL_TRACE_DEBUG("added qos class:{} to DB",
                    qos_class->key);

    // TODO: Increment the ref counts of dependent objects

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
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    qos_class = (qos_class_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("create abort CB {}");

    // 1. delete call to PD
    if (qos_class->pd) {
        pd::pd_qos_class_delete_args_init(&pd_qos_class_args);
        pd_qos_class_args.qos_class = qos_class;
        // ret = pd::pd_qos_class_delete(&pd_qos_class_args);
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_QOS_CLASS_DELETE, (void *)&pd_qos_class_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("failed to delete qos_class pd, err : {}",
                          ret);
        }
    }

    // 2. remove object from hal_handle id based hash table in infra
    hal_handle_free(hal_handle);

    // 3. Free PI qos class
    qos_class_free(qos_class, false);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// Dummy create cleanup callback
// ----------------------------------------------------------------------------
hal_ret_t
qos_class_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
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

    if (spec.has_pfc()) {
        no_drop = true;
        qos_class->pfc_cos = spec.pfc().pfc_cos();
        if (qos_class->pfc_cos >= HAL_MAX_PFC_COS_VALS) {
            HAL_TRACE_ERR("Invalid pfc cos value {}",
                          qos_class->pfc_cos);
            return HAL_RET_INVALID_ARG;
        }
    }

    if (no_drop) {
        if (!spec.has_buffer()) {
            return HAL_RET_INVALID_ARG;
        }

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
        HAL_TRACE_ERR("Invalid class map specified");
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

    hal_api_trace(" API Begin: qos_class create");
    // dump spec
    qos_class_spec_print(spec);

    ret = validate_qos_class_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Validation failed ret {}", ret);
        goto end;
    }

    HAL_TRACE_DEBUG("qos_class create for qos-group {} ",

                    qos_spec_qos_group_to_qos_group(spec.key_or_handle().qos_group()));

    // instantiate qos class
    qos_class = qos_class_alloc_init();
    if (qos_class == NULL) {
        HAL_TRACE_ERR("unable to allocate handle/memory ret: {}",
                      ret);
        ret = HAL_RET_OOM;
        goto end;
    }

    // initialize the qos class record
    HAL_SPINLOCK_INIT(&qos_class->slock, PTHREAD_PROCESS_PRIVATE);

    // populate from the spec
    ret = qos_class_populate_from_spec(qos_class, spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("error in populating qos-class from spec");
        goto end;
    }

    // allocate hal handle id
    qos_class->hal_handle = hal_handle_alloc(HAL_OBJ_ID_QOS_CLASS);
    if (qos_class->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("failed to alloc handle");
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
            qos_class_free(qos_class, true);
            qos_class = NULL;
        }
    }

    qos_class_prepare_rsp(rsp, ret, qos_class ? qos_class->hal_handle : HAL_HANDLE_INVALID);
    hal_api_trace(" API End: qos_class create");
    return ret;
}

//------------------------------------------------------------------------------
// validate qos_class update request
//------------------------------------------------------------------------------
hal_ret_t
validate_qos_class_update (QosClassSpec& spec, QosClassResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;
    qos_class_t *qos_class;

    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("spec has no key or handle");
        ret =  HAL_RET_INVALID_ARG;
    }

    qos_class = find_qos_class_by_key_handle(spec.key_or_handle());
    if (qos_class == NULL) {
        HAL_TRACE_ERR("failed to find qos_class, group {}, handle {}",

                      spec.key_or_handle().qos_group(),
                      spec.key_or_handle().qos_class_handle());
        return HAL_RET_QOS_CLASS_NOT_FOUND;
    }

    ret = validate_qos_class_spec(spec, qos_class->key.qos_group);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("qos_class spec validation failed ret {}", ret);
        return ret;
    }

    return ret;
}

//------------------------------------------------------------------------------
// This is the first call back infra does for update.
// 1. PD Call to update PD
//------------------------------------------------------------------------------
hal_ret_t
qos_class_update_upd_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                      ret = HAL_RET_OK;
    pd::pd_qos_class_update_args_t pd_qos_class_args = { 0 };
    dllist_ctxt_t                  *lnode = NULL;
    dhl_entry_t                    *dhl_entry = NULL;
    qos_class_t                    *qos_class_clone = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-qos_class{}:invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    qos_class_clone = (qos_class_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update upd CB {}",
                    qos_class_clone->key);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_qos_class_update_args_init(&pd_qos_class_args);
    pd_qos_class_args.qos_class = qos_class_clone;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_QOS_CLASS_UPDATE, (void *)&pd_qos_class_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to update qos_class pd, err : {}",
                      ret);
    }

end:
    return ret;
}


//------------------------------------------------------------------------------
// Make a clone
// - Both PI and PD objects cloned.
//------------------------------------------------------------------------------
hal_ret_t
qos_class_make_clone (qos_class_t *qos_class,
                      qos_class_t **qos_class_clone_p,
                      QosClassSpec& spec)
{
    hal_ret_t ret = HAL_RET_OK;
    pd::pd_qos_class_make_clone_args_t args;
    qos_class_t *qos_class_clone;

    *qos_class_clone_p = qos_class_alloc_init();
    qos_class_clone = *qos_class_clone_p;

    qos_class_clone->key = qos_class->key;
    qos_class_clone->hal_handle = qos_class->hal_handle;
    qos_class_clone->pd = NULL;

    args.qos_class = qos_class;
    args.clone = *qos_class_clone_p;
    pd::hal_pd_call(pd::PD_FUNC_ID_QOS_CLASS_MAKE_CLONE, (void *)&args);

    // Update with the new spec
    ret = qos_class_populate_from_spec(qos_class_clone, spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("error in populating qos_class from spec");
        goto end;
    }

end:
    if (ret != HAL_RET_OK) {
        if (*qos_class_clone_p) {
            qos_class_free(*qos_class_clone_p, true);
            *qos_class_clone_p = NULL;
        }
    }
    return ret;
}

//------------------------------------------------------------------------------
// After all hw programming is done
//  1. Free original PI & PD qos_class.
// Note: Infra make clone as original by replacing original pointer by clone.
//------------------------------------------------------------------------------
hal_ret_t
qos_class_update_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t             ret = HAL_RET_OK;
    dllist_ctxt_t         *lnode = NULL;
    dhl_entry_t           *dhl_entry = NULL;
    qos_class_t           *qos_class = NULL;
    qos_class_t           *qos_class_clone = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-qos_class{}:invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    qos_class = (qos_class_t *)dhl_entry->obj;
    qos_class_clone = (qos_class_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update commit CB {}",
                    qos_class->key);

    qos_class_update_db(qos_class, qos_class_clone);

    // Free PI.
    qos_class_free(qos_class, true);
end:
    return ret;
}

//------------------------------------------------------------------------------
// Update didnt go through.
//  1. Kill the clones
//------------------------------------------------------------------------------
hal_ret_t
qos_class_update_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t         ret = HAL_RET_OK;
    dllist_ctxt_t     *lnode = NULL;
    dhl_entry_t       *dhl_entry = NULL;
    qos_class_t             *qos_class_clone = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-qos_class{}:invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    qos_class_clone = (qos_class_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update abort CB {}",
                    qos_class_clone->key);

    // Free Clone
    qos_class_free(qos_class_clone, true);
end:

    return ret;
}

hal_ret_t
qos_class_update_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

static inline hal_ret_t
qos_class_handle_update (QosClassSpec& spec, qos_class_t *qos_class,
                         qos_class_update_app_ctxt_t *app_ctxt)
{
    if (qos_class_is_user_defined(qos_class)) {
        // Do validations to check that the dot1q_pcp and ip_dscp are not
        // associated with other classes
        qos_uplink_cmap_t *cmap = &qos_class->uplink_cmap;
        if (spec.uplink_class_map().dot1q_pcp() != cmap->dot1q_pcp) {
            if (g_hal_state->qos_cmap_pcp_bmp()->is_set(spec.uplink_class_map().dot1q_pcp())) {
                HAL_TRACE_ERR("Dot1q pcp {} is already in use",
                              spec.uplink_class_map().dot1q_pcp());
                return HAL_RET_INVALID_ARG;
            }
        }

        for (int i = 0; i < spec.uplink_class_map().ip_dscp_size(); i++) {
            if (!cmap->ip_dscp[spec.uplink_class_map().ip_dscp(i)]) {
                if (g_hal_state->qos_cmap_dscp_bmp()->is_set(spec.uplink_class_map().ip_dscp(i))) {
                    HAL_TRACE_ERR("IP dscp {} is already in use",
                                  spec.uplink_class_map().ip_dscp(i));
                    return HAL_RET_INVALID_ARG;
                }
            }
        }
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a qos_class update request
//------------------------------------------------------------------------------
hal_ret_t
qos_class_update (QosClassSpec& spec, QosClassResponse *rsp)
{
    hal_ret_t                   ret = HAL_RET_OK;
    qos_class_t                 *qos_class = NULL;
    cfg_op_ctxt_t               cfg_ctxt = { 0 };
    dhl_entry_t                 dhl_entry = { 0 };
    qos_class_update_app_ctxt_t app_ctxt;
    const QosClassKeyHandle     &kh = spec.key_or_handle();

    hal_api_trace(" API Begin: qos_class update");

    // dump spec
    qos_class_spec_print(spec);

    // validate the request message
    ret = validate_qos_class_update(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("qos_class delete validation failed, ret : {}",
                      ret);
        goto end;
    }

    qos_class = find_qos_class_by_key_handle(kh);
    if (qos_class == NULL) {
        HAL_TRACE_ERR("failed to find qos_class, group {}, handle {}",
                      kh.qos_group(), kh.qos_class_handle());
        ret = HAL_RET_QOS_CLASS_NOT_FOUND;
        goto end;
    }
    HAL_TRACE_DEBUG("update qos_class {}",
                    qos_class->key);

    ret = qos_class_handle_update(spec, qos_class, &app_ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("qos_class failed to handle update for {} , ret: {}",
                      qos_class->key,
                      ret);
        goto end;
    }

    qos_class_make_clone(qos_class, (qos_class_t **)&dhl_entry.cloned_obj, spec);

    // form ctxt and call infra update object
    dhl_entry.handle = qos_class->hal_handle;
    dhl_entry.obj = qos_class;
    cfg_ctxt.app_ctxt = NULL;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_upd_obj(qos_class->hal_handle, &cfg_ctxt,
                             qos_class_update_upd_cb,
                             qos_class_update_commit_cb,
                             qos_class_update_abort_cb,
                             qos_class_update_cleanup_cb);

end:
    qos_class_prepare_rsp(rsp, ret,
                          qos_class ? qos_class->hal_handle : HAL_HANDLE_INVALID);
    hal_api_trace(" API End: qos_class update ");
    return ret;
}

//------------------------------------------------------------------------------
// validate qos_class delete request
//------------------------------------------------------------------------------
hal_ret_t
validate_qos_class_delete_req (QosClassDeleteRequest& req, QosClassDeleteResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;
    qos_class_t *qos_class;

    // key-handle field must be set
    if (!req.has_key_or_handle()) {
        HAL_TRACE_ERR("spec has no key or handle");
        ret =  HAL_RET_INVALID_ARG;
    }

    qos_class = find_qos_class_by_key_handle(req.key_or_handle());
    // Deletion is supported for user-defined classes only
    if (qos_class && !qos_class_is_user_defined(qos_class)) {
        HAL_TRACE_ERR("qos_class delete is supported on user defined classes only");
        ret = HAL_RET_INVALID_ARG;
    }

    return ret;
}

//------------------------------------------------------------------------------
// 1. PD Call to delete PD and free up resources and deprogram HW
//------------------------------------------------------------------------------
hal_ret_t
qos_class_delete_del_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                      ret         = HAL_RET_OK;
    pd::pd_qos_class_delete_args_t pd_qos_class_args = { 0 };
    dllist_ctxt_t                  *lnode      = NULL;
    dhl_entry_t                    *dhl_entry  = NULL;
    qos_class_t                    *qos_class        = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // TODO: Check the dependency ref count for the qos_class.
    //       If its non zero, fail the delete.


    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    qos_class = (qos_class_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("delete del CB {} handle {}",
                    qos_class->key, qos_class->hal_handle);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_qos_class_delete_args_init(&pd_qos_class_args);
    pd_qos_class_args.qos_class = qos_class;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_QOS_CLASS_DELETE, (void *)&pd_qos_class_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to delete qos_class pd, err : {}",
                      ret);
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// Update PI DBs as qos_class_delete_del_cb() was a succcess
//      a. Delete from qos_class id hash table
//      b. Remove object from handle id based hash table
//      c. Free PI qos_class
//------------------------------------------------------------------------------
hal_ret_t
qos_class_delete_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t       ret = HAL_RET_OK;
    dllist_ctxt_t   *lnode = NULL;
    dhl_entry_t     *dhl_entry = NULL;
    qos_class_t           *qos_class = NULL;
    hal_handle_t    hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    qos_class = (qos_class_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("delete commit CB {} handle {}",
                    qos_class->key, qos_class->hal_handle);

    // a. Remove from qos_class id hash table
    ret = qos_class_del_from_db(qos_class);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to del qos_class {} from db, err : {}",
                      qos_class->key, ret);
        goto end;
    }

    // b. Remove object from handle id based hash table
    hal_handle_free(hal_handle);

    // c. Free PI qos_class
    qos_class_free(qos_class, false);

end:
    return ret;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
qos_class_delete_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
qos_class_delete_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a qos_class delete request
//------------------------------------------------------------------------------
hal_ret_t
qos_class_delete (QosClassDeleteRequest& req, QosClassDeleteResponse *rsp)
{
    hal_ret_t               ret = HAL_RET_OK;
    qos_class_t             *qos_class = NULL;
    cfg_op_ctxt_t           cfg_ctxt = { 0 };
    dhl_entry_t             dhl_entry = { 0 };
    const QosClassKeyHandle &kh = req.key_or_handle();

    hal_api_trace(" API Begin: qos_class delete ");

    // validate the request message
    ret = validate_qos_class_delete_req(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("qos_class delete validation failed, ret : {}",
                      ret);
        goto end;
    }

    qos_class = find_qos_class_by_key_handle(kh);
    if (qos_class == NULL) {
        HAL_TRACE_ERR("failed to find qos_class, group {}, handle {}",
                      kh.qos_group(), kh.qos_class_handle());
        ret = HAL_RET_QOS_CLASS_NOT_FOUND;
        goto end;
    }

    HAL_TRACE_DEBUG("deleting qos_class {} handle {}",
                    qos_class->key, qos_class->hal_handle);

    // form ctxt and call infra add
    dhl_entry.handle = qos_class->hal_handle;
    dhl_entry.obj = qos_class;
    cfg_ctxt.app_ctxt = NULL;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_del_obj(qos_class->hal_handle, &cfg_ctxt,
                             qos_class_delete_del_cb,
                             qos_class_delete_commit_cb,
                             qos_class_delete_abort_cb,
                             qos_class_delete_cleanup_cb);

end:
    rsp->set_api_status(hal_prepare_rsp(ret));
    hal_api_trace(" API End: qos_class delete ");
    return ret;
}

//------------------------------------------------------------------------------
// Fetch qos-cos-info for tx-scheduler.
//------------------------------------------------------------------------------

hal_ret_t
find_qos_cos_info_from_spec (QosClassKeyHandle kh, hal_handle_t pinned_uplink,
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

    HAL_TRACE_DEBUG("adding to copp hash table");
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
        HAL_TRACE_ERR("failed to add key to handle mapping, "
                      "err : {}", ret);
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

    HAL_TRACE_DEBUG("removing from hash table");

    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)g_hal_state->copp_ht()->
        remove(&copp->key);

    if (entry) {
        // free up
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }

    return HAL_RET_OK;
}

static hal_ret_t
copp_free (copp_t *copp, bool free_pd)
{
    hal_ret_t         ret = HAL_RET_OK;
    pd::pd_copp_mem_free_args_t pd_copp_args = { 0 };
    if (!copp) {
        return HAL_RET_OK;
    }
    if (free_pd) {
        pd::pd_copp_mem_free_args_init(&pd_copp_args);
        pd_copp_args.copp = copp;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_COPP_MEM_FREE, (void *)&pd_copp_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("failed to delete copp pd, err : {}",
                          ret);
            return ret;
        }
    }
    HAL_SPINLOCK_DESTROY(&copp->slock);
    hal::delay_delete_to_slab(HAL_SLAB_COPP, copp);
    return ret;
}

//-----------------------------------------------------------------------------
// Print copp spec
//-----------------------------------------------------------------------------
static hal_ret_t
copp_spec_print (CoppSpec& spec)
{
    fmt::MemoryWriter   buf;

    buf.write("Copp Spec: ");
    if (spec.has_key_or_handle()) {
        auto kh = spec.key_or_handle();
        if (kh.key_or_handle_case() == CoppKeyHandle::kCoppType) {
            buf.write("copp_type:{}, ", kh.copp_type());
        } else if (kh.key_or_handle_case() == CoppKeyHandle::kCoppHandle) {
            buf.write("qos_hdl:{}, ", kh.copp_handle());
        }
    } else {
        buf.write("copp_key_hdl:NULL, ");
    }

    if (spec.has_policer()) {
        buf.write("bps_rate: {}, burst_size: {}, ",
                  spec.policer().bps_rate(),
                  spec.policer().burst_size());
    }

    HAL_TRACE_DEBUG("{}", buf.c_str());
    return HAL_RET_OK;
}

static hal_ret_t
validate_copp_spec (CoppSpec& spec)
{
    // Copp policer rate cannot be zero
    if (!spec.has_policer() || !spec.policer().bps_rate()) {
        HAL_TRACE_ERR("policer spec not set in request");
        return HAL_RET_INVALID_ARG;
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// validate an incoming copp create request
//------------------------------------------------------------------------------
static hal_ret_t
validate_copp_create (CoppSpec& spec)
{
    hal_ret_t ret = HAL_RET_OK;
    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("copp-type not set in request");
        return HAL_RET_INVALID_ARG;
    }

    auto kh = spec.key_or_handle();
    if (kh.key_or_handle_case() != CoppKeyHandle::kCoppType) {
        // key-handle field set, but copp-type not provided
        HAL_TRACE_ERR("copp-type not set in request");
        return HAL_RET_INVALID_ARG;
    }

    if (find_copp_by_key_handle(kh)) {
        HAL_TRACE_ERR("copp already exists");
        return HAL_RET_ENTRY_EXISTS;
    }

    ret = validate_copp_spec(spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("copp spec validation failed ret {}", ret);
        return ret;
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
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    copp = (copp_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("create add CB {}",
                    copp->key);

    // PD Call to allocate PD resources and HW programming
    pd::pd_copp_create_args_init(&pd_copp_args);
    pd_copp_args.copp = copp;
    // ret = pd::pd_copp_create(&pd_copp_args);
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_COPP_CREATE, (void *)&pd_copp_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to create copp pd, err : {}",
                      ret);
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
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    copp = (copp_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("create commit CB {}",
                    copp->key);

    // Add to DB
    ret = copp_add_to_db (copp, hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to add copp:{} to DB",
                      copp->key);
        goto end;
    }

    HAL_TRACE_DEBUG("added copp:{} to DB",
                    copp->key);

    // TODO: Increment the ref counts of dependent objects

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
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    copp = (copp_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("create abort CB {}");

    // 1. delete call to PD
    if (copp->pd) {
        pd::pd_copp_delete_args_init(&pd_copp_args);
        pd_copp_args.copp = copp;
        // ret = pd::pd_copp_delete(&pd_copp_args);
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_COPP_DELETE, (void *)&pd_copp_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("failed to delete copp pd, err : {}",
                          ret);
        }
    }

    // 2. remove object from hal_handle id based hash table in infra
    hal_handle_free(hal_handle);

    // 3. Free PI copp
    copp_free(copp, false);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// Dummy create cleanup callback
// ----------------------------------------------------------------------------
hal_ret_t
copp_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
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

    hal_api_trace(" API Begin: copp create");
    // dump spec
    copp_spec_print(spec);

    ret = validate_copp_create(spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Validation failed ret {}", ret);
        goto end;
    }

    HAL_TRACE_DEBUG("copp create for copp-type {} ",

                    copp_spec_copp_type_to_copp_type(spec.key_or_handle().copp_type()));

    // instantiate copp
    copp = copp_alloc_init();
    if (copp == NULL) {
        HAL_TRACE_ERR("unable to allocate handle/memory ret: {}",
                      ret);
        ret = HAL_RET_OOM;
        goto end;
    }

    // initialize the copp record
    HAL_SPINLOCK_INIT(&copp->slock, PTHREAD_PROCESS_PRIVATE);

    // populate from the spec
    ret = copp_populate_from_spec(copp, spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("error in populating copp from spec");
        goto end;
    }

    // allocate hal handle id
    copp->hal_handle = hal_handle_alloc(HAL_OBJ_ID_COPP);
    if (copp->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("failed to alloc handle");
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
            copp_free(copp, true);
            copp = NULL;
        }
    }

    copp_prepare_rsp(rsp, ret, copp ? copp->hal_handle : HAL_HANDLE_INVALID);
    hal_api_trace(" API End: copp create");
    return ret;
}

//------------------------------------------------------------------------------
// validate copp update request
//------------------------------------------------------------------------------
hal_ret_t
validate_copp_update (CoppSpec& spec, CoppResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("spec has no key or handle");
        ret =  HAL_RET_INVALID_ARG;
    }

    ret = validate_copp_spec(spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("copp spec validation failed ret {}", ret);
        return ret;
    }
    return ret;
}

//------------------------------------------------------------------------------
// This is the first call back infra does for update.
// 1. PD Call to update PD
//------------------------------------------------------------------------------
hal_ret_t
copp_update_upd_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                 ret = HAL_RET_OK;
    pd::pd_copp_update_args_t pd_copp_args = { 0 };
    dllist_ctxt_t             *lnode = NULL;
    dhl_entry_t               *dhl_entry = NULL;
    copp_t                    *copp_clone = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-copp{}:invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    copp_clone = (copp_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update upd CB {}",
                    copp_clone->key);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_copp_update_args_init(&pd_copp_args);
    pd_copp_args.copp = copp_clone;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_COPP_UPDATE, (void *)&pd_copp_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to update copp pd, err : {}",
                      ret);
    }

end:
    return ret;
}


//------------------------------------------------------------------------------
// Make a clone
// - Both PI and PD objects cloned.
//------------------------------------------------------------------------------
hal_ret_t
copp_make_clone (copp_t *copp, copp_t **copp_clone_p, CoppSpec& spec)
{
    hal_ret_t ret = HAL_RET_OK;
    pd::pd_copp_make_clone_args_t args;
    copp_t *copp_clone;

    *copp_clone_p = copp_alloc_init();
    copp_clone = *copp_clone_p;

    copp_clone->key = copp->key;
    copp_clone->hal_handle = copp->hal_handle;
    copp_clone->pd = NULL;

    args.copp = copp;
    args.clone = *copp_clone_p;
    pd::hal_pd_call(pd::PD_FUNC_ID_COPP_MAKE_CLONE, (void *)&args);

    // Update with the new spec
    ret = copp_populate_from_spec(copp_clone, spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("error in populating copp from spec");
        goto end;
    }

end:
    if (ret != HAL_RET_OK) {
        if (*copp_clone_p) {
            copp_free(*copp_clone_p, true);
            *copp_clone_p = NULL;
        }
    }
    return ret;
}

//------------------------------------------------------------------------------
// After all hw programming is done
//  1. Free original PI & PD copp.
// Note: Infra make clone as original by replacing original pointer by clone.
//------------------------------------------------------------------------------
hal_ret_t
copp_update_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t     ret = HAL_RET_OK;
    dllist_ctxt_t *lnode = NULL;
    dhl_entry_t   *dhl_entry = NULL;
    copp_t        *copp = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-copp{}:invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    copp = (copp_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("update commit CB {}",
                    copp->key);

    // Free PI.
    copp_free(copp, true);
end:
    return ret;
}

//------------------------------------------------------------------------------
// Update didnt go through.
//  1. Kill the clones
//------------------------------------------------------------------------------
hal_ret_t
copp_update_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t         ret = HAL_RET_OK;
    dllist_ctxt_t     *lnode = NULL;
    dhl_entry_t       *dhl_entry = NULL;
    copp_t             *copp_clone = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-copp{}:invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    copp_clone = (copp_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update abort CB {}",
                    copp_clone->key);

    // Free Clone
    copp_free(copp_clone, true);
end:

    return ret;
}

hal_ret_t
copp_update_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

static inline hal_ret_t
copp_handle_update (CoppSpec& spec, copp_t *copp,
                    copp_update_app_ctxt_t *app_ctxt)
{
    policer_t policer = {0};

    qos_policer_update_from_spec(spec.policer(), &policer);
    if (!qos_policer_spec_same(&copp->policer, &policer)) {
        app_ctxt->policer_changed = true;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a copp update request
//------------------------------------------------------------------------------
hal_ret_t
copp_update (CoppSpec& spec, CoppResponse *rsp)
{
    hal_ret_t              ret = HAL_RET_OK;
    copp_t                 *copp = NULL;
    cfg_op_ctxt_t          cfg_ctxt = { 0 };
    dhl_entry_t            dhl_entry = { 0 };
    copp_update_app_ctxt_t app_ctxt = { 0 };
    const CoppKeyHandle    &kh = spec.key_or_handle();

    hal_api_trace(" API Begin: copp update");

    // dump spec
    copp_spec_print(spec);

    // validate the request message
    ret = validate_copp_update(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("copp delete validation failed, ret : {}",
                      ret);
        goto end;
    }

    copp = find_copp_by_key_handle(kh);
    if (copp == NULL) {
        HAL_TRACE_ERR("failed to find copp, type {}, handle {}",
                      kh.copp_type(), kh.copp_handle());
        ret = HAL_RET_COPP_NOT_FOUND;
        goto end;
    }
    HAL_TRACE_DEBUG("update copp {}",
                    copp->key);

    ret = copp_handle_update(spec, copp, &app_ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("copp failed to handle update for {} , ret: {}",
                      copp->key,
                      ret);
        goto end;
    }

    if (!app_ctxt.policer_changed) {
        HAL_TRACE_ERR("no change in copp update: noop");
        goto end;
    }

    copp_make_clone(copp, (copp_t **)&dhl_entry.cloned_obj, spec);

    // form ctxt and call infra update object
    dhl_entry.handle = copp->hal_handle;
    dhl_entry.obj = copp;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_upd_obj(copp->hal_handle, &cfg_ctxt,
                             copp_update_upd_cb,
                             copp_update_commit_cb,
                             copp_update_abort_cb,
                             copp_update_cleanup_cb);

end:
    copp_prepare_rsp(rsp, ret,
                     copp ? copp->hal_handle : HAL_HANDLE_INVALID);
    hal_api_trace(" API End: copp update ");
    return ret;
}

//------------------------------------------------------------------------------
// process a copp get request
//------------------------------------------------------------------------------
hal_ret_t
copp_get (CoppGetRequest& req, CoppGetResponse *rsp)
{
    copp_t        *copp;
    CoppSpec      *spec;

    hal_api_trace(" API Begin: copp get ");
    // key-handle field must be set
    if (!req.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    copp = find_copp_by_key_handle(req.key_or_handle());
    if (copp == NULL) {
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_COPP_NOT_FOUND;
    }

    // fill config spec of this copp
    spec = rsp->mutable_spec();
    spec->mutable_key_or_handle()->set_copp_type(
        copp_type_to_spec_type(copp->key.copp_type));
    spec->mutable_policer()->set_bps_rate(copp->policer.bps_rate);
    spec->mutable_policer()->set_burst_size(copp->policer.burst_size);

    // fill operational state of this copp
    rsp->mutable_status()->set_copp_handle(copp->hal_handle);

    // fill stats of this copp
    rsp->set_api_status(types::API_STATUS_OK);
    hal_api_trace(" API End: copp get ");
    return HAL_RET_OK;
}

// copp_delete is not supported

hal_ret_t
hal_qos_init (void)
{
    hal_ret_t   ret = HAL_RET_OK;

    pt::ptree &qos_config_tree = g_hal_state->catalog()->catalog_qos_config_tree();

    auto qos_class_configs = qos_config_tree.get_child_optional("qos_class");
    auto copp_configs = qos_config_tree.get_child_optional("copp");

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
                HAL_TRACE_ERR("Error  creating qos class ret: {}",
                              ret);
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
                HAL_TRACE_ERR("Error  creating copp ret: {}",
                              ret);
                goto end;
            }
        }
    }

end:
    hal_cfg_db_close();
    return ret;

}

}    // namespace hal
