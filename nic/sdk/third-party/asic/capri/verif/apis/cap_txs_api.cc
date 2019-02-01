#include "cap_txs_api.h"
#include "msg_man.h"

#define DTDM_CALENDAR_SIZE 64

#define SCHGRP_SIZE_BYTE    16*64 // 8k bits
#define HBM_LINE_SIZE_BYTE  64 // number of bytes/line in hbm
#define TMR_HASH_NUMB       17 // 16 + 1


// soft reset sequence 
void cap_txs_soft_reset(int chip_id, int inst_id) {

   //msg_man::init();
   //MSG_STREAM_COUT.set_level("/", msg_stream::debug);

   PLOG_MSG ("Dumping Sknob vars" << std::endl);
   //sknobs_dump();

   PLOG_MSG("inside softreset\n");
   //cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);
}

// soft reset, value = 1 means apply reset, value = 0 release soft reset
void cap_txs_set_soft_reset(int chip_id, int inst_id, int value) {
}

// init start, no polling yet
void cap_txs_init_start(int chip_id, int inst_id) {
   cpp_int       temp;
   string        tmp_str;
   stringstream  tmp_ss;
   cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);
   string        hier_path_str = txs_csr.get_hier_path();

   PLOG_MSG("inside init start\n");


   cpp_int sram_init;
   cpp_int hbm_init;
   int     hbm_mem_backdoor_done;

   cpp_int tmr_sram_init;
   cpp_int tmr_hbm_init;
   cpp_int ftmr_enable;
   cpp_int stmr_enable;
   cpp_int ftmr_pause;
   cpp_int stmr_pause;
   cpp_int ftmr_tick;
   cpp_int stmr_tick;
   cpp_int tmr_hbm_base;
   cpp_int tmr_hsh_depth;
   cpp_int tmr_wheel_depth;

   cpp_int sch_grp_depth;
   cpp_int hbm_base;

   cpp_int dtdm_lo_map;
   cpp_int dtdm_hi_map;
   cpp_int dtdmlo_calendar[DTDM_CALENDAR_SIZE];
   cpp_int dtdmhi_calendar[DTDM_CALENDAR_SIZE];

   std::replace( hier_path_str.begin(), hier_path_str.end(), '.', '/');
   std::replace( hier_path_str.begin(), hier_path_str.end(), '[', '/');
   std::replace( hier_path_str.begin(), hier_path_str.end(), ']', '/');
   tmp_ss.str("");
   tmp_ss << hier_path_str << "/cfw_scheduler_static";
   tmp_str = tmp_ss.str();
   PLOG_MSG("tmp_str = " << tmp_str << endl);
   txs_csr.cfw_scheduler_static.read();
   if (sknobs_exists((char *)tmp_str.c_str())) {
       temp.assign(sknobs_get_string((char *)tmp_str.c_str(), (char *)""));
       txs_csr.cfw_scheduler_static.all(temp);
       txs_csr.cfw_scheduler_static.show();
       txs_csr.cfw_scheduler_static.write();
   }
   tmp_ss.str("");
   tmp_ss << hier_path_str << "/cfg_timer_static";
   tmp_str = tmp_ss.str();
   PLOG_MSG("tmp_str = " << tmp_str << endl);
   txs_csr.cfg_timer_static.read();
   if (sknobs_exists((char *)tmp_str.c_str())) {
       temp.assign(sknobs_get_string((char *)tmp_str.c_str(), (char *)""));
       txs_csr.cfg_timer_static.all(temp);
       txs_csr.cfg_timer_static.show();
       txs_csr.cfg_timer_static.write();
   }
   tmp_ss.str("");
   tmp_ss << hier_path_str << "/cfg_timer_dbg2";
   tmp_str = tmp_ss.str();
   PLOG_MSG("tmp_str = " << tmp_str << endl);
   txs_csr.cfg_timer_dbg2.read();
   if (sknobs_exists((char *)tmp_str.c_str())) {
       temp.assign(sknobs_get_string((char *)tmp_str.c_str(), (char *)""));
       txs_csr.cfg_timer_dbg2.max_bcnt(temp);
       txs_csr.cfg_timer_dbg2.show();
       txs_csr.cfg_timer_dbg2.write();
   }

   // hbm init done in +fill_pattern=0
   hbm_mem_backdoor_done = 0;
   tmp_ss.str("");;
   tmp_ss << "fill_pattern";
   tmp_str = tmp_ss.str();
   if (sknobs_exists((char *)tmp_str.c_str())) {
      hbm_mem_backdoor_done = 1;
      PLOG_MSG("tmp_str = " << tmp_str << " hbm_mem_backdoor_done:" << hbm_mem_backdoor_done << endl);
   }

   if( (cpu::access()->get_access_type() == back_door_e) ) {
      txs_csr.cfg_tmr_cnt_sram.read();
      txs_csr.cfg_tmr_cnt_sram.ecc_disable_cor(1);
      txs_csr.cfg_tmr_cnt_sram.ecc_disable_det(1);
      txs_csr.cfg_tmr_cnt_sram.write();
      txs_csr.cfg_tmr_cnt_sram.read();

      txs_csr.cfg_sch_lif_map_sram.read();
      txs_csr.cfg_sch_lif_map_sram.ecc_disable_cor(1);
      txs_csr.cfg_sch_lif_map_sram.ecc_disable_det(1);
      txs_csr.cfg_sch_lif_map_sram.write();
      txs_csr.cfg_sch_lif_map_sram.read();

      txs_csr.cfg_sch_rlid_map_sram.read();
      txs_csr.cfg_sch_rlid_map_sram.ecc_disable_cor(1);
      txs_csr.cfg_sch_rlid_map_sram.ecc_disable_det(1);
      txs_csr.cfg_sch_rlid_map_sram.write();
      txs_csr.cfg_sch_rlid_map_sram.read();

      txs_csr.cfg_sch_grp_sram.read();
      txs_csr.cfg_sch_grp_sram.ecc_disable_cor(1);
      txs_csr.cfg_sch_grp_sram.ecc_disable_det(1);
      txs_csr.cfg_sch_grp_sram.write();
      txs_csr.cfg_sch_grp_sram.read();
   }
 
   // bypass mem init
   hbm_init.assign(SKNOBS_GET("cap0/txs/cfw_scheduler_static/hbm_hw_init", 1));
   sram_init.assign(SKNOBS_GET("cap0/txs/cfw_scheduler_static/sram_hw_init", 1));
   if (SKNOBS_GET("sam_only",0)) {
     hbm_init.assign(0);
     sram_init.assign(0);
   }

   // tmr
   tmr_hbm_init.assign(SKNOBS_GET("cap0/txs/cfw_timer_glb/hbm_hw_init", 0));
   tmr_sram_init.assign(SKNOBS_GET("cap0/txs/cfw_timer_glb/sram_hw_init", 0));
   ftmr_enable.assign(SKNOBS_GET("cap0/txs/cfw_timer_glb/ftmr_enable", 0));
   stmr_enable.assign(SKNOBS_GET("cap0/txs/cfw_timer_glb/stmr_enable", 0));

   // dtdm : default all priority mapped to low
   // txs_csr.dhs_dtdmlo_calendar.get_depth_entry()
   int cnt = 0;
   dtdm_lo_map.assign(SKNOBS_GET("cap0/txs/cfg_sch/dtdm_lo_map", 0xffff));
   dtdm_hi_map.assign(SKNOBS_GET("cap0/txs/cfg_sch/dtdm_hi_map", 0x0));
   for (int i=0; i<DTDM_CALENDAR_SIZE; i++) {
      tmp_ss.str("");;
      tmp_ss << "cap0/txs/dhs_dtdmlo_calendar/entry/" << i << "/dtdm_calendar";
      tmp_str = tmp_ss.str();
      dtdmlo_calendar[i].assign(SKNOBS_GET((char *)tmp_str.c_str(), cnt%16));

      tmp_ss.str("");;
      tmp_ss << "cap0/txs/dhs_dtdmhi_calendar/entry/" << i << "/dtdm_calendar";
      tmp_str = tmp_ss.str();
      dtdmhi_calendar[i].assign(SKNOBS_GET((char *)tmp_str.c_str(), cnt%16));

      //dtdmlo_calendar[i].assign(SKNOBS_GET("cap0/txs/dhs_dtdmlo_calendar/entry/"+itos(i)+"/dtdm_calendar", cnt%16));
      //dtdmhi_calendar[i].assign(SKNOBS_GET("cap0/txs/dhs_dtdmhi_calendar/entry/"+itos(i)+"/dtdm_calendar", cnt%16));
      cnt++;
   }

   txs_csr.cfw_scheduler_glb.read();
   hbm_base = txs_csr.cfw_scheduler_static.hbm_base();
   PLOG_MSG("cfw_scheduler_static.hbm_base :0x" << hex << hbm_base << dec << endl);
   if (hbm_base >= 0x80000000) {
     txs_csr.cfw_scheduler_glb.hbm_hw_init(hbm_init);
   }
   txs_csr.cfw_scheduler_glb.sram_hw_init(sram_init);
   // txs_csr.cfw_scheduler_glb.enable(1);
   txs_csr.cfw_scheduler_glb.show();
   // txs_csr.cfw_scheduler_glb.enable_set_lkup(0);
   txs_csr.cfw_scheduler_glb.enable_set_byp(0);
   txs_csr.cfw_scheduler_glb.write();
   
   txs_csr.cfg_timer_static.read();
   tmr_hbm_base = txs_csr.cfg_timer_static.hbm_base();
   PLOG_MSG("cfg_timer_static.hbm_base :0x" << hex << tmr_hbm_base << dec << endl);
   // Workaround to add one to the depth during hwinit
   tmr_hsh_depth = txs_csr.cfg_timer_static.tmr_hsh_depth();
   txs_csr.cfg_timer_static.tmr_hsh_depth(tmr_hsh_depth+1);
   txs_csr.cfg_timer_static.show();
   txs_csr.cfg_timer_static.write();
   
   txs_csr.cfw_timer_glb.read();
   if (tmr_hbm_base >= 0x80000000) {
      txs_csr.cfw_timer_glb.hbm_hw_init(tmr_hbm_init);
      PLOG_MSG("inside txs init start : tmr_hbm_base:0x" << hex << tmr_hbm_base << dec << endl);
   } else {
      PLOG_MSG("inside txs init start : illegal tmr_hbm_base:0x" << hex << tmr_hbm_base << dec << endl);
   }
   txs_csr.cfw_timer_glb.sram_hw_init(tmr_sram_init);
   // Need to wait before turning this on until the init is done
   // txs_csr.cfw_timer_glb.ftmr_enable(ftmr_enable);
   // txs_csr.cfw_timer_glb.stmr_enable(stmr_enable);
   txs_csr.cfw_timer_glb.show();
   txs_csr.cfw_timer_glb.write();
   

//   // backdoor init for hbm
//   // hbm init done in +fill_pattern=0 if hbm_mem_backdoor_done==1
//   hbm_base = txs_csr.cfw_scheduler_static.hbm_base();
//   sch_grp_depth = txs_csr.cfw_scheduler_static.sch_grp_depth();
//   if (hbm_init == 0 && hbm_mem_backdoor_done == 0) {
//      unsigned char *data;
//      data = new unsigned char[SCHGRP_SIZE_BYTE];
//      for (int i=0; i<SCHGRP_SIZE_BYTE; i++) {
//         data[i] = 0x0;
//      }
//      for (int i=0; i<sch_grp_depth; i++) {
//#ifndef CAPRI_HAL
//         cap_mem_access::instancep()->write(uint64_t (hbm_base + i*SCHGRP_SIZE_BYTE), data, SCHGRP_SIZE_BYTE);
//#endif
//      } 
//      delete [] data;
//   }
//   if ((tmr_hbm_init == 0) && (stmr_enable == 1 || ftmr_enable == 1) && hbm_mem_backdoor_done == 0) {
//      unsigned char *data;
//      data = new unsigned char[HBM_LINE_SIZE_BYTE];
//      for (int i=0; i<HBM_LINE_SIZE_BYTE; i++) {
//         data[i] = 0x0;
//      }
//      for (int i=0; i<(tmr_hsh_depth*TMR_HASH_NUMB); i++) {
//#ifndef CAPRI_HAL
//         cap_mem_access::instancep()->write(uint64_t (tmr_hbm_base + i*HBM_LINE_SIZE_BYTE), data, HBM_LINE_SIZE_BYTE);
//#endif
//      } 
//      delete [] data;
//   }

   for (int i = 0; i <DTDM_CALENDAR_SIZE ; i++) {
     txs_csr.dhs_dtdmlo_calendar.entry[i].read();
     txs_csr.dhs_dtdmlo_calendar.entry[i].dtdm_calendar(dtdmlo_calendar[i]);
     txs_csr.dhs_dtdmlo_calendar.entry[i].show();
     txs_csr.dhs_dtdmlo_calendar.entry[i].write();
   }
   for (int i = 0; i <DTDM_CALENDAR_SIZE ; i++) {
     txs_csr.dhs_dtdmhi_calendar.entry[i].read();
     txs_csr.dhs_dtdmhi_calendar.entry[i].dtdm_calendar(dtdmhi_calendar[i]);
     txs_csr.dhs_dtdmhi_calendar.entry[i].show();
     txs_csr.dhs_dtdmhi_calendar.entry[i].write();
   }
     
   txs_csr.cfg_sch.read();
   txs_csr.cfg_sch.dtdm_lo_map(dtdm_lo_map);
   txs_csr.cfg_sch.dtdm_hi_map(dtdm_hi_map);
   txs_csr.cfg_sch.timeout(0);
   txs_csr.cfg_sch.pause(0);
   txs_csr.cfg_sch.enable(1);
   txs_csr.cfg_sch.write();

   txs_csr.sta_sch_max_hbm_byp.set_access_no_zero_time(1);
   txs_csr.sta_tmr_max_hbm_byp.set_access_no_zero_time(1);
   txs_csr.sta_tmr_max_keys.set_access_no_zero_time(1);
   txs_csr.sta_stmr_max_bcnt.set_access_no_zero_time(1);
   txs_csr.sta_ftmr_max_bcnt.set_access_no_zero_time(1);

  for(int i = 0; i < txs_csr.dhs_doorbell.get_depth_entry(); i++) {
    txs_csr.dhs_doorbell.entry[i].set_access_no_zero_time(1);
  }

   PLOG_MSG("inside init start done\n");
}

// poll for init done
void cap_txs_init_done(int chip_id, int inst_id) {

   int     count;
   int     done;
   cpp_int sch_grp_depth;
   cpp_int tmr_hsh_depth;
   cpp_int ftmr_enable;
   cpp_int stmr_enable;
   int tmr_wheel_depth;

   // Tmr
   ftmr_enable.assign(SKNOBS_GET("cap0/txs/cfw_timer_glb/ftmr_enable", 0));
   stmr_enable.assign(SKNOBS_GET("cap0/txs/cfw_timer_glb/stmr_enable", 0));

   PLOG_MSG("inside init done\n");
   cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);
   txs_csr.sta_scheduler.read();
   txs_csr.cfw_scheduler_glb.read();
   txs_csr.cfw_scheduler_glb.hbm_hw_init();
   txs_csr.cfw_scheduler_glb.sram_hw_init();
   PLOG_MSG("cap_txs_init_done: hbm_hw_init:" << txs_csr.cfw_scheduler_glb.hbm_hw_init() << " sram_hw_init:" << txs_csr.cfw_scheduler_glb.sram_hw_init() << endl);

   txs_csr.cfw_timer_glb.read();
   txs_csr.cfg_timer_static.read();
   txs_csr.cfw_scheduler_static.read();
   txs_csr.cfw_timer_glb.show();
   txs_csr.cfg_timer_static.show();
   txs_csr.cfw_scheduler_static.show();

   count = 0;
   done  = 0;
   sch_grp_depth = txs_csr.cfw_scheduler_static.sch_grp_depth();
   tmr_hsh_depth = 17 * txs_csr.cfg_timer_static.tmr_hsh_depth();
   int max_wait_done = 10* (int)sch_grp_depth;
   if (tmr_hsh_depth > sch_grp_depth) {
      max_wait_done = 10* (int)tmr_hsh_depth;
   }

   // replace SLEEP with non zero time access
   txs_csr.sta_scheduler.set_access_no_zero_time(1);
   txs_csr.sta_timer.set_access_no_zero_time(1);
   while (!done) {
      txs_csr.sta_scheduler.read();
      txs_csr.sta_timer.read();
      if ( (txs_csr.sta_scheduler.hbm_init_done()  || (txs_csr.cfw_scheduler_glb.hbm_hw_init()  == 0))  && 
           (txs_csr.sta_scheduler.sram_init_done() || (txs_csr.cfw_scheduler_glb.sram_hw_init() == 0))  &&
           (txs_csr.sta_timer.hbm_init_done()      || (txs_csr.cfw_timer_glb.hbm_hw_init()  == 0))      &&
           (txs_csr.sta_timer.sram_init_done()     || (txs_csr.cfw_timer_glb.sram_hw_init() == 0))      ) {
         done = 1;
      } else if (count > max_wait_done) {
         PLOG_MSG("ERROR: cap_txs_init_done: timeout hbm_init_done:" << txs_csr.sta_scheduler.hbm_init_done() << " sram_init_done:" << txs_csr.sta_scheduler.sram_init_done() << " tmr hbm_init_done:" << txs_csr.sta_timer.hbm_init_done() << " tmr sram_init_done:" << txs_csr.sta_timer.sram_init_done() << endl);
         done = 1;
      } else {
         count++;
         //SLEEP(100);
      }
   }

   // Workaround to add one to the depth during hwinit
   // Restore original depth now
   tmr_hsh_depth = txs_csr.cfg_timer_static.tmr_hsh_depth();
   txs_csr.cfg_timer_static.tmr_hsh_depth(tmr_hsh_depth-1);
   txs_csr.cfg_timer_static.show();
   txs_csr.cfg_timer_static.write();
   // Manually init last entry since HW_INIT miss that one
   tmr_wheel_depth = (int) txs_csr.cfg_timer_static.tmr_wheel_depth();
   txs_csr.dhs_tmr_cnt_sram.entry[tmr_wheel_depth].slow_cbcnt(0);
   txs_csr.dhs_tmr_cnt_sram.entry[tmr_wheel_depth].slow_bcnt(0);
   txs_csr.dhs_tmr_cnt_sram.entry[tmr_wheel_depth].slow_lcnt(0);
   txs_csr.dhs_tmr_cnt_sram.entry[tmr_wheel_depth].fast_cbcnt(0);
   txs_csr.dhs_tmr_cnt_sram.entry[tmr_wheel_depth].fast_bcnt(0);
   txs_csr.dhs_tmr_cnt_sram.entry[tmr_wheel_depth].fast_lcnt(0);
   txs_csr.dhs_tmr_cnt_sram.entry[tmr_wheel_depth].write();

   PLOG_MSG("cap_txs_init_done: hbm_init_done:" << txs_csr.sta_scheduler.hbm_init_done() << " sram_init_done:" << txs_csr.sta_scheduler.sram_init_done() << " tmr hbm_init_done:" << txs_csr.sta_timer.hbm_init_done() << " tmr sram_init_done:" << txs_csr.sta_timer.sram_init_done() << endl);

   // Clear IRQ here:
   txs_csr.int_sch.intreg.read();
   txs_csr.int_sch.intreg.sch_wid_empty_interrupt(1);
   txs_csr.int_sch.intreg.write();

   txs_csr.int_tmr.intreg.read();
   txs_csr.int_tmr.intreg.tmr_wid_empty_interrupt(1);
   txs_csr.int_tmr.intreg.write();


   // Enable Scheduler and Timer now
   txs_csr.cfw_scheduler_glb.read();
   txs_csr.cfw_scheduler_glb.hbm_hw_init(0);
   txs_csr.cfw_scheduler_glb.sram_hw_init(0);
   txs_csr.cfw_scheduler_glb.enable(1);
   txs_csr.cfw_scheduler_glb.show();
   txs_csr.cfw_scheduler_glb.write();

   txs_csr.cfw_timer_glb.read();
   txs_csr.cfw_timer_glb.hbm_hw_init(0);
   txs_csr.cfw_timer_glb.sram_hw_init(0);
   txs_csr.cfw_timer_glb.ftmr_enable(ftmr_enable);
   txs_csr.cfw_timer_glb.stmr_enable(stmr_enable);
   txs_csr.cfw_timer_glb.show();
   txs_csr.cfw_timer_glb.write();

   //sch_grp_depth.assign(sknobs_get_value("cap0.txs.sch_grp_depth", 10));

   // txs_csr.dhs_sch_grp_entry.get_depth_entry();
   //for (int i = 0; i < sch_grp_depth; i++) {
   //  txs_csr.dhs_sch_grp_entry.entry[i].auto_clr(1);
     // TODO: need fix lif
   //  txs_csr.dhs_sch_grp_entry.entry[i].lif(1);
   //  txs_csr.dhs_sch_grp_entry.entry[i].write();
   //}

   txs_csr.cfg_glb.read();
   txs_csr.cfg_glb.show();
   PLOG_MSG("inside init done done\n");
}


// use sknobs base load cfg 
void cap_txs_load_from_cfg(int chip_id, int inst_id) {   
    cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, 0, 0);
    PLOG_MSG("start load txs cfg...." << endl)

    for (int i = 0; i < 2048; i++) {
      txs_csr.dhs_sch_lif_map_sram.entry[i].set_reset_val((0x4000 + i*8) | 1<<2);
    }

    txs_csr.load_from_cfg();

    // flush out all the write, read scratch
    txs_csr.dhs_sch_grp_entry.entry[0].read();
    txs_csr.base.read(); 

    PLOG_MSG("start load txs cfg done...." << endl)
}

#ifndef CAPRI_HAL
// eos check, this functions should call eos_cnt, eos_int, eos_sta 
void cap_txs_eos(int chip_id, int inst_id) {

   PLOG_MSG("inside eos check\n");

   cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, 0, 0);
   string        hier_path_str = txs_csr.get_hier_path();
   string        tmp_str;
   stringstream  tmp_ss;
   cpp_int       hbm_init;
   cpp_int       tmr_hbm_init;
   cpp_int       hbm_base;
   cpp_int       sch_grp_depth;
   cpp_int       tmr_hbm_base;
   cpp_int       tmr_hsh_depth;
   cpp_int       tmr_wheel_depth;
   int           eos_pass;

   cpp_int       ftmr_enable_d = txs_csr.cfw_timer_glb.ftmr_enable().convert_to<int>();
   cpp_int       stmr_enable_d = txs_csr.cfw_timer_glb.stmr_enable().convert_to<int>();

   std::replace( hier_path_str.begin(), hier_path_str.end(), '.', '/');
   std::replace( hier_path_str.begin(), hier_path_str.end(), '[', '/');
   std::replace( hier_path_str.begin(), hier_path_str.end(), ']', '/');

   hbm_base = txs_csr.cfw_scheduler_static.hbm_base();
   sch_grp_depth = txs_csr.cfw_scheduler_static.sch_grp_depth();
   tmr_wheel_depth = txs_csr.cfg_timer_static.tmr_wheel_depth();

   if (SKNOBS_GET("cap0/txs/cfw_scheduler_static/sch_grp_depth_override", 0)) {
     sch_grp_depth = 0; 
   }

   cpp_int dol_enable_eos_timers_chk;
   dol_enable_eos_timers_chk.assign(SKNOBS_GET("dol_enable_eos_timers_chk", 1));
   PLOG_MSG("sw use: dol_enable_eos_timers_chk = " << dol_enable_eos_timers_chk << endl)

   //sch_grp_depth = 10; // if back_door is not used
   // disble timer sm first
   txs_csr.cfw_timer_glb.read();
   txs_csr.cfw_timer_glb.ftmr_enable(0);
   txs_csr.cfw_timer_glb.stmr_enable(0);
   txs_csr.cfw_timer_glb.write();
   // now u can read the status

   txs_csr.sta_sch_max_hbm_byp.read();
   txs_csr.sta_tmr_max_hbm_byp.read();
   txs_csr.sta_tmr_max_keys.read();
   txs_csr.sta_ftmr_max_bcnt.read();
   txs_csr.sta_stmr_max_bcnt.read();
   
   cpp_int schHbmByp = txs_csr.sta_sch_max_hbm_byp.cnt();
   cpp_int tmrHbmByp = txs_csr.sta_tmr_max_hbm_byp.cnt();
   cpp_int tmrKeysNum = txs_csr.sta_tmr_max_keys.cnt();
   cpp_int ftmrBcntNum = txs_csr.sta_ftmr_max_bcnt.bcnt();
   cpp_int stmrBcntNum = txs_csr.sta_stmr_max_bcnt.bcnt();

   PLOG_MSG("Scheduler HBM Byp Max usage = " << schHbmByp << endl)
   PLOG_MSG("Timer HBM Byp Max usage = " << tmrHbmByp << endl)
   PLOG_MSG("Timer Keys Num Max usage = " << tmrKeysNum << endl)
   PLOG_MSG("Fast Timer BCNT Num Max usage = " << ftmrBcntNum << endl)
   PLOG_MSG("Slow Timer BCNT Num Max usage = " << stmrBcntNum << endl)

   txs_csr.sta_scheduler_dbg2.read();
   txs_csr.sta_timer_dbg2.read();

   cpp_int schWrPending = txs_csr.sta_scheduler_dbg2.hbm_wr_pending_cnt();
   cpp_int tmrWrPending = txs_csr.sta_timer_dbg2.hbm_wr_pending_cnt();
   cpp_int schRdPending = txs_csr.sta_scheduler_dbg2.hbm_rd_pending_cnt();
   cpp_int tmrRdPending = txs_csr.sta_timer_dbg2.hbm_rd_pending_cnt();
   cpp_int schBypPending = txs_csr.sta_scheduler_dbg2.hbm_byp_pending_cnt();
   cpp_int tmrBypPending = txs_csr.sta_timer_dbg2.hbm_byp_pending_cnt();

   if (schWrPending != 0) {
     PLOG_MSG("ERROR: sta_scheduler_dbg2.hbm_wr_pending_cnt = " << schWrPending << endl)   
   } else {
     PLOG_MSG("OK sta_scheduler_dbg2.hbm_wr_pending_cnt = " << schWrPending << endl);
   }
   
   if (tmrWrPending != 0) {
     PLOG_MSG("ERROR: sta_timer_dbg2.hbm_wr_pending_cnt = " << tmrWrPending << endl)   
   } else {
     PLOG_MSG("OK sta_timer_dbg2.hbm_wr_pending_cnt = " << tmrWrPending << endl);
   }
   
   if (schRdPending != 0) {
     PLOG_MSG("ERROR: sta_scheduler_dbg2.hbm_rd_pending_cnt = " << schRdPending << endl)   
   } else {
     PLOG_MSG("OK sta_scheduler_dbg2.hbm_rd_pending_cnt = " << schRdPending << endl);
   }
   
   if (tmrRdPending != 0) {
     PLOG_MSG("ERROR: sta_timer_dbg2.hbm_rd_pending_cnt = " << tmrRdPending << endl)   
   } else {
     PLOG_MSG("OK sta_timer_dbg2.hbm_rd_pending_cnt = " << tmrRdPending << endl);
   }
   
   if (schBypPending != 0) {
     PLOG_MSG("ERROR: sta_scheduler_dbg2.hbm_byp_pending_cnt = " << schBypPending << endl)   
   } else {
     PLOG_MSG("OK sta_scheduler_dbg2.hbm_byp_pending_cnt = " << schBypPending << endl);
   }
   
   if (tmrBypPending != 0) {
     PLOG_MSG("ERROR: sta_timer_dbg2.hbm_byp_pending_cnt = " << tmrBypPending << endl)   
   } else {
     PLOG_MSG("OK sta_timer_dbg2.hbm_byp_pending_cnt = " << tmrBypPending << endl);
   }
   
   //cpu_access_type_e cur_cpu_access_type = cpu::access()->get_access_type();
   //PLOG_MSG("do_main : current cpu_access type : " << cur_cpu_access_type << " setting to front door in main_phase" << endl)
   //cpu::access()->set_access_type(back_door_e);

   eos_pass = 1;
//    for (int i=0; i<sch_grp_depth; i++) {
//      txs_csr.dhs_sch_grp_cnt_entry.entry[i].read();
//      cpp_int popcnt;         
//      cpp_int drb_cnt;         
//      popcnt = txs_csr.dhs_sch_grp_cnt_entry.entry[i].popcnt();
//      drb_cnt = txs_csr.dhs_sch_grp_cnt_entry.entry[i].drb_cnt();
//      PLOG_MSG("eos: FRONT door sch_grp table addr = " << i << " popcnt = " << popcnt << " drb_cnt = " << drb_cnt << endl);          
//    }

   for (int i=0; i<sch_grp_depth; i++) {
     txs_csr.dhs_sch_grp_sram.entry[i].read();
     cpp_int drb_cnt = txs_csr.dhs_sch_grp_sram.entry[i].drb_cnt();
     cpp_int lif = txs_csr.dhs_sch_grp_sram.entry[i].lif();
     cpp_int hbm_ln_cnt[16];
     cpp_int qid_fifo_vld[16];
     cpp_int qid_fifo_jnk[16];
     cpp_int qid_fifo_qid[16];         
     cpp_int popcnt;         
     hbm_ln_cnt[0] = txs_csr.dhs_sch_grp_sram.entry[i].hbm_ln_cnt0();
     qid_fifo_vld[0] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm0_vld();
     qid_fifo_jnk[0] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm0_jnk();
     qid_fifo_qid[0] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm0_qid();
     hbm_ln_cnt[1] = txs_csr.dhs_sch_grp_sram.entry[i].hbm_ln_cnt1();
     qid_fifo_vld[1] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm1_vld();
     qid_fifo_jnk[1] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm1_jnk();
     qid_fifo_qid[1] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm1_qid();
     hbm_ln_cnt[2] = txs_csr.dhs_sch_grp_sram.entry[i].hbm_ln_cnt2();
     qid_fifo_vld[2] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm2_vld();
     qid_fifo_jnk[2] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm2_jnk();
     qid_fifo_qid[2] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm2_qid();
     hbm_ln_cnt[3] = txs_csr.dhs_sch_grp_sram.entry[i].hbm_ln_cnt3();
     qid_fifo_vld[3] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm3_vld();
     qid_fifo_jnk[3] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm3_jnk();
     qid_fifo_qid[3] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm3_qid();
     hbm_ln_cnt[4] = txs_csr.dhs_sch_grp_sram.entry[i].hbm_ln_cnt4();
     qid_fifo_vld[4] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm4_vld();
     qid_fifo_jnk[4] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm4_jnk();
     qid_fifo_qid[4] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm4_qid();
     hbm_ln_cnt[5] = txs_csr.dhs_sch_grp_sram.entry[i].hbm_ln_cnt5();
     qid_fifo_vld[5] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm5_vld();
     qid_fifo_jnk[5] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm5_jnk();
     qid_fifo_qid[5] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm5_qid();
     hbm_ln_cnt[6] = txs_csr.dhs_sch_grp_sram.entry[i].hbm_ln_cnt6();
     qid_fifo_vld[6] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm6_vld();
     qid_fifo_jnk[6] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm6_jnk();
     qid_fifo_qid[6] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm6_qid();
     hbm_ln_cnt[7] = txs_csr.dhs_sch_grp_sram.entry[i].hbm_ln_cnt7();
     qid_fifo_vld[7] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm7_vld();
     qid_fifo_jnk[7] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm7_jnk();
     qid_fifo_qid[7] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm7_qid();
     hbm_ln_cnt[8] = txs_csr.dhs_sch_grp_sram.entry[i].hbm_ln_cnt8();
     qid_fifo_vld[8] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm8_vld();
     qid_fifo_jnk[8] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm8_jnk();
     qid_fifo_qid[8] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm8_qid();
     hbm_ln_cnt[9] = txs_csr.dhs_sch_grp_sram.entry[i].hbm_ln_cnt9();
     qid_fifo_vld[9] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm9_vld();
     qid_fifo_jnk[9] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm9_jnk();
     qid_fifo_qid[9] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm9_qid();
     hbm_ln_cnt[10] = txs_csr.dhs_sch_grp_sram.entry[i].hbm_ln_cnt10();
     qid_fifo_vld[10] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm10_vld();
     qid_fifo_jnk[10] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm10_jnk();
     qid_fifo_qid[10] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm10_qid();
     hbm_ln_cnt[11] = txs_csr.dhs_sch_grp_sram.entry[i].hbm_ln_cnt11();
     qid_fifo_vld[11] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm11_vld();
     qid_fifo_jnk[11] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm11_jnk();
     qid_fifo_qid[11] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm11_qid();
     hbm_ln_cnt[12] = txs_csr.dhs_sch_grp_sram.entry[i].hbm_ln_cnt12();
     qid_fifo_vld[12] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm12_vld();
     qid_fifo_jnk[12] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm12_jnk();
     qid_fifo_qid[12] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm12_qid();
     hbm_ln_cnt[13] = txs_csr.dhs_sch_grp_sram.entry[i].hbm_ln_cnt13();
     qid_fifo_vld[13] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm13_vld();
     qid_fifo_jnk[13] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm13_jnk();
     qid_fifo_qid[13] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm13_qid();
     hbm_ln_cnt[14] = txs_csr.dhs_sch_grp_sram.entry[i].hbm_ln_cnt14();
     qid_fifo_vld[14] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm14_vld();
     qid_fifo_jnk[14] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm14_jnk();
     qid_fifo_qid[14] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm14_qid();
     hbm_ln_cnt[15] = txs_csr.dhs_sch_grp_sram.entry[i].hbm_ln_cnt15();
     qid_fifo_vld[15] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm15_vld();
     qid_fifo_jnk[15] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm15_jnk();
     qid_fifo_qid[15] = txs_csr.dhs_sch_grp_sram.entry[i].qid_fifo_elm15_qid();
     ////FIXME popcnt = txs_csr.dhs_sch_grp_sram.entry[i].popcnt();

     if (drb_cnt != 0) {
        PLOG_MSG("ERROR: eos: sch_grp table addr = " << i << " lif = " << lif << " drb_cnt = " << drb_cnt << " != 0" << endl);     
        eos_pass = 0;
     } else {
//        PLOG_MSG("eos: sch_grp table addr = " << i << " lif = " << lif << " drb_cnt = " << drb_cnt << endl);     
     }
     ////FIXME if (popcnt != 0) {
        ////FIXME PLOG_MSG("eos: sch_grp table addr = " << i << " lif = " << lif << " popcnt = " << popcnt << endl);     
     ////FIXME } 
     for (int j=0; j<16; j++) {
        if (hbm_ln_cnt[j] != 0) {
           PLOG_MSG("ERROR: eos: sch_grp table addr = " << i << " lif = " << lif << " hbm_ln_cnt" << j << " = " << hbm_ln_cnt[j] << " != 0" << endl);     
           eos_pass = 0;
        } else {
//           PLOG_MSG("eos: sch_grp table addr = " << i << " lif = " << lif << " hbm_ln_cnt" << j << " = " << hbm_ln_cnt[j] << endl);     
        }
        if (qid_fifo_vld[j] == 1 && qid_fifo_jnk[j] == 0) {
          PLOG_MSG("ERROR: eos: sch_grp table addr = " << i << " lif = " << lif << " qid = " << qid_fifo_qid[j] << " qid_fifo_vld " << j << " = " << qid_fifo_vld[j] << " != 0" << endl);     
           PLOG_MSG("ERROR: eos: sch_grp table addr = " << i << " lif = " << lif << " qid = " << qid_fifo_qid[j] << " qid_fifo_jnk " << j << " = " << qid_fifo_jnk[j] << " != 1" << endl);     
           eos_pass = 0;
        } else {
//           PLOG_MSG("eos: sch_grp table addr = " << i << " lif = " << lif << " qid = " << qid_fifo_qid[j] << " qid_fifo_vld " << j << " = " << qid_fifo_vld[j] << endl);     
//           PLOG_MSG("eos: sch_grp table addr = " << i << " lif = " << lif << " qid = " << qid_fifo_qid[j] << " qid_fifo_jnk " << j << " = " << qid_fifo_jnk[j] << endl);     
        }
     }
//     PLOG_MSG("eos: sch_grp table addr = " << i << "  = " << lif << " drb_cnt = " << drb_cnt << endl);     
   }
   if (eos_pass) PLOG_MSG("eos: PASS sch_grp_sram chk over sch_grp_depth:0x" << hex << sch_grp_depth << dec << endl);     

   int           tmr_eos_pass = 1;
   if ((ftmr_enable_d == 1 || stmr_enable_d == 1) && (dol_enable_eos_timers_chk == 1)) {
     cpp_int fast_cbcnt;
     cpp_int fast_bcnt;
     cpp_int fast_lcnt;
     cpp_int slow_cbcnt;
     cpp_int slow_bcnt;
     cpp_int slow_lcnt;
     for (int i=0; i<tmr_wheel_depth; i++) {
       txs_csr.dhs_tmr_cnt_sram.entry[i].read();
      if (ftmr_enable_d == 1) {
         fast_cbcnt = txs_csr.dhs_tmr_cnt_sram.entry[i].fast_cbcnt();
         fast_bcnt  = txs_csr.dhs_tmr_cnt_sram.entry[i].fast_bcnt();
         fast_lcnt  = txs_csr.dhs_tmr_cnt_sram.entry[i].fast_lcnt();
         if (fast_cbcnt != 0) {
           PLOG_MSG("ERROR: eos: tmr_cnt table addr = " << i << "  fast_cbcnt:0x != 0" << hex << fast_cbcnt << dec << endl);            
           tmr_eos_pass = 0;
         } else {
    //       PLOG_MSG("OK: eos: tmr_cnt table addr = " << i << "  fast_cbcnt:0x = 0" << hex << fast_cbcnt << dec << endl);            
         }
         if (fast_bcnt != 0) {
           PLOG_MSG("ERROR: eos: tmr_cnt table addr = " << i << "  fast_bcnt:0x != 0" << hex << fast_bcnt << dec << endl);            
           tmr_eos_pass = 0;
         } else {
    //       PLOG_MSG("OK: eos: tmr_cnt table addr = " << i << "  fast_bcnt:0x = 0" << hex << fast_bcnt << dec << endl);            
         }
         if (fast_lcnt != 0) {
           PLOG_MSG("ERROR: eos: tmr_cnt table addr = " << i << "  fast_lcnt:0x != 0" << hex << fast_lcnt << dec << endl);            
           tmr_eos_pass = 0;
         } else {
    //       PLOG_MSG("OK: eos: tmr_cnt table addr = " << i << "  fast_lcnt:0x = 0" << hex << fast_lcnt << dec << endl);            
         }
       }
      if (stmr_enable_d == 1) {
         slow_cbcnt = txs_csr.dhs_tmr_cnt_sram.entry[i].slow_cbcnt();
         slow_bcnt  = txs_csr.dhs_tmr_cnt_sram.entry[i].slow_bcnt();
         slow_lcnt  = txs_csr.dhs_tmr_cnt_sram.entry[i].slow_lcnt();
         if (slow_cbcnt != 0) {
           PLOG_MSG("ERROR: eos: tmr_cnt table addr = " << i << "  slow_cbcnt:0x != 0" << hex << slow_cbcnt << dec << endl);            
           tmr_eos_pass = 0;
         } else {
    //       PLOG_MSG("OK: eos: tmr_cnt table addr = " << i << "  slow_cbcnt:0x = 0" << hex << slow_cbcnt << dec << endl);            
         }
         if (slow_bcnt != 0) {
           PLOG_MSG("ERROR: eos: tmr_cnt table addr = " << i << "  slow_bcnt:0x != 0" << hex << slow_bcnt << dec << endl);            
           tmr_eos_pass = 0;
         } else {
    //       PLOG_MSG("OK: eos: tmr_cnt table addr = " << i << "  slow_bcnt:0x = 0" << hex << slow_bcnt << dec << endl);            
         }
         if (slow_lcnt != 0) {
           PLOG_MSG("ERROR: eos: tmr_cnt table addr = " << i << "  slow_lcnt:0x != 0" << hex << slow_lcnt << dec << endl);            
           tmr_eos_pass = 0;
         } else {
    //       PLOG_MSG("OK: eos: tmr_cnt table addr = " << i << "  slow_lcnt:0x = 0" << hex << slow_lcnt << dec << endl);            
         }
       }
     }
   }
   if (tmr_eos_pass) PLOG_MSG("eos: PASS tmr_cntr chk over tmr_wheel_depth:0x" << hex << tmr_wheel_depth << dec << endl);     

   hbm_init.assign(SKNOBS_GET("cap0/txs/cfw_scheduler_static/hbm_hw_init", 1));
   hbm_base = txs_csr.cfw_scheduler_static.hbm_base();
   sch_grp_depth = txs_csr.cfw_scheduler_static.sch_grp_depth();
   if (hbm_init == 0) {
      eos_pass = 1;
      unsigned char *data;
      data = new unsigned char[SCHGRP_SIZE_BYTE];
      for (int i=0; i<sch_grp_depth; i++) {
         cap_mem_access::instancep()->read(uint64_t (hbm_base + i*SCHGRP_SIZE_BYTE), data, SCHGRP_SIZE_BYTE);
         for (int j=0; j<SCHGRP_SIZE_BYTE; j++) {
            if ((data[j]&0xff) != 0) {
               PLOG_MSG("ERROR: eos: hbm_clr_chk addr = " << (i*SCHGRP_SIZE_BYTE + j) << " data: != 0 0x" << hex << (data[j]&0xff) << dec << endl);     
               eos_pass = 0;
            } else {
               //PLOG_MSG("PASS: eos: hb_clr_chk  addr = " << (i*SCHGRP_SIZE_BYTE + j) << " data: == 0 0x" << hex << (data[j]&0xff) << dec << endl);     
            }
         }
      }
      delete [] data;
      if (eos_pass) PLOG_MSG("eos: PASS hbm_clr_chk read base_addr:0x" << hex << hbm_base << " over sch_grp_depth:0x" << sch_grp_depth << dec << endl);     
   }

   tmr_hbm_base = txs_csr.cfg_timer_static.hbm_base();
   tmr_hsh_depth = txs_csr.cfg_timer_static.tmr_hsh_depth();

   if ((ftmr_enable_d == 1 || stmr_enable_d == 1) && (dol_enable_eos_timers_chk == 1)) {
      eos_pass = 1;
      unsigned char *data;

      data = new unsigned char[64];

      for (int i=0; i<(tmr_hsh_depth+1); i++) {
         cpp_int myAdds = tmr_hbm_base + i*64 + (16*(tmr_hsh_depth+1)*64);
         cap_mem_access::instancep()->read(myAdds.convert_to<uint64_t>(), data, 64);
         //cap_mem_access::instancep()->read(uint64_t (tmr_hbm_base + i*64 + (16*(tmr_hsh_depth+1))), data, 64);
         for (int j=0; j<64; j++) {
            if ((data[j]&0xff) != 0x0) {
               PLOG_MSG("ERROR: eos: tmr hbm_clr_chk addr = 0x" << hex << myAdds.convert_to<uint64_t>() << " data: != 0 0x" << (data[j]&0xff) << dec << endl);     
               eos_pass = 0;
            } else {
              // PLOG_MSG("PASS: eos: tmr hbm_clr_chk  addr = 0x" << hex << myAdds.convert_to<uint64_t>() << " data: == 0 0x" << (data[j]&0xff) << dec << endl);     
            }
         }
      }
      delete [] data;
      if (eos_pass) PLOG_MSG("eos: PASS tmr hbm_clr_chk read base_addr:0x" << hex << hbm_base << " overtmr_hsh_depth :0x" << tmr_hsh_depth << dec << endl);     
   }


   cap_txs_eos_cnt(chip_id, inst_id);
   cap_txs_eos_int(chip_id, inst_id);
}
#endif

void cap_txs_sch_enable_wr(int chip_id, int inst_id, int enable) {

   cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, 0, 0);
   PLOG_MSG("Sch enable:0x" << hex << enable << endl);

   txs_csr.cfg_sch.enable(enable);
   txs_csr.cfg_sch.write();
}

void cap_txs_sch_doorbell_wr(int chip_id, int inst_id, int lif, int qid, int cos, int set) {
   cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, 0, 0);
   PLOG_MSG("Ringing doorbell BackDoor lif:0x" <<hex<<lif<< ", qid:0x" <<qid<<dec<< ", cos:" << cos<<", set:" << set << endl);
   txs_csr.dhs_doorbell.entry[lif].cos(cos);
   txs_csr.dhs_doorbell.entry[lif].set(set);
   txs_csr.dhs_doorbell.entry[lif].qid(qid);
   txs_csr.dhs_doorbell.entry[lif].write();
}

int cap_txs_sch_doorbell_rd(int chip_id, int inst_id, int lif, int qid, int cos) {
   cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, 0, 0);
   txs_csr.dhs_doorbell.entry[lif].cos(cos);
   txs_csr.dhs_doorbell.entry[lif].set(0);
   txs_csr.dhs_doorbell.entry[lif].qid(qid);
   txs_csr.dhs_doorbell.entry[lif].read();
   
   cpp_int rtnVal = txs_csr.dhs_doorbell.entry[lif].set();

   PLOG_MSG("Checking doorbell BackDoor lif:0x" <<hex<<lif<< ", qid:0x"  <<dec<< ", cos:" << cos << ", set:" << rtnVal << endl);
   return 0;
   
}

void cap_txs_eos_cnt(int chip_id, int inst_id) {
   cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);
   string         csr_name;

   //if (sknobs_exists((char *) "no_cnt_chk") == 0) 
   if (sknobs_exists((char *) "txs_cnt_chk") == 1) {
       for(auto itr: txs_csr.get_children_prefix("cnt_")) {
           csr_name = itr->get_name();
           //if ((csr_name.compare("cnt_sch_fifo_empty") != 0) &&
               //(csr_name.compare(0, 11, "cnt_sch_axi") != 0) &&
               //(csr_name.compare(0, 11, "cnt_tmr_axi") != 0) )  {
              //itr->read_compare();
           //}
           if ((csr_name.find("cnt_sch_fifo_empty") == string::npos) ||
               (csr_name.find("cnt_sch_axi")        == string::npos) ||
               (csr_name.find("cnt_tmr_axi")        == string::npos) )  {
              // skip these cnt chk
           } else {
              itr->read_compare();
           }
       }
   }
   txs_cnt_show(chip_id, inst_id);
   tmr_cnt_show(chip_id, inst_id);
}

void cap_txs_eos_int(int chip_id, int inst_id) {
   cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);
   string         csr_name;

   ///if (sknobs_exists((char *) "no_int_chk") == 0) 
   if (sknobs_exists((char *) "txs_int_chk") == 1) {

       if (sknobs_exists((char *) "txs_int_resp_err_exp") == 1) {
          PLOG_MSG("int_sch/int_tmr mask sch_rresp_err sch_bresp_err interrupt" << endl);
          txs_csr.int_sch.intreg.sch_rresp_err_interrupt(1);
          txs_csr.int_sch.intreg.sch_bresp_err_interrupt(1);
          txs_csr.int_tmr.intreg.tmr_rresp_err_interrupt(1);
          txs_csr.int_tmr.intreg.tmr_bresp_err_interrupt(1);
       }
       // mask all efull int bits
       PLOG_MSG("int_sch mask efull interrupt" << endl);
       txs_csr.int_sch.intreg.sch_txdma_msg_efull_interrupt(1);
       txs_csr.int_sch.intreg.sch_hbm_re_efull_interrupt(1);
       txs_csr.int_sch.intreg.sch_hbm_we_efull_interrupt(1);
       txs_csr.int_sch.intreg.sch_hbm_byp_pending_efull_interrupt(1);
       txs_csr.int_sch.intreg.sch_hbm_rd_pending_efull_interrupt(1);
       txs_csr.int_sch.intreg.sch_hbm_wr_pending_efull_interrupt(1);
       txs_csr.int_sch.write();

       PLOG_MSG("int_tmr mask efull interrupt" << endl);
       txs_csr.int_tmr.intreg.ftmr_fifo_efull_interrupt(1);
       txs_csr.int_tmr.intreg.stmr_fifo_efull_interrupt(1);
       txs_csr.int_tmr.intreg.tmr_rejct_drb_ovf_interrupt(1);
       txs_csr.int_tmr.intreg.tmr_rejct_drb_efull_interrupt(1);
       txs_csr.int_tmr.intreg.tmr_hbm_re_efull_interrupt(1);
       txs_csr.int_tmr.intreg.tmr_hbm_we_efull_interrupt(1);
       txs_csr.int_tmr.intreg.tmr_hbm_byp_pending_efull_interrupt(1);
       txs_csr.int_tmr.intreg.tmr_hbm_rd_pending_efull_interrupt(1);
       txs_csr.int_tmr.intreg.tmr_hbm_wr_pending_efull_interrupt(1);
       txs_csr.int_tmr.write();

       for(auto itr: txs_csr.get_children_prefix("int_")) {
           csr_name = itr->get_name();
           itr->read();
           uint32_t intr_rd = itr->all().convert_to<uint32_t>();
           if (intr_rd != 0) {
              PLOG_MSG("ERROR csr_name:" << csr_name << " intr != 0" << hex << intr_rd << dec << endl);
           } else {
              PLOG_MSG("csr_name:" << csr_name << " pass " << hex << intr_rd << dec << endl);
           }
       }
   }
}

void cap_txs_eos_sta(int chip_id, int inst_id) {
}

void cap_txs_csr_set_hdl_path(int chip_id, int inst_id, string path) {
  cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);

  // id : 0 is given to csr instance.
  // id : 1 is given to sub_csr modules
  // id : 2 is given to memories

  for(int i = 0; i < txs_csr.dhs_sch_grp_entry.get_depth_entry(); i++) {
    txs_csr.dhs_sch_grp_entry.entry[i].set_access_no_zero_time(1);
  }

  txs_csr.set_csr_inst_path(0, (path + ".cap_txs_csr_inst0"));

  //PLOG_MSG("DEBUG path:" << path << " mem:" << path + ".cap_txs_tmr_cnt_mem_inst0.mem.mem" << endl);
  //PLOG_MSG("DEBUG path:" << path << " mem:" << path + ".cap_txs_sch_grp_mem_inst0.mem.mem" << endl);
  txs_csr.dhs_tmr_cnt_sram.set_csr_inst_path(2, path + ".cap_txs_tmr_cnt_mem_inst0.mem.mem");
  txs_csr.dhs_sch_grp_sram.set_csr_inst_path(2, path + ".cap_txs_sch_grp_mem_inst0.mem.mem");
  txs_csr.dhs_sch_rlid_map_sram.set_csr_inst_path(2, path + ".cap_txs_sch_rlid_map_inst0.mem.mem");
  txs_csr.dhs_sch_lif_map_sram.set_csr_inst_path(2, path + ".cap_txs_sch_lif_map_inst0.mem.mem");
  txs_csr.dhs_dtdmlo_calendar.set_csr_inst_path(2, path + ".cap_txs_scheduler_inst0.cap_txs_sch_inst0.cap_txs_dtdmlo_inst0.calendar");
  txs_csr.dhs_dtdmhi_calendar.set_csr_inst_path(2, path + ".cap_txs_scheduler_inst0.cap_txs_sch_inst0.cap_txs_dtdmhi_inst0.calendar");

  //txs_csr.dhs_dtdmhi_calendar.set_csr_inst_path(2, path + ".cap_txs_scheduler_inst0.cap_txs_hbm.axi_rdbuff_inst0.mem.mem");
  //txs_csr.dhs_dtdmhi_calendar.set_csr_inst_path(2, path + ".cap_txs_scheduler_inst0.cap_txs_hbm_byp.hbm_byp_inst0.mem.mem");
  //txs_csr.dhs_dtdmhi_calendar.set_csr_inst_path(2, path + ".cap_txs_timers_inst0.cap_txs_hbm.axi_rdbuff_inst0.mem.mem");
  //txs_csr.dhs_dtdmhi_calendar.set_csr_inst_path(2, path + ".cap_txs_timers_inst0.cap_txs_hbm_byp.hbm_byp_inst0.mem.mem");

}


void txs_tmr_start_no_stop(int chip_id, int inst_id, int ftmr, int dtime, int typ, int ring, int qid, int entry) {
   cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);

   if (ftmr) {
      txs_csr.dhs_fast_timer_start_no_stop.entry[entry].typ(typ);
      txs_csr.dhs_fast_timer_start_no_stop.entry[entry].qid(qid);
      txs_csr.dhs_fast_timer_start_no_stop.entry[entry].ring(ring);
      txs_csr.dhs_fast_timer_start_no_stop.entry[entry].dtime(dtime);
      txs_csr.dhs_fast_timer_start_no_stop.entry[entry].show();
      txs_csr.dhs_fast_timer_start_no_stop.entry[entry].write();
   } else {
      txs_csr.dhs_slow_timer_start_no_stop.entry[entry].typ(typ);
      txs_csr.dhs_slow_timer_start_no_stop.entry[entry].qid(qid);
      txs_csr.dhs_slow_timer_start_no_stop.entry[entry].ring(ring);
      txs_csr.dhs_slow_timer_start_no_stop.entry[entry].dtime(dtime);
      txs_csr.dhs_slow_timer_start_no_stop.entry[entry].show();
      txs_csr.dhs_slow_timer_start_no_stop.entry[entry].write();
   }
}

void txs_tmr_force_ctime(int chip_id, int inst_id, int ftmr, int enable, int ctime) {
   cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);

   if (ftmr) {
      txs_csr.cfg_force_fast_timer.enable(enable);
      txs_csr.cfg_force_fast_timer.ctime(ctime);
      txs_csr.cfg_force_fast_timer.show();
      txs_csr.cfg_force_fast_timer.write();
   } else {
      txs_csr.cfg_force_slow_timer.enable(enable);
      txs_csr.cfg_force_slow_timer.ctime(ctime);
      txs_csr.cfg_force_slow_timer.show();
      txs_csr.cfg_force_slow_timer.write();
   }
}


/*
void tmr_cnt_check(int chip_id, int inst_id) {
   cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);
   txs_csr.cnt_sch_axi_rd_req.read();
   txs_csr.cnt_sch_axi_rd_rsp.read();
   txs_csr.cnt_sch_axi_rd_rsp_err.read();
   txs_csr.cnt_sch_axi_rd_rsp_uexp.read();

   txs_csr.cnt_sch_axi_wr_req.read();
   txs_csr.cnt_sch_axi_bid.read();
   txs_csr.cnt_sch_axi_bid_err.read();
   txs_csr.cnt_sch_axi_bid_rerr.read();
   txs_csr.cnt_sch_axi_bid_uexp.read();
}
**/


void tmr_cnt_show(int chip_id, int inst_id) {
   cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);

   txs_csr.cnt_ftmr_push.read();
   //txs_csr.cnt_ftmr_push.show();

   txs_csr.cnt_ftmr_pop.read();
   //txs_csr.cnt_ftmr_pop.show();

   txs_csr.cnt_stmr_push.read();
   //txs_csr.cnt_stmr_push.show();

   txs_csr.cnt_stmr_pop.read();
   //txs_csr.cnt_stmr_pop.show();

   txs_csr.cnt_ftmr_key_not_push.read();
   //txs_csr.cnt_ftmr_key_not_push.show();

   txs_csr.cnt_ftmr_key_not_found.read();
   //txs_csr.cnt_ftmr_key_not_found.show();

   txs_csr.cnt_stmr_key_not_push.read();
   //txs_csr.cnt_stmr_key_not_push.show();

   txs_csr.cnt_stmr_key_not_found.read();
   //txs_csr.cnt_stmr_key_not_found.show();
}

void txs_cnt_show(int chip_id, int inst_id) {

   cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);

   txs_csr.cnt_sch_doorbell_set.read();
   //txs_csr.cnt_sch_doorbell_set.show();

   txs_csr.cnt_sch_doorbell_clr.read();
   //txs_csr.cnt_sch_doorbell_clr.show();

   txs_csr.cnt_sch_fifo_empty.read();
   //txs_csr.cnt_sch_fifo_empty.show();

   txs_csr.cnt_sch_txdma_cos0.read();
   //txs_csr.cnt_sch_txdma_cos0.show();

   txs_csr.cnt_sch_txdma_cos1.read();
   //txs_csr.cnt_sch_txdma_cos1.show();

   txs_csr.cnt_sch_txdma_cos2.read();
   //txs_csr.cnt_sch_txdma_cos2.show();

   txs_csr.cnt_sch_txdma_cos3.read();
   //txs_csr.cnt_sch_txdma_cos3.show();

   txs_csr.cnt_sch_txdma_cos4.read();
   //txs_csr.cnt_sch_txdma_cos4.show();

   txs_csr.cnt_sch_txdma_cos5.read();
   //txs_csr.cnt_sch_txdma_cos5.show();

   txs_csr.cnt_sch_txdma_cos6.read();
   //txs_csr.cnt_sch_txdma_cos6.show();

   txs_csr.cnt_sch_txdma_cos7.read();
   //txs_csr.cnt_sch_txdma_cos7.show();

   txs_csr.cnt_sch_txdma_cos8.read();
   //txs_csr.cnt_sch_txdma_cos8.show();

   txs_csr.cnt_sch_txdma_cos9.read();
   //txs_csr.cnt_sch_txdma_cos9.show();

   txs_csr.cnt_sch_txdma_cos10.read();
   //txs_csr.cnt_sch_txdma_cos10.show();

   txs_csr.cnt_sch_txdma_cos11.read();
   //txs_csr.cnt_sch_txdma_cos11.show();

   txs_csr.cnt_sch_txdma_cos12.read();
   //txs_csr.cnt_sch_txdma_cos12.show();

   txs_csr.cnt_sch_txdma_cos13.read();
   //txs_csr.cnt_sch_txdma_cos13.show();

   txs_csr.cnt_sch_txdma_cos14.read();
   //txs_csr.cnt_sch_txdma_cos14.show();

   txs_csr.cnt_sch_txdma_cos15.read();
   //txs_csr.cnt_sch_txdma_cos15.show();
}

void cap_txs_bist_test(int chip_id, int inst_id) {
   cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);
   int   loop_cnt;

   txs_csr.cfg_tmr_hbm_sram.bist_run(1);
   txs_csr.cfg_tmr_hbm_sram.write();

   txs_csr.cfg_tmr_hbm_byp_sram.bist_run(1);
   txs_csr.cfg_tmr_hbm_byp_sram.write();

   txs_csr.cfg_tmr_fifo_sram.bist_run(1);
   txs_csr.cfg_tmr_fifo_sram.write();

   txs_csr.cfg_tmr_cnt_sram.bist_run(1);
   txs_csr.cfg_tmr_cnt_sram.write();

   txs_csr.cfg_sch_lif_map_sram.bist_run(1);
   txs_csr.cfg_sch_lif_map_sram.write();

   txs_csr.cfg_sch_rlid_map_sram.bist_run(1);
   txs_csr.cfg_sch_rlid_map_sram.write();

   txs_csr.cfg_sch_grp_sram.bist_run(1);
   txs_csr.cfg_sch_grp_sram.write();

   txs_csr.cfg_sch_hbm_sram.bist_run(1);
   txs_csr.cfg_sch_hbm_sram.write();

   txs_csr.cfg_sch_hbm_byp_sram.bist_run(1);
   txs_csr.cfg_sch_hbm_byp_sram.write();

    loop_cnt = 0;
    while(loop_cnt < 10000) {
        txs_csr.sta_srams_tmr_hbm.read();
        if ((txs_csr.sta_srams_tmr_hbm.bist_done_fail() == 1)||(txs_csr.sta_srams_tmr_hbm.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, TXS sta_srams_tmr_hbm bist can not be done!" << endl)
    }
    else if (txs_csr.sta_srams_tmr_hbm.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, TXS sta_srams_tmr_hbm bist fail!" << endl)
    }
    else {
        PLOG_MSG("TXS sta_srams_tmr_hbm bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        txs_csr.sta_srams_tmr_hbm_byp.read();
        if ((txs_csr.sta_srams_tmr_hbm_byp.bist_done_fail() == 1)||(txs_csr.sta_srams_tmr_hbm_byp.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, TXS sta_srams_tmr_hbm_byp bist can not be done!" << endl)
    }
    else if (txs_csr.sta_srams_tmr_hbm_byp.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, TXS sta_srams_tmr_hbm_byp bist fail!" << endl)
    }
    else {
        PLOG_MSG("TXS sta_srams_tmr_hbm_byp bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        txs_csr.sta_srams_tmr_fifo.read();
        if ((txs_csr.sta_srams_tmr_fifo.bist_done_fail() == 1)||(txs_csr.sta_srams_tmr_fifo.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, TXS sta_srams_tmr_fifo bist can not be done!" << endl)
    }
    else if (txs_csr.sta_srams_tmr_fifo.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, TXS sta_srams_tmr_fifo bist fail!" << endl)
    }
    else {
        PLOG_MSG("TXS sta_srams_tmr_fifo bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 100000) {
        txs_csr.sta_srams_ecc_tmr_cnt.read();
        if ((txs_csr.sta_srams_ecc_tmr_cnt.bist_done_fail() == 1)||(txs_csr.sta_srams_ecc_tmr_cnt.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 100000) {
        PLOG_ERR("FAIL, TXS sta_srams_ecc_tmr_cnt bist can not be done!" << endl)
    }
    else if (txs_csr.sta_srams_ecc_tmr_cnt.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, TXS sta_srams_ecc_tmr_cnt bist fail!" << endl)
    }
    else {
        PLOG_MSG("TXS sta_srams_ecc_tmr_cnt bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        txs_csr.sta_srams_ecc_sch_lif_map.read();
        if ((txs_csr.sta_srams_ecc_sch_lif_map.bist_done_fail() == 1)||(txs_csr.sta_srams_ecc_sch_lif_map.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, TXS sta_srams_ecc_sch_lif_map bist can not be done!" << endl)
    }
    else if (txs_csr.sta_srams_ecc_sch_lif_map.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, TXS sta_srams_ecc_sch_lif_map bist fail!" << endl)
    }
    else {
        PLOG_MSG("TXS sta_srams_ecc_sch_lif_map bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        txs_csr.sta_srams_ecc_sch_rlid_map.read();
        if ((txs_csr.sta_srams_ecc_sch_rlid_map.bist_done_fail() == 1)||(txs_csr.sta_srams_ecc_sch_rlid_map.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, TXS sta_srams_ecc_sch_rlid_map bist can not be done!" << endl)
    }
    else if (txs_csr.sta_srams_ecc_sch_rlid_map.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, TXS sta_srams_ecc_sch_rlid_map bist fail!" << endl)
    }
    else {
        PLOG_MSG("TXS sta_srams_ecc_sch_rlid_map bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        txs_csr.sta_srams_ecc_sch_grp.read();
        if ((txs_csr.sta_srams_ecc_sch_grp.bist_done_fail() == 1)||(txs_csr.sta_srams_ecc_sch_grp.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, TXS sta_srams_ecc_sch_grp bist can not be done!" << endl)
    }
    else if (txs_csr.sta_srams_ecc_sch_grp.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, TXS sta_srams_ecc_sch_grp bist fail!" << endl)
    }
    else {
        PLOG_MSG("TXS sta_srams_ecc_sch_grp bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        txs_csr.sta_srams_sch_hbm.read();
        if ((txs_csr.sta_srams_sch_hbm.bist_done_fail() == 1)||(txs_csr.sta_srams_sch_hbm.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, TXS sta_srams_sch_hbm bist can not be done!" << endl)
    }
    else if (txs_csr.sta_srams_sch_hbm.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, TXS sta_srams_sch_hbm bist fail!" << endl)
    }
    else {
        PLOG_MSG("TXS sta_srams_sch_hbm bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        txs_csr.sta_srams_sch_hbm_byp.read();
        if ((txs_csr.sta_srams_sch_hbm_byp.bist_done_fail() == 1)||(txs_csr.sta_srams_sch_hbm_byp.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, TXS sta_srams_sch_hbm_byp bist can not be done!" << endl)
    }
    else if (txs_csr.sta_srams_sch_hbm_byp.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, TXS sta_srams_sch_hbm bist fail!" << endl)
    }
    else {
        PLOG_MSG("TXS sta_srams_sch_hbm_byp bist pass!" << endl)
    }

}

void cap_txs_bist_start(int chip_id, int inst_id) {
   cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);

   txs_csr.cfg_tmr_hbm_sram.bist_run(1);
   txs_csr.cfg_tmr_hbm_sram.write();

   txs_csr.cfg_tmr_hbm_byp_sram.bist_run(1);
   txs_csr.cfg_tmr_hbm_byp_sram.write();

   txs_csr.cfg_tmr_fifo_sram.bist_run(1);
   txs_csr.cfg_tmr_fifo_sram.write();

   txs_csr.cfg_tmr_cnt_sram.bist_run(1);
   txs_csr.cfg_tmr_cnt_sram.write();

   txs_csr.cfg_sch_lif_map_sram.bist_run(1);
   txs_csr.cfg_sch_lif_map_sram.write();

   txs_csr.cfg_sch_rlid_map_sram.bist_run(1);
   txs_csr.cfg_sch_rlid_map_sram.write();

   txs_csr.cfg_sch_grp_sram.bist_run(1);
   txs_csr.cfg_sch_grp_sram.write();

   txs_csr.cfg_sch_hbm_sram.bist_run(1);
   txs_csr.cfg_sch_hbm_sram.write();

   txs_csr.cfg_sch_hbm_byp_sram.bist_run(1);
   txs_csr.cfg_sch_hbm_byp_sram.write();
}

void cap_txs_bist_check(int chip_id, int inst_id) {
   cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);
   int   loop_cnt;

    loop_cnt = 0;
    while(loop_cnt < 10000) {
        txs_csr.sta_srams_tmr_hbm.read();
        if ((txs_csr.sta_srams_tmr_hbm.bist_done_fail() == 1)||(txs_csr.sta_srams_tmr_hbm.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, TXS sta_srams_tmr_hbm bist can not be done!" << endl)
    }
    else if (txs_csr.sta_srams_tmr_hbm.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, TXS sta_srams_tmr_hbm bist fail!" << endl)
    }
    else {
        PLOG_MSG("TXS sta_srams_tmr_hbm bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        txs_csr.sta_srams_tmr_hbm_byp.read();
        if ((txs_csr.sta_srams_tmr_hbm_byp.bist_done_fail() == 1)||(txs_csr.sta_srams_tmr_hbm_byp.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, TXS sta_srams_tmr_hbm_byp bist can not be done!" << endl)
    }
    else if (txs_csr.sta_srams_tmr_hbm_byp.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, TXS sta_srams_tmr_hbm_byp bist fail!" << endl)
    }
    else {
        PLOG_MSG("TXS sta_srams_tmr_hbm_byp bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        txs_csr.sta_srams_tmr_fifo.read();
        if ((txs_csr.sta_srams_tmr_fifo.bist_done_fail() == 1)||(txs_csr.sta_srams_tmr_fifo.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, TXS sta_srams_tmr_fifo bist can not be done!" << endl)
    }
    else if (txs_csr.sta_srams_tmr_fifo.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, TXS sta_srams_tmr_fifo bist fail!" << endl)
    }
    else {
        PLOG_MSG("TXS sta_srams_tmr_fifo bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 100000) {
        txs_csr.sta_srams_ecc_tmr_cnt.read();
        if ((txs_csr.sta_srams_ecc_tmr_cnt.bist_done_fail() == 1)||(txs_csr.sta_srams_ecc_tmr_cnt.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 100000) {
        PLOG_ERR("FAIL, TXS sta_srams_ecc_tmr_cnt bist can not be done!" << endl)
    }
    else if (txs_csr.sta_srams_ecc_tmr_cnt.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, TXS sta_srams_ecc_tmr_cnt bist fail!" << endl)
    }
    else {
        PLOG_MSG("TXS sta_srams_ecc_tmr_cnt bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        txs_csr.sta_srams_ecc_sch_lif_map.read();
        if ((txs_csr.sta_srams_ecc_sch_lif_map.bist_done_fail() == 1)||(txs_csr.sta_srams_ecc_sch_lif_map.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, TXS sta_srams_ecc_sch_lif_map bist can not be done!" << endl)
    }
    else if (txs_csr.sta_srams_ecc_sch_lif_map.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, TXS sta_srams_ecc_sch_lif_map bist fail!" << endl)
    }
    else {
        PLOG_MSG("TXS sta_srams_ecc_sch_lif_map bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        txs_csr.sta_srams_ecc_sch_rlid_map.read();
        if ((txs_csr.sta_srams_ecc_sch_rlid_map.bist_done_fail() == 1)||(txs_csr.sta_srams_ecc_sch_rlid_map.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, TXS sta_srams_ecc_sch_rlid_map bist can not be done!" << endl)
    }
    else if (txs_csr.sta_srams_ecc_sch_rlid_map.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, TXS sta_srams_ecc_sch_rlid_map bist fail!" << endl)
    }
    else {
        PLOG_MSG("TXS sta_srams_ecc_sch_rlid_map bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        txs_csr.sta_srams_ecc_sch_grp.read();
        if ((txs_csr.sta_srams_ecc_sch_grp.bist_done_fail() == 1)||(txs_csr.sta_srams_ecc_sch_grp.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, TXS sta_srams_ecc_sch_grp bist can not be done!" << endl)
    }
    else if (txs_csr.sta_srams_ecc_sch_grp.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, TXS sta_srams_ecc_sch_grp bist fail!" << endl)
    }
    else {
        PLOG_MSG("TXS sta_srams_ecc_sch_grp bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        txs_csr.sta_srams_sch_hbm.read();
        if ((txs_csr.sta_srams_sch_hbm.bist_done_fail() == 1)||(txs_csr.sta_srams_sch_hbm.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, TXS sta_srams_sch_hbm bist can not be done!" << endl)
    }
    else if (txs_csr.sta_srams_sch_hbm.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, TXS sta_srams_sch_hbm bist fail!" << endl)
    }
    else {
        PLOG_MSG("TXS sta_srams_sch_hbm bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        txs_csr.sta_srams_sch_hbm_byp.read();
        if ((txs_csr.sta_srams_sch_hbm_byp.bist_done_fail() == 1)||(txs_csr.sta_srams_sch_hbm_byp.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, TXS sta_srams_sch_hbm_byp bist can not be done!" << endl)
    }
    else if (txs_csr.sta_srams_sch_hbm_byp.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, TXS sta_srams_sch_hbm bist fail!" << endl)
    }
    else {
        PLOG_MSG("TXS sta_srams_sch_hbm_byp bist pass!" << endl)
    }

   txs_csr.cfg_tmr_hbm_sram.bist_run(0);
   txs_csr.cfg_tmr_hbm_sram.write();

   txs_csr.cfg_tmr_hbm_byp_sram.bist_run(0);
   txs_csr.cfg_tmr_hbm_byp_sram.write();

   txs_csr.cfg_tmr_fifo_sram.bist_run(0);
   txs_csr.cfg_tmr_fifo_sram.write();

   txs_csr.cfg_tmr_cnt_sram.bist_run(0);
   txs_csr.cfg_tmr_cnt_sram.write();

   txs_csr.cfg_sch_lif_map_sram.bist_run(0);
   txs_csr.cfg_sch_lif_map_sram.write();

   txs_csr.cfg_sch_rlid_map_sram.bist_run(0);
   txs_csr.cfg_sch_rlid_map_sram.write();

   txs_csr.cfg_sch_grp_sram.bist_run(0);
   txs_csr.cfg_sch_grp_sram.write();

   txs_csr.cfg_sch_hbm_sram.bist_run(0);
   txs_csr.cfg_sch_hbm_sram.write();

   txs_csr.cfg_sch_hbm_byp_sram.bist_run(0);
   txs_csr.cfg_sch_hbm_byp_sram.write();
}

#ifndef CAPRI_HAL

////////////////////////////////////////////
//  for TXS bring-up

void cap_txs_dump_tx_cos(int chip_id, int inst_id) {

    cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);

    map <string, uint64_t> cntr_cos0;
    map <string, uint64_t> cntr_cos1;
    map <string, uint64_t> cntr_cos2;
    map <string, uint64_t> cntr_cos3;
    map <string, uint64_t> cntr_cos4;
    map <string, uint64_t> cntr_cos5;
    map <string, uint64_t> cntr_cos6;
    map <string, uint64_t> cntr_cos7;
    map <string, uint64_t> cntr_cos8;
    map <string, uint64_t> cntr_cos9;
    map <string, uint64_t> cntr_cos10;
    map <string, uint64_t> cntr_cos11;
    map <string, uint64_t> cntr_cos12;
    map <string, uint64_t> cntr_cos13;
    map <string, uint64_t> cntr_cos14;
    map <string, uint64_t> cntr_cos15;
    uint64_t cos_sum = 0;
    float    cos_ratio;

    txs_csr.cnt_sch_txdma_cos0.read();
    cntr_cos0["TXDMA_COS"] = txs_csr.cnt_sch_txdma_cos0.all().convert_to<uint64_t>();
    cos_sum += txs_csr.cnt_sch_txdma_cos0.all().convert_to<uint64_t>();
    txs_csr.cnt_sch_txdma_cos1.read();
    cntr_cos1["TXDMA_COS"] = txs_csr.cnt_sch_txdma_cos1.all().convert_to<uint64_t>();
    cos_sum += txs_csr.cnt_sch_txdma_cos1.all().convert_to<uint64_t>();
    txs_csr.cnt_sch_txdma_cos2.read();
    cntr_cos2["TXDMA_COS"] = txs_csr.cnt_sch_txdma_cos2.all().convert_to<uint64_t>();
    cos_sum += txs_csr.cnt_sch_txdma_cos2.all().convert_to<uint64_t>();
    txs_csr.cnt_sch_txdma_cos3.read();
    cntr_cos3["TXDMA_COS"] = txs_csr.cnt_sch_txdma_cos3.all().convert_to<uint64_t>();
    cos_sum += txs_csr.cnt_sch_txdma_cos3.all().convert_to<uint64_t>();
    txs_csr.cnt_sch_txdma_cos4.read();
    cntr_cos4["TXDMA_COS"] = txs_csr.cnt_sch_txdma_cos4.all().convert_to<uint64_t>();
    cos_sum += txs_csr.cnt_sch_txdma_cos4.all().convert_to<uint64_t>();
    txs_csr.cnt_sch_txdma_cos5.read();
    cntr_cos5["TXDMA_COS"] = txs_csr.cnt_sch_txdma_cos5.all().convert_to<uint64_t>();
    cos_sum += txs_csr.cnt_sch_txdma_cos5.all().convert_to<uint64_t>();
    txs_csr.cnt_sch_txdma_cos6.read();
    cntr_cos6["TXDMA_COS"] = txs_csr.cnt_sch_txdma_cos6.all().convert_to<uint64_t>();
    cos_sum += txs_csr.cnt_sch_txdma_cos6.all().convert_to<uint64_t>();
    txs_csr.cnt_sch_txdma_cos7.read();
    cntr_cos7["TXDMA_COS"] = txs_csr.cnt_sch_txdma_cos7.all().convert_to<uint64_t>();
    cos_sum += txs_csr.cnt_sch_txdma_cos7.all().convert_to<uint64_t>();
    txs_csr.cnt_sch_txdma_cos8.read();
    cntr_cos8["TXDMA_COS"] = txs_csr.cnt_sch_txdma_cos8.all().convert_to<uint64_t>();
    cos_sum += txs_csr.cnt_sch_txdma_cos8.all().convert_to<uint64_t>();
    txs_csr.cnt_sch_txdma_cos9.read();
    cntr_cos9["TXDMA_COS"] = txs_csr.cnt_sch_txdma_cos9.all().convert_to<uint64_t>();
    cos_sum += txs_csr.cnt_sch_txdma_cos9.all().convert_to<uint64_t>();
    txs_csr.cnt_sch_txdma_cos10.read();
    cntr_cos10["TXDMA_COS"] = txs_csr.cnt_sch_txdma_cos10.all().convert_to<uint64_t>();
    cos_sum += txs_csr.cnt_sch_txdma_cos10.all().convert_to<uint64_t>();
    txs_csr.cnt_sch_txdma_cos11.read();
    cntr_cos11["TXDMA_COS"] = txs_csr.cnt_sch_txdma_cos11.all().convert_to<uint64_t>();
    cos_sum += txs_csr.cnt_sch_txdma_cos11.all().convert_to<uint64_t>();
    txs_csr.cnt_sch_txdma_cos12.read();
    cntr_cos12["TXDMA_COS"] = txs_csr.cnt_sch_txdma_cos12.all().convert_to<uint64_t>();
    cos_sum += txs_csr.cnt_sch_txdma_cos12.all().convert_to<uint64_t>();
    txs_csr.cnt_sch_txdma_cos13.read();
    cntr_cos13["TXDMA_COS"] = txs_csr.cnt_sch_txdma_cos13.all().convert_to<uint64_t>();
    cos_sum += txs_csr.cnt_sch_txdma_cos13.all().convert_to<uint64_t>();
    txs_csr.cnt_sch_txdma_cos14.read();
    cntr_cos14["TXDMA_COS"] = txs_csr.cnt_sch_txdma_cos14.all().convert_to<uint64_t>();
    cos_sum += txs_csr.cnt_sch_txdma_cos14.all().convert_to<uint64_t>();
    txs_csr.cnt_sch_txdma_cos15.read();
    cntr_cos15["TXDMA_COS"] = txs_csr.cnt_sch_txdma_cos15.all().convert_to<uint64_t>();
    cos_sum += txs_csr.cnt_sch_txdma_cos15.all().convert_to<uint64_t>();
   TXS_PRN_MSG(setw(125) << "========================================================= TXSDMA COS CNTRS ==============================================================\n");
   TXS_PRN_MSG(setw(32) << "cos" << 0 << setw(12) << "cos" << 1 << setw(12) << "cos" << 2 << setw(12) << "cos" << 3 << setw(12) << "cos" << 4 << setw(12) << "cos" << 5 << setw(12) << "cos" << 6 << setw(12) << "cos" << 7 << "\n");
   TXS_PRN_MSG(setw(125) << "----------------------------------------------------------------------------------------------------------------------------------\n");
   for(auto i : cntr_cos0) {
     string counter_name = i.first;
     counter_name.append(21-counter_name.length(),' ');
     stringstream sdata;
     stringstream rval0;
     stringstream cval0;
     if (i.second == 0) {
       rval0 << "-";
     } else {
       cos_ratio = (float) i.second / (float) cos_sum;
       cval0 << "0x" << hex << i.second;
       rval0 << std::setprecision(2) << cos_ratio;
     }
     stringstream rval1;
     stringstream cval1;
     rval1.str("-");
     cval1.str("-");
     if (cntr_cos1[i.first] != 0) {
       cos_ratio = (float) cntr_cos1[i.first] / (float) cos_sum;
       cval1 << "0x" << hex << cntr_cos1[i.first];
       rval1 << std::setprecision(2) << cos_ratio;
     }
     stringstream rval2;
     stringstream cval2;
     rval2.str("-");
     cval2.str("-");
     if (cntr_cos2[i.first] != 0) {
       cos_ratio = (float) cntr_cos2[i.first] / (float) cos_sum;
       cval2 << "0x" << hex << cntr_cos2[i.first];
       rval2 << std::setprecision(2) << cos_ratio;
     }
     stringstream rval3;
     stringstream cval3;
     rval3.str("-");
     cval3.str("-");
     if (cntr_cos3[i.first] != 0) {
       cos_ratio = (float) cntr_cos3[i.first] / (float) cos_sum;
       cval3 << "0x" << hex << cntr_cos3[i.first];
       rval3 << std::setprecision(2) << cos_ratio;
     }
     stringstream rval4;
     stringstream cval4;
     rval4.str("-");
     cval4.str("-");
     if (cntr_cos4[i.first] != 0) {
       cos_ratio = (float) cntr_cos4[i.first] / (float) cos_sum;
       cval4 << "0x" << hex << cntr_cos4[i.first];
       rval4 << std::setprecision(2) << cos_ratio;
     }
     stringstream rval5;
     stringstream cval5;
     rval5.str("-");
     cval5.str("-");
     if (cntr_cos5[i.first] != 0) {
       cos_ratio = (float) cntr_cos5[i.first] / (float) cos_sum;
       cval5 << "0x" << hex << cntr_cos5[i.first];
       rval5 << std::setprecision(2) << cos_ratio;
     }
     stringstream rval6;
     stringstream cval6;
     rval6.str("-");
     cval6.str("-");
     if (cntr_cos6[i.first] != 0) {
       cos_ratio = (float) cntr_cos6[i.first] / (float) cos_sum;
       cval6 << "0x" << hex << cntr_cos6[i.first];
       rval6 << std::setprecision(2) << cos_ratio;
     }
     stringstream rval7;
     stringstream cval7;
     rval7.str("-");
     cval7.str("-");
     if (cntr_cos7[i.first] != 0) {
       cos_ratio = (float) cntr_cos7[i.first] / (float) cos_sum;
       cval7 << "0x" << hex << cntr_cos7[i.first];
       rval7 << std::setprecision(2) << cos_ratio;
     }
     sdata.str("");
     sdata << setw(13) << cval0.str() << setw(13) << cval1.str() << setw(13) << cval2.str() << setw(13) << cval3.str() << setw(13) << cval4.str() << setw(13) << cval5.str() << setw(13) << cval6.str() << setw(13) << cval7.str();
     TXS_PRN_MSG(setw(17) << counter_name << sdata.str() << "\n");
     sdata.str("");
     sdata << setw(13) << rval0.str() << setw(13) << rval1.str() << setw(13) << rval2.str() << setw(13) << rval3.str() << setw(13) << rval4.str() << setw(13) << rval5.str() << setw(13) << rval6.str() << setw(13) << rval7.str();
     TXS_PRN_MSG(setw(17) << counter_name << sdata.str() << "\n\n");
   }

   TXS_PRN_MSG(setw(32) << "cos" << 8 << setw(12) << "cos" << 9 << setw(12) << "cos" << 10 << setw(11) << "cos" << 11 << setw(11) << "cos" << 12 << setw(11) << "cos" << 13 << setw(11) << "cos" << 14 << setw(11) << "cos" << 15 << "\n");
   TXS_PRN_MSG(setw(125) << "----------------------------------------------------------------------------------------------------------------------------------\n");
   for(auto i : cntr_cos8) {
     string counter_name = i.first;
     counter_name.append(21-counter_name.length(),' ');
     stringstream sdata;
     stringstream rval0;
     stringstream cval0;
     if (i.second == 0) {
       rval0 << "-";
     } else {
       cos_ratio = (float) i.second / (float) cos_sum;
       cval0 << "0x" << hex << i.second;
       rval0 << std::setprecision(2) << cos_ratio;
     }
     stringstream rval1;
     stringstream cval1;
     rval1.str("-");
     cval1.str("-");
     if (cntr_cos9[i.first] != 0) {
       cos_ratio = (float) cntr_cos9[i.first] / (float) cos_sum;
       cval1 << "0x" << hex << cntr_cos9[i.first];
       rval1 << std::setprecision(2) << cos_ratio;
     }
     stringstream rval2;
     stringstream cval2;
     rval2.str("-");
     cval2.str("-");
     if (cntr_cos10[i.first] != 0) {
       cos_ratio = (float) cntr_cos10[i.first] / (float) cos_sum;
       cval2 << "0x" << hex << cntr_cos10[i.first];
       rval2 << std::setprecision(2) << cos_ratio;
     }
     stringstream rval3;
     stringstream cval3;
     rval3.str("-");
     cval3.str("-");
     if (cntr_cos11[i.first] != 0) {
       cos_ratio = (float) cntr_cos11[i.first] / (float) cos_sum;
       cval3 << "0x" << hex << cntr_cos11[i.first];
       rval3 << std::setprecision(2) << cos_ratio;
     }
     stringstream rval4;
     stringstream cval4;
     rval4.str("-");
     cval4.str("-");
     if (cntr_cos12[i.first] != 0) {
       cos_ratio = (float) cntr_cos12[i.first] / (float) cos_sum;
       cval4 << "0x" << hex << cntr_cos12[i.first];
       rval4 << std::setprecision(2) << cos_ratio;
     }
     stringstream rval5;
     stringstream cval5;
     rval5.str("-");
     cval5.str("-");
     if (cntr_cos13[i.first] != 0) {
       cos_ratio = (float) cntr_cos13[i.first] / (float) cos_sum;
       cval5 << "0x" << hex << cntr_cos13[i.first];
       rval5 << std::setprecision(2) << cos_ratio;
     }
     stringstream rval6;
     stringstream cval6;
     rval6.str("-");
     cval6.str("-");
     if (cntr_cos14[i.first] != 0) {
       cos_ratio = (float) cntr_cos14[i.first] / (float) cos_sum;
       cval6 << "0x" << hex << cntr_cos14[i.first];
       rval6 << std::setprecision(2) << cos_ratio;
     }
     stringstream rval7;
     stringstream cval7;
     rval7.str("-");
     cval7.str("-");
     if (cntr_cos15[i.first] != 0) {
       cos_ratio = (float) cntr_cos15[i.first] / (float) cos_sum;
       cval7 << "0x" << hex << cntr_cos15[i.first];
       rval7 << std::setprecision(2) << cos_ratio;
     }
     sdata.str("");
     sdata << setw(13) << cval0.str() << setw(13) << cval1.str() << setw(13) << cval2.str() << setw(13) << cval3.str() << setw(13) << cval4.str() << setw(13) << cval5.str() << setw(13) << cval6.str() << setw(13) << cval7.str();
     TXS_PRN_MSG(setw(17) << counter_name << sdata.str() << "\n");
     sdata.str("");
     sdata << setw(13) << rval0.str() << setw(13) << rval1.str() << setw(13) << rval2.str() << setw(13) << rval3.str() << setw(13) << rval4.str() << setw(13) << rval5.str() << setw(13) << rval6.str() << setw(13) << rval7.str();
     TXS_PRN_MSG(setw(17) << counter_name << sdata.str() << "\n\n");
   }

}

void cap_txs_clr_tx_cos(int chip_id, int inst_id) {

    cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);

    txs_csr.cnt_sch_txdma_cos0.all(0);
    txs_csr.cnt_sch_txdma_cos0.write();
    txs_csr.cnt_sch_txdma_cos1.all(0);
    txs_csr.cnt_sch_txdma_cos1.write();
    txs_csr.cnt_sch_txdma_cos2.all(0);
    txs_csr.cnt_sch_txdma_cos2.write();
    txs_csr.cnt_sch_txdma_cos3.all(0);
    txs_csr.cnt_sch_txdma_cos3.write();
    txs_csr.cnt_sch_txdma_cos4.all(0);
    txs_csr.cnt_sch_txdma_cos4.write();
    txs_csr.cnt_sch_txdma_cos5.all(0);
    txs_csr.cnt_sch_txdma_cos5.write();
    txs_csr.cnt_sch_txdma_cos6.all(0);
    txs_csr.cnt_sch_txdma_cos6.write();
    txs_csr.cnt_sch_txdma_cos7.all(0);
    txs_csr.cnt_sch_txdma_cos7.write();
    txs_csr.cnt_sch_txdma_cos8.all(0);
    txs_csr.cnt_sch_txdma_cos8.write();
    txs_csr.cnt_sch_txdma_cos9.all(0);
    txs_csr.cnt_sch_txdma_cos9.write();
    txs_csr.cnt_sch_txdma_cos10.all(0);
    txs_csr.cnt_sch_txdma_cos10.write();
    txs_csr.cnt_sch_txdma_cos11.all(0);
    txs_csr.cnt_sch_txdma_cos11.write();
    txs_csr.cnt_sch_txdma_cos12.all(0);
    txs_csr.cnt_sch_txdma_cos12.write();
    txs_csr.cnt_sch_txdma_cos13.all(0);
    txs_csr.cnt_sch_txdma_cos13.write();
    txs_csr.cnt_sch_txdma_cos14.all(0);
    txs_csr.cnt_sch_txdma_cos14.write();
    txs_csr.cnt_sch_txdma_cos15.all(0);
    txs_csr.cnt_sch_txdma_cos15.write();
}

void cap_txs_dump_cntr(int chip_id, int inst_id) {
    cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);

    txs_csr.cnt_sch_doorbell_set.read();
    txs_csr.cnt_sch_doorbell_clr.read();
    txs_csr.cnt_sch_fifo_empty.read();
    txs_csr.cnt_sch_rlid_start.read();
    txs_csr.cnt_sch_rlid_stop.read();

    uint64_t cnt_sch_doorbell_set = txs_csr.cnt_sch_doorbell_set.all().convert_to<uint64_t>();
    uint64_t cnt_sch_doorbell_clr = txs_csr.cnt_sch_doorbell_clr.all().convert_to<uint64_t>();
    uint64_t cnt_sch_fifo_empty   = txs_csr.cnt_sch_fifo_empty.all().convert_to<uint64_t>();
    uint64_t cnt_sch_rlid_start   = txs_csr.cnt_sch_rlid_start.all().convert_to<uint64_t>();
    uint64_t cnt_sch_rlid_stop    = txs_csr.cnt_sch_rlid_stop.all().convert_to<uint64_t>();

    stringstream sdata;
    sdata.str("");
    TXS_PRN_MSG(setw(1) << "========================================================= TXS CNTRS ==============================================================\n");
    TXS_PRN_MSG(setw(32) << "cnt_sch_doorbell_set" << setw(10) << cnt_sch_doorbell_set << "\n");
    TXS_PRN_MSG(setw(32) << "cnt_sch_doorbell_clr" << setw(10) << cnt_sch_doorbell_clr << "\n");
    TXS_PRN_MSG(setw(32) << "cnt_sch_fifo_empty" << setw(10) << cnt_sch_fifo_empty << "\n");
    TXS_PRN_MSG(setw(32) << "cnt_sch_rlid_start" << setw(10) << cnt_sch_rlid_start << "\n");
    TXS_PRN_MSG(setw(32) << "cnt_sch_rlid_stop" << setw(10) << cnt_sch_rlid_stop << "\n");
}

void cap_txs_clr_cntr(int chip_id, int inst_id) {
    cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);

    txs_csr.cnt_sch_doorbell_set.all(0);
    txs_csr.cnt_sch_doorbell_clr.all(0);
    txs_csr.cnt_sch_fifo_empty.all(0);
    txs_csr.cnt_sch_rlid_start.all(0);
    txs_csr.cnt_sch_rlid_stop.all(0);

    txs_csr.cnt_sch_doorbell_set.write();
    txs_csr.cnt_sch_doorbell_clr.write();
    txs_csr.cnt_sch_fifo_empty.write();
    txs_csr.cnt_sch_rlid_start.write();
    txs_csr.cnt_sch_rlid_stop.write();
}

void cap_tmr_dump_cntr(int chip_id, int inst_id) {
    cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);

    txs_csr.cnt_ftmr_push.read();
    txs_csr.cnt_ftmr_pop.read();
    txs_csr.cnt_ftmr_key_not_push.read();
    txs_csr.cnt_ftmr_key_not_found.read();
    txs_csr.cnt_ftmr_push_out_of_wheel.read();

    txs_csr.cnt_stmr_push.read();
    txs_csr.cnt_stmr_pop.read();
    txs_csr.cnt_stmr_key_not_push.read();
    txs_csr.cnt_stmr_key_not_found.read();
    txs_csr.cnt_stmr_push_out_of_wheel.read();

    txs_csr.sta_sch_max_hbm_byp.read();
    txs_csr.sta_tmr_max_hbm_byp.read();
    txs_csr.sta_tmr_max_keys.read();
    txs_csr.sta_ftmr_max_bcnt.read();
    txs_csr.sta_stmr_max_bcnt.read();
   
    uint64_t cnt_ftmr_push               = txs_csr.cnt_ftmr_push.all().convert_to<uint64_t>();
    uint64_t cnt_ftmr_pop                = txs_csr.cnt_ftmr_pop.all().convert_to<uint64_t>();
    uint64_t cnt_ftmr_key_not_push       = txs_csr.cnt_ftmr_key_not_push.all().convert_to<uint64_t>();
    uint64_t cnt_ftmr_key_not_found      = txs_csr.cnt_ftmr_key_not_found.all().convert_to<uint64_t>();
    uint64_t cnt_ftmr_push_out_of_wheel  = txs_csr.cnt_ftmr_push_out_of_wheel.all().convert_to<uint64_t>();

    uint64_t cnt_stmr_push               = txs_csr.cnt_stmr_push.all().convert_to<uint64_t>();
    uint64_t cnt_stmr_pop                = txs_csr.cnt_stmr_pop.all().convert_to<uint64_t>();
    uint64_t cnt_stmr_key_not_push       = txs_csr.cnt_stmr_key_not_push.all().convert_to<uint64_t>();
    uint64_t cnt_stmr_key_not_found      = txs_csr.cnt_stmr_key_not_found.all().convert_to<uint64_t>();
    uint64_t cnt_stmr_push_out_of_wheel  = txs_csr.cnt_stmr_push_out_of_wheel.all().convert_to<uint64_t>();

    cpp_int schHbmByp = txs_csr.sta_sch_max_hbm_byp.cnt();
    cpp_int tmrHbmByp = txs_csr.sta_tmr_max_hbm_byp.cnt();
    cpp_int tmrKeysNum = txs_csr.sta_tmr_max_keys.cnt();
    cpp_int ftmrBcntNum = txs_csr.sta_ftmr_max_bcnt.bcnt();
    cpp_int stmrBcntNum = txs_csr.sta_stmr_max_bcnt.bcnt();
 
    TXS_PRN_MSG(setw(1) << "========================================================= TMR CNTRS ==============================================================\n");
    TXS_PRN_MSG(setw(1) << "--------------------------------------------------------- fast cntrs -------------------------------------------------------------\n");
    TXS_PRN_MSG(setw(32) << "cnt_ftmr_push" << setw(10) << cnt_ftmr_push << "\n");
    TXS_PRN_MSG(setw(32) << "cnt_ftmr_pop" << setw(10) << cnt_ftmr_pop << "\n");
    TXS_PRN_MSG(setw(32) << "cnt_ftmr_key_not_push" << setw(10) << cnt_ftmr_key_not_push << "\n");
    TXS_PRN_MSG(setw(32) << "cnt_ftmr_key_not_found" << setw(10) << cnt_ftmr_key_not_found << "\n");
    TXS_PRN_MSG(setw(32) << "cnt_ftmr_push_out_of_wheel" << setw(10) << cnt_ftmr_push_out_of_wheel << "\n");
    TXS_PRN_MSG(setw(1) << "--------------------------------------------------------- slow cntrs -------------------------------------------------------------\n");
    TXS_PRN_MSG(setw(32) << "cnt_stmr_push" << setw(10) << cnt_stmr_push << "\n");
    TXS_PRN_MSG(setw(32) << "cnt_stmr_pop" << setw(10) << cnt_stmr_pop << "\n");
    TXS_PRN_MSG(setw(32) << "cnt_stmr_key_not_push" << setw(10) << cnt_stmr_key_not_push << "\n");
    TXS_PRN_MSG(setw(32) << "cnt_stmr_key_not_found" << setw(10) << cnt_stmr_key_not_found << "\n");
    TXS_PRN_MSG(setw(32) << "cnt_stmr_push_out_of_wheel" << setw(10) << cnt_stmr_push_out_of_wheel << "\n");
    TXS_PRN_MSG(setw(1) << "--------------------------------------------------------- tmr cntrs -------------------------------------------------------------\n");
    TXS_PRN_MSG(setw(32) << "sta_ftmr_max_bcnt" << setw(10) << ftmrBcntNum << "\n");
    TXS_PRN_MSG(setw(32) << "sta_stmr_max_bcnt" << setw(10) << stmrBcntNum << "\n");
    TXS_PRN_MSG(setw(32) << "sta_tmr_max_keys" << setw(10) << tmrKeysNum << "\n");
    TXS_PRN_MSG(setw(32) << "sta_sch_max_hbm_byp" << setw(10) << schHbmByp << "\n");
    TXS_PRN_MSG(setw(32) << "sta_tmr_max_hbm_byp" << setw(10) << tmrHbmByp << "\n");
}

void cap_tmr_clr_cntr(int chip_id, int inst_id) {
    cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);

    txs_csr.cnt_ftmr_push.all(0);
    txs_csr.cnt_ftmr_pop.all(0);
    txs_csr.cnt_ftmr_key_not_push.all(0);
    txs_csr.cnt_ftmr_key_not_found.all(0);
    txs_csr.cnt_ftmr_push_out_of_wheel.all(0);

    txs_csr.cnt_stmr_push.all(0);
    txs_csr.cnt_stmr_pop.all(0);
    txs_csr.cnt_stmr_key_not_push.all(0);
    txs_csr.cnt_stmr_key_not_found.all(0);
    txs_csr.cnt_stmr_push_out_of_wheel.all(0);

    txs_csr.cnt_ftmr_push.write();
    txs_csr.cnt_ftmr_pop.write();
    txs_csr.cnt_ftmr_key_not_push.write();
    txs_csr.cnt_ftmr_key_not_found.write();
    txs_csr.cnt_ftmr_push_out_of_wheel.write();

    txs_csr.cnt_stmr_push.write();
    txs_csr.cnt_stmr_pop.write();
    txs_csr.cnt_stmr_key_not_push.write();
    txs_csr.cnt_stmr_key_not_found.write();
    txs_csr.cnt_stmr_push_out_of_wheel.write();
}

void cap_tmr_bcnt_lcnt_cntr(int chip_id, int inst_id) {

    cpp_int ftmr_cbcnt;
    cpp_int ftmr_bcnt;
    cpp_int ftmr_lcnt;
    cpp_int stmr_cbcnt;
    cpp_int stmr_bcnt;
    cpp_int stmr_lcnt;

    cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);

    txs_csr.cfg_timer_static.read();
    cpp_int tmr_wheel_depth =  txs_csr.cfg_timer_static.tmr_wheel_depth().convert_to<int>();

    txs_csr.cfw_timer_glb.read();
    cpp_int  ftmr_enable = txs_csr.cfw_timer_glb.ftmr_enable().convert_to<int>();
    cpp_int  stmr_enable = txs_csr.cfw_timer_glb.stmr_enable().convert_to<int>();

    TXS_PRN_MSG(setw(10) << "===================================== TMR_BCNT_LCNT ftmr_enable:" << ftmr_enable << " stmr_enable:" << stmr_enable << " tmr_wheel_depth:" << tmr_wheel_depth << " ======================================\n");
    for (int i=0; i<tmr_wheel_depth; i++) {
       txs_csr.dhs_tmr_cnt_sram.entry[i].read();
       if (ftmr_enable) {
          ftmr_cbcnt = txs_csr.dhs_tmr_cnt_sram.entry[i].fast_cbcnt();
          ftmr_bcnt  = txs_csr.dhs_tmr_cnt_sram.entry[i].fast_bcnt();
          ftmr_lcnt  = txs_csr.dhs_tmr_cnt_sram.entry[i].fast_lcnt();
       } else {
          ftmr_cbcnt = 0;
          ftmr_bcnt  = 0;
          ftmr_lcnt  = 0;
       }
       if (stmr_enable) {
          stmr_cbcnt = txs_csr.dhs_tmr_cnt_sram.entry[i].slow_cbcnt();
          stmr_bcnt  = txs_csr.dhs_tmr_cnt_sram.entry[i].slow_bcnt();
          stmr_lcnt  = txs_csr.dhs_tmr_cnt_sram.entry[i].slow_lcnt();
       } else {
          stmr_cbcnt = 0;
          stmr_bcnt  = 0;
          stmr_lcnt  = 0;
       }
       if (ftmr_cbcnt != 0 || ftmr_bcnt != 0 || ftmr_lcnt != 0 || stmr_cbcnt != 0 || stmr_bcnt != 0 || stmr_lcnt != 0) {
          TXS_PRN_MSG(setw(10) << "tick:" << i << " slow_cbcnt:" << setw(3) << stmr_cbcnt << " slow_bcnt:" << setw(3) << stmr_bcnt << " slow_lcnt:" << setw(3)<< stmr_lcnt);
          TXS_PRN_MSG(setw(10)  << " fast_cbcnt:" << setw(3) << ftmr_cbcnt << " fast_bcnt:" << setw(3) << ftmr_bcnt << " fast_lcnt:" << setw(3)<< ftmr_lcnt << "\n");
       }
    }
    TXS_PRN_MSG(setw(10) << "---------------------------------------------------------------------------------------------------------------------------------\n");
}


void cap_txs_dump_sch_grp_cntr(int chip_id, int inst_id, int print_nonzero_drb) {

    cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);

    int non_zero_pop  = 0;
    int non_zero_drb  = 0;
    int print_cnt = 0;

    map <string, uint64_t> cntr_pop[8];
    map <string, uint64_t> cntr_drb[8];
    
    TXS_PRN_MSG(setw(125) << "========================================================= SCH_GRP_CNT_ENTRY pop,drb (non zero) =========================================================\n");

  for (int ss=0; ss<256; ss++) {

   print_cnt = 0;
   for (int pp=0; pp<8; pp++) {
    txs_csr.dhs_sch_grp_cnt_entry.entry[ss*8 + pp].read();
    cntr_pop[pp]["SCH_GRP_CNT"] = txs_csr.dhs_sch_grp_cnt_entry.entry[ss*8 + pp].popcnt().convert_to<uint64_t>();
    cntr_drb[pp]["SCH_GRP_CNT"] = txs_csr.dhs_sch_grp_cnt_entry.entry[ss*8 + pp].drb_cnt().convert_to<uint64_t>();
    if (cntr_pop[pp]["SCH_GRP_CNT"] != 0x0ul) {
       if (print_nonzero_drb == 0) {
          print_cnt = 1;
       }
       non_zero_pop++;
    }
    if (cntr_drb[pp]["SCH_GRP_CNT"] != 0x0ul) {
       if (print_nonzero_drb == 1) {
          print_cnt = 1;
       }
       non_zero_drb++;
    }
   }

    if (print_cnt == 1) {
      TXS_PRN_MSG(setw(32) << "sg" << setfill(' ') << setw(4) << (ss*8+0) << setw(11) << "sg" << setfill(' ') << setw(4) << (ss*8+1) << setw(11) << "sg" << setfill(' ') << setw(4) << (ss*8+2) << setw(11) << "sg" << setfill(' ') << setw(4) << (ss*8+3) << setw(11) << "sg" << setfill(' ') << setw(4) << (ss*8+4) << setw(11) << "sg" << setfill(' ') << setw(4) << (ss*8+5) << setw(11) << "sg" << setfill(' ') << setw(4) << (ss*8+6) << setw(11) << "sg" << setfill(' ') << setw(4) << (ss*8+7) << "\n");

     for(auto i : cntr_pop[0]) {
       string counter_name = i.first;
       counter_name.append(21-counter_name.length(),' ');
       stringstream sdata;
       sdata.str("");

       stringstream rval0;
       rval0.str("-");
       if (cntr_pop[0][i.first] != 0) {
         //rval0 << "0x" << hex << setfill(' ') << setw(2) << cntr_pop[0][i.first] << ",0x" << setfill(' ') << setw(4) << cntr_drb[0][i.first];
         rval0 << setfill(' ') << setw(2) << cntr_pop[0][i.first] << "," << setfill(' ') << setw(4) << cntr_drb[0][i.first];
       }
       stringstream rval1;
       rval1.str("-");
       if (cntr_pop[1][i.first] != 0) {
         //rval1 << "0x" << hex << setfill(' ') << setw(2) << cntr_pop[1][i.first] << ",0x" << setfill(' ') << setw(4) << cntr_drb[1][i.first];
         rval1 << setfill(' ') << setw(2) << cntr_pop[1][i.first] << "," << setfill(' ') << setw(4) << cntr_drb[1][i.first];
       }
       stringstream rval2;
       rval2.str("-");
       if (cntr_pop[2][i.first] != 0) {
         //rval2 << "0x" << hex << setfill(' ') << setw(2) << cntr_pop[2][i.first] << ",0x" << setfill(' ') << setw(4) << cntr_drb[2][i.first];
         rval2 << setfill(' ') << setw(2) << cntr_pop[2][i.first] << "," << setfill(' ') << setw(4) << cntr_drb[2][i.first];
       }
       stringstream rval3;
       rval3.str("-");
       if (cntr_pop[3][i.first] != 0) {
         //rval3 << "0x" << hex << setfill(' ') << setw(2) << cntr_pop[3][i.first] << ",0x" << setfill(' ') << setw(4) << cntr_drb[3][i.first];
         rval3 << setfill(' ') << setw(2) << cntr_pop[3][i.first] << "," << setfill(' ') << setw(4) << cntr_drb[3][i.first];
       }
       stringstream rval4;
       rval4.str("-");
       if (cntr_pop[4][i.first] != 0) {
         //rval4 << "0x" << hex << setfill(' ') << setw(2) << cntr_pop[4][i.first] << ",0x" << setfill(' ') << setw(4) << cntr_drb[4][i.first];
         rval4 << setfill(' ') << setw(2) << cntr_pop[4][i.first] << "," << setfill(' ') << setw(4) << cntr_drb[4][i.first];
       }
       stringstream rval5;
       rval5.str("-");
       if (cntr_pop[5][i.first] != 0) {
         //rval5 << "0x" << hex << setfill(' ') << setw(2) << cntr_pop[5][i.first] << ",0x" << setfill(' ') << setw(4) << cntr_drb[5][i.first];
         rval5 << setfill(' ') << setw(2) << cntr_pop[5][i.first] << "," << setfill(' ') << setw(4) << cntr_drb[5][i.first];
       }
       stringstream rval6;
       rval6.str("-");
       if (cntr_pop[6][i.first] != 0) {
         //rval6 << "0x" << hex << setfill(' ') << setw(2) << cntr_pop[6][i.first] << ",0x" << setfill(' ') << setw(4) << cntr_drb[6][i.first];
         rval6 << setfill(' ') << setw(2) << cntr_pop[6][i.first] << "," << setfill(' ') << setw(4) << cntr_drb[6][i.first];
       }
       stringstream rval7;
       rval7.str("-");
       if (cntr_pop[7][i.first] != 0) {
         //rval7 << "0x" << hex << setfill(' ') << setw(2) << cntr_pop[7][i.first] << ",0x" << setfill(' ') << setw(4) << cntr_drb[7][i.first];
         rval7 << setfill(' ') << setw(2) << cntr_pop[7][i.first] << "," << setfill(' ') << setw(4) << cntr_drb[7][i.first];
       }
       sdata << setw(15) << rval0.str() << setw(15) << rval1.str() << setw(15) << rval2.str() << setw(15) << rval3.str() << setw(15) << rval4.str() << setw(15) << rval5.str() << setw(15) << rval6.str() << setw(15) << rval7.str();
       TXS_PRN_MSG(setw(17) << counter_name << sdata.str() << "\n\n");
     }
   } else {
      ////TXS_PRN_MSG(setw(32) << "sg" << setfill(' ') << setw(4) << (ss*8+0) << setw(14) << "sg" << setfill(' ') << setw(4) << (ss*8+1) << setw(14) << "sg" << setfill(' ') << setw(4) << (ss*8+2) << setw(14) << "sg" << setfill(' ') << setw(4) << (ss*8+3) << setw(14) << "sg" << setfill(' ') << setw(4) << (ss*8+4) << setw(14) << "sg" << setfill(' ') << setw(4) << (ss*8+5) << setw(14) << "sg" << setfill(' ') << setw(4) << (ss*8+6) << setw(14) << "sg" << setfill(' ') << setw(4) << (ss*8+7) << "\n");
   }
  }

   if (non_zero_pop == 0) { 
      TXS_PRN_MSG(setw(0) << "SCH_GRP_CNT  sg0 - sg2047 cnt_pop:0" << "\n\n");
   } else {
      TXS_PRN_MSG(setw(0) << "SCH_GRP_CNT  total sch_grp with cnt_pop>0 " << non_zero_pop << "\n\n");
   }
   if (non_zero_drb == 0) { 
      TXS_PRN_MSG(setw(0) << "SCH_GRP_CNT  sg0 - sg2047 cnt_drb:0" << "\n\n");
   } else {
      TXS_PRN_MSG(setw(0) << "SCH_GRP_CNT  total sch_grp with cnt_drb>0 " << non_zero_drb << "\n\n");
   }
}

void cap_txs_mon_xoff(int chip_id, int inst_id, int sample) {

    cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);

    uint64_t rd_val; 
    int      xoff_cnt[16];
    stringstream sdata;
    stringstream xoff0;
    stringstream xoff1;
    stringstream xoff2;
    stringstream xoff3;
    stringstream xoff4;
    stringstream xoff5;
    stringstream xoff6;
    stringstream xoff7;
    for (int j=0; j<16; j++) {
       xoff_cnt[j] = 0;
    }

    for (int i=0; i<sample; i++) {
       txs_csr.sta_glb.read();
       rd_val =  txs_csr.sta_glb.pb_xoff().convert_to<uint64_t>();
       if (rd_val > 0x0) {
          for (int j=0; j<16; j++) {
             if ((rd_val & (1<<j)) != 0) {
                xoff_cnt[j]++;
             }
          }
       }
    }

   TXS_PRN_MSG("========================================================= XOFF MON /" << sample << " samples ==============================================================\n");
   TXS_PRN_MSG(setw(32) << "cos" << 0 << setw(11) << "cos" << 1 << setw(11) << "cos" << 2 << setw(11) << "cos" << 3 << setw(11) << "cos" << 4 << setw(11) << "cos" << 5 << setw(11) << "cos" << 6 << setw(11) << "cos" << 7 << "\n");
   TXS_PRN_MSG("-----------------------------------------------------------------------------------------------------------------------------------------------------------\n");
     sdata.str("");
     xoff0.str("");
     if (xoff_cnt[0] == 0) {
        xoff0 << "-";
     } else {
        xoff0 << xoff_cnt[0];
     }
     xoff1.str("");
     if (xoff_cnt[1] == 0) {
        xoff1 << "-";
     } else {
        xoff1 << xoff_cnt[1];
     }
     xoff2.str("");
     if (xoff_cnt[2] == 0) {
        xoff2 << "-";
     } else {
        xoff2 << xoff_cnt[2];
     }
     xoff3.str("");
     if (xoff_cnt[3] == 0) {
        xoff3 << "-";
     } else {
        xoff3 << xoff_cnt[3];
     }
     xoff4.str("");
     if (xoff_cnt[4] == 0) {
        xoff4 << "-";
     } else {
        xoff4 << xoff_cnt[4];
     }
     xoff5.str("");
     if (xoff_cnt[5] == 0) {
        xoff5 << "-";
     } else {
        xoff5 << xoff_cnt[5];
     }
     xoff6.str("");
     if (xoff_cnt[6] == 0) {
        xoff6 << "-";
     } else {
        xoff6 << xoff_cnt[6];
     }
     xoff7.str("");
     if (xoff_cnt[7] == 0) {
        xoff7 << "-";
     } else {
        xoff7 << xoff_cnt[7];
     }
     sdata << setw(12) << xoff0.str() << setw(12) << xoff1.str() << setw(12) << xoff2.str() << setw(12) << xoff3.str() << setw(12) << xoff4.str() << setw(12) << xoff5.str() << setw(12) << xoff6.str() << setw(12) << xoff7.str();
    TXS_PRN_MSG(setw(20) << "xoff " << sdata.str() << "\n");
   TXS_PRN_MSG(setw(32) << "cos" << 8 << setw(11) << "cos" << 9 << setw(11) << "cos" << 10 << setw(10) << "cos" << 11 << setw(10) << "cos" << 12 << setw(10) << "cos" << 13 << setw(10) << "cos" << 14 << setw(10) << "cos" << 15 << "\n");
   TXS_PRN_MSG("-----------------------------------------------------------------------------------------------------------------------------------------------------------\n");
     sdata.str("");
     xoff0.str("");
     if (xoff_cnt[8] == 0) {
        xoff0 << "-";
     } else {
        xoff0 << xoff_cnt[8];
     }
     xoff1.str("");
     if (xoff_cnt[9] == 0) {
        xoff1 << "-";
     } else {
        xoff1 << xoff_cnt[9];
     }
     xoff2.str("");
     if (xoff_cnt[10] == 0) {
        xoff2 << "-";
     } else {
        xoff2 << xoff_cnt[10];
     }
     xoff3.str("");
     if (xoff_cnt[11] == 0) {
        xoff3 << "-";
     } else {
        xoff3 << xoff_cnt[11];
     }
     xoff4.str("");
     if (xoff_cnt[12] == 0) {
        xoff4 << "-";
     } else {
        xoff4 << xoff_cnt[12];
     }
     xoff5.str("");
     if (xoff_cnt[13] == 0) {
        xoff5 << "-";
     } else {
        xoff5 << xoff_cnt[13];
     }
     xoff6.str("");
     if (xoff_cnt[14] == 0) {
        xoff6 << "-";
     } else {
        xoff6 << xoff_cnt[14];
     }
     xoff7.str("");
     if (xoff_cnt[15] == 0) {
        xoff7 << "-";
     } else {
        xoff7 << xoff_cnt[15];
     }
     sdata << setw(12) << xoff0.str() << setw(12) << xoff1.str() << setw(12) << xoff2.str() << setw(12) << xoff3.str() << setw(12) << xoff4.str() << setw(12) << xoff5.str() << setw(12) << xoff6.str() << setw(12) << xoff7.str();
    TXS_PRN_MSG(setw(20) << "xoff " << sdata.str() << "\n");
}

void cap_txs_mon_txdma_drdy(int chip_id, int inst_id, int sample) {

    cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);

    uint64_t rd_val; 
    int      drdy_cnt = 0;
    for (int i=0; i<sample; i++) {
       txs_csr.sta_scheduler_dbg.read();
       rd_val =  txs_csr.sta_scheduler_dbg.txdma_drdy().convert_to<uint64_t>();
       if (rd_val == 0x1) {
          drdy_cnt++;
       }
    }

    TXS_PRN_MSG(setw(32) << "txdma_drdy : " << drdy_cnt  << "/" << sample << "\n");
}

// cos_idx -> cos
int cap_txs_get_cos(int chip_id, int inst_id, int lif, int cos_idx) {

    cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);

    uint64_t sg_act_cos;
    int      idx = 0;

    txs_csr.dhs_sch_lif_map_sram.entry[lif].read();
    sg_act_cos = txs_csr.dhs_sch_lif_map_sram.entry[lif].sg_act_cos().convert_to<uint64_t>();

    for (int i=0; i<16; i++) {
       if ((sg_act_cos>>i) & 0x1) {
          if (idx == cos_idx) {
             return i;
          }
          idx++;
       }
    }
    
    PLOG_MSG ("cap_txs_get_cos:: cos not found lif:" << lif << " cos_idx:" << cos_idx << " sg_act_cos:0x" << hex << sg_act_cos << dec << endl);

    return -1;
}

// cos -> cos_idx
int cap_txs_get_cos_idx(int chip_id, int inst_id, int lif, int cos) {

    cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);

    uint64_t sg_act_cos;
    int      cos_idx = 0;

    txs_csr.dhs_sch_lif_map_sram.entry[lif].read();
    sg_act_cos = txs_csr.dhs_sch_lif_map_sram.entry[lif].sg_act_cos().convert_to<uint64_t>();

    for (int i=0; i<16; i++) {
       if ((sg_act_cos>>i) & 0x1) {
          if (i == cos) {
             return cos_idx;
          }
          cos_idx++;
       }
    }
    
    PLOG_MSG ("cap_txs_get_cos_idx:: cos not found lif:" << lif << " cos:" << cos << " sg_act_cos:0x" << hex << sg_act_cos << dec << endl);

    return -1;
}


// sw db set
// cos 
void cap_txs_drb_set(int chip_id, int inst_id, int lif, int cos, int qid, int set) {

    cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);
    // Read DHS reg
    txs_csr.dhs_doorbell.entry[lif].qid(qid);
    txs_csr.dhs_doorbell.entry[lif].cos(cos);
    txs_csr.dhs_doorbell.entry[lif].set(set);

    txs_csr.dhs_doorbell.entry[lif].write();

}

int cap_txs_drb_get(int chip_id, int inst_id, int lif, int cos, int qid) {

    cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);
    // Write which entry to read
    txs_csr.cfg_scheduler_dbg2.qid_read(qid);
    txs_csr.cfg_scheduler_dbg2.cos_read(cos);
    txs_csr.cfg_scheduler_dbg2.write();
    // Read DHS reg
    txs_csr.dhs_doorbell.entry[lif].read();
    return txs_csr.dhs_doorbell.entry[lif].set().convert_to<uint64_t>();

}


void cap_txs_dump_tmr_pending(int chip_id, int inst_id, int tmr_fast, int print_only_nonzero_bcnt) {

    cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);

    int non_zero_bcnt  = 0;
    int non_zero_lcnt  = 0;
    int print_cnt = 0;
    string tmr_name = tmr_fast ? "FAST" : "SLOW";

    map <string, uint64_t> bcnt[8];
    map <string, uint64_t> lcnt[8];
    
    TXS_PRN_MSG(setw(25) << "========================================================= " << tmr_name << " TMR_CNT bcnt,lcnt (non zero) =========================================================\n");

  for (int ss=0; ss<512; ss++) {
   print_cnt = 0;
   for (int pp=0; ss<8; ss++) {
    if (bcnt[pp]["TMR_CNT"] != 0x0ul) {
       print_cnt = 1;
       non_zero_bcnt++;
    }
    if (lcnt[pp]["TMR_CNT"] != 0x0ul) {
       if (print_only_nonzero_bcnt == 0) {
          print_cnt = 1;
       }
       non_zero_lcnt++;
    }
   }

    if (print_cnt == 1) {
      TXS_PRN_MSG(setw(32) << "tmr" << setfill(' ') << setw(4) << (ss*8+0) << setw(10) << "tmr" << setfill(' ') << setw(4) << (ss*8+1) << setw(10) << "tmr" << setfill(' ') << setw(4) << (ss*8+2) << setw(10) << "tmr" << setfill(' ') << setw(4) << (ss*8+3) << setw(10) << "tmr" << setfill(' ') << setw(4) << (ss*8+4) << setw(10) << "tmr" << setfill(' ') << setw(4) << (ss*8+5) << setw(10) << "tmr" << setfill(' ') << setw(4) << (ss*8+6) << setw(10) << "tmr" << setfill(' ') << setw(4) << (ss*8+7) << "\n");

     for(auto i : lcnt[0]) {
       string counter_name = i.first;
       counter_name.append(21-counter_name.length(),' ');
       stringstream sdata;
       sdata.str("");

       stringstream rval0;
       if (tmr_fast == 1) {
          txs_csr.dhs_fast_timer_pending.entry[(ss*8 + 0)].read();
          bcnt[0]["BCNT"] = txs_csr.dhs_fast_timer_pending.entry[(ss*8 + 0)].bcnt().convert_to<uint64_t>();
          lcnt[0]["BCNT"] = txs_csr.dhs_fast_timer_pending.entry[(ss*8 + 0)].lcnt().convert_to<uint64_t>();
       } else {
          txs_csr.dhs_slow_timer_pending.entry[(ss*8 + 0)].read();
          bcnt[0]["BCNT"] = txs_csr.dhs_slow_timer_pending.entry[(ss*8 + 0)].bcnt().convert_to<uint64_t>();
          lcnt[0]["BCNT"] = txs_csr.dhs_slow_timer_pending.entry[(ss*8 + 0)].lcnt().convert_to<uint64_t>();
       }
       rval0.str("-");
       if (lcnt[0][i.first] != 0) {
         rval0 << setfill(' ') << setw(2) << lcnt[0][i.first] << "," << setfill(' ') << setw(4) << bcnt[0][i.first];
       }
       stringstream rval1;
       if (tmr_fast == 1) {
          txs_csr.dhs_fast_timer_pending.entry[(ss*8 + 1)].read();
          bcnt[1]["BCNT"] = txs_csr.dhs_fast_timer_pending.entry[(ss*8 + 1)].bcnt().convert_to<uint64_t>();
          lcnt[1]["BCNT"] = txs_csr.dhs_fast_timer_pending.entry[(ss*8 + 1)].lcnt().convert_to<uint64_t>();
       } else {
          txs_csr.dhs_slow_timer_pending.entry[(ss*8 + 1)].read();
          bcnt[1]["BCNT"] = txs_csr.dhs_slow_timer_pending.entry[(ss*8 + 1)].bcnt().convert_to<uint64_t>();
          lcnt[1]["BCNT"] = txs_csr.dhs_slow_timer_pending.entry[(ss*8 + 1)].lcnt().convert_to<uint64_t>();
       }
       rval1.str("-");
       if (lcnt[1][i.first] != 0) {
         rval1 << setfill(' ') << setw(2) << lcnt[1][i.first] << "," << setfill(' ') << setw(4) << bcnt[1][i.first];
       }
       stringstream rval2;
       if (tmr_fast == 1) {
          txs_csr.dhs_fast_timer_pending.entry[(ss*8 + 2)].read();
          bcnt[2]["BCNT"] = txs_csr.dhs_fast_timer_pending.entry[(ss*8 + 2)].bcnt().convert_to<uint64_t>();
          lcnt[2]["BCNT"] = txs_csr.dhs_fast_timer_pending.entry[(ss*8 + 2)].lcnt().convert_to<uint64_t>();
       } else {
          txs_csr.dhs_slow_timer_pending.entry[(ss*8 + 2)].read();
          bcnt[2]["BCNT"] = txs_csr.dhs_slow_timer_pending.entry[(ss*8 + 2)].bcnt().convert_to<uint64_t>();
          lcnt[2]["BCNT"] = txs_csr.dhs_slow_timer_pending.entry[(ss*8 + 2)].lcnt().convert_to<uint64_t>();
       }
       rval2.str("-");
       if (lcnt[2][i.first] != 0) {
         rval2 << setfill(' ') << setw(2) << lcnt[2][i.first] << "," << setfill(' ') << setw(4) << bcnt[2][i.first];
       }
       stringstream rval3;
       if (tmr_fast == 1) {
          txs_csr.dhs_fast_timer_pending.entry[(ss*8 + 3)].read();
          bcnt[3]["BCNT"] = txs_csr.dhs_fast_timer_pending.entry[(ss*8 + 3)].bcnt().convert_to<uint64_t>();
          lcnt[3]["BCNT"] = txs_csr.dhs_fast_timer_pending.entry[(ss*8 + 3)].lcnt().convert_to<uint64_t>();
       } else {
          txs_csr.dhs_slow_timer_pending.entry[(ss*8 + 3)].read();
          bcnt[3]["BCNT"] = txs_csr.dhs_slow_timer_pending.entry[(ss*8 + 3)].bcnt().convert_to<uint64_t>();
          lcnt[3]["BCNT"] = txs_csr.dhs_slow_timer_pending.entry[(ss*8 + 3)].lcnt().convert_to<uint64_t>();
       }
       rval3.str("-");
       if (lcnt[3][i.first] != 0) {
         rval3 << setfill(' ') << setw(2) << lcnt[3][i.first] << "," << setfill(' ') << setw(4) << bcnt[3][i.first];
       }
       stringstream rval4;
       if (tmr_fast == 1) {
          txs_csr.dhs_fast_timer_pending.entry[(ss*8 + 4)].read();
          bcnt[4]["BCNT"] = txs_csr.dhs_fast_timer_pending.entry[(ss*8 + 4)].bcnt().convert_to<uint64_t>();
          lcnt[4]["BCNT"] = txs_csr.dhs_fast_timer_pending.entry[(ss*8 + 4)].lcnt().convert_to<uint64_t>();
       } else {
          txs_csr.dhs_slow_timer_pending.entry[(ss*8 + 4)].read();
          bcnt[4]["BCNT"] = txs_csr.dhs_slow_timer_pending.entry[(ss*8 + 4)].bcnt().convert_to<uint64_t>();
          lcnt[4]["BCNT"] = txs_csr.dhs_slow_timer_pending.entry[(ss*8 + 4)].lcnt().convert_to<uint64_t>();
       }
       rval4.str("-");
       if (lcnt[4][i.first] != 0) {
         rval4 << setfill(' ') << setw(2) << lcnt[4][i.first] << "," << setfill(' ') << setw(4) << bcnt[4][i.first];
       }
       stringstream rval5;
       if (tmr_fast == 1) {
          txs_csr.dhs_fast_timer_pending.entry[(ss*8 + 5)].read();
          bcnt[5]["BCNT"] = txs_csr.dhs_fast_timer_pending.entry[(ss*8 + 5)].bcnt().convert_to<uint64_t>();
          lcnt[5]["BCNT"] = txs_csr.dhs_fast_timer_pending.entry[(ss*8 + 5)].lcnt().convert_to<uint64_t>();
       } else {
          txs_csr.dhs_slow_timer_pending.entry[(ss*8 + 5)].read();
          bcnt[5]["BCNT"] = txs_csr.dhs_slow_timer_pending.entry[(ss*8 + 5)].bcnt().convert_to<uint64_t>();
          lcnt[5]["BCNT"] = txs_csr.dhs_slow_timer_pending.entry[(ss*8 + 5)].lcnt().convert_to<uint64_t>();
       }
       rval5.str("-");
       if (lcnt[5][i.first] != 0) {
         rval5 << setfill(' ') << setw(2) << lcnt[5][i.first] << "," << setfill(' ') << setw(4) << bcnt[5][i.first];
       }
       stringstream rval6;
       if (tmr_fast == 1) {
          txs_csr.dhs_fast_timer_pending.entry[(ss*8 + 6)].read();
          bcnt[6]["BCNT"] = txs_csr.dhs_fast_timer_pending.entry[(ss*8 + 6)].bcnt().convert_to<uint64_t>();
          lcnt[6]["BCNT"] = txs_csr.dhs_fast_timer_pending.entry[(ss*8 + 6)].lcnt().convert_to<uint64_t>();
       } else {
          txs_csr.dhs_slow_timer_pending.entry[(ss*8 + 6)].read();
          bcnt[6]["BCNT"] = txs_csr.dhs_slow_timer_pending.entry[(ss*8 + 6)].bcnt().convert_to<uint64_t>();
          lcnt[6]["BCNT"] = txs_csr.dhs_slow_timer_pending.entry[(ss*8 + 6)].lcnt().convert_to<uint64_t>();
       }
       rval6.str("-");
       if (lcnt[6][i.first] != 0) {
         rval6 << setfill(' ') << setw(2) << lcnt[6][i.first] << "," << setfill(' ') << setw(4) << bcnt[6][i.first];
       }
       stringstream rval7;
       if (tmr_fast == 1) {
          txs_csr.dhs_fast_timer_pending.entry[(ss*8 + 7)].read();
          bcnt[7]["BCNT"] = txs_csr.dhs_fast_timer_pending.entry[(ss*8 + 7)].bcnt().convert_to<uint64_t>();
          lcnt[7]["BCNT"] = txs_csr.dhs_fast_timer_pending.entry[(ss*8 + 7)].lcnt().convert_to<uint64_t>();
       } else {
          txs_csr.dhs_slow_timer_pending.entry[(ss*8 + 7)].read();
          bcnt[7]["BCNT"] = txs_csr.dhs_slow_timer_pending.entry[(ss*8 + 7)].bcnt().convert_to<uint64_t>();
          lcnt[7]["BCNT"] = txs_csr.dhs_slow_timer_pending.entry[(ss*8 + 7)].lcnt().convert_to<uint64_t>();
       }
       rval7.str("-");
       if (lcnt[7][i.first] != 0) {
         rval7 << setfill(' ') << setw(2) << lcnt[7][i.first] << "," << setfill(' ') << setw(4) << bcnt[7][i.first];
       }
       sdata << setw(15) << rval0.str() << setw(15) << rval1.str() << setw(15) << rval2.str() << setw(15) << rval3.str() << setw(15) << rval4.str() << setw(15) << rval5.str() << setw(15) << rval6.str() << setw(15) << rval7.str();
       TXS_PRN_MSG(setw(17) << counter_name << sdata.str() << "\n\n");
     }
   } else {
      ////TXS_PRN_MSG(setw(32) << "sg" << setfill(' ') << setw(4) << (ss*8+0) << setw(14) << "sg" << setfill(' ') << setw(4) << (ss*8+1) << setw(14) << "sg" << setfill(' ') << setw(4) << (ss*8+2) << setw(14) << "sg" << setfill(' ') << setw(4) << (ss*8+3) << setw(14) << "sg" << setfill(' ') << setw(4) << (ss*8+4) << setw(14) << "sg" << setfill(' ') << setw(4) << (ss*8+5) << setw(14) << "sg" << setfill(' ') << setw(4) << (ss*8+6) << setw(14) << "sg" << setfill(' ') << setw(4) << (ss*8+7) << "\n");
   }
  }

   if (non_zero_bcnt == 0) { 
      TXS_PRN_MSG(setw(0) << tmr_name << " TMR PENDING  tick0 - tick4095 bcnt:0" << "\n\n");
   } else {
      TXS_PRN_MSG(setw(0) << tmr_name << " TMR PENDING  total bcnt>0 " << non_zero_bcnt << "\n\n");
   }
   if (non_zero_lcnt == 0) { 
      TXS_PRN_MSG(setw(0) << tmr_name << " TMR PENDING  tick0 - tick4095 lcnt:0" << "\n\n");
   } else {
      TXS_PRN_MSG(setw(0) << tmr_name << " TMR PENDING  total lcnt>0 " << non_zero_lcnt << "\n\n");
   }
}

void cap_tmr_status(int chip_id, int inst_id) {

    cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);

    uint64_t tick, ctime, ptime;
    stringstream sdata;

    TXS_PRN_MSG("========================================================= tmr status ==============================================================\n");
    txs_csr.sta_slow_timer.read();
    tick  =  txs_csr.sta_slow_timer.tick().convert_to<uint64_t>();
    ctime =  txs_csr.sta_slow_timer.cTime().convert_to<uint64_t>();
    ptime =  txs_csr.sta_slow_timer.pTime().convert_to<uint64_t>();
    TXS_PRN_MSG(setw(32) << "slow tmr" << setw(15) << "tick:" << tick << setfill(' ') << setw(15) << "ctime:0x" << hex << ctime << setfill(' ') << setw(15) << "ptime:0x" << ptime << dec << "\n");

    txs_csr.sta_fast_timer.read();
    tick  =  txs_csr.sta_fast_timer.tick().convert_to<uint64_t>();
    ctime =  txs_csr.sta_fast_timer.cTime().convert_to<uint64_t>();
    ptime =  txs_csr.sta_fast_timer.pTime().convert_to<uint64_t>();
    TXS_PRN_MSG(setw(32) << "fast tmr" << setw(15) << "tick:" << tick << setfill(' ') << setw(15) << "ctime:0x" << hex << ctime << setfill(' ') << setw(15) << "ptime:0x" << ptime << dec << "\n");

    txs_csr.sta_timer_dbg2.read();
    cpp_int tmrWrPending = txs_csr.sta_timer_dbg2.hbm_wr_pending_cnt();
    cpp_int tmrRdPending = txs_csr.sta_timer_dbg2.hbm_rd_pending_cnt();
    cpp_int tmrBypPending = txs_csr.sta_timer_dbg2.hbm_byp_pending_cnt();
    cpp_int ftmrStall   = txs_csr.sta_timer_dbg2.ftmr_stall();
    cpp_int stmrStall   = txs_csr.sta_timer_dbg2.stmr_stall();

    TXS_PRN_MSG(setw(32) << "hbm_wr_pending_cnt" << setw(15) << tmrWrPending << dec << "\n");
    TXS_PRN_MSG(setw(32) << "hbm_rd_pending_cnt" << setw(15) << tmrRdPending << dec << "\n");
    TXS_PRN_MSG(setw(32) << "hbm_byp_pending_cnt" << setw(15) << tmrBypPending << dec << "\n");
    TXS_PRN_MSG(setw(32) << "ftmr_stall" << setw(15) << ftmrStall << dec << "\n");
    TXS_PRN_MSG(setw(32) << "stmr_stall" << setw(15) << stmrStall << dec << "\n");

    TXS_PRN_MSG("-----------------------------------------------------------------------------------------------------------------------------------\n");

}

void cap_txs_sch_status(int chip_id, int inst_id) {

    cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);

    stringstream sdata;

    TXS_PRN_MSG("========================================================= sch status ==============================================================\n");
    txs_csr.sta_scheduler_dbg2.read();

    cpp_int tmrWrPending = txs_csr.sta_scheduler_dbg2.hbm_wr_pending_cnt();
    cpp_int tmrRdPending = txs_csr.sta_scheduler_dbg2.hbm_rd_pending_cnt();
    cpp_int tmrBypPending = txs_csr.sta_scheduler_dbg2.hbm_byp_pending_cnt();

    TXS_PRN_MSG(setw(32) << "hbm_wr_pending_cnt" << setw(15) << tmrWrPending << dec << "\n");
    TXS_PRN_MSG(setw(32) << "hbm_rd_pending_cnt" << setw(15) << tmrRdPending << dec << "\n");
    TXS_PRN_MSG(setw(32) << "hbm_byp_pending_cnt" << setw(15) << tmrBypPending << dec << "\n");

    TXS_PRN_MSG("-----------------------------------------------------------------------------------------------------------------------------------\n");

}

// 1: active & not rlid stop
// 0: non active or rlid stop
void cap_txs_sg_active_status(int chip_id, int inst_id, int active_txdma_only) {

    cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);
    int num_active_sg = 0;

    TXS_PRN_MSG("========================================================= active sg to txdma/cfg sg_active ==============================================================\n");
    TXS_PRN_MSG("                                     1: (active sg & rlid stop), 0: (nonactive sg or rlid stop)\n");
    for (int i=0; i<64; i++) {
       txs_csr.dhs_sch_flags.entry[i].read();
       cpp_int active_sg = txs_csr.dhs_sch_flags.entry[i].sch_flags();
       if (active_sg != 0) {
          TXS_PRN_MSG("========== sg   ");
          for (int j=0; j<32; j++) {
             stringstream sdata;
             sdata << setw(1) << "  " << ((31-j) + i*32);
             TXS_PRN_MSG(setw(5) << sdata.str());
          }
          TXS_PRN_MSG("\n");
          TXS_PRN_MSG("active_sg:" << setw(3) << i*32 << "-" << setw(3) << i*32+31);
          for (int j=0; j<32; j++) {
             stringstream sdata;
             txs_csr.dhs_sch_lif_map_sram.entry[i*32 + 31-j].read();
             cpp_int cfg_sg_active = txs_csr.dhs_sch_lif_map_sram.entry[i*32 + 31-j].sg_active();
             if ( ((active_sg>>(31-j))&0x1) == 0) {
                if (active_txdma_only) {
                   sdata << setw(1) << "-" << "  ";
                } else {
                   if (cfg_sg_active == 0) {
                      sdata << setw(1) << "-" << "/" << "-";
                   } else {
                      sdata << setw(1) << "-" << "/" << cfg_sg_active;
                   }
                }
             } else {
                num_active_sg++;
                if (active_txdma_only) {
                   sdata << setw(1) << ((active_sg>>(31-j))&0x1) << "  ";
                } else {
                   sdata << setw(1) << ((active_sg>>(31-j))&0x1) << "/" << cfg_sg_active;
                }
             }
             TXS_PRN_MSG(setw(5) << sdata.str());
          }
          TXS_PRN_MSG("  active_sg:0x" << hex << active_sg << dec);
          TXS_PRN_MSG("\n");
       }
    }
    TXS_PRN_MSG("     total active sch group (send to txdma):" << num_active_sg << "\n");
    TXS_PRN_MSG("---------------------------------------------------------------------------------------------------------------------------------------------------------\n");
}

void cap_sch_cfg(int chip_id, int inst_id) {
    cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);

    uint64_t hbm_base, sch_grp_depth; 
    uint64_t enable, enable_set_lkup, enable_set_byp; 
    uint64_t hbm_hw_init, sram_hw_init;

    txs_csr.cfw_scheduler_static.read();
    hbm_base      =  txs_csr.cfw_scheduler_static.hbm_base().convert_to<uint64_t>();
    sch_grp_depth =  txs_csr.cfw_scheduler_static.sch_grp_depth().convert_to<uint64_t>();

    txs_csr.cfw_scheduler_glb.read();
    enable          =  txs_csr.cfw_scheduler_glb.enable().convert_to<uint64_t>();
    hbm_hw_init     =  txs_csr.cfw_scheduler_glb.hbm_hw_init().convert_to<uint64_t>();
    sram_hw_init    =  txs_csr.cfw_scheduler_glb.sram_hw_init().convert_to<uint64_t>();
    enable_set_lkup =  txs_csr.cfw_scheduler_glb.enable_set_lkup().convert_to<uint64_t>();
    enable_set_byp  =  txs_csr.cfw_scheduler_glb.enable_set_byp ().convert_to<uint64_t>();

    TXS_PRN_MSG("========================================================= sch cfg (cfw_scheduler_static, cfw_scheduler_glb) =======================\n");
    TXS_PRN_MSG(setw(32) << "hbm_base" << setw(10) << hex << "0x" << hbm_base << dec << "\n");
    TXS_PRN_MSG(setw(32) << "sch_grp_depth" << setw(10) << hex << "0x" << sch_grp_depth << dec << "\n");
    TXS_PRN_MSG(setw(32) << "enable" << setw(10) << hex << "0x" << enable << dec << "\n");
    TXS_PRN_MSG(setw(32) << "hbm_hw_init" << setw(10) << hex << "0x" << hbm_hw_init << dec << "\n");
    TXS_PRN_MSG(setw(32) << "sram_hw_init" << setw(10) << hex << "0x" << sram_hw_init << dec << "\n");
    TXS_PRN_MSG(setw(32) << "enable_set_lkup" << setw(10) << hex << "0x" << enable_set_lkup << dec << "\n");
    TXS_PRN_MSG(setw(32) << "enable_set_byp" << setw(10) << hex << "0x" << enable_set_byp << dec << "\n");
}

void cap_tmr_cfg(int chip_id, int inst_id) {
    cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);

    uint64_t hbm_base; 
    uint64_t tmr_hsh_depth, tmr_wheel_depth; 
    uint64_t hbm_hw_init, sram_hw_init;
    uint64_t ftmr_enable, stmr_enable, ftmr_pause, stmr_pause;
    uint64_t max_bcnt, spare;
    uint64_t slow_tmr_tick;
    //uint64_t slow_hash_sel;
    uint64_t fast_tmr_tick;
    //uint64_t fast_hash_sel;

    txs_csr.cfg_timer_static.read();
    hbm_base        =  txs_csr.cfg_timer_static.hbm_base().convert_to<uint64_t>();
    tmr_hsh_depth   =  txs_csr.cfg_timer_static.tmr_hsh_depth().convert_to<uint64_t>();
    tmr_wheel_depth =  txs_csr.cfg_timer_static.tmr_wheel_depth().convert_to<uint64_t>();

    txs_csr.cfw_timer_glb.read();
    hbm_hw_init     =  txs_csr.cfw_timer_glb.hbm_hw_init().convert_to<uint64_t>();
    sram_hw_init    =  txs_csr.cfw_timer_glb.sram_hw_init().convert_to<uint64_t>();
    ftmr_enable     =  txs_csr.cfw_timer_glb.ftmr_enable().convert_to<uint64_t>();
    stmr_enable     =  txs_csr.cfw_timer_glb.stmr_enable().convert_to<uint64_t>();
    ftmr_pause      =  txs_csr.cfw_timer_glb.ftmr_pause().convert_to<uint64_t>();
    stmr_pause      =  txs_csr.cfw_timer_glb.stmr_pause().convert_to<uint64_t>();

    txs_csr.cfg_timer_dbg2.read();
    max_bcnt      =  txs_csr.cfg_timer_dbg2.max_bcnt().convert_to<uint64_t>();
    spare         =  txs_csr.cfg_timer_dbg2.spare().convert_to<uint64_t>();

    txs_csr.cfg_fast_timer.read();
    fast_tmr_tick      =  txs_csr.cfg_fast_timer.tick().convert_to<uint64_t>();
    //fast_hash_sel      =  txs_csr.cfg_fast_timer.hash_sel().convert_to<uint64_t>();

    txs_csr.cfg_slow_timer.read();
    slow_tmr_tick      =  txs_csr.cfg_slow_timer.tick().convert_to<uint64_t>();
    //slow_hash_sel      =  txs_csr.cfg_slow_timer.hash_sel().convert_to<uint64_t>();

    TXS_PRN_MSG("========================================================= sch cfg ===========================================================\n");
    TXS_PRN_MSG("--------------------------------------------------------- cfg_timer_static --------------------------------------------------\n");
    TXS_PRN_MSG(setw(32) << "hbm_base" << setw(10) << hex << "0x" << hbm_base << dec << "\n");
    TXS_PRN_MSG(setw(32) << "tmr_hsh_depth" << setw(10) << hex << "0x" << tmr_hsh_depth << dec << "\n");
    TXS_PRN_MSG(setw(32) << "tmr_wheel_depth" << setw(10) << hex << "0x" << tmr_wheel_depth << dec << "\n");
    TXS_PRN_MSG("--------------------------------------------------------- cfw_timer_glb -----------------------------------------------------\n");
    TXS_PRN_MSG(setw(32) << "hbm_hw_init" << setw(10) << hex << "0x" << hbm_hw_init << dec << "\n");
    TXS_PRN_MSG(setw(32) << "sram_hw_init" << setw(10) << hex << "0x" << sram_hw_init << dec << "\n");
    TXS_PRN_MSG(setw(32) << "ftmr_enable" << setw(10) << hex << "0x" << ftmr_enable << dec << "\n");
    TXS_PRN_MSG(setw(32) << "stmr_enable" << setw(10) << hex << "0x" << stmr_enable << dec << "\n");
    TXS_PRN_MSG(setw(32) << "ftmr_pause" << setw(10) << hex << "0x" << ftmr_pause << dec << "\n");
    TXS_PRN_MSG(setw(32) << "stmr_pause" << setw(10) << hex << "0x" << stmr_pause << dec << "\n");
    TXS_PRN_MSG("--------------------------------------------------------- cfg_timer_dbg2 ----------------------------------------------------\n");
    TXS_PRN_MSG(setw(32) << "max_bcnt" << setw(10) << hex << "0x" << max_bcnt << dec << "\n");
    TXS_PRN_MSG(setw(32) << "spare (flow_ctrl)" << setw(10) << hex << "0x" << spare << dec << "\n");
    TXS_PRN_MSG("--------------------------------------------------------- cfg_fast_timer ----------------------------------------------------\n");
    TXS_PRN_MSG(setw(32) << "fast_tmr_tick" << setw(10) << hex << "0x" << fast_tmr_tick << dec << "\n");
    //TXS_PRN_MSG(setw(32) << "fast_hash_sel" << setw(10) << hex << "0x" << fast_hash_sel << dec << "\n");
    TXS_PRN_MSG("--------------------------------------------------------- cfg_slow_timer ----------------------------------------------------\n");
    TXS_PRN_MSG(setw(32) << "slow_tmr_tick" << setw(10) << hex << "0x" << slow_tmr_tick << dec << "\n");
    //TXS_PRN_MSG(setw(32) << "slow_hash_sel" << setw(10) << hex << "0x" << slow_hash_sel << dec << "\n");
}

void cap_txs_cfg_dtdm(int chip_id, int inst_id) {

    cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);

    uint64_t wt_lo[16];
    uint64_t wt_hi[16];
    uint64_t idx;

    uint64_t dtdm_lo_map;
    uint64_t dtdm_hi_map;
    uint64_t pause;
    uint64_t timeout;
    uint64_t enable;
    float    dtdm_ratio;

    stringstream sdata;
    stringstream dtdm0;
    stringstream rdtdm0;
    stringstream dtdm1;
    stringstream rdtdm1;
    stringstream dtdm2;
    stringstream rdtdm2;
    stringstream dtdm3;
    stringstream rdtdm3;
    stringstream dtdm4;
    stringstream rdtdm4;
    stringstream dtdm5;
    stringstream rdtdm5;
    stringstream dtdm6;
    stringstream rdtdm6;
    stringstream dtdm7;
    stringstream rdtdm7;
    for (int i=0; i<16; i++) {
       wt_lo[i] = 0;
       wt_hi[i] = 0;
    }

    for (uint64_t i=0; i<64; i++) {
       txs_csr.dhs_dtdmlo_calendar.entry[i].read();
       idx = txs_csr.dhs_dtdmlo_calendar.entry[i].dtdm_calendar().convert_to<uint64_t>();
       wt_lo[idx]++;

       txs_csr.dhs_dtdmhi_calendar.entry[i].read();
       idx = txs_csr.dhs_dtdmhi_calendar.entry[i].dtdm_calendar().convert_to<uint64_t>();
       wt_hi[idx]++;
    }

    txs_csr.cfg_sch.read();
    dtdm_lo_map   =  txs_csr.cfg_sch.dtdm_lo_map().convert_to<uint64_t>();
    dtdm_hi_map   =  txs_csr.cfg_sch.dtdm_hi_map().convert_to<uint64_t>();
    pause         =  txs_csr.cfg_sch.pause().convert_to<uint64_t>();
    timeout       =  txs_csr.cfg_sch.timeout().convert_to<uint64_t>();
    enable        =  txs_csr.cfg_sch.enable().convert_to<uint64_t>();

    if ((dtdm_lo_map & dtdm_hi_map) > 0) {
       TXS_PRN_MSG(setw(32) << "BAD dtdm map cfg (dtdm_lo_map & dtdm_hi_map) != 0" << setw(10) << hex << "0x" << (dtdm_lo_map & dtdm_hi_map) << dec << "\n");
    }

    TXS_PRN_MSG("========================================================= DTDM cfg ===========================================================\n");
    TXS_PRN_MSG(setw(32) << "dtdm_lo_map" << setw(10) << hex << "0x" << dtdm_lo_map << dec << "\n");
    TXS_PRN_MSG(setw(32) << "dtdm_hi_map" << setw(10) << hex << "0x" << dtdm_hi_map << dec << "\n");
    TXS_PRN_MSG(setw(32) << "timeout" << setw(10) << hex << "0x" << timeout << dec << "\n");
    TXS_PRN_MSG(setw(32) << "pause" << setw(10) << hex << "0x" << pause << dec << "\n");
    TXS_PRN_MSG(setw(32) << "enable" << setw(10) << hex << "0x" << enable << dec << "\n");

    TXS_PRN_MSG(setw(125) << "====================================================== DTDM calendar (wt) =========================================================\n");
   TXS_PRN_MSG(setw(32) << "cos" << 0 << setw(11) << "cos" << 1 << setw(11) << "cos" << 2 << setw(11) << "cos" << 3 << setw(11) << "cos" << 4 << setw(11) << "cos" << 5 << setw(11) << "cos" << 6 << setw(11) << "cos" << 7 << "\n");
   TXS_PRN_MSG("-----------------------------------------------------------------------------------------------------------------------------------\n");
     dtdm_ratio = (float) wt_lo[0] / 64.0;
     dtdm0.str("");
     rdtdm0.str("");
     if (wt_lo[0] == 0) {
        dtdm0 << "-";
        rdtdm0 << "-";
     } else {
        dtdm0 << wt_lo[0];
        rdtdm0 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_lo[1] / 64.0;
     dtdm1.str("");
     rdtdm1.str("");
     if (wt_lo[1] == 0) {
        dtdm1 << "-";
        rdtdm1 << "-";
     } else {
        dtdm1 << wt_lo[1];
        rdtdm1 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_lo[2] / 64.0;
     dtdm2.str("");
     rdtdm2.str("");
     if (wt_lo[2] == 0) {
        dtdm2 << "-";
        rdtdm2 << "-";
     } else {
        dtdm2 << wt_lo[2];
        rdtdm2 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_lo[3] / 64.0;
     dtdm3.str("");
     rdtdm3.str("");
     if (wt_lo[3] == 0) {
        dtdm3 << "-";
        rdtdm3 << "-";
     } else {
        dtdm3 << wt_lo[3];
        rdtdm3 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_lo[4] / 64.0;
     dtdm4.str("");
     rdtdm4.str("");
     if (wt_lo[4] == 0) {
        dtdm4 << "-";
        rdtdm4 << "-";
     } else {
        dtdm4 << wt_lo[4];
        rdtdm4 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_lo[5] / 64.0;
     dtdm5.str("");
     rdtdm5.str("");
     if (wt_lo[5] == 0) {
        dtdm5 << "-";
        rdtdm5 << "-";
     } else {
        dtdm5 << wt_lo[5];
        rdtdm5 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_lo[6] / 64.0;
     dtdm6.str("");
     rdtdm6.str("");
     if (wt_lo[6] == 0) {
        dtdm6 << "-";
        rdtdm6 << "-";
     } else {
        dtdm6 << wt_lo[6];
        rdtdm6 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_lo[7] / 64.0;
     dtdm7.str("");
     rdtdm7.str("");
     if (wt_lo[7] == 0) {
        dtdm7 << "-";
        rdtdm7 << "-";
     } else {
        dtdm7 << wt_lo[7];
        rdtdm7 << std::setprecision(2) << dtdm_ratio;
     }
     sdata.str("");
     sdata << setw(12) << dtdm0.str() << setw(12) << dtdm1.str() << setw(12) << dtdm2.str() << setw(12) << dtdm3.str() << setw(12) << dtdm4.str() << setw(12) << dtdm5.str() << setw(12) << dtdm6.str() << setw(12) << dtdm7.str();
   TXS_PRN_MSG(setw(20) << "dtdmlo " << sdata.str() << "\n");
     sdata.str("");
     sdata << setw(12) << rdtdm0.str() << setw(12) << rdtdm1.str() << setw(12) << rdtdm2.str() << setw(12) << rdtdm3.str() << setw(12) << rdtdm4.str() << setw(12) << rdtdm5.str() << setw(12) << rdtdm6.str() << setw(12) << rdtdm7.str();
   TXS_PRN_MSG(setw(20) << "dtdmlo " << sdata.str() << "\n");
   TXS_PRN_MSG("-----------------------------------------------------------------------------------------------------------------------------------\n");

     dtdm_ratio = (float) wt_hi[0] / 64.0;
     dtdm0.str("");
     rdtdm0.str("");
     if (wt_hi[0] == 0) {
        dtdm0 << "-";
        rdtdm0 << "-";
     } else {
        dtdm0  << wt_hi[0];
        rdtdm0 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_hi[1] / 64.0;
     dtdm1.str("");
     rdtdm1.str("");
     if (wt_hi[1] == 0) {
        dtdm1 << "-";
        rdtdm1 << "-";
     } else {
        dtdm1  << wt_hi[1];
        rdtdm1 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_hi[2] / 64.0;
     dtdm2.str("");
     rdtdm2.str("");
     if (wt_hi[2] == 0) {
        dtdm2 << "-";
        rdtdm2 << "-";
     } else {
        dtdm2  << wt_hi[2];
        rdtdm2 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_hi[3] / 64.0;
     dtdm3.str("");
     rdtdm3.str("");
     if (wt_hi[3] == 0) {
        dtdm3 << "-";
        rdtdm3 << "-";
     } else {
        dtdm3  << wt_hi[3];
        rdtdm3 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_hi[4] / 64.0;
     dtdm4.str("");
     rdtdm4.str("");
     if (wt_hi[4] == 0) {
        dtdm4 << "-";
        rdtdm4 << "-";
     } else {
        dtdm4  << wt_hi[4];
        rdtdm4 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_hi[5] / 64.0;
     dtdm5.str("");
     rdtdm5.str("");
     if (wt_hi[5] == 0) {
        dtdm5 << "-";
        rdtdm5 << "-";
     } else {
        dtdm5  << wt_hi[5];
        rdtdm5 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_hi[6] / 64.0;
     dtdm6.str("");
     rdtdm6.str("");
     if (wt_hi[6] == 0) {
        dtdm6 << "-";
        rdtdm6 << "-";
     } else {
        dtdm6  << wt_hi[6];
        rdtdm6 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_hi[7] / 64.0;
     dtdm7.str("");
     rdtdm7.str("");
     if (wt_hi[7] == 0) {
        dtdm7 << "-";
        rdtdm7 << "-";
     } else {
        dtdm7  << wt_hi[7];
        rdtdm7 << std::setprecision(2) << dtdm_ratio;
     }
     sdata.str("");
     sdata << setw(12) << dtdm0.str() << setw(12) << dtdm1.str() << setw(12) << dtdm2.str() << setw(12) << dtdm3.str() << setw(12) << dtdm4.str() << setw(12) << dtdm5.str() << setw(12) << dtdm6.str() << setw(12) << dtdm7.str();
   TXS_PRN_MSG(setw(20) << "dtdmhi " << sdata.str() << "\n");
     sdata.str("");
     sdata << setw(12) << rdtdm0.str() << setw(12) << rdtdm1.str() << setw(12) << rdtdm2.str() << setw(12) << rdtdm3.str() << setw(12) << rdtdm4.str() << setw(12) << rdtdm5.str() << setw(12) << rdtdm6.str() << setw(12) << rdtdm7.str();
   TXS_PRN_MSG(setw(20) << "dtdmhi " << sdata.str() << "\n");
   TXS_PRN_MSG("-----------------------------------------------------------------------------------------------------------------------------------\n");
   TXS_PRN_MSG(setw(32) << "cos" << 8 << setw(11) << "cos" << 9 << setw(11) << "cos" << 10 << setw(10) << "cos" << 11 << setw(10) << "cos" << 12 << setw(10) << "cos" << 13 << setw(10) << "cos" << 14 << setw(10) << "cos" << 15 << "\n");
   TXS_PRN_MSG("-----------------------------------------------------------------------------------------------------------------------------------\n");
     dtdm_ratio = (float) wt_lo[8] / 64.0;
     dtdm0.str("");
     rdtdm0.str("");
     if (wt_lo[8] == 0) {
        dtdm0 << "-";
        rdtdm0 << "-";
     } else {
        dtdm0 << wt_lo[8];
        rdtdm0 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_lo[9] / 64.0;
     dtdm1.str("");
     rdtdm1.str("");
     if (wt_lo[9] == 0) {
        dtdm1 << "-";
        rdtdm1 << "-";
     } else {
        dtdm1 << wt_lo[9];
        rdtdm1 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_lo[10] / 64.0;
     dtdm2.str("");
     rdtdm2.str("");
     if (wt_lo[10] == 0) {
        dtdm2 << "-";
        rdtdm2 << "-";
     } else {
        dtdm2 << wt_lo[10];
        rdtdm2 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_lo[11] / 64.0;
     dtdm3.str("");
     rdtdm3.str("");
     if (wt_lo[11] == 0) {
        dtdm3 << "-";
        rdtdm3 << "-";
     } else {
        dtdm3 << wt_lo[11];
        rdtdm3 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_lo[12] / 64.0;
     dtdm4.str("");
     rdtdm4.str("");
     if (wt_lo[12] == 0) {
        dtdm4 << "-";
        rdtdm4 << "-";
     } else {
        dtdm4 << wt_lo[12];
        rdtdm4 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_lo[13] / 64.0;
     dtdm5.str("");
     rdtdm5.str("");
     if (wt_lo[13] == 0) {
        dtdm5 << "-";
        rdtdm5 << "-";
     } else {
        dtdm5 << wt_lo[13];
        rdtdm5 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_lo[14] / 64.0;
     dtdm6.str("");
     rdtdm6.str("");
     if (wt_lo[14] == 0) {
        dtdm6 << "-";
        rdtdm6 << "-";
     } else {
        dtdm6 << wt_lo[14];
        rdtdm6 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_lo[15] / 64.0;
     dtdm7.str("");
     rdtdm7.str("");
     if (wt_lo[15] == 0) {
        dtdm7 << "-";
        rdtdm7 << "-";
     } else {
        dtdm7 << wt_lo[15];
        rdtdm7 << std::setprecision(2) << dtdm_ratio;
     }
     sdata.str("");
     sdata << setw(12) << dtdm0.str() << setw(12) << dtdm1.str() << setw(12) << dtdm2.str() << setw(12) << dtdm3.str() << setw(12) << dtdm4.str() << setw(12) << dtdm5.str() << setw(12) << dtdm6.str() << setw(12) << dtdm7.str();
   TXS_PRN_MSG(setw(20) << "dtdmlo " << sdata.str() << "\n");
     sdata.str("");
     sdata << setw(12) << rdtdm0.str() << setw(12) << rdtdm1.str() << setw(12) << rdtdm2.str() << setw(12) << rdtdm3.str() << setw(12) << rdtdm4.str() << setw(12) << rdtdm5.str() << setw(12) << rdtdm6.str() << setw(12) << rdtdm7.str();
   TXS_PRN_MSG(setw(20) << "dtdmlo " << sdata.str() << "\n");
   TXS_PRN_MSG("-----------------------------------------------------------------------------------------------------------------------------------\n");

     dtdm_ratio = (float) wt_hi[8] / 64.0;
     dtdm0.str("");
     rdtdm0.str("");
     if (wt_hi[8] == 0) {
        dtdm0 << "-";
        rdtdm0 << "-";
     } else {
        dtdm0  << wt_hi[8];
        rdtdm0 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_hi[9] / 64.0;
     dtdm1.str("");
     rdtdm1.str("");
     if (wt_hi[9] == 0) {
        dtdm1 << "-";
        rdtdm1 << "-";
     } else {
        dtdm1  << wt_hi[9];
        rdtdm1 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_hi[10] / 64.0;
     dtdm2.str("");
     rdtdm2.str("");
     if (wt_hi[10] == 0) {
        dtdm2 << "-";
        rdtdm2 << "-";
     } else {
        dtdm2  << wt_hi[10];
        rdtdm2 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_hi[11] / 64.0;
     dtdm3.str("");
     rdtdm3.str("");
     if (wt_hi[11] == 0) {
        dtdm3 << "-";
        rdtdm3 << "-";
     } else {
        dtdm3  << wt_hi[11];
        rdtdm3 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_hi[12] / 64.0;
     dtdm4.str("");
     rdtdm4.str("");
     if (wt_hi[12] == 0) {
        dtdm4 << "-";
        rdtdm4 << "-";
     } else {
        dtdm4  << wt_hi[12];
        rdtdm4 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_hi[13] / 64.0;
     dtdm5.str("");
     rdtdm5.str("");
     if (wt_hi[13] == 0) {
        dtdm5 << "-";
        rdtdm5 << "-";
     } else {
        dtdm5  << wt_hi[13];
        rdtdm5 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_hi[14] / 64.0;
     dtdm6.str("");
     rdtdm6.str("");
     if (wt_hi[14] == 0) {
        dtdm6 << "-";
        rdtdm6 << "-";
     } else {
        dtdm6  << wt_hi[14];
        rdtdm6 << std::setprecision(2) << dtdm_ratio;
     }
     dtdm_ratio = (float) wt_hi[15] / 64.0;
     dtdm7.str("");
     rdtdm7.str("");
     if (wt_hi[15] == 0) {
        dtdm7 << "-";
        rdtdm7 << "-";
     } else {
        dtdm7  << wt_hi[15];
        rdtdm7 << std::setprecision(2) << dtdm_ratio;
     }
     sdata.str("");
     sdata << setw(12) << dtdm0.str() << setw(12) << dtdm1.str() << setw(12) << dtdm2.str() << setw(12) << dtdm3.str() << setw(12) << dtdm4.str() << setw(12) << dtdm5.str() << setw(12) << dtdm6.str() << setw(12) << dtdm7.str();
   TXS_PRN_MSG(setw(20) << "dtdmhi " << sdata.str() << "\n");
     sdata.str("");
     sdata << setw(12) << rdtdm0.str() << setw(12) << rdtdm1.str() << setw(12) << rdtdm2.str() << setw(12) << rdtdm3.str() << setw(12) << rdtdm4.str() << setw(12) << rdtdm5.str() << setw(12) << rdtdm6.str() << setw(12) << rdtdm7.str();
   TXS_PRN_MSG(setw(20) << "dtdmhi " << sdata.str() << "\n");
   TXS_PRN_MSG("-----------------------------------------------------------------------------------------------------------------------------------\n");
}

// read active queues in sg
// 2k sch_grp
// 8k queues (512b * 16)
void cap_txs_get_all_active_q(int chip_id, int inst_id, int sample) {
   cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);

   //uint64_t hbm_base;
   uint64_t sch_grp_depth;

   txs_csr.cfw_scheduler_static.read();
   //hbm_base      =  txs_csr.cfw_scheduler_static.hbm_base().convert_to<uint64_t>();
   sch_grp_depth =  txs_csr.cfw_scheduler_static.sch_grp_depth().convert_to<uint64_t>();

   for (unsigned i=0; i<sch_grp_depth; i++) {
      cap_txs_get_active_q(chip_id, inst_id, i, sample);
   }
}

void cap_txs_get_active_q(int chip_id, int inst_id, int sch_grp, int sample) {
   cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);

   map<uint32_t, uint32_t> active_q_map;

   uint64_t hbm_base, sch_grp_depth;
   int      found = 0;

   txs_csr.cfw_scheduler_static.read();
   hbm_base      =  txs_csr.cfw_scheduler_static.hbm_base().convert_to<uint64_t>();
   sch_grp_depth =  txs_csr.cfw_scheduler_static.sch_grp_depth().convert_to<uint64_t>();

   //TXS_PRN_MSG(setw(32) << "active queue for sg:" << sch_grp << " (0x" << hex << sch_grp << ")" << dec << "\n");

   if (sch_grp <  (int)sch_grp_depth) {
     unsigned char *data;
     data = new unsigned char[SCHGRP_SIZE_BYTE];

     for (int s=0; s<sample; s++) {
        cap_mem_access::instancep()->read(uint64_t (hbm_base + sch_grp*SCHGRP_SIZE_BYTE), data, SCHGRP_SIZE_BYTE);
        for (int i=0; i<SCHGRP_SIZE_BYTE; i++) {
           if ((data[i]&0xff) != 0) {
              found = 1;
              for (int qq=0; qq<8; qq++) {
                 if ((data[i]>>qq)&0x1) {
                    int active_q = i*8 + qq;
                    active_q_map[active_q] = 1;
                 }
              } 
           }
        }
     }
     delete [] data;
   } else {
      TXS_PRN_MSG("ERROR sch_grp:" << sch_grp << " >= sch_grp_depth:" << sch_grp_depth << "\n");
   }

   if (found == 0) {
      TXS_PRN_MSG(setw(32) << "sg:" << sch_grp << " (0x" << hex << sch_grp << dec << ") no active q found\n"); 
   } else {
      for(auto i : active_q_map) {
         if (active_q_map[i.first] != 0) {
            TXS_PRN_MSG(setw(32) << "sg:" << sch_grp << " (0x" << hex << sch_grp << dec << ") active q:" << i.first << " (0x" << hex << i.first << dec << ") \n"); 
         }
      }
   }
}

void cap_lif_sch_cfg_chk(int chip_id, int inst_id, int sg) {
   cap_txs_csr_t & txs_csr = CAP_BLK_REG_MODEL_ACCESS(cap_txs_csr_t, chip_id, inst_id);
   
   int lif      =  txs_csr.dhs_sch_grp_entry.entry[sg].lif().convert_to<uint32_t>();
   //int cos      =  txs_csr.dhs_sch_grp_entry.entry[sg].rr_sel().convert_to<uint32_t>(); // rr_sel is the cos in txdma

   int sg_active  =  txs_csr.dhs_sch_lif_map_sram.entry[lif].sg_active().convert_to<uint32_t>();
   int sg_start   =  txs_csr.dhs_sch_lif_map_sram.entry[lif].sg_start().convert_to<uint32_t>();
   int sg_per_cos =  txs_csr.dhs_sch_lif_map_sram.entry[lif].sg_per_cos().convert_to<uint32_t>();
   int sg_act_cos =  txs_csr.dhs_sch_lif_map_sram.entry[lif].sg_act_cos().convert_to<uint32_t>();
   // txs_csr.dhs_sch_lif_map_sram.entry[lif].show();

   int num_sch_in_lif;
   int sg_act_cos_cnt;

   int num_act_cos = 0;
   for (int i=0; i<16; i++) {
      if ((sg_act_cos>>i)&0x1) {
         num_act_cos++;
      }
   }
   num_sch_in_lif = sg_per_cos * num_act_cos;
   sg_act_cos_cnt = 0;
   
   TXS_PRN_MSG(setw(8) << "=== sg:" << sg << " :: all sg/lif binding for lif:" << lif << " sg_active:" << sg_active << "\n");
   for (int i=0; i<num_sch_in_lif; i++) {
      int mylif        =  txs_csr.dhs_sch_grp_entry.entry[sg_start+i].lif().convert_to<uint32_t>();
      int myrr_sel     =  txs_csr.dhs_sch_grp_entry.entry[sg_start+i].rr_sel().convert_to<uint32_t>(); // rr_sel is the cos in txdma
      int myqid_offset =  txs_csr.dhs_sch_grp_entry.entry[sg_start+i].qid_offset().convert_to<uint32_t>(); // rr_sel is the cos in txdma
      // txs_csr.dhs_sch_grp_entry.entry[sg_start+i].show();
      if (mylif != lif) {
         TXS_PRN_MSG(setw(10) << "ERROR bad cfg dhs_sch_grp_entry sg: " << (sg_start+i)  << " lif:" << mylif << " should be the same lif:" << lif << "\n");
      } else {
         int mycos    = cap_txs_get_cos(chip_id, inst_id, lif, sg_act_cos_cnt);
         if (mycos != myrr_sel) {
            TXS_PRN_MSG(setw(10) << "ERROR bad cfg lif:" << lif << " <---> sg: " << (sg_start+i) << " cos:" << mycos << " != rr_sel:" << myrr_sel << "\n");
         } else {
            TXS_PRN_MSG(setw(10) << "lif:" << lif << " <---> sg: " << (sg_start+i) << " cos:" << mycos << "\n");
         }
         if ((i%sg_per_cos) != myqid_offset) {
            TXS_PRN_MSG(setw(10) << "ERROR bad cfg lif:" << lif << " <---> sg: " << (sg_start+i) << " qid_offset:" << myqid_offset << " != :" << (i%sg_per_cos) << "\n");
         }
      }
      if ((i%sg_per_cos) == sg_per_cos-1) {
         sg_act_cos_cnt++;
      }
   }
}


#endif //  CAPRI_HAL

void txs_print_msg(string msg) {
#ifdef _CSV_INCLUDED_
   vpi_printf((PLI_BYTE8*) "%s", msg.c_str());
#else
   std::cout << msg;
#endif
}


