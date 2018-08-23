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

#include <stdio.h>

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
	struct ionic_ctx *ctx;
	struct uionic_ctx req = {};
	struct uionic_ctx_resp resp = {};
	int rc, version, compat;

	ctx = verbs_init_and_alloc_context(ibdev, cmd_fd, ctx, vctx,
					   RDMA_DRIVER_UNKNOWN);
	if (!ctx) {
		rc = errno;
		goto err_ctx;
	}

	req.fallback = ionic_env_fallback();

	rc = ibv_cmd_get_context(&ctx->vctx, &req.ibv_cmd, sizeof(req),
				 &resp.ibv_resp, sizeof(resp));
	if (rc)
		goto err_cmd;

	verbs_set_ops(&ctx->vctx, &fallback_ctx_ops);

	ctx->fallback = resp.fallback != 0;
	ctx->pg_shift = resp.page_shift;

	if (!ctx->fallback) {
		version = resp.version;

		if (version <= IONIC_MAX_RDMA_VERSION) {
			compat = 0;
		} else {
			compat = version - IONIC_MAX_RDMA_VERSION;
			version = IONIC_MAX_RDMA_VERSION;
		}

		while (version > 0 && compat < 7) {
			if (resp.qp_opcodes[compat])
				break;
			--version;
			++compat;
		}

		if (version < IONIC_MIN_RDMA_VERSION) {
			fprintf(stderr, "ionic: Firmware RDMA Version %u\n",
				resp.version);
			fprintf(stderr, "ionic: Driver Min RDMA Version %u\n",
				IONIC_MIN_RDMA_VERSION);
			rc = EINVAL;
			goto err_cmd;
		}

		if (compat >= 7) {
			fprintf(stderr, "ionic: Firmware RDMA Version %u\n",
				resp.version);
			fprintf(stderr, "ionic: Driver Max RDMA Version %u\n",
				IONIC_MAX_RDMA_VERSION);
			rc = EINVAL;
			goto err_cmd;
		}

		ctx->version = version;
		ctx->compat = compat;
		ctx->opcodes = resp.qp_opcodes[compat];

		ctx->sq_qtype = resp.sq_qtype;
		ctx->rq_qtype = resp.rq_qtype;
		ctx->cq_qtype = resp.cq_qtype;

		ctx->max_stride = resp.max_stride;

		ctx->dbpage = ionic_map_device(1u << ctx->pg_shift, cmd_fd,
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
	struct ionic_ctx *ctx = to_ionic_ctx(ibctx);

	tbl_destroy(&ctx->qp_tbl);
	pthread_mutex_destroy(&ctx->mut);

	ionic_unmap(ctx->dbpage, 1u << ctx->pg_shift);

	verbs_uninit_context(&ctx->vctx);
	free(ctx);
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
