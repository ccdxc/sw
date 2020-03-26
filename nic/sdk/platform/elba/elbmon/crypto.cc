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

const char *offloadname[9] = {"GCM0", "GCM1", "XTS", "XTS_ENC", "HE",
                              "CP",   "DC",   "MP",  "Master"};

int
read_num_entries (uint32_t base, uint32_t pi_addr, uint32_t ci_addr,
                  uint32_t ring_size_addr)
{
    uint32_t pi, ci, ring_size;
    pal_reg_rd32w(base + pi_addr, &pi, 1);
    pal_reg_rd32w(base + pi_addr, &ci, 1);
    pal_reg_rd32w(base + pi_addr, &ring_size, 1);
    if (pi >= ci)
        return (pi - ci);
    else
        return ((ring_size - ci) + pi);
}

void
crypto_read_queues (int verbose)
{
    uint32_t cnt;
    uint64_t aw, dw, wrsp, ar, dr, wrsp_err, rrsp_err;

    // XTS ENC
    cnt = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_XTS_ENC_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_XTS_ENC_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_XTS_ENC_RING_SIZE_BYTE_ADDRESS);
    if (cnt != 0)
        printf(" XTS ENC has %d ring entries\n", cnt);

    //. XTS
    cnt = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_XTS_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_XTS_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_XTS_RING_SIZE_BYTE_ADDRESS);
    if (cnt != 0)
        printf(" XTS has %d ring entries\n", cnt);

    // GCM0
    cnt = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GCM0_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GCM0_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GCM0_RING_SIZE_BYTE_ADDRESS);
    if (cnt != 0)
        printf(" GCM0 has %d ring entries\n", cnt);

    // GCM1
    cnt = read_num_entries(
        ELB_ADDR_BASE_MD_HENS_OFFSET,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GCM1_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GCM1_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_HENS_CSR_DHS_CRYPTO_CTL_GCM1_RING_SIZE_BYTE_ADDRESS);
    if (cnt != 0)
        printf(" GCM1 has %d ring entries\n", cnt);

    // CP hotq
    uint32_t pi, ci, cp_cfg;

    pal_reg_rd32w(
        ELB_ADDR_BASE_MD_HENS_OFFSET +
            ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_HOTQ_PD_IDX_BYTE_ADDRESS,
        &pi, 1);
    pal_reg_rd32w(
        ELB_ADDR_BASE_MD_HENS_OFFSET +
            ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_STA_HOTQ_CP_IDX_BYTE_ADDRESS,
        &ci, 1);
    pal_reg_rd32w(ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_DIST_BYTE_ADDRESS +
                      ELB_ADDR_BASE_MD_HENS_OFFSET,
                  &cp_cfg, 1);
    uint32_t hotq_size = (cp_cfg >> 6) & 0xfff;
    uint32_t descq_size = (cp_cfg >> 18) & 0xfff;
    if (pi >= ci)
        cnt = pi - ci;
    else
        cnt = (hotq_size - ci) + pi;
    if (cnt != 0)
        printf(" CP HOTQ has %d ring entries\n", cnt);

    // CP
    pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_Q_PD_IDX_BYTE_ADDRESS,
                  &pi, 1);
    pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_DHS_CRYPTO_CTL_CP_STA_Q_CP_IDX_BYTE_ADDRESS,
                  &ci, 1);
    if (pi >= ci)
        cnt = pi - ci;
    else
        cnt = (descq_size - ci) + pi;
    if (cnt != 0)
        printf(" CP has %d ring entries\n", cnt);

    // DC hotq
    pal_reg_rd32w(
        ELB_ADDR_BASE_MD_HENS_OFFSET +
            ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_HOTQ_PD_IDX_BYTE_ADDRESS,
        &pi, 1);
    pal_reg_rd32w(
        ELB_ADDR_BASE_MD_HENS_OFFSET +
            ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_STA_HOTQ_CP_IDX_BYTE_ADDRESS,
        &ci, 1);
    pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_DIST_BYTE_ADDRESS,
                  &cp_cfg, 1);
    hotq_size = (cp_cfg >> 6) & 0xfff;
    descq_size = (cp_cfg >> 18) & 0xfff;
    if (pi >= ci)
        cnt = pi - ci;
    else
        cnt = (hotq_size - ci) + pi;
    if (cnt != 0)
        printf(" DC HOTQ has %d ring entries\n", cnt);

    // DC
    pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_Q_PD_IDX_BYTE_ADDRESS,
                  &pi, 1);
    pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_DHS_CRYPTO_CTL_DC_STA_Q_CP_IDX_BYTE_ADDRESS,
                  &ci, 1);
    if (pi >= ci)
        cnt = pi - ci;
    else
        cnt = (descq_size - ci) + pi;
    if (cnt != 0)
        printf(" DC has %d ring entries\n", cnt);

    // MPP0
    cnt = read_num_entries(
        ELB_ADDR_BASE_MP_MPNS_OFFSET,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP0_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP0_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP0_RING_SIZE_BYTE_ADDRESS);
    if (cnt != 0)
        printf(" MPP0 has %d ring entries\n", cnt);

    // MPP1
    cnt = read_num_entries(
        ELB_ADDR_BASE_MP_MPNS_OFFSET,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP1_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP1_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP1_RING_SIZE_BYTE_ADDRESS);
    if (cnt != 0)
        printf(" MPP1 has %d ring entries\n", cnt);

    // MPP2
    cnt = read_num_entries(
        ELB_ADDR_BASE_MP_MPNS_OFFSET,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP2_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP2_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP2_RING_SIZE_BYTE_ADDRESS);
    if (cnt != 0)
        printf(" MPP2 has %d ring entries\n", cnt);

    // MPP3
    cnt = read_num_entries(
        ELB_ADDR_BASE_MP_MPNS_OFFSET,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP3_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP3_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP3_RING_SIZE_BYTE_ADDRESS);
    if (cnt != 0)
        printf(" MPP3 has %d ring entries\n", cnt);

    // MPP4
    cnt = read_num_entries(
        ELB_ADDR_BASE_MP_MPNS_OFFSET,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP4_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP4_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP4_RING_SIZE_BYTE_ADDRESS);
    if (cnt != 0)
        printf(" MPP4 has %d ring entries\n", cnt);

    // MPP5
    cnt = read_num_entries(
        ELB_ADDR_BASE_MP_MPNS_OFFSET,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP5_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP5_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP5_RING_SIZE_BYTE_ADDRESS);
    if (cnt != 0)
        printf("MPP5 has %d ring entries\n", cnt);

    // MPP6
    cnt = read_num_entries(
        ELB_ADDR_BASE_MP_MPNS_OFFSET,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP6_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP6_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP6_RING_SIZE_BYTE_ADDRESS);
    if (cnt != 0)
        printf(" MPP6 has %d ring entries\n", cnt);

    // MPP7
    cnt = read_num_entries(
        ELB_ADDR_BASE_MP_MPNS_OFFSET,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP7_PRODUCER_IDX_BYTE_ADDRESS,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP7_CONSUMER_IDX_BYTE_ADDRESS,
        ELB_MPNS_CSR_DHS_CRYPTO_CTL_MPP7_RING_SIZE_BYTE_ADDRESS);
    if (cnt != 0)
        printf(" MPP7 has %d ring entries\n", cnt);

    if (verbose) {
        uint32_t stride = (ELB_HENS_CSR_CNT_AXI_AW_GCM1_BYTE_ADDRESS -
                           ELB_HENS_CSR_CNT_AXI_AW_GCM0_BYTE_ADDRESS);
        for (int i = 0; i < 9; i++) {
            pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                              ELB_HENS_CSR_CNT_AXI_AW_GCM0_BYTE_ADDRESS +
                              (i * stride),
                          (uint32_t *)&aw, 1);
            pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                              ELB_HENS_CSR_CNT_AXI_DW_GCM0_BYTE_ADDRESS +
                              (i * stride),
                          (uint32_t *)&dw, 1);
            pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                              ELB_HENS_CSR_CNT_AXI_WRSP_GCM0_BYTE_ADDRESS +
                              (i * stride),
                          (uint32_t *)&wrsp, 1);
            pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                              ELB_HENS_CSR_CNT_AXI_AR_GCM0_BYTE_ADDRESS +
                              (i * stride),
                          (uint32_t *)&ar, 1);
            pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                              ELB_HENS_CSR_CNT_AXI_DR_GCM0_BYTE_ADDRESS +
                              (i * stride),
                          (uint32_t *)&dr, 1);
            pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                              ELB_HENS_CSR_CNT_AXI_WRSP_ERR_GCM0_BYTE_ADDRESS +
                              (i * stride),
                          (uint32_t *)&wrsp_err, 1);
            pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                              ELB_HENS_CSR_CNT_AXI_RRSP_ERR_GCM0_BYTE_ADDRESS +
                              (i * stride),
                          (uint32_t *)&rrsp_err, 1);
            printf(" offload=%s AW=%ld DW=%ld WRSP=%ld AR=%ld DR=%ld "
                   "WR_ERR=%ld RR_ERR=%ld\n",
                   offloadname[i], aw, dw, wrsp, ar, dr, wrsp_err, rrsp_err);
        }
    }
}

static inline void
elbmon_asic_crypto_store (uint64_t xts_cnt, uint64_t xtsenc_cnt,
                          uint64_t gcm0_cnt, uint64_t gcm1_cnt, uint64_t pk_cnt)
{
    asic->xts_cnt = xts_cnt;
    asic->xtsenc_cnt = xtsenc_cnt;
    asic->gcm0_cnt = gcm0_cnt;
    asic->gcm1_cnt = gcm1_cnt;
    asic->pk_cnt = pk_cnt;
}

void
crypto_read_counters (int verbose)
{
    uint64_t xts_cnt, xtsenc_cnt, gcm0_cnt, gcm1_cnt, pk_cnt;

    pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_CNT_DOORBELL_XTS_BYTE_ADDRESS,
                  (uint32_t *)&xts_cnt, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_CNT_DOORBELL_XTS_ENC_BYTE_ADDRESS,
                  (uint32_t *)&xtsenc_cnt, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_CNT_DOORBELL_GCM0_BYTE_ADDRESS,
                  (uint32_t *)&gcm0_cnt, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_CNT_DOORBELL_GCM1_BYTE_ADDRESS,
                  (uint32_t *)&gcm1_cnt, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_CNT_DOORBELL_PK_BYTE_ADDRESS,
                  (uint32_t *)&pk_cnt, 2);

    elbmon_asic_crypto_store(xts_cnt, xtsenc_cnt, gcm0_cnt, gcm1_cnt, pk_cnt);
}

void
crypto_reset_counters (int verbose)
{
    uint32_t zero[4] = {0};

    pal_reg_wr32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_CNT_DOORBELL_XTS_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_CNT_DOORBELL_XTS_ENC_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_CNT_DOORBELL_GCM0_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_CNT_DOORBELL_GCM1_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(ELB_ADDR_BASE_MD_HENS_OFFSET +
                      ELB_HENS_CSR_CNT_DOORBELL_PK_BYTE_ADDRESS,
                  zero, 2);
}
