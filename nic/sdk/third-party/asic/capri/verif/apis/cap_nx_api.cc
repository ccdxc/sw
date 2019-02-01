#ifdef CAPRI_SW
#include "cap_sw_glue.h"
#endif
#include "cap_nx_api.h"
#ifndef CAPRI_SW
#include "msg_man.h"
#endif

#ifndef SWPRINTF
#define SW_PRINT printf
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)       (sizeof (a) / sizeof ((a)[0]))
#endif

#define RBM_BRIDGE_(x)  (CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_##x##_BYTE_ADDRESS)
#define RBM_AGENT_(x)   (CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_##x##_BYTE_ADDRESS)

static void cap_nx_block_write(uint32_t chip, uint64_t addr, int size, uint32_t *data_in , bool no_zero_time, uint32_t flags)
{
#ifndef CAPRI_SW
   vector<uint32_t> data;
   data.resize(1);
   data[0] = data_in[0];
   cpu::access()->block_write(chip, addr, size, data, no_zero_time, flags);
#else 
   cap_sw_writereg(addr, *data_in);
#endif
}

#ifndef CAPRI_SW
static uint32_t cap_nx_block_read(uint32_t chip, uint64_t addr, int size, bool no_zero_time, uint32_t flags=secure_acc_e)
#else
static uint32_t cap_nx_block_read(uint32_t chip, uint64_t addr, int size, bool no_zero_time, uint32_t flags)
#endif
{
#ifndef CAPRI_SW
   vector<uint32_t> rd_data;
   rd_data.resize(1);
   rd_data =  cpu::access()->block_read(chip, addr, size, no_zero_time, flags); 
   return rd_data[0];
#else
   return cap_sw_readreg(addr);
#endif
}

void cap_nx_write_regs(const uint32_t *rtab, int nelems, uint32_t data)
{
    while (nelems--) {
        cap_nx_block_write(0, *rtab++, 1, &data, true, 1);
    }
}

void cap_nx_enable_rbm_access(int chip_id, int inst_id) {

   static const uint32_t ms_s_rbm[] = {
       RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_RBM_0),
       RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_RBM_0),
       RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_RBM_0),
   };

   cap_nx_write_regs(ms_s_rbm, ARRAY_SIZE(ms_s_rbm), 0xffe00000);
}

// DAC2
void cap_nx_soft_reset_sw(int chip_id, int inst_id, int cache_enable, int pgm_coh, int cache_override, int cache_override_val, int clr_modify_on_masters, int setup_event_counters, int nwl_hash_mode) {

//vector<uint32_t> data;
uint32_t data[2];
//vector<uint32_t> rd_data;

  SW_PRINT("NOC:NOC Init Started\n");
PLOG_API_MSG("NX", "Inside NX_INIT raw\n");
PLOG_API_MSG("NX", "Dumping Sknob Vars\n");
//sknobs_dump();


   //data.resize(1); 
   //rd_data.resize(1); 


/*
AdMask (NC)
NC0 uuuuuuuuuuuuuuuuuuuuuuuu1111_1111_1000_0000_0000_0000_0000_0000_0001_0000 => 0xff_80000010 === Change to => 0xff_8000_0000
NC1 uuuuuuuuuuuuuuuuuuuuuuuu1111_1111_0000_0000_0000_0000_0000_0000_0001_0000 => 0xff_00000010 === Change to => 0xff_0000_0000
NC2 uuuuuuuuuuuuuuuuuuuuuuuu1111_1110_0000_0000_0000_0000_0000_0000_0001_0000 => 0xfe_00000010 === Change to => 0xfe_0000_0000
*/

   static const uint32_t hbm_nc0[] = {
       RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC0_0),
       RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC0_0),
       RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC0_0),
       RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC0_0),
       RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC0_0),
       RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC0_0),
       RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC0_0),
   };
   if (pgm_coh == 0) {
   cap_nx_write_regs(hbm_nc0, ARRAY_SIZE(hbm_nc0), 0x80000000);
   }

   static const uint32_t hbm_nc1[] = {
       RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC1_0),
       RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC1_0),
       RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC1_0),
       RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC1_0),
       RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC1_0),
       RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC1_0),
       RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC1_0),
   };
   if (pgm_coh == 0) {
   cap_nx_write_regs(hbm_nc1, ARRAY_SIZE(hbm_nc1), 0x00000000);
   }

   static const uint32_t hbm_nc2[] = {
       RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC2_0),
       RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC2_0),
       RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC2_0),
       RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC2_0),
       RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC2_0),
       RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC2_0),
       RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC2_0),
   };
   if (pgm_coh == 0) {
   cap_nx_write_regs(hbm_nc2, ARRAY_SIZE(hbm_nc2), 0x00000000);
   }

   static const uint32_t hbm_hbm0[] = {
       RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM0_0),
       RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM0_0),
       RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM0_0),
       RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM0_0),
       RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM0_0),
       RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM0_0),
       RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM0_0),
   };
   if (pgm_coh == 0) {
   cap_nx_write_regs(hbm_hbm0, ARRAY_SIZE(hbm_hbm0), 0x80000010);
   }

   static const uint32_t hbm_hbm1[] = {
       RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM1_0),
       RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM1_0),
       RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM1_0),
       RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM1_0),
       RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM1_0),
       RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM1_0),
       RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM1_0),
   };
   if (pgm_coh == 0) {
   cap_nx_write_regs(hbm_hbm1, ARRAY_SIZE(hbm_hbm1), 0x00000010);
   }

   static const uint32_t hbm_hbm2[] = {
       RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM2_0),
       RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM2_0),
       RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM2_0),
       RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM2_0),
       RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM2_0),
       RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM2_0),
       RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM2_0),
   };
   if (pgm_coh == 0) {
   cap_nx_write_regs(hbm_hbm2, ARRAY_SIZE(hbm_hbm2), 0x00000010);
   }

   static const uint32_t ms_s_rbm[] = {
       RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_RBM_0),
   };
   cap_nx_write_regs(ms_s_rbm, ARRAY_SIZE(ms_s_rbm), 0xffe00000);

   // enable PT to access all MS regions in bypass mode
   //data.resize(1); 
#ifndef CAPRI_SW

   data[0] = 0x80000000; 
   cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_0), 1, data, true, 1);

   PLOG_API_MSG("NX: cache_enable set to " << cache_enable << " ", "during config\n");
   if (cache_enable == 0) {
      cap_nx_cache_disable();
   }

   if (cache_override != 0) {
      cache_override_val &= 0xf;
      cap_nx_cache_override(cache_override_val);
   }

   PLOG_API_MSG("NX: clr_modify_on_masters except ARM set to " << clr_modify_on_masters << " ", "during config\n");
   if (clr_modify_on_masters == 1) {
      cap_nx_clr_modifiable(); // Clear Modifiable Bit on All masters EXCEPT ARM
   }

   PLOG_API_MSG("NX: setup_event_counters set to " << setup_event_counters << " ", "during config\n");
   if (setup_event_counters == 1) {
      cap_nx_setup_llc_counters(0x8);
      cap_nx_setup_am_counters(0);
   }

#endif

   if (nwl_hash_mode == 0) { 
      cap_nx_set_hash_func(0x222222200ul, 0x111111100ul, 0x888888800ul, 0x444444400ul );
   } else {
      cap_nx_set_hash_func(0x222222000ul, 0x111111000ul, 0x888888000ul, 0x444444000ul );

      //data.resize(1);
      data[0] = 0x8; // to remove bits 15:12
      cap_nx_block_write(0, 0x6a100008, 1, data, true, 1);
      cap_nx_block_write(0, 0x6a200008, 1, data, true, 1);
      cap_nx_block_write(0, 0x6a300008, 1, data, true, 1);
      cap_nx_block_write(0, 0x6a400008, 1, data, true, 1);
      cap_nx_block_write(0, 0x6a500008, 1, data, true, 1);
      cap_nx_block_write(0, 0x6a600008, 1, data, true, 1);
      cap_nx_block_write(0, 0x6a700008, 1, data, true, 1);
      cap_nx_block_write(0, 0x6a800008, 1, data, true, 1);
   }

   // set PB to be highest priority
   cap_nx_set_pb_high_priority(255,3,3,3);

   PLOG_API_MSG("NX", "done with init\n");
   SW_PRINT("NOC:NOC Init done\n");

   cap_nx_enable_rbm_access(chip_id, inst_id) ;
}

void cap_nx_enable_1G_to_3G(int chip_id, int inst_id) {

SW_PRINT("NOC:NOC Enable HBM access only from 1G-3G (0xc000_0000 to 0xffff_ffff\n");
PLOG_API_MSG("NX", "Enable HBM access only from 1G-3G (0xc000_0000 to 0xffff_ffff\n");

   // Program all non-coherent background to be disabled too (assumes that the coherent background is already disabled)
   static const uint32_t hbm_nc0[] = {
       RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC0_0),
       RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC0_0),
       RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC0_0),
       RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC0_0),
       RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC0_0),
       RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC0_0),
   };
   cap_nx_write_regs(hbm_nc0, ARRAY_SIZE(hbm_nc0), 0xc0000010);

   static const uint32_t hbm_nc1[] = {
       RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC1_0),
       RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC1_0),
       RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC1_0),
       RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC1_0),
       RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC1_0),
       RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC1_0),
   };
   cap_nx_write_regs(hbm_nc1, ARRAY_SIZE(hbm_nc1), 0x00000010);

   static const uint32_t hbm_nc2[] = {
       RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC2_0),
       RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC2_0),
       RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC2_0),
       RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC2_0),
       RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC2_0),
       RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC2_0),
       RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC2_0),
   };
   cap_nx_write_regs(hbm_nc2, ARRAY_SIZE(hbm_nc2), 0x80000010);

   // 
   // Program the PT/PR/PX/PB/MD/SS to access HBM in coherent space for 64MB and non-coherent 2G-64MB for payload 
   // 

   cap_nx_add_noncoh_secure_pt_to_hbm(chip_id, inst_id, 0, 0x80000000, 0xffc0000010);
   cap_nx_add_noncoh_secure_pt_to_hbm(chip_id, inst_id, 1, 0xc4000000, 0xfffc000010);
   cap_nx_add_noncoh_secure_pt_to_hbm(chip_id, inst_id, 2, 0xc8000000, 0xfff8000010);
   cap_nx_add_noncoh_secure_pt_to_hbm(chip_id, inst_id, 3, 0xd0000000, 0xffd0000010);
   cap_nx_add_noncoh_secure_pt_to_hbm(chip_id, inst_id, 4, 0xe0000000, 0xffe0000010);
   cap_nx_add_noncoh_secure_pt_to_hbm(chip_id, inst_id, 5, 0x100000000, 0xf100000010);
   cap_nx_add_noncoh_secure_pt_to_hbm(chip_id, inst_id, 6, 0x200000000, 0xf200000010);

   cap_nx_add_secure_pt_to_hbm(chip_id, inst_id, 0, 0xc0000000 , 0xfffc000010);

   cap_nx_add_noncoh_secure_pr_to_hbm(chip_id, inst_id, 0, 0x80000000, 0xffc0000010);
   cap_nx_add_noncoh_secure_pr_to_hbm(chip_id, inst_id, 1, 0xc4000000, 0xfffc000010);
   cap_nx_add_noncoh_secure_pr_to_hbm(chip_id, inst_id, 2, 0xc8000000, 0xfff8000010);
   cap_nx_add_noncoh_secure_pr_to_hbm(chip_id, inst_id, 3, 0xd0000000, 0xffd0000010);
   cap_nx_add_noncoh_secure_pr_to_hbm(chip_id, inst_id, 4, 0xe0000000, 0xffe0000010);
   cap_nx_add_noncoh_secure_pr_to_hbm(chip_id, inst_id, 5, 0x100000000, 0xf100000010);
   cap_nx_add_noncoh_secure_pr_to_hbm(chip_id, inst_id, 6, 0x200000000, 0xf200000010);

   cap_nx_add_secure_pr_to_hbm(chip_id, inst_id, 0, 0xc0000000 , 0xfffc000010);

   cap_nx_add_noncoh_secure_px_to_hbm(chip_id, inst_id, 0, 0x80000000, 0xffc0000010);
   cap_nx_add_noncoh_secure_px_to_hbm(chip_id, inst_id, 1, 0xc4000000, 0xfffc000010);
   cap_nx_add_noncoh_secure_px_to_hbm(chip_id, inst_id, 2, 0xc8000000, 0xfff8000010);
   cap_nx_add_noncoh_secure_px_to_hbm(chip_id, inst_id, 3, 0xd0000000, 0xffd0000010);
   cap_nx_add_noncoh_secure_px_to_hbm(chip_id, inst_id, 4, 0xe0000000, 0xffe0000010);
   cap_nx_add_noncoh_secure_px_to_hbm(chip_id, inst_id, 5, 0x100000000, 0xf100000010);
   cap_nx_add_noncoh_secure_px_to_hbm(chip_id, inst_id, 6, 0x200000000, 0xf200000010);

   cap_nx_add_secure_px_to_hbm(chip_id, inst_id, 0, 0xc0000000 , 0xfffc000010);

   cap_nx_add_noncoh_secure_pb_to_hbm(chip_id, inst_id, 0, 0x80000000, 0xffc0000010);
   cap_nx_add_noncoh_secure_pb_to_hbm(chip_id, inst_id, 1, 0xc4000000, 0xfffc000010);
   cap_nx_add_noncoh_secure_pb_to_hbm(chip_id, inst_id, 2, 0xc8000000, 0xfff8000010);
   cap_nx_add_noncoh_secure_pb_to_hbm(chip_id, inst_id, 3, 0xd0000000, 0xffd0000010);
   cap_nx_add_noncoh_secure_pb_to_hbm(chip_id, inst_id, 4, 0xe0000000, 0xffe0000010);
   cap_nx_add_noncoh_secure_pb_to_hbm(chip_id, inst_id, 5, 0x100000000, 0xf100000010);
   cap_nx_add_noncoh_secure_pb_to_hbm(chip_id, inst_id, 6, 0x200000000, 0xf200000010);

   cap_nx_add_secure_pb_to_hbm(chip_id, inst_id, 0, 0xc0000000 , 0xfffc000010);

   cap_nx_add_noncoh_secure_md_to_hbm(chip_id, inst_id, 0, 0x80000000, 0xffc0000010);
   cap_nx_add_noncoh_secure_md_to_hbm(chip_id, inst_id, 1, 0xc4000000, 0xfffc000010);
   cap_nx_add_noncoh_secure_md_to_hbm(chip_id, inst_id, 2, 0xc8000000, 0xfff8000010);
   cap_nx_add_noncoh_secure_md_to_hbm(chip_id, inst_id, 3, 0xd0000000, 0xffd0000010);
   cap_nx_add_noncoh_secure_md_to_hbm(chip_id, inst_id, 4, 0xe0000000, 0xffe0000010);
   cap_nx_add_noncoh_secure_md_to_hbm(chip_id, inst_id, 5, 0x100000000, 0xf100000010);
   cap_nx_add_noncoh_secure_md_to_hbm(chip_id, inst_id, 6, 0x200000000, 0xf200000010);

   cap_nx_add_secure_md_to_hbm(chip_id, inst_id, 0, 0xc0000000 , 0xfffc000010);

   cap_nx_add_noncoh_secure_ms_to_hbm(chip_id, inst_id, 0, 0x80000000, 0xffc0000010);
   cap_nx_add_noncoh_secure_ms_to_hbm(chip_id, inst_id, 1, 0xc4000000, 0xfffc000010);
   cap_nx_add_noncoh_secure_ms_to_hbm(chip_id, inst_id, 2, 0xc8000000, 0xfff8000010);
   cap_nx_add_noncoh_secure_ms_to_hbm(chip_id, inst_id, 3, 0xd0000000, 0xffd0000010);
   cap_nx_add_noncoh_secure_ms_to_hbm(chip_id, inst_id, 4, 0xe0000000, 0xffe0000010);
   cap_nx_add_noncoh_secure_ms_to_hbm(chip_id, inst_id, 5, 0x100000000, 0xf100000010);
   cap_nx_add_noncoh_secure_ms_to_hbm(chip_id, inst_id, 6, 0x200000000, 0xf200000010);

   cap_nx_add_secure_ms_to_hbm(chip_id, inst_id, 0, 0xc0000000 , 0xfffc000010);

   cap_nx_add_noncoh_secure_ss_to_hbm(chip_id, inst_id, 0, 0x80000000, 0xffc0000010);
   cap_nx_add_noncoh_secure_ss_to_hbm(chip_id, inst_id, 1, 0xc4000000, 0xfffc000010);
   cap_nx_add_noncoh_secure_ss_to_hbm(chip_id, inst_id, 2, 0xc8000000, 0xfff8000010);
   cap_nx_add_noncoh_secure_ss_to_hbm(chip_id, inst_id, 3, 0xd0000000, 0xffd0000010);
   cap_nx_add_noncoh_secure_ss_to_hbm(chip_id, inst_id, 4, 0xe0000000, 0xffe0000010);
   cap_nx_add_noncoh_secure_ss_to_hbm(chip_id, inst_id, 5, 0x100000000, 0xf100000010);
   cap_nx_add_noncoh_secure_ss_to_hbm(chip_id, inst_id, 6, 0x200000000, 0xf200000010);

   cap_nx_add_secure_ss_to_hbm(chip_id, inst_id, 0, 0xc0000000 , 0xfffc000010);

}

//
// Allow accesses from PT/PR/SS/MD to go only to MS doorbell and semaphore 
//
void cap_nx_ms_filter_db_sema(int chip_id, int inst_id) {

//uint32_t data[2];

SW_PRINT("NOC:MS filter for PT/PR/SS/MD masters to access only doorbell and semaphore regions in MS\n");
PLOG_API_MSG("NX", "NOC:MS filter for PT/PR/SS/MD masters to access only doorbell and semaphore regions in MS\n");

   static const uint32_t ms_s_base[] = {
       RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_0),
       RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_0),
       RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_0),
       RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_0),
   };

   cap_nx_write_regs(ms_s_base, ARRAY_SIZE(ms_s_base), 0x0);

   static const uint32_t ms_s_mask[] = {
       RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_0),
       RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_0),
       RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_0),
       RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_0),
   };
   cap_nx_write_regs(ms_s_mask, ARRAY_SIZE(ms_s_mask), 0x80000012);

   cap_nx_add_secure_pt_to_ms(chip_id, inst_id, 0, 0x8000002 , 0xffff000012);
   cap_nx_add_secure_pt_to_ms(chip_id, inst_id, 1, 0x9000002 , 0xffff000012);
   cap_nx_add_secure_pt_to_ms(chip_id, inst_id, 2, 0x40000002, 0xfff0000012);
   cap_nx_add_secure_pt_to_ms(chip_id, inst_id, 3, 0x50000002, 0xfff0000012);
   cap_nx_add_secure_pt_to_ms(chip_id, inst_id, 4, 0x6000002 , 0xffff000012);
   cap_nx_add_secure_pt_to_ms(chip_id, inst_id, 5, 0x70000002 , 0xfff0000012);

   cap_nx_add_secure_pr_to_ms(chip_id, inst_id, 0, 0x8000002 , 0xffff000012);
   cap_nx_add_secure_pr_to_ms(chip_id, inst_id, 1, 0x9000002 , 0xffff000012);
   cap_nx_add_secure_pr_to_ms(chip_id, inst_id, 2, 0x40000002, 0xfff0000012);
   cap_nx_add_secure_pr_to_ms(chip_id, inst_id, 3, 0x50000002, 0xfff0000012);
   cap_nx_add_secure_pr_to_ms(chip_id, inst_id, 4, 0x6000002 , 0xffff000012);
   cap_nx_add_secure_pr_to_ms(chip_id, inst_id, 5, 0x70000002 , 0xfff0000012);

   cap_nx_add_secure_ss_to_ms(chip_id, inst_id, 0, 0x8000002 , 0xffff000012);
   cap_nx_add_secure_ss_to_ms(chip_id, inst_id, 1, 0x9000002 , 0xffff000012);
   cap_nx_add_secure_ss_to_ms(chip_id, inst_id, 2, 0x40000002, 0xfff0000012);
   cap_nx_add_secure_ss_to_ms(chip_id, inst_id, 3, 0x50000002, 0xfff0000012);
   cap_nx_add_secure_ss_to_ms(chip_id, inst_id, 4, 0x6000002 , 0xffff000012);
   cap_nx_add_secure_ss_to_ms(chip_id, inst_id, 5, 0x70000002 , 0xfff0000012);

   cap_nx_add_secure_md_to_ms(chip_id, inst_id, 0, 0x8000002 , 0xffff000012);
   cap_nx_add_secure_md_to_ms(chip_id, inst_id, 1, 0x9000002 , 0xffff000012);
   cap_nx_add_secure_md_to_ms(chip_id, inst_id, 2, 0x40000002, 0xfff0000012);
   cap_nx_add_secure_md_to_ms(chip_id, inst_id, 3, 0x50000002, 0xfff0000012);
   cap_nx_add_secure_md_to_ms(chip_id, inst_id, 4, 0x6000002 , 0xffff000012);
   cap_nx_add_secure_md_to_ms(chip_id, inst_id, 5, 0x70000002 , 0xfff0000012);

}

void cap_nx_field_print_am_err(int chip_id, int inst_id, uint64_t value) {

  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E46_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit46: CDDATA Parity Err" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E45_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit45: WDATA Parity Err" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E44_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit44: AWADDR Parity Err" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E43_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit43: AW Parity Err" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E42_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit42: ARADDR Parity Err" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E41_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit41: AR Parity Err" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E40_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit40: Indicates that portcheck detected error (SIB mode only)" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E35_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit35: Parity error in configuration/status registers" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E34_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit34: Traffic sent to a noc layer which is power gate" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E33_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit33: Capture counter1 overflow" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E32_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit32: Capture counter0 overflow " << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E24_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit24: Unexpected narrow write detected" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E23_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit23: Write WRAP not equal to supported cacheline size" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E22_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit22: Write response timeout" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E21_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit21: Write address multi-hit" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E20_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit20: Write exclusive split" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E19_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit19: Non modifiable WRAP" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E18_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit18: Write slave error" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E17_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit17: Write address decode error from slave" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E16_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit16: Local write address decode error" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E8_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit8: Unexpected narrow read detected" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E7_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit7: Read WRAP not equal to supported cacheline size: A WRAP command of unupported cache line size was detected" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E6_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit6: Read response timeout: Read response timeout occurred. With timeout enabled, a response wasn't received within the expected interval" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E5_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit5: Read address multi-hit: An AR command matched against multiple entries in the address table" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E4_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit4: Read exclusive split: An AR command of FIXED burst type was detected" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E3_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit3: Non modifiable WRAP: A WRAP command marked as non-modifiable (ARCACHE[0]=0) was detected" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E2_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit2: Read slave error: A slave error response was received from a slave device" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E1_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit1: Read address decode error from slave: A decode error response was received from a slave device" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PT_M_5_136_AM_ERR_E0_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AM_ERR: bit0: Local read address decode error: ARADDR did not find a match in the master bridges address table and a decode error was issued" << "\n");
  }
}

void cap_nx_field_print_as_err(int chip_id, int inst_id, uint64_t value) {

  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PX_S_57_786_AS_ERR_E38_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AS_ERR: bit38: ACADDR Parity error" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PX_S_57_786_AS_ERR_E37_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AS_ERR: bit37: AC Parity error" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PX_S_57_786_AS_ERR_E36_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AS_ERR: bit36: BRESP Parity error" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PX_S_57_786_AS_ERR_E35_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AS_ERR: bit35: RRESP Parity error" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PX_S_57_786_AS_ERR_E34_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AS_ERR: bit34: RDATA Parity error" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PX_S_57_786_AS_ERR_E33_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AS_ERR: bit33: Parity error in config/status registers" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PX_S_57_786_AS_ERR_E32_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AS_ERR: bit32: Traffic sent to a noc layer which is power gated" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PX_S_57_786_AS_ERR_E19_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AS_ERR: bit19: Write command modified: A write command which was marked as non-modifiable was modified by the slave bridge" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PX_S_57_786_AS_ERR_E18_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AS_ERR: bit18: Unknown write response destination: BID from write response produces a destination which is not present in the routing table" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PX_S_57_786_AS_ERR_E17_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AS_ERR: bit17: Write slave error response: Slave error response received from slave device for write command" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PX_S_57_786_AS_ERR_E16_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AS_ERR: bit16: Write decode error response: Decode error response received from slave device for write command" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PX_S_57_786_AS_ERR_E4_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AS_ERR: bit4: Read command modified: A read command which was marked as non-modifiable was modified by the slave bridge" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PX_S_57_786_AS_ERR_E3_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AS_ERR: bit3: Interleaved read response: Interleaved read response. This can occur if interleaved read response is received from a slave device for which a de-interleaver was not specified" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PX_S_57_786_AS_ERR_E2_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AS_ERR: bit2: Unknown read response destination: RID from read response produces a destination which is not present in the routing table" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PX_S_57_786_AS_ERR_E1_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AS_ERR: bit1: Read slave error response: Slave error response received from slave device for read command" << "\n");
  }
  if ( NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_PX_S_57_786_AS_ERR_E0_GET(value) == 1) {
     PLOG_API_MSG("NX", setw(10) << "AS_ERR: bit0: Read decode error response: Decode error response received from slave device for read command" << "\n");
  }
}

#ifndef CAPRI_SW

void cap_nx_soft_reset(int chip_id, int inst_id) {

int cache_enable;
int pgm_coh;
int cache_override;
int cache_override_val;
int clr_modify_on_masters;
int setup_event_counters;
int nwl_hash_mode;
//vector<uint32_t> data;
string tmp_str;

PLOG_API_MSG("NX", "Inside NX_INIT\n");
PLOG_API_MSG("NX", "Dumping Sknob Vars\n");
//sknobs_dump();




   tmp_str = "pgm_coh";
   pgm_coh = sknobs_get_value((char *)tmp_str.c_str(),0);
   PLOG_API_MSG("NX: pgm_coh set to " << pgm_coh << " ", "during config\n");

   tmp_str = "cache_enable";
   cache_enable = sknobs_get_value((char *)tmp_str.c_str(),1);
   PLOG_API_MSG("NX: cache_enable set to " << cache_enable << " ", "during config\n");


   tmp_str = "cache_override";
   cache_override = sknobs_get_value((char *)tmp_str.c_str(),0);
   tmp_str = "cache_override_val";
   cache_override_val = sknobs_get_value((char *)tmp_str.c_str(),0);


   tmp_str = "clr_modify_on_masters";
   clr_modify_on_masters = sknobs_get_value((char *)tmp_str.c_str(),0);
   PLOG_API_MSG("NX: clr_modify_on_masters except ARM set to " << clr_modify_on_masters << " ", "during config\n");

   tmp_str = "setup_event_counters";
   setup_event_counters = sknobs_get_value((char *)tmp_str.c_str(), 0);
   PLOG_API_MSG("NX: setup_event_counters set to " << setup_event_counters << " ", "during config\n");
   
   tmp_str = "nwl_hash_mode";
   nwl_hash_mode = sknobs_get_value((char *)tmp_str.c_str(), 0);

   cap_nx_soft_reset_sw(chip_id, inst_id, cache_enable, pgm_coh, cache_override, cache_override_val, clr_modify_on_masters, setup_event_counters, nwl_hash_mode);

}

// DAC2
void cap_nx_set_pb_high_priority(uint32_t qos ) {

//vector<uint32_t> data;

   //data.resize(1); 
   uint32_t data;
   data = (qos & 0xff) | ((qos & 0xff) << 8) | ((qos & 0xff) << 16) | ((qos & 0xff) << 24);

   PLOG_API_MSG("NX", "Inside set_pb_high_priority\n");
   static const uint32_t regs[] = {
       RBM_BRIDGE_(PB_M_3_150_P_0),
       RBM_BRIDGE_(PB_M_3_150_P_1),
       RBM_BRIDGE_(PB_M_3_150_P_2),
       RBM_BRIDGE_(PB_M_3_150_P_3),
   };
   cap_nx_write_regs(regs, ARRAY_SIZE(regs), data);
}
#endif

void cap_nx_set_pb_high_priority(uint32_t qos0, uint32_t qos1, uint32_t qos2, uint32_t qos3 ) {

//vector<uint32_t> data;
uint32_t data[2];

   //data.resize(1); 
   data[0] = (qos0 & 0xff) | ((qos1 & 0xff) << 8) | ((qos2 & 0xff) << 16) | ((qos3 & 0xff) << 24);

   PLOG_API_MSG("NX", "Inside set_pb_high_priority\n");
   cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_P_0), 1, data, true, 1);
}

void cap_nx_set_pt_high_priority(uint32_t qos0, uint32_t qos1, uint32_t qos2, uint32_t qos3 ) {

//vector<uint32_t> data;
uint32_t data[2];

   //data.resize(1); 
   data[0] = (qos0 & 0xff) | ((qos1 & 0xff) << 8) | ((qos2 & 0xff) << 16) | ((qos3 & 0xff) << 24);

   PLOG_API_MSG("NX", "Inside set_pt_high_priority\n");
   cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_P_0), 1, data, true, 1);
}

void cap_nx_set_md_high_priority(uint32_t qos0, uint32_t qos1, uint32_t qos2, uint32_t qos3 ) {
//vector<uint32_t> data;
uint32_t data[2];

   //data.resize(1); 
   data[0] = (qos0 & 0xff) | ((qos1 & 0xff) << 8) | ((qos2 & 0xff) << 16) | ((qos3 & 0xff) << 24);

   PLOG_API_MSG("NX", "Inside set_md_high_priority\n");
   cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_P_0), 1, data, true, 1);
}

// DAC2
void cap_nx_cache_disable() {

//vector<uint32_t> data;

   //data.resize(1); 

   PLOG_API_MSG("NX", "Inside cache disable\n");
   static const uint32_t regs[] = {
       RBM_AGENT_(CCC0_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC1_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC2_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC3_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC4_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC5_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC6_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC7_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC8_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC9_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC10_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC11_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC12_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC13_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC14_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC15_LLC_CACHE_WAY_ENABLE),
   };
   cap_nx_write_regs(regs, ARRAY_SIZE(regs), 0);
}

// DAC2
void cap_nx_cache_enable() {

//vector<uint32_t> data;

   //data.resize(1); 

   PLOG_API_MSG("NX", "Inside cache enable\n");
   static const uint32_t regs[] = {
       RBM_AGENT_(CCC0_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC1_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC2_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC3_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC4_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC5_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC6_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC7_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC8_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC9_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC10_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC11_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC12_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC13_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC14_LLC_CACHE_WAY_ENABLE),
       RBM_AGENT_(CCC15_LLC_CACHE_WAY_ENABLE),
   };
   cap_nx_write_regs(regs, ARRAY_SIZE(regs), 0xff);
}

// DAC2
void cap_nx_axcache_enable() {

//vector<uint32_t> data;


   PLOG_API_MSG("NX", "Inside LLC ALLOC Cache Enable\n");

   //data.resize(1); 
   static const uint32_t regs0[] = {
       RBM_AGENT_(CCC0_LLC_ALLOC_ARCACHE_EN),
       RBM_AGENT_(CCC0_LLC_ALLOC_AWCACHE_EN),
       RBM_AGENT_(CCC1_LLC_ALLOC_ARCACHE_EN),
       RBM_AGENT_(CCC1_LLC_ALLOC_AWCACHE_EN),
       RBM_AGENT_(CCC2_LLC_ALLOC_ARCACHE_EN),
       RBM_AGENT_(CCC2_LLC_ALLOC_AWCACHE_EN),
       RBM_AGENT_(CCC3_LLC_ALLOC_ARCACHE_EN),
       RBM_AGENT_(CCC3_LLC_ALLOC_AWCACHE_EN),
       RBM_AGENT_(CCC4_LLC_ALLOC_ARCACHE_EN),
       RBM_AGENT_(CCC4_LLC_ALLOC_AWCACHE_EN),
       RBM_AGENT_(CCC5_LLC_ALLOC_ARCACHE_EN),
       RBM_AGENT_(CCC5_LLC_ALLOC_AWCACHE_EN),
       RBM_AGENT_(CCC6_LLC_ALLOC_ARCACHE_EN),
       RBM_AGENT_(CCC6_LLC_ALLOC_AWCACHE_EN),
       RBM_AGENT_(CCC7_LLC_ALLOC_ARCACHE_EN),
       RBM_AGENT_(CCC7_LLC_ALLOC_AWCACHE_EN),
       RBM_AGENT_(CCC8_LLC_ALLOC_ARCACHE_EN),
       RBM_AGENT_(CCC8_LLC_ALLOC_AWCACHE_EN),
       RBM_AGENT_(CCC9_LLC_ALLOC_ARCACHE_EN),
       RBM_AGENT_(CCC9_LLC_ALLOC_AWCACHE_EN),
       RBM_AGENT_(CCC10_LLC_ALLOC_ARCACHE_EN),
       RBM_AGENT_(CCC10_LLC_ALLOC_AWCACHE_EN),
       RBM_AGENT_(CCC11_LLC_ALLOC_ARCACHE_EN),
       RBM_AGENT_(CCC11_LLC_ALLOC_AWCACHE_EN),
       RBM_AGENT_(CCC12_LLC_ALLOC_ARCACHE_EN),
       RBM_AGENT_(CCC12_LLC_ALLOC_AWCACHE_EN),
       RBM_AGENT_(CCC13_LLC_ALLOC_ARCACHE_EN),
       RBM_AGENT_(CCC13_LLC_ALLOC_AWCACHE_EN),
       RBM_AGENT_(CCC14_LLC_ALLOC_ARCACHE_EN),
       RBM_AGENT_(CCC14_LLC_ALLOC_AWCACHE_EN),
       RBM_AGENT_(CCC15_LLC_ALLOC_ARCACHE_EN),
       RBM_AGENT_(CCC15_LLC_ALLOC_AWCACHE_EN),
   };
   cap_nx_write_regs(regs0, ARRAY_SIZE(regs0), 0xff);

   static const uint32_t regs1[] = {
       RBM_AGENT_(CCC0_LLC_ALLOC_RD_EN),
       RBM_AGENT_(CCC0_LLC_ALLOC_WR_EN),
       RBM_AGENT_(CCC1_LLC_ALLOC_RD_EN),
       RBM_AGENT_(CCC1_LLC_ALLOC_WR_EN),
       RBM_AGENT_(CCC10_LLC_ALLOC_RD_EN),
       RBM_AGENT_(CCC10_LLC_ALLOC_WR_EN),
       RBM_AGENT_(CCC11_LLC_ALLOC_RD_EN),
       RBM_AGENT_(CCC11_LLC_ALLOC_WR_EN),
       RBM_AGENT_(CCC12_LLC_ALLOC_RD_EN),
       RBM_AGENT_(CCC12_LLC_ALLOC_WR_EN),
       RBM_AGENT_(CCC13_LLC_ALLOC_RD_EN),
       RBM_AGENT_(CCC13_LLC_ALLOC_WR_EN),
       RBM_AGENT_(CCC14_LLC_ALLOC_RD_EN),
       RBM_AGENT_(CCC14_LLC_ALLOC_WR_EN),
       RBM_AGENT_(CCC15_LLC_ALLOC_RD_EN),
       RBM_AGENT_(CCC15_LLC_ALLOC_WR_EN),
       RBM_AGENT_(CCC2_LLC_ALLOC_RD_EN),
       RBM_AGENT_(CCC2_LLC_ALLOC_WR_EN),
       RBM_AGENT_(CCC3_LLC_ALLOC_RD_EN),
       RBM_AGENT_(CCC3_LLC_ALLOC_WR_EN),
       RBM_AGENT_(CCC4_LLC_ALLOC_RD_EN),
       RBM_AGENT_(CCC4_LLC_ALLOC_WR_EN),
       RBM_AGENT_(CCC5_LLC_ALLOC_RD_EN),
       RBM_AGENT_(CCC5_LLC_ALLOC_WR_EN),
       RBM_AGENT_(CCC6_LLC_ALLOC_RD_EN),
       RBM_AGENT_(CCC6_LLC_ALLOC_WR_EN),
       RBM_AGENT_(CCC7_LLC_ALLOC_RD_EN),
       RBM_AGENT_(CCC7_LLC_ALLOC_WR_EN),
       RBM_AGENT_(CCC8_LLC_ALLOC_RD_EN),
       RBM_AGENT_(CCC8_LLC_ALLOC_WR_EN),
       RBM_AGENT_(CCC9_LLC_ALLOC_RD_EN),
       RBM_AGENT_(CCC9_LLC_ALLOC_WR_EN),
   };
   cap_nx_write_regs(regs1, ARRAY_SIZE(regs1), 0);

}

// DAC2
void cap_nx_ecc_disable() {

//vector<uint32_t> data;
uint32_t data[2];

   //data.resize(1); 
   data[0] = 1;

   PLOG_API_MSG("NX", "Inside NX ECC disable\n");

   static const uint32_t regs[] = {
       RBM_AGENT_(CCC0_CCC_ECC_DISABLE),
       RBM_AGENT_(CCC0_LLC_ECC_DISABLE),
       RBM_AGENT_(CCC1_CCC_ECC_DISABLE),
       RBM_AGENT_(CCC1_LLC_ECC_DISABLE),
       RBM_AGENT_(CCC10_CCC_ECC_DISABLE),
       RBM_AGENT_(CCC10_LLC_ECC_DISABLE),
       RBM_AGENT_(CCC11_CCC_ECC_DISABLE),
       RBM_AGENT_(CCC11_LLC_ECC_DISABLE),
       RBM_AGENT_(CCC12_CCC_ECC_DISABLE),
       RBM_AGENT_(CCC12_LLC_ECC_DISABLE),
       RBM_AGENT_(CCC13_CCC_ECC_DISABLE),
       RBM_AGENT_(CCC13_LLC_ECC_DISABLE),
       RBM_AGENT_(CCC14_CCC_ECC_DISABLE),
       RBM_AGENT_(CCC14_LLC_ECC_DISABLE),
       RBM_AGENT_(CCC15_CCC_ECC_DISABLE),
       RBM_AGENT_(CCC15_LLC_ECC_DISABLE),
       RBM_AGENT_(CCC2_CCC_ECC_DISABLE),
       RBM_AGENT_(CCC2_LLC_ECC_DISABLE),
       RBM_AGENT_(CCC3_CCC_ECC_DISABLE),
       RBM_AGENT_(CCC3_LLC_ECC_DISABLE),
       RBM_AGENT_(CCC4_CCC_ECC_DISABLE),
       RBM_AGENT_(CCC4_LLC_ECC_DISABLE),
       RBM_AGENT_(CCC5_CCC_ECC_DISABLE),
       RBM_AGENT_(CCC5_LLC_ECC_DISABLE),
       RBM_AGENT_(CCC6_CCC_ECC_DISABLE),
       RBM_AGENT_(CCC6_LLC_ECC_DISABLE),
       RBM_AGENT_(CCC7_CCC_ECC_DISABLE),
       RBM_AGENT_(CCC7_LLC_ECC_DISABLE),
       RBM_AGENT_(CCC8_CCC_ECC_DISABLE),
       RBM_AGENT_(CCC8_LLC_ECC_DISABLE),
       RBM_AGENT_(CCC9_CCC_ECC_DISABLE),
       RBM_AGENT_(CCC9_LLC_ECC_DISABLE)
   };
   for (unsigned i = 0; i < ARRAY_SIZE(regs); i++) {
       cap_nx_block_write(0,regs[i], 1, data, true, 1);
   }
}

#ifndef CAPRI_SW
void cap_nx_display_ecc_count() {
//vector<uint32_t> rd_data;
uint32_t rd_data[2];

   //rd_data.resize(1);
   rd_data[0]=0;
   PLOG_API_MSG("NX", "Inside NX Display ECC err count\n");
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC0_CCC_ECC_INFO), 1, false, 1);
   display_ecc_info(0, rd_data[0]);
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC1_CCC_ECC_INFO), 1, false, 1);
   display_ecc_info(1, rd_data[0]);
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC2_CCC_ECC_INFO), 1, false, 1);
   display_ecc_info(2, rd_data[0]);
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC3_CCC_ECC_INFO), 1, false, 1);
   display_ecc_info(3, rd_data[0]);
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC4_CCC_ECC_INFO), 1, false, 1);
   display_ecc_info(4, rd_data[0]);
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC5_CCC_ECC_INFO), 1, false, 1);
   display_ecc_info(5, rd_data[0]);
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC6_CCC_ECC_INFO), 1, false, 1);
   display_ecc_info(6, rd_data[0]);
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC7_CCC_ECC_INFO), 1, false, 1);
   display_ecc_info(7, rd_data[0]);
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC8_CCC_ECC_INFO), 1, false, 1);
   display_ecc_info(8, rd_data[0]);
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC9_CCC_ECC_INFO), 1, false, 1);
   display_ecc_info(9, rd_data[0]);
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC10_CCC_ECC_INFO), 1, false, 1);
   display_ecc_info(10, rd_data[0]);
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC11_CCC_ECC_INFO), 1, false, 1);
   display_ecc_info(11, rd_data[0]);
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC12_CCC_ECC_INFO), 1, false, 1);
   display_ecc_info(12, rd_data[0]);
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC13_CCC_ECC_INFO), 1, false, 1);
   display_ecc_info(13, rd_data[0]);
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC14_CCC_ECC_INFO), 1, false, 1);
   display_ecc_info(14, rd_data[0]);
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC15_CCC_ECC_INFO), 1, false, 1);
   display_ecc_info(15, rd_data[0]);
}

void display_ecc_info(int ccc_num, uint32_t val) {

uint64_t rd_val;
uint64_t get_val;

/*
DIR_INDEX[39:32] - Index of first detected error.
WAY[2] - Directory bank of first detected error
ECC_COUNT[31:16] - Number of ECC errors found.
DB[1] - 1'b1: Detected double of multi-bit error.
SB[0] - 1'b1: Detected single-bit error.
*/
   if (val == 0) return;

   rd_val = val;
   get_val = NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC0_CCC_ECC_INFO_ECC_COUNT_GET(rd_val);
   PLOG_API_MSG("NX: CCC ECC-Count: 0x" << get_val << "", "\n");
   get_val = NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC0_CCC_ECC_INFO_DB_GET(rd_val);
   PLOG_API_MSG("NX: CCC ECC-DoubleBit Detected: 0x" << get_val << "", "\n");
   get_val = NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC0_CCC_ECC_INFO_SB_GET(rd_val);
   PLOG_API_MSG("NX: CCC ECC-SingleBit Detected: 0x" << get_val << "", "\n");
   return;
}
#endif

// DAC2
void cap_nx_clr_modifiable() {

//vector<uint32_t> data;
uint32_t data;

   //data.resize(1); 
   data = (0x2 << 4) | 0x0; // should clear bit 1 of axcache on all P4/P4+/Crypto masters (PB is not needed since it doesn't go to MS/PX)

   static const uint32_t regs[] = {
       RBM_BRIDGE_(PT_M_5_136_AM_AWOVRD),
       RBM_BRIDGE_(PT_M_5_136_AM_AROVRD),
       RBM_BRIDGE_(PR_M_4_141_AM_AWOVRD),
       RBM_BRIDGE_(PR_M_4_141_AM_AROVRD),
       RBM_BRIDGE_(MD_M_1_138_AM_AWOVRD),
       RBM_BRIDGE_(MD_M_1_138_AM_AROVRD),
       RBM_BRIDGE_(SS_M_7_396_AM_AWOVRD),
       RBM_BRIDGE_(SS_M_7_396_AM_AROVRD),
       RBM_BRIDGE_(PB_M_3_150_AM_AWOVRD),
       RBM_BRIDGE_(PB_M_3_150_AM_AROVRD),
   };
   cap_nx_write_regs(regs, ARRAY_SIZE(regs), data);
}

// DAC2
void cap_nx_cache_override(uint8_t cache_val) {

//vector<uint32_t> data;
uint32_t data;

   //data.resize(1); 
   data = (0xf << 4) | cache_val; // enable all 4 bits and override with cache_value

   PLOG_API_MSG("NX", "Inside cache override \n");

   static const uint32_t regs[] = {
       RBM_BRIDGE_(RBM_M_58_744_AM_AWOVRD),
       RBM_BRIDGE_(AR_M_0_809_AM_AWOVRD),
       RBM_BRIDGE_(MD_M_1_138_AM_AWOVRD),
       RBM_BRIDGE_(MS_M_2_774_AM_AWOVRD),
       RBM_BRIDGE_(MS_S_56_774_AS_AWOVRD),
       RBM_BRIDGE_(PB_M_3_150_AM_AWOVRD),
       RBM_BRIDGE_(PR_M_4_141_AM_AWOVRD),
       RBM_BRIDGE_(PT_M_5_136_AM_AWOVRD),
       RBM_BRIDGE_(PX_M_6_786_AM_AWOVRD),
       RBM_BRIDGE_(PX_S_57_786_AS_AWOVRD),
       RBM_BRIDGE_(SS_M_7_396_AM_AWOVRD),
       RBM_BRIDGE_(CCC0_S_24_264_AS_AWOVRD),
       RBM_BRIDGE_(CCC1_S_25_296_AS_AWOVRD),
       RBM_BRIDGE_(CCC10_S_26_584_AS_AWOVRD),
       RBM_BRIDGE_(CCC11_S_27_616_AS_AWOVRD),
       RBM_BRIDGE_(CCC12_S_28_648_AS_AWOVRD),
       RBM_BRIDGE_(CCC13_S_29_680_AS_AWOVRD),
       RBM_BRIDGE_(CCC14_S_30_712_AS_AWOVRD),
       RBM_BRIDGE_(CCC15_S_31_744_AS_AWOVRD),
       RBM_BRIDGE_(CCC2_S_32_328_AS_AWOVRD),
       RBM_BRIDGE_(CCC3_S_33_360_AS_AWOVRD),
       RBM_BRIDGE_(CCC4_S_34_392_AS_AWOVRD),
       RBM_BRIDGE_(CCC5_S_35_424_AS_AWOVRD),
       RBM_BRIDGE_(CCC6_S_36_456_AS_AWOVRD),
       RBM_BRIDGE_(CCC7_S_37_488_AS_AWOVRD),
       RBM_BRIDGE_(CCC8_S_38_520_AS_AWOVRD),
       RBM_BRIDGE_(CCC9_S_39_552_AS_AWOVRD),

       RBM_BRIDGE_(RBM_M_58_744_AM_AROVRD),
       RBM_BRIDGE_(AR_M_0_809_AM_AROVRD),
       RBM_BRIDGE_(MD_M_1_138_AM_AROVRD),
       RBM_BRIDGE_(MS_M_2_774_AM_AROVRD),
       RBM_BRIDGE_(MS_S_56_774_AS_AROVRD),
       RBM_BRIDGE_(PB_M_3_150_AM_AROVRD),
       RBM_BRIDGE_(PR_M_4_141_AM_AROVRD),
       RBM_BRIDGE_(PT_M_5_136_AM_AROVRD),
       RBM_BRIDGE_(PX_M_6_786_AM_AROVRD),
       RBM_BRIDGE_(PX_S_57_786_AS_AROVRD),
       RBM_BRIDGE_(SS_M_7_396_AM_AROVRD),
       RBM_BRIDGE_(CCC0_S_24_264_AS_AROVRD),
       RBM_BRIDGE_(CCC1_S_25_296_AS_AROVRD),
       RBM_BRIDGE_(CCC10_S_26_584_AS_AROVRD),
       RBM_BRIDGE_(CCC11_S_27_616_AS_AROVRD),
       RBM_BRIDGE_(CCC12_S_28_648_AS_AROVRD),
       RBM_BRIDGE_(CCC13_S_29_680_AS_AROVRD),
       RBM_BRIDGE_(CCC14_S_30_712_AS_AROVRD),
       RBM_BRIDGE_(CCC15_S_31_744_AS_AROVRD),
       RBM_BRIDGE_(CCC2_S_32_328_AS_AROVRD),
       RBM_BRIDGE_(CCC3_S_33_360_AS_AROVRD),
       RBM_BRIDGE_(CCC4_S_34_392_AS_AROVRD),
       RBM_BRIDGE_(CCC5_S_35_424_AS_AROVRD),
       RBM_BRIDGE_(CCC6_S_36_456_AS_AROVRD),
       RBM_BRIDGE_(CCC7_S_37_488_AS_AROVRD),
       RBM_BRIDGE_(CCC8_S_38_520_AS_AROVRD),
       RBM_BRIDGE_(CCC9_S_39_552_AS_AROVRD),
   };
   cap_nx_write_regs(regs, ARRAY_SIZE(regs), data);
}

// DAC2
void cap_nx_force_alloc_enable(uint32_t value) { // value of 0xff is enabling and 0x0 is disabling

//vector<uint32_t> data;
uint32_t data;

   //data.resize(1); 
   data = value;

   PLOG_API_MSG("NX", "Inside force-allocate-enable\n");
   static const uint32_t regs[] = {
       RBM_AGENT_(CCC0_LLC_FORCE_PARTIAL_WR_ALLOC),
       RBM_AGENT_(CCC1_LLC_FORCE_PARTIAL_WR_ALLOC),
       RBM_AGENT_(CCC2_LLC_FORCE_PARTIAL_WR_ALLOC),
       RBM_AGENT_(CCC3_LLC_FORCE_PARTIAL_WR_ALLOC),
       RBM_AGENT_(CCC4_LLC_FORCE_PARTIAL_WR_ALLOC),
       RBM_AGENT_(CCC5_LLC_FORCE_PARTIAL_WR_ALLOC),
       RBM_AGENT_(CCC6_LLC_FORCE_PARTIAL_WR_ALLOC),
       RBM_AGENT_(CCC7_LLC_FORCE_PARTIAL_WR_ALLOC),
       RBM_AGENT_(CCC8_LLC_FORCE_PARTIAL_WR_ALLOC),
       RBM_AGENT_(CCC9_LLC_FORCE_PARTIAL_WR_ALLOC),
       RBM_AGENT_(CCC10_LLC_FORCE_PARTIAL_WR_ALLOC),
       RBM_AGENT_(CCC11_LLC_FORCE_PARTIAL_WR_ALLOC),
       RBM_AGENT_(CCC12_LLC_FORCE_PARTIAL_WR_ALLOC),
       RBM_AGENT_(CCC13_LLC_FORCE_PARTIAL_WR_ALLOC),
       RBM_AGENT_(CCC14_LLC_FORCE_PARTIAL_WR_ALLOC),
       RBM_AGENT_(CCC15_LLC_FORCE_PARTIAL_WR_ALLOC),
   };
   cap_nx_write_regs(regs, ARRAY_SIZE(regs), data);
}

// DAC2
void cap_nx_enable_interrupts() {

   PLOG_API_MSG("NX", "Inside enable-interrupts\n");
   static const uint32_t regs[] = {
       RBM_BRIDGE_(RBM_M_58_744_TXEM),
       RBM_BRIDGE_(RBM_M_58_744_RXEM),
       RBM_BRIDGE_(AR_M_0_809_TXEM),
       RBM_BRIDGE_(AR_M_0_809_RXEM),
       RBM_BRIDGE_(MD_M_1_138_TXEM),
       RBM_BRIDGE_(MD_M_1_138_RXEM),
       RBM_BRIDGE_(MS_M_2_774_TXEM),
       RBM_BRIDGE_(MS_M_2_774_RXEM),
       RBM_BRIDGE_(MS_S_56_774_TXEM),
       RBM_BRIDGE_(MS_S_56_774_RXEM),
       RBM_BRIDGE_(PB_M_3_150_TXEM),
       RBM_BRIDGE_(PB_M_3_150_RXEM),
       RBM_BRIDGE_(PR_M_4_141_TXEM),
       RBM_BRIDGE_(PR_M_4_141_RXEM),
       RBM_BRIDGE_(PT_M_5_136_TXEM),
       RBM_BRIDGE_(PT_M_5_136_RXEM),
       RBM_BRIDGE_(PX_M_6_786_TXEM),
       RBM_BRIDGE_(PX_M_6_786_RXEM),
       RBM_BRIDGE_(PX_S_57_786_TXEM),
       RBM_BRIDGE_(PX_S_57_786_RXEM),
       RBM_BRIDGE_(SS_M_7_396_TXEM),
       RBM_BRIDGE_(SS_M_7_396_RXEM),
       RBM_BRIDGE_(CCC0_S_24_264_TXEM),
       RBM_BRIDGE_(CCC0_S_24_264_RXEM),
       RBM_BRIDGE_(CCC1_S_25_296_TXEM),
       RBM_BRIDGE_(CCC1_S_25_296_RXEM),
       RBM_BRIDGE_(CCC10_S_26_584_TXEM),
       RBM_BRIDGE_(CCC10_S_26_584_RXEM),
       RBM_BRIDGE_(CCC11_S_27_616_TXEM),
       RBM_BRIDGE_(CCC11_S_27_616_RXEM),
       RBM_BRIDGE_(CCC12_S_28_648_TXEM),
       RBM_BRIDGE_(CCC12_S_28_648_RXEM),
       RBM_BRIDGE_(CCC13_S_29_680_TXEM),
       RBM_BRIDGE_(CCC13_S_29_680_RXEM),
       RBM_BRIDGE_(CCC14_S_30_712_TXEM),
       RBM_BRIDGE_(CCC14_S_30_712_RXEM),
       RBM_BRIDGE_(CCC15_S_31_744_TXEM),
       RBM_BRIDGE_(CCC15_S_31_744_RXEM),
       RBM_BRIDGE_(CCC2_S_32_328_TXEM),
       RBM_BRIDGE_(CCC2_S_32_328_RXEM),
       RBM_BRIDGE_(CCC3_S_33_360_TXEM),
       RBM_BRIDGE_(CCC3_S_33_360_RXEM),
       RBM_BRIDGE_(CCC4_S_34_392_TXEM),
       RBM_BRIDGE_(CCC4_S_34_392_RXEM),
       RBM_BRIDGE_(CCC5_S_35_424_TXEM),
       RBM_BRIDGE_(CCC5_S_35_424_RXEM),
       RBM_BRIDGE_(CCC6_S_36_456_TXEM),
       RBM_BRIDGE_(CCC6_S_36_456_RXEM),
       RBM_BRIDGE_(CCC7_S_37_488_TXEM),
       RBM_BRIDGE_(CCC7_S_37_488_RXEM),
       RBM_BRIDGE_(CCC8_S_38_520_TXEM),
       RBM_BRIDGE_(CCC8_S_38_520_RXEM),
       RBM_BRIDGE_(CCC9_S_39_552_TXEM),
       RBM_BRIDGE_(CCC9_S_39_552_RXEM),

       RBM_BRIDGE_(RBM_M_58_744_AM_INTM),
       RBM_BRIDGE_(AR_M_0_809_AM_INTM),
       RBM_BRIDGE_(MD_M_1_138_AM_INTM),
       RBM_BRIDGE_(MS_M_2_774_AM_INTM),
       RBM_BRIDGE_(PB_M_3_150_AM_INTM),
       RBM_BRIDGE_(PR_M_4_141_AM_INTM),
       RBM_BRIDGE_(PT_M_5_136_AM_INTM),
       RBM_BRIDGE_(PX_M_6_786_AM_INTM),
       RBM_BRIDGE_(SS_M_7_396_AM_INTM),
       
       RBM_BRIDGE_(RBM_M_58_744_BRPERRM0),
       RBM_BRIDGE_(AR_M_0_809_BRPERRM0),
       RBM_BRIDGE_(MD_M_1_138_BRPERRM0),
       RBM_BRIDGE_(MS_M_2_774_BRPERRM0),
       RBM_BRIDGE_(MS_S_56_774_BRPERRM0),
       RBM_BRIDGE_(PB_M_3_150_BRPERRM0),
       RBM_BRIDGE_(PR_M_4_141_BRPERRM0),
       RBM_BRIDGE_(PT_M_5_136_BRPERRM0),
       RBM_BRIDGE_(PX_M_6_786_BRPERRM0),
       RBM_BRIDGE_(PX_S_57_786_BRPERRM0),
       RBM_BRIDGE_(SS_M_7_396_BRPERRM0),
       RBM_BRIDGE_(CCC0_S_24_264_BRPERRM0),
       RBM_BRIDGE_(CCC1_S_25_296_BRPERRM0),
       RBM_BRIDGE_(CCC10_S_26_584_BRPERRM0),
       RBM_BRIDGE_(CCC11_S_27_616_BRPERRM0),
       RBM_BRIDGE_(CCC12_S_28_648_BRPERRM0),
       RBM_BRIDGE_(CCC13_S_29_680_BRPERRM0),
       RBM_BRIDGE_(CCC14_S_30_712_BRPERRM0),
       RBM_BRIDGE_(CCC15_S_31_744_BRPERRM0),
       RBM_BRIDGE_(CCC2_S_32_328_BRPERRM0),
       RBM_BRIDGE_(CCC3_S_33_360_BRPERRM0),
       RBM_BRIDGE_(CCC4_S_34_392_BRPERRM0),
       RBM_BRIDGE_(CCC5_S_35_424_BRPERRM0),
       RBM_BRIDGE_(CCC6_S_36_456_BRPERRM0),
       RBM_BRIDGE_(CCC7_S_37_488_BRPERRM0),
       RBM_BRIDGE_(CCC8_S_38_520_BRPERRM0),
       RBM_BRIDGE_(CCC9_S_39_552_BRPERRM0),
       
       RBM_BRIDGE_(MS_S_56_774_AS_INTM),
       RBM_BRIDGE_(PX_S_57_786_AS_INTM),
       RBM_BRIDGE_(CCC0_S_24_264_AS_INTM),
       RBM_BRIDGE_(CCC1_S_25_296_AS_INTM),
       RBM_BRIDGE_(CCC10_S_26_584_AS_INTM),
       RBM_BRIDGE_(CCC11_S_27_616_AS_INTM),
       RBM_BRIDGE_(CCC12_S_28_648_AS_INTM),
       RBM_BRIDGE_(CCC13_S_29_680_AS_INTM),
       RBM_BRIDGE_(CCC14_S_30_712_AS_INTM),
       RBM_BRIDGE_(CCC15_S_31_744_AS_INTM),
       RBM_BRIDGE_(CCC2_S_32_328_AS_INTM),
       RBM_BRIDGE_(CCC3_S_33_360_AS_INTM),
       RBM_BRIDGE_(CCC4_S_34_392_AS_INTM),
       RBM_BRIDGE_(CCC5_S_35_424_AS_INTM),
       RBM_BRIDGE_(CCC6_S_36_456_AS_INTM),
       RBM_BRIDGE_(CCC7_S_37_488_AS_INTM),
       RBM_BRIDGE_(CCC8_S_38_520_AS_INTM),
       RBM_BRIDGE_(CCC9_S_39_552_AS_INTM),
       
       RBM_BRIDGE_(RBM_M_58_744_AM_INTM)+4,
       RBM_BRIDGE_(AR_M_0_809_AM_INTM)+4,
       RBM_BRIDGE_(MD_M_1_138_AM_INTM)+4,
       RBM_BRIDGE_(MS_M_2_774_AM_INTM)+4,
       RBM_BRIDGE_(PB_M_3_150_AM_INTM)+4,
       RBM_BRIDGE_(PR_M_4_141_AM_INTM)+4,
       RBM_BRIDGE_(PT_M_5_136_AM_INTM)+4,
       RBM_BRIDGE_(PX_M_6_786_AM_INTM)+4,
       RBM_BRIDGE_(SS_M_7_396_AM_INTM)+4,
       
       RBM_BRIDGE_(RBM_M_58_744_BRPERRM0)+4,
       RBM_BRIDGE_(AR_M_0_809_BRPERRM0)+4,
       RBM_BRIDGE_(MD_M_1_138_BRPERRM0)+4,
       RBM_BRIDGE_(MS_M_2_774_BRPERRM0)+4,
       RBM_BRIDGE_(MS_S_56_774_BRPERRM0)+4,
       RBM_BRIDGE_(PB_M_3_150_BRPERRM0)+4,
       RBM_BRIDGE_(PR_M_4_141_BRPERRM0)+4,
       RBM_BRIDGE_(PT_M_5_136_BRPERRM0)+4,
       RBM_BRIDGE_(PX_M_6_786_BRPERRM0)+4,
       RBM_BRIDGE_(PX_S_57_786_BRPERRM0)+4,
       RBM_BRIDGE_(SS_M_7_396_BRPERRM0)+4,
       RBM_BRIDGE_(CCC0_S_24_264_BRPERRM0)+4,
       RBM_BRIDGE_(CCC1_S_25_296_BRPERRM0)+4,
       RBM_BRIDGE_(CCC10_S_26_584_BRPERRM0)+4,
       RBM_BRIDGE_(CCC11_S_27_616_BRPERRM0)+4,
       RBM_BRIDGE_(CCC12_S_28_648_BRPERRM0)+4,
       RBM_BRIDGE_(CCC13_S_29_680_BRPERRM0)+4,
       RBM_BRIDGE_(CCC14_S_30_712_BRPERRM0)+4,
       RBM_BRIDGE_(CCC15_S_31_744_BRPERRM0)+4,
       RBM_BRIDGE_(CCC2_S_32_328_BRPERRM0)+4,
       RBM_BRIDGE_(CCC3_S_33_360_BRPERRM0)+4,
       RBM_BRIDGE_(CCC4_S_34_392_BRPERRM0)+4,
       RBM_BRIDGE_(CCC5_S_35_424_BRPERRM0)+4,
       RBM_BRIDGE_(CCC6_S_36_456_BRPERRM0)+4,
       RBM_BRIDGE_(CCC7_S_37_488_BRPERRM0)+4,
       RBM_BRIDGE_(CCC8_S_38_520_BRPERRM0)+4,
       RBM_BRIDGE_(CCC9_S_39_552_BRPERRM0)+4,
       
       RBM_BRIDGE_(MS_S_56_774_AS_INTM)+4,
       RBM_BRIDGE_(PX_S_57_786_AS_INTM)+4,
       RBM_BRIDGE_(CCC0_S_24_264_AS_INTM)+4,
       RBM_BRIDGE_(CCC1_S_25_296_AS_INTM)+4,
       RBM_BRIDGE_(CCC10_S_26_584_AS_INTM)+4,
       RBM_BRIDGE_(CCC11_S_27_616_AS_INTM)+4,
       RBM_BRIDGE_(CCC12_S_28_648_AS_INTM)+4,
       RBM_BRIDGE_(CCC13_S_29_680_AS_INTM)+4,
       RBM_BRIDGE_(CCC14_S_30_712_AS_INTM)+4,
       RBM_BRIDGE_(CCC15_S_31_744_AS_INTM)+4,
       RBM_BRIDGE_(CCC2_S_32_328_AS_INTM)+4,
       RBM_BRIDGE_(CCC3_S_33_360_AS_INTM)+4,
       RBM_BRIDGE_(CCC4_S_34_392_AS_INTM)+4,
       RBM_BRIDGE_(CCC5_S_35_424_AS_INTM)+4,
       RBM_BRIDGE_(CCC6_S_36_456_AS_INTM)+4,
       RBM_BRIDGE_(CCC7_S_37_488_AS_INTM)+4,
       RBM_BRIDGE_(CCC8_S_38_520_AS_INTM)+4,
       RBM_BRIDGE_(CCC9_S_39_552_AS_INTM)+4,
   };
   cap_nx_write_regs(regs, ARRAY_SIZE(regs), 0);
}

void cap_nx_disable_interrupts() {

   PLOG_API_MSG("NX", "Inside disable-interrupts\n");
   static const uint32_t regs[] = {
       RBM_BRIDGE_(RBM_M_58_744_TXEM),
       RBM_BRIDGE_(RBM_M_58_744_RXEM),
       RBM_BRIDGE_(AR_M_0_809_TXEM),
       RBM_BRIDGE_(AR_M_0_809_RXEM),
       RBM_BRIDGE_(MD_M_1_138_TXEM),
       RBM_BRIDGE_(MD_M_1_138_RXEM),
       RBM_BRIDGE_(MS_M_2_774_TXEM),
       RBM_BRIDGE_(MS_M_2_774_RXEM),
       RBM_BRIDGE_(MS_S_56_774_TXEM),
       RBM_BRIDGE_(MS_S_56_774_RXEM),
       RBM_BRIDGE_(PB_M_3_150_TXEM),
       RBM_BRIDGE_(PB_M_3_150_RXEM),
       RBM_BRIDGE_(PR_M_4_141_TXEM),
       RBM_BRIDGE_(PR_M_4_141_RXEM),
       RBM_BRIDGE_(PT_M_5_136_TXEM),
       RBM_BRIDGE_(PT_M_5_136_RXEM),
       RBM_BRIDGE_(PX_M_6_786_TXEM),
       RBM_BRIDGE_(PX_M_6_786_RXEM),
       RBM_BRIDGE_(PX_S_57_786_TXEM),
       RBM_BRIDGE_(PX_S_57_786_RXEM),
       RBM_BRIDGE_(SS_M_7_396_TXEM),
       RBM_BRIDGE_(SS_M_7_396_RXEM),
       RBM_BRIDGE_(CCC0_S_24_264_TXEM),
       RBM_BRIDGE_(CCC0_S_24_264_RXEM),
       RBM_BRIDGE_(CCC1_S_25_296_TXEM),
       RBM_BRIDGE_(CCC1_S_25_296_RXEM),
       RBM_BRIDGE_(CCC10_S_26_584_TXEM),
       RBM_BRIDGE_(CCC10_S_26_584_RXEM),
       RBM_BRIDGE_(CCC11_S_27_616_TXEM),
       RBM_BRIDGE_(CCC11_S_27_616_RXEM),
       RBM_BRIDGE_(CCC12_S_28_648_TXEM),
       RBM_BRIDGE_(CCC12_S_28_648_RXEM),
       RBM_BRIDGE_(CCC13_S_29_680_TXEM),
       RBM_BRIDGE_(CCC13_S_29_680_RXEM),
       RBM_BRIDGE_(CCC14_S_30_712_TXEM),
       RBM_BRIDGE_(CCC14_S_30_712_RXEM),
       RBM_BRIDGE_(CCC15_S_31_744_TXEM),
       RBM_BRIDGE_(CCC15_S_31_744_RXEM),
       RBM_BRIDGE_(CCC2_S_32_328_TXEM),
       RBM_BRIDGE_(CCC2_S_32_328_RXEM),
       RBM_BRIDGE_(CCC3_S_33_360_TXEM),
       RBM_BRIDGE_(CCC3_S_33_360_RXEM),
       RBM_BRIDGE_(CCC4_S_34_392_TXEM),
       RBM_BRIDGE_(CCC4_S_34_392_RXEM),
       RBM_BRIDGE_(CCC5_S_35_424_TXEM),
       RBM_BRIDGE_(CCC5_S_35_424_RXEM),
       RBM_BRIDGE_(CCC6_S_36_456_TXEM),
       RBM_BRIDGE_(CCC6_S_36_456_RXEM),
       RBM_BRIDGE_(CCC7_S_37_488_TXEM),
       RBM_BRIDGE_(CCC7_S_37_488_RXEM),
       RBM_BRIDGE_(CCC8_S_38_520_TXEM),
       RBM_BRIDGE_(CCC8_S_38_520_RXEM),
       RBM_BRIDGE_(CCC9_S_39_552_TXEM),
       RBM_BRIDGE_(CCC9_S_39_552_RXEM),

       RBM_BRIDGE_(RBM_M_58_744_AM_INTM),
       RBM_BRIDGE_(AR_M_0_809_AM_INTM),
       RBM_BRIDGE_(MD_M_1_138_AM_INTM),
       RBM_BRIDGE_(MS_M_2_774_AM_INTM),
       RBM_BRIDGE_(PB_M_3_150_AM_INTM),
       RBM_BRIDGE_(PR_M_4_141_AM_INTM),
       RBM_BRIDGE_(PT_M_5_136_AM_INTM),
       RBM_BRIDGE_(PX_M_6_786_AM_INTM),
       RBM_BRIDGE_(SS_M_7_396_AM_INTM),
       
       RBM_BRIDGE_(RBM_M_58_744_BRPERRM0),
       RBM_BRIDGE_(AR_M_0_809_BRPERRM0),
       RBM_BRIDGE_(MD_M_1_138_BRPERRM0),
       RBM_BRIDGE_(MS_M_2_774_BRPERRM0),
       RBM_BRIDGE_(MS_S_56_774_BRPERRM0),
       RBM_BRIDGE_(PB_M_3_150_BRPERRM0),
       RBM_BRIDGE_(PR_M_4_141_BRPERRM0),
       RBM_BRIDGE_(PT_M_5_136_BRPERRM0),
       RBM_BRIDGE_(PX_M_6_786_BRPERRM0),
       RBM_BRIDGE_(PX_S_57_786_BRPERRM0),
       RBM_BRIDGE_(SS_M_7_396_BRPERRM0),
       RBM_BRIDGE_(CCC0_S_24_264_BRPERRM0),
       RBM_BRIDGE_(CCC1_S_25_296_BRPERRM0),
       RBM_BRIDGE_(CCC10_S_26_584_BRPERRM0),
       RBM_BRIDGE_(CCC11_S_27_616_BRPERRM0),
       RBM_BRIDGE_(CCC12_S_28_648_BRPERRM0),
       RBM_BRIDGE_(CCC13_S_29_680_BRPERRM0),
       RBM_BRIDGE_(CCC14_S_30_712_BRPERRM0),
       RBM_BRIDGE_(CCC15_S_31_744_BRPERRM0),
       RBM_BRIDGE_(CCC2_S_32_328_BRPERRM0),
       RBM_BRIDGE_(CCC3_S_33_360_BRPERRM0),
       RBM_BRIDGE_(CCC4_S_34_392_BRPERRM0),
       RBM_BRIDGE_(CCC5_S_35_424_BRPERRM0),
       RBM_BRIDGE_(CCC6_S_36_456_BRPERRM0),
       RBM_BRIDGE_(CCC7_S_37_488_BRPERRM0),
       RBM_BRIDGE_(CCC8_S_38_520_BRPERRM0),
       RBM_BRIDGE_(CCC9_S_39_552_BRPERRM0),
       
       RBM_BRIDGE_(MS_S_56_774_AS_INTM),
       RBM_BRIDGE_(PX_S_57_786_AS_INTM),
       RBM_BRIDGE_(CCC0_S_24_264_AS_INTM),
       RBM_BRIDGE_(CCC1_S_25_296_AS_INTM),
       RBM_BRIDGE_(CCC10_S_26_584_AS_INTM),
       RBM_BRIDGE_(CCC11_S_27_616_AS_INTM),
       RBM_BRIDGE_(CCC12_S_28_648_AS_INTM),
       RBM_BRIDGE_(CCC13_S_29_680_AS_INTM),
       RBM_BRIDGE_(CCC14_S_30_712_AS_INTM),
       RBM_BRIDGE_(CCC15_S_31_744_AS_INTM),
       RBM_BRIDGE_(CCC2_S_32_328_AS_INTM),
       RBM_BRIDGE_(CCC3_S_33_360_AS_INTM),
       RBM_BRIDGE_(CCC4_S_34_392_AS_INTM),
       RBM_BRIDGE_(CCC5_S_35_424_AS_INTM),
       RBM_BRIDGE_(CCC6_S_36_456_AS_INTM),
       RBM_BRIDGE_(CCC7_S_37_488_AS_INTM),
       RBM_BRIDGE_(CCC8_S_38_520_AS_INTM),
       RBM_BRIDGE_(CCC9_S_39_552_AS_INTM),
       
       RBM_BRIDGE_(RBM_M_58_744_AM_INTM)+4,
       RBM_BRIDGE_(AR_M_0_809_AM_INTM)+4,
       RBM_BRIDGE_(MD_M_1_138_AM_INTM)+4,
       RBM_BRIDGE_(MS_M_2_774_AM_INTM)+4,
       RBM_BRIDGE_(PB_M_3_150_AM_INTM)+4,
       RBM_BRIDGE_(PR_M_4_141_AM_INTM)+4,
       RBM_BRIDGE_(PT_M_5_136_AM_INTM)+4,
       RBM_BRIDGE_(PX_M_6_786_AM_INTM)+4,
       RBM_BRIDGE_(SS_M_7_396_AM_INTM)+4,
       
       RBM_BRIDGE_(RBM_M_58_744_BRPERRM0)+4,
       RBM_BRIDGE_(AR_M_0_809_BRPERRM0)+4,
       RBM_BRIDGE_(MD_M_1_138_BRPERRM0)+4,
       RBM_BRIDGE_(MS_M_2_774_BRPERRM0)+4,
       RBM_BRIDGE_(MS_S_56_774_BRPERRM0)+4,
       RBM_BRIDGE_(PB_M_3_150_BRPERRM0)+4,
       RBM_BRIDGE_(PR_M_4_141_BRPERRM0)+4,
       RBM_BRIDGE_(PT_M_5_136_BRPERRM0)+4,
       RBM_BRIDGE_(PX_M_6_786_BRPERRM0)+4,
       RBM_BRIDGE_(PX_S_57_786_BRPERRM0)+4,
       RBM_BRIDGE_(SS_M_7_396_BRPERRM0)+4,
       RBM_BRIDGE_(CCC0_S_24_264_BRPERRM0)+4,
       RBM_BRIDGE_(CCC1_S_25_296_BRPERRM0)+4,
       RBM_BRIDGE_(CCC10_S_26_584_BRPERRM0)+4,
       RBM_BRIDGE_(CCC11_S_27_616_BRPERRM0)+4,
       RBM_BRIDGE_(CCC12_S_28_648_BRPERRM0)+4,
       RBM_BRIDGE_(CCC13_S_29_680_BRPERRM0)+4,
       RBM_BRIDGE_(CCC14_S_30_712_BRPERRM0)+4,
       RBM_BRIDGE_(CCC15_S_31_744_BRPERRM0)+4,
       RBM_BRIDGE_(CCC2_S_32_328_BRPERRM0)+4,
       RBM_BRIDGE_(CCC3_S_33_360_BRPERRM0)+4,
       RBM_BRIDGE_(CCC4_S_34_392_BRPERRM0)+4,
       RBM_BRIDGE_(CCC5_S_35_424_BRPERRM0)+4,
       RBM_BRIDGE_(CCC6_S_36_456_BRPERRM0)+4,
       RBM_BRIDGE_(CCC7_S_37_488_BRPERRM0)+4,
       RBM_BRIDGE_(CCC8_S_38_520_BRPERRM0)+4,
       RBM_BRIDGE_(CCC9_S_39_552_BRPERRM0)+4,
       
       RBM_BRIDGE_(MS_S_56_774_AS_INTM)+4,
       RBM_BRIDGE_(PX_S_57_786_AS_INTM)+4,
       RBM_BRIDGE_(CCC0_S_24_264_AS_INTM)+4,
       RBM_BRIDGE_(CCC1_S_25_296_AS_INTM)+4,
       RBM_BRIDGE_(CCC10_S_26_584_AS_INTM)+4,
       RBM_BRIDGE_(CCC11_S_27_616_AS_INTM)+4,
       RBM_BRIDGE_(CCC12_S_28_648_AS_INTM)+4,
       RBM_BRIDGE_(CCC13_S_29_680_AS_INTM)+4,
       RBM_BRIDGE_(CCC14_S_30_712_AS_INTM)+4,
       RBM_BRIDGE_(CCC15_S_31_744_AS_INTM)+4,
       RBM_BRIDGE_(CCC2_S_32_328_AS_INTM)+4,
       RBM_BRIDGE_(CCC3_S_33_360_AS_INTM)+4,
       RBM_BRIDGE_(CCC4_S_34_392_AS_INTM)+4,
       RBM_BRIDGE_(CCC5_S_35_424_AS_INTM)+4,
       RBM_BRIDGE_(CCC6_S_36_456_AS_INTM)+4,
       RBM_BRIDGE_(CCC7_S_37_488_AS_INTM)+4,
       RBM_BRIDGE_(CCC8_S_38_520_AS_INTM)+4,
       RBM_BRIDGE_(CCC9_S_39_552_AS_INTM)+4,
   };
   cap_nx_write_regs(regs, ARRAY_SIZE(regs), 0xffffffff);
}

void cap_nx_clear_interrupts(int chip_id) {

   static const uint32_t regs[] = {
         RBM_BRIDGE_(RBM_M_58_744_TXE) ,
         RBM_BRIDGE_(RBM_M_58_744_RXE) ,
         RBM_BRIDGE_(AR_M_0_809_TXE) ,
         RBM_BRIDGE_(AR_M_0_809_RXE) ,
         RBM_BRIDGE_(MD_M_1_138_TXE) ,
         RBM_BRIDGE_(MD_M_1_138_RXE) ,
         RBM_BRIDGE_(MS_M_2_774_TXE) ,
         RBM_BRIDGE_(MS_M_2_774_RXE) ,
         RBM_BRIDGE_(MS_S_56_774_TXE) ,
         RBM_BRIDGE_(MS_S_56_774_RXE) ,
         RBM_BRIDGE_(PB_M_3_150_TXE) ,
         RBM_BRIDGE_(PB_M_3_150_RXE) ,
         RBM_BRIDGE_(PR_M_4_141_TXE) ,
         RBM_BRIDGE_(PR_M_4_141_RXE) ,
         RBM_BRIDGE_(PT_M_5_136_TXE) ,
         RBM_BRIDGE_(PT_M_5_136_RXE) ,
         RBM_BRIDGE_(PX_M_6_786_TXE) ,
         RBM_BRIDGE_(PX_M_6_786_RXE) ,
         RBM_BRIDGE_(PX_S_57_786_TXE) ,
         RBM_BRIDGE_(PX_S_57_786_RXE) ,
         RBM_BRIDGE_(SS_M_7_396_TXE) ,
         RBM_BRIDGE_(SS_M_7_396_RXE) ,
         RBM_BRIDGE_(CCC0_S_24_264_TXE) ,
         RBM_BRIDGE_(CCC0_S_24_264_RXE) ,
         RBM_BRIDGE_(CCC1_S_25_296_TXE) ,
         RBM_BRIDGE_(CCC1_S_25_296_RXE) ,
         RBM_BRIDGE_(CCC10_S_26_584_TXE) ,
         RBM_BRIDGE_(CCC10_S_26_584_RXE) ,
         RBM_BRIDGE_(CCC11_S_27_616_TXE) ,
         RBM_BRIDGE_(CCC11_S_27_616_RXE) ,
         RBM_BRIDGE_(CCC12_S_28_648_TXE) ,
         RBM_BRIDGE_(CCC12_S_28_648_RXE) ,
         RBM_BRIDGE_(CCC13_S_29_680_TXE) ,
         RBM_BRIDGE_(CCC13_S_29_680_RXE) ,
         RBM_BRIDGE_(CCC14_S_30_712_TXE) ,
         RBM_BRIDGE_(CCC14_S_30_712_RXE) ,
         RBM_BRIDGE_(CCC15_S_31_744_TXE) ,
         RBM_BRIDGE_(CCC15_S_31_744_RXE) ,
         RBM_BRIDGE_(CCC2_S_32_328_TXE) ,
         RBM_BRIDGE_(CCC2_S_32_328_RXE) ,
         RBM_BRIDGE_(CCC3_S_33_360_TXE) ,
         RBM_BRIDGE_(CCC3_S_33_360_RXE) ,
         RBM_BRIDGE_(CCC4_S_34_392_TXE) ,
         RBM_BRIDGE_(CCC4_S_34_392_RXE) ,
         RBM_BRIDGE_(CCC5_S_35_424_TXE) ,
         RBM_BRIDGE_(CCC5_S_35_424_RXE) ,
         RBM_BRIDGE_(CCC6_S_36_456_TXE) ,
         RBM_BRIDGE_(CCC6_S_36_456_RXE) ,
         RBM_BRIDGE_(CCC7_S_37_488_TXE) ,
         RBM_BRIDGE_(CCC7_S_37_488_RXE) ,
         RBM_BRIDGE_(CCC8_S_38_520_TXE) ,
         RBM_BRIDGE_(CCC8_S_38_520_RXE) ,
         RBM_BRIDGE_(CCC9_S_39_552_TXE) ,
         RBM_BRIDGE_(CCC9_S_39_552_RXE) ,
         RBM_BRIDGE_(RBM_M_58_744_AM_ERR) ,
         RBM_BRIDGE_(AR_M_0_809_AM_ERR) ,
         RBM_BRIDGE_(MD_M_1_138_AM_ERR) ,
         RBM_BRIDGE_(MS_M_2_774_AM_ERR) ,
         RBM_BRIDGE_(PB_M_3_150_AM_ERR) ,
         RBM_BRIDGE_(PR_M_4_141_AM_ERR) ,
         RBM_BRIDGE_(PT_M_5_136_AM_ERR) ,
         RBM_BRIDGE_(PX_M_6_786_AM_ERR) ,
         RBM_BRIDGE_(SS_M_7_396_AM_ERR) ,
         RBM_BRIDGE_(MS_S_56_774_AS_ERR) ,
         RBM_BRIDGE_(PX_S_57_786_AS_ERR) ,
         RBM_BRIDGE_(CCC0_S_24_264_AS_ERR) ,
         RBM_BRIDGE_(CCC1_S_25_296_AS_ERR) ,
         RBM_BRIDGE_(CCC10_S_26_584_AS_ERR) ,
         RBM_BRIDGE_(CCC11_S_27_616_AS_ERR) ,
         RBM_BRIDGE_(CCC12_S_28_648_AS_ERR) ,
         RBM_BRIDGE_(CCC13_S_29_680_AS_ERR) ,
         RBM_BRIDGE_(CCC14_S_30_712_AS_ERR) ,
         RBM_BRIDGE_(CCC15_S_31_744_AS_ERR) ,
         RBM_BRIDGE_(CCC2_S_32_328_AS_ERR) ,
         RBM_BRIDGE_(CCC3_S_33_360_AS_ERR) ,
         RBM_BRIDGE_(CCC4_S_34_392_AS_ERR) ,
         RBM_BRIDGE_(CCC5_S_35_424_AS_ERR) ,
         RBM_BRIDGE_(CCC6_S_36_456_AS_ERR) ,
         RBM_BRIDGE_(CCC7_S_37_488_AS_ERR) ,
         RBM_BRIDGE_(CCC8_S_38_520_AS_ERR) ,
         RBM_BRIDGE_(CCC9_S_39_552_AS_ERR) ,
         RBM_BRIDGE_(RBM_M_58_744_BRPERR0) ,
         RBM_BRIDGE_(AR_M_0_809_BRPERR0) ,
         RBM_BRIDGE_(MD_M_1_138_BRPERR0) ,
         RBM_BRIDGE_(MS_M_2_774_BRPERR0) ,
         RBM_BRIDGE_(MS_S_56_774_BRPERR0) ,
         RBM_BRIDGE_(PB_M_3_150_BRPERR0) ,
         RBM_BRIDGE_(PR_M_4_141_BRPERR0) ,
         RBM_BRIDGE_(PT_M_5_136_BRPERR0) ,
         RBM_BRIDGE_(PX_M_6_786_BRPERR0) ,
         RBM_BRIDGE_(PX_S_57_786_BRPERR0) ,
         RBM_BRIDGE_(SS_M_7_396_BRPERR0) ,
         RBM_BRIDGE_(CCC0_S_24_264_BRPERR0) ,
         RBM_BRIDGE_(CCC1_S_25_296_BRPERR0) ,
         RBM_BRIDGE_(CCC10_S_26_584_BRPERR0) ,
         RBM_BRIDGE_(CCC11_S_27_616_BRPERR0) ,
         RBM_BRIDGE_(CCC12_S_28_648_BRPERR0) ,
         RBM_BRIDGE_(CCC13_S_29_680_BRPERR0) ,
         RBM_BRIDGE_(CCC14_S_30_712_BRPERR0) ,
         RBM_BRIDGE_(CCC15_S_31_744_BRPERR0) ,
         RBM_BRIDGE_(CCC2_S_32_328_BRPERR0) ,
         RBM_BRIDGE_(CCC3_S_33_360_BRPERR0) ,
         RBM_BRIDGE_(CCC4_S_34_392_BRPERR0) ,
         RBM_BRIDGE_(CCC5_S_35_424_BRPERR0) ,
         RBM_BRIDGE_(CCC6_S_36_456_BRPERR0) ,
         RBM_BRIDGE_(CCC7_S_37_488_BRPERR0) ,
         RBM_BRIDGE_(CCC8_S_38_520_BRPERR0) ,
         RBM_BRIDGE_(CCC9_S_39_552_BRPERR0) ,
         RBM_BRIDGE_(RBM_M_58_744_AM_ERR)+4 ,
         RBM_BRIDGE_(AR_M_0_809_AM_ERR)+4 ,
         RBM_BRIDGE_(MD_M_1_138_AM_ERR)+4 ,
         RBM_BRIDGE_(MS_M_2_774_AM_ERR)+4 ,
         RBM_BRIDGE_(PB_M_3_150_AM_ERR)+4 ,
         RBM_BRIDGE_(PR_M_4_141_AM_ERR)+4 ,
         RBM_BRIDGE_(PT_M_5_136_AM_ERR)+4 ,
         RBM_BRIDGE_(PX_M_6_786_AM_ERR)+4 ,
         RBM_BRIDGE_(SS_M_7_396_AM_ERR)+4 ,
         RBM_BRIDGE_(MS_S_56_774_AS_ERR)+4 ,
         RBM_BRIDGE_(PX_S_57_786_AS_ERR)+4 ,
         RBM_BRIDGE_(CCC0_S_24_264_AS_ERR)+4 ,
         RBM_BRIDGE_(CCC1_S_25_296_AS_ERR)+4 ,
         RBM_BRIDGE_(CCC10_S_26_584_AS_ERR)+4 ,
         RBM_BRIDGE_(CCC11_S_27_616_AS_ERR)+4 ,
         RBM_BRIDGE_(CCC12_S_28_648_AS_ERR)+4 ,
         RBM_BRIDGE_(CCC13_S_29_680_AS_ERR)+4 ,
         RBM_BRIDGE_(CCC14_S_30_712_AS_ERR)+4 ,
         RBM_BRIDGE_(CCC15_S_31_744_AS_ERR)+4 ,
         RBM_BRIDGE_(CCC2_S_32_328_AS_ERR)+4 ,
         RBM_BRIDGE_(CCC3_S_33_360_AS_ERR)+4 ,
         RBM_BRIDGE_(CCC4_S_34_392_AS_ERR)+4 ,
         RBM_BRIDGE_(CCC5_S_35_424_AS_ERR)+4 ,
         RBM_BRIDGE_(CCC6_S_36_456_AS_ERR)+4 ,
         RBM_BRIDGE_(CCC7_S_37_488_AS_ERR)+4 ,
         RBM_BRIDGE_(CCC8_S_38_520_AS_ERR)+4 ,
         RBM_BRIDGE_(CCC9_S_39_552_AS_ERR)+4 ,
    };
/*
         RBM_BRIDGE_(RBM_M_58_744_BRPERR0)+4 ,
         RBM_BRIDGE_(AR_M_0_809_BRPERR0)+4 ,
         RBM_BRIDGE_(MD_M_1_138_BRPERR0)+4 ,
         RBM_BRIDGE_(MS_M_2_774_BRPERR0)+4 ,
         RBM_BRIDGE_(MS_S_56_774_BRPERR0)+4 ,
         RBM_BRIDGE_(PB_M_3_150_BRPERR0)+4 ,
         RBM_BRIDGE_(PR_M_4_141_BRPERR0)+4 ,
         RBM_BRIDGE_(PT_M_5_136_BRPERR0)+4 ,
         RBM_BRIDGE_(PX_M_6_786_BRPERR0)+4 ,
         RBM_BRIDGE_(PX_S_57_786_BRPERR0)+4 ,
         RBM_BRIDGE_(SS_M_7_396_BRPERR0)+4 ,
         RBM_BRIDGE_(CCC0_S_24_264_BRPERR0)+4 ,
         RBM_BRIDGE_(CCC1_S_25_296_BRPERR0)+4 ,
         RBM_BRIDGE_(CCC10_S_26_584_BRPERR0)+4 ,
         RBM_BRIDGE_(CCC11_S_27_616_BRPERR0)+4 ,
         RBM_BRIDGE_(CCC12_S_28_648_BRPERR0)+4 ,
         RBM_BRIDGE_(CCC13_S_29_680_BRPERR0)+4 ,
         RBM_BRIDGE_(CCC14_S_30_712_BRPERR0)+4 ,
         RBM_BRIDGE_(CCC15_S_31_744_BRPERR0)+4 ,
         RBM_BRIDGE_(CCC2_S_32_328_BRPERR0)+4 ,
         RBM_BRIDGE_(CCC3_S_33_360_BRPERR0)+4 ,
         RBM_BRIDGE_(CCC4_S_34_392_BRPERR0)+4 ,
         RBM_BRIDGE_(CCC5_S_35_424_BRPERR0)+4 ,
         RBM_BRIDGE_(CCC6_S_36_456_BRPERR0)+4 ,
         RBM_BRIDGE_(CCC7_S_37_488_BRPERR0)+4 ,
         RBM_BRIDGE_(CCC8_S_38_520_BRPERR0)+4 ,
         RBM_BRIDGE_(CCC9_S_39_552_BRPERR0)+4 ,
*/
    cap_nx_write_regs(regs, ARRAY_SIZE(regs), 0x0); // write zero to clear
}

// DAC2
void cap_nx_set_hash_func(uint64_t hash3_bmap, uint64_t hash2_bmap, uint64_t hash1_bmap, uint64_t hash0_bmap) {

//vector<uint32_t> data0;
//vector<uint32_t> data1;
//vector<uint32_t> data2;
//vector<uint32_t> data3;
uint32_t data0;
uint32_t data1;
uint32_t data2;
uint32_t data3;

   //data0.resize(1); data1.resize(1); data2.resize(1); data3.resize(1); 

   data3 = hash3_bmap & 0xffffffff;
   data2 = hash2_bmap & 0xffffffff;
   data1 = hash1_bmap & 0xffffffff;
   data0 = hash0_bmap & 0xffffffff;

   PLOG_API_MSG("NX", "Inside set-hash-function\n");
   static const uint32_t regs0[] = {
       RBM_BRIDGE_(AR_M_0_809_AM_HASH_FUNC_HASH0),
       RBM_BRIDGE_(MD_M_1_138_AM_HASH_FUNC_HASH0),
       RBM_BRIDGE_(MS_M_2_774_AM_HASH_FUNC_HASH0),
       RBM_BRIDGE_(PB_M_3_150_AM_HASH_FUNC_HASH0),
       RBM_BRIDGE_(PR_M_4_141_AM_HASH_FUNC_HASH0),
       RBM_BRIDGE_(PT_M_5_136_AM_HASH_FUNC_HASH0),
       RBM_BRIDGE_(PX_M_6_786_AM_HASH_FUNC_HASH0),
       RBM_BRIDGE_(SS_M_7_396_AM_HASH_FUNC_HASH0),
   };
   cap_nx_write_regs(regs0, ARRAY_SIZE(regs0), data0);

   static const uint32_t regs1[] = {
       RBM_BRIDGE_(AR_M_0_809_AM_HASH_FUNC_HASH1),
       RBM_BRIDGE_(MD_M_1_138_AM_HASH_FUNC_HASH1),
       RBM_BRIDGE_(MS_M_2_774_AM_HASH_FUNC_HASH1),
       RBM_BRIDGE_(PB_M_3_150_AM_HASH_FUNC_HASH1),
       RBM_BRIDGE_(PR_M_4_141_AM_HASH_FUNC_HASH1),
       RBM_BRIDGE_(PT_M_5_136_AM_HASH_FUNC_HASH1),
       RBM_BRIDGE_(PX_M_6_786_AM_HASH_FUNC_HASH1),
       RBM_BRIDGE_(SS_M_7_396_AM_HASH_FUNC_HASH1),
   };
   cap_nx_write_regs(regs1, ARRAY_SIZE(regs1), data1);

   static const uint32_t regs2[] = {
       RBM_BRIDGE_(AR_M_0_809_AM_HASH_FUNC_HASH2),
       RBM_BRIDGE_(MD_M_1_138_AM_HASH_FUNC_HASH2),
       RBM_BRIDGE_(MS_M_2_774_AM_HASH_FUNC_HASH2),
       RBM_BRIDGE_(PB_M_3_150_AM_HASH_FUNC_HASH2),
       RBM_BRIDGE_(PR_M_4_141_AM_HASH_FUNC_HASH2),
       RBM_BRIDGE_(PT_M_5_136_AM_HASH_FUNC_HASH2),
       RBM_BRIDGE_(PX_M_6_786_AM_HASH_FUNC_HASH2),
       RBM_BRIDGE_(SS_M_7_396_AM_HASH_FUNC_HASH2),
   };
   cap_nx_write_regs(regs2, ARRAY_SIZE(regs2), data2);

   static const uint32_t regs3[] = {
       RBM_BRIDGE_(AR_M_0_809_AM_HASH_FUNC_HASH3),
       RBM_BRIDGE_(MD_M_1_138_AM_HASH_FUNC_HASH3),
       RBM_BRIDGE_(MS_M_2_774_AM_HASH_FUNC_HASH3),
       RBM_BRIDGE_(PB_M_3_150_AM_HASH_FUNC_HASH3),
       RBM_BRIDGE_(PR_M_4_141_AM_HASH_FUNC_HASH3),
       RBM_BRIDGE_(PT_M_5_136_AM_HASH_FUNC_HASH3),
       RBM_BRIDGE_(PX_M_6_786_AM_HASH_FUNC_HASH3),
       RBM_BRIDGE_(SS_M_7_396_AM_HASH_FUNC_HASH3),
   };
   cap_nx_write_regs(regs3, ARRAY_SIZE(regs3), data3);

   data3 = (hash3_bmap & (0xfful <<  32)) >> 32; // to get bits 32:39
   data2 = (hash2_bmap & (0xfful <<  32)) >> 32; 
   data1 = (hash1_bmap & (0xfful <<  32)) >> 32;
   data0 = (hash0_bmap & (0xfful <<  32)) >> 32;

   static const uint32_t regs4[] = {
       RBM_BRIDGE_(AR_M_0_809_AM_HASH_FUNC_HASH0)+4,
       RBM_BRIDGE_(MD_M_1_138_AM_HASH_FUNC_HASH0)+4,
       RBM_BRIDGE_(MS_M_2_774_AM_HASH_FUNC_HASH0)+4,
       RBM_BRIDGE_(PB_M_3_150_AM_HASH_FUNC_HASH0)+4,
       RBM_BRIDGE_(PR_M_4_141_AM_HASH_FUNC_HASH0)+4,
       RBM_BRIDGE_(PT_M_5_136_AM_HASH_FUNC_HASH0)+4,
       RBM_BRIDGE_(PX_M_6_786_AM_HASH_FUNC_HASH0)+4,
       RBM_BRIDGE_(SS_M_7_396_AM_HASH_FUNC_HASH0)+4,
   };
   cap_nx_write_regs(regs4, ARRAY_SIZE(regs4), data0);

   static const uint32_t regs5[] = {
       RBM_BRIDGE_(AR_M_0_809_AM_HASH_FUNC_HASH1)+4,
       RBM_BRIDGE_(MD_M_1_138_AM_HASH_FUNC_HASH1)+4,
       RBM_BRIDGE_(MS_M_2_774_AM_HASH_FUNC_HASH1)+4,
       RBM_BRIDGE_(PB_M_3_150_AM_HASH_FUNC_HASH1)+4,
       RBM_BRIDGE_(PR_M_4_141_AM_HASH_FUNC_HASH1)+4,
       RBM_BRIDGE_(PT_M_5_136_AM_HASH_FUNC_HASH1)+4,
       RBM_BRIDGE_(PX_M_6_786_AM_HASH_FUNC_HASH1)+4,
       RBM_BRIDGE_(SS_M_7_396_AM_HASH_FUNC_HASH1)+4,
   };
   cap_nx_write_regs(regs5, ARRAY_SIZE(regs5), data1);

   static const uint32_t regs6[] = {
       RBM_BRIDGE_(AR_M_0_809_AM_HASH_FUNC_HASH2)+4,
       RBM_BRIDGE_(MD_M_1_138_AM_HASH_FUNC_HASH2)+4,
       RBM_BRIDGE_(MS_M_2_774_AM_HASH_FUNC_HASH2)+4,
       RBM_BRIDGE_(PB_M_3_150_AM_HASH_FUNC_HASH2)+4,
       RBM_BRIDGE_(PR_M_4_141_AM_HASH_FUNC_HASH2)+4,
       RBM_BRIDGE_(PT_M_5_136_AM_HASH_FUNC_HASH2)+4,
       RBM_BRIDGE_(PX_M_6_786_AM_HASH_FUNC_HASH2)+4,
       RBM_BRIDGE_(SS_M_7_396_AM_HASH_FUNC_HASH2)+4,
   };
   cap_nx_write_regs(regs6, ARRAY_SIZE(regs6), data2);

   static const uint32_t regs7[] = {
       RBM_BRIDGE_(AR_M_0_809_AM_HASH_FUNC_HASH3)+4,
       RBM_BRIDGE_(MD_M_1_138_AM_HASH_FUNC_HASH3)+4,
       RBM_BRIDGE_(MS_M_2_774_AM_HASH_FUNC_HASH3)+4,
       RBM_BRIDGE_(PB_M_3_150_AM_HASH_FUNC_HASH3)+4,
       RBM_BRIDGE_(PR_M_4_141_AM_HASH_FUNC_HASH3)+4,
       RBM_BRIDGE_(PT_M_5_136_AM_HASH_FUNC_HASH3)+4,
       RBM_BRIDGE_(PX_M_6_786_AM_HASH_FUNC_HASH3)+4,
       RBM_BRIDGE_(SS_M_7_396_AM_HASH_FUNC_HASH3)+4,
   };
   cap_nx_write_regs(regs7, ARRAY_SIZE(regs7), data3);
}

#ifndef CAPRI_SW
void cap_nx_global_alloc(uint32_t alloc_value) { // 0xff for alloc enable, 0x00 for alloc-disable

vector<uint32_t> data;

   PLOG_API_MSG("NX", "Inside nx-cache llc-global-alloc\n");
   data.resize(1); 
   data[0] = alloc_value;

   cpu::access()->block_write(0, RBM_AGENT_(CCC0_LLC_GLOBAL_ALLOC), 1, data, true, 1);
   cpu::access()->block_write(0, RBM_AGENT_(CCC1_LLC_GLOBAL_ALLOC), 1, data, true, 1);
   cpu::access()->block_write(0, RBM_AGENT_(CCC2_LLC_GLOBAL_ALLOC), 1, data, true, 1);
   cpu::access()->block_write(0, RBM_AGENT_(CCC3_LLC_GLOBAL_ALLOC), 1, data, true, 1);
   cpu::access()->block_write(0, RBM_AGENT_(CCC4_LLC_GLOBAL_ALLOC), 1, data, true, 1);
   cpu::access()->block_write(0, RBM_AGENT_(CCC5_LLC_GLOBAL_ALLOC), 1, data, true, 1);
   cpu::access()->block_write(0, RBM_AGENT_(CCC6_LLC_GLOBAL_ALLOC), 1, data, true, 1);
   cpu::access()->block_write(0, RBM_AGENT_(CCC7_LLC_GLOBAL_ALLOC), 1, data, true, 1);
   cpu::access()->block_write(0, RBM_AGENT_(CCC8_LLC_GLOBAL_ALLOC), 1, data, true, 1);
   cpu::access()->block_write(0, RBM_AGENT_(CCC9_LLC_GLOBAL_ALLOC), 1, data, true, 1);
   cpu::access()->block_write(0, RBM_AGENT_(CCC10_LLC_GLOBAL_ALLOC), 1, data, true, 1);
   cpu::access()->block_write(0, RBM_AGENT_(CCC11_LLC_GLOBAL_ALLOC), 1, data, true, 1);
   cpu::access()->block_write(0, RBM_AGENT_(CCC12_LLC_GLOBAL_ALLOC), 1, data, true, 1);
   cpu::access()->block_write(0, RBM_AGENT_(CCC13_LLC_GLOBAL_ALLOC), 1, data, true, 1);
   cpu::access()->block_write(0, RBM_AGENT_(CCC14_LLC_GLOBAL_ALLOC), 1, data, true, 1);
   cpu::access()->block_write(0, RBM_AGENT_(CCC15_LLC_GLOBAL_ALLOC), 1, data, true, 1);

}
#endif

// DAC2
void cap_nx_flush_cache() {

//vector<uint32_t> data;

   PLOG_API_MSG("NX", "Inside nx-flush cache\n");

   //data.resize(1); 

   PLOG_API_MSG("NX", "Inside cache disable\n");
   static const uint32_t regs[] = {
       RBM_AGENT_(CCC0_LLC_WAY_FLUSH),
       RBM_AGENT_(CCC1_LLC_WAY_FLUSH),
       RBM_AGENT_(CCC2_LLC_WAY_FLUSH),
       RBM_AGENT_(CCC3_LLC_WAY_FLUSH),
       RBM_AGENT_(CCC4_LLC_WAY_FLUSH),
       RBM_AGENT_(CCC5_LLC_WAY_FLUSH),
       RBM_AGENT_(CCC6_LLC_WAY_FLUSH),
       RBM_AGENT_(CCC7_LLC_WAY_FLUSH),
       RBM_AGENT_(CCC8_LLC_WAY_FLUSH),
       RBM_AGENT_(CCC9_LLC_WAY_FLUSH),
       RBM_AGENT_(CCC10_LLC_WAY_FLUSH),
       RBM_AGENT_(CCC11_LLC_WAY_FLUSH),
       RBM_AGENT_(CCC12_LLC_WAY_FLUSH),
       RBM_AGENT_(CCC13_LLC_WAY_FLUSH),
       RBM_AGENT_(CCC14_LLC_WAY_FLUSH),
       RBM_AGENT_(CCC15_LLC_WAY_FLUSH),
   };
   cap_nx_write_regs(regs, ARRAY_SIZE(regs), 0xffffffff);

   cap_nx_wait_flush_done();
}

// DAC2
void cap_nx_wait_flush_done() {

//vector<uint32_t> rd_data;
uint32_t rd_data;
uint32_t flush_done = 0;
uint32_t timeout_cnt = 256;
unsigned i;

   PLOG_API_MSG("NX", "Inside wait for nx-flush done\n");

   //rd_data.resize(1); 

   static const uint32_t regs[] = {
          RBM_AGENT_(CCC0_LLC_WAY_FLUSH),
          RBM_AGENT_(CCC1_LLC_WAY_FLUSH),
          RBM_AGENT_(CCC2_LLC_WAY_FLUSH),
          RBM_AGENT_(CCC3_LLC_WAY_FLUSH),
          RBM_AGENT_(CCC4_LLC_WAY_FLUSH),
          RBM_AGENT_(CCC5_LLC_WAY_FLUSH),
          RBM_AGENT_(CCC6_LLC_WAY_FLUSH),
          RBM_AGENT_(CCC7_LLC_WAY_FLUSH),
          RBM_AGENT_(CCC8_LLC_WAY_FLUSH),
          RBM_AGENT_(CCC9_LLC_WAY_FLUSH),
          RBM_AGENT_(CCC10_LLC_WAY_FLUSH),
          RBM_AGENT_(CCC11_LLC_WAY_FLUSH),
          RBM_AGENT_(CCC12_LLC_WAY_FLUSH),
          RBM_AGENT_(CCC13_LLC_WAY_FLUSH),
          RBM_AGENT_(CCC14_LLC_WAY_FLUSH),
          RBM_AGENT_(CCC15_LLC_WAY_FLUSH),
   };
   while (flush_done == 0 && (timeout_cnt != 0)) {
      flush_done = 1; 
      for (i = 0; i < ARRAY_SIZE(regs); i++) {
          rd_data = cap_nx_block_read(0, regs[i], 1, false, 1);
          if (rd_data != 0) flush_done = 0; 
      }
      timeout_cnt = timeout_cnt - 1; 
   }
}

void cap_nx_invalidate_cache() {

//vector<uint32_t> data;
uint32_t data[2];

   PLOG_API_MSG("NX", "Inside nx-invalidate cache\n");

   //data.resize(1); 
   data[0] = 0xff;

   cap_nx_block_write(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC0_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, data, true, 1);
   cap_nx_block_write(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC1_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, data, true, 1);
   cap_nx_block_write(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC2_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, data, true, 1);
   cap_nx_block_write(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC3_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, data, true, 1);
   cap_nx_block_write(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC4_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, data, true, 1);
   cap_nx_block_write(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC5_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, data, true, 1);
   cap_nx_block_write(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC6_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, data, true, 1);
   cap_nx_block_write(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC7_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, data, true, 1);
   cap_nx_block_write(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC8_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, data, true, 1);
   cap_nx_block_write(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC9_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, data, true, 1);
   cap_nx_block_write(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC10_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, data, true, 1);
   cap_nx_block_write(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC11_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, data, true, 1);
   cap_nx_block_write(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC12_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, data, true, 1);
   cap_nx_block_write(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC13_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, data, true, 1);
   cap_nx_block_write(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC14_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, data, true, 1);
   cap_nx_block_write(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC15_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, data, true, 1);

   cap_nx_wait_invalidate_done();
}

void cap_nx_wait_invalidate_done() {

//vector<uint32_t> rd_data;
uint32_t rd_data[2];
uint32_t invalidate_done = 0;
uint32_t timeout_cnt = 256;

   PLOG_API_MSG("NX", "Inside wait for nx-invalidate done\n");

   //rd_data.resize(1); 
   rd_data[0] = 0x0;

   while (invalidate_done == 0 && (timeout_cnt != 0)) {
      invalidate_done = 1; 
      rd_data[0] = cap_nx_block_read(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC0_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, false, 1);
      if (rd_data[0] != 0) invalidate_done = 0; 
      rd_data[0] = cap_nx_block_read(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC1_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, false, 1);
      if (rd_data[0] != 0) invalidate_done = 0; 
      rd_data[0] = cap_nx_block_read(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC2_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, false, 1);
      if (rd_data[0] != 0) invalidate_done = 0; 
      rd_data[0] = cap_nx_block_read(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC3_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, false, 1);
      if (rd_data[0] != 0) invalidate_done = 0; 
      rd_data[0] = cap_nx_block_read(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC4_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, false, 1);
      if (rd_data[0] != 0) invalidate_done = 0; 
      rd_data[0] = cap_nx_block_read(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC5_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, false, 1);
      if (rd_data[0] != 0) invalidate_done = 0; 
      rd_data[0] = cap_nx_block_read(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC6_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, false, 1);
      if (rd_data[0] != 0) invalidate_done = 0; 
      rd_data[0] = cap_nx_block_read(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC7_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, false, 1);
      if (rd_data[0] != 0) invalidate_done = 0; 
      rd_data[0] = cap_nx_block_read(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC8_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, false, 1);
      if (rd_data[0] != 0) invalidate_done = 0; 
      rd_data[0] = cap_nx_block_read(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC9_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, false, 1);
      if (rd_data[0] != 0) invalidate_done = 0; 
      rd_data[0] = cap_nx_block_read(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC10_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, false, 1);
      if (rd_data[0] != 0) invalidate_done = 0; 
      rd_data[0] = cap_nx_block_read(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC11_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, false, 1);
      if (rd_data[0] != 0) invalidate_done = 0; 
      rd_data[0] = cap_nx_block_read(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC12_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, false, 1);
      if (rd_data[0] != 0) invalidate_done = 0; 
      rd_data[0] = cap_nx_block_read(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC13_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, false, 1);
      if (rd_data[0] != 0) invalidate_done = 0; 
      rd_data[0] = cap_nx_block_read(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC14_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, false, 1);
      if (rd_data[0] != 0) invalidate_done = 0; 
      rd_data[0] = cap_nx_block_read(0, CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_CCC15_LLC_TAG_INV_CTL_BYTE_ADDRESS, 1, false, 1);
      if (rd_data[0] != 0) invalidate_done = 0; 
      timeout_cnt = timeout_cnt - 1; 
   }
}

void cap_nx_set_soft_reset(int chip_id, int inst_id, int value) {
}

void cap_nx_set_axi_attrs(int chip_id, int inst_id ) {

uint32_t data[2];
uint32_t read_vector[2];

   PLOG_API_MSG("NX", "Set AXI attributes of Fill-Cache CP/DC \n");

   data[0] = 0x4f;
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_RPC_PICS_OFFSET + CAP_PICS_CSR_PICC_CFG_CACHE_FILL_AXI_BYTE_ADDRESS, 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_TPC_PICS_OFFSET + CAP_PICS_CSR_PICC_CFG_CACHE_FILL_AXI_BYTE_ADDRESS, 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_SSI_PICS_OFFSET + CAP_PICS_CSR_PICC_CFG_CACHE_FILL_AXI_BYTE_ADDRESS, 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_SSE_PICS_OFFSET + CAP_PICS_CSR_PICC_CFG_CACHE_FILL_AXI_BYTE_ADDRESS, 1, data, true, 1);

   data[0] = 0xff;
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MS_MS_OFFSET + CAP_MS_CSR_CFG_MSEM_BYTE_ADDRESS, 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MS_MS_OFFSET + CAP_MS_CSR_CFG_CACHE_ESEC_BYTE_ADDRESS, 1, data, true, 1);

   read_vector[0] = cap_nx_block_read(0, CAP_ADDR_BASE_SSE_PICS_OFFSET + CAP_PICS_CSR_PICC_CFG_CACHE_FILL_AXI_BYTE_ADDRESS, true, 1, 1);
   if (read_vector[0] != 0x4f) {
      PLOG_MSG("CAP_NX: set-axi-attrs write/read failed for PICS_CACHE_FILL_AXI. Check security bit" << endl);
      SW_PRINT("ERROR:set-axi-attrs write/read failed for PICS_CACHE_FILL_AXI. Check security flag\n");
   }

   data[0] = 0x3c0f03c0;
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MD_HENS_OFFSET + CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_AXI_SETTINGS_W0_BYTE_ADDRESS, 1, data, true, 1);
   data[0] = 0xf0;
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MD_HENS_OFFSET + CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_AXI_SETTINGS_W1_BYTE_ADDRESS, 1, data, true, 1);
   data[0] = 0x3c0f03c0;
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MD_HENS_OFFSET + CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_AXI_SETTINGS_W0_BYTE_ADDRESS, 1, data, true, 1);
   data[0] = 0xf0;
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MD_HENS_OFFSET + CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_AXI_SETTINGS_W1_BYTE_ADDRESS, 1, data, true, 1);

   read_vector[0] = cap_nx_block_read(0, CAP_ADDR_BASE_MD_HENS_OFFSET + CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_AXI_SETTINGS_W0_BYTE_ADDRESS, true, 1, 1);
   if (read_vector[0] != 0x3c0f03c0) {
      PLOG_MSG("CAP_NX: set-axi-attrs write/read failed for HENS_CSR_DHS_CRYPTO_CTL AXI settings. Check security bit" << endl);
      SW_PRINT("ERROR:set-axi-attrs write/read failed for HENS_CSR_DHS_CRYPTO_CTL AXI settings. Check security flag\n");
   }


}
 
void cap_nx_set_md_axi_attrs(int chip_id, int inst_id ) { // NOTE: Needs MD-MP/HE/ init for this code to work 

uint32_t data[2];
uint32_t read_vector[2];

   PLOG_API_MSG("NX", "Set AXI attributes of MD-HENS/HESE/MPSE\n");

   data[0] = 0x3c0;
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP0_AXI_DESC_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP0_AXI_DATA_READ_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP0_AXI_DATA_WRITE_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP0_AXI_STATUS_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP1_AXI_DESC_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP1_AXI_DATA_READ_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP1_AXI_DATA_WRITE_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP1_AXI_STATUS_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP2_AXI_DESC_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP2_AXI_DATA_READ_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP2_AXI_DATA_WRITE_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP2_AXI_STATUS_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP3_AXI_DESC_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP3_AXI_DATA_READ_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP3_AXI_DATA_WRITE_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP3_AXI_STATUS_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP4_AXI_DESC_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP4_AXI_DATA_READ_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP4_AXI_DATA_WRITE_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP4_AXI_STATUS_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP5_AXI_DESC_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP5_AXI_DATA_READ_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP5_AXI_DATA_WRITE_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP5_AXI_STATUS_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP6_AXI_DESC_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP6_AXI_DATA_READ_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP6_AXI_DATA_WRITE_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP6_AXI_STATUS_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP7_AXI_DESC_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP7_AXI_DATA_READ_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP7_AXI_DATA_WRITE_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP7_AXI_STATUS_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MD_HESE_OFFSET +  CAP_HESE_CSR_DHS_CRYPTO_CTL_XTS_ENC_AXI_DESC_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MD_HESE_OFFSET +  CAP_HESE_CSR_DHS_CRYPTO_CTL_XTS_ENC_AXI_DATA_READ_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MD_HESE_OFFSET +  CAP_HESE_CSR_DHS_CRYPTO_CTL_XTS_ENC_AXI_DATA_WRITE_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MD_HESE_OFFSET +  CAP_HESE_CSR_DHS_CRYPTO_CTL_XTS_ENC_AXI_STATUS_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MD_HESE_OFFSET +  CAP_HESE_CSR_DHS_CRYPTO_CTL_XTS_AXI_DESC_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MD_HESE_OFFSET +  CAP_HESE_CSR_DHS_CRYPTO_CTL_XTS_AXI_DATA_READ_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MD_HESE_OFFSET +  CAP_HESE_CSR_DHS_CRYPTO_CTL_XTS_AXI_DATA_WRITE_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MD_HESE_OFFSET +  CAP_HESE_CSR_DHS_CRYPTO_CTL_XTS_AXI_STATUS_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MD_HESE_OFFSET +  CAP_HESE_CSR_DHS_CRYPTO_CTL_GCM0_AXI_DESC_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MD_HESE_OFFSET +  CAP_HESE_CSR_DHS_CRYPTO_CTL_GCM0_AXI_DATA_READ_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MD_HESE_OFFSET +  CAP_HESE_CSR_DHS_CRYPTO_CTL_GCM0_AXI_DATA_WRITE_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MD_HESE_OFFSET +  CAP_HESE_CSR_DHS_CRYPTO_CTL_GCM0_AXI_STATUS_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MD_HESE_OFFSET +  CAP_HESE_CSR_DHS_CRYPTO_CTL_GCM1_AXI_DESC_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MD_HESE_OFFSET +  CAP_HESE_CSR_DHS_CRYPTO_CTL_GCM1_AXI_DATA_READ_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MD_HESE_OFFSET +  CAP_HESE_CSR_DHS_CRYPTO_CTL_GCM1_AXI_DATA_WRITE_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MD_HESE_OFFSET +  CAP_HESE_CSR_DHS_CRYPTO_CTL_GCM1_AXI_STATUS_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MD_HESE_OFFSET +  CAP_HESE_CSR_DHS_CRYPTO_CTL_PK_AXI_DESC_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MD_HESE_OFFSET +  CAP_HESE_CSR_DHS_CRYPTO_CTL_PK_AXI_DATA_READ_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MD_HESE_OFFSET +  CAP_HESE_CSR_DHS_CRYPTO_CTL_PK_AXI_DATA_WRITE_BYTE_ADDRESS , 1, data, true, 1);
   cap_nx_block_write(chip_id, CAP_ADDR_BASE_MD_HESE_OFFSET +  CAP_HESE_CSR_DHS_CRYPTO_CTL_PK_AXI_STATUS_BYTE_ADDRESS , 1, data, true, 1);

   read_vector[0] = cap_nx_block_read(0, CAP_ADDR_BASE_MP_MPSE_OFFSET +  CAP_MPSE_CSR_DHS_CRYPTO_CTL_MPP7_AXI_STATUS_BYTE_ADDRESS, true, 1, 1);
   if (read_vector[0] != 0x3c0) {
      PLOG_MSG("CAP_NX: set-axi-attrs write/read failed for HESE_CSR_DHS_CRYPTO_MPP AXI settings. Check security bit" << endl);
      SW_PRINT("ERROR:set-axi-attrs write/read failed for HESE_CSR_DHS_CRYPTO_MPP AXI settings. Check security flag\n");
   }
   read_vector[0] = cap_nx_block_read(0, CAP_ADDR_BASE_MD_HESE_OFFSET +  CAP_HESE_CSR_DHS_CRYPTO_CTL_PK_AXI_STATUS_BYTE_ADDRESS, true, 1, 1);
   if (read_vector[0] != 0x3c0) {
      PLOG_MSG("CAP_NX: set-axi-attrs write/read failed for HESE_CSR_DHS_CRYPTO_PK/GCM0/GCM1/XTS AXI settings. Check security bit" << endl);
      SW_PRINT("ERROR:set-axi-attrs write/read failed for HESE_CSR_DHS_CRYPTO_PK/GCM0/GCM1/XTS AXI settings. Check security flag\n");
   }
}

void cap_nx_sw_reset(int chip_id, int inst_id) {
}

void cap_nx_init_start(int chip_id, int inst_id) {
}

void cap_nx_init_done(int chip_id, int inst_id) {

}

void cap_nx_load_from_cfg(int chip_id, int inst_id) {
 PLOG_API_MSG("NX", "load_from_cfg not present\n");
}


#ifndef CAPRI_SW
void cap_nx_eos(int chip_id, int inst_id) {
   cap_nx_eos_cnt(chip_id, inst_id);
   cap_nx_eos_int(chip_id, inst_id);
   cap_nx_eos_sta(chip_id, inst_id);
}

void cap_nx_eos_cnt(int chip_id, int inst_id) {
   cap_nx_display_aw_ar_cnt(); // display all counters
}

void cap_nx_eos_int(int chip_id, int inst_id) {

   nx_err_check_template(RBM_BRIDGE_(RBM_M_58_744_TXE) , "RBM_M_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(RBM_M_58_744_RXE) , "RBM_M_RXE", 0);
   nx_err_check_template(RBM_BRIDGE_(AR_M_0_809_TXE) , "AR_M_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(AR_M_0_809_RXE) , "AR_M_RXE", 0);
   nx_err_check_template(RBM_BRIDGE_(MD_M_1_138_TXE) , "MD_M_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(MD_M_1_138_RXE) , "MD_M_RXE", 0);
   nx_err_check_template(RBM_BRIDGE_(MS_M_2_774_TXE) , "MS_M_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(MS_M_2_774_RXE) , "MS_M_RXE", 0);
   nx_err_check_template(RBM_BRIDGE_(MS_S_56_774_TXE) , "MS_S_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(MS_S_56_774_RXE) , "MS_S_RXE", 0);
   nx_err_check_template(RBM_BRIDGE_(PB_M_3_150_TXE) , "PB_M_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(PB_M_3_150_RXE) , "PB_M_RXE", 0);
   nx_err_check_template(RBM_BRIDGE_(PR_M_4_141_TXE) , "PR_M_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(PR_M_4_141_RXE) , "PR_M_RXE", 0);
   nx_err_check_template(RBM_BRIDGE_(PT_M_5_136_TXE) , "PT_M_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(PT_M_5_136_RXE) , "PT_M_RXE", 0);
   nx_err_check_template(RBM_BRIDGE_(PX_M_6_786_TXE) , "PX_M_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(PX_M_6_786_RXE) , "PX_M_RXE", 0);
   nx_err_check_template(RBM_BRIDGE_(PX_S_57_786_TXE) , "PX_S_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(PX_S_57_786_RXE) , "PX_S_RXE", 0);
   nx_err_check_template(RBM_BRIDGE_(SS_M_7_396_TXE) , "SS_M_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(SS_M_7_396_RXE) , "SS_M_RXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC0_S_24_264_TXE) , "CCC0_S_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC0_S_24_264_RXE) , "CCC0_S_RXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC1_S_25_296_TXE) , "CCC1_S_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC1_S_25_296_RXE) , "CCC1_S_RXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC10_S_26_584_TXE) , "CCC10_S_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC10_S_26_584_RXE) , "CCC10_S_RXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC11_S_27_616_TXE) , "CCC11_S_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC11_S_27_616_RXE) , "CCC11_S_RXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC12_S_28_648_TXE) , "CCC12_S_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC12_S_28_648_RXE) , "CCC12_S_RXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC13_S_29_680_TXE) , "CCC13_S_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC13_S_29_680_RXE) , "CCC13_S_RXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC14_S_30_712_TXE) , "CCC14_S_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC14_S_30_712_RXE) , "CCC14_S_RXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC15_S_31_744_TXE) , "CCC15_S_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC15_S_31_744_RXE) , "CCC15_S_RXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC2_S_32_328_TXE) , "CCC2_S_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC2_S_32_328_RXE) , "CCC2_S_RXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC3_S_33_360_TXE) , "CCC3_S_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC3_S_33_360_RXE) , "CCC3_S_RXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC4_S_34_392_TXE) , "CCC4_S_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC4_S_34_392_RXE) , "CCC4_S_RXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC5_S_35_424_TXE) , "CCC5_S_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC5_S_35_424_RXE) , "CCC5_S_RXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC6_S_36_456_TXE) , "CCC6_S_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC6_S_36_456_RXE) , "CCC6_S_RXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC7_S_37_488_TXE) , "CCC7_S_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC7_S_37_488_RXE) , "CCC7_S_RXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC8_S_38_520_TXE) , "CCC8_S_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC8_S_38_520_RXE) , "CCC8_S_RXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC9_S_39_552_TXE) , "CCC9_S_TXE", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC9_S_39_552_RXE) , "CCC9_S_RXE", 0);

   nx_err_check_template(RBM_BRIDGE_(RBM_M_58_744_AM_ERR) , "RBM_M_AM_ERR", 0);
   nx_err_check_template(RBM_BRIDGE_(AR_M_0_809_AM_ERR) , "AR_M_AM_ERR", 0);
   nx_err_check_template(RBM_BRIDGE_(MD_M_1_138_AM_ERR) , "MD_M_AM_ERR", 0);
   nx_err_check_template(RBM_BRIDGE_(MS_M_2_774_AM_ERR) , "MS_M_AM_ERR", 0);
   nx_err_check_template(RBM_BRIDGE_(PB_M_3_150_AM_ERR) , "PB_M_AM_ERR", 0);
   nx_err_check_template(RBM_BRIDGE_(PR_M_4_141_AM_ERR) , "PR_M_AM_ERR", 0);
   nx_err_check_template(RBM_BRIDGE_(PT_M_5_136_AM_ERR) , "PT_M_AM_ERR", 0);
   nx_err_check_template(RBM_BRIDGE_(PX_M_6_786_AM_ERR) , "PX_M_AM_ERR", 0);
   nx_err_check_template(RBM_BRIDGE_(SS_M_7_396_AM_ERR) , "SS_M_AM_ERR", 0);

   nx_err_check_template(RBM_BRIDGE_(MS_S_56_774_AS_ERR) , "MS_S_AS_ERR", 0);
   nx_err_check_template(RBM_BRIDGE_(PX_S_57_786_AS_ERR) , "PX_S_AS_ERR", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC0_S_24_264_AS_ERR) , "CCC0_S_AS_ERR", 0x80010);
   nx_err_check_template(RBM_BRIDGE_(CCC1_S_25_296_AS_ERR) , "CCC1_S_AS_ERR", 0x80010);
   nx_err_check_template(RBM_BRIDGE_(CCC10_S_26_584_AS_ERR) , "CCC10_S_AS_ERR", 0x80010);
   nx_err_check_template(RBM_BRIDGE_(CCC11_S_27_616_AS_ERR) , "CCC11_S_AS_ERR", 0x80010);
   nx_err_check_template(RBM_BRIDGE_(CCC12_S_28_648_AS_ERR) , "CCC12_S_AS_ERR", 0x80010);
   nx_err_check_template(RBM_BRIDGE_(CCC13_S_29_680_AS_ERR) , "CCC13_S_AS_ERR", 0x80010);
   nx_err_check_template(RBM_BRIDGE_(CCC14_S_30_712_AS_ERR) , "CCC14_S_AS_ERR", 0x80010);
   nx_err_check_template(RBM_BRIDGE_(CCC15_S_31_744_AS_ERR) , "CCC15_S_AS_ERR", 0x80010);
   nx_err_check_template(RBM_BRIDGE_(CCC2_S_32_328_AS_ERR) , "CCC2_S_AS_ERR", 0x80010);
   nx_err_check_template(RBM_BRIDGE_(CCC3_S_33_360_AS_ERR) , "CCC3_S_AS_ERR", 0x80010);
   nx_err_check_template(RBM_BRIDGE_(CCC4_S_34_392_AS_ERR) , "CCC4_S_AS_ERR", 0x80010);
   nx_err_check_template(RBM_BRIDGE_(CCC5_S_35_424_AS_ERR) , "CCC5_S_AS_ERR", 0x80010);
   nx_err_check_template(RBM_BRIDGE_(CCC6_S_36_456_AS_ERR) , "CCC6_S_AS_ERR", 0x80010);
   nx_err_check_template(RBM_BRIDGE_(CCC7_S_37_488_AS_ERR) , "CCC7_S_AS_ERR", 0x80010);
   nx_err_check_template(RBM_BRIDGE_(CCC8_S_38_520_AS_ERR) , "CCC8_S_AS_ERR", 0x80010);
   nx_err_check_template(RBM_BRIDGE_(CCC9_S_39_552_AS_ERR) , "CCC9_S_AS_ERR", 0x80010);

   nx_err_check_template(RBM_BRIDGE_(RBM_M_58_744_BRPERR0) , "RBM_M_BRPERR0", 0);
   nx_err_check_template(RBM_BRIDGE_(AR_M_0_809_BRPERR0) , "AR_M_BRPERR0", 0);
   nx_err_check_template(RBM_BRIDGE_(MD_M_1_138_BRPERR0) , "MD_M_BRPERR0", 0);
   nx_err_check_template(RBM_BRIDGE_(MS_M_2_774_BRPERR0) , "MS_M_BRPERR0", 0);
   nx_err_check_template(RBM_BRIDGE_(MS_S_56_774_BRPERR0) , "MS_S_BRPERR0", 0);
   nx_err_check_template(RBM_BRIDGE_(PB_M_3_150_BRPERR0) , "PB_M_BRPERR0", 0);
   nx_err_check_template(RBM_BRIDGE_(PR_M_4_141_BRPERR0) , "PR_M_BRPERR0", 0);
   nx_err_check_template(RBM_BRIDGE_(PT_M_5_136_BRPERR0) , "PT_M_BRPERR0", 0);
   nx_err_check_template(RBM_BRIDGE_(PX_M_6_786_BRPERR0) , "PX_M_BRPERR0", 0);
   nx_err_check_template(RBM_BRIDGE_(PX_S_57_786_BRPERR0) , "PX_S_BRPERR0", 0);
   nx_err_check_template(RBM_BRIDGE_(SS_M_7_396_BRPERR0) , "SS_M_BRPERR0", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC0_S_24_264_BRPERR0) , "CCC0_S_BRPERR0", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC1_S_25_296_BRPERR0) , "CCC1_S_BRPERR0", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC10_S_26_584_BRPERR0) , "CCC10_S_BRPERR0", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC11_S_27_616_BRPERR0) , "CCC11_S_BRPERR0", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC12_S_28_648_BRPERR0) , "CCC12_S_BRPERR0", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC13_S_29_680_BRPERR0) , "CCC13_S_BRPERR0", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC14_S_30_712_BRPERR0) , "CCC14_S_BRPERR0", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC15_S_31_744_BRPERR0) , "CCC15_S_BRPERR0", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC2_S_32_328_BRPERR0) , "CCC2_S_BRPERR0", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC3_S_33_360_BRPERR0) , "CCC3_S_BRPERR0", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC4_S_34_392_BRPERR0) , "CCC4_S_BRPERR0", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC5_S_35_424_BRPERR0) , "CCC5_S_BRPERR0", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC6_S_36_456_BRPERR0) , "CCC6_S_BRPERR0", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC7_S_37_488_BRPERR0) , "CCC7_S_BRPERR0", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC8_S_38_520_BRPERR0) , "CCC8_S_BRPERR0", 0);
   nx_err_check_template(RBM_BRIDGE_(CCC9_S_39_552_BRPERR0) , "CCC9_S_BRPERR0", 0);

}

void cap_nx_eos_sta(int chip_id, int inst_id) {

   nx_err_check_template( RBM_BRIDGE_(RBM_M_58_744_AM_STS) , "RBM_M_AM_STS", 0xc);
   nx_err_check_template( RBM_BRIDGE_(AR_M_0_809_AM_STS) , "AR_M_AM_STS", 0xc);
   nx_err_check_template( RBM_BRIDGE_(MD_M_1_138_AM_STS) , "MD_M_AM_STS", 0xc);
   nx_err_check_template( RBM_BRIDGE_(MS_M_2_774_AM_STS) , "MS_M_AM_STS", 0xc);
   nx_err_check_template( RBM_BRIDGE_(PB_M_3_150_AM_STS) , "PB_M_AM_STS", 0xc);
   nx_err_check_template( RBM_BRIDGE_(PR_M_4_141_AM_STS) , "PR_M_AM_STS", 0xc);
   nx_err_check_template( RBM_BRIDGE_(PT_M_5_136_AM_STS) , "PT_M_AM_STS", 0xc);
   nx_err_check_template( RBM_BRIDGE_(PX_M_6_786_AM_STS) , "PX_M_AM_STS", 0xc);
   nx_err_check_template( RBM_BRIDGE_(SS_M_7_396_AM_STS) , "SS_M_AM_STS", 0xc);

   nx_err_check_template( RBM_BRIDGE_(MS_S_56_774_AS_STS) , "MS_S_AS_STS", 0xc);
   nx_err_check_template( RBM_BRIDGE_(PX_S_57_786_AS_STS) , "PX_S_AS_STS", 0xc);
   nx_err_check_template( RBM_BRIDGE_(CCC0_S_24_264_AS_STS) , "CCC0_S_AS_STS", 0xc);
   nx_err_check_template( RBM_BRIDGE_(CCC1_S_25_296_AS_STS) , "CCC1_S_AS_STS", 0xc);
   nx_err_check_template( RBM_BRIDGE_(CCC10_S_26_584_AS_STS) , "CCC10_S_AS_STS", 0xc);
   nx_err_check_template( RBM_BRIDGE_(CCC11_S_27_616_AS_STS) , "CCC11_S_AS_STS", 0xc);
   nx_err_check_template( RBM_BRIDGE_(CCC12_S_28_648_AS_STS) , "CCC12_S_AS_STS", 0xc);
   nx_err_check_template( RBM_BRIDGE_(CCC13_S_29_680_AS_STS) , "CCC13_S_AS_STS", 0xc);
   nx_err_check_template( RBM_BRIDGE_(CCC14_S_30_712_AS_STS) , "CCC14_S_AS_STS", 0xc);
   nx_err_check_template( RBM_BRIDGE_(CCC15_S_31_744_AS_STS) , "CCC15_S_AS_STS", 0xc);
   nx_err_check_template( RBM_BRIDGE_(CCC2_S_32_328_AS_STS) , "CCC2_S_AS_STS", 0xc);
   nx_err_check_template( RBM_BRIDGE_(CCC3_S_33_360_AS_STS) , "CCC3_S_AS_STS", 0xc);
   nx_err_check_template( RBM_BRIDGE_(CCC4_S_34_392_AS_STS) , "CCC4_S_AS_STS", 0xc);
   nx_err_check_template( RBM_BRIDGE_(CCC5_S_35_424_AS_STS) , "CCC5_S_AS_STS", 0xc);
   nx_err_check_template( RBM_BRIDGE_(CCC6_S_36_456_AS_STS) , "CCC6_S_AS_STS", 0xc);
   nx_err_check_template( RBM_BRIDGE_(CCC7_S_37_488_AS_STS) , "CCC7_S_AS_STS", 0xc);
   nx_err_check_template( RBM_BRIDGE_(CCC8_S_38_520_AS_STS) , "CCC8_S_AS_STS", 0xc);
   nx_err_check_template( RBM_BRIDGE_(CCC9_S_39_552_AS_STS) , "CCC9_S_AS_STS", 0xc);
}

// Bridge Error Interrupt Registers
void nx_err_check_template (uint64_t rd_addr, const char *reg_name, uint32_t err_cmp_val) {

//vector<uint32_t> read_vector;
  uint32_t read_vector[2];
  read_vector[0] = cap_nx_block_read(0, rd_addr, true, 1, 1);
  for (uint32_t i=0; i < 1 ; i++) {
     read_vector[i] = read_vector[i] & ~(err_cmp_val); // mask out unnecessary bits
     if (read_vector[i] != 0) {
        PLOG_ERR("NX:: EOS:: interrupt-non-zero for reg:" << reg_name << ": 0x" << hex << read_vector[i] << dec << endl);
     }
  }
}
#endif

#ifndef CAPRI_SW
void cap_nx_setup_llc_counters(uint32_t event_mask) {

//vector<uint32_t> data;
uint32_t data[2];
/*
e9[9] - Retry access
e8[8] - Retry needed
e7[7] - Eviction
e6[6] - Cache maint op
e5[5] - Partial write
e4[4] - Cache miss
e3[3] - Cache hit
e2[2] - Scratchpad access
e1[1] - Cache write
e0[0] - Cache read
*/
   //data.resize(1);
   data[0] = event_mask;
   cap_nx_block_write(0, RBM_AGENT_(CCC0_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC1_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC10_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC11_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC12_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC13_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC14_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC15_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC2_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC3_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC4_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC5_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC6_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC7_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC8_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC9_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);

}

void cap_nx_setup_ccc_counters(uint32_t event_mask) {

//vector<uint32_t> data;
uint32_t data[2];
/*
e9[9] - Retry access
e8[8] - Retry needed
e7[7] - Eviction
e6[6] - Cache maint op
e5[5] - Partial write
e4[4] - Cache miss
e3[3] - Cache hit
e2[2] - Scratchpad access
e1[1] - Cache write
e0[0] - Cache read
*/
   //data.resize(1);
   data[0] = event_mask;
   cap_nx_block_write(0, RBM_AGENT_(CCC0_CCC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC1_CCC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC10_CCC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC11_CCC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC12_CCC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC13_CCC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC14_CCC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC15_CCC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC2_CCC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC3_CCC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC4_CCC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC5_CCC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC6_CCC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC7_CCC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC8_CCC_EVENT_COUNTER_MASK), 1, data, true, 1);
   cap_nx_block_write(0, RBM_AGENT_(CCC9_CCC_EVENT_COUNTER_MASK), 1, data, true, 1);

}
#endif

#ifndef CAPRI_SW
void cap_nx_display_ccc_counters() {

//vector<uint32_t> rd_data;
uint32_t rd_data[2];

   //rd_data.resize(1);

   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC0_CCC_EVENT_COUNTER_VALUE), 1, false, 1);
   PLOG_API_MSG("NX:", " CCC0_CCC_EventCount: 0x" << rd_data[0] << "\n");
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC1_CCC_EVENT_COUNTER_VALUE), 1, false, 1);
   PLOG_API_MSG("NX:", " CCC1_CCC_EventCount: 0x" << rd_data[0] << "\n");
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC2_CCC_EVENT_COUNTER_VALUE), 1, false, 1);
   PLOG_API_MSG("NX:", " CCC2_CCC_EventCount: 0x" << rd_data[0] << "\n");
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC3_CCC_EVENT_COUNTER_VALUE), 1, false, 1);
   PLOG_API_MSG("NX:", " CCC3_CCC_EventCount: 0x" << rd_data[0] << "\n");
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC4_CCC_EVENT_COUNTER_VALUE), 1, false, 1);
   PLOG_API_MSG("NX:", " CCC4_CCC_EventCount: 0x" << rd_data[0] << "\n");
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC5_CCC_EVENT_COUNTER_VALUE), 1, false, 1);
   PLOG_API_MSG("NX:", " CCC5_CCC_EventCount: 0x" << rd_data[0] << "\n");
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC6_CCC_EVENT_COUNTER_VALUE), 1, false, 1);
   PLOG_API_MSG("NX:", " CCC6_CCC_EventCount: 0x" << rd_data[0] << "\n");
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC7_CCC_EVENT_COUNTER_VALUE), 1, false, 1);
   PLOG_API_MSG("NX:", " CCC7_CCC_EventCount: 0x" << rd_data[0] << "\n");
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC8_CCC_EVENT_COUNTER_VALUE), 1, false, 1);
   PLOG_API_MSG("NX:", " CCC8_CCC_EventCount: 0x" << rd_data[0] << "\n");
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC9_CCC_EVENT_COUNTER_VALUE), 1, false, 1);
   PLOG_API_MSG("NX:", " CCC9_CCC_EventCount: 0x" << rd_data[0] << "\n");
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC10_CCC_EVENT_COUNTER_VALUE), 1, false, 1);
   PLOG_API_MSG("NX:", " CCC10_CCC_EventCount: 0x" << rd_data[0] << "\n");
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC11_CCC_EVENT_COUNTER_VALUE), 1, false, 1);
   PLOG_API_MSG("NX:", " CCC11_CCC_EventCount: 0x" << rd_data[0] << "\n");
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC12_CCC_EVENT_COUNTER_VALUE), 1, false, 1);
   PLOG_API_MSG("NX:", " CCC12_CCC_EventCount: 0x" << rd_data[0] << "\n");
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC13_CCC_EVENT_COUNTER_VALUE), 1, false, 1);
   PLOG_API_MSG("NX:", " CCC13_CCC_EventCount: 0x" << rd_data[0] << "\n");
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC14_CCC_EVENT_COUNTER_VALUE), 1, false, 1);
   PLOG_API_MSG("NX:", " CCC14_CCC_EventCount: 0x" << rd_data[0] << "\n");
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC15_CCC_EVENT_COUNTER_VALUE), 1, false, 1);
   PLOG_API_MSG("NX:", " CCC15_CCC_EventCount: 0x" << rd_data[0] << "\n");
}


void cap_nx_setup_am_counters(uint32_t count_lat) {

//vector<uint32_t> data;
uint32_t data[2];
/*
CCMD0 0x2300000 (AR)
CCMD1 0x2700000 (AW)
CCMDMSK0 0x300000
CCMDMSK1 0x300000
*/

   //data.resize(1); 

   data[0] = 0x0;
   cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_CADDR), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_CADDR), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_CADDR), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_CADDR), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_CADDR), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_CADDR), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_CADDR), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_CADDR), 1, data, true, 1);

   cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_CADDR)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_CADDR)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_CADDR)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_CADDR)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_CADDR)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_CADDR)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_CADDR)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_CADDR)+4, 1, data, true, 1);

   data[0] = 0x3000000;

   cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_CCMD0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_CCMD0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_CCMD0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_CCMD0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_CCMD0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_CCMD0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_CCMD0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_CCMD0), 1, data, true, 1);
   data[0] = 0x13000000;
   cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_CCMD1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_CCMD1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_CCMD1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_CCMD1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_CCMD1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_CCMD1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_CCMD1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_CCMD1), 1, data, true, 1);

   data[0] = 0x3000000;
   cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_CCMDMSK0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_CCMDMSK0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_CCMDMSK0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_CCMDMSK0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_CCMDMSK0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_CCMDMSK0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_CCMDMSK0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_CCMDMSK0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_CCMDMSK1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_CCMDMSK1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_CCMDMSK1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_CCMDMSK1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_CCMDMSK1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_CCMDMSK1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_CCMDMSK1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_CCMDMSK1), 1, data, true, 1);

   data[0] = count_lat;
   cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_CCMD0)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_CCMD0)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_CCMD0)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_CCMD0)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_CCMD0)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_CCMD0)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_CCMD0)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_CCMD0)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_CCMD1)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_CCMD1)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_CCMD1)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_CCMD1)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_CCMD1)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_CCMD1)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_CCMD1)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_CCMD1)+4, 1, data, true, 1);

}

void cap_nx_setup_mbridge_counters(uint64_t caddr, uint64_t caddr_msk, uint64_t ccmd0, uint32_t ccmd0_msk, uint64_t ccmd1, uint32_t ccmd1_msk) {

uint32_t data[2];

   data[0] = (caddr & 0xffffffff);
   cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_CADDR), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_CADDR), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_CADDR), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_CADDR), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_CADDR), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_CADDR), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_CADDR), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_CADDR), 1, data, true, 1);

   data[0] = (caddr >> 32);
   cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_CADDR)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_CADDR)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_CADDR)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_CADDR)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_CADDR)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_CADDR)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_CADDR)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_CADDR)+4, 1, data, true, 1);

   data[0] = (ccmd0 & 0xffffffff);
   cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_CCMD0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_CCMD0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_CCMD0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_CCMD0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_CCMD0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_CCMD0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_CCMD0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_CCMD0), 1, data, true, 1);

   data[0] = (ccmd1 & 0xffffffff);
   cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_CCMD1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_CCMD1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_CCMD1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_CCMD1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_CCMD1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_CCMD1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_CCMD1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_CCMD1), 1, data, true, 1);

   data[0] = ccmd0_msk;
   cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_CCMDMSK0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_CCMDMSK0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_CCMDMSK0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_CCMDMSK0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_CCMDMSK0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_CCMDMSK0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_CCMDMSK0), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_CCMDMSK0), 1, data, true, 1);

   data[0] = ccmd1_msk;
   cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_CCMDMSK1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_CCMDMSK1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_CCMDMSK1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_CCMDMSK1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_CCMDMSK1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_CCMDMSK1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_CCMDMSK1), 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_CCMDMSK1), 1, data, true, 1);

   data[0] = (ccmd0 >> 32);
   cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_CCMD0)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_CCMD0)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_CCMD0)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_CCMD0)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_CCMD0)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_CCMD0)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_CCMD0)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_CCMD0)+4, 1, data, true, 1);

   data[0] = (ccmd1 >> 32);
   cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_CCMD1)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_CCMD1)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_CCMD1)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_CCMD1)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_CCMD1)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_CCMD1)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_CCMD1)+4, 1, data, true, 1);
   cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_CCMD1)+4, 1, data, true, 1);

}
#endif

void cap_nx_add_secure_md_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) {

//vector<uint32_t> data0;
//vector<uint32_t> data1;
uint32_t data0[2];
uint32_t data1[2];

   //data0.resize(1); 
   //data1.resize(1); 

   data0[0] = (adbase & 0xfffffffful) ;
   data1[0] = (admask & 0xfffffffful) ;
   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S0_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S0_0), 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S1_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S1_0), 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S2_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S2_0), 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S3_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S3_0), 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S4_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S4_0), 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S5_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S5_0), 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S6_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S6_0), 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S7_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S7_0), 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S8_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S8_0), 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S9_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S9_0), 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S10_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S10_0), 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S11_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S11_0), 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S12_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S12_0), 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S13_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S13_0), 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S14_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S14_0), 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S15_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S15_0), 1, data1, true, 1);
   } 

   data0[0] = ((adbase & 0xff00000000ul) >> 32 );
   data1[0] = ((admask & 0xff00000000ul) >> 32 );

   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S0_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S0_0)+4, 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S1_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S1_0)+4, 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S2_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S2_0)+4, 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S3_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S3_0)+4, 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S4_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S4_0)+4, 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S5_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S5_0)+4, 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S6_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S6_0)+4, 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S7_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S7_0)+4, 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S8_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S8_0)+4, 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S9_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S9_0)+4, 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S10_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S10_0)+4, 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S11_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S11_0)+4, 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S12_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S12_0)+4, 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S13_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S13_0)+4, 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S14_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S14_0)+4, 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S15_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S15_0)+4, 1, data1, true, 1);
   } 
}

void cap_nx_add_secure_pt_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) {

//vector<uint32_t> data0;
//vector<uint32_t> data1;
uint32_t data0[2];
uint32_t data1[2];

   //data0.resize(1); 
   //data1.resize(1); 

   data0[0] = (adbase & 0xfffffffful) ;
   data1[0] = (admask & 0xfffffffful) ;
   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S0_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S0_0), 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S1_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S1_0), 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S2_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S2_0), 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S3_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S3_0), 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S4_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S4_0), 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S5_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S5_0), 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S6_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S6_0), 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S7_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S7_0), 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S8_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S8_0), 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S9_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S9_0), 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S10_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S10_0), 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S11_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S11_0), 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S12_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S12_0), 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S13_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S13_0), 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S14_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S14_0), 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S15_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S15_0), 1, data1, true, 1);
   } 

   data0[0] = ((adbase & 0xff00000000ul) >> 32 );
   data1[0] = ((admask & 0xff00000000ul) >> 32 );

   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S0_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S0_0)+4, 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S1_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S1_0)+4, 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S2_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S2_0)+4, 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S3_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S3_0)+4, 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S4_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S4_0)+4, 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S5_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S5_0)+4, 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S6_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S6_0)+4, 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S7_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S7_0)+4, 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S8_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S8_0)+4, 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S9_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S9_0)+4, 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S10_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S10_0)+4, 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S11_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S11_0)+4, 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S12_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S12_0)+4, 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S13_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S13_0)+4, 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S14_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S14_0)+4, 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S15_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S15_0)+4, 1, data1, true, 1);
   } 
}

void cap_nx_add_secure_pr_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) {

//vector<uint32_t> data0;
//vector<uint32_t> data1;
uint32_t data0[2];
uint32_t data1[2];

   //data0.resize(1); 
   //data1.resize(1); 

   data0[0] = (adbase & 0xfffffffful) ;
   data1[0] = (admask & 0xfffffffful) ;
   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S0_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S0_0), 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S1_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S1_0), 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S2_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S2_0), 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S3_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S3_0), 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S4_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S4_0), 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S5_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S5_0), 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S6_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S6_0), 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S7_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S7_0), 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S8_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S8_0), 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S9_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S9_0), 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S10_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S10_0), 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S11_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S11_0), 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S12_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S12_0), 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S13_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S13_0), 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S14_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S14_0), 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S15_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S15_0), 1, data1, true, 1);
   } 

   data0[0] = ((adbase & 0xff00000000ul) >> 32 );
   data1[0] = ((admask & 0xff00000000ul) >> 32 );

   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S0_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S0_0)+4, 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S1_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S1_0)+4, 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S2_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S2_0)+4, 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S3_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S3_0)+4, 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S4_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S4_0)+4, 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S5_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S5_0)+4, 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S6_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S6_0)+4, 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S7_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S7_0)+4, 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S8_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S8_0)+4, 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S9_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S9_0)+4, 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S10_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S10_0)+4, 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S11_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S11_0)+4, 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S12_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S12_0)+4, 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S13_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S13_0)+4, 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S14_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S14_0)+4, 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S15_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S15_0)+4, 1, data1, true, 1);
   } 
}

void cap_nx_add_secure_ar_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) {

//vector<uint32_t> data0;
//vector<uint32_t> data1;
uint32_t data0[2];
uint32_t data1[2];

   //data0.resize(1); 
   //data1.resize(1); 

   data0[0] = (adbase & 0xfffffffful) ;
   data1[0] = (admask & 0xfffffffful) ;
   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S0_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S0_0), 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S1_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S1_0), 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S2_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S2_0), 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S3_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S3_0), 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S4_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S4_0), 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S5_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S5_0), 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S6_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S6_0), 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S7_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S7_0), 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S8_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S8_0), 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S9_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S9_0), 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S10_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S10_0), 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S11_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S11_0), 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S12_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S12_0), 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S13_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S13_0), 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S14_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S14_0), 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S15_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S15_0), 1, data1, true, 1);
   } 

   data0[0] = ((adbase & 0xff00000000ul) >> 32 );
   data1[0] = ((admask & 0xff00000000ul) >> 32 );

   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S0_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S0_0)+4, 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S1_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S1_0)+4, 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S2_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S2_0)+4, 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S3_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S3_0)+4, 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S4_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S4_0)+4, 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S5_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S5_0)+4, 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S6_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S6_0)+4, 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S7_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S7_0)+4, 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S8_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S8_0)+4, 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S9_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S9_0)+4, 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S10_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S10_0)+4, 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S11_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S11_0)+4, 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S12_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S12_0)+4, 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S13_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S13_0)+4, 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S14_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S14_0)+4, 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S15_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S15_0)+4, 1, data1, true, 1);
   } 
}


void cap_nx_add_secure_pb_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) {

//vector<uint32_t> data0;
//vector<uint32_t> data1;
uint32_t data0[2];
uint32_t data1[2];

   //data0.resize(1); 
   //data1.resize(1); 

   data0[0] = (adbase & 0xfffffffful) ;
   data1[0] = (admask & 0xfffffffful) ;
   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S0_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S0_0), 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S1_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S1_0), 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S2_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S2_0), 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S3_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S3_0), 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S4_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S4_0), 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S5_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S5_0), 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S6_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S6_0), 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S7_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S7_0), 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S8_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S8_0), 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S9_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S9_0), 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S10_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S10_0), 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S11_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S11_0), 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S12_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S12_0), 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S13_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S13_0), 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S14_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S14_0), 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S15_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S15_0), 1, data1, true, 1);
   } 

   data0[0] = ((adbase & 0xff00000000ul) >> 32 );
   data1[0] = ((admask & 0xff00000000ul) >> 32 );

   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S0_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S0_0)+4, 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S1_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S1_0)+4, 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S2_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S2_0)+4, 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S3_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S3_0)+4, 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S4_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S4_0)+4, 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S5_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S5_0)+4, 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S6_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S6_0)+4, 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S7_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S7_0)+4, 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S8_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S8_0)+4, 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S9_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S9_0)+4, 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S10_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S10_0)+4, 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S11_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S11_0)+4, 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S12_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S12_0)+4, 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S13_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S13_0)+4, 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S14_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S14_0)+4, 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S15_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S15_0)+4, 1, data1, true, 1);
   } 
}


void cap_nx_add_secure_ss_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) {

//vector<uint32_t> data0;
//vector<uint32_t> data1;
uint32_t data0[2];
uint32_t data1[2];

   //data0.resize(1); 
   //data1.resize(1); 

   data0[0] = (adbase & 0xfffffffful) ;
   data1[0] = (admask & 0xfffffffful) ;
   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S0_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S0_0), 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S1_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S1_0), 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S2_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S2_0), 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S3_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S3_0), 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S4_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S4_0), 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S5_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S5_0), 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S6_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S6_0), 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S7_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S7_0), 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S8_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S8_0), 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S9_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S9_0), 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S10_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S10_0), 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S11_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S11_0), 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S12_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S12_0), 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S13_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S13_0), 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S14_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S14_0), 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S15_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S15_0), 1, data1, true, 1);
   } 

   data0[0] = ((adbase & 0xff00000000ul) >> 32 );
   data1[0] = ((admask & 0xff00000000ul) >> 32 );

   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S0_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S0_0)+4, 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S1_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S1_0)+4, 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S2_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S2_0)+4, 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S3_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S3_0)+4, 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S4_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S4_0)+4, 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S5_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S5_0)+4, 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S6_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S6_0)+4, 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S7_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S7_0)+4, 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S8_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S8_0)+4, 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S9_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S9_0)+4, 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S10_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S10_0)+4, 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S11_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S11_0)+4, 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S12_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S12_0)+4, 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S13_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S13_0)+4, 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S14_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S14_0)+4, 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S15_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S15_0)+4, 1, data1, true, 1);
   } 
}


void cap_nx_add_secure_ms_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) {

//vector<uint32_t> data0;
//vector<uint32_t> data1;
uint32_t data0[2];
uint32_t data1[2];

   //data0.resize(1); 
   //data1.resize(1); 

   data0[0] = (adbase & 0xfffffffful) ;
   data1[0] = (admask & 0xfffffffful) ;
   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S0_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S0_0), 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S1_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S1_0), 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S2_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S2_0), 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S3_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S3_0), 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S4_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S4_0), 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S5_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S5_0), 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S6_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S6_0), 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S7_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S7_0), 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S8_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S8_0), 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S9_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S9_0), 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S10_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S10_0), 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S11_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S11_0), 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S12_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S12_0), 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S13_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S13_0), 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S14_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S14_0), 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S15_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S15_0), 1, data1, true, 1);
   } 

   data0[0] = ((adbase & 0xff00000000ul) >> 32 );
   data1[0] = ((admask & 0xff00000000ul) >> 32 );

   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S0_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S0_0)+4, 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S1_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S1_0)+4, 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S2_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S2_0)+4, 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S3_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S3_0)+4, 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S4_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S4_0)+4, 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S5_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S5_0)+4, 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S6_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S6_0)+4, 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S7_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S7_0)+4, 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S8_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S8_0)+4, 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S9_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S9_0)+4, 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S10_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S10_0)+4, 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S11_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S11_0)+4, 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S12_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S12_0)+4, 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S13_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S13_0)+4, 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S14_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S14_0)+4, 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S15_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S15_0)+4, 1, data1, true, 1);
   } 
}

void cap_nx_add_secure_px_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) {

//vector<uint32_t> data0;
//vector<uint32_t> data1;
uint32_t data0[2];
uint32_t data1[2];

   //data0.resize(1); 
   //data1.resize(1); 

   data0[0] = (adbase & 0xfffffffful) ;
   data1[0] = (admask & 0xfffffffful) ;
   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S0_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S0_0), 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S1_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S1_0), 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S2_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S2_0), 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S3_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S3_0), 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S4_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S4_0), 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S5_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S5_0), 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S6_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S6_0), 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S7_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S7_0), 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S8_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S8_0), 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S9_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S9_0), 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S10_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S10_0), 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S11_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S11_0), 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S12_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S12_0), 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S13_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S13_0), 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S14_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S14_0), 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S15_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S15_0), 1, data1, true, 1);
   } 

   data0[0] = ((adbase & 0xff00000000ul) >> 32 );
   data1[0] = ((admask & 0xff00000000ul) >> 32 );

   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S0_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S0_0)+4, 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S1_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S1_0)+4, 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S2_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S2_0)+4, 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S3_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S3_0)+4, 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S4_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S4_0)+4, 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S5_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S5_0)+4, 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S6_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S6_0)+4, 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S7_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S7_0)+4, 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S8_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S8_0)+4, 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S9_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S9_0)+4, 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S10_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S10_0)+4, 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S11_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S11_0)+4, 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S12_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S12_0)+4, 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S13_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S13_0)+4, 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S14_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S14_0)+4, 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S15_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S15_0)+4, 1, data1, true, 1);
   } 
}

void cap_nx_add_noncoh_secure_md_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) {

//vector<uint32_t> data0;
//vector<uint32_t> data1;
uint32_t data0[2];
uint32_t data1[2];

   //data0.resize(1); 
   //data1.resize(1); 

   data0[0] = (adbase & 0xfffffffful) ;
   data1[0] = (admask & 0xfffffffful) ;
   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS0_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS0_0), 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS1_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS1_0), 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS2_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS2_0), 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS3_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS3_0), 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS4_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS4_0), 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS5_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS5_0), 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS6_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS6_0), 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS7_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS7_0), 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS8_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS8_0), 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS9_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS9_0), 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSA_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSA_0), 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSB_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSB_0), 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSC_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSC_0), 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSD_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSD_0), 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSE_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSE_0), 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSF_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSF_0), 1, data1, true, 1);
   } 

   data0[0] = ((adbase & 0xff00000000ul) >> 32 );
   data1[0] = ((admask & 0xff00000000ul) >> 32 );

   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS0_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS0_0)+4, 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS1_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS1_0)+4, 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS2_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS2_0)+4, 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS3_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS3_0)+4, 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS4_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS4_0)+4, 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS5_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS5_0)+4, 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS6_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS6_0)+4, 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS7_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS7_0)+4, 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS8_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS8_0)+4, 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS9_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS9_0)+4, 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSA_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSA_0)+4, 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSB_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSB_0)+4, 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSC_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSC_0)+4, 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSD_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSD_0)+4, 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSE_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSE_0)+4, 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSF_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSF_0)+4, 1, data1, true, 1);
   } 
}

void cap_nx_add_noncoh_secure_pt_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) {

//vector<uint32_t> data0;
//vector<uint32_t> data1;
uint32_t data0[2];
uint32_t data1[2];

   //data0.resize(1); 
   //data1.resize(1); 

   data0[0] = (adbase & 0xfffffffful) ;
   data1[0] = (admask & 0xfffffffful) ;
   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS0_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS0_0), 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS1_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS1_0), 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS2_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS2_0), 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS3_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS3_0), 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS4_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS4_0), 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS5_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS5_0), 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS6_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS6_0), 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS7_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS7_0), 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS8_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS8_0), 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS9_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS9_0), 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSA_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSA_0), 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSB_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSB_0), 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSC_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSC_0), 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSD_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSD_0), 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSE_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSE_0), 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSF_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSF_0), 1, data1, true, 1);
   } 

   data0[0] = ((adbase & 0xff00000000ul) >> 32 );
   data1[0] = ((admask & 0xff00000000ul) >> 32 );

   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS0_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS0_0)+4, 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS1_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS1_0)+4, 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS2_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS2_0)+4, 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS3_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS3_0)+4, 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS4_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS4_0)+4, 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS5_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS5_0)+4, 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS6_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS6_0)+4, 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS7_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS7_0)+4, 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS8_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS8_0)+4, 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS9_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS9_0)+4, 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSA_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSA_0)+4, 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSB_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSB_0)+4, 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSC_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSC_0)+4, 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSD_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSD_0)+4, 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSE_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSE_0)+4, 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSF_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSF_0)+4, 1, data1, true, 1);
   } 
}

void cap_nx_add_noncoh_secure_pr_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) {

//vector<uint32_t> data0;
//vector<uint32_t> data1;
uint32_t data0[2];
uint32_t data1[2];

   //data0.resize(1); 
   //data1.resize(1); 

   data0[0] = (adbase & 0xfffffffful) ;
   data1[0] = (admask & 0xfffffffful) ;
   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS0_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS0_0), 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS1_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS1_0), 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS2_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS2_0), 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS3_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS3_0), 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS4_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS4_0), 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS5_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS5_0), 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS6_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS6_0), 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS7_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS7_0), 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS8_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS8_0), 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS9_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS9_0), 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSA_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSA_0), 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSB_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSB_0), 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSC_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSC_0), 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSD_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSD_0), 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSE_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSE_0), 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSF_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSF_0), 1, data1, true, 1);
   } 

   data0[0] = ((adbase & 0xff00000000ul) >> 32 );
   data1[0] = ((admask & 0xff00000000ul) >> 32 );

   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS0_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS0_0)+4, 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS1_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS1_0)+4, 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS2_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS2_0)+4, 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS3_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS3_0)+4, 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS4_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS4_0)+4, 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS5_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS5_0)+4, 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS6_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS6_0)+4, 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS7_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS7_0)+4, 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS8_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS8_0)+4, 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS9_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS9_0)+4, 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSA_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSA_0)+4, 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSB_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSB_0)+4, 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSC_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSC_0)+4, 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSD_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSD_0)+4, 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSE_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSE_0)+4, 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSF_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSF_0)+4, 1, data1, true, 1);
   } 
}

void cap_nx_add_noncoh_secure_ar_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) {

//vector<uint32_t> data0;
//vector<uint32_t> data1;
uint32_t data0[2];
uint32_t data1[2];

   //data0.resize(1); 
   //data1.resize(1); 

   data0[0] = (adbase & 0xfffffffful) ;
   data1[0] = (admask & 0xfffffffful) ;
   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS0_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS0_0), 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS1_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS1_0), 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS2_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS2_0), 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS3_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS3_0), 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS4_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS4_0), 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS5_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS5_0), 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS6_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS6_0), 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS7_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS7_0), 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS8_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS8_0), 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS9_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS9_0), 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSA_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSA_0), 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSB_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSB_0), 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSC_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSC_0), 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSD_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSD_0), 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSE_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSE_0), 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSF_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSF_0), 1, data1, true, 1);
   } 

   data0[0] = ((adbase & 0xff00000000ul) >> 32 );
   data1[0] = ((admask & 0xff00000000ul) >> 32 );

   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS0_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS0_0)+4, 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS1_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS1_0)+4, 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS2_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS2_0)+4, 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS3_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS3_0)+4, 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS4_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS4_0)+4, 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS5_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS5_0)+4, 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS6_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS6_0)+4, 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS7_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS7_0)+4, 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS8_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS8_0)+4, 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS9_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS9_0)+4, 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSA_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSA_0)+4, 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSB_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSB_0)+4, 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSC_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSC_0)+4, 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSD_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSD_0)+4, 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSE_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSE_0)+4, 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSF_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSF_0)+4, 1, data1, true, 1);
   } 
}

void cap_nx_add_noncoh_secure_pb_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) {

//vector<uint32_t> data0;
//vector<uint32_t> data1;
uint32_t data0[2];
uint32_t data1[2];

   //data0.resize(1); 
   //data1.resize(1); 

   data0[0] = (adbase & 0xfffffffful) ;
   data1[0] = (admask & 0xfffffffful) ;
   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS0_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS0_0), 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS1_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS1_0), 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS2_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS2_0), 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS3_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS3_0), 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS4_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS4_0), 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS5_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS5_0), 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS6_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS6_0), 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS7_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS7_0), 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS8_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS8_0), 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS9_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS9_0), 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSA_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSA_0), 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSB_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSB_0), 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSC_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSC_0), 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSD_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSD_0), 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSE_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSE_0), 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSF_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSF_0), 1, data1, true, 1);
   } 

   data0[0] = ((adbase & 0xff00000000ul) >> 32 );
   data1[0] = ((admask & 0xff00000000ul) >> 32 );

   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS0_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS0_0)+4, 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS1_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS1_0)+4, 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS2_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS2_0)+4, 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS3_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS3_0)+4, 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS4_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS4_0)+4, 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS5_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS5_0)+4, 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS6_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS6_0)+4, 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS7_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS7_0)+4, 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS8_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS8_0)+4, 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS9_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS9_0)+4, 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSA_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSA_0)+4, 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSB_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSB_0)+4, 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSC_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSC_0)+4, 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSD_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSD_0)+4, 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSE_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSE_0)+4, 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSF_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSF_0)+4, 1, data1, true, 1);
   } 
}

void cap_nx_add_noncoh_secure_ss_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) {

//vector<uint32_t> data0;
//vector<uint32_t> data1;
uint32_t data0[2];
uint32_t data1[2];

   //data0.resize(1); 
   //data1.resize(1); 

   data0[0] = (adbase & 0xfffffffful) ;
   data1[0] = (admask & 0xfffffffful) ;
   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS0_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS0_0), 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS1_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS1_0), 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS2_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS2_0), 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS3_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS3_0), 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS4_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS4_0), 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS5_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS5_0), 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS6_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS6_0), 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS7_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS7_0), 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS8_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS8_0), 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS9_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS9_0), 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSA_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSA_0), 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSB_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSB_0), 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSC_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSC_0), 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSD_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSD_0), 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSE_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSE_0), 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSF_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSF_0), 1, data1, true, 1);
   } 

   data0[0] = ((adbase & 0xff00000000ul) >> 32 );
   data1[0] = ((admask & 0xff00000000ul) >> 32 );

   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS0_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS0_0)+4, 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS1_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS1_0)+4, 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS2_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS2_0)+4, 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS3_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS3_0)+4, 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS4_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS4_0)+4, 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS5_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS5_0)+4, 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS6_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS6_0)+4, 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS7_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS7_0)+4, 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS8_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS8_0)+4, 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS9_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS9_0)+4, 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSA_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSA_0)+4, 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSB_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSB_0)+4, 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSC_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSC_0)+4, 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSD_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSD_0)+4, 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSE_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSE_0)+4, 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSF_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSF_0)+4, 1, data1, true, 1);
   } 
}

void cap_nx_add_noncoh_secure_ms_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) {

//vector<uint32_t> data0;
//vector<uint32_t> data1;
uint32_t data0[2];
uint32_t data1[2];

   //data0.resize(1); 
   //data1.resize(1); 

   data0[0] = (adbase & 0xfffffffful) ;
   data1[0] = (admask & 0xfffffffful) ;
   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS0_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS0_0), 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS1_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS1_0), 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS2_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS2_0), 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS3_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS3_0), 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS4_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS4_0), 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS5_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS5_0), 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS6_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS6_0), 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS7_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS7_0), 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS8_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS8_0), 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS9_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS9_0), 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSA_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSA_0), 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSB_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSB_0), 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSC_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSC_0), 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSD_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSD_0), 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSE_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSE_0), 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSF_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSF_0), 1, data1, true, 1);
   } 

   data0[0] = ((adbase & 0xff00000000ul) >> 32 );
   data1[0] = ((admask & 0xff00000000ul) >> 32 );

   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS0_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS0_0)+4, 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS1_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS1_0)+4, 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS2_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS2_0)+4, 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS3_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS3_0)+4, 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS4_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS4_0)+4, 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS5_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS5_0)+4, 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS6_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS6_0)+4, 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS7_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS7_0)+4, 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS8_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS8_0)+4, 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS9_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS9_0)+4, 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSA_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSA_0)+4, 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSB_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSB_0)+4, 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSC_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSC_0)+4, 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSD_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSD_0)+4, 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSE_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSE_0)+4, 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSF_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSF_0)+4, 1, data1, true, 1);
   } 
}

void cap_nx_add_noncoh_secure_px_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) {

//vector<uint32_t> data0;
//vector<uint32_t> data1;
uint32_t data0[2];
uint32_t data1[2];

   //data0.resize(1); 
   //data1.resize(1); 

   data0[0] = (adbase & 0xfffffffful) ;
   data1[0] = (admask & 0xfffffffful) ;
   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS0_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS0_0), 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS1_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS1_0), 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS2_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS2_0), 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS3_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS3_0), 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS4_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS4_0), 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS5_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS5_0), 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS6_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS6_0), 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS7_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS7_0), 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS8_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS8_0), 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS9_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS9_0), 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSA_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSA_0), 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSB_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSB_0), 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSC_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSC_0), 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSD_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSD_0), 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSE_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSE_0), 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSF_0), 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSF_0), 1, data1, true, 1);
   } 

   data0[0] = ((adbase & 0xff00000000ul) >> 32 );
   data1[0] = ((admask & 0xff00000000ul) >> 32 );

   if (range_num == 0) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS0_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS0_0)+4, 1, data1, true, 1);
   } else if (range_num == 1) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS1_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS1_0)+4, 1, data1, true, 1);
   } else if (range_num == 2) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS2_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS2_0)+4, 1, data1, true, 1);
   } else if (range_num == 3) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS3_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS3_0)+4, 1, data1, true, 1);
   } else if (range_num == 4) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS4_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS4_0)+4, 1, data1, true, 1);
   } else if (range_num == 5) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS5_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS5_0)+4, 1, data1, true, 1);
   } else if (range_num == 6) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS6_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS6_0)+4, 1, data1, true, 1);
   } else if (range_num == 7) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS7_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS7_0)+4, 1, data1, true, 1);
   } else if (range_num == 8) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS8_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS8_0)+4, 1, data1, true, 1);
   } else if (range_num == 9) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS9_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS9_0)+4, 1, data1, true, 1);
   } else if (range_num == 10) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSA_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSA_0)+4, 1, data1, true, 1);
   } else if (range_num == 11) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSB_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSB_0)+4, 1, data1, true, 1);
   } else if (range_num == 12) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSC_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSC_0)+4, 1, data1, true, 1);
   } else if (range_num == 13) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSD_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSD_0)+4, 1, data1, true, 1);
   } else if (range_num == 14) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSE_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSE_0)+4, 1, data1, true, 1);
   } else if (range_num == 15) {
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSF_0)+4, 1, data0, true, 1);
      cap_nx_block_write(0, RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSF_0)+4, 1, data1, true, 1);
   } 
}

#ifndef CAPRI_SW
void cap_nx_display_aranges(int chip_id, int inst_id) {

   cap_display_address_range_hdr() ;

   cap_nx_display_bg_range(chip_id, inst_id, "BG_SC", "AR", "HBM");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_NC", "AR", "HBM");
   cap_nx_display_fg_sec_range(chip_id, inst_id, "FG_SC", "AR", "HBM");
   cap_nx_display_fg_nc_range(chip_id, inst_id, "FG_NC", "AR", "HBM");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_SC", "PR", "HBM");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_NC", "PR", "HBM");
   cap_nx_display_fg_sec_range(chip_id, inst_id, "FG_SC", "PR", "HBM");
   cap_nx_display_fg_nc_range(chip_id, inst_id, "FG_NC", "PR", "HBM");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_SC", "PT", "HBM");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_NC", "PT", "HBM");
   cap_nx_display_fg_sec_range(chip_id, inst_id, "FG_SC", "PT", "HBM");
   cap_nx_display_fg_nc_range(chip_id, inst_id, "FG_NC", "PT", "HBM");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_SC", "PB", "HBM");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_NC", "PB", "HBM");
   cap_nx_display_fg_sec_range(chip_id, inst_id, "FG_SC", "PB", "HBM");
   cap_nx_display_fg_nc_range(chip_id, inst_id, "FG_NC", "PB", "HBM");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_SC", "MD", "HBM");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_NC", "MD", "HBM");
   cap_nx_display_fg_sec_range(chip_id, inst_id, "FG_SC", "MD", "HBM");
   cap_nx_display_fg_nc_range(chip_id, inst_id, "FG_NC", "MD", "HBM");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_SC", "SS", "HBM");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_NC", "SS", "HBM");
   cap_nx_display_fg_sec_range(chip_id, inst_id, "FG_SC", "SS", "HBM");
   cap_nx_display_fg_nc_range(chip_id, inst_id, "FG_NC", "SS", "HBM");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_SC", "MS", "HBM");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_NC", "MS", "HBM");
   cap_nx_display_fg_sec_range(chip_id, inst_id, "FG_SC", "MS", "HBM");
   cap_nx_display_fg_nc_range(chip_id, inst_id, "FG_NC", "MS", "HBM");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_SC", "PX", "HBM");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_NC", "PX", "HBM");
   cap_nx_display_fg_sec_range(chip_id, inst_id, "FG_SC", "PX", "HBM");
   cap_nx_display_fg_nc_range(chip_id, inst_id, "FG_NC", "PX", "HBM");


   cap_nx_display_bg_range(chip_id, inst_id, "BG_SC", "AR", "MS");
   cap_nx_display_fg_sec_range(chip_id, inst_id, "FG_SC", "AR", "MS");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_SC", "PR", "MS");
   cap_nx_display_fg_sec_range(chip_id, inst_id, "FG_SC", "PR", "MS");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_SC", "PT", "MS");
   cap_nx_display_fg_sec_range(chip_id, inst_id, "FG_SC", "PT", "MS");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_SC", "PB", "MS");
   cap_nx_display_fg_sec_range(chip_id, inst_id, "FG_SC", "PB", "MS");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_SC", "MD", "MS");
   cap_nx_display_fg_sec_range(chip_id, inst_id, "FG_SC", "MD", "MS");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_SC", "SS", "MS");
   cap_nx_display_fg_sec_range(chip_id, inst_id, "FG_SC", "SS", "MS");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_SC", "MS", "MS");
   cap_nx_display_fg_sec_range(chip_id, inst_id, "FG_SC", "MS", "MS");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_SC", "PX", "MS");
   cap_nx_display_fg_sec_range(chip_id, inst_id, "FG_SC", "PX", "MS");

   cap_nx_display_bg_range(chip_id, inst_id, "BG_SC", "AR", "PX");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_SC", "PR", "PX");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_SC", "PT", "PX");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_SC", "PB", "PX");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_SC", "MD", "PX");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_SC", "SS", "PX");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_SC", "MS", "PX");
   cap_nx_display_bg_range(chip_id, inst_id, "BG_SC", "PX", "PX");
}

void cap_nx_display_bg_range(int chip_id, int inst_id, const char *range_name, const char *master_name, const char *slave_name) {

string l_master_name = master_name;
string l_slave_name = slave_name;
string l_range_name = range_name;
//vector<uint32_t> rd_data;
//uint32_t rd_data[2];

   if (l_slave_name == "HBM") {
      if (l_master_name == "AR") {
         if (l_range_name == "BG_SC") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM0_0),
			RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM1_0),
			RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM2_0),
			RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM2_0));
         } else {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC0_0),
			RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NC0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC1_0),
			RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NC1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC2_0),
			RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NC2_0));
         }
      }
      else if (l_master_name == "MD") {
         if (l_range_name == "BG_SC") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM0_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM1_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM2_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM2_0));
         } else {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC0_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NC0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC1_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NC1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC2_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NC2_0));
         }
      }
      else if (l_master_name == "PT")    {
         if (l_range_name == "BG_SC") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM0_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM1_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM2_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM2_0));
         } else {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC0_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NC0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC1_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NC1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC2_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NC2_0));
         }
      }
      else if (l_master_name == "PR") {
         if (l_range_name == "BG_SC") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM0_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM1_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM2_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM2_0));
         } else {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC0_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NC0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC1_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NC1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC2_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NC2_0));
         }
      }
      else if (l_master_name == "SS") {
         if (l_range_name == "BG_SC") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM0_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM1_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM2_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM2_0));
         } else {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC0_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NC0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC1_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NC1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC2_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NC2_0));
         }
      }
      else if (l_master_name == "PB") {
         if (l_range_name == "BG_SC") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM0_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM1_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM2_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM2_0));
         } else {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC0_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NC0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC1_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NC1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC2_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NC2_0));
         }
      }
      else if (l_master_name == "PX") {
         if (l_range_name == "BG_SC") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM0_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM1_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM2_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM2_0));
         } else {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC0_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NC0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC1_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NC1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC2_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NC2_0));
         }
      }
      else if (l_master_name == "MS") {
         if (l_range_name == "BG_SC") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM0_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM1_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM2_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM2_0));
         } else {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC0_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NC0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC1_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NC1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NC2_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NC2_0));
         }
      } else { // Illegal
      }
   }
   if (l_slave_name == "MS") {
      if (l_master_name == "AR") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_0),
			RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_0));
      }
      else if (l_master_name == "MD") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_0));
      }
      else if (l_master_name == "PT")    {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_0));
      }
      else if (l_master_name == "PR") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_0));
      }
      else if (l_master_name == "SS") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_0));
      }
      else if (l_master_name == "PB") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_0));
      }
      else if (l_master_name == "PX") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_0));
      }
      else if (l_master_name == "MS") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_0));
      } else { // Illegal
      }
   }
   if (l_slave_name == "PX") {
      if (l_master_name == "AR") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_PX_S_MAP_PX0_0),
			RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_PX_S_MAP_PX0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_PX_S_MAP_PX1_0),
			RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_PX_S_MAP_PX1_0));
      }
      else if (l_master_name == "MD") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_PX_S_MAP_PX0_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_PX_S_MAP_PX0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_PX_S_MAP_PX1_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_PX_S_MAP_PX1_0));
      }
      else if (l_master_name == "PT")    {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_PX_S_MAP_PX0_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_PX_S_MAP_PX0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_PX_S_MAP_PX1_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_PX_S_MAP_PX1_0));
      }
      else if (l_master_name == "PR") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_PX_S_MAP_PX0_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_PX_S_MAP_PX0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_PX_S_MAP_PX1_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_PX_S_MAP_PX1_0));
      }
      else if (l_master_name == "SS") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_PX_S_MAP_PX0_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_PX_S_MAP_PX0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_PX_S_MAP_PX1_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_PX_S_MAP_PX1_0));
      }
      else if (l_master_name == "PB") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_PX_S_MAP_PX0_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_PX_S_MAP_PX0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_PX_S_MAP_PX1_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_PX_S_MAP_PX1_0));
      }
      else if (l_master_name == "PX") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_PX_S_MAP_PX0_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_PX_S_MAP_PX0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_PX_S_MAP_PX1_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_PX_S_MAP_PX1_0));
      }
      else if (l_master_name == "MS") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_PX_S_MAP_PX0_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_PX_S_MAP_PX0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_PX_S_MAP_PX1_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_PX_S_MAP_PX1_0));
      } else { // Illegal
      }
   }
}

void cap_nx_display_fg_sec_range(int chip_id, int inst_id, const char *range_name, const char *master_name, const char *slave_name) {

string l_master_name = master_name;
string l_slave_name = slave_name;
//vector<uint32_t> rd_data;
//uint32_t rd_data[2];

   if (l_slave_name == "HBM") {
      if (l_master_name == "AR") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S0_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S1_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S2_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S2_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S3_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S3_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S4_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S4_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S5_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S5_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S6_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S6_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S7_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S7_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S8_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S8_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S9_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S9_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S10_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S10_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S11_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S11_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S12_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S12_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S13_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S13_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S14_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S14_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S15_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S15_0));
      }
      else if (l_master_name == "MD") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S0_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S1_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S2_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S2_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S3_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S3_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S4_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S4_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S5_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S5_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S6_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S6_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S7_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S7_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S8_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S8_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S9_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S9_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S10_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S10_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S11_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S11_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S12_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S12_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S13_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S13_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S14_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S14_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S15_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S15_0));
      }
      else if (l_master_name == "PT")    {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S0_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S1_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S2_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S2_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S3_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S3_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S4_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S4_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S5_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S5_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S6_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S6_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S7_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S7_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S8_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S8_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S9_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S9_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S10_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S10_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S11_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S11_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S12_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S12_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S13_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S13_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S14_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S14_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S15_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S15_0));
      }
      else if (l_master_name == "PR") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S0_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S1_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S2_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S2_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S3_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S3_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S4_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S4_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S5_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S5_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S6_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S6_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S7_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S7_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S8_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S8_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S9_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S9_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S10_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S10_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S11_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S11_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S12_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S12_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S13_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S13_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S14_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S14_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S15_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S15_0));
      }
      else if (l_master_name == "SS") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S0_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S1_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S2_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S2_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S3_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S3_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S4_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S4_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S5_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S5_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S6_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S6_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S7_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S7_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S8_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S8_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S9_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S9_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S10_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S10_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S11_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S11_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S12_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S12_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S13_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S13_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S14_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S14_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S15_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S15_0));
      }
      else if (l_master_name == "PB") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S0_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S1_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S2_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S2_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S3_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S3_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S4_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S4_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S5_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S5_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S6_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S6_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S7_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S7_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S8_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S8_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S9_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S9_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S10_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S10_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S11_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S11_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S12_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S12_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S13_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S13_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S14_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S14_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S15_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S15_0));
      }
      else if (l_master_name == "PX") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S0_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S1_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S2_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S2_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S3_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S3_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S4_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S4_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S5_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S5_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S6_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S6_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S7_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S7_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S8_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S8_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S9_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S9_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S10_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S10_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S11_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S11_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S12_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S12_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S13_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S13_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S14_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S14_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S15_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S15_0));
      }
      else if (l_master_name == "MS") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S0_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S1_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S2_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S2_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S3_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S3_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S4_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S4_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S5_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S5_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S6_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S6_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S7_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S7_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S8_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S8_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S9_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S9_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S10_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S10_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S11_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S11_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S12_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S12_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S13_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S13_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S14_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S14_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_S15_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_S15_0));
      } else { // Illegal
      }
   }
   if (l_slave_name == "MS") {
      if (l_master_name == "AR") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S0_0),
			RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S1_0),
			RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S2_0),
			RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S2_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S3_0),
			RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S3_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S4_0),
			RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S4_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S5_0),
			RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S5_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S6_0),
			RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S6_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S7_0),
			RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S7_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S8_0),
			RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S8_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S9_0),
			RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S9_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S10_0),
			RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S10_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S11_0),
			RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S11_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S12_0),
			RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S12_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S13_0),
			RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S13_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S14_0),
			RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S14_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S15_0),
			RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S15_0));
      }
      else if (l_master_name == "MD") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S0_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S1_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S2_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S2_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S3_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S3_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S4_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S4_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S5_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S5_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S6_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S6_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S7_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S7_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S8_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S8_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S9_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S9_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S10_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S10_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S11_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S11_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S12_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S12_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S13_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S13_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S14_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S14_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S15_0),
			RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S15_0));
      }
      else if (l_master_name == "PT")    {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S0_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S1_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S2_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S2_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S3_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S3_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S4_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S4_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S5_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S5_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S6_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S6_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S7_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S7_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S8_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S8_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S9_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S9_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S10_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S10_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S11_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S11_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S12_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S12_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S13_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S13_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S14_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S14_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S15_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S15_0));
      }
      else if (l_master_name == "PR") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S0_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S1_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S2_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S2_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S3_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S3_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S4_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S4_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S5_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S5_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S6_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S6_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S7_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S7_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S8_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S8_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S9_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S9_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S10_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S10_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S11_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S11_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S12_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S12_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S13_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S13_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S14_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S14_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S15_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S15_0));
      }
      else if (l_master_name == "SS") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S0_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S1_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S2_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S2_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S3_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S3_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S4_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S4_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S5_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S5_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S6_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S6_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S7_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S7_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S8_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S8_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S9_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S9_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S10_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S10_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S11_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S11_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S12_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S12_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S13_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S13_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S14_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S14_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S15_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S15_0));
      }
      else if (l_master_name == "PB") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S0_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S1_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S2_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S2_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S3_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S3_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S4_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S4_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S5_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S5_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S6_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S6_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S7_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S7_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S8_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S8_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S9_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S9_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S10_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S10_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S11_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S11_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S12_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S12_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S13_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S13_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S14_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S14_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S15_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S15_0));
      }
      else if (l_master_name == "PX") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S0_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S1_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S2_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S2_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S3_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S3_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S4_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S4_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S5_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S5_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S6_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S6_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S7_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S7_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S8_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S8_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S9_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S9_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S10_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S10_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S11_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S11_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S12_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S12_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S13_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S13_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S14_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S14_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S15_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S15_0));
      }
      else if (l_master_name == "MS") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S0_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S1_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S2_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S2_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S3_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S3_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S4_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S4_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S5_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S5_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S6_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S6_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S7_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S7_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S8_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S8_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S9_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S9_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S10_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S10_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S11_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S11_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S12_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S12_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S13_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S13_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S14_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S14_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S15_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S15_0));
      } else { // Illegal
      }
   }
}

void cap_nx_display_fg_nc_range(int chip_id, int inst_id, const char *range_name, const char *master_name, const char *slave_name) {

string l_master_name = master_name;
string l_slave_name = slave_name;
//vector<uint32_t> rd_data;
//uint32_t rd_data[2];

   if (l_slave_name == "HBM") {
      if (l_master_name == "AR") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS0_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS1_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS2_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS2_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS3_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS3_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS4_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS4_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS5_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS5_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS6_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS6_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS7_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS7_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS8_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS8_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS9_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS9_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSA_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSA_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSB_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSB_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSC_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSC_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSD_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSD_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSE_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSE_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSF_0),
                        RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSF_0));
      }
      else if (l_master_name == "MD") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS0_0),
                        RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS1_0),
                        RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS2_0),
                        RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS2_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS3_0),
                        RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS3_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS4_0),
                        RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS4_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS5_0),
                        RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS5_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS6_0),
                        RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS6_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS7_0),
                        RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS7_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS8_0),
                        RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS8_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS9_0),
                        RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS9_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSA_0),
                        RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSA_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSB_0),
                        RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSB_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSC_0),
                        RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSC_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSD_0),
                        RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSD_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSE_0),
                        RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSE_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
                        RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSF_0),
                        RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSF_0));
      }
      else if (l_master_name == "PT")    {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS0_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS1_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS2_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS2_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS3_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS3_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS4_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS4_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS5_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS5_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS6_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS6_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS7_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS7_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS8_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS8_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS9_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS9_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSA_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSA_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSB_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSB_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSC_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSC_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSD_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSD_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSE_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSE_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSF_0),
			RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSF_0));
      }
      else if (l_master_name == "PR") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS0_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS1_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS2_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS2_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS3_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS3_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS4_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS4_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS5_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS5_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS6_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS6_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS7_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS7_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS8_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS8_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS9_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS9_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSA_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSA_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSB_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSB_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSC_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSC_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSD_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSD_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSE_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSE_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSF_0),
			RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSF_0));
      }
      else if (l_master_name == "SS") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS0_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS1_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS2_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS2_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS3_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS3_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS4_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS4_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS5_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS5_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS6_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS6_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS7_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS7_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS8_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS8_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS9_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS9_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSA_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSA_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSB_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSB_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSC_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSC_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSD_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSD_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSE_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSE_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSF_0),
			RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSF_0));
      }
      else if (l_master_name == "PB") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS0_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS1_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS2_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS2_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS3_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS3_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS4_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS4_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS5_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS5_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS6_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS6_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS7_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS7_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS8_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS8_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS9_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS9_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSA_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSA_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSB_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSB_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSC_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSC_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSD_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSD_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSE_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSE_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSF_0),
			RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSF_0));
      }
      else if (l_master_name == "PX") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS0_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS1_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS2_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS2_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS3_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS3_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS4_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS4_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS5_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS5_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS6_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS6_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS7_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS7_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS8_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS8_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS9_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS9_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSA_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSA_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSB_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSB_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSC_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSC_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSD_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSD_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSE_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSE_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSF_0),
			RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSF_0));
      }
      else if (l_master_name == "MS") {
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS0_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS0_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS1_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS1_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS2_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS2_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS3_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS3_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS4_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS4_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS5_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS5_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS6_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS6_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS7_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS7_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS8_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS8_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNS9_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNS9_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSA_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSA_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSB_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSB_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSC_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSC_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSD_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSD_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSE_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSE_0));
         cap_display_address_range_entry(chip_id, inst_id, range_name, master_name, slave_name, 
			RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_HBM0_S_MAP_HBM_NCNSF_0),
			RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_HBM0_S_MAP_HBM_NCNSF_0));
      } else { // Illegal
      }
   }
}

void cap_display_address_range_entry(int chip_id, int inst_id, const char *range_name, const char *master_name, const char *slave_name, uint64_t adbase_addr, uint64_t admask_addr) {

//vector<uint32_t> rd_data;
uint32_t rd_data[2];
uint64_t adbase;
uint64_t admask;

   //rd_data.resize(1); 

   rd_data[0] = cap_nx_block_read(0, adbase_addr+4, 1, false, 1);
   adbase = rd_data[0];
   rd_data[0] = cap_nx_block_read(0, adbase_addr+0, 1, false, 1);
   adbase = (adbase << 32) | rd_data[0];
   rd_data[0] = cap_nx_block_read(0, admask_addr+4, 1, false, 1);
   admask = rd_data[0];
   rd_data[0] = cap_nx_block_read(0, admask_addr+0, 1, false, 1);
   admask = (admask << 32) | rd_data[0];
   cap_display_address_range_val(range_name, master_name, slave_name, adbase, admask);
}

void cap_display_address_range_hdr() {

   PLOG_API_MSG("NX", setfill('=') << setw(100) << "=" << "\n");
   PLOG_API_MSG("NX", setfill(' ') << "\n");
   PLOG_API_MSG("NX", setw(10) << "RNG_TYP" << setw(10) << "MASTER" << setw(10) << "SLAVE" << hex << setw(20) << "ADDR_BASE" << setw(20) << "ADDR_MASK" << setw(10) << "RNG_DIS" << setw(10) << "SEC_VLD" << setw(10) << "TRS_MST" << setw(10) << "SEC_MSK" << "\n");
   PLOG_API_MSG("NX", setfill('=') << setw(100) << "=" << "\n");
   PLOG_API_MSG("NX", setfill(' ') << "\n");

}

void cap_nx_display_hash() {

   PLOG_API_MSG("NX", setfill('=') << setw(100) << "=" << "\n");
   PLOG_API_MSG("NX", setfill(' ') << "\n");
   PLOG_API_MSG("NX", setw(16) << "MASTER_NAME" << setw(16) << "HASH3" << setw(16) << "HASH2" << setw(16) << "HASH1" << setw(16) << "HASH0" << "\n");
   PLOG_API_MSG("NX", setfill('=') << setw(100) << "=" << "\n");
   PLOG_API_MSG("NX", setfill(' ') << "\n");
   cap_nx_get_hash("AR");
   cap_nx_get_hash("MD");
   cap_nx_get_hash("MS");
   cap_nx_get_hash("PB");
   cap_nx_get_hash("PR");
   cap_nx_get_hash("PT");
   cap_nx_get_hash("PX");
   cap_nx_get_hash("SS");
}

void cap_nx_get_hash(const char *master_name) {

vector<uint32_t> data0;
vector<uint32_t> data1;
vector<uint32_t> data2;
vector<uint32_t> data3;

uint64_t hash3, tmp_hash3;
uint64_t hash2, tmp_hash2;
uint64_t hash1, tmp_hash1;
uint64_t hash0, tmp_hash0;

string l_master_name = master_name;

   data0.resize(1); data1.resize(1); data2.resize(1); data3.resize(1); 

   if (l_master_name == "AR") {
      data3 = cpu::access()->block_read(0, RBM_BRIDGE_(AR_M_0_809_AM_HASH_FUNC_HASH3) , 1, false, 1);
      data2 = cpu::access()->block_read(0, RBM_BRIDGE_(AR_M_0_809_AM_HASH_FUNC_HASH2) , 1, false, 1);
      data1 = cpu::access()->block_read(0, RBM_BRIDGE_(AR_M_0_809_AM_HASH_FUNC_HASH1) , 1, false, 1);
      data0 = cpu::access()->block_read(0, RBM_BRIDGE_(AR_M_0_809_AM_HASH_FUNC_HASH0) , 1, false, 1);

      hash3 = data3[0]; hash2 = data2[0]; hash1 = data1[0]; hash0 = data0[0];

      data3 = cpu::access()->block_read(0, RBM_BRIDGE_(AR_M_0_809_AM_HASH_FUNC_HASH3) + 4, 1, false, 1);
      data2 = cpu::access()->block_read(0, RBM_BRIDGE_(AR_M_0_809_AM_HASH_FUNC_HASH2) + 4, 1, false, 1);
      data1 = cpu::access()->block_read(0, RBM_BRIDGE_(AR_M_0_809_AM_HASH_FUNC_HASH1) + 4, 1, false, 1);
      data0 = cpu::access()->block_read(0, RBM_BRIDGE_(AR_M_0_809_AM_HASH_FUNC_HASH0) + 4, 1, false, 1);

   }

   if (l_master_name == "MD") {
      data3 = cpu::access()->block_read(0, RBM_BRIDGE_(MD_M_1_138_AM_HASH_FUNC_HASH3) , 1, false, 1);
      data2 = cpu::access()->block_read(0, RBM_BRIDGE_(MD_M_1_138_AM_HASH_FUNC_HASH2) , 1, false, 1);
      data1 = cpu::access()->block_read(0, RBM_BRIDGE_(MD_M_1_138_AM_HASH_FUNC_HASH1) , 1, false, 1);
      data0 = cpu::access()->block_read(0, RBM_BRIDGE_(MD_M_1_138_AM_HASH_FUNC_HASH0) , 1, false, 1);

      hash3 = data3[0]; hash2 = data2[0]; hash1 = data1[0]; hash0 = data0[0];

      data3 = cpu::access()->block_read(0, RBM_BRIDGE_(MD_M_1_138_AM_HASH_FUNC_HASH3) + 4, 1, false, 1);
      data2 = cpu::access()->block_read(0, RBM_BRIDGE_(MD_M_1_138_AM_HASH_FUNC_HASH2) + 4, 1, false, 1);
      data1 = cpu::access()->block_read(0, RBM_BRIDGE_(MD_M_1_138_AM_HASH_FUNC_HASH1) + 4, 1, false, 1);
      data0 = cpu::access()->block_read(0, RBM_BRIDGE_(MD_M_1_138_AM_HASH_FUNC_HASH0) + 4, 1, false, 1);

   }

   if (l_master_name == "MS") {
      data3 = cpu::access()->block_read(0, RBM_BRIDGE_(MS_M_2_774_AM_HASH_FUNC_HASH3) , 1, false, 1);
      data2 = cpu::access()->block_read(0, RBM_BRIDGE_(MS_M_2_774_AM_HASH_FUNC_HASH2) , 1, false, 1);
      data1 = cpu::access()->block_read(0, RBM_BRIDGE_(MS_M_2_774_AM_HASH_FUNC_HASH1) , 1, false, 1);
      data0 = cpu::access()->block_read(0, RBM_BRIDGE_(MS_M_2_774_AM_HASH_FUNC_HASH0) , 1, false, 1);

      hash3 = data3[0]; hash2 = data2[0]; hash1 = data1[0]; hash0 = data0[0];

      data3 = cpu::access()->block_read(0, RBM_BRIDGE_(MS_M_2_774_AM_HASH_FUNC_HASH3) + 4, 1, false, 1);
      data2 = cpu::access()->block_read(0, RBM_BRIDGE_(MS_M_2_774_AM_HASH_FUNC_HASH2) + 4, 1, false, 1);
      data1 = cpu::access()->block_read(0, RBM_BRIDGE_(MS_M_2_774_AM_HASH_FUNC_HASH1) + 4, 1, false, 1);
      data0 = cpu::access()->block_read(0, RBM_BRIDGE_(MS_M_2_774_AM_HASH_FUNC_HASH0) + 4, 1, false, 1);


   }

   if (l_master_name == "PB") {
      data3 = cpu::access()->block_read(0, RBM_BRIDGE_(PB_M_3_150_AM_HASH_FUNC_HASH3) , 1, false, 1);
      data2 = cpu::access()->block_read(0, RBM_BRIDGE_(PB_M_3_150_AM_HASH_FUNC_HASH2) , 1, false, 1);
      data1 = cpu::access()->block_read(0, RBM_BRIDGE_(PB_M_3_150_AM_HASH_FUNC_HASH1) , 1, false, 1);
      data0 = cpu::access()->block_read(0, RBM_BRIDGE_(PB_M_3_150_AM_HASH_FUNC_HASH0) , 1, false, 1);

      hash3 = data3[0]; hash2 = data2[0]; hash1 = data1[0]; hash0 = data0[0];

      data3 = cpu::access()->block_read(0, RBM_BRIDGE_(PB_M_3_150_AM_HASH_FUNC_HASH3) + 4, 1, false, 1);
      data2 = cpu::access()->block_read(0, RBM_BRIDGE_(PB_M_3_150_AM_HASH_FUNC_HASH2) + 4, 1, false, 1);
      data1 = cpu::access()->block_read(0, RBM_BRIDGE_(PB_M_3_150_AM_HASH_FUNC_HASH1) + 4, 1, false, 1);
      data0 = cpu::access()->block_read(0, RBM_BRIDGE_(PB_M_3_150_AM_HASH_FUNC_HASH0) + 4, 1, false, 1);

   }

   if (l_master_name == "PR") {
      data3 = cpu::access()->block_read(0, RBM_BRIDGE_(PR_M_4_141_AM_HASH_FUNC_HASH3) , 1, false, 1);
      data2 = cpu::access()->block_read(0, RBM_BRIDGE_(PR_M_4_141_AM_HASH_FUNC_HASH2) , 1, false, 1);
      data1 = cpu::access()->block_read(0, RBM_BRIDGE_(PR_M_4_141_AM_HASH_FUNC_HASH1) , 1, false, 1);
      data0 = cpu::access()->block_read(0, RBM_BRIDGE_(PR_M_4_141_AM_HASH_FUNC_HASH0) , 1, false, 1);

      hash3 = data3[0]; hash2 = data2[0]; hash1 = data1[0]; hash0 = data0[0];

      data3 = cpu::access()->block_read(0, RBM_BRIDGE_(PR_M_4_141_AM_HASH_FUNC_HASH3) + 4, 1, false, 1);
      data2 = cpu::access()->block_read(0, RBM_BRIDGE_(PR_M_4_141_AM_HASH_FUNC_HASH2) + 4, 1, false, 1);
      data1 = cpu::access()->block_read(0, RBM_BRIDGE_(PR_M_4_141_AM_HASH_FUNC_HASH1) + 4, 1, false, 1);
      data0 = cpu::access()->block_read(0, RBM_BRIDGE_(PR_M_4_141_AM_HASH_FUNC_HASH0) + 4, 1, false, 1);

   }

   if (l_master_name == "PT") {
      data3 = cpu::access()->block_read(0, RBM_BRIDGE_(PT_M_5_136_AM_HASH_FUNC_HASH3) , 1, false, 1);
      data2 = cpu::access()->block_read(0, RBM_BRIDGE_(PT_M_5_136_AM_HASH_FUNC_HASH2) , 1, false, 1);
      data1 = cpu::access()->block_read(0, RBM_BRIDGE_(PT_M_5_136_AM_HASH_FUNC_HASH1) , 1, false, 1);
      data0 = cpu::access()->block_read(0, RBM_BRIDGE_(PT_M_5_136_AM_HASH_FUNC_HASH0) , 1, false, 1);

      hash3 = data3[0]; hash2 = data2[0]; hash1 = data1[0]; hash0 = data0[0];

      data3 = cpu::access()->block_read(0, RBM_BRIDGE_(PT_M_5_136_AM_HASH_FUNC_HASH3) + 4, 1, false, 1);
      data2 = cpu::access()->block_read(0, RBM_BRIDGE_(PT_M_5_136_AM_HASH_FUNC_HASH2) + 4, 1, false, 1);
      data1 = cpu::access()->block_read(0, RBM_BRIDGE_(PT_M_5_136_AM_HASH_FUNC_HASH1) + 4, 1, false, 1);
      data0 = cpu::access()->block_read(0, RBM_BRIDGE_(PT_M_5_136_AM_HASH_FUNC_HASH0) + 4, 1, false, 1);

   }

   if (l_master_name == "PX") {
      data3 = cpu::access()->block_read(0, RBM_BRIDGE_(PX_M_6_786_AM_HASH_FUNC_HASH3) , 1, false, 1);
      data2 = cpu::access()->block_read(0, RBM_BRIDGE_(PX_M_6_786_AM_HASH_FUNC_HASH2) , 1, false, 1);
      data1 = cpu::access()->block_read(0, RBM_BRIDGE_(PX_M_6_786_AM_HASH_FUNC_HASH1) , 1, false, 1);
      data0 = cpu::access()->block_read(0, RBM_BRIDGE_(PX_M_6_786_AM_HASH_FUNC_HASH0) , 1, false, 1);

      hash3 = data3[0]; hash2 = data2[0]; hash1 = data1[0]; hash0 = data0[0];

      data3 = cpu::access()->block_read(0, RBM_BRIDGE_(PX_M_6_786_AM_HASH_FUNC_HASH3) + 4, 1, false, 1);
      data2 = cpu::access()->block_read(0, RBM_BRIDGE_(PX_M_6_786_AM_HASH_FUNC_HASH2) + 4, 1, false, 1);
      data1 = cpu::access()->block_read(0, RBM_BRIDGE_(PX_M_6_786_AM_HASH_FUNC_HASH1) + 4, 1, false, 1);
      data0 = cpu::access()->block_read(0, RBM_BRIDGE_(PX_M_6_786_AM_HASH_FUNC_HASH0) + 4, 1, false, 1);

   }

   if (l_master_name == "SS") {
      data3 = cpu::access()->block_read(0, RBM_BRIDGE_(SS_M_7_396_AM_HASH_FUNC_HASH3) , 1, false, 1);
      data2 = cpu::access()->block_read(0, RBM_BRIDGE_(SS_M_7_396_AM_HASH_FUNC_HASH2) , 1, false, 1);
      data1 = cpu::access()->block_read(0, RBM_BRIDGE_(SS_M_7_396_AM_HASH_FUNC_HASH1) , 1, false, 1);
      data0 = cpu::access()->block_read(0, RBM_BRIDGE_(SS_M_7_396_AM_HASH_FUNC_HASH0) , 1, false, 1);

      hash3 = data3[0]; hash2 = data2[0]; hash1 = data1[0]; hash0 = data0[0];

      data3 = cpu::access()->block_read(0, RBM_BRIDGE_(SS_M_7_396_AM_HASH_FUNC_HASH3) + 4, 1, false, 1);
      data2 = cpu::access()->block_read(0, RBM_BRIDGE_(SS_M_7_396_AM_HASH_FUNC_HASH2) + 4, 1, false, 1);
      data1 = cpu::access()->block_read(0, RBM_BRIDGE_(SS_M_7_396_AM_HASH_FUNC_HASH1) + 4, 1, false, 1);
      data0 = cpu::access()->block_read(0, RBM_BRIDGE_(SS_M_7_396_AM_HASH_FUNC_HASH0) + 4, 1, false, 1);

   }
   tmp_hash3 = data3[0];tmp_hash2 = data2[0];tmp_hash1 = data1[0];tmp_hash0 = data0[0];
   hash3 |= (tmp_hash3 << 32ul); hash2 |= (tmp_hash2 << 32ul); hash1 |= (tmp_hash1 << 32ul); hash0 |= (tmp_hash0 << 32ul);

   PLOG_API_MSG("NX", setw(16) << master_name << setw(16) << hex << hash3 << setw(16) << hash2 << setw(16) << hash1 << setw(16) << hash0 << "\n");

}

void cap_display_address_range_val(const char *rname, const char *mname, const char *sname, uint64_t adbase, uint64_t admask) {

//uint32_t adbase_llc_disable = (adbase & (1 << 5)) >> 5;
uint32_t adbase_range_disable = (adbase & (1 << 4)) >> 4;
//uint32_t adbase_rd_wr_n = (adbase & (1 << 3)) >> 3;
//uint32_t adbase_instr = (adbase & (1 << 2)) >> 2;
uint32_t adbase_non_sec = (adbase & (1 << 1)) >> 1;
//uint32_t adbase_privileged = (adbase & (1 << 0)) >> 0;

uint32_t admask_trusted_master = (admask & (1 << 4)) >> 4;
//uint32_t admask_rd_wr_n_valid = (admask & (1 << 3)) >> 3;
//uint32_t admask_instr = (admask & (1 << 2)) >> 2;
uint32_t admask_non_sec = (admask & (1 << 1)) >> 1;
//uint32_t admask_privileged = (admask & (1 << 0)) >> 0;

   adbase = adbase & 0xffffffffffffffc0ul;
   admask = admask & 0xffffffffffffffc0ul;
   PLOG_API_MSG("NX", setw(10) << rname << setw(10) << mname << setw(10) << sname << hex << setw(20) << adbase << setw(20) << admask << setw(10) << adbase_range_disable << setw(10) << adbase_non_sec << setw(10) << admask_trusted_master << setw(10) << admask_non_sec << "\n");

}

void cap_nx_display_llc_counters() {

//vector<uint32_t> rd_data;
uint32_t rd_data[2];
string ev_name = "";

   PLOG_API_MSG("NX", setfill('=') << setw(100) << "=" << "\n");
   PLOG_API_MSG("NX", setfill(' ') << "\n");
   PLOG_API_MSG("NX", setw(10) << "LLC_NUM" << setw(40) << "EV_NAME" << hex << setw(20) << "COUNTER" << "\n");
   PLOG_API_MSG("NX", setfill('=') << setw(100) << "=" << "\n");
   PLOG_API_MSG("NX", setfill(' ') << "\n");

   //rd_data.resize(1);
   rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC0_LLC_EVENT_COUNTER_MASK), 1, false, 1);

   if ((rd_data[0] & 0x1) == 0x1) {
      ev_name = ev_name + "cache_rd ";
   }
   if ((rd_data[0] & 0x2) == 0x2) {
      ev_name = ev_name + "cache_wr ";
   }
   if ((rd_data[0] & 0x4) == 0x4) {
      ev_name = ev_name + "scratch ";
   }
   if ((rd_data[0] & 0x8) == 0x8) {
      ev_name = ev_name + "cache_hit";
   }
   if ((rd_data[0] & 0x10) == 0x10) {
      ev_name = ev_name + "cache_mis";
   }
   if ((rd_data[0] & 0x20) == 0x20) {
      ev_name = ev_name + "partal_wr";
   }
   if ((rd_data[0] & 0x40) == 0x40) {
      ev_name = ev_name + "cache_mnt";
   }
   if ((rd_data[0] & 0x80) == 0x80) {
      ev_name = ev_name + "cache_evc";
   }
   if ((rd_data[0] & 0x100) == 0x100) {
      ev_name = ev_name + "retry_ned";
   }
   if ((rd_data[0] & 0x200) == 0x200) {
      ev_name = ev_name + "retry_acc";
   }

   for (int i=0; i < 16; i++) {
      switch (i) {
         case 0:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC0_LLC_EVENT_COUNTER), 1, false, 1);
		break;
         case 1:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC1_LLC_EVENT_COUNTER), 1, false, 1);
		break;
         case 2:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC2_LLC_EVENT_COUNTER), 1, false, 1);
		break;
         case 3:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC3_LLC_EVENT_COUNTER), 1, false, 1);
		break;
         case 4:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC4_LLC_EVENT_COUNTER), 1, false, 1);
		break;
         case 5:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC5_LLC_EVENT_COUNTER), 1, false, 1);
		break;
         case 6:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC6_LLC_EVENT_COUNTER), 1, false, 1);
		break;
         case 7:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC7_LLC_EVENT_COUNTER), 1, false, 1);
		break;
         case 8:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC8_LLC_EVENT_COUNTER), 1, false, 1);
		break;
         case 9:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC9_LLC_EVENT_COUNTER), 1, false, 1);
		break;
         case 10:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC10_LLC_EVENT_COUNTER), 1, false, 1);
		break;
         case 11:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC11_LLC_EVENT_COUNTER), 1, false, 1);
		break;
         case 12:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC12_LLC_EVENT_COUNTER), 1, false, 1);
		break;
         case 13:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC13_LLC_EVENT_COUNTER), 1, false, 1);
		break;
         case 14:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC14_LLC_EVENT_COUNTER), 1, false, 1);
		break;
         case 15:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC15_LLC_EVENT_COUNTER), 1, false, 1);
		break;
      }
      PLOG_API_MSG("NX", setw(10) << i << setw(40) << ev_name << dec << setw(20) << rd_data[0] << "\n");
   }
}

void cap_nx_display_cache_enables() {

//vector<uint32_t> rd_data;
uint32_t rd_data[2];

   PLOG_API_MSG("NX", setfill('=') << setw(80) << "=" << "\n");
   PLOG_API_MSG("NX", setfill(' ') << "\n");
   PLOG_API_MSG("NX", setw(10) << "LLC_NUM" << hex << setw(30) << "CACHE_WAY_EN_BITVEC" << "\n");
   PLOG_API_MSG("NX", setfill('=') << setw(80) << "=" << "\n");
   PLOG_API_MSG("NX", setfill(' ') << "\n");

   //rd_data.resize(1);

   for (int i=0; i < 16; i++) {
      switch (i) {
         case 0:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC0_LLC_CACHE_WAY_ENABLE), 1, false, 1);
		break;
         case 1:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC1_LLC_CACHE_WAY_ENABLE), 1, false, 1);
		break;
         case 2:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC2_LLC_CACHE_WAY_ENABLE), 1, false, 1);
		break;
         case 3:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC3_LLC_CACHE_WAY_ENABLE), 1, false, 1);
		break;
         case 4:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC4_LLC_CACHE_WAY_ENABLE), 1, false, 1);
		break;
         case 5:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC5_LLC_CACHE_WAY_ENABLE), 1, false, 1);
		break;
         case 6:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC6_LLC_CACHE_WAY_ENABLE), 1, false, 1);
		break;
         case 7:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC7_LLC_CACHE_WAY_ENABLE), 1, false, 1);
		break;
         case 8:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC8_LLC_CACHE_WAY_ENABLE), 1, false, 1);
		break;
         case 9:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC9_LLC_CACHE_WAY_ENABLE), 1, false, 1);
		break;
         case 10:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC10_LLC_CACHE_WAY_ENABLE), 1, false, 1);
		break;
         case 11:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC11_LLC_CACHE_WAY_ENABLE), 1, false, 1);
		break;
         case 12:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC12_LLC_CACHE_WAY_ENABLE), 1, false, 1);
		break;
         case 13:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC13_LLC_CACHE_WAY_ENABLE), 1, false, 1);
		break;
         case 14:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC14_LLC_CACHE_WAY_ENABLE), 1, false, 1);
		break;
         case 15:
                rd_data[0] = cap_nx_block_read(0, RBM_AGENT_(CCC15_LLC_CACHE_WAY_ENABLE), 1, false, 1);
		break;
      }
      PLOG_API_MSG("NX", setw(10) << i << setw(30) << hex << rd_data[0] << "\n");
   }
}

unsigned int cap_nx_read_pb_axi_cnt(int rd) { // 1=>rd , 0=> wr
uint32_t rd_data1[2];
uint32_t rd_data0[2];
   if (rd == 1) {
       rd_data0[0] = cap_nx_block_read(0, RBM_BRIDGE_(PB_M_3_150_AM_CNTR0), 1, false, 1);
       return (unsigned int)rd_data0[0];
   } else {
       rd_data1[0] = cap_nx_block_read(0, RBM_BRIDGE_(PB_M_3_150_AM_CNTR1), 1, false, 1);
       return (unsigned int)rd_data1[0];
   }
}

void cap_nx_display_aw_ar_cnt() { // 0=>PT, 1=>PR, 2=>AR, 3=>MD, 4=>MS, 5=>PB, 6 =>SS, 7=>PX Else All Counters

//vector<uint32_t> rd_data0;
//vector<uint32_t> rd_data1;
uint32_t rd_data1[2];
uint32_t rd_data0[2];
string mname = "";

   PLOG_API_MSG("NX", setfill('=') << setw(100) << "=" << "\n");
   PLOG_API_MSG("NX", setfill(' ') << "\n");
   PLOG_API_MSG("NX", setw(10) << "MASTER_NAME" << setw(20) << "AR_COUNT" << setw(20) << "AW_COUNT" << "\n");
   PLOG_API_MSG("NX", setfill('=') << setw(100) << "=" << "\n");
   PLOG_API_MSG("NX", setfill(' ') << "\n");

  //rd_data0.resize(1); 
  //rd_data1.resize(1); 

  for (int master_id=0; master_id < 8; master_id++) {

     switch (master_id) {
        case 0: mname = "PT_M"; 
                rd_data0[0] = 0x0;
                rd_data0[0] = cap_nx_block_read(0, RBM_BRIDGE_(PT_M_5_136_AM_CNTR0), 1, false, 1);
                rd_data1[0] = 0x0;
                rd_data1[0] = cap_nx_block_read(0, RBM_BRIDGE_(PT_M_5_136_AM_CNTR1), 1, false, 1);
                break;
        case 1: mname = "PR_M"; 
                rd_data0[0] = 0x0;
                rd_data0[0] = cap_nx_block_read(0, RBM_BRIDGE_(PR_M_4_141_AM_CNTR0), 1, false, 1);
                rd_data1[0] = 0x0;
                rd_data1[0] = cap_nx_block_read(0, RBM_BRIDGE_(PR_M_4_141_AM_CNTR1), 1, false, 1);
                break;
        case 2: mname = "AR_M";
                rd_data0[0] = 0x0;
                rd_data0[0] = cap_nx_block_read(0, RBM_BRIDGE_(AR_M_0_809_AM_CNTR0), 1, false, 1);
                rd_data1[0] = 0x0;
                rd_data1[0] = cap_nx_block_read(0, RBM_BRIDGE_(AR_M_0_809_AM_CNTR1), 1, false, 1);
                break;
        case 3: mname = "MD_M";
                rd_data0[0] = 0x0;
                rd_data0[0] = cap_nx_block_read(0, RBM_BRIDGE_(MD_M_1_138_AM_CNTR0), 1, false, 1);
                rd_data1[0] = 0x0;
                rd_data1[0] = cap_nx_block_read(0, RBM_BRIDGE_(MD_M_1_138_AM_CNTR1), 1, false, 1);
                break;
        case 4: mname = "MS_M";
                rd_data0[0] = 0x0;
                rd_data0[0] = cap_nx_block_read(0, RBM_BRIDGE_(MS_M_2_774_AM_CNTR0), 1, false, 1);
                rd_data1[0] = 0x0;
                rd_data1[0] = cap_nx_block_read(0, RBM_BRIDGE_(MS_M_2_774_AM_CNTR1), 1, false, 1);
                break;
        case 5: mname = "PB_M";
                rd_data0[0] = 0x0;
                rd_data0[0] = cap_nx_block_read(0, RBM_BRIDGE_(PB_M_3_150_AM_CNTR0), 1, false, 1);
                rd_data1[0] = 0x0;
                rd_data1[0] = cap_nx_block_read(0, RBM_BRIDGE_(PB_M_3_150_AM_CNTR1), 1, false, 1);
                break;
        case 6: mname = "SS_M";
                rd_data0[0] = 0x0;
                rd_data0[0] = cap_nx_block_read(0, RBM_BRIDGE_(SS_M_7_396_AM_CNTR0), 1, false, 1);
                rd_data1[0] = 0x0;
                rd_data1[0] = cap_nx_block_read(0, RBM_BRIDGE_(SS_M_7_396_AM_CNTR1), 1, false, 1);
                break;
        case 7: mname = "PX_M";
                rd_data0[0] = 0x0;
                rd_data0[0] = cap_nx_block_read(0, RBM_BRIDGE_(PX_M_6_786_AM_CNTR0), 1, false, 1);
                rd_data1[0] = 0x0;
                rd_data1[0] = cap_nx_block_read(0, RBM_BRIDGE_(PX_M_6_786_AM_CNTR1), 1, false, 1);
                break;
     }
     PLOG_API_MSG("NX", setw(10) << mname << setw(20) << rd_data0[0] << setw(20) << rd_data1[0] << "\n");

  }
}

void cap_nx_display_master_err_addr() { // 0=>PT, 1=>PR, 2=>AR, 3=>MD, 4=>MS, 5=>PB, 6 =>SS, 7=>PX Else All Counters

uint32_t rd_data0[2];
uint32_t rd_data1[2];
string mname = "";

   PLOG_API_MSG("NX", setfill('=') << setw(100) << "=" << "\n");
   PLOG_API_MSG("NX", setfill(' ') << "\n");
   PLOG_API_MSG("NX", setw(10) << "MASTER_NAME" << setw(20) << "ERR_RD_ADDR" << setw(20) << "ERR_WR_ADDR" << "\n");
   PLOG_API_MSG("NX", setfill('=') << setw(100) << "=" << "\n");
   PLOG_API_MSG("NX", setfill(' ') << "\n");

  for (int master_id=0; master_id < 8; master_id++) {

     switch (master_id) {
        case 0: mname = "PT_M"; 
                rd_data0[0] = 0x0;
                rd_data0[0] = cap_nx_block_read(0, RBM_BRIDGE_(PT_M_5_136_AM_ERA), 1, false, 1);
                rd_data1[0] = 0x0;
                rd_data1[0] = cap_nx_block_read(0, RBM_BRIDGE_(PT_M_5_136_AM_EWA), 1, false, 1);
                break;
        case 1: mname = "PR_M"; 
                rd_data0[0] = 0x0;
                rd_data0[0] = cap_nx_block_read(0, RBM_BRIDGE_(PR_M_4_141_AM_ERA), 1, false, 1);
                rd_data1[0] = 0x0;
                rd_data1[0] = cap_nx_block_read(0, RBM_BRIDGE_(PR_M_4_141_AM_EWA), 1, false, 1);
                break;
        case 2: mname = "AR_M";
                rd_data0[0] = 0x0;
                rd_data0[0] = cap_nx_block_read(0, RBM_BRIDGE_(AR_M_0_809_AM_ERA), 1, false, 1);
                rd_data1[0] = 0x0;
                rd_data1[0] = cap_nx_block_read(0, RBM_BRIDGE_(AR_M_0_809_AM_EWA), 1, false, 1);
                break;
        case 3: mname = "MD_M";
                rd_data0[0] = 0x0;
                rd_data0[0] = cap_nx_block_read(0, RBM_BRIDGE_(MD_M_1_138_AM_ERA), 1, false, 1);
                rd_data1[0] = 0x0;
                rd_data1[0] = cap_nx_block_read(0, RBM_BRIDGE_(MD_M_1_138_AM_EWA), 1, false, 1);
                break;
        case 4: mname = "MS_M";
                rd_data0[0] = 0x0;
                rd_data0[0] = cap_nx_block_read(0, RBM_BRIDGE_(MS_M_2_774_AM_ERA), 1, false, 1);
                rd_data1[0] = 0x0;
                rd_data1[0] = cap_nx_block_read(0, RBM_BRIDGE_(MS_M_2_774_AM_EWA), 1, false, 1);
                break;
        case 5: mname = "PB_M";
                rd_data0[0] = 0x0;
                rd_data0[0] = cap_nx_block_read(0, RBM_BRIDGE_(PB_M_3_150_AM_ERA), 1, false, 1);
                rd_data1[0] = 0x0;
                rd_data1[0] = cap_nx_block_read(0, RBM_BRIDGE_(PB_M_3_150_AM_EWA), 1, false, 1);
                break;
        case 6: mname = "SS_M";
                rd_data0[0] = 0x0;
                rd_data0[0] = cap_nx_block_read(0, RBM_BRIDGE_(SS_M_7_396_AM_ERA), 1, false, 1);
                rd_data1[0] = 0x0;
                rd_data1[0] = cap_nx_block_read(0, RBM_BRIDGE_(SS_M_7_396_AM_EWA), 1, false, 1);
                break;
        case 7: mname = "PX_M";
                rd_data0[0] = 0x0;
                rd_data0[0] = cap_nx_block_read(0, RBM_BRIDGE_(PX_M_6_786_AM_ERA), 1, false, 1);
                rd_data1[0] = 0x0;
                rd_data1[0] = cap_nx_block_read(0, RBM_BRIDGE_(PX_M_6_786_AM_EWA), 1, false, 1);
                break;
     }
     PLOG_API_MSG("NX", setw(10) << mname << setw(20) << rd_data0[0] << setw(20) << rd_data1[0] << "\n");
  }
}
#endif

// DAC2X
void cap_nx_add_secure_pt_to_ms(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) {

//vector<uint32_t> data0;
//vector<uint32_t> data1;
uint32_t data0[2];
uint32_t data1[2];

   //data0.resize(1); 
   //data1.resize(1); 

   static const struct {
       uint32_t adbase;
       uint32_t admask;
   } ranges[] = {
      { RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S0_0), RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S0_0) },
      { RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S1_0), RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S1_0) },
      { RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S2_0), RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S2_0) },
      { RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S3_0), RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S3_0) },
      { RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S4_0), RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S4_0) },
      { RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S5_0), RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S5_0) },
      { RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S6_0), RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S6_0) },
      { RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S7_0), RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S7_0) },
      { RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S8_0), RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S8_0) },
      { RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S9_0), RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S9_0) },
      { RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S10_0), RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S10_0) },
      { RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S11_0), RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S11_0) },
      { RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S12_0), RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S12_0) },
      { RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S13_0), RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S13_0) },
      { RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S14_0), RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S14_0) },
      { RBM_BRIDGE_(PT_M_5_136_AM_ADBASE_MEM_MS_S_MAP_MS_S15_0), RBM_BRIDGE_(PT_M_5_136_AM_ADMASK_MEM_MS_S_MAP_MS_S15_0) },
   };

   data0[0] = (adbase & 0xfffffffful) ;
   data1[0] = (admask & 0xfffffffful) ;
   cap_nx_block_write(0, ranges[range_num].adbase, 1, data0, true, 1);
   cap_nx_block_write(0, ranges[range_num].admask, 1, data1, true, 1);

   data0[0] = ((adbase & 0xff00000000ul) >> 32 );
   data1[0] = ((admask & 0xff00000000ul) >> 32 );
   cap_nx_block_write(0, ranges[range_num].adbase + 4, 1, data0, true, 1);
   cap_nx_block_write(0, ranges[range_num].admask + 4, 1, data1, true, 1);
}

// DAC2X
void cap_nx_add_secure_pr_to_ms(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) {

//vector<uint32_t> data0;
//vector<uint32_t> data1;
uint32_t data0[2];
uint32_t data1[2];

   //data0.resize(1); 
   //data1.resize(1); 

   static const struct {
       uint32_t adbase;
       uint32_t admask;
   } ranges[] = {
      { RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S0_0), RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S0_0) },
      { RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S1_0), RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S1_0) },
      { RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S2_0), RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S2_0) },
      { RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S3_0), RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S3_0) },
      { RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S4_0), RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S4_0) },
      { RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S5_0), RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S5_0) },
      { RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S6_0), RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S6_0) },
      { RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S7_0), RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S7_0) },
      { RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S8_0), RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S8_0) },
      { RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S9_0), RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S9_0) },
      { RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S10_0), RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S10_0) },
      { RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S11_0), RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S11_0) },
      { RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S12_0), RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S12_0) },
      { RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S13_0), RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S13_0) },
      { RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S14_0), RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S14_0) },
      { RBM_BRIDGE_(PR_M_4_141_AM_ADBASE_MEM_MS_S_MAP_MS_S15_0), RBM_BRIDGE_(PR_M_4_141_AM_ADMASK_MEM_MS_S_MAP_MS_S15_0) },
   };

   data0[0] = (adbase & 0xfffffffful) ;
   data1[0] = (admask & 0xfffffffful) ;
   cap_nx_block_write(0, ranges[range_num].adbase, 1, data0, true, 1);
   cap_nx_block_write(0, ranges[range_num].admask, 1, data1, true, 1);

   data0[0] = ((adbase & 0xff00000000ul) >> 32 );
   data1[0] = ((admask & 0xff00000000ul) >> 32 );
   cap_nx_block_write(0, ranges[range_num].adbase + 4, 1, data0, true, 1);
   cap_nx_block_write(0, ranges[range_num].admask + 4, 1, data1, true, 1);
}

// DAC2X
void cap_nx_add_secure_md_to_ms(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) {

//vector<uint32_t> data0;
//vector<uint32_t> data1;
uint32_t data0[2];
uint32_t data1[2];

   //daa0.resize(1); 
   //data1.resize(1); 

   static const struct {
       uint32_t adbase;
       uint32_t admask;
   } ranges[] = {
      { RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S0_0), RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S0_0) },
      { RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S1_0), RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S1_0) },
      { RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S2_0), RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S2_0) },
      { RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S3_0), RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S3_0) },
      { RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S4_0), RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S4_0) },
      { RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S5_0), RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S5_0) },
      { RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S6_0), RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S6_0) },
      { RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S7_0), RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S7_0) },
      { RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S8_0), RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S8_0) },
      { RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S9_0), RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S9_0) },
      { RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S10_0), RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S10_0) },
      { RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S11_0), RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S11_0) },
      { RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S12_0), RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S12_0) },
      { RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S13_0), RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S13_0) },
      { RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S14_0), RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S14_0) },
      { RBM_BRIDGE_(MD_M_1_138_AM_ADBASE_MEM_MS_S_MAP_MS_S15_0), RBM_BRIDGE_(MD_M_1_138_AM_ADMASK_MEM_MS_S_MAP_MS_S15_0) },
   };

   data0[0] = (adbase & 0xfffffffful) ;
   data1[0] = (admask & 0xfffffffful) ;
   cap_nx_block_write(0, ranges[range_num].adbase, 1, data0, true, 1);
   cap_nx_block_write(0, ranges[range_num].admask, 1, data1, true, 1);

   data0[0] = ((adbase & 0xff00000000ul) >> 32 );
   data1[0] = ((admask & 0xff00000000ul) >> 32 );
   cap_nx_block_write(0, ranges[range_num].adbase + 4, 1, data0, true, 1);
   cap_nx_block_write(0, ranges[range_num].admask + 4, 1, data1, true, 1);
}

// DAC2X
void cap_nx_add_secure_ss_to_ms(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) {

//vector<uint32_t> data0;
//vector<uint32_t> data1;
uint32_t data0[2];
uint32_t data1[2];

   //data0.resize(1); 
   //data1.resize(1); 

   static const struct {
       uint32_t adbase;
       uint32_t admask;
   } ranges[] = {
      { RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S0_0), RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S0_0) },
      { RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S1_0), RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S1_0) },
      { RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S2_0), RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S2_0) },
      { RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S3_0), RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S3_0) },
      { RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S4_0), RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S4_0) },
      { RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S5_0), RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S5_0) },
      { RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S6_0), RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S6_0) },
      { RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S7_0), RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S7_0) },
      { RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S8_0), RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S8_0) },
      { RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S9_0), RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S9_0) },
      { RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S10_0), RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S10_0) },
      { RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S11_0), RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S11_0) },
      { RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S12_0), RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S12_0) },
      { RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S13_0), RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S13_0) },
      { RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S14_0), RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S14_0) },
      { RBM_BRIDGE_(SS_M_7_396_AM_ADBASE_MEM_MS_S_MAP_MS_S15_0), RBM_BRIDGE_(SS_M_7_396_AM_ADMASK_MEM_MS_S_MAP_MS_S15_0) },
   };

   data0[0] = (adbase & 0xfffffffful) ;
   data1[0] = (admask & 0xfffffffful) ;
   cap_nx_block_write(0, ranges[range_num].adbase, 1, data0, true, 1);
   cap_nx_block_write(0, ranges[range_num].admask, 1, data1, true, 1);

   data0[0] = ((adbase & 0xff00000000ul) >> 32 );
   data1[0] = ((admask & 0xff00000000ul) >> 32 );
   cap_nx_block_write(0, ranges[range_num].adbase + 4, 1, data0, true, 1);
   cap_nx_block_write(0, ranges[range_num].admask + 4, 1, data1, true, 1);
}

// DAC2X
void cap_nx_add_secure_ar_to_ms(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) {

//vector<uint32_t> data0;
//vector<uint32_t> data1;
uint32_t data0[2];
uint32_t data1[2];

   //data0.resize(1); 
   //data1.resize(1); 

   static const struct {
       uint32_t adbase;
       uint32_t admask;
   } ranges[] = {
      { RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S0_0), RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S0_0) },
      { RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S1_0), RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S1_0) },
      { RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S2_0), RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S2_0) },
      { RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S3_0), RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S3_0) },
      { RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S4_0), RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S4_0) },
      { RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S5_0), RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S5_0) },
      { RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S6_0), RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S6_0) },
      { RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S7_0), RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S7_0) },
      { RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S8_0), RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S8_0) },
      { RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S9_0), RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S9_0) },
      { RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S10_0), RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S10_0) },
      { RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S11_0), RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S11_0) },
      { RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S12_0), RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S12_0) },
      { RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S13_0), RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S13_0) },
      { RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S14_0), RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S14_0) },
      { RBM_BRIDGE_(AR_M_0_809_AM_ADBASE_MEM_MS_S_MAP_MS_S15_0), RBM_BRIDGE_(AR_M_0_809_AM_ADMASK_MEM_MS_S_MAP_MS_S15_0) },
   };

   data0[0] = (adbase & 0xfffffffful) ;
   data1[0] = (admask & 0xfffffffful) ;
   cap_nx_block_write(0, ranges[range_num].adbase, 1, data0, true, 1);
   cap_nx_block_write(0, ranges[range_num].admask, 1, data1, true, 1);

   data0[0] = ((adbase & 0xff00000000ul) >> 32 );
   data1[0] = ((admask & 0xff00000000ul) >> 32 );
   cap_nx_block_write(0, ranges[range_num].adbase + 4, 1, data0, true, 1);
   cap_nx_block_write(0, ranges[range_num].admask + 4, 1, data1, true, 1);
}


// DAC2X
void cap_nx_add_secure_pb_to_ms(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) {

//vector<uint32_t> data0;
//vector<uint32_t> data1;
uint32_t data0[2];
uint32_t data1[2];

   //data0.resize(1); 
   //data1.resize(1); 

   static const struct {
       uint32_t adbase;
       uint32_t admask;
   } ranges[] = {
      { RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S0_0), RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S0_0) },
      { RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S1_0), RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S1_0) },
      { RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S2_0), RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S2_0) },
      { RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S3_0), RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S3_0) },
      { RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S4_0), RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S4_0) },
      { RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S5_0), RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S5_0) },
      { RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S6_0), RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S6_0) },
      { RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S7_0), RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S7_0) },
      { RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S8_0), RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S8_0) },
      { RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S9_0), RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S9_0) },
      { RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S10_0), RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S10_0) },
      { RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S11_0), RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S11_0) },
      { RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S12_0), RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S12_0) },
      { RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S13_0), RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S13_0) },
      { RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S14_0), RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S14_0) },
      { RBM_BRIDGE_(PB_M_3_150_AM_ADBASE_MEM_MS_S_MAP_MS_S15_0), RBM_BRIDGE_(PB_M_3_150_AM_ADMASK_MEM_MS_S_MAP_MS_S15_0) },
   };

   data0[0] = (adbase & 0xfffffffful) ;
   data1[0] = (admask & 0xfffffffful) ;
   cap_nx_block_write(0, ranges[range_num].adbase, 1, data0, true, 1);
   cap_nx_block_write(0, ranges[range_num].admask, 1, data1, true, 1);

   data0[0] = ((adbase & 0xff00000000ul) >> 32 );
   data1[0] = ((admask & 0xff00000000ul) >> 32 );
   cap_nx_block_write(0, ranges[range_num].adbase + 4, 1, data0, true, 1);
   cap_nx_block_write(0, ranges[range_num].admask + 4, 1, data1, true, 1);
}

// DAC2X
void cap_nx_add_secure_ms_to_ms(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) {

//vector<uint32_t> data0;
//vector<uint32_t> data1;
uint32_t data0[2];
uint32_t data1[2];

   //data0.resize(1); 
   //data1.resize(1); 

   static const struct {
       uint32_t adbase;
       uint32_t admask;
   } ranges[] = {
      { RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S0_0), RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S0_0) },
      { RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S1_0), RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S1_0) },
      { RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S2_0), RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S2_0) },
      { RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S3_0), RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S3_0) },
      { RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S4_0), RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S4_0) },
      { RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S5_0), RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S5_0) },
      { RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S6_0), RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S6_0) },
      { RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S7_0), RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S7_0) },
      { RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S8_0), RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S8_0) },
      { RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S9_0), RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S9_0) },
      { RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S10_0), RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S10_0) },
      { RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S11_0), RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S11_0) },
      { RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S12_0), RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S12_0) },
      { RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S13_0), RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S13_0) },
      { RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S14_0), RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S14_0) },
      { RBM_BRIDGE_(MS_M_2_774_AM_ADBASE_MEM_MS_S_MAP_MS_S15_0), RBM_BRIDGE_(MS_M_2_774_AM_ADMASK_MEM_MS_S_MAP_MS_S15_0) },
   };

   data0[0] = (adbase & 0xfffffffful) ;
   data1[0] = (admask & 0xfffffffful) ;
   cap_nx_block_write(0, ranges[range_num].adbase, 1, data0, true, 1);
   cap_nx_block_write(0, ranges[range_num].admask, 1, data1, true, 1);

   data0[0] = ((adbase & 0xff00000000ul) >> 32 );
   data1[0] = ((admask & 0xff00000000ul) >> 32 );
   cap_nx_block_write(0, ranges[range_num].adbase + 4, 1, data0, true, 1);
   cap_nx_block_write(0, ranges[range_num].admask + 4, 1, data1, true, 1);
}

// DAC2X
void cap_nx_add_secure_px_to_ms(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) {

//vector<uint32_t> data0;
//vector<uint32_t> data1;
uint32_t data0[2];
uint32_t data1[2];

   //data0.resize(1); 
   //data1.resize(1); 

   static const struct {
       uint32_t adbase;
       uint32_t admask;
   } ranges[] = {
      { RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S0_0), RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S0_0) },
      { RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S1_0), RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S1_0) },
      { RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S2_0), RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S2_0) },
      { RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S3_0), RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S3_0) },
      { RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S4_0), RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S4_0) },
      { RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S5_0), RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S5_0) },
      { RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S6_0), RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S6_0) },
      { RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S7_0), RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S7_0) },
      { RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S8_0), RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S8_0) },
      { RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S9_0), RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S9_0) },
      { RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S10_0), RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S10_0) },
      { RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S11_0), RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S11_0) },
      { RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S12_0), RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S12_0) },
      { RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S13_0), RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S13_0) },
      { RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S14_0), RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S14_0) },
      { RBM_BRIDGE_(PX_M_6_786_AM_ADBASE_MEM_MS_S_MAP_MS_S15_0), RBM_BRIDGE_(PX_M_6_786_AM_ADMASK_MEM_MS_S_MAP_MS_S15_0) },
   };

   data0[0] = (adbase & 0xfffffffful) ;
   data1[0] = (admask & 0xfffffffful) ;
   cap_nx_block_write(0, ranges[range_num].adbase, 1, data0, true, 1);
   cap_nx_block_write(0, ranges[range_num].admask, 1, data1, true, 1);

   data0[0] = ((adbase & 0xff00000000ul) >> 32 );
   data1[0] = ((admask & 0xff00000000ul) >> 32 );
   cap_nx_block_write(0, ranges[range_num].adbase + 4, 1, data0, true, 1);
   cap_nx_block_write(0, ranges[range_num].admask + 4, 1, data1, true, 1);
}
