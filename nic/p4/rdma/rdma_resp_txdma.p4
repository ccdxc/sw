/***********************************************************************/
/* rdma_resp_txdma.p4 */
/***********************************************************************/

#include "../common-p4+/common_txdma_dummy.p4"

#define tx_table_s0_t0 rdma_stage0_table
#define tx_table_s0_t0_action  rdma_stage0_table_action
#define tx_table_s0_t0_action1 rdma_stage0_recirc_action

/**** table declarations ****/

#define tx_table_s0_t1 resp_tx_s0_t1
#define tx_table_s0_t2 resp_tx_s0_t2
#define tx_table_s0_t3 resp_tx_s0_t3

#define tx_table_s1_t0 resp_tx_s1_t0
#define tx_table_s1_t1 resp_tx_s1_t1
#define tx_table_s1_t2 resp_tx_s1_t2
#define tx_table_s1_t3 resp_tx_s1_t3

#define tx_table_s2_t0 resp_tx_s2_t0
#define tx_table_s2_t1 resp_tx_s2_t1
#define tx_table_s2_t2 resp_tx_s2_t2
#define tx_table_s2_t3 resp_tx_s2_t3

#define tx_table_s3_t0 resp_tx_s3_t0
#define tx_table_s3_t1 resp_tx_s3_t1
#define tx_table_s3_t2 resp_tx_s3_t2
#define tx_table_s3_t3 resp_tx_s3_t3

#define tx_table_s4_t0 resp_tx_s4_t0
#define tx_table_s4_t1 resp_tx_s4_t1
#define tx_table_s4_t2 resp_tx_s4_t2
#define tx_table_s4_t3 resp_tx_s4_t3

#define tx_table_s5_t0 resp_tx_s5_t0
#define tx_table_s5_t1 resp_tx_s5_t1
#define tx_table_s5_t2 resp_tx_s5_t2
#define tx_table_s5_t3 resp_tx_s5_t3

#define tx_table_s6_t0 resp_tx_s6_t0
#define tx_table_s6_t1 resp_tx_s6_t1
#define tx_table_s6_t2 resp_tx_s6_t2
#define tx_table_s6_t3 resp_tx_s6_t3

#define tx_table_s7_t0 resp_tx_s7_t0
#define tx_table_s7_t1 resp_tx_s7_t1
#define tx_table_s7_t2 resp_tx_s7_t2
#define tx_table_s7_t3 resp_tx_s7_t3


#define tx_table_s0_t0_action resp_tx_rqcb_process

#define tx_table_s1_t0_action   resp_tx_rqcb2_process
#define tx_table_s1_t0_action1  resp_tx_ack_process
#define tx_table_s1_t0_action2  resp_tx_rsq_backtrack_adjust_process
#define tx_table_s1_t0_action3  resp_tx_rqcb2_bt_process

#define tx_table_s2_t0_action   resp_tx_rsqwqe_process
#define tx_table_s2_t0_action1  resp_tx_rsqwqe_bt_process

#define tx_table_s3_t0_action resp_tx_rsqrkey_process

#define tx_table_s3_t1_action   resp_tx_rqcb0_bt_write_back_process

#define tx_table_s4_t0_action   resp_tx_rsqptseg_process
#define tx_table_s4_t0_action1  resp_tx_dcqcn_rate_process
#define tx_table_s4_t0_action2  resp_tx_dcqcn_timer_process

#define tx_table_s4_t1_action   resp_tx_dcqcn_enforce_process

#define tx_table_s5_t1_action   resp_tx_rqcb0_write_back_process

#include "../common-p4+/common_txdma.p4"
#include "./rdma_txdma_headers.p4"

/**** Macros ****/

#define GENERATE_GLOBAL_K \
    modify_field(phv_global_common_scr.lif, phv_global_common.lif);\
    modify_field(phv_global_common_scr.qid, phv_global_common.qid);\
    modify_field(phv_global_common_scr.qtype, phv_global_common.qtype);\
    modify_field(phv_global_common_scr.cb_addr, phv_global_common.cb_addr);\
    modify_field(phv_global_common_scr.pt_base_addr_page_id, phv_global_common.pt_base_addr_page_id);\
    modify_field(phv_global_common_scr.log_num_pt_entries, phv_global_common.log_num_pt_entries);\
    modify_field(phv_global_common_scr.pad, phv_global_common.pad);\
    modify_field(phv_global_common_scr.error_disable_qp, phv_global_common.error_disable_qp);\

/**** header definitions ****/

header_type resp_tx_to_stage_rqcb1_wb_info_t {
    fields {
        new_c_index                      :   16;
        ack_nak_process                  :    1;
        rsvd                             :  111;
    }
}

header_type resp_tx_rkey_to_ptseg_info_t {
    fields {
        pt_seg_offset                    :   32;
        pt_seg_bytes                     :   32;
        dma_cmd_start_index              :    8;
        log_page_size                    :    5;
        tbl_id                           :    2;
        pad                              :   81;
    }
}

header_type resp_tx_rqcb_to_ack_info_t {
    fields {
        header_template_addr             :   32;
        pad                              :  128;
    }
}

header_type resp_tx_to_stage_rsq_rkey_info_t {
    fields {
        dcqcn_cb_addr                    :   34;
        congestion_mgmt_enable           :    1;
        rsvd                             :   93;
    }
}

header_type phv_global_common_t {
    fields {
        lif                              :   11;
        qid                              :   24;
        qtype                            :    3;
        cb_addr                          :   25;
        pt_base_addr_page_id             :   22;
        log_num_pt_entries               :    5;
        pad                              :   22;
        error_disable_qp                 :    1;
    }
}

header_type resp_tx_rqcb_to_rqcb2_info_t {
    fields {
        rsqwqe_addr                      :   64;
        curr_read_rsp_psn                :   24;
        log_pmtu                         :    5;
        serv_type                        :    3;
        header_template_addr             :   32;
        header_template_size             :    8;
        read_rsp_in_progress             :    1;
        pad                              :   23;
    }
}

header_type resp_tx_to_stage_wqe_info_t {
    fields {
        dcqcn_cb_addr                    :   34;
        rsvd                             :   94;
    }
}

header_type resp_tx_rsq_backtrack_adjust_info_t {
    fields {
        adjust_rsq_c_index               :   16;
        rsq_bt_p_index                   :   16;
        pad                              :  128;
    }
}

header_type resp_tx_s1_info_t {
    fields {
        rsvd                             :  128;
    }
}

header_type resp_tx_rqcb0_write_back_info_t {
    fields {
        curr_read_rsp_psn                :   24;
        read_rsp_in_progress             :    1;
        rate_enforce_failed              :    1;
        pad                              :  134;
    }
}

header_type resp_tx_to_stage_dcqcn_info_t {
    fields {
        dcqcn_cb_addr                    :   34;
        congestion_mgmt_enable           :    1;
        packet_len                       :   14;
        new_timer_cindex                 :   16;
        rsvd                             :   63;
    }
}

header_type resp_tx_to_stage_bt_info_t {
    fields {
        log_rsq_size                     :    5;
        log_pmtu                         :    5;
        rsq_base_addr                    :   32; 
        bt_cindex                        :   16;
        end_index                        :   16;
        search_index                     :   16;
        curr_read_rsp_psn                :   24;
        read_rsp_in_progress             :    1;
        bt_in_progress                   :    1; 
        rsvd                             :   12;
    }
}

header_type resp_tx_bt_info_t {
    fields {
        read_or_atomic                   :    1;
        rsvd                             :    7;
        psn                              :   24;
        va                               :   64;
        r_key                            :   32;
        len                              :   32;
    }
}

header_type resp_tx_rqcb0_bt_write_back_info_t {
    fields {
        curr_read_rsp_psn                :   24;
        read_rsp_in_progress             :    1;
        bt_in_progress                   :    1;
        update_read_rsp_in_progress      :    1; 
        update_rsq_cindex                :    1;
        update_bt_cindex                 :    1;
        update_bt_in_progress            :    1;
        update_bt_rsq_cindex             :    1;
        rsvd                             :    1;
        rsq_cindex                       :   16; 
        bt_cindex                        :   16;
        bt_rsq_cindex                    :   16;
        pad                              :   80;
    }
}

header_type resp_tx_s6_info_t {
    fields {
        rsvd                             :  128;
    }
}

header_type resp_tx_rqcb_to_rqcb1_info_t {
    fields {
        rsqwqe_addr                      :   64;
        log_pmtu                         :    5;
        serv_type                        :    3;
        timer_event_process              :    1;
        curr_read_rsp_psn                :   24;
        read_rsp_in_progress             :    1;
        pad                              :   62;
    }
}

header_type resp_tx_rsqwqe_to_rkey_info_t {
    fields {
        transfer_va                      :   64;
        header_template_addr             :   32;
        curr_read_rsp_psn                :   24;
        log_pmtu                         :    5;
        key_id                           :    1;
        send_aeth                        :    1;
        last_or_only                     :    1;
        transfer_bytes                   :   12;
        header_template_size             :    8;
        pad                              :   12;
    }
}

header_type resp_tx_rqcb_to_cnp_info_t {
    fields {
        new_c_index                      :   16;
        pad                              :  144;
    }
}

header_type resp_tx_rqcb2_to_rsqwqe_info_t {
    fields {
        curr_read_rsp_psn                :   24;
        log_pmtu                         :    5;
        serv_type                        :    3;
        header_template_addr             :   32;
        header_template_size             :    8;
        read_rsp_in_progress             :    1;
        pad                              :   87;
    }
}

header_type resp_tx_s0_info_t {
    fields {
        rsvd                             :  128;
    }
}

header_type resp_tx_s7_info_t {
    fields {
        rsvd                             :  128;
    }
}


/**** header unions and scratch ****/

@pragma pa_header_union ingress app_header rdma_recirc

/* metadata common_global_phv_t common_phv; */
metadata roce_recirc_header_t rdma_recirc;

@pragma scratch_metadata
/* metadata common_global_phv_t common_global_scratch; */
metadata roce_recirc_header_t rdma_recirc_scr;

/**** global header unions ****/

@pragma pa_header_union ingress common_global
metadata phv_global_common_t phv_global_common;
@pragma scratch_metadata
metadata phv_global_common_t phv_global_common_scr;

/**** to stage header unions ****/

@pragma pa_header_union ingress to_stage_1
metadata resp_tx_to_stage_dcqcn_info_t to_s1_dcqcn_info;
@pragma scratch_metadata
metadata resp_tx_to_stage_dcqcn_info_t to_s1_dcqcn_info_scr;

@pragma pa_header_union ingress to_stage_1
metadata resp_tx_to_stage_bt_info_t to_s1_bt_info;
@pragma scratch_metadata
metadata resp_tx_to_stage_bt_info_t to_s1_bt_info_scr;

@pragma pa_header_union ingress to_stage_2
metadata resp_tx_to_stage_dcqcn_info_t to_s2_dcqcn_info;
@pragma scratch_metadata
metadata resp_tx_to_stage_dcqcn_info_t to_s2_dcqcn_info_scr;

@pragma pa_header_union ingress to_stage_2
metadata resp_tx_to_stage_bt_info_t to_s2_bt_info;
@pragma scratch_metadata
metadata resp_tx_to_stage_bt_info_t to_s2_bt_info_scr;

@pragma pa_header_union ingress to_stage_3
metadata resp_tx_to_stage_dcqcn_info_t to_s3_dcqcn_info;
@pragma scratch_metadata
metadata resp_tx_to_stage_dcqcn_info_t to_s3_dcqcn_info_scr;

@pragma pa_header_union ingress to_stage_3
metadata resp_tx_to_stage_bt_info_t to_s3_bt_info;
@pragma scratch_metadata
metadata resp_tx_to_stage_bt_info_t to_s3_bt_info_scr;

@pragma pa_header_union ingress to_stage_4
metadata resp_tx_to_stage_dcqcn_info_t to_s4_dcqcn_info;
@pragma scratch_metadata
metadata resp_tx_to_stage_dcqcn_info_t to_s4_dcqcn_info_scr;

@pragma pa_header_union ingress to_stage_5
metadata resp_tx_to_stage_rqcb1_wb_info_t to_s5_rqcb1_wb_info;
@pragma scratch_metadata
metadata resp_tx_to_stage_rqcb1_wb_info_t to_s5_rqcb1_wb_info_scr;

/**** stage to stage header unions ****/

@pragma pa_header_union ingress common_t0_s2s
metadata resp_tx_rqcb_to_rqcb2_info_t t0_s2s_rqcb_to_rqcb2_info;
@pragma scratch_metadata
metadata resp_tx_rqcb_to_rqcb2_info_t t0_s2s_rqcb_to_rqcb2_info_scr;

@pragma pa_header_union ingress common_t0_s2s
metadata resp_tx_rqcb_to_ack_info_t t0_s2s_rqcb_to_ack_info;
@pragma scratch_metadata
metadata resp_tx_rqcb_to_ack_info_t t0_s2s_rqcb_to_ack_info_scr;

@pragma pa_header_union ingress common_t0_s2s
metadata resp_tx_rsq_backtrack_adjust_info_t t0_s2s_rsq_backtrack_adjust_info;
@pragma scratch_metadata
metadata resp_tx_rsq_backtrack_adjust_info_t t0_s2s_rsq_backtrack_adjust_info_scr;

@pragma pa_header_union ingress common_t0_s2s
metadata resp_tx_rqcb2_to_rsqwqe_info_t t0_s2s_rqcb2_to_rsqwqe_info;
@pragma scratch_metadata
metadata resp_tx_rqcb2_to_rsqwqe_info_t t0_s2s_rqcb2_to_rsqwqe_info_scr;

@pragma pa_header_union ingress common_t0_s2s
metadata resp_tx_rsqwqe_to_rkey_info_t t0_s2s_rsqwqe_to_rkey_info;
@pragma scratch_metadata
metadata resp_tx_rsqwqe_to_rkey_info_t t0_s2s_rsqwqe_to_rkey_info_scr;

@pragma pa_header_union ingress common_t0_s2s
metadata resp_tx_rkey_to_ptseg_info_t t0_s2s_rkey_to_ptseg_info;
@pragma scratch_metadata
metadata resp_tx_rkey_to_ptseg_info_t t0_s2s_rkey_to_ptseg_info_scr;

@pragma pa_header_union ingress common_t1_s2s
metadata resp_tx_rqcb0_write_back_info_t t1_s2s_rqcb0_write_back_info;
@pragma scratch_metadata
metadata resp_tx_rqcb0_write_back_info_t t1_s2s_rqcb0_write_back_info_scr;

@pragma pa_header_union ingress common_t1_s2s
metadata resp_tx_rqcb0_bt_write_back_info_t t1_s2s_rqcb0_bt_write_back_info;
@pragma scratch_metadata
metadata resp_tx_rqcb0_bt_write_back_info_t t1_s2s_rqcb0_bt_write_back_info_scr;

@pragma pa_header_union ingress common_t0_s2s
metadata resp_tx_bt_info_t t0_s2s_bt_info;
@pragma scratch_metadata
metadata resp_tx_bt_info_t t0_s2s_bt_info_scr;


/*
 * Stage 0 table 0 action
 */
action rdma_stage0_table_action() {

    // k + i for stage 0
}

/*
 * Stage 0 table 0 recirc action
 */
action rdma_stage0_recirc_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);

    // recirc header bits
    modify_field(rdma_recirc_scr.recirc_reason, rdma_recirc.recirc_reason);
}

action resp_tx_ack_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_rqcb_to_ack_info_scr.header_template_addr, t0_s2s_rqcb_to_ack_info.header_template_addr);
    modify_field(t0_s2s_rqcb_to_ack_info_scr.pad, t0_s2s_rqcb_to_ack_info.pad);

}
#if 0
action resp_tx_dcqcn_enforce_process_s2 () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_dcqcn_info_scr.dcqcn_cb_addr, to_s2_dcqcn_info.dcqcn_cb_addr);
    modify_field(to_s2_dcqcn_info_scr.congestion_mgmt_enable, to_s2_dcqcn_info.congestion_mgmt_enable);
    modify_field(to_s2_dcqcn_info_scr.packet_len, to_s2_dcqcn_info.packet_len);
    modify_field(to_s2_dcqcn_info_scr.new_timer_cindex, to_s2_dcqcn_info.new_timer_cindex);
    modify_field(to_s2_dcqcn_info_scr.rsvd, to_s2_dcqcn_info.rsvd);

    // stage to stage
    modify_field(t1_s2s_rqcb0_write_back_info_scr.curr_read_rsp_psn, t1_s2s_rqcb0_write_back_info.curr_read_rsp_psn);
    modify_field(t1_s2s_rqcb0_write_back_info_scr.read_rsp_in_progress, t1_s2s_rqcb0_write_back_info.read_rsp_in_progress);
    modify_field(t1_s2s_rqcb0_write_back_info_scr.rate_enforce_failed, t1_s2s_rqcb0_write_back_info.rate_enforce_failed);
    modify_field(t1_s2s_rqcb0_write_back_info_scr.pad, t1_s2s_rqcb0_write_back_info.pad);

}
#endif
action resp_tx_dcqcn_enforce_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_dcqcn_info_scr.dcqcn_cb_addr, to_s4_dcqcn_info.dcqcn_cb_addr);
    modify_field(to_s4_dcqcn_info_scr.congestion_mgmt_enable, to_s4_dcqcn_info.congestion_mgmt_enable);
    modify_field(to_s4_dcqcn_info_scr.packet_len, to_s4_dcqcn_info.packet_len);
    modify_field(to_s4_dcqcn_info_scr.new_timer_cindex, to_s4_dcqcn_info.new_timer_cindex);
    modify_field(to_s4_dcqcn_info_scr.rsvd, to_s4_dcqcn_info.rsvd);

    // stage to stage
    modify_field(t1_s2s_rqcb0_write_back_info_scr.curr_read_rsp_psn, t1_s2s_rqcb0_write_back_info.curr_read_rsp_psn);
    modify_field(t1_s2s_rqcb0_write_back_info_scr.read_rsp_in_progress, t1_s2s_rqcb0_write_back_info.read_rsp_in_progress);
    modify_field(t1_s2s_rqcb0_write_back_info_scr.rate_enforce_failed, t1_s2s_rqcb0_write_back_info.rate_enforce_failed);
    modify_field(t1_s2s_rqcb0_write_back_info_scr.pad, t1_s2s_rqcb0_write_back_info.pad);

}
action resp_tx_dcqcn_rate_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_dcqcn_info_scr.dcqcn_cb_addr, to_s4_dcqcn_info.dcqcn_cb_addr);
    modify_field(to_s4_dcqcn_info_scr.congestion_mgmt_enable, to_s4_dcqcn_info.congestion_mgmt_enable);
    modify_field(to_s4_dcqcn_info_scr.packet_len, to_s4_dcqcn_info.packet_len);
    modify_field(to_s4_dcqcn_info_scr.new_timer_cindex, to_s4_dcqcn_info.new_timer_cindex);
    modify_field(to_s4_dcqcn_info_scr.rsvd, to_s4_dcqcn_info.rsvd);

    // stage to stage
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.rsqwqe_addr, t0_s2s_rqcb_to_rqcb2_info.rsqwqe_addr);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.curr_read_rsp_psn, t0_s2s_rqcb_to_rqcb2_info.curr_read_rsp_psn);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.log_pmtu, t0_s2s_rqcb_to_rqcb2_info.log_pmtu);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.serv_type, t0_s2s_rqcb_to_rqcb2_info.serv_type);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.header_template_addr, t0_s2s_rqcb_to_rqcb2_info.header_template_addr);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.header_template_size, t0_s2s_rqcb_to_rqcb2_info.header_template_size);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.read_rsp_in_progress, t0_s2s_rqcb_to_rqcb2_info.read_rsp_in_progress);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.pad, t0_s2s_rqcb_to_rqcb2_info.pad);

}
action resp_tx_dcqcn_timer_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_dcqcn_info_scr.dcqcn_cb_addr, to_s4_dcqcn_info.dcqcn_cb_addr);
    modify_field(to_s4_dcqcn_info_scr.congestion_mgmt_enable, to_s4_dcqcn_info.congestion_mgmt_enable);
    modify_field(to_s4_dcqcn_info_scr.packet_len, to_s4_dcqcn_info.packet_len);
    modify_field(to_s4_dcqcn_info_scr.new_timer_cindex, to_s4_dcqcn_info.new_timer_cindex);
    modify_field(to_s4_dcqcn_info_scr.rsvd, to_s4_dcqcn_info.rsvd);

    // stage to stage
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.rsqwqe_addr, t0_s2s_rqcb_to_rqcb2_info.rsqwqe_addr);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.curr_read_rsp_psn, t0_s2s_rqcb_to_rqcb2_info.curr_read_rsp_psn);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.log_pmtu, t0_s2s_rqcb_to_rqcb2_info.log_pmtu);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.serv_type, t0_s2s_rqcb_to_rqcb2_info.serv_type);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.header_template_addr, t0_s2s_rqcb_to_rqcb2_info.header_template_addr);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.header_template_size, t0_s2s_rqcb_to_rqcb2_info.header_template_size);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.read_rsp_in_progress, t0_s2s_rqcb_to_rqcb2_info.read_rsp_in_progress);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.pad, t0_s2s_rqcb_to_rqcb2_info.pad);

}
action resp_tx_rqcb0_write_back_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s5_rqcb1_wb_info_scr.new_c_index, to_s5_rqcb1_wb_info.new_c_index);
    modify_field(to_s5_rqcb1_wb_info_scr.ack_nak_process, to_s5_rqcb1_wb_info.ack_nak_process);
    modify_field(to_s5_rqcb1_wb_info_scr.rsvd, to_s5_rqcb1_wb_info.rsvd);

    // stage to stage
    modify_field(t1_s2s_rqcb0_write_back_info_scr.curr_read_rsp_psn, t1_s2s_rqcb0_write_back_info.curr_read_rsp_psn);
    modify_field(t1_s2s_rqcb0_write_back_info_scr.read_rsp_in_progress, t1_s2s_rqcb0_write_back_info.read_rsp_in_progress);
    modify_field(t1_s2s_rqcb0_write_back_info_scr.rate_enforce_failed, t1_s2s_rqcb0_write_back_info.rate_enforce_failed);
    modify_field(t1_s2s_rqcb0_write_back_info_scr.pad, t1_s2s_rqcb0_write_back_info.pad);

}
action resp_tx_rqcb0_bt_write_back_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_bt_info_scr.log_rsq_size, to_s3_bt_info.log_rsq_size);
    modify_field(to_s3_bt_info_scr.log_pmtu, to_s3_bt_info.log_pmtu);
    modify_field(to_s3_bt_info_scr.rsq_base_addr, to_s3_bt_info.rsq_base_addr);
    modify_field(to_s3_bt_info_scr.bt_cindex, to_s3_bt_info.bt_cindex);
    modify_field(to_s3_bt_info_scr.end_index, to_s3_bt_info.end_index);
    modify_field(to_s3_bt_info_scr.search_index, to_s3_bt_info.search_index);
    modify_field(to_s3_bt_info_scr.curr_read_rsp_psn, to_s3_bt_info.curr_read_rsp_psn);
    modify_field(to_s3_bt_info_scr.read_rsp_in_progress, to_s3_bt_info.read_rsp_in_progress);
    modify_field(to_s3_bt_info_scr.bt_in_progress, to_s3_bt_info.bt_in_progress);
    modify_field(to_s3_bt_info_scr.rsvd, to_s3_bt_info.rsvd);
    
    // stage to stage
    modify_field(t1_s2s_rqcb0_bt_write_back_info_scr.curr_read_rsp_psn, t1_s2s_rqcb0_bt_write_back_info.curr_read_rsp_psn);
    modify_field(t1_s2s_rqcb0_bt_write_back_info_scr.read_rsp_in_progress, t1_s2s_rqcb0_bt_write_back_info.read_rsp_in_progress);
    modify_field(t1_s2s_rqcb0_bt_write_back_info_scr.bt_in_progress, t1_s2s_rqcb0_bt_write_back_info.bt_in_progress);
    modify_field(t1_s2s_rqcb0_bt_write_back_info_scr.update_read_rsp_in_progress, t1_s2s_rqcb0_bt_write_back_info.update_read_rsp_in_progress);
    modify_field(t1_s2s_rqcb0_bt_write_back_info_scr.update_rsq_cindex, t1_s2s_rqcb0_bt_write_back_info.update_rsq_cindex);
    modify_field(t1_s2s_rqcb0_bt_write_back_info_scr.update_bt_cindex, t1_s2s_rqcb0_bt_write_back_info.update_bt_cindex);
    modify_field(t1_s2s_rqcb0_bt_write_back_info_scr.update_bt_in_progress, t1_s2s_rqcb0_bt_write_back_info.update_bt_in_progress);
    modify_field(t1_s2s_rqcb0_bt_write_back_info_scr.update_bt_rsq_cindex, t1_s2s_rqcb0_bt_write_back_info.update_bt_rsq_cindex);
    modify_field(t1_s2s_rqcb0_bt_write_back_info_scr.rsvd, t1_s2s_rqcb0_bt_write_back_info.rsvd);
    modify_field(t1_s2s_rqcb0_bt_write_back_info_scr.rsq_cindex, t1_s2s_rqcb0_bt_write_back_info.rsq_cindex);
    modify_field(t1_s2s_rqcb0_bt_write_back_info_scr.bt_cindex, t1_s2s_rqcb0_bt_write_back_info.bt_cindex);
    modify_field(t1_s2s_rqcb0_bt_write_back_info_scr.bt_rsq_cindex, t1_s2s_rqcb0_bt_write_back_info.bt_rsq_cindex);
    modify_field(t1_s2s_rqcb0_bt_write_back_info_scr.pad, t1_s2s_rqcb0_bt_write_back_info.pad);
}
action resp_tx_rqcb2_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.rsqwqe_addr, t0_s2s_rqcb_to_rqcb2_info.rsqwqe_addr);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.curr_read_rsp_psn, t0_s2s_rqcb_to_rqcb2_info.curr_read_rsp_psn);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.log_pmtu, t0_s2s_rqcb_to_rqcb2_info.log_pmtu);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.serv_type, t0_s2s_rqcb_to_rqcb2_info.serv_type);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.header_template_addr, t0_s2s_rqcb_to_rqcb2_info.header_template_addr);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.header_template_size, t0_s2s_rqcb_to_rqcb2_info.header_template_size);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.read_rsp_in_progress, t0_s2s_rqcb_to_rqcb2_info.read_rsp_in_progress);
    modify_field(t0_s2s_rqcb_to_rqcb2_info_scr.pad, t0_s2s_rqcb_to_rqcb2_info.pad);

}
action resp_tx_rqcb2_bt_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_bt_info_scr.log_rsq_size, to_s1_bt_info.log_rsq_size);
    modify_field(to_s1_bt_info_scr.log_pmtu, to_s1_bt_info.log_pmtu);
    modify_field(to_s1_bt_info_scr.rsq_base_addr, to_s1_bt_info.rsq_base_addr);
    modify_field(to_s1_bt_info_scr.bt_cindex, to_s1_bt_info.bt_cindex);
    modify_field(to_s1_bt_info_scr.end_index, to_s1_bt_info.end_index);
    modify_field(to_s1_bt_info_scr.search_index, to_s1_bt_info.search_index);
    modify_field(to_s1_bt_info_scr.curr_read_rsp_psn, to_s1_bt_info.curr_read_rsp_psn);
    modify_field(to_s1_bt_info_scr.read_rsp_in_progress, to_s1_bt_info.read_rsp_in_progress);
    modify_field(to_s1_bt_info_scr.bt_in_progress, to_s1_bt_info.bt_in_progress);
    modify_field(to_s1_bt_info_scr.rsvd, to_s1_bt_info.rsvd);

    // stage to stage
}
action resp_tx_rqcb_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage

}
action resp_tx_rsq_backtrack_adjust_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_rsq_backtrack_adjust_info_scr.adjust_rsq_c_index, t0_s2s_rsq_backtrack_adjust_info.adjust_rsq_c_index);
    modify_field(t0_s2s_rsq_backtrack_adjust_info_scr.rsq_bt_p_index, t0_s2s_rsq_backtrack_adjust_info.rsq_bt_p_index);
    modify_field(t0_s2s_rsq_backtrack_adjust_info_scr.pad, t0_s2s_rsq_backtrack_adjust_info.pad);

}
action resp_tx_rsqptseg_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_rkey_to_ptseg_info_scr.pt_seg_offset, t0_s2s_rkey_to_ptseg_info.pt_seg_offset);
    modify_field(t0_s2s_rkey_to_ptseg_info_scr.pt_seg_bytes, t0_s2s_rkey_to_ptseg_info.pt_seg_bytes);
    modify_field(t0_s2s_rkey_to_ptseg_info_scr.dma_cmd_start_index, t0_s2s_rkey_to_ptseg_info.dma_cmd_start_index);
    modify_field(t0_s2s_rkey_to_ptseg_info_scr.log_page_size, t0_s2s_rkey_to_ptseg_info.log_page_size);
    modify_field(t0_s2s_rkey_to_ptseg_info_scr.tbl_id, t0_s2s_rkey_to_ptseg_info.tbl_id);
    modify_field(t0_s2s_rkey_to_ptseg_info_scr.pad, t0_s2s_rkey_to_ptseg_info.pad);

}
action resp_tx_rsqrkey_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_dcqcn_info_scr.dcqcn_cb_addr, to_s3_dcqcn_info.dcqcn_cb_addr);
    modify_field(to_s3_dcqcn_info_scr.congestion_mgmt_enable, to_s3_dcqcn_info.congestion_mgmt_enable);
    modify_field(to_s3_dcqcn_info_scr.packet_len, to_s3_dcqcn_info.packet_len);
    modify_field(to_s3_dcqcn_info_scr.new_timer_cindex, to_s3_dcqcn_info.new_timer_cindex);
    modify_field(to_s3_dcqcn_info_scr.rsvd, to_s3_dcqcn_info.rsvd);

    // stage to stage
    modify_field(t0_s2s_rsqwqe_to_rkey_info_scr.transfer_va, t0_s2s_rsqwqe_to_rkey_info.transfer_va);
    modify_field(t0_s2s_rsqwqe_to_rkey_info_scr.header_template_addr, t0_s2s_rsqwqe_to_rkey_info.header_template_addr);
    modify_field(t0_s2s_rsqwqe_to_rkey_info_scr.curr_read_rsp_psn, t0_s2s_rsqwqe_to_rkey_info.curr_read_rsp_psn);
    modify_field(t0_s2s_rsqwqe_to_rkey_info_scr.log_pmtu, t0_s2s_rsqwqe_to_rkey_info.log_pmtu);
    modify_field(t0_s2s_rsqwqe_to_rkey_info_scr.key_id, t0_s2s_rsqwqe_to_rkey_info.key_id);
    modify_field(t0_s2s_rsqwqe_to_rkey_info_scr.send_aeth, t0_s2s_rsqwqe_to_rkey_info.send_aeth);
    modify_field(t0_s2s_rsqwqe_to_rkey_info_scr.last_or_only, t0_s2s_rsqwqe_to_rkey_info.last_or_only);
    modify_field(t0_s2s_rsqwqe_to_rkey_info_scr.transfer_bytes, t0_s2s_rsqwqe_to_rkey_info.transfer_bytes);
    modify_field(t0_s2s_rsqwqe_to_rkey_info_scr.header_template_size, t0_s2s_rsqwqe_to_rkey_info.header_template_size);
    modify_field(t0_s2s_rsqwqe_to_rkey_info_scr.pad, t0_s2s_rsqwqe_to_rkey_info.pad);

}
action resp_tx_rsqwqe_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
    modify_field(t0_s2s_rqcb2_to_rsqwqe_info_scr.curr_read_rsp_psn, t0_s2s_rqcb2_to_rsqwqe_info.curr_read_rsp_psn);
    modify_field(t0_s2s_rqcb2_to_rsqwqe_info_scr.log_pmtu, t0_s2s_rqcb2_to_rsqwqe_info.log_pmtu);
    modify_field(t0_s2s_rqcb2_to_rsqwqe_info_scr.serv_type, t0_s2s_rqcb2_to_rsqwqe_info.serv_type);
    modify_field(t0_s2s_rqcb2_to_rsqwqe_info_scr.header_template_addr, t0_s2s_rqcb2_to_rsqwqe_info.header_template_addr);
    modify_field(t0_s2s_rqcb2_to_rsqwqe_info_scr.header_template_size, t0_s2s_rqcb2_to_rsqwqe_info.header_template_size);
    modify_field(t0_s2s_rqcb2_to_rsqwqe_info_scr.read_rsp_in_progress, t0_s2s_rqcb2_to_rsqwqe_info.read_rsp_in_progress);
    modify_field(t0_s2s_rqcb2_to_rsqwqe_info_scr.pad, t0_s2s_rqcb2_to_rsqwqe_info.pad);

}
action resp_tx_rsqwqe_bt_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_bt_info_scr.log_rsq_size, to_s2_bt_info.log_rsq_size);
    modify_field(to_s2_bt_info_scr.log_pmtu, to_s2_bt_info.log_pmtu);
    modify_field(to_s2_bt_info_scr.rsq_base_addr, to_s2_bt_info.rsq_base_addr);
    modify_field(to_s2_bt_info_scr.bt_cindex, to_s2_bt_info.bt_cindex);
    modify_field(to_s2_bt_info_scr.end_index, to_s2_bt_info.end_index);
    modify_field(to_s2_bt_info_scr.search_index, to_s2_bt_info.search_index);
    modify_field(to_s2_bt_info_scr.curr_read_rsp_psn, to_s2_bt_info.curr_read_rsp_psn);
    modify_field(to_s2_bt_info_scr.read_rsp_in_progress, to_s2_bt_info.read_rsp_in_progress);
    modify_field(to_s2_bt_info_scr.bt_in_progress, to_s2_bt_info.bt_in_progress);
    modify_field(to_s2_bt_info_scr.rsvd, to_s2_bt_info.rsvd);

    // stage to stage
    modify_field(t0_s2s_bt_info_scr.read_or_atomic, t0_s2s_bt_info.read_or_atomic);
    modify_field(t0_s2s_bt_info_scr.rsvd, t0_s2s_bt_info.rsvd);
    modify_field(t0_s2s_bt_info_scr.psn, t0_s2s_bt_info.psn);
    modify_field(t0_s2s_bt_info_scr.va, t0_s2s_bt_info.va);
    modify_field(t0_s2s_bt_info_scr.r_key, t0_s2s_bt_info.r_key);
    modify_field(t0_s2s_bt_info_scr.len, t0_s2s_bt_info.len);
}

