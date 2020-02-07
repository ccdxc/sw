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
#include <pthread.h>
#include <sys/endian.h>

#include <infiniband/driver.h>
#include <infiniband/verbs.h>
#include <infiniband/udma_barrier.h>

#include "ionic-abi.h"

#include "ionic_memory.h"
#include "ionic_queue.h"
#include "ionic_stats.h"
#include "ionic_table.h"
#include "list.h"

#define IONIC_MIN_RDMA_VERSION	1
#define IONIC_MAX_RDMA_VERSION	1

#define IONIC_META_LAST ((void *)1ul)
#define IONIC_META_POSTED ((void *)2ul)

#define IONIC_CQ_GRACE 100

struct ionic_ctx {
	struct ibv_context	ibctx;

	bool			lockfree;
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
	union {
		struct verbs_qp		vqp;
		struct verbs_srq	vsrq;
	};

	uint32_t		qpid;
	bool			has_sq;
	bool			has_rq;
	bool			is_srq;

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

static inline struct ionic_qp *to_ionic_srq(struct ibv_srq *ibsrq)
{
	return container_of(ibsrq, struct ionic_qp, vsrq.srq);
}

static inline struct ionic_ah *to_ionic_ah(struct ibv_ah *ibah)
{
        return container_of(ibah, struct ionic_ah, ibah);
}

static inline bool ionic_ibop_is_local(enum ibv_wr_opcode op)
{
	return op == IBV_WR_LOCAL_INV || op == IBV_WR_BIND_MW;
}

#endif /* IONIC_H */
