// SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB
/*
 * Copyright (c) 2018-2020 Pensando Systems, Inc.  All rights reserved.
 */

#include <linux/ctype.h>
#include <linux/debugfs.h>
#include <linux/module.h>

#include "ionic_sysfs.h"
#include "ionic_ibdev.h"

bool ionic_dbg_enable = true;
module_param_named(dbgfs, ionic_dbg_enable, bool, 0444);
MODULE_PARM_DESC(dbgfs, "Enable debugfs for this driver.");

int ionic_sqcmb_order = 5; /* 32 pages */
module_param_named(sqcmb_order, ionic_sqcmb_order, int, 0644);
MODULE_PARM_DESC(sqcmb_order, "Only alloc sq cmb less than order.");

bool ionic_sqcmb_inline = false;
module_param_named(sqcmb_inline, ionic_sqcmb_inline, bool, 0644);
MODULE_PARM_DESC(sqcmb_inline, "Only alloc sq cmb for inline data capability.");

int ionic_rqcmb_order = 5; /* 32 pages */
module_param_named(rqcmb_order, ionic_rqcmb_order, int, 0644);
MODULE_PARM_DESC(rqcmb_order, "Only alloc rq cmb less than order.");

u16 ionic_aq_depth = 63;
module_param_named(aq_depth, ionic_aq_depth, ushort, 0444);
MODULE_PARM_DESC(aq_depth, "Min depth for admin queues.");

int ionic_aq_count = 4;
module_param_named(ionic_rdma_aq_count, ionic_aq_count, int, 0644);
MODULE_PARM_DESC(ionic_rdma_aq_count, "Limit number of admin queues created.");

u16 ionic_eq_depth = 511;
module_param_named(eq_depth, ionic_eq_depth, ushort, 0444);
MODULE_PARM_DESC(eq_depth, "Min depth for event queues.");

u16 ionic_eq_isr_budget = 10;
module_param_named(isr_budget, ionic_eq_isr_budget, ushort, 0644);
MODULE_PARM_DESC(isr_budget, "Max events to poll per round in isr context.");

u16 ionic_eq_work_budget = 1000;
module_param_named(work_budget, ionic_eq_work_budget, ushort, 0644);
MODULE_PARM_DESC(work_budget, "Max events to poll per round in work context.");

int ionic_max_pd = 1024;
module_param_named(max_pd, ionic_max_pd, int, 0444);
MODULE_PARM_DESC(max_pd, "Max number of PDs.");

static bool ionic_nosupport = false;
module_param_named(nosupport, ionic_nosupport, bool, 0644);
MODULE_PARM_DESC(nosupport, "Enable unsupported config values");

static int ionic_set_spec(const char *val, const struct kernel_param *kp)
{
	int rc, tmp;

	rc = kstrtoint(val, 0, &tmp);
	if (rc)
		return rc;

	if (tmp != 8 && tmp != 16 && !ionic_nosupport) {
		pr_info("ionic_rdma: invalid spec %d, using 8 instead\n", tmp);
		pr_info("ionic_rdma: valid spec values are 8 and 16\n");
		tmp = 8;
	}

	*(int *)kp->arg = tmp;

	return 0;
}
static const struct kernel_param_ops ionic_spec_ops = {
	.set = ionic_set_spec,
	.get = param_get_int,
};
int ionic_spec = 16;
module_param_cb(spec, &ionic_spec_ops, &ionic_spec, 0644);
MODULE_PARM_DESC(spec, "Max SGEs for speculation.");

static void ionic_umem_show(struct seq_file *s, const char *w,
			    struct ib_umem *umem)
{
	seq_printf(s, "%sumem.length:\t%#lx\n", w, umem->length);
	seq_printf(s, "%sumem.address:\t%#lx\n", w, umem->address);
	seq_printf(s, "%sumem.page_size:\t%lu\n", w, PAGE_SIZE);
	seq_printf(s, "%sumem.writable:\t%d\n", w, umem->writable);
	seq_printf(s, "%sumem.nmap:\t%d\n", w, umem->nmap);

	seq_printf(s, "%sumem.offset():\t%#x\n", w, ib_umem_offset(umem));
	seq_printf(s, "%sumem.num_pages():\t%lu\n",
		   w, ib_umem_num_pages(umem));
	seq_printf(s, "%sumem.page_count():\t%d\n",
		   w, ib_umem_page_count(umem));
}

static void ionic_umem_dump(struct seq_file *s, struct ib_umem *umem)
{
	struct scatterlist *sg;
	dma_addr_t pagedma;
	int sg_i, pg_i = 0, pg_end = 0;

	for_each_sg(umem->sg_head.sgl, sg, umem->nmap, sg_i) {
		pagedma = sg_dma_address(sg);
		pg_end += sg_dma_len(sg) * PAGE_SIZE;
		for (; pg_i < pg_end; ++pg_i) {
			seq_printf(s, "%#llx\n", pagedma);
			pagedma += PAGE_SIZE;
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
	int i;

	seq_printf(s, "lif_index:\t%d\n", dev->lif_index);
	seq_printf(s, "dbid:\t%u\n", dev->dbid);

	seq_printf(s, "rdma_version:\t%u\n", dev->rdma_version);
	seq_printf(s, "qp_opcodes:\t%u\n", dev->qp_opcodes);
	seq_printf(s, "admin_opcodes:\t%u\n", dev->admin_opcodes);
	seq_printf(s, "reset_cnt:\t%u\n", dev->reset_cnt);

	seq_printf(s, "aq_base:\t%u\n", dev->aq_base);
	seq_printf(s, "cq_base:\t%u\n", dev->cq_base);
	seq_printf(s, "eq_base:\t%u\n", dev->eq_base);

	seq_printf(s, "aq_count:\t%u\n", dev->aq_count);
	seq_printf(s, "eq_count:\t%u\n", dev->eq_count);

	seq_printf(s, "aq_qtype:\t%u\n", dev->aq_qtype);
	seq_printf(s, "sq_qtype:\t%u\n", dev->sq_qtype);
	seq_printf(s, "rq_qtype:\t%u\n", dev->rq_qtype);
	seq_printf(s, "cq_qtype:\t%u\n", dev->cq_qtype);
	seq_printf(s, "eq_qtype:\t%u\n", dev->eq_qtype);

	seq_printf(s, "max_stride:\t%u\n", dev->max_stride);
	seq_printf(s, "cl_stride:\t%u\n", dev->cl_stride);
	seq_printf(s, "pte_stride:\t%u\n", dev->pte_stride);
	seq_printf(s, "rrq_stride:\t%u\n", dev->rrq_stride);
	seq_printf(s, "rsq_stride:\t%u\n", dev->rsq_stride);

	seq_printf(s, "admin_state:\t%u\n", dev->admin_state);

	seq_printf(s, "inuse_pdid:\t%u\n",
		   bitmap_weight(dev->inuse_pdid.inuse,
				 dev->inuse_pdid.inuse_size));
	seq_printf(s, "size_pdid:\t%u\n", dev->inuse_pdid.inuse_size);
	seq_printf(s, "next_pdid:\t%u\n", dev->inuse_pdid.next_id);

	seq_printf(s, "inuse_mrid:\t%u\n",
		   bitmap_weight(dev->inuse_mrid.inuse,
				 dev->inuse_mrid.inuse_size));
	seq_printf(s, "size_mrid:\t%u\n", dev->inuse_mrid.inuse_size);
	seq_printf(s, "next_mrid:\t%u\n", dev->inuse_mrid.next_id);
	seq_printf(s, "next_mrkey:\t%u\n", dev->next_mrkey);

	seq_printf(s, "inuse_cqid:\t%u\n",
		   bitmap_weight(dev->inuse_cqid.inuse,
				 dev->inuse_cqid.inuse_size));
	seq_printf(s, "size_cqid:\t%u\n", dev->inuse_cqid.inuse_size);
	seq_printf(s, "next_cqid:\t%u\n", dev->inuse_cqid.next_id);

	seq_printf(s, "inuse_qpid:\t%u\n",
		   bitmap_weight(dev->inuse_qpid.inuse,
				 dev->size_qpid));
	seq_printf(s, "size_qpid:\t%u\n", dev->size_qpid);
	seq_printf(s, "next_qpid:\t%u\n", dev->inuse_qpid.next_id);

#ifdef IONIC_SRQ_XRC
	/* includes inuse_qpid: subtract inuse_qpid to get only srqid */
	seq_printf(s, "inuse_srqid:\t%u\n",
		   bitmap_weight(dev->inuse_qpid.inuse,
				 dev->size_srqid));
	seq_printf(s, "size_srqid:\t%u\n", dev->size_srqid);
	seq_printf(s, "next_srqid:\t%u\n", dev->next_srqid);
#endif /* IONIC_SRQ_XRC */

	seq_printf(s, "inuse_restbl:\t%u\n",
		   bitmap_weight(dev->inuse_restbl.inuse,
				 dev->inuse_restbl.inuse_size));
	seq_printf(s, "size_restbl:\t%u\n",
		   dev->inuse_restbl.inuse_size);
	seq_printf(s, "order_restbl:\t%u\n", dev->inuse_restbl.order_max);

	for (i = 0; i <= dev->inuse_restbl.order_max; ++i)
		seq_printf(s, "next_restbl[%d]:\t%u\n", i,
			   dev->inuse_restbl.order_next[i]);

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

static ssize_t ionic_dev_reset_write(struct file *fp, const char __user *ubuf,
				     size_t count, loff_t *ppos)
{
	struct ionic_ibdev *dev = fp->private_data;
	char buf[16] = {};
	int rc = 0;

	if (count > sizeof(buf))
		return -EINVAL;

	rc = copy_from_user(buf, ubuf, count);
	if (rc)
		return rc;

	if (strcmp(buf, "1") && strcmp(buf, "1\n"))
		return -EINVAL;

	dev_warn(&dev->ibdev.dev, "resetting...\n");
	ionic_ibdev_reset(dev);
	return count;
}

static const struct file_operations ionic_dev_reset_fops = {
	.open = simple_open,
	.write = ionic_dev_reset_write,
};

void ionic_dbg_add_dev(struct ionic_ibdev *dev, struct dentry *parent)
{
	dev->debug = NULL;
	dev->debug_aq = NULL;
	dev->debug_cq = NULL;
	dev->debug_eq = NULL;
	dev->debug_mr = NULL;
	dev->debug_qp = NULL;

	if (IS_ERR_OR_NULL(parent))
		return;

	dev->debug = debugfs_create_dir("rdma", parent);
	if (IS_ERR(dev->debug))
		dev->debug = NULL;
	if (!dev->debug)
		return;

	debugfs_create_file("info", 0440, dev->debug, dev,
			    &ionic_dev_info_fops);
	debugfs_create_file("reset", 0220, dev->debug, dev,
			    &ionic_dev_reset_fops);

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

	dev->debug_qp = debugfs_create_dir("qp", dev->debug);
	if (IS_ERR(dev->debug_qp))
		dev->debug_qp = NULL;
}

void ionic_dbg_rm_dev(struct ionic_ibdev *dev)
{
	debugfs_remove_recursive(dev->debug);

	dev->debug = NULL;
	dev->debug_cq = NULL;
	dev->debug_eq = NULL;
	dev->debug_mr = NULL;
	dev->debug_qp = NULL;
}

static int ionic_eq_info_show(struct seq_file *s, void *v)
{
	struct ionic_eq *eq = s->private;
	struct ionic_intr __iomem *intr;

	seq_printf(s, "eqid:\t%u\n", eq->eqid);
	seq_printf(s, "intr:\t%u\n", eq->intr);

	ionic_q_show(s, "q.",  &eq->q);
	seq_printf(s, "enable:\t%u\n", eq->enable);
	seq_printf(s, "armed:\t%u\n", eq->armed);
	seq_printf(s, "irq:\t%u\n", eq->irq);
	seq_printf(s, "name:\t%s\n", eq->name);

	/* interrupt control readback */
	intr = &eq->dev->intr_ctrl[eq->intr];
	seq_printf(s, "intr_coalesce_init:\t%#x\n",
		   ioread32(&intr->coal_init));
	seq_printf(s, "intr_mask:\t%#x\n",
		   ioread32(&intr->mask));
	seq_printf(s, "intr_credits:\t%#x\n",
		   ioread32(&intr->credits));
	seq_printf(s, "intr_mask_assert:\t%#x\n",
		   ioread32(&intr->mask_assert));
	seq_printf(s, "intr_coalesce:\t%#x\n",
		   ioread32(&intr->coal));

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

void ionic_dbg_add_eq(struct ionic_ibdev *dev, struct ionic_eq *eq)
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

	debugfs_create_file("info", 0440, eq->debug, eq,
			    &ionic_eq_info_fops);

	debugfs_create_file("q", 0440, eq->debug, eq,
			    &ionic_eq_q_fops);
}

void ionic_dbg_rm_eq(struct ionic_eq *eq)
{
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

void ionic_dbg_add_mr(struct ionic_ibdev *dev, struct ionic_mr *mr)
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

	debugfs_create_file("info", 0440, mr->debug, mr,
			    &ionic_mr_info_fops);

	if (mr->umem)
		debugfs_create_file("umem", 0440, mr->debug, mr,
				    &ionic_mr_umem_fops);

	if (mr->buf.tbl_buf)
		debugfs_create_file("buf", 0440, mr->debug, mr,
				    &ionic_mr_tbl_buf_fops);
}

void ionic_dbg_rm_mr(struct ionic_mr *mr)
{
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

void ionic_dbg_add_cq(struct ionic_ibdev *dev, struct ionic_cq *cq)
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

	debugfs_create_file("info", 0440, cq->debug, cq,
			    &ionic_cq_info_fops);

	if (cq->q.ptr)
		debugfs_create_file("q", 0440, cq->debug, cq,
				    &ionic_cq_q_fops);

	if (cq->umem)
		debugfs_create_file("umem", 0440, cq->debug, cq,
				    &ionic_cq_umem_fops);
}

void ionic_dbg_rm_cq(struct ionic_cq *cq)
{
	debugfs_remove_recursive(cq->debug);

	cq->debug = NULL;
}

static int ionic_aq_info_show(struct seq_file *s, void *v)
{
	struct ionic_aq *aq = s->private;

	seq_printf(s, "armed:\t%u\n", aq->armed);
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

#ifdef NOT_UPSTREAM
static int ionic_aq_wqe_show(struct seq_file *s, void *v)
{
	struct ionic_aq *aq = s->private;
	struct ionic_v1_admin_wqe *wqe = &aq->debug_wr->wqe;

	seq_hex_dump(s, "", DUMP_PREFIX_OFFSET, 16, 1, wqe, sizeof(*wqe),
		     true);

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

	seq_hex_dump(s, "", DUMP_PREFIX_OFFSET, 16, 1, cqe, sizeof(*cqe),
		     true);

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

struct ionic_dbg_admin_wr {
	struct ionic_aq *aq;
	struct ionic_admin_wr wr;
	void *data;
	dma_addr_t dma;
};

static int ionic_aq_data_show(struct seq_file *s, void *v)
{
	struct ionic_aq *aq = s->private;
	struct ionic_dbg_admin_wr *wr =
		container_of(aq->debug_wr, struct ionic_dbg_admin_wr, wr);

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
	struct ionic_dbg_admin_wr *wr =
		container_of(aq->debug_wr, struct ionic_dbg_admin_wr, wr);
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
	kfree(buf);

	return rc ?: count;
}

static const struct file_operations ionic_aq_ctrl_fops = {
	.open = simple_open,
	.write = ionic_aq_ctrl_write,
};

#endif /* NOT_UPSTREAM */
void ionic_dbg_add_aq(struct ionic_ibdev *dev, struct ionic_aq *aq)
{
#ifdef NOT_UPSTREAM
	struct ionic_dbg_admin_wr *wr;
#endif /* NOT_UPSTREAM */
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

	debugfs_create_file("info", 0440, aq->debug, aq,
			    &ionic_aq_info_fops);

	if (aq->q.ptr)
		debugfs_create_file("q", 0440, aq->debug, aq,
				    &ionic_aq_q_fops);
#ifdef NOT_UPSTREAM

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

	debugfs_create_file("dbg_wr_wqe", 0440, aq->debug, aq,
			    &ionic_aq_wqe_fops);

	debugfs_create_file("dbg_wr_cqe", 0440, aq->debug, aq,
			    &ionic_aq_cqe_fops);

	debugfs_create_file("dbg_wr_data", 0440, aq->debug, aq,
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
#endif /* NOT_UPSTREAM */
}

void ionic_dbg_rm_aq(struct ionic_aq *aq)
{
#ifdef NOT_UPSTREAM
	struct ionic_ibdev *dev = aq->dev;
	struct ionic_dbg_admin_wr *wr;

#endif /* NOT_UPSTREAM */
	debugfs_remove_recursive(aq->debug);

	aq->debug = NULL;
#ifdef NOT_UPSTREAM

	if (!aq->debug_wr)
		return;

	wr = container_of(aq->debug_wr, struct ionic_dbg_admin_wr, wr);

	dma_unmap_single(dev->hwdev, wr->dma, PAGE_SIZE, DMA_FROM_DEVICE);
	kfree(wr->data);
	kfree(wr);
#endif /* NOT_UPSTREAM */
}

static int ionic_qp_info_show(struct seq_file *s, void *v)
{
	struct ionic_qp *qp = s->private;

	seq_printf(s, "qpid:\t%u\n", qp->qpid);
	seq_printf(s, "state:\t%d\n", qp->state);

	if (qp->has_sq) {
		if (qp->sq.ptr) {
			ionic_q_show(s, "sq.", &qp->sq);
			seq_printf(s, "sq_msn_prod:\t%#06x\n",
				   qp->sq_msn_prod);
			seq_printf(s, "sq_msn_cons:\t%#06x\n",
				   qp->sq_msn_cons);
		}

		if (qp->sq_umem)
			ionic_umem_show(s, "sq.", qp->sq_umem);

		ionic_tbl_res_show(s, "sq.", &qp->sq_res);

		seq_printf(s, "sq_is_cmb:\t%d\n", qp->sq_is_cmb);
		if (qp->sq_is_cmb) {
			seq_printf(s, "sq_cmb_order:\t%d\n", qp->sq_cmb_order);
			seq_printf(s, "sq_cmb_pgid:\t%d\n", qp->sq_cmb_pgid);
			seq_printf(s, "sq_cmb_addr:\t%#llx\n",
				   (u64)qp->sq_cmb_addr);
		}

		seq_printf(s, "sq_flush:\t%d\n", qp->sq_flush);
		seq_printf(s, "sq_flush_rcvd:\t%d\n", qp->sq_flush_rcvd);
		seq_printf(s, "sq_spec:\t%d\n", qp->sq_spec);
		seq_printf(s, "sq_cqid:\t%u\n", qp->sq_cqid);
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
			seq_printf(s, "rq_cmb_addr:\t%#llx\n",
				   (u64)qp->rq_cmb_addr);
		}

		seq_printf(s, "rq_flush:\t%d\n", qp->rq_flush);
		seq_printf(s, "rq_spec:\t%d\n", qp->rq_spec);
		seq_printf(s, "rq_cqid:\t%u\n", qp->rq_cqid);
	}

	seq_printf(s, "dcqcn_profile:\t%d\n", qp->dcqcn_profile);

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

void ionic_dbg_add_qp(struct ionic_ibdev *dev, struct ionic_qp *qp)
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

	debugfs_create_file("info", 0440, qp->debug, qp,
			    &ionic_qp_info_fops);

	if (qp->has_sq) {
		if (qp->sq.ptr)
			debugfs_create_file("sq", 0440, qp->debug, qp,
					    &ionic_qp_sq_fops);

		if (qp->sq_umem)
			debugfs_create_file("sq_umem", 0440, qp->debug, qp,
					    &ionic_qp_sq_umem_fops);
	}

	if (qp->has_rq) {
		if (qp->rq.ptr)
			debugfs_create_file("rq", 0440, qp->debug, qp,
					    &ionic_qp_rq_fops);

		if (qp->rq_umem)
			debugfs_create_file("rq_umem", 0440, qp->debug, qp,
					    &ionic_qp_rq_umem_fops);
	}
}

void ionic_dbg_rm_qp(struct ionic_qp *qp)
{
	debugfs_remove_recursive(qp->debug);

	qp->debug = NULL;
}
