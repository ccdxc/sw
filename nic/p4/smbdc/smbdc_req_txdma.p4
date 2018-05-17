/***********************************************************************/
/* smbdc_req_txdma.p4 */
/***********************************************************************/

#include "../common-p4+/common_txdma_dummy.p4"

/**** table declarations ****/

#define tx_table_s0_t0 smbdc_req_tx_s0_t0
#define tx_table_s0_t1 smbdc_req_tx_s0_t1
#define tx_table_s0_t2 smbdc_req_tx_s0_t2
#define tx_table_s0_t3 smbdc_req_tx_s0_t3

#define tx_table_s1_t0 smbdc_req_tx_s1_t0
#define tx_table_s1_t1 smbdc_req_tx_s1_t1
#define tx_table_s1_t2 smbdc_req_tx_s1_t2
#define tx_table_s1_t3 smbdc_req_tx_s1_t3

#define tx_table_s2_t0 smbdc_req_tx_s2_t0
#define tx_table_s2_t1 smbdc_req_tx_s2_t1
#define tx_table_s2_t2 smbdc_req_tx_s2_t2
#define tx_table_s2_t3 smbdc_req_tx_s2_t3

#define tx_table_s3_t0 smbdc_req_tx_s3_t0
#define tx_table_s3_t1 smbdc_req_tx_s3_t1
#define tx_table_s3_t2 smbdc_req_tx_s3_t2
#define tx_table_s3_t3 smbdc_req_tx_s3_t3

#define tx_table_s4_t0 smbdc_req_tx_s4_t0
#define tx_table_s4_t1 smbdc_req_tx_s4_t1
#define tx_table_s4_t2 smbdc_req_tx_s4_t2
#define tx_table_s4_t3 smbdc_req_tx_s4_t3

#define tx_table_s5_t0 smbdc_req_tx_s5_t0
#define tx_table_s5_t1 smbdc_req_tx_s5_t1
#define tx_table_s5_t2 smbdc_req_tx_s5_t2
#define tx_table_s5_t3 smbdc_req_tx_s5_t3

#define tx_table_s6_t0 smbdc_req_tx_s6_t0
#define tx_table_s6_t1 smbdc_req_tx_s6_t1
#define tx_table_s6_t2 smbdc_req_tx_s6_t2
#define tx_table_s6_t3 smbdc_req_tx_s6_t3

#define tx_table_s7_t0 smbdc_req_tx_s7_t0
#define tx_table_s7_t1 smbdc_req_tx_s7_t1
#define tx_table_s7_t2 smbdc_req_tx_s7_t2
#define tx_table_s7_t3 smbdc_req_tx_s7_t3


#define tx_table_s0_t0_action smbdc_req_tx_process
#define tx_table_s1_t0_action smbdc_req_tx_wqe_process
#define tx_table_s1_t0_action1 smbdc_req_tx_rdma_proxy_cqcb_process
#define tx_table_s2_t0_action smbdc_req_tx_mr_select_process
#define tx_table_s2_t0_action1 smbdc_req_tx_rdma_cqe_process
#define tx_table_s2_t0_action2 smbdc_req_tx_sge_process
#define tx_table_s3_t0_action smbdc_req_tx_post_rdma_req_process
#define tx_table_s3_t0_action1 smbdc_req_tx_sq_wqe_context_process
#define tx_table_s4_t0_action smbdc_req_tx_sqcb_writeback_cq_process
#define tx_table_s4_t0_action1 smbdc_req_tx_sqcb_writeback_sq_process
#define tx_table_s4_t0_action2 smbdc_req_tx_cqcb_process

#include "../common-p4+/common_txdma.p4"

/**** Macros ****/

#define GENERATE_GLOBAL_K \
    modify_field(phv_global_common_scr.lif, phv_global_common.lif);\
    modify_field(phv_global_common_scr.qid, phv_global_common.qid);\
    modify_field(phv_global_common_scr.qtype, phv_global_common.qtype);\
    modify_field(phv_global_common_scr.cb_addr, phv_global_common.cb_addr);\
    modify_field(phv_global_common_scr.pad, phv_global_common.pad);\

/**** header definitions ****/

header_type phv_global_common_t {
    fields {
        lif                              :   11;
        qid                              :   24;
        qtype                            :    3;
        cb_addr                          :   25;
        pad                              :   52;
    }
}

header_type smbdc_req_tx_sqcb_to_wqe_info_t {
    fields {
        max_fragmented_size              :   32;
        max_send_size                    :   32;
        current_sge_id                   :   8;
        current_sge_offset               :   32;
        in_progress                      :   1;
        send_credits_available           :   1;
        pad                              :   54;
    }
}

header_type smbdc_req_tx_wqe_to_sge_info_t {
    fields {
        wrid                             :   32;
        current_total_offset             :   32;
        current_sge_id                   :   8;
        current_sge_offset               :   32;
        num_valid_sges                   :   2;
        max_send_size                    :   32;
        pad                              :   22;
    }
}

header_type smbdc_req_tx_wqe_to_mr_select_info_t {
    fields {
        num_mrs                          :     8;
        pad                              :   152;
    }
}

header_type smbdc_req_tx_mr_select_to_rdma_info_t {
    fields {
        num_wqes                         :     8;
        fill_smbd_hdr_sge                :     1;
        dma_cmd_start_index              :     5;
        pad                              :   146;
    }
}

header_type smbdc_req_tx_rdma_proxy_cqcb_info_t {
    fields {
        rdma_cq_proxy_cindex             :   16;
        wqe_context_addr                 :   34;
        pad                              :   110;
    }
}

header_type smbdc_req_tx_rdma_cqe_info_t {
    fields {
        wqe_context_addr                 :   34;
        pad                              :   126;
    }
}

header_type smbdc_req_tx_rdma_cqe_to_wqe_context_info_t {
    fields {
        msn                              :   24;
        pad                              :   136;
    }
}

header_type smbdc_req_tx_writeback_info_cq_t {
    fields {
        pad                              :   160;
    }
}

header_type smbdc_req_tx_writeback_info_sq_t {
    fields {
        pad                              :   160;
    }
}


header_type smbdc_req_tx_to_stage_t {
    fields {
        pad                              :   128;
    }
}

header_type smbdc_req_tx_to_stage_s2_t {
    fields {
        total_data_length                :   32;
        pad                              :   96;
    }
}

header_type smbdc_req_tx_to_stage_s3_t {
    fields {
        cqcb_addr                        :   34;
        pad                              :   94;
    }
}

header_type smbdc_req_tx_to_stage_s4_cq_t {
    fields {
        incr_proxy_cq_cindex             :   1;
        incr_rdma_cq_cindex              :   1;
        incr_sq_unack_pindex             :   1;
        clear_busy                       :   1;
        clear_in_progress                :   1;
        rdma_cq_lif                      :   12;
        rdma_cq_qtype                    :   3;
        rdma_cq_qid                      :   24;
        rdma_cq_ring_id                  :   2;
        pad                              :   82;
    }
}

header_type smbdc_req_tx_to_stage_s4_sq_t {
    fields {
        in_progress                      :   1;
        clear_busy_and_exit              :   1;
        current_sge_id                   :   8;
        current_sge_offset               :   32;
        current_total_offset             :   32;
        incr_sq_cindex                   :   1;
        decr_send_credits                :   1;
        pad                              :   45;
    }
}

/**** global header unions ****/

@pragma pa_header_union ingress common_global
metadata phv_global_common_t phv_global_common;
@pragma scratch_metadata
metadata phv_global_common_t phv_global_common_scr;

/**** to stage header unions ****/

//To-Stage-0
@pragma pa_header_union ingress to_stage_0 to_s0_to_stage

metadata smbdc_req_tx_to_stage_t to_s0_to_stage;
@pragma scratch_metadata
metadata smbdc_req_tx_to_stage_t to_s0_to_stage_scr;


//To-Stage-1
@pragma pa_header_union ingress to_stage_1 to_s1_to_stage

metadata smbdc_req_tx_to_stage_t to_s1_to_stage;
@pragma scratch_metadata
metadata smbdc_req_tx_to_stage_t to_s1_to_stage_scr;

//To-Stage-2
@pragma pa_header_union ingress to_stage_2 to_s2_to_stage

metadata smbdc_req_tx_to_stage_s2_t to_s2_to_stage;
@pragma scratch_metadata
metadata smbdc_req_tx_to_stage_s2_t to_s2_to_stage_scr;

//To-Stage-3
@pragma pa_header_union ingress to_stage_3 to_s3_to_stage

metadata smbdc_req_tx_to_stage_s3_t to_s3_to_stage;
@pragma scratch_metadata
metadata smbdc_req_tx_to_stage_s3_t to_s3_to_stage_scr;

//To-Stage-4
@pragma pa_header_union ingress to_stage_4 to_s4_to_stage_cq to_s4_to_stage_sq

metadata smbdc_req_tx_to_stage_s4_cq_t to_s4_to_stage_cq;
@pragma scratch_metadata
metadata smbdc_req_tx_to_stage_s4_cq_t to_s4_to_stage_cq_scr;

metadata smbdc_req_tx_to_stage_s4_sq_t to_s4_to_stage_sq;
@pragma scratch_metadata
metadata smbdc_req_tx_to_stage_s4_sq_t to_s4_to_stage_sq_scr;

//To-Stage-5
@pragma pa_header_union ingress to_stage_5 to_s5_to_stage

metadata smbdc_req_tx_to_stage_t to_s5_to_stage;
@pragma scratch_metadata
metadata smbdc_req_tx_to_stage_t to_s5_to_stage_scr;

//To-Stage-6
@pragma pa_header_union ingress to_stage_6 to_s6_to_stage

metadata smbdc_req_tx_to_stage_t to_s6_to_stage;
@pragma scratch_metadata
metadata smbdc_req_tx_to_stage_t to_s6_to_stage_scr;

//To-Stage-7
@pragma pa_header_union ingress to_stage_7 to_s7_to_stage

metadata smbdc_req_tx_to_stage_t to_s7_to_stage;
@pragma scratch_metadata
metadata smbdc_req_tx_to_stage_t to_s7_to_stage_scr;


/**** stage to stage header unions ****/

//Table-0
@pragma pa_header_union ingress common_t0_s2s t0_s2s_sqcb_to_wqe_info t0_s2s_rdma_proxy_cqcb_info t0_s2s_wqe_to_mr_select_info t0_s2s_wqe_to_sge_info t0_s2s_mr_select_to_rdma_info t0_s2s_rdma_cqe_info t0_s2s_rdma_cqe_to_wqe_context_info t0_s2s_writeback_info_cq t0_s2s_writeback_info_sq

metadata smbdc_req_tx_sqcb_to_wqe_info_t t0_s2s_sqcb_to_wqe_info;
@pragma scratch_metadata
metadata smbdc_req_tx_sqcb_to_wqe_info_t t0_s2s_sqcb_to_wqe_info_scr;

metadata smbdc_req_tx_rdma_proxy_cqcb_info_t t0_s2s_rdma_proxy_cqcb_info;
@pragma scratch_metadata
metadata smbdc_req_tx_rdma_proxy_cqcb_info_t t0_s2s_rdma_proxy_cqcb_info_scr;

metadata smbdc_req_tx_rdma_cqe_info_t t0_s2s_rdma_cqe_info;
@pragma scratch_metadata
metadata smbdc_req_tx_rdma_cqe_info_t t0_s2s_rdma_cqe_info_scr;

metadata smbdc_req_tx_rdma_cqe_to_wqe_context_info_t t0_s2s_rdma_cqe_to_wqe_context_info;
@pragma scratch_metadata
metadata smbdc_req_tx_rdma_cqe_to_wqe_context_info_t t0_s2s_rdma_cqe_to_wqe_context_info_scr;

metadata smbdc_req_tx_wqe_to_mr_select_info_t t0_s2s_wqe_to_mr_select_info;
@pragma scratch_metadata
metadata smbdc_req_tx_wqe_to_mr_select_info_t t0_s2s_wqe_to_mr_select_info_scr;

metadata smbdc_req_tx_wqe_to_sge_info_t t0_s2s_wqe_to_sge_info;
@pragma scratch_metadata
metadata smbdc_req_tx_wqe_to_sge_info_t t0_s2s_wqe_to_sge_info_scr;

metadata smbdc_req_tx_mr_select_to_rdma_info_t t0_s2s_mr_select_to_rdma_info;
@pragma scratch_metadata
metadata smbdc_req_tx_mr_select_to_rdma_info_t t0_s2s_mr_select_to_rdma_info_scr;

metadata smbdc_req_tx_writeback_info_cq_t t0_s2s_writeback_info_cq;
@pragma scratch_metadata
metadata smbdc_req_tx_writeback_info_cq_t t0_s2s_writeback_info_cq_scr;

metadata smbdc_req_tx_writeback_info_sq_t t0_s2s_writeback_info_sq;
@pragma scratch_metadata
metadata smbdc_req_tx_writeback_info_sq_t t0_s2s_writeback_info_sq_scr;

//Table-1

//Table-2

//Table-3

action smbdc_req_tx_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
}

action smbdc_req_tx_wqe_process () {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_to_stage_scr.pad, to_s1_to_stage.pad);

    // stage to stage
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.max_fragmented_size, t0_s2s_sqcb_to_wqe_info.max_fragmented_size);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.max_send_size, t0_s2s_sqcb_to_wqe_info.max_send_size);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.current_sge_id, t0_s2s_sqcb_to_wqe_info.current_sge_id);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.current_sge_offset, t0_s2s_sqcb_to_wqe_info.current_sge_offset);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.in_progress, t0_s2s_sqcb_to_wqe_info.in_progress);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.send_credits_available, t0_s2s_sqcb_to_wqe_info.send_credits_available);
    modify_field(t0_s2s_sqcb_to_wqe_info_scr.pad, t0_s2s_sqcb_to_wqe_info.pad);

}

action smbdc_req_tx_mr_select_process () {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_to_stage_scr.pad, to_s2_to_stage.pad);

    // stage to stage
    modify_field(t0_s2s_wqe_to_mr_select_info_scr.num_mrs, t0_s2s_wqe_to_mr_select_info.num_mrs);
    modify_field(t0_s2s_wqe_to_mr_select_info_scr.pad, t0_s2s_wqe_to_mr_select_info.pad);

}

action smbdc_req_tx_sge_process () {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_to_stage_scr.total_data_length, to_s2_to_stage.total_data_length);
    modify_field(to_s2_to_stage_scr.pad, to_s2_to_stage.pad);

    // stage to stage
    modify_field(t0_s2s_wqe_to_sge_info_scr.wrid, t0_s2s_wqe_to_sge_info.wrid);
    modify_field(t0_s2s_wqe_to_sge_info_scr.current_total_offset, t0_s2s_wqe_to_sge_info.current_total_offset);
    modify_field(t0_s2s_wqe_to_sge_info_scr.current_sge_id, t0_s2s_wqe_to_sge_info.current_sge_id);
    modify_field(t0_s2s_wqe_to_sge_info_scr.current_sge_offset, t0_s2s_wqe_to_sge_info.current_sge_offset);
    modify_field(t0_s2s_wqe_to_sge_info_scr.num_valid_sges, t0_s2s_wqe_to_sge_info.num_valid_sges);
    modify_field(t0_s2s_wqe_to_sge_info_scr.max_send_size, t0_s2s_wqe_to_sge_info.max_send_size);
    modify_field(t0_s2s_wqe_to_sge_info_scr.pad, t0_s2s_wqe_to_sge_info.pad);

}


action smbdc_req_tx_post_rdma_req_process () {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_to_stage_scr.pad, to_s3_to_stage.pad);

    // stage to stage
    modify_field(t0_s2s_mr_select_to_rdma_info_scr.num_wqes, t0_s2s_mr_select_to_rdma_info.num_wqes);
    modify_field(t0_s2s_mr_select_to_rdma_info_scr.fill_smbd_hdr_sge, t0_s2s_mr_select_to_rdma_info.fill_smbd_hdr_sge);
    modify_field(t0_s2s_mr_select_to_rdma_info_scr.dma_cmd_start_index, t0_s2s_mr_select_to_rdma_info.dma_cmd_start_index);
    modify_field(t0_s2s_mr_select_to_rdma_info_scr.pad, t0_s2s_mr_select_to_rdma_info.pad);

}

action smbdc_req_tx_rdma_proxy_cqcb_process () {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s1_to_stage_scr.pad, to_s1_to_stage.pad);

    // stage to stage
    modify_field(t0_s2s_rdma_proxy_cqcb_info_scr.rdma_cq_proxy_cindex, t0_s2s_rdma_proxy_cqcb_info.rdma_cq_proxy_cindex);
    modify_field(t0_s2s_rdma_proxy_cqcb_info_scr.wqe_context_addr, t0_s2s_rdma_proxy_cqcb_info.wqe_context_addr);
    modify_field(t0_s2s_rdma_proxy_cqcb_info_scr.pad, t0_s2s_rdma_proxy_cqcb_info.pad);
}

action smbdc_req_tx_rdma_cqe_process () {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s2_to_stage_scr.pad, to_s2_to_stage.pad);

    // stage to stage
    modify_field(t0_s2s_rdma_cqe_info_scr.wqe_context_addr, t0_s2s_rdma_cqe_info.wqe_context_addr);
    modify_field(t0_s2s_rdma_cqe_info_scr.pad, t0_s2s_rdma_cqe_info.pad);
}

action smbdc_req_tx_sq_wqe_context_process () {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s3_to_stage_scr.pad, to_s3_to_stage.pad);
    modify_field(to_s3_to_stage_scr.cqcb_addr, to_s3_to_stage.cqcb_addr);

    // stage to stage
    modify_field(t0_s2s_rdma_cqe_to_wqe_context_info_scr.msn, t0_s2s_rdma_cqe_to_wqe_context_info.msn);
    modify_field(t0_s2s_rdma_cqe_to_wqe_context_info_scr.pad, t0_s2s_rdma_cqe_to_wqe_context_info.pad);
}

action smbdc_req_tx_cqcb_process () {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_to_stage_cq_scr.pad, to_s4_to_stage_cq.pad);

    // stage to stage
}


action smbdc_req_tx_sqcb_writeback_cq_process () {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_to_stage_cq_scr.incr_proxy_cq_cindex, to_s4_to_stage_cq.incr_proxy_cq_cindex);
    modify_field(to_s4_to_stage_cq_scr.incr_rdma_cq_cindex, to_s4_to_stage_cq.incr_rdma_cq_cindex);
    modify_field(to_s4_to_stage_cq_scr.incr_sq_unack_pindex, to_s4_to_stage_cq.incr_sq_unack_pindex);
    modify_field(to_s4_to_stage_cq_scr.clear_busy, to_s4_to_stage_cq.clear_busy);
    modify_field(to_s4_to_stage_cq_scr.clear_in_progress, to_s4_to_stage_cq.clear_in_progress);
    modify_field(to_s4_to_stage_cq_scr.rdma_cq_lif, to_s4_to_stage_cq.rdma_cq_lif);
    modify_field(to_s4_to_stage_cq_scr.rdma_cq_qtype, to_s4_to_stage_cq.rdma_cq_qtype);
    modify_field(to_s4_to_stage_cq_scr.rdma_cq_qid, to_s4_to_stage_cq.rdma_cq_qid);
    modify_field(to_s4_to_stage_cq_scr.rdma_cq_ring_id, to_s4_to_stage_cq.rdma_cq_ring_id);
    modify_field(to_s4_to_stage_cq_scr.pad, to_s4_to_stage_cq.pad);

    // stage to stage
    modify_field(t0_s2s_writeback_info_cq_scr.pad, t0_s2s_writeback_info_cq.pad);

}

action smbdc_req_tx_sqcb_writeback_sq_process () {

    // from ki global
    GENERATE_GLOBAL_K

    // to stage
    modify_field(to_s4_to_stage_sq_scr.in_progress, to_s4_to_stage_sq.in_progress);
    modify_field(to_s4_to_stage_sq_scr.clear_busy_and_exit, to_s4_to_stage_sq.clear_busy_and_exit);
    modify_field(to_s4_to_stage_sq_scr.current_sge_id, to_s4_to_stage_sq.current_sge_id);
    modify_field(to_s4_to_stage_sq_scr.current_sge_offset, to_s4_to_stage_sq.current_sge_offset);
    modify_field(to_s4_to_stage_sq_scr.incr_sq_cindex, to_s4_to_stage_sq.incr_sq_cindex);
    modify_field(to_s4_to_stage_sq_scr.decr_send_credits, to_s4_to_stage_sq.decr_send_credits);
    modify_field(to_s4_to_stage_sq_scr.pad, to_s4_to_stage_sq.pad);

    // stage to stage
    modify_field(t0_s2s_writeback_info_sq_scr.pad, t0_s2s_writeback_info_sq.pad);

}
