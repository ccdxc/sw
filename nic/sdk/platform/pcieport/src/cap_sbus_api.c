#ifdef CAPRI_SW
#include "cap_sw_glue.h"
#endif
#include "cap_sbus_api.h"
#ifndef CAPRI_SW
#include "cap_pcie_api.h"
#include "cap_pp_csr.h"
#endif
#include "cap_pp_c_hdr.h"
#ifndef _PCIE_BLOCK_LEVEL_
#ifndef CAPRI_SW
#include "cap_ms_csr.h"
#endif    
#include "cap_mc_c_hdr.h"
#include "cap_ms_c_hdr.h"
#ifdef _CSV_INCLUDED_    
extern "C" void hbm_download_serdes_code(const char * id);
#endif    
#endif // _PCIE_BLOCK_LEVEL_
#ifndef SWPRINTF
#define SW_PRINT printf
#endif
#ifdef CAPRI_SW
#define PLOG(f, ...) do {} while (0)
#endif

void cap_sbus_reg_write(uint64_t addr, uint32_t wr_data) {
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

uint32_t cap_sbus_reg_read(uint64_t addr) {
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

#ifndef CAPRI_SW
void *romfile_open(const void *rom_info)
{
    return fopen((char *)rom_info, "r");
}

int romfile_read(void *f, unsigned int *datap)
{
    return fscanf((FILE *)f, "%x", datap);
}

void romfile_close(void *f)
{
    fclose((FILE *)f);
}
#endif

#ifndef _PCIE_BLOCK_LEVEL_
void cap_ms_sbus_write(int chip_id, int rcvr_addr, int data_addr, unsigned int data) {

  uint32_t val;

  val = rcvr_addr |
	(data_addr << CAP_MS_CSR_CFG_SBUS_INDIR_DATA_ADDR_LSB) |
	(CAP_SBUS_SBUS_WR << CAP_MS_CSR_CFG_SBUS_INDIR_COMMAND_LSB) ;
  cap_sbus_reg_write((CAP_ADDR_BASE_MS_MS_OFFSET) + CAP_MS_CSR_CFG_SBUS_INDIR_BYTE_ADDRESS, val);
  cap_sbus_reg_write((CAP_ADDR_BASE_MS_MS_OFFSET) + CAP_MS_CSR_DHS_SBUS_INDIR_BYTE_ADDRESS, data);
}

unsigned int cap_ms_sbus_read(int chip_id, int rcvr_addr, int data_addr) {

  int rd_data;
  uint32_t val;
  val = rcvr_addr |
	(data_addr << CAP_MS_CSR_CFG_SBUS_INDIR_DATA_ADDR_LSB) |
	(CAP_SBUS_SBUS_RD << CAP_MS_CSR_CFG_SBUS_INDIR_COMMAND_LSB) ;
  cap_sbus_reg_write((CAP_ADDR_BASE_MS_MS_OFFSET) + CAP_MS_CSR_CFG_SBUS_INDIR_BYTE_ADDRESS, val);
  rd_data = cap_sbus_reg_read((CAP_ADDR_BASE_MS_MS_OFFSET) + CAP_MS_CSR_DHS_SBUS_INDIR_BYTE_ADDRESS);

  return rd_data;
}

//////////////////////////////////////////////////////////////////////////////
void cap_ms_sbus_reset(int chip_id, int rcvr_addr) {

  uint32_t val;
  uint32_t data_addr=0;
  val = rcvr_addr |
	(data_addr << CAP_MS_CSR_CFG_SBUS_INDIR_DATA_ADDR_LSB) |
	(CAP_SBUS_SBUS_RST << CAP_MS_CSR_CFG_SBUS_INDIR_COMMAND_LSB) ;
  cap_sbus_reg_write((CAP_ADDR_BASE_MS_MS_OFFSET) + CAP_MS_CSR_CFG_SBUS_INDIR_BYTE_ADDRESS, val);
  cap_sbus_reg_write((CAP_ADDR_BASE_MS_MS_OFFSET) + CAP_MS_CSR_DHS_SBUS_INDIR_BYTE_ADDRESS, 0);
}


#else 
void hbm_download_serdes_code(const char * id) { }
void cap_ms_sbus_write(int chip_id, int rcvr_addr, int data_addr, int data) { }
int cap_ms_sbus_read(int chip_id, int rcvr_addr, int data_addr) { return 0xdead; }
#endif // _PCIE_BLOCK_LEVEL_



//////////////////////////////////////////////////////////////////////////////
void cap_pp_sbus_reset(int chip_id, int rcvr_addr) {

  uint32_t val;
  uint32_t data_addr=0;

  val = rcvr_addr |
	(data_addr << CAP_PP_CSR_CFG_SBUS_INDIR_DATA_ADDR_LSB) |
	(CAP_SBUS_SBUS_RST << CAP_PP_CSR_CFG_SBUS_INDIR_COMMAND_LSB) ;
  cap_sbus_reg_write((CAP_ADDR_BASE_PP_PP_OFFSET) + CAP_PP_CSR_CFG_SBUS_INDIR_BYTE_ADDRESS, val);
  cap_sbus_reg_write((CAP_ADDR_BASE_PP_PP_OFFSET) + CAP_PP_CSR_DHS_SBUS_INDIR_BYTE_ADDRESS, 0);
  //PLOG_MSG("KCM:cap_pp_sbus_reset:0x" << hex << rcvr_addr << dec << endl);
}

void cap_pp_sbus_write(int chip_id, int rcvr_addr, int data_addr, unsigned int data) {

  uint32_t val;

   PLOG("cap_pp_sbus", " cap_pp_sbus_write:0x" << hex << rcvr_addr << " data_addr: 0x" << hex << data_addr << " data:0x" << data << dec << endl);
  val = rcvr_addr |
	(data_addr << CAP_PP_CSR_CFG_SBUS_INDIR_DATA_ADDR_LSB) |
	(CAP_SBUS_SBUS_WR << CAP_PP_CSR_CFG_SBUS_INDIR_COMMAND_LSB) ;
  cap_sbus_reg_write((CAP_ADDR_BASE_PP_PP_OFFSET) + CAP_PP_CSR_CFG_SBUS_INDIR_BYTE_ADDRESS, val);
  cap_sbus_reg_write((CAP_ADDR_BASE_PP_PP_OFFSET) + CAP_PP_CSR_DHS_SBUS_INDIR_BYTE_ADDRESS, data);
}

unsigned int cap_pp_sbus_read(int chip_id, int rcvr_addr, int data_addr) {

  int rd_data;
  uint32_t val;
  PLOG("cap_pp_sbus", " cap_pp_sbus_read:0x" << hex << rcvr_addr << " data_addr: 0x" << hex << data_addr << dec << endl);

  val = rcvr_addr |
	(data_addr << CAP_PP_CSR_CFG_SBUS_INDIR_DATA_ADDR_LSB) |
	(CAP_SBUS_SBUS_RD << CAP_PP_CSR_CFG_SBUS_INDIR_COMMAND_LSB) ;
  cap_sbus_reg_write((CAP_ADDR_BASE_PP_PP_OFFSET) + CAP_PP_CSR_CFG_SBUS_INDIR_BYTE_ADDRESS, val);
  rd_data = cap_sbus_reg_read((CAP_ADDR_BASE_PP_PP_OFFSET) + CAP_PP_CSR_DHS_SBUS_INDIR_BYTE_ADDRESS);

  return rd_data;
}

#ifdef ASIC_ORIG

//----------------------------------------
// Uploads spico firmware directly into imem rather than via sbus comands.
// Firmware is passed in as a system verilog string.
// - Refer to: SBusMaster16_Spec11.pdf
//     Table 12 : Spico register map
//     Table  9 : Sbus address
//          Spico, sbus addr:0xfd
//
// firmware: /home/asic/vendors/brcm/design_kit/latest/serdes/firmware/serdes.0x108C_2347.rom
//----------------------------------------
#ifndef CAPRI_SW
void upload_sbus_master_firmware(int chip_id, int sbus_ring_ms, int device_addr, const void* rom_info) {
#else 
void upload_sbus_master_firmware(int chip_id, int sbus_ring_ms, int device_addr, void* rom_info) {
#endif

   PLOG_MSG("Sbus Upload firmware: " << (char*) rom_info << " device_addr: 0x" << hex << device_addr << dec << endl);
   SW_PRINT("NWL/PCIE:SBUS:Upload SBUS \n");
 
   void *ctx;
   ctx = romfile_open(rom_info);
 
   if (ctx == NULL) {
#ifndef CAPRI_SW
      PLOG_ERR("upload_sbus_master_firmware :: Exiting function...Unable to open file : " << (char *)rom_info << endl);
#endif
      return;
   }
 
   int word_count;         // number of words in the burst
   int addr;               // address
   int valid;              // Returns 1 for each word read.  Returns -1 for end-of-file.
   // 32 bits
   unsigned int sbus_data;       // Addr/Data sent via sbus to load rom 
   // 10 bits
   unsigned int imem_data;        // data word read from file
 
   if (sbus_ring_ms) {
     // SBusMaster16_Spec: Table 10 
     cap_ms_sbus_write(chip_id, device_addr, 0x1, 0xc0);  // Place SPICO into Reset and Enable off
     cap_ms_sbus_write(chip_id, device_addr, 0x1, 0x240); // Remove Reset, Enable off, IMEM_CNTL_EN on
     cap_ms_sbus_write(chip_id, device_addr, 0x3, 0x80000000); // Set starting IMEM address for burst download
   } else {
     // SerDes16_Spec: Table 67
     cap_pp_sbus_write(chip_id, device_addr, 0x7, 0x11);  // Place SerDes in Reset and disable SPICO
     cap_pp_sbus_write(chip_id, device_addr, 0x7, 0x10); // Remove SerDes Reset
     cap_pp_sbus_write(chip_id, device_addr, 0x0, 0xc0000000); // Set starting IMEM override
   }

#ifndef CAPRI_SW 
   bool load_frontdoor_ms   = sknobs_get_value((char*)"test/sbus/load_rom_file_frontdoor",1) && (sbus_ring_ms == 1);
   bool load_frontdoor_pcie = sknobs_get_value((char*)"test/sbus/load_pcie_rom_file_frontdoor",0) && (sbus_ring_ms == 0);
#else 
   bool load_frontdoor_ms = 1;
   bool load_frontdoor_pcie = 1;
#endif

   if (load_frontdoor_ms || load_frontdoor_pcie) {   

     // Burst Load the ROM code
     addr  = 0;
     valid = 1;
     
     while (valid > 0) {
       sbus_data = 0;
       word_count = 0;
       
       // data word 1
       valid = romfile_read(ctx, &imem_data);
       //PLOG_MSG( "DEBUG: 1 Spico valid: 0x" << hex << valid << " imem_data: 0x" << imem_data << dec << endl);
       if (valid  > 0) {
	 sbus_data = imem_data & 0x3ff;
	 word_count++;
	 valid = romfile_read(ctx, &imem_data);
	 //PLOG_MSG( "DEBUG: 2 Spico valid: 0x" << hex << valid << " imem_data: 0x" << imem_data << dec << endl);
       }
       
       // data word 2
       if (valid  > 0) {
	 sbus_data = sbus_data | ((imem_data & 0x3ff) << 10);
	 word_count++;
	 valid = romfile_read(ctx, &imem_data);
	 //PLOG_MSG( "DEBUG: 3 Spico valid: 0x" << hex << valid << " imem_data: 0x" << imem_data << dec << endl);
       }
       
       // data word 3
       if (valid  > 0) {
	 sbus_data = sbus_data | ((imem_data & 0x3ff) << 20);
	 word_count++;
       }
       
       // Word count
       if (word_count  > 0) {
	 sbus_data = sbus_data | (word_count << 30);
	 
	 // Send the instruction burst
	 // PLOG_MSG( "DEBUG: Spico upload addr: 0x" << hex << addr << " data: 0x" << sbus_data << dec << endl);
	 if (sbus_ring_ms) {
	   cap_ms_sbus_write(chip_id, device_addr, 0x14, sbus_data);
	 } else {
	   cap_pp_sbus_write(chip_id, device_addr, 0xa, sbus_data);
	 }
	 addr += word_count;
       }
     }

     if(sbus_ring_ms == 0) {
         // do a read to flush all the write
         PLOG_MSG("PP read to flush all writes, this will cause firmware download completed" << endl);
         cap_pp_sbus_read(chip_id, 1, 0xff);
     }

     romfile_close(ctx);

   }   
   else {
     if (sbus_ring_ms) {
#ifdef _CSV_INCLUDED_    
	  svScope old_scope = svGetScope();
	  svScope new_scope = svGetScopeFromName("top_tb");
	  svSetScope(new_scope); 
#ifndef NX_MS_NWL_TB 
#ifndef NX_MS_TB 
	  hbm_download_serdes_code("default");
#endif
#endif
	  svSetScope(old_scope);
	  PLOG_MSG("HBM backdoor firmware download completed" << endl);
#else
	  PLOG_ERR("HBM backdoor firmware download only supported in _CSV_INCLUDED_ (verilog) mode" << endl);
#endif
	  PLOG_MSG("HBM frontdoor firmware download completed" << endl);
     }
     else {
       PLOG_ERR("backdoor firmware load only supported for HBM for now" << endl);
     }
   }

   if (sbus_ring_ms) {
     uint32_t data;
    if(load_frontdoor_ms) {
        cap_ms_sbus_write(chip_id, device_addr, 0x14, 0xc0000000); // Pad with 0's
        cap_ms_sbus_write(chip_id, device_addr, 0x14, 0xc0000000); // Pad with 0's
        cap_ms_sbus_write(chip_id, device_addr, 0x14, 0xc0000000); // Pad with 0's
     }
     // SBusMaster16_Spec: Table 10 
     cap_ms_sbus_write(0, device_addr, 0x1, 0x40);  // Place SPICO into Reset and Enable off
     cap_ms_sbus_write(0, device_addr, 0x1, 0x140); // Remove Reset, Enable off, IMEM_CNTL_EN on	  

     PLOG_MSG("MS read to flush all writes, this will cause firmware download completed" << endl);
     data = cap_ms_sbus_read(chip_id, 1, 0xff);
     SW_PRINT("NWL:SBUS:Done with load .. flushed \n");
     (void)data;
   } else {
     // SerDes16_Spec: Table 67
     cap_pp_sbus_write(chip_id, device_addr, 0xa, 0xc0000000); // Pad with 0's
     cap_pp_sbus_write(chip_id, device_addr, 0xa, 0xc0000000); // Pad with 0's
     cap_pp_sbus_write(chip_id, device_addr, 0xa, 0xc0000000); // Pad with 0's

     cap_pp_sbus_write(chip_id, device_addr, 0x0, 0x0); // IMEM override off
     cap_pp_sbus_write(chip_id, device_addr, 0xb, 0xc0000); // Turn ECC on
     cap_pp_sbus_write(chip_id, device_addr, 0x7, 0x2); // Turn SPICO Enable on
     cap_pp_sbus_write(chip_id, device_addr, 0x8, 0x0); // Enable core and hw interrupts

     // do a read to flush all the write
     PLOG_MSG("PP read to flush all writes, this will cause firmware download completed" << endl);
     cap_pp_sbus_read(chip_id, 1, 0xff);
     SW_PRINT("PCIE:SBUS:Done with load .. flushed \n");
   }

 
   // Give Spico time to get through init code
   PLOG_MSG( "SBUS: Waiting for spico to initialize" << endl);
 
   SW_PRINT("NWL/PCIE:SBUS:Is sleep enough? \n");
   SLEEP(1000);
   // 1000 SBUS_MASTER_PATH.spico_clk
   // $display( "DEBUG: %0t Waiting for spico timout:%0d, state:%0d", $time, timeout, `SBUS_MASTER_PATH.spico.spico_main.spico_state );
   // (`SBUS_MASTER_PATH.spico.spico_main.spico_state == 5'h1f || `SBUS_MASTER_PATH.spico.spico_main.spico_state == 5'h12) 
   //   if (`SBUS_MASTER_PATH.spico.spico_main.spico_state != 'h12 ) 
   //     $display( "ERROR: %0t spico processor failured to initialize", $time );
}

#else // ASIC_ORIG
static void
upload_ms_sbus_fw(const int device_addr, void *ctx)
{
}

static void
upload_pp_sbus_fw(const int device_addr, void *rom_info)
{
    const int chip_id = 0;
    unsigned int sbus_data;
    void *ctx = romfile_open(rom_info);

    if (ctx == NULL) {
        SW_PRINT("romfile_open failed\n");
        return;
    }

    // SerDes16_Spec: Table 67

    // Place SerDes in Reset and disable SPICO
    cap_pp_sbus_write(chip_id, device_addr, 0x7, 0x11);
    // Remove SerDes Reset
    cap_pp_sbus_write(chip_id, device_addr, 0x7, 0x10);
    // Set starting IMEM override
    cap_pp_sbus_write(chip_id, device_addr, 0x0, 0xc0000000);

    // Burst Load the ROM code
    while (romfile_read(ctx, &sbus_data)) {
        cap_pp_sbus_write(chip_id, device_addr, 0xa, sbus_data);
    }
    // do a read to flush all the write
    cap_pp_sbus_read(chip_id, 1, 0xff);

    romfile_close(ctx);

    // SerDes16_Spec: Table 67
    cap_pp_sbus_write(chip_id, device_addr, 0xa, 0xc0000000); // Pad with 0's
    cap_pp_sbus_write(chip_id, device_addr, 0xa, 0xc0000000); // Pad with 0's
    cap_pp_sbus_write(chip_id, device_addr, 0xa, 0xc0000000); // Pad with 0's

    cap_pp_sbus_write(chip_id, device_addr, 0x0, 0x0); // IMEM override off
    cap_pp_sbus_write(chip_id, device_addr, 0xb, 0xc0000); // Turn ECC on
    cap_pp_sbus_write(chip_id, device_addr, 0x7, 0x2); // Turn SPICO Enable on
    // Enable core and hw interrupts
    cap_pp_sbus_write(chip_id, device_addr, 0x8, 0x0);

    // do a read to flush all the write
    cap_pp_sbus_read(chip_id, 1, 0xff);
    SW_PRINT("PCIE:SBUS:Done with load .. flushed \n");

    SW_PRINT("NWL/PCIE:SBUS:Is sleep enough? \n");
    SLEEP(1000);
}

void
upload_sbus_master_firmware(int chip_id,
                            int sbus_ring_ms,
                            int device_addr,
                            void *rom_info)
{
    SW_PRINT("NWL/PCIE:SBUS:Upload SBUS \n");

    if (sbus_ring_ms) {
        upload_ms_sbus_fw(device_addr, rom_info);
    } else {
        upload_pp_sbus_fw(device_addr, rom_info);
    }
}
#endif // ASIC_ORIG

//----------------------------------------
// Issue a spico interrupt and wait for it to complete
//----------------------------------------
uint32_t sbus_master_spico_interrupt(int chip_id, int sbus_ring_ms, int device_addr, int interrupt_code, int interrupt_value, uint32_t data, int no_busy_check, int wait_timeout) {
   int timeout;
   int error;

  PLOG_MSG("KCM:sbus_master_spico_interrupt: " << hex << device_addr << " data_addr: 0x" << hex << interrupt_code << dec << endl);
   // Assert Interrupt
   if (sbus_ring_ms) {
      // Set spico interrupt code and value
      // refer to SBusMaster16_Spec_11 table 12
      cap_ms_sbus_write(chip_id, device_addr, 2, ((interrupt_value << 16) | (interrupt_code & 0xffff)));

      data = cap_ms_sbus_read(chip_id, device_addr, 7);
      cap_ms_sbus_write(chip_id, device_addr, 7, (data | 1));

      // Lower Interrupt
      cap_ms_sbus_write(chip_id, device_addr, 7, (data & 0xfffffffe));
   } else {
      // refer to SerDes16_Spec table 67
      cap_pp_sbus_write(chip_id, device_addr, 3, ((interrupt_code << 16) | (interrupt_value & 0xffff)));

      data = cap_pp_sbus_read(chip_id, device_addr, 4);

      // Lower Interrupt
      ///cap_pp_sbus_write(chip_id, device_addr, 7, (data & 0xfffffffe));
   }

   if (no_busy_check == 0) {
     // Wait for interrupt to complete
     timeout = wait_timeout;
     PLOG_MSG("SBUS: Waiting for interrupt 0x" << hex << (interrupt_code  & 0xffff) << dec << " to complete" << endl);
     while (timeout > 0) {
       if (sbus_ring_ms) {
         data = cap_ms_sbus_read(chip_id, device_addr, 8);
       } else {
         data = cap_pp_sbus_read(chip_id, device_addr, 4);
       }
       PLOG_MSG("SBUS: Interrupt 0x" << hex << (interrupt_code & 0xffff) << " result/status: 0x" << (data >> 16) << " / 0x" <<  (data & 0xffff) << dec << endl);
  
       // Check if interrupt busy flag is set 
       if (sbus_ring_ms) {
         if ((data & 0x8000) == 0)
           break;
       } else {
         if ((data & 0x10000) == 0) // 0: interrupt_in_prog
           break;
       }
  
       SLEEP(1000);
       timeout = timeout - 1;
     }
  
     error = 0;
     if (timeout == 0) {
       PLOG_ERR( "ERROR: Interrupt 0x" << hex << (interrupt_code  & 0xffff) << " timeout while waiting for completion." << dec << endl);
       error = 1;
     } else {
       // Read again for since the read is un-triggered, this means the data read when the
       // status changed to "done" may not be valid. So read again to get a valid result data.
       if (sbus_ring_ms) {
          data = cap_ms_sbus_read(chip_id, device_addr, 8);
          data = (data >> 16) & 0xffff;
       } else {
          data = cap_pp_sbus_read(chip_id, device_addr, 4);
          data = data & 0xffff;
       }
     }
     error = error + 1; // FIXME Suresh
     return data;
   } else {
     return 0;
   }

}

/*****
uint32_t sbus_master_sbus_interrupt(int chip_id, int device_addr, int interrupt_code_data) {
   int timeout;
   int error;

   // Assert Interrupt
   // refer to SpicoFW_21 table 5
   cap_pp_sbus_write(chip_id, device_addr, 3, interrupt_code_data);

   data = cap_pp_sbus_read(chip_id, device_addr, 4);

   // Wait for interrupt to complete
   timeout = 100;
   PLOG_MSG("SBUS: Waiting for interrupt code/data 0x" << hex << interrupt_code_data << " to complete" << endl);
   while (timeout > 0) {
     data = cap_pp_sbus_read(chip_id, device_addr, 4);
     PLOG_MSG("SBUS: Interrupt 0x" << hex << (interrupt_code & 0xffff) << " result/status: 0x" << (data >> 16) << " / 0x" <<  (data & 0xffff) << dec << endl);

     // Check if interrupt busy flag is set 
     if (data & 0x4000) {
     } else {
     }// valid == 0
     if (data & 0x8000)
       break;

     SLEEP(1000);
     timeout = timeout - 1;
   }

   error = 0;
   if (timeout == 0) {
     PLOG_ERR( "ERROR: Interrupt 0x" << hex << (interrupt_code  & 0xffff) << " timeout while waiting for completion." << dec << endl);
     error = 1;
   } else {
     // Read again for since the read is un-triggered, this means the data read when the
     // status changed to "done" may not be valid. So read again to get a valid result data.
     if (sbus_ring_ms) {
        data = cap_ms_sbus_read(chip_id, device_addr, 8);
     } else {
        data = cap_pp_sbus_read(chip_id, device_addr, 8);
     }
     data = (data >> 16) & 0xffff;
   }

   return data;
}
****/


//----------------------------------------
// Prints the firmware revision.  Firmware mus be uploaded first. 
//----------------------------------------
uint32_t report_firmware_revision(int sbus_ring_ms, int device_addr) {
   uint32_t data;

   data = sbus_master_spico_interrupt(0, sbus_ring_ms, device_addr, 0x0, 0, 0, 0, 100); // FIXME Suresh data
   PLOG_MSG( "SBUS: Spico Firmware Revision: 0x" << hex << data << " device_addr: 0x" << device_addr << dec << endl);
   SW_PRINT("NWL:SBUS:Firmware Revision %d, device address %d\n", data, device_addr);
   return data;
}


//----------------------------------------
// Prints the firmware build ID.  Firmware mus be uploaded first. 
//----------------------------------------
uint32_t report_firmware_build_id(int sbus_ring_ms, int device_addr) {
   uint32_t data;

   data = sbus_master_spico_interrupt(0, sbus_ring_ms, device_addr, 0x1, 0, 0, 0, 100); // FIXME Suresh data
   PLOG_MSG( "SBUS: Spico Firmware Build ID:: 0x" << hex << data << " device_addr: 0x" << device_addr << dec << endl);
   SW_PRINT("NWL:SBUS:Firmware Build ID %d, device address %d\n", data, device_addr);
   return data;
}

//----------------------------------------
// Set broadcast group
//  sbus_ring_node is the node number in the sbus_ring
//----------------------------------------
void set_sbus_broadcast_grp(int chip_id, int sbus_ring_ms, int sbus_ring_node, int bcast_addr, int bgroup) {
   uint32_t rd_val;
   uint32_t wr_val;
   uint32_t device_addr;

   device_addr = 0xfd; // spico (Table 12) and serdes (Table 67)
   PLOG_MSG( "SBUS: set_sbus_broadcast_grp: is_ms:" << sbus_ring_ms << " bgroup:" << bgroup << " device_addr:0x" << hex << device_addr << " bcast_addr:" << bcast_addr << dec << endl);
   if (bgroup == 0) {
      if (sbus_ring_ms) {
         rd_val = cap_ms_sbus_read(chip_id, sbus_ring_node, device_addr);  
         wr_val = (rd_val & 0xff000) | ((bcast_addr << 16) & 0xff0);
         cap_ms_sbus_write(chip_id, sbus_ring_node, device_addr, wr_val);  // set sbus_ring_node (data_addr) to bgroup
      } else {
         rd_val = cap_pp_sbus_read(chip_id, sbus_ring_node, device_addr);  
         wr_val = (rd_val & 0xff0000) | ((bcast_addr << 4) & 0xff0);
         cap_pp_sbus_write(chip_id, sbus_ring_node, device_addr, wr_val);  // set sbus_ring_node (data_addr) to bgroup
         rd_val = cap_pp_sbus_read(chip_id, sbus_ring_node, device_addr);  
         PLOG_MSG( "SBUS: pp set_sbus_broadcast_grp:" << bgroup << " device_addr:0x" << hex << device_addr << " wr:0x" << wr_val << " rd:0x" << rd_val << dec << endl);
      }
   } else if (bgroup == 1) {
      if (sbus_ring_ms) {
         rd_val = cap_pp_sbus_read(chip_id, sbus_ring_node, device_addr);  
         wr_val = (rd_val & 0x00ff0) | ((bcast_addr << 16) & 0xff000);
         cap_ms_sbus_write(chip_id, sbus_ring_node, device_addr, wr_val);  // set sbus_ring_node (data_addr) to bgroup
      } else {
         rd_val = cap_pp_sbus_read(chip_id, sbus_ring_node, device_addr);  
         wr_val = (rd_val & 0x00ff0) | ((bcast_addr << 16) & 0xff0000);
         cap_pp_sbus_write(chip_id, sbus_ring_node, device_addr, wr_val);  // set sbus_ring_node (data_addr) to bgroup
         rd_val = cap_pp_sbus_read(chip_id, sbus_ring_node, device_addr);  
         PLOG_MSG( "SBUS: pp set_sbus_broadcast_grp:" << bgroup << " device_addr:0x" << hex << device_addr << " wr:0x" << wr_val << " rd:0x" << rd_val << dec << endl);
          
      }
   } else {
      PLOG_ERR("sbus_set_broadcast_grp :: illegal broadcast group : " << bgroup << " 0 or 1 only" << endl);
   } 

   PLOG_MSG("sbus_set_broadcast_grp :: device_addr 0x:" << hex << device_addr << " rd_value:0x" << rd_val << " wr_value:0x" << wr_val << " bcast_addr:0x" << bcast_addr << " sbus_ring_node:0x" << sbus_ring_node << " broadcast group: " << bgroup << dec << endl);
}

void cap_sbus_pp_set_rom_enable(int chip_id, int inst_id, int val) {

  cap_sbus_reg_write((CAP_ADDR_BASE_PP_PP_OFFSET) + CAP_PP_CSR_CFG_PP_SBUS_BYTE_ADDRESS, val);
}


void cap_pp_ld_sbus_master_firmware(int chip_id, int inst_id, const char * rom_file, int backdoor) { 

  if (backdoor == 1) {
    PLOG_MSG("Entering load sbus firmware backdoor " << endl);
    cap_pp_sbus_write(chip_id, 0xfd, 0x1, 0xc0);
    cap_pp_sbus_write(chip_id, 0xfd, 0x1, 0x240);
    cap_pp_sbus_write(chip_id, 0xfd, 0x3, 0x80000000);
#ifdef _CSV_INCLUDED_
    PLOG_MSG("load sbus firmware backdoor " << endl);
#ifndef NX_MS_NWL_TB 
#ifndef NX_MS_TB 
    top_sbus_ld_sbus_rom(inst_id, rom_file);
#endif
#endif
#endif    
    cap_pp_sbus_write(chip_id, 0xfd, 0x1, 0x40);
    cap_pp_sbus_write(chip_id, 0xfd, 0x16, 0xc0000);
    cap_pp_sbus_write(chip_id, 0xfd, 0x1, 0x140);
  } else {
    PLOG_MSG("load sbus firmware frontdoor " << endl);
    cap_sbus_pp_set_rom_enable(chip_id, inst_id, 1);
    SLEEP (10);
    cap_sbus_pp_set_rom_enable(chip_id, inst_id, 0);
  }
  PLOG_MSG("Done loading sbus firmware " << endl);
}

//----------------------------------------
// Issue a spico interrupt read
//----------------------------------------
uint32_t sbus_master_spico_interrupt_wait_done(int chip_id, int sbus_ring_ms, int device_addr, int wait_timeout) {

   int timeout = wait_timeout;
   int error;
   uint32_t data;

   data  = 0;
  PLOG_MSG("KCM:sbus_master_spico_interrupt_wait_done: " << hex << device_addr << " data_addr: 0x" << hex << wait_timeout << dec << endl);
   PLOG_MSG("SBUS: Checking interrupt busy" << endl);
   while (timeout > 0) {
     if (sbus_ring_ms) {
       data = cap_ms_sbus_read(chip_id, device_addr, 8);
     } else {
       data = cap_pp_sbus_read(chip_id, device_addr, 4);
     }
     PLOG_MSG("SBUS: result/status: 0x" << (data >> 16) << " / 0x" <<  (data & 0xffff) << dec << endl);

     // Check if interrupt busy flag is set 
     if (sbus_ring_ms) {
       if ((data & 0x8000) == 0)
         break;
     } else {
       if ((data & 0x10000) == 0) // 0: interrupt_in_prog
         break;
     }

     SLEEP(1000);
     timeout = timeout - 1;
   }

   error = 0;
   if (timeout == 0) {
     PLOG_ERR( "ERROR: Interrupt checking timeout" << endl);
     error = error + 1; // FIXME Suresh
   } else {
     // Read again for since the read is un-triggered, this means the data read when the
     // status changed to "done" may not be valid. So read again to get a valid result data.
     if (sbus_ring_ms) {
        data = cap_ms_sbus_read(chip_id, device_addr, 8);
        data = (data >> 16) & 0xffff;
     } else {
        data = cap_pp_sbus_read(chip_id, device_addr, 4);
        data = data & 0xffff;
     }
   }

   return data;
}

