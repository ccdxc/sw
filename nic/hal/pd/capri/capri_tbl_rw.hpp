/*
 * capri_tbl_rw.hpp
 * Mahesh Shirshyad (Pensando Systems)
 */

#ifndef __CAPRI_TBL_RW_HPP__
#define __CAPRI_TBL_RW_HPP__

#include <stdio.h>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#ifndef P4PD_CLI
#include "nic/include/base.h"
#endif

#define CAPRI_TIMER_WHEEL_DEPTH         4096

#define CAPRI_TIMER_NUM_KEY_PER_CACHE_LINE    16
#define CAPRI_TIMER_NUM_DATA_PER_CACHE_LINE   12

// This needs to be a power of 2
#define CAPRI_TIMER_NUM_KEY_CACHE_LINES 1024

// each line is 64B
// Each key in key line takes up 1 line in data space
#define CAPRI_TIMER_HBM_DATA_SPACE \
    (CAPRI_TIMER_NUM_KEY_CACHE_LINES * CAPRI_TIMER_NUM_KEY_PER_CACHE_LINE * 64)

#define CAPRI_TIMER_HBM_KEY_SPACE \
    (CAPRI_TIMER_NUM_KEY_CACHE_LINES * 64)

#define CAPRI_TIMER_HBM_SPACE \
    (CAPRI_TIMER_HBM_KEY_SPACE + CAPRI_TIMER_HBM_DATA_SPACE)

#define CAPRI_MAX_TIMERS \
    (CAPRI_TIMER_NUM_KEY_CACHE_LINES * CAPRI_TIMER_NUM_KEY_PER_CACHE_LINE * \
     CAPRI_TIMER_NUM_DATA_PER_CACHE_LINE)

#define CAPRI_P4_NUM_STAGES     6
#define CAPRI_P4PLUS_NUM_STAGES 8

#define CAPRI_OK (0)
#define CAPRI_FAIL (-1)

#define CACHE_LINE_SIZE_SHIFT   6
#define CACHE_LINE_SIZE         (1 << CACHE_LINE_SIZE_SHIFT)   
#define CACHE_LINE_SIZE_MASK    (CACHE_LINE_SIZE - 1)

typedef enum {
    P4PLUS_CACHE_ACTION_NONE        = 0x0,
    P4PLUS_CACHE_INVALIDATE_RXDMA   = 0x1,
    P4PLUS_CACHE_INVALIDATE_TXDMA   = 0x2,
    P4PLUS_CACHE_INVALIDATE_BOTH    = P4PLUS_CACHE_INVALIDATE_RXDMA |
                                      P4PLUS_CACHE_INVALIDATE_TXDMA
} p4plus_cache_action_t;

typedef struct capri_table_mem_layout_ {
    uint16_t    entry_width;    /* In units of memory words.. 16b  in case of PIPE tables */
                                /* In units of bytes in case of HBM table */
    uint16_t    entry_width_bits;
    uint32_t    start_index;
    uint32_t    end_index;
    uint16_t    top_left_x;
    uint16_t    top_left_y;
    uint8_t     top_left_block;
    uint16_t    btm_right_x;
    uint16_t    btm_right_y;
    uint8_t     btm_right_block;
    uint8_t     num_buckets;
    uint32_t    tabledepth;
    char        *tablename;
} capri_table_mem_layout_t;

int capri_table_rw_init();

void capri_table_rw_cleanup();

int capri_table_entry_write(uint32_t tableid,
                            uint32_t index,
                            uint8_t  *hwentry,
                            uint16_t hwentry_bit_len,
                            capri_table_mem_layout_t &tbl_info, int gress,
                            bool is_oflow_table, bool ingress,
                            uint32_t ofl_parent_tbl_depth);

int capri_table_entry_read(uint32_t tableid,
                           uint32_t index,
                           uint8_t  *hwentry,
                           uint16_t *hwentry_bit_len,
                           capri_table_mem_layout_t &tbl_info, int gress,
                           bool is_oflow_table,
                           uint32_t ofl_parent_tbl_depth);

int capri_table_hw_entry_read(uint32_t tableid,
                              uint32_t index,
                              uint8_t  *hwentry,
                              uint16_t *hwentry_bit_len,
                              capri_table_mem_layout_t &tbl_info, int gress,
                              bool is_oflow_table, bool ingress,
                              uint32_t ofl_parent_tbl_depth);

int capri_tcam_table_entry_write (uint32_t tableid,
                                  uint32_t index,
                                  uint8_t  *trit_x,
                                  uint8_t  *trit_y,
                                  uint16_t hwentry_bit_len,
                                  capri_table_mem_layout_t &tbl_info,
                                  int gress, bool ingress);

int capri_tcam_table_entry_read(uint32_t tableid,
                                uint32_t index,
                                uint8_t  *trit_x,
                                uint8_t  *trit_y,
                                uint16_t *hwentry_bit_len,
                                capri_table_mem_layout_t &tbl_info,
                                int gress);

int capri_tcam_table_hw_entry_read(uint32_t tableid,
                                   uint32_t index,
                                   uint8_t  *trit_x,
                                   uint8_t  *trit_y,
                                   uint16_t *hwentry_bit_len,
                                   capri_table_mem_layout_t &tbl_info,
                                   bool ingress);

int capri_hbm_table_entry_write(uint32_t tableid,
                                uint32_t index,
                                uint8_t *hwentry,
                                uint16_t entry_size,
                                capri_table_mem_layout_t &tbl_info);

int capri_hbm_table_entry_cache_invalidate (bool ingress,
                                            uint64_t entry_addr,
                                            capri_table_mem_layout_t &tbl_info);

int capri_hbm_table_entry_read(uint32_t tableid,
                               uint32_t index,
                               uint8_t *hwentry,
                               uint16_t *entry_size,
                                capri_table_mem_layout_t &tbl_info);

int capri_table_constant_write(uint64_t val, uint32_t stage,
                               uint32_t stage_tableid, bool ingress);

int capri_table_constant_read(uint64_t *val, uint32_t stage,
                              int stage_tableid, bool ingress);

void capri_set_action_asm_base(int tableid, int actionid,
                               uint64_t asm_base);

void capri_set_action_rxdma_asm_base(int tableid, int actionid,
                                     uint64_t asm_base);

void capri_set_action_txdma_asm_base(int tableid, int actionid,
                                     uint64_t asm_base);

void capri_set_table_rxdma_asm_base (int tableid, uint64_t asm_base);

void capri_set_table_txdma_asm_base (int tableid, uint64_t asm_base);

void capri_program_p4plus_sram_table_mpu_pc(int tableid, int stage_tbl_id,
                                            int stage);

void capri_program_table_mpu_pc(int tableid, bool gress, int stage, int stage_tableid,
                           uint64_t capri_table_asm_err_offset,
                           uint64_t capri_table_asm_base);

void capri_timer_init_helper(uint32_t key_lines);

int capri_toeplitz_init(int stage, int stage_tableid);

int capri_p4plus_table_init(int stage_apphdr, int stage_tableid_apphdr,
                            int stage_apphdr_ext, int stage_tableid_apphdr_ext,
                            int stage_apphdr_off, int stage_tableid_apphdr_off,
                            int stage_apphdr_ext_off, int stage_tableid_apphdr_ext_off,
                            int stage_txdma_act, int stage_tableid_txdma_act,
                            int stage_txdma_act_ext, int stage_tableid_txdma_act_ext);

void capri_deparser_init(int tm_port_ingress, int tm_port_egress);

void capri_program_hbm_table_base_addr(int stage_tableid, char *tablename,
                                       int stage, bool ingress);

void capri_p4plus_recirc_init();

void capri_timer_init();

uint8_t capri_get_action_id(uint32_t tableid, uint8_t actionpc);

uint8_t capri_get_action_pc(uint32_t tableid, uint8_t actionid);

bool p4plus_invalidate_cache(uint64_t addr, uint32_t size_in_bytes,
                             p4plus_cache_action_t action);

#endif
