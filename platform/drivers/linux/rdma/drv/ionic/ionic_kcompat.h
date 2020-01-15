/* SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB */
/*
 * Copyright (c) 2018-2019 Pensando Systems, Inc.  All rights reserved.
 */

#ifndef IONIC_KCOMPAT
#define IONIC_KCOMPAT

#include <linux/version.h>
#include <linux/netdevice.h>
#include <rdma/ib_pack.h>
#include <rdma/ib_verbs.h>

/* marks code inserted to silence false positive warnings */
#define IONIC_STATIC_ANALYSIS_HINTS_NOT_FOR_UPSTREAM 1

/****************************************************************************
 *
 * Compatibility for kernel-only features (not affected by OFA version)
 *
 */
#if defined(RHEL_RELEASE_VERSION)
#define IONIC_KCOMPAT_KERN_VERSION_PRIOR_TO(LX_MAJ, LX_MIN, RH_MAJ, RH_MIN) \
	(RHEL_RELEASE_CODE < RHEL_RELEASE_VERSION(RH_MAJ, RH_MIN))
#else
#define IONIC_KCOMPAT_KERN_VERSION_PRIOR_TO(LX_MAJ, LX_MIN, RH_MAJ, RH_MIN) \
	(LINUX_VERSION_CODE < KERNEL_VERSION(LX_MAJ, LX_MIN, 0))
#endif

#if IONIC_KCOMPAT_KERN_VERSION_PRIOR_TO(/* Linux */ 4,14, /* RHEL */ 99,99)
#else /* 4.14.0 and later */
#define HAVE_GET_VECTOR_AFFINITY
#endif

#if IONIC_KCOMPAT_KERN_VERSION_PRIOR_TO(/* Linux */ 3,11, /* RHEL */ 99,99)
#define netdev_notifier_info_to_dev(ptr) (ptr)
#endif

#if IONIC_KCOMPAT_KERN_VERSION_PRIOR_TO(/* Linux */ 4,10, /* RHEL */ 99,99)
#else /* 4.10.0 and later */
#define HAVE_NETDEV_MAX_MTU
#endif

#if IONIC_KCOMPAT_KERN_VERSION_PRIOR_TO(/* Linux */ 4,11, /* RHEL */ 99,99)
/* XXX: Later stable versions of 4.9 and 4.10 added refcount_t */
#define refcount_t            atomic_t
#define refcount_set          atomic_set
#define refcount_dec_and_test atomic_dec_and_test
#define refcount_inc          atomic_inc
#endif

#if IONIC_KCOMPAT_KERN_VERSION_PRIOR_TO(/* Linux */ 4,17, /* RHEL */ 99,99)
/* Create an xarray that includes a radix_tree_root and a spinlock */
#include <linux/radix-tree.h>

struct xarray {
	spinlock_t x_lock;
	struct radix_tree_root x_tree;
};
#define xa_tree(_xa) &(_xa)->x_tree

#define xa_lock(_xa) spin_lock(&(_xa)->x_lock)
#define xa_unlock(_xa) spin_unlock(&(_xa)->x_lock)
#define xa_lock_irq(_xa) spin_lock_irq(&(_xa)->x_lock)
#define xa_unlock_irq(_xa) spin_unlock_irq(&(_xa)->x_lock)
#define xa_lock_irqsave(_xa, _flags)					\
	spin_lock_irqsave(&(_xa)->x_lock, _flags)
#define xa_unlock_irqrestore(_xa, _flags)				\
	spin_unlock_irqrestore(&(_xa)->x_lock, _flags)

static inline void xa_init(struct xarray *xa)
{
	spin_lock_init(&xa->x_lock);
	INIT_RADIX_TREE(xa_tree(xa), GFP_KERNEL);
}

#elif IONIC_KCOMPAT_KERN_VERSION_PRIOR_TO(/* Linux */ 4,20, /* RHEL */ 99,99)
/* 4.17, 4.18, 4.19: A radix_tree now includes a spinlock called xa_lock */
#include <linux/radix-tree.h>

#define xarray       radix_tree_root
#define xa_tree
#define xa_init(_xa) INIT_RADIX_TREE(xa_tree(_xa), GFP_KERNEL)

#elif IONIC_KCOMPAT_KERN_VERSION_PRIOR_TO(/* Linux */ 5,0, /* RHEL */ 99,99)
#define HAVE_XARRAY

/* 4.20: xa_for_each() has extra arguments */
#include <linux/xarray.h>
#undef xa_for_each
#define xa_for_each(xa, index, entry)					\
	for (entry = xa_find(xa, &index, ULONG_MAX, XA_PRESENT); entry;	\
	     entry = xa_find_after(xa, &index, ULONG_MAX, XA_PRESENT))

#else /* 5.0 and later */
#define HAVE_XARRAY
#endif

#ifndef HAVE_XARRAY

#define xa_iter radix_tree_iter
#define xa_for_each_slot(_xa, _slot, _iter)				\
	radix_tree_for_each_slot((_slot), xa_tree(_xa), (_iter), 0)
#define xa_load(_xa, _idx) radix_tree_lookup(xa_tree(_xa), _idx)
#define xa_destroy(_xa)

static inline void *xa_store_irq(struct xarray *xa, unsigned long idx,
				 void *item, gfp_t unused)
{
	int ret;

	xa_lock_irq(xa);
	ret = radix_tree_insert(xa_tree(xa), idx, item);
	xa_unlock_irq(xa);

	return (ret ? ERR_PTR(ret) : item);
}

static inline int xa_err(void *item)
{
	return (IS_ERR(item) ? PTR_ERR(item) : 0);
}

static inline void xa_erase_irq(struct xarray *xa, unsigned long idx)
{
	xa_lock_irq(xa);
	radix_tree_delete(xa_tree(xa), idx);
	xa_unlock_irq(xa);
}

#endif /* HAVE_XARRAY */


/****************************************************************************
 *
 * Compatibility for OFED features that may be affected by OFA version
 *
 */
#if defined(OFA_KERNEL)
#include "ionic_kcompat_ofa.h"
#define IONIC_KCOMPAT_VERSION_PRIOR_TO(LX_MAJ, LX_MIN, RH_MAJ, RH_MIN, OFA) \
	OFA_COMPAT_CHECK(OFA_KERNEL, OFA)
#else
#define IONIC_KCOMPAT_VERSION_PRIOR_TO(LX_MAJ, LX_MIN, RH_MAJ, RH_MIN, OFA) \
	IONIC_KCOMPAT_KERN_VERSION_PRIOR_TO(LX_MAJ, LX_MIN, RH_MAJ, RH_MIN)
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 4,8, /* RHEL */ 7,6, /* OFA */ 4_8)
#else /* 4.9.0 and later */
#define HAVE_QP_RWQ_IND_TBL
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 4,9, /* RHEL */ 7,4, /* OFA */ 4_9a)
#define IB_DEVICE_NODE_DESC_MAX sizeof(((struct ib_device *)0)->node_desc)
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 4,9, /* RHEL */ 7,5, /* OFA */ 4_9b)
#else /* 4.9.0 and later */
#define HAVE_IB_PD_UNSAFE_DMA_RKEY
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 4,10, /* RHEL */ 7,4, /* OFA */ 4_10a)

static inline enum ib_mtu ib_mtu_int_to_enum(int mtu)
{
	if (mtu >= 4096)
		return IB_MTU_4096;
	else if (mtu >= 2048)
		return IB_MTU_2048;
	else if (mtu >= 1024)
		return IB_MTU_1024;
	else if (mtu >= 512)
		return IB_MTU_512;
	else
		return IB_MTU_256;
}

#else /* 4.10.0 and later */
#define HAVE_GET_DEV_FW_STR
#define HAVE_CREATE_AH_UDATA
#define HAVE_EX_CMD_MODIFY_QP
#define HAVE_QP_RATE_LIMIT
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 4,11, /* RHEL */ 7,5, /* OFA */ 4_11)
#define HAVE_REQUIRED_DMA_DEVICE
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 4,12, /* RHEL */ 7,5, /* OFA */ 4_12)

#define rdma_ah_attr ib_ah_attr
#define rdma_ah_read_grh(attr) (&(attr)->grh)

static inline void rdma_ah_set_sl(struct rdma_ah_attr *attr, u8 sl)
{
	attr->sl = sl;
}

static inline void rdma_ah_set_port_num(struct rdma_ah_attr *attr, u8 port_num)
{
	attr->port_num = port_num;
}

static inline void rdma_ah_set_grh(struct rdma_ah_attr *attr,
				   union ib_gid *dgid, u32 flow_label,
				   u8 sgid_index, u8 hop_limit,
				   u8 traffic_class)
{
	struct ib_global_route *grh = rdma_ah_read_grh(attr);

	attr->ah_flags = IB_AH_GRH;
	if (dgid)
		grh->dgid = *dgid;
	grh->flow_label = flow_label;
	grh->sgid_index = sgid_index;
	grh->hop_limit = hop_limit;
	grh->traffic_class = traffic_class;
}

static inline void rdma_ah_set_dgid_raw(struct rdma_ah_attr *attr, void *dgid)
{
	struct ib_global_route *grh = rdma_ah_read_grh(attr);

	memcpy(grh->dgid.raw, dgid, sizeof(grh->dgid));
}

#else /* 4.12.0 and later */
#define HAVE_RDMA_AH_ATTR_TYPE_ROCE
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 4,14, /* RHEL */ 7,5, /* OFA */ 4_14a)
static inline int ib_get_eth_speed(struct ib_device *dev, u8 port_num,
				   u8 *speed, u8 *width)
{
	*width = IB_WIDTH_4X;
	*speed = IB_SPEED_EDR;
	return 0;
}
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 4,14, /* RHEL */ 7,5, /* OFA */ 4_14b)
static inline bool ib_srq_has_cq(enum ib_srq_type srq_type)
{
	return srq_type == IB_SRQT_XRC;
}

#define HAVE_GET_DEV_FW_STR_LEN

#else /* 4.14.0 and later */
#define HAVE_SRQ_EXT_CQ
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 4,14, /* RHEL */ 99,99, /* OFA */ 4_14c)
#else /* 4.14.0 and later */
#define HAVE_QP_INIT_SRC_QPN
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 4,15, /* RHEL */ 7,6, /* OFA */ 4_15)
/* XXX UEK
 *
 * Oracle Linux removed the symbol export for ib_resolve_eth_dmac, so we can't
 * call it from the driver.  Defining the macro below disables the driver code.
 *
 * User space verbs with UD type QPs is broken on Oracle Linux.
 */
#ifdef _LINUX_UEK_KABI_H
#define HAVE_CREATE_AH_UDATA_DMAC
#endif
#else /* 4.15.0 and later */
#define HAVE_CREATE_AH_UDATA_DMAC
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 4,17, /* RHEL */ 99,99, /* OFA */ 4_17)
#define HAVE_IB_GID_DEV_PORT_INDEX
#else /* 4.17.0 and later */
#define HAVE_RDMA_DRIVER_ID
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 4,19, /* RHEL */ 99,99, /* OFA */ 4_19a)
#define HAVE_REQUIRED_IB_GID
#define ib_gid_to_network_type(gid_type, gid) \
	ib_gid_to_network_type(gid_type, (union ib_gid *)(gid))
#else
#define HAVE_AH_ATTR_CACHED_GID
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 4,19, /* RHEL */ 99,99, /* OFA */ 4_19b)
#define ud_wr(wr) ud_wr((struct ib_send_wr *)(wr))
#define rdma_wr(wr) rdma_wr((struct ib_send_wr *)(wr))
#define atomic_wr(wr) atomic_wr((struct ib_send_wr *)(wr))
#define reg_wr(wr) reg_wr((struct ib_send_wr *)(wr))
#else
#define HAVE_CONST_IB_WR
#define HAVE_IBDEV_MAX_SEND_RECV_SGE
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 4,19, /* RHEL */ 99,99, /* OFA */ 4_19c)
#else
#define HAVE_PORT_ATTR_IP_GIDS
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 4,20, /* RHEL */ 99,99, /* OFA */ 4_20)
#define ib_modify_qp_is_ok(cur_state, new_state, qp_type, attr_mask) \
	ib_modify_qp_is_ok(cur_state, new_state, qp_type, attr_mask, \
			   IB_LINK_LAYER_ETHERNET)
#else
#define HAVE_IB_REGISTER_DEVICE_NAME
#define HAVE_RDMA_DEV_SYSFS_GROUP
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 5,0, /* RHEL */ 99,99, /* OFA */ 5_0)

struct ib_device_ops {
#ifdef HAVE_CONST_IB_WR
	int (*post_send)(struct ib_qp *qp, const struct ib_send_wr *send_wr,
			 const struct ib_send_wr **bad_send_wr);
	int (*post_recv)(struct ib_qp *qp, const struct ib_recv_wr *recv_wr,
			 const struct ib_recv_wr **bad_recv_wr);
#else
	int (*post_send)(struct ib_qp *qp, struct ib_send_wr *send_wr,
			 struct ib_send_wr **bad_send_wr);
	int (*post_recv)(struct ib_qp *qp, struct ib_recv_wr *recv_wr,
			 struct ib_recv_wr **bad_recv_wr);
#endif
	void (*drain_rq)(struct ib_qp *qp);
	void (*drain_sq)(struct ib_qp *qp);
	int (*poll_cq)(struct ib_cq *cq, int num_entries, struct ib_wc *wc);
	int (*peek_cq)(struct ib_cq *cq, int wc_cnt);
	int (*req_notify_cq)(struct ib_cq *cq, enum ib_cq_notify_flags flags);
	int (*req_ncomp_notif)(struct ib_cq *cq, int wc_cnt);
#ifdef HAVE_CONST_IB_WR
	int (*post_srq_recv)(struct ib_srq *srq,
			     const struct ib_recv_wr *recv_wr,
			     const struct ib_recv_wr **bad_recv_wr);
#else
	int (*post_srq_recv)(struct ib_srq *srq,
			     struct ib_recv_wr *recv_wr,
			     struct ib_recv_wr **bad_recv_wr);
#endif
	int (*query_device)(struct ib_device *device,
			    struct ib_device_attr *device_attr,
			    struct ib_udata *udata);
	int (*modify_device)(struct ib_device *device, int device_modify_mask,
			     struct ib_device_modify *device_modify);
#ifdef HAVE_GET_DEV_FW_STR_LEN
	void (*get_dev_fw_str)(struct ib_device *device, char *str,
			       size_t str_len);
#else
	void (*get_dev_fw_str)(struct ib_device *device, char *str);
#endif
#ifdef HAVE_GET_VECTOR_AFFINITY
	const struct cpumask *(*get_vector_affinity)(struct ib_device *ibdev,
						     int comp_vector);
#endif
	int (*query_port)(struct ib_device *device, u8 port_num,
			  struct ib_port_attr *port_attr);
	int (*modify_port)(struct ib_device *device, u8 port_num,
			   int port_modify_mask,
			   struct ib_port_modify *port_modify);
	int (*get_port_immutable)(struct ib_device *device, u8 port_num,
				  struct ib_port_immutable *immutable);
	enum rdma_link_layer (*get_link_layer)(struct ib_device *device,
					       u8 port_num);
	struct net_device *(*get_netdev)(struct ib_device *device, u8 port_num);
#ifdef HAVE_REQUIRED_IB_GID
	int (*query_gid)(struct ib_device *device, u8 port_num, int index,
			 union ib_gid *gid);
#ifdef HAVE_IB_GID_DEV_PORT_INDEX
	int (*add_gid)(struct ib_device *device, u8 port, unsigned int index,
		       const union ib_gid *gid, const struct ib_gid_attr *attr,
		       void **context);
	int (*del_gid)(struct ib_device *device, u8 port, unsigned int index,
		       void **context);
#else
	int (*add_gid)(const union ib_gid *gid, const struct ib_gid_attr *attr,
		       void **context);
	int (*del_gid)(const struct ib_gid_attr *attr, void **context);
#endif
#endif
	int (*query_pkey)(struct ib_device *device, u8 port_num, u16 index,
			  u16 *pkey);
	struct ib_ucontext *(*alloc_ucontext)(struct ib_device *device,
					      struct ib_udata *udata);
	int (*dealloc_ucontext)(struct ib_ucontext *context);
	int (*mmap)(struct ib_ucontext *context, struct vm_area_struct *vma);
	void (*disassociate_ucontext)(struct ib_ucontext *ibcontext);
	struct ib_pd *(*alloc_pd)(struct ib_device *device,
				  struct ib_ucontext *context,
				  struct ib_udata *udata);
	int (*dealloc_pd)(struct ib_pd *pd);
#ifdef HAVE_CREATE_AH_UDATA
#ifdef HAVE_CREATE_AH_FLAGS
	struct ib_ah *(*create_ah)(struct ib_pd *pd,
				   struct rdma_ah_attr *ah_attr, u32 flags,
				   struct ib_udata *udata);
#else
	struct ib_ah *(*create_ah)(struct ib_pd *pd,
				   struct rdma_ah_attr *ah_attr,
				   struct ib_udata *udata);
#endif
#else
	struct ib_ah *(*create_ah)(struct ib_pd *pd,
				   struct rdma_ah_attr *ah_attr);
#endif
	int (*modify_ah)(struct ib_ah *ah, struct rdma_ah_attr *ah_attr);
	int (*query_ah)(struct ib_ah *ah, struct rdma_ah_attr *ah_attr);
#ifdef HAVE_CREATE_AH_FLAGS
	int (*destroy_ah)(struct ib_ah *ah, u32 flags);
#else
	int (*destroy_ah)(struct ib_ah *ah);
#endif
	struct ib_srq *(*create_srq)(struct ib_pd *pd,
				     struct ib_srq_init_attr *srq_init_attr,
				     struct ib_udata *udata);
	int (*modify_srq)(struct ib_srq *srq, struct ib_srq_attr *srq_attr,
			  enum ib_srq_attr_mask srq_attr_mask,
			  struct ib_udata *udata);
	int (*query_srq)(struct ib_srq *srq, struct ib_srq_attr *srq_attr);
	int (*destroy_srq)(struct ib_srq *srq);
	struct ib_qp *(*create_qp)(struct ib_pd *pd,
				   struct ib_qp_init_attr *qp_init_attr,
				   struct ib_udata *udata);
	int (*modify_qp)(struct ib_qp *qp, struct ib_qp_attr *qp_attr,
			 int qp_attr_mask, struct ib_udata *udata);
	int (*query_qp)(struct ib_qp *qp, struct ib_qp_attr *qp_attr,
			int qp_attr_mask, struct ib_qp_init_attr *qp_init_attr);
	int (*destroy_qp)(struct ib_qp *qp);
	struct ib_cq *(*create_cq)(struct ib_device *device,
				   const struct ib_cq_init_attr *attr,
				   struct ib_ucontext *context,
				   struct ib_udata *udata);
	int (*modify_cq)(struct ib_cq *cq, u16 cq_count, u16 cq_period);
	int (*destroy_cq)(struct ib_cq *cq);
	int (*resize_cq)(struct ib_cq *cq, int cqe, struct ib_udata *udata);
	struct ib_mr *(*get_dma_mr)(struct ib_pd *pd, int mr_access_flags);
#ifdef HAVE_IB_USER_MR_INIT_ATTR
	struct ib_mr *(*reg_user_mr)(struct ib_pd *pd,
				     struct ib_mr_init_attr *attr,
				     struct ib_udata *udata);
#else
	struct ib_mr *(*reg_user_mr)(struct ib_pd *pd, u64 start, u64 length,
				     u64 virt_addr, int mr_access_flags,
				     struct ib_udata *udata);
#endif
	int (*rereg_user_mr)(struct ib_mr *mr, int flags, u64 start, u64 length,
			     u64 virt_addr, int mr_access_flags,
			     struct ib_pd *pd, struct ib_udata *udata);
	int (*dereg_mr)(struct ib_mr *mr);
	struct ib_mr *(*alloc_mr)(struct ib_pd *pd, enum ib_mr_type mr_type,
				  u32 max_num_sg);
	int (*map_mr_sg)(struct ib_mr *mr, struct scatterlist *sg, int sg_nents,
			 unsigned int *sg_offset);
	int (*check_mr_status)(struct ib_mr *mr, u32 check_mask,
			       struct ib_mr_status *mr_status);
	struct ib_mw *(*alloc_mw)(struct ib_pd *pd, enum ib_mw_type type,
				  struct ib_udata *udata);
	int (*dealloc_mw)(struct ib_mw *mw);
	int (*attach_mcast)(struct ib_qp *qp, union ib_gid *gid, u16 lid);
	int (*detach_mcast)(struct ib_qp *qp, union ib_gid *gid, u16 lid);
	struct ib_xrcd *(*alloc_xrcd)(struct ib_device *device,
				      struct ib_ucontext *ucontext,
				      struct ib_udata *udata);
	int (*dealloc_xrcd)(struct ib_xrcd *xrcd);
	struct rdma_hw_stats *(*alloc_hw_stats)(struct ib_device *device,
						u8 port_num);
	int (*get_hw_stats)(struct ib_device *device,
			    struct rdma_hw_stats *stats, u8 port, int index);
};

static inline void ib_set_device_ops(struct ib_device *dev,
				     const struct ib_device_ops *ops)
{
#define SET_DEVICE_OP(name) \
	(dev->name = dev->name ?: ops->name)

#ifdef HAVE_REQUIRED_IB_GID
	SET_DEVICE_OP(add_gid);
#endif
	SET_DEVICE_OP(alloc_hw_stats);
	SET_DEVICE_OP(alloc_mr);
	SET_DEVICE_OP(alloc_mw);
	SET_DEVICE_OP(alloc_pd);
	SET_DEVICE_OP(alloc_ucontext);
	SET_DEVICE_OP(alloc_xrcd);
	SET_DEVICE_OP(attach_mcast);
	SET_DEVICE_OP(check_mr_status);
	SET_DEVICE_OP(create_ah);
	SET_DEVICE_OP(create_cq);
	SET_DEVICE_OP(create_qp);
	SET_DEVICE_OP(create_srq);
	SET_DEVICE_OP(dealloc_mw);
	SET_DEVICE_OP(dealloc_pd);
	SET_DEVICE_OP(dealloc_ucontext);
	SET_DEVICE_OP(dealloc_xrcd);
#ifdef HAVE_REQUIRED_IB_GID
	SET_DEVICE_OP(del_gid);
#endif
	SET_DEVICE_OP(dereg_mr);
	SET_DEVICE_OP(destroy_ah);
	SET_DEVICE_OP(destroy_cq);
	SET_DEVICE_OP(destroy_qp);
	SET_DEVICE_OP(destroy_srq);
	SET_DEVICE_OP(detach_mcast);
	SET_DEVICE_OP(disassociate_ucontext);
	SET_DEVICE_OP(drain_rq);
	SET_DEVICE_OP(drain_sq);
#ifdef HAVE_GET_DEV_FW_STR
	SET_DEVICE_OP(get_dev_fw_str);
#endif
	SET_DEVICE_OP(get_dma_mr);
	SET_DEVICE_OP(get_hw_stats);
	SET_DEVICE_OP(get_link_layer);
	SET_DEVICE_OP(get_netdev);
	SET_DEVICE_OP(get_port_immutable);
#ifdef HAVE_GET_VECTOR_AFFINITY
	SET_DEVICE_OP(get_vector_affinity);
#endif
	SET_DEVICE_OP(map_mr_sg);
	SET_DEVICE_OP(mmap);
	SET_DEVICE_OP(modify_ah);
	SET_DEVICE_OP(modify_cq);
	SET_DEVICE_OP(modify_device);
	SET_DEVICE_OP(modify_port);
	SET_DEVICE_OP(modify_qp);
	SET_DEVICE_OP(modify_srq);
	SET_DEVICE_OP(peek_cq);
	SET_DEVICE_OP(poll_cq);
	SET_DEVICE_OP(post_recv);
	SET_DEVICE_OP(post_send);
	SET_DEVICE_OP(post_srq_recv);
	SET_DEVICE_OP(query_ah);
	SET_DEVICE_OP(query_device);
#ifdef HAVE_REQUIRED_IB_GID
	SET_DEVICE_OP(query_gid);
#endif
	SET_DEVICE_OP(query_pkey);
	SET_DEVICE_OP(query_port);
	SET_DEVICE_OP(query_qp);
	SET_DEVICE_OP(query_srq);
	SET_DEVICE_OP(reg_user_mr);
	SET_DEVICE_OP(req_ncomp_notif);
	SET_DEVICE_OP(req_notify_cq);
	SET_DEVICE_OP(rereg_user_mr);
	SET_DEVICE_OP(resize_cq);
#undef SET_DEVICE_OP
}

#undef dma_alloc_coherent
#define dma_alloc_coherent dma_zalloc_coherent
#define RDMA_CREATE_AH_SLEEPABLE 0
#else
#define HAVE_CREATE_AH_FLAGS
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 5,1, /* RHEL */ 99,99, /* OFA */ 5_0)
#else /* 5.1 and later */
#define HAVE_IB_REGISTER_DEVICE_NAME_ONLY
#define HAVE_IB_ALLOC_DEV_CONTAINER
#define HAVE_IB_ALLOC_UCTX_OBJ
#define HAVE_IB_DEALLOC_UCTX_VOID
#define HAVE_IB_ALLOC_PD_OBJ
#define HAVE_IB_DEALLOC_PD_VOID
#define HAVE_IB_UMEM_GET_UDATA
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 5,2, /* RHEL */ 99,99, /* OFA */ 5_0)
#else /* 5.2 and later */
#define HAVE_IB_ALLOC_AH_OBJ
#define HAVE_IB_DESTROY_AH_VOID
#define HAVE_IB_ALLOC_SRQ_OBJ
#define HAVE_IB_DESTROY_SRQ_VOID
#define HAVE_IB_API_UDATA
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 5,3, /* RHEL */ 99,99, /* OFA */ 5_0)
#else /* 5.3 and later */
#define HAVE_RDMA_DEV_OPS_EXT
#define HAVE_IB_ALLOC_CQ_OBJ
#define HAVE_IB_DESTROY_CQ_VOID
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 5,4, /* RHEL */ 99,99, /* OFA */ 5_0)
/* This is finally part of the stack in kernel 5.4 */
enum ib_port_phys_state {
	IB_PORT_PHYS_STATE_SLEEP = 1,
	IB_PORT_PHYS_STATE_POLLING = 2,
	IB_PORT_PHYS_STATE_DISABLED = 3,
	IB_PORT_PHYS_STATE_PORT_CONFIGURATION_TRAINING = 4,
	IB_PORT_PHYS_STATE_LINK_UP = 5,
	IB_PORT_PHYS_STATE_LINK_ERROR_RECOVERY = 6,
	IB_PORT_PHYS_STATE_PHY_TEST = 7,
};
#else /* 5.4 and later */
#endif

/* other compat for not yet upstream changes */

/**
 * roce_ud_header_unpack - Unpack UD header struct from RoCE wire format
 * @header:UD header struct
 * @buf:Buffer to unpack into
 *
 * roce_ud_header_pack() unpacks the UD header structure @header from RoCE wire
 * format in the buffer @buf.
 */
int roce_ud_header_unpack(void *buf, struct ib_ud_header *header);

#define IF_LLADDR(ndev) (ndev->dev_addr)

#ifdef HAVE_RDMA_DRIVER_ID
#include <rdma/rdma_user_ioctl_cmds.h>
/* Upstream: QIB, EFA, SIW, <us> */
enum {
	RDMA_DRIVER_IONIC = RDMA_DRIVER_QIB + 3,
};
#endif

#endif /* IONIC_KCOMPAT */
