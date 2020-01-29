#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "flow_decoder.h"
#include "lib/operd/decoder.h"

static size_t
vpp_decoder (uint8_t encoder, const char *data, size_t data_length,
             char *output, size_t output_size)
{
    const operd_flow_t *flow = (operd_flow_t *)data;
    int len;

    assert(sizeof(*flow) == data_length);

    switch (flow->type) {
    case OPERD_FLOW_TYPE_IP4:
        len = snprintf(output, output_size, "IPv4 0x%x 0%x %hu %hu",
                       flow->v4.src, flow->v4.dst, flow->v4.sport,
                       flow->v4.dport);
        break;
    case OPERD_FLOW_TYPE_IP6:
        len = snprintf(output, output_size, "IPv6 flow");
        break;
    case OPERD_FLOW_TYPE_L2:
        len = snprintf(output, output_size, "L2 flow");
        break;
    default:
        len = 0;
        break;
    }

    return len;
}

extern "C" void
decoder_lib_init(register_decoder_fn register_decoder)
{
    register_decoder(OPERD_DECODER_VPP, vpp_decoder);
}
