/*
 * Copyright (c) 2018 Pensando Systems, Inc.  All rights reserved.
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
#include <rdma/ib_verbs.h>

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

static inline int kstrtobool(const char *s, bool *res)
{
	int rc, val;

	rc = kstrtoint(s, 0, &val);

	if (!rc)
		*res = val;

	return rc;
}

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

#include <linux/radix-tree.h>

struct xarray {
	spinlock_t x_lock;
	struct radix_tree_root x_tree;
};

static inline void xa_init(struct xarray *xa)
{
	spin_lock_init(&xa->x_lock);
	INIT_RADIX_TREE(&xa->x_tree, GFP_KERNEL);
}

static inline void *xa_load(struct xarray *xa, unsigned long idx)
{
	return radix_tree_lookup(&xa->x_tree, idx);
}

static inline void *xa_store(struct xarray *xa, unsigned long idx, void *item,
			     gfp_t unused)
{
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&xa->x_lock, flags);
	ret = radix_tree_insert(&xa->x_tree, idx, item);
	spin_unlock_irqrestore(&xa->x_lock, flags);

	return (ret ? ERR_PTR(ret) : item);
}

static inline int xa_err(void *item)
{
	return (IS_ERR(item) ? PTR_ERR(item) : 0);
}

static inline void xa_erase(struct xarray *xa, unsigned long idx)
{
	unsigned long flags;

	spin_lock_irqsave(&xa->x_lock, flags);
	radix_tree_delete(&xa->x_tree, idx);
	spin_unlock_irqrestore(&xa->x_lock, flags);
}

static inline void xa_destroy(struct xarray *xa)
{
	/* No equivalent for radix-tree */
}

#ifndef HAVE_REAL_SRCU

/* RCU cruft */
#undef  rcu_read_lock
#define rcu_read_lock()							\
	unsigned long _rcuflags;					\
	read_lock_irqsave(&dev->rcu_lock, _rcuflags)

#undef  rcu_read_unlock
#define rcu_read_unlock()						\
	read_unlock_irqrestore(&dev->rcu_lock, _rcuflags)

#undef  synchronize_rcu
#define synchronize_rcu()						\
	do {								\
		unsigned long _rcuflags;				\
		write_lock_irqsave(&dev->rcu_lock, _rcuflags);		\
		write_unlock_irqrestore(&dev->rcu_lock, _rcuflags);	\
	} while (0)

#endif /* HAVE_REAL_SRCU */

#endif /* IONIC_KCOMPAT */
