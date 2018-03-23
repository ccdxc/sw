#ifndef IONIC_ABI_H
#define IONIC_ABI_H

#include <linux/types.h>

#define IONIC_ABI_VERSION	1

struct ionic_ctx_req {
	__u32 version;
	__u32 fallback;
};

struct ionic_ctx_resp {
	__u32 version;
	__u32 fallback;
	__u64 dbell_offset;
	__u8 sq_qtype;
	__u8 rq_qtype;
	__u8 cq_qtype;
};

struct ionic_qdesc {
	__u64 addr;
	__u32 size;
	__u16 mask;
	__u16 stride;
};

struct ionic_cq_req {
	struct ionic_qdesc cq;
};

struct ionic_cq_resp {
	__u32 cqid;
};

struct ionic_qp_req {
	struct ionic_qdesc sq;
	struct ionic_qdesc rq;
};

struct ionic_qp_resp {
	__u32 qpid;
	__u32 rsvd;
	__u64 sq_hbm_offset;
};

#endif /* IONIC_ABI_H */
