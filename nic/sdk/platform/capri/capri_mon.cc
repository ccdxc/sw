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
read_cattrip_table(cattrip_t *entry, uint64_t nwl_base_addr, uint8_t channel)
{
    uint32_t reg_value = 0;
    sdk::asic::asic_reg_read(nwl_base_addr + entry->offset, &reg_value, 1, true);
    if (reg_value) {
        SDK_OBFL_TRACE_ERR("ECCERR::reading register %x value is %d %s %d",
                            nwl_base_addr + entry->offset, reg_value, entry->message.c_str(), channel);
    }
}

static bool
read_cattrip_reg(uint64_t nwl_base_addr, uint8_t channel)
{
    uint32_t reg_value = 0;
    sdk::asic::asic_reg_read(nwl_base_addr +
            DDR_CSR_APB_CPU0_DFI_STAT_DFI_CATTRIP_BYTE_ADDRESS, &reg_value, 1, true);
    if (reg_value) {
        SDK_OBFL_TRACE_ERR("CRITCATTRIP::reading register %x value is %d %s %d", nwl_base_addr +
                            DDR_CSR_APB_CPU0_DFI_STAT_DFI_CATTRIP_BYTE_ADDRESS, reg_value,
                            "CRITICAL CATTRIP at channel =", channel);
        return true;
    }
    return false;
}

static void
print_mch_sta_data(uint64_t mc_base_addr, uint8_t channel)
{
    uint32_t mc_sta_reg_value[7];

    SDK_OBFL_TRACE_ERR("ECCERR::MCH_MC_STA channel %d", channel);
    for (int counter = 0; counter < ARRAY_SIZE(mc_sta_reg_value); counter++) {
        sdk::asic::asic_reg_read(mc_base_addr + CAP_MC_CSR_MCH_MC_STA_MC_STA_0_7_BYTE_ADDRESS +
                                 (counter * MC_STA_OFFSET), &mc_sta_reg_value[counter], 1, true);
    }
    // Data from STA 0
    SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_1BIT_PS0: %d",
        CAP_MCH_CSR_MC_STA_MC_STA_0_7_ECC_ERROR_1BIT_PS0_GET(mc_sta_reg_value[0]));
    SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_2BIT_PS0: %d",
        CAP_MCH_CSR_MC_STA_MC_STA_0_7_ECC_ERROR_2BIT_PS0_GET(mc_sta_reg_value[0]));
    SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_POS_PS0_23_0: %d",
        CAP_MCH_CSR_MC_STA_MC_STA_0_7_ECC_ERROR_POS_PS0_23_0_GET(mc_sta_reg_value[0]));

    // Data from STA 1
    SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_ADDR_PS0: %d",
        CAP_MCH_CSR_MC_STA_MC_STA_1_7_ECC_ERROR_POS_PS0_27_24_GET(mc_sta_reg_value[1]));
    SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_2BIT_PS0: %d",
        CAP_MCH_CSR_MC_STA_MC_STA_1_7_ECC_ERROR_ADDR_PS0_GET(mc_sta_reg_value[1]));
    SDK_OBFL_TRACE_ERR("ECCERR::ECC_RMW_ERROR_PS0: %d",
        CAP_MCH_CSR_MC_STA_MC_STA_1_7_ECC_RMW_ERROR_PS0_GET(mc_sta_reg_value[1]));
    SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_1BIT_PS1: %d",
        CAP_MCH_CSR_MC_STA_MC_STA_1_7_ECC_ERROR_1BIT_PS1_GET(mc_sta_reg_value[1]));
    SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_2BIT_PS1: %d",
        CAP_MCH_CSR_MC_STA_MC_STA_1_7_ECC_ERROR_2BIT_PS1_GET(mc_sta_reg_value[1]));
    SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_POS_PS1_14_0: %d",
        CAP_MCH_CSR_MC_STA_MC_STA_1_7_ECC_ERROR_POS_PS1_14_0_GET(mc_sta_reg_value[1]));

    // Data from STA 2
    SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_POS_PS1_27_15: %d",
        CAP_MCH_CSR_MC_STA_MC_STA_2_7_ECC_ERROR_POS_PS1_27_15_GET(mc_sta_reg_value[2]));
    SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_ADDR_PS1: %d",
        CAP_MCH_CSR_MC_STA_MC_STA_2_7_ECC_ERROR_ADDR_PS1_GET(mc_sta_reg_value[2]));
    SDK_OBFL_TRACE_ERR("ECCERR::ECC_RMW_ERROR_PS1: %d",
        CAP_MCH_CSR_MC_STA_MC_STA_2_7_ECC_RMW_ERROR_PS1_GET(mc_sta_reg_value[2]));
    SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_CNT_1BIT_PS0_13_0: %d",
        CAP_MCH_CSR_MC_STA_MC_STA_2_7_ECC_ERROR_CNT_1BIT_PS0_13_0_GET(mc_sta_reg_value[2]));

    // Data from STA 3
    SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_CNT_1BIT_PS0_31_14: %d",
        CAP_MCH_CSR_MC_STA_MC_STA_3_7_ECC_ERROR_CNT_1BIT_PS0_31_14_GET(mc_sta_reg_value[3]));
    SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_CNT_1BIT_PS1_13_0: %d",
        CAP_MCH_CSR_MC_STA_MC_STA_3_7_ECC_ERROR_CNT_1BIT_PS1_13_0_GET(mc_sta_reg_value[3]));

    // Data from STA 4
    SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_CNT_1BIT_PS1_31_14: %d",
        CAP_MCH_CSR_MC_STA_MC_STA_4_7_ECC_ERROR_CNT_1BIT_PS1_31_14_GET(mc_sta_reg_value[4]));
    SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_CNT_2BIT_PS0_13_0: %d",
        CAP_MCH_CSR_MC_STA_MC_STA_4_7_ECC_ERROR_CNT_2BIT_PS0_13_0_GET(mc_sta_reg_value[4]));

    // Data from STA 5
    SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_CNT_2BIT_PS0_31_14: %d",
        CAP_MCH_CSR_MC_STA_MC_STA_5_7_ECC_ERROR_CNT_2BIT_PS0_31_14_GET(mc_sta_reg_value[5]));
    SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_CNT_2BIT_PS1_13_0: %d",
        CAP_MCH_CSR_MC_STA_MC_STA_5_7_ECC_ERROR_CNT_2BIT_PS1_13_0_GET(mc_sta_reg_value[5]));

    // Data from STA 6
    SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_CNT_2BIT_PS1_31_14: %d",
        CAP_MCH_CSR_MC_STA_MC_STA_6_7_ECC_ERROR_CNT_2BIT_PS1_31_14_GET(mc_sta_reg_value[6]));
}

sdk_ret_t capri_unravel_hbm_intrs(bool *iscattrip)
{
    uint64_t nwl_base_addr;
    uint64_t mc_base_addr;
    bool cattrip = false;
    for (uint8_t counter = 0; counter < MAX_CHANNEL; counter++)
    {
        nwl_base_addr = CAP_NWL_ADDR(counter);
        mc_base_addr = CAP_MC_ADDR(counter);
        for(uint32_t t_offset = 0; t_offset < ARRAY_SIZE(cattrip_table); t_offset++)
        {
            read_cattrip_table(&cattrip_table[t_offset], nwl_base_addr, counter);
        }
        print_mch_sta_data(mc_base_addr , counter);
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
