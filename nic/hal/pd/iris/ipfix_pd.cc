#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/hal/src/lif.hpp"
#include "nic/hal/src/proxy.hpp"
#include "nic/hal/src/lif_manager.hpp"

#include "nic/p4/nw/include/defines.h"
#include "nic/hal/pd/p4pd_api.hpp"
#include "nic/gen/iris/include/p4pd.h"
#include "nic/hal/pd/iris/p4plus_pd_api.h"
#include "nic/hal/pd/capri/capri_hbm.hpp"

namespace hal {
namespace pd {

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
    uint64_t sindex : 32;
    uint64_t eindex : 32;
    uint64_t pktaddr : 64;
    uint64_t pktsize : 16;
    uint64_t rstart : 16;
    uint64_t rnext : 16;
    uint8_t  pad[(512-272)/8];
} ipfix_qstate_t;

static void
ipfix_test_init(uint32_t sindex, uint32_t eindex) {
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

    // flow hash
    uint8_t *hwkey = NULL;
    uint32_t hwkey_len = 0;
    uint32_t hwdata_len = 0;
    p4pd_hwentry_query(P4TBL_ID_FLOW_HASH, &hwkey_len, NULL, &hwdata_len);
    hwkey_len = (hwkey_len >> 3) + ((hwkey_len & 0x7) ? 1 : 0);
    hwdata_len = (hwdata_len >> 3) + ((hwdata_len & 0x7) ? 1 : 0);
    hwkey = new uint8_t[hwkey_len];

    flow_hash_swkey_t key;
    flow_hash_actiondata hash_data;
    for(; sindex <= eindex; sindex++) {
        memset(&key, 0, sizeof(key));
        memset(&hash_data, 0, sizeof(hash_data));

        key.flow_lkp_metadata_lkp_type = FLOW_KEY_LOOKUP_TYPE_IPV4;
        key.flow_lkp_metadata_lkp_inst = 0;
        key.flow_lkp_metadata_lkp_dir = 0;
        key.flow_lkp_metadata_lkp_vrf = 0;
        memcpy(&key.flow_lkp_metadata_lkp_src, &sip, sizeof(sip));
        memcpy(&key.flow_lkp_metadata_lkp_dst, &dip, sizeof(dip));
        key.flow_lkp_metadata_lkp_proto = proto;
        key.flow_lkp_metadata_lkp_sport = sport;
        key.flow_lkp_metadata_lkp_dport = dport;

        hash_data.actionid = FLOW_HASH_FLOW_HASH_INFO_ID;
        hash_data.flow_hash_action_u.flow_hash_flow_hash_info.entry_valid = TRUE;
        hash_data.flow_hash_action_u.flow_hash_flow_hash_info.export_en = TRUE;
        hash_data.flow_hash_action_u.flow_hash_flow_hash_info.flow_index =
            flow_index;

        memset(hwkey, 0, hwkey_len);
        p4pd_hwkey_hwmask_build(P4TBL_ID_FLOW_HASH, &key, NULL, hwkey, NULL);
        p4pd_entry_write(P4TBL_ID_FLOW_HASH, sindex, hwkey, NULL, &hash_data);

        sip++; dip++; sport++; dport++;
    }
    delete [] hwkey;

    // flow info
    flow_info_actiondata flow_data;
    memset(&flow_data, 0, sizeof(flow_data));
    flow_data.actionid = FLOW_INFO_FLOW_INFO_ID;
    flow_data.flow_info_action_u.flow_info_flow_info.dst_lport = dst_lport;
    flow_data.flow_info_action_u.flow_info_flow_info.session_state_index =
        session_index;
    p4pd_entry_write(P4TBL_ID_FLOW_INFO, flow_index, NULL, NULL, &flow_data);

    // session state
    session_state_actiondata session_data;
    memset(&session_data, 0, sizeof(session_data));
    p4pd_entry_write(P4TBL_ID_SESSION_STATE, session_index, NULL, NULL,
                     &session_data);
    // flow stats
    flow_stats_actiondata stats_data;
    memset(&stats_data, 0, sizeof(stats_data));
    p4pd_entry_write(P4TBL_ID_FLOW_STATS, flow_index, NULL, NULL, &stats_data);

    // atomic add region
    uint64_t base_addr = get_start_offset(JP4_ATOMIC_STATS) + (flow_index * 32);
    p4plus_hbm_write(base_addr, (uint8_t *)&permit_bytes, sizeof(permit_bytes));
    p4plus_hbm_write(base_addr + 8 , (uint8_t *)&permit_packets,
                     sizeof(permit_packets));
    p4plus_hbm_write(base_addr + 16, (uint8_t *)&deny_bytes,
                     sizeof(deny_bytes));
    p4plus_hbm_write(base_addr + 24 , (uint8_t *)&deny_packets,
                     sizeof(deny_packets));
}

hal_ret_t
ipfix_init(uint16_t export_id, uint64_t pktaddr, uint16_t payload_start,
           uint16_t payload_size) {
    lif_id_t lif_id = SERVICE_LIF_IPFIX;
    uint32_t qid = export_id * 2;

    ipfix_qstate_t qstate = { 0 };
    uint8_t pgm_offset = 0;
    int ret = g_lif_manager->GetPCOffset("p4plus", "txdma_stage0.bin",
                                         "ipfix_tx_stage0", &pgm_offset);
    HAL_ABORT(ret == 0);
    qstate.pc = pgm_offset;
    qstate.total_rings = 1;

    // first records start 16B after ipfix header
    qstate.pktaddr = htobe64(pktaddr);
    qstate.pktsize = payload_size;
    qstate.rstart = payload_start;
    qstate.rnext = qstate.rstart + 16;
    qstate.sindex = 100;
    qstate.eindex = 110;

    // install entries for testing (to be removed)
    ipfix_test_init(qstate.sindex, qstate.eindex);

    g_lif_manager->WriteQState(lif_id, 0, qid,
                               (uint8_t *)&qstate, sizeof(qstate));
    g_lif_manager->WriteQState(lif_id, 0, qid + 1,
                               (uint8_t *)&qstate, sizeof(qstate));
    return HAL_RET_OK;
}

} // namespace pd
} // namespace hal
