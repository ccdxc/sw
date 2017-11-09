#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "nic/hal/src/proxy.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/hal/pd/common/pd_api.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/endpoint_pd.hpp"
#include "nic/p4/nw/include/defines.h"
#include "nic/asm/cpu-p4plus/include/cpu-defines.h"
#include "nic/hal/src/session.hpp"
#include "nic/include/base.h"
#include "nic/include/encap.hpp"
#include "nic/include/list.hpp"
#include "nic/utils/ht/ht.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/pd/common/cpupkt_headers.hpp"
#include "nic/hal/pd/common/cpupkt_api.hpp"
#include "nic/asm/cpu-p4plus/include/cpu-defines.h"

namespace hal {
namespace proxy {

uint8_t pkt[] = {
    0x00, 0xee, 0xff, 0x00, 0x00, 0x05, 0x00, 0xee, 0xff, 0x00, 0x00, 0x04,
    0xaa, 0xaa, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

thread_local fte::ctx_t *my_gl_ctx;

void
quiesce_transmit_pkt(unsigned char* pkt,
        unsigned int len)
{
    if (my_gl_ctx) {
        hal::pd::cpu_to_p4plus_header_t cpu_header;
        hal::pd::p4plus_to_p4_header_t  p4plus_header;

        p4plus_header.flags = 0;
        cpu_header.src_lif = hal::SERVICE_LIF_CPU;
        cpu_header.hw_vlan_id = 0;
        cpu_header.flags = 0;
        cpu_header.l2_offset = 0;
        HAL_TRACE_DEBUG("quiesce: txpkt cpu_header src_lif={} hw_vlan_id={} flags={}",
                cpu_header.src_lif, cpu_header.hw_vlan_id, cpu_header.flags);

        my_gl_ctx->queue_txpkt(pkt,
                               len,
                               &cpu_header,
                               &p4plus_header,
                               hal::SERVICE_LIF_CPU,
                               CPU_ASQ_QTYPE,
                               types::CPUCB_ID_QUIESCE,
                               CPU_SCHED_RING_ASQ);
    }
}

void
quiesce_message_tx(void) {
    unsigned int len = sizeof(pkt)/sizeof(pkt[0]);
    HAL_TRACE_DEBUG("quiesce_message_tx");
    quiesce_transmit_pkt(pkt, len);
}

fte::pipeline_action_t
quiesce_exec_cpu_lif(fte::ctx_t& ctx)
{
    my_gl_ctx = &ctx;
    static uint8_t          count = 1;
    HAL_TRACE_DEBUG("quiesce_exec_cpu_lif: count: {}", count);
    if (count == 1) {
        if (!ctx.protobuf_request()) {
            quiesce_message_tx();
            count++;
        }
    }
    return fte::PIPELINE_CONTINUE;
}

fte::pipeline_action_t
quiesce_exec_cpu_rx_lif(fte::ctx_t& ctx)
{
    HAL_TRACE_DEBUG("quiesce_exec_cpu_rx_lif");
    return fte::PIPELINE_CONTINUE;
}

fte::pipeline_action_t
quiesce_exec(fte::ctx_t& ctx)
{
    const fte::cpu_rxhdr_t* cpu_rxhdr = ctx.cpu_rxhdr();
    HAL_TRACE_DEBUG("quiesce_exec: lif: {}",  cpu_rxhdr->lif);
    if (cpu_rxhdr && (cpu_rxhdr->lif == hal::SERVICE_LIF_CPU)) {
        return quiesce_exec_cpu_rx_lif(ctx);
    } else {
        return quiesce_exec_cpu_lif(ctx);
    }

    return fte::PIPELINE_CONTINUE;
}

} // namespace hal
} // namespace net
