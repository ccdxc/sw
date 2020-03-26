// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#ifndef __ASIC_HBM_HPP__
#define __ASIC_HBM_HPP__

#include "asic/asic.hpp"
#include "platform/utils/mpartition.hpp"
#include "include/sdk/types.hpp"

namespace sdk  {
namespace asic {

//void                 set_mem_partition(mpartition *mempartition);
mem_addr_t           asic_get_mem_base(void);
mem_addr_t           asic_get_mem_offset(const char *reg_name);
mem_addr_t           asic_get_mem_addr (const char *reg_name);
uint32_t             asic_get_mem_size_kb (const char *reg_name);
mpartition_region_t  *asic_get_mem_region(char *reg_name);
mpartition_region_t  *asic_get_hbm_region_by_address(uint64_t addr);
void                 asic_reset_hbm_regions(asic_cfg_t *asic_cfg);

}    // namespace asic 
}    // namespace sdk

using sdk::asic::asic_get_mem_base;
using sdk::asic::asic_get_mem_offset;
using sdk::asic::asic_get_mem_addr;
using sdk::asic::asic_get_mem_size_kb;
using sdk::asic::asic_get_mem_region;
using sdk::asic::asic_get_hbm_region_by_address;

#endif    // __ASIC_HBM_HPP__
