/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */
#include <stdint.h>
#include <stdio.h>

#include "dtls.hpp"

#include "cap_ppa_c_hdr.h"
#include "cap_top_csr_defines.h"

#include "platform/src/lib/pal/include/pal.h"

void
parser_read_counters(int verbose, uint32_t base_addr)
{
    uint32_t cnt[4];
    uint64_t from_pb, to_dp, to_ma;

    pal_reg_rd32w(base_addr + CAP_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS, cnt, 4);
    from_pb = ((uint64_t)CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_0_4_SOP_31_0_GET(cnt[0]) +
               ((uint64_t)CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_1_4_SOP_39_32_GET(cnt[1]) << 32));

    pal_reg_rd32w(base_addr + CAP_PPA_CSR_CNT_PPA_DP_BYTE_ADDRESS, cnt, 4);
    to_dp = ((uint64_t)CAP_PPA_CSR_CNT_PPA_DP_CNT_PPA_DP_0_4_SOP_31_0_GET(cnt[0]) +
             ((uint64_t)CAP_PPA_CSR_CNT_PPA_DP_CNT_PPA_DP_1_4_SOP_39_32_GET(cnt[1]) << 32));

    pal_reg_rd32w(base_addr + CAP_PPA_CSR_CNT_PPA_MA_BYTE_ADDRESS, cnt, 4);
    to_ma = ((uint64_t)CAP_PPA_CSR_CNT_PPA_MA_CNT_PPA_MA_0_4_SOP_31_0_GET(cnt[0]) +
             ((uint64_t)CAP_PPA_CSR_CNT_PPA_MA_CNT_PPA_MA_1_4_SOP_39_32_GET(cnt[1]) << 32));

    printf(" Parser: pkt_from_pb=%ld phv_to_s0=%ld pkt_to_dp=%ld\n", from_pb, to_ma, to_dp);
}

void
parser_reset_counters(int verbose, uint32_t base_addr)
{
    uint32_t zero[4] = {0};

    pal_reg_wr32w(base_addr + CAP_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS, zero, 4);
    pal_reg_wr32w(base_addr + CAP_PPA_CSR_CNT_PPA_DP_BYTE_ADDRESS, zero, 4);
    pal_reg_wr32w(base_addr + CAP_PPA_CSR_CNT_PPA_MA_BYTE_ADDRESS, zero, 4);
}
