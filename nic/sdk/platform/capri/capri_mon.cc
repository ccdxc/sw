// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

/*
 * capri_mon.cc
 * Rahul Shekhar (Pensando Systems)
 */
#include "platform/capri/capri_mon.hpp"
#include "asic/pd/pd.hpp"

namespace sdk {
namespace platform {
namespace capri {

cattrip_t cattrip_table[] = {
    {DDR_CSR_APB_CPU0_ECC_STAT_INT_ECC_1BIT_THRESH_PS0_BYTE_ADDRESS, "1BIT ECC(PS0) errors channel ="},
    {DDR_CSR_APB_CPU0_ECC_STAT_INT_ECC_1BIT_THRESH_PS1_BYTE_ADDRESS, "1BIT ECC(PS1) errors channel ="},
    {DDR_CSR_APB_CPU0_CA_PAR_ERR_STAT_CA_PARITY_ERROR_PS0_BYTE_ADDRESS, "AERR ECC(PS0) errors channel ="},
    {DDR_CSR_APB_CPU0_CA_PAR_ERR_STAT_CA_PARITY_ERROR_PS1_BYTE_ADDRESS, "AERR ECC(PS1) errors channel ="},
    {DDR_CSR_APB_CPU0_MC_BASE1_STAT_WRITE_DATA_PARITY_ERROR_PS0_BYTE_ADDRESS, "DERR WR ECC(PS0) errors channel ="},
    {DDR_CSR_APB_CPU0_MC_BASE1_STAT_WRITE_DATA_PARITY_ERROR_PS1_BYTE_ADDRESS, "DERR WR ECC(PS1) errors channel ="},
    {DDR_CSR_APB_CPU0_MC_BASE1_STAT_READ_DATA_PARITY_ERROR_PS0_BYTE_ADDRESS, "DERR RD ECC(PS0) errors channel ="},
    {DDR_CSR_APB_CPU0_MC_BASE1_STAT_READ_DATA_PARITY_ERROR_PS1_BYTE_ADDRESS, "DERR RD ECC(PS1) errors channel ="},
};

static void
read_cattrip_table(cattrip_t *entry, uint64_t nwl_base_addr, uint8_t t_offset)
{
    uint32_t reg_value = 0;
    sdk::asic::asic_reg_read(nwl_base_addr + entry->offset, &reg_value, 1, true);
    if (reg_value) {
        SDK_OBFL_TRACE_ERR("ECCERR::reading register %x %s %d",
                            nwl_base_addr + entry->offset, entry->message.c_str(), t_offset);
    }
}

static bool
read_cattrip_reg(uint64_t nwl_base_addr, uint8_t channel)
{
    uint32_t reg_value = 0;
    sdk::asic::asic_reg_read(nwl_base_addr +
            DDR_CSR_APB_CPU0_DFI_STAT_DFI_CATTRIP_BYTE_ADDRESS, &reg_value, 1, true);
    if (reg_value) {
        SDK_OBFL_TRACE_ERR("CRITCATTRIP::reading register %x %s %d", nwl_base_addr +
                            DDR_CSR_APB_CPU0_DFI_STAT_DFI_CATTRIP_BYTE_ADDRESS,
                            "CRITICAL CATTRIP at channel =", channel);
        return true;
    }
    return false;
}

sdk_ret_t capri_unravel_hbm_intrs(bool *iscattrip)
{
    uint64_t nwl_base_addr;
    bool cattrip = false;
    for (uint8_t counter = 0; counter < MAX_CHANNEL; counter++)
    {
        nwl_base_addr = CAP_NWL_ADDR(counter);
        for(uint32_t t_offset = 0; t_offset < ARRAY_SIZE(cattrip_table); t_offset++)
        {
            read_cattrip_table(&cattrip_table[t_offset], nwl_base_addr, t_offset);
        }
        cattrip = cattrip | read_cattrip_reg(nwl_base_addr, counter);
    }
    *iscattrip = cattrip;
    return SDK_RET_OK;
}

pen_adjust_perf_status_t capri_adjust_perf(int chip_id, int inst_id,
                                          pen_adjust_index_t &idx,
                                          pen_adjust_perf_type_t perf_type)
{
    return cap_top_adjust_perf(chip_id, inst_id, idx, perf_type);
}

void capri_set_half_clock(int chip_id, int inst_id)
{
    return cap_top_set_half_clock(chip_id, inst_id);
}

} // namespace capri
} // namespace platform
} // namespace sdk
