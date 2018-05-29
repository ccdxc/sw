#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/mman.h>
#include <net/addrconf.h>
#include <rdma/ib_addr.h>
#include <rdma/ib_cache.h>
#include <rdma/ib_mad.h>

#include "ionic_fw.h"
#include "ionic_ibdev.h"
#include "ionic_ibdebug.h"

MODULE_AUTHOR("Allen Hubbe <allenbh@pensando.io>");
MODULE_DESCRIPTION("Pensando Capri RoCE HCA driver");
MODULE_LICENSE("Dual BSD/GPL");

#define DRIVER_NAME "ionic_rdma"
#define DRIVER_VERSION "0.2 pre-release"
#define DRIVER_DESCRIPTION "Pensando Capri RoCE HCA driver"
#define DEVICE_DESCRIPTION "Pensando Capri RoCE HCA"

#define PHYS_STATE_UP 5
#define PHYS_STATE_DOWN 3

/* XXX remove this section for release */
static bool ionic_xxx_haps = false;
module_param_named(xxx_haps, ionic_xxx_haps, bool, 0444);
MODULE_PARM_DESC(xxx_haps, "XXX Misc workarounds for HAPS.");
static bool ionic_xxx_limits = false;
module_param_named(xxx_limits, ionic_xxx_limits, bool, 0444);
MODULE_PARM_DESC(xxx_limits, "XXX Hardcode resource limits.");
static bool ionic_xxx_kdbid = false;
module_param_named(xxx_kdbid, ionic_xxx_kdbid, bool, 0444);
MODULE_PARM_DESC(xxx_kdbid, "XXX Kernel doorbell id in user space.");
static bool ionic_xxx_notify = false;
module_param_named(xxx_notify, ionic_xxx_notify, bool, 0444);
MODULE_PARM_DESC(xxx_notify, "XXX Fake notify without arming cq.");
/* XXX remove above section for release */

static bool ionic_dbgfs_enable = true; /* XXX false for release */
module_param_named(dbgfs, ionic_dbgfs_enable, bool, 0444);
MODULE_PARM_DESC(dbgfs, "Enable debugfs for this driver.");

static u16 ionic_eq_depth = 0x1ff; /* XXX needs tuning */
module_param_named(eq_depth, ionic_eq_depth, ushort, 0444);
MODULE_PARM_DESC(eq_depth, "Min depth for event queues.");

static u16 ionic_eq_isr_budget = 10; /* XXX needs tuning */
module_param_named(isr_budget, ionic_eq_isr_budget, ushort, 0644);
MODULE_PARM_DESC(isr_budget, "Max events to poll per round in isr context.");

static u16 ionic_eq_work_budget = 1000; /* XXX needs tuning */
module_param_named(work_budget, ionic_eq_work_budget, ushort, 0644);
MODULE_PARM_DESC(work_budget, "Max events to poll per round in work context.");

static bool ionic_sqhbm_inline = true;
module_param_named(sqhbm_inline, ionic_sqhbm_inline, bool, 0644);
MODULE_PARM_DESC(sqhbm_inline, "Only alloc sq hbm for inline data capability.");

static int ionic_sqhbm_order = 0; /* XXX needs tuning */
module_param_named(sqhbm_order, ionic_sqhbm_order, int, 0644);
MODULE_PARM_DESC(sqhbm_order, "Only alloc sq hbm less than order.");

static struct workqueue_struct *ionic_workq;

/* access single-threaded thru ionic_workq cpu 0 */
static LIST_HEAD(ionic_ibdev_list);

static int ionic_validate_udata(struct ib_udata *udata,
				size_t inlen, size_t outlen)
{
	if (udata) {
		if (udata->inlen != inlen || udata->outlen != outlen) {
			pr_debug("have udata in %lu out %lu want %lu %lu\n",
				 udata->inlen, udata->outlen,
				 inlen, outlen);
			return -EINVAL;
		}
	} else {
		if (inlen != 0 || outlen != 0) {
			pr_debug("no udata want %lu %lu\n",
				 inlen, outlen);
			return -EINVAL;
		}
	}

	return 0;
}

static int ionic_validate_qdesc(struct ionic_qdesc *q)
{
	if (!q->addr || !q->size || !q->mask || !q->stride)
		return -EINVAL;

	if (q->addr & (PAGE_SIZE - 1))
		return -EINVAL;

	if (q->mask & (q->mask + 1))
		return -EINVAL;

	if (q->stride & (q->stride - 1))
		return -EINVAL;

	if (q->size < ((u32)q->mask + 1) * q->stride)
		return -EINVAL;

	return 0;
}

static int ionic_validate_qdesc_zero(struct ionic_qdesc *q)
{
	if (q->addr || q->size || q->mask || q->stride)
		return -EINVAL;

	return 0;
}

static int ionic_verbs_status_to_rc(u32 status)
{
	/* TODO */
	if (status)
		return -EIO;

	return 0;
}

static int ionic_get_pdid(struct ionic_ibdev *dev, u32 *pdid)
{
	u32 id;

	mutex_lock(&dev->inuse_lock);

	id = find_next_zero_bit(dev->inuse_pdid, dev->size_pdid, dev->next_pdid);
	if (id != dev->size_pdid)
		goto found;

	id = find_first_zero_bit(dev->inuse_pdid, dev->next_pdid);
	if (id != dev->next_pdid)
		goto found;

	mutex_unlock(&dev->inuse_lock);

	/* not found */
	return -ENOMEM;

found:
	set_bit(id, dev->inuse_pdid);
	dev->next_pdid = id + 1;

	mutex_unlock(&dev->inuse_lock);

	*pdid = id;

	return 0;
}

static int ionic_get_mrid(struct ionic_ibdev *dev, u32 *lkey, u32 *rkey)
{
	u32 id, key;

	mutex_lock(&dev->inuse_lock);

	id = find_next_zero_bit(dev->inuse_mrid, dev->size_mrid, dev->next_mrid);
	if (id != dev->size_mrid)
		goto found;

	id = find_first_zero_bit(dev->inuse_mrid, dev->next_mrid);
	if (id != dev->next_mrid)
		goto found;

	mutex_unlock(&dev->inuse_lock);

	/* not found */
	return -ENOMEM;

found:
	set_bit(id, dev->inuse_mrid);
	dev->next_mrid = id + 1;

	key = dev->next_rkey_key++;

	mutex_unlock(&dev->inuse_lock);

	if (0) {
		/* XXX rkey key doesn't work yet */
		*lkey = id;
		*rkey = id | (key << 24);
	} else {
		/* XXX for now, lkey rkey even odd */
		*lkey = id * 2;
		*rkey = id * 2 + 1;
	}

	return 0;
}

static void ionic_replace_rkey(struct ionic_ibdev *dev, u32 *rkey)
{
	u32 key;

	mutex_lock(&dev->inuse_lock);
	key = dev->next_rkey_key++;
	mutex_unlock(&dev->inuse_lock);

	*rkey = (*rkey & 0xffffff) | (key << 24);
}

static int ionic_get_pgtbl(struct ionic_ibdev *dev, u32 *pos, int order)
{
	int rc;

	mutex_lock(&dev->inuse_lock);
	rc = bitmap_find_free_region(dev->inuse_pgtbl, dev->size_pgtbl, order);
	mutex_unlock(&dev->inuse_lock);

	if (rc < 0)
		return rc;

	*pos = (u32)rc;

	return 0;
}

static int ionic_get_cqid(struct ionic_ibdev *dev, u32 *cqid)
{
	u32 id;

	mutex_lock(&dev->inuse_lock);

	id = find_next_zero_bit(dev->inuse_cqid, dev->size_cqid, dev->next_cqid);
	if (id != dev->size_cqid)
		goto found;

	id = find_first_zero_bit(dev->inuse_cqid, dev->next_cqid);
	if (id != dev->next_cqid)
		goto found;

	mutex_unlock(&dev->inuse_lock);

	/* not found */
	return -ENOMEM;

found:
	set_bit(id, dev->inuse_cqid);
	dev->next_cqid = id + 1;

	mutex_unlock(&dev->inuse_lock);

	*cqid = id;

	return 0;
}

static int ionic_get_gsi_qpid(struct ionic_ibdev *dev, u32 *qpid)
{
	int rc = 0;

	mutex_lock(&dev->inuse_lock);
	if (test_bit(IB_QPT_GSI, dev->inuse_qpid))
		rc = -EINVAL;
	else {
		set_bit(IB_QPT_GSI, dev->inuse_qpid);
		*qpid = IB_QPT_GSI;
	}
	mutex_unlock(&dev->inuse_lock);

	return rc;
}

static int ionic_get_qpid(struct ionic_ibdev *dev, u32 *qpid)
{
	u32 id;

	mutex_lock(&dev->inuse_lock);

	id = find_next_zero_bit(dev->inuse_qpid, dev->size_qpid, dev->next_qpid);
	if (id != dev->size_qpid)
		goto found;

	id = find_next_zero_bit(dev->inuse_qpid, dev->next_qpid, 2);
	if (id != dev->next_qpid)
		goto found;

	mutex_unlock(&dev->inuse_lock);

	/* not found */
	return -ENOMEM;

found:
	set_bit(id, dev->inuse_qpid);
	dev->next_qpid = id + 1;

	mutex_unlock(&dev->inuse_lock);

	*qpid = id;

	return 0;
}

static int ionic_get_srqid(struct ionic_ibdev *dev, u32 *qpid)
{
	u32 id;

	mutex_lock(&dev->inuse_lock);

	id = find_next_zero_bit(dev->inuse_qpid, dev->size_srqid, dev->next_srqid);
	if (id != dev->size_srqid)
		goto found;

	id = find_next_zero_bit(dev->inuse_qpid, dev->next_srqid, dev->size_qpid);
	if (id != dev->next_srqid)
		goto found;

	id = find_next_zero_bit(dev->inuse_qpid, dev->size_qpid, dev->next_qpid);
	if (id != dev->size_qpid)
		goto found;

	id = find_next_zero_bit(dev->inuse_qpid, dev->next_qpid, 2);
	if (id != dev->next_qpid)
		goto found;

	mutex_unlock(&dev->inuse_lock);

	/* not found */
	return -ENOMEM;

found:
	set_bit(id, dev->inuse_qpid);

	if (id < dev->size_qpid)
		dev->next_qpid = id + 1;
	else
		dev->next_srqid = id + 1;

	mutex_unlock(&dev->inuse_lock);

	*qpid = id;

	return 0;
}

static void ionic_put_pdid(struct ionic_ibdev *dev, u32 pdid)
{
	clear_bit(pdid, dev->inuse_pdid);
}

static void ionic_put_mrid(struct ionic_ibdev *dev, u32 mrid)
{
	clear_bit(mrid / 2, dev->inuse_mrid); /* XXX see get_mrid */
}

static void ionic_put_pgtbl(struct ionic_ibdev *dev, u32 pos, int order)
{
	mutex_lock(&dev->inuse_lock);
	bitmap_release_region(dev->inuse_pgtbl, pos, order);
	mutex_unlock(&dev->inuse_lock);
}

static void ionic_put_cqid(struct ionic_ibdev *dev, u32 cqid)
{
	clear_bit(cqid, dev->inuse_cqid);
}

static void ionic_put_qpid(struct ionic_ibdev *dev, u32 qpid)
{
	clear_bit(qpid, dev->inuse_qpid);
}

static void ionic_put_srqid(struct ionic_ibdev *dev, u32 qpid)
{
	clear_bit(qpid, dev->inuse_qpid);
}

static int ionic_pgtbl_init(struct ionic_ibdev *dev, struct ib_umem *umem,
			    u32 *pos, int *order)
{
	int rc;

	rc = order_base_2(ib_umem_num_pages(umem));
	*order = rc;

	if (rc)
		rc = ionic_get_pgtbl(dev, pos, rc);
	else
		*pos = 0;

	return rc;
}

static struct rdma_hw_stats *ionic_alloc_hw_stats(struct ib_device *ibdev,
						  u8 port)
{
	if (port != 1)
		return NULL;

	return NULL;
}

static int ionic_get_hw_stats(struct ib_device *ibdev,
			      struct rdma_hw_stats *stats,
			      u8 port, int index)
{
	if (port != 1)
		return -EINVAL;

	return -ENOSYS;
}

static int ionic_query_device(struct ib_device *ibdev,
			      struct ib_device_attr *attr,
			      struct ib_udata *udata)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibdev);

	*attr = dev->dev_attr;

	return 0;
}

static int ionic_query_port(struct ib_device *ibdev, u8 port,
			    struct ib_port_attr *attr)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibdev);
	int rc;

	if (port != 1)
		return -EINVAL;

	*attr = dev->port_attr;

	rc = ib_get_eth_speed(ibdev, port,
			      &attr->active_speed,
			      &attr->active_width);

	return 0;
}

static enum rdma_link_layer ionic_get_link_layer(struct ib_device *ibdev,
						 u8 port)
{
	return IB_LINK_LAYER_ETHERNET;
}

static struct net_device *ionic_get_netdev(struct ib_device *ibdev, u8 port)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibdev);

	if (port != 1)
		return ERR_PTR(-EINVAL);

	dev_hold(dev->ndev);

	return dev->ndev;
}

static int ionic_query_gid(struct ib_device *ibdev, u8 port, int index,
			   union ib_gid *gid)
{
	int rc;

	rc = ib_get_cached_gid(ibdev, port, index, gid, NULL);
	if (rc == -EAGAIN) {
		memcpy(gid, &zgid, sizeof(*gid));
		return 0;
	}

	return rc;
}

static int ionic_add_gid(struct ib_device *ibdev, u8 port, unsigned int index,
			 const union ib_gid *gid,
			 const struct ib_gid_attr *attr, void **context)
{
	enum rdma_network_type net;

	/* XXX const cast */
	net = ib_gid_to_network_type(attr->gid_type, (union ib_gid *)gid);
	if (net != RDMA_NETWORK_IPV4 && net != RDMA_NETWORK_IPV6)
		return -EINVAL;

	return 0;
}

static int ionic_del_gid(struct ib_device *ibdev, u8 port, unsigned int index,
			 void **context)
{
	return 0;
}

static int ionic_query_pkey(struct ib_device *ibdev, u8 port, u16 index,
			    u16 *pkey)
{
	if (port != 1)
		return -EINVAL;

	if (index != 0)
		return -EINVAL;

	*pkey = 0xffff;

	return 0;
}

static int ionic_modify_device(struct ib_device *ibdev, int mask,
			       struct ib_device_modify *attr)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibdev);

	if (mask & ~IB_DEVICE_MODIFY_NODE_DESC)
		return -EOPNOTSUPP;

	if (mask & IB_DEVICE_MODIFY_NODE_DESC)
		memcpy(dev->ibdev.node_desc, attr->node_desc,
		       IB_DEVICE_NODE_DESC_MAX);

	return 0;
}

static int ionic_modify_port(struct ib_device *ibdev, u8 port, int mask,
			     struct ib_port_modify *attr)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibdev);

	if (port != 1)
		return -EINVAL;

	dev->port_attr.port_cap_flags |= attr->set_port_cap_mask;
	dev->port_attr.port_cap_flags &= ~attr->clr_port_cap_mask;

	if (mask & IB_PORT_RESET_QKEY_CNTR)
		dev->port_attr.qkey_viol_cntr = 0;

	return 0;
}

static struct ib_ucontext *ionic_alloc_ucontext(struct ib_device *ibdev,
						struct ib_udata *udata)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibdev);
	struct ionic_ctx *ctx;
	struct ionic_ctx_req req;
	struct ionic_ctx_resp resp = {};
	int rc;

	rc = ionic_validate_udata(udata, sizeof(req), sizeof(resp));
	if (!rc)
		rc = ib_copy_from_udata(&req, udata, sizeof(req));

	if (rc)
		goto err_ctx;

	ctx = kmalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx) {
		rc = -ENOMEM;
		goto err_ctx;
	}

	/* maybe force fallback to kernel space */
	ctx->fallback = req.fallback > 1;
	if (!ctx->fallback) {
		/* try to allocate dbid for user ctx */
		if (ionic_xxx_kdbid)
			ctx->dbid = dev->dbid; /* XXX kernel dbid in user space */
		else
			ctx->dbid = ionic_api_get_dbid(dev->lif);
		if (ctx->dbid < 0) {
			rc = ctx->dbid;
			/* maybe allow fallback to kernel space */
			ctx->fallback = req.fallback > 0;
			if (!ctx->fallback)
				goto err_dbid;
			ctx->dbid = ~0u;
		}
	}
	if (ctx->fallback)
		dev_dbg(&dev->ibdev.dev, "fallback kernel space\n");
	else
		dev_dbg(&dev->ibdev.dev, "user space dbid %u\n", ctx->dbid);

	mutex_init(&ctx->mmap_mut);
	ctx->mmap_off = PAGE_SIZE;
	INIT_LIST_HEAD(&ctx->mmap_list);

	ctx->mmap_dbell.offset = 0;
	ctx->mmap_dbell.size = PAGE_SIZE;
	ctx->mmap_dbell.pfn = PHYS_PFN(dev->phys_dbpage_base +
				       dev->dbid * PAGE_SIZE);
	list_add(&ctx->mmap_dbell.ctx_ent, &ctx->mmap_list);

	resp.version = IONIC_ABI_VERSION; /* XXX fw abi version */
	resp.fallback = ctx->fallback;
	resp.dbell_offset = 0;
	resp.sq_qtype = dev->sq_qtype;
	resp.rq_qtype = dev->rq_qtype;
	resp.cq_qtype = dev->cq_qtype;

	rc = ib_copy_to_udata(udata, &resp, sizeof(resp));
	if (rc)
		goto err_resp;

	return &ctx->ibctx;

err_resp:
	if (!ionic_xxx_kdbid)
		ionic_api_put_dbid(dev->lif, ctx->dbid);
err_dbid:
	kfree(ctx);
err_ctx:
	return ERR_PTR(rc);
}

static int ionic_dealloc_ucontext(struct ib_ucontext *ibctx)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibctx->device);
	struct ionic_ctx *ctx = to_ionic_ctx(ibctx);

	list_del(&ctx->mmap_dbell.ctx_ent);

	if (WARN_ON(!list_empty(&ctx->mmap_list)))
		list_del(&ctx->mmap_list);

	if (!ionic_xxx_kdbid)
		ionic_api_put_dbid(dev->lif, ctx->dbid);
	kfree(ctx);

	return 0;
}

static int ionic_mmap(struct ib_ucontext *ibctx, struct vm_area_struct *vma)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibctx->device);
	struct ionic_ctx *ctx = to_ionic_ctx(ibctx);
	struct ionic_mmap_info *info;
	unsigned long size = vma->vm_end - vma->vm_start;
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
	int rc = 0;

	if ((vma->vm_flags & (VM_READ | VM_EXEC)) ||
	    (~vma->vm_flags & (VM_WRITE | VM_SHARED))) {
		dev_dbg(&dev->ibdev.dev,
			"invalid pgprot rd %d (0) wr %d (1) ex %d (0) sh %d (1)\n",
			!!(vma->vm_flags & VM_READ), !!(vma->vm_flags & VM_WRITE),
			!!(vma->vm_flags & VM_EXEC), !!(vma->vm_flags & VM_SHARED));
		rc = -EACCES;
		goto out;
	}

	mutex_lock(&ctx->mmap_mut);

	list_for_each_entry(info, &ctx->mmap_list, ctx_ent)
		if (info->offset == offset)
			goto found;

	mutex_unlock(&ctx->mmap_mut);

	/* not found */
	dev_dbg(&dev->ibdev.dev, "not found %#lx\n", offset);
	rc = -EINVAL;
	goto out;

found:
	list_del_init(&info->ctx_ent);
	mutex_unlock(&ctx->mmap_mut);

	if (info->size != size) {
		dev_dbg(&dev->ibdev.dev, "invalid size %#lx (%#lx)\n",
			size, info->size);
		rc = -EINVAL;
		goto out;
	}

	dev_dbg(&dev->ibdev.dev, "remap st %#lx pf %#lx sz %#lx\n",
		vma->vm_start, info->pfn, size);
	rc = io_remap_pfn_range(vma, vma->vm_start, info->pfn, size,
				vma->vm_page_prot);
	if (rc)
		dev_dbg(&dev->ibdev.dev, "remap failed %d\n", rc);

out:
	return rc;
}

static struct ib_pd *ionic_alloc_pd(struct ib_device *ibdev,
				    struct ib_ucontext *ibctx,
				    struct ib_udata *udata)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibdev);
	struct ionic_pd *pd;
	int rc;

	pd = kmalloc(sizeof(*pd), GFP_KERNEL);
	if (!pd) {
		rc = -ENOMEM;
		goto err_pd;
	}

	rc = ionic_get_pdid(dev, &pd->pdid);
	if (rc)
		goto err_pdid;

	return &pd->ibpd;

err_pdid:
	kfree(pd);
err_pd:
	return ERR_PTR(rc);
}

static int ionic_dealloc_pd(struct ib_pd *ibpd)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibpd->device);
	struct ionic_pd *pd = to_ionic_pd(ibpd);

	ionic_put_pdid(dev, pd->pdid);
	kfree(pd);

	return 0;
}

static int ionic_build_hdr(struct ionic_ibdev *dev,
			   struct ib_ud_header *hdr,
			   const struct rdma_ah_attr *attr)
{
	const struct ib_global_route *grh;
	struct ib_gid_attr sgid_attr;
	union ib_gid sgid;
	u8 smac[ETH_ALEN];
	u16 vlan;
	enum rdma_network_type net;
	int rc;

	if (attr->ah_flags != IB_AH_GRH)
		return -EINVAL;

	if (attr->type != RDMA_AH_ATTR_TYPE_ROCE)
		return -EINVAL;

	grh = rdma_ah_read_grh(attr);

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

	/* XXX const cast */
	if (net != ib_gid_to_network_type(sgid_attr.gid_type,
					  (union ib_gid *)&grh->dgid))
		return -EINVAL;

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

	ether_addr_copy(hdr->eth.smac_h, smac);
	ether_addr_copy(hdr->eth.dmac_h, attr->roce.dmac);

	if (net == RDMA_NETWORK_IPV4) {
		hdr->eth.type = cpu_to_be16(ETH_P_IP);
		hdr->ip4.tos = grh->traffic_class;
		hdr->ip4.frag_off = cpu_to_be16(0x4000); /* don't fragment */
		hdr->ip4.ttl = grh->hop_limit;
		hdr->ip4.saddr = *(__be32 *)(sgid.raw + 12);
		hdr->ip4.daddr = *(__be32 *)(grh->dgid.raw + 12);
	} else {
		hdr->eth.type = cpu_to_be16(ETH_P_IPV6);
		hdr->grh.traffic_class = grh->traffic_class;
		hdr->grh.flow_label = cpu_to_be32(grh->flow_label);
		hdr->grh.hop_limit = grh->hop_limit;
		hdr->grh.source_gid = sgid;
		hdr->grh.destination_gid = grh->dgid;
	}

	if (vlan != 0xffff) {
		hdr->vlan.tag = cpu_to_be16(vlan);
		hdr->vlan.type = hdr->eth.type;
		hdr->eth.type = cpu_to_be16(ETH_P_8021Q);
	}

	hdr->udp.sport = cpu_to_be16(49152); /* XXX hardcode val */
	hdr->udp.dport = cpu_to_be16(ROCE_V2_UDP_DPORT);

	return 0;
}

static int ionic_create_ah_cmd(struct ionic_ibdev *dev,
			       struct ionic_ah *ah,
			       struct ionic_pd *pd,
			       struct rdma_ah_attr *attr)
{
	struct ionic_admin_ctx admin = {
		.work = COMPLETION_INITIALIZER_ONSTACK(admin.work),
		/* XXX endian? */
		.cmd.create_ah = {
			.opcode = CMD_OPCODE_RDMA_CREATE_AH,
			.pd_id = pd->pdid,
		},
	};
	struct ib_ud_header *hdr;
	//void *hdr_buf;
	struct rdma_create_ah_data *hdr_buf;
	dma_addr_t hdr_dma = 0;
	int rc, hdr_len = 0;

	hdr = kmalloc(sizeof(*hdr), GFP_KERNEL);
	if (!hdr) {
		rc = -ENOMEM;
		goto err_hdr;
	}

	rc = ionic_build_hdr(dev, hdr, attr);
	if (rc)
		goto err_buf;

	/* XXX create ah should take header template */
	//hdr_buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	hdr_buf = kmalloc(sizeof(*hdr_buf), GFP_KERNEL);
	if (!hdr_buf) {
		rc = -ENOMEM;
		goto err_buf;
	}

	/* XXX create ah should take header template */
	//hdr_len = ib_ud_header_pack(hdr, hdr_buf);
	//hdr_len -= IB_BTH_BYTES;
	//hdr_len -= IB_DETH_BYTES;
	//
	//dev_dbg(&dev->ibdev.dev, "roce packet header template\n");
	//print_hex_dump_debug("hdr ", DUMP_PREFIX_OFFSET, 16, 1,
	//		     hdr_buf, hdr_len, true);

	/* XXX create ah should take header template (delete code below) */

	ether_addr_copy(hdr_buf->smac, hdr->eth.smac_h);
	ether_addr_copy(hdr_buf->dmac, hdr->eth.dmac_h);

	if (hdr->vlan_present) {
		hdr_buf->vlan = 0xffff;
		hdr_buf->vlan_cfi = 0;
		hdr_buf->vlan_pri = 0;
	} else {
		hdr_buf->vlan = hdr->vlan.tag & 0xfff;
		hdr_buf->vlan_cfi = (hdr->vlan.tag >> 12) & 1;
		hdr_buf->vlan_pri = (hdr->vlan.tag >> 13) & 7;
	}

	if (hdr->ipv4_present) {
		hdr_buf->ip_ver = 4;
		hdr_buf->ip_tos = hdr->ip4.tos;
		hdr_buf->ip_ttl = hdr->ip4.ttl;
		hdr_buf->ip.v4.saddr = hdr->ip4.saddr;
		hdr_buf->ip.v4.daddr = hdr->ip4.daddr;
	} else {
		hdr_buf->ip_ver = 6;
		hdr_buf->ip_tos = hdr->grh.traffic_class;
		hdr_buf->ip_ttl = hdr->grh.hop_limit;
		memcpy(hdr_buf->ip.v6.saddr, &hdr->grh.source_gid, 16);
		memcpy(hdr_buf->ip.v6.daddr, &hdr->grh.destination_gid, 16);
	}

	hdr_len = sizeof(*hdr_buf);

	/* XXX create ah should take header template (delete code above) */

	hdr_dma = ib_dma_map_single(&dev->ibdev, hdr_buf, hdr_len,
				    DMA_TO_DEVICE);

	rc = ib_dma_mapping_error(&dev->ibdev, hdr_dma);
	if (rc)
		goto err_dma;

	admin.cmd.create_ah.hdr_info = hdr_dma;

	/* XXX for HAPS: side-data */
	if (ionic_xxx_haps) {
#ifndef ADMINQ
		admin.side_data = hdr_buf;
		admin.side_data_len = round_up(hdr_len, sizeof(u32));
#endif
	}

	rc = ionic_api_adminq_post(dev->lif, &admin);
	if (rc)
		goto err_cmd;

	wait_for_completion(&admin.work);

	if (0 /* TODO: admin queue failure */) {
		rc = -EIO;
		goto err_cmd;
	}

	rc = ionic_verbs_status_to_rc(admin.comp.create_qp.status);
	if (rc)
		goto err_cmd;

	/* XXX driver should alloc ah id */
	ah->ahid = (u32)admin.comp.create_ah.handle;
	ah->len = admin.comp.create_ah.len;

	/* handle is 32 bits, len is 8 bits, in send wqe */
	WARN_ON(admin.comp.create_ah.handle >> 32);
	WARN_ON(admin.comp.create_ah.len >> 8);

err_cmd:
	ib_dma_unmap_single(&dev->ibdev, hdr_dma, hdr_len,
			    DMA_TO_DEVICE);
err_dma:
	kfree(hdr_buf);
err_buf:
	kfree(hdr);
err_hdr:
	return rc;
}

static struct ib_ah *ionic_create_ah(struct ib_pd *ibpd,
				     struct rdma_ah_attr *attr,
				     struct ib_udata *udata)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibpd->device);
	struct ionic_ctx *ctx = to_ionic_ctx_uobj(ibpd->uobject);
	struct ionic_pd *pd = to_ionic_pd(ibpd);
	struct ionic_ah *ah;
	struct ionic_ah_resp resp = {};
	int rc;

	if (ctx)
		rc = ionic_validate_udata(udata, 0, sizeof(resp));
	else
		rc = ionic_validate_udata(udata, 0, 0);
	if (rc)
		goto err_ah;

	ah = kmalloc(sizeof(*ah), GFP_KERNEL);
	if (!ah) {
		rc = -ENOMEM;
		goto err_ah;
	}

	/* XXX driver should alloc ah id */
	//rc = ionic_get_ahid(dev, &ah->ahid);
	//if (rc)
	//	goto err_ahid;

	rc = ionic_create_ah_cmd(dev, ah, pd, attr);
	if (rc)
		goto err_cmd;

	if (ctx) {
		resp.ahid = ah->ahid;
		resp.len = ah->len;

		rc = ib_copy_to_udata(udata, &resp, sizeof(resp));
		if (rc)
			goto err_resp;
	}

	return &ah->ibah;

err_resp:
	/* XXX destroy ah */
err_cmd:
	kfree(ah);
err_ah:
	return ERR_PTR(rc);
}

static int ionic_modify_ah(struct ib_ah *ibah, struct rdma_ah_attr *attr)
{
	return -ENOSYS;
}

static int ionic_query_ah(struct ib_ah *ibah, struct rdma_ah_attr *attr)
{
	return -ENOSYS;
}

static int ionic_destroy_ah(struct ib_ah *ibah)
{
	return -ENOSYS;
}

static int ionic_kernel_mr_cmd(struct ionic_ibdev *dev, struct ionic_pd *pd,
			       u32 lkey, u32 rkey, u64 start, u64 length,
			       u32 page_size, dma_addr_t dma, int access)
{
	struct ionic_admin_ctx admin = {
		.work = COMPLETION_INITIALIZER_ONSTACK(admin.work),
		/* XXX endian? */
		.cmd.create_mr = {
			.opcode = CMD_OPCODE_RDMA_CREATE_MR,
			.pd_num = pd->pdid,
			/* XXX lif should be dbid */
			.lif = dev->lif_id,
			.access_flags = access,
			.start = start,
			.length = length,
			/* XXX .iova = addr, */
			/* XXX can lkey,rkey be just rkey? */
			.lkey = lkey,
			.rkey = lkey,
			/* XXX can page_size be just page shift? */
			.page_size = page_size,
		},
	};
	size_t pagedir_size;
	u64 *pagedir;
	dma_addr_t pagedma;
	int rc, pg_i, npages;

	npages = DIV_ROUND_UP_ULL(length, page_size);

	pagedir_size = npages * sizeof(*pagedir);
	pagedir = kmalloc(pagedir_size, GFP_KERNEL);
	if (!pagedir) {
		rc = -ENOMEM;
		goto err_pagedir;
	}

	pagedma = dma;
	for (pg_i = 0; pg_i < npages; ++pg_i) {
		pagedir[pg_i] = BIT_ULL(63) | pagedma;
		pagedma += page_size;
	}

	WARN_ON(pg_i != npages);

	pagedma = ib_dma_map_single(&dev->ibdev, pagedir, pagedir_size,
				    DMA_TO_DEVICE);
	rc = ib_dma_mapping_error(&dev->ibdev, pagedma);
	if (rc)
		goto err_pagedma;

	/* XXX endian? */
	admin.cmd.create_mr.nchunks = npages;
	admin.cmd.create_mr.pt_dma = pagedma;

	/* XXX for HAPS: side-data */
	if (ionic_xxx_haps) {
#ifndef ADMINQ
		admin.side_data = pagedir;
		admin.side_data_len = pagedir_size;
#endif
	}

	rc = ionic_api_adminq_post(dev->lif, &admin);
	if (rc)
		goto err_cmd;

	wait_for_completion(&admin.work);

	if (0 /* XXX did the queue fail? */) {
		rc = -EIO;
		goto err_cmd;
	}

	rc = ionic_verbs_status_to_rc(admin.comp.create_mr.status);

err_cmd:
	ib_dma_unmap_single(&dev->ibdev, pagedma, pagedir_size,
			    DMA_TO_DEVICE);
err_pagedma:
	kfree(pagedir);
err_pagedir:
	return rc;
}

static int ionic_kernel_umr_cmd(struct ionic_ibdev *dev, struct ionic_pd *pd,
				u32 lkey, u32 rkey, u64 start, u64 length,
				struct ib_umem *umem, int access)
{
	struct ionic_admin_ctx admin = {
		.work = COMPLETION_INITIALIZER_ONSTACK(admin.work),
		/* XXX endian? */
		.cmd.create_mr = {
			.opcode = CMD_OPCODE_RDMA_CREATE_MR,
			.pd_num = pd->pdid,
			/* XXX lif should be dbid */
			.lif = dev->lif_id,
			.access_flags = access,
			.start = start,
			.length = length,
			/* XXX .iova = addr, */
			/* XXX can lkey,rkey be just rkey? */
			.lkey = lkey,
			.rkey = rkey,
			/* XXX can page_size be just page shift? */
			.page_size = BIT_ULL(umem->page_shift),
		},
	};
	size_t pagedir_size;
	struct scatterlist *sg;
	u64 *pagedir;
	dma_addr_t pagedma;
	int rc, sg_i, pg_i, pg_end, npages;

	npages = ib_umem_page_count(umem);

	pagedir_size = npages * sizeof(*pagedir);
	pagedir = kmalloc(pagedir_size, GFP_KERNEL);
	if (!pagedir) {
		rc = -ENOMEM;
		goto err_pagedir;
	}

	pg_i = 0;
	pg_end = 0;
	for_each_sg(umem->sg_head.sgl, sg, umem->nmap, sg_i) {
		pagedma = sg_dma_address(sg);
		pg_end += sg_dma_len(sg) >> umem->page_shift;
		for (; pg_i < pg_end; ++pg_i) {
			/* XXX endian? */
			pagedir[pg_i] = BIT_ULL(63) | pagedma;
			pagedma += BIT_ULL(umem->page_shift);
		}
	}

	WARN_ON(pg_i != npages);

	pagedma = ib_dma_map_single(&dev->ibdev, pagedir, pagedir_size,
				    DMA_TO_DEVICE);
	rc = ib_dma_mapping_error(&dev->ibdev, pagedma);
	if (rc)
		goto err_pagedma;

	/* XXX endian? */
	admin.cmd.create_mr.nchunks = npages;
	admin.cmd.create_mr.pt_dma = pagedma;

	/* XXX for HAPS: side-data */
	if (ionic_xxx_haps) {
#ifndef ADMINQ
		admin.side_data = pagedir;
		admin.side_data_len = pagedir_size;
#endif
	}

	rc = ionic_api_adminq_post(dev->lif, &admin);
	if (rc)
		goto err_cmd;

	wait_for_completion(&admin.work);

	if (0 /* XXX did the queue fail? */) {
		rc = -EIO;
		goto err_cmd;
	}

	rc = ionic_verbs_status_to_rc(admin.comp.create_mr.status);

err_cmd:
	ib_dma_unmap_single(&dev->ibdev, pagedma, pagedir_size,
			    DMA_TO_DEVICE);
err_pagedma:
	kfree(pagedir);
err_pagedir:
	return rc;
}

static int ionic_create_mr_cmd(struct ionic_ibdev *dev, struct ionic_pd *pd,
			       struct ionic_mr *mr, u64 start, u64 length,
			       u64 addr, int access)
{
	struct ionic_admin_ctx admin = {
		.work = COMPLETION_INITIALIZER_ONSTACK(admin.work),
		/* XXX endian? */
		.cmd.create_mr = {
			.opcode = CMD_OPCODE_RDMA_CREATE_MR,
			.pd_num = pd->pdid,
			/* XXX lif should be dbid */
			.lif = dev->lif_id,
			.access_flags = access,
			.start = start,
			.length = length,
			/* XXX .iova = addr, */
			/* XXX can lkey,rkey be just rkey? */
			.lkey = mr->ibmr.lkey,
			.rkey = mr->ibmr.rkey,
			/* XXX can page_size be just page shift? */
			.page_size = BIT_ULL(mr->umem->page_shift),
		},
	};
	size_t pagedir_size;
	struct scatterlist *sg;
	u64 *pagedir;
	dma_addr_t pagedma;
	int rc, sg_i, pg_i, pg_end, npages;

	if (mr->umem) {
		npages = ib_umem_page_count(mr->umem);
	} else {
		rc = -ENOSYS;
		goto err_pagedir;
	}

	pagedir_size = npages * sizeof(*pagedir);
	pagedir = kmalloc(pagedir_size, GFP_KERNEL);
	if (!pagedir) {
		rc = -ENOMEM;
		goto err_pagedir;
	}

	if (mr->umem) {
		pg_i = 0;
		pg_end = 0;
		for_each_sg(mr->umem->sg_head.sgl, sg, mr->umem->nmap, sg_i) {
			pagedma = sg_dma_address(sg);
			pg_end += sg_dma_len(sg) >> mr->umem->page_shift;
			for (; pg_i < pg_end; ++pg_i) {
				/* XXX endian? */
				pagedir[pg_i] = BIT_ULL(63) | pagedma;
				pagedma += BIT_ULL(mr->umem->page_shift);
			}
		}
	} else {
		rc = -ENOSYS;
		goto err_pagedma;
	}

	WARN_ON(pg_i != npages);

	pagedma = ib_dma_map_single(&dev->ibdev, pagedir, pagedir_size,
				    DMA_TO_DEVICE);
	rc = ib_dma_mapping_error(&dev->ibdev, pagedma);
	if (rc)
		goto err_pagedma;

	/* XXX endian? */
	admin.cmd.create_mr.nchunks = npages;
	admin.cmd.create_mr.pt_dma = pagedma;

	/* XXX for HAPS: side-data */
	if (ionic_xxx_haps) {
#ifndef ADMINQ
		admin.side_data = pagedir;
		admin.side_data_len = pagedir_size;
#endif
	}

	rc = ionic_api_adminq_post(dev->lif, &admin);
	if (rc)
		goto err_cmd;

	wait_for_completion(&admin.work);

	if (0 /* XXX did the queue fail? */) {
		rc = -EIO;
		goto err_cmd;
	}

	rc = ionic_verbs_status_to_rc(admin.comp.create_mr.status);

err_cmd:
	ib_dma_unmap_single(&dev->ibdev, pagedma, pagedir_size,
			    DMA_TO_DEVICE);
err_pagedma:
	kfree(pagedir);
err_pagedir:
	return rc;
}

static int ionic_destroy_mr_cmd(struct ionic_ibdev *dev,
				struct ionic_mr *mr)
{
#if 1
	/* need destroy_mr admin command */
	return 0;
#else
	struct ionic_admin_ctx admin = {
		.work = COMPLETION_INITIALIZER_ONSTACK(admin.work),
		/* XXX endian? */
		.cmd.destroy_mr = {
			.opcode = CMD_OPCODE_RDMA_DESTROY_MR,
			.lkey = mr->ibmr.lkey,
			.rkey = mr->ibmr.rkey,
		},
	};
	int rc;

	rc = ionic_api_adminq_post(dev->lif, &admin);
	if (rc)
		goto err_cmd;

	wait_for_completion(&admin.work);

	if (0 /* XXX did the queue fail? */) {
		rc = -EIO;
		goto err_cmd;
	}

	rc = ionic_verbs_status_to_rc(admin.comp.destroy_mr.status);

err_cmd:
	return rc;
#endif
}

static struct ib_mr *ionic_get_dma_mr(struct ib_pd *ibpd, int access)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibpd->device);
	struct ionic_pd *pd = to_ionic_pd(ibpd);
	struct ionic_mr *mr;
	int rc;

	mr = kmalloc(sizeof(*mr), GFP_KERNEL);
	if (!mr) {
		rc = -ENOMEM;
		goto err_mr;
	}

	rc = ionic_get_mrid(dev, &mr->ibmr.lkey, &mr->ibmr.rkey);
	if (rc)
		goto err_mrid;

	mr->umem = NULL;

	mr->tbl_order = 0;
	mr->tbl_pos = 0;

	rc = ionic_kernel_mr_cmd(dev, pd, mr->ibmr.lkey, mr->ibmr.rkey,
				 0, BIT(31), BIT(31)/8, 0, access);
	if (rc)
		goto err_cmd;

	ionic_dbgfs_add_mr(dev, mr);

	return &mr->ibmr;

err_cmd:
	ionic_put_mrid(dev, mr->ibmr.lkey);
err_mrid:
	kfree(mr);
err_mr:
	return ERR_PTR(rc);
}

static struct ib_mr *ionic_reg_user_mr(struct ib_pd *ibpd, u64 start,
				       u64 length, u64 addr, int access,
				       struct ib_udata *udata)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibpd->device);
	struct ionic_pd *pd = to_ionic_pd(ibpd);
	struct ionic_mr *mr;
	int rc;

	rc = ionic_validate_udata(udata, 0, 0);
	if (rc)
		goto err_mr;

	mr = kmalloc(sizeof(*mr), GFP_KERNEL);
	if (!mr) {
		rc = -ENOMEM;
		goto err_mr;
	}

	rc = ionic_get_mrid(dev, &mr->ibmr.lkey, &mr->ibmr.rkey);
	if (rc)
		goto err_mrid;

	mr->umem = ib_umem_get(ibpd->uobject->context, start, length, access, 0);
	if (IS_ERR(mr->umem)) {
		rc = PTR_ERR(mr->umem);
		goto err_umem;
	}

	rc = ionic_pgtbl_init(dev, mr->umem, &mr->tbl_pos, &mr->tbl_order);
	if (rc)
		goto err_pgtbl;

	rc = ionic_create_mr_cmd(dev, pd, mr, start, length, addr, access);
	if (rc)
		goto err_cmd;

	ionic_dbgfs_add_mr(dev, mr);

	return &mr->ibmr;

err_cmd:
	ionic_put_pgtbl(dev, mr->tbl_pos, mr->tbl_order);
err_pgtbl:
	ib_umem_release(mr->umem);
err_umem:
	ionic_put_mrid(dev, mr->ibmr.lkey);
err_mrid:
	kfree(mr);
err_mr:
	return ERR_PTR(rc);
}

static int ionic_rereg_user_mr(struct ib_mr *ibmr, int flags, u64 start,
			       u64 length, u64 virt_addr, int access,
			       struct ib_pd *pd, struct ib_udata *udata)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibmr->device);
	struct ionic_mr *mr = to_ionic_mr(ibmr);

	if (0 /* TODO */)
		ionic_replace_rkey(dev, &mr->ibmr.rkey);

	return -ENOSYS;
}

static int ionic_dereg_mr(struct ib_mr *ibmr)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibmr->device);
	struct ionic_mr *mr = to_ionic_mr(ibmr);
	int rc;

	rc = ionic_destroy_mr_cmd(dev, mr);
	if (rc)
		return rc;

	ionic_dbgfs_rm_mr(mr);

	ionic_put_pgtbl(dev, mr->tbl_pos, mr->tbl_order);

	if (mr->umem)
		ib_umem_release(mr->umem);

	ionic_put_mrid(dev, mr->ibmr.lkey);

	return 0;
}

static struct ib_mr *ionic_alloc_mr(struct ib_pd *ibpd,
				    enum ib_mr_type type,
				    u32 max_sg)
{
	return ERR_PTR(-ENOSYS);
}

static int ionic_map_mr_sg(struct ib_mr *ibmr, struct scatterlist *sg,
			   int sg_nents, unsigned int *sg_offset)
{
	return -ENOSYS;
}

static struct ib_mw *ionic_alloc_mw(struct ib_pd *ibpd, enum ib_mw_type type,
				    struct ib_udata *udata)
{
	return ERR_PTR(-ENOSYS);
}

static int ionic_dealloc_mw(struct ib_mw *ibmw)
{
	return -ENOSYS;
}

static int ionic_create_cq_cmd(struct ionic_ibdev *dev, struct ionic_cq *cq,
			       u32 intr)
{
	struct ionic_admin_ctx admin = {
		.work = COMPLETION_INITIALIZER_ONSTACK(admin.work),
		/* XXX endian? */
		.cmd.create_cq = {
			.opcode = CMD_OPCODE_RDMA_CREATE_CQ,
			/* XXX lif should be dbid */
			.lif_id = dev->lif_id,
			.cq_num = cq->cqid,
			/* XXX may overflow, zero means 2^16 */
			.num_cq_wqes = cq->q.mask + 1,
			.host_pg_size = PAGE_SIZE,
			.va_len = cq->q.size,
			.cq_wqe_size = cq->q.stride,
			.eq_id = intr,
			/* XXX cleanup */
			.cq_lkey = cq->lkey,
		},
	};
	size_t pagedir_size;
	struct scatterlist *sg;
	u64 *pagedir;
	dma_addr_t pagedma;
	int rc, sg_i, pg_i, pg_end, npages;

	if (cq->umem) {
		npages = ib_umem_page_count(cq->umem);
	} else {
		npages = DIV_ROUND_UP_ULL(cq->q.size, PAGE_SIZE);
	}

	pagedir_size = npages * sizeof(*pagedir);
	pagedir = kmalloc(pagedir_size, GFP_KERNEL);
	if (!pagedir) {
		rc = -ENOMEM;
		goto err_pagedir;
	}

	if (cq->umem) {
		pg_i = 0;
		pg_end = 0;
		for_each_sg(cq->umem->sg_head.sgl, sg, cq->umem->nmap, sg_i) {
			pagedma = sg_dma_address(sg);
			pg_end += sg_dma_len(sg) >> PAGE_SHIFT;
			for (; pg_i < pg_end; ++pg_i) {
				/* XXX endian? */
				pagedir[pg_i] = BIT_ULL(63) | pagedma;
				pagedma += PAGE_SIZE;
			}
		}
	} else {
		pagedma = cq->q.dma;
		for (pg_i = 0; pg_i < npages; ++pg_i) {
			pagedir[pg_i] = BIT_ULL(63) | pagedma;
			pagedma += PAGE_SIZE;
		}
	}

	WARN_ON(pg_i != npages);

	pagedma = ib_dma_map_single(&dev->ibdev, pagedir, pagedir_size,
				    DMA_TO_DEVICE);
	rc = ib_dma_mapping_error(&dev->ibdev, pagedma);
	if (rc)
		goto err_pagedma;

	/* XXX endian? */
	admin.cmd.create_cq.pt_size = npages;
	admin.cmd.create_cq.pt_base_addr = pagedma;

	/* XXX for HAPS: side-data */
	if (ionic_xxx_haps) {
#ifndef ADMINQ
		admin.side_data = pagedir;
		admin.side_data_len = pagedir_size;
#endif
	}

	rc = ionic_api_adminq_post(dev->lif, &admin);
	if (rc)
		goto err_cmd;

	wait_for_completion(&admin.work);

	if (0 /* XXX did the queue fail? */) {
		rc = -EIO;
		goto err_cmd;
	}

	rc = ionic_verbs_status_to_rc(admin.comp.create_cq.status);

err_cmd:
	ib_dma_unmap_single(&dev->ibdev, pagedma, pagedir_size,
			    DMA_TO_DEVICE);
err_pagedma:
	kfree(pagedir);
err_pagedir:
	return rc;
}

static int ionic_destroy_cq_cmd(struct ionic_ibdev *dev,
				struct ionic_cq *cq)
{
#if 1
	/* need destroy_cq admin command */
	return 0;
#else
	struct ionic_admin_ctx admin = {
		.work = COMPLETION_INITIALIZER_ONSTACK(admin.work),
		/* XXX endian? */
		.cmd.destroy_cq = {
			.opcode = CMD_OPCODE_RDMA_DESTROY_CQ,
			.cq_num = cq->cqid,
		},
	};
	int rc;

	rc = ionic_api_adminq_post(dev->lif, &admin);
	if (rc)
		goto err_cmd;

	wait_for_completion(&admin.work);

	if (0 /* XXX did the queue fail? */) {
		rc = -EIO;
		goto err_cmd;
	}

	rc = ionic_verbs_status_to_rc(admin.comp.destroy_cq.status);

err_cmd:
	return rc;
#endif
}

static void ionic_cq_fake_notify(struct work_struct *ws)
{
	struct ionic_cq *cq = container_of(ws, struct ionic_cq,
					   notify_work.work);
	cq->ibcq.comp_handler(&cq->ibcq, cq->ibcq.cq_context);
}

static struct ib_cq *ionic_create_cq(struct ib_device *ibdev,
				     const struct ib_cq_init_attr *attr,
				     struct ib_ucontext *ibctx,
				     struct ib_udata *udata)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibdev);
	struct ionic_ctx *ctx = to_ionic_ctx_fb(ibctx);
	struct ionic_cq *cq;
	struct ionic_cq_req req;
	struct ionic_cq_resp resp;
	u32 rkey; /* XXX cleanup */
	int rc, intr;

	if (!ctx) {
		rc = ionic_validate_udata(udata, 0, 0);
	} else {
		rc = ionic_validate_udata(udata, sizeof(req), sizeof(resp));
		if (!rc)
			rc = ib_copy_from_udata(&req, udata, sizeof(req));
	}

	if (rc)
		goto err_cq;

	intr = attr->comp_vector;
	if (intr >= dev->eq_count)
		intr = 0;
	if (dev->eq_count)
		intr = dev->eq_vec[intr]->intr;

	cq = kmalloc(sizeof(*cq), GFP_KERNEL);
	if (!cq) {
		rc = -ENOMEM;
		goto err_cq;
	}

	INIT_LIST_HEAD(&cq->qp_poll);

	INIT_DELAYED_WORK(&cq->notify_work, ionic_cq_fake_notify);

	rc = ionic_get_cqid(dev, &cq->cqid);
	if (rc)
		goto err_cqid;

	rc = ionic_get_mrid(dev, &cq->lkey, &rkey);
	if (rc)
		goto err_mrid;

	if (!ctx) {
		cq->umem = NULL;
		cq->tbl_order = 0;
		cq->tbl_pos = 0;

		rc = ionic_queue_init(&cq->q, dev->hwdev, attr->cqe,
				      ionic_cqe_size());
		if (rc)
			goto err_q;

		ionic_queue_dbell_init(&cq->q, cq->cqid);
		ionic_queue_color_init(&cq->q);
	} else {
		rc = ionic_validate_qdesc(&req.cq);
		if (rc)
			goto err_q;

		cq->umem = ib_umem_get(&ctx->ibctx, req.cq.addr, req.cq.size,
				       IB_ACCESS_LOCAL_WRITE, 0);
		if (IS_ERR(cq->umem)) {
			rc = PTR_ERR(cq->umem);
			goto err_q;
		}

		rc = ionic_pgtbl_init(dev, cq->umem, &cq->tbl_pos,
				      &cq->tbl_order);
		if (rc)
			goto err_pgtbl;

		cq->q.ptr = NULL;
		cq->q.size = req.cq.size;
		cq->q.mask = req.cq.mask;
		cq->q.stride = req.cq.stride;
	}

	spin_lock_init(&cq->lock);

	rc = ionic_create_cq_cmd(dev, cq, intr);
	if (rc)
		goto err_cmd;

	if (ctx) {
		resp.cqid = cq->cqid;

		rc = ib_copy_to_udata(udata, &resp, sizeof(resp));
		if (rc)
			goto err_resp;
	}

	ionic_dbgfs_add_cq(dev, cq);

	mutex_lock(&dev->tbl_lock);
	tbl_alloc_node(&dev->cq_tbl);
	tbl_insert(&dev->cq_tbl, cq, cq->cqid);
	mutex_unlock(&dev->tbl_lock);

	return &cq->ibcq;

err_resp:
	ionic_destroy_cq_cmd(dev, cq);
err_cmd:
	if (cq->tbl_order)
		ionic_put_pgtbl(dev, cq->tbl_pos, cq->tbl_order);
err_pgtbl:
	if (cq->umem)
		ib_umem_release(cq->umem);
	else
		ionic_queue_destroy(&cq->q, dev->hwdev);
err_q:
	ionic_put_mrid(dev, cq->lkey);
err_mrid:
	ionic_put_cqid(dev, cq->cqid);
err_cqid:
	kfree(cq);
err_cq:
	return ERR_PTR(rc);
}

static int ionic_destroy_cq(struct ib_cq *ibcq)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibcq->device);
	struct ionic_cq *cq = to_ionic_cq(ibcq);
	int rc;

	rc = ionic_destroy_cq_cmd(dev, cq);
	if (rc)
		return rc;

	mutex_lock(&dev->tbl_lock);
	tbl_free_node(&dev->cq_tbl);
	tbl_delete(&dev->cq_tbl, cq->cqid);
	mutex_unlock(&dev->tbl_lock);

	synchronize_rcu();

	cancel_delayed_work_sync(&cq->notify_work);

	ionic_dbgfs_rm_cq(cq);

	/* XXX cleanup */
	ionic_put_mrid(dev, cq->lkey);

	if (cq->tbl_order)
		ionic_put_pgtbl(dev, cq->tbl_pos, cq->tbl_order);

	if (cq->umem)
		ib_umem_release(cq->umem);
	else
		ionic_queue_destroy(&cq->q, dev->hwdev);

	ionic_put_cqid(dev, cq->cqid);

	kfree(cq);

	return 0;
}

static int ionic_resize_cq(struct ib_cq *ibcq, int cqe,
			   struct ib_udata *udata)
{
	return -ENOSYS;
}

static int ionic_poll_recv(struct ionic_qp *cqe_qp, struct ib_wc *wc,
			   struct cqwqe_be_t *cqe)
{
	struct ionic_qp *qp = NULL;
	struct ionic_rq_meta *meta;

	if (cqe_qp->has_rq) {
		qp = cqe_qp;
	} else {
		if (unlikely(cqe_qp->is_srq))
			return -EIO;

		if (unlikely(!cqe_qp->ibqp.srq))
			return -EIO;

		qp = to_ionic_srq(cqe_qp->ibqp.srq);
	}

	/* there had better be something in the recv queue to complete */
	if (ionic_queue_empty(&qp->rq))
		return -EIO;

	meta = &qp->rq_meta[qp->rq.cons];

	memset(wc, 0, sizeof(*wc));

	wc->status = ionic_to_ib_wc_status(cqe->status);
	wc->vendor_err = cqe->status;

	wc->wr_id = cqe->id.wrid;

	if (cqe_qp->is_srq)
		wc->qp = NULL;
	else
		/* XXX possible use-after-free after rcu read unlock */
		wc->qp = &cqe_qp->ibqp;

	if (wc->status != IB_WC_SUCCESS)
		goto out;

	if (cqe->op_type == OP_TYPE_RDMA_OPER_WITH_IMM) {
		wc->opcode = IB_WC_RECV_RDMA_WITH_IMM;
		wc->wc_flags |= IB_WC_WITH_IMM;
		wc->ex.imm_data = cqe->imm_data; /* be32 in wc */
	} else {
		wc->opcode = IB_WC_RECV;
		if (0 /* TODO: cqe has invalidated rkey */) {
			wc->wc_flags |= IB_WC_WITH_INVALIDATE;
			wc->ex.invalidate_rkey = be32_to_cpu(cqe->imm_data);
		} else if (cqe->color_flags & IMM_DATA_VLD_MASK) {
			wc->wc_flags |= IB_WC_WITH_IMM;
			wc->ex.imm_data = cqe->imm_data; /* be32 in wc */
		}
	}

	wc->byte_len = meta->len; /* XXX byte_len must come from cqe */
	wc->src_qp = ionic_cqe_src_qpn(cqe);

	/* XXX: also need from cqe... pkey_index, slid, sl, dlid_path_bits */

out:
	ionic_queue_consume(&qp->rq);

	return 1;
}

static int ionic_poll_send(struct ionic_qp *qp, struct ib_wc *wc)
{
	struct ionic_sq_meta *meta;
	int npolled = 1;

	/* completed all send queue requests? */
	if (ionic_queue_empty(&qp->sq))
		return 0;

	meta = &qp->sq_meta[qp->sq.cons];

	/* waiting for a cqe to complete the next request? */
	if (ionic_op_is_local(meta->op)) {
		if (meta->seq == qp->sq_npg_cons)
			return 0;
	} else {
		if (meta->seq == qp->sq_msn_cons)
			return 0;
	}

	memset(wc, 0, sizeof(*wc));

	wc->status = ionic_to_ib_wc_status(meta->status);
	wc->vendor_err = meta->status;

	wc->wr_id = meta->wrid;

	/* XXX possible use-after-free after rcu read unlock */
	wc->qp = &qp->ibqp;

	if (wc->status != IB_WC_SUCCESS)
		goto out;

	if (!meta->signal) {
		npolled = 0;
		goto out;
	}

	wc->opcode = ionic_to_ib_wc_opcd(meta->op);
	wc->byte_len = meta->len;

out:
	if (ionic_op_is_local(meta->op))
		qp->sq_npg_cons = (qp->sq_npg_cons + 1) & qp->sq.mask;
	else
		qp->sq_msn_cons = (qp->sq_msn_cons + 1) & qp->sq.mask;

	ionic_queue_consume(&qp->sq);

	return npolled;
}

static int ionic_poll_send_many(struct ionic_qp *qp, struct ib_wc *wc, int nwc)
{
	int rc = 0, npolled = 0;

	while(npolled < nwc) {
		rc = ionic_poll_send(qp, wc + npolled);
		if (rc <= 0)
			break;

		npolled += rc;
	}

	return npolled ?: rc;
}

static int ionic_validate_cons(u16 prod, u16 cons,
			       u16 comp, u16 mask)
{
	if (((prod - cons) & mask) < ((comp - cons) & mask))
		return -EIO;

	return 0;
}

static int ionic_comp_msn(struct ionic_qp *qp, struct cqwqe_be_t *cqe)
{
	u16 cqe_seq, cqe_idx;
	int rc;

	cqe_seq = be32_to_cpu(cqe->id.msn) & qp->sq.mask;
	cqe_idx = qp->sq_msn_idx[cqe_seq];

	rc = ionic_validate_cons(qp->sq_msn_prod,
				 qp->sq_msn_cons,
				 cqe_seq, qp->sq.mask);
	if (rc)
		return rc;

	qp->sq_msn_cons = cqe_seq;

	qp->sq_meta[cqe_idx].status = cqe->status;

	return 0;
}

static int ionic_comp_npg(struct ionic_qp *qp, struct cqwqe_be_t *cqe)
{
	u16 cqe_seq, cqe_idx;
	int rc;

	cqe_idx = be32_to_cpu(cqe->id.msn) & qp->sq.mask;
	cqe_seq = qp->sq_meta[cqe_idx].seq;

	rc = ionic_validate_cons(qp->sq_npg_prod,
				 qp->sq_npg_cons,
				 cqe_seq, qp->sq.mask);
	if (rc)
		return rc;

	qp->sq_npg_cons = cqe_seq;

	qp->sq_meta[cqe_idx].status = cqe->status;

	return 0;
}

static bool ionic_next_qp_cqe(struct ionic_cq *cq, struct ionic_qp **qp,
			      struct cqwqe_be_t *cqe)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(cq->ibcq.device);
	struct cqwqe_be_t *qcqe;

again:
	qcqe = ionic_queue_at_prod(&cq->q);

	if (ionic_queue_color(&cq->q) != ionic_cqe_color(qcqe))
		return false;

	rmb();

	*cqe = *qcqe;

	dev_dbg(&dev->ibdev.dev, "poll cq %d prod %d", cq->cqid, cq->q.prod);
	print_hex_dump_debug("cqe ", DUMP_PREFIX_OFFSET, 16, 1,
			     cqe, cq->q.stride, true);

	*qp = tbl_lookup(&dev->qp_tbl, ionic_cqe_qpn(cqe));

	/* Skip this cqe if the qp was destroyed.
	 * Unlikely race: qp was destroyed, and new qp created with same id.
	 */
	if (unlikely(!*qp)) {
		ionic_queue_produce(&cq->q);
		ionic_queue_color_wrap(&cq->q);
		goto again;
	}

	return true;
}

static int ionic_poll_cq(struct ib_cq *ibcq, int nwc, struct ib_wc *wc)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibcq->device);
	struct ionic_cq *cq = to_ionic_cq(ibcq);
	struct ionic_qp *qp, *qp_next;
	struct cqwqe_be_t cqe;
	u16 old_prod;
	int rc = 0, npolled = 0;
	unsigned long irqflags;

	/* Note about rc: (noted here because poll is different)
	 *
	 * Functions without "poll" in the name, if they return an integer,
	 * return zero on success, or a positive error number.  Functions
	 * returning a pointer return NULL on error and set errno to a positve
	 * error number.
	 *
	 * Functions with "poll" in the name return negative error numbers, or
	 * greater or equal to zero number of completions on success.
	 */

	if (nwc < 1)
		return 0;

	rcu_read_lock();

	spin_lock_irqsave(&cq->lock, irqflags);

	/* poll already indicated work completions for send queue */

	list_for_each_entry_safe(qp, qp_next, &cq->qp_poll, cq_poll_ent) {
		if (npolled == nwc)
			break;

		spin_lock(&qp->sq_lock);
		rc = ionic_poll_send_many(qp, wc + npolled, nwc - npolled);
		spin_unlock(&qp->sq_lock);

		if (rc > 0)
			npolled += rc;
		else
			list_del_init(&qp->cq_poll_ent);
	}

	/* poll for more work completions */

	old_prod = cq->q.prod;

	while (npolled < nwc) {
		if (!ionic_next_qp_cqe(cq, &qp, &cqe))
			break;

		/* ionic cqe op_type must be valid even if status is not ok */
		switch(cqe.op_type) {
		case OP_TYPE_SEND:
		case OP_TYPE_SEND_INV:
		case OP_TYPE_SEND_IMM:
		case OP_TYPE_READ:
		case OP_TYPE_WRITE:
		case OP_TYPE_WRITE_IMM:
		case OP_TYPE_CMP_N_SWAP:

			/* completion for packet-generating request */

			spin_lock(&qp->sq_lock);
			rc = ionic_comp_msn(qp, &cqe);
			spin_unlock(&qp->sq_lock);

			if (rc < 0)
				goto out;

			list_del(&qp->cq_poll_ent);
			list_add_tail(&qp->cq_poll_ent, &cq->qp_poll);
			break;

		case OP_TYPE_FETCH_N_ADD:
		case OP_TYPE_LOCAL_INV:
		case OP_TYPE_BIND_MW:

			/* completion for non-packet-generating request */

			spin_lock(&qp->sq_lock);
			rc = ionic_comp_npg(qp, &cqe);
			spin_unlock(&qp->sq_lock);

			if (rc < 0)
				goto out;

			list_del(&qp->cq_poll_ent);
			list_add_tail(&qp->cq_poll_ent, &cq->qp_poll);
			break;

		case OP_TYPE_SEND_RCVD:
		case OP_TYPE_RDMA_OPER_WITH_IMM:

			/* completion for receive */

			spin_lock(&qp->rq_lock);
			rc = ionic_poll_recv(qp, wc + npolled, &cqe);
			spin_unlock(&qp->rq_lock);

			if (rc < 0)
				goto out;

			npolled += rc;
			break;

		default:
			rc = -EIO;
			goto out;
		}

		ionic_queue_produce(&cq->q);
		ionic_queue_color_wrap(&cq->q);
	}

	/* poll newly indicated work completions for send queue */

	list_for_each_entry_safe(qp, qp_next, &cq->qp_poll, cq_poll_ent) {
		if (npolled == nwc)
			break;

		spin_lock(&qp->sq_lock);
		rc = ionic_poll_send_many(qp, wc + npolled, nwc - npolled);
		spin_unlock(&qp->sq_lock);

		if (rc > 0)
			npolled += rc;
		else
			list_del_init(&qp->cq_poll_ent);
	}

out:
	if (likely(cq->q.prod != old_prod)) {
		dev_dbg(&dev->ibdev.dev, "dbell qtype %d val %#llx\n",
			dev->cq_qtype, ionic_queue_dbell_val(&cq->q));
		/* the cq doorbell is not ready yet */
		//ionic_dbell_ring(&dev->dbpage[dev->cq_qtype],
		//		 ionic_queue_dbell_val(&cq->q));
	}

	spin_unlock_irqrestore(&cq->lock, irqflags);

	rcu_read_unlock();

	return npolled ?: rc;
}

static int ionic_peek_cq(struct ib_cq *ibcq, int nwc)
{
	return -ENOSYS;
}

static int ionic_req_notify_cq(struct ib_cq *ibcq,
			       enum ib_cq_notify_flags flags)
{
	//struct ionic_ibdev *dev = to_ionic_ibdev(ibcq->device);
	struct ionic_cq *cq = to_ionic_cq(ibcq);
	struct cqwqe_be_t *qcqe;
	u32 dbell_val;
	int rc = 0;

	/* IB_CQ_REPORT_MISSED_EVENTS:
	 *
	 * The queue index in the dbell value guarantees no missed events.
	 *
	 * Here, we check if the color bit in the next cqe is flipped.  If it
	 * is flipped, then progress can be made by immediatly polling the cq.
	 * Sill, the cq will be armed, and an event will be generated.  The cq
	 * may be empty when polled after the event, because the next poll
	 * after arming the cq can empty it.
	 */
	if (flags & IB_CQ_REPORT_MISSED_EVENTS) {
		qcqe = ionic_queue_at_prod(&cq->q);
		if (ionic_queue_color(&cq->q) != ionic_cqe_color(qcqe))
			rc = 1;
	}

	dbell_val = ionic_queue_dbell_val(&cq->q);

	if (flags & IB_CQ_SOLICITED)
		dbell_val |= IONIC_DBELL_RING_ARM_SOLICITED;
	else
		dbell_val |= IONIC_DBELL_RING_ARM;

	/* XXX the cq doorbell is not ready yet */
	if (ionic_xxx_notify)
		queue_delayed_work(ionic_workq, &cq->notify_work, HZ/4);
	else
		//ionic_dbell_ring(&dev->dbpage[dev->cq_qtype],
		//		 ionic_queue_dbell_val_arm(&cq->q));
		(void)cq; /* kill warning unused variable */

	return rc;
}

static int ionic_create_qp_cmd(struct ionic_ibdev *dev,
			       struct ionic_pd *pd,
			       struct ionic_cq *send_cq,
			       struct ionic_cq *recv_cq,
			       struct ionic_qp *qp,
			       struct ib_qp_init_attr *attr)
{
	struct ionic_admin_ctx admin = {
		.work = COMPLETION_INITIALIZER_ONSTACK(admin.work),
		/* XXX endian? */
		.cmd.create_qp = {
			.opcode = CMD_OPCODE_RDMA_CREATE_QP,
			.sq_wqe_size = qp->sq.stride,
			.rq_wqe_size = qp->rq.stride,
			.num_sq_wqes = (u32)qp->sq.mask + 1,
			.num_rq_wqes = (u32)qp->rq.mask + 1,
			.num_rsq_wqes = IONIC_NUM_RSQ_WQE,
			.num_rrq_wqes = IONIC_NUM_RRQ_WQE,
			.pd = pd->pdid,
			/* XXX lif should be dbid */
			.lif_id = dev->lif_id,
			.service = ib_qp_type_to_ionic(attr->qp_type),
			.pmtu = ib_mtu_enum_to_int(dev->port_attr.active_mtu), /* XXX should set mtu in modify */
			.qp_num = qp->qpid,
			.sq_cq_num = send_cq->cqid,
			.rq_cq_num = recv_cq->cqid,
			.host_pg_size = PAGE_SIZE,
			.sq_lkey = qp->sq_lkey,
			.rq_lkey = qp->rq_lkey,
		},
	};
	int rc;

	if (qp->has_sq) {
		if (qp->sq_umem)
			rc = ionic_kernel_umr_cmd(dev, pd,
						  qp->sq_lkey, qp->sq_lkey, 0,
						  qp->sq.size, qp->sq_umem, 0);
		else
			rc = ionic_kernel_mr_cmd(dev, pd, qp->sq_lkey,
						 qp->sq_lkey, 0, qp->sq.size,
						 PAGE_SIZE, qp->sq.dma, 0);
		if (rc)
			goto err_cmd;
	}

	if (qp->has_rq) {
		if (qp->rq_umem)
			rc = ionic_kernel_umr_cmd(dev, pd,
						  qp->rq_lkey, qp->rq_lkey, 0,
						  qp->rq.size, qp->rq_umem, 0);
		else
			rc = ionic_kernel_mr_cmd(dev, pd, qp->rq_lkey,
						 qp->rq_lkey, 0, qp->rq.size,
						 PAGE_SIZE, qp->rq.dma, 0);
		if (rc)
			goto err_cmd;
	}

	rc = ionic_api_adminq_post(dev->lif, &admin);
	if (rc)
		goto err_cmd;

	wait_for_completion(&admin.work);

	if (0 /* TODO: admin queue failure */) {
		rc = -EIO;
		goto err_cmd;
	}

	rc = ionic_verbs_status_to_rc(admin.comp.create_qp.status);

err_cmd:
	return rc;
}

static int ionic_modify_qp_cmd(struct ionic_ibdev *dev,
			       struct ionic_qp *qp,
			       struct ib_qp_attr *attr,
			       int mask)
{
	struct ionic_admin_ctx admin = {
		.work = COMPLETION_INITIALIZER_ONSTACK(admin.work),
		/* XXX endian? */
		.cmd.modify_qp = {
			.opcode = CMD_OPCODE_RDMA_MODIFY_QP,
			.qp_num = qp->ibqp.qp_num,
			.attr_mask = (u32)mask,
			.q_key = attr->qkey,
			.dest_qp_num = attr->dest_qp_num,
			.e_psn = attr->rq_psn,
			.sq_psn = attr->sq_psn,
			/* XXX path mtu */
		},
	};
	struct ib_ud_header *hdr;
	void *hdr_buf = NULL;
	dma_addr_t hdr_dma = 0;
	int rc, hdr_len = 0;

	if (mask & IB_QP_AV) {
		hdr = kmalloc(sizeof(*hdr), GFP_KERNEL);
		if (!hdr) {
			rc = -ENOMEM;
			goto err_hdr;
		}

		rc = ionic_build_hdr(dev, hdr, &attr->ah_attr);
		if (rc)
			goto err_buf;

		hdr_buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
		if (!hdr_buf) {
			rc = -ENOMEM;
			goto err_buf;
		}

		hdr_len = ib_ud_header_pack(hdr, hdr_buf);
		hdr_len -= IB_BTH_BYTES;
		hdr_len -= IB_DETH_BYTES;

		dev_dbg(&dev->ibdev.dev, "roce packet header template\n");
		print_hex_dump_debug("hdr ", DUMP_PREFIX_OFFSET, 16, 1,
				     hdr_buf, hdr_len, true);

		hdr_dma = ib_dma_map_single(&dev->ibdev, hdr_buf, hdr_len,
					    DMA_TO_DEVICE);

		rc = ib_dma_mapping_error(&dev->ibdev, hdr_dma);
		if (rc)
			goto err_dma;

		admin.cmd.modify_qp.header_template = hdr_dma;
		admin.cmd.modify_qp.header_template_size = hdr_len;

		/* XXX for HAPS: side-data */
		if (ionic_xxx_haps) {
#ifndef ADMINQ
			admin.side_data = hdr_buf;
			admin.side_data_len = round_up(hdr_len, sizeof(u32));
#endif
		}
	}

	rc = ionic_api_adminq_post(dev->lif, &admin);
	if (rc)
		goto err_cmd;

	wait_for_completion(&admin.work);

	if (0 /* TODO: admin queue failure */) {
		rc = -EIO;
		goto err_cmd;
	}

	rc = ionic_verbs_status_to_rc(admin.comp.create_qp.status);

err_cmd:
	if (mask & IB_QP_AV)
		ib_dma_unmap_single(&dev->ibdev, hdr_dma, hdr_len,
				    DMA_TO_DEVICE);
err_dma:
	if (mask & IB_QP_AV)
		kfree(hdr_buf);
err_buf:
	if (mask & IB_QP_AV)
		kfree(hdr);
err_hdr:
	return rc;
}

static int ionic_destroy_qp_cmd(struct ionic_ibdev *dev,
				struct ionic_qp *qp)
{
#if 1
	/* need destroy_qp admin command */
	return 0;
#else
	struct ionic_admin_ctx admin = {
		.work = COMPLETION_INITIALIZER_ONSTACK(admin.work),
		/* XXX endian? */
		.cmd.destroy_qp = {
			.opcode = CMD_OPCODE_RDMA_DESTROY_QP,
			.qp_num = qp->qpid,
		},
	};
	int rc;

	rc = ionic_api_adminq_post(dev->lif, &admin);
	if (rc)
		goto err_cmd;

	wait_for_completion(&admin.work);

	if (0 /* XXX did the queue fail? */) {
		rc = -EIO;
		goto err_cmd;
	}

	rc = ionic_verbs_status_to_rc(admin.comp.destroy_qp.status);

err_cmd:
	return rc;
#endif
}

static void ionic_qp_sq_init_hbm(struct ionic_ibdev *dev,
				 struct ionic_ctx *ctx,
				 struct ionic_qp *qp,
				 bool cap_inline)
{
	int rc;

	INIT_LIST_HEAD(&qp->sq_hbm_mmap.ctx_ent);

	if (!qp->has_sq)
		goto err_hbm;

	if (ionic_sqhbm_inline && !cap_inline)
		goto err_hbm;

	qp->sq_hbm_order = order_base_2(qp->sq.size / PAGE_SIZE);

	if (qp->sq_hbm_order >= ionic_sqhbm_order)
		goto err_hbm;

	rc = ionic_api_get_hbm(dev->lif, &qp->sq_hbm_pgid,
			       &qp->sq_hbm_addr, qp->sq_hbm_order);
	if (rc)
		goto err_hbm;

	if (ctx) {
		qp->sq_hbm_ptr = NULL;
	} else {
		qp->sq_hbm_ptr = ioremap(qp->sq_hbm_addr, qp->sq.size);
		if (!qp->sq_hbm_ptr)
			goto err_map;
	}

	qp->sq_is_hbm = true;

	return;

err_map:
	ionic_api_put_hbm(dev->lif, qp->sq_hbm_pgid, qp->sq_hbm_order);
err_hbm:
	qp->sq_is_hbm = false;
	qp->sq_hbm_ptr = NULL;
	qp->sq_hbm_order = -1;
	qp->sq_hbm_pgid = 0;
	qp->sq_hbm_addr = 0;

	qp->sq_hbm_mmap.offset = 0;
	qp->sq_hbm_mmap.size = 0;
	qp->sq_hbm_mmap.pfn = 0;
}

static void ionic_qp_sq_destroy_hbm(struct ionic_ibdev *dev,
				    struct ionic_ctx *ctx,
				    struct ionic_qp *qp)
{
	if (!qp->sq_is_hbm)
		return;

	if (ctx) {
		mutex_lock(&ctx->mmap_mut);
		list_del(&qp->sq_hbm_mmap.ctx_ent);
		mutex_unlock(&ctx->mmap_mut);
	} else {
		iounmap(&qp->sq_hbm_ptr);
	}

	ionic_api_put_hbm(dev->lif, qp->sq_hbm_pgid, qp->sq_hbm_order);
}

static void ionic_qp_no_sq(struct ionic_qp *qp)
{
	memset(&qp->sq, 0, sizeof(qp->sq));

	qp->sq_is_hbm = false;
	qp->sq_hbm_ptr = NULL;
	qp->sq_hbm_prod = 0;
	qp->sq_hbm_order = 0;
	qp->sq_hbm_pgid = 0;
	qp->sq_hbm_addr = 0;

	INIT_LIST_HEAD(&qp->sq_hbm_mmap.ctx_ent);
	qp->sq_hbm_mmap.offset = 0;
	qp->sq_hbm_mmap.size = 0;
	qp->sq_hbm_mmap.pfn = 0;

	qp->sq_umem = NULL;
	qp->sq_tbl_order = 0;
	qp->sq_tbl_pos = 0;

	qp->sq_lkey = 0;
}

static int ionic_qp_sq_init(struct ionic_ibdev *dev, struct ionic_ctx *ctx,
			    struct ionic_qp *qp, struct ionic_qdesc *sq,
			    int max_wr, int max_sge, int max_inline)
{
	int rc;
	u32 wqe_size;

	qp->sq_msn_prod = 0;
	qp->sq_msn_cons = 0;
	qp->sq_npg_prod = 0;
	qp->sq_npg_cons = 0;
	qp->sq_hbm_prod = 0;

	if (!qp->has_sq) {
		if (ctx) {
			rc = ionic_validate_qdesc_zero(sq);
			if (rc)
				goto err_sq;
		}

		ionic_qp_no_sq(qp);
	} else {
		if (ctx) {
			rc = ionic_validate_qdesc(sq);
			if (rc)
				goto err_sq;

			qp->sq.ptr = NULL;
			qp->sq.size = sq->size;
			qp->sq.mask = sq->mask;
			qp->sq.stride = sq->stride;

			qp->sq_meta = NULL;
			qp->sq_msn_idx = NULL;

			qp->sq_umem = ib_umem_get(&ctx->ibctx, sq->addr,
						  sq->size, 0, 0);
			if (IS_ERR(qp->sq_umem)) {
				rc = PTR_ERR(qp->sq_umem);
				goto err_sq;
			}

			ionic_qp_sq_init_hbm(dev, ctx, qp, max_inline > 0);

			if (!qp->sq_is_hbm) {
				rc = ionic_pgtbl_init(dev, qp->sq_umem,
						      &qp->sq_tbl_pos,
						      &qp->sq_tbl_order);
				if (rc)
					goto err_sq_tbl;
			}
		} else {
			qp->sq_umem = NULL;
			qp->sq_tbl_order = 0;
			qp->sq_tbl_pos = 0;

			wqe_size = ionic_sq_wqe_size(max_sge, max_inline);
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

			ionic_qp_sq_init_hbm(dev, ctx, qp, max_inline > 0);
		}
	}

	return 0;

err_sq_tbl:
	ionic_qp_sq_destroy_hbm(dev, ctx, qp);

	if (qp->sq_msn_idx)
		kfree(qp->sq_msn_idx);
err_sq_msn:
	if (qp->sq_meta)
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

	if (qp->sq_tbl_order)
		ionic_put_pgtbl(dev, qp->sq_tbl_pos, qp->sq_tbl_order);

	ionic_qp_sq_destroy_hbm(dev, ctx, qp);

	if (qp->sq_msn_idx)
		kfree(qp->sq_msn_idx);

	if (qp->sq_meta)
		kfree(qp->sq_meta);

	if (qp->sq_umem)
		ib_umem_release(qp->sq_umem);
	else
		ionic_queue_destroy(&qp->sq, dev->hwdev);
}

static void ionic_qp_no_rq(struct ionic_qp *qp)
{
	memset(&qp->rq, 0, sizeof(qp->rq));

	qp->rq_umem = NULL;
	qp->rq_tbl_order = 0;
	qp->rq_tbl_pos = 0;

	qp->rq_lkey = 0;
}

static int ionic_qp_rq_init(struct ionic_ibdev *dev, struct ionic_ctx *ctx,
			    struct ionic_qp *qp, struct ionic_qdesc *rq,
			    int max_wr, int max_sge)
{
	int rc;
	u32 wqe_size;

	if (!qp->has_rq) {
		if (ctx) {
			rc = ionic_validate_qdesc_zero(rq);
			if (rc)
				goto err_rq;
		}

		ionic_qp_no_rq(qp);
	} else {
		if (ctx) {
			rc = ionic_validate_qdesc(rq);
			if (rc)
				goto err_rq;

			qp->rq.ptr = NULL;
			qp->rq.size = rq->size;
			qp->rq.mask = rq->mask;
			qp->rq.stride = rq->stride;

			qp->rq_meta = NULL;

			qp->rq_umem = ib_umem_get(&ctx->ibctx, rq->addr,
						  rq->size, 0, 0);
			if (IS_ERR(qp->rq_umem)) {
				rc = PTR_ERR(qp->rq_umem);
				goto err_rq;
			}

			rc = ionic_pgtbl_init(dev, qp->rq_umem,
					      &qp->rq_tbl_pos,
					      &qp->rq_tbl_order);
			if (rc)
				goto err_rq_tbl;
		} else {
			qp->rq_umem = NULL;
			qp->rq_tbl_order = 0;
			qp->rq_tbl_pos = 0;

			wqe_size = ionic_rq_wqe_size(max_sge);
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
		}
	}

	return 0;

err_rq_tbl:
	if (qp->rq_meta)
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

	if (qp->rq_tbl_order)
		ionic_put_pgtbl(dev, qp->rq_tbl_pos, qp->rq_tbl_order);

	if (qp->rq_meta)
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
	struct ionic_ctx *ctx = to_ionic_ctx_uobj(ibpd->uobject);
	struct ionic_pd *pd = to_ionic_pd(ibpd);
	struct ionic_qp *qp;
	struct ionic_qp_req req;
	struct ionic_qp_resp resp = {0};
	int rc;

	if (!ctx) {
		rc = ionic_validate_udata(udata, 0, 0);
	} else {
		rc = ionic_validate_udata(udata, sizeof(req), sizeof(resp));
		if (!rc)
			rc = ib_copy_from_udata(&req, udata, sizeof(req));
	}

	if (rc)
		goto err_qp;

	qp = kmalloc(sizeof(*qp), GFP_KERNEL);
	if (!qp) {
		rc = -ENOMEM;
		goto err_qp;
	}

	INIT_LIST_HEAD(&qp->cq_poll_ent);

	if (attr->qp_type == IB_QPT_SMI) {
		rc = -EINVAL;
		goto err_qp;
	}

	if (attr->qp_type > IB_QPT_UD) {
		rc = -EINVAL;
		goto err_qp;
	}

	if (attr->qp_type == IB_QPT_GSI)
		rc = ionic_get_gsi_qpid(dev, &qp->qpid);
	else
		rc = ionic_get_qpid(dev, &qp->qpid);
	if (rc)
		goto err_qpid;

	rc = ionic_get_mrid(dev, &qp->sq_lkey, &qp->rq_lkey);
	if (rc)
		goto err_mrid;

	qp->has_sq = attr->qp_type != IB_QPT_XRC_TGT;

	qp->has_rq = !attr->srq &&
		attr->qp_type != IB_QPT_XRC_INI &&
		attr->qp_type != IB_QPT_XRC_TGT;

	qp->is_srq = false;

	spin_lock_init(&qp->sq_lock);
	spin_lock_init(&qp->rq_lock);

	rc = ionic_qp_sq_init(dev, ctx, qp, &req.sq, attr->cap.max_send_wr,
			      attr->cap.max_send_sge, attr->cap.max_inline_data);
	if (rc)
		goto err_sq;

	rc = ionic_qp_rq_init(dev, ctx, qp, &req.rq, attr->cap.max_recv_wr,
			      attr->cap.max_recv_sge);
	if (rc)
		goto err_rq;

	rc = ionic_create_qp_cmd(dev, pd,
				 to_ionic_cq(attr->send_cq),
				 to_ionic_cq(attr->recv_cq),
				 qp, attr);
	if (rc)
		goto err_cmd;

	if (ctx) {
		resp.qpid = qp->qpid;

		if (qp->sq_is_hbm) {
			qp->sq_hbm_mmap.size = qp->sq.size;
			qp->sq_hbm_mmap.pfn = PHYS_PFN(qp->sq_hbm_addr);

			mutex_lock(&ctx->mmap_mut);
			qp->sq_hbm_mmap.offset = ctx->mmap_off;
			ctx->mmap_off += qp->sq.size;
			list_add(&qp->sq_hbm_mmap.ctx_ent, &ctx->mmap_list);
			mutex_unlock(&ctx->mmap_mut);

			resp.sq_hbm_offset = qp->sq_hbm_mmap.offset;
		}

		rc = ib_copy_to_udata(udata, &resp, sizeof(resp));
		if (rc)
			goto err_resp;
	}

	qp->ibqp.qp_num = qp->qpid;

	ionic_dbgfs_add_qp(dev, qp);

	mutex_lock(&dev->tbl_lock);
	tbl_alloc_node(&dev->qp_tbl);
	tbl_insert(&dev->qp_tbl, qp, qp->qpid);
	mutex_unlock(&dev->tbl_lock);

	return &qp->ibqp;

err_resp:
	ionic_destroy_qp_cmd(dev, qp);
err_cmd:
	ionic_qp_rq_destroy(dev, ctx, qp);
err_rq:
	ionic_qp_sq_destroy(dev, ctx, qp);
err_sq:
	ionic_put_mrid(dev, qp->sq_lkey);
err_mrid:
	ionic_put_qpid(dev, qp->qpid);
err_qpid:
	kfree(qp);
err_qp:
	return ERR_PTR(rc);
}

static int ionic_modify_qp(struct ib_qp *ibqp, struct ib_qp_attr *attr,
			   int mask, struct ib_udata *udata)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibqp->device);
	struct ionic_qp *qp = to_ionic_qp(ibqp);
	int rc;

	rc = ionic_validate_udata(udata, 0, 0);
	if (rc)
		goto err_qp;

	rc = ionic_modify_qp_cmd(dev, qp, attr, mask);

err_qp:
	return rc;
}

static int ionic_query_qp(struct ib_qp *ibqp, struct ib_qp_attr *attr,
			  int mask, struct ib_qp_init_attr *init_attr)
{
	return -ENOSYS;
}

static int ionic_destroy_qp(struct ib_qp *ibqp)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibqp->device);
	struct ionic_ctx *ctx = to_ionic_ctx_uobj(ibqp->uobject);
	struct ionic_qp *qp = to_ionic_qp(ibqp);
	struct ionic_cq *cq;
	unsigned long irqflags;
	int rc;

	rc = ionic_destroy_qp_cmd(dev, qp);
	if (rc)
		return rc;

	mutex_lock(&dev->tbl_lock);
	tbl_free_node(&dev->qp_tbl);
	tbl_delete(&dev->qp_tbl, qp->qpid);
	mutex_unlock(&dev->tbl_lock);

	synchronize_rcu();

	if (qp->has_sq) {
		cq = to_ionic_cq(qp->ibqp.send_cq);
		spin_lock_irqsave(&cq->lock, irqflags);
		list_del(&qp->cq_poll_ent);
		spin_unlock_irqrestore(&cq->lock, irqflags);
	}

	ionic_dbgfs_rm_qp(qp);

	ionic_qp_rq_destroy(dev, ctx, qp);
	ionic_qp_sq_destroy(dev, ctx, qp);
	ionic_put_mrid(dev, qp->sq_lkey);
	ionic_put_qpid(dev, qp->qpid);

	kfree(qp);

	return 0;
}

static s64 ionic_prep_inline(void *data, size_t max_data,
			     struct ib_sge *ib_sgl, int num_sge)
{
	static const s64 bit_31 = 1u << 31;
	s64 len = 0, sg_len;
	int sg_i;

	for (sg_i = 0; sg_i < num_sge; ++sg_i) {
		sg_len = ib_sgl[sg_i].length;

		/* sge length zero means 2GB */
		if (unlikely(sg_len == 0))
			sg_len = bit_31;

		/* greater than max inline data is invalid */
		if (unlikely(len + sg_len > max_data))
			return -EINVAL;

		memcpy(data + len, (void *)ib_sgl[sg_i].addr, sg_len);

		len += sg_len;
	}

	return len;
}

static s64 ionic_prep_sgl(struct sge_t *sgl, u32 max_sge,
			  struct ib_sge *ib_sgl, int num_sge)
{
	static const s64 bit_31 = 1l << 31;
	s64 len = 0, sg_len;
	int sg_i;

	if (unlikely(num_sge < 0 || (u32)num_sge > max_sge))
		return -EINVAL;

	for (sg_i = 0; sg_i < num_sge; ++sg_i) {
		sg_len = ib_sgl[sg_i].length;

		/* sge length zero means 2GB */
		if (unlikely(sg_len == 0))
			sg_len = bit_31;

		/* greater than 2GB data is invalid */
		if (unlikely(len + sg_len > bit_31))
			return -EINVAL;

		sgl[sg_i].va = cpu_to_be64(ib_sgl[sg_i].addr);
		sgl[sg_i].len = cpu_to_be32(sg_len);
		sgl[sg_i].lkey = cpu_to_be32(ib_sgl[sg_i].lkey);

		len += sg_len;
	}

	return len;
}

static void ionic_prep_base(struct ionic_qp *qp,
			    struct ib_send_wr *wr,
			    struct ionic_sq_meta *meta,
			    struct sqwqe_t *wqe)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(qp->ibqp.device);

	meta->wrid = wr->wr_id;
	meta->status = CQ_STATUS_SUCCESS;
	meta->signal = false;

	/* XXX wqe wrid can be removed */
	wqe->base.wrid = 0;

	if (wr->send_flags & IB_SEND_FENCE)
		wqe->base.fence = 1;

	if (wr->send_flags & IB_SEND_SOLICITED)
		wqe->base.solicited_event = 1;

	if (qp->sig_all || wr->send_flags & IB_SEND_SIGNALED) {
		wqe->base.complete_notify = 1;
		meta->signal = true;
	}

	if (ionic_op_is_local(qp->sq_meta[qp->sq.prod].op)) {
		meta->seq = qp->sq_npg_prod;
		qp->sq_npg_prod = (qp->sq_npg_prod + 1) & qp->sq.mask;
	} else {
		meta->seq = qp->sq_msn_prod;
		qp->sq_msn_prod = (qp->sq_msn_prod + 1) & qp->sq.mask;

		qp->sq_msn_idx[meta->seq] = qp->sq.prod;
	}

	dev_dbg(&dev->ibdev.dev, "post send %u prod %u", qp->qpid, qp->sq.prod);
	print_hex_dump_debug("wqe ", DUMP_PREFIX_OFFSET, 16, 1,
			     wqe, qp->sq.stride, true);

	ionic_queue_produce(&qp->sq);
}

static int ionic_prep_common(struct ionic_qp *qp,
			     struct ib_send_wr *wr,
			     struct ionic_sq_meta *meta,
			     struct sqwqe_t *wqe,
			     /* XXX length field offset differs per opcode */
			     __u32 *wqe_length_field)
{
	s64 signed_len;

	if (wr->send_flags & IB_SEND_INLINE) {
		wqe->base.num_sges = 0;
		wqe->base.inline_data_vld = 1;
		signed_len = ionic_prep_inline(wqe->u.non_atomic.sg_arr,
					       ionic_sq_wqe_max_inline(qp->sq.stride),
					       wr->sg_list, wr->num_sge);
	} else {
		wqe->base.num_sges = wr->num_sge;
		signed_len = ionic_prep_sgl(wqe->u.non_atomic.sg_arr,
					    ionic_sq_wqe_max_sge(qp->sq.stride),
					    wr->sg_list, wr->num_sge);
	}

	if (unlikely(signed_len < 0))
		return (int)-signed_len;

	meta->len = (u32)signed_len;
	*wqe_length_field = cpu_to_be32((u32)signed_len);

	ionic_prep_base(qp, wr, meta, wqe);

	return 0;
}

static int ionic_prep_send(struct ionic_qp *qp,
			   struct ib_send_wr *wr)
{
	struct ionic_sq_meta *meta;
	struct sqwqe_t *wqe;

	meta = &qp->sq_meta[qp->sq.prod];
	wqe = ionic_queue_at_prod(&qp->sq);

	memset(wqe, 0, qp->sq.stride);

	switch (wr->opcode) {
	case IB_WR_SEND:
		meta->op = IONIC_WR_OPCD_SEND;
		wqe->base.op_type = IONIC_WR_OPCD_SEND;
		break;
	case IB_WR_SEND_WITH_IMM:
		meta->op = IONIC_WR_OPCD_SEND_IMM;
		wqe->base.op_type = IONIC_WR_OPCD_SEND_IMM;
		wqe->u.non_atomic.wqe.send.imm_data = wr->ex.imm_data;
		break;
	case IB_WR_SEND_WITH_INV:
		meta->op = IONIC_WR_OPCD_SEND_INVAL;
		wqe->base.op_type = IONIC_WR_OPCD_SEND_INVAL;
		wqe->u.non_atomic.wqe.send.imm_data =
			cpu_to_be32(wr->ex.invalidate_rkey);

		/* XXX just use imm_data field */
		wqe->u.non_atomic.wqe.send.inv_key =
			cpu_to_be32(wr->ex.invalidate_rkey);
		break;
	default:
		return EINVAL;
	}

	return ionic_prep_common(qp, wr, meta, wqe,
				 &wqe->u.non_atomic.wqe.send.length);
}

static int ionic_prep_send_ud(struct ionic_qp *qp,
			      struct ib_ud_wr *wr)
{
	struct ionic_sq_meta *meta;
	struct sqwqe_t *wqe;
	struct ionic_ah *ah;

	if (unlikely(!wr->ah))
		return EINVAL;

	ah = to_ionic_ah(wr->ah);

	wqe = ionic_queue_at_prod(&qp->sq);

	/* XXX endian? */
	wqe->u.non_atomic.wqe.ud_send.q_key = wr->remote_qkey;
	wqe->u.non_atomic.wqe.ud_send.ah_size = ah->len;
	wqe->u.non_atomic.wqe.ud_send.dst_qp = wr->remote_qpn;
	wqe->u.non_atomic.wqe.ud_send.ah_handle = ah->ahid;

	/* XXX duplicated code.
	 *
	 * This can be cleaned up by making sqwqe_rc_send_t and sqwqe_ud_send_t
	 * the same descriptor format, and replacing following dup'd code with
	 * call to ionic_prep_send().
	 */

	meta = &qp->sq_meta[qp->sq.prod];
	wqe = ionic_queue_at_prod(&qp->sq);

	memset(wqe, 0, qp->sq.stride);

	switch (wr->wr.opcode) {
	case IB_WR_SEND:
		meta->op = IONIC_WR_OPCD_SEND;
		wqe->base.op_type = IONIC_WR_OPCD_SEND;
		break;
	case IB_WR_SEND_WITH_IMM:
		meta->op = IONIC_WR_OPCD_SEND_IMM;
		wqe->base.op_type = IONIC_WR_OPCD_SEND_IMM;
		wqe->u.non_atomic.wqe.ud_send.imm_data = wr->wr.ex.imm_data;
		break;
	default:
		return EINVAL;
	}

	return ionic_prep_common(qp, &wr->wr, meta, wqe,
				 &wqe->u.non_atomic.wqe.ud_send.length);
}

static int ionic_prep_rdma(struct ionic_qp *qp,
			   struct ib_rdma_wr *wr)
{
	struct ionic_sq_meta *meta;
	struct sqwqe_t *wqe;

	meta = &qp->sq_meta[qp->sq.prod];
	wqe = ionic_queue_at_prod(&qp->sq);

	memset(wqe, 0, qp->sq.stride);

	switch (wr->wr.opcode) {
	case IB_WR_RDMA_READ:
		if (wr->wr.send_flags & (IB_SEND_SOLICITED | IB_SEND_INLINE))
			return EINVAL;
		meta->op = IONIC_WR_OPCD_RDMA_READ;
		wqe->base.op_type = IONIC_WR_OPCD_RDMA_READ;
		break;
	case IB_WR_RDMA_WRITE:
		if (wr->wr.send_flags & IB_SEND_SOLICITED)
			return EINVAL;
		meta->op = IONIC_WR_OPCD_RDMA_WRITE;
		wqe->base.op_type = IONIC_WR_OPCD_RDMA_WRITE;
		break;
	case IB_WR_RDMA_WRITE_WITH_IMM:
		meta->op = IONIC_WR_OPCD_RDMA_WRITE_IMM;
		wqe->base.op_type = IONIC_WR_OPCD_RDMA_WRITE_IMM;
		wqe->u.non_atomic.wqe.ud_send.imm_data = wr->wr.ex.imm_data;
		break;
	default:
		return EINVAL;
	}

	wqe->u.non_atomic.wqe.rdma.va = cpu_to_be64(wr->remote_addr);
	wqe->u.non_atomic.wqe.rdma.r_key = cpu_to_be32(wr->rkey);

	return ionic_prep_common(qp, &wr->wr, meta, wqe,
				 &wqe->u.non_atomic.wqe.rdma.length);
}

static int ionic_prep_atomic(struct ionic_qp *qp,
			     struct ib_atomic_wr *wr)
{
	struct ionic_sq_meta *meta;
	struct sqwqe_t *wqe;

	if (wr->wr.num_sge != 1 || wr->wr.sg_list[0].length != 8)
		return EINVAL;

	if (wr->wr.send_flags & (IB_SEND_SOLICITED | IB_SEND_INLINE))
		return EINVAL;

	meta = &qp->sq_meta[qp->sq.prod];
	wqe = ionic_queue_at_prod(&qp->sq);

	memset(wqe, 0, qp->sq.stride);

	switch (wr->wr.opcode) {
	case IB_WR_ATOMIC_CMP_AND_SWP:
		meta->op = IONIC_WR_OPCD_ATOMIC_CS;
		wqe->base.op_type = IONIC_WR_OPCD_ATOMIC_CS;
		wqe->u.atomic.swap_or_add_data =
			cpu_to_be64(wr->swap);
		wqe->u.atomic.cmp_data =
			cpu_to_be64(wr->compare_add);
		break;
	case IB_WR_ATOMIC_FETCH_AND_ADD:
		meta->op = IONIC_WR_OPCD_ATOMIC_FA;
		wqe->base.op_type = IONIC_WR_OPCD_ATOMIC_FA;
		wqe->u.atomic.swap_or_add_data =
			cpu_to_be64(wr->compare_add);
		break;
	default:
		return EINVAL;
	}

	wqe->u.atomic.r_key = cpu_to_be32(wr->rkey);
	wqe->u.atomic.va = cpu_to_be64(wr->remote_addr);

	wqe->base.num_sges = 1;
	wqe->u.atomic.sge.va = cpu_to_be64(wr->wr.sg_list[0].addr);
	wqe->u.atomic.sge.lkey = cpu_to_be32(wr->wr.sg_list[0].lkey);
	wqe->u.atomic.sge.len = cpu_to_be32(8);

	ionic_prep_base(qp, &wr->wr, meta, wqe);

	return 0;
}

static int ionic_prep_one_rc(struct ionic_qp *qp,
			     struct ib_send_wr *wr)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(qp->ibqp.device);
	int rc = 0;

	switch (wr->opcode) {
	case IB_WR_SEND:
	case IB_WR_SEND_WITH_IMM:
	case IB_WR_SEND_WITH_INV:
		rc = ionic_prep_send(qp, wr);
		break;
	case IB_WR_RDMA_READ:
	case IB_WR_RDMA_WRITE:
	case IB_WR_RDMA_WRITE_WITH_IMM:
		rc = ionic_prep_rdma(qp, rdma_wr(wr));
		break;
	case IB_WR_ATOMIC_CMP_AND_SWP:
	case IB_WR_ATOMIC_FETCH_AND_ADD:
		rc = ionic_prep_atomic(qp, atomic_wr(wr));
		break;
	default:
		dev_dbg(&dev->ibdev.dev, "invalid opcode %d", wr->opcode);
		rc = -EINVAL;
	}

	return rc;
}

static int ionic_prep_one_ud(struct ionic_qp *qp,
			     struct ib_send_wr *wr)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(qp->ibqp.device);
	int rc = 0;

	switch (wr->opcode) {
	case IB_WR_SEND:
	case IB_WR_SEND_WITH_IMM:
		rc = ionic_prep_send_ud(qp, ud_wr(wr));
		break;
	default:
		dev_dbg(&dev->ibdev.dev, "invalid opcode %d", wr->opcode);
		rc = -EINVAL;
	}

	return rc;
}

static void ionic_post_hbm(struct ionic_ibdev *dev, struct ionic_qp *qp)
{
	void *hbm_ptr;
	void *wqe_ptr;
	u16 pos, end, mask, stride;

	pos = qp->sq_hbm_prod;
	end = qp->sq.prod;
	mask = qp->sq.mask;
	stride = qp->sq.stride;

	while (pos != end) {
		hbm_ptr = qp->sq_hbm_ptr + pos * stride;
		wqe_ptr = ionic_queue_at(&qp->sq, pos);

		memcpy_toio(hbm_ptr, wqe_ptr, qp->sq.stride);

		pos = (pos + 1) & mask;

		ionic_dbell_ring(&dev->dbpage[dev->sq_qtype],
				 qp->sq.dbell | pos);
	}

	qp->sq_hbm_prod = end;
}

static int ionic_prep_recv(struct ionic_qp *qp,
			   struct ib_recv_wr *wr)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(qp->ibqp.device);
	struct ionic_rq_meta *meta;
	struct rqwqe_t *wqe;
	s64 signed_len;

	meta = &qp->rq_meta[qp->rq.prod];
	wqe = ionic_queue_at_prod(&qp->rq);

	memset(wqe, 0, qp->rq.stride);

	signed_len = ionic_prep_sgl(wqe->sge_arr,
				    ionic_rq_wqe_max_sge(qp->rq.stride),
				    wr->sg_list, wr->num_sge);
	if (signed_len < 0)
		return (int)-signed_len;

	wqe->wrid = wr->wr_id;
	wqe->num_sges = wr->num_sge;

	meta->len = (u32)signed_len;

	dev_dbg(&dev->ibdev.dev, "post recv %u prod %u", qp->qpid, qp->rq.prod);
	print_hex_dump_debug("wqe ", DUMP_PREFIX_OFFSET, 16, 1,
			     wqe, qp->rq.stride, true);

	ionic_queue_produce(&qp->rq);

	return 0;
}

static int ionic_post_send_common(struct ionic_ibdev *dev,
				  struct ionic_ctx *ctx,
				  struct ionic_qp *qp,
				  struct ib_send_wr *wr,
				  struct ib_send_wr **bad)
{
	u16 old_prod;
	unsigned long irqflags;
	int rc = 0;

	if (!bad)
		return -EINVAL;

	if (ctx || !qp->has_sq) {
		*bad = wr;
		return -EINVAL;
	}

	spin_lock_irqsave(&qp->sq_lock, irqflags);

	old_prod = qp->sq.prod;

	if (qp->ibqp.qp_type == IB_QPT_UD || qp->ibqp.qp_type == IB_QPT_GSI) {
		while (wr) {
			if (ionic_queue_full(&qp->sq)) {
				dev_dbg(&dev->ibdev.dev, "queue full");
				rc = -ENOMEM;
				goto out;
			}

			rc = ionic_prep_one_ud(qp, wr);
			if (rc)
				goto out;

			wr = wr->next;
		}
	} else {
		while (wr) {
			if (ionic_queue_full(&qp->sq)) {
				dev_dbg(&dev->ibdev.dev, "queue full");
				rc = -ENOMEM;
				goto out;
			}

			rc = ionic_prep_one_rc(qp, wr);
			if (rc)
				goto out;

			wr = wr->next;
		}
	}

out:
	if (likely(qp->sq.prod != old_prod)) {
		if (qp->sq_hbm_ptr)
			ionic_post_hbm(dev, qp);
		else
			ionic_dbell_ring(&dev->dbpage[dev->sq_qtype],
					 ionic_queue_dbell_val(&qp->sq));
	}

	spin_unlock_irqrestore(&qp->sq_lock, irqflags);

	*bad = wr;
	return rc;
}

static int ionic_post_recv_common(struct ionic_ibdev *dev,
				  struct ionic_ctx *ctx,
				  struct ionic_qp *qp,
				  struct ib_recv_wr *wr,
				  struct ib_recv_wr **bad)
{
	u16 old_prod;
	unsigned long irqflags;
	int rc = 0;

	if (!bad)
		return -EINVAL;

	if (ctx || !qp->has_rq) {
		*bad = wr;
		return -EINVAL;
	}

	spin_lock_irqsave(&qp->rq_lock, irqflags);

	old_prod = qp->rq.prod;

	while (wr) {
		if (ionic_queue_full(&qp->rq)) {
			dev_dbg(&dev->ibdev.dev, "queue full");
			rc = -ENOMEM;
			goto out;
		}

		rc = ionic_prep_recv(qp, wr);
		if (rc)
			goto out;

		wr = wr->next;
	}

out:
	if (likely(qp->rq.prod != old_prod))
		ionic_dbell_ring(&dev->dbpage[dev->rq_qtype],
				 ionic_queue_dbell_val(&qp->rq));

	spin_unlock_irqrestore(&qp->rq_lock, irqflags);

	*bad = wr;
	return rc;
}

static int ionic_post_send(struct ib_qp *ibqp, struct ib_send_wr *wr,
			   struct ib_send_wr **bad)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibqp->device);
	struct ionic_ctx *ctx = to_ionic_ctx_uobj(ibqp->uobject);
	struct ionic_qp *qp = to_ionic_qp(ibqp);

	return ionic_post_send_common(dev, ctx, qp, wr, bad);
}

static int ionic_post_recv(struct ib_qp *ibqp, struct ib_recv_wr *wr,
			   struct ib_recv_wr **bad)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibqp->device);
	struct ionic_ctx *ctx = to_ionic_ctx_uobj(ibqp->uobject);
	struct ionic_qp *qp = to_ionic_qp(ibqp);

	return ionic_post_recv_common(dev, ctx, qp, wr, bad);
}

static struct ib_srq *ionic_create_srq(struct ib_pd *ibpd,
				       struct ib_srq_init_attr *attr,
				       struct ib_udata *udata)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibpd->device);
	struct ionic_ctx *ctx = to_ionic_ctx_uobj(ibpd->uobject);
	struct ionic_qp *qp;
	struct ionic_qp_req req;
	struct ionic_qp_resp resp = {0};
	int rc;

	if (!ctx) {
		rc = ionic_validate_udata(udata, 0, 0);
	} else {
		rc = ionic_validate_udata(udata, sizeof(req), sizeof(resp));
		if (!rc)
			rc = ib_copy_from_udata(&req, udata, sizeof(req));
	}

	if (rc)
		goto err_srq;

	qp = kmalloc(sizeof(*qp), GFP_KERNEL);
	if (!qp) {
		rc = -ENOSYS;
		goto err_srq;
	}

	rc = ionic_get_srqid(dev, &qp->qpid);
	if (rc)
		goto err_srqid;

	rc = ionic_get_mrid(dev, &qp->sq_lkey, &qp->rq_lkey);
	if (rc)
		goto err_mrid;

	qp->has_sq = false;
	qp->has_rq = true;
	qp->is_srq = true;

	spin_lock_init(&qp->rq_lock);

	ionic_qp_no_sq(qp);

	rc = ionic_qp_rq_init(dev, ctx, qp, &req.rq, attr->attr.max_wr,
			      attr->attr.max_sge);
	if (rc)
		goto err_rq;

	/* TODO need admin command */
	rc = -ENOSYS;
	goto err_cmd;

	qp->ibsrq.ext.xrc.srq_num = qp->qpid;

	return &qp->ibsrq;

err_cmd:
	ionic_qp_rq_destroy(dev, ctx, qp);
err_rq:
	ionic_put_mrid(dev, qp->sq_lkey);
err_mrid:
	ionic_put_srqid(dev, qp->qpid);
err_srqid:
	kfree(qp);
err_srq:
	return ERR_PTR(rc);
}

static int ionic_modify_srq(struct ib_srq *ibsrq, struct ib_srq_attr *attr,
			    enum ib_srq_attr_mask mask, struct ib_udata *udata)
{
	return -ENOSYS;
}

static int ionic_query_srq(struct ib_srq *ibsrq,
			   struct ib_srq_attr *attr)
{
	return -ENOSYS;
}

static int ionic_destroy_srq(struct ib_srq *ibsrq)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibsrq->device);
	struct ionic_ctx *ctx = to_ionic_ctx_uobj(ibsrq->uobject);
	struct ionic_qp *qp = to_ionic_srq(ibsrq);
	int rc;

	/* TODO: need admin command */
	rc = -ENOSYS;
	if (rc)
		return rc;

	ionic_qp_rq_destroy(dev, ctx, qp);
	ionic_put_mrid(dev, qp->sq_lkey);
	ionic_put_srqid(dev, qp->qpid);

	kfree(qp);

	return 0;
}

static int ionic_post_srq_recv(struct ib_srq *ibsrq, struct ib_recv_wr *wr,
			       struct ib_recv_wr **bad)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibsrq->device);
	struct ionic_ctx *ctx = to_ionic_ctx_uobj(ibsrq->uobject);
	struct ionic_qp *qp = to_ionic_srq(ibsrq);

	return ionic_post_recv_common(dev, ctx, qp, wr, bad);
}

static int ionic_attach_mcast(struct ib_qp *ibqp, union ib_gid *gid, u16 lid)
{
	return -ENOSYS;
}

static int ionic_detach_mcast(struct ib_qp *ibqp, union ib_gid *gid, u16 lid)
{
	return -ENOSYS;
}

static struct ib_xrcd *ionic_alloc_xrcd(struct ib_device *ibdev,
					struct ib_ucontext *ibctx,
					struct ib_udata *udata)
{
	return ERR_PTR(-ENOSYS);
}

static int ionic_dealloc_xrcd(struct ib_xrcd *xrcd)
{
	return -ENOSYS;
}

static int ionic_get_port_immutable(struct ib_device *ibdev, u8 port,
				    struct ib_port_immutable *attr)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibdev);

	if (port != 1)
		return -EINVAL;

	attr->core_cap_flags = RDMA_CORE_PORT_IBA_ROCE_UDP_ENCAP;

	attr->pkey_tbl_len = dev->port_attr.pkey_tbl_len;
	attr->gid_tbl_len = dev->port_attr.gid_tbl_len;
	attr->max_mad_size = IB_MGMT_MAD_SIZE;

	return 0;
}

static void ionic_get_dev_fw_str(struct ib_device *ibdev, char *str)
{
	str[0] = 0;
}

static const struct cpumask *ionic_get_vector_affinity(struct ib_device *ibdev,
						       int comp_vector)
{
	struct ionic_ibdev *dev = to_ionic_ibdev(ibdev);

	if (comp_vector < 0 || comp_vector >= dev->eq_count)
		return ERR_PTR(-EINVAL);

	return &dev->eq_vec[comp_vector]->cpumask;
}

static void ionic_port_event(struct ionic_ibdev *dev, enum ib_event_type event)
{
	struct ib_event ev;

	ev.device = &dev->ibdev;
	ev.element.port_num = 1;
	ev.event = event;

	ib_dispatch_event(&ev);
}

static void ionic_cq_event(struct ionic_ibdev *dev, u32 cqid, u8 code)
{
	struct ib_event ibev;
	struct ionic_cq *cq;

	rcu_read_lock();

	cq = tbl_lookup(&dev->cq_tbl, cqid);
	if (!cq) {
		dev_dbg(&dev->ibdev.dev, "missing cqid %#x code %u\n",
			cqid, code);
		goto out;
	}

	ibev.device = &dev->ibdev;
	ibev.element.cq = &cq->ibcq;

	switch(code) {
	case IONIC_EQE_CODE_CQ_COMP:
		cq->ibcq.comp_handler(&cq->ibcq, cq->ibcq.cq_context);
		goto out;

	case IONIC_EQE_CODE_CQ_ERR:
		ibev.event = IB_EVENT_CQ_ERR;
		break;

	default:
		dev_dbg(&dev->ibdev.dev, "unrecognized cqid %#x code %u\n",
			cqid, code);
		goto out;
	}

	cq->ibcq.event_handler(&ibev, cq->ibcq.cq_context);

out:
	rcu_read_unlock();
}

static bool ionic_next_eqe(struct ionic_eq *eq, struct ionic_eqe *eqe)
{
	struct ionic_eqe *qeqe;
	u32 event;
	bool color;

	qeqe = ionic_queue_at_prod(&eq->q);
	event = le32_to_cpu(qeqe->event);
	color = ionic_eqe_color(event);

	if (ionic_queue_color(&eq->q) != color)
		return false;

	*eqe = *qeqe;

	dev_dbg(&eq->dev->ibdev.dev, "poll eq prod %d\n", eq->q.prod);
	print_hex_dump_debug("eqe ", DUMP_PREFIX_OFFSET, 16, 1,
			     eqe, eq->q.stride, true);

	return true;
}

static u16 ionic_poll_eq(struct ionic_eq *eq, u16 budget)
{
	struct ionic_ibdev *dev = eq->dev;
	struct ionic_eqe eqe;
	u32 event, qid;
	u8 cls, code;
	u16 old_prod;
	u16 npolled = 0;

	old_prod = eq->q.prod;

	while (npolled < budget) {
		if (!ionic_next_eqe(eq, &eqe))
			break;

		ionic_queue_produce(&eq->q);
		ionic_queue_color_wrap(&eq->q);

		++npolled;

		event = le32_to_cpu(eqe.event);
		qid = ionic_eqe_qid(event);
		code = ionic_eqe_code(event);
		cls = ionic_eqe_cls(event);

		switch (cls) {
		case IONIC_EQE_CLS_CQ:
			ionic_cq_event(dev, qid, code);
			break;

		case IONIC_EQE_CLS_QP:
			/* TODO - fall thru default case for now */

		case IONIC_EQE_CLS_SRQ:
			/* TODO - fall thru default case for now */

		case IONIC_EQE_CLS_PORT:
			/* TODO - fall thru default case for now */

		case IONIC_EQE_CLS_DEV:
			/* TODO - fall thru default case for now */

		default:
			dev_dbg(&dev->ibdev.dev,
				"unrecognized event %#x cls %u\n",
				event, cls);
		}
	}

	if (old_prod != eq->q.prod)
		ionic_dbell_ring(&dev->dbpage[dev->eq_qtype],
				 ionic_queue_dbell_val(&eq->q));

	return npolled;
}

static void ionic_poll_eq_work(struct work_struct *work)
{
	struct ionic_eq *eq = container_of(work, struct ionic_eq, work);
	int npolled;

	if (unlikely(!eq->enable) || WARN_ON(eq->armed))
		return;

	npolled = ionic_poll_eq(eq, ionic_eq_work_budget);

	if (npolled) {
		ionic_intr_credits(eq->dev, eq->intr, npolled);
		queue_work(ionic_workq, &eq->work);
	} else {
		xchg(&eq->armed, true);
		ionic_intr_credits(eq->dev, eq->intr, IONIC_INTR_CRED_UNMASK);
	}
}

static irqreturn_t ionic_poll_eq_isr(int irq, void *eqptr)
{
	struct ionic_eq *eq = eqptr;
	u16 npolled;
	bool was_armed;

	was_armed = xchg(&eq->armed, false);

	if (unlikely(!eq->enable) || !was_armed)
		return IRQ_HANDLED;

	npolled = ionic_poll_eq(eq, ionic_eq_isr_budget);

	ionic_intr_credits(eq->dev, eq->intr, npolled);
	queue_work(ionic_workq, &eq->work);

	return IRQ_HANDLED;
}

static int ionic_create_eq_cmd(struct ionic_ibdev *dev, struct ionic_eq *eq)
{
	struct ionic_admin_ctx admin = {
		.work = COMPLETION_INITIALIZER_ONSTACK(admin.work),
		/* XXX endian? */
		.cmd.create_eq = {
			.opcode = CMD_OPCODE_RDMA_CREATE_EQ,
			.intr = eq->intr,
			/* XXX lif should be dbid */
			.lif_id = dev->lif_id,
			.log_depth = order_base_2(eq->q.mask),
			.log_stride = order_base_2(eq->q.stride),
			.dma_addr = eq->q.dma,
		},
	};
	int rc;

	rc = ionic_api_adminq_post(dev->lif, &admin);
	if (rc)
		goto err_cmd;

	wait_for_completion(&admin.work);

	if (0 /* XXX did the queue fail? */) {
		rc = -EIO;
		goto err_cmd;
	}

	rc = ionic_verbs_status_to_rc(admin.comp.comp.status);

err_cmd:
	return rc;
}

static int ionic_destroy_eq_cmd(struct ionic_ibdev *dev,
				struct ionic_eq *eq)
{
#if 1
	/* need destroy_eq admin command */
	return -ENOSYS;
#else
	struct ionic_admin_ctx admin = {
		.work = COMPLETION_INITIALIZER_ONSTACK(admin.work),
		/* XXX endian? */
		.cmd.destroy_eq = {
			.opcode = CMD_OPCODE_RDMA_DESTROY_EQ,
			.intr = eq->intr,
		},
	};
	int rc;

	rc = ionic_api_adminq_post(dev->lif, &admin);
	if (rc)
		goto err_cmd;

	wait_for_completion(&admin.work);

	if (0 /* XXX did the queue fail? */) {
		rc = -EIO;
		goto err_cmd;
	}

	rc = ionic_verbs_status_to_rc(admin.comp.comp.status);

err_cmd:
	return rc;
#endif
}

static struct ionic_eq *ionic_create_eq(struct ionic_ibdev *dev,
					int vec, int cpu)
{
	struct ionic_eq *eq;
	int rc;

	eq = kmalloc(sizeof(*eq), GFP_KERNEL);
	if (!eq) {
		rc = -ENOMEM;
		goto err_eq;
	}

	eq->vec = vec;
	eq->cpu = cpu;

	eq->armed = true;
	eq->enable = false;
	INIT_WORK(&eq->work, ionic_poll_eq_work);

	rc = ionic_queue_init(&eq->q, dev->hwdev, ionic_eq_depth,
			      sizeof(struct ionic_eqe));
	if (rc)
		goto err_q;

	eq->intr = ionic_api_get_intr(dev->lif, &eq->irq);
	if (eq->intr < 0) {
		rc = eq->intr;
		goto err_intr;
	}

	ionic_queue_dbell_init(&eq->q, eq->intr);

	snprintf(eq->name, sizeof(eq->name), "%s-%d-%d-eq",
		 DRIVER_NAME, dev->lif_id, eq->intr);

	ionic_intr_mask(dev, eq->intr, IONIC_INTR_MASK_SET);
	ionic_intr_mask_assert(dev, eq->intr, IONIC_INTR_MASK_SET);
	ionic_intr_coalesce_init(dev, eq->intr, 0);
	ionic_intr_credits(dev, eq->intr, 0);

	eq->enable = true;

	rc = request_irq(eq->irq, ionic_poll_eq_isr, 0, eq->name, eq);
	if (rc)
		goto err_irq;

	cpumask_clear(&eq->cpumask);
	cpumask_set_cpu(cpu, &eq->cpumask);
	rc = irq_set_affinity_hint(eq->irq, &eq->cpumask);
	if (rc)
		dev_dbg(dev->hwdev, "rejected affinity hint %d\n", rc);

	rc = ionic_create_eq_cmd(dev, eq);
	if (rc)
		goto err_cmd;

	ionic_intr_mask(dev, eq->intr, IONIC_INTR_MASK_CLEAR);

	ionic_dbgfs_add_eq(dev, eq);

	return eq;

err_cmd:
	irq_set_affinity_hint(eq->irq, NULL);
	free_irq(eq->irq, eq);
	eq->enable = false;
	flush_work(&eq->work);
err_irq:
	ionic_api_put_intr(dev->lif, eq->intr);
err_intr:
	ionic_queue_destroy(&eq->q, dev->hwdev);
err_q:
	kfree(eq);
err_eq:
	return ERR_PTR(rc);
}

static int ionic_destroy_eq(struct ionic_eq *eq)
{
	struct ionic_ibdev *dev = eq->dev;
	int rc;

	ionic_dbgfs_rm_eq(eq);

	if (eq->enable) {
		irq_set_affinity_hint(eq->irq, NULL);
		free_irq(eq->irq, eq);
		eq->enable = false;
		flush_work(&eq->work);
	}

	disable_irq(eq->irq);
	eq->enable = false;
	flush_work(&eq->work);

	rc = ionic_destroy_eq_cmd(dev, eq);
	if (rc)
		return rc;

	ionic_api_put_intr(dev->lif, eq->intr);
	ionic_queue_destroy(&eq->q, dev->hwdev);
	kfree(eq);

	return 0;
}

static void ionic_destroy_eqvec(struct ionic_ibdev *dev)
{
	struct ionic_eq *eq;
	int rc;

	while (dev->eq_count > 0) {
		eq = dev->eq_vec[--dev->eq_count];
		rc = ionic_destroy_eq(eq);

		/* warning: eq is disabled, but will leak eq, irq, and intr */
		WARN_ON(rc);
	}

	kfree(dev->eq_vec);
}

static int ionic_create_eqvec(struct ionic_ibdev *dev)
{
	struct ionic_eq *eq;
	int rc, cpu, vec = 0;

	dev->eq_count = vec;
	dev->eq_vec = kmalloc_array(num_possible_cpus(), sizeof(*dev->eq_vec),
				    GFP_KERNEL);
	if (!dev->eq_vec) {
		rc = -ENOMEM;
		goto err_eqvec;
	}

	for_each_possible_cpu(cpu) {
		eq = ionic_create_eq(dev, vec, cpu);
		if (IS_ERR(eq)) {
			rc = PTR_ERR(eq);
			/* XXX allow zero eq for now */
			if (0 && !dev->eq_count)
				goto err_eq;

			/* ok, just fewer eq than ncpu */
			dev_dbg(dev->hwdev,
				"fewer than ncpu eq count %d rc %d\n",
				dev->eq_count, rc);
			goto out;
		}

		dev->eq_vec[vec] = eq;
		dev->eq_count = ++vec;
	}

out:
	return 0;

err_eq:
	ionic_destroy_eqvec(dev);
err_eqvec:
	return rc;
}

static void ionic_destroy_ibdev(struct ionic_ibdev *dev)
{
	struct net_device *ndev = dev->ndev;

	list_del(&dev->driver_ent);

	ib_unregister_device(&dev->ibdev);

	ionic_destroy_eqvec(dev);

	ionic_dbgfs_rm_dev(dev);

	kfree(dev->inuse_qpid);
	kfree(dev->inuse_cqid);
	kfree(dev->inuse_pgtbl);
	kfree(dev->inuse_mrid);
	kfree(dev->inuse_pdid);

	tbl_destroy(&dev->qp_tbl);
	tbl_destroy(&dev->cq_tbl);

	ib_dealloc_device(&dev->ibdev);

	dev_put(ndev);
}

static struct ionic_ibdev *ionic_create_ibdev(struct lif *lif,
					      struct net_device *ndev)
{
	struct ib_device *ibdev;
	struct ionic_ibdev *dev;
	const union identity *ident;
	struct dentry *lif_dbgfs;
	size_t size;
	int rc;

	dev_hold(ndev);

	ibdev = ib_alloc_device(sizeof(*dev));
	if (!ibdev) {
		rc = -ENOMEM;
		goto err_dev;
	}

	dev = to_ionic_ibdev(ibdev);
	dev->hwdev = ndev->dev.parent;
	dev->ndev = ndev;
	dev->lif = lif;

	ident = ionic_api_get_identity(lif, &dev->lif_id);

	ionic_api_get_dbpages(lif, &dev->dbid, &dev->dbpage,
			      &dev->phys_dbpage_base,
			      &dev->intr_ctrl);

	/* XXX hardcode values, should come from identify */
	dev->sq_qtype = 3;
	dev->rq_qtype = 4;
	dev->cq_qtype = 5;
	dev->eq_qtype = 6;

	/* XXX hardcode values, intentionally low, should come from identify */
	dev->dev_attr.fw_ver = 0;
	dev->dev_attr.sys_image_guid = 0;
	dev->dev_attr.max_mr_size = 0x10000 * PAGE_SIZE; /* XXX ident->dev.pgtbl_size*PAGE_SIZE */
	dev->dev_attr.page_size_cap = 0x1000;
	dev->dev_attr.vendor_id = 0;
	dev->dev_attr.vendor_part_id = 0;
	dev->dev_attr.hw_ver = 0;
	dev->dev_attr.max_qp = ident->dev.nrdmasqs_per_lif;
	dev->dev_attr.max_qp_wr = 0xffff;
	dev->dev_attr.device_cap_flags =
		//IB_DEVICE_LOCAL_DMA_LKEY |
		//IB_DEVICE_MEM_WINDOW |
		//IB_DEVICE_XRC |
		//IB_DEVICE_MEM_MGMT_EXTENSIONS |
		//IB_DEVICE_MEM_WINDOW_TYPE_2A |
		//IB_DEVICE_MEM_WINDOW_TYPE_2B |
		0;
	dev->dev_attr.max_sge = 6;
	dev->dev_attr.max_sge_rd = 7;
	dev->dev_attr.max_cq = ident->dev.ncqs_per_lif;
	dev->dev_attr.max_cqe = 0xffff;
	dev->dev_attr.max_mr = 4096; /* XXX need from identify */
	dev->dev_attr.max_pd = 0x10000; /* XXX only limited by the size of bitset we can alloc */
	dev->dev_attr.max_qp_rd_atom = 0;
	dev->dev_attr.max_ee_rd_atom = 0;
	dev->dev_attr.max_res_rd_atom = 0;
	dev->dev_attr.max_qp_init_rd_atom = 0;
	dev->dev_attr.max_ee_init_rd_atom = 0;
	dev->dev_attr.atomic_cap = IB_ATOMIC_HCA; /* XXX or global? */
	dev->dev_attr.masked_atomic_cap = IB_ATOMIC_HCA; /* XXX or global? */
	dev->dev_attr.max_mw = 0; /* XXX same as max_mr */
	dev->dev_attr.max_mcast_grp = 0;
	dev->dev_attr.max_mcast_qp_attach = 0;
	dev->dev_attr.max_ah = 4096; /* XXX need from identify */
	dev->dev_attr.max_srq = ident->dev.nrdmarqs_per_lif;
	dev->dev_attr.max_srq_wr = 0xffff;
	dev->dev_attr.max_srq_sge = 7;
	dev->dev_attr.max_fast_reg_page_list_len = 0; /* XXX ident->dev.pgtbl_size */
	dev->dev_attr.max_pkeys = 1;

	/* XXX hardcode values, intentionally low, should come from identify */
	if (netif_running(ndev) && netif_carrier_ok(ndev)) {
		dev->port_attr.state = IB_PORT_ACTIVE;
		dev->port_attr.phys_state = PHYS_STATE_UP;
	} else {
		dev->port_attr.state = IB_PORT_DOWN;
		dev->port_attr.phys_state = PHYS_STATE_DOWN;
	}
	dev->port_attr.max_mtu = ib_mtu_int_to_enum(ndev->max_mtu);
	dev->port_attr.active_mtu = ib_mtu_int_to_enum(ndev->mtu);
	dev->port_attr.gid_tbl_len = 4096; /* XXX same as max_ah, or unlimited? */
	dev->port_attr.port_cap_flags = IB_PORT_IP_BASED_GIDS;
	dev->port_attr.max_msg_sz = 0x80000000;
	dev->port_attr.pkey_tbl_len = 1;
	dev->port_attr.max_vl_num = 1;
	dev->port_attr.subnet_prefix = 0xfe80000000000000ull;

	/* XXX workarounds and overrides, remove for release */
	if (ident->dev.ndbpgs_per_lif < 2)
		ionic_xxx_kdbid = true;
	if (ionic_xxx_limits) {
		dev->dev_attr.max_qp = 20;
		dev->dev_attr.max_cq = 40;
		dev->dev_attr.max_srq = 40;
	} else {
		if (!dev->dev_attr.max_qp)
			dev->dev_attr.max_qp = 40;
		if (!dev->dev_attr.max_cq)
			dev->dev_attr.max_cq = dev->dev_attr.max_qp;
		if (!dev->dev_attr.max_srq)
			dev->dev_attr.max_srq = dev->dev_attr.max_qp;
	}

	mutex_init(&dev->tbl_lock);

	tbl_init(&dev->qp_tbl);
	tbl_init(&dev->cq_tbl);

	mutex_init(&dev->inuse_lock);

	dev->size_pdid = dev->dev_attr.max_pd;
	dev->next_pdid = 0;
	size = sizeof(long) * BITS_TO_LONGS(dev->size_pdid);
	dev->inuse_pdid = kzalloc(size, GFP_KERNEL);
	if (!dev->inuse_pdid) {
		rc = -ENOMEM;
		goto err_pdid;
	}

	dev->size_mrid = dev->dev_attr.max_mr;
	dev->next_mrid = 0;
	dev->next_rkey_key = 0;
	size = sizeof(long) * BITS_TO_LONGS(dev->size_mrid);
	dev->inuse_mrid = kzalloc(size, GFP_KERNEL);
	if (!dev->inuse_mrid) {
		rc = -ENOMEM;
		goto err_mrid;
	}

	dev->size_cqid = dev->dev_attr.max_cq;
	dev->next_cqid = 0;
	size = sizeof(long) * BITS_TO_LONGS(dev->size_cqid);
	dev->inuse_cqid = kzalloc(size, GFP_KERNEL);
	if (!dev->inuse_cqid) {
		rc = -ENOMEM;
		goto err_cqid;
	}

	dev->size_qpid = dev->dev_attr.max_qp;
	dev->next_qpid = 2;
	dev->size_srqid = dev->dev_attr.max_srq;
	dev->next_srqid = dev->dev_attr.max_qp;
	size = sizeof(long) * BITS_TO_LONGS(dev->size_srqid);
	dev->inuse_qpid = kzalloc(size, GFP_KERNEL);
	if (!dev->inuse_qpid) {
		rc = -ENOMEM;
		goto err_qpid;
	}

	dev->size_pgtbl = dev->dev_attr.max_mr_size / PAGE_SIZE;
	size = sizeof(long) * BITS_TO_LONGS(dev->size_pgtbl);
	dev->inuse_pgtbl = kzalloc(size, GFP_KERNEL);
	if (!dev->inuse_pgtbl) {
		rc = -ENOMEM;
		goto err_pgtbl;
	}

	if (ionic_dbgfs_enable)
		lif_dbgfs = ionic_api_get_debugfs(lif);
	else
		lif_dbgfs = NULL;

	ionic_dbgfs_add_dev(dev, lif_dbgfs);

	rc = ionic_create_eqvec(dev);
	if (rc)
		goto err_eqvec;

	ibdev->owner = THIS_MODULE;
	ibdev->dev.parent = dev->hwdev;

	strlcpy(ibdev->name, "ionic_%d", IB_DEVICE_NAME_MAX);
	strlcpy(ibdev->node_desc, DEVICE_DESCRIPTION, IB_DEVICE_NODE_DESC_MAX);

	ibdev->node_type = RDMA_NODE_IB_CA;
	ibdev->phys_port_cnt = 1;
	ibdev->num_comp_vectors = max(1, dev->eq_count);
	/* XXX should not use max, but count may be zero: see ionic_create_eqvec */

	addrconf_ifid_eui48((u8 *)&ibdev->node_guid, ndev);

	ibdev->uverbs_abi_ver = IONIC_ABI_VERSION;
	ibdev->uverbs_cmd_mask =
		BIT_ULL(IB_USER_VERBS_CMD_GET_CONTEXT)		|
		BIT_ULL(IB_USER_VERBS_CMD_QUERY_DEVICE)		|
		BIT_ULL(IB_USER_VERBS_CMD_QUERY_PORT)		|
		BIT_ULL(IB_USER_VERBS_CMD_ALLOC_PD)		|
		BIT_ULL(IB_USER_VERBS_CMD_DEALLOC_PD)		|
		BIT_ULL(IB_USER_VERBS_CMD_CREATE_AH)		|
		BIT_ULL(IB_USER_VERBS_CMD_MODIFY_AH)		|
		BIT_ULL(IB_USER_VERBS_CMD_QUERY_AH)		|
		BIT_ULL(IB_USER_VERBS_CMD_DESTROY_AH)		|
		BIT_ULL(IB_USER_VERBS_CMD_REG_MR)		|
		BIT_ULL(IB_USER_VERBS_CMD_REG_SMR)		|
		BIT_ULL(IB_USER_VERBS_CMD_REREG_MR)		|
		BIT_ULL(IB_USER_VERBS_CMD_DEREG_MR)		|
		BIT_ULL(IB_USER_VERBS_CMD_ALLOC_MW)		|
		BIT_ULL(IB_USER_VERBS_CMD_BIND_MW)		|
		BIT_ULL(IB_USER_VERBS_CMD_DEALLOC_MW)		|
		BIT_ULL(IB_USER_VERBS_CMD_CREATE_COMP_CHANNEL)	|
		BIT_ULL(IB_USER_VERBS_CMD_CREATE_CQ)		|
		BIT_ULL(IB_USER_VERBS_CMD_RESIZE_CQ)		|
		BIT_ULL(IB_USER_VERBS_CMD_DESTROY_CQ)		|
		BIT_ULL(IB_USER_VERBS_CMD_POLL_CQ)		|
		BIT_ULL(IB_USER_VERBS_CMD_PEEK_CQ)		|
		BIT_ULL(IB_USER_VERBS_CMD_REQ_NOTIFY_CQ)	|
		BIT_ULL(IB_USER_VERBS_CMD_CREATE_QP)		|
		BIT_ULL(IB_USER_VERBS_CMD_QUERY_QP)		|
		BIT_ULL(IB_USER_VERBS_CMD_MODIFY_QP)		|
		BIT_ULL(IB_USER_VERBS_CMD_DESTROY_QP)		|
		BIT_ULL(IB_USER_VERBS_CMD_POST_SEND)		|
		BIT_ULL(IB_USER_VERBS_CMD_POST_RECV)		|
		BIT_ULL(IB_USER_VERBS_CMD_CREATE_SRQ)		|
		BIT_ULL(IB_USER_VERBS_CMD_MODIFY_SRQ)		|
		BIT_ULL(IB_USER_VERBS_CMD_QUERY_SRQ)		|
		BIT_ULL(IB_USER_VERBS_CMD_DESTROY_SRQ)		|
		BIT_ULL(IB_USER_VERBS_CMD_POST_SRQ_RECV)	|
		BIT_ULL(IB_USER_VERBS_CMD_OPEN_XRCD)		|
		BIT_ULL(IB_USER_VERBS_CMD_CLOSE_XRCD)		|
		BIT_ULL(IB_USER_VERBS_CMD_CREATE_XSRQ)		|
		BIT_ULL(IB_USER_VERBS_CMD_OPEN_QP)		|
		0;
	ibdev->uverbs_ex_cmd_mask =
		BIT_ULL(IB_USER_VERBS_EX_CMD_CREATE_QP)		|
		BIT_ULL(IB_USER_VERBS_EX_CMD_MODIFY_QP)		|
		0;

	dev->ibdev.alloc_hw_stats	= ionic_alloc_hw_stats;
	dev->ibdev.get_hw_stats		= ionic_get_hw_stats;

	dev->ibdev.query_device		= ionic_query_device;
	dev->ibdev.query_port		= ionic_query_port;
	dev->ibdev.get_link_layer	= ionic_get_link_layer;
	dev->ibdev.get_netdev		= ionic_get_netdev;
	dev->ibdev.query_gid		= ionic_query_gid;
	dev->ibdev.add_gid		= ionic_add_gid;
	dev->ibdev.del_gid		= ionic_del_gid;
	dev->ibdev.query_pkey		= ionic_query_pkey;
	dev->ibdev.modify_device	= ionic_modify_device;
	dev->ibdev.modify_port		= ionic_modify_port;

	dev->ibdev.alloc_ucontext	= ionic_alloc_ucontext;
	dev->ibdev.dealloc_ucontext	= ionic_dealloc_ucontext;
	dev->ibdev.mmap			= ionic_mmap;

	dev->ibdev.alloc_pd		= ionic_alloc_pd;
	dev->ibdev.dealloc_pd		= ionic_dealloc_pd;

	dev->ibdev.create_ah		= ionic_create_ah;
	dev->ibdev.modify_ah		= ionic_modify_ah;
	dev->ibdev.query_ah		= ionic_query_ah;
	dev->ibdev.destroy_ah		= ionic_destroy_ah;

	dev->ibdev.get_dma_mr		= ionic_get_dma_mr;
	dev->ibdev.reg_user_mr		= ionic_reg_user_mr;
	dev->ibdev.rereg_user_mr	= ionic_rereg_user_mr;
	dev->ibdev.dereg_mr		= ionic_dereg_mr;
	dev->ibdev.alloc_mr		= ionic_alloc_mr;
	dev->ibdev.map_mr_sg		= ionic_map_mr_sg;

	dev->ibdev.alloc_mw		= ionic_alloc_mw;
	dev->ibdev.dealloc_mw		= ionic_dealloc_mw;

	dev->ibdev.create_cq		= ionic_create_cq;
	dev->ibdev.destroy_cq		= ionic_destroy_cq;
	dev->ibdev.resize_cq		= ionic_resize_cq;
	dev->ibdev.poll_cq		= ionic_poll_cq;
	dev->ibdev.peek_cq		= ionic_peek_cq;
	dev->ibdev.req_notify_cq	= ionic_req_notify_cq;

	dev->ibdev.create_qp		= ionic_create_qp;
	dev->ibdev.modify_qp		= ionic_modify_qp;
	dev->ibdev.query_qp		= ionic_query_qp;
	dev->ibdev.destroy_qp		= ionic_destroy_qp;
	dev->ibdev.post_send		= ionic_post_send;
	dev->ibdev.post_recv		= ionic_post_recv;

	dev->ibdev.create_srq		= ionic_create_srq;
	dev->ibdev.modify_srq		= ionic_modify_srq;
	dev->ibdev.query_srq		= ionic_query_srq;
	dev->ibdev.destroy_srq		= ionic_destroy_srq;
	dev->ibdev.post_srq_recv	= ionic_post_srq_recv;

	dev->ibdev.attach_mcast		= ionic_attach_mcast;
	dev->ibdev.detach_mcast		= ionic_detach_mcast;

	dev->ibdev.alloc_xrcd		= ionic_alloc_xrcd;
	dev->ibdev.dealloc_xrcd		= ionic_dealloc_xrcd;

	dev->ibdev.get_port_immutable	= ionic_get_port_immutable;
	dev->ibdev.get_dev_fw_str	= ionic_get_dev_fw_str;
	dev->ibdev.get_vector_affinity	= ionic_get_vector_affinity;

	rc = ib_register_device(ibdev, NULL);
	if (rc)
		goto err_register;

	list_add(&dev->driver_ent, &ionic_ibdev_list);

	return dev;

err_register:
	ionic_destroy_eqvec(dev);
err_eqvec:
	ionic_dbgfs_rm_dev(dev);
	kfree(dev->inuse_pgtbl);
err_pgtbl:
	kfree(dev->inuse_qpid);
err_qpid:
	kfree(dev->inuse_cqid);
err_cqid:
	kfree(dev->inuse_mrid);
err_mrid:
	kfree(dev->inuse_pdid);
err_pdid:
	tbl_destroy(&dev->qp_tbl);
	tbl_destroy(&dev->cq_tbl);
	ib_dealloc_device(ibdev);
err_dev:
	dev_put(ndev);
	return ERR_PTR(rc);
}

struct ionic_netdev_work {
	struct work_struct ws;
	unsigned long event;
	struct net_device *ndev;
	struct lif *lif;
};

static void ionic_netdev_work(struct work_struct *ws)
{
	struct ionic_netdev_work *work =
		container_of(ws, struct ionic_netdev_work, ws);
	struct net_device *ndev = work->ndev;
	struct ionic_ibdev *dev;
	int rc;

	dev = ionic_api_get_private(work->lif, IONIC_RDMA_PRIVATE);

	switch (work->event) {
	case NETDEV_REGISTER:
		if (dev) {
			dev_dbg(&ndev->dev, "already registered\n");
			break;
		}

		dev_dbg(&ndev->dev, "register ibdev\n");

		dev = ionic_create_ibdev(work->lif, ndev);
		if (IS_ERR(dev)) {
			dev_dbg(&ndev->dev, "error register ibdev %d\n",
				(int)PTR_ERR(dev));
			break;
		}

		rc = ionic_api_set_private(work->lif, dev, IONIC_RDMA_PRIVATE);
		if (rc) {
			dev_dbg(&ndev->dev, "error set private %d\n", rc);
			ionic_destroy_ibdev(dev);
		}

		break;

	case NETDEV_UNREGISTER:
		if (!dev) {
			dev_dbg(&ndev->dev, "not registered\n");
			break;
		}

		dev_dbg(&ndev->dev, "unregister ibdev\n");

		ionic_api_set_private(work->lif, NULL, IONIC_RDMA_PRIVATE);
		ionic_destroy_ibdev(dev);

		break;

	case NETDEV_UP:
		if (!dev)
			break;

		dev->port_attr.state = IB_PORT_ACTIVE;
		dev->port_attr.phys_state = PHYS_STATE_UP;
		ionic_port_event(dev, IB_EVENT_PORT_ACTIVE);

		break;

	case NETDEV_DOWN:
		if (!dev)
			break;

		dev->port_attr.state = IB_PORT_DOWN;
		dev->port_attr.phys_state = PHYS_STATE_DOWN;
		ionic_port_event(dev, IB_EVENT_PORT_ERR);

		break;

	case NETDEV_CHANGE:
		if (!dev)
			break;

		if (netif_running(ndev) && netif_carrier_ok(ndev)) {
			dev->port_attr.state = IB_PORT_ACTIVE;
			dev->port_attr.phys_state = PHYS_STATE_UP;
			ionic_port_event(dev, IB_EVENT_PORT_ACTIVE);
		} else {
			dev->port_attr.state = IB_PORT_DOWN;
			dev->port_attr.phys_state = PHYS_STATE_DOWN;
			ionic_port_event(dev, IB_EVENT_PORT_ERR);
		}

		break;

	case NETDEV_CHANGEMTU:
		if (!dev)
			break;

		dev->port_attr.active_mtu = ib_mtu_int_to_enum(ndev->mtu);

		break;

	default:
		if (!dev)
			break;

		dev_dbg(&ndev->dev, "unhandled event %lu\n", work->event);
	}

	dev_put(ndev);
	kfree(work);

	module_put(THIS_MODULE);
}

static int ionic_netdev_event(struct notifier_block *notifier,
			      unsigned long event, void *ptr)
{
	struct ionic_netdev_work *work;
	struct net_device *ndev;
	struct lif *lif;
	int rc;

	ndev = netdev_notifier_info_to_dev(ptr);

	lif = get_netdev_ionic_lif(ndev, IONIC_API_VERSION);
	if (!lif) {
		pr_devel("unrecognized netdev: %s (%s)\n",
			 ndev->name, ndev->name);
		goto out;
	}

	pr_devel("ionic netdev: %s\n", ndev->name);
	dev_dbg(&ndev->dev, "event %lu\n", event);

	if (!try_module_get(THIS_MODULE))
		goto out;

	work = kmalloc(sizeof(*work), GFP_ATOMIC);
	if (WARN_ON_ONCE(!work)) {
		rc = -ENOMEM;
		goto err_work;
	}

	dev_hold(ndev);

	INIT_WORK(&work->ws, ionic_netdev_work);
	work->event = event;
	work->ndev = ndev;
	work->lif = lif;

	queue_work_on(0, ionic_workq, &work->ws);

out:
	return NOTIFY_DONE;

err_work:
	module_put(THIS_MODULE);
	return notifier_from_errno(rc);
}

static struct notifier_block ionic_netdev_notifier = {
	.notifier_call = ionic_netdev_event,
};

static int __init ionic_mod_init(void)
{
	int rc;

	pr_info("%s v%s : %s\n", DRIVER_NAME, DRIVER_VERSION, DRIVER_DESCRIPTION);

	ionic_workq = create_workqueue(DRIVER_NAME);
	if (!ionic_workq) {
		rc = -ENOMEM;
		goto err_workq;
	}

	rc = register_netdevice_notifier(&ionic_netdev_notifier);
	if (rc)
		goto err_notifier;

	return 0;

err_notifier:
	destroy_workqueue(ionic_workq);
err_workq:
	return rc;
}

static void __exit ionic_exit_work(struct work_struct *ws)
{
	struct ionic_ibdev *dev, *dev_next;

	list_for_each_entry_safe_reverse(dev, dev_next, &ionic_ibdev_list,
					 driver_ent) {
		ionic_api_set_private(dev->lif, NULL, IONIC_RDMA_PRIVATE);
		ionic_destroy_ibdev(dev);
	}
}

static void __exit ionic_mod_exit(void)
{
	struct work_struct ws;

	unregister_netdevice_notifier(&ionic_netdev_notifier);

	INIT_WORK_ONSTACK(&ws, ionic_exit_work);
	queue_work_on(0, ionic_workq, &ws);
	flush_work(&ws);
	destroy_work_on_stack(&ws);

	destroy_workqueue(ionic_workq);
}

module_init(ionic_mod_init);
module_exit(ionic_mod_exit);
