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

#include <stdio.h>
#include <stdlib.h>

#include "ionic.h"
#include "ionic_dbg.h"
#include "ionic_stats.h"

extern void ionic_set_ops(struct ibv_context *ibctx);

FILE *IONIC_DEBUG_FILE;

static void ionic_debug_file_close(void)
{
	fclose(IONIC_DEBUG_FILE);
}

static void ionic_debug_file_open(void)
{
	const char *name = getenv("IONIC_DEBUG_FILE");

	if (!name) {
		IONIC_DEBUG_FILE = IONIC_DEFAULT_DEBUG_FILE;
		return;
	}

	IONIC_DEBUG_FILE = fopen(name, "w");

	if (!IONIC_DEBUG_FILE) {
		perror("ionic debug file: ");
		return;
	}

	atexit(ionic_debug_file_close);
}

static void ionic_debug_file_init(void)
{
	static pthread_once_t once = PTHREAD_ONCE_INIT;

	pthread_once(&once, ionic_debug_file_open);
}

static int ionic_env_val_def(const char *name, int def)
{
	const char *env = getenv(name);

	if (!env)
		return def;

	return atoi(env);
}

static int ionic_env_val(const char *name)
{
	return ionic_env_val_def(name, 0);
}

static int ionic_env_debug(void)
{
	if (!(IONIC_DEBUG))
		return 0;

	return ionic_env_val("IONIC_DEBUG");
}

static int ionic_env_stats(void)
{
	if (!(IONIC_STATS))
		return 0;

	return ionic_env_val("IONIC_STATS");
}

static int ionic_env_lats(void)
{
	if (!(IONIC_LATS))
		return 0;

	return ionic_env_val("IONIC_LATS");
}

static int ionic_env_lockfree(void)
{
	return ionic_env_val("IONIC_LOCKFREE");
}

static int ionic_env_spec(int kspec)
{
	return ionic_env_val_def("IONIC_SPEC", kspec);
}

static int ionic_init_context(struct verbs_device *vdev,
			      struct ibv_context *ibctx,
			      int cmd_fd)
{
	struct ionic_dev *dev = to_ionic_dev(&vdev->device);
	struct ionic_ctx *ctx = to_ionic_ctx(ibctx);
	struct uionic_ctx req = {};
	struct uionic_ctx_resp resp = {};
	int rc, version, level;

	ionic_debug_file_init();

	ibctx->cmd_fd = cmd_fd;

	rc = ibv_cmd_get_context(ibctx, &req.ibv_cmd, sizeof(req),
				 &resp.ibv_resp, sizeof(resp));
	if (rc)
		goto err_cmd;

	ctx->pg_shift = resp.page_shift;

	version = resp.version;
	if (version < IONIC_MIN_RDMA_VERSION) {
		fprintf(stderr, "ionic: Firmware RDMA Version %u\n",
			version);
		fprintf(stderr, "ionic: Driver Min RDMA Version %u\n",
			IONIC_MIN_RDMA_VERSION);
		rc = EINVAL;
		goto err_cmd;
	}

	if (version > IONIC_MAX_RDMA_VERSION) {
		fprintf(stderr, "ionic: Firmware RDMA Version %u\n",
			version);
		fprintf(stderr, "ionic: Driver Max RDMA Version %u\n",
			IONIC_MAX_RDMA_VERSION);
		rc = EINVAL;
		goto err_cmd;
	}

	ctx->version = version;
	ctx->opcodes = resp.qp_opcodes;
	if (ctx->opcodes <= IONIC_V1_OP_BIND_MW) {
		fprintf(stderr, "ionic: qp opcodes %d want min %d\n",
			ctx->opcodes, IONIC_V1_OP_BIND_MW + 1);
		rc = EINVAL;
		goto err_cmd;
	}

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
	ionic_tbl_init(&ctx->qp_tbl);

	ionic_set_ops(ibctx);

	ctx->lockfree = ionic_env_lockfree();

	if (dev->abi_ver <= 1) {
		ctx->spec = 0;
	} else {
		ctx->spec = ionic_env_spec(resp.max_spec);
		if (ctx->spec < 0 || ctx->spec > 16)
			ctx->spec = 0;
	}

	if (ionic_env_debug())
		ctx->dbg_file = IONIC_DEBUG_FILE;

	level = ionic_env_stats();
	if (level) {
		ctx->stats = calloc(1, sizeof(*ctx->stats));

		if (ctx->stats && level > 1)
			ctx->stats->histogram = 1;
	}

	level = ionic_env_lats();
	if (level) {
		ctx->lats = calloc(1, sizeof(*ctx->lats));

		if (ctx->lats && level > 1)
			ctx->lats->histogram = 1;

		ionic_lat_init(ctx->lats);
	}

err_cmd:
	return rc;
}

static void ionic_uninit_context(struct verbs_device *vdev,
				 struct ibv_context *ibctx)
{
	struct ionic_ctx *ctx = to_ionic_ctx(ibctx);
	int rc;

	rc = ionic_tbl_destroy(&ctx->qp_tbl);
	if (rc)
		ionic_err("context freed before destroying resources");

	pthread_mutex_destroy(&ctx->mut);

	ionic_unmap(ctx->dbpage, 1u << ctx->pg_shift);

	ionic_stats_print(IONIC_DEBUG_FILE, ctx->stats);
	free(ctx->stats);

	ionic_lats_print(IONIC_DEBUG_FILE, ctx->lats);
	free(ctx->lats);
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

	if (ibv_read_sysfs_file(sysfs_path, "device/vendor",
				value, sizeof(value)) < 0)
		return NULL;
	if (sscanf(value, "%i", &vendor) != 1)
		return NULL;

	if (ibv_read_sysfs_file(sysfs_path, "device/device",
				value, sizeof(value)) < 0)
		return NULL;
	if (sscanf(value, "%i", &device) != 1)
		return NULL;

	for (i = 0; cna_table[i].vendor; ++i)
		if (vendor == cna_table[i].vendor &&
		    device == cna_table[i].device)
			goto found;

	return NULL;

found:
	if (abi_version < 1 || abi_version > IONIC_ABI_VERSION)
		return NULL;

	dev = calloc(1, sizeof(*dev));
	if (!dev)
		return NULL;

	dev->vdev.ops = &ionic_dev_ops;
	dev->vdev.sz = sizeof(*dev);
	dev->vdev.size_of_context =
		sizeof(struct ionic_ctx) - sizeof(struct ibv_context);

	dev->abi_ver = abi_version;

	return &dev->vdev;
}

static __attribute__((constructor)) void ionic_register_driver(void)
{
	verbs_register_driver("ionic", ionic_alloc_device);

	static_assert(sizeof(struct ionic_v1_cqe) == 32, "bad size");
	static_assert(sizeof(struct ionic_v1_base_hdr) == 16, "bad size");
	static_assert(sizeof(struct ionic_v1_recv_bdy) == 48, "bad size");
	static_assert(sizeof(struct ionic_v1_common_bdy) == 48, "bad size");
	static_assert(sizeof(struct ionic_v1_atomic_bdy) == 48, "bad size");
	static_assert(sizeof(struct ionic_v1_bind_mw_bdy) == 48, "bad size");
	static_assert(sizeof(struct ionic_v1_wqe) == 64, "bad size");
}
