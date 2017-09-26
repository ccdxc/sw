#ifndef __TYPES_H
#define __TYPES_H

#define HBM_NUM_KEY_ENTRIES_PER_CACHE_LINE 2

#define HBM_NUM_PT_ENTRIES_PER_CACHE_LINE 8
#define HBM_PAGE_SIZE_SHIFT 12 // HBM page size is assumed as 4K
#define HBM_CACHE_LINE_SIZE 64 // Bytes
#define HBM_CACHE_LINE_SIZE_BITS (HBM_CACHE_LINE_SIZE * BITS_PER_BYTE)
#define HBM_CACHE_LINE_SIZE_MASK (HBM_CACHE_LINE_SIZE - 1)
#define LOG_HBM_CACHE_LINE_SIZE 6 // 2^6 = 64 Bytes
#define LOG_HBM_NUM_PT_ENTRIES_PER_CACHELINE 3 // 2^3 = 8 entries

#define MAX_SGES_PER_PASS   2
#define HBM_NUM_SGES_PER_CACHELINE 4
#define SQCB_ADDR_SHIFT    9
#define RQCB_ADDR_SHIFT    9
#define RDMA_PAYLOAD_DMA_CMDS_START 8

#define LOG_SIZEOF_CQCB_T   5   // 2^5 = 32 Bytes
#define LOG_CB_UNIT_SIZE_BYTES 6
#define CB_UNIT_SIZE_BYTES  64

#define LOG_SIZEOF_EQCB_T   6   // 2^6 = 64 Bytes

#define SQCB0_ADDR_GET(_r)                         \
    sll     _r, k.global.cb_addr, SQCB_ADDR_SHIFT;

#define SQCB1_ADDR_GET(_r)                         \
     sll    _r, k.global.cb_addr, SQCB_ADDR_SHIFT; \
     add    _r, _r, CB_UNIT_SIZE_BYTES

#define RQCB1_ADDR_GET(_r) \
    sll     _r, k.global.cb_addr, RQCB_ADDR_SHIFT; \
    add     _r, _r, CB_UNIT_SIZE_BYTES
    
#define RQCB0_ADDR_GET(_r) \
    sll     _r, k.global.cb_addr, RQCB_ADDR_SHIFT;

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

#define AETH_CODE_ACK   0
#define AETH_CODE_RNR   1
#define AETH_CODE_RSVD  2
#define AETH_CODE_NAK   3

#define AETH_SYNDROME_CODE_SHIFT    5
#define AETH_SYNDROME_GET(_dst, _code, _val) \
    or  _dst, _val, _code, AETH_SYNDROME_CODE_SHIFT

#define AETH_NAK_SYNDROME_GET(_dst, _nak_code) \
    AETH_SYNDROME_GET(_dst, AETH_CODE_NAK, _nak_code)

#define AETH_ACK_SYNDROME_GET(_dst, _credits) \
    AETH_SYNDROME_GET(_dst, AETH_CODE_ACK, _credits)

#define AETH_RNR_SYNDROME_GET(_dst, _t) \
    AETH_SYNDROME_GET(_dst, AETH_CODE_NAK, _t)

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

    struct rdma_deth_t       deth;

    struct {
        struct rdma_deth_t   deth;
        struct rdma_immeth_t immeth;
    } deth_immeth;
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
    payload_len      : 16;
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
    payload_len      : 16;
    struct rdma_hdr0_t rdma_hdr;   // 112 bits
};

struct req_rx_flags_t {
    _error_disable_qp:1;
    _first:1;
    _middle:1;
    _last:1;
    _only:1;
    _read_resp:1;
    _ack:1;
    _atomic_ack:1;
    _completion:1;
    _aeth:1;
    _atomic_aeth:1;
};

#define REQ_RX_FLAG_ERR_DIS_QP         0x0001
#define REQ_RX_FLAG_FIRST              0x0002
#define REQ_RX_FLAG_MIDDLE             0x0004
#define REQ_RX_FLAG_LAST               0x0008
#define REQ_RX_FLAG_ONLY               0x0010
#define REQ_RX_FLAG_READ_RESP          0x0020
#define REQ_RX_FLAG_ACK                0x0040
#define REQ_RX_FLAG_ATOMIC_ACK         0x0080
#define REQ_RX_FLAG_COMPLETION         0x0100
#define REQ_RX_FLAG_AETH               0x0200
#define REQ_RX_FLAG_ATOMIC_AETH        0x0400

struct resp_rx_flags_t {
    _error_disable_qp:1;
    _first:1;
    _middle:1;
    _last:1;
    _only:1;
    _send:1;
    _read_req:1;
    _write:1;
    _atomic_fna:1;
    _atomic_cswap:1;
    _immdt:1;
    _inv_rkey:1;
    _completion:1;
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

struct req_tx_flags_t {
    error_disable_qp: 1;
    incr_lsn: 1;
    immeth_vld: 1;
};

#define REQ_TX_FLAG_ERR_DIS_QP          0x0001
#define REQ_TX_FLAG_INCR_LSN            0x0002

struct resp_tx_flags_t {
    error_disable_qp: 1;
};

#define RESP_TX_FLAG_ERR_DIS_QP         0x0001


#define ARE_ALL_FLAGS_SET(_c, _flags_r, _flags_test) \
    smeqh   _c, _flags_r, _flags_test, _flags_test
#define IS_ANY_FLAG_SET(_c, _flags_r, _flags_test) \
    smneh   _c, _flags_r, _flags_test, 0

#define ARE_ALL_FLAGS_SET_B(_c, _flags, _flags_test) \
    smeqb   _c, _flags, _flags_test, _flags_test
#define IS_ANY_FLAG_SET_B(_c, _flags, _flags_test) \
    smneb   _c, _flags, _flags_test, 0

#define RDMA_OPC_SERV_TYPE_SHIFT               5

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

union roce_opcode_flags_t {
    flags: 16;
    struct req_rx_flags_t req_rx;
    struct resp_rx_flags_t resp_rx;
    struct req_tx_flags_t req_tx;
    struct resp_tx_flags_t resp_tx;
}; 

struct ack_info_t {
    struct rdma_aeth_t  aeth;
    psn: 24;
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
#define OP_TYPE_FRPNR               8
#define OP_TYPE_LOCAL_INV           9
#define OP_TYPE_BIND_MW             10
#define OP_TYPE_SEND_INV_IMM        11 // vendor specific

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
    rsvd1              : 1;
    num_sges           : 8;
    rsvd2              : 16;
};

// send
struct sqwqe_send_t {
    imm_data           : 32;
    inv_key            : 32;
    length             : 32;
    rsvd               : 64;    // for now
};

// write
struct sqwqe_write_t {
    imm_data           : 32;
    va                 : 64;
    length             : 32;
    r_key              : 32;
};

// Read

struct sqwqe_read_t {
    rsvd               : 32;
    va                 : 64;
    length             : 32;
    r_key              : 32;
};

struct sqwqe_t {
    struct sqwqe_base_t base;
    union {
        struct sqwqe_send_t send;
        struct sqwqe_write_t write;
        struct sqwqe_read_t read;
    };
    pad : 256;
};

#define LOG_RRQ_WQE_SIZE 6
struct rrqwqe_read_t {
    len                : 32;
    wqe_sge_list_addr  : 64;
    pad                : 352; 
};

struct rrqwqe_atomic_t {
    struct sge_t sge;
    op_type            : 8;
    pad                : 312;

};

#define RRQ_OP_TYPE_READ 0
#define RRQ_OP_TYPE_ATOMIC 1

struct rrqwqe_t {
    read_rsp_or_atomic : 1;
    num_sges           : 7;
    psn                : 24;
    msn                : 24;
    rsvd               : 8;
    union {
        struct rrqwqe_read_t   read;
        struct rrqwqe_atomic_t atomic;
    };
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

struct cqwqe_t {
    union {
        wrid: 64;
        msn: 32;
    } id;
    op_type: 8;
    status: 8;
    rkey_inv_vld: 1;
    imm_data_vld: 1;
    color: 1;
    rsvd1: 5;
    qp: 24;
    imm_data: 32;
    r_key: 32;
    rsvd2: 80;
};

struct eqwqe_t {
    cq_id: 24;
    color: 1;
    rsvd: 7;
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

#define LOG_SIZEOF_RSQWQE_T 5 // 2^5 = 32 bytes

struct rsqwqe_t {
    read_or_atomic: 1;
    rsvd1: 7;
    psn: 24;
    rsvd2: 8;
    union {
        struct rsqwqe_read_t read;
        struct rsqwqe_atomic_t atomic;
    };
    
    // this pad added such that when it gets loaded in the d-vector, it is easy to parse the fields
    pad: 256; 
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

#define KEY_INDEX_MASK      0x00ffffff
#define KEY_INDEX_SHIFT     0
#define KEY_USER_KEY_MASK   0xff000000
#define KEY_USER_KEY_SHIFT  24

//MR_TYPE
#define MR_TYPE_MR_INVALID  0
#define MR_TYPE_MR          1
#define MR_TYPE_MW_TYPE_1   2
#define MR_TYPE_MW_TYPE_2A  3
#define MR_TYPE_MW_TYPE_2B  4

//MR_FLAG
#define MR_FLAG_MW_EN   0x1 // is memory window enabled ?
#define MR_FLAG_INV_EN  0x2 // is memory invalidation enabled ?
#define MR_FLAG_ZBVA    0x4 //is it a zbva ?
#define MR_FLAG_UKEY_EN 0x8 //user-key is enabled

#define KEY_ENTRY_T struct key_entry_t
#define LOG_SIZEOF_KEY_ENTRY_T  5   // 2^5 = 32 bytes
#define LOG_SIZEOF_KEY_ENTRY_T_BITS (LOG_SIZEOF_KEY_ENTRY_T + LOG_BITS_PER_BYTE)

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
    pt_size: 32; // looks like we are not using this field ?
    flags: 8;
    qp: 24; //qp which bound the MW ?
#if 0
    u32         peer_key: 24; //index of peer l_key or r_key
#endif
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

struct ethhdr_t {
    dmac       : 48;
    sma        : 48;
    ethertype  : 16;
};

struct vlanhdr_t {
    pri            : 3;
    cfi            : 1;
    vlan           : 12;
    ethertype      : 16;
};


struct header_template_t {
    struct ethhdr_t     eth; 
    struct vlanhdr_t    vlan;
    struct iphdr_t      ip;
    struct udphdr_t     udp;
};

#define HDR_TEMPLATE_T struct header_template_t
#define HDR_TEMPLATE_T_SIZE_BYTES (sizeof(struct header_template_t)/8)
 
#endif //__TYPES_H
