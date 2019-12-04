/* SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB */
/*
 * Copyright (c) 2018-2019 Pensando Systems, Inc.  All rights reserved.
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

#ifndef IONIC_KCOMPAT
#define IONIC_KCOMPAT

#include <linux/sysfs.h>
#include <linux/delay.h>
#include <linux/etherdevice.h>
#include <rdma/ib_pack.h>
#include <rdma/ib_verbs.h>
#include <ionic_kpicompat.h>

#if __FreeBSD_version >= 1200000
#include <ck_queue.h>
#endif

/* marks code inserted to silence false positive warnings */
#define IONIC_STATIC_ANALYSIS_HINTS_NOT_FOR_UPSTREAM 1

#define rdma_ah_attr ib_ah_attr
#define rdma_ah_read_grh(attr) (&(attr)->grh)

static inline void rdma_ah_set_sl(struct ib_ah_attr *attr, u8 sl)
{
	attr->sl = sl;
}

static inline void rdma_ah_set_port_num(struct ib_ah_attr *attr, u8 port_num)
{
	attr->port_num = port_num;
}

static inline void rdma_ah_set_grh(struct ib_ah_attr *attr,
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

static inline void rdma_ah_set_dgid_raw(struct ib_ah_attr *attr, void *dgid)
{
	struct ib_global_route *grh = rdma_ah_read_grh(attr);

	memcpy(grh->dgid.raw, dgid, sizeof(grh->dgid));
}

/**
 * roce_ud_header_unpack - Unpack UD header struct from RoCE wire format
 * @header:UD header struct
 * @buf:Buffer to unpack into
 *
 * roce_ud_header_pack() unpacks the UD header structure @header from RoCE wire
 * format in the buffer @buf.
 */
int roce_ud_header_unpack(void *buf, struct ib_ud_header *header);

extern const struct sysfs_ops kobj_sysfs_ops;

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

int sysfs_create_groups(struct kobject *kobj,
			const struct attribute_group **groups);

void sysfs_remove_groups(struct kobject *kobj,
			 const struct attribute_group **groups);

static inline bool
dev_is_pci(struct device *d)
{
	return true;
}

#define refcount_t            atomic_t
#define refcount_set          atomic_set
#define refcount_dec_and_test atomic_dec_and_test
#define refcount_inc          atomic_inc

/* Create an xarray that includes a radix_tree_root and a spinlock */
#include <linux/radix-tree.h>

struct xarray {
	spinlock_t x_lock;
	struct radix_tree_root x_tree;
};
#define xa_tree(_xa) &(_xa)->x_tree

#define xa_lock(_xa) spin_lock(&(_xa)->x_lock)
#define xa_unlock(_xa) spin_unlock(&(_xa)->x_lock);
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

static inline bool ib_srq_has_cq(enum ib_srq_type srq_type)
{
	return (srq_type == IB_SRQT_XRC);
}

static inline int ib_get_eth_speed(struct ib_device *ibdev, u8 port,
				   u8 *speed, u8 *width)
{
	*width = IB_WIDTH_4X;
	*speed = IB_SPEED_EDR;

	return 0;
}

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

#define HAVE_GET_DEV_FW_STR_LEN

#define HAVE_NETDEV_IF_MTU

#define local_irq_save(f) do { (f) = 0; } while (0)
#define local_irq_restore(f) (void)(f)

enum ib_port_phys_state {
	IB_PORT_PHYS_STATE_SLEEP = 1,
	IB_PORT_PHYS_STATE_POLLING = 2,
	IB_PORT_PHYS_STATE_DISABLED = 3,
	IB_PORT_PHYS_STATE_PORT_CONFIGURATION_TRAINING = 4,
	IB_PORT_PHYS_STATE_LINK_UP = 5,
	IB_PORT_PHYS_STATE_LINK_ERROR_RECOVERY = 6,
	IB_PORT_PHYS_STATE_PHY_TEST = 7,
};

#endif /* IONIC_KCOMPAT */
