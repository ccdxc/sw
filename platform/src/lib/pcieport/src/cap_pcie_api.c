#ifdef CAPRI_SW
#include "cap_sw_glue.h"
#endif
#ifndef CAPRI_SW
#include "cap_pcie_api.h"
#include "cap_sbus_api.h"
#include "sknobs.h"
#include "LogMsg.h"
#include "pknobs_reader.h"
#include "cap_pxb_model_decoders.h"
#endif
#include "cap_pp_c_hdr.h"
#include "cap_sbus_api.h"

#define NUM_PP_SBUS_DEV 52

#ifndef SWPRINT
#define SW_PRINT printf
#endif

#ifndef CAPRI_SW
uint64_t get_knob_val(string str, uint64_t def_val) {
    if(sknobs_exists((char *) str.c_str()) != 0) {
        string value_s = sknobs_get_string((char*) str.c_str(),(char*)"");
        if (isdigit(value_s[0])) {
            return cpp_int(value_s).convert_to<uint64_t>();
        } else {
            return PKnobsReader::evalKnob(str);
        }
    } else {
        return def_val;
    }
    return def_val;
}
#endif

void cap_pcie_reg_write(uint64_t addr, uint32_t wr_data) {
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

uint32_t cap_pcie_reg_read(uint64_t addr) {
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

int cap_pcie_setup_pll_raw(void) {
     
  uint32_t pll_lock;
  uint32_t pll_lock_reg;
  uint32_t count;


  PLOG_MSG("PCIE:SW FUNCTION 1 " << dec << endl);
  PLOG_MSG("PCIE:PLL_LOCK entered " << dec << endl);

  {
     cap_pcie_reg_write((CAP_ADDR_BASE_PP_PP_OFFSET) + CAP_PP_CSR_CFG_PP_PCIE_PLL_RST_N_BYTE_ADDRESS, 0x3);
     pll_lock = 0;
     count = 0;

     while ((pll_lock != 0xf) && (count < 1000)) {
       SLEEP(1000);
       pll_lock_reg = cap_pcie_reg_read((CAP_ADDR_BASE_PP_PP_OFFSET) + CAP_PP_CSR_STA_PP_PCIE_PLL_BYTE_ADDRESS);
       pll_lock = (pll_lock_reg & (0xf));   
       count++;
     }
     if (pll_lock == 0xf) {
       PLOG_MSG("PCIE: " << count << ":STA_PLL locked = " <<  hex << pll_lock_reg << dec << endl);
       SW_PRINT("PCIE PLL locked in %d iterations\n", count);
       return 0;
     }
     else {
       PLOG_ERR("PCIE: " << "PCIE STA_PLL not locked: status = " <<  pll_lock << endl);
       SW_PRINT("ERROR:****PCIE PLL not locked 0x%x\n", pll_lock);
       return 1;
     }
  }
  return 1;
}

int pcie_check_sd_core_status (int chip_id, int inst_id) { 
#ifndef CAPRI_SW
    cap_pp_csr_t & pp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pp_csr_t, chip_id, inst_id);
    cpp_int_helper hlp;
#endif
    int rd_data;

    int count= 0;
    bool done = false;
    PLOG_MSG("PCIE:SW FUNCTION 2 " << dec << endl);
    while(!done) {
               SLEEP(1000);
#ifndef CAPRI_SW
               pp_csr.sta_pp_sd_core_status.read();
               if((count % 10) == 0) {
                   pp_csr.sta_pp_sd_core_status.show();
               }
#endif

               done = true;
               //cpp_int all_val = pp_csr.sta_pp_sd_core_status.all();
               for(unsigned l_count = 0; l_count < 16; l_count++) {
  	           rd_data = cap_pcie_reg_read((CAP_ADDR_BASE_PP_PP_OFFSET) + 
			(CAP_PP_CSR_STA_PP_SD_CORE_STATUS_STA_PP_SD_CORE_STATUS_0_8_BYTE_ADDRESS + 
			(l_count/2) * (CAP_PP_CSR_STA_PP_SD_CORE_STATUS_STA_PP_SD_CORE_STATUS_1_8_BYTE_ADDRESS - CAP_PP_CSR_STA_PP_SD_CORE_STATUS_STA_PP_SD_CORE_STATUS_0_8_BYTE_ADDRESS)));
		   if(((rd_data >> (5+((l_count %2)*16))) & 0x1) == 0) {
                   //if(hlp.get_slc( all_val , (l_count*16)+5 , (l_count*16)+5 ).convert_to<unsigned>() == 0) {
                       done = false; break;
                   }
               }

               count++;
               if(count > 1000) { /* 1s timeout */
                   PLOG_ERR("sta_pp_sd_core_status is not set" << endl);
                   SW_PRINT("ERROR:sta_pp_sd_core_status is not set 0x%x\n", rd_data);
#ifndef CAPRI_SW
                   pp_csr.sta_pp_sd_core_status.show() ; 
#endif
                   done = 1;
		   return 1;
               }
    }
    SW_PRINT("INFO:sta_pp_sd_core_status is set after %d iterations\n", count);
    return 0;
}

void 
pcie_enable_interrupts(void)
{
   PLOG_MSG("PCIE:SW FUNCTION 3 " << dec << endl);
   cap_pcie_reg_write((CAP_ADDR_BASE_PP_PP_OFFSET + (CAP_PP_CSR_CFG_PP_PCS_INTERRUPT_DISABLE_BYTE_ADDRESS)), 0x0);
}

int pcie_core_interrupts_sw (int chip_id, int inst_id, int int_code, int int_data, int l_port, int sd_rom_auto_download, int sbus_real_firmware_download, int phy_port, int lanes, int logical_ports) {
   
    //cap_pp_csr_t & pp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pp_csr_t, chip_id, inst_id);

    int pp_pcsd_intr_beg  = phy_port * 2;
    int pp_pcsd_intr_end  = (phy_port * 2) + lanes;
    int pp_pcsd_intr_lane = ((1 << lanes) -1) << (phy_port * 2);
    int rd_data;


   PLOG_MSG("PCIE:SW FUNCTION 4 " << dec << endl);
   // *** issue core_interrupt to serdes ***
    PLOG_MSG("inside serdes interrupts " << endl);

   // set up interrupt code and data per lane
   for(int j = pp_pcsd_intr_beg; j < pp_pcsd_intr_end; j++) {
      cap_pcie_reg_write((CAP_ADDR_BASE_PP_PP_OFFSET + (CAP_PP_CSR_CFG_PP_PCSD_INTERRUPT_BYTE_ADDRESS + j *4)), (int_data << 16|int_code));
/*
      pp_csr.cfg_pp_pcsd_interrupt[j].code(int_code);
      pp_csr.cfg_pp_pcsd_interrupt[j].data(int_data);
      pp_csr.cfg_pp_pcsd_interrupt[j].write();
*/
   }
  
   // issue interrupt request, with lane mask for all  configured lanes
      cap_pcie_reg_write(CAP_ADDR_BASE_PP_PP_OFFSET + (CAP_PP_CSR_CFG_PP_PCSD_INTERRUPT_REQUEST_BYTE_ADDRESS), pp_pcsd_intr_lane);
	/*
      pp_csr.cfg_pp_pcsd_interrupt_request.lanemask(pp_pcsd_intr_lane);
      pp_csr.cfg_pp_pcsd_interrupt_request.write();
	*/

   // poll for interrupt_in_progress to go high, for all lanes
       bool done=false;
       int count= 0;
       while(!done) {
           if(sbus_real_firmware_download) {
               SLEEP(10000);
           } else if (sd_rom_auto_download == 0) { 
               SLEEP(10);
           } else {
               SLEEP(10000);
           }
/*
           pp_csr.sta_pp_pcsd_interrupt_in_progress.read();
           pp_csr.sta_pp_pcsd_interrupt_in_progress.show();
           done = ((pp_csr.sta_pp_pcsd_interrupt_in_progress.per_lane().convert_to<int>() & pp_pcsd_intr_lane) == pp_pcsd_intr_lane);
*/
           rd_data = cap_pcie_reg_read((CAP_ADDR_BASE_PP_PP_OFFSET) + CAP_PP_CSR_STA_PP_PCSD_INTERRUPT_IN_PROGRESS_BYTE_ADDRESS);
           done = ((rd_data & pp_pcsd_intr_lane) == pp_pcsd_intr_lane);

       count++;
       if(count > 100) { /* 1s timeout */
           PLOG_ERR("sta_pp_pcsd_interrupt_in_progress is not set interupt_value=" << hex << rd_data << dec <<endl);
           /* pp_csr.sta_pp_pcsd_interrupt_in_progress.show() ; */
           done = 1;
           SW_PRINT("sta_pp_pcsd_interrupt_in_progress high timeout rd_data 0x%x\n", rd_data);
           return 1;
        }
      }

   // deassert the interrupt request 
      cap_pcie_reg_write(CAP_ADDR_BASE_PP_PP_OFFSET + (CAP_PP_CSR_CFG_PP_PCSD_INTERRUPT_REQUEST_BYTE_ADDRESS), 0x0);
	/*
      pp_csr.cfg_pp_pcsd_interrupt_request.lanemask(0x0);
      pp_csr.cfg_pp_pcsd_interrupt_request.write();
	*/


   // poll for interrupt_in_progress to go LOW, for all lanes
       done=false;
       count= 0;
       while(!done) {
           SLEEP(1000);
	/*
           pp_csr.sta_pp_pcsd_interrupt_in_progress.read();
           pp_csr.sta_pp_pcsd_interrupt_in_progress.show();
           done = ((pp_csr.sta_pp_pcsd_interrupt_in_progress.per_lane().convert_to<int>() & pp_pcsd_intr_lane) == 0x0);
	*/
           rd_data = cap_pcie_reg_read((CAP_ADDR_BASE_PP_PP_OFFSET) + CAP_PP_CSR_STA_PP_PCSD_INTERRUPT_IN_PROGRESS_BYTE_ADDRESS);
           done = ((rd_data & pp_pcsd_intr_lane) == 0x0);

       count++;
       if(count > 1000) { /* 1s timeout */
           PLOG_ERR("sta_pp_pcsd_interrupt_in_progress is set interupt_value=" << hex << rd_data << dec <<endl);
           /* pp_csr.sta_pp_pcsd_interrupt_in_progress.show() ; */
           /* PLOG_ERR("sta_pp_pcsd_interrupt_in_progress is not set" << endl);
           pp_csr.sta_pp_pcsd_interrupt_in_progress.show() ; 
	   */
           done = 1;
           SW_PRINT("sta_pp_pcsd_interrupt_in_progress low timeout rd_data 0x%x\n", rd_data);
           return 1;
        }
      }

     // read interrupt response data 
     if ((logical_ports == 1) && (lanes == 16)) { 
      rd_data = cap_pcie_reg_read((CAP_ADDR_BASE_PP_PP_OFFSET) + CAP_PP_CSR_STA_PP_PCSD_INTERRUPT_DATA_OUT_BYTE_ADDRESS);
      /* pp_csr.sta_pp_pcsd_interrupt_data_out.read();
      pp_csr.sta_pp_pcsd_interrupt_data_out.show();
	*/
      SW_PRINT("Interrupt read data = %x\n", rd_data);
      PLOG_MSG("serdes core_interrupt dat = " << hex << rd_data << endl);
     }

     PLOG_MSG("serdes core_interrupt complete " << endl);
     return 0;
}

int cap_pcie_serdes_reset(int chip_id, int inst_id)
{
     return pcie_core_interrupts_sw(chip_id, inst_id, 0xa, 0x1,
                                    0, 0, 0, 0, 16, 1);
}

#ifndef CAPRI_SW
int cap_pcie_upload_firmware(int chip_id, int inst_id, const void *rom_info) {
#else 
int cap_pcie_upload_firmware(int chip_id, int inst_id, void *rom_info) {
#endif
   int bcast_grp = 0xe1;

   PLOG_MSG("PCIE:SW FUNCTION 5 " << dec << endl);
   /* set broad cast group to 0xe1 */
   for (int aa=1; aa<33; aa++) {
      if (aa == 18 || aa == 20 || aa == 22 || aa == 24 || aa == 26 || aa == 28 || aa == 30 || aa == 32) 
         set_sbus_broadcast_grp(0, 0, aa, bcast_grp, 0);
      if (aa == 2 || aa == 4 || aa == 6 || aa == 8 || aa == 10 || aa == 12 || aa == 14 || aa == 16) 
         set_sbus_broadcast_grp(0, 0, aa, bcast_grp, 0);
   }
   upload_sbus_master_firmware(0, 0, bcast_grp, rom_info);

   // upload_sbus_master_firmware(0, 0, bcast_grp, "/home/asic/vendors/brcm/design_kit/latest/serdes/firmware/serdes.0x1066_2000.rom");

   return 0;
}

int cap_pcie_complete_serdes_initialization(int chip_id, int inst_id)
{
  int return_val = 0;
  int phy_port =0;
  int lanes =0;
  int logical_ports =0;
  int sd_rom_auto_download = 0;
  int sbus_real_firmware_download = 0;

  PLOG_MSG("PCIE:SW FUNCTION 6 " << dec << endl);
  // *** deassert PCS RESET_N  ***
  cap_pcie_reg_write((CAP_ADDR_BASE_PP_PP_OFFSET + (CAP_PP_CSR_CFG_PP_PCS_RESET_N_BYTE_ADDRESS)), 0xffff);

  if(pcie_check_sd_core_status(chip_id, inst_id)) {
	return_val = 1;
        return 1;
  }

  for (int l_port=0; l_port<1; l_port++) {
     // Hemant Please check
     phy_port = l_port;
     lanes = 16;
     logical_ports = 1;
   
     SW_PRINT("Reading build ID\n");
     PLOG_MSG("reading build ID" << endl);
     if (pcie_core_interrupts_sw(chip_id, inst_id, 0, 0, l_port, sd_rom_auto_download, sbus_real_firmware_download, phy_port, lanes, logical_ports)) {
         return 1;
     }
   
     // Rev ID 
     SW_PRINT("Reading Rev ID\n");
     PLOG_MSG("reading Rev ID" << endl);
     if (pcie_core_interrupts_sw(chip_id, inst_id, 0x3f, 0, l_port, sd_rom_auto_download, sbus_real_firmware_download, phy_port, lanes, logical_ports)) {
         return 1;
     }
   
     // Int_run_iCal_on_Eq_Eval for Equalization : 
     if (pcie_core_interrupts_sw(chip_id, inst_id, 0x26, 0x5211, l_port, sd_rom_auto_download, sbus_real_firmware_download, phy_port, lanes, logical_ports)) {
         return 1;
     }
   
     // DFE_0, DFE_1 and DFE_2 params 
     if (pcie_core_interrupts_sw(chip_id, inst_id, 0x26, 0x0006, l_port, sd_rom_auto_download, sbus_real_firmware_download, phy_port, lanes, logical_ports)) {
         return 1;
     }
     if (pcie_core_interrupts_sw(chip_id, inst_id, 0x26, 0x010c, l_port, sd_rom_auto_download, sbus_real_firmware_download, phy_port, lanes, logical_ports)) {
         return 1;
     }
     if (pcie_core_interrupts_sw(chip_id, inst_id, 0x26, 0x0200, l_port, sd_rom_auto_download, sbus_real_firmware_download, phy_port, lanes, logical_ports)) {
         return 1;
     }
  }

  pcie_enable_interrupts();

  return return_val;
} 

#ifndef CAPRI_SW
void cap_pcie_serdes_setup_wrapper(int chip_id, int inst_id, int gen1, string rom_info) {
    cap_pcie_serdes_setup(chip_id, inst_id, gen1,  (const void *) rom_info.c_str());
}
#endif

#ifndef CAPRI_SW
int cap_pcie_serdes_setup(int chip_id, int inst_id, int gen1, const void *rom_info) {
#else
int cap_pcie_serdes_setup(int chip_id, int inst_id, int gen1, void *rom_info) {
#endif

   PLOG_MSG("PCIE:SW FUNCTION 7 " << dec << endl);
   if(cap_pcie_setup_pll_raw()) {
     SW_PRINT("pcie pll lock failed\n");
     return -1;
   }

   PLOG_MSG("TEST:SBUS cap_top_pp_sbus_test clk divider =6 " << endl);
   cap_pp_sbus_write(chip_id, 0xfe, 0xa, 0x3);
   cap_pp_sbus_write(chip_id, 0xfe, 0xb, 0x5);
   cap_pp_sbus_write(chip_id, 0xfe, 0xa, 0x83);

   if(gen1) {
     cap_sbus_pp_set_rom_enable(chip_id, inst_id, 0);
   } else {
     cap_sbus_pp_set_rom_enable(chip_id, inst_id, 1);
     if(cap_pcie_upload_firmware(chip_id, inst_id, rom_info) == 0) // will set broadcast group; upload firmware; deassert pcs reset
          SW_PRINT("serdes upload happened\n");
   }
   /* do the HAPS specific thing - BRAD */
   /* - Ensure port0 = 16x for ASIC v/s 4x for HAPS */
   if(cap_pcie_complete_serdes_initialization(chip_id, inst_id)) {// send core interrupts, check revision, check build id, load dfe/ical; enable interrupts
          SW_PRINT("serdes initialization failed\n");
          return -1;
   }
   return 0;
}

#ifndef CAPRI_SW

#ifdef _CSV_INCLUDED_    
extern "C" void pcie_download_serdes_code(const char * id);
#endif    

// soft reset sequence 
void cap_pcie_soft_reset(int chip_id, int inst_id, string hint) {

    if ( (hint.compare("serdes_init") == 0) || (hint.compare("all") == 0)) {
        pcie_serdes_init(chip_id, inst_id);
    }

    if((hint.compare("tcam_rst") == 0)  || (hint.compare("all") == 0)) {
        cap_pcie_disable_backdoor_paths(chip_id, inst_id);
        cap_pcie_tcam_rst(chip_id, inst_id, 1);
        SLEEP(100);
        cap_pcie_tcam_rst(chip_id, inst_id, 0);
    }
}

// soft reset, value = 1 means apply reset, value = 0 release soft reset
void cap_pcie_set_soft_reset(int chip_id, int inst_id, int value) {
}

// init start, no polling yet
void cap_pcie_init_start(int chip_id, int inst_id, int cur_l_port) {
    // TODO : download firmware
    int logical_ports = cap_pcie_port_db::access(chip_id)->get_logical_ports();
#ifdef _CSV_INCLUDED_ 
    int sd_rom_auto_download = cap_pcie_port_db::access(chip_id)->get_port_values(0 , "sd_rom_auto_download");
    int sbus_real_firmware_download = cap_pcie_port_db::access(chip_id)->get_port_values(0 , "sbus_real_firmware_download");
    svScope old_scope = svGetScope();
    //if (sknobs_exists((char *)"stub_build")) {
    //svScope new_scope = svGetScopeFromName("\\LIBVERIF.top_tb");
    //svSetScope(new_scope);
    //} else {
    //}
    svScope new_scope = svGetScopeFromName("top_tb");
    svSetScope(new_scope);

    if(sbus_real_firmware_download && (logical_ports > 2)) {
        PLOG_ERR("sbus_real_firmware_download is only valid for logical_ports < 2, it is " << logical_ports << endl);
    }
    if(sbus_real_firmware_download) {
        // broadcast grp0
        for(int j = 0; j < logical_ports; j++) {
            if( (cur_l_port == j) || (cur_l_port == -1)) {
                cap_pcie_init_sbus(chip_id, inst_id, j);
                pcie_pcs_reset(chip_id, inst_id, j);
            }
        }
    } else if(sd_rom_auto_download) {
        pcie_download_serdes_code("sd_rom_auto_download");
        for(int j = 0; j < logical_ports; j++) {
            if( (cur_l_port == j) || (cur_l_port == -1)) {
                pcie_pcs_reset(chip_id, inst_id, j);
            }
        }
    } else {
        pcie_download_serdes_code("default");
        for(int j = 0; j < logical_ports; j++) {
            if( (cur_l_port == j) || (cur_l_port == -1)) {
                pcie_pcs_reset(chip_id, inst_id, j);
            }
        }
    }
    svSetScope(old_scope);
#endif    

#ifdef _ZEBU_
    for(int j = 0; j < logical_ports; j++) {
        pcie_pcs_reset(chip_id, inst_id, j);
    }
#endif    

#ifdef _DIAG_

    int sd_rom_auto_download = cap_pcie_port_db::access(chip_id)->get_port_values(0 , "sd_rom_auto_download");
    int sbus_real_firmware_download = cap_pcie_port_db::access(chip_id)->get_port_values(0 , "sbus_real_firmware_download");
    if(sbus_real_firmware_download) {
        // broadcast grp0
        for(int j = 0; j < logical_ports; j++) {
            if( (cur_l_port == j) || (cur_l_port == -1)) {
                cap_pcie_init_sbus(chip_id, inst_id, j);
                pcie_pcs_reset(chip_id, inst_id, j);
            }
        }
    } else if(sd_rom_auto_download) {
        //pcie_download_serdes_code("sd_rom_auto_download");
        for(int j = 0; j < logical_ports; j++) {
            if( (cur_l_port == j) || (cur_l_port == -1)) {
                pcie_pcs_reset(chip_id, inst_id, j);
            }
        }
    } else {
        PLOG_ERR("For DIAG, we should not be here" << endl);
        //pcie_download_serdes_code("default");
        //for(int j = 0; j < logical_ports; j++) {
        //    if( (cur_l_port == j) || (cur_l_port == -1)) {
        //        pcie_pcs_reset(chip_id, inst_id, j);
        //    }
        //}
    }
#endif    
}

// poll for init done
void cap_pcie_init_done(int chip_id, int inst_id, string hint, int cur_l_port) {
    int logical_ports, lanes, phy_port ;
    pcie_api_mode_t mode;
    logical_ports = cap_pcie_port_db::access(chip_id)->get_logical_ports();

    if (sknobs_get_value(const_cast<char *>(string("sam_only").c_str()), 0)) {
        cap_pcie_block_level_setup(chip_id, inst_id);
        return;
    }

    if( (hint.compare("sw_rst") == 0) || (hint.compare("all") == 0)) {

        for(int j = 0; j < logical_ports; j++) {
            if( (cur_l_port == j) || (cur_l_port == -1)) {
                lanes = cap_pcie_port_db::access(chip_id)->get_port_values(j,"lanes");
                phy_port = cap_pcie_port_db::access(chip_id)->get_port_values(j,"phy_port");
                switch(lanes) {
                    case 2 : mode = PCIE_API_PORT_x2; break;
                    case 4: mode = PCIE_API_PORT_x4; break;
                    case 8: mode = PCIE_API_PORT_x8; break;
                    case 16: mode = PCIE_API_PORT_x16; break;
                    default: mode = PCIE_API_PORT_x1; break;
                }
                pcie_per_port_phystatus_poll(chip_id, inst_id, phy_port , mode);
            }
        }
        PLOG_MSG("pcie ports: phystatus poll done" << endl);




        for(int j = 0; j < logical_ports; j++) { 
            if( (cur_l_port == j) || (cur_l_port == -1)) {
                pcie_per_port_mac_k_cfg(chip_id, inst_id, cap_pcie_port_db::access(chip_id)->logical_to_phy_convert(j), "sw_rst");
            }
        }
    }


    if( (hint.compare("dly0") == 0) || (hint.compare("all") == 0)) {
        SLEEP(1024);
    }

    if( (hint.compare("ltssm_en") == 0) || (hint.compare("all") == 0)) {
        for(int j = 0; j < logical_ports; j++) { 
            if( (cur_l_port == j) || (cur_l_port == -1)) {
                pcie_per_port_mac_k_cfg(chip_id, inst_id, cap_pcie_port_db::access(chip_id)->logical_to_phy_convert(j), "rest_cfg");
            }
        }
        for(int j = 0; j < logical_ports; j++) {
            if( (cur_l_port == j) || (cur_l_port == -1)) {
                pcie_per_port_mac_cfg(chip_id, inst_id, cap_pcie_port_db::access(chip_id)->logical_to_phy_convert(j), "ltssm_en");
            }
        }
    }

    if( (hint.compare("dly1") == 0) || (hint.compare("all") == 0)) {
        SLEEP(3000);
    }

    if( (hint.compare("portgate_open") == 0) || (hint.compare("all") == 0)) {
        for(int j = 0; j < logical_ports; j++) { 
            if( (cur_l_port == j) || (cur_l_port == -1)) {
                pcie_per_port_mac_cfg(chip_id, inst_id, cap_pcie_port_db::access(chip_id)->logical_to_phy_convert(j), "portgate_open"); 
            }
        }
    }
    if( (hint.compare("dly2") == 0) || (hint.compare("all") == 0)) {
        SLEEP(5000);
    }
    if( (hint.compare("poll_ltssm") == 0) || (hint.compare("all") == 0)) {
        for(int j = 0; j < logical_ports; j++) { 
            if( (cur_l_port == j) || (cur_l_port == -1)) {
                pcie_per_port_mac_cfg(chip_id, inst_id, cap_pcie_port_db::access(chip_id)->logical_to_phy_convert(j), "poll_ltssm"); 
            }
        }
        for(int j = 0; j < logical_ports; j++) { 
            if( (cur_l_port == j) || (cur_l_port == -1)) {
                int device_type = cap_pcie_port_db::access(chip_id)->get_port_values(j, "device_type");
                if(device_type != 2) {
                    pcie_per_port_mac_cfg(chip_id, inst_id, cap_pcie_port_db::access(chip_id)->logical_to_phy_convert(j), "retry_en"); 
                }
            }
        }
        for(int j = 0; j < logical_ports; j++) { 
            if( (cur_l_port == j) || (cur_l_port == -1)) {
                pcie_per_port_mac_cfg(chip_id, inst_id, cap_pcie_port_db::access(chip_id)->logical_to_phy_convert(j), "rest_cfg"); 
            }
        }
        int read_mac_apb = sknobs_get_value(const_cast<char *>(string("read_mac_apb").c_str()), 0);
        if(read_mac_apb) {
            for(int j = 0; j < logical_ports; j++) { 
                if( (cur_l_port == j) || (cur_l_port == -1)) {
                    pcie_per_port_mac_cfg(chip_id, inst_id, cap_pcie_port_db::access(chip_id)->logical_to_phy_convert(j), "read_mac_apb"); 
                }
            }
        }
        for(int j = 0; j < logical_ports; j++) { 
            if( (cur_l_port == j) || (cur_l_port == -1)) {
                int auto_enum = cap_pcie_port_db::access(chip_id)->get_port_values(j,"auto_enum");
                cap_pcie_per_port_retry_en(chip_id, inst_id, cap_pcie_port_db::access(chip_id)->logical_to_phy_convert(j) , auto_enum ? 1 : 0); 
            }
        }


        if( (cur_l_port == (logical_ports-1)) || (cur_l_port == -1)) {
            // test only, dont checkin -- cap_pcie_bist_test(chip_id, inst_id) ; 
            cap_pcie_block_level_setup(chip_id, inst_id);
        }
    }

}


// use sknobs base load cfg 
void cap_pcie_load_from_cfg(int chip_id, int inst_id) {
    //cap_pp_csr_t & pp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pp_csr_t, chip_id, inst_id);
    cap_pxb_csr_t & pxb_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, chip_id, inst_id);

    pxb_csr.load_from_cfg();

    uint32_t pic_cache_enable  = sknobs_get_value((char*)"cap0/top/main/pic_cache_enable", 0);
    if (pic_cache_enable == 1) {
       pxb_csr.filter_addr_hi.data[0].read();
       pxb_csr.filter_addr_hi.data[0].value(0xfffffff);
       pxb_csr.filter_addr_hi.data[0].write();
       pxb_csr.filter_addr_ctl.value[0].read();
       pxb_csr.filter_addr_ctl.value[0].value(0xf);
       pxb_csr.filter_addr_ctl.value[0].write();
    }

    //cap_pcie_program_top_level_setup(chip_id, inst_id);
}

void cap_pcie_print_inval_master_cfg_info(int chip_id, int inst_id) {
    cap_pxb_csr_t & pxb_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, chip_id, inst_id);

    PLOG_INFO("PCIE : inval_master config info : " << endl)
    for(int idx = 0; idx < 8; idx++) {
       pxb_csr.filter_addr_ctl.value[idx].read();
       if( (pxb_csr.filter_addr_ctl.value[idx].value() & 0x8) == 0x8) { //bit3 (vld)
         pxb_csr.filter_addr_hi.data[idx].read();
         pxb_csr.filter_addr_lo.data[idx].read();
         if( (pxb_csr.filter_addr_ctl.value[idx].value() & 0x4) == 0x4) { //bit2 (include/use cache)
            PLOG_INFO("PCIE : cache filter idx : " << idx << " enabled with include addr range" 
               << " filter_addr_lo : 0x" << hex << pxb_csr.filter_addr_lo.data[idx].all()
               << " filter_addr_hi : 0x" << hex << pxb_csr.filter_addr_hi.data[idx].all()
               << " inval_send : " << (pxb_csr.filter_addr_ctl.value[idx].value() & 0x1)
               << " inval_fill : " << ( (pxb_csr.filter_addr_ctl.value[idx].value()>>1) & 0x1)
               << dec << endl)
         } else {
            PLOG_INFO("PCIE : cache filter idx : " << idx << " enabled with exclude addr range" 
               << " filter_addr_lo : 0x" << hex << pxb_csr.filter_addr_lo.data[idx].all()
               << " filter_addr_hi : 0x" << hex << pxb_csr.filter_addr_hi.data[idx].all()
               << dec << endl)
         }
       }
    }
}

// eos check, this functions should call eos_cnt, eos_int, eos_sta 
void cap_pcie_eos(int chip_id, int inst_id) {
}

void cap_pcie_eos_cnt(int chip_id, int inst_id) {
    cap_pcie_print_inval_master_cfg_info(chip_id, inst_id);
}

void cap_pcie_eos_int(int chip_id, int inst_id) {
}

void cap_pcie_eos_sta(int chip_id, int inst_id) {
}

void cap_pcie_init_sbus(int chip_id, int inst_id, int l_port) {

  int last_sbus_address;
  int rd_data;
  int phy_port = cap_pcie_port_db::access(chip_id)->get_port_values(l_port,"phy_port");
  int lanes = cap_pcie_port_db::access(chip_id)->get_port_values(l_port,"lanes");
  int sbus_clk_div6 = sknobs_get_value(const_cast<char *>(string("sbus_clk_div6").c_str()),0);

  int bcast_grp = -1;
  if( (phy_port == 0) && (lanes == 8)) bcast_grp = 0;
  else if( (phy_port == 4) && (lanes == 8)) bcast_grp = 1;

  int dev_idcode[NUM_PP_SBUS_DEV] = {0x2, 0xb,        // pmro_0
                      0x1, 0x9, 0x1, 0x9,        // port7: pcie serdes, PCS
                      0x1, 0x9, 0x1, 0x9,        // port6: pcie serdes, PCS
                      0x1, 0x9, 0x1, 0x9,        // port5: pcie serdes, PCS
                      0x1, 0x9, 0x1, 0x9,        // port4: pcie serdes, PCS
                      0x1, 0x9, 0x1, 0x9,        // port3: pcie serdes, PCS
                      0x1, 0x9, 0x1, 0x9,        // port2: pcie serdes, PCS
                      0x1, 0x9, 0x1, 0x9,        // port1: pcie serdes, PCS
                      0x1, 0x9, 0x1, 0x9,        // port0: pcie serdes, PCS
                      0x1, 0x1, 0x1, 0x1,        // M1_0..3
                      0x1, 0x1, 0x1, 0x1,        // M0_0..3
                      0x1,                       // BX
                      0xb, 0xb, 0xb,             // pmro_1..3
                      0xa, 0xa,                  // pll_pcie_0,1
                      0xa,                       // pll_flash
                      0xa,                       // pll_core
                      0xa,                       // pll_cpu
                      0x11};                     // ts Thermal Sensor


    ////////////////////
    cpu_access_type_e cpu_access_type = cpu::access()->get_access_type();
    cpu::access()->set_access_type(front_door_e);

  // set clk divider
    if (sbus_clk_div6) {
       PLOG_MSG("TEST:SBUS cap_top_pp_sbus_test clk divider =6 " << endl);
       cap_pp_sbus_write(chip_id, 0xfe, 0xa, 0x3);
       cap_pp_sbus_write(chip_id, 0xfe, 0xb, 0x5);
       cap_pp_sbus_write(chip_id, 0xfe, 0xa, 0x83);
    } else { 
       PLOG_MSG("TEST:SBUS cap_top_pp_sbus_test clk divider =1 (div2)" << endl);
       cap_pp_sbus_write(chip_id, 0xfe, 0xa, 0x1);
    } 

#ifndef _PCIE_BLOCK_LEVEL_
    rd_data = cap_pp_sbus_read(chip_id, 0xfe, 0xff);
    if (rd_data == dev_idcode[0]) {
       PLOG_MSG("TEST:SBUS PASS cap_top_pp_sbus_test sbus controller idcode:0x" << hex << rd_data << dec << endl);
    } else {
       PLOG_ERR("TEST:SBUS cap_top_pp_sbus_test sbus controller idcode:0x" << hex << rd_data << " exp:" << dev_idcode[0] << dec << endl);
    }

    last_sbus_address = cap_pp_sbus_read(0, 0xfe, 0x2);
    if (last_sbus_address == NUM_PP_SBUS_DEV) {
      PLOG_MSG("TEST:SBUS PASS cap_top_pp_sbus_test last_sbus_address:" << last_sbus_address << endl);
    } else {
      PLOG_ERR("TEST:SBUS cap_top_pp_sbus_test last_sbus_address:" << last_sbus_address << " exp:" << NUM_PP_SBUS_DEV << endl);
    }
    for (int aa=1; aa<last_sbus_address; aa++) {
       rd_data = cap_pp_sbus_read(chip_id, aa, 0xff);
       if (rd_data == dev_idcode[aa]) {
          PLOG_MSG("TEST:SBUS PASS cap_top_pp_sbus_test sbus controller idcode:0x" << hex << rd_data << dec << endl);
       } else {
          PLOG_ERR("TEST:SBUS cap_top_pp_sbus_test sbus controller idcode:0x" << hex << rd_data << " exp:" << dev_idcode[aa] << dec << endl);
       }
    }

#endif
    if (bcast_grp == 0) {
       for (int aa=1; aa<33; aa++) {
          if (aa == 18 || aa == 20 || aa == 22 || aa == 24 || aa == 26 || aa == 28 || aa == 30 || aa == 32) {
             set_sbus_broadcast_grp(0, 0, aa, 0xe1, 0);
          }
       }
       ////GROUP 0
       PLOG_MSG("TEST:SBUS upload_sbus_master_firmware grp0" << endl);
       string asic_src  = string(std::getenv("ASIC_SRC"));
       upload_sbus_master_firmware(0, 0, 0xe1, (void *) string(asic_src + "/ip/cosim/capri/serdes.0x108C_2347.rom").c_str());
    } else if (bcast_grp == 1) {
       for (int aa=1; aa<33; aa++) {
          if (aa == 2 || aa == 4 || aa == 6 || aa == 8 || aa == 10 || aa == 12 || aa == 14 || aa == 16) {
             //set_sbus_broadcast_grp(0, 0, aa, 0xe2, 0);
             set_sbus_broadcast_grp(0, 0, aa, 0xe2, 1);
          }
       }
       ////GROUP 1
       PLOG_MSG("TEST:SBUS upload_sbus_master_firmware grp1" << endl);
       string asic_src  = string(std::getenv("ASIC_SRC"));
       upload_sbus_master_firmware(0, 0, 0xe2, (void *) string(asic_src + "/ip/cosim/capri/serdes.0x108C_2347.rom").c_str());
    } else {
       for (int aa=1; aa<33; aa++) {
          if ( ((aa == 2 || aa == 4 || aa == 6 || aa == 8 || aa == 10 || aa == 12 || aa == 14 || aa == 16)) ||
               ((aa == 18 || aa == 20 || aa == 22 || aa == 24 || aa == 26 || aa == 28 || aa == 30 || aa == 32)) ) {
             set_sbus_broadcast_grp(0, 0, aa, 0xe1, 0);
          }
       }

       // update to all serdes
       PLOG_MSG("TEST:SBUS upload_sbus_master_firmware" << endl);
       string asic_src  = string(std::getenv("ASIC_SRC"));
       upload_sbus_master_firmware(0, 0, 0xe1, (void *) string(asic_src + "/ip/cosim/capri/serdes.0x108C_2347.rom").c_str());
    }


    //report_firmware_revision(0, 2);
    //report_firmware_build_id(0, 2);
    //for (int ii=2; ii<33; ii+=2) {
       //report_firmware_revision(0, ii);
       //report_firmware_build_id(0, ii);
    //}

    SLEEP(1000);

    // clear interrupts
    for(int aa=1; aa<33;aa++) {
        if( 
                ( ((bcast_grp == 0) || (bcast_grp == -1)) && (aa == 18 || aa == 20 || aa == 22 || aa == 24 || aa == 26 || aa == 28 || aa == 30 || aa == 32))  ||
                ( ((bcast_grp == 1) || (bcast_grp == -1)) && (aa == 2 || aa == 4 || aa == 6 || aa == 8 || aa == 10 || aa == 12 || aa == 14 || aa == 16)) 
          ) {
            cap_pp_sbus_write(0, aa, 0xc, 0xc0000000);
        }
    }

  //restore cpu access type
  cpu::access()->set_access_type(cpu_access_type);
}


/*
void cap_pcie_set_csr_inst_path(int chip_id, int inst_id, string path) {
    cap_pp_csr_t & pp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pp_csr_t, chip_id, inst_id);
    cap_pxb_csr_t & pxb_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, chip_id, inst_id);
    pp_csr.set_csr_inst_path("def", path + "pp.u_pp.csr");
    for(int ii = 0; ii < 8; ii++) {
        pp_csr.port_c[ii].set_csr_inst_path("def", path + "pp.u_pp.u_pcie_port" + to_string(ii) + "_16x.c_csr");
        pp_csr.port_p[ii].set_csr_inst_path("def", path + "pp.u_pp.u_pcie_port" + to_string(ii) + "_16x.p_csr");
    }
    pxb_csr.set_csr_inst_path("def", path + "px.u_pxb.csr");


}
*/
void pcie_serdes_init(int chip_id, int inst_id) {
    cap_pp_csr_t & pp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pp_csr_t, chip_id, inst_id);
    
    int sd_rom_auto_download = cap_pcie_port_db::access(chip_id)->get_port_values(0 , "sd_rom_auto_download");
    int toggle_perst_pads = sknobs_get_value(const_cast<char *>(string("toggle_perst_pads").c_str()),0);

    //cap_pxb_csr_t & pxb_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, chip_id, inst_id);

   // ******** serdes init STEP 0 ***************************************************
   //   Power UP  (core_sbus_reset=1; RESET_N=0; I_ASYNC_RESET_N =0; i_pcs_interrupt _disable=1)
   //   Wait for min 64 sbus clocks, deassert core_sbus_reset and unreset PLL
   // *******************************************************************************

   int use_common_clk = cap_pcie_port_db::access(chip_id)->get_port_values(0, "use_common_clk");
   if(use_common_clk) {
       PLOG_MSG("Using common clk for pclk" << endl);
       pp_csr.cfg_pp_pcie_pll_refclk_sel.all(0xff);
       pp_csr.cfg_pp_pcie_pll_refclk_sel.write();
       pp_csr.cfg_pp_pcie_pll_refclk_source_sel.all(0x3);
       pp_csr.cfg_pp_pcie_pll_refclk_source_sel.write();
   }


   // deassert sbus_reset,  i_rom_enable register is inverted outside (rom_enable ECO) i.e. sbus_master.rom_enable's default=0. Set it to 1 for
   // auto hard rom download. so write 0 to invert it outside   
   if (sd_rom_auto_download == 1 ) {
   	pp_csr.cfg_pp_sbus.all(0);
   	pp_csr.cfg_pp_sbus.write();
   }


   PLOG_MSG("pcie_serdes_init: wait for reset done" << endl);

   int pipe_sim = cap_pcie_port_db::access(chip_id)->get_port_values(0, "pipe_sim");
   //int mac_test_in = cap_pcie_port_db::access(chip_id)->get_port_values(0, "mac_test_in");
   //if(pipe_sim && (cpu::access()->get_access_type() == back_door_e) ) {
   //}
   if(pipe_sim) {
    SLEEP(512);
   } else {
    SLEEP(10000);
   }


   /*
   pp_csr.cfg_pp_pcie_pll_refclk_source_sel.pll_0(0);
   pp_csr.cfg_pp_pcie_pll_refclk_source_sel.pll_1(1);
   pp_csr.cfg_pp_pcie_pll_refclk_source_sel.write();
   pp_csr.cfg_pp_pcie_pll_refclk_sel.p0(0);
   pp_csr.cfg_pp_pcie_pll_refclk_sel.p1(1);
   pp_csr.cfg_pp_pcie_pll_refclk_sel.p2(0);
   pp_csr.cfg_pp_pcie_pll_refclk_sel.p3(1);
   pp_csr.cfg_pp_pcie_pll_refclk_sel.p4(0);
   pp_csr.cfg_pp_pcie_pll_refclk_sel.p5(1);
   pp_csr.cfg_pp_pcie_pll_refclk_sel.p6(0);
   pp_csr.cfg_pp_pcie_pll_refclk_sel.p7(1);
   pp_csr.cfg_pp_pcie_pll_refclk_sel.write();
   SLEEP(512);

   */

   // PLL deassert nreset (not sure if hi-lo-hi is needed)
   pp_csr.cfg_pp_pcie_pll_rst_n.all(0x3);
   pp_csr.cfg_pp_pcie_pll_rst_n.write();
   SLEEP(1000);
   pp_csr.cfg_pp_pcie_pll_rst_n.all(0);
   pp_csr.cfg_pp_pcie_pll_rst_n.write();
   SLEEP(1000);
   pp_csr.cfg_pp_pcie_pll_rst_n.all(0x3);
   pp_csr.cfg_pp_pcie_pll_rst_n.write();
   SLEEP(512);

   // *** check for PLL lock ***
   pp_csr.sta_pp_pcie_pll.read();
   PLOG_MSG("pcie_serdes_init: Read data from sta_pp_pcie_pll : 0x" << pp_csr.sta_pp_pcie_pll.all() << endl)

   if(!pipe_sim) {
   // poll for pll_locks to go high, for both PLLs
       bool done=false;
       int count= 0;
       while(!done) {
           SLEEP(1000);
           pp_csr.sta_pp_pcie_pll.read();
           if(PLOG_CHECK_MSG_LEVEL("pcie_api")) { pp_csr.sta_pp_pcie_pll.show(); }
           done = (((pp_csr.sta_pp_pcie_pll.pcie_0_pll_lock().convert_to<int>()) == 1) &&
                   ((pp_csr.sta_pp_pcie_pll.pcie_0_dll_lock().convert_to<int>()) == 1) &&
                   ((pp_csr.sta_pp_pcie_pll.pcie_1_pll_lock().convert_to<int>()) == 1) && 
                   ((pp_csr.sta_pp_pcie_pll.pcie_1_dll_lock().convert_to<int>()) == 1)); 
       count++;
       if(count > 10000) {
           PLOG_ERR("sta_pp_pcie_pll pll_lock, dll_lock  is not set" << endl);
           pp_csr.sta_pp_pcie_pll.show() ; 
           done = 1;
        }
      }
   }


   PLOG_MSG("pcie_serdes_init: Poll for PLL lock complete sta_pp_pcie_pll : 0x" << pp_csr.sta_pp_pcie_pll.all() << endl)

   // *** test for pcie reset outputs to check pads ****
  
   if (toggle_perst_pads) { 
        PLOG_MSG("pcie_perst_pads_output_toggle_test...." << endl)
        pp_csr.cfg_pp_rc_perstn.all(0x5555);
        pp_csr.cfg_pp_rc_perstn.write();
        SLEEP(512);
        pp_csr.cfg_pp_rc_perstn.all(0xDDDD);
        pp_csr.cfg_pp_rc_perstn.write();
        SLEEP(512);
        pp_csr.cfg_pp_rc_perstn.all(0x5555);
        pp_csr.cfg_pp_rc_perstn.write();
        SLEEP(512);
        pp_csr.cfg_pp_rc_perstn.all(0x7777);
        pp_csr.cfg_pp_rc_perstn.write();
        SLEEP(512);
        pp_csr.cfg_pp_rc_perstn.all(0x5555);
        pp_csr.cfg_pp_rc_perstn.write();
        SLEEP(512);
        pp_csr.cfg_pp_rc_perstn.all(0x0000);
        pp_csr.cfg_pp_rc_perstn.write();
   }    


   // ******** serdes init STEP 1 ***************************************************
   //   Clocks stable   
   //   core_sbus_reset=0; RESET_N=0; I_ASYNC_RESET_N =1; i_pcs_interrupt _disable=1
   // ******** ***********************************************************************


   // *** deassert Serdes ASYNC_RESET_N  ***
   pp_csr.cfg_pp_sd_async_reset_n.all(0xffff);
   pp_csr.cfg_pp_sd_async_reset_n.write();
   
}

void pcie_core_interrupts (int chip_id, int inst_id, int int_code, int int_data, int l_port) { 
   
    int sd_rom_auto_download = cap_pcie_port_db::access(chip_id)->get_port_values(0 , "sd_rom_auto_download");
    int sbus_real_firmware_download = sknobs_get_value(const_cast<char *>(string("sbus_real_firmware_download").c_str()),0);

    int phy_port = cap_pcie_port_db::access(chip_id)->get_port_values(l_port,"phy_port");
    int lanes = cap_pcie_port_db::access(chip_id)->get_port_values(l_port,"lanes");
    int logical_ports = cap_pcie_port_db::access(chip_id)->get_logical_ports();

   // *** issue core_interrupt to serdes ***
    PLOG_MSG("inside serdes interrupts " << endl);
    pcie_core_interrupts_sw (chip_id, inst_id, int_code, int_data, l_port, sd_rom_auto_download, sbus_real_firmware_download, phy_port, lanes, logical_ports);

     PLOG_MSG("serdes core_interrupt complete " << endl);
}

   //// ******** serdes init STEP 2 ***************************************************
   ////  Serdes/Spico firmware download 
   ////   core_sbus_reset=0; RESET_N=0; I_ASYNC_RESET_N =1; i_pcs_interrupt _disable=1
   //// ******** ***********************************************************************

   //   // send a signal to top.sv to initiate serdes download and wait for download to complete
   //   serdes_fw_download_trig = 1;
   //   uvm_config_db#(int)::set( .cntxt( null ), .inst_name( "uvm_test_pcie_base" ), .field_name( "serdes_fw_download_trig" ), .value( serdes_fw_download_trig ) );

   //   `uvm_info("pcie_test_base_configure_pcie", $sformatf("wait for serdes firmware download to complete "), UVM_MEDIUM);
   //   while (serdes_fw_download_done==0) 
   //   begin
   //   uvm_config_db#(int)::get( .cntxt( null ), .inst_name( "uvm_test_pcie_base" ), .field_name( "serdes_fw_download_done" ), .value( serdes_fw_download_done ) );
   //   repeat(10) @(posedge env_h.pcie_prp_agt_h.vif.clk); 
   //   end
   //   `uvm_info("pcie_test_base_configure_pcie", $sformatf("finished serdes firmware download"), UVM_MEDIUM);

   // ******** serdes init STEP 3 ***************************************************
   //  Serdes/Spico firmware download  complete
   //   core_sbus_reset=0; RESET_N=1; I_ASYNC_RESET_N =1; i_pcs_interrupt _disable=0
   // ******** ***********************************************************************


void cap_pcie_toggle_pcs_reset(int chip_id, int inst_id, int l_port) {
    cap_pp_csr_t & pp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pp_csr_t, chip_id, inst_id);
    int phy_port = cap_pcie_port_db::access(chip_id)->get_port_values(l_port,"phy_port");
    int lanes = cap_pcie_port_db::access(chip_id)->get_port_values(l_port,"lanes");
    cpp_int_helper hlp;

   // *** deassert PCS RESET_N  ***
   cpp_int pcs_reset_n_val;
   pp_csr.cfg_pp_pcs_reset_n.read();
   // write 0 first
   pcs_reset_n_val = pp_csr.cfg_pp_pcs_reset_n.all();
   pcs_reset_n_val = hlp.set_slc( pcs_reset_n_val , 0, (2*phy_port), (2*phy_port) + lanes - 1 ) ;
   pp_csr.cfg_pp_pcs_reset_n.all(pcs_reset_n_val);
   pp_csr.cfg_pp_pcs_reset_n.write();

   // write 1 now
   pcs_reset_n_val = pp_csr.cfg_pp_pcs_reset_n.all();
   pcs_reset_n_val = hlp.set_slc( pcs_reset_n_val , 0xffff, (2*phy_port), (2*phy_port) + lanes - 1 ) ;
   pp_csr.cfg_pp_pcs_reset_n.all(pcs_reset_n_val);
   pp_csr.cfg_pp_pcs_reset_n.write();
}

void pcie_pcs_reset (int chip_id, int inst_id, int l_port) { 
    cap_pp_csr_t & pp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pp_csr_t, chip_id, inst_id);
    int phy_port = cap_pcie_port_db::access(chip_id)->get_port_values(l_port,"phy_port");
    int lanes = cap_pcie_port_db::access(chip_id)->get_port_values(l_port,"lanes");
    cpp_int_helper hlp;

   int lanes_encoded;


   if(l_port == 0) { // only call 1 time
	   int logical_ports = cap_pcie_port_db::access(chip_id)->get_logical_ports();
	   cpp_int tmp;
	   pp_csr.cfg_pp_linkwidth.all(0);
	   for(int jj=0; jj < logical_ports; jj++) {
		   int phy_port = cap_pcie_port_db::access(chip_id)->get_port_values(jj,"phy_port");
		   int lanes = cap_pcie_port_db::access(chip_id)->get_port_values(jj,"lanes");
		   switch(lanes) {
			   case 2 : lanes_encoded = 3; break;
			   case 4: lanes_encoded = 2; break;
			   case 8: lanes_encoded = 1; break;
			   case 16: lanes_encoded = 0; break;
			   default: lanes_encoded = 3; break;
		   }
		   tmp = pp_csr.cfg_pp_linkwidth.all();
		   tmp = hlp.set_slc( tmp, lanes_encoded, (phy_port*2), (phy_port*2)+1); 
		   pp_csr.cfg_pp_linkwidth.all(tmp);
	   }
	   pp_csr.cfg_pp_linkwidth.write();
           SLEEP(100);
   }

   // *** deassert PCS RESET_N  ***
   cpp_int pcs_reset_n_val;
   pp_csr.cfg_pp_pcs_reset_n.read();
   pcs_reset_n_val = pp_csr.cfg_pp_pcs_reset_n.all();
   pcs_reset_n_val = hlp.set_slc( pcs_reset_n_val , 0xffff, (2*phy_port), (2*phy_port) + lanes - 1 ) ;


   pp_csr.cfg_pp_pcs_reset_n.all(pcs_reset_n_val);
   pp_csr.cfg_pp_pcs_reset_n.write();

    int pipe_sim = cap_pcie_port_db::access(chip_id)->get_port_values(0, "pipe_sim");
    int poll_sd_core_status = sknobs_get_value(const_cast<char *>(string("pcie_poll_sd_core_status").c_str()), 0);
   if (!pipe_sim) { 
       if(!poll_sd_core_status) {
           SLEEP(1000);
       } else {
           int logical_ports = cap_pcie_port_db::access(chip_id)->get_logical_ports();
           if(l_port == logical_ports-1) {
               pcie_check_sd_core_status(chip_id, inst_id);
           }
       }
       // *** issue core interrupts to Serdes ***
       // build ID 
       SW_PRINT("Reading build ID\n");
       PLOG_MSG("reading build ID" << endl);
       pcie_core_interrupts(chip_id, inst_id, 0, 0, l_port);
       // Rev ID 
       SW_PRINT("Reading Rev ID\n");
       PLOG_MSG("reading Rev ID" << endl);
       pcie_core_interrupts(chip_id, inst_id, 0x3f, 0, l_port);
       // Int_run_iCal_on_Eq_Eval for Equalization : 
       pcie_core_interrupts(chip_id, inst_id, 0x26, 0x5201, l_port);
       // DFE_0, DFE_1 and DFE_2 params 
       pcie_core_interrupts(chip_id, inst_id, 0x26, 0x000E, l_port);
       pcie_core_interrupts(chip_id, inst_id, 0x26, 0x0102, l_port);
       pcie_core_interrupts(chip_id, inst_id, 0x26, 0x0238, l_port);
    
      // Check FW CRC : Caution: interrupt 0x3c takes long time for response in simulations -- ~2ms 
      //pcie_core_interrupts(chip_id, inst_id, 0x3c, 0);
   }

   // *** enable PCS interrupts ***
   /* 
 *  cpp_int pcs_interrupt_disable;
   pp_csr.cfg_pp_pcs_interrupt_disable.read();
   pcs_interrupt_disable = pp_csr.cfg_pp_pcs_interrupt_disable.all();
   pcs_interrupt_disable = hlp.set_slc( pcs_interrupt_disable , 0x00, (2*phy_port), (2*phy_port) + lanes - 1 ); 


   pp_csr.cfg_pp_pcs_interrupt_disable.all(pcs_interrupt_disable);
   pp_csr.cfg_pp_pcs_interrupt_disable.write();
*/
   cap_pcie_reg_write((CAP_ADDR_BASE_PP_PP_OFFSET + (CAP_PP_CSR_CFG_PP_PCS_INTERRUPT_DISABLE_BYTE_ADDRESS)), 0x0);



}

   // ******** serdes init STEP 4 ***************************************************
   // Wait for PHY status to go low on all serdes 
   // ******** ***********************************************************************


void pcie_per_port_phystatus_poll(int chip_id, int inst_id, int port_no, int mode) {
    cap_pp_csr_t & pp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pp_csr_t, chip_id, inst_id);
    //cap_pxb_csr_t & pxb_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, chip_id, inst_id);

    bool done=false;
    int count= 0;
    while(!done) {
        SLEEP(100);
        pp_csr.port_c[port_no].sta_c_port_phystatus.read();
        if(mode == PCIE_API_PORT_x16) {
            done = ((pp_csr.port_c[port_no].sta_c_port_phystatus.per_lane().convert_to<int>() & 0xffff) == 0);
        } else if(mode == PCIE_API_PORT_x8) {
            done = ((pp_csr.port_c[port_no].sta_c_port_phystatus.per_lane().convert_to<int>() & 0xff) == 0);
        } else if(mode == PCIE_API_PORT_x4) {
            done = ((pp_csr.port_c[port_no].sta_c_port_phystatus.per_lane().convert_to<int>() & 0xf) == 0);
        } else if(mode == PCIE_API_PORT_x2) {
            done = ((pp_csr.port_c[port_no].sta_c_port_phystatus.per_lane().convert_to<int>() & 0x3) == 0);
        } else if(mode == PCIE_API_PORT_x1) {
            done = ((pp_csr.port_c[port_no].sta_c_port_phystatus.per_lane().convert_to<int>() & 0x1) == 0);
        } else {
            PLOG_ERR("pcie_per_port_phystatus_poll: error mode" << mode << endl);
            done =1;
        }
       count++;
       if(count > 500000) {
           PLOG_ERR("sta_c_port_phystatus is not set" << endl);
           pp_csr.port_c[port_no].sta_c_port_phystatus.show();
           done = 1;
       }

    }

}
   // FIXME .. when PRP read works, add a polling loop here for phy_Status ready
   //repeat(24000) @(posedge env_h.pcie_prp_agt_h.vif.clk); 

   // ******** MAC/Port init STEP 5 ***************************************************
   // Capri pcie port configuration :
   // unreset port, open barrier, set k_* params 
   // ******** ***********************************************************************

void pcie_per_port_mac_k_cfg(int chip_id, int inst_id, int port_no, string hint) {
    cap_pp_csr_t & pp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pp_csr_t, chip_id, inst_id);
    cap_pxb_csr_t & pxb_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, chip_id, inst_id);
    cpp_int_helper hlp;


    if(!hint.compare("sw_rst")) {
        // *** unreset Pcie Port  ***
        pp_csr.cfg_pp_sw_reset.read();

        cpp_int tmp;

        // toggle reset
        tmp = pp_csr.cfg_pp_sw_reset.all();
        tmp = hlp.set_slc( tmp, 0xffffffff, (port_no*2), (port_no*2)+1); 
        pp_csr.cfg_pp_sw_reset.all(tmp);
        pp_csr.cfg_pp_sw_reset.write();

        tmp = pp_csr.cfg_pp_sw_reset.all();
        tmp = hlp.set_slc( tmp, 0, (port_no*2), (port_no*2)+1); 
        pp_csr.cfg_pp_sw_reset.all(tmp);
        pp_csr.cfg_pp_sw_reset.write();


    }

    if(!hint.compare("rest_cfg")) {
        int l_port = cap_pcie_port_db::access(chip_id)->phy_to_logical_convert(port_no);
        // *** set MAC's static K_* and TEST modes  ***
        int speed = cap_pcie_port_db::access(chip_id)->get_port_values(l_port, "speed");
        int pipe_sim = cap_pcie_port_db::access(chip_id)->get_port_values(l_port, "pipe_sim");
        string apci_test_name = cap_pcie_port_db::access(chip_id)->get_apci_test_name();
        int device_type = cap_pcie_port_db::access(chip_id)->get_port_values(l_port, "device_type");
        int serdes_ext_lpbk = cap_pcie_port_db::access(chip_id)->get_port_values(l_port, "serdes_ext_lpbk");
        int ext_lpbk_rc_host_init = sknobs_get_value(const_cast<char *>(string("ext_lpbk_rc_host_init").c_str()), 0);
        int gen4_x16_reduced_credit_mode = sknobs_get_value(const_cast<char *>(string("gen4_x16_reduced_credit_mode").c_str()), 0);

        int lanes = cap_pcie_port_db::access(chip_id)->get_port_values(l_port, "lanes");
        if( (speed == 4) && (lanes == 16)) gen4_x16_reduced_credit_mode = 1;
        else gen4_x16_reduced_credit_mode = 0;
        //speed = 1; pipe_sim = 0;

#ifndef _COSIM_
        int mac_test_in = cap_pcie_port_db::access(chip_id)->get_port_values(l_port, "mac_test_in");
        if(pipe_sim == 1) {
            // sim_mode, enable warnings and notes
            pp_csr.port_c[port_no].cfg_c_mac_test_in.dw0(0x19);
        } else if(mac_test_in) {
            // configurable per test  
           pp_csr.port_c[port_no].cfg_c_mac_test_in.dw0(mac_test_in);
        } else {
            // serdes mode: extended sim_mode, enable warnings and notes
            pp_csr.port_c[port_no].cfg_c_mac_test_in.dw0(0x180019);
        }
        pp_csr.port_c[port_no].cfg_c_mac_test_in.write();
#endif

        int logical_ports = cap_pcie_port_db::access(chip_id)->get_logical_ports();

        if(1) {
            unsigned word0, word1;
            word0 = 0x400200;
            word1 = 0x200020;
            if(logical_ports == 8) {
                word0 = 0x200080;
                word1 = 0x200020;
            } else if (serdes_ext_lpbk ) {
                word0 = 0x7f0640;
                word1 = 0x400040;
            } else if (ext_lpbk_rc_host_init) {
                word0 = 0x7f07ff;
                word1 = 0x7f007f;
            } else if (gen4_x16_reduced_credit_mode ) {
                word0 = 0x200080;
                word1 = 0x100010;
            }
            int phy_port = cap_pcie_port_db::access(chip_id)->get_port_values(l_port,"phy_port");
            pp_csr.port_c[phy_port].cfg_c_mac_k_rx_cred.word0(word0);
            pp_csr.port_c[phy_port].cfg_c_mac_k_rx_cred.word1(word1);
            pp_csr.port_c[phy_port].cfg_c_mac_k_rx_cred.write();

            if(l_port == 0) {
                unsigned base=0;
                unsigned range=1024/logical_ports;
                pxb_csr.cfg_tgt_rx_credit_bfr.adr_base0(base);
                pxb_csr.cfg_tgt_rx_credit_bfr.adr_limit0(base+range-1);
                pxb_csr.cfg_tgt_rx_credit_bfr.update0(1);
                if(logical_ports >= 8) { base=base+range;  }
                pxb_csr.cfg_tgt_rx_credit_bfr.adr_base1(base);
                pxb_csr.cfg_tgt_rx_credit_bfr.adr_limit1(base+range-1);
                pxb_csr.cfg_tgt_rx_credit_bfr.update1(1);
                if(logical_ports >= 4) { base=base+range;  }
                pxb_csr.cfg_tgt_rx_credit_bfr.adr_base2(base);
                pxb_csr.cfg_tgt_rx_credit_bfr.adr_limit2(base+range-1);
                pxb_csr.cfg_tgt_rx_credit_bfr.update2(1);
                if(logical_ports >= 8) { base=base+range;  }
                pxb_csr.cfg_tgt_rx_credit_bfr.adr_base3(base);
                pxb_csr.cfg_tgt_rx_credit_bfr.adr_limit3(base+range-1);
                pxb_csr.cfg_tgt_rx_credit_bfr.update3(1);
                if(logical_ports >= 2) { base=base+range; }
                pxb_csr.cfg_tgt_rx_credit_bfr.adr_base4(base);
                pxb_csr.cfg_tgt_rx_credit_bfr.adr_limit4(base+range-1);
                pxb_csr.cfg_tgt_rx_credit_bfr.update4(1);
                if(logical_ports >= 8) { base=base+range;  }
                pxb_csr.cfg_tgt_rx_credit_bfr.adr_base5(base);
                pxb_csr.cfg_tgt_rx_credit_bfr.adr_limit5(base+range-1);
                pxb_csr.cfg_tgt_rx_credit_bfr.update5(1);
                if(logical_ports >= 4) { base=base+range; }
                pxb_csr.cfg_tgt_rx_credit_bfr.adr_base6(base);
                pxb_csr.cfg_tgt_rx_credit_bfr.adr_limit6(base+range-1);
                pxb_csr.cfg_tgt_rx_credit_bfr.update6(1);
                if(logical_ports >= 8) { base=base+range;  }
                pxb_csr.cfg_tgt_rx_credit_bfr.adr_base7(base);
                pxb_csr.cfg_tgt_rx_credit_bfr.adr_limit7(base+range-1);
                pxb_csr.cfg_tgt_rx_credit_bfr.update7(1);
                pxb_csr.cfg_tgt_rx_credit_bfr.write();

                // special case for MTP mode, port 4 gets most of the buffer. 
                if (ext_lpbk_rc_host_init) {
                   pxb_csr.cfg_tgt_rx_credit_bfr.adr_base0(0);
                   pxb_csr.cfg_tgt_rx_credit_bfr.adr_limit0(127);
                   pxb_csr.cfg_tgt_rx_credit_bfr.update0(1);
                   pxb_csr.cfg_tgt_rx_credit_bfr.adr_base4(128);
                   pxb_csr.cfg_tgt_rx_credit_bfr.adr_limit4(1023);
                   pxb_csr.cfg_tgt_rx_credit_bfr.update4(1);

                   pxb_csr.cfg_tgt_rx_credit_bfr.update1(0);
                   pxb_csr.cfg_tgt_rx_credit_bfr.update2(0);
                   pxb_csr.cfg_tgt_rx_credit_bfr.update3(0);
                   pxb_csr.cfg_tgt_rx_credit_bfr.update5(0);
                   pxb_csr.cfg_tgt_rx_credit_bfr.update6(0);
                   pxb_csr.cfg_tgt_rx_credit_bfr.update7(0);

                   pxb_csr.cfg_tgt_rx_credit_bfr.write();
                } 

                pxb_csr.cfg_tgt_rx_credit_bfr.update0(0);
                pxb_csr.cfg_tgt_rx_credit_bfr.update1(0);
                pxb_csr.cfg_tgt_rx_credit_bfr.update2(0);
                pxb_csr.cfg_tgt_rx_credit_bfr.update3(0);
                pxb_csr.cfg_tgt_rx_credit_bfr.update4(0);
                pxb_csr.cfg_tgt_rx_credit_bfr.update5(0);
                pxb_csr.cfg_tgt_rx_credit_bfr.update6(0);
                pxb_csr.cfg_tgt_rx_credit_bfr.update7(0);
                pxb_csr.cfg_tgt_rx_credit_bfr.write();

                // TODO: revisit for lab, writing only if no of ports are more than 4 
                if(logical_ports > 4) { 
                    unsigned tgt_port_id_limit = 128/logical_ports;
                    for(int ii=0; ii < pxb_csr.get_depth_cfg_tgt_port(); ii++) {
                        pxb_csr.cfg_tgt_port[ii].id_limit(tgt_port_id_limit);
                        pxb_csr.cfg_tgt_port[ii].write();
                    }

                }

                if(logical_ports != 1) {
                    unsigned rd_id_limit = 64;
                    for(int idx=0; idx < logical_ports; idx++) {
                        if( (cap_pcie_port_db::access(chip_id)->get_port_values(idx, "max_payload") == 0) ||
                                (cap_pcie_port_db::access(chip_id)->get_port_values(idx, "max_read_payload") == 0)) {
                            rd_id_limit = 40;
                        }
                    }

                    pxb_csr.cfg_itr_axi_resp_order.rd_id_limit(rd_id_limit);
                    pxb_csr.cfg_itr_axi_resp_order.write();
                    if(rd_id_limit == 40) {
                        cap_pcie_atomic_db::access(chip_id)->set_max_atomic_context(8);
                    } else {
                        cap_pcie_atomic_db::access(chip_id)->set_max_atomic_context(32);
                    }
                }

	       int mtp_eco_disable = sknobs_get_value(const_cast<char *>(string("mtp_eco_disable").c_str()), 0);
              
               if (mtp_eco_disable==1) {
                    pxb_csr.cfg_itr_tx_req.macfifo_thres(0x9);
                    pxb_csr.cfg_itr_tx_req.write();
                    pxb_csr.cfg_pxb_spare2.data(0x1);
                    pxb_csr.cfg_pxb_spare2.write();

               } else { 

                 // Update mac tx_fifo threshold due to ECO for CAP-453, change ITR tlp mux to look at this threshold only at SOP 
                    unsigned macfifo_theshold = 5;
                    for(int idx=0; idx < logical_ports; idx++) {
                        if( (cap_pcie_port_db::access(chip_id)->get_port_values(idx, "max_payload") == 0) ||
                                (cap_pcie_port_db::access(chip_id)->get_port_values(idx, "max_read_payload") == 0)) {
                            macfifo_theshold = 3;
                        }
                    }

                PLOG_MSG("ECO : cfg_itr_tx_req macfifo_thres updated to " << macfifo_theshold << endl);

                pxb_csr.cfg_itr_tx_req.macfifo_thres(macfifo_theshold);
                pxb_csr.cfg_itr_tx_req.write();
             }
            } 

        }

        /*
        if(serdes_ext_lpbk) {
            if(l_port == 0) {
                pxb_csr.cfg_tgt_rx_credit_bfr.adr_base0(0);
                pxb_csr.cfg_tgt_rx_credit_bfr.adr_limit0(511);
                pxb_csr.cfg_tgt_rx_credit_bfr.update0(1);
                pxb_csr.cfg_tgt_rx_credit_bfr.write();
                pxb_csr.cfg_tgt_rx_credit_bfr.update0(0);
                pxb_csr.cfg_tgt_rx_credit_bfr.write();

            } else {
                pxb_csr.cfg_tgt_rx_credit_bfr.adr_base4(512);
                pxb_csr.cfg_tgt_rx_credit_bfr.adr_limit4(1023);
                pxb_csr.cfg_tgt_rx_credit_bfr.update4(1);
                pxb_csr.cfg_tgt_rx_credit_bfr.write();
                pxb_csr.cfg_tgt_rx_credit_bfr.update4(0);
                pxb_csr.cfg_tgt_rx_credit_bfr.write();
            }
            pp_csr.port_c[port_no].cfg_c_mac_k_rx_cred.word0(0x7f0640);
            pp_csr.port_c[port_no].cfg_c_mac_k_rx_cred.word1(0x400040);
            pp_csr.port_c[port_no].cfg_c_mac_k_rx_cred.write();
        }
        */



        pp_csr.port_c[port_no].cfg_c_mac_k_gen.read();
        pp_csr.port_c[port_no].cfg_c_mac_k_gen.gen1_supported(1);
        pp_csr.port_c[port_no].cfg_c_mac_k_gen.gen2_supported(speed >= 2);
        pp_csr.port_c[port_no].cfg_c_mac_k_gen.gen3_supported(speed >= 3);
        pp_csr.port_c[port_no].cfg_c_mac_k_gen.gen4_supported(speed >= 4);
        pp_csr.port_c[port_no].cfg_c_mac_k_gen.pf_intr_tx_mode(0);
        pp_csr.port_c[port_no].cfg_c_mac_k_gen.rx_wdog_en(0);
	int eq_phase23_dis = sknobs_get_value(const_cast<char *>(string("eq_phase23_dis").c_str()), 0);
        if(pipe_sim || eq_phase23_dis)  pp_csr.port_c[port_no].cfg_c_mac_k_gen.eq_ph23_en(0);
        if(device_type == 2) { // RC
            pp_csr.port_c[port_no].cfg_c_mac_k_gen.port_type(4);
        }



#ifdef _COSIM_
        pp_csr.port_c[port_no].cfg_c_mac_k_gen.sris_mode(0);
#endif        
        switch(lanes) {
            case 2 : 
                pp_csr.port_c[port_no].cfg_c_mac_k_gen.disable_lanes(0xe);
                break;
            case 4: 
                pp_csr.port_c[port_no].cfg_c_mac_k_gen.disable_lanes(0xc);
                break;
            case 8:
                pp_csr.port_c[port_no].cfg_c_mac_k_gen.disable_lanes(0x8);
                break;
            case 16: 
                pp_csr.port_c[port_no].cfg_c_mac_k_gen.disable_lanes(0x0);
                break;
            default: 
                pp_csr.port_c[port_no].cfg_c_mac_k_gen.disable_lanes(0xe);
                break;
        }

        pp_csr.port_c[port_no].cfg_c_mac_k_gen.write();   // FIXME_TEMP.. set to gen1 for now
        if(PLOG_CHECK_MSG_LEVEL("pcie_api")) { pp_csr.port_c[port_no].cfg_c_mac_k_gen.show(); }

        if( 0 
            ||(apci_test_name.compare("apcit_config_1_40") == 0) 
          ) {        // Turn off atomic support , ARI forwarding and completion timeout disable support 
            pp_csr.port_c[port_no].cfg_c_mac_k_pexconf.read();
            cpp_int all_bits = pp_csr.port_c[port_no].cfg_c_mac_k_pexconf.all();
            all_bits = hlp.set_slc( all_bits , 0 ,36 , 41); 
            pp_csr.port_c[port_no].cfg_c_mac_k_pexconf.all(all_bits);
            pp_csr.port_c[port_no].cfg_c_mac_k_pexconf.write();
            


        }

        if( 0 
            ||(apci_test_name.compare("apcit_config_1_5") == 0) 
          ) {        // Turn off ASPM L0s/L1 support for SRIS mode 
            pp_csr.port_c[port_no].cfg_c_mac_k_pexconf.read();
            cpp_int all_bits = pp_csr.port_c[port_no].cfg_c_mac_k_pexconf.all();
            all_bits = hlp.set_slc( all_bits , 0 ,74 , 75); 
            pp_csr.port_c[port_no].cfg_c_mac_k_pexconf.all(all_bits);
            pp_csr.port_c[port_no].cfg_c_mac_k_pexconf.write();
        }

        if( 0 
            ||(apci_test_name.compare("apcit_aspm_L0s") == 0) 
            ||(apci_test_name.compare("apcit_aspm_L0s_fts_exsynch_timeout") == 0) 
            ||(apci_test_name.compare("apcit_aspm_L0s_fts_timeout") == 0) 
            ||(apci_test_name.compare("apcit_aspm_L0s_nfts_count") == 0) 
            ||(apci_test_name.compare("apcit_aspm_L0s_ufc") == 0) 
            ||(apci_test_name.compare("apcit_aspm_L1") == 0) 
            ||(apci_test_name.compare("apcit_aspm_L1_linkwidth_change") == 0) 
            ||(apci_test_name.compare("apcit_aspm_L1_speed_change") == 0) 
          ) {        // Turn ON ASPM L0s/L1 support 
            pp_csr.port_c[port_no].cfg_c_mac_k_pexconf.read();
            cpp_int all_bits = pp_csr.port_c[port_no].cfg_c_mac_k_pexconf.all();
            all_bits = hlp.set_slc( all_bits , 3 ,74 , 75); 
            pp_csr.port_c[port_no].cfg_c_mac_k_pexconf.all(all_bits);
            pp_csr.port_c[port_no].cfg_c_mac_k_pexconf.write();
            // Set tl_brsw bits to enable L1, L0s entry  
            pp_csr.port_c[port_no].cfg_c_brsw.tl_brsw_in(0x42);
            pp_csr.port_c[port_no].cfg_c_brsw.write();
        }

        if( 0 
            ||(apci_test_name.compare("apcit_atomic_unsupported_length") == 0) 
          ) {        // Turn off atomic support bits and run this test 
            pp_csr.port_c[port_no].cfg_c_mac_k_pexconf.read();
            cpp_int all_bits = pp_csr.port_c[port_no].cfg_c_mac_k_pexconf.all();
            all_bits = hlp.set_slc( all_bits , 0 ,40 , 41); 
            pp_csr.port_c[port_no].cfg_c_mac_k_pexconf.all(all_bits);
            pp_csr.port_c[port_no].cfg_c_mac_k_pexconf.write();
        }

        if( 0 
            ||(apci_test_name.compare("apcit_pci_pm_L1") == 0) 
          ) {        // Set tl_brsw bits to enable L1, L0s entry  
            pp_csr.port_c[port_no].cfg_c_brsw.tl_brsw_in(0x42);
            pp_csr.port_c[port_no].cfg_c_brsw.write();
        }

        if( 0 
            ||(apci_test_name.compare("apcit_msg_vendor_define_type1") == 0) 
            ||(apci_test_name.compare("apcit_msg_undefined_msg_code") == 0) 
            ||(apci_test_name.compare("apcit_dllp_invalid_type") == 0) 
            ||(apci_test_name.compare("apcit_link_corrupt_lcrc") == 0) 
            ||(apci_test_name.compare("apcit_link_dllp_crc") == 0) 
          ) {        // Turn off sharing of aerr of hw mac port 
            pp_csr.port_c[port_no].cfg_c_port_mac.read();
            pp_csr.port_c[port_no].cfg_c_port_mac.aer_common_en(0);
            pp_csr.port_c[port_no].cfg_c_port_mac.write();
        }

    }
}


void cap_pcie_retry_en(int chip_id, int inst_id, int val) {
    int logical_ports = cap_pcie_port_db::access(chip_id)->get_logical_ports();
    for(int j = 0; j < logical_ports; j++) {
        cap_pcie_per_port_retry_en(chip_id, inst_id, cap_pcie_port_db::access(chip_id)->logical_to_phy_convert(j), val); 
    }
}

void cap_pcie_per_port_retry_en(int chip_id, int inst_id, int port_no, int val) {
    cap_pp_csr_t & pp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pp_csr_t, chip_id, inst_id);
    //cap_pxb_csr_t & pxb_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, chip_id, inst_id);
    pp_csr.port_c[port_no].cfg_c_port_mac.read();
    pp_csr.port_c[port_no].cfg_c_port_mac.cfg_retry_en(val);
    pp_csr.port_c[port_no].cfg_c_port_mac.write();
}

void pcie_per_port_mac_cfg(int chip_id, int inst_id, int port_no, string hint) {
    cap_pp_csr_t & pp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pp_csr_t, chip_id, inst_id);
    cap_pxb_csr_t & pxb_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, chip_id, inst_id);

    int l_port = cap_pcie_port_db::access(chip_id)->phy_to_logical_convert(port_no);
    int clock_sel = cap_pcie_port_db::access(chip_id)->get_port_values(l_port, "clock_sel");
    int device_type = cap_pcie_port_db::access(chip_id)->get_port_values(l_port, "device_type");
    int speed = cap_pcie_port_db::access(chip_id)->get_port_values(l_port, "speed");
    cpp_int_helper hlp;

    if(!hint.compare("ltssm_en")) {



        // *** unreset MAC, LTSSM_enable  ***
        pp_csr.port_c[port_no].cfg_c_port_mac.read();
        pp_csr.port_c[port_no].cfg_c_port_mac.reset(0);
        pp_csr.port_c[port_no].cfg_c_port_mac.write();
        SLEEP(256);


        if( (device_type == 1) || (device_type == 2)) {
            pp_csr.port_c[port_no].dhs_c_mac_apb.entry[(0xB0/4)].read();
            cpp_int read_val;
            read_val = pp_csr.port_c[port_no].dhs_c_mac_apb.entry[(0xB0/4)].all();
            read_val = hlp.set_slc( read_val , speed , 0, 3);
            pp_csr.port_c[port_no].dhs_c_mac_apb.entry[(0xB0/4)].all( read_val  );
            pp_csr.port_c[port_no].dhs_c_mac_apb.entry[(0xB0/4)].write();
        }


        pp_csr.port_c[port_no].cfg_c_port_mac.tx_stream(0);
        pp_csr.port_c[port_no].cfg_c_port_mac.ltssm_en(1);
        pp_csr.port_c[port_no].cfg_c_port_mac.cfg_retry_en(1);
        if(device_type == 2) {
            pp_csr.port_c[port_no].cfg_c_port_mac.cfg_retry_en(0);
            pp_csr.port_c[port_no].cfg_c_port_mac.port_type(0);
        }
        switch(clock_sel) 
        {
            case 0: { pp_csr.port_c[port_no].cfg_c_port_mac.tl_clock_freq(830); break; }
            case 1: { pp_csr.port_c[port_no].cfg_c_port_mac.tl_clock_freq(208); break; }
            case 2: { pp_csr.port_c[port_no].cfg_c_port_mac.tl_clock_freq(50); break; }
            case 3: { pp_csr.port_c[port_no].cfg_c_port_mac.tl_clock_freq(8); break; }
            case 4: { pp_csr.port_c[port_no].cfg_c_port_mac.tl_clock_freq(1000); break; }
            case 5: { pp_csr.port_c[port_no].cfg_c_port_mac.tl_clock_freq(1100); break; }
            default : { pp_csr.port_c[port_no].cfg_c_port_mac.tl_clock_freq(830); break; }
        }
#ifdef _ZEBU_
        pp_csr.port_c[port_no].cfg_c_port_mac.cfg_retry_en(0);
        pp_csr.port_c[port_no].cfg_c_port_mac.tx_stream(1);
        pp_csr.port_c[port_no].cfg_c_port_mac.tl_clock_freq(3320); 
#elif _COSIM_        
        pp_csr.port_c[port_no].cfg_c_port_mac.cfg_retry_en(0);
#endif        

        pp_csr.port_c[port_no].cfg_c_port_mac.write();
    }

    if(!hint.compare("portgate_open")) {
        // *** Open Portgate  ***
        bool done = 0;
        int cfg_count = 0;

        while(!done) {
            pp_csr.port_c[port_no].cfg_c_portgate_open.data(1);
            pp_csr.port_c[port_no].cfg_c_portgate_open.write();


            int count = 0;
            bool inner_done = 0;
            while(!inner_done) {
                SLEEP(100);
                pp_csr.port_c[port_no].sta_c_port_mac.read();
                if(PLOG_CHECK_MSG_LEVEL("pcie_api")) { pp_csr.port_c[port_no].sta_c_port_mac.show(); }
                inner_done = pp_csr.port_c[port_no].sta_c_port_mac.portgate_open().convert_to<int>() == 1;
                count++;
                if(count > 500) {
                    PLOG_MSG("sta_c_port_mac poll # " << count << "portgate_open is not set" << endl);
                    pp_csr.port_c[port_no].sta_c_port_mac.show();
                    break;
                }
            }

            done = inner_done;


            cfg_count++;
            if(cfg_count > 2) {
                PLOG_MSG("try # " << cfg_count << " writing cfg_c_portgate_open" << endl);
                if(cfg_count > 25) {
                    PLOG_ERR("I give it, portgate_open is not set" << endl);
                    done = 1;
                }
            }
        }
       
    }
   

   // *** Read MAC CFG Space ***
    if(!hint.compare("poll_ltssm")) {
        bool done = 0;

        int count = 0;

	int mac_test_in = cap_pcie_port_db::access(chip_id)->get_port_values(l_port, "mac_test_in");
        int ltssm_compliance_test = cap_pcie_port_db::access(chip_id)->get_port_values(0 , "ltssm_compliance_test");
        if (ltssm_compliance_test != 0) {
                done = 1;
        } 
        while(!done) {
            SLEEP(100);
            pp_csr.port_c[port_no].sta_c_port_mac.read();
            if(PLOG_CHECK_MSG_LEVEL("pcie_api")) { pp_csr.port_c[port_no].sta_c_port_mac.show(); }
            done = pp_csr.port_c[port_no].sta_c_port_mac.dl_up().convert_to<int>() == 1;
            done = (pp_csr.port_c[port_no].sta_c_port_mac.ltssm().convert_to<int>() == 0x10);
            // mac_test_in : bit0 - simulation mode
            if(mac_test_in & 0x1) count++;
            if(count > 5000) {
                PLOG_ERR("sta_c_port_mac dl_up & ltssm is not set" << endl);
                pp_csr.port_c[port_no].sta_c_port_mac.show();
                done = 1;
            }
        }
        pp_csr.port_p[port_no].sat_p_port_cnt_ltssm_state_changed.read();
        if(PLOG_CHECK_MSG_LEVEL("pcie_api")) { pp_csr.port_p[port_no].sat_p_port_cnt_ltssm_state_changed.show(); }
        pp_csr.port_p[port_no].sta_p_port_mac.read();
        if(PLOG_CHECK_MSG_LEVEL("pcie_api")) { pp_csr.port_p[port_no].sta_p_port_mac.show(); }

        if(device_type == 2) {
            pp_csr.port_c[port_no].dhs_c_mac_apb.entry[(0x18/4)].read();
            cpp_int read_val;
            read_val =  pp_csr.port_c[port_no].dhs_c_mac_apb.entry[(0x18/4)].all() ;
            read_val = hlp.set_slc( read_val , 0x1 , 16, 23);
            read_val = hlp.set_slc( read_val , 0x1 , 8, 15);
            pp_csr.port_c[port_no].dhs_c_mac_apb.entry[(0x18/4)].all( read_val); 
            pp_csr.port_c[port_no].dhs_c_mac_apb.entry[(0x18/4)].write();

            pp_csr.port_c[port_no].dhs_c_mac_apb.entry[(0x4/4)].read();
            pp_csr.port_c[port_no].dhs_c_mac_apb.entry[(0x4/4)].all( 0x7); 
            pp_csr.port_c[port_no].dhs_c_mac_apb.entry[(0x4/4)].write();


            pp_csr.port_c[port_no].dhs_c_mac_apb.entry[(0x88/4)].read();
            read_val =  pp_csr.port_c[port_no].dhs_c_mac_apb.entry[(0x88/4)].all() ;
            read_val = hlp.set_slc( read_val , cap_pcie_port_db::access(chip_id)->get_port_values(l_port, "max_payload") , 5 , 7);
            read_val = hlp.set_slc( read_val , cap_pcie_port_db::access(chip_id)->get_port_values(l_port, "max_read_payload") , 12, 14);
            pp_csr.port_c[port_no].dhs_c_mac_apb.entry[(0x88/4)].all( read_val ); 
            pp_csr.port_c[port_no].dhs_c_mac_apb.entry[(0x88/4)].write();

        }


    }

   if(!hint.compare("read_mac_apb")) { 
      for (int i = 0; i < 64; i++) {
          pp_csr.port_c[port_no].dhs_c_mac_apb.entry[i].read();
          pp_csr.port_c[port_no].dhs_c_mac_apb.entry[i].show();
      }
   }

    if(!hint.compare("retry_en")) {
        // *** MAC CFG_RETRY disable  ***
        pp_csr.port_c[port_no].cfg_c_port_mac.read();
        pp_csr.port_c[port_no].cfg_c_port_mac.cfg_retry_en(1);
        pp_csr.port_c[port_no].cfg_c_port_mac.write();
    }

    if(!hint.compare("rst_rxfifo")) {
        for(int val = 1; val >= 0; val--) {
            if(port_no == 0) pxb_csr.cfg_tgt_rx_credit_bfr.rst_rxfifo0(val);
            if(port_no == 1) pxb_csr.cfg_tgt_rx_credit_bfr.rst_rxfifo1(val);
            if(port_no == 2) pxb_csr.cfg_tgt_rx_credit_bfr.rst_rxfifo2(val);
            if(port_no == 3) pxb_csr.cfg_tgt_rx_credit_bfr.rst_rxfifo3(val);
            if(port_no == 4) pxb_csr.cfg_tgt_rx_credit_bfr.rst_rxfifo4(val);
            if(port_no == 5) pxb_csr.cfg_tgt_rx_credit_bfr.rst_rxfifo5(val);
            if(port_no == 6) pxb_csr.cfg_tgt_rx_credit_bfr.rst_rxfifo6(val);
            if(port_no == 7) pxb_csr.cfg_tgt_rx_credit_bfr.rst_rxfifo7(val);
            pxb_csr.cfg_tgt_rx_credit_bfr.write();
            SLEEP(100);
        }
    }
    /*
    if(!hint.compare("sw_rst_port")) {
        // *** toggle  Pcie Port's Soft Reset  ***
        pp_csr.cfg_pp_sw_reset.read();

        cpp_int tmp = pp_csr.cfg_pp_sw_reset.all();

        for(int val = 1; val >= 0; val--) {
           tmp = hlp.set_slc( tmp, val, (port_no*2)+1, (port_no*2)+1); 
           pp_csr.cfg_pp_sw_reset.all(tmp);
           pp_csr.cfg_pp_sw_reset.write();
           SLEEP(100);
        }
    }
    */
    if(!hint.compare("pcs_rst_port")) {
        // *** toggle  Pcie Port 0 PCS Reset *** only called during cold reset barrier test  ***

        pp_csr.cfg_pp_pcs_reset_n.read();
        cpp_int tmp = pp_csr.cfg_pp_pcs_reset_n.all();

        tmp = hlp.set_slc( tmp, 0x0, 0 , 0); 
        pp_csr.cfg_pp_pcs_reset_n.all(tmp);
        pp_csr.cfg_pp_pcs_reset_n.write();
        SLEEP(100);
        tmp = hlp.set_slc( tmp, 0xf, 0 , 0); 
        pp_csr.cfg_pp_pcs_reset_n.all(tmp);
        pp_csr.cfg_pp_pcs_reset_n.write();
        SLEEP(100);
    }




}

void cap_pcie_tcam_rst(int chip_id, int inst_id, int val) {
    cap_pxb_csr_t & pxb_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, chip_id, inst_id);
    pxb_csr.cfg_tgt_pmt_grst.vld(val);
    pxb_csr.cfg_tgt_pmt_grst.write();
}



void cap_pcie_block_level_setup(int chip_id, int inst_id) {
    cap_pxb_csr_t & pxb_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, chip_id, inst_id);

    for(int ii = 0; ii < 128; ii++) {
        pxb_csr.dhs_itr_portmap.entry[ii].all(0);
        pxb_csr.dhs_itr_portmap.entry[ii].write();
    }


    pxb_csr.dhs_itr_pcihdrt.entry[0].all(0);
    pxb_csr.dhs_itr_pcihdrt.entry[0].valid(1);
    pxb_csr.dhs_itr_pcihdrt.entry[0].bdf(0x0);
    pxb_csr.dhs_itr_pcihdrt.entry[0].write();

    pxb_csr.dhs_itr_pcihdrt.entry[1].all(0);
    pxb_csr.dhs_itr_pcihdrt.entry[1].valid(1);
    pxb_csr.dhs_itr_pcihdrt.entry[1].bdf(0x0);
    pxb_csr.dhs_itr_pcihdrt.entry[1].write();

    //clear topmost pmr entry to avoid X on ecc, the read is speculative when no pmt hit 
    pxb_csr.dhs_tgt_pmr.entry[0x3ff].all(0);
    pxb_csr.dhs_tgt_pmr.entry[0x3ff].write();


    pxb_csr.dhs_tgt_romask.entry[1].data(0xff00ff00);
    pxb_csr.dhs_tgt_romask.entry[1].write();

    pxb_csr.cfg_tgt_req_notify_int.base_addr_35_2(0x0dead);
    pxb_csr.cfg_tgt_req_notify_int.write(); 
    pxb_csr.cfg_tgt_req_indirect_int.base_addr_35_2(0x01111);
    pxb_csr.cfg_tgt_req_indirect_int.write();
    pxb_csr.cfg_pxb_spare0.data(0x500);     // [10] = disable atomic timeout, [9:0] = additional timeout value for read 
    pxb_csr.cfg_pxb_spare0.write();

    int all_lif_valid = sknobs_get_value(const_cast<char *>(string("pcie_all_lif_valid").c_str()), 0);
    int sdf_lif_valid = sknobs_get_value(const_cast<char *>(string("sdf_lif_valid").c_str()), 0);
    int enable_relaxed_ordering = sknobs_get_value(const_cast<char *>(string("pcie_all_rd_relaxed_ordering").c_str()),0);
    if (sdf_lif_valid) {
        // limited entries for fast init of sdf sim 
        for(int j=0; j < 32; j++) {
            pxb_csr.dhs_itr_pcihdrt.entry[j].valid(1);
            pxb_csr.dhs_itr_pcihdrt.entry[j].attr2_1_rd(enable_relaxed_ordering);
            pxb_csr.dhs_itr_pcihdrt.entry[j].write();
        }
            pxb_csr.dhs_itr_pcihdrt.entry[1965].valid(1);
            pxb_csr.dhs_itr_pcihdrt.entry[1965].attr2_1_rd(enable_relaxed_ordering);
            pxb_csr.dhs_itr_pcihdrt.entry[1965].write();

    } else if(all_lif_valid) {
        for(int j=0; j < pxb_csr.dhs_itr_pcihdrt.get_depth_entry(); j++) {
            pxb_csr.dhs_itr_pcihdrt.entry[j].valid(1);
            pxb_csr.dhs_itr_pcihdrt.entry[j].attr2_1_rd(enable_relaxed_ordering);
            pxb_csr.dhs_itr_pcihdrt.entry[j].write();
        }
    }

    uint32_t force_lif_invalid = sknobs_get_value(const_cast<char *>(string("force_lif_invalid").c_str()), 0xffffffff);
    if( (force_lif_invalid > 0) && (force_lif_invalid <  (unsigned) pxb_csr.dhs_itr_pcihdrt.get_depth_entry())) {
        pxb_csr.dhs_itr_pcihdrt.entry[force_lif_invalid].valid(0);
        pxb_csr.dhs_itr_pcihdrt.entry[force_lif_invalid].write();
    }

    int logical_ports = cap_pcie_port_db::access(chip_id)->get_logical_ports();
    for(int j = 0; j < logical_ports; j++) { 
        int device_type =  cap_pcie_port_db::access(chip_id)->get_port_values(j, "device_type");
        int phy_port = cap_pcie_port_db::access(chip_id)->get_port_values(j,"phy_port");
        if(device_type == 2) {
            pxb_csr.cfg_pc_port_type.all( pxb_csr.cfg_pc_port_type.all().convert_to<unsigned>() & ~(1 << phy_port));
        }
    }
    pxb_csr.cfg_pc_port_type.write();

    int ext_lpbk_rc_host_init = sknobs_get_value(const_cast<char *>(string("ext_lpbk_rc_host_init").c_str()), 0);
    if(ext_lpbk_rc_host_init != 0) {
	inject_raw_config_wr(chip_id, inst_id, 4, 0x01000018 ,0x0102ff00);
	inject_raw_config_wr(chip_id, inst_id, 4, 0x01000024 ,0x0100f1ff);
	inject_raw_config_wr(chip_id, inst_id, 4, 0x01000028 ,0x00000000);
	inject_raw_config_wr(chip_id, inst_id, 4, 0x0100002c ,0x0f000000);
	inject_raw_config_wr(chip_id, inst_id, 4, 0x01000004 ,0x07000000);
	inject_raw_config_wr(chip_id, inst_id, 4, 0x01000088 ,0x20110000);   // extended tag_en=1
	//inject_raw_config_wr(chip_id, inst_id, 4, 0x01000088 ,0x20100000);     // extended tag_en=0 
	inject_raw_config_wr(chip_id, inst_id, 4, 0x01000218 ,0xe0010000);
	inject_raw_config_wr(chip_id, inst_id, 4, 0x01000208 ,0xffffffff);
	inject_raw_config_wr(chip_id, inst_id, 4, 0x01000214 ,0xffffffff);
        pxb_csr.dhs_tgt_rc_bdfmap.entry[0].all(0x80401);
        pxb_csr.dhs_tgt_rc_bdfmap.entry[0].write();
        
    }

    //auto old_access_type = cpu::access()->get_access_type();
    //cpu::access()->set_access_type(front_door_e);
    //pxb_csr.cfg_tgt_axi_attr.set_access_secure(1);
    //pxb_csr.cfg_tgt_axi_attr.read();
    //pxb_csr.cfg_tgt_axi_attr.qos(1);
    //pxb_csr.cfg_tgt_axi_attr.awcache(0); 
    //pxb_csr.cfg_tgt_axi_attr.arcache(0xB);
    //pxb_csr.cfg_tgt_axi_attr.write();


    pxb_csr.cfg_pcie_local_memaddr_decode.atomic_page_id(0x3ff);
    pxb_csr.cfg_pcie_local_memaddr_decode.atomic(0xf);
    pxb_csr.cfg_pcie_local_memaddr_decode.rc_cfg(0);
    pxb_csr.cfg_pcie_local_memaddr_decode.write();

    //cpu::access()->set_access_type(old_access_type);

   

} 

void cap_pcie_program_tcam_sram_pair(int chip_id, int inst_id, uint32_t idx, 
        cap_pxb_csr_dhs_tgt_pmr_entry_t _sram_entry,
        cap_pxb_csr_dhs_tgt_pmt_entry_t _tcam_entry) {


    //cap_pp_csr_t & pp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pp_csr_t, chip_id, inst_id);
    cap_pxb_csr_t & pxb_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, chip_id, inst_id);


    // key     mask ==>     x   y
    //  0       0           0   1
    //  0       1           0   0
    //  1       0           1   0
    //  1       1           0   0
    //

    // key    mask  ==>     x   y
    //  0       0   ==>     0   1        
    //  0       1   ==>     1   0   
    //  1       0   ==>     0   0      
    //  1       1   ==>     0   1     
    //pxb_csr.dhs_tgt_pmt.entry[idx].y_data( (~_tcam_entry.x_data() & ~_tcam_entry.y_data() ) | 
    //        (_tcam_entry.x_data() & _tcam_entry.y_data() ) );
    pxb_csr.dhs_tgt_pmt.entry[idx].x_data( _tcam_entry.x_data() & ~_tcam_entry.y_data());
    pxb_csr.dhs_tgt_pmt.entry[idx].y_data( ~_tcam_entry.x_data() & ~_tcam_entry.y_data());
    pxb_csr.dhs_tgt_pmt.entry[idx].valid(_tcam_entry.valid());

    // for invalidating tcam, write tcam first and sram last
    // for valid tcam programming, write sram first and then tcam
    if(_tcam_entry.valid() == 1) {
        pxb_csr.dhs_tgt_pmr.entry[idx].all(_sram_entry.all());
        pxb_csr.dhs_tgt_pmr.entry[idx].write();

        pxb_csr.dhs_tgt_pmt.entry[idx].write();
    } else {
        pxb_csr.dhs_tgt_pmt.entry[idx].write();

        pxb_csr.dhs_tgt_pmr.entry[idx].all(_sram_entry.all());
        pxb_csr.dhs_tgt_pmr.entry[idx].write();
    }

    // read back to make sure write is flushed
    pxb_csr.dhs_tgt_pmr.entry[idx].read();
    
}


void cap_pcie_program_key_type_cfg_tcam_sram_pair(int chip_id, int inst_id, uint32_t idx, 
        cap_pxb_decoders_pmr_key_type_cfg_t _sram,
        cap_pxb_decoders_pmt_tcam_key_type_cfg_entry_t _tcam) {


    if(PLOG_CHECK_MSG_LEVEL("pcie_api")) { 
        PLOG_MSG("programming entry @ " << idx << endl);
        _tcam.show();
        _sram.show();
    }
    cap_pxb_csr_dhs_tgt_pmt_entry_t tcam_entry;
    tcam_entry.all(_tcam.all());

    cap_pxb_csr_dhs_tgt_pmr_entry_t sram_entry;
    sram_entry.all(_sram.all());
    if(PLOG_CHECK_MSG_LEVEL("pcie_api")) { 
        tcam_entry.show();
        sram_entry.show();
    }


    cap_pcie_program_tcam_sram_pair(chip_id, inst_id, idx, sram_entry, tcam_entry); 

}

void cap_pcie_program_key_type_memio_tcam_sram_pair(int chip_id, int inst_id, uint32_t idx, 
        cap_pxb_decoders_pmr_key_type_memio_t _sram,
        cap_pxb_decoders_pmt_tcam_key_type_memio_entry_t _tcam) {

    cap_pxb_csr_dhs_tgt_pmt_entry_t tcam_entry;
    tcam_entry.all(_tcam.all());

    cap_pxb_csr_dhs_tgt_pmr_entry_t sram_entry;
    sram_entry.all(_sram.all());


    cap_pcie_program_tcam_sram_pair(chip_id, inst_id, idx, sram_entry, tcam_entry); 

}


void cap_pcie_program_key_type_rcdma_tcam_sram_pair(int chip_id, int inst_id, uint32_t idx, 
        cap_pxb_decoders_pmr_key_type_memio_t _sram,
        cap_pxb_decoders_pmt_tcam_key_type_rcdma_entry_t _tcam) {

    cap_pxb_csr_dhs_tgt_pmt_entry_t tcam_entry;
    tcam_entry.all(_tcam.all());
    cap_pxb_csr_dhs_tgt_pmr_entry_t sram_entry;
    sram_entry.all(_sram.all());

    cap_pcie_program_tcam_sram_pair(chip_id, inst_id, idx, sram_entry, tcam_entry); 

}



void cap_pcie_program_prt_type_memio_sram(int chip_id, int inst_id, uint32_t idx,
        cap_pxb_decoders_prt_memio_t _sram) {

    //cap_pp_csr_t & pp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pp_csr_t, chip_id, inst_id);
    cap_pxb_csr_t & pxb_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, chip_id, inst_id);
    
    pxb_csr.dhs_tgt_prt.entry[idx].all( _sram.all());
    pxb_csr.dhs_tgt_prt.entry[idx].write();
}
void cap_pcie_program_prt_type_db_sram(int chip_id, int inst_id, uint32_t idx,
        cap_pxb_decoders_prt_db_t _sram) {
    //cap_pp_csr_t & pp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pp_csr_t, chip_id, inst_id);
    cap_pxb_csr_t & pxb_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, chip_id, inst_id);

    pxb_csr.dhs_tgt_prt.entry[idx].all( _sram.all());
    pxb_csr.dhs_tgt_prt.entry[idx].write();

}


#if 0
void cap_pcie_program_top_level_setup(int chip_id, int inst_id) {
    cap_pxb_decoders_pmt_tcam_key_type_memio_entry_t pmt_memio_entry;
    cap_pxb_decoders_pmr_key_type_memio_t pmr_memio_entry;
    cap_pxb_decoders_prt_memio_t prt_memio_entry;


    //cap_pp_csr_t & pp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pp_csr_t, chip_id, inst_id);
    cap_pxb_csr_t & pxb_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, chip_id, inst_id);
    pmr_memio_entry.init();
    pmr_memio_entry.all(0);

    for(int ii = 0; ii < 8; ii++) {
        if(pxb_csr.cfg_tgt_req_notifyint[ii].base_addr_35_2().convert_to<uint64_t>() != 0) {
            pxb_csr.cfg_tgt_req_notifyint[ii].base_addr_35_2( pxb_csr.cfg_tgt_req_notifyint[ii].base_addr_35_2() | (ii << 8));
            pxb_csr.cfg_tgt_req_notifyint[ii].write();
        }

        if(pxb_csr.cfg_tgt_req_indint[ii].base_addr_35_2().convert_to<uint64_t>() != 0) {
            pxb_csr.cfg_tgt_req_indint[ii].base_addr_35_2( pxb_csr.cfg_tgt_req_indint[ii].base_addr_35_2() | (ii << 8));
            pxb_csr.cfg_tgt_req_indint[ii].write();
        }
    }

    for(int ii = 0; ii < pxb_csr.dhs_tgt_prt.get_depth_entry(); ii++) {
        pmr_memio_entry.all(0);
        pmr_memio_entry.set_name("decoded/" + pxb_csr.dhs_tgt_prt.entry[ii].get_hier_path());
        pmr_memio_entry.load_from_cfg(0,1);
        if(pmr_memio_entry.valid()) {
            PLOG_MSG("Found entry @ index: " << ii << endl) ;
            pmr_memio_entry.show();
        }


    }

    PLOG_MSG("End of cfg" << endl);
    pmr_memio_entry.all(0);
    pmr_memio_entry.valid(1);
    pmr_memio_entry.key_type(1);

    // Address
    // VF: 24:14
    // PRT: 13:12
    // Page: 11:0

    // Page Size: 4k
    // PRT Size (v2-1) : 11
    // PRT Count = 4
    // VF Start (v3) : 14
    // VF limit (v4-1) : 11'h7fff 
    //
    //
    // VF:  30:22
    // PRT: 21:20
    // Page: 19:0

    // Page Size: 1M
    // PRT Size (v2-1) : 19
    // PRT Count = 4
    // VF Start (v3) : 22
    // VF limit (v4-1) : 11'h7fff 
    //
    //

    pmr_memio_entry.prt_base(128);
    pmr_memio_entry.prt_size(19);
    pmr_memio_entry.prt_count(4);

    pmr_memio_entry.vf_start(22);
    pmr_memio_entry.vf_endplus1(31);
    pmr_memio_entry.vf_limit(0x7fff);


    pmr_memio_entry.cpl_bdf(0xfc00);
    pmr_memio_entry.page_size(4); // 1MB
    pmr_memio_entry.vfid_start(0);



    prt_memio_entry.all(0);
    prt_memio_entry.valid(1);
    prt_memio_entry.prt_type(0); // CAP_PCIE_BAR_PRT_TYPE_RESOURCE
    //prt_memio_entry.resource_dwaddr(0x80000000 >> 2); 

    prt_memio_entry.resource_dwaddr(0x80000000 >> 2);
    prt_memio_entry.resource_dwsize(0x7fff); // max
    prt_memio_entry.vf_stride(22);


    pmt_memio_entry.valid(1);
    pmt_memio_entry.key.all(0);
    pmt_memio_entry.key.common.valid(1);
    pmt_memio_entry.key.common.key_type(1); // MEM
    pmt_memio_entry.key.common.port_id(0);
    pmt_memio_entry.key.address( 0xaaaaaa0080000000ULL >> 2);

    pmt_memio_entry.mask.all(0);
    pmt_memio_entry.mask.all( ~pmt_memio_entry.mask.all());
    pmt_memio_entry.mask.address( 0x000000007fffffffULL >> 2);
    pmt_memio_entry.mask.common.valid(0);
    pmt_memio_entry.mask.common.key_type(0);
    pmt_memio_entry.mask.common.port_id(0);

    cpp_int tmp;
    cpp_int_helper hlp;
    if(pmr_memio_entry.vf_start().convert_to<uint32_t>() > 0) {
        cpp_int all_ones = 1;
        all_ones <<= 64; all_ones = all_ones - 1;
        tmp = hlp.set_slc( tmp, all_ones , 0, pmr_memio_entry.vf_start().convert_to<uint32_t>() );
    }
    pmt_memio_entry.mask.address(tmp);

    cap_pcie_program_prt_type_memio_sram(chip_id, inst_id, 128, prt_memio_entry);
    cap_pcie_program_key_type_memio_tcam_sram_pair(chip_id, inst_id, 30, pmr_memio_entry, pmt_memio_entry);

}

#endif

unsigned cap_pcie_read_indirect_req(int chip_id, int inst_id, int port, cap_pxb_target_indirect_info_t & info) {
    //cap_pp_csr_t & pp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pp_csr_t, chip_id, inst_id);
    cap_pxb_csr_t & pxb_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, chip_id, inst_id);
    cpp_int_helper hlp;

    pxb_csr.sta_tgt_ind_info[port].read();

    if(pxb_csr.sta_tgt_ind_info[port].pending().convert_to<int>()) {
        unsigned int idx = pxb_csr.sta_tgt_ind_info[port].ramaddr().convert_to<int>();
        pxb_csr.dhs_tgt_aximst0.entry[idx].read();
        pxb_csr.dhs_tgt_aximst1.entry[idx].read();
        pxb_csr.dhs_tgt_aximst2.entry[idx].read();
        pxb_csr.dhs_tgt_aximst3.entry[idx].read();
        pxb_csr.dhs_tgt_aximst4.entry[idx].read();

        cpp_int val;

        for(int ii = 0; ii < 5; ii++) {
            switch(ii) {
                case 0: { val = hlp.set_slc( val, pxb_csr.dhs_tgt_aximst0.entry[idx].data(), (ii*128), (ii*128) + 127); break; }
                case 1: { val = hlp.set_slc( val, pxb_csr.dhs_tgt_aximst1.entry[idx].data(), (ii*128), (ii*128) + 127); break; }
                case 2: { val = hlp.set_slc( val, pxb_csr.dhs_tgt_aximst2.entry[idx].data(), (ii*128), (ii*128) + 127); break; }
                case 3: { val = hlp.set_slc( val, pxb_csr.dhs_tgt_aximst3.entry[idx].data(), (ii*128), (ii*128) + 127); break; }
                case 4: { val = hlp.set_slc( val, pxb_csr.dhs_tgt_aximst4.entry[idx].data(), (ii*128), (ii*128) + 127); break; }
            }
        }

        info.all(val);
        return 1;
    }
    info.all(0);
    return 0;
}


void cap_pcie_process_indirect_req(int chip_id, int inst_id, int port, unsigned axi_id, vector<uint32_t> dw, unsigned cpl_stat) {
    cap_pxb_csr_t & pxb_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, chip_id, inst_id);

    while(dw.size() < 4) { dw.push_back(0); }
    for(int ii = 0; ii < 4; ii++) {
        switch(ii) {
            case 0: {pxb_csr.dhs_tgt_ind_rsp.entry.data0(dw[0]); break; }
            case 1: {pxb_csr.dhs_tgt_ind_rsp.entry.data1(dw[1]); break; }
            case 2: {pxb_csr.dhs_tgt_ind_rsp.entry.data2(dw[2]); break; }
            case 3: {pxb_csr.dhs_tgt_ind_rsp.entry.data3(dw[3]); break; }
        }
    }

    PLOG_MSG("writing to dhs_tgt_ind_rsp with port " << port << " axi_id " << axi_id << endl);
    pxb_csr.dhs_tgt_ind_rsp.entry.port_id(port);
    pxb_csr.dhs_tgt_ind_rsp.entry.axi_id(axi_id);
    pxb_csr.dhs_tgt_ind_rsp.entry.cpl_stat(cpl_stat);
    pxb_csr.dhs_tgt_ind_rsp.entry.fetch_rsp(0);
    pxb_csr.dhs_tgt_ind_rsp.entry.write();
}


map<int,cap_pcie_port_db*> cap_pcie_port_db::port_db_ptr_map;
cap_pcie_port_db::cap_pcie_port_db(int _chip_id) {
    logical_ports = 0;
    apci_test_name = "none";
    chip_id = _chip_id;
    populate_sknobs_data();
    
}

void cap_pcie_set_port_values(int chip_id, int l_port, string name, int data) {
    cap_pcie_port_db::access(chip_id)->set_port_values(l_port, name, data);
}
void cap_pcie_port_db::set_port_values(int l_port, string name, uint32_t data) {
    port_values[l_port][name] = data;
}

uint32_t cap_pcie_port_db::get_port_values(int l_port, string name) {
    for(auto int_map : port_values) {
        if(int_map.first == l_port) {
            for(auto j : int_map.second) {
                if(name.compare(j.first) == 0) {
                    return j.second;
                }
            }
        }
    }
    PLOG_ERR("nothing found for logical port : " << l_port << " name:" << name << endl);
    return 0;
}

int cap_pcie_port_db::logical_to_phy_convert(int l_port) {
    if(port_values.find(l_port) != port_values.end()) {
        return port_values[l_port]["phy_port"];
    } else {
        PLOG_ERR("unknown logical port : " << l_port << " no entry found" << endl);
        return 0;
    }
}


int cap_pcie_port_db::phy_to_logical_convert(int p_port) {
    for(auto i : port_values) {
        for(auto j : i.second) {
            if( (int(j.second) == p_port) && (j.first.compare("phy_port") == 0)) {
                return i.first;
            }
        }
    }
    PLOG_ERR("unkown phy port provided: " << p_port << " no entry found" << endl);
    return 0;
}
void cap_pcie_port_db::populate_sknobs_data() {
    logical_ports = sknobs_get_value(const_cast<char *>(string("chip/"+to_string(chip_id) + "/pcie_port/logical_ports").c_str()), 1);
    int glb_auto_enum = sknobs_get_value(const_cast<char *>(string("auto_enum").c_str()), 0);
    int pipe_sim = sknobs_get_value(const_cast<char *>(string("CAP_PCIE_PIPE_SIM").c_str()),0);
    int use_common_clk = sknobs_get_value(const_cast<char *>(string("use_common_clk").c_str()),1);
    int serdes_ext_lpbk = sknobs_get_value(const_cast<char *>(string("SERDES_EXT_LPBK").c_str()),0);
    int mac_test_in = sknobs_get_value(const_cast<char *>(string("mac_test_in").c_str()),0);
    int sd_rom_auto_download = sknobs_get_value(const_cast<char *>(string("sd_rom_auto_download").c_str()),0);
    int sbus_real_firmware_download = sknobs_get_value(const_cast<char *>(string("sbus_real_firmware_download").c_str()),0);
    int ltssm_compliance_test = sknobs_get_value(const_cast<char *>(string("ltssm_compliance_test").c_str()),0);
    apci_test_name = sknobs_get_string(const_cast<char *>(string("APCI_TEST").c_str()),(char*)"none");
    int clock_sel = sknobs_get_value(const_cast<char *>(string("global_clk_sel").c_str()),0);
    for(int j = 0; j < logical_ports; j++) {

        string prefix = string("chip/"+to_string(chip_id) + "/pcie_port/logical/"+ to_string(j) + "/");

        port_values[j]["lanes"] = get_knob_val( prefix + "lanes", 16); 
        
        port_values[j]["phy_port"] = get_knob_val( prefix + "phy_port", 0);
        port_values[j]["speed"] = get_knob_val( prefix + "speed", 3);
        port_values[j]["phy_poll_val"] = get_knob_val( prefix + "phy_poll_val", 0xffff);
        port_values[j]["device_type"] = get_knob_val( prefix + "device_type", 1);
        port_values[j]["auto_enum"] = get_knob_val( prefix + "auto_enum", glb_auto_enum);
        port_values[j]["ecrc"] = get_knob_val( prefix + "ecrc", 0);
        port_values[j]["max_payload"] = get_knob_val( prefix + "max_payload", 1);
        port_values[j]["max_read_payload"] = get_knob_val( prefix + "max_read_payload", 1);
        port_values[j]["APCI_FC_ph"] = get_knob_val( prefix + "APCI_FC_ph", 64);
        port_values[j]["APCI_FC_pd"] = get_knob_val( prefix + "APCI_FC_pd", 64);
        port_values[j]["APCI_FC_npd"] = get_knob_val( prefix + "APCI_FC_npd", 64);
        port_values[j]["APCI_FC_nph"] = get_knob_val( prefix + "APCI_FC_nph", 64);
        port_values[j]["APCI_FC_cplh"] = get_knob_val( prefix + "APCI_FC_cplh", 0);
        port_values[j]["APCI_FC_cpld"] = get_knob_val( prefix + "APCI_FC_cpld", 0);
        port_values[j]["use_32b_bar"] = get_knob_val( prefix + "use_32b_bar", 0);
        port_values[j]["reverse_lane"] = get_knob_val( prefix + "reverse_lane", 0);
        port_values[j]["extended_tag_en"] = get_knob_val( prefix + "extended_tag_en", 0);
        port_values[j]["pasid_en"] = get_knob_val( prefix + "pasid_en", 0);
        port_values[j]["pipe_sim"] = pipe_sim; 
        port_values[j]["serdes_ext_lpbk"] = serdes_ext_lpbk; 
        port_values[j]["mac_test_in"] = mac_test_in; 
        port_values[j]["sd_rom_auto_download"] = sd_rom_auto_download; 
        port_values[j]["sbus_real_firmware_download"] = sbus_real_firmware_download; 
        port_values[j]["ltssm_compliance_test"] = ltssm_compliance_test; 
        port_values[j]["clock_sel"] = clock_sel; 
        port_values[j]["use_common_clk"] = use_common_clk; 


        PLOG_MSG("port_values for chip: " << chip_id << endl);
        PLOG_MSG("lanes : " << port_values[j]["lanes"] << endl);
        PLOG_MSG("phy_port : " << port_values[j]["phy_port"] << endl);
        PLOG_MSG("speed : " << port_values[j]["speed"] << endl);
        PLOG_MSG("phy_poll_val : " << port_values[j]["phy_poll_val"] << endl);
        PLOG_MSG("device_type : " << port_values[j]["device_type"] << endl);
        PLOG_MSG("ecrc : " << port_values[j]["ecrc"] << endl);
        PLOG_MSG("max_payload : " << port_values[j]["max_payload"] << endl);
        PLOG_MSG("max_read_payload : " << port_values[j]["max_read_payload"] << endl);
        PLOG_MSG("use_32b_bar : " << port_values[j]["use_32b_bar"] << endl);
        PLOG_MSG("reverse_lane : " << port_values[j]["reverse_lane"] << endl);
        PLOG_MSG("extended_tag_en : " << port_values[j]["extended_tag_en"] << endl);
        PLOG_MSG("pasid_en : " << port_values[j]["pasid_en"] << endl);
        PLOG_MSG("APCI_FC_ph : " << port_values[j]["APCI_FC_ph"] << endl);
        PLOG_MSG("APCI_FC_pd : " << port_values[j]["APCI_FC_pd"] << endl);
        PLOG_MSG("APCI_FC_nph : " << port_values[j]["APCI_FC_nph"] << endl);
        PLOG_MSG("APCI_FC_npd : " << port_values[j]["APCI_FC_npd"] << endl);
        PLOG_MSG("APCI_FC_cplh : " << port_values[j]["APCI_FC_cplh"] << endl);
        PLOG_MSG("APCI_FC_cpld : " << port_values[j]["APCI_FC_cpld"] << endl);
        PLOG_MSG("auto_enum : " << port_values[j]["auto_enum"] << endl);
        PLOG_MSG("pipe_sim : " << port_values[j]["pipe_sim"] << endl);
        PLOG_MSG("mac_test_in : " << port_values[j]["mac_test_in"] << endl);
        PLOG_MSG("sd_rom_auto_download : " << port_values[j]["sd_rom_auto_download"] << endl);
        PLOG_MSG("sbus_real_firmware_download : " << port_values[j]["sbus_real_firmware_download"] << endl);
        PLOG_MSG("ltssm_compliance_test : " << port_values[j]["ltssm_compliance_test"] << endl);
    }
}

int cap_pcie_port_db::get_logical_ports() { return logical_ports; }
string cap_pcie_port_db::get_apci_test_name() { return apci_test_name; }


void cap_pcie_disable_backdoor_paths(int chip_id, int inst_id) {
    cap_pp_csr_t & pp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pp_csr_t, chip_id, inst_id);
    cap_pxb_csr_t & pxb_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, chip_id, inst_id);

    for(int i = 0; i < pxb_csr.dhs_tgt_pmt.get_depth_entry(); i++) {
        pxb_csr.dhs_tgt_pmt.entry[i].set_access_no_zero_time(1);
    }

    for(int i = 0; i < pxb_csr.dhs_tgt_notify.get_depth_entry(); i++) {
        pxb_csr.dhs_tgt_notify.entry[i].set_access_no_zero_time(1);
    }
    pxb_csr.dhs_tgt_ind_rsp.entry.set_access_no_zero_time(1);
    pxb_csr.cfg_itr_raw_tlp.set_access_no_zero_time(1);
    pxb_csr.cfg_itr_raw_tlp_cmd.set_access_no_zero_time(1);
    pxb_csr.sta_itr_raw_tlp.set_access_no_zero_time(1);
    pxb_csr.cfg_tgt_rx_credit_bfr.set_access_no_zero_time(1);
    pxb_csr.cfg_tgt_pmt_grst.set_access_no_zero_time(1);

    for(int i = 0; i < 8; i++) {
        pp_csr.port_c[i].cfg_c_portgate_open.set_access_no_zero_time(1);
        pp_csr.port_c[i].cfg_c_portgate_close.set_access_no_zero_time(1);
        pp_csr.port_c[i].cfg_c_ltr_latency.set_access_no_zero_time(1);
        pp_csr.port_c[i].cfg_c_autonomous_linkwidth.set_access_no_zero_time(1);
        pp_csr.port_c[i].cfg_c_mac_test_in.set_access_no_zero_time(1);
        pp_csr.port_c[i].cfg_c_tl_report.set_access_no_zero_time(1);
        pp_csr.port_c[i].cfg_c_brsw.set_access_no_zero_time(1);
        
        for(int j = 0; j < pp_csr.port_c[i].dhs_c_mac_apb.get_depth_entry(); j++) {
            pp_csr.port_c[i].dhs_c_mac_apb.entry[j].set_access_no_zero_time(1);
        }
    }

    if( (cpu::access()->get_access_type() == back_door_e) ) {
        pxb_csr.cfg_itr_ecc_disable.read();
        pxb_csr.cfg_itr_ecc_disable.pcihdrt_dhs(1);
        pxb_csr.cfg_itr_ecc_disable.pcihdrt_cor(1);
        pxb_csr.cfg_itr_ecc_disable.pcihdrt_det(1);
        pxb_csr.cfg_itr_ecc_disable.portmap_dhs(1);
        pxb_csr.cfg_itr_ecc_disable.portmap_det(1);
        pxb_csr.cfg_itr_ecc_disable.portmap_cor(1);
        pxb_csr.cfg_itr_ecc_disable.write();
        pxb_csr.cfg_tgt_ecc_disable.read();
        pxb_csr.cfg_tgt_ecc_disable.pmr_dhs(1);
        pxb_csr.cfg_tgt_ecc_disable.pmr_cor(1);
        pxb_csr.cfg_tgt_ecc_disable.pmr_det(1);
        pxb_csr.cfg_tgt_ecc_disable.prt_dhs(1);
        pxb_csr.cfg_tgt_ecc_disable.prt_cor(1);
        pxb_csr.cfg_tgt_ecc_disable.prt_det(1);
        pxb_csr.cfg_tgt_ecc_disable.romask_dhs(1);
        pxb_csr.cfg_tgt_ecc_disable.romask_cor(1);
        pxb_csr.cfg_tgt_ecc_disable.romask_det(1);
        pxb_csr.cfg_tgt_ecc_disable.write();
    }

}

void cap_pxb_csr_set_hdl_path(int chip_id, int inst_id, string path) {
    cap_pxb_csr_t & pxb = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, chip_id, inst_id);

    pxb.set_csr_inst_path(0, (path + ".csr"), 0);
    pxb.set_csr_inst_path(1, path, 0);

    pxb.dhs_itr_pcihdrt.set_csr_inst_path(1,"u_itr.u_bdft.mem.mem"); 
    pxb.dhs_itr_portmap.set_csr_inst_path(1, "u_itr.u_portmap.mem.mem");
    pxb.dhs_tgt_pmr.set_csr_inst_path(1, "u_tgt.u_pmr.mem.mem");
    pxb.dhs_tgt_prt.set_csr_inst_path(1, "u_tgt.u_prt.mem.mem");
    pxb.dhs_tgt_romask.set_csr_inst_path(1, "u_tgt.u_romask.mem.mem");
    pxb.dhs_tgt_aximst0.set_csr_inst_path(1, "u_tgt.u_aximst0.mem.mem");
    pxb.dhs_tgt_aximst1.set_csr_inst_path(1, "u_tgt.u_aximst1.mem.mem");
    pxb.dhs_tgt_aximst2.set_csr_inst_path(1, "u_tgt.u_aximst2.mem.mem");
    pxb.dhs_tgt_aximst3.set_csr_inst_path(1, "u_tgt.u_aximst3.mem.mem");
    pxb.dhs_tgt_aximst4.set_csr_inst_path(1, "u_tgt.u_aximst4.mem.mem");
    pxb.dhs_tgt_rc_bdfmap.set_csr_inst_path(1, "u_tgt.u_rc_bdf2vfid.tlb_entry.mem");
}

void cap_pp_csr_set_hdl_path(int chip_id, int inst_id, string path) {
    cap_pp_csr_t & pp = CAP_BLK_REG_MODEL_ACCESS(cap_pp_csr_t, chip_id, inst_id);

    pp.set_csr_inst_path(0, (path + ".csr"), 0);
    pp.set_csr_inst_path(1, path, 0);

    for(int i=0; i< pp.get_depth_port_p();i++) {
        switch(i) 
        {
            case 0: { 
                        pp.port_c[i].set_csr_inst_path(1, "u_pcie_port0_16x.c_csr"); 
                        pp.port_p[i].set_csr_inst_path(1, "u_pcie_port0_16x.p_csr"); 
                        break; 
                    }
            case 1: { 
                        pp.port_c[i].set_csr_inst_path(1, "u_pcie_port1_2x.c_csr"); 
                        pp.port_p[i].set_csr_inst_path(1, "u_pcie_port1_2x.p_csr"); 
                        break; 
                    }
            case 2: { 
                        pp.port_c[i].set_csr_inst_path(1, "u_pcie_port2_4x.c_csr"); 
                        pp.port_p[i].set_csr_inst_path(1, "u_pcie_port2_4x.p_csr"); 
                        break; 
                    }
            case 3: { 
                        pp.port_c[i].set_csr_inst_path(1, "u_pcie_port3_2x.c_csr"); 
                        pp.port_p[i].set_csr_inst_path(1, "u_pcie_port3_2x.p_csr"); 
                        break; 
                    }
            case 4: { 
                        pp.port_c[i].set_csr_inst_path(1, "u_pcie_port4_8x.c_csr"); 
                        pp.port_c[i].set_csr_inst_path(1, "u_pcie_port4_8x.p_csr"); 
                        break; 
                    }
            case 5: { 
                        pp.port_c[i].set_csr_inst_path(1, "u_pcie_port5_2x.c_csr"); 
                        pp.port_c[i].set_csr_inst_path(1, "u_pcie_port5_2x.p_csr"); 
                        break; 
                    }
            case 6: { 
                        pp.port_c[i].set_csr_inst_path(1, "u_pcie_port6_4x.c_csr"); 
                        pp.port_c[i].set_csr_inst_path(1, "u_pcie_port6_4x.p_csr"); 
                        break; 
                    }
            case 7: { 
                        pp.port_c[i].set_csr_inst_path(1, "u_pcie_port7_2x.c_csr"); 
                        pp.port_c[i].set_csr_inst_path(1, "u_pcie_port7_2x.p_csr"); 
                        break; 
                    }
        }

    }

}


map<int, cap_pcie_atomic_db*> cap_pcie_atomic_db::atomic_db_ptr_map;
void cap_pcie_atomic_db::set_max_atomic_context(unsigned max_id) {
    atomic_req_array.clear();
    if(max_id > max_atomic_context) {
       max_id = max_atomic_context;
       PLOG_MSG("resizing cap_pcie_atomic_db: max_id to " << max_id << endl);
    }
    for(unsigned i=0; i< max_id; i++) {
        std::shared_ptr<cap_pxb_decoders_itr_atomicop_req_t> ptr (new cap_pxb_decoders_itr_atomicop_req_t);
        ptr->set_name("atomic_req_array["+to_string(i) + "]");
        atomic_req_array.push_back( ptr);
        atomic_state.push_back(ATOMIC_IDLE);
    }
    max_atomic_context = max_id;
}

cap_pcie_atomic_db::cap_pcie_atomic_db(int chip_id) {
    max_atomic_context =  get_knob_val(  "pcie_atomic_db/max_atomic_context", 32);
    for(unsigned i=0; i< max_atomic_context; i++) {
        std::shared_ptr<cap_pxb_decoders_itr_atomicop_req_t> ptr (new cap_pxb_decoders_itr_atomicop_req_t);
        ptr->set_name("atomic_req_array["+to_string(i) + "]");
        atomic_req_array.push_back( ptr);
        atomic_state.push_back(ATOMIC_IDLE);
    }
}

int cap_pcie_atomic_db::generate_atomic_xn(uint64_t host_addr, int atomic_type, unsigned tlp_length, vector<uint32_t> dw) {
    int id=-1;
    for(unsigned i=0; i<max_atomic_context; i++) {
        if(atomic_state[i] == ATOMIC_IDLE) {
            id=i; 
            PLOG_MSG("cap_pcie_atomic_db: found id" << id << " allocating now.." << endl);
            break;
        }
    }

    if(id != -1) {
        if(!( 
            (atomic_type==0xc) || 
            (atomic_type == 0xd) ||
            (atomic_type == 0xe))) {
            PLOG_ERR("valid atomic type are 0xc or 0xd or 0xe, receiced 0x" << hex << atomic_type << endl << dec);
            return -1;
        }

        if( (atomic_type == 0xc) || (atomic_type == 0xd)) { // FetchAdd or swap
            if(tlp_length >= 2) {
                dw.resize(2);
                tlp_length = 2;
            }
            while(tlp_length < 1) {
                dw.push_back(0);
                tlp_length++;
            }
        } else { // cas
            if(tlp_length >= 8) {
                dw.resize(8);
                tlp_length = 8;
            }
            while(tlp_length < 2) {
                dw.push_back(0);
                tlp_length++;
            }
        }
        if(tlp_length != dw.size()) {
            PLOG_ERR("tlp_length " << tlp_length << " doesn't match with dw size " << dw.size() << endl);
            return -1;
        }

        if(tlp_length <= 2) {
            host_addr = host_addr & (0xfffffffffff8ULL);
        } else if(tlp_length <= 4) {
            host_addr = host_addr & (0xfffffffffff0ULL);
        } else {
            host_addr = host_addr & (0xffffffffffe0ULL);
        }
        atomic_req_array[id]->host_addr(host_addr);

        if(!( 
            (tlp_length==0x1) || 
            (tlp_length == 0x2) ||
            (tlp_length == 0x4) ||
            (tlp_length == 0x8))) {
            PLOG_ERR("valid tlp_length are 1,2,4,8  receiced 0x" << hex << tlp_length << endl << dec);
            return -1;
        }

        atomic_req_array[id]->atomic_type(atomic_type);

        atomic_req_array[id]->tlp_length(tlp_length);
        atomic_req_array[id]->operand_data(0);
        cpp_int data=0;
        for(unsigned i=0; i < dw.size(); i++) {
            data = (data << 32) + dw[i];
        }
        atomic_req_array[id]->operand_data(data);
        atomic_req_array[id]->show();
        atomic_state[id] = ATOMIC_ALLOC;
    }

    return id;
}

std::shared_ptr<cap_pxb_decoders_itr_atomicop_req_t> cap_pcie_atomic_db::get_atomic_wr_req(int id) {
    if(atomic_state[id] == ATOMIC_ALLOC) {
        atomic_state[id] = ATOMIC_WR;
        return atomic_req_array[id];
    }
    return nullptr;
}


std::shared_ptr<cap_pxb_decoders_itr_atomicop_req_t> cap_pcie_atomic_db::get_atomic_rd_req(int id) {
    if(atomic_state[id] == ATOMIC_WR) {
        atomic_state[id] = ATOMIC_RD;
        return atomic_req_array[id];
    }
    return nullptr;
}


void cap_pcie_atomic_db::set_atomic_rd_rsp(int id) {
    if(atomic_state[id] == ATOMIC_RD) {
        atomic_state[id] = ATOMIC_IDLE;
    } else {
        PLOG_ERR("atomic_state[" << id << "] is state " << atomic_state[id] << " expected to be ATOMIC_RD" << endl);
    }
}


void cap_pcie_base_test(int chip_id, int inst_id, uint32_t val) {
    cap_pp_csr_t & pp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pp_csr_t, chip_id, inst_id);
    pp_csr.base.all(val);
    pp_csr.base.write();
    PLOG_MSG("pp_csr base: write base:0x" << hex << val << dec << endl);
    pp_csr.base.read();
    PLOG_MSG("pp_csr base: read base:0x" << hex << pp_csr.base.all().convert_to<uint32_t>() << dec << endl);
}

void inject_raw_config_wr(int chip_id, int inst_id, unsigned port_id, uint32_t addr, uint32_t data) {
	vector<uint32_t> hdr;
	vector<uint32_t> payload;
	hdr.push_back(0x44000001);
        hdr.push_back(0xf);
        hdr.push_back(addr);
	payload.push_back(data);
	inject_raw_tlp(chip_id, inst_id, hdr, payload, port_id, 1);
}

void inject_raw_tlp(int chip_id, int inst_id, vector<uint32_t> & hdr_array, vector<uint32_t> & payload_array, unsigned port_id, bool poll_for_finish) {
    cap_pxb_csr_t & pxb = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, chip_id, inst_id);

    vector<uint32_t> raw_array;
    for(auto itr : hdr_array) { raw_array.push_back(itr); }
    for(auto itr : payload_array) { raw_array.push_back(itr); }

    for(unsigned i=0; i < raw_array.size(); i++) {
        switch(i) {
            case 0: {
                        pxb.cfg_itr_raw_tlp.dw0(raw_array[i]);
                        break;
                    }
            case 1: {
                        pxb.cfg_itr_raw_tlp.dw1(raw_array[i]);
                        break;
                    }
            case 2: {
                        pxb.cfg_itr_raw_tlp.dw2(raw_array[i]);
                        break;
                    }
            case 3: {
                        pxb.cfg_itr_raw_tlp.dw3(raw_array[i]);
                        break;
                    }
            case 4: {
                        pxb.cfg_itr_raw_tlp.dw4(raw_array[i]);
                        break;
                    }
            case 5: {
                        pxb.cfg_itr_raw_tlp.dw5(raw_array[i]);
                        break;
                    }
            case 6: {
                        pxb.cfg_itr_raw_tlp.dw6(raw_array[i]);
                        break;
                    }
            case 7: {
                        pxb.cfg_itr_raw_tlp.dw7(raw_array[i]);
                        break;
                    }
            case 8: {
                        pxb.cfg_itr_raw_tlp.dw8(raw_array[i]);
                        break;
                    }
            case 9: {
                        pxb.cfg_itr_raw_tlp.dw9(raw_array[i]);
                        break;
                    }
            case 10: {
                         pxb.cfg_itr_raw_tlp.dw10(raw_array[i]);
                         break;
                     }
            case 11: {
                         pxb.cfg_itr_raw_tlp.dw11(raw_array[i]);
                         break;
                     }

            default: {
                         PLOG_ERR("inject_raw_tlp : max 12 dw supported. got " << hdr_array.size() << " headers & " << payload_array.size() << " data dw" << endl);
                         break;
                     }
        }
    }
    
    pxb.cfg_itr_raw_tlp.write();

    pxb.cfg_itr_raw_tlp_cmd.port_id(port_id);
    pxb.cfg_itr_raw_tlp_cmd.dw_cnt(raw_array.size());
    pxb.cfg_itr_raw_tlp_cmd.cmd_go(1);
    pxb.cfg_itr_raw_tlp_cmd.write();
    if(PLOG_CHECK_MSG_LEVEL("pcie_api")) { 
        pxb.cfg_itr_raw_tlp_cmd.show();
    }


    pxb.cfg_itr_raw_tlp_cmd.cmd_go(0) ; 
    pxb.cfg_itr_raw_tlp_cmd.write();
    if(PLOG_CHECK_MSG_LEVEL("pcie_api")) { 
        pxb.cfg_itr_raw_tlp_cmd.show();
    }
    //auto write_vec = pxb.cfg_itr_raw_tlp.get_write_vec();
    //uint64_t write_addr = pxb.cfg_itr_raw_tlp.get_offset();
    //if(write_vec.size()) { 
    //    cpu::access()->write(pxb.cfg_itr_raw_tlp.get_chip_id(), write_addr + ((write_vec.size() -1)*4), write_vec[write_vec.size()- 1]);
    //} else {
    //    PLOG_ERR("inject_raw_tlp: unexpected size is " << write_vec.size() << endl);
    //}

    if(poll_for_finish) {
        poll_done_raw_tlp(chip_id, inst_id); 
    }
}

bool is_pcie_raw_tlp_ready(int chip_id, int inst_id) {
    cap_pxb_csr_t & pxb = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, chip_id, inst_id);
    pxb.sta_itr_raw_tlp.read() ; 
    return (pxb.sta_itr_raw_tlp.resp_rdy().convert_to<int>() == 1);
}

void poll_done_raw_tlp(int chip_id, int inst_id) {
    cap_pxb_csr_t & pxb = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, chip_id, inst_id);

    unsigned count =0;
    while (is_pcie_raw_tlp_ready(chip_id, inst_id) == 0) {
        SLEEP(1000);
        count++;
        if(count == 500) {
            PLOG_ERR("inject_raw_tlp: max poll_for_finish touched max poll count, exiting" << endl);
            break;
        }
    } 
    pxb.sta_itr_raw_tlp_data.read();
    if(PLOG_CHECK_MSG_LEVEL("pcie_api")) { 
        pxb.sta_itr_raw_tlp.show();
        pxb.sta_itr_raw_tlp_data.show();
    }
}

unsigned pcie_query_port_val(int chip_id, int inst_id, unsigned lif) {
    cap_pxb_csr_t & pxb = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, chip_id, inst_id);
    cpp_int_helper hlp;
    auto & portmap_entry = pxb.dhs_itr_portmap.entry[lif >> 4];
    cpp_int dest = portmap_entry.dest();
    return hlp.get_slc( dest ,  ((lif & 0xf) * 3), ((lif & 0xf)*3) + 3 - 1).convert_to<unsigned>();
}


void pcie_program_tgt_debug_catch_all(int chip_id, int inst_id, int port, int tcam_idx, int prt_base, int flags) {
    cap_pxb_decoders_pmt_tcam_key_type_memio_entry_t pmt_memio_entry;
    cap_pxb_decoders_pmr_key_type_memio_t pmr_memio_entry;
    cap_pxb_decoders_prt_memio_t prt_memio_entry;
    cpp_int_helper hlp;
    // catch all entry
    pmt_memio_entry.valid(1);
    pmt_memio_entry.mask.all(0);
    pmt_memio_entry.mask.all( pmt_memio_entry.mask.all() -1 ); 
    pmt_memio_entry.mask.common.port_id(0);
    pmt_memio_entry.key.common.port_id(port);
    pmt_memio_entry.mask.common.valid(0);
    pmt_memio_entry.key.common.valid(1);
    pmt_memio_entry.mask.common.key_type(0);
    pmt_memio_entry.key.common.key_type(CAP_PCIE_API_KEY_TYPE_MEM);
    if(flags & (1<<1)) {
        cpp_int val = 0; 
        pmt_memio_entry.mask.address(0);
        pmt_memio_entry.mask.address( pmt_memio_entry.mask.address() -1);
        val = pmt_memio_entry.mask.address();
        val = hlp.set_slc(val, 0 , 35 -2 , 35-2);
        pmt_memio_entry.mask.address(val); 
        val = 0;
        val = hlp.set_slc(val, 1 , 35 -2 , 35-2);
        pmt_memio_entry.key.address(val);
    }
    // Address
    // PRT SIZE: 30:0
    // PRT IDX : 34:31
    // BIT 35: NOC Loopback

    pmr_memio_entry.valid(1);
    pmr_memio_entry.prt_base(prt_base);
    pmr_memio_entry.prt_size(31);
    pmr_memio_entry.prt_count(16);

    pmr_memio_entry.vf_start(35);
    pmr_memio_entry.vf_endplus1(2); 
    pmr_memio_entry.vf_limit(0x7fff);


    pmr_memio_entry.cpl_bdf(0x0);
    pmr_memio_entry.vfid_start(0);
    pmr_memio_entry.key_type(CAP_PCIE_API_KEY_TYPE_MEM);
       

    prt_memio_entry.all(0);
    prt_memio_entry.valid(1);
    prt_memio_entry.prt_type(CAP_PCIE_API_RESOURCE_PRT);
    prt_memio_entry.resource_dwaddr(0x0);
    prt_memio_entry.resource_dwsize(0x7fff);
    if(flags & 1) {
        prt_memio_entry.capri_address63(1);
    }
    prt_memio_entry.vf_stride(0);
    prt_memio_entry.pmv_check_disable(1);
    PLOG_MSG("programming PMT & PMR for catch all" << endl);
    if(PLOG_CHECK_MSG_LEVEL("pcie_api")) { 
        pmt_memio_entry.show();
        pmr_memio_entry.show();
    }

    cap_pcie_program_key_type_memio_tcam_sram_pair(chip_id, inst_id, tcam_idx, pmr_memio_entry, pmt_memio_entry);
    for(unsigned i=0; i < pmr_memio_entry.prt_count().convert_to<unsigned>(); i++) {
        if(PLOG_CHECK_MSG_LEVEL("pcie_api")) { 
            prt_memio_entry.show();
        }

      cpp_int tmp_addr;
      tmp_addr = hlp.set_slc(tmp_addr , i , 31-2, 34-2);
      prt_memio_entry.resource_dwaddr(tmp_addr); 
      cap_pcie_program_prt_type_memio_sram(chip_id, inst_id, pmr_memio_entry.prt_base().convert_to<unsigned>() + i, prt_memio_entry);
    }


} 

void cap_pp_set_rom_enable(int chip_id, int inst_id, int val) {

  cap_pp_csr_t & pp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pp_csr_t, chip_id, inst_id);

  pp_csr.cfg_pp_sbus.read();
  pp_csr.cfg_pp_sbus.rom_en(val);
  pp_csr.cfg_pp_sbus.write();
}

void cap_pcie_bist_test (int chip_id, int inst_id ) {

  cap_pp_csr_t & pp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pp_csr_t, chip_id, inst_id);
  cap_pxb_csr_t & pxb_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, chip_id, inst_id);
  int loop_cnt; 
  uint32_t tmp_result ; 

  // Make sure all pcie ports are unreset 
  pp_csr.cfg_pp_sw_reset.all(0); 
  pp_csr.cfg_pp_sw_reset.write(); 
  SLEEP(1024); 


  // Write BIST RUN for sbus_master
  pp_csr.cfg_pp_sbus_master_bist.run(1) ; 
  pp_csr.cfg_pp_sbus_master_bist.write() ; 

  // Write BIST RUN for 8 pcie port rams 
  for(int ii=0; ii < 8 ; ii++) {
      pp_csr.port_p[ii].cfg_p_sram_bist.run(0xff);
      pp_csr.port_p[ii].cfg_p_sram_bist.write();
  }

  // Write PX BIST run   sram+tcam
      pxb_csr.cfg_sram_bist.run(0x3fffffffff);
      pxb_csr.cfg_sram_bist.write();
      pxb_csr.cfg_tcam_bist.run(1);
      pxb_csr.cfg_tcam_bist.write();

  for(int ii=0; ii < 8 ; ii++) {
     tmp_result = 0 ; 
     while (loop_cnt < 10000) {
        SLEEP(1000); 
        pp_csr.port_p[ii].sta_p_sram_bist.read();
        pp_csr.port_p[ii].sta_p_sram_bist.show();
        tmp_result = pp_csr.port_p[ii].sta_p_sram_bist.done_pass().convert_to<uint32_t>() |
                     pp_csr.port_p[ii].sta_p_sram_bist.done_fail().convert_to<uint32_t>() ; 
        if (tmp_result == 0xff) {
           break; 
        } 
        loop_cnt++;
     }
     if (pp_csr.port_p[ii].sta_p_sram_bist.done_fail().convert_to<int>() > 0 ) {
                   PLOG_ERR("pcie bist failed for port: " << ii << endl);
     } else  { 
                   PLOG_MSG("pcie bist PASS for port: " << ii << endl);
     }
  }

   // check PX TCAM bist status
     loop_cnt = 0;
     while (loop_cnt < 1000) {
        SLEEP(1000); 
        pxb_csr.sta_tcam_bist.read();
        pxb_csr.sta_tcam_bist.show();
        if ((pxb_csr.sta_tcam_bist.done_pass().convert_to<int>() == 1) || (pxb_csr.sta_tcam_bist.done_fail().convert_to<int>() == 1))  {
           break; 
        } 
        loop_cnt++;
     }
     if (pxb_csr.sta_tcam_bist.done_fail().convert_to<int>() == 1) {
                   PLOG_ERR("pcie bist failed for TCAM "  << endl);
     } else  { 
                   PLOG_MSG("pcie bist PASS for TCAM "  << endl);
     }
  
   // check PX SRAM bist status
   
    uint64_t tmp_pxb_bist_sta ; 
    uint64_t tmp_pxb_bist_sta_fail ; 

    tmp_pxb_bist_sta = 0  ; 
    tmp_pxb_bist_sta_fail = 0  ; 

     loop_cnt = 0;
     while (loop_cnt < 1000) {
        SLEEP(1000); 
        pxb_csr.sta_sram_bist.read();
        pxb_csr.sta_sram_bist.show();

        tmp_pxb_bist_sta =  pxb_csr.sta_sram_bist.done_pass().convert_to<uint64_t>() |
                            pxb_csr.sta_sram_bist.done_fail().convert_to<uint64_t>() ; 

        tmp_pxb_bist_sta_fail =  pxb_csr.sta_sram_bist.done_fail().convert_to<uint64_t>() ; 

        if (tmp_pxb_bist_sta == 0x3fffffffff) {
           break; 
        } 
        loop_cnt++;
      } 
 
      if (tmp_pxb_bist_sta_fail > 0 ) {
                   PLOG_ERR("pcie bist failed for PXB sram: "  << endl);
      } else  { 
                   PLOG_MSG("pcie bist PASS for PXB sram: "  << endl);
      }
}



int cap_run_pcie_tcam_rdwr_test(int chip_id, int inst_id, int fast_mode, int verbosity) {

	    //cap_pp_csr_t & pp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pp_csr_t, chip_id, inst_id);
	    cap_pxb_csr_t & pxb_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, chip_id, inst_id);

	
	    cpp_int_helper hlp;
            cap_pxb_csr_dhs_tgt_pmt_entry_t::x_data_cpp_int_t x_data;
            cap_pxb_csr_dhs_tgt_pmt_entry_t::y_data_cpp_int_t y_data;
            cpp_int val = 1;
	    cpp_int tmp_x_data;
	    cpp_int inv_x_data;
            cpp_int exp_hit; 
            cpp_int exp_hit_addr; 
            uint32_t n_tcam_addrbits = log2(pxb_csr.dhs_tgt_pmt.get_depth_entry());

            int fail = 0;

            PLOG_MSG("n_tcam_addrbits " << n_tcam_addrbits << endl);

            // Walking ones on TCAM data , first 64 entry addesses 
            for (uint32_t n_iter=0; n_iter < 2; n_iter++ ) { 

                 PLOG_MSG( "inside TCAM test, with valid=" << n_iter << std::endl);

                 // Write TCAM entries
  	         for(uint32_t ii = 0; ii < 64; ii++) {
                         tmp_x_data = 0;
          		 tmp_x_data = hlp.set_slc( tmp_x_data , val , ii % 64 , ii % 64 );
                         //x_data = key;
                         //y_data = ~key;
		         pxb_csr.dhs_tgt_pmt.entry[ii].x_data(tmp_x_data);
		         pxb_csr.dhs_tgt_pmt.entry[ii].y_data(~tmp_x_data);
		         pxb_csr.dhs_tgt_pmt.entry[ii].valid(~n_iter);
		         pxb_csr.dhs_tgt_pmt.entry[ii].write();
	          }

                 // Read back what was written 
                  unsigned numb_line = (fast_mode == 1) ? 1 : 64;
	          for(uint32_t ii = 0; ii < numb_line ; ii++) {
		          pxb_csr.dhs_tgt_pmt.entry[ii].read();
		          if (verbosity == 1) pxb_csr.dhs_tgt_pmt.entry[ii].show();
		          val = 1;
                          tmp_x_data = 0;
          	 	  tmp_x_data = hlp.set_slc( tmp_x_data , val , ii % numb_line , ii % numb_line );
			  inv_x_data = 1;	
                          inv_x_data <<= 64;
                          inv_x_data = inv_x_data - 1;
		          val = 0;
          	 	  inv_x_data = hlp.set_slc( inv_x_data , val, ii % numb_line , ii % numb_line );
			  //inv_x_data = (~tmp_x_data & ((1 << 64)-1));

                          if (pxb_csr.dhs_tgt_pmt.entry[ii].x_data() != tmp_x_data) {
				fail = 1;
				PLOG_ERR("TCAM Read X data mismatches, rec 0x" << hex << pxb_csr.dhs_tgt_pmt.entry[ii].x_data() << " expected: 0x" << tmp_x_data << endl << dec);
			  }
                          if (pxb_csr.dhs_tgt_pmt.entry[ii].y_data() != inv_x_data) {
				fail = 1;
				PLOG_ERR("TCAM Read Y data mismatches, rec 0x" << hex << pxb_csr.dhs_tgt_pmt.entry[ii].y_data() << " expected: 0x" << inv_x_data << endl << dec);
			  }
                          if (pxb_csr.dhs_tgt_pmt.entry[ii].valid().convert_to< uint32_t >() != (n_iter ? 0 : 1) ) {
				fail = 1;
				PLOG_ERR("TCAM Read Valid bit mismatches, rec 0x" << hex << pxb_csr.dhs_tgt_pmt.entry[ii].valid() << " expected: 0x" << (n_iter ? 0 :1) << endl << dec);
			  }
	          }
		  val = 1;

                 // Search Indirect
	          for(uint32_t ii = 0; ii < 64 ; ii++) {
                          tmp_x_data = 0;
                          pxb_csr.cfg_tgt_pmt_ind.mask(~tmp_x_data) ;     // TCAM Search enable= CEi=all_ones
		          tmp_x_data = hlp.set_slc( tmp_x_data , val , ii % 64 , ii % 64 );
                          pxb_csr.cfg_tgt_pmt_ind.key(tmp_x_data) ; 
                          pxb_csr.cfg_tgt_pmt_ind.write() ; 
                          pxb_csr.dhs_tgt_pmt_ind.entry.write() ; 
                          pxb_csr.sta_tgt_pmt_ind.read() ; 
                          if (verbosity == 1) pxb_csr.sta_tgt_pmt_ind.show() ; 
                          exp_hit = n_iter ? 0 : 1 ; 
                          exp_hit_addr = ii ; 
                          if (pxb_csr.sta_tgt_pmt_ind.hit() != exp_hit) {
				fail = 1;
				PLOG_ERR("TCAM Hit mismatches, rec 0x" << hex << pxb_csr.sta_tgt_pmt_ind.hit() << " expected: 0x" << exp_hit << endl << dec);
			  } else {
                            if (exp_hit && (pxb_csr.sta_tgt_pmt_ind.hit_addr().convert_to< uint32_t >() != ii)) {
				fail = 1;
				PLOG_ERR("TCAM Hit_addr mismatches, rec 0x" << hex << pxb_csr.sta_tgt_pmt_ind.hit() << " expected: 0x" << ii << endl << dec);
			    }
                          }
	          }
             }

             // Walking ones on addresses
             
                 // Write TCAM entries
  	         for(uint32_t ii = 0; ii < (n_tcam_addrbits); ii++) {
                         tmp_x_data = 0;
                         val = (1<< ii) ;  
          		 tmp_x_data = hlp.set_slc( tmp_x_data , val , 0 , 15 );
		         pxb_csr.dhs_tgt_pmt.entry[(1 << ii)].x_data(tmp_x_data);
		         pxb_csr.dhs_tgt_pmt.entry[(1 << ii)].y_data(~tmp_x_data);
		         pxb_csr.dhs_tgt_pmt.entry[(1 << ii)].valid(1);
		         pxb_csr.dhs_tgt_pmt.entry[(1 << ii)].write();
	          }
                 
                 // Search Indirect
	          for(uint32_t ii = 0; ii < (n_tcam_addrbits) ; ii++) {
                          tmp_x_data = 0;
                          pxb_csr.cfg_tgt_pmt_ind.mask(~tmp_x_data) ;     // TCAM Search enable= CEi=all_ones
                          val = (1<< ii) ; 
           		  tmp_x_data = hlp.set_slc( tmp_x_data , val , 0 , 15 );

                          pxb_csr.cfg_tgt_pmt_ind.key(tmp_x_data) ; 
                          pxb_csr.cfg_tgt_pmt_ind.write() ; 
                          pxb_csr.dhs_tgt_pmt_ind.entry.write() ; 
                          pxb_csr.sta_tgt_pmt_ind.read() ; 
                          if (verbosity == 1) pxb_csr.sta_tgt_pmt_ind.show() ; 
                          exp_hit = 1 ; 
                          if (pxb_csr.sta_tgt_pmt_ind.hit() != exp_hit) {
				fail = 1;
				PLOG_ERR("TCAM Hit mismatches, rec 0x" << hex << pxb_csr.sta_tgt_pmt_ind.hit() << " expected: 0x" << exp_hit << endl << dec);
			  }
	          }

                  // issue TCAM grst
                  pxb_csr.cfg_tgt_pmt_grst.vld(1);
                  pxb_csr.cfg_tgt_pmt_grst.write();

                 // Search Again, now it should miss
	          for(uint32_t ii = 0; ii < (n_tcam_addrbits) ; ii++) {
                          tmp_x_data = 0;
                          pxb_csr.cfg_tgt_pmt_ind.mask(~tmp_x_data) ;     // TCAM Search enable= CEi=all_ones
                          val = (1<< ii) ; 
           		  tmp_x_data = hlp.set_slc( tmp_x_data , val , 0 , 15 );

                          pxb_csr.cfg_tgt_pmt_ind.key(tmp_x_data) ; 
                          pxb_csr.cfg_tgt_pmt_ind.write() ; 
                          pxb_csr.dhs_tgt_pmt_ind.entry.write() ; 
                          pxb_csr.sta_tgt_pmt_ind.read() ; 
                          if (verbosity == 1) pxb_csr.sta_tgt_pmt_ind.show() ; 
                          exp_hit = 0 ; 
                          if (pxb_csr.sta_tgt_pmt_ind.hit() != exp_hit) {
				fail = 1;
				PLOG_ERR("TCAM Hit mismatches, rec 0x" << hex << pxb_csr.sta_tgt_pmt_ind.hit() << " expected: 0x" << exp_hit << endl << dec);
			  }
	          }

	    SLEEP(5000);
            cap_pcie_retry_en(chip_id, inst_id,0);

            if (fail == 0) {
               PLOG_MSG("=== PCIE TCAM pass chip_id:" << chip_id << " inst_id:" << inst_id << endl << dec);
            } else {
               PLOG_ERR("=== PCIE TCAM fail chip_id:" << chip_id << " inst_id:" << inst_id << endl << dec);
            }

            return (fail);
}



void randomize_csr_with_knob(cap_csr_base * csr_ptr , Knob * knob) {
    int width = csr_ptr->get_width();
    cpp_int data;
    for(int i = 0; i < (width+31)/32; i++) {
        data = (data << 32) | knob->eval();
    }
    csr_ptr->all(data);


}



int cap_run_pcie_tcam_rdwr_test_NEW(int chip_id, int inst_id, int fast_mode, int verbosity) {

    //cap_pp_csr_t & pp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pp_csr_t, chip_id, inst_id);
    cap_pxb_csr_t & pxb_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, chip_id, inst_id);


    cpp_int tmp_x_data;
    cpp_int exp_hit; 
    cpp_int exp_hit_addr; 

    auto & mem_ref = pxb_csr.dhs_tgt_pmt;
    auto & cfg_indirect = pxb_csr.cfg_tgt_pmt_ind; 
    auto & sta_indirect = pxb_csr.sta_tgt_pmt_ind; 
    auto & cfg_indirect_search = pxb_csr.dhs_tgt_pmt_ind.entry;
    auto & cfg_tcam_reset = pxb_csr.cfg_tgt_pmt_grst;
    uint32_t tcam_depth = pxb_csr.dhs_tgt_pmt.get_depth_entry();
    if(verbosity) { 
        PLOG_MSG("TCAM test starts for : " << pxb_csr.dhs_tgt_pmt.get_hier_path() << endl);
    }

    unsigned fail  = PLOG_GET_ERR_CNT();
    uint32_t start_idx  = 0;
    uint32_t n_tcam_addrbits = ceil(log2(tcam_depth));

    pknobs::RRKnob rand_knob("rand", 0, 0xffffffff);
    unsigned num_entries = mem_ref.get_depth_entry();
    if(fast_mode >= 1) {
        switch(fast_mode) {
            case 2 : num_entries = (num_entries*2)/3; break; 
            case 3 : num_entries = (num_entries*1)/2; break; 
            case 4 : num_entries = (num_entries*1)/3; break; 
            case 5 : num_entries = (num_entries*1)/4; break; 
            case 6 : {
                         num_entries = (num_entries*1)/10; 
                         start_idx = tcam_depth - num_entries; 
                         break; 
                     }
            case 7 : {
                         num_entries = (num_entries*1)/10; 
                         start_idx = 0; 
                         break; 
                     }

            default: num_entries = 64; break; 
        }
    }
    uint32_t n_tcam_key_size = (mem_ref.entry[0].get_width()-1)/2;

    if(verbosity) { 
        PLOG_MSG("tcam_depth    " << tcam_depth << endl);
        PLOG_MSG("num_entries   " << num_entries << endl);
        PLOG_MSG("n_tcam_key_size " << n_tcam_key_size << endl);
        PLOG_MSG("n_tcam_addrbits " << n_tcam_addrbits << endl);
        PLOG_MSG("start_idx " << start_idx << endl);
    }


    // issue TCAM grst
    cfg_tcam_reset.vld(1);
    cfg_tcam_reset.write();
    SLEEP(2);



    // Walking ones on TCAM data , all entries 
    for (uint32_t n_iter=0; n_iter < 2; n_iter++ ) { 

        if(verbosity) {
            if(n_iter == 0) { PLOG_MSG("TCAM random write-read back test, case 0, X one hot, Y random" << endl); }
            else if(n_iter == 1) { PLOG_MSG("TCAM random write-read back test, case 1, Y one hot, X random" << endl); }
        }

        // case 0 : Write TCAM entries, X one-hot and Y random value
        // case 1 : Write TCAM entries, Y one-hot and X random value
        for(uint32_t ii = start_idx; ii < num_entries; ii++) {
            tmp_x_data = 1;
            tmp_x_data <<= (ii % n_tcam_key_size);
            uint32_t idx = ii % num_entries; 
            randomize_csr_with_knob(&mem_ref.entry[idx], &rand_knob);
            if(n_iter == 0) { 
                mem_ref.entry[idx].x_data(tmp_x_data);
                mem_ref.entry[idx].y_data( mem_ref.entry[idx].y_data() & ~tmp_x_data);
            } else {
                mem_ref.entry[idx].y_data(tmp_x_data);
                mem_ref.entry[idx].x_data( mem_ref.entry[idx].x_data() & ~tmp_x_data);
            }
            mem_ref.entry[idx].write();
        }

        // Read back what was written 
        for(uint32_t ii = start_idx; ii < num_entries; ii++) {
            unsigned valid = mem_ref.entry[ii].valid().convert_to<unsigned>();
            if(valid) {
                mem_ref.entry[ii].read_compare();
            } else {
                mem_ref.entry[ii].read();
                if(mem_ref.entry[ii].valid().convert_to<unsigned>()) {
                    PLOG_ERR("PXB TCAM: idx " << ii << " valid expected 0 but got 1 " << endl);
                }
            }
        }
    }


    // invalidate all entries
    cfg_tcam_reset.vld(1);
    cfg_tcam_reset.write();
    SLEEP(2);

    // Walking ones on TCAM data , all entries 
    for (uint32_t n_iter=0; n_iter < 2; n_iter++ ) { 

        if(verbosity) {
            if(n_iter == 0) { PLOG_MSG("TCAM one-hot write on X, inverted Y, valid : 1 - indirect search to check" << endl); }
            else if(n_iter == 1) { PLOG_MSG("TCAM one-hot write on X, inverted Y, valid : 0 - indirect search to check" << endl); }
        }


        // Write TCAM entries X = idx, Y=~X
        for(uint32_t ii = start_idx; ii < n_tcam_key_size; ii++) {
            tmp_x_data = 1;
            tmp_x_data <<= (ii % n_tcam_key_size);
            uint32_t idx = ii % num_entries;
            mem_ref.entry[idx].x_data(tmp_x_data);
            mem_ref.entry[idx].y_data(~tmp_x_data);
            mem_ref.entry[idx].valid(~n_iter);
            mem_ref.entry[idx].write();
        }

        // Search Indirect
        for(uint32_t ii = start_idx; ii < n_tcam_key_size; ii++) {
            tmp_x_data = 0;
            cfg_indirect.mask(~tmp_x_data) ;     // TCAM Search enable= CEi=all_ones
            tmp_x_data = 1;
            tmp_x_data <<= (ii % n_tcam_key_size);
            cfg_indirect.key(tmp_x_data) ; 
            cfg_indirect.write() ; 
            cfg_indirect_search.write() ; 
            sta_indirect.read() ; 
            //if (verbosity == 1) sta_indirect.show() ; 
            exp_hit = n_iter ? 0 : 1 ; 
            exp_hit_addr = ii % num_entries; 
            if (sta_indirect.hit() != exp_hit) {
                PLOG_ERR("TCAM Hit mismatches, rec 0x" << hex << sta_indirect.hit() << " expected: 0x" << exp_hit << endl << dec);
            } else {
                if (exp_hit && (sta_indirect.hit_addr().convert_to< uint32_t >() != exp_hit_addr)) {
                    PLOG_ERR("TCAM Hit_addr mismatches, rec 0x" << hex << sta_indirect.hit() << " expected: 0x" << exp_hit_addr << endl << dec);
                }
            }
        }
    }

    cfg_tcam_reset.vld(1);
    cfg_tcam_reset.write();
    SLEEP(2);


    // Walking ones on addresses

    // Write TCAM entries -- address bits
    if(verbosity) {
        PLOG_MSG("TCAM one-hot write on index - indirect search to check" << endl);
    }
    for(uint32_t ii = 0; ii < (n_tcam_addrbits); ii++) {
        tmp_x_data = 1;
        tmp_x_data <<= ii;
        uint32_t idx = (start_idx + (1 << ii)) % tcam_depth; 
        mem_ref.entry[idx].x_data(tmp_x_data);
        mem_ref.entry[idx].y_data(~tmp_x_data);
        mem_ref.entry[idx].valid(1);
        mem_ref.entry[idx].write();
    }

    // Search Indirect
    for(uint32_t ii = 0; ii < (n_tcam_addrbits) ; ii++) {
        tmp_x_data = 0;
        cfg_indirect.mask(~tmp_x_data) ;     // TCAM Search enable= CEi=all_ones
        cfg_indirect.key( 1 << ii ) ; 
        cfg_indirect.write() ; 
        cfg_indirect_search.write() ; 
        sta_indirect.read() ; 
        //if (verbosity == 1) sta_indirect.show() ; 
        exp_hit = 1 ; 
        exp_hit_addr = (start_idx + (1 << ii)) % tcam_depth; 
        if (sta_indirect.hit() != exp_hit) {
            PLOG_ERR("TCAM Hit mismatches, rec 0x" << hex << sta_indirect.hit() << " expected: 0x" << exp_hit << endl << dec);
        }
        if (exp_hit && (sta_indirect.hit_addr().convert_to< uint32_t >() != exp_hit_addr)) {
            PLOG_ERR("TCAM Hit_addr mismatches, rec 0x" << hex << sta_indirect.hit() << " expected: 0x" << exp_hit_addr << endl << dec);
        }

    }

    if(verbosity) {
        PLOG_MSG("TCAM invalid - indirect search to check" << endl);
    }

    // issue TCAM grst, only write event is necessary
    // write event with data=1 is enough
    cfg_tcam_reset.vld(1);
    cfg_tcam_reset.write();
    SLEEP(2);

    // Search Again, now it should miss
    for(uint32_t ii = 0; ii < (n_tcam_addrbits) ; ii++) {
        tmp_x_data = 0;
        cfg_indirect.mask(~tmp_x_data) ;     // TCAM Search enable= CEi=all_ones
        cfg_indirect.key(1 << ii) ; 
        cfg_indirect.write() ; 
        cfg_indirect_search.write() ; 
        sta_indirect.read() ; 
        //if (verbosity == 1) sta_indirect.show() ; 
        exp_hit = 0 ; 
        if (sta_indirect.hit() != exp_hit) {
            PLOG_ERR("TCAM Hit mismatches, rec 0x" << hex << sta_indirect.hit() << " expected: 0x" << exp_hit << endl << dec);
        }
    }

    if(PLOG_GET_ERR_CNT() == fail) {
        PLOG_MSG("=== PCIE TCAM pass chip_id:" << chip_id << " inst_id:" << inst_id << endl << dec);
    } else {
        PLOG_ERR("=== PCIE TCAM fail chip_id:" << chip_id << " inst_id:" << inst_id << endl << dec);
    }

    return (PLOG_GET_ERR_CNT() - fail);
}

void cap_pcie_program_vnic_entries(int chip_id, int inst_id, uint64_t bar , uint64_t capri_address, int port, int tcam_idx, int bar_size, int prt_base, int prt_count) {
    cap_pxb_decoders_pmt_tcam_key_type_memio_entry_t pmt_memio_entry;
    cap_pxb_decoders_pmr_key_type_memio_t pmr_memio_entry;
    cpp_int_helper hlp;
    pmt_memio_entry.set_name("pmt");
    pmt_memio_entry.init();
    pmr_memio_entry.set_name("pmr");
    pmr_memio_entry.init();

    pmt_memio_entry.valid(1);
    pmt_memio_entry.mask.all(0);
    pmt_memio_entry.mask.all( pmt_memio_entry.mask.all() -1 ); 
    pmt_memio_entry.mask.common.port_id(0);
    pmt_memio_entry.key.common.port_id(port);
    pmt_memio_entry.mask.common.valid(0);
    pmt_memio_entry.key.common.valid(1);
    pmt_memio_entry.mask.common.key_type(0);
    pmt_memio_entry.key.common.key_type(CAP_PCIE_API_KEY_TYPE_MEM);
    pmt_memio_entry.mask.address( (1 << (bar_size-2))-1);
    pmt_memio_entry.key.address(bar >> 2ULL);
    // Address
    // PRT SIZE: bar_size:0
    // PRT IDX : constant 

    pmr_memio_entry.valid(1);
    pmr_memio_entry.prt_base(prt_base);
    pmr_memio_entry.prt_size(bar_size);
    pmr_memio_entry.prt_count(1);

    pmr_memio_entry.vf_start(35);
    pmr_memio_entry.vf_endplus1(2); 
    pmr_memio_entry.vf_limit(0x7fff);


    pmr_memio_entry.cpl_bdf(0x0);
    pmr_memio_entry.vfid_start(0);
    pmr_memio_entry.key_type(CAP_PCIE_API_KEY_TYPE_MEM);
       

    if(PLOG_CHECK_MSG_LEVEL("pcie_api")) { 
        pmt_memio_entry.show();
        pmr_memio_entry.show();
    }

    cap_pcie_program_key_type_memio_tcam_sram_pair(chip_id, inst_id, tcam_idx, pmr_memio_entry, pmt_memio_entry);
    cap_pxb_decoders_prt_memio_t prt_memio_entry;
    prt_memio_entry.all(0);
    prt_memio_entry.valid(1);
    prt_memio_entry.prt_type(CAP_PCIE_API_RESOURCE_PRT);
    prt_memio_entry.resource_dwsize(0x7fff);
    prt_memio_entry.vf_stride(0);
    prt_memio_entry.pmv_check_disable(1);
    for(unsigned i=0; i < (unsigned) prt_count; i++) {
      cpp_int tmp_addr;
      tmp_addr = capri_address + ((1ULL << bar_size) * i);
      prt_memio_entry.resource_dwaddr(tmp_addr >> 2); 

      if(PLOG_CHECK_MSG_LEVEL("pcie_api")) { 
          prt_memio_entry.show();
      }
      cap_pcie_program_prt_type_memio_sram(chip_id, inst_id, prt_base + i, prt_memio_entry);
    }

} 
#endif

#ifdef _COSIM_
#include "cap_pcie_enum_cfg.cc"
#endif
