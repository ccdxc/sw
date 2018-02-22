#ifndef __IONIC_H__
#define __IONIC_H__

#include "../../../eth/ionic/ionic.h"
#include "../../../eth/ionic/ionic_lif.h"
#include "../../../eth/ionic/ionic_if.h"
#include "ib_verbs.h"
//#include "ionic_dev.h"

#define ROCE_DRV_MODULE_NAME		"ionic"
#define ROCE_DRV_MODULE_VERSION		"1.0.0"

#define IONIC_DESC	"Pensando RoCE v2 Driver"

#define IONIC_PAGE_SIZE_4K		BIT(12)
#define IONIC_PAGE_SIZE_8K		BIT(13)
#define IONIC_PAGE_SIZE_64K		BIT(16)
#define IONIC_PAGE_SIZE_2M		BIT(21)
#define IONIC_PAGE_SIZE_8M		BIT(23)
#define IONIC_PAGE_SIZE_1G		BIT(30)

#define IONIC_MAX_MR_SIZE		BIT(30)

#define IONIC_MAX_PD              1
#define IONIC_MAX_QP_COUNT		(1024)
#define IONIC_MAX_MRW_COUNT		(1024)
#define IONIC_MAX_SRQ_COUNT		(1024)
#define IONIC_MAX_CQ_COUNT		(1024)
#define IONIC_MAX_QP_WQE  		(2 * 1024)
#define IONIC_MAX_RD_ATOM         16

#define IONIC_MAX_SGID		    16
#define IONIC_MAX_PKEY          1

#define IONIC_MAX_SGE             2  //TODO: extend in future

#define IONIC_UD_QP_HW_STALL		0x400000

#define IONIC_RQ_WQE_THRESHOLD	32

#define IONIC_CQE_SIZE            32

#define IONIC_NUM_RSQ_WQE         4
#define IONIC_NUM_RRQ_WQE         4

#define PID_PAGE_ETH      0  //Page 0 is reserved for ETH driver
#define PID_PAGE_RDMA     1  //Pages 1 and above are for RDMA

#define IONIC_FLAG_TASK_IN_PROG		     0
#define IONIC_FLAG_IBDEV_REGISTERED      1

#define RDMA_SERV_TYPE_RC                      0
#define RDMA_SERV_TYPE_UD                      3

struct ionic_ib_dev {
	struct ib_device       ibdev;
	struct list_head	   list;    
	struct pci_dev        *pdev;
    struct lif            *lif;
    struct ionic          *ionic;
	void __iomem          *regs;
    unsigned long		   flags;
	bool                   ib_active;

    int				       id;
    
	atomic_t			qp_count;
	atomic_t			cq_count;
	atomic_t			srq_count;
	atomic_t			mr_count;
	atomic_t			mw_count;
    atomic_t            num_pds;

	struct mutex			qp_lock;	/* protect qp list */
	struct list_head		qp_list;
    
	/* Network device information. */
	struct net_device *netdev;
	struct notifier_block nb_netdev;
    unsigned int			version, major, minor;
    
    u8				active_speed;
	u8				active_width;

    struct ionic_cq **cq_tbl;
	spinlock_t cq_tbl_lock;
	struct ionic_qp **qp_tbl;
	spinlock_t qp_tbl_lock;
	union ib_gid *sgid_tbl;
};

struct ionic_work {
	struct work_struct	 work;
	unsigned long		 event;
	struct ionic_ib_dev *rdev;
	struct net_device	*vlan_dev;
};


/*
 * Assumptions: Host Page Size - 8K and host VA & PA takes 64bit address
 * 
 */
#define IONIC_PG_TR_PER_PAGE	     512
#define IONIC_PG_TR_PER_PAGE_BITS  9
#define IONIC_PAGES_NEEDED(num_tr)	((((num_tr)-1) >> IONIC_PG_TR_PER_PAGE_BITS) + 1)
#define IONIC_PAGE_DIR_MAX_PAGES	(1 * 512 * 512)


#define to_ionic_ib_dev(ptr)                 \
	container_of((ptr), struct ionic_ib_dev, ibdev)

#define dev_to_ionic_ib_dev(ptr)                         \
	container_of((ptr), struct ionic_ib_dev, ibdev.dev)

static inline struct
ionic_ucontext *to_ionic_ucontext(struct ib_ucontext *ibucontext)
{
	return container_of(ibucontext, struct ionic_ucontext, ibucontext);
}

static inline
struct ionic_cq * to_ionic_cq(struct ib_cq *ib_cq)
{
	return container_of(ib_cq, struct ionic_cq, ib_cq);
}

static inline
struct ionic_qp * to_ionic_qp(struct ib_qp *ib_qp)
{
	return container_of(ib_qp, struct ionic_qp, ibqp);
}

static inline struct device *rdev_to_dev(struct ionic_ib_dev *rdev)
{
	if (rdev)
		return  &rdev->ibdev.dev;
	return NULL;
}

static inline struct ionic_pd *to_ionic_pd(struct ib_pd *ibpd)
{
	return container_of(ibpd, struct ionic_pd, ibpd);
}


static inline enum ionic_qp_type ib_qp_type_to_ionic(enum ib_qp_type type)
{
    switch (type) {
    case IB_QPT_RC:
        return IONIC_QP_TYPE_RC;
    case IB_QPT_UD:
        return IONIC_QP_TYPE_UD;
    default:
        return IONIC_QP_TYPE_MAX;
    }
}

static inline enum ib_qp_type ionic_qp_type_to_ib(enum ionic_qp_type type)
{
    switch (type) {
    case IONIC_QP_TYPE_RC:
        return IB_QPT_RC;
    case IONIC_QP_TYPE_UD:
        return IB_QPT_UD;
    default:
        return IB_QPT_RESERVED1;
    }
}

extern int ionic_page_dir_init(struct ionic_ib_dev *dev, struct ionic_page_dir *pdir, u64 npages);
extern void ionic_page_dir_cleanup(struct ionic_ib_dev *dev, struct ionic_page_dir *pdir);
extern int ionic_page_dir_insert_dma(struct ionic_page_dir *pdir, u64 idx, dma_addr_t daddr);
extern int ionic_page_dir_insert_umem(struct ionic_page_dir *pdir, struct ib_umem *umem, u64 offset);
extern void ionic_page_dir_print(struct ionic_page_dir *pdir);
extern int
ionic_rdma_cmd_post(struct ionic_ib_dev *dev,
                    void *req,
                    void *resp);

#endif /* __IONIC_H__ */
