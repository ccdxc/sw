/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <sys/types.h>

#include "cap_top_csr_defines.h"
#include "cap_intr_c_hdr.h"
#include "cap_wa_c_hdr.h"
#include "utils_impl.h"

#define INTR_BASE               CAP_ADDR_BASE_INTR_INTR_OFFSET

#define INTR_MSIXCFG_OFFSET     CAP_INTR_CSR_DHS_INTR_MSIXCFG_BYTE_OFFSET
#define INTR_MSIXCFG_BASE       (INTR_BASE + INTR_MSIXCFG_OFFSET)
#define INTR_MSIXCFG_STRIDE     0x10

#define INTR_FWCFG_OFFSET       CAP_INTR_CSR_DHS_INTR_FWCFG_BYTE_OFFSET
#define INTR_FWCFG_BASE         (INTR_BASE + INTR_FWCFG_OFFSET)
#define INTR_FWCFG_STRIDE       0x8

#define INTR_DRVCFG_OFFSET      CAP_INTR_CSR_DHS_INTR_DRVCFG_BYTE_OFFSET
#define INTR_DRVCFG_BASE        (INTR_BASE + INTR_DRVCFG_OFFSET)
#define INTR_DRVCFG_STRIDE      0x20

#define INTR_PBA_CFG_OFFSET     CAP_INTR_CSR_DHS_INTR_PBA_CFG_BYTE_OFFSET
#define INTR_PBA_CFG_BASE       (INTR_BASE + INTR_PBA_CFG_OFFSET)
#define INTR_PBA_CFG_STRIDE     0x4

#define INTR_PBA_OFFSET         CAP_INTR_CSR_DHS_INTR_PBA_ARRAY_BYTE_OFFSET
#define INTR_PBA_BASE           (INTR_BASE + INTR_PBA_OFFSET)
#define INTR_PBA_STRIDE         0x8

#define INTR_ASSERT_OFFSET      CAP_INTR_CSR_DHS_INTR_ASSERT_BYTE_OFFSET
#define INTR_ASSERT_BASE        (INTR_BASE + INTR_ASSERT_OFFSET)
#define INTR_ASSERT_STRIDE      0x4

#define INTR_COALESCE_OFFSET    CAP_INTR_CSR_DHS_INTR_COALESCE_BYTE_OFFSET
#define INTR_COALESCE_BASE      (INTR_BASE + INTR_COALESCE_OFFSET)
#define INTR_COALESCE_STRIDE    0x8

#define INTR_STATE_OFFSET       CAP_INTR_CSR_DHS_INTR_STATE_BYTE_OFFSET
#define INTR_STATE_BASE         (INTR_BASE + INTR_STATE_OFFSET)
#define INTR_STATE_STRIDE       0x10

u_int64_t
intr_msixcfg_addr(const int intr)
{
    return INTR_MSIXCFG_BASE + (intr * INTR_MSIXCFG_STRIDE);
}

u_int64_t
intr_fwcfg_addr(const int intr)
{
    return INTR_FWCFG_BASE + (intr * INTR_FWCFG_STRIDE);
}

u_int64_t
intr_drvcfg_addr(const int intr)
{
    return INTR_DRVCFG_BASE + (intr * INTR_DRVCFG_STRIDE);
}

u_int64_t
intr_assert_addr(const int intr)
{
    return INTR_ASSERT_BASE + (intr * INTR_ASSERT_STRIDE);
}

u_int64_t
intr_pba_addr(const int lif)
{
    return INTR_PBA_BASE + (lif * INTR_PBA_STRIDE);
}

u_int64_t
intr_pba_cfg_addr(const int lif)
{
    return INTR_PBA_CFG_BASE + (lif * INTR_PBA_CFG_STRIDE);
}

u_int64_t
db_host_addr(const int lif)
{
#define DB_BASE         CAP_ADDR_BASE_DB_WA_OFFSET
#define DB_HOST_BASE    (DB_BASE + CAP_WA_CSR_DHS_HOST_DOORBELL_BYTE_OFFSET)
#define DB_HOST_STRIDE  0x40

    return DB_HOST_BASE + (lif * DB_HOST_STRIDE);
}
