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

#include <linux/ctype.h>
#include <linux/debugfs.h>

#include "ionic_ibdebug.h"
#include "ionic_ibdev.h"

static void ionic_umem_show(struct seq_file *s, const char *w,
			    struct ib_umem *umem)
{
	seq_printf(s, "%sumem.length:\t%#lx\n", w, umem->length);
	seq_printf(s, "%sumem.address:\t%#lx\n", w, umem->address);
	seq_printf(s, "%sumem.page_shift:\t%d\n", w, umem->page_shift);
	seq_printf(s, "%sumem.writable:\t%d\n", w, umem->writable);
	seq_printf(s, "%sumem.hugetlb:\t%d\n", w, umem->hugetlb);
	seq_printf(s, "%sumem.nmap:\t%d\n", w, umem->nmap);
	seq_printf(s, "%sumem.npages:\t%d\n", w, umem->npages);

	seq_printf(s, "%sumem.offset():\t%#x\n", w, ib_umem_offset(umem));
	seq_printf(s, "%sumem.start():\t%#lx\n", w, ib_umem_start(umem));
	seq_printf(s, "%sumem.end():\t%#lx\n", w, ib_umem_end(umem));
	seq_printf(s, "%sumem.num_pages():\t%lu\n", w, ib_umem_num_pages(umem));
	seq_printf(s, "%sumem.page_count():\t%d\n", w, ib_umem_page_count(umem));
}

static void ionic_umem_dump(struct seq_file *s, struct ib_umem *umem)
{
	struct scatterlist *sg;
	dma_addr_t pagedma;
	int sg_i, pg_i = 0, pg_end = 0;

	for_each_sg(umem->sg_head.sgl, sg, umem->nmap, sg_i) {
		pagedma = sg_dma_address(sg);
		pg_end += sg_dma_len(sg) >> umem->page_shift;
		for (; pg_i < pg_end; ++pg_i) {
			seq_printf(s, "%#llx\n", pagedma);
			pagedma += BIT_ULL(umem->page_shift);
		}
	}
}

static void ionic_q_show(struct seq_file *s, const char *w,
			 struct ionic_queue *q)
{
	seq_printf(s, "%ssize:\t%#llx\n", w, (u64)q->size);
	seq_printf(s, "%sdma:\t%#llx\n", w, (u64)q->dma);
	seq_printf(s, "%sprod:\t%#06x (%#llx)\n",
		   w, q->prod, (u64)q->prod << q->stride_log2);
	seq_printf(s, "%scons:\t%#06x (%#llx)\n",
		   w, q->cons, (u64)q->cons << q->stride_log2);
	seq_printf(s, "%smask:\t%#06x\n", w, q->mask);
	seq_printf(s, "%sdepth_log2:\t%u\n", w, q->depth_log2);
	seq_printf(s, "%sstride_log2:\t%u\n", w, q->stride_log2);
	seq_printf(s, "%sdbell:\t%#llx\n", w, q->dbell);
}

static void ionic_q_dump(struct seq_file *s, struct ionic_queue *q)
{
	seq_hex_dump(s, "", DUMP_PREFIX_OFFSET, 16, 1, q->ptr, q->size, true);
}

static void ionic_tbl_res_show(struct seq_file *s, const char *w,
			       struct ionic_tbl_res *res)
{
	seq_printf(s, "%stbl_order:\t%u\n", w, res->tbl_order);
	seq_printf(s, "%stbl_pos:\t%#x\n", w, res->tbl_pos);
}

static void ionic_tbl_buf_show(struct seq_file *s, const char *w,
			       struct ionic_tbl_buf *buf)
{
	seq_printf(s, "%stbl_limit:\t%u\n", w, buf->tbl_limit);
	seq_printf(s, "%stbl_pages:\t%u\n", w, buf->tbl_pages);
	seq_printf(s, "%stbl_size:\t%zu\n", w, buf->tbl_size);
	seq_printf(s, "%stbl_dma:\t%#llx\n", w, buf->tbl_dma);
	seq_printf(s, "%spage_size_log2:\t%u\n", w, buf->page_size_log2);
}

static void ionic_tbl_buf_dump(struct seq_file *s, struct ionic_tbl_buf *buf)
{
	int page_i;

	if (!buf->tbl_buf)
		return;

	for (page_i = 0; page_i < buf->tbl_pages; ++page_i)
		seq_printf(s, "%llx\n", buf->tbl_buf[page_i]);
}

static int ionic_dev_info_show(struct seq_file *s, void *v)
{
	struct ionic_ibdev *dev = s->private;

	seq_printf(s, "lif_id:\t%d\n", dev->lif_id);
	seq_printf(s, "dbid:\t%u\n", dev->dbid);

	seq_printf(s, "rdma_version:\t%u\n", dev->rdma_version);
	seq_printf(s, "qp_opcodes:\t%u\n", dev->qp_opcodes);
	seq_printf(s, "admin_opcodes:\t%u\n", dev->admin_opcodes);

	seq_printf(s, "admin_qtype:\t%u\n", dev->admin_qtype);
	seq_printf(s, "sq_qtype:\t%u\n", dev->sq_qtype);
	seq_printf(s, "rq_qtype:\t%u\n", dev->rq_qtype);
	seq_printf(s, "cq_qtype:\t%u\n", dev->cq_qtype);
	seq_printf(s, "eq_qtype:\t%u\n", dev->eq_qtype);

	seq_printf(s, "max_stride:\t%u\n", dev->max_stride);
	seq_printf(s, "cl_stride:\t%u\n", dev->cl_stride);
	seq_printf(s, "pte_stride:\t%u\n", dev->pte_stride);
	seq_printf(s, "rrq_stride:\t%u\n", dev->rrq_stride);
	seq_printf(s, "rsq_stride:\t%u\n", dev->rsq_stride);

	/* XXX remove if for relase */
	if (dev->adminq)
		seq_printf(s, "adminq:\t%u\n", dev->adminq->aqid);
	if (dev->admincq)
		seq_printf(s, "admincq:\t%u\n", dev->admincq->cqid);
	seq_printf(s, "admin_armed:\t%u\n", dev->admin_armed);
	seq_printf(s, "admin_state:\t%u\n", dev->admin_state);

	seq_printf(s, "inuse_pdid:\t%u\n", bitmap_weight(dev->inuse_pdid,
							 dev->size_pdid));
	seq_printf(s, "size_pdid:\t%u\n", dev->size_pdid);
	seq_printf(s, "next_pdid:\t%u\n", dev->next_pdid);

	seq_printf(s, "inuse_mrid:\t%u\n", bitmap_weight(dev->inuse_mrid,
							 dev->size_mrid));
	seq_printf(s, "size_mrid:\t%u\n", dev->size_mrid);
	seq_printf(s, "next_mrid:\t%u\n", dev->next_mrid);
	seq_printf(s, "next_mrkey:\t%u\n", dev->next_mrkey);

	seq_printf(s, "inuse_cqid:\t%u\n", bitmap_weight(dev->inuse_cqid,
							 dev->size_cqid));
	seq_printf(s, "size_cqid:\t%u\n", dev->size_cqid);
	seq_printf(s, "next_cqid:\t%u\n", dev->next_cqid);

	seq_printf(s, "inuse_qpid:\t%u\n", bitmap_weight(dev->inuse_qpid,
							 dev->size_qpid));
	seq_printf(s, "size_qpid:\t%u\n", dev->size_qpid);
	seq_printf(s, "next_qpid:\t%u\n", dev->next_qpid);

	/* includes inuse_qpid: subtract inuse_qpid to get only srqid */
	seq_printf(s, "inuse_srqid:\t%u\n", bitmap_weight(dev->inuse_qpid,
							  dev->size_srqid));
	seq_printf(s, "size_srqid:\t%u\n", dev->size_srqid);
	seq_printf(s, "next_srqid:\t%u\n", dev->next_srqid);

	seq_printf(s, "inuse_restbl:\t%u\n", bitmap_weight(dev->inuse_restbl,
							  dev->size_restbl));
	seq_printf(s, "size_restbl:\t%u\n", dev->size_restbl);

	return 0;
}

static int ionic_dev_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, ionic_dev_info_show, inode->i_private);
}

static const struct file_operations ionic_dev_info_fops = {
	.open = ionic_dev_info_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

void ionic_dbgfs_add_dev(struct ionic_ibdev *dev, struct dentry *parent)
{
	dev->debug = NULL;
	dev->debug_ah = NULL;
	dev->debug_aq = NULL;
	dev->debug_cq = NULL;
	dev->debug_eq = NULL;
	dev->debug_mr = NULL;
	dev->debug_mw = NULL;
	dev->debug_pd = NULL;
	dev->debug_qp = NULL;
	dev->debug_srq = NULL;

	if (IS_ERR_OR_NULL(parent))
		return;

	dev->debug = debugfs_create_dir("rdma", parent);
	if (IS_ERR(dev->debug))
		dev->debug = NULL;
	if (!dev->debug)
		return;

	debugfs_create_file("info", 0220, dev->debug, dev,
			    &ionic_dev_info_fops);

	dev->debug_ah = debugfs_create_dir("ah", dev->debug);
	if (IS_ERR(dev->debug_ah))
		dev->debug_ah = NULL;

	dev->debug_aq = debugfs_create_dir("aq", dev->debug);
	if (IS_ERR(dev->debug_aq))
		dev->debug_aq = NULL;

	dev->debug_cq = debugfs_create_dir("cq", dev->debug);
	if (IS_ERR(dev->debug_cq))
		dev->debug_cq = NULL;

	dev->debug_eq = debugfs_create_dir("eq", dev->debug);
	if (IS_ERR(dev->debug_eq))
		dev->debug_eq = NULL;

	dev->debug_mr = debugfs_create_dir("mr", dev->debug);
	if (IS_ERR(dev->debug_mr))
		dev->debug_mr = NULL;

	dev->debug_mw = debugfs_create_dir("mw", dev->debug);
	if (IS_ERR(dev->debug_mw))
		dev->debug_mw = NULL;

	dev->debug_pd = debugfs_create_dir("pd", dev->debug);
	if (IS_ERR(dev->debug_pd))
		dev->debug_pd = NULL;

	dev->debug_qp = debugfs_create_dir("qp", dev->debug);
	if (IS_ERR(dev->debug_qp))
		dev->debug_qp = NULL;

	dev->debug_srq = debugfs_create_dir("srq", dev->debug);
	if (IS_ERR(dev->debug_srq))
		dev->debug_srq = NULL;
}

void ionic_dbgfs_rm_dev(struct ionic_ibdev *dev)
{
	if (dev->debug)
		debugfs_remove_recursive(dev->debug);

	dev->debug = NULL;
	dev->debug_ah = NULL;
	dev->debug_cq = NULL;
	dev->debug_eq = NULL;
	dev->debug_mr = NULL;
	dev->debug_mw = NULL;
	dev->debug_pd = NULL;
	dev->debug_qp = NULL;
	dev->debug_srq = NULL;
}

static int ionic_eq_info_show(struct seq_file *s, void *v)
{
	struct ionic_eq *eq = s->private;
	u32 __iomem *intr;

	seq_printf(s, "eqid:\t%u\n", eq->eqid);
	seq_printf(s, "intr:\t%u\n", eq->intr);

	ionic_q_show(s, "q.",  &eq->q);
	seq_printf(s, "enable:\t%u\n", eq->enable);
	seq_printf(s, "armed:\t%u\n", eq->armed);
	seq_printf(s, "irq:\t%u\n", eq->irq);
	seq_printf(s, "name:\t%s\n", eq->name);

	/* interrupt control readback */
	intr = &eq->dev->intr_ctrl[eq->intr * IONIC_INTR_REGS_PER];
	seq_printf(s, "intr_coalesce_init:\t%#x\n",
		   ioread32(&intr[IONIC_INTR_REG_COALESCE_INIT]));
	seq_printf(s, "intr_mask:\t%#x\n",
		   ioread32(&intr[IONIC_INTR_REG_MASK]));
	seq_printf(s, "intr_credits:\t%#x\n",
		   ioread32(&intr[IONIC_INTR_REG_CREDITS]));
	seq_printf(s, "intr_mask_assert:\t%#x\n",
		   ioread32(&intr[IONIC_INTR_REG_MASK_ASSERT]));
	seq_printf(s, "intr_coalesce:\t%#x\n",
		   ioread32(&intr[IONIC_INTR_REG_COALESCE]));

	return 0;
}

static int ionic_eq_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, ionic_eq_info_show, inode->i_private);
}

static const struct file_operations ionic_eq_info_fops = {
	.open = ionic_eq_info_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

static int ionic_eq_q_show(struct seq_file *s, void *v)
{
	struct ionic_eq *eq = s->private;

	ionic_q_dump(s, &eq->q);

	return 0;
}

static int ionic_eq_q_open(struct inode *inode, struct file *file)
{
	return single_open(file, ionic_eq_q_show, inode->i_private);
}

static const struct file_operations ionic_eq_q_fops = {
	.open = ionic_eq_q_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

void ionic_dbgfs_add_eq(struct ionic_ibdev *dev, struct ionic_eq *eq)
{
	char name[8];

	eq->debug = NULL;

	if (!dev->debug_eq)
		return;

	snprintf(name, sizeof(name), "%u", eq->eqid);

	eq->debug = debugfs_create_dir(name, dev->debug_eq);
	if (IS_ERR(eq->debug))
		eq->debug = NULL;
	if (!eq->debug)
		return;

	debugfs_create_file("info", 0220, eq->debug, eq,
			    &ionic_eq_info_fops);

	debugfs_create_file("q", 0220, eq->debug, eq,
			    &ionic_eq_q_fops);
}

void ionic_dbgfs_rm_eq(struct ionic_eq *eq)
{
	if (eq->debug)
		debugfs_remove_recursive(eq->debug);

	eq->debug = NULL;
}

static int ionic_mr_info_show(struct seq_file *s, void *v)
{
	struct ionic_mr *mr = s->private;

	seq_printf(s, "mrid:\t%u\n", mr->mrid);

	ionic_tbl_res_show(s, "", &mr->res);
	ionic_tbl_buf_show(s, "", &mr->buf);

	if (mr->umem)
		ionic_umem_show(s, "", mr->umem);

	return 0;
}

static int ionic_mr_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, ionic_mr_info_show, inode->i_private);
}

static const struct file_operations ionic_mr_info_fops = {
	.open = ionic_mr_info_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

static int ionic_mr_umem_show(struct seq_file *s, void *v)
{
	struct ionic_mr *mr = s->private;

	ionic_umem_dump(s, mr->umem);

	return 0;
}

static int ionic_mr_umem_open(struct inode *inode, struct file *file)
{
	return single_open(file, ionic_mr_umem_show, inode->i_private);
}

static const struct file_operations ionic_mr_umem_fops = {
	.open = ionic_mr_umem_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

static int ionic_mr_tbl_buf_show(struct seq_file *s, void *v)
{
	struct ionic_mr *mr = s->private;

	ionic_tbl_buf_dump(s, &mr->buf);

	return 0;
}

static int ionic_mr_tbl_buf_open(struct inode *inode, struct file *file)
{
	return single_open(file, ionic_mr_tbl_buf_show, inode->i_private);
}

static const struct file_operations ionic_mr_tbl_buf_fops = {
	.open = ionic_mr_tbl_buf_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

void ionic_dbgfs_add_mr(struct ionic_ibdev *dev, struct ionic_mr *mr)
{
	char name[8];

	mr->debug = NULL;

	if (!dev->debug_mr)
		return;

	snprintf(name, sizeof(name), "%u", ionic_mrid_index(mr->mrid));

	mr->debug = debugfs_create_dir(name, dev->debug_mr);
	if (IS_ERR(mr->debug))
		mr->debug = NULL;
	if (!mr->debug)
		return;

	debugfs_create_file("info", 0220, mr->debug, mr,
			    &ionic_mr_info_fops);

	if (mr->umem)
		debugfs_create_file("umem", 0220, mr->debug, mr,
				    &ionic_mr_umem_fops);

	if (mr->buf.tbl_buf)
		debugfs_create_file("buf", 0220, mr->debug, mr,
				    &ionic_mr_tbl_buf_fops);
}

void ionic_dbgfs_rm_mr(struct ionic_mr *mr)
{
	if (mr->debug)
		debugfs_remove_recursive(mr->debug);

	mr->debug = NULL;
}

static int ionic_cq_info_show(struct seq_file *s, void *v)
{
	struct ionic_cq *cq = s->private;

	seq_printf(s, "cqid:\t%u\n", cq->cqid);
	seq_printf(s, "eqid:\t%u\n", cq->eqid);

	ionic_tbl_res_show(s, "", &cq->res);

	if (cq->q.ptr) {
		ionic_q_show(s, "", &cq->q);
		seq_printf(s, "arm_any_prod:\t%#06x\n", cq->arm_any_prod);
		seq_printf(s, "arm_sol_prod:\t%#06x\n", cq->arm_sol_prod);
	}

	if (cq->umem)
		ionic_umem_show(s, "", cq->umem);

	return 0;
}

static int ionic_cq_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, ionic_cq_info_show, inode->i_private);
}

static const struct file_operations ionic_cq_info_fops = {
	.open = ionic_cq_info_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

static int ionic_cq_q_show(struct seq_file *s, void *v)
{
	struct ionic_cq *cq = s->private;

	ionic_q_dump(s, &cq->q);

	return 0;
}

static int ionic_cq_q_open(struct inode *inode, struct file *file)
{
	return single_open(file, ionic_cq_q_show, inode->i_private);
}

static const struct file_operations ionic_cq_q_fops = {
	.open = ionic_cq_q_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

static int ionic_cq_umem_show(struct seq_file *s, void *v)
{
	struct ionic_cq *cq = s->private;

	ionic_umem_dump(s, cq->umem);

	return 0;
}

static int ionic_cq_umem_open(struct inode *inode, struct file *file)
{
	return single_open(file, ionic_cq_umem_show, inode->i_private);
}

static const struct file_operations ionic_cq_umem_fops = {
	.open = ionic_cq_umem_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

void ionic_dbgfs_add_cq(struct ionic_ibdev *dev, struct ionic_cq *cq)
{
	char name[8];

	cq->debug = NULL;

	if (!dev->debug_cq)
		return;

	snprintf(name, sizeof(name), "%u", cq->cqid);

	cq->debug = debugfs_create_dir(name, dev->debug_cq);
	if (IS_ERR(cq->debug))
		cq->debug = NULL;
	if (!cq->debug)
		return;

	debugfs_create_file("info", 0220, cq->debug, cq,
			    &ionic_cq_info_fops);

	if (cq->q.ptr)
		debugfs_create_file("q", 0220, cq->debug, cq,
				    &ionic_cq_q_fops);

	if (cq->umem)
		debugfs_create_file("umem", 0220, cq->debug, cq,
				    &ionic_cq_umem_fops);
}

void ionic_dbgfs_rm_cq(struct ionic_cq *cq)
{
	if (cq->debug)
		debugfs_remove_recursive(cq->debug);

	cq->debug = NULL;
}

static int ionic_aq_info_show(struct seq_file *s, void *v)
{
	struct ionic_aq *aq = s->private;

	seq_printf(s, "aqid:\t%u\n", aq->aqid);
	seq_printf(s, "cqid:\t%u\n", aq->cqid);

	if (aq->q.ptr)
		ionic_q_show(s, "", &aq->q);

	return 0;
}

static int ionic_aq_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, ionic_aq_info_show, inode->i_private);
}

static const struct file_operations ionic_aq_info_fops = {
	.open = ionic_aq_info_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

static int ionic_aq_q_show(struct seq_file *s, void *v)
{
	struct ionic_aq *aq = s->private;

	ionic_q_dump(s, &aq->q);

	return 0;
}

static int ionic_aq_q_open(struct inode *inode, struct file *file)
{
	return single_open(file, ionic_aq_q_show, inode->i_private);
}

static const struct file_operations ionic_aq_q_fops = {
	.open = ionic_aq_q_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

static int ionic_aq_wqe_show(struct seq_file *s, void *v)
{
	struct ionic_aq *aq = s->private;
	struct ionic_v1_admin_wqe *wqe = &aq->debug_wr->wqe;

	seq_hex_dump(s, "", DUMP_PREFIX_OFFSET, 16, 1, wqe, sizeof(*wqe), true);

	return 0;
}

static int ionic_aq_wqe_open(struct inode *inode, struct file *file)
{
	return single_open(file, ionic_aq_wqe_show, inode->i_private);
}

static const struct file_operations ionic_aq_wqe_fops = {
	.open = ionic_aq_wqe_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

static int ionic_aq_cqe_show(struct seq_file *s, void *v)
{
	struct ionic_aq *aq = s->private;
	struct ionic_v1_cqe *cqe = &aq->debug_wr->cqe;

	seq_hex_dump(s, "", DUMP_PREFIX_OFFSET, 16, 1, cqe, sizeof(*cqe), true);

	return 0;
}

static int ionic_aq_cqe_open(struct inode *inode, struct file *file)
{
	return single_open(file, ionic_aq_cqe_show, inode->i_private);
}

static const struct file_operations ionic_aq_cqe_fops = {
	.open = ionic_aq_cqe_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

struct ionic_dbgfs_admin_wr {
	struct ionic_aq *aq;
	struct ionic_admin_wr wr;
	void *data;
	dma_addr_t dma;
};

static int ionic_aq_data_show(struct seq_file *s, void *v)
{
	struct ionic_aq *aq = s->private;
	struct ionic_dbgfs_admin_wr *wr =
		container_of(aq->debug_wr, struct ionic_dbgfs_admin_wr, wr);

	dma_sync_single_for_cpu(aq->dev->hwdev, wr->dma, PAGE_SIZE,
				DMA_FROM_DEVICE);

	seq_hex_dump(s, "", DUMP_PREFIX_OFFSET, 16, 1,
		     wr->data, PAGE_SIZE, true);

	dma_sync_single_for_device(aq->dev->hwdev, wr->dma, PAGE_SIZE,
				   DMA_FROM_DEVICE);

	return 0;
}

static int ionic_aq_data_open(struct inode *inode, struct file *file)
{
	return single_open(file, ionic_aq_data_show, inode->i_private);
}

static const struct file_operations ionic_aq_data_fops = {
	.open = ionic_aq_data_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
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

static ssize_t ionic_aq_ctrl_write(struct file *fp, const char __user *ubuf,
				   size_t count, loff_t *ppos)
{
	struct ionic_aq *aq = fp->private_data;
	struct ionic_dbgfs_admin_wr *wr =
		container_of(aq->debug_wr, struct ionic_dbgfs_admin_wr, wr);
	long timeout;
	char *buf;
	int val, num, pos = 0, rc = 0;

	buf = kmalloc(count + 1, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	rc = copy_from_user(buf, ubuf, count);
	if (rc)
		goto out;

	buf[count] = 0;

	while (pos < count) {
		if (isspace(buf[pos])) {
			++pos;
			continue;
		}

		num = match_whole_prefix(buf + pos, "post");
		if (num) {
			pos += num;

			reinit_completion(&wr->wr.work);

			ionic_admin_post(aq->dev, &wr->wr);

			timeout = wait_for_completion_interruptible_timeout(&wr->wr.work, HZ);
			if (timeout > 0)
				rc = 0;
			else if (timeout == 0)
				rc = -ETIMEDOUT;
			else
				rc = timeout;

			if (rc) {
				dev_warn(&aq->dev->ibdev.dev, "wait %d\n", rc);
				ionic_admin_cancel(aq->dev, &wr->wr);
				goto out;
			} else if (wr->wr.status == IONIC_ADMIN_KILLED) {
				dev_dbg(&aq->dev->ibdev.dev, "killed\n");
				rc = 0;
				goto out;
			} else if (ionic_v1_cqe_error(&wr->wr.cqe)) {
				dev_warn(&aq->dev->ibdev.dev, "cqe error %u\n",
					 le32_to_cpu(wr->wr.cqe.status_length));
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
	kfree(buf);

	return rc ?: count;
}

static const struct file_operations ionic_aq_ctrl_fops = {
	.open = simple_open,
	.write = ionic_aq_ctrl_write,
};

void ionic_dbgfs_add_aq(struct ionic_ibdev *dev, struct ionic_aq *aq)
{
	struct ionic_dbgfs_admin_wr *wr;
	char name[8];

	aq->debug = NULL;

	if (!dev->debug_aq)
		return;

	snprintf(name, sizeof(name), "%u", aq->aqid);

	aq->debug = debugfs_create_dir(name, dev->debug_aq);
	if (IS_ERR(aq->debug))
		aq->debug = NULL;
	if (!aq->debug)
		return;

	debugfs_create_file("info", 0220, aq->debug, aq,
			    &ionic_aq_info_fops);

	if (aq->q.ptr)
		debugfs_create_file("q", 0220, aq->debug, aq,
				    &ionic_aq_q_fops);

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

	debugfs_create_file("dbg_wr_wqe", 0220, aq->debug, aq,
			    &ionic_aq_wqe_fops);

	debugfs_create_file("dbg_wr_cqe", 0220, aq->debug, aq,
			    &ionic_aq_cqe_fops);

	debugfs_create_file("dbg_wr_data", 0220, aq->debug, aq,
			    &ionic_aq_data_fops);

	debugfs_create_file("dbg_wr_ctrl", 0220, aq->debug, aq,
			    &ionic_aq_ctrl_fops);

	return;

err_dma:
	kfree(wr->data);
err_data:
	kfree(wr);
err_wr:
	return;
}

void ionic_dbgfs_rm_aq(struct ionic_aq *aq)
{
	struct ionic_ibdev *dev = aq->dev;
	struct ionic_dbgfs_admin_wr *wr;

	if (aq->debug)
		debugfs_remove_recursive(aq->debug);

	aq->debug = NULL;

	if (!aq->debug_wr)
		return;

	wr = container_of(aq->debug_wr, struct ionic_dbgfs_admin_wr, wr);

	dma_unmap_single(dev->hwdev, wr->dma, PAGE_SIZE, DMA_FROM_DEVICE);
	kfree(wr->data);
	kfree(wr);
}

static int ionic_qp_info_show(struct seq_file *s, void *v)
{
	struct ionic_qp *qp = s->private;

	seq_printf(s, "qpid:\t%u\n", qp->qpid);

	if (qp->has_sq) {
		if (qp->sq.ptr)
			ionic_q_show(s, "sq.", &qp->sq);

		if (qp->sq_umem)
			ionic_umem_show(s, "sq.", qp->sq_umem);

		ionic_tbl_res_show(s, "sq.", &qp->sq_res);

		seq_printf(s, "sq_is_cmb:\t%d\n", qp->sq_is_cmb);
		if (qp->sq_is_cmb) {
			seq_printf(s, "sq_cmb_order:\t%d\n", qp->sq_cmb_order);
			seq_printf(s, "sq_cmb_pgid:\t%d\n", qp->sq_cmb_pgid);
			seq_printf(s, "sq_cmb_addr:\t%#llx\n", (u64)qp->sq_cmb_addr);
		}
	}

	if (qp->has_rq) {
		if (qp->rq.ptr)
			ionic_q_show(s, "rq.", &qp->rq);

		if (qp->rq_umem)
			ionic_umem_show(s, "rq.", qp->rq_umem);

		ionic_tbl_res_show(s, "rq.", &qp->rq_res);

		seq_printf(s, "rq_is_cmb:\t%d\n", qp->rq_is_cmb);
		if (qp->rq_is_cmb) {
			seq_printf(s, "rq_cmb_order:\t%d\n", qp->rq_cmb_order);
			seq_printf(s, "rq_cmb_pgid:\t%d\n", qp->rq_cmb_pgid);
			seq_printf(s, "rq_cmb_addr:\t%#llx\n", (u64)qp->rq_cmb_addr);
		}
	}

	return 0;
}

static int ionic_qp_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, ionic_qp_info_show, inode->i_private);
}

static const struct file_operations ionic_qp_info_fops = {
	.open = ionic_qp_info_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

static int ionic_qp_sq_show(struct seq_file *s, void *v)
{
	struct ionic_qp *qp = s->private;

	ionic_q_dump(s, &qp->sq);

	return 0;
}

static int ionic_qp_sq_open(struct inode *inode, struct file *file)
{
	return single_open(file, ionic_qp_sq_show, inode->i_private);
}

static const struct file_operations ionic_qp_sq_fops = {
	.open = ionic_qp_sq_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

static int ionic_qp_sq_umem_show(struct seq_file *s, void *v)
{
	struct ionic_qp *qp = s->private;

	ionic_umem_dump(s, qp->sq_umem);

	return 0;
}

static int ionic_qp_sq_umem_open(struct inode *inode, struct file *file)
{
	return single_open(file, ionic_qp_sq_umem_show, inode->i_private);
}

static const struct file_operations ionic_qp_sq_umem_fops = {
	.open = ionic_qp_sq_umem_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

static int ionic_qp_rq_show(struct seq_file *s, void *v)
{
	struct ionic_qp *qp = s->private;

	ionic_q_dump(s, &qp->rq);

	return 0;
}

static int ionic_qp_rq_open(struct inode *inode, struct file *file)
{
	return single_open(file, ionic_qp_rq_show, inode->i_private);
}

static const struct file_operations ionic_qp_rq_fops = {
	.open = ionic_qp_rq_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

static int ionic_qp_rq_umem_show(struct seq_file *s, void *v)
{
	struct ionic_qp *qp = s->private;

	ionic_umem_dump(s, qp->rq_umem);

	return 0;
}

static int ionic_qp_rq_umem_open(struct inode *inode, struct file *file)
{
	return single_open(file, ionic_qp_rq_umem_show, inode->i_private);
}

static const struct file_operations ionic_qp_rq_umem_fops = {
	.open = ionic_qp_rq_umem_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

void ionic_dbgfs_add_qp(struct ionic_ibdev *dev, struct ionic_qp *qp)
{
	char name[8];

	qp->debug = NULL;

	if (!dev->debug_qp)
		return;

	snprintf(name, sizeof(name), "%u", qp->qpid);

	qp->debug = debugfs_create_dir(name, dev->debug_qp);
	if (IS_ERR(qp->debug))
		qp->debug = NULL;
	if (!qp->debug)
		return;

	debugfs_create_file("info", 0220, qp->debug, qp,
			    &ionic_qp_info_fops);

	if (qp->has_sq) {
		if (qp->sq.ptr)
			debugfs_create_file("sq", 0220, qp->debug, qp,
					    &ionic_qp_sq_fops);

		if (qp->sq_umem)
			debugfs_create_file("sq_umem", 0220, qp->debug, qp,
					    &ionic_qp_sq_umem_fops);
	}

	if (qp->has_rq) {
		if (qp->rq.ptr)
			debugfs_create_file("rq", 0220, qp->debug, qp,
					    &ionic_qp_rq_fops);

		if (qp->rq_umem)
			debugfs_create_file("rq_umem", 0220, qp->debug, qp,
					    &ionic_qp_rq_umem_fops);
	}
}

void ionic_dbgfs_rm_qp(struct ionic_qp *qp)
{
	if (qp->debug)
		debugfs_remove_recursive(qp->debug);

	qp->debug = NULL;
}
