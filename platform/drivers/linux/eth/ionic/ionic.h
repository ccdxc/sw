/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2017 - 2019 Pensando Systems, Inc */

#ifndef _IONIC_H_
#define _IONIC_H_

#include "ionic_api.h"
#include "ionic_dev.h"
#include "ionic_debugfs.h"

#define DRV_NAME		"ionic"
#define DRV_DESCRIPTION		"Pensando Ethernet NIC Driver"
#define DRV_VERSION		"0.2.0"

extern unsigned int ntxq_descs;
extern unsigned int nrxq_descs;
extern unsigned int ntxqs;
extern unsigned int nrxqs;
extern unsigned int devcmd_timeout;

struct ionic {
	struct pci_dev *pdev;
	struct platform_device *pfdev;
	struct device *dev;
	struct ionic_dev idev;
	struct dentry *dentry;
	struct ionic_dev_bar bars[IONIC_BARS_MAX];
	unsigned int num_bars;
	union identity *ident;
	dma_addr_t ident_pa;
	struct list_head lifs;
	bool is_mgmt_nic;
	unsigned int nnqs_per_lif;
	unsigned int neqs_per_lif;
	unsigned int ntxqs_per_lif;
	unsigned int nrxqs_per_lif;
	unsigned int nintrs;
	DECLARE_BITMAP(intrs, INTR_CTRL_REGS_MAX);
#ifdef CONFIG_DEBUG_FS
#ifdef DEBUGFS_TEST_API
	void *scratch_bufs[NUM_SCRATCH_BUFS];
	dma_addr_t scratch_bufs_pa[NUM_SCRATCH_BUFS];
	struct debugfs_blob_wrapper scratch_bufs_blob[NUM_SCRATCH_BUFS];
#endif
#endif
	struct work_struct nb_work;
	struct notifier_block nb;
};

int ionic_adminq_check_err(struct lif *lif, struct ionic_admin_ctx *ctx,
			   bool timeout);
int ionic_adminq_post_wait(struct lif *lif, struct ionic_admin_ctx *ctx);
int ionic_napi(struct napi_struct *napi, int budget, ionic_cq_cb cb,
	       void *cb_arg);
int ionic_dev_cmd_wait_check(struct ionic_dev *idev, unsigned long max_wait);
int ionic_set_dma_mask(struct ionic *ionic);
int ionic_setup(struct ionic *ionic);
int ionic_identify(struct ionic *ionic);
void ionic_forget_identity(struct ionic *ionic);
int ionic_reset(struct ionic *ionic);

#endif /* _IONIC_H_ */
