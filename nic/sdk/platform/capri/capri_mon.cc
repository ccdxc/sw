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

static const hbmerrcause_t hbmerrcause_table[] = {
    {DDR_CSR_APB_CPU0_ECC_STAT_INT_ECC_1BIT_THRESH_PS0_BYTE_ADDRESS, "1BIT ECC(PS0) errors channel ="},
    {DDR_CSR_APB_CPU0_ECC_STAT_INT_ECC_1BIT_THRESH_PS1_BYTE_ADDRESS, "1BIT ECC(PS1) errors channel ="},
    {DDR_CSR_APB_CPU0_CA_PAR_ERR_STAT_CA_PARITY_ERROR_PS0_BYTE_ADDRESS, "AERR ECC(PS0) errors channel ="},
    {DDR_CSR_APB_CPU0_CA_PAR_ERR_STAT_CA_PARITY_ERROR_PS1_BYTE_ADDRESS, "AERR ECC(PS1) errors channel ="},
    {DDR_CSR_APB_CPU0_MC_BASE1_STAT_WRITE_DATA_PARITY_ERROR_PS0_BYTE_ADDRESS, "DERR WR ECC(PS0) errors channel ="},
    {DDR_CSR_APB_CPU0_MC_BASE1_STAT_WRITE_DATA_PARITY_ERROR_PS1_BYTE_ADDRESS, "DERR WR ECC(PS1) errors channel ="},
    {DDR_CSR_APB_CPU0_MC_BASE1_STAT_READ_DATA_PARITY_ERROR_PS0_BYTE_ADDRESS, "DERR RD ECC(PS0) errors channel ="},
    {DDR_CSR_APB_CPU0_MC_BASE1_STAT_READ_DATA_PARITY_ERROR_PS1_BYTE_ADDRESS, "DERR RD ECC(PS1) errors channel ="},
};

static const uint32_t slave_err_reg[] = {
    NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_CCC0_S_24_264_AS_ERR_BYTE_ADDRESS,
    NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_CCC1_S_25_296_AS_ERR_BYTE_ADDRESS,
    NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_CCC2_S_32_328_AS_ERR_BYTE_ADDRESS,
    NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_CCC3_S_33_360_AS_ERR_BYTE_ADDRESS,
    NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_CCC4_S_34_392_AS_ERR_BYTE_ADDRESS,
    NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_CCC5_S_35_424_AS_ERR_BYTE_ADDRESS,
    NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_CCC6_S_36_456_AS_ERR_BYTE_ADDRESS,
    NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_CCC7_S_37_488_AS_ERR_BYTE_ADDRESS,
    NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_CCC8_S_38_520_AS_ERR_BYTE_ADDRESS,
    NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_CCC9_S_39_552_AS_ERR_BYTE_ADDRESS,
    NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_CCC10_S_26_584_AS_ERR_BYTE_ADDRESS,
    NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_CCC11_S_27_616_AS_ERR_BYTE_ADDRESS,
    NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_CCC12_S_28_648_AS_ERR_BYTE_ADDRESS,
    NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_CCC13_S_29_680_AS_ERR_BYTE_ADDRESS,
    NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_CCC14_S_30_712_AS_ERR_BYTE_ADDRESS,
    NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_CCC15_S_31_744_AS_ERR_BYTE_ADDRESS,
};

static const uint32_t master_err_reg[] = {
    NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_RBM_M_58_744_AM_ERR_BYTE_ADDRESS,
    NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_AR_M_0_809_AM_ERR_BYTE_ADDRESS,
    NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_MD_M_1_138_AM_ERR_BYTE_ADDRESS,
    NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_MS_M_2_774_AM_ERR_BYTE_ADDRESS,
    NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PB_M_3_150_AM_ERR_BYTE_ADDRESS,
    NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PR_M_4_141_AM_ERR_BYTE_ADDRESS,
    NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_BYTE_ADDRESS,
    NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PX_M_6_786_AM_ERR_BYTE_ADDRESS,
    NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_SS_M_7_396_AM_ERR_BYTE_ADDRESS,
};

static bool
read_hbmerrcause_table(const hbmerrcause_t *entry, uint64_t nwl_base_addr,
                       uint8_t channel, bool logging)
{
    uint32_t reg_value = 0;
    sdk::asic::asic_reg_read(nwl_base_addr + entry->offset, &reg_value, 1, true);
    if (reg_value && logging) {
        SDK_OBFL_TRACE_ERR("ECCERR::reading register %x value is %d %s %d",
                            nwl_base_addr + entry->offset, reg_value, entry->message.c_str(), channel);
        return true;
    }
    return false;
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
    for (uint32_t counter = 0; counter < ARRAY_SIZE(mc_sta_reg_value); counter++) {
        sdk::asic::asic_reg_read(mc_base_addr + CAP_MC_CSR_MCH_MC_STA_MC_STA_0_7_BYTE_ADDRESS +
                                 (counter * MC_STA_OFFSET), &mc_sta_reg_value[counter], 1, true);
    }
    // Data from STA 0
    if (mc_sta_reg_value[0]) {
        SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_1BIT_PS0: %d",
            CAP_MCH_CSR_MC_STA_MC_STA_0_7_ECC_ERROR_1BIT_PS0_GET(mc_sta_reg_value[0]));
        SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_2BIT_PS0: %d",
            CAP_MCH_CSR_MC_STA_MC_STA_0_7_ECC_ERROR_2BIT_PS0_GET(mc_sta_reg_value[0]));
        SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_POS_PS0_23_0: %d",
            CAP_MCH_CSR_MC_STA_MC_STA_0_7_ECC_ERROR_POS_PS0_23_0_GET(mc_sta_reg_value[0]));
    }

    // Data from STA 1
    if (mc_sta_reg_value[1]) {
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
    }

    // Data from STA 2
    if (mc_sta_reg_value[2]) {
        SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_POS_PS1_27_15: %d",
            CAP_MCH_CSR_MC_STA_MC_STA_2_7_ECC_ERROR_POS_PS1_27_15_GET(mc_sta_reg_value[2]));
        SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_ADDR_PS1: %d",
            CAP_MCH_CSR_MC_STA_MC_STA_2_7_ECC_ERROR_ADDR_PS1_GET(mc_sta_reg_value[2]));
        SDK_OBFL_TRACE_ERR("ECCERR::ECC_RMW_ERROR_PS1: %d",
            CAP_MCH_CSR_MC_STA_MC_STA_2_7_ECC_RMW_ERROR_PS1_GET(mc_sta_reg_value[2]));
        SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_CNT_1BIT_PS0_13_0: %d",
            CAP_MCH_CSR_MC_STA_MC_STA_2_7_ECC_ERROR_CNT_1BIT_PS0_13_0_GET(mc_sta_reg_value[2]));
    }

    // Data from STA 3
    if (mc_sta_reg_value[3]) {
        SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_CNT_1BIT_PS0_31_14: %d",
            CAP_MCH_CSR_MC_STA_MC_STA_3_7_ECC_ERROR_CNT_1BIT_PS0_31_14_GET(mc_sta_reg_value[3]));
        SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_CNT_1BIT_PS1_13_0: %d",
            CAP_MCH_CSR_MC_STA_MC_STA_3_7_ECC_ERROR_CNT_1BIT_PS1_13_0_GET(mc_sta_reg_value[3]));
    }

    // Data from STA 4
    if (mc_sta_reg_value[4]) {
        SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_CNT_1BIT_PS1_31_14: %d",
            CAP_MCH_CSR_MC_STA_MC_STA_4_7_ECC_ERROR_CNT_1BIT_PS1_31_14_GET(mc_sta_reg_value[4]));
        SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_CNT_2BIT_PS0_13_0: %d",
            CAP_MCH_CSR_MC_STA_MC_STA_4_7_ECC_ERROR_CNT_2BIT_PS0_13_0_GET(mc_sta_reg_value[4]));
    }

    // Data from STA 5
    if (mc_sta_reg_value[5]) {
        SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_CNT_2BIT_PS0_31_14: %d",
            CAP_MCH_CSR_MC_STA_MC_STA_5_7_ECC_ERROR_CNT_2BIT_PS0_31_14_GET(mc_sta_reg_value[5]));
        SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_CNT_2BIT_PS1_13_0: %d",
            CAP_MCH_CSR_MC_STA_MC_STA_5_7_ECC_ERROR_CNT_2BIT_PS1_13_0_GET(mc_sta_reg_value[5]));
    }

    // Data from STA 6
    if (mc_sta_reg_value[6]) {
        SDK_OBFL_TRACE_ERR("ECCERR::ECC_ERROR_CNT_2BIT_PS1_31_14: %d",
            CAP_MCH_CSR_MC_STA_MC_STA_6_7_ECC_ERROR_CNT_2BIT_PS1_31_14_GET(mc_sta_reg_value[6]));
    }
}

static bool
print_slave_err_regs(void)
{
    uint32_t slave_err_reg_value;
    uint32_t slave_err_reg_reset = NOC_REGISTER_RESET_VALUE;
    bool eccerr = false;
    for (uint32_t t_offset = 0; t_offset < ARRAY_SIZE(slave_err_reg); t_offset++) {
        sdk::asic::asic_reg_read(CAP_ADDR_BASE_MS_RBM_OFFSET + slave_err_reg[t_offset],
            &slave_err_reg_value, 1, true);
        // Data from slave error registers
        if (slave_err_reg_value & ~(NOC_REGISTER_RESET_VALUE)) {
            SDK_OBFL_TRACE_ERR("ECCERR::Slave error register 0x%x value is : 0x%x",
                CAP_ADDR_BASE_MS_RBM_OFFSET + slave_err_reg[t_offset], slave_err_reg_value);
            sdk::asic::asic_reg_write(CAP_ADDR_BASE_MS_RBM_OFFSET + slave_err_reg[t_offset],
                &slave_err_reg_reset, 1, ASIC_WRITE_MODE_WRITE_THRU);
            eccerr = true;
        }
    }
    return eccerr;
}

static bool
print_master_err_regs(void)
{
    uint32_t master_err_reg_value;
    uint32_t master_err_reg_reset = 0;
    bool eccerr = false;
    for (uint32_t t_offset = 0; t_offset < ARRAY_SIZE(master_err_reg); t_offset++) {
        sdk::asic::asic_reg_read(CAP_ADDR_BASE_MS_RBM_OFFSET + master_err_reg[t_offset],
                                 &master_err_reg_value, 1, true);
        // Data from slave error registers
        if (master_err_reg_value) {
            SDK_OBFL_TRACE_ERR("ECCERR::Master error register 0x%x value is : 0x%x",
                CAP_ADDR_BASE_MS_RBM_OFFSET + master_err_reg[t_offset], master_err_reg_value);
            sdk::asic::asic_reg_write(CAP_ADDR_BASE_MS_RBM_OFFSET + master_err_reg[t_offset],
                &master_err_reg_reset, 1, ASIC_WRITE_MODE_WRITE_THRU);
            eccerr = true;
        }
    }
    return eccerr;
}


sdk_ret_t capri_unravel_hbm_intrs(bool *iscattrip, bool *iseccerr, bool logging)
{
    uint64_t nwl_base_addr;
    uint64_t mc_base_addr;
    bool cattrip = false;
    bool eccerr = false;
    for (uint8_t counter = 0; counter < MAX_CHANNEL; counter++)
    {
        nwl_base_addr = CAP_NWL_ADDR(counter);
        mc_base_addr = CAP_MC_ADDR(counter);
        for(uint32_t t_offset = 0; t_offset < ARRAY_SIZE(hbmerrcause_table); t_offset++)
        {
            eccerr |= read_hbmerrcause_table(&hbmerrcause_table[t_offset], nwl_base_addr, counter, logging);
        }
        // if it not a cattrip print all the supporting registers
        if (logging) {
            print_mch_sta_data(mc_base_addr , counter);
        }
        cattrip = cattrip | read_cattrip_reg(nwl_base_addr, counter);
    }
    if (logging) {
        eccerr |= print_slave_err_regs();
        eccerr |= print_master_err_regs();
    }
    *iscattrip = cattrip;
    *iseccerr = eccerr;
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
