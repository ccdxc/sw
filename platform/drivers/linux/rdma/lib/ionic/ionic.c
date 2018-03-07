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

static int ionic_query_device(struct ibv_context *ibctx,
			      struct ibv_device_attr *dev_attr)
{
	struct ibv_query_device cmd;
	uint64_t fw_ver;
	int status;

	IONIC_LOG("");
	memset(dev_attr, 0, sizeof(struct ibv_device_attr));
	status = ibv_cmd_query_device(ibctx, dev_attr, &fw_ver,
				      &cmd, sizeof(cmd));
	return status;
}

static int ionic_query_port(struct ibv_context *ibctx, uint8_t port,
			    struct ibv_port_attr *port_attr)
{
	struct ibv_query_port cmd;

	IONIC_LOG("");
	memset(port_attr, 0, sizeof(struct ibv_port_attr));
	return ibv_cmd_query_port(ibctx, port, port_attr, &cmd, sizeof(cmd));
}

static struct ibv_pd *ionic_alloc_pd(struct ibv_context *ibctx)
{
	struct ibv_alloc_pd cmd;
	struct ionic_pd_resp resp;
	struct ionic_pd *pd;

	pd = calloc(1, sizeof(*pd));
	if (!pd)
		return NULL;

	IONIC_LOG("");
	memset(&resp, 0, sizeof(resp));
	if (ibv_cmd_alloc_pd(ibctx, &pd->ibpd, &cmd, sizeof(cmd),
			     &resp.resp, sizeof(resp)))
		goto out;

	pd->pdid = resp.pdid;

	return &pd->ibpd;
out:
	free(pd);
	return NULL;
}

static int ionic_free_pd(struct ibv_pd *ibpd)
{
	struct ionic_pd *pd = to_ionic_pd(ibpd);
	int status;

	IONIC_LOG("");
	status = ibv_cmd_dealloc_pd(ibpd);
	if (status)
		return status;
	/* DPI un-mapping will be during uninit_ucontext */
	free(pd);

	return 0;
}

static struct ibv_mr *ionic_reg_mr(struct ibv_pd *ibpd,
				   void *sva,
				   size_t len,
				   int access)
{
	struct ionic_mr *mr;
	struct ibv_reg_mr cmd;
	struct ionic_mr_resp resp;

	IONIC_LOG("");
	mr = calloc(1, sizeof(*mr));
	if (!mr)
		return NULL;

	if (ibv_cmd_reg_mr(ibpd, sva, len, (uintptr_t)sva, access, &mr->ibmr,
			   &cmd, sizeof(cmd), &resp.resp, sizeof(resp))) {
		free(mr);
		return NULL;
	}

	return &mr->ibmr;
}

static int ionic_dereg_mr(struct ibv_mr *ibmr)
{
	struct ionic_mr *mr = (struct ionic_mr *)ibmr;
	int status;

	IONIC_LOG("");
	status = ibv_cmd_dereg_mr(ibmr);
	if (status)
		return status;
	free(mr);

	return 0;
}

static struct ibv_cq *ionic_create_cq(struct ibv_context *ibctx, int ncqe,
				      struct ibv_comp_channel *channel, int vec)
{
	struct ionic_dev *dev = to_ionic_dev(ibctx->device);
	struct ionic_ctx *ctx = to_ionic_ctx(ibctx);
	struct ionic_cq *cq;
	struct ionic_cq_req cmd;
	struct ionic_cq_resp resp;

	int rc;

	IONIC_LOG("");
	if (ncqe > dev->max_cq_depth) {
		rc = EINVAL;
		goto err;
	}

	cq = calloc(1, sizeof(*cq));
	if (!cq) {
		rc = ENOMEM;
		goto err;
	}

	pthread_spin_init(&cq->lock, PTHREAD_PROCESS_PRIVATE);

	rc = ionic_queue_init(&cq->q, dev->pg_size, ncqe, dev->cqe_size);
	if (rc)
		goto err_queue;

	ionic_queue_color_init(&cq->q);

	memset(&cmd, 0, sizeof(cmd));
	memset(&resp, 0, sizeof(resp));

	cmd.cq_va = (uintptr_t)cq->q.ptr;
	cmd.cq_bytes = cq->q.size;

	IONIC_LOG("cmd.cq_va %#lx", (unsigned long)cmd.cq_va);
	IONIC_LOG("cmd.cq_bytes %#lx", (unsigned long)cmd.cq_bytes);

	rc = ibv_cmd_create_cq(ibctx, ncqe, channel, vec,
			       &cq->ibcq, &cmd.cmd, sizeof(cmd),
			       &resp.resp, sizeof(resp));
	if (rc)
		goto err_cmd;

	cq->cqid = resp.cqid;
	ionic_queue_dbell_init(&cq->q, cq->cqid);

	/* XXX cleanup */
	cq->qtype = resp.qtype;
	/* XXX cleanup */

	pthread_mutex_lock(&ctx->mut);
	list_add_tail(&ctx->cq_list, &cq->ctx_ent);
	pthread_mutex_unlock(&ctx->mut);

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
	struct ionic_ctx *ctx = to_ionic_ctx(ibcq->context);
	struct ionic_cq *cq = to_ionic_cq(ibcq);
	int rc;

	rc = ibv_cmd_destroy_cq(ibcq);
	if (rc)
		return rc;

	pthread_mutex_lock(&ctx->mut);
	list_del(&cq->ctx_ent);
	pthread_mutex_unlock(&ctx->mut);

	ionic_queue_destroy(&cq->q);
	pthread_spin_destroy(&cq->lock);
	free(cq);

	return 0;
}

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

static int ionic_poll_recv(struct ionic_qp *qp, struct ibv_wc *wc,
			   struct cqwqe_be_t *cqe)
{
	struct ionic_rq_meta *meta;

	/* there had better be something in the recv queue to complete */
	if (ionic_queue_empty(&qp->rq))
		return -EIO;

	meta = &qp->rq_meta[qp->rq.cons];

	memset(wc, 0, sizeof(*wc));

	wc->status = ionic_to_ibv_wc_status(cqe->status);
	wc->vendor_err = cqe->status;

	wc->wr_id = meta->wrid;
	wc->qp_num = qp->qpid;

	if (wc->status != IBV_WC_SUCCESS)
		return 0;

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

	ionic_queue_consume(&qp->rq);

	return 0;
}

static int ionic_poll_send(struct ionic_qp *qp, struct ibv_wc *wc,
			   struct cqwqe_be_t *cqe)
{
	struct ionic_sq_meta *meta;

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
		return 0;

	wc->opcode = ionic_to_ibv_wc_opcd(cqe->op_type);
	wc->byte_len = meta->len; /* XXX byte_len should come from cqe */

	if (ionic_op_is_local(cqe->op_type))
		++qp->sq_local;
	else
		++qp->sq_msn;

	ionic_queue_consume(&qp->sq);

	return 0;
}

static int ionic_poll_send_ok(struct ionic_qp *qp, struct ibv_wc *wc)
{
	struct ionic_sq_meta *meta;

	/* there had better be something in the send queue to complete OK */
	if (ionic_queue_empty(&qp->sq))
		return -EIO;

	meta = &qp->sq_meta[qp->sq.cons];

	wc->status = IBV_WC_SUCCESS;
	wc->vendor_err = 0;

	wc->wr_id = meta->wrid;
	wc->qp_num = qp->qpid;
	wc->opcode = ionic_to_ibv_wc_opcd(meta->op);
	wc->byte_len = be32toh(meta->len);

	if (ionic_op_is_local(meta->op))
		++qp->sq_local;
	else
		++qp->sq_msn;

	ionic_queue_consume(&qp->sq);

	return 0;
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
		if (rc)
			break;

		++npolled;
		++wc;
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
		if (rc)
			break;

		++npolled;
		++wc;
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
			if (rc) {
				pthread_spin_unlock(&qp->sq_lock);
				goto out;
			}
			++npolled;

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

			++npolled;

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
		ionic_dbell_ring(ctx->dbpage,
				 ionic_queue_dbell_val(&cq->q),
				 IONIC_QTYPE_RDMA_COMP);
	}

	pthread_spin_unlock(&cq->lock);

	return npolled ?: rc;
}

static void ionic_cq_event(struct ibv_cq *ibcq)
{

}

static int ionic_arm_cq(struct ibv_cq *ibcq, int flags)
{
	IONIC_LOG("");
	return 0;
}

static int ionic_check_qp_limits(struct ionic_ctx *ctx,
				 struct ibv_qp_init_attr *attr)
{
	struct ibv_device_attr devattr;
	int ret;

	IONIC_LOG("");
	ret = ionic_query_device(&ctx->vctx.context, &devattr);
	if (ret)
		return ret;
	if (attr->cap.max_send_sge > devattr.max_sge)
		return EINVAL;
	if (attr->cap.max_recv_sge > devattr.max_sge)
		return EINVAL;
	if (attr->cap.max_inline_data > IONIC_MAX_INLINE_SIZE)
		return EINVAL;
	if (attr->cap.max_send_wr > devattr.max_qp_wr)
		attr->cap.max_send_wr = devattr.max_qp_wr;
	if (attr->cap.max_recv_wr > devattr.max_qp_wr)
		attr->cap.max_recv_wr = devattr.max_qp_wr;

	return 0;
}

static struct ibv_qp *ionic_create_qp(struct ibv_pd *ibpd,
				      struct ibv_qp_init_attr *attr)
{
	struct ionic_dev *dev = to_ionic_dev(ibpd->context->device);
	struct ionic_ctx *ctx = to_ionic_ctx(ibpd->context);
	struct ionic_qp *qp;
	struct ionic_qp_req req;
	struct ionic_qp_resp resp;
	struct ionic_qpcap *cap;

	int rc;

	rc = ionic_check_qp_limits(ctx, attr);
	if (rc)
		goto err_qp;

	qp = calloc(1, sizeof(*qp));
	if (!qp) {
		rc = ENOMEM;
		goto err_qp;
	}

	/* alloc queues */
	rc = ionic_queue_init(&qp->sq, dev->pg_size,
			      attr->cap.max_send_wr,
			      ionic_get_sqe_size()); /* TODO: max_sge */
	if (rc)
		goto err_sq;

	qp->sq_meta = calloc((uint32_t)qp->sq.mask + 1, sizeof(*qp->sq_meta));
	if (!qp->sq_meta) {
		rc = ENOMEM;
		goto err_sq_meta;
	}

	if (attr->srq) {
		rc = EINVAL; // srq is not supported
		goto err_rq;
	} else {
		rc = ionic_queue_init(&qp->rq, dev->pg_size,
				      attr->cap.max_recv_wr,
				      ionic_get_rqe_size()); /* TODO: max_sge */
		if (rc)
			goto err_rq;

		qp->rq_meta = calloc((uint32_t)qp->rq.mask + 1,
				     sizeof(*qp->rq_meta));
		if (!qp->rq_meta) {
			rc = ENOMEM;
			goto err_rq_meta;
		}
	}

	pthread_spin_init(&qp->sq_lock, PTHREAD_PROCESS_PRIVATE);
	pthread_spin_init(&qp->rq_lock, PTHREAD_PROCESS_PRIVATE);

	/* Fill ibv_cmd */
	cap = &qp->cap;
	req.qpsva = (uintptr_t)qp->sq.ptr;
	req.sq_bytes = qp->sq.size;
	req.qprva = (uintptr_t)qp->rq.ptr;
	req.rq_bytes = qp->rq.size;
	req.sq_wqe_size = qp->sq.stride;
	req.rq_wqe_size = qp->rq.stride;

	IONIC_LOG("req.qpsva %#lx", (unsigned long)req.qpsva);
	IONIC_LOG("req.sq_bytes %#lx", (unsigned long)req.sq_bytes);
	IONIC_LOG("req.qprva %#lx", (unsigned long)req.qprva);
	IONIC_LOG("req.rq_bytes %#lx", (unsigned long)req.rq_bytes);
	IONIC_LOG("req.sq_wqe_size %#lx", (unsigned long)req.sq_wqe_size);
	IONIC_LOG("req.rq_wqe_size %#lx", (unsigned long)req.rq_wqe_size);

	rc = ibv_cmd_create_qp(ibpd, &qp->ibqp, attr,
			       &req.cmd, sizeof(req),
			       &resp.resp, sizeof(resp));
	if (rc)
		goto err_cmd;

	qp->qpid = resp.qpid;
	ionic_queue_dbell_init(&qp->sq, qp->qpid);
	ionic_queue_dbell_init(&qp->rq, qp->qpid);

	qp->qptype = attr->qp_type;
	qp->sq_qtype = resp.sq_qtype;
	qp->rq_qtype = resp.rq_qtype;
	qp->qpst = IBV_QPS_RESET;

	/* Save/return the altered Caps. */
	cap->max_ssge = attr->cap.max_send_sge;
	cap->max_rsge = attr->cap.max_recv_sge;
	cap->max_inline = attr->cap.max_inline_data;
	cap->sqsig = attr->sq_sig_all;

	pthread_mutex_lock(&ctx->mut);
	tbl_alloc_node(&ctx->qp_tbl);
	ionic_lock_all_cqs(ctx);
	tbl_insert(&ctx->qp_tbl, qp, qp->qpid);
	ionic_unlock_all_cqs(ctx);
	pthread_mutex_unlock(&ctx->mut);

	return &qp->ibqp;

err_cmd:
	pthread_spin_destroy(&qp->rq_lock);
	pthread_spin_destroy(&qp->sq_lock);
	free(qp->rq_meta);
err_rq_meta:
	ionic_queue_destroy(&qp->rq);
err_rq:
	free(qp->sq_meta);
err_sq_meta:
	ionic_queue_destroy(&qp->sq);
err_sq:
	free(qp);
err_qp:
	errno = rc;
	return NULL;
}

static int ionic_modify_qp(struct ibv_qp *ibqp,
			   struct ibv_qp_attr *attr,
			   int attr_mask)
{
	struct ibv_modify_qp cmd = {};
	struct ionic_qp *qp = to_ionic_qp(ibqp);
	int rc;

	IONIC_LOG("");
	/* Sanity check */
	if (!attr_mask)
		return 0;

	rc = ibv_cmd_modify_qp(ibqp, attr, attr_mask, &cmd, sizeof(cmd));
	if (!rc) {
		if (attr_mask & IBV_QP_STATE) {
			qp->qpst = attr->qp_state;
			/* transition to reset */
			if (qp->qpst == IBV_QPS_RESET) {
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
	struct ionic_qp *qp = to_ionic_qp(ibqp);
	int rc;

	IONIC_LOG("");
	rc = ibv_cmd_query_qp(ibqp, attr, attr_mask, init_attr,
			      &cmd, sizeof(cmd));
	if (!rc)
		qp->qpst = ibqp->state;

	init_attr->cap.max_inline_data = IONIC_MAX_INLINE_SIZE;

	attr->cap = init_attr->cap;

	return rc;
}

static int ionic_destroy_qp(struct ibv_qp *ibqp)
{
	struct ionic_ctx *ctx = to_ionic_ctx(ibqp->context);
	struct ionic_qp *qp = to_ionic_qp(ibqp);
	int status;

	IONIC_LOG("");
	status = ibv_cmd_destroy_qp(ibqp);
	if (status)
		return status;

	pthread_mutex_lock(&ctx->mut);
	tbl_free_node(&ctx->qp_tbl);
	ionic_lock_all_cqs(ctx);
	tbl_delete(&ctx->qp_tbl, qp->qpid);
	ionic_unlock_all_cqs(ctx);
	pthread_mutex_unlock(&ctx->mut);

#if TODO
	ionic_cleanup_cq(qp, qp->rcq);
	ionic_cleanup_cq(qp, qp->scq);
#endif

	pthread_spin_destroy(&qp->rq_lock);
	pthread_spin_destroy(&qp->sq_lock);
	ionic_queue_destroy(&qp->rq);
	ionic_queue_destroy(&qp->sq);
	free(qp);

	return 0;
}

#if 0
static inline uint8_t ionic_set_hdr_flags(struct ionic_bsqe *hdr,
					  uint32_t send_flags, uint8_t sqsig)
{
	uint8_t is_inline = false;
	uint32_t hdrval = 0;

	if (send_flags & IBV_SEND_SIGNALED || sqsig)
		hdrval |= ((IONIC_WR_FLAGS_SIGNALED & IONIC_HDR_FLAGS_MASK)
			   << IONIC_HDR_FLAGS_SHIFT);
	if (send_flags & IBV_SEND_FENCE)
		/*TODO: See when RD fence can be used. */
		hdrval |= ((IONIC_WR_FLAGS_UC_FENCE & IONIC_HDR_FLAGS_MASK)
			   << IONIC_HDR_FLAGS_SHIFT);
	if (send_flags & IBV_SEND_SOLICITED)
		hdrval |= ((IONIC_WR_FLAGS_SE & IONIC_HDR_FLAGS_MASK)
			   << IONIC_HDR_FLAGS_SHIFT);

	if (send_flags & IBV_SEND_INLINE) {
		hdrval |= ((IONIC_WR_FLAGS_INLINE & IONIC_HDR_FLAGS_MASK)
			   << IONIC_HDR_FLAGS_SHIFT);
		is_inline = true;
	}
	hdr->rsv_ws_fl_wt = htole32(hdrval);

	return is_inline;
}
#endif

static int ionic_build_sge(struct sge_t *sge, struct ibv_sge *sg_list,
			   uint32_t num_sge, uint8_t is_inline) {
	int indx, length = 0;
	void *dst;

	IONIC_LOG("");
	if (!num_sge) {
		memset(sge, 0, sizeof(*sge));
		return 0;
	}

	if (is_inline) {
		dst = sge;
		for (indx = 0; indx < num_sge; indx++) {
			length += sg_list[indx].length;
			if (length > IONIC_MAX_INLINE_SIZE)
				return -ENOMEM;
			memcpy(dst, (void *)(uintptr_t)sg_list[indx].addr,
			       sg_list[indx].length);
			dst = dst + sg_list[indx].length;
		}
	} else {
		for (indx = 0; indx < num_sge; indx++) {
			sge[indx].va = htobe64(sg_list[indx].addr);
			sge[indx].lkey = htobe32(sg_list[indx].lkey);
			sge[indx].len = htobe32(sg_list[indx].length);
			length += sg_list[indx].length;
		}
	}

	return length;
}

#if TODO
static void ionic_fill_psns(struct ionic_qp *qp, struct ionic_psns *psns,
			    uint8_t opcode, uint32_t len)
{
	uint32_t pkt_cnt = 0, nxt_psn;

	memset(psns, 0, sizeof(*psns));
	if (qp->qptype == IBV_QPT_RC) {
		psns->opc_spsn = htole32(qp->sq_psn & IONIC_PSNS_SPSN_MASK);
		pkt_cnt = (len / qp->mtu);
		if (len % qp->mtu)
			pkt_cnt++;
		nxt_psn = ((qp->sq_psn + pkt_cnt) & IONIC_PSNS_NPSN_MASK);
		psns->flg_npsn = htole32(nxt_psn);
		qp->sq_psn = nxt_psn;
	}
	opcode = ionic_ibv_wr_to_wc_opcd(opcode);
	psns->opc_spsn |= htole32(((opcode & IONIC_PSNS_OPCD_MASK) <<
				   IONIC_PSNS_OPCD_SHIFT));
}
#endif

#if TODO
static void ionic_fill_wrid(struct ionic_wrid *wrid, struct ibv_send_wr *wr,
			    uint32_t len, uint8_t sqsig)
{
	wrid->wrid = wr->wr_id;
	wrid->bytes = len;
	wrid->sig = 0;
	if (wr->send_flags & IBV_SEND_SIGNALED || sqsig)
		wrid->sig = IBV_SEND_SIGNALED;
}
#endif

static int ionic_build_send_sqe(struct ionic_qp *qp,
				struct sqwqe_t *wqe,
				struct ibv_send_wr *wr,
				uint8_t is_inline)
{
	struct sge_t *sge = (struct sge_t *)&wqe->u.non_atomic.inline_data;
	int len;
	uint8_t opcode;

	IONIC_LOG("");
	len = ionic_build_sge(sge, wr->sg_list, wr->num_sge, is_inline);
	if (len < 0) {
		IONIC_LOG("build sge failed");
		return len;
	}

	/* Fill Header */
	opcode = ibv_to_ionic_wr_opcd(wr->opcode);
	if (opcode == IONIC_WR_OPCD_INVAL) {
		IONIC_LOG("invalid opcode %d", wr->opcode);
		return -EINVAL;
	}

	wqe->base.op_type = opcode & IONIC_WR_OPCODE_MASK;

	if (is_inline) {
		// TODO
		wqe->u.non_atomic.wqe.send.length = len;
	} else {
		wqe->base.num_sges = wr->num_sge;
		wqe->u.non_atomic.wqe.send.length = htobe32(len);
	}

	ionic_set_ibv_send_flags(wr->send_flags, wqe);

	wqe->base.wrid = wr->wr_id;
	return len;
}

static int ionic_build_ud_sqe(struct ionic_qp *qp,
			      struct sqwqe_t *wqe,
			      struct ibv_send_wr *wr,
			      uint8_t is_inline)
{
	struct ionic_ah *ah;
	int len;
	struct sge_t *sge = (struct sge_t *)&wqe->u.non_atomic.inline_data;
	uint8_t opcode;

	IONIC_LOG("");
	len = ionic_build_sge(sge, wr->sg_list, wr->num_sge, is_inline);
	if (len < 0)
		return len;

	/* Fill Header */
	opcode = ibv_to_ionic_wr_opcd(wr->opcode);
	if (opcode == IONIC_WR_OPCD_INVAL)
		return -EINVAL;

	wqe->base.op_type = opcode & IONIC_WR_OPCODE_MASK;

	if (is_inline) {
		// TODO
		wqe->u.non_atomic.wqe.ud_send.length = len;
	} else {
		wqe->base.num_sges = wr->num_sge;
		wqe->u.non_atomic.wqe.ud_send.length = len;
	}

	ionic_set_ibv_send_flags(wr->send_flags, wqe);

	wqe->base.wrid = wr->wr_id;

	wqe->u.non_atomic.wqe.ud_send.q_key = wr->wr.ud.remote_qkey;
	wqe->u.non_atomic.wqe.ud_send.dst_qp = wr->wr.ud.remote_qpn;

	if (!wr->wr.ud.ah) {
		return (-EINVAL);
	}

	ah = to_ionic_ah(wr->wr.ud.ah);
	wqe->u.non_atomic.wqe.ud_send.ah_handle = ah->avid;

	return len;
}

static int ionic_build_rdma_write_sqe(struct ionic_qp *qp,
				      struct sqwqe_t *wqe,
				      struct ibv_send_wr *wr,
				      uint8_t is_inline)
{
	struct sge_t *sge = (struct sge_t *)&wqe->u.non_atomic.inline_data;
	int len;
	uint8_t opcode;

	IONIC_LOG("");
	len = ionic_build_sge(sge, wr->sg_list, wr->num_sge, is_inline);
	if (len < 0)
		return len;

	/* Fill Header */
	opcode = ibv_to_ionic_wr_opcd(wr->opcode);
	if (opcode == IONIC_WR_OPCD_INVAL)
		return -EINVAL;

	wqe->base.op_type = opcode & IONIC_WR_OPCODE_MASK;

	if (is_inline) {
		// TODO
		wqe->u.non_atomic.wqe.write.length = len;
	} else {
		wqe->base.num_sges = wr->num_sge;
		wqe->u.non_atomic.wqe.write.length = len;
	}

	ionic_set_ibv_send_flags(wr->send_flags, wqe);

	wqe->base.wrid = wr->wr_id;

	wqe->u.non_atomic.wqe.write.va = wr->wr.rdma.remote_addr;
	wqe->u.non_atomic.wqe.write.r_key = wr->wr.rdma.rkey;

	return len;
}

static int ionic_build_rdma_read_sqe(struct ionic_qp *qp,
				     struct sqwqe_t *wqe,
				     struct ibv_send_wr *wr,
				     uint8_t is_inline)
{
	struct sge_t *sge = (struct sge_t *)&wqe->u.non_atomic.inline_data;
	int len;
	uint8_t opcode;

	IONIC_LOG("");
	len = ionic_build_sge(sge, wr->sg_list, wr->num_sge, is_inline);
	if (len < 0)
		return len;

	/* Fill Header */
	opcode = ibv_to_ionic_wr_opcd(wr->opcode);
	if (opcode == IONIC_WR_OPCD_INVAL) {
		IONIC_LOG("invlid opcode %d", wr->opcode);
		return -EINVAL;
	}

	wqe->base.op_type = opcode & IONIC_WR_OPCODE_MASK;

	if (is_inline) {
		// TODO
		wqe->u.non_atomic.wqe.read.length = len;
	} else {
		wqe->base.num_sges = wr->num_sge;
		wqe->u.non_atomic.wqe.read.length = len;
	}

	ionic_set_ibv_send_flags(wr->send_flags, wqe);

	wqe->base.wrid = wr->wr_id;

	wqe->u.non_atomic.wqe.read.va = wr->wr.rdma.remote_addr;
	wqe->u.non_atomic.wqe.read.r_key = wr->wr.rdma.rkey;

	return len;
}

/*
 * TODO:
 * Assumption is driver always run on host with Little Endian format (BE endian
 * driver support needs to come later) and HCA is Big Endian format. For now
 * driver is doing all the endian conversions as needed. Later we need to
 * optimize this to see capri can take WR entry data in little endian format
 * without compromising performance.
 */
static int ionic_post_send(struct ibv_qp *ibqp,
			   struct ibv_send_wr *wr,
			   struct ibv_send_wr **bad)
{
	struct ionic_ctx *ctx = to_ionic_ctx(ibqp->context);
	struct ionic_qp *qp = to_ionic_qp(ibqp);
	struct sqwqe_t *sqe;
	struct ionic_sq_meta *meta;
	int ret = 0, bytes = 0, nreq = 0;
	uint8_t is_inline = false;

	IONIC_LOG("");
	pthread_spin_lock(&qp->sq_lock);
	while (wr) {
		if ((qp->qpst != IBV_QPS_RTS) && (qp->qpst != IBV_QPS_SQD)) {
			*bad = wr;
			ret = -EINVAL;
			IONIC_LOG("Bad QP State %d", qp->qpst);
			goto out;
		}

		if ((qp->qptype == IBV_QPT_UD) &&
		    (wr->opcode != IBV_WR_SEND &&
		     wr->opcode != IBV_WR_SEND_WITH_IMM)) {
			*bad = wr;
			ret = -EINVAL;
			IONIC_LOG("Bad QP type %d opcode %d", qp->qptype, wr->opcode);
			goto out;
		}

		if (ionic_queue_full(&qp->sq) ||
		    wr->num_sge > qp->cap.max_ssge) {
			IONIC_LOG("Queue Full");
			*bad = wr;
			ret = -ENOMEM;
			goto out;
		}

		sqe = ionic_queue_at_prod(&qp->sq);
		memset(sqe, 0, qp->sq.stride);

		sqe->base.wrid = wr->wr_id;
		if (wr->send_flags & IBV_SEND_INLINE) {
			is_inline = true;
			sqe->base.inline_data_vld = 1;
		}

		switch (wr->opcode) {
		case IBV_WR_SEND_WITH_IMM:
			sqe->u.non_atomic.wqe.send.imm_data = wr->imm_data;
			SWITCH_FALLTHROUGH;
		case IBV_WR_SEND:
			if (qp->qptype == IBV_QPT_UD)
				bytes = ionic_build_ud_sqe(qp, sqe, wr,
							   is_inline);
			else
				bytes = ionic_build_send_sqe(qp, sqe, wr,
							     is_inline);
			break;
		case IBV_WR_RDMA_WRITE_WITH_IMM:
			sqe->u.non_atomic.wqe.write.imm_data = wr->imm_data;
			SWITCH_FALLTHROUGH;
		case IBV_WR_RDMA_WRITE:
			bytes = ionic_build_rdma_write_sqe(qp, sqe, wr, is_inline);
			break;
		case IBV_WR_RDMA_READ:
			bytes = ionic_build_rdma_read_sqe(qp, sqe, wr, false);
			break;

#ifdef LATER
			//TODO
		case IBV_WR_ATOMIC_CMP_AND_SWP:
			bytes = ionic_build_cns_sqe(qp, sqe, wr);
			break;
		case IBV_WR_ATOMIC_FETCH_AND_ADD:
			bytes = ionic_build_fna_sqe(qp, sqe, wr);
			break;
#endif
		default:
			bytes = -EINVAL;
			break;
		}

		if (bytes < 0) {
			ret = (bytes == -EINVAL) ? EINVAL : ENOMEM;
			*bad = wr;
			IONIC_LOG("Invalid Bytes built");
			break;
		}

		meta = &qp->sq_meta[qp->sq.prod];
		meta->wrid = wr->wr_id;
		meta->len = bytes;
		meta->op = ibv_to_ionic_wr_opcd(wr->opcode);

		ionic_dbg(ctx, "post send prod %d", qp->sq.prod);
		ionic_dbg_xdump(ctx, "wqe", sqe, qp->sq.stride);

		ionic_queue_produce(&qp->sq);
		qp->wqe_cnt++;
		nreq++;
		wr = wr->next;
	}

out:
	if (nreq) {
		ionic_dbell_ring(ctx->dbpage,
				 ionic_queue_dbell_val(&qp->sq),
				 IONIC_QTYPE_RDMA_SEND);
	}
	pthread_spin_unlock(&qp->sq_lock);
	return ret;
}

static int ionic_post_recv(struct ibv_qp *ibqp,
			   struct ibv_recv_wr *wr,
			   struct ibv_recv_wr **bad)
{
	struct ionic_ctx *ctx = to_ionic_ctx(ibqp->context);
	struct ionic_qp *qp = to_ionic_qp(ibqp);
	struct rqwqe_t *rqe;
	struct ionic_rq_meta *meta;
	int ret = 0, nreq = 0, bytes = 0;
	struct sge_t *sge;

	IONIC_LOG("");
	pthread_spin_lock(&qp->rq_lock);
	while (wr) {
		/* check QP state, abort if it is ERR or RST */
		if (qp->qpst == IBV_QPS_RESET || qp->qpst == IBV_QPS_ERR) {
			*bad = wr;
			ret = -EINVAL;
			goto out;
		}

		if (ionic_queue_full(&qp->rq) ||
		    wr->num_sge > qp->cap.max_rsge) {
			*bad = wr;
			ret = -ENOMEM;
			goto out;
		}

		rqe = ionic_queue_at_prod(&qp->rq);
		memset(rqe, 0, qp->rq.stride);

		sge = (struct sge_t *) rqe->sge_arr;
		bytes = ionic_build_sge(sge, wr->sg_list, wr->num_sge, false);

		rqe->wrid = wr->wr_id;
		rqe->num_sges = wr->num_sge;

		meta = &qp->rq_meta[qp->rq.prod];
		meta->wrid = wr->wr_id;
		meta->len = bytes; /* XXX see ionic_poll_recv, byte_len */

		ionic_dbg(ctx, "post recv prod %d", qp->rq.prod);
		ionic_dbg_xdump(ctx, "wqe", rqe, qp->rq.stride);

		ionic_queue_produce(&qp->rq);
		qp->wqe_cnt++;
		wr = wr->next;
		nreq++;
	}

out:
	if (nreq) {
		ionic_dbell_ring(ctx->dbpage,
				 ionic_queue_dbell_val(&qp->rq),
				 IONIC_QTYPE_RDMA_RECV);
	}
	pthread_spin_unlock(&qp->rq_lock);
	return ret;
}

static struct ibv_srq *ionic_create_srq(struct ibv_pd *ibpd,
					struct ibv_srq_init_attr *attr)
{
	IONIC_LOG("");
	errno = ENOSYS;
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

static const struct verbs_context_ops ionic_ctx_ops = {
	.query_device		= ionic_query_device,
	.query_port		= ionic_query_port,
	.alloc_pd		= ionic_alloc_pd,
	.dealloc_pd		= ionic_free_pd,
	.reg_mr			= ionic_reg_mr,
	.dereg_mr		= ionic_dereg_mr,
	.create_cq		= ionic_create_cq,
	.poll_cq		= ionic_poll_cq,
	.req_notify_cq		= ionic_arm_cq,
	.cq_event		= ionic_cq_event,
	.resize_cq		= ionic_resize_cq,
	.destroy_cq		= ionic_destroy_cq,
	.create_srq		= ionic_create_srq,
	.modify_srq		= ionic_modify_srq,
	.query_srq		= ionic_query_srq,
	.destroy_srq		= ionic_destroy_srq,
	.post_srq_recv		= ionic_post_srq_recv,
	.create_qp		= ionic_create_qp,
	.query_qp		= ionic_query_qp,
	.modify_qp		= ionic_modify_qp,
	.destroy_qp		= ionic_destroy_qp,
	.post_send		= ionic_post_send,
	.post_recv		= ionic_post_recv,
	.create_ah		= ionic_create_ah,
	.destroy_ah		= ionic_destroy_ah
};

/* Context Init functions */
static struct verbs_context *ionic_alloc_context(struct ibv_device *ibdev,
						 int cmd_fd)
{
	struct ionic_dev *dev = to_ionic_dev(ibdev);
	struct ionic_ctx *ctx;
	struct ibv_get_context cmd;
	struct ionic_ctx_resp resp;
	int rc;

	ctx = verbs_init_and_alloc_context(ibdev, cmd_fd, ctx, vctx);
	if (!ctx) {
		rc = errno;
		goto err_ctx;
	}

	rc = ibv_cmd_get_context(&ctx->vctx, &cmd, sizeof(cmd),
				 &resp.resp, sizeof(resp));
	if (rc)
		goto err_cmd;


	ctx->max_qp = resp.max_qp;
	/* XXX cleanup, overwrites dev fields on each init ctx */
	dev->pg_size = resp.pg_size;
	dev->cqe_size = resp.cqe_size;
	dev->max_cq_depth = resp.max_cqd;

	ctx->dbpage = ionic_map_device(dev->pg_size, cmd_fd, 0);
	if (!ctx->dbpage) {
		rc = errno;
		goto err_cmd;
	}

	pthread_mutex_init(&ctx->mut, NULL);
	tbl_init(&ctx->qp_tbl);
	list_head_init(&ctx->cq_list);

	verbs_set_ops(&ctx->vctx, &ionic_ctx_ops);

	return &ctx->vctx;

err_cmd:
	verbs_uninit_context(&ctx->vctx);
err_ctx:
	errno = rc;
	return NULL;
}

static void ionic_free_context(struct ibv_context *ibctx)
{
	struct ionic_dev *dev = to_ionic_dev(ibctx->device);
	struct ionic_ctx *ctx = to_ionic_ctx(ibctx);

	tbl_destroy(&ctx->qp_tbl);
	pthread_mutex_destroy(&ctx->mut);

	ionic_unmap(ctx->dbpage, dev->pg_size);

	verbs_uninit_context(&ctx->vctx);
	free(ctx);
}

static struct verbs_device *ionic_alloc_device(struct verbs_sysfs_dev *sysfs_dev)
{
	struct ionic_dev *dev;

	dev = calloc(1, sizeof(*dev));
	if (!dev)
		return NULL;

	return &dev->vdev;
}

static void ionic_uninit_device(struct verbs_device *vdev)
{
	struct ionic_dev *dev = to_ionic_dev(&vdev->device);

	free(dev);
}

#define PCI_VENDOR_ID_PENSANDO 0x1dd8
#define CNA(v, d) VERBS_PCI_MATCH(PCI_VENDOR_ID_##v, d, NULL)

static const struct verbs_match_ent cna_table[] = {
	CNA(PENSANDO, 0x1002), /* capri */
	{}
};

static const struct verbs_device_ops ionic_dev_ops = {
	.name			= "ionic",
	.match_min_abi_version	= IONIC_ABI_VERSION,
	.match_max_abi_version	= IONIC_ABI_VERSION,
	.match_table		= cna_table,
	.alloc_device		= ionic_alloc_device,
	.uninit_device		= ionic_uninit_device,
	.alloc_context		= ionic_alloc_context,
	.free_context		= ionic_free_context,
};
PROVIDER_DRIVER(ionic_dev_ops);
