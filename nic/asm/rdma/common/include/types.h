#ifndef __TYPES_H
#define __TYPES_H

#define HBM_NUM_KEY_ENTRIES_PER_CACHE_LINE 2

#define RDMA_RESERVED_LKEY_ID 0
#define RDMA_EQ_ID_ASYNC 0

#define HBM_NUM_PT_ENTRIES_PER_CACHE_LINE 8
#define HBM_PAGE_SIZE_SHIFT 12 // HBM page size is assumed as 4K
#define HBM_CACHE_LINE_SIZE 64 // Bytes
#define HBM_CACHE_LINE_SIZE_BITS (HBM_CACHE_LINE_SIZE * BITS_PER_BYTE)
#define HBM_CACHE_LINE_SIZE_MASK (HBM_CACHE_LINE_SIZE - 1)
#define LOG_HBM_CACHE_LINE_SIZE 6 // 2^6 = 64 Bytes
#define LOG_HBM_NUM_PT_ENTRIES_PER_CACHELINE 3 // 2^3 = 8 entries

//pt_base_addr is 8-byte aligned as each entry in page table stores 64-bit address.
//hence when pt_base_addr is encoded in various data structures, bottom 3 bits are not
//stored. At the runtime pt_base_addr field is shifted left by 3 bits to get the 
//actual pt_base_addr.
#define PT_BASE_ADDR_SHIFT 3
#define HDR_TEMP_ADDR_SHIFT 3
#define RRQ_BASE_ADDR_SHIFT 3
#define RSQ_BASE_ADDR_SHIFT 3
#define HBM_SQ_BASE_ADDR_SHIFT 3
#define CQCB_ADDR_HI_SHIFT 10 // 24 bits of cqcb base addr, so shift 10 bits
#define SQCB_ADDR_HI_SHIFT 10 // 24 bits of cqcb base addr, so shift 10 bits
#define RQCB_ADDR_HI_SHIFT 10 // 24 bits of cqcb base addr, so shift 10 bits

#define MAX_SGES_PER_PASS   2
#define HBM_NUM_SGES_PER_CACHELINE 4
#define SQCB_ADDR_SHIFT    9
#define RQCB_ADDR_SHIFT    9
#define CQCB_ADDR_SHIFT    6
#define AQCB_ADDR_SHIFT    6
#define RDMA_PAYLOAD_DMA_CMDS_START 8

#define LOG_SIZEOF_CQCB_T   6   // 2^6 = 64 Bytes
#define LOG_CB_UNIT_SIZE_BYTES 6
#define CB_UNIT_SIZE_BYTES  64
#define CB3_OFFSET_BYTES (3 * 64)

#define LOG_SIZEOF_EQCB_T   6   // 2^6 = 64 Bytes

#define LOG_SIZEOF_SQCB_T   10
#define LOG_SIZEOF_RQCB_T   10

#define SQCB0_ADDR_GET(_r) \
    sll     _r, k.{phv_global_common_cb_addr_sbit0_ebit1...phv_global_common_cb_addr_sbit18_ebit24}, SQCB_ADDR_SHIFT;

#define SQCB1_ADDR_GET(_r) \
    add     _r, CB_UNIT_SIZE_BYTES, k.{phv_global_common_cb_addr_sbit0_ebit1...phv_global_common_cb_addr_sbit18_ebit24}, SQCB_ADDR_SHIFT;

#define SQCB2_ADDR_GET(_r) \
    add     _r, (2 * CB_UNIT_SIZE_BYTES), k.{phv_global_common_cb_addr_sbit0_ebit1...phv_global_common_cb_addr_sbit18_ebit24}, SQCB_ADDR_SHIFT;

#define SQCB3_ADDR_GET(_r) \
    add     _r, (3 * CB_UNIT_SIZE_BYTES), k.{phv_global_common_cb_addr_sbit0_ebit1...phv_global_common_cb_addr_sbit18_ebit24}, SQCB_ADDR_SHIFT;

#define SQCB4_ADDR_GET(_r) \
    add     _r, (4 * CB_UNIT_SIZE_BYTES), k.{phv_global_common_cb_addr_sbit0_ebit1...phv_global_common_cb_addr_sbit18_ebit24}, SQCB_ADDR_SHIFT;

#define SQCB5_ADDR_GET(_r) \
    add     _r, (5 * CB_UNIT_SIZE_BYTES), k.{phv_global_common_cb_addr_sbit0_ebit1...phv_global_common_cb_addr_sbit18_ebit24}, SQCB_ADDR_SHIFT;

#define SQCB0_FIELD_ADDR_GET(_r, _field_offset) \
     add    _r, _field_offset, k.{phv_global_common_cb_addr_sbit0_ebit1...phv_global_common_cb_addr_sbit18_ebit24}, SQCB_ADDR_SHIFT;
  
//RQCB organization
//0-256B RQCB0..RQCB3
//256-320B RESP_RX_STATS
//320-376B RESP_TX_STATS
#define RQCB0_ADDR_GET(_r) \
    sll     _r, k.{phv_global_common_cb_addr_sbit0_ebit1...phv_global_common_cb_addr_sbit18_ebit24}, RQCB_ADDR_SHIFT;

#define RQCB1_ADDR_GET(_r) \
    add     _r, CB_UNIT_SIZE_BYTES, k.{phv_global_common_cb_addr_sbit0_ebit1...phv_global_common_cb_addr_sbit18_ebit24}, RQCB_ADDR_SHIFT;

#define RQCB2_ADDR_GET(_r) \
    add     _r, (2 * CB_UNIT_SIZE_BYTES), k.{phv_global_common_cb_addr_sbit0_ebit1...phv_global_common_cb_addr_sbit18_ebit24}, RQCB_ADDR_SHIFT;

#define RQCB3_ADDR_GET(_r) \
    add     _r, (3 * CB_UNIT_SIZE_BYTES), k.{phv_global_common_cb_addr_sbit0_ebit1...phv_global_common_cb_addr_sbit18_ebit24}, RQCB_ADDR_SHIFT;

#define RQCB3_WRID_ADDR_GET(_r) \
    add     _r, ((3 * CB_UNIT_SIZE_BYTES) + FIELD_OFFSET(rqcb3_t, wrid)), k.{phv_global_common_cb_addr_sbit0_ebit1...phv_global_common_cb_addr_sbit18_ebit24}, RQCB_ADDR_SHIFT;

#define RQCB4_ADDR_GET(_r) \
    add     _r, (4 * CB_UNIT_SIZE_BYTES), k.{phv_global_common_cb_addr_sbit0_ebit1...phv_global_common_cb_addr_sbit18_ebit24}, RQCB_ADDR_SHIFT;
    
#define RQCB5_ADDR_GET(_r) \
    add     _r, (5 * CB_UNIT_SIZE_BYTES), k.{phv_global_common_cb_addr_sbit0_ebit1...phv_global_common_cb_addr_sbit18_ebit24}, RQCB_ADDR_SHIFT;

 //CQCB organization
//0-63B
#define CQCB0_ADDR_GET(_r) \
    sll     _r, k.{phv_global_common_cb_addr_sbit0_ebit1...phv_global_common_cb_addr_sbit18_ebit24}, CQCB_ADDR_SHIFT;


#define MAX_PYLD_DMA_CMDS_PER_SGE   3

#define BTH_OPC_SVC_SHIFT 5

struct rdma_bth_t {
    opcode  : 8;
    se      : 1;
    m       : 1;
    pad     : 2;
    tver    : 4;
    pkey    : 16;
    f       : 1;
    b       : 1;
    rsvd1   : 6;
    dst_qp  : 24;
    a       : 1;
    rsvd2   : 7;
    psn     : 24;
};

struct rdma_rdeth_t {
    rsvd    : 8;
    ee_ctxt : 24;
};

struct rdma_deth_t {
    q_key   : 32;
    rsvd    : 8;
    src_qp  : 24;
};

struct rdma_reth_t {
    va      : 64;
    r_key   : 32;
    dma_len : 32;
};

struct rdma_ieth_t {
    rkey    : 32;
};

struct rdma_atomiceth_t {
    va                 : 64;
    r_key              : 32;
    swap_or_add_data   : 64;
    cmp_data           : 64;
};

#define NAK_CODE_SEQ_ERR        0
#define NAK_CODE_INV_REQ        1
#define NAK_CODE_REM_ACC_ERR    2
#define NAK_CODE_REM_OP_ERR     3
#define NAK_CODE_INV_RD_REQ     4
#define NAK_CODE_FLUSH_RQ       0x1f    //vendor specific code
#define NAK_CODE_MASK           0x1f

#define AETH_CODE_ACK   0
#define AETH_CODE_RNR   1
#define AETH_CODE_RSVD  2
#define AETH_CODE_NAK   3

#define RNR_NAK_TIMEOUT 0 // this will be popluted by HAL in rqcb2

#define AETH_SYNDROME_CODE_SHIFT    5
#define AETH_SYNDROME_GET(_dst, _code, _val) \
    or  _dst, _val, _code, AETH_SYNDROME_CODE_SHIFT

#define AETH_SYNDROME_GET_C(_dst, _code, _val, _c) \
    or._c  _dst, _val, _code, AETH_SYNDROME_CODE_SHIFT

#define AETH_NAK_SYNDROME_GET(_dst, _nak_code) \
    AETH_SYNDROME_GET(_dst, AETH_CODE_NAK, _nak_code)

#define AETH_NAK_SYNDROME_INLINE_GET(_nak_code) \
    ((AETH_CODE_NAK << AETH_SYNDROME_CODE_SHIFT) | (_nak_code))


#define AETH_ACK_SYNDROME_GET(_dst, _credits) \
    AETH_SYNDROME_GET(_dst, AETH_CODE_ACK, _credits)

#define AETH_ACK_SYNDROME_GET_C(_dst, _credits, _c) \
    AETH_SYNDROME_GET_C(_dst, AETH_CODE_ACK, _credits, _c)

#define AETH_RNR_SYNDROME_GET(_dst, _t) \
    AETH_SYNDROME_GET(_dst, AETH_CODE_RNR, _t)

#define AETH_RNR_SYNDROME_GET_C(_dst, _t, _c) \
    AETH_SYNDROME_GET_C(_dst, AETH_CODE_RNR, _t, _c)

#define AETH_RNR_SYNDROME_INLINE_GET(_t) \
    ((AETH_CODE_RNR << AETH_SYNDROME_CODE_SHIFT) | (_t))

//TODO perform log(credits) * 2 
#define RQ_CREDITS_GET(_credits, _tmp, _tmp_c) \
    add             _tmp, r0, d.log_num_wqes; \
    sllv            _tmp, 1, _tmp; \
    add             _credits, PROXY_RQ_P_INDEX, _tmp; \
    sub             _credits, _credits, PROXY_RQ_C_INDEX; \
    mincr           _credits, d.log_num_wqes, 0; \
    seq             _tmp_c, _credits, r0; \
    clz.!_tmp_c     _credits, _credits; \
    sub.!_tmp_c     _credits, 63, _credits; \
    sll.!_tmp_c     _credits, _credits, 1;

struct rdma_cnp_rsvd_t {
    rsvd        : 128;
};  

struct rdma_aeth_t {
    syndrome    : 8;
    msn         : 24;
};

struct rdma_atomicaeth_t {
    orig_data : 64;
};

struct rdma_immeth_t {
    data : 32;
};

struct rdma_xrceth_t {
    rsvd    : 8;
    xrcsrq  : 24;
};

struct ethhdr_t {
dmac       : 48;
sma        : 48;
ethertype  : 16;
};

union rdma_ext_t {

    struct rdma_immeth_t immeth;

    struct rdma_reth_t   reth;

    struct {
        struct rdma_reth_t   reth;
        struct rdma_immeth_t immeth;
    } reth_immeth;

    struct rdma_aeth_t   aeth;

    struct rdma_atomiceth_t  atomiceth;

    struct {
        struct rdma_aeth_t aeth;
        struct rdma_atomicaeth_t atomicaeth;
    } aeth_atomicaeth;

    struct rdma_ieth_t       ieth;

    struct {
        struct rdma_rdeth_t rdeth;
        struct rdma_deth_t  deth;
    } rdeth_deth;

    struct {
        struct rdma_rdeth_t  rdeth;
        struct rdma_deth_t   deth;
        struct rdma_immeth_t immeth;
    } rdeth_deth_immeth;

    struct {
        struct rdma_rdeth_t  rdeth;
        struct rdma_deth_t   deth;
        struct rdma_reth_t   reth;
    } rdeth_deth_reth;

    struct {
        struct rdma_rdeth_t  rdeth;
        struct rdma_deth_t   deth;
        struct rdma_reth_t   reth;
        struct rdma_immeth_t immeth;
    } rdeth_deth_reth_immeth;

    struct {
        struct rdma_rdeth_t  rdeth;
        struct rdma_aeth_t   aeth;
    } rdeth_aeth;

    struct {
        struct rdma_rdeth_t      rdeth;
        struct rdma_aeth_t       aeth;
        struct rdma_atomicaeth_t atomicaeth;
    } rdeth_aeth_atomicaeth;

    struct {
        struct rdma_rdeth_t     rdeth;
        struct rdma_atomiceth_t atomiceth;
    } rdeth_atomiceth;

    struct {
        struct rdma_deth_t       deth;
        struct ethhdr_t         eth;
    } deth_eth;

    struct {
        struct rdma_deth_t   deth;
        struct rdma_immeth_t immeth;
        struct ethhdr_t     eth;
    } deth_immeth_eth;
};

struct rdma_hdr_t {
    struct rdma_bth_t bth;
    union  rdma_ext_t ext;
};

struct p4_2_p4plus_app_hdr_t {
    app_type         : 4;
    flags            : 4;
    rdma_hdr_len     : 8;  // copied directly from p4 rdma table
//    opcode           : 8;  // from rdma.bth
    raw_flags        : 16; // copied directly from p4 rdma table
//    mac_da_inner     : 48; // from inner l2-hdr
//    mac_sa_inner     : 48; // from inner l2-hdr
    ecn              : 2;
    payload_len      : 14;
    struct rdma_hdr_t rdma_hdr;
};

struct rdma_hdr0_t {
    struct rdma_bth_t bth;
    pad : 16;
};

struct app_data0_0_t {
    app_type         : 4;
    flags            : 4;
    rdma_hdr_len     : 8;  // copied directly from p4 rdma table
    raw_flags        : 16; // copied directly from p4 rdma table
    ecn              : 2;
    payload_len      : 14;
    struct rdma_hdr0_t rdma_hdr;   // 112 bits
};

struct cq_rx_flags_t {
    _feedback: 1;
    _rsvd: 15;
};

#define CQ_RX_FLAG_RDMA_FEEDBACK       0x0001
#define AQ_RX_FLAG_RDMA_FEEDBACK       0x0002

struct req_rx_flags_t {
    _feedback: 1;
    _rsvd: 5;
    _atomic_aeth:1;
    _aeth:1;
    _completion:1;
    _ack:1;
    _read_resp:1;
    _only:1;
    _last:1;
    _middle:1;
    _first:1;
    _error_disable_qp:1;
};

#define REQ_RX_FLAG_ERR_DIS_QP         0x0001
#define REQ_RX_FLAG_FIRST              0x0002
#define REQ_RX_FLAG_MIDDLE             0x0004
#define REQ_RX_FLAG_LAST               0x0008
#define REQ_RX_FLAG_ONLY               0x0010
#define REQ_RX_FLAG_READ_RESP          0x0020
#define REQ_RX_FLAG_ACK                0x0040
#define REQ_RX_FLAG_COMPLETION         0x0080
#define REQ_RX_FLAG_AETH               0x0100
#define REQ_RX_FLAG_ATOMIC_AETH        0x0200

#define REQ_RX_FLAG_RDMA_FEEDBACK      0x8000

struct resp_rx_flags_t {
    _ud:1;
    _ring_dbell:1;
    _ack_req:1;
    _completion:1;
    _inv_rkey:1;
    _immdt:1;
    _atomic_cswap:1;
    _atomic_fna:1;
    _write:1;
    _read_req:1;
    _send:1;
    _only:1;
    _last:1;
    _middle:1;
    _first:1;
    _error_disable_qp:1;
};

#define RESP_RX_FLAG_ERR_DIS_QP         0x0001
#define RESP_RX_FLAG_FIRST              0x0002
#define RESP_RX_FLAG_MIDDLE             0x0004
#define RESP_RX_FLAG_LAST               0x0008
#define RESP_RX_FLAG_ONLY               0x0010
#define RESP_RX_FLAG_SEND               0x0020
#define RESP_RX_FLAG_READ_REQ           0x0040
#define RESP_RX_FLAG_WRITE              0x0080
#define RESP_RX_FLAG_ATOMIC_FNA         0x0100
#define RESP_RX_FLAG_ATOMIC_CSWAP       0x0200
#define RESP_RX_FLAG_IMMDT              0x0400
#define RESP_RX_FLAG_INV_RKEY           0x0800
#define RESP_RX_FLAG_COMPLETION         0x1000
#define RESP_RX_FLAG_ACK_REQ            0x2000
#define RESP_RX_FLAG_RING_DBELL         0x4000
#define RESP_RX_FLAG_UD                 0x8000

struct req_tx_flags_t {
    _ud:1;
    _inline:1;
    _rsvd2:1;
    _rsvd1:1;
    _inv_rkey:1;
    _immdt:1;
    _atomic_cswap:1;
    _atomic_fna:1;
    _write:1;
    _read_req:1;
    _send:1;
    _only:1;
    _last:1;
    _middle:1;
    _first:1;
    _error_disable_qp:1;
};

#define REQ_TX_FLAG_ERR_DIS_QP         0x0001
#define REQ_TX_FLAG_FIRST              0x0002
#define REQ_TX_FLAG_MIDDLE             0x0004
#define REQ_TX_FLAG_LAST               0x0008
#define REQ_TX_FLAG_ONLY               0x0010
#define REQ_TX_FLAG_SEND               0x0020
#define REQ_TX_FLAG_READ_REQ           0x0040
#define REQ_TX_FLAG_WRITE              0x0080
#define REQ_TX_FLAG_ATOMIC_FNA         0x0100
#define REQ_TX_FLAG_ATOMIC_CSWAP       0x0200
#define REQ_TX_FLAG_IMMDT              0x0400
#define REQ_TX_FLAG_INV_RKEY           0x0800
#define REQ_TX_FLAG_RSVD1              0x1000
#define REQ_TX_FLAG_RSVD2              0x2000
#define REQ_TX_FLAG_INLINE             0x4000
#define REQ_TX_FLAG_UD                 0x8000


struct resp_tx_flags_t {
    _error_disable_qp: 1;
    _only:1;
    _first:1;
    _middle:1;
    _last:1;
    _read_resp:1;
    _atomic_resp:1;
    _ack:1;
    _rsvd_flags:8;
};

#define RESP_TX_FLAG_ACK                0x0100
#define RESP_TX_FLAG_ATOMIC_RESP        0x0200
#define RESP_TX_FLAG_READ_RESP          0x0400
#define RESP_TX_FLAG_LAST               0x0800
#define RESP_TX_FLAG_MIDDLE             0x1000
#define RESP_TX_FLAG_FIRST              0x2000
#define RESP_TX_FLAG_ONLY               0x4000
#define RESP_TX_FLAG_ERR_DIS_QP         0x8000


#define ARE_ALL_FLAGS_SET(_c, _flags_r, _flags_test) \
    smeqh   _c, _flags_r, _flags_test, _flags_test
#define IS_ANY_FLAG_SET(_c, _flags_r, _flags_test) \
    smneh   _c, _flags_r, _flags_test, 0
#define IS_FLAG_NOT_SET(_c, _flags_r, _flags_test) \
    smeqh   _c, _flags_r, _flags_test, 0
#define IS_FLAG_NOT_SET_C(_c1, _flags_r, _flags_test, _c2) \
    smeqh._c2   _c1, _flags_r, _flags_test, 0

#define ARE_ALL_FLAGS_SET_B(_c, _flags, _flags_test) \
    smeqb   _c, _flags, _flags_test, _flags_test
#define IS_ANY_FLAG_SET_B(_c, _flags, _flags_test) \
    smneb   _c, _flags, _flags_test, 0

#define IS_MASKED_VAL_EQUAL(_c, _cmp_r, _mask, _val) \
    smeqh   _c, _cmp_r, _mask, _val

#define IS_MASKED_VAL_EQUAL_B(_c, _cmp_r, _mask, _val) \
    smeqb   _c, _cmp_r, _mask, _val

#define RDMA_OPC_SERV_TYPE_SHIFT               5

#define RDMA_SERV_TYPE_RC                      0
#define RDMA_SERV_TYPE_UD                      3
#define RDMA_SERV_TYPE_CNP                     4

#define RDMA_PKT_OPC_SEND_FIRST                0
#define RDMA_PKT_OPC_SEND_MIDDLE               1
#define RDMA_PKT_OPC_SEND_LAST                 2
#define RDMA_PKT_OPC_SEND_LAST_WITH_IMM        3
#define RDMA_PKT_OPC_SEND_ONLY                 4
#define RDMA_PKT_OPC_SEND_ONLY_WITH_IMM        5
#define RDMA_PKT_OPC_RDMA_WRITE_FIRST          6
#define RDMA_PKT_OPC_RDMA_WRITE_MIDDLE         7
#define RDMA_PKT_OPC_RDMA_WRITE_LAST           8
#define RDMA_PKT_OPC_RDMA_WRITE_LAST_WITH_IMM  9
#define RDMA_PKT_OPC_RDMA_WRITE_ONLY           10
#define RDMA_PKT_OPC_RDMA_WRITE_ONLY_WITH_IMM  11
#define RDMA_PKT_OPC_RDMA_READ_REQ             12
#define RDMA_PKT_OPC_RDMA_READ_RESP_FIRST      13
#define RDMA_PKT_OPC_RDMA_READ_RESP_MID        14
#define RDMA_PKT_OPC_RDMA_READ_RESP_LAST       15
#define RDMA_PKT_OPC_RDMA_READ_RESP_ONLY       16
#define RDMA_PKT_OPC_ACK                       17
#define RDMA_PKT_OPC_ATOMIC_ACK                18
#define RDMA_PKT_OPC_CMP_SWAP                  19
#define RDMA_PKT_OPC_FETCH_ADD                 20
#define RDMA_PKT_OPC_RESYNC                    21
#define RDMA_PKT_OPC_SEND_LAST_WITH_INV        22
#define RDMA_PKT_OPC_SEND_ONLY_WITH_INV        23

#define RDMA_PKT_OPC_CNP                       129

union roce_opcode_flags_t {
    flags: 16;
    struct req_rx_flags_t req_rx;
    struct resp_rx_flags_t resp_rx;
    struct req_tx_flags_t req_tx;
    struct resp_tx_flags_t resp_tx;
}; 

struct ack_info_t {
    psn: 24;
    msn: 24;
    syndrome: 8; // used for NAK
    credits: 5; // used for ACK
    rsvd: 3;
};

#define SGE_T struct sge_t
#define SIZEOF_SGE_T 16 //Bytes
#define SIZEOF_SGE_T_BITS (SIGEOF_SGE_T * BITS_PER_BYTE)
#define LOG_SIZEOF_SGE_T   4   // 2^4 = 16 Bytes
#define LOG_SIZEOF_SGE_T_BITS   (LOG_SIZEOF_SGE_T + LOG_BITS_PER_BYTE)
struct sge_t {
    va: 64;
    len: 32;
    l_key: 32;
};

#define OP_TYPE_SEND                0
#define OP_TYPE_SEND_INV            1
#define OP_TYPE_SEND_IMM            2
#define OP_TYPE_READ                3
#define OP_TYPE_WRITE               4
#define OP_TYPE_WRITE_IMM           5
#define OP_TYPE_CMP_N_SWAP          6
#define OP_TYPE_FETCH_N_ADD         7
#define OP_TYPE_FRPMR               8
#define OP_TYPE_LOCAL_INV           9
#define OP_TYPE_BIND_MW             10
#define OP_TYPE_SEND_INV_IMM        11 // vendor specific
#define OP_TYPE_FRMR                12

#define OP_TYPE_RDMA_OPER_WITH_IMM 16
#define OP_TYPE_SEND_RCVD          17
#define OP_TYPE_INVALID            18

#define TXWQE_SGE_OFFSET  32 //
#define TXWQE_SGE_OFFSET_BITS   256 // 32 * 8

struct sqwqe_base_t {
    wrid               : 64;
    op_type            : 4;
    complete_notify    : 1;
    fence              : 1;
    solicited_event    : 1;
    inline_data_vld    : 1;
    num_sges           : 8;
    color              : 1;
    rsvd2              : 15;
};

// RC send
struct sqwqe_send_t {
    imm_data           : 32;
    inv_key            : 32;
    rsvd1              : 32;  // to align length to match position in write/read reqs
    length             : 32;
    rsvd2              : 32;    // for now
};

// UD send
struct sqwqe_ud_send_t {
    imm_data           : 32;
    q_key              : 32;
    length             : 32;
    dst_qp             : 24;
    rsvd               :  8;
    ah_handle          : 32;
};

// write
struct sqwqe_write_t {
    imm_data           : 32;
    va                 : 64;
    length             : 32;
    r_key              : 32;
};

//Atomic
struct sqwqe_atomic_t {
    r_key             : 32;
    va                : 64;
    swap_or_add_data  : 64;
    cmp_data          : 64;
    pad               : 64;
    struct sge_t sge;
};

// Read
struct sqwqe_read_t {
    rsvd               : 32;
    va                 : 64;
    length             : 32;
    r_key              : 32;
};

// Local Invalidate
struct sqwqe_local_inv_t {
    l_key              : 32;    
    pad                : 128;
};


// FRMR
struct sqwqe_frmr_t {
    mr_id              : 24;
    type               :  3;
    dma_src_address    : 64;
    num_entries        : 32;
    pad                : 37;
};

// Bind Memory Window
struct sqwqe_bind_mw_t {
    va                 : 64;
    len                : 32;
    l_key              : 32;
    r_key              : 32;
    new_r_key_key      : 8;
    access_ctrl        : 8;
    mw_type            : 2;
    zbva               : 1;
    pad                : 237;
};

// Fast-Register Physical MR (FRPMR)
struct sqwqe_frpmr_t {
    l_key              : 32;
    new_user_key       : 8;
    access_ctrl        : 8;
    log_page_size      : 8;
    num_pt_entries     : 32;
    base_va            : 64;
    dma_src_address    : 64;
    len                : 64;
    pt_start_offset    : 3;
    zbva               : 1;
    mw_en              : 1;
    pad                : 131;
};

struct sqwqe_t {
    struct sqwqe_base_t base;
    union {
        struct sqwqe_atomic_t atomic;
        struct sqwqe_bind_mw_t bind_mw;
        struct sqwqe_frpmr_t frpmr;
        struct {
            union {
                struct sqwqe_send_t send;
                struct sqwqe_ud_send_t ud_send;
                struct sqwqe_write_t write;
                struct sqwqe_read_t read;
                struct sqwqe_local_inv_t local_inv;
                struct sqwqe_frmr_t frmr;
            };
            union {
                pad : 256;
                inline_data: 256;
            };
        };
    };
};

#define MAX_RD_ATOMIC 16
#define LOG_MAX_RD_ATOMIC 4
#define LOG_RRQ_WQE_SIZE 5
#define LOG_RRQ_QP_SIZE (LOG_MAX_RD_ATOMIC + LOG_RRQ_WQE_SIZE)
struct rrqwqe_read_t {
    len                : 32;
    wqe_sge_list_addr  : 64;
    pad                : 96; 
};

struct rrqwqe_atomic_t {
    struct sge_t sge;
    op_type            : 8;
    pad                : 56;

};

#define RRQ_OP_TYPE_READ 0
#define RRQ_OP_TYPE_ATOMIC 1

struct rrqwqe_t {
    read_rsp_or_atomic : 1;
    rsvd               : 7;
    num_sges           : 8;
    psn                : 24;
    msn                : 24;
    union {
        struct rrqwqe_read_t   read;
        struct rrqwqe_atomic_t atomic;
    };
};

struct rrqwqe_d_t {
    read_rsp_or_atomic : 1;
    rsvd               : 7;
    num_sges           : 8;
    psn                : 24;
    msn                : 24;
    union {
        struct rrqwqe_read_t   read;
        struct rrqwqe_atomic_t atomic;
    };
    // pad added for easy access of d[] in mpu program
    pad                : 256; 
};

#define RQWQE_SGE_OFFSET  32
#define RQWQE_SGE_OFFSET_BITS 256 // 32 * 8
#define LOG_RQWQE_SGE_OFFSET 5 // 2^5 = 32
#define LOG_RQWQE_SGE_OFFSET_BITS 8 // 2^8 = 256

struct rqwqe_base_t {
    wrid: 64;
    num_sges: 8;
    rsvd: 184;
    rsvd2:256;
};

#define     CQ_STATUS_SUCCESS               0
#define     CQ_STATUS_LOCAL_LEN_ERR         1
#define     CQ_STATUS_LOCAL_QP_OPER_ERR     2
#define     CQ_STATUS_LOCAL_PROT_ERR        3
#define     CQ_STATUS_WQE_FLUSHED_ERR       4
#define     CQ_STATUS_MEM_MGMT_OPER_ERR     5
#define     CQ_STATUS_BAD_RESP_ERR          6
#define     CQ_STATUS_LOCAL_ACC_ERR         7
#define     CQ_STATUS_REMOTE_INV_REQ_ERR    8
#define     CQ_STATUS_REMOTE_ACC_ERR        9
#define     CQ_STATUS_REMOTE_OPER_ERR       10
#define     CQ_STATUS_RETRY_EXCEEDED        11
#define     CQ_STATUS_RNR_RETRY_EXCEEDED    12
#define     CQ_STATUS_XRC_VIO_ERR           13

struct cqe_t {

    union {
        struct {
            rsvd: 160;
            old_sq_cindex: 16;
            old_rq_cq_cindex: 16;
        } admin;
        struct {
            wrid: 64;
            op_type: 8;
            src_qp: 24;
            smac: 48;
            pkey_index: 16;
            union {
                imm_data: 32;
                r_key: 32;
            };
        } recv;
        struct {
            rsvd1: 32;
            msn:   32;
            rsvd2: 64;
            wrid: 64;
        } send;
    };
    union {
        status: 32;
        length: 32;
    };
    qid: 24;
    type: 3;
    union {
        struct {
            rsvd: 3;
        } admin_flags;
        struct {
            imm_data_vld: 1;
            rkey_inv_vld: 1;
            ipv4: 1;
        } recv_flags;
        struct {
            rsvd: 3;
        } send_flags;
    };
    error: 1;
    color: 1;
};

#define CQE_TYPE_ADMIN 0
#define CQE_TYPE_RECV  1
#define CQE_TYPE_SEND_MSN 2
#define CQE_TYPE_SEND_NPG 3


# Keep it compatible with - platform/drivers/linux/rdma/drv/ionic/ionic_fw.h

#define EQE_CODE_WIDTH 4

#define EQE_TYPE_CQ 0

//0-7 for good cases, 8-15 for error cases
#define EQE_CODE_CQ_NOTIFY      0
#define EQE_CODE_CQ_ERR_FULL    8

#define EQE_TYPE_QP 1

#define EQE_CODE_QP_SRQ_LEVEL   0
#define EQE_CODE_QP_SQ_DRAIN    1
#define EQE_CODE_QP_COMM_EST    2
#define EQE_CODE_QP_LAST_WQE    3

#define EQE_CODE_QP_ERR         8
#define EQE_CODE_QP_ERR_NOCQE   9
#define EQE_CODE_QP_ERR_REQEST 10
#define EQE_CODE_QP_ERR_ACCESS 11


struct eqwqe_t {
    qid: 24;
    code: 4;
    type: 3;
    color: 1;
};

struct rsqwqe_read_t {
    r_key: 32;
    va: 64;
    len: 32;
    offset: 32;
};

struct rsqwqe_atomic_t {
    r_key: 32;
    va: 64;
    orig_data: 64;
};

#define RSQ_OP_TYPE_READ 0
#define RSQ_OP_TYPE_ATOMIC 1

#define MAX_DEST_RD_ATOMIC 16
#define LOG_MAX_DEST_RD_ATOMIC 4
#define LOG_SIZEOF_RSQWQE_T 5 // 2^5 = 32 bytes
#define LOG_RSQ_QP_SIZE (LOG_SIZEOF_RSQWQE_T + LOG_MAX_DEST_RD_ATOMIC)

struct rsqwqe_d_t {
    read_or_atomic: 1;
    rsvd1: 7;
    psn: 24;
    rsvd2: 64;
    union {
        struct rsqwqe_read_t read;
        struct rsqwqe_atomic_t atomic;
    };
    
    // this pad added such that when it gets loaded in the d-vector, it is easy to parse the fields
    pad: 256; 
};

#define RSQWQE_ORIG_DATA_OFFSET  24
#define RSQWQE_R_KEY_OFFSET     12
#define RSQWQE_VA_OFFSET        16

struct rsqwqe_t {
    read_or_atomic: 1;
    rsvd1: 7;
    psn: 24;
    rsvd2: 64;
    union {
        struct rsqwqe_read_t read;
        struct rsqwqe_atomic_t atomic;
    };
};

#define ACC_CTRL_LOCAL_WRITE        0x1
#define ACC_CTRL_REMOTE_WRITE       0x2
#define ACC_CTRL_REMOTE_READ        0x4
#define ACC_CTRL_REMOTE_ATOMIC      0x8

#define INVALID_KEY                 0xFFFFFF //24b

//KEY_STATE
#define KEY_STATE_INVALID   0
#define KEY_STATE_FREE      1
#define KEY_STATE_VALID     2

#define KEY_INDEX_MASK      0xffffff00
#define KEY_INDEX_SHIFT     8
#define KEY_USER_KEY_MASK   0x000000ff
#define KEY_USER_KEY_SHIFT  0

//MR_TYPE
#define MR_TYPE_MR             0
#define MR_TYPE_MW_TYPE_1      1
#define MR_TYPE_MW_TYPE_2      2
#define MR_TYPE_MW_TYPE_1_OR_2 3
//#define MR_TYPE_MW_TYPE_2B  3

//MR_FLAG
#define MR_FLAG_MW_EN   0x1 // is memory window enabled ?
#define MR_FLAG_INV_EN  0x2 // is memory invalidation enabled ?
#define MR_FLAG_ZBVA    0x4 //is it a zbva ?
#define MR_FLAG_UKEY_EN 0x8 //user-key is enabled

// Bit positions of MR Flags
#define LOG_MR_FLAG_MW_EN    0 
#define LOG_MR_FLAG_INV_EN   1
#define LOG_MR_FLAG_ZBVA     2
#define LOG_MR_FLAG_UKEY_EN  3

#define KEY_ENTRY_T struct key_entry_t
#define LOG_SIZEOF_KEY_ENTRY_T  6   // 2^6 = 64 bytes
#define LOG_SIZEOF_KEY_ENTRY_T_BITS (LOG_SIZEOF_KEY_ENTRY_T + LOG_BITS_PER_BYTE)

// index = lkey >> KEY_INDEX_SHIFT
#define KEY_INDEX_GET(_key_index_r, _key_r) \
    srl     _key_index_r, _key_r, KEY_INDEX_SHIFT

#define KEY_USER_KEY_GET(_reg, _key_r) \
    and     _reg, _key_r, 0xff

// entry_addr = base_addr + (index * sizeof(key_entry_t))
#define KEY_ENTRY_ADDR_GET(_entry_addr_r, _kt_base_r, _key_index_r) \
    add     _entry_addr_r, _kt_base_r, _key_index_r[31:8], LOG_SIZEOF_KEY_ENTRY_T;

// aligned_entry_addr =  entry_addr & ~HBM_CACHE_LINE_MASK;
#define KEY_ENTRY_ALIGNED_ADDR_GET(_aligned_entry_addr_r, _entry_addr_r) \
    sub     _aligned_entry_addr_r, _entry_addr_r, _entry_addr_r[5:0];

//key_id = (key_addr % HBM_CACHE_LINE_SIZE) / sizeof(key_entry_t);
#define KEY_ID_GET(_key_id_r, _key_addr_r) \
    srl     _key_id_r, _key_addr_r[5:0], LOG_SIZEOF_KEY_ENTRY_T


struct key_entry_t {
    user_key: 8;
    state: 4;
    type: 4;
    acc_ctrl: 8;
    log_page_size: 8;
    len: 32;
    base_va: 64;
    pt_base: 32;
    pd: 32;
    host_addr: 1;
    override_lif_vld: 1;
    override_lif: 12;
    rsvd1: 18;
    flags: 8;
    qp: 24; //qp which bound the MW ?
    mr_l_key: 32;
    mr_cookie: 32;
    num_pt_entries_rsvd: 32;
    rsvd2: 160;
};

struct key_entry_aligned_t {
    user_key: 8;
    state: 4;
    type: 4;
    acc_ctrl: 8;
    log_page_size: 8;
    len: 32;
    base_va: 64;
    pt_base: 32;
    pd: 32;
    host_addr: 1;
    override_lif_vld: 1;
    override_lif: 12;
    rsvd1: 18;
    flags: 8;
    qp: 24; //qp which bound the MW ?
    mr_l_key: 32;
    mr_cookie: 32;
    num_pt_entries_rsvd: 32;
    // pad added for easy access of d[] in mpu program
    rsvd2: 160;
};

#define GET_NUM_PAGES(_va_r, _bytes_r, _page_size_imm, _num_pages_r, _scratch_r)  \
     add    _num_pages_r, _va_r, _bytes_r;                                        \
     srl    _num_pages_r, _num_pages_r, _page_size_imm;                           \
     srl    _scratch_r, _va_r, _page_size_imm;                                    \
     sub    _num_pages_r, _num_pages_r, _scratch_r;                               \
     add    _num_pages_r, _num_pages_r, 1;


struct udphdr_t {
    sport   : 16;
    dport   : 16;
    length  : 16;
    csum    : 16;
};

struct iphdr_t {
    version     : 4;
    ihl         : 4;
    tos         : 8;
    tot_len     : 16;
    id          : 16;
    frag_off    : 16;
    ttl         : 8;
    protocol    : 8;
    check       : 16;
    saddr       : 32;
    daddr       : 32;
/*The options start here. */
};

struct ipv6hdr_t {
    version     : 4;
    tc          : 8;
    flow_label  : 20;
    payload_len : 16;
    nh          : 8;
    hop_limit   : 8;
    saddr       : 128;
    daddr       : 128;
};


struct vlanhdr_t {
    pri            : 3;
    cfi            : 1;
    vlan           : 12;
    ethertype      : 16;
};


struct header_template_v4_t {
    struct ethhdr_t     eth; 
    struct vlanhdr_t    vlan;
    struct iphdr_t      ip;
    struct udphdr_t     udp;
};

struct header_template_v6_t {
    struct ethhdr_t     eth; 
    struct vlanhdr_t    vlan;
    struct ipv6hdr_t    ip;
    struct udphdr_t     udp;
};

union header_template_t {
    struct header_template_v4_t v4;
    struct header_template_v6_t v6;
};
    
#define HDR_TEMPLATE_T struct header_template_t
#define HDR_TEMPLATE_T_SIZE_BYTES (sizeof(struct header_template_t)/8)
#define AH_ENTRY_T_SIZE_BYTES (HDR_TEMPLATE_T_SIZE_BYTES + 1)
#define AT_ENTRY_SIZE_BYTES 136
#define GRH_HDR_T_SIZE_BYTES (sizeof(struct ipv6hdr_t)/8)
 
#define ACK_SYNDROME        0x00
#define RNR_SYNDROME        0x20
#define RESV_SYNDROME       0x40
#define NAK_SYNDROME        0x60
#define SYNDROME_MASK       0x60

#define SYNDROME_CREDITS_MASK 0x1F

#define DECODE_ACK_SYNDROME_CREDITS(_credits_r, _syndrome, _cf)         \
    seq            _cf, _syndrome[4:0], r0;                             \
    srl            _credits_r, _syndrome[4:0], 1;                       \
    sllv.!_cf      _credits_r, 1, _credits_r;


/* Definitions for RDMA Feedback phv */
struct phv_intr_global_t {
    tm_iport : 4;
    tm_oport : 4;
    tm_iq : 5;
    lif : 11;
    timestamp : 48;
    tm_span_session : 8;
    tm_replicate_ptr : 16;
    tm_replicate_en : 1;
    tm_cpu : 1;
    tm_q_depth : 14;
    drop : 1;
    bypass : 1;
    hw_error : 1;
    tm_oq : 5;
    debug_trace : 1;
    csum_err : 5;
    error_bits : 6;
    tm_instance_type : 4;
};

struct phv_intr_p4_t {
    crc_err : 1;
    len_err : 4;
    recirc_count : 3;
    parser_err : 1;
    p4_pad : 1;
    frame_size : 14;
    no_data : 1;
    recirc : 1;
    packet_len : 14;
};

struct phv_intr_rxdma_t {
    intr_qid : 24;
    intr_dma_cmd_ptr : 6;
    intr_qstate_addr : 34;
    intr_qtype : 3;
    intr_rx_splitter_offset : 10;
    intr_rxdma_rsv : 3;
};

struct p4_to_p4plus_roce_header_t {
    p4plus_app_id : 4;
    table0_valid : 1;
    table1_valid : 1;
    table2_valid : 1;
    table3_valid : 1;
    //Parsed UDP options valid flags
    roce_opt_ts_vld   : 1;
    roce_opt_msss_vld : 1;
    rdma_hdr_len : 6;
    raw_flags    : 16;
    ecn          : 2;
    payload_len  : 14;
    //Parsed UDP options
    roce_opt_timestamp  : 64;
    roce_opt_new        : 32;
    roce_int_recirc_hdr : 16;
};

struct rdma_aq_feedback_create_qp_ext_t {
    rq_dma_addr: 64;
    rq_id      : 24;
    rsvd: 24;
};
        
struct aq_p4_to_p4plus_roce_header_t {
    p4plus_app_id : 4;
    table0_valid : 1;
    table1_valid : 1;
    table2_valid : 1;
    table3_valid : 1;
        //Parsed UDP options valid flags
    roce_opt_ts_vld   : 1;
    roce_opt_msss_vld : 1;
    rdma_hdr_len : 6;
    raw_flags    : 16;
    ecn          : 2;
    payload_len  : 14;
    
    // 14 bytes to use for AQ params

    union {
        struct rdma_aq_feedback_create_qp_ext_t create_qp_ext;
    };
};

//Common DCQCN CB for both req and resp paths.
struct dcqcn_cb_t {
    // CNP generation params.
    last_cnp_timestamp: 48;

    /* DCQCN algorithm params*/

    // Configurable params.
    byte_counter_thr:       32; // byte-counter-threshold in Bytes. (Bc)
    timer_exp_thr:          16; // Timer T threshold in terms of alpha-timer.
    g_val:                  16; // constant g.
    // Algorithm computed params.
    rate_enforced:          32; // Enforced rate in Mbps. (Rc)
    target_rate:            32; // Target rate in Mbps. (Rt)
    alpha_value:            16; // rate-reduction-factor alpha val in int.
    // Helper counters.
    cur_byte_counter:       32; // cur-byte-counter value in Bytes.
    byte_counter_exp_cnt:   16; // Num of times byte-counter expired.
    timer_exp_cnt:          16; // Num of times timer T expired.(T)
    num_alpha_exp_cnt:      16; // Num times alpha-timer expired since timer T expired.
    num_cnp_rcvd:           8;  // Num of CNP received used by rate-compute-ring for processing CNPs.
    num_cnp_processed:      8;  // Num of CNP processed used by rate-compute-ring.
    max_rate_reached:       1;  // This will be set if we have reached max-qp-rate and dcqcn rate-increase timers are stopped.
    log_sq_size:            5;
    rsvd0:                  2;

    // Rate-limiter token-bucket related params.
    last_sched_timestamp:   48;
    delta_ticks_last_sched: 16;
    cur_avail_tokens:       48;
    token_bucket_size:      48; // DCQCN enforced BC (committed-burst) in bits.
    sq_cindex:              16;

    // For model testing only.
    num_sched_drop: 8; // Number of times packet was scheduled and dropped due to insufficient tokens.
    cur_timestamp:  32; // For debugging on Model since model doesnt have timestamps
};

#define RDMA_COMPLETION_FEEDBACK      0x1
#define RDMA_CQ_ARM_FEEDBACK          0x2
#define RDMA_TIMER_EXPIRY_FEEDBACK    0x3
#define RDMA_AQ_FEEDBACK              0x4
#define RDMA_SQ_DRAIN_FEEDBACK        0x5

struct rdma_feedback_t {
    feedback_type:8;
    union {
        /* TYPE: RDMA_COMPLETION_FEEDBACK */
        struct {
            wrid: 64;
            status: 8;
            error: 1;
            pad: 7;
        }completion;
        /* Type: RDMA_CQ_ARM_FEEDBACK */
        struct {
            cindex: 16; /* arm cindex */
            color : 1;
            arm   : 1;
            sarm  : 1;
            pad   : 61;
        }arm;
        /* TYPE: RDMA_TIMER_EXPIRY_FEEDBACK */
        struct {
            rexmit_psn: 24;
            ssn: 24;
            tx_psn: 24;
            pad: 8;
        }timer_expiry;
        /* TYPE: RDMA_AQ_FEEDBACK */
        struct {
            cq_num: 24; 
            status: 8;
            error:  1;
            pad: 47;
        }aq_completion;
        /* TYPE: RDMA_SQ_DRAIN_FEEDBACK */
        struct {
            tx_psn: 24;
            ssn: 24;
            pad: 32;
        }sq_drain;
    }; 
};

struct rdma_aq_feedback_t {
    feedback_type: 8;
    struct {
        cq_num: 24; 
        status:  8;
        error:   1;
        op:      8;
        pad:     7;
    }aq_completion;
    union {
        struct {
            rq_cq_id: 24;
            rq_depth_log2: 8;
            rq_stride_log2: 8;
            rq_page_size_log2: 8;
            rq_tbl_index: 32;
            rq_map_count: 32;
            pd:           32;
            rq_type_state: 8;
            rq_id        : 24;
        } create_qp;
        pad: 176;
    };
};

#define RDMA_FEEDBACK_SPLITTER_OFFSET  \
    ((sizeof(struct phv_intr_global_t) + sizeof(struct phv_intr_p4_t) + sizeof(struct phv_intr_rxdma_t) + sizeof(struct p4_to_p4plus_roce_header_t) + sizeof(struct rdma_feedback_t)) >> 3)

#define RDMA_AQ_FEEDBACK_SPLITTER_OFFSET                                   \
    ((sizeof(struct phv_intr_global_t) + sizeof(struct phv_intr_p4_t) + sizeof(struct phv_intr_rxdma_t) + sizeof(struct p4_to_p4plus_roce_header_t) + sizeof(struct rdma_aq_feedback_t)) >> 3)

struct rdma_atomic_resource_t {
    data0: 64;
    data1: 64;
    data2: 64;
    data3: 64;
    pad0: 64;
    pad1: 64;
    pad2: 64;
    pad3: 64;
};

#define PCIE_ATOMIC_TYPE_FNA    0xc
#define PCIE_ATOMIC_TYPE_SWAP   0xd
#define PCIE_ATOMIC_TYPE_CSWAP  0xe

#define PCIE_TLP_LEN_FNA    2   //2 double words = 8B
#define PCIE_TLP_LEN_CSWAP  4   //4 double words = 16B

struct rdma_pcie_atomic_reg_t {
    compare_data_or_add_data: 64;
    swap_data: 64;
    rsvd_operand_data: 128;
    host_addr: 64;
    atomic_type: 8;
    rsvd2: 24;
    tlp_len: 8;
    rsvd3: 152;
};

struct resp_bt_info_t {
    read_or_atomic: 1;
    rsvd: 7;
    psn: 24;
    va: 64;
    r_key: 32;
    len: 32;
};


// the state numbers are assigned this way so that assembly code can use 
// single < or > operation for fast path checks
#define QP_STATE_RESET      0
#define QP_STATE_INIT       1
#define QP_STATE_ERR        2
#define QP_STATE_RTR        3
#define QP_STATE_RTS        4
#define QP_STATE_SQ_ERR     5
#define QP_STATE_SQD        6
#define QP_STATE_SQD_ON_ERR 7

#define AQ_OP_TYPE_NOP          0
#define AQ_OP_TYPE_CREATE_CQ    1
#define AQ_OP_TYPE_CREATE_QP    2
#define AQ_OP_TYPE_REG_MR       3
#define AQ_OP_TYPE_STATS_HDRS   4
#define AQ_OP_TYPE_STATS_VALS   5
#define AQ_OP_TYPE_DEREG_MR     6
#define AQ_OP_TYPE_RESIZE_CQ    7
#define AQ_OP_TYPE_DESTROY_CQ   8
#define AQ_OP_TYPE_MODIFY_QP    9
#define AQ_OP_TYPE_QUERY_QP     10
#define AQ_OP_TYPE_DESTROY_QP   11
#define AQ_OP_TYPE_STATS_DUMP   12

//Define all stat types requested by the driver
#define AQ_STATS_DUMP_TYPE_QP   0
#define AQ_STATS_DUMP_TYPE_CQ   1
#define AQ_STATS_DUMP_TYPE_EQ   2

struct aqwqe_t {
	op: 8;
    type_state: 8;
    dbid_flags: 16;
    id_ver: 32;
	union {
		struct {
			dma_addr: 64;
			length: 32;
			rsvd: 352;
		} stats;
		struct {
			dma_addr: 64;
			rsvd: 384;
		} ah;
		struct {
			va: 64;
			length: 64;
			pd_id: 32;
            access_flags: 16;            
			rsvd: 128;
			dir_size_log2: 8;
			page_size_log2: 8;
			tbl_index: 32;
			map_count: 32;
			dma_addr: 64;
		} mr;
		struct {
			eq_id: 32;
			depth_log2: 8;
			stride_log2: 8;
			dir_size_log2_rsvd: 8;
			page_size_log2: 8;
			rsvd: 256;
			tbl_index: 32;
			map_count: 32;
			dma_addr: 64;
		} cq;
		struct {
			pd_id: 32;
			access_perms_flags: 16;
			access_perms_rsvd: 16;
			sq_cq_id: 32;
			sq_depth_log2: 8;
			sq_stride_log2: 8;
			sq_dir_size_log2_rsvd: 8;
			sq_page_size_log2: 8;
			sq_tbl_index_xrcd_id: 32;
			sq_map_count: 32;
			sq_dma_addr:64 ;
			rq_cq_id: 32;
			rq_depth_log2: 8;
			rq_stride_log2: 8;
			rq_dir_size_log2_rsvd: 8;
			rq_page_size_log2: 8;
			rq_tbl_index_srq_id: 32;
			rq_map_count: 32;
			rq_dma_addr: 64;
		} qp;
		struct {
			pmtu: 4;
            rsvd1: 4;
            rnr_try: 4;
			retry: 4;
			rnr_timer: 8;
			retry_timeout: 8;
			access_perms_flags: 16;
			access_perms_mask: 16;
			rq_psn: 32;
			sq_psn: 32;
			qkey_dest_qpn: 32;
			rate_limit_kbps: 32;
			rsq_depth: 8;
			rrq_depth: 8;
			pkey_id: 16;
            ah_id: 24;
			ah_id_len: 8;
			rsvd2 : 128;
			dma_addr: 64;
		} mod_qp;
	};
};

#define AQ_WQE_T_LOG_SIZE_BYTES 6
#endif //__TYPES_H
