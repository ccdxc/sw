#include "../../../p4/include/intrinsic.p4"

#include "../include/defines.h"
#include "../include/table_sizes.h"
#include "../include/sacl_defines.h"
#include "../include/headers.p4"

#include "sacl.p4"
#include "metadata.p4"
#include "udp_flow.p4"
#include "packet_queue.p4"

parser start {
    return ingress;
}

control ingress {
    sacl();
    udp_flow_queuing();
    pkt_enqueue();
}

