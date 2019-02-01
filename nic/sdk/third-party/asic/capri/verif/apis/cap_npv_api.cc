#include "cap_npv_api.h"

// soft reset sequence 
void cap_npv_soft_reset(int chip_id, int inst_id) {
}

// soft reset, value = 1 means apply reset, value = 0 release soft reset
void cap_npv_set_soft_reset(int chip_id, int inst_id, int value) {
}

// init start, no polling yet
void cap_npv_init_start(int chip_id, int inst_id) {
    cap_pt_csr_t &pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, chip_id, inst_id);

    pt_csr.psp.cfg_profile.collapse_enable(1);
    pt_csr.psp.cfg_profile.pb_err_enable(1);
    pt_csr.psp.cfg_profile.write();
}

// poll for init done
void cap_npv_init_done(int chip_id, int inst_id) {
    cap_pt_csr_t &pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, chip_id, inst_id);

    pt_csr.psp.cfg_qstate_map_rsp.enable(1);
    pt_csr.psp.cfg_qstate_map_rsp.qid_enable(1);
    pt_csr.psp.cfg_qstate_map_rsp.qtype_enable(1);
    pt_csr.psp.cfg_qstate_map_rsp.addr_enable(1);
    pt_csr.psp.cfg_qstate_map_rsp.no_data_enable(0);
    pt_csr.psp.cfg_qstate_map_rsp.write();   
    
    pt_csr.psp.cfg_profile.ptd_npv_phv_full_enable(1);
    pt_csr.psp.cfg_profile.recirc_max(7);
    pt_csr.psp.cfg_profile.recirc_max_enable(1);
    pt_csr.psp.cfg_profile.recirc_inc_enable(1);
    pt_csr.psp.cfg_profile.input_enable(1);
    pt_csr.psp.cfg_profile.swphv_enable(1);
    pt_csr.psp.cfg_profile.write();

    pt_csr.psp.cfg_npv_values.localtime_enable(1);
    pt_csr.psp.cfg_npv_values.write();
    
    pt_csr.psp.base.read();

}

// use sknobs base load cfg 
void cap_npv_load_from_cfg(int chip_id, int inst_id) {
    cap_pt_csr_t &pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, chip_id, inst_id);
    pu_cpp_int<512>  reset_value = ~0x0;

    PLOG_MSG("npv load from cfg.... " << endl)

    for (int i = 0; i < 16; i++) 
        pt_csr.psp.dhs_sw_phv_mem.entry[i].set_reset_val(reset_value);
    for (int i = 0; i < 8; i++)
        pt_csr.psp.cfg_sw_phv_control[i].set_reset_val(reset_value);

    pt_csr.psp.load_from_cfg();
    
    pt_csr.psp.base.read();
}

// eos check, this functions should call eos_cnt, eos_int, eos_sta 
void cap_npv_eos(int chip_id, int inst_id) {
    cap_npv_eos_cnt(chip_id, inst_id);
    cap_npv_eos_int_sim(chip_id, inst_id);
    cap_npv_eos_sta(chip_id, inst_id);
}

void cap_npv_eos_cnt(int chip_id, int inst_id) {
    cap_pt_csr_t &pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, chip_id, inst_id);

    pt_csr.psp.cnt_sw_sop.read();
    pt_csr.psp.cnt_sw_eop.read();
    pt_csr.psp.cnt_sw_xns.read();
    if ((pt_csr.psp.cnt_sw_sop.all() & 0xff) != pt_csr.psp.cnt_sw_eop.all()) {
        PLOG_ERR("pt_csr.cnt_sw.sop = 0x" << hex << pt_csr.psp.cnt_sw_sop.all() << ", vs eop = 0x" << pt_csr.psp.cnt_sw_eop.all() << endl)
    }
    sknobs_set_value((char*)"pt_csr/psp/cnt_sw/sop", pt_csr.psp.cnt_sw_sop.all().convert_to<unsigned long long>()); 
    sknobs_set_value((char*)"pt_csr/psp/cnt_sw/xns", pt_csr.psp.cnt_sw_xns.all().convert_to<unsigned long long>()); 
    
    //this is txs event cnt
    pt_csr.psp.cnt_pb_pbus_xns.read();
    pt_csr.psp.cnt_pb_pbus_sop.read();
    pt_csr.psp.cnt_pb_pbus_eop.read();
    pt_csr.psp.cnt_pb_pbus_no_data.read();
    if ((pt_csr.psp.cnt_pb_pbus_sop.all() & 0xff) != pt_csr.psp.cnt_pb_pbus_eop.all()) {
        PLOG_ERR("pt_csr.cnt_pb_pbus.sop = 0x" << hex << pt_csr.psp.cnt_pb_pbus_sop.all() << ", vs eop = 0x" << pt_csr.psp.cnt_pb_pbus_eop.all() << endl)
    }
    if (pt_csr.psp.cnt_pb_pbus_sop.all() != pt_csr.psp.cnt_pb_pbus_xns.all()) {
        PLOG_ERR("pt_csr.cnt_pb_pbus.sop = 0x" << hex << pt_csr.psp.cnt_pb_pbus_sop.all() << ", vs xns = 0x" << pt_csr.psp.cnt_pb_pbus_xns.all() << endl)
    }
    if (pt_csr.psp.cnt_pb_pbus_no_data.all() != 0) {
        PLOG_ERR("pt_csr.cnt_pb_pbus.no_data = 0x" << hex << pt_csr.psp.cnt_pb_pbus_no_data.all() << " vs exp = 0" << endl)
    }
    sknobs_set_value((char*)"pt_csr/psp/cnt_pb_pbus/sop", pt_csr.psp.cnt_pb_pbus_sop.all().convert_to<unsigned long long>()); 
    sknobs_set_value((char*)"pt_csr/psp/cnt_pb_pbus/xns", pt_csr.psp.cnt_pb_pbus_xns.all().convert_to<unsigned long long>()); 
    
    pt_csr.psp.cnt_ma_xns.read();
    pt_csr.psp.cnt_ma_sop.read();
    pt_csr.psp.cnt_ma_eop.read();
    pt_csr.psp.cnt_ma_recirc.read();
    pt_csr.psp.cnt_ma_no_data.read();
    pt_csr.psp.cnt_ma_collapsed.read();
    pt_csr.psp.cnt_ma_drop.read();
    if ((pt_csr.psp.cnt_ma_sop.all() & 0xff) != pt_csr.psp.cnt_ma_eop.all()) {
        PLOG_ERR("pt_csr.cnt_ma.sop = 0x" << hex << pt_csr.psp.cnt_ma_sop.all() << ", vs eop = 0x" << pt_csr.psp.cnt_ma_eop.all() << endl)
    }
    sknobs_set_value((char*)"pt_csr/psp/cnt_ma/sop", pt_csr.psp.cnt_ma_sop.all().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pt_csr/psp/cnt_ma/xns", pt_csr.psp.cnt_ma_xns.all().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pt_csr/psp/cnt_ma/recirc", pt_csr.psp.cnt_ma_recirc.all().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pt_csr/psp/cnt_ma/no_data", pt_csr.psp.cnt_ma_no_data.all().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pt_csr/psp/cnt_ma/collapsed", pt_csr.psp.cnt_ma_collapsed.all().convert_to<unsigned long long>());

    pt_csr.psp.cnt_pr_pbus_xns.read();
    if (pt_csr.psp.cnt_pr_pbus_xns.all() != 0) {
        PLOG_ERR("pt_csr.cnt_pr_pbus_xns.cnt = 0x" << hex << pt_csr.psp.cnt_pr_pbus_xns.all() << ", vs exp = 0x0!" << endl)
        pt_csr.psp.cnt_pr_pbus_xns.show();
    }

    pt_csr.psp.cnt_pr_pbus_sop.read();
    if (pt_csr.psp.cnt_pr_pbus_sop.all() != 0) {
        PLOG_ERR("pt_csr.cnt_pr_pbus_sop.cnt = 0x" << hex << pt_csr.psp.cnt_pr_pbus_sop.all() << ", vs exp = 0x0!" << endl)
        pt_csr.psp.cnt_pr_pbus_sop.show();
    }

    pt_csr.psp.cnt_pr_pbus_eop.read();
    if (pt_csr.psp.cnt_pr_pbus_eop.all() != 0) {
        PLOG_ERR("pt_csr.cnt_pr_pbus_eop.cnt = 0x" << hex << pt_csr.psp.cnt_pr_pbus_eop.all() << ", vs exp = 0x0!" << endl)
        pt_csr.psp.cnt_pr_pbus_eop.show();
    }

    pt_csr.psp.cnt_pr_pbus_recirc.read();
    if (pt_csr.psp.cnt_pr_pbus_recirc.all() != 0) {
        PLOG_ERR("pt_csr.cnt_pr_pbus_recirc.cnt = 0x" << hex << pt_csr.psp.cnt_pr_pbus_recirc.all() << ", vs exp = 0x0!" << endl)
        pt_csr.psp.cnt_pr_pbus_recirc.show();
    }

    pt_csr.psp.cnt_pr_resub_pbus_xns.read();
    if (pt_csr.psp.cnt_pr_resub_pbus_xns.all() != 0) {
        PLOG_ERR("pt_csr.cnt_pr_resub_pbus_xns.cnt = 0x" << hex << pt_csr.psp.cnt_pr_resub_pbus_xns.all() << ", vs exp = 0x0!" << endl)
        pt_csr.psp.cnt_pr_resub_pbus_xns.show();
    }

    pt_csr.psp.cnt_pr_resub_pbus_sop.read();
    if (pt_csr.psp.cnt_pr_resub_pbus_sop.all() != 0) {
        PLOG_ERR("pt_csr.cnt_pr_resub_pbus_sop.cnt = 0x" << hex << pt_csr.psp.cnt_pr_resub_pbus_sop.all() << ", vs exp = 0x0!" << endl)
        pt_csr.psp.cnt_pr_resub_pbus_sop.show();
    }

    pt_csr.psp.cnt_pr_resub_pbus_eop.read();
    if (pt_csr.psp.cnt_pr_resub_pbus_eop.all() != 0) {
        PLOG_ERR("pt_csr.cnt_pr_resub_pbus_eop.cnt = 0x" << hex << pt_csr.psp.cnt_pr_resub_pbus_eop.all() << ", vs exp = 0x0!" << endl)
        pt_csr.psp.cnt_pr_resub_pbus_eop.show();
    }

    pt_csr.psp.cnt_pr_resub_xns.read();
    pt_csr.psp.cnt_pr_resub_sop.read();
    pt_csr.psp.cnt_pr_resub_eop.read();
    if ((pt_csr.psp.cnt_pr_resub_sop.all() & 0xff) != pt_csr.psp.cnt_pr_resub_eop.all()) {
        PLOG_ERR("pt_csr.cnt_pr_resub.sop = 0x" << hex << pt_csr.psp.cnt_pr_resub_sop.all() << ", vs eop = 0x" << pt_csr.psp.cnt_pr_resub_eop.all() << endl)
    }
    sknobs_set_value((char*)"pt_csr/psp/cnt_pr_resub/sop", pt_csr.psp.cnt_pr_resub_sop.all().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pt_csr/psp/cnt_pr_resub/xns", pt_csr.psp.cnt_pr_resub_xns.all().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pt_csr/psp/cnt_pr_resub/no_data", pt_csr.psp.cnt_pr_resub_no_data.all().convert_to<unsigned long long>());

    pt_csr.psp.sat_psp_csr_read_access_err.read();
    if (pt_csr.psp.sat_psp_csr_read_access_err.all() != 0) {
        PLOG_ERR("pt_csr.sat_psp_csr.read_access_err = 0x" << hex << pt_csr.psp.sat_psp_csr_read_access_err.all() << endl)
    }
    pt_csr.psp.sat_psp_csr_write_access_err.write();
    if (pt_csr.psp.sat_psp_csr_write_access_err.all() != 0) {
        PLOG_ERR("pt_csr.sat_psp_csr.write_access_err = 0x" << hex << pt_csr.psp.sat_psp_csr_write_access_err.all() << endl)
    }
    if (((pt_csr.psp.cnt_sw_sop.all() + pt_csr.psp.cnt_pb_pbus_sop.all() + pt_csr.psp.cnt_pr_resub_sop.all())&0xffffffffff) != ((pt_csr.psp.cnt_ma_sop.all() + pt_csr.psp.cnt_ma_collapsed.all()) & 0xffffffffff)) {
        PLOG_ERR("pt_csr.cnt_sw = " << pt_csr.psp.cnt_sw_sop.all() << "+ cnt_pb_pbus = " << pt_csr.psp.cnt_pb_pbus_sop.all() << "+ cnt_pt_resub = " << pt_csr.psp.cnt_pr_resub_sop.all() << " != cnt_ma = " << pt_csr.psp.cnt_ma_sop.all() << " + collapsed = " << pt_csr.psp.cnt_ma_collapsed.all() << endl)
    }
    if (pt_csr.psp.cnt_pr_resub_sop.all() != pt_csr.psp.cnt_ma_recirc.all()) {
        PLOG_ERR("pt_csr.cnt_pr_resub = " << pt_csr.psp.cnt_pr_resub_sop.all() << " != cnt_ma = " << pt_csr.psp.cnt_ma_recirc.all() << endl)
    }

    if (!sknobs_exists((char*)"pt_csr/psp/disable_err_cnt_check")) {

        pt_csr.psp.sat_lif_table_ecc_err.read();
        if (pt_csr.psp.sat_lif_table_ecc_err.all() != 0) {
            PLOG_ERR("pt_csr.sat_lif_table_ecc_err should be 0" << endl)
            pt_csr.psp.sat_lif_table_ecc_err.show();
        }

        pt_csr.psp.sat_lif_table_qid_err.read();
        if (pt_csr.psp.sat_lif_table_qid_err.all() != 0) {
            PLOG_ERR("pt_csr.sat_lif_table_qid_err should be 0" << endl)
            pt_csr.psp.sat_lif_table_qid_err.show();
        }

        pt_csr.psp.sat_pb_pbus_sop_err.read();
        if (pt_csr.psp.sat_pb_pbus_sop_err.all() != 0) {
            PLOG_ERR("sat_pb_pbus_sop_err expect to be 0" << endl)
            pt_csr.psp.sat_pb_pbus_sop_err.show();
        }

        pt_csr.psp.sat_pb_pbus_eop_err.read();
        if (pt_csr.psp.sat_pb_pbus_eop_err.all() != 0) {
            PLOG_ERR("sat_pb_pbus_eop_err expect to be 0" << endl)
            pt_csr.psp.sat_pb_pbus_eop_err.show();
        }

        pt_csr.psp.sat_pb_pbus_err.read();
        if (pt_csr.psp.sat_pb_pbus_err.all() != 0) {
            PLOG_ERR("sat_pb_pbus_err expect to be 0" << endl)
            pt_csr.psp.sat_pb_pbus_err.show();
        }

        pt_csr.psp.sat_pb_pbus_dummy.read();
        if (pt_csr.psp.sat_pb_pbus_dummy.all() != 0) {
            PLOG_ERR("sat_pb_pbus_dummy expect to be 0" << endl)
            pt_csr.psp.sat_pb_pbus_dummy.show();
        }

        pt_csr.psp.sat_pb_pbus_nopkt.read();
        if (pt_csr.psp.sat_pb_pbus_nopkt.all() != 0) {
            PLOG_ERR("sat_pb_pbus_nopkt expect to be 0" << endl)
            pt_csr.psp.sat_pb_pbus_nopkt.show();
        }

        pt_csr.psp.sat_ma_sop_err.read();
        if (pt_csr.psp.sat_ma_sop_err.all() != 0) {
            PLOG_ERR("sat_ma_sop_err expect to be 0" << endl)
            pt_csr.psp.sat_ma_sop_err.show();
        }

        pt_csr.psp.sat_ma_eop_err.read();
        if (pt_csr.psp.sat_ma_eop_err.all() != 0) {
            PLOG_ERR("sat_ma_eop_err expect to be 0" << endl)
            pt_csr.psp.sat_ma_eop_err.show();
        }

        pt_csr.psp.sat_pr_pbus_sop_err.read();
        if (pt_csr.psp.sat_pr_pbus_sop_err.all() != 0) {
            PLOG_ERR("sat_pr_pbus_sop_err expect to be 0" << endl)
            pt_csr.psp.sat_pr_pbus_sop_err.show();
        }

        pt_csr.psp.sat_pr_pbus_eop_err.read();
        if (pt_csr.psp.sat_pr_pbus_eop_err.all() != 0) {
            PLOG_ERR("sat_pr_pbus_eop_err expect to be 0" << endl)
            pt_csr.psp.sat_pr_pbus_eop_err.show();
        }

        pt_csr.psp.sat_pr_pbus_err.read();
        if (pt_csr.psp.sat_pr_pbus_err.all() != 0) {
            PLOG_ERR("sat_pr_pbus_err expect to be 0" << endl)
            pt_csr.psp.sat_pr_pbus_err.show();
        }

        pt_csr.psp.sat_pr_resub_pbus_sop_err.read();
        if (pt_csr.psp.sat_pr_resub_pbus_sop_err.all() != 0) {
            PLOG_ERR("sat_pr_resub_pbus_sop_err expect to be 0" << endl)
            pt_csr.psp.sat_pr_resub_pbus_sop_err.show();
        }

        pt_csr.psp.sat_pr_resub_pbus_eop_err.read();
        if (pt_csr.psp.sat_pr_resub_pbus_eop_err.all() != 0) {
            PLOG_ERR("sat_pr_resub_pbus_eop_err expect to be 0" << endl)
            pt_csr.psp.sat_pr_resub_pbus_eop_err.show();
        }

        pt_csr.psp.sat_pr_resub_pbus_err.read();
        if (pt_csr.psp.sat_pr_resub_pbus_err.all() != 0) {
            PLOG_ERR("sat_pr_resub_pbus_err expect to be 0" << endl)
            pt_csr.psp.sat_pr_resub_pbus_err.show();
        }

        pt_csr.psp.sat_pr_resub_sop_err.read();
        if (pt_csr.psp.sat_pr_resub_sop_err.all() != 0) {
            PLOG_ERR("sat_pr_resub_sop_err expect to be 0" << endl)
            pt_csr.psp.sat_pr_resub_sop_err.show();
        }

        pt_csr.psp.sat_pr_resub_eop_err.read();
        if (pt_csr.psp.sat_pr_resub_eop_err.all() != 0) {
            PLOG_ERR("sat_pr_resub_eop_err expect to be 0" << endl)
            pt_csr.psp.sat_pr_resub_eop_err.show();
        }

    }
}

void cap_npv_eos_int_sim(int chip_id, int inst_id) {
    cap_pt_csr_t &pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, chip_id, inst_id);
    string  csr_name;
     
    if (sknobs_exists((char*)"no_int_chk") == 0) {
        pt_csr.psp.int_sw_phv_mem.intreg.read();
        pt_csr.psp.int_sw_phv_mem.intreg.show();
        if (sknobs_exists((char*)"PEN_IP_MEM_ECC_CORRUPT_ENABLE")) {
            if (sknobs_exists((char*)"no_ecc_chk") == 0) {
                if (pt_csr.psp.int_sw_phv_mem.intreg.ecc_correctable_interrupt() != 1)
                    PLOG_ERR("pt.npv.int_sw_phv_mem.ecc_correctable should be 1!" << endl)
            }
        }
        else {
            if (pt_csr.psp.int_sw_phv_mem.intreg.ecc_correctable_interrupt() != 0)
                PLOG_ERR("pt.npv.int_sw_phv_mem.ecc_correctable should be 0!" << endl)
        }
        if (pt_csr.psp.int_sw_phv_mem.intreg.ecc_uncorrectable_interrupt() != 0)
            PLOG_ERR("pt.npv.int_sw_phv_mem.ecc_uncorrectable should be 0!" << endl)
        if (pt_csr.psp.int_sw_phv_mem.intreg.phv_invalid_sram_interrupt() != 0)
            PLOG_ERR("pt.npv.int_sw_phv_mem.phv_invalid_sram should be 0!" << endl)
        if (pt_csr.psp.int_sw_phv_mem.intreg.phv_invalid_data_interrupt() != 0)
            PLOG_ERR("pt.npv.int_sw_phv_mem.phv_invalid_data should be 0!" << endl)
        pt_csr.psp.int_info.intreg.read();
        pt_csr.psp.int_info.intreg.show();

        pt_csr.psp.int_err.intreg.read_compare();
        pt_csr.psp.int_err.intreg.show();

        pt_csr.psp.int_fatal.intreg.read_compare();
        pt_csr.psp.int_fatal.intreg.show();

        pt_csr.psp.int_lif_qstate_map.intreg.read();
        pt_csr.psp.int_lif_qstate_map.intreg.show();
        if (sknobs_exists((char*)"PEN_IP_MEM_ECC_CORRUPT_ENABLE")) {
            if (sknobs_exists((char*)"no_ecc_chk") == 0) {
                if (pt_csr.psp.int_lif_qstate_map.intreg.ecc_correctable_interrupt() != 1)
                    PLOG_ERR("pt.npv.int_lif_qstate_map.ecc_correctable should be 1!" << endl)
            }
        }
        else {
            if (pt_csr.psp.int_lif_qstate_map.intreg.ecc_correctable_interrupt() != 0)
                PLOG_ERR("pt.npv.int_lif_qstate_map.ecc_correctable should be 0!" << endl)
        }
        if (pt_csr.psp.int_lif_qstate_map.intreg.ecc_uncorrectable_interrupt() != 0)
            PLOG_ERR("pt.npv.int_lif_qstate_map.ecc_uncorrectable should be 0!" << endl)
        if (pt_csr.psp.int_lif_qstate_map.intreg.qid_invalid_interrupt() != 0)
            PLOG_ERR("pt.npv.int_lif_qstate_map.qid_invalid should be 0!" << endl)
    }
    else {
        pt_csr.psp.int_sw_phv_mem.intreg.read();
        pt_csr.psp.int_sw_phv_mem.intreg.show();

        pt_csr.psp.int_info.intreg.read();
        pt_csr.psp.int_info.intreg.show();

        pt_csr.psp.int_err.intreg.read_compare();
        pt_csr.psp.int_err.intreg.show();

        pt_csr.psp.int_fatal.intreg.read_compare();
        pt_csr.psp.int_fatal.intreg.show();

        pt_csr.psp.int_lif_qstate_map.intreg.read();
        pt_csr.psp.int_lif_qstate_map.intreg.show();

    }

}

void cap_npv_eos_int(int chip_id, int inst_id) {
    cap_pt_csr_t &pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, chip_id, inst_id);
    
    pt_csr.psp.int_err.intreg.read_compare();
    pt_csr.psp.int_fatal.intreg.read_compare();
}

void cap_npv_eos_sta(int chip_id, int inst_id) {
    cap_pt_csr_t &pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, chip_id, inst_id);

    pt_csr.psp.sta.read();
    if (pt_csr.psp.sta.empty() != 1)
        PLOG_ERR("pt_csr.npv.sta.empty should be 1!" << endl)
    if (pt_csr.psp.sta.pkt_empty() != 1)
        PLOG_ERR("pt_csr.npv.sta.pkt_empty should be 1!" << endl)
    if (pt_csr.psp.sta.phv_empty() != 1)
        PLOG_ERR("pt_csr.npv.sta.phv_empty should be 1!" << endl)
    if (pt_csr.psp.sta.pkt_inflight() != 0)
        PLOG_ERR("pt_csr.npv.sta.pkt_inflight should be 0 vs " << pt_csr.psp.sta.pkt_inflight() << "!" << endl)
    if (pt_csr.psp.sta.phv_inflight() != 0)
        PLOG_ERR("pt_csr.npv.sta.phv_inflight should be 0! vs " << pt_csr.psp.sta.phv_inflight() << "!" << endl)

    pt_csr.psp.sta_fifo.read();
    if (pt_csr.psp.sta_fifo.bus_pkt_empty() != 1)
        PLOG_ERR("pt_csr.npv.sta_fifo.bus_pkt_empty should be 1!" << endl)
    if (pt_csr.psp.sta_fifo.sv10_ff_pkt_srdy() != 0)
        PLOG_ERR("pt_csr.npv.sta_fifo.sv10_ff_pkt_srdy should be 0!" << endl)
    if (pt_csr.psp.sta_fifo.sv10_ff_rsp_srdy() != 0)
        PLOG_ERR("pt_csr.npv.sta_fifo.sv10_ff_rsp_srdy should be 0!" << endl)
    if (pt_csr.psp.sta_fifo.sv12_dc_phv_srdy() != 0)
        PLOG_ERR("pt_csr.npv.sta_fifo.sv12_dc_phv_srdy should be 0!" << endl)
    if (pt_csr.psp.sta_fifo.sv12_dc_pkt_srdy() != 0)
        PLOG_ERR("pt_csr.npv.sta_fifo.sv12_dc_pkt_srdy should be 0!" << endl)
    if (pt_csr.psp.sta_fifo.lpbk_phv_ff_empty() != 1)
        PLOG_ERR("pt_csr.npv.sta_fifo.lpbk_phv_ff_empty should be 1!" << endl)
    if (pt_csr.psp.sta_fifo.sv13_lb_pkt_srdy() != 0)
        PLOG_ERR("pt_csr.npv.sta_fifo.sv13_lb_pkt_srdy should be 0!" << endl)
    if (pt_csr.psp.sta_fifo.out_phv_empty() != 1)
        PLOG_ERR("pt_csr.npv.sta_fifo.out_phv_empty should be 1!" << endl)
    if (pt_csr.psp.sta_fifo.sv21_ma_phv_srdy() != 0)
        PLOG_ERR("pt_csr.npv.sta_fifo.sv21_ma_phv_srdy should be 0!" << endl)
    if (pt_csr.psp.sta_fifo.pkt_order_pop_srdy() != 0)
        PLOG_ERR("pt_csr.npv.sta_fifo.pkt_order_pop_srdy should be 0!" << endl)
    if (pt_csr.psp.sta_fifo.sv21_pr_pkt_srdy() != 0)
        PLOG_ERR("pt_csr.npv.sta_fifo.sv21_pr_pkt_srdy should be 0!" << endl)  
            
    pt_csr.psp.sta_flow.read();
    if (pt_csr.psp.sta_flow.ma_drdy() != 1)
        PLOG_ERR("pt_csr.psp.sta_flow.ma_drdy should be 1!" << endl)
    if (pt_csr.psp.sta_flow.pb_pbus_drdy() != 1)
        PLOG_ERR("pt_csr.psp.sta_flow.pb_pbus_drdy should be 1!" << endl)
    if (pt_csr.psp.sta_flow.pkt_order_push_drdy() != 1)
        PLOG_ERR("pt_csr.psp.sta_flow.pkt_order_push_drdy should be 1!" << endl)
    if (pt_csr.psp.sta_flow.pr_resub_drdy() != 1)
        PLOG_ERR("pt_csr.psp.sta_flow.pr_resub_drdy should be 1!" << endl)
    if (pt_csr.psp.sta_flow.pr_resub_pbus_drdy() != 1)
        PLOG_ERR("pt_csr.psp.sta_flow.pr_resub_pbus_drdy should be 1!" << endl)
    if (pt_csr.psp.sta_flow.sv01_pb_pkt_drdy() != 1)
        PLOG_ERR("pt_csr.psp.sv01_pb_pkt_drdy should be 1!" << endl)
    if (pt_csr.psp.sta_flow.sv01_lb_phv_drdy() != 1)
        PLOG_ERR("pt_csr.psp.sta_flow.sv01_lb_phv_drdy should be 1!" << endl)
    if (pt_csr.psp.sta_flow.sv02_lb_pkt_drdy() != 1)
        PLOG_ERR("pt_csr.psp.sta_flow.sv02_lb_pkt_drdy should be 1!" << endl)
    if (pt_csr.psp.sta_flow.sv04_lt_ovr_drdy() != 1)
        PLOG_ERR("pt_csr.psp.sta_flow.sv04_lt_ovr_drdy should be 1!" << endl)
    if (pt_csr.psp.sta_flow.sv04_lt_rsp_drdy() != 1)
        PLOG_ERR("pt_csr.psp.sta_flow.sv04_lt_rsp_drdy should be 1!" << endl)
    if (pt_csr.psp.sta_flow.sv11_in_phv_drdy() != 1)
        PLOG_ERR("pt_csr.psp.sta_flow.sv11_in_phv_drdy should be 1!" << endl)
    if (pt_csr.psp.sta_flow.sv11_in_pkt_drdy() != 1)
        PLOG_ERR("pt_csr.psp.sta_flow.sv11_in_pkt_drdy should be 1!" << endl)
    if (pt_csr.psp.sta_flow.sv14_ma_phv_drdy() != 1)
        PLOG_ERR("pt_csr.psp.sta_flow.sv14_ma_phv_drdy should be 1!" << endl)
    if (pt_csr.psp.sta_flow.sv15_pr_pkt_drdy() != 1)
        PLOG_ERR("pt_csr.psp.sta_flow.sv15_pr_pkt_drdy should be 1!" << endl)
    if (pt_csr.psp.sta_flow.sv20_ma_phv_drdy() != 1)
        PLOG_ERR("pt_csr.psp.sta_flow.sv20_ma_phv_drdy should be 1!" << endl)
    if (pt_csr.psp.sta_flow.sv21_ma_phv_drdy() != 1)
        PLOG_ERR("pt_csr.psp.sta_flow.sv21_ma_phv_drdy should be 1!" << endl)
    if (pt_csr.psp.sta_flow.pr_pkt_ff_almost_full() != 0)
        PLOG_ERR("pt_csr.psp.sta_flow.pr_pkt_ff_almost_full should be 0!" << endl)
    if (pt_csr.psp.sta_flow.ptd_npv_phv_full() != 0)
        PLOG_ERR("pt_csr.psp.sta_flow.ptd_npv_phv_full should be 0!" << endl)
    
    pt_csr.psp.sta_fsm.read();
    if (pt_csr.psp.sta_fsm.split_state() != 0)
        PLOG_ERR("pt_csr.psp.sta_fsm.split_state should be 0!" << endl)
    if (pt_csr.psp.sta_fsm.phv_state() != 0)
        PLOG_ERR("pt_csr.psp.sta_fsm.phv_state should be 0!" << endl)
    if (pt_csr.psp.sta_fsm.pkt_state() != 0)
        PLOG_ERR("pt_csr.pps.sta_fsm.pkt_state should be 0!" << endl)
    if (pt_csr.psp.sta_fsm.sv01_pb_pkt_in_pkt() != 0)
        PLOG_ERR("pt_csr.psp.sta_fsm.sv01_pb_pkt_in_pkt should be 0!" << endl)
    if (pt_csr.psp.sta_fsm.sv03_lt_req_in_progress() != 0)
        PLOG_ERR("pt_csr.psp.sta_fsm.sv03_lt_req_in_progress should be 0!" << endl)
    if (pt_csr.psp.sta_fsm.sv03_sw_req_in_pkt() != 0)
        PLOG_ERR("pt_csr.psp.sta_fsm.sv03_sw_req_in_pkt should be 0!" << endl)
    if (pt_csr.psp.sta_fsm.sv13_lb_phv_in_pkt() != 0)
        PLOG_ERR("pt_csr.psp.sta_fsm.sv13_lb_phv_in_pkt should be 0!" << endl)
    if (pt_csr.psp.sta_fsm.sv13_lb_pkt_in_pkt() != 0)
        PLOG_ERR("pt_csr.psp.sta_fsm.sv13_lb_pkt_in_pkt should be 0!" << endl)
    if (pt_csr.psp.sta_fsm.sv15_pr_pkt_in_pkt() != 0)
        PLOG_ERR("pt_csr.psp.sta_fsm.sv15_pr_pkt_in_pkt should be 0!" << endl)
    if (pt_csr.psp.sta_fsm.sv20_ma_phv_in_pkt() != 0)
        PLOG_ERR("pt_csr.psp.sta_fsm.sv20_ma_phv_in_pkt should be 0!" << endl)
}

/////self defined
void cap_npv_lif_qstate_map(int lif, cap_lif_qstate_data_bundle_t lif_qstate_entry) {
    int chip_id = 0;
    int inst_id = 0;

    cap_pt_csr_t &pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, chip_id, inst_id);
    
    pt_csr.psp.dhs_lif_qstate_map.entry[lif].all(lif_qstate_entry.all());
    pt_csr.psp.dhs_lif_qstate_map.entry[lif].write();
}

void cap_npv_sw_phv_mem(int index, cap_psp_csr_dhs_sw_phv_mem_entry_t  sw_phv_mem_entry) {
    int chip_id = 0;
    int inst_id = 0;

    cap_pt_csr_t &pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, chip_id, inst_id);

    pt_csr.psp.dhs_sw_phv_mem.entry[index].all(sw_phv_mem_entry.all());
    pt_csr.psp.dhs_sw_phv_mem.entry[index].write();
    pt_csr.psp.dhs_sw_phv_mem.entry[index].show();
}

void cap_npv_cfg_sw_phv_control(int profile_id, int data) {
    int chip_id = 0;
    int inst_id = 0;

    cap_pt_csr_t &pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, chip_id, inst_id);

    pt_csr.psp.cfg_sw_phv_control[profile_id].all(data);
    pt_csr.psp.cfg_sw_phv_control[profile_id].write();
    pt_csr.psp.cfg_sw_phv_control[profile_id].show();
}

void cap_npv_cfg_sw_phv_config(int profile_id, cap_psp_csr_cfg_sw_phv_config_t sw_phv_config) {
    int chip_id = 0;
    int inst_id = 0;

    cap_pt_csr_t &pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, chip_id, inst_id);

    pt_csr.psp.cfg_sw_phv_config[profile_id].all(sw_phv_config.all());
    pt_csr.psp.cfg_sw_phv_config[profile_id].write();
    pt_csr.psp.cfg_sw_phv_config[profile_id].show();
}

void cap_npv_sw_phv_start_enable(int profile_id, int enable) {
    int chip_id = 0;
    int inst_id = 0;

    cap_pt_csr_t &pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, chip_id, inst_id);

    pt_csr.psp.cfg_sw_phv_control[profile_id].read();
    pt_csr.psp.cfg_sw_phv_control[profile_id].start_enable(enable);
    pt_csr.psp.cfg_sw_phv_control[profile_id].write();
    pt_csr.psp.cfg_sw_phv_control[profile_id].read();
}

void cap_npv_sw_phv_status(int profile_id, uint64_t &data) {
    int chip_id = 0;
    int inst_id = 0;

    cap_pt_csr_t &pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, chip_id, inst_id);

    pt_csr.psp.sta_sw_phv_state[profile_id].read();
    data = pt_csr.psp.sta_sw_phv_state[profile_id].all().convert_to<uint64_t>();
    PLOG_MSG("profile_id = " << profile_id << endl)
    pt_csr.psp.sta_sw_phv_state[profile_id].show();
}

void cap_npv_sw_phv_display() {
    int chip_id = 0;
    int inst_id = 0;

    cap_pt_csr_t &pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, chip_id, inst_id);

    PLOG_MSG("############################[SW PHV CFG]#########################################" << endl)
    PLOG_MSG(setw(2) << "prof_id" << setw(10) << "#sta_en" << setw(10) << "#cnt_rpt_en" << setw(10) << "#ltime_en" << setw(10) << "#frm_size_en" << setw(8) << "#sta_addr" << setw(8) << "#num_flits" << setw(8) << "#insert_prd" << setw(10) << "#cnt_max" << setw(10) << endl)
    for (int i = 0; i < 8; i++) {
        PLOG_MSG(setw(2) << i << setw(10) << pt_csr.psp.cfg_sw_phv_control[i].start_enable() << setw(10) << pt_csr.psp.cfg_sw_phv_control[i].counter_repeat_enable() << setw(10) << pt_csr.psp.cfg_sw_phv_control[i].localtime_enable() << setw(10) << pt_csr.psp.cfg_sw_phv_control[i].frame_size_enable() << setw(16) << pt_csr.psp.cfg_sw_phv_config[i].start_addr() << setw(10) << pt_csr.psp.cfg_sw_phv_config[i].num_flits() << setw(10) << pt_csr.psp.cfg_sw_phv_config[i].insertion_period_clocks() << setw(10) <<  pt_csr.psp.cfg_sw_phv_config[i].counter_max() << endl)
    }
    PLOG_MSG("############################[END OF SW PHV CFG]##################################" << endl)
}

void cap_npv_sw_prof_cfg(int profile_id, cpp_int *sw_phv_data, int num_flits, int cnt_rep_en, int localtime_en, int frm_sz_en, uint64_t insert_period, int counter_max) {
    int chip_id = 0;
    int inst_id = 0;
    int sta_addr;
    cpp_int flit_data;

    cap_pt_csr_t &pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, chip_id, inst_id);

    pt_csr.psp.cfg_sw_phv_control[profile_id].start_enable(0);
    pt_csr.psp.cfg_sw_phv_control[profile_id].counter_repeat_enable(cnt_rep_en);
    pt_csr.psp.cfg_sw_phv_control[profile_id].qid_repeat_enable(0);
    pt_csr.psp.cfg_sw_phv_control[profile_id].localtime_enable(localtime_en);
    pt_csr.psp.cfg_sw_phv_control[profile_id].frame_size_enable(frm_sz_en);
    pt_csr.psp.cfg_sw_phv_control[profile_id].packet_len_enable(0);
    pt_csr.psp.cfg_sw_phv_control[profile_id].qid_enable(0);
    pt_csr.psp.cfg_sw_phv_control[profile_id].write();
    pt_csr.psp.cfg_sw_phv_control[profile_id].show();

    if (profile_id == 0)
        sta_addr = 0; 
    else
        sta_addr = pt_csr.psp.cfg_sw_phv_config[profile_id-1].start_addr().convert_to<int>() + pt_csr.psp.cfg_sw_phv_config[profile_id-1].num_flits().convert_to<int>() + 1; 

    pt_csr.psp.cfg_sw_phv_config[profile_id].start_addr(sta_addr);
    pt_csr.psp.cfg_sw_phv_config[profile_id].num_flits(num_flits - 1);
    pt_csr.psp.cfg_sw_phv_config[profile_id].insertion_period_clocks(insert_period);
    pt_csr.psp.cfg_sw_phv_config[profile_id].counter_max(counter_max);
    pt_csr.psp.cfg_sw_phv_config[profile_id].qid_min(0);
    pt_csr.psp.cfg_sw_phv_config[profile_id].qid_max(0);
    pt_csr.psp.cfg_sw_phv_config[profile_id].write();
    pt_csr.psp.cfg_sw_phv_config[profile_id].show();

    for (int i = 0; i < num_flits; i++) {
        flit_data = cpp_int_helper::s_get_slc(*sw_phv_data, 512, i*512);
        pt_csr.psp.dhs_sw_phv_mem.entry[sta_addr + i].all(flit_data);
        pt_csr.psp.dhs_sw_phv_mem.entry[sta_addr + i].write();
        pt_csr.psp.dhs_sw_phv_mem.entry[sta_addr + i].show();
    }
}

void display_npv_phv_mem_out() {
    cap_pt_csr_t &pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, 0, 0);
    cap_phv_intr_txdma_t  txdma_intr;

    for (int i = 0; i < 16; i++) {
        pt_csr.psp.dhs_psp_out_mem.entry[i].read();
        PLOG_MSG("npv_phv_mem[" << i << "]: sop = " << pt_csr.psp.dhs_psp_out_mem.entry[i].sop().convert_to<int>() << ", eop = " << pt_csr.psp.dhs_psp_out_mem.entry[i].eop().convert_to<int>() << ", err = " << pt_csr.psp.dhs_psp_out_mem.entry[i].err().convert_to<int>() << endl)
        PLOG_MSG("data = 0x" << hex << pt_csr.psp.dhs_psp_out_mem.entry[i].data() << endl)
        if (pt_csr.psp.dhs_psp_out_mem.entry[i].sop().convert_to<int>() == 1) {
            txdma_intr.all(pt_csr.psp.dhs_psp_out_mem.entry[i].data());
            txdma_intr.show();
        }
    }
}

void clear_npv_phv_mem_out() {
    cap_pt_csr_t &pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, 0, 0);
    
    for (int i = 0; i < 16; i++) {
        pt_csr.psp.dhs_psp_out_mem.entry[i].all(0);
        pt_csr.psp.dhs_psp_out_mem.entry[i].write();
    }
}

void cap_npv_dump_cntr() {
    cap_pt_csr_t &pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, 0, 0);
    
    pt_csr.psp.cnt_pb_pbus_sop.read();
    pt_csr.psp.cnt_sw_sop.read();
    pt_csr.psp.cnt_ma_recirc.read();
    pt_csr.psp.cnt_ma_sop.read();
    pt_csr.psp.cnt_ma_no_data.read();
    pt_csr.psp.cnt_ma_collapsed.read();
    pt_csr.psp.cnt_ma_drop.read();
    pt_csr.psp.cnt_ptd_npv_phv_full.read();

    NPV_PRN_MSG(setw(1) << "========================================================= TXDMA NPV CNTRS ==============================================================\n");
    NPV_PRN_MSG(setw(32) << "cnt_txs_sch_event" << setw(10) << pt_csr.psp.cnt_pb_pbus_sop.all().convert_to<unsigned int>() << "\n");
    NPV_PRN_MSG(setw(32) << "cnt_sw_sop" << setw(10) << pt_csr.psp.cnt_sw_sop.all().convert_to<unsigned int>() << "\n");
    NPV_PRN_MSG(setw(32) << "cnt_ma_recirc" << setw(10) << pt_csr.psp.cnt_ma_recirc.all().convert_to<unsigned int>() << "\n");
    NPV_PRN_MSG(setw(32) << "cnt_ma_sop" << setw(10) << pt_csr.psp.cnt_ma_sop.all().convert_to<unsigned int>() << "\n");
    NPV_PRN_MSG(setw(32) << "cnt_ma_no_data" << setw(10) << pt_csr.psp.cnt_ma_no_data.all().convert_to<unsigned int>() << "\n");
    NPV_PRN_MSG(setw(32) << "cnt_ma_collapsed" << setw(10) << pt_csr.psp.cnt_ma_collapsed.all().convert_to<unsigned int>() << "\n");
    NPV_PRN_MSG(setw(32) << "cnt_ma_drop" << setw(10) << pt_csr.psp.cnt_ma_drop.all().convert_to<unsigned int>() << "\n");
    NPV_PRN_MSG(setw(32) << "cnt_ptd_npv_phv_full" << setw(10) << pt_csr.psp.cnt_ptd_npv_phv_full.all().convert_to<unsigned int>() << "\n");

}

void cap_npv_clr_cntr() {
    cap_pt_csr_t &pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, 0, 0);

    pt_csr.psp.cnt_pb_pbus_sop.all(0);
    pt_csr.psp.cnt_sw_sop.all(0);
    pt_csr.psp.cnt_ma_recirc.all(0);
    pt_csr.psp.cnt_ma_sop.all(0);
    pt_csr.psp.cnt_ma_no_data.all(0);
    pt_csr.psp.cnt_ma_collapsed.all(0);
    pt_csr.psp.cnt_ma_drop.all(0);
    pt_csr.psp.cnt_ptd_npv_phv_full.all(0);

    pt_csr.psp.cnt_pb_pbus_sop.write();
    pt_csr.psp.cnt_sw_sop.write();
    pt_csr.psp.cnt_ma_recirc.write();
    pt_csr.psp.cnt_ma_sop.write();
    pt_csr.psp.cnt_ma_no_data.write();
    pt_csr.psp.cnt_ma_collapsed.write();
    pt_csr.psp.cnt_ma_drop.write();
    pt_csr.psp.cnt_ptd_npv_phv_full.write();

}

void npv_print_msg(string msg) {
#ifdef _CSV_INCLUDED_
   vpi_printf((PLI_BYTE8*) "%s", msg.c_str());
#else
   std::cout << msg;
#endif
}

