#include "cap_prd_api.h"

// soft reset sequence 
void cap_prd_soft_reset(int chip_id, int inst_id) {
}

// soft reset, value = 1 means apply reset, value = 0 release soft reset
void cap_prd_set_soft_reset(int chip_id, int inst_id, int value) {
}

// init start, no polling yet
void cap_prd_init_start(int chip_id, int inst_id) {
    cap_pr_csr_t & pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, chip_id, inst_id);

    pr_csr.prd.axi_attr.set_access_secure(1);
    if (sknobs_exists((char*)"for_dol_test") == 1) {
        pr_csr.prd.cfg_rdata_mem.ecc_disable_cor(1);
        pr_csr.prd.cfg_rdata_mem.ecc_disable_det(1);
        pr_csr.prd.cfg_rdata_mem.write();
    }
    pr_csr.prd.cfg_ctrl.pkt_phv_sync_err_recovery_en(0);
    pr_csr.prd.cfg_ctrl.write();
}

// poll for init done
void cap_prd_init_done(int chip_id, int inst_id) {
}


// use sknobs base load cfg 
void cap_prd_load_from_cfg(int chip_id, int inst_id) {
    cap_pr_csr_t & pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, chip_id, inst_id);
    
    PLOG_MSG("prd load from config..." << endl)

    pr_csr.load_from_cfg();

    pr_csr.prd.base.read();
}

// eos check, this functions should call eos_cnt, eos_int, eos_sta 
void cap_prd_eos(int chip_id, int inst_id) {
    cap_prd_eos_cnt(chip_id, inst_id);
    cap_prd_eos_int_sim(chip_id, inst_id);
    cap_prd_eos_sta(chip_id, inst_id);
}

void cap_prd_eos_cnt(int chip_id, int inst_id) {
    cap_pr_csr_t & pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, chip_id, inst_id);

    pr_csr.prd.CNT_ma.read();
    if ((pr_csr.prd.CNT_ma.sop() & 0xffffffff) != pr_csr.prd.CNT_ma.eop()) {
        PLOG_ERR("prd.CNT_ma.sop = 0x" << hex << pr_csr.prd.CNT_ma.sop() << " vs eop = 0x" << pr_csr.prd.CNT_ma.eop() << endl)
    if (pr_csr.prd.CNT_ma.err() != 0)
        PLOG_ERR("prd.CNT_ma.err = 0x" << hex << pr_csr.prd.CNT_ma.err() << endl)
    if (pr_csr.prd.CNT_ma.sop_err() != 0)
        PLOG_ERR("prd.CNT_ma.sop_err = 0x" << hex << pr_csr.prd.CNT_ma.sop_err() << endl)
    if (pr_csr.prd.CNT_ma.eop_err() != 0)
        PLOG_ERR("prd.CNT_ma.eop_err = 0x" << hex << pr_csr.prd.CNT_ma.eop_err() << endl)
    }
    sknobs_set_value((char*)"pr_csr/prd/CNT_ma/sop", pr_csr.prd.CNT_ma.sop().convert_to<unsigned long long>());

    pr_csr.prd.CNT_ps_resub_pkt.read();
    if ((pr_csr.prd.CNT_ps_resub_pkt.sop() & 0xffffffff) != pr_csr.prd.CNT_ps_resub_pkt.eop()) 
        PLOG_ERR("prd.CNT_ps_resub_pkt.sop() = 0x" << hex << pr_csr.prd.CNT_ps_resub_pkt.sop() << " vs eop = 0x" << pr_csr.prd.CNT_ps_resub_pkt.eop() << endl)
    if (pr_csr.prd.CNT_ps_resub_pkt.err() != 0)
        PLOG_ERR("prd.CNT_ps_resub_pkt.err = 0x" << pr_csr.prd.CNT_ps_resub_pkt.err() << endl)
    if (pr_csr.prd.CNT_ps_resub_pkt.sop_err() != 0)
        PLOG_ERR("prd.CNT_ps_resub_pkt.sop_err = 0x" << pr_csr.prd.CNT_ps_resub_pkt.sop_err() << endl)
    if (pr_csr.prd.CNT_ps_resub_pkt.eop_err() != 0)
        PLOG_ERR("prd.CNT_ps_resub_pkt.eop_err = 0x" << pr_csr.prd.CNT_ps_resub_pkt.eop_err() << endl)
    sknobs_set_value((char*)"pr_csr/prd/CNT_ps_resub_pkt/sop", pr_csr.prd.CNT_ps_resub_pkt.sop().convert_to<unsigned long long>());

    pr_csr.prd.CNT_ps_resub_phv.read();
    if ((pr_csr.prd.CNT_ps_resub_phv.sop() & 0xffffffff) != pr_csr.prd.CNT_ps_resub_phv.eop())
        PLOG_ERR("prd.CNT_ps_resub_phv.sop() = 0x" << hex << pr_csr.prd.CNT_ps_resub_phv.sop() << " vs eop = 0x" << pr_csr.prd.CNT_ps_resub_phv.eop() << endl)
    if (pr_csr.prd.CNT_ps_resub_phv.err() != 0)
        PLOG_ERR("prd.CNT_ps_resub_phv.err = 0x" << hex << pr_csr.prd.CNT_ps_resub_phv.err() << endl)
    if (pr_csr.prd.CNT_ps_resub_phv.sop_err() != 0)
        PLOG_ERR("prd.CNT-ps_resub_phv.sop_err = 0x" << hex << pr_csr.prd.CNT_ps_resub_phv.sop_err() << endl)
    if (pr_csr.prd.CNT_ps_resub_phv.eop_err() != 0)
        PLOG_ERR("prd.CNT-ps_resub_phv.eop_err = 0x" << hex << pr_csr.prd.CNT_ps_resub_phv.eop_err() << endl)
    sknobs_set_value((char*)"pr_csr/prd/CNT_ps_resub_phv/sop", pr_csr.prd.CNT_ps_resub_phv.sop().convert_to<unsigned long long>());

    pr_csr.prd.CNT_ps_pkt.read();
    if ((pr_csr.prd.CNT_ps_pkt.sop() & 0xffffffff) != pr_csr.prd.CNT_ps_pkt.eop())
        PLOG_ERR("prd.CNT_ps_pkt.sop() = 0x" << hex << pr_csr.prd.CNT_ps_pkt.sop() << " vs eop = 0x" << pr_csr.prd.CNT_ps_pkt.eop() << endl)
    if (pr_csr.prd.CNT_ps_pkt.err() != 0)
        PLOG_ERR("prd.CNT_ps_pkt.err() = 0x" << hex << pr_csr.prd.CNT_ps_pkt.err() << endl)
    if (pr_csr.prd.CNT_ps_pkt.sop_err() != 0)
        PLOG_ERR("prd.CNT_ps_pkt.sop_err() = 0x" << hex << pr_csr.prd.CNT_ps_pkt.sop_err() << endl)
    if (pr_csr.prd.CNT_ps_pkt.eop_err() != 0)
        PLOG_ERR("prd.CNT_ps_pkt.eop_err() = 0x" << hex << pr_csr.prd.CNT_ps_pkt.eop_err() << endl)
    sknobs_set_value((char*)"pr_csr/prd/CNT_ps_pkt/sop", pr_csr.prd.CNT_ps_pkt.sop().convert_to<unsigned long long>());

    pr_csr.prd.CNT_phv.read();
    sknobs_set_value((char*)"pr_csr/prd/CNT_phv/no_data", pr_csr.prd.CNT_phv.no_data().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pr_csr/prd/CNT_phv/drop", pr_csr.prd.CNT_phv.drop().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pr_csr/prd/CNT_phv/err", pr_csr.prd.CNT_phv.err().convert_to<unsigned long long>());
    sknobs_set_value((char*)"pr_csr/prd/CNT_phv/recirc", pr_csr.prd.CNT_phv.recirc().convert_to<unsigned long long>());

    pr_csr.prd.CNT_pkt.read();
    sknobs_set_value((char*)"pr_csr/prd/CNT_pkt/drop", pr_csr.prd.CNT_pkt.drop().convert_to<unsigned long long>());
}

void cap_prd_eos_int_sim(int chip_id, int inst_id) {
    cap_pr_csr_t & pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, chip_id, inst_id); 
    string         csr_name;
    
    if (sknobs_exists((char*)"no_int_chk") == 0) {
        if (sknobs_exists((char*)"PEN_IP_MEM_ECC_CORRUPT_ENABLE")) {
            pr_csr.prd.int_ecc.intreg.read();
            if (sknobs_exists((char*)"no_ecc_chk") == 0) {
                if (pr_csr.prd.int_ecc.intreg.rdata_mem_correctable_interrupt() != 1)
                    PLOG_ERR("pr_csr.prd.int_ecc.intreg.rdata_mem_correctable should be 1!" << endl)
                if (pr_csr.prd.int_ecc.intreg.pkt_mem_correctable_interrupt() != 1)
                    PLOG_ERR("pr_csr.prd.int_ecc.intreg.pkt_mem_correctable should be 1!" << endl)
            }
            if (pr_csr.prd.int_ecc.intreg.rdata_mem_uncorrectable_interrupt() != 0)
                PLOG_ERR("pr_csr.prd.int_ecc.intreg.rdata_mem_uncorrectable should be 1!" << endl)
            if (pr_csr.prd.int_ecc.intreg.pkt_mem_uncorrectable_interrupt() != 0)
                PLOG_ERR("pr_csr.prd.int_ecc.intreg.pkt_mem_uncorrectable should be 1!" << endl)
        }
        else {
            pr_csr.prd.int_ecc.intreg.read_compare();
            pr_csr.prd.int_ecc.intreg.show();
        }
        pr_csr.prd.int_fifo.intreg.read_compare();
        pr_csr.prd.int_fifo.intreg.show();
        pr_csr.prd.int_intf.intreg.read_compare();
        pr_csr.prd.int_intf.intreg.show();
    }
    else {
        pr_csr.prd.int_ecc.intreg.read();
        sknobs_set_value((char*)"pr_csr/prd/int_ecc",  pr_csr.prd.int_ecc.intreg.all().convert_to<unsigned long long>()); 
        pr_csr.prd.int_intf.intreg.read();
        sknobs_set_value((char*)"pr_csr/prd/int_intf", pr_csr.prd.int_intf.intreg.all().convert_to<unsigned long long>()); 
    }
    pr_csr.prd.int_grp2.intreg.read();
    pr_csr.prd.int_grp2.intreg.show();
    if (pr_csr.prd.int_grp2.intreg.wr_axi_rd_err_recovery_interrupt() == 1) {
        PLOG_ERR("prd.wr_axi_rd_err_recovery asserted!");
    }
    if (pr_csr.prd.int_grp2.intreg.wr_not_enuf_pkt_bytes_interrupt() == 1) {
        PLOG_ERR("prd.wr_not_enuf_pkt_bytes asserted!");
    }
    if (pr_csr.prd.int_grp2.intreg.wr_invalid_cmd_interrupt() == 1) {
        PLOG_ERR("prd.wr_invalid_cmd asserted!");
    }
    if (pr_csr.prd.int_grp2.intreg.wr_seq_id_interrupt() == 1) {
        //PLOG_ERR("prd.wr_seq_id asserted!");
    }
    if (pr_csr.prd.int_grp2.intreg.wr_no_data_pkt_cmd_interrupt() == 1)  {
        PLOG_ERR("prd.wr_no_data_pkt_cmd asserted!");
    }
    if (pr_csr.prd.int_grp2.intreg.pkt_not_sop_interrupt() == 1) {
        PLOG_ERR("prd.pkt_no_sop asserted!");
    }
    if (pr_csr.prd.int_grp2.intreg.spurious_rd_resp_interrupt() == 1) {
        PLOG_ERR("prd.spurious_rd_resp asserted!");
    }
    if (pr_csr.prd.int_grp2.intreg.rdrsp_axi_id_out_of_range_interrupt() == 1) {
        PLOG_ERR("prd.rdrsp_axi_id_out_of_range asserted!");
    }
    if (pr_csr.prd.int_grp2.intreg.spurious_wr_resp_interrupt() == 1) {
        PLOG_ERR("prd.spurious_wr_resp asserted!");
    }
    if (pr_csr.prd.int_grp2.intreg.wrreq_info_first_missing_interrupt() == 1) {
        PLOG_ERR("prd.wrreq_info_first_missing asserted!");
    }
    if (pr_csr.prd.int_grp2.intreg.wrreq_num_bytes_zero_interrupt() == 1) {
        PLOG_ERR("prd.wrreq_num_bytes_zero asserted!");
    }
    if (pr_csr.prd.int_grp2.intreg.wrreq_num_bytes_more_than_64_interrupt() == 1) {
        PLOG_ERR("prd.wrreq_num_bytes_more_than_64 asserted!");
    }
    sknobs_set_value((char*)"pr_csr/prd/int_grp2", pr_csr.prd.int_grp2.intreg.all().convert_to<unsigned long long>()); 

    pr_csr.prd.int_grp1.intreg.read();
    pr_csr.prd.int_grp1.intreg.show();

    sknobs_set_value((char*)"pr_csr/prd/int_grp1", pr_csr.prd.int_grp1.intreg.all().convert_to<unsigned long long>()); 
    if (pr_csr.prd.int_grp1.intreg.rcv_phv_dma_ptr_interrupt() == 1) {
        PLOG_ERR("prd.rcv_phv_dma_ptr asserted!");
    }
    if (pr_csr.prd.int_grp1.intreg.rcv_phv_addr_interrupt() == 1) {
        PLOG_ERR("prd.rcv_phv_addr asserted!");
    }
    if (pr_csr.prd.int_grp1.intreg.rcv_exceed_16byte_interrupt() == 1) {
        PLOG_ERR("prd.rcv_exceed_16byte asserted!");
    }
    if (pr_csr.prd.int_grp1.intreg.rcv_phv_not_sop_interrupt() == 1) {
        PLOG_ERR("prd.rcv_phv_not_sop asserted!");
    }
    if (pr_csr.prd.int_grp1.intreg.rcv_pkt_order_ff_full_interrupt() == 1) {
        PLOG_ERR("prd.rcv_pkt_order_ff_full asserted!");
    }
    if (pr_csr.prd.int_grp1.intreg.rcv_pend_phv_more_than_2_interrupt() == 1) {
        PLOG_ERR("prd.rcv_pend_phv_more_than_2 asserted!");
    }
    if (pr_csr.prd.int_grp1.intreg.rcv_pend_phv_less_interrupt() == 1) {
        PLOG_ERR("prd.rcv_pend_phv_less asserted!");
    }
    if (pr_csr.prd.int_grp1.intreg.rcv_cmd_out_not_sop_interrupt() == 1) {
        PLOG_ERR("prd.rcv_cmd_out_not_sop asserted!");
    }
    if (pr_csr.prd.int_grp1.intreg.rcv_no_data_but_pkt2mem_cmd_interrupt() == 1) {
        PLOG_ERR("prd.rcv_no_data_but_pkt2mem_cmd asserted!");
    }
    if (pr_csr.prd.int_grp1.intreg.rcv_no_data_but_skip_cmd_interrupt() == 1) {
        PLOG_ERR("prd.rcv_no_data_but_skip_cmd asserted!");
    }
    if (pr_csr.prd.int_grp1.intreg.rcv_mem2pkt_seen_interrupt() == 1) {
        PLOG_ERR("prd.rcv_mem2pkt_seen asserted!");
    }
    if (pr_csr.prd.int_grp1.intreg.rcv_phv2pkt_seen_interrupt() == 1) {
        PLOG_ERR("prd.rcv_phv2pkt_seen asserted!");
    }
    if (pr_csr.prd.int_grp1.intreg.rcv_m2m_dst_not_seen_interrupt() == 1) {
        PLOG_ERR("prd.rcv_m2m_dst_not_seen asserted!");
    }
    if (pr_csr.prd.int_grp1.intreg.rcv_m2m_src_not_seen_interrupt() == 1) {
        PLOG_ERR("prd.rcv_m2m_src_not_seen asserted!");
    }
    if (pr_csr.prd.int_grp1.intreg.rdreq_invalid_cmd_seen_interrupt() == 1) {
        PLOG_ERR("prd.rdreq_invalid_cmd_seen asserted!");
    }
    if (pr_csr.prd.int_grp1.intreg.rdreq_mem2mem_psize_zero_interrupt() == 1) {
        PLOG_ERR("prd.rdreq_mem2mem_psize_zero asserted!");
    }
    if (pr_csr.prd.int_grp1.intreg.rdreq_m2m_phv2mem_exceed_16byte_interrupt() == 1) {
        PLOG_ERR("prd.rdreq_m2m_phv2mem_exceed_16byte asserted!");
    }
    if (pr_csr.prd.int_grp1.intreg.rdreq_phv2mem_fence_exceed_16byte_interrupt() ==1 ) {
        PLOG_ERR("prd.rdreq_phv2mem_fence_exceed_16byte asserted!");
    }
    if (pr_csr.prd.int_grp1.intreg.rdreq_pkt2mem_psize_zero_interrupt() == 1) {
        PLOG_ERR("prd.rdreq_pkt2mem_psize_zero asserted!");
    }
    if (pr_csr.prd.int_grp1.intreg.rdreq_skip_psize_zero_interrupt() ==1 ) {
        PLOG_ERR("prd.rdreq_skip_psize_zero asserted!");
    }
}

void cap_prd_eos_int(int chip_id, int inst_id) {
    cap_pr_csr_t & pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, chip_id, inst_id); 

    pr_csr.prd.int_ecc.intreg.read_compare();
    pr_csr.prd.int_fifo.intreg.read_compare();
    pr_csr.prd.int_grp1.intreg.read_compare();
    pr_csr.prd.int_grp2.intreg.read_compare();
    pr_csr.prd.int_intf.intreg.read_compare();
}

void cap_prd_eos_sta(int chip_id, int inst_id) {
    cap_pr_csr_t & pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, chip_id, inst_id);

    pr_csr.prd.sta_fifo.read();
    if (pr_csr.prd.sta_fifo.rd_lat_ff_full() == 1)
        PLOG_ERR("pr_csr.prd.sta_fifo.rd_lat_ff_full is 1" << endl)
    if (pr_csr.prd.sta_fifo.rd_lat_ff_empty() == 0)
        PLOG_ERR("pr_csr.prd.sta_fifo.rd_lat_ff_empty is 0" << endl)
    if (pr_csr.prd.sta_fifo.wr_lat_ff_full() == 1)
        PLOG_ERR("pr_csr.prd.sta_fifo.wr_lat_ff_full is 1" << endl)
    if (pr_csr.prd.sta_fifo.wr_lat_ff_empty() == 0)
        PLOG_ERR("pr_csr.prd.sta_fifo.wr_lat_ff_empty is 0" << endl)
    if (pr_csr.prd.sta_fifo.wdata_ff_full() == 1)
        PLOG_ERR("pr_csr.prd.sta_fifo.wdata_ff_full is 1" << endl)
    if (pr_csr.prd.sta_fifo.wdata_ff_empty() == 0)
        PLOG_ERR("pr_csr.prd.sta_fifo.wdata_ff_empty is 0" << endl)
    if (pr_csr.prd.sta_fifo.rcv_stg_ff_full() == 1)
        PLOG_ERR("pr_csr.prd.sta_fifo.rcv_stg_ff_full is 1" << endl)
    if (pr_csr.prd.sta_fifo.rcv_stg_ff_empty() == 0)
        PLOG_ERR("pr_csr.prd.sta_fifo.rcv_stg_ff_empty is 0" << endl)
    if (pr_csr.prd.sta_fifo.cmdflit_ff_full() == 1)
        PLOG_ERR("pr_csr.prd.sta_fifo.cmdflit_ff_full is 1" << endl)
    if (pr_csr.prd.sta_fifo.cmdflit_ff_empty() == 0)
        PLOG_ERR("pr_csr.prd.sta_fifo.cmdflit_ff_empty is 0" << endl)
    if (pr_csr.prd.sta_fifo.cmd_ff_full() == 1)
        PLOG_ERR("pr_csr.prd.sta_fifo.cmd_ff_full is 1" << endl)
    if (pr_csr.prd.sta_fifo.cmd_ff_empty() == 0)
        PLOG_ERR("pr_csr.prd.sta_fifo.cmd_ff_empty is 0" << endl)
    if (pr_csr.prd.sta_fifo.pkt_ff_full() == 1)
        PLOG_ERR("pr_csr.prd.sta_fifo.pkt_ff_full is 1" << endl)
    if (pr_csr.prd.sta_fifo.pkt_ff_empty() == 0)
        PLOG_ERR("pr_csr.prd.sta_fifo.pkt_ff_empty is 0" << endl)
    if (pr_csr.prd.sta_fifo.pkt_order_ff_full() == 1)
        PLOG_ERR("pr_csr.prd.sta_fifo.pkt_order_ff_full is 1" << endl)
    if (pr_csr.prd.sta_fifo.pkt_order_ff_empty() == 0)
        PLOG_ERR("pr_csr.prd.sta_fifo.pkt_order_ff_empty is 0" << endl)
    if (pr_csr.prd.sta_fifo.pkt_stg_ff_full() == 1)
        PLOG_ERR("pr_csr.prd.sta_fifo.pkt_stg_ff_full is 1" << endl)
    if (pr_csr.prd.sta_fifo.pkt_stg_ff_empty() == 0)
        PLOG_ERR("pr_csr.prd.sta_fifo.pkt_stg_ff_empty is 0" << endl)
    if (pr_csr.prd.sta_fifo.wr_mem_ff_full() == 1)
        PLOG_ERR("pr_csr.prd.sta_fifo.wr_mem_ff_full is 1" << endl)
    if (pr_csr.prd.sta_fifo.wr_mem_ff_empty() == 0)
        PLOG_ERR("pr_csr.prd.sta_fifo.wr_mem_ff_empty is 0" << endl)
    if (pr_csr.prd.sta_fifo.dfence_ff_full() == 1)
        PLOG_ERR("pr_csr.prd.sta_fifo.dfence_ff_full is 1" << endl)
    if (pr_csr.prd.sta_fifo.dfence_ff_empty() == 0)
        PLOG_ERR("pr_csr.prd.sta_fifo.dfence_ff_empty is 0" << endl)
    if (pr_csr.prd.sta_fifo.ffence_ff_full() == 1)
        PLOG_ERR("pr_csr.prd.sta_fifo.ffence_ff_full is 1" << endl)
    if (pr_csr.prd.sta_fifo.ffence_ff_empty() == 0)
        PLOG_ERR("pr_csr.prd.sta_fifo.ffence_ff_empty is 0" << endl)
    if (pr_csr.prd.sta_fifo.phv_lpbk_in_drdy() == 0)
        PLOG_ERR("pr_csr.prd.sta_fifo.phv_lpbk_in_drdy is 0" << endl)
    if (pr_csr.prd.sta_fifo.phv_lpbk_out_srdy() == 1)
        PLOG_ERR("pr_csr.prd.sta_fifo.phv_lpbk_out_srdy is 1" << endl)
            
    pr_csr.prd.sta_id.read();
    if (pr_csr.prd.sta_id.wr_pend_cnt() != 0)
        PLOG_ERR("pr_csr.prd.sta_id.wr_pend_cnt = 0x" << hex << pr_csr.prd.sta_id.wr_pend_cnt() << endl)
    if (pr_csr.prd.sta_id.rd_pend_cnt() != 0)
        PLOG_ERR("pr_csr.prd.sta_id.rd_pend_cnt = 0x" << hex << pr_csr.prd.sta_id.rd_pend_cnt() << endl)

    pr_csr.prd.sta_xoff.read();
    if (pr_csr.prd.sta_xoff.numphv_counter() != 0)
        PLOG_ERR("prd.sta_xoff.numphv_counter = 0x" << hex << pr_csr.prd.sta_xoff.numphv_counter() << endl)
    if (pr_csr.prd.sta_xoff.numphv_xoff() != 0)
        PLOG_ERR("prd.sta_xoff.numphv_xoff = 1" << endl)
    if (pr_csr.prd.sta_xoff.hostq_xoff_counter() != 0)
        PLOG_ERR("prd.sta_xoff.hostq_xoff_counter = 0x" << hex << pr_csr.prd.sta_xoff.hostq_xoff_counter() << endl)
    if (pr_csr.prd.sta_xoff.pkt_xoff_counter() != 0)
        PLOG_ERR("prd.sta_xoff.pkt_xoff_counter = 0x" << hex << pr_csr.prd.sta_xoff.pkt_xoff_counter() << endl)
    if (pr_csr.prd.sta_xoff.host_pbus_xoff() != 0)
        PLOG_ERR("prd.sta_xoff.host_pbus_xoff = 1" << endl)
    if (pr_csr.prd.sta_xoff.pkt_pbus_xoff() != 0)
        PLOG_ERR("prd.sta_xoff.pkt_pbus_xoff = 1" << endl)
    if (pr_csr.prd.sta_xoff.pbpr_p15_pbus_xoff() != 0)
        PLOG_ERR("prd.sta_xoff.pbpr_p15_pbus_xoff = 1" << endl)
    if (pr_csr.prd.sta_xoff.pkt_ff_depth() != 0)
        PLOG_ERR("prd.sta_xoff.pkt_ff_depth = 0x" << pr_csr.prd.sta_xoff.pkt_ff_depth() << endl)
}

void cap_prd_bist_test(int chip_id, int inst_id) {
    cap_pr_csr_t & pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, chip_id, inst_id);
    int   loop_cnt = 0;

    pr_csr.prd.cfg_rdata_mem.bist_run(1);
    pr_csr.prd.cfg_rdata_mem.write();

    pr_csr.prd.cfg_pkt_mem.bist_run(1);
    pr_csr.prd.cfg_pkt_mem.write();
    
    pr_csr.prd.cfg_lat_mem.bist_run(1);
    pr_csr.prd.cfg_lat_mem.write();

    pr_csr.prd.cfg_phv_mem.bist_run(1);
    pr_csr.prd.cfg_phv_mem.write();
    
    pr_csr.prd.cfg_ffence_mem.bist_run(1);
    pr_csr.prd.cfg_ffence_mem.write();

    pr_csr.prd.cfg_dfence_mem.bist_run(1);
    pr_csr.prd.cfg_dfence_mem.write();

    pr_csr.psp.cfg_psp_phv_mem.bist_run(1);
    pr_csr.psp.cfg_psp_phv_mem.write();

    pr_csr.psp.cfg_psp_out_mem.bist_run(1);
    pr_csr.psp.cfg_psp_out_mem.write();

    pr_csr.psp.cfg_psp_pkt_mem.bist_run(1);
    pr_csr.psp.cfg_psp_pkt_mem.write();

    pr_csr.psp.cfg_lif_table_sram_bist.run(1);
    pr_csr.psp.cfg_lif_table_sram_bist.write();

    pr_csr.psp.cfg_sw_phv_mem.bist_run(1);
    pr_csr.psp.cfg_sw_phv_mem.write();

    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pr_csr.prd.sta_rdata_mem.read();
        if ((pr_csr.prd.sta_rdata_mem.bist_done_fail() == 1)||(pr_csr.prd.sta_rdata_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PRD rdata_mem bist can not be done!" << endl)
    }
    else if (pr_csr.prd.sta_rdata_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PRD rdata_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PRD rdata mem bist pass!" << endl)
    }

    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pr_csr.prd.sta_pkt_mem.read();
        if ((pr_csr.prd.sta_pkt_mem.bist_done_fail() == 1)||(pr_csr.prd.sta_pkt_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PRD pkt_mem bist can not be done!" << endl)
    }
    else if (pr_csr.prd.sta_pkt_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PRD pkt_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PRD pkt_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pr_csr.prd.sta_lat_mem.read();
        if ((pr_csr.prd.sta_lat_mem.bist_done_fail() == 1)||(pr_csr.prd.sta_lat_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PRD lat_mem bist can not be done!" << endl)
    }
    else if (pr_csr.prd.sta_lat_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PRD lat_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PRD lat_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pr_csr.prd.sta_phv_mem.read();
        if ((pr_csr.prd.sta_phv_mem.bist_done_fail() == 1)||(pr_csr.prd.sta_phv_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PRD phv_mem bist can not be done!" << endl)
    }
    else if (pr_csr.prd.sta_phv_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PRD phv_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PRD phv_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pr_csr.prd.sta_ffence_mem.read();
        if ((pr_csr.prd.sta_ffence_mem.bist_done_fail() == 1)||(pr_csr.prd.sta_ffence_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PRD ffence_mem bist can not be done!" << endl)
    }
    else if (pr_csr.prd.sta_ffence_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PRD ffence_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PRD ffence_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pr_csr.prd.sta_dfence_mem.read();
        if ((pr_csr.prd.sta_dfence_mem.bist_done_fail() == 1)||(pr_csr.prd.sta_dfence_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PRD dfence_mem bist can not be done!" << endl)
    }
    else if (pr_csr.prd.sta_dfence_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PRD dfence_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PRD dfence_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pr_csr.psp.sta_psp_phv_mem.read();
        if ((pr_csr.psp.sta_psp_phv_mem.bist_done_fail() == 1)||(pr_csr.psp.sta_psp_phv_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PSP psp_phv_mem bist can not be done!" << endl)
    }
    else if (pr_csr.psp.sta_psp_phv_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PSP psp_phv_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PSP psp_phv_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pr_csr.psp.sta_psp_out_mem.read();
        if ((pr_csr.psp.sta_psp_out_mem.bist_done_fail() == 1)||(pr_csr.psp.sta_psp_out_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PSP psp_out_mem bist can not be done!" << endl)
    }
    else if (pr_csr.psp.sta_psp_out_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PSP psp_out_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PSP psp_out_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 100000) {
        pr_csr.psp.sta_lif_table_sram_bist.read();
        if ((pr_csr.psp.sta_lif_table_sram_bist.done_fail() == 1)||(pr_csr.psp.sta_lif_table_sram_bist.done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 100000) {
        PLOG_ERR("FAIL, PSP sta_lif_table_sram_bist bist can not be done!" << endl)
    }
    else if (pr_csr.psp.sta_lif_table_sram_bist.done_fail() == 1) {
        PLOG_ERR("FAIL, PSP sta_lif_table_sram_bist bist fail!" << endl)
    }
    else {
        PLOG_MSG("PSP sta_lif_table_sram_bist bist pass!" << endl)
    }
    

    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pr_csr.psp.sta_sw_phv_mem.read();
        if ((pr_csr.psp.sta_sw_phv_mem.bist_done_fail() == 1)||(pr_csr.psp.sta_sw_phv_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PSP sta_psp_out_mem bist can not be done!" << endl)
    }
    else if (pr_csr.psp.sta_sw_phv_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PSP sta_sw_phv_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PSP sta_sw_phv_mem bist pass!" << endl)
    }


}

void cap_prd_bist_test_start(int chip_id, int inst_id) {
    cap_pr_csr_t & pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, chip_id, inst_id);
    //int   loop_cnt = 0;

    pr_csr.prd.cfg_rdata_mem.bist_run(1);
    pr_csr.prd.cfg_rdata_mem.write();

    pr_csr.prd.cfg_pkt_mem.bist_run(1);
    pr_csr.prd.cfg_pkt_mem.write();
    
    pr_csr.prd.cfg_lat_mem.bist_run(1);
    pr_csr.prd.cfg_lat_mem.write();

    pr_csr.prd.cfg_phv_mem.bist_run(1);
    pr_csr.prd.cfg_phv_mem.write();
    
    pr_csr.prd.cfg_ffence_mem.bist_run(1);
    pr_csr.prd.cfg_ffence_mem.write();

    pr_csr.prd.cfg_dfence_mem.bist_run(1);
    pr_csr.prd.cfg_dfence_mem.write();

    pr_csr.psp.cfg_psp_phv_mem.bist_run(1);
    pr_csr.psp.cfg_psp_phv_mem.write();

    pr_csr.psp.cfg_psp_out_mem.bist_run(1);
    pr_csr.psp.cfg_psp_out_mem.write();

    pr_csr.psp.cfg_psp_pkt_mem.bist_run(1);
    pr_csr.psp.cfg_psp_pkt_mem.write();

    pr_csr.psp.cfg_lif_table_sram_bist.run(1);
    pr_csr.psp.cfg_lif_table_sram_bist.write();

    pr_csr.psp.cfg_sw_phv_mem.bist_run(1);
    pr_csr.psp.cfg_sw_phv_mem.write();

} //_start


void cap_prd_bist_test_clear(int chip_id, int inst_id) {
    cap_pr_csr_t & pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, chip_id, inst_id);
    //int   loop_cnt = 0;

    pr_csr.prd.cfg_rdata_mem.bist_run(0);
    pr_csr.prd.cfg_rdata_mem.write();

    pr_csr.prd.cfg_pkt_mem.bist_run(0);
    pr_csr.prd.cfg_pkt_mem.write();
    
    pr_csr.prd.cfg_lat_mem.bist_run(0);
    pr_csr.prd.cfg_lat_mem.write();

    pr_csr.prd.cfg_phv_mem.bist_run(0);
    pr_csr.prd.cfg_phv_mem.write();
    
    pr_csr.prd.cfg_ffence_mem.bist_run(0);
    pr_csr.prd.cfg_ffence_mem.write();

    pr_csr.prd.cfg_dfence_mem.bist_run(0);
    pr_csr.prd.cfg_dfence_mem.write();

    pr_csr.psp.cfg_psp_phv_mem.bist_run(0);
    pr_csr.psp.cfg_psp_phv_mem.write();

    pr_csr.psp.cfg_psp_out_mem.bist_run(0);
    pr_csr.psp.cfg_psp_out_mem.write();

    pr_csr.psp.cfg_psp_pkt_mem.bist_run(0);
    pr_csr.psp.cfg_psp_pkt_mem.write();

    pr_csr.psp.cfg_lif_table_sram_bist.run(0);
    pr_csr.psp.cfg_lif_table_sram_bist.write();

    pr_csr.psp.cfg_sw_phv_mem.bist_run(0);
    pr_csr.psp.cfg_sw_phv_mem.write();

} //_clear

void cap_prd_bist_test_chk(int chip_id, int inst_id) {
    cap_pr_csr_t & pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, chip_id, inst_id);
    int   loop_cnt = 0;
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pr_csr.prd.sta_rdata_mem.read();
        if ((pr_csr.prd.sta_rdata_mem.bist_done_fail() == 1)||(pr_csr.prd.sta_rdata_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PRD rdata_mem bist can not be done!" << endl)
    }
    else if (pr_csr.prd.sta_rdata_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PRD rdata_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PRD rdata mem bist pass!" << endl)
    }

    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pr_csr.prd.sta_pkt_mem.read();
        if ((pr_csr.prd.sta_pkt_mem.bist_done_fail() == 1)||(pr_csr.prd.sta_pkt_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PRD pkt_mem bist can not be done!" << endl)
    }
    else if (pr_csr.prd.sta_pkt_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PRD pkt_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PRD pkt_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pr_csr.prd.sta_lat_mem.read();
        if ((pr_csr.prd.sta_lat_mem.bist_done_fail() == 1)||(pr_csr.prd.sta_lat_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PRD lat_mem bist can not be done!" << endl)
    }
    else if (pr_csr.prd.sta_lat_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PRD lat_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PRD lat_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pr_csr.prd.sta_phv_mem.read();
        if ((pr_csr.prd.sta_phv_mem.bist_done_fail() == 1)||(pr_csr.prd.sta_phv_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PRD phv_mem bist can not be done!" << endl)
    }
    else if (pr_csr.prd.sta_phv_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PRD phv_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PRD phv_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pr_csr.prd.sta_ffence_mem.read();
        if ((pr_csr.prd.sta_ffence_mem.bist_done_fail() == 1)||(pr_csr.prd.sta_ffence_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PRD ffence_mem bist can not be done!" << endl)
    }
    else if (pr_csr.prd.sta_ffence_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PRD ffence_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PRD ffence_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pr_csr.prd.sta_dfence_mem.read();
        if ((pr_csr.prd.sta_dfence_mem.bist_done_fail() == 1)||(pr_csr.prd.sta_dfence_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PRD dfence_mem bist can not be done!" << endl)
    }
    else if (pr_csr.prd.sta_dfence_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PRD dfence_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PRD dfence_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pr_csr.psp.sta_psp_phv_mem.read();
        if ((pr_csr.psp.sta_psp_phv_mem.bist_done_fail() == 1)||(pr_csr.psp.sta_psp_phv_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PSP psp_phv_mem bist can not be done!" << endl)
    }
    else if (pr_csr.psp.sta_psp_phv_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PSP psp_phv_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PSP psp_phv_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pr_csr.psp.sta_psp_out_mem.read();
        if ((pr_csr.psp.sta_psp_out_mem.bist_done_fail() == 1)||(pr_csr.psp.sta_psp_out_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PSP psp_out_mem bist can not be done!" << endl)
    }
    else if (pr_csr.psp.sta_psp_out_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PSP psp_out_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PSP psp_out_mem bist pass!" << endl)
    }
    
    loop_cnt = 0;
    while(loop_cnt < 100000) {
        pr_csr.psp.sta_lif_table_sram_bist.read();
        if ((pr_csr.psp.sta_lif_table_sram_bist.done_fail() == 1)||(pr_csr.psp.sta_lif_table_sram_bist.done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 100000) {
        PLOG_ERR("FAIL, PSP sta_lif_table_sram_bist bist can not be done!" << endl)
    }
    else if (pr_csr.psp.sta_lif_table_sram_bist.done_fail() == 1) {
        PLOG_ERR("FAIL, PSP sta_lif_table_sram_bist bist fail!" << endl)
    }
    else {
        PLOG_MSG("PSP sta_lif_table_sram_bist bist pass!" << endl)
    }
    

    loop_cnt = 0;
    while(loop_cnt < 10000) {
        pr_csr.psp.sta_sw_phv_mem.read();
        if ((pr_csr.psp.sta_sw_phv_mem.bist_done_fail() == 1)||(pr_csr.psp.sta_sw_phv_mem.bist_done_pass() == 1)) {
            break;
        }
        loop_cnt ++;
    }
    if (loop_cnt == 10000) {
        PLOG_ERR("FAIL, PSP sta_psp_out_mem bist can not be done!" << endl)
    }
    else if (pr_csr.psp.sta_sw_phv_mem.bist_done_fail() == 1) {
        PLOG_ERR("FAIL, PSP sta_sw_phv_mem bist fail!" << endl)
    }
    else {
        PLOG_MSG("PSP sta_sw_phv_mem bist pass!" << endl)
    }


} //_chk


void display_prd_phv_mem_entry(unsigned addr, int & dma_ptr, bool wr_mem_sel) {
    cap_pr_csr_t & pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, 0, 0);
    cpp_int   read_data;

    if (wr_mem_sel == true) {
        pr_csr.prd.cfg_debug_ctrl.dbg_mem_sel(0);
        pr_csr.prd.cfg_debug_ctrl.write();
    }

    pr_csr.prd.cfg_dhs_dbg_mem.addr(addr);
    pr_csr.prd.cfg_dhs_dbg_mem.write();
    pr_csr.prd.dhs_dbg_mem.entry.read();

    read_data = pr_csr.prd.dhs_dbg_mem.entry.all();
    int sop = cpp_int_helper::s_get_slc(read_data, 1, 513).convert_to<int>();
    int eop = cpp_int_helper::s_get_slc(read_data, 1, 512).convert_to<int>();

    if (sop == 1) {
        cap_phv_intr_rxdma_t   rxdma_intr;
        read_data = pr_csr.prd.dhs_dbg_mem.entry.all().convert_to<pu_cpp_int<512> >();
        rxdma_intr.all(read_data);
        dma_ptr = rxdma_intr.dma_cmd_ptr().convert_to<int>();
        rxdma_intr.show();
    }
    PLOG_MSG("phv_mem[" << dec << addr << "]: sop = " << sop << ", eop = " << eop << " 0x" << hex << read_data << endl)
    
    int line_num;
    if (addr >= 24) {
        line_num = addr - 24;
    }
    else if (addr >= 12) {
        line_num = addr - 12;
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

            cap_ptd_decoders_cmd_pkt2mem_t    pkt2mem_cmd; 
            cap_ptd_decoders_cmd_skip_t       skip_cmd;
            cap_ptd_decoders_cmd_phv2mem_t    phv2mem_cmd;
            cap_ptd_decoders_cmd_mem2mem_t    mem2mem_cmd;

            pkt2mem_cmd.all(command);
            if (pkt2mem_cmd.cmdtype() == 4) {  //pkt2mem 
                pkt2mem_cmd.all(command);
                pkt2mem_cmd.show();
            }
            else if (pkt2mem_cmd.cmdtype() ==5) {  //skip
                skip_cmd.all(command);
                skip_cmd.show();
            }
            else if (pkt2mem_cmd.cmdtype() == 3) {  //phv2mem
                phv2mem_cmd.all(command);
                phv2mem_cmd.show();
            }
            else if (pkt2mem_cmd.cmdtype() == 6) {  //mem2mem
                mem2mem_cmd.all(command);
                mem2mem_cmd.show();
            }
            else if (pkt2mem_cmd.cmdtype() == 0) {  //nop
                PLOG_MSG("nop" << endl)
            }
        }
    }

    if (eop == 1)
        dma_ptr = 1024; //>768, so will not parse command in next line.

}

void display_prd_rdata_mem_entry(unsigned addr, bool wr_mem_sel) {
    cap_pr_csr_t & pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, 0, 0);

    if (wr_mem_sel == true) {
        pr_csr.prd.cfg_debug_ctrl.dbg_mem_sel(1);
        pr_csr.prd.cfg_debug_ctrl.write();
    }

    pr_csr.prd.cfg_dhs_dbg_mem.addr(addr);
    pr_csr.prd.cfg_dhs_dbg_mem.write();
    pr_csr.prd.dhs_dbg_mem.entry.read();
    PLOG_MSG("rdata_mem[" << dec << addr << "]: 0x" << hex << pr_csr.prd.dhs_dbg_mem.entry.all() << endl)
}

void display_prd_pkt_mem_entry(unsigned addr, bool wr_mem_sel) {
    cap_pr_csr_t & pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, 0, 0);
    cpp_int   read_data;

    if (wr_mem_sel == true) {
        pr_csr.prd.cfg_debug_ctrl.dbg_mem_sel(2);
        pr_csr.prd.cfg_debug_ctrl.write();
    }

    pr_csr.prd.cfg_dhs_dbg_mem.addr(addr);
    pr_csr.prd.cfg_dhs_dbg_mem.write();
    pr_csr.prd.dhs_dbg_mem.entry.read();
    read_data = pr_csr.prd.dhs_dbg_mem.entry.all();
    int pbus_size = cpp_int_helper::s_get_slc(read_data, 6, 512).convert_to<int>();
    int pbus_err  = cpp_int_helper::s_get_slc(read_data, 1, 518).convert_to<int>();
    int pbus_eop  = cpp_int_helper::s_get_slc(read_data, 1, 519).convert_to<int>();
    int pbus_sop  = cpp_int_helper::s_get_slc(read_data, 1, 520).convert_to<int>();

    PLOG_MSG("pkt_mem[" << dec << addr << "]: sop = " << pbus_sop << ", eop = " << pbus_eop << ", err = " << pbus_err << ", size = " << pbus_size << endl)
    PLOG_MSG("data = 0x" << cpp_int_helper::s_get_slc(read_data, 512, 0).convert_to<pu_cpp_int<512> >() << endl)

}

void display_prd_lat_mem_entry(unsigned addr, bool wr_mem_sel) {
    cap_pr_csr_t & pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, 0, 0);
    cpp_int  read_data;

    if (wr_mem_sel == true) {
        pr_csr.prd.cfg_debug_ctrl.dbg_mem_sel(3);
        pr_csr.prd.cfg_debug_ctrl.write();
    }

    pr_csr.prd.cfg_dhs_dbg_mem.addr(addr);
    pr_csr.prd.cfg_dhs_dbg_mem.write();
    pr_csr.prd.dhs_dbg_mem.entry.read();
    
    read_data = pr_csr.prd.dhs_dbg_mem.entry.all();
    pu_cpp_int<128> phv_data = cpp_int_helper::s_get_slc(read_data, 128, 128).convert_to<pu_cpp_int<128> >();
    int  cmdtype = cpp_int_helper::s_get_slc(read_data, 3, 0).convert_to<int>();
    if (cmdtype == 6) {
        int  cmdeop  = cpp_int_helper::s_get_slc(read_data, 1, 3).convert_to<int>();
        int  pkteop  = cpp_int_helper::s_get_slc(read_data, 1, 4).convert_to<int>();
        int  start_offset = cpp_int_helper::s_get_slc(read_data, 6, 5).convert_to<int>();
        int  psize   = cpp_int_helper::s_get_slc(read_data, 14, 11).convert_to<int>();
        int  rd_id   = cpp_int_helper::s_get_slc(read_data, 7, 25).convert_to<int>();
        int  wr_cache= cpp_int_helper::s_get_slc(read_data, 1, 32).convert_to<int>();
        pu_cpp_int<64> wr_axi_addr = cpp_int_helper::s_get_slc(read_data, 64, 33).convert_to<int>();
        int  wr_fence_fence = cpp_int_helper::s_get_slc(read_data, 1, 97).convert_to<int>();
        int  wr_data_fence  = cpp_int_helper::s_get_slc(read_data, 1, 98).convert_to<int>();
        int  wr_round       = cpp_int_helper::s_get_slc(read_data, 1, 99).convert_to<int>();
        int  rd_sz_64byte   = cpp_int_helper::s_get_slc(read_data, 1, 100).convert_to<int>();
        PLOG_MSG("lat_mem[" << addr << "]: m2m.wr" << ", cmdeop = " << cmdeop << ", pkteop = " << pkteop << ", start_offset = 0x" << hex << start_offset << ", psize = 0x" << psize << ", rd_id = " << dec << rd_id << ", wr_cache = " << wr_cache << ", wr_axi_addr = 0x" << hex << wr_axi_addr << ", wr_fence_fence = " << wr_fence_fence << ", wr_data_fence = " << wr_data_fence << ", wr_round = " << wr_round << ", rd_sz_64byte = " << rd_sz_64byte << endl)
        int phv_size = cpp_int_helper::s_get_slc(read_data, 14, 4).convert_to<int>();
        wr_cache = cpp_int_helper::s_get_slc(read_data, 1, 18).convert_to<int>();
        wr_axi_addr = cpp_int_helper::s_get_slc(read_data, 64, 19).convert_to<int>();
        wr_fence_fence = cpp_int_helper::s_get_slc(read_data, 1, 83).convert_to<int>();
        wr_data_fence  = cpp_int_helper::s_get_slc(read_data, 1, 84).convert_to<int>();
        wr_round       = cpp_int_helper::s_get_slc(read_data, 1, 85).convert_to<int>();
        int barrier    = cpp_int_helper::s_get_slc(read_data, 1, 86).convert_to<int>();
        int pcie_msg   = cpp_int_helper::s_get_slc(read_data, 1, 87).convert_to<int>();
        PLOG_MSG("or:lat_mem[" << addr << "]: m2m.phv2mem" << ", cmdeop = " << cmdeop << ", phv_size = 0x" << hex << phv_size << ", wr_cache = " << wr_cache << ", wr_axi_addr = 0x" << wr_axi_addr << ", wr_fence_fence = " << wr_fence_fence << ", wr_data_fence = " << wr_data_fence << ", wr_round = " << wr_round << ", barrier = " << barrier << ", pcie_msg = " << pcie_msg << endl)
        PLOG_MSG("phv_data = 0x" << hex << phv_data << endl)
    }
    else if (cmdtype == 2) {
        int cmdeop = cpp_int_helper::s_get_slc(read_data, 1, 1).convert_to<int>();
        int pkteop = cpp_int_helper::s_get_slc(read_data, 1, 2).convert_to<int>();
        int phv_size = cpp_int_helper::s_get_slc(read_data, 14, 3).convert_to<int>();
        PLOG_MSG("lat_mem[" << addr << "]: phv2pkt" << ", , cmdeop = " << cmdeop << ", pkteop = " << pkteop << ", phv_size = 0x" << phv_size << endl)
        PLOG_MSG("phv_data = 0x" << hex << phv_data << endl)
    }
    else {
        PLOG_MSG("lat_mem[" << addr << "]: 0x" << cpp_int_helper::s_get_slc(read_data, 128, 0).convert_to<pu_cpp_int<128> >() << endl)
        PLOG_MSG("phv_data = 0x" << hex << phv_data << endl)
    }
}

void display_prd_phv_mem() {
    int  dma_ptr;
    for (int i = 0; i < 32; i++) {
        if (i == 0)
            display_prd_phv_mem_entry(i, dma_ptr, true);
        else
            display_prd_phv_mem_entry(i, dma_ptr, false);
    }
}

void display_prd_rdata_mem() {
    for (int i = 0; i < 256; i++) {
        if (i == 0)
            display_prd_rdata_mem_entry(i, true);
        else
            display_prd_rdata_mem_entry(i, false);
    }

}

void display_prd_pkt_mem() {
    for (int i = 0; i < 1024; i++) {
        if (i == 0)
            display_prd_pkt_mem_entry(i, true);
        else
            display_prd_pkt_mem_entry(i, false);
    }

}

void display_prd_lat_mem() {  
    for (int i = 0; i < 128; i++) {
        if (i == 0)
            display_prd_lat_mem_entry(i, true);
        else
            display_prd_lat_mem_entry(i, false);
    }
}

void clear_prd_phv_mem_entry(unsigned addr, bool wr_mem_sel) {
    cap_pr_csr_t & pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, 0, 0);

    if (wr_mem_sel == true) {
        pr_csr.prd.cfg_debug_ctrl.dbg_mem_sel(0);
        pr_csr.prd.cfg_debug_ctrl.write();
    }

    pr_csr.prd.cfg_dhs_dbg_mem.addr(addr);
    pr_csr.prd.cfg_dhs_dbg_mem.write();
    pr_csr.prd.dhs_dbg_mem.entry.all(0);
    pr_csr.prd.dhs_dbg_mem.entry.write();
}

void clear_prd_phv_mem() {
    for (int i = 0; i < 32; i++) {
        if (i == 0)
            clear_prd_phv_mem_entry(i, true);
        else
            clear_prd_phv_mem_entry(i, false);
    }
}

void clear_prd_rdata_mem_entry(unsigned addr, bool wr_mem_sel) {
    cap_pr_csr_t & pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, 0, 0);

    if (wr_mem_sel == true) {
        pr_csr.prd.cfg_debug_ctrl.dbg_mem_sel(1);
        pr_csr.prd.cfg_debug_ctrl.write();
    }

    pr_csr.prd.cfg_dhs_dbg_mem.addr(addr);
    pr_csr.prd.cfg_dhs_dbg_mem.write();
    pr_csr.prd.dhs_dbg_mem.entry.all(0);
    pr_csr.prd.dhs_dbg_mem.entry.write();
}

void clear_prd_rdata_mem() {
    for (int i = 0; i < 256; i++) {
        if (i == 0)
            clear_prd_rdata_mem_entry(i, true);
        else
            clear_prd_rdata_mem_entry(i, false);
    }
}

void clear_prd_pkt_mem_entry(unsigned addr, bool wr_mem_sel) {
    cap_pr_csr_t & pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, 0, 0);

    if (wr_mem_sel == true) {
        pr_csr.prd.cfg_debug_ctrl.dbg_mem_sel(2);
        pr_csr.prd.cfg_debug_ctrl.write();
    }

    pr_csr.prd.cfg_dhs_dbg_mem.addr(addr);
    pr_csr.prd.cfg_dhs_dbg_mem.write();
    pr_csr.prd.dhs_dbg_mem.entry.all(0);
    pr_csr.prd.dhs_dbg_mem.entry.write();
}

void clear_prd_pkt_mem() {
    for (int i = 0; i < 1024; i++) {
        if (i == 0)
            clear_prd_pkt_mem_entry(i, true);
        else
            clear_prd_pkt_mem_entry(i, false);
    }
}


void clear_prd_lat_mem_entry(unsigned addr, bool wr_mem_sel) {  
    cap_pr_csr_t & pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, 0, 0);
    cpp_int  read_data;

    if (wr_mem_sel == true) {
        pr_csr.prd.cfg_debug_ctrl.dbg_mem_sel(3);
        pr_csr.prd.cfg_debug_ctrl.write();
    }

    pr_csr.prd.cfg_dhs_dbg_mem.addr(addr);
    pr_csr.prd.cfg_dhs_dbg_mem.write();
    pr_csr.prd.dhs_dbg_mem.entry.all(0);
    pr_csr.prd.dhs_dbg_mem.entry.write();

}

void clear_prd_lat_mem() {
    for (int i = 0; i < 128; i++) {
        if (i == 0)
            clear_prd_lat_mem_entry(i, true);
        else
            clear_prd_lat_mem_entry(i, false);
    }
}

void cap_prd_dump_cntr() {
    cap_pr_csr_t & pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, 0, 0);

    pr_csr.prd.CNT_ma.read();
    pr_csr.prd.CNT_ps_resub_pkt.read();
    pr_csr.prd.CNT_ps_resub_phv.read();
    pr_csr.prd.CNT_ps_pkt.read();
    pr_csr.prd.CNT_phv.read();
    pr_csr.prd.CNT_pkt.read();
    pr_csr.prd.CNT_wr.read();
    pr_csr.prd.CNT_axi_wr.read();
    pr_csr.prd.CNT_axi_rd.read();  
    
    PRD_PRN_MSG(setw(1) << "========================================================= RXDMA PRD CNTRS ==============================================================\n");
    PRD_PRN_MSG(setw(32) << "cnt_ma_sop" << setw(10) << pr_csr.prd.CNT_ma.sop().convert_to<unsigned int>() << "\n");
    PRD_PRN_MSG(setw(32) << "cnt_ps_resub_pkt_sop" << setw(10) << pr_csr.prd.CNT_ps_resub_pkt.sop().convert_to<unsigned int>() << "\n");
    PRD_PRN_MSG(setw(32) << "cnt_ps_resub_phv_sop" << setw(10) << pr_csr.prd.CNT_ps_resub_phv.sop().convert_to<unsigned int>() << "\n");
    PRD_PRN_MSG(setw(32) << "cnt_ps_pkt_sop" << setw(10) << pr_csr.prd.CNT_ps_pkt.sop().convert_to<unsigned int>() << "\n");
    PRD_PRN_MSG(setw(32) << "cnt_phv_no_data" << setw(10) << pr_csr.prd.CNT_phv.no_data().convert_to<unsigned int>() << "\n");
    PRD_PRN_MSG(setw(32) << "cnt_phv_drop" << setw(10) << pr_csr.prd.CNT_phv.drop().convert_to<unsigned int>() << "\n");
    PRD_PRN_MSG(setw(32) << "cnt_phv_err" << setw(10) << pr_csr.prd.CNT_phv.err().convert_to<unsigned int>() << "\n");
    PRD_PRN_MSG(setw(32) << "cnt_phv_recirc" << setw(10) << pr_csr.prd.CNT_phv.recirc().convert_to<unsigned int>() << "\n");
    PRD_PRN_MSG(setw(32) << "cnt_pkt_drop" << setw(10) << pr_csr.prd.CNT_pkt.drop().convert_to<unsigned int>() << "\n");
    PRD_PRN_MSG(setw(32) << "cnt_wr_pkt_drop" << setw(10) << pr_csr.prd.CNT_wr.pkt_drop().convert_to<unsigned int>() << "\n");
    PRD_PRN_MSG(setw(32) << "cnt_wr_wr_lat_drop" << setw(10) << pr_csr.prd.CNT_wr.wr_lat_drop().convert_to<unsigned int>() << "\n");
    PRD_PRN_MSG(setw(32) << "cnt_wr_rd_lat_drop" << setw(10) << pr_csr.prd.CNT_wr.rd_lat_drop().convert_to<unsigned int>() << "\n");
    PRD_PRN_MSG(setw(32) << "cnt_axi_wr_req_vld" << setw(10) << pr_csr.prd.CNT_axi_wr.req_vld().convert_to<unsigned int>() << "\n");
    PRD_PRN_MSG(setw(32) << "cnt_axi_wr_rsp_vld" << setw(10) << pr_csr.prd.CNT_axi_wr.rsp_vld().convert_to<unsigned int>() << "\n");
    PRD_PRN_MSG(setw(32) << "cnt_axi_rd_req_vld" << setw(10) << pr_csr.prd.CNT_axi_rd.req_vld().convert_to<unsigned int>() << "\n");
    PRD_PRN_MSG(setw(32) << "cnt_axi_rd_rsp_vld" << setw(10) << pr_csr.prd.CNT_axi_rd.rsp_vld().convert_to<unsigned int>() << "\n");

}

void cap_prd_clr_cntr() {
    cap_pr_csr_t & pr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pr_csr_t, 0, 0);

    pr_csr.prd.CNT_ma.all(0);
    pr_csr.prd.CNT_ps_resub_pkt.all(0);
    pr_csr.prd.CNT_ps_resub_phv.all(0);
    pr_csr.prd.CNT_ps_pkt.all(0);
    pr_csr.prd.CNT_phv.all(0);
    pr_csr.prd.CNT_pkt.all(0);
    pr_csr.prd.CNT_wr.all(0);
    pr_csr.prd.CNT_axi_wr.all(0);
    pr_csr.prd.CNT_axi_rd.all(0);  

    pr_csr.prd.CNT_ma.write();
    pr_csr.prd.CNT_ps_resub_pkt.write();
    pr_csr.prd.CNT_ps_resub_phv.write();
    pr_csr.prd.CNT_ps_pkt.write();
    pr_csr.prd.CNT_phv.write();
    pr_csr.prd.CNT_pkt.write();
    pr_csr.prd.CNT_wr.write();
    pr_csr.prd.CNT_axi_wr.write();
    pr_csr.prd.CNT_axi_rd.write();  
}

void prd_print_msg(string msg) {
#ifdef _CSV_INCLUDED_
   vpi_printf((PLI_BYTE8*) "%s", msg.c_str());
#else
   std::cout << msg;
#endif
}

