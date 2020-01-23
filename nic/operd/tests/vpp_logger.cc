#include <stdio.h>
#include <stdlib.h>

#include "lib/operd/operd.hpp"
#include "lib/operd/logger.hpp"
#include "lib/operd/decoder.h"

#include "nic/operd/vpp-decoder/operd-vpp.h"

int
main (int argc, const char *argv[])
{
    operd_flow_t flow;
    sdk::operd::producer_ptr producer = sdk::operd::producer::create("vpp");

    flow.version = 4;
    flow.v4.src = 0x01010101;
    flow.v4.dst = 0x02020202;
    flow.v4.sport = 10;
    flow.v4.dport = 20;

    producer->write(OPERD_DECODER_VPP, sdk::operd::INFO, &flow, sizeof(flow));

    return 0;
}
