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

int capri_table_rw_init();

void capri_table_rw_cleanup();

int capri_table_entry_write(uint32_t tableid,
                            uint32_t index,
                            uint8_t  *hwentry,
                            uint16_t hwentry_bit_len);


int capri_table_entry_read(uint32_t tableid,
                           uint32_t index,
                           uint8_t  *hwentry,
                           uint16_t *hwentry_bit_len);

int capri_table_hw_entry_read(uint32_t tableid,
                              uint32_t index,
                              uint8_t  *hwentry,
                              uint16_t *hwentry_bit_len);


int capri_tcam_table_entry_write(uint32_t tableid,
                                 uint32_t index,
                                 uint8_t  *trit_x,
                                 uint8_t  *trit_y,
                                 uint16_t hwentry_bit_len);

int capri_tcam_table_entry_read(uint32_t tableid,
                                uint32_t index,
                                uint8_t  *trit_x,
                                uint8_t  *trit_y,
                                uint16_t *hwentry_bit_len);

int capri_tcam_table_hw_entry_read(uint32_t tableid,
                                   uint32_t index,
                                   uint8_t  *trit_x,
                                   uint8_t  *trit_y,
                                   uint16_t *hwentry_bit_len);

int capri_hbm_table_entry_write(uint32_t tableid,
                                uint32_t index,
                                uint8_t *hwentry,
                                uint16_t entry_size);

int capri_hbm_table_entry_read(uint32_t tableid,
                               uint32_t index,
                               uint8_t *hwentry,
                               uint16_t *entry_size);

int capri_table_constant_write(uint64_t val, uint32_t stage,
                               uint32_t stage_tableid, bool ingress);

int capri_table_constant_read(uint64_t *val, uint32_t stage,
                              int stage_tableid, bool ingress);

void capri_timer_init_helper(uint32_t key_lines);

#endif
