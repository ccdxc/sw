#ifndef __CAPRI_H
#define __CAPRI_H

#include "asic/cmn/asic_common.hpp"

#define CAPRI_NUM_STAGES    8
#define CAPRI_STAGE_FIRST   0
#define CAPRI_STAGE_LAST    (CAPRI_NUM_STAGES-1)

#define CAPRI_INT_ASSERT_DATA     0x01000000

// intrinsic fields
#define CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR k.{p4_rxdma_intr_qstate_addr_sbit0_ebit1...p4_rxdma_intr_qstate_addr_sbit2_ebit33}
#define CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR_WITH_SHIFT(_shift) k.{p4_rxdma_intr_qstate_addr_sbit0_ebit1...p4_rxdma_intr_qstate_addr_sbit2_ebit33[31:_shift]}
#define CAPRI_RXDMA_INTRINSIC_LIF k.{p4_intr_global_lif_sbit0_ebit2...p4_intr_global_lif_sbit3_ebit10}
#define CAPRI_RXDMA_INTRINSIC_QTYPE k.p4_rxdma_intr_qtype
#define CAPRI_RXDMA_INTRINSIC_QID k.p4_rxdma_intr_qid
#define CAPRI_RXDMA_INTRINSIC_QID_QTYPE k.{p4_rxdma_intr_qid...p4_rxdma_intr_qtype}
#define CAPRI_RXDMA_INTRINSIC_RECIRC_COUNT k.p4_intr_recirc_count
#define CAPRI_TXDMA_INTRINSIC_RECIRC_COUNT k.p4_intr_recirc_count

#define CAPRI_RXDMA_RETH_VA k.{rdma_bth_reth_reth_va_1_sbit0_ebit7...rdma_bth_reth_reth_va_2_sbit16_ebit31}

#define CAPRI_RXDMA_BTH_ATOMICETH_SWAP_OR_ADD_DATA(_r) \
    add _r, k.{rdma_bth_atomiceth_atomiceth_swap_or_add_data_sbit56_ebit63}, k.{rdma_bth_atomiceth_atomiceth_swap_or_add_data_sbit0_ebit15...rdma_bth_atomiceth_atomiceth_swap_or_add_data_sbit48_ebit55}, 8

#define CAPRI_RXDMA_RETH_DMA_LEN k.{rdma_bth_reth_reth_dma_len1...rdma_bth_reth_reth_dma_len2}
#define CAPRI_RXDMA_RETH_R_KEY k.{rdma_bth_reth_reth_r_key_sbit0_ebit23...rdma_bth_reth_reth_r_key_sbit24_ebit31}

#define CAPRI_RXDMA_RETH_VA_R_KEY k.{rdma_bth_reth_reth_va_1_sbit0_ebit7...rdma_bth_reth_reth_r_key_sbit24_ebit31}
#define CAPRI_RXDMA_RETH_VA_R_KEY_LEN k.{rdma_bth_reth_reth_va_1_sbit0_ebit7...rdma_bth_reth_reth_dma_len2}

#define CAPRI_RXDMA_BTH_IMMETH_IMMDATA  k.{rdma_bth_immeth_immeth_data_sbit0_ebit7...rdma_bth_immeth_immeth_data_sbit24_ebit31}
#define CAPRI_RXDMA_BTH_RETH_IMMETH_IMMDATA_C(_r, _c_flag) \
    add._c_flag _r,  k.{rdma_bth_reth_immeth_immeth_data_sbit24_ebit31}, k.{rdma_bth_reth_immeth_immeth_data_sbit0_ebit15...rdma_bth_reth_immeth_immeth_data_sbit16_ebit23}, 8

#define CAPRI_RXDMA_BTH_IETH_R_KEY k.{rdma_bth_ieth_ieth_r_key_sbit0_ebit7...rdma_bth_ieth_ieth_r_key_sbit24_ebit31}

#define CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}
#define CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR_WITH_SHIFT(_shift) k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33[31:_shift]}
#define CAPRI_TXDMA_INTRINSIC_LIF k.{p4_intr_global_lif_sbit0_ebit2...p4_intr_global_lif_sbit3_ebit10}
#define CAPRI_TXDMA_INTRINSIC_QTYPE k.p4_txdma_intr_qtype
#define CAPRI_TXDMA_INTRINSIC_QID k.p4_txdma_intr_qid

// app data fields
#define CAPRI_APP_DATA_RAW_FLAGS k.rdma_bth_raw_flags
#define CAPRI_APP_DATA_RAW_FLAG_UD k.rdma_bth_raw_flags[15]
#define CAPRI_APP_DATA_RAW_FLAG_ERR_DIS_QP k.rdma_bth_raw_flags[0]
#define CAPRI_APP_DATA_PAYLOAD_LEN k.{rdma_bth_payload_len_sbit0_ebit5...rdma_bth_payload_len_sbit6_ebit13}

#define CAPRI_APP_DATA_ROCE_OPT_TS_VALID k.{rdma_bth_roce_opt_ts_valid}
#define CAPRI_APP_DATA_ROCE_OPT_TS_VALUE_AND_ECHO k.{rdma_bth_roce_opt_ts_value_sbit0_ebit15...rdma_bth_roce_opt_ts_echo_sbit16_ebit31}
#define CAPRI_APP_DATA_ROCE_OPT_MSS k.{rdma_bth_roce_opt_mss}

#define CAPRI_RECIRC_REASON_NONE                    0
#define CAPRI_RECIRC_REASON_INORDER_WORK_NOT_DONE   1
#define CAPRI_RECIRC_REASON_INORDER_WORK_DONE       2
#define CAPRI_RECIRC_REASON_SGE_WORK_PENDING        3
#define CAPRI_RECIRC_REASON_ATOMIC_RNR              4
#define CAPRI_RECIRC_REASON_ERROR_DISABLE_QP        5

#define CAPRI_APP_DATA_RECIRC_REASON k.rdma_recirc_recirc_reason
#define CAPRI_APP_DATA_RECIRC_TOKEN_ID k.rdma_recirc_token_id
#define CAPRI_APP_DATA_RECIRC_SPEC_CINDEX  k.rdma_recirc_recirc_spec_cindex
#define CAPRI_APP_DATA_RECIRC_ITER_COUNT k.rdma_recirc_recirc_iter_count

//BTH header fields
#define CAPRI_APP_DATA_BTH_OPCODE k.rdma_bth_bth_opcode
#define CAPRI_APP_DATA_BTH_PAD k.rdma_bth_bth_pad
#define CAPRI_APP_DATA_BTH_PSN k.{rdma_bth_bth_psn_sbit0_ebit7...rdma_bth_bth_psn_sbit16_ebit23}
#define CAPRI_APP_DATA_AETH_MSN k.{rdma_bth_aeth_aeth_msn_sbit0_ebit15...rdma_bth_aeth_aeth_msn_sbit16_ebit23}
#define CAPRI_APP_DATA_AETH_SYNDROME k.rdma_bth_aeth_aeth_syndrome
#define CAPRI_APP_DATA_BTH_ACK_REQ  k.rdma_bth_bth_a
#define CAPRI_APP_DATA_BTH_SE k.rdma_bth_bth_se
#define CAPRI_APP_DATA_BTH_P_KEY k.rdma_bth_bth_pkey

//DETH header fields
#define CAPRI_RXDMA_DETH_Q_KEY                k.{rdma_bth_deth_deth_q_key_sbit0_ebit7...rdma_bth_deth_deth_q_key_sbit24_ebit31}
#define CAPRI_RXDMA_DETH_SRC_QP               k.{rdma_bth_deth_deth_src_qp_sbit0_ebit7...rdma_bth_deth_deth_src_qp_sbit8_ebit23}
#define CAPRI_RXDMA_DETH_IMMETH_DATA          k.{rdma_bth_deth_immeth_immeth_data_sbit0_ebit23...rdma_bth_deth_immeth_immeth_data_sbit24_ebit31}
#define CAPRI_RXDMA_DETH_SMAC(_r) \
   add _r, k.{rdma_bth_deth_smac_sbit40_ebit47}, k.{rdma_bth_deth_smac_sbit0_ebit15...rdma_bth_deth_smac_sbit32_ebit39}, 8
#define CAPRI_RXDMA_DETH_IMMETH_SMAC1(_r) \
   add _r, k.{rdma_bth_deth_immeth_smac_1_sbit8_ebit15}, k.{rdma_bth_deth_immeth_smac_1_sbit0_ebit7}, 8

// Feedback header fields
#define CAPRI_FEEDBACK_FEEDBACK_TYPE k.rdma_completion_feedback_feedback_type

// Completion feedback header fields
#define CAPRI_COMPLETION_FEEDBACK_WRID(_r) \
add _r, k.{rdma_completion_feedback_wrid_sbit32_ebit47...rdma_completion_feedback_wrid_sbit56_ebit63}, k.{rdma_completion_feedback_wrid_sbit0_ebit7...rdma_completion_feedback_wrid_sbit24_ebit31}, 32

#define CAPRI_COMPLETION_FEEDBACK_OPTYPE k.rdma_completion_feedback_optype
#define CAPRI_COMPLETION_FEEDBACK_STATUS k.rdma_completion_feedback_status
#define CAPRI_COMPLETION_FEEDBACK_ERROR k.rdma_completion_feedback_error
#define CAPRI_COMPLETION_FEEDBACK_ERR_QP_INSTANTLY k.rdma_completion_feedback_err_qp_instantly
#define CAPRI_COMPLETION_FEEDBACK_LIF_STATS_INFO k.{rdma_completion_feedback_lif_cqe_error_id_vld, rdma_completion_feedback_lif_error_id_vld, rdma_completion_feedback_lif_error_id}
#define CAPRI_COMPLETION_FEEDBACK_SSN k.{rdma_completion_feedback_ssn_sbit0_ebit7...rdma_completion_feedback_ssn_sbit8_ebit23}
#define CAPRI_COMPLETION_FEEDBACK_TX_PSN k.{rdma_completion_feedback_tx_psn_sbit0_ebit7...rdma_completion_feedback_tx_psn_sbit16_ebit23}

// Timer expiry feedback header fields
#define CAPRI_TIMER_EXPIRY_FEEDBACK_SSN(_r) \
    add _r, k.rdma_timer_expiry_feedback_ssn_sbit8_ebit23, k.rdma_timer_expiry_feedback_ssn_sbit0_ebit7, 16
#define CAPRI_TIMER_EXPIRY_FEEDBACK_REXMIT_PSN \
    k.{rdma_timer_expiry_feedback_rexmit_psn_sbit0_ebit7...rdma_timer_expiry_feedback_rexmit_psn_sbit8_ebit23}
#define CAPRI_TIMER_EXPIRY_FEEDBACK_TX_PSN \
    k.{rdma_timer_expiry_feedback_tx_psn_sbit0_ebit7...rdma_timer_expiry_feedback_tx_psn_sbit16_ebit23}
#define CAPRI_SQ_DRAIN_FEEDBACK_SSN(_r) \
    add _r, k.rdma_sq_drain_feedback_ssn_sbit8_ebit23, k.rdma_sq_drain_feedback_ssn_sbit0_ebit7, 16
#define CAPRI_SQ_DRAIN_FEEDBACK_TX_PSN \
    k.{rdma_sq_drain_feedback_tx_psn_sbit0_ebit7...rdma_sq_drain_feedback_tx_psn_sbit8_ebit23}

#define CAPRI_RAW_TABLE_PC_SHIFT 6

#define INTRINSIC_RAW_K_T struct capri_intrinsic_raw_k_t
struct capri_intrinsic_raw_k_t {
    lock_en: 1;
    table_read_size: 3;
    table_pc: 28;
    table_addr: 64;
};

// TODO: Verify the accuracy
#define INTRINSIC_S0_K_T struct capri_intrinsic_s0_k_t
struct capri_intrinsic_s0_k_t {
    lock_en: 1;
    table_read_size: 3;
    table_pc: 28;
};

struct capri_intrinsic_qstate_t {
    pc: 8;
    rsvd: 8;
    cosB: 4;
    cosA: 4;
    cos_selector: 8;
    eval_last: 8;
    total_rings: 4;
    host_rings: 4;
    pid: 16;
};

struct capri_intrinsic_ring_t {
    pindex: 16;
    cindex: 16;
};

#define BITS_PER_BYTE 8
#define LOG_BITS_PER_BYTE 3 // 2^3 = BITS_PER_BYTE)
#define BYTE_OFFSETOF(_struct, _field) offsetof(struct _struct, _field)/BITS_PER_BYTE
#define BYTE_SIZEOF(_struct, _field)   sizeof(struct _struct, _field)/BITS_PER_BYTE

#define CAPRI_SEG_PAGE_MASK 0x7
#define CAPRI_LOG_SIZEOF_U64 3  // 2^3 = 8 bytes = sizeof(u64)
#define CAPRI_LOG_SIZEOF_U64_BITS (CAPRI_LOG_SIZEOF_U64 + LOG_BITS_PER_BYTE) // 2^6 = 64 bits = 8 bytes = sizeof(u64)
#define CAPRI_SIZEOF_U64_BITS 64
#define CAPRI_SIZEOF_U64_BYTES 8

#define CAPRI_FLIT_SIZE         64 // Bytes
#define CAPRI_FLIT_SIZE_BITS    (CAPRI_FLIT_SIZE * BITS_PER_BYTE)


#define STAGE_0     0
#define STAGE_1     1
#define STAGE_2     2
#define STAGE_3     3
#define STAGE_4     4
#define STAGE_5     5
#define STAGE_6     6
#define STAGE_7     7

// Table related defines
#define TABLE_0     0
#define TABLE_1     1
#define TABLE_2     2
#define TABLE_3     3

#define CAPRI_TABLE_SIZE_8_BITS        0
#define CAPRI_TABLE_SIZE_16_BITS       1
#define CAPRI_TABLE_SIZE_32_BITS       2
#define CAPRI_TABLE_SIZE_64_BITS       3
#define CAPRI_TABLE_SIZE_128_BITS      4
#define CAPRI_TABLE_SIZE_256_BITS      5
#define CAPRI_TABLE_SIZE_512_BITS      6
#define CAPRI_TABLE_SIZE_0_BITS        7

#define CAPRI_TABLE_LOCK_EN    1
#define CAPRI_TABLE_LOCK_DIS   0

#define CAPRI_SET_TABLE_FIELD_LOCAL(_base_r, _struct_name, _field_name, _src) \
    tblwrp.l  _base_r, offsetof(_struct_name, _field_name), sizeof(_struct_name._field_name), _src;

#define CAPRI_SET_TABLE_FIELD_LOCAL_C(_base_r, _struct_name, _field_name, _src, _c_flag) \
    tblwrp._c_flag.l  _base_r, offsetof(_struct_name, _field_name), sizeof(_struct_name._field_name), _src;

#define CAPRI_SET_FIELD(_base_r, _struct_name, _field_name, _src) \
    phvwrp  _base_r, offsetof(_struct_name, _field_name), sizeof(_struct_name._field_name), _src;

#define CAPRI_SET_2_FIELDS(_base_r, _struct_name, _first_field_name, _second_field_name, _src) \
    phvwrp  _base_r, offsetof(_struct_name, _first_field_name), \
            sizeof(_struct_name._first_field_name) + sizeof(_struct_name._second_field_name), _src;

#define CAPRI_SET_FIELD_IMM(_base_r, _struct_name, _field_name, _immdata) \
    phvwrpi  _base_r, offsetof(_struct_name, _field_name), sizeof(_struct_name._field_name), _immdata;

#define CAPRI_TABLE_GET_FIELD(_dst_r, _base_r, _struct_name, _field_name) \
    tblrdp  _dst_r, _base_r, offsetof(_struct_name, _field_name), sizeof(_struct_name._field_name);

//get field conditionally
#define CAPRI_TABLE_GET_FIELD_C(_dst_r, _base_r, _struct_name, _field_name, _c_flag) \
    tblrdp._c_flag  _dst_r, _base_r, offsetof(_struct_name, _field_name), sizeof(_struct_name._field_name);

#define CAPRI_TABLE_SET_FIELD(_src_r, _base_r, _struct_name, _field_name) \
    tblwrp  _base_r, offsetof(_struct_name, _field_name), sizeof(_struct_name._field_name), _src_r;

//set field conditionally
#define CAPRI_SET_FIELD_C(_base_r, _struct_name, _field_name, _src, _c_flag) \
    phvwrp._c_flag  _base_r, offsetof(_struct_name, _field_name), sizeof(_struct_name._field_name), _src;

#define CAPRI_SIZEOF_RANGE(_s, _f1, _fn) \
    (offsetof(_s, _f1) + sizeof(_s._f1) - offsetof(_s, _fn))

// f1 is the most significant field and fn is the least significant field in big-endian format
#define CAPRI_SET_FIELD_RANGE(_base_r, _s, _f1, _fn, _src_range)    \
    phvwrp  _base_r, offsetof(_s, _fn), CAPRI_SIZEOF_RANGE(_s, _f1, _fn), _src_range

#define CAPRI_SET_FIELD_RANGE_C(_base_r, _s, _f1, _fn, _src_range, _c_flag)    \
    phvwrp._c_flag  _base_r, offsetof(_s, _fn), CAPRI_SIZEOF_RANGE(_s, _f1, _fn), _src_range


// new macros

#define CAT7(a,b,c,d,e,f,g) a##b##c##d##e##f##g
#define CAT9(a,b,c,d,e,f,g,h,i) a##b##c##d##e##f##g##h##i
#define CAT3(a,b,c) a##b##c
#define CAT4(a,b,c,d) a##b##c##d

#define CAPRI_KEY_RANGE(_kp, _kf1, _kfn) \
    k.{CAT7(_kp, _, _kf1, ..., _kp, _, _kfn)}

#define CAPRI_PHV_RANGE(_pp, _pf1, _pfn) \
    p.{CAT9(common., _pp, _, _pf1, ..., common., _pp, _, _pfn)}

#define CAPRI_KEY_FIELD(_kp, _kf) \
    k.CAT3(_kp, _, _kf)

#define CAPRI_PHV_FIELD(_pp, _pf) \
    p.CAT4(common., _pp, _, _pf)

#define CAPRI_SET_FIELD2(_pp, _pf, _src) \
    phvwr   p.CAT4(common., _pp, _, _pf), _src

#define CAPRI_SET_FIELD2_C(_pp, _pf, _src, _c_flag) \
    phvwr._c_flag   p.CAT4(common., _pp, _, _pf), _src

#define CAPRI_SET_FIELD_RANGE2(_pp, _pf1, _pfn, _src_range) \
    phvwr   CAPRI_PHV_RANGE(_pp, _pf1, _pfn), _src_range

#define CAPRI_SET_FIELD_RANGE2_C(_pp, _pf1, _pfn, _src_range, _c_flag) \
    phvwr._c_flag   CAPRI_PHV_RANGE(_pp, _pf1, _pfn), _src_range

#define CAPRI_SET_FIELD_RANGE2_IMM(_pp, _pf1, _pfn, _imm) \
    phvwr   CAPRI_PHV_RANGE(_pp, _pf1, _pfn), _imm

#define CAPRI_SET_FIELD_RANGE2_IMM_C(_pp, _pf1, _pfn, _imm, _c_flag) \
    phvwr._c_flag   CAPRI_PHV_RANGE(_pp, _pf1, _pfn), _imm


#define CAPRI_GET_TABLE_ADDR(_addr_r, _table_base_r, _eid, _entry_size_shift) \
    add  _addr_r, r0, _eid; \
    sll  _addr_r, _addr_r, _entry_size_shift; \
    add  _addr_r, _addr_r, _table_base_r;

#define CAPRI_GET_TABLE_ADDR_WITH_OFFSET(_addr_r, _table_base_r, _eid, _entry_size_shift, _state_offset) \
    CAPRI_GET_TABLE_ADDR(_addr_r, _table_base_r, _eid, _entry_size_shift); \
    addi _addr_r, _addr_r, _state_offset;

#define CAPRI_SET_RAW_TABLE_PC(_r, _pc) \
    addui _r, r0, hiword(_pc); \
    addi  _r, _r, loword(_pc); \

#define CAPRI_SET_RAW_TABLE_PC_C(_c, _r, _pc) \
    addui._c _r, r0, hiword(_pc); \
    addi._c  _r, _r, loword(_pc); \

#define CAPRI_NEXT_TABLE_I_READ_PC(_base_r, _lock_en, _table_read_size, _table_pc, _table_addr_r) \
    phvwrpi  _base_r, offsetof(INTRINSIC_RAW_K_T, table_read_size), 4, (_lock_en << 3)|(_table_read_size); \
    phvwrpi  _base_r, offsetof(INTRINSIC_RAW_K_T, table_pc), sizeof(INTRINSIC_RAW_K_T.table_pc), _table_pc[33:CAPRI_RAW_TABLE_PC_SHIFT]; \
    phvwrp  _base_r, offsetof(INTRINSIC_RAW_K_T, table_addr), sizeof(INTRINSIC_RAW_K_T.table_addr), _table_addr_r;

#define CAPRI_NEXT_TABLE_I_READ_PC_C(_base_r, _lock_en, _table_read_size, _table_pc1, _table_pc2, _table_addr_r, _c) \
    phvwrpi     _base_r, offsetof(INTRINSIC_RAW_K_T, table_read_size), 4, (_lock_en << 3)|(_table_read_size); \
    phvwrpi._c  _base_r, offsetof(INTRINSIC_RAW_K_T, table_pc), sizeof(INTRINSIC_RAW_K_T.table_pc), _table_pc1[33:CAPRI_RAW_TABLE_PC_SHIFT]; \
    phvwrpi.!_c _base_r, offsetof(INTRINSIC_RAW_K_T, table_pc), sizeof(INTRINSIC_RAW_K_T.table_pc), _table_pc2[33:CAPRI_RAW_TABLE_PC_SHIFT]; \
    phvwrp      _base_r, offsetof(INTRINSIC_RAW_K_T, table_addr), sizeof(INTRINSIC_RAW_K_T.table_addr), _table_addr_r;

#define CAPRI_NEXT_TABLE0_READ_PC(_lock_en, _table_read_size, _table_pc, _table_addr_r) \
    phvwrpair p.{common.common_te0_phv_table_lock_en...common.common_te0_phv_table_raw_table_size}, ((_lock_en << 3)|(_table_read_size)), p.common.common_te0_phv_table_addr, _table_addr_r; \
    phvwri    p.common.common_te0_phv_table_pc, _table_pc[33:CAPRI_RAW_TABLE_PC_SHIFT]; \
    CAPRI_SET_TABLE_0_VALID(1);

#define CAPRI_NEXT_TABLE0_READ_PC_E(_lock_en, _table_read_size, _table_pc, _table_addr_r) \
    phvwrpair p.{common.common_te0_phv_table_lock_en...common.common_te0_phv_table_raw_table_size}, ((_lock_en << 3)|(_table_read_size)), p.common.common_te0_phv_table_addr, _table_addr_r; \
    phvwri.e  p.common.common_te0_phv_table_pc, _table_pc[33:CAPRI_RAW_TABLE_PC_SHIFT]; \
    CAPRI_SET_TABLE_0_VALID(1);

#define CAPRI_NEXT_TABLE0_C_READ_PC_E(_lock_en1, _table_read_size1, _table_addr1_r, _lock_en2, _table_read_size2, _table_addr2_r, _table_pc, _c) \
    phvwrpair._c p.{common.common_te0_phv_table_lock_en...common.common_te0_phv_table_raw_table_size}, ((_lock_en1 << 3)|(_table_read_size1)), p.common.common_te0_phv_table_addr, _table_addr1_r; \
    phvwrpair.!_c p.{common.common_te0_phv_table_lock_en...common.common_te0_phv_table_raw_table_size}, ((_lock_en2 << 3)|(_table_read_size2)), p.common.common_te0_phv_table_addr, _table_addr2_r; \
    phvwri.e  p.common.common_te0_phv_table_pc, _table_pc[33:CAPRI_RAW_TABLE_PC_SHIFT]; \
    CAPRI_SET_TABLE_0_VALID(1);

#define CAPRI_NEXT_TABLE0_READ_PC_C(_lock_en, _table_read_size, _table_pc1, _table_pc2, _table_addr_r, _c) \
    phvwrpair p.{common.common_te0_phv_table_lock_en...common.common_te0_phv_table_raw_table_size}, ((_lock_en << 3)|(_table_read_size)), p.common.common_te0_phv_table_addr, _table_addr_r; \
    phvwri._c   p.common.common_te0_phv_table_pc, _table_pc1[33:CAPRI_RAW_TABLE_PC_SHIFT]; \
    phvwri.!_c  p.common.common_te0_phv_table_pc, _table_pc2[33:CAPRI_RAW_TABLE_PC_SHIFT]; \
    CAPRI_SET_TABLE_0_VALID(1);

#define CAPRI_NEXT_TABLE0_READ_PC_CE(_lock_en, _table_read_size, _table_pc1, _table_pc2, _table_addr_r, _c) \
    phvwri._c   p.common.common_te0_phv_table_pc, _table_pc1[33:CAPRI_RAW_TABLE_PC_SHIFT]; \
    phvwri.!_c  p.common.common_te0_phv_table_pc, _table_pc2[33:CAPRI_RAW_TABLE_PC_SHIFT]; \
    phvwrpair.e p.{common.common_te0_phv_table_lock_en...common.common_te0_phv_table_raw_table_size}, ((_lock_en << 3)|(_table_read_size)), p.common.common_te0_phv_table_addr, _table_addr_r; \
    CAPRI_SET_TABLE_0_VALID(1);

#define CAPRI_NEXT_TABLE1_READ_PC(_lock_en, _table_read_size, _table_pc, _table_addr_r) \
    phvwrpair p.{common.common_te1_phv_table_lock_en...common.common_te1_phv_table_raw_table_size}, ((_lock_en << 3)|(_table_read_size)), p.common.common_te1_phv_table_addr, _table_addr_r; \
    phvwri    p.common.common_te1_phv_table_pc, _table_pc[33:CAPRI_RAW_TABLE_PC_SHIFT]; \
    CAPRI_SET_TABLE_1_VALID(1);

#define CAPRI_NEXT_TABLE1_READ_PC_E(_lock_en, _table_read_size, _table_pc, _table_addr_r) \
    phvwrpair p.{common.common_te1_phv_table_lock_en...common.common_te1_phv_table_raw_table_size}, ((_lock_en << 3)|(_table_read_size)), p.common.common_te1_phv_table_addr, _table_addr_r; \
    phvwri.e  p.common.common_te1_phv_table_pc, _table_pc[33:CAPRI_RAW_TABLE_PC_SHIFT]; \
    CAPRI_SET_TABLE_1_VALID(1);

#define CAPRI_NEXT_TABLE1_READ_PC_C(_lock_en, _table_read_size, _table_pc1, _table_pc2, _table_addr_r, _c) \
    phvwrpair p.{common.common_te1_phv_table_lock_en...common.common_te1_phv_table_raw_table_size}, ((_lock_en << 3)|(_table_read_size)), p.common.common_te1_phv_table_addr, _table_addr_r; \
    phvwri._c   p.common.common_te1_phv_table_pc, _table_pc1[33:CAPRI_RAW_TABLE_PC_SHIFT]; \
    phvwri.!_c  p.common.common_te1_phv_table_pc, _table_pc2[33:CAPRI_RAW_TABLE_PC_SHIFT]; \
    CAPRI_SET_TABLE_1_VALID(1);

#define CAPRI_NEXT_TABLE1_READ_PC_CE(_lock_en, _table_read_size, _table_pc1, _table_pc2, _table_addr_r, _c) \
    phvwri._c   p.common.common_te1_phv_table_pc, _table_pc1[33:CAPRI_RAW_TABLE_PC_SHIFT]; \
    phvwri.!_c  p.common.common_te1_phv_table_pc, _table_pc2[33:CAPRI_RAW_TABLE_PC_SHIFT]; \
    phvwrpair.e p.{common.common_te1_phv_table_lock_en...common.common_te1_phv_table_raw_table_size}, ((_lock_en << 3)|(_table_read_size)), p.common.common_te1_phv_table_addr, _table_addr_r; \
    CAPRI_SET_TABLE_1_VALID(1);

#define CAPRI_NEXT_TABLE2_READ_PC(_lock_en, _table_read_size, _table_pc, _table_addr_r) \
    phvwrpair p.{common.common_te2_phv_table_lock_en...common.common_te2_phv_table_raw_table_size}, ((_lock_en << 3)|(_table_read_size)), p.common.common_te2_phv_table_addr, _table_addr_r; \
    phvwri    p.common.common_te2_phv_table_pc, _table_pc[33:CAPRI_RAW_TABLE_PC_SHIFT]; \
    CAPRI_SET_TABLE_2_VALID(1);

#define CAPRI_NEXT_TABLE2_READ_PC_E(_lock_en, _table_read_size, _table_pc, _table_addr_r) \
    phvwrpair p.{common.common_te2_phv_table_lock_en...common.common_te2_phv_table_raw_table_size}, ((_lock_en << 3)|(_table_read_size)), p.common.common_te2_phv_table_addr, _table_addr_r; \
    phvwri.e  p.common.common_te2_phv_table_pc, _table_pc[33:CAPRI_RAW_TABLE_PC_SHIFT]; \
    CAPRI_SET_TABLE_2_VALID(1);

#define CAPRI_NEXT_TABLE2_READ_PC_CE(_lock_en, _table_read_size, _table_pc1, _table_pc2, _table_addr_r, _c) \
    phvwri._c   p.common.common_te2_phv_table_pc, _table_pc1[33:CAPRI_RAW_TABLE_PC_SHIFT]; \
    phvwri.!_c  p.common.common_te2_phv_table_pc, _table_pc2[33:CAPRI_RAW_TABLE_PC_SHIFT]; \
    phvwrpair.e p.{common.common_te2_phv_table_lock_en...common.common_te2_phv_table_raw_table_size}, ((_lock_en << 3)|(_table_read_size)), p.common.common_te2_phv_table_addr, _table_addr_r; \
    CAPRI_SET_TABLE_2_VALID(1);

#define CAPRI_NEXT_TABLE3_READ_PC(_lock_en, _table_read_size, _table_pc, _table_addr_r) \
    phvwrpair p.{common.common_te3_phv_table_lock_en...common.common_te3_phv_table_raw_table_size}, ((_lock_en << 3)|(_table_read_size)), p.common.common_te3_phv_table_addr, _table_addr_r; \
    phvwri    p.common.common_te3_phv_table_pc, _table_pc[33:CAPRI_RAW_TABLE_PC_SHIFT]; \
    CAPRI_SET_TABLE_3_VALID(1);

#define CAPRI_NEXT_TABLE3_READ_PC_E(_lock_en, _table_read_size, _table_pc, _table_addr_r) \
    phvwrpair p.{common.common_te3_phv_table_lock_en...common.common_te3_phv_table_raw_table_size}, ((_lock_en << 3)|(_table_read_size)), p.common.common_te3_phv_table_addr, _table_addr_r; \
    phvwri.e  p.common.common_te3_phv_table_pc, _table_pc[33:CAPRI_RAW_TABLE_PC_SHIFT]; \
    CAPRI_SET_TABLE_3_VALID(1);

#define CAPRI_NEXT_TABLE_I_READ(_base_r, _lock_en, _table_read_size, _table_pc_r, _table_addr_r) \
    phvwrpi  _base_r, offsetof(INTRINSIC_RAW_K_T, table_read_size), 4, (_lock_en << 3)|(_table_read_size); \
    phvwrp  _base_r, offsetof(INTRINSIC_RAW_K_T, table_pc), sizeof(INTRINSIC_RAW_K_T.table_pc), _table_pc_r[63:CAPRI_RAW_TABLE_PC_SHIFT]; \
    phvwrp  _base_r, offsetof(INTRINSIC_RAW_K_T, table_addr), sizeof(INTRINSIC_RAW_K_T.table_addr), _table_addr_r;

#define CAPRI_NEXT_TABLE_I_READ_E(_base_r, _lock_en, _table_read_size, _table_pc_r, _table_addr_r) \
    phvwrpi  _base_r, offsetof(INTRINSIC_RAW_K_T, table_read_size), 4, (_lock_en << 3)|(_table_read_size); \
    phvwrp.e _base_r, offsetof(INTRINSIC_RAW_K_T, table_pc), sizeof(INTRINSIC_RAW_K_T.table_pc), _table_pc_r[63:CAPRI_RAW_TABLE_PC_SHIFT]; \
    phvwrp   _base_r, offsetof(INTRINSIC_RAW_K_T, table_addr), sizeof(INTRINSIC_RAW_K_T.table_addr), _table_addr_r;

#define CAPRI_NEXT_TABLE_I_READ_PC_C(_base_r, _lock_en, _table_read_size, _table_pc1, _table_pc2, _table_addr_r, _c) \
    phvwrpi  _base_r, offsetof(INTRINSIC_RAW_K_T, table_read_size), 4, (_lock_en << 3)|(_table_read_size); \
    phvwrpi._c  _base_r, offsetof(INTRINSIC_RAW_K_T, table_pc), sizeof(INTRINSIC_RAW_K_T.table_pc), _table_pc1[33:CAPRI_RAW_TABLE_PC_SHIFT]; \
    phvwrpi.!_c  _base_r, offsetof(INTRINSIC_RAW_K_T, table_pc), sizeof(INTRINSIC_RAW_K_T.table_pc), _table_pc2[33:CAPRI_RAW_TABLE_PC_SHIFT]; \
    phvwrp  _base_r, offsetof(INTRINSIC_RAW_K_T, table_addr), sizeof(INTRINSIC_RAW_K_T.table_addr), _table_addr_r;

#define CAPRI_NEXT_TABLE_I_READ_SET_SIZE(_base_r, _lock_en, _table_read_size) \
    phvwrpi  _base_r, offsetof(INTRINSIC_RAW_K_T, table_read_size), 4, (_lock_en << 3)|(_table_read_size);

#define CAPRI_NEXT_TABLE_I_READ_SET_SIZE_E(_base_r, _lock_en, _table_read_size) \
    phvwrpi.e  _base_r, offsetof(INTRINSIC_RAW_K_T, table_read_size), 4, (_lock_en << 3)|(_table_read_size);

#define CAPRI_NEXT_TABLE_I_READ_SET_SIZE_C(_base_r, _lock_en, _table_read_size, _c) \
    phvwrpi._c  _base_r, offsetof(INTRINSIC_RAW_K_T, table_read_size), 4, (_lock_en << 3)|(_table_read_size);

#define CAPRI_NEXT_TABLE_I_READ_SET_SIZE_PC(_base_r, _lock_en, _table_read_size, _table_pc) \
    phvwrpi  _base_r, offsetof(INTRINSIC_RAW_K_T, table_read_size), 4, (_lock_en << 3)|(_table_read_size); \
    phvwrpi  _base_r, offsetof(INTRINSIC_RAW_K_T, table_pc), sizeof(INTRINSIC_RAW_K_T.table_pc), _table_pc[33:CAPRI_RAW_TABLE_PC_SHIFT]; \

#define CAPRI_NEXT_TABLE_I_READ_SET_SIZE_PC_E(_base_r, _lock_en, _table_read_size, _table_pc) \
    phvwrpi.e  _base_r, offsetof(INTRINSIC_RAW_K_T, table_read_size), 4, (_lock_en << 3)|(_table_read_size); \
    phvwrpi  _base_r, offsetof(INTRINSIC_RAW_K_T, table_pc), sizeof(INTRINSIC_RAW_K_T.table_pc), _table_pc[33:CAPRI_RAW_TABLE_PC_SHIFT]; \

#define CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(_base_r, _lock_en, _table_read_size, _table_addr_r) \
    phvwrpi  _base_r, offsetof(INTRINSIC_RAW_K_T, table_read_size), 4, (_lock_en << 3)|(_table_read_size);\
    phvwrp  _base_r, offsetof(INTRINSIC_RAW_K_T, table_addr), sizeof(INTRINSIC_RAW_K_T.table_addr), _table_addr_r;

#define CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR_E(_base_r, _lock_en, _table_read_size, _table_addr_r) \
    phvwrpi.e  _base_r, offsetof(INTRINSIC_RAW_K_T, table_read_size), 4, (_lock_en << 3)|(_table_read_size);\
    phvwrp  _base_r, offsetof(INTRINSIC_RAW_K_T, table_addr), sizeof(INTRINSIC_RAW_K_T.table_addr), _table_addr_r;

#define CAPRI_SET_TABLE_0_VALID(_vld) \
    phvwri   p.common.app_header_table0_valid, _vld;
#define CAPRI_SET_TABLE_1_VALID(_vld) \
    phvwri   p.common.app_header_table1_valid, _vld;
#define CAPRI_SET_TABLE_2_VALID(_vld) \
    phvwri   p.common.app_header_table2_valid, _vld;
#define CAPRI_SET_TABLE_3_VALID(_vld) \
    phvwri   p.common.app_header_table3_valid, _vld;

#define CAPRI_SET_TABLE_0_1_VALID(_vld0, _vld1) \
    phvwri   p.{common.app_header_table0_valid...common.app_header_table1_valid}, ((_vld0 << 1) | (_vld1));

#define CAPRI_SET_TABLE_2_3_VALID(_vld2, _vld3) \
    phvwri   p.{common.app_header_table2_valid...common.app_header_table3_valid}, ((_vld2 << 1) | (_vld3));

#define CAPRI_SET_TABLE_0_VALID_C(_c, _vld) \
    phvwri._c    p.common.app_header_table0_valid, _vld;
#define CAPRI_SET_TABLE_1_VALID_C(_c, _vld) \
    phvwri._c    p.common.app_header_table1_valid, _vld;
#define CAPRI_SET_TABLE_2_VALID_C(_c, _vld) \
    phvwri._c    p.common.app_header_table2_valid, _vld;
#define CAPRI_SET_TABLE_3_VALID_C(_c, _vld) \
    phvwri._c    p.common.app_header_table3_valid, _vld;

#define CAPRI_SET_TABLE_0_VALID_CE(_c, _vld) \
    phvwri._c.e  p.common.app_header_table0_valid, _vld;
#define CAPRI_SET_TABLE_1_VALID_CE(_c, _vld) \
    phvwri._c.e  p.common.app_header_table1_valid, _vld;
#define CAPRI_SET_TABLE_2_VALID_CE(_c, _vld) \
    phvwri._c.e  p.common.app_header_table2_valid, _vld;
#define CAPRI_SET_TABLE_3_VALID_CE(_c, _vld) \
    phvwri._c.e  p.common.app_header_table3_valid, _vld;

#define CAPRI_SET_ALL_TABLES_VALID(_vld) \
    CAPRI_SET_TABLE_0_VALID(_vld); \
    CAPRI_SET_TABLE_1_VALID(_vld); \
    CAPRI_SET_TABLE_2_VALID(_vld); \
    CAPRI_SET_TABLE_3_VALID(_vld);

#define CAPRI_SET_TABLE_I_VALID(_tbl_id_r, _vld) \
    .brbegin; \
    br          _tbl_id_r[1:0]; \
    nop; \
    .brcase 0; \
        b _table_i_valid; \
        CAPRI_SET_TABLE_0_VALID(_vld); \
    .brcase 1; \
        b _table_i_valid; \
        CAPRI_SET_TABLE_1_VALID(_vld); \
    .brcase 2; \
        b _table_i_valid; \
        CAPRI_SET_TABLE_2_VALID(_vld); \
    .brcase 3; \
        b _table_i_valid; \
        CAPRI_SET_TABLE_3_VALID(_vld); \
    .brend; \
_table_i_valid:;

#define CAPRI_SET_TABLE_I_VALID_C(_c, _tbl_id_r, _vld) \
    .brbegin; \
    br          _tbl_id_r[1:0]; \
    nop; \
    .brcase 0; \
        b _table_i_valid_c; \
        CAPRI_SET_TABLE_0_VALID_C(_c, _vld); \
    .brcase 1; \
        b _table_i_valid_c; \
        CAPRI_SET_TABLE_1_VALID_C(_c, _vld); \
    .brcase 2; \
        b _table_i_valid_c; \
        CAPRI_SET_TABLE_2_VALID_C(_c, _vld); \
    .brcase 3; \
        b _table_i_valid_c; \
        CAPRI_SET_TABLE_3_VALID_C(_c, _vld); \
    .brend; \
_table_i_valid_c:;


#define CAPRI_GET_TABLE_I_K_AND_ARG(_phv_name, _tbl_id_r, _k_base_r, _arg_base_r) \
    .brbegin; \
    br      _tbl_id_r[1:0]; \
    nop; \
    .brcase 0; \
        add  _k_base_r, 0, offsetof(struct _phv_name, common.common_te0_phv_table_addr); \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_t0_s2s_s2s_data); \
        phvwr   p.common.common_t0_s2s_s2s_data, r0; \
        b _next; \
        CAPRI_SET_TABLE_0_VALID(1);  \
    .brcase 1; \
        add  _k_base_r, 0, offsetof(struct _phv_name, common.common_te1_phv_table_addr); \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_t1_s2s_s2s_data); \
        phvwr   p.common.common_t1_s2s_s2s_data, r0; \
        b _next; \
        CAPRI_SET_TABLE_1_VALID(1);  \
    .brcase 2; \
        add  _k_base_r, 0, offsetof(struct _phv_name, common.common_te2_phv_table_addr); \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_t2_s2s_s2s_data); \
        phvwr   p.common.common_t2_s2s_s2s_data, r0; \
        b _next; \
        CAPRI_SET_TABLE_2_VALID(1);  \
    .brcase 3; \
        add  _k_base_r, 0, offsetof(struct _phv_name, common.common_te3_phv_table_addr); \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_t3_s2s_s2s_data); \
        phvwr   p.common.common_t3_s2s_s2s_data, r0; \
        b _next; \
        CAPRI_SET_TABLE_3_VALID(1);  \
    .brend; \
_next:;

#define CAPRI_GET_TABLE_I2_K_AND_ARG(_phv_name, _tbl_id_r, _k_base_r, _arg_base_r) \
    .brbegin; \
    br      _tbl_id_r[1:0]; \
    nop; \
    .brcase 0; \
        add  _k_base_r, 0, offsetof(struct _phv_name, common.common_te0_phv_table_addr); \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_t0_s2s_s2s_data); \
        phvwr   p.common.common_t0_s2s_s2s_data, r0; \
        b _next2; \
        CAPRI_SET_TABLE_0_VALID(1);  \
    .brcase 1; \
        add  _k_base_r, 0, offsetof(struct _phv_name, common.common_te1_phv_table_addr); \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_t1_s2s_s2s_data); \
        phvwr   p.common.common_t1_s2s_s2s_data, r0; \
        b _next2; \
        CAPRI_SET_TABLE_1_VALID(1);  \
    .brcase 2; \
        add  _k_base_r, 0, offsetof(struct _phv_name, common.common_te2_phv_table_addr); \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_t2_s2s_s2s_data); \
        phvwr   p.common.common_t2_s2s_s2s_data, r0; \
        b _next2; \
        CAPRI_SET_TABLE_2_VALID(1);  \
    .brcase 3; \
        add  _k_base_r, 0, offsetof(struct _phv_name, common.common_te3_phv_table_addr); \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_t3_s2s_s2s_data); \
        phvwr   p.common.common_t3_s2s_s2s_data, r0; \
        b _next2; \
        CAPRI_SET_TABLE_3_VALID(1);  \
    .brend; \
_next2:;

#define CAPRI_GET_TABLE_0_K_NO_VALID(_phv_name, _k_base_r) \
    add     _k_base_r, 0, offsetof(struct _phv_name, common.common_te0_phv_table_addr);

#define CAPRI_GET_TABLE_1_K_NO_VALID(_phv_name, _k_base_r) \
    add     _k_base_r, 0, offsetof(struct _phv_name, common.common_te1_phv_table_addr);

#define CAPRI_GET_TABLE_2_K_NO_VALID(_phv_name, _k_base_r) \
    add     _k_base_r, 0, offsetof(struct _phv_name, common.common_te2_phv_table_addr);

#define CAPRI_GET_TABLE_3_K_NO_VALID(_phv_name, _k_base_r) \
    add     _k_base_r, 0, offsetof(struct _phv_name, common.common_te3_phv_table_addr);

#define CAPRI_GET_TABLE_0_K(_phv_name, _k_base_r) \
    CAPRI_GET_TABLE_0_K_NO_VALID(_phv_name, _k_base_r) \
    CAPRI_SET_TABLE_0_VALID(1);

#define CAPRI_GET_TABLE_1_K(_phv_name, _k_base_r) \
    CAPRI_GET_TABLE_1_K_NO_VALID(_phv_name, _k_base_r) \
    CAPRI_SET_TABLE_1_VALID(1);

#define CAPRI_GET_TABLE_2_K(_phv_name, _k_base_r) \
    CAPRI_GET_TABLE_2_K_NO_VALID(_phv_name, _k_base_r) \
    CAPRI_SET_TABLE_2_VALID(1);

#define CAPRI_GET_TABLE_3_K(_phv_name, _k_base_r) \
    CAPRI_GET_TABLE_3_K_NO_VALID(_phv_name, _k_base_r) \
    CAPRI_SET_TABLE_3_VALID(1);

#define CAPRI_GET_TABLE_0_OR_1_K_NO_VALID(_phv_name, _k_base_r, _cf) \
    cmov    _k_base_r, _cf, offsetof(struct _phv_name, common.common_te0_phv_table_addr), offsetof(struct _phv_name, common.common_te1_phv_table_addr);

#define CAPRI_GET_TABLE_0_OR_1_K(_phv_name, _k_base_r, _cf) \
    CAPRI_GET_TABLE_0_OR_1_K_NO_VALID(_phv_name, _k_base_r, _cf) \
    CAPRI_SET_TABLE_0_VALID_C(_cf, 1); \
    CAPRI_SET_TABLE_1_VALID_C(!_cf, 1);

#define CAPRI_GET_TABLE_0_ARG_NO_RESET(_phv_name, _arg_base_r) \
    add     _arg_base_r, 0, offsetof(struct _phv_name, common.common_t0_s2s_s2s_data);
#define CAPRI_GET_TABLE_1_ARG_NO_RESET(_phv_name, _arg_base_r) \
    add     _arg_base_r, 0, offsetof(struct _phv_name, common.common_t1_s2s_s2s_data);
#define CAPRI_GET_TABLE_2_ARG_NO_RESET(_phv_name, _arg_base_r) \
    add     _arg_base_r, 0, offsetof(struct _phv_name, common.common_t2_s2s_s2s_data);
#define CAPRI_GET_TABLE_3_ARG_NO_RESET(_phv_name, _arg_base_r) \
    add     _arg_base_r, 0, offsetof(struct _phv_name, common.common_t3_s2s_s2s_data);

#define CAPRI_GET_TABLE_0_OR_1_ARG_NO_RESET(_phv_name, _arg_base_r, _cf) \
    cmov    _arg_base_r, _cf, offsetof(struct _phv_name, common.common_t0_s2s_s2s_data), offsetof(struct _phv_name, common.common_t1_s2s_s2s_data); \


#define CAPRI_RESET_TABLE_0_ARG() \
    phvwr   p.common.common_t0_s2s_s2s_data, r0; \

#define CAPRI_RESET_TABLE_1_ARG() \
    phvwr   p.common.common_t1_s2s_s2s_data, r0; \

#define CAPRI_RESET_TABLE_2_ARG() \
    phvwr   p.common.common_t2_s2s_s2s_data, r0; \

#define CAPRI_RESET_TABLE_3_ARG() \
    phvwr   p.common.common_t3_s2s_s2s_data, r0; \

#define CAPRI_GET_TABLE_0_ARG(_phv_name, _arg_base_r) \
    CAPRI_GET_TABLE_0_ARG_NO_RESET(_phv_name, _arg_base_r) \
    CAPRI_RESET_TABLE_0_ARG();

#define CAPRI_GET_TABLE_1_ARG(_phv_name, _arg_base_r) \
    CAPRI_GET_TABLE_1_ARG_NO_RESET(_phv_name, _arg_base_r) \
    CAPRI_RESET_TABLE_1_ARG();

#define CAPRI_GET_TABLE_2_ARG(_phv_name, _arg_base_r) \
    CAPRI_GET_TABLE_2_ARG_NO_RESET(_phv_name, _arg_base_r) \
    CAPRI_RESET_TABLE_2_ARG();

#define CAPRI_GET_TABLE_3_ARG(_phv_name, _arg_base_r) \
    CAPRI_GET_TABLE_3_ARG_NO_RESET(_phv_name, _arg_base_r) \
    CAPRI_RESET_TABLE_3_ARG();

#define CAPRI_GET_TABLE_0_OR_1_ARG(_phv_name, _arg_base_r, _cf) \
    CAPRI_GET_TABLE_0_OR_1_ARG_NO_RESET(_phv_name, _arg_base_r, _cf) \
    phvwr._cf   p.common.common_t0_s2s_s2s_data, r0; \
    phvwr.!_cf   p.common.common_t1_s2s_s2s_data, r0

#define CAPRI_RESET_TABLE_0_AND_1_ARG() \
    CAPRI_RESET_TABLE_0_ARG(); \
    CAPRI_RESET_TABLE_1_ARG(); \

#define CAPRI_GET_TABLE_I_ARG(_phv_name, _tbl_id_r, _arg_base_r) \
    .brbegin; \
    br      _tbl_id_r[1:0]; \
    nop; \
    .brcase 0; \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_t0_s2s_s2s_data); \
        b _I_ARG_next; \
        phvwr    p.common.common_t0_s2s_s2s_data, r0; \
    .brcase 1; \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_t1_s2s_s2s_data); \
        b _I_ARG_next; \
        phvwr    p.common.common_t1_s2s_s2s_data, r0; \
    .brcase 2; \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_t2_s2s_s2s_data); \
        b _I_ARG_next; \
        phvwr    p.common.common_t2_s2s_s2s_data, r0; \
    .brcase 3; \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_t3_s2s_s2s_data); \
        b _I_ARG_next; \
        phvwr    p.common.common_t3_s2s_s2s_data, r0; \
    .brend; \
_I_ARG_next:;

#define CAPRI_GET_TABLE_I_K(_phv_name, _tbl_id_r, _arg_base_r) \
    .brbegin; \
    br      _tbl_id_r[1:0]; \
    nop; \
    .brcase 0; \
        CAPRI_SET_TABLE_0_VALID(1); \
        b _I_K_next; \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_te0_phv_table_addr); \
    .brcase 1; \
        CAPRI_SET_TABLE_1_VALID(1); \
        b _I_K_next; \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_te1_phv_table_addr); \
    .brcase 2; \
        CAPRI_SET_TABLE_2_VALID(1); \
        b _I_K_next; \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_te2_phv_table_addr); \
    .brcase 3; \
        CAPRI_SET_TABLE_3_VALID(1); \
        b _I_K_next; \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_te3_phv_table_addr); \
    .brend; \
_I_K_next:;

#define CAPRI_GET_STAGE_0_ARG(_phv_name, _arg_base_r) \
    add     _arg_base_r, 0, offsetof(struct _phv_name, common.to_stage_0_to_stage_data);

#define CAPRI_GET_STAGE_1_ARG(_phv_name, _arg_base_r) \
    add     _arg_base_r, 0, offsetof(struct _phv_name, common.to_stage_1_to_stage_data);

#define CAPRI_GET_STAGE_2_ARG(_phv_name, _arg_base_r) \
    add     _arg_base_r, 0, offsetof(struct _phv_name, common.to_stage_2_to_stage_data);

#define CAPRI_GET_STAGE_3_ARG(_phv_name, _arg_base_r) \
    add     _arg_base_r, 0, offsetof(struct _phv_name, common.to_stage_3_to_stage_data);

#define CAPRI_GET_STAGE_4_ARG(_phv_name, _arg_base_r) \
    add     _arg_base_r, 0, offsetof(struct _phv_name, common.to_stage_4_to_stage_data);

#define CAPRI_GET_STAGE_5_ARG(_phv_name, _arg_base_r) \
    add     _arg_base_r, 0, offsetof(struct _phv_name, common.to_stage_5_to_stage_data);

#define CAPRI_GET_STAGE_6_ARG(_phv_name, _arg_base_r) \
    add     _arg_base_r, 0, offsetof(struct _phv_name, common.to_stage_6_to_stage_data);

#define CAPRI_GET_STAGE_7_ARG(_phv_name, _arg_base_r) \
    add     _arg_base_r, 0, offsetof(struct _phv_name, common.to_stage_7_to_stage_data);

# Info on spr_mpuid
# bits 6-2 represente pipe+stage, we care about stage only (bits 4:2)
#    bits 6-5 has pipeline info:
#       00 : P4 Egress
#       01 : P4 Ingress
#       10 : TXDMA
#       11 : RXDMA
#    bits 4-2 has stage number : 0-7
# bits 1-0 represents mpuid: 0-3
#branch on current stage-id, so 0 maps to 1 etc.,
#define CAPRI_GET_STAGE_NEXT_ARG(_phv_name, _arg_base_r) \
    mfspr   _arg_base_r, spr_mpuid; \
    srl     _arg_base_r, _arg_base_r, 2; \
    .brbegin; \
    br      _arg_base_r[2:0]; \
    nop; \
    .brcase 0; \
        b _STAGE_K_next; \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.to_stage_1_to_stage_data); \
    .brcase 1; \
        b _STAGE_K_next; \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.to_stage_2_to_stage_data); \
    .brcase 2; \
        b _STAGE_K_next; \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.to_stage_3_to_stage_data); \
    .brcase 3; \
        b _STAGE_K_next; \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.to_stage_4_to_stage_data); \
    .brcase 4; \
        b _STAGE_K_next; \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.to_stage_5_to_stage_data); \
    .brcase 5; \
        b _STAGE_K_next; \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.to_stage_6_to_stage_data); \
    .brcase 6; \
        b _STAGE_K_next; \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.to_stage_7_to_stage_data); \
    .brcase 7; \
        b _STAGE_K_next; \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.to_stage_0_to_stage_data); \
    .brend; \
_STAGE_K_next:;



// DMA related defines

#define DMA_CMD_TYPE_NOP            0       //TX-RX
#define DMA_CMD_TYPE_MEM2PKT        1       //TX mem read
#define DMA_CMD_TYPE_PHV2PKT        2       //TX
#define DMA_CMD_TYPE_PHV2MEM        3       //TX-RX mem write
#define DMA_CMD_TYPE_PKT2MEM        4       //RX mem write
#define DMA_CMD_TYPE_SKIP           5       //RX ?
#define DMA_CMD_TYPE_MEM2MEM        6       //TX-RX mem read/mem write


#define DMA_CMD_MEM2MEM_TYPE_SRC        0
#define DMA_CMD_MEM2MEM_TYPE_DST        1
#define DMA_CMD_MEM2MEM_TYPE_PHV2MEM    2

#define DMA_CMD_MEM2PKT_T                struct capri_dma_cmd_mem2pkt_t
#define MEM2PKT_HOST_ADDR_OFFSET         offsetof(DMA_CMD_MEM2PKT_T, host_addr)
#define MEM2PKT_CMDTYPE_OFFSET           offsetof(DMA_CMD_MEM2PKT_T, cmdtype)
//TX - axi read, then write to packet
struct capri_dma_cmd_mem2pkt_t {
    rsvd: 42;
    size: 14;
    rsvd1: 1;
    override_lif: 11;
    addr: 52;
    use_override_lif: 1;
    cache: 1;
    host_addr: 1;
    pkteop: 1;
    cmdeop: 1;
    cmdtype: 3;
};

#define DMA_CMD_PHV2PKT_T                 struct capri_dma_cmd_phv2pkt_t
#define PHV2PKT_PHV_START_OFFSET          offsetof(DMA_CMD_PHV2PKT_T, phv_start)
#define PHV2PKT_PHV_END_OFFSET            offsetof(DMA_CMD_PHV2PKT_T, phv_end)
#define PHV2PKT_CMDTYPE_OFFSET            offsetof(DMA_CMD_PHV2PKT_T, cmdtype)
#define PHV2PKT_CMDSIZE_OFFSET            offsetof(DMA_CMD_PHV2PKT_T, cmd_size)
//TX - write to packet
struct capri_dma_cmd_phv2pkt_t {
    rsvd: 41;
    phv_end3: 10;
    phv_start3: 10;
    phv_end2: 10;
    phv_start2: 10;
    phv_end1: 10;
    phv_start1: 10;
    phv_end: 10;
    phv_start: 10;
    cmd_size: 2;
    pkteop: 1;
    cmdeop: 1;
    cmdtype: 3;
};

//TX/RX - axi write to mem
#define DMA_CMD_PHV2MEM_T                 struct capri_dma_cmd_phv2mem_t
#define PHV2MEM_PHV_START_OFFSET          offsetof(DMA_CMD_PHV2MEM_T, phv_start)
#define PHV2MEM_PHV_END_OFFSET            offsetof(DMA_CMD_PHV2MEM_T, phv_end)
#define PHV2MEM_HOST_ADDR_OFFSET          offsetof(DMA_CMD_PHV2MEM_T, host_addr)
#define PHV2MEM_CMDTYPE_OFFSET            offsetof(DMA_CMD_PHV2MEM_T, cmdtype)
#define PHV2MEM_WR_FENCE_OFFSET           offsetof(DMA_CMD_PHV2MEM_T, wr_fence)
#define PHV2MEM_CMD_EOP_OFFSET            offsetof(DMA_CMD_PHV2MEM_T, cmdeop)
struct capri_dma_cmd_phv2mem_t {
    rsvd: 33;
    override_lif: 11;
    addr: 52;
    barrier: 1;
    round: 1;
    pcie_msg: 1;
    use_override_lif: 1;
    phv_end: 10;
    phv_start: 10;
    wr_fence_fence: 1;
    wr_fence: 1;
    cache: 1;
    host_addr: 1;
    cmdeop: 1;
    cmdtype: 3;
};

//RX
#define DMA_CMD_PKT2MEM_T                struct capri_dma_cmd_pkt2mem_t
#define PKT2MEM_HOST_ADDR_OFFSET         offsetof(DMA_CMD_PKT2MEM_T, host_addr)
#define PKT2MEM_CMDTYPE_OFFSET           offsetof(DMA_CMD_PKT2MEM_T, cmdtype)
struct capri_dma_cmd_pkt2mem_t {
    rsvd: 42;
    size: 14;
    rsvd1: 1;
    override_lif: 11;
    addr: 52;
    use_override_lif: 1;
    cache: 1;
    host_addr: 1;
    round: 1;
    cmdeop: 1;
    cmdtype: 3;
};

//RX
#define DMA_CMD_SKIP_T struct capri_dma_cmd_skip_t
#define SKIP_SKIP_TO_EOP_OFFSET offsetof(DMA_CMD_SKIP_T, skip_to_eop)
#define SKIP_CMDEOP_OFFSET offsetof(DMA_CMD_SKIP_T, cmdeop)
#define SKIP_CMDTYPE_OFFSET offsetof(DMA_CMD_SKIP_T, cmdtype)
struct capri_dma_cmd_skip_t {
    rsvd: 109;
    skip_to_eop: 1;
    size: 14;
    cmdeop: 1;
    cmdtype: 3;
};

//TX - axi read, then write to mem
#define DMA_CMD_MEM2MEM_T struct capri_dma_cmd_mem2mem_t
#define MEM2MEM_PHV_START_OFFSET  offsetof(DMA_CMD_MEM2MEM_T, phv_start)
#define MEM2MEM_PHV_END_OFFSET offsetof(DMA_CMD_MEM2MEM_T, phv_end)
#define MEM2MEM_HOST_ADDR_OFFSET offsetof(DMA_CMD_MEM2MEM_T, host_addr)
#define MEM2MEM_CMDTYPE_OFFSET offsetof(DMA_CMD_MEM2MEM_T, cmdtype)
#define MEM2MEM_TYPE_OFFSET offsetof(DMA_CMD_MEM2MEM_T, mem2mem_type)
struct capri_dma_cmd_mem2mem_t {
    rsvd: 16;
    size: 14;
    rsvd1: 1;
    override_lif: 11;
    addr: 52;
    barrier: 1;
    round: 1;
    pcie_msg: 1;
    use_override_lif: 1;
    phv_end: 10;
    phv_start: 10;
    wr_fence_fence: 1;
    wr_fence: 1;
    cache: 1;
    host_addr: 1;
    mem2mem_type: 2;
    cmdeop: 1;
    cmdtype: 3;
};

#define NUM_DMA_CMDS_PER_FLIT   4
#define LOG_NUM_DMA_CMDS_PER_FLIT 2 // 2^2 = 4
#define NUM_BITS_PER_FLIT 512
#define LOG_NUM_BITS_PER_FLIT 9
#define DMA_CMD_SIZE    16
#define DMA_CMD_SIZE_BITS    (DMA_CMD_SIZE*BITS_PER_BYTE) //16 Bytes in bits
#define LOG_DMA_CMD_SIZE    4 // 2^4
#define LOG_DMA_CMD_SIZE_BITS (LOG_DMA_CMD_SIZE + LOG_BITS_PER_BYTE)
#define DMA_SWITCH_TO_NEXT_FLIT_BITS (DMA_CMD_SIZE_BITS * NUM_DMA_CMDS_PER_FLIT * 2 - DMA_CMD_SIZE_BITS)

#define RXDMA_DMA_CMD_PTR_SET_C(_flit_id, _index, _cf) \
    phvwr._cf   p.common.p4_rxdma_intr_dma_cmd_ptr, ((_flit_id * NUM_DMA_CMDS_PER_FLIT) + _index);
#define RXDMA_DMA_CMD_PTR_SET(_flit_id, _index) \
    RXDMA_DMA_CMD_PTR_SET_C(_flit_id, _index, c0); \

#define TXDMA_DMA_CMD_PTR_SET(_flit_id, _index) \
    phvwr       p.common.p4_txdma_intr_dma_cmd_ptr, (_flit_id * NUM_DMA_CMDS_PER_FLIT + _index)

#define DMA_CMD_I_BASE_GET(_base_r, _tmp_r, _flit_id, _index) \
    srl         _base_r, _index, LOG_NUM_DMA_CMDS_PER_FLIT; \
    sll         _base_r, _base_r, LOG_NUM_BITS_PER_FLIT; \
    add         _base_r, _base_r, (_flit_id+1), LOG_NUM_BITS_PER_FLIT; \
    add         _tmp_r, r0, _index; \
    add         _tmp_r, _tmp_r[1:0], 1; \
    sub         _base_r, _base_r, _tmp_r, LOG_DMA_CMD_SIZE_BITS;

#define DMA_CMD_STATIC_BASE_GET_C(_base_r, _flit_id, _index, _cf) \
addi._cf _base_r, r0,(((_index) >> LOG_NUM_DMA_CMDS_PER_FLIT) << LOG_NUM_BITS_PER_FLIT) +  \
                ((_flit_id+1) << LOG_NUM_BITS_PER_FLIT) - \
                (((_index & 0x3) + 1) << LOG_DMA_CMD_SIZE_BITS);

#define DMA_CMD_STATIC_BASE_GET_E(_base_r, _flit_id, _index) \
addi.e   _base_r, r0,(((_index) >> LOG_NUM_DMA_CMDS_PER_FLIT) << LOG_NUM_BITS_PER_FLIT) +  \
                ((_flit_id+1) << LOG_NUM_BITS_PER_FLIT) - \
                (((_index & 0x3) + 1) << LOG_DMA_CMD_SIZE_BITS);

#define DMA_CMD_STATIC_BASE_GET(_base_r, _flit_id, _index) \
    DMA_CMD_STATIC_BASE_GET_C(_base_r, _flit_id, _index, c0);

#define DMA_NEXT_CMD_I_BASE_GET(_base_r, _cmd_idx) \
    sub         _base_r, _base_r, _cmd_idx, LOG_DMA_CMD_SIZE_BITS

#define DMA_NEXT_CMD_I_BASE_GET_C(_base_r, _cmd_idx, _cf)               \
    sub._cf         _base_r, _base_r, _cmd_idx, LOG_DMA_CMD_SIZE_BITS

// x = offsetof(_field)
// flit = x / 512
// base = flit * 512
// start_offset = (flit + 1) * 512 - (x + sizeof(x)) + base
// start_offset = (offsetof(x) / 512 + 1) * 512 - offsetof(x) - sizeof(x) + (offsetof(x) / 512) * 512
#define PHV_FIELD_START_OFFSET(_field) \
        (((offsetof(p, _field) / 512 + 1) * 512 - offsetof(p, _field) \
                                         - sizeof(p._field) + (offsetof(p, _field) / 512) * 512) >> 3)

//TODO: This macro seems fine if the sizeof(__struct) is 64B (512b).
//      Need to see if it holds good for any generic struct size.
//      This macro is needed because structures used by asm are defined in
//      big-endian format for the easier access to asm routines. But when
//      doing DMA or memwr to any of these struct fields, it need to be
//      converted to little-endian accordingly ?

#define FIELD_OFFSET(__struct, __field) \
        ((sizeof(struct __struct) - offsetof(struct __struct, __field) - sizeof(struct __struct.__field)) >> 3)


#define PHV_FIELD_END_OFFSET(_field) \
        (((offsetof(p, _field) / 512 + 1) * 512 - offsetof(p, _field) \
                                         + (offsetof(p, _field) / 512) * 512) >> 3)

// Phv2Mem DMA: Specify the address of the start and end fields in the PHV
//              and the destination address
#define DMA_PHV2MEM_SETUP(_base_r, _cf, _start, _end, _addr)        \
    phvwrpi       _base_r, offsetof(DMA_CMD_PHV2MEM_T, cmdtype), CAPRI_SIZEOF_RANGE(DMA_CMD_PHV2MEM_T, phv_end, cmdtype), ((PHV_FIELD_END_OFFSET(_end) - 1) << PHV2MEM_PHV_END_OFFSET) | (PHV_FIELD_START_OFFSET(_start) << PHV2MEM_PHV_START_OFFSET) | (DMA_CMD_TYPE_PHV2MEM << PHV2MEM_CMDTYPE_OFFSET); \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2MEM_T, addr), sizeof(DMA_CMD_PHV2MEM_T.addr), _addr; \
    seq          _cf, _addr[63], 1;                      \
    phvwrp._cf   _base_r, offsetof(DMA_CMD_PHV2MEM_T, host_addr), sizeof(DMA_CMD_PHV2MEM_T.host_addr), 1; \

#define DMA_PHV2MEM_SETUP_E(_base_r, _cf, _start, _end, _addr)        \
    phvwrpi       _base_r, offsetof(DMA_CMD_PHV2MEM_T, cmdtype), CAPRI_SIZEOF_RANGE(DMA_CMD_PHV2MEM_T, phv_end, cmdtype), ((PHV_FIELD_END_OFFSET(_end) - 1) << PHV2MEM_PHV_END_OFFSET) | (PHV_FIELD_START_OFFSET(_start) << PHV2MEM_PHV_START_OFFSET) | (DMA_CMD_TYPE_PHV2MEM << PHV2MEM_CMDTYPE_OFFSET); \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2MEM_T, addr), sizeof(DMA_CMD_PHV2MEM_T.addr), _addr; \
    seq.e        _cf, _addr[63], 1;                      \
    phvwrp._cf   _base_r, offsetof(DMA_CMD_PHV2MEM_T, host_addr), sizeof(DMA_CMD_PHV2MEM_T.host_addr), 1; \

//Variation to specify a flag to say if it is host memory or not instead of
//gleaning bit 63 from address which need not be true in many cases.
#define DMA_PHV2MEM_SETUP2(_base_r, _host_cf, _start, _end, _addr)            \
    phvwrpi       _base_r, offsetof(DMA_CMD_PHV2MEM_T, cmdtype), CAPRI_SIZEOF_RANGE(DMA_CMD_PHV2MEM_T, phv_end, cmdtype), ((PHV_FIELD_END_OFFSET(_end) - 1) << PHV2MEM_PHV_END_OFFSET) | (PHV_FIELD_START_OFFSET(_start) << PHV2MEM_PHV_START_OFFSET) | (DMA_CMD_TYPE_PHV2MEM << PHV2MEM_CMDTYPE_OFFSET); \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2MEM_T, addr), sizeof(DMA_CMD_PHV2MEM_T.addr), _addr; \
    phvwrp._host_cf   _base_r, offsetof(DMA_CMD_PHV2MEM_T, host_addr), sizeof(DMA_CMD_PHV2MEM_T.host_addr), 1; \

#define DMA_HBM_PHV2MEM_SETUP(_base_r, _start, _end, _addr)        \
    phvwrpi       _base_r, offsetof(DMA_CMD_PHV2MEM_T, cmdtype), CAPRI_SIZEOF_RANGE(DMA_CMD_PHV2MEM_T, phv_end, cmdtype), ((PHV_FIELD_END_OFFSET(_end) - 1) << PHV2MEM_PHV_END_OFFSET) | (PHV_FIELD_START_OFFSET(_start) << PHV2MEM_PHV_START_OFFSET) | (DMA_CMD_TYPE_PHV2MEM << PHV2MEM_CMDTYPE_OFFSET); \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2MEM_T, addr), sizeof(DMA_CMD_PHV2MEM_T.addr), _addr; \

#define DMA_HBM_PHV2MEM_SETUP_E(_base_r, _start, _end, _addr)        \
    phvwrpi.e    _base_r, offsetof(DMA_CMD_PHV2MEM_T, cmdtype), CAPRI_SIZEOF_RANGE(DMA_CMD_PHV2MEM_T, phv_end, cmdtype), ((PHV_FIELD_END_OFFSET(_end) - 1) << PHV2MEM_PHV_END_OFFSET) | (PHV_FIELD_START_OFFSET(_start) << PHV2MEM_PHV_START_OFFSET) | (DMA_CMD_TYPE_PHV2MEM << PHV2MEM_CMDTYPE_OFFSET); \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2MEM_T, addr), sizeof(DMA_CMD_PHV2MEM_T.addr), _addr; \

#define DMA_HBM_PHV2MEM_PHV_END_SETUP(_base_r, _end) \
    phvwrpi      _base_r, offsetof(DMA_CMD_PHV2MEM_T, phv_end), sizeof(DMA_CMD_PHV2MEM_T.phv_end), (PHV_FIELD_END_OFFSET(_end) - 1);

#define DMA_HBM_PHV2MEM_PHV_END_SETUP_C(_base_r, _end, _cf) \
    phvwrpi._cf  _base_r, offsetof(DMA_CMD_PHV2MEM_T, phv_end), sizeof(DMA_CMD_PHV2MEM_T.phv_end), (PHV_FIELD_END_OFFSET(_end) - 1);

#define DMA_HBM_PHV2MEM_SETUP_F(_base_r, _start, _end, _addr)        \
    phvwrpi       _base_r, offsetof(DMA_CMD_PHV2MEM_T, cmdtype), CAPRI_SIZEOF_RANGE(DMA_CMD_PHV2MEM_T, phv_end, cmdtype), ((PHV_FIELD_END_OFFSET(_end) - 1) << PHV2MEM_PHV_END_OFFSET) | (PHV_FIELD_START_OFFSET(_start) << PHV2MEM_PHV_START_OFFSET) | (DMA_CMD_TYPE_PHV2MEM << PHV2MEM_CMDTYPE_OFFSET) | (1 << PHV2MEM_WR_FENCE_OFFSET); \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2MEM_T, addr), sizeof(DMA_CMD_PHV2MEM_T.addr), _addr; \

#define DMA_HOST_PHV2MEM_SETUP(_base_r, _start, _end, _addr)        \
    phvwrpi       _base_r, offsetof(DMA_CMD_PHV2MEM_T, cmdtype), CAPRI_SIZEOF_RANGE(DMA_CMD_PHV2MEM_T, phv_end, cmdtype), ((PHV_FIELD_END_OFFSET(_end) - 1) << PHV2MEM_PHV_END_OFFSET) | (PHV_FIELD_START_OFFSET(_start) << PHV2MEM_PHV_START_OFFSET) | (1 << PHV2MEM_HOST_ADDR_OFFSET) | (DMA_CMD_TYPE_PHV2MEM << PHV2MEM_CMDTYPE_OFFSET); \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2MEM_T, addr), sizeof(DMA_CMD_PHV2MEM_T.addr), _addr; \

#define DMA_PHV2PKT_SETUP(_base_r, _start, _end)         \
    phvwrpi       _base_r, offsetof(DMA_CMD_PHV2PKT_T, cmdtype), CAPRI_SIZEOF_RANGE(DMA_CMD_PHV2PKT_T, phv_end, cmdtype), ((PHV_FIELD_END_OFFSET(_end) - 1) << PHV2PKT_PHV_END_OFFSET) | (PHV_FIELD_START_OFFSET(_start) << PHV2PKT_PHV_START_OFFSET) | (DMA_CMD_TYPE_PHV2PKT << PHV2PKT_CMDTYPE_OFFSET); \

#define DMA_PHV2PKT_SETUP_MULTI_ADDR_0(_base_r, _start, _end, _num_addrs) \
    phvwrpi       _base_r, offsetof(DMA_CMD_PHV2PKT_T, cmdtype), CAPRI_SIZEOF_RANGE(DMA_CMD_PHV2PKT_T, phv_end, cmdtype), ((PHV_FIELD_END_OFFSET(_end) - 1) << PHV2PKT_PHV_END_OFFSET) | (PHV_FIELD_START_OFFSET(_start) << PHV2PKT_PHV_START_OFFSET) | ((_num_addrs - 1) << PHV2PKT_CMDSIZE_OFFSET) | (DMA_CMD_TYPE_PHV2PKT << PHV2PKT_CMDTYPE_OFFSET); \

#define DMA_PHV2PKT_SETUP_MULTI_ADDR_N(_base_r, _start, _end, _addr_num) \
    phvwrpi       _base_r, offsetof(DMA_CMD_PHV2PKT_T, phv_start##_addr_num), CAPRI_SIZEOF_RANGE(DMA_CMD_PHV2PKT_T, phv_end##_addr_num, phv_start##_addr_num), ((PHV_FIELD_END_OFFSET(_end) - 1) << sizeof(DMA_CMD_PHV2PKT_T.phv_start##_addr_num)) | (PHV_FIELD_START_OFFSET(_start) << 0); \

#define DMA_PHV2PKT_SETUP_MULTI_ADDR_N_C(_base_r, _start, _end, _addr_num, _cf) \
    phvwrpi._cf    _base_r, offsetof(DMA_CMD_PHV2PKT_T, phv_start##_addr_num), CAPRI_SIZEOF_RANGE(DMA_CMD_PHV2PKT_T, phv_end##_addr_num, phv_start##_addr_num), ((PHV_FIELD_END_OFFSET(_end) - 1) << sizeof(DMA_CMD_PHV2PKT_T.phv_start##_addr_num)) | (PHV_FIELD_START_OFFSET(_start) << 0); \

#define DMA_PHV2PKT_SETUP_CMDSIZE(_base_r, _num_addrs)  \
    phvwrp         _base_r, offsetof(DMA_CMD_PHV2PKT_T, cmd_size), sizeof(DMA_CMD_PHV2PKT_T.cmd_size), (_num_addrs-1);

#define DMA_PHV2PKT_SETUP_CMDSIZE_C(_base_r, _num_addrs, _cf)  \
    phvwrp._cf     _base_r, offsetof(DMA_CMD_PHV2PKT_T, cmd_size), sizeof(DMA_CMD_PHV2PKT_T.cmd_size), (_num_addrs-1);

// length in bytes
#define DMA_PHV2PKT_START_LEN_SETUP(_base_r, _tmp_r, _start, _len)         \
    phvwrpi       _base_r, offsetof(DMA_CMD_PHV2PKT_T, cmdtype), CAPRI_SIZEOF_RANGE(DMA_CMD_PHV2PKT_T, phv_start, cmdtype), (PHV_FIELD_START_OFFSET(_start) << PHV2PKT_PHV_START_OFFSET) | (DMA_CMD_TYPE_PHV2PKT << PHV2PKT_CMDTYPE_OFFSET); \
    add          _tmp_r, PHV_FIELD_START_OFFSET(_start)-1, _len; \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2PKT_T, phv_end), sizeof(DMA_CMD_PHV2PKT_T.phv_end), _tmp_r; \

#define DMA_PHV2PKT_END_LEN_SETUP(_base_r, _tmp_r, _end, _len) \
    phvwrpi       _base_r, offsetof(DMA_CMD_PHV2PKT_T, cmdtype), CAPRI_SIZEOF_RANGE(DMA_CMD_PHV2PKT_T, phv_end, cmdtype), ((PHV_FIELD_END_OFFSET(_end)-1) << PHV2PKT_PHV_END_OFFSET) | (DMA_CMD_TYPE_PHV2PKT << PHV2PKT_CMDTYPE_OFFSET); \
    sub          _tmp_r, PHV_FIELD_END_OFFSET(_end), _len; \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2PKT_T, phv_start), sizeof(DMA_CMD_PHV2PKT_T.phv_start), _tmp_r; \

#define DMA_PHV2PKT_SETUP_C(_base_r, _start, _end, _cf)         \
    phvwrpi._cf       _base_r, offsetof(DMA_CMD_PHV2PKT_T, cmdtype), CAPRI_SIZEOF_RANGE(DMA_CMD_PHV2PKT_T, phv_end, cmdtype), ((PHV_FIELD_END_OFFSET(_end) - 1) << PHV2PKT_PHV_END_OFFSET) | (PHV_FIELD_START_OFFSET(_start) << PHV2PKT_PHV_START_OFFSET) | (DMA_CMD_TYPE_PHV2PKT << PHV2PKT_CMDTYPE_OFFSET); \

#define DMA_PKT2MEM_SETUP(_base_r, _cf_host_addr, _size, _addr) \
    phvwrp      _base_r, offsetof(DMA_CMD_PKT2MEM_T, size), sizeof(DMA_CMD_PKT2MEM_T.size), _size; \
    phvwrp      _base_r, offsetof(DMA_CMD_PKT2MEM_T, addr), sizeof(DMA_CMD_PKT2MEM_T.addr), _addr; \
    phvwrp._cf_host_addr  _base_r, offsetof(DMA_CMD_PKT2MEM_T, host_addr), sizeof(DMA_CMD_PKT2MEM_T.host_addr), 1; \
    phvwrp      _base_r, offsetof(DMA_CMD_PKT2MEM_T, cmdtype), sizeof(DMA_CMD_PKT2MEM_T.cmdtype), DMA_CMD_TYPE_PKT2MEM;

#define DMA_PKT2MEM_SETUP_OVERRIDE_LIF(_base_r, _cf_host_addr, _size, _addr, _cf_override_lif, _override_lif) \
    phvwrp      _base_r, offsetof(DMA_CMD_PKT2MEM_T, size), sizeof(DMA_CMD_PKT2MEM_T.size), _size; \
    phvwrp      _base_r, offsetof(DMA_CMD_PKT2MEM_T, addr), sizeof(DMA_CMD_PKT2MEM_T.addr), _addr; \
    phvwrp._cf_override_lif    _base_r, offsetof(DMA_CMD_PKT2MEM_T, use_override_lif), sizeof(DMA_CMD_PKT2MEM_T.use_override_lif), 1; \
    phvwrp._cf_override_lif    _base_r, offsetof(DMA_CMD_PKT2MEM_T, override_lif), sizeof(DMA_CMD_PKT2MEM_T.override_lif), _override_lif; \
    phvwrp._cf_host_addr       _base_r, offsetof(DMA_CMD_PKT2MEM_T, host_addr), sizeof(DMA_CMD_PKT2MEM_T.host_addr), 1; \
    phvwrp      _base_r, offsetof(DMA_CMD_PKT2MEM_T, cmdtype), sizeof(DMA_CMD_PKT2MEM_T.cmdtype), DMA_CMD_TYPE_PKT2MEM;

#define DMA_HBM_PKT2MEM_SETUP(_base_r, _size, _addr) \
    phvwrp      _base_r, offsetof(DMA_CMD_PKT2MEM_T, size), sizeof(DMA_CMD_PKT2MEM_T.size), _size; \
    phvwrp      _base_r, offsetof(DMA_CMD_PKT2MEM_T, addr), sizeof(DMA_CMD_PKT2MEM_T.addr), _addr; \
    phvwrp       _base_r, offsetof(DMA_CMD_PKT2MEM_T, cmdtype), sizeof(DMA_CMD_PKT2MEM_T.cmdtype), DMA_CMD_TYPE_PKT2MEM;

#define DMA_HOST_PKT2MEM_SETUP(_base_r, _size, _addr) \
    phvwrp      _base_r, offsetof(DMA_CMD_PKT2MEM_T, size), sizeof(DMA_CMD_PKT2MEM_T.size), _size; \
    phvwrp      _base_r, offsetof(DMA_CMD_PKT2MEM_T, addr), sizeof(DMA_CMD_PKT2MEM_T.addr), _addr; \
    phvwrp      _base_r, offsetof(DMA_CMD_PKT2MEM_T, cmdtype), CAPRI_SIZEOF_RANGE(DMA_CMD_PKT2MEM_T, host_addr, cmdtype), (1 << PKT2MEM_HOST_ADDR_OFFSET) | (DMA_CMD_TYPE_PKT2MEM << PKT2MEM_CMDTYPE_OFFSET)

#define DMA_MEM2PKT_SETUP(_base_r, _cf_host_addr, _size, _addr)                                                    \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2PKT_T, size), sizeof(DMA_CMD_MEM2PKT_T.size), _size;       \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2PKT_T, addr), sizeof(DMA_CMD_MEM2PKT_T.addr), _addr;       \
    phvwrp._cf_host_addr  _base_r, offsetof(DMA_CMD_MEM2PKT_T, host_addr), sizeof(DMA_CMD_MEM2PKT_T.host_addr), 1;   \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2PKT_T, cmdtype), sizeof(DMA_CMD_MEM2PKT_T.cmdtype), DMA_CMD_TYPE_MEM2PKT;

#define DMA_HBM_MEM2PKT_SETUP(_base_r, _size, _addr)                                                                    \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2PKT_T, size), sizeof(DMA_CMD_MEM2PKT_T.size), _size;                      \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2PKT_T, addr), sizeof(DMA_CMD_MEM2PKT_T.addr), _addr;                      \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2PKT_T, cmdtype), sizeof(DMA_CMD_MEM2PKT_T.cmdtype), DMA_CMD_TYPE_MEM2PKT;

#define DMA_HOST_MEM2PKT_SETUP(_base_r, _size, _addr)                                                                   \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2PKT_T, size), sizeof(DMA_CMD_MEM2PKT_T.size), _size;                      \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2PKT_T, addr), sizeof(DMA_CMD_MEM2PKT_T.addr), _addr;                      \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2PKT_T, cmdtype), CAPRI_SIZEOF_RANGE(DMA_CMD_MEM2PKT_T, host_addr, cmdtype), (1 << MEM2PKT_HOST_ADDR_OFFSET) | (DMA_CMD_TYPE_MEM2PKT << MEM2PKT_CMDTYPE_OFFSET);

//TODO:
// all MEM2MEM_PHV2MEM macros are not needed to set size field, but because of model bug we have to set non-zero value. Otherwise model
// was wrongly detecting it as an ERROR and dropping the DMA command. We need to remove this workaround once model fix is available.
#define DMA_MEM2MEM_PHV2MEM_SETUP(_base_r, _cf, _start, _end, _addr)        \
    phvwrpi       _base_r, offsetof(DMA_CMD_MEM2MEM_T, cmdtype), CAPRI_SIZEOF_RANGE(DMA_CMD_MEM2MEM_T, phv_end, cmdtype), ((PHV_FIELD_END_OFFSET(_end) - 1) << MEM2MEM_PHV_END_OFFSET) | (PHV_FIELD_START_OFFSET(_start) << MEM2MEM_PHV_START_OFFSET) | (DMA_CMD_MEM2MEM_TYPE_PHV2MEM << MEM2MEM_TYPE_OFFSET) | (DMA_CMD_TYPE_MEM2MEM << MEM2MEM_CMDTYPE_OFFSET); \
    phvwrp       _base_r, offsetof(DMA_CMD_MEM2MEM_T, addr), sizeof(DMA_CMD_MEM2MEM_T.addr), _addr; \
    seq          _cf, _addr[63], 1;                      \
    phvwrp._cf   _base_r, offsetof(DMA_CMD_MEM2MEM_T, host_addr), sizeof(DMA_CMD_MEM2MEM_T.host_addr), 1; \
    phvwrpi      _base_r, offsetof(DMA_CMD_MEM2MEM_T, size), sizeof(DMA_CMD_MEM2MEM_T.size), 1; \

#define DMA_HBM_MEM2MEM_PHV2MEM_SETUP(_base_r, _start, _end, _addr)        \
    phvwrpi       _base_r, offsetof(DMA_CMD_MEM2MEM_T, cmdtype), CAPRI_SIZEOF_RANGE(DMA_CMD_MEM2MEM_T, phv_end, cmdtype), ((PHV_FIELD_END_OFFSET(_end) - 1) << MEM2MEM_PHV_END_OFFSET) | (PHV_FIELD_START_OFFSET(_start) << MEM2MEM_PHV_START_OFFSET) | (DMA_CMD_MEM2MEM_TYPE_PHV2MEM << MEM2MEM_TYPE_OFFSET) | (DMA_CMD_TYPE_MEM2MEM << MEM2MEM_CMDTYPE_OFFSET); \
    phvwrp       _base_r, offsetof(DMA_CMD_MEM2MEM_T, addr), sizeof(DMA_CMD_MEM2MEM_T.addr), _addr; \
    phvwrpi      _base_r, offsetof(DMA_CMD_MEM2MEM_T, size), sizeof(DMA_CMD_MEM2MEM_T.size), 1; \

#define DMA_HOST_MEM2MEM_PHV2MEM_SETUP(_base_r, _start, _end, _addr)        \
    phvwrpi       _base_r, offsetof(DMA_CMD_MEM2MEM_T, cmdtype), CAPRI_SIZEOF_RANGE(DMA_CMD_MEM2MEM_T, phv_end, cmdtype), ((PHV_FIELD_END_OFFSET(_end) - 1) << MEM2MEM_PHV_END_OFFSET) | (PHV_FIELD_START_OFFSET(_start) << MEM2MEM_PHV_START_OFFSET) | (1 << MEM2MEM_HOST_ADDR_OFFSET) | (DMA_CMD_MEM2MEM_TYPE_PHV2MEM << MEM2MEM_TYPE_OFFSET) | (DMA_CMD_TYPE_MEM2MEM << MEM2MEM_CMDTYPE_OFFSET); \
    phvwrp       _base_r, offsetof(DMA_CMD_MEM2MEM_T, addr), sizeof(DMA_CMD_MEM2MEM_T.addr), _addr; \
    phvwrpi      _base_r, offsetof(DMA_CMD_MEM2MEM_T, size), sizeof(DMA_CMD_MEM2MEM_T.size), 1; \

#define DMA_HBM_MEM2MEM_SRC_SETUP(_base_r, _size, _addr) \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2MEM_T, size), sizeof(DMA_CMD_MEM2MEM_T.size), _size; \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2MEM_T, addr), sizeof(DMA_CMD_MEM2MEM_T.addr), _addr; \
    phvwrp       _base_r, offsetof(DMA_CMD_MEM2MEM_T, cmdtype), CAPRI_SIZEOF_RANGE(DMA_CMD_MEM2MEM_T, mem2mem_type, cmdtype), (DMA_CMD_MEM2MEM_TYPE_SRC << MEM2MEM_TYPE_OFFSET) | (DMA_CMD_TYPE_MEM2MEM << MEM2MEM_CMDTYPE_OFFSET);

#define DMA_HOST_MEM2MEM_SRC_SETUP(_base_r, _size, _addr) \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2MEM_T, size), sizeof(DMA_CMD_MEM2MEM_T.size), _size; \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2MEM_T, addr), sizeof(DMA_CMD_MEM2MEM_T.addr), _addr; \
    phvwrp       _base_r, offsetof(DMA_CMD_MEM2MEM_T, cmdtype), CAPRI_SIZEOF_RANGE(DMA_CMD_MEM2MEM_T, host_addr, cmdtype), (1 << MEM2MEM_HOST_ADDR_OFFSET) | (DMA_CMD_MEM2MEM_TYPE_SRC << MEM2MEM_TYPE_OFFSET) | (DMA_CMD_TYPE_MEM2MEM << MEM2MEM_CMDTYPE_OFFSET);

#define DMA_HOST_MEM2MEM_SRC_SETUP_C(_base_r, _size, _addr1, _addr2, _cf) \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2MEM_T, size), sizeof(DMA_CMD_MEM2MEM_T.size), _size; \
    phvwrp._cf  _base_r, offsetof(DMA_CMD_MEM2MEM_T, addr), sizeof(DMA_CMD_MEM2MEM_T.addr), _addr1; \
    phvwrp.!_cf _base_r, offsetof(DMA_CMD_MEM2MEM_T, addr), sizeof(DMA_CMD_MEM2MEM_T.addr), _addr2; \
    phvwrp       _base_r, offsetof(DMA_CMD_MEM2MEM_T, cmdtype), CAPRI_SIZEOF_RANGE(DMA_CMD_MEM2MEM_T, host_addr, cmdtype), (1 << MEM2MEM_HOST_ADDR_OFFSET) | (DMA_CMD_MEM2MEM_TYPE_SRC << MEM2MEM_TYPE_OFFSET) | (DMA_CMD_TYPE_MEM2MEM << MEM2MEM_CMDTYPE_OFFSET);


#define DMA_HBM_MEM2MEM_DST_SETUP(_base_r, _size, _addr) \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2MEM_T, size), sizeof(DMA_CMD_MEM2MEM_T.size), _size; \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2MEM_T, addr), sizeof(DMA_CMD_MEM2MEM_T.addr), _addr; \
    phvwrp       _base_r, offsetof(DMA_CMD_MEM2MEM_T, cmdtype), CAPRI_SIZEOF_RANGE(DMA_CMD_MEM2MEM_T, mem2mem_type, cmdtype), (DMA_CMD_MEM2MEM_TYPE_DST << MEM2MEM_TYPE_OFFSET) | (DMA_CMD_TYPE_MEM2MEM << MEM2MEM_CMDTYPE_OFFSET);

#define DMA_HOST_MEM2MEM_DST_SETUP(_base_r, _size, _addr) \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2MEM_T, size), sizeof(DMA_CMD_MEM2MEM_T.size), _size; \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2MEM_T, addr), sizeof(DMA_CMD_MEM2MEM_T.addr), _addr; \
    phvwrp       _base_r, offsetof(DMA_CMD_MEM2MEM_T, cmdtype), CAPRI_SIZEOF_RANGE(DMA_CMD_MEM2MEM_T, host_addr, cmdtype), (1 << MEM2MEM_HOST_ADDR_OFFSET) | (DMA_CMD_MEM2MEM_TYPE_DST << MEM2MEM_TYPE_OFFSET) | (DMA_CMD_TYPE_MEM2MEM << MEM2MEM_CMDTYPE_OFFSET);

#define DMA_SKIP_CMD_SETUP(_base_r, _cmd_eop, _skip_to_eop) \
    phvwrpi      _base_r, offsetof(DMA_CMD_SKIP_T, cmdtype), CAPRI_SIZEOF_RANGE(DMA_CMD_SKIP_T, skip_to_eop, cmdtype), (_skip_to_eop << SKIP_SKIP_TO_EOP_OFFSET) | (_cmd_eop << SKIP_CMDEOP_OFFSET) | (DMA_CMD_TYPE_SKIP << SKIP_CMDTYPE_OFFSET);

#define DMA_SKIP_CMD_SETUP_C(_base_r, _cmd_eop, _skip_to_eop, _cf) \
    phvwrpi._cf  _base_r, offsetof(DMA_CMD_SKIP_T, cmdtype), CAPRI_SIZEOF_RANGE(DMA_CMD_SKIP_T, skip_to_eop, cmdtype), (_skip_to_eop << SKIP_SKIP_TO_EOP_OFFSET) | (_cmd_eop << SKIP_CMDEOP_OFFSET) | (DMA_CMD_TYPE_SKIP << SKIP_CMDTYPE_OFFSET);

#define DMA_SKIP_CMD_SETUP_CE(_base_r, _cmd_eop, _skip_to_eop, _cf) \
    phvwrpi._cf.e  _base_r, offsetof(DMA_CMD_SKIP_T, cmdtype), CAPRI_SIZEOF_RANGE(DMA_CMD_SKIP_T, skip_to_eop, cmdtype), (_skip_to_eop << SKIP_SKIP_TO_EOP_OFFSET) | (_cmd_eop << SKIP_CMDEOP_OFFSET) | (DMA_CMD_TYPE_SKIP << SKIP_CMDTYPE_OFFSET);

#define DMA_SET_END_OF_CMDS(_cmd_t, _base_r)                                  \
    phvwrp     _base_r, offsetof(_cmd_t, cmdeop), sizeof(_cmd_t.cmdeop), 1

#define DMA_SET_END_OF_CMDS_E(_cmd_t, _base_r)                                  \
    phvwrp.e   _base_r, offsetof(_cmd_t, cmdeop), sizeof(_cmd_t.cmdeop), 1

#define DMA_SET_END_OF_CMDS_C(_cmd_t, _base_r, _cf)                                  \
    phvwrp._cf _base_r, offsetof(_cmd_t, cmdeop), sizeof(_cmd_t.cmdeop), 1

#define DMA_SET_END_OF_CMDS_CE(_cmd_t, _base_r, _cf)                                  \
    phvwrp._cf.e    _base_r, offsetof(_cmd_t, cmdeop), sizeof(_cmd_t.cmdeop), 1

#define DMA_SET_END_OF_PKT(_cmd_t, _base_r)                                  \
    phvwrp     _base_r, offsetof(_cmd_t, pkteop), sizeof(_cmd_t.pkteop), 1

#define DMA_SET_END_OF_PKT_C(_cmd_t, _base_r, _cf)                                  \
    phvwrp._cf _base_r, offsetof(_cmd_t, pkteop), sizeof(_cmd_t.pkteop), 1

#define DMA_SET_WR_FENCE(_cmd_t, _base_r) \
    phvwrp     _base_r, offsetof(_cmd_t, wr_fence), sizeof(_cmd_t.wr_fence), 1

#define DMA_SET_WR_FENCE_FENCE(_cmd_t, _base_r) \
    phvwrp     _base_r, offsetof(_cmd_t, wr_fence_fence), sizeof(_cmd_t.wr_fence_fence), 1

#define DMA_SET_END_OF_PKT_END_OF_CMDS(_cmd_t, _base_r) \
    phvwrp     _base_r, offsetof(_cmd_t, cmdeop), CAPRI_SIZEOF_RANGE(_cmd_t, pkteop, cmdeop), (1 << (offsetof(_cmd_t, pkteop) - offsetof(_cmd_t, cmdeop)) | (1 << 0))

#define DMA_SET_END_OF_PKT_END_OF_CMDS_E(_cmd_t, _base_r) \
    phvwrp.e   _base_r, offsetof(_cmd_t, cmdeop), CAPRI_SIZEOF_RANGE(_cmd_t, pkteop, cmdeop), (1 << (offsetof(_cmd_t, pkteop) - offsetof(_cmd_t, cmdeop)) | (1 << 0))

#define DMA_SET_WR_FENCE_END_OF_CMDS_E(_cmd_t, _base_r) \
    phvwrp.e   _base_r, offsetof(_cmd_t, cmdeop), CAPRI_SIZEOF_RANGE(_cmd_t, wr_fence, cmdeop),  (1 << (offsetof(_cmd_t, wr_fence) - offsetof(_cmd_t, cmdeop)) | (1 << 0))

#define DMA_UNSET_END_OF_CMDS(_cmd_t, _base_r)                                  \
    phvwrp     _base_r, offsetof(_cmd_t, cmdeop), sizeof(_cmd_t.cmdeop), 0

#define DMA_UNSET_END_OF_CMDS_C(_cmd_t, _base_r, _cf)                           \
    phvwrp._cf _base_r, offsetof(_cmd_t, cmdeop), sizeof(_cmd_t.cmdeop), 0

#ifdef ELBA
#define DB_ADDR_BASE           0x10800000
#else
#define DB_ADDR_BASE           0x8800000
#endif

#define DB_RING_UPD_SHIFT      2
#define DB_UPD_SHIFT          17
#define DB_LIF_SHIFT           6
#define DB_QTYPE_SHIFT         3
#define DB_RING_SHIFT         16
#define DB_QID_SHIFT          24

#define DB_NO_SCHED_WR        0x0
#define DB_SCHED_WR_EVAL_RING 0x1
#define DB_SCHED_WR_0         0x2
#define DB_SCHED_WR_1         0x3

#define DB_NO_UPDATE          0x0
#define DB_SET_CINDEX         0x1
#define DB_SET_PINDEX         0x2
#define DB_INC_PINDEX         0x3

#define CAPRI_ENCODE_DB_ADDR_UPD_BITS(__ring_upd, __sched_wr) \
    (DB_ADDR_BASE + (((__ring_upd << DB_RING_UPD_SHIFT) + __sched_wr) << DB_UPD_SHIFT))

#define CAPRI_SETUP_DB_ADDR(_addr_prefix, _ring_upd, _sched_wr, _lif, _qtype, _capri_addr)       \
    add     _capri_addr, r0, _lif, DB_LIF_SHIFT; \
    add     _capri_addr, _capri_addr, _qtype, DB_QTYPE_SHIFT; \
    addi    _capri_addr, _capri_addr, CAPRI_ENCODE_DB_ADDR_UPD_BITS(_ring_upd, _sched_wr);

#define CAPRI_SETUP_DB_DATA(_qid, _ring, _ring_index, _capri_data)                               \
    add       _capri_data, _ring_index, _ring, DB_RING_SHIFT;                                    \
    add       _capri_data, _capri_data, _qid, DB_QID_SHIFT;

#define PREPARE_DOORBELL_WRITE_CINDEX(_lif, _qtype, _qid, _ring_id, _cindex, _addr, _data)                  \
    CAPRI_SETUP_DB_ADDR(DB_ADDR_BASE, DB_SET_CINDEX, DB_SCHED_WR_EVAL_RING, _lif, _qtype, _addr);   \
    CAPRI_SETUP_DB_DATA(_qid, _ring_id, _cindex, _data);                                            \

#define DOORBELL_WRITE_CINDEX(_lif, _qtype, _qid, _ring_id, _cindex, _addr, _data)                  \
    PREPARE_DOORBELL_WRITE_CINDEX(_lif, _qtype, _qid, _ring_id, _cindex, _addr, _data);                 \
    memwr.dx   _addr, _data;

#define PREPARE_DOORBELL_WRITE_PINDEX(_lif, _qtype, _qid, _ring_id, _pindex, _addr, _data)                   \
    CAPRI_SETUP_DB_DATA(_qid, _ring_id, _pindex, _data);                                            \
    CAPRI_SETUP_DB_ADDR(DB_ADDR_BASE, DB_SET_PINDEX, DB_SCHED_WR_EVAL_RING, _lif, _qtype, _addr);   \

#define DOORBELL_WRITE_PINDEX(_lif, _qtype, _qid, _ring_id, _pindex, _addr, _data)                   \
    PREPARE_DOORBELL_WRITE_PINDEX(_lif, _qtype, _qid, _ring_id, _pindex, _addr, _data);                  \
    memwr.dx   _addr, _data;

#define PREPARE_DOORBELL_INC_PINDEX(_lif, _qtype, _qid, _ring_id, _addr, _data)                     \
    CAPRI_SETUP_DB_ADDR(DB_ADDR_BASE, DB_INC_PINDEX, DB_SCHED_WR_EVAL_RING, _lif, _qtype, _addr);   \
    CAPRI_SETUP_DB_DATA(_qid, _ring_id, r0, _data);                                                 \

#define DOORBELL_INC_PINDEX(_lif, _qtype, _qid, _ring_id, _addr, _data)                             \
    PREPARE_DOORBELL_INC_PINDEX(_lif, _qtype, _qid, _ring_id, _addr, _data);                        \
    memwr.dx   _addr, _data;

// always use ring_id as r0
#define PREPARE_DOORBELL_NO_UPDATE(_lif, _qtype, _qid, _addr, _data)                   \
    CAPRI_SETUP_DB_ADDR(DB_ADDR_BASE, DB_NO_UPDATE, DB_SCHED_WR_EVAL_RING, _lif, _qtype, _addr);   \
    CAPRI_SETUP_DB_DATA(_qid, r0, r0, _data);                                            \

#define DOORBELL_NO_UPDATE(_lif, _qtype, _qid, _addr, _data)                   \
    PREPARE_DOORBELL_NO_UPDATE(_lif, _qtype, _qid, _addr, _data);                  \
    memwr.dx   _addr, _data;

#define PREPARE_DOORBELL_NO_UPDATE_DISABLE_SCHEDULER(_lif, _qtype, _qid, _ring_id, _addr, _data) \
    CAPRI_SETUP_DB_ADDR(DB_ADDR_BASE, DB_NO_UPDATE, DB_SCHED_WR_0, _lif, _qtype, _addr);   \
    CAPRI_SETUP_DB_DATA(_qid, _ring_id, r0, _data);                                            \

#define DOORBELL_NO_UPDATE_DISABLE_SCHEDULER(_lif, _qtype, _qid, _ring_id, _addr, _data)         \
    PREPARE_DOORBELL_NO_UPDATE_DISABLE_SCHEDULER(_lif, _qtype, _qid, _ring_id, _addr, _data);     \
    memwr.dx   _addr, _data;

#define PREPARE_DOORBELL_NO_UPDATE_ENABLE_SCHEDULER(_lif, _qtype, _qid, _ring_id, _addr, _data) \
    CAPRI_SETUP_DB_ADDR(DB_ADDR_BASE, DB_NO_UPDATE, DB_SCHED_WR_1, _lif, _qtype, _addr);   \
    CAPRI_SETUP_DB_DATA(_qid, _ring_id, r0, _data);                                            \

#define DOORBELL_NO_UPDATE_ENABLE_SCHEDULER(_lif, _qtype, _qid, _ring_id, _addr, _data)         \
    PREPARE_DOORBELL_NO_UPDATE_ENABLE_SCHEDULER(_lif, _qtype, _qid, _ring_id, _addr, _data);     \
    memwr.dx   _addr, _data;

#define ARE_ALL_RINGS_EMPTY(_c, _flags_r, _ring_id_bmap) \
    smeqb   _c, _flags_r, _ring_id_bmap, 0;

#define PENDING_RECIR_PKTS_MAX  4

// Timers
#define CAPRI_FAST_TIMER_ADDR(_lif) \
        (ASIC_MEM_FAST_TIMER_START + (_lif << 3))

#define CAPRI_SLOW_TIMER_ADDR(_lif) \
        (ASIC_MEM_SLOW_TIMER_START + (_lif << 3))

#define TIMER_ADDR_LIF_SHFT         3
#define TIMER_QID_SHFT              3
#define TIMER_RING_SHFT             27
#define TIMER_DELTA_TIME_SHFT       30

#define CAPRI_SETUP_TIMER_ADDR(__addr, _base, _lif) \
        addi            __addr, r0, _base; \
        add             __addr, __addr, _lif, TIMER_ADDR_LIF_SHFT;

#define CAPRI_SETUP_TIMER_DATA(__data, _qtype, _qid, _ring, _delta_time) \
        add             __data, _qtype, _qid, TIMER_QID_SHFT;\
        or              __data, __data, _ring, TIMER_RING_SHFT;\
        or              __data, __data, _delta_time, TIMER_DELTA_TIME_SHFT;

#define CAPRI_START_SLOW_TIMER(__addr, __data, _lif, _qtype, _qid, _ring, _delta_time) \
        CAPRI_SETUP_TIMER_ADDR(__addr, ASIC_MEM_SLOW_TIMER_START, _lif); \
        CAPRI_SETUP_TIMER_DATA(__data, _qtype, _qid, _ring, _delta_time); \
        memwr.dx        __addr, __data;

#define CAPRI_START_FAST_TIMER(__addr, __data, _lif, _qtype, _qid, _ring, _delta_time) \
        CAPRI_SETUP_TIMER_ADDR(__addr, ASIC_MEM_FAST_TIMER_START, _lif); \
        CAPRI_SETUP_TIMER_DATA(__data, _qtype, _qid, _ring, _delta_time); \
        memwr.dx        __addr, __data;

#endif //__CAPRI_H
