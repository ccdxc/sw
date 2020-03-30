/* SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB */
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

#ifndef IONIC_H
#define IONIC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/endian.h>
#include <pthread.h>

#include <infiniband/driver.h>
#include <infiniband/verbs.h>
#include <infiniband/udma_barrier.h>

#include "ionic-abi.h"

#include "ionic_memory.h"
#include "ionic_queue.h"
#include "ionic_table.h"

#include <stdio.h>
#include <inttypes.h>
#include <stdatomic.h>
#include <stdint.h>

struct ionic_stats;
struct ionic_latencies;

#ifndef likely
#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)
#endif /* likely and unlikely */

struct list_node {
	struct list_node *next;
	struct list_node *prev;
};
struct list_head {
	struct list_node head;
};

static inline void list_node_init(struct list_node *node)
{
	node->next = node;
	node->prev = node;
}

static inline void list_head_init(struct list_head *head)
{
	list_node_init(&head->head);
}

static inline void list_del(struct list_node *node)
{
	node->next->prev = node->prev;
	node->prev->next = node->next;
}

static inline void list_del_init(struct list_node *node)
{
	list_del(node);
	list_node_init(node);
}

static inline void list_add_tail(struct list_head *head,
				 struct list_node *node)
{
	node->next = &head->head;
	node->prev = head->head.prev;
	head->head.prev->next = node;
	head->head.prev = node;
}

#define list_for_each_safe(hd, pt, nx, member) \
	for ((pt = container_of((hd)->head.next,	\
			       typeof(*(pt)),		\
			       member));		\
	     (nx = container_of(((pt)->member).next,	\
				 typeof(*(pt)),		\
				 member)),		\
	     ((&(pt)->member) != &(hd)->head);		\
	     (pt = (nx)))

#define IONIC_MIN_RDMA_VERSION	1
#define IONIC_MAX_RDMA_VERSION	1

#define IONIC_META_LAST ((void *)1ul)
#define IONIC_META_POSTED ((void *)2ul)

#define IONIC_CQ_GRACE 100

struct ionic_ctx {
	struct ibv_context	ibctx;

	int			spec;
	uint32_t		pg_shift;

	int			version;
	uint8_t			compat;
	uint8_t			opcodes;

	uint8_t			sq_qtype;
	uint8_t			rq_qtype;
	uint8_t			cq_qtype;

	uint8_t			max_stride;

	uint64_t		*dbpage;

	pthread_mutex_t		mut;
	struct ionic_tbl_root	qp_tbl;

	FILE			*dbg_file;
	struct ionic_stats	*stats;
	struct ionic_latencies	*lats;
};

struct ionic_cq {
	struct ibv_cq		ibcq;

	uint32_t		cqid;

	pthread_spinlock_t	lock;
	struct list_head	poll_sq;
	bool			flush;
	struct list_head	flush_sq;
	struct list_head	flush_rq;
	struct ionic_queue	q;
	bool			color;
	bool			deferred_arm;
	bool			deferred_arm_sol_only;
	bool			lockfree;
	int			reserve;
	uint16_t		arm_any_prod;
	uint16_t		arm_sol_prod;
};

struct ionic_sq_meta {
	uint64_t		wrid;
	uint32_t		len;
	uint16_t		seq;
	uint8_t			ibop;
	uint8_t			ibsts;
	bool			remote;
	bool			signal;
	bool			local_comp;
};

struct ionic_rq_meta {
	struct ionic_rq_meta	*next;
	uint64_t		wrid;
};

struct ionic_qp {
	struct verbs_qp		vqp;

	uint32_t		qpid;
	bool			has_sq;
	bool			has_rq;
	bool			lockfree;

	bool			sig_all;

	struct list_node	cq_poll_sq;
	struct list_node	cq_flush_sq;
	struct list_node	cq_flush_rq;

	pthread_spinlock_t	sq_lock;
	bool			sq_flush;
	bool			sq_flush_rcvd;
	struct ionic_queue	sq;
	void			*sq_cmb_ptr;
	struct ionic_sq_meta	*sq_meta;
	uint16_t		*sq_msn_idx;

	int			sq_spec;
	uint16_t		sq_old_prod;
	uint16_t		sq_msn_prod;
	uint16_t		sq_msn_cons;
	uint16_t		sq_cmb_prod;

	pthread_spinlock_t	rq_lock;
	bool			rq_flush;
	struct ionic_queue	rq;
	void			*rq_cmb_ptr;
	struct ionic_rq_meta	*rq_meta;
	struct ionic_rq_meta	*rq_meta_head;

	int			rq_spec;
	uint16_t		rq_old_prod;
	uint16_t		rq_cmb_prod;
};

struct ionic_ah {
	struct ibv_ah		ibah;
	uint32_t		ahid;
};

struct ionic_dev {
	struct verbs_device	vdev;
	int			abi_ver;
};

static inline struct ionic_dev *to_ionic_dev(struct ibv_device *ibdev)
{
	return container_of(ibdev, struct ionic_dev, vdev.device);
}

static inline struct ionic_ctx *to_ionic_ctx(struct ibv_context *ibctx)
{
	return container_of(ibctx, struct ionic_ctx, ibctx);
}

static inline struct ionic_cq *to_ionic_cq(struct ibv_cq *ibcq)
{
	return container_of(ibcq, struct ionic_cq, ibcq);
}

static inline struct ionic_qp *to_ionic_qp(struct ibv_qp *ibqp)
{
	return container_of(ibqp, struct ionic_qp, vqp.qp);
}

static inline struct ionic_ah *to_ionic_ah(struct ibv_ah *ibah)
{
	return container_of(ibah, struct ionic_ah, ibah);
}

static inline bool ionic_ibop_is_local(enum ibv_wr_opcode op)
{
	return op == IBV_WR_LOCAL_INV || op == IBV_WR_BIND_MW;
}

#ifndef IONIC_DEBUG
#define IONIC_DEBUG true
#endif

extern FILE *IONIC_DEBUG_FILE;

#define IONIC_DEFAULT_DEBUG_FILE stderr

#define _ionic_dbg(file, fmt, ...)					\
	fprintf(file, "%s:%d: " fmt "\n",				\
		__func__, __LINE__, ##__VA_ARGS__)

#define ionic_dbg(ctx, fmt, ...) do {					\
	if ((IONIC_DEBUG) && unlikely(ctx->dbg_file))			\
		_ionic_dbg(ctx->dbg_file, fmt, ##__VA_ARGS__);		\
} while (0)

#define ionic_err(fmt, ...)						\
	_ionic_dbg(IONIC_DEBUG_FILE, fmt, ##__VA_ARGS__)

static inline void ionic_dbg_xdump(struct ionic_ctx *ctx, const char *str,
				   const void *ptr, size_t size)
{
	const uint8_t *ptr8 = ptr;
	int i;

	if (!(IONIC_DEBUG) || likely(!ctx->dbg_file))
		return;

	for (i = 0; i < size; i += 8)
		_ionic_dbg(ctx->dbg_file,
			   "%s: %02x %02x %02x %02x %02x %02x %02x %02x", str,
			   ptr8[i + 0], ptr8[i + 1], ptr8[i + 2], ptr8[i + 3],
			   ptr8[i + 4], ptr8[i + 5], ptr8[i + 6], ptr8[i + 7]);
}

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
 * struct ionic_stats - Counters incremented by driver
 * @ring_cq_dbell:	Times rang cq doorbell to advance index
 * @arm_cq_any:		Times rang cq doorbell to arm any event
 * @arm_cq_sol:		Times rang cq doorbell to arm solicited only
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
 * @post_recv_cmb:	Total posted to rq in cmb
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
	bool		histogram;
	atomic_ullong	ring_cq_dbell;
	atomic_ullong	arm_cq_any;
	atomic_ullong	arm_cq_sol;
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
	atomic_ullong	post_recv_cmb;
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
static inline int ionic_fls(unsigned long val)
{
	/* Set all bits of lesser significance */
	val |= val >> 32;
	val |= val >> 16;
	val |= val >> 8;
	val |= val >> 4;
	val |= val >> 2;
	val |= val >> 1;
	/* Leave only the most significant bit set */
	val ^= val >> 1;
	/* The most significant is the least significant bit */
	return ffs(val);
}

#define _ionic_stat_add(stats, name, val)				\
	atomic_fetch_add_explicit(&((stats)->name), val,		\
				  memory_order_relaxed)

#define _ionic_stat_incr(stats, name)					\
	_ionic_stat_add(stats, name, 1);

#define ionic_stat_add(stats, name, val) do {				\
	if ((IONIC_STATS) && stats)					\
		_ionic_stat_add(stats, name, val);			\
} while (0)

#define ionic_stat_incr(stats, name)					\
	ionic_stat_add(stats, name, 1)

#define ionic_stat_incr_idx(stats, name, idx)				\
	ionic_stat_incr(stats, name[(idx)])

#define ionic_stat_incr_idx_fls(stats, name, idx) do {			\
	if ((IONIC_STATS) && stats && stats->histogram)			\
		_ionic_stat_incr(stats, name[ionic_fls((idx))]);	\
} while (0)

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

	fprintf(file, "counter statistics:\n");

	ionic_stat_print(file, stats, ring_cq_dbell);
	ionic_stat_print(file, stats, arm_cq_any);
	ionic_stat_print(file, stats, arm_cq_sol);
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
	ionic_stat_print(file, stats, post_recv_cmb);
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

#ifndef IONIC_LATS
#define IONIC_LATS true
#endif

//#define IONIC_LATS_TSC
#define IONIC_LATS_BINS		32
#define IONIC_LATS_OUTLIER	4

/**
 * struct ionic_latstat - One dimension of latency statistics
 * @min:		Mininum latency seen
 * @max:		Maximum latency seen after warmup
 * @accum:		Accumulated latency after warmup
 * @count:		Number of samples after warmup, for averaging
 * @warmup:		Number of samples discarded for warmup
 * @hist:		Histogram of samples, log scale
 */
struct ionic_latstat {
	unsigned long		min;
	unsigned long		max;
	unsigned long		accum;
	unsigned int		count;
	unsigned int		warmup;
	unsigned long		hist[IONIC_LATS_BINS];
};

/**
 * struct ionic_latencies - Latency statistics
 * @overhead:		Overhead per trace
 * @stamp:		Most recent timestamp
 * @application:	Duration between calls
 * @poll_cq_empty:	Duration of poll_cq empty cq
 * @poll_cq_compl:	Duration of poll_cq with completions
 * @post_send:		Duration of post_send
 * @post_recv:		Duration of post_recv
 *
 * Only single-thread supported to keep overhead low.
 */
struct ionic_latencies {
	unsigned long		overhead;
	unsigned long		stamp;
	bool			enable;
	bool			histogram;
	struct ionic_latstat	application;
	struct ionic_latstat	poll_cq_empty;
	struct ionic_latstat	poll_cq_compl;
	struct ionic_latstat	post_send;
	struct ionic_latstat	post_recv;
};

#ifdef IONIC_LATS_TSC
#define IONIC_LATS_UNITS "tsc"
static inline unsigned long ionic_lat_stamp(void) {
	unsigned hi, lo;

	asm volatile ("rdtsc" : "=a"(lo), "=d"(hi));

	return (unsigned long)lo | ((unsigned long)hi << 32);
}
#else
#define IONIC_LATS_UNITS "ns"
static inline unsigned long ionic_lat_stamp(void) {
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (unsigned long)ts.tv_sec * 1000000000 +
		(unsigned long)ts.tv_nsec;
}
#endif

static inline void __ionic_lat_trace(struct ionic_latencies *lats,
				     struct ionic_latstat *stat)
{
	unsigned long now, dif;

	now = ionic_lat_stamp();
	dif = now - lats->stamp - lats->overhead;
	lats->stamp = now;

	if (unlikely(!lats->enable))
		return;

	/* adjust minimum */
	if (!stat->min || dif < stat->min) {
		stat->min = dif;
		/* continuation of warmup */
		if (!stat->count) {
			++stat->warmup;
			return;
		}
	}

	/* end of warmup */
	if (!stat->count) {
		stat->accum = stat->min;
		++stat->count;
		--stat->warmup;
		return;
	}

	/* average and max exclude outliers */
	if (likely(dif < stat->min * IONIC_LATS_OUTLIER)) {
		stat->accum += dif;
		++stat->count;
		if (unlikely(dif > stat->max))
			stat->max = dif;
	}

	if (!lats->histogram)
		return;

	/* convert to log scale */
	if (unlikely(dif >> (IONIC_LATS_BINS - 1)))
		++stat->hist[IONIC_LATS_BINS - 1];
	else
		++stat->hist[ionic_fls(dif)];
}

#define ionic_lat_trace(lats, name) do {				\
	if ((IONIC_LATS) && lats)					\
		__ionic_lat_trace(lats, &(lats)->name);			\
} while (0)

static inline void ionic_lat_init(struct ionic_latencies *lats)
{
	unsigned long start, end;
	int i, count = 1000;

	if (!lats)
		return;

	lats->enable = true;
	/* warmup for measured loop */
	for (i = 0; i < count; ++i) {
		ionic_lat_trace(lats, application);
		ionic_lat_trace(lats, poll_cq_compl);
		ionic_lat_trace(lats, application);
		ionic_lat_trace(lats, post_recv);
		ionic_lat_trace(lats, application);
		ionic_lat_trace(lats, post_send);
		ionic_lat_trace(lats, application);
		ionic_lat_trace(lats, poll_cq_empty);
	}
	/* measured to determine overhead of tracing */
	start = ionic_lat_stamp();
	for (i = 0; i < count; ++i) {
		ionic_lat_trace(lats, application);
		ionic_lat_trace(lats, poll_cq_compl);
		ionic_lat_trace(lats, application);
		ionic_lat_trace(lats, post_recv);
		ionic_lat_trace(lats, application);
		ionic_lat_trace(lats, post_send);
		ionic_lat_trace(lats, application);
		ionic_lat_trace(lats, poll_cq_empty);
	}
	end = ionic_lat_stamp();

	memset(lats, 0, sizeof(*lats));

	/* There are eight traces in the body of the measured loop.
	 * Estimate the overhead more conservatively by dividing by 14.
	 */
	lats->overhead = (end - start) / count / 14;
	lats->stamp = end;
	lats->enable = false;
}

#define ionic_lat_enable(lats, v) do {					\
	if (lats)							\
		(lats)->enable = (v);					\
} while (0)

#define ionic_lat_print_idx(file, lats, name, idx) do {			\
	const unsigned long _val = (lats)->name.hist[idx];		\
	const unsigned long _range = 1ul << (idx);			\
	if (_val)							\
		fprintf(file, "%s[%lu..%lu%s]:\t%lu\n",			\
			#name, _range >> 1, _range - 1,			\
			IONIC_LATS_UNITS, _val);			\
} while (0)

#define ionic_lat_print_avg(file, lats, name) do {			\
	const unsigned long _min = (lats)->name.min;			\
	const unsigned long _max = (lats)->name.max;			\
	const unsigned long _accum = (lats)->name.accum;		\
	const unsigned int _count = (lats)->name.count;			\
	const unsigned int _warmup = (lats)->name.warmup;		\
	const unsigned long _avg = _count ? _accum / _count : 0;	\
	fprintf(file, "%s (count, warmup):\t%u,\t%u\n",			\
		#name, _count, _warmup);				\
	fprintf(file, "%s (min, max, avg):\t%lu%s,\t%lu%s,\t%lu%s\n",	\
		#name, _min, IONIC_LATS_UNITS, _max, IONIC_LATS_UNITS,	\
		_avg, IONIC_LATS_UNITS);				\
} while (0)

static inline void ionic_lats_print(FILE *file, struct ionic_latencies *lats)
{
	int i;

	if (!file || !lats)
		return;

	fprintf(file, "latency statistics:\n");

	fprintf(file, "overhead:\t%lu%s\n", lats->overhead, IONIC_LATS_UNITS);

	ionic_lat_print_avg(file, lats, application);
	for (i = 0; i < IONIC_LATS_BINS; ++i)
		ionic_lat_print_idx(file, lats, application, i);

	ionic_lat_print_avg(file, lats, poll_cq_empty);
	for (i = 0; i < IONIC_LATS_BINS; ++i)
		ionic_lat_print_idx(file, lats, poll_cq_empty, i);

	ionic_lat_print_avg(file, lats, poll_cq_compl);
	for (i = 0; i < IONIC_LATS_BINS; ++i)
		ionic_lat_print_idx(file, lats, poll_cq_compl, i);

	ionic_lat_print_avg(file, lats, post_send);
	for (i = 0; i < IONIC_LATS_BINS; ++i)
		ionic_lat_print_idx(file, lats, post_send, i);

	ionic_lat_print_avg(file, lats, post_recv);
	for (i = 0; i < IONIC_LATS_BINS; ++i)
		ionic_lat_print_idx(file, lats, post_recv, i);

}

#endif /* IONIC_H */
