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

#include <linux/version.h>
#include <linux/netdevice.h>
#include <rdma/ib_verbs.h>

#ifdef RHEL_RELEASE_VERSION
#define IONIC_KCOMPAT_VERSION_PRIOR_TO(LX_MAJ, LX_MIN, RH_MAJ, RH_MIN) \
	(RHEL_RELEASE_CODE < RHEL_RELEASE_VERSION(RH_MAJ, RH_MIN))
#else
#define IONIC_KCOMPAT_VERSION_PRIOR_TO(LX_MAJ, LX_MIN, RH_MAJ, RH_MIN) \
	(LINUX_VERSION_CODE < KERNEL_VERSION(LX_MAJ, LX_MIN, 0))
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 3,11, /* RHEL */ 99,99)
#define netdev_notifier_info_to_dev(ptr) (ptr)
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 4,8, /* RHEL */ 7,6)
#else /* 4.9.0 and later */
#define HAVE_QP_RWQ_IND_TBL
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 4,9, /* RHEL */ 7,6)
#define IB_DEVICE_NODE_DESC_MAX sizeof(((struct ib_device *)0)->node_desc)
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 4,10, /* RHEL */ 7,6)

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

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 4,10, /* RHEL */ 99,99)
#else /* 4.10.0 and later */
#define HAVE_NETDEV_MAX_MTU
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 4,11, /* RHEL */ 7,6)
#define HAVE_REQUIRED_DMA_DEVICE
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 4,12, /* RHEL */ 7,6)

#define rdma_ah_attr ib_ah_attr
#define rdma_ah_read_grh(attr) (&(attr)->grh)

#else /* 4.12.0 and later */
#define HAVE_RDMA_AH_ATTR_TYPE_ROCE
#define HAVE_UMEM_PAGE_SHIFT
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 4,14, /* RHEL */ 7,6)

static inline bool ib_srq_has_cq(enum ib_srq_type srq_type)
{
	return srq_type == IB_SRQT_XRC;
}

static inline int ib_get_eth_speed(struct ib_device *dev, u8 port_num,
				   u8 *speed, u8 *width)
{
	*width = IB_WIDTH_4X;
	*speed = IB_SPEED_EDR;
	return 0;
}

#define HAVE_GET_DEV_FW_STR_LEN

#else /* 4.14.0 and later */
#define HAVE_SRQ_EXT_CQ
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 4,14, /* RHEL */ 99,99)
#else /* 4.14.0 and later */
#define HAVE_GET_VECTOR_AFFINITY
#define HAVE_QP_INIT_SRC_QPN
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 4,17, /* RHEL */ 99,99)
#define HAVE_IB_GID_DEV_PORT_INDEX
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 4,19, /* RHEL */ 99,99)
#define HAVE_REQUIRED_IB_GID
#define HAVE_IBDEV_PORT_CAP_FLAGS
#define ud_wr(wr) ud_wr((struct ib_send_wr *)(wr))
#define rdma_wr(wr) rdma_wr((struct ib_send_wr *)(wr))
#define atomic_wr(wr) atomic_wr((struct ib_send_wr *)(wr))
#define reg_wr(wr) reg_wr((struct ib_send_wr *)(wr))
#else
#define HAVE_AH_ATTR_CACHED_GID
#define HAVE_CONST_IB_WR
#define HAVE_IBDEV_IP_GIDS
#define HAVE_IBDEV_MAX_SEND_RECV_SGE
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 4,20, /* RHEL */ 99,99)
#define ib_modify_qp_is_ok(cur_state, new_state, qp_type, attr_mask) \
	ib_modify_qp_is_ok(cur_state, new_state, qp_type, attr_mask, \
			   IB_LINK_LAYER_ETHERNET)
#else
#define HAVE_IB_REGISTER_DEVICE_NAME
#endif

#if IONIC_KCOMPAT_VERSION_PRIOR_TO(/* Linux */ 5,0, /* RHEL */ 99,99)
#undef dma_alloc_coherent
#define dma_alloc_coherent dma_zalloc_coherent
#define RDMA_CREATE_AH_SLEEPABLE 0
#else
#define HAVE_IB_DEVICE_OPS
#define HAVE_CREATE_AH_FLAGS
#endif

/* other compat for not yet upstream changes */

/* change ib_gid_to_network_type to accept const ib_gid */
#define ib_gid_to_network_type(gid_type, gid) \
	ib_gid_to_network_type(gid_type, (union ib_gid *)(gid))

#endif
