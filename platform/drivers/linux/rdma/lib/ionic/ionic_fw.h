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

#ifndef IONIC_FW_H
#define IONIC_FW_H

/* common to all versions */

/* wqe scatter gather element */
struct ionic_sge {
	__be64				va;
	__be32				len;
	__be32				lkey;
};

/* admin queue mr type */
enum ionic_mr_flags {
	/* bits that determine mr access */
	IONIC_MRF_LOCAL_WRITE		= (1u << 0),
	IONIC_MRF_REMOTE_WRITE		= (1u << 1),
	IONIC_MRF_REMOTE_READ		= (1u << 2),
	IONIC_MRF_REMOTE_ATOMIC		= (1u << 3),
	IONIC_MRF_MW_BIND		= (1u << 4),
	IONIC_MRF_ZERO_BASED		= (1u << 5),
	IONIC_MRF_ON_DEMAND		= (1u << 6),
	IONIC_MRF_ACCESS_MASK		= (1u << 12) - 1,

	/* bits that determine mr type */
	IONIC_MRF_IS_MW			= (1u << 14),
	IONIC_MRF_INV_EN		= (1u << 15),

	/* base flags combinations for mr types */
	IONIC_MRF_USER_MR		= 0,
	IONIC_MRF_PHYS_MR		= IONIC_MRF_INV_EN,
	IONIC_MRF_MW_1			= IONIC_MRF_IS_MW,
	IONIC_MRF_MW_2			= IONIC_MRF_IS_MW | IONIC_MRF_INV_EN,
};

static inline int to_ionic_mr_flags(int access)
{
	int flags = 0;

	if (access & IBV_ACCESS_LOCAL_WRITE)
		flags |= IONIC_MRF_LOCAL_WRITE;

	if (access & IBV_ACCESS_REMOTE_READ)
		flags |= IONIC_MRF_REMOTE_READ;

	if (access & IBV_ACCESS_REMOTE_WRITE)
		flags |= IONIC_MRF_REMOTE_WRITE;

	if (access & IBV_ACCESS_REMOTE_ATOMIC)
		flags |= IONIC_MRF_REMOTE_ATOMIC;

	if (access & IBV_ACCESS_MW_BIND)
		flags |= IONIC_MRF_MW_BIND;

	if (access & IBV_ACCESS_ZERO_BASED)
		flags |= IONIC_MRF_ZERO_BASED;

	return flags;
}

/* cqe status indicated in status_length field when err bit is set */
enum ionic_status {
	IONIC_STS_OK,
	IONIC_STS_LOCAL_LEN_ERR,
	IONIC_STS_LOCAL_QP_OPER_ERR,
	IONIC_STS_LOCAL_PROT_ERR,
	IONIC_STS_WQE_FLUSHED_ERR,
	IONIC_STS_MEM_MGMT_OPER_ERR,
	IONIC_STS_BAD_RESP_ERR,
	IONIC_STS_LOCAL_ACC_ERR,
	IONIC_STS_REMOTE_INV_REQ_ERR,
	IONIC_STS_REMOTE_ACC_ERR,
	IONIC_STS_REMOTE_OPER_ERR,
	IONIC_STS_RETRY_EXCEEDED,
	IONIC_STS_RNR_RETRY_EXCEEDED,
	IONIC_STS_XRC_VIO_ERR,
};

static inline int ionic_to_ibv_status(int sts)
{
	switch (sts) {
	case IONIC_STS_OK:
		return IBV_WC_SUCCESS;
	case IONIC_STS_LOCAL_LEN_ERR:
		return IBV_WC_LOC_LEN_ERR;
	case IONIC_STS_LOCAL_QP_OPER_ERR:
		return IBV_WC_LOC_QP_OP_ERR;
	case IONIC_STS_LOCAL_PROT_ERR:
		return IBV_WC_LOC_PROT_ERR;
	case IONIC_STS_WQE_FLUSHED_ERR:
		return IBV_WC_WR_FLUSH_ERR;
	case IONIC_STS_MEM_MGMT_OPER_ERR:
		return IBV_WC_MW_BIND_ERR;
	case IONIC_STS_BAD_RESP_ERR:
		return IBV_WC_BAD_RESP_ERR;
	case IONIC_STS_LOCAL_ACC_ERR:
		return IBV_WC_LOC_ACCESS_ERR;
	case IONIC_STS_REMOTE_INV_REQ_ERR:
		return IBV_WC_REM_INV_REQ_ERR;
	case IONIC_STS_REMOTE_ACC_ERR:
		return IBV_WC_REM_ACCESS_ERR;
	case IONIC_STS_REMOTE_OPER_ERR:
		return IBV_WC_REM_OP_ERR;
	case IONIC_STS_RETRY_EXCEEDED:
		return IBV_WC_RETRY_EXC_ERR;
	case IONIC_STS_RNR_RETRY_EXCEEDED:
		return IBV_WC_RNR_RETRY_EXC_ERR;
	case IONIC_STS_XRC_VIO_ERR:
	default:
		return IBV_WC_GENERAL_ERR;
	}
}

/* fw abi v1 */

/* completion queue v1 cqe */
struct ionic_v1_cqe {
	union {
		struct {
			__u64		wqe_id;
			__be32		src_qpn_op;
			__u8		src_mac[6];
			__be16		pkey_index;
			__be32		imm_data_rkey;
		} recv;
		struct {
			__u8		rsvd[4];
			__be32		msg_msn;
			__u8		rsvd2[8];
			__u64		npg_wqe_id;
		} send;
	};
	__be32				status_length;
	__be32				qid_type_flags;
};

/* bits for cqe recv */
enum ionic_v1_cqe_src_qpn_bits {
	IONIC_V1_CQE_RECV_QPN_MASK	= 0xffffff,
	IONIC_V1_CQE_RECV_OP_SHIFT	= 24,

	IONIC_V1_CQE_RECV_OP_SEND	= 0,
	IONIC_V1_CQE_RECV_OP_SEND_INV	= 1,
	IONIC_V1_CQE_RECV_OP_SEND_IMM	= 2,
	IONIC_V1_CQE_RECV_OP_RDMA_IMM	= 3,
};

/* bits for cqe qid_type_flags */
enum ionic_v1_cqe_qtf_bits {
	IONIC_V1_CQE_COLOR		= (1u << 0),
	IONIC_V1_CQE_ERROR		= (1u << 1),
	IONIC_V1_CQE_RCVD_IPV4		= (1u << 2),
	IONIC_V1_CQE_RCVD_WITH_INV	= (1u << 3),
	IONIC_V1_CQE_RCVD_WITH_IMM	= (1u << 4),
	IONIC_V1_CQE_TYPE_SHIFT		= 5,
	IONIC_V1_CQE_TYPE_MASK		= 0x7,
	IONIC_V1_CQE_QID_SHIFT		= 8,

	IONIC_V1_CQE_TYPE_RECV		= 1,
	IONIC_V1_CQE_TYPE_SEND_MSN	= 2,
	IONIC_V1_CQE_TYPE_SEND_NPG	= 3,
};

static inline bool ionic_v1_cqe_color(struct ionic_v1_cqe *cqe)
{
	return !!(cqe->qid_type_flags & htobe32(IONIC_V1_CQE_COLOR));
}

static inline bool ionic_v1_cqe_error(struct ionic_v1_cqe *cqe)
{
	return !!(cqe->qid_type_flags & htobe32(IONIC_V1_CQE_ERROR));
}

static inline void ionic_v1_cqe_clean(struct ionic_v1_cqe *cqe)
{
	cqe->qid_type_flags |= htobe32(~0u << IONIC_V1_CQE_QID_SHIFT);
}

static inline uint32_t ionic_v1_cqe_qtf(struct ionic_v1_cqe *cqe)
{
	return be32toh(cqe->qid_type_flags);
}

static inline uint8_t ionic_v1_cqe_qtf_type(uint32_t qtf)
{
	return (qtf >> IONIC_V1_CQE_TYPE_SHIFT) & IONIC_V1_CQE_TYPE_MASK;
}

static inline uint32_t ionic_v1_cqe_qtf_qid(uint32_t qtf)
{
	return qtf >> IONIC_V1_CQE_QID_SHIFT;
}

/* v1 base wqe header */
struct ionic_v1_base_hdr {
	__u64				wqe_id;
	__u8				op;
	__u8				num_sge_key;
	__be16				flags;
	__be32				imm_data_key;
};

/* v1 receive wqe body */
struct ionic_v1_recv_bdy {
	__u8				rsvd[16]; /* XXX want sge here */
	struct ionic_sge		sgl[2];
};

/* v1 send/rdma wqe body (common, has sgl) */
struct ionic_v1_common_bdy {
	union {
		struct {
			__le32		ah_id;
			__be32		dest_qpn;
			__be32		dest_qkey;
		} send;
		struct {
			__be32		remote_va_high;
			__be32		remote_va_low;
			__be32		remote_rkey;
		} rdma;
	};
	__be32				length;
	union {
		__u8			data[32];
		struct ionic_sge	sgl[2];
	};
};

/* v1 atomic wqe body */
struct ionic_v1_atomic_bdy {
	__be32				remote_va_high;
	__be32				remote_va_low;
	__be32				remote_rkey;
	__be32				swap_add_high;
	__be32				swap_add_low;
	__be32				compare_high;
	__be32				compare_low;
	__u8				rsvd[4];
	struct ionic_sge		sge;
};

/* v1 bind mw wqe body */
struct ionic_v1_bind_mw_bdy {
	__be64				va;
	__be64				length;
	__be32				lkey;
	__be16				flags;
	__u8				rsvd[26];
};

/* v1 send/recv wqe */
struct ionic_v1_wqe {
	struct ionic_v1_base_hdr	base;
	union {
		struct ionic_v1_recv_bdy	recv;
		struct ionic_v1_common_bdy	common;
		struct ionic_v1_atomic_bdy	atomic;
		struct ionic_v1_bind_mw_bdy	bind_mw;
	};
};

/* queue pair v1 send opcodes */
enum ionic_v1_op {
	/* XXX during development, v0 and v1 ops can coexist, let v1 ops begin with opcode 16 */
	IONIC_V1_OP_SEND = 16,
	IONIC_V1_OP_SEND_IMM,
	IONIC_V1_OP_SEND_INV,
	IONIC_V1_OP_RDMA_READ,
	IONIC_V1_OP_RDMA_WRITE,
	IONIC_V1_OP_RDMA_WRITE_IMM,
	IONIC_V1_OP_ATOMIC_CS,
	IONIC_V1_OP_ATOMIC_FA,
	IONIC_V1_OP_REG_MR,
	IONIC_V1_OP_LOCAL_INV,
	IONIC_V1_OP_BIND_MW,

	/* flags */
	IONIC_V1_FLAG_FENCE		= (1u << 0),
	IONIC_V1_FLAG_SOL		= (1u << 1),
	IONIC_V1_FLAG_INL		= (1u << 2),
	IONIC_V1_FLAG_SIG		= (1u << 3),
};

static inline size_t ionic_v1_send_wqe_min_size(int min_sge, int min_data)
{
	size_t sz_wqe, sz_sgl, sz_data;

	sz_wqe = sizeof(struct ionic_v1_wqe);
	sz_sgl = offsetof(struct ionic_v1_wqe, common.sgl[min_sge]);
	sz_data = offsetof(struct ionic_v1_wqe, common.data[min_data]);

	if (sz_sgl > sz_wqe)
		sz_wqe = sz_sgl;

	if (sz_data > sz_wqe)
		sz_wqe = sz_data;

	return sz_wqe;
}

static inline int ionic_v1_send_wqe_max_sge(uint8_t stride_log2)
{
	struct ionic_v1_wqe *wqe = (void *)0;
	struct ionic_sge *sge = (void *)(1ull << stride_log2);

	return sge - wqe->common.sgl;
}

static inline int ionic_v1_send_wqe_max_data(uint8_t stride_log2)
{
	struct ionic_v1_wqe *wqe = (void *)0;
	__u8 *data = (void *)(1ull << stride_log2);

	return data - wqe->common.data;
}

static inline size_t ionic_v1_recv_wqe_min_size(int min_sge)
{
	size_t sz_wqe, sz_sgl;

	sz_wqe = sizeof(struct ionic_v1_wqe);
	sz_sgl = offsetof(struct ionic_v1_wqe, recv.sgl[min_sge]);

	if (sz_sgl > sz_wqe)
		sz_wqe = sz_sgl;

	return sz_wqe;
}

static inline int ionic_v1_recv_wqe_max_sge(uint8_t stride_log2)
{
	struct ionic_v1_wqe *wqe = (void *)0;
	struct ionic_sge *sge = (void *)(1ull << stride_log2);

	return sge - wqe->recv.sgl;
}

/* XXX to end of file: makeshift, will be removed */

#define OP_TYPE_RDMA_OPER_WITH_IMM	16
#define OP_TYPE_SEND_RCVD		17
#define OP_TYPE_INVALID			18

enum ionic_wr_opcode {
	IONIC_WR_OPCD_SEND		    = 0x00,
	IONIC_WR_OPCD_SEND_INVAL	    = 0x01,
	IONIC_WR_OPCD_SEND_IMM	    = 0x02,
	IONIC_WR_OPCD_RDMA_READ	    = 0x03,
	IONIC_WR_OPCD_RDMA_WRITE	    = 0x04,
	IONIC_WR_OPCD_RDMA_WRITE_IMM	= 0x05,
	IONIC_WR_OPCD_ATOMIC_CS	    = 0x06,
	IONIC_WR_OPCD_ATOMIC_FA	    = 0x07,
	IONIC_WR_OPCD_FRPNR    	    = 0x08,    
	IONIC_WR_OPCD_LOC_INVAL	    = 0x09,
	IONIC_WR_OPCD_BIND		    = 0x0a,
	IONIC_WR_OPCD_SEND_INV_IMM    = 0x0b,
    
	IONIC_WR_OPCD_INVAL		    = 0x0F
};

struct sqwqe_base_t {
    __u64 wrid;
    __u8  inline_data_vld    : 1;
    __u8  solicited_event    : 1;
    __u8  fence              : 1;
    __u8  complete_notify    : 1;
    __u8  op_type:4;
    __u8  num_sges;
    __u16 rsvd2;
}__attribute__ ((__packed__));

struct sqwqe_rc_send_t {
    __u32 imm_data;
    __u32 inv_key;
    __u32 rsvd1;
    __u32 length;
    __u32 rsvd2;
}__attribute__ ((__packed__));

struct sqwqe_ud_send_t {
    __u32 imm_data;
    __u32 q_key;
    __u32 length;
    __u32 dst_qp:24;
    __u32 ah_size:8;
    __u32 ah_handle;
}__attribute__ ((__packed__));

struct sqwqe_rdma_t {
    __u32 imm_data;
    __u64 va;
    __u32 length;
    __u32 r_key;
}__attribute__ ((__packed__));

struct sqwqe_atomic_t {
    __u32 r_key;
    __u64 va;
    __u64 swap_or_add_data;
    __u64 cmp_data;
    __u64 pad;
    struct ionic_sge sge;
}__attribute__ ((__packed__));

struct sqwqe_non_atomic_t {
    union {
        struct sqwqe_rc_send_t send;
        struct sqwqe_ud_send_t ud_send;
        struct sqwqe_rdma_t rdma;
    }wqe;
    struct ionic_sge sg_arr[2];
}__attribute__ ((__packed__));

struct sqwqe_t {
    struct sqwqe_base_t base;
    union {
        struct sqwqe_atomic_t atomic;
        struct sqwqe_non_atomic_t non_atomic;
    }u;
}__attribute__ ((__packed__));

#endif
