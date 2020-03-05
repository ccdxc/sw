//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "gen/hal/include/hal_api_stats.hpp"
#include "nic/hal/plugins/cfg/aclqos/qos.hpp"
#include "nic/sdk/platform/capri/capri_tm_utils.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "gen/proto/qos.pb.h"
#include <google/protobuf/util/json_util.h>

namespace hal {

qos::QosPauseType g_pause_type = qos::QOS_PAUSE_TYPE_NONE;

static inline void
qos_class_lock (qos_class_t *qos_class,
                const char *fname, int lineno, const char *fxname)
{
    SDK_SPINLOCK_LOCK(&qos_class->slock);
}

static inline void
qos_class_unlock (qos_class_t *qos_class,
                  const char *fname, int lineno, const char *fxname)
{
    SDK_SPINLOCK_UNLOCK(&qos_class->slock);
}

static inline qos_pause_type_t
qos_pause_type_spec_to_pause_type (qos::QosPauseType pause_type)
{
    switch (pause_type) {
    case qos::QOS_PAUSE_TYPE_LINK_LEVEL:
        return QOS_PAUSE_TYPE_LINK_LEVEL;

    case qos::QOS_PAUSE_TYPE_PFC:
        return QOS_PAUSE_TYPE_PFC;

    case qos::QOS_PAUSE_TYPE_NONE:
    default:
        return QOS_PAUSE_TYPE_NONE;
    }
}

// validate qos group
static hal_ret_t
validate_group (qos_group_t qos_group)
{
    if (!valid_qos_group(qos_group)) {
        HAL_TRACE_ERR("Not valid qos group {}", qos_group);
        return HAL_RET_QOS_CLASS_QOS_GROUP_INVALID;
    }
    return HAL_RET_OK;
}

// validate MTU
static hal_ret_t
validate_mtu_spec (const QosClassSpec &spec)
{
    uint32_t mtu = spec.mtu();

    // if mtu=0, then pd sets to default value
    if (mtu != 0 && ((mtu < HAL_MIN_MTU) || (mtu > HAL_JUMBO_MTU))) {
        HAL_TRACE_ERR("mtu {} not within {}-{} bytes",
                      mtu, HAL_MIN_MTU, HAL_JUMBO_MTU);
        return HAL_RET_QOS_CLASS_MTU_INVALID;
    }
    return HAL_RET_OK;
}

//validate pause
static hal_ret_t
validate_pause_spec (const QosClassSpec &spec)
{
    if (spec.has_pause()) {
        if (spec.pause().type() != g_pause_type) {
            HAL_TRACE_ERR("config pause type {} does not match "
                          "global pause type {}",
                          spec.pause().type(),
                          g_pause_type);
            return HAL_RET_QOS_CLASS_PAUSE_TYPE_INVALID;
        }
    } else {
        if (g_pause_type == qos::QOS_PAUSE_TYPE_LINK_LEVEL) {
            HAL_TRACE_ERR("Global pause type set to link-level. "
                          "No_drop to drop change not allowed");
            return HAL_RET_QOS_CLASS_PAUSE_TYPE_INVALID;
        }
    }
    return HAL_RET_OK;
}

// validate scheduler
static hal_ret_t
validate_sched_spec (const QosClassSpec &spec)
{
    if (!spec.has_sched()) {
        HAL_TRACE_ERR("scheduler not set in request");
        return HAL_RET_QOS_CLASS_SCHEDULER_NOT_SET;
    }
    if (spec.sched().has_dwrr() &&
        (spec.sched().dwrr().bw_percentage() > 100)) {
        HAL_TRACE_ERR("bw_percentage {} cannot be more than 100!",
                      spec.sched().dwrr().bw_percentage());
        return HAL_RET_QOS_CLASS_DWRR_INVALID;
    }
    return HAL_RET_OK;
}

static inline hal_ret_t
validate_ip_dscp (uint32_t ip_dscp)
{
    if (ip_dscp >= HAL_MAX_IP_DSCP_VALS) {
        HAL_TRACE_ERR("Invalid ip_dscp {} in the uplink class map", ip_dscp);
        return HAL_RET_QOS_CLASS_IP_DSCP_INVALID;
    }
    return HAL_RET_OK;
}

static inline hal_ret_t
validate_dot1q_pcp_exists (uint32_t dot1q_pcp)
{
    if (g_hal_state->qos_cmap_pcp_bmp()->is_set(dot1q_pcp)) {
        HAL_TRACE_ERR("Dot1q pcp {} is already in use", dot1q_pcp);
        return HAL_RET_QOS_CLASS_DOT1Q_PCP_ALREADY_IN_USE;
    }
    return HAL_RET_OK;
}

static inline hal_ret_t
validate_ip_dscp_exists (uint32_t ip_dscp)
{
    if (g_hal_state->qos_cmap_dscp_bmp()->is_set(ip_dscp)) {
        HAL_TRACE_ERR("IP dscp {} is already in use", ip_dscp);
        return HAL_RET_QOS_CLASS_IP_DSCP_ALREADY_IN_USE;
    }
    return HAL_RET_OK;
}

// class map validation for creates
static hal_ret_t
validate_cmap_spec_create (const QosClassSpec &spec)
{
    hal_ret_t ret;
    bool spec_has_pcp = false;
    bool spec_has_dscp = false;
    qos_group_t qos_group;
    uint32_t ip_dscp;

    qos_group =
        qos_spec_qos_group_to_qos_group(spec.key_or_handle().qos_group());
    spec_has_pcp = spec_cmap_type_pcp(spec.class_map().type());
    spec_has_dscp = spec_cmap_type_dscp(spec.class_map().type());

    if (qos_group_is_user_defined(qos_group)) {
        if (spec_has_pcp) {
            ret = validate_dot1q_pcp_exists(spec.class_map().dot1q_pcp());
            if (ret != HAL_RET_OK) {
                return ret;
            }
        }
        if (spec_has_dscp) {
            for (int i = 0; i < spec.class_map().ip_dscp_size(); i++) {
                ip_dscp = spec.class_map().ip_dscp(i);
                ret = validate_ip_dscp(ip_dscp);
                if (ret != HAL_RET_OK) {
                    return ret;
                }
                ret = validate_ip_dscp_exists(ip_dscp);
                if (ret != HAL_RET_OK) {
                    return ret;
                }
            }
        }
    }
    return HAL_RET_OK;
}

// common class map validations for creates and updates
static hal_ret_t
validate_cmap_spec (const QosClassSpec &spec)
{
    bool spec_has_pcp = false;
    qos_group_t qos_group;

    qos_group =
        qos_spec_qos_group_to_qos_group(spec.key_or_handle().qos_group());
    spec_has_pcp = spec_cmap_type_pcp(spec.class_map().type());

    if (qos_group_is_user_defined(qos_group)) {
        // class map must be set for user-defined classes
        if (!spec.has_class_map()) {
            HAL_TRACE_ERR("uplink class map not set");
            return HAL_RET_QOS_CLASS_UPLINK_CLASS_MAP_NOT_SET;
        }
        if (spec_has_pcp) {
            if (spec.class_map().dot1q_pcp() >= HAL_MAX_DOT1Q_PCP_VALS) {
                HAL_TRACE_ERR("Invalid dot1q_pcp {} in the uplink class map",
                              spec.class_map().dot1q_pcp());
                return HAL_RET_QOS_CLASS_DOT1Q_PCP_INVALID;
            }
        }
    } else {
        // class map must not be set for non-user-defined classes
        if (spec.has_class_map()) {
            HAL_TRACE_ERR("uplink class map set for internal class");
            return HAL_RET_QOS_CLASS_UPLINK_CLASS_MAP_SET;
        }
    }
    return HAL_RET_OK;
}

// validate qos marking
static hal_ret_t
validate_marking_spec (const QosClassSpec &spec)
{
    if (spec.has_marking()) {
        if (spec.marking().dot1q_pcp_rewrite_en() &&
            (spec.marking().dot1q_pcp() >= HAL_MAX_DOT1Q_PCP_VALS)) {
            HAL_TRACE_ERR("Invalid dot1q_pcp {} in marking",
                          spec.marking().dot1q_pcp());
            return HAL_RET_QOS_CLASS_DOT1Q_PCP_MARKING_INVALID;
        }
        if (spec.marking().ip_dscp_rewrite_en() &&
            (spec.marking().ip_dscp() >= HAL_MAX_IP_DSCP_VALS)) {
            HAL_TRACE_ERR("Invalid ip_dscp {} in marking",
                          spec.marking().ip_dscp());
            return HAL_RET_QOS_CLASS_IP_DSCP_MARKING_INVALID;
        }
    }
    return HAL_RET_OK;
}

hal_ret_t
qos_class_init_tc_to_iq_map(void)
{
    hal_ret_t ret = HAL_RET_OK;
    pd::pd_func_args_t pd_func_args = {0};

    HAL_TRACE_DEBUG("invoked to init TC to IQ mapping");

    ret = pd::hal_pd_call(pd::PD_FUNC_ID_QOS_CLASS_INIT_TC_TO_IQ_MAP,
                          &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to init TC to IQ mapping, err {}",
                      ret);
    }
    return ret;
}

hal_ret_t
qos_class_set_global_pause_type (
                        qos::QosClassSetGlobalPauseTypeRequest& req,
                        qos::QosClassSetGlobalPauseTypeResponseMsg *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    pd::pd_qos_class_set_global_pause_type_args_t pd_qos_class_args;
    pd::pd_func_args_t pd_func_args = {0};

    HAL_TRACE_DEBUG("invoked to update global_pause_type "
                    "g_pause_type {} req.pause_type() {}", 
                    g_pause_type, req.pause_type());

    if (g_pause_type != req.pause_type()) {
        g_pause_type = req.pause_type();

        // PD Call to set global pause type
        pd::pd_qos_class_set_global_pause_type_init(&pd_qos_class_args);
        pd_qos_class_args.pause_type =
                        qos_pause_type_spec_to_pause_type(g_pause_type);
        pd_func_args.pd_qos_class_set_global_pause_type = &pd_qos_class_args;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_QOS_CLASS_SET_GLOBAL_PAUSE_TYPE,
                              &pd_func_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("failed to set global pause type, err {}",
                          ret);
        }
    }
    return ret;
}

// ----------------------------------------------------------------------------
// hash table qos_group => ht_entry
//  - Get key from entry
// ----------------------------------------------------------------------------
void *
qos_class_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t    *ht_entry;
    qos_class_t                 *qos_class = NULL;

    SDK_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    qos_class = find_qos_class_by_handle(ht_entry->handle_id);
    return (void *)&(qos_class->key);
}

// ----------------------------------------------------------------------------
// hash table qos_group key size
// ----------------------------------------------------------------------------
uint32_t
qos_class_key_size ()
{
    return sizeof(qos_class_key_t);
}

// allocate a QosClass instance
static inline qos_class_t *
qos_class_alloc (void)
{
    qos_class_t    *qos_class;

    qos_class = (qos_class_t *)g_hal_state->qos_class_slab()->alloc();
    if (qos_class == NULL) {
        return NULL;
    }
    return qos_class;
}

// initialize a QosClass instance
static inline qos_class_t *
qos_class_init (qos_class_t *qos_class)
{
    if (!qos_class) {
        return NULL;
    }
    SDK_SPINLOCK_INIT(&qos_class->slock, PTHREAD_PROCESS_SHARED);

    // initialize the operational state
    qos_class->hal_handle   = HAL_HANDLE_INVALID;
    qos_class->pd           = NULL;

    qos_class->lif_list_rx = block_list::factory(sizeof(hal_handle_t));
    qos_class->lif_list_tx = block_list::factory(sizeof(hal_handle_t));

    return qos_class;
}

// allocate and initialize a qos_class instance
static inline qos_class_t *
qos_class_alloc_init (void)
{
    return qos_class_init(qos_class_alloc());
}

qos_class_t *
find_qos_class_by_group (qos_group_t qos_group)
{
    hal_handle_id_ht_entry_t    *entry;
    qos_class_key_t             qos_class_key;
    qos_class_t                 *qos_class;

    qos_class_key.qos_group = qos_group;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
        qos_class_ht()->lookup(&qos_class_key);
    if (entry && (entry->handle_id != HAL_HANDLE_INVALID)) {
        // check for object type
        SDK_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() ==
                   HAL_OBJ_ID_QOS_CLASS);
        qos_class = (qos_class_t *)hal_handle_get_obj(entry->handle_id);
        return qos_class;
    }
    return NULL;
}

qos_class_t *
find_qos_class_by_handle (hal_handle_t handle)
{
    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }
    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_ERR("{}:failed to find object with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_QOS_CLASS) {
        HAL_TRACE_ERR("{}:failed to find qos_class with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    return (qos_class_t *)hal_handle_get_obj(handle);
}

qos_class_t *
find_qos_class_by_key_handle (const QosClassKeyHandle& kh)
{
    if (kh.key_or_handle_case() == QosClassKeyHandle::kQosGroup) {
        qos_group_t qos_group = qos_spec_qos_group_to_qos_group(kh.qos_group());
        return valid_qos_group(qos_group) ?
                                    find_qos_class_by_group(qos_group) : NULL;
    } else if (kh.key_or_handle_case() == QosClassKeyHandle::kQosClassHandle) {
        return find_qos_class_by_handle(kh.qos_class_handle());
    }
    return NULL;
}

static inline void
qos_class_cmap_db_add (qos_class_t *qos_class)
{
    qos_cmap_t *cmap = &qos_class->cmap;

    // Update the global bmps for the cmaps
    if (qos_class_is_user_defined(qos_class)) {
        if (cmap_type_pcp(cmap->type)) {
            g_hal_state->qos_cmap_pcp_bmp()->set(cmap->dot1q_pcp);
        }
        if (cmap_type_dscp(cmap->type)) {
            for (unsigned i = 0; i < SDK_ARRAY_SIZE(cmap->ip_dscp); i++) {
                if (cmap->ip_dscp[i]) {
                    g_hal_state->qos_cmap_dscp_bmp()->set(i);
                }
            }
        }
    }
}

static inline void
qos_class_cmap_db_delete (qos_class_t *qos_class)
{
    qos_cmap_t           *cmap = &qos_class->cmap;
    // Update the global bmps for the cmaps
    if (qos_class_is_user_defined(qos_class)) {
        g_hal_state->qos_cmap_pcp_bmp()->clear(cmap->dot1q_pcp);
        for (unsigned i = 0; i < SDK_ARRAY_SIZE(cmap->ip_dscp); i++) {
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
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("failed to add key to handle mapping, "
                      "err : {}", ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }

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
qos_class_process_get (qos_class_t *qos_class, qos::QosClassGetResponse *rsp)
{
    hal_ret_t                   ret    = HAL_RET_OK;
    pd::pd_qos_class_get_args_t args   = {0};
    QosClassSpec                *spec;
    pd::pd_func_args_t          pd_func_args = {0};

    spec = rsp->mutable_spec();
    spec->mutable_key_or_handle()->set_qos_group(qos_group_to_qos_spec_qos_group(qos_class->key.qos_group));
    spec->set_mtu(qos_class->mtu);
    spec->set_no_drop(qos_class->no_drop);
    spec->mutable_pause()->set_xon_threshold(qos_class->pause.xon_threshold);
    spec->mutable_pause()->set_xoff_threshold(qos_class->pause.xoff_threshold);
    spec->mutable_pause()->set_pfc_cos(qos_class->pause.pfc_cos);
    spec->mutable_pause()->set_type(qos::QOS_PAUSE_TYPE_LINK_LEVEL);
    if (qos_class->pause.pfc_enable == true) {
        spec->mutable_pause()->set_type(qos::QOS_PAUSE_TYPE_PFC);
    }
    if (qos_class->sched.type == QOS_SCHED_TYPE_DWRR) {
        spec->mutable_sched()->mutable_dwrr()->set_bw_percentage(qos_class->sched.dwrr.bw);
    } else {
        spec->mutable_sched()->mutable_strict()->set_bps(qos_class->sched.strict.bps);
    }
    for (uint32_t i = 0; i < SDK_ARRAY_SIZE(qos_class->cmap.ip_dscp); i ++) {
        if (qos_class->cmap.ip_dscp[i]) {
            spec->mutable_class_map()->add_ip_dscp(i);
        }
    }
    spec->mutable_class_map()->set_dot1q_pcp(qos_class->cmap.dot1q_pcp);
    switch (qos_class->cmap.type) {
    case QOS_CMAP_TYPE_NONE:
        spec->mutable_class_map()->set_type(qos::QOS_CLASS_MAP_TYPE_NONE);
        break;
    case QOS_CMAP_TYPE_PCP:
        spec->mutable_class_map()->set_type(qos::QOS_CLASS_MAP_TYPE_PCP);
        break;
    case QOS_CMAP_TYPE_DSCP:
        spec->mutable_class_map()->set_type(qos::QOS_CLASS_MAP_TYPE_DSCP);
        break;
    case QOS_CMAP_TYPE_PCP_DSCP:
        spec->mutable_class_map()->set_type(qos::QOS_CLASS_MAP_TYPE_PCP_DSCP);
        break;
    }
    spec->mutable_marking()->set_dot1q_pcp(qos_class->marking.pcp);
    spec->mutable_marking()->set_dot1q_pcp_rewrite_en(qos_class->marking.pcp_rewrite_en);
    spec->mutable_marking()->set_ip_dscp(qos_class->marking.dscp);
    spec->mutable_marking()->set_ip_dscp_rewrite_en(qos_class->marking.dscp_rewrite_en);

    // fill operational state of this qos_class
    rsp->mutable_status()->set_qos_class_handle(qos_class->hal_handle);

    // fill stats of this qos_class
    rsp->mutable_stats()->set_num_lifs_tx(qos_class->lif_list_tx->num_elems());
    rsp->mutable_stats()->set_num_lifs_rx(qos_class->lif_list_rx->num_elems());

    // Getting PD information
    args.qos_class = qos_class;
    args.rsp = rsp;
    pd_func_args.pd_qos_class_get = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_QOS_CLASS_GET, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to do PD get for qos_class : {}. ret : {}",
                      qos_class->key, ret);
    }

    rsp->set_api_status(types::API_STATUS_OK);
}

static bool
qos_class_get_ht_cb(void *ht_entry, void *ctxt)
{
    hal_handle_id_ht_entry_t    *entry      = (hal_handle_id_ht_entry_t *)ht_entry;
    qos::QosClassGetResponseMsg *rsp        = (QosClassGetResponseMsg *)ctxt;
    qos::QosClassGetResponse    *response   = rsp->add_response();
    qos_class_t                 *qos_class  = NULL;

    qos_class = (qos_class_t *)find_qos_class_by_handle(entry->handle_id);
    qos_class_process_get(qos_class, response);

    // Always return false here, so that we walk through all hash table
    // entries.
    return false;
}

hal_ret_t
qos_class_thresholds_get (qos::QosClassThresholdsGetRequest& req,
                          qos::QosClassThresholdsGetResponseMsg *rsp)
{
    sdk_ret_t sdk_ret = SDK_RET_OK;
    auto response = rsp->add_response();
    sdk::platform::capri::capri_thresholds_t thresholds = {0};

    sdk_ret = sdk::platform::capri::capri_thresholds_get(&thresholds);
    if (sdk_ret != SDK_RET_OK) {
        response->set_api_status(types::API_STATUS_ERR);
        HAL_TRACE_ERR("Unable to do PD get for qos class threshold. ret : {}", hal_sdk_ret_to_hal_ret(sdk_ret));
    }

    for (uint32_t port = TM_UPLINK_PORT_BEGIN; port <= TM_UPLINK_PORT_END; port++) {
        auto port_occupancy = response->add_port_occupancy();
        port_occupancy->set_port_num(port);
        for (uint32_t i = 0; i < CAPRI_QUEUES_PER_PORT; i ++) {
            auto occupancy = port_occupancy->add_occupancy();
            occupancy->set_queue_idx(i);
            occupancy->set_occupancy(thresholds.occupancy[port].queue_occupancy[i]);
        }
    }
    for (uint32_t i = 0; i < CAPRI_TM_MAX_HBM_ETH_CONTEXTS; i ++) {
        auto threshold = response->add_thresholds();
        threshold->set_hbm_context(i);
        threshold->set_xon_threshold(thresholds.threshold[i].xon_threshold);
        threshold->set_xoff_threshold(thresholds.threshold[i].xoff_threshold);
    }
    response->set_api_status(types::API_STATUS_OK);

    return hal_sdk_ret_to_hal_ret(sdk_ret);
}

hal_ret_t
qosclass_get (qos::QosClassGetRequest& req,
              qos::QosClassGetResponseMsg *rsp)
{
    qos_class_t                 *qos_class;
    qos::QosClassGetResponse    *response;

    if (!req.has_key_or_handle()) {
        g_hal_state->qos_class_ht()->walk(qos_class_get_ht_cb, rsp);
        HAL_API_STATS_INC(HAL_API_QOSCLASS_GET_SUCCESS);
        return HAL_RET_OK;
    }

    auto kh = req.key_or_handle();
    response = rsp->add_response();
    qos_class = find_qos_class_by_key_handle(kh);
    if (!qos_class) {
        response->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_QOS_CLASS_NOT_FOUND;
    }

    qos_class_process_get(qos_class, response);
    HAL_API_STATS_INC(HAL_API_QOSCLASS_GET_SUCCESS);
    return HAL_RET_OK;
}

static hal_ret_t
qos_class_free (qos_class_t *qos_class, bool free_pd)
{
    hal_ret_t         ret = HAL_RET_OK;
    pd::pd_qos_class_mem_free_args_t pd_qos_class_args = { 0 };
    pd::pd_func_args_t          pd_func_args = {0};

    if (!qos_class) {
        return HAL_RET_OK;
    }
    if (free_pd) {
        pd::pd_qos_class_mem_free_args_init(&pd_qos_class_args);
        pd_qos_class_args.qos_class = qos_class;
        pd_func_args.pd_qos_class_mem_free = &pd_qos_class_args;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_QOS_CLASS_MEM_FREE, &pd_func_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("failed to delete qos_class pd, err : {}",
                          ret);
            return ret;
        }
    }

    hal::delay_delete_to_slab(HAL_SLAB_QOS_CLASS, qos_class);
    return ret;
}

static hal_ret_t
qos_class_cleanup (qos_class_t *qos_class, bool free_pd)
{
    block_list::destroy(qos_class->lif_list_rx);
    block_list::destroy(qos_class->lif_list_tx);
    SDK_SPINLOCK_DESTROY(&qos_class->slock);
    return qos_class_free(qos_class, free_pd);
}

//-----------------------------------------------------------------------------
// Print qos_class spec
//-----------------------------------------------------------------------------
static hal_ret_t
qos_class_spec_dump (QosClassSpec& spec)
{
    std::string buf;
    google::protobuf::util::JsonPrintOptions options;

    if (hal::utils::hal_trace_level() < ::utils::trace_debug)  {
        return HAL_RET_OK;
    }

    options.add_whitespace = false;
    options.preserve_proto_field_names = true;
    google::protobuf::util::MessageToJsonString(spec, &buf, options);
    HAL_TRACE_DEBUG("QosClass Spec: {}", buf);
    return HAL_RET_OK;
}

static hal_ret_t
validate_qos_class_spec (QosClassSpec& spec)
{
    hal_ret_t ret = HAL_RET_OK;

    // validate MTU
    ret = validate_mtu_spec(spec);
    if (ret != HAL_RET_OK) {
        return ret;
    }
    // validate pause
    ret = validate_pause_spec(spec);
    if (ret != HAL_RET_OK) {
        return ret;
    }
    // validate scheduler
    ret = validate_sched_spec(spec);
    if (ret != HAL_RET_OK) {
        return ret;
    }
    // validate class maps
    ret = validate_cmap_spec(spec);
    if (ret != HAL_RET_OK) {
        return ret;
    }
    // validate qos marking
    ret = validate_marking_spec(spec);
    if (ret != HAL_RET_OK) {
        return ret;
    }
    return ret;
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
    qos_group = qos_spec_qos_group_to_qos_group(
                            spec.key_or_handle().qos_group());
    ret = validate_group(qos_group);
    if (ret != HAL_RET_OK) {
        return ret;
    }
    ret = validate_qos_class_spec(spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("qos_class spec validation failed ret {}", ret);
        return ret;
    }
    ret = validate_cmap_spec_create(spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("qos_class cmap validation failed ret {}", ret);
        return ret;
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
    pd::pd_func_args_t          pd_func_args = {0};

    SDK_ASSERT(cfg_ctxt != NULL);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    qos_class = (qos_class_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("create add cb {}",
                    qos_class->key);

    // PD Call to allocate PD resources and HW programming
    pd::pd_qos_class_create_args_init(&pd_qos_class_args);
    pd_qos_class_args.qos_class = qos_class;
    // ret = pd::pd_qos_class_create(&pd_qos_class_args);
    pd_func_args.pd_qos_class_create = &pd_qos_class_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_QOS_CLASS_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to create qos_class pd, err : {}",
                      ret);
    }

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

    SDK_ASSERT(cfg_ctxt != NULL);

    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    qos_class = (qos_class_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("create commit cb {}",
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
// helper function to cleanup all the qos_class related state during abort operation
// when create failed
//------------------------------------------------------------------------------
static hal_ret_t
qos_class_create_abort_cleanup (qos_class_t *qos_class, hal_handle_t hal_handle)
{
    hal_ret_t                   ret;
    pd::pd_qos_class_delete_args_t    pd_qos_class_args = { 0 };
    pd::pd_func_args_t                pd_func_args = {0};

    // 1. delete call to PD
    if (qos_class->pd) {
        pd::pd_qos_class_delete_args_init(&pd_qos_class_args);
        pd_qos_class_args.qos_class = qos_class;
        pd_func_args.pd_qos_class_delete = &pd_qos_class_args;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_QOS_CLASS_DELETE, &pd_func_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to delete qos_class {} pd, err : {}", qos_class->key, ret);
        }
    }

    // 2. remove object from hal_handle id based hash table in infra
    hal_handle_free(hal_handle);

    // 3. free qos_class
    qos_class_cleanup(qos_class, false);

    return HAL_RET_OK;
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
    dllist_ctxt_t           *lnode = NULL;
    dhl_entry_t             *dhl_entry = NULL;
    qos_class_t             *qos_class = NULL;
    hal_handle_t            hal_handle = 0;

    SDK_ASSERT(cfg_ctxt != NULL);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    qos_class = (qos_class_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("QosClass {} create abort cb", qos_class->key);
    ret = qos_class_create_abort_cleanup(qos_class, hal_handle);

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
update_pfc_params (const QosClassSpec& spec, qos_class_t *qos_class)
{
    //if (spec.has_pause()) {
    if ( spec.no_drop() || 
         (spec.key_or_handle().qos_group() == kh::DEFAULT) ||
         (spec.key_or_handle().qos_group() == kh::SPAN) ) {
        qos_class->pause.xon_threshold = spec.pause().xon_threshold();
        qos_class->pause.xoff_threshold = spec.pause().xoff_threshold();
        qos_class->pause.pfc_cos = spec.pause().pfc_cos();
        if (spec.pause().type() == qos::QOS_PAUSE_TYPE_PFC) {
            qos_class->pause.pfc_enable = true;
        }
        // no_drop is always true for pause
        qos_class->no_drop = true;
    } else {
        qos_class->no_drop = false;
    }
    HAL_TRACE_DEBUG("no_drop {} xon_thr {} xoff_thr {} pfc_cos {} pfc_enable {}", 
                    qos_class->no_drop, qos_class->pause.xon_threshold, 
                    qos_class->pause.xoff_threshold, qos_class->pause.pfc_cos, 
                    qos_class->pause.pfc_enable);
    return HAL_RET_OK;
}

static hal_ret_t
update_sched_params (const QosClassSpec& spec, qos_class_t *qos_class)
{
    qos_sched_t *sched = &qos_class->sched;

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
update_cmap_params (const QosClassSpec& spec, qos_class_t *qos_class)
{
    qos_cmap_t *cmap = &qos_class->cmap;
    bool spec_has_pcp = false;
    bool spec_has_dscp = false;

    spec_has_pcp = spec_cmap_type_pcp(spec.class_map().type());
    spec_has_dscp = spec_cmap_type_dscp(spec.class_map().type());
    if (spec_has_pcp) {
        cmap->dot1q_pcp = spec.class_map().dot1q_pcp();
        cmap->type = QOS_CMAP_TYPE_PCP;
    }
    if (spec_has_dscp) {
        for (int i = 0; i < spec.class_map().ip_dscp_size(); i++) {
            cmap->ip_dscp[spec.class_map().ip_dscp(i)] = true;
        }
        cmap->type = QOS_CMAP_TYPE_DSCP;
    }
    return HAL_RET_OK;
}

static hal_ret_t
update_marking_params (const QosClassSpec& spec, qos_class_t *qos_class)
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
qos_class_init_from_spec (qos_class_t *qos_class, const QosClassSpec& spec)
{
    hal_ret_t ret;

    qos_class->key.qos_group =
        qos_spec_qos_group_to_qos_group(spec.key_or_handle().qos_group());
    qos_class->mtu = spec.mtu();
    ret = update_pfc_params(spec, qos_class);
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
// initialize a qos_class's oper status from its status object
//------------------------------------------------------------------------------
static hal_ret_t
qos_class_init_from_status (qos_class_t *qos_class, const QosClassStatus& status)
{
    qos_class->hal_handle = status.qos_class_handle();
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// initialize a qos_class's oper stats from its stats object
//------------------------------------------------------------------------------
static hal_ret_t
qos_class_init_from_stats (qos_class_t *qos_class, const QosClassStats& stats)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a qos_class create request
//------------------------------------------------------------------------------
hal_ret_t
qosclass_create (QosClassSpec& spec, QosClassResponse *rsp)
{
    hal_ret_t                   ret = HAL_RET_OK;
    qos_class_t                 *qos_class = NULL;
    dhl_entry_t                 dhl_entry = { 0 };
    cfg_op_ctxt_t               cfg_ctxt = { 0 };

    hal_api_trace(" API Begin: qos_class create");
    // dump spec
    qos_class_spec_dump(spec);

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

    // populate from the spec
    ret = qos_class_init_from_spec(qos_class, spec);
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
            // PD wouldn't have been allocated if we're coming here
            // PD gets allocated in create_add_cb and if it failed,
            // create_abort_cb would free everything
            qos_class_cleanup(qos_class, false);
            qos_class = NULL;
        }
        HAL_API_STATS_INC(HAL_API_QOSCLASS_CREATE_FAIL);
    } else {
        HAL_API_STATS_INC(HAL_API_QOSCLASS_CREATE_SUCCESS);
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
    ret = validate_qos_class_spec(spec);
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
    qos_class_update_app_ctxt_t    *app_ctxt = NULL;
    pd::pd_func_args_t             pd_func_args = {0};

    SDK_ASSERT(cfg_ctxt != NULL);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (qos_class_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    qos_class_clone = (qos_class_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update upd cb {}", qos_class_clone->key);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_qos_class_update_args_init(&pd_qos_class_args);
    pd_qos_class_args.qos_class = qos_class_clone;
    pd_qos_class_args.mtu_changed = app_ctxt->mtu_changed;
    pd_qos_class_args.threshold_changed = app_ctxt->threshold_changed;
    pd_qos_class_args.ip_dscp_changed = app_ctxt->ip_dscp_changed;
    pd_qos_class_args.dot1q_pcp_changed = app_ctxt->dot1q_pcp_changed;
    pd_qos_class_args.dot1q_pcp_remove = app_ctxt->dot1q_pcp_remove;
    SDK_ASSERT(sizeof(pd_qos_class_args.ip_dscp_remove) ==
               sizeof(app_ctxt->ip_dscp_remove));
    memcpy(pd_qos_class_args.ip_dscp_remove, app_ctxt->ip_dscp_remove,
           sizeof(app_ctxt->ip_dscp_remove));
    pd_qos_class_args.pfc_cos_changed = app_ctxt->pfc_cos_changed;
    pd_qos_class_args.scheduler_changed = app_ctxt->scheduler_changed;
    pd_qos_class_args.marking_changed = app_ctxt->marking_changed;
    pd_func_args.pd_qos_class_update = &pd_qos_class_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_QOS_CLASS_UPDATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to update qos_class pd, err : {}",
                      ret);
    }

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
    pd::pd_func_args_t pd_func_args = {0};

    *qos_class_clone_p = qos_class_alloc();
    qos_class_clone = *qos_class_clone_p;
    // Assumption is clone will have empty cmap. So not doing memcpy
    // memcpy(qos_class_clone, qos_class, sizeof(qos_class_t));

    qos_class_clone->slock = qos_class->slock;
    qos_class_clone->lif_list_rx = qos_class->lif_list_rx;
    qos_class_clone->lif_list_tx = qos_class->lif_list_tx;
    qos_class_clone->key = qos_class->key;
    qos_class_clone->hal_handle = qos_class->hal_handle;
    qos_class_clone->pd = NULL;

    args.qos_class = qos_class;
    args.clone = *qos_class_clone_p;
    pd_func_args.pd_qos_class_make_clone = &args;
    pd::hal_pd_call(pd::PD_FUNC_ID_QOS_CLASS_MAKE_CLONE, &pd_func_args);

    // Update with the new spec
    ret = qos_class_init_from_spec(qos_class_clone, spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("error in populating qos_class from spec");
        goto end;
    }

end:
    if (ret != HAL_RET_OK) {
        if (*qos_class_clone_p) {
            qos_class_cleanup(*qos_class_clone_p, true);
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

    SDK_ASSERT(cfg_ctxt != NULL);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    qos_class = (qos_class_t *)dhl_entry->obj;
    qos_class_clone = (qos_class_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update commit cb {}",
                    qos_class->key);

    qos_class_update_db(qos_class, qos_class_clone);

    // Free PI.
    qos_class_free(qos_class, true);
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

    SDK_ASSERT(cfg_ctxt != NULL);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    qos_class_clone = (qos_class_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update abort cb {}",
                    qos_class_clone->key);

    // Free Clone
    qos_class_free(qos_class_clone, true);

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
    hal_ret_t ret;
    uint32_t ip_dscp;
    bool ip_dscp_to_rem[HAL_MAX_IP_DSCP_VALS];
    bool spec_has_pcp = false, spec_has_dscp = false;
    bool qos_has_pcp = false, qos_has_dscp = false;

    spec_has_pcp = spec_cmap_type_pcp(spec.class_map().type());
    spec_has_dscp = spec_cmap_type_dscp(spec.class_map().type());
    qos_has_pcp = cmap_type_pcp(qos_class->cmap.type);
    qos_has_dscp = cmap_type_dscp(qos_class->cmap.type);

    if (spec.mtu() != qos_class->mtu) {
        app_ctxt->mtu_changed = true;
    }
    if (qos_class_is_user_defined(qos_class)) {
        qos_cmap_t *cmap = &qos_class->cmap;
        if (qos_has_pcp) {
            if (spec_has_pcp &&
                    (spec.class_map().dot1q_pcp() != cmap->dot1q_pcp)) {
                app_ctxt->dot1q_pcp_changed = true;
                app_ctxt->dot1q_pcp_remove = cmap->dot1q_pcp;
                ret = validate_dot1q_pcp_exists(spec.class_map().dot1q_pcp());
                if (ret != HAL_RET_OK) {
                    return ret;
                }
            } else if (!spec_has_pcp) {
                app_ctxt->dot1q_pcp_changed = true;
                app_ctxt->dot1q_pcp_remove = cmap->dot1q_pcp;
            }
        } else {
            if (spec_has_pcp == true) {
                // update from dot1q_pcp to dscp not allowed
                return HAL_RET_QOS_CLASS_DOT1Q_PCP_INVALID;
            }
        }
        if (qos_has_dscp) {
            memcpy(ip_dscp_to_rem, cmap->ip_dscp, sizeof(ip_dscp_to_rem));
            if (spec_has_dscp) {
                for (int i = 0; i < spec.class_map().ip_dscp_size(); i++) {
                    ip_dscp = spec.class_map().ip_dscp(i);
                    ret = validate_ip_dscp(ip_dscp);
                    if (ret != HAL_RET_OK) {
                        return ret;
                    }
                    // dscp value already being used by this class
                    if (cmap->ip_dscp[ip_dscp] == true) {
                        ip_dscp_to_rem[ip_dscp] = false;
                    } else {
                        // new dscp value
                        app_ctxt->ip_dscp_changed = true;
                        // check if new dscp value is being used by other class
                        ret = validate_ip_dscp_exists(ip_dscp);
                        if (ret != HAL_RET_OK) {
                            return ret;
                        }
                    }
                }
            }
            for (unsigned i = 0; i < SDK_ARRAY_SIZE(ip_dscp_to_rem); i++) {
                if (ip_dscp_to_rem[i]) {
                    app_ctxt->ip_dscp_changed = true;
                    break;
                }
            }
            memcpy(app_ctxt->ip_dscp_remove, ip_dscp_to_rem, sizeof(ip_dscp_to_rem));
        } else {
            if (spec_has_dscp == true) {
                // update from dscp to pcp not allowed
                return HAL_RET_QOS_CLASS_IP_DSCP_INVALID;
            }
        }
    }
    if (spec.has_pause()) {
        // pause type change not allowed
        if ((spec.pause().type() == qos::QOS_PAUSE_TYPE_PFC &&
                qos_class->pause.pfc_enable == false) ||
            (spec.pause().type() == qos::QOS_PAUSE_TYPE_LINK_LEVEL &&
                qos_class->pause.pfc_enable == true)) {
                return HAL_RET_QOS_CLASS_PAUSE_TYPE_INVALID;
        }
        if ((spec.pause().xon_threshold() != qos_class->pause.xon_threshold) ||
            (spec.pause().xoff_threshold() != qos_class->pause.xoff_threshold)) {
            app_ctxt->threshold_changed = true;
        }
        if (spec.pause().pfc_cos() != qos_class->pause.pfc_cos) {
            app_ctxt->pfc_cos_changed = true;
        }
    }
    // allow no_drop<->drop change only for default class
    //if (qos_class->no_drop != spec.has_pause() &&
    if (qos_class->no_drop != spec.no_drop() &&
        qos_class->key.qos_group != QOS_GROUP_DEFAULT) {
        HAL_TRACE_ERR("{} class cannot be changed to {} class",
                      qos_class->no_drop ? "No drop" : "Drop",
                      spec.has_pause() ? "No drop" : "Drop");
        return HAL_RET_QOS_CLASS_DROP_NO_DROP_CHANGE_NOT_ALLOWED;
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a qos_class update request
//------------------------------------------------------------------------------
hal_ret_t
qosclass_update (QosClassSpec& spec, QosClassResponse *rsp)
{
    hal_ret_t                   ret = HAL_RET_OK;
    qos_class_t                 *qos_class = NULL;
    cfg_op_ctxt_t               cfg_ctxt = { 0 };
    dhl_entry_t                 dhl_entry = { 0 };
    qos_class_update_app_ctxt_t app_ctxt;
    const QosClassKeyHandle     &kh = spec.key_or_handle();
    qos_class_t                 *qos_class_clone;

    hal_api_trace(" API Begin: qos_class update");

    // dump spec
    qos_class_spec_dump(spec);

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
    HAL_TRACE_DEBUG("update qos_class {}", qos_class->key);

    ret = qos_class_handle_update(spec, qos_class, &app_ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("qos_class failed to handle update for {} , ret: {}",
                      qos_class->key,
                      ret);
        goto end;
    }

    ret = qos_class_make_clone(qos_class, (qos_class_t **)&dhl_entry.cloned_obj, spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("qos_class failed to make a clone for {} , ret: {}",
                      qos_class->key,
                      ret);
        goto end;
    }

    qos_class_clone = (qos_class_t *)dhl_entry.cloned_obj;

    if (memcmp(&qos_class_clone->sched, &qos_class->sched, sizeof(qos_class->sched))) {
        app_ctxt.scheduler_changed = true;
    }
    if (memcmp(&qos_class_clone->marking, &qos_class->marking, sizeof(qos_class->marking))) {
        app_ctxt.marking_changed = true;
    }

    // form ctxt and call infra update object
    dhl_entry.handle = qos_class->hal_handle;
    dhl_entry.obj = qos_class;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_upd_obj(qos_class->hal_handle, &cfg_ctxt,
                             qos_class_update_upd_cb,
                             qos_class_update_commit_cb,
                             qos_class_update_abort_cb,
                             qos_class_update_cleanup_cb);

end:
    if (ret != HAL_RET_OK) {
        HAL_API_STATS_INC(HAL_API_QOSCLASS_UPDATE_FAIL);
    } else {
        HAL_API_STATS_INC(HAL_API_QOSCLASS_UPDATE_SUCCESS);
    }
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
    pd::pd_func_args_t             pd_func_args = {0};

    SDK_ASSERT(cfg_ctxt != NULL);

    // TODO: Check the dependency ref count for the qos_class.
    //       If its non zero, fail the delete.


    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    qos_class = (qos_class_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("delete del cb {} handle {}",
                    qos_class->key, qos_class->hal_handle);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_qos_class_delete_args_init(&pd_qos_class_args);
    pd_qos_class_args.qos_class = qos_class;
    pd_func_args.pd_qos_class_delete = &pd_qos_class_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_QOS_CLASS_DELETE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to delete qos_class pd, err : {}",
                      ret);
    }

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

    SDK_ASSERT(cfg_ctxt != NULL);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    qos_class = (qos_class_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("delete commit cb {} handle {}",
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
    qos_class_cleanup(qos_class, false);

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

static hal_ret_t
validate_qos_class_delete (qos_class_t *qos_class)
{
    if (qos_class->lif_list_rx->num_elems()) {
        HAL_TRACE_ERR("Qos class delete failure, lifs still referring rx:");
        hal_print_handles_block_list(qos_class->lif_list_rx);
        return HAL_RET_OBJECT_IN_USE;
    }
    if (qos_class->lif_list_tx->num_elems()) {
        HAL_TRACE_ERR("Qos class delete failure, lifs still referring tx:");
        hal_print_handles_block_list(qos_class->lif_list_tx);
        return HAL_RET_OBJECT_IN_USE;
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a qos_class delete request
//------------------------------------------------------------------------------
hal_ret_t
qosclass_delete (QosClassDeleteRequest& req, QosClassDeleteResponse *rsp)
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

    // validate if there no objects referring this qos-class
    ret = validate_qos_class_delete(qos_class);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("qos_class delete validation failed, err : {}", ret);
        goto end;
    }

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
    if (ret == HAL_RET_OK) {
        HAL_API_STATS_INC(HAL_API_QOSCLASS_DELETE_SUCCESS);
    } else {
        HAL_API_STATS_INC(HAL_API_QOSCLASS_DELETE_FAIL);
    }
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
    pd::pd_func_args_t pd_func_args = {0};

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
    pd_func_args.pd_qos_class_get_qos_class_id = &q_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_GET_QOS_CLASSID, &pd_func_args);

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error deriving qos-class-id for Qos class "
                      "{} ret {}",
                      qos_class->key, ret);
        return ret;
    }
    return ret;
}

static hal_ret_t
qos_class_add_lif (qos_class_t *qos_class, lif_t *lif, bool rx)
{
    hal_ret_t                   ret = HAL_RET_OK;

    if (qos_class == NULL || lif == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    qos_class_lock(qos_class, __FILENAME__, __LINE__, __func__);      // lock
    if (rx) {
        ret = qos_class->lif_list_rx->insert(&lif->hal_handle);
    } else {
        ret = qos_class->lif_list_tx->insert(&lif->hal_handle);
    }
    qos_class_unlock(qos_class, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed to add lif {} to qos_class {}",
                        lif->lif_id, qos_class->key);
        goto end;
    }

    HAL_TRACE_DEBUG("Added lif {} to qos_class {}", lif->lif_id, qos_class->key);
end:
    return ret;
}

static hal_ret_t
qos_class_del_lif (qos_class_t *qos_class, lif_t *lif, bool rx)
{
    hal_ret_t                   ret = HAL_RET_OK;

    if (qos_class == NULL || lif == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    qos_class_lock(qos_class, __FILENAME__, __LINE__, __func__);      // lock
    if (rx) {
        ret = qos_class->lif_list_rx->remove(&lif->hal_handle);
    } else {
        ret = qos_class->lif_list_tx->remove(&lif->hal_handle);
    }
    qos_class_unlock(qos_class, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to remove lif {} from from qos_class {}, err : {}",
                       lif->lif_id, qos_class->key, ret);
        goto end;
    }
    HAL_TRACE_DEBUG("Deleted lif {} from qos_class {}", lif->lif_id, qos_class->key);

end:
    return ret;
}

//-----------------------------------------------------------------------------
// add lif to qos_class rx list
//-----------------------------------------------------------------------------
hal_ret_t
qos_class_add_lif_rx (qos_class_t *qos_class, lif_t *lif)
{
    return qos_class_add_lif(qos_class, lif, true);
}

//-----------------------------------------------------------------------------
// remove lif from qos_class rx list
//-----------------------------------------------------------------------------
hal_ret_t
qos_class_del_lif_rx (qos_class_t *qos_class, lif_t *lif)
{
    return qos_class_del_lif(qos_class, lif, true);
}

//-----------------------------------------------------------------------------
// add lif to qos_class tx list
//-----------------------------------------------------------------------------
hal_ret_t
qos_class_add_lif_tx (qos_class_t *qos_class, lif_t *lif)
{
    return qos_class_add_lif(qos_class, lif, false);
}

//-----------------------------------------------------------------------------
// remove lif from qos_class tx list
//-----------------------------------------------------------------------------
hal_ret_t
qos_class_del_lif_tx (qos_class_t *qos_class, lif_t *lif)
{
    return qos_class_del_lif(qos_class, lif, false);
}

//-----------------------------------------------------------------------------
// given a qos_class, marshall it for persisting the qos_class state (spec, status, stats)
//
// obj points to qos_class object i.e., qos_class_t
// mem is the memory buffer to serialize the state into
// len is the length of the buffer provided
// mlen is to be filled by this function with marshalled state length
//-----------------------------------------------------------------------------
hal_ret_t
qos_class_store_cb (void *obj, uint8_t *mem, uint32_t len, uint32_t *mlen)
{
    QosClassGetResponse qos_class_info;
    uint32_t            serialized_state_sz;
    qos_class_t         *qos_class = (qos_class_t *)obj;

    SDK_ASSERT((qos_class != NULL) && (mlen != NULL));
    *mlen = 0;

    // get all information about this qos_class (includes spec, status & stats)
    qos_class_process_get(qos_class, &qos_class_info);
    serialized_state_sz = qos_class_info.ByteSizeLong();
    if (serialized_state_sz > len) {
        HAL_TRACE_ERR("Failed to marshall QOS_CLASS {}, not enough room, "
                      "required size {}, available size {}",
                      qos_class->key, serialized_state_sz, len);
        return HAL_RET_OOM;
    }

    // serialize all the state
    if (qos_class_info.SerializeToArray(mem, serialized_state_sz) == false) {
        HAL_TRACE_ERR("Failed to serialize qos_class {}", qos_class->key);
        return HAL_RET_OOM;
    }
    *mlen = serialized_state_sz;
    HAL_TRACE_DEBUG("Marshalled qos_class {}, len {}",
                    qos_class->key, serialized_state_sz);
    return HAL_RET_OK;
}

static hal_ret_t
qos_class_restore_add (qos_class_t *qos_class, const QosClassGetResponse& qos_class_info)
{
    hal_ret_t                       ret;
    pd::pd_qos_class_restore_args_t pd_qos_class_args = { 0 };
    pd::pd_func_args_t pd_func_args = {0};

    // restore pd state
    pd::pd_qos_class_restore_args_init(&pd_qos_class_args);
    pd_qos_class_args.qos_class = qos_class;
    pd_qos_class_args.qos_class_status = &qos_class_info.status();
    pd_func_args.pd_qos_class_restore = &pd_qos_class_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_QOS_CLASS_RESTORE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to restore qos_class {} pd, err : {}",
                      qos_class->key, ret);
    }
    return ret;
}

static hal_ret_t
qos_class_restore_commit (qos_class_t *qos_class, const QosClassGetResponse& qos_class_info)
{
    hal_ret_t          ret;

    HAL_TRACE_DEBUG("Committing qos_class {} restore", qos_class->key);

    ret = qos_class_add_to_db(qos_class, qos_class->hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to restore qos_class {} to db, err : {}",
                      qos_class->key, ret);
    }
    return ret;
}

static hal_ret_t
qos_class_restore_abort (qos_class_t *qos_class, const QosClassGetResponse& qos_class_info)
{
    HAL_TRACE_ERR("Aborting qos_class {} restore", qos_class->key);
    qos_class_create_abort_cleanup(qos_class, qos_class->hal_handle);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// qos_class's restore cb.
//  - restores qos_class to the PI and PD state before the upgrade
//------------------------------------------------------------------------------
uint32_t
qos_class_restore_cb (void *obj, uint32_t len)
{
    hal_ret_t           ret;
    QosClassGetResponse qos_class_info;
    qos_class_t         *qos_class;

    // de-serialize the object
    if (qos_class_info.ParseFromArray(obj, len) == false) {
        HAL_TRACE_ERR("Failed to de-serialize a serialized qos_class obj");
        SDK_ASSERT(0);
        return 0;
    }

    // allocate QOS_CLASS obj from slab
    qos_class = qos_class_alloc_init();
    if (qos_class == NULL) {
        HAL_TRACE_ERR("Failed to alloc/init qos_class, err : {}", ret);
        return 0;
    }

    // initialize qos_class attrs from its spec
    qos_class_init_from_spec(qos_class, qos_class_info.spec());
    qos_class_init_from_status(qos_class, qos_class_info.status());
    qos_class_init_from_stats(qos_class, qos_class_info.stats());

    // repopulate handle db
    hal_handle_insert(HAL_OBJ_ID_QOS_CLASS, qos_class->hal_handle, (void *)qos_class);

    ret = qos_class_restore_add(qos_class, qos_class_info);
    if (ret != HAL_RET_OK) {
        qos_class_restore_abort(qos_class, qos_class_info);
        qos_class = NULL;
        goto end;
    }
    qos_class_restore_commit(qos_class, qos_class_info);
end:
    if (ret != HAL_RET_OK) {
        if (qos_class) {
            // PD wouldn't have been allocated if we're coming here
            // PD gets allocated in restore_add and if it failed,
            // restore_abort would free everything
            qos_class_free(qos_class, true);
            qos_class = NULL;
        }
    }
    return 0;    // TODO: fix me
}


hal_ret_t 
qos_swm_queue_init(uint32_t swm_uplink_port, uint64_t dmac)
{
    hal_ret_t ret = HAL_RET_OK;
    pd::pd_qos_swm_queue_init_args_t pd_qos_swm_args = {0};
    pd::pd_func_args_t pd_func_args = {0};

    HAL_TRACE_DEBUG("invoked SWM queue init for uplink port {} dmac {}", 
                    swm_uplink_port, dmac);

    // PD Call to init SWM queue
    pd_qos_swm_args.swm_uplink_port = swm_uplink_port;
    pd_qos_swm_args.dmac = dmac;
    pd_func_args.pd_qos_swm_queue_init = &pd_qos_swm_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_QOS_SWM_QUEUE_INIT, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("FAILED to init SWM Queues for uplink {}, err {}",
                      swm_uplink_port, ret);
    }

    return ret;
}

hal_ret_t 
qos_swm_queue_deinit(uint32_t swm_uplink_port)
{
    hal_ret_t ret = HAL_RET_OK;
    pd::pd_qos_swm_queue_deinit_args_t pd_qos_swm_args = {0};
    pd::pd_func_args_t pd_func_args = {0};

    HAL_TRACE_DEBUG("invoked SWM queue de-init for uplink port {}", 
                    swm_uplink_port);

    // PD Call to de-init SWM queue
    pd_qos_swm_args.swm_uplink_port = swm_uplink_port;
    pd_func_args.pd_qos_swm_queue_deinit = &pd_qos_swm_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_QOS_SWM_QUEUE_DEINIT, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("FAILED to deinit SWM Queues for uplink {}, err {}",
                      swm_uplink_port, ret);
    }

    return ret;
}


// Copp
static inline void
copp_lock (copp_t *copp,
           const char *fname, int lineno, const char *fxname)
{
    SDK_SPINLOCK_LOCK(&copp->slock);
}

static inline void
copp_unlock (copp_t *copp,
             const char *fname, int lineno, const char *fxname)
{
    SDK_SPINLOCK_UNLOCK(&copp->slock);
}

// ----------------------------------------------------------------------------
// hash table copp_type => ht_entry
//  - Get key from entry
// ----------------------------------------------------------------------------
void *
copp_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t *ht_entry;
    copp_t                   *copp = NULL;

    SDK_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    copp = find_copp_by_handle(ht_entry->handle_id);
    return (void *)&(copp->key);
}

// ----------------------------------------------------------------------------
// hash table copp_type key size
// ----------------------------------------------------------------------------
uint32_t
copp_key_size ()
{
    return sizeof(copp_key_t);
}

// allocate a Copp instance
static inline copp_t *
copp_alloc (void)
{
    copp_t    *copp;

    copp = (copp_t *)g_hal_state->copp_slab()->alloc();
    if (copp == NULL) {
        return NULL;
    }
    return copp;
}

// initialize a Copp instance
static inline copp_t *
copp_init (copp_t *copp)
{
    if (!copp) {
        return NULL;
    }
    SDK_SPINLOCK_INIT(&copp->slock, PTHREAD_PROCESS_SHARED);

    // initialize the operational state
    copp->hal_handle   = HAL_HANDLE_INVALID;
    copp->pd           = NULL;

    copp->acl_list = block_list::factory(sizeof(hal_handle_t));

    return copp;
}

// allocate and initialize a copp instance
static inline copp_t *
copp_alloc_init (void)
{
    return copp_init(copp_alloc());
}

copp_t *
find_copp_by_copp_type (copp_type_t copp_type)
{
    hal_handle_id_ht_entry_t *entry;
    copp_key_t       copp_key;
    copp_t           *copp;

    copp_key.copp_type = copp_type;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
        copp_ht()->lookup(&copp_key);
    if (entry && (entry->handle_id != HAL_HANDLE_INVALID)) {
        // check for object type
        SDK_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() ==
                   HAL_OBJ_ID_COPP);
        copp = (copp_t *)hal_handle_get_obj(entry->handle_id);
        return copp;
    }
    return NULL;
}

copp_t *
find_copp_by_handle (hal_handle_t handle)
{
    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }
    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_ERR("{}:failed to find object with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_COPP) {
        HAL_TRACE_ERR("{}:failed to find copp with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    return (copp_t *)hal_handle_get_obj(handle);
}

copp_t *
find_copp_by_key_handle (const CoppKeyHandle& kh)
{
    if (kh.key_or_handle_case() == CoppKeyHandle::kCoppType) {
        return find_copp_by_copp_type(copp_spec_copp_type_to_copp_type(kh.copp_type()));
    } else if (kh.key_or_handle_case() == CoppKeyHandle::kCoppHandle) {
        return find_copp_by_handle(kh.copp_handle());
    }
    return NULL;
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
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("failed to add key to handle mapping, "
                      "err : {}", ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }

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
    pd::pd_func_args_t          pd_func_args = {0};

    if (!copp) {
        return HAL_RET_OK;
    }
    if (free_pd) {
        pd::pd_copp_mem_free_args_init(&pd_copp_args);
        pd_copp_args.copp = copp;
        pd_func_args.pd_copp_mem_free = &pd_copp_args;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_COPP_MEM_FREE, &pd_func_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("failed to delete copp pd, err : {}",
                          ret);
            return ret;
        }
    }
    SDK_SPINLOCK_DESTROY(&copp->slock);
    hal::delay_delete_to_slab(HAL_SLAB_COPP, copp);
    return ret;
}

static hal_ret_t
copp_cleanup (copp_t *copp, bool free_pd)
{
    block_list::destroy(copp->acl_list);
    return copp_free(copp, free_pd);
}

//-----------------------------------------------------------------------------
// Print copp spec
//-----------------------------------------------------------------------------
static hal_ret_t
copp_spec_dump (CoppSpec& spec)
{
    std::string buf;
    google::protobuf::util::JsonPrintOptions options;

    if (hal::utils::hal_trace_level() < ::utils::trace_debug)  {
        return HAL_RET_OK;
    }

    options.add_whitespace = false;
    options.preserve_proto_field_names = true;
    google::protobuf::util::MessageToJsonString(spec, &buf, options);
    HAL_TRACE_DEBUG("Copp Spec: {}", buf);
    return HAL_RET_OK;
}

static hal_ret_t
validate_copp_spec (CoppSpec& spec)
{
    // Copp policer rate cannot be zero
    if (!spec.has_policer()) {
        HAL_TRACE_ERR("policer spec not set in request");
        return HAL_RET_INVALID_ARG;
    }

    if ((spec.policer().has_pps_policer() &&
         !spec.policer().pps_policer().packets_per_sec()) ||
        (spec.policer().has_bps_policer() &&
         !spec.policer().bps_policer().bytes_per_sec())) {
        HAL_TRACE_ERR("Copp policer rate not set in request");
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
    pd::pd_func_args_t pd_func_args = {0};

    SDK_ASSERT(cfg_ctxt != NULL);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    copp = (copp_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("create add cb {}",
                    copp->key);

    // PD Call to allocate PD resources and HW programming
    pd::pd_copp_create_args_init(&pd_copp_args);
    pd_copp_args.copp = copp;
    // ret = pd::pd_copp_create(&pd_copp_args);
    pd_func_args.pd_copp_create = &pd_copp_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_COPP_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to create copp pd, err : {}",
                      ret);
    }

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

    SDK_ASSERT(cfg_ctxt != NULL);

    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    copp = (copp_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("create commit cb {}",
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
// helper function to cleanup all the copp related state during abort operation
// when create failed
//------------------------------------------------------------------------------
static hal_ret_t
copp_create_abort_cleanup (copp_t *copp, hal_handle_t hal_handle)
{
    hal_ret_t                   ret;
    pd::pd_copp_delete_args_t   pd_copp_args = { 0 };
    pd::pd_func_args_t          pd_func_args = {0};

    // 1. delete call to PD
    if (copp->pd) {
        pd::pd_copp_delete_args_init(&pd_copp_args);
        pd_copp_args.copp = copp;
        pd_func_args.pd_copp_delete = &pd_copp_args;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_COPP_DELETE, &pd_func_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to delete copp {} pd, err : {}", copp->key, ret);
        }
    }

    // 2. remove object from hal_handle id based hash table in infra
    hal_handle_free(hal_handle);

    // 3. free copp
    copp_cleanup(copp, false);

    return HAL_RET_OK;
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
    dllist_ctxt_t      *lnode = NULL;
    dhl_entry_t        *dhl_entry = NULL;
    copp_t             *copp = NULL;
    hal_handle_t       hal_handle = 0;

    SDK_ASSERT(cfg_ctxt != NULL);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    copp = (copp_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("Copp {} create abort cb", copp->key);
    ret = copp_create_abort_cleanup(copp, hal_handle);

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
copp_init_from_spec (copp_t *copp, const CoppSpec& spec)
{
    copp->key.copp_type =
        copp_spec_copp_type_to_copp_type(spec.key_or_handle().copp_type());
    qos_policer_update_from_spec(spec.policer(), &copp->policer);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// initialize a copp's oper status from its status object
//------------------------------------------------------------------------------
static hal_ret_t
copp_init_from_status (copp_t *copp, const CoppStatus& status)
{
    copp->hal_handle = status.copp_handle();
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// initialize a copp's oper stats from its stats object
//------------------------------------------------------------------------------
static hal_ret_t
copp_init_from_stats (copp_t *copp, const CoppStats& stats)
{
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
    copp_spec_dump(spec);

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
    SDK_SPINLOCK_INIT(&copp->slock, PTHREAD_PROCESS_SHARED);

    // populate from the spec
    ret = copp_init_from_spec(copp, spec);
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
            // PD wouldn't have been allocated if we're coming here
            // PD gets allocated in create_add_cb and if it failed,
            // create_abort_cb would free everything
            copp_cleanup(copp, false);
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
    pd::pd_func_args_t        pd_func_args = {0};

    SDK_ASSERT(cfg_ctxt != NULL);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    copp_clone = (copp_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update upd cb {}",
                    copp_clone->key);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_copp_update_args_init(&pd_copp_args);
    pd_copp_args.copp = copp_clone;
    pd_func_args.pd_copp_update = &pd_copp_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_COPP_UPDATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to update copp pd, err : {}",
                      ret);
    }

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
    pd::pd_func_args_t pd_func_args = {0};

    *copp_clone_p = copp_alloc_init();
    copp_clone = *copp_clone_p;

    copp_clone->key = copp->key;
    copp_clone->hal_handle = copp->hal_handle;
    copp_clone->pd = NULL;

    args.copp = copp;
    args.clone = *copp_clone_p;
    pd_func_args.pd_copp_make_clone = &args;
    pd::hal_pd_call(pd::PD_FUNC_ID_COPP_MAKE_CLONE, &pd_func_args);

    // Update with the new spec
    ret = copp_init_from_spec(copp_clone, spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("error in populating copp from spec");
        goto end;
    }

end:
    if (ret != HAL_RET_OK) {
        if (*copp_clone_p) {
            copp_cleanup(*copp_clone_p, true);
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

    SDK_ASSERT(cfg_ctxt != NULL);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    copp = (copp_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("update commit cb {}",
                    copp->key);

    // Free PI.
    copp_free(copp, true);
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

    SDK_ASSERT(cfg_ctxt != NULL);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    copp_clone = (copp_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update abort cb {}",
                    copp_clone->key);

    // Free Clone
    copp_free(copp_clone, true);

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
    policer_t policer = { POLICER_TYPE_PPS };

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
    copp_spec_dump(spec);

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
    if (ret == HAL_RET_OK) {
        HAL_API_STATS_INC(HAL_API_COPP_UPDATE_SUCCESS);
    } else {
        HAL_API_STATS_INC(HAL_API_COPP_UPDATE_FAIL);
    }
    copp_prepare_rsp(rsp, ret,
                     copp ? copp->hal_handle : HAL_HANDLE_INVALID);
    hal_api_trace(" API End: copp update ");
    return ret;
}

static void
copp_process_get (copp_t *copp, qos::CoppGetResponse *rsp)
{
    hal_ret_t              ret    = HAL_RET_OK;
    pd::pd_copp_get_args_t args   = {0};
    CoppSpec               *spec;
    pd::pd_func_args_t     pd_func_args = {0};

    spec = rsp->mutable_spec();

    // fill config spec of this copp
    spec = rsp->mutable_spec();
    spec->mutable_key_or_handle()->set_copp_type(
        copp_type_to_spec_type(copp->key.copp_type));
    qos_policer_to_spec(&copp->policer, spec->mutable_policer());

    // fill operational state of this copp
    rsp->mutable_status()->set_copp_handle(copp->hal_handle);

    // TODO: fill stats of this copp
    rsp->mutable_stats()->set_num_acls(copp->acl_list->num_elems());


    // Getting PD information
    args.copp = copp;
    args.rsp = rsp;
    pd_func_args.pd_copp_get = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_COPP_GET, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to do PD get for copp : {}. ret : {}",
                      copp->key, ret);
    }

    rsp->set_api_status(types::API_STATUS_OK);
}

static bool
copp_get_ht_cb(void *ht_entry, void *ctxt)
{
    hal_handle_id_ht_entry_t *entry      = (hal_handle_id_ht_entry_t *)ht_entry;
    qos::CoppGetResponseMsg  *rsp        = (CoppGetResponseMsg *)ctxt;
    qos::CoppGetResponse     *response   = rsp->add_response();
    copp_t                   *copp       = NULL;

    copp = (copp_t *)find_copp_by_handle(entry->handle_id);
    copp_process_get(copp, response);

    // Always return false here, so that we walk through all hash table
    // entries.
    return false;
}

//------------------------------------------------------------------------------
// process a copp get request
//------------------------------------------------------------------------------
hal_ret_t
copp_get (CoppGetRequest& req, CoppGetResponseMsg *rsp)
{
    copp_t          *copp;
    CoppGetResponse *response;

    if (!req.has_key_or_handle()) {
        g_hal_state->copp_ht()->walk(copp_get_ht_cb, rsp);
        HAL_API_STATS_INC(HAL_API_COPP_GET_SUCCESS);
        return HAL_RET_OK;
    }

    auto kh = req.key_or_handle();
    response = rsp->add_response();
    copp = find_copp_by_key_handle(kh);
    if (!copp) {
        response->set_api_status(types::API_STATUS_NOT_FOUND);
        HAL_API_STATS_INC(HAL_API_COPP_GET_FAIL);
        return HAL_RET_COPP_NOT_FOUND;
    }

    copp_process_get(copp, response);
    HAL_API_STATS_INC(HAL_API_COPP_GET_SUCCESS);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// validate copp delete request
//------------------------------------------------------------------------------
hal_ret_t
validate_copp_delete_req (CoppDeleteRequest& req, CoppDeleteResponse *rsp)
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
copp_delete_del_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                 ret         = HAL_RET_OK;
    pd::pd_copp_delete_args_t pd_copp_args = { 0 };
    dllist_ctxt_t             *lnode      = NULL;
    dhl_entry_t               *dhl_entry  = NULL;
    copp_t                    *copp        = NULL;
    pd::pd_func_args_t        pd_func_args = {0};

    SDK_ASSERT(cfg_ctxt != NULL);

    // TODO: Check the dependency ref count for the copp.
    //       If its non zero, fail the delete.


    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    copp = (copp_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("delete del cb {} handle {}",
                    copp->key, copp->hal_handle);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_copp_delete_args_init(&pd_copp_args);
    pd_copp_args.copp = copp;
    pd_func_args.pd_copp_delete = &pd_copp_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_COPP_DELETE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to delete copp pd, err : {}",
                      ret);
    }

    return ret;
}

//------------------------------------------------------------------------------
// Update PI DBs as copp_delete_del_cb() was a succcess
//      a. Delete from copp id hash table
//      b. Remove object from handle id based hash table
//      c. Free PI copp
//------------------------------------------------------------------------------
hal_ret_t
copp_delete_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t       ret = HAL_RET_OK;
    dllist_ctxt_t   *lnode = NULL;
    dhl_entry_t     *dhl_entry = NULL;
    copp_t           *copp = NULL;
    hal_handle_t    hal_handle = 0;

    SDK_ASSERT(cfg_ctxt != NULL);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    copp = (copp_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("delete commit cb {} handle {}",
                    copp->key, copp->hal_handle);

    // a. Remove from copp id hash table
    ret = copp_del_from_db(copp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to del copp {} from db, err : {}",
                      copp->key, ret);
        goto end;
    }

    // b. Remove object from handle id based hash table
    hal_handle_free(hal_handle);

    // c. Free PI copp
    copp_cleanup(copp, false);

end:
    return ret;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
copp_delete_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
copp_delete_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

static hal_ret_t
validate_copp_delete (copp_t *copp)
{
    if (copp->acl_list->num_elems()) {
        HAL_TRACE_ERR("Copp delete failure, acls still referring :");
        hal_print_handles_block_list(copp->acl_list);
        return HAL_RET_OBJECT_IN_USE;
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a copp delete request
//------------------------------------------------------------------------------
hal_ret_t
copp_delete (CoppDeleteRequest& req, CoppDeleteResponse *rsp)
{
    hal_ret_t           ret = HAL_RET_OK;
    copp_t              *copp = NULL;
    cfg_op_ctxt_t       cfg_ctxt = { 0 };
    dhl_entry_t         dhl_entry = { 0 };
    const CoppKeyHandle &kh = req.key_or_handle();

    hal_api_trace(" API Begin: copp delete ");

    // validate the request message
    ret = validate_copp_delete_req(req, rsp);
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

    HAL_TRACE_DEBUG("deleting copp {} handle {}",
                    copp->key, copp->hal_handle);

    // validate if there no objects referring this copp
    ret = validate_copp_delete(copp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("copp delete validation failed, err : {}", ret);
        goto end;
    }

    // form ctxt and call infra add
    dhl_entry.handle = copp->hal_handle;
    dhl_entry.obj = copp;
    cfg_ctxt.app_ctxt = NULL;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_del_obj(copp->hal_handle, &cfg_ctxt,
                             copp_delete_del_cb,
                             copp_delete_commit_cb,
                             copp_delete_abort_cb,
                             copp_delete_cleanup_cb);

end:
    rsp->set_api_status(hal_prepare_rsp(ret));
    hal_api_trace(" API End: copp delete ");
    return ret;
}

//-----------------------------------------------------------------------------
// add acl to copp list
//-----------------------------------------------------------------------------
hal_ret_t
copp_add_acl (copp_t *copp, acl_t *acl)
{
    hal_ret_t                   ret = HAL_RET_OK;

    if (copp == NULL || acl == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    copp_lock(copp, __FILENAME__, __LINE__, __func__);      // lock
    ret = copp->acl_list->insert(&acl->hal_handle);
    copp_unlock(copp, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed to add acl {} to copp {}",
                        acl->key, copp->key);
        goto end;
    }

    HAL_TRACE_DEBUG("Added acl {} to copp {}", acl->key, copp->key);
end:
    return ret;
}

//-----------------------------------------------------------------------------
// remove acl from copp list
//-----------------------------------------------------------------------------
hal_ret_t
copp_del_acl (copp_t *copp, acl_t *acl)
{
    hal_ret_t                   ret = HAL_RET_OK;

    if (copp == NULL || acl == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    copp_lock(copp, __FILENAME__, __LINE__, __func__);      // lock
    ret = copp->acl_list->remove(&acl->hal_handle);
    copp_unlock(copp, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to remove acl {} from from copp {}, err : {}",
                       acl->key, copp->key, ret);
        goto end;
    }
    HAL_TRACE_DEBUG("Deleted acl {} from copp {}", acl->key, copp->key);

end:
    return ret;
}

//-----------------------------------------------------------------------------
// given a copp, marshall it for persisting the copp state (spec, status, stats)
//
// obj points to copp object i.e., copp_t
// mem is the memory buffer to serialize the state into
// len is the length of the buffer provided
// mlen is to be filled by this function with marshalled state length
//-----------------------------------------------------------------------------
hal_ret_t
copp_store_cb (void *obj, uint8_t *mem, uint32_t len, uint32_t *mlen)
{
    CoppGetResponse copp_info;
    uint32_t        serialized_state_sz;
    copp_t          *copp = (copp_t *)obj;

    SDK_ASSERT((copp != NULL) && (mlen != NULL));
    *mlen = 0;

    // get all information about this copp (includes spec, status & stats)
    copp_process_get(copp, &copp_info);
    serialized_state_sz = copp_info.ByteSizeLong();
    if (serialized_state_sz > len) {
        HAL_TRACE_ERR("Failed to marshall COPP {}, not enough room, "
                      "required size {}, available size {}",
                      copp->key, serialized_state_sz, len);
        return HAL_RET_OOM;
    }

    // serialize all the state
    if (copp_info.SerializeToArray(mem, serialized_state_sz) == false) {
        HAL_TRACE_ERR("Failed to serialize copp {}", copp->key);
        return HAL_RET_OOM;
    }
    *mlen = serialized_state_sz;
    HAL_TRACE_DEBUG("Marshalled copp {}, len {}",
                    copp->key, serialized_state_sz);
    return HAL_RET_OK;
}

static hal_ret_t
copp_restore_add (copp_t *copp, const CoppGetResponse& copp_info)
{
    hal_ret_t                  ret;
    pd::pd_copp_restore_args_t pd_copp_args = { 0 };
    pd::pd_func_args_t         pd_func_args = {0};

    // restore pd state
    pd::pd_copp_restore_args_init(&pd_copp_args);
    pd_copp_args.copp = copp;
    pd_copp_args.copp_status = &copp_info.status();
    pd_func_args.pd_copp_restore = &pd_copp_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_COPP_RESTORE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to restore copp {} pd, err : {}",
                      copp->key, ret);
    }
    return ret;
}

static hal_ret_t
copp_restore_commit (copp_t *copp, const CoppGetResponse& copp_info)
{
    hal_ret_t          ret;

    HAL_TRACE_DEBUG("Committing copp {} restore", copp->key);

    ret = copp_add_to_db(copp, copp->hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to restore copp {} to db, err : {}",
                      copp->key, ret);
    }
    return ret;
}

static hal_ret_t
copp_restore_abort (copp_t *copp, const CoppGetResponse& copp_info)
{
    HAL_TRACE_ERR("Aborting copp {} restore", copp->key);
    copp_create_abort_cleanup(copp, copp->hal_handle);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// copp's restore cb.
//  - restores copp to the PI and PD state before the upgrade
//------------------------------------------------------------------------------
uint32_t
copp_restore_cb (void *obj, uint32_t len)
{
    hal_ret_t       ret;
    CoppGetResponse copp_info;
    copp_t          *copp;

    // de-serialize the object
    if (copp_info.ParseFromArray(obj, len) == false) {
        HAL_TRACE_ERR("Failed to de-serialize a serialized copp obj");
        SDK_ASSERT(0);
        return 0;
    }

    // allocate COPP obj from slab
    copp = copp_alloc_init();
    if (copp == NULL) {
        HAL_TRACE_ERR("Failed to alloc/init copp, err : {}", ret);
        return 0;
    }

    // initialize copp attrs from its spec
    copp_init_from_spec(copp, copp_info.spec());
    copp_init_from_status(copp, copp_info.status());
    copp_init_from_stats(copp, copp_info.stats());

    // repopulate handle db
    hal_handle_insert(HAL_OBJ_ID_COPP, copp->hal_handle, (void *)copp);

    ret = copp_restore_add(copp, copp_info);
    if (ret != HAL_RET_OK) {
        copp_restore_abort(copp, copp_info);
        copp = NULL;
        goto end;
    }
    copp_restore_commit(copp, copp_info);

end:

    if (ret != HAL_RET_OK) {
        if (copp) {
            // PD wouldn't have been allocated if we're coming here
            // PD gets allocated in restore_add and if it failed,
            // restore_abort would free everything
            copp_free(copp, true);
            copp = NULL;
        }
    }
    return 0;    // TODO: fix me
}
}    // namespace hal
