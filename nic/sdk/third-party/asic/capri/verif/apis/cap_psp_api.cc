#include "cap_psp_api.h"

// soft reset sequence 
void cap_psp_soft_reset(int chip_id, int inst_id) {
}

// soft reset, value = 1 means apply reset, value = 0 release soft reset
void cap_psp_set_soft_reset(int chip_id, int inst_id, int value) {
}

// init start, no polling yet
void cap_psp_init_start(int chip_id, int inst_id) {
    cap_pr_csr_t & pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, chip_id, inst_id);
    
    pr_csr.psp.cfg_profile.ptd_npv_phv_full_enable(1);
    pr_csr.psp.cfg_profile.recirc_max(7);
    pr_csr.psp.cfg_profile.recirc_max_enable(1);
    pr_csr.psp.cfg_profile.recirc_inc_enable(1);
    pr_csr.psp.cfg_profile.swphv_enable(1);   
    pr_csr.psp.cfg_profile.pb_err_enable(1);
    pr_csr.psp.cfg_profile.error_enable(1);
    pr_csr.psp.cfg_profile.phv_min_drop_enable(1);
    pr_csr.psp.cfg_profile.phv_min(32);
    pr_csr.psp.cfg_profile.pr_frame_size_enable(1);
    pr_csr.psp.cfg_profile.write();
    pr_csr.psp.base.read();
}

// poll for init done
void cap_psp_init_done(int chip_id, int inst_id) {
    cap_pr_csr_t & pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, chip_id, inst_id);

    cap_psp_config(chip_id, inst_id);

    pr_csr.psp.base.read();
}

// use sknobs base load cfg 
void cap_psp_load_from_cfg(int chip_id, int inst_id) {
    cap_pr_csr_t & pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, chip_id, inst_id);
    pu_cpp_int<512>      reset_value = ~0x0;
    
    PLOG_MSG("psp load from config..." << endl)
    
    for (int i = 0; i < 16; i++)
        pr_csr.psp.dhs_sw_phv_mem.entry[i].set_reset_val(reset_value);
    for (int i = 0; i < 8; i++)
        pr_csr.psp.cfg_sw_phv_control[i].set_reset_val(reset_value);
    pr_csr.psp.load_from_cfg();

    pr_csr.psp.base.read();
}

// eos check, this functions should call eos_cnt, eos_int, eos_sta 
void cap_psp_eos(int chip_id, int inst_id) {
    cap_psp_eos_cnt(chip_id, inst_id);
    cap_psp_eos_int_sim(chip_id, inst_id);
    cap_psp_eos_sta(chip_id, inst_id);
}

void cap_psp_eos_cnt(int chip_id, int inst_id) {
    int no_data_hw_set;
    int no_data_hw_reset;
    int min_phv_drop;

    cap_pr_csr_t &pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, chip_id, inst_id);

    pr_csr.psp.cnt_sw_xns.read();
    pr_csr.psp.cnt_sw_sop.read();
    pr_csr.psp.cnt_sw_eop.read();
    if ((pr_csr.psp.cnt_sw_sop.all() & 0xff) != pr_csr.psp.cnt_sw_eop.all()) {
        PLOG_ERR("pr_csr.psp.cnt_sw_sop.all = 0x" << hex << pr_csr.psp.cnt_sw_sop.all() << ", vs eop = 0x" << pr_csr.psp.cnt_sw_eop.all() << endl)
    }
    sknobs_set_value((char*)"pr_csr/psp/cnt_sw/sop", pr_csr.psp.cnt_sw_sop.all().convert_to<unsigned long long>()); 
    sknobs_set_value((char*)"pr_csr/psp/cnt_sw/xns", pr_csr.psp.cnt_sw_xns.all().convert_to<unsigned long long>()); 

    pr_csr.psp.cnt_pb_pbus_xns.read();
    pr_csr.psp.cnt_pb_pbus_sop.read();
    pr_csr.psp.cnt_pb_pbus_eop.read();
    pr_csr.psp.cnt_pb_pbus_no_data.read();


    if ((pr_csr.psp.cnt_pb_pbus_sop.all() & 0xff) != pr_csr.psp.cnt_pb_pbus_eop.all()) {
        PLOG_ERR("pr_csr.psp.cnt_pb_pbus_sop.all = 0x" << hex << pr_csr.psp.cnt_pb_pbus_sop.all() << ", vs eop = 0x" << pr_csr.psp.cnt_pb_pbus_eop.all() << endl)
    }
    sknobs_set_value((char*)"pr_csr/psp/cnt_pb_pbus/sop", pr_csr.psp.cnt_pb_pbus_sop.all().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pr_csr/psp/cnt_pb_pbus/xns", pr_csr.psp.cnt_pb_pbus_xns.all().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pr_csr/psp/cnt_pb_pbus/no_data", pr_csr.psp.cnt_pb_pbus_no_data.all().convert_to<unsigned long long>());
    
    no_data_hw_set   = pr_csr.psp.cnt_pb_pbus_no_data_hw_set.read_hw().convert_to<int>();
    no_data_hw_reset = pr_csr.psp.cnt_pb_pbus_no_data_hw_reset.read_hw().convert_to<int>();
    min_phv_drop     = pr_csr.psp.cnt_pb_pbus_min_phv_drop.read_hw().convert_to<int>();
    PLOG_MSG("no_data_hw_set = " << no_data_hw_set << ", no_data_hw_reset = " << no_data_hw_reset << endl)
    
    #ifdef MODULE_SIM
    if (no_data_hw_set != pr_csr.psp.cnt_pb_pbus_no_data_hw_set.value().convert_to<int>())
        PLOG_ERR("pr_csr.psp.cnt_pb_pbus_no_data_hw_set = 0x" << hex << no_data_hw_set << ", vs exp = 0x" << pr_csr.psp.cnt_pb_pbus_no_data_hw_set.value().convert_to<int>() << endl)
    if (no_data_hw_reset != pr_csr.psp.cnt_pb_pbus_no_data_hw_reset.value().convert_to<int>())
        PLOG_ERR("pr_csr.psp.cnt_pb_pbus_no_data_hw_reset = 0x" << hex << no_data_hw_reset << ", vs exp = 0x" << pr_csr.psp.cnt_pb_pbus_no_data_hw_reset.value().convert_to<int>() << endl)
    if (min_phv_drop != pr_csr.psp.cnt_pb_pbus_min_phv_drop.value().convert_to<int>())
        PLOG_ERR("pr_csr.psp.cnt_pb_pbus_min_phv_drop = 0x" << hex << min_phv_drop << ", vs exp = 0x" << pr_csr.psp.cnt_pb_pbus_min_phv_drop.value().convert_to<int>() << endl)
    #endif
    
    pr_csr.psp.cnt_ma_xns.read();
    pr_csr.psp.cnt_ma_sop.read();
    pr_csr.psp.cnt_ma_eop.read();
    pr_csr.psp.cnt_ma_recirc.read();
    pr_csr.psp.cnt_ma_no_data.read();
    pr_csr.psp.cnt_ma_collapsed.read();
    pr_csr.psp.cnt_ma_drop.read();

    if ((pr_csr.psp.cnt_ma_sop.all() & 0xff) != pr_csr.psp.cnt_ma_eop.all()) {
        PLOG_ERR("pr_csr.psp.cnt_ma_sop.all = 0x" << hex << pr_csr.psp.cnt_ma_sop.all() << ", vs eop = 0x" << pr_csr.psp.cnt_ma_eop.all() << endl)
    }
    sknobs_set_value((char*)"pr_csr/psp/cnt_ma/sop", pr_csr.psp.cnt_ma_sop.all().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pr_csr/psp/cnt_ma/xns", pr_csr.psp.cnt_ma_xns.all().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pr_csr/psp/cnt_ma/recirc", pr_csr.psp.cnt_ma_recirc.all().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pr_csr/psp/cnt_ma/no_data", pr_csr.psp.cnt_ma_no_data.all().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pr_csr/psp/cnt_ma/collapsed", pr_csr.psp.cnt_ma_collapsed.all().convert_to<unsigned long long>());

    
    pr_csr.psp.cnt_pr_pbus_xns.read();
    pr_csr.psp.cnt_pr_pbus_sop.read();
    pr_csr.psp.cnt_pr_pbus_eop.read();
    pr_csr.psp.cnt_pr_pbus_recirc.read();
    if ((pr_csr.psp.cnt_pr_pbus_sop.all() & 0xff) != pr_csr.psp.cnt_pr_pbus_eop.all()) {
        PLOG_ERR("pr_csr.psp.cnt_pr_pbus_sop.all = 0x" << hex << pr_csr.psp.cnt_pr_pbus_sop.all() << ", vs eop = 0x" << pr_csr.psp.cnt_pr_pbus_eop.all() << endl)
    }
    sknobs_set_value((char*)"pr_csr/psp/cnt_pr_pbus/sop", pr_csr.psp.cnt_pr_pbus_sop.all().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pr_csr/psp/cnt_pr_pbus/xns", pr_csr.psp.cnt_pr_pbus_xns.all().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pr_csr/psp/cnt_pr_pbus/recirc", pr_csr.psp.cnt_pr_pbus_recirc.all().convert_to<unsigned long long>());

    pr_csr.psp.cnt_pr_resub_pbus_xns.read();
    pr_csr.psp.cnt_pr_resub_pbus_sop.read();
    pr_csr.psp.cnt_pr_resub_pbus_eop.read();
    if ((pr_csr.psp.cnt_pr_resub_pbus_sop.all() & 0xff) != pr_csr.psp.cnt_pr_resub_pbus_eop.all()) {
        PLOG_ERR("pr_csr.psp.cnt_pr_resub_pbus_sop.all = 0x" << hex << pr_csr.psp.cnt_pr_resub_pbus_sop.all() << ", vs eop = 0x" << pr_csr.psp.cnt_pr_resub_pbus_eop.all() << endl)
    }
    sknobs_set_value((char*)"pr_csr/psp/cnt_pr_resub_pbus/sop", pr_csr.psp.cnt_pr_resub_pbus_sop.all().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pr_csr/psp/cnt_pr_resub_pbus/xns", pr_csr.psp.cnt_pr_resub_pbus_xns.all().convert_to<unsigned long long>());

    pr_csr.psp.cnt_pr_resub_xns.read();
    pr_csr.psp.cnt_pr_resub_sop.read();
    pr_csr.psp.cnt_pr_resub_eop.read();
    pr_csr.psp.cnt_pr_resub_no_data.read();
    if ((pr_csr.psp.cnt_pr_resub_sop.all() & 0xff) != pr_csr.psp.cnt_pr_resub_eop.all()) {
        PLOG_ERR("pr_csr.psp.cnt_pr_resub_sop.all = 0x" << hex << pr_csr.psp.cnt_pr_resub_sop.all() << ", vs eop = 0x" << pr_csr.psp.cnt_pr_resub_eop.all() << endl)
    }
    sknobs_set_value((char*)"pr_csr/psp/cnt_pr_resub/sop", pr_csr.psp.cnt_pr_resub_sop.all().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pr_csr/psp/cnt_pr_resub/xns", pr_csr.psp.cnt_pr_resub_xns.all().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pr_csr/psp/cnt_pr_resub/no_data", pr_csr.psp.cnt_pr_resub_no_data.all().convert_to<unsigned long long>());

    pr_csr.psp.sat_psp_csr_read_access_err.read();
    if (pr_csr.psp.sat_psp_csr_read_access_err.all() != 0) {
        PLOG_ERR("pr_csr.sat_psp_csr.read_access_err = 0x" << hex << pr_csr.psp.sat_psp_csr_read_access_err.all() << endl)
    }
    pr_csr.psp.sat_psp_csr_write_access_err.write();
    if (pr_csr.psp.sat_psp_csr_write_access_err.all() != 0) {
        PLOG_ERR("pr_csr.sat_psp_csr.write_access_err = 0x" << hex << pr_csr.psp.sat_psp_csr_write_access_err.all() << endl)
    }
    
    if (((pr_csr.psp.cnt_sw_sop.all() + pr_csr.psp.cnt_pb_pbus_sop.all() + pr_csr.psp.cnt_pr_resub_sop.all())&0xffffffffff) != ((pr_csr.psp.cnt_ma_sop.all() + min_phv_drop) & 0xffffffffff)) {
        PLOG_ERR("pr_csr.psp.cnt_sw_sop(" << pr_csr.psp.cnt_sw_sop.all() << ") + cnt_pb_pbus(" << pr_csr.psp.cnt_pb_pbus_sop.all() << ") + cnt_pr_resub(" << pr_csr.psp.cnt_pr_resub_sop.all() << ") vs cnt_ma(" << pr_csr.psp.cnt_ma_sop.all() << ") + min_phv_drop(" << min_phv_drop << ")" << endl)
    }
    if (pr_csr.psp.cnt_ma_recirc.all() != pr_csr.psp.cnt_pr_resub_sop.all()) {
        PLOG_ERR("pr_csr.psp.cnt_ma_recirc = " << pr_csr.psp.cnt_ma_recirc.all() << ", cnt_pr_resub = " << pr_csr.psp.cnt_pr_resub_sop.all() << endl)
    }
    //only when min_phv_drop = 0 we can check this, otherwise, don't know drop is no_data=0 or no_data=1
    if (min_phv_drop == 0) {
        if (((pr_csr.psp.cnt_sw_sop.all() + pr_csr.psp.cnt_pr_resub_no_data.all() + pr_csr.psp.cnt_pb_pbus_no_data.all() - no_data_hw_reset + no_data_hw_set - min_phv_drop)&0xffffffffff) != (pr_csr.psp.cnt_ma_no_data.all()&0xffffffffff)) {
            PLOG_ERR("pr_csr.psp.cnt_sw_sop(" << pr_csr.psp.cnt_sw_sop.all() << ") + cnt_pr_resub.no_data(" << pr_csr.psp.cnt_pr_resub_no_data.all() << ") + cnt_pb_pbus.no_data(" << pr_csr.psp.cnt_pb_pbus_no_data.all() << ") + cnt_pb_pbus.no_data_hw_set(" << no_data_hw_set << ") - cnt_pb_pbus_no_data_hw_reset(" << no_data_hw_reset << ") - cnt_pb_pbus_min_phv_drop(" << min_phv_drop << ") != cnt_ma.no_data(" << pr_csr.psp.cnt_ma_no_data.all() << ")" << endl)
        }
    }
    if (min_phv_drop == 0) {
        if (pr_csr.psp.cnt_pr_pbus_sop.all() != ((pr_csr.psp.cnt_pr_resub_pbus_sop.all() + pr_csr.psp.cnt_pb_pbus_sop.all() - pr_csr.psp.cnt_pb_pbus_no_data.all() - no_data_hw_set + no_data_hw_reset)&0xffffffffff)) {
            PLOG_ERR("pr_csr.psp.cnt_pr_pbus(" << pr_csr.psp.cnt_pr_pbus_sop.all() << ") != cnt_pr_resub_pbus(" << pr_csr.psp.cnt_pr_resub_pbus_sop.all() << ") + cnt_pb_pbus(" << pr_csr.psp.cnt_pb_pbus_sop.all() << ") - cnt_pb_pbus.no_data(" << pr_csr.psp.cnt_pb_pbus_no_data.all() << ") - cnt_pb_pbus.no_data_hw_set(" << no_data_hw_set << ") + no_data_hw_reset(" << no_data_hw_reset << ")" << endl)
        }
    }
    if (pr_csr.psp.cnt_pr_pbus_recirc.all() != pr_csr.psp.cnt_pr_resub_pbus_sop.all()) {
        PLOG_ERR("pr_csr.psp.cnt_pr_pbus_recirc.all = " << pr_csr.psp.cnt_pr_pbus_recirc.all() << " != cnt_pr_resub_pbus.sop = " << pr_csr.psp.cnt_pr_resub_pbus_sop.all() << endl)
    }


    if (!sknobs_exists((char*)"pr_csr/psp/disable_err_cnt_check")) {
        pr_csr.psp.sat_lif_table_ecc_err.read();
        if (pr_csr.psp.sat_lif_table_ecc_err.all() != 0) {
            PLOG_ERR("pr_csr.sat_lif_table_ecc_err should be 0" << endl)
            pr_csr.psp.sat_lif_table_ecc_err.show();
        }

        pr_csr.psp.sat_lif_table_qid_err.read();
        if (pr_csr.psp.sat_lif_table_qid_err.all() != 0) {
            PLOG_ERR("pr_csr.sat_lif_table_qid_err should be 0" << endl)
            pr_csr.psp.sat_lif_table_qid_err.show();
        }

        sknobs_set_value((char*)"pr_csr/psp/sat_pb_pbus/nopkt", pr_csr.psp.sat_pb_pbus_nopkt.all().convert_to<unsigned long long>());
        sknobs_set_value((char*)"pr_csr/psp/sat_pb_pbus/err", pr_csr.psp.sat_pb_pbus_err.all().convert_to<unsigned long long>());

        pr_csr.psp.sat_pb_pbus_sop_err.read();
        if (pr_csr.psp.sat_pb_pbus_sop_err.all() != 0) {
            PLOG_ERR("sat_pb_pbus_sop_err expect to be 0" << endl)
            pr_csr.psp.sat_pb_pbus_sop_err.show();
        }

        pr_csr.psp.sat_pb_pbus_eop_err.read();
        if (pr_csr.psp.sat_pb_pbus_eop_err.all() != 0) {
            PLOG_ERR("sat_pb_pbus_eop_err expect to be 0" << endl)
            pr_csr.psp.sat_pb_pbus_eop_err.show();
        }

        pr_csr.psp.sat_pb_pbus_err.read();
        if (pr_csr.psp.sat_pb_pbus_err.all() != 0) {
            PLOG_ERR("sat_pb_pbus_err expect to be 0" << endl)
            pr_csr.psp.sat_pb_pbus_err.show();
        }

        pr_csr.psp.sat_pb_pbus_dummy.read();
        if (pr_csr.psp.sat_pb_pbus_dummy.all() != 0) {
            PLOG_ERR("sat_pb_pbus_dummy expect to be 0" << endl)
            pr_csr.psp.sat_pb_pbus_dummy.show();
        }

        pr_csr.psp.sat_pb_pbus_nopkt.read();
        if (pr_csr.psp.sat_pb_pbus_nopkt.all() != 0) {
            PLOG_ERR("sat_pb_pbus_nopkt expect to be 0" << endl)
            pr_csr.psp.sat_pb_pbus_nopkt.show();
        }

        pr_csr.psp.sat_ma_sop_err.read();
        if (pr_csr.psp.sat_ma_sop_err.all() != 0) {
            PLOG_ERR("sat_ma_sop_err expect to be 0" << endl)
            pr_csr.psp.sat_ma_sop_err.show();
        }

        pr_csr.psp.sat_ma_eop_err.read();
        if (pr_csr.psp.sat_ma_eop_err.all() != 0) {
            PLOG_ERR("sat_ma_eop_err expect to be 0" << endl)
            pr_csr.psp.sat_ma_eop_err.show();
        }

        pr_csr.psp.sat_pr_pbus_sop_err.read();
        if (pr_csr.psp.sat_pr_pbus_sop_err.all() != 0) {
            PLOG_ERR("sat_pr_pbus_sop_err expect to be 0" << endl)
            pr_csr.psp.sat_pr_pbus_sop_err.show();
        }

        pr_csr.psp.sat_pr_pbus_eop_err.read();
        if (pr_csr.psp.sat_pr_pbus_eop_err.all() != 0) {
            PLOG_ERR("sat_pr_pbus_eop_err expect to be 0" << endl)
            pr_csr.psp.sat_pr_pbus_eop_err.show();
        }

        pr_csr.psp.sat_pr_pbus_err.read();
        if (pr_csr.psp.sat_pr_pbus_err.all() != 0) {
            PLOG_ERR("sat_pr_pbus_err expect to be 0" << endl)
            pr_csr.psp.sat_pr_pbus_err.show();
        }

        pr_csr.psp.sat_pr_resub_pbus_sop_err.read();
        if (pr_csr.psp.sat_pr_resub_pbus_sop_err.all() != 0) {
            PLOG_ERR("sat_pr_resub_pbus_sop_err expect to be 0" << endl)
            pr_csr.psp.sat_pr_resub_pbus_sop_err.show();
        }

        pr_csr.psp.sat_pr_resub_pbus_eop_err.read();
        if (pr_csr.psp.sat_pr_resub_pbus_eop_err.all() != 0) {
            PLOG_ERR("sat_pr_resub_pbus_eop_err expect to be 0" << endl)
            pr_csr.psp.sat_pr_resub_pbus_eop_err.show();
        }

        pr_csr.psp.sat_pr_resub_pbus_err.read();
        if (pr_csr.psp.sat_pr_resub_pbus_err.all() != 0) {
            PLOG_ERR("sat_pr_resub_pbus_err expect to be 0" << endl)
            pr_csr.psp.sat_pr_resub_pbus_err.show();
        }

        pr_csr.psp.sat_pr_resub_sop_err.read();
        if (pr_csr.psp.sat_pr_resub_sop_err.all() != 0) {
            PLOG_ERR("sat_pr_resub_sop_err expect to be 0" << endl)
            pr_csr.psp.sat_pr_resub_sop_err.show();
        }

        pr_csr.psp.sat_pr_resub_eop_err.read();
        if (pr_csr.psp.sat_pr_resub_eop_err.all() != 0) {
            PLOG_ERR("sat_pr_resub_eop_err expect to be 0" << endl)
            pr_csr.psp.sat_pr_resub_eop_err.show();
        }

    }
}

void cap_psp_eos_int_sim(int chip_id, int inst_id) {
    cap_pr_csr_t &pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, chip_id, inst_id); 
    string         csr_name;
    
    if (sknobs_exists((char*)"no_int_chk") == 0) {
        pr_csr.psp.int_sw_phv_mem.intreg.read();
        pr_csr.psp.int_sw_phv_mem.intreg.show();  
        if (pr_csr.psp.int_sw_phv_mem.intreg.ecc_uncorrectable_interrupt() != 0)
            PLOG_ERR("pr.psp.int_sw_phv_mem.ecc_uncorrectable should be 0!" << endl)
        if (sknobs_exists((char*)"PEN_IP_MEM_ECC_CORRUPT_ENABLE")) {
            if (sknobs_exists((char*)"no_ecc_chk") == 0) {
                if (pr_csr.psp.int_sw_phv_mem.intreg.ecc_correctable_interrupt() != 1)
                    PLOG_ERR("pt.npv.int_sw_phv_mem.ecc_correctable should be 1!" << endl)
            }
        }
        else {    
            if (pr_csr.psp.int_sw_phv_mem.intreg.ecc_correctable_interrupt() != 0)
                PLOG_ERR("pr.psp.int_sw_phv_mem.ecc_correctable should be 0!" << endl)
        }
        if (pr_csr.psp.int_sw_phv_mem.intreg.phv_invalid_sram_interrupt() != 0)
            PLOG_ERR("pr.psp.int_sw_phv_mem.phv_invalid_sram should be 0!" << endl)
        if (pr_csr.psp.int_sw_phv_mem.intreg.phv_invalid_data_interrupt() != 0)
            PLOG_ERR("pr_csr.psp.int_sw_phv_mem.phv_invalid_data should be 0!" << endl)
        pr_csr.psp.int_info.intreg.read();
        pr_csr.psp.int_info.intreg.show();

        //if (pr_csr.psp.cnt_pb_pbus_no_data_hw_set.all() != 0) {
        //  pr_csr.psp.int_err.intreg.pb_pbus_dummy_err_interrupt(1);
        //}
        pr_csr.psp.int_err.intreg.read_compare();
        pr_csr.psp.int_err.intreg.show();  
        

        pr_csr.psp.int_fatal.intreg.read();
        if (pr_csr.psp.int_fatal.intreg.pb_pbus_sop_err_interrupt() == 1)
            PLOG_ERR("pr_csr.psp.int_fatal.pb_pbus_sop_err should be 0" << endl)
        if (pr_csr.psp.int_fatal.intreg.pb_pbus_eop_err_interrupt() == 1)
            PLOG_ERR("pr_csr.psp.int_fatal.pb_pbus_eop_err should be 0" << endl)
        if (pr_csr.psp.int_fatal.intreg.ma_sop_err_interrupt() == 1)
            PLOG_ERR("pr_csr.psp.int_fatal.ma_sop_err should be 0" << endl)
        if (pr_csr.psp.int_fatal.intreg.ma_eop_err_interrupt() == 1)
            PLOG_ERR("pr_csr.psp.int_fatal.ma_eop_err should be 0" << endl)
        if (pr_csr.psp.int_fatal.intreg.pr_pbus_sop_err_interrupt() == 1)
            PLOG_ERR("pr_csr.psp.int_fatal.pr_pbus_sop_err should be 0" << endl)
        if (pr_csr.psp.int_fatal.intreg.pr_pbus_eop_err_interrupt() == 1)
            PLOG_ERR("pr_csr.psp.int_fatal.pr_pbus_eop_err should be 0" << endl)
        if (pr_csr.psp.int_fatal.intreg.pr_resub_sop_err_interrupt() == 1)
            PLOG_ERR("pr_csr.psp.int_fatal.pr_resub_sop_err should be 0" << endl)
        if (pr_csr.psp.int_fatal.intreg.pr_resub_eop_err_interrupt() == 1)
            PLOG_ERR("pr_csr.psp.int_fatal.pr_resub_eop_err should be 0" << endl)
        if (pr_csr.psp.int_fatal.intreg.lif_ind_table_rsp_err_interrupt() == 1)
            PLOG_ERR("pr_csr.psp.int_fatal.lif_ind_table_rsp_err should be 0" << endl)
        if (pr_csr.psp.int_fatal.intreg.flit_cnt_oflow_err_interrupt() == 1)
            PLOG_ERR("pr_csr.psp.int_fatal.flit_cnt_oflow_err should be 0" << endl)
        if (pr_csr.psp.int_fatal.intreg.lb_phv_sop_err_interrupt() == 1)
            PLOG_ERR("pr_csr.psp.int_fatal.lb_phv_sop_err should be 0" << endl)
        if (pr_csr.psp.int_fatal.intreg.dc_phv_sop_err_interrupt() == 1)
            PLOG_ERR("pr_csr.psp.int_fatal.dc_phv_sop_err should be 0" << endl)
        if (pr_csr.psp.int_fatal.intreg.lb_pkt_sop_err_interrupt() == 1)
            PLOG_ERR("pr_csr.psp.int_fatal.lb_pkt_sop_err should be 0" << endl)
        if (pr_csr.psp.int_fatal.intreg.dc_pkt_sop_err_interrupt() == 1)
            PLOG_ERR("pr_csr.psp.int_fatal.dc_pkt_sop_err should be 0" << endl)
        if (pr_csr.psp.int_fatal.intreg.sw_phv_jabber_err_interrupt() == 1)
            PLOG_ERR("pr_csr.psp.int_fatal.sw_phv_jabber_err should be 0" << endl)

        pr_csr.psp.int_fatal.intreg.show();  

        pr_csr.psp.int_lif_qstate_map.intreg.read(); 
        pr_csr.psp.int_lif_qstate_map.intreg.show(); 
        if (sknobs_exists((char*)"PEN_IP_MEM_ECC_CORRUPT_ENABLE")) {
            if (sknobs_exists((char*)"no_ecc_chk") == 0) {
                if (pr_csr.psp.int_lif_qstate_map.intreg.ecc_correctable_interrupt() != 1)
                    PLOG_ERR("pr.psp.int_lif_qstate_map.ecc_correctable should be 1!" << endl)
            }
        }
        else {
            if (pr_csr.psp.int_lif_qstate_map.intreg.ecc_correctable_interrupt() != 0)
                PLOG_ERR("pr.psp.int_lif_qstate_map.ecc_correctable should be 0!" << endl)
        }
        if (pr_csr.psp.int_lif_qstate_map.intreg.ecc_uncorrectable_interrupt() != 0)
            PLOG_ERR("pr.psp.int_lif_qstate_map.ecc_uncorrectable should be 0!" << endl)
        if (pr_csr.psp.int_lif_qstate_map.intreg.qid_invalid_interrupt() != 0)
            PLOG_ERR("pr.psp.int_lif_qstate_map.qid_invalid should be 0!" << endl)
    }
}

void cap_psp_eos_int(int chip_id, int inst_id) {
    cap_pr_csr_t &pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, chip_id, inst_id);

    pr_csr.psp.int_err.intreg.read_compare();
    pr_csr.psp.int_fatal.intreg.read_compare();
}

void cap_psp_eos_sta(int chip_id, int inst_id) {  
    cap_pr_csr_t &pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, chip_id, inst_id);

    pr_csr.psp.sta.read();
    if (pr_csr.psp.sta.empty() != 1)
        PLOG_ERR("pr_csr.psp.sta.empty should be 1!" << endl)
    if (pr_csr.psp.sta.pkt_empty() != 1)
        PLOG_ERR("pr_csr.psp.sta.pkt_empty should be 1!" << endl)
    if (pr_csr.psp.sta.phv_empty() != 1)
        PLOG_ERR("pr_csr.psp.sta.phv_empty should be 1!" << endl)
    if (pr_csr.psp.sta.pkt_inflight() != 0)
        PLOG_ERR("pr_csr.psp.sta.pkt_inflight should be 0 vs " << pr_csr.psp.sta.pkt_inflight() << "!" << endl)
    if (pr_csr.psp.sta.phv_inflight() != 0)
        PLOG_ERR("pr_csr.psp.sta.phv_inflight should be 0! vs " << pr_csr.psp.sta.phv_inflight() << "!" << endl)

    pr_csr.psp.sta_fifo.read();
    if (pr_csr.psp.sta_fifo.bus_pkt_empty() != 1)
        PLOG_ERR("pr_csr.psp.sta_fifo.bus_pkt_empty should be 1!" << endl)
    if (pr_csr.psp.sta_fifo.sv10_ff_pkt_srdy() != 0)
        PLOG_ERR("pr_csr.psp.sta_fifo.sv10_ff_pkt_srdy should be 0!" << endl)
    if (pr_csr.psp.sta_fifo.sv10_ff_rsp_srdy() != 0)
        PLOG_ERR("pr_csr.psp.sta_fifo.sv10_ff_rsp_srdy should be 0!" << endl)
    if (pr_csr.psp.sta_fifo.sv12_dc_phv_srdy() != 0)
        PLOG_ERR("pr_csr.psp.sta_fifo.sv12_dc_phv_srdy should be 0!" << endl)
    if (pr_csr.psp.sta_fifo.sv12_dc_pkt_srdy() != 0)
        PLOG_ERR("pr_csr.psp.sta_fifo.sv12_dc_pkt_srdy should be 0!" << endl)
    if (pr_csr.psp.sta_fifo.lpbk_phv_ff_empty() != 1)
        PLOG_ERR("pr_csr.psp.sta_fifo.lpbk_phv_ff_empty should be 1!" << endl)
    if (pr_csr.psp.sta_fifo.sv13_lb_pkt_srdy() != 0)
        PLOG_ERR("pr_csr.psp.sta_fifo.sv13_lb_pkt_srdy should be 0!" << endl)
    if (pr_csr.psp.sta_fifo.out_phv_empty() != 1)
        PLOG_ERR("pr_csr.psp.sta_fifo.out_phv_empty should be 1!" << endl)
    if (pr_csr.psp.sta_fifo.sv21_ma_phv_srdy() != 0)
        PLOG_ERR("pr_csr.psp.sta_fifo.sv21_ma_phv_srdy should be 0!" << endl)
    if (pr_csr.psp.sta_fifo.pkt_order_pop_srdy() != 0)
        PLOG_ERR("pr_csr.psp.sta_fifo.pkt_order_pop_srdy should be 0!" << endl)
    if (pr_csr.psp.sta_fifo.sv21_pr_pkt_srdy() != 0)
        PLOG_ERR("pr_csr.psp.sta_fifo.sv21_pr_pkt_srdy should be 0!" << endl)

    pr_csr.psp.sta_flow.read();
    if (pr_csr.psp.sta_flow.ma_drdy() != 1)
        PLOG_ERR("pr_csr.psp.sta_flow.ma_drdy should be 1!" << endl)
    if (pr_csr.psp.sta_flow.pb_pbus_drdy() != 1)
        PLOG_ERR("pr_csr.psp.sta_flow.pb_pbus_drdy should be 1!" << endl)
    if (pr_csr.psp.sta_flow.pkt_order_push_drdy() != 1)
        PLOG_ERR("pr_csr.psp.sta_flow.pkt_order_push_drdy should be 1!" << endl)
    if (pr_csr.psp.sta_flow.pr_resub_drdy() != 1)
        PLOG_ERR("pr_csr.psp.sta_flow.pr_resub_drdy should be 1!" << endl)
    if (pr_csr.psp.sta_flow.pr_resub_pbus_drdy() != 1)
        PLOG_ERR("pr_csr.psp.sta_flow.pr_resub_pbus_drdy should be 1!" << endl)
    if (pr_csr.psp.sta_flow.sv01_pb_pkt_drdy() != 1)
        PLOG_ERR("pr_csr.psp.sv01_pb_pkt_drdy should be 1!" << endl)
    if (pr_csr.psp.sta_flow.sv01_lb_phv_drdy() != 1)
        PLOG_ERR("pr_csr.psp.sta_flow.sv01_lb_phv_drdy should be 1!" << endl)
    if (pr_csr.psp.sta_flow.sv02_lb_pkt_drdy() != 1)
        PLOG_ERR("pr_csr.psp.sta_flow.sv02_lb_pkt_drdy should be 1!" << endl)
    if (pr_csr.psp.sta_flow.sv04_lt_ovr_drdy() != 1)
        PLOG_ERR("pr_csr.psp.sta_flow.sv04_lt_ovr_drdy should be 1!" << endl)
    if (pr_csr.psp.sta_flow.sv04_lt_rsp_drdy() != 1)
        PLOG_ERR("pr_csr.psp.sta_flow.sv04_lt_rsp_drdy should be 1!" << endl)
    if (pr_csr.psp.sta_flow.sv11_in_phv_drdy() != 1)
        PLOG_ERR("pr_csr.psp.sta_flow.sv11_in_phv_drdy should be 1!" << endl)
    if (pr_csr.psp.sta_flow.sv11_in_pkt_drdy() != 1)
        PLOG_ERR("pr_csr.psp.sta_flow.sv11_in_pkt_drdy should be 1!" << endl)
    if (pr_csr.psp.sta_flow.sv14_ma_phv_drdy() != 1)
        PLOG_ERR("pr_csr.psp.sta_flow.sv14_ma_phv_drdy should be 1!" << endl)
    if (pr_csr.psp.sta_flow.sv15_pr_pkt_drdy() != 1)
        PLOG_ERR("pr_csr.psp.sta_flow.sv15_pr_pkt_drdy should be 1!" << endl)
    if (pr_csr.psp.sta_flow.sv20_ma_phv_drdy() != 1)
        PLOG_ERR("pr_csr.psp.sta_flow.sv20_ma_phv_drdy should be 1!" << endl)
    if (pr_csr.psp.sta_flow.sv21_ma_phv_drdy() != 1)
        PLOG_ERR("pr_csr.psp.sta_flow.sv21_ma_phv_drdy should be 1!" << endl)
    if (pr_csr.psp.sta_flow.pr_pkt_ff_almost_full() != 0)
        PLOG_ERR("pr_csr.psp.sta_flow.pr_pkt_ff_almost_full should be 0!" << endl)
    if (pr_csr.psp.sta_flow.ptd_npv_phv_full() != 0)
        PLOG_ERR("pr_csr.psp.sta_flow.ptd_npv_phv_full should be 0!" << endl)
    
    pr_csr.psp.sta_fsm.read();
    if (pr_csr.psp.sta_fsm.split_state() != 0)
        PLOG_ERR("pr_csr.psp.sta_fsm.split_state should be 0!" << endl)
    if (pr_csr.psp.sta_fsm.phv_state() != 0)
        PLOG_ERR("pr_csr.psp.sta_fsm.phv_state should be 0!" << endl)
    if (pr_csr.psp.sta_fsm.pkt_state() != 0)
        PLOG_ERR("pr_csr.pps.sta_fsm.pkt_state should be 0!" << endl)
    if (pr_csr.psp.sta_fsm.sv01_pb_pkt_in_pkt() != 0)
        PLOG_ERR("pr_csr.psp.sta_fsm.sv01_pb_pkt_in_pkt should be 0!" << endl)
    if (pr_csr.psp.sta_fsm.sv03_lt_req_in_progress() != 0)
        PLOG_ERR("pr_csr.psp.sta_fsm.sv03_lt_req_in_progress should be 0!" << endl)
    if (pr_csr.psp.sta_fsm.sv03_sw_req_in_pkt() != 0)
        PLOG_ERR("pr_csr.psp.sta_fsm.sv03_sw_req_in_pkt should be 0!" << endl)
    if (pr_csr.psp.sta_fsm.sv13_lb_phv_in_pkt() != 0)
        PLOG_ERR("pr_csr.psp.sta_fsm.sv13_lb_phv_in_pkt should be 0!" << endl)
    if (pr_csr.psp.sta_fsm.sv13_lb_pkt_in_pkt() != 0)
        PLOG_ERR("pr_csr.psp.sta_fsm.sv13_lb_pkt_in_pkt should be 0!" << endl)
    if (pr_csr.psp.sta_fsm.sv15_pr_pkt_in_pkt() != 0)
        PLOG_ERR("pr_csr.psp.sta_fsm.sv15_pr_pkt_in_pkt should be 0!" << endl)
    if (pr_csr.psp.sta_fsm.sv20_ma_phv_in_pkt() != 0)
        PLOG_ERR("pr_csr.psp.sta_fsm.sv20_ma_phv_in_pkt should be 0!" << endl)
}

/////self defined
void cap_psp_config(int chip_id, int inst_id) {
    cap_pr_csr_t &pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, chip_id, inst_id);

    PLOG_MSG("psp_config...." << endl)

    pr_csr.psp.cfg_qstate_map_rsp.enable(1);
    pr_csr.psp.cfg_qstate_map_rsp.qid_enable(0);
    pr_csr.psp.cfg_qstate_map_rsp.qtype_enable(0);
    pr_csr.psp.cfg_qstate_map_rsp.addr_enable(1);
    //pr_csr.psp.cfg_qstate_map_rsp.no_data_enable(1);
    pr_csr.psp.cfg_qstate_map_rsp.write();
}

void cap_psp_lif_qstate_map(int lif, cap_lif_qstate_data_bundle_t lif_qstate_entry) {
    int chip_id = 0;
    int inst_id = 0;

    cap_pr_csr_t &pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, chip_id, inst_id);
    
    //pr_csr.psp.cfg_qstate_map_rsp.all(15);
    //pr_csr.psp.cfg_qstate_map_rsp.write();

    pr_csr.psp.dhs_lif_qstate_map.entry[lif].all(lif_qstate_entry.all());
    pr_csr.psp.dhs_lif_qstate_map.entry[lif].write();
}

void cap_psp_sw_phv_mem(int index, cap_psp_csr_dhs_sw_phv_mem_entry_t  sw_phv_mem_entry) {
    int chip_id = 0;
    int inst_id = 0;

    cap_pr_csr_t &pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, chip_id, inst_id);

    pr_csr.psp.dhs_sw_phv_mem.entry[index].all(sw_phv_mem_entry.all());
    pr_csr.psp.dhs_sw_phv_mem.entry[index].write();
}

void cap_psp_cfg_sw_phv_control(int profile_id, int data) {
    int chip_id = 0;
    int inst_id = 0;

    cap_pr_csr_t &pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, chip_id, inst_id);

    pr_csr.psp.cfg_sw_phv_control[profile_id].all(data);
    pr_csr.psp.cfg_sw_phv_control[profile_id].write();
    pr_csr.psp.cfg_sw_phv_control[profile_id].show();
}

void cap_psp_cfg_sw_phv_config(int profile_id, cap_psp_csr_cfg_sw_phv_config_t sw_phv_config) {
    int chip_id = 0;
    int inst_id = 0;

    cap_pr_csr_t &pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, chip_id, inst_id);

    pr_csr.psp.cfg_sw_phv_config[profile_id].all(sw_phv_config.all());
    pr_csr.psp.cfg_sw_phv_config[profile_id].write();
    pr_csr.psp.cfg_sw_phv_config[profile_id].show();
}

void cap_psp_sw_phv_start_enable(int profile_id, int enable) {
    int chip_id = 0;
    int inst_id = 0;

    cap_pr_csr_t &pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, chip_id, inst_id);

    pr_csr.psp.cfg_sw_phv_control[profile_id].read();
    pr_csr.psp.cfg_sw_phv_control[profile_id].start_enable(enable);
    pr_csr.psp.cfg_sw_phv_control[profile_id].write();
    pr_csr.psp.cfg_sw_phv_control[profile_id].read();
}

void cap_psp_sw_phv_status(int profile_id, uint64_t &data) {
    int chip_id = 0;
    int inst_id = 0;

    cap_pr_csr_t &pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, chip_id, inst_id);

    pr_csr.psp.sta_sw_phv_state[profile_id].read();
    data = pr_csr.psp.sta_sw_phv_state[profile_id].all().convert_to<uint64_t>();
    PLOG_MSG("profile_id = " << profile_id << endl)
    pr_csr.psp.sta_sw_phv_state[profile_id].show();
}

void cap_psp_sw_phv_display() {
    int chip_id = 0;
    int inst_id = 0;

    cap_pr_csr_t &pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, chip_id, inst_id);

    PLOG_MSG("############################[SW PHV CFG]#########################################" << endl)
    PLOG_MSG(setw(2) << "prof_id" << setw(10) << "#sta_en" << setw(10) << "#cnt_rpt_en" << setw(10) << "#ltime_en" << setw(10) << "#frm_size_en" << setw(8) << "#sta_addr" << setw(8) << "#num_flits" << setw(8) << "#insert_prd" << setw(10) << "#cnt_max" << setw(10) << endl)
    for (int i = 0; i < 8; i++) {
        PLOG_MSG(setw(2) << i << setw(10) << pr_csr.psp.cfg_sw_phv_control[i].start_enable() << setw(10) << pr_csr.psp.cfg_sw_phv_control[i].counter_repeat_enable() << setw(10) << pr_csr.psp.cfg_sw_phv_control[i].localtime_enable() << setw(10) << pr_csr.psp.cfg_sw_phv_control[i].frame_size_enable() << setw(16) << pr_csr.psp.cfg_sw_phv_config[i].start_addr() << setw(10) << pr_csr.psp.cfg_sw_phv_config[i].num_flits() << setw(10) << pr_csr.psp.cfg_sw_phv_config[i].insertion_period_clocks() << setw(10) <<  pr_csr.psp.cfg_sw_phv_config[i].counter_max() << endl)
    }
    PLOG_MSG("############################[END OF SW PHV CFG]##################################" << endl)
}

void cap_psp_sw_prof_cfg(int profile_id, cpp_int *sw_phv_data, int num_flits, int cnt_rep_en, int localtime_en, int frm_sz_en, uint64_t insert_period, int counter_max) {
    int chip_id = 0;
    int inst_id = 0;
    int sta_addr;
    cpp_int flit_data;

    cap_pr_csr_t &pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, chip_id, inst_id);

    pr_csr.psp.cfg_sw_phv_control[profile_id].start_enable(0);
    pr_csr.psp.cfg_sw_phv_control[profile_id].counter_repeat_enable(cnt_rep_en);
    pr_csr.psp.cfg_sw_phv_control[profile_id].qid_repeat_enable(0);
    pr_csr.psp.cfg_sw_phv_control[profile_id].localtime_enable(localtime_en);
    pr_csr.psp.cfg_sw_phv_control[profile_id].frame_size_enable(frm_sz_en);
    pr_csr.psp.cfg_sw_phv_control[profile_id].packet_len_enable(0);
    pr_csr.psp.cfg_sw_phv_control[profile_id].qid_enable(0);
    pr_csr.psp.cfg_sw_phv_control[profile_id].write();
    pr_csr.psp.cfg_sw_phv_control[profile_id].show();

    if (profile_id == 0)
        sta_addr = 0; 
    else
        sta_addr = pr_csr.psp.cfg_sw_phv_config[profile_id-1].start_addr().convert_to<int>() + pr_csr.psp.cfg_sw_phv_config[profile_id-1].num_flits().convert_to<int>() + 1; 

    pr_csr.psp.cfg_sw_phv_config[profile_id].start_addr(sta_addr);
    pr_csr.psp.cfg_sw_phv_config[profile_id].num_flits(num_flits - 1);
    pr_csr.psp.cfg_sw_phv_config[profile_id].insertion_period_clocks(insert_period);
    pr_csr.psp.cfg_sw_phv_config[profile_id].counter_max(counter_max);
    pr_csr.psp.cfg_sw_phv_config[profile_id].qid_min(0);
    pr_csr.psp.cfg_sw_phv_config[profile_id].qid_max(0);
    pr_csr.psp.cfg_sw_phv_config[profile_id].write();
    pr_csr.psp.cfg_sw_phv_config[profile_id].show();

    for (int i = 0; i < num_flits; i++) {
        flit_data = cpp_int_helper::s_get_slc(*sw_phv_data, 512, i*512);
        pr_csr.psp.dhs_sw_phv_mem.entry[sta_addr + i].all(flit_data);
        pr_csr.psp.dhs_sw_phv_mem.entry[sta_addr + i].write();
        pr_csr.psp.dhs_sw_phv_mem.entry[sta_addr + i].show();
    }
}
void cap_psp_dump_cntr() {
    cap_pr_csr_t &pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, 0, 0);
    
    pr_csr.psp.cnt_pb_pbus_sop.read();
    pr_csr.psp.cnt_pb_pbus_no_data.read();
    pr_csr.psp.cnt_sw_sop.read();
    pr_csr.psp.cnt_ma_recirc.read();
    pr_csr.psp.cnt_ma_sop.read();
    pr_csr.psp.cnt_ma_no_data.read();
    pr_csr.psp.cnt_ma_drop.read();
    pr_csr.psp.cnt_pr_pbus_sop.read();
    pr_csr.psp.cnt_pr_pbus_recirc.read();
    pr_csr.psp.cnt_pr_resub_pbus_sop.read();
    pr_csr.psp.cnt_pr_resub_sop.read();
    pr_csr.psp.cnt_pr_resub_no_data.read();
    pr_csr.psp.cnt_psp_prd_phv_valid.read();

    PSP_PRN_MSG(setw(1) << "========================================================= RXDMA PSP CNTRS ==============================================================\n");
    PSP_PRN_MSG(setw(32) << "cnt_pb_pbus_sop" << setw(10) << pr_csr.psp.cnt_pb_pbus_sop.all().convert_to<unsigned int>() << "\n");
    PSP_PRN_MSG(setw(32) << "cnt_pb_pbus_no_data" << setw(10) << pr_csr.psp.cnt_pb_pbus_no_data.all().convert_to<unsigned int>() << "\n");
    PSP_PRN_MSG(setw(32) << "cnt_sw_sop" << setw(10) << pr_csr.psp.cnt_sw_sop.all().convert_to<unsigned int>() << "\n");
    PSP_PRN_MSG(setw(32) << "cnt_ma_recirc" << setw(10) << pr_csr.psp.cnt_ma_recirc.all().convert_to<unsigned int>() << "\n");
    PSP_PRN_MSG(setw(32) << "cnt_ma_sop" << setw(10) << pr_csr.psp.cnt_ma_sop.all().convert_to<unsigned int>() << "\n");
    PSP_PRN_MSG(setw(32) << "cnt_ma_no_data" << setw(10) << pr_csr.psp.cnt_ma_no_data.all().convert_to<unsigned int>() << "\n");
    PSP_PRN_MSG(setw(32) << "cnt_ma_drop" << setw(10) << pr_csr.psp.cnt_ma_drop.all().convert_to<unsigned int>() << "\n");
    PSP_PRN_MSG(setw(32) << "cnt_pr_pbus_sop" << setw(10) << pr_csr.psp.cnt_pr_pbus_sop.all().convert_to<unsigned int>() << "\n");
    PSP_PRN_MSG(setw(32) << "cnt_pr_pbus_recirc" << setw(10) << pr_csr.psp.cnt_pr_pbus_recirc.all().convert_to<unsigned int>() << "\n");
    PSP_PRN_MSG(setw(32) << "cnt_pr_resub_pbus_sop" << setw(10) << pr_csr.psp.cnt_pr_resub_pbus_sop.all().convert_to<unsigned int>() << "\n");
    PSP_PRN_MSG(setw(32) << "cnt_pr_resub_sop" << setw(10) << pr_csr.psp.cnt_pr_resub_sop.all().convert_to<unsigned int>() << "\n");
    PSP_PRN_MSG(setw(32) << "cnt_pr_resub_no_data" << setw(10) << pr_csr.psp.cnt_pr_resub_no_data.all().convert_to<unsigned int>() << "\n");
    PSP_PRN_MSG(setw(32) << "cnt_psp_prd_phv_valid" << setw(10) << pr_csr.psp.cnt_psp_prd_phv_valid.all().convert_to<unsigned int>() << "\n");

}

void cap_psp_clr_cntr() {
    cap_pr_csr_t &pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, 0, 0);

    pr_csr.psp.cnt_pb_pbus_sop.all(0);
    pr_csr.psp.cnt_pb_pbus_no_data.all(0);
    pr_csr.psp.cnt_sw_sop.all(0);
    pr_csr.psp.cnt_ma_recirc.all(0);
    pr_csr.psp.cnt_ma_sop.all(0);
    pr_csr.psp.cnt_ma_no_data.all(0);
    pr_csr.psp.cnt_ma_drop.all(0);
    pr_csr.psp.cnt_pr_pbus_sop.all(0);
    pr_csr.psp.cnt_pr_pbus_recirc.all(0);
    pr_csr.psp.cnt_pr_resub_pbus_sop.all(0);
    pr_csr.psp.cnt_pr_resub_sop.all(0);
    pr_csr.psp.cnt_pr_resub_no_data.all(0);
    pr_csr.psp.cnt_psp_prd_phv_valid.all(0);

    pr_csr.psp.cnt_pb_pbus_sop.write();
    pr_csr.psp.cnt_pb_pbus_no_data.write();
    pr_csr.psp.cnt_sw_sop.write();
    pr_csr.psp.cnt_ma_recirc.write();
    pr_csr.psp.cnt_ma_sop.write();
    pr_csr.psp.cnt_ma_no_data.write();
    pr_csr.psp.cnt_ma_drop.write();
    pr_csr.psp.cnt_pr_pbus_sop.write();
    pr_csr.psp.cnt_pr_pbus_recirc.write();
    pr_csr.psp.cnt_pr_resub_pbus_sop.write();
    pr_csr.psp.cnt_pr_resub_sop.write();
    pr_csr.psp.cnt_pr_resub_no_data.write();
    pr_csr.psp.cnt_psp_prd_phv_valid.write();

}

void psp_print_msg(string msg) {
#ifdef _CSV_INCLUDED_
   vpi_printf((PLI_BYTE8*) "%s", msg.c_str());
#else
   std::cout << msg;
#endif
}

