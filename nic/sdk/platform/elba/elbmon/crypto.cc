//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the implementation for fetching and storing crypto info
/// elbmon -c
///
//===----------------------------------------------------------------------===//

#include <stdint.h>
#include <stdio.h>

#include "dtls.hpp"

#include "elb_hens_c_hdr.h"
#include "elb_mpns_c_hdr.h"
#include "elb_top_csr_defines.h"

#include "platform/pal/include/pal.h"
#include "elbmon.hpp"

const char *offloadname[15] = {"GCM_XTS_0", "GCM_XTS_1", "GCM_XTS_2", "GCM_XTS_3", "HE", "CP", "DC", "MP",
                               "HS", "CS", "EC", "GZ", "GU", "Master0", "Master1"};

int
read_num_entries (uint32_t base, uint32_t pi_addr, uint32_t ci_addr,
                  uint32_t ring_size_addr, int ring_size_shift,
		  int ring_size_mask)
{
    uint32_t pi, ci, ring_size;
    pal_reg_rd32w(base + pi_addr, &pi, 1);
    pal_reg_rd32w(base + pi_addr, &ci, 1);
    pal_reg_rd32w(base + pi_addr, &ring_size, 1);
    ring_size = ring_size >> ring_size_shift;
    ring_size = ring_size & ring_size_mask;
    if (pi >= ci)
        return (pi - ci);
    else
        return ((ring_size - ci) + pi);
}

void
crypto_read_queues (int verbose)
{
    uint32_t cnt[8];
    uint64_t aw, dw, wrsp, ar, dr, wrsp_err, rrsp_err;

    // GCM_XTS_0
    cnt[0] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM0_RING0_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM0_RING0_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM0_RING0_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[1] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM0_RING1_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM0_RING1_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM0_RING1_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[2] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM0_RING2_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM0_RING2_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM0_RING2_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[3] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM0_RING3_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM0_RING3_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM0_RING3_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[4] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM0_RING4_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM0_RING4_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM0_RING4_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[5] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM0_RING5_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM0_RING5_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM0_RING5_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[6] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM0_RING6_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM0_RING6_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM0_RING6_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[7] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM0_RING7_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM0_RING7_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM0_RING7_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    for (int i = 0; i < 8; i++) {
        if (cnt[i] != 0)
            printf(" GCM_XTS_0 ring %d has %d ring entries\n", i, cnt[i]);
    }

    // GCM_XTS_1
    cnt[0] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM1_RING0_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM1_RING0_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM1_RING0_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[1] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM1_RING1_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM1_RING1_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM1_RING1_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[2] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM1_RING2_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM1_RING2_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM1_RING2_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[3] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM1_RING3_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM1_RING3_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM1_RING3_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[4] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM1_RING4_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM1_RING4_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM1_RING4_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[5] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM1_RING5_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM1_RING5_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM1_RING5_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[6] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM1_RING6_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM1_RING6_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM1_RING6_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[7] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM1_RING7_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM1_RING7_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM1_RING7_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    for (int i = 0; i < 8; i++) {
        if (cnt[i] != 0)
            printf(" GCM_XTS_1 ring %d has %d ring entries\n", i, cnt[i]);
    }

    // GCM_XTS_2
    cnt[0] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM2_RING0_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM2_RING0_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM2_RING0_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[1] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM2_RING1_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM2_RING1_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM2_RING1_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[2] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM2_RING2_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM2_RING2_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM2_RING2_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[3] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM2_RING3_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM2_RING3_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM2_RING3_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[4] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM2_RING4_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM2_RING4_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM2_RING4_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[5] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM2_RING5_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM2_RING5_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM2_RING5_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[6] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM2_RING6_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM2_RING6_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM2_RING6_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[7] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM2_RING7_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM2_RING7_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM2_RING7_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    for (int i = 0; i < 8; i++) {
        if (cnt[i] != 0)
            printf(" GCM_XTS_2 ring %d has %d ring entries\n", i, cnt[i]);
    }

    // GCM_XTS_3
    cnt[0] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM3_RING0_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM3_RING0_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM3_RING0_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[1] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM3_RING1_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM3_RING1_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM3_RING1_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[2] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM3_RING2_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM3_RING2_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM3_RING2_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[3] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM3_RING3_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM3_RING3_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM3_RING3_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[4] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM3_RING4_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM3_RING4_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM3_RING4_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[5] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM3_RING5_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM3_RING5_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM3_RING5_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[6] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM3_RING6_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM3_RING6_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM3_RING6_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    cnt[7] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM3_RING7_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM3_RING7_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_SYM3_RING7_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    for (int i = 0; i < 8; i++) {
        if (cnt[i] != 0)
            printf(" GCM_XTS_3 ring %d has %d ring entries\n", i, cnt[i]);
    }

    // CP
    cnt[0] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_Q0_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_STA_Q0_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_DIST_QSIZE0_W0_BYTE_ADDRESS, 0, 0xfff);
    cnt[1] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_Q1_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_STA_Q1_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_DIST_QSIZE0_W0_BYTE_ADDRESS, 12, 0xfff);
    cnt[2] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_Q2_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_STA_Q2_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_DIST_QSIZE0_W1_BYTE_ADDRESS, 0, 0xfff);
    cnt[3] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_Q3_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_STA_Q3_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_DIST_QSIZE0_W1_BYTE_ADDRESS, 12, 0xfff);
    cnt[4] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_Q4_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_STA_Q4_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_DIST_QSIZE1_W0_BYTE_ADDRESS, 0, 0xfff);
    cnt[5] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_Q5_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_STA_Q5_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_DIST_QSIZE1_W0_BYTE_ADDRESS, 12, 0xfff);
    cnt[6] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_Q6_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_STA_Q6_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_DIST_QSIZE1_W1_BYTE_ADDRESS, 0, 0xfff);
    cnt[7] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_Q7_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_STA_Q7_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_DIST_QSIZE1_W1_BYTE_ADDRESS, 12, 0xfff);
    for (int i = 0; i < 8; i++) {
        if (cnt[i] != 0)
            printf(" CP ring %d has %d ring entries\n", i, cnt[i]);
    }

    // DC
    cnt[0] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_Q0_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_STA_Q0_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_DIST_QSIZE0_W0_BYTE_ADDRESS, 0, 0xfff);
    cnt[1] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_Q1_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_STA_Q1_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_DIST_QSIZE0_W0_BYTE_ADDRESS, 12, 0xfff);
    cnt[2] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_Q2_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_STA_Q2_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_DIST_QSIZE0_W1_BYTE_ADDRESS, 0, 0xfff);
    cnt[3] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_Q3_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_STA_Q3_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_DIST_QSIZE0_W1_BYTE_ADDRESS, 12, 0xfff);
    cnt[4] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_Q4_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_STA_Q4_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_DIST_QSIZE1_W0_BYTE_ADDRESS, 0, 0xfff);
    cnt[5] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_Q5_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_STA_Q5_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_DIST_QSIZE1_W0_BYTE_ADDRESS, 12, 0xfff);
    cnt[6] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_Q6_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_STA_Q6_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_DIST_QSIZE1_W1_BYTE_ADDRESS, 0, 0xfff);
    cnt[7] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_Q7_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_STA_Q7_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_DIST_QSIZE1_W1_BYTE_ADDRESS, 12, 0xfff);
    for (int i = 0; i < 8; i++) {
        if (cnt[i] != 0)
            printf(" DC ring %d has %d ring entries\n", i, cnt[i]);
    }

    // CS
    cnt[0] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CS_CFG_Q0_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CS_STA_Q0_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CS_CFG_DIST_QSIZE0_W0_BYTE_ADDRESS, 0, 0xfff);
    cnt[1] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CS_CFG_Q1_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CS_STA_Q1_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CS_CFG_DIST_QSIZE0_W0_BYTE_ADDRESS, 12, 0xfff);
    cnt[2] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CS_CFG_Q2_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CS_STA_Q2_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CS_CFG_DIST_QSIZE0_W1_BYTE_ADDRESS, 0, 0xfff);
    cnt[3] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CS_CFG_Q3_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CS_STA_Q3_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CS_CFG_DIST_QSIZE0_W1_BYTE_ADDRESS, 12, 0xfff);
    cnt[4] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CS_CFG_Q4_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CS_STA_Q4_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CS_CFG_DIST_QSIZE1_W0_BYTE_ADDRESS, 0, 0xfff);
    cnt[5] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CS_CFG_Q5_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CS_STA_Q5_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CS_CFG_DIST_QSIZE1_W0_BYTE_ADDRESS, 12, 0xfff);
    cnt[6] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CS_CFG_Q6_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CS_STA_Q6_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CS_CFG_DIST_QSIZE1_W1_BYTE_ADDRESS, 0, 0xfff);
    cnt[7] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CS_CFG_Q7_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CS_STA_Q7_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_CS_CFG_DIST_QSIZE1_W1_BYTE_ADDRESS, 12, 0xfff);
    for (int i = 0; i < 8; i++) {
        if (cnt[i] != 0)
            printf(" CS ring %d has %d ring entries\n", i, cnt[i]);
    }

    // HS
    cnt[0] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_HS_CFG_Q0_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_HS_STA_Q0_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_HS_CFG_DIST_QSIZE0_W0_BYTE_ADDRESS, 0, 0xfff);
    cnt[1] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_HS_CFG_Q1_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_HS_STA_Q1_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_HS_CFG_DIST_QSIZE0_W0_BYTE_ADDRESS, 12, 0xfff);
    cnt[2] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_HS_CFG_Q2_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_HS_STA_Q2_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_HS_CFG_DIST_QSIZE0_W1_BYTE_ADDRESS, 0, 0xfff);
    cnt[3] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_HS_CFG_Q3_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_HS_STA_Q3_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_HS_CFG_DIST_QSIZE0_W1_BYTE_ADDRESS, 12, 0xfff);
    cnt[4] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_HS_CFG_Q4_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_HS_STA_Q4_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_HS_CFG_DIST_QSIZE1_W0_BYTE_ADDRESS, 0, 0xfff);
    cnt[5] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_HS_CFG_Q5_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_HS_STA_Q5_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_HS_CFG_DIST_QSIZE1_W0_BYTE_ADDRESS, 12, 0xfff);
    cnt[6] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_HS_CFG_Q6_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_HS_STA_Q6_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_HS_CFG_DIST_QSIZE1_W1_BYTE_ADDRESS, 0, 0xfff);
    cnt[7] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_HS_CFG_Q7_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_HS_STA_Q7_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_HS_CFG_DIST_QSIZE1_W1_BYTE_ADDRESS, 12, 0xfff);
    for (int i = 0; i < 8; i++) {
        if (cnt[i] != 0)
            printf(" HS ring %d has %d ring entries\n", i, cnt[i]);
    }

    // EC
    cnt[0] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_EC_CFG_Q0_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_EC_STA_Q0_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_EC_CFG_DIST_QSIZE0_W0_BYTE_ADDRESS, 0, 0xfff);
    cnt[1] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_EC_CFG_Q1_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_EC_STA_Q1_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_EC_CFG_DIST_QSIZE0_W0_BYTE_ADDRESS, 12, 0xfff);
    cnt[2] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_EC_CFG_Q2_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_EC_STA_Q2_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_EC_CFG_DIST_QSIZE0_W1_BYTE_ADDRESS, 0, 0xfff);
    cnt[3] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_EC_CFG_Q3_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_EC_STA_Q3_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_EC_CFG_DIST_QSIZE0_W1_BYTE_ADDRESS, 12, 0xfff);
    cnt[4] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_EC_CFG_Q4_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_EC_STA_Q4_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_EC_CFG_DIST_QSIZE1_W0_BYTE_ADDRESS, 0, 0xfff);
    cnt[5] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_EC_CFG_Q5_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_EC_STA_Q5_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_EC_CFG_DIST_QSIZE1_W0_BYTE_ADDRESS, 12, 0xfff);
    cnt[6] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_EC_CFG_Q6_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_EC_STA_Q6_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_EC_CFG_DIST_QSIZE1_W1_BYTE_ADDRESS, 0, 0xfff);
    cnt[7] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_EC_CFG_Q7_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_EC_STA_Q7_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_EC_CFG_DIST_QSIZE1_W1_BYTE_ADDRESS, 12, 0xfff);
    for (int i = 0; i < 8; i++) {
        if (cnt[i] != 0)
            printf(" EC ring %d has %d ring entries\n", i, cnt[i]);
    }

    // GZ
    cnt[0] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GZ_CFG_Q0_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GZ_STA_Q0_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GZ_CFG_DIST_QSIZE0_W0_BYTE_ADDRESS, 0, 0xfff);
    cnt[1] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GZ_CFG_Q1_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GZ_STA_Q1_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GZ_CFG_DIST_QSIZE0_W0_BYTE_ADDRESS, 12, 0xfff);
    cnt[2] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GZ_CFG_Q2_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GZ_STA_Q2_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GZ_CFG_DIST_QSIZE0_W1_BYTE_ADDRESS, 0, 0xfff);
    cnt[3] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GZ_CFG_Q3_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GZ_STA_Q3_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GZ_CFG_DIST_QSIZE0_W1_BYTE_ADDRESS, 12, 0xfff);
    cnt[4] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GZ_CFG_Q4_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GZ_STA_Q4_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GZ_CFG_DIST_QSIZE1_W0_BYTE_ADDRESS, 0, 0xfff);
    cnt[5] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GZ_CFG_Q5_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GZ_STA_Q5_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GZ_CFG_DIST_QSIZE1_W0_BYTE_ADDRESS, 12, 0xfff);
    cnt[6] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GZ_CFG_Q6_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GZ_STA_Q6_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GZ_CFG_DIST_QSIZE1_W1_BYTE_ADDRESS, 0, 0xfff);
    cnt[7] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GZ_CFG_Q7_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GZ_STA_Q7_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GZ_CFG_DIST_QSIZE1_W1_BYTE_ADDRESS, 12, 0xfff);
    for (int i = 0; i < 8; i++) {
        if (cnt[i] != 0)
            printf(" GZ ring %d has %d ring entries\n", i, cnt[i]);
    }

    // GU
    cnt[0] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GU_CFG_Q0_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GU_STA_Q0_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GU_CFG_DIST_QSIZE0_W0_BYTE_ADDRESS, 0, 0xfff);
    cnt[1] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GU_CFG_Q1_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GU_STA_Q1_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GU_CFG_DIST_QSIZE0_W0_BYTE_ADDRESS, 12, 0xfff);
    cnt[2] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GU_CFG_Q2_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GU_STA_Q2_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GU_CFG_DIST_QSIZE0_W1_BYTE_ADDRESS, 0, 0xfff);
    cnt[3] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GU_CFG_Q3_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GU_STA_Q3_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GU_CFG_DIST_QSIZE0_W1_BYTE_ADDRESS, 12, 0xfff);
    cnt[4] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GU_CFG_Q4_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GU_STA_Q4_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GU_CFG_DIST_QSIZE1_W0_BYTE_ADDRESS, 0, 0xfff);
    cnt[5] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GU_CFG_Q5_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GU_STA_Q5_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GU_CFG_DIST_QSIZE1_W0_BYTE_ADDRESS, 12, 0xfff);
    cnt[6] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GU_CFG_Q6_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GU_STA_Q6_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GU_CFG_DIST_QSIZE1_W1_BYTE_ADDRESS, 0, 0xfff);
    cnt[7] = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GU_CFG_Q7_PD_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GU_STA_Q7_CP_IDX_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GU_CFG_DIST_QSIZE1_W1_BYTE_ADDRESS, 12, 0xfff);
    for (int i = 0; i < 8; i++) {
        if (cnt[i] != 0)
            printf(" GU ring %d has %d ring entries\n", i, cnt[i]);
    }

    // MPP0
    cnt[0] = read_num_entries(
        ELB_ADDR_BASE_MP_MPNS_OFFSET,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP0_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP0_CONSUMER_IDX_BYTE_ADDRESS,
        (uint32_t)ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP0_RING_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    if (cnt[0] != 0)
        printf(" MPP0 has %d ring entries\n", cnt[0]);

    // MPP1
    cnt[0] = read_num_entries(
        ELB_ADDR_BASE_MP_MPNS_OFFSET,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP1_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP1_CONSUMER_IDX_BYTE_ADDRESS,
        (uint32_t)ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP1_RING_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    if (cnt[0] != 0)
        printf(" MPP1 has %d ring entries\n", cnt[0]);

    // MPP2
    cnt[0] = read_num_entries(
        ELB_ADDR_BASE_MP_MPNS_OFFSET,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP2_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP2_CONSUMER_IDX_BYTE_ADDRESS,
        (uint32_t)ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP2_RING_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    if (cnt[0] != 0)
        printf(" MPP2 has %d ring entries\n", cnt[0]);

    // MPP3
    cnt[0] = read_num_entries(
        ELB_ADDR_BASE_MP_MPNS_OFFSET,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP3_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP3_CONSUMER_IDX_BYTE_ADDRESS,
        (uint32_t)ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP3_RING_SIZE_BYTE_ADDRESS, 0, 0xffffffff);
    if (cnt[0] != 0)
        printf(" MPP3 has %d ring entries\n", cnt[0]);

    if (verbose) {
        uint32_t stride = (ELB_HENS_CSR_CNT_AXI_AW_SYM1_BYTE_ADDRESS -
                           ELB_HENS_CSR_CNT_AXI_AW_SYM0_BYTE_ADDRESS);
        for (int i = 0; i < 15; i++) {
            pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                              ELB_HENS_CSR_CNT_AXI_AW_SYM0_BYTE_ADDRESS +
                              (i * stride),
                          (uint32_t *)&aw, 1);
            pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                              ELB_HENS_CSR_CNT_AXI_DW_SYM0_BYTE_ADDRESS +
                              (i * stride),
                          (uint32_t *)&dw, 1);
            pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                              ELB_HENS_CSR_CNT_AXI_WRSP_SYM0_BYTE_ADDRESS +
                              (i * stride),
                          (uint32_t *)&wrsp, 1);
            pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                              ELB_HENS_CSR_CNT_AXI_AR_SYM0_BYTE_ADDRESS +
                              (i * stride),
                          (uint32_t *)&ar, 1);
            pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                              ELB_HENS_CSR_CNT_AXI_DR_SYM0_BYTE_ADDRESS +
                              (i * stride),
                          (uint32_t *)&dr, 1);
            pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                              ELB_HENS_CSR_CNT_AXI_WRSP_ERR_SYM0_BYTE_ADDRESS +
                              (i * stride),
                          (uint32_t *)&wrsp_err, 1);
            pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                              ELB_HENS_CSR_CNT_AXI_RRSP_ERR_SYM0_BYTE_ADDRESS +
                              (i * stride),
                          (uint32_t *)&rrsp_err, 1);
            printf(" offload=%s AW=%ld DW=%ld WRSP=%ld AR=%ld DR=%ld "
                   "WR_ERR=%ld RR_ERR=%ld\n",
                   offloadname[i], aw, dw, wrsp, ar, dr, wrsp_err, rrsp_err);
        }
    }
}

static inline void
elbmon_asic_crypto_store (uint64_t cnt[])
{
    asic->gcm_xts0_cnt = cnt[0];
    asic->gcm_xts1_cnt = cnt[1];
    asic->gcm_xts2_cnt = cnt[2];
    asic->gcm_xts3_cnt = cnt[3];
    asic->pk0_cnt = cnt[4];
    asic->pk1_cnt = cnt[5];
    asic->mpp0_cnt = cnt[6];
    asic->mpp1_cnt = cnt[7];
    asic->mpp2_cnt = cnt[8];
    asic->mpp3_cnt = cnt[9];
}

void
crypto_read_counters (int verbose)
{
    uint64_t cnt[10];

    pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_CNT_DOORBELL_SYM0_BYTE_ADDRESS,
                  (uint32_t *)&cnt[0], 2);

    pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_CNT_DOORBELL_SYM1_BYTE_ADDRESS,
                  (uint32_t *)&cnt[1], 2);

    pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_CNT_DOORBELL_SYM2_BYTE_ADDRESS,
                  (uint32_t *)&cnt[2], 2);

    pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_CNT_DOORBELL_SYM3_BYTE_ADDRESS,
                  (uint32_t *)&cnt[3], 2);

    pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_CNT_DOORBELL_PK0_BYTE_ADDRESS,
                  (uint32_t *)&cnt[4], 2);

    pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_CNT_DOORBELL_PK1_BYTE_ADDRESS,
                  (uint32_t *)&cnt[5], 2);

    pal_reg_rd32w(ELB_ADDR_BASE_MP_MPNS_OFFSET +
                      ELB_MPNS_CSR_CNT_DOORBELL_MPP0_BYTE_ADDRESS,
                  (uint32_t *)&cnt[6], 2);

    pal_reg_rd32w(ELB_ADDR_BASE_MP_MPNS_OFFSET +
                      ELB_MPNS_CSR_CNT_DOORBELL_MPP1_BYTE_ADDRESS,
                  (uint32_t *)&cnt[7], 2);

    pal_reg_rd32w(ELB_ADDR_BASE_MP_MPNS_OFFSET +
                      ELB_MPNS_CSR_CNT_DOORBELL_MPP2_BYTE_ADDRESS,
                  (uint32_t *)&cnt[8], 2);

    pal_reg_rd32w(ELB_ADDR_BASE_MP_MPNS_OFFSET +
                      ELB_MPNS_CSR_CNT_DOORBELL_MPP3_BYTE_ADDRESS,
                  (uint32_t *)&cnt[9], 2);

    elbmon_asic_crypto_store(cnt);
}

void
crypto_reset_counters (int verbose)
{
    uint32_t zero[4] = {0};

    pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_CNT_DOORBELL_SYM0_BYTE_ADDRESS,
                  zero, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_CNT_DOORBELL_SYM1_BYTE_ADDRESS,
                  zero, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_CNT_DOORBELL_SYM2_BYTE_ADDRESS,
                  zero, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_CNT_DOORBELL_SYM3_BYTE_ADDRESS,
                  zero, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_CNT_DOORBELL_PK0_BYTE_ADDRESS,
                  zero, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_CNT_DOORBELL_PK1_BYTE_ADDRESS,
                  zero, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_MP_MPNS_OFFSET +
                      ELB_MPNS_CSR_CNT_DOORBELL_MPP0_BYTE_ADDRESS,
                  zero, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_MP_MPNS_OFFSET +
                      ELB_MPNS_CSR_CNT_DOORBELL_MPP1_BYTE_ADDRESS,
                  zero, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_MP_MPNS_OFFSET +
                      ELB_MPNS_CSR_CNT_DOORBELL_MPP2_BYTE_ADDRESS,
                  zero, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_MP_MPNS_OFFSET +
                      ELB_MPNS_CSR_CNT_DOORBELL_MPP3_BYTE_ADDRESS,
                  zero, 2);

}
