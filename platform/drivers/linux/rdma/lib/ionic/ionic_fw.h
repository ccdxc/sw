#ifndef IONIC_FW_H
#define IONIC_FW_H

/* completion queue v1 cqe */
struct ionic_v1_cqe {
	union {
		struct {
			u16		wqe_id;
			u8		op;
			u8		flags;
			__be32		src_qpn;
			u8		src_mac[6];
			__be16		pkey_index;
			u8		rsvd[4];
			__be32		imm_data_rkey;
		} recv;
	};
	union {
		__le32			status_length_cindex;
		__be32			msn;
	};
	__le32				qid_type_flags;
};

/* bits for cqe qid_type_flags */
enum ionic_v1_cqe_qtf_bits {
	IONIC_V1_CQE_COLOR		= BIT(31),
	IONIC_V1_CQE_ERROR		= BIT(30),
	IONIC_V1_CQE_TYPE_SHIFT		= 24,
	IONIC_V1_CQE_TYPE_MASK		= 0x3f,
	IONIC_V1_CQE_QID_MASK		= 0xffffff,

	IONIC_V1_CQE_TYPE_RECV		= 1,
	IONIC_V1_CQE_TYPE_SEND_MSN	= 2,
	IONIC_V1_CQE_TYPE_SEND_NPG	= 3,
};

static inline bool ionic_v1_cqe_color(struct ionic_v1_cqe *cqe)
{
	return !!(cqe->qid_type_flags & cpu_to_le32(IONIC_V1_CQE_COLOR));
}

static inline u32 ionic_v1_cqe_qtf(struct ionic_v1_cqe *cqe)
{
	return le32_to_cpu(cqe->qid_type_flags);
}

static inline bool ionic_v1_cqe_qtf_error(u32 qtf)
{
	return !!(qtf & IONIC_V1_CQE_ERROR);
}

static inline bool ionic_v1_cqe_qtf_type(u32 qtf)
{
	return (qtf >> IONIC_V1_CQE_TYPE_SHIFT) & IONIC_V1_CQE_TYPE_MASK;
}

static inline u32 ionic_v1_cqe_qtf_qid(u32 qtf)
{
	return qtf & IONIC_V1_CQE_QID_MASK;
}

/* queue pair v1 sge */
struct ionic_v1_sge {
	__le64				va;
	__le32				length;
	__le32				lkey;
};

/* queue pair v1 recv wqe */
struct ionic_v1_recv_wqe {
	u8				op_rsvd;
	u8				num_sge;
	u16				wqe_id;
	__le32				length;
	u8				rsvd[8];
	struct ionic_v1_sge		sgl[3];
};

/* queue pair v1 send wqe */
struct ionic_v1_send_wqe {
	u8				op;
	u8				num_sge_key;
	u16				flags;
	__le32				length_key;
	union {
		struct {
			u8		rsvd[8];
			__le32		ah_id;
			__be32		dest_qpn;
			__be32		dest_qkey;
			__be32		imm_data_rkey;
			union {
				u8	data[32];
				struct ionic_v1_sge sgl[2];
			};
		} send;
		struct {
			u8		rsvd[8];
			__be64		remote_va;
			__be32		remote_rkey;
			__be32		imm_data;
			union {
				u8	data[32];
				struct ionic_v1_sge sgl[2];
			};
		} rdma;
		struct {
			u8		rsvd[8];
			__be64		remote_va;
			__be32		remote_rkey;
			__be32		swap_add_high;
			__be32		swap_add_low;
			__be32		compare_high;
			__be32		compare_low;
			u8		rsvd2[4];
			struct ionic_v1_sge sgl[1];
		} atomic;
		struct {
			__le64		va;
			__le64		length;
			__le64		offset;
			__le64		dma_addr;
			u8		dir_size_log2;
			u8		page_size_log2;
			u8		rsvd[22];
		} reg_mr;
		struct {
			__le64		va;
			__le64		length;
			__le32		lkey;
			u8		rsvd[36];
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
	return max(sizeof(struct ionic_v1_send_wqe),
		   max(offsetof(struct ionic_v1_send_wqe, send.sgl[min_sge]),
		       offsetof(struct ionic_v1_send_wqe, send.data[min_data])));
}

static inline int ionic_v1_send_wqe_max_sge(u8 stride_log2)
{
	struct ionic_v1_send_wqe *wqe = (void *)0;
	struct ionic_v1_sge *sge = (void *)BIT_ULL(stride_log2);

	return sge - wqe->send.sgl;
}

static inline int ionic_v1_send_wqe_max_data(u8 stride_log2)
{
	struct ionic_v1_send_wqe *wqe = (void *)0;
	u8 *data = (void *)BIT_ULL(stride_log2);

	return data - wqe->send.data;
}

static inline size_t ionic_v1_recv_wqe_min_size(int min_sge)
{
	return max(sizeof(struct ionic_v1_recv_wqe),
		   offsetof(struct ionic_v1_recv_wqe, sgl[min_sge]));
}

static inline int ionic_v1_recv_wqe_max_sge(u8 stride_log2)
{
	struct ionic_v1_recv_wqe *wqe = (void *)0;
	struct ionic_v1_sge *sge = (void *)BIT_ULL(stride_log2);

	return sge - wqe->sgl;
}

#endif
