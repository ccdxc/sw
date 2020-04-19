/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    hack_impl.cc
 *
 * @brief   placeholder file for temporary hacks
 */

#include <cmath>
#include <cstdio>
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/platform/utils/lif_manager_base.hpp"
#include "nic/sdk/platform/utils/qstate_mgr.hpp"
#include "nic/sdk/asic/cmn/asic_qstate.hpp"
#include "nic/sdk/p4/loader/loader.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/impl/apollo/apollo_impl.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/p4/include/defines.h"
#include "nic/sdk/platform/devapi/devapi_types.hpp"
#include "nic/apollo/api/impl/devapi_impl.hpp"
#include "nic/apollo/api/upgrade_state.hpp"

using sdk::platform::utils::program_info;
using namespace sdk::asic::pd;

#define JLIF2QSTATE_MAP_NAME        "lif2qstate_map"
#define JRXDMA_TO_TXDMA_BUF_NAME    "rxdma_to_txdma_buf"
#define JRXDMA_TO_TXDMA_DESC_NAME   "rxdma_to_txdma_desc"

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

    uint16_t sw_pindex;
    uint16_t sw_cindex;
    uint64_t ring0_base : 64;
    uint64_t ring1_base : 64;
    uint64_t ring_size : 16;
    uint64_t rxdma_cindex_addr : 64;

    uint8_t  pad[(512-336)/8];
} lifqstate_t;

/**
 * @brief    init routine to initialize service LIFs
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
init_service_lif (uint32_t lif_id, const char *cfg_path)
{
    uint8_t pgm_offset = 0;
    std::string prog_info_file;

    prog_info_file = std::string(cfg_path) + std::string("/") +
        std::string(LDD_INFO_FILE_RPATH) +
        std::string("/") + std::string(LDD_INFO_FILE_NAME);

    program_info *pginfo = program_info::factory(prog_info_file.c_str());
    SDK_ASSERT(pginfo != NULL);
    api::g_pds_state.set_prog_info(pginfo);

    sdk::platform::utils::LIFQState qstate = { 0 };
    qstate.lif_id = lif_id;
    qstate.hbm_address = api::g_pds_state.mempartition()->start_addr(JLIF2QSTATE_MAP_NAME);
    SDK_ASSERT(qstate.hbm_address != INVALID_MEM_ADDRESS);
    qstate.params_in.type[0].entries = 1;
    qstate.params_in.type[0].size = 1; // 64B
    asicpd_qstate_push(&qstate, 0);

    sdk::asic::get_pc_offset(pginfo, "txdma_stage0.bin",
                             "apollo_read_qstate", &pgm_offset);

    lifqstate_t lif_qstate = { 0 };
    lif_qstate.pc = pgm_offset;
    lif_qstate.ring0_base = api::g_pds_state.mempartition()->start_addr(JRXDMA_TO_TXDMA_BUF_NAME);
    SDK_ASSERT(lif_qstate.ring0_base != INVALID_MEM_ADDRESS);
    lif_qstate.ring1_base = api::g_pds_state.mempartition()->start_addr(JRXDMA_TO_TXDMA_DESC_NAME);
    SDK_ASSERT(lif_qstate.ring1_base != INVALID_MEM_ADDRESS);
    lif_qstate.ring_size = log2((api::g_pds_state.mempartition()->size(JRXDMA_TO_TXDMA_BUF_NAME) >> 10) / 10);
    lif_qstate.total_rings = 1;
    sdk::asic::write_qstate(qstate.hbm_address, (uint8_t *) &lif_qstate,
                            sizeof(lif_qstate));

    lifqstate_t txdma_qstate = { 0 };
    txdma_qstate.pc = pgm_offset;
    txdma_qstate.rxdma_cindex_addr = qstate.hbm_address + offsetof(lifqstate_t, sw_cindex);
    txdma_qstate.ring0_base = api::g_pds_state.mempartition()->start_addr(JRXDMA_TO_TXDMA_BUF_NAME);
    SDK_ASSERT(txdma_qstate.ring0_base != INVALID_MEM_ADDRESS);
    txdma_qstate.ring1_base = api::g_pds_state.mempartition()->start_addr(JRXDMA_TO_TXDMA_DESC_NAME);
    SDK_ASSERT(txdma_qstate.ring1_base != INVALID_MEM_ADDRESS);
    txdma_qstate.ring_size = log2((api::g_pds_state.mempartition()->size(JRXDMA_TO_TXDMA_BUF_NAME) >> 10) / 10);
    txdma_qstate.total_rings = 1;
    sdk::asic::write_qstate(qstate.hbm_address + sizeof(lifqstate_t),
                            (uint8_t *) &txdma_qstate, sizeof(txdma_qstate));

    //Program the TxDMA scheduler for this LIF.
    sdk::platform::lif_info_t lif_info;

    memset(&lif_info, 0, sizeof(lif_info));
    strncpy(lif_info.name, "Apollo Service LIF", sizeof(lif_info.name));
    lif_info.lif_id = lif_id;
    lif_info.type = sdk::platform::LIF_TYPE_SERVICE;
    lif_info.tx_sched_table_offset = INVALID_INDEXER_INDEX;
    lif_info.tx_sched_num_table_entries = 0;
    lif_info.queue_info[0].type_num = 0;
    lif_info.queue_info[0].size = 1; // 64B
    lif_info.queue_info[0].entries = 1; // 2 Queues
    api::impl::devapi_impl::lif_program_tx_scheduler(&lif_info);

    return SDK_RET_OK;
}

/**
 * @brief     routine to validate service LIFs config during upgrade
 *            during A to B upgrade this will be called by B
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
service_lif_upg_verify (uint32_t lif_id, const char *cfg_path)
{
    uint8_t pgm_offset = 0;
    int32_t rv;
    std::string prog_info_file;
    lifqstate_t lif_qstate;

    prog_info_file = std::string(cfg_path) + std::string("/") +
        std::string(LDD_INFO_FILE_RPATH) +
        std::string("/") + std::string(LDD_INFO_FILE_NAME);

    program_info *pginfo = program_info::factory(prog_info_file.c_str());
    SDK_ASSERT(pginfo != NULL);
    api::g_pds_state.set_prog_info(pginfo);

    sdk::platform::utils::LIFQState qstate = { 0 };
    qstate.lif_id = lif_id;
    qstate.hbm_address = api::g_pds_state.mempartition()->start_addr(JLIF2QSTATE_MAP_NAME);
    if (qstate.hbm_address == INVALID_MEM_ADDRESS) {
        PDS_TRACE_ERR("LIF map not found");
        return SDK_RET_ERR;
    }
    // get the qstate pc offsets
    rv = sdk::asic::get_pc_offset(pginfo, "txdma_stage0.bin",
                                  "apollo_read_qstate", &pgm_offset);
    if (rv != 0) {
        PDS_TRACE_ERR("TXDMA stage0 pgm not found");
        return SDK_RET_ERR;
    }

    rv = sdk::asic::read_qstate(qstate.hbm_address, (uint8_t *) &lif_qstate,
                                sizeof(lifqstate_t));
    if (rv != 0) {
        PDS_TRACE_ERR("RXDMA qstate read failed");
        return SDK_RET_ERR;
    }
    // compare the ring configuration done by A with B config
    // it should be matching in address and size
    if ((lif_qstate.ring0_base != api::g_pds_state.mempartition()->start_addr(JRXDMA_TO_TXDMA_BUF_NAME)) ||
        (lif_qstate.ring1_base != api::g_pds_state.mempartition()->start_addr(JRXDMA_TO_TXDMA_DESC_NAME)) ||
        (lif_qstate.ring_size != log2((api::g_pds_state.mempartition()->size(JRXDMA_TO_TXDMA_BUF_NAME) >> 10) / 10))) {
        PDS_TRACE_ERR("RXDMA qstate config mismatch found");
        return SDK_RET_ERR;
    }

    PDS_TRACE_DEBUG("Moving qstate addr 0x%lx, pc offset %u, to offset %u",
                    qstate.hbm_address, lif_qstate.pc, pgm_offset);

    rv = sdk::asic::read_qstate(qstate.hbm_address + sizeof(lifqstate_t),
                                (uint8_t *)&lif_qstate, sizeof(lifqstate_t));
    if (rv != 0) {
        PDS_TRACE_ERR("TXDMA qstate read failed");
        return SDK_RET_ERR;
    }
    // compare the ring configuration done by A with B config
    // it should be matching in address and size
    if ((lif_qstate.ring0_base != api::g_pds_state.mempartition()->start_addr(JRXDMA_TO_TXDMA_BUF_NAME)) ||
        (lif_qstate.ring1_base != api::g_pds_state.mempartition()->start_addr(JRXDMA_TO_TXDMA_DESC_NAME)) ||
        (lif_qstate.ring_size != log2((api::g_pds_state.mempartition()->size(JRXDMA_TO_TXDMA_BUF_NAME) >> 10) / 10))) {
        PDS_TRACE_ERR("TXDMA qstate config mismatch found");
        return SDK_RET_ERR;
    }

    PDS_TRACE_DEBUG("Moving qstate addr 0x%lx, pc offset %u, to offset %u",
                    qstate.hbm_address + sizeof(lifqstate_t), lif_qstate.pc, pgm_offset);

    // save the qstate hbm address
    // this will be applied during switchover stage from A to B
    if (pgm_offset != lif_qstate.pc) {
        lif_qstate.pc = pgm_offset;
        api::g_upg_state->set_qstate_cfg(qstate.hbm_address, sizeof(lifqstate_t), pgm_offset);
        api::g_upg_state->set_qstate_cfg(qstate.hbm_address + sizeof(lifqstate_t),
                                         sizeof(lifqstate_t), pgm_offset);
    }
    return SDK_RET_OK;
}
