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
	struct uionic_ctx req = {};
	struct uionic_ctx_resp resp = {};
	int rc, version, compat;

	req.fallback = ionic_env_fallback();

	ibctx->cmd_fd = cmd_fd;

	rc = ibv_cmd_get_context(ibctx, &req.ibv_cmd, sizeof(req),
				 &resp.ibv_resp, sizeof(resp));
	if (rc)
		goto out;

	ionic_set_fallback_ops(ibctx);

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
			goto out;
		}

		if (compat >= 7) {
			fprintf(stderr, "ionic: Firmware RDMA Version %u\n",
				resp.version);
			fprintf(stderr, "ionic: Driver Max RDMA Version %u\n",
				IONIC_MAX_RDMA_VERSION);
			rc = EINVAL;
			goto out;
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
	struct ionic_ctx *ctx = to_ionic_ctx(ibctx);

	tbl_destroy(&ctx->qp_tbl);
	pthread_mutex_destroy(&ctx->mut);

	ionic_unmap(ctx->dbpage, 1u << ctx->pg_shift);
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
