/* SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB */
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

#ifndef IONIC_QUEUE_H
#define IONIC_QUEUE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __FreeBSD__
#include <assert.h>
#include <stdatomic.h>
#include <sys/endian.h>

static inline void mmio_write64_le(uint64_t *reg, uint64_t val)
{
	atomic_store_explicit((_Atomic(uint64_t)*)reg, htole64(val),
			      memory_order_relaxed);
}

static inline void mmio_memcpy_x64(void *dst, void *src, size_t size)
{
	uint64_t *dst64 = dst, *src64 = src;

	assert(!(size & 63));

	for (size /= 64; size; --size) {
		*dst64++ = *src64++;
		*dst64++ = *src64++;
		*dst64++ = *src64++;
		*dst64++ = *src64++;
		*dst64++ = *src64++;
		*dst64++ = *src64++;
		*dst64++ = *src64++;
		*dst64++ = *src64++;
	}
}
#else
#include <util/mmio.h>
#include <util/udma_barrier.h>
#endif /* __FreeBSD__ */

#define IONIC_QID_MASK		((1ull << 24) - 1)
#define IONIC_DBELL_QID_SHIFT	24
#define IONIC_DBELL_RING_ARM	(1ull << 16)
#define IONIC_DBELL_RING_SONLY	(1ull << 17)

/** struct ionic_queue - Ring buffer used between device and driver.
 * @ptr:	Buffer virtual address.
 * @prod:	Driver position in the queue.
 * @cons:	Device position in the queue.
 * @mask:	Capacity of the queue, subtracting the hole.
 *		This value is eqal to ((1 << depth_log2) - 1).
 * @depth_log2: Log base two size depth of the queue.
 * @stride_log2: Log base two size of an element in the queue.
 * @dbell:	Doorbell identifying bits.
 */
struct ionic_queue {
	size_t size;
	void *ptr;
	uint16_t prod;
	uint16_t cons;
	uint16_t mask;
	uint8_t depth_log2;
	uint8_t stride_log2;
	uint64_t dbell;
};

/** ionic_queue_init - Initialize user space queue.
 * @q:		Uninitialized queue structure.
 * @pg_shift:	Host page shift for buffer size-alignment and mapping.
 * @depth:	Depth of the queue.
 * @stride:	Size of each element of the queue.
 *
 * Return: status code.
 */
int ionic_queue_init(struct ionic_queue *q, int pg_shift,
		     int depth, size_t stride);

/** ionic_queue_destroy - Destroy user space queue.
 * @q:		Queue structure.
 *
 * Return: status code.
 */
void ionic_queue_destroy(struct ionic_queue *q);

/** ionic_queue_empty - Test if queue is empty.
 * @q:		Queue structure.
 *
 * This is only valid for to-device queues.
 *
 * Return: is empty.
 */
static inline bool ionic_queue_empty(struct ionic_queue *q)
{
	return q->prod == q->cons;
}

/** ionic_queue_length - Get the current length of the queue.
 * @q:		Queue structure.
 *
 * This is only valid for to-device queues.
 *
 * Return: length.
 */
static inline uint16_t ionic_queue_length(struct ionic_queue *q)
{
	return (q->prod - q->cons) & q->mask;
}

/** ionic_queue_length_remaining - Get the remaining length of the queue.
 * @q:		Queue structure.
 *
 * This is only valid for to-device queues.
 *
 * Return: length remaining.
 */
static inline uint16_t ionic_queue_length_remaining(struct ionic_queue *q)
{
	return q->mask - ionic_queue_length(q);
}

/** ionic_queue_full - Test if queue is full.
 * @q:		Queue structure.
 *
 * This is only valid for to-device queues.
 *
 * Return: is full.
 */
static inline bool ionic_queue_full(struct ionic_queue *q)
{
	return q->mask == ionic_queue_length(q);
}

/** ionic_color_wrap - Flip the color if prod is wrapped.
 * @prod:	Queue index just after advancing.
 * @color:	Queue color just prior to advancing the index.
 *
 * Return: color after advancing the index.
 */
static inline bool ionic_color_wrap(uint16_t prod, bool color)
{
	/* logical xor color with (prod == 0) */
	return color != (prod == 0);
}

/** ionic_queue_at - Get the element at the given index.
 * @q:		Queue structure.
 * @idx:	Index in the queue.
 *
 * The index must be within the bounds of the queue.  It is not checked here.
 *
 * Return: pointer to element at index.
 */
static inline void *ionic_queue_at(struct ionic_queue *q, uint16_t idx)
{
	return q->ptr + ((unsigned long)idx << q->stride_log2);
}

/** ionic_queue_at_prod - Get the element at the producer index.
 * @q:		Queue structure.
 *
 * Return: pointer to element at producer index.
 */
static inline void *ionic_queue_at_prod(struct ionic_queue *q)
{
	return ionic_queue_at(q, q->prod);
}

/** ionic_queue_at_cons - Get the element at the consumer index.
 * @q:		Queue structure.
 *
 * Return: pointer to element at consumer index.
 */
static inline void *ionic_queue_at_cons(struct ionic_queue *q)
{
	return ionic_queue_at(q, q->cons);
}

/** ionic_queue_next - Compute the next index.
 * @q:		Queue structure.
 * @idx:	Index.
 *
 * Return: next index after idx.
 */
static inline uint16_t ionic_queue_next(struct ionic_queue *q, uint16_t idx)
{
	return (idx + 1) & q->mask;
}

/** ionic_queue_produce - Increase the producer index.
 * @q:		Queue structure.
 *
 * Caller must ensure that the queue is not full.  It is not checked here.
 */
static inline void ionic_queue_produce(struct ionic_queue *q)
{
	q->prod = ionic_queue_next(q, q->prod);
}

/** ionic_queue_consume - Increase the consumer index.
 * @q:		Queue structure.
 *
 * Caller must ensure that the queue is not empty.  It is not checked here.
 *
 * This is only valid for to-device queues.
 */
static inline void ionic_queue_consume(struct ionic_queue *q)
{
	q->cons = ionic_queue_next(q, q->cons);
}

/** ionic_queue_dbell_init - Initialize doorbell bits for queue id.
 * @q:		Queue structure.
 * @qid:	Queue identifying number.
 */
static inline void ionic_queue_dbell_init(struct ionic_queue *q,
					  uint32_t qid)
{
	q->dbell = ((uint64_t)qid & IONIC_QID_MASK) << IONIC_DBELL_QID_SHIFT;
}

/** ionic_queue_dbell_val - Get current doorbell update value.
 * @q:		Queue structure.
 */
static inline uint64_t ionic_queue_dbell_val(struct ionic_queue *q)
{
	return q->dbell | q->prod;
}

/** ionic_dbell_ring - Write the doorbell value to register.
 * @dbreg:	Doorbell register.
 * @val:	Doorbell value from queue.
 */
static inline void ionic_dbell_ring(uint64_t *dbreg, uint64_t val)
{
	mmio_write64_le(dbreg, htole64(val));
}

#endif /* IONIC_QUEUE_H */
