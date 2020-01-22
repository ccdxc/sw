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

#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <rdma/ib_addr.h>
#include <rdma/ib_cache.h>
#include <rdma/ib_mad.h>
#include <rdma/ib_pack.h>
#include <rdma/ib_user_verbs.h>

#include "ionic_fw.h"
#include "ionic_ibdev.h"

MODULE_AUTHOR("Pensando Systems, Inc");
MODULE_DESCRIPTION("Pensando RoCE HCA driver");
MODULE_LICENSE("Dual BSD/GPL");

#define DRIVER_NAME "ionic_rdma"
#define DRIVER_SHORTNAME "ionr"
/* XXX update or delete version string before submitting upstream */
#define DRIVER_VERSION "0.8.0"
#define DRIVER_DESCRIPTION "Pensando RoCE HCA driver"
#define DEVICE_DESCRIPTION "Pensando RoCE HCA"

/* not a valid queue position or negative error status */
#define IONIC_ADMIN_POSTED 0x10000

/* cpu can be held with irq disabled for COUNT * MS  (for create/destroy_ah) */
#define IONIC_ADMIN_BUSY_RETRY_COUNT 2000
#define IONIC_ADMIN_BUSY_RETRY_MS 1

/* admin queue will be considered failed if a command takes longer */
#define IONIC_ADMIN_TIMEOUT (HZ * 2)
#define IONIC_ADMIN_WARN (HZ)

/* will poll for admin cq to tolerate and report from missed event */
#define IONIC_ADMIN_DELAY (HZ / 4)

/* resource is not reserved on the device, indicated in tbl_order */
#define IONIC_RES_INVALID -1

#define ionic_set_ecn(tos) (((tos) | 2u) & ~1u)
#define ionic_clear_ecn(tos) ((tos) & ~3u)

/* XXX remove this section for release */
static bool ionic_xxx_aq_dbell = true;
module_param_named(ionic_rdma_xxx_aq_dbell, ionic_xxx_aq_dbell, bool, 0644);
MODULE_PARM_DESC(ionic_rdma_xxx_aq_dbell, "XXX Enable ringing aq doorbell (to test handling of aq failure).");
static bool ionic_xxx_qp_dbell = true;
module_param_named(ionic_rdma_xxx_qp_dbell, ionic_xxx_qp_dbell, bool, 0644);
MODULE_PARM_DESC(ionic_rdma_xxx_qp_dbell, "XXX Enable ringing qp doorbell (to test handling of dev failure).");
/* XXX remove above section for release */

/* work queue for handling network events, managing ib devices */
static struct workqueue_struct *ionic_dev_workq;

/* work queue for polling the event queue and admin cq */
static struct workqueue_struct *ionic_evt_workq;

/* access single-threaded thru ionic_dev_workq */
static LIST_HEAD(ionic_ibdev_list);

/* linuxkpi kmalloc is not physically contiguous, so use this instead */
static void *contig_kmalloc(size_t size, gfp_t gfp)
{
	return contigmalloc(size, M_KMALLOC, linux_check_m_flags(gfp),
			    0, ~0ull, PAGE_SIZE, 0);
}
static void *contig_kzalloc(size_t size, gfp_t gfp)
{
	return contig_kmalloc(size, gfp | __GFP_ZERO);
}

/* use this to free bufs allocated above */
static void contig_kfree(void *ptr, size_t size)
{
	if (ptr)
		contigfree(ptr, size, M_KMALLOC);
}

static int ionic_qid_skip = 512;
static void ionic_resid_skip(struct ionic_resid_bits *bits)
{
	int i = ionic_qid_skip - 1;

	while (i < bits->inuse_size) {
		set_bit(i, bits->inuse);
		i += ionic_qid_skip;
	}
}

static ssize_t hca_type_show(struct device *device,
			     struct device_attribute *attr,
			     char *buf)
{
	struct ionic_ibdev *dev =
		container_of(device, struct ionic_ibdev, ibdev.dev);
	return sprintf(buf, "Pensando Naples %u\n",
		       pci_get_device(dev->hwdev->bsddev));
}
static DEVICE_ATTR_RO(hca_type);

static ssize_t hw_rev_show(struct device *device,
			   struct device_attribute *attr,
			   char *buf)
{
	struct ionic_ibdev *dev =
		container_of(device, struct ionic_ibdev, ibdev.dev);
	return sprintf(buf, "%x\n", dev->info->asic_rev);
}
static DEVICE_ATTR_RO(hw_rev);

static ssize_t board_id_show(struct device *device,
			     struct device_attribute *attr,
			     char *buf)
{
	return sprintf(buf, "%.*s\n", 32, "Pensando Naples");
}
static DEVICE_ATTR_RO(board_id);

static struct device_attribute *ionic_dev_attributes[] = {
	&dev_attr_hw_rev,
	&dev_attr_hca_type,
	&dev_attr_board_id
};

static int ionic_validate_udata(struct ib_udata *udata,
				size_t inlen, size_t outlen)
{
	if (udata) {
		if (udata->inlen < inlen || udata->outlen < outlen) {
			pr_debug("have udata in %lu out %lu want %lu %lu\n",
				 udata->inlen, udata->outlen,
				 inlen, outlen);
			return -EINVAL;
		}
	} else {
		if (inlen != 0 || outlen != 0) {
			pr_debug("no udata want %lu %lu\n",
				 inlen, outlen);
			return -EINVAL;
		}
	}

	return 0;
}

static int ionic_validate_qdesc(struct ionic_qdesc *q)
{
	if (!q->addr || !q->size || !q->mask ||
	    !q->depth_log2 || !q->stride_log2)
		return -EINVAL;

	if (q->addr & (PAGE_SIZE - 1))
		return -EINVAL;

	if (q->mask != BIT(q->depth_log2) - 1)
		return -EINVAL;

	if (q->size < BIT_ULL(q->depth_log2 + q->stride_log2))
		return -EINVAL;

	return 0;
}

static int ionic_validate_qdesc_zero(struct ionic_qdesc *q)
{
	if (q->addr || q->size || q->mask || q->depth_log2 || q->stride_log2)
		return -EINVAL;

	return 0;
}

static int ionic_verbs_status_to_rc(u32 status)
{
	switch (status) {
	case IONIC_RC_SUCCESS:	return 0;
	case IONIC_RC_EPERM:	return -EPERM;
	case IONIC_RC_ENOENT:	return -ENOENT;
	case IONIC_RC_EINTR:	return -EINTR;
	case IONIC_RC_EAGAIN:	return -EAGAIN;
	case IONIC_RC_ENOMEM:	return -ENOMEM;
	case IONIC_RC_EFAULT:	return -EFAULT;
	case IONIC_RC_EBUSY:	return -EBUSY;
	case IONIC_RC_EEXIST:	return -EEXIST;
	case IONIC_RC_ENOSPC:	return -ENOSPC;
	case IONIC_RC_ERANGE:	return -ERANGE;
	case IONIC_RC_BAD_ADDR: return -EFAULT;
	case IONIC_RC_DEV_CMD:	return -ENOTTY;
	case IONIC_RC_ENOSUPP:	return -ENODEV;
	case IONIC_RC_ERDMA:	return -EPROTO;
	case IONIC_RC_EINVAL:
	case IONIC_RC_EVERSION:
	case IONIC_RC_EOPCODE:
	case IONIC_RC_EQID:
	case IONIC_RC_EQTYPE:	return -EINVAL;
	case IONIC_RC_EIO:
	case IONIC_RC_ERROR:
	default:		return -EIO;
	}
}

static int ionic_get_pdid(struct ionic_ibdev *dev, u32 *pdid)
{
	int rc;

	mutex_lock(&dev->inuse_lock);
	rc = ionic_resid_get(&dev->inuse_pdid);
	mutex_unlock(&dev->inuse_lock);

	if (rc >= 0) {
		*pdid = rc;
		rc = 0;
	}

	return rc;
}

static int ionic_get_ahid(struct ionic_ibdev *dev, u32 *ahid)
{
	unsigned long irqflags;
	int rc;

	spin_lock_irqsave(&dev->inuse_splock, irqflags);
	rc = ionic_resid_get(&dev->inuse_ahid);
	spin_unlock_irqrestore(&dev->inuse_splock, irqflags);

	if (rc >= 0) {
		*ahid = rc;
		rc = 0;
	}

	return rc;
}

static int ionic_get_mrid(struct ionic_ibdev *dev, u32 *mrid)
{
	int rc;

	mutex_lock(&dev->inuse_lock);
	/* wrap to 1, skip reserved lkey */
	rc = ionic_resid_get_wrap(&dev->inuse_mrid, 1);
	if (rc >= 0) {
		*mrid = ionic_mrid(rc, dev->next_mrkey++);
		rc = 0;
	}
	mutex_unlock(&dev->inuse_lock);

	return rc;
}

static int ionic_get_cqid(struct ionic_ibdev *dev, u32 *cqid)
{
	int rc;

	mutex_lock(&dev->inuse_lock);
	rc = ionic_resid_get(&dev->inuse_cqid);
	mutex_unlock(&dev->inuse_lock);

	if (rc >= 0) {
		*cqid = rc + dev->cq_base;
		rc = 0;
	}

	return rc;
}

static int ionic_get_gsi_qpid(struct ionic_ibdev *dev, u32 *qpid)
{
	int rc = 0;

	mutex_lock(&dev->inuse_lock);
	if (test_bit(IB_QPT_GSI, dev->inuse_qpid.inuse)) {
		rc = -EINVAL;
	} else {
		set_bit(IB_QPT_GSI, dev->inuse_qpid.inuse);
		*qpid = IB_QPT_GSI;
	}
	mutex_unlock(&dev->inuse_lock);

	return rc;
}

static int ionic_get_qpid(struct ionic_ibdev *dev, u32 *qpid)
{
	int rc = 0;

	mutex_lock(&dev->inuse_lock);
	rc = ionic_resid_get_shared(&dev->inuse_qpid, 2,
				    dev->inuse_qpid.next_id,
				    dev->size_qpid);
	if (rc >= 0) {
		dev->inuse_qpid.next_id = rc + 1;
		*qpid = rc;
		rc = 0;
	}
	mutex_unlock(&dev->inuse_lock);

	return rc;
}

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
#endif /* IONIC_SRQ_XRC */

static void ionic_put_pdid(struct ionic_ibdev *dev, u32 pdid)
{
	ionic_resid_put(&dev->inuse_pdid, pdid);
}

static void ionic_put_ahid(struct ionic_ibdev *dev, u32 ahid)
{
	ionic_resid_put(&dev->inuse_ahid, ahid);
}

static void ionic_put_mrid(struct ionic_ibdev *dev, u32 mrid)
{
	ionic_resid_put(&dev->inuse_mrid, ionic_mrid_index(mrid));
}

static void ionic_put_cqid(struct ionic_ibdev *dev, u32 cqid)
{
	ionic_resid_put(&dev->inuse_cqid, cqid - dev->cq_base);
}

static void ionic_put_qpid(struct ionic_ibdev *dev, u32 qpid)
{
	ionic_resid_put(&dev->inuse_qpid, qpid);
}

#ifdef IONIC_SRQ_XRC
static void ionic_put_srqid(struct ionic_ibdev *dev, u32 qpid)
{
	ionic_resid_put(&dev->inuse_qpid, qpid);
}
#endif /* IONIC_SRQ_XRC */

static int ionic_res_order(int count, int stride, int cl_stride)
{
	/* count becomes log2 of size in bytes */
	count = order_base_2(count) + stride;

	/* zero if less than one cache line */
	if (count < cl_stride)
		return 0;

	return count - cl_stride;
}

static int ionic_get_res(struct ionic_ibdev *dev, struct ionic_tbl_res *res)
{
	int rc = 0;

	mutex_lock(&dev->inuse_lock);
	rc = ionic_buddy_get(&dev->inuse_restbl, res->tbl_order);
	mutex_unlock(&dev->inuse_lock);

	if (rc < 0) {
		res->tbl_order = IONIC_RES_INVALID;
		res->tbl_pos = 0;
		return rc;
	}

	res->tbl_pos = rc << (dev->cl_stride - dev->pte_stride);

	return 0;
}

static bool ionic_put_res(struct ionic_ibdev *dev, struct ionic_tbl_res *res)
{
	if (res->tbl_order == IONIC_RES_INVALID)
		return false;

	res->tbl_pos >>= dev->cl_stride - dev->pte_stride;

	mutex_lock(&dev->inuse_lock);
	ionic_buddy_put(&dev->inuse_restbl, res->tbl_pos, res->tbl_order);
	mutex_unlock(&dev->inuse_lock);

	res->tbl_order = IONIC_RES_INVALID;
	res->tbl_pos = 0;

	return true;
}

static inline __le64 ionic_pgtbl_dma(struct ionic_tbl_buf *buf, u64 va)
{
	u64 dma;

	if (!buf->tbl_pages)
		return cpu_to_le64(0);

	if (buf->tbl_pages > 1)
		return cpu_to_le64(buf->tbl_dma);

	if (buf->tbl_buf)
		dma = le64_to_cpu(buf->tbl_buf[0]);
	else
		dma = buf->tbl_dma;

	return cpu_to_le64(dma + (va & ~PAGE_MASK));
}

static inline __be64 ionic_pgtbl_off(struct ionic_tbl_buf *buf, u64 va)
{
	if (buf->tbl_pages > 1)
		return cpu_to_be64(va & ~PAGE_MASK);
	else
		return 0;
}

static int ionic_pgtbl_page(struct ionic_tbl_buf *buf, u64 dma)
{
	if (unlikely(buf->tbl_pages == buf->tbl_limit))
		return -ENOMEM;

	if (buf->tbl_buf)
		buf->tbl_buf[buf->tbl_pages] = cpu_to_le64(dma);
	else
		buf->tbl_dma = dma;

	++buf->tbl_pages;

	return 0;
}

static int ionic_pgtbl_umem(struct ionic_tbl_buf *buf, struct ib_umem *umem)
{
	struct scatterlist *sg;
	u64 page_dma;
	int rc = 0, sg_i, page_i, page_count;

	for_each_sg(umem->sg_head.sgl, sg, umem->nmap, sg_i) {
		page_dma = sg_dma_address(sg);
		page_count = sg_dma_len(sg) >> PAGE_SHIFT;

		for (page_i = 0; page_i < page_count; ++page_i) {
			rc = ionic_pgtbl_page(buf, page_dma);
			if (rc)
				goto out;

			page_dma += PAGE_SIZE;
		}
	}

	buf->page_size_log2 = PAGE_SHIFT;

out:
	return rc;
}

static int ionic_pgtbl_init(struct ionic_ibdev *dev, struct ionic_tbl_res *res,
			    struct ionic_tbl_buf *buf, struct ib_umem *umem,
			    dma_addr_t dma, int limit)
{
	int rc;

	if (umem)
		limit = ib_umem_num_pages(umem);

	if (limit < 1)
		return -EINVAL;

	res->tbl_order = ionic_res_order(limit, dev->pte_stride,
					 dev->cl_stride);
	res->tbl_pos = 0;

	buf->tbl_limit = limit;
	buf->tbl_pages = 0;
	buf->tbl_size = 0;
	buf->tbl_buf = NULL;
	buf->page_size_log2 = 0;

	/* skip pgtbl if contiguous / direct translation */
	if (limit > 1) {
		/* A reservation will be made for page table resources.
		 *
		 * The page table reservation must be large enough to account
		 * for alignment of the first page within the page table.  The
		 * requirement is therefore greater-than-page-size alignment.
		 * If not accounted for, the placement of pages in the page
		 * table could extend beyond the bounds of the reservation.
		 *
		 * To account for the alignment of the page in the reservation,
		 * extend the reservation based on the worst case first page
		 * alignment.  The worst case is required for alloc_mr, because
		 * only the number of pages is given, but the alignment is not
		 * known until registration.
		 */
		if (likely(dev->cl_stride > dev->pte_stride)) {
			/* increase the limit to account for worst case */
			buf->tbl_limit =
				1 + ALIGN(limit - 1, BIT(dev->cl_stride -
							 dev->pte_stride));

			/* and recalculate the reservation dimensions */
			res->tbl_order = ionic_res_order(buf->tbl_limit,
							 dev->pte_stride,
							 dev->cl_stride);
		}

		rc = ionic_get_res(dev, res);
		if (rc)
			goto err_res;

		/* limit for pte reservation should not affect anything else */
		buf->tbl_limit = limit;
	} else {
		res->tbl_order = IONIC_RES_INVALID;
	}

	if (limit > 1) {
		buf->tbl_size = buf->tbl_limit * sizeof(*buf->tbl_buf);
		buf->tbl_buf = contig_kmalloc(buf->tbl_size, GFP_KERNEL);
		if (!buf->tbl_buf) {
			rc = -ENOMEM;
			goto err_buf;
		}

		buf->tbl_dma = dma_map_single(dev->hwdev, buf->tbl_buf,
					      buf->tbl_size, DMA_FROM_DEVICE);
		rc = dma_mapping_error(dev->hwdev, buf->tbl_dma);
		if (rc)
			goto err_dma;
	}

	if (umem) {
		rc = ionic_pgtbl_umem(buf, umem);
		if (rc)
			goto err_umem;
	} else {
		rc = ionic_pgtbl_page(buf, dma);
		if (rc)
			goto err_umem;

		buf->page_size_log2 = 0;
	}

	return 0;

err_umem:
	if (buf->tbl_buf) {
		dma_unmap_single(dev->hwdev, buf->tbl_dma, buf->tbl_size,
				 DMA_FROM_DEVICE);
err_dma:
		contig_kfree(buf->tbl_buf, buf->tbl_size);
	}
err_buf:
	ionic_put_res(dev, res);
err_res:
	buf->tbl_buf = NULL;
	buf->tbl_limit = 0;
	buf->tbl_pages = 0;
	return rc;
}

static void ionic_pgtbl_unbuf(struct ionic_ibdev *dev,
			      struct ionic_tbl_buf *buf)
{
	if (buf->tbl_buf) {
		dma_unmap_single(dev->hwdev, buf->tbl_dma, buf->tbl_size,
				 DMA_FROM_DEVICE);
		contig_kfree(buf->tbl_buf, buf->tbl_size);
	}

	buf->tbl_buf = NULL;
	buf->tbl_limit = 0;
	buf->tbl_pages = 0;
}

static bool ionic_next_cqe(struct ionic_cq *cq, struct ionic_v1_cqe **cqe)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(cq->ibcq.device);
	struct ionic_v1_cqe *qcqe = ionic_queue_at_prod(&cq->q);

	if (unlikely(cq->color != ionic_v1_cqe_color(qcqe)))
		return false;

	rmb();

	dev_dbg(&dev->ibdev.dev, "poll cq %u prod %u\n", cq->cqid, cq->q.prod);
	print_hex_dump_debug("cqe ", DUMP_PREFIX_OFFSET, 16, 1,
			     qcqe, BIT(cq->q.stride_log2), true);

	*cqe = qcqe;

	return true;
}

static void ionic_clean_cq(struct ionic_cq *cq, u32 qpid)
{
	struct ionic_v1_cqe *qcqe;
	int prod, qtf, qid, type;
	bool color;

	if (!cq->q.ptr)
		return;

	color = cq->color;
	prod = cq->q.prod;
	qcqe = ionic_queue_at(&cq->q, prod);

	while (color == ionic_v1_cqe_color(qcqe)) {
		qtf = ionic_v1_cqe_qtf(qcqe);
		qid = ionic_v1_cqe_qtf_qid(qtf);
		type = ionic_v1_cqe_qtf_type(qtf);

		if (qid == qpid && type != IONIC_V1_CQE_TYPE_ADMIN)
			ionic_v1_cqe_clean(qcqe);

		prod = ionic_queue_next(&cq->q, prod);
		qcqe = ionic_queue_at(&cq->q, prod);
		color = ionic_color_wrap(prod, color);
	}
}

static void ionic_admin_timedout(struct ionic_aq *aq)
{
	struct ionic_ibdev *dev = aq->dev;
	struct ionic_cq *cq = aq->cq;
	unsigned long irqflags;
	u16 pos;

	spin_lock_irqsave(&aq->lock, irqflags);
	if (ionic_queue_empty(&aq->q))
		goto out;

	/* Reset ALL adminq if any one times out */
	queue_work(ionic_evt_workq, &dev->reset_work);

	dev_err(&dev->ibdev.dev, "admin command timed out, aq %d\n", aq->aqid);

	dev_warn(&dev->ibdev.dev, "admin timeout was set for %ums\n",
		 (u32)jiffies_to_msecs(IONIC_ADMIN_TIMEOUT));
	dev_warn(&dev->ibdev.dev, "admin inactivity for %ums\n",
		 (u32)jiffies_to_msecs(jiffies - aq->stamp));

	dev_warn(&dev->ibdev.dev, "admin commands outstanding %u\n",
		 ionic_queue_length(&aq->q));
	dev_warn(&dev->ibdev.dev, "more commands pending? %s\n",
		 list_empty(&aq->wr_post) ? "no" : "yes");

	pos = cq->q.prod;

	dev_warn(&dev->ibdev.dev, "admin cq pos %u (next to complete)\n", pos);
	print_hex_dump(KERN_WARNING, "cqe ", DUMP_PREFIX_OFFSET, 16, 1,
		       ionic_queue_at(&cq->q, pos),
		       BIT(cq->q.stride_log2), true);

	pos = (pos - 1) & cq->q.mask;

	dev_warn(&dev->ibdev.dev, "admin cq pos %u (last completed)\n", pos);
	print_hex_dump(KERN_WARNING, "cqe ", DUMP_PREFIX_OFFSET, 16, 1,
		       ionic_queue_at(&cq->q, pos),
		       BIT(cq->q.stride_log2), true);

	pos = aq->q.cons;

	dev_warn(&dev->ibdev.dev, "admin pos %u (next to complete)\n", pos);
	print_hex_dump(KERN_WARNING, "cmd ", DUMP_PREFIX_OFFSET, 16, 1,
		       ionic_queue_at(&aq->q, pos),
		       BIT(aq->q.stride_log2), true);

	pos = (aq->q.prod - 1) & aq->q.mask;
	if (pos == aq->q.cons)
		goto out;

	dev_warn(&dev->ibdev.dev, "admin pos %u (last posted)\n", pos);
	print_hex_dump(KERN_WARNING, "cmd ", DUMP_PREFIX_OFFSET, 16, 1,
		       ionic_queue_at(&aq->q, pos),
		       BIT(aq->q.stride_log2), true);

out:
	spin_unlock_irqrestore(&aq->lock, irqflags);
}

static void ionic_admin_reset_dwork(struct ionic_ibdev *dev)
{
	if (dev->admin_state < IONIC_ADMIN_KILLED)
		queue_delayed_work(ionic_evt_workq, &dev->admin_dwork,
				   IONIC_ADMIN_DELAY);
}

static void ionic_admin_reset_wdog(struct ionic_aq *aq)
{
	aq->stamp = jiffies;
	ionic_admin_reset_dwork(aq->dev);
}

static void ionic_admin_poll_locked(struct ionic_aq *aq)
{
	struct ionic_ibdev *dev = aq->dev;
	struct ionic_cq *cq = aq->cq;
	struct ionic_admin_wr *wr, *wr_next;
	struct ionic_v1_admin_wqe *wqe;
	struct ionic_v1_cqe *cqe;
	u32 qtf, qid;
	u8 type;
	u16 old_prod;

	if (dev->admin_state >= IONIC_ADMIN_KILLED) {
		list_for_each_entry_safe(wr, wr_next, &aq->wr_prod, aq_ent) {
			INIT_LIST_HEAD(&wr->aq_ent);
			aq->q_wr[wr->status] = NULL;
			wr->status = dev->admin_state;
			complete_all(&wr->work);
		}
		INIT_LIST_HEAD(&aq->wr_prod);

		list_for_each_entry_safe(wr, wr_next, &aq->wr_post, aq_ent) {
			INIT_LIST_HEAD(&wr->aq_ent);
			wr->status = dev->admin_state;
			complete_all(&wr->work);
		}
		INIT_LIST_HEAD(&aq->wr_post);

		return;
	}

	old_prod = cq->q.prod;

	while (ionic_next_cqe(cq, &cqe)) {
		qtf = ionic_v1_cqe_qtf(cqe);
		qid = ionic_v1_cqe_qtf_qid(qtf);
		type = ionic_v1_cqe_qtf_type(qtf);

		if (unlikely(type != IONIC_V1_CQE_TYPE_ADMIN)) {
			dev_warn_ratelimited(&dev->ibdev.dev,
					     "unexpected cqe type %u\n", type);
			goto cq_next;
		}

		if (unlikely(qid != aq->aqid)) {
			dev_warn_ratelimited(&dev->ibdev.dev,
					     "unexpected cqe qid %u\n", qid);
			goto cq_next;
		}

		if (unlikely(be16_to_cpu(cqe->admin.cmd_idx) != aq->q.cons)) {
			dev_warn_ratelimited(&dev->ibdev.dev,
					     "unexpected idx %u cons %u qid %u\n",
					     be16_to_cpu(cqe->admin.cmd_idx),
					     aq->q.cons, qid);
			goto cq_next;
		}

		if (unlikely(ionic_queue_empty(&aq->q))) {
			dev_warn_ratelimited(&dev->ibdev.dev,
					     "unexpected cqe for empty adminq\n");
			goto cq_next;
		}

		wr = aq->q_wr[aq->q.cons];
		if (wr) {
			aq->q_wr[aq->q.cons] = NULL;
			list_del_init(&wr->aq_ent);

			wr->cqe = *cqe;
			wr->status = dev->admin_state;
			complete_all(&wr->work);
		}

		ionic_queue_consume(&aq->q);

cq_next:
		ionic_queue_produce(&cq->q);
		cq->color = ionic_color_wrap(cq->q.prod, cq->color);
	}

	if (old_prod != cq->q.prod) {
		ionic_admin_reset_wdog(aq);
		cq->q.cons = cq->q.prod;
		ionic_dbell_ring(dev->dbpage, dev->cq_qtype,
				 ionic_queue_dbell_val(&cq->q));
		queue_work(ionic_evt_workq, &aq->work);
	} else if (!aq->armed) {
		aq->armed = true;
		cq->arm_any_prod = ionic_queue_next(&cq->q, cq->arm_any_prod);
		ionic_dbell_ring(dev->dbpage, dev->cq_qtype,
				 cq->q.dbell | IONIC_CQ_RING_ARM |
				 cq->arm_any_prod);
		queue_work(ionic_evt_workq, &aq->work);
	}

	if (dev->admin_state != IONIC_ADMIN_ACTIVE)
		return;

	old_prod = aq->q.prod;

	if (ionic_queue_empty(&aq->q) && !list_empty(&aq->wr_post))
		ionic_admin_reset_wdog(aq);

	while (!ionic_queue_full(&aq->q) && !list_empty(&aq->wr_post)) {
		wqe = ionic_queue_at_prod(&aq->q);

		wr = list_first_entry(&aq->wr_post,
				      struct ionic_admin_wr, aq_ent);

		list_move(&wr->aq_ent, &aq->wr_prod);

		wr->status = aq->q.prod;
		aq->q_wr[aq->q.prod] = wr;

		*wqe = wr->wqe;

		dev_dbg(&dev->ibdev.dev, "post admin prod %u\n", aq->q.prod);
		print_hex_dump_debug("wqe ", DUMP_PREFIX_OFFSET, 16, 1,
				     ionic_queue_at_prod(&aq->q),
				     BIT(aq->q.stride_log2), true);

		ionic_queue_produce(&aq->q);
	}

	if (old_prod != aq->q.prod && ionic_xxx_aq_dbell)
		ionic_dbell_ring(dev->dbpage, dev->aq_qtype,
				 ionic_queue_dbell_val(&aq->q));
}

static void ionic_admin_dwork(struct work_struct *ws)
{
	struct ionic_ibdev *dev =
		container_of(ws, struct ionic_ibdev, admin_dwork.work);
	struct ionic_aq *aq, *bad_aq = NULL;
	unsigned long irqflags;
	int i;
	bool do_reset = false;
	bool do_reschedule = false;
	u16 pos;

	for (i = 0; i < dev->aq_count; i++) {
		aq = dev->aq_vec[i];

		spin_lock_irqsave(&aq->lock, irqflags);

		if (ionic_queue_empty(&aq->q))
			goto next_aq;

		/* Reschedule if any queue has outstanding work */
		do_reschedule = true;

		if (time_is_after_eq_jiffies(aq->stamp + IONIC_ADMIN_WARN))
			/* Warning threshold not met, nothing to do */
			goto next_aq;

		/* See if polling now makes some progress */
		pos = aq->q.cons;
		ionic_admin_poll_locked(aq);
		if (pos != aq->q.cons) {
			dev_dbg(&dev->ibdev.dev,
				"missed event for acq %d\n", aq->cqid);
			goto next_aq;
		}

		if (time_is_after_eq_jiffies(aq->stamp +
					     IONIC_ADMIN_TIMEOUT)) {
			/* Timeout threshold not met */
			dev_dbg(&dev->ibdev.dev, "no progress after %ums\n",
				(u32)jiffies_to_msecs(jiffies - aq->stamp));
			goto next_aq;
		}

		/* Queue timed out */
		bad_aq = aq;
		do_reset = true;
next_aq:
		spin_unlock_irqrestore(&aq->lock, irqflags);
	}

	if (do_reset)
		/* Reset device on a timeout */
		ionic_admin_timedout(bad_aq);
	else if (do_reschedule)
		/* Try to poll again later */
		ionic_admin_reset_dwork(dev);
}

static void ionic_admin_work(struct work_struct *ws)
{
	struct ionic_aq *aq = container_of(ws, struct ionic_aq, work);
	unsigned long irqflags;

	spin_lock_irqsave(&aq->lock, irqflags);
	ionic_admin_poll_locked(aq);
	spin_unlock_irqrestore(&aq->lock, irqflags);
}

void ionic_admin_post_aq(struct ionic_aq *aq, struct ionic_admin_wr *wr)
{
	unsigned long irqflags;
	bool poll;

	wr->status = IONIC_ADMIN_POSTED;
	wr->aq = aq;

	spin_lock_irqsave(&aq->lock, irqflags);
	poll = list_empty(&aq->wr_post);
	list_add(&wr->aq_ent, &aq->wr_post);
	if (poll)
		ionic_admin_poll_locked(aq);
	spin_unlock_irqrestore(&aq->lock, irqflags);
}

void ionic_admin_post(struct ionic_ibdev *dev, struct ionic_admin_wr *wr)
{
	int aq_idx;

	aq_idx = raw_smp_processor_id() % dev->aq_count;
	ionic_admin_post_aq(dev->aq_vec[aq_idx], wr);
}

void ionic_admin_cancel(struct ionic_admin_wr *wr)
{
	struct ionic_aq *aq = wr->aq;
	unsigned long irqflags;

	spin_lock_irqsave(&aq->lock, irqflags);

	if (!list_empty(&wr->aq_ent)) {
		list_del(&wr->aq_ent);
		if (wr->status != IONIC_ADMIN_POSTED)
			aq->q_wr[wr->status] = NULL;
	}

	spin_unlock_irqrestore(&aq->lock, irqflags);
}

static void ionic_admin_wait(struct ionic_admin_wr *wr)
{
	wait_for_completion(&wr->work);
}

static int ionic_admin_busy_wait(struct ionic_admin_wr *wr)
{
	struct ionic_aq *aq = wr->aq;
	unsigned long irqflags;
	int try_i;

	for (try_i = 0; ; ++try_i) {
		if (completion_done(&wr->work))
			return 0;

		/*
		 * Did not complete before timeout: do not continue waiting,
		 * but initiate RDMA LIF reset and indicate error to caller.
		 */
		if (try_i >= IONIC_ADMIN_BUSY_RETRY_COUNT) {
			ionic_admin_timedout(aq);
			return -ETIMEDOUT;
		}

		mdelay(IONIC_ADMIN_BUSY_RETRY_MS);

		spin_lock_irqsave(&aq->lock, irqflags);
		ionic_admin_poll_locked(aq);
		spin_unlock_irqrestore(&aq->lock, irqflags);
	}

	/* unreachable */
	return -EINTR;
}

static bool ionic_v1_stat_normalize(struct ionic_v1_stat *stat)
{
	stat->type_off = be32_to_cpu(stat->be_type_off);
	stat->name[sizeof(stat->name) - 1] = 0;

	return ionic_v1_stat_type(stat) != IONIC_V1_STAT_TYPE_NONE;
}

static u64 ionic_v1_stat_val(struct ionic_v1_stat *stat,
			     void *vals_buf, size_t vals_len)
{
	int type = ionic_v1_stat_type(stat);
	unsigned off = ionic_v1_stat_off(stat);

#define __ionic_v1_stat_validate(__type) do { \
		if (off + sizeof(__type) > vals_len)	\
			goto err;			\
		if (!IS_ALIGNED(off, sizeof(__type)))	\
			goto err;			\
	} while (0)

	switch (type) {
	case IONIC_V1_STAT_TYPE_8:
		__ionic_v1_stat_validate(u8);
		return *(u8 *)(vals_buf + off);
	case IONIC_V1_STAT_TYPE_LE16:
		__ionic_v1_stat_validate(__le16);
		return le16_to_cpu(*(__le16 *)(vals_buf + off));
	case IONIC_V1_STAT_TYPE_LE32:
		__ionic_v1_stat_validate(__le32);
		return le32_to_cpu(*(__le32 *)(vals_buf + off));
	case IONIC_V1_STAT_TYPE_LE64:
		__ionic_v1_stat_validate(__le64);
		return le64_to_cpu(*(__le64 *)(vals_buf + off));
	case IONIC_V1_STAT_TYPE_BE16:
		__ionic_v1_stat_validate(__be16);
		return be16_to_cpu(*(__be16 *)(vals_buf + off));
	case IONIC_V1_STAT_TYPE_BE32:
		__ionic_v1_stat_validate(__be32);
		return be32_to_cpu(*(__be32 *)(vals_buf + off));
	case IONIC_V1_STAT_TYPE_BE64:
		__ionic_v1_stat_validate(__be64);
		return be64_to_cpu(*(__be64 *)(vals_buf + off));
	}

err:
	return ~0ull;
#undef __ionic_v1_stat_validate
}

static int ionic_stats_cmd(struct ionic_ibdev *dev,
			   dma_addr_t dma, size_t len, int op)
{
	struct ionic_admin_wr wr = {
		.work = COMPLETION_INITIALIZER_ONSTACK(wr.work),
		.wqe = {
			.op = op,
			.stats = {
				.dma_addr = cpu_to_le64(dma),
				.length = cpu_to_le32(len),
			}
		}
	};
	int rc;

	if (dev->admin_opcodes <= op)
		return -ENOSYS;

	ionic_admin_post(dev, &wr);

	rc = wait_for_completion_interruptible(&wr.work);
	if (rc) {
		dev_warn(&dev->ibdev.dev, "wait status %d\n", rc);
		ionic_admin_cancel(&wr);
	} else if (wr.status == IONIC_ADMIN_KILLED) {
		dev_dbg(&dev->ibdev.dev, "killed\n");
		rc = -ENODEV;
	} else if (ionic_v1_cqe_error(&wr.cqe)) {
		dev_warn(&dev->ibdev.dev, "error %u\n",
			 be32_to_cpu(wr.cqe.status_length));
		rc = -EINVAL;
	} else {
		rc = 0;
	}

	return rc;
}

static int ionic_stats_hdrs_cmd(struct ionic_ibdev *dev,
				dma_addr_t dma, size_t len)
{
	return ionic_stats_cmd(dev, dma, len, IONIC_V1_ADMIN_STATS_HDRS);
}

static int ionic_stats_vals_cmd(struct ionic_ibdev *dev,
				dma_addr_t dma, size_t len)
{
	return ionic_stats_cmd(dev, dma, len, IONIC_V1_ADMIN_STATS_VALS);
}

static int ionic_init_hw_stats(struct ionic_ibdev *dev)
{
	dma_addr_t stats_dma;
	struct ionic_v1_stat *stat;
	int rc, stat_i, stats_count;

	if (dev->stats_hdrs)
		return 0;

	dev->stats_count = 0;

	/* buffer for current values from the device */
	dev->stats_buf = contig_kzalloc(PAGE_SIZE, GFP_KERNEL);
	if (!dev->stats_buf) {
		rc = -ENOMEM;
		goto err_buf;
	}

	/* buffer for names, sizes, offsets of values */
	dev->stats = contig_kzalloc(PAGE_SIZE, GFP_KERNEL);
	if (!dev->stats) {
		rc = -ENOMEM;
		goto err_stats;
	}

	/* request the names, sizes, offsets */
	stats_dma = dma_map_single(dev->hwdev, dev->stats,
				   PAGE_SIZE, DMA_FROM_DEVICE);
	rc = dma_mapping_error(dev->hwdev, stats_dma);
	if (rc)
		goto err_dma;

	rc = ionic_stats_hdrs_cmd(dev, stats_dma, PAGE_SIZE);
	if (rc)
		goto err_cmd;

	dma_unmap_single(dev->hwdev, stats_dma, PAGE_SIZE, DMA_FROM_DEVICE);

	/* normalize and count the number of stats */
	stats_count = PAGE_SIZE / sizeof(*dev->stats);
	for (stat_i = 0; stat_i < stats_count; ++stat_i) {
		stat = &dev->stats[stat_i];

		if (!ionic_v1_stat_normalize(stat))
			break;
	}

	if (!stat_i) {
		rc = -ENOSYS;
		goto err_dma;
	}

	stats_count = stat_i;
	dev->stats_count = stat_i;

	/* alloc and init array of names, for alloc_hw_stats */
	dev->stats_hdrs = kmalloc_array(stats_count, sizeof(*dev->stats_hdrs),
					GFP_KERNEL);
	if (!dev->stats_hdrs) {
		rc = -ENOMEM;
		goto err_dma;
	}

	for (stat_i = 0; stat_i < stats_count; ++stat_i) {
		stat = &dev->stats[stat_i];
		dev->stats_hdrs[stat_i] = stat->name;
	}

	return 0;

err_cmd:
	dma_unmap_single(dev->hwdev, stats_dma, PAGE_SIZE, DMA_FROM_DEVICE);
err_dma:
	contig_kfree(dev->stats, PAGE_SIZE);
err_stats:
	contig_kfree(dev->stats_buf, PAGE_SIZE);
err_buf:
	dev->stats_count = 0;
	dev->stats = NULL;
	dev->stats_buf = NULL;
	dev->stats_hdrs = NULL;
	return rc;
}

static struct rdma_hw_stats *ionic_alloc_hw_stats(struct ib_device *ibdev,
						  u8 port)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibdev);
	int rc;

	if (port != 1)
		return NULL;

	rc = ionic_init_hw_stats(dev);
	if (rc)
		return NULL;

	return rdma_alloc_hw_stats_struct(dev->stats_hdrs, dev->stats_count,
					  RDMA_HW_STATS_DEFAULT_LIFESPAN);
}

static int ionic_get_hw_stats(struct ib_device *ibdev,
			      struct rdma_hw_stats *stats,
			      u8 port, int index)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibdev);
	dma_addr_t stats_dma;
	int rc, stat_i;

	if (port != 1)
		return -EINVAL;

	stats_dma = dma_map_single(dev->hwdev, dev->stats_buf,
				   PAGE_SIZE, DMA_FROM_DEVICE);
	rc = dma_mapping_error(dev->hwdev, stats_dma);
	if (rc)
		goto err_dma;

	rc = ionic_stats_vals_cmd(dev, stats_dma, PAGE_SIZE);
	if (rc)
		goto err_cmd;

	dma_unmap_single(dev->hwdev, stats_dma,
			 PAGE_SIZE, DMA_FROM_DEVICE);

	for (stat_i = 0; stat_i < dev->stats_count; ++stat_i)
		stats->value[stat_i] =
			ionic_v1_stat_val(&dev->stats[stat_i],
					  dev->stats_buf, PAGE_SIZE);

	return stat_i;

err_cmd:
	dma_unmap_single(dev->hwdev, stats_dma,
			 PAGE_SIZE, DMA_FROM_DEVICE);
err_dma:
	return rc;
}

static int ionic_query_device(struct ib_device *ibdev,
			      struct ib_device_attr *attr,
			      struct ib_udata *udata)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibdev);

	addrconf_ifid_eui48((u8 *)&attr->sys_image_guid, dev->ndev);
	attr->max_mr_size =
		((u64)dev->inuse_restbl.inuse_size * PAGE_SIZE / 2) <<
		(dev->cl_stride - dev->pte_stride);
	attr->page_size_cap = PAGE_SIZE;
	attr->vendor_id = pci_get_vendor(dev->hwdev->bsddev);
	attr->vendor_part_id = pci_get_device(dev->hwdev->bsddev);
	attr->hw_ver = dev->info->asic_rev;
	attr->fw_ver = 0;
	attr->max_qp = dev->size_qpid;
	attr->max_qp_wr = IONIC_MAX_DEPTH;
	attr->device_cap_flags =
		IB_DEVICE_LOCAL_DMA_LKEY |
		IB_DEVICE_MEM_WINDOW |
		IB_DEVICE_MEM_MGT_EXTENSIONS |
		IB_DEVICE_MEM_WINDOW_TYPE_2B |
		0;
#ifdef HAVE_IBDEV_MAX_SEND_RECV_SGE
	attr->max_send_sge =
		min(ionic_v1_send_wqe_max_sge(dev->max_stride, 0),
		    ionic_spec);
	attr->max_recv_sge =
		min(ionic_v1_recv_wqe_max_sge(dev->max_stride, 0),
		    ionic_spec);
	attr->max_sge_rd = attr->max_send_sge;
#else
	attr->max_sge =
		min3(ionic_v1_send_wqe_max_sge(dev->max_stride, 0),
		     ionic_v1_recv_wqe_max_sge(dev->max_stride, 0),
		     ionic_spec);
	attr->max_sge_rd = attr->max_sge;
#endif
	attr->max_cq = dev->inuse_cqid.inuse_size;
	attr->max_cqe = IONIC_MAX_CQ_DEPTH;
	attr->max_mr = dev->inuse_mrid.inuse_size;
	attr->max_pd = ionic_max_pd;
	attr->max_qp_rd_atom = IONIC_MAX_RD_ATOM;
	attr->max_ee_rd_atom = 0;
	attr->max_res_rd_atom = IONIC_MAX_RD_ATOM;
	attr->max_qp_init_rd_atom = IONIC_MAX_RD_ATOM;
	attr->max_ee_init_rd_atom = 0;
	attr->atomic_cap = IB_ATOMIC_GLOB;
	attr->masked_atomic_cap = IB_ATOMIC_GLOB;
	attr->max_mw = dev->inuse_mrid.inuse_size;
	attr->max_mcast_grp = 0;
	attr->max_mcast_qp_attach = 0;
	attr->max_ah = dev->inuse_ahid.inuse_size;
#ifdef IONIC_SRQ_XRC
	attr->max_srq = dev->size_srqid;
	attr->max_srq_wr = IONIC_MAX_DEPTH;
	attr->max_srq_sge =
		min(ionic_v1_recv_wqe_max_sge(dev->max_stride, 0),
		    ionic_spec);
#endif /* IONIC_SRQ_XRC */
	attr->max_fast_reg_page_list_len =
		(dev->inuse_restbl.inuse_size / 2) <<
		(dev->cl_stride - dev->pte_stride);
	attr->max_pkeys = IONIC_PKEY_TBL_LEN;

	return 0;
}

static int ionic_query_port(struct ib_device *ibdev, u8 port,
			    struct ib_port_attr *attr)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibdev);
	struct net_device *ndev = dev->ndev;

	if (port != 1)
		return -EINVAL;

	if (netif_running(ndev) && netif_carrier_ok(ndev)) {
		attr->state = IB_PORT_ACTIVE;
		attr->phys_state = IB_PORT_PHYS_STATE_LINK_UP;
	} else if (netif_running(ndev)) {
		attr->state = IB_PORT_DOWN;
		attr->phys_state = IB_PORT_PHYS_STATE_POLLING;
	} else {
		attr->state = IB_PORT_DOWN;
		attr->phys_state = IB_PORT_PHYS_STATE_DISABLED;
	}

#ifdef HAVE_NETDEV_MAX_MTU
	attr->max_mtu = ib_mtu_int_to_enum(ndev->max_mtu);
#else
	attr->max_mtu = IB_MTU_4096;
#endif
#ifdef HAVE_NETDEV_IF_MTU
	attr->active_mtu = ib_mtu_int_to_enum(ndev->if_mtu);
#else
	attr->active_mtu = ib_mtu_int_to_enum(ndev->mtu);
#endif
	attr->gid_tbl_len = IONIC_GID_TBL_LEN;
#ifdef HAVE_PORT_ATTR_IP_GIDS
	attr->ip_gids = true;
	attr->port_cap_flags = 0;
#else
	attr->port_cap_flags = IB_PORT_IP_BASED_GIDS;
#endif
	attr->max_msg_sz = 0x80000000;
	attr->pkey_tbl_len = IONIC_PKEY_TBL_LEN;
	attr->max_vl_num = 1;
	attr->subnet_prefix = 0xfe80000000000000ull;

	return ib_get_eth_speed(ibdev, port,
				&attr->active_speed,
				&attr->active_width);
}

static enum rdma_link_layer ionic_get_link_layer(struct ib_device *ibdev,
						 u8 port)
{
	return IB_LINK_LAYER_ETHERNET;
}

static struct net_device *ionic_get_netdev(struct ib_device *ibdev, u8 port)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibdev);

	if (port != 1)
		return ERR_PTR(-EINVAL);

	dev_hold(dev->ndev);

	return dev->ndev;
}

static int ionic_query_gid(struct ib_device *ibdev, u8 port, int index,
			   union ib_gid *gid)
{
	int rc;

	rc = ib_get_cached_gid(ibdev, port, index, gid, NULL);
	if (rc == -EAGAIN) {
		memcpy(gid, &zgid, sizeof(*gid));
		return 0;
	}

	return rc;
}

static int ionic_add_gid(struct ib_device *ibdev, u8 port,
			 unsigned int index, const union ib_gid *gid,
			 const struct ib_gid_attr *attr, void **context)
{
	if (attr->gid_type == IB_GID_TYPE_IB)
		return -EINVAL;

	return 0;
}

static int ionic_del_gid(struct ib_device *ibdev, u8 port,
			 unsigned int index, void **context)
{
	return 0;
}

static int ionic_query_pkey(struct ib_device *ibdev, u8 port, u16 index,
			    u16 *pkey)
{
	if (port != 1)
		return -EINVAL;

	if (index != 0)
		return -EINVAL;

	*pkey = 0xffff;

	return 0;
}

static int ionic_modify_device(struct ib_device *ibdev, int mask,
			       struct ib_device_modify *attr)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibdev);

	if (mask & ~IB_DEVICE_MODIFY_NODE_DESC)
		return -EOPNOTSUPP;

	if (mask & IB_DEVICE_MODIFY_NODE_DESC)
		memcpy(dev->ibdev.node_desc, attr->node_desc,
		       IB_DEVICE_NODE_DESC_MAX);

	return 0;
}

static int ionic_modify_port(struct ib_device *ibdev, u8 port, int mask,
			     struct ib_port_modify *attr)
{
	return 0;
}

static struct ib_ucontext *ionic_alloc_ucontext(struct ib_device *ibdev,
						struct ib_udata *udata)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibdev);
	struct ionic_ctx *ctx;
	struct ionic_ctx_req req;
	struct ionic_ctx_resp resp = {0};
	phys_addr_t db_phys = 0;
	int rc;

	rc = ionic_validate_udata(udata, sizeof(req), sizeof(resp));
	if (!rc)
		rc = ib_copy_from_udata(&req, udata, sizeof(req));

	if (rc)
		goto err_ctx;

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx) {
		rc = -ENOMEM;
		goto err_ctx;
	}

	/* try to allocate dbid for user ctx */
	rc = ionic_api_get_dbid(dev->handle, &ctx->dbid, &db_phys);
	if (rc < 0)
		goto err_dbid;

	dev_dbg(&dev->ibdev.dev, "user space dbid %u\n", ctx->dbid);

	mutex_init(&ctx->mmap_mut);
	ctx->mmap_off = PAGE_SIZE;
	INIT_LIST_HEAD(&ctx->mmap_list);

	ctx->mmap_dbell.offset = 0;
	ctx->mmap_dbell.size = PAGE_SIZE;
	ctx->mmap_dbell.pfn = PHYS_PFN(db_phys);
	ctx->mmap_dbell.writecombine = false;
	list_add(&ctx->mmap_dbell.ctx_ent, &ctx->mmap_list);

	resp.page_shift = PAGE_SHIFT;

	resp.dbell_offset = 0;

	resp.version = dev->rdma_version;
	resp.qp_opcodes = dev->qp_opcodes;
	resp.admin_opcodes = dev->admin_opcodes;

	resp.sq_qtype = dev->sq_qtype;
	resp.rq_qtype = dev->rq_qtype;
	resp.cq_qtype = dev->cq_qtype;
	resp.admin_qtype = dev->aq_qtype;
	resp.max_stride = dev->max_stride;
	resp.max_spec = ionic_spec;

	rc = ib_copy_to_udata(udata, &resp, sizeof(resp));
	if (rc)
		goto err_resp;

	return &ctx->ibctx;

err_resp:
	ionic_api_put_dbid(dev->handle, ctx->dbid);
err_dbid:
	kfree(ctx);
err_ctx:
	return ERR_PTR(rc);
}

static int ionic_dealloc_ucontext(struct ib_ucontext *ibctx)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibctx->device);
	struct ionic_ctx *ctx = to_ionic_ctx(ibctx);

	list_del(&ctx->mmap_dbell.ctx_ent);

	if (WARN_ON(!list_empty(&ctx->mmap_list)))
		list_del(&ctx->mmap_list);

	ionic_api_put_dbid(dev->handle, ctx->dbid);
	kfree(ctx);

	return 0;
}

static int ionic_mmap(struct ib_ucontext *ibctx, struct vm_area_struct *vma)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibctx->device);
	struct ionic_ctx *ctx = to_ionic_ctx(ibctx);
	struct ionic_mmap_info *info;
	unsigned long size = vma->vm_end - vma->vm_start;
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
	int rc = 0;

	mutex_lock(&ctx->mmap_mut);

	list_for_each_entry(info, &ctx->mmap_list, ctx_ent)
		if (info->offset == offset)
			goto found;

	mutex_unlock(&ctx->mmap_mut);

	/* not found */
	dev_dbg(&dev->ibdev.dev, "not found %#lx\n", offset);
	rc = -EINVAL;
	goto out;

found:
	list_del_init(&info->ctx_ent);
	mutex_unlock(&ctx->mmap_mut);

	if (info->size != size) {
		dev_dbg(&dev->ibdev.dev, "invalid size %#lx (%#lx)\n",
			size, info->size);
		rc = -EINVAL;
		goto out;
	}

	dev_dbg(&dev->ibdev.dev, "writecombine? %d\n", info->writecombine);
	if (info->writecombine)
		vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	else
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	dev_dbg(&dev->ibdev.dev, "remap st %#lx pf %#lx sz %#lx\n",
		vma->vm_start, info->pfn, size);
	rc = io_remap_pfn_range(vma, vma->vm_start, info->pfn, size,
				vma->vm_page_prot);
	if (rc)
		dev_dbg(&dev->ibdev.dev, "remap failed %d\n", rc);

out:
	return rc;
}

static struct ib_pd *ionic_alloc_pd(struct ib_device *ibdev,
				    struct ib_ucontext *ibctx,
				    struct ib_udata *udata)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibdev);
	struct ionic_pd *pd;
	int rc;

	pd = kzalloc(sizeof(*pd), GFP_KERNEL);
	if (!pd) {
		rc = -ENOMEM;
		goto err_pd;
	}

	rc = ionic_get_pdid(dev, &pd->pdid);
	if (rc)
		goto err_pdid;

	return &pd->ibpd;

err_pdid:
	kfree(pd);
err_pd:
	return ERR_PTR(rc);
}

static int ionic_dealloc_pd(struct ib_pd *ibpd)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibpd->device);
	struct ionic_pd *pd = to_ionic_pd(ibpd);

	ionic_put_pdid(dev, pd->pdid);
	kfree(pd);

	return 0;
}

static int ionic_build_hdr(struct ionic_ibdev *dev,
			   struct ib_ud_header *hdr,
			   const struct rdma_ah_attr *attr,
			   bool want_ecn)
{
	const struct ib_global_route *grh;
	struct ib_gid_attr sgid_attr;
	union ib_gid sgid;
	union ib_gid dgid_copy;
	u8 smac[ETH_ALEN];
	enum rdma_network_type net;
	u16 vlan;
	int rc;

	if (attr->ah_flags != IB_AH_GRH)
		return -EINVAL;

	grh = rdma_ah_read_grh(attr);

	rc = ib_get_cached_gid(&dev->ibdev, 1, grh->sgid_index,
			       &sgid, &sgid_attr);
	if (rc)
		return rc;

	if (!sgid_attr.ndev)
		return -ENXIO;

	ether_addr_copy(smac, IF_LLADDR(sgid_attr.ndev));
	vlan = rdma_vlan_dev_vlan_id(sgid_attr.ndev);
	net = ib_gid_to_network_type(sgid_attr.gid_type, &sgid);

	dev_put(sgid_attr.ndev); /* hold from ib_get_cached_gid */
	sgid_attr.ndev = NULL;

	dgid_copy = grh->dgid;
	if (net != ib_gid_to_network_type(sgid_attr.gid_type, &dgid_copy))
		return -EINVAL;

	rc = ib_ud_header_init(0,	/* no payload */
			       0,	/* no lrh */
			       1,	/* yes eth */
			       vlan != 0xffff,
			       0,	/* no grh */
			       net == RDMA_NETWORK_IPV4 ? 4 : 6,
			       1,	/* yes udp */
			       0,	/* no imm */
			       hdr);
	if (rc)
		return rc;

	ether_addr_copy(hdr->eth.smac_h, smac);
	ether_addr_copy(hdr->eth.dmac_h, attr->dmac);

	if (net == RDMA_NETWORK_IPV4) {
		hdr->eth.type = cpu_to_be16(ETH_P_IP);
		hdr->ip4.frag_off = cpu_to_be16(0x4000); /* don't fragment */
		hdr->ip4.ttl = grh->hop_limit;
		hdr->ip4.tot_len = cpu_to_be16(0xffff);
		hdr->ip4.saddr = *(const __be32 *)(sgid.raw + 12);
		hdr->ip4.daddr = *(const __be32 *)(grh->dgid.raw + 12);

		if (want_ecn)
			hdr->ip4.tos = ionic_set_ecn(grh->traffic_class);
		else
			hdr->ip4.tos = ionic_clear_ecn(grh->traffic_class);
	} else {
		hdr->eth.type = cpu_to_be16(ETH_P_IPV6);
		hdr->grh.flow_label = cpu_to_be32(grh->flow_label);
		hdr->grh.hop_limit = grh->hop_limit;
		hdr->grh.source_gid = sgid;
		hdr->grh.destination_gid = grh->dgid;

		if (want_ecn)
			hdr->grh.traffic_class =
				ionic_set_ecn(grh->traffic_class);
		else
			hdr->grh.traffic_class =
				ionic_clear_ecn(grh->traffic_class);
	}

	if (vlan != 0xffff) {
		vlan |= attr->sl << 13; /* 802.1q PCP */
		hdr->vlan.tag = cpu_to_be16(vlan);
		hdr->vlan.type = hdr->eth.type;
		hdr->eth.type = cpu_to_be16(ETH_P_8021Q);
	}

	hdr->udp.sport = cpu_to_be16(IONIC_ROCE_UDP_SPORT);
	hdr->udp.dport = cpu_to_be16(ROCE_V2_UDP_DPORT);

	return 0;
}

static int ionic_set_ah_attr(struct ionic_ibdev *dev,
			     struct rdma_ah_attr *ah_attr,
			     void *hdr_buf, int sgid_index)

{
	struct ib_ud_header *hdr = NULL;
	u32 flow_label;
	u16 vlan = 0;
	u8  tos, ttl;
	int rc;

	hdr = kzalloc(sizeof(*hdr), GFP_KERNEL);
	if (!hdr) {
		rc = -ENOMEM;
		return rc;
	}

	rc = roce_ud_header_unpack(hdr_buf, hdr);
	if (rc)
		goto err_hdr;

	if (hdr->vlan_present)
		vlan = be16_to_cpu(hdr->vlan.tag);

	if (hdr->ipv4_present) {
		flow_label = 0;
		ttl = hdr->ip4.ttl;
		tos = hdr->ip4.tos;
		*(__be16 *)(hdr->grh.destination_gid.raw + 10) = 0xffff;
		*(__be32 *)(hdr->grh.destination_gid.raw + 12) =
			hdr->ip4.daddr;
	} else {
		flow_label = be32_to_cpu(hdr->grh.flow_label);
		ttl = hdr->grh.hop_limit;
		tos = hdr->grh.traffic_class;
	}

	memset(ah_attr, 0, sizeof(*ah_attr));
#ifdef HAVE_RDMA_AH_ATTR_TYPE_ROCE
	ah_attr->type = RDMA_AH_ATTR_TYPE_ROCE;
#endif
	if (hdr->eth_present) {
#ifdef HAVE_RDMA_AH_ATTR_TYPE_ROCE
		memcpy(&ah_attr->roce.dmac, &hdr->eth.dmac_h, ETH_ALEN);
#else
		memcpy(&ah_attr->dmac, &hdr->eth.dmac_h, ETH_ALEN);
#endif
	}
	rdma_ah_set_sl(ah_attr, vlan >> 13);
	rdma_ah_set_port_num(ah_attr, 1);
	rdma_ah_set_grh(ah_attr, NULL, flow_label, sgid_index, ttl, tos);
	rdma_ah_set_dgid_raw(ah_attr, &hdr->grh.destination_gid);

err_hdr:
	kfree(hdr);

	return rc;
}

static int ionic_create_ah_cmd(struct ionic_ibdev *dev,
			       struct ionic_ah *ah,
			       struct ionic_pd *pd,
			       struct rdma_ah_attr *attr,
			       u32 flags)
{
	struct ionic_admin_wr wr = {
		.work = COMPLETION_INITIALIZER_ONSTACK(wr.work),
		.wqe = {
			.op = IONIC_V1_ADMIN_CREATE_AH,
			.dbid_flags = cpu_to_le16(dev->dbid),
			.id_ver = cpu_to_le32(ah->ahid),
			.ah = {
				.pd_id = cpu_to_le32(pd->pdid),
			}
		}
	};
	struct ib_ud_header *hdr;
	dma_addr_t hdr_dma = 0;
	void *hdr_buf;
	gfp_t gfp = GFP_ATOMIC;
	int rc, hdr_len = 0;

	if (dev->admin_opcodes <= IONIC_V1_ADMIN_CREATE_AH)
		return -ENOSYS;

	if (flags & RDMA_CREATE_AH_SLEEPABLE)
		gfp = GFP_KERNEL;

	hdr = kmalloc(sizeof(*hdr), gfp);
	if (!hdr) {
		rc = -ENOMEM;
		goto err_hdr;
	}

	rc = ionic_build_hdr(dev, hdr, attr, false);
	if (rc)
		goto err_buf;

	ah->sgid_index = rdma_ah_read_grh(attr)->sgid_index;

	hdr_buf = contig_kmalloc(PAGE_SIZE, gfp);
	if (!hdr_buf) {
		rc = -ENOMEM;
		goto err_buf;
	}

	hdr_len = ib_ud_header_pack(hdr, hdr_buf);
	hdr_len -= IB_BTH_BYTES;
	hdr_len -= IB_DETH_BYTES;

	dev_dbg(&dev->ibdev.dev, "roce packet header template\n");
	print_hex_dump_debug("hdr ", DUMP_PREFIX_OFFSET, 16, 1,
			     hdr_buf, hdr_len, true);

	hdr_dma = dma_map_single(dev->hwdev, hdr_buf, hdr_len,
				 DMA_TO_DEVICE);

	rc = dma_mapping_error(dev->hwdev, hdr_dma);
	if (rc)
		goto err_dma;

	wr.wqe.ah.dma_addr = cpu_to_le64(hdr_dma);
	wr.wqe.ah.length = cpu_to_le32(hdr_len);

	ionic_admin_post(dev, &wr);

	if (flags & RDMA_CREATE_AH_SLEEPABLE) {
		ionic_admin_wait(&wr);
		rc = 0;
	} else {
		rc = ionic_admin_busy_wait(&wr);
	}

	if (rc) {
		dev_warn(&dev->ibdev.dev, "wait status %d\n", rc);
		ionic_admin_cancel(&wr);
	} else if (wr.status == IONIC_ADMIN_KILLED) {
		dev_dbg(&dev->ibdev.dev, "killed\n");
		rc = -ENODEV;
	} else if (ionic_v1_cqe_error(&wr.cqe)) {
		dev_warn(&dev->ibdev.dev, "error %u\n",
			 be32_to_cpu(wr.cqe.status_length));
		rc = -EINVAL;
	} else {
		rc = 0;
	}

	dma_unmap_single(dev->hwdev, hdr_dma, hdr_len,
			 DMA_TO_DEVICE);
err_dma:
	contig_kfree(hdr_buf, PAGE_SIZE);
err_buf:
	kfree(hdr);
err_hdr:
	return rc;
}

static int ionic_query_ah_cmd(struct ionic_ibdev *dev,
			      struct ionic_ah *ah,
			      struct rdma_ah_attr *ah_attr)
{
	struct ionic_admin_wr wr = {
		.work = COMPLETION_INITIALIZER_ONSTACK(wr.work),
		.wqe = {
			.op = IONIC_V1_ADMIN_QUERY_AH,
			.id_ver = cpu_to_le32(ah->ahid),
		}
	};
	dma_addr_t hdr_dma;
	void *hdr_buf = NULL;
	int rc;

	if (dev->admin_opcodes <= IONIC_V1_ADMIN_QUERY_AH)
		return -ENOSYS;

	hdr_buf = contig_kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!hdr_buf) {
		rc = -ENOMEM;
		goto err_buf;
	}

	hdr_dma = dma_map_single(dev->hwdev, hdr_buf,
				 PAGE_SIZE, DMA_TO_DEVICE);
	rc = dma_mapping_error(dev->hwdev, hdr_dma);
	if (rc)
		goto err_dma;

	wr.wqe.query_ah.dma_addr = cpu_to_le64(hdr_dma);

	ionic_admin_post(dev, &wr);
	ionic_admin_wait(&wr);

	if (wr.status == IONIC_ADMIN_KILLED) {
		dev_dbg(&dev->ibdev.dev, "killed\n");
		rc = -ENODEV;
	} else if (ionic_v1_cqe_error(&wr.cqe)) {
		dev_warn(&dev->ibdev.dev, "cqe error %u\n",
			 be32_to_cpu(wr.cqe.status_length));
		rc = -EINVAL;
	} else {
		rc = 0;
	}

	dma_unmap_single(dev->hwdev, hdr_dma, PAGE_SIZE, DMA_FROM_DEVICE);

	if (!rc)
		rc = ionic_set_ah_attr(dev, ah_attr, hdr_buf, ah->sgid_index);

err_dma:
	contig_kfree(hdr_buf, PAGE_SIZE);
err_buf:
	return rc;
}

static int ionic_destroy_ah_cmd(struct ionic_ibdev *dev, u32 ahid, u32 flags)
{
	struct ionic_admin_wr wr = {
		.work = COMPLETION_INITIALIZER_ONSTACK(wr.work),
		.wqe = {
			.op = IONIC_V1_ADMIN_DESTROY_AH,
			.id_ver = cpu_to_le32(ahid),
		}
	};
	int rc;

	if (dev->admin_opcodes <= IONIC_V1_ADMIN_DESTROY_AH)
		return -ENOSYS;

	ionic_admin_post(dev, &wr);

	if (flags & RDMA_CREATE_AH_SLEEPABLE) {
		ionic_admin_wait(&wr);
		rc = 0;
	} else {
		rc = ionic_admin_busy_wait(&wr);
	}

	if (rc) {
		dev_warn(&dev->ibdev.dev, "wait status %d\n", rc);
		ionic_admin_cancel(&wr);

		/* No host-memory resource is associated with ah, so it is ok
		 * to "succeed" and complete this destroy ah on the host.
		 */
		rc = 0;
	} else if (wr.status == IONIC_ADMIN_KILLED) {
		dev_dbg(&dev->ibdev.dev, "killed\n");
		rc = 0;
	} else if (ionic_v1_cqe_error(&wr.cqe)) {
		dev_warn(&dev->ibdev.dev, "error %u\n",
			 be32_to_cpu(wr.cqe.status_length));
		rc = -EINVAL;
	} else {
		rc = 0;
	}

	return rc;
}

static struct ib_ah *ionic_create_ah(struct ib_pd *ibpd,
				     struct rdma_ah_attr *attr,
				     struct ib_udata *udata)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibpd->device);
	struct ionic_pd *pd = to_ionic_pd(ibpd);
	struct ionic_ah *ah;
	struct ionic_ctx *ctx = to_ionic_ctx_uobj(pd->ibpd.uobject);
	struct ionic_ah_resp resp = {0};
	int rc;
	u32 flags = 0;

	if (udata) {
		rc = ib_resolve_eth_dmac(&dev->ibdev, attr);
		if (rc)
			goto err_ah;
	}

	if (ctx)
		rc = ionic_validate_udata(udata, 0, sizeof(resp));
	else
		rc = ionic_validate_udata(udata, 0, 0);
	if (rc)
		goto err_ah;

	ah = kzalloc(sizeof(*ah),
		     (flags & RDMA_CREATE_AH_SLEEPABLE) ?
		     GFP_KERNEL : GFP_ATOMIC);
	if (!ah) {
		rc = -ENOMEM;
		goto err_ah;
	}

	rc = ionic_get_ahid(dev, &ah->ahid);
	if (rc)
		goto err_ahid;

	rc = ionic_create_ah_cmd(dev, ah, pd, attr, flags);
	if (rc)
		goto err_cmd;

	if (ctx) {
		resp.ahid = ah->ahid;

		rc = ib_copy_to_udata(udata, &resp, sizeof(resp));
		if (rc)
			goto err_resp;
	}

	return &ah->ibah;

err_resp:
	ionic_destroy_ah_cmd(dev, ah->ahid, flags);
err_cmd:
	ionic_put_ahid(dev, ah->ahid);
err_ahid:
	kfree(ah);
err_ah:
	return ERR_PTR(rc);
}

static int ionic_query_ah(struct ib_ah *ibah,
			  struct rdma_ah_attr *ah_attr)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibah->device);
	struct ionic_ah *ah = to_ionic_ah(ibah);
	int rc;

	rc = ionic_query_ah_cmd(dev, ah, ah_attr);
	if (rc)
		return rc;

	return 0;
}

static int ionic_destroy_ah(struct ib_ah *ibah)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibah->device);
	struct ionic_ah *ah = to_ionic_ah(ibah);
	int rc;
	u32 flags = 0;

	rc = ionic_destroy_ah_cmd(dev, ah->ahid, flags);
	if (rc) {
		dev_warn(&dev->ibdev.dev, "destroy_ah error %d\n", rc);
		return rc;
	}

	ionic_put_ahid(dev, ah->ahid);
	kfree(ah);

	return 0;
}

static int ionic_create_mr_cmd(struct ionic_ibdev *dev, struct ionic_pd *pd,
			       struct ionic_mr *mr, u64 addr, u64 length)
{
	struct ionic_admin_wr wr = {
		.work = COMPLETION_INITIALIZER_ONSTACK(wr.work),
		.wqe = {
			.op = IONIC_V1_ADMIN_CREATE_MR,
			.dbid_flags = cpu_to_le16(mr->flags),
			.id_ver = cpu_to_le32(mr->mrid),
			.mr = {
				.va = cpu_to_le64(addr),
				.length = cpu_to_le64(length),
				.pd_id = cpu_to_le32(pd->pdid),
				.page_size_log2 = mr->buf.page_size_log2,
				.tbl_index = cpu_to_le32(mr->res.tbl_pos),
				.map_count = cpu_to_le32(mr->buf.tbl_pages),
				//.offset = ionic_pgtbl_off(&mr->buf, addr),
				.dma_addr = ionic_pgtbl_dma(&mr->buf, addr),
			}
		}
	};
	int rc;

	if (dev->admin_opcodes <= IONIC_V1_ADMIN_CREATE_MR)
		return -ENOSYS;

	ionic_admin_post(dev, &wr);
	ionic_admin_wait(&wr);

	if (wr.status == IONIC_ADMIN_KILLED) {
		dev_dbg(&dev->ibdev.dev, "killed\n");
		rc = -ENODEV;
	} else if (ionic_v1_cqe_error(&wr.cqe)) {
		dev_warn(&dev->ibdev.dev, "cqe error %u\n",
			 be32_to_cpu(wr.cqe.status_length));
		rc = -EINVAL;
	} else {
		mr->created = true;
		rc = 0;
	}

	return rc;
}

static int ionic_destroy_mr_cmd(struct ionic_ibdev *dev, u32 mrid)
{
	struct ionic_admin_wr wr = {
		.work = COMPLETION_INITIALIZER_ONSTACK(wr.work),
		.wqe = {
			.op = IONIC_V1_ADMIN_DESTROY_MR,
			.id_ver = cpu_to_le32(mrid),
		}
	};
	int rc;

	if (dev->admin_opcodes <= IONIC_V1_ADMIN_DESTROY_MR)
		return -ENOSYS;

	ionic_admin_post(dev, &wr);
	ionic_admin_wait(&wr);

	if (wr.status == IONIC_ADMIN_KILLED) {
		dev_dbg(&dev->ibdev.dev, "killed\n");
		rc = 0;
	} else if (ionic_v1_cqe_error(&wr.cqe)) {
		dev_warn(&dev->ibdev.dev, "cqe error %u\n",
			 be32_to_cpu(wr.cqe.status_length));
		rc = -EINVAL;
	} else {
		rc = 0;
	}

	return rc;
}

static struct ib_mr *ionic_get_dma_mr(struct ib_pd *ibpd, int access)
{
	struct ionic_mr *mr;

	mr = kzalloc(sizeof(*mr), GFP_KERNEL);
	if (!mr)
		return ERR_PTR(-ENOMEM);

	return &mr->ibmr;
}

static struct ib_mr *ionic_reg_user_mr(struct ib_pd *ibpd, u64 start,
				       u64 length, u64 addr, int access,
				       struct ib_udata *udata)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibpd->device);
	struct ionic_pd *pd = to_ionic_pd(ibpd);
	struct ionic_mr *mr;
	int rc;

	rc = ionic_validate_udata(udata, 0, 0);
	if (rc)
		goto err_mr;

	mr = kzalloc(sizeof(*mr), GFP_KERNEL);
	if (!mr) {
		rc = -ENOMEM;
		goto err_mr;
	}

	rc = ionic_get_mrid(dev, &mr->mrid);
	if (rc)
		goto err_mrid;

	mr->ibmr.lkey = mr->mrid;
	mr->ibmr.rkey = mr->mrid;
	mr->ibmr.iova = addr;
	mr->ibmr.length = length;

	mr->flags = IONIC_MRF_USER_MR | to_ionic_mr_flags(access);

	mr->umem = ib_umem_get(ibpd->uobject->context,
			       start, length, access, 0);
	if (IS_ERR(mr->umem)) {
		rc = PTR_ERR(mr->umem);
		goto err_umem;
	}

	rc = ionic_pgtbl_init(dev, &mr->res, &mr->buf, mr->umem, 0, 1);
	if (rc)
		goto err_pgtbl;

	rc = ionic_create_mr_cmd(dev, pd, mr, addr, length);
	if (rc)
		goto err_cmd;

	ionic_pgtbl_unbuf(dev, &mr->buf);

	ionic_dbg_add_mr(dev, mr);

	return &mr->ibmr;

err_cmd:
	ionic_pgtbl_unbuf(dev, &mr->buf);
	ionic_put_res(dev, &mr->res);
err_pgtbl:
	ib_umem_release(mr->umem);
err_umem:
	ionic_put_mrid(dev, mr->mrid);
err_mrid:
	kfree(mr);
err_mr:
	return ERR_PTR(rc);
}

static int ionic_rereg_user_mr(struct ib_mr *ibmr, int flags, u64 start,
			       u64 length, u64 addr, int access,
			       struct ib_pd *ibpd, struct ib_udata *udata)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibmr->device);
	struct ionic_mr *mr = to_ionic_mr(ibmr);
	struct ionic_pd *pd;
	int rc;

	if (!mr->ibmr.lkey)
		return -EINVAL;

	if (!mr->created) {
		/* must set translation if not already on device */
		if (~flags & IB_MR_REREG_TRANS)
			return -EINVAL;
	} else {
		/* destroy on device first if not already on device */
		rc = ionic_destroy_mr_cmd(dev, mr->mrid);
		if (rc)
			return rc;

		mr->created = false;
	}

	if (~flags & IB_MR_REREG_PD)
		ibpd = mr->ibmr.pd;
	pd = to_ionic_pd(ibpd);

	mr->mrid = ib_inc_rkey(mr->mrid);
	mr->ibmr.lkey = mr->mrid;
	mr->ibmr.rkey = mr->mrid;

	if (flags & IB_MR_REREG_ACCESS)
		mr->flags = IONIC_MRF_USER_MR | to_ionic_mr_flags(access);

	if (flags & IB_MR_REREG_TRANS) {
		ionic_pgtbl_unbuf(dev, &mr->buf);
		ionic_put_res(dev, &mr->res);

		if (mr->umem)
			ib_umem_release(mr->umem);

		mr->ibmr.iova = addr;
		mr->ibmr.length = length;

		mr->umem = ib_umem_get(ibpd->uobject->context, start,
				       length, access, 0);
		if (IS_ERR(mr->umem)) {
			rc = PTR_ERR(mr->umem);
			goto err_umem;
		}

		rc = ionic_pgtbl_init(dev, &mr->res, &mr->buf, mr->umem, 0, 1);
		if (rc)
			goto err_pgtbl;
	}

	rc = ionic_create_mr_cmd(dev, pd, mr, addr, length);
	if (rc)
		goto err_cmd;

	ionic_pgtbl_unbuf(dev, &mr->buf);

	return 0;

err_cmd:
	ionic_pgtbl_unbuf(dev, &mr->buf);
	ionic_put_res(dev, &mr->res);
err_pgtbl:
	ib_umem_release(mr->umem);
	mr->umem = NULL;
err_umem:
	mr->res.tbl_order = IONIC_RES_INVALID;
	return rc;
}

static int ionic_dereg_mr(struct ib_mr *ibmr)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibmr->device);
	struct ionic_mr *mr = to_ionic_mr(ibmr);
	int rc;

	if (!mr->ibmr.lkey)
		goto out;

	if (mr->created) {
		rc = ionic_destroy_mr_cmd(dev, mr->mrid);
		if (rc)
			return rc;
	}

	ionic_dbg_rm_mr(mr);

	ionic_pgtbl_unbuf(dev, &mr->buf);
	ionic_put_res(dev, &mr->res);

	if (mr->umem)
		ib_umem_release(mr->umem);

	ionic_put_mrid(dev, mr->mrid);

out:
	kfree(mr);

	return 0;
}

static struct ib_mr *ionic_alloc_mr(struct ib_pd *ibpd,
				    enum ib_mr_type type,
				    u32 max_sg)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibpd->device);
	struct ionic_pd *pd = to_ionic_pd(ibpd);
	struct ionic_mr *mr;
	int rc;

	if (type != IB_MR_TYPE_MEM_REG) {
		rc = -EINVAL;
		goto err_mr;
	}

	mr = kzalloc(sizeof(*mr), GFP_KERNEL);
	if (!mr) {
		rc = -ENOMEM;
		goto err_mr;
	}

	rc = ionic_get_mrid(dev, &mr->mrid);
	if (rc)
		goto err_mrid;

	mr->ibmr.lkey = mr->mrid;
	mr->ibmr.rkey = mr->mrid;

	mr->flags = IONIC_MRF_PHYS_MR;

	rc = ionic_pgtbl_init(dev, &mr->res, &mr->buf, mr->umem, 0, max_sg);
	if (rc)
		goto err_pgtbl;

	mr->buf.tbl_pages = 0;

	rc = ionic_create_mr_cmd(dev, pd, mr, 0, 0);
	if (rc)
		goto err_cmd;

	ionic_dbg_add_mr(dev, mr);

	return &mr->ibmr;

err_cmd:
	ionic_pgtbl_unbuf(dev, &mr->buf);
	ionic_put_res(dev, &mr->res);
err_pgtbl:
	ionic_put_mrid(dev, mr->mrid);
err_mrid:
	kfree(mr);
err_mr:
	return ERR_PTR(rc);
}

static int ionic_map_mr_page(struct ib_mr *ibmr, u64 dma)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibmr->device);
	struct ionic_mr *mr = to_ionic_mr(ibmr);

	dev_dbg(&dev->ibdev.dev, "dma %p\n", (void *)dma);
	return ionic_pgtbl_page(&mr->buf, dma);
}

static int ionic_map_mr_sg(struct ib_mr *ibmr, struct scatterlist *sg,
			   int sg_nents, unsigned int *sg_offset)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibmr->device);
	struct ionic_mr *mr = to_ionic_mr(ibmr);
	int rc;

	/* mr must be allocated using ib_alloc_mr() */
	if (unlikely(!mr->buf.tbl_limit))
		return -EINVAL;

	mr->buf.tbl_pages = 0;

	if (mr->buf.tbl_buf)
		dma_sync_single_for_cpu(dev->hwdev, mr->buf.tbl_dma,
					mr->buf.tbl_size, DMA_TO_DEVICE);

	dev_dbg(&dev->ibdev.dev, "sg %p nent %d\n", sg, sg_nents);
	rc = ib_sg_to_pages(ibmr, sg, sg_nents, sg_offset, ionic_map_mr_page);

	mr->buf.page_size_log2 = order_base_2(ibmr->page_size);

	if (mr->buf.tbl_buf)
		dma_sync_single_for_device(dev->hwdev, mr->buf.tbl_dma,
					   mr->buf.tbl_size, DMA_TO_DEVICE);

	return rc;
}

static struct ib_mw *ionic_alloc_mw(struct ib_pd *ibpd, enum ib_mw_type type,
				    struct ib_udata *udata)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibpd->device);
	struct ionic_pd *pd = to_ionic_pd(ibpd);
	struct ionic_mr *mr;
	int rc;

	mr = kzalloc(sizeof(*mr), GFP_KERNEL);
	if (!mr) {
		rc = -ENOMEM;
		goto err_mr;
	}

	rc = ionic_get_mrid(dev, &mr->mrid);
	if (rc)
		goto err_mrid;

	mr->ibmw.rkey = mr->mrid;
	mr->ibmw.type = type;

	if (type == IB_MW_TYPE_1)
		mr->flags = IONIC_MRF_MW_1;
	else
		mr->flags = IONIC_MRF_MW_2;

	rc = ionic_create_mr_cmd(dev, pd, mr, 0, 0);
	if (rc)
		goto err_cmd;

	ionic_dbg_add_mr(dev, mr);

	return &mr->ibmw;

err_cmd:
	ionic_put_mrid(dev, mr->mrid);
err_mrid:
	kfree(mr);
err_mr:
	return ERR_PTR(rc);
}

static int ionic_dealloc_mw(struct ib_mw *ibmw)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibmw->device);
	struct ionic_mr *mr = to_ionic_mw(ibmw);
	int rc;

	rc = ionic_destroy_mr_cmd(dev, mr->mrid);
	if (rc)
		return rc;

	ionic_dbg_rm_mr(mr);

	ionic_put_mrid(dev, mr->mrid);

	kfree(mr);

	return 0;
}

static int ionic_create_cq_cmd(struct ionic_ibdev *dev,
			       struct ionic_ctx *ctx,
			       struct ionic_cq *cq,
			       struct ionic_tbl_buf *buf)
{
	const u16 dbid = ionic_ctx_dbid(dev, ctx);
	struct ionic_admin_wr wr = {
		.work = COMPLETION_INITIALIZER_ONSTACK(wr.work),
		.wqe = {
			.op = IONIC_V1_ADMIN_CREATE_CQ,
			.dbid_flags = cpu_to_le16(dbid),
			.id_ver = cpu_to_le32(cq->cqid),
			.cq = {
				.eq_id = cpu_to_le32(cq->eqid),
				.depth_log2 = cq->q.depth_log2,
				.stride_log2 = cq->q.stride_log2,
				.page_size_log2 = buf->page_size_log2,
				.tbl_index = cpu_to_le32(cq->res.tbl_pos),
				.map_count = cpu_to_le32(buf->tbl_pages),
				.dma_addr = ionic_pgtbl_dma(buf, 0),
			}
		}
	};
	int rc;

	if (dev->admin_opcodes <= IONIC_V1_ADMIN_CREATE_CQ)
		return -ENOSYS;

	ionic_admin_post(dev, &wr);
	ionic_admin_wait(&wr);

	if (wr.status == IONIC_ADMIN_KILLED) {
		dev_dbg(&dev->ibdev.dev, "killed\n");
		rc = -ENODEV;
	} else if (ionic_v1_cqe_error(&wr.cqe)) {
		dev_warn(&dev->ibdev.dev, "cqe error %u\n",
			 be32_to_cpu(wr.cqe.status_length));
		rc = -EINVAL;
	} else {
		rc = 0;
	}

	return rc;
}

static int ionic_destroy_cq_cmd(struct ionic_ibdev *dev, u32 cqid)
{
	struct ionic_admin_wr wr = {
		.work = COMPLETION_INITIALIZER_ONSTACK(wr.work),
		.wqe = {
			.op = IONIC_V1_ADMIN_DESTROY_CQ,
			.id_ver = cpu_to_le32(cqid),
		}
	};
	int rc;

	if (dev->admin_opcodes <= IONIC_V1_ADMIN_DESTROY_CQ)
		return -ENOSYS;

	ionic_admin_post(dev, &wr);
	ionic_admin_wait(&wr);

	if (wr.status == IONIC_ADMIN_KILLED) {
		dev_dbg(&dev->ibdev.dev, "killed\n");
		rc = 0;
	} else if (ionic_v1_cqe_error(&wr.cqe)) {
		dev_warn(&dev->ibdev.dev, "cqe error %u\n",
			 be32_to_cpu(wr.cqe.status_length));
		rc = -EINVAL;
	} else {
		rc = 0;
	}

	return rc;
}

static void ionic_cq_add_ref(struct ionic_cq *cq)
{
	refcount_inc(&cq->cq_refcnt);
}

static void ionic_cq_rem_ref(struct ionic_cq *cq)
{
	if (refcount_dec_and_test(&cq->cq_refcnt))
		complete(&cq->cq_rel_comp);
}

static int __ionic_create_cq(struct ionic_cq *cq,
			     struct ionic_tbl_buf *buf,
			     const struct ib_cq_init_attr *attr,
			     struct ib_ucontext *ibctx,
			     struct ib_udata *udata)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(cq->ibcq.device);
	struct ionic_ctx *ctx = to_ionic_ctx(ibctx);
	struct ionic_cq_req req;
	struct ionic_cq_resp resp;
	int rc, eq_idx;
	unsigned long irqflags;

	if (!ctx) {
		rc = ionic_validate_udata(udata, 0, 0);
	} else {
		rc = ionic_validate_udata(udata, sizeof(req), sizeof(resp));
		if (!rc)
			rc = ib_copy_from_udata(&req, udata, sizeof(req));
	}

	if (rc)
		goto err_args;

	if (attr->cqe < 1 || attr->cqe + IONIC_CQ_GRACE > 0xffff) {
		rc = -EINVAL;
		goto err_args;
	}

	rc = ionic_get_cqid(dev, &cq->cqid);
	if (rc)
		goto err_cqid;

	/* the first eq is reserved for async events */
	eq_idx = attr->comp_vector;
	if (eq_idx >= dev->eq_count - 1)
		eq_idx %= dev->eq_count - 1;
	eq_idx += 1;

	cq->eqid = dev->eq_vec[eq_idx]->eqid;

	spin_lock_init(&cq->lock);
	INIT_LIST_HEAD(&cq->poll_sq);
	INIT_LIST_HEAD(&cq->flush_sq);
	INIT_LIST_HEAD(&cq->flush_rq);

	if (!ctx) {
		rc = ionic_queue_init(&cq->q, dev->hwdev,
				      attr->cqe + IONIC_CQ_GRACE,
				      sizeof(struct ionic_v1_cqe));
		if (rc)
			goto err_q;

		ionic_queue_dbell_init(&cq->q, cq->cqid);
		cq->color = true;
		cq->reserve = cq->q.mask;
	} else {
		rc = ionic_validate_qdesc(&req.cq);
		if (rc)
			goto err_q;

		cq->umem = ib_umem_get(ibctx, req.cq.addr, req.cq.size,
				       IB_ACCESS_LOCAL_WRITE, 0);
		if (IS_ERR(cq->umem)) {
			rc = PTR_ERR(cq->umem);
			goto err_q;
		}

		cq->q.ptr = NULL;
		cq->q.size = req.cq.size;
		cq->q.mask = req.cq.mask;
		cq->q.depth_log2 = req.cq.depth_log2;
		cq->q.stride_log2 = req.cq.stride_log2;

		resp.cqid = cq->cqid;

		rc = ib_copy_to_udata(udata, &resp, sizeof(resp));
		if (rc)
			goto err_resp;
	}

	rc = ionic_pgtbl_init(dev, &cq->res, buf, cq->umem, cq->q.dma, 1);
	if (rc)
		goto err_resp;

	init_completion(&cq->cq_rel_comp);
	refcount_set(&cq->cq_refcnt, 1);

	rc = xa_err(xa_store_irq(&dev->cq_tbl, cq->cqid, cq, GFP_KERNEL));
	if (rc)
		goto err_xa;

	spin_lock_irqsave(&dev->dev_lock, irqflags);
	list_add_tail(&cq->cq_list_ent, &dev->cq_list);
	spin_unlock_irqrestore(&dev->dev_lock, irqflags);

	ionic_dbg_add_cq(dev, cq);

	return 0;

err_xa:
	ionic_pgtbl_unbuf(dev, buf);
	ionic_put_res(dev, &cq->res);
err_resp:
	if (cq->umem)
		ib_umem_release(cq->umem);
	else
		ionic_queue_destroy(&cq->q, dev->hwdev);
err_q:
	ionic_put_cqid(dev, cq->cqid);
err_cqid:
err_args:
	return rc;
}

static void __ionic_destroy_cq(struct ionic_ibdev *dev, struct ionic_cq *cq)
{
	unsigned long irqflags;

	spin_lock_irqsave(&dev->dev_lock, irqflags);
	list_del(&cq->cq_list_ent);
	spin_unlock_irqrestore(&dev->dev_lock, irqflags);

	xa_erase_irq(&dev->cq_tbl, cq->cqid);

	ionic_cq_rem_ref(cq);
	wait_for_completion(&cq->cq_rel_comp);

	ionic_dbg_rm_cq(cq);

	ionic_put_res(dev, &cq->res);

	if (cq->umem)
		ib_umem_release(cq->umem);
	else
		ionic_queue_destroy(&cq->q, dev->hwdev);

	ionic_put_cqid(dev, cq->cqid);
}

static struct ib_cq *ionic_create_cq(struct ib_device *ibdev,
				     const struct ib_cq_init_attr *attr,
				     struct ib_ucontext *ibctx,
				     struct ib_udata *udata)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibdev);
	struct ionic_cq *cq;
	struct ionic_ctx *ctx = to_ionic_ctx(ibctx);
	struct ionic_tbl_buf buf = {0};
	int rc;

	cq = kzalloc(sizeof(*cq), GFP_KERNEL);
	if (!cq) {
		rc = -ENOMEM;
		goto err_alloc;
	}
	cq->ibcq.device = ibdev;

	rc = __ionic_create_cq(cq, &buf, attr, &ctx->ibctx, udata);
	if (rc)
		goto err_init;

	rc = ionic_create_cq_cmd(dev, ctx, cq, &buf);
	if (rc)
		goto err_cmd;

	ionic_pgtbl_unbuf(dev, &buf);

	return &cq->ibcq;

err_cmd:
	ionic_pgtbl_unbuf(dev, &buf);
	__ionic_destroy_cq(dev, cq);
err_init:
	kfree(cq);
err_alloc:
	return ERR_PTR(rc);
}

static int ionic_destroy_cq(struct ib_cq *ibcq)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibcq->device);
	struct ionic_cq *cq = to_ionic_cq(ibcq);
	int rc;

	rc = ionic_destroy_cq_cmd(dev, cq->cqid);
	if (rc) {
		dev_warn(&dev->ibdev.dev, "destroy_cq error %d\n", rc);
		return rc;
	}

	__ionic_destroy_cq(dev, cq);
	kfree(cq);

	return 0;
}

static int ionic_flush_recv(struct ionic_qp *qp, struct ib_wc *wc)
{
	struct ionic_v1_wqe *wqe;
	struct ionic_rq_meta *meta;

	if (!qp->rq_flush)
		return 0;

	if (ionic_queue_empty(&qp->rq))
		return 0;

	/* This depends on the RQ polled in-order.  It does not work for SRQ,
	 * which can be polled out-of-order.  Driver does not flush SRQ.
	 */
	wqe = ionic_queue_at_cons(&qp->rq);

	/* wqe_id must be a valid queue index */
	if (unlikely(wqe->base.wqe_id >> qp->rq.depth_log2)) {
		dev_warn(&qp->ibqp.device->dev,
			 "flush qp %u recv index %llu invalid\n",
			 qp->qpid, (unsigned long long)wqe->base.wqe_id);
		return -EIO;
	}

	/* wqe_id must indicate a request that is outstanding */
	meta = &qp->rq_meta[wqe->base.wqe_id];
	if (unlikely(meta->next != IONIC_META_POSTED)) {
		dev_warn(&qp->ibqp.device->dev,
			 "flush qp %u recv index %llu not posted\n",
			 qp->qpid, (unsigned long long)wqe->base.wqe_id);
		return -EIO;
	}

	ionic_queue_consume(&qp->rq);

	memset(wc, 0, sizeof(*wc));

	wc->status = IB_WC_WR_FLUSH_ERR;
	wc->wr_id = meta->wrid;
	wc->qp = &qp->ibqp;

	meta->next = qp->rq_meta_head;
	qp->rq_meta_head = meta;

	return 1;
}

static int ionic_flush_recv_many(struct ionic_qp *qp,
				 struct ib_wc *wc, int nwc)
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

static int ionic_flush_send(struct ionic_qp *qp, struct ib_wc *wc)
{
	struct ionic_sq_meta *meta;

	if (!qp->sq_flush)
		return 0;

	if (ionic_queue_empty(&qp->sq))
		return 0;

	meta = &qp->sq_meta[qp->sq.cons];

	ionic_queue_consume(&qp->sq);

	memset(wc, 0, sizeof(*wc));

	wc->status = IB_WC_WR_FLUSH_ERR;
	wc->wr_id = meta->wrid;
	wc->qp = &qp->ibqp;

	return 1;
}

static int ionic_flush_send_many(struct ionic_qp *qp,
				 struct ib_wc *wc, int nwc)
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

static int ionic_poll_recv(struct ionic_ibdev *dev, struct ionic_cq *cq,
			   struct ionic_qp *cqe_qp, struct ionic_v1_cqe *cqe,
			   struct ib_wc *wc)
{
	struct ionic_qp *qp = NULL;
	struct ionic_rq_meta *meta;
	u32 src_qpn, st_len;
	u16 vlan_tag;
	u8 op;

	if (cqe_qp->rq_flush)
		return 0;

	if (cqe_qp->has_rq) {
		qp = cqe_qp;
	} else {
		if (unlikely(cqe_qp->is_srq)) {
			dev_warn(&dev->ibdev.dev, "cqe_qp %u srq with no rq\n",
				 cqe_qp->qpid);
			return -EIO;
		}

		if (unlikely(!cqe_qp->ibqp.srq)) {
			dev_warn(&dev->ibdev.dev, "cqe_qp %u no rq or srq\n",
				 cqe_qp->qpid);
			return -EIO;
		}

		qp = to_ionic_srq(cqe_qp->ibqp.srq);
	}

	st_len = be32_to_cpu(cqe->status_length);

	/* ignore wqe_id in case of flush error */
	if (ionic_v1_cqe_error(cqe) && st_len == IONIC_STS_WQE_FLUSHED_ERR) {
		/* should only see flushed for rq, never srq, check anyway */
		cqe_qp->rq_flush = !qp->is_srq;
		if (cqe_qp->rq_flush) {
			cq->flush = true;
			list_move_tail(&qp->cq_flush_rq, &cq->flush_rq);
		}

		/* posted recvs (if any) flushed by ionic_flush_recv */
		return 0;
	}

	/* there had better be something in the recv queue to complete */
	if (ionic_queue_empty(&qp->rq)) {
		dev_warn(&dev->ibdev.dev, "qp %u is empty\n", qp->qpid);
		return -EIO;
	}

	/* wqe_id must be a valid queue index */
	if (unlikely(cqe->recv.wqe_id >> qp->rq.depth_log2)) {
		dev_warn(&dev->ibdev.dev,
			 "qp %u recv index %llu invalid\n",
			 qp->qpid, (unsigned long long)cqe->recv.wqe_id);
		return -EIO;
	}

	/* wqe_id must indicate a request that is outstanding */
	meta = &qp->rq_meta[cqe->recv.wqe_id];
	if (unlikely(meta->next != IONIC_META_POSTED)) {
		dev_warn(&dev->ibdev.dev,
			 "qp %u recv index %llu not posted\n",
			 qp->qpid, (unsigned long long)cqe->recv.wqe_id);
		return -EIO;
	}

	meta->next = qp->rq_meta_head;
	qp->rq_meta_head = meta;

	memset(wc, 0, sizeof(*wc));

	wc->wr_id = meta->wrid;

	if (!cqe_qp->is_srq)
		wc->qp = &cqe_qp->ibqp;

	if (ionic_v1_cqe_error(cqe)) {
		wc->vendor_err = st_len;
		wc->status = ionic_to_ib_status(st_len);

		cqe_qp->rq_flush = !qp->is_srq;
		if (cqe_qp->rq_flush) {
			cq->flush = true;
			list_move_tail(&qp->cq_flush_rq, &cq->flush_rq);
		}

		dev_warn(&dev->ibdev.dev,
			 "qp %d recv cqe with error\n",
			 qp->qpid);
		print_hex_dump(KERN_WARNING, "cqe ", DUMP_PREFIX_OFFSET, 16, 1,
			       cqe, BIT(cq->q.stride_log2), true);

		goto out;
	}

	wc->vendor_err = 0;
	wc->status = IB_WC_SUCCESS;

	src_qpn = be32_to_cpu(cqe->recv.src_qpn_op);
	op = src_qpn >> IONIC_V1_CQE_RECV_OP_SHIFT;

	src_qpn &= IONIC_V1_CQE_RECV_QPN_MASK;
	op &= IONIC_V1_CQE_RECV_OP_MASK;

	wc->opcode = IB_WC_RECV;
	switch (op) {
	case IONIC_V1_CQE_RECV_OP_RDMA_IMM:
		wc->opcode = IB_WC_RECV_RDMA_WITH_IMM;
		/* fallthrough */
	case IONIC_V1_CQE_RECV_OP_SEND_IMM:
		wc->wc_flags |= IB_WC_WITH_IMM;
		wc->ex.imm_data = cqe->recv.imm_data_rkey; /* be32 in wc */
		break;
	case IONIC_V1_CQE_RECV_OP_SEND_INV:
		wc->wc_flags |= IB_WC_WITH_INVALIDATE;
		wc->ex.invalidate_rkey = be32_to_cpu(cqe->recv.imm_data_rkey);
	}

	wc->byte_len = st_len;
	wc->src_qp = src_qpn;

	if (qp->ibqp.qp_type == IB_QPT_UD ||
	    qp->ibqp.qp_type == IB_QPT_GSI) {
		wc->wc_flags |= IB_WC_GRH | IB_WC_WITH_SMAC;
		ether_addr_copy(wc->smac, cqe->recv.src_mac);

		wc->wc_flags |= IB_WC_WITH_NETWORK_HDR_TYPE;
		if (ionic_v1_cqe_recv_is_ipv4(cqe))
			wc->network_hdr_type = RDMA_NETWORK_IPV4;
		else
			wc->network_hdr_type = RDMA_NETWORK_IPV6;

		if (ionic_v1_cqe_recv_is_vlan(cqe))
			wc->wc_flags |= IB_WC_WITH_VLAN;

		/* vlan_tag in cqe will be valid from dpath even if no vlan */
		vlan_tag = be16_to_cpu(cqe->recv.vlan_tag);
		wc->vlan_id = vlan_tag & 0xfff; /* 802.1q VID */
		wc->sl = vlan_tag >> 13; /* 802.1q PCP */
	}

	wc->pkey_index = 0;
	wc->port_num = 1;

out:
	ionic_queue_consume(&qp->rq);

	return 1;
}

static bool ionic_peek_send(struct ionic_qp *qp)
{
	struct ionic_sq_meta *meta;

	if (qp->sq_flush)
		return false;

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
			   struct ib_wc *wc)
{
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

		ionic_queue_consume(&qp->sq);

		/* produce wc only if signaled or error status */
	} while (!meta->signal && meta->ibsts == IB_WC_SUCCESS);

	memset(wc, 0, sizeof(*wc));

	wc->status = meta->ibsts;
	wc->wr_id = meta->wrid;
	wc->qp = &qp->ibqp;

	if (meta->ibsts == IB_WC_SUCCESS) {
		wc->byte_len = meta->len;
		wc->opcode = meta->ibop;
	} else {
		wc->vendor_err = meta->len;

		qp->sq_flush = true;
		cq->flush = true;
		list_move_tail(&qp->cq_flush_sq, &cq->flush_sq);
	}

	return 1;

out_empty:
	if (qp->sq_flush_rcvd) {
		qp->sq_flush = true;
		cq->flush = true;
		list_move_tail(&qp->cq_flush_sq, &cq->flush_sq);
	}
	return 0;
}

static int ionic_poll_send_many(struct ionic_cq *cq, struct ionic_qp *qp,
				struct ib_wc *wc, int nwc)
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

static int ionic_validate_cons(u16 prod, u16 cons,
			       u16 comp, u16 mask)
{
	if (((prod - cons) & mask) < ((comp - cons) & mask))
		return -EIO;

	return 0;
}

static int ionic_comp_msn(struct ionic_qp *qp, struct ionic_v1_cqe *cqe)
{
	struct ionic_sq_meta *meta;
	u16 cqe_seq, cqe_idx;
	int rc;

	if (qp->sq_flush)
		return 0;

	cqe_seq = be32_to_cpu(cqe->send.msg_msn) & qp->sq.mask;

	if (ionic_v1_cqe_error(cqe))
		cqe_idx = qp->sq_msn_idx[cqe_seq];
	else
		cqe_idx = qp->sq_msn_idx[(cqe_seq - 1) & qp->sq.mask];

	rc = ionic_validate_cons(qp->sq_msn_prod,
				 qp->sq_msn_cons,
				 cqe_seq, qp->sq.mask);
	if (rc) {
		dev_warn(&qp->ibqp.device->dev,
			 "qp %u invalid msn %#x seq %u for prod %u cons %u\n",
			 qp->qpid, be32_to_cpu(cqe->send.msg_msn),
			 cqe_seq, qp->sq_msn_prod, qp->sq_msn_cons);
		return rc;
	}

	qp->sq_msn_cons = cqe_seq;

	if (ionic_v1_cqe_error(cqe)) {
		meta = &qp->sq_meta[cqe_idx];
		meta->len = be32_to_cpu(cqe->status_length);
		meta->ibsts = ionic_to_ib_status(meta->len);
		meta->remote = false;

		dev_warn(&qp->ibqp.device->dev,
			 "qp %d msn cqe with error\n",
			 qp->qpid);
		print_hex_dump(KERN_WARNING, "cqe ", DUMP_PREFIX_OFFSET, 16, 1,
			       cqe, sizeof(*cqe), true);
	}

	return 0;
}

static int ionic_comp_npg(struct ionic_qp *qp, struct ionic_v1_cqe *cqe)
{
	struct ionic_sq_meta *meta;
	u16 cqe_idx;
	u32 st_len;

	if (qp->sq_flush)
		return 0;

	st_len = be32_to_cpu(cqe->status_length);

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
		meta->ibsts = ionic_to_ib_status(st_len);
		meta->remote = false;

		dev_warn(&qp->ibqp.device->dev,
			 "qp %d npg cqe with error\n",
			 qp->qpid);
		print_hex_dump(KERN_WARNING, "cqe ", DUMP_PREFIX_OFFSET, 16, 1,
			       cqe, sizeof(*cqe), true);
	}

	return 0;
}

static void ionic_reserve_sync_cq(struct ionic_ibdev *dev, struct ionic_cq *cq)
{
	if (!ionic_queue_empty(&cq->q)) {
		cq->reserve += ionic_queue_length(&cq->q);
		cq->q.cons = cq->q.prod;

		ionic_dbell_ring(dev->dbpage, dev->cq_qtype,
				 ionic_queue_dbell_val(&cq->q));
	}
}

static void ionic_reserve_cq(struct ionic_ibdev *dev, struct ionic_cq *cq,
			     int spend)
{
	cq->reserve -= spend;

	if (cq->reserve <= 0)
		ionic_reserve_sync_cq(dev, cq);
}

static int ionic_poll_cq(struct ib_cq *ibcq, int nwc, struct ib_wc *wc)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibcq->device);
	struct ionic_cq *cq = to_ionic_cq(ibcq);
	struct ionic_qp *qp, *qp_next;
	struct ionic_v1_cqe *cqe;
	u32 qtf, qid;
	u8 type;
	bool peek;
	int rc = 0, npolled = 0;
	unsigned long irqflags;

	if (nwc < 1)
		return 0;

	spin_lock_irqsave(&cq->lock, irqflags);

	/* poll already indicated work completions for send queue */

	list_for_each_entry_safe(qp, qp_next, &cq->poll_sq, cq_poll_sq) {
		if (npolled == nwc)
			goto out;

		spin_lock(&qp->sq_lock);
		rc = ionic_poll_send_many(cq, qp, wc + npolled, nwc - npolled);
		spin_unlock(&qp->sq_lock);

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

		qp = xa_load(&dev->qp_tbl, qid);
		if (unlikely(!qp)) {
			dev_dbg(&dev->ibdev.dev,
				"missing qp for qid %u\n", qid);
			goto cq_next;
		}

		switch (type) {
		case IONIC_V1_CQE_TYPE_RECV:
			spin_lock(&qp->rq_lock);
			rc = ionic_poll_recv(dev, cq, qp, cqe, wc + npolled);
			spin_unlock(&qp->rq_lock);

			if (rc < 0)
				goto out;

			npolled += rc;

			break;

		case IONIC_V1_CQE_TYPE_SEND_MSN:
			spin_lock(&qp->sq_lock);
			rc = ionic_comp_msn(qp, cqe);
			if (!rc) {
				rc = ionic_poll_send_many(cq, qp,
							  wc + npolled,
							  nwc - npolled);
				peek = ionic_peek_send(qp);
			}
			spin_unlock(&qp->sq_lock);

			if (rc < 0)
				goto out;

			npolled += rc;

			if (peek)
				list_move_tail(&qp->cq_poll_sq, &cq->poll_sq);
			break;

		case IONIC_V1_CQE_TYPE_SEND_NPG:
			spin_lock(&qp->sq_lock);
			rc = ionic_comp_npg(qp, cqe);
			if (!rc) {
				rc = ionic_poll_send_many(cq, qp,
							  wc + npolled,
							  nwc - npolled);
				peek = ionic_peek_send(qp);
			}
			spin_unlock(&qp->sq_lock);

			if (rc < 0)
				goto out;

			npolled += rc;

			if (peek)
				list_move_tail(&qp->cq_poll_sq, &cq->poll_sq);
			break;

		default:
			dev_warn(&dev->ibdev.dev,
				 "unexpected cqe type %u\n", type);

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

	list_for_each_entry_safe(qp, qp_next, &cq->flush_sq, cq_flush_sq) {
		if (npolled == nwc)
			goto out;

		spin_lock(&qp->sq_lock);
		rc = ionic_flush_send_many(qp, wc + npolled, nwc - npolled);
		spin_unlock(&qp->sq_lock);

		if (rc > 0)
			npolled += rc;

		if (npolled < nwc)
			list_del_init(&qp->cq_flush_sq);
		else
			cq->flush = true;
	}

	list_for_each_entry_safe(qp, qp_next, &cq->flush_rq, cq_flush_rq) {
		if (npolled == nwc)
			goto out;

		spin_lock(&qp->rq_lock);
		rc = ionic_flush_recv_many(qp, wc + npolled, nwc - npolled);
		spin_unlock(&qp->rq_lock);

		if (rc > 0)
			npolled += rc;

		if (npolled < nwc)
			list_del_init(&qp->cq_flush_rq);
		else
			cq->flush = true;
	}

out:
	/* in case reserve was depleted (more work posted than cq depth) */
	if (cq->reserve <= 0)
		ionic_reserve_sync_cq(dev, cq);

	spin_unlock_irqrestore(&cq->lock, irqflags);

	return npolled ?: rc;
}

static int ionic_req_notify_cq(struct ib_cq *ibcq,
			       enum ib_cq_notify_flags flags)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibcq->device);
	struct ionic_cq *cq = to_ionic_cq(ibcq);
	u64 dbell_val = cq->q.dbell;

	if (flags & IB_CQ_SOLICITED) {
		cq->arm_sol_prod = ionic_queue_next(&cq->q, cq->arm_sol_prod);
		dbell_val |= cq->arm_sol_prod | IONIC_CQ_RING_SOL;
	} else {
		cq->arm_any_prod = ionic_queue_next(&cq->q, cq->arm_any_prod);
		dbell_val |= cq->arm_any_prod | IONIC_CQ_RING_ARM;
	}

	ionic_reserve_sync_cq(dev, cq);

	ionic_dbell_ring(dev->dbpage, dev->cq_qtype, dbell_val);

	/* IB_CQ_REPORT_MISSED_EVENTS:
	 *
	 * The queue index in ring zero guarantees no missed events.
	 *
	 * Here, we check if the color bit in the next cqe is flipped.  If it
	 * is flipped, then progress can be made by immediately polling the cq.
	 * Still, the cq will be armed, and an event will be generated.  The cq
	 * may be empty when polled after the event, because the next poll
	 * after arming the cq can empty it.
	 */
	return (flags & IB_CQ_REPORT_MISSED_EVENTS) &&
		cq->color == ionic_v1_cqe_color(ionic_queue_at_prod(&cq->q));
}

static bool pd_local_privileged(struct ib_pd *pd)
{
	return !pd->uobject;
}

static bool pd_remote_privileged(struct ib_pd *pd)
{
	return pd->flags & IB_PD_UNSAFE_GLOBAL_RKEY;
}

static int ionic_create_qp_cmd(struct ionic_ibdev *dev,
			       struct ionic_pd *pd,
			       struct ionic_cq *send_cq,
			       struct ionic_cq *recv_cq,
			       struct ionic_qp *qp,
			       struct ionic_tbl_buf *sq_buf,
			       struct ionic_tbl_buf *rq_buf,
			       struct ib_qp_init_attr *attr)
{
	const u16 dbid = ionic_obj_dbid(dev, pd->ibpd.uobject);
	const u32 flags = to_ionic_qp_flags(0, 0, qp->sq_is_cmb, qp->rq_is_cmb,
					    qp->sq_spec, qp->rq_spec,
					    pd_local_privileged(&pd->ibpd),
					    pd_remote_privileged(&pd->ibpd));
	struct ionic_admin_wr wr = {
		.work = COMPLETION_INITIALIZER_ONSTACK(wr.work),
		.wqe = {
			.op = IONIC_V1_ADMIN_CREATE_QP,
			.type_state = to_ionic_qp_type(attr->qp_type),
			.dbid_flags = cpu_to_le16(dbid),
			.id_ver = cpu_to_le32(qp->qpid),
			.qp = {
				.pd_id = cpu_to_le32(pd->pdid),
				.priv_flags = cpu_to_be32(flags),
			}
		}
	};
	int rc;

	if (dev->admin_opcodes <= IONIC_V1_ADMIN_CREATE_QP)
		return -ENOSYS;

	if (qp->has_sq) {
		wr.wqe.qp.sq_cq_id = cpu_to_le32(send_cq->cqid);
		wr.wqe.qp.sq_depth_log2 = qp->sq.depth_log2;
		wr.wqe.qp.sq_stride_log2 = qp->sq.stride_log2;
		wr.wqe.qp.sq_page_size_log2 = sq_buf->page_size_log2;
		wr.wqe.qp.sq_tbl_index_xrcd_id =
			cpu_to_le32(qp->sq_res.tbl_pos);
		wr.wqe.qp.sq_map_count = cpu_to_le32(sq_buf->tbl_pages);
		wr.wqe.qp.sq_dma_addr = ionic_pgtbl_dma(sq_buf, 0);
#ifdef IONIC_SRQ_XRC
	} else if (attr->xrcd) {
		wr.wqe.qp.sq_tbl_index_xrcd_id = 0;
#endif /* IONIC_SRQ_XRC */
	}

	if (qp->has_rq) {
		wr.wqe.qp.rq_cq_id = cpu_to_le32(recv_cq->cqid);
		wr.wqe.qp.rq_depth_log2 = qp->rq.depth_log2;
		wr.wqe.qp.rq_stride_log2 = qp->rq.stride_log2;
		wr.wqe.qp.rq_page_size_log2 = rq_buf->page_size_log2;
		wr.wqe.qp.rq_tbl_index_srq_id =
			cpu_to_le32(qp->rq_res.tbl_pos);
		wr.wqe.qp.rq_map_count = cpu_to_le32(rq_buf->tbl_pages);
		wr.wqe.qp.rq_dma_addr = ionic_pgtbl_dma(rq_buf, 0);
#ifdef IONIC_SRQ_XRC
	} else if (attr->srq) {
		wr.wqe.qp.rq_tbl_index_srq_id =
			cpu_to_le32(to_ionic_srq(attr->srq)->qpid);
#endif /* IONIC_SRQ_XRC */
	}

	ionic_admin_post(dev, &wr);
	ionic_admin_wait(&wr);

	if (wr.status == IONIC_ADMIN_KILLED) {
		dev_dbg(&dev->ibdev.dev, "killed\n");
		rc = -ENODEV;
	} else if (ionic_v1_cqe_error(&wr.cqe)) {
		dev_warn(&dev->ibdev.dev, "cqe error %u\n",
			 be32_to_cpu(wr.cqe.status_length));
		rc = -EINVAL;
	} else {
		rc = 0;
	}

	return rc;
}

static int ionic_modify_qp_cmd(struct ionic_ibdev *dev,
			       struct ionic_qp *qp,
			       struct ib_qp_attr *attr,
			       int mask)
{
	const u32 flags = to_ionic_qp_flags(attr->qp_access_flags,
					    attr->en_sqd_async_notify,
					    qp->sq_is_cmb, qp->rq_is_cmb,
					    qp->sq_spec, qp->rq_spec,
					    pd_local_privileged(qp->ibqp.pd),
					    pd_remote_privileged(qp->ibqp.pd));
	const u8 state = to_ionic_qp_modify_state(attr->qp_state,
						  attr->cur_qp_state);
	struct ionic_admin_wr wr = {
		.work = COMPLETION_INITIALIZER_ONSTACK(wr.work),
		.wqe = {
			.op = IONIC_V1_ADMIN_MODIFY_QP,
			.type_state = state,
			.id_ver = cpu_to_le32(qp->qpid),
			.mod_qp = {
				.attr_mask = cpu_to_be32(mask),
				.access_flags = cpu_to_be16(flags),
				.rq_psn = cpu_to_le32(attr->rq_psn),
				.sq_psn = cpu_to_le32(attr->sq_psn),
				.pmtu = (attr->path_mtu + 7),
				.retry = (attr->retry_cnt |
					  (attr->rnr_retry << 4)),
				.rnr_timer = attr->min_rnr_timer,
				.retry_timeout = attr->timeout,
			}
		}
	};
	struct ib_ud_header *hdr = NULL;
	void *hdr_buf = NULL;
	dma_addr_t hdr_dma = 0;
	int rc, hdr_len = 0;

	if (dev->admin_opcodes <= IONIC_V1_ADMIN_MODIFY_QP)
		return -ENOSYS;

	if ((mask & IB_QP_MAX_DEST_RD_ATOMIC) && attr->max_dest_rd_atomic) {
		/* Note, round up/down was already done for allocating
		 * resources on the device. The allocation order is in cache
		 * line size.  We can't use the order of the resource
		 * allocation to determine the order wqes here, because for
		 * queue length <= one cache line it is not distinct.
		 *
		 * Therefore, order wqes is computed again here.
		 *
		 * Account for hole and round up to the next order.
		 */
		wr.wqe.mod_qp.rsq_depth =
			order_base_2(attr->max_dest_rd_atomic + 1);
		wr.wqe.mod_qp.rsq_index = cpu_to_le32(qp->rsq_res.tbl_pos);
	}

	if ((mask & IB_QP_MAX_QP_RD_ATOMIC) && attr->max_rd_atomic) {
		/* Account for hole and round down to the next order */
		wr.wqe.mod_qp.rrq_depth =
			order_base_2(attr->max_rd_atomic + 2) - 1;
		wr.wqe.mod_qp.rrq_index = cpu_to_le32(qp->rrq_res.tbl_pos);
	}

	if (qp->ibqp.qp_type == IB_QPT_RC || qp->ibqp.qp_type == IB_QPT_UC)
		wr.wqe.mod_qp.qkey_dest_qpn = cpu_to_le32(attr->dest_qp_num);
	else
		wr.wqe.mod_qp.qkey_dest_qpn = cpu_to_le32(attr->qkey);

	if (mask & IB_QP_AV) {
		hdr = kmalloc(sizeof(*hdr), GFP_KERNEL);
		if (!hdr) {
			rc = -ENOMEM;
			goto err_hdr;
		}

		rc = ionic_build_hdr(dev, hdr, &attr->ah_attr, true);
		if (rc)
			goto err_buf;

		qp->sgid_index = rdma_ah_read_grh(&attr->ah_attr)->sgid_index;

		hdr_buf = contig_kmalloc(PAGE_SIZE, GFP_KERNEL);
		if (!hdr_buf) {
			rc = -ENOMEM;
			goto err_buf;
		}

		hdr_len = ib_ud_header_pack(hdr, hdr_buf);
		hdr_len -= IB_BTH_BYTES;
		hdr_len -= IB_DETH_BYTES;

		dev_dbg(&dev->ibdev.dev, "roce packet header template\n");
		print_hex_dump_debug("hdr ", DUMP_PREFIX_OFFSET, 16, 1,
				     hdr_buf, hdr_len, true);

		hdr_dma = dma_map_single(dev->hwdev, hdr_buf, hdr_len,
					 DMA_TO_DEVICE);

		rc = dma_mapping_error(dev->hwdev, hdr_dma);
		if (rc)
			goto err_dma;

		wr.wqe.mod_qp.ah_id_len =
			cpu_to_le32(qp->ahid | (hdr_len << 24));
		wr.wqe.mod_qp.dma_addr = cpu_to_le64(hdr_dma);

		wr.wqe.mod_qp.dcqcn_profile = qp->dcqcn_profile;
		wr.wqe.mod_qp.en_pcp = attr->ah_attr.sl;
		wr.wqe.mod_qp.ip_dscp =
			rdma_ah_read_grh(&attr->ah_attr)->traffic_class >> 2;
	}

	ionic_admin_post(dev, &wr);
	ionic_admin_wait(&wr);

	if (wr.status == IONIC_ADMIN_KILLED) {
		dev_dbg(&dev->ibdev.dev, "killed\n");
		rc = -ENODEV;
	} else if (ionic_v1_cqe_error(&wr.cqe)) {
		dev_warn(&dev->ibdev.dev, "cqe error %u\n",
			 be32_to_cpu(wr.cqe.status_length));
		rc = -EINVAL;
	} else {
		rc = 0;
	}

	if (mask & IB_QP_AV)
		dma_unmap_single(dev->hwdev, hdr_dma, hdr_len,
				 DMA_TO_DEVICE);
err_dma:
	if (mask & IB_QP_AV)
		contig_kfree(hdr_buf, PAGE_SIZE);
err_buf:
	if (mask & IB_QP_AV)
		kfree(hdr);
err_hdr:
	return rc;
}

static int ionic_destroy_qp_cmd(struct ionic_ibdev *dev, u32 qpid)
{
	struct ionic_admin_wr wr = {
		.work = COMPLETION_INITIALIZER_ONSTACK(wr.work),
		.wqe = {
			.op = IONIC_V1_ADMIN_DESTROY_QP,
			.id_ver = cpu_to_le32(qpid),
		}
	};
	int rc;

	if (dev->admin_opcodes <= IONIC_V1_ADMIN_DESTROY_QP)
		return -ENOSYS;

	ionic_admin_post(dev, &wr);
	ionic_admin_wait(&wr);

	if (wr.status == IONIC_ADMIN_KILLED) {
		dev_dbg(&dev->ibdev.dev, "killed\n");
		rc = 0;
	} else if (ionic_v1_cqe_error(&wr.cqe)) {
		dev_warn(&dev->ibdev.dev, "cqe error %u\n",
			 be32_to_cpu(wr.cqe.status_length));
		rc = -EINVAL;
	} else {
		rc = 0;
	}

	return rc;
}

static void ionic_qp_sq_init_cmb(struct ionic_ibdev *dev,
				 struct ionic_ctx *ctx,
				 struct ionic_qp *qp,
				 bool cap_inline)
{
	int rc;

	if (!qp->has_sq)
		goto not_in_cmb;

	if (ionic_sqcmb_inline && !cap_inline)
		goto not_in_cmb;

	qp->sq_cmb_order = order_base_2(qp->sq.size / PAGE_SIZE);

	if (qp->sq_cmb_order >= ionic_sqcmb_order)
		goto not_in_cmb;

	rc = ionic_api_get_cmb(dev->handle, &qp->sq_cmb_pgid,
			       &qp->sq_cmb_addr, qp->sq_cmb_order);
	if (rc)
		goto not_in_cmb;

	qp->sq_cmb_ptr = ioremap_wc(qp->sq_cmb_addr, qp->sq.size);
	if (!qp->sq_cmb_ptr)
		goto err_map;

	memset_io(qp->sq_cmb_ptr, 0, qp->sq.size);

	if (ctx) {
		iounmap(qp->sq_cmb_ptr);
		qp->sq_cmb_ptr = NULL;
	}

	qp->sq_is_cmb = true;

	return;

err_map:
	ionic_api_put_cmb(dev->handle, qp->sq_cmb_pgid, qp->sq_cmb_order);
not_in_cmb:
	qp->sq_is_cmb = false;
	qp->sq_cmb_ptr = NULL;
	qp->sq_cmb_order = IONIC_RES_INVALID;
	qp->sq_cmb_pgid = 0;
	qp->sq_cmb_addr = 0;

	qp->sq_cmb_mmap.offset = 0;
	qp->sq_cmb_mmap.size = 0;
	qp->sq_cmb_mmap.pfn = 0;
}

static void ionic_qp_sq_destroy_cmb(struct ionic_ibdev *dev,
				    struct ionic_ctx *ctx,
				    struct ionic_qp *qp)
{
	if (!qp->sq_is_cmb)
		return;

	if (ctx) {
		mutex_lock(&ctx->mmap_mut);
		list_del(&qp->sq_cmb_mmap.ctx_ent);
		mutex_unlock(&ctx->mmap_mut);
	} else {
		iounmap(qp->sq_cmb_ptr);
	}

	ionic_api_put_cmb(dev->handle, qp->sq_cmb_pgid, qp->sq_cmb_order);
}

static int ionic_qp_sq_init(struct ionic_ibdev *dev, struct ionic_ctx *ctx,
			    struct ionic_qp *qp, struct ionic_qdesc *sq,
			    struct ionic_tbl_buf *buf, int max_wr, int max_sge,
			    int max_data, int sq_spec, struct ib_udata *udata)
{
	int rc = 0;
	u32 wqe_size;

	qp->sq_msn_prod = 0;
	qp->sq_msn_cons = 0;
	qp->sq_cmb_prod = 0;

	INIT_LIST_HEAD(&qp->sq_cmb_mmap.ctx_ent);

	if (!qp->has_sq) {
		if (buf) {
			buf->tbl_buf = NULL;
			buf->tbl_limit = 0;
			buf->tbl_pages = 0;
		}
		if (ctx)
			rc = ionic_validate_qdesc_zero(sq);

		return rc;
	}

	rc = -EINVAL;

	if (max_wr < 0 || max_wr > 0xffff)
		goto err_sq;

	if (max_sge < 1 ||
	    max_sge > ionic_v1_send_wqe_max_sge(dev->max_stride, 0))
		goto err_sq;

	if (max_data < 0 ||
	    max_data > ionic_v1_send_wqe_max_data(dev->max_stride))
		goto err_sq;

	if (ctx) {
		rc = ionic_validate_qdesc(sq);
		if (rc)
			goto err_sq;

		qp->sq_spec = sq_spec;

		qp->sq.ptr = NULL;
		qp->sq.size = sq->size;
		qp->sq.mask = sq->mask;
		qp->sq.depth_log2 = sq->depth_log2;
		qp->sq.stride_log2 = sq->stride_log2;

		qp->sq_meta = NULL;
		qp->sq_msn_idx = NULL;

		qp->sq_umem = ib_umem_get(&ctx->ibctx, sq->addr,
					  sq->size, 0, 0);
		if (IS_ERR(qp->sq_umem)) {
			rc = PTR_ERR(qp->sq_umem);
			goto err_sq;
		}
	} else {
		qp->sq_umem = NULL;

		qp->sq_spec = ionic_v1_use_spec_sge(max_sge, sq_spec);
		if (sq_spec && !qp->sq_spec)
			dev_dbg(&dev->ibdev.dev,
				"init sq: max_sge %u disables spec\n",
				max_sge);

		wqe_size = ionic_v1_send_wqe_min_size(max_sge, max_data,
						      qp->sq_spec);
		rc = ionic_queue_init(&qp->sq, dev->hwdev,
				      max_wr, wqe_size);
		if (rc)
			goto err_sq;

		ionic_queue_dbell_init(&qp->sq, qp->qpid);

		qp->sq_meta = kmalloc_array((u32)qp->sq.mask + 1,
					    sizeof(*qp->sq_meta),
					    GFP_KERNEL);
		if (!qp->sq_meta) {
			rc = -ENOMEM;
			goto err_sq_meta;
		}

		qp->sq_msn_idx = kmalloc_array((u32)qp->sq.mask + 1,
					       sizeof(*qp->sq_msn_idx),
					       GFP_KERNEL);
		if (!qp->sq_msn_idx) {
			rc = -ENOMEM;
			goto err_sq_msn;
		}
	}

	ionic_qp_sq_init_cmb(dev, ctx, qp, max_data > 0);
	if (qp->sq_is_cmb)
		rc = ionic_pgtbl_init(dev, &qp->sq_res, buf, NULL,
				      (u64)qp->sq_cmb_pgid << PAGE_SHIFT, 1);
	else
		rc = ionic_pgtbl_init(dev, &qp->sq_res, buf,
				      qp->sq_umem, qp->sq.dma, 1);
	if (rc)
		goto err_sq_tbl;

	return 0;

err_sq_tbl:
	ionic_qp_sq_destroy_cmb(dev, ctx, qp);

	kfree(qp->sq_msn_idx);
err_sq_msn:
	kfree(qp->sq_meta);
err_sq_meta:
	if (qp->sq_umem)
		ib_umem_release(qp->sq_umem);
	else
		ionic_queue_destroy(&qp->sq, dev->hwdev);
err_sq:
	return rc;
}

static void ionic_qp_sq_destroy(struct ionic_ibdev *dev,
				struct ionic_ctx *ctx,
				struct ionic_qp *qp)
{
	if (!qp->has_sq)
		return;

	ionic_put_res(dev, &qp->sq_res);

	ionic_qp_sq_destroy_cmb(dev, ctx, qp);

	kfree(qp->sq_msn_idx);
	kfree(qp->sq_meta);

	if (qp->sq_umem)
		ib_umem_release(qp->sq_umem);
	else
		ionic_queue_destroy(&qp->sq, dev->hwdev);
}

static void ionic_qp_rq_init_cmb(struct ionic_ibdev *dev,
				 struct ionic_ctx *ctx,
				 struct ionic_qp *qp)
{
	int rc;

	if (!qp->has_rq)
		goto not_in_cmb;

	qp->rq_cmb_order = order_base_2(qp->rq.size / PAGE_SIZE);

	if (qp->rq_cmb_order >= ionic_rqcmb_order)
		goto not_in_cmb;

	rc = ionic_api_get_cmb(dev->handle, &qp->rq_cmb_pgid,
			       &qp->rq_cmb_addr, qp->rq_cmb_order);
	if (rc)
		goto not_in_cmb;

	qp->rq_cmb_ptr = ioremap_wc(qp->rq_cmb_addr, qp->rq.size);
	if (!qp->rq_cmb_ptr)
		goto err_map;

	memset_io(qp->rq_cmb_ptr, 0, qp->rq.size);

	if (ctx) {
		iounmap(qp->rq_cmb_ptr);
		qp->rq_cmb_ptr = NULL;
	}

	qp->rq_is_cmb = true;

	return;

err_map:
	ionic_api_put_cmb(dev->handle, qp->rq_cmb_pgid, qp->rq_cmb_order);
not_in_cmb:
	qp->rq_is_cmb = false;
	qp->rq_cmb_ptr = NULL;
	qp->rq_cmb_order = IONIC_RES_INVALID;
	qp->rq_cmb_pgid = 0;
	qp->rq_cmb_addr = 0;

	qp->rq_cmb_mmap.offset = 0;
	qp->rq_cmb_mmap.size = 0;
	qp->rq_cmb_mmap.pfn = 0;
}

static void ionic_qp_rq_destroy_cmb(struct ionic_ibdev *dev,
				    struct ionic_ctx *ctx,
				    struct ionic_qp *qp)
{
	if (!qp->rq_is_cmb)
		return;

	if (ctx) {
		mutex_lock(&ctx->mmap_mut);
		list_del(&qp->rq_cmb_mmap.ctx_ent);
		mutex_unlock(&ctx->mmap_mut);
	} else {
		iounmap(qp->rq_cmb_ptr);
	}

	ionic_api_put_cmb(dev->handle, qp->rq_cmb_pgid, qp->rq_cmb_order);
}

static int ionic_qp_rq_init(struct ionic_ibdev *dev, struct ionic_ctx *ctx,
			    struct ionic_qp *qp, struct ionic_qdesc *rq,
			    struct ionic_tbl_buf *buf, int max_wr, int max_sge,
			    int rq_spec, struct ib_udata *udata)
{
	u32 wqe_size;
	int rc = 0, i;

	qp->rq_cmb_prod = 0;

	INIT_LIST_HEAD(&qp->rq_cmb_mmap.ctx_ent);

	if (!qp->has_rq) {
		if (buf) {
			buf->tbl_buf = NULL;
			buf->tbl_limit = 0;
			buf->tbl_pages = 0;
		}
		if (ctx)
			rc = ionic_validate_qdesc_zero(rq);

		return rc;
	}

	rc = -EINVAL;

	if (max_wr < 0 || max_wr > 0xffff)
		goto err_rq;

	if (max_sge < 1 ||
	    max_sge > ionic_v1_recv_wqe_max_sge(dev->max_stride, 0))
		goto err_rq;

	if (ctx) {
		rc = ionic_validate_qdesc(rq);
		if (rc)
			goto err_rq;

		qp->rq_spec = rq_spec;

		qp->rq.ptr = NULL;
		qp->rq.size = rq->size;
		qp->rq.mask = rq->mask;
		qp->rq.depth_log2 = rq->depth_log2;
		qp->rq.stride_log2 = rq->stride_log2;

		qp->rq_meta = NULL;

		qp->rq_umem = ib_umem_get(&ctx->ibctx, rq->addr,
					  rq->size, 0, 0);
		if (IS_ERR(qp->rq_umem)) {
			rc = PTR_ERR(qp->rq_umem);
			goto err_rq;
		}
	} else {
		qp->rq_umem = NULL;
		qp->rq_res.tbl_order = IONIC_RES_INVALID;
		qp->rq_res.tbl_pos = 0;

		qp->rq_spec = ionic_v1_use_spec_sge(max_sge, rq_spec);
		if (rq_spec && !qp->rq_spec)
			dev_dbg(&dev->ibdev.dev,
				"init rq: max_sge %u disables spec\n",
				max_sge);

		wqe_size = ionic_v1_recv_wqe_min_size(max_sge, qp->rq_spec);
		rc = ionic_queue_init(&qp->rq, dev->hwdev,
				      max_wr, wqe_size);
		if (rc)
			goto err_rq;

		ionic_queue_dbell_init(&qp->rq, qp->qpid);

		qp->rq_meta = kmalloc_array((u32)qp->rq.mask + 1,
					    sizeof(*qp->rq_meta),
					    GFP_KERNEL);
		if (!qp->rq_meta) {
			rc = -ENOMEM;
			goto err_rq_meta;
		}

		for (i = 0; i < qp->rq.mask; ++i)
			qp->rq_meta[i].next = &qp->rq_meta[i + 1];
		qp->rq_meta[i].next = IONIC_META_LAST;
		qp->rq_meta_head = &qp->rq_meta[0];
	}

	ionic_qp_rq_init_cmb(dev, ctx, qp);
	if (qp->rq_is_cmb)
		rc = ionic_pgtbl_init(dev, &qp->rq_res, buf, NULL,
				      (u64)qp->rq_cmb_pgid << PAGE_SHIFT, 1);
	else
		rc = ionic_pgtbl_init(dev, &qp->rq_res, buf,
				      qp->rq_umem, qp->rq.dma, 1);
	if (rc)
		goto err_rq_tbl;

	return 0;

err_rq_tbl:
	ionic_qp_rq_destroy_cmb(dev, ctx, qp);

	kfree(qp->rq_meta);
err_rq_meta:
	if (qp->rq_umem)
		ib_umem_release(qp->rq_umem);
	else
		ionic_queue_destroy(&qp->rq, dev->hwdev);
err_rq:

	return rc;
}

static void ionic_qp_rq_destroy(struct ionic_ibdev *dev,
				struct ionic_ctx *ctx,
				struct ionic_qp *qp)
{
	if (!qp->has_rq)
		return;

	ionic_put_res(dev, &qp->rq_res);

	ionic_qp_rq_destroy_cmb(dev, ctx, qp);

	kfree(qp->rq_meta);

	if (qp->rq_umem)
		ib_umem_release(qp->rq_umem);
	else
		ionic_queue_destroy(&qp->rq, dev->hwdev);
}

static void ionic_qp_add_ref(struct ionic_qp *qp)
{
        refcount_inc(&qp->qp_refcnt);
}

static void ionic_qp_rem_ref(struct ionic_qp *qp)
{
        if (refcount_dec_and_test(&qp->qp_refcnt))
                complete(&qp->qp_rel_comp);
}

static struct ib_qp *ionic_create_qp(struct ib_pd *ibpd,
				     struct ib_qp_init_attr *attr,
				     struct ib_udata *udata)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibpd->device);
	struct ionic_ctx *ctx = to_ionic_ctx_uobj(ibpd->uobject);
	struct ionic_pd *pd = to_ionic_pd(ibpd);
	struct ionic_qp *qp;
	struct ionic_cq *cq;
	struct ionic_qp_req req;
	struct ionic_qp_resp resp = {0};
	struct ionic_tbl_buf sq_buf = {0}, rq_buf = {0};
	unsigned long irqflags;
	int rc;

	if (!ctx) {
		req.sq_spec = ionic_spec;
		req.rq_spec = ionic_spec;
		rc = ionic_validate_udata(udata, 0, 0);
	} else {
		rc = ionic_validate_udata(udata, sizeof(req), sizeof(resp));
		if (!rc)
			rc = ib_copy_from_udata(&req, udata, sizeof(req));
	}

	if (rc)
		goto err_qp;

	if (attr->qp_type == IB_QPT_SMI ||
	    attr->qp_type > IB_QPT_UD) {
		rc = -EINVAL;
		goto err_qp;
	}

	qp = kzalloc(sizeof(*qp), GFP_KERNEL);
	if (!qp) {
		rc = -ENOMEM;
		goto err_qp;
	}

	qp->state = IB_QPS_RESET;

	INIT_LIST_HEAD(&qp->cq_poll_sq);
	INIT_LIST_HEAD(&qp->cq_flush_sq);
	INIT_LIST_HEAD(&qp->cq_flush_rq);

	spin_lock_init(&qp->sq_lock);
	spin_lock_init(&qp->rq_lock);

	if (attr->qp_type == IB_QPT_GSI)
		rc = ionic_get_gsi_qpid(dev, &qp->qpid);
	else
		rc = ionic_get_qpid(dev, &qp->qpid);
	if (rc)
		goto err_qpid;

	qp->has_ah = attr->qp_type == IB_QPT_RC;

	qp->has_sq = attr->qp_type != IB_QPT_XRC_TGT;

	qp->has_rq = !attr->srq &&
		attr->qp_type != IB_QPT_XRC_INI &&
		attr->qp_type != IB_QPT_XRC_TGT;

	qp->is_srq = false;

	if (qp->has_ah) {
		rc = ionic_get_ahid(dev, &qp->ahid);
		if (rc)
			goto err_ahid;
	}

	qp->rrq_res.tbl_order = IONIC_RES_INVALID;
	qp->rrq_res.tbl_pos = 0;
	qp->rsq_res.tbl_order = IONIC_RES_INVALID;
	qp->rsq_res.tbl_pos = 0;

	rc = ionic_qp_sq_init(dev, ctx, qp, &req.sq, &sq_buf,
			      attr->cap.max_send_wr, attr->cap.max_send_sge,
			      attr->cap.max_inline_data, req.sq_spec, udata);
	if (rc)
		goto err_sq;

	rc = ionic_qp_rq_init(dev, ctx, qp, &req.rq, &rq_buf,
			      attr->cap.max_recv_wr, attr->cap.max_recv_sge,
			      req.rq_spec, udata);
	if (rc)
		goto err_rq;

	rc = ionic_create_qp_cmd(dev, pd,
				 to_ionic_cq(attr->send_cq),
				 to_ionic_cq(attr->recv_cq),
				 qp, &sq_buf, &rq_buf, attr);
	if (rc)
		goto err_cmd;

	if (ctx) {
		resp.qpid = qp->qpid;

		if (qp->sq_is_cmb) {
			qp->sq_cmb_mmap.size = qp->sq.size;
			qp->sq_cmb_mmap.pfn = PHYS_PFN(qp->sq_cmb_addr);
			qp->sq_cmb_mmap.writecombine = true;

			mutex_lock(&ctx->mmap_mut);
			qp->sq_cmb_mmap.offset = ctx->mmap_off;
			ctx->mmap_off += qp->sq.size;
			list_add(&qp->sq_cmb_mmap.ctx_ent, &ctx->mmap_list);
			mutex_unlock(&ctx->mmap_mut);

			resp.sq_cmb_offset = qp->sq_cmb_mmap.offset;
		}

		if (qp->rq_is_cmb) {
			qp->rq_cmb_mmap.size = qp->rq.size;
			qp->rq_cmb_mmap.pfn = PHYS_PFN(qp->rq_cmb_addr);
			qp->rq_cmb_mmap.writecombine = true;

			mutex_lock(&ctx->mmap_mut);
			qp->rq_cmb_mmap.offset = ctx->mmap_off;
			ctx->mmap_off += qp->rq.size;
			list_add(&qp->rq_cmb_mmap.ctx_ent, &ctx->mmap_list);
			mutex_unlock(&ctx->mmap_mut);

			resp.rq_cmb_offset = qp->rq_cmb_mmap.offset;
		}

		rc = ib_copy_to_udata(udata, &resp, sizeof(resp));
		if (rc)
			goto err_resp;
	}

	ionic_pgtbl_unbuf(dev, &rq_buf);
	ionic_pgtbl_unbuf(dev, &sq_buf);

	qp->ibqp.qp_num = qp->qpid;

	init_completion(&qp->qp_rel_comp);
	refcount_set(&qp->qp_refcnt, 1);

	rc = xa_err(xa_store_irq(&dev->qp_tbl, qp->qpid, qp, GFP_KERNEL));
	if (rc)
		goto err_resp;

	if (qp->has_sq) {
		cq = to_ionic_cq(attr->send_cq);
		spin_lock_irqsave(&cq->lock, irqflags);
		spin_unlock_irqrestore(&cq->lock, irqflags);

		attr->cap.max_send_wr = qp->sq.mask;
		attr->cap.max_send_sge =
			ionic_v1_send_wqe_max_sge(qp->sq.stride_log2,
						  qp->sq_spec);
		attr->cap.max_inline_data =
			ionic_v1_send_wqe_max_data(qp->sq.stride_log2);
		qp->sq_cqid = cq->cqid;
	}

	if (qp->has_rq) {
		cq = to_ionic_cq(attr->recv_cq);
		spin_lock_irqsave(&cq->lock, irqflags);
		spin_unlock_irqrestore(&cq->lock, irqflags);

		attr->cap.max_recv_wr = qp->rq.mask;
		attr->cap.max_recv_sge =
			ionic_v1_recv_wqe_max_sge(qp->rq.stride_log2,
						  qp->rq_spec);
		qp->rq_cqid = cq->cqid;
	}

	spin_lock_irqsave(&dev->dev_lock, irqflags);
	list_add_tail(&qp->qp_list_ent, &dev->qp_list);
	spin_unlock_irqrestore(&dev->dev_lock, irqflags);

	ionic_dbg_add_qp(dev, qp);

	return &qp->ibqp;

err_resp:
	ionic_destroy_qp_cmd(dev, qp->qpid);
err_cmd:
	ionic_pgtbl_unbuf(dev, &rq_buf);
	ionic_qp_rq_destroy(dev, ctx, qp);
err_rq:
	ionic_pgtbl_unbuf(dev, &sq_buf);
	ionic_qp_sq_destroy(dev, ctx, qp);
err_sq:
	if (qp->has_ah)
		ionic_put_ahid(dev, qp->ahid);
err_ahid:
	ionic_put_qpid(dev, qp->qpid);
err_qpid:
	kfree(qp);
err_qp:
	return ERR_PTR(rc);
}

static void ionic_notify_flush_cq(struct ionic_cq *cq)
{
	if (cq->flush && cq->ibcq.comp_handler)
		cq->ibcq.comp_handler(&cq->ibcq, cq->ibcq.cq_context);
}

static void ionic_notify_qp_cqs(struct ionic_qp *qp)
{
	if (qp->ibqp.send_cq)
		ionic_notify_flush_cq(to_ionic_cq(qp->ibqp.send_cq));
	if (qp->ibqp.recv_cq && qp->ibqp.recv_cq != qp->ibqp.send_cq)
		ionic_notify_flush_cq(to_ionic_cq(qp->ibqp.recv_cq));
}

static void ionic_flush_qp(struct ionic_qp *qp)
{
	struct ionic_cq *cq;
	unsigned long irqflags;

	if (qp->ibqp.send_cq) {
		cq = to_ionic_cq(qp->ibqp.send_cq);

		/* Hold the CQ lock and QP sq_lock to set up flush */
		spin_lock_irqsave(&cq->lock, irqflags);
		spin_lock(&qp->sq_lock);
		qp->sq_flush = true;
		if (!ionic_queue_empty(&qp->sq)) {
			cq->flush = true;
			list_move_tail(&qp->cq_flush_sq, &cq->flush_sq);
		}
		spin_unlock(&qp->sq_lock);
		spin_unlock_irqrestore(&cq->lock, irqflags);
	}

	if (qp->ibqp.recv_cq) {
		cq = to_ionic_cq(qp->ibqp.recv_cq);

		/* Hold the CQ lock and QP rq_lock to set up flush */
		spin_lock_irqsave(&cq->lock, irqflags);
		spin_lock(&qp->rq_lock);
		qp->rq_flush = true;
		if (!ionic_queue_empty(&qp->rq)) {
			cq->flush = true;
			list_move_tail(&qp->cq_flush_rq, &cq->flush_rq);
		}
		spin_unlock(&qp->rq_lock);
		spin_unlock_irqrestore(&cq->lock, irqflags);
	}
}

static void ionic_reset_qp(struct ionic_qp *qp)
{
	struct ionic_cq *cq;
	unsigned long irqflags;
	int i;

	local_irq_save(irqflags);

	if (qp->ibqp.send_cq) {
		cq = to_ionic_cq(qp->ibqp.send_cq);
		spin_lock(&cq->lock);
		ionic_clean_cq(cq, qp->qpid);
		spin_unlock(&cq->lock);
	}

	if (qp->ibqp.recv_cq) {
		cq = to_ionic_cq(qp->ibqp.recv_cq);
		spin_lock(&cq->lock);
		ionic_clean_cq(cq, qp->qpid);
		spin_unlock(&cq->lock);
	}

	if (qp->has_sq) {
		spin_lock(&qp->sq_lock);
		qp->sq_flush = false;
		qp->sq_flush_rcvd = false;
		qp->sq_msn_prod = 0;
		qp->sq_msn_cons = 0;
		qp->sq_cmb_prod = 0;
		qp->sq.prod = 0;
		qp->sq.cons = 0;
		spin_unlock(&qp->sq_lock);
	}

	if (qp->has_rq) {
		spin_lock(&qp->rq_lock);
		qp->rq_flush = false;
		qp->rq_cmb_prod = 0;
		qp->rq.prod = 0;
		qp->rq.cons = 0;
		for (i = 0; i < qp->rq.mask; ++i)
			qp->rq_meta[i].next = &qp->rq_meta[i + 1];
		qp->rq_meta[i].next = IONIC_META_LAST;
		qp->rq_meta_head = &qp->rq_meta[0];
		spin_unlock(&qp->rq_lock);
	}

	local_irq_restore(irqflags);
}

static bool ionic_qp_cur_state_is_ok(enum ib_qp_state q_state,
				     enum ib_qp_state attr_state)
{
	if (q_state == attr_state)
		return true;

	if (attr_state == IB_QPS_ERR)
		return true;

	if (attr_state == IB_QPS_SQE)
		return q_state == IB_QPS_RTS || q_state == IB_QPS_SQD;

	return false;
}

static int ionic_check_modify_qp(struct ionic_qp *qp, struct ib_qp_attr *attr,
				 int mask)
{
	enum ib_qp_state cur_state = (mask & IB_QP_CUR_STATE) ?
		attr->cur_qp_state : qp->state;
	enum ib_qp_state next_state = (mask & IB_QP_STATE) ?
		attr->qp_state : cur_state;

	if ((mask & IB_QP_CUR_STATE) &&
	    !ionic_qp_cur_state_is_ok(qp->state, attr->cur_qp_state))
		return -EINVAL;

	if (!ib_modify_qp_is_ok(cur_state, next_state, qp->ibqp.qp_type, mask,
				IB_LINK_LAYER_ETHERNET))
		return -EINVAL;

	/* unprivileged qp not allowed privileged qkey */
	if ((mask & IB_QP_QKEY) && (attr->qkey & 0x80000000) &&
	    qp->ibqp.uobject)
		return -EPERM;

	return 0;
}

static int ionic_modify_qp(struct ib_qp *ibqp, struct ib_qp_attr *attr,
			   int mask, struct ib_udata *udata)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibqp->device);
	struct ionic_qp *qp = to_ionic_qp(ibqp);
	unsigned long max_rsrq;
	int rc;

	rc = ionic_validate_udata(udata, 0, 0);
	if (rc)
		goto err_qp;

	rc = ionic_check_modify_qp(qp, attr, mask);
	if (rc)
		goto err_qp;

	if (mask & IB_QP_CAP) {
		rc = -EINVAL;
		goto err_qp;
	}

	if (mask & IB_QP_AV)
		qp->dcqcn_profile =
			ionic_dcqcn_select_profile(dev, &attr->ah_attr);

	if ((mask & IB_QP_MAX_QP_RD_ATOMIC) && attr->max_rd_atomic) {
		WARN_ON(ionic_put_res(dev, &qp->rrq_res));

		/* Account for hole and round down to the next order */
		max_rsrq = attr->max_rd_atomic;
		max_rsrq = rounddown_pow_of_two(max_rsrq + 1);

		qp->rrq_res.tbl_order = ionic_res_order(max_rsrq,
							dev->rrq_stride,
							dev->cl_stride);

		rc = ionic_get_res(dev, &qp->rrq_res);
		if (rc)
			goto err_qp;
	}

	if ((mask & IB_QP_MAX_DEST_RD_ATOMIC) && attr->max_dest_rd_atomic) {
		WARN_ON(ionic_put_res(dev, &qp->rsq_res));

		/* Account for hole and round up to the next order */
		max_rsrq = attr->max_dest_rd_atomic;
		max_rsrq = roundup_pow_of_two(max_rsrq + 1);

		qp->rsq_res.tbl_order = ionic_res_order(max_rsrq,
							dev->rsq_stride,
							dev->cl_stride);

		rc = ionic_get_res(dev, &qp->rsq_res);
		if (rc)
			goto err_qp;
	}

	rc = ionic_modify_qp_cmd(dev, qp, attr, mask);
	if (rc)
		goto err_qp;

	if (mask & IB_QP_STATE) {
		qp->state = attr->qp_state;

		if (attr->qp_state == IB_QPS_ERR) {
			ionic_flush_qp(qp);
			ionic_notify_qp_cqs(qp);
		} else if (attr->qp_state == IB_QPS_RESET) {
			ionic_reset_qp(qp);
			ionic_put_res(dev, &qp->rrq_res);
			ionic_put_res(dev, &qp->rsq_res);
		}
	}

	return 0;

err_qp:
	if (mask & IB_QP_MAX_QP_RD_ATOMIC)
		ionic_put_res(dev, &qp->rrq_res);

	if (mask & IB_QP_MAX_DEST_RD_ATOMIC)
		ionic_put_res(dev, &qp->rsq_res);

	return rc;
}

static int ionic_query_qp_cmd(struct ionic_ibdev *dev,
			      struct ionic_qp *qp,
			      struct ib_qp_attr *attr,
			      int mask)
{
	struct ionic_admin_wr wr = {
		.work = COMPLETION_INITIALIZER_ONSTACK(wr.work),
		.wqe = {
			.op = IONIC_V1_ADMIN_QUERY_QP,
			.id_ver = cpu_to_le32(qp->qpid),
		}
	};
	struct ionic_v1_admin_query_qp_sq *query_sqbuf;
	struct ionic_v1_admin_query_qp_rq *query_rqbuf;
	dma_addr_t query_sqdma;
	dma_addr_t query_rqdma;
	dma_addr_t hdr_dma = 0;
	void *hdr_buf = NULL;
	int flags, rc;

	if (dev->admin_opcodes <= IONIC_V1_ADMIN_QUERY_QP)
		return -ENOSYS;

	if (qp->has_sq) {
		attr->cap.max_send_sge =
			ionic_v1_send_wqe_max_sge(qp->sq.stride_log2,
						  qp->sq_spec);
		attr->cap.max_inline_data =
			ionic_v1_send_wqe_max_data(qp->sq.stride_log2);
	}

	if (qp->has_rq) {
		attr->cap.max_recv_sge =
			ionic_v1_recv_wqe_max_sge(qp->rq.stride_log2,
						  qp->rq_spec);
	}

	query_sqbuf = contig_kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!query_sqbuf) {
		rc = -ENOMEM;
		goto err_sqbuf;
	}
	query_rqbuf = contig_kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!query_rqbuf) {
		rc = -ENOMEM;
		goto err_rqbuf;
	}

	query_sqdma = dma_map_single(dev->hwdev, query_sqbuf, PAGE_SIZE,
				     DMA_FROM_DEVICE);
	rc = dma_mapping_error(dev->hwdev, query_sqdma);
	if (rc)
		goto err_sqdma;

	query_rqdma = dma_map_single(dev->hwdev, query_rqbuf, PAGE_SIZE,
				     DMA_FROM_DEVICE);
	rc = dma_mapping_error(dev->hwdev, query_rqdma);
	if (rc)
		goto err_rqdma;

	if (mask & IB_QP_AV) {
		hdr_buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
		if (!hdr_buf) {
			rc = -ENOMEM;
			goto err_hdrbuf;
		}

		hdr_dma = dma_map_single(dev->hwdev, hdr_buf,
					 PAGE_SIZE, DMA_FROM_DEVICE);
		rc = dma_mapping_error(dev->hwdev, hdr_dma);
		if (rc)
			goto err_hdrdma;
	}

	wr.wqe.query_qp.sq_dma_addr = cpu_to_le64(query_sqdma);
	wr.wqe.query_qp.rq_dma_addr = cpu_to_le64(query_rqdma);
	wr.wqe.query_qp.hdr_dma_addr = cpu_to_le64(hdr_dma);
	wr.wqe.query_qp.ah_id = cpu_to_le32(qp->ahid);

	ionic_admin_post(dev, &wr);
	ionic_admin_wait(&wr);

	if (wr.status == IONIC_ADMIN_KILLED) {
		dev_dbg(&dev->ibdev.dev, "killed\n");
		rc = -ENODEV;
	} else if (ionic_v1_cqe_error(&wr.cqe)) {
		dev_warn(&dev->ibdev.dev, "cqe error %u\n",
			 be32_to_cpu(wr.cqe.status_length));
		rc = -EINVAL;
	} else {
		rc = 0;
	}

	dma_unmap_single(dev->hwdev, query_sqdma, sizeof(*query_sqbuf),
			 DMA_FROM_DEVICE);
	dma_unmap_single(dev->hwdev, query_rqdma, sizeof(*query_rqbuf),
			 DMA_FROM_DEVICE);

	if (mask & IB_QP_AV)
		dma_unmap_single(dev->hwdev, hdr_dma,
				 PAGE_SIZE, DMA_FROM_DEVICE);

	if (rc)
		goto err_sqdma;

	flags = be16_to_cpu(query_sqbuf->access_perms_flags);

	attr->qp_state = from_ionic_qp_state(query_rqbuf->state_pmtu & 0xf);
	attr->cur_qp_state = attr->qp_state;
	attr->path_mtu = (query_rqbuf->state_pmtu >> 4) - 7;
	attr->path_mig_state = IB_MIG_MIGRATED;
	attr->qkey = be32_to_cpu(query_sqbuf->qkey_dest_qpn);
	attr->rq_psn = be32_to_cpu(query_sqbuf->rq_psn);
	attr->sq_psn = be32_to_cpu(query_rqbuf->sq_psn);
	attr->dest_qp_num = attr->qkey;
	attr->qp_access_flags = from_ionic_qp_flags(flags);
	attr->pkey_index = 0;
	attr->alt_pkey_index = 0;
	attr->en_sqd_async_notify = !!(flags & IONIC_QPF_SQD_NOTIFY);
	attr->sq_draining = !!(flags & IONIC_QPF_SQ_DRAINING);
	attr->max_rd_atomic = BIT(query_rqbuf->rrq_depth) - 1;
	attr->max_dest_rd_atomic = BIT(query_rqbuf->rsq_depth) - 1;
	attr->min_rnr_timer = query_sqbuf->rnr_timer;
	attr->port_num = 0;
	attr->timeout = query_sqbuf->retry_timeout;
	attr->retry_cnt = query_rqbuf->retry_rnrtry & 0xf;
	attr->rnr_retry = query_rqbuf->retry_rnrtry >> 4;
	attr->alt_port_num = 0;
	attr->alt_timeout = 0;

	if (mask & IB_QP_AV)
		rc = ionic_set_ah_attr(dev, &attr->ah_attr,
				       hdr_buf, qp->sgid_index);

err_hdrdma:
	kfree(hdr_buf);
err_hdrbuf:
	dma_unmap_single(dev->hwdev, query_rqdma, sizeof(*query_rqbuf),
			 DMA_FROM_DEVICE);
err_rqdma:
	dma_unmap_single(dev->hwdev, query_sqdma, sizeof(*query_sqbuf),
			 DMA_FROM_DEVICE);
err_sqdma:
	contig_kfree(query_rqbuf, sizeof(*query_rqbuf));
err_rqbuf:
	contig_kfree(query_sqbuf, sizeof(*query_sqbuf));
err_sqbuf:
	return rc;
}

static int ionic_query_qp(struct ib_qp *ibqp, struct ib_qp_attr *attr,
			  int mask, struct ib_qp_init_attr *init_attr)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibqp->device);
	struct ionic_qp *qp = to_ionic_qp(ibqp);
	int rc;

	memset(attr, 0, sizeof(*attr));
	memset(init_attr, 0, sizeof(*init_attr));

	rc = ionic_query_qp_cmd(dev, qp, attr, mask);
	if (rc)
		goto err_cmd;

	if (qp->has_sq)
		attr->cap.max_send_wr = qp->sq.mask;

	if (qp->has_rq)
		attr->cap.max_recv_wr = qp->rq.mask;

	init_attr->event_handler = ibqp->event_handler;
	init_attr->qp_context = ibqp->qp_context;
	init_attr->send_cq = ibqp->send_cq;
	init_attr->recv_cq = ibqp->recv_cq;
	init_attr->srq = ibqp->srq;
	init_attr->xrcd = ibqp->xrcd;
	init_attr->cap = attr->cap;
	init_attr->sq_sig_type = qp->sig_all ?
		IB_SIGNAL_ALL_WR : IB_SIGNAL_REQ_WR;
	init_attr->qp_type = ibqp->qp_type;
	init_attr->create_flags = 0;
	init_attr->port_num = 0;
	init_attr->rwq_ind_tbl = ibqp->rwq_ind_tbl;

err_cmd:
	return rc;
}

static int ionic_destroy_qp(struct ib_qp *ibqp)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibqp->device);
	struct ionic_ctx *ctx = to_ionic_ctx_uobj(ibqp->uobject);
	struct ionic_qp *qp = to_ionic_qp(ibqp);
	struct ionic_cq *cq;
	unsigned long irqflags;
	int rc;

	rc = ionic_destroy_qp_cmd(dev, qp->qpid);
	if (rc)
		return rc;

	xa_erase_irq(&dev->qp_tbl, qp->qpid);

	ionic_qp_rem_ref(qp);
	wait_for_completion(&qp->qp_rel_comp);

	spin_lock_irqsave(&dev->dev_lock, irqflags);
	list_del(&qp->qp_list_ent);
	spin_unlock_irqrestore(&dev->dev_lock, irqflags);

	ionic_dbg_rm_qp(qp);

	if (qp->ibqp.send_cq) {
		cq = to_ionic_cq(qp->ibqp.send_cq);
		spin_lock_irqsave(&cq->lock, irqflags);
		ionic_clean_cq(cq, qp->qpid);
		list_del(&qp->cq_poll_sq);
		list_del(&qp->cq_flush_sq);
		spin_unlock_irqrestore(&cq->lock, irqflags);
	}

	if (qp->ibqp.recv_cq) {
		cq = to_ionic_cq(qp->ibqp.recv_cq);
		spin_lock_irqsave(&cq->lock, irqflags);
		ionic_clean_cq(cq, qp->qpid);
		list_del(&qp->cq_flush_rq);
		spin_unlock_irqrestore(&cq->lock, irqflags);
	}

	ionic_qp_rq_destroy(dev, ctx, qp);
	ionic_qp_sq_destroy(dev, ctx, qp);
	if (qp->has_ah)
		ionic_put_ahid(dev, qp->ahid);
	ionic_put_qpid(dev, qp->qpid);

	ionic_put_res(dev, &qp->rrq_res);
	ionic_put_res(dev, &qp->rsq_res);

	kfree(qp);

	return 0;
}

static s64 ionic_prep_inline(void *data, u32 max_data,
			     const struct ib_sge *ib_sgl, int num_sge)
{
	static const s64 bit_31 = 1u << 31;
	s64 len = 0, sg_len;
	int sg_i;

	for (sg_i = 0; sg_i < num_sge; ++sg_i) {
		sg_len = ib_sgl[sg_i].length;

		/* sge length zero means 2GB */
		if (unlikely(sg_len == 0))
			sg_len = bit_31;

		/* greater than max inline data is invalid */
		if (unlikely(len + sg_len > max_data))
			return -EINVAL;

		memcpy(data + len, (void *)ib_sgl[sg_i].addr, sg_len);

		len += sg_len;
	}

	return len;
}

static s64 ionic_prep_pld(struct ionic_v1_wqe *wqe,
			  union ionic_v1_pld *pld,
			  int spec, u32 max_sge,
			  const struct ib_sge *ib_sgl,
			  int num_sge)
{
	static const s64 bit_31 = 1l << 31;
	struct ionic_sge *sgl;
	__be32 *spec32 = NULL;
	__be16 *spec16 = NULL;
	s64 len = 0, sg_len;
	int sg_i = 0;

	if (unlikely(num_sge < 0 || (u32)num_sge > max_sge))
		return -EINVAL;

	if (spec && num_sge > IONIC_V1_SPEC_FIRST_SGE) {
		sg_i = IONIC_V1_SPEC_FIRST_SGE;

		if (num_sge > 8) {
			wqe->base.flags |= cpu_to_be16(IONIC_V1_FLAG_SPEC16);
			spec16 = pld->spec16;
		} else {
			wqe->base.flags |= cpu_to_be16(IONIC_V1_FLAG_SPEC32);
			spec32 = pld->spec32;
		}
	}

	sgl = &pld->sgl[sg_i];

	for (sg_i = 0; sg_i < num_sge; ++sg_i) {
		sg_len = ib_sgl[sg_i].length;

		/* sge length zero means 2GB */
		if (unlikely(sg_len == 0))
			sg_len = bit_31;

		/* greater than 2GB data is invalid */
		if (unlikely(len + sg_len > bit_31))
			return -EINVAL;

		sgl[sg_i].va = cpu_to_be64(ib_sgl[sg_i].addr);
		sgl[sg_i].len = cpu_to_be32(sg_len);
		sgl[sg_i].lkey = cpu_to_be32(ib_sgl[sg_i].lkey);

		if (spec32) {
			spec32[sg_i] = sgl[sg_i].len;
		} else if (spec16) {
			if (unlikely(sg_len > U16_MAX))
				return -EINVAL;
			spec16[sg_i] = cpu_to_be16(sg_len);
		}

		len += sg_len;
	}

	return len;
}

static void ionic_prep_base(struct ionic_qp *qp,
			    struct ib_send_wr *wr,
			    struct ionic_sq_meta *meta,
			    struct ionic_v1_wqe *wqe)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(qp->ibqp.device);

	meta->wrid = wr->wr_id;
	meta->ibsts = IB_WC_SUCCESS;
	meta->signal = false;
	meta->local_comp = false;

	wqe->base.wqe_id = qp->sq.prod;

	if (wr->send_flags & IB_SEND_FENCE)
		wqe->base.flags |= cpu_to_be16(IONIC_V1_FLAG_FENCE);

	if (wr->send_flags & IB_SEND_SOLICITED)
		wqe->base.flags |= cpu_to_be16(IONIC_V1_FLAG_SOL);

	if (qp->sig_all || wr->send_flags & IB_SEND_SIGNALED) {
		wqe->base.flags |= cpu_to_be16(IONIC_V1_FLAG_SIG);
		meta->signal = true;
	}

	meta->seq = qp->sq_msn_prod;
	meta->remote =
		qp->ibqp.qp_type != IB_QPT_UD &&
		qp->ibqp.qp_type != IB_QPT_GSI &&
		!ionic_ibop_is_local(wr->opcode);

	if (meta->remote) {
		qp->sq_msn_idx[meta->seq] = qp->sq.prod;
		qp->sq_msn_prod = ionic_queue_next(&qp->sq, qp->sq_msn_prod);
	}

	dev_dbg(&dev->ibdev.dev,
		"post send %u prod %u\n", qp->qpid, qp->sq.prod);
	print_hex_dump_debug("wqe ", DUMP_PREFIX_OFFSET, 16, 1,
			     wqe, BIT(qp->sq.stride_log2), true);

	ionic_queue_produce(&qp->sq);
}

static int ionic_prep_common(struct ionic_qp *qp,
			     struct ib_send_wr *wr,
			     struct ionic_sq_meta *meta,
			     struct ionic_v1_wqe *wqe)
{
	s64 signed_len;
	u32 mval;

	if (wr->send_flags & IB_SEND_INLINE) {
		wqe->base.num_sge_key = 0;
		wqe->base.flags |= cpu_to_be16(IONIC_V1_FLAG_INL);
		mval = ionic_v1_send_wqe_max_data(qp->sq.stride_log2);
		signed_len = ionic_prep_inline(wqe->common.pld.data, mval,
					       wr->sg_list, wr->num_sge);
	} else {
		wqe->base.num_sge_key = wr->num_sge;
		mval = ionic_v1_send_wqe_max_sge(qp->sq.stride_log2,
						 qp->sq_spec);
		signed_len = ionic_prep_pld(wqe, &wqe->common.pld,
					    qp->sq_spec, mval,
					    wr->sg_list, wr->num_sge);
	}

	if (unlikely(signed_len < 0))
		return signed_len;

	meta->len = signed_len;
	wqe->common.length = cpu_to_be32(signed_len);

	ionic_prep_base(qp, wr, meta, wqe);

	return 0;
}

static int ionic_prep_send(struct ionic_qp *qp,
			   struct ib_send_wr *wr)
{
	struct ionic_sq_meta *meta;
	struct ionic_v1_wqe *wqe;

	meta = &qp->sq_meta[qp->sq.prod];
	wqe = ionic_queue_at_prod(&qp->sq);

	memset(wqe, 0, 1u << qp->sq.stride_log2);

	meta->ibop = IB_WC_SEND;

	switch (wr->opcode) {
	case IB_WR_SEND:
		wqe->base.op = IONIC_V1_OP_SEND;
		break;
	case IB_WR_SEND_WITH_IMM:
		wqe->base.op = IONIC_V1_OP_SEND_IMM;
		wqe->base.imm_data_key = wr->ex.imm_data;
		break;
	case IB_WR_SEND_WITH_INV:
		wqe->base.op = IONIC_V1_OP_SEND_INV;
		wqe->base.imm_data_key =
			cpu_to_be32(wr->ex.invalidate_rkey);
		break;
	default:
		return -EINVAL;
	}

	return ionic_prep_common(qp, wr, meta, wqe);
}

static int ionic_prep_send_ud(struct ionic_qp *qp,
			      struct ib_ud_wr *wr)
{
	struct ionic_sq_meta *meta;
	struct ionic_v1_wqe *wqe;
	struct ionic_ah *ah;

	if (unlikely(!wr->ah))
		return -EINVAL;

	ah = to_ionic_ah(wr->ah);

	meta = &qp->sq_meta[qp->sq.prod];
	wqe = ionic_queue_at_prod(&qp->sq);

	memset(wqe, 0, 1u << qp->sq.stride_log2);

	wqe->common.send.ah_id = cpu_to_be32(ah->ahid);
	wqe->common.send.dest_qpn = cpu_to_be32(wr->remote_qpn);
	wqe->common.send.dest_qkey = cpu_to_be32(wr->remote_qkey);

	meta->ibop = IB_WC_SEND;

	switch (wr->wr.opcode) {
	case IB_WR_SEND:
		wqe->base.op = IONIC_V1_OP_SEND;
		break;
	case IB_WR_SEND_WITH_IMM:
		wqe->base.op = IONIC_V1_OP_SEND_IMM;
		wqe->base.imm_data_key = wr->wr.ex.imm_data;
		break;
	default:
		return -EINVAL;
	}

	return ionic_prep_common(qp, &wr->wr, meta, wqe);
}

static int ionic_prep_rdma(struct ionic_qp *qp,
			   struct ib_rdma_wr *wr)
{
	struct ionic_sq_meta *meta;
	struct ionic_v1_wqe *wqe;

	meta = &qp->sq_meta[qp->sq.prod];
	wqe = ionic_queue_at_prod(&qp->sq);

	memset(wqe, 0, 1u << qp->sq.stride_log2);

	meta->ibop = IB_WC_RDMA_WRITE;

	switch (wr->wr.opcode) {
	case IB_WR_RDMA_READ:
		if (wr->wr.send_flags & (IB_SEND_SOLICITED | IB_SEND_INLINE))
			return -EINVAL;
		meta->ibop = IB_WC_RDMA_READ;
		wqe->base.op = IONIC_V1_OP_RDMA_READ;
		break;
	case IB_WR_RDMA_WRITE:
		if (wr->wr.send_flags & IB_SEND_SOLICITED)
			return -EINVAL;
		wqe->base.op = IONIC_V1_OP_RDMA_WRITE;
		break;
	case IB_WR_RDMA_WRITE_WITH_IMM:
		wqe->base.op = IONIC_V1_OP_RDMA_WRITE_IMM;
		wqe->base.imm_data_key = wr->wr.ex.imm_data;
		break;
	default:
		return -EINVAL;
	}

	wqe->common.rdma.remote_va_high = cpu_to_be32(wr->remote_addr >> 32);
	wqe->common.rdma.remote_va_low = cpu_to_be32(wr->remote_addr);
	wqe->common.rdma.remote_rkey = cpu_to_be32(wr->rkey);

	return ionic_prep_common(qp, &wr->wr, meta, wqe);
}

static int ionic_prep_atomic(struct ionic_qp *qp,
			     struct ib_atomic_wr *wr)
{
	struct ionic_sq_meta *meta;
	struct ionic_v1_wqe *wqe;

	if (wr->wr.num_sge != 1 || wr->wr.sg_list[0].length != 8)
		return -EINVAL;

	if (wr->wr.send_flags & (IB_SEND_SOLICITED | IB_SEND_INLINE))
		return -EINVAL;

	meta = &qp->sq_meta[qp->sq.prod];
	wqe = ionic_queue_at_prod(&qp->sq);

	memset(wqe, 0, 1u << qp->sq.stride_log2);

	meta->ibop = IB_WC_RDMA_WRITE;

	switch (wr->wr.opcode) {
	case IB_WR_ATOMIC_CMP_AND_SWP:
		meta->ibop = IB_WC_COMP_SWAP;
		wqe->base.op = IONIC_V1_OP_ATOMIC_CS;
		wqe->atomic.swap_add_high = cpu_to_be32(wr->swap >> 32);
		wqe->atomic.swap_add_low = cpu_to_be32(wr->swap);
		wqe->atomic.compare_high = cpu_to_be32(wr->compare_add >> 32);
		wqe->atomic.compare_low = cpu_to_be32(wr->compare_add);
		break;
	case IB_WR_ATOMIC_FETCH_AND_ADD:
		meta->ibop = IB_WC_FETCH_ADD;
		wqe->base.op = IONIC_V1_OP_ATOMIC_FA;
		wqe->atomic.swap_add_high = cpu_to_be32(wr->compare_add >> 32);
		wqe->atomic.swap_add_low = cpu_to_be32(wr->compare_add);
		break;
	default:
		return -EINVAL;
	}

	wqe->atomic.remote_va_high = cpu_to_be32(wr->remote_addr >> 32);
	wqe->atomic.remote_va_low = cpu_to_be32(wr->remote_addr);
	wqe->atomic.remote_rkey = cpu_to_be32(wr->rkey);

	wqe->base.num_sge_key = 1;
	wqe->atomic.sge.va = cpu_to_be64(wr->wr.sg_list[0].addr);
	wqe->atomic.sge.len = cpu_to_be32(8);
	wqe->atomic.sge.lkey = cpu_to_be32(wr->wr.sg_list[0].lkey);

	return ionic_prep_common(qp, &wr->wr, meta, wqe);
}

static int ionic_prep_inv(struct ionic_qp *qp,
			  struct ib_send_wr *wr)
{
	struct ionic_sq_meta *meta;
	struct ionic_v1_wqe *wqe;

	if (wr->send_flags & (IB_SEND_SOLICITED | IB_SEND_INLINE))
		return -EINVAL;

	meta = &qp->sq_meta[qp->sq.prod];
	wqe = ionic_queue_at_prod(&qp->sq);

	memset(wqe, 0, 1u << qp->sq.stride_log2);

	wqe->base.op = IONIC_V1_OP_LOCAL_INV;
	wqe->base.imm_data_key = cpu_to_be32(wr->ex.invalidate_rkey);

	meta->len = 0;
	meta->ibop = IB_WC_LOCAL_INV;

	ionic_prep_base(qp, wr, meta, wqe);

	return 0;
}

static int ionic_prep_reg(struct ionic_qp *qp,
			  struct ib_reg_wr *wr)
{
	struct ionic_mr *mr = to_ionic_mr(wr->mr);
	struct ionic_sq_meta *meta;
	struct ionic_v1_wqe *wqe;
	int flags;

	if (wr->wr.send_flags & (IB_SEND_SOLICITED | IB_SEND_INLINE))
		return EINVAL;

	/* must call ib_map_mr_sg before posting reg wr */
	if (!mr->buf.tbl_pages)
		return EINVAL;

	meta = &qp->sq_meta[qp->sq.prod];
	wqe = ionic_queue_at_prod(&qp->sq);

	memset(wqe, 0, 1u << qp->sq.stride_log2);

	flags = to_ionic_mr_flags(wr->access);

	wqe->base.op = IONIC_V1_OP_REG_MR;
	wqe->base.num_sge_key = wr->key;
	wqe->base.imm_data_key = cpu_to_be32(mr->ibmr.lkey);
	wqe->reg_mr.va = cpu_to_be64(mr->ibmr.iova);
	wqe->reg_mr.length = cpu_to_be64(mr->ibmr.length);
	wqe->reg_mr.offset = ionic_pgtbl_off(&mr->buf, mr->ibmr.iova),
	wqe->reg_mr.dma_addr = ionic_pgtbl_dma(&mr->buf, mr->ibmr.iova);

	wqe->reg_mr.map_count = cpu_to_be32(mr->buf.tbl_pages);
	wqe->reg_mr.flags = cpu_to_be16(flags);
	wqe->reg_mr.dir_size_log2 = 0;
	wqe->reg_mr.page_size_log2 = order_base_2(mr->ibmr.page_size);

	meta->len = 0;
	meta->ibop = IB_WC_REG_MR;

	ionic_prep_base(qp, &wr->wr, meta, wqe);

	return 0;
}

static int ionic_prep_one_rc(struct ionic_qp *qp,
			     struct ib_send_wr *wr)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(qp->ibqp.device);
	int rc = 0;

	switch (wr->opcode) {
	case IB_WR_SEND:
	case IB_WR_SEND_WITH_IMM:
	case IB_WR_SEND_WITH_INV:
		rc = ionic_prep_send(qp, wr);
		break;
	case IB_WR_RDMA_READ:
	case IB_WR_RDMA_WRITE:
	case IB_WR_RDMA_WRITE_WITH_IMM:
		rc = ionic_prep_rdma(qp, rdma_wr(wr));
		break;
	case IB_WR_ATOMIC_CMP_AND_SWP:
	case IB_WR_ATOMIC_FETCH_AND_ADD:
		rc = ionic_prep_atomic(qp, atomic_wr(wr));
		break;
	case IB_WR_LOCAL_INV:
		rc = ionic_prep_inv(qp, wr);
		break;
	case IB_WR_REG_MR:
		rc = ionic_prep_reg(qp, reg_wr(wr));
		break;
	default:
		dev_dbg(&dev->ibdev.dev, "invalid opcode %d\n", wr->opcode);
		rc = -EINVAL;
	}

	return rc;
}

static int ionic_prep_one_ud(struct ionic_qp *qp,
			     struct ib_send_wr *wr)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(qp->ibqp.device);
	int rc = 0;

	switch (wr->opcode) {
	case IB_WR_SEND:
	case IB_WR_SEND_WITH_IMM:
		rc = ionic_prep_send_ud(qp, ud_wr(wr));
		break;
	default:
		dev_dbg(&dev->ibdev.dev, "invalid opcode %d\n", wr->opcode);
		rc = -EINVAL;
	}

	return rc;
}

static void ionic_post_send_cmb(struct ionic_ibdev *dev, struct ionic_qp *qp)
{
	void __iomem *cmb_ptr;
	void *wqe_ptr;
	u32 stride;
	u16 pos, end;
	u8 stride_log2;

	stride_log2 = qp->sq.stride_log2;
	stride = BIT(stride_log2);

	pos = qp->sq_cmb_prod;
	end = qp->sq.prod;

	while (pos != end) {
		cmb_ptr = qp->sq_cmb_ptr + ((size_t)pos << stride_log2);
		wqe_ptr = ionic_queue_at(&qp->sq, pos);

		memcpy_toio(cmb_ptr, wqe_ptr, stride);

		pos = ionic_queue_next(&qp->sq, pos);

		if (ionic_xxx_qp_dbell)
			ionic_dbell_ring(dev->dbpage, dev->sq_qtype,
					 qp->sq.dbell | pos);
	}

	qp->sq_cmb_prod = end;
}

static void ionic_post_recv_cmb(struct ionic_ibdev *dev, struct ionic_qp *qp)
{
	void __iomem *cmb_ptr;
	void *wqe_ptr;
	u32 stride;
	u16 pos, end;
	u8 stride_log2;

	stride_log2 = qp->rq.stride_log2;

	pos = qp->rq_cmb_prod;
	end = qp->rq.prod;


	if (pos > end) {
		cmb_ptr = qp->rq_cmb_ptr + ((size_t)pos << stride_log2);
		wqe_ptr = ionic_queue_at(&qp->rq, pos);

		stride = (u32)(qp->rq.mask - pos + 1) << stride_log2;
		memcpy_toio(cmb_ptr, wqe_ptr, stride);

		pos = 0;

		ionic_dbell_ring(dev->dbpage, dev->rq_qtype,
				 qp->rq.dbell | pos);
	}

	if (pos < end) {
		cmb_ptr = qp->rq_cmb_ptr + ((size_t)pos << stride_log2);
		wqe_ptr = ionic_queue_at(&qp->rq, pos);

		stride = (u32)(end - pos) << stride_log2;
		memcpy_toio(cmb_ptr, wqe_ptr, stride);

		pos = end;

		ionic_dbell_ring(dev->dbpage, dev->rq_qtype,
				 qp->rq.dbell | pos);
	}

	qp->rq_cmb_prod = end;
}

static int ionic_prep_recv(struct ionic_qp *qp,
			   struct ib_recv_wr *wr)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(qp->ibqp.device);
	struct ionic_rq_meta *meta;
	struct ionic_v1_wqe *wqe;
	s64 signed_len;
	u32 mval;

	wqe = ionic_queue_at_prod(&qp->rq);

	/* if wqe is owned by device, caller can try posting again soon */
	if (wqe->base.flags & cpu_to_be16(IONIC_V1_FLAG_FENCE))
		return -EAGAIN;

	meta = qp->rq_meta_head;
	if (unlikely(meta == IONIC_META_LAST) ||
	    unlikely(meta == IONIC_META_POSTED))
		return -EIO;

	memset(wqe, 0, 1u << qp->rq.stride_log2);

	mval = ionic_v1_recv_wqe_max_sge(qp->rq.stride_log2, qp->rq_spec);
	signed_len = ionic_prep_pld(wqe, &wqe->recv.pld,
				    qp->rq_spec, mval,
				    wr->sg_list, wr->num_sge);
	if (signed_len < 0)
		return signed_len;

	meta->wrid = wr->wr_id;

	wqe->base.wqe_id = meta - qp->rq_meta;
	wqe->base.num_sge_key = wr->num_sge;

	/* total length for recv goes in base imm_data_key */
	wqe->base.imm_data_key = cpu_to_be32(signed_len);

	/* if this is a srq, set fence bit to indicate device ownership */
	if (qp->is_srq)
		wqe->base.flags |= cpu_to_be16(IONIC_V1_FLAG_FENCE);

	dev_dbg(&dev->ibdev.dev,
		"post recv %u prod %u\n", qp->qpid, qp->rq.prod);
	print_hex_dump_debug("wqe ", DUMP_PREFIX_OFFSET, 16, 1,
			     wqe, BIT(qp->rq.stride_log2), true);

	ionic_queue_produce(&qp->rq);

	qp->rq_meta_head = meta->next;
	meta->next = IONIC_META_POSTED;

	return 0;
}

static int ionic_post_send_common(struct ionic_ibdev *dev,
				  struct ionic_ctx *ctx,
				  struct ionic_cq *cq,
				  struct ionic_qp *qp,
				  struct ib_send_wr *wr,
				  struct ib_send_wr **bad)
{
	unsigned long irqflags;
	bool notify = false;
	int spend, rc = 0;

	if (!bad)
		return -EINVAL;

	if (ctx || !qp->has_sq) {
		*bad = wr;
		return -EINVAL;
	}

	if (qp->state < IB_QPS_RTS) {
		*bad = wr;
		return -EINVAL;
	}

	spin_lock_irqsave(&qp->sq_lock, irqflags);

	if (qp->ibqp.qp_type == IB_QPT_UD || qp->ibqp.qp_type == IB_QPT_GSI) {
		while (wr) {
			if (ionic_queue_full(&qp->sq)) {
				dev_dbg(&dev->ibdev.dev, "queue full");
				rc = -ENOMEM;
				goto out;
			}

			rc = ionic_prep_one_ud(qp, wr);
			if (rc)
				goto out;

			wr = wr->next;
		}
	} else {
		while (wr) {
			if (ionic_queue_full(&qp->sq)) {
				dev_dbg(&dev->ibdev.dev, "queue full");
				rc = -ENOMEM;
				goto out;
			}

			rc = ionic_prep_one_rc(qp, wr);
			if (rc)
				goto out;

			wr = wr->next;
		}
	}

out:
	/* irq remains saved here, not restored/saved again */
	if (!spin_trylock(&cq->lock)) {
		spin_unlock(&qp->sq_lock);
		spin_lock(&cq->lock);
		spin_lock(&qp->sq_lock);
	}

	if (likely(qp->sq.prod != qp->sq_old_prod)) {
		/* ring cq doorbell just in time */
		spend = (qp->sq.prod - qp->sq_old_prod) & qp->sq.mask;
		ionic_reserve_cq(dev, cq, spend);

		qp->sq_old_prod = qp->sq.prod;

		if (qp->sq_cmb_ptr)
			ionic_post_send_cmb(dev, qp);
		else if (ionic_xxx_qp_dbell)
			ionic_dbell_ring(dev->dbpage, dev->sq_qtype,
					 ionic_queue_dbell_val(&qp->sq));
	}

	if (qp->sq_flush) {
		notify = true;
		cq->flush = true;
		list_move_tail(&qp->cq_flush_sq, &cq->flush_sq);
	}

	spin_unlock(&qp->sq_lock);
	spin_unlock_irqrestore(&cq->lock, irqflags);

	if (notify && cq->ibcq.comp_handler)
		cq->ibcq.comp_handler(&cq->ibcq, cq->ibcq.cq_context);

	*bad = wr;
	return rc;
}

static int ionic_post_recv_common(struct ionic_ibdev *dev,
				  struct ionic_ctx *ctx,
				  struct ionic_cq *cq,
				  struct ionic_qp *qp,
				  struct ib_recv_wr *wr,
				  struct ib_recv_wr **bad)
{
	unsigned long irqflags;
	bool notify = false;
	int spend, rc = 0;

	if (!bad)
		return -EINVAL;

	if (ctx || !qp->has_rq) {
		*bad = wr;
		return -EINVAL;
	}

	if (qp->state < IB_QPS_INIT) {
		*bad = wr;
		return -EINVAL;
	}

	spin_lock_irqsave(&qp->rq_lock, irqflags);

	while (wr) {
		if (ionic_queue_full(&qp->rq)) {
			dev_dbg(&dev->ibdev.dev, "queue full");
			rc = -ENOMEM;
			goto out;
		}

		rc = ionic_prep_recv(qp, wr);
		if (rc)
			goto out;

		wr = wr->next;
	}

out:
	if (!cq) {
		spin_unlock_irqrestore(&qp->rq_lock, irqflags);
		goto out_unlocked;
	}

	/* irq remains saved here, not restored/saved again */
	if (!spin_trylock(&cq->lock)) {
		spin_unlock(&qp->rq_lock);
		spin_lock(&cq->lock);
		spin_lock(&qp->rq_lock);
	}

	if (likely(qp->rq.prod != qp->rq_old_prod)) {
		/* ring cq doorbell just in time */
		spend = (qp->rq.prod - qp->rq_old_prod) & qp->rq.mask;
		ionic_reserve_cq(dev, cq, spend);

		qp->rq_old_prod = qp->rq.prod;

		if (qp->rq_cmb_ptr)
			ionic_post_recv_cmb(dev, qp);
		else
			ionic_dbell_ring(dev->dbpage, dev->rq_qtype,
					 ionic_queue_dbell_val(&qp->rq));
	}

	if (qp->rq_flush) {
		notify = true;
		cq->flush = true;
		list_move_tail(&qp->cq_flush_rq, &cq->flush_rq);
	}

	spin_unlock(&qp->rq_lock);
	spin_unlock_irqrestore(&cq->lock, irqflags);

	if (notify && cq->ibcq.comp_handler)
		cq->ibcq.comp_handler(&cq->ibcq, cq->ibcq.cq_context);

out_unlocked:
	*bad = wr;
	return rc;
}


static int ionic_post_send(struct ib_qp *ibqp,
			   struct ib_send_wr *wr,
			   struct ib_send_wr **bad)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibqp->device);
	struct ionic_ctx *ctx = to_ionic_ctx_uobj(ibqp->uobject);
	struct ionic_cq *cq = to_ionic_cq(ibqp->send_cq);
	struct ionic_qp *qp = to_ionic_qp(ibqp);

	return ionic_post_send_common(dev, ctx, cq, qp, wr, bad);
}

static int ionic_post_recv(struct ib_qp *ibqp,
			   struct ib_recv_wr *wr,
			   struct ib_recv_wr **bad)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibqp->device);
	struct ionic_ctx *ctx = to_ionic_ctx_uobj(ibqp->uobject);
	struct ionic_cq *cq = to_ionic_cq(ibqp->recv_cq);
	struct ionic_qp *qp = to_ionic_qp(ibqp);

	return ionic_post_recv_common(dev, ctx, cq, qp, wr, bad);
}

#ifdef IONIC_SRQ_XRC
static struct ib_srq *ionic_create_srq(struct ib_pd *ibpd,
				       struct ib_srq_init_attr *attr,
				       struct ib_udata *udata)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibpd->device);
	struct ionic_ctx *ctx = to_ionic_ctx_uobj(ibpd->uobject);
	struct ionic_qp *qp;
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

	qp = kzalloc(sizeof(*qp), GFP_KERNEL);
	if (!qp) {
		rc = -ENOSYS;
		goto err_srq;
	}

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

		cq = to_ionic_cq(attr->ext.xrc.cq);
		spin_lock_irqsave(&cq->lock, irqflags);
		spin_unlock_irqrestore(&cq->lock, irqflags);
	}

	attr->attr.max_wr = qp->rq.mask;
	attr->attr.max_sge =
		ionic_v1_recv_wqe_max_sge(qp->rq.stride_log2,
					  qp->rq_spec);

	ionic_dbg_add_qp(dev, qp);

	return &qp->ibsrq;

err_resp:
	ionic_destroy_qp_cmd(dev, qp->qpid);
err_cmd:
	ionic_pgtbl_unbuf(dev, &rq_buf);
	ionic_qp_rq_destroy(dev, ctx, qp);
err_rq:
	ionic_put_srqid(dev, qp->qpid);
err_srqid:
	kfree(qp);
err_srq:
	return ERR_PTR(rc);
}

static int ionic_modify_srq(struct ib_srq *ibsrq, struct ib_srq_attr *attr,
			    enum ib_srq_attr_mask mask, struct ib_udata *udata)
{
	return -ENOSYS;
}

static int ionic_query_srq(struct ib_srq *ibsrq,
			   struct ib_srq_attr *attr)
{
	return -ENOSYS;
}

static int ionic_destroy_srq(struct ib_srq *ibsrq)
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
		return rc;
	}

	ionic_dbg_rm_qp(qp);

	if (ib_srq_has_cq(qp->ibsrq.srq_type)) {
		xa_erase_irq(&dev->qp_tbl, qp->qpid);

		cq = to_ionic_cq(qp->ibsrq.ext.xrc.cq);
		spin_lock_irqsave(&cq->lock, irqflags);
		ionic_clean_cq(cq, qp->qpid);
		list_del(&qp->cq_flush_rq);
		spin_unlock_irqrestore(&cq->lock, irqflags);
	}

	ionic_qp_rq_destroy(dev, ctx, qp);
	ionic_put_srqid(dev, qp->qpid);
	kfree(qp);

	return 0;
}

static int ionic_post_srq_recv(struct ib_srq *ibsrq,
			       struct ib_recv_wr *wr,
			       struct ib_recv_wr **bad)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibsrq->device);
	struct ionic_ctx *ctx = to_ionic_ctx_uobj(ibsrq->uobject);
	struct ionic_cq *cq = NULL;
	struct ionic_qp *qp = to_ionic_srq(ibsrq);

	if (ibsrq->ext.xrc.cq)
		cq = to_ionic_cq(ibsrq->ext.xrc.cq);

	return ionic_post_recv_common(dev, ctx, cq, qp, wr, bad);
}
#endif /* IONIC_SRQ_XRC */

static int ionic_get_port_immutable(struct ib_device *ibdev, u8 port,
				    struct ib_port_immutable *attr)
{
	if (port != 1)
		return -EINVAL;

	attr->core_cap_flags = RDMA_CORE_PORT_IBA_ROCE_UDP_ENCAP;

	attr->pkey_tbl_len = IONIC_PKEY_TBL_LEN;
	attr->gid_tbl_len = IONIC_GID_TBL_LEN;
	attr->max_mad_size = IB_MGMT_MAD_SIZE;

	return 0;
}

#ifdef HAVE_GET_DEV_FW_STR_LEN
static void ionic_get_dev_fw_str(struct ib_device *ibdev, char *str,
				 size_t str_len)
#else
static void ionic_get_dev_fw_str(struct ib_device *ibdev, char *str)
#endif
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibdev);
#ifndef HAVE_GET_DEV_FW_STR_LEN
	size_t str_len = IB_FW_VERSION_NAME_MAX;
#endif

	strlcpy(str, dev->info->fw_version, str_len);
}

static void ionic_port_event(struct ionic_ibdev *dev, enum ib_event_type event)
{
	struct ib_event ev;

	ev.device = &dev->ibdev;
	ev.element.port_num = 1;
	ev.event = event;

	ib_dispatch_event(&ev);
}

static void ionic_cq_event(struct ionic_ibdev *dev, u32 cqid, u8 code)
{
	struct ib_event ibev;
	struct ionic_cq *cq;
	unsigned long irqflags;

	xa_lock_irqsave(&dev->cq_tbl, irqflags);
	cq = xa_load(&dev->cq_tbl, cqid);
	if (cq)
		ionic_cq_add_ref(cq);
	xa_unlock_irqrestore(&dev->cq_tbl, irqflags);

	if (!cq) {
		dev_dbg(&dev->ibdev.dev, "missing cqid %#x code %u\n",
			cqid, code);
		goto out;
	}

	ibev.device = &dev->ibdev;
	ibev.element.cq = &cq->ibcq;

	switch (code) {
	case IONIC_V1_EQE_CQ_NOTIFY:
		if (cq->ibcq.comp_handler)
			cq->ibcq.comp_handler(&cq->ibcq, cq->ibcq.cq_context);
		goto out;

	case IONIC_V1_EQE_CQ_ERR:
		ibev.event = IB_EVENT_CQ_ERR;
		break;

	default:
		dev_dbg(&dev->ibdev.dev, "unrecognized cqid %#x code %u\n",
			cqid, code);
		goto out;
	}

	if (cq->ibcq.event_handler)
		cq->ibcq.event_handler(&ibev, cq->ibcq.cq_context);

out:
	if (cq)
		ionic_cq_rem_ref(cq);
}

static void ionic_qp_event(struct ionic_ibdev *dev, u32 qpid, u8 code)
{
	struct ib_event ibev;
	struct ionic_qp *qp;
	unsigned long irqflags;

	xa_lock_irqsave(&dev->qp_tbl, irqflags);
	qp = xa_load(&dev->qp_tbl, qpid);
	if (qp)
		ionic_qp_add_ref(qp);
	xa_unlock_irqrestore(&dev->qp_tbl, irqflags);

	if (!qp) {
		dev_dbg(&dev->ibdev.dev, "missing qpid %#x code %u\n",
			qpid, code);
		goto out;
	}

	ibev.device = &dev->ibdev;

	if (qp->is_srq) {
#ifdef IONIC_SRQ_XRC
		ibev.element.srq = &qp->ibsrq;

		switch (code) {
		case IONIC_V1_EQE_SRQ_LEVEL:
			ibev.event = IB_EVENT_SRQ_LIMIT_REACHED;
			break;

		case IONIC_V1_EQE_QP_ERR:
			ibev.event = IB_EVENT_SRQ_ERR;
			break;

		default:
			dev_dbg(&dev->ibdev.dev,
				"unrecognized srqid %#x code %u\n",
				qpid, code);
			goto out;
		}

		if (qp->ibsrq.event_handler)
			qp->ibsrq.event_handler(&ibev, qp->ibsrq.srq_context);
#endif /* IONIC_SRQ_XRC */
	} else {
		ibev.element.qp = &qp->ibqp;

		switch (code) {
		case IONIC_V1_EQE_SQ_DRAIN:
			ibev.event = IB_EVENT_SQ_DRAINED;
			break;

		case IONIC_V1_EQE_QP_COMM_EST:
			ibev.event = IB_EVENT_COMM_EST;
			break;

		case IONIC_V1_EQE_QP_LAST_WQE:
			ibev.event = IB_EVENT_QP_LAST_WQE_REACHED;
			break;

		case IONIC_V1_EQE_QP_ERR:
			ibev.event = IB_EVENT_QP_FATAL;
			break;

		case IONIC_V1_EQE_QP_ERR_REQUEST:
			ibev.event = IB_EVENT_QP_REQ_ERR;
			break;

		case IONIC_V1_EQE_QP_ERR_ACCESS:
			ibev.event = IB_EVENT_QP_ACCESS_ERR;
			break;

		default:
			dev_dbg(&dev->ibdev.dev,
				"unrecognized qpid %#x code %u\n",
				qpid, code);
			goto out;
		}

		if (qp->ibqp.event_handler)
			qp->ibqp.event_handler(&ibev, qp->ibqp.qp_context);
	}

out:
	if (qp)
		ionic_qp_rem_ref(qp);
}

static bool ionic_next_eqe(struct ionic_eq *eq, struct ionic_v1_eqe *eqe)
{
	struct ionic_v1_eqe *qeqe;
	bool color;

	qeqe = ionic_queue_at_prod(&eq->q);
	color = ionic_v1_eqe_color(qeqe);

	/* cons is color for eq */
	if (eq->q.cons != color)
		return false;

	rmb();

	dev_dbg(&eq->dev->ibdev.dev, "poll eq prod %u\n", eq->q.prod);
	print_hex_dump_debug("eqe ", DUMP_PREFIX_OFFSET, 16, 1,
			     qeqe, BIT(eq->q.stride_log2), true);

	*eqe = *qeqe;

	return true;
}

static u16 ionic_poll_eq(struct ionic_eq *eq, u16 budget)
{
	struct ionic_ibdev *dev = eq->dev;
	struct ionic_v1_eqe eqe;
	u32 evt, qid;
	u8 type, code;
	u16 old_prod;
	u16 npolled = 0;

	old_prod = eq->q.prod;

	while (npolled < budget) {
		if (!ionic_next_eqe(eq, &eqe))
			break;

		ionic_queue_produce(&eq->q);

		/* cons is color for eq */
		eq->q.cons = ionic_color_wrap(eq->q.prod, eq->q.cons);

		++npolled;

		evt = ionic_v1_eqe_evt(&eqe);
		type = ionic_v1_eqe_evt_type(evt);
		code = ionic_v1_eqe_evt_code(evt);
		qid = ionic_v1_eqe_evt_qid(evt);

		switch (type) {
		case IONIC_V1_EQE_TYPE_CQ:
			ionic_cq_event(dev, qid, code);
			break;

		case IONIC_V1_EQE_TYPE_QP:
			ionic_qp_event(dev, qid, code);
			break;

		default:
			dev_dbg(&dev->ibdev.dev,
				"unrecognized event %#x type %u\n",
				evt, type);
		}
	}

	return npolled;
}

static void ionic_poll_eq_work(struct work_struct *work)
{
	struct ionic_eq *eq = container_of(work, struct ionic_eq, work);
	u32 npolled;

	if (unlikely(!eq->enable) || WARN_ON(eq->armed))
		return;

	npolled = ionic_poll_eq(eq, ionic_eq_work_budget);

	if (npolled) {
		ionic_intr_credits(eq->dev->intr_ctrl, eq->intr, npolled, 0);
		queue_work(ionic_evt_workq, &eq->work);
	} else {
		xchg(&eq->armed, true);
		ionic_intr_credits(eq->dev->intr_ctrl, eq->intr,
				   0, IONIC_INTR_CRED_UNMASK);
	}
}

static irqreturn_t ionic_poll_eq_isr(int irq, void *eqptr)
{
	struct ionic_eq *eq = eqptr;
	u32 npolled;
	bool was_armed;

	was_armed = xchg(&eq->armed, false);

	if (unlikely(!eq->enable) || !was_armed)
		return IRQ_HANDLED;

	npolled = ionic_poll_eq(eq, ionic_eq_isr_budget);

	ionic_intr_credits(eq->dev->intr_ctrl, eq->intr, npolled, 0);
	queue_work(ionic_evt_workq, &eq->work);

	return IRQ_HANDLED;
}

static int ionic_rdma_devcmd(struct ionic_ibdev *dev,
			     struct ionic_admin_ctx *admin)
{
	int rc;

	rc = ionic_api_adminq_post(dev->handle, admin);
	if (rc)
		goto err_cmd;

	wait_for_completion(&admin->work);

	rc = ionic_verbs_status_to_rc(admin->comp.comp.status);
err_cmd:
	return rc;
}

static int ionic_rdma_reset_devcmd(struct ionic_ibdev *dev)
{
	struct ionic_admin_ctx admin = {
		.work = COMPLETION_INITIALIZER_ONSTACK(admin.work),
		.cmd.rdma_reset = {
			.opcode = CMD_OPCODE_RDMA_RESET_LIF,
			.lif_index = cpu_to_le16(dev->lif_index),
		},
	};

	return ionic_rdma_devcmd(dev, &admin);
}

static int ionic_rdma_queue_devcmd(struct ionic_ibdev *dev,
				   struct ionic_queue *q,
				   u32 qid, u32 cid, u16 opcode)
{
	struct ionic_admin_ctx admin = {
		.work = COMPLETION_INITIALIZER_ONSTACK(admin.work),
		.cmd.rdma_queue = {
			.opcode = opcode,
			.lif_index = cpu_to_le16(dev->lif_index),
			.qid_ver = cpu_to_le32(qid),
			.cid = cpu_to_le32(cid),
			.dbid = cpu_to_le16(dev->dbid),
			.depth_log2 = q->depth_log2,
			.stride_log2 = q->stride_log2,
			.dma_addr = cpu_to_le64(q->dma),
		},
	};

	return ionic_rdma_devcmd(dev, &admin);
}

static struct ionic_eq *ionic_create_eq(struct ionic_ibdev *dev, int eqid)
{
	struct ionic_eq *eq;
	int rc;

	eq = kmalloc(sizeof(*eq), GFP_KERNEL);
	if (!eq) {
		rc = -ENOMEM;
		goto err_eq;
	}

	eq->dev = dev;

	rc = ionic_queue_init(&eq->q, dev->hwdev, ionic_eq_depth,
			      sizeof(struct ionic_v1_eqe));
	if (rc)
		goto err_q;

	eq->eqid = eqid;

	eq->armed = true;
	eq->enable = false;
	INIT_WORK(&eq->work, ionic_poll_eq_work);

	rc = ionic_api_get_intr(dev->handle, &eq->irq);
	if (rc < 0)
		goto err_intr;

	eq->intr = rc;

	ionic_queue_dbell_init(&eq->q, eq->eqid);

	/* cons is color for eq */
	eq->q.cons = true;

	snprintf(eq->name, sizeof(eq->name), "%s-%d-%d-eq",
		 DRIVER_SHORTNAME, dev->lif_index, eq->eqid);

	ionic_intr_mask(dev->intr_ctrl, eq->intr, IONIC_INTR_MASK_SET);
	ionic_intr_mask_assert(dev->intr_ctrl, eq->intr, IONIC_INTR_MASK_SET);
	ionic_intr_coal_init(dev->intr_ctrl, eq->intr, 0);
	ionic_intr_clean(dev->intr_ctrl, eq->intr);

	eq->enable = true;

	rc = request_irq(eq->irq, ionic_poll_eq_isr, 0, eq->name, eq);
	if (rc)
		goto err_irq;

	rc = ionic_rdma_queue_devcmd(dev, &eq->q, eq->eqid, eq->intr,
				     CMD_OPCODE_RDMA_CREATE_EQ);
	if (rc)
		goto err_cmd;

	ionic_intr_mask(dev->intr_ctrl, eq->intr, IONIC_INTR_MASK_CLEAR);

	ionic_dbg_add_eq(dev, eq);

	return eq;

err_cmd:
	eq->enable = false;
	flush_work(&eq->work);
	free_irq(eq->irq, eq);
err_irq:
	ionic_api_put_intr(dev->handle, eq->intr);
err_intr:
	ionic_queue_destroy(&eq->q, dev->hwdev);
err_q:
	kfree(eq);
err_eq:
	return ERR_PTR(rc);
}

static void ionic_destroy_eq(struct ionic_eq *eq)
{
	struct ionic_ibdev *dev = eq->dev;

	ionic_dbg_rm_eq(eq);

	eq->enable = false;
	flush_work(&eq->work);
	free_irq(eq->irq, eq);

	ionic_api_put_intr(dev->handle, eq->intr);
	ionic_queue_destroy(&eq->q, dev->hwdev);
	kfree(eq);
}

static void ionic_rdma_admincq_comp(struct ib_cq *ibcq, void *cq_context)
{
	struct ionic_aq *aq = cq_context;
	struct ionic_ibdev *dev = aq->dev;
	unsigned long irqflags;

	spin_lock_irqsave(&aq->lock, irqflags);
	aq->armed = false;
	if (dev->admin_state < IONIC_ADMIN_KILLED)
		queue_work(ionic_evt_workq, &aq->work);
	spin_unlock_irqrestore(&aq->lock, irqflags);
}

static void ionic_rdma_admincq_event(struct ib_event *event, void *cq_context)
{
	struct ionic_aq *aq = cq_context;
	struct ionic_ibdev *dev = aq->dev;

	dev_err(&dev->ibdev.dev, "admincq event %d\n", event->event);
}

static struct ionic_cq *ionic_create_rdma_admincq(struct ionic_ibdev *dev,
						  int comp_vector)
{
	struct ionic_cq *cq;
	struct ionic_tbl_buf buf = {0};
	struct ib_cq_init_attr attr = {
		.cqe = ionic_aq_depth,
		.comp_vector = comp_vector,
	};
	int rc;

	cq = kzalloc(sizeof(*cq), GFP_KERNEL);
	if (!cq) {
		rc = -ENOMEM;
		goto err_alloc;
	}

	cq->ibcq.device = &dev->ibdev;
	cq->ibcq.uobject = NULL;
	cq->ibcq.comp_handler = ionic_rdma_admincq_comp;
	cq->ibcq.event_handler = ionic_rdma_admincq_event;
	cq->ibcq.cq_context = NULL;
	atomic_set(&cq->ibcq.usecnt, 0);

	rc = __ionic_create_cq(cq, &buf, &attr, NULL, NULL);
	if (rc)
		goto err_init;

	rc = ionic_rdma_queue_devcmd(dev, &cq->q, cq->cqid, cq->eqid,
				     CMD_OPCODE_RDMA_CREATE_CQ);
	if (rc)
		goto err_cmd;

	return cq;

err_cmd:
	__ionic_destroy_cq(dev, cq);
err_init:
	kfree(cq);
err_alloc:
	return ERR_PTR(rc);
}

static struct ionic_aq *__ionic_create_rdma_adminq(struct ionic_ibdev *dev,
						   u32 aqid, u32 cqid)
{
	struct ionic_aq *aq;
	int rc;

	aq = kmalloc(sizeof(*aq), GFP_KERNEL);
	if (!aq) {
		rc = -ENOMEM;
		goto err_aq;
	}

	aq->dev = dev;

	aq->aqid = aqid;

	aq->cqid = cqid;

	spin_lock_init(&aq->lock);

	rc = ionic_queue_init(&aq->q, dev->hwdev, ionic_aq_depth,
			      sizeof(struct ionic_v1_admin_wqe));
	if (rc)
		goto err_q;

	ionic_queue_dbell_init(&aq->q, aq->aqid);

	aq->q_wr = kcalloc((u32)aq->q.mask + 1, sizeof(*aq->q_wr), GFP_KERNEL);
	if (!aq->q_wr) {
		rc = -ENOMEM;
		goto err_wr;
	}

	INIT_LIST_HEAD(&aq->wr_prod);
	INIT_LIST_HEAD(&aq->wr_post);

	INIT_WORK(&aq->work, ionic_admin_work);
	aq->armed = false;

	ionic_dbg_add_aq(dev, aq);

	return aq;

err_wr:
	ionic_queue_destroy(&aq->q, dev->hwdev);
err_q:
	kfree(aq);
err_aq:
	return ERR_PTR(rc);
}

static void __ionic_destroy_rdma_adminq(struct ionic_ibdev *dev,
					struct ionic_aq *aq)
{

	ionic_dbg_rm_aq(aq);

	ionic_queue_destroy(&aq->q, dev->hwdev);
	kfree(aq);
}

static struct ionic_aq *ionic_create_rdma_adminq(struct ionic_ibdev *dev,
						 u32 aqid, u32 cqid)
{
	struct ionic_aq *aq;
	int rc;

	aq = __ionic_create_rdma_adminq(dev, aqid, cqid);
	if (IS_ERR(aq)) {
		rc = PTR_ERR(aq);
		goto err_aq;
	}

	rc = ionic_rdma_queue_devcmd(dev, &aq->q, aq->aqid, aq->cqid,
				     CMD_OPCODE_RDMA_CREATE_ADMINQ);
	if (rc)
		goto err_cmd;

	return aq;

err_cmd:
	__ionic_destroy_rdma_adminq(dev, aq);
err_aq:
	return ERR_PTR(rc);
}

static void ionic_kill_ibdev(struct ionic_ibdev *dev, bool fatal_path)
{
	bool do_flush = false;
	unsigned long irqflags;
	int i;

	local_irq_save(irqflags);

	/* Mark the admin queue, flushing at most once */
	for (i = 0; i < dev->aq_count; i++)
		spin_lock(&dev->aq_vec[i]->lock);

	if (dev->admin_state != IONIC_ADMIN_KILLED) {
		dev->admin_state = IONIC_ADMIN_KILLED;
		do_flush = true;
	}

	for (i = dev->aq_count; i > 0;) {
		--i;
		/* Flush incomplete admin commands */
		if (do_flush)
			ionic_admin_poll_locked(dev->aq_vec[i]);
		spin_unlock(&dev->aq_vec[i]->lock);
	}

	if (do_flush) {
		struct xa_iter iter;
		void **slot;

		/* Flush qp send and recv */
		xa_lock(&dev->qp_tbl);
		xa_for_each_slot(&dev->qp_tbl, slot, &iter)
			ionic_flush_qp(*slot);
		xa_unlock(&dev->qp_tbl);

		/* Notify completions */
		xa_lock(&dev->cq_tbl);
		xa_for_each_slot(&dev->cq_tbl, slot, &iter)
			ionic_notify_flush_cq(*slot);
		xa_unlock(&dev->cq_tbl);
	}

	local_irq_restore(irqflags);

	/* Post a fatal event if requested */
	if (fatal_path)
		ionic_port_event(dev, IB_EVENT_DEVICE_FATAL);
}

static void ionic_kill_ibdev_cb(void *dev_ptr)
{
	struct ionic_ibdev *dev = dev_ptr;

	dev_warn(&dev->ibdev.dev, "reset has been indicated\n");

	ionic_kill_ibdev(dev, true);
}

static void ionic_kill_rdma_admin(struct ionic_ibdev *dev, bool fatal_path)
{
	unsigned long irqflags = 0;
	bool do_reset = false;
	int i, rc;

	if (!dev->aq_vec)
		return;

	local_irq_save(irqflags);
	for (i = 0; i < dev->aq_count; i++)
		spin_lock(&dev->aq_vec[i]->lock);

	/* pause rdma admin queues to reset device */
	if (dev->admin_state == IONIC_ADMIN_ACTIVE) {
		dev->admin_state = IONIC_ADMIN_PAUSED;
		do_reset = true;
	}

	while (i-- > 0)
		spin_unlock(&dev->aq_vec[i]->lock);
	local_irq_restore(irqflags);

	if (!do_reset)
		return;

	/* After resetting the device, it will be safe to resume the rdma admin
	 * queues in the killed state.  Commands will not be issued to the
	 * device, but will complete locally with status IONIC_ADMIN_KILLED.
	 * Handling completion will ensure that creating or modifying resources
	 * fails, but destroying resources succeeds.
	 *
	 * If there was a failure resetting the device using this strategy,
	 * then the state of the device is unknown.  The rdma admin queue is
	 * left here in the paused state.  No new commands are issued to the
	 * device, nor are any completed locally.  The eth driver will use a
	 * different strategy to reset the device.  A callback from the eth
	 * driver will indicate that the reset is done and it is safe to
	 * continue.  Then, the rdma admin queue will be transitioned to the
	 * killed state and new and outstanding commands will complete locally.
	 */

	rc = ionic_rdma_reset_devcmd(dev);
	if (unlikely(rc)) {
		dev_err(&dev->ibdev.dev, "failed to reset rdma %d\n", rc);
		ionic_api_request_reset(dev->handle);
	} else {
		ionic_kill_ibdev(dev, fatal_path);
	}
}

static void ionic_reset_work(struct work_struct *ws)
{
	struct ionic_ibdev *dev =
		container_of(ws, struct ionic_ibdev, reset_work);

	ionic_kill_rdma_admin(dev, true);
}

static int ionic_create_rdma_admin(struct ionic_ibdev *dev)
{
	struct ionic_aq *aq;
	struct ionic_cq *cq;
	struct ionic_eq *eq;
	int eq_i = 0, aq_i = 0;
	int rc = 0;

	dev->eq_vec = NULL;
	dev->aq_vec = NULL;

	INIT_WORK(&dev->reset_work, ionic_reset_work);
	INIT_DELAYED_WORK(&dev->admin_dwork, ionic_admin_dwork);
	dev->admin_state = IONIC_ADMIN_KILLED;

	INIT_LIST_HEAD(&dev->qp_list);
	INIT_LIST_HEAD(&dev->cq_list);
	spin_lock_init(&dev->dev_lock);

	if (ionic_aq_count > 0 && ionic_aq_count < dev->aq_count) {
		dev_dbg(&dev->ibdev.dev, "limiting adminq count to %d\n",
			ionic_aq_count);
		dev->aq_count = ionic_aq_count;
	}

	/* need at least two eq and one aq */
	if (dev->eq_count < 2 || !dev->aq_count) {
		rc = -EINVAL;
		goto out;
	}

	dev->eq_vec = kmalloc_array(dev->eq_count, sizeof(*dev->eq_vec),
				    GFP_KERNEL);
	if (!dev->eq_vec) {
		rc = -ENOMEM;
		goto out;
	}

	for (; eq_i < dev->eq_count; ++eq_i) {
		eq = ionic_create_eq(dev, eq_i + dev->eq_base);
		if (IS_ERR(eq)) {
			rc = PTR_ERR(eq);

			/* need at least two eq */
			if (eq_i < 2) {
				dev_err(dev->hwdev, "fail create eq %d\n", rc);
				goto out;
			}

			/* ok, just fewer eq than device supports */
			dev_dbg(dev->hwdev, "eq count %d want %d rc %d\n",
				eq_i, dev->eq_count, rc);

			rc = 0;
			break;
		}

		dev->eq_vec[eq_i] = eq;
	}

	dev->eq_count = eq_i;

	dev->aq_vec = kmalloc_array(dev->aq_count, sizeof(*dev->aq_vec),
				      GFP_KERNEL);
	if (!dev->aq_vec) {
		rc = -ENOMEM;
		goto out;
	}

	/* Create one CQ per AQ */
	for (; aq_i < dev->aq_count; ++aq_i) {
		cq = ionic_create_rdma_admincq(dev, aq_i % eq_i);
		if (IS_ERR(cq)) {
			rc = PTR_ERR(cq);

			if (!aq_i) {
				dev_err(dev->hwdev,
					"fail create acq %d\n", rc);
				goto out;
			}

			/* ok, just fewer adminq than device supports */
			dev_dbg(dev->hwdev, "acq count %d want %d rc %d\n",
				aq_i, dev->aq_count, rc);
			break;
		}

		aq = ionic_create_rdma_adminq(dev, aq_i + dev->aq_base,
					      cq->cqid);
		if (IS_ERR(aq)) {
			/* Clean up the dangling CQ */
			__ionic_destroy_cq(dev, cq);
			kfree(cq);

			rc = PTR_ERR(aq);

			if (!aq_i) {
				dev_err(dev->hwdev,
					"fail create aq %d\n", rc);
				goto out;
			}

			/* ok, just fewer adminq than device supports */
			dev_dbg(dev->hwdev, "aq count %d want %d rc %d\n",
				aq_i, dev->aq_count, rc);
			break;
		}

		cq->ibcq.cq_context = aq;
		aq->cq = cq;

		dev->aq_vec[aq_i] = aq;
	}

	dev->admin_state = IONIC_ADMIN_ACTIVE;
out:
	dev->eq_count = eq_i;
	dev->aq_count = aq_i;

	return rc;
}

static void ionic_destroy_rdma_admin(struct ionic_ibdev *dev)
{
	struct ionic_aq *aq;
	struct ionic_cq *cq;
	struct ionic_eq *eq;

	cancel_delayed_work_sync(&dev->admin_dwork);
	cancel_work_sync(&dev->reset_work);

	if (dev->aq_vec) {
		while (dev->aq_count > 0) {
			aq = dev->aq_vec[--dev->aq_count];
			cq = aq->cq;

			cancel_work_sync(&aq->work);

			__ionic_destroy_rdma_adminq(dev, aq);
			if (cq) {
				__ionic_destroy_cq(dev, cq);
				kfree(cq);
			}
		}

		kfree(dev->aq_vec);
	}

	if (dev->eq_vec) {
		while (dev->eq_count > 0) {
			eq = dev->eq_vec[--dev->eq_count];
			ionic_destroy_eq(eq);
		}

		kfree(dev->eq_vec);
	}
}

static void ionic_destroy_ibdev(struct ionic_ibdev *dev)
{
	struct net_device *ndev = dev->ndev;
	int i;

	list_del(&dev->driver_ent);

	ionic_kill_rdma_admin(dev, false);

	ionic_dcqcn_destroy(dev);

	for (i = 0; i < ARRAY_SIZE(ionic_dev_attributes); i++)
		device_remove_file(&dev->ibdev.dev,
				   ionic_dev_attributes[i]);
	ib_unregister_device(&dev->ibdev);

	ionic_destroy_rdma_admin(dev);

	ionic_api_clear_private(dev->handle);

	contig_kfree(dev->stats, PAGE_SIZE);
	contig_kfree(dev->stats_buf, PAGE_SIZE);
	kfree(dev->stats_hdrs);

	ionic_dbg_rm_dev(dev);

	ionic_resid_destroy(&dev->inuse_qpid);
	ionic_resid_destroy(&dev->inuse_cqid);
	ionic_resid_destroy(&dev->inuse_mrid);
	ionic_resid_destroy(&dev->inuse_ahid);
	ionic_resid_destroy(&dev->inuse_pdid);
	ionic_buddy_destroy(&dev->inuse_restbl);
	xa_destroy(&dev->qp_tbl);
	xa_destroy(&dev->cq_tbl);

	ib_dealloc_device(&dev->ibdev);

	dev_put(ndev);
}

static struct ionic_ibdev *ionic_create_ibdev(void *handle,
					      struct net_device *ndev)
{
	struct ib_device *ibdev;
	struct ionic_ibdev *dev;
	struct device *hwdev;
	const union lif_identity *ident;
	struct sysctl_oid *dbg_ctx;
	int rc, val, lif_index, version;
	int i;

	dev_hold(ndev);

	ident = ionic_api_get_identity(handle, &lif_index);

	netdev_dbg(ndev, "rdma.version %d\n",
		ident->rdma.version);
	netdev_dbg(ndev, "rdma.qp_opcodes %d\n",
		ident->rdma.qp_opcodes);
	netdev_dbg(ndev, "rdma.admin_opcodes %d\n",
		ident->rdma.admin_opcodes);
	netdev_dbg(ndev, "rdma.npts_per_lif %d\n",
		ident->rdma.npts_per_lif);
	netdev_dbg(ndev, "rdma.nmrs_per_lif %d\n",
		ident->rdma.nmrs_per_lif);
	netdev_dbg(ndev, "rdma.nahs_per_lif %d\n",
		ident->rdma.nahs_per_lif);
	netdev_dbg(ndev, "rdma.aq.qtype %d rdma.aq.base %d rdma.aq.count %d\n",
		ident->rdma.aq_qtype.qtype,
		ident->rdma.aq_qtype.qid_base, ident->rdma.aq_qtype.qid_count);
	netdev_dbg(ndev, "rdma.sq.qtype %d rdma.sq.base %d rdma.sq.count %d\n",
		ident->rdma.sq_qtype.qtype,
		ident->rdma.sq_qtype.qid_base, ident->rdma.sq_qtype.qid_count);
	netdev_dbg(ndev, "rdma.rq.qtype %d rdma.rq.base %d rdma.rq.count %d\n",
		ident->rdma.rq_qtype.qtype,
		ident->rdma.rq_qtype.qid_base, ident->rdma.rq_qtype.qid_count);
	netdev_dbg(ndev, "rdma.cq.qtype %d rdma.cq.base %d rdma.cq.count %d\n",
		ident->rdma.cq_qtype.qtype,
		ident->rdma.cq_qtype.qid_base, ident->rdma.cq_qtype.qid_count);
	netdev_dbg(ndev, "rdma.eq.qtype %d rdma.eq.base %d rdma.eq.count %d\n",
		ident->rdma.eq_qtype.qtype,
		ident->rdma.eq_qtype.qid_base, ident->rdma.eq_qtype.qid_count);

	version = ident->rdma.version;

	if (version < IONIC_MIN_RDMA_VERSION) {
		netdev_err(ndev,
			   "ionic_rdma: Firmware RDMA Version %u\n",
			   version);
		netdev_err(ndev,
			   "ionic_rdma: Driver Min RDMA Version %u\n",
			   IONIC_MIN_RDMA_VERSION);
		rc = -EINVAL;
		goto err_dev;
	}

	if (version > IONIC_MAX_RDMA_VERSION) {
		netdev_err(ndev,
			   "ionic_rdma: Firmware RDMA Version %u\n",
			   version);
		netdev_err(ndev,
			   "ionic_rdma: Driver Max RDMA Version %u\n",
			   IONIC_MAX_RDMA_VERSION);
		rc = -EINVAL;
		goto err_dev;
	}

	/* Ensure that our parent is a true PCI device */
	hwdev = ionic_api_get_device(handle);
	if (!dev_is_pci(hwdev)) {
		netdev_err(ndev,
			   "ionic_rdma: Cannot bind to non-PCI device\n");
		rc = -ENXIO;
		goto err_dev;
	}

	ibdev = ib_alloc_device(sizeof(*dev));
	if (!ibdev) {
		rc = -ENOMEM;
		goto err_dev;
	}
	dev = to_ionic_ibdev(ibdev);

	dev->hwdev = hwdev;
	dev->ndev = ndev;
	dev->handle = handle;
	dev->lif_index = lif_index;
	dev->ident = ident;
	dev->info = ionic_api_get_devinfo(handle);

	ionic_api_kernel_dbpage(handle, &dev->intr_ctrl,
				&dev->dbid, &dev->dbpage);

	dev->rdma_version = version;
	dev->qp_opcodes = ident->rdma.qp_opcodes;
	dev->admin_opcodes = ident->rdma.admin_opcodes;

	/* base opcodes must be supported, extended opcodes are optional*/
	if (dev->qp_opcodes <= IONIC_V1_OP_BIND_MW) {
		netdev_dbg(ndev, "ionic_rdma: qp opcodes %d want min %d\n",
			   dev->qp_opcodes, IONIC_V1_OP_BIND_MW + 1);
		rc = -ENODEV;
		goto err_dev;
	}

	/* need at least one rdma admin queue (driver creates one) */
	val = le32_to_cpu(ident->rdma.aq_qtype.qid_count);
	if (!val) {
		netdev_dbg(ndev, "ionic_rdma: No RDMA Admin Queue\n");
		rc = -ENODEV;
		goto err_dev;
	}

	/* qp ids start at zero, and sq id == qp id */
	val = le32_to_cpu(ident->rdma.sq_qtype.qid_base);
	if (val) {
		netdev_dbg(ndev, "ionic_rdma: Nonzero sq qid base %u\n", val);
		rc = -EINVAL;
		goto err_dev;
	}

	/* qp ids start at zero, and rq id == qp id */
	val = le32_to_cpu(ident->rdma.rq_qtype.qid_base);
	if (val) {
		netdev_dbg(ndev, "ionic_rdma: Nonzero rq qid base %u\n", val);
		rc = -EINVAL;
		goto err_dev;
	}

	/* driver supports these qtypes starting at nonzero base */
	dev->aq_base = le32_to_cpu(ident->rdma.aq_qtype.qid_base);
	dev->cq_base = le32_to_cpu(ident->rdma.cq_qtype.qid_base);
	dev->eq_base = le32_to_cpu(ident->rdma.eq_qtype.qid_base);

	/*
	 * ionic_create_rdma_admin() may reduce aq_count or eq_count if
	 * it is unable to allocate all that were requested.
	 * aq_count is tunable; see ionic_rdma_aq_count
	 */
	dev->aq_count = le32_to_cpu(ident->rdma.aq_qtype.qid_count);
	dev->eq_count = le32_to_cpu(ident->rdma.eq_qtype.qid_count);

	dev->aq_qtype = ident->rdma.aq_qtype.qtype;
	dev->sq_qtype = ident->rdma.sq_qtype.qtype;
	dev->rq_qtype = ident->rdma.rq_qtype.qtype;
	dev->cq_qtype = ident->rdma.cq_qtype.qtype;
	dev->eq_qtype = ident->rdma.eq_qtype.qtype;

	dev->max_stride = ident->rdma.max_stride;
	dev->cl_stride = ident->rdma.cl_stride;
	dev->pte_stride = ident->rdma.pte_stride;
	dev->rrq_stride = ident->rdma.rrq_stride;
	dev->rsq_stride = ident->rdma.rsq_stride;

	xa_init(&dev->qp_tbl);
	xa_init(&dev->cq_tbl);

	mutex_init(&dev->inuse_lock);
	spin_lock_init(&dev->inuse_splock);

	rc = ionic_buddy_init(&dev->inuse_restbl,
			      le32_to_cpu(ident->rdma.npts_per_lif) >>
			      (dev->cl_stride - dev->pte_stride));
	if (rc)
		goto err_restbl;

	rc = ionic_resid_init(&dev->inuse_pdid, ionic_max_pd);
	if (rc)
		goto err_pdid;

	rc = ionic_resid_init(&dev->inuse_ahid,
			      le32_to_cpu(ident->rdma.nahs_per_lif));
	if (rc)
		goto err_ahid;

	rc = ionic_resid_init(&dev->inuse_mrid,
			      le32_to_cpu(ident->rdma.nmrs_per_lif));
	if (rc)
		goto err_mrid;

	/* skip reserved lkey */
	dev->inuse_mrid.next_id = 1;
	dev->next_mrkey = 1;

	rc = ionic_resid_init(&dev->inuse_cqid,
			      le32_to_cpu(ident->rdma.cq_qtype.qid_count));
	if (rc)
		goto err_cqid;

	/* prefer srqids after qpids */
	dev->size_qpid = le32_to_cpu(ident->rdma.sq_qtype.qid_count);
	dev->size_srqid = le32_to_cpu(ident->rdma.rq_qtype.qid_count);
	dev->next_srqid = dev->size_qpid;

	rc = ionic_resid_init(&dev->inuse_qpid, max(dev->size_qpid,
						    dev->size_srqid));
	if (rc)
		goto err_qpid;

	if (ionic_qid_skip > 0) {
		ionic_resid_skip(&dev->inuse_qpid);
		ionic_resid_skip(&dev->inuse_cqid);
	}

	/* skip reserved SMI and GSI qpids */
	dev->inuse_qpid.next_id = 2;

	if (ionic_dbg_enable)
		dbg_ctx = ionic_api_get_debug_ctx(handle);
	else
		dbg_ctx = NULL;

	ionic_dbg_add_dev(dev, dbg_ctx);

	rc = ionic_rdma_reset_devcmd(dev);
	if (rc)
		goto err_reset;

	rc = ionic_create_rdma_admin(dev);
	if (rc)
		goto err_register;

	ionic_dbg_add_dev_info(dev);

	ibdev->owner = THIS_MODULE;
	ibdev->dev.parent = dev->hwdev;

	strlcpy(ibdev->name, "ionic_%d", IB_DEVICE_NAME_MAX);
	strlcpy(ibdev->node_desc, DEVICE_DESCRIPTION, IB_DEVICE_NODE_DESC_MAX);

	ibdev->node_type = RDMA_NODE_IB_CA;
	ibdev->phys_port_cnt = 1;

	/* the first eq is reserved for async events */
	ibdev->num_comp_vectors = dev->eq_count - 1;

	addrconf_ifid_eui48((u8 *)&ibdev->node_guid, ndev);

	ibdev->uverbs_abi_ver = IONIC_ABI_VERSION;
	ibdev->uverbs_cmd_mask =
		BIT_ULL(IB_USER_VERBS_CMD_GET_CONTEXT)		|
		BIT_ULL(IB_USER_VERBS_CMD_QUERY_DEVICE)		|
		BIT_ULL(IB_USER_VERBS_CMD_QUERY_PORT)		|
		BIT_ULL(IB_USER_VERBS_CMD_ALLOC_PD)		|
		BIT_ULL(IB_USER_VERBS_CMD_DEALLOC_PD)		|
		BIT_ULL(IB_USER_VERBS_CMD_CREATE_AH)		|
		BIT_ULL(IB_USER_VERBS_CMD_QUERY_AH)		|
		BIT_ULL(IB_USER_VERBS_CMD_DESTROY_AH)		|
		BIT_ULL(IB_USER_VERBS_CMD_REG_MR)		|
		BIT_ULL(IB_USER_VERBS_CMD_REG_SMR)		|
		BIT_ULL(IB_USER_VERBS_CMD_REREG_MR)		|
		BIT_ULL(IB_USER_VERBS_CMD_DEREG_MR)		|
		BIT_ULL(IB_USER_VERBS_CMD_ALLOC_MW)		|
		BIT_ULL(IB_USER_VERBS_CMD_BIND_MW)		|
		BIT_ULL(IB_USER_VERBS_CMD_DEALLOC_MW)		|
		BIT_ULL(IB_USER_VERBS_CMD_CREATE_COMP_CHANNEL)	|
		BIT_ULL(IB_USER_VERBS_CMD_CREATE_CQ)		|
		BIT_ULL(IB_USER_VERBS_CMD_DESTROY_CQ)		|
		BIT_ULL(IB_USER_VERBS_CMD_POLL_CQ)		|
		BIT_ULL(IB_USER_VERBS_CMD_PEEK_CQ)		|
		BIT_ULL(IB_USER_VERBS_CMD_REQ_NOTIFY_CQ)	|
		BIT_ULL(IB_USER_VERBS_CMD_CREATE_QP)		|
		BIT_ULL(IB_USER_VERBS_CMD_QUERY_QP)		|
		BIT_ULL(IB_USER_VERBS_CMD_MODIFY_QP)		|
		BIT_ULL(IB_USER_VERBS_CMD_DESTROY_QP)		|
		BIT_ULL(IB_USER_VERBS_CMD_POST_SEND)		|
		BIT_ULL(IB_USER_VERBS_CMD_POST_RECV)		|
#ifdef IONIC_SRQ_XRC
		BIT_ULL(IB_USER_VERBS_CMD_CREATE_SRQ)		|
		BIT_ULL(IB_USER_VERBS_CMD_MODIFY_SRQ)		|
		BIT_ULL(IB_USER_VERBS_CMD_QUERY_SRQ)		|
		BIT_ULL(IB_USER_VERBS_CMD_DESTROY_SRQ)		|
		BIT_ULL(IB_USER_VERBS_CMD_POST_SRQ_RECV)	|
		BIT_ULL(IB_USER_VERBS_CMD_OPEN_XRCD)		|
		BIT_ULL(IB_USER_VERBS_CMD_CLOSE_XRCD)		|
		BIT_ULL(IB_USER_VERBS_CMD_CREATE_XSRQ)		|
		BIT_ULL(IB_USER_VERBS_CMD_OPEN_QP)		|
#endif /* IONIC_SRQ_XRC */
		0;
	ibdev->uverbs_ex_cmd_mask =
		BIT_ULL(IB_USER_VERBS_EX_CMD_CREATE_QP)		|
#ifdef HAVE_EX_CMD_MODIFY_QP
		BIT_ULL(IB_USER_VERBS_EX_CMD_MODIFY_QP)		|
#endif
		0;

	dev->ibdev.alloc_hw_stats	= ionic_alloc_hw_stats;
	dev->ibdev.get_hw_stats		= ionic_get_hw_stats;

	dev->ibdev.query_device		= ionic_query_device;
	dev->ibdev.query_port		= ionic_query_port;
	dev->ibdev.get_link_layer	= ionic_get_link_layer;
	dev->ibdev.get_netdev		= ionic_get_netdev;
	dev->ibdev.query_gid		= ionic_query_gid;
	dev->ibdev.add_gid		= ionic_add_gid;
	dev->ibdev.del_gid		= ionic_del_gid;
	dev->ibdev.query_pkey		= ionic_query_pkey;
	dev->ibdev.modify_device	= ionic_modify_device;
	dev->ibdev.modify_port		= ionic_modify_port;

	dev->ibdev.alloc_ucontext	= ionic_alloc_ucontext;
	dev->ibdev.dealloc_ucontext	= ionic_dealloc_ucontext;
	dev->ibdev.mmap			= ionic_mmap;

	dev->ibdev.alloc_pd		= ionic_alloc_pd;
	dev->ibdev.dealloc_pd		= ionic_dealloc_pd;

	dev->ibdev.create_ah		= ionic_create_ah;
	dev->ibdev.query_ah		= ionic_query_ah;
	dev->ibdev.destroy_ah		= ionic_destroy_ah;

	dev->ibdev.get_dma_mr		= ionic_get_dma_mr;
	dev->ibdev.reg_user_mr		= ionic_reg_user_mr;
	dev->ibdev.rereg_user_mr	= ionic_rereg_user_mr;
	dev->ibdev.dereg_mr		= ionic_dereg_mr;
	dev->ibdev.alloc_mr		= ionic_alloc_mr;
	dev->ibdev.map_mr_sg		= ionic_map_mr_sg;

	dev->ibdev.alloc_mw		= ionic_alloc_mw;
	dev->ibdev.dealloc_mw		= ionic_dealloc_mw;

	dev->ibdev.create_cq		= ionic_create_cq;
	dev->ibdev.destroy_cq		= ionic_destroy_cq;
	dev->ibdev.poll_cq		= ionic_poll_cq;
	dev->ibdev.req_notify_cq	= ionic_req_notify_cq;

	dev->ibdev.create_qp		= ionic_create_qp;
	dev->ibdev.modify_qp		= ionic_modify_qp;
	dev->ibdev.query_qp		= ionic_query_qp;
	dev->ibdev.destroy_qp		= ionic_destroy_qp;
	dev->ibdev.post_send		= ionic_post_send;
	dev->ibdev.post_recv		= ionic_post_recv;

#ifdef IONIC_SRQ_XRC
	dev->ibdev.create_srq		= ionic_create_srq;
	dev->ibdev.modify_srq		= ionic_modify_srq;
	dev->ibdev.query_srq		= ionic_query_srq;
	dev->ibdev.destroy_srq		= ionic_destroy_srq;
	dev->ibdev.post_srq_recv	= ionic_post_srq_recv;
#endif /* IONIC_SRQ_XRC */

	dev->ibdev.get_port_immutable	= ionic_get_port_immutable;
	dev->ibdev.get_dev_fw_str	= ionic_get_dev_fw_str;

	ibdev->dma_device = ibdev->dev.parent;

	rc = ib_register_device(ibdev, NULL);
	if (rc)
		goto err_register;

	rc = ionic_api_set_private(handle, dev, ionic_kill_ibdev_cb,
				   IONIC_PRSN_RDMA);
	if (rc)
		goto err_api;

	for (i = 0; i < ARRAY_SIZE(ionic_dev_attributes); i++) {
		rc = device_create_file(&dev->ibdev.dev,
					ionic_dev_attributes[i]);
		if (rc)
			goto err_attrib;
	}

	ionic_dcqcn_init(dev, ident->rdma.dcqcn_profiles);

	list_add(&dev->driver_ent, &ionic_ibdev_list);

	return dev;

err_attrib:
	while (i-- > 0)
		device_remove_file(&dev->ibdev.dev, ionic_dev_attributes[i]);
	ionic_api_clear_private(handle);
err_api:
	ib_unregister_device(&dev->ibdev);
err_register:
	ionic_kill_rdma_admin(dev, false);
	ionic_destroy_rdma_admin(dev);
	contig_kfree(dev->stats, PAGE_SIZE);
	contig_kfree(dev->stats_buf, PAGE_SIZE);
	kfree(dev->stats_hdrs);
err_reset:
	ionic_dbg_rm_dev(dev);
	ionic_resid_destroy(&dev->inuse_qpid);
err_qpid:
	ionic_resid_destroy(&dev->inuse_cqid);
err_cqid:
	ionic_resid_destroy(&dev->inuse_mrid);
err_mrid:
	ionic_resid_destroy(&dev->inuse_ahid);
err_ahid:
	ionic_resid_destroy(&dev->inuse_pdid);
err_pdid:
	ionic_buddy_destroy(&dev->inuse_restbl);
err_restbl:
	xa_destroy(&dev->qp_tbl);
	xa_destroy(&dev->cq_tbl);
	ib_dealloc_device(ibdev);
err_dev:
	dev_put(ndev);
	return ERR_PTR(rc);
}

struct ionic_netdev_work {
	struct work_struct ws;
	unsigned long event;
	struct net_device *ndev;
	void *handle;
	u32 reset_cnt;
};

static void ionic_netdev_work(struct work_struct *ws)
{
	struct ionic_netdev_work *work =
		container_of(ws, struct ionic_netdev_work, ws);
	struct net_device *ndev = work->ndev;
	struct ionic_ibdev *dev;

	dev = ionic_api_get_private(work->handle, IONIC_PRSN_RDMA);

	switch (work->event) {
	case NETDEV_REGISTER:
		if (dev) {
			netdev_dbg(ndev, "already registered\n");
			break;
		}

		netdev_dbg(ndev, "register ibdev\n");

		dev = ionic_create_ibdev(work->handle, ndev);
		if (IS_ERR(dev)) {
			netdev_dbg(ndev, "error register ibdev %d\n",
				   (int)PTR_ERR(dev));
			break;
		}

		dev->reset_cnt = work->reset_cnt;
		dev_info(&dev->ibdev.dev, "registered\n");
		break;

	case NETDEV_UNREGISTER:
		if (!dev) {
			netdev_dbg(ndev, "not registered\n");
			break;
		}

		/* Avoid spurious unregisters caused by ethernet LIF reset */
		if (!dev->reset_posted &&
		    ionic_api_stay_registered(work->handle)) {
			netdev_dbg(ndev, "stay registered\n");
			break;
		}

		netdev_dbg(ndev, "unregister ibdev\n");
		ionic_destroy_ibdev(dev);

		netdev_dbg(ndev, "unregistered\n");
		break;

	case NETDEV_UP:
		if (!dev)
			break;

		ionic_port_event(dev, IB_EVENT_PORT_ACTIVE);

		break;

	case NETDEV_DOWN:
		if (!dev)
			break;

		ionic_port_event(dev, IB_EVENT_PORT_ERR);

		break;

	case NETDEV_CHANGE:
		if (!dev)
			break;

		if (netif_running(ndev) && netif_carrier_ok(ndev))
			ionic_port_event(dev, IB_EVENT_PORT_ACTIVE);
		else
			ionic_port_event(dev, IB_EVENT_PORT_ERR);

		break;

	default:
		if (!dev)
			break;

		netdev_dbg(ndev, "unhandled event %lu\n", work->event);
	}

	dev_put(ndev);
	kfree(work);
}

static int ionic_netdev_event_post(struct net_device *ndev,
				   unsigned long event, u32 reset_cnt)
{
	struct ionic_netdev_work *work;
	void *handle;

	handle = get_netdev_ionic_handle(ndev, IONIC_API_VERSION,
					 IONIC_PRSN_RDMA);
	if (!handle) {
		pr_devel("unrecognized netdev: %s\n", netdev_name(ndev));
		return 0;
	}

	pr_devel("ionic netdev: %s\n", netdev_name(ndev));
	netdev_dbg(ndev, "event %lu\n", event);

	work = kmalloc(sizeof(*work), GFP_ATOMIC);
	if (WARN_ON_ONCE(!work))
		return -ENOMEM;

	dev_hold(ndev);

	INIT_WORK(&work->ws, ionic_netdev_work);
	work->event = event;
	work->ndev = ndev;
	work->handle = handle;

	/* Preserve reset_cnt in work struct since ibdev will be freed */
	work->reset_cnt = reset_cnt;

	queue_work(ionic_dev_workq, &work->ws);

	return 0;
}

static int ionic_netdev_event(struct notifier_block *notifier,
			      unsigned long event, void *ptr)
{
	struct net_device *ndev;
	int rc;

	ndev = netdev_notifier_info_to_dev(ptr);

	rc = ionic_netdev_event_post(ndev, event, 0);

	return rc ? notifier_from_errno(rc) : NOTIFY_DONE;
}

static struct notifier_block ionic_netdev_notifier = {
	.notifier_call = ionic_netdev_event,
};

static void ionic_netdev_discover(void)
{
	VNET_ITERATOR_DECL(vnet);
	struct net_device *ndev;

	VNET_LIST_RLOCK();
	VNET_FOREACH(vnet) {
		IFNET_RLOCK();
		CURVNET_SET_QUIET(vnet);
#if __FreeBSD_version >= 1200000
		CK_STAILQ_FOREACH(ndev, &V_ifnet, if_link)
#else
		TAILQ_FOREACH(ndev, &V_ifnet, if_link)
#endif
			ionic_netdev_event(&ionic_netdev_notifier,
					   NETDEV_REGISTER, ndev);
		CURVNET_RESTORE();
		IFNET_RUNLOCK();
	}
	VNET_LIST_RUNLOCK();
}

void ionic_ibdev_reset(struct ionic_ibdev *dev)
{
	int rc;

	dev->reset_posted = true;
	dev->reset_cnt++;

	rc = ionic_netdev_event_post(dev->ndev, NETDEV_UNREGISTER, 0);
	if (rc) {
		dev_warn(&dev->ibdev.dev,
			 "failed to post unregister event: %d\n", rc);
		return;
	}

	rc = ionic_netdev_event_post(dev->ndev, NETDEV_REGISTER,
				     dev->reset_cnt);
	if (rc)
		dev_warn(&dev->ibdev.dev,
			 "failed to post register event: %d\n", rc);
}

static int __init ionic_mod_init(void)
{
	int rc;

	pr_info("%s ver %s : %s\n",
		DRIVER_NAME, DRIVER_VERSION, DRIVER_DESCRIPTION);

	ionic_dev_workq = create_singlethread_workqueue(DRIVER_NAME "-dev");
	if (!ionic_dev_workq) {
		rc = -ENOMEM;
		goto err_dev_workq;
	}

	ionic_evt_workq = create_workqueue(DRIVER_NAME "-evt");
	if (!ionic_evt_workq) {
		rc = -ENOMEM;
		goto err_evt_workq;
	}

	rc = register_netdevice_notifier(&ionic_netdev_notifier);
	if (rc)
		goto err_notifier;

	ionic_netdev_discover();

	return 0;

err_notifier:
	destroy_workqueue(ionic_evt_workq);
err_evt_workq:
	destroy_workqueue(ionic_dev_workq);
err_dev_workq:
	return rc;
}

static void __exit ionic_exit_work(struct work_struct *ws)
{
	struct ionic_ibdev *dev, *dev_next;

	list_for_each_entry_safe_reverse(dev, dev_next, &ionic_ibdev_list,
					 driver_ent) {
		ionic_destroy_ibdev(dev);
	}
}

static void __exit ionic_mod_exit(void)
{
	struct work_struct ws;

	unregister_netdevice_notifier(&ionic_netdev_notifier);

	INIT_WORK_ONSTACK(&ws, ionic_exit_work);
	queue_work(ionic_dev_workq, &ws);
	flush_work(&ws);
	destroy_work_on_stack(&ws);

	destroy_workqueue(ionic_evt_workq);
	destroy_workqueue(ionic_dev_workq);

	BUILD_BUG_ON(sizeof(struct ionic_v1_cqe) != 32);
	BUILD_BUG_ON(sizeof(struct ionic_v1_base_hdr) != 16);
	BUILD_BUG_ON(sizeof(struct ionic_v1_recv_bdy) != 48);
	BUILD_BUG_ON(sizeof(struct ionic_v1_common_bdy) != 48);
	BUILD_BUG_ON(sizeof(struct ionic_v1_atomic_bdy) != 48);
	BUILD_BUG_ON(sizeof(struct ionic_v1_reg_mr_bdy) != 48);
	BUILD_BUG_ON(sizeof(struct ionic_v1_bind_mw_bdy) != 48);
	BUILD_BUG_ON(sizeof(struct ionic_v1_wqe) != 64);
	BUILD_BUG_ON(sizeof(struct ionic_v1_admin_wqe) != 64);
	BUILD_BUG_ON(sizeof(struct ionic_v1_eqe) != 4);
}

module_init(ionic_mod_init);
module_exit(ionic_mod_exit);

MODULE_DEPEND(ionic_rdma, linuxkpi, 1, 1, 1);
MODULE_DEPEND(ionic_rdma, ibcore, 1, 1, 1);
MODULE_DEPEND(ionic_rdma, ionic, 1, 1, 1);
