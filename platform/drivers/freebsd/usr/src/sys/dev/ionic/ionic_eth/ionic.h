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
#include "ionic_osdep.h"

#define DRV_NAME			"ionic"
#define DRV_DESCRIPTION		"Pensando Ethernet NIC Driver"
#define DRV_VERSION			"0.8.0"

#define SIZE_1K			1024
#define IONIX_TX_MIN_DESC	64
#define IONIX_TX_MAX_DESC	(16 * SIZE_1K)
#define IONIX_RX_MIN_DESC	64
#define IONIX_RX_MAX_DESC	(16 * SIZE_1K)

#define IONIC_ADDR_BITS		52
#define IONIC_MAX_ADDR		(BIT_ULL(IONIC_ADDR_BITS) - 1)
/* TSO DMA related definitions. */
#define IONIC_MAX_TSO_SG_ENTRIES 	64
#define IONIC_MAX_TSO_SG_SIZE 		(64 * SIZE_1K)
#define IONIC_MAX_TSO_SIZE 		(64 * SIZE_1K)

#define QUEUE_NAME_MAX_SZ		8
MALLOC_DECLARE(M_IONIC);

/* Init time debug. */
#ifdef IONIC_DEBUG
#define	IONIC_INFO(fmt, ...)							\
		printf("[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__);
#else
#define	IONIC_INFO(fmt, ...)
#endif
#define	IONIC_ERROR(fmt, ...)							\
		printf("[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__);

/* Device related */
#define	IONIC_DEV_DEBUG(dev, fmt, ...)						\
		device_printf((dev)->bsddev, fmt, ##__VA_ARGS__);

#ifdef IONIC_DEBUG
#define IONIC_DEV_INFO(d, f, args...) 						\
		IONIC_DEV_DEBUG(d, "[%s:%d] " f, __func__, __LINE__, ## args)
#else
#define IONIC_DEV_INFO(d, f, args...)
#endif

#define IONIC_DEV_WARN(d, f, args...) 						\
		IONIC_DEV_DEBUG(d, "[%s:%d] " f, __func__, __LINE__, ## args)
#define IONIC_DEV_ERROR(d, f, args...) 						\
		IONIC_DEV_DEBUG(d, "[%s:%d] " f, __func__, __LINE__, ## args)

/* Netdev related. */
#define IONIC_NETDEV_DEBUG(dev, fmt, ...) 					\
		if_printf(dev, "[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)

#ifdef IONIC_DEBUG
#define IONIC_NETDEV_INFO(dev, fmt, ...) 					\
		IONIC_NETDEV_DEBUG(dev, fmt, ##__VA_ARGS__)
#else
#define IONIC_NETDEV_INFO(dev, fmt, ...)
#endif

#define IONIC_NETDEV_WARN(dev, fmt, ...) 					\
		IONIC_NETDEV_DEBUG(dev, fmt, ##__VA_ARGS__)
#define IONIC_NETDEV_ERROR(dev, fmt, ...) 					\
		IONIC_NETDEV_DEBUG(dev, fmt, ##__VA_ARGS__)

/* Print the MAC address. */
#define IONIC_NETDEV_ADDR_INFO(dev, addr, fmt, ...)				\
		IONIC_NETDEV_INFO(dev, fmt " MAC: %6D\n", ##__VA_ARGS__, (addr), ":")
#define IONIC_NETDEV_ADDR_DEBUG(dev, addr, fmt, ...)				\
		IONIC_NETDEV_DEBUG(dev, fmt " MAC: %6D\n", ##__VA_ARGS__, (addr), ":")
#define IONIC_NETDEV_ADDR_WARN(dev, addr, fmt, ...)				\
		IONIC_NETDEV_WARN(dev, fmt " MAC: %6D\n", ##__VA_ARGS__, (addr), ":")
#define IONIC_NETDEV_ADDR_ERROR(dev, addr, fmt, ...)				\
		IONIC_NETDEV_ERROR(dev, fmt " MAC: %6D\n", ##__VA_ARGS__, (addr), ":")

/* Netdevice queue related macros. */
#define IONIC_QUE_DEBUG(q, fmt, ...)						\
		if_printf(q->lif->netdev, "[%s:%d] %s: " fmt, 			\
			__func__, __LINE__, q->name, ##__VA_ARGS__);

#ifdef IONIC_DEBUG
#define IONIC_QUE_INFO(q, fmt, ...)	 					\
		IONIC_QUE_DEBUG(q, "info: " fmt, ##__VA_ARGS__)
#else
#define IONIC_QUE_INFO(q, fmt, ...)
#endif

#define IONIC_QUE_WARN(q, fmt, ...)						\
		IONIC_QUE_DEBUG(q, fmt, ##__VA_ARGS__)
#define IONIC_QUE_ERROR(q, fmt, ...)						\
		IONIC_QUE_DEBUG(q, fmt, ##__VA_ARGS__)

#if defined(IONIC_ENABLE_TRACING)
#define IONIC_TX_TRACE(q, fmt, ...)	IONIC_QUE_DEBUG(q, fmt, ##__VA_ARGS__)
#define IONIC_RX_TRACE(q, fmt, ...)	IONIC_QUE_DEBUG(q, fmt, ##__VA_ARGS__)
#else
#define IONIC_TX_TRACE(q, fmt, ...)
#define IONIC_RX_TRACE(q, fmt, ...)
#endif

struct ionic_dev;

/*
 * @cos: valid value 0-7
 * @weight: 0-99.
 */
struct ionic_qos_tc {
	bool enable;
	bool drop;
	uint8_t dot1q_pcp;
	uint8_t dwrr_weight;
	uint32_t mtu;
};

struct ionic {
	struct pci_dev *pdev;
	struct platform_device *pfdev;
	struct device *dev;
	struct ionic_dev idev;
	struct dentry *dentry;
	struct ionic_dev_bar bars[IONIC_BARS_MAX];
	struct ionic_dma_info port_dma;
	unsigned int num_bars;
	struct identity ident;
	struct list_head lifs;
	bool is_mgmt_nic;

	struct sx sx;
	char sx_name[QUEUE_NAME_MAX_SZ];
	unsigned int nnqs_per_lif;
	unsigned int neqs_per_lif;
	unsigned int ntxqs_per_lif;
	unsigned int nrxqs_per_lif;
	unsigned int nintrs;

	/* QoS software config. */
	struct ionic_qos_tc qos_tc[IONIC_QOS_CLASS_MAX];

	DECLARE_BITMAP(intrs, INTR_CTRL_REGS_MAX);
};

#define IONIC_DEV_LOCK_INIT(x)		sx_init(&(x)->sx, (x)->sx_name)
#define IONIC_DEV_LOCK_DESTROY(x)	sx_destroy(&(x)->sx)
#define IONIC_DEV_LOCK(x)		sx_xlock(&(x)->sx)
#define IONIC_DEV_UNLOCK(x)		sx_xunlock(&(x)->sx)
#define IONIC_DEV_LOCK_OWNED(x)		sx_xlocked(&(x)->sx)

/* Valid on struct ionic_device and struct lif */
#define IONIC_WDOG_LOCK_INIT(x)		mtx_init(&(x)->wdog_mtx, \
						 "wdog_mtx", NULL, MTX_DEF)
#define IONIC_WDOG_LOCK_DESTROY(x)	mtx_destroy(&(x)->wdog_mtx)
#define IONIC_WDOG_LOCK(x)		mtx_lock(&(x)->wdog_mtx);
#define IONIC_WDOG_UNLOCK(x)		mtx_unlock(&(x)->wdog_mtx);
#define IONIC_WDOG_LOCK_OWNED(x)	mtx_owned(&(x)->wdog_mtx)

int ionic_adminq_check_err(struct lif *lif, struct ionic_admin_ctx *ctx, bool);
int ionic_adminq_post_wait(struct lif *lif, struct ionic_admin_ctx *ctx);

int ionic_dev_cmd_wait_check(struct ionic_dev *idev, unsigned long max_wait);
int ionic_dev_cmd_sleep_check(struct ionic_dev *idev, unsigned long max_wait);
int ionic_set_dma_mask(struct ionic *ionic);
int ionic_setup(struct ionic *ionic);

int ionic_identify(struct ionic *ionic);
int ionic_init(struct ionic *ionic);
int ionic_reset(struct ionic *ionic);

int ionic_port_identify(struct ionic *ionic);
int ionic_port_init(struct ionic *ionic);
int ionic_port_reset(struct ionic *ionic);
void ionic_set_port_state(struct ionic *ionic, uint8_t state);

int ionic_qos_class_identify(struct ionic *ionic);
int ionic_qos_class_init(struct ionic *ionic, uint8_t group,
	union qos_config *config);
int ionic_qos_class_reset(struct ionic *ionic, uint8_t group);

#endif /* _IONIC_H_ */
