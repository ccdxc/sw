#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "flow_decoder.h"
#include "gen/proto/oper.pb.h"
#include "gen/proto/types.pb.h"
#include "lib/operd/decoder.h"

static uint64_t
mac_to_u64 (const uint8_t *mac)
{
    return
        static_cast<uint64_t>(mac[0]) << 0  |
        static_cast<uint64_t>(mac[1]) << 8  |
        static_cast<uint64_t>(mac[2]) << 16 |
        static_cast<uint64_t>(mac[3]) << 24 |
        static_cast<uint64_t>(mac[4]) << 32 |
        static_cast<uint64_t>(mac[5]) << 40;
}

static size_t
vpp_decoder (uint8_t encoder, const char *data, size_t data_length,
             char *output, size_t output_size)
{
    const operd_flow_t *flow = (operd_flow_t *)data;
    pds::FlowLog flowLog;
    size_t len;
    
    assert(sizeof(*flow) == data_length);

    switch (flow->action) {
    case OPERD_FLOW_ACTION_ALLOW:
        flowLog.set_flowaction(::types::SECURITY_RULE_ACTION_ALLOW);
        break;
    case OPERD_FLOW_ACTION_DENY:
        flowLog.set_flowaction(::types::SECURITY_RULE_ACTION_DENY);
        break;
    default:
        assert(0);
    }

    switch (flow->op) {
    case OPERD_FLOW_OPERATION_ADD:
        flowLog.set_logtype(::pds::FLOW_LOG_TYPE_OPEN);
        break;
    case OPERD_FLOW_OPERATION_DEL:
        flowLog.set_logtype(::pds::FLOW_LOG_TYPE_CLOSE);
        break;
    default:
        assert(0);
    }
    
    switch (flow->type) {
    case OPERD_FLOW_TYPE_IP4:
    {
        types::FlowKey *flowKey = new types::FlowKey();
        flowLog.set_allocated_key(flowKey);

        types::IPFlowKey *ipKey = new types::IPFlowKey();
        flowKey->set_allocated_ipflowkey(ipKey);

        ipKey->set_vpcid(&flow->v4.lookup_id, sizeof(flow->v4.lookup_id));
        
        types::IPAddress *srcIp = new types::IPAddress();
        srcIp->set_af(types::IP_AF_INET);
        srcIp->set_v4addr(flow->v4.src);
        ipKey->set_allocated_srcip(srcIp);

        types::IPAddress *dstIp = new types::IPAddress();
        dstIp->set_af(types::IP_AF_INET);
        dstIp->set_v4addr(flow->v4.dst);
        ipKey->set_allocated_dstip(dstIp);
        
        ipKey->set_ipprotocol(flow->v4.proto);

        types::FlowL4Info *l4Info = new types::FlowL4Info();
        ipKey->set_allocated_l4info(l4Info);
        
        types::FlowL4Info::TCPUDPInfo *tcpUdpInfo =
            new types::FlowL4Info::TCPUDPInfo();
        l4Info->set_allocated_tcpudpinfo(tcpUdpInfo);
        tcpUdpInfo->set_srcport(flow->v4.sport);
        tcpUdpInfo->set_dstport(flow->v4.dport);
    }
    break;
    case OPERD_FLOW_TYPE_IP6:
    {
        types::FlowKey *flowKey = new types::FlowKey();
        flowLog.set_allocated_key(flowKey);

        types::IPFlowKey *ipKey = new types::IPFlowKey();
        flowKey->set_allocated_ipflowkey(ipKey);

        ipKey->set_vpcid(&flow->v6.lookup_id, sizeof(flow->v6.lookup_id));
        
        types::IPAddress *srcIp = new types::IPAddress();
        srcIp->set_af(types::IP_AF_INET6);
        srcIp->set_v6addr(&flow->v6.src, IP6_ADDR8_LEN);
        ipKey->set_allocated_srcip(srcIp);

        types::IPAddress *dstIp = new types::IPAddress();
        dstIp->set_af(types::IP_AF_INET6);
        dstIp->set_v6addr(flow->v6.dst, IP6_ADDR8_LEN);
        ipKey->set_allocated_dstip(dstIp);
        
        ipKey->set_ipprotocol(flow->v6.proto);

        types::FlowL4Info *l4Info = new types::FlowL4Info();
        ipKey->set_allocated_l4info(l4Info);
        
        types::FlowL4Info::TCPUDPInfo *tcpUdpInfo =
            new types::FlowL4Info::TCPUDPInfo();
        l4Info->set_allocated_tcpudpinfo(tcpUdpInfo);
        tcpUdpInfo->set_srcport(flow->v6.sport);
        tcpUdpInfo->set_dstport(flow->v6.dport);
    }
    break;
    case OPERD_FLOW_TYPE_L2:
    {
        types::FlowKey *flowKey = new types::FlowKey();
        flowLog.set_allocated_key(flowKey);

        types::MACFlowKey *macKey = new types::MACFlowKey();
        flowKey->set_allocated_macflowkey(macKey);

        macKey->set_subnet(&flow->l2.bd_id, sizeof(flow->l2.bd_id));
        macKey->set_srcmac(mac_to_u64(flow->l2.src));
        macKey->set_dstmac(mac_to_u64(flow->l2.dst));
    }
    break;
    default:
        assert(0);
        break;
    }

    len = flowLog.ByteSizeLong();
    assert(len <= output_size);
    bool result = flowLog.SerializeToArray(output, output_size);
    assert(result == true);

    return len;
}

extern "C" void
decoder_lib_init(register_decoder_fn register_decoder)
{
    register_decoder(OPERD_DECODER_VPP, vpp_decoder);
}
