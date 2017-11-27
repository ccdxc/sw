#include "nic/include/hal_lock.hpp"
#include "nic/p4/nw/include/table_sizes.h"
#include "nic/hal/pd/utils/tcam/tcam.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/system_pd.hpp"
#include "nic/p4/nw/include/defines.h"


namespace hal {
namespace pd {

hal_ret_t
pd_system_populate_drop_stats(DropStatsEntry *stats_entry, 
                              uint8_t idx);

hal_ret_t
pd_drop_stats_get(pd_system_args_t *pd_sys_args)
{
    hal_ret_t               ret = HAL_RET_OK;
    SystemResponse          *rsp = pd_sys_args->rsp;
    DropStatsEntry          *stats_entry = NULL;

    HAL_TRACE_DEBUG("PD-System: Querying drop stats");

    for (int i = 0; i < DROP_STATS_TABLE_SIZE; i++) {
        stats_entry = rsp->mutable_stats()->mutable_drop_stats()->
            add_drop_entries();
        pd_system_populate_drop_stats(stats_entry, i);
    }

    return ret;
}

hal_ret_t
pd_system_populate_drop_stats(DropStatsEntry *stats_entry, 
                              uint8_t idx)
{
    hal_ret_t               ret = HAL_RET_OK;
    Tcam                    *tcam;
    drop_stats_swkey         key = { 0 };
    drop_stats_swkey_mask    key_mask = { 0 };
    drop_stats_actiondata    data = { 0 };
    uint64_t                 hbm_counter = 0, 
                             hbm_counter1  = 0, hbm_counter2 = 0;


    HAL_TRACE_DEBUG("PD-System: Populating drop stats");


    tcam = g_hal_state_pd->tcam_table(P4TBL_ID_DROP_STATS);
    HAL_ASSERT(tcam != NULL);

    // Retrieve from SW to get the stats idx
    ret = tcam->retrieve(idx, &key, &key_mask, &data);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to retrieve stats idx for: {}",
                idx);
        goto end;
    }

    // TODO: Read count from HBM with stats idx.
    // hbm_counter1 = 

    // Read from drop stats table
    ret = tcam->retrieve_from_hw(idx, &key, &key_mask, &data);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to retrieve drop stats for entry: {}",
                idx);
        goto end;
    }

    if (key.entry_inactive_drop_stats) {
        goto end;
    }


    // TODO: Read count from HBM with stats idx.
    // hbm_counter2 = 

    // TODO: Do we need a check. Why cant we read drop_stats, hbm 
    if (hbm_counter2 > hbm_counter1) {
        hbm_counter = hbm_counter2;
    } else {
        hbm_counter = hbm_counter1;
    }

    pd_system_decode(&key, &key_mask, &data, stats_entry, hbm_counter);

end:
    return ret;
}

hal_ret_t
pd_system_decode(drop_stats_swkey *key, drop_stats_swkey_mask *key_mask, 
        drop_stats_actiondata *data, DropStatsEntry *stats_entry,
        uint64_t hbm_counter)
{
    hal_ret_t   ret = HAL_RET_OK;

    key->control_metadata_drop_reason &= 
        key_mask->control_metadata_drop_reason_mask;

    stats_entry->set_drop_input_mapping(
            key->control_metadata_drop_reason & DROP_INPUT_MAPPING);
    stats_entry->set_drop_input_mapping_dejavu(
            key->control_metadata_drop_reason & DROP_INPUT_MAPPING_DEJAVU);
    stats_entry->set_drop_flow_hit(
            key->control_metadata_drop_reason & DROP_FLOW_HIT);
    stats_entry->set_drop_flow_miss(
            key->control_metadata_drop_reason & DROP_FLOW_MISS);
    stats_entry->set_drop_ipsg(
            key->control_metadata_drop_reason & DROP_IPSG);
    stats_entry->set_drop_ingress_policer(
            key->control_metadata_drop_reason & DROP_INGRESS_POLICER);
    stats_entry->set_drop_egress_policer(
            key->control_metadata_drop_reason & DROP_EGRESS_POLICER);
    stats_entry->set_drop_nacl(
            key->control_metadata_drop_reason & DROP_NACL);
    stats_entry->set_drop_malformed_pkt(
            key->control_metadata_drop_reason & DROP_MALFORMED_PKT);
    stats_entry->set_drop_ping_of_death(
            key->control_metadata_drop_reason & DROP_PING_OF_DEATH);
    stats_entry->set_drop_fragment_too_small(
            key->control_metadata_drop_reason & DROP_FRAGMENT_TOO_SMALL);
    stats_entry->set_drop_ip_normalization(
            key->control_metadata_drop_reason & DROP_IP_NORMALIZATION);
    stats_entry->set_drop_tcp_normalization(
            key->control_metadata_drop_reason & DROP_TCP_NORMALIZATION);
    stats_entry->set_drop_tcp_non_syn_first_pkt(
            key->control_metadata_drop_reason & DROP_TCP_NON_SYN_FIRST_PKT);
    stats_entry->set_drop_icmp_normalization(
            key->control_metadata_drop_reason & DROP_ICMP_NORMALIZATION);
    stats_entry->set_drop_icmp_src_quench_msg(
            key->control_metadata_drop_reason & DROP_ICMP_SRC_QUENCH_MSG);
    stats_entry->set_drop_icmp_redirect_msg(
            key->control_metadata_drop_reason & DROP_ICMP_REDIRECT_MSG);
    stats_entry->set_drop_icmp_info_req_msg(
            key->control_metadata_drop_reason & DROP_ICMP_INFO_REQ_MSG);
    stats_entry->set_drop_icmp_addr_req_msg(
            key->control_metadata_drop_reason & DROP_ICMP_ADDR_REQ_MSG);
    stats_entry->set_drop_icmp_traceroute_msg(
            key->control_metadata_drop_reason & DROP_ICMP_TRACEROUTE_MSG);
    stats_entry->set_drop_icmp_rsvd_type_msg(
            key->control_metadata_drop_reason & DROP_ICMP_RSVD_TYPE_MSG);
    stats_entry->set_drop_input_properties_miss(
            key->control_metadata_drop_reason & DROP_INPUT_PROPERTIES_MISS);
    stats_entry->set_drop_tcp_out_of_window(
            key->control_metadata_drop_reason & DROP_TCP_OUT_OF_WINDOW);
    stats_entry->set_drop_tcp_split_handshake(
            key->control_metadata_drop_reason & DROP_TCP_SPLIT_HANDSHAKE);
    stats_entry->set_drop_tcp_win_zero_drop(
            key->control_metadata_drop_reason & DROP_TCP_WIN_ZERO_DROP);
    stats_entry->set_drop_tcp_ack_err(
            key->control_metadata_drop_reason & DROP_TCP_ACK_ERR);
    stats_entry->set_drop_tcp_data_after_fin(
            key->control_metadata_drop_reason & DROP_TCP_DATA_AFTER_FIN);
    stats_entry->set_drop_tcp_non_rst_pkt_after_rst(
            key->control_metadata_drop_reason & DROP_TCP_NON_RST_PKT_AFTER_RST);
    stats_entry->set_drop_tcp_invalid_responder_first_pkt(
            key->control_metadata_drop_reason & DROP_TCP_INVALID_RESPONDER_FIRST_PKT);
    stats_entry->set_drop_tcp_unexpected_syn(
            key->control_metadata_drop_reason & DROP_TCP_UNEXPECTED_PKT);

    stats_entry->set_drop_count(
            hbm_counter + 
            data->drop_stats_action_u.drop_stats_drop_stats.drop_pkts);


    return ret;
}


}    // namespace pd
}    // namespace hal
