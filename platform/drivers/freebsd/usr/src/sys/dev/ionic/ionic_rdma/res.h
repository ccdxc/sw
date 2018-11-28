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

#ifndef RES_H
#define RES_H

/** struct resid_bits - Number allocator based on find_first_zero_bit.
 *
 * The allocator find_first_zero_bit suffers O(N^2) search time complexity,
 * for N allocations.  This is because it starts from the beginning of the
 * bitmap each time.  To find a free bit in the bitmap, the search time
 * increases for each allocation as the beginning of the bitmap is filled.  On
 * the other hand, it is desirable for O(1) memory size complexity, assuming
 * the capacity is constant.
 *
 * This allocator is intended to keep the desired memory size complexity, but
 * improve the search time complexity for typical workloads.  The search time
 * complexity is expected to be closer N, for N allocations, although it
 * remains bounded by O(N^2) in the worst case.
 */
struct resid_bits {
	int next_id;
	int inuse_size;
	unsigned long *inuse;
};

/** resid_init() - Initialize a resid allocator.
 * @resid:	Uninitialized resid allocator.
 * @size:	Capacity of the allocator.
 *
 * Return: Zero on success, or negative error number.
 */
int resid_init(struct resid_bits *resid, int size);

/** resid_destroy() - Destroy a resid allocator.
 * @resid:	Resid allocator.
 */
static inline void resid_destroy(struct resid_bits *resid)
{
	kfree(resid->inuse);
}

/** resid_get_shared() - Allocate an available shared resource id.
 * @resid:	Resid allocator.
 * @wrap_id:	Smallest valid resource id.
 * @next_id:	Start the search at resource id.
 * @size_id:	One after largest valid resource id.
 *
 * This does not update the next_id.  Caller should update the next_id for for
 * the resource that shares the id space, and/or the shared resid->next_id as
 * appropriate.
 *
 * Return: Resource id, or negative error number.
 */
int resid_get_shared(struct resid_bits *resid, int wrap_id,
		     int next_id, int size);

/** resid_get_wrap() - Allocate an available resource id, wrap to nonzero.
 * @resid:	Resid allocator.
 * @wrap_id:	Smallest valid resource id.
 *
 * Return: Resource id, or negative error number.
 */
static inline int resid_get_wrap(struct resid_bits *resid, int wrap_id)
{
	int rc;

	rc = resid_get_shared(resid, wrap_id,
			      resid->next_id,
			      resid->inuse_size);
	if (rc >= 0)
		resid->next_id = rc + 1;

	return rc;
}

/** resid_get() - Allocate an available resource id.
 * @resid:	Resid allocator.
 *
 * Return: Resource id, or negative error number.
 */
static inline int resid_get(struct resid_bits *resid)
{
	return resid_get_wrap(resid, 0);
}

/** resid_put() - Free a resource id.
 * @resid:	Resid allocator.
 * @id:		Resource id.
 */
static inline void resid_put(struct resid_bits *resid, int id)
{
	clear_bit(id, resid->inuse);
}

/** struct buddy_bits - Buddy allocator based on bitmap_find_free_region
 *
 * The allocator bitmap_find_free_region suffers O(N^2) search time complexity,
 * for N allocations.  This is because it starts from the beginning of the
 * bitmap each time.  To find a free range of bits in the bitmap, the search
 * time increases for each allocation as the beginning of the bitmap is filled.
 * On the other hand, it is desirable for O(1) memory size complexity, assuming
 * the capacity is constant.
 *
 * This allocator is intended to keep the desired memory size complexity, but
 * improve the search time complexity for typical workloads.  The search time
 * complexity is expected to be closer N, for N allocations, although it
 * remains bounded by O(N^2) in the worst case.
 */
struct buddy_bits {
	int inuse_size;
	int inuse_longs;
	unsigned long *inuse;
	int order_max;
	int *order_next;
};

/** buddy_init() - Initialize a buddy allocator.
 * @buddy:	Uninitialized buddy allocator.
 * @size:	Capacity of the allocator.
 *
 * Return: Zero on success, or negative error number.
 */
int buddy_init(struct buddy_bits *buddy, int size);

/** buddy_destroy() - Destroy a buddy allocator.
 * @buddy:	Buddy allocator.
 */
static inline void buddy_destroy(struct buddy_bits *buddy)
{
	kfree(buddy->order_next);
	kfree(buddy->inuse);
}

/** buddy_get() - Allocate an order-sized range.
 * @buddy:	Buddy allocator.
 * @order:	Log size of the range to get.
 *
 * Return: Position of the start of the range, or negative error number.
 */
int buddy_get(struct buddy_bits *buddy, int order);

/** buddy_put() - Free an order-sized range.
 * @buddy:	Buddy allocator.
 * @pos:	Position of the start of the range.
 * @order:	Log size of the range to put.
 */
void buddy_put(struct buddy_bits *buddy, int pos, int order);

#endif
