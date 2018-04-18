#ifndef IONIC_FW_H
#define IONIC_FW_H

#define IONIC_NUM_RSQ_WQE         4
#define IONIC_NUM_RRQ_WQE         4

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

#endif
