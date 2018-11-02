/*
 * Copyright (c) 2018 Pensando Systems, Inc.  All rights reserved.
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

#ifndef IONIC_IBDEV_H
#define IONIC_IBDEV_H

#include <linux/device.h>
#include <linux/netdevice.h>
#include <rdma/ib_umem.h>
#include <rdma/ib_verbs.h>
#include <rdma/ib_pack.h>

#include <rdma/ionic-abi.h>
#include <ionic_api.h>

#include "ionic_kcompat.h"
#include "ionic_fw.h"
#include "ionic_queue.h"
#include "table.h"

#define IONIC_MIN_RDMA_VERSION 0
#define IONIC_MAX_RDMA_VERSION 1

#define IONIC_DBPAGE_SIZE	0x1000
#define IONIC_MAX_MRID		0xffffff
#define IONIC_MAX_QPID		0xffffff
#define IONIC_MAX_CMB_ORDER	15

#define IONIC_META_LAST ((void *)1ul)
#define IONIC_META_POSTED ((void *)2ul)

#define IONIC_CQ_GRACE		100

struct ionic_aq;
struct ionic_cq;
struct ionic_eq;

enum ionic_admin_state {
	IONIC_ADMIN_ACTIVE, /* subbmitting admin commands to queue */
	IONIC_ADMIN_PAUSED, /* not submitting, but may complete normally */
	IONIC_ADMIN_KILLED, /* not submitting, fake normal completion */
	IONIC_ADMIN_FAILED, /* not submitting, failed completion */
};

struct ionic_mmap_info {
	struct list_head ctx_ent;
	unsigned long offset;
	unsigned long size;
	unsigned long pfn;
	bool writecombine;
};

struct ionic_ibdev {
	struct ib_device	ibdev;

	struct list_head	driver_ent;

	struct ib_device_attr	dev_attr;
	struct ib_port_attr	port_attr;

	struct device		*hwdev;
	struct net_device	*ndev;
	struct lif		*lif;
	int			lif_id;

	u32			dbid;
	phys_addr_t		xxx_dbpage_phys;
	u64		__iomem *dbpage;
	u32		__iomem *intr_ctrl;

	u16			rdma_version;
	u8			rdma_compat;
	u8			qp_opcodes;
	u8			admin_opcodes;

	u8			admin_qtype;
	u8			sq_qtype;
	u8			rq_qtype;
	u8			cq_qtype;
	u8			eq_qtype;

	u8			max_stride;
	u8			cl_stride;
	u8			pte_stride;
	u8			rrq_stride;
	u8			rsq_stride;

	struct mutex		tbl_lock; /* for modify cq_tbl, qp_tbl */

	struct tbl_root		qp_tbl;
	struct tbl_root		cq_tbl;

	spinlock_t		inuse_lock; /* for id reservation */

	unsigned long		*inuse_pdid;
	u32			size_pdid;
	u32			next_pdid;

	unsigned long		*inuse_ahid;
	u32			size_ahid;
	u32			next_ahid;

	unsigned long		*inuse_mrid;
	u32			size_mrid;
	u32			next_mrid;
	u8			next_mrkey;

	unsigned long		*inuse_cqid;
	u32			size_cqid;
	u32			next_cqid;

	unsigned long		*inuse_qpid;
	u32			size_qpid;
	u32			next_qpid;
	u32			size_srqid;
	u32			next_srqid;

	unsigned long		*inuse_restbl;
	u32			size_restbl;

	struct work_struct	admin_work;
	spinlock_t		admin_lock;
	struct ionic_aq		*adminq;
	struct ionic_cq		*admincq;
	bool			admin_armed;
	enum ionic_admin_state	admin_state;

	struct ionic_eq		**eq_vec;
	int			eq_count;

	int			stats_count;
	size_t			stats_size;
	char			*stats_buf;
	const char		**stats_hdrs;

	struct dentry		*debug;
	struct dentry		*debug_ah;
	struct dentry		*debug_aq;
	struct dentry		*debug_cq;
	struct dentry		*debug_eq;
	struct dentry		*debug_mr;
	struct dentry		*debug_mw;
	struct dentry		*debug_pd;
	struct dentry		*debug_qp;
	struct dentry		*debug_srq;
};

struct ionic_eq {
	struct ionic_ibdev	*dev;

	u32			eqid;
	u32			intr;

	struct ionic_queue	q;

	bool			enable;
	bool			armed;

	struct work_struct	work;

	int			irq;
	char			name[32];

	struct dentry		*debug;
};

struct ionic_admin_wr {
	struct completion	work;
	struct list_head	aq_ent;
	struct ionic_v1_admin_wqe wqe;
	struct ionic_v1_cqe	cqe;
	int			status;
};

struct ionic_aq {
	struct ionic_ibdev	*dev;

	u32			aqid;
	u32			cqid;

	spinlock_t		lock; /* for posting */
	struct ionic_queue	q;
	struct ionic_admin_wr	**q_wr;
	struct list_head	wr_prod;
	struct list_head	wr_post;

	struct dentry		*debug;
	struct ionic_admin_wr	*debug_wr;
};

struct ionic_ctx {
	struct ib_ucontext	ibctx;

	/* fallback to kernel-impl verbs for this user context */
	bool			fallback;

	u32			dbid;

	struct mutex		mmap_mut; /* for mmap_list */
	unsigned long long	mmap_off;
	struct list_head	mmap_list;
	struct ionic_mmap_info	mmap_dbell;
};

struct ionic_tbl_res {
	int			tbl_order;
	int			tbl_pos;
};

struct ionic_tbl_buf {
	u32			tbl_limit;
	u32			tbl_pages;
	size_t			tbl_size;
	__le64			*tbl_buf;
	dma_addr_t		tbl_dma;
	u8			page_size_log2;
};

struct ionic_pd {
	struct ib_pd		ibpd;

	u32			pdid;
};

struct ionic_cq {
	struct ib_cq		ibcq;

	u32			cqid;
	u32			eqid;

	spinlock_t		lock; /* for polling */
	struct list_head	poll_sq;
	bool			flush;
	struct list_head	flush_sq;
	struct list_head	flush_rq;
	struct ionic_queue	q;
	bool			color;
	int			reserve;
	u16			arm_any_prod;
	u16			arm_sol_prod;

	/* infrequently accessed, keep at end */
	struct ib_umem		*umem;
	struct ionic_tbl_res	res;

	u8			compat;

	struct dentry		*debug;

	/* XXX xxx_notify */
	struct delayed_work	notify_work;
};

struct ionic_sq_meta {
	u64			wrid;
	u32			len;
	u16			seq;
	u8			ibop;
	u8			ibsts;
	bool			remote;
	bool			signal;
};

struct ionic_rq_meta {
	struct ionic_rq_meta	*next;
	u64			wrid;
	u32			len; /* XXX byte_len must come from cqe */
};

struct ionic_qp {
	union {
		struct ib_qp	ibqp;
		struct ib_srq	ibsrq;
	};
	enum ib_qp_state	state;

	u32			qpid;
	u32			ahid;

	bool			has_ah;
	bool			has_sq;
	bool			has_rq;
	bool			is_srq;

	bool			sig_all;

	struct list_head	cq_poll_sq;
	struct list_head	cq_flush_sq;
	struct list_head	cq_flush_rq;

	spinlock_t		sq_lock; /* for posting and polling */
	bool			sq_flush;
	struct ionic_queue	sq;
	void		__iomem *sq_cmb_ptr;
	struct ionic_sq_meta	*sq_meta;
	u16			*sq_msn_idx;

	u16			sq_old_prod;
	u16			sq_msn_prod;
	u16			sq_msn_cons;
	u16			sq_npg_cons;
	u16			sq_cmb_prod;

	spinlock_t		rq_lock; /* for posting and polling */
	bool			rq_flush;
	struct ionic_queue	rq;
	void		__iomem *rq_cmb_ptr;
	struct ionic_rq_meta	*rq_meta;
	struct ionic_rq_meta	*rq_meta_head;

	u16			rq_old_prod;
	u16			rq_cmb_prod;

	/* infrequently accessed, keep at end */
	bool			sq_is_cmb;
	int			sq_cmb_order;
	u32			sq_cmb_pgid;
	phys_addr_t		sq_cmb_addr;
	struct ionic_mmap_info	sq_cmb_mmap;

	struct ib_umem		*sq_umem;
	struct ionic_tbl_res	sq_res;

	bool			rq_is_cmb;
	int			rq_cmb_order;
	u32			rq_cmb_pgid;
	phys_addr_t		rq_cmb_addr;
	struct ionic_mmap_info	rq_cmb_mmap;

	struct ib_umem		*rq_umem;
	struct ionic_tbl_res	rq_res;

	struct ionic_tbl_res	rsq_res;
	struct ionic_tbl_res	rrq_res;

	u8			compat;

	struct dentry		*debug;
};

struct ionic_ah {
	struct ib_ah		ibah;
	u32			ahid;
};

struct ionic_mr {
	union {
		struct ib_mr	ibmr;
		struct ib_mw	ibmw;
	};

	u32			mrid;
	int			flags;

	struct ib_umem		*umem;
	struct ionic_tbl_res	res;
	struct ionic_tbl_buf	buf;

	struct dentry		*debug;
};

static inline struct ionic_ibdev *to_ionic_ibdev(struct ib_device *ibdev)
{
	return container_of(ibdev, struct ionic_ibdev, ibdev);
}

static inline struct ionic_ctx *to_ionic_ctx(struct ib_ucontext *ibctx)
{
	return container_of(ibctx, struct ionic_ctx, ibctx);
}

static inline struct ionic_ctx *to_ionic_ctx_fb(struct ib_ucontext *ibctx)
{
	struct ionic_ctx *ctx;

	if (!ibctx)
		return NULL;

	ctx = to_ionic_ctx(ibctx);

	if (ctx->fallback)
		return NULL;

	return ctx;
}

static inline struct ionic_ctx *to_ionic_ctx_uobj(struct ib_uobject *uobj)
{
	struct ionic_ctx *ctx;

	if (!uobj)
		return NULL;

	ctx = to_ionic_ctx(uobj->context);

	if (ctx->fallback)
		return NULL;

	return ctx;
}

static inline struct ionic_pd *to_ionic_pd(struct ib_pd *ibpd)
{
	return container_of(ibpd, struct ionic_pd, ibpd);
}

static inline struct ionic_mr *to_ionic_mr(struct ib_mr *ibmr)
{
	return container_of(ibmr, struct ionic_mr, ibmr);
}

static inline struct ionic_mr *to_ionic_mw(struct ib_mw *ibmw)
{
	return container_of(ibmw, struct ionic_mr, ibmw);
}

static inline struct ionic_cq *to_ionic_cq(struct ib_cq *ibcq)
{
	return container_of(ibcq, struct ionic_cq, ibcq);
}

static inline struct ionic_qp *to_ionic_qp(struct ib_qp *ibqp)
{
	return container_of(ibqp, struct ionic_qp, ibqp);
}

static inline struct ionic_qp *to_ionic_srq(struct ib_srq *ibsrq)
{
	return container_of(ibsrq, struct ionic_qp, ibsrq);
}

static inline struct ionic_ah *to_ionic_ah(struct ib_ah *ibah)
{
        return container_of(ibah, struct ionic_ah, ibah);
}

static inline u32 ionic_dbid(struct ionic_ibdev *dev,
					   struct ib_uobject *uobj)
{
	struct ionic_ctx *ctx = to_ionic_ctx_uobj(uobj);

	if (!ctx)
		return dev->dbid;

	return ctx->dbid;
}

enum ionic_intr_bits {
	/* register set */
	IONIC_INTR_REGS_PER		= 8, /* eight registers per intr */
	IONIC_INTR_REG_COALESCE_INIT	= 0, /* coalesce timer reset value */
	IONIC_INTR_REG_MASK		= 1, /* mask interrupts */
	IONIC_INTR_REG_CREDITS		= 2, /* update credits */
	IONIC_INTR_REG_MASK_ASSERT	= 3, /* mask interrupts on assert */
	IONIC_INTR_REG_COALESCE		= 4, /* coalesce timer */

	/* mask (and mask-on-assert) values */
	IONIC_INTR_MASK_CLEAR		= 0u, /* unmask interrupts */
	IONIC_INTR_MASK_SET		= 1u, /* mask interrupts */

	/* credits values */
	IONIC_INTR_CRED_UNMASK		= 0x10000u, /* unmask interrupts */
	IONIC_INTR_CRED_RESET_COALESCE	= 0x20000u, /* reset coalesce timer */
};

static inline void ionic_intr_coalesce_init(struct ionic_ibdev *dev,
					    int intr, u32 usec)
{
	intr *= IONIC_INTR_REGS_PER;
	intr += IONIC_INTR_REG_COALESCE_INIT;

	iowrite32(usec, &dev->intr_ctrl[intr]);
}

static inline void ionic_intr_mask(struct ionic_ibdev *dev,
				   int intr, u32 mask)
{
	intr *= IONIC_INTR_REGS_PER;
	intr += IONIC_INTR_REG_MASK;

	iowrite32(mask, &dev->intr_ctrl[intr]);
}

static inline void ionic_intr_credits(struct ionic_ibdev *dev,
				      int intr, u32 cred)
{
	intr *= IONIC_INTR_REGS_PER;
	intr += IONIC_INTR_REG_CREDITS;

	iowrite32(cred, &dev->intr_ctrl[intr]);
}

static inline void ionic_intr_mask_assert(struct ionic_ibdev *dev,
					  int intr, u32 mask)
{
	intr *= IONIC_INTR_REGS_PER;
	intr += IONIC_INTR_REG_MASK_ASSERT;

	iowrite32(mask, &dev->intr_ctrl[intr]);
}

static inline bool ionic_ibop_is_local(enum ib_wr_opcode op)
{
	return op == IB_WR_LOCAL_INV || op == IB_WR_REG_MR;
}

void ionic_admin_post(struct ionic_ibdev *dev, struct ionic_admin_wr *wr);
void ionic_admin_cancel(struct ionic_ibdev *dev, struct ionic_admin_wr *wr);

#endif /* IONIC_IBDEV_H */
