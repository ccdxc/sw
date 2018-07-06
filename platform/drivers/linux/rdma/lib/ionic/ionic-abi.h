#ifndef __IONIC_ABI_H__
#define __IONIC_ABI_H__

#include <infiniband/kern-abi.h>
#include <infiniband/verbs.h>
#include <rdma/ionic-abi.h>
#include <kernel-abi/ionic-abi.h>
#include "ionic_fw.h"

DECLARE_DRV_CMD(uionic_ctx, IB_USER_VERBS_CMD_GET_CONTEXT,
		ionic_ctx_req, ionic_ctx_resp);
DECLARE_DRV_CMD(uionic_ah, IB_USER_VERBS_CMD_CREATE_AH,
		empty, ionic_ah_resp);
DECLARE_DRV_CMD(uionic_cq, IB_USER_VERBS_CMD_CREATE_CQ,
		ionic_cq_req, ionic_cq_resp);
DECLARE_DRV_CMD(uionic_qp, IB_USER_VERBS_EX_CMD_CREATE_QP,
		ionic_qp_req, ionic_qp_resp);

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
