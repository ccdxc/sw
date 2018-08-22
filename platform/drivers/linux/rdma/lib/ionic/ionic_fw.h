#ifndef IONIC_FW_H
#define IONIC_FW_H

/* common to all versions */

/* wqe scatter gather element */
struct ionic_sge {
	__be64				va;
	__be32				len;
	__be32				lkey;
};

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

/* specific to version 1 */

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
	__le32				status_length;
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
struct ionic_v1_base_wqe {
	uint64_t			wqe_id;
	uint8_t				op;
	uint8_t				num_sge_key;
	__be16				flags;
	__be32				length_key;
};

/* v1 recv wqe */
struct ionic_v1_recv_wqe {
	struct ionic_v1_base_wqe	base;
	uint8_t				rsvd[16]; /* XXX want sge here */
	struct ionic_sge		sgl[2];
};

/* v1 send wqe */
struct ionic_v1_send_wqe {
	struct ionic_v1_base_wqe	base;
	union {
		struct {
			union {
				struct {
					__le32	ah_id;
					__be32	dest_qpn;
					__be32	dest_qkey;
					__be32	imm_data_rkey;
				} send;
				struct {
					__be64	remote_va;
					__be32	remote_rkey;
					__be32	imm_data;
				} rdma;
			};
			union {
				uint8_t	data[32];
				struct ionic_sge sgl[2];
			};
		} common;
		struct {
			__be64		remote_va;
			__be32		remote_rkey;
			__be32		swap_add_high;
			__be32		swap_add_low;
			__be32		compare_high;
			__be32		compare_low;
			uint8_t		rsvd[4];
			struct ionic_sge sge;
		} atomic;
		struct {
			__le64		va;
			__le64		length;
			__le64		offset;
			__le64		dma_addr;
			uint8_t		dir_size_log2;
			uint8_t		page_size_log2;
			uint8_t		rsvd[14];
		} reg_mr;
		struct {
			__le64		va;
			__le64		length;
			__le32		lkey;
			uint8_t		rsvd[28];
		} bind_mw;
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

	sz_wqe = sizeof(struct ionic_v1_send_wqe);
	sz_sgl = offsetof(struct ionic_v1_send_wqe, common.sgl[min_sge]);
	sz_data = offsetof(struct ionic_v1_send_wqe, common.data[min_data]);

	if (sz_sgl > sz_wqe)
		sz_wqe = sz_sgl;

	if (sz_data > sz_wqe)
		sz_wqe = sz_data;

	return sz_wqe;
}

static inline int ionic_v1_send_wqe_max_sge(uint8_t stride_log2)
{
	struct ionic_v1_send_wqe *wqe = (void *)0;
	struct ionic_sge *sge = (void *)(1ull << stride_log2);

	return sge - wqe->common.sgl;
}

static inline int ionic_v1_send_wqe_max_data(uint8_t stride_log2)
{
	struct ionic_v1_send_wqe *wqe = (void *)0;
	uint8_t *data = (void *)(1ull << stride_log2);

	return data - wqe->common.data;
}

static inline size_t ionic_v1_recv_wqe_min_size(int min_sge)
{
	size_t sz_wqe, sz_sgl;

	sz_wqe = sizeof(struct ionic_v1_recv_wqe);
	sz_sgl = offsetof(struct ionic_v1_recv_wqe, sgl[min_sge]);

	if (sz_sgl > sz_wqe)
		sz_wqe = sz_sgl;

	return sz_wqe;
}

static inline int ionic_v1_recv_wqe_max_sge(uint8_t stride_log2)
{
	struct ionic_v1_recv_wqe *wqe = (void *)0;
	struct ionic_sge *sge = (void *)(1ull << stride_log2);

	return sge - wqe->sgl;
}

/* XXX to end of file: makeshift, will be removed */

/* XXX makeshift, moved here from ionic-abi.h */

#define IONIC_FULL_FLAG_DELTA        0x80

#define IONIC_WR_OPCODE_MASK        0xF

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

#define IONIC_MAX_INLINE_SIZE		0x100

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

/* XXX makeshift, moved to here from ionic.h */

static inline uint8_t ibv_to_ionic_wr_opcd(uint8_t ibv_opcd)
{
	uint8_t bnxt_opcd;

	switch (ibv_opcd) {
	case IBV_WR_SEND:
		bnxt_opcd = IONIC_WR_OPCD_SEND;
		break;
	case IBV_WR_SEND_WITH_IMM:
		bnxt_opcd = IONIC_WR_OPCD_SEND_IMM;
		break;
	case IBV_WR_RDMA_WRITE:
		bnxt_opcd = IONIC_WR_OPCD_RDMA_WRITE;
		break;
	case IBV_WR_RDMA_WRITE_WITH_IMM:
		bnxt_opcd = IONIC_WR_OPCD_RDMA_WRITE_IMM;
		break;
	case IBV_WR_RDMA_READ:
		bnxt_opcd = IONIC_WR_OPCD_RDMA_READ;
		break;
	case IBV_WR_ATOMIC_CMP_AND_SWP:
		bnxt_opcd = IONIC_WR_OPCD_ATOMIC_CS;
		break;
	case IBV_WR_ATOMIC_FETCH_AND_ADD:
		bnxt_opcd = IONIC_WR_OPCD_ATOMIC_FA;
		break;
		/* TODO: Add other opcodes */
	default:
		bnxt_opcd = IONIC_WR_OPCD_INVAL;
		break;
	};

	return bnxt_opcd;
}

static inline void ionic_set_ibv_send_flags(int flags, struct sqwqe_t *wqe)
{
	if (flags & IBV_SEND_FENCE) {
		wqe->base.fence = 1;
	}
	if (flags & IBV_SEND_SOLICITED) {
		wqe->base.solicited_event = 1;
	}
	if (flags & IBV_SEND_INLINE) {
		wqe->base.inline_data_vld = 1;
	}
	if (flags & IBV_SEND_SIGNALED) {
		wqe->base.complete_notify = 1;
	}
}

static inline uint8_t ionic_ibv_wr_to_wc_opcd(uint8_t wr_opcd)
{
	uint8_t wc_opcd;

	switch (wr_opcd) {
	case IBV_WR_SEND_WITH_IMM:
	case IBV_WR_SEND:
		wc_opcd = IBV_WC_SEND;
		break;
	case IBV_WR_RDMA_WRITE_WITH_IMM:
	case IBV_WR_RDMA_WRITE:
		wc_opcd = IBV_WC_RDMA_WRITE;
		break;
	case IBV_WR_RDMA_READ:
		wc_opcd = IBV_WC_RDMA_READ;
		break;
	case IBV_WR_ATOMIC_CMP_AND_SWP:
		wc_opcd = IBV_WC_COMP_SWAP;
		break;
	case IBV_WR_ATOMIC_FETCH_AND_ADD:
		wc_opcd = IBV_WC_FETCH_ADD;
		break;
	default:
		wc_opcd = 0xFF;
		break;
	}

	return wc_opcd;
}

#define OP_TYPE_SEND			0
#define OP_TYPE_SEND_INV		1
#define OP_TYPE_SEND_IMM		2
#define OP_TYPE_READ			3
#define OP_TYPE_WRITE			4
#define OP_TYPE_WRITE_IMM		5
#define OP_TYPE_CMP_N_SWAP		6
#define OP_TYPE_FETCH_N_ADD		7
#define OP_TYPE_FRPNR			8
#define OP_TYPE_LOCAL_INV		9
#define OP_TYPE_BIND_MW			10
#define OP_TYPE_SEND_INV_IMM		11 // vendor specific

#define OP_TYPE_RDMA_OPER_WITH_IMM	16
#define OP_TYPE_SEND_RCVD		17
#define OP_TYPE_INVALID			18

#endif
