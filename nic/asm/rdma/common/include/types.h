#ifndef __TYPES_H
#define __TYPES_H

#define HBM_NUM_KEY_ENTRIES_PER_CACHE_LINE 2

#define RDMA_RESERVED_LKEY_ID 0
#define RDMA_EQ_ID_ASYNC 0

#define RDMA_AQ_QID_START 1

#define HBM_NUM_PT_ENTRIES_PER_CACHE_LINE 8
#define HBM_PAGE_SIZE_SHIFT 12 // HBM page size is assumed as 4K
#define HBM_PAGE_SIZE 4096
#define HBM_PAGE_ALIGN_MASK ~(HBM_PAGE_SIZE - 1)
#define BARMAP_BASE_SHIFT 23 // Barmap is 8M aligned
#define BARMAP_SIZE_SHIFT 23 // Barmap is in units of 8M
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
#define DCQCN_CFG_ADDR_SHIFT 3
#define RRQ_BASE_ADDR_SHIFT 3
#define RSQ_BASE_ADDR_SHIFT 3
#define HBM_SQ_BASE_ADDR_SHIFT 3
#define HBM_RQ_BASE_ADDR_SHIFT 3
#define CQCB_ADDR_HI_SHIFT 10 // 24 bits of cqcb base addr, so shift 10 bits
#define SQCB_ADDR_HI_SHIFT 10 // 24 bits of cqcb base addr, so shift 10 bits
#define RQCB_ADDR_HI_SHIFT 10 // 24 bits of cqcb base addr, so shift 10 bits
#define PHY_BASE_ADDR_SHIFT 12

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
#define CB2_OFFSET_BYTES    (2 * 64)
#define CB3_OFFSET_BYTES    (3 * 64)
#define CB4_OFFSET_BYTES    (4 * 64)
#define TOTAL_CB_BYTES      (8 * 64)
#define PAGE_SIZE_4K        4096

#define TOTAL_AQCB_BYTES    (2 * 64)

#define LOG_SIZEOF_EQCB_T   6   // 2^6 = 64 Bytes

#define LOG_SIZEOF_SQCB_T   9
#define LOG_SIZEOF_RQCB_T   9

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

// This will be made DCQCN configurable parameter.
// Unit is micro-seconds converted to core-clock-ticks. so 50us will be (50 * 833) = 41650
#define DCQCN_MIN_TIME_BTWN_CNPS 41650

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
#define RQ_CREDITS_GET(_credits, _spec_cindex, _tmp1, _tmp2, _tmp_c) \
    sub             _credits, PROXY_RQ_P_INDEX, _spec_cindex; \
    mincr           _credits, d.log_num_wqes, 0; \
    sle             _tmp_c, _credits, 1; \
    clz.!_tmp_c     _tmp1, _credits; \
    sub.!_tmp_c     _tmp1, 63, _tmp1; \
    sub.!_tmp_c     _tmp2, _tmp1, 1; \
    srl.!_tmp_c     _tmp2, _credits, _tmp2; \
    sll.!_tmp_c     _credits, _tmp1, 1; \
    add.!_tmp_c     _credits, _credits, _tmp2[0];

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
    _rexmit:1;
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
#define REQ_TX_FLAG_REXMIT             0x1000
#define REQ_TX_FLAG_RSVD2              0x2000
#define REQ_TX_FLAG_INLINE             0x4000
#define REQ_TX_FLAG_UD                 0x8000


struct resp_tx_flags_t {
    _rsvd_flags:8;
    _atomic_resp:1;
    _read_resp:1;
    _ack:1;
    _only:1;
    _last:1;
    _middle:1;
    _first:1;
    _error_disable_qp: 1;
};

#define RESP_TX_FLAG_ERR_DIS_QP         0x0001
#define RESP_TX_FLAG_FIRST              0x0002
#define RESP_TX_FLAG_MIDDLE             0x0004
#define RESP_TX_FLAG_LAST               0x0008
#define RESP_TX_FLAG_ONLY               0x0010
#define RESP_TX_FLAG_ACK                0x0020
#define RESP_TX_FLAG_READ_RESP          0x0040
#define RESP_TX_FLAG_ATOMIC_RESP        0x0080

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

struct req_tx_phv_global_pad_t {
    spec_cindex: 16;
    rsvd: 6;
};

struct resp_rx_phv_global_pad_t {
    spec_cindex: 16;
    log_num_kt_entries: 5;
    pad: 1;
};

union phv_global_pad_t {
    pad: 22;
    struct req_tx_phv_global_pad_t req_tx;
    struct resp_rx_phv_global_pad_t resp_rx;
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

#define WQE_8x4_T struct len_encoding_8x4_t
#define SIZEOF_WQE_8x4_T 4 //Bytes
#define SIZEOF_WQE_8x4_T_BITS (SIGEOF_WQE_8x4_T * BITS_PER_BYTE)
#define LOG_SIZEOF_WQE_8x4_T   2   // 2^2 = 4 Bytes
#define LOG_SIZEOF_WQE_8x4_T_BITS   (LOG_SIZEOF_WQE_8x4_T + LOG_BITS_PER_BYTE)
struct len_encoding_8x4_t {
    len: 32;
};

#define WQE_16x2_T struct len_encoding_16x2_t
#define SIZEOF_WQE_16x2_T 2 //Bytes
#define SIZEOF_WQE_16x2_T_BITS (SIGEOF_WQE_16x2_T * BITS_PER_BYTE)
#define LOG_SIZEOF_WQE_16x2_T   1   // 2^1 = 2 Bytes
#define LOG_SIZEOF_WQE_16x2_T_BITS   (LOG_SIZEOF_WQE_16x2_T + LOG_BITS_PER_BYTE)
struct len_encoding_16x2_t {
    len: 16;
};

// wqe op types
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
#define OP_TYPE_INVALID             13

// recv cqe op types
#define OP_TYPE_CQE_RECV_SEND       0
#define OP_TYPE_CQE_RECV_SEND_INV   1
#define OP_TYPE_CQE_RECV_SEND_IMM   2
#define OP_TYPE_CQE_RECV_RDMA_IMM   3

//recv cqe flags
#define CQE_RECV_IS_VLAN            (1 << 0)
#define CQE_RECV_IS_IPv4            (1 << 1)

#define TXWQE_SGE_OFFSET  32 //
#define TXWQE_SGE_OFFSET_BITS   256 // 32 * 8
#define TXWQE_SGE_LEN_ENC_SIZE 32

#define FRPMR_DMA_ADDR_OFFSET ((sizeof(struct sqwqe_base_t) / 8) + FIELD_OFFSET(sqwqe_frpmr_t, dma_src_address))

union access_ctrl_flags_t {
    struct {
        rsvd_acc_flags: 1;
        on_demand     : 1;
        zbva          : 1;
        mw_bind       : 1;
        remote_atomic : 1;
        remote_rd     : 1;
        remote_wr     : 1;
        local_wr      : 1;
    };
    flags             : 8;
};

union mr_flags_t {
    struct {
        inv_en        : 1;
        is_mw         : 1;
        ukey_en       : 1;
        rsvd          : 5;
    };
    flags             : 8;
};


#define SQWQE_FORMAT_DEFAULT  0
#define SQWQE_FORMAT_8x4      1
#define SQWQE_FORMAT_16x2     2

#define SQWQE_SGE_TABLE_READ_SIZE 32
#define SQWQE_OPT_SGE_OFFSET_BITS 256 // 32 * 8
#define SQWQE_OPT_LAST_SGE_OFFSET_BITS 128

#define RRQWQE_SGE_TABLE_READ_SIZE 32
#define RRQWQE_OPT_SGE_OFFSET_BITS 256 // 32 * 8
#define RRQWQE_OPT_LAST_SGE_OFFSET_BITS 128

//16B
struct sqwqe_base_t {
    wrid               : 64;
    op_type_rsvd       : 4;
    op_type            : 4;
    union {
        num_sges       : 8;
        new_r_key_key  : 8;
        new_user_key   : 8;
    };

    //flags - u16
    wqe_format         : 4;
    rsvd_flags         : 7;
    color              : 1;
    signalled_compl    : 1;
    inline_data_vld    : 1;
    solicited_event    : 1;
    fence              : 1;

    union {
        imm_data       : 32; //for send/write with imm
        key            : 32; //for local-inv, frpmr
                             //for send-with-inv
                             //for bind-mw, rkey of the mw
    };
};

//16B
// send
struct sqwqe_send_t {
    ah_handle          : 32;
    rsvd               :  8;
    dst_qp             : 24;
    q_key              : 32;
    length             : 32;
};

//16B
// write/read
struct sqwqe_rdma_t { 
    va                 : 64;
    r_key              : 32;
    length             : 32;
};

//32B + 16B SGE
//Atomic
struct sqwqe_atomic_t {
    va                : 64;
    r_key             : 32;
    swap_or_add_data  : 64;
    cmp_data          : 64;
    pad               : 32;
    struct sge_t sge;
};

// 22B + 26B pad
// Bind Memory Window
struct sqwqe_bind_mw_t {
    va                 : 64;
    len                : 64;
    l_key              : 32;

    //total flags - 16 bits
    //other flags -  8 bits
    inv_en             : 1;
    rsvd_flags         : 7;
    //8 bit access_ctrl flags
    union access_ctrl_flags_t access_ctrl; //8 bits

    //26B
    pad                : 208;
};


// 48 Bytes
// Fast-Register Physical MR (FRPMR)
struct sqwqe_frpmr_t {
    base_va            : 64;
    len                : 64;
    offset             : 64;
    dma_src_address    : 64;
    num_pt_entries     : 32;

    //total flags - 16 bits
    //other flags -  8 bits
    rsvd_inv_en        : 1;
    rsvd_flags         : 7;
    //8 bit access_ctrl flags
    union access_ctrl_flags_t access_ctrl; //8 bits

    log_dir_size      : 8;
    log_page_size     : 8;

    //8B
    pad                : 64;
};



//16B
// FRMR
struct sqwqe_frmr_t {
    mr_id              : 24;
    type               :  3;
    pad                :  5;
    num_entries        : 32;
    dma_src_address    : 64;
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
                struct sqwqe_send_t ud_send;
                struct sqwqe_rdma_t write;
                struct sqwqe_rdma_t read;
                struct sqwqe_frmr_t frmr;
            };
            union {
                pad : 256;
                inline_data: 256;
                sge8x4 : 256;
                sge16x2 : 256;
                base_sges : 256;
            };
        };
    };
};

#define MAX_RD_ATOMIC 16
#define LOG_MAX_RD_ATOMIC 4
#define LOG_RRQ_WQE_SIZE 6
#define LOG_RRQ_QP_SIZE (LOG_MAX_RD_ATOMIC + LOG_RRQ_WQE_SIZE)
struct rrqwqe_read_t {
    len                : 32;
    wqe_sge_list_addr  : 64;
    pad                : 72;
    union {
        base_sges      : 256;
        sge8x4         : 256;
        sge16x2        : 256;
    };
};

struct rrqwqe_atomic_t {
    struct sge_t sge;
    op_type            : 8;
    pad                : 288;

};

#define RRQ_OP_TYPE_READ 0
#define RRQ_OP_TYPE_ATOMIC 1
#define RRQWQE_SGE_OFFSET  32
#define RRQWQE_SGE_OFFSET_BITS 256 // 32 * 8

struct rrqwqe_t {
    read_rsp_or_atomic : 1;
    wqe_format         : 4;
    rsvd               : 3;
    num_sges           : 8;
    psn                : 24;
    e_psn              : 24;
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
    e_psn              : 24;
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

#define RQWQE_OPT_SGE_OFFSET  64
#define LOG_RQWQE_OPT_SGE_OFFSET 6

#define RQWQE_OPT_SGE_OFFSET_BITS 256 // 32 * 8
#define RQWQE_OPT_LAST_SGE_OFFSET_BITS 128

#define RQWQE_FORMAT_DEFAULT  0
#define RQWQE_FORMAT_8x4      1
#define RQWQE_FORMAT_16x2     2

struct rqwqe_base_t {
    wrid: 64;
    rsvd0: 8;
    num_sges: 8;
    wqe_format: 4;
    rsvd: 172;
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

#define     AQ_CQ_STATUS_SUCCESS            0
#define     AQ_CQ_STATUS_BAD_CMD            1
#define     AQ_CQ_STATUS_BAD_INDEX          2
#define     AQ_CQ_STATUS_BAD_STATE          3
#define     AQ_CQ_STATUS_BAD_TYPE           4
#define     AQ_CQ_STATUS_BAD_ATTR           5

struct cqe_t {

    union {
        struct {
            wqe_id: 16;
            op_type: 8;
            rsvd: 136;
            old_sq_cindex: 16;
            old_rq_cq_cindex: 16;
        } admin;
        struct {
            wrid: 64;
            struct {
                ipv4: 1;
                vlan: 1;
                rsvd: 1;
            } flags;
            op_type: 5;
            src_qp: 24;
            smac: 48;
            vlan_tag: 16;
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
    qid:  24;
    type:  3;
    rsvd:  3;
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
#define EQE_CODE_QP_ERR_REQEST  9
#define EQE_CODE_QP_ERR_ACCESS 10


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

#define LOG_SIZEOF_DCQCN_CONFIG_T  6   // 2^6 = 64 bytes
#define DCQCN_CONFIG_SIZE_BYTES    64

#define ACC_CTRL_LOCAL_WRITE        0x1
#define ACC_CTRL_REMOTE_WRITE       0x2
#define ACC_CTRL_REMOTE_READ        0x4
#define ACC_CTRL_REMOTE_ATOMIC      0x8
#define ACC_CTRL_MW_BIND            0x10
#define ACC_CTRL_ZERO_BASED         0x20
#define ACC_CTRL_ON_DEMAND          0x40

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
#define MR_FLAG_INV_EN  0x80 // is memory invalidation enabled ?
#define MR_FLAG_IS_MW   0x40 // is memory window
#define MR_FLAG_UKEY_EN 0x20 //user-key is enabled

// Bit positions of MR Flags
#define LOG_MR_FLAG_INV_EN   7
#define LOG_MR_FLAG_IS_MW    6
#define LOG_MR_FLAG_UKEY_EN  5

#define KEY_ENTRY_T struct key_entry_t
#define LOG_SIZEOF_KEY_ENTRY_T  6   // 2^6 = 64 bytes
#define LOG_SIZEOF_KEY_ENTRY_T_BITS (LOG_SIZEOF_KEY_ENTRY_T + LOG_BITS_PER_BYTE)
#define KEY_ENTRY_SIZE_BYTES    64

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

#define DEFAULT_PKEY 0xffff

struct key_entry_t {
    user_key: 8;
    state: 4;
    type: 4;
    union access_ctrl_flags_t acc_ctrl; //8 bits
    log_page_size: 8;
    len: 64;
    base_va: 64;
    pt_base: 32;
    pd: 32;
    host_addr: 1;
    override_lif_vld: 1;
    override_lif: 12;
    is_phy_addr : 1;
    rsvd1: 17;
    union mr_flags_t mr_flags; //8 bits
    qp: 24; //qp which bound the MW ?
    mr_l_key: 32;
    mr_cookie: 32;
    num_pt_entries_rsvd: 32;
    phy_base_addr : 64;
    rsvd2: 64;
};

struct key_entry_aligned_t {
    user_key: 8;
    state: 4;
    type: 4;
    union access_ctrl_flags_t acc_ctrl; //8 bits
    log_page_size: 8;
    len: 64;
    base_va: 64;
    pt_base: 32;
    pd: 32;
    host_addr: 1;
    override_lif_vld: 1;
    override_lif: 12;
    is_phy_addr :1;
    rsvd1: 17;
    union mr_flags_t mr_flags; //8 bits
    qp: 24; //qp which bound the MW ?
    mr_l_key: 32;
    mr_cookie: 32;
    num_pt_entries_rsvd: 32;
    phy_base_addr : 64;
    // pad added for easy access of d[] in mpu program
    rsvd2: 64;
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


// if rnr syndrome value is represented as TTTTT, first 4bits
// is say "n" and last bit is "m" then  timeout value is
// (2^n + m*2^(n-1))/100 msec. As per the spec, 0 is 655.36 msec
// and so, if syndrome is 0, 0x20 is stored in sqcb2 by RXDMA
#define DECODE_RNR_SYNDROME_TIMEOUT(_timeout_r, _rnr, _tmp_r, _cf)      \
    sll            _timeout_r, 1, _rnr[5:1];                            \
    sne            _cf, _rnr[0], 0;                                     \
    srl._cf        _tmp_r, _timeout_r, 1;                               \
    add._cf        _timeout_r, _timeout_r, _tmp_r;

   
#define HDR_TEMPLATE_T struct header_template_t
#define HDR_TEMPLATE_T_SIZE_BYTES (sizeof(struct header_template_t)/8)
#define GRH_HDR_T_SIZE_BYTES (sizeof(struct ipv6hdr_t)/8)
#define PRIVILEGE_KEY_BASE 0x80000000


// Note: not using sizeof, because this needs to match exactly hal and nicmgr,
// and any discrepency will be easier to spot as plain numbers vs sizeof.
//
// We still must be sure that header_template_t, dcqcn_cb_t actually fit.
//
// Header template, bytes size of header_template_t, size aligned to 8 bytes
#define AH_ENTRY_T_SIZE_BYTES           72
// DCQCN CB, bytes size of dcqcn_cb_t, size aligned to 8 bytes
#define DCQCN_CB_T_SIZE_BYTES           64
// ROME CB, 64 bytes
#define ROME_CB_T_SIZE_BYTES    64
// AH Table Entry has header template, DCQCN CB and ROME CB
#if !(defined (HAPS) || defined (HW))
#define AT_ENTRY_SIZE_BYTES             (AH_ENTRY_T_SIZE_BYTES + DCQCN_CB_T_SIZE_BYTES + ROME_CB_T_SIZE_BYTES)
#else
#define AT_ENTRY_SIZE_BYTES             (AH_ENTRY_T_SIZE_BYTES + DCQCN_CB_T_SIZE_BYTES)
#endif

#define ACK_SYNDROME        0x00
#define RNR_SYNDROME        0x20
#define RESV_SYNDROME       0x40
#define NAK_SYNDROME        0x60
#define SYNDROME_MASK       0x60

#define SYNDROME_CREDITS_MASK 0x1F

#define DECODE_ACK_SYNDROME_CREDITS(_credits_r, _tmp_r, _syndrome, _cf) \
    sll            _credits_r, 1, _syndrome[4:1];                       \
    sne            _cf, _syndrome[0], 0;                                \
    srl._cf        _tmp_r, _credits_r, 1;                               \
    add._cf        _credits_r, _credits_r, _tmp_r;                      \
    seq            _cf, _syndrome[4:0], 0;                              \
    add._cf        _credits_r, 0, 0;

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
    rq_dma_addr  : 64;
    rq_id        : 24;
    rq_cmb       :  1;
    rq_spec      :  1;
    qp_privileged:  1;
    log_pmtu     :  5;
    access_flags :  3;
    rsvd         : 13;
};

struct rdma_aq_feedback_modify_qp_ext_t {
    rq_psn                           :   24;
    rq_psn_valid                     :    1;
    rq_id                            :   24;
    rsvd                             :   63;
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
        struct rdma_aq_feedback_modify_qp_ext_t modify_qp_ext;
    };
};

//Common DCQCN CB for configurable params.
struct dcqcn_config_cb_t {
            pad:64;
            // NP
            np_incp_802p_prio:8;
            np_cnp_dscp:8;
            rp_min_target_rate:32;
            // RP Alpha update
            rp_dce_tcp_g:16;
            rp_dce_tcp_rtt:32;
            // RP Rate Decrease
            rp_rate_reduce_monitor_period:32;
            rp_rate_to_set_on_first_cnp:32;
            rp_min_rate:32;
            rp_initial_alpha_value:16;
            rp_gd:8;
            rp_min_dec_fac:8;
            // RP Rate increase
            rp_clamp_flags:8;
            rp_rsvd3:3;        
            rp_threshold:5;
            rp_time_reset:16;
            rp_qp_rate:32;
            rp_byte_reset:32;
            rp_rsvd:14;
            rp_ai_rate:18;
            rp_rsvd1:14;
            rp_hai_rate:18;
            rp_token_bucket_size:64;
};

//Common DCQCN CB for both req and resp paths.
struct dcqcn_cb_t {
    // CNP generation params.
    last_cnp_timestamp: 48;

    /* DCQCN algorithm params*/

    // Configurable params.
    byte_counter_thr:       32; // byte-counter-threshold in Bytes. (Bc)
    rsvd1:                   8;
    sq_msg_psn:             24;
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
    resp_rl_failure:        1;  // Track rl_failures caused by dcqcn until reset by stage0
    rsvd0:                  1;

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

// ROME CB on sender side
struct rome_sender_cb_t {
    localClkResolution:      8; // power of 2 multiplier of local clk to define timestamp unit of measurement
    remoteClkRightShift:     8; // right shift after multiply to convert result into clk ticks at the remote clk rate
    clkScaleFactor:         31; // scaling factor to convert clk ticks from local clk rate into remote clk rate
    txMinRate:              17; // minimum pkt transmission rate in Mbps

    remoteClkStartEpoch:    32; // value of remote clk counter at the start of the current epoch
    localClkStartEpoch:     48; // value of local hardware clk free-running counter at the start of the current epoch

    totalBytesSent:         32; // total bytes transmitted on this flow modulo 2^32
    totalBytesAcked:        32; // total bytes acknowledged by receiver on this flow modulo 2^32
    window:                 32; // window size, max tx bytes allowed before ack
    currentRate:            27; // current pkt transmission rate in Kbps
    log_sq_size:             5;
    numCnpPktsRx:           32; // number of CNP pkts received on this flow

    last_sched_timestamp:   48;
    delta_ticks_last_sched: 16;
    cur_avail_tokens:       48;
    token_bucket_size:      48; // DCQCN enforced BC (committed-burst) in bits.
    sq_cindex:              16;
    cur_timestamp:          32; // For debugging on Model since model doesnt have timestamps
};

// ROME CB on receiver side.
struct rome_receiver_cb_t {
    state:                   3; // flowStateEnum, current state of flow state machine
    rsvd0:                   5;

    minTimestampDiff:       32; // minimum observed difference between local and remote integer timestamps

    linkDataRate:            2; // data transmission rate on wire in Kbps, set to the minimum of sender and receiver link rate, potentially only 2 bit to indicate 25G, 50G, 40G and 100G?
    recoverRate:            27; // RxMDA: target transmission rate in Kbps
    minRate:                27; //lowest transmission rate we can set on any flow in Kbps
    
    weight:                  4; // weight determines the Additive Increase amount: MinRate*weight
    rxDMA_tick:              3; // RxMDA: tick used to sync up RxDMA and TxDMA
    wait:                    1; // RxMDA: waiting for TxDMA to finish

    avgDelay:               20; // RxMDA: exponentially weighted moving average of one-way pkt delay
    preAvgDelay:            20; // TxMDA: value of avgDelay from the previous update period
    cycleMinDelay:          20; // RxMDA: min pkt latency observed during current cycle of 3 update periods
    cycleMaxDelay:          20; // RxMDA: max pkt latency observed during current cycle of 3 update periods

    totalBytesRx:           32; // RxMDA: total bytes received by the QP, used to return credit in credit loop
    rxBurstBytes:           16; // RxMDA: credit accumulator for credit loop in bytes
    byte_update:            16; // RxMDA: bytes received since last update
    th_byte:                32; // RxMDA: bytes accumulated since last throughput update

    cur_timestamp:          10; // For debugging on Model since model doesnt have timestamps
    thput:                  27; // RxMDA: the current measure of flow throughput
    MD_amount:              27; // RxMDA: accumulated MD rate deduction
    last_cycle:             32; // RxMDA: time of last min/max latency cycle reset in units of localClk
    last_thput:             32; // RxMDA: time of last throughput measurement
    last_epoch:             32; // RxMDA: time of last epoch of target delay reset
    last_update:            32; // RxMDA: time of last periodic update

    txDMA_tick:              3; // TxMDA: tick used to sync up RxDMA and TxDMA
    fspeed_cnt:             10; // TxMDA: number of update-periods after reaching full speed
    currentRate:            27; // TxMDA: current transmission rate in Kbps, changed in TxDMA
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
            err_qp_instantly: 1;
            lif_cqe_error_id_vld: 1;
            lif_error_id_vld: 1;
            lif_error_id: 4;
        }completion;
        /* Type: RDMA_CQ_ARM_FEEDBACK */
        struct {
            cindex: 16; /* arm cindex */
            color : 1;
            arm   : 1;
            sarm  : 1;
            pad   : 61;
        }arm;
    };
};

struct rdma_req_feedback_t {
    feedback_type:8;
    union {
        /* TYPE: RDMA_COMPLETION_FEEDBACK */
        struct {
            wrid: 64;
            status: 8;
            error: 1;
            err_qp_instantly: 1;
            lif_cqe_error_id_vld: 1;
            lif_error_id_vld: 1;
            lif_error_id: 4;
            rsvd: 8;
            ssn: 24;
            tx_psn: 24;
        }completion;
        /* TYPE: RDMA_TIMER_EXPIRY_FEEDBACK */
        struct {
            rexmit_psn: 24;
            ssn: 24;
            tx_psn: 24;
            pad: 64;
        }timer_expiry;
        /* TYPE: RDMA_SQ_DRAIN_FEEDBACK */
        struct {
            tx_psn: 24;
            ssn: 24;
            pad: 88;
        }sq_drain;
    };
};

struct rdma_aq_feedback_t {
    feedback_type: 8;
    struct {
        wqe_id:  16;
        status:  8;
        op:      8;
        error:   1;
        pad:     15;
    }aq_completion;
    union {
        struct { 
            rq_cq_id: 24;
            rq_depth_log2: 8;
            rq_stride_log2: 8;
            rq_page_size_log2: 8;
            rq_tbl_index:  32;
            rq_map_count:  32;
            pd:            32;
            rq_type_state: 8;
            pid:           16;
            rsvd:          8;
        } create_qp;
        struct {
            ah_len                   :  8;
            ah_addr                  : 32;
            av_valid                 :  1;            
            state                    :  3;
            state_valid              :  1;
            pmtu_log2                :  5;
            pmtu_valid               :  1;            
            rrq_base_addr            : 32;
            rrq_depth_log2           :  5;
            rrq_valid                :  1;
            err_retry_count          :  3;
            err_retry_count_valid    :  1;
            tx_psn_valid             :  1;
            tx_psn                   : 24;
            rnr_retry_count          :  3;
            rnr_retry_valid          :  1;
            q_key_rsq_base_addr      : 32;
            q_key_valid              :  1;
            rsq_depth_log2           :  5;
            rsq_valid                :  1;
            sqd_async_notify_en      :  1;
            access_flags_valid       :  1;
            access_flags             :  3;
            cur_state                :  3;
            cur_state_valid          :  1;
            congestion_mgmt_enable   :  1;
            dcqcn_cfg_id             :  4;
            rsvd                     :  1;
        } modify_qp;
        struct {
            rq_id                    : 24;
            pad                      : 16;
            dma_addr                 : 64;
            rsvd                     : 72;
        } query_destroy_qp;
        pad: 176;
    };
};

#define RDMA_FEEDBACK_SPLITTER_OFFSET  \
    ((sizeof(struct phv_intr_global_t) + sizeof(struct phv_intr_p4_t) + sizeof(struct phv_intr_rxdma_t) + sizeof(struct p4_to_p4plus_roce_header_t) + sizeof(struct rdma_feedback_t)) >> 3)

#define RDMA_REQ_FEEDBACK_SPLITTER_OFFSET  \
    ((sizeof(struct phv_intr_global_t) + sizeof(struct phv_intr_p4_t) + sizeof(struct phv_intr_rxdma_t) + sizeof(struct p4_to_p4plus_roce_header_t) + sizeof(struct rdma_req_feedback_t)) >> 3)

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

struct rq_prefetch_info_t {
    proxy_pindex: 16;
};

struct resp_rx_send_fml_t {
    spec_psn: 24;
    rsvd:      8;
};

#define IBV_QP_STATE_RESET      0
#define IBV_QP_STATE_INIT       1
#define IBV_QP_STATE_RTR        2
#define IBV_QP_STATE_RTS        3
#define IBV_QP_STATE_SQD        4
#define IBV_QP_STATE_SQ_ERR     5
#define IBV_QP_STATE_ERR        6
#define IBV_QP_STATE_SQD_ON_ERR 7

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
#define AQ_OP_TYPE_CREATE_AH    13
#define AQ_OP_TYPE_QUERY_AH     14
#define AQ_OP_TYPE_MODIFY_DCQCN 15
#define AQ_OP_TYPE_DESTROY_AH   16

//Define all stat types requested by the driver
#define AQ_STATS_DUMP_TYPE_QP   0
#define AQ_STATS_DUMP_TYPE_CQ   1
#define AQ_STATS_DUMP_TYPE_EQ   2
#define AQ_STATS_DUMP_TYPE_PT   3
#define AQ_STATS_DUMP_TYPE_KT   4
#define AQ_STATS_DUMP_TYPE_AQ   5
#define AQ_STATS_DUMP_TYPE_LIF  6
#define AQ_CAPTRACE_ENABLE      7
#define AQ_CAPTRACE_DISABLE     8
#define AQ_STATS_DUMP_TYPE_DCQCN_CONFIG 9
#define AQ_STATS_DUMP_TYPE_DCQCN_CB 10
#if !(defined (HAPS) || defined (HW))
#define AQ_STATS_DUMP_TYPE_ROME_RECEIVER 11
#endif

#define AQ_QPF_LOCAL_WRITE      0x00000001
#define AQ_QPF_REMOTE_WRITE     0x00000002
#define AQ_QPF_REMOTE_READ      0x00000004
#define AQ_QPF_REMOTE_ATOMIC    0x00000008
#define AQ_QPF_MW_BIND          0x00000010
#define AQ_QPF_ZERO_BASED       0x00000020
#define AQ_QPF_ON_DEMAND        0x00000040

#define AQ_QPF_SQ_SPEC          0x00000100
#define AQ_QPF_RQ_SPEC          0x00000200
#define AQ_QPF_SQD_NOTIFY       0x00001000
#define AQ_QPF_SQ_CMB           0x00002000
#define AQ_QPF_RQ_CMB           0x00004000
#define AQ_QPF_PRIVILEGED       0x00008000

struct aqwqe_t {
    op: 8;
    type_state: 8;
    union {
        dbid_flags: 16;
        struct {
            union access_ctrl_flags_t acc_ctrl; //8 bits
            union mr_flags_t mr_flags; //8 bits
        };
    };
    id_ver: 32;
    union {
        struct {
            dma_addr: 64;
            length: 32;
            rsvd: 352;
        } stats;
        struct {
            va: 64;
            length: 64;
            pd_id: 32;
            rsvd_flags: 16;            
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
            union {
                priv_flags: 32;
                struct {
                    pad1: 16;
                    privileged:1;
                    rq_cmb:1;
                    sq_cmb:1;
                    sqd_notify:1;
                    sq_draining:1;
                    remote_previleged:1;
                    rq_spec:1;
                    sq_spec:1;
                    pad3:5;
                    access_remote_atomic:1;
                    access_remote_read:1;
                    access_remote_write:1;
                };
            };
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
            attr_mask:32;
            dcqcn_profile:8;
            rsvd2:8;
            access_flags:16;
            rq_psn:32;
            sq_psn:32;
            qkey_dest_qpn:32;
            rate_limit_kbps:32;
            pmtu:8;
            retry:8;
            rnr_timer:8;
            retry_timeout:8;
            rsq_depth:8;
            rrq_depth:8;
            pkey_id:16;
            ah_id_len:32;
            en_pcp:8;
            ip_dscp:8;
            rsvd:16;
            rrq_index:32;
            rsq_index:32;
            dma_addr:64;
        } mod_qp;
        struct {
            dma_addr:64;
            length:32;
            pd_id:32;
            rsvd:320;
        } ah;
        struct {
            dma_addr: 64;
            rsvd: 384;
        } query_ah;
        struct {
            hdr_dma_addr: 64;
            ah_id: 32;
            rsvd: 224;
            sq_dma_addr: 64;
            rq_dma_addr: 64;
        } query;
        struct {
            np_incp_802p_prio:8;
            np_cnp_dscp:8;
            np_rsvd:32;
            rp_dce_tcp_g:16;
            rp_dce_tcp_rtt:32;
            rp_rate_reduce_monitor_period:32;
            rp_rate_to_set_on_first_cnp:32;
            rp_min_rate:32;
            rp_initial_alpha_value:16;
            rp_gd:8;
            rp_min_dec_fac:8;
            rp_clamp_flags:8;
            rp_threshold:8;
            rp_time_reset:16;
            rp_qp_rate:32;
            rp_byte_reset:32;
            rp_rsvd:14;
            rp_ai_rate:18;
            rp_rsvd2:14;
            rp_hai_rate:18;
            rp_token_bucket_size:64;
        } mod_dcqcn;
    };
};

struct aq_query_qp_sq_buf {
    rsvd:3;
    rnr_timer:5;
    retry_timeout:8;
    access_perms_flags:16;
    access_perms_rsvd:16;
    pkey_id:16;
    qkey_dest_qpn:32;
    rate_limit_kbps:32;
    rq_psn:32;
};

struct aq_query_qp_rq_buf {
    pad1:1;
    state:3;
    pmtu:4;
    pad2:1;
    retry_cnt:3;
    pad3:1;
    rnr_retry:3;
    rrq_depth:8;
    rsq_depth:8;
    pad4:8;
    sq_psn:24;
    ah_id_len:32;
};

struct aq_dscp_cos_map {
    dscp_pcp_tx_iq_map:512;
};

#define AQ_WQE_T_LOG_SIZE_BYTES 6

// 2-bit flag to indicate which opcode type to expect next
#define NEXT_OP_TYPE_ANY            0x0 // send/write/read/atomic
#define NEXT_OP_TYPE_SEND           0x1 // send
#define NEXT_OP_TYPE_WRITE          0x2 // write
#define NEXT_OP_TYPE_INVALID        0x3 // invalid

// 1-bit flag to indicate which packet type to expect next
#define NEXT_PKT_TYPE_FIRST_ONLY    0x0
#define NEXT_PKT_TYPE_MID_LAST      0x1

// 3-bit flag to indicate QP level access permissions
#define QP_ACCESS_REMOTE_WRITE      0x1
#define QP_ACCESS_REMOTE_READ       0x2
#define QP_ACCESS_REMOTE_ATOMIC     0x4

#endif //__TYPES_H
