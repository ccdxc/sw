// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#include "lib/pal/pal.hpp"
#include "asic/cmn/asic_hbm.hpp"
#include "asic/rw/asicrw.hpp"
#include "asic/cmn/asic_state.hpp"

namespace sdk {
namespace asic {


static mpartition* 
get_mem_partition (void)
{
    return g_asic_state->mempartition();
}

asic_cfg_t *get_asic_cfg (void)
{
    return g_asic_state->asic_cfg();
}

mem_addr_t
asic_get_mem_base (void)
{
    return get_mem_partition()->base();
}

mem_addr_t
asic_get_mem_offset (const char *reg_name)
{
    return get_mem_partition()->start_offset(reg_name);
}

mem_addr_t
asic_get_mem_addr (const char *reg_name)
{
    return get_mem_partition()->start_addr(reg_name);
}

uint32_t
asic_get_mem_size_kb (const char *reg_name)
{
    return (get_mem_partition()->size(reg_name) >> 10 );
}

mpartition_region_t *
asic_get_mem_region (char *reg_name)
{
    return get_mem_partition()->region(reg_name);
}

mpartition_region_t *
asic_get_hbm_region_by_address (uint64_t addr)
{
    return get_mem_partition()->region_by_address(addr);
}

// for HW platform this is now done during uboot
void
asic_reset_hbm_regions (asic_cfg_t *asic_cfg)
{
    mpartition_region_t *reg;
    mem_addr_t va, pa;
    bool force_reset =
        sdk::platform::upgrade_mode_none(asic_cfg->upg_init_mode) ? false : true;

    if (!asic_cfg)
        return;

    if (!force_reset) {
        if (asic_cfg->platform != platform_type_t::PLATFORM_TYPE_HAPS &&
            asic_cfg->platform != platform_type_t::PLATFORM_TYPE_HW) {
            return;
        }
    }

    for (int i = 0; i < get_mem_partition()->num_regions(); i++) {
        reg = get_mem_partition()->region(i);
        if (reg->reset) {
            // Reset only for haps
            SDK_TRACE_DEBUG("Resetting %s hbm region", reg->mem_reg_name);

            pa = get_mem_partition()->addr(reg->start_offset);
            va = (mem_addr_t)sdk::lib::pal_mem_map(pa, reg->size);
            if (va) {
                memset((void *)va, 0, reg->size);
                sdk::lib::pal_mem_unmap((void *)va);
            } else {
                uint8_t zeros[1024] = {0};
                int64_t rem = reg->size;
                while (rem > 0) {
                    sdk::asic::asic_mem_write(pa, zeros, (uint64_t)rem > sizeof(zeros) ? sizeof(zeros) : rem);
                    pa += sizeof(zeros);
                    rem -= sizeof(zeros);
                }
            }
            SDK_TRACE_DEBUG("Resetting %s hbm region done", reg->mem_reg_name);
        }
    }
}

}    // asic
}    // sdk 
