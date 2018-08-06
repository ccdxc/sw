#ifndef IONIC_FW_H
#define IONIC_FW_H

#include <linux/kernel.h>

/* completion queue v1 cqe */
struct ionic_v1_cqe {
	union {
		struct {
			__u8		rsvd[20];
			__le16		old_sq_cindex;
			__le16		old_rq_cq_cindex;
		} admin;
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
	IONIC_V1_CQE_COLOR		= BIT(0),
	IONIC_V1_CQE_ERROR		= BIT(1),
	IONIC_V1_CQE_RCVD_IPV4		= BIT(2),
	IONIC_V1_CQE_RCVD_WITH_INV	= BIT(3),
	IONIC_V1_CQE_RCVD_WITH_IMM	= BIT(4),
	IONIC_V1_CQE_TYPE_SHIFT		= 5,
	IONIC_V1_CQE_TYPE_MASK		= 0x7,
	IONIC_V1_CQE_QID_SHIFT		= 8,

	IONIC_V1_CQE_TYPE_ADMIN		= 0,
	IONIC_V1_CQE_TYPE_RECV		= 1,
	IONIC_V1_CQE_TYPE_SEND_MSN	= 2,
	IONIC_V1_CQE_TYPE_SEND_NPG	= 3,
};

static inline bool ionic_v1_cqe_color(struct ionic_v1_cqe *cqe)
{
	return !!(cqe->qid_type_flags & cpu_to_be32(IONIC_V1_CQE_COLOR));
}

static inline bool ionic_v1_cqe_error(struct ionic_v1_cqe *cqe)
{
	return !!(cqe->qid_type_flags & cpu_to_be32(IONIC_V1_CQE_ERROR));
}

static inline u32 ionic_v1_cqe_qtf(struct ionic_v1_cqe *cqe)
{
	return be32_to_cpu(cqe->qid_type_flags);
}

static inline u8 ionic_v1_cqe_qtf_type(u32 qtf)
{
	return (qtf >> IONIC_V1_CQE_TYPE_SHIFT) & IONIC_V1_CQE_TYPE_MASK;
}

static inline u32 ionic_v1_cqe_qtf_qid(u32 qtf)
{
	return qtf >> IONIC_V1_CQE_QID_SHIFT;
}

/* queue pair v1 sge */
struct ionic_v1_sge {
	__be64				va;
	__be32				length;
	__be32				lkey;
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
				struct ionic_v1_sge sgl[2];
				u8	data[32];
			};
		} send;
		struct {
			u8		rsvd[8];
			__be64		remote_va;
			__be32		remote_rkey;
			__be32		imm_data;
			union {
				struct ionic_v1_sge sgl[2];
				u8	data[32];
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
	size_t sz_wqe, sz_sgl;

	sz_wqe = sizeof(struct ionic_v1_recv_wqe);
	sz_sgl = offsetof(struct ionic_v1_recv_wqe, sgl[min_sge]);

	if (sz_sgl > sz_wqe)
		sz_wqe = sz_sgl;

	return sz_wqe;
}

static inline int ionic_v1_recv_wqe_max_sge(u8 stride_log2)
{
	struct ionic_v1_recv_wqe *wqe = (void *)0;
	struct ionic_v1_sge *sge = (void *)BIT_ULL(stride_log2);

	return sge - wqe->sgl;
}

/* admin queue v1 wqe */
struct ionic_v1_admin_wqe {
	u8				op;
	u8				type_state;
	__le16				dbid;
	__le32				id_ver;
	union {
		struct {
			__le64		dma_addr;
			__le32		length;
			u8		rsvd[44];
		} stats;
		struct {
			__le64		dma_addr;
			u8		rsvd[48];
		} ah;
		struct {
			__le32		lkey;
			__le32		rkey;
			__le64		va;
			__le64		length;
			__le64		offset;
			__le32		odp_id;
			__le16		access_flags;
			u8		dir_size_log2;
			u8		page_size_log2;
			__le32		tbl_index;
			__le32		map_count;
			__le64		dma_addr;
		} mr;
		struct {
			__le32		eq_id;
			u8		depth_log2;
			u8		stride_log2;
			u8		dir_size_log2_rsvd;
			u8		page_size_log2;
			u8		rsvd[32];
			__le32		tbl_index;
			__le32		map_count;
			__le64		dma_addr;
		} cq;
		struct {
			__le32		pd_id;
			__le16		access_perms_flags;
			__le16		access_perms_rsvd;
			__le32		sq_cq_id;
			u8		sq_depth_log2;
			u8		sq_stride_log2;
			u8		sq_dir_size_log2_rsvd;
			u8		sq_page_size_log2;
			__le32		sq_tbl_index_xrcd_id;
			__le32		sq_map_count;
			__le64		sq_dma_addr;
			__le32		rq_cq_id;
			u8		rq_depth_log2;
			u8		rq_stride_log2;
			u8		rq_dir_size_log2_rsvd;
			u8		rq_page_size_log2;
			__le32		rq_tbl_index_srq_id;
			__le32		rq_map_count;
			__le64		rq_dma_addr;
		} qp;
		struct {
			u8		pmtu;
			u8		retry;
			u8		rnr_timer;
			u8		retry_timeout;
			__le16		access_perms_flags;
			__le16		access_perms_mask;
			__le32		rq_psn;
			__le32		sq_psn;
			__le32		qkey_dest_qpn;
			__le32		rate_limit_kbps;
			u8		rsq_depth;
			u8		rrq_depth;
			__le16		pkey_id;
			__le32		ah_id_len;
			u8		rsvd[16];
			__le64		dma_addr;
		} mod_qp;
	};
};

/* side data for modify qp if resize */
struct ionic_v1_admin_resize_qp {
	u8				sq_depth_log2;
	u8				sq_stride_log2;
	u8				sq_dir_size_log2_rsvd;
	u8				sq_page_size_log2;
	u8				rq_depth_log2;
	u8				rq_stride_log2;
	u8				rq_dir_size_log2_rsvd;
	u8				rq_page_size_log2;
	__le32				sq_table_index;
	__le32				sq_map_count;
	__le64				sq_dma_addr;
	__le32				rq_table_index;
	__le32				rq_map_count;
	__le64				rq_dma_addr;
};

/* side data for query qp */
struct ionic_v1_admin_query_qp {
	u8				state_pmtu;
	u8				retry_rnrtry;
	u8				rnr_timer;
	u8				retry_timeout;
	__le16				access_perms_flags;
	__le16				access_perms_rsvd;
	__le32				rq_psn;
	__le32				sq_psn;
	__le32				qkey_dest_qpn;
	__le32				rate_limit_kbps;
	u8				rsq_depth;
	u8				rrq_depth;
	__le16				pkey_id;
	__le32				ah_id_len;
};

/* admin queue v1 opcodes */
enum ionic_v1_admin_op {
	IONIC_V1_ADMIN_NOOP,
	IONIC_V1_ADMIN_CREATE_CQ,


	/* TODO: move ops up as they are assigned and implemented */
	IONIC_V1_ADMIN_NOT_IMPLEMENTED = 100,
	IONIC_V1_ADMIN_CREATE_QP,
	IONIC_V1_ADMIN_STATS_HDRS,
	IONIC_V1_ADMIN_STATS_VALS,
	IONIC_V1_ADMIN_REG_MR,
	IONIC_V1_ADMIN_DEREG_MR,
	IONIC_V1_ADMIN_RESIZE_CQ,
	IONIC_V1_ADMIN_DESTROY_CQ,
	IONIC_V1_ADMIN_MODIFY_QP,
	IONIC_V1_ADMIN_QUERY_QP,
	IONIC_V1_ADMIN_DESTROY_QP,
};

/* admin queue v1 cqe status */
enum ionic_v1_admin_status {
	IONIC_V1_ASTS_OK,
	IONIC_V1_ASTS_BAD_INDEX,
	IONIC_V1_ASTS_BAD_STATE,
	IONIC_V1_ASTS_BAD_TYPE,
	IONIC_V1_ASTS_BAD_ATTR,
};

/* admin queue v1 mr type */
enum ionic_v1_mr_type {
	IONIC_V1_MRT_REG_MR,
	IONIC_V1_MRT_PHYS_MR,
	IONIC_V1_MRT_ODP_MR,
	IONIC_V1_MRT_MW_1,
	IONIC_V1_MRT_MW_2A,
	IONIC_V1_MRT_MW_2B,
};

/* admin queue v1 qp type */
enum ionic_v1_qp_type {
	IONIC_V1_QPT_RC,
	IONIC_V1_QPT_UC,
	IONIC_V1_QPT_UD,
	IONIC_V1_QPT_RD,
	IONIC_V1_QPT_SRQ,
	IONIC_V1_QPT_XRC_INI,
	IONIC_V1_QPT_XRC_TGT,
	IONIC_V1_QPT_XRC_SRQ,
};

/* admin queue v1 qp state */
enum ionic_v1_qp_state {
	IONIC_V1_QPS_RESET,
	IONIC_V1_QPS_INIT,
	IONIC_V1_QPS_RTR,
	IONIC_V1_QPS_RTS,
	IONIC_V1_QPS_SQD,
	IONIC_V1_QPS_SQE,
	IONIC_V1_QPS_ERR,
};

/* event queue v1 eqe */
struct ionic_v1_eqe {
	__be32				evt;
};

/* bits for cqe queue_type_flags */
enum ionic_v1_eqe_evt_bits {
	IONIC_V1_EQE_COLOR		= BIT(0),
	IONIC_V1_EQE_TYPE_SHIFT		= 1,
	IONIC_V1_EQE_TYPE_MASK		= 0x7,
	IONIC_V1_EQE_CODE_SHIFT		= 4,
	IONIC_V1_EQE_CODE_MASK		= 0xf,
	IONIC_V1_EQE_QID_SHIFT		= 8,

	/* cq events */
	IONIC_V1_EQE_TYPE_CQ		= 0,
	/* cq normal events */
	IONIC_V1_EQE_CQ_NOTIFY		= 0,
	/* cq error events */
	IONIC_V1_EQE_CQ_ERR		= 8,

	/* qp and srq events */
	IONIC_V1_EQE_TYPE_QP		= 1,
	/* qp normal events */
	IONIC_V1_EQE_SRQ_LEVEL		= 0,
	IONIC_V1_EQE_SQ_DRAIN		= 1,
	IONIC_V1_EQE_QP_COMM_EST	= 2,
	IONIC_V1_EQE_QP_LAST_WQE	= 3,
	/* qp error events */
	IONIC_V1_EQE_QP_ERR		= 8,
	IONIC_V1_EQE_QP_ERR_NOCQE	= 9,
	IONIC_V1_EQE_QP_ERR_REQUEST	= 10,
	IONIC_V1_EQE_QP_ERR_ACCESS	= 11,
};

static inline bool ionic_v1_eqe_color(struct ionic_v1_eqe *eqe)
{
	return !!(eqe->evt & cpu_to_be32(IONIC_V1_EQE_COLOR));
}

static inline u32 ionic_v1_eqe_evt(struct ionic_v1_eqe *eqe)
{
	return be32_to_cpu(eqe->evt);
}

static inline u8 ionic_v1_eqe_evt_type(u32 evt)
{
	return (evt >> IONIC_V1_EQE_TYPE_SHIFT) & IONIC_V1_EQE_TYPE_MASK;
}

static inline u8 ionic_v1_eqe_evt_code(u32 evt)
{
	return (evt >> IONIC_V1_EQE_CODE_SHIFT) & IONIC_V1_EQE_CODE_MASK;
}

static inline u32 ionic_v1_eqe_evt_qid(u32 evt)
{
	return evt >> IONIC_V1_EQE_QID_SHIFT;
}

/* --- below: from makeshift driver --- */

#define IONIC_NUM_RSQ_WQE         4
#define IONIC_NUM_RRQ_WQE         4

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
#define OP_TYPE_REG_MR			20

#define CQ_STATUS_SUCCESS		0
#define CQ_STATUS_LOCAL_LEN_ERR		1
#define CQ_STATUS_LOCAL_QP_OPER_ERR	2
#define CQ_STATUS_LOCAL_PROT_ERR	3
#define CQ_STATUS_WQE_FLUSHED_ERR	4
#define CQ_STATUS_MEM_MGMT_OPER_ERR	5
#define CQ_STATUS_BAD_RESP_ERR		6
#define CQ_STATUS_LOCAL_ACC_ERR		7
#define CQ_STATUS_REMOTE_INV_REQ_ERR	8
#define CQ_STATUS_REMOTE_ACC_ERR	9
#define CQ_STATUS_REMOTE_OPER_ERR	10
#define CQ_STATUS_RETRY_EXCEEDED	11
#define CQ_STATUS_RNR_RETRY_EXCEEDED	12
#define CQ_STATUS_XRC_VIO_ERR		13

enum ionic_qp_type {
	IONIC_QP_TYPE_RC = 0,
	IONIC_QP_TYPE_UC,
	IONIC_QP_TYPE_RD,
	IONIC_QP_TYPE_UD,
	IONIC_QP_TYPE_MAX = 255,
};

static inline enum ionic_qp_type ib_qp_type_to_ionic(enum ib_qp_type ibtype)
{
	switch (ibtype) {
	case IB_QPT_RC:
		return IONIC_QP_TYPE_RC;
	case IB_QPT_UC:
		return IONIC_QP_TYPE_UC;
	case IB_QPT_UD:
		return IONIC_QP_TYPE_UD;
	default:
		return IONIC_QP_TYPE_MAX;
	}
}

struct sge_t {
	__be64 va;
	__be32 len;
	__be32 lkey;
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
	__be32 imm_data;
	__be32 inv_key;
	__u32 rsvd1;
	__be32 length;
	__u32 rsvd2;
}__attribute__ ((__packed__));

struct sqwqe_ud_send_t {
	__be32 imm_data;
	__u32 q_key;
	__be32 length;
	__u32 dst_qp:24;
	__u32 ah_size:8;
	__u32 ah_handle;
}__attribute__ ((__packed__));

struct sqwqe_rdma_t {
	__be32 imm_data;
	__be64 va;
	__be32 length;
	__be32 r_key;
}__attribute__ ((__packed__));

struct sqwqe_atomic_t {
	__be32 r_key;
	__be64 va;
	__be64 swap_or_add_data;
	__be64 cmp_data;
	__u64 pad;
	struct sge_t sge;
}__attribute__ ((__packed__));

struct sqwqe_non_atomic_t {
	union {
		struct sqwqe_rc_send_t send;
		struct sqwqe_ud_send_t ud_send;
		struct sqwqe_rdma_t rdma;
	}wqe;
	union {
		struct sge_t sg_arr[2];
		u8 sg_data[32];
	};
}__attribute__ ((__packed__));

struct sqwqe_t {
	struct sqwqe_base_t base;
	union {
		struct sqwqe_atomic_t atomic;
		struct sqwqe_non_atomic_t non_atomic;
	}u;
}__attribute__ ((__packed__));

struct rqwqe_t {
	__u64        wrid;
	__u8         num_sges;
	__u8         rsd[23];
	struct sge_t sge_arr[2];
}__attribute__((__packed__));

static inline u16 ionic_sq_wqe_size(u16 min_sge, u16 min_inline)
{
	size_t sz_wqe, sz_sgl, sz_data;

	sz_wqe = sizeof(struct sqwqe_t);
	sz_sgl = offsetof(struct sqwqe_t, u.non_atomic.sg_arr[min_sge]);
	sz_data = offsetof(struct sqwqe_t, u.non_atomic.sg_data[min_inline]);

	if (sz_sgl > sz_wqe)
		sz_wqe = sz_sgl;

	if (sz_data > sz_wqe)
		sz_wqe = sz_data;

	return sz_wqe;
}

static inline int ionic_sq_wqe_max_sge(u16 wqe_size)
{
	struct sqwqe_t *wqe = (void *)0;
	struct sge_t *sge = (void *)(unsigned long)wqe_size;

	return sge - wqe->u.non_atomic.sg_arr;
}

static inline int ionic_sq_wqe_max_inline(u16 wqe_size)
{
	struct sqwqe_t *wqe = (void *)0;
	u8 *data = (void *)(unsigned long)wqe_size;

	return data - wqe->u.non_atomic.sg_data;
}

static inline u16 ionic_rq_wqe_size(u16 min_sge)
{
	size_t sz_wqe, sz_sgl;

	sz_wqe = sizeof(struct rqwqe_t);
	sz_sgl = offsetof(struct rqwqe_t, sge_arr[min_sge]);

	if (sz_sgl > sz_wqe)
		sz_wqe = sz_sgl;

	return sz_wqe;
}

static inline int ionic_rq_wqe_max_sge(u16 wqe_size)
{
	struct rqwqe_t *wqe = (void *)0;
	struct sge_t *sge = (void *)(unsigned long)wqe_size;

	return sge - wqe->sge_arr;
}

static inline bool ionic_op_is_local(uint8_t opcd)
{
	return opcd == OP_TYPE_LOCAL_INV || opcd == OP_TYPE_REG_MR;
}

static inline enum ib_wc_opcode ionic_to_ib_wc_opcd(uint8_t ionic_opcd)
{
	enum ib_wc_opcode ib_opcd;

	switch (ionic_opcd) {
	case OP_TYPE_SEND:
	case OP_TYPE_SEND_INV:
	case OP_TYPE_SEND_IMM:
		ib_opcd = IB_WC_SEND;
		break;
	case OP_TYPE_READ:
		ib_opcd = IB_WC_RDMA_READ;
		break;
	case OP_TYPE_WRITE:
	case OP_TYPE_WRITE_IMM:
		ib_opcd = IB_WC_RDMA_WRITE;
		break;
	case OP_TYPE_CMP_N_SWAP:
		ib_opcd = IB_WC_COMP_SWAP;
		break;
	case OP_TYPE_FETCH_N_ADD:
		ib_opcd = IB_WC_FETCH_ADD;
		break;
	case OP_TYPE_LOCAL_INV:
		ib_opcd = IB_WC_LOCAL_INV;
		break;
	case OP_TYPE_REG_MR:
		ib_opcd = IB_WC_REG_MR;
		break;
	default:
		ib_opcd = 0;
	}

	return ib_opcd;
}

static inline uint8_t ionic_to_ib_wc_status(uint8_t wcst)
{
	uint8_t ib_wcst;

	/* XXX should this use ionic_{req,rsp}_wc_status instead?
	 * also, do we really need two different enums for wc status? */
	switch (wcst) {
	case 0:
		ib_wcst = IB_WC_SUCCESS;
		break;
	case CQ_STATUS_LOCAL_LEN_ERR:
		ib_wcst = IB_WC_LOC_LEN_ERR;
		break;
	case CQ_STATUS_LOCAL_QP_OPER_ERR:
		ib_wcst = IB_WC_LOC_QP_OP_ERR;
		break;
	case CQ_STATUS_LOCAL_PROT_ERR:
		ib_wcst = IB_WC_LOC_PROT_ERR;
		break;
	case CQ_STATUS_WQE_FLUSHED_ERR:
		ib_wcst = IB_WC_WR_FLUSH_ERR;
		break;
	case CQ_STATUS_LOCAL_ACC_ERR:
		ib_wcst = IB_WC_LOC_ACCESS_ERR;
		break;
	case CQ_STATUS_REMOTE_INV_REQ_ERR:
		ib_wcst = IB_WC_REM_INV_REQ_ERR;
		break;
	case CQ_STATUS_REMOTE_ACC_ERR:
		ib_wcst = IB_WC_REM_ACCESS_ERR;
		break;
	case CQ_STATUS_REMOTE_OPER_ERR:
		ib_wcst = IB_WC_REM_OP_ERR;
		break;
	case CQ_STATUS_RNR_RETRY_EXCEEDED:
		ib_wcst = IB_WC_RNR_RETRY_EXC_ERR;
		break;
	case CQ_STATUS_RETRY_EXCEEDED:
		ib_wcst = IB_WC_RETRY_EXC_ERR;
		break;
	default:
		ib_wcst = IB_WC_GENERAL_ERR;
		break;
	}

	return ib_wcst;
}

#endif
