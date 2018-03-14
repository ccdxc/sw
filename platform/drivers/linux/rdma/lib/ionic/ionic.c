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

	qp->sq_qtype = resp.sq_qtype;
	qp->rq_qtype = resp.rq_qtype;

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
			/* transition to reset */
			if (attr->qp_state == IBV_QPS_RESET) {
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

	/* XXX wqe wrid can be removed */
	wqe->base.wrid = 0;

	if (wr->send_flags & IBV_SEND_FENCE)
		wqe->base.fence = 1;

	if (wr->send_flags & IBV_SEND_SOLICITED)
		wqe->base.solicited_event = 1;

	if (wr->send_flags & IBV_SEND_SIGNALED) {
		/* TODO meta->signaled if completion should produce wc */
		wqe->base.complete_notify = 1;
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
					       qp->cap.max_inline,
					       wr->sg_list, wr->num_sge);
	} else {
		wqe->base.num_sges = wr->num_sge;
		signed_len = ionic_prep_sgl(wqe->u.non_atomic.sg_arr,
					    qp->cap.max_ssge,
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

/* XXX duplicated code.
 *
 * This can be cleaned up by making sqwqe_rc_send_t and sqwqe_ud_send_t the
 * same descriptor format, deleting this function, and using ionic_prep_send()
 * for both rc and ud.
 */
static int ionic_prep_send_duplicated_code_ud(struct ionic_qp *qp,
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
		wqe->u.non_atomic.wqe.ud_send.imm_data = wr->imm_data;
		break;
	default:
		return EINVAL;
	}

	return ionic_prep_common(qp, wr, meta, wqe,
				 &wqe->u.non_atomic.wqe.ud_send.length);
}

static int ionic_prep_send_ah(struct ionic_qp *qp,
			      struct ibv_send_wr *wr)
{
	struct ionic_ah *ah;
	struct sqwqe_t *wqe;

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
	struct ionic_ctx *ctx = to_ionic_ctx(qp->ibqp.context);
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
	struct ionic_ctx *ctx = to_ionic_ctx(qp->ibqp.context);
	int rc = 0;

	switch (wr->opcode) {
	case IBV_WR_SEND:
	case IBV_WR_SEND_WITH_IMM:
		rc = ionic_prep_send_duplicated_code_ud(qp, wr);
		if (rc)
			break;
		rc = ionic_prep_send_ah(qp, wr);
		break;
	default:
		ionic_dbg(ctx, "invalid opcode %d", wr->opcode);
		rc = EINVAL;
	}

	return rc;
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

	if (ibqp->state != IBV_QPS_RTS &&
	    ibqp->state != IBV_QPS_SQD) {
		ionic_dbg(ctx, "invalid qp state %d", ibqp->state);
		rc = EINVAL;
		goto out;
	}

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
		ionic_dbell_ring(ctx->dbpage,
				 ionic_queue_dbell_val(&qp->sq),
				 IONIC_QTYPE_RDMA_SEND);
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

	signed_len = ionic_prep_sgl(wqe->sge_arr, qp->cap.max_rsge,
				    wr->sg_list, wr->num_sge);
	if (signed_len < 0)
		return (int)-signed_len;

	wqe->wrid = 0; /* XXX tag for SRQ? */
	wqe->num_sges = wr->num_sge;

	meta->wrid = wr->wr_id;
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

	if (ibqp->state == IBV_QPS_RESET ||
	    ibqp->state == IBV_QPS_ERR) {
		ionic_dbg(ctx, "invalid qp state %d", ibqp->state);
		rc = EINVAL;
		goto out;
	}

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
		ionic_dbell_ring(ctx->dbpage,
				 ionic_queue_dbell_val(&qp->rq),
				 IONIC_QTYPE_RDMA_RECV);
	}

	pthread_spin_unlock(&qp->rq_lock);

	*bad = wr;
	return rc;
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
