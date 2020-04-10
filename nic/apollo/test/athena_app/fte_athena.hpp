//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains external interfaces of FTE module
///
//----------------------------------------------------------------------------

#ifndef __FTE_FTE_HPP__
#define __FTE_FTE_HPP__

#include "nic/sdk/third-party/zmq/include/zmq.h"
#include "nic/apollo/api/include/athena/pds_base.h"

namespace fte_ath {

void fte_init(void);
void fte_fini(void);

sdk_ret_t fte_flow_prog(struct rte_mbuf *m);
void fte_thread_init(unsigned int core_id);
sdk_ret_t fte_flows_init(void);
pds_ret_t fte_dump_flows(zmq_msg_t *rx_msg = nullptr,
                         zmq_msg_t *tx_msg = nullptr);
pds_ret_t fte_dump_flow_stats(zmq_msg_t *rx_msg = nullptr,
                              zmq_msg_t *tx_msg = nullptr);

#define MAX_LINE_SZ 128
static inline void pkt_hex_dump_trace(const char *label, char *buf, uint16_t len)
{
    char            line[MAX_LINE_SZ];
    char            *lineptr;
    uint16_t        idx = 0;
    uint16_t        lineoffset = 0;

    lineptr = &line[0];
    PDS_TRACE_DEBUG("%s:", label);
    for (idx = 0; idx < len; idx++) {

        lineoffset += snprintf(lineptr + lineoffset, (MAX_LINE_SZ - lineoffset - 1),
                "%02hhx ", buf[idx]);

        if (((idx + 1) % 16) == 0) {
            PDS_TRACE_DEBUG("%s", line);
            lineoffset = 0;
        }
    }
    if (lineoffset) {
        PDS_TRACE_DEBUG("%s", line);
    }
}

#define ATHENA_APP_MODE_CPP         0
#define ATHENA_APP_MODE_L2_FWD      1
#define ATHENA_APP_MODE_NO_DPDK     2

extern uint8_t g_athena_app_mode;

}    // namespace fte

#endif    // __FTE_FTE_HPP__

