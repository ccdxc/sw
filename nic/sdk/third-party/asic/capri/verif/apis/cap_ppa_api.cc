#include "cap_ppa_api.h"
#include "msg_man.h"

void cap_ppa_soft_reset(int chip_id, int inst_id) {

PLOG_API_MSG("PPA", "inside softreset\n");

PLOG_API_MSG("PPA", "Dumping Sknob Vars\n");
//sknobs_dump();

 //cap_ppa_csr_t & ppa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_ppa_csr_t, chip_id, inst_id);

 cap_ppa_disable_backdoor_paths(chip_id, inst_id);

 PLOG_API_MSG("PPA", "done with softreset\n");
}

void cap_ppa_set_soft_reset(int chip_id, int inst_id, int value) { 
}

void cap_ppa_init_start(int chip_id, int inst_id) {

 cap_ppa_csr_t & ppa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_ppa_csr_t, chip_id, inst_id);

 PLOG_API_MSG("PPA", "inside init_start\n");

 //Disable lkp_sram ECC detection & correction if CPU backdoor access is enabled.
 //TBD_P2 : TODO_P2 : Review this if ECC can be written to SRAM during backdoor access
 string access_type_str = sknobs_get_string((char *)"cpu_access", (char *)"");
 if(!access_type_str.compare("back_door")) {
   ppa_csr.cfg_bndl0_state_lkp_sram.ecc_disable_det(1);
   ppa_csr.cfg_bndl0_state_lkp_sram.ecc_disable_cor(1);
   ppa_csr.cfg_bndl0_state_lkp_sram.dhs_ecc_bypass(1);
   ppa_csr.cfg_bndl0_state_lkp_sram.write();

   ppa_csr.cfg_bndl1_state_lkp_sram.ecc_disable_det(1);
   ppa_csr.cfg_bndl1_state_lkp_sram.ecc_disable_cor(1);
   ppa_csr.cfg_bndl1_state_lkp_sram.dhs_ecc_bypass(1);
   ppa_csr.cfg_bndl1_state_lkp_sram.write();

   ppa_csr.cfg_sw_phv_mem.ecc_detect(1);
   ppa_csr.cfg_sw_phv_mem.ecc_correct(1);
   ppa_csr.cfg_sw_phv_mem.write();
 }

#ifdef MODULE_SIM
  //Configure SRAM entry[state_lkp_catchall_entry] with action=1(stop) for default catch all entry.
  cap_ppa_decoders_ppa_lkp_sram_entry_t sram_entry;
  sram_entry.all((pu_cpp_int< 764 >) 0);
  sram_entry.action((pu_cpp_int< 1 >) 1); //action=stop
  ppa_csr.dhs_bndl0_state_lkp_sram.entry[ppa_csr.cfg_ctrl.state_lkp_catchall_entry().convert_to<int>()].data((pu_cpp_int< 764 >) sram_entry.all());
  ppa_csr.dhs_bndl0_state_lkp_sram.entry[ppa_csr.cfg_ctrl.state_lkp_catchall_entry().convert_to<int>()].write();
 
  ppa_csr.dhs_bndl1_state_lkp_sram.entry[ppa_csr.cfg_ctrl.state_lkp_catchall_entry().convert_to<int>()].data((pu_cpp_int< 764 >) sram_entry.all());
  ppa_csr.dhs_bndl1_state_lkp_sram.entry[ppa_csr.cfg_ctrl.state_lkp_catchall_entry().convert_to<int>()].write();
#endif
 
 //Flush all the writes.
 ppa_csr.cfg_ctrl.read();
 PLOG_API_MSG("PPA", "done with init_start \n");
}

void cap_ppa_init_done(int chip_id, int inst_id) {
}

void cap_ppa_load_from_cfg(int chip_id, int inst_id) {
 PLOG_API_MSG("PPA", "Before load_from_cfg\n");

 cap_ppa_csr_t & ppa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_ppa_csr_t, chip_id, inst_id);

 ppa_csr.load_from_cfg();

#ifdef MODULE_SIM
  //Configure SRAM entry[state_lkp_catchall_entry] with action=1(stop) for default catch all entry.
  cap_ppa_decoders_ppa_lkp_sram_entry_t sram_entry;
  sram_entry.all((pu_cpp_int< 764 >) 0);
  sram_entry.action((pu_cpp_int< 1 >) 1); //action=stop

  ppa_csr.dhs_bndl0_state_lkp_sram.entry[ppa_csr.cfg_ctrl.state_lkp_catchall_entry().convert_to<int>()].data((pu_cpp_int< 764 >) sram_entry.all());
  ppa_csr.dhs_bndl0_state_lkp_sram.entry[ppa_csr.cfg_ctrl.state_lkp_catchall_entry().convert_to<int>()].write();
 
  ppa_csr.dhs_bndl1_state_lkp_sram.entry[ppa_csr.cfg_ctrl.state_lkp_catchall_entry().convert_to<int>()].data((pu_cpp_int< 764 >) sram_entry.all());
  ppa_csr.dhs_bndl1_state_lkp_sram.entry[ppa_csr.cfg_ctrl.state_lkp_catchall_entry().convert_to<int>()].write(); 

  //Make sure vxlan_flag_val and vxlan_flag_mask are valid
  ppa_csr.cfg_preparse.vxlan_flag_val(ppa_csr.cfg_preparse.vxlan_flag_val() & ppa_csr.cfg_preparse.vxlan_flag_mask());
  ppa_csr.cfg_preparse.write();

#endif

 //Flush all the writes. Tcam always takes front door access, hence read one valid tcam entry to flush all front door accesses if we are in backdoor mode.
 if (cpu::access()->get_access_type() == back_door_e) {
   for(int idx = 0; idx < ppa_csr.dhs_bndl0_state_lkp_tcam.get_depth_entry(); idx++) {
    if(ppa_csr.dhs_bndl0_state_lkp_tcam.entry[idx].valid() == 1) {
       ppa_csr.dhs_bndl0_state_lkp_tcam.entry[idx].read();
       break;
    }
   }
 }

 //Flush all the writes.
 ppa_csr.cfg_ctrl.read();
 PLOG_API_MSG("PPA", "After load_from_cfg\n");
}


void cap_ppa_eos(int chip_id, int inst_id) {
   cap_ppa_eos_cnt(chip_id, inst_id);
   cap_ppa_eos_int(chip_id, inst_id);
   cap_ppa_eos_sta(chip_id, inst_id);
}

void cap_ppa_eos_cnt(int chip_id, int inst_id) {
//TBD :: Enable these in fullchip after they are debugged in block
#ifdef MODULE_SIM
 cap_ppa_csr_t & ppa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_ppa_csr_t, chip_id, inst_id);
 string                 csr_name;

 PLOG_API_MSG("PPA", "inside cap_ppa_eos_cnt \n");

 for(auto itr: ppa_csr.get_children_prefix("cnt_")) {
     csr_name = itr->get_name();
     //TBD_P2 :: Review this later.
     //CNT_ppa_pe<*>_pkt_in & CNT_ppa_pe<*> are informational counters for RTL debug purpose, 
     //thier timing is not deterministic, hence skipping them from comapre.
     if( (csr_name.find("CNT_ppa_pe") == string::npos)  || (csr_name.find("phv_out") != string::npos) ) {
        PLOG_API_MSG("PPA", "read_compare " << itr->get_name() << endl);
        itr->read_compare();
     }
 }
#endif

}

void cap_ppa_eos_int(int chip_id, int inst_id) {
//TBD :: Enable these in fullchip after they are debugged in block
#ifdef MODULE_SIM
 cap_ppa_csr_t & ppa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_ppa_csr_t, chip_id, inst_id);
 string    type_name;

 PLOG_API_MSG("PPA", "inside cap_ppa_eos_int \n");

 for(auto itr: ppa_csr.get_children_prefix("intreg")) {
     type_name = itr->get_hier_path();

     if(  (type_name.find("intreg_status_t") == string::npos)
       && (type_name.find("int_pa") == string::npos) 
       && (type_name.find("int_ecc") == string::npos) 
       && (type_name.find("int_sw_phv_mem") == string::npos) ) {
        //PLOG_API_MSG("PPA", "read_compare " << type_name << endl);
        if(type_name.find("int_bndl") == string::npos) {
           itr->read_compare();
        } else {
           //mask state_lkp_srch_req_not_rdy field before comparing. 
           //This is hardware timing specific int, hence can't be modeled.
           pu_cpp_int<32>  expd_val(itr->all() & 0xfffffffe);
           itr->read();
           pu_cpp_int<32>  rcvd_val(itr->all() & 0xfffffffe);
           if(expd_val != rcvd_val) {
             PLOG_ERR(" read_compare : " << type_name << " exp : 0x" << hex << expd_val << " actual : 0x" << rcvd_val << endl)
           }
        }
     }
 }
#endif

}

void cap_ppa_eos_sta(int chip_id, int inst_id) {
  cap_ppa_csr_t & ppa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_ppa_csr_t, chip_id, inst_id);
  PLOG_API_MSG("PPA", "inside cap_ppa_eos_sta \n");

  cpp_int my_val;
  cpp_int_helper hlp;

  ppa_csr.sta_fifo.read();
  //sta_fifo is 76bit value. HEnce 32-32-12
  my_val = hlp.set_slc(my_val ,0xAAAAAAAA , 0 , 31 );
  my_val = hlp.set_slc(my_val ,0xAAAAAAAA , 32, 63 );
  my_val = hlp.set_slc(my_val ,0xAAA      , 64, 75 );
  if(ppa_csr.sta_fifo.all() != my_val) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_fifo mismatched; expd : 0x" << hex << my_val << " rcvd : 0x" << ppa_csr.sta_fifo.all() << dec << endl);
  }

  ppa_csr.sta_preparse.read();
  if(ppa_csr.sta_preparse.all() != 0x28) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_preparse mismatched; expd : 0x28 rcvd : 0x" << ppa_csr.sta_preparse.all() << dec << endl);
  }

}


   //TCAM entry XY array read-write access, Bitcell definition 
   //XY== 00= any match, 01=match if 0, 10=match if 1, 11=no match ";
void cap_ppa_add_bndl0_tcam_entry(int chip_id, int inst_id, uint32_t idx, uint32_t state, uint32_t ctrl, uint32_t lkp_val0, uint32_t lkp_val1, uint32_t lkp_val2) {

    cap_ppa_csr_t & ppa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_ppa_csr_t, chip_id, inst_id);

    cap_ppa_decoders_ppa_lkp_tcam_key_t x, y;
    x.state(state);
    x.control(ctrl);
    x.lkp_val(lkp_val0, 0);
    x.lkp_val(lkp_val1, 1);
    x.lkp_val(lkp_val2, 2);

    y.state(~state);
    y.control(~ctrl);
    y.lkp_val(lkp_val0, 0);
    y.lkp_val(lkp_val1, 1);
    y.lkp_val(lkp_val2, 2);

    ppa_csr.dhs_bndl0_state_lkp_tcam.entry[idx].valid((pu_cpp_int < 1 >) 1);
    ppa_csr.dhs_bndl0_state_lkp_tcam.entry[idx].x_data((pu_cpp_int < 64 > )x.all());
    ppa_csr.dhs_bndl0_state_lkp_tcam.entry[idx].y_data((pu_cpp_int < 64 > )y.all());
    ppa_csr.dhs_bndl0_state_lkp_tcam.entry[idx].write();
    //ppa_csr.dhs_bndl0_state_lkp_tcam.entry[idx].read();
    ppa_csr.cfg_ctrl.read();  //dummy read.
}

void cap_ppa_add_bndl1_tcam_entry(int chip_id, int inst_id, uint32_t idx, uint32_t state, uint32_t ctrl, uint32_t lkp_val0, uint32_t lkp_val1, uint32_t lkp_val2) {

    cap_ppa_csr_t & ppa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_ppa_csr_t, chip_id, inst_id);

    cap_ppa_decoders_ppa_lkp_tcam_key_t x, y;
    x.state(state);
    x.control(ctrl);
    x.lkp_val(lkp_val0, 0);
    x.lkp_val(lkp_val1, 1);
    x.lkp_val(lkp_val2, 2);

    y.state(~state);
    y.control(~ctrl);
    y.lkp_val(lkp_val0, 0);
    y.lkp_val(lkp_val1, 1);
    y.lkp_val(lkp_val2, 2);


    ppa_csr.dhs_bndl0_state_lkp_tcam.entry[idx].valid((pu_cpp_int < 1 > ) 1);
    ppa_csr.dhs_bndl0_state_lkp_tcam.entry[idx].x_data((pu_cpp_int < 64 > ) x.all());
    ppa_csr.dhs_bndl0_state_lkp_tcam.entry[idx].y_data((pu_cpp_int < 64 > ) y.all());
    ppa_csr.dhs_bndl0_state_lkp_tcam.entry[idx].write();
    //ppa_csr.dhs_bndl0_state_lkp_tcam.entry[idx].read();
    ppa_csr.cfg_ctrl.read();  //dummy read.

}

void cap_ppa_program_tcam_sram_pair(int chip_id, int inst_id, uint32_t idx, cap_ppa_decoders_ppa_lkp_tcam_entry_t _tcam,
           cap_ppa_decoders_ppa_lkp_sram_entry_t _sram) {

    cap_ppa_csr_t & ppa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_ppa_csr_t, chip_id, inst_id);

    cap_ppa_decoders_ppa_lkp_tcam_key_t x, y;
    x.all(_tcam.key.all() & ~_tcam.mask.all());
    y.all(~_tcam.key.all() & ~_tcam.mask.all());

   // bndl0
    //ppa_csr.cfg_ctrl.read();  //dummy read.
    ppa_csr.dhs_bndl0_state_lkp_sram.entry[idx].data((pu_cpp_int< 764 >) _sram.all());
    PLOG_API_MSG("PPA", "index" << idx << hex << " writing bndl0_state_lkp_sram: " << _sram.all() << endl << dec)
    ppa_csr.dhs_bndl0_state_lkp_sram.entry[idx].write();
    ppa_csr.dhs_bndl0_state_lkp_sram.entry[idx].show();

    //ppa_csr.cfg_ctrl.read();  //dummy read.
    ppa_csr.dhs_bndl0_state_lkp_tcam.entry[idx].y_data((pu_cpp_int < 64 > ) y.all());
    ppa_csr.dhs_bndl0_state_lkp_tcam.entry[idx].x_data((pu_cpp_int < 64 > ) x.all());
    PLOG_API_MSG("PPA", "index" << idx << hex << " writing bndl0_state_lkp_tcam: " << endl << dec)
    ppa_csr.dhs_bndl0_state_lkp_tcam.entry[idx].valid(_tcam.valid());
    ppa_csr.dhs_bndl0_state_lkp_tcam.entry[idx].write();

   // bndl1
    //ppa_csr.cfg_ctrl.read();  //dummy read.
    PLOG_API_MSG("PPA", "index" << idx << hex << " writing bndl1_state_lkp_sram: " << endl << dec)
    ppa_csr.dhs_bndl1_state_lkp_sram.entry[idx].data((pu_cpp_int< 764 >) _sram.all());
    ppa_csr.dhs_bndl1_state_lkp_sram.entry[idx].write();

    //ppa_csr.cfg_ctrl.read();  //dummy read.
    PLOG_API_MSG("PPA", "index" << idx << hex << " writing bndl1_state_lkp_tcam: " << endl << dec)
    ppa_csr.dhs_bndl1_state_lkp_tcam.entry[idx].y_data((pu_cpp_int < 64 > ) y.all());
    ppa_csr.dhs_bndl1_state_lkp_tcam.entry[idx].x_data((pu_cpp_int < 64 > ) x.all());
    ppa_csr.dhs_bndl1_state_lkp_tcam.entry[idx].valid(_tcam.valid());
    ppa_csr.dhs_bndl1_state_lkp_tcam.entry[idx].write();
    //ppa_csr.dhs_bndl1_state_lkp_tcam.entry[idx].read();
   
    ppa_csr.cfg_ctrl.read();  //dummy read.
}

void cap_ppa_read_and_show_tcam_sram_pair(int chip_id, int inst_id, uint32_t idx) {

    cap_ppa_csr_t & ppa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_ppa_csr_t, chip_id, inst_id);
    PLOG_API_MSG("PPA", "reading tcam_sram_pair at index : " << idx << endl);

   // bndl0
    ppa_csr.dhs_bndl0_state_lkp_sram.entry[idx].read_hw();
    ppa_csr.dhs_bndl0_state_lkp_sram.entry[idx].show();

    ppa_csr.dhs_bndl0_state_lkp_tcam.entry[idx].read_hw();
    ppa_csr.dhs_bndl0_state_lkp_tcam.entry[idx].show();

   // bndl1
    ppa_csr.dhs_bndl1_state_lkp_sram.entry[idx].read_hw();
    ppa_csr.dhs_bndl1_state_lkp_sram.entry[idx].show();

    ppa_csr.dhs_bndl1_state_lkp_tcam.entry[idx].read_hw();
    ppa_csr.dhs_bndl1_state_lkp_tcam.entry[idx].show();
}

void cap_ppa_check_progr_sanity(int chip_id, int inst_id) {

  PLOG_API_MSG("PPA[" << inst_id << "]", "running programming sanity\n");
  //TBD :: Implement progr_sanity
  PLOG_API_MSG("PPA[" << inst_id << "]", "Finished running programming sanity\n");

}

void cap_ppa_disable_backdoor_paths(int chip_id, int inst_id) {
    cap_ppa_csr_t & ppa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_ppa_csr_t, chip_id, inst_id);

    //Backdoor access is not enabled for TCAMs, hence disable it on TCAMs.
    for(int idx = 0; idx < ppa_csr.dhs_bndl0_state_lkp_tcam.get_depth_entry(); idx++) {
        ppa_csr.dhs_bndl0_state_lkp_tcam.entry[idx].set_access_no_zero_time(1);
    }
    for(int idx = 0; idx < ppa_csr.dhs_bndl1_state_lkp_tcam.get_depth_entry(); idx++) {
        ppa_csr.dhs_bndl1_state_lkp_tcam.entry[idx].set_access_no_zero_time(1);
    }

}


void cap_ppa_csr_set_hdl_path(int chip_id, int inst_id, string path) {
    cap_ppa_csr_t & ppa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_ppa_csr_t, chip_id, inst_id);

    // id : 0 is given to csr instance.
    // id : 1 is given to memories
    ppa_csr.set_csr_inst_path(0, (path + ".csr")); 

    ppa_csr.dhs_bndl0_state_lkp_sram.set_csr_inst_path(1, (path + ".parse_top_inst0.ppa_bndl_inst0.ppa_state_lkp_inst0.ppa_state_sram_inst0.mem.mem"));
    ppa_csr.dhs_bndl1_state_lkp_sram.set_csr_inst_path(1, (path + ".parse_top_inst0.ppa_bndl_inst1.ppa_state_lkp_inst0.ppa_state_sram_inst0.mem.mem"));

}

//This api writes random values to CFG & CNT type registers.
void cap_ppa_randomize_csr(int chip_id, int inst_id) {
 cap_ppa_csr_t & ppa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_ppa_csr_t, chip_id, inst_id);

 PLOG_MSG( "inside cap_ppa_randomize_csr " << endl);

 string                 csr_name;
 pen_csr_base *         csr_ptr;
 vector<pen_csr_base *> csr_cfg;
 vector<pen_csr_base *> csr_cfg_tmp  = ppa_csr.get_children_prefix("cfg_");
 vector<pen_csr_base *> csr_cnt  = ppa_csr.get_children_prefix("cnt_");


 //Do not randomize cfg_global* registers as they have some timing/RTL sensitive fields.
 //Do not randomize cfg_bndl*_state_lkp_sram regs as they control ecc
 for (auto i: csr_cfg_tmp) {
     csr_name = i->get_name();
     //TBD : Review this exclude list 
     if(csr_name.find("state_lkp_sram") == string::npos
     && csr_name.find("cfg_pkt_mem") == string::npos
     && csr_name.find("cfg_sw_phv") == string::npos
     && csr_name.find("cfg_ctrl_thresh") == string::npos
     && csr_name.find("state_lkp_tcam") == string::npos
     && csr_name.find("cfg_debug") == string::npos
     && csr_name.find("cfg_ctrl") == string::npos
     && csr_name.find("_mem") == string::npos                 //Exlcude all cfg.*_mem regs as they wil start running bist, also change ecc configs
     && csr_name.find("cfg_init_profile") != string::npos
       ) {
        csr_cfg.push_back(i);
     } else {
        //PLOG_MSG(" do_main csr_cfg : removing item " << csr_name << endl)
     }
 }
 csr_cfg_tmp.clear();
            
 for(unsigned idx=0; idx<csr_cfg.size(); idx++) {
    csr_ptr   = csr_cfg[idx];
    PLOG_MSG( "randomizing cfg : " << csr_ptr->get_name() << endl);
    cap_csr_randomize(csr_ptr, true);
 }

 /* TBD : TODO : Enable this after EOS counter mismatches are debugged.
 for(int idx=0; idx<csr_cnt.size(); idx++) {
    csr_ptr   = csr_cnt[idx];
    PLOG_MSG( "randomizing cnt : " << csr_ptr->get_name() << endl);
    cap_csr_randomize(csr_ptr, true);
 }
 */

 //flush previuos writes
 ppa_csr.cfg_ctrl.read();  //dummy read.

} //cap_ppa_randomize_csr

void cap_ppa_sw_prof_cfg(int profile_id, cpp_int *sw_phv_data, int num_flits, int cnt_rep_en, int localtime_en, int frm_sz_en, uint64_t insert_period, int counter_max) {
    int chip_id = 0;
    int inst_id = 0;
    int sta_addr;
    cpp_int flit_data;

    cap_ppa_csr_t & ppa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_ppa_csr_t, chip_id, inst_id);

    ppa_csr.cfg_sw_phv_control[profile_id].start_enable(0);
    ppa_csr.cfg_sw_phv_control[profile_id].counter_repeat_enable(cnt_rep_en);
    ppa_csr.cfg_sw_phv_control[profile_id].qid_repeat_enable(0);
    ppa_csr.cfg_sw_phv_control[profile_id].localtime_enable(localtime_en);
    ppa_csr.cfg_sw_phv_control[profile_id].frame_size_enable(frm_sz_en);
    ppa_csr.cfg_sw_phv_control[profile_id].packet_len_enable(0);
    ppa_csr.cfg_sw_phv_control[profile_id].qid_enable(0);
    ppa_csr.cfg_sw_phv_control[profile_id].write();
    ppa_csr.cfg_sw_phv_control[profile_id].show();

    if (profile_id == 0)
        sta_addr = 0; 
    else
        sta_addr = ppa_csr.cfg_sw_phv_config[profile_id-1].start_addr().convert_to<int>() + ppa_csr.cfg_sw_phv_config[profile_id-1].num_flits().convert_to<int>() + 1; 

    ppa_csr.cfg_sw_phv_config[profile_id].start_addr(sta_addr);
    ppa_csr.cfg_sw_phv_config[profile_id].num_flits(num_flits - 1);
    ppa_csr.cfg_sw_phv_config[profile_id].insertion_period_clocks(insert_period);
    ppa_csr.cfg_sw_phv_config[profile_id].counter_max(counter_max);
    ppa_csr.cfg_sw_phv_config[profile_id].qid_min(0);
    ppa_csr.cfg_sw_phv_config[profile_id].qid_max(0);
    ppa_csr.cfg_sw_phv_config[profile_id].write();
    ppa_csr.cfg_sw_phv_config[profile_id].show();

    for (int i = 0; i < num_flits; i++) {
        flit_data = cpp_int_helper::s_get_slc(*sw_phv_data, 512, i*512);
        ppa_csr.dhs_sw_phv_mem.entry[sta_addr + i].all(flit_data);
        ppa_csr.dhs_sw_phv_mem.entry[sta_addr + i].write();
        ppa_csr.dhs_sw_phv_mem.entry[sta_addr + i].show();
    }
}

void cap_ppa_sw_phv_start_enable(int profile_id, int enable) {
    int chip_id = 0;
    int inst_id = 0;

    cap_ppa_csr_t & ppa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_ppa_csr_t, chip_id, inst_id);

    ppa_csr.cfg_sw_phv_control[profile_id].read();
    ppa_csr.cfg_sw_phv_control[profile_id].start_enable(enable);
    ppa_csr.cfg_sw_phv_control[profile_id].write();
}

void cap_ppa_bist_test(int chip_id, int inst_id) {
 cap_ppa_csr_t & ppa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_ppa_csr_t, chip_id, inst_id);
 int           loop_cnt;

 PLOG_API_MSG("PPA", "inside cap_ppa_bist_test \n");

 // CFG_BIST RUN Write
  ppa_csr.cfg_pkt_mem.bist_run(1);
  ppa_csr.cfg_bndl0_state_lkp_sram.bist_run(1);
  ppa_csr.cfg_bndl1_state_lkp_sram.bist_run(1);

  ppa_csr.cfg_bndl0_inq_mem.bist_run(1);
  ppa_csr.cfg_bndl1_inq_mem.bist_run(1);
  ppa_csr.cfg_outq_phv_mem.bist_run(1);
  ppa_csr.cfg_outq_ohi_mem.bist_run(1);
  ppa_csr.cfg_bndl0_state_lkp_tcam.bist_run(1);
  ppa_csr.cfg_bndl1_state_lkp_tcam.bist_run(1);

  ppa_csr.cfg_sw_phv_mem.bist_run(1);
  ppa_csr.cfg_pe0_phv_mem.bist_run(1);
  ppa_csr.cfg_pe1_phv_mem.bist_run(1);
  ppa_csr.cfg_pe2_phv_mem.bist_run(1);
  ppa_csr.cfg_pe3_phv_mem.bist_run(1);
  ppa_csr.cfg_pe4_phv_mem.bist_run(1);
  ppa_csr.cfg_pe5_phv_mem.bist_run(1);
  ppa_csr.cfg_pe6_phv_mem.bist_run(1);
  ppa_csr.cfg_pe7_phv_mem.bist_run(1);
  ppa_csr.cfg_pe8_phv_mem.bist_run(1);
  ppa_csr.cfg_pe9_phv_mem.bist_run(1);

  ppa_csr.cfg_pkt_mem.write();
  ppa_csr.cfg_bndl0_state_lkp_sram.write();
  ppa_csr.cfg_bndl1_state_lkp_sram.write();

  ppa_csr.cfg_bndl0_inq_mem.write();
  ppa_csr.cfg_bndl1_inq_mem.write();
  ppa_csr.cfg_outq_phv_mem.write();
  ppa_csr.cfg_outq_ohi_mem.write();
  ppa_csr.cfg_bndl0_state_lkp_tcam.write();
  ppa_csr.cfg_bndl1_state_lkp_tcam.write();

  ppa_csr.cfg_sw_phv_mem.write();
  ppa_csr.cfg_pe0_phv_mem.write();
  ppa_csr.cfg_pe1_phv_mem.write();
  ppa_csr.cfg_pe2_phv_mem.write();
  ppa_csr.cfg_pe3_phv_mem.write();
  ppa_csr.cfg_pe4_phv_mem.write();
  ppa_csr.cfg_pe5_phv_mem.write();
  ppa_csr.cfg_pe6_phv_mem.write();
  ppa_csr.cfg_pe7_phv_mem.write();
  ppa_csr.cfg_pe8_phv_mem.write();
  ppa_csr.cfg_pe9_phv_mem.write();

  // Check BIST_DONE_Pass/Fail in a loop till loop-timer expires and check status
  loop_cnt = 0;
  while (loop_cnt < 10000) {
    ppa_csr.sta_pkt_mem.read();
    if ( ppa_csr.sta_pkt_mem.bist_done_fail() == 1 || ppa_csr.sta_pkt_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 10000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pkt_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_pkt_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pkt_mem.bist_done_fail is set " << endl);
  }

  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_bndl0_state_lkp_sram.read();
    if ( ppa_csr.sta_bndl0_state_lkp_sram.bist_done_fail() == 1 || ppa_csr.sta_bndl0_state_lkp_sram.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_bndl0_state_lkp_sram.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_bndl0_state_lkp_sram.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_bndl0_state_lkp_sram.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_bndl1_state_lkp_sram.read();
    if ( ppa_csr.sta_bndl1_state_lkp_sram.bist_done_fail() == 1 || ppa_csr.sta_bndl1_state_lkp_sram.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_bndl1_state_lkp_sram.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_bndl1_state_lkp_sram.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_bndl1_state_lkp_sram.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_bndl0_inq_mem.read();
    if ( ppa_csr.sta_bndl0_inq_mem.bist_done_fail() == 1 || ppa_csr.sta_bndl0_inq_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_bndl0_inq_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_bndl0_inq_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_bndl0_inq_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_bndl1_inq_mem.read();
    if ( ppa_csr.sta_bndl1_inq_mem.bist_done_fail() == 1 || ppa_csr.sta_bndl1_inq_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_bndl1_inq_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_bndl1_inq_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_bndl1_inq_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_outq_phv_mem.read();
    if ( ppa_csr.sta_outq_phv_mem.bist_done_fail() == 1 || ppa_csr.sta_outq_phv_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_outq_phv_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_outq_phv_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_outq_phv_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_outq_ohi_mem.read();
    if ( ppa_csr.sta_outq_ohi_mem.bist_done_fail() == 1 || ppa_csr.sta_outq_ohi_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_outq_ohi_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_outq_ohi_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_outq_ohi_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_bndl0_state_lkp_tcam.read();
    if ( ppa_csr.sta_bndl0_state_lkp_tcam.bist_done_fail() == 1 || ppa_csr.sta_bndl0_state_lkp_tcam.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_bndl0_state_lkp_tcam.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_bndl0_state_lkp_tcam.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_bndl0_state_lkp_tcam.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_bndl1_state_lkp_tcam.read();
    if ( ppa_csr.sta_bndl1_state_lkp_tcam.bist_done_fail() == 1 || ppa_csr.sta_bndl1_state_lkp_tcam.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_bndl1_state_lkp_tcam.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_bndl1_state_lkp_tcam.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_bndl1_state_lkp_tcam.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_sw_phv_mem.read();
    if ( ppa_csr.sta_sw_phv_mem.bist_done_fail() == 1 || ppa_csr.sta_sw_phv_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_sw_phv_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_sw_phv_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_sw_phv_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_pe0_phv_mem.read();
    if ( ppa_csr.sta_pe0_phv_mem.bist_done_fail() == 1 || ppa_csr.sta_pe0_phv_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe0_phv_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_pe0_phv_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe0_phv_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_pe1_phv_mem.read();
    if ( ppa_csr.sta_pe1_phv_mem.bist_done_fail() == 1 || ppa_csr.sta_pe1_phv_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe1_phv_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_pe1_phv_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe1_phv_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_pe2_phv_mem.read();
    if ( ppa_csr.sta_pe2_phv_mem.bist_done_fail() == 1 || ppa_csr.sta_pe2_phv_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe2_phv_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_pe2_phv_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe2_phv_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_pe3_phv_mem.read();
    if ( ppa_csr.sta_pe3_phv_mem.bist_done_fail() == 1 || ppa_csr.sta_pe3_phv_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe3_phv_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_pe3_phv_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe3_phv_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_pe4_phv_mem.read();
    if ( ppa_csr.sta_pe4_phv_mem.bist_done_fail() == 1 || ppa_csr.sta_pe4_phv_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe4_phv_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_pe4_phv_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe4_phv_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_pe5_phv_mem.read();
    if ( ppa_csr.sta_pe5_phv_mem.bist_done_fail() == 1 || ppa_csr.sta_pe5_phv_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe5_phv_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_pe5_phv_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe5_phv_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_pe6_phv_mem.read();
    if ( ppa_csr.sta_pe6_phv_mem.bist_done_fail() == 1 || ppa_csr.sta_pe6_phv_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe6_phv_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_pe6_phv_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe6_phv_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_pe7_phv_mem.read();
    if ( ppa_csr.sta_pe7_phv_mem.bist_done_fail() == 1 || ppa_csr.sta_pe7_phv_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe7_phv_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_pe7_phv_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe7_phv_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_pe8_phv_mem.read();
    if ( ppa_csr.sta_pe8_phv_mem.bist_done_fail() == 1 || ppa_csr.sta_pe8_phv_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe8_phv_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_pe8_phv_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe8_phv_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_pe9_phv_mem.read();
    if ( ppa_csr.sta_pe9_phv_mem.bist_done_fail() == 1 || ppa_csr.sta_pe9_phv_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe9_phv_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_pe9_phv_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe9_phv_mem.bist_done_fail is set " << endl);
  }
      
} //cap_ppa_bist_test


void cap_ppa_bist_test_start(int chip_id, int inst_id) {
 cap_ppa_csr_t & ppa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_ppa_csr_t, chip_id, inst_id);
 //int           loop_cnt;

 PLOG_API_MSG("PPA", "inside cap_ppa_bist_test \n");

 // CFG_BIST RUN Write
  ppa_csr.cfg_pkt_mem.bist_run(1);
  ppa_csr.cfg_bndl0_state_lkp_sram.bist_run(1);
  ppa_csr.cfg_bndl1_state_lkp_sram.bist_run(1);

  ppa_csr.cfg_bndl0_inq_mem.bist_run(1);
  ppa_csr.cfg_bndl1_inq_mem.bist_run(1);
  ppa_csr.cfg_outq_phv_mem.bist_run(1);
  ppa_csr.cfg_outq_ohi_mem.bist_run(1);
  ppa_csr.cfg_bndl0_state_lkp_tcam.bist_run(1);
  ppa_csr.cfg_bndl1_state_lkp_tcam.bist_run(1);

  ppa_csr.cfg_sw_phv_mem.bist_run(1);
  ppa_csr.cfg_pe0_phv_mem.bist_run(1);
  ppa_csr.cfg_pe1_phv_mem.bist_run(1);
  ppa_csr.cfg_pe2_phv_mem.bist_run(1);
  ppa_csr.cfg_pe3_phv_mem.bist_run(1);
  ppa_csr.cfg_pe4_phv_mem.bist_run(1);
  ppa_csr.cfg_pe5_phv_mem.bist_run(1);
  ppa_csr.cfg_pe6_phv_mem.bist_run(1);
  ppa_csr.cfg_pe7_phv_mem.bist_run(1);
  ppa_csr.cfg_pe8_phv_mem.bist_run(1);
  ppa_csr.cfg_pe9_phv_mem.bist_run(1);

  ppa_csr.cfg_pkt_mem.write();
  ppa_csr.cfg_bndl0_state_lkp_sram.write();
  ppa_csr.cfg_bndl1_state_lkp_sram.write();

  ppa_csr.cfg_bndl0_inq_mem.write();
  ppa_csr.cfg_bndl1_inq_mem.write();
  ppa_csr.cfg_outq_phv_mem.write();
  ppa_csr.cfg_outq_ohi_mem.write();
  ppa_csr.cfg_bndl0_state_lkp_tcam.write();
  ppa_csr.cfg_bndl1_state_lkp_tcam.write();

  ppa_csr.cfg_sw_phv_mem.write();
  ppa_csr.cfg_pe0_phv_mem.write();
  ppa_csr.cfg_pe1_phv_mem.write();
  ppa_csr.cfg_pe2_phv_mem.write();
  ppa_csr.cfg_pe3_phv_mem.write();
  ppa_csr.cfg_pe4_phv_mem.write();
  ppa_csr.cfg_pe5_phv_mem.write();
  ppa_csr.cfg_pe6_phv_mem.write();
  ppa_csr.cfg_pe7_phv_mem.write();
  ppa_csr.cfg_pe8_phv_mem.write();
  ppa_csr.cfg_pe9_phv_mem.write();
      
} //cap_ppa_bist_test_start


void cap_ppa_bist_test_clear(int chip_id, int inst_id) {
 cap_ppa_csr_t & ppa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_ppa_csr_t, chip_id, inst_id);
 //int           loop_cnt;

 PLOG_API_MSG("PPA", "inside cap_ppa_bist_test \n");

 // CFG_BIST RUN Write
  ppa_csr.cfg_pkt_mem.bist_run(0);
  ppa_csr.cfg_bndl0_state_lkp_sram.bist_run(0);
  ppa_csr.cfg_bndl1_state_lkp_sram.bist_run(0);

  ppa_csr.cfg_bndl0_inq_mem.bist_run(0);
  ppa_csr.cfg_bndl1_inq_mem.bist_run(0);
  ppa_csr.cfg_outq_phv_mem.bist_run(0);
  ppa_csr.cfg_outq_ohi_mem.bist_run(0);
  ppa_csr.cfg_bndl0_state_lkp_tcam.bist_run(0);
  ppa_csr.cfg_bndl1_state_lkp_tcam.bist_run(0);

  ppa_csr.cfg_sw_phv_mem.bist_run(0);
  ppa_csr.cfg_pe0_phv_mem.bist_run(0);
  ppa_csr.cfg_pe1_phv_mem.bist_run(0);
  ppa_csr.cfg_pe2_phv_mem.bist_run(0);
  ppa_csr.cfg_pe3_phv_mem.bist_run(0);
  ppa_csr.cfg_pe4_phv_mem.bist_run(0);
  ppa_csr.cfg_pe5_phv_mem.bist_run(0);
  ppa_csr.cfg_pe6_phv_mem.bist_run(0);
  ppa_csr.cfg_pe7_phv_mem.bist_run(0);
  ppa_csr.cfg_pe8_phv_mem.bist_run(0);
  ppa_csr.cfg_pe9_phv_mem.bist_run(0);

  ppa_csr.cfg_pkt_mem.write();
  ppa_csr.cfg_bndl0_state_lkp_sram.write();
  ppa_csr.cfg_bndl1_state_lkp_sram.write();

  ppa_csr.cfg_bndl0_inq_mem.write();
  ppa_csr.cfg_bndl1_inq_mem.write();
  ppa_csr.cfg_outq_phv_mem.write();
  ppa_csr.cfg_outq_ohi_mem.write();
  ppa_csr.cfg_bndl0_state_lkp_tcam.write();
  ppa_csr.cfg_bndl1_state_lkp_tcam.write();

  ppa_csr.cfg_sw_phv_mem.write();
  ppa_csr.cfg_pe0_phv_mem.write();
  ppa_csr.cfg_pe1_phv_mem.write();
  ppa_csr.cfg_pe2_phv_mem.write();
  ppa_csr.cfg_pe3_phv_mem.write();
  ppa_csr.cfg_pe4_phv_mem.write();
  ppa_csr.cfg_pe5_phv_mem.write();
  ppa_csr.cfg_pe6_phv_mem.write();
  ppa_csr.cfg_pe7_phv_mem.write();
  ppa_csr.cfg_pe8_phv_mem.write();
  ppa_csr.cfg_pe9_phv_mem.write();
      
} //cap_ppa_bist_test_clear

void cap_ppa_bist_test_chk(int chip_id, int inst_id) {
 cap_ppa_csr_t & ppa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_ppa_csr_t, chip_id, inst_id);
 int           loop_cnt;

 PLOG_API_MSG("PPA", "inside cap_ppa_bist_test_chk \n");

  // Check BIST_DONE_Pass/Fail in a loop till loop-timer expires and check status
  loop_cnt = 0;
  while (loop_cnt < 10000) {
    ppa_csr.sta_pkt_mem.read();
    if ( ppa_csr.sta_pkt_mem.bist_done_fail() == 1 || ppa_csr.sta_pkt_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 10000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pkt_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_pkt_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pkt_mem.bist_done_fail is set " << endl);
  }

  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_bndl0_state_lkp_sram.read();
    if ( ppa_csr.sta_bndl0_state_lkp_sram.bist_done_fail() == 1 || ppa_csr.sta_bndl0_state_lkp_sram.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_bndl0_state_lkp_sram.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_bndl0_state_lkp_sram.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_bndl0_state_lkp_sram.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_bndl1_state_lkp_sram.read();
    if ( ppa_csr.sta_bndl1_state_lkp_sram.bist_done_fail() == 1 || ppa_csr.sta_bndl1_state_lkp_sram.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_bndl1_state_lkp_sram.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_bndl1_state_lkp_sram.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_bndl1_state_lkp_sram.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_bndl0_inq_mem.read();
    if ( ppa_csr.sta_bndl0_inq_mem.bist_done_fail() == 1 || ppa_csr.sta_bndl0_inq_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_bndl0_inq_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_bndl0_inq_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_bndl0_inq_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_bndl1_inq_mem.read();
    if ( ppa_csr.sta_bndl1_inq_mem.bist_done_fail() == 1 || ppa_csr.sta_bndl1_inq_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_bndl1_inq_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_bndl1_inq_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_bndl1_inq_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_outq_phv_mem.read();
    if ( ppa_csr.sta_outq_phv_mem.bist_done_fail() == 1 || ppa_csr.sta_outq_phv_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_outq_phv_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_outq_phv_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_outq_phv_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_outq_ohi_mem.read();
    if ( ppa_csr.sta_outq_ohi_mem.bist_done_fail() == 1 || ppa_csr.sta_outq_ohi_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_outq_ohi_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_outq_ohi_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_outq_ohi_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_bndl0_state_lkp_tcam.read();
    if ( ppa_csr.sta_bndl0_state_lkp_tcam.bist_done_fail() == 1 || ppa_csr.sta_bndl0_state_lkp_tcam.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_bndl0_state_lkp_tcam.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_bndl0_state_lkp_tcam.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_bndl0_state_lkp_tcam.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_bndl1_state_lkp_tcam.read();
    if ( ppa_csr.sta_bndl1_state_lkp_tcam.bist_done_fail() == 1 || ppa_csr.sta_bndl1_state_lkp_tcam.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_bndl1_state_lkp_tcam.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_bndl1_state_lkp_tcam.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_bndl1_state_lkp_tcam.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_sw_phv_mem.read();
    if ( ppa_csr.sta_sw_phv_mem.bist_done_fail() == 1 || ppa_csr.sta_sw_phv_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_sw_phv_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_sw_phv_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_sw_phv_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_pe0_phv_mem.read();
    if ( ppa_csr.sta_pe0_phv_mem.bist_done_fail() == 1 || ppa_csr.sta_pe0_phv_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe0_phv_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_pe0_phv_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe0_phv_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_pe1_phv_mem.read();
    if ( ppa_csr.sta_pe1_phv_mem.bist_done_fail() == 1 || ppa_csr.sta_pe1_phv_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe1_phv_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_pe1_phv_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe1_phv_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_pe2_phv_mem.read();
    if ( ppa_csr.sta_pe2_phv_mem.bist_done_fail() == 1 || ppa_csr.sta_pe2_phv_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe2_phv_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_pe2_phv_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe2_phv_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_pe3_phv_mem.read();
    if ( ppa_csr.sta_pe3_phv_mem.bist_done_fail() == 1 || ppa_csr.sta_pe3_phv_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe3_phv_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_pe3_phv_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe3_phv_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_pe4_phv_mem.read();
    if ( ppa_csr.sta_pe4_phv_mem.bist_done_fail() == 1 || ppa_csr.sta_pe4_phv_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe4_phv_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_pe4_phv_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe4_phv_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_pe5_phv_mem.read();
    if ( ppa_csr.sta_pe5_phv_mem.bist_done_fail() == 1 || ppa_csr.sta_pe5_phv_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe5_phv_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_pe5_phv_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe5_phv_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_pe6_phv_mem.read();
    if ( ppa_csr.sta_pe6_phv_mem.bist_done_fail() == 1 || ppa_csr.sta_pe6_phv_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe6_phv_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_pe6_phv_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe6_phv_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_pe7_phv_mem.read();
    if ( ppa_csr.sta_pe7_phv_mem.bist_done_fail() == 1 || ppa_csr.sta_pe7_phv_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe7_phv_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_pe7_phv_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe7_phv_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_pe8_phv_mem.read();
    if ( ppa_csr.sta_pe8_phv_mem.bist_done_fail() == 1 || ppa_csr.sta_pe8_phv_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe8_phv_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_pe8_phv_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe8_phv_mem.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 5000) {
    ppa_csr.sta_pe9_phv_mem.read();
    if ( ppa_csr.sta_pe9_phv_mem.bist_done_fail() == 1 || ppa_csr.sta_pe9_phv_mem.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 5000) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe9_phv_mem.bist_done not set for timeout cycles " << endl);
  } else if( ppa_csr.sta_pe9_phv_mem.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : ppa[" << inst_id << "] : sta_pe9_phv_mem.bist_done_fail is set " << endl);
  }
      
} //cap_ppa_bist_chk


void cap_pa_sw_phv_start_enable(int chip_id, int inst_id, int profile_id, int enable) {
    cap_ppa_csr_t & ppa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_ppa_csr_t, chip_id, inst_id);

    ppa_csr.cfg_sw_phv_control[profile_id].read();
    ppa_csr.cfg_sw_phv_control[profile_id].start_enable(enable);
    ppa_csr.cfg_sw_phv_control[profile_id].write();
    ppa_csr.cfg_sw_phv_control[profile_id].read();
}


int cap_run_ppa_tcam_rdwr_test(int chip_id, int inst_id, int bndl, int fast_mode, int verbosity) {

            cap_ppa_csr_t & ppa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_ppa_csr_t, chip_id, inst_id);

	    cpp_int_helper hlp;
            cap_ppa_csr_dhs_bndl0_state_lkp_tcam_entry_t::x_data_cpp_int_t x_data;
            cap_ppa_csr_dhs_bndl0_state_lkp_tcam_entry_t::y_data_cpp_int_t y_data;
            cpp_int val = 1;
	    cpp_int tmp_x_data;
	    cpp_int inv_x_data;
            cpp_int exp_hit; 
            cpp_int exp_hit_addr; 
            uint32_t n_tcam_addrbits;
          
            int fail = 0;

            if (bndl == 0) {
               n_tcam_addrbits = log2(ppa_csr.dhs_bndl0_state_lkp_tcam.get_depth_entry());
            } else {
               n_tcam_addrbits = log2(ppa_csr.dhs_bndl1_state_lkp_tcam.get_depth_entry());
            }
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
                         if (bndl == 0) {
		            ppa_csr.dhs_bndl0_state_lkp_tcam.entry[ii].x_data(tmp_x_data);
		            ppa_csr.dhs_bndl0_state_lkp_tcam.entry[ii].y_data(~tmp_x_data);
		            ppa_csr.dhs_bndl0_state_lkp_tcam.entry[ii].valid(~n_iter);
		            ppa_csr.dhs_bndl0_state_lkp_tcam.entry[ii].write();
                         } else {
		            ppa_csr.dhs_bndl1_state_lkp_tcam.entry[ii].x_data(tmp_x_data);
		            ppa_csr.dhs_bndl1_state_lkp_tcam.entry[ii].y_data(~tmp_x_data);
		            ppa_csr.dhs_bndl1_state_lkp_tcam.entry[ii].valid(~n_iter);
		            ppa_csr.dhs_bndl1_state_lkp_tcam.entry[ii].write();
                         }
	          }

                 // Read back what was written 
                  unsigned num_line = (fast_mode == 1) ? 1 : 64;
	          for(uint32_t ii = 0; ii < num_line ; ii++) {
                          if (bndl == 0) {
		            ppa_csr.dhs_bndl0_state_lkp_tcam.entry[ii].read();
		            if (verbosity == 1) ppa_csr.dhs_bndl0_state_lkp_tcam.entry[ii].show();
                          } else {
		            ppa_csr.dhs_bndl1_state_lkp_tcam.entry[ii].read();
		            if (verbosity == 1) ppa_csr.dhs_bndl1_state_lkp_tcam.entry[ii].show();
                          } 
		          val = 1;
                          tmp_x_data = 0;
          	 	  tmp_x_data = hlp.set_slc( tmp_x_data , val , ii % num_line , ii % num_line );
			  inv_x_data = 1;	
                          inv_x_data <<= 64;
                          inv_x_data = inv_x_data - 1;
		          val = 0;
          	 	  inv_x_data = hlp.set_slc( inv_x_data , val, ii % num_line , ii % num_line );
			  //inv_x_data = (~tmp_x_data & ((1 << 64)-1));

                          if (bndl == 0) {
                             if (ppa_csr.dhs_bndl0_state_lkp_tcam.entry[ii].x_data() != tmp_x_data) {
                                fail = 1;
				PLOG_ERR("TCAM Read X data mismatches, rec 0x" << hex << ppa_csr.dhs_bndl0_state_lkp_tcam.entry[ii].x_data() << " expected: 0x" << tmp_x_data << endl << dec);
			     }
                             if (ppa_csr.dhs_bndl0_state_lkp_tcam.entry[ii].y_data() != inv_x_data) {
                                fail = 1;
				PLOG_ERR("TCAM Read Y data mismatches, rec 0x" << hex << ppa_csr.dhs_bndl0_state_lkp_tcam.entry[ii].y_data() << " expected: 0x" << inv_x_data << endl << dec);
			     }
                             if (ppa_csr.dhs_bndl0_state_lkp_tcam.entry[ii].valid().convert_to< uint32_t >() != (n_iter ? 0 : 1) ) {
                                fail = 1;
				PLOG_ERR("TCAM Read Valid bit mismatches, rec 0x" << hex << ppa_csr.dhs_bndl0_state_lkp_tcam.entry[ii].valid() << " expected: 0x" << (n_iter ? 0 :1) << endl << dec);
			    }
			  } else {
                             if (ppa_csr.dhs_bndl1_state_lkp_tcam.entry[ii].x_data() != tmp_x_data) {
                                fail = 1;
				PLOG_ERR("TCAM Read X data mismatches, rec 0x" << hex << ppa_csr.dhs_bndl1_state_lkp_tcam.entry[ii].x_data() << " expected: 0x" << tmp_x_data << endl << dec);
			     }
                             if (ppa_csr.dhs_bndl1_state_lkp_tcam.entry[ii].y_data() != inv_x_data) {
                                fail = 1;
				PLOG_ERR("TCAM Read Y data mismatches, rec 0x" << hex << ppa_csr.dhs_bndl1_state_lkp_tcam.entry[ii].y_data() << " expected: 0x" << inv_x_data << endl << dec);
			     }
                             if (ppa_csr.dhs_bndl1_state_lkp_tcam.entry[ii].valid().convert_to< uint32_t >() != (n_iter ? 0 : 1) ) {
                                fail = 1;
				PLOG_ERR("TCAM Read Valid bit mismatches, rec 0x" << hex << ppa_csr.dhs_bndl1_state_lkp_tcam.entry[ii].valid() << " expected: 0x" << (n_iter ? 0 :1) << endl << dec);
			    }
			  } 
	          }
		  val = 1;

                 // Search Indirect
	          for(uint32_t ii = 0; ii < 64 ; ii++) {
                          tmp_x_data = 0;
                          if (bndl == 0) {
                             ppa_csr.cfg_bndl0_state_lkp_tcam_ind.mask(~tmp_x_data) ;     // TCAM Search enable= CEi=all_ones
		             tmp_x_data = hlp.set_slc( tmp_x_data , val , ii % 64 , ii % 64 );
                             ppa_csr.cfg_bndl0_state_lkp_tcam_ind.key(tmp_x_data) ; 
                             ppa_csr.cfg_bndl0_state_lkp_tcam_ind.write() ; 
                             ppa_csr.dhs_bndl0_state_lkp_tcam_ind.entry.write() ; 
                             ppa_csr.sta_bndl0_state_lkp_tcam_ind.read() ; 
                             if (verbosity == 1) ppa_csr.sta_bndl0_state_lkp_tcam_ind.show() ; 
                          } else {
                             ppa_csr.cfg_bndl1_state_lkp_tcam_ind.mask(~tmp_x_data) ;     // TCAM Search enable= CEi=all_ones
		             tmp_x_data = hlp.set_slc( tmp_x_data , val , ii % 64 , ii % 64 );
                             ppa_csr.cfg_bndl1_state_lkp_tcam_ind.key(tmp_x_data) ; 
                             ppa_csr.cfg_bndl1_state_lkp_tcam_ind.write() ; 
                             ppa_csr.dhs_bndl1_state_lkp_tcam_ind.entry.write() ; 
                             ppa_csr.sta_bndl1_state_lkp_tcam_ind.read() ; 
                             if (verbosity == 1) ppa_csr.sta_bndl1_state_lkp_tcam_ind.show() ; 
                          }

                          exp_hit = n_iter ? 0 : 1 ; 
                          exp_hit_addr = ii ; 

                          if (bndl == 0) {
                            if (ppa_csr.sta_bndl0_state_lkp_tcam_ind.hit() != exp_hit) {
                                fail = 1;
				PLOG_ERR("data walk TCAM Hit mismatches, rec 0x" << hex << ppa_csr.sta_bndl0_state_lkp_tcam_ind.hit() << " expected: 0x" << exp_hit << endl << dec);
			    } else {
                              if (exp_hit && (ppa_csr.sta_bndl0_state_lkp_tcam_ind.hit_addr().convert_to< uint32_t >() != ii)) {
                                fail = 1;
				PLOG_ERR("data walk TCAM Hit_addr mismatches, rec 0x" << hex << ppa_csr.sta_bndl0_state_lkp_tcam_ind.hit_addr() << " expected: 0x" << ii << endl << dec);
			      }
                            }
                          } else {
                            if (ppa_csr.sta_bndl1_state_lkp_tcam_ind.hit() != exp_hit) {
                                fail = 1;
				PLOG_ERR("data walk TCAM Hit mismatches, rec 0x" << hex << ppa_csr.sta_bndl1_state_lkp_tcam_ind.hit() << " expected: 0x" << exp_hit << endl << dec);
			    } else {
                              if (exp_hit && (ppa_csr.sta_bndl1_state_lkp_tcam_ind.hit_addr().convert_to< uint32_t >() != ii)) {
                                fail = 1;
				PLOG_ERR("data walk TCAM Hit_addr mismatches, rec 0x" << hex << ppa_csr.sta_bndl1_state_lkp_tcam_ind.hit_addr() << " expected: 0x" << ii << endl << dec);
			      }
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
                         if (bndl == 0) {
		            ppa_csr.dhs_bndl0_state_lkp_tcam.entry[(1 << ii)].x_data(tmp_x_data);
		            ppa_csr.dhs_bndl0_state_lkp_tcam.entry[(1 << ii)].y_data(~tmp_x_data);
		            ppa_csr.dhs_bndl0_state_lkp_tcam.entry[(1 << ii)].valid(1);
		            ppa_csr.dhs_bndl0_state_lkp_tcam.entry[(1 << ii)].write();
                         } else {
		            ppa_csr.dhs_bndl1_state_lkp_tcam.entry[(1 << ii)].x_data(tmp_x_data);
		            ppa_csr.dhs_bndl1_state_lkp_tcam.entry[(1 << ii)].y_data(~tmp_x_data);
		            ppa_csr.dhs_bndl1_state_lkp_tcam.entry[(1 << ii)].valid(1);
		            ppa_csr.dhs_bndl1_state_lkp_tcam.entry[(1 << ii)].write();
                         }
	          }
                 
                 // Search Indirect
	          for(uint32_t ii = 0; ii < (n_tcam_addrbits) ; ii++) {
                          tmp_x_data = 0;
                          if (bndl == 0) {
                             ppa_csr.cfg_bndl0_state_lkp_tcam_ind.mask(~tmp_x_data) ;     // TCAM Search enable= CEi=all_ones
                          } else {
                             ppa_csr.cfg_bndl1_state_lkp_tcam_ind.mask(~tmp_x_data) ;     // TCAM Search enable= CEi=all_ones
                          }

                          val = (1<< ii) ; 
           		  tmp_x_data = hlp.set_slc( tmp_x_data , val , 0 , 15 );

                          if (bndl == 0) {
                             ppa_csr.cfg_bndl0_state_lkp_tcam_ind.key(tmp_x_data) ; 
                             ppa_csr.cfg_bndl0_state_lkp_tcam_ind.write() ; 
                             ppa_csr.dhs_bndl0_state_lkp_tcam_ind.entry.write() ; 
                             ppa_csr.sta_bndl0_state_lkp_tcam_ind.read() ; 
                             if (verbosity == 1) ppa_csr.sta_bndl0_state_lkp_tcam_ind.show() ; 
                             exp_hit = 1 ; 
                             if (ppa_csr.sta_bndl0_state_lkp_tcam_ind.hit() != exp_hit) {
                                   fail = 1;
				   PLOG_ERR("addr walk TCAM Hit mismatches, rec 0x" << hex << ppa_csr.sta_bndl0_state_lkp_tcam_ind.hit() << " expected: 0x" << exp_hit << endl << dec);
			     } else {
				   //PLOG_MSG("addr walk TCAM Hit pass, rec 0x" << hex << ppa_csr.sta_bndl0_state_lkp_tcam_ind.hit() << endl << dec);
			     }
                          } else {
                             ppa_csr.cfg_bndl1_state_lkp_tcam_ind.key(tmp_x_data) ; 
                             ppa_csr.cfg_bndl1_state_lkp_tcam_ind.write() ; 
                             ppa_csr.dhs_bndl1_state_lkp_tcam_ind.entry.write() ; 
                             ppa_csr.sta_bndl1_state_lkp_tcam_ind.read() ; 
                             if (verbosity == 1) ppa_csr.sta_bndl1_state_lkp_tcam_ind.show() ; 
                             exp_hit = 1 ; 
                             if (ppa_csr.sta_bndl1_state_lkp_tcam_ind.hit() != exp_hit) {
                                   fail = 1;
				   PLOG_ERR("addr walk TCAM Hit mismatches, rec 0x" << hex << ppa_csr.sta_bndl1_state_lkp_tcam_ind.hit() << " expected: 0x" << exp_hit << endl << dec);
			     } else {
				   //PLOG_MSG("addr walk TCAM Hit pass, rec 0x" << hex << ppa_csr.sta_bndl1_state_lkp_tcam_ind.hit() << endl << dec);
                             }
                          }
	          }

                  // issue TCAM grst
                  if (bndl == 0) {
                     ppa_csr.cfg_bndl0_state_lkp_tcam_grst.vld(1);
                     ppa_csr.cfg_bndl0_state_lkp_tcam_grst.write();
                  } else {
                     ppa_csr.cfg_bndl1_state_lkp_tcam_grst.vld(1);
                     ppa_csr.cfg_bndl1_state_lkp_tcam_grst.write();
                  }

                 // Search Again, now it should miss
	          for(uint32_t ii = 0; ii < (n_tcam_addrbits) ; ii++) {
                          tmp_x_data = 0;
                          if (bndl == 0) {
                             ppa_csr.cfg_bndl0_state_lkp_tcam_ind.mask(~tmp_x_data) ;     // TCAM Search enable= CEi=all_ones
                             val = (1<< ii) ; 
           		     tmp_x_data = hlp.set_slc( tmp_x_data , val , 0 , 15 );

                             ppa_csr.cfg_bndl0_state_lkp_tcam_ind.key(tmp_x_data) ; 
                             ppa_csr.cfg_bndl0_state_lkp_tcam_ind.write() ; 
                             ppa_csr.dhs_bndl0_state_lkp_tcam_ind.entry.write() ; 
                             ppa_csr.sta_bndl0_state_lkp_tcam_ind.read() ; 
                             if (verbosity == 1) ppa_csr.sta_bndl0_state_lkp_tcam_ind.show() ; 
                             exp_hit = 0 ; 
                             if (ppa_csr.sta_bndl0_state_lkp_tcam_ind.hit() != exp_hit) {
                                   fail = 1;
				  PLOG_ERR("addr walk TCAM Hit mismatches, rec 0x" << hex << ppa_csr.sta_bndl0_state_lkp_tcam_ind.hit() << " expected: 0x" << exp_hit << endl << dec);
			     } else {
				  //PLOG_MSG("addr walk TCAM Hit pass, rec 0x" << hex << ppa_csr.sta_bndl0_state_lkp_tcam_ind.hit() << endl << dec);
			     }
			  } else {
                             ppa_csr.cfg_bndl1_state_lkp_tcam_ind.mask(~tmp_x_data) ;     // TCAM Search enable= CEi=all_ones
                             val = (1<< ii) ; 
           		     tmp_x_data = hlp.set_slc( tmp_x_data , val , 0 , 15 );

                             ppa_csr.cfg_bndl1_state_lkp_tcam_ind.key(tmp_x_data) ; 
                             ppa_csr.cfg_bndl1_state_lkp_tcam_ind.write() ; 
                             ppa_csr.dhs_bndl1_state_lkp_tcam_ind.entry.write() ; 
                             ppa_csr.sta_bndl1_state_lkp_tcam_ind.read() ; 
                             if (verbosity == 1) ppa_csr.sta_bndl1_state_lkp_tcam_ind.show() ; 
                             exp_hit = 0 ; 
                             if (ppa_csr.sta_bndl1_state_lkp_tcam_ind.hit() != exp_hit) {
                                   fail = 1;
				  PLOG_ERR("addr walk TCAM Hit mismatches, rec 0x" << hex << ppa_csr.sta_bndl1_state_lkp_tcam_ind.hit() << " expected: 0x" << exp_hit << endl << dec);
			     } else {
				  //PLOG_MSG("addr walk TCAM Hit pass, rec 0x" << hex << ppa_csr.sta_bndl1_state_lkp_tcam_ind.hit() << endl << dec);
			     }
			  }
	          }

            if (fail == 0) {
	       PLOG_MSG("=== PPA TCAM pass chip_id:" << chip_id << " inst_id:" << inst_id << " bndl:" << bndl << endl << dec);
            } else {
	       PLOG_ERR("=== PPA TCAM fail chip_id:" << chip_id << " inst_id:" << inst_id << " bndl:" << bndl << endl << dec);
            }

            return (fail);

}
