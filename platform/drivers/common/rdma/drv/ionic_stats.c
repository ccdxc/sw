// SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB
#ifdef __FreeBSD__
/*
 * Copyright (c) 2018-2020 Pensando Systems, Inc.  All rights reserved.
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
#else
/*
 * Copyright (c) 2018-2020 Pensando Systems, Inc.  All rights reserved.
 */
#endif /* __FreeBSD__ */

#include <linux/printk.h>

#include "ionic_fw.h"
#include "ionic_ibdev.h"

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

#define __ionic_v1_stat_validate(__type) do {		\
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
#ifdef __FreeBSD__
	dev->stats_buf = contig_kzalloc(PAGE_SIZE, GFP_KERNEL);
#else
	dev->stats_buf = kzalloc(PAGE_SIZE, GFP_KERNEL);
#endif
	if (!dev->stats_buf) {
		rc = -ENOMEM;
		goto err_buf;
	}

	/* buffer for names, sizes, offsets of values */
#ifdef __FreeBSD__
	dev->stats = contig_kzalloc(PAGE_SIZE, GFP_KERNEL);
#else
	dev->stats = kzalloc(PAGE_SIZE, GFP_KERNEL);
#endif
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
#ifdef __FreeBSD__
	contig_kfree(dev->stats, PAGE_SIZE);
#else
	kfree(dev->stats);
#endif
err_stats:
#ifdef __FreeBSD__
	contig_kfree(dev->stats_buf, PAGE_SIZE);
#else
	kfree(dev->stats_buf);
#endif
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

static const struct ib_device_ops ionic_stats_ops = {
#ifdef HAVE_RDMA_DEV_OPS_EXT
	.driver_id		= RDMA_DRIVER_IONIC,
#endif
	.alloc_hw_stats		= ionic_alloc_hw_stats,
	.get_hw_stats		= ionic_get_hw_stats,
};

void ionic_stats_setops(struct ionic_ibdev *dev)
{
	ib_set_device_ops(&dev->ibdev, &ionic_stats_ops);
}
