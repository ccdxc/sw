// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/plugins/cfg/lif/lif.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "gen/p4gen/p4/include/p4pd.h"
#include "nic/hal/pd/iris/internal/p4plus_pd_api.h"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/iris/datapath/p4/include/table_sizes.h"
#include "nic/hal/pd/cpupkt_api.hpp"
#include "lib/periodic/periodic.hpp"

namespace hal {
namespace pd {

#define HAL_IPFIX_DOORBELL_TIMER_INTVL        (1 * TIME_MSECS_PER_SEC)

thread_local void *t_ipfix_doorbell_timer;

typedef struct __attribute__((__packed__)) ipfix_qstate_  {
    uint64_t pc : 8;
    uint64_t rsvd : 8;
    uint64_t cos_a : 4;
    uint64_t coa_b : 4;
    uint64_t cos_sel : 8;
    uint64_t eval_last : 8;
    uint64_t host_rings : 4;
    uint64_t total_rings : 4;
    uint64_t pid : 16;
    uint64_t pindex : 16;
    uint64_t cindex : 16;

    uint64_t pktaddr : 64;
    uint64_t pktsize : 16;
    uint64_t seq_no : 32;
    uint64_t domain_id : 32;
    uint64_t ipfix_hdr_offset : 16;
    uint64_t next_record_offset : 16;

    uint64_t flow_hash_table_type : 8;
    uint64_t flow_hash_index_next : 32;
    uint64_t flow_hash_index_max : 32;
    uint64_t flow_hash_overflow_index_max : 32;

    uint64_t export_time : 32;

    uint8_t  pad[(512-408)/8];
} ipfix_qstate_t;

static void
ipfix_test_init(uint32_t sindex, uint32_t eindex, uint16_t export_id) {
    uint32_t sip   = 0x0A010201;
    uint32_t dip   = 0x0B010201;
    uint8_t  proto = IP_PROTO_TCP;
    uint16_t sport = 0xABBA;
    uint16_t dport = 0xBEEF;
    uint32_t flow_index = 0xDEAF;
    uint32_t session_index = 0x5432;
    uint16_t dst_lport = 0x789;
    uint64_t permit_bytes = 0x40302010a0b0c0d0ull;
    uint64_t permit_packets = 0x7060504030201010ull;
    uint64_t deny_bytes = 0x0f0e0d0c0b0a0908ull;
    uint64_t deny_packets = 0x0102030405060708ull;

    // last exported stats
    uint64_t exported_stats_addr = get_mem_addr(IPFIX_EXPORTED_FLOW_STATS);
    SDK_ASSERT(exported_stats_addr != INVALID_MEM_ADDRESS);
    uint64_t data, data_r = 0;

    // flow hash
    uint8_t *hwkey = NULL;
    uint32_t hwkey_len = 0;
    uint32_t hwdata_len = 0;
    p4pd_hwentry_query(P4TBL_ID_FLOW_HASH, &hwkey_len, NULL, &hwdata_len);
    hwkey_len = (hwkey_len >> 3) + ((hwkey_len & 0x7) ? 1 : 0);
    hwdata_len = (hwdata_len >> 3) + ((hwdata_len & 0x7) ? 1 : 0);
    hwkey = new uint8_t[hwkey_len];

    flow_hash_swkey_t key;
    flow_hash_actiondata_t hash_data;
    for(uint32_t i = sindex; i < eindex; i++) {
        memset(&key, 0, sizeof(key));
        memset(&hash_data, 0, sizeof(hash_data));

        key.flow_lkp_metadata_lkp_type = FLOW_KEY_LOOKUP_TYPE_IPV4;
        key.flow_lkp_metadata_lkp_inst = 0;
        key.flow_lkp_metadata_lkp_vrf = 0;
        memcpy(&key.flow_lkp_metadata_lkp_src, &sip, sizeof(sip));
        memcpy(&key.flow_lkp_metadata_lkp_dst, &dip, sizeof(dip));
        key.flow_lkp_metadata_lkp_proto = proto;
        key.flow_lkp_metadata_lkp_sport = sport;
        key.flow_lkp_metadata_lkp_dport = dport;

        hash_data.action_id = FLOW_HASH_FLOW_HASH_INFO_ID;
        hash_data.action_u.flow_hash_flow_hash_info.entry_valid = true;
        hash_data.action_u.flow_hash_flow_hash_info.export_en = 0x1;
        hash_data.action_u.flow_hash_flow_hash_info.flow_index =
            flow_index + i;

        memset(hwkey, 0, hwkey_len);
        p4pd_hwkey_hwmask_build(P4TBL_ID_FLOW_HASH, &key, NULL, hwkey, NULL);
        p4pd_entry_write(P4TBL_ID_FLOW_HASH, i, hwkey, NULL, &hash_data);

        sip++; dip++; sport++; dport++;

        // flow info
        flow_info_actiondata_t flow_data;
        memset(&flow_data, 0, sizeof(flow_data));
        flow_data.action_id = FLOW_INFO_FLOW_INFO_ID;
        flow_data.action_u.flow_info_flow_info.dst_lport = dst_lport;
        flow_data.action_u.flow_info_flow_info.session_state_index =
            session_index;
        flow_data.action_u.flow_info_flow_info.export_id1 = export_id;
        p4pd_entry_write(P4TBL_ID_FLOW_INFO, flow_index+i, NULL, NULL,
                         &flow_data);

        // session state
        session_state_actiondata_t session_data;
        memset(&session_data, 0, sizeof(session_data));
        p4pd_entry_write(P4TBL_ID_SESSION_STATE, session_index, NULL, NULL,
                         &session_data);

        // flow stats
        flow_stats_actiondata_t stats_data;
        memset(&stats_data, 0, sizeof(stats_data));
        memcpy(stats_data.action_u.flow_stats_flow_stats.permit_bytes,
               &permit_bytes, sizeof(permit_bytes));
        memcpy(stats_data.action_u.flow_stats_flow_stats.permit_packets,
               &permit_packets, sizeof(permit_packets));
        memcpy(stats_data.action_u.flow_stats_flow_stats.drop_bytes,
               &deny_bytes, sizeof(deny_bytes));
        memcpy(stats_data.action_u.flow_stats_flow_stats.drop_packets,
               &deny_packets, sizeof(deny_packets));
        p4pd_entry_write(P4TBL_ID_FLOW_STATS, flow_index+i, NULL, NULL,
                         &stats_data);

        // last exported stats
        data_r = permit_packets - 100;
        memrev((uint8_t *)&data, (uint8_t *)&data_r, sizeof(data));
        sdk::asic::asic_mem_write(exported_stats_addr + ((flow_index + i) << 5),
                                  (uint8_t *)&data, sizeof(data));
        data_r = permit_bytes - (100 * 64);
        memrev((uint8_t *)&data, (uint8_t *)&data_r, sizeof(data));
        sdk::asic::asic_mem_write(exported_stats_addr +
                                  ((flow_index + i) << 5) + sizeof(data),
                                  (uint8_t *)&data, sizeof(data));
        data_r = deny_packets - 10;
        memrev((uint8_t *)&data, (uint8_t *)&data_r, sizeof(data));
        sdk::asic::asic_mem_write(exported_stats_addr +
                                  ((flow_index + i) << 5) + (2 * sizeof(data)),
                                  (uint8_t *)&data, sizeof(data));
        data_r = deny_bytes - (10 * 64);
        memrev((uint8_t *)&data, (uint8_t *)&data_r, sizeof(data));
        sdk::asic::asic_mem_write(exported_stats_addr +
                                  ((flow_index + i) << 5) + (3 * sizeof(data)),
                                  (uint8_t *)&data, sizeof(data));
    }
    delete [] hwkey;
}

hal_ret_t
ipfix_init(uint16_t export_id, uint64_t pktaddr, uint16_t payload_start,
           uint16_t payload_size) {
    int             ret;
    uint8_t         pgm_offset = 0;
    uint32_t        qid = export_id;
    lif_id_t        lif_id = SERVICE_LIF_IPFIX;
    ipfix_qstate_t  qstate = { 0 };


    ret = hal_get_pc_offset("txdma_stage0.bin", "ipfix_tx_stage0", &pgm_offset);

    HAL_ABORT(ret == 0);
    qstate.pc = pgm_offset;
    qstate.total_rings = 1;

    // first records start 16B after ipfix header
    qstate.pktaddr = pktaddr;
    qstate.pktsize = (payload_size > 1500) ? 1500 : payload_size;
    qstate.ipfix_hdr_offset = payload_start;
    qstate.next_record_offset = qstate.ipfix_hdr_offset + 16;

    if (!is_platform_type_hw()) {
        // For SIM, HAPS and RTL mode we need to use a smaller range since
        // the full flow hash table walk takes too long. Also install fake flow
        // entries within that range
        // TODO: Ideally this should be removed and test environment should
        // drive the flow hash table entries
        qstate.flow_hash_index_next = (100 * qid) + 1000;
        qstate.flow_hash_index_max = (100 * qid) + 1011;
        ipfix_test_init(qstate.flow_hash_index_next, qstate.flow_hash_index_max,
                        export_id);
    } else {
        // For HW mode, we need to walk the full flow hash table
        qstate.flow_hash_index_next = 0;
        qstate.flow_hash_index_max = FLOW_HASH_TABLE_SIZE;
        qstate.flow_hash_overflow_index_max = FLOW_HASH_OVERFLOW_TABLE_SIZE;
    }

    lif_manager()->write_qstate(lif_id, 0, qid,
                                (uint8_t *)&qstate, sizeof(qstate));

    lif_manager()->write_qstate(lif_id, 0, qid + 16,
                                (uint8_t *)&qstate, sizeof(qstate));

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// timer callback to ring doorbell to trigger ipfix p4+ program
//------------------------------------------------------------------------------
void
ipfix_doorbell_ring_cb (void *timer, uint32_t timer_id, void *ctxt)
{
    uint64_t address, data, qid;
    uint64_t upd = 3;
    uint64_t qtype = 0, pid = 0, ring_id = 0, p_index = 0;
    uint32_t timev;
    lif_id_t lif_id = SERVICE_LIF_IPFIX;
    ipfix_qstate_t  qstate = { 0 };

    // qid is equal to the exporter id which is encoded in the timer_id
    qid = timer_id - HAL_TIMER_ID_IPFIX_MIN;
    address = DB_ADDR_BASE + (upd << DB_UPD_SHFT) +
        (SERVICE_LIF_IPFIX << DB_LIF_SHFT) + (qtype << DB_TYPE_SHFT);
    data = (pid << DB_PID_SHFT) | (qid << DB_QID_SHFT) |
        (ring_id << DB_RING_SHFT) | p_index;

    // Update (RMW) export time in the Qstate table
    lif_manager()->read_qstate(lif_id, 0, qid,
                                (uint8_t *)&qstate, sizeof(qstate));
    timev = (uint32_t)time(NULL);
    qstate.export_time = timev;
    lif_manager()->write_qstate(lif_id, 0, qid,
                                (uint8_t *)&qstate, sizeof(qstate));

    HAL_TRACE_DEBUG("cpupkt: ringing Doorbell with addr: {:#x} data: {:#x} time: {}",
                    address, data, timev);
    sdk::asic::asic_ring_doorbell(address, data);
    return;
}

//------------------------------------------------------------------------------
// ipfix module initialization
//------------------------------------------------------------------------------
hal_ret_t
ipfix_module_init (hal_cfg_t *hal_cfg)
{
    // wait until the periodic thread is ready
    while (!sdk::lib::periodic_thread_is_running()) {
        pthread_yield();
    }

    // no periodic doorbell in sim mode
    if (is_platform_type_sim()) {
        return HAL_RET_OK;
    }

    t_ipfix_doorbell_timer =
        sdk::lib::timer_schedule(HAL_TIMER_ID_IPFIX_MIN,
                                 HAL_IPFIX_DOORBELL_TIMER_INTVL,
                                 (void *)0,    // ctxt
                                 ipfix_doorbell_ring_cb,
                                 true);
    if (!t_ipfix_doorbell_timer) {
        HAL_TRACE_ERR("Failed to start periodic ipfix doorbell ring timer");
        return HAL_RET_ERR;
    }
    HAL_TRACE_DEBUG("Started periodic ipfix doorbell ring timer with "
                    "{} ms interval", HAL_IPFIX_DOORBELL_TIMER_INTVL);
    return HAL_RET_OK;
}

} // namespace pd
} // namespace hal
