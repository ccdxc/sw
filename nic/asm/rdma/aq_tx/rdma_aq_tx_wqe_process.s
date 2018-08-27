#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "common_phv.h"
#include "p4/common/defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct aqwqe_t d;
struct aq_tx_s1_t0_k k;

#define IN_TO_S_P to_s1_info
    
#define K_CQCB_BASE_ADDR_HI CAPRI_KEY_FIELD(IN_TO_S_P, cqcb_base_addr_hi)
#define K_SQCB_BASE_ADDR_HI CAPRI_KEY_FIELD(IN_TO_S_P, sqcb_base_addr_hi) 

%%

    .param      dummy
    .param      rdma_cq_tx_stage0
    .param      rdma_req_tx_stage0    
    .param      rdma_aq_tx_feedback_process
    .param      rdma_aq_tx_modify_qp_2_process
.align
rdma_aq_tx_wqe_process:

    add         r1, r0, d.op  //BD-slot

    .brbegin
    br          r1[3:0]
    nop         //BD slot

    .brcase     AQ_OP_TYPE_NOP
        b           prepare_feedback
        nop
    .brcase     AQ_OP_TYPE_CREATE_CQ
        b           create_cq
        nop
    .brcase     AQ_OP_TYPE_CREATE_QP
        b           create_qp
        nop
    .brcase     AQ_OP_TYPE_REG_MR
        b           reg_mr
        nop
    .brcase     AQ_OP_TYPE_STATS_HDRS
        b           exit
        nop
    .brcase     AQ_OP_TYPE_STATS_VALS
        b           exit
        nop
    .brcase     AQ_OP_TYPE_DEREG_MR
        b           exit
        nop
    .brcase     AQ_OP_TYPE_RESIZE_CQ
        b           exit
        nop
    .brcase     AQ_OP_TYPE_DESTROY_CQ
        b           exit
        nop
    .brcase     AQ_OP_TYPE_MODIFY_QP
        b           modify_qp
        nop
    .brcase     AQ_OP_TYPE_QUERY_QP
        b           exit
        nop
    .brcase     AQ_OP_TYPE_DESTROY_QP
        b           exit
        nop
    .brcase     AQ_OP_TYPE_STATS_DUMP
        b           stats_dump
        nop
    .brcase     13
        b           exit
        nop
    .brcase     14
        b           exit
        nop
    .brcase     15
        b           exit
        nop

    .brend

reg_mr:
    // Key table index (high 3 bytes) and user_key (low byte)
    // XXX: Use KEY_INDEX_GET
    srl         r1, d.mr.lkey, 8
    // XXX: Use KEY_USER_KEY_GET if KEY_INDEX_GET is deprecated after lkey format update
    and         r2, d.mr.lkey, 0xFF
    phvwrpair   p.key.user_key, r2, p.key.state, KEY_STATE_VALID
    phvwrpair   p.key.type, MR_TYPE_MR, p.key.acc_ctrl, d.mr.access_flags[7:0]
    phvwrpair   p.key.log_page_size, d.mr.page_size_log2, p.key.len, d.mr.length[31:0]
    phvwrpair   p.key.base_va, d.mr.va, p.key.pt_base, d.mr.tbl_index
    phvwrpair   p.key.pd, d.mr.pd_id, p.key.flags, 0x0B
    phvwrpair   p.key.mr_l_key, 0, p.key.mr_cookie, 0

    KT_BASE_ADDR_GET2(r3, r4)
    // XXX: Use KEY_ENTRY_ADDR_GET after lkey format update
    // key_entry_p = key_base_addr + (lkey_index * sizeof(struct key_entry_t))
    add         r4, r3, r1, LOG_SIZEOF_KEY_ENTRY_T
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_KT_UPDATE)
    DMA_PHV2MEM_SETUP(r6, c2, key, key, r4)
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_PT_SRC_HOST)
    add         r3, r0, d.mr.map_count, CAPRI_LOG_SIZEOF_U64
    DMA_HOST_MEM2MEM_SRC_SETUP(r6, r3, d.mr.dma_addr)
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_PT_DST_HBM)

    // pt_seg_size = HBM_NUM_PT_ENTRIES_PER_CACHE_LINE * lkey->hostmem_page_size
    sll         r4, HBM_NUM_PT_ENTRIES_PER_CACHE_LINE, d.mr.page_size_log2
    sub         r4, r4, 1
    // pt_seg_offset = lkey->base_va % pt_seg_size
    and         r4, d.mr.va, r4

    // hbm_add = (pt_seg_offset + lkey->pt_base) * 8 + (pt_base_addr)
    PT_BASE_ADDR_GET2(r2)
    add         r4, r4, d.mr.tbl_index
    add         r5, r2, r4, CAPRI_LOG_SIZEOF_U64
    DMA_HBM_MEM2MEM_DST_SETUP(r6, r3, r5)

    b           prepare_feedback
    nop

create_cq:

    phvwr   p.{cqcb.intrinsic.host_rings, cqcb.intrinsic.total_rings}, (MAX_CQ_RINGS<<4|MAX_CQ_DOORBELL_RINGS)

    // r3 will have the pt_base_address where pt translations
    // should be copied to
    PT_BASE_ADDR_GET2(r4) 
    add         r3, r4, d.cq.tbl_index, CAPRI_LOG_SIZEOF_U64
    phvwrpair   p.cqcb.pt_base_addr, r3, p.cqcb.log_cq_page_size, d.cq.page_size_log2[4:0]
    phvwrpair   p.cqcb.log_wqe_size, d.cq.stride_log2[4:0], p.cqcb.log_num_wqes, d.cq.depth_log2[4:0]
    phvwrpair   p.cqcb.cq_id, d.id_ver[23:0], p.cqcb.eq_id, d.cq.eq_id[23:0]
    
    //Setup DMA to copy PT translations from host to HBM
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_CREATE_CQ_PT)    

    add         r4, r0, d.cq.map_count, CAPRI_LOG_SIZEOF_U64
    DMA_HOST_MEM2MEM_SRC_SETUP(r6, r4, d.cq.dma_addr)
    DMA_HBM_MEM2MEM_DST_SETUP(r6, r4, r3)

    // log_num_pages in r2
    add         r2, d.cq.depth_log2, d.cq.stride_log2
    sub         r2, r2, d.cq.page_size_log2

    add         r1, r0, r0
    mincr       r1, r2, 1
    phvwr       p.cqcb.pt_next_pg_index, r1

    //compute the offset of the label of CQ program
    addi        r4, r0, rdma_cq_tx_stage0[33:CAPRI_RAW_TABLE_PC_SHIFT] ;
    addi        r3, r0, dummy[33:CAPRI_RAW_TABLE_PC_SHIFT] ;
    sub         r4, r4, r3
    phvwr       p.cqcb.intrinsic.pc, r4

    //          setup the DMA for CQCB
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_CREATE_CQ_CB)        
    CAPRI_GET_TABLE_0_K(aq_tx_phv_t, r7)
    AQ_TX_CQCB_ADDR_GET(r1, d.id_ver[23:0], K_CQCB_BASE_ADDR_HI)

    DMA_PHV2MEM_SETUP(r6, c1, cqcb, cqcb, r1)

    b           prepare_feedback
    nop
    
create_qp:

    // SQCB0:
    
    phvwr       p.{sqcb0.intrinsic.host_rings, sqcb0.intrinsic.total_rings}, (MAX_SQ_HOST_RINGS<<4|MAX_SQ_DOORBELL_RINGS)
    phvwr       p.sqcb0.log_num_wqes, d.qp.sq_depth_log2[4: 0]
    

    // TODO: For now setting it to RTS, but later change it to INIT
    // state. modify_qp is supposed to set it to RTR and RTS.
    phvwr       p.sqcb0.state, QP_STATE_RTS
    phvwr       p.sqcb0.color, 1

    //TODO: SQ in HBM still need to be implemented

    phvwr       p.sqcb0.log_sq_page_size, d.qp.sq_page_size_log2[4:0]
    phvwr       p.sqcb0.log_wqe_size, d.qp.sq_stride_log2
    phvwr       p.sqcb0.pd, d.qp.pd_id
    phvwr       p.sqcb0.service, d.type_state
    
    //compute the offset of the label of CQ program
    addi        r4, r0, rdma_req_tx_stage0[33:CAPRI_RAW_TABLE_PC_SHIFT] ;
    addi        r3, r0, dummy[33:CAPRI_RAW_TABLE_PC_SHIFT] ;
    sub         r4, r4, r3              
    phvwr       p.sqcb0.intrinsic.pc, r4
    
    // SQCB1:

    phvwr       p.sqcb1.cq_id, d.qp.sq_cq_id[23:0]
    phvwr       p.sqcb1.state, QP_STATE_RTS

    //TODO: Standard says initial value of LSN should be 0. Need to fix
    phvwrpair   p.sqcb1.service, d.type_state[3:0], p.sqcb1.lsn, 128
    phvwr       p.sqcb1.ssn, 1
    phvwr       p.sqcb1.max_ssn, 1
    
    //TODO: what    is the correct value of credits? zero?
    //infinite  retries                 
    phvwr       p.sqcb1.credits, 0xe
    phvwr       p.{sqcb1.err_retry_count, sqcb1.rnr_retry_count}, (0x7<<3|0x7)

//SQCB2:

    phvwrpair   p.sqcb2.log_sq_size, d.qp.sq_depth_log2[4: 0], p.sqcb2.credits, 0xe
    phvwr       p.sqcb2.ssn, 1
    phvwrpair   p.{sqcb2.err_retry_ctr, sqcb2.rnr_retry_ctr}, (0x7<<3|0x7), p.sqcb2.lsn,128

    //          TODO: Move RSQ/RRQ allocation to modify_qp frm create_qp
    //          TODO: Move pmtu setup to modify_qp
    
    //populate the PC in SQCB0, SQCB1
    addi        r4, r0, rdma_req_tx_stage0[33:CAPRI_RAW_TABLE_PC_SHIFT] ;
    addi        r3, r0, dummy[33:CAPRI_RAW_TABLE_PC_SHIFT] ;
    sub         r4, r4, r3
    phvwr       p.sqcb0.intrinsic.pc, r4
    phvwr       p.sqcb1.pc, r4

    //Setup     DMA for SQ PT

    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_CREATE_QP_SQPT)
    add         r4, r0, d.qp.sq_map_count, CAPRI_LOG_SIZEOF_U64

    PT_BASE_ADDR_GET2(r4) 
    add         r3, r4, d.qp.sq_tbl_index_xrcd_id, CAPRI_LOG_SIZEOF_U64
    phvwr       p.sqcb0.pt_base_addr, r3

    DMA_HOST_MEM2MEM_SRC_SETUP(r6, r4, d.qp.sq_dma_addr)
    DMA_HBM_MEM2MEM_DST_SETUP(r6, r4, r3)

    // setup DMA for SQCB
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_CREATE_QP_CB)        

    CAPRI_GET_TABLE_0_K(aq_tx_phv_t, r7)
    SQCB_ADDR_GET(r1, d.id_ver[23:0], K_SQCB_BASE_ADDR_HI)

    DMA_PHV2MEM_SETUP(r6, c1, sqcb0, sqcb2, r1)

    phvwrpair   p.rdma_feedback.aq_completion.op, AQ_OP_TYPE_CREATE_QP, p.rdma_feedback.create_qp.rq_cq_id, d.qp.rq_cq_id[23:0]
    phvwrpair   p.rdma_feedback.create_qp.rq_depth_log2, d.qp.rq_depth_log2, p.rdma_feedback.create_qp.rq_stride_log2, d.qp.rq_stride_log2
    phvwrpair  p.rdma_feedback.create_qp.rq_page_size_log2, d.qp.rq_page_size_log2, p.rdma_feedback.create_qp.pd, d.qp.pd_id   
    phvwr       p.p4_to_p4plus.create_qp_ext.rq_dma_addr, d.qp.rq_dma_addr
    phvwrpair     p.rdma_feedback.create_qp.rq_type_state, d.type_state, p.rdma_feedback.create_qp.rq_id, d.id_ver[23:0]

    b           prepare_feedback
    nop
    
stats_dump:
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_STATS_DUMP_1)

    add         r1, r0, d.type_state
    .brbegin
    br          r1[3:0]
    nop         // BD slot

    .brcase     AQ_STATS_DUMP_TYPE_QP
        b           qp_dump
        nop

    .brcase     1
    .brcase     2
    .brcase     3
    .brcase     4
    .brcase     5
    .brcase     6
    .brcase     7
    .brcase     8
    .brcase     9
    .brcase     10
    .brcase     11
    .brcase     12
    .brcase     13
    .brcase     14
    .brcase     15
        b           exit
        nop
    .brend

qp_dump:
    //SQCB_ADDR_GET(r1, d.id_ver[55:32], K_SQCB_BASE_ADDR_HI)
    //RQCB_ADDR_GET(r2, d.id_ver[55:32], K_RQCB_BASE_ADDR_HI)

    DMA_HBM_MEM2MEM_SRC_SETUP(r6, CB3_OFFSET_BYTES, r1)
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_STATS_DUMP_2)
    DMA_HOST_MEM2MEM_DST_SETUP(r6, CB3_OFFSET_BYTES, d.stats.dma_addr)
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_STATS_DUMP_3)
    DMA_HBM_MEM2MEM_SRC_SETUP(r6, CB3_OFFSET_BYTES, r2)
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_STATS_DUMP_4)
    add         r3, d.stats.dma_addr, CB3_OFFSET_BYTES
    DMA_HOST_MEM2MEM_DST_SETUP(r6, CB3_OFFSET_BYTES, r3)

    b           prepare_feedback
    nop

modify_qp:

    //SQCB_ADDR_GET(r1, d.id_ver[55:32], K_SQCB_BASE_ADDR_HI)
    //RQCB_ADDR_GET(r2, d.id_ver[55:32], K_RQCB_BASE_ADDR_HI)

    add         r3, r0, d.dbid

dst_qp:
    andi        r6, r3, RDMA_UPDATE_QP_OPER_SET_DEST_QP
    beq         r6, r0, e_psn
    andi        r6, r3, RDMA_UPDATE_QP_OPER_SET_E_PSN

    // Invoke sqcb2 for QP
    add         r4, r1, (CB_UNIT_SIZE_BYTES * 2)
    add         r4, r4, FIELD_OFFSET(sqcb2_t, dst_qp)
    memwr.d     r4, d.mod_qp.qkey_dest_qpn

    add         r5, r2, r0
    add         r5, r5, FIELD_OFFSET(rqcb0_t, dst_qp)
    memwr.d     r5, d.mod_qp.qkey_dest_qpn

e_psn:
    beq         r6, r0, tx_psn
    andi        r6, r3, RDMA_UPDATE_QP_OPER_SET_TX_PSN

    // Invoke rqcb1
    add         r5, r2, (CB_UNIT_SIZE_BYTES)
    add         r5, r5, FIELD_OFFSET(rqcb1_t, e_psn)
    memwr.d     r5, d.mod_qp.rq_psn

tx_psn:
    beq         r6, r0, q_key
    andi        r6, r3, RDMA_UPDATE_QP_OPER_SET_Q_KEY

    // Invoke sqcb1
    add         r4, r1, (CB_UNIT_SIZE_BYTES)
    add         r5, r4, FIELD_OFFSET(sqcb1_t, tx_psn)
    memwr.d     r5, d.mod_qp.sq_psn

    add         r5, r4, FIELD_OFFSET(sqcb1_t, max_tx_psn)
    memwr.d     r5, d.mod_qp.sq_psn

    add         r5, r4, FIELD_OFFSET(sqcb1_t, rexmit_psn)
    memwr.d     r5, d.mod_qp.sq_psn

    //Invoke sqcb2
    add         r4, r1, (CB_UNIT_SIZE_BYTES * 2)
    add         r5, r4, FIELD_OFFSET(sqcb2_t, tx_psn)
    memwr.d     r5, d.mod_qp.sq_psn

    add         r5, r4, FIELD_OFFSET(sqcb2_t, exp_rsp_psn)
    sub         r4, d.mod_qp.sq_psn, 1
    memwr.d     r5, r4

q_key:
    mfspr       r7, spr_tbladdr
    beq         r6, r0, prepare_feedback
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_tx_modify_qp_2_process, r7) // BD slot

    //Invoke rqcb0
    add         r4, r2, r0
    add         r5, r4, FIELD_OFFSET(rqcb0_t, q_key)
    memwr.d     r5, d.mod_qp.qkey_dest_qpn

    //Invoke rqcb1
    add         r4, r2, (CB_UNIT_SIZE_BYTES)
    add         r5, r4, FIELD_OFFSET(rqcb1_t, q_key)
    memwr.d     r5, d.mod_qp.qkey_dest_qpn

    //Invoke sqcb2
    add         r4, r1, (CB_UNIT_SIZE_BYTES * 2)
    add         r5, r4, FIELD_OFFSET(sqcb2_t, q_key)
    memwr.d     r5, d.mod_qp.qkey_dest_qpn

    b           prepare_feedback
    nop

prepare_feedback:

    CAPRI_RESET_TABLE_0_ARG()
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, rdma_aq_tx_feedback_process, r0)

    nop.e
    nop         //Exit Slot
                                  
exit: 
    phvwr       p.common.p4_intr_global_drop, 1
    nop.e       
    nop         //Exit Slot
