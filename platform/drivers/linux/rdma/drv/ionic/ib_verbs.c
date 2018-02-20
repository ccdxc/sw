/*
 * Description: IB Verbs interpreter
 */

#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/if_ether.h>
#include <linux/wait.h>

#include <rdma/ib_verbs.h>
#include <rdma/ib_user_verbs.h>
#include <rdma/ib_umem.h>
#include <rdma/ib_addr.h>
#include <rdma/ib_mad.h>
#include <rdma/ib_cache.h>

#include "ionic_ib.h"
//#include "ionic_dev.h"
#include "ib_verbs.h"
//#include <rdma/ionic-abi.h>
#include "../../include/ionic-abi.h"

//global key for memory regions
u32 g_key = 0;
u32 cq_num = 0;
u32 g_qp_num = 2; //0 & 1 are reserved

/*
 * For now we are hard coding to some dummy values.
 */
int ionic_query_device(struct ib_device *ibdev,
                     struct ib_device_attr *ib_attr,
                     struct ib_udata *udata)
{
	struct ionic_ib_dev *rdev = to_ionic_ib_dev(ibdev);

    pr_info("\n%s:", __FUNCTION__);
    
	memset(ib_attr, 0, sizeof(*ib_attr));

	ib_attr->fw_ver = (u64)(unsigned long)(rdev->ionic->ident->dev.fw_version);

	ib_attr->max_mr_size = IONIC_MAX_MR_SIZE;
	ib_attr->page_size_cap = IONIC_PAGE_SIZE_4K;

	ib_attr->vendor_id = rdev->pdev->vendor;
	ib_attr->vendor_part_id = rdev->pdev->device;
	ib_attr->hw_ver = rdev->pdev->subsystem_device;

	ib_attr->max_qp = IONIC_MAX_QP_COUNT;
	ib_attr->max_qp_wr = IONIC_MAX_QP_WQE;
    
	ib_attr->device_cap_flags =
        IB_DEVICE_CURR_QP_STATE_MOD
        | IB_DEVICE_RC_RNR_NAK_GEN
        | IB_DEVICE_SHUTDOWN_PORT
        | IB_DEVICE_SYS_IMAGE_GUID
        | IB_DEVICE_LOCAL_DMA_LKEY
        | IB_DEVICE_RESIZE_MAX_WR
        | IB_DEVICE_PORT_ACTIVE_EVENT
        | IB_DEVICE_N_NOTIFY_CQ;

	ib_attr->max_sge = IONIC_MAX_SGE;
    
	ib_attr->max_cq = IONIC_MAX_CQ_COUNT;
	ib_attr->max_cqe = IONIC_MAX_QP_WQE;
    
	ib_attr->max_mr = IONIC_MAX_MRW_COUNT;
	ib_attr->max_pd = IONIC_MAX_PD;
    
	ib_attr->max_qp_rd_atom = IONIC_MAX_RD_ATOM;
	ib_attr->max_qp_init_rd_atom = IONIC_MAX_RD_ATOM;

    ib_attr->atomic_cap = IB_ATOMIC_HCA;

	ib_attr->max_mw = IONIC_MAX_MRW_COUNT;

	ib_attr->max_srq = IONIC_MAX_SRQ_COUNT;
	ib_attr->max_srq_wr = IONIC_MAX_QP_WQE;
	ib_attr->max_srq_sge = IONIC_MAX_SGE;

	ib_attr->max_pkeys = IONIC_MAX_PKEY;

	return 0;
}


int ionic_query_port(struct ib_device    *ibdev,
                       u8                   port_num,
                       struct ib_port_attr *port_attr)
{
	struct ionic_ib_dev *rdev = to_ionic_ib_dev(ibdev);

    pr_info("\n%s:", __FUNCTION__);
	memset(port_attr, 0, sizeof(*port_attr));

	if (netif_running(rdev->netdev) && netif_oper_up(rdev->netdev)) {
		port_attr->state = IB_PORT_ACTIVE;
		port_attr->phys_state = 5;
	} else {
		port_attr->state = IB_PORT_DOWN;
		port_attr->phys_state = 3;
	}
	port_attr->max_mtu = IB_MTU_4096;
	port_attr->active_mtu = iboe_get_mtu(rdev->netdev->mtu);
	port_attr->gid_tbl_len = IONIC_MAX_SGID;
    
	port_attr->port_cap_flags = IB_PORT_CM_SUP | IB_PORT_REINIT_SUP |
        IB_PORT_DEVICE_MGMT_SUP |
        IB_PORT_VENDOR_CLASS_SUP |
        IB_PORT_IP_BASED_GIDS;

	/* Max MSG size set to 2G for now */
	port_attr->max_msg_sz = 0x80000000;
	port_attr->bad_pkey_cntr = 0;
	port_attr->qkey_viol_cntr = 0;
	port_attr->pkey_tbl_len = IONIC_MAX_PKEY;
	port_attr->lid = 0;
	port_attr->sm_lid = 0;
	port_attr->lmc = 0;
	port_attr->max_vl_num = 4;
	port_attr->sm_sl = 0;
	port_attr->subnet_timeout = 0;
	port_attr->init_type_reply = 0;
	port_attr->active_speed = rdev->active_speed;
	port_attr->active_width = rdev->active_width;

	return 0;
}

int ionic_query_pkey(struct ib_device *ibdev,
                       u8                port_num,
                       u16               index,
                       u16              *pkey)
{
	if (index > 1)
		return -EINVAL;

    pr_info("\n%s:", __FUNCTION__);
	*pkey = 0xffff;
	return 0;
}

/*
 * This is mandatory function, so proveded code for this. But this would not be
 * needed from user space application.
 */
int ionic_query_gid(struct ib_device *ibdev,
                     u8 port,
                     int index,
                     union ib_gid *sgid)
{
	int ret;
	struct ionic_ib_dev *dev;

    pr_info("\n%s:", __FUNCTION__);
    
	if (index >= IONIC_MAX_SGID)
		return -EINVAL;

	dev = to_ionic_ib_dev(ibdev);
	memset(sgid, 0, sizeof(*sgid));

	ret = ib_get_cached_gid(ibdev, port, index, sgid, NULL);
	if (ret == -EAGAIN) {
		memcpy(sgid, &zgid, sizeof(*sgid));
		return 0;
	}

	return ret;
}

struct ib_pd *ionic_alloc_pd(struct ib_device *ibdev,
                             struct ib_ucontext *context,
                             struct ib_udata *udata)
{
	struct ionic_pd *pd;
	struct ionic_ib_dev *dev = to_ionic_ib_dev(ibdev);
	void *ptr;

    pr_info("\n%s:", __FUNCTION__);

	/* Check allowed max pds */
	if (!atomic_add_unless(&dev->num_pds, 1, IONIC_MAX_PD))
		return ERR_PTR(-ENOMEM);

	pd = kmalloc(sizeof(*pd), GFP_KERNEL);
	if (!pd) {
		ptr = ERR_PTR(-ENOMEM);
		goto err;
	}

    /*
     * TODO: Ideally need to send alloc_pd adminQ command to Capri.
     */
    
	pd->pdn = 1;
    pd->rdev = dev;
    
	if (context) {
		if (ib_copy_to_udata(udata, &pd->pdn, sizeof(__u32))) {
            pr_err("failed to copy back protection domain\n");
			ionic_dealloc_pd(&pd->ibpd);
			return ERR_PTR(-EFAULT);
		}
	}

	/* u32 pd handle */
	return &pd->ibpd;

err:
	atomic_dec(&dev->num_pds);
	return ptr;
}

int ionic_dealloc_pd(struct ib_pd *pd)
{
	struct ionic_ib_dev *dev = to_ionic_ib_dev(pd->device);

    pr_info("\n%s:", __FUNCTION__);
	kfree(to_ionic_pd(pd));
	atomic_dec(&dev->num_pds);

	return 0;
}

struct ib_ah *ionic_create_ah(struct ib_pd *pd, struct rdma_ah_attr *ah_attr,
                               struct ib_udata *udata)
{
    pr_info("\n%s:", __FUNCTION__);
    return NULL;
}

int ionic_destroy_ah(struct ib_ah *ah)
{
    pr_info("\n%s:", __FUNCTION__);
	return 0;
}

/*
 * 
 */
struct ib_ucontext *ionic_alloc_ucontext(struct ib_device *ibdev,
                                          struct ib_udata *udata)
{
	struct ionic_ib_dev *rdev = to_ionic_ib_dev(ibdev);
	struct ionic_ucontext *context;
    struct ionic_uctx_resp resp = {0};
    int ret;
    dma_addr_t db_pages =  rdev->ionic->idev.phy_db_pages;

    pr_info("\n%s: phy_db_pages %llx", __FUNCTION__,
            rdev->ionic->idev.phy_db_pages);
	context = kmalloc(sizeof(*context), GFP_KERNEL);
	if (!context)
		return ERR_PTR(-ENOMEM);

	context->dev = rdev;

    //Ideally pid has to be diffrent for each process.
    context->pid = 1;    //pid 0 is used by ETH driver.

    //context->db_pages = (struct doorbell *) db_pages;

    // pr_info("\n%s: eth db_pages %lx rdma db_pages %lx %p page_size 0x%lx",
    //         __FUNCTION__, (unsigned long)rdev->ionic->idev.db_pages,
    //         (unsigned long)context->db_pages,
    //         context->db_pages,
    //         PAGE_SIZE);

    context->pfn = (db_pages >> PAGE_SHIFT) + context->pid;
    resp.max_qp = IONIC_MAX_QP_COUNT;
    resp.pg_size = PAGE_SIZE;
    resp.cqe_size = IONIC_CQE_SIZE;
    resp.max_cqd = IONIC_MAX_QP_WQE;

	ret = ib_copy_to_udata(udata, &resp, sizeof(resp));
	if (ret) {
        pr_err("\n%s: ib_copy_to_udata failed", __FUNCTION__);
        return ERR_PTR(ret);
    }
    
	return &context->ibucontext;
}

int ionic_dealloc_ucontext(struct ib_ucontext *ibuctx)
{
    pr_info("\n%s:", __FUNCTION__);
	kfree(ibuctx);

	return 0;
}

int ionic_mmap(struct ib_ucontext *ibcontext, struct vm_area_struct *vma)
{
	struct ionic_ucontext *context = to_ionic_ucontext(ibcontext);
	unsigned long size = vma->vm_end - vma->vm_start;
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
    
    pr_info("\n%s: pfn 0x%lx start 0x%lx size 0x%lx offset 0x%lx",
            __FUNCTION__, (unsigned long)context->pfn, vma->vm_start, size, offset);

    //pr_info("\n%s dbpages phy address 0x%llx", __FUNCTION__, virt_to_phys(context->db_pages));
	if ((size != PAGE_SIZE) || (offset & ~PAGE_MASK)) {
        pr_err("invalid params for mmap region\n");
		return -EINVAL;
	}

    //pfn = virt_to_phys(context->db_pages) >> PAGE_SHIFT;
	vma->vm_flags |= VM_DONTCOPY | VM_DONTEXPAND;
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	if (io_remap_pfn_range(vma, vma->vm_start,
                           context->pfn,
                           PAGE_SIZE,
                           vma->vm_page_prot)) {
        pr_err("Failed to map shared page");
		return -EAGAIN;
    }

#if 0
    pfn = virt_to_phys(context->db_pages) >> PAGE_SHIFT;
    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
    if (remap_pfn_range(vma, vma->vm_start,
                        pfn, PAGE_SIZE, vma->vm_page_prot)) {
        pr_err("Failed to map shared page");
        return -EAGAIN;
    }
#endif
    
	return 0;
}

// Not needed
struct ib_mr *ionic_get_dma_mr(struct ib_pd *ibpd,
                             int mr_access_flags)
{
	pr_info("\n%s:not implemented, returning %d", __FUNCTION__,-ENOSYS);
	return ERR_PTR(-ENOSYS);
}

// Not needed
int ionic_get_port_immutable(struct ib_device *ibdev,
                              u8 port,
                              struct ib_port_immutable *immutable)
{
	struct ionic_ib_dev *dev;

    pr_info("\n%s:", __FUNCTION__);
    
	dev = to_ionic_ib_dev(ibdev);
	immutable->core_cap_flags = RDMA_CORE_PORT_IBA_ROCE | RDMA_CORE_CAP_PROT_ROCE_UDP_ENCAP;

	immutable->pkey_tbl_len = IONIC_MAX_PKEY;
	immutable->gid_tbl_len = IONIC_MAX_SGID;
	immutable->max_mad_size = IB_MGMT_MAD_SIZE;

	return 0;
}

struct ib_mr *ionic_reg_user_mr(struct ib_pd *ibpd, u64 start, u64 length,
                                u64 virt_addr, int mr_access_flags,
                                struct ib_udata *udata)
{
	struct ionic_pd *pd = container_of(ibpd, struct ionic_pd, ibpd);
	struct ionic_ib_dev *rdev = pd->rdev;
	struct ionic_mr *mr;
	struct ib_umem *umem;
	int umem_pgs, rc;
    struct create_mr_cmd cmd;
    struct create_mr_comp resp;

    pr_info("\n%s:", __FUNCTION__);
	if (length == 0 ||
        length > IONIC_MAX_MR_SIZE) {
		pr_err("invalid MR Size: %lld > Max supported:%ld\n",
                length, IONIC_MAX_MR_SIZE);
		return ERR_PTR(-ENOMEM);
	}

	mr = kzalloc(sizeof(*mr), GFP_KERNEL);
	if (!mr)
		return ERR_PTR(-ENOMEM);

	mr->rdev = rdev;

	umem = ib_umem_get(ibpd->uobject->context, start, length,
                       mr_access_flags, 0);
	if (IS_ERR(umem)) {
		pr_err("Failed to get umem");
		rc = -EFAULT;
		goto free_mr;
	}
	mr->ib_umem = umem;
    mr->iova = virt_addr;
    mr->size = length;
    
    mr->ib_mr.lkey = g_key++;
    mr->ib_mr.rkey = g_key++;
    
	umem_pgs = ib_umem_page_count(umem);
	if (!umem_pgs) {
		pr_err("umem is invalid!");
		rc = -EINVAL;
		goto free_mrw;
	}

#if 0    
	pbl_tbl = kcalloc(umem_pgs, sizeof(u64 *), GFP_KERNEL);
	if (!pbl_tbl) {
		rc = -EINVAL;
		goto free_mrw;
	}
    mr->pages = pbl_tbl;
#endif
    
    rc = ionic_page_dir_init(rdev, &mr->pdir, umem_pgs);
    if (rc) {
        pr_err("could not allocate page directory\n");
        goto free_mrw;
    }

	if (umem->page_shift != PAGE_SHIFT) {
		pr_err("umem page shift unsupported!");
		rc = -EFAULT;
		goto fail;
	}

    ionic_page_dir_insert_umem(&mr->pdir, umem, 0);

#if 0
	/* Map umem buf ptrs to the PBL */
	for_each_sg(umem->sg_head.sgl, sg, umem->nmap, entry) {
		pages = sg_dma_len(sg) >> umem->page_shift;
		for (i = 0; i < pages; i++, pbl_tbl++)
			*pbl_tbl = sg_dma_address(sg) + (i << umem->page_shift);
	}
#endif
    
	memset(&cmd, 0, sizeof(cmd));
    memset(&resp, 0, sizeof(resp));
    cmd.opcode = CMD_OPCODE_RDMA_CREATE_MR;
	cmd.pd_num = to_ionic_pd(ibpd)->pdn;
    cmd.lif = rdev->lif->index;
	cmd.access_flags = mr_access_flags;
	cmd.start = start;
	cmd.length = length;
	cmd.pt_dma = (u64) mr->pdir.tbl_dma;
    cmd.page_size = PAGE_SIZE;
	cmd.nchunks = umem_pgs;
    cmd.lkey = mr->ib_mr.lkey;
    cmd.rkey = mr->ib_mr.rkey;
    ionic_rdma_cmd_post(rdev, &cmd, &resp);

	atomic_inc(&rdev->mr_count);

	return &mr->ib_mr;
fail:
	ionic_page_dir_cleanup(rdev, &mr->pdir);
free_mrw:
	ib_umem_release(umem);
free_mr:
	kfree(mr);
	return ERR_PTR(rc);
}

/*
 * Not fully implemented. Just there because it is mandatory
 */
int ionic_dereg_mr(struct ib_mr *ibmr)
{
    pr_info("\n%s:", __FUNCTION__);
	return 0;
}

struct ib_cq *ionic_create_cq(struct ib_device *ibdev,
                              const struct ib_cq_init_attr *attr,
                              struct ib_ucontext *context,
                              struct ib_udata *udata)
{
	struct ionic_ib_dev *rdev = to_ionic_ib_dev(ibdev);
	struct ionic_cq *cq = NULL;
	int rc, entries;
	int cqe = attr->cqe;
	unsigned long flags;
    int ret = 0, npages;
    struct create_cq_cmd cmd;
    struct create_cq_comp comp;
    struct ionic_cq_req req;
    
    pr_info("\n%s (%d):", __FUNCTION__, __LINE__);
	/* Validate CQ fields */
	if (cqe < 1 || cqe > IONIC_MAX_QP_WQE) {
		pr_err("Failed to create CQ -max exceeded");
		return ERR_PTR(-EINVAL);
	}

	cq = kzalloc(sizeof(*cq), GFP_KERNEL);
	if (!cq)
		return ERR_PTR(-ENOMEM);

	cq->rdev = rdev;

	entries = roundup_pow_of_two(cqe);
	if (entries > IONIC_MAX_QP_WQE)
		entries = IONIC_MAX_QP_WQE;

	if (!atomic_add_unless(&rdev->cq_count, 1, IONIC_MAX_CQ_COUNT))
		return ERR_PTR(-ENOMEM);
    
	if (context) {
		if (ib_copy_from_udata(&req, udata, sizeof(req))) {
			rc = -EFAULT;
			goto fail;
		}

        pr_info("\n%s (%d): cq_va 0x%llx cq_bytes 0x%llx",
                __FUNCTION__, __LINE__,
                req.cq_va,
                req.cq_bytes);
        
		cq->umem = ib_umem_get(context, req.cq_va,
                               req.cq_bytes,
                               IB_ACCESS_LOCAL_WRITE, 1);
		if (IS_ERR(cq->umem)) {
			rc = PTR_ERR(cq->umem);
			goto fail;
		}
		npages = ib_umem_page_count(cq->umem);
        pr_info("\n%s (%d): cq_pages %d",
                __FUNCTION__, __LINE__,
                npages);
        
	} else {
        /*
         * No support for Kernel RDMA clients.
         */
        return ERR_PTR(-EINVAL);
    }

	if (npages < 0 || npages > IONIC_PAGE_DIR_MAX_PAGES) {
		ret  = -EINVAL;
		goto err_umem;
	}

	ret = ionic_page_dir_init(rdev, &cq->pdir, npages);
	if (ret) {
        pr_err("could not allocate page directory\n");
		goto err_umem;
	}

    if (context) {
        ionic_page_dir_insert_umem(&cq->pdir, cq->umem, 0);
        pr_info("\n%s(%d): CQ VA 0x%llx bytes 0x%llx num_pages %x", __FUNCTION__, __LINE__, req.cq_va, req.cq_bytes, npages);
        ionic_page_dir_print(&cq->pdir);
        
    } else {
        //cant reach here unless there is a kernel client
    }

    atomic_set(&cq->refcnt, 1);
	init_waitqueue_head(&cq->wait);    
    spin_lock_init(&cq->cq_lock);

    cq->ib_cq.cqe = entries;
    
	memset(&cmd, 0, sizeof(cmd));
    memset(&comp, 0, sizeof(comp));
	cmd.opcode = CMD_OPCODE_RDMA_CREATE_CQ;
    cmd.lif_id = rdev->lif->index;
	cmd.cq_num = cq->cq_num = ++cq_num;
    cmd.num_cq_wqes = entries;
    cmd.host_pg_size = PAGE_SIZE;
	cmd.pt_base_addr = cq->pdir.tbl_dma;
    cmd.cq_lkey = g_key++;
    cmd.cq_va = req.cq_va;
    cmd.va_len = req.cq_bytes;
    cmd.pt_size = npages;
    cmd.cq_wqe_size = IONIC_CQE_SIZE;

	ret = ionic_rdma_cmd_post(rdev, &cmd, &comp);
	if (ret < 0) {
        pr_err("could not create completion queue, error: %d\n", ret);
		goto err_page_dir;
	}

	cq->qtype = comp.qtype;

	spin_lock_irqsave(&rdev->cq_tbl_lock, flags);
	rdev->cq_tbl[cq_num % IONIC_MAX_QP_COUNT] = cq;
	spin_unlock_irqrestore(&rdev->cq_tbl_lock, flags);

    if (context) {
        struct ionic_cq_resp  uresp = {0};

        uresp.cqid = cq_num;
        uresp.qtype = comp.qtype;
        
		/* Copy udata back. */
		if (ib_copy_to_udata(udata, &uresp, sizeof(uresp))) {
			pr_err("%s:failed to copy back udata\n", __FUNCTION__);
			ionic_destroy_cq(&cq->ib_cq);
			return ERR_PTR(-EINVAL);
		}
	}

	return &cq->ib_cq;

err_page_dir:
	ionic_page_dir_cleanup(rdev, &cq->pdir);
err_umem:
	if (context)
		ib_umem_release(cq->umem);
fail:    
	atomic_dec(&rdev->cq_count);
	kfree(cq);

	return ERR_PTR(ret);
}

static void ionic_free_cq(struct ionic_ib_dev *dev, struct ionic_cq *cq)
{
    pr_info("\n%s:", __FUNCTION__);
	atomic_dec(&cq->refcnt);
	wait_event(cq->wait, !atomic_read(&cq->refcnt));

    ib_umem_release(cq->umem);

	ionic_page_dir_cleanup(dev, &cq->pdir);
	kfree(cq);
}

int ionic_destroy_cq(struct ib_cq *ib_cq)
{
	struct ionic_cq *cq = to_ionic_cq(ib_cq);
	struct ionic_ib_dev *dev = cq->rdev;
	unsigned long flags;
	int ret=0;

    pr_info("\n%s:", __FUNCTION__);
    
#ifdef LATER    
	memset(cmd, 0, sizeof(*cmd));
	cmd->hdr.cmd = IONIC_CMD_DESTROY_CQ;
	cmd->cq_handle = cq->cq_handle;

	ret = ionic_rdma_cmd_post(dev, &req, NULL);
	if (ret < 0)
        pr_err("could not destroy completion queue, error: %d\n",
               ret);
#endif
    
	/* free cq's resources */
	spin_lock_irqsave(&dev->cq_tbl_lock, flags);
	dev->cq_tbl[cq->cq_num] = NULL;
	spin_unlock_irqrestore(&dev->cq_tbl_lock, flags);

	ionic_free_cq(dev, cq);
	atomic_dec(&dev->cq_count);

	return ret;
}

/*
 * TODO: Need to implement.
 */
int ionic_poll_cq(struct ib_cq *ibcq, int num_entries, struct ib_wc *wc)
{
    pr_info("\n%s:", __FUNCTION__);
    return 0;
}

/*
 * TODO: Need to implement.
 */
int ionic_req_notify_cq(struct ib_cq *ibcq,
                      enum ib_cq_notify_flags notify_flags)
{
    pr_info("\n%s:", __FUNCTION__);
    return 0;
}

struct ib_qp *ionic_create_qp(struct ib_pd *ibpd,
                              struct ib_qp_init_attr *init_attr,
                              struct ib_udata *udata)
{
	struct ionic_qp *qp = NULL;
	struct ionic_ib_dev *dev = to_ionic_ib_dev(ibpd->device);
    struct ionic_pd *pd = to_ionic_pd(ibpd);
	struct ionic_qp_req ucmd;
    struct ionic_qp_resp uresp = {0};
    struct create_qp_cmd cmd;
    struct create_qp_comp comp;
    unsigned long flags;
	int ret;
    struct create_mr_cmd mr_cmd;
    struct create_mr_comp mr_resp;
    u32 sq_lkey, rq_lkey;

    pr_info("\n%s (%d):", __FUNCTION__, __LINE__);
	if (init_attr->qp_type != IB_QPT_RC) {
        pr_err("queuepair type %d not supported\n",
                 init_attr->qp_type);
		return ERR_PTR(-EINVAL);
	}

	if (!atomic_add_unless(&dev->qp_count, 1, IONIC_MAX_QP_COUNT))
		return ERR_PTR(-ENOMEM);

	switch (init_attr->qp_type) {
	case IB_QPT_RC:
	case IB_QPT_UD:
		qp = kzalloc(sizeof(*qp), GFP_KERNEL);
		if (!qp) {
			ret = -ENOMEM;
			goto err_qp;
		}

		spin_lock_init(&qp->sq.lock);
		spin_lock_init(&qp->rq.lock);
		mutex_init(&qp->mutex);
		atomic_set(&qp->refcnt, 1);
		init_waitqueue_head(&qp->wait);
        
		qp->state = IB_QPS_RESET;

		if (ibpd->uobject && udata) {
			pr_err("create queuepair from user space\n");

			if (ib_copy_from_udata(&ucmd, udata, sizeof(ucmd))) {
				ret = -EFAULT;
				goto err_qp;
			}

            pr_info("\n%s (%d): qprva 0x%llx rq_bytes %d qpsva 0x%llx sq_bytes %d",
                    __FUNCTION__, __LINE__, ucmd.qprva,
                    ucmd.rq_bytes, ucmd.qpsva,
                    ucmd.sq_bytes);            
			/* set qp->sq.wqe_cnt, shift, buf_size.. */
			qp->rumem = ib_umem_get(ibpd->uobject->context,
                                    ucmd.qprva,
                                    ucmd.rq_bytes, 0, 0);
			if (IS_ERR(qp->rumem)) {
				ret = PTR_ERR(qp->rumem);
				goto err_qp;
			}

			qp->sumem = ib_umem_get(ibpd->uobject->context,
                                    ucmd.qpsva,
                                    ucmd.sq_bytes, 0, 0);
			if (IS_ERR(qp->sumem)) {
				ib_umem_release(qp->rumem);
				ret = PTR_ERR(qp->sumem);
				goto err_qp;
			}

			qp->npages_send = ib_umem_page_count(qp->sumem);
			qp->npages_recv = ib_umem_page_count(qp->rumem);
			qp->npages = qp->npages_send + qp->npages_recv;
            pr_info("\n%s (%d): send_pages %d recv_pages %d",
                    __FUNCTION__, __LINE__,
                    qp->npages_send, qp->npages_recv);            
		} else {
			qp->is_kernel = true;

            ret = -EINVAL;
            goto err_qp;
		}

		if (qp->npages < 0 || qp->npages > IONIC_PAGE_DIR_MAX_PAGES) {
            pr_err("overflow pages in queuepair\n");
			ret = -EINVAL;
			goto err_umem;
		}

		ret = ionic_page_dir_init(dev, &qp->pdir, qp->npages);
		if (ret) {
            pr_err("could not allocate page directory\n");
			goto err_umem;
		}

        ionic_page_dir_insert_umem(&qp->pdir, qp->sumem, 0);
        ionic_page_dir_insert_umem(&qp->pdir, qp->rumem,
                                   qp->npages_send);
        pr_info("\n%s(%d): SQ VA 0x%llx bytes 0x%x num_pages %x", __FUNCTION__, __LINE__, ucmd.qpsva, ucmd.sq_bytes, qp->npages_send);
        pr_info("\n%s(%d): RQ VA 0x%llx bytes 0x%x num_pages %x", __FUNCTION__, __LINE__, ucmd.qprva, ucmd.rq_bytes, qp->npages_recv);
        ionic_page_dir_print(&qp->pdir);
		break;
	default:
		ret = -EINVAL;
		goto err_qp;
	}

    /*
    * TODO:
    * Since dev command is only 64 bytes, sending separate meomry registrations
    * first and then qp_create command. Need to change later as we move to
    * adminQ command.
    */

    //register sq memory
	memset(&mr_cmd, 0, sizeof(mr_cmd));
    memset(&mr_resp, 0, sizeof(mr_resp));

    mr_cmd.opcode = CMD_OPCODE_RDMA_CREATE_MR;
	mr_cmd.pd_num = to_ionic_pd(ibpd)->pdn;
    mr_cmd.lif = dev->lif->index;
	mr_cmd.start  = ucmd.qpsva;
	mr_cmd.length = ucmd.sq_bytes;
	mr_cmd.access_flags = IB_ACCESS_LOCAL_WRITE;
	mr_cmd.nchunks = qp->npages_send;
	mr_cmd.pt_dma = (u64)qp->pdir.tbl_dma;
    mr_cmd.lkey = sq_lkey = g_key++;
    mr_cmd.page_size = PAGE_SIZE;
    
    ionic_rdma_cmd_post(dev, &mr_cmd, &mr_resp);
    
    // register rq memory
	memset(&mr_cmd, 0, sizeof(mr_cmd));
    memset(&mr_resp, 0, sizeof(mr_resp));

    mr_cmd.opcode = CMD_OPCODE_RDMA_CREATE_MR;
    mr_cmd.pd_num = to_ionic_pd(ibpd)->pdn;
    mr_cmd.lif = dev->lif->index;
	mr_cmd.start  = ucmd.qprva;
	mr_cmd.length = ucmd.rq_bytes;
	mr_cmd.access_flags = IB_ACCESS_LOCAL_WRITE;
	mr_cmd.nchunks = qp->npages_recv;
	mr_cmd.pt_dma = (u64)qp->pdir.tbl_dma + qp->npages_send*sizeof(u64);
    mr_cmd.lkey = rq_lkey = g_key++;
    mr_cmd.page_size = PAGE_SIZE;
    
    ionic_rdma_cmd_post(dev, &mr_cmd, &mr_resp);

    //register QP
	memset(&cmd, 0, sizeof(cmd));
    memset(&comp, 0, sizeof(comp));
	cmd.opcode = CMD_OPCODE_RDMA_CREATE_QP;
    cmd.sq_wqe_size = ucmd.sq_wqe_size;
    cmd.rq_wqe_size = ucmd.rq_wqe_size;
	cmd.num_sq_wqes = init_attr->cap.max_send_wr;
	cmd.num_rq_wqes = init_attr->cap.max_recv_wr;
    cmd.num_rsq_wqes = IONIC_NUM_RSQ_WQE;
    cmd.num_rrq_wqes = IONIC_NUM_RRQ_WQE;
    cmd.pd = pd->pdn;
    cmd.lif_id = dev->lif->index;

	cmd.service = ib_qp_type_to_ionic(init_attr->qp_type);
    cmd.pmtu = 1024;
    cmd.qp_num = g_qp_num++;

	cmd.sq_cq_num = to_ionic_cq(init_attr->send_cq)->cq_num;
	cmd.rq_cq_num = to_ionic_cq(init_attr->recv_cq)->cq_num;
    cmd.host_pg_size = PAGE_SIZE;
    cmd.sq_lkey = sq_lkey;
    cmd.rq_lkey = rq_lkey;
    
	pr_err("create queuepair with %d, %d\n",
           cmd.num_sq_wqes, cmd.num_rq_wqes);

	ret = ionic_rdma_cmd_post(dev, &cmd, &comp);
	if (ret < 0) {
		pr_err("could not create queuepair, error: %d\n", ret);
		goto err_pdir;
	}

	/* max_send_wr/_recv_wr/_send_sge/_recv_sge/_inline_data */
	qp->port = init_attr->port_num;
	qp->ibqp.qp_num = cmd.qp_num;
    qp->rdev = dev;

	spin_lock_irqsave(&dev->qp_tbl_lock, flags);
	dev->qp_tbl[qp->ibqp.qp_num % IONIC_MAX_QP_COUNT] = qp;
	spin_unlock_irqrestore(&dev->qp_tbl_lock, flags);

    if (ibpd->uobject && udata) {
        uresp.qpid = cmd.qp_num;
        uresp.sq_qtype = comp.sq_qtype;
        uresp.rq_qtype = comp.rq_qtype;
        pr_info("sq_qtype %d rq_qtype %d\n", uresp.sq_qtype, uresp.rq_qtype);
		/* Copy udata back. */
		if (ib_copy_to_udata(udata, &uresp, sizeof(uresp))) {
			pr_err("%s:failed to copy back udata\n", __FUNCTION__);
			ionic_destroy_qp(&qp->ibqp);
			return ERR_PTR(-EINVAL);
		}
	}
    
	return &qp->ibqp;

err_pdir:
	ionic_page_dir_cleanup(dev, &qp->pdir);
err_umem:
	if (ibpd->uobject && udata) {
		if (qp->rumem)
			ib_umem_release(qp->rumem);
		if (qp->sumem)
			ib_umem_release(qp->sumem);
	}
err_qp:
	kfree(qp);
	atomic_dec(&dev->qp_count);

	return ERR_PTR(ret);
}

static void ionic_free_qp(struct ionic_qp *qp)
{
	struct ionic_ib_dev *dev = to_ionic_ib_dev(qp->ibqp.device);
	struct ionic_cq *scq;
	struct ionic_cq *rcq;
    unsigned long flags;
    
    pr_info("\n%s:", __FUNCTION__);
	/* In case cq is polling */
	scq = to_ionic_cq(qp->ibqp.send_cq);
    rcq = to_ionic_cq(qp->ibqp.recv_cq);

#ifdef LATER    
	unsigned long scq_flags, rcq_flags;
	ionic_lock_cqs(scq, rcq, &scq_flags, &rcq_flags);
    
	_ionic_flush_cqe(qp, scq);
	if (scq != rcq)
		_ionic_flush_cqe(qp, rcq);
#endif

	spin_lock_irqsave(&dev->qp_tbl_lock, flags);
	dev->qp_tbl[qp->ibqp.qp_num] = NULL;
	spin_unlock_irqrestore(&dev->qp_tbl_lock, flags);

#ifdef LATER    
	ionic_unlock_cqs(scq, rcq, &scq_flags, &rcq_flags);
#endif
    
	atomic_dec(&qp->refcnt);
	wait_event(qp->wait, !atomic_read(&qp->refcnt));

	ionic_page_dir_cleanup(dev, &qp->pdir);

	kfree(qp);

	atomic_dec(&dev->qp_count);
}

/**
 * ionic_destroy_qp - destroy a queue pair
 * @qp: the queue pair to destroy
 *
 * @return: 0 on success.
 */
int ionic_destroy_qp(struct ib_qp *qp)
{
	struct ionic_qp *pqp = to_ionic_qp(qp);
	//int ret;

    pr_info("\n%s:", __FUNCTION__);

	ionic_free_qp(pqp);

	return 0;
}


uint8_t *memrev(uint8_t *block, size_t elnum)
{
    uint8_t *s, *t, tmp;

    for (s = block, t = s + (elnum - 1); s < t; s++, t--) {
        tmp = *s;
        *s = *t;
        *t = tmp;
    }
    return block;
}

/**
 * ionic_modify_qp - modify queue pair attributes
 * @ibqp: the queue pair
 * @attr: the new queue pair's attributes
 * @attr_mask: attributes mask
 * @udata: user data
 *
 * @returns 0 on success, otherwise returns an errno.
 */
int ionic_modify_qp(struct ib_qp *ibqp,
                  struct ib_qp_attr *attr,
                  int attr_mask,
                  struct ib_udata *udata)
{
	//struct ionic_ib_dev *dev = to_ionic_ib_dev(ibqp->device);
	struct ionic_qp *qp = to_ionic_qp(ibqp);
    struct modify_qp_cmd cmd;
    struct modify_qp_comp comp;
	int ret = 0;
    const struct ib_global_route *grh = rdma_ah_read_grh(&attr->ah_attr);
    union ib_gid sgid;
    struct ib_gid_attr sgid_attr;
    int i;
    
    pr_info("\n%s:", __FUNCTION__);
	/* Sanity checking. Should need lock here */
	mutex_lock(&qp->mutex);

	memset(&cmd, 0, sizeof(cmd));
	memset(&comp, 0, sizeof(comp));    

	if (attr_mask & IB_QP_QKEY)
		qp->qkey = attr->qkey;

    if (attr_mask & IB_QP_STATE)
        qp->state = attr->qp_state;

    if (attr_mask & IB_QP_DEST_QPN)
        qp->state = attr->dest_qp_num;

    //TODO: There is lot of hard coding here. Fix later
    if (attr_mask & IB_QP_AV) {

        ret = ib_get_cached_gid(&qp->rdev->ibdev, 1,
                                grh->sgid_index, &sgid, &sgid_attr);

        if (ret) {
            pr_err("\n%s:Failed to query gid at index %d port %d ret %d",
                   __FUNCTION__, grh->sgid_index, attr->port_num, ret);
            mutex_unlock(&qp->mutex);
            return ret;
        }

        //TODO: May be an extreme to allocate a page just for
        //header template
        qp->header = dma_alloc_coherent(&qp->rdev->pdev->dev,
                                        PAGE_SIZE,
                                        (dma_addr_t *)&qp->hdr_dma,
                                        GFP_KERNEL);
        memset(qp->header, 0, sizeof(header_template_t));
        ether_addr_copy(qp->header->eth.smac, qp->rdev->netdev->dev_addr);
		ether_addr_copy(qp->header->eth.dmac,
                        attr->ah_attr.roce.dmac);
        memrev((uint8_t *)qp->header->eth.smac, sizeof(qp->header->eth.smac));
        memrev((uint8_t *)qp->header->eth.dmac, sizeof(qp->header->eth.dmac));
        //is_vlan = is_vlan_dev(qp->rdev->netdev);
        
        qp->header->eth.ethertype = 0x0800;
        qp->header->ip.version = 4;
        qp->header->ip.ihl = 5;
        qp->header->ip.tos = 0;
        qp->header->ip.ttl = 64;
        qp->header->ip.id = 0x5c2a;
        qp->header->ip.frag_off = 0x4000;
        qp->header->ip.protocol = 17;
        memcpy(&qp->header->ip.saddr, sgid.raw + 12, 4);
        memcpy(&qp->header->ip.daddr, grh->dgid.raw + 12, 4);
        memrev((uint8_t *)&qp->header->ip.saddr, 4);
        memrev((uint8_t *)&qp->header->ip.daddr, 4);
        qp->header->ip.id = 1;
        qp->header->udp.sport = 49152;
        qp->header->udp.dport = 4791;

        /*
         * Prepare for little endian to big endian.
         */
        memrev((uint8_t *)qp->header, sizeof(header_template_t));
        pr_info("Header Template:\n");
        for (i = 0; i < sizeof(header_template_t); i++) {
            pr_info("%x", ((uint8_t *)qp->header)[i]);
        }
        pr_info("\n");
    }

	cmd.opcode = CMD_OPCODE_RDMA_MODIFY_QP;
	cmd.qp_num = qp->ibqp.qp_num;
	cmd.attr_mask = (u32)attr_mask;
	cmd.q_key = attr->qkey;
	cmd.dest_qp_num = attr->dest_qp_num;
    cmd.e_psn = attr->rq_psn;
    cmd.sq_psn = attr->sq_psn;    
    cmd.header_template = (u64) qp->hdr_dma;
    cmd.header_template_size = sizeof(header_template_t);   

	ret = ionic_rdma_cmd_post(qp->rdev, &cmd, &comp);
	if (ret < 0) {
        pr_err("could not modify queuepair, error: %d\n", ret);
	} else if (comp.status > 0) {
		pr_err("cannot modify queuepair, error: %d\n", comp.status);
		ret = -EINVAL;
	}

    mutex_unlock(&qp->mutex);
	return ret;
}

/*
 * TODO: Yet to implement
 */
int ionic_query_qp(struct ib_qp *ibqp, struct ib_qp_attr *attr,
                    int attr_mask, struct ib_qp_init_attr *init_attr)
{
    pr_info("\n%s:", __FUNCTION__);
	return 0;
}

/*
 * TODO: Yet to implement
 */
int ionic_post_send(struct ib_qp *ibqp, struct ib_send_wr *wr,
                     struct ib_send_wr **bad_wr)
{
    pr_info("\n%s:", __FUNCTION__);
    return 0;
}

/*
 * TODO: Yet to implement
 */
int ionic_post_recv(struct ib_qp *ibqp, struct ib_recv_wr *wr,
                     struct ib_recv_wr **bad_wr)
{
    pr_info("\n%s:", __FUNCTION__);
    return 0;
}

int ionic_add_gid(struct ib_device *ibdev,
                  u8 port_num,
                  unsigned int index,
                  const union ib_gid *gid,
                  const struct ib_gid_attr *attr,
                  void **context)
{
	struct ionic_ib_dev *dev = to_ionic_ib_dev(ibdev);

    pr_info("\n%s:", __FUNCTION__);
	if (!dev->sgid_tbl) {
		pr_err("sgid table not initialized\n");
		return -EINVAL;
	}

	memcpy(&dev->sgid_tbl[index], gid, sizeof(*gid));
    return 0;
}

int ionic_del_gid(struct ib_device *ibdev,
                          u8 port_num,
                          unsigned int index,
                          void **context)
{
    struct ionic_ib_dev *dev = to_ionic_ib_dev(ibdev);

	pr_info("\nremoving gid at index %u from %s",
           index, dev->netdev->name);

	if (!dev->sgid_tbl) {
		pr_err("sgid table not initialized\n");
		return -EINVAL;
	}

	memset(&dev->sgid_tbl[index], 0, 16);
	return 0;
}

struct net_device *ionic_get_netdev(struct ib_device *ibdev,
                                    u8 port_num)
{
	struct net_device *netdev;
	struct ionic_ib_dev *dev = to_ionic_ib_dev(ibdev);

    pr_info("\n%s:", __FUNCTION__);
	rcu_read_lock();
	netdev = dev->netdev;
	if (netdev)
		dev_hold(netdev);
	rcu_read_unlock();

	return netdev;
}

enum rdma_link_layer ionic_port_link_layer(struct ib_device *ibdev,
                                           u8 port)
{
    pr_info("\n%s:", __FUNCTION__);
	return IB_LINK_LAYER_ETHERNET;
}
