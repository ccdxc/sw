/*
 * Copyright 2017-2018 Pensando Systems, Inc.  All rights reserved.
 *
 * This program is free software; you may redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef _IONIC_H_
#define _IONIC_H_

#include "ionic_dev.h"
#include "ionic_api.h"
#include "ionic_kpicompat.h"

#define DRV_NAME		"ionic"
#define DRV_DESCRIPTION	"Pensando Ethernet NIC Driver"
#define DRV_VERSION		"0.1"

#define SIZE_1K				1024
#define IONIX_TX_MAX_DESC	(64 * SIZE_1K)
#define IONIX_RX_MAX_DESC	(64 * SIZE_1K)

/* TSO DMA related definitions. */
/* XXX: these are arbitrary, we are not limited in h/w. */
#define IONIC_MAX_TSO_SEG 		32
#define IONIC_MAX_TSO_SEG_SIZE 	(64 * SIZE_1K)
#define IONIC_MAX_TSO_SIZE 		(16 * IONIC_MAX_TSO_SEG_SIZE)

MALLOC_DECLARE(M_IONIC);

/* Init time debug. */
#ifdef IONIC_DEBUG
#define	IONIC_INFO(fmt, ...)								\
		printf("[%s:%d]" fmt, __func__, __LINE__, ##__VA_ARGS__);
#else
#define	IONIC_INFO(fmt, ...)
#endif
#define	IONIC_ERROR(fmt, ...)								\
			printf("[%s:%d]" fmt, __func__, __LINE__, ##__VA_ARGS__);

/* Device related */
#define	IONIC_DEV_DEBUG(dev, fmt, ...)							\
		device_printf((dev)->bsddev, fmt, ##__VA_ARGS__);

#ifdef IONIC_DEBUG
#define IONIC_DEV_INFO(d, f, args...) 						\
		IONIC_DEV_DEBUG(d, "[%s:%d]" f, __func__, __LINE__, ## args)
#else
#define IONIC_DEV_INFO(d, f, args...)
#endif

#define IONIC_DEV_WARN(d, f, args...) 						\
		IONIC_DEV_DEBUG(d, "[%s:%d]WARN:" f, __func__, __LINE__, ## args)
#define IONIC_DEV_ERROR(d, f, args...) 						\
		IONIC_DEV_DEBUG(d, "[%s:%d]ERROR:" f, __func__, __LINE__, ## args)

/* Netdev related. */
#define IONIC_NETDEV_DEBUG(dev, fmt, ...) 					\
			if_printf(dev, "[%s:%d]" fmt, __func__, __LINE__, ##__VA_ARGS__)

#ifdef IONIC_DEBUG
#define IONIC_NETDEV_INFO(dev, fmt, ...) 						\
		IONIC_NETDEV_DEBUG(dev, fmt, ##__VA_ARGS__)
#else
#define IONIC_NETDEV_INFO(dev, fmt, ...)
#endif

#define IONIC_NETDEV_WARN(dev, fmt, ...) 						\
		IONIC_NETDEV_DEBUG(dev, "WARN:" fmt, ##__VA_ARGS__)
#define IONIC_NETDEV_ERROR(dev, fmt, ...) 						\
		IONIC_NETDEV_DEBUG(dev, "ERROR:" fmt, ##__VA_ARGS__)

/* Print the MAC address. */
#ifdef __FreeBSD__
#define IONIC_NETDEV_ADDR_INFO(dev, addr, fmt, ...) 			\
		IONIC_NETDEV_INFO(dev, fmt " MAC:%6D\n", ##__VA_ARGS__, (addr), ":")
#else
#define IONIC_NETDEV_ADDR_INFO(dev, addr, fmt, ...) 			\
		IONIC_NETDEV_INFO(dev, fmt " %pM\n", ##__VA_ARGS__, (addr))
#endif

/* Netdevice queue related macros. */
#define IONIC_QUE_DEBUG(q, fmt, ...)							\
		if_printf(q->lif->netdev, "[%s:%d]%s:" fmt, 			\
			__func__, __LINE__, q->name, ##__VA_ARGS__);

#ifdef IONIC_DEBUG
#define IONIC_QUE_INFO(q, fmt, ...)	 							\
		IONIC_QUE_DEBUG(q, "info:" fmt, ##__VA_ARGS__)
#else
#define IONIC_QUE_INFO(q, fmt, ...)
#endif

#define IONIC_QUE_WARN(q, fmt, ...)								\
		IONIC_QUE_DEBUG(q, "WARN:" fmt, ##__VA_ARGS__)
#define IONIC_QUE_ERROR(q, fmt, ...)							\
		IONIC_QUE_DEBUG(q, "ERROR:" fmt, ##__VA_ARGS__)

#if defined(IONIC_ENABLE_TRACING)
#define IONIC_TX_TRACE(q, fmt, ...)		IONIC_QUE_DEBUG(q, fmt, ##__VA_ARGS__)
#define IONIC_RX_TRACE(q, fmt, ...)		IONIC_QUE_DEBUG(q, fmt, ##__VA_ARGS__)
#else
#define IONIC_TX_TRACE(q, fmt, ...)
#define IONIC_RX_TRACE(q, fmt, ...)
#endif

struct ionic_dev;

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
};

int ionic_adminq_check_err(struct lif *lif, struct ionic_admin_ctx *ctx, bool);
int ionic_adminq_post_wait(struct lif *lif, struct ionic_admin_ctx *ctx);

int ionic_dev_cmd_wait_check(struct ionic_dev *idev, unsigned long max_wait);
int ionic_set_dma_mask(struct ionic *ionic);
int ionic_setup(struct ionic *ionic);
int ionic_identify(struct ionic *ionic);
void ionic_forget_identity(struct ionic *ionic);
int ionic_reset(struct ionic *ionic);

#endif /* _IONIC_H_ */
