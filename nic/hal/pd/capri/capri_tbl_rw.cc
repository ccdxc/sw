/*
 * capri_tbl_rw.cc
 * Mahesh Shirshyad (Pensando Systems)
 */

#include <stdio.h>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include <map>
#include "nic/include/capri_common.h"

#include "nic/gen/common_rxdma_actions/include/common_rxdma_actions_p4pd_table.h"
#include "nic/gen/common_txdma_actions/include/common_txdma_actions_p4pd_table.h"
#include "nic/gen/common_rxdma_actions/include/common_rxdma_actions_p4pd.h"
#include "nic/gen/common_txdma_actions/include/common_txdma_actions_p4pd.h"
#include "nic/hal/pd/p4pd_api.hpp"
#include "nic/hal/pd/capri/capri_tbl_rw.hpp"
#include "nic/include/hal.hpp"
#include "nic/include/hal_cfg.hpp"
#include "nic/asic/capri/model/utils/cap_csr_py_if.h"

#include "nic/include/asic_pd.hpp"
#include "nic/asic/capri/model/utils/cap_blk_reg_model.h"
#include "nic/asic/capri/model/cap_top/cap_top_csr.h"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/capri/csr/cpu_hal_if.h"
#include "nic/hal/pd/capri/capri_loader.h"

#include "nic/utils/pack_bytes/pack_bytes.hpp"
#include "sdk/asic/capri/csrlite/cap_top_csr.hpp"

/* When ready to use unified memory mgmt library, change CALLOC and FREE then */
#define CAPRI_CALLOC  calloc
#define CAPRI_FREE    free

typedef int capri_error_t;

/*  Design decisions + Table Update Flow:
 *
 *  Maintain Shadow memory for SRAM and TCAM Units.
 *
 *   1.  Shadow memory maintained in ARM memory will be mirror representation
 *       of how capri TCAM/SRAM addressing is done. This implies
 *       uint8_t ram_row[10 blocks * 16 bytes]. 4k such rows will be created.
 *
 *   2.  To start with there will be single lock to protect updates to shadow
 *       memory (if HAL updates can come from multiple threads)
 *       To improve update performance and avoid lock contention, 4K rows
 *       can be divided into zones and one lock per zone can be maintained.
 *       (TO START WITH, I HAVE DECIDED TO USE ONE LOCK)
 *
 *   3. Using table property API provided by p4pd_api, start block, start
 *      row, and number of buckets (number of table entries within
 *      a single row) are obtained.
 *
 *   4. Using index (at which table entry need to be written to or read from)
 *      row#, one or more blocks#, and start word# in first block,
 *      end word# in last block are computed.
 *
 *   5. Read + modify of the relevant blocks and push those blocks to Capri
 *
 */

/*
 *
 *
 *  CAPRI SRAM ADDRESSING:
 *
 *       +=================================================================+
 * Row0  | Block0 (128bits) | Block1 (128b) | ...........| Block9 (128b)   |
 *       +-----------------------------------------------------------------+
 * Row1  | Block0 (128bits) | Block1 (128b) | ...........| Block9 (128b)   |
 *       +-----------------------------------------------------------------+
 * Row2  | Block0 (128bits) | Block1 (128b) | ...........| Block9 (128b)   |
 *       +-----------------------------------------------------------------+
 *
 *                          :
 *                          :
 *       +-----------------------------------------------------------------+
 * Row   | Block0 (128bits) | Block1 (128b) | ...........| Block9 (128b)   |
 * 4K-1  +=================================================================+
 *
 *
 *     1. Any memory writes / reads are done in units of block. To update
 *        a table entry that is within one or more blocks, all such memory
 *        blocks on a particular Row will need to modified and written back
 *        to capri.
 *
 *     2. Table entry start and end on 16b boundary. Multiple such 16b words
 *        are updated or read from when performing table write or read.
 *
 */

#define CAPRI_SRAM_BLOCK_COUNT      (10)
#define CAPRI_SRAM_BLOCK_WIDTH      (128) // bits
#define CAPRI_SRAM_WORD_WIDTH       (16)  // bits; is also unit of allocation.
#define CAPRI_SRAM_WORDS_PER_BLOCK  (8)
#define CAPRI_SRAM_ROWS             (0x1000) // 4K

#define CAPRI_TCAM_BLOCK_COUNT      (8)
#define CAPRI_TCAM_BLOCK_WIDTH      (128) // bits
#define CAPRI_TCAM_WORD_WIDTH       (16)  // bits; is also unit of allocation.
#define CAPRI_TCAM_WORDS_PER_BLOCK  (8)
#define CAPRI_TCAM_ROWS             (0x400) // 1K

typedef struct capri_sram_shadow_mem_ {
    uint8_t zones;          // Using entire memory as one zone.
                            // TBD: carve into multiple zones
                            // to reduce access/update contention

    //pthread_mutex_t mutex; // TBD: when its decided to make HAL thread safe

    // Since writes/read access to SRAM are in done in units of block
    // a three dim array is maintained
    // Since word width is 16bits, uint16_t is used. A table entry starts at 16b
    // boundary
    uint16_t mem[CAPRI_SRAM_ROWS][CAPRI_SRAM_BLOCK_COUNT][CAPRI_SRAM_WORDS_PER_BLOCK];

} capri_sram_shadow_mem_t;

typedef struct capri_tcam_shadow_mem_ {
    uint8_t zones;          // Using entire memory as one zone.
                            // TBD: carve into multiple zones
                            // to reduce access/update contention

    //pthread_mutex_t mutex; // TBD: when its decided to make HAL thread safe

    uint16_t mem_x[CAPRI_TCAM_ROWS][CAPRI_TCAM_BLOCK_COUNT][CAPRI_TCAM_WORDS_PER_BLOCK];
    uint16_t mem_y[CAPRI_TCAM_ROWS][CAPRI_TCAM_BLOCK_COUNT][CAPRI_TCAM_WORDS_PER_BLOCK];

} capri_tcam_shadow_mem_t;


static capri_sram_shadow_mem_t *g_shadow_sram_p4[2];
static capri_tcam_shadow_mem_t *g_shadow_tcam_p4[2];
static capri_sram_shadow_mem_t *g_shadow_sram_rxdma;
static capri_sram_shadow_mem_t *g_shadow_sram_txdma;

static capri_sram_shadow_mem_t *
get_sram_shadow_for_table (uint32_t tableid, int gress)
{
    if ((tableid >= p4pd_tableid_min_get()) &&
        (tableid <= p4pd_tableid_max_get())) {
        HAL_TRACE_DEBUG("Working with p4 sram shadow for table {}\n", tableid);
        return (g_shadow_sram_p4[gress]);
    } else if ((tableid >= P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMIN) &&
         (tableid <= P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMAX)) {
        HAL_TRACE_DEBUG("Working with rxdma shadow for table {}\n", tableid);
        return (g_shadow_sram_rxdma);
    } else if ((tableid >= P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMIN) &&
         (tableid <= P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMAX)) {
        HAL_TRACE_DEBUG("Working with txdma shadow for table {}\n", tableid);
        return (g_shadow_sram_txdma);
    } else {
        HAL_ASSERT(0);
    }
    return NULL;
}

/* HBM base address in System memory map; Cached once at the init time */
static uint64_t hbm_mem_base_addr;

/* Store action pc for every action of the table. */
static uint64_t capri_action_asm_base[P4TBL_ID_MAX][P4TBL_MAX_ACTIONS];
static uint64_t capri_action_rxdma_asm_base[P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMAX][P4TBL_MAX_ACTIONS];
static uint64_t capri_action_txdma_asm_base[P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMAX][P4TBL_MAX_ACTIONS];
static uint64_t capri_table_rxdma_asm_base[P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMAX];
static uint64_t capri_table_txdma_asm_base[P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMAX];

typedef enum capri_tbl_rw_logging_levels_ {
    CAP_TBL_RW_LOG_LEVEL_ALL = 0,
    CAP_TBL_RW_LOG_LEVEL_INFO,
    CAP_TBL_RW_LOG_LEVEL_ERROR,
} capri_tbl_rw_logging_levels;


#define HAL_LOG_TBL_UPDATES

void
capri_program_table_mpu_pc (int tableid, bool ingress, int stage,
                            int stage_tableid,
                            uint64_t capri_table_asm_err_offset,
                            uint64_t capri_table_asm_base)
{
    /* Program table base address into capri TE */
    using namespace sdk::lib::csrlite;

    assert(stage_tableid < 16);
    HAL_TRACE_DEBUG("====Stage: {} Tbl_id: {}, Stg_Tbl_id {}, Tbl base: {:#x}==",
                    stage, tableid, stage_tableid, capri_table_asm_base);

    if (ingress) {
        sdk::lib::csrlite::cap_te_csr_t &te_csr = *cap_top_csr.sgi.te[stage].alloc();
        // Push to HW/Capri from entry_start_block to block
        te_csr.cfg_table_property[stage_tableid].read();
        te_csr.cfg_table_property[stage_tableid]
                .mpu_pc(((capri_table_asm_base) >> 6));
        te_csr.cfg_table_property[stage_tableid]
                .mpu_pc_ofst_err(capri_table_asm_err_offset);
        te_csr.cfg_table_property[stage_tableid].write();
        cap_top_csr.free(&te_csr);
    } else {
        sdk::lib::csrlite::cap_te_csr_t &te_csr = *cap_top_csr.sge.te[stage].alloc();
        // Push to HW/Capri from entry_start_block to block
        te_csr.cfg_table_property[stage_tableid].read();
        te_csr.cfg_table_property[stage_tableid]
                .mpu_pc(((capri_table_asm_base) >> 6));
        te_csr.cfg_table_property[stage_tableid]
                .mpu_pc_ofst_err(capri_table_asm_err_offset);
        te_csr.cfg_table_property[stage_tableid].write();
        cap_top_csr.free(&te_csr);
    }
}

void
capri_program_hbm_table_base_addr (int stage_tableid, char *tablename,
                                   int stage, bool ingress)
{
    /* Program table base address into capri TE */
    using namespace sdk::lib::csrlite;
    // For each HBM table, program HBM table start address
    assert(stage_tableid < 16);
    HAL_TRACE_DEBUG("===HBM Tbl Name: {}, Stage: {}, "
                    "StageTblID: {}===",
                    tablename, stage,
                    stage_tableid);
                    //get_start_offset(tablename));
    if (ingress) {
        sdk::lib::csrlite::cap_te_csr_t &te_csr = *cap_top_csr.sgi.te[stage].alloc();
        // Push to HW/Capri from entry_start_block to block
        te_csr.cfg_table_property[stage_tableid].read();
        te_csr.cfg_table_property[stage_tableid]
                .addr_base(get_start_offset(tablename));
        te_csr.cfg_table_property[stage_tableid].write();
        cap_top_csr.free(&te_csr);
    } else {
        sdk::lib::csrlite::cap_te_csr_t &te_csr = *cap_top_csr.sge.te[stage].alloc();
        // Push to HW/Capri from entry_start_block to block
        te_csr.cfg_table_property[stage_tableid].read();
        te_csr.cfg_table_property[stage_tableid]
                .addr_base(get_start_offset(tablename));
        te_csr.cfg_table_property[stage_tableid].write();
        cap_top_csr.free(&te_csr);
    }
}

#define CAPRI_P4PLUS_RX_STAGE0_QSTATE_OFFSET_0            0
#define CAPRI_P4PLUS_RX_STAGE0_QSTATE_OFFSET_64           64

static void
capri_program_p4plus_table_mpu_pc_args (int tbl_id, sdk::lib::csrlite::cap_te_csr_t *te_csr,
                                        uint64_t pc, uint32_t offset)
{
    te_csr->cfg_table_property[tbl_id].read();
    te_csr->cfg_table_property[tbl_id].mpu_pc(pc >> 6);
    te_csr->cfg_table_property[tbl_id].mpu_pc_dyn(1);
    te_csr->cfg_table_property[tbl_id].addr_base(offset);
    te_csr->cfg_table_property[tbl_id].write();
}

#define CAPRI_P4PLUS_HANDLE         "p4plus"
#define CAPRI_P4PLUS_RXDMA_PROG		"rxdma_stage0.bin"
#define CAPRI_P4PLUS_RXDMA_EXT_PROG	"rxdma_stage0_ext.bin"
#define CAPRI_P4PLUS_TXDMA_PROG		"txdma_stage0.bin"
#define CAPRI_P4PLUS_TXDMA_EXT_PROG	"txdma_stage0_ext.bin"

void
capri_program_p4plus_sram_table_mpu_pc (int tableid, int stage_tbl_id,
                                        int stage)
{
    uint64_t pc = 0;
    sdk::lib::csrlite::cap_te_csr_t *te_csr = NULL;

    using namespace sdk::lib::csrlite;

    if (tableid >= P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMIN &&
            tableid < P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMAX) {
        te_csr = cap_top_csr.pcr.te[stage].alloc();
        pc = capri_table_rxdma_asm_base[tableid];
    } else if (tableid >= P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMIN &&
            tableid < P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMAX) {
        te_csr = cap_top_csr.pct.te[stage].alloc();
        pc = capri_table_txdma_asm_base[tableid];
    }
    if (pc == 0) {
        return;
    }
    te_csr->cfg_table_property[stage_tbl_id].read();
    te_csr->cfg_table_property[stage_tbl_id].mpu_pc(pc >> 6);
    te_csr->cfg_table_property[stage_tbl_id].mpu_pc_dyn(0);
    te_csr->cfg_table_property[stage_tbl_id].addr_base(0);
    te_csr->cfg_table_property[stage_tbl_id].write();
    cap_top_csr.free(te_csr);

}

// RSS Topelitz Table
#define ETH_RSS_INDIR_PROGRAM               "eth_rx_rss_indir.bin"
// Maximum number of queue per LIF
#define ETH_RSS_MAX_QUEUES                  (128)
// Number of entries in a LIF's indirection table
#define ETH_RSS_LIF_INDIR_TBL_LEN           ETH_RSS_MAX_QUEUES
// Size of each LIF indirection table entry
#define ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ      (sizeof(eth_rx_rss_indir_eth_rx_rss_indir_t))
// Size of a LIF's indirection table
#define ETH_RSS_LIF_INDIR_TBL_SZ            (ETH_RSS_LIF_INDIR_TBL_LEN * ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ)
// Max number of LIFs supported
#define MAX_LIFS                            (2048)
// Size of the entire LIF indirection table
#define ETH_RSS_INDIR_TBL_SZ                (MAX_LIFS * ETH_RSS_LIF_INDIR_TBL_SZ)

int
capri_toeplitz_init (int stage, int stage_tableid)
{
    int tbl_id;
    uint64_t pc;
    uint64_t tbl_base;
    sdk::lib::csrlite::cap_te_csr_t *te_csr = NULL;

    using namespace sdk::lib::csrlite;

    if (capri_program_to_base_addr((char *) CAPRI_P4PLUS_HANDLE,
                                   (char *) ETH_RSS_INDIR_PROGRAM,
                                   &pc) < 0) {
        HAL_TRACE_DEBUG("Could not resolve handle {} program {} \n",
                        (char *) CAPRI_P4PLUS_HANDLE,
                        (char *) ETH_RSS_INDIR_PROGRAM);
        return CAPRI_FAIL;
    }
    HAL_TRACE_DEBUG("Resolved handle {} program {} to PC {:#x}\n",
                    (char *) CAPRI_P4PLUS_HANDLE,
                    (char *) ETH_RSS_INDIR_PROGRAM,
                    pc);

    // Program rss params table with the PC
    te_csr = cap_top_csr.pcr.te[stage].alloc();

    tbl_id = stage_tableid;

    tbl_base = get_start_offset(CAPRI_HBM_REG_RSS_INDIR_TABLE);
    HAL_ASSERT(tbl_base > 0);
    // Align the table address because while calculating the read address TE shifts the LIF
    // value by LOG2 of size of the per lif indirection table.
    tbl_base = (tbl_base + ETH_RSS_INDIR_TBL_SZ) & ~(ETH_RSS_INDIR_TBL_SZ - 1);

    HAL_TRACE_DEBUG("rss_indir_table id {} table_base {}\n", tbl_id, tbl_base);

    te_csr->cfg_table_property[tbl_id].read();
    te_csr->cfg_table_property[tbl_id].mpu_pc(pc >> 6);
    te_csr->cfg_table_property[tbl_id].mpu_pc_dyn(0);
    // HBM Table
    te_csr->cfg_table_property[tbl_id].axi(0); //1==table in SRAM, 0== table in HBM
    // TE addr = hash
    // TE mask = (1 << addr_sz) - 1
    te_csr->cfg_table_property[tbl_id].addr_sz((uint8_t)log2(ETH_RSS_LIF_INDIR_TBL_LEN));
    // TE addr <<= addr_shift
    te_csr->cfg_table_property[tbl_id].addr_shift((uint8_t)log2(ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ));
    // TE addr = (hash & mask) + addr_base
    te_csr->cfg_table_property[tbl_id].addr_base(tbl_base);
    // TE lif_shift_en
    te_csr->cfg_table_property[tbl_id].addr_vf_id_en(1);
    // TE lif_shift
    te_csr->cfg_table_property[tbl_id].addr_vf_id_loc((uint8_t)log2(ETH_RSS_LIF_INDIR_TBL_SZ));
    // addr |= (lif << lif_shift)
    // TE addr = addr & ((1 << chain_shift) - 1) if 0 <= cycle_id < 63 else addr
    te_csr->cfg_table_property[tbl_id].chain_shift(0x3f);
    // size of each indirection table entry
    te_csr->cfg_table_property[tbl_id].lg2_entry_size((uint8_t)log2(ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ));
    te_csr->cfg_table_property[tbl_id].write();

    cap_top_csr.free(te_csr);
    return CAPRI_OK;
}

int
capri_p4plus_table_init (int stage_apphdr, int stage_tableid_apphdr,
                         int stage_apphdr_ext, int stage_tableid_apphdr_ext,
                         int stage_apphdr_off, int stage_tableid_apphdr_off,
                         int stage_apphdr_ext_off, int stage_tableid_apphdr_ext_off,
                         int stage_txdma_act, int stage_tableid_txdma_act,
                         int stage_txdma_act_ext, int stage_tableid_txdma_act_ext)
{
    sdk::lib::csrlite::cap_te_csr_t *te_csr = NULL;
    uint64_t capri_action_p4plus_asm_base;

    using namespace sdk::lib::csrlite;

    hal::hal_cfg_t *hal_cfg =
                (hal::hal_cfg_t *)hal::hal_get_current_thread()->data();
    HAL_ASSERT(hal_cfg);

    // Resolve the p4plus rxdma stage 0 program to its action pc
    if (capri_program_to_base_addr((char *) CAPRI_P4PLUS_HANDLE,
                                   (char *) CAPRI_P4PLUS_RXDMA_PROG,
                                   &capri_action_p4plus_asm_base) < 0) {
        HAL_TRACE_DEBUG("Could not resolve handle {} program {} \n",
                        (char *) CAPRI_P4PLUS_HANDLE,
                        (char *) CAPRI_P4PLUS_RXDMA_PROG);
        return CAPRI_FAIL;
    }
    HAL_TRACE_DEBUG("Resolved handle {} program {} to PC {:#x}\n",
                    (char *) CAPRI_P4PLUS_HANDLE,
                    (char *) CAPRI_P4PLUS_RXDMA_PROG,
                    capri_action_p4plus_asm_base);

    // Program app-header table config @(stage, stage_tableid) with the PC
    te_csr = cap_top_csr.pcr.te[stage_apphdr].alloc();
    capri_program_p4plus_table_mpu_pc_args(
            stage_tableid_apphdr, te_csr,
            capri_action_p4plus_asm_base,
            CAPRI_P4PLUS_RX_STAGE0_QSTATE_OFFSET_0);

    // Program app-header offset 64 table config @(stage, stage_tableid) with the same PC as above
    capri_program_p4plus_table_mpu_pc_args(
            stage_tableid_apphdr_off, te_csr,
            capri_action_p4plus_asm_base,
            CAPRI_P4PLUS_RX_STAGE0_QSTATE_OFFSET_64);

    cap_top_csr.free(te_csr);

    // Resolve the p4plus rxdma stage 0 "ext" program to its action pc
    if (capri_program_to_base_addr((char *) CAPRI_P4PLUS_HANDLE,
                                   (char *) CAPRI_P4PLUS_RXDMA_EXT_PROG,
                                   &capri_action_p4plus_asm_base) < 0) {
        HAL_TRACE_DEBUG("Could not resolve handle {} program {} \n",
                        (char *) CAPRI_P4PLUS_HANDLE,
                        (char *) CAPRI_P4PLUS_RXDMA_EXT_PROG);
        return CAPRI_FAIL;
    }
    HAL_TRACE_DEBUG("Resolved handle {} program {} to PC {:#x}\n",
                    (char *) CAPRI_P4PLUS_HANDLE,
                    (char *) CAPRI_P4PLUS_RXDMA_EXT_PROG,
                    capri_action_p4plus_asm_base);

    // Program app-header table config @(stage, stage_tableid) with the PC
    te_csr = cap_top_csr.pcr.te[stage_apphdr_ext].alloc();
    capri_program_p4plus_table_mpu_pc_args(
            stage_tableid_apphdr_ext, te_csr,
            capri_action_p4plus_asm_base,
            CAPRI_P4PLUS_RX_STAGE0_QSTATE_OFFSET_0);

    // Program app-header offset 64 table config @(stage, stage_tableid) with the same PC as above
    capri_program_p4plus_table_mpu_pc_args(
            stage_tableid_apphdr_ext_off, te_csr,
            capri_action_p4plus_asm_base,
            CAPRI_P4PLUS_RX_STAGE0_QSTATE_OFFSET_64);

    cap_top_csr.free(te_csr);

    // Resolve the p4plus txdma stage 0 program to its action pc
    if (capri_program_to_base_addr((char *) CAPRI_P4PLUS_HANDLE,
                                   (char *) CAPRI_P4PLUS_TXDMA_PROG,
                                   &capri_action_p4plus_asm_base) < 0) {
        HAL_TRACE_DEBUG("Could not resolve handle {} program {} \n",
                        (char *) CAPRI_P4PLUS_HANDLE,
                        (char *) CAPRI_P4PLUS_TXDMA_PROG);
        return CAPRI_FAIL;
    }
    HAL_TRACE_DEBUG("Resolved handle {} program {} to PC {:#x}\n",
                    (char *) CAPRI_P4PLUS_HANDLE,
                    (char *) CAPRI_P4PLUS_TXDMA_PROG,
                    capri_action_p4plus_asm_base);

    // Program table config @(stage, stage_tableid) with the PC
    te_csr = cap_top_csr.pct.te[stage_txdma_act].alloc();
    capri_program_p4plus_table_mpu_pc_args(
            stage_tableid_txdma_act, te_csr,
            capri_action_p4plus_asm_base, 0);

    if (stage_txdma_act == 0 &&
        hal_cfg->platform_mode != hal::HAL_PLATFORM_MODE_SIM) {
        // TODO: This should 16 as we can process 16 packets per doorbell.
        te_csr->cfg_table_property[stage_tableid_txdma_act].max_bypass_cnt(0x10); 
        te_csr->cfg_table_property[stage_tableid_txdma_act].write();
    }

    cap_top_csr.free(te_csr);

    // Resolve the p4plus txdma stage 0 "ext" program to its action pc
    if (capri_program_to_base_addr((char *) CAPRI_P4PLUS_HANDLE,
                                   (char *) CAPRI_P4PLUS_TXDMA_EXT_PROG,
                                   &capri_action_p4plus_asm_base) < 0) {
        HAL_TRACE_DEBUG("Could not resolve handle {} program {} \n",
                        (char *) CAPRI_P4PLUS_HANDLE,
                        (char *) CAPRI_P4PLUS_TXDMA_EXT_PROG);
        return CAPRI_FAIL;
    }
    HAL_TRACE_DEBUG("Resolved handle {} program {} to PC {:#x}\n",
                    (char *) CAPRI_P4PLUS_HANDLE,
                    (char *) CAPRI_P4PLUS_TXDMA_EXT_PROG,
                    capri_action_p4plus_asm_base);

    // Program table config @(stage, stage_tableid) with the PC
    te_csr = cap_top_csr.pct.te[stage_txdma_act_ext].alloc();
    capri_program_p4plus_table_mpu_pc_args(
            stage_tableid_txdma_act_ext, te_csr,
            capri_action_p4plus_asm_base, 0);

    if (stage_txdma_act_ext == 0 &&
        hal_cfg->platform_mode != hal::HAL_PLATFORM_MODE_SIM) {
        // TODO: This should 16 as we can process 16 packets per doorbell.
        te_csr->cfg_table_property[stage_tableid_txdma_act_ext].max_bypass_cnt(0x10); 
        te_csr->cfg_table_property[stage_tableid_txdma_act_ext].write();
    }

    cap_top_csr.free(te_csr);
    return CAPRI_OK ;
}

void
capri_p4plus_recirc_init (void)
{
    sdk::lib::csrlite::cap_psp_csr_cfg_profile_t *cfg_profie;

    using namespace sdk::lib::csrlite;

    cfg_profie = cap_top_csr.pr.pr.psp.cfg_profile.alloc(); // RxDMA
    cfg_profie->read();
    cfg_profie->recirc_max_enable(1);
    cfg_profie->recirc_max(7);
    cfg_profie->write();
    cap_top_csr.free(cfg_profie);

    cfg_profie = cap_top_csr.pt.pt.psp.cfg_profile.alloc(); // TxDMA
    cfg_profie->read();
    cfg_profie->recirc_max_enable(1);
    cfg_profie->recirc_max(7);
    cfg_profie->write();
    cap_top_csr.free(cfg_profie);
}

/* This function initializes the stage id register for p4 plus pipelines such that:
         val0  : 4
         val1  : 5
         val2  : 6
         val3  : 7
         val4  : 0
         val5  : 1
         val6  : 2
         val7  : 3
*/
static void
capri_p4p_stage_id_init (void)
{
    using namespace sdk::lib::csrlite;
    sdk::lib::csrlite::cap_pics_csr_cfg_stage_id_t *cfg_stage_id;

    cfg_stage_id = cap_top_csr.rpc.pics.cfg_stage_id.alloc();
    cfg_stage_id->val0(4); // (0x688FAC >>  0) & 0x7
    cfg_stage_id->val1(5); // (0x688FAC >>  3) & 0x7
    cfg_stage_id->val2(6); // (0x688FAC >>  6) & 0x7
    cfg_stage_id->val3(7); // (0x688FAC >>  9) & 0x7
    cfg_stage_id->val4(0); // (0x688FAC >> 12) & 0x7
    cfg_stage_id->val5(1); // (0x688FAC >> 15) & 0x7
    cfg_stage_id->val6(2); // (0x688FAC >> 18) & 0x7
    cfg_stage_id->val7(3); // (0x688FAC >> 21) & 0x7
    cfg_stage_id->write();
    cap_top_csr.free(cfg_stage_id);

    cfg_stage_id = cap_top_csr.tpc.pics.cfg_stage_id.alloc();
    cfg_stage_id->val0(4); // (0x688FAC >>  0) & 0x7
    cfg_stage_id->val1(5); // (0x688FAC >>  3) & 0x7
    cfg_stage_id->val2(6); // (0x688FAC >>  6) & 0x7
    cfg_stage_id->val3(7); // (0x688FAC >>  9) & 0x7
    cfg_stage_id->val4(0); // (0x688FAC >> 12) & 0x7
    cfg_stage_id->val5(1); // (0x688FAC >> 15) & 0x7
    cfg_stage_id->val6(2); // (0x688FAC >> 18) & 0x7
    cfg_stage_id->val7(3); // (0x688FAC >> 21) & 0x7
    cfg_stage_id->write();
    cap_top_csr.free(cfg_stage_id);
}

static inline bool
p4plus_invalidate_cache_aligned(uint64_t addr, uint32_t size_in_bytes,
        p4plus_cache_action_t action)
{
    using namespace sdk::lib::csrlite;

    assert ((addr & ~CACHE_LINE_SIZE_MASK) == addr);

    while ((int)size_in_bytes > 0) {
        if (action & P4PLUS_CACHE_INVALIDATE_RXDMA) {
            sdk::lib::csrlite::cap_picc_csr_dhs_cache_invalidate_entry_t *entry = cap_top_csr.rpc.pics.picc.dhs_cache_invalidate.entry.alloc();
            entry->addr(addr >> CACHE_LINE_SIZE_SHIFT);
            entry->write();
            cap_top_csr.rpc.pics.picc.dhs_cache_invalidate.entry.free(entry);
        }
        if (action & P4PLUS_CACHE_INVALIDATE_TXDMA) {
            sdk::lib::csrlite::cap_picc_csr_dhs_cache_invalidate_entry_t *entry = cap_top_csr.tpc.pics.picc.dhs_cache_invalidate.entry.alloc();
            entry->addr(addr >> CACHE_LINE_SIZE_SHIFT);
            entry->write();
            cap_top_csr.tpc.pics.picc.dhs_cache_invalidate.entry.free(entry);
        }
        size_in_bytes -= CACHE_LINE_SIZE;
        addr += CACHE_LINE_SIZE;
    }

    return true;
}

bool
p4plus_invalidate_cache(uint64_t addr, uint32_t size_in_bytes,
        p4plus_cache_action_t action)
{
    bool ret;

    if ((addr & ~CACHE_LINE_SIZE_MASK) == addr) {
        ret = p4plus_invalidate_cache_aligned(addr, size_in_bytes, action);
    } else {
        int unalign_size = addr & CACHE_LINE_SIZE_MASK;
        ret = p4plus_invalidate_cache_aligned(addr & ~CACHE_LINE_SIZE_MASK,
                                        size_in_bytes + unalign_size,
                                        action);
    }

    return ret;
}


void
capri_deparser_init(int tm_port_ingress, int tm_port_egress) {
    using namespace sdk::lib::csrlite;
    sdk::lib::csrlite::cap_dpr_csr_cfg_global_2_t *cfg_global_2;
    uint64_t recirc_rw_bm = 0;
    // Ingress deparser is indexed with 1
    cfg_global_2 = cap_top_csr.dpr.dpr[1].cfg_global_2.alloc();
    cfg_global_2->read();
    cfg_global_2->increment_recirc_cnt_en(1);
    cfg_global_2->drop_max_recirc_cnt(1);
    // Drop after 4 recircs
    cfg_global_2->max_recirc_cnt(4);
    cfg_global_2->recirc_oport(tm_port_ingress);
    cfg_global_2->clear_recirc_bit_en(1);
    recirc_rw_bm |= 1<<tm_port_ingress;
    recirc_rw_bm |= 1<<tm_port_egress;
    cfg_global_2->recirc_rw_bm(recirc_rw_bm);
    cfg_global_2->write();
    cap_top_csr.free(cfg_global_2);
    // Egress deparser is indexed with 0
    cfg_global_2 = cap_top_csr.dpr.dpr[0].cfg_global_2.alloc();
    cfg_global_2->read();
    cfg_global_2->increment_recirc_cnt_en(0);
    cfg_global_2->write();
    cap_top_csr.free(cfg_global_2);
}

static void
capri_mpu_icache_invalidate (void)
{
    using namespace sdk::lib::csrlite;
    sdk::lib::csrlite::cap_mpu_csr_icache_t *icache;
    int i;

    for (i = 0; i < CAPRI_P4_NUM_STAGES; i++) {
        icache = cap_top_csr.sgi.mpu[i].icache.alloc();
        icache->read();
        icache->invalidate(1);
        icache->write();
        cap_top_csr.free(icache);
        icache = cap_top_csr.sge.mpu[i].icache.alloc();
        icache->read();
        icache->invalidate(1);
        icache->write();
        cap_top_csr.free(icache);
    }
    for (i = 0; i < CAPRI_P4PLUS_NUM_STAGES; i++) {
        icache = cap_top_csr.pcr.mpu[i].icache.alloc();
        icache->read();
        icache->invalidate(1);
        icache->write();
        cap_top_csr.free(icache);
        icache = cap_top_csr.pct.mpu[i].icache.alloc();
        icache->read();
        icache->invalidate(1);
        icache->write();
        cap_top_csr.free(icache);
    }
}


/* After power on, reset tcam memories.
 * This will ensure that all tcam entry lines are invalid
 */
static void
capri_tcam_memory_init(bool ingress)
{
    using namespace sdk::lib::csrlite;

    /* Toggle reset enable bit */
    if (ingress) {
        sdk::lib::csrlite::cap_pict_csr_t & pict_csr = *cap_top_csr.tsi.pict.alloc();
        pict_csr.cfg_tcam_reset.enable(1);
        pict_csr.cfg_tcam_reset.vec(0xff);
        pict_csr.cfg_tcam_reset.write();
        pict_csr.cfg_tcam_reset.enable(0);
        pict_csr.cfg_tcam_reset.vec(0x00);
        pict_csr.cfg_tcam_reset.write();
        cap_top_csr.free(&pict_csr);
    } else {
        sdk::lib::csrlite::cap_pict_csr_t & pict_csr = *cap_top_csr.tse.pict.alloc();
        pict_csr.cfg_tcam_reset.enable(1);
        pict_csr.cfg_tcam_reset.vec(0xff);
        pict_csr.cfg_tcam_reset.write();
        pict_csr.cfg_tcam_reset.enable(0);
        pict_csr.cfg_tcam_reset.vec(0x00);
        pict_csr.cfg_tcam_reset.write();
        cap_top_csr.free(&pict_csr);
    }
}

void capri_debug_hbm_reset(void);
int
capri_table_rw_init (void)
{
    // !!!!!!
    // Before making this call, it is expected that
    // in HAL init sequence, p4pd_init() is already called..
    // !!!!!!
    /* 1. Create shadow memory and init to zero */
    for (int i = P4_GRESS_INGRESS; i <= P4_GRESS_EGRESS; i++) {
        g_shadow_sram_p4[i] = (capri_sram_shadow_mem_t*)
            CAPRI_CALLOC(1, sizeof(capri_sram_shadow_mem_t));
        g_shadow_tcam_p4[i] = (capri_tcam_shadow_mem_t*)
            CAPRI_CALLOC(1, sizeof(capri_tcam_shadow_mem_t));
        if (!g_shadow_sram_p4[i] || !g_shadow_tcam_p4[i]) {
            // TODO: Log error/trace
            capri_table_rw_cleanup();
            return CAPRI_FAIL;
        }
        // Initialize shadow tcam to match all ones. This makes all entries
        // to be treated as inactive
        memset(g_shadow_tcam_p4[i]->mem_x, 0xFF,
               sizeof(g_shadow_tcam_p4[i]->mem_x));
    }
    g_shadow_sram_rxdma = (capri_sram_shadow_mem_t*)CAPRI_CALLOC(1,
                                sizeof(capri_sram_shadow_mem_t));
    g_shadow_sram_txdma = (capri_sram_shadow_mem_t*)CAPRI_CALLOC(1,
                                sizeof(capri_sram_shadow_mem_t));
    if (!g_shadow_sram_rxdma || !g_shadow_sram_txdma) {
        // TODO: Log error/trace
        capri_table_rw_cleanup();
        return CAPRI_FAIL;
    }
    // register hal cpu interface
    auto cpu_if = new cpu_hal_if("cpu", "all");
    cpu::access()->add_if("cpu_if", cpu_if);
    cpu::access()->set_cur_if_name("cpu_if");

    // Register at top level all MRL classes.
    cap_top_csr_t *cap0_ptr = new cap_top_csr_t("cap0");

    cap0_ptr->init(0);
    CAP_BLK_REG_MODEL_REGISTER(cap_top_csr_t, 0, 0, cap0_ptr);
    register_chip_inst("cap0", 0, 0);

    // Initialize the csr-lite helper
    sdk::lib::csrlite::cap_top_csr.init();

    /* Initialize stage id registers for p4p */
    capri_p4p_stage_id_init();

    hbm_mem_base_addr = (uint64_t)get_start_offset((char*)JP4_PRGM);

    capri_mpu_icache_invalidate();
    capri_debug_hbm_reset();

    /* If cold boot, initialize tcam memories */
    /* ingress pipe tcam memory init */
    capri_tcam_memory_init(true);
    /* egress pipe tcam memory init */
    capri_tcam_memory_init(false);

    return (CAPRI_OK);
}

void
capri_table_rw_cleanup (void)
{
    for (int i = P4_GRESS_INGRESS; i <= P4_GRESS_EGRESS; i++) {
        if (g_shadow_sram_p4[i]) {
            CAPRI_FREE(g_shadow_sram_p4[i]);
        }
        g_shadow_sram_p4[i] = NULL;
        if (g_shadow_tcam_p4[i]) {
            CAPRI_FREE(g_shadow_tcam_p4[i]);
        }
        g_shadow_tcam_p4[i] = NULL;
    }
    if (g_shadow_sram_rxdma) {
        CAPRI_FREE(g_shadow_sram_rxdma);
    }
    if (g_shadow_sram_txdma) {
        CAPRI_FREE(g_shadow_sram_txdma);
    }
    g_shadow_sram_rxdma = NULL;
    g_shadow_sram_txdma = NULL;
}

uint8_t
capri_get_action_pc (uint32_t tableid, uint8_t actionid)
{
    if ((tableid >= p4pd_tableid_min_get()) &&
        (tableid <= p4pd_tableid_max_get())) {
        return ((uint8_t)capri_action_asm_base[tableid][actionid]);
    } else if ((tableid >= P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMIN) &&
         (tableid <= P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMAX)) {
        return ((uint8_t)capri_action_rxdma_asm_base[tableid][actionid]);
    } else if ((tableid >= P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMIN) &&
         (tableid <= P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMAX)) {
        return ((uint8_t)capri_action_txdma_asm_base[tableid][actionid]);
    } else {
        HAL_ASSERT(0);
    }
}

uint8_t
capri_get_action_id (uint32_t tableid, uint8_t actionpc)
{
    if ((tableid >= p4pd_tableid_min_get()) &&
        (tableid <= p4pd_tableid_max_get())) {
        for (int j = 0; j < p4pd_get_max_action_id(tableid); j++) {
            if (capri_action_asm_base[tableid][j] == actionpc) {
                return j;
            }
        }
    } else if ((tableid >= P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMIN) &&
         (tableid <= P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMAX)) {
        for (int j = 0; j < p4pd_common_rxdma_actions_get_max_action_id(tableid); j++) {
            if (capri_action_rxdma_asm_base[tableid][j] == actionpc) {
                return j;
            }
        }
    } else if ((tableid >= P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMIN) &&
         (tableid <= P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMAX)) {
        for (int j = 0; j < p4pd_common_txdma_actions_get_max_action_id(tableid); j++) {
            if (capri_action_txdma_asm_base[tableid][j] == actionpc) {
                return j;
            }
        }
    }
    return (0xff);
}

static void
capri_sram_entry_details_get (uint32_t index,
                              int *sram_row, int *entry_start_block,
                              int *entry_end_block, int *entry_start_word,
                              uint16_t top_left_x, uint16_t top_left_y,
                              uint8_t top_left_block, uint16_t btm_right_y,
                              uint8_t num_buckets, uint16_t entry_width)
{
    *sram_row = top_left_y + (index/num_buckets);
    assert(*sram_row <= btm_right_y);
    int tbl_col = index % num_buckets;
    /* entry_width is in units of SRAM word  -- 16b */

    *entry_start_word = (top_left_x + (tbl_col * entry_width))
                        % CAPRI_SRAM_WORDS_PER_BLOCK;
    /* Capri 16b word within a 128b block is numbered from right to left.*/
    //*entry_start_word = (CAPRI_SRAM_WORDS_PER_BLOCK - 1) - *entry_start_word;

    *entry_start_block = (top_left_block * CAPRI_SRAM_ROWS)
                         + ((((tbl_col * entry_width) + top_left_x)
                           / CAPRI_SRAM_WORDS_PER_BLOCK) * CAPRI_SRAM_ROWS)
                         + top_left_y + (index/num_buckets);

    *entry_end_block = *entry_start_block + (((entry_width - 1) +
                         (*entry_start_word % CAPRI_SRAM_WORDS_PER_BLOCK))
                         / CAPRI_SRAM_WORDS_PER_BLOCK) * CAPRI_SRAM_ROWS;

}

sdk::lib::csrlite::cap_pics_csr_t *
capri_global_pics_get (uint32_t tableid)
{
    using namespace sdk::lib::csrlite;
    if ((tableid >= p4pd_tableid_min_get()) &&
        (tableid <= p4pd_tableid_max_get())) {
        return cap_top_csr.ssi.pics.alloc();
    } else if ((tableid >= P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMIN) &&
               (tableid <= P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMAX)) {
        return cap_top_csr.rpc.pics.alloc();
    } else if ((tableid >= P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMIN) &&
               (tableid <= P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMAX)) {
        return cap_top_csr.tpc.pics.alloc();
    } else {
        HAL_ASSERT(0);
    }
    return ((sdk::lib::csrlite::cap_pics_csr_t*)nullptr);
}

int
capri_table_entry_write (uint32_t tableid,
                         uint32_t index,
                         uint8_t  *hwentry,
                         uint8_t  *hwentry_mask,
                         uint16_t hwentry_bit_len,
                         capri_table_mem_layout_t &tbl_info, int gress,
                         bool is_oflow_table, bool ingress,
                         uint32_t ofl_parent_tbl_depth)
{
    /* 1. When a Memory line is shared by multiple tables, only tableid's
     *    table entry bits need to be modified in the memory line.
     *    1. read Shadow memory line (entire 128bits in case of SRAM)
     *    2. clear out bits that corresponds to table.
     * 2. Argument hwentry contains byte stream that is already in format that
     *    agrees to Capri.
     *    Bytes read from match-table (SRAM or TCAM) are swizzled before
     *    comparing key bits. Today as per HW team, byte swizzing is
     *    Byte 0 in memory is Byte 63 in KeyMaker (512b keymaker)
     *    Byte 1 in memory is Byte 62 in KeyMaker (512b keymaker)
     *    :
     *    Byte 63 in memory is Byte 0 in KeyMaker (512b keymaker)
     * 3. Write all 128bits back to HW. In case of wide key write back
     *    multiple 128b blocks. When writing back all 128b blocks its
     *    possible to update neighbour table's entry back with same value
     *    as before.
     */

    // Assuming a table entry is contained within a SRAM row...
    // Entry cannot be wider than entire row (10 x 128bits)

    int sram_row, entry_start_block, entry_end_block;
    int entry_start_word;
    capri_sram_shadow_mem_t *shadow_sram;

    shadow_sram = get_sram_shadow_for_table(tableid, gress);

    // In case of overflow TCAM, SRAM associated with the table
    // is folded along with its parent's hash table.
    // Change index to parent table size + index
    if (is_oflow_table) {
        index += ofl_parent_tbl_depth;
    }

    capri_sram_entry_details_get(index,
                                 &sram_row, &entry_start_block,
                                 &entry_end_block, &entry_start_word,
                                 tbl_info.top_left_x,
                                 tbl_info.top_left_y,
                                 tbl_info.top_left_block,
                                 tbl_info.btm_right_y,
                                 tbl_info.num_buckets,
                                 tbl_info.entry_width);
    int tbl_col = index % tbl_info.num_buckets;
    int blk = tbl_info.top_left_block
                  + (((tbl_col * tbl_info.entry_width) + tbl_info.top_left_x) /
                     CAPRI_SRAM_WORDS_PER_BLOCK);
    int block = blk;
    int copy_bits = hwentry_bit_len;
    uint16_t *_hwentry = (uint16_t*)hwentry;

    if (hwentry_mask) {
        /* If mask is specified, it should encompass the entire macros currently */
        if ((entry_start_word != 0) || (tbl_info.entry_width % CAPRI_SRAM_WORDS_PER_BLOCK)) {
            HAL_TRACE_ERR("Masked write with entry_start_word {} and width {} "
                          "not supported",
                          entry_start_word, tbl_info.entry_width);
            return HAL_RET_INVALID_ARG;
        }
    }

    for (int j = 0; j < tbl_info.entry_width; j++) {
        if (copy_bits >= 16) {
            shadow_sram->mem[sram_row][block % CAPRI_SRAM_BLOCK_COUNT][entry_start_word] = *_hwentry;
            _hwentry++;
            copy_bits -= 16;
        } else if (copy_bits) {
            assert(0);
        }
        entry_start_word++;
        if (entry_start_word % CAPRI_SRAM_WORDS_PER_BLOCK == 0) {
            // crossed over to next block
            //block += CAPRI_SRAM_ROWS;
            block++;
            entry_start_word = 0;
        }
    }

    using namespace sdk::lib::csrlite;
    // Push to HW/Capri from entry_start_block to block
    uint8_t temp[16], tempmask[16];
    for (int i = entry_start_block; i <= entry_end_block; i += CAPRI_SRAM_ROWS, blk++) {
        //all shadow_sram->mem[sram_row][i] to be pushed to capri..
        uint8_t *s = (uint8_t*)(shadow_sram->mem[sram_row][blk]);
        for (int p = 15; p >= 0; p--) {
            temp[p] = *s; s++;
        }
        sdk::lib::csrlite::cap_pics_csr_t *pics_csr;
        if (ingress) {
            pics_csr = capri_global_pics_get(tableid);
        } else {
            pics_csr = cap_top_csr.sse.pics.alloc();
        }

        if (hwentry_mask) {
            uint8_t *m = hwentry_mask + (i-entry_start_block)*(CAPRI_SRAM_BLOCK_WIDTH>>3) ;
            for (int p = 15; p >= 0; p--) {
                tempmask[p] = *m; m++;
            }

            pics_csr->dhs_sram_update_addr.entry.address(i);
            pics_csr->dhs_sram_update_addr.entry.write();
            pics_csr->dhs_sram_update_data.entry.data(temp);
            pics_csr->dhs_sram_update_data.entry.mask(tempmask);
            pics_csr->dhs_sram_update_data.entry.write();
        } else {
            pics_csr->dhs_sram.entry[i].data(temp);
            pics_csr->dhs_sram.entry[i].write();
        }

        cap_top_csr.free(pics_csr);
    }
    return (CAPRI_OK);
}

int
capri_table_entry_read (uint32_t tableid,
                        uint32_t index,
                        uint8_t  *hwentry,
                        uint16_t *hwentry_bit_len,
                        capri_table_mem_layout_t &tbl_info, int gress,
                        bool is_oflow_table,
                        uint32_t ofl_parent_tbl_depth)
{
    /*
     * Unswizzing of the bytes into readable format is
     * expected to be done by caller of the API.
     */
    int sram_row, entry_start_block, entry_end_block;
    int entry_start_word;
    // In case of overflow TCAM, SRAM associated with the table
    // is folded along with its parent's hash table.
    // Change index to parent table size + index
    if (is_oflow_table) {
        index += ofl_parent_tbl_depth;
    }
    capri_sram_entry_details_get(index,
                                 &sram_row, &entry_start_block,
                                 &entry_end_block, &entry_start_word,
                                 tbl_info.top_left_x,
                                 tbl_info.top_left_y,
                                 tbl_info.top_left_block,
                                 tbl_info.btm_right_y,
                                 tbl_info.num_buckets,
                                 tbl_info.entry_width);
    int tbl_col = index % tbl_info.num_buckets;
    int blk = tbl_info.top_left_block
              + (((tbl_col * tbl_info.entry_width) + tbl_info.top_left_x) /
                 CAPRI_SRAM_WORDS_PER_BLOCK);
    int block = blk;
    int copy_bits = tbl_info.entry_width_bits;
    uint16_t *_hwentry = (uint16_t*)hwentry;

    capri_sram_shadow_mem_t *shadow_sram;

    shadow_sram = get_sram_shadow_for_table(tableid, gress);

    while(copy_bits) {
        if (copy_bits >= 16) {
            *_hwentry = shadow_sram->mem[sram_row][block % CAPRI_SRAM_BLOCK_COUNT][entry_start_word];
            _hwentry++;
            copy_bits -= 16;
        } else {
            if (copy_bits > 8) {
                *_hwentry = shadow_sram->mem[sram_row][block % CAPRI_SRAM_BLOCK_COUNT][entry_start_word];
            } else {
                *(uint8_t*)_hwentry =
                        shadow_sram->mem[sram_row][block % CAPRI_SRAM_BLOCK_COUNT][entry_start_word] >> 8;
            }
            copy_bits = 0;
        }
        entry_start_word++;
        if (entry_start_word % CAPRI_SRAM_WORDS_PER_BLOCK == 0) {
            // crossed over to next block
            block++;
            entry_start_word = 0;
        }
    }

    *hwentry_bit_len = tbl_info.entry_width_bits;

    return CAPRI_OK;
}

int
capri_table_hw_entry_read (uint32_t tableid,
                           uint32_t index,
                           uint8_t  *hwentry,
                           uint16_t *hwentry_bit_len,
                           capri_table_mem_layout_t &tbl_info, int gress,
                           bool is_oflow_table, bool ingress,
                           uint32_t ofl_parent_tbl_depth)
{
    /*
     * Unswizzing of the bytes into readable format is
     * expected to be done by caller of the API.
     */
    int sram_row, entry_start_block, entry_end_block;
    int entry_start_word;
    // In case of overflow TCAM, SRAM associated with the table
    // is folded along with its parent's hash table.
    // Change index to parent table size + index
    if (is_oflow_table) {
        index += ofl_parent_tbl_depth;
    }
    capri_sram_entry_details_get(index,
                                 &sram_row, &entry_start_block,
                                 &entry_end_block, &entry_start_word,
                                 tbl_info.top_left_x,
                                 tbl_info.top_left_y,
                                 tbl_info.top_left_block,
                                 tbl_info.btm_right_y,
                                 tbl_info.num_buckets,
                                 tbl_info.entry_width);
    using namespace sdk::lib::csrlite;
    int copy_bits = tbl_info.entry_width_bits;
    uint8_t *_hwentry = (uint8_t*)hwentry;
    uint8_t  byte, to_copy;
    // read from HW/Capri from entry_start_block to block
    uint8_t *temp;
    for (int i = entry_start_block; (i <= entry_end_block) && (copy_bits > 0);
         i += CAPRI_SRAM_ROWS) {
        if (ingress) {
            sdk::lib::csrlite::cap_pics_csr_t *pics_csr = capri_global_pics_get(tableid);
            pics_csr->dhs_sram.entry[i].read();
            temp = pics_csr->dhs_sram.entry[i].data();
            cap_top_csr.free(pics_csr);
        } else {
            sdk::lib::csrlite::cap_pics_csr_t & pics_csr = *cap_top_csr.sse.pics.alloc();
            pics_csr.dhs_sram.entry[i].read();
            temp = pics_csr.dhs_sram.entry[i].data();
            cap_top_csr.free(&pics_csr);
        }
        for (int p = 15; p >= 8; p--) {
            byte = temp[p];
            temp[p] = temp[15-p];
            temp[15-p] = byte;
        }
        if (entry_start_word) {
            int bits_in_macro = 128 - (entry_start_word * 16);
            to_copy = (copy_bits > bits_in_macro) ? bits_in_macro : copy_bits;
        } else {
            to_copy = (copy_bits > 128) ? 128 : copy_bits;
        }
        uint8_t to_copy_bytes = ((((to_copy - 1) >> 4) + 1) << 4) / 8;
        memcpy(_hwentry, temp + (entry_start_word << 1), to_copy_bytes);
        copy_bits -= to_copy;
        _hwentry += to_copy_bytes;
        entry_start_word = 0;
    }
    *hwentry_bit_len = tbl_info.entry_width_bits;
    return (CAPRI_OK);
}

/*
 *  CAPRI TCAM ADDRESSING:
 *
 *       +====================================================================+
 * Row0  | V | Block0 (128b value,mask) | .... | V | Block7 (128b value,mask) |
 *       +--------------------------------------------------------------------+
 * Row1  | V | Block0 (128b value,mask) | .... | V | Block7 (128b value,mask) |
 *       +--------------------------------------------------------------------+
 *                              :
 *                              :
 *       +--------------------------------------------------------------------+
 * Row   | V | Block0 (128b value,mask) | .... | V | Block7 (128b value,mask) |
 * 1K-1  +====================================================================+
 *
 *
 *     1. Any memory writes / reads are done in units of block. To update
 *        a table entry that is within one or more blocks, all such memory
 *        blocks on a particular Row will need to modified and written back
 *        to capri.
 *
 *     2. Table entry start and end on 16b boundary. Multiple such 16b words
 *        are updated or read from when performing table write or read.
 */
static void
capri_tcam_entry_details_get (uint32_t index,
                              int *tcam_row, int *entry_start_block,
                              int *entry_end_block, int *entry_start_word,
                              uint16_t top_left_y, uint8_t top_left_block,
                              uint16_t btm_right_y, uint8_t num_buckets,
                              uint16_t entry_width, uint32_t start_index)
{
    *tcam_row = top_left_y + (index/num_buckets);
    assert (*tcam_row <= btm_right_y);
    int tbl_col = index % num_buckets;

    /* entry_width is in units of TCAM word  -- 16b */
    /* Since every tcam table entry occupies one TCAM block */
    *entry_start_word = start_index % CAPRI_TCAM_WORDS_PER_BLOCK;
    /* Capri 16b word within a 128b block is numbered from right to left.*/
    //*entry_start_word = (CAPRI_TCAM_WORDS_PER_BLOCK - 1) - *entry_start_word;

    /* Start block will be column away from top-left because in case of
     * tcam, atmost one entry/column of table can occupy a TCAM block.
     */
    *entry_start_block = ((top_left_block + tbl_col) * CAPRI_TCAM_ROWS)
                         + top_left_y
                         + (index/num_buckets);
    *entry_end_block = *entry_start_block + (((entry_width - 1) +
                         (*entry_start_word % CAPRI_TCAM_WORDS_PER_BLOCK))
                          / CAPRI_TCAM_WORDS_PER_BLOCK) * CAPRI_TCAM_ROWS;
}

int
capri_tcam_table_entry_write (uint32_t tableid,
                              uint32_t index,
                              uint8_t  *trit_x,
                              uint8_t  *trit_y,
                              uint16_t hwentry_bit_len,
                              capri_table_mem_layout_t &tbl_info,
                              int gress, bool ingress)
{
    /* 1. When a Memory line is shared by multiple tables, only tableid's
     *    table entry bits need to be modified in the memory line.
     *    1. read Shadow memory line (entire 64bits)
     *    2. clear out bits that corresponds to table
     * 2. Argument trit_x contains key byte stream that is already in format
     *    that agrees to Capri. trit_y contains mask byte stream that is
     *    already in format
     *    Bytes read from match-table (TCAM) are swizzled before
     *    comparing key bits. Today as per HW team, byte swizzing is
     *    Byte 0 in memory is Byte 63 in KeyMaker (512b keymaker)
     *    Byte 1 in memory is Byte 62 in KeyMaker (512b keymaker)
     *    :
     *    Byte 63 in memory is Byte 0 in KeyMaker (512b keymaker)
     * 3. Write all 64bits key,mask back to HW. In case of wide key write back
     *    multiple 64b blocks. When writing back all 64b blocks its
     *    possible to update neighbour table's entry back with same value
     *    as before.
     */
    int tcam_row, entry_start_block, entry_end_block;
    int entry_start_word;

    capri_tcam_entry_details_get(index,
                                 &tcam_row, &entry_start_block,
                                 &entry_end_block, &entry_start_word,
                                 tbl_info.top_left_y,
                                 tbl_info.top_left_block,
                                 tbl_info.btm_right_y,
                                 tbl_info.num_buckets,
                                 tbl_info.entry_width,
                                 tbl_info.start_index);
    int tbl_col = index % tbl_info.num_buckets;
    int blk = tbl_info.top_left_block
                 + ((tbl_col * tbl_info.entry_width) /
                     CAPRI_TCAM_WORDS_PER_BLOCK);
    int block = blk;
    int copy_bits = hwentry_bit_len;
    uint16_t *_trit_x = (uint16_t*)trit_x;
    uint16_t *_trit_y = (uint16_t*)trit_y;
    for (int j = 0; j < tbl_info.entry_width; j++) {
        if (copy_bits >= 16) {
            g_shadow_tcam_p4[gress]->mem_x[tcam_row]
                [block % CAPRI_TCAM_BLOCK_COUNT][entry_start_word] = *_trit_x;
            g_shadow_tcam_p4[gress]->mem_y[tcam_row]
                [block % CAPRI_TCAM_BLOCK_COUNT][entry_start_word] = *_trit_y;
            _trit_x++;
            _trit_y++;
            copy_bits -= 16;
        } else if (copy_bits) {
            assert(0);
        } else {
            // do not match remaining bits from end of entry bits to next 16b
            // aligned word
            g_shadow_tcam_p4[gress]->mem_x[tcam_row]
                [block % CAPRI_TCAM_BLOCK_COUNT][entry_start_word] = 0;
            g_shadow_tcam_p4[gress]->mem_y[tcam_row]
                [block % CAPRI_TCAM_BLOCK_COUNT][entry_start_word] = 0;
        }
        entry_start_word++;
        if (entry_start_word % CAPRI_TCAM_WORDS_PER_BLOCK == 0) {
            // crossed over to next block
            block++;
            entry_start_word = 0;
        }
    }

    using namespace sdk::lib::csrlite;
    // Push to HW/Capri from entry_start_block to block
    uint8_t temp_x[16];
    uint8_t temp_y[16];
    for (int i = entry_start_block; i <= entry_end_block; i += CAPRI_TCAM_ROWS, blk++) {
        uint8_t *s = (uint8_t*)(g_shadow_tcam_p4[gress]->mem_x[tcam_row][blk]);
        for (int p = 15; p >= 0; p--) {
            temp_x[p] = *s; s++;
        }
        s = (uint8_t*)(g_shadow_tcam_p4[gress]->mem_y[tcam_row][blk]);
        for (int p = 15; p >= 0; p--) {
            temp_y[p] = *s; s++;
        }
        if (ingress) {
            sdk::lib::csrlite::cap_pict_csr_t & pict_csr = *cap_top_csr.tsi.pict.alloc();
            pict_csr.dhs_tcam_xy.entry[i].x(temp_x);
            pict_csr.dhs_tcam_xy.entry[i].y(temp_y);
            pict_csr.dhs_tcam_xy.entry[i].valid(1);
            pict_csr.dhs_tcam_xy.entry[i].write();
            cap_top_csr.free(&pict_csr);
        } else {
            sdk::lib::csrlite::cap_pict_csr_t & pict_csr = *cap_top_csr.tse.pict.alloc();
            pict_csr.dhs_tcam_xy.entry[i].x(temp_x);
            pict_csr.dhs_tcam_xy.entry[i].y(temp_y);
            pict_csr.dhs_tcam_xy.entry[i].valid(1);
            pict_csr.dhs_tcam_xy.entry[i].write();
            cap_top_csr.free(&pict_csr);
        }
    }
    return CAPRI_OK;
}

int
capri_tcam_table_entry_read (uint32_t tableid,
                             uint32_t index,
                             uint8_t  *trit_x,
                             uint8_t  *trit_y,
                             uint16_t *hwentry_bit_len,
                             capri_table_mem_layout_t &tbl_info,
                             int gress)
{
    int tcam_row, entry_start_block, entry_end_block;
    int entry_start_word;

    capri_tcam_entry_details_get(index,
                                &tcam_row, &entry_start_block,
                                &entry_end_block, &entry_start_word,
                                tbl_info.top_left_y,
                                tbl_info.top_left_block,
                                tbl_info.btm_right_y,
                                tbl_info.num_buckets,
                                tbl_info.entry_width,
                                tbl_info.start_index);
    int tbl_col = index % tbl_info.num_buckets;
    int blk = tbl_info.top_left_block
                 + ((tbl_col * tbl_info.entry_width) /
                     CAPRI_TCAM_WORDS_PER_BLOCK);
    int block = blk;
    int copy_bits = tbl_info.entry_width_bits;
    int start_word = entry_start_word;
    uint16_t *_trit_x = (uint16_t*)trit_x;
    uint16_t *_trit_y = (uint16_t*)trit_y;
    while(copy_bits) {
        if (copy_bits >= 16) {
            *_trit_x = g_shadow_tcam_p4[gress]->mem_x[tcam_row]
                [block%CAPRI_TCAM_BLOCK_COUNT][start_word];
            *_trit_y = g_shadow_tcam_p4[gress]->mem_y[tcam_row]
                [block%CAPRI_TCAM_BLOCK_COUNT][start_word];
            _trit_x++;
            _trit_y++;
            copy_bits -= 16;
        } else if (copy_bits) {
            if (copy_bits > 8) {
                *_trit_x = g_shadow_tcam_p4[gress]->mem_x[tcam_row]
                    [block%CAPRI_TCAM_BLOCK_COUNT][start_word];
                *_trit_y = g_shadow_tcam_p4[gress]->mem_y[tcam_row]
                    [block%CAPRI_TCAM_BLOCK_COUNT][start_word];
            } else {
                *(uint8_t*)_trit_x = g_shadow_tcam_p4[gress]->mem_x[tcam_row]
                    [block%CAPRI_TCAM_BLOCK_COUNT][start_word] >> 8;
                *(uint8_t*)_trit_y = g_shadow_tcam_p4[gress]->mem_y[tcam_row]
                    [block%CAPRI_TCAM_BLOCK_COUNT][start_word] >> 8;
            }
            copy_bits = 0;
        }
        start_word++;
        if (start_word % CAPRI_TCAM_WORDS_PER_BLOCK == 0) {
            // crossed over to next block
            block++;
            start_word = 0;
        }
    }

    *hwentry_bit_len = tbl_info.entry_width_bits;;

    return CAPRI_OK;
}

int
capri_tcam_table_hw_entry_read (uint32_t tableid,
                                uint32_t index,
                                uint8_t  *trit_x,
                                uint8_t  *trit_y,
                                uint16_t *hwentry_bit_len,
                                capri_table_mem_layout_t &tbl_info,
                                bool ingress)
{
    int tcam_row, entry_start_block, entry_end_block;
    int entry_start_word;

    capri_tcam_entry_details_get(index,
                                 &tcam_row, &entry_start_block,
                                 &entry_end_block, &entry_start_word,
                                 tbl_info.top_left_y,
                                 tbl_info.top_left_block,
                                 tbl_info.btm_right_y,
                                 tbl_info.num_buckets,
                                 tbl_info.entry_width,
                                 tbl_info.start_index);
    int copy_bits = tbl_info.entry_width_bits;
    uint8_t byte, to_copy;
    uint8_t *_trit_x = (uint8_t*)trit_x;
    uint8_t *_trit_y = (uint8_t*)trit_y;

    using namespace sdk::lib::csrlite;
    // Push to HW/Capri from entry_start_block to block
    cpp_int tcam_block_data_x;
    cpp_int tcam_block_data_y;
    uint8_t *temp_x;
    uint8_t *temp_y;
    for (int i = entry_start_block; (i <= entry_end_block) && (copy_bits > 0);
         i += CAPRI_TCAM_ROWS) {
        if (ingress) {
            sdk::lib::csrlite::cap_pict_csr_t & pict_csr = *cap_top_csr.tsi.pict.alloc();
            pict_csr.dhs_tcam_xy.entry[i].read();
            temp_x = pict_csr.dhs_tcam_xy.entry[i].x();
            temp_y = pict_csr.dhs_tcam_xy.entry[i].y();
            cap_top_csr.free(&pict_csr);
        } else {
            sdk::lib::csrlite::cap_pict_csr_t & pict_csr = *cap_top_csr.tse.pict.alloc();
            pict_csr.dhs_tcam_xy.entry[i].read();
            temp_x = pict_csr.dhs_tcam_xy.entry[i].x();
            temp_y = pict_csr.dhs_tcam_xy.entry[i].y();
            cap_top_csr.free(&pict_csr);
        }
        for (int p = 15; p >= 8; p--) {
            byte = temp_x[p];
            temp_x[p] = temp_x[15-p];
            temp_x[15-p] = byte;
        }
        for (int p = 15; p >= 8; p--) {
            byte = temp_y[p];
            temp_y[p] = temp_y[15-p];
            temp_y[15-p] = byte;
        }
        if (entry_start_word) {
            int bits_in_macro = 128 - (entry_start_word * 16);
            to_copy = (copy_bits > bits_in_macro) ? bits_in_macro : copy_bits;
        } else {
            to_copy = (copy_bits > 128) ? 128: copy_bits;
        }
        uint8_t to_copy_bytes = ((((to_copy - 1) >> 4) + 1) << 4) / 8;
        memcpy(_trit_x, temp_x + (entry_start_word << 1), to_copy_bytes);
        memcpy(_trit_y, temp_y + (entry_start_word << 1), to_copy_bytes);
        _trit_x += to_copy_bytes;
        _trit_y += to_copy_bytes;
        entry_start_word = 0;
        copy_bits -= to_copy;
    }
    *hwentry_bit_len = tbl_info.entry_width_bits;;

    return (CAPRI_OK);
}

int
capri_hbm_table_entry_write (uint32_t tableid,
                             uint32_t index,
                             uint8_t *hwentry,
                             uint16_t entry_size,
                             capri_table_mem_layout_t &tbl_info)
{
    assert((entry_size >> 3) <= tbl_info.entry_width);
    assert(index < tbl_info.tabledepth);
    uint64_t entry_start_addr = (index * tbl_info.entry_width);

    hal::pd::asic_mem_write(get_start_offset(tbl_info.tablename) + entry_start_addr,
                            hwentry, (entry_size >> 3));
    return CAPRI_OK;
}

int
capri_hbm_table_entry_cache_invalidate (bool ingress,
                                        uint64_t entry_addr,
                                        capri_table_mem_layout_t &tbl_info)
{
    using namespace sdk::lib::csrlite;

    if (ingress) {
        sdk::lib::csrlite::cap_pics_csr_t & pics_csr = *cap_top_csr.ssi.pics.alloc();
        // write upper 28b of 34b hbm addr.
        pics_csr.picc.dhs_cache_invalidate.entry.addr((get_start_offset(tbl_info.tablename) + entry_addr) >> 6);
        pics_csr.picc.dhs_cache_invalidate.entry.write();
        cap_top_csr.free(&pics_csr);
    } else {
        sdk::lib::csrlite::cap_pics_csr_t & pics_csr = *cap_top_csr.sse.pics.alloc();
        // write upper 28b of 34b hbm addr.
        pics_csr.picc.dhs_cache_invalidate.entry.addr((get_start_offset(tbl_info.tablename) + entry_addr) >> 6);
        pics_csr.picc.dhs_cache_invalidate.entry.write();
        cap_top_csr.free(&pics_csr);
    }

    return CAPRI_OK;
}


int
capri_hbm_table_entry_read (uint32_t tableid,
                            uint32_t index,
                            uint8_t *hwentry,
                            uint16_t *entry_size,
                            capri_table_mem_layout_t &tbl_info)
{
    assert(index < tbl_info.tabledepth);
    uint64_t entry_start_addr = (index * tbl_info.entry_width);

    hal::pd::asic_mem_read(get_start_offset(tbl_info.tablename) + entry_start_addr,
                           hwentry, tbl_info.entry_width);
    *entry_size = tbl_info.entry_width;
    return CAPRI_OK;
}

int
capri_table_constant_write (uint64_t val, uint32_t stage,
                            uint32_t stage_tableid, bool ingress)
{
    using namespace sdk::lib::csrlite;
    if (ingress) {
        sdk::lib::csrlite::cap_te_csr_t &te_csr = *cap_top_csr.sgi.te[stage].alloc();
        te_csr.cfg_table_mpu_const[stage_tableid].value(val);
        te_csr.cfg_table_mpu_const[stage_tableid].write();
        cap_top_csr.free(&te_csr);
    } else {
        sdk::lib::csrlite::cap_te_csr_t &te_csr = *cap_top_csr.sge.te[stage].alloc();
        te_csr.cfg_table_mpu_const[stage_tableid].value(val);
        te_csr.cfg_table_mpu_const[stage_tableid].write();
        cap_top_csr.free(&te_csr);
    }
    return CAPRI_OK;
}

int
capri_table_constant_read (uint64_t *val, uint32_t stage,
                           int stage_tableid, bool ingress)
{
    using namespace sdk::lib::csrlite;
    if (ingress) {
        sdk::lib::csrlite::cap_te_csr_t &te_csr = *cap_top_csr.sgi.te[stage].alloc();
        te_csr.cfg_table_mpu_const[stage_tableid].read();
        *val = te_csr.cfg_table_mpu_const[stage_tableid].value();
        cap_top_csr.free(&te_csr);
    } else {
        sdk::lib::csrlite::cap_te_csr_t &te_csr = *cap_top_csr.sge.te[stage].alloc();
        te_csr.cfg_table_mpu_const[stage_tableid].read();
        *val = te_csr.cfg_table_mpu_const[stage_tableid].value();
        cap_top_csr.free(&te_csr);
    }
    return CAPRI_OK;
}

void
capri_set_action_asm_base (int tableid, int actionid,
                           uint64_t asm_base)
{
    capri_action_asm_base[tableid][actionid] = asm_base;
    return;
}

void
capri_set_action_rxdma_asm_base (int tableid, int actionid,
                                 uint64_t asm_base)
{
    capri_action_rxdma_asm_base[tableid][actionid] = asm_base;
    return;
}

void
capri_set_action_txdma_asm_base (int tableid, int actionid,
                                 uint64_t asm_base)
{
    capri_action_txdma_asm_base[tableid][actionid] = asm_base;
    return;
}

void
capri_set_table_rxdma_asm_base (int tableid,
                                uint64_t asm_base)
{
    capri_table_rxdma_asm_base[tableid] = asm_base;
    return;
}

void
capri_set_table_txdma_asm_base (int tableid,
                                uint64_t asm_base)
{
    capri_table_txdma_asm_base[tableid] = asm_base;
    return;
}

void 
capri_debug_hbm_read (void)
{
    hal_ret_t ret = HAL_RET_OK;
    uint64_t start_addr = get_start_offset("mpu-debug");
    uint64_t count = (get_size_kb("mpu-debug") << 10) >> 3;
    uint64_t addr;
    uint64_t data;

    HAL_TRACE_DEBUG("------------------ READ HBM START -----------");
    for (uint64_t i = 0; i < count; i++) {
        addr = (start_addr + (i<<3)) & 0xffffffff8; 
        ret = hal::pd::asic_mem_read(addr, (uint8_t *)&data, sizeof(data));
        if (ret != HAL_RET_OK) {
            HAL_TRACE_DEBUG("ERROR reading {:#x} ret {} ", i, ret);
            continue;
        }
        if (data != 0xabababababababab) {
            HAL_TRACE_DEBUG("addr {:#x}, data {:#x}", i, data);
        }
    }
    HAL_TRACE_DEBUG("------------------ READ HBM END -----------");
}

void 
capri_debug_hbm_reset (void)
{
    hal_ret_t ret = HAL_RET_OK;
    uint64_t start_addr = get_start_offset("mpu-debug");
    uint64_t count = (get_size_kb("mpu-debug") << 10) >> 3;
    uint64_t addr;
    uint64_t data = 0xabababababababab;

    HAL_ASSERT(start_addr == get_start_offset("mpu-debug"));

    HAL_TRACE_DEBUG("------------------ RESET HBM START -----------");
    for (uint64_t i = 0; i < count; i++) {
        addr = (start_addr + (i<<3)) & 0xffffffff8; 
        ret = hal::pd::asic_mem_write(addr, (uint8_t *)&data, sizeof(data));
        if (ret != HAL_RET_OK) {
            HAL_TRACE_DEBUG("ERROR writing {:#x} ret {} ", i, ret);
            continue;
        }
        HAL_TRACE_DEBUG("addr {:#x}, data {:#x}", i, data);
    }
    HAL_TRACE_DEBUG("------------------ RESET HBM END -----------");
}

extern cap_csr_base *get_csr_base_from_path(string);

namespace hal {
namespace pd {

vector < tuple < string, string, string >>
asic_csr_dump_reg (char *block_name, bool exclude_mem)
{

    typedef vector< tuple< std::string, std::string, std::string> > reg_data;
    cap_csr_base *objP = get_csr_base_from_path(block_name);
    if (objP == 0) { HAL_TRACE_DEBUG("invalid reg name"); return reg_data(); };
    vector<cap_csr_base *> cap_child_base = objP->get_children(-1);
    reg_data data_tl;
    for (auto itr : cap_child_base) {
        if (itr->get_csr_type() == cap_csr_base::CSR_TYPE_REGISTER) {
            if(itr->get_parent() != nullptr && exclude_mem) {
                if (itr->get_parent()->get_csr_type() == cap_csr_base::CSR_TYPE_MEMORY) {
                    continue;
                }
            }
            // read name of register
            string name = itr->get_hier_path();
            // read data - same as csr_read()
            itr->read();
            cpp_int data = itr->all();
            stringstream ss;
            ss << hex << "0x" << data;

            // read offset
            uint64_t offset = itr->get_offset();
            stringstream addr;
            addr << hex << "0x" << offset;

            data_tl.push_back( tuple< std::string, string, std::string>(name, addr.str(), ss.str()));

        }
    }
    return data_tl;
}

vector <string>
asic_csr_list_get (string path, int level)
{
    cap_csr_base *objP = get_csr_base_from_path(path);
    vector <string> block_name;
    if (objP == 0) return vector<string>();
    for (auto itr : objP->get_children(level)) {
        block_name.push_back(itr->get_hier_path());
    }
    return block_name;
}

}
}

extern void capri_tm_dump_debug_regs(void);
extern void capri_tm_dump_config_regs(void);
extern void capri_tm_dump_all_regs(void);

std::string
hal::pd::asic_csr_dump (char *csr_str)
{
    HAL_TRACE_DEBUG("{} csr_str print: {}", __FUNCTION__, csr_str);
    std::string val = "";

    if (!strcmp(csr_str, "pbc_debug")) {
        capri_tm_dump_debug_regs();
    } else if (!strcmp(csr_str, "pbc_config")) {
        capri_tm_dump_config_regs();
    } else if (!strcmp(csr_str, "pbc_all")) {
        capri_tm_dump_all_regs();
    } else if (!strcmp(csr_str, "mpu_debug")) {
        capri_debug_hbm_read();
    } else if (!strcmp(csr_str, "mpu_reset")) {
        capri_debug_hbm_reset();
    } else {
        val = csr_read(std::string(csr_str));
        uint64_t offset = csr_get_offset(std::string(csr_str));
        HAL_TRACE_DEBUG("{0:s}, csr: {1:s}, offset: 0x{2:x}, value: {3:s}",
                        __FUNCTION__, csr_str, offset, val);
        csr_show(std::string(csr_str), -1);
    }
    return val;
}
