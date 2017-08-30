#ifndef __CAPRI_H
#define __CAPRI_H

// intrinsic fields
#define CAPRI_INTRINSIC_LIF k.{p4_intr_global_lif_sbit0_ebit2...p4_intr_global_lif_sbit3_ebit10}
#define CAPRI_INTRINSIC_QTYPE k.p4_rxdma_intr_qtype
#define CAPRI_INTRINSIC_QID k.p4_rxdma_intr_qid

#define CAPRI_INTRINSIC_QSTATE_ADDR k.{p4_rxdma_intr_qstate_addr_sbit0_ebit1...p4_rxdma_intr_qstate_addr_sbit2_ebit33}

// app data fields
#define CAPRI_APP_DATA_RAW_FLAGS k.rdma_bth_raw_flags
#define CAPRI_APP_DATA_PAYLOAD_LEN k.rdma_bth_payload_len

//BTH header fields
#define CAPRI_APP_DATA_BTH_OPCODE k.rdma_bth_bth_opcode
#define CAPRI_APP_DATA_BTH_PSN k.rdma_bth_bth_psn

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

#define CAPRI_SET_FIELD_IMM(_base_r, _struct_name, _field_name, _immdata) \
    phvwrpi  _base_r, offsetof(_struct_name, _field_name), sizeof(_struct_name._field_name), _immdata; 

#define CAPRI_TABLE_GET_FIELD(_dst_r, _base_r, _struct_name, _field_name) \
    tblrdp  _dst_r, _base_r, offsetof(_struct_name, _field_name), sizeof(_struct_name._field_name);

//set field conditionally
#define CAPRI_SET_FIELD_C(_base_r, _struct_name, _field_name, _src, _c_flag) \
    phvwrp._c_flag  _base_r, offsetof(_struct_name, _field_name), sizeof(_struct_name._field_name), _src; 

#define CAPRI_GET_TABLE_ADDR(_addr_r, _table_base_r, _eid, _entry_size_shift) \
    add  _addr_r, r0, _eid; \
    sll  _addr_r, _addr_r, _entry_size_shift; \
    add  _addr_r, _addr_r, _table_base_r;

#define CAPRI_GET_TABLE_ADDR_WITH_OFFSET(_addr_r, _table_base_r, _eid, _entry_size_shift, _state_offset) \
    CAPRI_GET_TABLE_ADDR(_addr_r, _table_base_r, _eid, _entry_size_shift); \
    addi _addr_r, _addr_r, _state_offset;

#define CAPRI_SET_RAW_TABLE_PC(_r, _pc) \
    addi _r, r0, _pc; \
    srl _r, _r, CAPRI_RAW_TABLE_PC_SHIFT;

#define CAPRI_SET_RAW_TABLE_PC_C(_c, _r, _pc) \
    addi._c    _r, r0, _pc; \
    srl._c     _r, _r, CAPRI_RAW_TABLE_PC_SHIFT;

#define CAPRI_NEXT_TABLE_I_READ(_base_r, _lock_en, _table_read_size, _table_pc_r, _table_addr_r) \
    phvwrpi  _base_r, offsetof(INTRINSIC_RAW_K_T, lock_en), sizeof(INTRINSIC_RAW_K_T.lock_en), _lock_en; \
    phvwrpi  _base_r, offsetof(INTRINSIC_RAW_K_T, table_read_size), sizeof(INTRINSIC_RAW_K_T.table_read_size), _table_read_size; \
    phvwrp  _base_r, offsetof(INTRINSIC_RAW_K_T, table_pc), sizeof(INTRINSIC_RAW_K_T.table_pc), _table_pc_r; \
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
_table_i_valid:

    
#define CAPRI_GET_TABLE_I_K_AND_ARG(_phv_name, _tbl_id_r, _k_base_r, _arg_base_r) \
    .brbegin; \
    br      _tbl_id_r[1:0]; \
    nop; \
    .brcase 0; \
        add  _k_base_r, 0, offsetof(struct _phv_name, common.common_te0_phv_table_addr); \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_t0_s2s_s2s_data); \
        CAPRI_SET_TABLE_0_VALID(1);  \
        b _next; \
        nop; \
    .brcase 1; \
        add  _k_base_r, 0, offsetof(struct _phv_name, common.common_te1_phv_table_addr); \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_t1_s2s_s2s_data); \
        CAPRI_SET_TABLE_1_VALID(1);  \
        b _next; \
        nop; \
    .brcase 2; \
        add  _k_base_r, 0, offsetof(struct _phv_name, common.common_te2_phv_table_addr); \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_t2_s2s_s2s_data); \
        CAPRI_SET_TABLE_2_VALID(1);  \
        b _next; \
        nop; \
    .brcase 3; \
        add  _k_base_r, 0, offsetof(struct _phv_name, common.common_te3_phv_table_addr); \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_t3_s2s_s2s_data); \
        CAPRI_SET_TABLE_3_VALID(1);  \
        b _next; \
        nop; \
    .brend; \
_next:;
    
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

#define CAPRI_GET_TABLE_0_ARG(_phv_name, _arg_base_r) \
    add     _arg_base_r, 0, offsetof(struct _phv_name, common.common_t0_s2s_s2s_data);
#define CAPRI_GET_TABLE_1_ARG(_phv_name, _arg_base_r) \
    add     _arg_base_r, 0, offsetof(struct _phv_name, common.common_t1_s2s_s2s_data);
#define CAPRI_GET_TABLE_2_ARG(_phv_name, _arg_base_r) \
    add     _arg_base_r, 0, offsetof(struct _phv_name, common.common_t2_s2s_s2s_data);
#define CAPRI_GET_TABLE_3_ARG(_phv_name, _arg_base_r) \
    add     _arg_base_r, 0, offsetof(struct _phv_name, common.common_t3_s2s_s2s_data);

#define CAPRI_GET_TABLE_I_ARG(_phv_name, _tbl_id_r, _arg_base_r) \
    .brbegin; \
    br      _tbl_id_r[1:0]; \
    nop; \
    .brcase 0; \
        b _I_ARG_next; \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_t0_s2s_s2s_data); \
    .brcase 1; \
        b _I_ARG_next; \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_t1_s2s_s2s_data); \
    .brcase 2; \
        b _I_ARG_next; \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_t2_s2s_s2s_data); \
    .brcase 3; \
        b _I_ARG_next; \
        add _arg_base_r, 0, offsetof(struct _phv_name, common.common_t3_s2s_s2s_data); \
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
    rsvd: 43;
    override_lif: 11;
    use_override_lif: 1;
    size: 14;
    addr: 52;
    cache: 1;
    host_addr: 1;
    pkteop: 1;
    cmdeop: 1;
    cmdtype: 3;
};

#define DMA_CMD_PHV2PKT_T struct capri_dma_cmd_phv2pkt_t
//TX - write to packet      
struct capri_dma_cmd_phv2pkt_t {
    rsvd: 103;
    phv_end: 10;
    phv_start: 10;
    pkteop: 1;
    cmdeop: 1;
    cmdtype: 3;
};

//TX/RX - axi write to mem
#define DMA_CMD_PHV2MEM_T struct capri_dma_cmd_phv2mem_t
struct capri_dma_cmd_phv2mem_t {
    rsvd: 37;
    override_lif: 11;
    use_override_lif: 1;
    phv_end: 10;
    phv_start: 10;
    wr_fence: 1;
    addr: 52;
    cache: 1;
    host_addr: 1;
    cmdeop: 1;
    cmdtype: 3;
};

//RX       
#define DMA_CMD_PKT2MEM_T struct capri_dma_cmd_pkt2mem_t
struct capri_dma_cmd_pkt2mem_t {
    rsvd: 44;
    override_lif: 11;
    use_override_lif: 1;
    size: 14;
    addr: 52;
    cache: 1;
    host_addr: 1;
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
    rsvd: 21;
    override_lif: 11;
    use_override_lif: 1;
    phv_end: 10;
    phv_start: 10;
    wr_fence: 1;
    size: 14;
    addr: 52;
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

#define RXDMA_DMA_CMD_PTR_SET(_flit_id) \
    phvwr       p.common.p4_rxdma_intr_dma_cmd_ptr, (_flit_id * NUM_DMA_CMDS_PER_FLIT)
#define TXDMA_DMA_CMD_PTR_SET(_flit_id) \
    phvwr       p.common.p4_txdma_intr_dma_cmd_ptr, (_flit_id * NUM_DMA_CMDS_PER_FLIT)

#define DMA_CMD_I_BASE_GET(_base_r, _tmp_r, _flit_id, _index) \
    srl         _base_r, _index, (LOG_NUM_BITS_PER_FLIT - LOG_NUM_DMA_CMDS_PER_FLIT); \
    add         _base_r, _base_r, (_flit_id+1), LOG_NUM_BITS_PER_FLIT; \
    add         _tmp_r, r0, _index; \
    mincr       _tmp_r, LOG_NUM_DMA_CMDS_PER_FLIT, r0; \
    add         _tmp_r, _tmp_r, 1; \
    sll         _tmp_r, _tmp_r, LOG_DMA_CMD_SIZE_BITS; \
    sub         _base_r, _base_r, _tmp_r;
    
// x = offsetof(_field)
// flit = x / 512
// base = flit * 512
// start_offset = (flit + 1) * 512 - (x + sizeof(x)) + base
// start_offset = (offsetof(x) / 512 + 1) * 512 - offsetof(x) - sizeof(x) + (offsetof(x) / 512) * 512
#define PHV_FIELD_START_OFFSET(_field) \
        (((offsetof(p, _field) / 512 + 1) * 512 - offsetof(p, _field) \
                                         - sizeof(p._field) + (offsetof(p, _field) / 512) * 512) >> 3)

#define PHV_FIELD_END_OFFSET(_field) \
        (((offsetof(p, _field) / 512 + 1) * 512 - offsetof(p, _field) \
                                         + (offsetof(p, _field) / 512) * 512) >> 3)

// Phv2Mem DMA: Specify the address of the start and end fields in the PHV
//              and the destination address
#define DMA_PHV2MEM_SETUP(_base_r, _cf, _start, _end, _addr)        \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2MEM_T, phv_start), sizeof(DMA_CMD_PHV2MEM_T.phv_start), (sizeof (p) >> 3) - ((offsetof(p, _start) + sizeof(p._start)) >> 3); \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2MEM_T, phv_end), sizeof(DMA_CMD_PHV2MEM_T.phv_end), (sizeof(p) >> 3) - (offsetof(p, _end) >> 3); \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2MEM_T, addr), sizeof(DMA_CMD_PHV2MEM_T.addr), _addr; \
    seq          _cf, _addr[63], 1;                      \
    phvwrp._cf   _base_r, offsetof(DMA_CMD_PHV2MEM_T, host_addr), sizeof(DMA_CMD_PHV2MEM_T.host_addr), 1; \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2MEM_T, cmdtype), sizeof(DMA_CMD_PHV2MEM_T.cmdtype), DMA_CMD_TYPE_PHV2MEM;

#define DMA_PHV2PKT_SETUP(_base_r, _start, _end)         \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2PKT_T, phv_start), sizeof(DMA_CMD_PHV2PKT_T.phv_start), (sizeof (p) >> 3) - ((offsetof(p, _start) + sizeof(p._start)) >> 3); \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2PKT_T, phv_end), sizeof(DMA_CMD_PHV2PKT_T.phv_end), (sizeof(p) >> 3) - (offsetof(p, _end) >> 3); \
    phvwrp       _base_r, offsetof(DMA_CMD_PHV2PKT_T, cmdtype), sizeof(DMA_CMD_PHV2PKT_T.cmdtype), DMA_CMD_TYPE_PHV2PKT;

#define DMA_PKT2MEM_SETUP(_base_r, _cf, _size, _addr) \
    phvwrp      _base_r, offsetof(DMA_CMD_PKT2MEM_T, size), sizeof(DMA_CMD_PKT2MEM_T.size), _size; \
    phvwrp      _base_r, offsetof(DMA_CMD_PKT2MEM_T, addr), sizeof(DMA_CMD_PKT2MEM_T.addr), _addr; \
    seq         _cf, _addr[63], 1; \
    phvwrp._cf  _base_r, offsetof(DMA_CMD_PKT2MEM_T, host_addr), sizeof(DMA_CMD_PKT2MEM_T.host_addr), 1; \
    phvwrp       _base_r, offsetof(DMA_CMD_PKT2MEM_T, cmdtype), sizeof(DMA_CMD_PKT2MEM_T.cmdtype), DMA_CMD_TYPE_PKT2MEM;

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
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2PKT_T, cmdtype), sizeof(DMA_CMD_MEM2PKT_T.cmdtype), DMA_CMD_TYPE_MEM2PKT; \
    phvwrp      _base_r, offsetof(DMA_CMD_MEM2PKT_T, cmdtype), sizeof(DMA_CMD_MEM2PKT_T.cmdtype), DMA_CMD_TYPE_MEM2PKT;

#define DMA_SET_END_OF_CMDS(_cmd_t, _base_r)                                  \
    phvwrp     _base_r, offsetof(_cmd_t, cmdeop), sizeof(_cmd_t.cmdeop), 1

#define DMA_SET_END_OF_CMDS_C(_cmd_t, _base_r, _cf)                                  \
    phvwrp._cf _base_r, offsetof(_cmd_t, cmdeop), sizeof(_cmd_t.cmdeop), 1

#endif //__CAPRI_H
