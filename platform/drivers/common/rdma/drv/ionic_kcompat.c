// SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB
#ifdef __FreeBSD__
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
#else
/*
 * Copyright (c) 2018-2020 Pensando Systems, Inc.  All rights reserved.
 */
#endif /* __FreeBSD__ */

#include <rdma/ib_pack.h>

#include "ionic_kcompat.h"

#define STRUCT_FIELD(header, field) \
	.struct_offset_bytes = offsetof(struct ib_unpacked_ ## header, field),      \
	.struct_size_bytes   = sizeof ((struct ib_unpacked_ ## header *) 0)->field, \
	.field_name          = #header ":" #field

static const struct ib_field eth_table[]  = {
	{ STRUCT_FIELD(eth, dmac_h),
	  .offset_words = 0,
	  .offset_bits  = 0,
	  .size_bits    = 32 },
	{ STRUCT_FIELD(eth, dmac_l),
	  .offset_words = 1,
	  .offset_bits  = 0,
	  .size_bits    = 16 },
	{ STRUCT_FIELD(eth, smac_h),
	  .offset_words = 1,
	  .offset_bits  = 16,
	  .size_bits    = 16 },
	{ STRUCT_FIELD(eth, smac_l),
	  .offset_words = 2,
	  .offset_bits  = 0,
	  .size_bits    = 32 },
	{ STRUCT_FIELD(eth, type),
	  .offset_words = 3,
	  .offset_bits  = 0,
	  .size_bits    = 16 }
};

static const struct ib_field vlan_table[]  = {
	{ STRUCT_FIELD(vlan, tag),
	  .offset_words = 0,
	  .offset_bits  = 0,
	  .size_bits    = 16 },
	{ STRUCT_FIELD(vlan, type),
	  .offset_words = 0,
	  .offset_bits  = 16,
	  .size_bits    = 16 }
};

static const struct ib_field ip4_table[]  = {
	{ STRUCT_FIELD(ip4, ver),
	  .offset_words = 0,
	  .offset_bits  = 0,
	  .size_bits    = 4 },
	{ STRUCT_FIELD(ip4, hdr_len),
	  .offset_words = 0,
	  .offset_bits  = 4,
	  .size_bits    = 4 },
	{ STRUCT_FIELD(ip4, tos),
	  .offset_words = 0,
	  .offset_bits  = 8,
	  .size_bits    = 8 },
	{ STRUCT_FIELD(ip4, tot_len),
	  .offset_words = 0,
	  .offset_bits  = 16,
	  .size_bits    = 16 },
	{ STRUCT_FIELD(ip4, id),
	  .offset_words = 1,
	  .offset_bits  = 0,
	  .size_bits    = 16 },
	{ STRUCT_FIELD(ip4, frag_off),
	  .offset_words = 1,
	  .offset_bits  = 16,
	  .size_bits    = 16 },
	{ STRUCT_FIELD(ip4, ttl),
	  .offset_words = 2,
	  .offset_bits  = 0,
	  .size_bits    = 8 },
	{ STRUCT_FIELD(ip4, protocol),
	  .offset_words = 2,
	  .offset_bits  = 8,
	  .size_bits    = 8 },
	{ STRUCT_FIELD(ip4, check),
	  .offset_words = 2,
	  .offset_bits  = 16,
	  .size_bits    = 16 },
	{ STRUCT_FIELD(ip4, saddr),
	  .offset_words = 3,
	  .offset_bits  = 0,
	  .size_bits    = 32 },
	{ STRUCT_FIELD(ip4, daddr),
	  .offset_words = 4,
	  .offset_bits  = 0,
	  .size_bits    = 32 }
};

static const struct ib_field udp_table[]  = {
	{ STRUCT_FIELD(udp, sport),
	  .offset_words = 0,
	  .offset_bits  = 0,
	  .size_bits    = 16 },
	{ STRUCT_FIELD(udp, dport),
	  .offset_words = 0,
	  .offset_bits  = 16,
	  .size_bits    = 16 },
	{ STRUCT_FIELD(udp, length),
	  .offset_words = 1,
	  .offset_bits  = 0,
	  .size_bits    = 16 },
	{ STRUCT_FIELD(udp, csum),
	  .offset_words = 1,
	  .offset_bits  = 16,
	  .size_bits    = 16 }
};

static const struct ib_field grh_table[]  = {
	{ STRUCT_FIELD(grh, ip_version),
	  .offset_words = 0,
	  .offset_bits  = 0,
	  .size_bits    = 4 },
	{ STRUCT_FIELD(grh, traffic_class),
	  .offset_words = 0,
	  .offset_bits  = 4,
	  .size_bits    = 8 },
	{ STRUCT_FIELD(grh, flow_label),
	  .offset_words = 0,
	  .offset_bits  = 12,
	  .size_bits    = 20 },
	{ STRUCT_FIELD(grh, payload_length),
	  .offset_words = 1,
	  .offset_bits  = 0,
	  .size_bits    = 16 },
	{ STRUCT_FIELD(grh, next_header),
	  .offset_words = 1,
	  .offset_bits  = 16,
	  .size_bits    = 8 },
	{ STRUCT_FIELD(grh, hop_limit),
	  .offset_words = 1,
	  .offset_bits  = 24,
	  .size_bits    = 8 },
	{ STRUCT_FIELD(grh, source_gid),
	  .offset_words = 2,
	  .offset_bits  = 0,
	  .size_bits    = 128 },
	{ STRUCT_FIELD(grh, destination_gid),
	  .offset_words = 6,
	  .offset_bits  = 0,
	  .size_bits    = 128 }
};

int roce_ud_header_unpack(void *buf, struct ib_ud_header *header)
{
	u16	type;

	ib_unpack(eth_table, ARRAY_SIZE(eth_table),
		  buf, &header->eth);
	buf += IB_ETH_BYTES;
	header->eth_present = 1;
	type = be16_to_cpu(header->eth.type);

	if (type == ETH_P_8021Q) {
		ib_unpack(vlan_table, ARRAY_SIZE(vlan_table),
			  buf, &header->vlan);
		header->vlan_present = 1;
		buf += IB_VLAN_BYTES;
		type = be16_to_cpu(header->vlan.type);
	}

	switch (type) {
		case ETH_P_IPV6:
			ib_unpack(grh_table, ARRAY_SIZE(grh_table),
				  buf, &header->grh);
			header->grh_present = 1;
			buf += IB_GRH_BYTES;
			if (header->grh.ip_version != 6) {
				pr_warn("Invalid GRH.ip_version %d\n",
					header->grh.ip_version);
				return -EINVAL;
			}

			if (header->grh.next_header != IPPROTO_UDP) {
				pr_warn("Invalid GRH.next_header 0x%02x\n",
					header->grh.next_header);
				return -EINVAL;
			}
			break;

		case ETH_P_IP:
			ib_unpack(ip4_table, ARRAY_SIZE(ip4_table),
				  buf, &header->ip4);
			header->ipv4_present = 1;
			buf += IB_IP4_BYTES;
			if (header->ip4.ver != 4) {
				pr_warn("Invalid IP4.ip_version %d\n",
					header->ip4.ver);
				return -EINVAL;
			}

			if (header->ip4.protocol != IPPROTO_UDP) {
				pr_warn("Invalid IP4.protocol %d\n",
					header->ip4.protocol);
				return -EINVAL;
			}
			break;

		default:
			pr_warn("Invalid Ether type 0x%x\n", type);
			return -EINVAL;
	}

	ib_unpack(udp_table, ARRAY_SIZE(udp_table),
		  buf, &header->udp);
	header->udp_present = 1;

	return 0;
}

#ifdef __FreeBSD__
static ssize_t kobj_attr_show(struct kobject *kobj, struct attribute *attr,
			      char *buf)
{
	struct kobj_attribute *kattr;
	ssize_t ret = -EIO;

	kattr = container_of(attr, struct kobj_attribute, attr);
	if (kattr->show)
		ret = kattr->show(kobj, kattr, buf);
	return ret;
}

static ssize_t kobj_attr_store(struct kobject *kobj, struct attribute *attr,
			       const char *buf, size_t count)
{
	struct kobj_attribute *kattr;
	ssize_t ret = -EIO;

	kattr = container_of(attr, struct kobj_attribute, attr);
	if (kattr->store)
		ret = kattr->store(kobj, kattr, buf, count);
	return ret;
}

const struct sysfs_ops kobj_sysfs_ops = {
	.show	= kobj_attr_show,
	.store	= kobj_attr_store,
};

#if __FreeBSD_version < 1200000
int sysfs_create_groups(struct kobject *kobj,
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
#endif

void sysfs_remove_groups(struct kobject *kobj,
			 const struct attribute_group **groups)
{
	int i;

	if (!groups)
		return;
	for (i = 0; groups[i]; i++)
		sysfs_remove_group(kobj, groups[i]);
}

#endif /* __FreeBSD__ */
#ifdef HAVE_CUSTOM_IB_SET_DEVICE_OPS
void ib_set_device_ops(struct ib_device *dev, const struct ib_device_ops *ops)
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
#endif /* HAVE_CUSTOM_IB_SET_DEVICE_OPS */
