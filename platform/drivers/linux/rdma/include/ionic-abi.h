/*
  * Description: Uverbs ABI header file
  */
#ifndef __IONIC_UVERBS_ABI_H__
#define __IONIC_UVERBS_ABI_H__

#include <linux/types.h>

#define IONIC_ABI_VERSION	1

struct ionic_uctx_resp {
	__u32 dev_id;
	__u32 max_qp;
	__u32 pg_size;
	__u32 cqe_size;
	__u32 max_cqd;
	__u32 rsvd;
};

struct ionic_pd_resp {
	__u32 pdid;
};

struct ionic_cq_req {
	__u64 cq_va;
    __u64 cq_bytes;
};

struct ionic_cq_resp {
	__u32 cqid;
	__u8  qtype;
};

struct ionic_qp_req {
	__u64 qpsva;
	__u64 qprva;
    __u32 sq_bytes;
    __u32 rq_bytes;
    __u32 sq_wqe_size;
    __u32 rq_wqe_size;
};

struct ionic_qp_resp {
	__u32 qpid;
    __u8  sq_qtype;
    __u8  rq_qtype;
	__u16 rsvd;
};

enum ionic_shpg_offt {
	IONIC_BEG_RESV_OFFT	= 0x00,
	IONIC_AVID_OFFT	= 0x10,
	IONIC_AVID_SIZE	= 0x04,
	IONIC_END_RESV_OFFT	= 0xFF0
};

#endif /* __IONIC_UVERBS_ABI_H__*/
