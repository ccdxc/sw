#ifdef CAPRI_SW
#include "cap_sw_glue.h"
#endif
#include "cap_sbus_api.h"
#ifndef CAPRI_SW
#include "cap_ms_csr.h"
#endif
#include "cap_ms_apb_defines.h"
#include "cap_nwl_sbus_api.h"
#ifndef CAPRI_SW
#include "cpp_int_helper.h"
#endif
#include "cap_ms_c_hdr.h"

// uint32_t current_instr_length=0;

#define NUM_SBUS_DEV 27
#ifndef SWPRINTF
#define SW_PRINT printf
#endif
#ifdef CAPRI_SW
//#define PLOG(f, ...) do {} while (0)
#define SW_POLL() do {} while (0)
#define PEN_SLEEP sleep
#else
#define PEN_SLEEP usleep
#define SW_POLL() do {} while (0)
#endif

#ifdef _CSV_INCLUDED_    
extern "C" void hbm_download_serdes_code(const char * id);
#endif    

void cap_nwl_sbus_reg_write(uint64_t addr, uint32_t wr_data) {
#ifdef ARM_BOOT_SEQUENCE
  ///  writereg(addr, wr_data);
#else
#ifndef CAPRI_SW
  vector<uint32_t> data;
  data.resize(1);  
  data[0] = wr_data;  
  cpu::access()->block_write(0,addr, 1, data, true, 1);   
#else
  cap_sw_writereg(addr, wr_data);
#endif
#endif
}

uint32_t cap_nwl_sbus_reg_read(uint64_t addr) {
  uint32_t rd_data=0;
#ifdef ARM_BOOT_SEQUENCE
  ///  rd_data = readreg(addr);
#else
#ifndef CAPRI_SW
  vector<uint32_t> rd_vec;  
  rd_vec = cpu::access()->block_read(0, addr, 1, false, 1);
  if (rd_vec.size() > 0) {
    rd_data = rd_vec[0];
  }
#else
  rd_data = cap_sw_readreg(addr);
#endif
#endif
  return rd_data;
}


void avago_top_sbus_command(uint32_t cmd, uint32_t address, uint32_t wr_data, uint32_t *rd_data) {

  uint32_t rcvr_addr = CAP_SBUS_HBM_PHY_RCVR_ADDR;

  if (cmd == CAP_SBUS_SBUS_WR) {    
    cap_ms_sbus_write(0, rcvr_addr, address, wr_data);
    PLOG("nwl_sbus_api", "CAP_NWL_SBUS_API: avago_top_sbus_command called: CAP_SBUS_SBUS_WR: addr: 0x" << hex << address << " wr_data=0x" << wr_data << endl);
  }
  else if (cmd == CAP_SBUS_SBUS_RD) {
    *rd_data = cap_ms_sbus_read(0, rcvr_addr, address);
    PLOG("nwl_sbus_api", "CAP_NWL_SBUS_API: avago_top_sbus_command called: CAP_SBUS_SBUS_RD: addr: 0x" << hex << address << " rd_data=0x" << *rd_data << endl);
  }
  else {
    PLOG_ERR("CAP_NWL_SBUS_API: avago_top_sbus_command: Unsupported command: " << cmd << endl);
  }
}


void cap_nwl_sbus_check_idcode(void) {

  PLOG_MSG("cap_nwl_sbus_check_idcode" << endl);

  int chip_id = 0;
  int last_sbus_address;
  int rd_data;

  // IDCODE
  // chain1:  SBM -> HBMPLL -> M40 (ctc0-3) -> HBMPHY -> M41 (ctc4-7) -> SBM
  int dev_idcode[NUM_SBUS_DEV] = {0x2, 0xa, 
				  0xf, 0xf, 0x10,  // M40 ctc0
				  0xf, 0xf, 0x10,  // M40 ctc1
				  0xf, 0xf, 0x10,  // M40 ctc2
				  0xf, 0xf, 0x10,  // M40 ctc3
				  0x2a, // HBMPHY
				  0xf, 0xf, 0x10,  // M41 ctc0
				  0xf, 0xf, 0x10,  // M41 ctc1
				  0xf, 0xf, 0x10,  // M41 ctc2
				  0xf, 0xf, 0x10};  // M41 ctc3
 
#ifndef CAPRI_SW
  int idcode_rd         = sknobs_get_value((char*)"test/sbus/idcode_rd",0);

  // set clk divider
  int sbus_divider         = sknobs_get_value((char*)"test/sbus/sbus_divider",1);
#else
  int idcode_rd         = 0;

  // set clk divider
  int sbus_divider         = 1;
#endif 

  PLOG_MSG("TEST:SBUS cap_top_ms_sbus_test clk divider set to " << sbus_divider << endl);

  if (sbus_divider == 6) { // set non-power-of-2 sbus divider (6 is the mission mode value that needs to be tested as per Avago)
     // Refer Table 2 of Section 2.1.3 of 16 nm SBUS Master Specifications (Ver 1.1/ Sep 2015) for other values
     cap_ms_sbus_write(chip_id, CAP_SBUS_CTRLR_BCAST_ID, 0xb, 0x5); //  corresponds to divider of 6 (in conjunction with 0xA)
     cap_ms_sbus_write(chip_id, CAP_SBUS_CTRLR_BCAST_ID, 0xa, (0x3 | (1 << 7)) ); // 1 << 7 is to set the non-power-of-2 mode, 3 sets divider-mode to 6
  } else { // power-of-2 dividers
     if ((sbus_divider <= 0) || (sbus_divider > 15)) {
        PLOG_ERR("TEST:SBUS cap_top_ms_sbus_divider programmed with illegal value :0x" << sbus_divider << endl);
     }
     cap_ms_sbus_write(chip_id, 0xfe, 0xa, sbus_divider); // legal values are 0,1,2,...15
  }

  rd_data = cap_ms_sbus_read(chip_id, 0xfe, 0xff);
  if (rd_data == dev_idcode[0]) {
     PLOG_MSG("TEST:SBUS PASS cap_top_ms_sbus_test sbus controller idcode:0x" << hex << rd_data << dec << endl);
  } else {
     PLOG_ERR("TEST:SBUS cap_top_ms_sbus_test sbus controller idcode:0x" << hex << rd_data << " exp:" << dev_idcode[0] << dec << endl);
  }

  last_sbus_address = cap_ms_sbus_read(chip_id, 0xfe, 0x2);
  if (last_sbus_address == NUM_SBUS_DEV) {
    PLOG_MSG("TEST:SBUS PASS cap_top_ms_sbus_test last_sbus_address:" << last_sbus_address << endl);
  } else {
    PLOG_ERR("TEST:SBUS cap_top_ms_sbus_test last_sbus_address:" << last_sbus_address << " exp:" << NUM_SBUS_DEV << endl);
  }

  for (int aa=1; aa<last_sbus_address; aa++) {
    if (idcode_rd) {
      rd_data = cap_ms_sbus_read(chip_id, aa, 0xff);

      if (rd_data == dev_idcode[aa]) {
         PLOG_MSG("TEST:SBUS PASS cap_top_ms_sbus_test sbus controller idcode:0x" << hex << rd_data << dec << endl);
      } else {
         PLOG_ERR("TEST:SBUS cap_top_ms_sbus_test sbus controller idcode:0x" << hex << rd_data << " exp:" << dev_idcode[aa] << dec << endl);
      }
    } 
  }
}

void cap_nwl_scrub_expmt_ctc_start(int chip_id, int ctc_id, unsigned int addr_low) {
   cap_nwl_set_max_addr(chip_id, ctc_id, addr_low);
   cap_nwl_ctc_check_prbs(chip_id, ctc_id, addr_low, 0x3, 0, 0, 0, 0, 0xffffffff, 0);
}

void cap_nwl_scrub_ecc_ctc_start(int chip_id, int ctc_id) {
   cap_nwl_set_max_addr(chip_id, ctc_id, 0x10000000);
   cap_nwl_ctc_check_prbs(chip_id, ctc_id, 0x800000, 0x3, 0, 0, 0, 0, 0xffffffff, 0);
}
int cap_nwl_scrub_ecc_ctc_wait_for_done(int chip_id, int ctc_id) {
   return cap_nwl_ctc_wait_for_complete(chip_id, ctc_id);
}

void cap_nwl_scrub_8G_ecc_ctc_start(int chip_id, int ctc_id) {
   cap_nwl_set_max_addr(chip_id, ctc_id, 0x20000000);
   cap_nwl_ctc_check_prbs(chip_id, ctc_id, 0x1000000, 0x3, 0, 0, 0, 0, 0xffffffff, 0);
}
int cap_nwl_scrub_8G_ecc_ctc_wait_for_done(int chip_id, int ctc_id) {
   return cap_nwl_ctc_wait_for_complete(chip_id, ctc_id);
}

void cap_nwl_release_ctc(int chip_id, int ctc_id) {

  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_CTC_MUX_CONTROL, 0x0);   // Turn MUX to normal mode
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_CTC_RUN_CONTROL, 0x0);   // Turn off RUN
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_CTC_RESET_CONTROL, 0x1); // Assert CTC_RESET
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_CTC_RESET_CONTROL, 0x0); // Deassert CTC_RESET
}

void cap_nwl_set_max_addr(int chip_id, int ctc_id, int max_addr) {

  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_WRITE_ADDRESS_MAX_LO, max_addr);
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_WRITE_ADDRESS_MAX_HI, 0x0);
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_READ_ADDRESS_MAX_LO, max_addr);
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_READ_ADDRESS_MAX_HI, 0x0);
}

void cap_nwl_ctc_check_prbs(int chip_id, int ctc_id, unsigned int ctc_pkt_count, unsigned int ctc_prbs_type, unsigned int ctc_prbs_value, unsigned int ctc_addr_lo, unsigned int ctc_addr_hi, unsigned int ctc_test_mode, unsigned int ctc_rd_wr_mix, unsigned int ctc_rd_wr_inv) {

  PLOG_MSG("cap_nwl_ctc_check_prbs" << endl);

  int last_sbus_address;
  //int rd_data; FIXME

  // set clk divider
  PLOG_MSG("TEST:SBUS cap_top_ms_sbus_test clk divider" << endl);
  cap_ms_sbus_write(chip_id, CAP_SBUS_CTRLR_BCAST_ID, 0xa, 0x1);

  //rd_data = cap_ms_sbus_read(chip_id, CAP_SBUS_CTRLR_BCAST_ID, 0xff); // FIXME
  last_sbus_address = cap_ms_sbus_read(chip_id, CAP_SBUS_CTRLR_BCAST_ID, 0x2);
  last_sbus_address = last_sbus_address + 1; // FIXME

  PLOG_MSG("CAP_NWL_SBUS_API: Starting CTC PRBS testing" << endl);
  cap_nwl_test_prbs(chip_id, ctc_id, ctc_pkt_count, ctc_prbs_type, ctc_prbs_value, ctc_addr_lo, ctc_addr_hi, ctc_test_mode, ctc_rd_wr_mix, ctc_rd_wr_inv);
}

void cap_nwl_test_prbs(int chip_id, int ctc_id, unsigned int ctc_pkt_count, unsigned int ctc_prbs_type, unsigned int ctc_prbs_value, unsigned int ctc_addr_lo, unsigned int ctc_addr_hi, unsigned int ctc_test_mode, unsigned int ctc_rd_wr_mix, unsigned int ctc_rd_wr_inv) {

  unsigned int scratch_data0 = 0;
  unsigned int scratch_data1 = 0;

  PLOG_MSG("CAP_NWL_SBUS_API: CTC configuration for ctc:" << ctc_id << " num_wrrds:" << ctc_pkt_count << " prbs_mode:" << ctc_prbs_type << " addr_lo:0x" << hex << ctc_addr_lo << dec << endl);
  

  PLOG_MSG("CAP_NWL_SBUS_API: CTC Mux Control" << endl);
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_CTC_MUX_CONTROL, 0x6);

  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_CTC_RUN_CONTROL, 0x0);   // Turn off RUN
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_CTC_RESET_CONTROL, 0x1);   // Assert CTC_RESET
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_CTC_RESET_CONTROL, 0x0);   // Deassert CTC_RESET

  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_VALID_COUNT_CONTROL, ctc_pkt_count);   // Set VALID_COUNT to 65 - 2x+ through

  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_ERROR_CONTROL_STATUS, 0x1);   // Enable error check
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_CTC_RESETS, 0x7f);    // Assert all generator resets

  // R/W mix
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_WRITE_MIX, ctc_rd_wr_mix);
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_READ_MIX, ~ctc_rd_wr_mix);

  // Clear B2B:           
  if (ctc_prbs_value == 0) {
     cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_BACK2BACK_TRANSACTION, 0xffffffff);
  } else {
     cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_BACK2BACK_TRANSACTION, 0x0);
  }

  // No invert
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_WDATA_INVERT_3, ctc_rd_wr_inv);
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_WDATA_INVERT_2, ctc_rd_wr_inv);
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_WDATA_INVERT_1, ctc_rd_wr_inv);
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_WDATA_INVERT_0, ctc_rd_wr_inv);
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_RDATA_INVERT_3, ctc_rd_wr_inv);
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_RDATA_INVERT_2, ctc_rd_wr_inv);
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_RDATA_INVERT_1, ctc_rd_wr_inv);
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_RDATA_INVERT_0, ctc_rd_wr_inv);

  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_PRBS_TYPE, ctc_prbs_type);
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_DATA_INITIALIZE_PRBS, ctc_prbs_value);

  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_WRITE_ADDRESS_INIT_LO, ctc_addr_lo);
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_READ_ADDRESS_INIT_LO, ctc_addr_lo);
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_WRITE_ADDRESS_INIT_HI, ctc_addr_hi);
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_READ_ADDRESS_INIT_HI, ctc_addr_hi);

  // Data select from PRBS
  // Data Selects: 0=PRBS;1=user;2=walk;3=incrementing per 32b word
  //
  if (ctc_test_mode == 0) {
     scratch_data0 = 0; 
     scratch_data1 = 0; 
  } else if (ctc_test_mode == 1) { 
     scratch_data0 = 0xffffffff; 
     scratch_data1 = 0; 
  } else if (ctc_test_mode == 2) { 
     scratch_data0 = 0; 
     scratch_data1 = 0xffffffff; 
  } else if (ctc_test_mode == 3) { 
     scratch_data0 = 0xffffffff; 
     scratch_data1 = 0xffffffff; 
  }
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_DATA_SOURCE_BIT0_3, scratch_data0);
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_DATA_SOURCE_BIT0_2, scratch_data0);
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_DATA_SOURCE_BIT0_1, scratch_data0);
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_DATA_SOURCE_BIT0_0, scratch_data0);

  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_DATA_SOURCE_BIT1_3, scratch_data1);
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_DATA_SOURCE_BIT1_2, scratch_data1);
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_DATA_SOURCE_BIT1_1, scratch_data1);
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_DATA_SOURCE_BIT1_0, scratch_data1);

  //cap_ms_sbus_write(chip_id, CAP_SBUS_DDR_CTC_BCAST_ID, CAP_COMPARE_BUFFER_CONTROL, 0x0);   // No writes into data buffer
  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_CTC_RESETS, 0x3c);   // Deassert all PRBS resets

  cap_ms_sbus_write(chip_id, ctc_id, CAP_CTC_CTC_RUN_CONTROL, 0x1);   // Run

  //PLOG_MSG("CAP_NWL_SBUS_API: Waiting for CTC completion" << endl);
  //cap_nwl_ctc_wait_for_complete(chip_id, ctc_id);

}

int cap_nwl_ctc_wait_for_complete(int chip_id, int ctc_id) {

  int   ctc_busy;
  int   timeout;
  int   data = 0;

  ctc_busy = 1;
  timeout = 20000000;

  while (ctc_busy == 1 && timeout > 0) {
     SW_POLL();
     data = cap_ms_sbus_read(chip_id, ctc_id, CAP_CTC_CTC_STATUS); 
     ctc_busy = data & 0x1;
     //if (timeout % 100000 == 0) {
     //PLOG_MSG("CAP_NWL_SBUS_API: ctc_busy:" << ctc_busy << endl);
     //}
     timeout = timeout - 1;
  }

  if (ctc_busy == 1 && timeout == 0) {
    PLOG_MSG("CAP_NWL_SBUS_API: Timeout waiting for CTC completion" << endl);
    SW_PRINT("ERROR:NWL:CAP_NWL_SBUS_API: Timeout waiting for CTC completion\n");
    return 1;
  }

  if ( (data & 0x3) == 0x2 ) { // CTC done (not busy) with error
    data = cap_ms_sbus_read(chip_id, ctc_id, CAP_CTC_ERROR_CONTROL_STATUS); 
    PLOG_ERR("CAP_NWL_SBUS_API: CTC test completed with error with error-control-status: 0x" << hex << data << dec << endl);
    SW_PRINT("CAP_NWL_SBUS_API: CTC test completed with error\n");
    return 1;
  }

  //SW_PRINT("NWL:CAP_NWL_SBUS_API: CTC completed\n");
  return 0;
}


#ifndef CAPRI_SW
void cap_nwl_sbus_phy_sbus_start_raw(uint32_t hbmsbm, const void* rom_info) {
#else
void cap_nwl_sbus_phy_sbus_start_raw(uint32_t hbmsbm, void* rom_info) {
#endif
  uint32_t rd_data;
  uint32_t data;

  PLOG_MSG("KCM:cap_nwl_sbus_phy_sbus_start_raw: " << hex << rom_info << dec << endl);
  SW_PRINT("NWL:SBUS:Start \n");
  cap_nwl_sbus_check_idcode();
  data = cap_ms_sbus_read(0,0, 0xff);
  PLOG_MSG("KCM:initial: " << hex << data << dec << endl);
  SW_PRINT("NWL:SBUS:ID Code %d \n", data);
  report_firmware_revision(1, 0xfd);
  PLOG_MSG("KCM:revision done " << endl);


/*
  cap_ms_csr_t & ms_csr = CAP_BLK_REG_MODEL_ACCESS(cap_ms_csr_t, 0, 0);
  ms_csr.spico.i_rom_enable_hbmsbm(hbmsbm);
  ms_csr.spico.write();
*/

  SW_PRINT("NWL:SBUS:iROM turn off\n");
  rd_data = cap_nwl_sbus_reg_read((CAP_ADDR_BASE_MS_MS_OFFSET) + CAP_MS_CSR_SPICO_SPICO_1_2_BYTE_ADDRESS);
  PLOG_MSG("KCM:hbmsbm: " << hex << rd_data << dec << endl);
  rd_data = (rd_data & (!(1 << (CAP_MS_CSR_SPICO_SPICO_1_2_I_ROM_ENABLE_HBMSBM_LSB)))) | (1 << CAP_MS_CSR_SPICO_SPICO_1_2_I_ROM_ENABLE_HBMSBM_LSB);
  PLOG_MSG("KCM:hbmsbm after: " << hex << rd_data << dec << endl);
  cap_nwl_sbus_reg_write((CAP_ADDR_BASE_MS_MS_OFFSET) + CAP_MS_CSR_SPICO_SPICO_1_2_BYTE_ADDRESS, rd_data);

  rd_data = cap_nwl_sbus_reg_read((CAP_ADDR_BASE_MS_MS_OFFSET) + CAP_MS_CSR_SPICO_SPICO_1_2_BYTE_ADDRESS);
  PLOG_MSG("KCM:hbmsbm: " << hex << rd_data << dec << endl);
  rd_data = (rd_data & (!(1 << (CAP_MS_CSR_SPICO_SPICO_1_2_I_ROM_ENABLE_HBMSBM_LSB)))) | (hbmsbm << CAP_MS_CSR_SPICO_SPICO_1_2_I_ROM_ENABLE_HBMSBM_LSB);
  PLOG_MSG("KCM:hbmsbm after: " << hex << rd_data << dec << endl);
  cap_nwl_sbus_reg_write((CAP_ADDR_BASE_MS_MS_OFFSET) + CAP_MS_CSR_SPICO_SPICO_1_2_BYTE_ADDRESS, rd_data);

  upload_sbus_master_firmware(0, 1, 0xfd, rom_info);
  SW_POLL();
  //cap_nwl_sbus_check_idcode();
  for (uint32_t i = 0; i < 100; i = i+1) {    
    data = cap_ms_sbus_read(0,0, 0xff);
  }

  SW_PRINT("NWL/PCIE:SBUS:revision and build after upload? \n");
  report_firmware_revision(1, 0xfd);
  report_firmware_build_id(1, 0xfd);
  
  avago_top_sbus_command(CAP_SBUS_SBUS_WR, 10, 1, &rd_data);
 

}


void cap_nwl_sbus_phy_sbus_start() {
  uint32_t hbmsbm=0;
  int short_rom_file    = sknobs_get_value((char*)"test/sbus/short_rom_file",0);
  
  PLOG_MSG("CAP_NWL_SBUS_API: cap_nwl_sbus_phy_sbus_start called" << endl);

  cap_nwl_sbus_check_idcode();

  hbmsbm = sknobs_get_value((char*)"cap0/top/nwl_sbus_hbmsbm", 0);
  PLOG_MSG("CAP_NWL_SBUS_API: cap_nwl_sbus_phy_sbus_start called: setting i_rom_enable_hbmsbm: " << hbmsbm << endl);

  if (short_rom_file == 1) {
     cap_nwl_sbus_phy_sbus_start_raw(hbmsbm, (char*) "/home/kalyan/sbus_master.0x101F_2000.rom");
  } else {
     cap_nwl_sbus_phy_sbus_start_raw(hbmsbm, (char*) "/home/asic/vendors/brcm/design_kit/latest/hbm/documentation/phy16_HBMtop_0_testbench/ip16_HBM_testbench_0_09052017/src/firmware/sbus_master.hbm.latest.rom");
  }


}


void cap_nwl_sbus_phy_hbm_reset() {
  uint32_t rd_data;

  PLOG_MSG("CAP_NWL_SBUS_API: cap_nwl_sbus_phy_hbm_reset called" << endl);
  SW_PRINT("NWL:PHY:NWL RESET PHY HBM RESET .. sleeps ok?\n");
  SW_POLL();

  avago_top_sbus_command(CAP_SBUS_SBUS_WR, CAP_SBUS_APC_INIT_COMPLETE, 0x0, &rd_data);
  avago_top_sbus_command(CAP_SBUS_SBUS_WR, CAP_SBUS_APC_BIST_EN_REG, 0xff, &rd_data);
  PEN_SLEEP(200); // #200ns
  avago_top_sbus_command(CAP_SBUS_SBUS_WR, CAP_SBUS_SBUS_RESET_REG, 0x35, &rd_data); // 0x35 // Assert the HBM WRSTN reset
  PEN_SLEEP(200); // #200ns
  avago_top_sbus_command(CAP_SBUS_SBUS_WR, CAP_SBUS_SBUS_RESET_REG, 0x11, &rd_data); // 0x11 // Assert the 1500 PHY WRSTN and the HBM RESET_N
  
  PEN_SLEEP(200); // #200ns  
  // Deassert the HBM reset, but keep WRSTN_PHY WRSTN_HBM and APC_1500_MACHINE_RST_L asserted
  avago_top_sbus_command(CAP_SBUS_SBUS_WR, CAP_SBUS_SBUS_RESET_REG, 0x13, &rd_data); // 0x13 
  
  PEN_SLEEP(2000); // #2000ns  // tINIT3
  avago_top_sbus_command(CAP_SBUS_SBUS_WR, CAP_SBUS_SBUS_RESET_REG, 0x12, &rd_data); // 0x12

   // Deassert the CKE to drive CKE to the PHY channels
  avago_top_sbus_command(CAP_SBUS_SBUS_WR, CAP_SBUS_SBUS_RESET_REG, 0x13, &rd_data); // 0x13

  // Reset the 1500
  avago_top_sbus_command(CAP_SBUS_SBUS_WR, CAP_SBUS_SBUS_RESET_REG, 0x3f, &rd_data); // 0x3f
  PEN_SLEEP(200); // #200ns // tINIT5

  avago_top_sbus_command(CAP_SBUS_SBUS_WR, CAP_SBUS_APC_BIST_EN_REG, 0x0, &rd_data);
  avago_top_sbus_command(CAP_SBUS_SBUS_WR, CAP_SBUS_APC_INIT_COMPLETE, 0xff, &rd_data);

}

void cap_nwl_sbus_customize_avago_phy_settings() {
  uint32_t        data_32[10];
  uint32_t        channel;
  int gatesim    = sknobs_get_value((char*)"cap0/top/gatesim",0);

  PLOG_MSG("CAP_NWL_SBUS_API: cap_nwl_sbus_customize_avago_phy_settings called" << endl);

  // Set the 1500 WIR to write the mode registers for all channels
  channel = 0xf;
  cap_nwl_sbus_wir_write_channel( CAP_SBUS_PHY_SEL, channel, CAP_SBUS_PHY_CONFIG, CAP_SBUS_PHY_CONFIG_LENGTH);
  PLOG_MSG("CAP_NWL_SBUS_API: after cap_nwl_sbus_wir_write_channel called" << endl);

  PEN_SLEEP(64);
  /*
  repeat (64) begin
     @(posedge `AVAGO_TOP_PATH.CLK_DIV2);
  end
  */

  // Read the phy config values for a single channel
  channel = 0;
  cap_nwl_sbus_wdr_read(CAP_SBUS_PHY_CONFIG_LENGTH,0);
  PLOG_MSG("CAP_NWL_SBUS_API: after cap_nwl_sbus_wdr_read called" << endl);
  cap_nwl_sbus_wdr_read_sbus_data(channel, CAP_SBUS_PHY_CONFIG_LENGTH, data_32);
  PLOG_MSG("CAP_NWL_SBUS_API: after cap_nwl_sbus_wdr_read_sbus_data called" << endl);

#ifndef CAPRI_SW
#ifdef FIXME
  cpp_int_helper hlp;

  PLOG_MSG("CAP_NWL_SBUS_API: " << CAP_SBUS_SBUS_MSG << "DEFAULT PHY Config" << endl); //, CAP_SBUS_SBUS_MSG, $time);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD0:0x" << hlp.get_slc(data,0,23).convert_to<uint32_t>() << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[23:0]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD1:0x" << hlp.get_slc(data,24,47).convert_to<uint32_t>() << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[47:24]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " AWORD :0x" << hlp.get_slc(data,48,55).convert_to<uint32_t>() << endl); // 0b%08b" , CAP_SBUS_SBUS_MSG, $time, channel, data[55:48]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD2:0x" << hlp.get_slc(data,56,79).convert_to<uint32_t>() << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[79:56]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD3:0x" << hlp.get_slc(data,0,103).convert_to<uint32_t>() << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[103:80]);
#endif
#endif
  /*
  PLOG_MSG( "%0s: %t Channel:%0d -- PHY_CONFIG[ 22:21 , 5:4 , 2:0 ]: PBC_EN=%1b BPC_MODE=%1b PHY_PAR_LATENCY_B3=%1h T_RDLAT_OFFSET=%1h", CAP_SBUS_SBUS_MSG, $time, channel, data[ 22], data[ 21], data[ 4+:2], data[ 0+:3]);
  PLOG_MSG( "%0s: %t Channel:%0d -- PHY_CONFIG[ 46:45 ,29:28,26:24]: PBC_EN=%1b BPC_MODE=%1b PHY_PAR_LATENCY_B2=%1h T_RDLAT_OFFSET=%1h", CAP_SBUS_SBUS_MSG, $time, channel, data[ 46], data[ 45], data[28+:2], data[24+:3]);
  PLOG_MSG( "%0s: %t Channel:%0d -- PHY_CONFIG[ 78:77 ,61:60,58:56]: PBC_EN=%1b BPC_MODE=%1b PHY_PAR_LATENCY_B1=%1h T_RDLAT_OFFSET=%1h", CAP_SBUS_SBUS_MSG, $time, channel, data[ 78], data[ 77], data[60+:2], data[56+:3]);
  PLOG_MSG( "%0s: %t Channel:%0d -- PHY_CONFIG[102:101,85:84,82:80]: PBC_EN=%1b BPC_MODE=%1b PHY_PAR_LATENCY_B0=%1h T_RDLAT_OFFSET=%1h", CAP_SBUS_SBUS_MSG, $time, channel, data[102], data[101], data[84+:2], data[80+:3]);
  PLOG_MSG;
  */

#ifndef CAPRI_SW
#ifdef FIXME
  cpp_int temp_data;
  temp_data = hlp.set_slc(temp_data, static_cast<cpp_int>(0x5), 0, 2);
  temp_data = hlp.set_slc(temp_data, static_cast<cpp_int>(0x5), 24, 26);
  temp_data = hlp.set_slc(temp_data, static_cast<cpp_int>(0x5), 56, 58);
  temp_data = hlp.set_slc(temp_data, static_cast<cpp_int>(0x5), 80, 82);

  temp_data = hlp.set_slc(temp_data, static_cast<cpp_int>(0x0), 4, 5); // Byte 0, [   5:   4]
  temp_data = hlp.set_slc(temp_data, static_cast<cpp_int>(0x0), 28, 29); // Byte 1, [24+5:24+4]
  temp_data = hlp.set_slc(temp_data, static_cast<cpp_int>(0x0), 60, 61); // Byte 2, [56+5:56+4]
  temp_data = hlp.set_slc(temp_data, static_cast<cpp_int>(0x0), 84, 85); // Byte 3, [80+5:80+4]

  if (gatesim == 0) {
     temp_data = hlp.set_slc(temp_data, static_cast<cpp_int>(0x54fb605), 0, 31); // Byte 0, [   5:   4]
     temp_data = hlp.set_slc(temp_data, static_cast<cpp_int>(0x5fc4fb6), 32, 63); // Byte 1, [24+5:24+4]
     temp_data = hlp.set_slc(temp_data, static_cast<cpp_int>(0xb6054fb6), 64, 95); // Byte 2, [56+5:56+4]
     temp_data = hlp.set_slc(temp_data, static_cast<cpp_int>(0x4f), 96, 127); // Byte 3, [80+5:80+4]
  } else {
     temp_data = hlp.set_slc(temp_data, static_cast<cpp_int>(0x44fb604), 0, 31); // Byte 0, [   5:   4]
     temp_data = hlp.set_slc(temp_data, static_cast<cpp_int>(0x4fc4fb6), 32, 63); // Byte 1, [24+5:24+4]
     temp_data = hlp.set_slc(temp_data, static_cast<cpp_int>(0xb6044fb6), 64, 95); // Byte 2, [56+5:56+4]
     temp_data = hlp.set_slc(temp_data, static_cast<cpp_int>(0x4f), 96, 127); // Byte 3, [80+5:80+4]
  }

  data = temp_data.convert_to<pu_cpp_int<320> >();
#endif
#endif
  if (gatesim == 0) {
     data_32[0] = 0x54fb605;
     data_32[1] = 0x5fc4fb6;
     data_32[2] = 0xb6054fb6;
     data_32[3] = 0x4f;
  } else {
     data_32[0] = 0x44fb604;
     data_32[1] = 0x4fc4fb6;
     data_32[2] = 0xb6044fb6;
     data_32[3] = 0x4f;
  }

  /*
  // Change T_RDLAT_OFFSET for all bytes, all channels (determined by wir_write_channel)
  data = hlp.set_slc(data, static_cast<pu_cpp_int<320>>(0x5), 0, 2);
  data = hlp.set_slc(data, static_cast<pu_cpp_int<320>>(0x5), 26, 24);
  data = hlp.set_slc(data, static_cast<pu_cpp_int<320>>(0x5), 58, 56);
  data = hlp.set_slc(data, static_cast<pu_cpp_int<320>>(0x5), 82, 80);

  data = hlp.set_slc(data, static_cast<pu_cpp_int<320>>(0x0), 5, 4);   // Byte 0, [   5:   4]
  data = hlp.set_slc(data, static_cast<pu_cpp_int<320>>(0x0), 29, 28); // Byte 1, [24+5:24+4]
  data = hlp.set_slc(data, static_cast<pu_cpp_int<320>>(0x0), 61, 60); // Byte 2, [56+5:56+4]
  data = hlp.set_slc(data, static_cast<pu_cpp_int<320>>(0x0), 85, 84); // Byte 3, [80+5:80+4]
  */

  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Writing config PHY Config" << endl); //, CAP_SBUS_SBUS_MSG, $time);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD0:0x" << data_32[0] << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[23:0]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD1:0x" << data_32[0] << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[47:24]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " AWORD :0x" << data_32[0] << endl); // 0b%08b" , CAP_SBUS_SBUS_MSG, $time, channel, data[55:48]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD2:0x" << data_32[0] << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[79:56]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD3:0x" << data_32[0] << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[103:80]);


  // Write the phy config
  cap_nwl_sbus_wdr_write_data(data_32,CAP_SBUS_PHY_CONFIG_LENGTH,0);

  PEN_SLEEP(64);
  /*
  repeat (64) begin
     @(posedge `AVAGO_TOP_PATH.CLK_DIV2);
  end
  */

  // Read the phy config values to verify the change worked for a single
  // channel
  channel = 0;
  cap_nwl_sbus_wdr_read(CAP_SBUS_PHY_CONFIG_LENGTH,0);

  PEN_SLEEP(64);
  /*
  repeat (64) begin
     @(posedge `AVAGO_TOP_PATH.CLK_DIV2);
  end
  */

  cap_nwl_sbus_wdr_read_sbus_data(channel, CAP_SBUS_PHY_CONFIG_LENGTH, data_32);

  PEN_SLEEP(64);
  /*
  repeat (64) begin
     @(posedge `AVAGO_TOP_PATH.CLK_DIV2);
  end
  */
  
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Modified PHY Config" << endl); //, CAP_SBUS_SBUS_MSG, $time);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD0:0x" << data_32[0] << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[23:0]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD1:0x" << data_32[0] << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[47:24]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " AWORD :0x" << data_32[0] << endl); // 0b%08b" , CAP_SBUS_SBUS_MSG, $time, channel, data[55:48]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD2:0x" << data_32[0] << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[79:56]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD3:0x" << data_32[0] << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[103:80]);

#ifndef CAPRI_SW
 #ifdef FIXME 
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Modified PHY Config" << endl); //, CAP_SBUS_SBUS_MSG, $time);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD0:0x" << hlp.get_slc(data,0,31).convert_to<uint32_t>() << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[23:0]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD1:0x" << hlp.get_slc(data,32,63).convert_to<uint32_t>() << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[47:24]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " AWORD :0x" << hlp.get_slc(data,64,95).convert_to<uint32_t>() << endl); // 0b%08b" , CAP_SBUS_SBUS_MSG, $time, channel, data[55:48]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD2:0x" << hlp.get_slc(data,96,127).convert_to<uint32_t>() << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[79:56]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD3:0x" << hlp.get_slc(data,128,159).convert_to<uint32_t>() << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[103:80]);


  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Modified PHY Config" << endl); //, CAP_SBUS_SBUS_MSG, $time);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD0:0x" << hlp.get_slc(data,0,23).convert_to<uint32_t>() << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[23:0]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD1:0x" << hlp.get_slc(data,24,47).convert_to<uint32_t>() << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[47:24]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " AWORD :0x" << hlp.get_slc(data,48,55).convert_to<uint32_t>() << endl); // 0b%08b" , CAP_SBUS_SBUS_MSG, $time, channel, data[55:48]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD2:0x" << hlp.get_slc(data,56,79).convert_to<uint32_t>() << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[79:56]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD3:0x" << hlp.get_slc(data,0,103).convert_to<uint32_t>() << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[103:80]);
#endif
#endif

  /*
  PLOG_MSG( CAP_SBUS_SBUS_MSG << ": Channel:" << hex << channel << " -- PHY_CONFIG[ 22:21 , 5:4 , 2:0 ]: PBC_EN=%1b BPC_MODE=%1b PHY_PAR_LATENCY_B3=%1h T_RDLAT_OFFSET=%1h", CAP_SBUS_SBUS_MSG, $time, channel, data[ 22], data[ 21], data[ 4+:2], data[ 0+:3]);
  PLOG_MSG( CAP_SBUS_SBUS_MSG << ": Channel:" << hex << channel << " -- PHY_CONFIG[ 46:45 ,29:28,26:24]: PBC_EN=%1b BPC_MODE=%1b PHY_PAR_LATENCY_B2=%1h T_RDLAT_OFFSET=%1h", CAP_SBUS_SBUS_MSG, $time, channel, data[ 46], data[ 45], data[28+:2], data[24+:3]);
  PLOG_MSG( CAP_SBUS_SBUS_MSG << ": Channel:" << hex << channel << " -- PHY_CONFIG[ 78:77 ,61:60,58:56]: PBC_EN=%1b BPC_MODE=%1b PHY_PAR_LATENCY_B1=%1h T_RDLAT_OFFSET=%1h", CAP_SBUS_SBUS_MSG, $time, channel, data[ 78], data[ 77], data[60+:2], data[56+:3]);
  PLOG_MSG( CAP_SBUS_SBUS_MSG << ": Channel:" << hex << channel << " -- PHY_CONFIG[102:101,85:84,82:80]: PBC_EN=%1b BPC_MODE=%1b PHY_PAR_LATENCY_B0=%1h T_RDLAT_OFFSET=%1h", CAP_SBUS_SBUS_MSG, $time, channel, data[102], data[101], data[84+:2], data[80+:3]);
  PLOG_MSG;
  */
  
  // Read the phy config values to verify the change worked for a single
  // channel
  channel = 1;
  cap_nwl_sbus_wdr_read(CAP_SBUS_PHY_CONFIG_LENGTH,0);

  PEN_SLEEP(64);
  /*
  repeat (64) begin
     @(posedge `AVAGO_TOP_PATH.CLK_DIV2);
  end
  */

  cap_nwl_sbus_wdr_read_sbus_data(channel, CAP_SBUS_PHY_CONFIG_LENGTH, data_32);

  PEN_SLEEP(64);
  /*
  repeat (64) begin
     @(posedge `AVAGO_TOP_PATH.CLK_DIV2);
  end
  */

  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Modified PHY Config" << endl); //, CAP_SBUS_SBUS_MSG, $time);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD0:0x" << data_32[0] << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[23:0]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD1:0x" << data_32[0] << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[47:24]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " AWORD :0x" << data_32[0] << endl); // 0b%08b" , CAP_SBUS_SBUS_MSG, $time, channel, data[55:48]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD2:0x" << data_32[0] << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[79:56]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD3:0x" << data_32[0] << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[103:80]);

#ifndef CAPRI_SW
#ifdef FIXME
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "MODIFIED PHY Config" << endl); //, CAP_SBUS_SBUS_MSG, $time);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD0:0x" << hlp.get_slc(data,0,23).convert_to<uint32_t>() << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[23:0]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD1:0x" << hlp.get_slc(data,24,47).convert_to<uint32_t>() << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[47:24]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " AWORD :0x" << hlp.get_slc(data,48,55).convert_to<uint32_t>() << endl); // 0b%08b" , CAP_SBUS_SBUS_MSG, $time, channel, data[55:48]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD2:0x" << hlp.get_slc(data,56,79).convert_to<uint32_t>() << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[79:56]);
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << "Channel:" << hex << channel << " DWORD3:0x" << hlp.get_slc(data,0,103).convert_to<uint32_t>() << endl); // 0b%024b", CAP_SBUS_SBUS_MSG, $time, channel, data[103:80]);

#endif
#endif
  /*
  PLOG_MSG( "%0s: %t Channel:" << hex << channel << " -- PHY_CONFIG[ 22:21 , 5:4 , 2:0 ]: PBC_EN=%1b BPC_MODE=%1b PHY_PAR_LATENCY_B3=%1h T_RDLAT_OFFSET=%1h", CAP_SBUS_SBUS_MSG, $time, channel, data[ 22], data[ 21], data[ 4+:2], data[ 0+:3]);
  PLOG_MSG( "%0s: %t Channel:" << hex << channel << " -- PHY_CONFIG[ 46:45 ,29:28,26:24]: PBC_EN=%1b BPC_MODE=%1b PHY_PAR_LATENCY_B2=%1h T_RDLAT_OFFSET=%1h", CAP_SBUS_SBUS_MSG, $time, channel, data[ 46], data[ 45], data[28+:2], data[24+:3]);
  PLOG_MSG( "%0s: %t Channel:" << hex << channel << " -- PHY_CONFIG[ 78:77 ,61:60,58:56]: PBC_EN=%1b BPC_MODE=%1b PHY_PAR_LATENCY_B1=%1h T_RDLAT_OFFSET=%1h", CAP_SBUS_SBUS_MSG, $time, channel, data[ 78], data[ 77], data[60+:2], data[56+:3]);
  PLOG_MSG( "%0s: %t Channel:" << hex << channel << " -- PHY_CONFIG[102:101,85:84,82:80]: PBC_EN=%1b BPC_MODE=%1b PHY_PAR_LATENCY_B0=%1h T_RDLAT_OFFSET=%1h", CAP_SBUS_SBUS_MSG, $time, channel, data[102], data[101], data[84+:2], data[80+:3]);
  PLOG_MSG;
  */

}


void cap_nwl_sbus_test_PHY_soft_lane_repair() {
  uint32_t data_32[10];
#ifndef CAPRI_SW 
  pu_cpp_int<72> soft_lane_repair;
  cpp_int_helper hlp;
#endif
  uint32_t channel;


  PLOG_MSG("CAP_NWL_SBUS_API: cap_nwl_sbus_test_PHY_soft_lane_repair called" << endl);

  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << ": Read PHY Soft Lane Repair             " << endl);

  // Set the 1500 WIR to write the mode registers for all channels
  channel = 0xf;
  cap_nwl_sbus_wir_write_channel( CAP_SBUS_PHY_SEL, channel, CAP_SBUS_PHY_SOFT_LANE_REPAIR, CAP_SBUS_PHY_SOFT_LANE_REPAIR_LENGTH);

  // Read the phy soft lane repair values for a single channel
  channel = 0;
  cap_nwl_sbus_wdr_read(CAP_SBUS_PHY_SOFT_LANE_REPAIR_LENGTH,0);
  cap_nwl_sbus_wdr_read_sbus_data(channel, CAP_SBUS_PHY_SOFT_LANE_REPAIR_LENGTH, data_32);

#ifndef CAPRI_SW 
#ifdef FIXME
  // FIXME 
  soft_lane_repair = hlp.get_slc(data,0,71).convert_to<pu_cpp_int<72>>();

  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << ": PHY Soft Lane Repair: " << soft_lane_repair << endl);
#else
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << ": PHY Soft Lane Repair: " << data_32[2] << data_32[1] << data_32[0]  << endl);
#endif
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << ": Read PHY Soft Lane Repair ------- DONE" << endl);
#endif

}



//----------------------------------------
// Performs a 1500 read operation.  This reads the 1500 registers for all
// selected channel in parallel and stores the results in the APC.  The
// wdr_read_sbus_data must be used to fetch the data from the APC.
//----------------------------------------
void cap_nwl_sbus_wdr_read(uint32_t length,
	      uint32_t shift_only) {

  uint32_t error;
  uint32_t rd_data;


  if (length == 0) {
    //length = current_instr_length; // FIXME Suresh
    PLOG_ERR("cap_nwl_sbus_wdr_read: " <<  "length is zero." << endl);
  }

  avago_top_sbus_command(CAP_SBUS_SBUS_WR, CAP_SBUS_CONTROL_1500, CAP_SBUS_STOP_1500, &rd_data);

  if (shift_only == 1) {
    avago_top_sbus_command(CAP_SBUS_SBUS_WR, CAP_SBUS_CONTROL_1500, (length<<4 | CAP_SBUS_SHIFT_READ_WDR), &rd_data);
  }
  else {
    avago_top_sbus_command(CAP_SBUS_SBUS_WR, CAP_SBUS_CONTROL_1500, (length<<4 | CAP_SBUS_START_WDR_READ), &rd_data);
  }
  cap_nwl_sbus_apc_1500_busy_done_handshake(&error);

}

//----------------------------------------
// Sets the current 1500 device, channel and instruction.  The instruction
// length is passed in, but is not used in this task.  Instead it is saved for
// later use on other tasks.
//----------------------------------------
void cap_nwl_sbus_wir_write_channel(uint32_t  device_sel,
		       uint32_t  channel,
		       uint32_t  instruction,
		       uint32_t instr_length
		       ) {
  uint32_t error;
  uint32_t rd_data;
  
  // Save the instruction for later
  //current_instr_length = instr_length; // FIXME Suresh

  avago_top_sbus_command(CAP_SBUS_SBUS_WR, CAP_SBUS_WIR_REG, ((device_sel<<12) | (channel<<8) | instruction), &rd_data);
  avago_top_sbus_command(CAP_SBUS_SBUS_WR, CAP_SBUS_CONTROL_1500, 0x1, &rd_data);
  
  cap_nwl_sbus_apc_1500_busy_done_handshake(&error);
  
}


//----------------------------------------
// Performs a 1500 read operation.  This reads the 1500 registers for all
// selected channel in parallel and stores the results in the APC.  The
// wdr_read_sbus_data must be used to fetch the data from the APC.
//----------------------------------------
void cap_nwl_sbus_wdr_read_sbus_data(uint32_t   channel,
			uint32_t length,
			uint32_t *result_32
			) { 
  
#ifdef FIXME
  pu_cpp_int<320>  mask;
#endif
  uint32_t  rd_data;

#ifdef FIXME
  cpp_int_helper hlp;
  cpp_int temp_data;
#endif

  if (length == 0) {
    //length = current_instr_length; // FIXME Suresh
    PLOG_ERR("cap_nwl_sbus_wdr_read_sbus_data: " <<  "length is zero." << endl);
  }

  //PLOG_MSG("CAP_NWL_SBUS_API: after  " << __LINE__ << " File=" << __FILE__ << endl);
  avago_top_sbus_command(CAP_SBUS_SBUS_WR, CAP_SBUS_READ_CHANNEL, 1<<channel, &rd_data);
  //PLOG_MSG("CAP_NWL_SBUS_API: after  " << __LINE__ << " File=" << __FILE__ << endl);
  for (uint32_t wr_bits = 0; wr_bits < 10; wr_bits = wr_bits+1) {    
    result_32[wr_bits] = 0;
  }
  //PLOG_MSG("CAP_NWL_SBUS_API: after  " << __LINE__ << " File=" << __FILE__ << endl);
  rd_data = 0;
  for (uint32_t wr_bits = 0; wr_bits < length; wr_bits = wr_bits+32) {    
    switch (wr_bits)
      {
      case 0   :  avago_top_sbus_command(CAP_SBUS_SBUS_RD, 20, 0, &rd_data); break;
      case 32  :  avago_top_sbus_command(CAP_SBUS_SBUS_RD, 21, 0, &rd_data); break;
      case 64  :  avago_top_sbus_command(CAP_SBUS_SBUS_RD, 22, 0, &rd_data); break;
      case 96  :  avago_top_sbus_command(CAP_SBUS_SBUS_RD, 23, 0, &rd_data); break;
      case 128 :  avago_top_sbus_command(CAP_SBUS_SBUS_RD, 24, 0, &rd_data); break;
      case 160 :  avago_top_sbus_command(CAP_SBUS_SBUS_RD, 25, 0, &rd_data); break;
      case 192 :  avago_top_sbus_command(CAP_SBUS_SBUS_RD, 26, 0, &rd_data); break;
      case 224 :  avago_top_sbus_command(CAP_SBUS_SBUS_RD, 27, 0, &rd_data); break;
      case 256 :  avago_top_sbus_command(CAP_SBUS_SBUS_RD, 28, 0, &rd_data); break;
      case 288 :  avago_top_sbus_command(CAP_SBUS_SBUS_RD, 29, 0, &rd_data); break;
      }

    result_32[wr_bits/32] = rd_data;
  //PLOG_MSG("CAP_NWL_SBUS_API: after  " << __LINE__ << " File=" << __FILE__ << endl);
#ifdef FIXME
    temp_data = hlp.set_slc(temp_data, static_cast<cpp_int>(rd_data), wr_bits, wr_bits+31);
#endif
  }

#ifndef CAPRI_SW
#ifdef FIXME
  // Mask out the unused bits since they could have garbage data in them
  mask = (1 << length) -1;
  result = result & mask;
#endif
#endif
  //PLOG_MSG("CAP_NWL_SBUS_API: after  " << __LINE__ << " File=" << __FILE__ << endl);

}

//----------------------------------------
// Write the 1500 regsiter data to the APC via sbus and then launches the 1500
// write operation.  The wir_write task must have been called prior to this to
// setup the WIR.  By default the full instruction length is scanned.
// Alternatively, a length and the shift_only bit can be set to perform
// multi-segment scans.
//----------------------------------------
void cap_nwl_sbus_wdr_write_data(uint32_t *data,
		    uint32_t length,
		    uint32_t shift_only) {
  uint32_t error;
  uint32_t rd_data;


  if (length == 0) {
    // length = current_instr_length; // FIXME Suresh
    PLOG_ERR("cap_nwl_sbus_wdr_write_data: " <<  "length is zero." << endl);
  }

  for (uint32_t wr_bits = 0; wr_bits < length; wr_bits = wr_bits+32) {
    switch (wr_bits) {
    case 0   :  avago_top_sbus_command(CAP_SBUS_SBUS_WR, 4,  data[0], &rd_data); break;
    case 32  :  avago_top_sbus_command(CAP_SBUS_SBUS_WR, 5,  data[1], &rd_data); break;
    case 64  :  avago_top_sbus_command(CAP_SBUS_SBUS_WR, 6,  data[2], &rd_data); break;
    case 96  :  avago_top_sbus_command(CAP_SBUS_SBUS_WR, 7,  data[3], &rd_data); break;
    case 128 :  avago_top_sbus_command(CAP_SBUS_SBUS_WR, 8,  data[4], &rd_data); break;
    case 160 :  avago_top_sbus_command(CAP_SBUS_SBUS_WR, 9,  data[5], &rd_data); break;
    case 192 :  avago_top_sbus_command(CAP_SBUS_SBUS_WR, 10, data[6], &rd_data); break;
    case 224 :  avago_top_sbus_command(CAP_SBUS_SBUS_WR, 11, data[7], &rd_data); break;
    case 256 :  avago_top_sbus_command(CAP_SBUS_SBUS_WR, 12, data[8], &rd_data); break;
    case 288 :  avago_top_sbus_command(CAP_SBUS_SBUS_WR, 13, data[9], &rd_data); break;
    }
  }

  if (shift_only == 1) {
    avago_top_sbus_command(CAP_SBUS_SBUS_WR, CAP_SBUS_CONTROL_1500, (length<<4 | CAP_SBUS_SHIFT_WRITE_WDR), &rd_data);
  }
  else {
    avago_top_sbus_command(CAP_SBUS_SBUS_WR, CAP_SBUS_CONTROL_1500, (length<<4 | CAP_SBUS_START_WDR_WRITE), &rd_data);
  }

  cap_nwl_sbus_apc_1500_busy_done_handshake(&error);
  
}

//----------------------------------------
// Wait for an expected value on the 1500 DONE bit. 
//----------------------------------------
void cap_nwl_sbus_wait_for_1500_done(uint32_t expected, uint32_t* error) {

  uint32_t done;
  uint32_t rd_data;
  uint32_t timeout;

  *error = 0;
  timeout = 500*4;
  done = expected ^ 1;

  PLOG("nwl_sbus_api", "CAP_NWL_SBUS_API: " << "Waiting for " << timeout << " time, expected: " << expected << endl);
   
  while ((done != expected) && timeout > 0) {
    avago_top_sbus_command(CAP_SBUS_SBUS_RD, CAP_SBUS_BUSY_DONE_1500_REG, 0, &rd_data);
    done = rd_data & 1;
    timeout = timeout - 1;
    
    if(done != expected) { PEN_SLEEP(10); }

    PLOG("nwl_sbus_api", "CAP_NWL_SBUS_API: " << "expected: " << expected << ", done: " << done << endl);
    ///repeat (10) @(posedge `AVAGO_TOP_PATH.CLK_DIV2);
  }
  
  if (timeout == 0) {
    PLOG_ERR("CAP_NWL_SBUS_API: " <<  "Timeout while waiting for 1500 DONE." << endl);
    *error = 1;
  }
}

//----------------------------------------
// After 1500 operations are launched, a handshake must be performed to ensure
// the commands are properly captured going from the sbus to the 1500 clock
// domain.
//----------------------------------------
void cap_nwl_sbus_apc_1500_busy_done_handshake(uint32_t * error) {
  uint32_t step_error;
  uint32_t rd_data;

  cap_nwl_sbus_wait_for_1500_done(1, &step_error);
  *error = *error | step_error;

  avago_top_sbus_command(CAP_SBUS_SBUS_WR, CAP_SBUS_CONTROL_1500, CAP_SBUS_STOP_1500, &rd_data);

  cap_nwl_sbus_wait_for_1500_done(0, &step_error);
  *error = *error | step_error;
}

void cap_nwl_sbus_test_1500_device_id(uint32_t *data_32, uint32_t *is_8g_8hi, uint32_t *is_samsung) {
#ifdef FIXME
  pu_cpp_int<320> data;
  pu_cpp_int<82> device_id;
#endif
  uint32_t   channel;


  PLOG_MSG("CAP_NWL_SBUS_API: cap_nwl_sbus_test_1500_device_id called" << endl);

  PLOG_MSG("CAP_NWL_SBUS_API: " << CAP_SBUS_HBM_1500_MSG << ": Read HBM Device ID                    " << endl);
  // Set the 1500 WIR to read the HBM device ID from channel 0
  channel = 0x0;
  cap_nwl_sbus_wir_write_channel( CAP_SBUS_HBM_SEL, channel, CAP_SBUS_HBM_DEVICE_ID, CAP_SBUS_HBM_DEVICE_ID_LENGTH);

  // Perform a 1500 read.
  cap_nwl_sbus_wdr_read(CAP_SBUS_HBM_DEVICE_ID_LENGTH,0);

  cap_nwl_sbus_wdr_read_sbus_data(channel, CAP_SBUS_HBM_DEVICE_ID_LENGTH, data_32);

#ifdef FIXME
  device_id = hlp.get_slc(data,0,81).convert_to<pu_cpp_int<82>>();

  PLOG_MSG("CAP_NWL_SBUS_API: " << CAP_SBUS_HBM_1500_MSG << ": HBM Device ID: 0x" << hex << device_id << endl);
#else
  PLOG_MSG("CAP_NWL_SBUS_API: " << CAP_SBUS_HBM_1500_MSG << ": HBM Device ID: 0x" << hex << data_32[2] << data_32[1] << data_32[0] << endl);
  if(!(data_32[0] & 0x80)) {
    PLOG_MSG("CAP_NWL_SBUS_API: " << CAP_SBUS_HBM_1500_MSG << ": HBM Device ID: 2HI/4HI" << dec << endl);
    SW_PRINT("HBM Device ID: 2HI/4HI\n");
    *is_8g_8hi = 0;
  } else {
    PLOG_MSG("CAP_NWL_SBUS_API: " << CAP_SBUS_HBM_1500_MSG << ": HBM Device ID: 8HI" << dec << endl);
    SW_PRINT("HBM Device ID: 8HI\n");
    *is_8g_8hi = 1;
  }
  if(((data_32[2]>>8)&0xf) == 0x1) {
    PLOG_MSG("CAP_NWL_SBUS_API: " << CAP_SBUS_HBM_1500_MSG << ": HBM Device ID: Samsung" << dec << endl);
    SW_PRINT("HBM Device ID: Samsung\n");
    *is_samsung = 1;
  } else {
   if(((data_32[2]>>8)&0xf) == 0x6) {
    PLOG_MSG("CAP_NWL_SBUS_API: " << CAP_SBUS_HBM_1500_MSG << ": HBM Device ID: SK Hynix" << dec << endl);
    SW_PRINT("HBM Device ID: SK Hynix\n");
    *is_samsung = 0;
   } else {
    PLOG_MSG("CAP_NWL_SBUS_API: " << CAP_SBUS_HBM_1500_MSG << ": HBM Device ID: Unknown" << dec << endl);
    SW_PRINT("HBM Device ID: Unknown Vendor\n");
   }
  }
#endif
  PLOG_MSG("CAP_NWL_SBUS_API: " << CAP_SBUS_HBM_1500_MSG << ": Read HBM Device ID -------------- DONE" << endl);

}


uint32_t cap_nwl_sbus_get_1500_temperature() {
#ifdef FIXME
  pu_cpp_int<320> data;
#endif
  uint32_t data_32[10];
  uint32_t channel;
  uint32_t valid;
  uint32_t temp = 0;
  uint32_t count;
  uint32_t valid_bit;

#ifdef FIXME
  cpp_int_helper hlp;
#endif

  PLOG("nwl_sbus_api", "CAP_NWL_SBUS_API: cap_nwl_sbus_get_1500_temperature called" << endl);

  PLOG("nwl_sbus_api", "CAP_NWL_SBUS_API: " << CAP_SBUS_HBM_1500_MSG <<  "Read HBM temperature                  "<< endl);
  // Set the 1500 WIR to read the HBM temperature from all channels
  channel = 0xf;
  cap_nwl_sbus_wir_write_channel( CAP_SBUS_HBM_SEL, channel, CAP_SBUS_HBM_TEMPERATURE, CAP_SBUS_HBM_TEMPERATURE_LENGTH);

  // Perform a 1500 read.
  cap_nwl_sbus_wdr_read(CAP_SBUS_HBM_TEMPERATURE_LENGTH,0);

// JEDEC Spec                                                                               
// =========================================================================================
//              |   Bit    |     Bit     |      |                                           
//  Temperature | Position |    Field    | Type | Description                               
// =========================================================================================
//              |          |             |      | Temperature Sensor Output Valid           
//              |    7     |  VALID[0]   |   R  | 0 - Valid                                 
//              |          |             |      | 1 - Invalid                               
// =========================================================================================
//              |          |             |      | Temperature in Degree (C)                 
//              |   6-0    |  TEMP[6:0]  |   R  | 7'b000_0000 =   0 C (or less)             
//              |          |             |      | 7'b001_1001 =  25 C                       
//              |          |             |      | 7'b111_1111 = 127 C                       
// =========================================================================================
//   Only read out valid temps                                                              

  // memory models that are spec-compliant on the memory Valid bit polarity    
  valid_bit = 0; // FIXME - Suresh
  valid = 0;
  count  = 0;
  while ((valid == valid_bit) && (count < 1000) ) {
    ++count;
    cap_nwl_sbus_wdr_read_sbus_data(channel, CAP_SBUS_HBM_TEMPERATURE_LENGTH, data_32);
#ifdef FIXME
    valid = hlp.get_slc(data,7,7).convert_to<uint32_t>();
    temp  = hlp.get_slc(data,0,6).convert_to<uint32_t>();
#else
    valid = (data_32[0] >> 7) & 0x1;
    temp = (data_32[0] ) & 0x7f;
#endif
  }

  PLOG("nwl_sbus_api", "CAP_NWL_SBUS_API: " << CAP_SBUS_HBM_1500_MSG << ": Read HBM temperature ------------ DONE" << endl);
  //SW_PRINT("NWL:1500:HBM temperature %d\n", temp);
  //temp  = temp + 1; // FIXME

  return(temp);
}

void cap_nwl_sbus_test_1500_temperature() {
  uint32_t temp = cap_nwl_sbus_get_1500_temperature();
  PLOG_MSG("CAP_NWL_SBUS_API: " << CAP_SBUS_HBM_1500_MSG << ": HBM temperature: " << temp << endl << endl);
  temp = temp + 1; // make the compiler shut up
}

void cap_nwl_sbus_test_1500_HBM_soft_lane_repair() {
#ifdef FIXME
  pu_cpp_int<320> data;
  pu_cpp_int<72>  soft_lane_repair;
#endif
  uint32_t        data_32[10];
  uint32_t  channel;
  
#ifdef FIXME
  cpp_int_helper hlp;
#endif

  PLOG_MSG("CAP_NWL_SBUS_API: cap_nwl_sbus_test_1500_HBM_soft_lane_repair called" << endl);

  PLOG_MSG("CAP_NWL_SBUS_API: " << CAP_SBUS_HBM_1500_MSG << ": Read HBM Soft Lane Repair             " << endl);
  // Set the 1500 WIR to read the HBM soft lane repair from channel 0
  channel = 0x0;
  cap_nwl_sbus_wir_write_channel( CAP_SBUS_HBM_SEL, channel, CAP_SBUS_HBM_SOFT_LANE_REPAIR, CAP_SBUS_HBM_SOFT_LANE_REPAIR_LENGTH);

  // Perform a 1500 read.
  cap_nwl_sbus_wdr_read(CAP_SBUS_HBM_SOFT_LANE_REPAIR_LENGTH,0);

  cap_nwl_sbus_wdr_read_sbus_data(channel, CAP_SBUS_HBM_SOFT_LANE_REPAIR_LENGTH, data_32);
#ifdef FIXME
  soft_lane_repair = hlp.get_slc(data,0,71).convert_to<pu_cpp_int<72>>();

  PLOG_MSG("CAP_NWL_SBUS_API: " << CAP_SBUS_HBM_1500_MSG << ": HBM Soft Lane Repair: 0x" << soft_lane_repair << endl);
#else
  PLOG_MSG("CAP_NWL_SBUS_API: " <<  CAP_SBUS_SBUS_MSG << ": PHY Soft Lane Repair: " << data_32[2] << data_32[1] << data_32[0]  << endl);
#endif
  PLOG_MSG("CAP_NWL_SBUS_API: " << CAP_SBUS_HBM_1500_MSG << ": Read HBM Soft Lane Repair ------- DONE" << endl);
}


#ifndef CAPRI_SW
//
// HBM FIRMWARE OPERATION API
//

//----------------------------------------
// Convert a firmware error code to a description
//----------------------------------------
string get_error_code_desc( int err_code) {

string ret_desc;

  switch (err_code) {
    case 0:   ret_desc = "NO_ERROR"; break;  
    case 1:   ret_desc = "ERROR_DETECTED"; break; 
    case 2:   ret_desc = "ERROR_UNEXPECTED_RESET_STATE"; break; 
    case 3:   ret_desc = "ERROR_ILLEGAL_CHANNEL_NUMBER"; break; 
    case 4:   ret_desc = "ERROR_TIMEOUT_WAITING_FOR_1500_DONE"; break; 
    case 5:   ret_desc = "ERROR_TIMEOUT_WAITING_FOR_BIST_DONE"; break; 
    case 6:   ret_desc = "ERROR_DATA_COMPARE_FAILED"; break; 
    case 7:   ret_desc = "ERROR_ALL_CHANNELS_NOT_SELECTED_FOR_RESET"; break; 
    case 8:   ret_desc = "ERROR_REPAIR_LIMIT_EXCEEDED"; break; 
    case 9:   ret_desc = "ERROR_NON_REPAIRABLE_FAULTS_FOUND"; break; 
    case 10:  ret_desc = "ERROR_MBIST_FAILED"; break; 
    case 11:  ret_desc = "ERROR_EXCEEDED_BANK_REPAIR_LIMIT"; break; 
    case 12:  ret_desc = "ERROR_ALL_CHANNELS_NOT_ENABLED"; break; 
    case 13:  ret_desc = "ERROR_TIMEOUT_WAITING_FOR_PHYUPD_HANDSHAKE"; break; 
    case 14:  ret_desc = "ERROR_CHANNEL_UNREPAIRABLE"; break; 
    case 15:  ret_desc = "ERROR_NO_FUSES_AVAILBALE_FOR_REPAIR"; break; 
    case 16:  ret_desc = "ERROR_TIMEOUT_WAITING_FOR_VALID_TEMP"; break; 
    case 17:  ret_desc = "ERROR_CHANNEL_FAILURES_EXIST"; break; 
    case 18:  ret_desc = "ERROR_UNKNOWN_ERROR"; break; 
    case 19:  ret_desc = "ERROR_TIMEOUT_WAITING_FOR_NWL_INIT"; break; 
    case 20:  ret_desc = "ERROR_CTC_WRITE_READ_COMPARE_FAILURE"; break; 
    case 21:  ret_desc = "ERROR_CTC_NO_WRITES_PERFORMED"; break; 
    case 22:  ret_desc = "ERROR_CTC_NO_READS_PERFORMED"; break; 
    case 23:  ret_desc = "ERROR_LANE_ERRORS_DETECTED"; break; 
    case 24:  ret_desc = "ERROR_CTC_TIMEOUT_WAITING_FOR_CTC_BUSY"; break; 
    case 25:  ret_desc = "ERROR_UNSUPPORTED_HBM_CONFIGURATION"; break; 
    default : ret_desc = "UNRECOGNIZED_ERROR_CODE"; break; 
  }
  return ret_desc;
}


//----------------------------------------
// Convert a firmware operation code to a description
//----------------------------------------
string get_operation_desc( int operation) {

string desc;
  
  switch(operation) {
    case 0x00 : desc = "OP_SUCCESS"; break; 
    case 0x01 : desc = "OP_BYPASS"; break; 
    case 0x02 : desc = "OP_DEVICE_ID"; break; 
    case 0x03 : desc = "OP_AWORD"; break; 
    case 0x04 : desc = "OP_AERR"; break; 
    case 0x05 : desc = "OP_AWORD_ILB"; break; 
    case 0x06 : desc = "OP_AERR_ILB"; break; 
    case 0x07 : desc = "OP_AERR_INJ_ILB"; break; 
    case 0x08 : desc = "OP_DWORD_WRITE"; break; 
    case 0x09 : desc = "OP_DWORD_READ"; break; 
    case 0x0a : desc = "OP_DERR"; break; 
    case 0x0b : desc = "OP_DWORD_UPPER_ILB"; break; 
    case 0x0c : desc = "OP_DWORD_LOWER_ILB"; break; 
    case 0x0d : desc = "OP_DERR_ILB"; break; 
    case 0x0e : desc = "OP_DERR_IBJ_ILB"; break; 
    case 0x0f : desc = "OP_LANE_REPAIR"; break; 
    case 0x10 : desc = "OP_DEVICE_TEMP"; break; 
    case 0x11 : desc = "OP_CONNECTIVITY_CHECK"; break; 
    case 0x12 : desc = "OP_RESET"; break; 
    case 0x13 : desc = "OP_MBIST"; break; 
    case 0x14 : desc = "OP_BITCELL_REPAIR"; break; 
    case 0x15 : desc = "OP_AWORD_SLB"; break; 
    case 0x16 : desc = "OP_AERR_SLB"; break; 
    case 0x17 : desc = "OP_AERR_INJ_SLB"; break; 
    case 0x18 : desc = "OP_DWORD_UPPER_SLB"; break; 
    case 0x19 : desc = "OP_DWORD_LOWER_SLB"; break; 
    case 0x1a : desc = "OP_DERR_SLB"; break; 
    case 0x1b : desc = "OP_DERR_INJ_SLB"; break; 
    case 0x1c : desc = "OP_TMRS"; break; 
    case 0x1d : desc = "OP_CHIPPING"; break; 
    case 0x1e : desc = "OP_MC_INIT"; break; 
    case 0x1f : desc = "OP_CTC"; break; 
    case 0x20 : desc = "OP_APPLY_LANE_REPAIR"; break; 
    case 0x21 : desc = "OP_BURN_LANE_REPAIR"; break; 
    default   : desc = "UNRECOGNIZED_OPERATION"; break; 
  }
  return desc;
}


//----------------------------------------
// Run a firmware-based hbm operation
//
//   Operation Values
//   0x00 - Device Connectivity Check
//
//   Result Addresses
//   0x00 - Operation Status
//            bit[0] - Operation done
//            bit[1] - Operation active
//            bit[1] - Operation errors detected
//   0x01 - Global Error Code
//   0x02 - Channel 0 Error Code
//   0x03 - Channel 1 Error Code
//   0x04 - Channel 2 Error Code
//   0x05 - Channel 3 Error Code
//   0x06 - Channel 4 Error Code
//   0x07 - Channel 5 Error Code
//   0x08 - Channel 6 Error Code
//   0x09 - Channel 7 Error Code
//----------------------------------------
int cap_nwl_hbm_firmware_operation(int operation) {

  int  err;
  int  data;
  int  global_error_code;
  int  channel_error_code[8];
  int  channel_operation[8];

  string global_error_code_desc;
  string channel_error_code_desc[8];
  string channel_operation_desc[8];

  // Run a firmware test operation
  data = sbus_master_spico_interrupt(0, 1, 0xfd, 0x30, operation, 0, 0, 100);
  if ( (data == 1) && (data != 0x3ff) ) {
    PLOG_ERR("CAP_NWL_SBUS::SPICO_INT: Interrupt 0x30 returned successfully" << endl);
  } else {
    PLOG_ERR("CAP_NWL_SBUS::SPICO_INT: Interrupt 0x30 returned error :" << data << endl);
  }

  // Wait for the firmware operation to complete.  Read the status result.
  //     bit[0] - Operation Done
  //     bit[1] - Operation Active
  //     bit[2] - Operation Error Results
  data = sbus_master_spico_interrupt(0, 1, 0xfd, 0x32, 0, 0, 0, 100);

  while ((data & 0x3) == 0x2) {
    PEN_SLEEP(100);
    data = sbus_master_spico_interrupt(0, 1, 0xfd, 0x32, 0, 0, 0, 100);
  }

  // Set the error return value
  err = (data & 0x4) >> 2;

  // Print an error status
  if (err == 0) {
    PLOG_MSG("CAP_NWL_SBUS::SPICO_INT: completed successfully: operation : " << operation << endl);
  } else {
    PLOG_ERR("CAP_NWL_SBUS::SPICO_INT: did not complete successfully: operation : " << operation << endl);
  }

  // If an operation error occurred, check the error codes
  if (err == 1) {
    global_error_code   = sbus_master_spico_interrupt(0, 1, 0xfd, 0x32, 1, 0, 0, 100);
    for (int i=0; i < 8; i++) {
       channel_error_code[i] = sbus_master_spico_interrupt(0, 1, 0xfd, 0x32, i+2, 0, 0, 100);
    }
    for (int i=0; i < 8; i++) {
       channel_operation[i] = sbus_master_spico_interrupt(0, 1, 0xfd, 0x32, i+10, 0, 0, 100);
    }

    global_error_code_desc   = get_error_code_desc(global_error_code);
    PLOG_MSG("CAP_NWL_SBUS:: global_error_code" << global_error_code_desc << endl);

    for (int i=0; i < 8; i++) {
       channel_error_code_desc[i] = get_error_code_desc(channel_error_code[i]);
       channel_operation_desc[i] = get_operation_desc(channel_operation[i]);
       PLOG_MSG("CAP_NWL_SBUS:: channel: " << i << " error_code:" << channel_error_code_desc[i] << " operation:" << channel_operation_desc[i] << endl);
    }
  }
  return err;
}
#endif

//----------------------------------------
// Sets an HBM firmware parameter offset and value
//----------------------------------------
int cap_nwl_set_hbm_parameter(int offset, int value) {

int data;
int err = 0;

   data = sbus_master_spico_interrupt(0, 1, 0xfd, 0x34, offset, 0, 0, 100);
   if (data == 0x1) {
      PLOG_MSG("CAP_NWL_SBUS::SPICO_INT: Set Param Offset completed successfully: " << endl);
   } else {
      PLOG_ERR("CAP_NWL_SBUS::SPICO_INT: Set Param Offset did not complete successfully: " << endl);
      err++;
   }
   data = sbus_master_spico_interrupt(0, 1, 0xfd, 0x35, value, 0, 0, 100);
   if (data == 0x1) {
      PLOG_MSG("CAP_NWL_SBUS::SPICO_INT: Set Param Value completed successfully: " << endl);
   } else {
      PLOG_ERR("CAP_NWL_SBUS::SPICO_INT: Set Param Value did not complete successfully: " << endl);
      err++;
   }

   return err;
}

#ifndef CAPRI_SW
int test_firmware_aerr(void) {

  // Reduce the Spico tINIT parameters for faster simulation
  cap_nwl_set_hbm_parameter(HBM_TINIT1_CYCLES, 10);
  cap_nwl_set_hbm_parameter(HBM_TINIT3_CYCLES, 10);
  cap_nwl_set_hbm_parameter(HBM_TINIT5_CYCLES, 10);

  // Set firmware div mode.
  //`ifdef DIV2_MODE
  cap_nwl_set_hbm_parameter(HBM_DIV_MODE, 1); // assuming DIV2 mode
  //`else
  //  cap_nwl_set_hbm_parameter(HBM_DIV_MODE, 0);
  //`endif

  // Set CLK_2X frequency in firmware
  cap_nwl_set_hbm_parameter(HBM_FREQ, 2000);


  // Run the HBM/PHY power_on tests
  return cap_nwl_hbm_firmware_operation(FW_AERR_TEST);

}

int test_firmware_derr(void) {

  // Reduce the Spico tINIT parameters for faster simulation
  cap_nwl_set_hbm_parameter(HBM_TINIT1_CYCLES, 10);
  cap_nwl_set_hbm_parameter(HBM_TINIT3_CYCLES, 10);
  cap_nwl_set_hbm_parameter(HBM_TINIT5_CYCLES, 10);

  // Set firmware div mode.
  //`ifdef DIV2_MODE
  cap_nwl_set_hbm_parameter(HBM_DIV_MODE, 1); // assuming DIV2 mode
  //`else
  //  cap_nwl_set_hbm_parameter(HBM_DIV_MODE, 0);
  //`endif

  // Set CLK_2X frequency in firmware
  cap_nwl_set_hbm_parameter(HBM_FREQ, 2000);


  // Run the HBM/PHY power_on tests
  return cap_nwl_hbm_firmware_operation(FW_DERR_TEST);
}

int test_firmware_ctc(void) {

//int ctc_address;

  //ctc_address  = CAP_SBUS_M40_CTC00_RCVR_ADDRs;

  // Reduce the Spico tINIT parameters for faster simulation
  cap_nwl_set_hbm_parameter(HBM_TINIT1_CYCLES, 10);
  cap_nwl_set_hbm_parameter(HBM_TINIT3_CYCLES, 10);
  cap_nwl_set_hbm_parameter(HBM_TINIT5_CYCLES, 10);

  // Set the minimum number of run cycles for ctc to speed up simulations
  cap_nwl_set_hbm_parameter(HBM_CTC_RUN_CYCLES, 0);
  
  // Set the CTC pattern 
  // 0 - PRBS (low power)
  // 1 - HIGH_POWER (50% toggle rate)
  // Note that the high power pattern take very long to simulate as it writes
  // the entire dram prior to doing continuos reads
  cap_nwl_set_hbm_parameter(HBM_CTC_PATTERN_TYPE, 0);

  // Reset the HBM and PHY
  cap_nwl_hbm_firmware_operation(FW_RESET);

  // Run the HBM/PHY power_on tests
  cap_nwl_hbm_firmware_operation(FW_INITIALIZE_NWL_MCS);

  // Run a CTC-based PRBS test
  cap_nwl_hbm_firmware_operation(FW_RUN_CTCS);

  return 1;
}
#endif
