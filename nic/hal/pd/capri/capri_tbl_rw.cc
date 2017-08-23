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
//#include <base.h>

#include <p4pd.h>
#include <p4pd_api.hpp>
#include <capri_tbl_rw.hpp>

#ifndef P4PD_CLI
#include <capri_loader.h>
#include <lib_model_client.h>
#include <cap_blk_reg_model.h>
#include <cap_top_csr.h>
#include <cap_pict_csr.h>
#include <cap_pics_csr.h>
#include <cap_te_csr.h>
#include <cpp_int_helper.h>
#include <capri_hbm.hpp>
#include "cpu_hal_if.h"
#endif

/* When ready to use unified memory mgmt library, change CALLOC and FREE then */
#define CAPRI_CALLOC  calloc
#define CAPRI_FREE    free

#define CAPRI_OK (0)
#define CAPRI_FAIL (-1)

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


static capri_sram_shadow_mem_t *g_shadow_sram;
static capri_tcam_shadow_mem_t *g_shadow_tcam;

/* HBM base address in System memory map; Cached once at the init time */
static uint64_t hbm_mem_base_addr;

/* Store base address for the table. */
static uint64_t capri_table_asm_base[P4TBL_ID_TBLMAX];

#define P4TBL_MAX_ACTIONS (64)
/* Store action pc for every action of the table. */
static uint64_t capri_action_asm_base[P4TBL_ID_TBLMAX][P4TBL_MAX_ACTIONS];
#define P4ACTION_NAME_MAX_LEN (100)

typedef enum capri_tbl_rw_logging_levels_ {
    CAP_TBL_RW_LOG_LEVEL_ALL = 0,
    CAP_TBL_RW_LOG_LEVEL_INFO,
    CAP_TBL_RW_LOG_LEVEL_ERROR,
} capri_tbl_rw_logging_levels;

static FILE *log_file_fd;

#define HAL_LOG_TBL_UPDATES

static void
capri_program_table_mpu_pc(void)
{
#ifndef P4PD_CLI
    p4pd_table_properties_t       tbl_ctx;
    /* Program table base address into capri TE */
    cap_top_csr_t & cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    for (int i = P4TBL_ID_TBLMIN; i < P4TBL_ID_TBLMAX; i++) {
        p4pd_table_properties_get(i, &tbl_ctx);
        if (tbl_ctx.is_oflow_table) {
            // Since Oflow and hash table share same tableid,
            // only one MPU_PC (Table base) address of hash table
            // should be programmed. In that case, skip
            // oflow table id.
            continue;
        }
        assert(tbl_ctx.stage_tableid < 16);
        HAL_TRACE_DEBUG("===========Tbl id: {}, Tbl base: {:#x}=========",
                        i, capri_table_asm_base[i]);
        if (tbl_ctx.gress == P4_GRESS_INGRESS) {
            cap_te_csr_t &te_csr = cap0.sgi.te[tbl_ctx.stage];
            // Push to HW/Capri from entry_start_block to block
            te_csr.cfg_table_property[tbl_ctx.stage_tableid].read();
            te_csr.cfg_table_property[tbl_ctx.stage_tableid]
                    .mpu_pc(((capri_table_asm_base[i]) >> 6));
            te_csr.cfg_table_property[tbl_ctx.stage_tableid].write();
        } else {
            cap_te_csr_t &te_csr = cap0.sge.te[tbl_ctx.stage];
            // Push to HW/Capri from entry_start_block to block
            te_csr.cfg_table_property[tbl_ctx.stage_tableid].read();
            te_csr.cfg_table_property[tbl_ctx.stage_tableid]
                    .mpu_pc(((capri_table_asm_base[i]) >> 6));
            te_csr.cfg_table_property[tbl_ctx.stage_tableid].write();
        }
    }
#endif
}

static int capri_stats_region_init()
{
    p4pd_table_properties_t       tbl_ctx;
    uint32_t                      stats_base_addr;
    uint32_t                      stats_region_start;
    uint32_t                      stats_region_size;

    stats_region_start = stats_base_addr = get_start_offset(JP4_ATOMIC_STATS);
    stats_region_size = get_size_kb(JP4_ATOMIC_STATS) * 1024;

    capri_table_constant_write(P4TBL_ID_FLOW_STATS, stats_base_addr);
    p4pd_table_properties_get(P4TBL_ID_FLOW_STATS, &tbl_ctx);
    stats_base_addr += tbl_ctx.tabledepth * 32;

    capri_table_constant_write(P4TBL_ID_INGRESS_POLICER_ACTION,
                               stats_base_addr);
    p4pd_table_properties_get(P4TBL_ID_INGRESS_POLICER_ACTION, &tbl_ctx);
    stats_base_addr += tbl_ctx.tabledepth * 32;

    capri_table_constant_write(P4TBL_ID_EGRESS_POLICER_ACTION, stats_base_addr);
    p4pd_table_properties_get(P4TBL_ID_EGRESS_POLICER_ACTION, &tbl_ctx);
    stats_base_addr += tbl_ctx.tabledepth * 32;

    capri_table_constant_write(P4TBL_ID_COPP_ACTION, stats_base_addr);
    p4pd_table_properties_get(P4TBL_ID_COPP_ACTION, &tbl_ctx);
    stats_base_addr += tbl_ctx.tabledepth * 32;

    capri_table_constant_write(P4TBL_ID_DROP_STATS, stats_base_addr);
    p4pd_table_properties_get(P4TBL_ID_DROP_STATS, &tbl_ctx);
    stats_base_addr += tbl_ctx.tabledepth;

    capri_table_constant_write(P4TBL_ID_TX_STATS, stats_base_addr);
    p4pd_table_properties_get(P4TBL_ID_TX_STATS, &tbl_ctx);
    stats_base_addr += tbl_ctx.tabledepth * 56;

    capri_table_constant_write(P4TBL_ID_INGRESS_TX_STATS, stats_base_addr);
    p4pd_table_properties_get(P4TBL_ID_INGRESS_TX_STATS, &tbl_ctx);
    stats_base_addr += tbl_ctx.tabledepth;

    assert(stats_base_addr <  (stats_region_start +  stats_region_size));
    return CAPRI_OK;
}

static void capri_program_p4plus_table_mpu_pc_for_stage0(uint32_t pc)
{
    cap_top_csr_t & cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

    //stage 0 - pcr = rxdma
    cap_te_csr_t &te_csr = cap0.pcr.te[0];

    //stage0 table0
    te_csr.cfg_table_property[1].read();
    //VijayS to fill the argument - PC addr right shift by 6.
    te_csr.cfg_table_property[1].mpu_pc(pc >> 6);
    te_csr.cfg_table_property[1].mpu_pc_dyn(1);
    te_csr.cfg_table_property[1].addr_base(0);
    te_csr.cfg_table_property[1].write();
}

int capri_table_rw_init()
{

    char action_name[P4ACTION_NAME_MAX_LEN];
    char progname[P4ACTION_NAME_MAX_LEN];
    uint64_t capri_action_p4plus_asm_base;

    // !!!!!!
    // Before making this call, it is expected that
    // in HAL init sequence, p4pd_init() is already called..
    // !!!!!!
    /* 1. Create shadow memory and init to zero */
    g_shadow_sram = (capri_sram_shadow_mem_t*)CAPRI_CALLOC(1,
                                sizeof(capri_sram_shadow_mem_t));
    g_shadow_tcam = (capri_tcam_shadow_mem_t*)CAPRI_CALLOC(1,
                                sizeof(capri_tcam_shadow_mem_t));
    if (!g_shadow_sram || !g_shadow_tcam) {
        // TODO: Log erorr/trace
        capri_table_rw_cleanup();
        return CAPRI_FAIL;
    }

    log_file_fd = fopen("./capri_tbl_rw.log", "w+");

#ifndef P4PD_CLI
    // register hal cpu interface
    auto cpu_if = new cpu_hal_if("cpu", "all");
    cpu::access()->add_if("cpu_if", cpu_if);
    cpu::access()->set_cur_if_name("cpu_if");

    // Register at top level all MRL classes.
    cap_top_csr_t *cap0_ptr = new cap_top_csr_t("cap0");
    cap0_ptr->init(0);
    CAP_BLK_REG_MODEL_REGISTER(cap_top_csr_t, 0, 0, cap0_ptr);

    // Fill up table base address and action-pcs
    for (int i = P4TBL_ID_TBLMIN; i < P4TBL_ID_TBLMAX; i++) {
        snprintf(progname, P4ACTION_NAME_MAX_LEN, "%s%s", p4pd_tbl_names[i], ".bin");
        capri_program_to_base_addr("iris", progname, &capri_table_asm_base[i]);
        for (int j = 0; j < p4pd_get_max_action_id(i); j++) {
            p4pd_get_action_name(i, j, action_name);
            capri_program_label_to_offset("iris", progname, action_name, &capri_action_asm_base[i][j]);
            capri_action_asm_base[i][j] >>= 6; /* Action base is in byte and 64B aligned... */
            HAL_TRACE_DEBUG("Program-Name {}, Action-Name {}, Action-Pc {:#x}",
                            progname, action_name, capri_action_asm_base[i][j]);
        }
    }

    capri_program_to_base_addr((char *)"p4plus",
            (char *)"rxdma_stage0.bin",
            &capri_action_p4plus_asm_base);
    HAL_TRACE_DEBUG("Program-Name {}, Action-PC {:#x}\n", "p4plus",
            capri_action_p4plus_asm_base);

    capri_program_p4plus_table_mpu_pc_for_stage0((uint32_t) capri_action_p4plus_asm_base);

    /* Program all P4 table base MPU address in all stages. */
    capri_program_table_mpu_pc();
    capri_stats_region_init();
    hbm_mem_base_addr = (uint64_t)get_start_offset((char*)JP4_PRGM);
#endif
    return (CAPRI_OK);
}


void capri_table_rw_cleanup()
{
    if (g_shadow_sram) {
        CAPRI_FREE(g_shadow_sram);
    }
    if (g_shadow_tcam) {
        CAPRI_FREE(g_shadow_tcam);
    }
    g_shadow_sram = NULL;
    g_shadow_tcam = NULL;

    fclose(log_file_fd);
}


uint8_t capri_get_action_pc(uint32_t tableid, uint8_t actionid)
{
    return ((uint8_t)capri_action_asm_base[tableid][actionid]);
}

uint8_t capri_get_action_id(uint32_t tableid, uint8_t actionpc)
{
    for (int j = 0; j < p4pd_get_max_action_id(tableid); j++) {
        if (capri_action_asm_base[tableid][j] == actionpc) {
            return j;
        }
    }
    return (0xff);
}

static void capri_sram_entry_details_get(uint32_t tableid, uint32_t index,
                                         int *sram_row, int *entry_start_block,
                                         int *entry_end_block, int *entry_start_word)
{
    p4pd_table_properties_t       tbl_ctx, *tblctx = &tbl_ctx;

    p4pd_table_properties_get(tableid, &tbl_ctx);

    //TODO Temporary fix for preventing crash for overflow TCAMs. We need to translate the tableid
    //TODO to the tableid of the original hash table and add the size of that table to the index.
    if (tbl_ctx.is_oflow_table)
    {
        *sram_row = 0;
        *entry_start_word = 0;
        *entry_start_block = 0;
        *entry_end_block = 0;
        return;
    }

    *sram_row = tblctx->sram_layout.top_left_y +
                         (index/tblctx->sram_layout.num_buckets);
    assert(*sram_row <= tblctx->sram_layout.btm_right_y);
    int tbl_col = index % tblctx->sram_layout.num_buckets;
    /* entry_width is in units of SRAM word  -- 16b */

    *entry_start_word = (tblctx->sram_layout.top_left_x + (tbl_col * tblctx->sram_layout.entry_width))
                        % CAPRI_SRAM_WORDS_PER_BLOCK;
    /* Capri 16b word within a 128b block is numbered from right to left.*/
    //*entry_start_word = (CAPRI_SRAM_WORDS_PER_BLOCK - 1) - *entry_start_word;

    *entry_start_block = (tblctx->sram_layout.top_left_block * CAPRI_SRAM_ROWS)
                         + (((tbl_col * tblctx->sram_layout.entry_width)
                              / CAPRI_SRAM_WORDS_PER_BLOCK) * CAPRI_SRAM_ROWS)
                         + tblctx->sram_layout.top_left_y
                         + (index/tblctx->sram_layout.num_buckets);

    *entry_end_block = *entry_start_block +
                       (((tblctx->sram_layout.entry_width - 1) +
                         (*entry_start_word % CAPRI_SRAM_WORDS_PER_BLOCK))
                        / CAPRI_SRAM_WORDS_PER_BLOCK) * CAPRI_SRAM_ROWS;

}

int capri_table_entry_write(uint32_t tableid,
                            uint32_t index,
                            uint8_t  *hwentry,
                            uint16_t hwentry_bit_len)
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
    p4pd_table_properties_t tbl_ctx;
    p4pd_table_properties_get(tableid, &tbl_ctx);
    assert(tbl_ctx.table_location != P4_TBL_LOCATION_HBM);
    capri_sram_entry_details_get(tableid, index,
                                 &sram_row, &entry_start_block,
                                 &entry_end_block, &entry_start_word);
    p4pd_table_dir_en dir = tbl_ctx.gress;
    int tbl_col = index % tbl_ctx.sram_layout.num_buckets;
    int blk = tbl_ctx.sram_layout.top_left_block
                 + ((tbl_col * tbl_ctx.sram_layout.entry_width) /
                     CAPRI_SRAM_WORDS_PER_BLOCK);
    int block = blk;
    int copy_bits = hwentry_bit_len;
    uint16_t *_hwentry = (uint16_t*)hwentry;
    for (int j = 0; j < tbl_ctx.sram_layout.entry_width; j++) {
        if (copy_bits >= 16) {
            g_shadow_sram->mem[sram_row][block % CAPRI_SRAM_BLOCK_COUNT][entry_start_word] = *_hwentry;
            _hwentry++;
            copy_bits -= 16;
        } else {
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

#ifndef P4PD_CLI
    cap_top_csr_t & cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    // Push to HW/Capri from entry_start_block to block
    pu_cpp_int<128> sram_block_data;
    uint8_t temp[16];
    for (int i = entry_start_block; i <= entry_end_block; i += CAPRI_SRAM_ROWS, blk++) {
        //all g_shadow_sram->mem[sram_row][i] to be pushed to capri..
        uint8_t *s = (uint8_t*)(g_shadow_sram->mem[sram_row][blk]);
        for (int p = 15; p >= 0; p--) {
            temp[p] = *s; s++;
        }
        if (dir == P4_GRESS_INGRESS) {
            cap_pics_csr_t & pics_csr = cap0.ssi.pics;
            sram_block_data = 0;
            pics_csr.hlp.s_cpp_int_from_array(sram_block_data, 0, 15, temp);
                                   // (uint8_t*)(g_shadow_sram->mem[sram_row][i % CAPRI_SRAM_BLOCK_COUNT]));
            pics_csr.dhs_sram.entry[i]
                        .data((pu_cpp_int<128>)sram_block_data);
            pics_csr.dhs_sram.entry[i].write();
        } else {
            cap_pics_csr_t & pics_csr = cap0.sse.pics;
            sram_block_data = 0;
            pics_csr.hlp.s_cpp_int_from_array(sram_block_data, 0, 15, temp);
                                    //(uint8_t*)(g_shadow_sram->mem[sram_row][i % CAPRI_SRAM_BLOCK_COUNT]));
            pics_csr.dhs_sram.entry[i]
                        .data((pu_cpp_int<128>)sram_block_data);
            pics_csr.dhs_sram.entry[i].write();
        }
    }
#endif

#ifdef HAL_LOG_TBL_UPDATES
    char    buffer[2048];
    memset(buffer, 0, sizeof(buffer));
    p4pd_table_entry_decoded_string_get(tableid,
                                        index,
                                        hwentry,
                                        NULL,
                                        hwentry_bit_len,
                                        buffer,
                                        sizeof(buffer));
    HAL_TRACE_DEBUG("{}", buffer);
#endif

    return (CAPRI_OK);
}


int capri_table_entry_read(uint32_t tableid,
                           uint32_t index,
                           uint8_t  *hwentry,
                           uint16_t *hwentry_bit_len)
{
    /*
     * Unswizzing of the bytes into readable format is
     * expected to be done by caller of the API.
     */

    int sram_row, entry_start_block, entry_end_block;
    int entry_start_word;
    p4pd_table_properties_t tbl_ctx;
    p4pd_table_properties_get(tableid, &tbl_ctx);
    assert(tbl_ctx.table_location != P4_TBL_LOCATION_HBM);
    capri_sram_entry_details_get(tableid, index,
                                 &sram_row, &entry_start_block,
                                 &entry_end_block, &entry_start_word);
    int tbl_col = index % tbl_ctx.sram_layout.num_buckets;
    int blk = tbl_ctx.sram_layout.top_left_block
                 + ((tbl_col * tbl_ctx.sram_layout.entry_width) /
                     CAPRI_SRAM_WORDS_PER_BLOCK);
    int block = blk;
    int copy_bits = tbl_ctx.sram_layout.entry_width_bits;
    uint16_t *_hwentry = (uint16_t*)hwentry;

    while(copy_bits) {
        if (copy_bits >= 16) {
            *_hwentry = g_shadow_sram->mem[sram_row][block % CAPRI_SRAM_BLOCK_COUNT][entry_start_word];
            _hwentry++;
            copy_bits -= 16;
        } else {
            if (copy_bits > 8) {
                *_hwentry = g_shadow_sram->mem[sram_row][block % CAPRI_SRAM_BLOCK_COUNT][entry_start_word];
            } else {
                *(uint8_t*)_hwentry =
                g_shadow_sram->mem[sram_row][block % CAPRI_SRAM_BLOCK_COUNT][entry_start_word];
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

    *hwentry_bit_len = tbl_ctx.sram_layout.entry_width_bits;

    // TODO : Read from HW is also needed... (not just returning from shadow)

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

static void capri_tcam_entry_details_get(uint32_t tableid, uint32_t index,
                                         int *tcam_row, int *entry_start_block,
                                         int *entry_end_block, int *entry_start_word)
{
    p4pd_table_properties_t       tbl_ctx, *tblctx = &tbl_ctx;

    p4pd_table_properties_get(tableid, &tbl_ctx);

    *tcam_row = tblctx->tcam_layout.top_left_y +
                         (index/tblctx->tcam_layout.num_buckets);
    assert(*tcam_row <= tblctx->tcam_layout.btm_right_y);
    int tbl_col = index % tblctx->tcam_layout.num_buckets;

    /* entry_width is in units of TCAM word  -- 16b */
    /* Since every tcam table entry occupies one TCAM block */
    *entry_start_word = tblctx->tcam_layout.start_index
                         % CAPRI_TCAM_WORDS_PER_BLOCK;
    /* Capri 16b word within a 128b block is numbered from right to left.*/
    //*entry_start_word = (CAPRI_TCAM_WORDS_PER_BLOCK - 1) - *entry_start_word;

    /* Start block will be column away from top-left because in case of
     * tcam, atmost one table can occupy a TCAM block.
     */
    *entry_start_block = ((tblctx->tcam_layout.top_left_block + tbl_col) * CAPRI_TCAM_ROWS)
                         + tblctx->tcam_layout.top_left_y
                         + (index/tblctx->tcam_layout.num_buckets);
    *entry_end_block = *entry_start_block
                         + (tblctx->tcam_layout.entry_width
                            / CAPRI_TCAM_WORDS_PER_BLOCK) * CAPRI_TCAM_ROWS;
}

int capri_tcam_table_entry_write(uint32_t tableid,
                                 uint32_t index,
                                 uint8_t  *trit_x,
                                 uint8_t  *trit_y,
                                 uint16_t hwentry_bit_len)
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
    p4pd_table_properties_t tbl_ctx;
    p4pd_table_properties_get(tableid, &tbl_ctx);
    assert(tbl_ctx.table_location != P4_TBL_LOCATION_HBM);

    capri_tcam_entry_details_get(tableid, index,
                                 &tcam_row, &entry_start_block,
                                 &entry_end_block, &entry_start_word);
    p4pd_table_dir_en dir = tbl_ctx.gress;
    int tbl_col = index % tbl_ctx.tcam_layout.num_buckets;
    int blk = tbl_ctx.tcam_layout.top_left_block
                 + ((tbl_col * tbl_ctx.tcam_layout.entry_width) /
                     CAPRI_TCAM_WORDS_PER_BLOCK);
    int block = blk;
    int copy_bits = hwentry_bit_len;
    uint16_t *_trit_x = (uint16_t*)trit_x;
    uint16_t *_trit_y = (uint16_t*)trit_y;
    for (int j = 0; j < tbl_ctx.tcam_layout.entry_width; j++) {
        if (copy_bits >= 16) {
            g_shadow_tcam->mem_x[tcam_row][block % CAPRI_TCAM_BLOCK_COUNT][entry_start_word] = *_trit_x;
            g_shadow_tcam->mem_y[tcam_row][block % CAPRI_TCAM_BLOCK_COUNT][entry_start_word] = *_trit_y;
            _trit_x++;
            _trit_y++;
            copy_bits -= 16;
        } else if (copy_bits) {
            if (copy_bits > 8) {
                g_shadow_tcam->mem_x[tcam_row][block % CAPRI_TCAM_BLOCK_COUNT][entry_start_word]
                    = (*_trit_x << (16 - copy_bits));
                g_shadow_tcam->mem_y[tcam_row][block % CAPRI_TCAM_BLOCK_COUNT][entry_start_word]
                    = (*_trit_y << (16 - copy_bits));
            } else {
                g_shadow_tcam->mem_x[tcam_row][block % CAPRI_TCAM_BLOCK_COUNT][entry_start_word]
                    = (*(uint8_t*)_trit_x << (16 - copy_bits));
                g_shadow_tcam->mem_y[tcam_row][block % CAPRI_TCAM_BLOCK_COUNT][entry_start_word]
                    = (*(uint8_t*)_trit_y << (16 - copy_bits));
            }
            copy_bits = 0;
        } else {
            // do not match remaining bits from end of entry bits to next 16b
            // aligned word
            g_shadow_tcam->mem_x[tcam_row][block % CAPRI_TCAM_BLOCK_COUNT][entry_start_word] = 0;
            g_shadow_tcam->mem_y[tcam_row][block % CAPRI_TCAM_BLOCK_COUNT][entry_start_word] = 0;
        }
        entry_start_word++;
        if (entry_start_word % CAPRI_TCAM_WORDS_PER_BLOCK == 0) {
            // crossed over to next block
            //block += CAPRI_TCAM_ROWS;
            block++;
            entry_start_word = 0;
        }
    }

#ifndef P4PD_CLI
    cap_top_csr_t & cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    // Push to HW/Capri from entry_start_block to block
    pu_cpp_int<128> tcam_block_data_x;
    pu_cpp_int<128> tcam_block_data_y;
    uint8_t temp_x[16];
    uint8_t temp_y[16];
    for (int i = entry_start_block; i <= entry_end_block; i += CAPRI_TCAM_ROWS, blk++) {
        uint8_t *s = (uint8_t*)(g_shadow_tcam->mem_x[tcam_row][blk]);
        for (int p = 15; p >= 0; p--) {
            temp_x[p] = *s; s++;
        }
        s = (uint8_t*)(g_shadow_tcam->mem_y[tcam_row][blk]);
        for (int p = 15; p >= 0; p--) {
            temp_y[p] = *s; s++;
        }
        if (dir == P4_GRESS_INGRESS) {
            cap_pict_csr_t & pict_csr = cap0.tsi.pict;
            tcam_block_data_x = 0;
            tcam_block_data_y = 0;
            pict_csr.hlp.s_cpp_int_from_array(tcam_block_data_x, 0, 15, temp_x);
                                    //(uint8_t*)(g_shadow_tcam->mem_x[tcam_row][i % CAPRI_TCAM_BLOCK_COUNT]));
            pict_csr.hlp.s_cpp_int_from_array(tcam_block_data_y, 0, 15, temp_y);
                                    //(uint8_t*)(g_shadow_tcam->mem_y[tcam_row][i % CAPRI_TCAM_BLOCK_COUNT]));
            pict_csr.dhs_tcam_xy.entry[i].x((pu_cpp_int<128>)tcam_block_data_x);
            pict_csr.dhs_tcam_xy.entry[i].y((pu_cpp_int<128>)tcam_block_data_y);
            pict_csr.dhs_tcam_xy.entry[i].valid(1);
            pict_csr.dhs_tcam_xy.entry[i].write();
        } else {
            cap_pict_csr_t & pict_csr = cap0.tse.pict;
            tcam_block_data_x = 0;
            tcam_block_data_y = 0;
            pict_csr.hlp.s_cpp_int_from_array(tcam_block_data_x, 0, 15, temp_x);
                                    //(uint8_t*)(g_shadow_tcam->mem_x[tcam_row][i % CAPRI_TCAM_BLOCK_COUNT]));
            pict_csr.hlp.s_cpp_int_from_array(tcam_block_data_y, 0, 15, temp_y);
                                    //(uint8_t*)(g_shadow_tcam->mem_y[tcam_row][i % CAPRI_TCAM_BLOCK_COUNT]));
            pict_csr.dhs_tcam_xy.entry[i].x((pu_cpp_int<128>)tcam_block_data_x);
            pict_csr.dhs_tcam_xy.entry[i].y((pu_cpp_int<128>)tcam_block_data_y);
            pict_csr.dhs_tcam_xy.entry[i].valid(1);
            pict_csr.dhs_tcam_xy.entry[i].write();
        }
    }
#endif


#ifdef HAL_LOG_TBL_UPDATES
    char    buffer[2048];
    memset(buffer, 0, sizeof(buffer));
    p4pd_table_entry_decoded_string_get(tableid,
                                        index,
                                        trit_x,
                                        trit_y,
                                        hwentry_bit_len,
                                        buffer,
                                        sizeof(buffer));
    HAL_TRACE_DEBUG("{}", buffer);
#endif


    return (CAPRI_OK);
}



int capri_tcam_table_entry_read(uint32_t tableid,
                                uint32_t index,
                                uint8_t  *trit_x,
                                uint8_t  *trit_y,
                                uint16_t *hwentry_bit_len)
{

    int tcam_row, entry_start_block, entry_end_block;
    int entry_start_word;

    p4pd_table_properties_t tbl_ctx;
    p4pd_table_properties_get(tableid, &tbl_ctx);
    assert(tbl_ctx.table_location != P4_TBL_LOCATION_HBM);
    capri_tcam_entry_details_get(tableid, index,
                                &tcam_row, &entry_start_block,
                                &entry_end_block, &entry_start_word);
    int tbl_col = index % tbl_ctx.tcam_layout.num_buckets;
    int blk = tbl_ctx.tcam_layout.top_left_block
                 + ((tbl_col * tbl_ctx.tcam_layout.entry_width) /
                     CAPRI_TCAM_WORDS_PER_BLOCK);
    int block = blk;
    int copy_bits = tbl_ctx.tcam_layout.entry_width_bits;
    int start_word = entry_start_word;
    uint16_t *_trit_x = (uint16_t*)trit_x;
    uint16_t *_trit_y = (uint16_t*)trit_y;
    while(copy_bits) {
        if (copy_bits >= 16) {
            *_trit_x = g_shadow_tcam->mem_x[tcam_row][block%CAPRI_TCAM_BLOCK_COUNT][start_word];
            *_trit_y = g_shadow_tcam->mem_y[tcam_row][block%CAPRI_TCAM_BLOCK_COUNT][start_word];
            _trit_x++;
            _trit_y++;
            copy_bits -= 16;
        } else if (copy_bits) {
            if (copy_bits > 8) {
                *_trit_x = g_shadow_tcam->mem_x[tcam_row][block%CAPRI_TCAM_BLOCK_COUNT][start_word];
                *_trit_y = g_shadow_tcam->mem_y[tcam_row][block%CAPRI_TCAM_BLOCK_COUNT][start_word];
            } else {
                *(uint8_t*)_trit_x = g_shadow_tcam->mem_x[tcam_row][block%CAPRI_TCAM_BLOCK_COUNT][start_word];
                *(uint8_t*)_trit_y = g_shadow_tcam->mem_y[tcam_row][block%CAPRI_TCAM_BLOCK_COUNT][start_word];
            }
            copy_bits = 0;
        }
        start_word++;
        if (start_word % CAPRI_TCAM_WORDS_PER_BLOCK == 0) {
            // crossed over to next block
            block++;
            //block += CAPRI_TCAM_ROWS;
            start_word = 0;
        }
    }

    *hwentry_bit_len = tbl_ctx.tcam_layout.entry_width_bits;;

    // TODO : Read from HW is also needed... (not just returning from shadow)

    return (CAPRI_OK);
}




int capri_hbm_table_entry_write(uint32_t tableid,
                                uint32_t index,
                                uint8_t *hwentry,
                                uint16_t entry_size)
{
    p4pd_table_properties_t       tbl_ctx;
    p4pd_table_properties_get(tableid, &tbl_ctx);


    assert((entry_size >> 3) <= tbl_ctx.hbm_layout.entry_width);
    assert(index < tbl_ctx.tabledepth);

    uint64_t entry_start_addr = tbl_ctx.hbm_layout.start_index
                                + (index * tbl_ctx.hbm_layout.entry_width);


#ifndef P4PD_CLI
    write_mem(hbm_mem_base_addr + entry_start_addr, hwentry, (entry_size >> 3));
#endif
#ifdef HAL_LOG_TBL_UPDATES
    char    buffer[2048];
    memset(buffer, 0, sizeof(buffer));
    p4pd_table_entry_decoded_string_get(tableid,
                                        index,
                                        hwentry,
                                        NULL,
                                        entry_size,
                                        buffer,
                                        sizeof(buffer));
    HAL_TRACE_DEBUG("{}", buffer);
#endif

    return (CAPRI_OK);
}

int capri_hbm_table_entry_read(uint32_t tableid,
                               uint32_t index,
                               uint8_t *hwentry,
                               uint16_t *entry_size)
{
    p4pd_table_properties_t       tbl_ctx;
    p4pd_table_properties_get(tableid, &tbl_ctx);

    assert(index < tbl_ctx.tabledepth);

    uint64_t entry_start_addr = tbl_ctx.hbm_layout.start_index
                                + (index * tbl_ctx.hbm_layout.entry_width);
#ifndef P4PD_CLI
    read_mem(hbm_mem_base_addr + entry_start_addr, hwentry,
             tbl_ctx.hbm_layout.entry_width);
#endif
    *entry_size = tbl_ctx.hbm_layout.entry_width;
    return (CAPRI_OK);
}

int capri_table_constant_write(uint32_t tableid, uint64_t val)
{
    p4pd_table_properties_t       tbl_ctx;
    cap_top_csr_t & cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    p4pd_table_properties_get(tableid, &tbl_ctx);
    if (tbl_ctx.gress == P4_GRESS_INGRESS) {
        cap_te_csr_t &te_csr = cap0.sgi.te[tbl_ctx.stage];
        te_csr.cfg_table_mpu_const[tbl_ctx.stage_tableid].value(val);
        te_csr.cfg_table_mpu_const[tbl_ctx.stage_tableid].write();
    } else {
        cap_te_csr_t &te_csr = cap0.sge.te[tbl_ctx.stage];
        te_csr.cfg_table_mpu_const[tbl_ctx.stage_tableid].value(val);
        te_csr.cfg_table_mpu_const[tbl_ctx.stage_tableid].write();
    }
    return (CAPRI_OK);
}

int capri_table_constant_read(uint32_t tableid, uint64_t *val)
{
    p4pd_table_properties_t       tbl_ctx;
    cap_top_csr_t & cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    p4pd_table_properties_get(tableid, &tbl_ctx);
    if (tbl_ctx.gress == P4_GRESS_INGRESS) {
        cap_te_csr_t &te_csr = cap0.sgi.te[tbl_ctx.stage];
        te_csr.cfg_table_mpu_const[tbl_ctx.stage_tableid].read();
        *val = te_csr.cfg_table_mpu_const[tbl_ctx.stage_tableid].
            value().convert_to<uint64_t>();
    } else {
        cap_te_csr_t &te_csr = cap0.sge.te[tbl_ctx.stage];
        te_csr.cfg_table_mpu_const[tbl_ctx.stage_tableid].read();
        *val = te_csr.cfg_table_mpu_const[tbl_ctx.stage_tableid].
            value().convert_to<uint64_t>();
    }
    return (CAPRI_OK);
}
