#include "fte.hpp"
#include "fte_flow.hpp"
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <defines.h>
#include <cpupkt_api.hpp>

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
    ret = cpupkt_register_rx_queue(arm_ctx, types::WRING_TYPE_ARQRX);
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
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        do {
            // Init ctx_t
            ret = ctx.init(cpu_rxhdr, pkt, pkt_len, iflow, rflow);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("fte: failied to init context, ret={}", ret);
                break;;
            }
            
            // execute the pipeline
            ret = execute_pipeline(ctx);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("fte: failied to execute pipeline, ret={}", ret);
                break;
            }
            
            // update GFT
            ret = ctx.update_gft();
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("fte: failied to updated gft, ret={}", ret);
                break;
            }
        } while(false);

        if (ret != HAL_RET_OK) {
            hal::hal_cfg_db_close(true);
            continue;
        }

        // Update and send the packet
        hal::hal_cfg_db_close(false);

        // write the packet
        if (ctx.pkt()) {
            hal::pd::p4plus_to_p4_header_t p4plus_header = {};
            hal::pd::cpu_to_p4plus_header_t cpu_header = {};

            cpu_header.src_lif = ctx.cpu_rxhdr()->src_lif;

            HAL_TRACE_DEBUG("fte:: txpkt slif={} len={} pkt={}", cpu_header.src_lif,
                            ctx.pkt_len(), hex_str(ctx.pkt(), ctx.pkt_len()));

            ret = hal::pd::cpupkt_send(arm_ctx, &cpu_header, &p4plus_header,
                                       (uint8_t *)ctx.pkt(), ctx.pkt_len());

            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("fte: failied to transmit pkt, ret={}", ret);
                continue;
            }
        }
    }
}
} //   namespace fte
