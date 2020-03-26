//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines ASIC Doorbell API's
///
//----------------------------------------------------------------------------

#include "asic/asic.hpp"
#include "asic/pd/db.hpp"
#include "asic/pd/pd.hpp"

#include "lib/pal/pal.hpp"
#include "platform/pal/include/pal.h"


namespace sdk {
namespace asic {
namespace pd {

static uint64_t g_db_addr_local_csr;
static uint64_t g_db32_addr_local_csr;
static uint64_t g_db_addr_host_csr;


#ifdef __x86_64__
#define ASIC_PAL_DB_WRITE(addr, data, bits)     \
    sdk::lib::pal_ring_db ## bits (addr, data);
#else
#define ASIC_PAL_DB_WRITE(addr, data, bits)     \
    pal_reg_wr ## bits (addr, data);
#endif

#define ASIC_DB_UPD_SHIFT                     17
#define ASIC_DB_LIF_SHIFT                     6
#define ASIC_DB_TYPE_SHIFT                    3

#define ASIC_HOST_DBADDR_GET(lif, qtype, upd)        \
    (((uint64_t)(lif) << ASIC_DB_LIF_SHIFT) |        \
    ((uint64_t)(qtype) << ASIC_DB_TYPE_SHIFT) |      \
    ((uint64_t)(upd) << ASIC_DB_UPD_SHIFT)) |        \
    ((uint64_t) sdk::asic::pd::g_db_addr_host_csr)

#define ASIC_LOCAL_DBADDR_GET(lif, qtype, upd)       \
    (((uint64_t)(lif) << ASIC_DB_LIF_SHIFT) |        \
    ((uint64_t)(qtype) << ASIC_DB_TYPE_SHIFT) |      \
    ((uint64_t)(upd) << ASIC_DB_UPD_SHIFT)) |        \
    ((uint64_t) sdk::asic::pd::g_db_addr_local_csr)

// 32b doorbell addresses are 32-bit aligned rather than 64-bit aligned.
#define ASIC_LOCAL_DB32_ADDR_GET(lif, qtype, upd)    \
    (((uint64_t)(lif) << (ASIC_DB_LIF_SHIFT - 1)) |  \
    ((uint64_t)(qtype) << (ASIC_DB_TYPE_SHIFT - 1)) |\
    ((uint64_t)(upd) << (ASIC_DB_UPD_SHIFT - 1))) |  \
    ((uint64_t) sdk::asic::pd::g_db32_addr_local_csr)

// host doorbell address for lif
uint64_t
asic_hostdb_addr (uint16_t lif_id, uint8_t q_type, uint8_t upd)
{
    return ASIC_HOST_DBADDR_GET(lif_id, q_type, upd);
}

// local doorbell address for lif
uint64_t
asic_localdb_addr (uint16_t lif_id, uint8_t q_type, uint8_t upd)
{
    return ASIC_LOCAL_DBADDR_GET(lif_id, q_type, upd);
}

uint64_t
asic_localdb32_addr (uint16_t lif_id, uint8_t q_type, uint8_t upd)
{
    if (unlikely(g_db32_addr_local_csr == 0)) {
        g_db32_addr_local_csr = sdk::asic::pd::asicpd_local_db32_addr_get();
    }
    return ASIC_LOCAL_DB32_ADDR_GET(lif_id, q_type, upd);
}

// host doorbell64 api
void
asic_ring_hostdb (asic_db_addr_t *db_addr, uint64_t data)
{
    uint64_t addr;

    if (unlikely(g_db_addr_host_csr == 0)) {
        g_db_addr_host_csr = sdk::asic::pd::asicpd_host_dbaddr_get();
    }
    SDK_ASSERT(db_addr);
    addr = ASIC_HOST_DBADDR_GET(db_addr->lif_id, db_addr->q_type, db_addr->upd);
    ASIC_PAL_DB_WRITE(addr, data, 64);
}

// doorbell64 api
void
asic_ring_db (asic_db_addr_t *db_addr, uint64_t data)
{
    uint64_t addr;

    if (unlikely(g_db_addr_local_csr == 0)) {
        g_db_addr_local_csr = sdk::asic::pd::asicpd_local_dbaddr_get();
    }
    SDK_ASSERT(db_addr);
    addr = ASIC_LOCAL_DBADDR_GET(db_addr->lif_id, db_addr->q_type, db_addr->upd);
    ASIC_PAL_DB_WRITE(addr, data, 64);
}

// doorbell32 api
void
asic_ring_db (asic_db_addr_t *db_addr, uint32_t data)
{
    uint64_t addr;

    if (unlikely(g_db32_addr_local_csr == 0)) {
        g_db32_addr_local_csr = sdk::asic::pd::asicpd_local_db32_addr_get();
    }
    SDK_ASSERT(db_addr);
    addr = ASIC_LOCAL_DB32_ADDR_GET(db_addr->lif_id, db_addr->q_type, db_addr->upd);
    ASIC_PAL_DB_WRITE(addr, data, 32);
}

// doorbell16 api
void
asic_ring_db (asic_db_addr_t *db_addr, uint16_t data)
{
    uint64_t addr;

    if (unlikely(g_db_addr_local_csr == 0)) {
        g_db_addr_local_csr = sdk::asic::pd::asicpd_local_dbaddr_get();
    }
    SDK_ASSERT(db_addr);
    addr = ASIC_LOCAL_DBADDR_GET(db_addr->lif_id, db_addr->q_type, db_addr->upd);
    ASIC_PAL_DB_WRITE(addr, data, 16);
}

}    // namespace pd
}    // namespace asic
}    // namespace sdk
