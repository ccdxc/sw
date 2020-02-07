// SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB
/*
 * Copyright (c) 2018-2020 Pensando Systems, Inc.  All rights reserved.
 */

#include <linux/printk.h>

#include "ionic_fw.h"
#include "ionic_ibdev.h"

#ifdef IONIC_SRQ_XRC

static int ionic_get_srqid(struct ionic_ibdev *dev, u32 *qpid)
{
	int rc = 0;

	mutex_lock(&dev->inuse_lock);
	rc = ionic_resid_get_shared(&dev->inuse_qpid,
				    dev->size_qpid,
				    dev->next_srqid,
				    dev->size_srqid);
	if (rc >= 0) {
		dev->next_srqid = rc + 1;
		*qpid = rc;
		rc = 0;
	} else {
		rc = ionic_resid_get_shared(&dev->inuse_qpid, 2,
					    dev->inuse_qpid.next_id,
					    dev->size_qpid);
		if (rc >= 0) {
			dev->inuse_qpid.next_id = rc + 1;
			*qpid = rc;
			rc = 0;
		}
	}
	mutex_unlock(&dev->inuse_lock);

	return rc;
}

static void ionic_put_srqid(struct ionic_ibdev *dev, u32 qpid)
{
	ionic_resid_put(&dev->inuse_qpid, qpid);
}

#ifdef HAVE_IB_ALLOC_SRQ_OBJ
static int ionic_create_srq(struct ib_srq *ibsrq,
			    struct ib_srq_init_attr *attr,
			    struct ib_udata *udata)
#else
static struct ib_srq *ionic_create_srq(struct ib_pd *ibpd,
				       struct ib_srq_init_attr *attr,
				       struct ib_udata *udata)
#endif
{
#ifdef HAVE_IB_ALLOC_SRQ_OBJ
	struct ionic_ibdev *dev = to_ionic_ibdev(ibsrq->device);
	struct ionic_ctx *ctx =
		rdma_udata_to_drv_context(udata, struct ionic_ctx, ibctx);
	struct ionic_qp *qp = to_ionic_srq(ibsrq);
#else
	struct ionic_ibdev *dev = to_ionic_ibdev(ibpd->device);
	struct ionic_ctx *ctx = to_ionic_ctx_uobj(ibpd->uobject);
	struct ionic_qp *qp;
#endif
	struct ionic_cq *cq;
	struct ionic_srq_req req;
	struct ionic_srq_resp resp = {0};
	struct ionic_tbl_buf rq_buf = {0};
	unsigned long irqflags;
	int rc;

	if (!ctx) {
		req.rq_spec = ionic_spec;
		rc = ionic_validate_udata(udata, 0, 0);
	} else {
		rc = ionic_validate_udata(udata, sizeof(req), sizeof(resp));
		if (!rc)
			rc = ib_copy_from_udata(&req, udata, sizeof(req));
	}

	if (rc)
		goto err_srq;

#ifndef HAVE_IB_ALLOC_SRQ_OBJ
	qp = kzalloc(sizeof(*qp), GFP_KERNEL);
	if (!qp) {
		rc = -ENOSYS;
		goto err_srq;
	}
#endif

	qp->state = IB_QPS_INIT;

	rc = ionic_get_srqid(dev, &qp->qpid);
	if (rc)
		goto err_srqid;

	qp->has_ah = false;
	qp->has_sq = false;
	qp->has_rq = true;
	qp->is_srq = true;

	spin_lock_init(&qp->rq_lock);

	rc = ionic_qp_rq_init(dev, ctx, qp, &req.rq, &rq_buf,
			      attr->attr.max_wr, attr->attr.max_sge,
			      req.rq_spec, udata);
	if (rc)
		goto err_rq;

	/* TODO SRQ need admin command */
	rc = -ENOSYS;
	goto err_cmd;

	if (ctx) {
		resp.qpid = qp->qpid;

		rc = ib_copy_to_udata(udata, &resp, sizeof(resp));
		if (rc)
			goto err_resp;
	}

	ionic_pgtbl_unbuf(dev, &rq_buf);

	if (ib_srq_has_cq(qp->ibsrq.srq_type)) {
		qp->ibsrq.ext.xrc.srq_num = qp->qpid;

		rc = xa_err(xa_store_irq(&dev->qp_tbl,
					 qp->qpid, qp, GFP_KERNEL));
		if (rc)
			goto err_resp;

#ifdef HAVE_SRQ_EXT_CQ
		cq = to_ionic_cq(attr->ext.cq);
#else
		cq = to_ionic_cq(attr->ext.xrc.cq);
#endif
		spin_lock_irqsave(&cq->lock, irqflags);
		spin_unlock_irqrestore(&cq->lock, irqflags);
	}

	attr->attr.max_wr = qp->rq.mask;
	attr->attr.max_sge =
		ionic_v1_recv_wqe_max_sge(qp->rq.stride_log2,
					  qp->rq_spec);

	ionic_dbg_add_qp(dev, qp);

#ifdef HAVE_IB_ALLOC_SRQ_OBJ
	return 0;
#else
	return &qp->ibsrq;
#endif

err_resp:
	ionic_destroy_qp_cmd(dev, qp->qpid);
err_cmd:
	ionic_pgtbl_unbuf(dev, &rq_buf);
	ionic_qp_rq_destroy(dev, ctx, qp);
err_rq:
	ionic_put_srqid(dev, qp->qpid);
#ifdef HAVE_IB_ALLOC_SRQ_OBJ
err_srqid:
err_srq:
	return rc;
#else
err_srqid:
	kfree(qp);
err_srq:
	return ERR_PTR(rc);
#endif
}

static int ionic_modify_srq(struct ib_srq *ibsrq, struct ib_srq_attr *attr,
			    enum ib_srq_attr_mask mask, struct ib_udata *udata)
{
	return -ENOSYS;
}

static int ionic_query_srq(struct ib_srq *ibsrq, struct ib_srq_attr *attr)
{
	return -ENOSYS;
}

#ifdef HAVE_IB_DESTROY_SRQ_VOID
#ifdef HAVE_IB_API_UDATA
static void ionic_destroy_srq(struct ib_srq *ibsrq, struct ib_udata *udata)
#else
static void ionic_destroy_srq(struct ib_srq *ibsrq)
#endif
#else
static int ionic_destroy_srq(struct ib_srq *ibsrq)
#endif
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibsrq->device);
	struct ionic_ctx *ctx = to_ionic_ctx_uobj(ibsrq->uobject);
	struct ionic_qp *qp = to_ionic_srq(ibsrq);
	struct ionic_cq *cq;
	unsigned long irqflags;
	int rc;

	rc = ionic_destroy_qp_cmd(dev, qp->qpid);
	if (rc) {
		dev_warn(&dev->ibdev.dev, "destroy_srq error %d\n", rc);
#ifdef HAVE_IB_DESTROY_SRQ_VOID
		return;
#else
		return rc;
#endif
	}

	ionic_dbg_rm_qp(qp);

	if (ib_srq_has_cq(qp->ibsrq.srq_type)) {
		xa_erase_irq(&dev->qp_tbl, qp->qpid);

#ifdef HAVE_SRQ_EXT_CQ
		cq = to_ionic_cq(qp->ibsrq.ext.cq);
#else
		cq = to_ionic_cq(qp->ibsrq.ext.xrc.cq);
#endif
		spin_lock_irqsave(&cq->lock, irqflags);
		ionic_clean_cq(cq, qp->qpid);
		list_del(&qp->cq_flush_rq);
		spin_unlock_irqrestore(&cq->lock, irqflags);
	}

	ionic_qp_rq_destroy(dev, ctx, qp);
	ionic_put_srqid(dev, qp->qpid);
#ifndef HAVE_IB_ALLOC_SRQ_OBJ
	kfree(qp);
#endif

#ifndef HAVE_IB_DESTROY_SRQ_VOID
	return 0;
#endif
}

static const struct ib_device_ops ionic_srq_ops = {
#ifdef HAVE_RDMA_DEV_OPS_EXT
	.driver_id		= RDMA_DRIVER_IONIC,
#endif
	.create_srq		= ionic_create_srq,
	.modify_srq		= ionic_modify_srq,
	.query_srq		= ionic_query_srq,
	.destroy_srq		= ionic_destroy_srq,
	.post_srq_recv		= ionic_post_srq_recv,
#ifdef HAVE_IB_ALLOC_SRQ_OBJ
	INIT_RDMA_OBJ_SIZE(ib_srq, ionic_qp, ibsrq),
#endif
};

void ionic_srq_setops(struct ionic_ibdev *dev)
{
	ib_set_device_ops(&dev->ibdev, &ionic_srq_ops);

	dev->ibdev.uverbs_cmd_mask |=
		BIT_ULL(IB_USER_VERBS_CMD_CREATE_SRQ)		|
		BIT_ULL(IB_USER_VERBS_CMD_MODIFY_SRQ)		|
		BIT_ULL(IB_USER_VERBS_CMD_QUERY_SRQ)		|
		BIT_ULL(IB_USER_VERBS_CMD_DESTROY_SRQ)		|
		BIT_ULL(IB_USER_VERBS_CMD_OPEN_XRCD)		|
		BIT_ULL(IB_USER_VERBS_CMD_CLOSE_XRCD)		|
		BIT_ULL(IB_USER_VERBS_CMD_CREATE_XSRQ)		|
		BIT_ULL(IB_USER_VERBS_CMD_OPEN_QP)		|
		0;
}

#endif /* IONIC_SRQ_XRC */
