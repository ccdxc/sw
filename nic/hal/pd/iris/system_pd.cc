#include "nic/include/hal_lock.hpp"
#include "nic/p4/iris/include/table_sizes.h"
#include "sdk/tcam.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/system_pd.hpp"
#include "nic/p4/iris/include/defines.h"
#include "nic/hal/pd/p4pd_api.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/capri/capri_tm_rw.hpp"
#include "nic/hal/pd/iris/p4plus_pd_api.h"

using sys::SystemConfigSpec;
namespace hal {
namespace pd {

thread_local void *g_clock_delta_timer;

#define HAL_TIMER_ID_CLOCK_SYNC         0
#define HAL_TIMER_ID_CLOCK_SYNC_INTVL  (60 * TIME_MSECS_PER_MIN)
#define HW_CLOCK_TICK_TO_NS(x)         (x * 1200480) //based on frequency of 833 Hz
#define NS_TO_HW_CLOCK_TICK(x)         (x / 1200480)

hal_ret_t
pd_system_populate_drop_stats (DropStatsEntry *stats_entry, uint8_t idx)
{
    hal_ret_t               ret = HAL_RET_OK;
    sdk_ret_t               sdk_ret;
    tcam                    *tcam;
    drop_stats_swkey         key = { 0 };
    drop_stats_swkey_mask    key_mask = { 0 };
    drop_stats_actiondata    data = { 0 };

    tcam = g_hal_state_pd->tcam_table(P4TBL_ID_DROP_STATS);
    HAL_ASSERT(tcam != NULL);

    // Retrieve from SW to get the stats idx
    sdk_ret = tcam->retrieve(idx, &key, &key_mask, &data);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to retrieve stats idx for: {}",
                idx);
        goto end;
    }

    // Read from drop stats table
    sdk_ret = tcam->retrieve_from_hw(idx, &key, &key_mask, &data);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to retrieve drop stats for entry: {}",
                idx);
        goto end;
    }

    if (key.entry_inactive_drop_stats) {
        goto end;
    }

    pd_system_decode(&key, &key_mask, &data, stats_entry);

end:

    return ret;
}

hal_ret_t
pd_drop_stats_get (pd_drop_stats_get_args_t *args)
{
    hal_ret_t               ret = HAL_RET_OK;
    pd_system_args_t        *pd_sys_args = args->pd_sys_args;
    SystemResponse          *rsp = pd_sys_args->rsp;
    DropStatsEntry          *stats_entry = NULL;

    HAL_TRACE_DEBUG("Querying drop stats");
    for (int i = 0; i < DROP_STATS_TABLE_SIZE; i++) {
        stats_entry = rsp->mutable_stats()->mutable_drop_stats()->
            add_drop_entries();
        pd_system_populate_drop_stats(stats_entry, i);
    }

    return ret;
}

inline hbm_addr_t
hbm_get_addr_for_stat_index (p4pd_table_id table_id,
                             uint8_t idx)
{
    hbm_addr_t  stats_base_addr;
    p4pd_table_properties_t  tbl_ctx;

    stats_base_addr =  get_start_offset(JP4_ATOMIC_STATS);
    stats_base_addr &= ~((uint64_t)0x80000000);

    switch (table_id) {
    case P4TBL_ID_INGRESS_TX_STATS:
        p4pd_table_properties_get(P4TBL_ID_TX_STATS, &tbl_ctx);
        stats_base_addr += (tbl_ctx.tabledepth << 6);
        // fall through
    case P4TBL_ID_TX_STATS:
        p4pd_table_properties_get(P4TBL_ID_COPP_ACTION, &tbl_ctx);
        stats_base_addr += (tbl_ctx.tabledepth << 5);
        // fall through
    case P4TBL_ID_COPP_ACTION:
        p4pd_table_properties_get(P4TBL_ID_RX_POLICER_ACTION, &tbl_ctx);
        stats_base_addr += (tbl_ctx.tabledepth << 5);
        // fall through
    case P4TBL_ID_RX_POLICER_ACTION:
        p4pd_table_properties_get(P4TBL_ID_FLOW_STATS, &tbl_ctx);
        stats_base_addr += (tbl_ctx.tabledepth << 5);
        // fall through
    case P4TBL_ID_FLOW_STATS:
    default:
        break;
    }

    switch (table_id) {
    case P4TBL_ID_FLOW_STATS:
    case P4TBL_ID_RX_POLICER_ACTION:
    case P4TBL_ID_COPP_ACTION:
        stats_base_addr += (idx << 5);
        break;
    case P4TBL_ID_INGRESS_TX_STATS:
        stats_base_addr += (idx << 3);
        break;
    case P4TBL_ID_TX_STATS:
        stats_base_addr += (idx << 6);
        break;
    default:
        break;
    }

    return stats_base_addr;
}

hal_ret_t
pd_system_drop_stats_set (int id, drop_stats_actiondata *data)
{
    hal_ret_t                ret;
    sdk_ret_t                sdk_ret;
    tcam                     *tcam;

    tcam = g_hal_state_pd->tcam_table(P4TBL_ID_DROP_STATS);
    HAL_ASSERT(tcam != NULL);
    
    data->actionid = DROP_STATS_DROP_STATS_ID;
    sdk_ret = tcam->update(id, data);
    if (sdk_ret != sdk::SDK_RET_OK) {
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        HAL_TRACE_ERR("flow stats table write failure, idx : {}, err : {}",
                id, ret);
        return ret;
    }
    return HAL_RET_OK;
}

hal_ret_t
pd_system_drop_action (pd_system_drop_action_args_t *args)
{
    SystemConfigSpec *spec = args->spec;
    uint8_t sessns = 0;
    drop_stats_actiondata    data = { 0 };

    for (int i = 0; i < spec->span_on_drop_sessions().size(); i++) {
        auto sess = spec->span_on_drop_sessions(i).session_id();
        if (sess > 7) {
            return HAL_RET_INVALID_ARG;
        }
        sessns = 1 << sess;
    }
    if (sessns > 0) {
        data.drop_stats_action_u.drop_stats_drop_stats.mirror_en = 1;
        data.drop_stats_action_u.drop_stats_drop_stats.mirror_session_id = sessns;
    } else {
        data.drop_stats_action_u.drop_stats_drop_stats.mirror_en = 0;
        data.drop_stats_action_u.drop_stats_drop_stats.mirror_session_id = sessns;
    }
    for (int i = DROP_MIN; i <= DROP_MAX; i++) {
        pd_system_drop_stats_set(i, &data);
    }
    return HAL_RET_OK;
}

hal_ret_t
pd_system_decode (drop_stats_swkey *key, drop_stats_swkey_mask *key_mask,
                  drop_stats_actiondata *data, DropStatsEntry *stats_entry)
{
    hal_ret_t   ret = HAL_RET_OK;
    uint64_t drop_reason, drop_reason_mask;

    memcpy(&drop_reason, key->control_metadata_drop_reason,
           sizeof(key->control_metadata_drop_reason));
    memcpy(&drop_reason_mask, key_mask->control_metadata_drop_reason_mask,
           sizeof(key_mask->control_metadata_drop_reason_mask));
    drop_reason &= drop_reason_mask;
    memcpy(key->control_metadata_drop_reason, &drop_reason,
           sizeof(key->control_metadata_drop_reason));

    stats_entry->set_drop_input_mapping(
            drop_reason & (1 << DROP_INPUT_MAPPING));
    stats_entry->set_drop_input_mapping_dejavu(
            drop_reason & (1 << DROP_INPUT_MAPPING_DEJAVU));
    stats_entry->set_drop_flow_hit(
            drop_reason & (1 << DROP_FLOW_HIT));
    stats_entry->set_drop_flow_miss(
            drop_reason & (1 << DROP_FLOW_MISS));
    stats_entry->set_drop_ipsg(
            drop_reason & (1 << DROP_IPSG));
    stats_entry->set_drop_nacl(
            drop_reason & (1 << DROP_NACL));
    stats_entry->set_drop_malformed_pkt(
            drop_reason & (1 << DROP_MALFORMED_PKT));
    stats_entry->set_drop_ip_normalization(
            drop_reason & (1 << DROP_IP_NORMALIZATION));
    stats_entry->set_drop_tcp_normalization(
            drop_reason & (1 << DROP_TCP_NORMALIZATION));
    stats_entry->set_drop_tcp_non_syn_first_pkt(
            drop_reason & (1 << DROP_TCP_NON_SYN_FIRST_PKT));
    stats_entry->set_drop_icmp_normalization(
            drop_reason & (1 << DROP_ICMP_NORMALIZATION));
    stats_entry->set_drop_input_properties_miss(
            drop_reason & (1 << DROP_INPUT_PROPERTIES_MISS));
    stats_entry->set_drop_tcp_out_of_window(
            drop_reason & (1 << DROP_TCP_OUT_OF_WINDOW));
    stats_entry->set_drop_tcp_split_handshake(
            drop_reason & (1 << DROP_TCP_SPLIT_HANDSHAKE));
    stats_entry->set_drop_tcp_win_zero_drop(
            drop_reason & (1 << DROP_TCP_WIN_ZERO_DROP));
    stats_entry->set_drop_tcp_data_after_fin(
            drop_reason & (1 << DROP_TCP_DATA_AFTER_FIN));
    stats_entry->set_drop_tcp_non_rst_pkt_after_rst(
            drop_reason & (1 << DROP_TCP_NON_RST_PKT_AFTER_RST));
    stats_entry->set_drop_tcp_invalid_responder_first_pkt(
            drop_reason & (1 << DROP_TCP_INVALID_RESPONDER_FIRST_PKT));
    stats_entry->set_drop_tcp_unexpected_pkt(
            drop_reason & (1 << DROP_TCP_UNEXPECTED_PKT));
    stats_entry->set_drop_src_lif_mismatch(
            drop_reason & (1 << DROP_SRC_LIF_MISMATCH));
    stats_entry->set_drop_parser_icrc_error(
            drop_reason & (1 << DROP_PARSER_ICRC_ERR));
    stats_entry->set_drop_parse_len_error(
            drop_reason & (1 << DROP_PARSER_LEN_ERR));
    stats_entry->set_drop_hardware_error(
            drop_reason & (1 << DROP_HARDWARE_ERR));

    uint64_t drop_stats_pkts = 0;
    memcpy(&drop_stats_pkts,
           data->drop_stats_action_u.drop_stats_drop_stats.drop_pkts,
           sizeof(data->drop_stats_action_u.drop_stats_drop_stats.drop_pkts));
    stats_entry->set_drop_count(drop_stats_pkts);

    return ret;
}

hal_ret_t
pd_system_populate_index_table_stats (sys::TableStatsEntry *stats_entry,
                                      p4pd_table_id id)
{
    hal_ret_t               ret = HAL_RET_OK;
    directmap               *dm;

    dm = g_hal_state_pd->dm_table(id);
    if (!dm) {
        return ret;
    }

    stats_entry->set_table_type(sys::TABLE_TYPE_INDEX);
    stats_entry->set_table_name(dm->name());
    stats_entry->set_table_size(dm->capacity());
    stats_entry->set_overflow_table_size(0);
    stats_entry->set_entries_in_use(dm->num_entries_in_use());
    stats_entry->set_overflow_entries_in_use(0);
    stats_entry->set_num_inserts(dm->num_inserts());
    stats_entry->set_num_insert_errors(dm->num_insert_errors());
    stats_entry->set_num_deletes(dm->num_deletes());
    stats_entry->set_num_delete_errors(dm->num_delete_errors());

    return ret;
}

hal_ret_t
pd_system_populate_flow_table_stats (sys::TableStatsEntry *stats_entry,
                                     p4pd_table_id id)
{
    hal_ret_t               ret = HAL_RET_OK;
    Flow                    *fl;

    fl = g_hal_state_pd->flow_table();
    if (!fl) {
        return ret;
    }

    stats_entry->set_table_type (sys::TABLE_TYPE_HASH);
    stats_entry->set_table_name(fl->table_name());
    stats_entry->set_table_size(fl->table_capacity());
    stats_entry->set_overflow_table_size(fl->oflow_table_capacity());
    stats_entry->set_entries_in_use(fl->table_num_entries_in_use());
    stats_entry->set_overflow_entries_in_use(fl->oflow_table_num_entries_in_use());
    stats_entry->set_num_inserts(fl->table_num_inserts());
    stats_entry->set_num_insert_errors(fl->table_num_insert_errors());
    stats_entry->set_num_deletes(fl->table_num_deletes());
    stats_entry->set_num_delete_errors(fl->table_num_delete_errors());

    return ret;
}

hal_ret_t
pd_system_populate_acl_tcam_table_stats (sys::TableStatsEntry *stats_entry,
                                         p4pd_table_id id)
{
    hal_ret_t               ret = HAL_RET_OK;
    acl_tcam                *acl_tcam;

    acl_tcam = g_hal_state_pd->acl_table();
    if (!acl_tcam) {
        return ret;
    }

    stats_entry->set_table_type (sys::TABLE_TYPE_TCAM);
    stats_entry->set_table_name(acl_tcam->table_name());
    stats_entry->set_table_size(acl_tcam->table_capacity());
    stats_entry->set_overflow_table_size(0);
    stats_entry->set_entries_in_use(acl_tcam->table_num_entries_in_use());
    stats_entry->set_overflow_entries_in_use(0);
    stats_entry->set_num_inserts(acl_tcam->table_num_inserts());
    stats_entry->set_num_insert_errors(acl_tcam->table_num_insert_errors());
    stats_entry->set_num_deletes(acl_tcam->table_num_deletes());
    stats_entry->set_num_delete_errors(acl_tcam->table_num_delete_errors());

    return ret;
}

hal_ret_t
pd_system_populate_tcam_table_stats (sys::TableStatsEntry *stats_entry,
                                     p4pd_table_id id)
{
    hal_ret_t               ret = HAL_RET_OK;
    tcam                    *tcam;

    tcam = g_hal_state_pd->tcam_table(id);
    if (!tcam) {
        return ret;
    }

    stats_entry->set_table_type (sys::TABLE_TYPE_TCAM);
    stats_entry->set_table_name(tcam->name());
    stats_entry->set_table_size(tcam->capacity());
    stats_entry->set_overflow_table_size(0);
    stats_entry->set_entries_in_use(tcam->num_entries_in_use());
    stats_entry->set_overflow_entries_in_use(0);
    stats_entry->set_num_inserts(tcam->num_inserts());
    stats_entry->set_num_insert_errors(tcam->num_insert_errors());
    stats_entry->set_num_deletes(tcam->num_deletes());
    stats_entry->set_num_delete_errors(tcam->num_delete_errors());

    return ret;
}

hal_ret_t
pd_system_populate_hash_tcam_table_stats (sys::TableStatsEntry *stats_entry,
                                          p4pd_table_id id)
{
    hal_ret_t               ret = HAL_RET_OK;
    sdk_hash                *hash;

    hash = g_hal_state_pd->hash_tcam_table(id);

    if (!hash) {
        return ret;
    }

    stats_entry->set_table_type (sys::TABLE_TYPE_HASH_TCAM);
    stats_entry->set_table_name(hash->name());
    stats_entry->set_table_size(hash->capacity());
    stats_entry->set_overflow_table_size(hash->oflow_capacity());
    stats_entry->set_entries_in_use(hash->num_entries_in_use());
    stats_entry->set_overflow_entries_in_use(hash->oflow_num_entries_in_use());
    stats_entry->set_num_inserts(hash->num_inserts());
    stats_entry->set_num_insert_errors(hash->num_insert_errors());
    stats_entry->set_num_deletes(hash->num_deletes());
    stats_entry->set_num_delete_errors(hash->num_delete_errors());

    return ret;
}

static hal_ret_t
pd_system_populate_table_stats (sys::TableStatsEntry *stats_entry,
                                uint32_t id)
{
    hal_ret_t               ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("PD-System: Populating table stats");

    if (id >= P4TBL_ID_INDEX_MIN && id <= P4TBL_ID_INDEX_MAX) {
        return pd_system_populate_index_table_stats(stats_entry,
                                                    (p4pd_table_id)id);
    } else if (id == P4TBL_ID_NACL) {
        return pd_system_populate_acl_tcam_table_stats(stats_entry,
                                                       (p4pd_table_id)id);
    } else if (id >= P4TBL_ID_TCAM_MIN && id <= P4TBL_ID_TCAM_MAX) {
        return pd_system_populate_tcam_table_stats(stats_entry,
                                                   (p4pd_table_id)id);
    } else if (id >= P4TBL_ID_HASH_OTCAM_MIN && id <= P4TBL_ID_HASH_OTCAM_MAX) {
        return pd_system_populate_hash_tcam_table_stats(stats_entry,
                                                        (p4pd_table_id)id);
    } else if (id >= P4TBL_ID_HASH_MIN && id <= P4TBL_ID_HASH_MAX) {
        return pd_system_populate_flow_table_stats(stats_entry,
                                                   (p4pd_table_id)id);
    } else {
        stats_entry->set_table_type (sys::TABLE_TYPE_NONE);
    }

    return ret;
}

hal_ret_t
pd_table_stats_get(pd_table_stats_get_args_t *args)
{
    hal_ret_t               ret = HAL_RET_OK;
    pd_system_args_t        *pd_sys_args = args->pd_sys_args;
    SystemResponse          *rsp = pd_sys_args->rsp;
    sys::TableStatsEntry    *stats_entry = NULL;
    int                     i;

    HAL_TRACE_DEBUG("PD-System: Querying table stats");

    for (i = (int)P4TBL_ID_TBLMIN; i <= (int)P4TBL_ID_TBLMAX; i++) {
        if (i >= (int)P4TBL_ID_MPU_MIN && i <= (int)P4TBL_ID_MPU_MAX) {
            continue;
        }
        stats_entry = rsp->mutable_stats()->mutable_table_stats()->
            add_table_stats();
     	pd_system_populate_table_stats(stats_entry, (p4pd_table_id)i);
    }

    return ret;
}

//------------------------------------------------------------------------------
// convert hardware timestamp to software timestamp
//------------------------------------------------------------------------------
hal_ret_t
pd_conv_hw_clock_to_sw_clock (pd_conv_hw_clock_to_sw_clock_args_t *args)
{
    if (g_hal_state_pd->clock_delta_op() == HAL_CLOCK_DELTA_OP_ADD) {
        HAL_TRACE_DEBUG("hw tick: {} sw_ns: {}", HW_CLOCK_TICK_TO_NS(args->hw_tick), 
                         (HW_CLOCK_TICK_TO_NS(args->hw_tick) + g_hal_state_pd->clock_delta()));
        *args->sw_ns = HW_CLOCK_TICK_TO_NS(args->hw_tick) + g_hal_state_pd->clock_delta();
    } else {
        *args->sw_ns = HW_CLOCK_TICK_TO_NS(args->hw_tick) - g_hal_state_pd->clock_delta();
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// convert hardware timestamp to software timestamp
//------------------------------------------------------------------------------
hal_ret_t
pd_conv_sw_clock_to_hw_clock (pd_conv_sw_clock_to_hw_clock_args_t *args)
{
    if (g_hal_state_pd->clock_delta_op() == HAL_CLOCK_DELTA_OP_ADD) {
        *args->hw_tick = NS_TO_HW_CLOCK_TICK((args->sw_ns - g_hal_state_pd->clock_delta())); 
    } else {
        *args->hw_tick = NS_TO_HW_CLOCK_TICK((args->sw_ns + g_hal_state_pd->clock_delta()));
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// compute delta between sw and hw clock
//----------------------------------------------------------------------
static void
clock_delta_comp_cb (void *timer, uint32_t timer_id, void *ctxt)
{
    uint64_t              hw_ns = 0, sw_ns = 0, delta_ns = 0;
    timespec_t            sw_ts;

    // Read hw time
    capri_tm_get_clock_tick(&hw_ns);
    HW_CLOCK_TICK_TO_NS(hw_ns);

    // get current time
    clock_gettime(CLOCK_MONOTONIC, &sw_ts);
    sdk::timestamp_to_nsecs(&sw_ts, &sw_ns);

    if (sw_ns == hw_ns) {
        // Do nothing. We are in sync in hw!
        return;
    } else if (sw_ns < hw_ns) {
        delta_ns = hw_ns - sw_ns;
        g_hal_state_pd->set_clock_delta_op(HAL_CLOCK_DELTA_OP_SUBTRACT);
    } else {
        // hw_ns < sw_ns
        delta_ns = sw_ns - hw_ns;
        g_hal_state_pd->set_clock_delta_op(HAL_CLOCK_DELTA_OP_ADD);
    }

    HAL_TRACE_DEBUG("Delta ns: {}", delta_ns);
    HAL_TRACE_DEBUG("Clock delta op: {}", g_hal_state_pd->clock_delta_op());
    g_hal_state_pd->set_clock_delta(delta_ns);
}

//------------------------------------------------------------------------------
// start a periodic timer for Hw and sw clock delta computation
//------------------------------------------------------------------------------
hal_ret_t
pd_clock_delta_comp (pd_clock_delta_comp_args_t *args)
{
    // wait until the periodic thread is ready
    while (!hal::periodic::periodic_thread_is_running()) {
        pthread_yield();
    }
 
    clock_delta_comp_cb(NULL, HAL_TIMER_ID_CLOCK_SYNC, NULL);
    g_clock_delta_timer =
        hal::periodic::timer_schedule(HAL_TIMER_ID_CLOCK_SYNC,            // timer_id
                                      HAL_TIMER_ID_CLOCK_SYNC_INTVL,
                                      (void *)0,    // ctxt
                                      clock_delta_comp_cb, true);
    if (!g_clock_delta_timer) {
        return HAL_RET_ERR;
    }
    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal
