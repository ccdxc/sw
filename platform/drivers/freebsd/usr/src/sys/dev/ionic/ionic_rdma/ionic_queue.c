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

#include <linux/dma-mapping.h>

#include "ionic_kcompat.h"
#include "ionic_queue.h"

int ionic_queue_init(struct ionic_queue *q, struct device *dma_dev,
		     int depth, size_t stride)
{
	if (depth < 0 || depth > 0xffff)
		return -EINVAL;

	if (stride == 0 || stride > 0x10000)
		return -EINVAL;

	if (depth == 0)
		depth = 1;

	q->depth_log2 = order_base_2(depth + 1);
	q->stride_log2 = order_base_2(stride);

	if (q->depth_log2 + q->stride_log2 < PAGE_SHIFT)
		q->depth_log2 = PAGE_SHIFT - q->stride_log2;

#ifdef IONIC_STATIC_ANALYSIS_HINTS_NOT_FOR_UPSTREAM
	/* freebsd clang warns of bit shift by enormous amount in BIT_ULL */
	if (WARN_ON(q->depth_log2 > 16))
		return -EINVAL;
	if (WARN_ON(q->stride_log2 > 16))
		return -EINVAL;
#endif

	q->size = BIT_ULL(q->depth_log2 + q->stride_log2);
	q->mask = BIT(q->depth_log2) - 1;

	q->ptr = dma_zalloc_coherent(dma_dev, q->size, &q->dma, GFP_KERNEL);
	if (!q->ptr)
		return -ENOMEM;

	/* it will always be page aligned, but just to be sure... */
	if (WARN_ON(!PAGE_ALIGNED(q->ptr))) {
		dma_free_coherent(dma_dev, q->size, q->ptr, q->dma);
		return -ENOMEM;
	}

	q->prod = 0;
	q->cons = 0;
	q->dbell = 0;

	return 0;
}

void ionic_queue_destroy(struct ionic_queue *q, struct device *dma_dev)
{
	dma_free_coherent(dma_dev, q->size, q->ptr, q->dma);
}
