#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/hal/src/lif.hpp"
#include "nic/hal/src/proxy.hpp"
#include "nic/hal/src/lif_manager.hpp"

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

    g_lif_manager->WriteQState(lif_id, 0, qid,
                               (uint8_t *)&qstate, sizeof(qstate));
    g_lif_manager->WriteQState(lif_id, 0, qid + 1,
                               (uint8_t *)&qstate, sizeof(qstate));
    return HAL_RET_OK;
}

} // namespace pd
} // namespace hal
