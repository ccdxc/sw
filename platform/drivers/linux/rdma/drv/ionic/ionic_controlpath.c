// SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB
/*
 * Copyright (c) 2018-2020 Pensando Systems, Inc.  All rights reserved.
 */

#include <linux/module.h>
#include <linux/printk.h>
#include <rdma/ib_addr.h>
#include <rdma/ib_cache.h>
#include <rdma/ib_user_verbs.h>

#include "ionic_fw.h"
#include "ionic_ibdev.h"

#if defined(HAVE_IB_API_UDATA) || defined(HAVE_RDMA_UDATA_DRV_CTX)
#include <rdma/uverbs_ioctl.h>

#endif
#define ionic_set_ecn(tos)   (((tos) | 2u) & ~1u)
#define ionic_clear_ecn(tos)  ((tos) & ~3u)

static int ionic_validate_qdesc(struct ionic_qdesc *q)
{
	if (!q->addr || !q->size || !q->mask ||
	    !q->depth_log2 || !q->stride_log2)
		return -EINVAL;

	if (q->addr & (PAGE_SIZE - 1))
		return -EINVAL;

	if (q->mask != BIT(q->depth_log2) - 1)
		return -EINVAL;

	if (q->size < BIT_ULL(q->depth_log2 + q->stride_log2))
		return -EINVAL;

	return 0;
}

static int ionic_validate_qdesc_zero(struct ionic_qdesc *q)
{
	if (q->addr || q->size || q->mask || q->depth_log2 || q->stride_log2)
		return -EINVAL;

	return 0;
}

static int ionic_get_pdid(struct ionic_ibdev *dev, u32 *pdid)
{
	int rc;

	mutex_lock(&dev->inuse_lock);
	rc = ionic_resid_get(&dev->inuse_pdid);
	mutex_unlock(&dev->inuse_lock);

	if (rc >= 0) {
		*pdid = rc;
		rc = 0;
	}

	return rc;
}

static int ionic_get_ahid(struct ionic_ibdev *dev, u32 *ahid)
{
	unsigned long irqflags;
	int rc;

	spin_lock_irqsave(&dev->inuse_splock, irqflags);
	rc = ionic_resid_get(&dev->inuse_ahid);
	spin_unlock_irqrestore(&dev->inuse_splock, irqflags);

	if (rc >= 0) {
		*ahid = rc;
		rc = 0;
	}

	return rc;
}

static int ionic_get_mrid(struct ionic_ibdev *dev, u32 *mrid)
{
	int rc;

	mutex_lock(&dev->inuse_lock);
	/* wrap to 1, skip reserved lkey */
	rc = ionic_resid_get_wrap(&dev->inuse_mrid, 1);
	if (rc >= 0) {
		*mrid = ionic_mrid(rc, dev->next_mrkey++);
		rc = 0;
	}
	mutex_unlock(&dev->inuse_lock);

	return rc;
}

static int ionic_get_cqid(struct ionic_ibdev *dev, u32 *cqid)
{
	int rc;

	mutex_lock(&dev->inuse_lock);
	rc = ionic_resid_get(&dev->inuse_cqid);
	mutex_unlock(&dev->inuse_lock);

	if (rc >= 0) {
		*cqid = rc + dev->cq_base;
		rc = 0;
	}

	return rc;
}

static int ionic_get_gsi_qpid(struct ionic_ibdev *dev, u32 *qpid)
{
	int rc = 0;

	mutex_lock(&dev->inuse_lock);
	if (test_bit(IB_QPT_GSI, dev->inuse_qpid.inuse)) {
		rc = -EINVAL;
	} else {
		set_bit(IB_QPT_GSI, dev->inuse_qpid.inuse);
		*qpid = IB_QPT_GSI;
	}
	mutex_unlock(&dev->inuse_lock);

	return rc;
}

static int ionic_get_qpid(struct ionic_ibdev *dev, u32 *qpid)
{
	int rc = 0;

	mutex_lock(&dev->inuse_lock);
	rc = ionic_resid_get_shared(&dev->inuse_qpid, 2,
				    dev->inuse_qpid.next_id,
				    dev->size_qpid);
	if (rc >= 0) {
		dev->inuse_qpid.next_id = rc + 1;
		*qpid = rc;
		rc = 0;
	}
	mutex_unlock(&dev->inuse_lock);

	return rc;
}

static void ionic_put_pdid(struct ionic_ibdev *dev, u32 pdid)
{
	ionic_resid_put(&dev->inuse_pdid, pdid);
}

static void ionic_put_ahid(struct ionic_ibdev *dev, u32 ahid)
{
	ionic_resid_put(&dev->inuse_ahid, ahid);
}

static void ionic_put_mrid(struct ionic_ibdev *dev, u32 mrid)
{
	ionic_resid_put(&dev->inuse_mrid, ionic_mrid_index(mrid));
}

static void ionic_put_cqid(struct ionic_ibdev *dev, u32 cqid)
{
	ionic_resid_put(&dev->inuse_cqid, cqid - dev->cq_base);
}

static void ionic_put_qpid(struct ionic_ibdev *dev, u32 qpid)
{
	ionic_resid_put(&dev->inuse_qpid, qpid);
}

int ionic_get_res(struct ionic_ibdev *dev, struct ionic_tbl_res *res)
{
	int rc = 0;

	mutex_lock(&dev->inuse_lock);
	rc = ionic_buddy_get(&dev->inuse_restbl, res->tbl_order);
	mutex_unlock(&dev->inuse_lock);

	if (rc < 0) {
		res->tbl_order = IONIC_RES_INVALID;
		res->tbl_pos = 0;
		return rc;
	}

	res->tbl_pos = rc << (dev->cl_stride - dev->pte_stride);

	return 0;
}

bool ionic_put_res(struct ionic_ibdev *dev, struct ionic_tbl_res *res)
{
	if (res->tbl_order == IONIC_RES_INVALID)
		return false;

	res->tbl_pos >>= dev->cl_stride - dev->pte_stride;

	mutex_lock(&dev->inuse_lock);
	ionic_buddy_put(&dev->inuse_restbl, res->tbl_pos, res->tbl_order);
	mutex_unlock(&dev->inuse_lock);

	res->tbl_order = IONIC_RES_INVALID;
	res->tbl_pos = 0;

	return true;
}

#ifdef HAVE_IB_ALLOC_UCTX_OBJ
static int ionic_alloc_ucontext(struct ib_ucontext *ibctx,
				struct ib_udata *udata)
#else
static struct ib_ucontext *ionic_alloc_ucontext(struct ib_device *ibdev,
						struct ib_udata *udata)
#endif
{
#ifdef HAVE_IB_ALLOC_UCTX_OBJ
	struct ionic_ibdev *dev = to_ionic_ibdev(ibctx->device);
	struct ionic_ctx *ctx = to_ionic_ctx(ibctx);
#else
	struct ionic_ibdev *dev = to_ionic_ibdev(ibdev);
	struct ionic_ctx *ctx;
#endif
	struct ionic_ctx_req req;
	struct ionic_ctx_resp resp = {0};
	phys_addr_t db_phys = 0;
	int rc;

	rc = ib_copy_from_udata(&req, udata, sizeof(req));
	if (rc)
		goto err_ctx;

#ifndef HAVE_IB_ALLOC_UCTX_OBJ
	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx) {
		rc = -ENOMEM;
		goto err_ctx;
	}

#endif
	/* try to allocate dbid for user ctx */
	rc = ionic_api_get_dbid(dev->handle, &ctx->dbid, &db_phys);
	if (rc < 0)
		goto err_dbid;

	ibdev_dbg(&dev->ibdev, "user space dbid %u\n", ctx->dbid);

	mutex_init(&ctx->mmap_mut);
	ctx->mmap_off = PAGE_SIZE;
	INIT_LIST_HEAD(&ctx->mmap_list);

	ctx->mmap_dbell.offset = 0;
	ctx->mmap_dbell.size = PAGE_SIZE;
	ctx->mmap_dbell.pfn = PHYS_PFN(db_phys);
	ctx->mmap_dbell.writecombine = false;
	list_add(&ctx->mmap_dbell.ctx_ent, &ctx->mmap_list);

	resp.page_shift = PAGE_SHIFT;

	resp.dbell_offset = 0;

	resp.version = dev->rdma_version;
	resp.qp_opcodes = dev->qp_opcodes;
	resp.admin_opcodes = dev->admin_opcodes;

	resp.sq_qtype = dev->sq_qtype;
	resp.rq_qtype = dev->rq_qtype;
	resp.cq_qtype = dev->cq_qtype;
	resp.admin_qtype = dev->aq_qtype;
	resp.max_stride = dev->max_stride;
	resp.max_spec = ionic_spec;

	rc = ib_copy_to_udata(udata, &resp, sizeof(resp));
	if (rc)
		goto err_resp;

#ifdef HAVE_IB_ALLOC_UCTX_OBJ
	return 0;
#else
	return &ctx->ibctx;
#endif

err_resp:
	ionic_api_put_dbid(dev->handle, ctx->dbid);
err_dbid:
#ifdef HAVE_IB_ALLOC_UCTX_OBJ
err_ctx:
	return rc;
#else
	kfree(ctx);
err_ctx:
	return ERR_PTR(rc);
#endif
}

#ifdef HAVE_IB_DEALLOC_UCTX_VOID
static void ionic_dealloc_ucontext(struct ib_ucontext *ibctx)
#else
static int ionic_dealloc_ucontext(struct ib_ucontext *ibctx)
#endif
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibctx->device);
	struct ionic_ctx *ctx = to_ionic_ctx(ibctx);

	list_del(&ctx->mmap_dbell.ctx_ent);

	if (WARN_ON(!list_empty(&ctx->mmap_list)))
		list_del(&ctx->mmap_list);

	ionic_api_put_dbid(dev->handle, ctx->dbid);
#ifndef HAVE_IB_ALLOC_UCTX_OBJ
	kfree(ctx);
#endif

#ifndef HAVE_IB_DEALLOC_UCTX_VOID
	return 0;
#endif
}

static int ionic_mmap(struct ib_ucontext *ibctx, struct vm_area_struct *vma)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibctx->device);
	struct ionic_ctx *ctx = to_ionic_ctx(ibctx);
	struct ionic_mmap_info *info;
	unsigned long size = vma->vm_end - vma->vm_start;
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
	int rc = 0;

	mutex_lock(&ctx->mmap_mut);

	list_for_each_entry(info, &ctx->mmap_list, ctx_ent)
		if (info->offset == offset)
			goto found;

	mutex_unlock(&ctx->mmap_mut);

	/* not found */
	ibdev_dbg(&dev->ibdev, "not found %#lx\n", offset);
	rc = -EINVAL;
	goto out;

found:
	list_del_init(&info->ctx_ent);
	mutex_unlock(&ctx->mmap_mut);

	if (info->size != size) {
		ibdev_dbg(&dev->ibdev, "invalid size %#lx (%#lx)\n",
			  size, info->size);
		rc = -EINVAL;
		goto out;
	}

	ibdev_dbg(&dev->ibdev, "writecombine? %d\n", info->writecombine);
	if (info->writecombine)
		vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	else
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	ibdev_dbg(&dev->ibdev, "remap st %#lx pf %#lx sz %#lx\n",
		  vma->vm_start, info->pfn, size);
	rc = io_remap_pfn_range(vma, vma->vm_start, info->pfn, size,
				vma->vm_page_prot);
	if (rc)
		ibdev_dbg(&dev->ibdev, "remap failed %d\n", rc);

out:
	return rc;
}

#ifdef HAVE_IB_ALLOC_PD_OBJ
#ifdef HAVE_IB_API_UDATA
static int ionic_alloc_pd(struct ib_pd *ibpd, struct ib_udata *udata)
#else
static int ionic_alloc_pd(struct ib_pd *ibpd,
			  struct ib_ucontext *ibctx,
			  struct ib_udata *udata)
#endif
#else
static struct ib_pd *ionic_alloc_pd(struct ib_device *ibdev,
				    struct ib_ucontext *ibctx,
				    struct ib_udata *udata)
#endif
{
#ifdef HAVE_IB_ALLOC_PD_OBJ
	struct ionic_ibdev *dev = to_ionic_ibdev(ibpd->device);
	struct ionic_pd *pd = to_ionic_pd(ibpd);
#else
	struct ionic_ibdev *dev = to_ionic_ibdev(ibdev);
	struct ionic_pd *pd;
#endif
	int rc;

#ifndef HAVE_IB_ALLOC_PD_OBJ
	pd = kzalloc(sizeof(*pd), GFP_KERNEL);
	if (!pd) {
		rc = -ENOMEM;
		goto err_pd;
	}

#endif
	rc = ionic_get_pdid(dev, &pd->pdid);
	if (rc)
		goto err_pdid;

#ifdef HAVE_IB_ALLOC_PD_OBJ
	return 0;
#else
	return &pd->ibpd;
#endif

err_pdid:
#ifdef HAVE_IB_ALLOC_PD_OBJ
	return rc;
#else
	kfree(pd);
err_pd:
	return ERR_PTR(rc);
#endif
}

#ifdef HAVE_IB_DEALLOC_PD_VOID
#ifdef HAVE_IB_API_UDATA
static void ionic_dealloc_pd(struct ib_pd *ibpd, struct ib_udata *udata)
#else
static void ionic_dealloc_pd(struct ib_pd *ibpd)
#endif
#else
static int ionic_dealloc_pd(struct ib_pd *ibpd)
#endif
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibpd->device);
	struct ionic_pd *pd = to_ionic_pd(ibpd);

	ionic_put_pdid(dev, pd->pdid);
#ifndef HAVE_IB_ALLOC_PD_OBJ
	kfree(pd);
#endif

#ifndef HAVE_IB_DEALLOC_PD_VOID
	return 0;
#endif
}

static int ionic_build_hdr(struct ionic_ibdev *dev,
			   struct ib_ud_header *hdr,
			   const struct rdma_ah_attr *attr,
			   bool want_ecn)
{
	const struct ib_global_route *grh;
#ifndef HAVE_AH_ATTR_CACHED_GID
	struct ib_gid_attr sgid_attr;
	union ib_gid sgid, dgid_copy;
	u8 smac[ETH_ALEN];
#endif
	enum rdma_network_type net;
	u16 vlan;
	int rc;

	if (attr->ah_flags != IB_AH_GRH)
		return -EINVAL;
#ifdef HAVE_RDMA_AH_ATTR_TYPE_ROCE
	if (attr->type != RDMA_AH_ATTR_TYPE_ROCE)
		return -EINVAL;
#endif

	grh = rdma_ah_read_grh(attr);

#ifdef HAVE_AH_ATTR_CACHED_GID
	vlan = rdma_vlan_dev_vlan_id(grh->sgid_attr->ndev);
	net = rdma_gid_attr_network_type(grh->sgid_attr);
#else
	rc = ib_get_cached_gid(&dev->ibdev, 1, grh->sgid_index,
			       &sgid, &sgid_attr);
	if (rc)
		return rc;

	if (!sgid_attr.ndev)
		return -ENXIO;

	ether_addr_copy(smac, sgid_attr.ndev->dev_addr);
	vlan = rdma_vlan_dev_vlan_id(sgid_attr.ndev);
	net = ib_gid_to_network_type(sgid_attr.gid_type, &sgid);

	dev_put(sgid_attr.ndev); /* hold from ib_get_cached_gid */
	sgid_attr.ndev = NULL;

	dgid_copy = grh->dgid;
	if (net != ib_gid_to_network_type(sgid_attr.gid_type, &dgid_copy))
		return -EINVAL;
#endif

	rc = ib_ud_header_init(0,	/* no payload */
			       0,	/* no lrh */
			       1,	/* yes eth */
			       vlan != 0xffff,
			       0,	/* no grh */
			       net == RDMA_NETWORK_IPV4 ? 4 : 6,
			       1,	/* yes udp */
			       0,	/* no imm */
			       hdr);
	if (rc)
		return rc;

#ifdef HAVE_AH_ATTR_CACHED_GID
	ether_addr_copy(hdr->eth.smac_h, grh->sgid_attr->ndev->dev_addr);
#else
	ether_addr_copy(hdr->eth.smac_h, smac);
#endif
#ifdef HAVE_RDMA_AH_ATTR_TYPE_ROCE
	ether_addr_copy(hdr->eth.dmac_h, attr->roce.dmac);
#else
	ether_addr_copy(hdr->eth.dmac_h, attr->dmac);
#endif

	if (net == RDMA_NETWORK_IPV4) {
		hdr->eth.type = cpu_to_be16(ETH_P_IP);
		hdr->ip4.frag_off = cpu_to_be16(0x4000); /* don't fragment */
		hdr->ip4.ttl = grh->hop_limit;
		hdr->ip4.tot_len = cpu_to_be16(0xffff);
#ifdef HAVE_AH_ATTR_CACHED_GID
		hdr->ip4.saddr =
			*(const __be32 *)(grh->sgid_attr->gid.raw + 12);
#else
		hdr->ip4.saddr = *(const __be32 *)(sgid.raw + 12);
#endif
		hdr->ip4.daddr = *(const __be32 *)(grh->dgid.raw + 12);

		if (want_ecn)
			hdr->ip4.tos = ionic_set_ecn(grh->traffic_class);
		else
			hdr->ip4.tos = ionic_clear_ecn(grh->traffic_class);
	} else {
		hdr->eth.type = cpu_to_be16(ETH_P_IPV6);
		hdr->grh.flow_label = cpu_to_be32(grh->flow_label);
		hdr->grh.hop_limit = grh->hop_limit;
#ifdef HAVE_AH_ATTR_CACHED_GID
		hdr->grh.source_gid = grh->sgid_attr->gid;
#else
		hdr->grh.source_gid = sgid;
#endif
		hdr->grh.destination_gid = grh->dgid;

		if (want_ecn)
			hdr->grh.traffic_class =
				ionic_set_ecn(grh->traffic_class);
		else
			hdr->grh.traffic_class =
				ionic_clear_ecn(grh->traffic_class);
	}

	if (vlan != 0xffff) {
		vlan |= attr->sl << 13; /* 802.1q PCP */
		hdr->vlan.tag = cpu_to_be16(vlan);
		hdr->vlan.type = hdr->eth.type;
		hdr->eth.type = cpu_to_be16(ETH_P_8021Q);
	}

	hdr->udp.sport = cpu_to_be16(IONIC_ROCE_UDP_SPORT);
	hdr->udp.dport = cpu_to_be16(ROCE_V2_UDP_DPORT);

	return 0;
}

static void ionic_set_ah_attr(struct ionic_ibdev *dev,
			      struct rdma_ah_attr *ah_attr,
			      struct ib_ud_header *hdr,
			      int sgid_index)
{
	u32 flow_label;
	u16 vlan = 0;
	u8  tos, ttl;

	if (hdr->vlan_present)
		vlan = be16_to_cpu(hdr->vlan.tag);

	if (hdr->ipv4_present) {
		flow_label = 0;
		ttl = hdr->ip4.ttl;
		tos = hdr->ip4.tos;
		*(__be16 *)(hdr->grh.destination_gid.raw + 10) = 0xffff;
		*(__be32 *)(hdr->grh.destination_gid.raw + 12) =
			hdr->ip4.daddr;
	} else {
		flow_label = be32_to_cpu(hdr->grh.flow_label);
		ttl = hdr->grh.hop_limit;
		tos = hdr->grh.traffic_class;
	}

	memset(ah_attr, 0, sizeof(*ah_attr));
#ifdef HAVE_RDMA_AH_ATTR_TYPE_ROCE
	ah_attr->type = RDMA_AH_ATTR_TYPE_ROCE;
#endif
	if (hdr->eth_present) {
#ifdef HAVE_RDMA_AH_ATTR_TYPE_ROCE
		memcpy(&ah_attr->roce.dmac, &hdr->eth.dmac_h, ETH_ALEN);
#else
		memcpy(&ah_attr->dmac, &hdr->eth.dmac_h, ETH_ALEN);
#endif
	}
	rdma_ah_set_sl(ah_attr, vlan >> 13);
	rdma_ah_set_port_num(ah_attr, 1);
	rdma_ah_set_grh(ah_attr, NULL, flow_label, sgid_index, ttl, tos);
	rdma_ah_set_dgid_raw(ah_attr, &hdr->grh.destination_gid);
}

static int ionic_create_ah_cmd(struct ionic_ibdev *dev,
			       struct ionic_ah *ah,
			       struct ionic_pd *pd,
			       struct rdma_ah_attr *attr,
			       u32 flags)
{
	struct ionic_admin_wr wr = {
		.work = COMPLETION_INITIALIZER_ONSTACK(wr.work),
		.wqe = {
			.op = IONIC_V1_ADMIN_CREATE_AH,
			.dbid_flags = cpu_to_le16(dev->dbid),
			.id_ver = cpu_to_le32(ah->ahid),
			.ah = {
				.pd_id = cpu_to_le32(pd->pdid),
			}
		}
	};
	enum ionic_admin_flags admin_flags = 0;
	dma_addr_t hdr_dma = 0;
	void *hdr_buf;
	gfp_t gfp = GFP_ATOMIC;
	int rc, hdr_len = 0;

	if (dev->admin_opcodes <= IONIC_V1_ADMIN_CREATE_AH)
		return -ENOSYS;

	if (flags & RDMA_CREATE_AH_SLEEPABLE)
		gfp = GFP_KERNEL;
	else
		admin_flags |= IONIC_ADMIN_F_BUSYWAIT;

	rc = ionic_build_hdr(dev, &ah->hdr, attr, false);
	if (rc)
		goto err_hdr;

	ah->sgid_index = rdma_ah_read_grh(attr)->sgid_index;

	hdr_buf = kmalloc(PAGE_SIZE, gfp);
	if (!hdr_buf) {
		rc = -ENOMEM;
		goto err_buf;
	}

	hdr_len = ib_ud_header_pack(&ah->hdr, hdr_buf);
	hdr_len -= IB_BTH_BYTES;
	hdr_len -= IB_DETH_BYTES;
	ibdev_dbg(&dev->ibdev, "roce packet header template\n");
	print_hex_dump_debug("hdr ", DUMP_PREFIX_OFFSET, 16, 1,
			     hdr_buf, hdr_len, true);

	hdr_dma = dma_map_single(dev->hwdev, hdr_buf, hdr_len,
				 DMA_TO_DEVICE);

	rc = dma_mapping_error(dev->hwdev, hdr_dma);
	if (rc)
		goto err_dma;

	wr.wqe.ah.dma_addr = cpu_to_le64(hdr_dma);
	wr.wqe.ah.length = cpu_to_le32(hdr_len);

	ionic_admin_post(dev, &wr);
	rc = ionic_admin_wait(dev, &wr, admin_flags);

	dma_unmap_single(dev->hwdev, hdr_dma, hdr_len,
			 DMA_TO_DEVICE);
err_dma:
	kfree(hdr_buf);
err_buf:
err_hdr:
	return rc;
}

static int ionic_destroy_ah_cmd(struct ionic_ibdev *dev, u32 ahid, u32 flags)
{
	struct ionic_admin_wr wr = {
		.work = COMPLETION_INITIALIZER_ONSTACK(wr.work),
		.wqe = {
			.op = IONIC_V1_ADMIN_DESTROY_AH,
			.id_ver = cpu_to_le32(ahid),
		}
	};
	enum ionic_admin_flags admin_flags = IONIC_ADMIN_F_TEARDOWN;

	if (dev->admin_opcodes <= IONIC_V1_ADMIN_DESTROY_AH)
		return -ENOSYS;

	if (!(flags & RDMA_CREATE_AH_SLEEPABLE))
		admin_flags |= IONIC_ADMIN_F_BUSYWAIT;

	ionic_admin_post(dev, &wr);
	ionic_admin_wait(dev, &wr, admin_flags);

	/* No host-memory resource is associated with ah, so it is ok
	 * to "succeed" and complete this destroy ah on the host.
	 */
	return 0;
}

#ifdef HAVE_CREATE_AH_UDATA
#ifdef HAVE_CREATE_AH_FLAGS
#ifdef HAVE_IB_ALLOC_AH_OBJ
static int ionic_create_ah(struct ib_ah *ibah,
			   struct rdma_ah_attr *attr,
			   u32 flags,
			   struct ib_udata *udata)
#else
static struct ib_ah *ionic_create_ah(struct ib_pd *ibpd,
				     struct rdma_ah_attr *attr,
				     u32 flags,
				     struct ib_udata *udata)
#endif /* HAVE_IB_ALLOC_AH_OBJ */
#else
static struct ib_ah *ionic_create_ah(struct ib_pd *ibpd,
				     struct rdma_ah_attr *attr,
				     struct ib_udata *udata)
#endif /* HAVE_CREATE_AH_FLAGS */
#else
static struct ib_ah *ionic_create_ah(struct ib_pd *ibpd,
				     struct rdma_ah_attr *attr)
#endif /* HAVE_CREATE_AH_UDATA */
{
#ifdef HAVE_IB_ALLOC_AH_OBJ
	struct ionic_ibdev *dev = to_ionic_ibdev(ibah->device);
	struct ionic_pd *pd = to_ionic_pd(ibah->pd);
	struct ionic_ah *ah = to_ionic_ah(ibah);
#else
	struct ionic_ibdev *dev = to_ionic_ibdev(ibpd->device);
	struct ionic_pd *pd = to_ionic_pd(ibpd);
	struct ionic_ah *ah;
#endif
#ifdef HAVE_CREATE_AH_UDATA
	struct ionic_ah_resp resp = {0};
#endif
	int rc;
#ifndef HAVE_CREATE_AH_FLAGS
	u32 flags = 0;
#endif

#ifdef HAVE_CREATE_AH_UDATA
#ifndef HAVE_CREATE_AH_UDATA_DMAC
	if (udata) {
		rc = ib_resolve_eth_dmac(&dev->ibdev, attr);
		if (rc)
			goto err_ah;
	}

#endif
#endif
#ifndef HAVE_IB_ALLOC_AH_OBJ
	ah = kzalloc(sizeof(*ah),
		     (flags & RDMA_CREATE_AH_SLEEPABLE) ?
		     GFP_KERNEL : GFP_ATOMIC);
	if (!ah) {
		rc = -ENOMEM;
		goto err_ah;
	}

#endif
	rc = ionic_get_ahid(dev, &ah->ahid);
	if (rc)
		goto err_ahid;

	rc = ionic_create_ah_cmd(dev, ah, pd, attr, flags);
	if (rc)
		goto err_cmd;

#ifdef HAVE_CREATE_AH_UDATA
	if (udata) {
		resp.ahid = ah->ahid;

		rc = ib_copy_to_udata(udata, &resp, sizeof(resp));
		if (rc)
			goto err_resp;
	}
#endif

#ifdef HAVE_IB_ALLOC_AH_OBJ
	return 0;
#else
	return &ah->ibah;
#endif

#ifdef HAVE_CREATE_AH_UDATA
err_resp:
	ionic_destroy_ah_cmd(dev, ah->ahid, flags);
#endif
err_cmd:
	ionic_put_ahid(dev, ah->ahid);
#ifdef HAVE_IB_ALLOC_AH_OBJ
err_ahid:
	return rc;
#else
err_ahid:
	kfree(ah);
err_ah:
	return ERR_PTR(rc);
#endif
}

static int ionic_query_ah(struct ib_ah *ibah,
			  struct rdma_ah_attr *ah_attr)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibah->device);
	struct ionic_ah *ah = to_ionic_ah(ibah);

	ionic_set_ah_attr(dev, ah_attr, &ah->hdr, ah->sgid_index);

	return 0;
}

#ifdef HAVE_CREATE_AH_FLAGS
#ifdef HAVE_IB_DESTROY_AH_VOID
static void ionic_destroy_ah(struct ib_ah *ibah, u32 flags)
#else
static int ionic_destroy_ah(struct ib_ah *ibah, u32 flags)
#endif
#else
static int ionic_destroy_ah(struct ib_ah *ibah)
#endif
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibah->device);
	struct ionic_ah *ah = to_ionic_ah(ibah);
	int rc;
#ifndef HAVE_CREATE_AH_FLAGS
	u32 flags = 0;
#endif

	rc = ionic_destroy_ah_cmd(dev, ah->ahid, flags);
	if (rc) {
		ibdev_warn(&dev->ibdev, "destroy_ah error %d\n", rc);
#ifdef HAVE_IB_DESTROY_AH_VOID
		return;
#else
		return rc;
#endif
	}

	ionic_put_ahid(dev, ah->ahid);
#ifndef HAVE_IB_ALLOC_AH_OBJ
	kfree(ah);
#endif

#ifndef HAVE_IB_DESTROY_AH_VOID
	return 0;
#endif
}

static int ionic_create_mr_cmd(struct ionic_ibdev *dev,
			       struct ionic_pd *pd,
			       struct ionic_mr *mr,
			       u64 addr,
			       u64 length)
{
	struct ionic_admin_wr wr = {
		.work = COMPLETION_INITIALIZER_ONSTACK(wr.work),
		.wqe = {
			.op = IONIC_V1_ADMIN_CREATE_MR,
			.dbid_flags = cpu_to_le16(mr->flags),
			.id_ver = cpu_to_le32(mr->mrid),
			.mr = {
				.va = cpu_to_le64(addr),
				.length = cpu_to_le64(length),
				.pd_id = cpu_to_le32(pd->pdid),
				.page_size_log2 = mr->buf.page_size_log2,
				.tbl_index = cpu_to_le32(mr->res.tbl_pos),
				.map_count = cpu_to_le32(mr->buf.tbl_pages),
				//.offset = ionic_pgtbl_off(&mr->buf, addr),
				.dma_addr = ionic_pgtbl_dma(&mr->buf, addr),
			}
		}
	};
	int rc;

	if (dev->admin_opcodes <= IONIC_V1_ADMIN_CREATE_MR)
		return -ENOSYS;

	ionic_admin_post(dev, &wr);
	rc = ionic_admin_wait(dev, &wr, 0);
	if (!rc)
		mr->created = true;

	return rc;
}

static int ionic_destroy_mr_cmd(struct ionic_ibdev *dev, u32 mrid)
{
	struct ionic_admin_wr wr = {
		.work = COMPLETION_INITIALIZER_ONSTACK(wr.work),
		.wqe = {
			.op = IONIC_V1_ADMIN_DESTROY_MR,
			.id_ver = cpu_to_le32(mrid),
		}
	};

	if (dev->admin_opcodes <= IONIC_V1_ADMIN_DESTROY_MR)
		return -ENOSYS;

	ionic_admin_post(dev, &wr);

	return ionic_admin_wait(dev, &wr, IONIC_ADMIN_F_TEARDOWN);
}

static struct ib_mr *ionic_get_dma_mr(struct ib_pd *ibpd, int access)
{
	struct ionic_mr *mr;

	mr = kzalloc(sizeof(*mr), GFP_KERNEL);
	if (!mr)
		return ERR_PTR(-ENOMEM);

	return &mr->ibmr;
}

#ifdef HAVE_IB_USER_MR_INIT_ATTR
static struct ib_mr *ionic_reg_user_mr(struct ib_pd *ibpd,
				       struct ib_mr_init_attr *attr,
				       struct ib_udata *udata)
#else
static struct ib_mr *ionic_reg_user_mr(struct ib_pd *ibpd, u64 start,
				       u64 length, u64 addr, int access,
				       struct ib_udata *udata)
#endif
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibpd->device);
	struct ionic_pd *pd = to_ionic_pd(ibpd);
	struct ionic_mr *mr;
#ifdef HAVE_IB_USER_MR_INIT_ATTR
	u64 start = attr->start;
	u64 length = attr->length;
	u64 addr = attr->hca_va;
	int access = attr->access_flags;
#endif
	int rc;

	mr = kzalloc(sizeof(*mr), GFP_KERNEL);
	if (!mr) {
		rc = -ENOMEM;
		goto err_mr;
	}

	rc = ionic_get_mrid(dev, &mr->mrid);
	if (rc)
		goto err_mrid;

	mr->ibmr.lkey = mr->mrid;
	mr->ibmr.rkey = mr->mrid;
	mr->ibmr.iova = addr;
	mr->ibmr.length = length;

	mr->flags = IONIC_MRF_USER_MR | to_ionic_mr_flags(access);

#ifdef HAVE_IB_UMEM_GET_UDATA
	mr->umem = ib_umem_get(udata, start, length, access, 0);
#else
	mr->umem = ib_umem_get(ibpd->uobject->context,
			       start, length, access, 0);
#endif
	if (IS_ERR(mr->umem)) {
		rc = PTR_ERR(mr->umem);
		goto err_umem;
	}

	rc = ionic_pgtbl_init(dev, &mr->res, &mr->buf, mr->umem, 0, 1);
	if (rc)
		goto err_pgtbl;

	rc = ionic_create_mr_cmd(dev, pd, mr, addr, length);
	if (rc)
		goto err_cmd;

	ionic_pgtbl_unbuf(dev, &mr->buf);

	ionic_dbg_add_mr(dev, mr);

	return &mr->ibmr;

err_cmd:
	ionic_pgtbl_unbuf(dev, &mr->buf);
	ionic_put_res(dev, &mr->res);
err_pgtbl:
	ib_umem_release(mr->umem);
err_umem:
	ionic_put_mrid(dev, mr->mrid);
err_mrid:
	kfree(mr);
err_mr:
	return ERR_PTR(rc);
}

static int ionic_rereg_user_mr(struct ib_mr *ibmr, int flags, u64 start,
			       u64 length, u64 addr, int access,
			       struct ib_pd *ibpd, struct ib_udata *udata)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibmr->device);
	struct ionic_mr *mr = to_ionic_mr(ibmr);
	struct ionic_pd *pd;
	int rc;

	if (!mr->ibmr.lkey)
		return -EINVAL;

	if (!mr->created) {
		/* must set translation if not already on device */
		if (~flags & IB_MR_REREG_TRANS)
			return -EINVAL;
	} else {
		/* destroy on device first if not already on device */
		rc = ionic_destroy_mr_cmd(dev, mr->mrid);
		if (rc)
			return rc;

		mr->created = false;
	}

	if (~flags & IB_MR_REREG_PD)
		ibpd = mr->ibmr.pd;
	pd = to_ionic_pd(ibpd);

	mr->mrid = ib_inc_rkey(mr->mrid);
	mr->ibmr.lkey = mr->mrid;
	mr->ibmr.rkey = mr->mrid;

	if (flags & IB_MR_REREG_ACCESS)
		mr->flags = IONIC_MRF_USER_MR | to_ionic_mr_flags(access);

	if (flags & IB_MR_REREG_TRANS) {
		ionic_pgtbl_unbuf(dev, &mr->buf);
		ionic_put_res(dev, &mr->res);

		if (mr->umem)
			ib_umem_release(mr->umem);

		mr->ibmr.iova = addr;
		mr->ibmr.length = length;

#ifdef HAVE_IB_UMEM_GET_UDATA
		mr->umem = ib_umem_get(udata, start, length, access, 0);
#else
		mr->umem = ib_umem_get(ibpd->uobject->context, start,
				       length, access, 0);
#endif
		if (IS_ERR(mr->umem)) {
			rc = PTR_ERR(mr->umem);
			goto err_umem;
		}

		rc = ionic_pgtbl_init(dev, &mr->res, &mr->buf, mr->umem, 0, 1);
		if (rc)
			goto err_pgtbl;
	}

	rc = ionic_create_mr_cmd(dev, pd, mr, addr, length);
	if (rc)
		goto err_cmd;

	ionic_pgtbl_unbuf(dev, &mr->buf);

	return 0;

err_cmd:
	ionic_pgtbl_unbuf(dev, &mr->buf);
	ionic_put_res(dev, &mr->res);
err_pgtbl:
	ib_umem_release(mr->umem);
	mr->umem = NULL;
err_umem:
	mr->res.tbl_order = IONIC_RES_INVALID;
	return rc;
}

#ifdef HAVE_IB_API_UDATA
static int ionic_dereg_mr(struct ib_mr *ibmr, struct ib_udata *udata)
#else
static int ionic_dereg_mr(struct ib_mr *ibmr)
#endif
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibmr->device);
	struct ionic_mr *mr = to_ionic_mr(ibmr);
	int rc;

	if (!mr->ibmr.lkey)
		goto out;

	if (mr->created) {
		rc = ionic_destroy_mr_cmd(dev, mr->mrid);
		if (rc)
			return rc;
	}

	ionic_dbg_rm_mr(mr);

	ionic_pgtbl_unbuf(dev, &mr->buf);
	ionic_put_res(dev, &mr->res);

	if (mr->umem)
		ib_umem_release(mr->umem);

	ionic_put_mrid(dev, mr->mrid);

out:
	kfree(mr);

	return 0;
}

#ifdef HAVE_IB_API_UDATA
static struct ib_mr *ionic_alloc_mr(struct ib_pd *ibpd,
				    enum ib_mr_type type,
				    u32 max_sg,
				    struct ib_udata *udata)
#else
static struct ib_mr *ionic_alloc_mr(struct ib_pd *ibpd,
				    enum ib_mr_type type,
				    u32 max_sg)
#endif
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibpd->device);
	struct ionic_pd *pd = to_ionic_pd(ibpd);
	struct ionic_mr *mr;
	int rc;

	if (type != IB_MR_TYPE_MEM_REG) {
		rc = -EINVAL;
		goto err_mr;
	}

	mr = kzalloc(sizeof(*mr), GFP_KERNEL);
	if (!mr) {
		rc = -ENOMEM;
		goto err_mr;
	}

	rc = ionic_get_mrid(dev, &mr->mrid);
	if (rc)
		goto err_mrid;

	mr->ibmr.lkey = mr->mrid;
	mr->ibmr.rkey = mr->mrid;

	mr->flags = IONIC_MRF_PHYS_MR;

	rc = ionic_pgtbl_init(dev, &mr->res, &mr->buf, mr->umem, 0, max_sg);
	if (rc)
		goto err_pgtbl;

	mr->buf.tbl_pages = 0;

	rc = ionic_create_mr_cmd(dev, pd, mr, 0, 0);
	if (rc)
		goto err_cmd;

	ionic_dbg_add_mr(dev, mr);

	return &mr->ibmr;

err_cmd:
	ionic_pgtbl_unbuf(dev, &mr->buf);
	ionic_put_res(dev, &mr->res);
err_pgtbl:
	ionic_put_mrid(dev, mr->mrid);
err_mrid:
	kfree(mr);
err_mr:
	return ERR_PTR(rc);
}

static int ionic_map_mr_page(struct ib_mr *ibmr, u64 dma)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibmr->device);
	struct ionic_mr *mr = to_ionic_mr(ibmr);

	ibdev_dbg(&dev->ibdev, "dma %p\n", (void *)dma);
	return ionic_pgtbl_page(&mr->buf, dma);
}

static int ionic_map_mr_sg(struct ib_mr *ibmr, struct scatterlist *sg,
			   int sg_nents, unsigned int *sg_offset)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibmr->device);
	struct ionic_mr *mr = to_ionic_mr(ibmr);
	int rc;

	/* mr must be allocated using ib_alloc_mr() */
	if (unlikely(!mr->buf.tbl_limit))
		return -EINVAL;

	mr->buf.tbl_pages = 0;

	if (mr->buf.tbl_buf)
		dma_sync_single_for_cpu(dev->hwdev, mr->buf.tbl_dma,
					mr->buf.tbl_size, DMA_TO_DEVICE);

	ibdev_dbg(&dev->ibdev, "sg %p nent %d\n", sg, sg_nents);
	rc = ib_sg_to_pages(ibmr, sg, sg_nents, sg_offset, ionic_map_mr_page);

	mr->buf.page_size_log2 = order_base_2(ibmr->page_size);

	if (mr->buf.tbl_buf)
		dma_sync_single_for_device(dev->hwdev, mr->buf.tbl_dma,
					   mr->buf.tbl_size, DMA_TO_DEVICE);

	return rc;
}

static struct ib_mw *ionic_alloc_mw(struct ib_pd *ibpd, enum ib_mw_type type,
				    struct ib_udata *udata)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibpd->device);
	struct ionic_pd *pd = to_ionic_pd(ibpd);
	struct ionic_mr *mr;
	int rc;

	mr = kzalloc(sizeof(*mr), GFP_KERNEL);
	if (!mr) {
		rc = -ENOMEM;
		goto err_mr;
	}

	rc = ionic_get_mrid(dev, &mr->mrid);
	if (rc)
		goto err_mrid;

	mr->ibmw.rkey = mr->mrid;
	mr->ibmw.type = type;

	if (type == IB_MW_TYPE_1)
		mr->flags = IONIC_MRF_MW_1;
	else
		mr->flags = IONIC_MRF_MW_2;

	rc = ionic_create_mr_cmd(dev, pd, mr, 0, 0);
	if (rc)
		goto err_cmd;

	ionic_dbg_add_mr(dev, mr);

	return &mr->ibmw;

err_cmd:
	ionic_put_mrid(dev, mr->mrid);
err_mrid:
	kfree(mr);
err_mr:
	return ERR_PTR(rc);
}

static int ionic_dealloc_mw(struct ib_mw *ibmw)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibmw->device);
	struct ionic_mr *mr = to_ionic_mw(ibmw);
	int rc;

	rc = ionic_destroy_mr_cmd(dev, mr->mrid);
	if (rc)
		return rc;

	ionic_dbg_rm_mr(mr);

	ionic_put_mrid(dev, mr->mrid);

	kfree(mr);

	return 0;
}

static int ionic_create_cq_cmd(struct ionic_ibdev *dev,
			       struct ionic_ctx *ctx,
			       struct ionic_cq *cq,
			       struct ionic_tbl_buf *buf)
{
	const u16 dbid = ionic_ctx_dbid(dev, ctx);
	struct ionic_admin_wr wr = {
		.work = COMPLETION_INITIALIZER_ONSTACK(wr.work),
		.wqe = {
			.op = IONIC_V1_ADMIN_CREATE_CQ,
			.dbid_flags = cpu_to_le16(dbid),
			.id_ver = cpu_to_le32(cq->cqid),
			.cq = {
				.eq_id = cpu_to_le32(cq->eqid),
				.depth_log2 = cq->q.depth_log2,
				.stride_log2 = cq->q.stride_log2,
				.page_size_log2 = buf->page_size_log2,
				.tbl_index = cpu_to_le32(cq->res.tbl_pos),
				.map_count = cpu_to_le32(buf->tbl_pages),
				.dma_addr = ionic_pgtbl_dma(buf, 0),
			}
		}
	};

	if (dev->admin_opcodes <= IONIC_V1_ADMIN_CREATE_CQ)
		return -ENOSYS;

	ionic_admin_post(dev, &wr);

	return ionic_admin_wait(dev, &wr, 0);
}

static int ionic_destroy_cq_cmd(struct ionic_ibdev *dev, u32 cqid)
{
	struct ionic_admin_wr wr = {
		.work = COMPLETION_INITIALIZER_ONSTACK(wr.work),
		.wqe = {
			.op = IONIC_V1_ADMIN_DESTROY_CQ,
			.id_ver = cpu_to_le32(cqid),
		}
	};

	if (dev->admin_opcodes <= IONIC_V1_ADMIN_DESTROY_CQ)
		return -ENOSYS;

	ionic_admin_post(dev, &wr);

	return ionic_admin_wait(dev, &wr, IONIC_ADMIN_F_TEARDOWN);
}

int ionic_create_cq_common(struct ionic_cq *cq,
			   struct ionic_tbl_buf *buf,
			   const struct ib_cq_init_attr *attr,
			   struct ionic_ctx *ctx,
			   struct ib_udata *udata)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(cq->ibcq.device);
	struct ionic_cq_req req;
	struct ionic_cq_resp resp;
	int rc, eq_idx;
	unsigned long irqflags;

	if (attr->cqe < 1 || attr->cqe + IONIC_CQ_GRACE > 0xffff) {
		rc = -EINVAL;
		goto err_args;
	}

	rc = ionic_get_cqid(dev, &cq->cqid);
	if (rc)
		goto err_cqid;

	/* the first eq is reserved for async events */
	eq_idx = attr->comp_vector;
	if (eq_idx >= dev->eq_count - 1)
		eq_idx %= dev->eq_count - 1;
	eq_idx += 1;

	cq->eqid = dev->eq_vec[eq_idx]->eqid;

	spin_lock_init(&cq->lock);
	INIT_LIST_HEAD(&cq->poll_sq);
	INIT_LIST_HEAD(&cq->flush_sq);
	INIT_LIST_HEAD(&cq->flush_rq);

	if (udata) {
		rc = ib_copy_from_udata(&req, udata, sizeof(req));
		if (rc)
			goto err_req;

		rc = ionic_validate_qdesc(&req.cq);
		if (rc)
			goto err_qdesc;

#ifdef HAVE_IB_UMEM_GET_UDATA
		cq->umem = ib_umem_get(udata, req.cq.addr, req.cq.size,
				       IB_ACCESS_LOCAL_WRITE, 0);
#else
		cq->umem = ib_umem_get(&ctx->ibctx, req.cq.addr, req.cq.size,
				       IB_ACCESS_LOCAL_WRITE, 0);
#endif
		if (IS_ERR(cq->umem)) {
			rc = PTR_ERR(cq->umem);
			goto err_umem;
		}

		cq->q.ptr = NULL;
		cq->q.size = req.cq.size;
		cq->q.mask = req.cq.mask;
		cq->q.depth_log2 = req.cq.depth_log2;
		cq->q.stride_log2 = req.cq.stride_log2;

		resp.cqid = cq->cqid;

		rc = ib_copy_to_udata(udata, &resp, sizeof(resp));
		if (rc)
			goto err_resp;
	} else {
		rc = ionic_queue_init(&cq->q, dev->hwdev,
				      attr->cqe + IONIC_CQ_GRACE,
				      sizeof(struct ionic_v1_cqe));
		if (rc)
			goto err_q_init;

		ionic_queue_dbell_init(&cq->q, cq->cqid);
		cq->color = true;
		cq->reserve = cq->q.mask;
	}

	rc = ionic_pgtbl_init(dev, &cq->res, buf, cq->umem, cq->q.dma, 1);
	if (rc)
		goto err_pgtbl_init;

	init_completion(&cq->cq_rel_comp);
	kref_init(&cq->cq_kref);

	rc = xa_err(xa_store_irq(&dev->cq_tbl, cq->cqid, cq, GFP_KERNEL));
	if (rc)
		goto err_xa;

	spin_lock_irqsave(&dev->dev_lock, irqflags);
	list_add_tail(&cq->cq_list_ent, &dev->cq_list);
	spin_unlock_irqrestore(&dev->dev_lock, irqflags);

	ionic_dbg_add_cq(dev, cq);

	return 0;

err_xa:
	ionic_pgtbl_unbuf(dev, buf);
	ionic_put_res(dev, &cq->res);
err_pgtbl_init:
	if (!udata)
		ionic_queue_destroy(&cq->q, dev->hwdev);
err_q_init:
err_resp:
	if (cq->umem)
		ib_umem_release(cq->umem);
err_umem:
err_qdesc:
err_req:
	ionic_put_cqid(dev, cq->cqid);
err_cqid:
err_args:
	return rc;
}

void ionic_destroy_cq_common(struct ionic_ibdev *dev, struct ionic_cq *cq)
{
	unsigned long irqflags;

	spin_lock_irqsave(&dev->dev_lock, irqflags);
	list_del(&cq->cq_list_ent);
	spin_unlock_irqrestore(&dev->dev_lock, irqflags);

	xa_erase_irq(&dev->cq_tbl, cq->cqid);

	kref_put(&cq->cq_kref, ionic_cq_complete);
	wait_for_completion(&cq->cq_rel_comp);

	ionic_dbg_rm_cq(cq);

	ionic_put_res(dev, &cq->res);

	if (cq->umem)
		ib_umem_release(cq->umem);
	else
		ionic_queue_destroy(&cq->q, dev->hwdev);

	ionic_put_cqid(dev, cq->cqid);
}

#ifdef HAVE_IB_API_UDATA
#ifdef HAVE_IB_ALLOC_CQ_OBJ
static int ionic_create_cq(struct ib_cq *ibcq,
			   const struct ib_cq_init_attr *attr,
			   struct ib_udata *udata)
#else
static struct ib_cq *ionic_create_cq(struct ib_device *ibdev,
				     const struct ib_cq_init_attr *attr,
				     struct ib_udata *udata)
#endif
#else
static struct ib_cq *ionic_create_cq(struct ib_device *ibdev,
				     const struct ib_cq_init_attr *attr,
				     struct ib_ucontext *ibctx,
				     struct ib_udata *udata)
#endif
{
#ifdef HAVE_IB_ALLOC_CQ_OBJ
	struct ionic_ibdev *dev = to_ionic_ibdev(ibcq->device);
	struct ionic_cq *cq = to_ionic_cq(ibcq);
#else
	struct ionic_ibdev *dev = to_ionic_ibdev(ibdev);
	struct ionic_cq *cq;
#endif
#ifdef HAVE_IB_API_UDATA
	struct ionic_ctx *ctx =
		rdma_udata_to_drv_context(udata, struct ionic_ctx, ibctx);
#else
	struct ionic_ctx *ctx = to_ionic_ctx(ibctx);
#endif
	struct ionic_tbl_buf buf = {0};
	int rc;

#ifndef HAVE_IB_ALLOC_CQ_OBJ
	cq = kzalloc(sizeof(*cq), GFP_KERNEL);
	if (!cq) {
		rc = -ENOMEM;
		goto err_alloc;
	}
	cq->ibcq.device = ibdev;

#endif
	rc = ionic_create_cq_common(cq, &buf, attr, ctx, udata);
	if (rc)
		goto err_init;

	rc = ionic_create_cq_cmd(dev, ctx, cq, &buf);
	if (rc)
		goto err_cmd;

	ionic_pgtbl_unbuf(dev, &buf);

#ifdef HAVE_IB_ALLOC_CQ_OBJ
	return 0;
#else
	return &cq->ibcq;
#endif

err_cmd:
	ionic_pgtbl_unbuf(dev, &buf);
	ionic_destroy_cq_common(dev, cq);
err_init:
#ifdef HAVE_IB_ALLOC_CQ_OBJ
	return rc;
#else
	kfree(cq);
err_alloc:
	return ERR_PTR(rc);
#endif
}

#ifdef HAVE_IB_API_UDATA
#ifdef HAVE_IB_DESTROY_CQ_VOID
static void ionic_destroy_cq(struct ib_cq *ibcq, struct ib_udata *udata)
#else
static int ionic_destroy_cq(struct ib_cq *ibcq, struct ib_udata *udata)
#endif
#else
static int ionic_destroy_cq(struct ib_cq *ibcq)
#endif
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibcq->device);
	struct ionic_cq *cq = to_ionic_cq(ibcq);
	int rc;

	rc = ionic_destroy_cq_cmd(dev, cq->cqid);
	if (rc) {
		ibdev_warn(&dev->ibdev, "destroy_cq error %d\n", rc);
#ifdef HAVE_IB_DESTROY_CQ_VOID
		return;
#else
		return rc;
#endif
	}

	ionic_destroy_cq_common(dev, cq);
#ifndef HAVE_IB_ALLOC_CQ_OBJ
	kfree(cq);
#endif

#ifndef HAVE_IB_DESTROY_CQ_VOID
	return 0;
#endif
}

static bool pd_local_privileged(struct ib_pd *pd)
{
	return !pd->uobject;
}

static bool pd_remote_privileged(struct ib_pd *pd)
{
#ifdef HAVE_IB_PD_FLAGS
	return pd->flags & IB_PD_UNSAFE_GLOBAL_RKEY;
#else
	return false;
#endif
}

static int ionic_create_qp_cmd(struct ionic_ibdev *dev,
			       struct ionic_pd *pd,
			       struct ionic_cq *send_cq,
			       struct ionic_cq *recv_cq,
			       struct ionic_qp *qp,
			       struct ionic_tbl_buf *sq_buf,
			       struct ionic_tbl_buf *rq_buf,
			       struct ib_qp_init_attr *attr)
{
	const u16 dbid = ionic_obj_dbid(dev, pd->ibpd.uobject);
	const u32 flags = to_ionic_qp_flags(0, 0, qp->sq_is_cmb, qp->rq_is_cmb,
					    qp->sq_spec, qp->rq_spec,
					    pd_local_privileged(&pd->ibpd),
					    pd_remote_privileged(&pd->ibpd));
	struct ionic_admin_wr wr = {
		.work = COMPLETION_INITIALIZER_ONSTACK(wr.work),
		.wqe = {
			.op = IONIC_V1_ADMIN_CREATE_QP,
			.type_state = to_ionic_qp_type(attr->qp_type),
			.dbid_flags = cpu_to_le16(dbid),
			.id_ver = cpu_to_le32(qp->qpid),
			.qp = {
				.pd_id = cpu_to_le32(pd->pdid),
				.priv_flags = cpu_to_be32(flags),
			}
		}
	};

	if (dev->admin_opcodes <= IONIC_V1_ADMIN_CREATE_QP)
		return -ENOSYS;

	if (qp->has_sq) {
		wr.wqe.qp.sq_cq_id = cpu_to_le32(send_cq->cqid);
		wr.wqe.qp.sq_depth_log2 = qp->sq.depth_log2;
		wr.wqe.qp.sq_stride_log2 = qp->sq.stride_log2;
		wr.wqe.qp.sq_page_size_log2 = sq_buf->page_size_log2;
		wr.wqe.qp.sq_tbl_index_xrcd_id =
			cpu_to_le32(qp->sq_res.tbl_pos);
		wr.wqe.qp.sq_map_count = cpu_to_le32(sq_buf->tbl_pages);
		wr.wqe.qp.sq_dma_addr = ionic_pgtbl_dma(sq_buf, 0);
	}

	if (qp->has_rq) {
		wr.wqe.qp.rq_cq_id = cpu_to_le32(recv_cq->cqid);
		wr.wqe.qp.rq_depth_log2 = qp->rq.depth_log2;
		wr.wqe.qp.rq_stride_log2 = qp->rq.stride_log2;
		wr.wqe.qp.rq_page_size_log2 = rq_buf->page_size_log2;
		wr.wqe.qp.rq_tbl_index_srq_id =
			cpu_to_le32(qp->rq_res.tbl_pos);
		wr.wqe.qp.rq_map_count = cpu_to_le32(rq_buf->tbl_pages);
		wr.wqe.qp.rq_dma_addr = ionic_pgtbl_dma(rq_buf, 0);
	}

	ionic_admin_post(dev, &wr);

	return ionic_admin_wait(dev, &wr, 0);
}

static int ionic_modify_qp_cmd(struct ionic_ibdev *dev,
			       struct ionic_qp *qp,
			       struct ib_qp_attr *attr,
			       int mask)
{
	const u32 flags = to_ionic_qp_flags(attr->qp_access_flags,
					    attr->en_sqd_async_notify,
					    qp->sq_is_cmb, qp->rq_is_cmb,
					    qp->sq_spec, qp->rq_spec,
					    pd_local_privileged(qp->ibqp.pd),
					    pd_remote_privileged(qp->ibqp.pd));
	const u8 state = to_ionic_qp_modify_state(attr->qp_state,
						  attr->cur_qp_state);
	struct ionic_admin_wr wr = {
		.work = COMPLETION_INITIALIZER_ONSTACK(wr.work),
		.wqe = {
			.op = IONIC_V1_ADMIN_MODIFY_QP,
			.type_state = state,
			.id_ver = cpu_to_le32(qp->qpid),
			.mod_qp = {
				.attr_mask = cpu_to_be32(mask),
				.access_flags = cpu_to_be16(flags),
				.rq_psn = cpu_to_le32(attr->rq_psn),
				.sq_psn = cpu_to_le32(attr->sq_psn),
#ifdef HAVE_QP_RATE_LIMIT
				.rate_limit_kbps =
					cpu_to_le32(attr->rate_limit),
#endif
				.pmtu = (attr->path_mtu + 7),
				.retry = (attr->retry_cnt |
					  (attr->rnr_retry << 4)),
				.rnr_timer = attr->min_rnr_timer,
				.retry_timeout = attr->timeout,
			}
		}
	};
	void *hdr_buf = NULL;
	dma_addr_t hdr_dma = 0;
	int rc, hdr_len = 0;

	if (dev->admin_opcodes <= IONIC_V1_ADMIN_MODIFY_QP)
		return -ENOSYS;

	if ((mask & IB_QP_MAX_DEST_RD_ATOMIC) && attr->max_dest_rd_atomic) {
		/* Note, round up/down was already done for allocating
		 * resources on the device. The allocation order is in cache
		 * line size.  We can't use the order of the resource
		 * allocation to determine the order wqes here, because for
		 * queue length <= one cache line it is not distinct.
		 *
		 * Therefore, order wqes is computed again here.
		 *
		 * Account for hole and round up to the next order.
		 */
		wr.wqe.mod_qp.rsq_depth =
			order_base_2(attr->max_dest_rd_atomic + 1);
		wr.wqe.mod_qp.rsq_index = cpu_to_le32(qp->rsq_res.tbl_pos);
	}

	if ((mask & IB_QP_MAX_QP_RD_ATOMIC) && attr->max_rd_atomic) {
		/* Account for hole and round down to the next order */
		wr.wqe.mod_qp.rrq_depth =
			order_base_2(attr->max_rd_atomic + 2) - 1;
		wr.wqe.mod_qp.rrq_index = cpu_to_le32(qp->rrq_res.tbl_pos);
	}

	if (qp->ibqp.qp_type == IB_QPT_RC || qp->ibqp.qp_type == IB_QPT_UC)
		wr.wqe.mod_qp.qkey_dest_qpn = cpu_to_le32(attr->dest_qp_num);
	else
		wr.wqe.mod_qp.qkey_dest_qpn = cpu_to_le32(attr->qkey);

	if (mask & IB_QP_AV) {
		if (!qp->hdr) {
			rc = -ENOMEM;
			goto err_hdr;
		}

		rc = ionic_build_hdr(dev, qp->hdr, &attr->ah_attr, true);
		if (rc)
			goto err_hdr;

		qp->sgid_index = rdma_ah_read_grh(&attr->ah_attr)->sgid_index;

		hdr_buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
		if (!hdr_buf) {
			rc = -ENOMEM;
			goto err_buf;
		}

		hdr_len = ib_ud_header_pack(qp->hdr, hdr_buf);
		hdr_len -= IB_BTH_BYTES;
		hdr_len -= IB_DETH_BYTES;
		ibdev_dbg(&dev->ibdev, "roce packet header template\n");
		print_hex_dump_debug("hdr ", DUMP_PREFIX_OFFSET, 16, 1,
				     hdr_buf, hdr_len, true);

		hdr_dma = dma_map_single(dev->hwdev, hdr_buf, hdr_len,
					 DMA_TO_DEVICE);

		rc = dma_mapping_error(dev->hwdev, hdr_dma);
		if (rc)
			goto err_dma;

		wr.wqe.mod_qp.ah_id_len =
			cpu_to_le32(qp->ahid | (hdr_len << 24));
		wr.wqe.mod_qp.dma_addr = cpu_to_le64(hdr_dma);

		wr.wqe.mod_qp.dcqcn_profile = qp->dcqcn_profile;
		wr.wqe.mod_qp.en_pcp = attr->ah_attr.sl;
		wr.wqe.mod_qp.ip_dscp =
			rdma_ah_read_grh(&attr->ah_attr)->traffic_class >> 2;
	}

	ionic_admin_post(dev, &wr);

	rc = ionic_admin_wait(dev, &wr, 0);

	if (mask & IB_QP_AV)
		dma_unmap_single(dev->hwdev, hdr_dma, hdr_len,
				 DMA_TO_DEVICE);
err_dma:
	if (mask & IB_QP_AV)
		kfree(hdr_buf);
err_buf:
err_hdr:
	return rc;
}

static int ionic_query_qp_cmd(struct ionic_ibdev *dev,
			      struct ionic_qp *qp,
			      struct ib_qp_attr *attr,
			      int mask)
{
	struct ionic_admin_wr wr = {
		.work = COMPLETION_INITIALIZER_ONSTACK(wr.work),
		.wqe = {
			.op = IONIC_V1_ADMIN_QUERY_QP,
			.id_ver = cpu_to_le32(qp->qpid),
		}
	};
	struct ionic_v1_admin_query_qp_sq *query_sqbuf;
	struct ionic_v1_admin_query_qp_rq *query_rqbuf;
	dma_addr_t query_sqdma;
	dma_addr_t query_rqdma;
	dma_addr_t hdr_dma = 0;
	void *hdr_buf = NULL;
	int flags, rc;

	if (dev->admin_opcodes <= IONIC_V1_ADMIN_QUERY_QP)
		return -ENOSYS;

	if (qp->has_sq) {
		attr->cap.max_send_sge =
			ionic_v1_send_wqe_max_sge(qp->sq.stride_log2,
						  qp->sq_spec);
		attr->cap.max_inline_data =
			ionic_v1_send_wqe_max_data(qp->sq.stride_log2);
	}

	if (qp->has_rq) {
		attr->cap.max_recv_sge =
			ionic_v1_recv_wqe_max_sge(qp->rq.stride_log2,
						  qp->rq_spec);
	}

	query_sqbuf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!query_sqbuf) {
		rc = -ENOMEM;
		goto err_sqbuf;
	}
	query_rqbuf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!query_rqbuf) {
		rc = -ENOMEM;
		goto err_rqbuf;
	}

	query_sqdma = dma_map_single(dev->hwdev, query_sqbuf, PAGE_SIZE,
				     DMA_FROM_DEVICE);
	rc = dma_mapping_error(dev->hwdev, query_sqdma);
	if (rc)
		goto err_sqdma;

	query_rqdma = dma_map_single(dev->hwdev, query_rqbuf, PAGE_SIZE,
				     DMA_FROM_DEVICE);
	rc = dma_mapping_error(dev->hwdev, query_rqdma);
	if (rc)
		goto err_rqdma;

	if (mask & IB_QP_AV) {
		hdr_buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
		if (!hdr_buf) {
			rc = -ENOMEM;
			goto err_hdrbuf;
		}

		hdr_dma = dma_map_single(dev->hwdev, hdr_buf,
					 PAGE_SIZE, DMA_FROM_DEVICE);
		rc = dma_mapping_error(dev->hwdev, hdr_dma);
		if (rc)
			goto err_hdrdma;
	}

	wr.wqe.query_qp.sq_dma_addr = cpu_to_le64(query_sqdma);
	wr.wqe.query_qp.rq_dma_addr = cpu_to_le64(query_rqdma);
	wr.wqe.query_qp.hdr_dma_addr = cpu_to_le64(hdr_dma);
	wr.wqe.query_qp.ah_id = cpu_to_le32(qp->ahid);

	ionic_admin_post(dev, &wr);

	rc = ionic_admin_wait(dev, &wr, 0);

	dma_unmap_single(dev->hwdev, query_sqdma, sizeof(*query_sqbuf),
			 DMA_FROM_DEVICE);
	dma_unmap_single(dev->hwdev, query_rqdma, sizeof(*query_rqbuf),
			 DMA_FROM_DEVICE);

	if (mask & IB_QP_AV)
		dma_unmap_single(dev->hwdev, hdr_dma,
				 PAGE_SIZE, DMA_FROM_DEVICE);

	if (rc)
		goto err_sqdma;

	flags = be16_to_cpu(query_sqbuf->access_perms_flags);

	attr->qp_state = from_ionic_qp_state(query_rqbuf->state_pmtu & 0xf);
	attr->cur_qp_state = attr->qp_state;
	attr->path_mtu = (query_rqbuf->state_pmtu >> 4) - 7;
	attr->path_mig_state = IB_MIG_MIGRATED;
	attr->qkey = be32_to_cpu(query_sqbuf->qkey_dest_qpn);
	attr->rq_psn = be32_to_cpu(query_sqbuf->rq_psn);
	attr->sq_psn = be32_to_cpu(query_rqbuf->sq_psn);
	attr->dest_qp_num = attr->qkey;
	attr->qp_access_flags = from_ionic_qp_flags(flags);
	attr->pkey_index = 0;
	attr->alt_pkey_index = 0;
	attr->en_sqd_async_notify = !!(flags & IONIC_QPF_SQD_NOTIFY);
	attr->sq_draining = !!(flags & IONIC_QPF_SQ_DRAINING);
	attr->max_rd_atomic = BIT(query_rqbuf->rrq_depth) - 1;
	attr->max_dest_rd_atomic = BIT(query_rqbuf->rsq_depth) - 1;
	attr->min_rnr_timer = query_sqbuf->rnr_timer;
	attr->port_num = 0;
	attr->timeout = query_sqbuf->retry_timeout;
	attr->retry_cnt = query_rqbuf->retry_rnrtry & 0xf;
	attr->rnr_retry = query_rqbuf->retry_rnrtry >> 4;
	attr->alt_port_num = 0;
	attr->alt_timeout = 0;
#ifdef HAVE_QP_RATE_LIMIT
	attr->rate_limit = be32_to_cpu(query_sqbuf->rate_limit_kbps);
#endif

	if (mask & IB_QP_AV)
		ionic_set_ah_attr(dev, &attr->ah_attr,
				  qp->hdr, qp->sgid_index);

err_hdrdma:
	kfree(hdr_buf);
err_hdrbuf:
	dma_unmap_single(dev->hwdev, query_rqdma, sizeof(*query_rqbuf),
			 DMA_FROM_DEVICE);
err_rqdma:
	dma_unmap_single(dev->hwdev, query_sqdma, sizeof(*query_sqbuf),
			 DMA_FROM_DEVICE);
err_sqdma:
	kfree(query_rqbuf);
err_rqbuf:
	kfree(query_sqbuf);
err_sqbuf:
	return rc;
}

static int ionic_destroy_qp_cmd(struct ionic_ibdev *dev, u32 qpid)
{
	struct ionic_admin_wr wr = {
		.work = COMPLETION_INITIALIZER_ONSTACK(wr.work),
		.wqe = {
			.op = IONIC_V1_ADMIN_DESTROY_QP,
			.id_ver = cpu_to_le32(qpid),
		}
	};

	if (dev->admin_opcodes <= IONIC_V1_ADMIN_DESTROY_QP)
		return -ENOSYS;

	ionic_admin_post(dev, &wr);

	return ionic_admin_wait(dev, &wr, IONIC_ADMIN_F_TEARDOWN);
}

static void ionic_qp_sq_init_cmb(struct ionic_ibdev *dev,
				 struct ionic_qp *qp,
				 struct ib_udata *udata,
				 bool cap_inline)
{
	int rc;

	if (!qp->has_sq)
		goto not_in_cmb;

	if (ionic_sqcmb_inline && !cap_inline)
		goto not_in_cmb;

	qp->sq_cmb_order = order_base_2(qp->sq.size / PAGE_SIZE);

	if (qp->sq_cmb_order >= ionic_sqcmb_order)
		goto not_in_cmb;

	rc = ionic_api_get_cmb(dev->handle, &qp->sq_cmb_pgid,
			       &qp->sq_cmb_addr, qp->sq_cmb_order);
	if (rc)
		goto not_in_cmb;

	qp->sq_cmb_ptr = ioremap_wc(qp->sq_cmb_addr, qp->sq.size);
	if (!qp->sq_cmb_ptr)
		goto err_map;

	memset_io(qp->sq_cmb_ptr, 0, qp->sq.size);

	if (udata) {
		iounmap(qp->sq_cmb_ptr);
		qp->sq_cmb_ptr = NULL;
	}

	qp->sq_is_cmb = true;

	return;

err_map:
	ionic_api_put_cmb(dev->handle, qp->sq_cmb_pgid, qp->sq_cmb_order);
not_in_cmb:
	qp->sq_is_cmb = false;
	qp->sq_cmb_ptr = NULL;
	qp->sq_cmb_order = IONIC_RES_INVALID;
	qp->sq_cmb_pgid = 0;
	qp->sq_cmb_addr = 0;

	qp->sq_cmb_mmap.offset = 0;
	qp->sq_cmb_mmap.size = 0;
	qp->sq_cmb_mmap.pfn = 0;
}

static void ionic_qp_sq_destroy_cmb(struct ionic_ibdev *dev,
				    struct ionic_ctx *ctx,
				    struct ionic_qp *qp)
{
	if (!qp->sq_is_cmb)
		return;

	if (ctx) {
		mutex_lock(&ctx->mmap_mut);
		list_del(&qp->sq_cmb_mmap.ctx_ent);
		mutex_unlock(&ctx->mmap_mut);
	} else {
		iounmap(qp->sq_cmb_ptr);
	}

	ionic_api_put_cmb(dev->handle, qp->sq_cmb_pgid, qp->sq_cmb_order);
}

static int ionic_qp_sq_init(struct ionic_ibdev *dev, struct ionic_ctx *ctx,
			    struct ionic_qp *qp, struct ionic_qdesc *sq,
			    struct ionic_tbl_buf *buf, int max_wr, int max_sge,
			    int max_data, int sq_spec, struct ib_udata *udata)
{
	int rc = 0;
	u32 wqe_size;

	qp->sq_msn_prod = 0;
	qp->sq_msn_cons = 0;
	qp->sq_cmb_prod = 0;

	INIT_LIST_HEAD(&qp->sq_cmb_mmap.ctx_ent);

	if (!qp->has_sq) {
		if (buf) {
			buf->tbl_buf = NULL;
			buf->tbl_limit = 0;
			buf->tbl_pages = 0;
		}
		if (udata)
			rc = ionic_validate_qdesc_zero(sq);

		return rc;
	}

	rc = -EINVAL;

	if (max_wr < 0 || max_wr > 0xffff)
		goto err_sq;

	if (max_sge < 1 ||
	    max_sge > ionic_v1_send_wqe_max_sge(dev->max_stride, 0))
		goto err_sq;

	if (max_data < 0 ||
	    max_data > ionic_v1_send_wqe_max_data(dev->max_stride))
		goto err_sq;

	if (udata) {
		rc = ionic_validate_qdesc(sq);
		if (rc)
			goto err_sq;

		qp->sq_spec = sq_spec;

		qp->sq.ptr = NULL;
		qp->sq.size = sq->size;
		qp->sq.mask = sq->mask;
		qp->sq.depth_log2 = sq->depth_log2;
		qp->sq.stride_log2 = sq->stride_log2;

		qp->sq_meta = NULL;
		qp->sq_msn_idx = NULL;

#ifdef HAVE_IB_UMEM_GET_UDATA
		qp->sq_umem = ib_umem_get(udata, sq->addr,
					  sq->size, 0, 0);
#else
		qp->sq_umem = ib_umem_get(&ctx->ibctx, sq->addr,
					  sq->size, 0, 0);
#endif
		if (IS_ERR(qp->sq_umem)) {
			rc = PTR_ERR(qp->sq_umem);
			goto err_sq;
		}
	} else {
		qp->sq_umem = NULL;

		qp->sq_spec = ionic_v1_use_spec_sge(max_sge, sq_spec);
		if (sq_spec && !qp->sq_spec)
			ibdev_dbg(&dev->ibdev,
				  "init sq: max_sge %u disables spec\n",
				  max_sge);

		wqe_size = ionic_v1_send_wqe_min_size(max_sge, max_data,
						      qp->sq_spec);
		rc = ionic_queue_init(&qp->sq, dev->hwdev,
				      max_wr, wqe_size);
		if (rc)
			goto err_sq;

		ionic_queue_dbell_init(&qp->sq, qp->qpid);

		qp->sq_meta = kmalloc_array((u32)qp->sq.mask + 1,
					    sizeof(*qp->sq_meta),
					    GFP_KERNEL);
		if (!qp->sq_meta) {
			rc = -ENOMEM;
			goto err_sq_meta;
		}

		qp->sq_msn_idx = kmalloc_array((u32)qp->sq.mask + 1,
					       sizeof(*qp->sq_msn_idx),
					       GFP_KERNEL);
		if (!qp->sq_msn_idx) {
			rc = -ENOMEM;
			goto err_sq_msn;
		}
	}

	ionic_qp_sq_init_cmb(dev, qp, udata, max_data > 0);
	if (qp->sq_is_cmb)
		rc = ionic_pgtbl_init(dev, &qp->sq_res, buf, NULL,
				      (u64)qp->sq_cmb_pgid << PAGE_SHIFT, 1);
	else
		rc = ionic_pgtbl_init(dev, &qp->sq_res, buf,
				      qp->sq_umem, qp->sq.dma, 1);
	if (rc)
		goto err_sq_tbl;

	return 0;

err_sq_tbl:
	ionic_qp_sq_destroy_cmb(dev, ctx, qp);

	kfree(qp->sq_msn_idx);
err_sq_msn:
	kfree(qp->sq_meta);
err_sq_meta:
	if (qp->sq_umem)
		ib_umem_release(qp->sq_umem);
	else
		ionic_queue_destroy(&qp->sq, dev->hwdev);
err_sq:
	return rc;
}

static void ionic_qp_sq_destroy(struct ionic_ibdev *dev,
				struct ionic_ctx *ctx,
				struct ionic_qp *qp)
{
	if (!qp->has_sq)
		return;

	ionic_put_res(dev, &qp->sq_res);

	ionic_qp_sq_destroy_cmb(dev, ctx, qp);

	kfree(qp->sq_msn_idx);
	kfree(qp->sq_meta);

	if (qp->sq_umem)
		ib_umem_release(qp->sq_umem);
	else
		ionic_queue_destroy(&qp->sq, dev->hwdev);
}

static void ionic_qp_rq_init_cmb(struct ionic_ibdev *dev,
				 struct ionic_qp *qp,
				 struct ib_udata *udata)
{
	int rc;

	if (!qp->has_rq)
		goto not_in_cmb;

	qp->rq_cmb_order = order_base_2(qp->rq.size / PAGE_SIZE);

	if (qp->rq_cmb_order >= ionic_rqcmb_order)
		goto not_in_cmb;

	rc = ionic_api_get_cmb(dev->handle, &qp->rq_cmb_pgid,
			       &qp->rq_cmb_addr, qp->rq_cmb_order);
	if (rc)
		goto not_in_cmb;

	qp->rq_cmb_ptr = ioremap_wc(qp->rq_cmb_addr, qp->rq.size);
	if (!qp->rq_cmb_ptr)
		goto err_map;

	memset_io(qp->rq_cmb_ptr, 0, qp->rq.size);

	if (udata) {
		iounmap(qp->rq_cmb_ptr);
		qp->rq_cmb_ptr = NULL;
	}

	qp->rq_is_cmb = true;

	return;

err_map:
	ionic_api_put_cmb(dev->handle, qp->rq_cmb_pgid, qp->rq_cmb_order);
not_in_cmb:
	qp->rq_is_cmb = false;
	qp->rq_cmb_ptr = NULL;
	qp->rq_cmb_order = IONIC_RES_INVALID;
	qp->rq_cmb_pgid = 0;
	qp->rq_cmb_addr = 0;

	qp->rq_cmb_mmap.offset = 0;
	qp->rq_cmb_mmap.size = 0;
	qp->rq_cmb_mmap.pfn = 0;
}

static void ionic_qp_rq_destroy_cmb(struct ionic_ibdev *dev,
				    struct ionic_ctx *ctx,
				    struct ionic_qp *qp)
{
	if (!qp->rq_is_cmb)
		return;

	if (ctx) {
		mutex_lock(&ctx->mmap_mut);
		list_del(&qp->rq_cmb_mmap.ctx_ent);
		mutex_unlock(&ctx->mmap_mut);
	} else {
		iounmap(qp->rq_cmb_ptr);
	}

	ionic_api_put_cmb(dev->handle, qp->rq_cmb_pgid, qp->rq_cmb_order);
}

static int ionic_qp_rq_init(struct ionic_ibdev *dev, struct ionic_ctx *ctx,
			    struct ionic_qp *qp, struct ionic_qdesc *rq,
			    struct ionic_tbl_buf *buf, int max_wr, int max_sge,
			    int rq_spec, struct ib_udata *udata)
{
	u32 wqe_size;
	int rc = 0, i;

	qp->rq_cmb_prod = 0;

	INIT_LIST_HEAD(&qp->rq_cmb_mmap.ctx_ent);

	if (!qp->has_rq) {
		if (buf) {
			buf->tbl_buf = NULL;
			buf->tbl_limit = 0;
			buf->tbl_pages = 0;
		}
		if (udata)
			rc = ionic_validate_qdesc_zero(rq);

		return rc;
	}

	rc = -EINVAL;

	if (max_wr < 0 || max_wr > 0xffff)
		goto err_rq;

	if (max_sge < 1 ||
	    max_sge > ionic_v1_recv_wqe_max_sge(dev->max_stride, 0))
		goto err_rq;

	if (udata) {
		rc = ionic_validate_qdesc(rq);
		if (rc)
			goto err_rq;

		qp->rq_spec = rq_spec;

		qp->rq.ptr = NULL;
		qp->rq.size = rq->size;
		qp->rq.mask = rq->mask;
		qp->rq.depth_log2 = rq->depth_log2;
		qp->rq.stride_log2 = rq->stride_log2;

		qp->rq_meta = NULL;

#ifdef HAVE_IB_UMEM_GET_UDATA
		qp->rq_umem = ib_umem_get(udata, rq->addr, rq->size, 0, 0);
#else
		qp->rq_umem = ib_umem_get(&ctx->ibctx, rq->addr,
					  rq->size, 0, 0);
#endif
		if (IS_ERR(qp->rq_umem)) {
			rc = PTR_ERR(qp->rq_umem);
			goto err_rq;
		}
	} else {
		qp->rq_umem = NULL;
		qp->rq_res.tbl_order = IONIC_RES_INVALID;
		qp->rq_res.tbl_pos = 0;

		qp->rq_spec = ionic_v1_use_spec_sge(max_sge, rq_spec);
		if (rq_spec && !qp->rq_spec)
			ibdev_dbg(&dev->ibdev,
				  "init rq: max_sge %u disables spec\n",
				  max_sge);

		wqe_size = ionic_v1_recv_wqe_min_size(max_sge, qp->rq_spec);
		rc = ionic_queue_init(&qp->rq, dev->hwdev,
				      max_wr, wqe_size);
		if (rc)
			goto err_rq;

		ionic_queue_dbell_init(&qp->rq, qp->qpid);

		qp->rq_meta = kmalloc_array((u32)qp->rq.mask + 1,
					    sizeof(*qp->rq_meta),
					    GFP_KERNEL);
		if (!qp->rq_meta) {
			rc = -ENOMEM;
			goto err_rq_meta;
		}

		for (i = 0; i < qp->rq.mask; ++i)
			qp->rq_meta[i].next = &qp->rq_meta[i + 1];
		qp->rq_meta[i].next = IONIC_META_LAST;
		qp->rq_meta_head = &qp->rq_meta[0];
	}

	ionic_qp_rq_init_cmb(dev, qp, udata);
	if (qp->rq_is_cmb)
		rc = ionic_pgtbl_init(dev, &qp->rq_res, buf, NULL,
				      (u64)qp->rq_cmb_pgid << PAGE_SHIFT, 1);
	else
		rc = ionic_pgtbl_init(dev, &qp->rq_res, buf,
				      qp->rq_umem, qp->rq.dma, 1);
	if (rc)
		goto err_rq_tbl;

	return 0;

err_rq_tbl:
	ionic_qp_rq_destroy_cmb(dev, ctx, qp);

	kfree(qp->rq_meta);
err_rq_meta:
	if (qp->rq_umem)
		ib_umem_release(qp->rq_umem);
	else
		ionic_queue_destroy(&qp->rq, dev->hwdev);
err_rq:

	return rc;
}

static void ionic_qp_rq_destroy(struct ionic_ibdev *dev,
				struct ionic_ctx *ctx,
				struct ionic_qp *qp)
{
	if (!qp->has_rq)
		return;

	ionic_put_res(dev, &qp->rq_res);

	ionic_qp_rq_destroy_cmb(dev, ctx, qp);

	kfree(qp->rq_meta);

	if (qp->rq_umem)
		ib_umem_release(qp->rq_umem);
	else
		ionic_queue_destroy(&qp->rq, dev->hwdev);
}

static struct ib_qp *ionic_create_qp(struct ib_pd *ibpd,
				     struct ib_qp_init_attr *attr,
				     struct ib_udata *udata)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibpd->device);
#ifdef HAVE_RDMA_UDATA_DRV_CTX
	struct ionic_ctx *ctx =
		rdma_udata_to_drv_context(udata, struct ionic_ctx, ibctx);
#else
	struct ionic_ctx *ctx = to_ionic_ctx_uobj(ibpd->uobject);
#endif
	struct ionic_pd *pd = to_ionic_pd(ibpd);
	struct ionic_qp *qp;
	struct ionic_cq *cq;
	struct ionic_qp_req req;
	struct ionic_qp_resp resp = {0};
	struct ionic_tbl_buf sq_buf = {0}, rq_buf = {0};
	unsigned long irqflags;
	int rc;

	if (udata) {
		rc = ib_copy_from_udata(&req, udata, sizeof(req));
		if (rc)
			goto err_req;
	} else {
		req.sq_spec = ionic_spec;
		req.rq_spec = ionic_spec;
	}

	if (attr->qp_type == IB_QPT_SMI ||
	    attr->qp_type > IB_QPT_UD) {
		rc = -EINVAL;
		goto err_qp;
	}

	qp = kzalloc(sizeof(*qp), GFP_KERNEL);
	if (!qp) {
		rc = -ENOMEM;
		goto err_qp;
	}

	qp->state = IB_QPS_RESET;

	INIT_LIST_HEAD(&qp->cq_poll_sq);
	INIT_LIST_HEAD(&qp->cq_flush_sq);
	INIT_LIST_HEAD(&qp->cq_flush_rq);

	spin_lock_init(&qp->sq_lock);
	spin_lock_init(&qp->rq_lock);

	if (attr->qp_type == IB_QPT_GSI)
		rc = ionic_get_gsi_qpid(dev, &qp->qpid);
	else
		rc = ionic_get_qpid(dev, &qp->qpid);
	if (rc)
		goto err_qpid;

	qp->has_ah = attr->qp_type == IB_QPT_RC;

	qp->has_sq = true;
	qp->has_rq = true;

	if (qp->has_ah) {
		qp->hdr = kzalloc(sizeof(*qp->hdr), GFP_KERNEL);
		if (!qp->hdr) {
			rc = -ENOMEM;
			goto err_ah_alloc;
		}

		rc = ionic_get_ahid(dev, &qp->ahid);
		if (rc)
			goto err_ahid;
	}

	qp->rrq_res.tbl_order = IONIC_RES_INVALID;
	qp->rrq_res.tbl_pos = 0;
	qp->rsq_res.tbl_order = IONIC_RES_INVALID;
	qp->rsq_res.tbl_pos = 0;

	rc = ionic_qp_sq_init(dev, ctx, qp, &req.sq, &sq_buf,
			      attr->cap.max_send_wr, attr->cap.max_send_sge,
			      attr->cap.max_inline_data, req.sq_spec, udata);
	if (rc)
		goto err_sq;

	rc = ionic_qp_rq_init(dev, ctx, qp, &req.rq, &rq_buf,
			      attr->cap.max_recv_wr, attr->cap.max_recv_sge,
			      req.rq_spec, udata);
	if (rc)
		goto err_rq;

	rc = ionic_create_qp_cmd(dev, pd,
				 to_ionic_cq(attr->send_cq),
				 to_ionic_cq(attr->recv_cq),
				 qp, &sq_buf, &rq_buf, attr);
	if (rc)
		goto err_cmd;

	if (udata) {
		resp.qpid = qp->qpid;

		if (qp->sq_is_cmb) {
			qp->sq_cmb_mmap.size = qp->sq.size;
			qp->sq_cmb_mmap.pfn = PHYS_PFN(qp->sq_cmb_addr);
			qp->sq_cmb_mmap.writecombine = true;

			mutex_lock(&ctx->mmap_mut);
			qp->sq_cmb_mmap.offset = ctx->mmap_off;
			ctx->mmap_off += qp->sq.size;
			list_add(&qp->sq_cmb_mmap.ctx_ent, &ctx->mmap_list);
			mutex_unlock(&ctx->mmap_mut);

			resp.sq_cmb_offset = qp->sq_cmb_mmap.offset;
		}

		if (qp->rq_is_cmb) {
			qp->rq_cmb_mmap.size = qp->rq.size;
			qp->rq_cmb_mmap.pfn = PHYS_PFN(qp->rq_cmb_addr);
			qp->rq_cmb_mmap.writecombine = true;

			mutex_lock(&ctx->mmap_mut);
			qp->rq_cmb_mmap.offset = ctx->mmap_off;
			ctx->mmap_off += qp->rq.size;
			list_add(&qp->rq_cmb_mmap.ctx_ent, &ctx->mmap_list);
			mutex_unlock(&ctx->mmap_mut);

			resp.rq_cmb_offset = qp->rq_cmb_mmap.offset;
		}

		rc = ib_copy_to_udata(udata, &resp, sizeof(resp));
		if (rc)
			goto err_resp;
	}

	ionic_pgtbl_unbuf(dev, &rq_buf);
	ionic_pgtbl_unbuf(dev, &sq_buf);

	qp->ibqp.qp_num = qp->qpid;

	init_completion(&qp->qp_rel_comp);
	kref_init(&qp->qp_kref);

	rc = xa_err(xa_store_irq(&dev->qp_tbl, qp->qpid, qp, GFP_KERNEL));
	if (rc)
		goto err_xa;

	if (qp->has_sq) {
		cq = to_ionic_cq(attr->send_cq);
		spin_lock_irqsave(&cq->lock, irqflags);
		spin_unlock_irqrestore(&cq->lock, irqflags);

		attr->cap.max_send_wr = qp->sq.mask;
		attr->cap.max_send_sge =
			ionic_v1_send_wqe_max_sge(qp->sq.stride_log2,
						  qp->sq_spec);
		attr->cap.max_inline_data =
			ionic_v1_send_wqe_max_data(qp->sq.stride_log2);
		qp->sq_cqid = cq->cqid;
	}

	if (qp->has_rq) {
		cq = to_ionic_cq(attr->recv_cq);
		spin_lock_irqsave(&cq->lock, irqflags);
		spin_unlock_irqrestore(&cq->lock, irqflags);

		attr->cap.max_recv_wr = qp->rq.mask;
		attr->cap.max_recv_sge =
			ionic_v1_recv_wqe_max_sge(qp->rq.stride_log2,
						  qp->rq_spec);
		qp->rq_cqid = cq->cqid;
	}

	spin_lock_irqsave(&dev->dev_lock, irqflags);
	list_add_tail(&qp->qp_list_ent, &dev->qp_list);
	spin_unlock_irqrestore(&dev->dev_lock, irqflags);

	ionic_dbg_add_qp(dev, qp);

	return &qp->ibqp;

err_xa:
err_resp:
	ionic_destroy_qp_cmd(dev, qp->qpid);
err_cmd:
	ionic_pgtbl_unbuf(dev, &rq_buf);
	ionic_qp_rq_destroy(dev, ctx, qp);
err_rq:
	ionic_pgtbl_unbuf(dev, &sq_buf);
	ionic_qp_sq_destroy(dev, ctx, qp);
err_sq:
	if (qp->has_ah)
		ionic_put_ahid(dev, qp->ahid);
err_ahid:
	kfree(qp->hdr);
err_ah_alloc:
	ionic_put_qpid(dev, qp->qpid);
err_qpid:
	kfree(qp);
err_qp:
err_req:
	return ERR_PTR(rc);
}

void ionic_notify_flush_cq(struct ionic_cq *cq)
{
	if (cq->flush && cq->ibcq.comp_handler)
		cq->ibcq.comp_handler(&cq->ibcq, cq->ibcq.cq_context);
}

static void ionic_notify_qp_cqs(struct ionic_qp *qp)
{
	if (qp->ibqp.send_cq)
		ionic_notify_flush_cq(to_ionic_cq(qp->ibqp.send_cq));
	if (qp->ibqp.recv_cq && qp->ibqp.recv_cq != qp->ibqp.send_cq)
		ionic_notify_flush_cq(to_ionic_cq(qp->ibqp.recv_cq));
}

void ionic_flush_qp(struct ionic_qp *qp)
{
	struct ionic_cq *cq;
	unsigned long irqflags;

	if (qp->ibqp.send_cq) {
		cq = to_ionic_cq(qp->ibqp.send_cq);

		/* Hold the CQ lock and QP sq_lock to set up flush */
		spin_lock_irqsave(&cq->lock, irqflags);
		spin_lock(&qp->sq_lock);
		qp->sq_flush = true;
		if (!ionic_queue_empty(&qp->sq)) {
			cq->flush = true;
			list_move_tail(&qp->cq_flush_sq, &cq->flush_sq);
		}
		spin_unlock(&qp->sq_lock);
		spin_unlock_irqrestore(&cq->lock, irqflags);
	}

	if (qp->ibqp.recv_cq) {
		cq = to_ionic_cq(qp->ibqp.recv_cq);

		/* Hold the CQ lock and QP rq_lock to set up flush */
		spin_lock_irqsave(&cq->lock, irqflags);
		spin_lock(&qp->rq_lock);
		qp->rq_flush = true;
		if (!ionic_queue_empty(&qp->rq)) {
			cq->flush = true;
			list_move_tail(&qp->cq_flush_rq, &cq->flush_rq);
		}
		spin_unlock(&qp->rq_lock);
		spin_unlock_irqrestore(&cq->lock, irqflags);
	}
}

static void ionic_clean_cq(struct ionic_cq *cq, u32 qpid)
{
	struct ionic_v1_cqe *qcqe;
	int prod, qtf, qid, type;
	bool color;

	if (!cq->q.ptr)
		return;

	color = cq->color;
	prod = cq->q.prod;
	qcqe = ionic_queue_at(&cq->q, prod);

	while (color == ionic_v1_cqe_color(qcqe)) {
		qtf = ionic_v1_cqe_qtf(qcqe);
		qid = ionic_v1_cqe_qtf_qid(qtf);
		type = ionic_v1_cqe_qtf_type(qtf);

		if (qid == qpid && type != IONIC_V1_CQE_TYPE_ADMIN)
			ionic_v1_cqe_clean(qcqe);

		prod = ionic_queue_next(&cq->q, prod);
		qcqe = ionic_queue_at(&cq->q, prod);
		color = ionic_color_wrap(prod, color);
	}
}

static void ionic_reset_qp(struct ionic_qp *qp)
{
	struct ionic_cq *cq;
	unsigned long irqflags;
	int i;

	local_irq_save(irqflags);

	if (qp->ibqp.send_cq) {
		cq = to_ionic_cq(qp->ibqp.send_cq);
		spin_lock(&cq->lock);
		ionic_clean_cq(cq, qp->qpid);
		spin_unlock(&cq->lock);
	}

	if (qp->ibqp.recv_cq) {
		cq = to_ionic_cq(qp->ibqp.recv_cq);
		spin_lock(&cq->lock);
		ionic_clean_cq(cq, qp->qpid);
		spin_unlock(&cq->lock);
	}

	if (qp->has_sq) {
		spin_lock(&qp->sq_lock);
		qp->sq_flush = false;
		qp->sq_flush_rcvd = false;
		qp->sq_msn_prod = 0;
		qp->sq_msn_cons = 0;
		qp->sq_cmb_prod = 0;
		qp->sq.prod = 0;
		qp->sq.cons = 0;
		spin_unlock(&qp->sq_lock);
	}

	if (qp->has_rq) {
		spin_lock(&qp->rq_lock);
		qp->rq_flush = false;
		qp->rq_cmb_prod = 0;
		qp->rq.prod = 0;
		qp->rq.cons = 0;
		for (i = 0; i < qp->rq.mask; ++i)
			qp->rq_meta[i].next = &qp->rq_meta[i + 1];
		qp->rq_meta[i].next = IONIC_META_LAST;
		qp->rq_meta_head = &qp->rq_meta[0];
		spin_unlock(&qp->rq_lock);
	}

	local_irq_restore(irqflags);
}

static bool ionic_qp_cur_state_is_ok(enum ib_qp_state q_state,
				     enum ib_qp_state attr_state)
{
	if (q_state == attr_state)
		return true;

	if (attr_state == IB_QPS_ERR)
		return true;

	if (attr_state == IB_QPS_SQE)
		return q_state == IB_QPS_RTS || q_state == IB_QPS_SQD;

	return false;
}

static int ionic_check_modify_qp(struct ionic_qp *qp, struct ib_qp_attr *attr,
				 int mask)
{
	enum ib_qp_state cur_state = (mask & IB_QP_CUR_STATE) ?
		attr->cur_qp_state : qp->state;
	enum ib_qp_state next_state = (mask & IB_QP_STATE) ?
		attr->qp_state : cur_state;

	if ((mask & IB_QP_CUR_STATE) &&
	    !ionic_qp_cur_state_is_ok(qp->state, attr->cur_qp_state))
		return -EINVAL;

	if (!ib_modify_qp_is_ok(cur_state, next_state, qp->ibqp.qp_type, mask))
		return -EINVAL;

	/* unprivileged qp not allowed privileged qkey */
	if ((mask & IB_QP_QKEY) && (attr->qkey & 0x80000000) &&
	    qp->ibqp.uobject)
		return -EPERM;

	return 0;
}

static int ionic_modify_qp(struct ib_qp *ibqp, struct ib_qp_attr *attr,
			   int mask, struct ib_udata *udata)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibqp->device);
	struct ionic_qp *qp = to_ionic_qp(ibqp);
	unsigned long max_rsrq;
	int rc;

	rc = ionic_check_modify_qp(qp, attr, mask);
	if (rc)
		goto err_qp;

	if (mask & IB_QP_CAP) {
		rc = -EINVAL;
		goto err_qp;
	}

	if (mask & IB_QP_AV)
		qp->dcqcn_profile =
			ionic_dcqcn_select_profile(dev, &attr->ah_attr);

	if ((mask & IB_QP_MAX_QP_RD_ATOMIC) && attr->max_rd_atomic) {
		WARN_ON(ionic_put_res(dev, &qp->rrq_res));

		/* Account for hole and round down to the next order */
		max_rsrq = attr->max_rd_atomic;
		max_rsrq = rounddown_pow_of_two(max_rsrq + 1);

		qp->rrq_res.tbl_order = ionic_res_order(max_rsrq,
							dev->rrq_stride,
							dev->cl_stride);

		rc = ionic_get_res(dev, &qp->rrq_res);
		if (rc)
			goto err_qp;
	}

	if ((mask & IB_QP_MAX_DEST_RD_ATOMIC) && attr->max_dest_rd_atomic) {
		WARN_ON(ionic_put_res(dev, &qp->rsq_res));

		/* Account for hole and round up to the next order */
		max_rsrq = attr->max_dest_rd_atomic;
		max_rsrq = roundup_pow_of_two(max_rsrq + 1);

		qp->rsq_res.tbl_order = ionic_res_order(max_rsrq,
							dev->rsq_stride,
							dev->cl_stride);

		rc = ionic_get_res(dev, &qp->rsq_res);
		if (rc)
			goto err_qp;
	}

	rc = ionic_modify_qp_cmd(dev, qp, attr, mask);
	if (rc)
		goto err_qp;

	if (mask & IB_QP_STATE) {
		qp->state = attr->qp_state;

		if (attr->qp_state == IB_QPS_ERR) {
			ionic_flush_qp(qp);
			ionic_notify_qp_cqs(qp);
		} else if (attr->qp_state == IB_QPS_RESET) {
			ionic_reset_qp(qp);
			ionic_put_res(dev, &qp->rrq_res);
			ionic_put_res(dev, &qp->rsq_res);
		}
	}

	return 0;

err_qp:
	if (mask & IB_QP_MAX_QP_RD_ATOMIC)
		ionic_put_res(dev, &qp->rrq_res);

	if (mask & IB_QP_MAX_DEST_RD_ATOMIC)
		ionic_put_res(dev, &qp->rsq_res);

	return rc;
}

static int ionic_query_qp(struct ib_qp *ibqp, struct ib_qp_attr *attr,
			  int mask, struct ib_qp_init_attr *init_attr)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibqp->device);
	struct ionic_qp *qp = to_ionic_qp(ibqp);
	int rc;

	memset(attr, 0, sizeof(*attr));
	memset(init_attr, 0, sizeof(*init_attr));

	rc = ionic_query_qp_cmd(dev, qp, attr, mask);
	if (rc)
		goto err_cmd;

	if (qp->has_sq)
		attr->cap.max_send_wr = qp->sq.mask;

	if (qp->has_rq)
		attr->cap.max_recv_wr = qp->rq.mask;

	init_attr->event_handler = ibqp->event_handler;
	init_attr->qp_context = ibqp->qp_context;
	init_attr->send_cq = ibqp->send_cq;
	init_attr->recv_cq = ibqp->recv_cq;
	init_attr->srq = ibqp->srq;
	init_attr->xrcd = ibqp->xrcd;
	init_attr->cap = attr->cap;
	init_attr->sq_sig_type = qp->sig_all ?
		IB_SIGNAL_ALL_WR : IB_SIGNAL_REQ_WR;
	init_attr->qp_type = ibqp->qp_type;
	init_attr->create_flags = 0;
	init_attr->port_num = 0;
#ifdef HAVE_QP_RWQ_IND_TBL
	init_attr->rwq_ind_tbl = ibqp->rwq_ind_tbl;
#endif
#ifdef HAVE_QP_INIT_SRC_QPN
	init_attr->source_qpn = 0;
#endif

err_cmd:
	return rc;
}

#ifdef HAVE_IB_API_UDATA
static int ionic_destroy_qp(struct ib_qp *ibqp, struct ib_udata *udata)
#else
static int ionic_destroy_qp(struct ib_qp *ibqp)
#endif
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibqp->device);
#if defined(HAVE_IB_API_UDATA) && defined(HAVE_RDMA_UDATA_DRV_CTX)
	struct ionic_ctx *ctx =
		rdma_udata_to_drv_context(udata, struct ionic_ctx, ibctx);
#else
	struct ionic_ctx *ctx = to_ionic_ctx_uobj(ibqp->uobject);
#endif
	struct ionic_qp *qp = to_ionic_qp(ibqp);
	struct ionic_cq *cq;
	unsigned long irqflags;
	int rc;

	rc = ionic_destroy_qp_cmd(dev, qp->qpid);
	if (rc)
		return rc;

	xa_erase_irq(&dev->qp_tbl, qp->qpid);

	kref_put(&qp->qp_kref, ionic_qp_complete);
	wait_for_completion(&qp->qp_rel_comp);

	spin_lock_irqsave(&dev->dev_lock, irqflags);
	list_del(&qp->qp_list_ent);
	spin_unlock_irqrestore(&dev->dev_lock, irqflags);

	ionic_dbg_rm_qp(qp);

	if (qp->ibqp.send_cq) {
		cq = to_ionic_cq(qp->ibqp.send_cq);
		spin_lock_irqsave(&cq->lock, irqflags);
		ionic_clean_cq(cq, qp->qpid);
		list_del(&qp->cq_poll_sq);
		list_del(&qp->cq_flush_sq);
		spin_unlock_irqrestore(&cq->lock, irqflags);
	}

	if (qp->ibqp.recv_cq) {
		cq = to_ionic_cq(qp->ibqp.recv_cq);
		spin_lock_irqsave(&cq->lock, irqflags);
		ionic_clean_cq(cq, qp->qpid);
		list_del(&qp->cq_flush_rq);
		spin_unlock_irqrestore(&cq->lock, irqflags);
	}

	ionic_qp_rq_destroy(dev, ctx, qp);
	ionic_qp_sq_destroy(dev, ctx, qp);
	if (qp->has_ah) {
		ionic_put_ahid(dev, qp->ahid);
		kfree(qp->hdr);
	}
	ionic_put_qpid(dev, qp->qpid);

	ionic_put_res(dev, &qp->rrq_res);
	ionic_put_res(dev, &qp->rsq_res);

	kfree(qp);

	return 0;
}

static const struct ib_device_ops ionic_controlpath_ops = {
#ifdef HAVE_RDMA_DEV_OPS_EXT
	.driver_id		= RDMA_DRIVER_IONIC,
#endif
	.alloc_ucontext		= ionic_alloc_ucontext,
	.dealloc_ucontext	= ionic_dealloc_ucontext,
	.mmap			= ionic_mmap,

	.alloc_pd		= ionic_alloc_pd,
	.dealloc_pd		= ionic_dealloc_pd,

	.create_ah		= ionic_create_ah,
	.query_ah		= ionic_query_ah,
	.destroy_ah		= ionic_destroy_ah,

	.get_dma_mr		= ionic_get_dma_mr,
	.reg_user_mr		= ionic_reg_user_mr,
	.rereg_user_mr		= ionic_rereg_user_mr,
	.dereg_mr		= ionic_dereg_mr,
	.alloc_mr		= ionic_alloc_mr,
	.map_mr_sg		= ionic_map_mr_sg,

	.alloc_mw		= ionic_alloc_mw,
	.dealloc_mw		= ionic_dealloc_mw,

	.create_cq		= ionic_create_cq,
	.destroy_cq		= ionic_destroy_cq,

	.create_qp		= ionic_create_qp,
	.modify_qp		= ionic_modify_qp,
	.query_qp		= ionic_query_qp,
	.destroy_qp		= ionic_destroy_qp,
#ifdef HAVE_IB_ALLOC_UCTX_OBJ
	INIT_RDMA_OBJ_SIZE(ib_ucontext, ionic_ctx, ibctx),
#endif
#ifdef HAVE_IB_ALLOC_PD_OBJ
	INIT_RDMA_OBJ_SIZE(ib_pd, ionic_pd, ibpd),
#endif
#ifdef HAVE_IB_ALLOC_AH_OBJ
	INIT_RDMA_OBJ_SIZE(ib_ah, ionic_ah, ibah),
#endif
#ifdef HAVE_IB_ALLOC_CQ_OBJ
	INIT_RDMA_OBJ_SIZE(ib_cq, ionic_cq, ibcq),
#endif
};

void ionic_controlpath_setops(struct ionic_ibdev *dev)
{
	ib_set_device_ops(&dev->ibdev, &ionic_controlpath_ops);

	dev->ibdev.uverbs_cmd_mask |=
		BIT_ULL(IB_USER_VERBS_CMD_ALLOC_PD)		|
		BIT_ULL(IB_USER_VERBS_CMD_DEALLOC_PD)		|
		BIT_ULL(IB_USER_VERBS_CMD_CREATE_AH)		|
		BIT_ULL(IB_USER_VERBS_CMD_QUERY_AH)		|
		BIT_ULL(IB_USER_VERBS_CMD_DESTROY_AH)		|
		BIT_ULL(IB_USER_VERBS_CMD_REG_MR)		|
		BIT_ULL(IB_USER_VERBS_CMD_REG_SMR)		|
		BIT_ULL(IB_USER_VERBS_CMD_DEREG_MR)		|
		BIT_ULL(IB_USER_VERBS_CMD_ALLOC_MW)		|
		BIT_ULL(IB_USER_VERBS_CMD_BIND_MW)		|
		BIT_ULL(IB_USER_VERBS_CMD_DEALLOC_MW)		|
		BIT_ULL(IB_USER_VERBS_CMD_CREATE_COMP_CHANNEL)	|
		BIT_ULL(IB_USER_VERBS_CMD_CREATE_CQ)		|
		BIT_ULL(IB_USER_VERBS_CMD_DESTROY_CQ)		|
		BIT_ULL(IB_USER_VERBS_CMD_CREATE_QP)		|
		BIT_ULL(IB_USER_VERBS_CMD_QUERY_QP)		|
		BIT_ULL(IB_USER_VERBS_CMD_MODIFY_QP)		|
		BIT_ULL(IB_USER_VERBS_CMD_DESTROY_QP)		|
		0;
	dev->ibdev.uverbs_ex_cmd_mask =
		BIT_ULL(IB_USER_VERBS_EX_CMD_CREATE_QP)		|
#ifdef HAVE_EX_CMD_MODIFY_QP
		BIT_ULL(IB_USER_VERBS_EX_CMD_MODIFY_QP)		|
#endif
		0;
}
