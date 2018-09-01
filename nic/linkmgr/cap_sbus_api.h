#ifndef CAP_SBUS_API_H
#define CAP_SBUS_API_H

#ifdef CAPRI_SW
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
#define CAP_SBUS_SBUS_RESET_REG                      0
#define CAP_SBUS_WIR_REG                             2
#define CAP_SBUS_CONTROL_1500                       16
#define CAP_SBUS_READ_CHANNEL                       17
#define CAP_SBUS_BUSY_DONE_1500_REG                 18
#define CAP_SBUS_APC_BIST_EN_REG                    30
#define CAP_SBUS_DFI_PHYLVL_REQ_N                   40
#define CAP_SBUS_APC_INIT_COMPLETE                  41
#define CAP_SBUS_DFI_PHYLVL_ACK_N                   42

#define CAP_SBUS_PHY_SEL                             0
#define CAP_SBUS_PHY_CONFIG_LENGTH                 104
#define CAP_SBUS_PHY_SOFT_LANE_REPAIR_LENGTH        72
#define CAP_SBUS_APC_ADDR                        0x01
#define CAP_SBUS_PHY_SOFT_LANE_REPAIR            0x12
#define CAP_SBUS_PHY_CONFIG                      0x14
#define CAP_SBUS_SBUS_RST                        0x20
#define CAP_SBUS_SBUS_WR                         0x21
#define CAP_SBUS_SBUS_RD                         0x22
#define CAP_SBUS_STOP_1500                       0x0 // 4'b0000
#define CAP_SBUS_SHIFT_READ_WDR                  0xc // 4'b1100
#define CAP_SBUS_START_WDR_READ                  0x4 // 4'b0100
#define CAP_SBUS_SHIFT_WRITE_WDR                 0x2 // 4'b1010
#define CAP_SBUS_START_WDR_WRITE                 0x2 // 4'b0010

#define CAP_SBUS_HBM_SEL                             1
#define CAP_SBUS_HBM_SOFT_LANE_REPAIR            0x12
#define CAP_SBUS_HBM_SOFT_LANE_REPAIR_LENGTH        72
#define CAP_SBUS_HBM_DEVICE_ID                   0x0e
#define CAP_SBUS_HBM_DEVICE_ID_LENGTH               82
#define CAP_SBUS_HBM_TEMPERATURE                 0x0f
#define CAP_SBUS_HBM_TEMPERATURE_LENGTH              8
#define CAP_SBUS_HBM_MODE_REG_DUMP_SET           0x10
#define CAP_SBUS_HBM_MODE_REG_DUMP_SET_LENGTH      128

//#define CAP_SBUS_SBUS_MSG "SBUS INFO"
#define CAP_SBUS_SBUS_MSG "SBUS FAST ACCESS"
#define CAP_SBUS_HBM_1500_MSG "(HBM 1500 FAST ACCESS)"

#ifdef _CSV_INCLUDED_
extern "C" void top_sbus_ld_sbus_rom(int inst, const char * rom_file);
#endif    

// application functions
void *romfile_open(void *rom_info);
int romfile_read(void *ctx, unsigned int *datap);
void romfile_close(void *ctx);

void cap_ms_sbus_write(int chip_id, int rcvr_addr, int data_addr, int data);
int  cap_ms_sbus_read(int chip_id, int rcvr_addr, int data_addr);
void cap_pp_sbus_write(int chip_id, int rcvr_addr, int data_addr, int data);
int  cap_pp_sbus_read(int chip_id, int rcvr_addr, int data_addr);
void cap_sbus_pp_set_rom_enable(int chip_id, int inst_id, int val);

// spico rom
void upload_sbus_master_firmware(int chip_id, int sbus_ring_ms, int device_addr, void* rom_info);
//uint32_t sbus_master_spico_interrupt(int chip_id, int sbus_ring_ms, int device_addr, int interrupt_code, int interrupt_value, uint32_t data, int no_busy_check=0, int wait_timeout=100);
uint32_t sbus_master_spico_interrupt(int chip_id, int sbus_ring_ms, int device_addr, int interrupt_code, int interrupt_value, uint32_t data, int no_busy_check, int wait_timeout);
uint32_t report_firmware_revision(int sbus_ring_ms, int device_addr);
uint32_t report_firmware_build_id(int sbus_ring_ms, int device_addr);
uint32_t sbus_master_spico_interrupt_wait_done(int chip_id, int sbus_ring_ms, int device_addr, int wait_timeout); 

// set spico sbus group
void set_sbus_broadcast_grp(int chip_id, int sbus_ring_ms, int sbus_ring_node, int bcast_addr, int bgroup);

void cap_pp_ld_sbus_master_firmware(int chip_id, int inst_id, const char * rom_file, int backdoor); 

#endif
