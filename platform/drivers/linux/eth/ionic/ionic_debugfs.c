/*
 * Copyright 2017-2018 Pensando Systems, Inc.  All rights reserved.
 *
 * This program is free software; you may redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <linux/debugfs.h>
#include <linux/netdevice.h>

#include "ionic.h"
#include "ionic_bus.h"
#include "ionic_lif.h"

#ifdef CONFIG_DEBUG_FS

#ifdef DEBUGFS_TEST_API

static int blob_open(struct inode *inode, struct file *filp)
{
	filp->private_data = inode->i_private;
	return 0;
}

static ssize_t blob_read(struct file *filp, char __user *buffer,
			 size_t count, loff_t *ppos)
{
	struct debugfs_blob_wrapper *blob = filp->private_data;

	if (*ppos >= blob->size)
		return 0;
	if (*ppos + count > blob->size)
		count = blob->size - *ppos;

	if (copy_to_user(buffer, blob->data + *ppos, count))
		return -EFAULT;

	*ppos += count;

	return count;
}

static ssize_t blob_write(struct file *filp, const char __user *buffer,
			  size_t count, loff_t *ppos)
{
	struct debugfs_blob_wrapper *blob = filp->private_data;

	if (*ppos >= blob->size)
		return 0;
	if (*ppos + count > blob->size)
		count = blob->size - *ppos;

	if (copy_from_user(blob->data + *ppos, buffer, count))
		return -EFAULT;

	*ppos += count;

	return count;
}
static const struct file_operations blob_fops = {
	.owner = THIS_MODULE,
	.open = blob_open,
	.read = blob_read,
	.write = blob_write,
};

struct dentry *debugfs_create_blob(const char *name, umode_t mode,
				   struct dentry *parent,
				   struct debugfs_blob_wrapper *blob)
{
	return debugfs_create_file(name, mode | 0200, parent, blob,
				   &blob_fops);
}

#define NUM_SCRATCH_BUFS	16
#define SIZE_SCRATCH_BUF	(4 * PAGE_SIZE)

static void *scratch_bufs[NUM_SCRATCH_BUFS];
static dma_addr_t scratch_bufs_pa[NUM_SCRATCH_BUFS];
static struct debugfs_blob_wrapper scratch_bufs_blob[NUM_SCRATCH_BUFS];

static int scratch_bufs_alloc(struct device *dev)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(scratch_bufs); i++) {
		scratch_bufs[i] = dma_alloc_coherent(dev, SIZE_SCRATCH_BUF,
						     &scratch_bufs_pa[i],
						     GFP_KERNEL | GFP_DMA);
		if (!scratch_bufs[i])
			return -ENOMEM;
	}

	return 0;
}

static int scratch_bufs_add(struct device *dev, struct dentry *parent)
{
	struct dentry *scratch_dentry;
	unsigned int i;
	char name[32];
	int err;

	err = scratch_bufs_alloc(dev);
	if (err)
		return err;

	scratch_dentry = debugfs_create_dir("scratch_bufs", parent);
	if (IS_ERR_OR_NULL(scratch_dentry))
		return PTR_ERR(scratch_dentry);

	for (i = 0; i < ARRAY_SIZE(scratch_bufs); i++) {
		scratch_bufs_blob[i].data = scratch_bufs[i];
		scratch_bufs_blob[i].size = SIZE_SCRATCH_BUF;
		snprintf(name, sizeof(name), "0x%016llx", scratch_bufs_pa[i]);
		debugfs_create_blob(name, 0600, scratch_dentry,
			&scratch_bufs_blob[i]);
	}

	return 0;
}

static void scratch_bufs_free(struct device *dev)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(scratch_bufs); i++)
		dma_free_coherent(dev, SIZE_SCRATCH_BUF, scratch_bufs[i],
				  scratch_bufs_pa[i]);
}

#endif

static struct dentry *ionic_dir;

#define single(name) \
static int name##_open(struct inode *inode, struct file *f)	\
{								\
	return single_open(f, name##_show, inode->i_private);	\
}								\
								\
static const struct file_operations name##_fops = {		\
	.owner = THIS_MODULE,					\
	.open = name##_open,					\
	.read = seq_read,					\
	.llseek = seq_lseek,					\
	.release = single_release,				\
}

void ionic_debugfs_create(void)
{
	ionic_dir = debugfs_create_dir(DRV_NAME, NULL);
}

void ionic_debugfs_destroy(void)
{
	debugfs_remove_recursive(ionic_dir);
}

int ionic_debugfs_add_dev(struct ionic *ionic)
{
	struct dentry *dentry;
#ifdef DEBUGFS_TEST_API
	int err;
#endif

	dentry = debugfs_create_dir(ionic_bus_info(ionic), ionic_dir);
	if (IS_ERR_OR_NULL(dentry))
		return PTR_ERR(dentry);

	ionic->dentry = dentry;

#ifdef DEBUGFS_TEST_API
	err = scratch_bufs_add(ionic->dev, dentry);
	if (err)
		debugfs_remove_recursive(ionic->dentry);
	return err;
#else
	return 0;
#endif
}

void ionic_debugfs_del_dev(struct ionic *ionic)
{
#ifdef DEBUGFS_TEST_API
	scratch_bufs_free(ionic->dev);
#endif
	debugfs_remove_recursive(ionic->dentry);
}

static int bars_show(struct seq_file *seq, void *v)
{
	struct ionic *ionic = seq->private;
	struct ionic_dev_bar *bars = ionic->bars;
	unsigned int i;

	for (i = 0; i < IONIC_BARS_MAX; i++)
		if (bars[i].vaddr)
			seq_printf(seq, "BAR%d: len 0x%lx vaddr %pK bus_addr %pad\n",
				   i, bars[i].len, bars[i].vaddr,
				   &bars[i].bus_addr);

	return 0;
}
single(bars);

int ionic_debugfs_add_bars(struct ionic *ionic)
{
	return debugfs_create_file("bars", 0400, ionic->dentry,
				   ionic, &bars_fops) ? 0 : -ENOTSUPP;
}

static const struct debugfs_reg32 dev_cmd_regs[] = {
	{ .name = "signature", .offset = 0, },
	{ .name = "done", .offset = 4, },
	{ .name = "cmd.word[0]", .offset = 8, },
	{ .name = "cmd.word[1]", .offset = 12, },
	{ .name = "cmd.word[2]", .offset = 16, },
	{ .name = "cmd.word[3]", .offset = 20, },
	{ .name = "cmd.word[4]", .offset = 24, },
	{ .name = "cmd.word[5]", .offset = 28, },
	{ .name = "cmd.word[6]", .offset = 32, },
	{ .name = "cmd.word[7]", .offset = 36, },
	{ .name = "cmd.word[8]", .offset = 40, },
	{ .name = "cmd.word[9]", .offset = 44, },
	{ .name = "cmd.word[10]", .offset = 48, },
	{ .name = "cmd.word[11]", .offset = 52, },
	{ .name = "cmd.word[12]", .offset = 56, },
	{ .name = "cmd.word[13]", .offset = 60, },
	{ .name = "cmd.word[14]", .offset = 64, },
	{ .name = "cmd.word[15]", .offset = 68, },
	{ .name = "comp.word[0]", .offset = 72, },
	{ .name = "comp.word[1]", .offset = 76, },
	{ .name = "comp.word[2]", .offset = 80, },
	{ .name = "comp.word[3]", .offset = 84, },
};

static const struct debugfs_reg32 dev_cmd_db_regs[] = {
	{ .name = "v", .offset = 0, },
};

int ionic_debugfs_add_dev_cmd(struct ionic *ionic)
{
	struct device *dev = ionic->dev;
	struct debugfs_regset32 *dev_cmd_regset;
	struct dentry *dentry;

	dev_cmd_regset = devm_kzalloc(dev, sizeof(*dev_cmd_regset),
				      GFP_KERNEL);
	if (!dev_cmd_regset)
		return -ENOMEM;
	dev_cmd_regset->regs = dev_cmd_regs;
	dev_cmd_regset->nregs = ARRAY_SIZE(dev_cmd_regs);
	dev_cmd_regset->base = ionic->idev.dev_cmd;

	dentry = debugfs_create_regset32("dev_cmd", 0400,
					 ionic->dentry, dev_cmd_regset);
	if (IS_ERR_OR_NULL(dentry))
		return PTR_ERR(dentry);

	dev_cmd_regset = devm_kzalloc(dev, sizeof(*dev_cmd_regset),
				      GFP_KERNEL);
	if (!dev_cmd_regset)
		return -ENOMEM;
	dev_cmd_regset->regs = dev_cmd_db_regs;
	dev_cmd_regset->nregs = ARRAY_SIZE(dev_cmd_db_regs);
	dev_cmd_regset->base = ionic->idev.dev_cmd_db;

	dentry = debugfs_create_regset32("dev_cmd_db", 0400,
					 ionic->dentry, dev_cmd_regset);
	if (IS_ERR_OR_NULL(dentry))
		return PTR_ERR(dentry);

	return 0;
}

static int identity_show(struct seq_file *seq, void *v)
{
	struct ionic *ionic = seq->private;
	union identity *ident = ionic->ident;

	seq_printf(seq, "asic_type:        %s\n",
		   ionic_dev_asic_name(ident->dev.asic_type));
	seq_printf(seq, "asic_rev:         0x%x\n", ident->dev.asic_rev);
	seq_printf(seq, "serial_num:       %s\n", ident->dev.serial_num);
	seq_printf(seq, "fw_version:       %s\n", ident->dev.fw_version);
	seq_printf(seq, "nlifs:            %d\n", ident->dev.nlifs);
	seq_printf(seq, "ndbpgs_per_lif:   %d\n", ident->dev.ndbpgs_per_lif);
	seq_printf(seq, "nadminqs_per_lif: %d\n", ident->dev.nadminqs_per_lif);
	seq_printf(seq, "ntxqs_per_lif:    %d\n", ident->dev.ntxqs_per_lif);
	seq_printf(seq, "nrxqs_per_lif:    %d\n", ident->dev.nrxqs_per_lif);
	seq_printf(seq, "ncqs_per_lif:     %d\n", ident->dev.ncqs_per_lif);
	seq_printf(seq, "nrdmasqs_per_lif: %d\n", ident->dev.nrdmasqs_per_lif);
	seq_printf(seq, "nrdmarqs_per_lif: %d\n", ident->dev.nrdmarqs_per_lif);
	seq_printf(seq, "neqs_per_lif:     %d\n", ident->dev.neqs_per_lif);
	seq_printf(seq, "nintrs:           %d\n", ident->dev.nintrs);
	seq_printf(seq, "nucasts_per_lif:  %d\n", ident->dev.nucasts_per_lif);
	seq_printf(seq, "nmcasts_per_lif:  %d\n", ident->dev.nmcasts_per_lif);

	return 0;
}
single(identity);

int ionic_debugfs_add_ident(struct ionic *ionic)
{
	return debugfs_create_file("identity", 0400, ionic->dentry,
				   ionic, &identity_fops) ? 0 : -ENOTSUPP;
}

int ionic_debugfs_add_sizes(struct ionic *ionic)
{
	debugfs_create_u32("nlifs", 0400, ionic->dentry,
			   &ionic->ident->dev.nlifs);
	debugfs_create_u32("ntxqs_per_lif", 0400, ionic->dentry,
			   &ionic->ntxqs_per_lif);
	debugfs_create_u32("nrxqs_per_lif", 0400, ionic->dentry,
			   &ionic->nrxqs_per_lif);
	debugfs_create_u32("nintrs", 0400, ionic->dentry, &ionic->nintrs);

	return 0;
}

static int q_tail_show(struct seq_file *seq, void *v)
{
	struct queue *q = seq->private;

	seq_printf(seq, "%d\n", q->tail->index);

	return 0;
}
single(q_tail);

static int q_head_show(struct seq_file *seq, void *v)
{
	struct queue *q = seq->private;

	seq_printf(seq, "%d\n", q->head->index);

	return 0;
}
single(q_head);

static int cq_tail_show(struct seq_file *seq, void *v)
{
	struct cq *cq = seq->private;

	seq_printf(seq, "%d\n", cq->tail->index);

	return 0;
}
single(cq_tail);

static const struct debugfs_reg32 intr_ctrl_regs[] = {
	{ .name = "coal_init", .offset = 0, },
	{ .name = "mask", .offset = 4, },
	{ .name = "credits", .offset = 8, },
	{ .name = "mask_on_assert", .offset = 12, },
	{ .name = "coal_timer", .offset = 16, },
};

int ionic_debugfs_add_qcq(struct lif *lif, struct dentry *lif_dentry,
			  struct qcq *qcq)
{
	struct device *dev = lif->ionic->dev;
	struct dentry *qcq_dentry, *q_dentry, *cq_dentry, *intr_dentry;
	struct dentry *stats_dentry;
	struct queue *q = &qcq->q;
	struct cq *cq = &qcq->cq;
	struct intr *intr = &qcq->intr;
	struct debugfs_regset32 *intr_ctrl_regset;
	struct debugfs_blob_wrapper *desc_blob;

	qcq_dentry = debugfs_create_dir(q->name, lif_dentry);
	if (IS_ERR_OR_NULL(qcq_dentry))
		return PTR_ERR(qcq_dentry);

	debugfs_create_x32("total_size", 0400, qcq_dentry, &qcq->total_size);
	debugfs_create_x64("base_pa", 0400, qcq_dentry, &qcq->base_pa);

	q_dentry = debugfs_create_dir("q", qcq_dentry);
	if (IS_ERR_OR_NULL(q_dentry))
		return PTR_ERR(q_dentry);

	debugfs_create_u32("index", 0400, q_dentry, &q->index);
	debugfs_create_x64("base_pa", 0400, q_dentry, &q->base_pa);
	if (qcq->flags & QCQ_F_SG) {
		debugfs_create_x64("sg_base_pa", 0400, q_dentry,
				   &q->sg_base_pa);
		debugfs_create_u32("sg_desc_size", 0400, q_dentry,
				   &q->sg_desc_size);
	}
	debugfs_create_u32("num_descs", 0400, q_dentry, &q->num_descs);
	debugfs_create_u32("desc_size", 0400, q_dentry, &q->desc_size);
	debugfs_create_u32("pid", 0400, q_dentry, &q->pid);
	debugfs_create_u32("qid", 0400, q_dentry, &q->qid);
	debugfs_create_u32("qtype", 0400, q_dentry, &q->qtype);

	debugfs_create_file("tail", 0400, q_dentry, q, &q_tail_fops);
	debugfs_create_file("head", 0400, q_dentry, q, &q_head_fops);

	desc_blob = devm_kzalloc(dev, sizeof(*desc_blob), GFP_KERNEL);
	if (!desc_blob)
		return -ENOMEM;
	desc_blob->data = q->base;
	desc_blob->size = q->num_descs * q->desc_size;
	debugfs_create_blob("desc_blob", 0400, q_dentry, desc_blob);

	if (qcq->flags & QCQ_F_SG) {
		desc_blob = devm_kzalloc(dev, sizeof(*desc_blob), GFP_KERNEL);
		if (!desc_blob)
			return -ENOMEM;
		desc_blob->data = q->sg_base;
		desc_blob->size = q->num_descs * q->sg_desc_size;
		debugfs_create_blob("sg_desc_blob", 0400, q_dentry,
				    desc_blob);
	}

	if (qcq->flags & QCQ_F_TX_STATS) {
		stats_dentry = debugfs_create_dir("tx_stats", q_dentry);
		if (IS_ERR_OR_NULL(stats_dentry))
			return PTR_ERR(stats_dentry);

		debugfs_create_u64("dma_map_err", 0400, stats_dentry,
				   &qcq->stats.tx.dma_map_err);
		debugfs_create_u64("pkts", 0400, stats_dentry,
				   &qcq->stats.tx.pkts);
		debugfs_create_u64("bytes", 0400, stats_dentry,
				   &qcq->stats.tx.bytes);
		debugfs_create_u64("clean", 0400, stats_dentry,
				   &qcq->stats.tx.clean);
		debugfs_create_u64("drop", 0400, stats_dentry,
				   &qcq->stats.tx.drop);
		debugfs_create_u64("linearize", 0400, stats_dentry,
				   &qcq->stats.tx.linearize);
		debugfs_create_u64("stop", 0400, stats_dentry,
				   &qcq->stats.tx.stop);
		debugfs_create_u64("no_csum", 0400, stats_dentry,
				   &qcq->stats.tx.no_csum);
		debugfs_create_u64("csum", 0400, stats_dentry,
				   &qcq->stats.tx.csum);
		debugfs_create_u64("crc32_csum", 0400, stats_dentry,
				   &qcq->stats.tx.crc32_csum);
		debugfs_create_u64("tso", 0400, stats_dentry,
				   &qcq->stats.tx.tso);
		debugfs_create_u64("frags", 0400, stats_dentry,
				   &qcq->stats.tx.frags);
	}

	if (qcq->flags & QCQ_F_RX_STATS) {
		stats_dentry = debugfs_create_dir("rx_stats", q_dentry);
		if (IS_ERR_OR_NULL(stats_dentry))
			return PTR_ERR(stats_dentry);

		debugfs_create_u64("dma_map_err", 0400, stats_dentry,
				   &qcq->stats.rx.dma_map_err);
		debugfs_create_u64("pkts", 0400, stats_dentry,
				   &qcq->stats.rx.pkts);
		debugfs_create_u64("bytes", 0400, stats_dentry,
				   &qcq->stats.rx.bytes);
	}

	cq_dentry = debugfs_create_dir("cq", qcq_dentry);
	if (IS_ERR_OR_NULL(cq_dentry))
		return PTR_ERR(cq_dentry);

	debugfs_create_x64("base_pa", 0400, cq_dentry, &cq->base_pa);
	debugfs_create_u32("num_descs", 0400, cq_dentry, &cq->num_descs);
	debugfs_create_u32("desc_size", 0400, cq_dentry, &cq->desc_size);
	debugfs_create_u8("done_color", 0400, cq_dentry,
			  (u8 *)&cq->done_color);

	debugfs_create_file("tail", 0400, cq_dentry, cq, &cq_tail_fops);

	desc_blob = devm_kzalloc(dev, sizeof(*desc_blob), GFP_KERNEL);
	if (!desc_blob)
		return -ENOMEM;
	desc_blob->data = cq->base;
	desc_blob->size = cq->num_descs * cq->desc_size;
	debugfs_create_blob("desc_blob", 0400, cq_dentry, desc_blob);

	if (qcq->flags & QCQ_F_INTR) {
		intr_dentry = debugfs_create_dir("intr", qcq_dentry);
		if (IS_ERR_OR_NULL(intr_dentry))
			return PTR_ERR(intr_dentry);

		debugfs_create_u32("index", 0400, intr_dentry,
				   &intr->index);
		debugfs_create_u32("vector", 0400, intr_dentry,
				   &intr->vector);

		intr_ctrl_regset = devm_kzalloc(dev, sizeof(*intr_ctrl_regset),
						GFP_KERNEL);
		if (!intr_ctrl_regset)
			return -ENOMEM;
		intr_ctrl_regset->regs = intr_ctrl_regs;
		intr_ctrl_regset->nregs = ARRAY_SIZE(intr_ctrl_regs);
		intr_ctrl_regset->base = intr->ctrl;

		debugfs_create_regset32("intr_ctrl", 0400, intr_dentry,
					intr_ctrl_regset);
	}

	return 0;
}

static int netdev_show(struct seq_file *seq, void *v)
{
	struct net_device *netdev = seq->private;

	seq_printf(seq, "%s\n", netdev->name);

	return 0;
}
single(netdev);

int ionic_debugfs_add_lif(struct lif *lif)
{
	struct dentry *lif_dentry, *netdev_dentry;
	unsigned int i;
	int err;

	lif_dentry = debugfs_create_dir(lif->name, lif->ionic->dentry);
	if (IS_ERR_OR_NULL(lif_dentry))
		return PTR_ERR(lif_dentry);

	netdev_dentry = debugfs_create_file("netdev", 0400, lif_dentry,
					    lif->netdev, &netdev_fops);
	if (IS_ERR_OR_NULL(netdev_dentry))
		return PTR_ERR(netdev_dentry);

	err = ionic_debugfs_add_qcq(lif, lif_dentry, lif->adminqcq);
	if (err)
		return err;

	for (i = 0; i < lif->ntxqcqs; i++) {
		err = ionic_debugfs_add_qcq(lif, lif_dentry, lif->txqcqs[i]);
		if (err)
			return err;
	}

	for (i = 0; i < lif->nrxqcqs; i++) {
		err = ionic_debugfs_add_qcq(lif, lif_dentry, lif->rxqcqs[i]);
		if (err)
			return err;
	}

	return 0;
}

#endif
