#ifndef __IONIC_ABI_H__
#define __IONIC_ABI_H__

#include <infiniband/kern-abi.h>
#include "ionic_fw.h"

#define IONIC_ABI_VERSION	3

struct uionic_ctx {
	struct ibv_get_context ibv_cmd;
	__u32 fallback;
	__u32 rsvd;
};

struct uionic_ctx_resp {
	struct ibv_get_context_resp ibv_resp;
	__u32 fallback;
	__u32 page_shift;

	__u64 dbell_offset;

	__u16 version;
	__u8 qp_opcodes[7];
	__u8 admin_opcodes[7];

	__u8 sq_qtype;
	__u8 rq_qtype;
	__u8 cq_qtype;
	__u8 admin_qtype;
};

struct ionic_qdesc {
	__u64 addr;
	__u32 size;
	__u16 mask;
	__u8 depth_log2;
	__u8 stride_log2;
};

struct uionic_ah_resp {
	struct ibv_create_ah_resp ibv_resp;
	__u32 ahid;
	__u32 len;
};

struct uionic_cq {
	struct ibv_create_cq ibv_cmd;
	struct ionic_qdesc cq;
};

struct uionic_cq_resp {
	struct ibv_create_cq_resp ibv_resp;
	__u32 cqid;
};

struct uionic_qp {
	struct ibv_create_qp_ex ibv_cmd;
	struct ionic_qdesc sq;
	struct ionic_qdesc rq;
};

struct uionic_qp_resp {
	struct ibv_create_qp_resp_ex ibv_resp;
	__u32 qpid;
	__u32 rsvd;
	__u64 sq_hbm_offset;
};


/* XXX cleanup: makeshift interface */

#define IONIC_FULL_FLAG_DELTA        0x80

#define IONIC_WR_OPCODE_MASK        0xF

enum ionic_wr_opcode {
	IONIC_WR_OPCD_SEND		    = 0x00,
	IONIC_WR_OPCD_SEND_INVAL	    = 0x01,
	IONIC_WR_OPCD_SEND_IMM	    = 0x02,
	IONIC_WR_OPCD_RDMA_READ	    = 0x03,
	IONIC_WR_OPCD_RDMA_WRITE	    = 0x04,
	IONIC_WR_OPCD_RDMA_WRITE_IMM	= 0x05,
	IONIC_WR_OPCD_ATOMIC_CS	    = 0x06,
	IONIC_WR_OPCD_ATOMIC_FA	    = 0x07,
	IONIC_WR_OPCD_FRPNR    	    = 0x08,    
	IONIC_WR_OPCD_LOC_INVAL	    = 0x09,
	IONIC_WR_OPCD_BIND		    = 0x0a,
	IONIC_WR_OPCD_SEND_INV_IMM    = 0x0b,
    
	IONIC_WR_OPCD_INVAL		    = 0x0F
};

#define IONIC_MAX_INLINE_SIZE		0x100

/*
 * Send WR entry
 */
struct sge_t {
    __u64 va;
    __u32 len;
    __u32 lkey;
};

struct sqwqe_base_t {
    __u64 wrid;
    __u8  inline_data_vld    : 1;
    __u8  solicited_event    : 1;
    __u8  fence              : 1;
    __u8  complete_notify    : 1;
    __u8  op_type:4;
    __u8  num_sges;
    __u16 rsvd2;
}__attribute__ ((__packed__));

// RC send
struct sqwqe_rc_send_t {
    __u32 imm_data;
    __u32 inv_key;
    __u32 rsvd1;
    __u32 length;
    __u32 rsvd2;
}__attribute__ ((__packed__));

// UD send
struct sqwqe_ud_send_t {
    __u32 imm_data;
    __u32 q_key;
    __u32 length;
    __u32 ah_size:8;
    __u32 dst_qp:24;
    __u32 ah_handle;
}__attribute__ ((__packed__));

// write
struct sqwqe_rdma_t {
    __u32 imm_data;
    __u64 va;
    __u32 length;
    __u32 r_key;
}__attribute__ ((__packed__));

//Atomic
struct sqwqe_atomic_t {
    __u32 r_key;
    __u64 va;
    __u64 swap_or_add_data;
    __u64 cmp_data;
    __u64 pad;
    struct sge_t sge;
}__attribute__ ((__packed__));

struct sqwqe_non_atomic_t {
    union {
        struct sqwqe_rc_send_t send;
        struct sqwqe_ud_send_t ud_send;
        struct sqwqe_rdma_t rdma;
    }wqe;
    struct sge_t sg_arr[2];
}__attribute__ ((__packed__));

/*
 * TODO: how do we make sure op specific parameters start on boundary of 32
 * instead of 64? Do we need to pack it?
 */
struct sqwqe_t {
    struct sqwqe_base_t base;
    union {
        struct sqwqe_atomic_t atomic;
        struct sqwqe_non_atomic_t non_atomic;
    }u;
}__attribute__ ((__packed__));

/********** End of send WQE definition  ***********/

/*
 * Definition of RQ WQE
 */


/********** End of Recv WQE definition  ***********/

struct rqwqe_t {
    __u64        wrid;
    __u8         num_sges;
    __u8         rsd[23];
    struct sge_t sge_arr[2];
}__attribute__((__packed__));

#endif
