#include "cap_wa_api.h"

void cap_wa_soft_reset(int chip_id, int inst_id) {
}

void cap_wa_set_soft_reset(int chip_id, int inst_id, int value) {
}

void cap_wa_sw_reset(int chip_id, int inst_id) {
}

void cap_wa_init_start(int chip_id, int inst_id) {
    cap_wa_csr_t & wa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_wa_csr_t, 0, 0);

    wa_csr.cfg_wa_merge.inflight_en(1);
    wa_csr.cfg_wa_merge.pre_axi_read_en(1);
    wa_csr.cfg_wa_merge.write();

    wa_csr.cfg_doorbell_axi_attr.set_access_secure(1);
}

void cap_wa_init_done(int chip_id, int inst_id) {
    cap_wa_csr_t & wa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_wa_csr_t, 0, 0);
    cpp_int        temp;
    string         tmp_str;
    stringstream   tmp_ss;
    string         hier_path_str = wa_csr.get_hier_path();
    PLOG_MSG("start load wa cfg...." << endl)

    //wa_csr.load_from_cfg();
    std::replace( hier_path_str.begin(), hier_path_str.end(), '.', '/');
    std::replace( hier_path_str.begin(), hier_path_str.end(), '[', '/');
    std::replace( hier_path_str.begin(), hier_path_str.end(), ']', '/');

    tmp_ss.str("");
    tmp_ss << hier_path_str << "/cfg_wa_axi/num_ids";
    tmp_str = tmp_ss.str();
    if (sknobs_exists((char *)tmp_str.c_str())) {
        temp.assign(sknobs_get_string((char *)tmp_str.c_str(), (char *)""));
        wa_csr.cfg_wa_axi.num_ids(temp);
        wa_csr.cfg_wa_axi.show();
        wa_csr.cfg_wa_axi.write();
    }

    tmp_ss.str("");
    tmp_ss << hier_path_str << "/cfg_doorbell_axi_attr/lock";
    tmp_str = tmp_ss.str();
    //PLOG_MSG("tmp_str = " << tmp_str.c_str() << endl)
    if (sknobs_exists((char *)tmp_str.c_str())) {
        temp.assign(sknobs_get_string((char *)tmp_str.c_str(), (char *)""));
        wa_csr.cfg_doorbell_axi_attr.lock(temp);
    }

    tmp_ss.str("");
    tmp_ss << hier_path_str << "/cfg_doorbell_axi_attr/qos";
    tmp_str = tmp_ss.str();
    //PLOG_MSG("tmp_str = " << tmp_str.c_str() << endl)
    if (sknobs_exists((char *)tmp_str.c_str())) {
        temp.assign(sknobs_get_string((char *)tmp_str.c_str(), (char *)""));
        wa_csr.cfg_doorbell_axi_attr.qos(temp);
    }

    tmp_ss.str("");
    tmp_ss << hier_path_str << "/cfg_doorbell_axi_attr/prot";
    tmp_str = tmp_ss.str();
    //PLOG_MSG("tmp_str = " << tmp_str.c_str() << endl)
    if (sknobs_exists((char *)tmp_str.c_str())) {
        temp.assign(sknobs_get_string((char *)tmp_str.c_str(), (char *)""));
        wa_csr.cfg_doorbell_axi_attr.prot(temp);
    }

    tmp_ss.str("");
    tmp_ss << hier_path_str << "/cfg_doorbell_axi_attr/awcache";
    tmp_str = tmp_ss.str();
    //PLOG_MSG("tmp_str = " << tmp_str.c_str() << endl)
    if (sknobs_exists((char *)tmp_str.c_str())) {
        temp.assign(sknobs_get_string((char *)tmp_str.c_str(), (char *)""));
        wa_csr.cfg_doorbell_axi_attr.awcache(temp);
    }

    tmp_ss.str("");
    tmp_ss << hier_path_str << "/cfg_doorbell_axi_attr/arcache";
    tmp_str = tmp_ss.str();
    //PLOG_MSG("tmp_str = " << tmp_str.c_str() << endl)
    if (sknobs_exists((char *)tmp_str.c_str())) {
        temp.assign(sknobs_get_string((char *)tmp_str.c_str(), (char *)""));
        wa_csr.cfg_doorbell_axi_attr.arcache(temp);
    }
    wa_csr.cfg_doorbell_axi_attr.show();
    wa_csr.cfg_doorbell_axi_attr.write();

    tmp_ss.str("");
    tmp_ss << hier_path_str << "/cfg_wa_sched_hint/enable_src_mask";
    tmp_str = tmp_ss.str();
    //PLOG_MSG("tmp_str = " << tmp_str.c_str() << endl)
    if (sknobs_exists((char *)tmp_str.c_str())) {
        temp.assign(sknobs_get_string((char *)tmp_str.c_str(), (char *)""));
        wa_csr.cfg_wa_sched_hint.all(temp);
        wa_csr.cfg_wa_sched_hint.show();
        wa_csr.cfg_wa_sched_hint.write();
    }

    tmp_ss.str("");
    tmp_ss << hier_path_str << "/cfg_wa_merge/inflight_en";
    tmp_str = tmp_ss.str();
    if (sknobs_exists((char *)tmp_str.c_str())) {
        temp.assign(sknobs_get_string((char *)tmp_str.c_str(), (char *)""));
        wa_csr.cfg_wa_merge.inflight_en(temp);
    }
    tmp_ss.str("");
    tmp_ss << hier_path_str << "/cfg_wa_merge/pre_axi_read_en";
    tmp_str = tmp_ss.str();
    if (sknobs_exists((char *)tmp_str.c_str())) {
        temp.assign(sknobs_get_string((char *)tmp_str.c_str(), (char *)""));
        wa_csr.cfg_wa_merge.pre_axi_read_en(temp);
    }
    wa_csr.cfg_wa_merge.show();
    wa_csr.cfg_wa_merge.write();
    
    for (int i = 0; i < 2048; i++) {
        tmp_ss.str("");
        tmp_ss << hier_path_str << "/dhs_lif_qstate_map/entry/" << i;
        tmp_str = tmp_ss.str();
        if (sknobs_exists((char *)tmp_str.c_str())) {
            temp.assign(sknobs_get_string((char *)tmp_str.c_str(), (char *)""));
            wa_csr.dhs_lif_qstate_map.entry[i].all(temp);
            wa_csr.dhs_lif_qstate_map.entry[i].write();
            wa_csr.dhs_lif_qstate_map.entry[i].show();
        }
    }

    for (int i = 0; i < 32; i++) {
        tmp_ss.str("");
        tmp_ss << hier_path_str << "/dhs_page4k_remap_tbl/entry/" << i;
        tmp_str = tmp_ss.str();
        if (sknobs_exists((char *)tmp_str.c_str())) {
            temp.assign(sknobs_get_string((char *)tmp_str.c_str(), (char *)""));
            wa_csr.dhs_page4k_remap_tbl.entry[i].all(temp);
            wa_csr.dhs_page4k_remap_tbl.entry[i].show();
            wa_csr.dhs_page4k_remap_tbl.entry[i].write();
        }
    }

    for(int idx = 0; idx < 8; idx++) {
       wa_csr.filter_addr_hi.data[idx].load_from_cfg();
       wa_csr.filter_addr_lo.data[idx].load_from_cfg();
       wa_csr.filter_addr_ctl.value[idx].load_from_cfg();
    }

    wa_csr.base.read();
}

void cap_wa_load_from_cfg(int chip_id, int inst_id) {
  cap_wa_csr_t & wa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_wa_csr_t, 0, 0);

  uint32_t pic_cache_enable  = sknobs_get_value((char*)"cap0/top/main/pic_cache_enable", 0);
  if (pic_cache_enable == 1) {
     wa_csr.filter_addr_hi.data[0].read();
     wa_csr.filter_addr_hi.data[0].value(0xfffffff);
     wa_csr.filter_addr_hi.data[0].write();
     wa_csr.filter_addr_ctl.value[0].read();
     wa_csr.filter_addr_ctl.value[0].value(0xf);
     wa_csr.filter_addr_ctl.value[0].write();
  }
}


void cap_wa_print_inval_master_cfg_info(int chip_id, int inst_id) {
    cap_wa_csr_t & wa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_wa_csr_t, 0, 0);

    PLOG_INFO("WA : inval_master config info : " << endl)
    for(int idx = 0; idx < 8; idx++) {
       wa_csr.filter_addr_ctl.value[idx].read();
       if( (wa_csr.filter_addr_ctl.value[idx].value() & 0x8) == 0x8) { //bit3 (vld)
         wa_csr.filter_addr_hi.data[idx].read();
         wa_csr.filter_addr_lo.data[idx].read();
         if( (wa_csr.filter_addr_ctl.value[idx].value() & 0x4) == 0x4) { //bit2 (include/use cache)
            PLOG_INFO("WA : cache filter idx : " << idx << " enabled with include addr range" 
               << " filter_addr_lo : 0x" << hex << wa_csr.filter_addr_lo.data[idx].all()
               << " filter_addr_hi : 0x" << hex << wa_csr.filter_addr_hi.data[idx].all()
               << " inval_send : " << (wa_csr.filter_addr_ctl.value[idx].value() & 0x1)
               << " inval_fill : " << ( (wa_csr.filter_addr_ctl.value[idx].value()>>1) & 0x1)
               << dec << endl)
         } else {
            PLOG_INFO("WA : cache filter idx : " << idx << " enabled with exclude addr range" 
               << " filter_addr_lo : 0x" << hex << wa_csr.filter_addr_lo.data[idx].all()
               << " filter_addr_hi : 0x" << hex << wa_csr.filter_addr_hi.data[idx].all()
               << dec << endl)
         }
       }
    }
}

void cap_wa_eos(int chip_id, int inst_id) {
    cap_wa_eos_cnt(chip_id, inst_id);
    cap_wa_eos_int_sim(chip_id, inst_id);
    cap_wa_eos_sta(chip_id, inst_id);
    cap_wa_print_inval_master_cfg_info(chip_id, inst_id);
}

void cap_wa_eos_cnt(int chip_id, int inst_id) {
    cap_wa_csr_t & wa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_wa_csr_t, 0, 0);

    wa_csr.sat_wa_host_access_err.read();
    sknobs_set_value((char*)"wa_csr/sat_wa_host_access_err/cnt", wa_csr.sat_wa_host_access_err.cnt().convert_to<unsigned long long>());
    wa_csr.sat_wa_host_access_err.show();
    
    wa_csr.sat_wa_ring_access_err.read();
    sknobs_set_value((char*)"wa_csr/sat_wa_ring_access_err/cnt", wa_csr.sat_wa_ring_access_err.cnt().convert_to<unsigned long long>());
    wa_csr.sat_wa_ring_access_err.show();
    
    wa_csr.sat_wa_qaddr_cam_conflict.read();
    sknobs_set_value((char*)"wa_csr/sat_wa_qaddr_cam_conflict/cnt", wa_csr.sat_wa_qaddr_cam_conflict.cnt().convert_to<unsigned long long>());
    wa_csr.sat_wa_qaddr_cam_conflict.show();

    wa_csr.sat_wa_merged_inflight.read();
    sknobs_set_value((char*)"wa_csr/sat_wa_merged_inflight/cnt", wa_csr.sat_wa_merged_inflight.cnt().convert_to<unsigned long long>());
    wa_csr.sat_wa_merged_inflight.show();

    wa_csr.sat_wa_merged_pre_axi_read.read();
    sknobs_set_value((char*)"wa_csr/sat_wa_merged_pre_axi_read/cnt", wa_csr.sat_wa_merged_pre_axi_read.cnt().convert_to<unsigned long long>());
    wa_csr.sat_wa_merged_pre_axi_read.show();

    wa_csr.sat_wa_qid_overflow.read();
    sknobs_set_value((char*)"wa_csr/sat_wa_qid_overflow/cnt", wa_csr.sat_wa_qid_overflow.cnt().convert_to<unsigned long long>());
    wa_csr.sat_wa_qid_overflow.show();

    wa_csr.sat_wa_pid_chkfail.read();
    sknobs_set_value((char*)"wa_csr/sat_wa_pid_chkfail/cnt", wa_csr.sat_wa_pid_chkfail.cnt().convert_to<unsigned long long>());
    wa_csr.sat_wa_pid_chkfail.show();

    wa_csr.sat_wa_axi_err.read();
    sknobs_set_value((char*)"wa_csr/sat_wa_axi_err/rresp", wa_csr.sat_wa_axi_err.rresp().convert_to<unsigned long long>());
    sknobs_set_value((char*)"wa_csr/sat_wa_axi_err/bresp", wa_csr.sat_wa_axi_err.bresp().convert_to<unsigned long long>());
    wa_csr.sat_wa_axi_err.show();

    wa_csr.cnt_wa_host_doorbells.read();
    sknobs_set_value((char*)"wa_csr/cnt_wa_host_doorbells/req", wa_csr.cnt_wa_host_doorbells.req().convert_to<unsigned long long>());
    wa_csr.cnt_wa_host_doorbells.show();

    wa_csr.cnt_wa_local_doorbells.read();
    sknobs_set_value((char*)"wa_csr/cnt_wa_local_doorbells/req", wa_csr.cnt_wa_local_doorbells.req().convert_to<unsigned long long>());
    wa_csr.cnt_wa_local_doorbells.show();

    wa_csr.cnt_wa_32b_doorbells.read();
    sknobs_set_value((char*)"wa_csr/cnt_wa_32b_doorbells/req", wa_csr.cnt_wa_32b_doorbells.req().convert_to<unsigned long long>());
    wa_csr.cnt_wa_32b_doorbells.show();

    wa_csr.cnt_wa_timer_doorbells.read();
    sknobs_set_value((char*)"wa_csr/cnt_wa_timer_doorbells/req", wa_csr.cnt_wa_timer_doorbells.req().convert_to<unsigned long long>());
    wa_csr.cnt_wa_timer_doorbells.show();

    wa_csr.cnt_wa_arm4k_doorbells.read();
    sknobs_set_value((char*)"wa_csr/cnt_wa_arm4k_doorbells/req", wa_csr.cnt_wa_arm4k_doorbells.req().convert_to<unsigned long long>());
    wa_csr.cnt_wa_arm4k_doorbells.show();

    wa_csr.cnt_wa_sched_out.read();
    sknobs_set_value((char*)"wa_csr/cnt_wa_sched_out/req", wa_csr.cnt_wa_sched_out.req().convert_to<unsigned long long>());
    wa_csr.cnt_wa_sched_out.show();
}

void cap_wa_eos_int_sim(int chip_id, int inst_id) {
    cap_wa_csr_t & wa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_wa_csr_t, 0, 0);
    string         csr_name;

    if (sknobs_exists((char*)"no_int_chk") == 0) { 
        for(auto itr: wa_csr.get_children_prefix("int_")) {
            csr_name = itr->get_name();
            PLOG_MSG("csr_name = " << csr_name.c_str() << endl)
            if (csr_name.compare("int_db") == 0) {
                wa_csr.int_db.intreg.read();
                wa_csr.int_db.intreg.show();
                if ((wa_csr.sat_wa_host_access_err.cnt() > 0)&&(wa_csr.int_db.intreg.host_ring_access_err_interrupt() == 0)) {
                    PLOG_ERR("int_db.host_ring_access_err_int should be 1" << endl)
                }
                else if ((wa_csr.sat_wa_host_access_err.cnt() == 0)&&(wa_csr.int_db.intreg.host_ring_access_err_interrupt() > 0)) {
                    PLOG_ERR("int_db.host_ring_access_err_int should be 0" << endl)
                }
                if ((wa_csr.sat_wa_ring_access_err.cnt() > 0)&&(wa_csr.int_db.intreg.total_ring_access_err_interrupt() == 0)) {
                    PLOG_ERR("int_db.total_ring_access_err_int should be 1" << endl)
                }
                else if ((wa_csr.sat_wa_ring_access_err.cnt() == 0)&&(wa_csr.int_db.intreg.total_ring_access_err_interrupt() > 0)) {
                    PLOG_ERR("int_db.total_ring_access_err_int should be 0" << endl)
                }
                if ((wa_csr.sat_wa_pid_chkfail.cnt() > 0)&&(wa_csr.int_db.intreg.db_pid_chk_fail_interrupt() == 0)) {
                    PLOG_ERR("int_db.db_pid_chk_fail_int should be 1" << endl)
                }
                else if ((wa_csr.sat_wa_pid_chkfail.cnt() == 0)&&(wa_csr.int_db.intreg.db_pid_chk_fail_interrupt() > 0)) {
                    PLOG_ERR("int_db.db_pid_chk_fail_int should be 0" << endl)
                }

                if ((wa_csr.sat_wa_qaddr_cam_conflict.cnt() > 0)&&(wa_csr.int_db.intreg.db_cam_conflict_interrupt() == 0)) {
                    PLOG_ERR("int_db.db_cam_conflict_interrupt should be 1" << endl)
                }
                else if ((wa_csr.sat_wa_qaddr_cam_conflict.cnt() == 0)&&(wa_csr.int_db.intreg.db_cam_conflict_interrupt() == 1)) {
                    PLOG_ERR("int_db.db_cam_conflict_interrupt should be 0" << endl)
                }

                if ((wa_csr.sat_wa_qid_overflow.cnt() > 0)&&(wa_csr.int_db.intreg.db_qid_overflow_interrupt() == 0)) {
                    PLOG_ERR("int_db.db_qid_overflow_interrupt should be 1" << endl)
                }
                else if ((wa_csr.sat_wa_qid_overflow.cnt() == 0)&&(wa_csr.int_db.intreg.db_qid_overflow_interrupt() == 1)) {
                    PLOG_ERR("int_db.db_qid_overflow_interrupt should be 0" << endl)
                }

                if ((wa_csr.sat_wa_axi_err.rresp() > 0)&&(wa_csr.int_db.intreg.rresp_err_interrupt() == 0)) {
                    PLOG_ERR("int_db.rresp_err_interrupt should be 1" << endl)
                }
                else if ((wa_csr.sat_wa_axi_err.rresp() == 0)&&(wa_csr.int_db.intreg.rresp_err_interrupt() == 1)) {
                    PLOG_ERR("int_db.rresp_err_interrupt should be 0" << endl)
                }

                if ((wa_csr.sat_wa_axi_err.bresp() > 0)&&(wa_csr.int_db.intreg.bresp_err_interrupt() == 0)) {
                    PLOG_ERR("int_db.bresp_err_interrupt should be 1" << endl)
                }
                else if ((wa_csr.sat_wa_axi_err.bresp() == 0)&&(wa_csr.int_db.intreg.bresp_err_interrupt() == 1)) {
                    PLOG_ERR("int_db.bresp_err_interrupt should be 0" << endl)
                }
            }
            else if (csr_name.compare("int_lif_qstate_map") == 0) {
                wa_csr.int_lif_qstate_map.intreg.read();
                wa_csr.int_lif_qstate_map.intreg.show();
                if (sknobs_exists((char*)"PEN_IP_MEM_ECC_CORRUPT_ENABLE")) {
                    if (wa_csr.int_lif_qstate_map.intreg.ecc_correctable_interrupt() != 1)
                        PLOG_ERR("int_lif_qstate_map.ecc_correctable should be 1!" << endl)
                }
                else {
                    if (wa_csr.int_lif_qstate_map.intreg.ecc_correctable_interrupt() != 0)
                        PLOG_ERR("int_lif_qstate_map.ecc_correctable should be 0!" << endl)
                }
                if (wa_csr.int_lif_qstate_map.intreg.ecc_uncorrectable_interrupt() != 0)
                    PLOG_ERR("int_lif_qstate_map.ecc_uncorrectable should be 0!" << endl)
            }
        }
    }
    else {
        wa_csr.int_lif_qstate_map.intreg.read();
        sknobs_set_value((char*)"wa_csr/int_lif_qstate_map", wa_csr.int_lif_qstate_map.intreg.all().convert_to<unsigned long long>());
        wa_csr.int_db.intreg.read();
        sknobs_set_value((char*)"wa_csr/int_db", wa_csr.int_db.intreg.all().convert_to<unsigned long long>());
    }
}

void cap_wa_eos_int(int chip_id, int inst_id) { 
    cap_wa_csr_t & wa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_wa_csr_t, 0, 0);
    
    wa_csr.int_db.intreg.read();
    if (wa_csr.int_db.intreg.host_ring_access_err_interrupt() == 1) 
        PLOG_ERR("int_db.host_ring_access_err_int should be 0!" << endl)
    if (wa_csr.int_db.intreg.total_ring_access_err_interrupt() == 1)
        PLOG_ERR("int_db.total_ring_access_err_int should be 0!" << endl)
    if (wa_csr.int_db.intreg.db_pid_chk_fail_interrupt() == 1)
        PLOG_ERR("int_db.db_pid_chk_fail_int should be 0!" << endl)
    if (wa_csr.int_db.intreg.db_qid_overflow_interrupt() == 1)
        PLOG_ERR("int_db.db_qid_overflow_int should be 0!" << endl)
    if (wa_csr.int_db.intreg.rresp_err_interrupt() == 1)
        PLOG_ERR("int_db.rresp_err_int should be 0!" << endl)
    if (wa_csr.int_db.intreg.bresp_err_interrupt() == 1)
        PLOG_ERR("int_db.bresp_err_int should be 0!" << endl)
    
    wa_csr.int_lif_qstate_map.intreg.read();
    if (wa_csr.int_lif_qstate_map.intreg.ecc_correctable_interrupt() == 1)
        PLOG_ERR("int_lif_qstate_map.ecc_correctable_int should be 0!" << endl)
    if (wa_csr.int_lif_qstate_map.intreg.ecc_uncorrectable_interrupt() == 1)
        PLOG_ERR("int_lif_qstate_map.ecc_uncorrectable_int should be 0!" << endl)
    if (wa_csr.int_lif_qstate_map.intreg.qid_invalid_interrupt() == 1)
        PLOG_ERR("int_lif_qstate_map.qid_invalid_int should be 0!" << endl)

}


void cap_wa_eos_sta(int chip_id, int inst_id) {
}

void wa_ring_doorbell(int upd, int lif, int type, int pid, int qid, int ring, int index, int db_type, uint32_t &addr, uint64_t &data) {
    cap_wa_csr_t & wa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_wa_csr_t, 0, 0);
    int          addr_reg;
    unsigned     found;

    if (db_type != SRC_ARM4K) {
        addr_reg = upd*(1<<14) + lif*(1<<3) + type;
    }
    else {
        found = 65535;
        for (int i = 0; i < 32; i++) {
            if (lif == wa_csr.dhs_page4k_remap_tbl.entry[i].lif().convert_to<int>()) {
                found = i;
                break;
            }
        }
        if (found == 65535) {
            PLOG_ERR("lif = 0x" << hex << lif << " is not correct!" << endl)
        }
        else {
            addr_reg = found *512 + (rand() % 64 )*8 + type;
        }
    }
    PLOG_MSG("upd = 0x" << hex << upd << ", lif = 0x" << hex << lif << ", type = 0x" << hex << type << ", qid = 0x" << hex << qid << ", pid = 0x" << hex << pid << endl)
    PLOG_MSG("addr_reg = 0x" << hex << addr_reg << endl)

    if (db_type == DB_HOST) {
        wa_csr.dhs_host_doorbell.entry[addr_reg].index(index);
        wa_csr.dhs_host_doorbell.entry[addr_reg].ring(ring);
        wa_csr.dhs_host_doorbell.entry[addr_reg].rsvd(0);
        wa_csr.dhs_host_doorbell.entry[addr_reg].qid(qid);
        wa_csr.dhs_host_doorbell.entry[addr_reg].pid(pid);

        wa_csr.dhs_host_doorbell.entry[addr_reg].show();
        if (sknobs_exists((char*)"block_write")) {
            wa_csr.dhs_host_doorbell.entry[addr_reg].block_write();
        }
        else
            wa_csr.dhs_host_doorbell.entry[addr_reg].write();

        addr = CAP_WA_CSR_DHS_HOST_DOORBELL_BYTE_ADDRESS + addr_reg * 8;
        data = wa_csr.dhs_host_doorbell.entry[addr_reg].all().convert_to<uint64_t>();
    }
    else if (db_type == DB_LOCAL) {
        wa_csr.dhs_local_doorbell.entry[addr_reg].index(index);
        wa_csr.dhs_local_doorbell.entry[addr_reg].ring(ring);
        wa_csr.dhs_local_doorbell.entry[addr_reg].rsvd(0);
        wa_csr.dhs_local_doorbell.entry[addr_reg].qid(qid);
        wa_csr.dhs_local_doorbell.entry[addr_reg].pid(pid);

        wa_csr.dhs_local_doorbell.entry[addr_reg].show();
        if (sknobs_exists((char*)"block_write"))
            wa_csr.dhs_local_doorbell.entry[addr_reg].block_write();
        else
            wa_csr.dhs_local_doorbell.entry[addr_reg].write();

        addr = CAP_WA_CSR_DHS_LOCAL_DOORBELL_BYTE_ADDRESS + addr_reg * 8;
        data = wa_csr.dhs_local_doorbell.entry[addr_reg].all().convert_to<uint64_t>();
    }
    else if (db_type == DB_32B) {
        wa_csr.dhs_32b_doorbell.entry[addr_reg].index(index);
        wa_csr.dhs_32b_doorbell.entry[addr_reg].qid(qid);

        wa_csr.dhs_32b_doorbell.entry[addr_reg].show();
        wa_csr.dhs_32b_doorbell.entry[addr_reg].write();

        addr = CAP_WA_CSR_DHS_32B_DOORBELL_BYTE_ADDRESS + addr_reg * 4;
        data = wa_csr.dhs_32b_doorbell.entry[addr_reg].all().convert_to<uint64_t>();
    }
    else if (db_type == SRC_ARM4K) {
        wa_csr.dhs_page4k_remap_db.entry[addr_reg].index(index);
        wa_csr.dhs_page4k_remap_db.entry[addr_reg].ring(ring);
        wa_csr.dhs_page4k_remap_db.entry[addr_reg].rsvd(0);
        wa_csr.dhs_page4k_remap_db.entry[addr_reg].qid(qid);
        wa_csr.dhs_page4k_remap_db.entry[addr_reg].rsvd4pid(0);

        wa_csr.dhs_page4k_remap_db.entry[addr_reg].show();
        if (sknobs_exists((char*)"block_write"))
            wa_csr.dhs_page4k_remap_db.entry[addr_reg].block_write();
        else
            wa_csr.dhs_page4k_remap_db.entry[addr_reg].write();

        addr = CAP_WA_CSR_DHS_PAGE4K_REMAP_DB_BYTE_ADDRESS + addr_reg * 8;
        data = wa_csr.dhs_page4k_remap_db.entry[addr_reg].all().convert_to<uint64_t>();
    }

}

void wa_ring_db(int upd, int lif, int type, int pid, int qid, int ring, int index, int db_type) {
    uint32_t  addr;
    uint64_t  data;

    wa_ring_doorbell(upd, lif, type, pid, qid, ring, index, db_type, addr, data);
}

void wa_err_activity_log(int times) {
    cap_wa_csr_t & wa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_wa_csr_t, 0, 0);
    string       str_tmp;
    
    PLOG_MSG("show wa_err_activity_log.." << endl)
    for (int i = 0; i < 16; i++) {
        wa_csr.dhs_doorbell_err_activity_log.entry[i].read();
        if (wa_csr.dhs_doorbell_err_activity_log.entry[i].valid() == 1) {
            PLOG_MSG("dhs_doorbell_err_activity_log[" << i << "]:" << endl)

            wa_csr.dhs_doorbell_err_activity_log.entry[i].show();
            
            str_tmp = "wa_csr/dhs_doorbell_err_activity_log/entry/" + itos(i + 16*times);
            sknobs_set_value((char *)str_tmp.c_str(), wa_csr.dhs_doorbell_err_activity_log.entry[i].all().convert_to<unsigned long long>());
        }
    }
}

void show_qstate(int lif, int type, int qid) {
    uint64_t   qstate_addr;
    cap_wa_csr_t & wa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_wa_csr_t, 0, 0);

    wa_csr.dhs_lif_qstate_map.entry[lif].read();
    if (wa_csr.dhs_lif_qstate_map.entry[lif].vld() == 0) {
        PLOG_ERR("lif = 0x" << hex << lif << " is invalid!" << endl)
        return;
    }

    qstate_addr = (wa_csr.dhs_lif_qstate_map.entry[lif].qstate_base().convert_to<uint64_t>() << 12);
    if (type > 0) {
        qstate_addr += ( ( uint64_t(32) << wa_csr.dhs_lif_qstate_map.entry[lif].size0().convert_to<unsigned>()) << wa_csr.dhs_lif_qstate_map.entry[lif].length0().convert_to<unsigned>() );
    }
    if (type == 0) {
        if (qid >= (1 << wa_csr.dhs_lif_qstate_map.entry[lif].length0().convert_to<int>())) {
            PLOG_ERR("qid = 0x" << hex << qid << ", qtype = " << type << " overflow!" << endl)
            wa_csr.dhs_lif_qstate_map.entry[lif].show();
            return;
        }
        qstate_addr +=  ( ( uint64_t(32) <<  wa_csr.dhs_lif_qstate_map.entry[lif].size0().convert_to<unsigned>() ) * qid);
    }
    if (type > 1) {
        qstate_addr +=  ( ( uint64_t(32) << wa_csr.dhs_lif_qstate_map.entry[lif].size1().convert_to<unsigned>()) << wa_csr.dhs_lif_qstate_map.entry[lif].length1().convert_to<unsigned>() );
    }
    if (type == 1) {
        if (qid >= (1 << wa_csr.dhs_lif_qstate_map.entry[lif].length1().convert_to<int>())) {
            PLOG_ERR("qid = 0x" << hex << qid << ", qtype = " << type << " overflow!" << endl)
            wa_csr.dhs_lif_qstate_map.entry[lif].show();
            return;
        }
        qstate_addr +=  ( ( uint64_t(32) <<  wa_csr.dhs_lif_qstate_map.entry[lif].size1().convert_to<unsigned>() ) * qid);
    }
    if (type > 2) {
        qstate_addr += ( ( uint64_t(32) << wa_csr.dhs_lif_qstate_map.entry[lif].size2().convert_to<unsigned>()) << wa_csr.dhs_lif_qstate_map.entry[lif].length2().convert_to<unsigned>() );
    }
    if (type == 2) {
        if (qid >= (1 << wa_csr.dhs_lif_qstate_map.entry[lif].length2().convert_to<int>())) {
            PLOG_ERR("qid = 0x" << hex << qid << ", qtype = " << type << " overflow!" << endl)
            wa_csr.dhs_lif_qstate_map.entry[lif].show();
            return;
        }
        qstate_addr +=  ( ( uint64_t(32) <<  wa_csr.dhs_lif_qstate_map.entry[lif].size2().convert_to<unsigned>() ) * qid);
    }
    if (type > 3) {
        qstate_addr += ( ( uint64_t(32) << wa_csr.dhs_lif_qstate_map.entry[lif].size3().convert_to<unsigned>()) << wa_csr.dhs_lif_qstate_map.entry[lif].length3().convert_to<unsigned>() );
    }
    if (type == 3) {
        if (qid >= (1 << wa_csr.dhs_lif_qstate_map.entry[lif].length3().convert_to<int>())) {
            PLOG_ERR("qid = 0x" << hex << qid << ", qtype = " << type << " overflow!" << endl)
            wa_csr.dhs_lif_qstate_map.entry[lif].show();
            return;
        }
        qstate_addr +=  ( ( uint64_t(32) <<  wa_csr.dhs_lif_qstate_map.entry[lif].size3().convert_to<unsigned>() ) * qid);
    }
    if (type > 4) {
        qstate_addr += ( ( uint64_t(32) << wa_csr.dhs_lif_qstate_map.entry[lif].size4().convert_to<unsigned>()) << wa_csr.dhs_lif_qstate_map.entry[lif].length4().convert_to<unsigned>() );
    }
    if (type == 4) {
        if (qid >= (1 << wa_csr.dhs_lif_qstate_map.entry[lif].length4().convert_to<int>())) {
            PLOG_ERR("qid = 0x" << hex << qid << ", qtype = " << type << " overflow!" << endl)
            wa_csr.dhs_lif_qstate_map.entry[lif].show();
            return;
        }
        qstate_addr +=  ( ( uint64_t(32) <<  wa_csr.dhs_lif_qstate_map.entry[lif].size4().convert_to<unsigned>() ) * qid);
    }
    if (type > 5) {
        qstate_addr += ( ( uint64_t(32) << wa_csr.dhs_lif_qstate_map.entry[lif].size5().convert_to<unsigned>()) << wa_csr.dhs_lif_qstate_map.entry[lif].length5().convert_to<unsigned>() );
    }
    if (type == 5) {
        if (qid >= (1 << wa_csr.dhs_lif_qstate_map.entry[lif].length5().convert_to<int>())) {
            PLOG_ERR("qid = 0x" << hex << qid << ", qtype = " << type << " overflow!" << endl)
            wa_csr.dhs_lif_qstate_map.entry[lif].show();
            return;
        }
        qstate_addr +=  ( ( uint64_t(32) <<  wa_csr.dhs_lif_qstate_map.entry[lif].size5().convert_to<unsigned>() ) * qid);
    }
    if (type > 6) {
        qstate_addr += ( ( uint64_t(32) << wa_csr.dhs_lif_qstate_map.entry[lif].size6().convert_to<unsigned>()) << wa_csr.dhs_lif_qstate_map.entry[lif].length6().convert_to<unsigned>() );
    }
    if (type == 6) {
        if (qid >= (1 << wa_csr.dhs_lif_qstate_map.entry[lif].length6().convert_to<int>())) {
            PLOG_ERR("qid = 0x" << hex << qid << ", qtype = " << type << " overflow!" << endl)
            wa_csr.dhs_lif_qstate_map.entry[lif].show();
            return;
        }
        qstate_addr +=  ( ( uint64_t(32) <<  wa_csr.dhs_lif_qstate_map.entry[lif].size6().convert_to<unsigned>() ) * qid);
    }
    if (type == 7) {
        if (qid >= (1 << wa_csr.dhs_lif_qstate_map.entry[lif].length7().convert_to<int>())) {
            PLOG_ERR("qid = 0x" << hex << qid << ", qtype = " << type << " overflow!" << endl)
            wa_csr.dhs_lif_qstate_map.entry[lif].show();
            return;
        }
        qstate_addr +=  ( ( uint64_t(32) <<  wa_csr.dhs_lif_qstate_map.entry[lif].size7().convert_to<unsigned>() ) * qid);
    }
    PLOG_MSG("show_qstate::qstate_addr = 0x" << hex << qstate_addr << endl)
    

    #ifdef _CSV_INCLUDED_
    cap_mem_access *ma = cap_mem_access::instancep();
    auto old_active_read = ma->active_read;
    //ma->active_read = CAP_MEMACC_RTL_FD;
    ma->active_read = CAP_MEMACC_UVM_DIRECT;
    auto old_primary_read = ma->primary_read;
    //ma->primary_read = CAP_MEMACC_RTL_FD;
    ma->primary_read = CAP_MEMACC_UVM_DIRECT;
    auto old_compare_read = ma->compare_read;
    ma->compare_read = false;
    ma->update();
    #endif
    unsigned char* data;
    data = new unsigned char[64];
    cap_mem_access::instancep()->read(qstate_addr, data, 64, 0, 0);
    
    cpp_int  rd_data;
    for (int i = 0; i < 64; i++) {
        cpp_int_helper::s_rep_slc(rd_data, data[i], i*8, i*8+7);
    }
    cap_qstate_decoder_t  wa_qstate;
    wa_qstate.all(rd_data);

    wa_qstate.show();

    #ifdef _CSV_INCLUDED_
    ma->active_read  = old_active_read;
    ma->compare_read = old_compare_read;
    ma->primary_read = old_primary_read;
    ma->update();
    #endif
}


