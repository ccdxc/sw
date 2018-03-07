#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "main.h"
#include "verbs.h"

#define PCI_VENDOR_ID_PENSANDO		0x1dd8

#define CNA(v, d) VERBS_PCI_MATCH(PCI_VENDOR_ID_##v, d, NULL)

static const struct verbs_match_ent cna_table[] = {
    CNA(PENSANDO, 0x1002),  /* capri */
    {}
};

static const struct verbs_context_ops ionic_ctx_ops = {
	.query_device  = ionic_query_device,
	.query_port    = ionic_query_port,
	.alloc_pd      = ionic_alloc_pd,
	.dealloc_pd    = ionic_free_pd,
	.reg_mr        = ionic_reg_mr,
	.dereg_mr      = ionic_dereg_mr,
	.create_cq     = ionic_create_cq,
	.poll_cq       = ionic_poll_cq,
	.req_notify_cq = ionic_arm_cq,
	.cq_event      = ionic_cq_event,
	.resize_cq     = ionic_resize_cq,
	.destroy_cq    = ionic_destroy_cq,
	.create_srq    = ionic_create_srq,
	.modify_srq    = ionic_modify_srq,
	.query_srq     = ionic_query_srq,
	.destroy_srq   = ionic_destroy_srq,
	.post_srq_recv = ionic_post_srq_recv,
	.create_qp     = ionic_create_qp,
	.query_qp      = ionic_query_qp,
	.modify_qp     = ionic_modify_qp,
	.destroy_qp    = ionic_destroy_qp,
	.post_send     = ionic_post_send,
	.post_recv     = ionic_post_recv,
	.create_ah     = ionic_create_ah,
	.destroy_ah    = ionic_destroy_ah
};

 /* Context Init functions */
static struct verbs_context *ionic_alloc_context(struct ibv_device *ibdev,
						 int cmd_fd)
{
	struct ionic_dev *dev = to_ionic_dev(ibdev);
	struct ionic_context *cntx;
	struct ibv_get_context cmd;
	struct ionic_cntx_resp resp;
	int rc;

	cntx = verbs_init_and_alloc_context(ibdev, cmd_fd, cntx, vctx);
	if (!cntx) {
		rc = errno;
		goto err_ctx;
	}

	rc = ibv_cmd_get_context(&cntx->vctx, &cmd, sizeof(cmd),
				&resp.resp, sizeof(resp));
	if (rc)
		goto err_cmd;


	cntx->max_qp = resp.max_qp;
	/* XXX cleanup, overwrites dev fields on each init ctx */
	dev->pg_size = resp.pg_size;
	dev->cqe_size = resp.cqe_size;
	dev->max_cq_depth = resp.max_cqd;

	/* mmap shared page. */
	cntx->udpi.dbpage = (struct ionic_doorbell *) mmap(NULL, dev->pg_size,
                                                       PROT_WRITE,
                                                       MAP_SHARED, cmd_fd, 0);

	pthread_mutex_init(&cntx->mut, NULL);
	tbl_init(&cntx->qp_tbl);
	list_head_init(&cntx->cq_list);
    
    pthread_spin_init(&cntx->udpi.db_lock,
                      PTHREAD_PROCESS_PRIVATE);

	verbs_set_ops(&cntx->vctx, &ionic_ctx_ops);

	return &cntx->vctx;

err_cmd:
	verbs_uninit_context(&cntx->vctx);
err_ctx:
	errno = rc;
	return NULL;
}

static void ionic_free_context(struct ibv_context *ibctx)
{
	struct ionic_dev *dev = to_ionic_dev(ibctx->device);
	struct ionic_context *cntx = to_ionic_context(ibctx);

	tbl_destroy(&cntx->qp_tbl);
	pthread_mutex_destroy(&cntx->mut);

	/* Un-map DPI only for the first PD that was
	 * allocated in this context.
	 */
	if (cntx->udpi.dbpage && cntx->udpi.dbpage != MAP_FAILED) {
		pthread_spin_destroy(&cntx->udpi.db_lock);
		munmap(cntx->udpi.dbpage, dev->pg_size);
		cntx->udpi.dbpage = NULL;
	}

	verbs_uninit_context(&cntx->vctx);
	free(cntx);
}

static struct verbs_device *ionic_alloc_device(struct verbs_sysfs_dev *sysfs_dev)
{
	struct ionic_dev *dev;

	dev = calloc(1, sizeof(*dev));
	if (!dev)
		return NULL;

	return &dev->vdev;
}

static void ionic_uninit_device(struct verbs_device *vdev)
{
	struct ionic_dev *dev = to_ionic_dev(&vdev->device);

	free(dev);
}

static const struct verbs_device_ops ionic_dev_ops = {
	.name = "ionic",
	.match_min_abi_version = IONIC_ABI_VERSION,
	.match_max_abi_version = IONIC_ABI_VERSION,
	.match_table = cna_table,
	.alloc_device = ionic_alloc_device,
	.uninit_device = ionic_uninit_device,
	.alloc_context = ionic_alloc_context,
	.free_context = ionic_free_context,
};
PROVIDER_DRIVER(ionic_dev_ops);
