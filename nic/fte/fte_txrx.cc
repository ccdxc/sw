#include "nic/fte/fte.hpp"
#include "nic/fte/fte_flow.hpp"
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "nic/p4/nw/include/defines.h"
#include "nic/include/cpupkt_api.hpp"

namespace fte {

// byte array to hex string for logging
std::string hex_str(const uint8_t *buf, size_t sz)
{
    std::ostringstream result;

    for(size_t i = 0; i < sz; i+=8) {
        result << " 0x";
        for (size_t j = i ; j < sz && j < i+8; j++) {
            result << std::setw(2) << std::setfill('0') << std::hex << (int)buf[j];
        }
    }

    return result.str();
}

// FTE main pkt loop
void
pkt_loop(uint8_t fte_id)
{
    hal_ret_t ret;

    hal::pd::cpupkt_ctxt_t* arm_ctx;
    cpu_rxhdr_t *cpu_rxhdr;
    uint8_t *pkt;
    size_t pkt_len;

    ctx_t ctx;
    flow_t iflow[ctx_t::MAX_STAGES], rflow[ctx_t::MAX_STAGES];

    // init arm driver context
    arm_ctx = hal::pd::cpupkt_ctxt_alloc_init();
    ret = cpupkt_register_rx_queue(arm_ctx, types::WRING_TYPE_ARQRX, fte_id);
    HAL_ASSERT_RETURN_VOID(ret == HAL_RET_OK);

    ret = cpupkt_register_rx_queue(arm_ctx, types::WRING_TYPE_ARQTX, fte_id);
    HAL_ASSERT_RETURN_VOID(ret == HAL_RET_OK);

    ret = cpupkt_register_tx_queue(arm_ctx, types::WRING_TYPE_ASQ);
    HAL_ASSERT_RETURN_VOID(ret == HAL_RET_OK);

    pkt = NULL;
    while(true) {
        // free the last pkt
        if (pkt != NULL) {
            hal::pd::cpupkt_free(cpu_rxhdr, pkt);
        }

        // read the packet
        ret = hal::pd::cpupkt_poll_receive(arm_ctx, &cpu_rxhdr, &pkt, &pkt_len);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("fte: arm rx failed, ret={}", ret);
            continue;
        }

        HAL_TRACE_DEBUG("fte: rxpkt cpu_rxhdr={}",
                        hex_str((uint8_t*)cpu_rxhdr, sizeof(*cpu_rxhdr)));

        HAL_TRACE_DEBUG("fte: rxpkt len={} pkt={}", pkt_len, hex_str(pkt, pkt_len));

        HAL_TRACE_DEBUG("fte: rxpkt slif={} lif={} qtype={} qid={} vrf={} "
                        "pad={} lkp_dir={} lkp_inst={} lkp_type={} flags={} "
                        "l2={} l3={} l4={} payload={}",
                        cpu_rxhdr->src_lif, cpu_rxhdr->lif, cpu_rxhdr->qtype,
                        cpu_rxhdr->qid, cpu_rxhdr->lkp_vrf, cpu_rxhdr->pad,
                        cpu_rxhdr->lkp_dir, cpu_rxhdr->lkp_inst, cpu_rxhdr->lkp_type,
                        cpu_rxhdr->flags, cpu_rxhdr->l2_offset, cpu_rxhdr->l3_offset,
                        cpu_rxhdr->l4_offset, cpu_rxhdr->payload_offset);

        // Process pkt with db open
        hal::hal_cfg_db_open(hal::CFG_OP_READ);
        do {
            // Init ctx_t
            ret = ctx.init(cpu_rxhdr, pkt, pkt_len, iflow, rflow);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("fte: failed to init context, ret={}", ret);
                break;;
            }
            
            // process the packet and update flow table
            ret = ctx.process();
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("fte: failied to process, ret={}", ret);
                break;
            }

            // write the packets
            ctx.send_queued_pkts(arm_ctx);
        } while(false);

        hal::hal_cfg_db_close();
    }
}
} //   namespace fte
