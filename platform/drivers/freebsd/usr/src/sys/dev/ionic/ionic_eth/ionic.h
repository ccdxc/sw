/*
 * Copyright (c) 2017-2019 Pensando Systems, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _IONIC_H_
#define _IONIC_H_

#include "ionic_dev.h"
#include "ionic_api.h"
#include "ionic_kpicompat.h"

#define DRV_NAME		"ionic"
#define DRV_DESCRIPTION	"Pensando Ethernet NIC Driver"
#define DRV_VERSION		"0.2.0"

#define SIZE_1K				1024
#define IONIX_TX_MAX_DESC	(64 * SIZE_1K)
#define IONIX_RX_MAX_DESC	(64 * SIZE_1K)

/* TSO DMA related definitions. */
#define IONIC_MAX_TSO_SG_ENTRIES 	(IONIC_TX_MAX_SG_ELEMS)
#define IONIC_MAX_TSO_SG_SIZE 		(64 * SIZE_1K)
#define IONIC_MAX_TSO_SIZE 		(64 * SIZE_1K)

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
		IONIC_DEV_DEBUG(d, "[%s:%d]" f, __func__, __LINE__, ## args)
#define IONIC_DEV_ERROR(d, f, args...) 						\
		IONIC_DEV_DEBUG(d, "[%s:%d]" f, __func__, __LINE__, ## args)

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
		IONIC_NETDEV_DEBUG(dev, fmt, ##__VA_ARGS__)
#define IONIC_NETDEV_ERROR(dev, fmt, ...) 						\
		IONIC_NETDEV_DEBUG(dev, fmt, ##__VA_ARGS__)

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
		IONIC_QUE_DEBUG(q, fmt, ##__VA_ARGS__)
#define IONIC_QUE_ERROR(q, fmt, ...)							\
		IONIC_QUE_DEBUG(q, fmt, ##__VA_ARGS__)

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
