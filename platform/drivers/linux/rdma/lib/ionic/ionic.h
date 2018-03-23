#ifndef IONIC_H
#define IONIC_H

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <endian.h>
#include <pthread.h>

#include <infiniband/driver.h>
#include <util/udma_barrier.h>

#include "ionic-abi.h"

#include "ionic_dbg.h"
#include "ionic_memory.h"
#include "ionic_queue.h"
#include "table.h"

#define DEV			"ionic : "

struct ionic_ctx {
	struct verbs_context	vctx;

	uint32_t		version;
	bool			fallback;

	uint8_t			sq_qtype;
	uint8_t			rq_qtype;
	uint8_t			cq_qtype;

	uint64_t		*dbpage;

	pthread_mutex_t		mut;
	struct tbl_root		qp_tbl;
};

struct ionic_cq {
	struct ibv_cq		ibcq;

	uint32_t		cqid;

	pthread_spinlock_t	lock;
	struct ionic_queue	q;
};

struct ionic_wrid {
	struct ionic_psns	*psns;
	uint64_t		wrid;
	uint32_t		bytes;
	uint8_t			sig;
};

struct ionic_qpcap {
	uint32_t		max_swr;
	uint32_t		max_rwr;
	uint32_t		max_ssge;
	uint32_t		max_rsge;
	uint32_t		max_inline;
	uint8_t			sqsig;
};

struct ionic_sq_meta {
	uint64_t		wrid;
	uint32_t		len;
	uint8_t			op;
};

struct ionic_rq_meta {
	uint64_t		wrid;
	uint32_t		len; /* XXX byte_len must come from cqe */
};

struct ionic_qp {
	union {
		struct verbs_qp		vqp;
		struct verbs_srq	vsrq;
	};

	bool			is_srq;

	uint32_t		qpid;

	pthread_spinlock_t	sq_lock;
	struct ionic_queue	sq;
	struct ionic_sq_meta	*sq_meta;

	void			*sq_hbm_ptr;
	uint16_t		sq_hbm_prod;

	/* next sequence number to complete, 24bit, initialized to 1 */
	uint32_t		sq_local;
	uint32_t		sq_msn;

	pthread_spinlock_t	rq_lock;
	struct ionic_queue	rq;
	struct ionic_rq_meta	*rq_meta;
};

struct ionic_ah {
	struct ibv_ah		ibah;
	uint32_t		avid;
};

struct ionic_dev {
	struct verbs_device	vdev;
	uint8_t			abi_version;
	size_t			pg_size;

	uint32_t		cqe_size;
	uint32_t		max_cq_depth;
};

static inline struct ionic_dev *to_ionic_dev(struct ibv_device *ibdev)
{
	return container_of(ibdev, struct ionic_dev, vdev.device);
}

static inline struct ionic_ctx *to_ionic_ctx(struct ibv_context *ibctx)
{
	return container_of(ibctx, struct ionic_ctx, vctx.context);
}

static inline struct ionic_cq *to_ionic_cq(struct ibv_cq *ibcq)
{
	return container_of(ibcq, struct ionic_cq, ibcq);
}

static inline struct ionic_qp *to_ionic_qp(struct ibv_qp *ibqp)
{
	return container_of(ibqp, struct ionic_qp, vqp.qp);
}

static inline struct ionic_qp *to_ionic_srq(struct ibv_srq *ibsrq)
{
	return container_of(ibsrq, struct ionic_qp, vsrq.srq);
}

static inline struct ionic_ah *to_ionic_ah(struct ibv_ah *ibah)
{
        return container_of(ibah, struct ionic_ah, ibah);
}

static inline uint16_t ionic_get_sqe_size(uint16_t max_sge,
					  uint16_t max_inline)
{
	if (max_sge < 2)
		max_sge = 2;

	max_sge *= 16;

	if (max_sge < max_inline)
		max_sge = max_inline;

	return 32 + max_sge;
}

static inline uint32_t ionic_get_rqe_size(uint16_t max_sge)
{
	if (max_sge < 2)
		max_sge = 2;

	max_sge *= 16;

	return 32 + max_sge;
}

static inline bool ionic_qp_has_rq(struct ionic_qp *qp)
{
	if (qp->is_srq)
		return true;

	if (qp->vqp.qp.qp_type == IBV_QPT_RC ||
	    qp->vqp.qp.qp_type == IBV_QPT_UC ||
	    qp->vqp.qp.qp_type == IBV_QPT_UD)
		return !qp->vqp.qp.srq;

	return false;
}

static inline bool ionic_qp_has_sq(struct ionic_qp *qp)
{
	if (qp->is_srq)
		return false;

	return qp->vqp.qp.qp_type == IBV_QPT_RC ||
		qp->vqp.qp.qp_type == IBV_QPT_UC ||
		qp->vqp.qp.qp_type == IBV_QPT_UD ||
		qp->vqp.qp.qp_type == IBV_QPT_XRC_SEND;
}

static inline uint8_t ibv_to_ionic_wr_opcd(uint8_t ibv_opcd)
{
	uint8_t bnxt_opcd;

	switch (ibv_opcd) {
	case IBV_WR_SEND:
		bnxt_opcd = IONIC_WR_OPCD_SEND;
		break;
	case IBV_WR_SEND_WITH_IMM:
		bnxt_opcd = IONIC_WR_OPCD_SEND_IMM;
		break;
	case IBV_WR_RDMA_WRITE:
		bnxt_opcd = IONIC_WR_OPCD_RDMA_WRITE;
		break;
	case IBV_WR_RDMA_WRITE_WITH_IMM:
		bnxt_opcd = IONIC_WR_OPCD_RDMA_WRITE_IMM;
		break;
	case IBV_WR_RDMA_READ:
		bnxt_opcd = IONIC_WR_OPCD_RDMA_READ;
		break;
	case IBV_WR_ATOMIC_CMP_AND_SWP:
		bnxt_opcd = IONIC_WR_OPCD_ATOMIC_CS;
		break;
	case IBV_WR_ATOMIC_FETCH_AND_ADD:
		bnxt_opcd = IONIC_WR_OPCD_ATOMIC_FA;
		break;
		/* TODO: Add other opcodes */
	default:
		bnxt_opcd = IONIC_WR_OPCD_INVAL;
		break;
	};

	return bnxt_opcd;
}

static inline void ionic_set_ibv_send_flags(int flags, struct sqwqe_t *wqe)
{
	if (flags & IBV_SEND_FENCE) {
		wqe->base.fence = 1;
	}
	if (flags & IBV_SEND_SOLICITED) {
		wqe->base.solicited_event = 1;
	}
	if (flags & IBV_SEND_INLINE) {
		wqe->base.inline_data_vld = 1;
	}
	if (flags & IBV_SEND_SIGNALED) {
		wqe->base.complete_notify = 1;
	}
}

static inline uint8_t ionic_ibv_wr_to_wc_opcd(uint8_t wr_opcd)
{
	uint8_t wc_opcd;

	switch (wr_opcd) {
	case IBV_WR_SEND_WITH_IMM:
	case IBV_WR_SEND:
		wc_opcd = IBV_WC_SEND;
		break;
	case IBV_WR_RDMA_WRITE_WITH_IMM:
	case IBV_WR_RDMA_WRITE:
		wc_opcd = IBV_WC_RDMA_WRITE;
		break;
	case IBV_WR_RDMA_READ:
		wc_opcd = IBV_WC_RDMA_READ;
		break;
	case IBV_WR_ATOMIC_CMP_AND_SWP:
		wc_opcd = IBV_WC_COMP_SWAP;
		break;
	case IBV_WR_ATOMIC_FETCH_AND_ADD:
		wc_opcd = IBV_WC_FETCH_ADD;
		break;
	default:
		wc_opcd = 0xFF;
		break;
	}

	return wc_opcd;
}

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

static inline enum ibv_wc_opcode ionic_to_ibv_wc_opcd(uint8_t ionic_opcd)
{
	enum ibv_wc_opcode ibv_opcd;

	/* XXX should this use ionic_wc_type instead? */
	switch (ionic_opcd) {
	case OP_TYPE_SEND:
	case OP_TYPE_SEND_INV:
	case OP_TYPE_SEND_IMM:
		ionic_opcd = IBV_WC_SEND;
		break;
	case OP_TYPE_READ:
		ionic_opcd = IBV_WC_RDMA_READ;
		break;
	case OP_TYPE_WRITE:
	case OP_TYPE_WRITE_IMM:
		ionic_opcd = IBV_WC_RDMA_WRITE;
		break;
	case OP_TYPE_CMP_N_SWAP:
		ionic_opcd = IBV_WC_COMP_SWAP;
		break;
	case OP_TYPE_FETCH_N_ADD:
		ionic_opcd = IBV_WC_FETCH_ADD;
		break;
	case OP_TYPE_LOCAL_INV:
		ionic_opcd = IBV_WC_LOCAL_INV;
		break;
	case OP_TYPE_BIND_MW:
		ionic_opcd = IBV_WC_BIND_MW;
		break;
	default:
		ibv_opcd = 0;
	}

	return ibv_opcd;
}

static inline uint8_t ionic_to_ibv_wc_status(uint8_t wcst)
{
	uint8_t ibv_wcst;

	/* XXX should this use ionic_{req,rsp}_wc_status instead?
	 * also, do we really need two different enums for wc status? */
	switch (wcst) {
	case 0:
		ibv_wcst = IBV_WC_SUCCESS;
		break;
	case CQ_STATUS_LOCAL_LEN_ERR:
		ibv_wcst = IBV_WC_LOC_LEN_ERR;
		break;
	case CQ_STATUS_LOCAL_QP_OPER_ERR:
		ibv_wcst = IBV_WC_LOC_QP_OP_ERR;
		break;
	case CQ_STATUS_LOCAL_PROT_ERR:
		ibv_wcst = IBV_WC_LOC_PROT_ERR;
		break;
	case CQ_STATUS_WQE_FLUSHED_ERR:
		ibv_wcst = IBV_WC_WR_FLUSH_ERR;
		break;
	case CQ_STATUS_LOCAL_ACC_ERR:
		ibv_wcst = IBV_WC_LOC_ACCESS_ERR;
		break;
	case CQ_STATUS_REMOTE_INV_REQ_ERR:
		ibv_wcst = IBV_WC_REM_INV_REQ_ERR;
		break;
	case CQ_STATUS_REMOTE_ACC_ERR:
		ibv_wcst = IBV_WC_REM_ACCESS_ERR;
		break;
	case CQ_STATUS_REMOTE_OPER_ERR:
		ibv_wcst = IBV_WC_REM_OP_ERR;
		break;
	case CQ_STATUS_RNR_RETRY_EXCEEDED:
		ibv_wcst = IBV_WC_RNR_RETRY_EXC_ERR;
		break;
	case CQ_STATUS_RETRY_EXCEEDED:
		ibv_wcst = IBV_WC_RETRY_EXC_ERR;
		break;
	default:
		ibv_wcst = IBV_WC_GENERAL_ERR;
		break;
	}

	return ibv_wcst;
}

static inline bool ionic_op_is_local(uint8_t opcd)
{
	return opcd == OP_TYPE_LOCAL_INV ||
		opcd == OP_TYPE_BIND_MW;
}

#if 0
static inline uint8_t ionic_is_cqe_valid(struct ionic_cq *cq,
					 struct ionic_bcqe *hdr)
{
	udma_from_device_barrier();
	return ((le32toh(hdr->flg_st_typ_ph) &
		 IONIC_BCQE_PH_MASK) == cq->phase);
}

static inline void ionic_change_cq_phase(struct ionic_cq *cq)
{
	if (!cq->cqq.head)
		cq->phase = (~cq->phase & IONIC_BCQE_PH_MASK);
}
#endif

/** ionic_synchronize_cq - RCU-like barrier for cq polling access to qp table.
 *
 * Synchronize is to be used when modifying the qp table, after inserting, but
 * particularly after removing a qp from the table.  The synchronization
 * barrier will ensure that the qp is not referenced by a polling cq after the
 * qp is removed from the table and freed.
 *
 * This is similarly required for xrc srq in the qp table.
 *
 * Used as shown in synopsis, this synchronization barrier ensures that any
 * previous or concurrent round of polling has completed, and the next round of
 * polling will not observe the qp in the table.
 *
 * Used for insertion, a barrier between creating the qp and posting the first
 * work requests will guarantee that on the first work completion, some other
 * polling thread will observe the qp in the table.
 *
 * Synopsis:
 *
 * tbl_insert(&ctx->qp_tbl, qpid);
 *    ibv_poll_cq(cq, ...); // thread on other cpu, may observe qp
 * ionic_synchroize_cq(to_ionic_cq(qp->vqp.qp.send_cq));
 * ionic_synchroize_cq(to_ionic_cq(qp->vqp.qp.recv_cq));
 * ibv_post_send(qp, ...);
 *    ibv_poll_cq(cq, ...); // thread on other cpu, will observe qp
 *
 * tbl_delete(&ctx->qp_tbl, qpid);
 *    ibv_poll_cq(cq, ...); // thread on other cpu, may observe qp
 * ionic_synchroize_cq(to_ionic_cq(qp->vqp.qp.send_cq));
 * ionic_synchroize_cq(to_ionic_cq(qp->vqp.qp.recv_cq));
 *    ibv_poll_cq(cq, ...); // thread on other cpu, will not observe qp
 * free(qp);
 */
static inline void ionic_synchronize_cq(struct ionic_cq *cq)
{
	pthread_spin_lock(&cq->lock);
	pthread_spin_unlock(&cq->lock);
}

#endif /* IONIC_H */
