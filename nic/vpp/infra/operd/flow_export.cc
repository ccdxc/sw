//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include "lib/operd/operd.hpp"
#include "lib/operd/logger.hpp"
#include "lib/operd/decoder.h"
#include "flow_export.h"

#define PDS_OPERD_PRODUCER_NAME "vpp"

static sdk::operd::producer_ptr g_operd_producer = NULL;

void
pds_operd_export_flow_ip4 (void *flow)
{

    if (unlikely(!g_operd_producer)) {
        g_operd_producer =
            sdk::operd::producer::create(PDS_OPERD_PRODUCER_NAME);
        SDK_ASSERT(g_operd_producer);
    }

    g_operd_producer->write(OPERD_DECODER_VPP,
                            sdk::operd::INFO,
                            (operd_flow_t *)flow,
                            sizeof(*(operd_flow_t *)flow));
    return;
}

void
pds_operd_export_flow_ip6 (uint8_t *sip,
                           uint8_t *dip,
                           uint8_t ip_proto,
                           uint16_t src_port,
                           uint16_t dst_port,
                           uint16_t lookup_id,
                           const operd_flow_stats_t *flow_stats,
                           uint32_t session_id,
                           uint8_t logtype,
                           uint8_t allow)
{
    operd_flow_t flow;
    uint64_t *src, *dst;

    if (unlikely(!g_operd_producer)) {
        g_operd_producer =
            sdk::operd::producer::create(PDS_OPERD_PRODUCER_NAME);
        SDK_ASSERT(g_operd_producer);
    }

    flow.type = OPERD_FLOW_TYPE_IP6;
    flow.action = allow ? OPERD_FLOW_ACTION_ALLOW : OPERD_FLOW_ACTION_DENY;
    flow.logtype = logtype;
    flow.session_id = session_id;
    dst = (uint64_t *) flow.v6.src;
    src = (uint64_t *) sip;
    dst[0] = src[0];
    dst[1] = src[1];
    dst = (uint64_t *) flow.v6.dst;
    src = (uint64_t *) dip;
    dst[0] = src[0];
    dst[1] = src[1];
    flow.v6.proto = ip_proto;
    flow.v6.sport = src_port;
    flow.v6.dport = dst_port;
    flow.v6.lookup_id = lookup_id;
    memcpy(&flow.stats, flow_stats, sizeof(operd_flow_stats_t));

    g_operd_producer->write(OPERD_DECODER_VPP, sdk::operd::INFO, &flow, sizeof(flow));
    return;
}

void
pds_operd_export_flow_l2 (uint8_t *smac,
                          uint8_t *dmac,
                          uint8_t ether_type,
                          uint16_t bd_id,
                          const operd_flow_stats_t *flow_stats,
                          uint32_t session_id,
                          uint8_t logtype,
                          uint8_t allow)
{
    operd_flow_t flow;

    if (unlikely(!g_operd_producer)) {
        g_operd_producer =
            sdk::operd::producer::create(PDS_OPERD_PRODUCER_NAME);
        SDK_ASSERT(g_operd_producer);
    }

    flow.type = OPERD_FLOW_TYPE_L2;
    flow.action = allow ? OPERD_FLOW_ACTION_ALLOW : OPERD_FLOW_ACTION_DENY;
    flow.logtype = logtype;
    flow.session_id = session_id;
    memcpy(flow.l2.src, smac, ETH_ADDR_LEN);
    memcpy(flow.l2.dst, dmac, ETH_ADDR_LEN);
    flow.l2.ether_type = ether_type;
    flow.l2.bd_id = bd_id;
    memcpy(&flow.stats, flow_stats, sizeof(operd_flow_stats_t));

    g_operd_producer->write(OPERD_DECODER_VPP, sdk::operd::INFO, &flow, sizeof(flow));
    return;
}

