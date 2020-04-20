#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/iris/datapath/p4/include/table_sizes.h"
#include "lib/table/tcam/tcam.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/internal/system_pd.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/asic/cmn/asic_hbm.hpp"
#include "platform/capri/capri_tm_rw.hpp"
#include "platform/capri/capri_tm_utils.hpp"
#include "nic/hal/pd/iris/internal/p4plus_pd_api.h"
#include "nic/hal/pd/iris/aclqos/qos_pd.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/sdk/lib/table/sldirectmap/sldirectmap.hpp"
#include "gen/p4gen/p4/include/p4pd.h"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/sdk/include/sdk/qos.hpp"

using namespace sdk;
using namespace sdk::asic::pd;

namespace hal {
namespace pd {

static double g_clock_adjustment = 0;
void *g_clock_delta_timer = NULL;
void *g_clock_rollover_timer = NULL;
static clock_gettimeofday_t *g_hbm_clockaddr = NULL;
static uint64_t g_clock_freq = 0;
static struct clock_table_info_s {
    uint8_t cache;
    uint16_t entry_width;
    sdk::types::mem_addr_t start_addr;
    uint64_t multiplier;
} g_clock_table_info;

#define HAL_TIMER_ID_CLOCK_SYNC_INTVL     (60 * TIME_MSECS_PER_MIN)
#define HAL_TIMER_ID_CLOCK_SYNC_INTVL_NS  (HAL_TIMER_ID_CLOCK_SYNC_INTVL * TIME_NSECS_PER_MSEC)
#define CLOCK_FREQ                     (g_clock_freq * 1000000) // Freq in MHz
#define HW_CLOCK_TICK_TO_NS(x)         (x * g_clock_adjustment)
#define NS_TO_HW_CLOCK_TICK(x)         (x / g_clock_adjustment)
#define CLOCK_WIDTH                    8
#define CAPRI_CLOCK_FREQ                     833

static hal_ret_t
pd_system_drop_stats_set (int id, drop_stats_actiondata_t *data)
{
    hal_ret_t                ret;
    sdk_ret_t                sdk_ret;
    tcam                     *tcam;

    tcam = g_hal_state_pd->tcam_table(P4TBL_ID_DROP_STATS);
    SDK_ASSERT(tcam != NULL);

    data->action_id = DROP_STATS_DROP_STATS_ID;
    sdk_ret = tcam->update(id, data);
    if (sdk_ret != sdk::SDK_RET_OK) {
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        HAL_TRACE_ERR("drop stats table write failure, idx : {}, err : {}",
                      id, ret);
        return ret;
    }
    return HAL_RET_OK;
}

hal_ret_t
pd_system_clear_drop_stats (uint8_t idx)
{
    hal_ret_t               ret = HAL_RET_OK;
    sdk_ret_t               sdk_ret;
    tcam                    *tcam;
    drop_stats_swkey         key = { 0 };
    drop_stats_swkey_mask    key_mask = { 0 };
    drop_stats_actiondata_t    data = { 0 };

    tcam = g_hal_state_pd->tcam_table(P4TBL_ID_DROP_STATS);
    SDK_ASSERT(tcam != NULL);

    // Read from drop stats table
    sdk_ret = tcam->retrieve_from_hw(idx, &key, &key_mask, &data);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to retrieve drop stats at idx : {}, err : {}",
                      idx, ret);
        goto end;
    }

    // Reset drop_pkts field in data
    if (!key.entry_inactive_drop_stats) {
        memset(data.action_u.drop_stats_drop_stats.drop_pkts,
               0, sizeof(data.action_u.drop_stats_drop_stats.drop_pkts));

        ret = pd_system_drop_stats_set(idx, &data);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Unable to reset drop stats at idx : {}, err : {}",
                          idx, ret);
            goto end;
        }
    }

end:
    return ret;
}

hal_ret_t
pd_system_populate_drop_stats (DropStatsEntry *stats_entry, uint8_t idx)
{
    hal_ret_t               ret = HAL_RET_OK;
    sdk_ret_t               sdk_ret;
    tcam                    *tcam;
    drop_stats_swkey         key = { 0 };
    drop_stats_swkey_mask    key_mask = { 0 };
    drop_stats_actiondata_t    data = { 0 };

    tcam = g_hal_state_pd->tcam_table(P4TBL_ID_DROP_STATS);
    SDK_ASSERT(tcam != NULL);

    // Retrieve from SW to get the stats idx
    sdk_ret = tcam->retrieve(idx, &key, &key_mask, &data);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to retrieve stats at idx : {}, err : {}",
                      idx, ret);
        goto end;
    }

    // Read from drop stats table
    sdk_ret = tcam->retrieve_from_hw(idx, &key, &key_mask, &data);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to retrieve drop stats at idx : {}, err : {}",
                      idx, ret);
        goto end;
    }

    if (key.entry_inactive_drop_stats) {
        HAL_TRACE_DEBUG("Drop stats entry is inactive for idx : {}", idx);
        goto end;
    }

    pd_system_decode(&key, &key_mask, &data, stats_entry);

end:

    return ret;
}

hal_ret_t
pd_drop_stats_clear (pd_func_args_t *pd_func_args)
{
    HAL_TRACE_DEBUG("Reset drop stats");
    for (int i = 0; i < DROP_STATS_TABLE_SIZE; i++) {
        pd_system_clear_drop_stats(i);
    }

    return HAL_RET_OK;
}

hal_ret_t
pd_drop_stats_get (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret = HAL_RET_OK;
    pd_drop_stats_get_args_t *args = pd_func_args->pd_drop_stats_get;
    pd_system_args_t        *pd_sys_args = args->pd_sys_args;
    SystemResponse          *rsp = pd_sys_args->rsp;
    DropStatsEntry          *stats_entry = NULL;

    HAL_TRACE_VERBOSE("Querying drop stats");
    for (int i = 0; i < (DROP_MAX + 1); i++) {
        stats_entry = rsp->mutable_stats()->mutable_drop_stats()->
            add_drop_entries();
        pd_system_populate_drop_stats(stats_entry, i);
    }

    return ret;
}

hal_ret_t
pd_system_egress_drop_decode (egress_drop_stats_swkey *key, egress_drop_stats_swkey_mask *key_mask,
                              egress_drop_stats_actiondata_t *data, EgressDropStatsEntry *stats_entry)
{
    hal_ret_t   ret = HAL_RET_OK;
    uint64_t drop_reason, drop_reason_mask;

    memcpy(&drop_reason, key->control_metadata_egress_drop_reason,
           sizeof(key->control_metadata_egress_drop_reason));
    memcpy(&drop_reason_mask, key_mask->control_metadata_egress_drop_reason_mask,
           sizeof(key_mask->control_metadata_egress_drop_reason_mask));
    drop_reason &= drop_reason_mask;
    memcpy(key->control_metadata_egress_drop_reason, &drop_reason,
           sizeof(key->control_metadata_egress_drop_reason));

    stats_entry->mutable_reasons()->set_drop_output_mapping(
            drop_reason & (1 << EGRESS_DROP_OUTPUT_MAPPING));
    stats_entry->mutable_reasons()->set_drop_prune_src_port(
            drop_reason & (1 << EGRESS_DROP_PRUNE_SRC_PORT));
    stats_entry->mutable_reasons()->set_drop_mirror(
            drop_reason & (1 << EGRESS_DROP_MIRROR));
    stats_entry->mutable_reasons()->set_drop_policer(
            drop_reason & (1 << EGRESS_DROP_POLICER));
    stats_entry->mutable_reasons()->set_drop_copp(
            drop_reason & (1 << EGRESS_DROP_COPP));
    stats_entry->mutable_reasons()->set_drop_checksum_err(
            drop_reason & (1 << EGRESS_DROP_CHECKSUM_ERR));

    uint64_t drop_stats_pkts = 0;
    memcpy(&drop_stats_pkts,
           data->action_u.egress_drop_stats_egress_drop_stats.drop_pkts,
           sizeof(data->action_u.egress_drop_stats_egress_drop_stats.drop_pkts));
    stats_entry->set_drop_count(drop_stats_pkts);

    return ret;
}

hal_ret_t
pd_system_populate_egress_drop_stats (EgressDropStatsEntry *stats_entry, uint8_t idx)
{
    hal_ret_t                       ret = HAL_RET_OK;
    sdk_ret_t                       sdk_ret;
    tcam                            *tcam;
    egress_drop_stats_swkey         key = { 0 };
    egress_drop_stats_swkey_mask    key_mask = { 0 };
    egress_drop_stats_actiondata_t    data = { 0 };

    tcam = g_hal_state_pd->tcam_table(P4TBL_ID_EGRESS_DROP_STATS);
    SDK_ASSERT(tcam != NULL);

    // Retrieve from SW to get the stats idx
    sdk_ret = tcam->retrieve(idx, &key, &key_mask, &data);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to retrieve egress drop stats at idx : {}, "
                      "err : {}", idx, ret);
        goto end;
    }

    // Read from drop stats table
    sdk_ret = tcam->retrieve_from_hw(idx, &key, &key_mask, &data);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to retrieve egress drop stats at idx : {}, "
                      "err : {}", idx, ret);
        goto end;
    }

    pd_system_egress_drop_decode(&key, &key_mask, &data, stats_entry);

end:

    return ret;
}

hal_ret_t
pd_egress_drop_stats_get (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret = HAL_RET_OK;
    pd_egress_drop_stats_get_args_t *args = pd_func_args->pd_egress_drop_stats_get;
    pd_system_args_t        *pd_sys_args = args->pd_sys_args;
    SystemResponse          *rsp = pd_sys_args->rsp;
    EgressDropStatsEntry    *stats_entry = NULL;

    HAL_TRACE_VERBOSE("Querying egress drop stats");
    for (int i = 0; i < (EGRESS_DROP_MAX + 1); i++) {
        stats_entry = rsp->mutable_stats()->mutable_egress_drop_stats()->
            add_drop_entries();
        pd_system_populate_egress_drop_stats(stats_entry, i);
    }

    return ret;
}

static hal_ret_t
pd_system_egress_drop_stats_set (int id, egress_drop_stats_actiondata_t *data)
{
    hal_ret_t                ret;
    sdk_ret_t                sdk_ret;
    tcam                     *tcam;

    tcam = g_hal_state_pd->tcam_table(P4TBL_ID_EGRESS_DROP_STATS);
    SDK_ASSERT(tcam != NULL);

    data->action_id = EGRESS_DROP_STATS_EGRESS_DROP_STATS_ID;
    sdk_ret = tcam->update(id, data);
    if (sdk_ret != sdk::SDK_RET_OK) {
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        HAL_TRACE_ERR("egress drop stats table write failure, idx : {}, err : {}",
                      id, ret);
        return ret;
    }
    return HAL_RET_OK;
}

hal_ret_t
pd_system_clear_egress_drop_stats (uint8_t idx)
{
    hal_ret_t                       ret = HAL_RET_OK;
    sdk_ret_t                       sdk_ret;
    tcam                            *tcam;
    egress_drop_stats_swkey         key = { 0 };
    egress_drop_stats_swkey_mask    key_mask = { 0 };
    egress_drop_stats_actiondata_t    data = { 0 };

    tcam = g_hal_state_pd->tcam_table(P4TBL_ID_EGRESS_DROP_STATS);
    SDK_ASSERT(tcam != NULL);

    // Read from drop stats table
    sdk_ret = tcam->retrieve_from_hw(idx, &key, &key_mask, &data);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to retrieve drop stats at idx : {}, err : {}",
                      idx, ret);
        goto end;
    }

    // Reset drop_pkts field in data
    memset(data.action_u.egress_drop_stats_egress_drop_stats.drop_pkts,
           0, sizeof(data.action_u.egress_drop_stats_egress_drop_stats.drop_pkts));

    ret = pd_system_egress_drop_stats_set(idx, &data);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to reset egress drop stats at idx : {}, err : {}",
                      idx, ret);
        goto end;
    }

end:
    return ret;
}

hal_ret_t
pd_egress_drop_stats_clear (pd_func_args_t *pd_func_args)
{
    HAL_TRACE_DEBUG("Clearing egress drop stats");
    for (int i = 0; i < (EGRESS_DROP_MAX + 1); i++) {
        pd_system_clear_egress_drop_stats(i);
    }

    return HAL_RET_OK;
}

inline mem_addr_t
hbm_get_addr_for_stat_index (p4pd_table_id table_id, uint8_t idx)
{
    mem_addr_t  stats_base_addr;
    p4pd_table_properties_t  tbl_ctx;

    stats_base_addr = asicpd_get_mem_addr(ASIC_HBM_REG_P4_ATOMIC_STATS);
    SDK_ASSERT(stats_base_addr != INVALID_MEM_ADDRESS);
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
pd_system_decode (drop_stats_swkey *key, drop_stats_swkey_mask *key_mask,
                  drop_stats_actiondata_t *data, DropStatsEntry *stats_entry)
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

    stats_entry->mutable_reasons()->set_drop_input_mapping(
            drop_reason & (1 << DROP_INPUT_MAPPING));
    stats_entry->mutable_reasons()->set_drop_input_mapping_dejavu(
            drop_reason & (1 << DROP_INPUT_MAPPING_DEJAVU));
    stats_entry->mutable_reasons()->set_drop_multi_dest_not_pinned_uplink(
            drop_reason & (1 << DROP_MULTI_DEST_NOT_PINNED_UPLINK));
    stats_entry->mutable_reasons()->set_drop_flow_hit(
            drop_reason & (1 << DROP_FLOW_HIT));
    stats_entry->mutable_reasons()->set_drop_flow_miss(
            drop_reason & (1 << DROP_FLOW_MISS));
    stats_entry->mutable_reasons()->set_drop_ipsg(
            drop_reason & (1 << DROP_IPSG));
    stats_entry->mutable_reasons()->set_drop_nacl(
            drop_reason & (1 << DROP_NACL));
    stats_entry->mutable_reasons()->set_drop_malformed_pkt(
            drop_reason & (1 << DROP_MALFORMED_PKT));
    stats_entry->mutable_reasons()->set_drop_ip_normalization(
            drop_reason & (1 << DROP_IP_NORMALIZATION));
    stats_entry->mutable_reasons()->set_drop_tcp_normalization(
            drop_reason & (1 << DROP_TCP_NORMALIZATION));
    stats_entry->mutable_reasons()->set_drop_tcp_non_syn_first_pkt(
            drop_reason & (1 << DROP_TCP_NON_SYN_FIRST_PKT));
    stats_entry->mutable_reasons()->set_drop_icmp_normalization(
            drop_reason & (1 << DROP_ICMP_NORMALIZATION));
    stats_entry->mutable_reasons()->set_drop_input_properties_miss(
            drop_reason & (1 << DROP_INPUT_PROPERTIES_MISS));
    stats_entry->mutable_reasons()->set_drop_tcp_out_of_window(
            drop_reason & (1 << DROP_TCP_OUT_OF_WINDOW));
    stats_entry->mutable_reasons()->set_drop_tcp_split_handshake(
            drop_reason & (1 << DROP_TCP_SPLIT_HANDSHAKE));
    stats_entry->mutable_reasons()->set_drop_tcp_win_zero_drop(
            drop_reason & (1 << DROP_TCP_WIN_ZERO_DROP));
    stats_entry->mutable_reasons()->set_drop_tcp_data_after_fin(
            drop_reason & (1 << DROP_TCP_DATA_AFTER_FIN));
    stats_entry->mutable_reasons()->set_drop_tcp_non_rst_pkt_after_rst(
            drop_reason & (1 << DROP_TCP_NON_RST_PKT_AFTER_RST));
    stats_entry->mutable_reasons()->set_drop_tcp_invalid_responder_first_pkt(
            drop_reason & (1 << DROP_TCP_INVALID_RESPONDER_FIRST_PKT));
    stats_entry->mutable_reasons()->set_drop_tcp_unexpected_pkt(
            drop_reason & (1 << DROP_TCP_UNEXPECTED_PKT));
    stats_entry->mutable_reasons()->set_drop_src_lif_mismatch(
            drop_reason & (1 << DROP_SRC_LIF_MISMATCH));
    stats_entry->mutable_reasons()->set_drop_parser_icrc_error(
            drop_reason & (1 << DROP_PARSER_ICRC_ERR));
    stats_entry->mutable_reasons()->set_drop_parse_len_error(
            drop_reason & (1 << DROP_PARSER_LEN_ERR));
    stats_entry->mutable_reasons()->set_drop_hardware_error(
            drop_reason & (1 << DROP_HARDWARE_ERR));
    stats_entry->mutable_reasons()->set_drop_vf_ip_label_mismatch(
            drop_reason & (1 << DROP_VF_IP_LABEL_MISMATCH));
    stats_entry->mutable_reasons()->set_drop_vf_bad_rr_dst_ip(
            drop_reason & (1 << DROP_VF_BAD_RR_DST_IP));
    stats_entry->mutable_reasons()->set_drop_icmp_frag_pkt(
            drop_reason & (1 << DROP_ICMP_FRAGMENT_PKT));

    uint64_t drop_stats_pkts = 0;
    memcpy(&drop_stats_pkts,
           data->action_u.drop_stats_drop_stats.drop_pkts,
           sizeof(data->action_u.drop_stats_drop_stats.drop_pkts));
    stats_entry->set_drop_count(drop_stats_pkts);

    return ret;
}

hal_ret_t
pd_system_populate_index_table_stats (sys::TableStatsEntry *stats_entry,
                                      p4pd_table_id id)
{
    hal_ret_t               ret = HAL_RET_OK;

    if (id == P4TBL_ID_SESSION_STATE || id == P4TBL_ID_FLOW_INFO ||
        id == P4TBL_ID_FLOW_STATS) {
        sldirectmap               *dm;

        dm = (sldirectmap *)g_hal_state_pd->dm_table(id);
        if (!dm) {
           return ret;
        }

        stats_entry->set_table_type(sys::TABLE_TYPE_INDEX);
        stats_entry->set_table_name(dm->name());
        stats_entry->set_table_size(dm->capacity());
        stats_entry->set_overflow_table_size(0);
        stats_entry->set_entries_in_use(dm->inuse());
        stats_entry->set_overflow_entries_in_use(0);
    } else {
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
    }

    return ret;
}

hal_ret_t
pd_system_populate_flow_table_stats (sys::TableStatsEntry *stats_entry,
                                     p4pd_table_id id)
{
    stats_entry->set_table_type(sys::TABLE_TYPE_HASH);
    return g_hal_state_pd->flow_table_pd_get()->stats_get(stats_entry);
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

    //HAL_TRACE_DEBUG("PD-System: Populating table stats");

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
pd_table_stats_get(pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret = HAL_RET_OK;
    pd_table_stats_get_args_t *args = pd_func_args->pd_table_stats_get;
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
pd_conv_hw_clock_to_sw_clock (pd_func_args_t *pd_func_args)
{
    pd_conv_hw_clock_to_sw_clock_args_t *args = pd_func_args->pd_conv_hw_clock_to_sw_clock;

    if (g_hal_state_pd->clock_delta_op() == HAL_CLOCK_DELTA_OP_ADD) {
        //HAL_TRACE_DEBUG("hw tick: {} sw_ns: {}", HW_CLOCK_TICK_TO_NS(args->hw_tick),
        //                 (HW_CLOCK_TICK_TO_NS(args->hw_tick) + g_hal_state_pd->clock_delta()));
        *args->sw_ns = HW_CLOCK_TICK_TO_NS(args->hw_tick)  + g_hal_state_pd->clock_delta();
    } else {
        //HAL_TRACE_DEBUG("hw tick: {} sw_ns: {}", HW_CLOCK_TICK_TO_NS(args->hw_tick),
        //                 (HW_CLOCK_TICK_TO_NS(args->hw_tick) - g_hal_state_pd->clock_delta()));
        *args->sw_ns = HW_CLOCK_TICK_TO_NS(args->hw_tick) - g_hal_state_pd->clock_delta();
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// convert hardware timestamp to software timestamp
//------------------------------------------------------------------------------
hal_ret_t
pd_conv_sw_clock_to_hw_clock (pd_func_args_t *pd_func_args)
{
    pd_conv_sw_clock_to_hw_clock_args_t *args = pd_func_args->pd_conv_sw_clock_to_hw_clock;

    if (g_hal_state_pd->clock_delta_op() == HAL_CLOCK_DELTA_OP_ADD) {
        //HAL_TRACE_DEBUG("sw ns: {} hw tick: {}", args->sw_ns,
        //                 NS_TO_HW_CLOCK_TICK((args->sw_ns - g_hal_state_pd->clock_delta())));
        *args->hw_tick = NS_TO_HW_CLOCK_TICK((args->sw_ns - g_hal_state_pd->clock_delta()));
    } else {
        //HAL_TRACE_DEBUG("sw ns: {} hw tick: {}", args->sw_ns,
        //                NS_TO_HW_CLOCK_TICK((args->sw_ns + g_hal_state_pd->clock_delta())));
        *args->hw_tick = NS_TO_HW_CLOCK_TICK((args->sw_ns + g_hal_state_pd->clock_delta()));
    }

    return HAL_RET_OK;
}

//--------------------------------------------------------------------------------------
// Get the clock details: hw clock, delta maintained and sw clock at the given instant
//---------------------------------------------------------------------------------------
hal_ret_t
pd_clock_detail_get (pd_func_args_t *pd_func_args)
{
    hal_ret_t                           ret = HAL_RET_OK;
    uint64_t                            hw_ns = 0;
    pd_clock_detail_get_args_t         *args = pd_func_args->pd_clock_detail_get;
    pd_conv_hw_clock_to_sw_clock_args_t conv_args = {0};
    pd_func_args_t                      conv_func_args;

    // Read hw time
    asicpd_tm_get_clock_tick(&hw_ns);
    HAL_TRACE_DEBUG("Hardware tick:{}", hw_ns);
    args->hw_clock = HW_CLOCK_TICK_TO_NS(hw_ns);
    args->sw_delta = g_hal_state_pd->clock_delta();
    args->clock_op = g_hal_state_pd->clock_delta_op();

    conv_args.hw_tick = args->hw_clock;
    conv_args.sw_ns = &args->sw_clock;
    conv_func_args.pd_conv_hw_clock_to_sw_clock = &conv_args;
    ret = pd_conv_hw_clock_to_sw_clock(&conv_func_args);

    HAL_TRACE_DEBUG("Hardware ns: {} sw ns: {}", args->hw_clock, args->sw_clock);
    return ret;
}

//------------------------------------------------------------------------------
// compute delta between sw and hw clock
//----------------------------------------------------------------------
static void
clock_delta_comp_cb (void *timer, uint32_t timer_id, void *ctxt)
{
    uint64_t              hw_tick = 0, hw_ns = 0, sw_ns = 0, delta_ns = 0;
    timespec_t            sw_ts;

    // Read hw time
    asicpd_tm_get_clock_tick(&hw_tick);
    hw_ns = HW_CLOCK_TICK_TO_NS(hw_tick);

    // get current time
    clock_gettime(CLOCK_MONOTONIC, &sw_ts);
    sdk::timestamp_to_nsecs(&sw_ts, &sw_ns);

    //Compute the delta if rollover happens before the next timer update
    if (timer_id != HAL_TIMER_ID_CLOCK_ROLLOVER) {
        uint64_t rollover_window = (HW_CLOCK_TICK_TO_NS(0xFFFFFFFFFFFF)-\
                                    (HAL_TIMER_ID_CLOCK_SYNC_INTVL_NS));
        if (hw_ns >= rollover_window) {
            g_clock_rollover_timer =
                        sdk::lib::timer_schedule(HAL_TIMER_ID_CLOCK_ROLLOVER, // timer_id
                                      (HW_CLOCK_TICK_TO_NS(0xFFFFFFFFFFFF)-hw_ns), //time to rollover
                                      (void *)0,    // ctxt
                                      clock_delta_comp_cb, false);
        }
    }

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

    HAL_TRACE_DEBUG("hw tick: {} Hw ns: {} sw ns: {} Delta ns: {}", hw_tick, hw_ns, sw_ns, delta_ns);
    HAL_TRACE_DEBUG("Clock delta op: {}", g_hal_state_pd->clock_delta_op());
    g_hal_state_pd->set_clock_delta(delta_ns);

    clock_gettime(CLOCK_REALTIME, &sw_ts);
    sdk::timestamp_to_nsecs(&sw_ts, &sw_ns);

    capri_hbm_table_entry_cache_invalidate((p4pd_table_cache_t)g_clock_table_info.cache, 0,
                                 g_clock_table_info.entry_width, g_clock_table_info.start_addr);
    bzero(g_hbm_clockaddr, sizeof(clock_gettimeofday_t));
    sdk::lib::memrev(g_hbm_clockaddr->time_in_ns, (uint8_t *)&sw_ns, CLOCK_WIDTH);
    sdk::lib::memrev(g_hbm_clockaddr->ticks, (uint8_t *)&hw_tick, CLOCK_WIDTH);
    sdk::lib::memrev(g_hbm_clockaddr->multiplier, (uint8_t *)&g_clock_table_info.multiplier, CLOCK_WIDTH);
}

//----------------------------------------------
// Read a new multiplier and reprogram HBM
//----------------------------------------------
hal_ret_t
pd_set_clock_multiplier (pd_func_args_t *pd_func_args)
{
    pd_set_clock_multiplier_args_t *args =
                                       pd_func_args->pd_set_clock_multiplier;

    // Read the multiplier again in case there is a change in frequency
    g_clock_freq = args->frequency;
    g_clock_table_info.multiplier =
                   g_hal_state->catalog()->clock_get_multiplier(g_clock_freq);
    pd_clock_trigger_sync(pd_func_args);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// start a periodic timer for Hw and sw clock delta computation
//------------------------------------------------------------------------------
hal_ret_t
pd_clock_delta_comp (pd_func_args_t *pd_func_args)
{
    p4pd_table_properties_t tinfo;
    sdk::types::mem_addr_t start_addr, vaddr;

    // This is to sync P4 clock to
    // linux clock. We dont need this in SIM
    if (!is_platform_type_hw())
        return HAL_RET_OK;

    // pd_clock_delta_comp_args_t *args = pd_func_args->pd_clock_delta_comp;
    // wait until the periodic thread is ready
    while (!sdk::lib::periodic_thread_is_running()) {
        pthread_yield();
    }

    // Get the clock frequence
    g_clock_freq = asicpd_clock_freq_get();

    // Get the base memory address of P4 clock table
    start_addr = asicpd_get_mem_addr(ASIC_HBM_REG_CLOCK_GETTIMEOFDAY);
    SDK_ASSERT(start_addr != INVALID_MEM_ADDRESS);

    // Fill in the table details for cache invalidate
    p4pd_table_properties_get(P4TBL_ID_CLOCK, &tinfo);
    g_clock_table_info.cache = tinfo.cache;
    g_clock_table_info.entry_width = tinfo.hbm_layout.entry_width;
    g_clock_table_info.start_addr = tinfo.base_mem_pa;

    // Get the virtual address mapping
    sdk::lib::pal_ret_t ret = sdk::lib::pal_physical_addr_to_virtual_addr(start_addr, &vaddr);
    SDK_ASSERT(ret == sdk::lib::PAL_RET_OK);
    g_hbm_clockaddr = (clock_gettimeofday_t *)vaddr;

    //Compute the clock adjustment based on the frequency to 1 decimal point
    g_clock_adjustment = floor(10.0 * ((double)(((double)1)/CLOCK_FREQ))*TIME_NSECS_PER_SEC)/10.0;
    g_clock_table_info.multiplier = g_hal_state->catalog()->clock_get_multiplier(g_clock_freq);
    HAL_TRACE_DEBUG("g_clock_freq: {} Multiplier: {}", g_clock_freq, g_clock_table_info.multiplier);
    printf("Freq: %lu Adjustment: %lf\n", CLOCK_FREQ, g_clock_adjustment);

    clock_delta_comp_cb(NULL, HAL_TIMER_ID_CLOCK_SYNC, NULL);
    g_clock_delta_timer =
        sdk::lib::timer_schedule(HAL_TIMER_ID_CLOCK_SYNC,            // timer_id
                                 HAL_TIMER_ID_CLOCK_SYNC_INTVL,
                                 (void *)0,    // ctxt
                                 clock_delta_comp_cb, true);
    if (!g_clock_delta_timer) {
        return HAL_RET_ERR;
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Trigger clock frequency computation from CLI
//------------------------------------------------------------------------------
hal_ret_t
pd_clock_trigger_sync (pd_func_args_t *pd_func_args)
{
    // We could be receiving the GRPC configs before
    // we start the sync timer (from FTE thread init).
    // We dont want to sync unless the timer is started
    if (g_clock_delta_timer != NULL) {
        clock_delta_comp_cb(NULL, HAL_TIMER_ID_CLOCK_SYNC, NULL);
    }

    return HAL_RET_OK;
}

hal_ret_t
pd_pb_stats_get (pd_func_args_t *pd_func_args)
{
    hal_ret_t              first_err_ret = HAL_RET_OK;
    hal_ret_t              ret = HAL_RET_OK;
    sdk_ret_t              sdk_ret;
    pd_pb_stats_get_args_t *args = pd_func_args->pd_pb_stats_get;
    pd_system_args_t       *pd_sys_args = args->pd_sys_args;
    SystemResponse         *rsp = pd_sys_args->rsp;
    sys::PacketBufferStats *pb_stats;
    tm_debug_stats_t       debug_stats = {};
    unsigned               port;
    // TODO: Hook up the reset flag to API call
    bool                   reset = false;
    capri_queue_stats_t    qos_queue_stats = {0};

    pb_stats = rsp->mutable_stats()->mutable_packet_buffer_stats();

    for (port = 0; port < TM_NUM_PORTS; port++) {
        sdk_ret = asicpd_tm_debug_stats_get(port, &debug_stats, reset);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get pb debug stats for port {} ret {}",
                          port, ret);
            // Continue
            if (first_err_ret == HAL_RET_OK) {
                first_err_ret = ret;
            }
        }

        auto port_stats = pb_stats->add_port_stats();
        qos_class_pd_port_to_packet_buffer_port(port, port_stats->mutable_packet_buffer_port());
        port_stats->mutable_buffer_stats()->set_sop_count_in(debug_stats.buffer_stats.sop_count_in);
        port_stats->mutable_buffer_stats()->set_eop_count_in(debug_stats.buffer_stats.eop_count_in);
        port_stats->mutable_buffer_stats()->set_sop_count_out(debug_stats.buffer_stats.sop_count_out);
        port_stats->mutable_buffer_stats()->set_eop_count_out(debug_stats.buffer_stats.eop_count_out);

        auto drop_stats = port_stats->mutable_buffer_stats()->mutable_drop_counts();
        for (int i = sdk::qos::BUFFER_INTRINSIC_DROP; i < sdk::qos::BUFFER_DROP_MAX; i ++) {
            auto drop_stats_entry = drop_stats->add_stats_entries();
            drop_stats_entry->set_reasons(sys::BufferDropReasons(i));
            drop_stats_entry->set_drop_count(debug_stats.buffer_stats.drop_counts[i]);
        }

        port_stats->mutable_oflow_fifo_stats()->set_sop_count_in(debug_stats.oflow_fifo_stats.sop_count_in);
        port_stats->mutable_oflow_fifo_stats()->set_eop_count_in(debug_stats.oflow_fifo_stats.eop_count_in);
        port_stats->mutable_oflow_fifo_stats()->set_sop_count_out(debug_stats.oflow_fifo_stats.sop_count_out);
        port_stats->mutable_oflow_fifo_stats()->set_eop_count_out(debug_stats.oflow_fifo_stats.eop_count_out);

        port_stats->mutable_oflow_fifo_stats()->mutable_drop_counts()->add_entry()->set_type(sys::OflowFifoDropType::OCCUPANCY_DROP);
        port_stats->mutable_oflow_fifo_stats()->mutable_drop_counts()->mutable_entry(0)->set_count(debug_stats.oflow_fifo_stats.drop_counts.occupancy_drop_count);
        port_stats->mutable_oflow_fifo_stats()->mutable_drop_counts()->add_entry()->set_type(sys::OflowFifoDropType::EMERGENCY_STOP_DROP);
        port_stats->mutable_oflow_fifo_stats()->mutable_drop_counts()->mutable_entry(1)->set_count(debug_stats.oflow_fifo_stats.drop_counts.emergency_stop_drop_count);
        port_stats->mutable_oflow_fifo_stats()->mutable_drop_counts()->add_entry()->set_type(sys::OflowFifoDropType::WRITE_BUFFER_ACK_FILL_UP_DROP);
        port_stats->mutable_oflow_fifo_stats()->mutable_drop_counts()->mutable_entry(2)->set_count(debug_stats.oflow_fifo_stats.drop_counts.write_buffer_ack_fill_up_drop_count);
        port_stats->mutable_oflow_fifo_stats()->mutable_drop_counts()->add_entry()->set_type(sys::OflowFifoDropType::WRITE_BUFFER_ACK_FULL_DROP);
        port_stats->mutable_oflow_fifo_stats()->mutable_drop_counts()->mutable_entry(3)->set_count(debug_stats.oflow_fifo_stats.drop_counts.write_buffer_ack_full_drop_count);
        port_stats->mutable_oflow_fifo_stats()->mutable_drop_counts()->add_entry()->set_type(sys::OflowFifoDropType::WRITE_BUFFER_FULL_DROP);
        port_stats->mutable_oflow_fifo_stats()->mutable_drop_counts()->mutable_entry(4)->set_count(debug_stats.oflow_fifo_stats.drop_counts.write_buffer_full_drop_count);
        port_stats->mutable_oflow_fifo_stats()->mutable_drop_counts()->add_entry()->set_type(sys::OflowFifoDropType::CONTROL_FIFO_FULL_DROP);
        port_stats->mutable_oflow_fifo_stats()->mutable_drop_counts()->mutable_entry(5)->set_count(debug_stats.oflow_fifo_stats.drop_counts.control_fifo_full_drop_count);

        sdk_ret = asicpd_queue_stats_get(port, (void *) &qos_queue_stats);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        auto q_stats = port_stats->mutable_qos_queue_stats();
        qos_class_queue_stats_to_proto_stats(q_stats, &qos_queue_stats);

        if (first_err_ret == HAL_RET_OK) {
            first_err_ret = ret;
        }
    }

    return first_err_ret;
}

hal_ret_t
pd_pb_stats_clear (pd_func_args_t *pd_func_args)
{
    hal_ret_t              first_err_ret = HAL_RET_OK;
    hal_ret_t              ret = HAL_RET_OK;
    sdk_ret_t              sdk_ret;
    unsigned               port;
    bool                   reset = true;

    HAL_TRACE_DEBUG("Clearing PB stats");

    for (port = 0; port < TM_NUM_PORTS; port++) {
        sdk_ret = asicpd_tm_debug_stats_get(port, NULL, reset);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to clear pb debug stats for port {} ret {}",
                          port, ret);
            // Continue
            if (first_err_ret != HAL_RET_OK) {
                first_err_ret = ret;
            }
        }
    }

    return first_err_ret;
}

hal_ret_t
pd_packet_buffer_update (pd_func_args_t *pd_func_args)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_system_drop_stats_get (pd_func_args_t *pd_func_args)
{
    pd_system_drop_stats_get_args_t    *args = pd_func_args->pd_system_drop_stats_get;
    pd_system_args_t                   *sys_args = args->pd_sys_args;
    pd_drop_stats_get_args_t            d_args;
    pd_egress_drop_stats_get_args_t     ed_args;
    //pd_pb_stats_get_args_t              pb_args;
    pd_func_args_t                      func_args;
    hal_ret_t                           ret;

    d_args.pd_sys_args = sys_args;
    func_args.pd_drop_stats_get = &d_args;
    ret = pd_drop_stats_get(&func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get drop stats, err : {}", ret);
    }

    ed_args.pd_sys_args = sys_args;
    func_args.pd_egress_drop_stats_get = &ed_args;
    ret = pd_egress_drop_stats_get(&func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get egress drop stats, err : {}", ret);
    }

#if 0
    t_args.pd_sys_args = &pd_system_args;
    pd_func_args.pd_table_stats_get = &t_args;
    ret = pd_pb_stats_get(&pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get packet buffer stats, err : {}", ret);
    }
#endif

    return HAL_RET_OK;
}

hal_ret_t
pd_span_threshold_update (pd_func_args_t *pd_func_args)
{
    pd_span_threshold_update_args_t *args =
        pd_func_args->pd_span_threshold_update;

    if (asicpd_tm_set_span_threshold(args->span_threshold) != SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to set span queue threshold");
        return HAL_RET_ERR;
    }
    return HAL_RET_OK;
}

hal_ret_t
pd_system_upgrade_table_reset(pd_func_args_t *pd_func_args)
{
    tcam *inp_prop_mac_vlan_tbl = NULL;
    sdk_hash *inp_prop_tbl = NULL;
    inp_prop_mac_vlan_tbl = g_hal_state_pd->
        tcam_table(P4TBL_ID_INPUT_PROPERTIES_MAC_VLAN);
    HAL_TRACE_DEBUG("De-programming input_properties_mac_vlan entries ...");
    inp_prop_mac_vlan_tbl->deprogram_entries();
    inp_prop_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_INPUT_PROPERTIES);
    HAL_TRACE_DEBUG("De-programming input_properties entries ...");
    inp_prop_tbl->deprogram_entries();

    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal
