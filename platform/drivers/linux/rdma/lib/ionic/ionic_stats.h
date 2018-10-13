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

#ifndef IONIC_STATS_H
#define IONIC_STATS_H

#include <inttypes.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>

#ifndef IONIC_STATS
#define IONIC_STATS true
#endif

/* Queue length can be 0..65535.  The histogram has 17 buckets.  There is one
 * bucket for zero, and one for each most significant bit set of 16 bits.
 */
#define IONIC_STATS_FLS	17

/* Valid opcodes are 0..255.  The histogram has 256 buckets. */
#define IONIC_STATS_OPS	256

/**
 * struct ionic_stats: Counters incremented by driver.
 * @poll_cq:		Times called poll_cq
 * @poll_cq_err:	Times returned error
 * @poll_cq_wc:		Total wc polled
 * @poll_cq_wc_err:	Total wc polled with error
 * @poll_cq_wc_flush:	Total wc flushed by driver
 * @poll_cq_cqe:	Total cqe (incl unsignaled)
 * @poll_cq_nwc:	Times polled batch of nwc (log)
 * @poll_cq_ncqe:	Times polled batch of ncqe (log)
 * @post_recv:		Timed called post_recv (or srq)
 * @post_recv_err:	Times returned error
 * @post_recv_wr:	Total recv wr posted
 * @post_recv_nwr:	Times posted batch of nwr (log)
 * @post_recv_qlen:	Qlen when called post_send (log)
 * @post_send:		Times called post_send
 * @post_send_err:	Times returned error
 * @post_send_wr:	Total send wr posted
 * @post_send_inl:	Total posted with inline data
 * @post_send_sig:	Total posted with signaled flag
 * @post_send_cmb:	Total posted to sq in cmb
 * @post_send_nwr:	Times posted batch of nwr (log)
 * @post_send_qlen:	Qlen when called post_send (log)
 * @post_send_op:	Times posted send opcode
 */
struct ionic_stats {
	atomic_ullong	poll_cq;
	atomic_ullong	poll_cq_err;
	atomic_ullong	poll_cq_wc;
	atomic_ullong	poll_cq_wc_err;
	atomic_ullong	poll_cq_wc_flush;
	atomic_ullong	poll_cq_cqe;
	atomic_ullong	poll_cq_nwc[IONIC_STATS_FLS];
	atomic_ullong	poll_cq_ncqe[IONIC_STATS_FLS];
	atomic_ullong	post_recv;
	atomic_ullong	post_recv_err;
	atomic_ullong	post_recv_wr;
	atomic_ullong	post_recv_nwr[IONIC_STATS_FLS];
	atomic_ullong	post_recv_qlen[IONIC_STATS_FLS];
	atomic_ullong	post_send;
	atomic_ullong	post_send_err;
	atomic_ullong	post_send_wr;
	atomic_ullong	post_send_inl;
	atomic_ullong	post_send_sig;
	atomic_ullong	post_send_cmb;
	atomic_ullong	post_send_nwr[IONIC_STATS_FLS];
	atomic_ullong	post_send_qlen[IONIC_STATS_FLS];
	atomic_ullong	post_send_op[IONIC_STATS_OPS];
};

/* Find the "last" or most significant bit that is set. */
static inline uint16_t ionic_fls(uint16_t val)
{
	/* Set all bits of lesser significance */
	val |= val >> 8;
	val |= val >> 4;
	val |= val >> 2;
	val |= val >> 1;
	/* Leave only the most significant bit set */
	val ^= val >> 1;
	/* The most significant is the least significant bit */
	return ffs(val);
}

#define ionic_stat_add(stats, name, val) do {				\
	if ((IONIC_STATS) && stats)					\
		atomic_fetch_add_explicit(&((stats)->name), val,	\
					  memory_order_relaxed);	\
} while (0)

#define ionic_stat_incr(stats, name)					\
	ionic_stat_add(stats, name, 1)

#define ionic_stat_incr_idx(stats, name, idx)				\
	ionic_stat_incr(stats, name[(idx)])

#define ionic_stat_incr_idx_fls(stats, name, idx)			\
	ionic_stat_incr(stats, name[ionic_fls((idx))])

#define ionic_stat_print(file, stats, name) do {			\
	const unsigned long long _val =					\
		atomic_load(&(stats)->name);				\
	fprintf(file, "%s:\t\t%llu\n", #name, _val);			\
} while (0)

#define ionic_stat_print_idx(file, stats, name, idx) do {		\
	const unsigned long long _val =					\
		atomic_load(&(stats)->name[(idx)]);			\
	if (_val)							\
		fprintf(file, "%s[%d]:\t%llu\n",			\
			#name, idx, _val);				\
} while (0)

#define ionic_stat_print_idx_fls(file, stats, name, idx) do {		\
	const unsigned long long _val =					\
		atomic_load(&(stats)->name[idx]);			\
	const int _range = 1 << (idx);					\
	if (_val)							\
		fprintf(file, "%s[%d..%d]:\t%llu\n",			\
			#name, _range >> 1, _range - 1, _val);		\
} while (0)

static inline void ionic_stats_print(FILE *file, struct ionic_stats *stats)
{
	int i;

	if (!file || !stats)
		return;

	ionic_stat_print(file, stats, poll_cq);
	ionic_stat_print(file, stats, poll_cq_err);
	ionic_stat_print(file, stats, poll_cq_wc);
	ionic_stat_print(file, stats, poll_cq_wc_err);
	ionic_stat_print(file, stats, poll_cq_cqe);
	for (i = 0; i < IONIC_STATS_FLS; ++i)
		ionic_stat_print_idx_fls(file, stats, poll_cq_nwc, i);
	for (i = 0; i < IONIC_STATS_FLS; ++i)
		ionic_stat_print_idx_fls(file, stats, poll_cq_ncqe, i);
	ionic_stat_print(file, stats, post_recv);
	ionic_stat_print(file, stats, post_recv_err);
	ionic_stat_print(file, stats, post_recv_wr);
	for (i = 0; i < IONIC_STATS_FLS; ++i)
		ionic_stat_print_idx_fls(file, stats, post_recv_nwr, i);
	for (i = 0; i < IONIC_STATS_FLS; ++i)
		ionic_stat_print_idx_fls(file, stats, post_recv_qlen, i);
	ionic_stat_print(file, stats, post_send);
	ionic_stat_print(file, stats, post_send_err);
	ionic_stat_print(file, stats, post_send_wr);
	ionic_stat_print(file, stats, post_send_inl);
	ionic_stat_print(file, stats, post_send_sig);
	ionic_stat_print(file, stats, post_send_cmb);
	for (i = 0; i < IONIC_STATS_FLS; ++i)
		ionic_stat_print_idx_fls(file, stats, post_send_nwr, i);
	for (i = 0; i < IONIC_STATS_FLS; ++i)
		ionic_stat_print_idx_fls(file, stats, post_send_qlen, i);
	for (i = 0; i < IONIC_STATS_OPS; ++i)
		ionic_stat_print_idx(file, stats, post_send_op, i);
}

#endif
