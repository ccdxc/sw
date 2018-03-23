#include "ionic.h"

extern const struct verbs_context_ops fallback_ctx_ops;
extern const struct verbs_context_ops ionic_ctx_ops;

static int ionic_env_fallback(void)
{
	const char *env = getenv("IONIC_FALLBACK");

	if (!env)
		return 0;

	return atoi(env);
}

static struct verbs_context *ionic_alloc_context(struct ibv_device *ibdev,
						 int cmd_fd)
{
	struct ionic_dev *dev = to_ionic_dev(ibdev);
	struct ionic_ctx *ctx;
	struct ionic_ctx_req req = {};
	struct ionic_ctx_resp resp = {};
	int rc;

	ctx = verbs_init_and_alloc_context(ibdev, cmd_fd, ctx, vctx);
	if (!ctx) {
		rc = errno;
		goto err_ctx;
	}

	req.version = IONIC_ABI_VERSION; /* XXX fw abi version */
	req.fallback = ionic_env_fallback();

	rc = ibv_cmd_get_context(&ctx->vctx, &req.req, sizeof(req),
				 &resp.resp, sizeof(resp));
	if (rc)
		goto err_cmd;

	verbs_set_ops(&ctx->vctx, &fallback_ctx_ops);

	ctx->version = resp.version; /* XXX fw abi version */
	ctx->fallback = resp.fallback != 0;

	if (!ctx->fallback) {
		ctx->sq_qtype = resp.sq_qtype;
		ctx->rq_qtype = resp.rq_qtype;
		ctx->cq_qtype = resp.cq_qtype;

		ctx->dbpage = ionic_map_device(dev->pg_size, cmd_fd,
					       resp.dbell_offset);
		if (!ctx->dbpage) {
			rc = errno;
			goto err_cmd;
		}

		pthread_mutex_init(&ctx->mut, NULL);
		tbl_init(&ctx->qp_tbl);

		verbs_set_ops(&ctx->vctx, &ionic_ctx_ops);
	}

	return &ctx->vctx;

err_cmd:
	verbs_uninit_context(&ctx->vctx);
err_ctx:
	errno = rc;
	return NULL;
}

static void ionic_free_context(struct ibv_context *ibctx)
{
	struct ionic_dev *dev = to_ionic_dev(ibctx->device);
	struct ionic_ctx *ctx = to_ionic_ctx(ibctx);

	tbl_destroy(&ctx->qp_tbl);
	pthread_mutex_destroy(&ctx->mut);

	ionic_unmap(ctx->dbpage, dev->pg_size);

	verbs_uninit_context(&ctx->vctx);
	free(ctx);
}

static struct verbs_device *ionic_alloc_device(struct verbs_sysfs_dev *sysfs_dev)
{
	struct ionic_dev *dev;

	dev = calloc(1, sizeof(*dev));
	if (!dev)
		return NULL;

	/* XXX hardcode value */
	dev->pg_size = 0x1000;

	return &dev->vdev;
}

static void ionic_uninit_device(struct verbs_device *vdev)
{
	struct ionic_dev *dev = to_ionic_dev(&vdev->device);

	free(dev);
}

#define PCI_VENDOR_ID_PENSANDO 0x1dd8
#define CNA(v, d) VERBS_PCI_MATCH(PCI_VENDOR_ID_##v, d, NULL)

static const struct verbs_match_ent cna_table[] = {
	CNA(PENSANDO, 0x1002), /* capri */
	{}
};

static const struct verbs_device_ops ionic_dev_ops = {
	.name			= "ionic",
	.match_min_abi_version	= IONIC_ABI_VERSION,
	.match_max_abi_version	= IONIC_ABI_VERSION,
	.match_table		= cna_table,
	.alloc_device		= ionic_alloc_device,
	.uninit_device		= ionic_uninit_device,
	.alloc_context		= ionic_alloc_context,
	.free_context		= ionic_free_context,
};
PROVIDER_DRIVER(ionic_dev_ops);
