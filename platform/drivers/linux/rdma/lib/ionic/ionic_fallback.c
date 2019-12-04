/*
 * Copyright (c) 2018-2019 Pensando Systems, Inc.  All rights reserved.
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

#include "ionic.h"

static int fallback_query_device(struct ibv_context *ibctx,
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

static int fallback_query_port(struct ibv_context *ibctx, uint8_t port,
			       struct ibv_port_attr *port_attr)
{
	struct ibv_query_port req = {};

	return ibv_cmd_query_port(ibctx, port, port_attr,
				  &req, sizeof(req));
}

static struct ibv_pd *fallback_alloc_pd(struct ibv_context *ibctx)
{
	struct ibv_pd *ibpd;
	struct ibv_alloc_pd req = {};
	struct ib_uverbs_alloc_pd_resp resp = {};
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

static int fallback_free_pd(struct ibv_pd *ibpd)
{
	int rc;

	rc = ibv_cmd_dealloc_pd(ibpd);
	if (rc)
		return rc;

	free(ibpd);

	return 0;
}

static struct ibv_mr *fallback_reg_mr(struct ibv_pd *ibpd,
				      void *addr,
				      size_t len,
				      uint64_t hca_va,
				      int access)
{
	struct verbs_mr *vmr;
	struct ibv_reg_mr req = {};
	struct ib_uverbs_reg_mr_resp resp = {};
	int rc;

	vmr = calloc(1, sizeof(*vmr));
	if (!vmr) {
		rc = errno;
		goto err_mr;
	}

	rc = ibv_cmd_reg_mr(ibpd, addr, len, hca_va, access, vmr,
			    &req, sizeof(req),
			    &resp, sizeof(resp));
	if (rc)
		goto err_cmd;

	return &vmr->ibv_mr;

err_cmd:
	free(vmr);
err_mr:
	errno = rc;
	return NULL;
}

static int fallback_rereg_mr(struct verbs_mr *vmr, int flags,
			     struct ibv_pd *pd, void *addr, size_t length,
			     int access)
{
	struct ibv_rereg_mr cmd;
	struct ib_uverbs_rereg_mr_resp resp;

	if (flags & IBV_REREG_MR_KEEP_VALID)
		return ENOTSUP;

	return ibv_cmd_rereg_mr(vmr, flags, addr, length,
				(uintptr_t)addr, access, pd,
				&cmd, sizeof(cmd),
				&resp, sizeof(resp));
}

static int fallback_dereg_mr(struct verbs_mr *vmr)
{
	int rc;

	rc = ibv_cmd_dereg_mr(vmr);
	if (rc)
		return rc;

	free(vmr);

	return 0;
}

static struct ibv_cq *fallback_create_cq(struct ibv_context *ibctx, int ncqe,
					 struct ibv_comp_channel *channel, int vec)
{
	struct ibv_cq *ibcq;
	struct ibv_create_cq req = {};
	struct ib_uverbs_create_cq_resp resp = {};
	int rc;

	ibcq = calloc(1, sizeof(*ibcq));
	if (!ibcq) {
		rc = errno;
		goto err_cq;
	}

	rc = ibv_cmd_create_cq(ibctx, ncqe, channel, vec, ibcq,
			       &req, sizeof(req),
			       &resp, sizeof(resp));
	if (rc)
		goto err_cmd;

	return ibcq;

err_cmd:
	free(ibcq);
err_cq:
	errno = rc;
	return NULL;
}

static int fallback_resize_cq(struct ibv_cq *ibcq, int ncqe)
{
	struct ibv_resize_cq req = {};
	struct ib_uverbs_resize_cq_resp resp = {};

	return ibv_cmd_resize_cq(ibcq, ncqe,
				 &req, sizeof(req),
				 &resp, sizeof(resp));
}

static int fallback_destroy_cq(struct ibv_cq *ibcq)
{
	int rc;

	rc = ibv_cmd_destroy_cq(ibcq);
	if (rc)
		return rc;

	free(ibcq);

	return 0;
}

static int fallback_poll_cq(struct ibv_cq *ibcq, int nwc, struct ibv_wc *wc)
{
	return ibv_cmd_poll_cq(ibcq, nwc, wc);
}

static int fallback_req_notify_cq(struct ibv_cq *ibcq, int solicited_only)
{
	return ibv_cmd_req_notify_cq(ibcq, solicited_only);
}

static struct ibv_qp *fallback_create_qp_ex(struct ibv_context *ibctx,
					    struct ibv_qp_init_attr_ex *ex)
{
	struct verbs_qp *vqp;
	struct ibv_create_qp_ex req = {};
	struct ib_uverbs_ex_create_qp_resp resp = {};
	int rc;

	vqp = calloc(1, sizeof(*vqp));
	if (!vqp) {
		rc = errno;
		goto err_qp;
	}

	rc = ibv_cmd_create_qp_ex2(ibctx, vqp, sizeof(*vqp), ex,
				   &req, sizeof(req),
				   &resp, sizeof(resp));
	if (rc)
		goto err_cmd;

	return &vqp->qp;

err_cmd:
	free(vqp);
err_qp:
	errno = rc;
	return NULL;
}

static struct ibv_qp *fallback_create_qp(struct ibv_pd *ibpd,
					 struct ibv_qp_init_attr *attr)
{
	struct verbs_context *vctx =
		container_of(ibpd->context, struct verbs_context, context);
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
	struct ibv_qp *ibqp;

	ibqp = vctx->create_qp_ex(&vctx->context, &ex);

	attr->cap = ex.cap;

	return ibqp;
}


static int fallback_modify_qp(struct ibv_qp *ibqp,
			      struct ibv_qp_attr *attr,
			      int mask)
{
	struct ibv_modify_qp req = {};

	return ibv_cmd_modify_qp(ibqp, attr, mask, &req, sizeof(req));
}

static int fallback_query_qp(struct ibv_qp *ibqp,
			     struct ibv_qp_attr *attr,
			     int mask,
			     struct ibv_qp_init_attr *init_attr)
{
	struct ibv_query_qp req = {};

	return ibv_cmd_query_qp(ibqp, attr, mask, init_attr,
				&req, sizeof(req));
}

static int fallback_destroy_qp(struct ibv_qp *ibqp)
{
	struct verbs_qp *vqp = container_of(ibqp, struct verbs_qp, qp);
	int rc;

	rc = ibv_cmd_destroy_qp(ibqp);
	if (rc)
		return rc;

	free(vqp);

	return 0;
}

static int fallback_post_send(struct ibv_qp *ibqp,
			      struct ibv_send_wr *wr,
			      struct ibv_send_wr **bad)
{
	return ibv_cmd_post_send(ibqp, wr, bad);
}

static int fallback_post_recv(struct ibv_qp *ibqp,
			      struct ibv_recv_wr *wr,
			      struct ibv_recv_wr **bad)
{
	return ibv_cmd_post_recv(ibqp, wr, bad);
}

static struct ibv_srq *fallback_create_srq_ex(struct ibv_context *ibctx,
					      struct ibv_srq_init_attr_ex *ex)
{
	struct verbs_srq *vsrq;
	struct ibv_create_xsrq req = {};
	struct ib_uverbs_create_srq_resp resp = {};
	int rc;

	vsrq = calloc(1, sizeof(*vsrq));
	if (!vsrq) {
		rc = errno;
		goto err_srq;
	}

	rc = ibv_cmd_create_srq_ex(ibctx, vsrq, sizeof(*vsrq), ex,
				   &req, sizeof(req),
				   &resp, sizeof(resp));
	if (rc)
		goto err_cmd;

	return &vsrq->srq;

err_cmd:
	free(vsrq);
err_srq:
	errno = rc;
	return NULL;
}

static struct ibv_srq *fallback_create_srq(struct ibv_pd *ibpd,
					   struct ibv_srq_init_attr *attr)
{
	struct verbs_context *vctx =
		container_of(ibpd->context, struct verbs_context, context);
	struct ibv_srq_init_attr_ex ex = {
		.srq_context = attr->srq_context,
		.attr = attr->attr,
		.srq_type = IBV_SRQT_BASIC,
		.pd = ibpd,
	};
	struct ibv_srq *ibsrq;

	ibsrq = vctx->create_srq_ex(&vctx->context, &ex);

	attr->attr = ex.attr;

	return ibsrq;
}

static int fallback_modify_srq(struct ibv_srq *ibsrq, struct ibv_srq_attr *attr,
			       int mask)
{
	struct ibv_modify_srq req = {};

	return ibv_cmd_modify_srq(ibsrq, attr, mask,
				  &req, sizeof(req));
}

static int fallback_query_srq(struct ibv_srq *ibsrq, struct ibv_srq_attr *attr)
{
	struct ibv_query_srq req = {};

	return ibv_cmd_query_srq(ibsrq, attr,
				 &req, sizeof(req));
}

static int fallback_destroy_srq(struct ibv_srq *ibsrq)
{
	struct verbs_srq *vsrq = container_of(ibsrq, struct verbs_srq, srq);
	int rc;

	rc = ibv_cmd_destroy_srq(ibsrq);
	if (rc)
		return rc;

	free(vsrq);

	return 0;
}

static int fallback_post_srq_recv(struct ibv_srq *ibsrq, struct ibv_recv_wr *wr,
				  struct ibv_recv_wr **bad)
{
	return ibv_cmd_post_srq_recv(ibsrq, wr, bad);
}

static struct ibv_ah *fallback_create_ah(struct ibv_pd *ibpd,
					 struct ibv_ah_attr *attr)
{
	struct ibv_ah *ibah;
	struct ib_uverbs_create_ah_resp resp;
	int rc;

	ibah = calloc(1, sizeof(*ibah));
	if (!ibah) {
		rc = errno;
		goto err_ah;
	}

	rc = ibv_cmd_create_ah(ibpd, ibah, attr,
			       &resp, sizeof(resp));
	if (rc)
		goto err_cmd;

	return ibah;

err_cmd:
	free(ibah);
err_ah:
	errno = rc;
	return NULL;
}

static int fallback_destroy_ah(struct ibv_ah *ibah)
{
	int rc;

	rc = ibv_cmd_destroy_ah(ibah);
	if (rc)
		return rc;

	free(ibah);

	return 0;
}

static struct ibv_mw *fallback_alloc_mw(struct ibv_pd *ibpd,
					enum ibv_mw_type type)
{
	struct ibv_mw *ibmw;
	struct ibv_alloc_mw cmd;
	struct ib_uverbs_alloc_mw_resp resp;
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

static int fallback_dealloc_mw(struct ibv_mw *ibmw)
{
	int rc;

	rc = ibv_cmd_dealloc_mw(ibmw);
	if (rc)
		return rc;

	free(ibmw);

	return 0;
}

const struct verbs_context_ops fallback_ctx_ops = {
	.query_device		= fallback_query_device,
	.query_port		= fallback_query_port,
	.alloc_pd		= fallback_alloc_pd,
	.dealloc_pd		= fallback_free_pd,
	.reg_mr			= fallback_reg_mr,
	.rereg_mr		= fallback_rereg_mr,
	.dereg_mr		= fallback_dereg_mr,
	.create_cq		= fallback_create_cq,
	.poll_cq		= fallback_poll_cq,
	.req_notify_cq		= fallback_req_notify_cq,
	.resize_cq		= fallback_resize_cq,
	.destroy_cq		= fallback_destroy_cq,
	.create_srq		= fallback_create_srq,
	.create_srq_ex		= fallback_create_srq_ex,
	.modify_srq		= fallback_modify_srq,
	.query_srq		= fallback_query_srq,
	.destroy_srq		= fallback_destroy_srq,
	.post_srq_recv		= fallback_post_srq_recv,
	.create_qp		= fallback_create_qp,
	.create_qp_ex		= fallback_create_qp_ex,
	.query_qp		= fallback_query_qp,
	.modify_qp		= fallback_modify_qp,
	.destroy_qp		= fallback_destroy_qp,
	.post_send		= fallback_post_send,
	.post_recv		= fallback_post_recv,
	.create_ah		= fallback_create_ah,
	.destroy_ah		= fallback_destroy_ah,
	.alloc_mw		= fallback_alloc_mw,
	.dealloc_mw		= fallback_dealloc_mw,
};
