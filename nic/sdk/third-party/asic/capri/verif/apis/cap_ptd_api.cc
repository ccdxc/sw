#include "cap_ptd_api.h"

// soft reset sequence 
void cap_ptd_soft_reset(int chip_id, int inst_id) {
}

// soft reset, value = 1 means apply reset, value = 0 release soft reset
void cap_ptd_set_soft_reset(int chip_id, int inst_id, int value) {
}

// init start, no polling yet
void cap_ptd_init_start(int chip_id, int inst_id) {
    cap_pt_csr_t & pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, chip_id, inst_id);

    pt_csr.ptd.axi_attr.set_access_secure(1);
    if (sknobs_exists((char*)"for_dol_test") == 1) {
        pt_csr.ptd.cfg_rdata_mem.ecc_disable_det(1);
        pt_csr.ptd.cfg_rdata_mem.ecc_disable_cor(1);
        pt_csr.ptd.cfg_rdata_mem.write();
    }
}

// poll for init done
void cap_ptd_init_done(int chip_id, int inst_id) {
}


// use sknobs base load cfg 
void cap_ptd_load_from_cfg(int chip_id, int inst_id) {
    cap_pt_csr_t & pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, chip_id, inst_id);
    cpp_int        temp;
    string         tmp_str;
    stringstream   tmp_ss;
    string         hier_path_str = pt_csr.get_hier_path();
    
    PLOG_MSG("ptd load from config..." << endl)

    std::replace( hier_path_str.begin(), hier_path_str.end(), '.', '/');
    std::replace( hier_path_str.begin(), hier_path_str.end(), '[', '/');
    std::replace( hier_path_str.begin(), hier_path_str.end(), ']', '/');
    
    pt_csr.load_from_cfg();

    pt_csr.ptd.base.read();
}

// eos check, this functions should call eos_cnt, eos_int, eos_sta 
void cap_ptd_eos(int chip_id, int inst_id) {
    cap_ptd_eos_cnt(chip_id, inst_id);
    cap_ptd_eos_int_sim(chip_id, inst_id);
    cap_ptd_eos_sta(chip_id, inst_id);
}

void cap_ptd_eos_cnt(int chip_id, int inst_id) {
    cap_pt_csr_t & pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, chip_id, inst_id);

    pt_csr.ptd.CNT_ma.read();
    if ((pt_csr.ptd.CNT_ma.sop() & 0xffffffff) != pt_csr.ptd.CNT_ma.eop()) 
        PLOG_ERR("ptd.CNT_ma.sop = 0x" << hex << pt_csr.ptd.CNT_ma.sop() << " vs eop = 0x" << hex << pt_csr.ptd.CNT_ma.eop() << endl)
    if (pt_csr.ptd.CNT_ma.err() != 0)
        PLOG_ERR("ptd.CNT_ma.err = 0x" << hex << pt_csr.ptd.CNT_ma.err() << endl)
    if (pt_csr.ptd.CNT_ma.sop_err() != 0)
        PLOG_ERR("ptd.CNT_ma.sop_err = 0x" << hex << pt_csr.ptd.CNT_ma.sop_err() << endl)
    if (pt_csr.ptd.CNT_ma.eop_err() != 0)
        PLOG_ERR("ptd.CNT_ma.eop_err = 0x" << hex << pt_csr.ptd.CNT_ma.eop_err() << endl)
    sknobs_set_value((char*)"pt_csr/ptd/CNT_ma/sop", pt_csr.ptd.CNT_ma.sop().convert_to<unsigned long long>());


    pt_csr.ptd.CNT_npv_resub.read();
    if ((pt_csr.ptd.CNT_npv_resub.sop() & 0xffffffff) != pt_csr.ptd.CNT_npv_resub.eop())
        PLOG_ERR("ptd.CNT_npv_resub.sop = 0x" << hex << pt_csr.ptd.CNT_npv_resub.sop() << " vs eop = 0x" << hex << pt_csr.ptd.CNT_npv_resub.eop() << endl)
    if (pt_csr.ptd.CNT_npv_resub.err() != 0)
        PLOG_ERR("ptd.CNT_npv_resub.err = 0x" << hex << pt_csr.ptd.CNT_npv_resub.err() << endl)
    if (pt_csr.ptd.CNT_npv_resub.sop_err() != 0)
        PLOG_ERR("ptd.CNT_npv_resub.sop_err = 0x" << hex << pt_csr.ptd.CNT_npv_resub.sop_err() << endl)
    if (pt_csr.ptd.CNT_npv_resub.eop_err() != 0)
        PLOG_ERR("ptd.CNT_npv_resub.eop_err = 0x" << hex << pt_csr.ptd.CNT_npv_resub.eop_err() << endl)
    sknobs_set_value((char*)"pt_csr/ptd/CNT_npv_resub/sop", pt_csr.ptd.CNT_npv_resub.sop().convert_to<unsigned long long>());

    pt_csr.ptd.CNT_pb.read();
    if ((pt_csr.ptd.CNT_pb.sop() & 0xffffffff) != pt_csr.ptd.CNT_pb.eop())
        PLOG_ERR("ptd.CNT_pb.sop = 0x" << hex << pt_csr.ptd.CNT_pb.sop() << " vs eop = 0x" << hex << pt_csr.ptd.CNT_pb.eop() << endl)
    if (pt_csr.ptd.CNT_pb.sop_err() != 0)
        PLOG_ERR("ptd.CNT_pb.sop_err = 0x" << hex << pt_csr.ptd.CNT_pb.sop_err() << endl)
    if (pt_csr.ptd.CNT_pb.eop_err() != 0)
        PLOG_ERR("ptd.CNT_pb.eop_err = 0x" << hex << pt_csr.ptd.CNT_pb.eop_err() << endl)
    sknobs_set_value((char*)"pt_csr/ptd/CNT_pb/sop", pt_csr.ptd.CNT_pb.sop().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pt_csr/ptd/CNT_pb/err", pt_csr.ptd.CNT_pb.err().convert_to<unsigned long long>());

    pt_csr.ptd.CNT_phv.read();
    sknobs_set_value((char*)"pt_csr/ptd/CNT_phv/no_data", pt_csr.ptd.CNT_phv.no_data().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pt_csr/ptd/CNT_phv/drop", pt_csr.ptd.CNT_phv.drop().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pt_csr/ptd/CNT_phv/recirc", pt_csr.ptd.CNT_phv.recirc().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pt_csr/ptd/CNT_phv/err", pt_csr.ptd.CNT_phv.err().convert_to<unsigned long long>());
    
    pt_csr.ptd.CNT_err.read();
    sknobs_set_value((char*)"pt_csr/ptd/CNT_err/wr_lat_drop", pt_csr.ptd.CNT_err.wr_lat_drop().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pt_csr/ptd/CNT_err/wr_wdata_drop", pt_csr.ptd.CNT_err.wr_wdata_drop().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pt_csr/ptd/CNT_err/pkt_force_eop", pt_csr.ptd.CNT_err.pkt_force_eop().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pt_csr/ptd/CNT_err/pkt_no_eop_seen", pt_csr.ptd.CNT_err.pkt_no_eop_seen().convert_to<unsigned long long>());

    pt_csr.ptd.sta_xoff.read();
    if (pt_csr.ptd.sta_xoff.numphv_counter() != 0)
        PLOG_ERR("ptd.sta_xoff.numphv_counter = 0x" << hex << pt_csr.ptd.sta_xoff.numphv_counter() << endl)
    if (pt_csr.ptd.sta_xoff.numphv_xoff() == 1)
        PLOG_ERR("ptd.sta_xoff.numphv_xoff = 1" << endl)

    
}

void cap_ptd_eos_int_sim(int chip_id, int inst_id) {
    cap_pt_csr_t & pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, chip_id, inst_id);
    string         csr_name;
    
    if (sknobs_exists((char*)"no_int_chk") == 0) { 
        if (sknobs_exists((char*)"PEN_IP_MEM_ECC_CORRUPT_ENABLE")) {
            pt_csr.ptd.int_ecc.intreg.read();
            pt_csr.ptd.int_ecc.intreg.show();
            if (sknobs_exists((char*)"no_ecc_chk") == 0) {
                if (pt_csr.ptd.int_ecc.intreg.rdata_mem_correctable_interrupt() != 1)
                    PLOG_ERR("pt_csr.ptd.int_ecc.rdata_mem_correctable should be 1!" << endl)
                if (pt_csr.ptd.int_ecc.intreg.lat_mem_correctable_interrupt() != 1)
                    PLOG_ERR("pt_csr.ptd.int_ecc.lat_mem_correctable should be 1!" << endl)
                if (pt_csr.ptd.int_ecc.intreg.wdata_mem_correctable_interrupt() != 1)
                    PLOG_ERR("pt_csr.ptd.int_ecc.wdata_mem_correctable should be 1!" << endl)
            }
            if (pt_csr.ptd.int_ecc.intreg.rdata_mem_uncorrectable_interrupt() != 0)
                PLOG_ERR("pt_csr.ptd.int_ecc.rdata_mem_uncorrectable should be 0!" << endl)
            if (pt_csr.ptd.int_ecc.intreg.lat_mem_uncorrectable_interrupt() != 0)
                PLOG_ERR("pt_csr.ptd.int_ecc.lat_mem_uncorrectable should be 0!" << endl)
            if (pt_csr.ptd.int_ecc.intreg.wdata_mem_uncorrectable_interrupt() != 0)
                PLOG_ERR("pt_csr.ptd.int_ecc.wdata_mem_uncorrectable should be 0!" << endl)
        }
        else {
            pt_csr.ptd.int_ecc.intreg.read_compare();
            pt_csr.ptd.int_ecc.intreg.show();
        }
        pt_csr.ptd.int_fifo.intreg.read_compare();
        pt_csr.ptd.int_fifo.intreg.show();

        if (pt_csr.ptd.CNT_pb.err() > 0)
            pt_csr.ptd.int_intf.intreg.pb_err_interrupt(1);
        pt_csr.ptd.int_intf.intreg.read_compare();
        pt_csr.ptd.int_intf.intreg.show();
    }
    else {
        pt_csr.ptd.int_ecc.intreg.read();
        pt_csr.ptd.int_ecc.intreg.show();
        sknobs_set_value((char*)"pt_csr/ptd/int_ecc",  pt_csr.ptd.int_ecc.intreg.all().convert_to<unsigned long long>());
    }

    pt_csr.ptd.int_grp2.intreg.read();
    pt_csr.ptd.int_grp2.intreg.show();
    if (pt_csr.ptd.int_grp2.intreg.wr_invalid_cmd_interrupt() == 1) {
        PLOG_ERR("ptd.wr_invalid_cmd asserted!");
    }
    if (pt_csr.ptd.int_grp2.intreg.wr_pend_sz_interrupt() == 1) {
        PLOG_ERR("ptd.wr_pend_sz asserted!");
    }
    //if (pt_csr.ptd.int_grp2.intreg.wr_axi_rd_resp_interrupt() == 1) {
    //    PLOG_ERR("ptd.wr_axi_rd_resp asserted!");
    //}
    //if (pt_csr.ptd.int_grp2.intreg.rdrsp_axi_interrupt() == 1) {
    //    PLOG_ERR("ptd.rdrsp_axi asserted!");
    //}
    //if (pt_csr.ptd.int_grp2.intreg.wrrsp_axi_interrupt() == 1) {
    //    PLOG_ERR("ptd.wrrsp_axi asserted!");
    //}
    if (pt_csr.ptd.int_grp2.intreg.spurious_rd_resp_interrupt() == 1) {
        PLOG_ERR("ptd.spurious_rd_resp asserted!");
    }
    if (pt_csr.ptd.int_grp2.intreg.spurious_wr_resp_interrupt() == 1) {
        PLOG_ERR("ptd.spurious_wr_resp asserted!");
    }
    if (pt_csr.ptd.int_grp2.intreg.wrreq_info_first_missing_interrupt() == 1) {
        PLOG_ERR("ptd.wrreq_info_first_missing asserted!");
    }
    if (pt_csr.ptd.int_grp2.intreg.wrreq_num_bytes_zero_interrupt() == 1) {
        PLOG_ERR("ptd.wrreq_num_bytes_zero asserted!");
    }
    if (pt_csr.ptd.int_grp2.intreg.wrreq_num_bytes_more_than_64_interrupt() == 1) {
        PLOG_ERR("ptd.wrreq_num_bytes_more_than_64 asserted");
    }

    sknobs_set_value((char*)"pt_csr/ptd/int_grp2", pt_csr.ptd.int_grp2.intreg.all().convert_to<unsigned long long>());
    pt_csr.ptd.int_intf.intreg.read();
    pt_csr.ptd.int_intf.intreg.show();
    if (pt_csr.ptd.int_intf.intreg.pb_sop_err_interrupt() == 1) {
        PLOG_ERR("ptd.pb_sop_err asserted!");
    }
    if (pt_csr.ptd.int_intf.intreg.pb_eop_err_interrupt() == 1) {
        PLOG_ERR("ptd.pb_eop_err asserted!");
    }
    if (pt_csr.ptd.int_intf.intreg.npv_resub_err_interrupt() == 1) {
        PLOG_ERR("ptd.npv_resub_err asserted!");
    }
    if (pt_csr.ptd.int_intf.intreg.npv_resub_sop_err_interrupt() ==1 ) {
        PLOG_ERR("ptd.npv_resub_sop_err asserted!");
    }
    if (pt_csr.ptd.int_intf.intreg.npv_resub_eop_err_interrupt() == 1) {
        PLOG_ERR("ptd.npv_resub_eop_err asserted!");
    }
    if (pt_csr.ptd.int_intf.intreg.ma_err_interrupt() == 1) {
        PLOG_ERR("ptd.ma_err asserted!");
    }
    if (pt_csr.ptd.int_intf.intreg.ma_sop_err_interrupt() == 1) {
        PLOG_ERR("ptd.ma_sop_err asserted!");
    }
    if (pt_csr.ptd.int_intf.intreg.ma_eop_err_interrupt() == 1) {
        PLOG_ERR("ptd.ma_eop_err asserted!");
    }
    sknobs_set_value((char*)"pt_csr/ptd/int_intf", pt_csr.ptd.int_intf.intreg.all().convert_to<unsigned long long>());
    pt_csr.ptd.int_grp1.intreg.read();
    pt_csr.ptd.int_grp1.intreg.show();
    if (pt_csr.ptd.int_grp1.intreg.rcv_phv_dma_ptr_interrupt() == 1) {
        PLOG_ERR("ptd.rcv_phv_dma_ptr asserted!");
    }
    if (pt_csr.ptd.int_grp1.intreg.rcv_phv_addr_interrupt() == 1) {
        PLOG_ERR("ptd.rcv_phv_addr asserted!");
    }
    if (pt_csr.ptd.int_grp1.intreg.rcv_exceed_16byte_interrupt() == 1) {
        PLOG_ERR("ptd.rcv_exceed_16byte asserted!");
    }
    if (pt_csr.ptd.int_grp1.intreg.rcv_phv_not_sop_interrupt() == 1) {
        PLOG_ERR("ptd.rcv_phv_not_sop asserted!");
    }
    if (pt_csr.ptd.int_grp1.intreg.rcv_pkt_order_ff_full_interrupt() == 1) {
        PLOG_ERR("ptd.rcv_pkt_order_ff_full asserted!");
    }
    if (pt_csr.ptd.int_grp1.intreg.rcv_pend_phv_more_than_2_interrupt() == 1) {
        PLOG_ERR("ptd.rcv_pend_phv_more_than_2 asserted!");
    }
    if (pt_csr.ptd.int_grp1.intreg.rcv_pend_phv_less_interrupt() == 1) {
        PLOG_ERR("ptd.rcv_pend_phv_less asserted!");
    }
    //if (pt_csr.ptd.int_grp1.intreg.rcv_cmd_nop_eop_interrupt() == 1) {
    //    PLOG_ERR("ptd.rcv_cmd_nop_eop asserted!");
    //}
    if (pt_csr.ptd.int_grp1.intreg.rcv_cmd_out_not_sop_interrupt() == 1) {
        PLOG_ERR("ptd.rcv_cmd_out_not_sop asserted!");
    }
    if (pt_csr.ptd.int_grp1.intreg.rcv_pkt2mem_seen_interrupt() == 1) {
        PLOG_ERR("ptd.rcv_pkt2mem_seen asserted!");
    }
    if (pt_csr.ptd.int_grp1.intreg.rcv_skip_seen_interrupt() == 1) {
        PLOG_ERR("ptd.rcv_skip_seen asserted!");
    }
    //if (pt_csr.ptd.int_grp1.intreg.rcv_phv_eop_no_cmd_eop_interrupt() == 1) {
    //    PLOG_ERR("ptd.rcv_phv_eop_no_cmd_eop asserted!");
    //}
    if (pt_csr.ptd.int_grp1.intreg.rcv_m2m_dst_not_seen_interrupt() == 1) {
        PLOG_ERR("ptd.rcv_m2m_dst_not_seen asserted!");
    }
    if (pt_csr.ptd.int_grp1.intreg.rcv_m2m_src_not_seen_interrupt() == 1) {
        PLOG_ERR("ptd.rcv_m2m_src_not_seen asserted!");
    }
    if (pt_csr.ptd.int_grp1.intreg.rdreq_invalid_cmd_seen_interrupt() == 1) {
        PLOG_ERR("ptd.rdreq_invalid_cmd_seen asserted!");
    }
    if (pt_csr.ptd.int_grp1.intreg.rdreq_mem2mem_psize_zero_interrupt() == 1) {
        PLOG_ERR("ptd.rdreq_mem2mem_psize_zero asserted!");
    }
    if (pt_csr.ptd.int_grp1.intreg.rdreq_m2m_phv2mem_exceed_16byte_interrupt() == 1) {
        PLOG_ERR("ptd.rdreq_m2m_phv2mem_exceed_16byte asserted!");
    }
    if (pt_csr.ptd.int_grp1.intreg.rdreq_phv2mem_fence_exceed_16byte_interrupt() == 1) {
        PLOG_ERR("ptd.rdreq_phv2mem_fence_exceed_16byte asserted!");
    }
    if (pt_csr.ptd.int_grp1.intreg.rdreq_mem2pkt_psize_zero_interrupt() == 1) {
        PLOG_ERR("ptd.rdreq_mem2pkt_psize_zero asserted!");
    }

    sknobs_set_value((char*)"pt_csr/ptd/int_grp1", pt_csr.ptd.int_grp1.intreg.all().convert_to<unsigned long long>());
}

void cap_ptd_eos_int(int chip_id, int inst_id) {
    cap_pt_csr_t & pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, chip_id, inst_id);

    pt_csr.ptd.int_ecc.intreg.read_compare();
    pt_csr.ptd.int_fifo.intreg.read_compare();
    pt_csr.ptd.int_grp1.intreg.read_compare();
    pt_csr.ptd.int_grp2.intreg.read_compare();
    pt_csr.ptd.int_intf.intreg.read_compare();
    
}

void cap_ptd_eos_sta(int chip_id, int inst_id) {
    cap_pt_csr_t & pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, chip_id, inst_id);

    pt_csr.ptd.sta_fifo.read();
    if (pt_csr.ptd.sta_fifo.lat_ff_full() == 1)
        PLOG_ERR("ptd.sta_fifo.lat_ff_full = 1" << endl)
    if (pt_csr.ptd.sta_fifo.lat_ff_empty() == 0) 
        PLOG_ERR("ptd.sta_fifo.lat_ff_empty = 0" << endl)
    if (pt_csr.ptd.sta_fifo.wdata_ff_full() == 1)
        PLOG_ERR("ptd.sta_fifo.wdata_ff_full = 1" << endl)
    if (pt_csr.ptd.sta_fifo.wdata_ff_empty() == 0)
        PLOG_ERR("ptd.sta_fifo.wdata_ff_empty = 0" << endl)
    if (pt_csr.ptd.sta_fifo.rcv_stg_ff_full() == 1)
        PLOG_ERR("ptd.sta_fifo.rcv_stg_ff_full = 1" << endl)
    if (pt_csr.ptd.sta_fifo.rcv_stg_ff_empty() == 0)
        PLOG_ERR("ptd.sta_fifo.rcv_stg_ff_empty = 0" << endl)
    if (pt_csr.ptd.sta_fifo.cmdflit_ff_full() == 1)
        PLOG_ERR("ptd.sta_fifo.cmdflit_ff_full = 1" << endl)
    if (pt_csr.ptd.sta_fifo.cmdflit_ff_empty() == 0)
        PLOG_ERR("ptd.sta_fifo.cmdflit_ff_empty = 0" << endl)
    if (pt_csr.ptd.sta_fifo.cmd_ff_full() == 1)
        PLOG_ERR("ptd.sta_fifo.cmd_ff_full = 1" << endl)
    if (pt_csr.ptd.sta_fifo.cmd_ff_empty() == 0)
        PLOG_ERR("ptd.sta_fifo.cmd_ff_empty = 0" << endl)
    if (pt_csr.ptd.sta_fifo.pkt_ff_full() == 1)
        PLOG_ERR("ptd.sta_fifo.pkt_ff_full = 1" << endl)
    if (pt_csr.ptd.sta_fifo.pkt_ff_empty() == 0)
        PLOG_ERR("ptd.sta_fifo.pkt_ff_empty = 0" << endl)
    if (pt_csr.ptd.sta_fifo.wr_mem_ff_full() == 1)
        PLOG_ERR("ptd.sta_fifo.wr_mem_ff_full = 1" << endl)
    if (pt_csr.ptd.sta_fifo.wr_mem_ff_empty() == 0)
        PLOG_ERR("ptd.sta_fifo.wr_mem_ff_empty = 0" << endl)
    if (pt_csr.ptd.sta_fifo.dfence_ff_full() == 1)
        PLOG_ERR("ptd.sta_fifo.dfence_ff_full = 1" << endl)
    if (pt_csr.ptd.sta_fifo.dfence_ff_empty() == 0)
        PLOG_ERR("ptd.sta_fifo.dfence_ff_empty = 0" << endl)
    if (pt_csr.ptd.sta_fifo.ffence_ff_full() == 1)
        PLOG_ERR("ptd.sta_fifo.ffence_ff_full = 1" << endl)
    if (pt_csr.ptd.sta_fifo.ffence_ff_empty() == 0)
        PLOG_ERR("ptd.sta_fifo.ffence_ff_empty = 0" << endl)
    if (pt_csr.ptd.sta_fifo.phv_lpbk_in_drdy() == 0)
        PLOG_ERR("ptd.sta_fifo.phv_lpbk_in_drdy = 0" << endl)
    if (pt_csr.ptd.sta_fifo.phv_lpbk_out_srdy() == 1)
        PLOG_ERR("ptd.sta_fifo.phv_lpbk_out_srdy = 1" << endl)
    
    pt_csr.ptd.sta_id.read();
    if (pt_csr.ptd.sta_id.wr_pend_cnt() != 0)
        PLOG_ERR("ptd.sta_id.wr_pend_cnt = 0x" << hex << pt_csr.ptd.sta_id.wr_pend_cnt() << endl)
    if (pt_csr.ptd.sta_id.rd_pend_cnt() != 0)
        PLOG_ERR("ptd.sta_id.rd_pend_cnt = 0x" << hex << pt_csr.ptd.sta_id.rd_pend_cnt() << endl)
}

void cap_ptd_bist_test(int chip_id, int inst_id) {
    cap_pt_csr_t & pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, chip_id, inst_id);
    int   loop_cnt = 0;

    pt_csr.ptd.cfg_rdata_mem.bist_run(1);
    pt_csr.ptd.cfg_rdata_mem.write();
    
    pt_csr.ptd.cfg_lat_mem.bist_run(1);
    pt_csr.ptd.cfg_lat_mem.write();

    pt_csr.ptd.cfg_wdata_mem.bist_run(1);
    pt_csr.ptd.cfg_wdata_mem.write();
 
    pt_csr.ptd.cfg_phv_mem.bist_run(1);
    pt_csr.ptd.cfg_phv_mem.write();
    
    pt_csr.ptd.cfg_ffence_mem.bist_run(1);
    pt_csr.ptd.cfg_ffence_mem.write();

    pt_csr.ptd.cfg_dfence_mem.bist_run(1);
    pt_csr.ptd.cfg_dfence_mem.write();

    pt_csr.psp.cfg_psp_phv_mem.bist_run(1);
    pt_csr.psp.cfg_psp_phv_mem.write();

    pt_csr.psp.cfg_psp_out_mem.bist_run(1);
    pt_csr.psp.cfg_psp_out_mem.write();

    pt_csr.psp.cfg_psp_pkt_mem.bist_run(1);
    pt_csr.psp.cfg_psp_pkt_mem.write();

    pt_csr.psp.cfg_lif_table_sram_bist.run(1);
    pt_csr.psp.cfg_lif_table_sram_bist.write();

    pt_csr.psp.cfg_sw_phv_mem.bist_run(1);
    pt_csr.psp.cfg_sw_phv_mem.write();

    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pt_csr.ptd.sta_rdata_mem.read();
        if ((pt_csr.ptd.sta_rdata_mem.bist_done_fail() == 1)||(pt_csr.ptd.sta_rdata_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PTD rdata_mem bist can not be done!" << endl)
    }
    else if (pt_csr.ptd.sta_rdata_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PTD rdata_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PTD rdata mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pt_csr.ptd.sta_lat_mem.read();
        if ((pt_csr.ptd.sta_lat_mem.bist_done_fail() == 1)||(pt_csr.ptd.sta_lat_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PTD lat_mem bist can not be done!" << endl)
    }
    else if (pt_csr.ptd.sta_lat_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PTD lat_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PTD lat_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pt_csr.ptd.sta_wdata_mem.read();
        if ((pt_csr.ptd.sta_wdata_mem.bist_done_fail() == 1)||(pt_csr.ptd.sta_wdata_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PTD wdata_mem bist can not be done!" << endl)
    }
    else if (pt_csr.ptd.sta_wdata_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PTD wdata_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PTD wdata_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pt_csr.ptd.sta_phv_mem.read();
        if ((pt_csr.ptd.sta_phv_mem.bist_done_fail() == 1)||(pt_csr.ptd.sta_phv_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PTD phv_mem bist can not be done!" << endl)
    }
    else if (pt_csr.ptd.sta_phv_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PTD phv_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PTD phv_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pt_csr.ptd.sta_ffence_mem.read();
        if ((pt_csr.ptd.sta_ffence_mem.bist_done_fail() == 1)||(pt_csr.ptd.sta_ffence_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PTD ffence_mem bist can not be done!" << endl)
    }
    else if (pt_csr.ptd.sta_ffence_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PTD ffence_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PTD ffence_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pt_csr.ptd.sta_dfence_mem.read();
        if ((pt_csr.ptd.sta_dfence_mem.bist_done_fail() == 1)||(pt_csr.ptd.sta_dfence_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PTD dfence_mem bist can not be done!" << endl)
    }
    else if (pt_csr.ptd.sta_dfence_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PTD dfence_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PTD dfence_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pt_csr.psp.sta_psp_phv_mem.read();
        if ((pt_csr.psp.sta_psp_phv_mem.bist_done_fail() == 1)||(pt_csr.psp.sta_psp_phv_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, NPV psp_phv_mem bist can not be done!" << endl)
    }
    else if (pt_csr.psp.sta_psp_phv_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, NPV psp_phv_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("NPV psp_phv_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pt_csr.psp.sta_psp_out_mem.read();
        if ((pt_csr.psp.sta_psp_out_mem.bist_done_fail() == 1)||(pt_csr.psp.sta_psp_out_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, NPV psp_out_mem bist can not be done!" << endl)
    }
    else if (pt_csr.psp.sta_psp_out_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, NPV psp_out_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("NPV psp_out_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 100000) {
        pt_csr.psp.sta_lif_table_sram_bist.read();
        if ((pt_csr.psp.sta_lif_table_sram_bist.done_fail() == 1)||(pt_csr.psp.sta_lif_table_sram_bist.done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 100000) {
        PLOG_ERR("FAIL, NPV sta_lif_table_sram_bist bist can not be done!" << endl)
    }
    else if (pt_csr.psp.sta_lif_table_sram_bist.done_fail() == 1) {
        PLOG_ERR("FAIL, NPV sta_lif_table_sram_bist bist fail!" << endl)
    }
    else {
        PLOG_MSG("NPV sta_lif_table_sram_bist bist pass!" << endl)
    }
    

    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pt_csr.psp.sta_sw_phv_mem.read();
        if ((pt_csr.psp.sta_sw_phv_mem.bist_done_fail() == 1)||(pt_csr.psp.sta_sw_phv_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, NPV sta_psp_out_mem bist can not be done!" << endl)
    }
    else if (pt_csr.psp.sta_sw_phv_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, NPV sta_sw_phv_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("NPV sta_sw_phv_mem bist pass!" << endl)
    }

}

void cap_ptd_bist_test_start(int chip_id, int inst_id) {
    cap_pt_csr_t & pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, chip_id, inst_id);
    //int   loop_cnt = 0;

    pt_csr.ptd.cfg_rdata_mem.bist_run(1);
    pt_csr.ptd.cfg_rdata_mem.write();
    
    pt_csr.ptd.cfg_lat_mem.bist_run(1);
    pt_csr.ptd.cfg_lat_mem.write();

    pt_csr.ptd.cfg_wdata_mem.bist_run(1);
    pt_csr.ptd.cfg_wdata_mem.write();
 
    pt_csr.ptd.cfg_phv_mem.bist_run(1);
    pt_csr.ptd.cfg_phv_mem.write();
    
    pt_csr.ptd.cfg_ffence_mem.bist_run(1);
    pt_csr.ptd.cfg_ffence_mem.write();

    pt_csr.ptd.cfg_dfence_mem.bist_run(1);
    pt_csr.ptd.cfg_dfence_mem.write();

    pt_csr.psp.cfg_psp_phv_mem.bist_run(1);
    pt_csr.psp.cfg_psp_phv_mem.write();

    pt_csr.psp.cfg_psp_out_mem.bist_run(1);
    pt_csr.psp.cfg_psp_out_mem.write();

    pt_csr.psp.cfg_psp_pkt_mem.bist_run(1);
    pt_csr.psp.cfg_psp_pkt_mem.write();

    pt_csr.psp.cfg_lif_table_sram_bist.run(1);
    pt_csr.psp.cfg_lif_table_sram_bist.write();

    pt_csr.psp.cfg_sw_phv_mem.bist_run(1);
    pt_csr.psp.cfg_sw_phv_mem.write();

} //_start

void cap_ptd_bist_test_clear(int chip_id, int inst_id) {
    cap_pt_csr_t & pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, chip_id, inst_id);
    //int   loop_cnt = 0;

    pt_csr.ptd.cfg_rdata_mem.bist_run(0);
    pt_csr.ptd.cfg_rdata_mem.write();
    
    pt_csr.ptd.cfg_lat_mem.bist_run(0);
    pt_csr.ptd.cfg_lat_mem.write();

    pt_csr.ptd.cfg_wdata_mem.bist_run(0);
    pt_csr.ptd.cfg_wdata_mem.write();
 
    pt_csr.ptd.cfg_phv_mem.bist_run(0);
    pt_csr.ptd.cfg_phv_mem.write();
    
    pt_csr.ptd.cfg_ffence_mem.bist_run(0);
    pt_csr.ptd.cfg_ffence_mem.write();

    pt_csr.ptd.cfg_dfence_mem.bist_run(0);
    pt_csr.ptd.cfg_dfence_mem.write();

    pt_csr.psp.cfg_psp_phv_mem.bist_run(0);
    pt_csr.psp.cfg_psp_phv_mem.write();

    pt_csr.psp.cfg_psp_out_mem.bist_run(0);
    pt_csr.psp.cfg_psp_out_mem.write();

    pt_csr.psp.cfg_psp_pkt_mem.bist_run(0);
    pt_csr.psp.cfg_psp_pkt_mem.write();

    pt_csr.psp.cfg_lif_table_sram_bist.run(0);
    pt_csr.psp.cfg_lif_table_sram_bist.write();

    pt_csr.psp.cfg_sw_phv_mem.bist_run(0);
    pt_csr.psp.cfg_sw_phv_mem.write();

} //_clear

void cap_ptd_bist_test_chk(int chip_id, int inst_id) {
    cap_pt_csr_t & pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, chip_id, inst_id);
    int   loop_cnt = 0;
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pt_csr.ptd.sta_rdata_mem.read();
        if ((pt_csr.ptd.sta_rdata_mem.bist_done_fail() == 1)||(pt_csr.ptd.sta_rdata_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PTD rdata_mem bist can not be done!" << endl)
    }
    else if (pt_csr.ptd.sta_rdata_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PTD rdata_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PTD rdata mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pt_csr.ptd.sta_lat_mem.read();
        if ((pt_csr.ptd.sta_lat_mem.bist_done_fail() == 1)||(pt_csr.ptd.sta_lat_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PTD lat_mem bist can not be done!" << endl)
    }
    else if (pt_csr.ptd.sta_lat_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PTD lat_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PTD lat_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pt_csr.ptd.sta_wdata_mem.read();
        if ((pt_csr.ptd.sta_wdata_mem.bist_done_fail() == 1)||(pt_csr.ptd.sta_wdata_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PTD wdata_mem bist can not be done!" << endl)
    }
    else if (pt_csr.ptd.sta_wdata_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PTD wdata_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PTD wdata_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pt_csr.ptd.sta_phv_mem.read();
        if ((pt_csr.ptd.sta_phv_mem.bist_done_fail() == 1)||(pt_csr.ptd.sta_phv_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PTD phv_mem bist can not be done!" << endl)
    }
    else if (pt_csr.ptd.sta_phv_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PTD phv_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PTD phv_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pt_csr.ptd.sta_ffence_mem.read();
        if ((pt_csr.ptd.sta_ffence_mem.bist_done_fail() == 1)||(pt_csr.ptd.sta_ffence_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PTD ffence_mem bist can not be done!" << endl)
    }
    else if (pt_csr.ptd.sta_ffence_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PTD ffence_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PTD ffence_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pt_csr.ptd.sta_dfence_mem.read();
        if ((pt_csr.ptd.sta_dfence_mem.bist_done_fail() == 1)||(pt_csr.ptd.sta_dfence_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PTD dfence_mem bist can not be done!" << endl)
    }
    else if (pt_csr.ptd.sta_dfence_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PTD dfence_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PTD dfence_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pt_csr.psp.sta_psp_phv_mem.read();
        if ((pt_csr.psp.sta_psp_phv_mem.bist_done_fail() == 1)||(pt_csr.psp.sta_psp_phv_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, NPV psp_phv_mem bist can not be done!" << endl)
    }
    else if (pt_csr.psp.sta_psp_phv_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, NPV psp_phv_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("NPV psp_phv_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pt_csr.psp.sta_psp_out_mem.read();
        if ((pt_csr.psp.sta_psp_out_mem.bist_done_fail() == 1)||(pt_csr.psp.sta_psp_out_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, NPV psp_out_mem bist can not be done!" << endl)
    }
    else if (pt_csr.psp.sta_psp_out_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, NPV psp_out_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("NPV psp_out_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 100000) {
        pt_csr.psp.sta_lif_table_sram_bist.read();
        if ((pt_csr.psp.sta_lif_table_sram_bist.done_fail() == 1)||(pt_csr.psp.sta_lif_table_sram_bist.done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 100000) {
        PLOG_ERR("FAIL, NPV sta_lif_table_sram_bist bist can not be done!" << endl)
    }
    else if (pt_csr.psp.sta_lif_table_sram_bist.done_fail() == 1) {
        PLOG_ERR("FAIL, NPV sta_lif_table_sram_bist bist fail!" << endl)
    }
    else {
        PLOG_MSG("NPV sta_lif_table_sram_bist bist pass!" << endl)
    }
    

    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pt_csr.psp.sta_sw_phv_mem.read();
        if ((pt_csr.psp.sta_sw_phv_mem.bist_done_fail() == 1)||(pt_csr.psp.sta_sw_phv_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, NPV sta_psp_out_mem bist can not be done!" << endl)
    }
    else if (pt_csr.psp.sta_sw_phv_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, NPV sta_sw_phv_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("NPV sta_sw_phv_mem bist pass!" << endl)
    }

} //_chk

void display_ptd_lat_mem_entry(unsigned addr, bool wr_mem_sel) {
    cap_pt_csr_t & pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, 0, 0);
    cpp_int   read_data;

    if (wr_mem_sel == 1) {
        pt_csr.ptd.cfg_debug_ctrl.dbg_mem_sel(3);
        pt_csr.ptd.cfg_debug_ctrl.write();
    }

    pt_csr.ptd.cfg_dhs_dbg_mem.addr(addr);
    pt_csr.ptd.cfg_dhs_dbg_mem.write();
    pt_csr.ptd.dhs_dbg_mem.entry.read();
    read_data = pt_csr.ptd.dhs_dbg_mem.entry.data();
    pt_csr.ptd.dhs_dbg_mem.show();
    //0: nop, 1: mem2pkt, 2: phv2pkt, 3: phv2mem, 4: pkt2mem, 5: skip, 6: mem2mem
    int cmd_type = cpp_int_helper::s_get_slc(read_data, 3, 0).convert_to<int>();
    int cmd_eop = cpp_int_helper::s_get_slc(read_data, 1, 3).convert_to<int>();
    if ((cmd_type == 6)||(cmd_type == 1)) {  
        int pkt_eop  = cpp_int_helper::s_get_slc(read_data, 1, 4).convert_to<int>();
        int start_offset = cpp_int_helper::s_get_slc(read_data, 6, 5).convert_to<int>();
        int psize    = cpp_int_helper::s_get_slc(read_data, 14, 11).convert_to<int>();
        int rd_id    = cpp_int_helper::s_get_slc(read_data, 7, 25).convert_to<int>();
        int wr_cache = cpp_int_helper::s_get_slc(read_data, 1, 32).convert_to<int>();
        uint64_t wr_axi_addr = cpp_int_helper::s_get_slc(read_data, 64, 33).convert_to<uint64_t>();
        int wr_fence_fence = cpp_int_helper::s_get_slc(read_data, 1, 97).convert_to<int>();
        int wr_data_fence = cpp_int_helper::s_get_slc(read_data, 1, 98).convert_to<int>();
        int wr_round = cpp_int_helper::s_get_slc(read_data, 1, 99).convert_to<int>();
        int rd_sz_64byte = cpp_int_helper::s_get_slc(read_data, 1, 100).convert_to<int>();
        PLOG_MSG("lat_mem[" << dec << addr << "]: cmd_type = " << dec << cmd_type << ", cmdeop = " << cmd_eop << ", pkt_eop = " << pkt_eop << ", start_offset = " << start_offset << ", psize = " << psize << ", rd_id = " << rd_id << ", wr_cache = " << wr_cache << ", wr_axi_addr = 0x" << hex << wr_axi_addr << ", wr_fence_fence = " << wr_fence_fence << ", wr_data_fence = " << wr_data_fence << ", wr_round = " << wr_round << ", rd_sz_64byte = " << rd_sz_64byte << endl)
    }
    else if (cmd_type == 2) {
        int pkt_eop = cpp_int_helper::s_get_slc(read_data, 1, 4).convert_to<int>();
        int phv_size = cpp_int_helper::s_get_slc(read_data, 14, 5).convert_to<int>();
        PLOG_MSG("lat_mem[" << dec << addr << "]: cmd_type = " << dec << cmd_type << ", cmdeop = " << cmd_eop << ", pkt_eop = " << pkt_eop << ", phv_size = " << phv_size << endl)
    }
    else if (cmd_type == 3) {
        int phv_size = cpp_int_helper::s_get_slc(read_data, 14, 4).convert_to<int>();
        int wr_cache = cpp_int_helper::s_get_slc(read_data, 1, 18).convert_to<int>();
        uint64_t wr_axi_addr = cpp_int_helper::s_get_slc(read_data, 64, 19).convert_to<uint64_t>();
        int wr_fence_fence = cpp_int_helper::s_get_slc(read_data, 1, 83).convert_to<int>();
        int wr_data_fence = cpp_int_helper::s_get_slc(read_data, 1, 84).convert_to<int>();
        int wr_round = cpp_int_helper::s_get_slc(read_data, 1, 85).convert_to<int>();
        int barrier  = cpp_int_helper::s_get_slc(read_data, 1, 86).convert_to<int>();
        int pcie_msg = cpp_int_helper::s_get_slc(read_data, 1, 87).convert_to<int>();
        PLOG_MSG("lat_mem[" << dec << addr << "]: cmd_type = " << dec << cmd_type << ", phv_size = " << phv_size << ", wr_cache = " << wr_cache << ", wr_axi_addr = 0x" << hex << wr_axi_addr << ", wr_fence_fence = " << wr_fence_fence << ", wr_data_fence = " << wr_data_fence << ", wr_round = " << wr_round << ", barrier = " << barrier << ", pcie_msg = " << pcie_msg << endl)
    }
    else if (cmd_type == 0) {
        PLOG_MSG("lat_mem[" << dec << addr << "]: cmd_type = " << dec << cmd_type << endl);
    }

}

void display_ptd_wdata_mem_entry(unsigned addr, bool wr_mem_sel) {
    cap_pt_csr_t & pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, 0, 0);

    if (wr_mem_sel == 1) {
        pt_csr.ptd.cfg_debug_ctrl.dbg_mem_sel(2);
        pt_csr.ptd.cfg_debug_ctrl.write();
    }

    pt_csr.ptd.cfg_dhs_dbg_mem.addr(addr);
    pt_csr.ptd.cfg_dhs_dbg_mem.write();
    pt_csr.ptd.dhs_dbg_mem.entry.read();
    PLOG_MSG("wdata_mem[" << dec << addr << "]: 0x" << hex << pt_csr.ptd.dhs_dbg_mem.entry.all() << endl)
}

void display_ptd_rdata_mem_entry(unsigned addr, bool wr_mem_sel) {
    cap_pt_csr_t & pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, 0, 0);

    if (wr_mem_sel == 1) {
        pt_csr.ptd.cfg_debug_ctrl.dbg_mem_sel(1);
        pt_csr.ptd.cfg_debug_ctrl.write();
    }

    pt_csr.ptd.cfg_dhs_dbg_mem.addr(addr);
    pt_csr.ptd.cfg_dhs_dbg_mem.write();
    pt_csr.ptd.dhs_dbg_mem.entry.read();
    PLOG_MSG("rdata_mem[" << dec << addr << "]: 0x" << hex << pt_csr.ptd.dhs_dbg_mem.entry.all() << endl)
}

void display_ptd_phv_mem_entry(unsigned addr, int &dma_ptr, bool wr_mem_sel) {
    cap_pt_csr_t & pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, 0, 0);
    cpp_int read_data;
    int     line_num;

    if (wr_mem_sel == 1) {
        pt_csr.ptd.cfg_debug_ctrl.dbg_mem_sel(0);
        pt_csr.ptd.cfg_debug_ctrl.write();
    }

    pt_csr.ptd.cfg_dhs_dbg_mem.addr(addr);
    pt_csr.ptd.cfg_dhs_dbg_mem.write();
    pt_csr.ptd.dhs_dbg_mem.entry.read();
    read_data = pt_csr.ptd.dhs_dbg_mem.entry.all();
    int sop = cpp_int_helper::s_get_slc(read_data, 1, 513).convert_to<int>();
    int eop = cpp_int_helper::s_get_slc(read_data, 1, 512).convert_to<int>();

    if (sop == 1) {
        cap_phv_intr_txdma_t  txdma_intr;
        read_data = pt_csr.ptd.dhs_dbg_mem.entry.all().convert_to<pu_cpp_int<512> >();
        txdma_intr.all(read_data);
        dma_ptr = txdma_intr.dma_cmd_ptr().convert_to<int>();
        txdma_intr.show();
    }
    PLOG_MSG("phv_mem[" << dec << addr << "]: sop = " << sop << ", eop = " << eop << " 0x" << hex << read_data << endl)

    if (addr >= 24) {
        line_num = addr - 24;
    }
    else if (addr >= 12) {
        line_num = addr = 12;
    }
    else {
        line_num = addr;
    }
    
    if ((dma_ptr>>2) <= line_num) {  
        pu_cpp_int<128> command0, command1, command2, command3, command;

        command0 = cpp_int_helper::s_get_slc(read_data, 128, 384).convert_to<pu_cpp_int<128>>();
        command1 = cpp_int_helper::s_get_slc(read_data, 128, 256).convert_to<pu_cpp_int<128>>();
        command2 = cpp_int_helper::s_get_slc(read_data, 128, 128).convert_to<pu_cpp_int<128>>();
        command3 = cpp_int_helper::s_get_slc(read_data, 128, 0).convert_to<pu_cpp_int<128>>();

        int   sta = 0;
        if ( (dma_ptr>>2) == line_num) {
            sta = dma_ptr % 4;
        }

        for (int i = sta; i < 4; i++) {
            switch(i) {
            case 0: command = command0; break;
            case 1: command = command1; break;
            case 2: command = command2; break;
            case 3: command = command3; break;
            }

            cap_ptd_decoders_cmd_mem2pkt_t    mem2pkt_cmd; 
            cap_ptd_decoders_cmd_phv2pkt_t    phv2pkt_cmd;
            cap_ptd_decoders_cmd_phv2mem_t    phv2mem_cmd;
            cap_ptd_decoders_cmd_mem2mem_t    mem2mem_cmd;

            mem2pkt_cmd.all(command);
            if (mem2pkt_cmd.cmdtype() == 3) {  //phv2mem
                phv2mem_cmd.all(command);
                phv2mem_cmd.show();
            }
            else if (mem2pkt_cmd.cmdtype() == 2) {  //phv2pkt
                phv2pkt_cmd.all(command);
                phv2pkt_cmd.show();
            }
            else if (mem2pkt_cmd.cmdtype() == 1) {  //mem2pkt
                mem2pkt_cmd.all(command);
                mem2pkt_cmd.show();
            }
            else if (mem2pkt_cmd.cmdtype() == 6) {  //mem2mem
                mem2mem_cmd.all(command);
                mem2mem_cmd.show();
            }
            else if (mem2pkt_cmd.cmdtype() == 0) {  //nop
                PLOG_MSG("nop" << endl)
            }
        }
    }

    if (eop == 1)
        dma_ptr = 1024;
}

void display_ptd_lat_mem() {
    for (int i = 0; i < 512; i++) {
        if (i == 0)
            display_ptd_lat_mem_entry(i, true);
        else
            display_ptd_lat_mem_entry(i, false);
    }
}

void display_ptd_wdata_mem() {
    for (int i = 0; i < 256; i++) {
        if (i == 0)
            display_ptd_wdata_mem_entry(i, true);
        else
            display_ptd_wdata_mem_entry(i, false);
    }
}

void display_ptd_rdata_mem() {
    for (int i = 0; i < 512; i++) {
        if (i == 0)
            display_ptd_rdata_mem_entry(i, true);
        else
            display_ptd_rdata_mem_entry(i, false);
    }
}

void display_ptd_phv_mem() {
    int  dma_ptr;
    for (int i = 0; i < 32; i++) {
        if (i == 0)
            display_ptd_phv_mem_entry(i, dma_ptr, true);
        else
            display_ptd_phv_mem_entry(i, dma_ptr, false);
    }
}

void clear_ptd_lat_mem_entry(unsigned addr, bool wr_mem_sel) {
    cap_pt_csr_t & pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, 0, 0);

    if (wr_mem_sel == 1) {
        pt_csr.ptd.cfg_debug_ctrl.dbg_mem_sel(3);
        pt_csr.ptd.cfg_debug_ctrl.write();
    }

    pt_csr.ptd.cfg_dhs_dbg_mem.addr(addr);
    pt_csr.ptd.cfg_dhs_dbg_mem.write();
    pt_csr.ptd.dhs_dbg_mem.entry.all(0);
    pt_csr.ptd.dhs_dbg_mem.entry.write();
}

void clear_ptd_wdata_mem_entry(unsigned addr, bool wr_mem_sel) {
    cap_pt_csr_t & pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, 0, 0);

    if (wr_mem_sel == 1) {
        pt_csr.ptd.cfg_debug_ctrl.dbg_mem_sel(2);
        pt_csr.ptd.cfg_debug_ctrl.write();
    }

    pt_csr.ptd.cfg_dhs_dbg_mem.addr(addr);
    pt_csr.ptd.cfg_dhs_dbg_mem.write();
    pt_csr.ptd.dhs_dbg_mem.entry.all(0);
    pt_csr.ptd.dhs_dbg_mem.entry.write();
}

void clear_ptd_rdata_mem_entry(unsigned addr, bool wr_mem_sel) {
    cap_pt_csr_t & pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, 0, 0);

    if (wr_mem_sel == 1) {
        pt_csr.ptd.cfg_debug_ctrl.dbg_mem_sel(1);
        pt_csr.ptd.cfg_debug_ctrl.write();
    }

    pt_csr.ptd.cfg_dhs_dbg_mem.addr(addr);
    pt_csr.ptd.cfg_dhs_dbg_mem.write();
    pt_csr.ptd.dhs_dbg_mem.entry.all(0);
    pt_csr.ptd.dhs_dbg_mem.entry.write();
}

void clear_ptd_phv_mem_entry(unsigned addr, bool wr_mem_sel) {
    cap_pt_csr_t & pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, 0, 0);

    if (wr_mem_sel == 1) {
        pt_csr.ptd.cfg_debug_ctrl.dbg_mem_sel(0);
        pt_csr.ptd.cfg_debug_ctrl.write();
    }

    pt_csr.ptd.cfg_dhs_dbg_mem.addr(addr);
    pt_csr.ptd.cfg_dhs_dbg_mem.write();
    pt_csr.ptd.dhs_dbg_mem.entry.all(0);
    pt_csr.ptd.dhs_dbg_mem.entry.write();
}

void clear_ptd_lat_mem() {
    for (int i = 0; i < 512; i++) {
        if (i == 0)
            clear_ptd_lat_mem_entry(i, true);
        else
            clear_ptd_lat_mem_entry(i, false);
    }
}

void clear_ptd_wdata_mem() {
    for (int i = 0; i < 256; i++) {
        if (i == 0)
            clear_ptd_wdata_mem_entry(i, true);
        else
            clear_ptd_wdata_mem_entry(i, false);
    }
}

void clear_ptd_rdata_mem() {
    for (int i = 0; i < 512; i++) {
        if (i == 0)
            clear_ptd_rdata_mem_entry(i, true);
        else
            clear_ptd_rdata_mem_entry(i, false);
    }
}

void clear_ptd_phv_mem() {
    for (int i = 0; i < 32; i++) {
        if (i == 0)
            clear_ptd_phv_mem_entry(i, true);
        else
            clear_ptd_phv_mem_entry(i, false);
    }
}

void cap_ptd_dump_cntr() {
    cap_pt_csr_t & pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, 0, 0);

    pt_csr.ptd.CNT_ma.read();
    pt_csr.ptd.CNT_npv_resub.read();
    pt_csr.ptd.CNT_pb.read();
    pt_csr.ptd.CNT_phv.read();
    pt_csr.ptd.CNT_err.read();
    pt_csr.ptd.CNT_axi_wr.read();
    pt_csr.ptd.CNT_axi_rd.read();

    PTD_PRN_MSG(setw(1) << "========================================================= TXDMA PTD CNTRS ==============================================================\n");
    PTD_PRN_MSG(setw(32) << "cnt_ma_sop" << setw(10) << pt_csr.ptd.CNT_ma.sop().convert_to<unsigned int>() << "\n");
    PTD_PRN_MSG(setw(32) << "cnt_npv_resub_sop" << setw(10) << pt_csr.ptd.CNT_npv_resub.sop().convert_to<unsigned int>() << "\n");
    PTD_PRN_MSG(setw(32) << "cnt_pb_sop" << setw(10) << pt_csr.ptd.CNT_pb.sop().convert_to<unsigned int>() << "\n");
    PTD_PRN_MSG(setw(32) << "cnt_phv_no_data" << setw(10) << pt_csr.ptd.CNT_phv.no_data().convert_to<unsigned int>() << "\n");
    PTD_PRN_MSG(setw(32) << "cnt_phv_drop" << setw(10) << pt_csr.ptd.CNT_phv.drop().convert_to<unsigned int>() << "\n");
    PTD_PRN_MSG(setw(32) << "cnt_phv_err" << setw(10) << pt_csr.ptd.CNT_phv.err().convert_to<unsigned int>() << "\n");
    PTD_PRN_MSG(setw(32) << "cnt_phv_recirc" << setw(10) << pt_csr.ptd.CNT_phv.recirc().convert_to<unsigned int>() << "\n");
    PTD_PRN_MSG(setw(32) << "wr_lat_drop" << setw(10) << pt_csr.ptd.CNT_err.wr_lat_drop().convert_to<unsigned int>() << "\n");
    PTD_PRN_MSG(setw(32) << "wr_wdata_drop" << setw(10) << pt_csr.ptd.CNT_err.wr_wdata_drop().convert_to<unsigned int>() << "\n");
    PTD_PRN_MSG(setw(32) << "pkt_force_eop" << setw(10) << pt_csr.ptd.CNT_err.pkt_force_eop().convert_to<unsigned int>() << "\n");
    PTD_PRN_MSG(setw(32) << "pkt_no_eop_seen" << setw(10) << pt_csr.ptd.CNT_err.pkt_no_eop_seen().convert_to<unsigned int>() << "\n");
    PTD_PRN_MSG(setw(32) << "pkt_cmd_eop_no_data" << setw(10) << pt_csr.ptd.CNT_err.pkt_cmd_eop_no_data().convert_to<unsigned int>() << "\n");
    PTD_PRN_MSG(setw(32) << "CNT_axi_wr_req_vld" << setw(10) << pt_csr.ptd.CNT_axi_wr.req_vld().convert_to<unsigned int>() << "\n");
    PTD_PRN_MSG(setw(32) << "CNT_axi_wr_rsp_vld" << setw(10) << pt_csr.ptd.CNT_axi_wr.rsp_vld().convert_to<unsigned int>() << "\n");
    PTD_PRN_MSG(setw(32) << "CNT_axi_rd_req_vld" << setw(10) << pt_csr.ptd.CNT_axi_rd.req_vld().convert_to<unsigned int>() << "\n");
    PTD_PRN_MSG(setw(32) << "CNT_axi_rd_rsp_vld" << setw(10) << pt_csr.ptd.CNT_axi_rd.rsp_vld().convert_to<unsigned int>() << "\n");

}

void cap_ptd_clr_cntr() {
    cap_pt_csr_t & pt_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pt_csr_t, 0, 0);
    
    pt_csr.ptd.CNT_ma.all(0);
    pt_csr.ptd.CNT_npv_resub.all(0);
    pt_csr.ptd.CNT_pb.all(0);
    pt_csr.ptd.CNT_phv.all(0);
    pt_csr.ptd.CNT_err.all(0);
    pt_csr.ptd.CNT_axi_wr.all(0);
    pt_csr.ptd.CNT_axi_rd.all(0);

    pt_csr.ptd.CNT_ma.write();
    pt_csr.ptd.CNT_npv_resub.write();
    pt_csr.ptd.CNT_pb.write();
    pt_csr.ptd.CNT_phv.write();
    pt_csr.ptd.CNT_err.write();
    pt_csr.ptd.CNT_axi_wr.write();
    pt_csr.ptd.CNT_axi_rd.write();
}

void ptd_print_msg(string msg) {
#ifdef _CSV_INCLUDED_
   vpi_printf((PLI_BYTE8*) "%s", msg.c_str());
#else
   std::cout << msg;
#endif
}

