/*
 * Description: IB Verbs interpreter (header)
 */

#ifndef __IONIC_IB_VERBS_H__
#define __IONIC_IB_VERBS_H__

struct ionic_gid_ctx {
	u32			idx;
	u32			refcnt;
};

#define IONIC_FENCE_BYTES	64
struct ionic_fence_data {
	u32 size;
	u8 va[IONIC_FENCE_BYTES];
	dma_addr_t dma_addr;
	struct ionic_mr *mr;
	struct ib_mw *mw;
//	struct bnxt_qplib_swqe bind_wqe;
	u32 bind_rkey;
};

struct ionic_pd {
	struct ib_pd     ibpd;
	struct ionic_ib_dev	*rdev;
    u32              pdn;
};

struct ionic_ah {
	struct ionic_ib_dev	*rdev;
	struct ib_ah		ib_ah;
//	struct bnxt_qplib_ah	qplib_ah;
};

struct ionic_page_dir {
	dma_addr_t tbl_dma;
	u64 *table;
	u64 npages;
    u64 phy_npages;
};

struct ionic_wq {
	struct ionic_ring *ring;
	spinlock_t lock; /* Work queue lock. */
	int wqe_cnt;
	int wqe_size;
	int max_sg;
	int offset;
};


struct ionic_qp_modify_attr {
	u32			qkey;
	u32			dest_qp_num;
};

#define PACKED __attribute__((__packed__))
typedef struct udphdr_s {
    uint16_t   csum;
    uint16_t   length;
    uint16_t   dport;
    uint16_t   sport;
} PACKED udphdr_t;

typedef struct iphdr_s {
    uint32_t   daddr;
    uint32_t   saddr;
    uint16_t   check;
    uint8_t    protocol;
    uint8_t    ttl;
    uint16_t   frag_off;
    uint16_t   id;
    uint16_t   tot_len;
    uint8_t    tos;
    uint8_t    ihl:4;
    uint8_t    version:4;
/*The options start here. */
} PACKED iphdr_t;

#define MAC_SIZE 6

typedef struct ethhdr_s {
    uint16_t  ethertype;
    uint8_t   smac[MAC_SIZE];
    uint8_t   dmac[MAC_SIZE];
} PACKED ethhdr_t;

typedef struct vlanhdr_s {
    uint16_t ethertype;
    uint16_t vlan: 12;
    uint16_t cfi: 1;
    uint16_t pri: 3;
} PACKED vlanhdr_t;

typedef struct header_template_s {
    udphdr_t    udp;
    iphdr_t     ip;
    //vlanhdr_t   vlan;
    ethhdr_t    eth;
} PACKED header_template_t;

struct ionic_qp {
	struct ib_qp		   ibqp;
	struct ionic_ib_dev	  *rdev;
	u32                    qkey;
	struct ionic_wq        sq;
	struct ionic_wq        rq;
	struct ib_umem        *rumem;
	struct ib_umem        *sumem;
	struct ionic_page_dir  pdir;
	int                    npages;
	int                    npages_send;
	int                    npages_recv;
	u32                    flags;
	u8                     port;
	u8                     state;
	bool                   is_kernel;
	struct mutex           mutex; /* QP state mutex. */
	atomic_t               refcnt;
	wait_queue_head_t      wait;
    header_template_t      *header;
    dma_addr_t             hdr_dma;
};

struct ionic_cq {
	struct ionic_ib_dev	      *rdev;
    struct ib_cq	       ib_cq;
	struct ib_umem        *umem;
	spinlock_t             cq_lock;	/* protect cq */
    struct ionic_page_dir    pdir;
    atomic_t               refcnt;
    u32                    cq_num;
    u8                     qtype;
	wait_queue_head_t      wait;
};

struct ionic_mr {
	struct ionic_ib_dev	 *rdev;
	struct ib_mr	  ib_mr;
	struct ib_umem	 *ib_umem;
    struct ionic_page_dir    pdir;
	//u32	    		  npages;
	//u64		    	 *pages;
    u64               iova;
    u64               size;
};

struct ionic_ucontext {
	struct ib_ucontext   ibucontext;
	struct ionic_ib_dev *dev;
    int                  pid;
    //struct doorbell     *db_pages;
    unsigned long pfn;
};

enum ionic_qp_type {
	IONIC_QP_TYPE_RC = 0,
	IONIC_QP_TYPE_UC,
	IONIC_QP_TYPE_RD,
	IONIC_QP_TYPE_UD,
	IONIC_QP_TYPE_MAX = 255,
};

struct net_device *ionic_get_netdev(struct ib_device *ibdev, u8 port_num);
int ionic_query_device(struct ib_device *ibdev,
			 struct ib_device_attr *ib_attr,
			 struct ib_udata *udata);
int ionic_modify_device(struct ib_device *ibdev,
			  int device_modify_mask,
			  struct ib_device_modify *device_modify);
int ionic_query_port(struct ib_device *ibdev, u8 port_num,
		       struct ib_port_attr *port_attr);
int ionic_get_port_immutable(struct ib_device *ibdev, u8 port_num,
			       struct ib_port_immutable *immutable);
int ionic_query_pkey(struct ib_device *ibdev, u8 port_num,
		       u16 index, u16 *pkey);
int ionic_del_gid(struct ib_device *ibdev, u8 port_num,
		    unsigned int index, void **context);
int ionic_add_gid(struct ib_device *ibdev, u8 port_num,
		    unsigned int index, const union ib_gid *gid,
		    const struct ib_gid_attr *attr, void **context);
int ionic_query_gid(struct ib_device *ibdev, u8 port_num,
		      int index, union ib_gid *gid);
enum rdma_link_layer ionic_port_link_layer(struct ib_device *ibdev,
					    u8 port_num);
struct ib_pd *ionic_alloc_pd(struct ib_device *ibdev,
			       struct ib_ucontext *context,
			       struct ib_udata *udata);
int ionic_dealloc_pd(struct ib_pd *pd);
struct ib_ah *ionic_create_ah(struct ib_pd *pd,
				struct rdma_ah_attr *ah_attr,
				struct ib_udata *udata);
int ionic_modify_ah(struct ib_ah *ah, struct rdma_ah_attr *ah_attr);
int ionic_query_ah(struct ib_ah *ah, struct rdma_ah_attr *ah_attr);
int ionic_destroy_ah(struct ib_ah *ah);
struct ib_qp *ionic_create_qp(struct ib_pd *pd,
				struct ib_qp_init_attr *qp_init_attr,
				struct ib_udata *udata);
int ionic_modify_qp(struct ib_qp *qp, struct ib_qp_attr *qp_attr,
		      int qp_attr_mask, struct ib_udata *udata);
int ionic_query_qp(struct ib_qp *qp, struct ib_qp_attr *qp_attr,
		     int qp_attr_mask, struct ib_qp_init_attr *qp_init_attr);
int ionic_destroy_qp(struct ib_qp *qp);
int ionic_post_send(struct ib_qp *qp, struct ib_send_wr *send_wr,
		      struct ib_send_wr **bad_send_wr);
int ionic_post_recv(struct ib_qp *qp, struct ib_recv_wr *recv_wr,
		      struct ib_recv_wr **bad_recv_wr);
struct ib_cq *ionic_create_cq(struct ib_device *ibdev,
				const struct ib_cq_init_attr *attr,
				struct ib_ucontext *context,
				struct ib_udata *udata);
int ionic_destroy_cq(struct ib_cq *cq);
int ionic_poll_cq(struct ib_cq *cq, int num_entries, struct ib_wc *wc);
int ionic_req_notify_cq(struct ib_cq *cq, enum ib_cq_notify_flags flags);
struct ib_mr *ionic_get_dma_mr(struct ib_pd *pd, int mr_access_flags);

int ionic_map_mr_sg(struct ib_mr *ib_mr, struct scatterlist *sg, int sg_nents,
		      unsigned int *sg_offset);
struct ib_mr *ionic_alloc_mr(struct ib_pd *ib_pd, enum ib_mr_type mr_type,
			       u32 max_num_sg);
int ionic_dereg_mr(struct ib_mr *mr);
struct ib_mw *ionic_alloc_mw(struct ib_pd *ib_pd, enum ib_mw_type type,
			       struct ib_udata *udata);
int ionic_dealloc_mw(struct ib_mw *mw);
struct ib_mr *ionic_reg_user_mr(struct ib_pd *pd, u64 start, u64 length,
				  u64 virt_addr, int mr_access_flags,
				  struct ib_udata *udata);
struct ib_ucontext *ionic_alloc_ucontext(struct ib_device *ibdev,
					   struct ib_udata *udata);
int ionic_dealloc_ucontext(struct ib_ucontext *context);
int ionic_mmap(struct ib_ucontext *context, struct vm_area_struct *vma);
#endif /* __IONIC_IB_VERBS_H__ */
