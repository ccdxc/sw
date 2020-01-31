#include <arpa/inet.h>
#include <inttypes.h>

#include "gen/proto/oper.pb.h"
#include "gen/proto/types.pb.h"
#include "lib/operd/operd.hpp"

extern "C" {

void
handler(sdk::operd::log_ptr entry)
{
    pds::FlowLog flow;

    bool result = flow.ParseFromArray(entry->data(), entry->data_length());
    assert(result == true);

    switch (flow.logtype()) {
    case pds::FLOW_LOG_TYPE_OPEN:
        printf("Add, ");
        break;
    case pds::FLOW_LOG_TYPE_CLOSE:
        printf("Delete, ");
        break;
    default:
        assert(0);
    }
    switch (flow.flowaction()) {
    case types::SECURITY_RULE_ACTION_ALLOW:
        printf("allow, ");
        break;
    case types::SECURITY_RULE_ACTION_DENY:
        printf("deny, ");
        break;
    default:
        assert(0);
    }

    if (flow.has_key()) {
        if (flow.key().has_ipflowkey()) { 
            struct in_addr ip_addr;
            printf("ip, ");
            if (flow.key().ipflowkey().srcip().af() == types::IP_AF_INET) {
                ip_addr.s_addr = flow.key().ipflowkey().srcip().v4addr();
                printf("source: %s:%" PRIu32 ", ", inet_ntoa(ip_addr),
                       flow.key().ipflowkey().l4info().tcpudpinfo().srcport());
            }

            if (flow.key().ipflowkey().dstip().af() == types::IP_AF_INET) {
                ip_addr.s_addr = flow.key().ipflowkey().dstip().v4addr();
                printf("destination: %s:%" PRIu32 ", ", inet_ntoa(ip_addr),
                       flow.key().ipflowkey().l4info().tcpudpinfo().dstport());
            }
            
            printf("proto: %" PRIu32 "\n", flow.key().ipflowkey().ipprotocol());
        } else if (flow.key().has_macflowkey()) {
            printf("l2\n");
        }
    }
}

}
