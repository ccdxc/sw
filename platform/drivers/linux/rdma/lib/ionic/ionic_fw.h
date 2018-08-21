#ifndef IONIC_FW_H
#define IONIC_FW_H

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

/* queue pair v1 sge */
struct ionic_v1_sge {
	__le64				va;
	__le32				length;
	__le32				lkey;
};

/* queue pair v1 recv wqe */
struct ionic_v1_recv_wqe {
	uint8_t				op_rsvd;
	uint8_t				num_sge;
	uint16_t			wqe_id;
	__le32				length;
	uint8_t				rsvd[8];
	struct ionic_v1_sge		sgl[3];
};

/* queue pair v1 send wqe */
struct ionic_v1_send_wqe {
	uint8_t				op;
	uint8_t				num_sge_key;
	uint16_t			flags;
	__le32				length_key;
	union {
		struct {
			uint8_t		rsvd[8];
			__le32		ah_id;
			__be32		dest_qpn;
			__be32		dest_qkey;
			__be32		imm_data_rkey;
			union {
				uint8_t	data[32];
				struct ionic_v1_sge sgl[2];
			};
		} send;
		struct {
			uint8_t		rsvd[8];
			__be64		remote_va;
			__be32		remote_rkey;
			__be32		imm_data;
			union {
				uint8_t	data[32];
				struct ionic_v1_sge sgl[2];
			};
		} rdma;
		struct {
			uint8_t		rsvd[8];
			__be64		remote_va;
			__be32		remote_rkey;
			__be32		swap_add_high;
			__be32		swap_add_low;
			__be32		compare_high;
			__be32		compare_low;
			uint8_t		rsvd2[4];
			struct ionic_v1_sge sgl[1];
		} atomic;
		struct {
			__le64		va;
			__le64		length;
			__le64		offset;
			__le64		dma_addr;
			uint8_t		dir_size_log2;
			uint8_t		page_size_log2;
			uint8_t		rsvd[22];
		} reg_mr;
		struct {
			__le64		va;
			__le64		length;
			__le32		lkey;
			uint8_t		rsvd[36];
		} bind_mw;
	};
};

/* queue pair v1 send opcodes */
enum ionic_v1_op {
	IONIC_V1_OP_SEND,
	IONIC_V1_OP_SEND_IMM,
	IONIC_V1_OP_SEND_INV,
	IONIC_V1_OP_READ,
	IONIC_V1_OP_WRITE,
	IONIC_V1_OP_WRITE_IMM,
	IONIC_V1_OP_ATOMIC_CS,
	IONIC_V1_OP_ATOMIC_FA,
	IONIC_V1_OP_FAST_MR,
	IONIC_V1_OP_BIND_MW,
	IONIC_V1_OP_INVAL,
};

/* queue pair v1 cqe status */
enum ionic_v1_status {
	IONIC_V1_STS_OK,
	IONIC_V1_STS_LOCAL_LEN_ERR,
	IONIC_V1_STS_LOCAL_QP_OPER_ERR,
	IONIC_V1_STS_LOCAL_PROT_ERR,
	IONIC_V1_STS_WQE_FLUSHED_ERR,
	IONIC_V1_STS_MEM_MGMT_OPER_ERR,
	IONIC_V1_STS_BAD_RESP_ERR,
	IONIC_V1_STS_LOCAL_ACC_ERR,
	IONIC_V1_STS_REMOTE_INV_REQ_ERR,
	IONIC_V1_STS_REMOTE_ACC_ERR,
	IONIC_V1_STS_REMOTE_OPER_ERR,
	IONIC_V1_STS_RETRY_EXCEEDED,
	IONIC_V1_STS_RNR_RETRY_EXCEEDED,
	IONIC_V1_STS_XRC_VIO_ERR,
};

static inline size_t ionic_v1_send_wqe_min_size(int min_sge, int min_data)
{
	size_t sz_wqe, sz_sgl, sz_data;

	sz_wqe = sizeof(struct ionic_v1_send_wqe);
	sz_sgl = offsetof(struct ionic_v1_send_wqe, send.sgl[min_sge]);
	sz_data = offsetof(struct ionic_v1_send_wqe, send.data[min_data]);

	if (sz_sgl > sz_wqe)
		sz_wqe = sz_sgl;

	if (sz_data > sz_wqe)
		sz_wqe = sz_data;

	return sz_wqe;
}

static inline int ionic_v1_send_wqe_max_sge(uint8_t stride_log2)
{
	struct ionic_v1_send_wqe *wqe = (void *)0;
	struct ionic_v1_sge *sge = (void *)(1ull << stride_log2);

	return sge - wqe->send.sgl;
}

static inline int ionic_v1_send_wqe_max_data(uint8_t stride_log2)
{
	struct ionic_v1_send_wqe *wqe = (void *)0;
	uint8_t *data = (void *)(1ull << stride_log2);

	return data - wqe->send.data;
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
	struct ionic_v1_sge *sge = (void *)(1ull << stride_log2);

	return sge - wqe->sgl;
}

#endif
