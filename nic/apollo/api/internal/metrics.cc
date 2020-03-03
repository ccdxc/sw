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
#include "nic/apollo/api/internal/metrics.hpp"

namespace api {

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
            .name = "Frames Rx drop",
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
    .name = "MgmtPort",
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

sdk::metrics::metrics_schema_t host_if_metrics_schema = {
    .name = "HostIf",
    .counters = {
        {
            .name = "Octets Rx unicast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx unicast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Octets Rx multicast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx multicast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Octets Rx broadcast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx broadcast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "rsvd0",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd1",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "Octets Rx unicast drop",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx unicast drop",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Octets Rx multicast drop",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx multicast drop",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Octets Rx broadcast drop",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx broadcast drop",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx DMA error",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "rsvd2",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "Octets Tx unicast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx unicast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Octets Tx multicast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx multicast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Octets Tx broadcast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx broadcast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "rsvd3",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd4",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "Octets Tx unicast drop",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx unicast drop",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Octets Tx multicast drop",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx multicast drop",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Octets Tx broadcast drop",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx broadcast drop",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Tx DMA error",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "rsvd5",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "Frames Rx queue disable drops",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx queue empty drops",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx queue scheduling errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx descriptor fetch errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx descriptor data fetch errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "rsvd6",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd7",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd8",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "Frames Tx queue disabled drops",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Tx queue scheduling errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Tx descriptor fetch errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Tx descriptor data fetch errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "rsvd9",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd10",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd11",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd12",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "Octets Tx RDMA unicast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx RDMA unicast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Octets Tx RDMA multicast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx RDMA multicast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Tx RDMA CNP",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "rsvd13",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd14",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd15",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "Octets Rx RDMA unicast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx RDMA unicast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Octets Rx RDMA multicast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx RDMA multicast",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx RDMA CNP",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx RDMA ECN",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "rsvd16",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd17",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd18",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd19",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd20",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd21",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd22",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd23",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd24",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd25",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd26",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd27",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd28",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd29",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd30",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd31",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd32",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd33",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd34",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd35",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd36",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd37",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd38",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd39",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd40",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd41",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd42",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd43",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd44",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd45",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd46",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd47",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd48",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd49",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "Frames Rx RDMA request sequence errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx RDMA request RNR retry errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx RDMA request remote access errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx RDMA request remote invalid request errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx RDMA request remote operation errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx RDMA request implied NACK sequence errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx RDMA request CQE errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx RDMA request CQE flush errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx RDMA request duplicate responses",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Frames Rx RDMA request invalid",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Tx RDMA request local access errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Tx RDMA request local operation errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Tx RDMA request memory management errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "rsvd52",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd53",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd54",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "Rx RDMA response duplicate request errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx RDMA response out of buffer errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx RDMA response out of sequence errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx RDMA response CQE errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx RDMA response CQE flush errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx RDMA response local length errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx RDMA response invalid request errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx RDMA response local QP operation errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Rx RDMA response out of atomic resources errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Tx RDMA response packet sequence errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Tx RDMA response remote invalid request errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Tx RDMA response remote access errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Tx RDMA response remote operation errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "Tx RDMA response RNR retry errors",
            .type = sdk::metrics::METRICS_COUNTER_VALUE64,
        },
        {
            .name = "rsvd57",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = "rsvd58",
            .type = sdk::metrics::METRICS_COUNTER_RSVD64,
        },
        {
            .name = NULL,
        },
    }
};

sdk_ret_t
register_metrics (void)
{
    sdk::metrics::metrics_register(&host_if_metrics_schema);
    return SDK_RET_OK;
}

}    // namespace api
