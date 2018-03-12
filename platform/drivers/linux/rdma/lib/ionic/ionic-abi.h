#ifndef __IONIC_ABI_H__
#define __IONIC_ABI_H__

#include <infiniband/kern-abi.h>

#define IONIC_ABI_VERSION 1

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

enum ionic_wr_flags {
	IONIC_WR_FLAGS_INLINE		= 0x10,
	IONIC_WR_FLAGS_SE		= 0x08,
	IONIC_WR_FLAGS_UC_FENCE	= 0x04,
	IONIC_WR_FLAGS_RD_FENCE	= 0x02,
	IONIC_WR_FLAGS_SIGNALED	= 0x01
};

enum ionic_wc_type {
	IONIC_WC_TYPE_SEND		= 0x00,
	IONIC_WC_TYPE_RECV_RC		= 0x01,
	IONIC_WC_TYPE_RECV_UD		= 0x02,
	IONIC_WC_TYPE_RECV_RAW	= 0x03,
	IONIC_WC_TYPE_TERM		= 0x0E,
	IONIC_WC_TYPE_COFF		= 0x0F
};

enum ionic_req_wc_status {
	IONIC_REQ_ST_OK		= 0x00,
	IONIC_REQ_ST_BAD_RESP		= 0x01,
	IONIC_REQ_ST_LOC_LEN		= 0x02,
	IONIC_REQ_ST_LOC_QP_OP	= 0x03,
	IONIC_REQ_ST_PROT		= 0x04,
	IONIC_REQ_ST_MEM_OP		= 0x05,
	IONIC_REQ_ST_REM_INVAL	= 0x06,
	IONIC_REQ_ST_REM_ACC		= 0x07,
	IONIC_REQ_ST_REM_OP		= 0x08,
	IONIC_REQ_ST_RNR_NAK_XCED	= 0x09,
	IONIC_REQ_ST_TRNSP_XCED	= 0x0A,
	IONIC_REQ_ST_WR_FLUSH		= 0x0B
};

enum ionic_rsp_wc_status {
	IONIC_RSP_ST_OK		= 0x00,
	IONIC_RSP_ST_LOC_ACC		= 0x01,
	IONIC_RSP_ST_LOC_LEN		= 0x02,
	IONIC_RSP_ST_LOC_PROT		= 0x03,
	IONIC_RSP_ST_LOC_QP_OP	= 0x04,
	IONIC_RSP_ST_MEM_OP		= 0x05,
	IONIC_RSP_ST_REM_INVAL	= 0x06,
	IONIC_RSP_ST_WR_FLUSH		= 0x07,
	IONIC_RSP_ST_HW_FLUSH		= 0x08
};

enum ionic_db_que_type {
	IONIC_QUE_TYPE_SQ		= 0x00,
	IONIC_QUE_TYPE_RQ		= 0x01,
	IONIC_QUE_TYPE_SRQ		= 0x02,
	IONIC_QUE_TYPE_SRQ_ARM	= 0x03,
	IONIC_QUE_TYPE_CQ		= 0x04,
	IONIC_QUE_TYPE_CQ_ARMSE	= 0x05,
	IONIC_QUE_TYPE_CQ_ARMALL	= 0x06,
	IONIC_QUE_TYPE_CQ_ARMENA	= 0x07,
	IONIC_QUE_TYPE_SRQ_ARMENA	= 0x08,
	IONIC_QUE_TYPE_CQ_CUT_ACK	= 0x09,
	IONIC_QUE_TYPE_NULL		= 0x0F
};

enum ionic_db_mask {
	IONIC_DB_INDX_MASK		= 0xFFFFFUL,
	IONIC_DB_QID_MASK		= 0xFFFFFUL,
	IONIC_DB_TYP_MASK		= 0x0FUL,
	IONIC_DB_TYP_SHIFT		= 0x1C
};

enum ionic_bcqe_mask {
	IONIC_BCQE_PH_MASK		= 0x01,
	IONIC_BCQE_TYPE_MASK		= 0x0F,
	IONIC_BCQE_TYPE_SHIFT		= 0x01,
	IONIC_BCQE_STATUS_MASK	= 0xFF,
	IONIC_BCQE_STATUS_SHIFT	= 0x08,
	IONIC_BCQE_FLAGS_MASK		= 0xFFFFU,
	IONIC_BCQE_FLAGS_SHIFT	= 0x10,
	IONIC_BCQE_RWRID_MASK		= 0xFFFFFU,
	IONIC_BCQE_SRCQP_MASK		= 0xFF,
	IONIC_BCQE_SRCQP_SHIFT	= 0x18
};

enum ionic_rc_flags_mask {
	IONIC_RC_FLAGS_SRQ_RQ_MASK	= 0x01,
	IONIC_RC_FLAGS_IMM_MASK	= 0x02,
	IONIC_RC_FLAGS_IMM_SHIFT	= 0x01,
	IONIC_RC_FLAGS_INV_MASK	= 0x04,
	IONIC_RC_FLAGS_INV_SHIFT	= 0x02,
	IONIC_RC_FLAGS_RDMA_MASK	= 0x08,
	IONIC_RC_FLAGS_RDMA_SHIFT	= 0x03
};

enum ionic_ud_flags_mask {
	IONIC_UD_FLAGS_SRQ_RQ_MASK	= 0x01,
	IONIC_UD_FLAGS_IMM_MASK	= 0x02,
	IONIC_UD_FLAGS_HDR_TYP_MASK	= 0x0C,

	IONIC_UD_FLAGS_SRQ		= 0x01,
	IONIC_UD_FLAGS_RQ		= 0x00,
	IONIC_UD_FLAGS_ROCE		= 0x00,
	IONIC_UD_FLAGS_ROCE_IPV4	= 0x02,
	IONIC_UD_FLAGS_ROCE_IPV6	= 0x03
};

enum ionic_ud_cqe_mask {
	IONIC_UD_CQE_MAC_MASK		= 0xFFFFFFFFFFFFULL,
	IONIC_UD_CQE_SRCQPLO_MASK	= 0xFFFF,
	IONIC_UD_CQE_SRCQPLO_SHIFT	= 0x30
};

enum ionic_shpg_offt {
	IONIC_SHPG_BEG_RESV_OFFT	= 0x00,
	IONIC_SHPG_AVID_OFFT		= 0x10,
	IONIC_SHPG_AVID_SIZE		= 0x04,
	IONIC_SHPG_END_RESV_OFFT	= 0xFF0
};

#define IONIC_MAX_INLINE_SIZE		0x100

struct ionic_cntx_resp {
	struct ibv_get_context_resp resp;
	__u32 dev_id;
	__u32 max_qp; /* To allocate qp-table */
	__u32 pg_size;
	__u32 cqe_size;
	__u32 max_cqd;
	__u32 rsvd;
};

struct ionic_pd_resp {
	struct ibv_alloc_pd_resp resp;
	__u32 pdid;
};

struct ionic_mr_resp {
	struct ibv_reg_mr_resp resp;
};

struct ionic_cq_req {
	struct ibv_create_cq cmd;
	__u64 cq_va;
	__u64 cq_bytes;
};

struct ionic_cq_resp {
	struct ibv_create_cq_resp resp;
	__u32 cqid;
	__u8  qtype;
};

#define COLOR_SHIFT 5

#define IMM_DATA_VLD_MASK 0x40

/*
 * CQE structure definition
 * TODO: Need to make sure of endian differences between what capri writes and
 * what driver sees.
 */
struct cqwqe_be_t {
    union {
        __u64 wrid;
        __u32 msn;
    } id;
    __u8  op_type;
    __u8  status;
    __u8  rsvd2;
    __u8  qp_hi;
    __u16 qp_lo;
    __u8  src_qp_hi;
    __u16 src_qp_lo;
    __u16 smac[3];
    __u8  color_flags;
    __u32 imm_data;
    __u32 r_key;
}__attribute__ ((__packed__));

struct cqwqe_t {
    __u32 r_key;
    __u32 imm_data;
    __u8 rsvd1:4;
    __u8 ipv4:1;
    __u8 color:1;
    __u8 imm_data_vld:1;
    __u8 rkey_inv_vld:1;
    __u16 smac[3];
    __u32 src_qp: 24;
    __u32 qp: 24;
    __u8 rsvd2;
    __u8 status;
    __u8 op_type;
    union {
        __u64 wrid;
        __u32 msn;
    } id;

}__attribute__ ((__packed__));

struct ionic_qp_req {
	struct ibv_create_qp cmd;
	__u64 qpsva;
	__u64 qprva;
    __u32 sq_bytes;
    __u32 rq_bytes;
    __u32 sq_wqe_size;
    __u32 rq_wqe_size;
};

struct ionic_qp_resp {
	struct ibv_create_qp_resp resp;
	__u32 qpid;
    __u8  sq_qtype;
    __u8  rq_qtype;
	__u16 rsvd;
};

/*
 * Send WR entry
 */
#define SQWQE_FLAGS_SHIFT   4

enum SQWQE_FLAGS {
    SQWQE_FLAGS_COMPLETE_NOTIFY = 0x1,
    SQWQE_FLAGS_FENCE           = 0x2,
    SQWQE_FLAGS_SE              = 0x4, // solicited event
    SQWQE_FLAGS_INLINE_DATA     = 0x8,
};

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
struct sqwqe_write_t {
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

// Read
struct sqwqe_read_t {
    __u32 rsvd;
    __u64 va;
    __u32 length;
    __u32 r_key;
}__attribute__ ((__packed__));

struct sqwqe_non_atomic_t {
    union {
        struct sqwqe_rc_send_t send;
        struct sqwqe_ud_send_t ud_send;
        struct sqwqe_write_t write;
        struct sqwqe_read_t read;
    }wqe;
    __u64 inline_data[4]; // It can accomodate 2 SGEs
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
