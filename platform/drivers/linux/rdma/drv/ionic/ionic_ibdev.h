#ifndef IONIC_IBDEV_H
#define IONIC_IBDEV_H

#include <linux/device.h>
#include <linux/netdevice.h>
#include <rdma/ib_umem.h>
#include <rdma/ib_verbs.h>

#include <rdma/ionic-abi.h>
#include <ionic_api.h>

#include "ionic_queue.h"
#include "table.h"

#define IONIC_DBPAGE_SIZE	0x1000
#define IONIC_MAX_MRID		0xffffff
#define IONIC_MAX_QPID		0xffffff
#define IONIC_MAX_HBM_ORDER	15

struct ionic_mmap_info {
	struct list_head ctx_ent;
	unsigned long offset;
	unsigned long size;
	unsigned long pfn;
};

struct ionic_ibdev {
	struct ib_device	ibdev;

	struct list_head	driver_ent;

	struct ib_device_attr	dev_attr;
	struct ib_port_attr	port_attr;

	struct net_device	*ndev;
	struct lif		*lif;
	int			lif_id;

	u32		__iomem *intr_ctrl;

	phys_addr_t		phys_dbpage_base;
	u64		__iomem *dbpage;
	u32			dbid;

	u8			sq_qtype;
	u8			rq_qtype;
	u8			cq_qtype;
	u8			eq_qtype;

	struct mutex		tbl_lock; /* for modify cq_tbl, qp_tbl */

	struct tbl_root		qp_tbl;
	struct tbl_root		cq_tbl;

	struct mutex		free_lock; /* for id reservation */

	unsigned long		*free_pdid;
	u32			size_pdid;
	u32			next_pdid;

	unsigned long		*free_mrid;
	u32			size_mrid;
	u32			next_mrid;
	u8			next_rkey_key;

	unsigned long		*free_cqid;
	u32			size_cqid;
	u32			next_cqid;

	unsigned long		*free_qpid;
	u32			size_qpid;
	u32			next_qpid;
	u32			size_srqid;
	u32			next_srqid;

	struct list_head	*free_hbm;
	struct list_head	*inuse_hbm;
	int			norder_hbm;
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

struct ionic_pd {
	struct ib_pd		ibpd;

	u32			pdid;
};

struct ionic_cq {
	struct ib_cq		ibcq;

	u32			cqid;

	spinlock_t		lock;
	struct ionic_queue	q;
	struct ib_umem		*umem;
};

struct ionic_sq_meta {
	u64			wrid;
	u32			len;
	u8			op;
};

struct ionic_qp {
	union {
		struct ib_qp	ibqp;
		struct ib_srq	ibsrq;
	};

	u32			qpid;
	bool			has_sq;
	bool			has_rq;
	bool			is_srq;

	spinlock_t		sq_lock;
	struct ionic_queue	sq;
	struct ionic_sq_meta	*sq_meta;

	u32			sq_local;
	u32			sq_msn;

	void			*sq_hbm_ptr;
	u16			sq_hbm_prod;

	spinlock_t		rq_lock;
	struct ionic_queue	rq;

	/* infrequently accessed, keep at end */
	struct ionic_buddy	*sq_hbm_buddy;
	struct ionic_mmap_info	sq_hbm_mmap;

	/* XXX cleanup */
	struct ib_mr		*rq_mr;
	struct ib_mr		*sq_mr;
};

struct ionic_ah {
	struct ib_ah		ibah;
	u32			avid;
};

struct ionic_mr {
	struct ib_mr		ibmr;

	struct ib_umem		*umem;
};

static inline struct ionic_ibdev *to_ionic_ibdev(struct ib_device *ibdev)
{
	return container_of(ibdev, struct ionic_ibdev, ibdev);
}

static inline struct ionic_ctx *to_ionic_ctx(struct ib_ucontext *ibctx)
{
	return container_of(ibctx, struct ionic_ctx, ibctx);
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

#endif /* IONIC_IBDEV_H */
