#ifndef ELB_SBUS_API_H
#define ELB_SBUS_API_H

#include "pen_c_useful_wrp.h"
#ifndef _DISABLE_SKNOBS_
#include "sknobs.h"
#endif

/////////////////////////////////////////////////////////////////
// SBUS Procedures
/////////////////////////////////////////////////////////////////
#define ELB_SBUS_SBUS_RESET_REG                      0
#define ELB_SBUS_WIR_REG                             2
#define ELB_SBUS_CONTROL_1500                       16
#define ELB_SBUS_READ_CHANNEL                       17
#define ELB_SBUS_BUSY_DONE_1500_REG                 18
#define ELB_SBUS_APC_BIST_EN_REG                    30
#define ELB_SBUS_DFI_PHYLVL_REQ_N                   40
#define ELB_SBUS_APC_INIT_COMPLETE                  41
#define ELB_SBUS_DFI_PHYLVL_ACK_N                   42

#define ELB_SBUS_PHY_SEL                             0
#define ELB_SBUS_PHY_CONFIG_LENGTH                 104
#define ELB_SBUS_PHY_SOFT_LANE_REPAIR_LENGTH        72
#define ELB_SBUS_APC_ADDR                        0x01
#define ELB_SBUS_PHY_SOFT_LANE_REPAIR            0x12
#define ELB_SBUS_PHY_CONFIG                      0x14
#define ELB_SBUS_SBUS_RST                        0x20
#define ELB_SBUS_SBUS_WR                         0x21
#define ELB_SBUS_SBUS_RD                         0x22
#define ELB_SBUS_STOP_1500                       0x0 // 4'b0000
#define ELB_SBUS_SHIFT_READ_WDR                  0xc // 4'b1100
#define ELB_SBUS_START_WDR_READ                  0x4 // 4'b0100
#define ELB_SBUS_SHIFT_WRITE_WDR                 0x2 // 4'b1010
#define ELB_SBUS_START_WDR_WRITE                 0x2 // 4'b0010

#define ELB_SBUS_HBM_SEL                             1
#define ELB_SBUS_HBM_SOFT_LANE_REPAIR            0x12
#define ELB_SBUS_HBM_SOFT_LANE_REPAIR_LENGTH        72
#define ELB_SBUS_HBM_DEVICE_ID                   0x0e
#define ELB_SBUS_HBM_DEVICE_ID_LENGTH               82
#define ELB_SBUS_HBM_TEMPERATURE                 0x0f
#define ELB_SBUS_HBM_TEMPERATURE_LENGTH              8
#define ELB_SBUS_HBM_MODE_REG_DUMP_SET           0x10
#define ELB_SBUS_HBM_MODE_REG_DUMP_SET_LENGTH      128

//#define ELB_SBUS_SBUS_MSG "SBUS INFO"
#define ELB_SBUS_SBUS_MSG "SBUS FAST ACCESS"
#define ELB_SBUS_HBM_1500_MSG "(HBM 1500 FAST ACCESS)"


// SBUS PLL Register Map

//Chain 0
#define SBUS_CHAIN0_MASTER_ID    0x2
#define SBUS_CHAIN0_PMRO5_ID     0x2c
#define SBUS_CHAIN0_PLL_CPU0_ID	 0xa
#define SBUS_CHAIN0_PMRO6_ID     0x2c
#define SBUS_CHAIN0_PLL_CPU1_ID	 0xa
#define SBUS_CHAIN0_PMRO7_ID     0x2c
#define SBUS_CHAIN0_PLL_CPU2_ID	 0xa
#define SBUS_CHAIN0_PMRO8_ID     0x2c
#define SBUS_CHAIN0_PLL_CPU3_ID	 0xa

#define SBUS_CHAIN0_MASTER_ADDR    0
#define SBUS_CHAIN0_PMRO5_ADDR     1
#define SBUS_CHAIN0_PLL_CPU0_ADDR  2
#define SBUS_CHAIN0_PMRO6_ADDR     3
#define SBUS_CHAIN0_PLL_CPU1_ADDR  4
#define SBUS_CHAIN0_PMRO7_ADDR     5
#define SBUS_CHAIN0_PLL_CPU2_ADDR  6
#define SBUS_CHAIN0_PMRO8_ADDR     7
#define SBUS_CHAIN0_PLL_CPU3_ADDR  8

//Chain 1 (PCIE)

//Chain 2
#define SBUS_CHAIN2_MASTER_ID    0x2
#define SBUS_CHAIN2_VTMON_ID     0x1b
#define SBUS_CHAIN2_PLL_FLASH_ID 0xa
#define SBUS_CHAIN2_PLL_DDR_ID   0xa
#define SBUS_CHAIN2_PLL_STG_ID   0xa
#define SBUS_CHAIN2_PMRO0_ID 0xb
#define SBUS_CHAIN2_PMRO1_ID 0xb

#define SBUS_CHAIN2_MASTER_ADDR    0
#define SBUS_CHAIN2_VTMON_ADDR     1
#define SBUS_CHAIN2_PLL_FLASH_ADDR 2
#define SBUS_CHAIN2_PLL_DDR_ADDR   3
#define SBUS_CHAIN2_PLL_STG_ADDR   4
#define SBUS_CHAIN2_PMRO0_ADDR 5
#define SBUS_CHAIN2_PMRO1_ADDR 6

//Chain 3
#define SBUS_CHAIN3_MASTER_ID    0x2
#define SBUS_CHAIN3_VTMON_ID     0x1b
#define SBUS_CHAIN3_PLL_ETH_ID	 0xa
#define SBUS_CHAIN3_PLL_CORE_ID  0xa
#define SBUS_CHAIN3_PMRO2_ID	 0x2c
#define SBUS_CHAIN3_MX_ID	 	 0x1
#define SBUS_CHAIN3_RESCAL_ID	 0x1e
#define SBUS_CHAIN3_PMRO3_ID 	 0x2c
#define SBUS_CHAIN3_BX_ID 		 0x05
#define SBUS_CHAIN3_PMR4_ID 	 0x2c

#define SBUS_CHAIN3_MASTER_ADDR  	0
#define SBUS_CHAIN3_VTMON_ADDR   	1
#define SBUS_CHAIN3_PLL_ETH_ADDR 	2
#define SBUS_CHAIN3_PLL_CORE_ADDR	3
#define SBUS_CHAIN3_PMRO2_ADDR	 	4
#define SBUS_CHAIN3_MX_ADDR	 	 	5
#define SBUS_CHAIN3_RESCAL_ADDR	 	6
#define SBUS_CHAIN3_PMRO3_ADDR 	 	7
#define SBUS_CHAIN3_BX_ADDR 	 	8
#define SBUS_CHAIN3_PMRO4_ADDR 	 	9

// SBM -> VTMON -> plls flash-ddr-stg -> pmro 0-1 -> SBM 
// (STG pll is in PCIE wrapper, but logically still in this chain)
// SBUS Chain 3 :   int dev_idcode[7] = {0x2, 0x1b, 0xa, 0xa, 0xa, 0xb, 0xb };
//

#define ELB_SBUS_PLL_OVERRIDE_ADDR    0
#define ELB_SBUS_PLL_PWR_CONTROL_ADDR 1
#define ELB_SBUS_PLL_RESETS_ADDR      2
#define ELB_SBUS_PLL_DLL_CNTL_ADDR    3
#define ELB_SBUS_PLL_CNTL1_ADDR   4
#define ELB_SBUS_PLL_CNTL2_ADDR   5
#define ELB_SBUS_PLL_REFCLK_SEL_ADDR  6
#define ELB_SBUS_PLL_COMMON_POST_DIVIDER1_ADDR 7
#define ELB_SBUS_PLL_COMMON_POST_DIVIDER2_ADDR 8
#define ELB_SBUS_PLL_OUTPUTS_ADDR 9
#define ELB_SBUS_PLL_AUX_DIVIDER_ADDR 10
#define ELB_SBUS_PLL_DLL_DIVIDER_ADDR 11
#define ELB_SBUS_PLL_STANDARD_DIVIDER1_ADDR 12
#define ELB_SBUS_PLL_STANDARD_DIVIDER2_ADDR 13
#define ELB_SBUS_PLL_OTHER_1 14
#define ELB_SBUS_PLL_OTHER_2 15
#define ELB_SBUS_PLL_OTHER_3 16
#define ELB_SBUS_PLL_OTHER_4 17
#define ELB_SBUS_PLL_OTHER_5 18
#define ELB_SBUS_PLL_OTHER_6 19
#define ELB_SBUS_PLL_OTHER_7 20
#define ELB_SBUS_PLL_BROADCAST_ADDR 22

#define ELB_SBUS_PLL_OVRD_ENABLE              0
#define ELB_SBUS_PLL_RESETS_IRESETB           0
#define ELB_SBUS_PLL_RESETS_I_POST_RESETB     1
#define ELB_SBUS_PLL_DLL_CNTL_FB_EN           0
#define ELB_SBUS_PLL_CNTL1_PDIV               0x3f
#define ELB_SBUS_PLL_CNTL1_NDIV_INT           (0x3ff << 6)
#define ELB_SBUS_PLL_CNTL1_NDIV_P             (0x3ff << 16)          
#define ELB_SBUS_PLL_CNTL2_NDIV_Q             0x3ff
#define ELB_SBUS_PLL_CNTL2_NDIV_FRAC_MODE_SEL (0x1 << 10)
#define ELB_SBUS_PLL_CNTL2_FREFEFF_INFO       (0xff << 11)

//#ifdef _CSV_INCLUDED_
//extern "C" void top_sbus_ld_sbus_rom(int inst, const char * rom_file);
//#endif    

#ifdef __cplusplus
extern "C" {
#endif
void elb_sbus_write(int chip_id, int sbus_id, int rcvr_addr, int data_addr, unsigned int data);
unsigned int  elb_sbus_read(int chip_id, int sbus_id, int rcvr_addr, int data_addr);
void elb_sbus_reset(int chip_id, int sbus_id, int rcvr_addr);

void elb_arm_sbus_write(int chip_id, int rcvr_addr, int data_addr, unsigned int data);
unsigned int  elb_arm_sbus_read(int chip_id, int rcvr_addr, int data_addr);
void elb_arm_sbus_reset(int chip_id, int rcvr_addr);

void elb_core_sbus_write(int chip_id, int rcvr_addr, int data_addr, unsigned int data);
unsigned int  elb_core_sbus_read(int chip_id, int rcvr_addr, int data_addr);
void elb_core_sbus_reset(int chip_id, int rcvr_addr);

void elb_aod_sbus_write(int chip_id, int rcvr_addr, int data_addr, unsigned int data);
unsigned int  elb_aod_sbus_read(int chip_id, int rcvr_addr, int data_addr);
void elb_aod_sbus_reset(int chip_id, int rcvr_addr);

void elb_pp_sbus_write(int chip_id, int rcvr_addr, int data_addr, unsigned int data);
unsigned int  elb_pp_sbus_read(int chip_id, int rcvr_addr, int data_addr);
void elb_pp_sbus_reset(int chip_id, int rcvr_addr);


void elb_ms_sbus_write(int chip_id, int rcvr_addr, int data_addr, unsigned int data);
unsigned int  elb_ms_sbus_read(int chip_id, int rcvr_addr, int data_addr);
void elb_ms_sbus_reset(int chip_id, int rcvr_addr);

void elb_sbus_pp_set_rom_enable(int chip_id, int inst_id, int val);

void elb_sbus_pll_reg_rd_all(int chip_id, int ss, int rcvr_addr); 

// spico rom
#ifndef SWIG //  exclude from zebu
void upload_sbus_master_firmware_w_wait(int chip_id, int sbus_ring_ms, int device_addr, const void * fila_name, int wait_time);
void upload_sbus_master_firmware(int chip_id, int sbus_ring_ms, int device_addr, const void * fila_name);
#endif
uint32_t sbus_master_spico_interrupt_w_wait(int chip_id, int sbus_ring_ms, int device_addr, int interrupt_code, int interrupt_value, uint32_t data, int no_busy_check, int wait_timeout);
uint32_t sbus_master_spico_interrupt(int chip_id, int sbus_ring_ms, int device_addr, int interrupt_code, int interrupt_value, uint32_t data, int no_busy_check);
uint32_t report_firmware_revision(int sbus_ring_ms, int device_addr);
uint32_t report_firmware_build_id(int sbus_ring_ms, int device_addr);
uint32_t sbus_master_spico_interrupt_wait_done(int chip_id, int sbus_ring_ms, int device_addr, int wait_timeout); 

// set spico sbus group
void set_sbus_broadcast_grp(int chip_id, int sbus_ring_ms, int sbus_ring_node, int bcast_addr, int bgroup);

void elb_pp_ld_sbus_master_firmware(int chip_id, int inst_id, const char * rom_file, int backdoor); 

#ifdef __cplusplus
}
#endif

#endif


