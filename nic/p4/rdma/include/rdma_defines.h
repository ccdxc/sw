#ifndef __RDMA_DEFINES_H__
#define __RDMA_DEFINES_H__

#define PACKED __attribute__((__packed__))

typedef struct rdma_bth_s {
    uint32_t opcode  : 8;
    uint32_t se      : 1;
    uint32_t m       : 1;
    uint32_t pad     : 2;
    uint32_t tver    : 4;
    uint32_t pkey    : 16;
    uint32_t f       : 1;
    uint32_t b       : 1;
    uint32_t rsvd1   : 6;
    uint32_t dst_qp  : 24;
    uint32_t a       : 1;
    uint32_t rsvd2   : 7;
    uint32_t psn     : 24;
} rdma_bth_t;

typedef struct rdma_rdeth_s {
    uint32_t rsvd    : 8;
    uint32_t ee_ctxt : 24;
} PACKED rdma_rdeth_t;

typedef struct rdma_deth_s {
    uint64_t q_key   : 32;
    uint64_t rsvd    : 8;
    uint64_t src_qp  : 24;
} PACKED rdma_deth_t;

typedef struct rdma_reth_s {
    uint64_t va      : 64;
    uint64_t r_key   : 32;
    uint64_t dma_len : 32;
} PACKED rdma_reth_t;


typedef struct rdma_ieth_s {
    uint32_t rkey    : 32;
} PACKED rdma_ieth_t;

typedef struct rdma_atomiceth_s {
    uint64_t va                 : 64;
    uint64_t r_key              : 32;
    uint64_t swap_or_add_data   : 64;
    uint64_t cmp_data           : 64;
} PACKED rdma_atomiceth_t;

typedef struct rdma_aeth_s {
    uint32_t syndrome    : 8;
    uint32_t msn         : 24;
} PACKED rdma_aeth_t;

typedef struct rdma_atomicaeth_s {
    uint64_t orig_data;
} PACKED rdma_atomicaeth_t;

typedef struct raeth_atom_aeth_s {
    rdma_aeth_t        aeth;
    rdma_atomicaeth_t  atom_aeth;
} PACKED raeth_atom_aeth_t;

typedef struct rdma_immeth_s {
    uint32_t data;
} PACKED rdma_immeth_t;

typedef struct rdma_xrceth_s {
    uint32_t rsvd    : 8;
    uint32_t xrcsrq  : 24;
} PACKED rdma_xrceth_t;


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
#define RESP_RX_FLAG_UD                 0x8000

#define ARE_ALL_FLAGS_SET(_c, _flags_r, _flags_test) \
    smeqh   _c, _flags_r, _flags_test, _flags_test
#define IS_ANY_FLAG_SET(_c, _flags_r, _flags_test) \
    smneh   _c, _flags_r, _flags, 0


#endif //__RDMA_DEFINES_H__

