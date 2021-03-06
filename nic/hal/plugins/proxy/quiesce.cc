//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"
#include "nic/hal/plugins/cfg/nw/interface_api.hpp"
#include "nic/hal/pd/pd_api.hpp"
//#include "nic/hal/pd/iris/hal_state_pd.hpp"
//#include "nic/hal/pd/iris/endpoint_pd.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/asm/cpu-p4plus/include/cpu-defines.h"
#include "nic/hal/plugins/cfg/nw/session.hpp"
#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/encap.hpp"
#include "lib/list/list.hpp"
#include "lib/ht/ht.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/pd/cpupkt_headers.hpp"
#include "nic/hal/pd/cpupkt_api.hpp"
#include "nic/asm/cpu-p4plus/include/cpu-defines.h"
#include "nic/fte/fte.hpp"

namespace hal {
namespace proxy {

uint8_t pkt[] = {
    0x00, 0xee, 0xff, 0x00, 0x00, 0x05, 0x00, 0xee, 0xff, 0x00, 0x00, 0x04,
    0xaa, 0xaa, //0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  //  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void
quiesce_transmit_pkt(void* data)
{
    unsigned int len = sizeof(pkt)/sizeof(pkt[0]);
    HAL_TRACE_DEBUG("quiesce_transmit_pkt len={}", len);
    hal::pd::cpu_to_p4plus_header_t cpu_header = {};
    hal::pd::p4plus_to_p4_header_t  p4plus_header = {};

    p4plus_header.p4plus_app_id = P4PLUS_APPTYPE_CPU;
    cpu_header.src_lif = HAL_LIF_CPU;
    cpu_header.hw_vlan_id = 0;
    cpu_header.flags = CPU_TO_P4PLUS_FLAGS_ADD_TX_QS_TRLR;
    cpu_header.l2_offset = 0;
    HAL_TRACE_DEBUG("quiesce: txpkt cpu_header src_lif={} hw_vlan_id={} flags={}",
            cpu_header.src_lif, cpu_header.hw_vlan_id, cpu_header.flags);
    fte::fte_asq_send(&cpu_header, &p4plus_header, pkt, len);
}

void
quiesce_message_tx(void) {
    HAL_TRACE_DEBUG("quiesce_message_tx");
    fte::fte_softq_enqueue(0, quiesce_transmit_pkt, NULL);
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
    if (cpu_rxhdr && (cpu_rxhdr->lif == HAL_LIF_CPU)) {
        return quiesce_exec_cpu_rx_lif(ctx);
    }

    return fte::PIPELINE_CONTINUE;
}

} // namespace hal
} // namespace net
