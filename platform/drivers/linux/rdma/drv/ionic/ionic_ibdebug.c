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
			seq_printf(s, "%#llx\n", BIT_ULL(63) | pagedma);
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
		   w, q->prod, (u64)q->prod * q->stride);
	seq_printf(s, "%scons:\t%#06x (%#llx)\n",
		   w, q->cons, (u64)q->cons * q->stride);
	seq_printf(s, "%smask:\t%#06x\n", w, q->mask);
	seq_printf(s, "%sstride:\t%#06x\n", w, q->stride);
	seq_printf(s, "%sdbell:\t%#llx\n", w, q->dbell);
}

static void ionic_q_dump(struct seq_file *s, struct ionic_queue *q)
{
	seq_hex_dump(s, "", DUMP_PREFIX_OFFSET, 16, 1, q->ptr, q->size, true);
}

static int ionic_dev_info_show(struct seq_file *s, void *v)
{
	struct ionic_ibdev *dev = s->private;

	seq_printf(s, "lif_id:\t%d\n", dev->lif_id);
	seq_printf(s, "phys_dbpage_base:\t%#llx\n",
		   (u64)dev->phys_dbpage_base);
	seq_printf(s, "dbid:\t%u\n", dev->dbid);

	seq_printf(s, "sq_qtype:\t%u\n", dev->sq_qtype);
	seq_printf(s, "rq_qtype:\t%u\n", dev->rq_qtype);
	seq_printf(s, "cq_qtype:\t%u\n", dev->cq_qtype);
	seq_printf(s, "eq_qtype:\t%u\n", dev->eq_qtype);

	seq_printf(s, "inuse_pdid:\t%u\n", bitmap_weight(dev->inuse_pdid,
							 dev->size_pdid));
	seq_printf(s, "size_pdid:\t%u\n", dev->size_pdid);
	seq_printf(s, "next_pdid:\t%u\n", dev->next_pdid);

	seq_printf(s, "inuse_mrid:\t%u\n", bitmap_weight(dev->inuse_mrid,
							 dev->size_mrid));
	seq_printf(s, "size_mrid:\t%u\n", dev->size_mrid);
	seq_printf(s, "next_mrid:\t%u\n", dev->next_mrid);
	seq_printf(s, "next_rkey_key:\t%u\n", dev->next_rkey_key);

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

	seq_printf(s, "inuse_pgtbl:\t%u\n", bitmap_weight(dev->inuse_pgtbl,
							  dev->size_pgtbl));
	seq_printf(s, "size_pgtbl:\t%u\n", dev->size_pgtbl);

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
	dev->debug_cq = NULL;
	dev->debug_eq = NULL;
	dev->debug_mr = NULL;
	dev->debug_mw = NULL;
	dev->debug_pd = NULL;
	dev->debug_qp = NULL;
	dev->debug_srq = NULL;

	if (!parent)
		return;

	dev->debug = debugfs_create_dir("rdma", parent);
	if (!dev->debug)
		return;

	debugfs_create_file("info", 0220, dev->debug, dev,
			    &ionic_dev_info_fops);

	dev->debug_ah = debugfs_create_dir("ah", dev->debug);
	dev->debug_cq = debugfs_create_dir("cq", dev->debug);
	dev->debug_eq = debugfs_create_dir("eq", dev->debug);
	dev->debug_mr = debugfs_create_dir("mr", dev->debug);
	dev->debug_mw = debugfs_create_dir("mw", dev->debug);
	dev->debug_pd = debugfs_create_dir("pd", dev->debug);
	dev->debug_qp = debugfs_create_dir("qp", dev->debug);
	dev->debug_srq = debugfs_create_dir("srq", dev->debug);
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

	ionic_q_show(s, "q.",  &eq->q);
	seq_printf(s, "enable:\t%u\n", eq->enable);
	seq_printf(s, "armed:\t%u\n", eq->armed);
	seq_printf(s, "vec:\t%d\n", eq->vec);
	seq_printf(s, "cpu:\t%d\n", eq->cpu);
	seq_printf(s, "intr:\t%u\n", eq->intr);
	seq_printf(s, "irq:\t%u\n", eq->irq);
	seq_printf(s, "name:\t%s\n", eq->name);

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

	snprintf(name, sizeof(name), "%u", eq->vec);

	eq->debug = debugfs_create_dir(name, dev->debug_eq);
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

	seq_printf(s, "lkey:\t%u\n", mr->ibmr.lkey);
	seq_printf(s, "rkey:\t%u\n", mr->ibmr.rkey);

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

void ionic_dbgfs_add_mr(struct ionic_ibdev *dev, struct ionic_mr *mr)
{
	char name[8];

	mr->debug = NULL;

	if (!dev->debug_mr)
		return;

	snprintf(name, sizeof(name), "%u", mr->ibmr.lkey);

	mr->debug = debugfs_create_dir(name, dev->debug_mr);
	if (!mr->debug)
		return;

	debugfs_create_file("info", 0220, mr->debug, mr,
			    &ionic_mr_info_fops);

	if (mr->umem)
		debugfs_create_file("umem", 0220, mr->debug, mr,
				    &ionic_mr_umem_fops);
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
	seq_printf(s, "tbl_pos:\t%u\n", cq->tbl_pos);
	seq_printf(s, "tbl_order:\t%d\n", cq->tbl_order);

	if (cq->q.ptr)
		ionic_q_show(s, "", &cq->q);

	if (cq->umem)
		ionic_umem_show(s, "", cq->umem);

	seq_printf(s, "lkey:\t%d\n", cq->lkey);

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

	snprintf(name, sizeof(name), "%u", cq->lkey);

	cq->debug = debugfs_create_dir(name, dev->debug_cq);
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

static int ionic_qp_info_show(struct seq_file *s, void *v)
{
	struct ionic_qp *qp = s->private;

	seq_printf(s, "qpid:\t%u\n", qp->qpid);

	if (qp->has_sq) {
		if (qp->sq.ptr)
			ionic_q_show(s, "sq.", &qp->sq);

		if (qp->sq_umem)
			ionic_umem_show(s, "sq.", qp->sq_umem);

		seq_printf(s, "sq_is_hbm:\t%d\n", qp->sq_is_hbm);
		if (qp->sq_is_hbm) {
			seq_printf(s, "sq_hbm_order:\t%d\n", qp->sq_hbm_order);
			seq_printf(s, "sq_hbm_pgid:\t%d\n", qp->sq_hbm_pgid);
			seq_printf(s, "sq_hbm_addr:\t%#llx\n", (u64)qp->sq_hbm_addr);
		}
	}

	if (qp->has_rq) {
		if (qp->rq.ptr)
			ionic_q_show(s, "rq.", &qp->rq);

		if (qp->rq_umem)
			ionic_umem_show(s, "rq.", qp->rq_umem);
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
