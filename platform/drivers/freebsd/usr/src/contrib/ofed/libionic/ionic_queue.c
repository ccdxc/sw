// SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB
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

#include <errno.h>
#include <string.h>

#include "ionic_queue.h"
#include "ionic_memory.h"

static int ionic_order_base2(size_t val)
{
	int order = 32;

	if (val <= 1)
		return 0;

	val -= 1;

	val |= val >> 1;
	val |= val >> 2;
	val |= val >> 4;
	val |= val >> 8;
	val |= val >> 16;

	/* leave zero, exactly one high bit set, or exactly one low bit set */
	val ^= val >> 1;

	if (sizeof(size_t) == 8)
		order += ffs(val >> 32);
	if (order == 32)
		order = ffs(val);

	return order;
}

int ionic_queue_init(struct ionic_queue *q, int pg_shift,
		     int depth, size_t stride)
{
	if (depth < 0 || depth > 0xffff)
		return -EINVAL;

	if (stride == 0 || stride > 0x10000)
		return -EINVAL;

	if (depth == 0)
		depth = 1;

	q->depth_log2 = ionic_order_base2(depth + 1);
	q->stride_log2 = ionic_order_base2(stride);

	if (q->depth_log2 + q->stride_log2 < pg_shift)
		q->depth_log2 = pg_shift - q->stride_log2;

	q->size = 1ull << (q->depth_log2 + q->stride_log2);
	q->mask = (1u << q->depth_log2) - 1;

	q->ptr = ionic_map_anon(q->size);
	if (!q->ptr)
		return errno;

	q->prod = 0;
	q->cons = 0;
	q->dbell = 0;

	return 0;
}

void ionic_queue_destroy(struct ionic_queue *q)
{
	ionic_unmap(q->ptr, q->size);
}
