// SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB
/*
 * Copyright (c) 2018-2020 Pensando Systems, Inc.  All rights reserved.
 */

#include <linux/dma-mapping.h>

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
	unsigned int off = ionic_v1_stat_off(stat);

#define __ionic_v1_stat_validate(__type)		\
	((off + sizeof(__type) <= vals_len) &&		\
	 (IS_ALIGNED(off, sizeof(__type))))

	switch (type) {
	case IONIC_V1_STAT_TYPE_8:
		if (__ionic_v1_stat_validate(u8))
			return *(u8 *)(vals_buf + off);
		break;
	case IONIC_V1_STAT_TYPE_LE16:
		if (__ionic_v1_stat_validate(__le16))
			return le16_to_cpu(*(__le16 *)(vals_buf + off));
		break;
	case IONIC_V1_STAT_TYPE_LE32:
		if (__ionic_v1_stat_validate(__le32))
			return le32_to_cpu(*(__le32 *)(vals_buf + off));
		break;
	case IONIC_V1_STAT_TYPE_LE64:
		if (__ionic_v1_stat_validate(__le64))
			return le64_to_cpu(*(__le64 *)(vals_buf + off));
		break;
	case IONIC_V1_STAT_TYPE_BE16:
		if (__ionic_v1_stat_validate(__be16))
			return be16_to_cpu(*(__be16 *)(vals_buf + off));
		break;
	case IONIC_V1_STAT_TYPE_BE32:
		if (__ionic_v1_stat_validate(__be32))
			return be32_to_cpu(*(__be32 *)(vals_buf + off));
		break;
	case IONIC_V1_STAT_TYPE_BE64:
		if (__ionic_v1_stat_validate(__be64))
			return be64_to_cpu(*(__be64 *)(vals_buf + off));
		break;
	}

	return ~0ull;
#undef __ionic_v1_stat_validate
}

static int ionic_hw_stats_cmd(struct ionic_ibdev *dev,
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

	if (dev->admin_opcodes <= op)
		return -ENOSYS;

	ionic_admin_post(dev, &wr);

	return ionic_admin_wait(dev, &wr, IONIC_ADMIN_F_INTERRUPT);
}

static int ionic_hw_stats_hdrs_cmd(struct ionic_ibdev *dev,
				   dma_addr_t dma, size_t len)
{
	return ionic_hw_stats_cmd(dev, dma, len, IONIC_V1_ADMIN_STATS_HDRS);
}

static int ionic_hw_stats_vals_cmd(struct ionic_ibdev *dev,
				   dma_addr_t dma, size_t len)
{
	return ionic_hw_stats_cmd(dev, dma, len, IONIC_V1_ADMIN_STATS_VALS);
}

static int ionic_init_hw_stats(struct ionic_ibdev *dev)
{
	dma_addr_t hw_stats_dma;
	struct ionic_v1_stat *stat;
	int rc, hw_stat_i, hw_stats_count;

	if (dev->hw_stats_hdrs)
		return 0;

	dev->hw_stats_count = 0;

	/* buffer for current values from the device */
	dev->hw_stats_buf = kzalloc(PAGE_SIZE, GFP_KERNEL);
	if (!dev->hw_stats_buf) {
		rc = -ENOMEM;
		goto err_buf;
	}

	/* buffer for names, sizes, offsets of values */
	dev->hw_stats = kzalloc(PAGE_SIZE, GFP_KERNEL);
	if (!dev->hw_stats) {
		rc = -ENOMEM;
		goto err_hw_stats;
	}

	/* request the names, sizes, offsets */
	hw_stats_dma = dma_map_single(dev->hwdev, dev->hw_stats,
				      PAGE_SIZE, DMA_FROM_DEVICE);
	rc = dma_mapping_error(dev->hwdev, hw_stats_dma);
	if (rc)
		goto err_dma;

	rc = ionic_hw_stats_hdrs_cmd(dev, hw_stats_dma, PAGE_SIZE);
	if (rc)
		goto err_cmd;

	dma_unmap_single(dev->hwdev, hw_stats_dma, PAGE_SIZE, DMA_FROM_DEVICE);

	/* normalize and count the number of hw_stats */
	hw_stats_count = PAGE_SIZE / sizeof(*dev->hw_stats);
	for (hw_stat_i = 0; hw_stat_i < hw_stats_count; ++hw_stat_i) {
		stat = &dev->hw_stats[hw_stat_i];

		if (!ionic_v1_stat_normalize(stat))
			break;
	}

	if (!hw_stat_i) {
		rc = -ENOSYS;
		goto err_dma;
	}

	hw_stats_count = hw_stat_i;
	dev->hw_stats_count = hw_stat_i;

	/* alloc and init array of names, for alloc_hw_stats */
	dev->hw_stats_hdrs = kmalloc_array(hw_stats_count,
					   sizeof(*dev->hw_stats_hdrs),
					   GFP_KERNEL);
	if (!dev->hw_stats_hdrs) {
		rc = -ENOMEM;
		goto err_dma;
	}

	for (hw_stat_i = 0; hw_stat_i < hw_stats_count; ++hw_stat_i) {
		stat = &dev->hw_stats[hw_stat_i];
		dev->hw_stats_hdrs[hw_stat_i] = stat->name;
	}

	return 0;

err_cmd:
	dma_unmap_single(dev->hwdev, hw_stats_dma, PAGE_SIZE, DMA_FROM_DEVICE);
err_dma:
	kfree(dev->hw_stats);
err_hw_stats:
	kfree(dev->hw_stats_buf);
err_buf:
	dev->hw_stats_count = 0;
	dev->hw_stats = NULL;
	dev->hw_stats_buf = NULL;
	dev->hw_stats_hdrs = NULL;
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

	return rdma_alloc_hw_stats_struct(dev->hw_stats_hdrs,
					  dev->hw_stats_count,
					  RDMA_HW_STATS_DEFAULT_LIFESPAN);
}

static int ionic_get_hw_stats(struct ib_device *ibdev,
			      struct rdma_hw_stats *hw_stats,
			      u8 port, int index)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibdev);
	dma_addr_t hw_stats_dma;
	int rc, hw_stat_i;

	if (port != 1)
		return -EINVAL;

	hw_stats_dma = dma_map_single(dev->hwdev, dev->hw_stats_buf,
				      PAGE_SIZE, DMA_FROM_DEVICE);
	rc = dma_mapping_error(dev->hwdev, hw_stats_dma);
	if (rc)
		goto err_dma;

	rc = ionic_hw_stats_vals_cmd(dev, hw_stats_dma, PAGE_SIZE);
	if (rc)
		goto err_cmd;

	dma_unmap_single(dev->hwdev, hw_stats_dma,
			 PAGE_SIZE, DMA_FROM_DEVICE);

	for (hw_stat_i = 0; hw_stat_i < dev->hw_stats_count; ++hw_stat_i)
		hw_stats->value[hw_stat_i] =
			ionic_v1_stat_val(&dev->hw_stats[hw_stat_i],
					  dev->hw_stats_buf, PAGE_SIZE);

	return hw_stat_i;

err_cmd:
	dma_unmap_single(dev->hwdev, hw_stats_dma,
			 PAGE_SIZE, DMA_FROM_DEVICE);
err_dma:
	return rc;
}

static const struct ib_device_ops ionic_hw_stats_ops = {
#ifdef HAVE_RDMA_DEV_OPS_EXT
	.driver_id		= RDMA_DRIVER_IONIC,
#endif
	.alloc_hw_stats		= ionic_alloc_hw_stats,
	.get_hw_stats		= ionic_get_hw_stats,
};

void ionic_hw_stats_setops(struct ionic_ibdev *dev)
{
	ib_set_device_ops(&dev->ibdev, &ionic_hw_stats_ops);
}
