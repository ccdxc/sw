#ifndef CAP_SBUS_API_H
#define CAP_SBUS_API_H

#ifdef CAPRI_SW
#include "cap_base_api.h"
#include "cap_top_csr_defines.h"
#else
#include "cpu.h"
#include "cap_base_api.h"
#include "cap_top_csr_defines.h"
#include "cap_blk_reg_model.h"
#include "sknobs.h"
#include "LogMsg.h"

using namespace std;
#endif

/////////////////////////////////////////////////////////////////
// SBUS Procedures
/////////////////////////////////////////////////////////////////
#define CAP_SBUS_DFI_PHYLVL_REQ_N                40
#define CAP_SBUS_DFI_PHYLVL_ACK_N                42

// Control 1500 Operations
#define CAP_SBUS_PHY_CONFIG_LENGTH               104
#define CAP_SBUS_APC_ADDR                        0x01
#define CAP_SBUS_SBUS_RST                        0x20
#define CAP_SBUS_SBUS_WR                         0x21
#define CAP_SBUS_SBUS_RD                         0x22
#define CAP_SBUS_STOP_1500                       0x0 // 4'b0000
#define CAP_SBUS_SHIFT_READ_WDR                  0xc // 4'b1100
#define CAP_SBUS_START_WDR_READ                  0x4 // 4'b0100
#define CAP_SBUS_SHIFT_WRITE_WDR                 0x2 // 4'b1010
#define CAP_SBUS_START_WDR_WRITE                 0x2 // 4'b0010
#define CAP_SBUS_START_WIR_WRITE                 0x1

//#define CAP_SBUS_SBUS_MSG "SBUS INFO"
#define CAP_SBUS_SBUS_MSG "SBUS FAST ACCESS"
#define CAP_SBUS_HBM_1500_MSG "(HBM 1500 FAST ACCESS)"

//
/// APC_REGISTERS
//
#define CAP_SBUS_SBUS_RESET_REG         0 
#define CAP_SBUS_OBSERVE_RST            1 
#define CAP_SBUS_WIR_REG                2 
#define CAP_SBUS_WDR_WRITE_DATA_31_0    4 
#define CAP_SBUS_WDR_WRITE_DATA_63_32   5 
#define CAP_SBUS_WDR_WRITE_DATA_95_64   6 
#define CAP_SBUS_WDR_WRITE_DATA_127_96  7 
#define CAP_SBUS_WDR_WRITE_DATA_159_128 8 
#define CAP_SBUS_WDR_WRITE_DATA_191_160 9 
#define CAP_SBUS_WDR_WRITE_DATA_223_192 10
#define CAP_SBUS_WDR_WRITE_DATA_255_224 11
#define CAP_SBUS_WDR_WRITE_DATA_287_256 12
#define CAP_SBUS_WDR_WRITE_DATA_319_288 13
#define CAP_SBUS_CONTROL_1500           16
#define CAP_SBUS_READ_CHANNEL           17
#define CAP_SBUS_BUSY_DONE_1500_REG     18
#define CAP_SBUS_WDR_READ_DATA_31_0     20
#define CAP_SBUS_WDR_READ_DATA_63_32    21
#define CAP_SBUS_WDR_READ_DATA_95_64    22
#define CAP_SBUS_WDR_READ_DATA_127_96   23
#define CAP_SBUS_WDR_READ_DATA_159_128  24
#define CAP_SBUS_WDR_READ_DATA_191_160  25
#define CAP_SBUS_WDR_READ_DATA_223_192  26
#define CAP_SBUS_WDR_READ_DATA_255_224  27
#define CAP_SBUS_WDR_READ_DATA_287_256  28
#define CAP_SBUS_WDR_READ_DATA_319_288  29
#define CAP_SBUS_APC_BIST_EN_REG        30
#define CAP_SBUS_APC_BIST_CONFIG        32
#define CAP_SBUS_APC_TEST_CYCLES        34
#define CAP_SBUS_APC_BIST_CONTROL       36
#define CAP_SBUS_BIST_BUSY_DONE_REG     38
#define CAP_SBUS_APC_CNTL_REQ           40
#define CAP_SBUS_APC_INIT_COMPLETE      41
#define CAP_SBUS_APC_CNTL_ACK           42
#define CAP_SBUS_TEMP_CATTRIP_REG       44
#define CAP_SBUS_LBP_DRV_IMP_REG        46
#define CAP_SBUS_DELAY_CFG_DLL_REG      48
#define CAP_SBUS_CKOBS_EN_REG           50
#define CAP_SBUS_CKOBS_R_W_SEL_REG      52
#define CAP_SBUS_PVT_DELAY_CLK_REG      54
#define CAP_SBUS_PVT_DELAY_STB_REG      56
#define CAP_SBUS_BIST_CLK_EN_REG        58
#define CAP_SBUS_APC_IGNORE_BROADCAST   61
#define CAP_SBUS_APC_REVISION_FIELD     62
#define CAP_SBUS_SPARE_0_REG            70
#define CAP_SBUS_SPARE_1_REG            71
#define CAP_SBUS_SPARE_2_REG            72
#define CAP_SBUS_SPARE_3_REG            73
#define CAP_SBUS_SPARE_4_REG            74
#define CAP_SBUS_SPARE_5_REG            75
#define CAP_SBUS_SPARE_6_REG            76
#define CAP_SBUS_SPARE_7_REG            77


// Define HBM 1500 register names and lengths
#define CAP_SBUS_HBM_SEL                        1
#define CAP_SBUS_HBM_BYPASS                     0   
#define CAP_SBUS_HBM_BYPASS_LENGTH              1   
#define CAP_SBUS_HBM_EXTEST_RX                  1
#define CAP_SBUS_HBM_EXTEST_RX_LENGTH           215
#define CAP_SBUS_HBM_EXTEST_TX                  2
#define CAP_SBUS_HBM_EXTEST_TX_LENGTH           215
#define CAP_SBUS_HBM_RESET                      5
#define CAP_SBUS_HBM_RESET_LENGTH               1
#define CAP_SBUS_HBM_DWORD_MISR                 9
#define CAP_SBUS_HBM_DWORD_MISR_LENGTH          320
#define CAP_SBUS_HBM_AWORD_MISR                 0xa
#define CAP_SBUS_HBM_AWORD_MISR_LENGTH          30
#define CAP_SBUS_HBM_AWORD_MISR_CONFIG          0xd 
#define CAP_SBUS_HBM_AWORD_MISR_CONFIG_LENGTH   8   
#define CAP_SBUS_HBM_DEVICE_ID                  0xe
#define CAP_SBUS_HBM_DEVICE_ID_LENGTH           82  
#define CAP_SBUS_HBM_TEMPERATURE                0xf 
#define CAP_SBUS_HBM_TEMPERATURE_LENGTH         8   
#define CAP_SBUS_HBM_MODE_REG_DUMP_SET          0x10
#define CAP_SBUS_HBM_MODE_REG_DUMP_SET_LENGTH   128 
#define CAP_SBUS_HBM_LFSR_COMPARE_STICKY        0x11
#define CAP_SBUS_HBM_LFSR_COMPARE_STICKY_LENGTH 175 
#define CAP_SBUS_HBM_SOFT_LANE_REPAIR           0x12
#define CAP_SBUS_HBM_SOFT_LANE_REPAIR_LENGTH    72 
#define CAP_SBUS_HBM_HARD_LANE_REPAIR           0x13
#define CAP_SBUS_HBM_HARD_LANE_REPAIR_LENGTH    72 

// Define PHY 1500 register names and lengths
#define CAP_SBUS_PHY_SEL                        0
#define CAP_SBUS_PHY_BYPASS                     0   
#define CAP_SBUS_PHY_BYPASS_LENGTH              1   
#define CAP_SBUS_PHY_EXTEST_RX                  0x1
#define CAP_SBUS_PHY_EXTEST_RX_LENGTH           215
#define CAP_SBUS_PHY_EXTEST_TX                  0x2
#define CAP_SBUS_PHY_EXTEST_TX_LENGTH           215
#define CAP_SBUS_PHY_AWORD_MISR_CONFIG          0xd
#define CAP_SBUS_PHY_AWORD_MISR_CONFIG_LENGTH   8
#define CAP_SBUS_PHY_DWORD_MISR                 0x9
#define CAP_SBUS_PHY_DWORD_MISR_LENGTH          320
#define CAP_SBUS_PHY_AWORD_MISR                 0xa
#define CAP_SBUS_PHY_AWORD_MISR_LENGTH          30
#define CAP_SBUS_PHY_DWORD_MISR_CONFIG          0x10
#define CAP_SBUS_PHY_DWORD_MISR_CONFIG_LENGTH   32
#define CAP_SBUS_PHY_LFSR_COMPARE_STICKY        0x11
#define CAP_SBUS_PHY_LFSR_COMPARE_STICKY_LENGTH 184
#define CAP_SBUS_PHY_SOFT_LANE_REPAIR           0x12
#define CAP_SBUS_PHY_SOFT_LANE_REPAIR_LENGTH    72 
#define CAP_SBUS_PHY_CONFIG                     0x14
#define CAP_SBUS_PHY_CTC_ID                     0x16
#define CAP_SBUS_PHY_CTC_ID_LENGTH              8   


// Firmware HBM Operations
#define FW_RESET                             0
#define FW_RESET_PHY                         1
#define FW_RESET_HBM                         2
#define FW_PACKAGE_TEST_FLOW                 3
#define FW_WAFER_TEST_FLOW                   4
#define FW_POWER_ON_FLOW                     5
#define FW_CONNECTIVITY_CHECK                6
#define FW_BYPASS_TEST                       7
#define FW_READ_DEVICE_ID                    8
#define FW_AWORD_TEST                        9
#define FW_AERR_TEST                         10
#define FW_DWORD_TEST                        11
#define FW_DERR_TEST                         12
#define FW_LANE_REPAIR                       13
#define FW_AWORD_ILB                         14
#define FW_DWORD_ILB                         15
#define FW_READ_TEMPERATURE                  16
#define FW_APPLY_SOFT_LANE_REPAIRS_AS_HARD_LANE_REPAIRS     17
#define FW_APPLY_HARD_LANE_REPAIRS_AS_PHY_SOFT_LANE_REPAIRS 18
#define FW_RUN_SAMSUNG_MBIST                 19
#define FW_RUN_SKH_MBIST                     20
#define FW_RESET_MODE_REGISTERS              21
#define FW_RESET_PHY_CONFIG                  22
#define FW_AWORD_SLB                         23
#define FW_DWORD_SLB                         24
#define FW_SAMSUNG_CHIPPING_TEST             25
#define FW_LOAD_SAMSUNG_TMRS                 26
#define FW_AERR_ILB                          27
#define FW_AERR_SLB                          28
#define FW_DERR_ILB                          29
#define FW_DERR_SLB                          30
#define FW_INITIALIZE_NWL_MCS                31
#define FW_RUN_CTCS                          32
#define FW_VERIFY_LANE_ERRORS                33
#define FW_START_CTC                         34
#define FW_STOP_CTC                          35
#define FW_TMRS                              36
#define FW_RELEASE_CTC_CONTROL               37

// Firmware HBM Parameters
#define HBM_INTERFACE_REQUEST               0
#define HBM_MAX_TIMEOUT                     1
#define HBM_TINIT1_CYCLES                   2
#define HBM_TINIT2_CYCLES                   3
#define HBM_TINIT3_CYCLES                   4
#define HBM_TINIT4_CYCLES                   5
#define HBM_TINIT5_CYCLES                   6
#define HBM_RW_LATENCY_OFFSET               7
#define HBM_LATENCY_ODD_N_EVEN              8
#define HBM_SAVE_RESTORE_CONFIG             9
#define HBM_MODE_REGISTER0                  10
#define HBM_MODE_REGISTER1                  11
#define HBM_MODE_REGISTER2                  12
#define HBM_MODE_REGISTER3                  13
#define HBM_MODE_REGISTER4                  14
#define HBM_MODE_REGISTER5                  15
#define HBM_MODE_REGISTER6                  16
#define HBM_MODE_REGISTER7                  17
#define HBM_MODE_REGISTER8                  18
#define HBM_MODE_REGISTER15                 58
#define HBM_PHY_CONFIG0                     19
#define HBM_PHY_CONFIG1                     20
#define HBM_PHY_CONFIG2                     21
#define HBM_PHY_CONFIG3                     22
#define HBM_PHY_CONFIG4                     23
#define HBM_PHY_CONFIG5                     24
#define HBM_PHY_CONFIG6                     25
#define HBM_LBP_DRV_IMP                     26
#define HBM_DELAY_CONFIG_DLL                27
#define HBM_IGNORE_PHYUPD_HANDSHAKE         28
#define HBM_TUPDMRS                         29
#define HBM_T_RDLAT_OFFSET                  30
#define HBM_MBIST_REPAIR_MODE               31
#define HBM_SAMSUNG_MBIST_PATTERN           32
#define HBM_SAMSUNG_MIST_HARD_REPAIR_CYCLES 33
#define HBM_HARD_LANE_REPAIR_CYCLES         34
#define HBM_POWER_ON_LANE_REPAIR_MODE       35
#define HBM_MBIST_BANK_ADDRESS_END          36
#define HBM_MBIST_ROW_ADDRESS_END           37
#define HBM_MBIST_COLUMN_ADDRESS_END        38
#define HBM_FREQ                            39
#define HBM_DIV_MODE                        40
#define HBM_CKE_EXIT_STATE                  41
#define HBM_TEST_MODE_REGISTER0             42
#define HBM_TEST_MODE_REGISTER1             43
#define HBM_TEST_MODE_REGISTER2             44
#define HBM_TEST_MODE_REGISTER3             45
#define HBM_TEST_MODE_REGISTER4             46
#define HBM_TEST_MODE_REGISTER5             47
#define HBM_TEST_MODE_REGISTER6             48
#define HBM_TEST_MODE_REGISTER7             49
#define HBM_TEST_MODE_REGISTER8             50
#define HBM_TEST_MODE_REGISTER15            59
#define HBM_CTC_RUN_CYCLES                  51
#define HBM_CTC_CHANNEL_IGNORE              52
#define HBM_CTC_INITIAL_ADDRESS_LO          53
#define HBM_CTC_INITIAL_ADDRESS_HI          54
#define HBM_CTC_MAX_ADDRESS_LO              55
#define HBM_CTC_MAX_ADDRESS_HI              56
#define HBM_TEST_T_RDLAT_OFFSET             57
#define HBM_MODE_REGISTER15                 58
#define HBM_TEST_MODE_REGISTER15            59
#define HBM_BYPASS_TESTMODE_RESET           60
#define HBM_DISABLE_ADDR_LANE_REPAIR        61
#define HBM_CTC_PATTERN_TYPE                62
#define HBM_BYPASS_REPAIR_ON_RESET          63
#define HBM_STACK_HEIGHT                    64
#define HBM_MANUFACTURER_ID                 65
#define HBM_DENSITY                         66
#define HBM_MANUALLY_CONFIGURE_ID           67
#define HBM_PARITY_LATENCY                  68
#define HBM_TEST_PARITY_LATENCY             69
#define HBM_DFI_T_RDDATA_EN                 70
#define HBM_MANUALLY_CONFIGURE_NWL          71
#define HBM_CTC_PSEUDO_CHANNEL              72


#ifdef _CSV_INCLUDED_
extern "C" void top_sbus_ld_sbus_rom(int inst, const char * rom_file);
#endif    

// application functions
#ifndef CAPRI_SW
void *romfile_open(const void *rom_info);
int romfile_read(void *ctx, unsigned int *datap);
void romfile_close(void *ctx);
#endif

void cap_ms_sbus_write(int chip_id, int rcvr_addr, int data_addr, unsigned int data);
unsigned int  cap_ms_sbus_read(int chip_id, int rcvr_addr, int data_addr);
void cap_pp_sbus_write(int chip_id, int rcvr_addr, int data_addr, unsigned int data);
unsigned int  cap_pp_sbus_read(int chip_id, int rcvr_addr, int data_addr);
void cap_pp_sbus_reset(int chip_id, int rcvr_addr);
void cap_ms_sbus_reset(int chip_id, int rcvr_addr);
void cap_sbus_pp_set_rom_enable(int chip_id, int inst_id, int val);

// spico rom
#ifndef CAPRI_SW
void upload_sbus_master_firmware(int chip_id, int sbus_ring_ms, int device_addr, const void* rom_info);
#else
void upload_sbus_master_firmware(int chip_id, int sbus_ring_ms, int device_addr, void* rom_info);
#endif
//uint32_t sbus_master_spico_interrupt(int chip_id, int sbus_ring_ms, int device_addr, int interrupt_code, int interrupt_value, uint32_t data, int no_busy_check=0, int wait_timeout=100);
uint32_t sbus_master_spico_interrupt(int chip_id, int sbus_ring_ms, int device_addr, int interrupt_code, int interrupt_value, uint32_t data, int no_busy_check, int wait_timeout);
uint32_t report_firmware_revision(int sbus_ring_ms, int device_addr);
uint32_t report_firmware_build_id(int sbus_ring_ms, int device_addr);
uint32_t sbus_master_spico_interrupt_wait_done(int chip_id, int sbus_ring_ms, int device_addr, int wait_timeout); 

// set spico sbus group
void set_sbus_broadcast_grp(int chip_id, int sbus_ring_ms, int sbus_ring_node, int bcast_addr, int bgroup);

void cap_pp_ld_sbus_master_firmware(int chip_id, int inst_id, const char * rom_file, int backdoor); 

#endif


