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

#ifndef IONIC_H
#define IONIC_H

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <endian.h>
#include <pthread.h>

#include <infiniband/driver.h>
#include <util/udma_barrier.h>
#include <ccan/list.h>

#include "ionic-abi.h"

#include "ionic_dbg.h"
#include "ionic_memory.h"
#include "ionic_queue.h"
#include "table.h"

#define IONIC_MIN_RDMA_VERSION	0
#define IONIC_MAX_RDMA_VERSION	1

#define IONIC_META_LAST ((void *)1ul)
#define IONIC_META_POSTED ((void *)2ul)

struct ionic_ctx {
	struct verbs_context	vctx;

	bool			fallback;
	uint32_t		pg_shift;

	int			version;
	uint8_t			compat;
	uint8_t			opcodes;

	uint8_t			sq_qtype;
	uint8_t			rq_qtype;
	uint8_t			cq_qtype;

	uint64_t		*dbpage;

	pthread_mutex_t		mut;
	struct tbl_root		qp_tbl;
};

struct ionic_cq {
	struct ibv_cq		ibcq;

	uint32_t		cqid;

	pthread_spinlock_t	lock;
	struct list_head	poll_sq;
	struct list_head	flush_sq;
	struct list_head	flush_rq;
	struct ionic_queue	q;
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
};

struct ionic_rq_meta {
	struct ionic_rq_meta	*next;
	uint64_t		wrid;
	uint32_t		len; /* XXX byte_len must come from cqe */
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
	struct ionic_queue	sq;
	struct ionic_sq_meta	*sq_meta;
	uint16_t		*sq_msn_idx;
	uint16_t		sq_msn_prod;
	uint16_t		sq_msn_cons;
	uint16_t		sq_npg_cons;

	void			*sq_hbm_ptr;
	uint16_t		sq_hbm_prod;

	pthread_spinlock_t	rq_lock;
	bool			rq_flush;
	struct ionic_queue	rq;
	struct ionic_rq_meta	*rq_meta;
	struct ionic_rq_meta	*rq_meta_head;
};

struct ionic_ah {
	struct ibv_ah		ibah;
	uint32_t		ahid;
};

struct ionic_dev {
	struct verbs_device	vdev;
	uint8_t			abi_version;
};

static inline struct ionic_dev *to_ionic_dev(struct ibv_device *ibdev)
{
	return container_of(ibdev, struct ionic_dev, vdev.device);
}

static inline struct ionic_ctx *to_ionic_ctx(struct ibv_context *ibctx)
{
	return container_of(ibctx, struct ionic_ctx, vctx.context);
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

static inline int ionic_ibop_to_wc(enum ibv_wr_opcode op)
{
	switch (op) {
	case IBV_WR_SEND:
	case IBV_WR_SEND_WITH_IMM:
	case IBV_WR_SEND_WITH_INV:
		return IBV_WC_SEND;
	case IBV_WR_RDMA_WRITE:
	case IBV_WR_RDMA_WRITE_WITH_IMM:
		return IBV_WC_RDMA_WRITE;
	case IBV_WR_RDMA_READ:
		return IBV_WC_RDMA_READ;
	case IBV_WR_ATOMIC_CMP_AND_SWP:
		return IBV_WC_COMP_SWAP;
	case IBV_WR_ATOMIC_FETCH_AND_ADD:
		return IBV_WC_FETCH_ADD;
	case IBV_WR_BIND_MW:
		return IBV_WC_BIND_MW;
	case IBV_WR_LOCAL_INV:
		return IBV_WC_LOCAL_INV;
	default:
		return -1;
	}
}

#endif /* IONIC_H */
