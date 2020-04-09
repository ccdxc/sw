/* SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB */
/*
 * Copyright (c) 2018-2020 Pensando Systems, Inc.  All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef IONIC_KCOMPAT_H
#define IONIC_KCOMPAT_H

#include <rdma/ib_verbs.h>

/****************************************************************************
 *
 * Compatibility for kernel-only features not affected by OFA version
 *
 */
#include <linux/sysfs.h>
#include <linux/delay.h>
#include <linux/etherdevice.h>
#include <ionic_kpicompat.h>
#if !defined(NETAPP_PATCH) && __FreeBSD_version >= 1200000
#include <ck_queue.h>
#endif /* __FreeBSD_version >= 1200000 */

#define FW_INFO "[Firmware Info]: "

#define PTR_ERR_OR_ZERO(p) PTR_ERR(p)

#if __FreeBSD_version < 1200518
static inline int
sysfs_create_group_check_name(struct kobject *kobj,
			      const struct attribute_group *grp)
{
	struct attribute **attr;

	if (grp->name) {
		sysfs_create_group(kobj, grp);
	} else {
		for (attr = grp->attrs; *attr; ++attr)
			sysfs_create_file(kobj, *attr);
	}

	return 0;
}
#define sysfs_create_group sysfs_create_group_check_name

#endif /* __FreeBSD_version < 1200518 r351937 */
/* TODO: This is necessary because linuxkpi panics on !grp->name */
static inline int
sysfs_remove_group_check_name(struct kobject *kobj,
			      const struct attribute_group *grp)
{
	struct attribute **attr;

	if (grp->name) {
		sysfs_remove_group(kobj, grp);
	} else {
		for (attr = grp->attrs; *attr; ++attr)
			sysfs_remove_file(kobj, *attr);
	}

	return 0;
}
#define sysfs_remove_group sysfs_remove_group_check_name

#if defined(NETAPP_PATCH) || __FreeBSD_version < 1200518
static inline int sysfs_create_groups(struct kobject *kobj,
				      const struct attribute_group **groups)
{
	int error = 0;
	int i;

	if (!groups)
		return 0;

	for (i = 0; groups[i]; i++) {
		error = sysfs_create_group(kobj, groups[i]);
		if (error) {
			while (--i >= 0)
				sysfs_remove_group(kobj, groups[i]);
			break;
		}
	}
	return error;
}

#endif /* __FreeBSD_version < 1200518 r351937 */
#if defined(NETAPP_PATCH) || __FreeBSD_version < 1201502
static inline void sysfs_remove_groups(struct kobject *kobj,
				       const struct attribute_group **groups)
{
	int i;

	if (!groups)
		return;
	for (i = 0; groups[i]; i++)
		sysfs_remove_group(kobj, groups[i]);
}

#endif /* __FreeBSD_version < 1201502 r354613 */
#define local_irq_save(f) do { (f) = 0; } while (0)
#define local_irq_restore(f) (void)(f)

extern bool ionic_dyndbg_enable;

#undef print_hex_dump_debug
#define print_hex_dump_debug(...) do {					\
		if (ionic_dyndbg_enable)				\
			print_hex_dump(NULL, ##__VA_ARGS__);		\
	} while (0)

/* Create an xarray from a radix_tree_root */
#include <linux/radix-tree.h>

struct xarray {
	spinlock_t x_lock;
	struct radix_tree_root x_tree;
};
#define xa_tree(_xa) &(_xa)->x_tree
static inline void xa_init_flags(struct xarray *xa, gfp_t flags)
{
	spin_lock_init(&xa->x_lock);
	INIT_RADIX_TREE(xa_tree(xa), flags);
}

#define xa_lock(_xa) spin_lock(&(_xa)->x_lock)
#define xa_unlock(_xa) spin_unlock(&(_xa)->x_lock)

#define xa_iter radix_tree_iter
#define xa_for_each_slot(_xa, _slot, _iter)				\
	radix_tree_for_each_slot((_slot), xa_tree(_xa), (_iter), 0)
#define xa_load(_xa, _idx) radix_tree_lookup(xa_tree(_xa), _idx)
#define xa_destroy(_xa)

static inline void *xa_store(struct xarray *xa, unsigned long idx,
			     void *item, gfp_t unused)
{
	int ret;

	xa_lock(xa);
	ret = radix_tree_insert(xa_tree(xa), idx, item);
	xa_unlock(xa);

	return (ret ? ERR_PTR(ret) : item);
}

static inline int xa_err(void *item)
{
	return (IS_ERR(item) ? PTR_ERR(item) : 0);
}

static inline void xa_erase(struct xarray *xa, unsigned long idx)
{
	xa_lock(xa);
	radix_tree_delete(xa_tree(xa), idx);
	xa_unlock(xa);
}

#define __static_assert(expr, msg, ...) _Static_assert(expr, msg)
#define static_assert(expr, ...) __static_assert(expr, ##__VA_ARGS__, #expr)

/****************************************************************************
 *
 * Compatibility for OFED features that may be affected by OFA version
 *
 */
#define RDMA_CREATE_AH_SLEEPABLE 0

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

static inline int ib_get_eth_speed(struct ib_device *dev, u8 port_num,
				   u8 *speed, u8 *width)
{
	*width = IB_WIDTH_4X;
	*speed = IB_SPEED_EDR;
	return 0;
}

struct ib_device_ops {
	int (*post_send)(struct ib_qp *qp, struct ib_send_wr *send_wr,
			 struct ib_send_wr **bad_send_wr);
	int (*post_recv)(struct ib_qp *qp, struct ib_recv_wr *recv_wr,
			 struct ib_recv_wr **bad_recv_wr);
	int (*post_srq_recv)(struct ib_srq *srq,
			     struct ib_recv_wr *recv_wr,
			     struct ib_recv_wr **bad_recv_wr);
	void (*drain_rq)(struct ib_qp *qp);
	void (*drain_sq)(struct ib_qp *qp);
	int (*poll_cq)(struct ib_cq *cq, int num_entries, struct ib_wc *wc);
	int (*peek_cq)(struct ib_cq *cq, int wc_cnt);
	int (*req_notify_cq)(struct ib_cq *cq, enum ib_cq_notify_flags flags);
	int (*req_ncomp_notif)(struct ib_cq *cq, int wc_cnt);
	int (*query_device)(struct ib_device *device,
			    struct ib_device_attr *device_attr,
			    struct ib_udata *udata);
	int (*modify_device)(struct ib_device *device, int device_modify_mask,
			     struct ib_device_modify *device_modify);
	void (*get_dev_fw_str)(struct ib_device *device, char *str,
			       size_t str_len);
	int (*query_port)(struct ib_device *device, u8 port_num,
			  struct ib_port_attr *port_attr);
	int (*modify_port)(struct ib_device *device, u8 port_num,
			   int port_modify_mask,
			   struct ib_port_modify *port_modify);
	int (*get_port_immutable)(struct ib_device *device, u8 port_num,
				  struct ib_port_immutable *immutable);
	enum rdma_link_layer (*get_link_layer)(struct ib_device *device,
					       u8 port_num);
	struct net_device *(*get_netdev)(struct ib_device *device,
					 u8 port_num);
	int (*query_gid)(struct ib_device *device, u8 port_num, int index,
			 union ib_gid *gid);
	int (*add_gid)(struct ib_device *device, u8 port, unsigned int index,
		       const union ib_gid *gid, const struct ib_gid_attr *attr,
		       void **context);
	int (*del_gid)(struct ib_device *device, u8 port, unsigned int index,
		       void **context);
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
	struct ib_ah *(*create_ah)(struct ib_pd *pd,
				   struct rdma_ah_attr *ah_attr,
				   struct ib_udata *udata);
	int (*modify_ah)(struct ib_ah *ah, struct rdma_ah_attr *ah_attr);
	int (*query_ah)(struct ib_ah *ah, struct rdma_ah_attr *ah_attr);
	int (*destroy_ah)(struct ib_ah *ah);
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
			int qp_attr_mask,
			struct ib_qp_init_attr *qp_init_attr);
	int (*destroy_qp)(struct ib_qp *qp);
	struct ib_cq *(*create_cq)(struct ib_device *device,
				   const struct ib_cq_init_attr *attr,
				   struct ib_ucontext *context,
				   struct ib_udata *udata);
	int (*modify_cq)(struct ib_cq *cq, u16 cq_count, u16 cq_period);
	int (*destroy_cq)(struct ib_cq *cq);
	int (*resize_cq)(struct ib_cq *cq, int cqe, struct ib_udata *udata);
	struct ib_mr *(*get_dma_mr)(struct ib_pd *pd, int mr_access_flags);
	struct ib_mr *(*reg_user_mr)(struct ib_pd *pd, u64 start, u64 length,
				     u64 virt_addr, int mr_access_flags,
				     struct ib_udata *udata);
	int (*rereg_user_mr)(struct ib_mr *mr, int flags, u64 start,
			     u64 length, u64 virt_addr, int mr_access_flags,
			     struct ib_pd *pd, struct ib_udata *udata);
	int (*dereg_mr)(struct ib_mr *mr);
	struct ib_mr *(*alloc_mr)(struct ib_pd *pd, enum ib_mr_type mr_type,
				  u32 max_num_sg);
	int (*map_mr_sg)(struct ib_mr *mr, struct scatterlist *sg,
			 int sg_nents, unsigned int *sg_offset);
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

	SET_DEVICE_OP(add_gid);
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
	SET_DEVICE_OP(del_gid);
	SET_DEVICE_OP(dereg_mr);
	SET_DEVICE_OP(destroy_ah);
	SET_DEVICE_OP(destroy_cq);
	SET_DEVICE_OP(destroy_qp);
	SET_DEVICE_OP(destroy_srq);
	SET_DEVICE_OP(detach_mcast);
	SET_DEVICE_OP(disassociate_ucontext);
	SET_DEVICE_OP(drain_rq);
	SET_DEVICE_OP(drain_sq);
	SET_DEVICE_OP(get_dev_fw_str);
	SET_DEVICE_OP(get_dma_mr);
	SET_DEVICE_OP(get_hw_stats);
	SET_DEVICE_OP(get_link_layer);
	SET_DEVICE_OP(get_netdev);
	SET_DEVICE_OP(get_port_immutable);
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
	SET_DEVICE_OP(query_gid);
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

enum ib_port_phys_state {
	IB_PORT_PHYS_STATE_SLEEP = 1,
	IB_PORT_PHYS_STATE_POLLING = 2,
	IB_PORT_PHYS_STATE_DISABLED = 3,
	IB_PORT_PHYS_STATE_PORT_CONFIGURATION_TRAINING = 4,
	IB_PORT_PHYS_STATE_LINK_UP = 5,
	IB_PORT_PHYS_STATE_LINK_ERROR_RECOVERY = 6,
	IB_PORT_PHYS_STATE_PHY_TEST = 7,
};

#define ibdev_dbg(ibdev, ...) do {					\
		if (ionic_dyndbg_enable)				\
			dev_info(&(ibdev)->dev, ##__VA_ARGS__);		\
	} while (0)
#define ibdev_info(ibdev, ...)	dev_info(&(ibdev)->dev, ##__VA_ARGS__)
#define ibdev_warn(ibdev, ...)	dev_warn(&(ibdev)->dev, ##__VA_ARGS__)
#define ibdev_err(ibdev, ...)	dev_err(&(ibdev)->dev, ##__VA_ARGS__)

#define ibdev_warn_ratelimited(ibdev, ...)				\
	dev_warn_ratelimited(&(ibdev)->dev, ##__VA_ARGS__)

#endif /* IONIC_KCOMPAT_H */
