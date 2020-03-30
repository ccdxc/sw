// SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB
/*
 * Copyright (c) 2018-2020 Pensando Systems, Inc.  All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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

#define ionic_spin_lock(ctx, lock) do {			\
	if (!(ctx)->lockfree)				\
		pthread_spin_lock(lock);		\
} while (0)

#define ionic_spin_trylock(ctx, lock)			\
	((ctx)->lockfree ?				\
	 0 : pthread_spin_trylock(lock))

#define ionic_spin_unlock(ctx, lock) do {		\
	if (!(ctx)->lockfree)				\
		pthread_spin_unlock(lock);		\
} while (0)

static int ionic_query_device(struct ibv_context *ibctx,
			      struct ibv_device_attr *dev_attr)
{
	struct ibv_query_device req = {};
	uint64_t fw_ver;
	int rc;

	rc = ibv_cmd_query_device(ibctx, dev_attr, &fw_ver,
				  &req, sizeof(req));
	if (rc)
		return rc;

	rc = ibv_read_sysfs_file(ibctx->device->ibdev_path, "fw_ver",
				 dev_attr->fw_ver, sizeof(dev_attr->fw_ver));
	if (rc < 0)
		dev_attr->fw_ver[0] = 0;

	return 0;
}

static int ionic_query_port(struct ibv_context *ibctx, uint8_t port,
			    struct ibv_port_attr *port_attr)
{
	struct ibv_query_port req = {};

	return ibv_cmd_query_port(ibctx, port, port_attr,
				  &req, sizeof(req));
}

static struct ibv_pd *ionic_alloc_pd(struct ibv_context *ibctx)
{
	struct ibv_pd *ibpd;
	struct ibv_alloc_pd req = {};
	struct ibv_alloc_pd_resp resp = {};
	int rc;

	ibpd = calloc(1, sizeof(*ibpd));
	if (!ibpd) {
		rc = errno;
		goto err_pd;
	}

	rc = ibv_cmd_alloc_pd(ibctx, ibpd,
			      &req, sizeof(req),
			      &resp, sizeof(resp));
	if (rc)
		goto err_cmd;

	return ibpd;

err_cmd:
	free(ibpd);
err_pd:
	errno = rc;
	return NULL;
}

static int ionic_dealloc_pd(struct ibv_pd *ibpd)
{
	int rc;

	rc = ibv_cmd_dealloc_pd(ibpd);
	if (rc)
		return rc;

	free(ibpd);

	return 0;
}

static struct ibv_mr *ionic_reg_mr(struct ibv_pd *ibpd,
				   void *addr,
				   size_t len,
				   int access)
{
	struct ibv_mr *vmr;
	struct ibv_reg_mr_resp resp = {};
	struct ibv_reg_mr req = {};
	int rc;

	vmr = calloc(1, sizeof(*vmr));
	if (!vmr) {
		rc = errno;
		goto err_mr;
	}

	rc = ibv_cmd_reg_mr(ibpd, addr, len,
			    (uintptr_t)addr,
			    access, vmr,
			    &req, sizeof(req),
			    &resp, sizeof(resp));
	if (rc)
		goto err_cmd;

	return vmr;

err_cmd:
	free(vmr);
err_mr:
	errno = rc;
	return NULL;
}

static int ionic_rereg_mr(struct ibv_mr *vmr, int flags, struct ibv_pd *pd,
			  void *addr, size_t length, int access)
{
	struct ibv_rereg_mr cmd;
	struct ibv_rereg_mr_resp resp;

	if (flags & IBV_REREG_MR_KEEP_VALID)
		return EOPNOTSUPP;

	return ibv_cmd_rereg_mr(vmr, flags, addr, length,
				(uintptr_t)addr, access, pd,
				&cmd, sizeof(cmd),
				&resp, sizeof(resp));
}

static int ionic_dereg_mr(struct ibv_mr *vmr)
{
	int rc;

	rc = ibv_cmd_dereg_mr(vmr);
	if (rc)
		return rc;

	free(vmr);

	return 0;
}

static struct ibv_cq *ionic_create_cq(struct ibv_context *ibctx, int ncqe,
				      struct ibv_comp_channel *channel,
				      int vec)
{
	struct ionic_ctx *ctx = to_ionic_ctx(ibctx);
	struct ionic_cq *cq;
	struct uionic_cq req = {};
	struct uionic_cq_resp resp = {};

	int rc;

	if (ncqe < 1 || ncqe + IONIC_CQ_GRACE > 0xffff) {
		rc = EINVAL;
		goto err;
	}

	cq = calloc(1, sizeof(*cq));
	if (!cq) {
		rc = ENOMEM;
		goto err;
	}

	pthread_spin_init(&cq->lock, PTHREAD_PROCESS_PRIVATE);
	list_head_init(&cq->poll_sq);
	list_head_init(&cq->flush_sq);
	list_head_init(&cq->flush_rq);

	rc = ionic_queue_init(&cq->q, ctx->pg_shift,
			      ncqe + IONIC_CQ_GRACE,
			      sizeof(struct ionic_v1_cqe));
	if (rc)
		goto err_queue;

	cq->color = true;
	cq->reserve = cq->q.mask;

	req.cq.addr = (uintptr_t)cq->q.ptr;
	req.cq.size = cq->q.size;
	req.cq.mask = cq->q.mask;
	req.cq.depth_log2 = cq->q.depth_log2;
	req.cq.stride_log2 = cq->q.stride_log2;

	rc = ibv_cmd_create_cq(ibctx, ncqe, channel, vec, &cq->ibcq,
			       &req.ibv_cmd, sizeof(req),
			       &resp.ibv_resp, sizeof(resp));
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

static int ionic_flush_recv(struct ionic_qp *qp, struct ibv_wc *wc)
{
	struct ionic_v1_wqe *wqe;
	struct ionic_rq_meta *meta;

	if (!qp->rq_flush)
		return 0;

	if (ionic_queue_empty(&qp->rq))
		return 0;

	wqe = ionic_queue_at_cons(&qp->rq);

	/* wqe_id must be a valid queue index */
	if (unlikely(wqe->base.wqe_id >> qp->rq.depth_log2))
	{
		ionic_err("invalid id %#lx",
			  (unsigned long)wqe->base.wqe_id);
		return -EIO;
	}

	/* wqe_id must indicate a request that is outstanding */
	meta = &qp->rq_meta[wqe->base.wqe_id];
	if (unlikely(meta->next != IONIC_META_POSTED))
	{
		ionic_err("wqe not posted %#lx",
			  (unsigned long)wqe->base.wqe_id);
		return -EIO;
	}

	ionic_queue_consume(&qp->rq);

	memset(wc, 0, sizeof(*wc));

	wc->status = IBV_WC_WR_FLUSH_ERR;
	wc->wr_id = meta->wrid;
	wc->qp_num = qp->qpid;

	meta->next = qp->rq_meta_head;
	qp->rq_meta_head = meta;

	return 1;
}

static int ionic_flush_recv_many(struct ionic_qp *qp,
				 struct ibv_wc *wc, int nwc)
{
	int rc = 0, npolled = 0;

	while (npolled < nwc) {
		rc = ionic_flush_recv(qp, wc + npolled);
		if (rc <= 0)
			break;

		npolled += rc;
	}

	return npolled ?: rc;
}

static int ionic_flush_send(struct ionic_qp *qp, struct ibv_wc *wc)
{
	struct ionic_sq_meta *meta;

	if (!qp->sq_flush)
		return 0;

	if (ionic_queue_empty(&qp->sq))
		return 0;

	meta = &qp->sq_meta[qp->sq.cons];

	ionic_queue_consume(&qp->sq);

	memset(wc, 0, sizeof(*wc));

	wc->status = IBV_WC_WR_FLUSH_ERR;
	wc->wr_id = meta->wrid;
	wc->qp_num = qp->qpid;

	return 1;
}

static int ionic_flush_send_many(struct ionic_qp *qp,
				 struct ibv_wc *wc, int nwc)
{
	int rc = 0, npolled = 0;

	while (npolled < nwc) {
		rc = ionic_flush_send(qp, wc + npolled);
		if (rc <= 0)
			break;

		npolled += rc;
	}

	return npolled ?: rc;
}

static int ionic_poll_recv(struct ionic_ctx *ctx, struct ionic_cq *cq,
			   struct ionic_qp *cqe_qp, struct ionic_v1_cqe *cqe,
			   struct ibv_wc *wc)
{
	struct ionic_qp *qp = NULL;
	struct ionic_rq_meta *meta;
	uint32_t src_qpn, st_len;
	uint16_t vlan_tag;
	uint8_t op;

	if (cqe_qp->rq_flush)
		return 0;

	qp = cqe_qp;

	st_len = be32toh(cqe->status_length);

	/* ignore wqe_id in case of flush error */
	if (ionic_v1_cqe_error(cqe) && st_len == IONIC_STS_WQE_FLUSHED_ERR) {
		cqe_qp->rq_flush = true;
		cq->flush = true;
		list_del(&cqe_qp->cq_flush_rq);
		list_add_tail(&cq->flush_rq, &cqe_qp->cq_flush_rq);

		ionic_stat_incr(ctx->stats, poll_cq_wc_err);

		/* posted recvs (if any) flushed by ionic_flush_recv */
		return 0;
	}

	/* there had better be something in the recv queue to complete */
	if (ionic_queue_empty(&qp->rq))
	{
		ionic_err("rq is empty %u", qp->qpid);
		return -EIO;
	}

	/* wqe_id must be a valid queue index */
	if (unlikely(cqe->recv.wqe_id >> qp->rq.depth_log2))
	{
		ionic_err("invalid id %#lx",
			  (unsigned long)cqe->recv.wqe_id);
		return -EIO;
	}

	/* wqe_id must indicate a request that is outstanding */
	meta = &qp->rq_meta[cqe->recv.wqe_id];
	if (unlikely(meta->next != IONIC_META_POSTED))
	{
		ionic_err("wqe is not posted %#lx",
			  (unsigned long)cqe->recv.wqe_id);
		return -EIO;
	}

	meta->next = qp->rq_meta_head;
	qp->rq_meta_head = meta;

	memset(wc, 0, sizeof(*wc));

	wc->wr_id = meta->wrid;
	wc->qp_num = cqe_qp->qpid;

	if (ionic_v1_cqe_error(cqe)) {
		wc->vendor_err = st_len;
		wc->status = ionic_to_ibv_status(st_len);

		cqe_qp->rq_flush = true;
		cq->flush = true;
		list_del(&cqe_qp->cq_flush_rq);
		list_add_tail(&cq->flush_rq, &cqe_qp->cq_flush_rq);

		ionic_stat_incr(ctx->stats, poll_cq_wc_err);

		goto out;
	}

	wc->vendor_err = 0;
	wc->status = IBV_WC_SUCCESS;

	src_qpn = be32toh(cqe->recv.src_qpn_op);
	op = src_qpn >> IONIC_V1_CQE_RECV_OP_SHIFT;

	src_qpn &= IONIC_V1_CQE_RECV_QPN_MASK;
	op &= IONIC_V1_CQE_RECV_OP_MASK;

	wc->opcode = IBV_WC_RECV;
	switch (op) {
	case IONIC_V1_CQE_RECV_OP_RDMA_IMM:
		wc->opcode = IBV_WC_RECV_RDMA_WITH_IMM;
		/* fallthrough */
	case IONIC_V1_CQE_RECV_OP_SEND_IMM:
		wc->wc_flags |= IBV_WC_WITH_IMM;
		wc->imm_data = cqe->recv.imm_data_rkey; /* be32 in wc */
		break;
	case IONIC_V1_CQE_RECV_OP_SEND_INV:
		wc->wc_flags |= IBV_WC_WITH_INV;
		wc->invalidated_rkey = be32toh(cqe->recv.imm_data_rkey);
	}

	wc->byte_len = st_len;
	wc->src_qp = src_qpn;

	if (qp->vqp.qp.qp_type == IBV_QPT_UD) {
		wc->wc_flags |= IBV_WC_GRH;

		/* vlan_tag in cqe will be valid from dpath even if no vlan */
		vlan_tag = be16toh(cqe->recv.vlan_tag);
		wc->sl = vlan_tag >> 13; /* 802.1q PCP */
	}

	wc->pkey_index = 0;

out:
	ionic_dbg(ctx, "poll cq %u qp %u cons %u st %u wrid %#lx op %u len %u",
		  cq->cqid, qp->qpid, qp->rq.cons,
		  wc->status, meta->wrid, wc->opcode, st_len);
	ionic_queue_consume(&qp->rq);

	return 1;
}

static bool ionic_peek_send(struct ionic_qp *qp)
{
	struct ionic_sq_meta *meta;

	if (qp->sq_flush)
		return 0;

	/* completed all send queue requests? */
	if (ionic_queue_empty(&qp->sq))
		return false;

	meta = &qp->sq_meta[qp->sq.cons];

	/* waiting for remote completion? */
	if (meta->remote && meta->seq == qp->sq_msn_cons)
		return false;

	/* waiting for local completion? */
	if (!meta->remote && !meta->local_comp)
		return false;

	return true;
}

static int ionic_poll_send(struct ionic_cq *cq, struct ionic_qp *qp,
			   struct ibv_wc *wc)
{
	struct ionic_ctx *ctx = to_ionic_ctx(cq->ibcq.context);
	struct ionic_sq_meta *meta;

	if (qp->sq_flush)
		return 0;

	do {
		/* completed all send queue requests? */
		if (ionic_queue_empty(&qp->sq))
			goto out_empty;

		meta = &qp->sq_meta[qp->sq.cons];

		/* waiting for remote completion? */
		if (meta->remote && meta->seq == qp->sq_msn_cons)
			goto out_empty;

		/* waiting for local completion? */
		if (!meta->remote && !meta->local_comp)
			goto out_empty;

		ionic_dbg(ctx,
			  "poll cq %u qp %u cons %u st %u wr %#lx op %u l %u",
			  cq->cqid, qp->qpid, qp->sq.cons,
			  meta->ibsts, meta->wrid, meta->ibop, meta->len);
		ionic_queue_consume(&qp->sq);

		/* produce wc only if signaled or error status */
	} while (!meta->signal && meta->ibsts == IBV_WC_SUCCESS);

	memset(wc, 0, sizeof(*wc));

	wc->status = meta->ibsts;
	wc->wr_id = meta->wrid;
	wc->qp_num = qp->qpid;

	if (meta->ibsts == IBV_WC_SUCCESS) {
		wc->byte_len = meta->len;
		wc->opcode = meta->ibop;
	} else {
		wc->vendor_err = meta->len;

		qp->sq_flush = true;
		cq->flush = true;
		list_del(&qp->cq_flush_sq);
		list_add_tail(&cq->flush_sq, &qp->cq_flush_sq);

		ionic_stat_incr(ctx->stats, poll_cq_wc_err);
	}

	return 1;

out_empty:
	if (qp->sq_flush_rcvd) {
		qp->sq_flush = true;
		cq->flush = true;
		list_del(&qp->cq_flush_sq);
		list_add_tail(&cq->flush_sq, &qp->cq_flush_sq);
	}
	return 0;
}

static int ionic_poll_send_many(struct ionic_cq *cq, struct ionic_qp *qp,
				struct ibv_wc *wc, int nwc)
{
	int rc = 0, npolled = 0;

	while (npolled < nwc) {
		rc = ionic_poll_send(cq, qp, wc + npolled);
		if (rc <= 0)
			break;

		npolled += rc;
	}

	return npolled ?: rc;
}

static int ionic_validate_cons(uint16_t prod, uint16_t cons,
			       uint16_t comp, uint16_t mask)
{
	if (((prod - cons) & mask) < ((comp - cons) & mask))
		return -EIO;

	return 0;
}

static int ionic_comp_msn(struct ionic_qp *qp, struct ionic_v1_cqe *cqe)
{
	struct ionic_sq_meta *meta;
	uint16_t cqe_seq, cqe_idx;
	int rc;

	if (qp->sq_flush)
		return 0;

	cqe_seq = be32toh(cqe->send.msg_msn) & qp->sq.mask;

	if (ionic_v1_cqe_error(cqe))
		cqe_idx = qp->sq_msn_idx[cqe_seq];
	else
		cqe_idx = qp->sq_msn_idx[(cqe_seq - 1) & qp->sq.mask];

	rc = ionic_validate_cons(qp->sq_msn_prod,
				 qp->sq_msn_cons,
				 cqe_seq, qp->sq.mask);
	if (rc)
	{
		ionic_err("wqe is not posted %#x (msn)",
			  be32toh(cqe->send.msg_msn));
		return rc;
	}

	qp->sq_msn_cons = cqe_seq;

	if (ionic_v1_cqe_error(cqe)) {
		meta = &qp->sq_meta[cqe_idx];
		meta->len = be32toh(cqe->status_length);
		meta->ibsts = ionic_to_ibv_status(meta->len);
		meta->remote = false;
	}

	return 0;
}

static int ionic_comp_npg(struct ionic_qp *qp, struct ionic_v1_cqe *cqe)
{
	struct ionic_sq_meta *meta;
	uint16_t cqe_idx;
	uint32_t st_len;

	if (qp->sq_flush)
		return 0;

	st_len = be32toh(cqe->status_length);

	if (ionic_v1_cqe_error(cqe) && st_len == IONIC_STS_WQE_FLUSHED_ERR) {
		/* Flush cqe does not consume a wqe on the device, and maybe
		 * no such work request is posted.
		 *
		 * The driver should begin flushing after the last indicated
		 * normal or error completion.  Here, only set a hint that the
		 * flush request was indicated.  In poll_send, if nothing more
		 * can be polled normally, then begin flushing.
		 */
		qp->sq_flush_rcvd = true;
		return 0;
	}

	cqe_idx = cqe->send.npg_wqe_id & qp->sq.mask;
	meta = &qp->sq_meta[cqe_idx];
	meta->local_comp = true;

	if (ionic_v1_cqe_error(cqe)) {
		meta->len = st_len;
		meta->ibsts = ionic_to_ibv_status(st_len);
		meta->remote = false;
	}

	return 0;
}

static bool ionic_next_cqe(struct ionic_cq *cq, struct ionic_v1_cqe **cqe)
{
	struct ionic_ctx *ctx = to_ionic_ctx(cq->ibcq.context);
	struct ionic_v1_cqe *qcqe = ionic_queue_at_prod(&cq->q);

	if (unlikely(cq->color != ionic_v1_cqe_color(qcqe)))
		return false;

	ionic_lat_enable(ctx->lats, true);

	udma_from_device_barrier();

	ionic_dbg_xdump(ctx, "cqe", qcqe, 1u << cq->q.stride_log2);
	*cqe = qcqe;

	return true;
}

static void ionic_clean_cq(struct ionic_cq *cq, uint32_t qpid)
{
	struct ionic_v1_cqe *qcqe;
	int prod, qtf, qid;
	bool color;

	color = cq->color;
	prod = cq->q.prod;
	qcqe = ionic_queue_at(&cq->q, prod);

	while (color == ionic_v1_cqe_color(qcqe)) {
		qtf = ionic_v1_cqe_qtf(qcqe);
		qid = ionic_v1_cqe_qtf_qid(qtf);

		if (qid == qpid)
			ionic_v1_cqe_clean(qcqe);

		prod = ionic_queue_next(&cq->q, prod);
		qcqe = ionic_queue_at(&cq->q, prod);
		color = ionic_color_wrap(prod, color);
	}
}

static void ionic_reserve_sync_cq(struct ionic_ctx *ctx, struct ionic_cq *cq)
{
	if (!ionic_queue_empty(&cq->q)) {
		cq->reserve += ionic_queue_length(&cq->q);
		cq->q.cons = cq->q.prod;

		ionic_dbg(ctx, "dbell cq %u val %#lx rsv %d",
			  cq->cqid, ionic_queue_dbell_val(&cq->q),
			  cq->reserve);
		ionic_dbell_ring(&ctx->dbpage[ctx->cq_qtype],
				 ionic_queue_dbell_val(&cq->q));

		ionic_stat_incr(ctx->stats, ring_cq_dbell);
	}
}

static void ionic_arm_cq(struct ionic_ctx *ctx, struct ionic_cq *cq)
{
	uint64_t dbell_val = cq->q.dbell;

	if (cq->deferred_arm_sol_only) {
		cq->arm_sol_prod = ionic_queue_next(&cq->q, cq->arm_sol_prod);
		dbell_val |= cq->arm_sol_prod | IONIC_DBELL_RING_SONLY;
		ionic_stat_incr(ctx->stats, arm_cq_sol);
	} else {
		cq->arm_any_prod = ionic_queue_next(&cq->q, cq->arm_any_prod);
		dbell_val |= cq->arm_any_prod | IONIC_DBELL_RING_ARM;
		ionic_stat_incr(ctx->stats, arm_cq_any);
	}

	ionic_dbg(ctx, "dbell cq %u val %#lx (%s)", cq->cqid, dbell_val,
		  cq->deferred_arm_sol_only ? "sonly" : "arm");
	ionic_dbell_ring(&ctx->dbpage[ctx->cq_qtype], dbell_val);
}

static void ionic_reserve_cq(struct ionic_ctx *ctx, struct ionic_cq *cq,
			     int spend)
{
	cq->reserve -= spend;

	if (cq->reserve <= 0 || cq->deferred_arm)
		ionic_reserve_sync_cq(ctx, cq);

	if (cq->deferred_arm) {
		ionic_arm_cq(ctx, cq);
		cq->deferred_arm = false;
	}
}

/*
 * Note about rc: (noted here because poll is different)
 *
 * Functions without "poll" in the name, if they return an integer, return
 * zero on success, or a positive error number.  Functions returning a
 * pointer return NULL on error and set errno to a positive error number.
 *
 * Functions with "poll" in the name return negative error numbers, or
 * greater or equal to zero number of completions on success.
 */
static int ionic_poll_cq(struct ibv_cq *ibcq, int nwc, struct ibv_wc *wc)
{
	struct ionic_ctx *ctx = to_ionic_ctx(ibcq->context);
	struct ionic_cq *cq = to_ionic_cq(ibcq);
	struct ionic_qp *qp, *qp_next;
	struct ionic_v1_cqe *cqe;
	uint32_t qtf, qid;
	uint8_t type;
	bool peek;
	int rc = 0, npolled = 0;
	uint16_t old_prod;

	ionic_lat_trace(ctx->lats, application);
	ionic_stat_incr(ctx->stats, poll_cq);

	if (nwc < 1)
		return 0;

	ionic_spin_lock(ctx, &cq->lock);

	old_prod = cq->q.prod;

	/* poll already indicated work completions for send queue */

	list_for_each_safe(&cq->poll_sq, qp, qp_next, cq_poll_sq) {
		if (npolled == nwc)
			goto out;

		ionic_spin_lock(ctx, &qp->sq_lock);
		rc = ionic_poll_send_many(cq, qp, wc + npolled, nwc - npolled);
		ionic_spin_unlock(ctx, &qp->sq_lock);

		if (rc > 0)
			npolled += rc;

		if (npolled < nwc)
			list_del_init(&qp->cq_poll_sq);
	}

	/* poll for more work completions */

	while (likely(ionic_next_cqe(cq, &cqe))) {
		if (npolled == nwc)
			goto out;

		qtf = ionic_v1_cqe_qtf(cqe);
		qid = ionic_v1_cqe_qtf_qid(qtf);
		type = ionic_v1_cqe_qtf_type(qtf);

		qp = ionic_tbl_lookup(&ctx->qp_tbl, qid);

		if (unlikely(!qp))
		{
			ionic_dbg(ctx, "missing qp for qid %#x", qid);
			goto cq_next;
		}

		switch (type) {
		case IONIC_V1_CQE_TYPE_RECV:
			ionic_spin_lock(ctx, &qp->rq_lock);
			rc = ionic_poll_recv(ctx, cq, qp, cqe, wc + npolled);
			ionic_spin_unlock(ctx, &qp->rq_lock);

			if (rc < 0)
				goto out;

			npolled += rc;
			break;

		case IONIC_V1_CQE_TYPE_SEND_MSN:
			ionic_spin_lock(ctx, &qp->sq_lock);
			rc = ionic_comp_msn(qp, cqe);
			if (!rc) {
				rc = ionic_poll_send_many(cq, qp,
							  wc + npolled,
							  nwc - npolled);
				peek = ionic_peek_send(qp);
			}
			ionic_spin_unlock(ctx, &qp->sq_lock);

			if (rc < 0)
				goto out;

			npolled += rc;

			if (peek) {
				list_del(&qp->cq_poll_sq);
				list_add_tail(&cq->poll_sq, &qp->cq_poll_sq);
			}

			break;

		case IONIC_V1_CQE_TYPE_SEND_NPG:
			ionic_spin_lock(ctx, &qp->sq_lock);
			rc = ionic_comp_npg(qp, cqe);
			if (!rc) {
				rc = ionic_poll_send_many(cq, qp,
							  wc + npolled,
							  nwc - npolled);
				peek = ionic_peek_send(qp);
			}
			ionic_spin_unlock(ctx, &qp->sq_lock);

			if (rc < 0)
				goto out;

			npolled += rc;

			if (peek) {
				list_del(&qp->cq_poll_sq);
				list_add_tail(&cq->poll_sq, &qp->cq_poll_sq);
			}

			break;

		default:
			ionic_err("unexpected cqe type %u", type);
			rc = -EIO;
			goto out;
		}

cq_next:
		ionic_queue_produce(&cq->q);
		cq->color = ionic_color_wrap(cq->q.prod, cq->color);
	}

	/* lastly, flush send and recv queues */

	if (likely(!cq->flush))
		goto out;

	cq->flush = false;

	list_for_each_safe(&cq->flush_sq, qp, qp_next, cq_flush_sq) {
		if (npolled == nwc)
			goto out;

		ionic_spin_lock(ctx, &qp->sq_lock);
		rc = ionic_flush_send_many(qp, wc + npolled, nwc - npolled);
		ionic_stat_add(ctx->stats, poll_cq_wc_flush, rc);
		ionic_spin_unlock(ctx, &qp->sq_lock);

		if (rc > 0)
			npolled += rc;

		if (npolled < nwc)
			list_del_init(&qp->cq_flush_sq);
		else
			cq->flush = true;
	}

	list_for_each_safe(&cq->flush_rq, qp, qp_next, cq_flush_rq) {
		if (npolled == nwc)
			goto out;

		ionic_spin_lock(ctx, &qp->rq_lock);
		rc = ionic_flush_recv_many(qp, wc + npolled, nwc - npolled);
		ionic_stat_add(ctx->stats, poll_cq_wc_flush, rc);
		ionic_spin_unlock(ctx, &qp->rq_lock);

		if (rc > 0)
			npolled += rc;

		if (npolled < nwc)
			list_del_init(&qp->cq_flush_rq);
		else
			cq->flush = true;
	}

out:
	ionic_reserve_cq(ctx, cq, 0);

	old_prod = (cq->q.prod - old_prod) & cq->q.mask;

	ionic_stat_add(ctx->stats, poll_cq_cqe, old_prod);
	ionic_stat_incr_idx_fls(ctx->stats, poll_cq_ncqe, old_prod);
	ionic_stat_add(ctx->stats, poll_cq_wc, npolled);
	ionic_stat_incr_idx_fls(ctx->stats, poll_cq_nwc, npolled);
	ionic_stat_add(ctx->stats, poll_cq_err, (npolled ?: rc) < 0);

	ionic_spin_unlock(ctx, &cq->lock);

	if (npolled) {
		ionic_lat_trace(ctx->lats, poll_cq_compl);
	} else {
		ionic_lat_trace(ctx->lats, poll_cq_empty);
		ionic_lat_enable(ctx->lats, false);
	}

	return npolled ?: rc;
}

static int ionic_req_notify_cq(struct ibv_cq *ibcq, int solicited_only)
{
	struct ionic_cq *cq = to_ionic_cq(ibcq);
	struct ionic_ctx *ctx = to_ionic_ctx(ibcq->context);

	ionic_spin_lock(ctx, &cq->lock);

	cq->deferred_arm = true;
	cq->deferred_arm_sol_only = (bool)solicited_only;

	ionic_spin_unlock(ctx, &cq->lock);

	return 0;
}

static int ionic_qp_sq_init(struct ionic_ctx *ctx, struct ionic_qp *qp,
			    int max_wr, int max_sge, int max_data)
{
	uint32_t wqe_size;
	int rc;

	if (!qp->has_sq)
		return 0;

	if (max_wr < 0 || max_wr > 0xffff)
		return EINVAL;
	if (max_sge < 0 ||
	    max_sge > ionic_v1_send_wqe_max_sge(ctx->max_stride, 0))
		return EINVAL;
	if (max_data < 0 ||
	    max_data > ionic_v1_send_wqe_max_data(ctx->max_stride))
		return EINVAL;

	qp->sq_spec = ionic_v1_use_spec_sge(max_sge, ctx->spec);

	wqe_size = ionic_v1_send_wqe_min_size(max_sge, max_data, qp->sq_spec);

	rc = ionic_queue_init(&qp->sq, ctx->pg_shift, max_wr, wqe_size);
	if (rc)
		goto err_sq;

	qp->sq_cmb_ptr = NULL;
	qp->sq_cmb_prod = 0;

	qp->sq_meta = calloc((uint32_t)qp->sq.mask + 1,
			     sizeof(*qp->sq_meta));
	if (!qp->sq_meta) {
		rc = ENOMEM;
		goto err_sq_meta;
	}

	qp->sq_msn_idx = calloc((uint32_t)qp->sq.mask + 1,
				sizeof(*qp->sq_msn_idx));
	if (!qp->sq_msn_idx) {
		rc = ENOMEM;
		goto err_sq_msn;
	}

	return 0;

err_sq_msn:
	free(qp->sq_meta);
err_sq_meta:
	ionic_queue_destroy(&qp->sq);
err_sq:
	return rc;
}

static void ionic_qp_sq_destroy(struct ionic_qp *qp)
{
	if (!qp->has_sq)
		return;

	free(qp->sq_msn_idx);
	free(qp->sq_meta);
	ionic_queue_destroy(&qp->sq);
}

static int ionic_qp_rq_init(struct ionic_ctx *ctx, struct ionic_qp *qp,
			    int max_wr, int max_sge)
{
	uint32_t wqe_size;
	int rc, i;

	if (!qp->has_sq)
		return 0;

	if (max_wr < 0 || max_wr > 0xffff)
		return EINVAL;
	if (max_sge < 0 ||
	    max_sge > ionic_v1_recv_wqe_max_sge(ctx->max_stride, 0))
		return EINVAL;

	qp->rq_spec = ionic_v1_use_spec_sge(max_sge, ctx->spec);

	wqe_size = ionic_v1_recv_wqe_min_size(max_sge, qp->rq_spec);

	rc = ionic_queue_init(&qp->rq, ctx->pg_shift, max_wr, wqe_size);
	if (rc)
		goto err_rq;

	qp->rq_cmb_ptr = NULL;
	qp->rq_cmb_prod = 0;

	qp->rq_meta = calloc((uint32_t)qp->rq.mask + 1,
			     sizeof(*qp->rq_meta));
	if (!qp->rq_meta) {
		rc = ENOMEM;
		goto err_rq_meta;
	}

	for (i = 0; i < qp->rq.mask; ++i)
		qp->rq_meta[i].next = &qp->rq_meta[i + 1];
	qp->rq_meta[i].next = IONIC_META_LAST;
	qp->rq_meta_head = &qp->rq_meta[0];

	return 0;

err_rq_meta:
	ionic_queue_destroy(&qp->rq);
err_rq:
	return rc;
}

static void ionic_qp_rq_destroy(struct ionic_qp *qp)
{
	if (!qp->has_rq)
		return;

	free(qp->rq_meta);
	ionic_queue_destroy(&qp->rq);
}

static struct ibv_qp *ionic_create_qp_ex(struct ibv_context *ibctx,
					 struct ibv_qp_init_attr_ex *ex)
{
	struct ionic_ctx *ctx = to_ionic_ctx(ibctx);
	struct ionic_qp *qp;
	struct ionic_cq *cq;
	struct uionic_qp req;
	struct uionic_qp_resp resp;
	int rc;

	qp = calloc(1, sizeof(*qp));
	if (!qp) {
		rc = ENOMEM;
		goto err_qp;
	}

	qp->vqp.qp.qp_type = ex->qp_type;

	qp->has_sq = true;
	qp->has_rq = true;

	qp->sig_all = ex->sq_sig_all;

	list_node_init(&qp->cq_poll_sq);
	list_node_init(&qp->cq_flush_sq);
	list_node_init(&qp->cq_flush_rq);

	pthread_spin_init(&qp->sq_lock, PTHREAD_PROCESS_PRIVATE);
	pthread_spin_init(&qp->rq_lock, PTHREAD_PROCESS_PRIVATE);

	rc = ionic_qp_sq_init(ctx, qp, ex->cap.max_send_wr,
			      ex->cap.max_send_sge, ex->cap.max_inline_data);
	if (rc)
		goto err_sq;

	rc = ionic_qp_rq_init(ctx, qp, ex->cap.max_recv_wr,
			      ex->cap.max_recv_sge);
	if (rc)
		goto err_rq;

	req.sq.addr = (uintptr_t)qp->sq.ptr;
	req.sq.size = qp->sq.size;
	req.sq.mask = qp->sq.mask;
	req.sq.depth_log2 = qp->sq.depth_log2;
	req.sq.stride_log2 = qp->sq.stride_log2;

	req.rq.addr = (uintptr_t)qp->rq.ptr;
	req.rq.size = qp->rq.size;
	req.rq.mask = qp->rq.mask;
	req.rq.depth_log2 = qp->rq.depth_log2;
	req.rq.stride_log2 = qp->rq.stride_log2;

	req.sq_spec = qp->sq_spec;
	req.rq_spec = qp->rq_spec;

	rc = ibv_cmd_create_qp_ex2(ibctx, &qp->vqp, sizeof(qp->vqp), ex,
				   &req.ibv_cmd,
				   sizeof(req.ibv_cmd),
				   sizeof(req),
				   &resp.ibv_resp,
				   sizeof(resp.ibv_resp),
				   sizeof(resp));
	if (rc)
		goto err_cmd;

	if (resp.sq_cmb_offset) {
		qp->sq_cmb_ptr = ionic_map_device(qp->sq.size,
						  ctx->ibctx.cmd_fd,
						  resp.sq_cmb_offset);
		if (!qp->sq_cmb_ptr) {
			rc = errno;
			goto err_cmb;
		}
	}

	if (resp.rq_cmb_offset) {
		qp->rq_cmb_ptr = ionic_map_device(qp->rq.size,
						  ctx->ibctx.cmd_fd,
						  resp.rq_cmb_offset);
		if (!qp->rq_cmb_ptr) {
			rc = errno;
			goto err_cmb;
		}
	}

	qp->qpid = resp.qpid;
	ionic_queue_dbell_init(&qp->sq, qp->qpid);
	ionic_queue_dbell_init(&qp->rq, qp->qpid);

	pthread_mutex_lock(&ctx->mut);
	ionic_tbl_alloc_node(&ctx->qp_tbl);
	ionic_tbl_insert(&ctx->qp_tbl, qp, qp->qpid);
	pthread_mutex_unlock(&ctx->mut);

	if (qp->has_sq) {
		cq = to_ionic_cq(qp->vqp.qp.send_cq);
		ionic_spin_lock(ctx, &cq->lock);
		ionic_spin_unlock(ctx, &cq->lock);

		ex->cap.max_send_wr = qp->sq.mask;
		ex->cap.max_send_sge =
			ionic_v1_send_wqe_max_sge(qp->sq.stride_log2,
						  qp->sq_spec);
		ex->cap.max_inline_data =
			ionic_v1_send_wqe_max_data(qp->sq.stride_log2);
	}

	if (qp->has_rq) {
		cq = to_ionic_cq(qp->vqp.qp.recv_cq);
		ionic_spin_lock(ctx, &cq->lock);
		ionic_spin_unlock(ctx, &cq->lock);

		ex->cap.max_recv_wr = qp->rq.mask;
		ex->cap.max_recv_sge =
			ionic_v1_recv_wqe_max_sge(qp->rq.stride_log2,
						  qp->rq_spec);
	}

	return &qp->vqp.qp;

err_cmb:
	ibv_cmd_destroy_qp(&qp->vqp.qp);
err_cmd:
	ionic_qp_rq_destroy(qp);
err_rq:
	ionic_qp_sq_destroy(qp);
err_sq:
	pthread_spin_destroy(&qp->rq_lock);
	pthread_spin_destroy(&qp->sq_lock);
	free(qp);
err_qp:
	errno = rc;
	return NULL;
}

static void ionic_flush_qp(struct ionic_qp *qp)
{
	struct ionic_ctx *ctx = to_ionic_ctx(qp->vqp.qp.context);
	struct ionic_cq *cq;

	if (qp->vqp.qp.send_cq) {
		cq = to_ionic_cq(qp->vqp.qp.send_cq);

		/* Hold the CQ lock and QP sq_lock while setting up flush */
		ionic_spin_lock(ctx, &cq->lock);
		ionic_spin_lock(ctx, &qp->sq_lock);
		qp->sq_flush = true;
		if (!ionic_queue_empty(&qp->sq)) {
			cq->flush = true;
			list_del(&qp->cq_flush_sq);
			list_add_tail(&cq->flush_sq, &qp->cq_flush_sq);
		}
		ionic_spin_unlock(ctx, &qp->sq_lock);
		ionic_spin_unlock(ctx, &cq->lock);
	}

	if (qp->vqp.qp.recv_cq) {
		cq = to_ionic_cq(qp->vqp.qp.recv_cq);

		/* Hold the CQ lock and QP rq_lock while setting up flush */
		ionic_spin_lock(ctx, &cq->lock);
		ionic_spin_lock(ctx, &qp->rq_lock);
		qp->rq_flush = true;
		if (!ionic_queue_empty(&qp->rq)) {
			cq->flush = true;
			list_del(&qp->cq_flush_rq);
			list_add_tail(&cq->flush_rq, &qp->cq_flush_rq);
		}
		ionic_spin_unlock(ctx, &qp->rq_lock);
		ionic_spin_unlock(ctx, &cq->lock);
	}
}

static void ionic_reset_qp(struct ionic_qp *qp)
{
	struct ionic_ctx *ctx = to_ionic_ctx(qp->vqp.qp.context);
	struct ionic_cq *cq;
	int i;

	if (qp->vqp.qp.send_cq) {
		cq = to_ionic_cq(qp->vqp.qp.send_cq);
		ionic_spin_lock(ctx, &cq->lock);
		ionic_clean_cq(cq, qp->qpid);
		ionic_spin_unlock(ctx, &cq->lock);
	}

	if (qp->vqp.qp.recv_cq) {
		cq = to_ionic_cq(qp->vqp.qp.recv_cq);
		ionic_spin_lock(ctx, &cq->lock);
		ionic_clean_cq(cq, qp->qpid);
		ionic_spin_unlock(ctx, &cq->lock);
	}

	if (qp->has_sq) {
		ionic_spin_lock(ctx, &qp->sq_lock);
		qp->sq_flush = false;
		qp->sq_flush_rcvd = false;
		qp->sq_msn_prod = 0;
		qp->sq_msn_cons = 0;
		qp->sq_cmb_prod = 0;
		qp->sq.prod = 0;
		qp->sq.cons = 0;
		ionic_spin_unlock(ctx, &qp->sq_lock);
	}

	if (qp->has_rq) {
		ionic_spin_lock(ctx, &qp->rq_lock);
		qp->rq_flush = false;
		qp->rq.prod = 0;
		qp->rq.cons = 0;
		qp->rq_cmb_prod = 0;
		for (i = 0; i < qp->rq.mask; ++i)
			qp->rq_meta[i].next = &qp->rq_meta[i + 1];
		qp->rq_meta[i].next = IONIC_META_LAST;
		qp->rq_meta_head = &qp->rq_meta[0];
		ionic_spin_unlock(ctx, &qp->rq_lock);
	}
}

static int ionic_modify_qp(struct ibv_qp *ibqp,
			   struct ibv_qp_attr *attr,
			   int attr_mask)
{
	struct ibv_modify_qp cmd = {};
	struct ionic_qp *qp = to_ionic_qp(ibqp);
	int rc;

	if (!attr_mask)
		return 0;

	rc = ibv_cmd_modify_qp(ibqp, attr, attr_mask, &cmd, sizeof(cmd));
	if (rc)
		goto err_cmd;

	if (attr_mask & IBV_QP_STATE) {
		if (attr->qp_state == IBV_QPS_ERR)
			ionic_flush_qp(qp);
		else if (attr->qp_state == IBV_QPS_RESET)
			ionic_reset_qp(qp);
	}

err_cmd:
	return rc;
}

static int ionic_query_qp(struct ibv_qp *ibqp,
			  struct ibv_qp_attr *attr,
			  int attr_mask,
			  struct ibv_qp_init_attr *init_attr)
{
	struct ionic_qp *qp = to_ionic_qp(ibqp);
	struct ibv_query_qp cmd;
	int rc;

	rc = ibv_cmd_query_qp(ibqp, attr, attr_mask, init_attr,
			      &cmd, sizeof(cmd));

	if (qp->has_sq) {
		init_attr->cap.max_send_wr = qp->sq.mask;
		init_attr->cap.max_send_sge =
			ionic_v1_send_wqe_max_sge(qp->sq.stride_log2,
						  qp->sq_spec);
		init_attr->cap.max_inline_data =
			ionic_v1_send_wqe_max_data(qp->sq.stride_log2);
	}

	if (qp->has_rq) {
		init_attr->cap.max_recv_wr = qp->rq.mask;
		init_attr->cap.max_recv_sge =
			ionic_v1_send_wqe_max_sge(qp->rq.stride_log2,
						  qp->rq_spec);
	}

	attr->cap = init_attr->cap;

	return rc;
}

static int ionic_destroy_qp(struct ibv_qp *ibqp)
{
	struct ionic_ctx *ctx = to_ionic_ctx(ibqp->context);
	struct ionic_qp *qp = to_ionic_qp(ibqp);
	struct ionic_cq *cq;
	int rc;

	rc = ibv_cmd_destroy_qp(ibqp);
	if (rc)
		return rc;

	pthread_mutex_lock(&ctx->mut);
	ionic_tbl_free_node(&ctx->qp_tbl);
	ionic_tbl_delete(&ctx->qp_tbl, qp->qpid);
	pthread_mutex_unlock(&ctx->mut);

	if (qp->vqp.qp.send_cq) {
		cq = to_ionic_cq(qp->vqp.qp.send_cq);
		ionic_spin_lock(ctx, &cq->lock);
		ionic_clean_cq(cq, qp->qpid);
		list_del(&qp->cq_poll_sq);
		list_del(&qp->cq_flush_sq);
		ionic_spin_unlock(ctx, &cq->lock);
	}

	if (qp->vqp.qp.recv_cq) {
		cq = to_ionic_cq(qp->vqp.qp.recv_cq);
		ionic_spin_lock(ctx, &cq->lock);
		ionic_clean_cq(cq, qp->qpid);
		list_del(&qp->cq_flush_rq);
		ionic_spin_unlock(ctx, &cq->lock);
	}

	ionic_unmap(qp->sq_cmb_ptr, qp->sq.size);
	ionic_unmap(qp->rq_cmb_ptr, qp->rq.size);

	pthread_spin_destroy(&qp->rq_lock);
	pthread_spin_destroy(&qp->sq_lock);
	ionic_queue_destroy(&qp->rq);
	ionic_queue_destroy(&qp->sq);
	free(qp);

	return 0;
}

static int64_t ionic_prep_inline(void *data, uint32_t max_data,
				 struct ibv_sge *ibv_sgl, int num_sge)
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

static int64_t ionic_v1_prep_pld(struct ionic_v1_wqe *wqe,
				 union ionic_v1_pld *pld,
				 int spec, uint32_t max_sge,
				 struct ibv_sge *ibv_sgl,
				 int num_sge)
{
	static const int64_t bit_31 = 1l << 31;
	struct ionic_sge *sgl;
	__be32 *spec32 = NULL;
	__be16 *spec16 = NULL;
	int64_t len = 0, sg_len;
	int sg_i = 0;

	if (unlikely(num_sge < 0 || (uint32_t)num_sge > max_sge))
		return -EINVAL;

	if (spec && num_sge > IONIC_V1_SPEC_FIRST_SGE) {
		sg_i = IONIC_V1_SPEC_FIRST_SGE;

		if (num_sge > 8) {
			wqe->base.flags |= htobe16(IONIC_V1_FLAG_SPEC16);
			spec16 = pld->spec16;
		} else {
			wqe->base.flags |= htobe16(IONIC_V1_FLAG_SPEC32);
			spec32 = pld->spec32;
		}
	}

	sgl = &pld->sgl[sg_i];

	for (sg_i = 0; sg_i < num_sge; ++sg_i) {
		sg_len = ibv_sgl[sg_i].length;

		/* sge length zero means 2GB */
		if (unlikely(sg_len == 0))
			sg_len = bit_31;

		/* greater than 2GB data is invalid */
		if (unlikely(len + sg_len > bit_31))
			return -EINVAL;

		sgl[sg_i].va = htobe64(ibv_sgl[sg_i].addr);
		sgl[sg_i].len = htobe32(sg_len);
		sgl[sg_i].lkey = htobe32(ibv_sgl[sg_i].lkey);

		if (spec32) {
			spec32[sg_i] = sgl[sg_i].len;
		} else if (spec16) {
			if (unlikely(sg_len > UINT16_MAX))
				return -EINVAL;
			spec16[sg_i] = htobe16(sg_len);
		}

		len += sg_len;
	}

	return len;
}

static void ionic_v1_prep_base(struct ionic_qp *qp,
			       struct ibv_send_wr *wr,
			       struct ionic_sq_meta *meta,
			       struct ionic_v1_wqe *wqe)
{
	struct ionic_ctx *ctx = to_ionic_ctx(qp->vqp.qp.context);

	meta->wrid = wr->wr_id;
	meta->ibsts = IBV_WC_SUCCESS;
	meta->signal = false;
	meta->local_comp = false;

	wqe->base.wqe_id = qp->sq.prod;

	if (wr->send_flags & IBV_SEND_FENCE)
		wqe->base.flags |= htobe16(IONIC_V1_FLAG_FENCE);

	if (wr->send_flags & IBV_SEND_SOLICITED)
		wqe->base.flags |= htobe16(IONIC_V1_FLAG_SOL);

	if (qp->sig_all || wr->send_flags & IBV_SEND_SIGNALED) {
		wqe->base.flags |= htobe16(IONIC_V1_FLAG_SIG);
		meta->signal = true;
	}

	meta->seq = qp->sq_msn_prod;
	meta->remote = qp->vqp.qp.qp_type != IBV_QPT_UD &&
		!ionic_ibop_is_local(wr->opcode);

	if (meta->remote) {
		qp->sq_msn_idx[meta->seq] = qp->sq.prod;
		qp->sq_msn_prod = ionic_queue_next(&qp->sq, qp->sq_msn_prod);
	}

	ionic_dbg(ctx, "post send %u prod %u", qp->qpid, qp->sq.prod);
	ionic_dbg_xdump(ctx, "wqe", wqe, 1u << qp->sq.stride_log2);

	ionic_stat_incr_idx(ctx->stats, post_send_op, wqe->base.op);
	ionic_stat_add(ctx->stats, post_send_sig,
		       !!(wqe->base.flags & htobe16(IONIC_V1_FLAG_SIG)));
	ionic_stat_add(ctx->stats, post_send_inl,
		       !!(wqe->base.flags & htobe16(IONIC_V1_FLAG_INL)));

	ionic_queue_produce(&qp->sq);
}

static int ionic_v1_prep_common(struct ionic_qp *qp,
				struct ibv_send_wr *wr,
				struct ionic_sq_meta *meta,
				struct ionic_v1_wqe *wqe)
{
	int64_t signed_len;
	uint32_t mval;

	if (wr->send_flags & IBV_SEND_INLINE) {
		wqe->base.num_sge_key = 0;
		wqe->base.flags |= htobe16(IONIC_V1_FLAG_INL);
		mval = ionic_v1_send_wqe_max_data(qp->sq.stride_log2);
		signed_len = ionic_prep_inline(wqe->common.pld.data, mval,
					       wr->sg_list, wr->num_sge);
	} else {
		wqe->base.num_sge_key = wr->num_sge;
		mval = ionic_v1_send_wqe_max_sge(qp->sq.stride_log2,
						 qp->sq_spec);
		signed_len = ionic_v1_prep_pld(wqe, &wqe->common.pld,
					       qp->sq_spec, mval,
					       wr->sg_list, wr->num_sge);
	}

	if (unlikely(signed_len < 0))
		return -signed_len;

	meta->len = signed_len;
	wqe->common.length = htobe32(signed_len);

	ionic_v1_prep_base(qp, wr, meta, wqe);

	return 0;
}

static int ionic_v1_prep_send(struct ionic_qp *qp,
			      struct ibv_send_wr *wr)
{
	struct ionic_sq_meta *meta;
	struct ionic_v1_wqe *wqe;

	meta = &qp->sq_meta[qp->sq.prod];
	wqe = ionic_queue_at_prod(&qp->sq);

	memset(wqe, 0, 1u << qp->sq.stride_log2);

	meta->ibop = IBV_WC_SEND;

	switch (wr->opcode) {
	case IBV_WR_SEND:
		wqe->base.op = IONIC_V1_OP_SEND;
		break;
	case IBV_WR_SEND_WITH_IMM:
		wqe->base.op = IONIC_V1_OP_SEND_IMM;
		wqe->base.imm_data_key = wr->imm_data;
		break;
	case IBV_WR_SEND_WITH_INV:
		wqe->base.op = IONIC_V1_OP_SEND_INV;
		wqe->base.imm_data_key = htobe32(wr->imm_data);
		break;
	default:
		return EINVAL;
	}

	return ionic_v1_prep_common(qp, wr, meta, wqe);
}

static int ionic_v1_prep_send_ud(struct ionic_qp *qp, struct ibv_send_wr *wr)
{
	struct ionic_sq_meta *meta;
	struct ionic_v1_wqe *wqe;
	struct ionic_ah *ah;

	if (unlikely(!wr->wr.ud.ah))
		return EINVAL;

	ah = to_ionic_ah(wr->wr.ud.ah);

	meta = &qp->sq_meta[qp->sq.prod];
	wqe = ionic_queue_at_prod(&qp->sq);

	memset(wqe, 0, 1u << qp->sq.stride_log2);

	wqe->common.send.ah_id = htobe32(ah->ahid);
	wqe->common.send.dest_qpn = htobe32(wr->wr.ud.remote_qpn);
	wqe->common.send.dest_qkey = htobe32(wr->wr.ud.remote_qkey);

	meta->ibop = IBV_WC_SEND;

	switch (wr->opcode) {
	case IBV_WR_SEND:
		wqe->base.op = IONIC_V1_OP_SEND;
		break;
	case IBV_WR_SEND_WITH_IMM:
		wqe->base.op = IONIC_V1_OP_SEND_IMM;
		wqe->base.imm_data_key = wr->imm_data;
		break;
	default:
		return EINVAL;
	}

	return ionic_v1_prep_common(qp, wr, meta, wqe);
}

static int ionic_v1_prep_rdma(struct ionic_qp *qp,
			      struct ibv_send_wr *wr)
{
	struct ionic_sq_meta *meta;
	struct ionic_v1_wqe *wqe;

	meta = &qp->sq_meta[qp->sq.prod];
	wqe = ionic_queue_at_prod(&qp->sq);

	memset(wqe, 0, 1u << qp->sq.stride_log2);

	meta->ibop = IBV_WC_RDMA_WRITE;

	switch (wr->opcode) {
	case IBV_WR_RDMA_READ:
		if (wr->send_flags & (IBV_SEND_SOLICITED | IBV_SEND_INLINE))
			return EINVAL;
		meta->ibop = IBV_WC_RDMA_READ;
		wqe->base.op = IONIC_V1_OP_RDMA_READ;
		break;
	case IBV_WR_RDMA_WRITE:
		if (wr->send_flags & IBV_SEND_SOLICITED)
			return EINVAL;
		wqe->base.op = IONIC_V1_OP_RDMA_WRITE;
		break;
	case IBV_WR_RDMA_WRITE_WITH_IMM:
		wqe->base.op = IONIC_V1_OP_RDMA_WRITE_IMM;
		wqe->base.imm_data_key = wr->imm_data;
		break;
	default:
		return EINVAL;
	}

	wqe->common.rdma.remote_va_high =
		htobe32(wr->wr.rdma.remote_addr >> 32);
	wqe->common.rdma.remote_va_low = htobe32(wr->wr.rdma.remote_addr);
	wqe->common.rdma.remote_rkey = htobe32(wr->wr.rdma.rkey);

	return ionic_v1_prep_common(qp, wr, meta, wqe);
}

static int ionic_v1_prep_atomic(struct ionic_qp *qp,
				struct ibv_send_wr *wr)
{
	struct ionic_sq_meta *meta;
	struct ionic_v1_wqe *wqe;

	if (wr->num_sge != 1 || wr->sg_list[0].length != 8)
		return EINVAL;

	if (wr->send_flags & (IBV_SEND_SOLICITED | IBV_SEND_INLINE))
		return EINVAL;

	meta = &qp->sq_meta[qp->sq.prod];
	wqe = ionic_queue_at_prod(&qp->sq);

	memset(wqe, 0, 1u << qp->sq.stride_log2);

	switch (wr->opcode) {
	case IBV_WR_ATOMIC_CMP_AND_SWP:
		meta->ibop = IBV_WC_COMP_SWAP;
		wqe->base.op = IONIC_V1_OP_ATOMIC_CS;
		wqe->atomic.swap_add_high = htobe32(wr->wr.atomic.swap >> 32);
		wqe->atomic.swap_add_low = htobe32(wr->wr.atomic.swap);
		wqe->atomic.compare_high =
			htobe32(wr->wr.atomic.compare_add >> 32);
		wqe->atomic.compare_low = htobe32(wr->wr.atomic.compare_add);
		break;
	case IBV_WR_ATOMIC_FETCH_AND_ADD:
		meta->ibop = IBV_WC_FETCH_ADD;
		wqe->base.op = IONIC_V1_OP_ATOMIC_FA;
		wqe->atomic.swap_add_high =
			htobe32(wr->wr.atomic.compare_add >> 32);
		wqe->atomic.swap_add_low = htobe32(wr->wr.atomic.compare_add);
		break;
	default:
		return EINVAL;
	}

	wqe->atomic.remote_va_high = htobe32(wr->wr.atomic.remote_addr >> 32);
	wqe->atomic.remote_va_low = htobe32(wr->wr.atomic.remote_addr);
	wqe->atomic.remote_rkey = htobe32(wr->wr.atomic.rkey);

	wqe->base.num_sge_key = 1;
	wqe->atomic.sge.va = htobe64(wr->sg_list[0].addr);
	wqe->atomic.sge.len = htobe32(8);
	wqe->atomic.sge.lkey = htobe32(wr->sg_list[0].lkey);

	ionic_v1_prep_base(qp, wr, meta, wqe);

	return 0;
}

static int ionic_v1_prep_inv(struct ionic_qp *qp, struct ibv_send_wr *wr)
{
	struct ionic_sq_meta *meta;
	struct ionic_v1_wqe *wqe;

	if (wr->send_flags & (IBV_SEND_SOLICITED | IBV_SEND_INLINE))
		return EINVAL;

	meta = &qp->sq_meta[qp->sq.prod];
	wqe = ionic_queue_at_prod(&qp->sq);

	memset(wqe, 0, 1u << qp->sq.stride_log2);

	wqe->base.op = IONIC_V1_OP_LOCAL_INV;
	wqe->base.imm_data_key = htobe32(wr->imm_data);

	ionic_v1_prep_base(qp, wr, meta, wqe);

	return 0;
}

static int ionic_v1_prep_bind(struct ionic_qp *qp,
			      struct ibv_send_wr *wr,
			      bool send_path)
{
	struct ionic_sq_meta *meta;
	struct ionic_v1_wqe *wqe;
	int flags;

	if (wr->send_flags & (IBV_SEND_SOLICITED | IBV_SEND_INLINE))
		return EINVAL;

	/* type 1 must use bind_mw; type 2 must use post_send */
	if (send_path == (wr->bind_mw.mw->type == IBV_MW_TYPE_1))
		return EINVAL;

	/* only remote access is allowed */
	if (wr->bind_mw.bind_info.mw_access_flags &
	    ~(IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_READ |
	      IBV_ACCESS_REMOTE_ATOMIC))
		return EINVAL;

	/* iff length is given, some kind of access must be allowed */
	if (!wr->bind_mw.bind_info.length !=
	    !wr->bind_mw.bind_info.mw_access_flags)
		return EINVAL;

	/* only type 1 can unbind with zero length */
	if (!wr->bind_mw.bind_info.length &&
	    wr->bind_mw.mw->type != IBV_MW_TYPE_1)
		return EINVAL;

	meta = &qp->sq_meta[qp->sq.prod];
	wqe = ionic_queue_at_prod(&qp->sq);

	memset(wqe, 0, 1u << qp->sq.stride_log2);

	flags = to_ionic_mr_flags(wr->bind_mw.bind_info.mw_access_flags);

	if (wr->bind_mw.mw->type == IBV_MW_TYPE_1)
		flags |= IONIC_MRF_MW_1;
	else
		flags |= IONIC_MRF_MW_2;

	wqe->base.op = IONIC_V1_OP_BIND_MW;
	wqe->base.num_sge_key = wr->bind_mw.rkey;
	wqe->base.imm_data_key = htobe32(wr->bind_mw.mw->rkey);
	wqe->bind_mw.va = htobe64(wr->bind_mw.bind_info.addr);
	wqe->bind_mw.length = htobe64(wr->bind_mw.bind_info.length);
	wqe->bind_mw.lkey = htobe32(wr->bind_mw.bind_info.mr->lkey);
	wqe->bind_mw.flags = htobe16(flags);

	ionic_v1_prep_base(qp, wr, meta, wqe);

	return 0;
}

static int ionic_v1_prep_one_rc(struct ionic_qp *qp,
				struct ibv_send_wr *wr,
				bool send_path)
{
	struct ionic_ctx *ctx = to_ionic_ctx(qp->vqp.qp.context);
	int rc = 0;

	switch (wr->opcode) {
	case IBV_WR_SEND:
	case IBV_WR_SEND_WITH_IMM:
	case IBV_WR_SEND_WITH_INV:
		rc = ionic_v1_prep_send(qp, wr);
		break;
	case IBV_WR_RDMA_READ:
	case IBV_WR_RDMA_WRITE:
	case IBV_WR_RDMA_WRITE_WITH_IMM:
		rc = ionic_v1_prep_rdma(qp, wr);
		break;
	case IBV_WR_ATOMIC_CMP_AND_SWP:
	case IBV_WR_ATOMIC_FETCH_AND_ADD:
		rc = ionic_v1_prep_atomic(qp, wr);
		break;
	case IBV_WR_LOCAL_INV:
		rc = ionic_v1_prep_inv(qp, wr);
		break;
	case IBV_WR_BIND_MW:
		rc = ionic_v1_prep_bind(qp, wr, send_path);
		break;
	default:
		ionic_dbg(ctx, "invalid opcode %d", wr->opcode);
		rc = EINVAL;
	}

	return rc;
}

static int ionic_v1_prep_one_ud(struct ionic_qp *qp,
				struct ibv_send_wr *wr)
{
	struct ionic_ctx *ctx = to_ionic_ctx(qp->vqp.qp.context);
	int rc = 0;

	switch (wr->opcode) {
	case IBV_WR_SEND:
	case IBV_WR_SEND_WITH_IMM:
		rc = ionic_v1_prep_send_ud(qp, wr);
		break;
	default:
		ionic_dbg(ctx, "invalid opcode %d", wr->opcode);
		rc = EINVAL;
	}

	return rc;
}

static void ionic_post_send_cmb(struct ionic_ctx *ctx, struct ionic_qp *qp)
{
	void *cmb_ptr;
	void *wqe_ptr;
	uint32_t stride;
	uint16_t pos, end;
	uint8_t stride_log2;

	stride_log2 = qp->sq.stride_log2;
	stride = 1u << stride_log2;

	pos = qp->sq_cmb_prod;
	end = qp->sq.prod;

	while (pos != end) {
		cmb_ptr = qp->sq_cmb_ptr + ((size_t)pos << stride_log2);
		wqe_ptr = ionic_queue_at(&qp->sq, pos);

		mmio_wc_start();
		mmio_memcpy_x64(cmb_ptr, wqe_ptr, stride);
		mmio_flush_writes();

		pos = ionic_queue_next(&qp->sq, pos);

		ionic_dbg(ctx, "dbell qp %u sq val %#lx",
			  qp->qpid, qp->sq.dbell | pos);
		ionic_dbell_ring(&ctx->dbpage[ctx->sq_qtype],
				 qp->sq.dbell | pos);
	}

	ionic_stat_add(ctx->stats, post_send_cmb,
		       (end - qp->sq_cmb_prod) & qp->sq.mask);

	qp->sq_cmb_prod = end;
}

static void ionic_post_recv_cmb(struct ionic_ctx *ctx, struct ionic_qp *qp)
{
	void *cmb_ptr;
	void *wqe_ptr;
	uint32_t stride;
	uint16_t pos, end;
	uint8_t stride_log2;

	stride_log2 = qp->rq.stride_log2;

	pos = qp->rq_cmb_prod;
	end = qp->rq.prod;

	if (pos > end) {
		cmb_ptr = qp->rq_cmb_ptr + ((size_t)pos << stride_log2);
		wqe_ptr = ionic_queue_at(&qp->rq, pos);

		stride = (uint32_t)(qp->rq.mask - pos + 1) << stride_log2;
		mmio_wc_start();
		mmio_memcpy_x64(cmb_ptr, wqe_ptr, stride);
		mmio_flush_writes();

		pos = 0;

		ionic_dbg(ctx, "dbell qp %u rq val %#lx",
			  qp->qpid, qp->rq.dbell | pos);
		ionic_dbell_ring(&ctx->dbpage[ctx->rq_qtype],
				 qp->rq.dbell | pos);
	}

	if (pos < end) {
		cmb_ptr = qp->rq_cmb_ptr + ((size_t)pos << stride_log2);
		wqe_ptr = ionic_queue_at(&qp->rq, pos);

		stride = (uint32_t)(end - pos) << stride_log2;
		mmio_wc_start();
		mmio_memcpy_x64(cmb_ptr, wqe_ptr, stride);
		mmio_flush_writes();

		pos = end;

		ionic_dbg(ctx, "dbell qp %u rq val %#lx",
			  qp->qpid, qp->rq.dbell | pos);
		ionic_dbell_ring(&ctx->dbpage[ctx->rq_qtype],
				 qp->rq.dbell | pos);
	}

	ionic_stat_add(ctx->stats, post_recv_cmb,
		       (end - qp->rq_cmb_prod) & qp->rq.mask);

	qp->rq_cmb_prod = end;
}

static int ionic_post_send_common(struct ionic_ctx *ctx,
				  struct ionic_cq *cq,
				  struct ionic_qp *qp,
				  struct ibv_send_wr *wr,
				  struct ibv_send_wr **bad,
				  bool send_path)
{
	int spend, rc = 0;
	uint16_t old_prod;

	ionic_lat_trace(ctx->lats, application);
	ionic_stat_incr(ctx->stats, post_send);

	if (unlikely(!bad))
		return EINVAL;

	if (unlikely(!qp->has_sq)) {
		*bad = wr;
		return EINVAL;
	}

	if (unlikely(qp->vqp.qp.state < IBV_QPS_RTS)) {
		*bad = wr;
		return EINVAL;
	}

	ionic_spin_lock(ctx, &qp->sq_lock);

	ionic_stat_incr_idx_fls(ctx->stats, post_send_qlen,
				ionic_queue_length(&qp->sq));

	old_prod = qp->sq.prod;

	while (wr) {
		if (ionic_queue_full(&qp->sq)) {
			ionic_dbg(ctx,
				  "send queue full cons %u prod %u",
				  qp->sq.cons, qp->sq.prod);
			rc = ENOMEM;
			goto out;
		}

		if (qp->vqp.qp.qp_type == IBV_QPT_UD)
			rc = ionic_v1_prep_one_ud(qp, wr);
		else
			rc = ionic_v1_prep_one_rc(qp, wr, send_path);
		if (rc)
			goto out;

		wr = wr->next;
	}

out:
	old_prod = (qp->sq.prod - old_prod) & qp->sq.mask;
	ionic_stat_incr_idx_fls(ctx->stats, post_send_nwr, old_prod);
	ionic_stat_add(ctx->stats, post_send_wr, old_prod);

	if (ionic_spin_trylock(ctx, &cq->lock)) {
		ionic_spin_unlock(ctx, &qp->sq_lock);
		ionic_spin_lock(ctx, &cq->lock);
		ionic_spin_lock(ctx, &qp->sq_lock);
	}

	if (likely(qp->sq.prod != qp->sq_old_prod)) {
		/* ring cq doorbell just in time */
		spend = (qp->sq.prod - qp->sq_old_prod) & qp->sq.mask;
		ionic_reserve_cq(ctx, cq, spend);

		qp->sq_old_prod = qp->sq.prod;

		if (qp->sq_cmb_ptr) {
			ionic_post_send_cmb(ctx, qp);
		} else {
			udma_to_device_barrier();
			ionic_dbg(ctx, "dbell qp %u sq val %#lx",
				  qp->qpid, ionic_queue_dbell_val(&qp->sq));
			ionic_dbell_ring(&ctx->dbpage[ctx->sq_qtype],
					 ionic_queue_dbell_val(&qp->sq));
		}
	}

	if (qp->sq_flush) {
		cq->flush = true;
		list_del(&qp->cq_flush_sq);
		list_add_tail(&cq->flush_sq, &qp->cq_flush_sq);
	}

	ionic_spin_unlock(ctx, &qp->sq_lock);
	ionic_spin_unlock(ctx, &cq->lock);

	ionic_stat_add(ctx->stats, post_send_err, !!rc);
	ionic_lat_trace(ctx->lats, post_send);

	*bad = wr;
	return rc;
}

static int ionic_v1_prep_recv(struct ionic_qp *qp,
			      struct ibv_recv_wr *wr)
{
	struct ionic_ctx *ctx = to_ionic_ctx(qp->vqp.qp.context);
	struct ionic_rq_meta *meta;
	struct ionic_v1_wqe *wqe;
	int64_t signed_len;
	uint32_t mval;

	wqe = ionic_queue_at_prod(&qp->rq);

	/* if wqe is owned by device, caller can try posting again soon */
	if (wqe->base.flags & htobe16(IONIC_V1_FLAG_FENCE))
		return -EAGAIN;

	meta = qp->rq_meta_head;
	if (unlikely(meta == IONIC_META_LAST) ||
	    unlikely(meta == IONIC_META_POSTED))
		return -EIO;

	memset(wqe, 0, 1u << qp->rq.stride_log2);

	mval = ionic_v1_recv_wqe_max_sge(qp->rq.stride_log2, qp->rq_spec);
	signed_len = ionic_v1_prep_pld(wqe, &wqe->recv.pld,
				       qp->rq_spec, mval,
				       wr->sg_list, wr->num_sge);
	if (signed_len < 0)
		return -signed_len;

	meta->wrid = wr->wr_id;

	wqe->base.wqe_id = meta - qp->rq_meta;
	wqe->base.num_sge_key = wr->num_sge;

	/* total length for recv goes in base imm_data_key */
	wqe->base.imm_data_key = htobe32(signed_len);

	ionic_dbg(ctx, "post recv %u prod %u", qp->qpid, qp->rq.prod);
	ionic_dbg_xdump(ctx, "wqe", wqe, 1u << qp->rq.stride_log2);
	ionic_queue_produce(&qp->rq);

	qp->rq_meta_head = meta->next;
	meta->next = IONIC_META_POSTED;

	return 0;
}

static int ionic_post_recv_common(struct ionic_ctx *ctx,
				  struct ionic_cq *cq,
				  struct ionic_qp *qp,
				  struct ibv_recv_wr *wr,
				  struct ibv_recv_wr **bad)
{
	int spend, rc = 0;
	uint16_t old_prod;

	ionic_lat_trace(ctx->lats, application);
	ionic_stat_incr(ctx->stats, post_recv);

	if (unlikely(!bad))
		return EINVAL;

	if (unlikely(!qp->has_rq)) {
		*bad = wr;
		return EINVAL;
	}

	if (unlikely(qp->vqp.qp.state < IBV_QPS_INIT)) {
		*bad = wr;
		return EINVAL;
	}

	ionic_spin_lock(ctx, &qp->rq_lock);

	ionic_stat_incr_idx_fls(ctx->stats, post_recv_qlen,
				ionic_queue_length(&qp->rq));

	old_prod = qp->rq.prod;

	while (wr) {
		if (ionic_queue_full(&qp->rq)) {
			ionic_dbg(ctx, "recv queue full cons %u prod %u",
				  qp->rq.cons, qp->rq.prod);
			rc = ENOMEM;
			goto out;
		}

		rc = ionic_v1_prep_recv(qp, wr);
		if (rc)
			goto out;

		wr = wr->next;
	}

out:
	old_prod = (qp->rq.prod - old_prod) & qp->rq.mask;
	ionic_stat_incr_idx_fls(ctx->stats, post_recv_nwr, old_prod);
	ionic_stat_add(ctx->stats, post_recv_wr, old_prod);

	if (!cq) {
		ionic_spin_unlock(ctx, &qp->rq_lock);
		goto out_unlocked;
	}

	if (ionic_spin_trylock(ctx, &cq->lock)) {
		ionic_spin_unlock(ctx, &qp->rq_lock);
		ionic_spin_lock(ctx, &cq->lock);
		ionic_spin_lock(ctx, &qp->rq_lock);
	}

	if (likely(qp->rq.prod != qp->rq_old_prod)) {
		/* ring cq doorbell just in time */
		spend = (qp->rq.prod - qp->rq_old_prod) & qp->rq.mask;
		ionic_reserve_cq(ctx, cq, spend);

		qp->rq_old_prod = qp->rq.prod;

		if (qp->rq_cmb_ptr) {
			ionic_post_recv_cmb(ctx, qp);
		} else {
			udma_to_device_barrier();
			ionic_dbg(ctx, "dbell qp %u rq val %#lx",
				  qp->qpid, ionic_queue_dbell_val(&qp->rq));
			ionic_dbell_ring(&ctx->dbpage[ctx->rq_qtype],
					 ionic_queue_dbell_val(&qp->rq));
		}
	}

	if (qp->rq_flush) {
		cq->flush = true;
		list_del(&qp->cq_flush_rq);
		list_add_tail(&cq->flush_rq, &qp->cq_flush_rq);
	}

	ionic_spin_unlock(ctx, &qp->rq_lock);
	ionic_spin_unlock(ctx, &cq->lock);

out_unlocked:
	ionic_lat_trace(ctx->lats, post_recv);
	ionic_stat_add(ctx->stats, post_recv_err, !!rc);

	*bad = wr;
	return rc;
}

static int ionic_post_send(struct ibv_qp *ibqp,
			   struct ibv_send_wr *wr,
			   struct ibv_send_wr **bad)
{
	struct ionic_ctx *ctx = to_ionic_ctx(ibqp->context);
	struct ionic_qp *qp = to_ionic_qp(ibqp);
	struct ionic_cq *cq = to_ionic_cq(ibqp->send_cq);

	return ionic_post_send_common(ctx, cq, qp, wr, bad, true);
}

static int ionic_post_recv(struct ibv_qp *ibqp,
			   struct ibv_recv_wr *wr,
			   struct ibv_recv_wr **bad)
{
	struct ionic_ctx *ctx = to_ionic_ctx(ibqp->context);
	struct ionic_qp *qp = to_ionic_qp(ibqp);
	struct ionic_cq *cq = to_ionic_cq(ibqp->recv_cq);

	return ionic_post_recv_common(ctx, cq, qp, wr, bad);
}

static struct ibv_qp *ionic_create_qp(struct ibv_pd *ibpd,
				      struct ibv_qp_init_attr *attr)
{
	struct ibv_qp_init_attr_ex ex = {
		.qp_context = attr->qp_context,
		.send_cq = attr->send_cq,
		.recv_cq = attr->recv_cq,
		.srq = attr->srq,
		.cap = attr->cap,
		.qp_type = attr->qp_type,
		.sq_sig_all = attr->sq_sig_all,
		.comp_mask = IBV_QP_INIT_ATTR_PD,
		.pd = ibpd,
	};
	struct verbs_context *vctx;
	struct ibv_qp *ibqp;

	vctx = verbs_get_ctx_op(ibpd->context, create_qp_ex);
	if (!vctx) {
		errno = EOPNOTSUPP;
		return NULL;
	}

	ibqp = vctx->create_qp_ex(&vctx->context, &ex);

	attr->cap = ex.cap;

	return ibqp;
}

static struct ibv_ah *ionic_create_ah(struct ibv_pd *ibpd,
				      struct ibv_ah_attr *attr)
{
	struct ionic_ah *ah;
	struct uionic_ah_resp resp;
	int rc;

	ah = calloc(1, sizeof(*ah));
	if (!ah) {
		rc = errno;
		goto err_ah;
	}

	rc = ibv_cmd_create_ah(ibpd, &ah->ibah, attr,
			       &resp.ibv_resp, sizeof(resp));
	if (rc)
		goto err_cmd;

	ah->ahid = resp.ahid;

	return &ah->ibah;

err_cmd:
	free(ah);
err_ah:
	errno = rc;
	return NULL;
}

static int ionic_destroy_ah(struct ibv_ah *ibah)
{
	struct ionic_ah *ah = to_ionic_ah(ibah);
	int rc;

	rc = ibv_cmd_destroy_ah(ibah);
	if (rc)
		return rc;

	free(ah);

	return 0;
}

static struct ibv_mw *ionic_alloc_mw(struct ibv_pd *ibpd,
				     enum ibv_mw_type type)
{
	struct ibv_mw *ibmw;
	struct ibv_alloc_mw cmd;
	struct ibv_alloc_mw_resp resp;
	int rc;

	ibmw = calloc(1, sizeof(*ibmw));
	if (!ibmw) {
		rc = errno;
		goto err_mw;
	}

	rc = ibv_cmd_alloc_mw(ibpd, type, ibmw,
			      &cmd, sizeof(cmd),
			      &resp, sizeof(resp));
	if (rc)
		goto err_cmd;

	return ibmw;

err_cmd:
	free(ibmw);
err_mw:
	errno = rc;
	return NULL;
}

static int ionic_bind_mw(struct ibv_qp *ibqp, struct ibv_mw *ibmw,
			 struct ibv_mw_bind *bind)
{
	struct ionic_ctx *ctx = to_ionic_ctx(ibqp->context);
	struct ionic_qp *qp = to_ionic_qp(ibqp);
	struct ionic_cq *cq = to_ionic_cq(ibqp->send_cq);
	struct ibv_send_wr *bad;
	struct ibv_send_wr wr = {
		.opcode = IBV_WR_BIND_MW,
		.wr_id = bind->wr_id,
		.send_flags = bind->send_flags,
		.bind_mw = {
			.mw = ibmw,
			.rkey = ibmw->rkey,
			.bind_info = bind->bind_info,
		}
	};
	int rc;

	if (bind->bind_info.length)
		wr.bind_mw.rkey = ibv_inc_rkey(ibmw->rkey);

	rc = ionic_post_send_common(ctx, cq, qp, &wr, &bad, false);
	if (!rc)
		ibmw->rkey = wr.bind_mw.rkey;

	return rc;
}

static int ionic_dealloc_mw(struct ibv_mw *ibmw)
{
	struct ibv_dealloc_mw cmd;
	int rc;

	rc = ibv_cmd_dealloc_mw(ibmw, &cmd, sizeof(cmd));
	if (rc)
		return rc;

	free(ibmw);

	return 0;
}

static const struct ibv_context_ops ionic_ctx_ops = {
	.query_device		= ionic_query_device,
	.query_port		= ionic_query_port,
	.alloc_pd		= ionic_alloc_pd,
	.dealloc_pd		= ionic_dealloc_pd,
	.reg_mr			= ionic_reg_mr,
	.rereg_mr		= ionic_rereg_mr,
	.dereg_mr		= ionic_dereg_mr,
	.create_cq		= ionic_create_cq,
	.poll_cq		= ionic_poll_cq,
	.req_notify_cq		= ionic_req_notify_cq,
	.destroy_cq		= ionic_destroy_cq,
	.create_qp		= ionic_create_qp,
	.query_qp		= ionic_query_qp,
	.modify_qp		= ionic_modify_qp,
	.destroy_qp		= ionic_destroy_qp,
	.post_send		= ionic_post_send,
	.post_recv		= ionic_post_recv,
	.create_ah		= ionic_create_ah,
	.destroy_ah		= ionic_destroy_ah,
	.alloc_mw		= ionic_alloc_mw,
	.bind_mw		= ionic_bind_mw,
	.dealloc_mw		= ionic_dealloc_mw,
};

void ionic_set_ops(struct ibv_context *ibctx)
{
	struct verbs_context *ionic_vctx = verbs_get_ctx(ibctx);

	ibctx->ops = ionic_ctx_ops;

	verbs_set_ctx_op(ionic_vctx, create_qp_ex, ionic_create_qp_ex);
}
