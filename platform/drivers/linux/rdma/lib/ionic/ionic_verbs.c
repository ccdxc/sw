#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ionic.h"

/* XXX use named constants or sizeof for 16, 32 */
static int ionic_max_recv_sge(uint16_t stride)
{
	return (stride - 16) / 16;
}

static int ionic_max_send_sge(uint16_t stride)
{
	return (stride - 32) / 16;
}

static int ionic_max_send_inline(uint16_t stride)
{
	return stride - 32;
}

static struct ibv_cq *ionic_create_cq(struct ibv_context *ibctx, int ncqe,
				      struct ibv_comp_channel *channel, int vec)
{
	struct ionic_dev *dev = to_ionic_dev(ibctx->device);
	struct ionic_cq *cq;
	struct ionic_cq_req req = {};
	struct ionic_cq_resp resp = {};

	int rc;

	IONIC_LOG("");
	/* XXX hardcode value */
	if (ncqe > 0xffff) {
		rc = EINVAL;
		goto err;
	}

	cq = calloc(1, sizeof(*cq));
	if (!cq) {
		rc = ENOMEM;
		goto err;
	}

	pthread_spin_init(&cq->lock, PTHREAD_PROCESS_PRIVATE);

	rc = ionic_queue_init(&cq->q, dev->pg_size, ncqe, sizeof(struct cqwqe_be_t));
	if (rc)
		goto err_queue;

	ionic_queue_color_init(&cq->q);

	req.cq.addr = (uintptr_t)cq->q.ptr;
	req.cq.size = cq->q.size;
	req.cq.mask = cq->q.mask;
	req.cq.stride = cq->q.stride;

	rc = ibv_cmd_create_cq(ibctx, ncqe, channel, vec, &cq->ibcq,
			       &req.req, sizeof(req),
			       &resp.resp, sizeof(resp));
	if (rc)
		goto err_cmd;

	cq->cqid = resp.cqid;

	ionic_queue_dbell_init(&cq->q, cq->cqid);

	return &cq->ibcq;

err_cmd:
	ionic_queue_destroy(&cq->q);
err_queue:
	pthread_spin_destroy(&cq->lock);
	free(cq);
err:
	errno = rc;
	return NULL;
}

static int ionic_resize_cq(struct ibv_cq *ibcq, int ncqe)
{
	return -ENOSYS;
}

static int ionic_destroy_cq(struct ibv_cq *ibcq)
{
	struct ionic_cq *cq = to_ionic_cq(ibcq);
	int rc;

	rc = ibv_cmd_destroy_cq(ibcq);
	if (rc)
		return rc;

	ionic_queue_destroy(&cq->q);
	pthread_spin_destroy(&cq->lock);
	free(cq);

	return 0;
}

/* XXX move these helpers to some interface header file */
static bool ionic_cqe_color(struct cqwqe_be_t *cqe)
{
	return (cqe->color_flags >> COLOR_SHIFT) != 0;
}

static uint32_t ionic_cqe_qpn(struct cqwqe_be_t *cqe)
{
	return ((uint32_t)cqe->qp_hi << 16) | be16toh(cqe->qp_lo);
}

static uint32_t ionic_cqe_src_qpn(struct cqwqe_be_t *cqe)
{
	return ((uint32_t)cqe->src_qp_hi << 16) | be16toh(cqe->src_qp_lo);
}

static int ionic_poll_recv(struct ionic_qp *cqe_qp, struct ibv_wc *wc,
			   struct cqwqe_be_t *cqe)
{
	struct ionic_qp *qp = NULL;
	struct ionic_rq_meta *meta;

	if (cqe_qp->has_rq) {
		qp = cqe_qp;
	} else {
		if (unlikely(cqe_qp->is_srq))
			return -EIO;

		if (unlikely(!cqe_qp->vqp.qp.srq))
			return -EIO;

		qp = to_ionic_srq(cqe_qp->vqp.qp.srq);
	}

	/* there had better be something in the recv queue to complete */
	if (ionic_queue_empty(&qp->rq))
		return -EIO;

	meta = &qp->rq_meta[qp->rq.cons];

	memset(wc, 0, sizeof(*wc));

	wc->status = ionic_to_ibv_wc_status(cqe->status);
	wc->vendor_err = cqe->status;

	wc->wr_id = cqe->id.wrid;
	wc->qp_num = cqe_qp->qpid;

	if (wc->status != IBV_WC_SUCCESS)
		goto out;

	if (cqe->op_type == OP_TYPE_RDMA_OPER_WITH_IMM) {
		wc->opcode = IBV_WC_RECV_RDMA_WITH_IMM;
		wc->wc_flags |= IBV_WC_WITH_IMM;
		wc->imm_data = cqe->imm_data; /* be32 in wc */
	} else {
		wc->opcode = IBV_WC_RECV;
		if (0 /* TODO: cqe has invalidated rkey */) {
			wc->wc_flags |= IBV_WC_WITH_INV;
			wc->invalidated_rkey = be32toh(cqe->imm_data);
		} else if (cqe->color_flags & IMM_DATA_VLD_MASK) {
			wc->wc_flags |= IBV_WC_WITH_IMM;
			wc->imm_data = cqe->imm_data; /* be32 in wc */
		}
	}

	wc->byte_len = meta->len; /* XXX byte_len must come from cqe */
	wc->src_qp = ionic_cqe_src_qpn(cqe);

	/* XXX: also need from cqe... pkey_index, slid, sl, dlid_path_bits */

out:
	ionic_queue_consume(&qp->rq);

	return 1;
}

static int ionic_poll_send(struct ionic_qp *qp, struct ibv_wc *wc,
			   struct cqwqe_be_t *cqe)
{
	struct ionic_sq_meta *meta;
	int npolled = 1;

	/* there had better be something in the send queue to complete */
	if (ionic_queue_empty(&qp->sq))
		return -EIO;

	meta = &qp->sq_meta[qp->sq.cons];

	memset(wc, 0, sizeof(*wc));

	wc->status = ionic_to_ibv_wc_status(cqe->status);
	wc->vendor_err = cqe->status;

	wc->wr_id = meta->wrid;
	wc->qp_num = qp->qpid;

	if (wc->status != IBV_WC_SUCCESS)
		goto out;

	if (!meta->signal) {
		npolled = 0;
		goto out;
	}

	wc->opcode = ionic_to_ibv_wc_opcd(cqe->op_type);
	wc->byte_len = meta->len; /* XXX byte_len should come from cqe */

	if (ionic_op_is_local(cqe->op_type))
		qp->sq_local = (qp->sq_local + 1) & 0xffffff;
	else
		qp->sq_msn = (qp->sq_msn + 1) & 0xffffff;

out:
	ionic_queue_consume(&qp->sq);

	return npolled;
}

static int ionic_poll_send_ok(struct ionic_qp *qp, struct ibv_wc *wc)
{
	struct ionic_sq_meta *meta;
	int npolled = 1;

	/* there had better be something in the send queue to complete OK */
	if (ionic_queue_empty(&qp->sq))
		return -EIO;

	meta = &qp->sq_meta[qp->sq.cons];

	if (!meta->signal) {
		npolled = 0;
		goto out;
	}

	wc->status = IBV_WC_SUCCESS;
	wc->vendor_err = 0;

	wc->wr_id = meta->wrid;
	wc->qp_num = qp->qpid;
	wc->opcode = ionic_to_ibv_wc_opcd(meta->op);
	wc->byte_len = be32toh(meta->len);

	if (ionic_op_is_local(meta->op))
		qp->sq_local = (qp->sq_local + 1) & 0xffffff;
	else
		qp->sq_msn = (qp->sq_msn + 1) & 0xffffff;

	ionic_queue_consume(&qp->sq);

out:
	return npolled;
}

static int ionic_poll_send_local_ok(struct ionic_qp *qp, struct ibv_wc *wc,
				    int nwc, uint32_t stop_local)
{
	int rc = 0, npolled = 0;

	// XXX disable cqe coalescing for now
	return 0;

	while (npolled < nwc) {
		if (qp->sq_local == stop_local)
			break;

		rc = ionic_poll_send_ok(qp, wc);
		if (rc < 0)
			break;

		npolled += rc;
		wc += rc;
	}

	return npolled ?: rc;
}

static int ionic_poll_send_msn_ok(struct ionic_qp *qp, struct ibv_wc *wc,
				  int nwc, uint32_t stop_msn)
{
	int rc = 0, npolled = 0;

	// XXX disable cqe coalescing for now
	return 0;

	while (npolled < nwc) {
		if (qp->sq_msn == stop_msn)
			break;

		rc = ionic_poll_send_ok(qp, wc);
		if (rc < 0)
			break;

		npolled += rc;
		wc += rc;
	}

	return npolled ?: rc;
}

static bool ionic_next_qp_cqe(struct ionic_cq *cq, struct ionic_qp **qp,
			      struct cqwqe_be_t *cqe)
{
	struct ionic_ctx *ctx = to_ionic_ctx(cq->ibcq.context);
	struct cqwqe_be_t *qcqe;

again:
	qcqe = ionic_queue_at_prod(&cq->q);

	if (ionic_queue_color(&cq->q) != ionic_cqe_color(qcqe))
		return false;

	udma_from_device_barrier();

	*cqe = *qcqe;

	ionic_dbg(ctx, "poll cq prod %d", cq->q.prod);
	ionic_dbg_xdump(ctx, "cqe", cqe, cq->q.stride);

	ionic_dbg_cqe(ctx, cqe);

	*qp = tbl_lookup(&ctx->qp_tbl, ionic_cqe_qpn(cqe));

	/* Skip this cqe if the qp was destroyed.
	 * Unlikely race: qp was destroyed, and new qp created with same id.
	 */
	if (unlikely(!*qp)) {
		ionic_queue_produce(&cq->q);
		ionic_queue_color_wrap(&cq->q);
		goto again;
	}

	return true;
}

static int ionic_poll_cq(struct ibv_cq *ibcq, int nwc, struct ibv_wc *wc)
{
	struct ionic_ctx *ctx = to_ionic_ctx(ibcq->context);
	struct ionic_cq *cq = to_ionic_cq(ibcq);
	struct ionic_qp *qp;
	struct cqwqe_be_t cqe;
	uint16_t old_prod;
	int rc = 0, npolled = 0;

	/* Note about rc: (noted here because poll is different)
	 *
	 * Functions without "poll" in the name, if they return an integer,
	 * return zero on success, or a positive error number.  Functions
	 * returning a pointer return NULL on error and set errno to a positve
	 * error number.
	 *
	 * Functions with "poll" in the name return negative error numbers.
	 * Functions that poll zero zero or more completions (ionic_poll_cq,
	 * ionic_poll_xxx_ok), positive values including zero indicate a number
	 * of successful completions polled.  Functions tht poll exactly one
	 * completion (ionic_poll_send, ionic_poll_recv), only zero indicates
	 * success.
	 */

	if (nwc < 1)
		return 0;

	pthread_spin_lock(&cq->lock);

	old_prod = cq->q.prod;

	while (npolled < nwc) {
		if (!ionic_next_qp_cqe(cq, &qp, &cqe))
			break;

		/* ionic cqe op_type must be valid even if status is not ok */
		switch(cqe.op_type) {
		case OP_TYPE_SEND:
		case OP_TYPE_SEND_INV:
		case OP_TYPE_SEND_IMM:
		case OP_TYPE_READ:
		case OP_TYPE_WRITE:
		case OP_TYPE_WRITE_IMM:
		case OP_TYPE_CMP_N_SWAP:
		case OP_TYPE_FETCH_N_ADD:
		case OP_TYPE_LOCAL_INV:
		case OP_TYPE_BIND_MW:
			pthread_spin_lock(&qp->sq_lock);

			/* combined work completions are successful */
			if (ionic_op_is_local(cqe.op_type))
				rc = ionic_poll_send_local_ok(qp, wc + npolled,
							      nwc - npolled,
							      be32toh(cqe.id.msn));
			else
				rc = ionic_poll_send_msn_ok(qp, wc + npolled,
							    nwc - npolled,
							    be32toh(cqe.id.msn));

			if (rc < 0) {
				pthread_spin_unlock(&qp->sq_lock);
				goto out;
			}

			npolled += rc;

			/* is another round of polling needed for this cqe? */
			if (npolled == nwc) {
				pthread_spin_unlock(&qp->sq_lock);
				goto out;
			}

			/* poll the current work completion */
			rc = ionic_poll_send(qp, wc + npolled, &cqe);
			if (rc < 0) {
				pthread_spin_unlock(&qp->sq_lock);
				goto out;
			}

			npolled += rc;

			pthread_spin_unlock(&qp->sq_lock);
			break;

		case OP_TYPE_SEND_RCVD:
		case OP_TYPE_RDMA_OPER_WITH_IMM:
			pthread_spin_lock(&qp->rq_lock);

			/* poll the current work completion */
			rc = ionic_poll_recv(qp, wc + npolled, &cqe);
			if (rc < 0) {
				pthread_spin_unlock(&qp->rq_lock);
				goto out;
			}

			npolled += rc;

			pthread_spin_unlock(&qp->rq_lock);
			break;

		default:
			IONIC_LOG("Unhandled op type %d\n", cqe.op_type);
		}

		/* All the wc for this cqe have been polled, and we advance cq.
		 * Any error, or npolled == nwc, goto out instead.
		 */
		ionic_queue_produce(&cq->q);
		ionic_queue_color_wrap(&cq->q);
	}

out:
	if (likely(cq->q.prod != old_prod)) {
		ionic_dbg(ctx, "dbell qtype %d val %#lx\n",
			  ctx->cq_qtype, ionic_queue_dbell_val(&cq->q));
		ionic_dbell_ring(&ctx->dbpage[ctx->cq_qtype],
				 ionic_queue_dbell_val(&cq->q));
	}

	pthread_spin_unlock(&cq->lock);

	return npolled ?: rc;
}

static int ionic_req_notify_cq(struct ibv_cq *ibcq, int solicited_only)
{
	struct ionic_ctx *ctx = to_ionic_ctx(ibcq->context);
	struct ionic_cq *cq = to_ionic_cq(ibcq);

	IONIC_LOG("");

	/* XXX need some ring bits for "solicited only" */
	if (solicited_only)
		ionic_dbg(ctx, "XXX solicited_only=%d", solicited_only);

	ionic_dbell_ring(&ctx->dbpage[ctx->cq_qtype],
			 ionic_queue_dbell_val_arm(&cq->q));

	return 0;
}

static int ionic_check_qp_limits(struct ionic_ctx *ctx,
				 struct ibv_qp_cap *cap)
{
	struct ibv_device_attr devattr;
	int rc;

	/* XXX unnecessary system call */
	rc = ibv_query_device(&ctx->vctx.context, &devattr);
	if (rc)
		return rc;

	if (cap->max_send_sge > devattr.max_sge)
		return EINVAL;
	if (cap->max_recv_sge > devattr.max_sge)
		return EINVAL;
	if (cap->max_inline_data > IONIC_MAX_INLINE_SIZE)
		return EINVAL;
	if (cap->max_send_wr > devattr.max_qp_wr)
		cap->max_send_wr = devattr.max_qp_wr;
	if (cap->max_recv_wr > devattr.max_qp_wr)
		cap->max_recv_wr = devattr.max_qp_wr;

	return 0;
}

static int ionic_alloc_queues(struct ionic_ctx *ctx, struct ionic_qp *qp,
			      struct ibv_qp_cap *cap)
{
	struct ionic_dev *dev = to_ionic_dev(ctx->vctx.context.device);
	uint16_t min_depth, min_stride;
	int rc;

	if (qp->has_sq) {
		min_depth = cap->max_send_wr;
		min_stride = ionic_get_sqe_size(cap->max_send_sge,
						cap->max_inline_data);

		rc = ionic_queue_init(&qp->sq, dev->pg_size,
				      min_depth, min_stride);
		if (rc)
			goto err_sq;

		qp->sq_hbm_ptr = NULL;
		qp->sq_hbm_prod = 0;

		qp->sq_meta = calloc((uint32_t)qp->sq.mask + 1,
				     sizeof(*qp->sq_meta));
		if (!qp->sq_meta) {
			rc = ENOMEM;
			goto err_sq_meta;
		}

		pthread_spin_init(&qp->sq_lock, PTHREAD_PROCESS_PRIVATE);
	}

	if (qp->has_rq) {
		min_depth = cap->max_recv_wr;
		min_stride = ionic_get_rqe_size(cap->max_recv_sge);

		rc = ionic_queue_init(&qp->rq, dev->pg_size,
				      min_depth, min_stride);
		if (rc)
			goto err_rq;

		qp->rq_meta = calloc((uint32_t)qp->rq.mask + 1,
				     sizeof(*qp->rq_meta));
		if (!qp->rq_meta) {
			rc = ENOMEM;
			goto err_rq_meta;
		}

		pthread_spin_init(&qp->rq_lock, PTHREAD_PROCESS_PRIVATE);
	}

	return 0;

err_rq_meta:
	ionic_queue_destroy(&qp->rq);
err_rq:
	if (qp->has_sq) {
		pthread_spin_destroy(&qp->sq_lock);
		free(qp->sq_meta);
err_sq_meta:
		ionic_queue_destroy(&qp->sq);
	}
err_sq:
	return rc;
}

static void ionic_free_queues(struct ionic_qp *qp)
{
	if (qp->has_rq) {
		pthread_spin_destroy(&qp->rq_lock);
		free(qp->rq_meta);
		ionic_queue_destroy(&qp->rq);
	}

	if (qp->has_sq) {
		pthread_spin_destroy(&qp->sq_lock);
		free(qp->sq_meta);
		ionic_queue_destroy(&qp->sq);
	}
}

static struct ibv_qp *ionic_create_qp_ex(struct ibv_context *ibctx,
					 struct ibv_qp_init_attr_ex *ex)
{
	struct ionic_ctx *ctx = to_ionic_ctx(ibctx);
	struct ionic_qp *qp;
	struct ionic_qp_req req;
	struct ionic_qp_resp resp;
	int rc;

	rc = ionic_check_qp_limits(ctx, &ex->cap);
	if (rc)
		goto err_qp;

	qp = calloc(1, sizeof(*qp));
	if (!qp) {
		rc = ENOMEM;
		goto err_qp;
	}

	qp->vqp.qp.qp_type = ex->qp_type;
	qp->vqp.qp.srq = ex->srq;

	qp->has_sq = ex->qp_type != IBV_QPT_XRC_RECV;

	qp->has_rq = !ex->srq &&
		ex->qp_type != IBV_QPT_XRC_SEND &&
		ex->qp_type != IBV_QPT_XRC_RECV;

	qp->is_srq = false;

	qp->sig_all = ex->sq_sig_all;

	rc = ionic_alloc_queues(ctx, qp, &ex->cap);
	if (rc)
		goto err_queues;

	req.sq.addr = (uintptr_t)qp->sq.ptr;
	req.sq.size = qp->sq.size;
	req.sq.mask = qp->sq.mask;
	req.sq.stride = qp->sq.stride;

	req.rq.addr = (uintptr_t)qp->rq.ptr;
	req.rq.size = qp->rq.size;
	req.rq.mask = qp->rq.mask;
	req.rq.stride = qp->rq.stride;

	rc = ibv_cmd_create_qp_ex2(ibctx, &qp->vqp, sizeof(qp->vqp), ex,
				   &req.req, sizeof(req.req), sizeof(req),
				   &resp.resp, sizeof(resp.resp), sizeof(resp));
	if (rc)
		goto err_cmd;

	if (resp.sq_hbm_offset) {
		qp->sq_hbm_ptr = ionic_map_device(qp->sq.size,
						  ctx->vctx.context.cmd_fd,
						  resp.sq_hbm_offset);
		if (!qp->sq_hbm_ptr) {
			rc = errno;
			goto err_hbm;
		}
	}

	qp->qpid = resp.qpid;
	ionic_queue_dbell_init(&qp->sq, qp->qpid);
	ionic_queue_dbell_init(&qp->rq, qp->qpid);

	pthread_mutex_lock(&ctx->mut);
	tbl_alloc_node(&ctx->qp_tbl);
	tbl_insert(&ctx->qp_tbl, qp, qp->qpid);
	pthread_mutex_unlock(&ctx->mut);

	ionic_synchronize_cq(to_ionic_cq(ex->send_cq));
	ionic_synchronize_cq(to_ionic_cq(ex->recv_cq));

	ex->cap.max_send_wr = qp->sq.mask;
	ex->cap.max_recv_wr = qp->rq.mask;
	ex->cap.max_send_sge = ionic_max_send_sge(qp->sq.stride);
	ex->cap.max_recv_sge = ionic_max_recv_sge(qp->rq.stride);
	ex->cap.max_inline_data = ionic_max_send_inline(qp->sq.stride);

	return &qp->vqp.qp;

err_hbm:
	ibv_cmd_destroy_qp(&qp->vqp.qp);
err_cmd:
	ionic_free_queues(qp);
err_queues:
	free(qp);
err_qp:
	errno = rc;
	return NULL;
}

static int ionic_modify_qp(struct ibv_qp *ibqp,
			   struct ibv_qp_attr *attr,
			   int attr_mask)
{
	struct ibv_modify_qp_ex cmd = {};
	struct ib_uverbs_ex_modify_qp_resp resp = {};
	struct ionic_qp *qp = to_ionic_qp(ibqp);
	int rc;

	IONIC_LOG("");
	/* Sanity check */
	if (!attr_mask)
		return 0;

	rc = ibv_cmd_modify_qp_ex(ibqp, attr, attr_mask,
				  &cmd, sizeof(cmd), sizeof(cmd),
				  &resp, sizeof(resp), sizeof(resp));
	if (!rc) {
		if (attr_mask & IBV_QP_STATE) {
			/* transition to reset */
			if (attr->qp_state == IBV_QPS_RESET) {
				qp->sq_msn = 1;
				qp->sq_local = 1;
				qp->sq.prod = 0;
				qp->sq.cons = 0;
				qp->rq.prod = 0;
				qp->rq.cons = 0;
			}
		}
	}

	return rc;
}

static int ionic_query_qp(struct ibv_qp *ibqp,
			  struct ibv_qp_attr *attr,
			  int attr_mask,
			  struct ibv_qp_init_attr *init_attr)
{
	struct ibv_query_qp cmd;
	int rc;

	IONIC_LOG("");
	rc = ibv_cmd_query_qp(ibqp, attr, attr_mask, init_attr,
			      &cmd, sizeof(cmd));

	init_attr->cap.max_inline_data = IONIC_MAX_INLINE_SIZE;

	attr->cap = init_attr->cap;

	return rc;
}

static int ionic_destroy_qp(struct ibv_qp *ibqp)
{
	struct ionic_ctx *ctx = to_ionic_ctx(ibqp->context);
	struct ionic_qp *qp = to_ionic_qp(ibqp);
	int rc;

	IONIC_LOG("");
	rc = ibv_cmd_destroy_qp(ibqp);
	if (rc)
		return rc;

	pthread_mutex_lock(&ctx->mut);
	tbl_free_node(&ctx->qp_tbl);
	tbl_delete(&ctx->qp_tbl, qp->qpid);
	pthread_mutex_unlock(&ctx->mut);

	ionic_synchronize_cq(to_ionic_cq(ibqp->send_cq));
	ionic_synchronize_cq(to_ionic_cq(ibqp->recv_cq));

	ionic_unmap(qp->sq_hbm_ptr, qp->sq.size);

	pthread_spin_destroy(&qp->rq_lock);
	pthread_spin_destroy(&qp->sq_lock);
	ionic_queue_destroy(&qp->rq);
	ionic_queue_destroy(&qp->sq);
	free(qp);

	return 0;
}

static int64_t ionic_prep_inline(void *data, size_t max_data,
				 struct ibv_sge *ibv_sgl, uint32_t num_sge)
{
	static const int64_t bit_31 = 1u << 31;
	int64_t len = 0, sg_len;
	int sg_i;

	for (sg_i = 0; sg_i < num_sge; ++sg_i) {
		sg_len = ibv_sgl[sg_i].length;

		/* sge length zero means 2GB */
		if (unlikely(sg_len == 0))
			sg_len = bit_31;

		/* greater than max inline data is invalid */
		if (unlikely(len + sg_len > max_data))
			return -EINVAL;

		memcpy(data + len, (void *)ibv_sgl[sg_i].addr, sg_len);

		len += sg_len;
	}

	return len;
}

static int64_t ionic_prep_sgl(struct sge_t *sgl, uint32_t max_sge,
			      struct ibv_sge *ibv_sgl, uint32_t num_sge)
{
	static const int64_t bit_31 = 1l << 31;
	int64_t len = 0, sg_len;
	int sg_i;

	if (unlikely(num_sge > max_sge))
		return -EINVAL;

	for (sg_i = 0; sg_i < num_sge; ++sg_i) {
		sg_len = ibv_sgl[sg_i].length;

		/* sge length zero means 2GB */
		if (unlikely(sg_len == 0))
			sg_len = bit_31;

		/* greater than 2GB data is invalid */
		if (unlikely(len + sg_len > bit_31))
			return -EINVAL;

		sgl[sg_i].va = htobe64(ibv_sgl[sg_i].addr);
		sgl[sg_i].lkey = htobe32(ibv_sgl[sg_i].lkey);
		sgl[sg_i].len = htobe32(sg_len);

		len += sg_len;
	}

	return len;
}

static void ionic_prep_base(struct ionic_qp *qp,
			    struct ibv_send_wr *wr,
			    struct ionic_sq_meta *meta,
			    struct sqwqe_t *wqe)
{
	meta->wrid = wr->wr_id;
	meta->signal = false;

	/* XXX wqe wrid can be removed */
	wqe->base.wrid = 0;

	if (wr->send_flags & IBV_SEND_FENCE)
		wqe->base.fence = 1;

	if (wr->send_flags & IBV_SEND_SOLICITED)
		wqe->base.solicited_event = 1;

	if (qp->sig_all || wr->send_flags & IBV_SEND_SIGNALED) {
		wqe->base.complete_notify = 1;
		meta->signal = true;
	}
}

static int ionic_prep_common(struct ionic_qp *qp,
			     struct ibv_send_wr *wr,
			     struct ionic_sq_meta *meta,
			     struct sqwqe_t *wqe,
			     /* XXX length field offset differs per opcode */
			     __u32 *wqe_length_field)
{
	int64_t signed_len;

	ionic_prep_base(qp, wr, meta, wqe);

	if (wr->send_flags & IBV_SEND_INLINE) {
		wqe->base.num_sges = 0;
		wqe->base.inline_data_vld = 1;
		signed_len = ionic_prep_inline(wqe->u.non_atomic.sg_arr,
					       ionic_max_send_inline(qp->sq.stride),
					       wr->sg_list, wr->num_sge);
	} else {
		wqe->base.num_sges = wr->num_sge;
		signed_len = ionic_prep_sgl(wqe->u.non_atomic.sg_arr,
					    ionic_max_send_sge(qp->sq.stride),
					    wr->sg_list, wr->num_sge);
	}

	if (unlikely(signed_len < 0))
		return (int)-signed_len;

	meta->len = (uint32_t)signed_len;
	*wqe_length_field = htobe32((uint32_t)signed_len);

	return 0;
}

static int ionic_prep_send(struct ionic_qp *qp,
			   struct ibv_send_wr *wr)
{
	struct ionic_sq_meta *meta;
	struct sqwqe_t *wqe;

	meta = &qp->sq_meta[qp->sq.prod];
	wqe = ionic_queue_at_prod(&qp->sq);

	memset(wqe, 0, qp->sq.stride);

	switch (wr->opcode) {
	case IBV_WR_SEND:
		meta->op = IONIC_WR_OPCD_SEND;
		wqe->base.op_type = IONIC_WR_OPCD_SEND;
		break;
	case IBV_WR_SEND_WITH_IMM:
		meta->op = IONIC_WR_OPCD_SEND_IMM;
		wqe->base.op_type = IONIC_WR_OPCD_SEND_IMM;
		wqe->u.non_atomic.wqe.send.imm_data = wr->imm_data;
		break;
	case IBV_WR_SEND_WITH_INV:
		meta->op = IONIC_WR_OPCD_SEND_INVAL;
		wqe->base.op_type = IONIC_WR_OPCD_SEND_INVAL;
		wqe->u.non_atomic.wqe.send.imm_data =
			htobe32(wr->invalidate_rkey);

		/* XXX just use imm_data field */
		wqe->u.non_atomic.wqe.send.inv_key =
			htobe32(wr->invalidate_rkey);
		break;
	default:
		return EINVAL;
	}

	return ionic_prep_common(qp, wr, meta, wqe,
				 &wqe->u.non_atomic.wqe.send.length);
}

static int ionic_prep_send_ud(struct ionic_qp *qp,
					      struct ibv_send_wr *wr)
{
	struct ionic_sq_meta *meta;
	struct sqwqe_t *wqe;
	struct ionic_ah *ah;
	int rc;

	/* XXX duplicated code.
	 *
	 * This can be cleaned up by making sqwqe_rc_send_t and sqwqe_ud_send_t
	 * the same descriptor format, and replacing following dup'd code with
	 * call to ionic_prep_send().
	 */

	meta = &qp->sq_meta[qp->sq.prod];
	wqe = ionic_queue_at_prod(&qp->sq);

	memset(wqe, 0, qp->sq.stride);

	switch (wr->opcode) {
	case IBV_WR_SEND:
		meta->op = IONIC_WR_OPCD_SEND;
		wqe->base.op_type = IONIC_WR_OPCD_SEND;
		break;
	case IBV_WR_SEND_WITH_IMM:
		meta->op = IONIC_WR_OPCD_SEND_IMM;
		wqe->base.op_type = IONIC_WR_OPCD_SEND_IMM;
		wqe->u.non_atomic.wqe.ud_send.imm_data = wr->imm_data;
		break;
	default:
		return EINVAL;
	}

	rc = ionic_prep_common(qp, wr, meta, wqe,
			       &wqe->u.non_atomic.wqe.ud_send.length);

	/* XXX replace duplicated code with ionic_prep_send: */
	// rc = ionic_prep_send(qp, wr);
	if (rc)
		return rc;

	if (unlikely(!wr->wr.ud.ah))
		return EINVAL;

	ah = to_ionic_ah(wr->wr.ud.ah);

	wqe = ionic_queue_at_prod(&qp->sq);

	/* XXX endian? */
	wqe->u.non_atomic.wqe.ud_send.q_key = wr->wr.ud.remote_qkey;
	wqe->u.non_atomic.wqe.ud_send.dst_qp = wr->wr.ud.remote_qpn;
	wqe->u.non_atomic.wqe.ud_send.ah_handle = ah->avid;

	return 0;
}

static int ionic_prep_rdma(struct ionic_qp *qp,
			   struct ibv_send_wr *wr)
{
	struct ionic_sq_meta *meta;
	struct sqwqe_t *wqe;

	meta = &qp->sq_meta[qp->sq.prod];
	wqe = ionic_queue_at_prod(&qp->sq);

	memset(wqe, 0, qp->sq.stride);

	switch (wr->opcode) {
	case IBV_WR_RDMA_READ:
		if (wr->send_flags & (IBV_SEND_SOLICITED | IBV_SEND_INLINE))
			return EINVAL;
		meta->op = IONIC_WR_OPCD_RDMA_READ;
		wqe->base.op_type = IONIC_WR_OPCD_RDMA_READ;
		break;
	case IBV_WR_RDMA_WRITE:
		if (wr->send_flags & IBV_SEND_SOLICITED)
			return EINVAL;
		meta->op = IONIC_WR_OPCD_RDMA_WRITE;
		wqe->base.op_type = IONIC_WR_OPCD_RDMA_WRITE;
		break;
	case IBV_WR_RDMA_WRITE_WITH_IMM:
		meta->op = IONIC_WR_OPCD_RDMA_WRITE_IMM;
		wqe->base.op_type = IONIC_WR_OPCD_RDMA_WRITE_IMM;
		wqe->u.non_atomic.wqe.ud_send.imm_data = wr->imm_data;
		break;
	default:
		return EINVAL;
	}

	/* XXX endian? */
	wqe->u.non_atomic.wqe.rdma.va = wr->wr.rdma.remote_addr;
	wqe->u.non_atomic.wqe.rdma.r_key = wr->wr.rdma.rkey;

	return ionic_prep_common(qp, wr, meta, wqe,
				 &wqe->u.non_atomic.wqe.ud_send.length);
}

static int ionic_prep_atomic(struct ionic_qp *qp,
			     struct ibv_send_wr *wr)
{
	struct ionic_sq_meta *meta;
	struct sqwqe_t *wqe;

	if (wr->num_sge != 1 || wr->sg_list[0].length != 8)
		return EINVAL;

	if (wr->send_flags & (IBV_SEND_SOLICITED | IBV_SEND_INLINE))
		return EINVAL;

	meta = &qp->sq_meta[qp->sq.prod];
	wqe = ionic_queue_at_prod(&qp->sq);

	memset(wqe, 0, qp->sq.stride);

	switch (wr->opcode) {
	case IBV_WR_ATOMIC_CMP_AND_SWP:
		meta->op = IONIC_WR_OPCD_ATOMIC_CS;
		wqe->base.op_type = IONIC_WR_OPCD_ATOMIC_CS;
		wqe->u.atomic.swap_or_add_data =
			htobe64(wr->wr.atomic.swap);
		wqe->u.atomic.cmp_data =
			htobe64(wr->wr.atomic.compare_add);
		break;
	case IBV_WR_ATOMIC_FETCH_AND_ADD:
		meta->op = IONIC_WR_OPCD_ATOMIC_FA;
		wqe->base.op_type = IONIC_WR_OPCD_ATOMIC_FA;
		wqe->u.atomic.swap_or_add_data =
			htobe64(wr->wr.atomic.compare_add);
		break;
	default:
		return EINVAL;
	}

	/* XXX endian? */
	wqe->u.atomic.r_key = wr->wr.atomic.rkey;
	wqe->u.atomic.va = wr->wr.atomic.remote_addr;

	wqe->base.num_sges = 1;
	wqe->u.atomic.sge.va = htobe64(wr->sg_list[0].addr);
	wqe->u.atomic.sge.lkey = htobe32(wr->sg_list[0].lkey);
	wqe->u.atomic.sge.len = htobe32(8);

	ionic_prep_base(qp, wr, meta, wqe);

	return 0;
}

static int ionic_prep_one_rc(struct ionic_qp *qp,
			     struct ibv_send_wr *wr)
{
	struct ionic_ctx *ctx = to_ionic_ctx(qp->vqp.qp.context);
	int rc = 0;

	switch (wr->opcode) {
	case IBV_WR_SEND:
	case IBV_WR_SEND_WITH_IMM:
	case IBV_WR_SEND_WITH_INV:
		rc = ionic_prep_send(qp, wr);
		break;
	case IBV_WR_RDMA_READ:
	case IBV_WR_RDMA_WRITE:
	case IBV_WR_RDMA_WRITE_WITH_IMM:
		rc = ionic_prep_rdma(qp, wr);
		break;
	case IBV_WR_ATOMIC_CMP_AND_SWP:
	case IBV_WR_ATOMIC_FETCH_AND_ADD:
		rc = ionic_prep_atomic(qp, wr);
		break;
	default:
		ionic_dbg(ctx, "invalid opcode %d", wr->opcode);
		rc = EINVAL;
	}

	return rc;
}

static int ionic_prep_one_ud(struct ionic_qp *qp,
			     struct ibv_send_wr *wr)
{
	struct ionic_ctx *ctx = to_ionic_ctx(qp->vqp.qp.context);
	int rc = 0;

	switch (wr->opcode) {
	case IBV_WR_SEND:
	case IBV_WR_SEND_WITH_IMM:
		rc = ionic_prep_send_ud(qp, wr);
	default:
		ionic_dbg(ctx, "invalid opcode %d", wr->opcode);
		rc = EINVAL;
	}

	return rc;
}

static void ionic_post_hbm(struct ionic_ctx *ctx, struct ionic_qp *qp)
{
	void *hbm_ptr;
	void *wqe_ptr;
	uint16_t pos, end, mask, stride;

	pos = qp->sq_hbm_prod;
	end = qp->sq.prod;
	mask = qp->sq.mask;
	stride = qp->sq.stride;

	while (pos != end) {
		hbm_ptr = qp->sq_hbm_ptr + pos * stride;
		wqe_ptr = ionic_queue_at(&qp->sq, pos);

		mmio_wc_start();
		mmio_memcpy_x64(hbm_ptr, wqe_ptr, qp->sq.stride);
		mmio_flush_writes();

		pos = (pos + 1) & mask;

		ionic_dbell_ring(&ctx->dbpage[ctx->sq_qtype],
				 qp->sq.dbell | pos);
	}

	qp->sq_hbm_prod = end;
}

static int ionic_post_send(struct ibv_qp *ibqp,
			   struct ibv_send_wr *wr,
			   struct ibv_send_wr **bad)
{
	struct ionic_ctx *ctx = to_ionic_ctx(ibqp->context);
	struct ionic_qp *qp = to_ionic_qp(ibqp);
	uint16_t old_prod;
	int rc = 0;

	IONIC_LOG("");

	if (!bad)
		return EINVAL;

	pthread_spin_lock(&qp->sq_lock);

	old_prod = qp->sq.prod;

	if (ibqp->qp_type == IBV_QPT_UD) {
		while (wr) {
			if (ionic_queue_full(&qp->sq)) {
				ionic_dbg(ctx, "queue full");
				rc = ENOMEM;
				goto out;
			}

			rc = ionic_prep_one_ud(qp, wr);
			if (rc)
				goto out;

			ionic_dbg(ctx, "post send prod %d", qp->sq.prod);
			ionic_dbg_xdump(ctx, "wqe", ionic_queue_at_prod(&qp->sq), qp->sq.stride);

			ionic_queue_produce(&qp->sq);

			wr = wr->next;
		}
	} else {
		while (wr) {
			if (ionic_queue_full(&qp->sq)) {
				ionic_dbg(ctx, "queue full");
				rc = ENOMEM;
				goto out;
			}

			rc = ionic_prep_one_rc(qp, wr);
			if (rc)
				goto out;

			ionic_dbg(ctx, "post send prod %d", qp->sq.prod);
			ionic_dbg_xdump(ctx, "wqe", ionic_queue_at_prod(&qp->sq), qp->sq.stride);

			ionic_queue_produce(&qp->sq);

			wr = wr->next;
		}
	}

out:
	if (likely(qp->sq.prod != old_prod)) {
		if (qp->sq_hbm_ptr) {
			ionic_post_hbm(ctx, qp);
		} else {
			udma_to_device_barrier();
			ionic_dbell_ring(&ctx->dbpage[ctx->sq_qtype],
					 ionic_queue_dbell_val(&qp->sq));
		}
	}

	pthread_spin_unlock(&qp->sq_lock);

	*bad = wr;
	return rc;
}

static int ionic_prep_recv(struct ionic_qp *qp,
			   struct ibv_recv_wr *wr)
{
	struct ionic_rq_meta *meta;
	struct rqwqe_t *wqe;
	int64_t signed_len;

	meta = &qp->rq_meta[qp->rq.prod];
	wqe = ionic_queue_at_prod(&qp->rq);

	memset(wqe, 0, qp->rq.stride);

	signed_len = ionic_prep_sgl(wqe->sge_arr,
				    ionic_max_recv_sge(qp->rq.stride),
				    wr->sg_list, wr->num_sge);
	if (signed_len < 0)
		return (int)-signed_len;

	wqe->wrid = wr->wr_id;
	wqe->num_sges = wr->num_sge;

	meta->len = (uint32_t)signed_len;

	return 0;
}

static int ionic_post_recv(struct ibv_qp *ibqp,
			   struct ibv_recv_wr *wr,
			   struct ibv_recv_wr **bad)
{
	struct ionic_ctx *ctx = to_ionic_ctx(ibqp->context);
	struct ionic_qp *qp = to_ionic_qp(ibqp);
	uint16_t old_prod;
	int rc = 0;

	IONIC_LOG("");

	if (!bad)
		return EINVAL;

	pthread_spin_lock(&qp->rq_lock);

	old_prod = qp->rq.prod;

	while (wr) {
		if (ionic_queue_full(&qp->rq)) {
			ionic_dbg(ctx, "queue full");
			rc = ENOMEM;
			goto out;
		}

		rc = ionic_prep_recv(qp, wr);
		if (rc)
			goto out;

		ionic_dbg(ctx, "post recv prod %d", qp->rq.prod);
		ionic_dbg_xdump(ctx, "wqe", ionic_queue_at_prod(&qp->rq), qp->rq.stride);

		ionic_queue_produce(&qp->rq);

		wr = wr->next;
	}

out:
	if (likely(qp->rq.prod != old_prod)) {
		udma_to_device_barrier();
		ionic_dbell_ring(&ctx->dbpage[ctx->rq_qtype],
				 ionic_queue_dbell_val(&qp->rq));
	}

	pthread_spin_unlock(&qp->rq_lock);

	*bad = wr;
	return rc;
}

static struct ibv_srq *ionic_create_srq_ex(struct ibv_context *ibctx,
					   struct ibv_srq_init_attr_ex *ex)
{
	struct ionic_ctx *ctx = to_ionic_ctx(ibctx);
	struct ionic_qp *qp;
	struct ibv_qp_cap cap = {
		.max_recv_wr = ex->attr.max_wr,
		.max_recv_sge = ex->attr.max_sge,
	};
	int rc;

	qp = calloc(1, sizeof(*qp));
	if (!qp) {
		rc = ENOMEM;
		goto err_qp;
	}

	qp->has_sq = false;
	qp->has_rq = true;
	qp->is_srq = true;

	qp->sig_all = false;

	rc = ionic_alloc_queues(ctx, qp, &cap);
	if (rc)
		goto err_queues;

	/* TODO: send command */
	rc = ENOSYS;
	if (rc)
		goto err_cmd;

	ex->attr.max_wr = qp->rq.mask;
	ex->attr.max_sge = ionic_max_recv_sge(qp->rq.stride);

	return &qp->vsrq.srq;

err_cmd:
	ionic_free_queues(qp);
err_queues:
	free(qp);
err_qp:
	errno = rc;
	return NULL;
}

static int ionic_modify_srq(struct ibv_srq *ibsrq, struct ibv_srq_attr *attr,
			    int init_attr)
{
	IONIC_LOG("");
	return -ENOSYS;
}

static int ionic_destroy_srq(struct ibv_srq *ibsrq)
{
	IONIC_LOG("");
	return -ENOSYS;
}

static int ionic_query_srq(struct ibv_srq *ibsrq, struct ibv_srq_attr *attr)
{
	IONIC_LOG("");
	return -ENOSYS;
}

static int ionic_post_srq_recv(struct ibv_srq *ibsrq, struct ibv_recv_wr *wr,
			       struct ibv_recv_wr **bad)
{
	IONIC_LOG("");
	return -ENOSYS;
}

static struct ibv_ah *ionic_create_ah(struct ibv_pd *ibpd,
				      struct ibv_ah_attr *attr)
{
	IONIC_LOG("");
	errno = ENOSYS;
	return NULL;
}

static int ionic_destroy_ah(struct ibv_ah *ibah)
{
	IONIC_LOG("");
	errno = ENOSYS;
	return -1;
}

const struct verbs_context_ops ionic_ctx_ops = {
	.create_cq		= ionic_create_cq,
	.poll_cq		= ionic_poll_cq,
	.req_notify_cq		= ionic_req_notify_cq,
	.resize_cq		= ionic_resize_cq,
	.destroy_cq		= ionic_destroy_cq,
	.create_srq_ex		= ionic_create_srq_ex,
	.modify_srq		= ionic_modify_srq,
	.query_srq		= ionic_query_srq,
	.destroy_srq		= ionic_destroy_srq,
	.post_srq_recv		= ionic_post_srq_recv,
	.create_qp_ex		= ionic_create_qp_ex,
	.query_qp		= ionic_query_qp,
	.modify_qp		= ionic_modify_qp,
	.destroy_qp		= ionic_destroy_qp,
	.post_send		= ionic_post_send,
	.post_recv		= ionic_post_recv,
	.create_ah		= ionic_create_ah,
	.destroy_ah		= ionic_destroy_ah
};
