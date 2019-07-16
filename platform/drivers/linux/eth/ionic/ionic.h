/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2017 - 2019 Pensando Systems, Inc */

#ifndef _IONIC_H_
#define _IONIC_H_

#include "kcompat.h"

#include "ionic_if.h"
#include "ionic_dev.h"
#include "ionic_devlink.h"

#define DRV_NAME		"ionic"
#define DRV_DESCRIPTION		"Pensando Ethernet NIC Driver"
#define DRV_VERSION		"0.14.0"

// TODO: register these with the official include/linux/pci_ids.h
#define PCI_VENDOR_ID_PENSANDO			0x1dd8

#define PCI_DEVICE_ID_PENSANDO_IONIC_ETH_PF	0x1002
#define PCI_DEVICE_ID_PENSANDO_IONIC_ETH_VF	0x1003
#define PCI_DEVICE_ID_PENSANDO_IONIC_ETH_MGMT	0x1004

#define IONIC_SUBDEV_ID_NAPLES_25	0x4000
#define IONIC_SUBDEV_ID_NAPLES_100_4	0x4001
#define IONIC_SUBDEV_ID_NAPLES_100_8	0x4002

extern unsigned int max_slaves;
extern unsigned int rx_copybreak;
extern unsigned int devcmd_timeout;

struct ionic {
	struct pci_dev *pdev;
	struct platform_device *pfdev;
	struct device *dev;
	struct ionic_dev idev;
	struct mutex dev_cmd_lock;	/* lock for dev_cmd operations */
	struct dentry *dentry;
	struct ionic_dev_bar bars[IONIC_BARS_MAX];
	unsigned int num_bars;
	bool is_mgmt_nic;
	struct lif *master_lif;
	struct list_head lifs;
	struct identity ident;
	unsigned int nnqs_per_lif;
	unsigned int neqs_per_lif;
	unsigned int ntxqs_per_lif;
	unsigned int nrxqs_per_lif;
	unsigned int nslaves;
	DECLARE_BITMAP(lifbits, IONIC_LIFS_MAX);
	unsigned int nintrs;
	DECLARE_BITMAP(intrs, INTR_CTRL_REGS_MAX);
#ifndef HAVE_PCI_IRQ_API
	struct msix_entry *msix;
#endif
	struct work_struct nb_work;
	struct notifier_block nb;
#ifdef IONIC_DEVLINK
	struct devlink *dl;
	struct devlink_port dl_port;
#endif

	struct timer_list watchdog_timer;
	int watchdog_period;
};

int ionic_napi(struct napi_struct *napi, int budget, ionic_cq_cb cb,
	       ionic_cq_done_cb done_cb, void *done_arg);

int ionic_adminq_post(struct lif *lif, struct ionic_admin_ctx *ctx);
int ionic_adminq_post_wait(struct lif *lif, struct ionic_admin_ctx *ctx);
int ionic_dev_cmd_wait(struct ionic *ionic, unsigned long max_wait);
int ionic_set_dma_mask(struct ionic *ionic);
int ionic_setup(struct ionic *ionic);

int ionic_identify(struct ionic *ionic);
int ionic_init(struct ionic *ionic);
int ionic_reset(struct ionic *ionic);

int ionic_port_identify(struct ionic *ionic);
int ionic_port_init(struct ionic *ionic);
int ionic_port_reset(struct ionic *ionic);

#endif /* _IONIC_H_ */
