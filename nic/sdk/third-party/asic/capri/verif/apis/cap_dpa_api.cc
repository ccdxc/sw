#include "cap_dpa_api.h"
#include "cap_ppa_api.h"
#include "msg_man.h"


void cap_dpa_soft_reset(int chip_id, int inst_id) {

 PLOG_API_MSG("DPA", "inside softreset\n");

 PLOG_API_MSG("DPA", "Dumping Sknob vars" << std::endl);
 //sknobs_dump();

 //cap_dpp_csr_t & dpp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpp_csr_t, chip_id, inst_id);
 //cap_dpr_csr_t & dpr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpr_csr_t, chip_id, inst_id);
 cap_dpa_disable_backdoor_paths(chip_id, inst_id);

 PLOG_API_MSG("DPA", "done with softreset\n");
}

void cap_dpa_set_soft_reset(int chip_id, int inst_id, int value) { 
}

void cap_dpa_init_start(int chip_id, int inst_id) {
PLOG_API_MSG("DPA", "inside init\n");

 cap_dpp_csr_t & dpp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpp_csr_t, chip_id, inst_id);
 cap_dpr_csr_t & dpr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpr_csr_t, chip_id, inst_id);

 dpp_csr.cfg_global_hw.framer_eop_pacer_timer(7);
 dpp_csr.cfg_global_hw.write();

 //disable_ptr_lookahead should be 0 and also eop_err_set should be 1  
 dpr_csr.cfg_global_hw_1.disable_ptr_lookahead(0);
 dpr_csr.cfg_global_hw_1.eop_err_set_disable(1);
 dpr_csr.cfg_global_hw_1.write();

 //pktout_fc_threshold should be 5  
 dpr_csr.cfg_global_hw.pktout_fc_threshold(5);
 dpr_csr.cfg_global_hw.write();


 //dpr_csr.int_reg1 can spuriously fire immendiately after reset, hence clear it (write 1 to clear).
 dpr_csr.int_reg1.all(0xffffffff);
 dpr_csr.int_reg1.write();
 dpr_csr.int_reg1.all(0x0);  //Write 0s in shadow value

#ifdef MODULE_SIM
   cpp_int_helper hlp;
   cpp_int tmp;
   RRKnob   rand_knob("rand_knob", 0, 0xffffffff);
   int      gate_sim = SKNOBS_GET("gate_sim", 0);

   if(gate_sim) {
     for(int idx=0; idx<32; idx++) {
       for(int bit_idx=0; bit_idx<1218; bit_idx+=32) {
         tmp = hlp.set_slc( tmp,  rand_knob.eval(), bit_idx, bit_idx+31); 
       }
       dpr_csr.mem.dhs_dpr_ptr_fifo_sram.entry[idx].data(tmp);
       dpr_csr.mem.dhs_dpr_ptr_fifo_sram.entry[idx].write();
       PLOG_API_MSG("DPA", " writing dhs_dpr_ptr_fifo_sram at entry : " << idx << endl);
       dpr_csr.mem.dhs_dpr_ptr_fifo_sram.entry[idx].show();
     }
     dpr_csr.mem.dhs_dpr_ptr_fifo_sram.entry[0].read();
   }
#endif

 //Dummy read to flush
 dpr_csr.cfg_global.read();

 PLOG_API_MSG("DPA", "done with init\n");
}

void cap_dpa_init_done(int chip_id, int inst_id) {
}


void cap_dpa_load_from_cfg(int chip_id, int inst_id) {
 PLOG_API_MSG("DPA", "Before load_from_cfg\n");

 cap_dpp_csr_t & dpp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpp_csr_t, chip_id, inst_id);
 cap_dpr_csr_t & dpr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpr_csr_t, chip_id, inst_id);

 dpp_csr.load_from_cfg();
 dpr_csr.load_from_cfg();

 int scapy_csum_crc_config_en = SKNOBS_GET("scapy_csum_crc_config_en", 0);
 if(scapy_csum_crc_config_en) {
    cap_dpa_csum_config(chip_id, inst_id);
    cap_dpa_crc_config(chip_id, inst_id);
 }

#ifdef MODULE_SIM
 RRKnob   rand_knob("rand_knob", 0, 0xffffffff);
 int tmp_val, tmp_val1;
 dpr_csr.cfg_pkt_padding.min_size(0);
 for(int i=0; i<4; i++) {
     tmp_val1 = rand_knob.eval();
     //10% : 0-63; 80% : 64:128 ; 10% : 128-255
     tmp_val  = (tmp_val1 % 100) < 10 ? rand_knob.eval()%64 : ( (tmp_val1 % 100) < 90 ? (64 + rand_knob.eval()%65) : (128 + rand_knob.eval()%128));
     dpr_csr.cfg_pkt_padding.min_size( ( (dpr_csr.cfg_pkt_padding.min_size().convert_to<unsigned>()) << 8) | tmp_val);
 }
 dpr_csr.cfg_pkt_padding.write();
#endif


 dpp_csr.cfg_error_mask.err_max_active_hdrs(1);
 dpp_csr.cfg_error_mask.write();

 dpp_csr.cfg_interrupt_mask.err_max_active_hdrs(1);
 dpp_csr.cfg_interrupt_mask.write();

 dpr_csr.cfg_interrupt_mask.err_crc_mask_offset_gt_pkt_size(1);
 dpr_csr.cfg_interrupt_mask.write();

 dpr_csr.int_reg1.all(0xffffffff);
 dpr_csr.int_reg1.write();
 dpr_csr.int_reg1.all(0x0);  //Write 0s in shadow value

 //Dummy read to flush
 dpr_csr.cfg_global.read();

 PLOG_API_MSG("DPA", "After load_from_cfg\n");
}


void cap_dpa_eos(int chip_id, int inst_id) {
   cap_dpa_eos_cnt(chip_id, inst_id);
   cap_dpa_eos_int(chip_id, inst_id);
   cap_dpa_eos_sta(chip_id, inst_id);
}

void cap_dpa_eos_cnt(int chip_id, int inst_id) {

//TBD :: Enable these in fullchip after they are debugged in block
#ifdef MODULE_SIM
 cap_dpp_csr_t & dpp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpp_csr_t, chip_id, inst_id);
 cap_dpr_csr_t & dpr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpr_csr_t, chip_id, inst_id);
 string                 csr_name;

 int pen_ip_ecc_en = SKNOBS_GET("PEN_IP_MEM_ECC_CORRUPT_ENABLE", 0);
 int dpa_ecc_test = SKNOBS_GET("dpa_ecc_test", 0);
 int ignore_ecc_cnt = pen_ip_ecc_en || dpa_ecc_test;

 PLOG_API_MSG("DPA", "inside cap_ppa_eos_cnt \n");

 cap_dppstats_csr_CNT_dpp_t CNT_dpp_exp;
 cap_dppstats_csr_CNT_dpp_t CNT_dpp_actual;

 CNT_dpp_exp.all(dpp_csr.stats.CNT_dpp.all());

 dpp_csr.stats.CNT_dpp.read();
 CNT_dpp_actual.all(dpp_csr.stats.CNT_dpp.all());

 CNT_dpp_exp.dpr_hdrfld_select_arr_vld(CNT_dpp_actual.dpr_hdrfld_select_arr_vld());
 CNT_dpp_exp.dpr_csum_crc_bundle(CNT_dpp_actual.dpr_csum_crc_bundle());

 if(CNT_dpp_exp.all() != CNT_dpp_actual.all()) {
    PLOG_ERR("read_compare : DPP CNT_dpp " << " exp: 0x" << hex << CNT_dpp_exp.all() << " actual: 0x" << CNT_dpp_actual.all() << dec << endl);
    PLOG_API_MSG("DPA", " CNT_dpp_exp is : \n");
    CNT_dpp_exp.show();
    PLOG_API_MSG("DPA", " CNT_dpp_actual is : \n");
    CNT_dpp_actual.show();
 }
 dpp_csr.stats.CNT_dpp.all(CNT_dpp_actual.all());
 
 cap_dprstats_csr_CNT_interface_t CNT_interface_exp;
 cap_dprstats_csr_CNT_interface_t CNT_interface_actual;

 CNT_interface_exp.all(dpr_csr.stats.CNT_interface.all());

 dpr_csr.stats.CNT_interface.read();
 CNT_interface_actual.all(dpr_csr.stats.CNT_interface.all());

 CNT_interface_exp.dpp_csum_crc_vld(CNT_interface_actual.dpp_csum_crc_vld());
 CNT_interface_exp.dpp_frame_vld(CNT_interface_actual.dpp_frame_vld());
 CNT_interface_exp.pkt_drop(CNT_interface_actual.pkt_drop());

 if(CNT_interface_exp.all() != CNT_interface_actual.all()) {
    PLOG_ERR("read_compare : DPR CNT_interface " << " exp: 0x" << hex << CNT_interface_exp.all() << " actual: 0x" << CNT_interface_actual.all() << dec << endl);
    PLOG_API_MSG("DPA", " CNT_interface_exp is : \n");
    CNT_interface_exp.show();
    PLOG_API_MSG("DPA", " CNT_interface_actual is : \n");
    CNT_interface_actual.show();
 }
 dpr_csr.stats.CNT_interface.all(CNT_interface_actual.all());
 

 for(auto itr: dpp_csr.get_children_prefix("cnt_")) {
     csr_name = itr->get_name();
     if( (csr_name.find("CNT_dpp_hdrfld_sel_lines") == string::npos)
      && (csr_name.find("CNT_dpp_spare") == string::npos)
      && (csr_name.find("dpp_dpr") == string::npos)
      && (csr_name.find("CNT_dpp_phv_no_data_drop_drop") == string::npos)
        ) {
       PLOG_API_MSG("DPA", "read_compare " << itr->get_name() << endl);
       itr->read_compare();
     }
 }
    
 for(auto itr: dpr_csr.get_children_prefix("cnt_")) {
     csr_name = itr->get_name();
     if( (csr_name.find("CNT_dpr_crc_rw") == string::npos) 
      && (csr_name.find("CNT_dpr_csum_rw_") == string::npos)
      && (csr_name.find("CNT_dpr_spare") == string::npos)
      && (csr_name.find("CNT_drop") == string::npos)
      && (csr_name.find("CNT_dpr_phv_drop") == string::npos)
      && (csr_name.find("CNT_dpr_padded") == string::npos)
      && (csr_name.find("CNT_dpr_pktout_phv_drop") == string::npos)
      && ( ignore_ecc_cnt == 0 || (csr_name.find("CNT_ecc_err") == string::npos))
       ) {
       PLOG_API_MSG("DPA", "read_compare " << itr->get_name() << endl);
       itr->read_compare();
     } else {
     }
 }
#endif

}

void cap_dpa_eos_int(int chip_id, int inst_id) {

//TBD :: Enable these in fullchip after they are debugged in block
#ifdef MODULE_SIM
 cap_dpp_csr_t & dpp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpp_csr_t, chip_id, inst_id);
 cap_dpr_csr_t & dpr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpr_csr_t, chip_id, inst_id);
 string    type_name;

 PLOG_API_MSG("DPA", "inside cap_ppa_eos_int \n");

 int skip_dpa_eos_int = SKNOBS_GET("skip_dpa_eos_int", 0);
 if(skip_dpa_eos_int) {
   return;
 }

 cpp_int  expd_int_val;
 cpp_int  rcvd_int_val;

 dpp_csr.int_reg1.intreg.err_min_pkt_size_interrupt(0);
 expd_int_val = dpp_csr.int_reg1.intreg.all();
 dpp_csr.int_reg1.intreg.read();
 dpp_csr.int_reg1.intreg.err_min_pkt_size_interrupt(0);
 rcvd_int_val = dpp_csr.int_reg1.intreg.all();
 if(expd_int_val != rcvd_int_val) {
   PLOG_ERR(" read_compare : dpp_csr.int_reg1 : exp : 0x" << hex << expd_int_val << " actual : 0x" << rcvd_int_val << endl)
   dpp_csr.int_reg1.intreg.show();
 }

 for(auto itr: dpp_csr.get_children_prefix("intreg")) {
     type_name = itr->get_hier_path();
     if( (type_name.find("intgrp_status_t") == string::npos)
        && (type_name.find("int_srams_ecc") == string::npos)
        && (type_name.find("int_reg1") == string::npos)
       ) {
       PLOG_API_MSG("DPA", "read_compare " << type_name << " before read " << endl);
       itr->show();
       itr->read_compare();
       PLOG_API_MSG("DPA", "read_compare " << type_name << " after read " << endl);
       itr->show();
     } else {
       itr->read();
       PLOG_API_MSG("DPA", "read " << type_name << " after read " << endl);
       itr->show();
     }
 }
    
 for(auto itr: dpr_csr.get_children_prefix("intreg")) {
     type_name = itr->get_hier_path();

     if( (type_name.find("intreg_status_t") == string::npos)
      && (type_name.find("int_srams_ecc") == string::npos)
      && (type_name.find("int_reg1") == string::npos)
      && (type_name.find("int_reg2") == string::npos)
      // && (type_name.find("int_spare") == string::npos)
       ) {
       PLOG_API_MSG("DPA", "read_compare " << type_name << " before read " << endl);
       itr->show();
       itr->read_compare();
       PLOG_API_MSG("DPA", "read_compare " << type_name << " after read " << endl);
       itr->show();
     } else {
       itr->read();
       PLOG_API_MSG("DPA", "read " << type_name << " after read " << endl);
       itr->show();
     }
 }
#endif

}
void cap_dpa_eos_sta(int chip_id, int inst_id) {
   cap_dpp_csr_t & dpp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpp_csr_t, chip_id, inst_id);
   cap_dpr_csr_t & dpr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpr_csr_t, chip_id, inst_id);

   dpp_csr.stats.sta_spare.read();
   if( (dpp_csr.stats.sta_spare.all() & 0xffffffef) != 0x5) { // We might need to mask bit 4 that is the capture engaged
      PLOG_ERR("cap_dpa_eos_sta : dpp_csr.stats.sta_spare : 0x" << hex << dpp_csr.stats.sta_spare.all() << " is not eaul to 0x5 " << dec << endl);
   }
   dpp_csr.stats.sta_global.read();
   if(dpp_csr.stats.sta_global.all() != 0x79baa) {
      PLOG_ERR("cap_dpa_eos_sta : dpp_csr.stats.sta_global : 0x" << hex << dpp_csr.stats.sta_global.all() << " is not eaul to 0x79baa " << dec << endl);
   }

   dpr_csr.stats.sta_fifo.read();
   if(dpr_csr.stats.sta_fifo.all() != 0x555) {
      PLOG_ERR("cap_dpa_eos_sta : dpr_csr.stats.sta_fifo : 0x" << hex << dpr_csr.stats.sta_fifo.all() << " is not eaul to 0x555 " << dec << endl);
   }

   dpr_csr.stats.sta_flop_fifo.read();
   if(dpr_csr.stats.sta_flop_fifo.all() != 0x15555555555ull)  {
      PLOG_ERR("cap_dpa_eos_sta : dpr_csr.stats.sta_flop_fifo : 0x" << hex << dpr_csr.stats.sta_flop_fifo.all() << " is not eaul to 0x15555555555ull " << dec << endl);
   }

   dpr_csr.stats.sta_spare.read();
   if( (dpr_csr.stats.sta_spare.all()  & 0xfffff7ff) != 0x345) { // We might need to mask bit 11 that is the capture engaged
      PLOG_ERR("cap_dpa_eos_sta : dpr_csr.stats.sta_spare : 0x" << hex << dpr_csr.stats.sta_spare.all() << " is not eaul to 0xfffff7ff " << dec << endl);
   }
}

void cap_dpa_check_progr_sanity(int chip_id, int inst_id) {

  PLOG_API_MSG("DPA[" << inst_id << "]", "running programming sanity\n");
  //TBD :: Implement progr_sanity
  PLOG_API_MSG("DPA[" << inst_id << "]", "Finished running programming sanity\n");

}

void cap_dpa_disable_backdoor_paths(int chip_id, int inst_id) {
  // Nothing to do here..
}

//API for block level.
void cap_dpa_basic_config(int chip_id, int inst_id) {

 cap_dpp_csr_t & dpp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpp_csr_t, chip_id, inst_id);
 cap_dpr_csr_t & dpr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpr_csr_t, chip_id, inst_id);


////////////////////////////////////////////////////////////////////////////////////////
//basic programming with 3 headers ethernet, vlan, payload.
//expect 3 bit in the hdrvld in that order.
//expand to 3 headerfields [0,1,2]
//ethernet_hdr: size(12) from cfg , value from packet, offset(0) from config
//vlan_hdr: size(4) from cfg, value from PHV,   
//payload: size from pkt_len - ohi_offset, value from packet, offset from packet

dpp_csr.cfg_ohi_payload.show();
dpp_csr.cfg_ohi_payload.ohi_slot_payload_ptr_bm(4);
dpp_csr.cfg_ohi_payload.write();
dpp_csr.cfg_ohi_payload.show();


//intrinsic header bit 0
dpp_csr.hdr.cfg_hdr_info[0].show();
dpp_csr.hdr.cfg_hdr_info[0].fld_start(0);
dpp_csr.hdr.cfg_hdr_info[0].fld_end(255);
dpp_csr.hdr.cfg_hdr_info[0].write();
dpp_csr.hdr.cfg_hdr_info[0].show();

dpp_csr.hdrfld.cfg_hdrfld_info[0].show();
dpp_csr.hdrfld.cfg_hdrfld_info[0].size_sel(0);//config
dpp_csr.hdrfld.cfg_hdrfld_info[0].size_val(8);//config
dpp_csr.hdrfld.cfg_hdrfld_info[0].write();
dpp_csr.hdrfld.cfg_hdrfld_info[0].show();

//ethernet header bit 1
dpp_csr.hdr.cfg_hdr_info[1].show();
dpp_csr.hdr.cfg_hdr_info[1].fld_start(1);
dpp_csr.hdr.cfg_hdr_info[1].fld_end(254);
dpp_csr.hdr.cfg_hdr_info[1].write();
dpp_csr.hdr.cfg_hdr_info[1].show();

dpp_csr.hdrfld.cfg_hdrfld_info[1].show();
dpp_csr.hdrfld.cfg_hdrfld_info[1].size_sel(0);//config
dpp_csr.hdrfld.cfg_hdrfld_info[1].size_val(14);//config including ethertype
dpp_csr.hdrfld.cfg_hdrfld_info[1].write();
dpp_csr.hdrfld.cfg_hdrfld_info[1].show();

//vlan header bit 2
dpp_csr.hdr.cfg_hdr_info[2].show();
dpp_csr.hdr.cfg_hdr_info[2].fld_start(2);
dpp_csr.hdr.cfg_hdr_info[2].fld_end(253);
dpp_csr.hdr.cfg_hdr_info[2].write();
dpp_csr.hdr.cfg_hdr_info[2].show();

dpp_csr.hdrfld.cfg_hdrfld_info[2].show();
dpp_csr.hdrfld.cfg_hdrfld_info[2].size_sel(0);//config
dpp_csr.hdrfld.cfg_hdrfld_info[2].size_val(4);//config
dpp_csr.hdrfld.cfg_hdrfld_info[2].write();
dpp_csr.hdrfld.cfg_hdrfld_info[2].show();

//IP Header 
dpp_csr.hdr.cfg_hdr_info[3].show();
dpp_csr.hdr.cfg_hdr_info[3].fld_start(3);
dpp_csr.hdr.cfg_hdr_info[3].fld_end(251); //IP header + IP options
dpp_csr.hdr.cfg_hdr_info[3].write();
dpp_csr.hdr.cfg_hdr_info[3].show();

dpp_csr.hdrfld.cfg_hdrfld_info[3].show();
dpp_csr.hdrfld.cfg_hdrfld_info[3].size_sel(0);//config
dpp_csr.hdrfld.cfg_hdrfld_info[3].size_val(20);//config
dpp_csr.hdrfld.cfg_hdrfld_info[3].write();
dpp_csr.hdrfld.cfg_hdrfld_info[3].show();


//IP Header options

dpp_csr.hdrfld.cfg_hdrfld_info[4].show();
dpp_csr.hdrfld.cfg_hdrfld_info[4].size_sel(2);//ohi
dpp_csr.hdrfld.cfg_hdrfld_info[4].size_val(4);//ohi slot
dpp_csr.hdrfld.cfg_hdrfld_info[4].write();
dpp_csr.hdrfld.cfg_hdrfld_info[4].show();


//payload header bit 4
dpp_csr.hdr.cfg_hdr_info[4].show();
dpp_csr.hdr.cfg_hdr_info[4].fld_start(255);
dpp_csr.hdr.cfg_hdr_info[4].fld_end(0);
dpp_csr.hdr.cfg_hdr_info[4].write();
dpp_csr.hdr.cfg_hdr_info[4].show();

dpp_csr.hdrfld.cfg_hdrfld_info[255].show();
dpp_csr.hdrfld.cfg_hdrfld_info[255].size_sel(2);//ohi
dpp_csr.hdrfld.cfg_hdrfld_info[255].size_val(2);//ohi_slot with payload offset
dpp_csr.hdrfld.cfg_hdrfld_info[255].write();
dpp_csr.hdrfld.cfg_hdrfld_info[255].show();

//trailer Header bit 5
dpp_csr.hdr.cfg_hdr_info[5].show();
dpp_csr.hdr.cfg_hdr_info[5].fld_start(6);
dpp_csr.hdr.cfg_hdr_info[5].fld_end(249);
dpp_csr.hdr.cfg_hdr_info[5].write();
dpp_csr.hdr.cfg_hdr_info[5].show();

dpp_csr.hdrfld.cfg_hdrfld_info[6].show();
dpp_csr.hdrfld.cfg_hdrfld_info[6].size_sel(1);//phv
dpp_csr.hdrfld.cfg_hdrfld_info[6].size_val(0);//phv slot
dpp_csr.hdrfld.cfg_hdrfld_info[6].write();
dpp_csr.hdrfld.cfg_hdrfld_info[6].show();

//intrinsic header
dpr_csr.hdrfld.cfg_hdrfld_info[0].show();
dpr_csr.hdrfld.cfg_hdrfld_info[0].source_sel(1);//pkt
dpr_csr.hdrfld.cfg_hdrfld_info[0].source_oft(108);//start of packet dfor the time being
dpr_csr.hdrfld.cfg_hdrfld_info[0].write();
dpr_csr.hdrfld.cfg_hdrfld_info[0].show();



//ethernet header
dpr_csr.hdrfld.cfg_hdrfld_info[1].show();
dpr_csr.hdrfld.cfg_hdrfld_info[1].source_sel(2);//pkt
dpr_csr.hdrfld.cfg_hdrfld_info[1].source_oft(0);//start of packet dfor the time being
dpr_csr.hdrfld.cfg_hdrfld_info[1].write();
dpr_csr.hdrfld.cfg_hdrfld_info[1].show();

//vlan header

dpr_csr.cfg.cfg_static_field[2].show();
dpr_csr.cfg.cfg_static_field[2].data(0x81);
dpr_csr.cfg.cfg_static_field[2].write();
dpr_csr.cfg.cfg_static_field[2].show();

dpr_csr.cfg.cfg_static_field[3].show();
dpr_csr.cfg.cfg_static_field[3].data(0x00);
dpr_csr.cfg.cfg_static_field[3].write();
dpr_csr.cfg.cfg_static_field[3].show();


dpr_csr.hdrfld.cfg_hdrfld_info[2].show();
dpr_csr.hdrfld.cfg_hdrfld_info[2].source_sel(1);//phv
dpr_csr.hdrfld.cfg_hdrfld_info[2].source_oft(100);//4 byted from phv slot start
dpr_csr.hdrfld.cfg_hdrfld_info[2].write();
dpr_csr.hdrfld.cfg_hdrfld_info[2].show();

//IP header

dpr_csr.hdrfld.cfg_hdrfld_info[3].show();
dpr_csr.hdrfld.cfg_hdrfld_info[3].source_sel(2);//pkt
dpr_csr.hdrfld.cfg_hdrfld_info[3].source_oft(3);//ohi_slot with IP header offset
dpr_csr.hdrfld.cfg_hdrfld_info[3].write();
dpr_csr.hdrfld.cfg_hdrfld_info[3].show();


dpr_csr.hdrfld.cfg_hdrfld_info[4].show();
dpr_csr.hdrfld.cfg_hdrfld_info[4].source_sel(2);//pkt
dpr_csr.hdrfld.cfg_hdrfld_info[4].source_oft(5);//ohi_slot with payload offset
dpr_csr.hdrfld.cfg_hdrfld_info[4].write();
dpr_csr.hdrfld.cfg_hdrfld_info[4].show();

//payload header

dpr_csr.hdrfld.cfg_hdrfld_info[255].show();
dpr_csr.hdrfld.cfg_hdrfld_info[255].source_sel(2);//pkt
dpr_csr.hdrfld.cfg_hdrfld_info[255].source_oft(2);//ohi_slot with ip heaer offset
dpr_csr.hdrfld.cfg_hdrfld_info[255].write();
dpr_csr.hdrfld.cfg_hdrfld_info[255].show();

//trailer header
dpr_csr.hdrfld.cfg_hdrfld_info[6].show();
dpr_csr.hdrfld.cfg_hdrfld_info[6].source_sel(1);//phv
dpr_csr.hdrfld.cfg_hdrfld_info[6].source_oft(104);//4 byted from phv slot start
dpr_csr.hdrfld.cfg_hdrfld_info[6].write();
dpr_csr.hdrfld.cfg_hdrfld_info[6].show();



//Dummy read to flush
dpp_csr.cfg_global.read();
}

void cap_dpa_csum_config(int chip_id, int inst_id) {

 cap_dpp_csr_t & dpp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpp_csr_t, chip_id, inst_id);
 //cap_dpr_csr_t & dpr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpr_csr_t, chip_id, inst_id);
 dpp_csr.cfg_global.read();
//csum configuration 
//unit 0 for IPV4
//unit 1 for TCP/UDP/SCTP

//header 0 is ipv4 
 dpp_csr.csum.cfg_csum_hdrs[0].show();
 dpp_csr.csum.cfg_csum_hdrs[0].hdr_num(32); //to be filled with ip 
 dpp_csr.csum.cfg_csum_hdrs[0].csum_vld(1);
 dpp_csr.csum.cfg_csum_hdrs[0].csum_unit(0);
 dpp_csr.csum.cfg_csum_hdrs[0].csum_profile(0);
 dpp_csr.csum.cfg_csum_hdrs[0].hdrfld_start(17); //to be filled
 dpp_csr.csum.cfg_csum_hdrs[0].hdrfld_end(24); //to be filled
 dpp_csr.csum.cfg_csum_hdrs[0].phdr_vld(1);//to be filled
 dpp_csr.csum.cfg_csum_hdrs[0].phdr_unit(1);//to be filled
 dpp_csr.csum.cfg_csum_hdrs[0].phdr_profile(0);//to be filled
 dpp_csr.csum.cfg_csum_hdrs[0].write();
 dpp_csr.csum.cfg_csum_hdrs[0].show();
 /*
 //header 1 is ipv6
 dpp_csr.csum.cfg_csum_hdrs[0].show();
 dpp_csr.csum.cfg_csum_hdrs[0].hdr_num(120); //to be filled with ip 
 dpp_csr.csum.cfg_csum_hdrs[0].csum_vld(0);
 dpp_csr.csum.cfg_csum_hdrs[0].csum_unit(0);
 dpp_csr.csum.cfg_csum_hdrs[0].csum_profile(0);
 dpp_csr.csum.cfg_csum_hdrs[0].hdrfld_start(1); //to be filled
 dpp_csr.csum.cfg_csum_hdrs[0].hdrfld_end(1); //to be filled
 dpp_csr.csum.cfg_csum_hdrs[0].phdr_vld(1);//to be filled
 dpp_csr.csum.cfg_csum_hdrs[0].phdr_unit(1);//to be filled
 dpp_csr.csum.cfg_csum_hdrs[0].phdr_profile(1);//to be filled
 dpp_csr.csum.cfg_csum_hdrs[0].write();
 dpp_csr.csum.cfg_csum_hdrs[0].show();
 */

 //header 2 is tcp
 dpp_csr.csum.cfg_csum_hdrs[2].show();
 dpp_csr.csum.cfg_csum_hdrs[2].hdr_num(75); //to be filled with ip 
 dpp_csr.csum.cfg_csum_hdrs[2].csum_vld(1);
 dpp_csr.csum.cfg_csum_hdrs[2].csum_unit(1);
 dpp_csr.csum.cfg_csum_hdrs[2].csum_profile(1);
 dpp_csr.csum.cfg_csum_hdrs[2].hdrfld_start(76); //to be filled
 dpp_csr.csum.cfg_csum_hdrs[2].hdrfld_end(78); //to be filled
 dpp_csr.csum.cfg_csum_hdrs[2].phdr_vld(0);//to be filled
 dpp_csr.csum.cfg_csum_hdrs[2].phdr_unit(0);//to be filled
 dpp_csr.csum.cfg_csum_hdrs[2].phdr_profile(0);//to be filled
 dpp_csr.csum.cfg_csum_hdrs[2].write();
 dpp_csr.csum.cfg_csum_hdrs[2].show();
 /*
 //header 3 is udp
 dpp_csr.csum.cfg_csum_hdrs[0].show();
 dpp_csr.csum.cfg_csum_hdrs[0].hdr_num(120); //to be filled with ip 
 dpp_csr.csum.cfg_csum_hdrs[0].csum_vld(0);
 dpp_csr.csum.cfg_csum_hdrs[0].csum_unit(1);
 dpp_csr.csum.cfg_csum_hdrs[0].csum_profile(2);
 dpp_csr.csum.cfg_csum_hdrs[0].hdrfld_start(1); //to be filled
 dpp_csr.csum.cfg_csum_hdrs[0].hdrfld_end(1); //to be filled
 dpp_csr.csum.cfg_csum_hdrs[0].phdr_vld(0);//to be filled
 dpp_csr.csum.cfg_csum_hdrs[0].phdr_unit(0);//to be filled
 dpp_csr.csum.cfg_csum_hdrs[0].phdr_profile(0);//to be filled
 dpp_csr.csum.cfg_csum_hdrs[0].write();
 dpp_csr.csum.cfg_csum_hdrs[0].show();
 */

 //csum profile 0 is for IPv4
 dpp_csr.csum.cfg_csum_profile[0].show();
 dpp_csr.csum.cfg_csum_profile[0].loc_adj(10);
 dpp_csr.csum.cfg_csum_profile[0].write();
 dpp_csr.csum.cfg_csum_profile[0].show();

 
 //csum profile 1 is for tcp
 dpp_csr.csum.cfg_csum_profile[1].show();
 dpp_csr.csum.cfg_csum_profile[1].use_phv_len(1);
 dpp_csr.csum.cfg_csum_profile[1].phv_len_sel(0);
 dpp_csr.csum.cfg_csum_profile[1].len_mask(0xffff);
 dpp_csr.csum.cfg_csum_profile[1].len_shift_left(0);
 dpp_csr.csum.cfg_csum_profile[1].len_shift_val(0);
 dpp_csr.csum.cfg_csum_profile[1].start_adj(0);
 dpp_csr.csum.cfg_csum_profile[1].end_adj(0);
 dpp_csr.csum.cfg_csum_profile[1].loc_adj(16);
 dpp_csr.csum.cfg_csum_profile[1].add_len(1);
 dpp_csr.csum.cfg_csum_profile[1].write();
 dpp_csr.csum.cfg_csum_profile[1].show();

 //phdr profile 0 is ipv4
 dpp_csr.csum.cfg_csum_phdr_profile[0].show();
 dpp_csr.csum.cfg_csum_phdr_profile[0].fld_en_0(1);
 dpp_csr.csum.cfg_csum_phdr_profile[0].fld_start_0(12);
 dpp_csr.csum.cfg_csum_phdr_profile[0].fld_end_0(20);
 dpp_csr.csum.cfg_csum_phdr_profile[0].fld_en_1(1);
 dpp_csr.csum.cfg_csum_phdr_profile[0].fld_start_1(9);
 dpp_csr.csum.cfg_csum_phdr_profile[0].fld_end_1(10);
 dpp_csr.csum.cfg_csum_phdr_profile[0].fld_align_1(1);
 dpp_csr.csum.cfg_csum_phdr_profile[0].write();
 dpp_csr.csum.cfg_csum_phdr_profile[0].show();

dpp_csr.cfg_global.read();


 /*

 //csum profile 2 is for udp
 dpp_csr.csum.cfg_csum_profile[1].show();
 dpp_csr.csum.cfg_csum_profile[1].use_phv_len(1);
 dpp_csr.csum.cfg_csum_profile[1].phv_len_sel(1);
 dpp_csr.csum.cfg_csum_profile[1].len_mask(0);
 dpp_csr.csum.cfg_csum_profile[1].len_shift_left(0);
 dpp_csr.csum.cfg_csum_profile[1].len_shift_val(0);
 dpp_csr.csum.cfg_csum_profile[1].start_adj(0);
 dpp_csr.csum.cfg_csum_profile[1].end_adj(0);
 dpp_csr.csum.cfg_csum_profile[1].loc_adj(6);
 dpp_csr.csum.cfg_csum_profile[1].add_len(1);
 dpp_csr.csum.cfg_csum_profile[1].write();
 dpp_csr.csum.cfg_csum_profile[1].show();
 */



  cap_ppa_csr_t & ppa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_ppa_csr_t, chip_id, inst_id);
   cap_ppa_decoders_ppa_lkp_sram_entry_t ppa_sram;
   ppa_sram.all((pu_cpp_int< 764 >) ppa_csr.dhs_bndl0_state_lkp_sram.entry[3].data());

          //mux_idx[3] : exteact pkt byte[2, 3] for ip_total length
          ppa_sram.mux_idx[3].sel(0);
          ppa_sram.mux_idx[3].idx(16);

          //mux_inst[2]: mux_pkt[3] - 20.
          ppa_sram.mux_inst[2].sel(0);
          ppa_sram.mux_inst[2].muxsel(3);
          ppa_sram.mux_inst[2].mask_val(0xffff);
          ppa_sram.mux_inst[2].addsub(0);        //subtract
          ppa_sram.mux_inst[2].addsub_val(20);   //subtract by 20

          //meta_inst[1]; phv[65] = mux_inst[2][7:0];
          ppa_sram.meta_inst[1].sel(3);   //replace with mux_inst_data
          ppa_sram.meta_inst[1].phv_idx(65);
          ppa_sram.meta_inst[1].val(2);
   ppa_csr.dhs_bndl0_state_lkp_sram.entry[3].data((pu_cpp_int< 764 >) ppa_sram.all());




}




void cap_dpa_crc_config(int chip_id, int inst_id) {

 cap_dpp_csr_t & dpp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpp_csr_t, chip_id, inst_id);
 //cap_dpr_csr_t & dpr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpr_csr_t, chip_id, inst_id);
 dpp_csr.cfg_global.read();
//crc configuration 
//unit 0 for ethernet csum

//header 0 is ethernet 
 dpp_csr.csum.cfg_crc_hdrs[0].show();
 dpp_csr.csum.cfg_crc_hdrs[0].hdr_num(3); //to be filled with ip 
 dpp_csr.csum.cfg_crc_hdrs[0].crc_vld(1);
 dpp_csr.csum.cfg_crc_hdrs[0].crc_unit(0);
 dpp_csr.csum.cfg_crc_hdrs[0].crc_profile(0);
 dpp_csr.csum.cfg_crc_hdrs[0].hdrfld_start(12); //to be filled
 dpp_csr.csum.cfg_crc_hdrs[0].hdrfld_end(254); //to be filled
 dpp_csr.csum.cfg_crc_hdrs[0].write();
 dpp_csr.csum.cfg_crc_hdrs[0].show();

 //crc profile 0 is for ethernet
 dpp_csr.csum.cfg_crc_profile[0].show();
 dpp_csr.csum.cfg_crc_profile[0].loc_adj(2);
 dpp_csr.csum.cfg_crc_profile[0].write();
 dpp_csr.csum.cfg_crc_profile[0].show();



}


void cap_dpp_csr_set_hdl_path(int chip_id, int inst_id, string path) {
  cap_dpp_csr_t & dpp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpp_csr_t, chip_id, inst_id);
 
  // id : 0 is given to csr instance.
  // id : 1 is given to sub_csr modules
  // id : 2 is given to memories

  dpp_csr.set_csr_inst_path(0, (path + ".csr_inst0"));
  
  dpp_csr.hdr.set_csr_inst_path(1,    path + ".hdrcsr_inst0");
  dpp_csr.hdrfld.set_csr_inst_path(1, path + ".hdrfldcsr_inst0");
  dpp_csr.csum.set_csr_inst_path(1,   path + ".csumcsr_inst0");
  dpp_csr.stats.set_csr_inst_path(1,  path + ".statscsr_inst0");
  dpp_csr.mem.set_csr_inst_path(1,    path + ".memcsr_inst0");
  
  dpp_csr.mem.dhs_dpp_phv_fifo_sram.set_csr_inst_path(2, path + ".dpp_phv_ff_inst0.fifo_mem_inst0.mem.mem");
  dpp_csr.mem.dhs_dpp_ohi_fifo_sram.set_csr_inst_path(2, path + ".dpp_ohi_ff_inst0.fifo_mem_inst0.mem.mem");

} 
 
void cap_dpr_csr_set_hdl_path(int chip_id, int inst_id, string path) {
 cap_dpr_csr_t & dpr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpr_csr_t, chip_id, inst_id);

 // id : 0 is given to csr instance.
 // id : 1 is given to sub_csr modules
 // id : 2 is given to memories
 dpr_csr.set_csr_inst_path(0, (path + ".csr"), 0);
 dpr_csr.set_csr_inst_path(1, path, 0);
 dpr_csr.set_csr_inst_path(2, path, 0);

 dpr_csr.hdrfld.set_csr_inst_path(1, "hdrfldcsr_inst0");
 dpr_csr.cfg.set_csr_inst_path(1, "cfgcsr_inst0");
 dpr_csr.stats.set_csr_inst_path(1, "statscsr_inst0");
 dpr_csr.mem.set_csr_inst_path(1, "memcsr_inst0");

 dpr_csr.mem.dhs_dpr_pktin_fifo_sram.set_csr_inst_path(2, "dpr_pktin_ff_inst0.fifo_mem_inst0.mem.mem");
 dpr_csr.mem.dhs_dpr_csum_fifo_sram.set_csr_inst_path(2, "dpa_csum_ff_inst0.fifo_mem_inst0.mem.mem");
 dpr_csr.mem.dhs_dpr_phv_fifo_sram.set_csr_inst_path(2, "dpr_phv_ff_inst0.fifo_mem_inst0.mem.mem");
 dpr_csr.mem.dhs_dpr_ohi_fifo_sram.set_csr_inst_path(2, "dpr_ohi_ff_inst0.fifo_mem_inst0.mem.mem");
 dpr_csr.mem.dhs_dpr_ptr_fifo_sram.set_csr_inst_path(2, "dpr_ptr_ff_inst0.fifo_mem_inst0.mem.mem");
 dpr_csr.mem.dhs_dpr_pktout_fifo_sram.set_csr_inst_path(2, "dpr_pktout_ff_inst0.fifo_mem_inst0.mem.mem");

}

//This api writes random values to CFG & CNT type registers.
void cap_dpa_randomize_csr(int chip_id, int inst_id) {
 cap_dpp_csr_t & dpp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpp_csr_t, chip_id, inst_id);
 cap_dpr_csr_t & dpr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpr_csr_t, chip_id, inst_id);

 PLOG_MSG( "inside cap_dpa_randomize_csr " << endl);

 string                 csr_name;
 pen_csr_base *         csr_ptr;
 vector<pen_csr_base *> csr_cfg;
 vector<pen_csr_base *> csr_cfg_tmp  = dpp_csr.get_children_prefix("cfg_");
 vector<pen_csr_base *> tmp_csr_vec = dpr_csr.get_children_prefix("cfg_");
 csr_cfg_tmp.insert(csr_cfg_tmp.end(),  tmp_csr_vec.begin(), tmp_csr_vec.end());

 vector<pen_csr_base *> csr_cnt  = dpp_csr.get_children_prefix("cnt_");
 tmp_csr_vec.clear();
 tmp_csr_vec = dpr_csr.get_children_prefix("cnt_");
 csr_cnt.insert(csr_cnt.end(),  tmp_csr_vec.begin(), tmp_csr_vec.end());


 //Do not randomize cfg_global* registers as they have some timing/RTL sensitive fields.
 for (auto i: csr_cfg_tmp) {
     csr_name = i->get_name();
     if(  csr_name.find("cfg_global") == string::npos 
         //Do not randomize these regs as they enable bist_run & toggle ecc configs.
       && csr_name.find("cfg_dpp_phv_fifo") == string::npos
       && csr_name.find("cfg_dpp_ohi_fifo") == string::npos
       && csr_name.find("cfg_dpr_pktin_fifo") == string::npos
       && csr_name.find("cfg_dpr_csum_fifo") == string::npos
       && csr_name.find("cfg_dpr_phv_fifo") == string::npos
       && csr_name.find("cfg_dpr_ohi_fifo") == string::npos
       && csr_name.find("cfg_dpr_ptr_fifo") == string::npos
       && csr_name.find("cfg_dpr_pktout_fifo") == string::npos
        ) {
        csr_cfg.push_back(i);
     } else {
        //PLOG_MSG(" do_main csr_cfg : removing item " << csr_name << endl)
     }
 }
 csr_cfg_tmp.clear();
            
 for(unsigned idx=0; idx<csr_cfg.size(); idx++) {
    csr_ptr   = csr_cfg[idx];
    //PLOG_MSG( "randomizing cfg : " << csr_ptr->get_name() << endl);
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
 dpr_csr.cfg_global.read();

} //cap_dpa_randomize_csr

void cap_dpa_bist_test(int chip_id, int inst_id) {
 cap_dpp_csr_t & dpp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpp_csr_t, chip_id, inst_id);
 cap_dpr_csr_t & dpr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpr_csr_t, chip_id, inst_id);
 int           loop_cnt;

 PLOG_API_MSG("DPA", "inside cap_dpa_bist_test \n");

 // CFG_BIST RUN Write
  dpp_csr.mem.cfg_dpp_phv_fifo.bist_run(1);
  dpp_csr.mem.cfg_dpp_ohi_fifo.bist_run(1);

  dpr_csr.mem.cfg_dpr_pktin_fifo.bist_run(1); 
  dpr_csr.mem.cfg_dpr_csum_fifo.bist_run(1);
  dpr_csr.mem.cfg_dpr_phv_fifo.bist_run(1);
  dpr_csr.mem.cfg_dpr_ohi_fifo.bist_run(1);
  dpr_csr.mem.cfg_dpr_ptr_fifo.bist_run(1);
  dpr_csr.mem.cfg_dpr_pktout_fifo.bist_run(1);

  dpp_csr.mem.cfg_dpp_phv_fifo.write();
  dpp_csr.mem.cfg_dpp_ohi_fifo.write();

  dpr_csr.mem.cfg_dpr_pktin_fifo.write();
  dpr_csr.mem.cfg_dpr_csum_fifo.write();
  dpr_csr.mem.cfg_dpr_phv_fifo.write();
  dpr_csr.mem.cfg_dpr_ohi_fifo.write();
  dpr_csr.mem.cfg_dpr_ptr_fifo.write();
  dpr_csr.mem.cfg_dpr_pktout_fifo.write();


  loop_cnt = 0;
  while (loop_cnt < 10000) {
    dpp_csr.mem.sta_srams_ecc_dpp_phv_fifo.read();
    if ( dpp_csr.mem.sta_srams_ecc_dpp_phv_fifo.bist_done_fail() == 1 || dpp_csr.mem.sta_srams_ecc_dpp_phv_fifo.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 10000) {
     PLOG_ERR("FAIL : dpp[" << inst_id << "] : sta_srams_ecc_dpp_phv_fifo.bist_done not set for timeout cycles " << endl);
  } else if( dpp_csr.mem.sta_srams_ecc_dpp_phv_fifo.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : dpp[" << inst_id << "] : sta_srams_ecc_dpp_phv_fifo.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 10000) {
    dpp_csr.mem.sta_srams_ecc_dpp_ohi_fifo.read();
    if ( dpp_csr.mem.sta_srams_ecc_dpp_ohi_fifo.bist_done_fail() == 1 || dpp_csr.mem.sta_srams_ecc_dpp_ohi_fifo.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 10000) {
     PLOG_ERR("FAIL : dpp[" << inst_id << "] : sta_srams_ecc_dpp_ohi_fifo.bist_done not set for timeout cycles " << endl);
  } else if( dpp_csr.mem.sta_srams_ecc_dpp_ohi_fifo.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : dpp[" << inst_id << "] : sta_srams_ecc_dpp_ohi_fifo.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 10000) {
    dpr_csr.mem.sta_srams_ecc_dpr_pktin_fifo.read();
    if ( dpr_csr.mem.sta_srams_ecc_dpr_pktin_fifo.bist_done_fail() == 1 || dpr_csr.mem.sta_srams_ecc_dpr_pktin_fifo.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 10000) {
     PLOG_ERR("FAIL : dpr[" << inst_id << "] : sta_srams_ecc_dpr_pktin_fifo.bist_done not set for timeout cycles " << endl);
  } else if( dpr_csr.mem.sta_srams_ecc_dpr_pktin_fifo.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : dpr[" << inst_id << "] : sta_srams_ecc_dpr_pktin_fifo.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 10000) {
    dpr_csr.mem.sta_srams_ecc_dpr_csum_fifo.read();
    if ( dpr_csr.mem.sta_srams_ecc_dpr_csum_fifo.bist_done_fail() == 1 || dpr_csr.mem.sta_srams_ecc_dpr_csum_fifo.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 10000) {
     PLOG_ERR("FAIL : dpr[" << inst_id << "] : sta_srams_ecc_dpr_csum_fifo.bist_done not set for timeout cycles " << endl);
  } else if( dpr_csr.mem.sta_srams_ecc_dpr_csum_fifo.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : dpr[" << inst_id << "] : sta_srams_ecc_dpr_csum_fifo.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 10000) {
    dpr_csr.mem.sta_srams_ecc_dpr_phv_fifo.read();
    if ( dpr_csr.mem.sta_srams_ecc_dpr_phv_fifo.bist_done_fail() == 1 || dpr_csr.mem.sta_srams_ecc_dpr_phv_fifo.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 10000) {
     PLOG_ERR("FAIL : dpr[" << inst_id << "] : sta_srams_ecc_dpr_phv_fifo.bist_done not set for timeout cycles " << endl);
  } else if( dpr_csr.mem.sta_srams_ecc_dpr_phv_fifo.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : dpr[" << inst_id << "] : sta_srams_ecc_dpr_phv_fifo.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 10000) {
    dpr_csr.mem.sta_srams_ecc_dpr_ohi_fifo.read();
    if ( dpr_csr.mem.sta_srams_ecc_dpr_ohi_fifo.bist_done_fail() == 1 || dpr_csr.mem.sta_srams_ecc_dpr_ohi_fifo.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 10000) {
     PLOG_ERR("FAIL : dpr[" << inst_id << "] : sta_srams_ecc_dpr_ohi_fifo.bist_done not set for timeout cycles " << endl);
  } else if( dpr_csr.mem.sta_srams_ecc_dpr_ohi_fifo.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : dpr[" << inst_id << "] : sta_srams_ecc_dpr_ohi_fifo.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 10000) {
    dpr_csr.mem.sta_srams_ecc_dpr_ptr_fifo.read();
    if ( dpr_csr.mem.sta_srams_ecc_dpr_ptr_fifo.bist_done_fail() == 1 || dpr_csr.mem.sta_srams_ecc_dpr_ptr_fifo.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 10000) {
     PLOG_ERR("FAIL : dpr[" << inst_id << "] : sta_srams_ecc_dpr_ptr_fifo.bist_done not set for timeout cycles " << endl);
  } else if( dpr_csr.mem.sta_srams_ecc_dpr_ptr_fifo.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : dpr[" << inst_id << "] : sta_srams_ecc_dpr_ptr_fifo.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 10000) {
    dpr_csr.mem.sta_srams_ecc_dpr_pktout_fifo.read();
    if ( dpr_csr.mem.sta_srams_ecc_dpr_pktout_fifo.bist_done_fail() == 1 || dpr_csr.mem.sta_srams_ecc_dpr_pktout_fifo.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 10000) {
     PLOG_ERR("FAIL : dpr[" << inst_id << "] : sta_srams_ecc_dpr_pktout_fifo.bist_done not set for timeout cycles " << endl);
  } else if( dpr_csr.mem.sta_srams_ecc_dpr_pktout_fifo.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : dpr[" << inst_id << "] : sta_srams_ecc_dpr_pktout_fifo.bist_done_fail is set " << endl);
  }
     

 } //cap_dpa_bist_test

void cap_dpa_bist_test_start(int chip_id, int inst_id) {
 cap_dpp_csr_t & dpp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpp_csr_t, chip_id, inst_id);
 cap_dpr_csr_t & dpr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpr_csr_t, chip_id, inst_id);
 //int           loop_cnt;

 PLOG_API_MSG("DPA", "inside cap_dpa_bist_test_start \n");

 // CFG_BIST RUN Write
  dpp_csr.mem.cfg_dpp_phv_fifo.bist_run(1);
  dpp_csr.mem.cfg_dpp_ohi_fifo.bist_run(1);

  dpr_csr.mem.cfg_dpr_pktin_fifo.bist_run(1); 
  dpr_csr.mem.cfg_dpr_csum_fifo.bist_run(1);
  dpr_csr.mem.cfg_dpr_phv_fifo.bist_run(1);
  dpr_csr.mem.cfg_dpr_ohi_fifo.bist_run(1);
  dpr_csr.mem.cfg_dpr_ptr_fifo.bist_run(1);
  dpr_csr.mem.cfg_dpr_pktout_fifo.bist_run(1);

  dpp_csr.mem.cfg_dpp_phv_fifo.write();
  dpp_csr.mem.cfg_dpp_ohi_fifo.write();

  dpr_csr.mem.cfg_dpr_pktin_fifo.write();
  dpr_csr.mem.cfg_dpr_csum_fifo.write();
  dpr_csr.mem.cfg_dpr_phv_fifo.write();
  dpr_csr.mem.cfg_dpr_ohi_fifo.write();
  dpr_csr.mem.cfg_dpr_ptr_fifo.write();
  dpr_csr.mem.cfg_dpr_pktout_fifo.write();

 } //cap_dpa_bist_test_start


void cap_dpa_bist_test_clear(int chip_id, int inst_id) {
 cap_dpp_csr_t & dpp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpp_csr_t, chip_id, inst_id);
 cap_dpr_csr_t & dpr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpr_csr_t, chip_id, inst_id);
 //int           loop_cnt;

 PLOG_API_MSG("DPA", "inside cap_dpa_bist_test_clear \n");

 // CFG_BIST RUN Write
  dpp_csr.mem.cfg_dpp_phv_fifo.bist_run(0);
  dpp_csr.mem.cfg_dpp_ohi_fifo.bist_run(0);

  dpr_csr.mem.cfg_dpr_pktin_fifo.bist_run(0); 
  dpr_csr.mem.cfg_dpr_csum_fifo.bist_run(0);
  dpr_csr.mem.cfg_dpr_phv_fifo.bist_run(0);
  dpr_csr.mem.cfg_dpr_ohi_fifo.bist_run(0);
  dpr_csr.mem.cfg_dpr_ptr_fifo.bist_run(0);
  dpr_csr.mem.cfg_dpr_pktout_fifo.bist_run(0);

  dpp_csr.mem.cfg_dpp_phv_fifo.write();
  dpp_csr.mem.cfg_dpp_ohi_fifo.write();

  dpr_csr.mem.cfg_dpr_pktin_fifo.write();
  dpr_csr.mem.cfg_dpr_csum_fifo.write();
  dpr_csr.mem.cfg_dpr_phv_fifo.write();
  dpr_csr.mem.cfg_dpr_ohi_fifo.write();
  dpr_csr.mem.cfg_dpr_ptr_fifo.write();
  dpr_csr.mem.cfg_dpr_pktout_fifo.write();

 } //cap_dpa_bist_test_clear


void cap_dpa_bist_test_chk(int chip_id, int inst_id) {
 cap_dpp_csr_t & dpp_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpp_csr_t, chip_id, inst_id);
 cap_dpr_csr_t & dpr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_dpr_csr_t, chip_id, inst_id);
 int           loop_cnt;

 PLOG_API_MSG("DPA", "inside cap_dpa_bist_test_chk \n");

  loop_cnt = 0;
  while (loop_cnt < 10000) {
    dpp_csr.mem.sta_srams_ecc_dpp_phv_fifo.read();
    if ( dpp_csr.mem.sta_srams_ecc_dpp_phv_fifo.bist_done_fail() == 1 || dpp_csr.mem.sta_srams_ecc_dpp_phv_fifo.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 10000) {
     PLOG_ERR("FAIL : dpp[" << inst_id << "] : sta_srams_ecc_dpp_phv_fifo.bist_done not set for timeout cycles " << endl);
  } else if( dpp_csr.mem.sta_srams_ecc_dpp_phv_fifo.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : dpp[" << inst_id << "] : sta_srams_ecc_dpp_phv_fifo.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 10000) {
    dpp_csr.mem.sta_srams_ecc_dpp_ohi_fifo.read();
    if ( dpp_csr.mem.sta_srams_ecc_dpp_ohi_fifo.bist_done_fail() == 1 || dpp_csr.mem.sta_srams_ecc_dpp_ohi_fifo.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 10000) {
     PLOG_ERR("FAIL : dpp[" << inst_id << "] : sta_srams_ecc_dpp_ohi_fifo.bist_done not set for timeout cycles " << endl);
  } else if( dpp_csr.mem.sta_srams_ecc_dpp_ohi_fifo.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : dpp[" << inst_id << "] : sta_srams_ecc_dpp_ohi_fifo.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 10000) {
    dpr_csr.mem.sta_srams_ecc_dpr_pktin_fifo.read();
    if ( dpr_csr.mem.sta_srams_ecc_dpr_pktin_fifo.bist_done_fail() == 1 || dpr_csr.mem.sta_srams_ecc_dpr_pktin_fifo.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 10000) {
     PLOG_ERR("FAIL : dpr[" << inst_id << "] : sta_srams_ecc_dpr_pktin_fifo.bist_done not set for timeout cycles " << endl);
  } else if( dpr_csr.mem.sta_srams_ecc_dpr_pktin_fifo.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : dpr[" << inst_id << "] : sta_srams_ecc_dpr_pktin_fifo.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 10000) {
    dpr_csr.mem.sta_srams_ecc_dpr_csum_fifo.read();
    if ( dpr_csr.mem.sta_srams_ecc_dpr_csum_fifo.bist_done_fail() == 1 || dpr_csr.mem.sta_srams_ecc_dpr_csum_fifo.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 10000) {
     PLOG_ERR("FAIL : dpr[" << inst_id << "] : sta_srams_ecc_dpr_csum_fifo.bist_done not set for timeout cycles " << endl);
  } else if( dpr_csr.mem.sta_srams_ecc_dpr_csum_fifo.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : dpr[" << inst_id << "] : sta_srams_ecc_dpr_csum_fifo.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 10000) {
    dpr_csr.mem.sta_srams_ecc_dpr_phv_fifo.read();
    if ( dpr_csr.mem.sta_srams_ecc_dpr_phv_fifo.bist_done_fail() == 1 || dpr_csr.mem.sta_srams_ecc_dpr_phv_fifo.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 10000) {
     PLOG_ERR("FAIL : dpr[" << inst_id << "] : sta_srams_ecc_dpr_phv_fifo.bist_done not set for timeout cycles " << endl);
  } else if( dpr_csr.mem.sta_srams_ecc_dpr_phv_fifo.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : dpr[" << inst_id << "] : sta_srams_ecc_dpr_phv_fifo.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 10000) {
    dpr_csr.mem.sta_srams_ecc_dpr_ohi_fifo.read();
    if ( dpr_csr.mem.sta_srams_ecc_dpr_ohi_fifo.bist_done_fail() == 1 || dpr_csr.mem.sta_srams_ecc_dpr_ohi_fifo.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 10000) {
     PLOG_ERR("FAIL : dpr[" << inst_id << "] : sta_srams_ecc_dpr_ohi_fifo.bist_done not set for timeout cycles " << endl);
  } else if( dpr_csr.mem.sta_srams_ecc_dpr_ohi_fifo.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : dpr[" << inst_id << "] : sta_srams_ecc_dpr_ohi_fifo.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 10000) {
    dpr_csr.mem.sta_srams_ecc_dpr_ptr_fifo.read();
    if ( dpr_csr.mem.sta_srams_ecc_dpr_ptr_fifo.bist_done_fail() == 1 || dpr_csr.mem.sta_srams_ecc_dpr_ptr_fifo.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 10000) {
     PLOG_ERR("FAIL : dpr[" << inst_id << "] : sta_srams_ecc_dpr_ptr_fifo.bist_done not set for timeout cycles " << endl);
  } else if( dpr_csr.mem.sta_srams_ecc_dpr_ptr_fifo.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : dpr[" << inst_id << "] : sta_srams_ecc_dpr_ptr_fifo.bist_done_fail is set " << endl);
  }
      
  loop_cnt = 0;
  while (loop_cnt < 10000) {
    dpr_csr.mem.sta_srams_ecc_dpr_pktout_fifo.read();
    if ( dpr_csr.mem.sta_srams_ecc_dpr_pktout_fifo.bist_done_fail() == 1 || dpr_csr.mem.sta_srams_ecc_dpr_pktout_fifo.bist_done_pass() == 1 ) {
         break;
    }
    loop_cnt++;
  }
  if(loop_cnt == 10000) {
     PLOG_ERR("FAIL : dpr[" << inst_id << "] : sta_srams_ecc_dpr_pktout_fifo.bist_done not set for timeout cycles " << endl);
  } else if( dpr_csr.mem.sta_srams_ecc_dpr_pktout_fifo.bist_done_fail() == 1 ) {
     PLOG_ERR("FAIL : dpr[" << inst_id << "] : sta_srams_ecc_dpr_pktout_fifo.bist_done_fail is set " << endl);
  }
     

 } //cap_dpa_bist_chk


void cap_dpa_pktin_ff_show(int chip_id, int inst_id) {
  
  cap_dpr_csr_t & dpr_csr = PEN_BLK_REG_MODEL_ACCESS(cap_dpr_csr_t, chip_id, inst_id);

  uint8_t        pktin_mem_arr [832*64];
  unsigned       sop_vec [832];
  unsigned       eop_vec[832];
  unsigned       byte_size_vec[832];
  unsigned       pkt_size;

  uint8_t        pktin_tmp[16*1024];

  cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t sta_pktin_fifo;
  cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t  pktin_mem_raw_data;
  dpr_csr.mem.sta_ff_ptr_dpr_pktin_fifo.read();
  sta_pktin_fifo.all(dpr_csr.mem.sta_ff_ptr_dpr_pktin_fifo.all());
  unsigned pktin_ff_rptr =  sta_pktin_fifo.sta_ptr_rptr().convert_to<unsigned>();
  unsigned pktin_ff_wptr =  sta_pktin_fifo.sta_ptr_wptr().convert_to<unsigned>();
  unsigned empty = sta_pktin_fifo.sta_empty().convert_to<unsigned>();
  unsigned full = sta_pktin_fifo.sta_full().convert_to<unsigned>();

  unsigned depth = empty ? 0 :
    (pktin_ff_wptr > pktin_ff_rptr) ? 
    (pktin_ff_wptr - pktin_ff_rptr) :
    (832 - pktin_ff_rptr + pktin_ff_wptr);
  
  PLOG_MSG("DPR_PKTIN_FIFO : empty= " << empty << 
	    " full=" << full <<
	    " rptr=" << pktin_ff_rptr << 
	    " wptr=" << pktin_ff_wptr <<
	    " depth=" << depth << endl);

  //read memory from rptr
  unsigned mem_addr;
  unsigned sop, eop, byte_size;
  //unsigned err;
  cpp_int data;

  unsigned pktin_mem_arr_addr = 0;
  pu_cpp_int<64>  pkt_bytes;
  uint8_t data_byte;

  
  for (int i = 0; i < 832; i++) {
    mem_addr = (i)%832;
    dpr_csr.mem.dhs_dpr_pktin_fifo_sram.entry[mem_addr].read();
    pktin_mem_raw_data.all(dpr_csr.mem.dhs_dpr_pktin_fifo_sram.entry[mem_addr].all());
    cpp_int pktin_mem_data = pktin_mem_raw_data.data();
    sop = cpp_int_helper::s_get_slc(pktin_mem_data, 1, 520).convert_to<unsigned>();
    eop = cpp_int_helper::s_get_slc(pktin_mem_data, 1, 519).convert_to<unsigned>();
    //err = cpp_int_helper::s_get_slc(pktin_mem_data, 1, 518).convert_to<unsigned>();
    byte_size = cpp_int_helper::s_get_slc(pktin_mem_data, 6, 512).convert_to<unsigned>();
    data = cpp_int_helper::s_get_slc(pktin_mem_data, 512, 0);
    //    PLOG_MSG("DPR_PKTIN_FIFO entry " << i << 
    //	      " ADDRESS=" << mem_addr <<
    //	      " SOP=" << sop <<
    //	      " EOP=" << eop <<
    //	      " BYTE_SIZE=" << byte_size << 
    //	      " DATA=0x" << hex << data << endl);
    sop_vec[i] = sop;
    eop_vec[i] = eop;
    byte_size_vec[i] = byte_size;
    
    
    for(int byte =0 ; byte < 64; byte++){
      //PLOG_MSG("DPR_PKTIN_FIFO byte " << byte << endl);
      data_byte = (uint8_t)cpp_int_helper::s_get_slc(data,8,(512-8*(byte+1))).convert_to<unsigned>();
      //      PLOG_MSG("DPR_PKTIN_FIFO byte " << byte << "value =0x" << hex << (unsigned)data_byte << endl);
      //PLOG_MSG("DPR_PKTIN_FIFO pktin_mem_arr_addr=" << pktin_mem_arr_addr << endl);
      //(*pktin_mem_arr)[pktin_mem_arr_addr] = cpp_int_helper::s_get_slc(data,8,(512-8*(byte+1))).convert_to<int>();
      pktin_mem_arr[pktin_mem_arr_addr] = data_byte;
      pktin_mem_arr_addr += 1;
      // PLOG_MSG("DPR_PKTIN_FIFO byte done" << byte << endl);
    }    
  }  

  
  unsigned first_sop_idx = 0;
  bool first_sop_found = false;
  unsigned pkt_number = 0;
  unsigned start = empty ? pktin_ff_rptr : ((pktin_ff_rptr + 832 -7) % 832);
  
 
  for (unsigned sop_idx = start; sop_idx < (start + 832*2); sop_idx++) {
    unsigned sop_idx_tmp =  sop_idx%832;  
    //look for next eop
    if(sop_vec[sop_idx_tmp] == 1) {
      PLOG_MSG("DPA-API: found SOP line=: " <<  sop_idx_tmp << endl);
       
      if(!first_sop_found) {
	first_sop_found = true;
	first_sop_idx = sop_idx_tmp;
      } else {
	if(sop_idx_tmp == first_sop_idx) break;
      }
      pkt_size = 0;
      unsigned eop_idx_tmp;
      for(unsigned eop_idx = sop_idx_tmp; eop_idx < (sop_idx_tmp+(16*1024/64)); eop_idx++) {
	eop_idx_tmp = eop_idx % 832;
	if(eop_vec[eop_idx_tmp] == 1) {
	  PLOG_MSG("DPA-API: found EOP line=: " <<  eop_idx_tmp << endl);
	  pkt_size += byte_size_vec[eop_idx_tmp];
	  break;
	} else {
	  pkt_size += 64;
	}
      }
      
      //copy packet
      // PLOG_MSG("DPA-API: copying packet: "<< endl);
      for(unsigned byte = 0; byte < pkt_size; byte++) {
	pktin_tmp[byte] = pktin_mem_arr[(sop_idx_tmp*64+byte)%(832*64)];
      }
      
      unsigned byte;
      bool pending = false;
      if(empty) { 
	pending = false;
      } else if (sop_idx_tmp > pktin_ff_rptr) {
	if((eop_idx_tmp > pktin_ff_rptr) && (eop_idx_tmp < sop_idx_tmp)) {
	  pending = true;
	}
      } else if (sop_idx_tmp < pktin_ff_rptr) {
	if(((eop_idx_tmp > pktin_ff_rptr) && (eop_idx_tmp > sop_idx_tmp)) ||
	   ((eop_idx_tmp < pktin_ff_rptr) && (eop_idx_tmp < sop_idx_tmp))) {
	  pending = true;
	} 
      }

      //print packet
      PLOG_MSG("DPA-API:  input packet number: " << pkt_number << " with size : " << pkt_size << " pending : " << pending << ", data : " << hex << endl);
      for(byte=0; byte<pkt_size; byte++) {
	cpp_int_helper::s_rep_slc(pkt_bytes, (unsigned)(pktin_tmp[byte]), 56-((byte%8)*8), 63-((byte%8)*8) );
	
	if((byte+1)%8 == 0) {
	  PLOG_MSG(setw(6) << (byte/8)*8 << ": " << hex << setw(16) << pkt_bytes << dec << endl);
	  pkt_bytes = 0;
	}
      }
      if(pkt_size % 8 != 0) {
	PLOG_MSG(setw(6) << (byte/8)*8 << ": " << hex << setw(16) << pkt_bytes << dec << endl);
      }
      
      //adjust sop_idx
      sop_idx = eop_idx_tmp;
      pkt_number += 1;
    }
  }
 
  
}

void cap_dpa_csum_ff_show(int chip_id, int inst_id) {
  
  cap_dpr_csr_t & dpr_csr = PEN_BLK_REG_MODEL_ACCESS(cap_dpr_csr_t, chip_id, inst_id);

  uint8_t        csum_mem_arr [512*64];
  unsigned       sop_vec [512];
  unsigned       eop_vec[512];
  unsigned       byte_size_vec[512];
  unsigned       pkt_size;

  uint8_t        csum_tmp[16*1024];

  cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t sta_csum_fifo;
  cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t  csum_mem_raw_data;
  dpr_csr.mem.sta_ff_ptr_dpr_csum_fifo.read();
  sta_csum_fifo.all(dpr_csr.mem.sta_ff_ptr_dpr_csum_fifo.all());
  unsigned csum_ff_rptr =  sta_csum_fifo.sta_ptr_rptr().convert_to<unsigned>();
  unsigned csum_ff_wptr =  sta_csum_fifo.sta_ptr_wptr().convert_to<unsigned>();
  unsigned empty = sta_csum_fifo.sta_empty().convert_to<unsigned>();
  unsigned full = sta_csum_fifo.sta_full().convert_to<unsigned>();

  unsigned depth = empty ? 0 :
    (csum_ff_wptr > csum_ff_rptr) ? 
    (csum_ff_wptr - csum_ff_rptr) :
    (512 - csum_ff_rptr + csum_ff_wptr);
  
  PLOG_MSG("DPR_CSUM_FIFO : empty= " << empty << 
	    " full=" << full <<
	    " rptr=" << csum_ff_rptr << 
	    " wptr=" << csum_ff_wptr <<
	    " depth=" << depth << endl);


  //read memory from rptr
  unsigned mem_addr;
  unsigned sop, eop, byte_size;
  //unsigned err;
  cpp_int data;

  unsigned csum_mem_arr_addr = 0;
  pu_cpp_int<64>  pkt_bytes;
  uint8_t data_byte;

  
  for (int i = 0; i < 512; i++) {
    mem_addr = (i)%512;
    dpr_csr.mem.dhs_dpr_csum_fifo_sram.entry[mem_addr].read();
    csum_mem_raw_data.all(dpr_csr.mem.dhs_dpr_csum_fifo_sram.entry[mem_addr].all());
    cpp_int csum_mem_data = csum_mem_raw_data.data();
    sop = cpp_int_helper::s_get_slc(csum_mem_data, 1, 520).convert_to<unsigned>();
    eop = cpp_int_helper::s_get_slc(csum_mem_data, 1, 519).convert_to<unsigned>();
    //err = cpp_int_helper::s_get_slc(csum_mem_data, 1, 518).convert_to<unsigned>();
    byte_size = cpp_int_helper::s_get_slc(csum_mem_data, 6, 512).convert_to<unsigned>();
    data = cpp_int_helper::s_get_slc(csum_mem_data, 512, 0);
    //    PLOG_MSG("DPR_CSUM_FIFO entry " << i << 
    //	     " ADDRESS=" << mem_addr <<
    //	     " SOP=" << sop <<
    //	     " EOP=" << eop <<
    //	     " BYTE_SIZE=" << byte_size << 
    //	     " DATA=0x" << hex << data << endl);
    sop_vec[i] = sop;
    eop_vec[i] = eop;
    byte_size_vec[i] = byte_size;
    
    
    for(int byte =0 ; byte < 64; byte++){
      //PLOG_MSG("DPR_CSUM_FIFO byte " << byte << endl);
      data_byte = (uint8_t)cpp_int_helper::s_get_slc(data,8,(512-8*(byte+1))).convert_to<unsigned>();
      //      PLOG_MSG("DPR_CSUM_FIFO byte " << byte << "value =0x" << hex << (unsigned)data_byte << endl);
      //PLOG_MSG("DPR_CSUM_FIFO csum_mem_arr_addr=" << csum_mem_arr_addr << endl);
      //(*csum_mem_arr)[csum_mem_arr_addr] = cpp_int_helper::s_get_slc(data,8,(512-8*(byte+1))).convert_to<int>();
      csum_mem_arr[csum_mem_arr_addr] = data_byte;
      csum_mem_arr_addr += 1;
      // PLOG_MSG("DPR_CSUM_FIFO byte done" << byte << endl);
    }    
  }  

  
  unsigned first_sop_idx = 0;
  bool first_sop_found = false;
  unsigned pkt_number = 0;
  unsigned start = empty ? csum_ff_rptr : ((csum_ff_rptr + 512 -7) % 512);
  
 
  for (unsigned sop_idx = start; sop_idx < (start + 512*2); sop_idx++) {
    unsigned sop_idx_tmp =  sop_idx%512;  
    //look for next eop
    if(sop_vec[sop_idx_tmp] == 1) {
      PLOG_MSG("DPA-API: found SOP line=: " <<  sop_idx_tmp << endl);
       
      if(!first_sop_found) {
	first_sop_found = true;
	first_sop_idx = sop_idx_tmp;
      } else {
	if(sop_idx_tmp == first_sop_idx) break;
      }
      pkt_size = 0;
      unsigned eop_idx_tmp;
      for(unsigned eop_idx = sop_idx_tmp; eop_idx < (sop_idx_tmp+(16*1024/64)); eop_idx++) {
	eop_idx_tmp = eop_idx % 512;
	if(eop_vec[eop_idx_tmp] == 1) {
	  PLOG_MSG("DPA-API: found EOP line=: " <<  eop_idx_tmp << endl);
	  pkt_size += byte_size_vec[eop_idx_tmp];
	  break;
	} else {
	  pkt_size += 64;
	}
      }
      
      //copy packet
      // PLOG_MSG("DPA-API: copying packet: "<< endl);
      for(unsigned byte = 0; byte < pkt_size; byte++) {
	csum_tmp[byte] = csum_mem_arr[(sop_idx_tmp*64+byte)%(512*64)];
      }
      
      unsigned byte;
      bool pending = false;
      if(empty) { 
	pending = false;
      } else if (sop_idx_tmp > csum_ff_rptr) {
	if((eop_idx_tmp > csum_ff_rptr) && (eop_idx_tmp < sop_idx_tmp)) {
	  pending = true;
	}
      } else if (sop_idx_tmp < csum_ff_rptr) {
	if(((eop_idx_tmp > csum_ff_rptr) && (eop_idx_tmp > sop_idx_tmp)) ||
	   ((eop_idx_tmp < csum_ff_rptr) && (eop_idx_tmp < sop_idx_tmp))) {
	  pending = true;
	} 
      }

      //print packet
      PLOG_MSG("DPA-API:  csum fifo packet number: " << pkt_number << " with size : " << pkt_size << " pending : " << pending << ", data : " << hex << endl);
      for(byte=0; byte<pkt_size; byte++) {
	cpp_int_helper::s_rep_slc(pkt_bytes, (unsigned)(csum_tmp[byte]), 56-((byte%8)*8), 63-((byte%8)*8) );
	
	if((byte+1)%8 == 0) {
	  PLOG_MSG(setw(6) << (byte/8)*8 << ": " << hex << setw(16) << pkt_bytes << dec << endl);
	  pkt_bytes = 0;
	}
      }
      if(pkt_size % 8 != 0) {
	PLOG_MSG(setw(6) << (byte/8)*8 << ": " << hex << setw(16) << pkt_bytes << dec << endl);
      }
      
      //adjust sop_idx
      sop_idx = eop_idx_tmp;
      pkt_number += 1;
    }
  }
 
  
}

