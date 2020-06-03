
#ifdef ELBA_SW
#include "elb_sw_glue.h"
#endif

#include "elb_top_csr_defines.h"
#include "elb_sbus_api.h"
#include "elb_soc_c_hdr.h"
#include "elb_mm_c_hdr.h"
#include "elb_mc_c_hdr.h"
#include "elb_ms_c_hdr.h"
#include <stdio.h>

char glb_log_buf[10240*8];

#ifdef ELBA_SW
#define pen_print(...) do {} while (0)
#else
#define pen_print pen_c_printf_msg
#endif

#ifndef SBUS_INFO
#define SBUS_INFO(...) { \
    sprintf(glb_log_buf,##__VA_ARGS__); \
    pen_print(glb_log_buf); \
}
#endif

#ifndef SBUS_MSG
#define SBUS_MSG(...) { \
    sprintf(glb_log_buf,##__VA_ARGS__); \
    pen_print(glb_log_buf); \
}
#endif

#ifndef SBUS_ERR
#define SBUS_ERR(...) { \
    sprintf(glb_log_buf,##__VA_ARGS__); \
    pen_print(glb_log_buf); \
}
#endif



void elb_sbus_reg_write(uint64_t addr, uint32_t wr_data) {
#ifdef ELBA_SW
  elb_sw_writereg(addr, wr_data);
#else
  pen_c_csr_write(addr, wr_data, 1, 1);
#endif
}

uint32_t elb_sbus_reg_read(uint64_t addr) {
#ifdef ELBA_SW
  return elb_sw_readreg(addr);
#else
  return pen_c_csr_read(addr, 1, 1);
#endif
}

#ifndef ELBA_SW
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
#define SBUS_7NM_IMEM_WORDS 8

#define SBUS_ARM  0
#define SBUS_PCIE 1
#define SBUS_CORE 2
#define SBUS_AOD  3

static const uint32_t elb_sbus_indir_addr[] = {
       ELB_SOC_CSR_CFG_SBUS_ARM_INDIR_BYTE_ADDRESS,
       ELB_SOC_CSR_CFG_SBUS_PCIE_INDIR_BYTE_ADDRESS,
       ELB_SOC_CSR_CFG_SBUS_CORE_INDIR_BYTE_ADDRESS,
       ELB_MM_CSR_CFG_SBUS_CORE_INDIR_BYTE_ADDRESS
   };

static const uint32_t elb_sbus_dhs_addr[] = {
       ELB_SOC_CSR_DHS_SBUS_ARM_INDIR_BYTE_ADDRESS,
       ELB_SOC_CSR_DHS_SBUS_PCIE_INDIR_BYTE_ADDRESS,
       ELB_SOC_CSR_DHS_SBUS_CORE_INDIR_BYTE_ADDRESS,
       ELB_MM_CSR_DHS_SBUS_CORE_INDIR_BYTE_ADDRESS
   };

static const uint32_t elb_sbus_base[] = {
       ELB_ADDR_BASE_MS_SOC_OFFSET,
       ELB_ADDR_BASE_MS_SOC_OFFSET,
       ELB_ADDR_BASE_MS_SOC_OFFSET,
       ELB_ADDR_BASE_MM_MM_OFFSET
   };

void elb_sbus_write(int chip_id, int sbus_id, int rcvr_addr, int data_addr, unsigned int data) {

  uint32_t val;


  val = rcvr_addr |
    	(data_addr << ELB_SOC_CSR_CFG_SBUS_PCIE_INDIR_DATA_ADDR_LSB) |
 	(ELB_SBUS_SBUS_WR << ELB_SOC_CSR_CFG_SBUS_PCIE_INDIR_COMMAND_LSB) ;

  elb_sbus_reg_write((elb_sbus_base[sbus_id]) + elb_sbus_indir_addr[sbus_id], val);
  elb_sbus_reg_write((elb_sbus_base[sbus_id]) + elb_sbus_dhs_addr[sbus_id], data);
}

unsigned int elb_sbus_read(int chip_id, int sbus_id, int rcvr_addr, int data_addr) {
  int rd_data;
  uint32_t val;

  val = rcvr_addr |
     	(data_addr << ELB_SOC_CSR_CFG_SBUS_PCIE_INDIR_DATA_ADDR_LSB) |
	(ELB_SBUS_SBUS_RD << ELB_SOC_CSR_CFG_SBUS_PCIE_INDIR_COMMAND_LSB) ;

  elb_sbus_reg_write((elb_sbus_base[sbus_id]) + elb_sbus_indir_addr[sbus_id], val);
  rd_data = elb_sbus_reg_read((elb_sbus_base[sbus_id]) + elb_sbus_dhs_addr[sbus_id]);

  return rd_data;
}

//////////////////////////////////////////////////////////////////////////////
void elb_sbus_reset(int chip_id, int sbus_id, int rcvr_addr) {

  uint32_t val;
  uint32_t data_addr=0;

  val = rcvr_addr |
     	(data_addr << ELB_SOC_CSR_CFG_SBUS_PCIE_INDIR_DATA_ADDR_LSB) |
	(ELB_SBUS_SBUS_RST << ELB_SOC_CSR_CFG_SBUS_PCIE_INDIR_COMMAND_LSB) ;

  elb_sbus_reg_write((elb_sbus_base[sbus_id]) + elb_sbus_indir_addr[sbus_id], val);
  elb_sbus_reg_write((elb_sbus_base[sbus_id]) + elb_sbus_dhs_addr[sbus_id], 0);
}

void elb_arm_sbus_write(int chip_id, int rcvr_addr, int data_addr, unsigned int data) {
   elb_sbus_write(chip_id, SBUS_ARM, rcvr_addr, data_addr, data);
}

unsigned int elb_arm_sbus_read(int chip_id, int rcvr_addr, int data_addr) {
   return elb_sbus_read(chip_id, SBUS_ARM, rcvr_addr, data_addr);
}

void elb_arm_sbus_reset(int chip_id, int rcvr_addr) {
   elb_sbus_reset(chip_id, SBUS_ARM, rcvr_addr);
}

void elb_pp_sbus_write(int chip_id, int rcvr_addr, int data_addr, unsigned int data) {
   elb_sbus_write(chip_id, SBUS_PCIE, rcvr_addr, data_addr, data);
}

unsigned int elb_pp_sbus_read(int chip_id, int rcvr_addr, int data_addr) {
   return elb_sbus_read(chip_id, SBUS_PCIE, rcvr_addr, data_addr);
}

void elb_pp_sbus_reset(int chip_id, int rcvr_addr) {
   elb_sbus_reset(chip_id, SBUS_PCIE, rcvr_addr);
}

void elb_core_sbus_write(int chip_id, int rcvr_addr, int data_addr, unsigned int data) {
   elb_sbus_write(chip_id, SBUS_CORE, rcvr_addr, data_addr, data);
}

unsigned int elb_core_sbus_read(int chip_id, int rcvr_addr, int data_addr) {
   return elb_sbus_read(chip_id, SBUS_CORE, rcvr_addr, data_addr);
}

void elb_core_sbus_reset(int chip_id, int rcvr_addr) {
   elb_sbus_reset(chip_id, SBUS_CORE, rcvr_addr);
}

void elb_aod_sbus_write(int chip_id, int rcvr_addr, int data_addr, unsigned int data) {
   SBUS_INFO("elb_aod_sbus_write, rcvr_addr 0x%x, data_addr 0x%x, data 0x%x\n", rcvr_addr, data_addr, data);
   elb_sbus_write(chip_id, SBUS_AOD, rcvr_addr, data_addr, data);
}

unsigned int elb_aod_sbus_read(int chip_id, int rcvr_addr, int data_addr) {
   SBUS_INFO("elb_aod_sbus_read, rcvr_addr 0x%x, data_addr 0x%x\n", rcvr_addr, data_addr);
   return elb_sbus_read(chip_id, SBUS_AOD, rcvr_addr, data_addr);
}

void elb_aod_sbus_reset(int chip_id, int rcvr_addr) {
   elb_sbus_reset(chip_id, SBUS_AOD, rcvr_addr);
}

void elb_ms_sbus_write(int chip_id, int rcvr_addr, int data_addr, unsigned int data) {

  uint32_t val;

  val = rcvr_addr |
        (data_addr << ELB_SOC_CSR_CFG_SBUS_PCIE_INDIR_DATA_ADDR_LSB) |
        (ELB_SBUS_SBUS_WR << ELB_SOC_CSR_CFG_SBUS_PCIE_INDIR_COMMAND_LSB) ;
  elb_sbus_reg_write((ELB_ADDR_BASE_MS_SOC_OFFSET) + ELB_SOC_CSR_CFG_SBUS_PCIE_INDIR_BYTE_ADDRESS, val);
  elb_sbus_reg_write((ELB_ADDR_BASE_MS_SOC_OFFSET) + ELB_SOC_CSR_DHS_SBUS_PCIE_INDIR_BYTE_ADDRESS, data);
  /*
  */
}

unsigned int elb_ms_sbus_read(int chip_id, int rcvr_addr, int data_addr) {
  //return 0xdead;

  int rd_data;
  uint32_t val;

  val = rcvr_addr |
  	(data_addr << ELB_SOC_CSR_CFG_SBUS_PCIE_INDIR_DATA_ADDR_LSB) |
 	(ELB_SBUS_SBUS_RD << ELB_SOC_CSR_CFG_SBUS_PCIE_INDIR_COMMAND_LSB) ;
  elb_sbus_reg_write((ELB_ADDR_BASE_MS_SOC_OFFSET) + ELB_SOC_CSR_CFG_SBUS_PCIE_INDIR_BYTE_ADDRESS, val);
  rd_data = elb_sbus_reg_read((ELB_ADDR_BASE_MS_SOC_OFFSET) + ELB_SOC_CSR_DHS_SBUS_PCIE_INDIR_BYTE_ADDRESS);

  return rd_data;
  /*
  */
}

//////////////////////////////////////////////////////////////////////////////
void elb_ms_sbus_reset(int chip_id, int rcvr_addr) {

  uint32_t val;
  uint32_t data_addr=0;

  val = rcvr_addr |
    	(data_addr << ELB_SOC_CSR_CFG_SBUS_PCIE_INDIR_DATA_ADDR_LSB) |
	(ELB_SBUS_SBUS_RST << ELB_SOC_CSR_CFG_SBUS_PCIE_INDIR_COMMAND_LSB) ;
  elb_sbus_reg_write((ELB_ADDR_BASE_MS_SOC_OFFSET) + ELB_SOC_CSR_CFG_SBUS_PCIE_INDIR_BYTE_ADDRESS, val);
  elb_sbus_reg_write((ELB_ADDR_BASE_MS_SOC_OFFSET) + ELB_SOC_CSR_DHS_SBUS_PCIE_INDIR_BYTE_ADDRESS, 0);
  /*
  */
}


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
//#ifndef _ZEBU_ // exclude from zebu

//void upload_sbus_master_firmware(int chip_id, int sbus_ring_ms, int device_addr, string fila_name, int wait_time) {

//}

void upload_sbus_master_firmware(int chip_id, int sbus_ring_ms, int device_addr, const void * fila_name) {
    upload_sbus_master_firmware_w_wait(chip_id, sbus_ring_ms, device_addr, fila_name, 100);
}

void upload_sbus_master_firmware_w_wait(int chip_id, int sbus_ring_ms, int device_addr, const void* rom_info, int wait_time) {

   SBUS_INFO("Sbus Upload firmware: %s device_addr: 0x%x",(char*) rom_info, device_addr);
   SBUS_MSG("NWL/PCIE:SBUS:Upload SBUS \n");
 
   void *ctx;
   ctx = romfile_open(rom_info);
 
   if (ctx == NULL) {
#ifndef ELBA_SW
      SBUS_INFO("Sbus Upload firmware: %s device_addr: 0x%x\n",(char*) rom_info, device_addr);
      SBUS_ERR("upload_sbus_master_firmware :: Exiting function...Unable to open file : %s\n", (char *)rom_info);
#endif
      return;
   }
 
   // 12 bits
   unsigned int imem_data0, imem_data1, imem_data2;        // data word read from file
   unsigned int data[SBUS_7NM_IMEM_WORDS];
   int valid[SBUS_7NM_IMEM_WORDS];
 
   if (sbus_ring_ms == 1) {
     // SBusMaster16_Spec: Table 10 
     elb_ms_sbus_write(chip_id, device_addr, 0x1, 0xc0);  // Place SPICO into Reset and Enable off
     elb_ms_sbus_write(chip_id, device_addr, 0x1, 0x240); // Remove Reset, Enable off, IMEM_CNTL_EN on
     elb_ms_sbus_write(chip_id, device_addr, 0x3, 0x80000000); // Set starting IMEM address for burst download
   } else if (sbus_ring_ms == 2) {
     // SerDes16_Spec: Table 67
     elb_aod_sbus_write(chip_id, device_addr, 0x7, 0x11);  // Place SerDes in Reset and disable SPICO
     elb_aod_sbus_write(chip_id, device_addr, 0x7, 0x10); // Remove SerDes Reset
     elb_aod_sbus_write(chip_id, device_addr, 0x0, 0x40000000); // Set starting IMEM override
     //elb_pp_sbus_write(chip_id, device_addr, 0x0, 0xc0000000); // Set starting IMEM override
   } else {
     // SerDes16_Spec: Table 67
     elb_pp_sbus_write(chip_id, device_addr, 0x7, 0x11);  // Place SerDes in Reset and disable SPICO
     elb_pp_sbus_write(chip_id, device_addr, 0x7, 0x10); // Remove SerDes Reset
     elb_pp_sbus_write(chip_id, device_addr, 0x0, 0x40000000); // Set starting IMEM override
     //elb_pp_sbus_write(chip_id, device_addr, 0x0, 0xc0000000); // Set starting IMEM override
   }

#ifndef _DISABLE_SKNOBS_
   uint8_t load_frontdoor_ms   = sknobs_get_value((char*)"test/sbus/load_rom_file_frontdoor",1) && (sbus_ring_ms == 1);
   uint8_t load_frontdoor_pcie = sknobs_get_value((char*)"test/sbus/load_pcie_rom_file_frontdoor",0) && (sbus_ring_ms == 0);
   uint8_t load_frontdoor_aod = sknobs_get_value((char*)"test/sbus/load_aod_rom_file_frontdoor",0) && (sbus_ring_ms == 2);
#else 
   uint8_t load_frontdoor_ms = 1;
   uint8_t load_frontdoor_pcie = 1;
   uint8_t load_frontdoor_aod = 1;
#endif

   uint8_t download_done = 0;
   unsigned line_no = 0;
   if (load_frontdoor_ms || load_frontdoor_pcie || load_frontdoor_aod) {   
     // Burst Load the ROM code
     do {

       for(int i =0; i< SBUS_7NM_IMEM_WORDS; i++) {
           if(!download_done) {
               valid[i] = romfile_read(ctx, &data[i]);
               line_no++;
           } else {
               valid[i] = 0;
               data[i] = 0;
           }

           if(valid[i] <= 0) { 
               data[i] = 0; 
               if(!download_done) SBUS_INFO("SBUS: file reached EOF : total line no %d\n", line_no);
               download_done = 1;
               //break; 
           }
       }
       
       if(valid[0] > 0) {
           if((line_no % (8*SBUS_7NM_IMEM_WORDS)) == 0) { SBUS_INFO("SBUS: file download in progress current line_no %d\n",line_no); }
           imem_data0 = data[0] | (data[1] << 12) | ((data[2] & 0xff) << 24);
           imem_data1 = ((data[2] >> 8) & 0xf) | (data[3] << 4) | (data[4] << 16) |  ((data[5] & 0xf) << 28);
           imem_data2 = ((data[5] >> 4) & 0xff) | (data[6] << 8) | ((data[7] & 0xfff) << 20);

           if(sbus_ring_ms == 1) {
               elb_ms_sbus_write(chip_id, device_addr, 0x14, imem_data0);
               elb_ms_sbus_write(chip_id, device_addr, 0x14, imem_data1);
               elb_ms_sbus_write(chip_id, device_addr, 0x14, imem_data2);
           } else if (sbus_ring_ms == 2) {
               elb_aod_sbus_write(chip_id, device_addr, 0xa, imem_data0);
               elb_aod_sbus_write(chip_id, device_addr, 0xa, imem_data1);
               elb_aod_sbus_write(chip_id, device_addr, 0xa, imem_data2);
           } else {
               elb_pp_sbus_write(chip_id, device_addr, 0xa, imem_data0);
               elb_pp_sbus_write(chip_id, device_addr, 0xa, imem_data1);
               elb_pp_sbus_write(chip_id, device_addr, 0xa, imem_data2);
           }
       }

     } while(!download_done);

     romfile_close(ctx);

   }
   else {
       SBUS_ERR("backdoor firmware load only supported for HBM for now\n");
   }

   if (sbus_ring_ms == 1) {
     uint32_t data;
    if(load_frontdoor_ms) {
        elb_ms_sbus_write(chip_id, device_addr, 0x14, 0xc0000000); // Pad with 0's
        elb_ms_sbus_write(chip_id, device_addr, 0x14, 0xc0000000); // Pad with 0's
        elb_ms_sbus_write(chip_id, device_addr, 0x14, 0xc0000000); // Pad with 0's
     }
     // SBusMaster16_Spec: Table 10 
     elb_ms_sbus_write(0, device_addr, 0x1, 0x40);  // Place SPICO into Reset and Enable off
     elb_ms_sbus_write(0, device_addr, 0x1, 0x140); // Remove Reset, Enable off, IMEM_CNTL_EN on	  

     SBUS_INFO("MS read to flush all writes, this will cause firmware download completed\n");
     data = elb_ms_sbus_read(chip_id, 1, 0xff);
     SBUS_MSG("NWL:SBUS:Done with load .. flushed \n");
     (void)data;
   } else if (sbus_ring_ms == 2) {
     //// SerDes16_Spec: Table 67
     //elb_aod_sbus_write(chip_id, device_addr, 0xa, 0x0); // Pad with 0's
     //elb_aod_sbus_write(chip_id, device_addr, 0xa, 0x0); // Pad with 0's
     //elb_aod_sbus_write(chip_id, device_addr, 0xa, 0x0); // Pad with 0's

     elb_aod_sbus_write(chip_id, device_addr, 0x0, 0x0); // IMEM override off
     elb_aod_sbus_write(chip_id, device_addr, 0xb, 0xc0000); // Turn ECC on
     elb_aod_sbus_write(chip_id, device_addr, 0x7, 0x2); // Turn SPICO Enable on
     elb_aod_sbus_write(chip_id, device_addr, 0x8, 0x0); // Enable core and hw interrupts

     // do a read to flush all the write
     SBUS_INFO("AOD read to flush all writes, this will cause firmware download completed\n");
     elb_aod_sbus_read(chip_id, device_addr, 0xff);
     SBUS_MSG("AOD:SBUS:Done with load .. flushed \n");
   } else {
     //// SerDes16_Spec: Table 67
     //elb_pp_sbus_write(chip_id, device_addr, 0xa, 0xc0000000); // Pad with 0's
     //elb_pp_sbus_write(chip_id, device_addr, 0xa, 0xc0000000); // Pad with 0's
     //elb_pp_sbus_write(chip_id, device_addr, 0xa, 0xc0000000); // Pad with 0's

     elb_pp_sbus_write(chip_id, device_addr, 0x0, 0x0); // IMEM override off
     elb_pp_sbus_write(chip_id, device_addr, 0xb, 0xc0000); // Turn ECC on
     elb_pp_sbus_write(chip_id, device_addr, 0x7, 0x2); // Turn SPICO Enable on
     elb_pp_sbus_write(chip_id, device_addr, 0x8, 0x0); // Enable core and hw interrupts

     // do a read to flush all the write
     SBUS_INFO("PP read to flush all writes, this will cause firmware download completed\n");
     elb_pp_sbus_read(chip_id, 1, 0xff);
     SBUS_MSG("PCIE:SBUS:Done with load .. flushed \n");
   }

 
   // Give Spico time to get through init code
   SBUS_INFO( "SBUS: Waiting for spico to initialize\n");
 
   SBUS_MSG("NWL/PCIE:SBUS:Is sleep enough? \n");
#ifndef ELBA_SW
   SLEEP(1000);
#else
   SLEEP(wait_time);
#endif
   // 1000 SBUS_MASTER_PATH.spico_clk
   // $display( "DEBUG: %0t Waiting for spico timout:%0d, state:%0d", $time, timeout, `SBUS_MASTER_PATH.spico.spico_main.spico_state );
   // (`SBUS_MASTER_PATH.spico.spico_main.spico_state == 5'h1f || `SBUS_MASTER_PATH.spico.spico_main.spico_state == 5'h12) 
   //   if (`SBUS_MASTER_PATH.spico.spico_main.spico_state != 'h12 ) 
   //     $display( "ERROR: %0t spico processor failured to initialize", $time );
}

//#endif // SWIG

//----------------------------------------
// Issue a spico interrupt and wait for it to complete
//----------------------------------------
uint32_t sbus_master_spico_interrupt(int chip_id, int sbus_ring_ms, int device_addr, int interrupt_code, int interrupt_value, uint32_t data, int no_busy_check) {
    return sbus_master_spico_interrupt_w_wait(chip_id, sbus_ring_ms, device_addr, interrupt_code, interrupt_value, data, no_busy_check, 100);
}

uint32_t sbus_master_spico_interrupt_w_wait(int chip_id, int sbus_ring_ms, int device_addr, int interrupt_code, int interrupt_value, uint32_t data, int no_busy_check, int wait_timeout) {
   int timeout;
   int error;

  SBUS_INFO("KCM:sbus_master_spico_interrupt: 0x%x data_addr 0x%x\n", device_addr, interrupt_code);
   // Assert Interrupt
   if (sbus_ring_ms == 1) {
      // Set spico interrupt code and value
      // refer to SBusMaster16_Spec_11 table 12
      elb_ms_sbus_write(chip_id, device_addr, 2, ((interrupt_value << 16) | (interrupt_code & 0xffff)));

      data = elb_ms_sbus_read(chip_id, device_addr, 7);
      elb_ms_sbus_write(chip_id, device_addr, 7, (data | 1));

      // Lower Interrupt
      elb_ms_sbus_write(chip_id, device_addr, 7, (data & 0xfffffffe));
   } else if (sbus_ring_ms == 2) {
      // refer to SerDes16_Spec table 67
      elb_aod_sbus_write(chip_id, device_addr, 3, ((interrupt_code << 16) | (interrupt_value & 0xffff)));

      data = elb_aod_sbus_read(chip_id, device_addr, 4);

      // Lower Interrupt
      ///elb_pp_sbus_write(chip_id, device_addr, 7, (data & 0xfffffffe));
   } else {
      // refer to SerDes16_Spec table 67
      elb_pp_sbus_write(chip_id, device_addr, 3, ((interrupt_code << 16) | (interrupt_value & 0xffff)));

      data = elb_pp_sbus_read(chip_id, device_addr, 4);

      // Lower Interrupt
      ///elb_pp_sbus_write(chip_id, device_addr, 7, (data & 0xfffffffe));
   }

   if (no_busy_check == 0) {
     // Wait for interrupt to complete
     timeout = wait_timeout;
     SBUS_INFO("SBUS: Waiting for interrupt 0x%x to complete\n", (interrupt_code  & 0xffff));
     while (timeout > 0) {
       if (sbus_ring_ms == 1) {
         data = elb_ms_sbus_read(chip_id, device_addr, 8);
       } else if (sbus_ring_ms == 2) {
         data = elb_aod_sbus_read(chip_id, device_addr, 4);
       } else {
         data = elb_pp_sbus_read(chip_id, device_addr, 4);
       }
       SBUS_INFO("SBUS: Interrupt 0x%x result/status 0x%x/ 0x%x\n", (interrupt_code & 0xffff) , (data >> 16) , (data & 0xffff));
  
       // Check if interrupt busy flag is set 
       if (sbus_ring_ms == 1) {
         if ((data & 0x8000) == 0)
           break;
       } else if (sbus_ring_ms == 2) {
         if ((data & 0x10000) == 0) // 0: interrupt_in_prog
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
       SBUS_ERR( "ERROR: Interrupt 0x%x timeout while waiting for completion.\n", (interrupt_code  & 0xffff));
       error = 1;
     } else {
       // Read again for since the read is un-triggered, this means the data read when the
       // status changed to "done" may not be valid. So read again to get a valid result data.
       if (sbus_ring_ms == 1) {
          data = elb_ms_sbus_read(chip_id, device_addr, 8);
          data = (data >> 16) & 0xffff;
       } else if (sbus_ring_ms == 2) {
          data = elb_aod_sbus_read(chip_id, device_addr, 4);
          data = data & 0xffff;
       } else {
          data = elb_pp_sbus_read(chip_id, device_addr, 4);
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
   elb_pp_sbus_write(chip_id, device_addr, 3, interrupt_code_data);

   data = elb_pp_sbus_read(chip_id, device_addr, 4);

   // Wait for interrupt to complete
   timeout = 100;
   PLOG_MSG("SBUS: Waiting for interrupt code/data 0x" << hex << interrupt_code_data << " to complete" << endl);
   while (timeout > 0) {
     data = elb_pp_sbus_read(chip_id, device_addr, 4);
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
        data = elb_ms_sbus_read(chip_id, device_addr, 8);
     } else {
        data = elb_pp_sbus_read(chip_id, device_addr, 8);
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

   data = sbus_master_spico_interrupt_w_wait(0, sbus_ring_ms, device_addr, 0x0, 0, 0, 0, 100); // FIXME Suresh data
   SBUS_INFO( "SBUS: Spico Firmware Revision: 0x%d device_addr 0x%x\n", data , device_addr);
   SBUS_MSG("NWL:SBUS:Firmware Revision %d, device address %d\n", data, device_addr);
   return data;
}


//----------------------------------------
// Prints the firmware build ID.  Firmware mus be uploaded first. 
//----------------------------------------
uint32_t report_firmware_build_id(int sbus_ring_ms, int device_addr) {
   uint32_t data;

   data = sbus_master_spico_interrupt_w_wait(0, sbus_ring_ms, device_addr, 0x1, 0, 0, 0, 100); // FIXME Suresh data
   SBUS_INFO( "SBUS: Spico Firmware Build ID:: 0x%x device_addr: 0x%x\n", data, device_addr);
   SBUS_MSG("NWL:SBUS:Firmware Build ID %d, device address %d\n", data, device_addr);
   return data;
}

//----------------------------------------
// Set broadcast group
//  sbus_ring_node is the node number in the sbus_ring
//----------------------------------------
void set_sbus_broadcast_grp(int chip_id, int sbus_ring_ms, int sbus_ring_node, int bcast_addr, int bgroup) {
   uint32_t rd_val = 0xdeadbeef;
   uint32_t wr_val = 0xdeadbeef;
   uint32_t device_addr;

   device_addr = 0xfd; // spico (Table 12) and serdes (Table 67)
   SBUS_INFO( "SBUS: set_sbus_broadcast_grp: is_ms:%d bgroup:%d device_addr:0x%x bcast_addr:0x%x\n", sbus_ring_ms, bgroup, device_addr, bcast_addr);
   if (bgroup == 0) {
      if (sbus_ring_ms == 1) {
         rd_val = elb_ms_sbus_read(chip_id, sbus_ring_node, device_addr);  
         wr_val = (rd_val & 0xff000) | ((bcast_addr << 16) & 0xff0);
         elb_ms_sbus_write(chip_id, sbus_ring_node, device_addr, wr_val);  // set sbus_ring_node (data_addr) to bgroup
      } else if (sbus_ring_ms == 2) {
         rd_val = elb_aod_sbus_read(chip_id, sbus_ring_node, device_addr);  
         wr_val = (rd_val & 0xff0000) | ((bcast_addr << 4) & 0xff0);
         elb_pp_sbus_write(chip_id, sbus_ring_node, device_addr, wr_val);  // set sbus_ring_node (data_addr) to bgroup
         rd_val = elb_aod_sbus_read(chip_id, sbus_ring_node, device_addr);  
         SBUS_INFO( "SBUS: aod set_sbus_broadcast_grp: %d  device_addr:0x%x wr:0x%x rd:0x%x\n",bgroup, device_addr, wr_val, rd_val);
      } else {
         rd_val = elb_pp_sbus_read(chip_id, sbus_ring_node, device_addr);  
         wr_val = (rd_val & 0xff0000) | ((bcast_addr << 4) & 0xff0);
         elb_pp_sbus_write(chip_id, sbus_ring_node, device_addr, wr_val);  // set sbus_ring_node (data_addr) to bgroup
         rd_val = elb_pp_sbus_read(chip_id, sbus_ring_node, device_addr);  
         SBUS_INFO( "SBUS: pp set_sbus_broadcast_grp: %d  device_addr:0x%x wr:0x%x rd:0x%x\n", bgroup, device_addr, wr_val, rd_val);
      }
   } else if (bgroup == 1) {
      if (sbus_ring_ms == 1) {
         rd_val = elb_pp_sbus_read(chip_id, sbus_ring_node, device_addr);  
         wr_val = (rd_val & 0x00ff0) | ((bcast_addr << 16) & 0xff000);
         elb_ms_sbus_write(chip_id, sbus_ring_node, device_addr, wr_val);  // set sbus_ring_node (data_addr) to bgroup
      } else if (sbus_ring_ms == 2) {
         rd_val = elb_aod_sbus_read(chip_id, sbus_ring_node, device_addr);  
         wr_val = (rd_val & 0x00ff0) | ((bcast_addr << 16) & 0xff0000);
         elb_pp_sbus_write(chip_id, sbus_ring_node, device_addr, wr_val);  // set sbus_ring_node (data_addr) to bgroup
         rd_val = elb_aod_sbus_read(chip_id, sbus_ring_node, device_addr);  
         SBUS_INFO( "SBUS: aod set_sbus_broadcast_grp: %d device_addr:0x%x wr:0x%x rd:0x%x\n", bgroup, device_addr,wr_val, rd_val);
      } else {
         rd_val = elb_pp_sbus_read(chip_id, sbus_ring_node, device_addr);  
         wr_val = (rd_val & 0x00ff0) | ((bcast_addr << 16) & 0xff0000);
         elb_pp_sbus_write(chip_id, sbus_ring_node, device_addr, wr_val);  // set sbus_ring_node (data_addr) to bgroup
         rd_val = elb_pp_sbus_read(chip_id, sbus_ring_node, device_addr);  
         SBUS_INFO( "SBUS: pp set_sbus_broadcast_grp: %d device_addr:0x%x wr:0x%x rd:0x%x\n", bgroup, device_addr,wr_val, rd_val);
      }
   } else {
      SBUS_ERR("sbus_set_broadcast_grp :: illegal broadcast group : %d 0 or 1 only\n", bgroup);
   } 

   SBUS_INFO("sbus_set_broadcast_grp :: device_addr 0x%x rd_value:0x%x wr_value:0x%x bcast_addr:0x%x sbus_ring_node:0x%x broadcast group: 0x%x\n", device_addr, rd_val, wr_val, bcast_addr, sbus_ring_node, bgroup);
}

void elb_sbus_pp_set_rom_enable(int chip_id, int inst_id, int val) {
  SBUS_ERR("FIXME: elb_pp_sbus_write not implemented\n");
  /*
  elb_sbus_reg_write((ELB_ADDR_BASE_PP_PP_OFFSET) + ELB_PP_CSR_CFG_PP_SBUS_BYTE_ADDRESS, val);
  */
}


void elb_pp_ld_sbus_master_firmware(int chip_id, int inst_id, const char * rom_file, int backdoor) { 
    SBUS_ERR("why am I here? elb_pp_ld_sbus_master_firmware");
//
//  if (backdoor == 1) {
//    SBUS_INFO("Entering load sbus firmware backdoor \n");
//    elb_pp_sbus_write(chip_id, 0xfd, 0x1, 0xc0);
//    elb_pp_sbus_write(chip_id, 0xfd, 0x1, 0x240);
//    elb_pp_sbus_write(chip_id, 0xfd, 0x3, 0x80000000);
//#ifdef _CSV_INCLUDED_
//    SBUS_INFO("load sbus firmware backdoor \n");
//#ifndef NX_MS_NWL_TB 
//#ifndef NX_MS_TB 
//    top_sbus_ld_sbus_rom(inst_id, rom_file);
//#endif
//#endif
//#endif    
//    elb_pp_sbus_write(chip_id, 0xfd, 0x1, 0x40);
//    elb_pp_sbus_write(chip_id, 0xfd, 0x16, 0xc0000);
//    elb_pp_sbus_write(chip_id, 0xfd, 0x1, 0x140);
//  } else {
//    SBUS_INFO("load sbus firmware frontdoor \n");
//    elb_sbus_pp_set_rom_enable(chip_id, inst_id, 1);
//    SLEEP (10);
//    elb_sbus_pp_set_rom_enable(chip_id, inst_id, 0);
//  }
//  SBUS_INFO("Done loading sbus firmware \n");
}

//----------------------------------------
// Issue a spico interrupt read
//----------------------------------------
uint32_t sbus_master_spico_interrupt_wait_done(int chip_id, int sbus_ring_ms, int device_addr, int wait_timeout) {

   int timeout = wait_timeout;
   int error;
   uint32_t data;

   data  = 0;
  SBUS_INFO("KCM:sbus_master_spico_interrupt_wait_done: 0x%x data_addr 0x%x\n",device_addr, wait_timeout);
   SBUS_INFO("SBUS: Checking interrupt busy\n");
   while (timeout > 0) {
     if (sbus_ring_ms == 1) {
       data = elb_ms_sbus_read(chip_id, device_addr, 8);
     } else if (sbus_ring_ms == 2) {
       data = elb_aod_sbus_read(chip_id, device_addr, 4);
     } else {
       data = elb_pp_sbus_read(chip_id, device_addr, 4);
     }
     SBUS_INFO("SBUS: result/status: 0x%x/0x%x\n",  (data >> 16), (data & 0xffff));

     // Check if interrupt busy flag is set 
     if (sbus_ring_ms == 1) {
       if ((data & 0x8000) == 0)
         break;
     } else if (sbus_ring_ms == 2) {
       if ((data & 0x10000) == 0) // 0: interrupt_in_prog
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
     SBUS_ERR( "ERROR: Interrupt checking timeout\n");
     error = error + 1; // FIXME Suresh
   } else {
     // Read again for since the read is un-triggered, this means the data read when the
     // status changed to "done" may not be valid. So read again to get a valid result data.
     if (sbus_ring_ms == 1) {
        data = elb_ms_sbus_read(chip_id, device_addr, 8);
        data = (data >> 16) & 0xffff;
     } else if (sbus_ring_ms == 2) {
        data = elb_aod_sbus_read(chip_id, device_addr, 4);
        data = data & 0xffff;
     } else {
        data = elb_pp_sbus_read(chip_id, device_addr, 4);
        data = data & 0xffff;
     }
   }

   return data;
}

