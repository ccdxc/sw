#ifndef IONIC_FW_H
#define IONIC_FW_H

#include <linux/kernel.h>

#define IONIC_CQE_SIZE		32
#define IONIC_SQ_WQE_SIZE	32
#define IONIC_SQ_WQE_MINSIZE	64
#define IONIC_RQ_WQE_SIZE	32
#define IONIC_RQ_WQE_MINSIZE	64
#define IONIC_SGE_SIZE		16

#define IONIC_NUM_RSQ_WQE         4
#define IONIC_NUM_RRQ_WQE         4

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

#define OP_TYPE_SEND			0
#define OP_TYPE_SEND_INV		1
#define OP_TYPE_SEND_IMM		2
#define OP_TYPE_READ			3
#define OP_TYPE_WRITE			4
#define OP_TYPE_WRITE_IMM		5
#define OP_TYPE_CMP_N_SWAP		6
#define OP_TYPE_FETCH_N_ADD		7
#define OP_TYPE_FRPNR			8
#define OP_TYPE_LOCAL_INV		9
#define OP_TYPE_BIND_MW			10
#define OP_TYPE_SEND_INV_IMM		11 // vendor specific
#define OP_TYPE_RDMA_OPER_WITH_IMM	16
#define OP_TYPE_SEND_RCVD		17
#define OP_TYPE_INVALID			18
#define OP_TYPE_REG_MR			20

#define CQ_STATUS_SUCCESS		0
#define CQ_STATUS_LOCAL_LEN_ERR		1
#define CQ_STATUS_LOCAL_QP_OPER_ERR	2
#define CQ_STATUS_LOCAL_PROT_ERR	3
#define CQ_STATUS_WQE_FLUSHED_ERR	4
#define CQ_STATUS_MEM_MGMT_OPER_ERR	5
#define CQ_STATUS_BAD_RESP_ERR		6
#define CQ_STATUS_LOCAL_ACC_ERR		7
#define CQ_STATUS_REMOTE_INV_REQ_ERR	8
#define CQ_STATUS_REMOTE_ACC_ERR	9
#define CQ_STATUS_REMOTE_OPER_ERR	10
#define CQ_STATUS_RETRY_EXCEEDED	11
#define CQ_STATUS_RNR_RETRY_EXCEEDED	12
#define CQ_STATUS_XRC_VIO_ERR		13

#define COLOR_SHIFT		5
#define IMM_DATA_VLD_MASK 0x40

enum ionic_qp_type {
	IONIC_QP_TYPE_RC = 0,
	IONIC_QP_TYPE_UC,
	IONIC_QP_TYPE_RD,
	IONIC_QP_TYPE_UD,
	IONIC_QP_TYPE_MAX = 255,
};

static inline enum ionic_qp_type ib_qp_type_to_ionic(enum ib_qp_type ibtype)
{
	switch (ibtype) {
	case IB_QPT_RC:
		return IONIC_QP_TYPE_RC;
	case IB_QPT_UC:
		return IONIC_QP_TYPE_UC;
	case IB_QPT_UD:
		return IONIC_QP_TYPE_UD;
	default:
		return IONIC_QP_TYPE_MAX;
	}
}

struct sge_t {
	__u64 va;
	__u32 len;
	__u32 lkey;
};

struct cqwqe_be_t {
	union {
		__u64 wrid;
		struct {
			__u32 rsvd;
			__u32 msn;
		};
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

struct sqwqe_rc_send_t {
	__u32 imm_data;
	__u32 inv_key;
	__u32 rsvd1;
	__u32 length;
	__u32 rsvd2;
}__attribute__ ((__packed__));

struct sqwqe_ud_send_t {
	__u32 imm_data;
	__u32 q_key;
	__u32 length;
	__u32 ah_size:8;
	__u32 dst_qp:24;
	__u32 ah_handle;
}__attribute__ ((__packed__));

struct sqwqe_rdma_t {
	__u32 imm_data;
	__u64 va;
	__u32 length;
	__u32 r_key;
}__attribute__ ((__packed__));

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

struct sqwqe_t {
	struct sqwqe_base_t base;
	union {
		struct sqwqe_atomic_t atomic;
		struct sqwqe_non_atomic_t non_atomic;
	}u;
}__attribute__ ((__packed__));

struct rqwqe_t {
	__u64        wrid;
	__u8         num_sges;
	__u8         rsd[23];
	struct sge_t sge_arr[2];
}__attribute__((__packed__));

static inline u16 ionic_cqe_size(void)
{
	BUILD_BUG_ON(sizeof(struct cqwqe_be_t) != IONIC_CQE_SIZE);

	return IONIC_CQE_SIZE;
}

static inline u16 ionic_sq_wqe_size(u16 max_sge, u16 max_inline)
{
	u16 sgl_size = max_t(u16, max_sge * IONIC_SGE_SIZE, max_inline);

	BUILD_BUG_ON(IONIC_SQ_WQE_MINSIZE != sizeof(struct sqwqe_t));
	BUILD_BUG_ON(IONIC_SQ_WQE_SIZE != offsetof(struct sqwqe_t,
						   u.non_atomic.sg_arr));

	return max_t(u16, IONIC_SQ_WQE_MINSIZE, IONIC_SQ_WQE_SIZE + sgl_size);
}

static inline int ionic_sq_wqe_max_sge(u16 wqe_size)
{
	return (wqe_size - IONIC_SQ_WQE_SIZE) / IONIC_SGE_SIZE;
}

static inline int ionic_sq_wqe_max_inline(u16 wqe_size)
{
	return wqe_size - IONIC_SQ_WQE_SIZE;
}

static inline u16 ionic_rq_wqe_size(u16 max_sge)
{
	u16 sgl_size = max_sge * IONIC_SGE_SIZE;

	BUILD_BUG_ON(IONIC_RQ_WQE_MINSIZE != sizeof(struct rqwqe_t));
	BUILD_BUG_ON(IONIC_RQ_WQE_SIZE != offsetof(struct rqwqe_t, sge_arr));

	return max_t(u16, IONIC_RQ_WQE_MINSIZE, IONIC_RQ_WQE_SIZE + sgl_size);
}

static inline int ionic_rq_wqe_max_sge(u16 wqe_size)
{
	return (wqe_size - IONIC_RQ_WQE_SIZE) / IONIC_SGE_SIZE;
}

static inline bool ionic_cqe_color(struct cqwqe_be_t *cqe)
{
	return (cqe->color_flags >> COLOR_SHIFT) != 0;
}

static inline u32 ionic_cqe_qpn(struct cqwqe_be_t *cqe)
{
	return ((u32)cqe->qp_hi << 16) | be16_to_cpu(cqe->qp_lo);
}

static inline u32 ionic_cqe_src_qpn(struct cqwqe_be_t *cqe)
{
	return ((u32)cqe->src_qp_hi << 16) | be16_to_cpu(cqe->src_qp_lo);
}

static inline bool ionic_op_is_local(uint8_t opcd)
{
	return opcd == OP_TYPE_LOCAL_INV || opcd == OP_TYPE_REG_MR;
}

static inline enum ib_wc_opcode ionic_to_ib_wc_opcd(uint8_t ionic_opcd)
{
	enum ib_wc_opcode ib_opcd;

	switch (ionic_opcd) {
	case OP_TYPE_SEND:
	case OP_TYPE_SEND_INV:
	case OP_TYPE_SEND_IMM:
		ib_opcd = IB_WC_SEND;
		break;
	case OP_TYPE_READ:
		ib_opcd = IB_WC_RDMA_READ;
		break;
	case OP_TYPE_WRITE:
	case OP_TYPE_WRITE_IMM:
		ib_opcd = IB_WC_RDMA_WRITE;
		break;
	case OP_TYPE_CMP_N_SWAP:
		ib_opcd = IB_WC_COMP_SWAP;
		break;
	case OP_TYPE_FETCH_N_ADD:
		ib_opcd = IB_WC_FETCH_ADD;
		break;
	case OP_TYPE_LOCAL_INV:
		ib_opcd = IB_WC_LOCAL_INV;
		break;
	case OP_TYPE_REG_MR:
		ib_opcd = IB_WC_REG_MR;
		break;
	default:
		ib_opcd = 0;
	}

	return ib_opcd;
}

static inline uint8_t ionic_to_ib_wc_status(uint8_t wcst)
{
	uint8_t ib_wcst;

	/* XXX should this use ionic_{req,rsp}_wc_status instead?
	 * also, do we really need two different enums for wc status? */
	switch (wcst) {
	case 0:
		ib_wcst = IB_WC_SUCCESS;
		break;
	case CQ_STATUS_LOCAL_LEN_ERR:
		ib_wcst = IB_WC_LOC_LEN_ERR;
		break;
	case CQ_STATUS_LOCAL_QP_OPER_ERR:
		ib_wcst = IB_WC_LOC_QP_OP_ERR;
		break;
	case CQ_STATUS_LOCAL_PROT_ERR:
		ib_wcst = IB_WC_LOC_PROT_ERR;
		break;
	case CQ_STATUS_WQE_FLUSHED_ERR:
		ib_wcst = IB_WC_WR_FLUSH_ERR;
		break;
	case CQ_STATUS_LOCAL_ACC_ERR:
		ib_wcst = IB_WC_LOC_ACCESS_ERR;
		break;
	case CQ_STATUS_REMOTE_INV_REQ_ERR:
		ib_wcst = IB_WC_REM_INV_REQ_ERR;
		break;
	case CQ_STATUS_REMOTE_ACC_ERR:
		ib_wcst = IB_WC_REM_ACCESS_ERR;
		break;
	case CQ_STATUS_REMOTE_OPER_ERR:
		ib_wcst = IB_WC_REM_OP_ERR;
		break;
	case CQ_STATUS_RNR_RETRY_EXCEEDED:
		ib_wcst = IB_WC_RNR_RETRY_EXC_ERR;
		break;
	case CQ_STATUS_RETRY_EXCEEDED:
		ib_wcst = IB_WC_RETRY_EXC_ERR;
		break;
	default:
		ib_wcst = IB_WC_GENERAL_ERR;
		break;
	}

	return ib_wcst;
}

#endif
