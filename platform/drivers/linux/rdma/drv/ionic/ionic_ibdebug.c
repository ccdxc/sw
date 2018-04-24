#include <linux/debugfs.h>

#include "ionic_ibdebug.h"
#include "ionic_ibdev.h"

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
