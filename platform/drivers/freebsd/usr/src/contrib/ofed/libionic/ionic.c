#include "ionic.h"

extern void ionic_set_fallback_ops(struct ibv_context *ibctx);
extern void ionic_set_ops(struct ibv_context *ibctx);

static int ionic_env_fallback(void)
{
	const char *env = getenv("IONIC_FALLBACK");

	if (!env)
		return 0;

	return atoi(env);
}

static int ionic_init_context(struct verbs_device *vdev,
			      struct ibv_context *ibctx,
			      int cmd_fd)
{
	struct ionic_dev *dev = to_ionic_dev(&vdev->device);
	struct ionic_ctx *ctx = to_ionic_ctx(ibctx);
	struct ionic_ctx_req req = {};
	struct ionic_ctx_resp resp = {};
	int rc = 0;

	req.version = IONIC_ABI_VERSION; /* XXX fw abi version */
	req.fallback = ionic_env_fallback();

	ibctx->cmd_fd = cmd_fd;

	rc = ibv_cmd_get_context(ibctx, &req.req, sizeof(req),
				 &resp.resp, sizeof(resp));
	if (rc)
		goto out;

	ionic_set_fallback_ops(ibctx);

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
			goto out;
		}

		pthread_mutex_init(&ctx->mut, NULL);
		tbl_init(&ctx->qp_tbl);

		ionic_set_ops(ibctx);
	}

out:
	return rc;
}

static void ionic_uninit_context(struct verbs_device *vdev,
				 struct ibv_context *ibctx)
{
	struct ionic_dev *dev = to_ionic_dev(&vdev->device);
	struct ionic_ctx *ctx = to_ionic_ctx(ibctx);

	tbl_destroy(&ctx->qp_tbl);
	pthread_mutex_destroy(&ctx->mut);

	ionic_unmap(ctx->dbpage, dev->pg_size);
}

#define PCI_VENDOR_ID_PENSANDO 0x1dd8
#define CNA(v, d) { .vendor = (PCI_VENDOR_ID_##v), .device = (d) }

static const struct {
	unsigned vendor;
	unsigned device;
} cna_table[] = {
	CNA(PENSANDO, 0x1002), /* capri */
	{}
};

static const struct verbs_device_ops ionic_dev_ops = {
	.init_context		= ionic_init_context,
	.uninit_context		= ionic_uninit_context,
};

static struct verbs_device *ionic_alloc_device(const char *sysfs_path,
					       int abi_version)
{
	struct ionic_dev *dev;
	unsigned vendor, device;
	char value[8];
	int i;

	goto found; /* for some reason no "device" group */

	if (ibv_read_sysfs_file(sysfs_path, "device/vendor",
				value, sizeof(value) < 0))
		return NULL;
	if (sscanf(value, "%i", &vendor) != 1)
		return NULL;

	if (ibv_read_sysfs_file(sysfs_path, "device/device",
				value, sizeof(value) < 0))
		return NULL;
	if (sscanf(value, "%i", &device) != 1)
		return NULL;

	for (i = 0; cna_table[i].vendor; ++i)
		if (vendor == cna_table[i].vendor &&
		    device == cna_table[i].device)
			goto found;

	return NULL;

found:
	if (abi_version != IONIC_ABI_VERSION)
		return NULL;

	dev = calloc(1, sizeof(*dev));
	if (!dev)
		return NULL;

	/* XXX hardcode value */
	dev->pg_size = 0x1000;

	dev->vdev.ops = &ionic_dev_ops;
	dev->vdev.sz = sizeof(*dev);
	dev->vdev.size_of_context =
		sizeof(struct ionic_ctx) - sizeof(struct ibv_context);

	return &dev->vdev;
}

static __attribute__((constructor)) void ionic_register_driver(void)
{
	verbs_register_driver("ionic", ionic_alloc_device);
}
