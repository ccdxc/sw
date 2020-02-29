//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// schema definitions for metrics and other helpers
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/lib/metrics/metrics.hpp"

sdk::metrics::metrics_schema_t port_metrics_schema = {
    .name = "Port",
    .counters = {
        {
            .name = "Frames Rx ok",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx all",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx bad fcs",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx bad all",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Octets Rx ok",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Octets Rx all",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx unicast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx multicast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx broadcast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx pause",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx bad length",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx undersized",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx oversized",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx fragments",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx jabber",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx priority pause",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx stomped crc",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx too long",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx vlan good",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx dropped",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx less than 64B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx 64B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx 65B-127B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx 128B-255B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx 256B-511B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx 512B-1023B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx 1024B-1518B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx 1519B-2047B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx 2048B-4095B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx 4096B-8191B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx 8192B-9215B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx other",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx ok",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx all",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx bad",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Octets Tx ok",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Octets Tx total",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx unicast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx multicast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx broadcast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx pause",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx priority pause",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx vlan",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx less than 64B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx 64B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx 65B-127B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx 128B-255B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx 256B-511B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx 512B-1023B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx 1024B-1518B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx 1519B-2047B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx 2048B-4095B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx 4096B-8191B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx 8192B-9215B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx other",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx priority 0",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx priority 1",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx priority 2",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx priority 3",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx priority 4",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx priority 5",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx priority 6",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx priority 7",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx priority 0",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx priority 1",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx priority 2",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx priority 3",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx priority 4",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx priority 5",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx priority 6",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx priority 7",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Tx priority pause 0 1us count",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Tx priority pause 1 1us count",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Tx priority pause 2 1us count",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Tx priority pause 3 1us count",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Tx priority pause 4 1us count",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Tx priority pause 5 1us count",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Tx priority pause 6 1us count",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Tx priority pause 7 1us count",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx priority pause 0 1us count",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx priority pause 1 1us count",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx priority pause 2 1us count",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx priority pause 3 1us count",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx priority pause 4 1us count",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx priority pause 5 1us count",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx priority pause 6 1us count",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx priority pause 7 1us count",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx pause 1us count",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx truncated",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = NULL,
        },
    }
};

sdk::metrics::metrics_schema_t mgmt_port_metrics_schema = {
    .name = "Port",
    .counters = {
        {
            .name = "Frames Rx ok",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx all",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx bad fcs",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx bad all",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Octets Rx ok",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Octets Rx all",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx unicast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx multicast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx broadcast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx pause",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx bad length",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx undersized",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx oversized",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx fragments",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx jabber",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx 64B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx 65B-127B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx 128B-255B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx 256B-511B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx 512B-1023B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx 1024B-1518B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx > 1518B",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx fifo full",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx ok",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx all",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx bad",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Octets Tx ok",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Octets Tx total",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx unicast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx multicast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx broadcast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx pause",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = NULL,
        },
    }
};

sdk_ret_t
register_metrics (void)
{
    return SDK_RET_ERR;
}
