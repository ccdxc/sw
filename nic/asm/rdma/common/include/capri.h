#ifndef __CAPRI_H
#define __CAPRI_H

#define CAPRI_NUM_STAGES    8
#define CAPRI_STAGE_FIRST   0
#define CAPRI_STAGE_LAST    (CAPRI_NUM_STAGES-1)


// intrinsic fields
#define CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR k.{p4_rxdma_intr_qstate_addr_sbit0_ebit1...p4_rxdma_intr_qstate_addr_sbit2_ebit33}
#define CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR_WITH_SHIFT(_shift) k.{p4_rxdma_intr_qstate_addr_sbit0_ebit1...p4_rxdma_intr_qstate_addr_sbit2_ebit33[31:_shift]}
#define CAPRI_RXDMA_INTRINSIC_LIF k.{p4_intr_global_lif_sbit0_ebit2...p4_intr_global_lif_sbit3_ebit10}
#define CAPRI_RXDMA_INTRINSIC_QTYPE k.p4_rxdma_intr_qtype
#define CAPRI_RXDMA_INTRINSIC_QID k.p4_rxdma_intr_qid
#define CAPRI_RXDMA_INTRINSIC_QID_QTYPE k.{p4_rxdma_intr_qid...p4_rxdma_intr_qtype}

#define CAPRI_RXDMA_RETH_VA(_r) \
    add _r, k.{rdma_bth_reth_reth_va_sbit56_ebit63}, k.{rdma_bth_reth_reth_va_sbit0_ebit7...rdma_bth_reth_reth_va_sbit40_ebit55}, 8

//#define CAPRI_RXDMA_RETH_DMA_LEN k.rdma_bth_reth_reth_dma_len
#define CAPRI_RXDMA_RETH_DMA_LEN k.{rdma_bth_reth_reth_dma_len1...rdma_bth_reth_reth_dma_len2}
#define CAPRI_RXDMA_RETH_R_KEY k.rdma_bth_reth_reth_r_key

#define CAPRI_RXDMA_BTH_IMMETH_IMMDATA  k.{rdma_bth_immeth_immeth_data_sbit0_ebit7...rdma_bth_immeth_immeth_data_sbit8_ebit31}
#define CAPRI_RXDMA_BTH_RETH_IMMETH_IMMDATA_C(_r, _c_flag) \
    add._c_flag _r,  k.{rdma_bth_reth_immeth_immeth_data_sbit16_ebit31}, k.{rdma_bth_reth_immeth_immeth_data_sbit0_ebit15}, 16

#define CAPRI_RXDMA_BTH_IETH_R_KEY k.{rdma_bth_ieth_ieth_r_key_sbit0_ebit7...rdma_bth_ieth_ieth_r_key_sbit8_ebit31}

#define CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}
#define CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR_WITH_SHIFT(_shift) k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33[31:_shift]}
#define CAPRI_TXDMA_INTRINSIC_LIF k.{p4_intr_global_lif_sbit0_ebit2...p4_intr_global_lif_sbit3_ebit10}
#define CAPRI_TXDMA_INTRINSIC_QTYPE k.p4_txdma_intr_qtype
#define CAPRI_TXDMA_INTRINSIC_QID k.p4_txdma_intr_qid

// app data fields
#define CAPRI_APP_DATA_RAW_FLAGS k.rdma_bth_raw_flags
#define CAPRI_APP_DATA_PAYLOAD_LEN k.rdma_bth_payload_len

//BTH header fields
#define CAPRI_APP_DATA_BTH_OPCODE k.rdma_bth_bth_opcode
#define CAPRI_APP_DATA_BTH_PSN k.rdma_bth_bth_psn
#define CAPRI_APP_DATA_AETH_MSN k.rdma_bth_aeth_aeth_msn
#define CAPRI_APP_DATA_AETH_SYNDROME k.rdma_bth_aeth_aeth_syndrome
#define CAPRI_APP_DATA_BTH_ACK_REQ  k.rdma_bth_bth_a

#define CAPRI_RAW_TABLE_PC_SHIFT 6
#define CAPRI_INTR_BASE 0x1234

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
    cosA: 4;
    cosB: 4;
    cos_selector: 8;
    eval_last: 8;
    host_rings: 4;
    total_rings: 4;
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

#define CAPRI_FLIT_SIZE         64 // Bytes
#define CAPRI_FLIT_SIZE_BITS    (CAPRI_FLIT_SIZE * BITS_PER_BYTE)     



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

#define CAPRI_TABLE_LOCK_EN    1
#define CAPRI_TABLE_LOCK_DIS   0

#define CAPRI_SET_FIELD(_base_r, _struct_name, _field_name, _src) \
    phvwrp  _base_r, offsetof(_struct_name, _field_name), sizeof(_struct_name._field_name), _src; 

#define CAPRI_SET_2_FIELDS(_base_r, _struct_name, _first_field_name, _second_field_name, _src) \
    phvwrp  _base_r, offsetof(_struct_name, _first_field_name), \
            sizeof(_struct_name._first_field_name) + sizeof(_struct_name._second_field_name), _src;

#define CAPRI_SET_FIELD_IMM(_base_r, _struct_name, _field_name, _immdata) \
    phvwrpi  _base_r, offsetof(_struct_name, _field_name), sizeof(_struct_name._field_name), _immdata; 

#define CAPRI_TABLE_GET_FIELD(_dst_r, _base_r, _struct_name, _field_name) \
    tblrdp  _dst_r, _base_r, offsetof(_struct_name, _field_name), sizeof(_struct_name._field_name);

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

#define CAPRI_GET_TABLE_ADDR(_addr_r, _table_base_r, _eid, _entry_size_shift) \
    add  _addr_r, r0, _eid; \
    sll  _addr_r, _addr_r, _entry_size_shift; \
    add  _addr_r, _addr_r, _table_base_r;

#define CAPRI_GET_TABLE_ADDR_WITH_OFFSET(_addr_r, _table_base_r, _eid, _entry_size_shift, _state_offset) \
    CAPRI_GET_TABLE_ADDR(_addr_r, _table_base_r, _eid, _entry_size_shift); \
    addi _addr_r, _addr_r, _state_offset;

#define CAPRI_SET_RAW_TABLE_PC(_r, _pc) \
    addi _r, r0, _pc; \

#define CAPRI_SET_RAW_TABLE_PC_C(_c, _r, _pc) \
    addi._c    _r, r0, _pc; \

#define CAPRI_NEXT_TABLE_I_READ(_base_r, _lock_en, _table_read_size, _table_pc_r, _table_addr_r) \
    phvwrpi  _base_r, offsetof(INTRINSIC_RAW_K_T, table_read_size), 4, (_lock_en << 3)|(_table_read_size); \
    phvwrp  _base_r, offsetof(INTRINSIC_RAW_K_T, table_pc), sizeof(INTRINSIC_RAW_K_T.table_pc), _table_pc_r[63:CAPRI_RAW_TABLE_PC_SHIFT]; \
    phvwrp  _base_r, offsetof(INTRINSIC_RAW_K_T, table_addr), sizeof(INTRINSIC_RAW_K_T.table_addr), _table_addr_r;

#define CAPRI_SET_TABLE_0_VALID(_vld) \
    phvwri   p.common.app_header_table0_valid, _vld;
#define CAPRI_SET_TABLE_1_VALID(_vld) \
    phvwri   p.common.app_header_table1_valid, _vld;
#define CAPRI_SET_TABLE_2_VALID(_vld) \
    phvwri   p.common.app_header_table2_valid, _vld;
#define CAPRI_SET_TABLE_3_VALID(_vld) \
    phvwri   p.common.app_header_table3_valid, _vld;
    
#define CAPRI_SET_TABLE_0_VALID_C(_c, _vld) \
    phvwri._c    p.common.app_header_table0_valid, _vld;
#define CAPRI_SET_TABLE_1_VALID_C(_c, _vld) \
    phvwri._c    p.common.app_header_table1_valid, _vld;
#define CAPRI_SET_TABLE_2_VALID_C(_c, _vld) \
    phvwri._c    p.common.app_header_table2_valid, _vld;
#define CAPRI_SET_TABLE_3_VALID_C(_c, _vld) \
    phvwri._c    p.common.app_header_table3_valid, _vld;
    
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
        b _next; \
        CAPRI_SET_TABLE_0_VALID(1);  \
    .brcase 1; \
        add  _k_base_r, 0, offsetof(struct _phv_name, common.common_te1_phv_table_addr); \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_t1_s2s_s2s_data); \
        b _next; \
        CAPRI_SET_TABLE_1_VALID(1);  \
    .brcase 2; \
        add  _k_base_r, 0, offsetof(struct _phv_name, common.common_te2_phv_table_addr); \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_t2_s2s_s2s_data); \
        b _next; \
        CAPRI_SET_TABLE_2_VALID(1);  \
    .brcase 3; \
        add  _k_base_r, 0, offsetof(struct _phv_name, common.common_te3_phv_table_addr); \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_t3_s2s_s2s_data); \
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
        b _next2; \
        CAPRI_SET_TABLE_0_VALID(1);  \
    .brcase 1; \
        add  _k_base_r, 0, offsetof(struct _phv_name, common.common_te1_phv_table_addr); \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_t1_s2s_s2s_data); \
        b _next2; \
        CAPRI_SET_TABLE_1_VALID(1);  \
    .brcase 2; \
        add  _k_base_r, 0, offsetof(struct _phv_name, common.common_te2_phv_table_addr); \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_t2_s2s_s2s_data); \
        b _next2; \
        CAPRI_SET_TABLE_2_VALID(1);  \
    .brcase 3; \
        add  _k_base_r, 0, offsetof(struct _phv_name, common.common_te3_phv_table_addr); \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_t3_s2s_s2s_data); \
        b _next2; \
        CAPRI_SET_TABLE_3_VALID(1);  \
    .brend; \
_next2:;
    
#define CAPRI_GET_TABLE_0_K(_phv_name, _k_base_r) \
    add     _k_base_r, 0, offsetof(struct _phv_name, common.common_te0_phv_table_addr); \
    CAPRI_SET_TABLE_0_VALID(1); 

#define CAPRI_GET_TABLE_1_K(_phv_name, _k_base_r) \
    add     _k_base_r, 0, offsetof(struct _phv_name, common.common_te1_phv_table_addr); \
    CAPRI_SET_TABLE_1_VALID(1);

#define CAPRI_GET_TABLE_2_K(_phv_name, _k_base_r) \
    add     _k_base_r, 0, offsetof(struct _phv_name, common.common_te2_phv_table_addr); \
    CAPRI_SET_TABLE_2_VALID(1);

#define CAPRI_GET_TABLE_3_K(_phv_name, _k_base_r) \
    add     _k_base_r, 0, offsetof(struct _phv_name, common.common_te3_phv_table_addr); \
    CAPRI_SET_TABLE_3_VALID(1);

#define CAPRI_GET_TABLE_0_OR_1_K(_phv_name, _k_base_r, _cf) \
    cmov    _k_base_r, _cf, offsetof(struct _phv_name, common.common_te0_phv_table_addr), offsetof(struct _phv_name, common.common_te1_phv_table_addr); \
    CAPRI_SET_TABLE_0_VALID_C(_cf, 1); \
    CAPRI_SET_TABLE_1_VALID_C(!_cf, 1);

#define CAPRI_GET_TABLE_0_ARG(_phv_name, _arg_base_r) \
    add     _arg_base_r, 0, offsetof(struct _phv_name, common.common_t0_s2s_s2s_data); \
    phvwr   p.common.common_t0_s2s_s2s_data, r0
#define CAPRI_GET_TABLE_1_ARG(_phv_name, _arg_base_r) \
    add     _arg_base_r, 0, offsetof(struct _phv_name, common.common_t1_s2s_s2s_data); \
    phvwr   p.common.common_t1_s2s_s2s_data, r0
#define CAPRI_GET_TABLE_2_ARG(_phv_name, _arg_base_r) \
    add     _arg_base_r, 0, offsetof(struct _phv_name, common.common_t2_s2s_s2s_data); \
    phvwr   p.common.common_t2_s2s_s2s_data, r0
#define CAPRI_GET_TABLE_3_ARG(_phv_name, _arg_base_r) \
    add     _arg_base_r, 0, offsetof(struct _phv_name, common.common_t3_s2s_s2s_data); \
    phvwr   p.common.common_t3_s2s_s2s_data, r0

#define CAPRI_GET_TABLE_0_OR_1_ARG(_phv_name, _arg_base_r, _cf) \
    cmov    _arg_base_r, _cf, offsetof(struct _phv_name, common.common_t0_s2s_s2s_data), offsetof(struct _phv_name, common.common_t1_s2s_s2s_data); \
    phvwr._cf   p.common.common_t0_s2s_s2s_data, r0; \
    phvwr.!_cf   p.common.common_t1_s2s_s2s_data, r0

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

// DMA related defines

#define DMA_CMD_TYPE_NOP            0       //TX-RX
#define DMA_CMD_TYPE_MEM2PKT        1       //TX mem read
#define DMA_CMD_TYPE_PHV2PKT        2       //TX
#define DMA_CMD_TYPE_PHV2MEM        3       //TX-RX mem write
#define DMA_CMD_TYPE_PKT2MEM        4       //RX mem write
#define DMA_CMD_TYPE_SKIP           5       //RX ?
#define DMA_CMD_TYPE_MEM2MEM        6       //TX-RX mem read/mem write


#define DMA_CMD_MEM2MEM_TYPE_SRC    0
#define DMA_CMD_MEM2MEM_TYPE_DST    1
#define DMA_CMD_MEM2MEM_PHV2MEM     2

#define DMA_CMD_MEM2PKT_T struct capri_dma_cmd_mem2pkt_t
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

#define DMA_CMD_PHV2PKT_T struct capri_dma_cmd_phv2pkt_t
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
#define DMA_CMD_PHV2MEM_T struct capri_dma_cmd_phv2mem_t
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
#define DMA_CMD_PKT2MEM_T struct capri_dma_cmd_pkt2mem_t
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
struct capri_dma_cmd_skip_t {
    rsvd: 109;
    skip_to_eop: 1;
    size: 14;
    cmdeop: 1;
    cmdtype: 3;
};

//TX - axi read, then write to mem
#define DMA_CMD_MEM2MEM_T struct capri_dma_cmd_mem2mem_t
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

#define RXDMA_DMA_CMD_PTR_SET(_flit_id) \
    phvwr       p.common.p4_rxdma_intr_dma_cmd_ptr, (_flit_id * NUM_DMA_CMDS_PER_FLIT)
#define TXDMA_DMA_CMD_PTR_SET(_flit_id) \
    phvwr       p.common.p4_txdma_intr_dma_cmd_ptr, (_flit_id * NUM_DMA_CMDS_PER_FLIT)

#define DMA_CMD_I_BASE_GET(_base_r, _tmp_r, _flit_id, _index) \
    srl         _base_r, _index, LOG_NUM_DMA_CMDS_PER_FLIT; \
    sll         _base_r, _base_r, LOG_NUM_BITS_PER_FLIT; \
    add         _base_r, _base_r, (_flit_id+1), LOG_NUM_BITS_PER_FLIT; \
    add         _tmp_r, r0, _index; \
    add         _tmp_r, _tmp_r[1:0], 1; \
    sub         _base_r, _base_r, _tmp_r, LOG_DMA_CMD_SIZE_BITS;

#define DMA_CMD_STATIC_BASE_GET(_base_r, _flit_id, _index) \
addi _base_r, r0,(((_index) >> LOG_NUM_DMA_CMDS_PER_FLIT) << LOG_NUM_BITS_PER_FLIT) +  \
                ((_flit_id+1) << LOG_NUM_BITS_PER_FLIT) - \
                (((_index & 0x3) + 1) << LOG_DMA_CMD_SIZE_BITS);

    
    
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
    phvwrpi       _base_r, offsetof(DMA_CMD_PHV2MEM_T, phv_start), sizeof(DMA_CMD_PHV2MEM_T.phv_start) + sizeof(DMA_CMD_PHV2MEM_T.phv_end), ((PHV_FIELD_END_OFFSET(_end) - 1) << 10) | PHV_FIELD_START_OFFSET(_start); \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2MEM_T, addr), sizeof(DMA_CMD_PHV2MEM_T.addr), _addr; \
    seq          _cf, _addr[63], 1;                      \
    phvwrp._cf   _base_r, offsetof(DMA_CMD_PHV2MEM_T, host_addr), sizeof(DMA_CMD_PHV2MEM_T.host_addr), 1; \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2MEM_T, cmdtype), sizeof(DMA_CMD_PHV2MEM_T.cmdtype), DMA_CMD_TYPE_PHV2MEM;

#define DMA_HBM_PHV2MEM_SETUP(_base_r, _start, _end, _addr)        \
    phvwrpi       _base_r, offsetof(DMA_CMD_PHV2MEM_T, phv_start), sizeof(DMA_CMD_PHV2MEM_T.phv_start) + sizeof(DMA_CMD_PHV2MEM_T.phv_end), ((PHV_FIELD_END_OFFSET(_end) - 1) << 10) | PHV_FIELD_START_OFFSET(_start); \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2MEM_T, addr), sizeof(DMA_CMD_PHV2MEM_T.addr), _addr; \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2MEM_T, cmdtype), sizeof(DMA_CMD_PHV2MEM_T.cmdtype), DMA_CMD_TYPE_PHV2MEM;

#define DMA_HOST_PHV2MEM_SETUP(_base_r, _start, _end, _addr)        \
    phvwrpi       _base_r, offsetof(DMA_CMD_PHV2MEM_T, phv_start), sizeof(DMA_CMD_PHV2MEM_T.phv_start) + sizeof(DMA_CMD_PHV2MEM_T.phv_end), ((PHV_FIELD_END_OFFSET(_end) - 1) << 10) | PHV_FIELD_START_OFFSET(_start); \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2MEM_T, addr), sizeof(DMA_CMD_PHV2MEM_T.addr), _addr; \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2MEM_T, host_addr), sizeof(DMA_CMD_PHV2MEM_T.host_addr), 1; \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2MEM_T, cmdtype), sizeof(DMA_CMD_PHV2MEM_T.cmdtype), DMA_CMD_TYPE_PHV2MEM;

#define DMA_PHV2PKT_SETUP(_base_r, _start, _end)         \
    phvwrpi       _base_r, offsetof(DMA_CMD_PHV2PKT_T, phv_start), sizeof(DMA_CMD_PHV2PKT_T.phv_start) + sizeof(DMA_CMD_PHV2PKT_T.phv_end), ((PHV_FIELD_END_OFFSET(_end) - 1) << 10) | PHV_FIELD_START_OFFSET(_start); \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2PKT_T, cmdtype), sizeof(DMA_CMD_PHV2PKT_T.cmdtype), DMA_CMD_TYPE_PHV2PKT;

// length in bytes
#define DMA_PHV2PKT_START_LEN_SETUP(_base_r, _tmp_r, _start, _len)         \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2PKT_T, phv_start), sizeof(DMA_CMD_PHV2PKT_T.phv_start), PHV_FIELD_START_OFFSET(_start); \
    add          _tmp_r, PHV_FIELD_START_OFFSET(_start)-1, _len; \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2PKT_T, phv_end), sizeof(DMA_CMD_PHV2PKT_T.phv_end), _tmp_r; \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2PKT_T, cmdtype), sizeof(DMA_CMD_PHV2PKT_T.cmdtype), DMA_CMD_TYPE_PHV2PKT;

#define DMA_PHV2PKT_SETUP_C(_base_r, _start, _end, _cf)         \
    phvwrpi._cf  _base_r, offsetof(DMA_CMD_PHV2PKT_T, phv_start), sizeof(DMA_CMD_PHV2PKT_T.phv_start) + sizeof(DMA_CMD_PHV2PKT_T.phv_end), ((PHV_FIELD_END_OFFSET(_end) - 1) << 10) | PHV_FIELD_START_OFFSET(_start); \
    phvwrp._cf   _base_r, offsetof(DMA_CMD_PHV2PKT_T, cmdtype), sizeof(DMA_CMD_PHV2PKT_T.cmdtype), DMA_CMD_TYPE_PHV2PKT;

#define DMA_PKT2MEM_SETUP(_base_r, _cf, _size, _addr) \
    phvwrp      _base_r, offsetof(DMA_CMD_PKT2MEM_T, size), sizeof(DMA_CMD_PKT2MEM_T.size), _size; \
    phvwrp      _base_r, offsetof(DMA_CMD_PKT2MEM_T, addr), sizeof(DMA_CMD_PKT2MEM_T.addr), _addr; \
    seq         _cf, _addr[63], 1; \
    phvwrp._cf  _base_r, offsetof(DMA_CMD_PKT2MEM_T, host_addr), sizeof(DMA_CMD_PKT2MEM_T.host_addr), 1; \
    phvwrp       _base_r, offsetof(DMA_CMD_PKT2MEM_T, cmdtype), sizeof(DMA_CMD_PKT2MEM_T.cmdtype), DMA_CMD_TYPE_PKT2MEM;

#define DMA_HBM_PKT2MEM_SETUP(_base_r, _size, _addr) \
    phvwrp      _base_r, offsetof(DMA_CMD_PKT2MEM_T, size), sizeof(DMA_CMD_PKT2MEM_T.size), _size; \
    phvwrp      _base_r, offsetof(DMA_CMD_PKT2MEM_T, addr), sizeof(DMA_CMD_PKT2MEM_T.addr), _addr; \
    phvwrp       _base_r, offsetof(DMA_CMD_PKT2MEM_T, cmdtype), sizeof(DMA_CMD_PKT2MEM_T.cmdtype), DMA_CMD_TYPE_PKT2MEM;

#define DMA_HOST_PKT2MEM_SETUP(_base_r, _size, _addr) \
    phvwrp      _base_r, offsetof(DMA_CMD_PKT2MEM_T, size), sizeof(DMA_CMD_PKT2MEM_T.size), _size; \
    phvwrp      _base_r, offsetof(DMA_CMD_PKT2MEM_T, addr), sizeof(DMA_CMD_PKT2MEM_T.addr), _addr; \
    phvwrp      _base_r, offsetof(DMA_CMD_PKT2MEM_T, host_addr), sizeof(DMA_CMD_PKT2MEM_T.host_addr), 1; \
    phvwrp      _base_r, offsetof(DMA_CMD_PKT2MEM_T, cmdtype), sizeof(DMA_CMD_PKT2MEM_T.cmdtype), DMA_CMD_TYPE_PKT2MEM;

#define DMA_MEM2PKT_SETUP(_base_r, _cf, _size, _addr)                                                    \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2PKT_T, size), sizeof(DMA_CMD_MEM2PKT_T.size), _size;       \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2PKT_T, addr), sizeof(DMA_CMD_MEM2PKT_T.addr), _addr;       \
    seq         _cf, _addr[63], 1;                                                                       \
    phvwrp._cf  _base_r, offsetof(DMA_CMD_MEM2PKT_T, host_addr), sizeof(DMA_CMD_MEM2PKT_T.host_addr), 1;   \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2PKT_T, cmdtype), sizeof(DMA_CMD_MEM2PKT_T.cmdtype), DMA_CMD_TYPE_MEM2PKT;

#define DMA_HBM_MEM2PKT_SETUP(_base_r, _size, _addr)                                                                    \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2PKT_T, size), sizeof(DMA_CMD_MEM2PKT_T.size), _size;                      \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2PKT_T, addr), sizeof(DMA_CMD_MEM2PKT_T.addr), _addr;                      \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2PKT_T, cmdtype), sizeof(DMA_CMD_MEM2PKT_T.cmdtype), DMA_CMD_TYPE_MEM2PKT;

#define DMA_HOST_MEM2PKT_SETUP(_base_r, _size, _addr)                                                                   \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2PKT_T, size), sizeof(DMA_CMD_MEM2PKT_T.size), _size;                      \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2PKT_T, addr), sizeof(DMA_CMD_MEM2PKT_T.addr), _addr;                      \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2PKT_T, host_addr), sizeof(DMA_CMD_MEM2PKT_T.host_addr), 1;                \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2PKT_T, cmdtype), sizeof(DMA_CMD_MEM2PKT_T.cmdtype), DMA_CMD_TYPE_MEM2PKT;

#define DMA_SET_END_OF_CMDS(_cmd_t, _base_r)                                  \
    phvwrp     _base_r, offsetof(_cmd_t, cmdeop), sizeof(_cmd_t.cmdeop), 1

#define DMA_SET_END_OF_CMDS_C(_cmd_t, _base_r, _cf)                                  \
    phvwrp._cf _base_r, offsetof(_cmd_t, cmdeop), sizeof(_cmd_t.cmdeop), 1

#define DMA_SET_END_OF_PKT(_cmd_t, _base_r)                                  \
    phvwrp     _base_r, offsetof(_cmd_t, pkteop), sizeof(_cmd_t.pkteop), 1

#define DMA_SET_END_OF_PKT_C(_cmd_t, _base_r, _cf)                                  \
    phvwrp._cf _base_r, offsetof(_cmd_t, pkteop), sizeof(_cmd_t.pkteop), 1

#define DMA_SET_WR_FENCE(_cmd_t, _base_r) \
    phvwrp     _base_r, offsetof(_cmd_t, wr_fence), sizeof(_cmd_t.wr_fence), 1

#define DMA_SET_WR_FENCE_FENCE(_cmd_t, _base_r) \
    phvwrp     _base_r, offsetof(_cmd_t, wr_fence_fence), sizeof(_cmd_t.wr_fence_fence), 1

#define DB_ADDR_BASE           0x68800000

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
    CAPRI_SETUP_DB_ADDR(DB_ADDR_BASE, DB_SET_PINDEX, DB_SCHED_WR_EVAL_RING, _lif, _qtype, _addr);   \
    CAPRI_SETUP_DB_DATA(_qid, _ring_id, _pindex, _data);                                            \

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

#define ARE_ALL_RINGS_EMPTY(_c, _flags_r, _ring_id_bmap) \
    smeqb   _c, _flags_r, _ring_id_bmap, 0;

#define PENDING_RECIR_PKTS_MAX  4
   
#endif //__CAPRI_H
