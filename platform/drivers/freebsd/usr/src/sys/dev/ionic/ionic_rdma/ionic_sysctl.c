// SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB
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

#include "ionic_sysctl.h"
#include "ionic_ibdev.h"

static SYSCTL_NODE(_hw, OID_AUTO, ionic_rdma, CTLFLAG_RD, 0,
                   "Pensando RDMA parameters");

bool ionic_dyndbg_enable = false;
SYSCTL_BOOL(_hw_ionic_rdma, OID_AUTO, dyndbg_enable, CTLFLAG_RWTUN,
    &ionic_dyndbg_enable, 0, "Print to dmesg for dev_dbg, et al");

bool ionic_dbg_enable = true;
SYSCTL_BOOL(_hw_ionic_rdma, OID_AUTO, dbgfs_enable, CTLFLAG_RDTUN,
    &ionic_dbg_enable, 0, "Expose resource info in debug sysctls");

int ionic_sqcmb_order = 5; /* 32 pages */
SYSCTL_INT(_hw_ionic_rdma, OID_AUTO, sqcmb_order, CTLFLAG_RWTUN,
    &ionic_sqcmb_order, 0, "Only alloc SQ in CMB if less than order");

bool ionic_sqcmb_inline = false;
SYSCTL_BOOL(_hw_ionic_rdma, OID_AUTO, sqcmb_inline, CTLFLAG_RWTUN,
    &ionic_sqcmb_inline, 0, "Only alloc SQ in CMB when using inline data");

int ionic_rqcmb_order = 5; /* 32 pages */
SYSCTL_INT(_hw_ionic_rdma, OID_AUTO, rqcmb_order, CTLFLAG_RWTUN,
    &ionic_rqcmb_order, 0, "Only alloc RQ in CMB if less than order");

u16 ionic_aq_depth = 63;
SYSCTL_U16(_hw_ionic_rdma, OID_AUTO, aq_depth, CTLFLAG_RDTUN,
    &ionic_aq_depth, 0, "Min depth for admin queues");

int ionic_aq_count = 4;
SYSCTL_INT(_hw_ionic_rdma, OID_AUTO, aq_count, CTLFLAG_RDTUN,
    &ionic_aq_count, 0, "Limit number of admin queues created");

u16 ionic_eq_depth = 511;
SYSCTL_U16(_hw_ionic_rdma, OID_AUTO, eq_depth, CTLFLAG_RDTUN,
    &ionic_eq_depth, 0, "Min depth for event queues");

u16 ionic_eq_isr_budget = 10;
SYSCTL_U16(_hw_ionic_rdma, OID_AUTO, eq_isr_budget, CTLFLAG_RWTUN,
    &ionic_eq_isr_budget, 0, "Max events to poll per round in isr context");

u16 ionic_eq_work_budget = 1000;
SYSCTL_U16(_hw_ionic_rdma, OID_AUTO, eq_work_budget, CTLFLAG_RWTUN,
    &ionic_eq_work_budget, 0, "Max events to poll per round in work context");

int ionic_max_pd = 1024;
SYSCTL_INT(_hw_ionic_rdma, OID_AUTO, max_pd, CTLFLAG_RDTUN,
    &ionic_max_pd, 0, "Max number of PDs");

static bool ionic_nosupport = false;
SYSCTL_BOOL(_hw_ionic_rdma, OID_AUTO, nosupport,
    CTLFLAG_RWTUN | CTLFLAG_SKIP,
    &ionic_nosupport, 0, "Enable unsupported config values");

static int ionic_spec_sysctl(SYSCTL_HANDLER_ARGS)
{
	int *spec = oidp->oid_arg1;
	int tmp, error;

	error = SYSCTL_OUT(req, spec, sizeof(*spec));
	if (error || !req->newptr)
		return (error);

	error = SYSCTL_IN(req, &tmp, sizeof(tmp));
	if (error)
		return (error);
	if (tmp != 8 && tmp != 16 && !ionic_nosupport) {
		pr_info("ionic_rdma: invalid spec %d, using 8 instead\n", tmp);
		pr_info("ionic_rdma: valid spec values are 8 and 16\n");
		tmp = 8;
	}

	*spec = tmp;

	return (0);
}
int ionic_spec = 16;
SYSCTL_PROC(_hw_ionic_rdma, OID_AUTO, spec, CTLFLAG_RWTUN | CTLTYPE_INT,
    &ionic_spec, 0, ionic_spec_sysctl, "I", "Max SGEs for speculation");

static struct sysctl_oid *ionic_node(struct sysctl_ctx_list *ctx,
				     struct sysctl_oid_list *parent,
				     const char *name, const char *descr)
{
	return SYSCTL_ADD_NODE(ctx, parent, OID_AUTO, name,
			       CTLFLAG_RD | CTLFLAG_MPSAFE,
			       NULL, descr);
}

static struct sysctl_oid *ionic_id_node(struct sysctl_ctx_list *ctx,
					struct sysctl_oid_list *parent,
					u32 id, const char *descr)
{
	char name[8] = { 0 };

	snprintf(name, sizeof(name), "%u", id);

	return ionic_node(ctx, parent, name, descr);
}

static void ionic_string(struct sysctl_ctx_list *ctx,
			 struct sysctl_oid_list *parent,
			 char *ptr,
			 const char *name,
			 const char *descr)
{
	SYSCTL_ADD_STRING(ctx, parent, OID_AUTO, name,
			  CTLFLAG_RD | CTLFLAG_MPSAFE,
			  ptr, 0, descr);
}

static void ionic_int(struct sysctl_ctx_list *ctx,
		      struct sysctl_oid_list *parent,
		      int *ptr,
		      const char *name,
		      const char *descr)
{
	SYSCTL_ADD_INT(ctx, parent, OID_AUTO, name,
		       CTLFLAG_RD | CTLFLAG_MPSAFE,
		       ptr, 0, descr);
}

static void ionic_bool(struct sysctl_ctx_list *ctx,
		       struct sysctl_oid_list *parent,
		       bool *ptr,
		       const char *name,
		       const char *descr)
{
	SYSCTL_ADD_BOOL(ctx, parent, OID_AUTO, name,
			CTLFLAG_RD | CTLFLAG_MPSAFE,
			ptr, 0, descr);
}

static void ionic_u8(struct sysctl_ctx_list *ctx,
		     struct sysctl_oid_list *parent,
		     u8 *ptr,
		     const char *name,
		     const char *descr)
{
	SYSCTL_ADD_U8(ctx, parent, OID_AUTO, name,
		      CTLFLAG_RD | CTLFLAG_MPSAFE,
		      ptr, 0, descr);
}

static void ionic_u16(struct sysctl_ctx_list *ctx,
		      struct sysctl_oid_list *parent,
		      u16 *ptr,
		      const char *name,
		      const char *descr)
{
	SYSCTL_ADD_U16(ctx, parent, OID_AUTO, name,
		       CTLFLAG_RD | CTLFLAG_MPSAFE,
		       ptr, 0, descr);
}

static void ionic_u32(struct sysctl_ctx_list *ctx,
		      struct sysctl_oid_list *parent,
		      u32 *ptr,
		      const char *name,
		      const char *descr)
{
	SYSCTL_ADD_U32(ctx, parent, OID_AUTO, name,
		       CTLFLAG_RD | CTLFLAG_MPSAFE,
		       ptr, 0, descr);
}

static void ionic_u64(struct sysctl_ctx_list *ctx,
		      struct sysctl_oid_list *parent,
		      u64 *ptr,
		      const char *name,
		      const char *descr)
{
	SYSCTL_ADD_U64(ctx, parent, OID_AUTO, name,
		       CTLFLAG_RD | CTLFLAG_MPSAFE,
		       ptr, 0, descr);
}

static void ionic_ulong(struct sysctl_ctx_list *ctx,
			struct sysctl_oid_list *parent,
			unsigned long *ptr,
			const char *name,
			const char *descr)
{
	SYSCTL_ADD_ULONG(ctx, parent, OID_AUTO, name,
			 CTLFLAG_RD | CTLFLAG_MPSAFE,
			 ptr, descr);
}

static void ionic_bytes(struct sysctl_ctx_list *ctx,
			struct sysctl_oid_list *parent,
			void *ptr, int size,
			const char *name, const char *descr)
{
	SYSCTL_ADD_OPAQUE(ctx, parent, OID_AUTO, name,
			  CTLFLAG_RD | CTLFLAG_MPSAFE,
			  ptr, size, "CU", descr);
}

static int ionic_sysctl_hweight(SYSCTL_HANDLER_ARGS)
{
	unsigned long *bitmap;
	int size, weight;

	bitmap = arg1;
	size = arg2;

	weight = bitmap_weight(bitmap, size);

	return sysctl_handle_int(oidp, &weight, 0, req);
}

static void ionic_hweight(struct sysctl_ctx_list *ctx,
			  struct sysctl_oid_list *parent,
			  unsigned long *bitmap, int size,
			  const char *name, const char *descr)
{
	SYSCTL_ADD_PROC(ctx, parent, OID_AUTO, name,
			CTLTYPE_UINT | CTLFLAG_RD | CTLFLAG_MPSAFE,
			bitmap, size,
			ionic_sysctl_hweight,
			"IU", descr);
}

static int ionic_sysctl_ioread32(SYSCTL_HANDLER_ARGS)
{
	void __iomem *reg;
	int val;

	reg = arg1;

	val = ioread32(reg);

	return sysctl_handle_int(oidp, &val, 0, req);
}

static void ionic_ioread32(struct sysctl_ctx_list *ctx,
			   struct sysctl_oid_list *parent,
			   u32 __iomem *ptr,
			   const char *name, const char *descr)
{
	SYSCTL_ADD_PROC(ctx, parent, OID_AUTO, name,
			CTLTYPE_UINT | CTLFLAG_RD | CTLFLAG_MPSAFE,
			ptr, 0,
			ionic_sysctl_ioread32,
			"IU", descr);
}

typedef int (*ionic_ctrl_handler)(void *context,
				  const char *buf, size_t count);

static int ionic_sysctl_ctrl(SYSCTL_HANDLER_ARGS)
{
	ionic_ctrl_handler handle_ctrl;
	char buf[256] = {0};
	int rc;

	rc = sysctl_handle_string(oidp, buf, sizeof(buf), req);
	if (rc || req->newptr == NULL)
		return rc;

	handle_ctrl = (void *)arg2;

	return handle_ctrl(arg1, buf, strlen(buf));
}

static void ionic_ctrl(struct sysctl_ctx_list *ctx,
		       struct sysctl_oid_list *parent,
		       void *context, ionic_ctrl_handler handle_ctrl,
		       const char *name, const char *descr)
{
	SYSCTL_ADD_PROC(ctx, parent, OID_AUTO, name,
			CTLTYPE_STRING | CTLFLAG_WR,
			context, (intmax_t)handle_ctrl,
			ionic_sysctl_ctrl,
			"A", descr);
}

static void ionic_umem_add(struct sysctl_ctx_list *ctx,
			   struct sysctl_oid_list *parent,
			   struct ib_umem *umem)
{
	struct sysctl_oid *oidp;

	oidp = ionic_node(ctx, parent, "umem", "User Memory");
	if (!oidp)
		return;

	parent = SYSCTL_CHILDREN(oidp);

	ionic_ulong(ctx, parent, &umem->length, "length", "Length");
	ionic_ulong(ctx, parent, &umem->address, "address", "Address");
	ionic_int(ctx, parent, &umem->page_size, "page_size", "Page Size");
	ionic_int(ctx, parent, &umem->nmap, "nmap", "Num Mappings");
}

static void ionic_tbl_res_add(struct sysctl_ctx_list *ctx,
			      struct sysctl_oid_list *parent,
			      struct ionic_tbl_res *res)
{
	ionic_int(ctx, parent, &res->tbl_order, "tbl_order", "Order Reserved");
	ionic_int(ctx, parent, &res->tbl_pos, "tbl_pos", "Start Position");
}

static void ionic_tbl_buf_add(struct sysctl_ctx_list *ctx,
			      struct sysctl_oid_list *parent,
			      struct ionic_tbl_buf *buf)
{
	ionic_int(ctx, parent, &buf->tbl_limit, "tbl_limit", "PgTbl Capacity");
	ionic_int(ctx, parent, &buf->tbl_pages, "tbl_pages", "PgTbl Pages");
	ionic_ulong(ctx, parent, &buf->tbl_size, "tbl_size", "Buffer Size");
	ionic_ulong(ctx, parent, &buf->tbl_dma, "tbl_dma", "Buffer DMA Addr");
	ionic_u8(ctx, parent, &buf->page_size_log2,
		 "page_size_log2", "Page Size (log2)");

	if (buf->tbl_buf)
		ionic_bytes(ctx, parent, buf->tbl_buf, buf->tbl_size,
			    "buf", "Raw Table Buffer");
}

static void ionic_q_add(struct sysctl_ctx_list *ctx,
			struct sysctl_oid_list *parent,
			struct ionic_queue *q,
			struct ionic_tbl_res *res,
			struct ib_umem *umem,
			const char *name,
			const char *descr)
{
	struct sysctl_oid *oidp;

	oidp = ionic_node(ctx, parent, name, descr);
	if (!oidp)
		return;

	parent = SYSCTL_CHILDREN(oidp);

	ionic_ulong(ctx, parent, &q->size, "size", "Size (bytes)");
	ionic_ulong(ctx, parent, &q->dma, "dma", "DMA Address");
	ionic_u8(ctx, parent, &q->depth_log2, "depth_log2", "Depth (log2)");
	ionic_u8(ctx, parent, &q->stride_log2, "stride_log2","Stride (log2)");
	ionic_u64(ctx, parent, &q->dbell, "dbell", "Doorbell Template");

	if (q->ptr) {
		ionic_u16(ctx, parent, &q->prod, "prod", "Producer Index");
		ionic_u16(ctx, parent, &q->cons, "cons", "Consumer Index");
		ionic_u16(ctx, parent, &q->mask, "mask", "Index Bitmask");
		ionic_bytes(ctx, parent, q->ptr, q->size,
			    "buf", "Raw Queue Buffer");
	}

	if (res)
		ionic_tbl_res_add(ctx, parent, res);

	if (umem)
		ionic_umem_add(ctx, parent, umem);
}

void ionic_dbg_add_dev(struct ionic_ibdev *dev, struct sysctl_oid *oidp)
{
	struct sysctl_ctx_list *ctx;
	struct sysctl_oid_list *parent;

	dev->debug = NULL;
	dev->debug_aq = NULL;
	dev->debug_cq = NULL;
	dev->debug_eq = NULL;
	dev->debug_mr = NULL;
	dev->debug_qp = NULL;

	if (!oidp)
		return;

	parent = SYSCTL_CHILDREN(oidp);

	ctx = &dev->debug_ctx;
	sysctl_ctx_init(ctx);

	oidp = ionic_node(ctx, parent, "rdma", "RDMA Device");
	if (!oidp)
		return;

	dev->debug = oidp;

	parent = SYSCTL_CHILDREN(oidp);

	dev->debug_aq = ionic_node(ctx, parent, "aq", "AQ Info");
	dev->debug_cq = ionic_node(ctx, parent, "cq", "CQ Info");
	dev->debug_eq = ionic_node(ctx, parent, "eq", "EQ Info");
	dev->debug_mr = ionic_node(ctx, parent, "mr", "MR/MW Info");
	dev->debug_qp = ionic_node(ctx, parent, "qp", "QP/SRQ Info");
}

static int ionic_dev_reset_write(void *context, const char *buf, size_t count)
{
	struct ionic_ibdev *dev = context;

	if (strcmp(buf, "1") && strcmp(buf, "1\n"))
		return -EINVAL;

	dev_warn(&dev->ibdev.dev, "resetting...\n");
	ionic_ibdev_reset(dev);
	return 0;
}

void ionic_dbg_add_dev_info(struct ionic_ibdev *dev)
{
	struct sysctl_ctx_list *ctx;
	struct sysctl_oid_list *parent;
	struct sysctl_oid *oidp;
	char cname[20];
	int i;

	if (!dev->debug)
		return;

	oidp = dev->debug;
	parent = SYSCTL_CHILDREN(oidp);

	ctx = &dev->debug_ctx;

	ionic_ctrl(ctx, parent, dev, ionic_dev_reset_write,
		   "reset", "Device Reset (write)");

	oidp = ionic_node(ctx, parent, "info", "Rdma Device Info");
	if (!oidp)
		return;

	parent = SYSCTL_CHILDREN(oidp);

	ionic_int(ctx, parent, &dev->lif_index, "lif_index", "LIF Index");
	ionic_int(ctx, parent, &dev->dbid, "dbid", "Doorbell ID");
	ionic_u16(ctx, parent, &dev->rdma_version, "rdma_version",
		  "RDMA FW ABI");
	ionic_u8(ctx, parent, &dev->qp_opcodes, "qp_opcodes", "QP Opcodes");
	ionic_u8(ctx, parent, &dev->admin_opcodes, "admin_opcodes",
		 "Admin Opcodes");
	ionic_u32(ctx, parent, &dev->reset_cnt, "reset_cnt", "Reset Count");

	ionic_u32(ctx, parent, &dev->aq_base, "aq_base", "Admin QID Base");
	ionic_u32(ctx, parent, &dev->cq_base, "cq_base",
		  "Completion QID Base");
	ionic_u32(ctx, parent, &dev->eq_base, "eq_base", "Event QID Base");

	ionic_u32(ctx, parent, &dev->aq_count, "aq_count", "Admin Q Count");
	ionic_u32(ctx, parent, &dev->eq_count, "eq_count", "Event Q Count");

	ionic_u8(ctx, parent, &dev->aq_qtype, "aq_qtype", "Admin QType");
	ionic_u8(ctx, parent, &dev->sq_qtype, "sq_qtype", "Send QType");
	ionic_u8(ctx, parent, &dev->rq_qtype, "rq_qtype", "Recv QType");
	ionic_u8(ctx, parent, &dev->cq_qtype, "cq_qtype", "Completion QType");
	ionic_u8(ctx, parent, &dev->eq_qtype, "eq_qtype", "Event QType");

	ionic_u8(ctx, parent, &dev->max_stride, "max_stride", "Max Stride");
	ionic_u8(ctx, parent, &dev->cl_stride, "cl_stride", "CL Stride");
	ionic_u8(ctx, parent, &dev->pte_stride, "pte_stride", "PT Stride");
	ionic_u8(ctx, parent, &dev->rrq_stride, "rrq_stride", "RRQ Stride");
	ionic_u8(ctx, parent, &dev->rsq_stride, "rsq_stride", "RSQ Stride");

	ionic_int(ctx, parent, (int *)&dev->admin_state, "admin_state",
		  "AQ State");

	ionic_hweight(ctx, parent,
		      dev->inuse_pdid.inuse,
		      dev->inuse_pdid.inuse_size,
		      "inuse_pdid", "In-use PD IDs");
	ionic_int(ctx, parent, &dev->inuse_pdid.inuse_size,
		  "size_pdid", "Total PD IDs");
	ionic_int(ctx, parent, &dev->inuse_pdid.next_id,
		  "next_pdid", "Next PD ID");

	ionic_hweight(ctx, parent,
		      dev->inuse_mrid.inuse,
		      dev->inuse_mrid.inuse_size,
		      "inuse_mrid", "In-use MR IDs");
	ionic_int(ctx, parent, &dev->inuse_mrid.inuse_size,
		  "size_mrid", "Total MR IDs");
	ionic_int(ctx, parent, &dev->inuse_mrid.next_id,
		  "next_mrid", "Next MR ID");
	ionic_u8(ctx, parent, &dev->next_mrkey,
		 "next_mrkey", "Next MR KEY");

	ionic_hweight(ctx, parent,
		      dev->inuse_cqid.inuse,
		      dev->inuse_cqid.inuse_size,
		      "inuse_cqid", "In-use CQ IDs");
	ionic_int(ctx, parent, &dev->inuse_cqid.inuse_size,
		  "size_cqid", "Total CQ IDs");
	ionic_int(ctx, parent, &dev->inuse_cqid.next_id,
		  "next_cqid", "Next CQ ID");

	ionic_hweight(ctx, parent,
		      dev->inuse_qpid.inuse,
		      dev->size_qpid,
		      "inuse_qpid", "In-use QP IDs");
	ionic_int(ctx, parent, &dev->size_qpid,
		  "size_qpid", "Total QP IDs");
	ionic_int(ctx, parent, &dev->inuse_qpid.next_id,
		  "next_qpid", "Next QP ID");

	ionic_hweight(ctx, parent,
		      dev->inuse_qpid.inuse,
		      dev->size_srqid,
		      "inuse_srqid", "In-use SRQ IDs");
	ionic_int(ctx, parent, &dev->size_srqid,
		  "size_srqid", "Total SRQ IDs");
	ionic_int(ctx, parent, &dev->next_srqid,
		  "next_srqid", "Next SRQ ID");

	ionic_hweight(ctx, parent,
		      dev->inuse_restbl.inuse,
		      dev->inuse_restbl.inuse_size,
		      "inuse_restbl", "In-use Resource Table");
	ionic_int(ctx, parent, &dev->inuse_restbl.inuse_size,
		  "size_restbl", "Size of Table");
	ionic_int(ctx, parent, &dev->inuse_restbl.order_max,
		  "order_restbl", "Max Order");

	for (i = 0; i < dev->inuse_restbl.order_max; ++i) {
		snprintf(cname, sizeof(cname), "next_restbl_%d", i);
		ionic_int(ctx, parent, &dev->inuse_restbl.order_next[i],
			  cname, "Order Next Long");
	}
}

void ionic_dbg_rm_dev(struct ionic_ibdev *dev)
{
	if (dev->debug)
		sysctl_ctx_free(&dev->debug_ctx);

	dev->debug = NULL;
	dev->debug_aq = NULL;
	dev->debug_cq = NULL;
	dev->debug_eq = NULL;
	dev->debug_mr = NULL;
	dev->debug_qp = NULL;
}

void ionic_dbg_add_eq(struct ionic_ibdev *dev, struct ionic_eq *eq)
{
	struct sysctl_ctx_list *ctx;
	struct sysctl_oid_list *parent;
	struct sysctl_oid *oidp;
	struct ionic_intr __iomem *intr;

	eq->debug = NULL;

	if (!dev->debug_eq)
		return;

	oidp = dev->debug_eq;
	parent = SYSCTL_CHILDREN(oidp);

	ctx = &eq->debug_ctx;
	sysctl_ctx_init(ctx);

	oidp = ionic_id_node(ctx, parent, eq->eqid, "EQ Info");
	if (!oidp)
		return;

	parent = SYSCTL_CHILDREN(oidp);

	eq->debug = oidp;

	ionic_u32(ctx, parent, &eq->eqid, "eqid", "EQ ID");
	ionic_u32(ctx, parent, &eq->intr, "intr", "EQ INTR");

	ionic_q_add(ctx, parent, &eq->q, NULL, NULL, "q", "Event Queue");

	ionic_bool(ctx, parent, &eq->enable, "enable", "EQ Enabled");
	ionic_bool(ctx, parent, &eq->armed, "armed", "EQ Armed");
	ionic_int(ctx, parent, &eq->irq, "irq", "EQ IRQ");
	ionic_string(ctx, parent, eq->name, "name", "EQ ISR Name");

	intr = &eq->dev->intr_ctrl[eq->intr];
	ionic_ioread32(ctx, parent, &intr->coal_init,
		       "intr_coalesce_init", "Read Intr Coal Init");
	ionic_ioread32(ctx, parent, &intr->mask,
		       "intr_mask", "Read Intr Mask");
	ionic_ioread32(ctx, parent, &intr->credits,
		       "intr_credits", "Read Intr Credits");
	ionic_ioread32(ctx, parent, &intr->mask_assert,
		       "intr_mask_assert", "Read Intr Mask Assert");
	ionic_ioread32(ctx, parent, &intr->coal,
		       "intr_coalesce", "Read Intr Coalesce");
}

void ionic_dbg_rm_eq(struct ionic_eq *eq)
{
	if (eq->debug)
		sysctl_ctx_free(&eq->debug_ctx);

	eq->debug = NULL;
}

void ionic_dbg_add_mr(struct ionic_ibdev *dev, struct ionic_mr *mr)
{
	struct sysctl_ctx_list *ctx;
	struct sysctl_oid_list *parent;
	struct sysctl_oid *oidp;

	mr->debug = NULL;

	if (!dev->debug_mr)
		return;

	oidp = dev->debug_mr;
	parent = SYSCTL_CHILDREN(oidp);

	ctx = &mr->debug_ctx;
	sysctl_ctx_init(ctx);

	oidp = ionic_id_node(ctx, parent, ionic_mrid_index(mr->mrid),
			     "MR Info");
	if (!oidp)
		return;

	parent = SYSCTL_CHILDREN(oidp);

	mr->debug = oidp;

	ionic_u32(ctx, parent, &mr->mrid, "mrid", "MR ID");

	ionic_tbl_res_add(ctx, parent, &mr->res);

	ionic_tbl_buf_add(ctx, parent, &mr->buf);

	if (mr->umem)
		ionic_umem_add(ctx, parent, mr->umem);
}

void ionic_dbg_rm_mr(struct ionic_mr *mr)
{
	if (mr->debug)
		sysctl_ctx_free(&mr->debug_ctx);

	mr->debug = NULL;
}

void ionic_dbg_add_cq(struct ionic_ibdev *dev, struct ionic_cq *cq)
{
	struct sysctl_ctx_list *ctx;
	struct sysctl_oid_list *parent;
	struct sysctl_oid *oidp;

	cq->debug = NULL;

	if (!dev->debug_cq)
		return;

	oidp = dev->debug_cq;
	parent = SYSCTL_CHILDREN(oidp);

	ctx = &cq->debug_ctx;
	sysctl_ctx_init(ctx);

	oidp = ionic_id_node(ctx, parent, cq->cqid, "CQ Info");
	if (!oidp)
		return;

	parent = SYSCTL_CHILDREN(oidp);

	cq->debug = oidp;

	ionic_u32(ctx, parent, &cq->cqid, "cqid", "CQ ID");
	ionic_u32(ctx, parent, &cq->eqid, "eqid", "EQ ID");
	ionic_bool(ctx, parent, &cq->color, "color", "CQ Color");

	ionic_q_add(ctx, parent, &cq->q, &cq->res, cq->umem,
		    "q", "Completion Queue");
}

void ionic_dbg_rm_cq(struct ionic_cq *cq)
{
	if (cq->debug)
		sysctl_ctx_free(&cq->debug_ctx);

	cq->debug = NULL;
}

struct ionic_dbg_admin_wr {
	struct ionic_aq *aq;
	struct ionic_admin_wr wr;
	void *data;
	dma_addr_t dma;
};

static int match_whole_prefix(const char *str, const char *pfx)
{
	int pos = 0;

	while (pfx[pos]) {
		if (pfx[pos] != str[pos])
			return 0;
		++pos;
	}

	return pos;
}

static int ionic_aq_ctrl_write(void *context, const char *buf, size_t count)
{
	struct ionic_aq *aq = context;
	struct ionic_dbg_admin_wr *wr =
		container_of(aq->debug_wr, struct ionic_dbg_admin_wr, wr);
	long timeout;
	int val, num, pos = 0, rc = 0;

	if (buf[count]) {
		rc = -EINVAL;
		goto out;
	}

	while (pos < count) {
		if (isspace(buf[pos])) {
			++pos;
			continue;
		}

		num = match_whole_prefix(buf + pos, "post");
		if (num) {
			pos += num;

			reinit_completion(&wr->wr.work);

			ionic_admin_post_aq(aq, &wr->wr);

			timeout = wait_for_completion_interruptible_timeout(
							     &wr->wr.work, HZ);
			if (timeout > 0)
				rc = 0;
			else if (timeout == 0)
				rc = -ETIMEDOUT;
			else
				rc = timeout;

			if (rc) {
				dev_warn(&aq->dev->ibdev.dev, "wait %d\n", rc);
				ionic_admin_cancel(&wr->wr);
				goto out;
			} else if (wr->wr.status == IONIC_ADMIN_KILLED) {
				dev_dbg(&aq->dev->ibdev.dev, "killed\n");
				rc = 0;
				goto out;
			} else if (ionic_v1_cqe_error(&wr->wr.cqe)) {
				dev_warn(&aq->dev->ibdev.dev, "cqe error %u\n",
					 be32_to_cpu(wr->wr.cqe.status_length));
				rc = -EINVAL;
				goto out;
			}
			continue;
		}

		num = match_whole_prefix(buf + pos, "tbl");
		if (num) {
			pos += num;

			rc = sscanf(buf + pos, " %d%n", &val, &num);
			if (rc != 1) {
				rc = -EINVAL;
				goto out;
			}

			pos += num;

			wr->wr.wqe.type_state = val;
			continue;
		}

		num = match_whole_prefix(buf + pos, "idx");
		if (rc == 1) {
			pos += num;

			rc = sscanf(buf + pos, " %d%n", &val, &num);
			if (rc != 1) {
				rc = -EINVAL;
				goto out;
			}

			pos += num;

			wr->wr.wqe.id_ver = cpu_to_le32(val);
			continue;
		}

		rc = -EINVAL;
		goto out;
	}

	rc = 0;
out:

	return -rc;
}

void ionic_dbg_add_aq(struct ionic_ibdev *dev, struct ionic_aq *aq)
{
	struct ionic_dbg_admin_wr *wr;
	struct sysctl_ctx_list *ctx;
	struct sysctl_oid_list *parent;
	struct sysctl_oid *oidp;

	aq->debug = NULL;
	aq->debug_wr = NULL;

	if (!dev->debug_aq)
		return;

	oidp = dev->debug_aq;
	parent = SYSCTL_CHILDREN(oidp);

	ctx = &aq->debug_ctx;
	sysctl_ctx_init(ctx);

	oidp = ionic_id_node(ctx, parent, aq->aqid, "AQ Info");
	if (!oidp)
		return;

	parent = SYSCTL_CHILDREN(oidp);

	aq->debug = oidp;

	ionic_bool(ctx, parent, &aq->armed, "armed", "AQ Armed");
	ionic_u32(ctx, parent, &aq->aqid, "aqid", "AQ ID");
	ionic_u32(ctx, parent, &aq->cqid, "cqid", "AQ CQ ID");

	ionic_q_add(ctx, parent, &aq->q, NULL, NULL,
		    "q", "RDMA Admin Queue");

	wr = kzalloc(sizeof(*wr), GFP_KERNEL);
	if (!wr)
		goto err_wr;

	wr->data = kzalloc(PAGE_SIZE, GFP_KERNEL);
	if (!wr->data)
		goto err_data;

	wr->dma = dma_map_single(dev->hwdev, wr->data, PAGE_SIZE,
				 DMA_FROM_DEVICE);
	if (dma_mapping_error(dev->hwdev, wr->dma))
		goto err_dma;

	wr->wr.wqe.op = IONIC_V1_ADMIN_DEBUG;
	wr->wr.wqe.stats.dma_addr = cpu_to_le64(wr->dma);
	wr->wr.wqe.stats.length = cpu_to_le32(PAGE_SIZE);

	init_completion(&wr->wr.work);

	aq->debug_wr = &wr->wr;

	ionic_bytes(ctx, parent, &wr->wr.wqe, sizeof(wr->wr.wqe),
		    "dbg_wr_wqe", "Debug WR WQE");
	ionic_bytes(ctx, parent, &wr->wr.cqe, sizeof(wr->wr.cqe),
		    "dbg_wr_cqe", "Debug WR CQE");
	ionic_bytes(ctx, parent, wr->data, PAGE_SIZE,
		    "dbg_wr_data", "Debug WR Data Page");
	ionic_ctrl(ctx, parent, aq, ionic_aq_ctrl_write,
		   "dbg_wr_ctrl", "Debug Control (write)");

	return;

err_dma:
	kfree(wr->data);
err_data:
	kfree(wr);
err_wr:
	return;
}

void ionic_dbg_rm_aq(struct ionic_aq *aq)
{
	struct ionic_ibdev *dev = aq->dev;
	struct ionic_dbg_admin_wr *wr;

	if (aq->debug)
		sysctl_ctx_free(&aq->debug_ctx);

	aq->debug = NULL;

	if (!aq->debug_wr)
		return;

	wr = container_of(aq->debug_wr, struct ionic_dbg_admin_wr, wr);

	dma_unmap_single(dev->hwdev, wr->dma, PAGE_SIZE, DMA_FROM_DEVICE);
	kfree(wr->data);
	kfree(wr);
}

void ionic_dbg_add_qp(struct ionic_ibdev *dev, struct ionic_qp *qp)
{
	struct sysctl_ctx_list *ctx;
	struct sysctl_oid_list *parent;
	struct sysctl_oid *oidp;

	qp->debug = NULL;

	if (!dev->debug_qp)
		return;

	oidp = dev->debug_qp;
	parent = SYSCTL_CHILDREN(oidp);

	ctx = &qp->debug_ctx;
	sysctl_ctx_init(ctx);

	oidp = ionic_id_node(ctx, parent, qp->qpid, "QP Info");
	if (!oidp)
		return;

	parent = SYSCTL_CHILDREN(oidp);

	qp->debug = oidp;

	ionic_u32(ctx, parent, &qp->qpid, "qpid", "QP ID");
	ionic_int(ctx, parent, (int *)&qp->state, "state", "QP State");

	if (qp->has_sq) {
		ionic_q_add(ctx, parent, &qp->sq, &qp->sq_res, qp->sq_umem,
			    "sq", "Send Queue");

		ionic_u16(ctx, parent, &qp->sq_msn_prod,
			  "sq_msn_prod", "SQ MSN Producer Index");
		ionic_u16(ctx, parent, &qp->sq_msn_cons,
			  "sq_msn_cons", "SQ MSN Consumer Index");

		ionic_bool(ctx, parent, &qp->sq_is_cmb,
			   "sq_is_cmb", "SQ in Ctrl Mem");
		if (qp->sq_is_cmb) {
			ionic_int(ctx, parent, &qp->sq_cmb_order,
				  "sq_cmb_order", "SQCMB Order Reserved");
			ionic_int(ctx, parent, &qp->sq_cmb_pgid,
				  "sq_cmb_pgid", "SQCMB Start Page");
			ionic_ulong(ctx, parent, &qp->sq_cmb_addr,
				    "sq_cmb_addr", "SQCMB Phys Addr");
		}

		ionic_bool(ctx, parent, &qp->sq_flush,
			   "sq_flush", "SQ Flush");
		ionic_bool(ctx, parent, &qp->sq_flush_rcvd,
			   "sq_flush_rcvd", "SQ Flush CQE Received");
		ionic_int(ctx, parent, &qp->sq_spec,
			  "sq_spec", "SQ Spec Configuration");
		ionic_u32(ctx, parent, &qp->sq_cqid,
			  "sq_cqid", "SQ CQ ID");
	}

	if (qp->has_rq) {
		ionic_q_add(ctx, parent, &qp->rq, &qp->rq_res, qp->rq_umem,
			    "rq", "Recv Queue");

		ionic_bool(ctx, parent, &qp->rq_is_cmb,
			   "rq_is_cmb", "RQ in Ctrl Mem");
		if (qp->rq_is_cmb) {
			ionic_int(ctx, parent, &qp->rq_cmb_order,
				  "rq_cmb_order", "RQCMB Order Reserved");
			ionic_int(ctx, parent, &qp->rq_cmb_pgid,
				  "rq_cmb_pgid", "RQCMB Start Page");
			ionic_ulong(ctx, parent, &qp->rq_cmb_addr,
				    "rq_cmb_addr", "RQCMB Phys Addr");
		}

		ionic_bool(ctx, parent, &qp->rq_flush,
			   "rq_flush", "RQ Flush");
		ionic_int(ctx, parent, &qp->rq_spec,
			  "rq_spec", "RQ Spec Configuration");
		ionic_u32(ctx, parent, &qp->rq_cqid,
			  "rq_cqid", "RQ CQ ID");
	}

	ionic_int(ctx, parent, &qp->dcqcn_profile,
		  "dcqcn_profile", "DCQCN Profile");
}

void ionic_dbg_rm_qp(struct ionic_qp *qp)
{
	if (qp->debug)
		sysctl_ctx_free(&qp->debug_ctx);

	qp->debug = NULL;
}
