/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    lpm_sport.hpp
 *
 * @brief   LPM source port implementation
 */

#if !defined (__LPM_SPORT_HPP__)
#define __LPM_SPORT_HPP__

#include "nic/apollo/api/impl/lpm/lpm.hpp"

#define LPM_SPORT_KEY_SIZE (2)

sdk_ret_t lpm_sport_add_key_to_stage(uint8_t *bytes, uint32_t idx, lpm_inode_t *lpm_inode);
sdk_ret_t lpm_sport_add_key_to_last_stage(uint8_t *bytes, uint32_t idx,
                                          lpm_inode_t *lpm_inode);
sdk_ret_t lpm_sport_set_default_data(uint8_t *bytes, uint32_t default_data);
sdk_ret_t lpm_sport_write_stage_table(mem_addr_t addr, uint8_t *bytes);
sdk_ret_t lpm_sport_write_last_stage_table(mem_addr_t addr, uint8_t *bytes);
uint32_t lpm_sport_key_size(void);
uint32_t lpm_sport_stages(uint32_t num_intrvls);

#endif    //__LPM_SPORT_HPP__
