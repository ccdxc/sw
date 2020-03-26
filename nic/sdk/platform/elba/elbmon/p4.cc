//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the implementation for storing ingress/egress
/// pipeline counters
///
//===----------------------------------------------------------------------===//

#include <stdint.h>
#include <stdio.h>

#include "dtls.hpp"

#include "elb_ppa_c_hdr.h"
#include "elb_top_csr_defines.h"

#include "platform/pal/include/pal.h"
#include "elbmon.hpp"

void
parser_read_counters (int type, int verbose, uint32_t base_addr)
{
    uint32_t cnt[4];
    uint64_t pkt_from_pb, phv_to_s0, pkt_to_dp;
    pipeline_t *pipeline = &asic->pipelines[type];

    pal_reg_rd32w(base_addr + ELB_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS, cnt, 4);
    pkt_from_pb =
        ((uint64_t)ELB_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_0_4_SOP_31_0_GET(cnt[0]) +
         ((uint64_t)ELB_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_1_4_SOP_39_32_GET(cnt[1])
          << 32));

    pal_reg_rd32w(base_addr + ELB_PPA_CSR_CNT_PPA_DP_BYTE_ADDRESS, cnt, 4);
    pkt_to_dp =
        ((uint64_t)ELB_PPA_CSR_CNT_PPA_DP_CNT_PPA_DP_0_4_SOP_31_0_GET(cnt[0]) +
         ((uint64_t)ELB_PPA_CSR_CNT_PPA_DP_CNT_PPA_DP_1_4_SOP_39_32_GET(cnt[1])
          << 32));

    pal_reg_rd32w(base_addr + ELB_PPA_CSR_CNT_PPA_MA_BYTE_ADDRESS, cnt, 4);
    phv_to_s0 =
        ((uint64_t)ELB_PPA_CSR_CNT_PPA_MA_CNT_PPA_MA_0_4_SOP_31_0_GET(cnt[0]) +
         ((uint64_t)ELB_PPA_CSR_CNT_PPA_MA_CNT_PPA_MA_1_4_SOP_39_32_GET(cnt[1])
          << 32));

    pipeline->pkt_from_pb = pkt_from_pb;
    pipeline->phv_to_s0 = phv_to_s0;
    pipeline->pkt_to_dp = pkt_to_dp;
}

void
parser_reset_counters (int verbose, uint32_t base_addr)
{
    uint32_t zero[4] = {0};

    pal_reg_wr32w(base_addr + ELB_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS, zero, 4);
    pal_reg_wr32w(base_addr + ELB_PPA_CSR_CNT_PPA_DP_BYTE_ADDRESS, zero, 4);
    pal_reg_wr32w(base_addr + ELB_PPA_CSR_CNT_PPA_MA_BYTE_ADDRESS, zero, 4);
}
