/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    hack_impl.cc
 *
 * @brief   placeholder file for temporary hacks
 */

#include <cmath>
//#include <boost/crc.hpp>
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/platform/utils/lif_manager_base.hpp"
#include "nic/sdk/platform/utils/qstate_mgr.hpp"
#include "nic/sdk/platform/capri/capri_lif_manager.hpp"
#include "nic/sdk/platform/capri/capri_qstate.hpp"
#include "nic/sdk/p4/loader/loader.hpp"
#include "nic/sdk/platform/capri/capri_qstate.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/impl/apollo_impl.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/p4/include/defines.h"

using sdk::platform::capri::LIFManager;
using sdk::platform::utils::program_info;

#define JLIF2QSTATE_MAP_NAME      "lif2qstate_map"
#define JRXDMA_TO_TXDMA_BUF_NAME "rxdma_to_txdma_buf"

typedef struct __attribute__((__packed__)) lifqstate_  {
    uint64_t pc : 8;
    uint64_t rsvd : 8;
    uint64_t cos_a : 4;
    uint64_t coa_b : 4;
    uint64_t cos_sel : 8;
    uint64_t eval_last : 8;
    uint64_t host_rings : 4;
    uint64_t total_rings : 4;
    uint64_t pid : 16;
    uint64_t pindex : 16;
    uint64_t cindex : 16;

    uint64_t arm_pindex : 16;
    uint64_t arm_cindex : 16;
    uint64_t sw_pindex : 16;
    uint16_t sw_cindex;
    uint64_t ring0_base : 64;
    uint64_t ring1_base : 64;
    uint64_t ring0_size : 16;
    uint64_t ring1_size : 16;

    uint8_t  pad[(512-320)/8];
} lifqstate_t;

typedef struct __attribute__((__packed__)) txdma_qstate_  {
    uint64_t pc : 8;
    uint64_t rsvd : 8;
    uint64_t cos_a : 4;
    uint64_t coa_b : 4;
    uint64_t cos_sel : 8;
    uint64_t eval_last : 8;
    uint64_t host_rings : 4;
    uint64_t total_rings : 4;
    uint64_t pid : 16;
    uint64_t pindex : 16;
    uint64_t cindex : 16;

    uint64_t sw_cindex : 16;
    uint64_t ring_size : 16;
    uint64_t ring_base : 64;
    uint64_t rxdma_cindex_addr : 64;

    uint8_t  pad[(512-256)/8];
} txdma_qstate_t;

/**
 * @brief    init routine to initialize service LIFs
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
init_service_lif (void)
{
    uint8_t pgm_offset = 0;

    program_info *pginfo = program_info::factory("conf/gen/mpu_prog_info.json");
    SDK_ASSERT(pginfo != NULL);
/*
    api::g_pds_state.set_prog_info(pginfo);
    LIFManager *lm = LIFManager::factory(api::g_pds_state.mempartition(),
                                         api::g_pds_state.prog_info(),
                                         JLIF2QSTATE_MAP_NAME);
    SDK_ASSERT(lm != NULL);
    api::g_pds_state.set_lif_mgr(lm);
    qstate_mgr *qstate_mgr::factory(const char *qstate_info_file, shmmgr *mmgr)


    lm->LIFRangeMarkAlloced(APOLLO_SERVICE_LIF, 1);
*/

    sdk::platform::utils::LIFQState qstate = { 0 };
    qstate.lif_id = APOLLO_SERVICE_LIF;
    qstate.hbm_address = api::g_pds_state.mempartition()->start_addr(JLIF2QSTATE_MAP_NAME);
    qstate.params_in.type[0].entries = 1;
    qstate.params_in.type[0].size = 1; // 64B
    sdk::platform::capri::push_qstate_to_capri(&qstate, 0);

    // hal_get_pc_offset("txdma_stage0.bin", "apollo_read_qstate", &pgm_offset);
    sdk::platform::capri::get_pc_offset(pginfo,
                                        "txdma_stage0.bin", "apollo_read_qstate", &pgm_offset);

    lifqstate_t lif_qstate = { 0 };
    lif_qstate.pc = pgm_offset;
    lif_qstate.ring0_base = api::g_pds_state.mempartition()->start_addr(JRXDMA_TO_TXDMA_BUF_NAME);
    lif_qstate.ring0_size = log2((api::g_pds_state.mempartition()->size(JRXDMA_TO_TXDMA_BUF_NAME) >> 10) / 10);
    lif_qstate.total_rings = 1;
    sdk::platform::capri::write_qstate(qstate.hbm_address, (uint8_t *)&lif_qstate, sizeof(lif_qstate));

    txdma_qstate_t txdma_qstate = { 0 };
    txdma_qstate.pc = pgm_offset;
    txdma_qstate.rxdma_cindex_addr = qstate.hbm_address + offsetof(lifqstate_t, sw_cindex);
    txdma_qstate.ring_base = api::g_pds_state.mempartition()->start_addr(JRXDMA_TO_TXDMA_BUF_NAME);
    txdma_qstate.ring_size = log2((api::g_pds_state.mempartition()->size(JRXDMA_TO_TXDMA_BUF_NAME) >> 10) / 10);
    txdma_qstate.total_rings = 1;
    sdk::platform::capri::write_qstate(qstate.hbm_address + sizeof(lifqstate_t),
                 (uint8_t *)&txdma_qstate, sizeof(txdma_qstate));

    return SDK_RET_OK;
}
