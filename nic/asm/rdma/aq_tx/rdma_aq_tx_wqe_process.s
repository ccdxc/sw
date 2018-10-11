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

#define PHV_GLOBAL_COMMON_P phv_global_common
    
#define K_CQCB_BASE_ADDR_HI CAPRI_KEY_FIELD(IN_TO_S_P, cqcb_base_addr_hi)
#define K_SQCB_BASE_ADDR_HI CAPRI_KEY_FIELD(IN_TO_S_P, sqcb_base_addr_hi) 
#define K_RQCB_BASE_ADDR_HI CAPRI_KEY_FIELD(IN_TO_S_P, rqcb_base_addr_hi)
#define K_LOG_NUM_CQ_ENTRIES CAPRI_KEY_FIELD(IN_TO_S_P, log_num_cq_entries)    
%%

    .param      tx_dummy
    .param      rdma_cq_tx_stage0
    .param      rdma_req_tx_stage0    
    .param      rdma_aq_tx_feedback_process
    .param      rdma_aq_tx_modify_qp_2_process
.align
rdma_aq_tx_wqe_process:

    add         r1, r0, d.op

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
        b           dereg_mr
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

    //TODO: Need to add host_addr bit after Madhav's checkin
    KEY_INDEX_GET(r1, d.{id_ver}.wx)
    KEY_USER_KEY_GET(r2, d.{id_ver}.wx)
    
    phvwrpair   p.key.user_key, r2, p.key.state, KEY_STATE_VALID
    phvwrpair   p.key.type, MR_TYPE_MR, p.key.acc_ctrl, d.dbid_flags[15:8]
    add         r3, d.{mr.length}.dx, r0
    phvwr       p.key.len, r3[31:0]
    phvwr       p.key.log_page_size, d.mr.page_size_log2

    //TODO: ideally host_addr bit should come from driver.
    phvwr       p.key.host_addr, 1
    add         r2, d.{mr.tbl_index}.wx, r0
    add         r3, d.{mr.va}.dx, r0
    phvwrpair   p.key.base_va, r3, p.key.pt_base, r2
    add         r2, d.{mr.pd_id}.wx, r0
    phvwrpair   p.key.pd, r2, p.key.flags, 0x0B
    phvwrpair   p.key.mr_l_key, 0, p.key.mr_cookie, 0

    KT_BASE_ADDR_GET2(r3, r4)
    // XXX: Use KEY_ENTRY_ADDR_GET after lkey format update
    // key_entry_p = key_base_addr + (lkey_index * sizeof(struct key_entry_t))
    add         r4, r3, r1, LOG_SIZEOF_KEY_ENTRY_T
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_MR_KT_UPDATE)
    DMA_PHV2MEM_SETUP(r6, c2, key, key, r4)

    // pt_seg_size = HBM_NUM_PT_ENTRIES_PER_CACHE_LINE * lkey->hostmem_page_size
    sll         r4, HBM_NUM_PT_ENTRIES_PER_CACHE_LINE, d.mr.page_size_log2
    sub         r4, r4, 1
    // pt_seg_offset = lkey->base_va % pt_seg_size
    and         r4, d.{mr.va}.dx, r4
    //start_page_id = pt_seg_offset >> lkey->hostmem_page_size
    srl         r4, r4, d.mr.page_size_log2

    // hbm_add = (start_page_id + lkey->pt_base) * 8 + (pt_base_addr)
    PT_BASE_ADDR_GET2(r2)
    add         r4, r4, d.{mr.tbl_index}.wx
    add         r5, r2, r4, CAPRI_LOG_SIZEOF_U64
    
    add         r3, r0, d.{mr.map_count}.wx, CAPRI_LOG_SIZEOF_U64
    beqi        r3, 1<<CAPRI_LOG_SIZEOF_U64, mr_skip_dma_pt
    nop
    
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_MR_PT_SRC)
    DMA_HOST_MEM2MEM_SRC_SETUP(r6, r3, d.{mr.dma_addr}.dx)
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_MR_PT_DST)

    DMA_HBM_MEM2MEM_DST_SETUP(r6, r3, r5)
    b           mr_no_skip_dma_pt
    nop

mr_skip_dma_pt:

    //copy      the phy address of a single page directly.
    //TODO: how     do we ensure this memwr is completed by the time we generate CQ for admin cmd.
    memwr.d    r5, d.mr.dma_addr //BD slot

mr_no_skip_dma_pt: 

    b           prepare_feedback
    nop

dereg_mr:

    KEY_INDEX_GET(r1, d.{id_ver}.wx)

    // Assumption is p.key structure is already zeroed out
    KT_BASE_ADDR_GET2(r3, r4)
    add         r4, r3, r1, LOG_SIZEOF_KEY_ENTRY_T
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_MR_KT_UPDATE)
    DMA_PHV2MEM_SETUP(r6, c2, key, key, r4)
    
    b           prepare_feedback
    nop
    
create_cq:

    phvwr   p.{cqcb.intrinsic.host_rings, cqcb.intrinsic.total_rings}, (MAX_CQ_RINGS<<4|MAX_CQ_DOORBELL_RINGS)

   //TODO: Need to find a way to initiali pt_pa and pt_next_pa

    //compute the offset of the label of CQ program
    addi        r4, r0, rdma_cq_tx_stage0[33:CAPRI_RAW_TABLE_PC_SHIFT] ;
    addi        r3, r0, tx_dummy[33:CAPRI_RAW_TABLE_PC_SHIFT] ;
    sub         r4, r4, r3
    phvwr       p.cqcb.intrinsic.pc, r4

    //          setup the DMA for CQCB
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_CREATE_CQ_CB)        
    add         r2, r0, d.{id_ver}.wx  //TODO: Need to optimize
    AQ_TX_CQCB_ADDR_GET(r1, r2[23:0], K_CQCB_BASE_ADDR_HI)

    DMA_PHV2MEM_SETUP(r6, c1, cqcb, cqcb, r1)
    
    // r3 will have the pt_base_address where pt translations
    // should be copied to
    PT_BASE_ADDR_GET2(r4) 
    add         r3, r4, d.{cq.tbl_index}.wx, CAPRI_LOG_SIZEOF_U64
    srl         r5, r3, CAPRI_LOG_SIZEOF_U64
    phvwrpair   p.cqcb.pt_base_addr, r5, p.cqcb.log_cq_page_size, d.cq.page_size_log2[4:0]
    phvwrpair   p.cqcb.log_wqe_size, d.cq.stride_log2[4:0], p.cqcb.log_num_wqes, d.cq.depth_log2[4:0]
    add         r2, r0, d.{id_ver}.wx  //TODO: Need to optimize
    phvwr       p.cqcb.cq_id, r2[23:0]
    phvwr       p.cqcb.eq_id, d.cq.eq_id[23:0]

//TODO:  host_addr should come from driver
    phvwr       p.cqcb.host_addr, 1
    
    add         r4, r0, d.{cq.map_count}.wx, CAPRI_LOG_SIZEOF_U64
    beqi        r4, 1<<CAPRI_LOG_SIZEOF_U64, cq_skip_dma_pt
    nop
    
    //Setup DMA to copy PT translations from host to HBM
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_CREATE_CQ_PT_SRC)    

    DMA_HOST_MEM2MEM_SRC_SETUP(r6, r4, d.{cq.dma_addr}.dx)
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_CREATE_CQ_PT_DST)        
    DMA_HBM_MEM2MEM_DST_SETUP(r6, r4, r3)

    //TODO: There   is a race condition here. DMA of CQCB and DMA of cqcb->pt_pa
    //Setup     DMA for first two translations in cqcb for optimized lookup
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_CREATE_CQCB_PT_SRC)    

    DMA_HOST_MEM2MEM_SRC_SETUP(r6, (2*CAPRI_SIZEOF_U64_BYTES), d.{cq.dma_addr}.dx)
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_CREATE_CQCB_PT_DST)        

    add r1, r1, FIELD_OFFSET(cqcb_t, pt_pa)
    DMA_HBM_MEM2MEM_DST_SETUP(r6, (2*CAPRI_SIZEOF_U64_BYTES), r1)

    phvwrpair   p.cqcb.pt_pg_index, 0, p.cqcb.pt_next_pg_index, 1
    
    b           cq_no_skip_dma_pt
    nop

cq_skip_dma_pt:

    //copy      the phy address of a single page directly.
    //TODO: how     do we ensure this memwr is completed by the time we generate CQ for admin cmd.
    memwr.d    r3, d.cq.dma_addr //BD slot
    phvwr       p.cqcb.pt_pa, d.cq.dma_addr
    phvwrpair   p.cqcb.pt_pg_index, 0, p.cqcb.pt_next_pg_index, 0x1ff
    
cq_no_skip_dma_pt: 
        
    b           prepare_feedback
    nop
    
create_qp:

    // SQCB0:
    
    phvwr       p.{sqcb0.intrinsic.host_rings, sqcb0.intrinsic.total_rings}, (MAX_SQ_HOST_RINGS<<4|MAX_SQ_DOORBELL_RINGS)
    phvwr       p.sqcb0.log_num_wqes, d.qp.sq_depth_log2[4:0]
    

    // TODO: For now setting it to RTS, but later change it to INIT
    // state. modify_qp is supposed to set it to RTR and RTS.
    phvwr       p.sqcb0.state, QP_STATE_RESET
    phvwr       p.sqcb0.color, 1

    //TODO: SQ in HBM still need to be implemented

    phvwr       p.sqcb0.log_sq_page_size, d.qp.sq_page_size_log2[4:0]
    phvwr       p.sqcb0.log_wqe_size, d.qp.sq_stride_log2[4:0]
    phvwr       p.sqcb0.pd, d.{qp.pd_id}.wx
    phvwr       p.sqcb0.service, d.type_state
    
    // SQCB1:

    add         r2, d.{qp.sq_cq_id}.wx, r0
    phvwr       p.sqcb1.cq_id, r2[23:0]
    phvwr       p.sqcb1.state, QP_STATE_RTS
    phvwr       p.sqcb1.pd, d.{qp.pd_id}.wx
    
    phvwr       p.sqcb1.service, d.type_state[3:0]
    phvwr       p.sqcb1.ssn, 1
    phvwr       p.sqcb1.max_ssn, 1
    
    //infinite  retries                 
    phvwr       p.sqcb1.credits, 0x1F
    phvwr       p.{sqcb1.err_retry_count, sqcb1.rnr_retry_count}, (0x7<<3|0x7)

    //SQCB2:

    phvwr       p.sqcb2.log_sq_size, d.qp.sq_depth_log2[4: 0]
    phvwr       p.sqcb2.ssn, 1
    // TODO Default should enable credits and set as part of connection negotiation
    phvwr       p.sqcb2.disable_credits, 1
    phvwrpair   p.{sqcb2.err_retry_ctr, sqcb2.rnr_retry_ctr}, (0x7<<3|0x7), p.sqcb2.lsn, 0
    phvwrpair   p.sqcb2.lsn_tx, 0, p.sqcb2.lsn_rx, 0

    //          TODO: Move RSQ/RRQ allocation to modify_qp frm create_qp
    //          TODO: Move pmtu setup to modify_qp
    
    //populate the PC in SQCB0, SQCB1
    addi        r4, r0, rdma_req_tx_stage0[33:CAPRI_RAW_TABLE_PC_SHIFT] ;
    addi        r3, r0, tx_dummy[33:CAPRI_RAW_TABLE_PC_SHIFT] ;
    sub         r4, r4, r3
    phvwr       p.sqcb0.intrinsic.pc, r4
    phvwr       p.sqcb1.pc, r4

    PT_BASE_ADDR_GET2(r4) 
    add         r3, r4, d.{qp.sq_tbl_index_xrcd_id}.wx, CAPRI_LOG_SIZEOF_U64
    srl         r5, r3, CAPRI_LOG_SIZEOF_U64
    phvwr       p.sqcb0.pt_base_addr, r5

    add         r4, r0, d.{qp.sq_map_count}.wx, CAPRI_LOG_SIZEOF_U64
    beqi        r4, 1<<CAPRI_LOG_SIZEOF_U64, qp_skip_dma_pt
    nop
    
    //Setup     DMA for SQ PT

    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_CREATE_QP_SQPT_SRC)

    DMA_HOST_MEM2MEM_SRC_SETUP(r6, r4, d.{qp.sq_dma_addr}.dx)
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_CREATE_QP_SQPT_DST)
    DMA_HBM_MEM2MEM_DST_SETUP(r6, r4, r3)

    b           qp_no_skip_dma_pt
    nop

qp_skip_dma_pt:

    //copy      the phy address of a single page directly.
    //TODO: how     do we ensure this memwr is completed by the time we generate CQ for admin cmd.
    memwr.d    r3, d.qp.sq_dma_addr //BD slot

qp_no_skip_dma_pt: 
    
    // setup DMA for SQCB
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_CREATE_QP_CB)        

    add         r2, r0, d.{id_ver}.wx  //TODO: Need to optimize
    SQCB_ADDR_GET(r1, r2[23:0], K_SQCB_BASE_ADDR_HI)

    DMA_PHV2MEM_SETUP(r6, c1, sqcb0, sqcb2, r1)

    add         r2, d.{qp.rq_cq_id}.wx, r0
    add         r2, r2[23:0], r0
    phvwrpair   p.rdma_feedback.aq_completion.op, AQ_OP_TYPE_CREATE_QP, p.rdma_feedback.create_qp.rq_cq_id, r2
    phvwrpair   p.rdma_feedback.create_qp.rq_depth_log2, d.qp.rq_depth_log2, p.rdma_feedback.create_qp.rq_stride_log2, d.qp.rq_stride_log2
    add         r2, d.{qp.pd_id}.wx, r0
    phvwrpair   p.rdma_feedback.create_qp.rq_page_size_log2, d.qp.rq_page_size_log2, p.rdma_feedback.create_qp.pd, r2
    phvwr       p.p4_to_p4plus.create_qp_ext.rq_dma_addr, d.{qp.rq_dma_addr}.dx
    phvwr       p.rdma_feedback.create_qp.rq_type_state, d.type_state
    phvwr       p.rdma_feedback.create_qp.rq_map_count, d.{qp.rq_map_count}.wx
    phvwr       p.rdma_feedback.create_qp.rq_tbl_index, d.{qp.rq_tbl_index_srq_id}.wx
    add         r2, r0, d.{id_ver}.wx  //TODO: Need to optimize
    phvwr p.p4_to_p4plus.create_qp_ext.rq_id, r2[23:0]

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
    .brcase     AQ_STATS_DUMP_TYPE_CQ
        b           cq_dump
        nop
    .brcase     AQ_STATS_DUMP_TYPE_EQ
        b           eq_dump
        nop
    .brcase     AQ_STATS_DUMP_TYPE_PT
        b           pt_dump
        nop
    .brcase     AQ_STATS_DUMP_TYPE_KT
        b           kt_dump
        nop
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

pt_dump:

    PT_BASE_ADDR_GET2(r4)
    add         r4, r4, d.{id_ver}.wx, CAPRI_LOG_SIZEOF_U64 

    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_STATS_DUMP_1)
    DMA_HBM_MEM2MEM_SRC_SETUP(r6, PAGE_SIZE_4K, r4)
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_STATS_DUMP_2)
    DMA_HOST_MEM2MEM_DST_SETUP(r6, PAGE_SIZE_4K, d.{stats.dma_addr}.dx) 
    
    b           prepare_feedback
    nop

kt_dump:
    
    b           prepare_feedback
    nop

qp_dump:

    add         r3, r0, d.{id_ver}.wx  //TODO: Need to optimize 
    SQCB_ADDR_GET(r1, r3[23:0], K_SQCB_BASE_ADDR_HI)
    RQCB_ADDR_GET(r2, r3[23:0], K_RQCB_BASE_ADDR_HI)

    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_STATS_DUMP_1)
    DMA_HBM_MEM2MEM_SRC_SETUP(r6, CB3_OFFSET_BYTES, r1)
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_STATS_DUMP_2)
    DMA_HOST_MEM2MEM_DST_SETUP(r6, CB3_OFFSET_BYTES, d.{stats.dma_addr}.dx) 

    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_STATS_DUMP_3)
    DMA_HBM_MEM2MEM_SRC_SETUP(r6, CB3_OFFSET_BYTES, r2)
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_STATS_DUMP_4)
    add         r3, r3, CB3_OFFSET_BYTES
    DMA_HOST_MEM2MEM_DST_SETUP(r6, CB3_OFFSET_BYTES, d.{stats.dma_addr}.dx)

    b           prepare_feedback
    nop

cq_dump:

    add         r3, r0, d.{id_ver}.wx  //TODO: Need to optimize
    CQCB_ADDR_GET(r1, r3[23:0], K_CQCB_BASE_ADDR_HI)

    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_STATS_DUMP_1)
    DMA_HBM_MEM2MEM_SRC_SETUP(r6, CB_UNIT_SIZE_BYTES, r1)
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_STATS_DUMP_2)
    DMA_HOST_MEM2MEM_DST_SETUP(r6, CB_UNIT_SIZE_BYTES, d.{stats.dma_addr}.dx)
    
    b           prepare_feedback
    nop
    
eq_dump:

    add         r3, r0, d.{id_ver}.wx  //TODO: Need to optimize
    EQCB_ADDR_GET(r1, r2, r3[23:0], K_CQCB_BASE_ADDR_HI, K_LOG_NUM_CQ_ENTRIES)

    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_STATS_DUMP_1)
    DMA_HBM_MEM2MEM_SRC_SETUP(r6, CB_UNIT_SIZE_BYTES, r1)
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_STATS_DUMP_2)
    DMA_HOST_MEM2MEM_DST_SETUP(r6, CB_UNIT_SIZE_BYTES, d.{stats.dma_addr}.dx)
    
    b           prepare_feedback
    nop
    
modify_qp:

    add         r3, r0, d.{id_ver}.wx  //TODO: Need to optimize 
    SQCB_ADDR_GET(r1, r3[23:0], K_SQCB_BASE_ADDR_HI)
    RQCB_ADDR_GET(r2, r3[23:0], K_RQCB_BASE_ADDR_HI)
    
dst_qp:
    bbne        d.mod_qp.attr_mask[RDMA_UPDATE_QP_OPER_SET_DEST_QPN], 1, e_psn

    // Invoke sqcb2 for QP
    add         r4, r1, (CB_UNIT_SIZE_BYTES * 2) //BD Slot
    add         r4, r4, FIELD_OFFSET(sqcb2_t, dst_qp)
    add         r5, d.{mod_qp.qkey_dest_qpn}.wx, r0
    memwr.h     r4, r5[15:8]
    add         r4, r4, 2
    memwr.b     r4, r5[7:0]

    add         r5, r2, r0
    add         r5, r5, FIELD_OFFSET(rqcb0_t, dst_qp)
    add         r4, d.{mod_qp.qkey_dest_qpn}.wx, r0
    memwr.h     r5, r4[15:8]
    add         r5, r5, 2
    memwr.b     r5, r4[7:0]

e_psn:
    bbne        d.mod_qp.attr_mask[RDMA_UPDATE_QP_OPER_SET_RQ_PSN], 1, q_key

    // Invoke rqcb1
    add         r5, r2, (CB_UNIT_SIZE_BYTES) //BD Slot
    add         r5, r5, FIELD_OFFSET(rqcb1_t, e_psn)
    memwr.w     r5, d.{mod_qp.rq_psn}.wx

q_key:
    bbne        d.mod_qp.attr_mask[RDMA_UPDATE_QP_OPER_SET_QKEY], 1, mod_qp_done

    //Invoke rqcb0
    add         r4, r2, r0
    add         r5, r4, FIELD_OFFSET(rqcb0_t, q_key)
    memwr.w     r5, d.{mod_qp.qkey_dest_qpn}.wx

    //Invoke rqcb1
    add         r4, r2, (CB_UNIT_SIZE_BYTES)
    add         r5, r4, FIELD_OFFSET(rqcb1_t, q_key)
    memwr.w     r5, d.{mod_qp.qkey_dest_qpn}.wx

    //Invoke sqcb2
    add         r4, r1, (CB_UNIT_SIZE_BYTES * 2)
    add         r5, r4, FIELD_OFFSET(sqcb2_t, q_key)
    memwr.w     r5, d.{mod_qp.qkey_dest_qpn}.wx

mod_qp_done:
    
    mfspr       r7, spr_tbladdr
    CAPRI_RESET_TABLE_1_ARG()
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_tx_modify_qp_2_process, r7) // BD slot

prepare_feedback:

    CAPRI_RESET_TABLE_0_ARG()
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, rdma_aq_tx_feedback_process, r0)

    nop.e
    nop         //Exit Slot
                                  
exit: 
    phvwr.e       p.common.p4_intr_global_drop, 1
    nop         //Exit Slot
