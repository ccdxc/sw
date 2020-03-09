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

struct ionic_lif;

#include "ionic_dev.h"
#include "ionic_api.h"
#include "ionic_kpicompat.h"
#include "ionic_osdep.h"

#define DRV_NAME		"ionic"
#define DRV_DESCRIPTION		"Pensando Ethernet NIC Driver"
#define DRV_VERSION		"0.0.0"

#define SIZE_1K			1024
#define IONIX_TX_MIN_DESC	64
#define IONIX_TX_MAX_DESC	(16 * SIZE_1K)
#define IONIX_RX_MIN_DESC	64
#define IONIX_RX_MAX_DESC	(16 * SIZE_1K)

#define IONIC_ADDR_BITS		52
#define IONIC_MAX_ADDR		(BIT_ULL(IONIC_ADDR_BITS) - 1)

/* This should be the largest SGL size supported for any descriptor */
#define IONIC_MAX_SEGMENTS	(IONIC_TX_MAX_SG_ELEMS_V1 + 1)

/* TSO DMA related definitions. */
#define IONIC_MAX_TSO_SEG_SIZE 	(64 * SIZE_1K)
#define IONIC_MAX_TSO_SIZE 	(64 * SIZE_1K)
#define IONIC_MAX_TSO_SEGMENTS	(64)


#define IONIC_QUEUE_NAME_MAX_SZ	8
MALLOC_DECLARE(M_IONIC);

#ifndef IONIC_NDEBUG
extern int ionic_debug;
extern int ionic_trace;
extern int ionic_tso_debug;
#define __IONIC_DEBUG ionic_debug
#define __IONIC_TRACE ionic_trace
#define __IONIC_TSO_DEBUG ionic_tso_debug
#else
#define __IONIC_DEBUG false
#define __IONIC_TRACE false
#define __IONIC_TSO_DEBUG false
#endif

#define	IONIC_PRINT(fmt, args...)					\
	printf("[%s:%d] " fmt, __func__, __LINE__, ##args);

#define	IONIC_ERROR		IONIC_PRINT
#define	IONIC_WARN		IONIC_PRINT
#define	IONIC_DEBUG		IONIC_PRINT

#define	IONIC_INFO(args...) do {					\
	if (__IONIC_DEBUG)						\
		IONIC_PRINT(args);					\
} while (0)

/* Device related */

#define	IONIC_DEV_PRINT(dev, fmt, args...)				\
	device_printf((dev)->bsddev, "[%s:%d] " fmt,			\
		      __func__, __LINE__, ##args)

#define IONIC_DEV_ERROR		IONIC_DEV_PRINT
#define IONIC_DEV_WARN		IONIC_DEV_PRINT
#define IONIC_DEV_DEBUG		IONIC_DEV_PRINT

#define	IONIC_DEV_INFO(args...) do {					\
	if (__IONIC_DEBUG)						\
		IONIC_DEV_PRINT(args);					\
} while (0)

/* Netdev related. */

#define IONIC_NETDEV_PRINT(ndev, fmt, ...)				\
	if_printf(ndev, "[%s:%d] " fmt,					\
		  __func__, __LINE__, ##__VA_ARGS__)

#define IONIC_NETDEV_ERROR	IONIC_NETDEV_PRINT
#define IONIC_NETDEV_WARN	IONIC_NETDEV_PRINT
#define IONIC_NETDEV_DEBUG	IONIC_NETDEV_PRINT

#define	IONIC_NETDEV_INFO(args...) do {					\
	if (__IONIC_DEBUG)						\
		IONIC_NETDEV_PRINT(args);				\
} while (0)

/* Print the MAC address. */

#define IONIC_NETDEV_ADDR_PRINT(ndev, addr, fmt, args...)		\
	IONIC_NETDEV_PRINT(ndev, "MAC: %6D " fmt,			\
			   (addr), ":", ##args)


#define IONIC_NETDEV_ADDR_ERROR	IONIC_NETDEV_ADDR_PRINT
#define IONIC_NETDEV_ADDR_WARN	IONIC_NETDEV_ADDR_PRINT
#define IONIC_NETDEV_ADDR_DEBUG	IONIC_NETDEV_ADDR_PRINT

#define	IONIC_NETDEV_ADDR_INFO(args...) do {				\
	if (__IONIC_DEBUG)						\
		IONIC_NETDEV_ADDR_PRINT(args);				\
} while (0)

/* Netdevice queue related macros. */

#define IONIC_QUE_PRINT(q, fmt, args...)				\
	IONIC_NETDEV_PRINT(q->lif->netdev, "%s: " fmt,			\
			   q->name, ##args)

#define IONIC_QUE_ERROR	IONIC_QUE_PRINT
#define IONIC_QUE_WARN	IONIC_QUE_PRINT
#define IONIC_QUE_DEBUG	IONIC_QUE_PRINT

#define	IONIC_QUE_INFO(args...) do {					\
	if (__IONIC_DEBUG)						\
		IONIC_QUE_PRINT(args);					\
} while (0)

#define	IONIC_QUE_TRACE(args...) do {					\
	if (__IONIC_TRACE)						\
		IONIC_QUE_PRINT(args);					\
} while (0)

#define	IONIC_RX_TRACE	IONIC_QUE_TRACE
#define	IONIC_TX_TRACE	IONIC_QUE_TRACE

struct ionic_dev;

struct ionic_qos {
	int max_tcs;
	enum ionic_qos_class_type class_type;
	/* Per TC value. */
	uint8_t dwrr_bw_perc[IONIC_QOS_TC_MAX];
	uint8_t enable_flag[IONIC_QOS_TC_MAX];
	uint8_t no_drop[IONIC_QOS_TC_MAX];
	uint8_t pfc_cos[IONIC_QOS_TC_MAX];
	uint8_t sched_type[IONIC_QOS_TC_MAX];
	/* PCP to TC map. */
	uint8_t pcp_to_tc[IONIC_QOS_PCP_MAX];
	/* DSCP to TC map. */
	uint8_t dscp_to_tc[IONIC_QOS_DSCP_MAX];
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
	struct ionic_identity ident;
	struct list_head lifs;
	bool is_mgmt_nic;

	struct sx sx;
	char sx_name[IONIC_QUEUE_NAME_MAX_SZ];
	unsigned int nnqs_per_lif;
	unsigned int nrdma_eqs_per_lif;
	unsigned int ntxqs_per_lif;
	unsigned int nrxqs_per_lif;
	unsigned int nintrs;
	unsigned int nlifs;

	/* QoS software config. */
	struct ionic_qos qos;

	DECLARE_BITMAP(lifbits, IONIC_LIFS_MAX);
	DECLARE_BITMAP(ethbits, IONIC_LIFS_MAX);
	DECLARE_BITMAP(intrs, IONIC_INTR_CTRL_REGS_MAX);
};

#define IONIC_DEV_LOCK_INIT(x)		sx_init(&(x)->sx, (x)->sx_name)
#define IONIC_DEV_LOCK_DESTROY(x)	sx_destroy(&(x)->sx)
#define IONIC_DEV_LOCK(x)		sx_xlock(&(x)->sx)
#define IONIC_DEV_UNLOCK(x)		sx_xunlock(&(x)->sx)
#define IONIC_DEV_LOCK_OWNED(x)		sx_xlocked(&(x)->sx)

/* Valid on struct ionic_device and struct ionic_lif */
#define IONIC_WDOG_LOCK_INIT(x)		mtx_init(&(x)->wdog_mtx, \
						 "wdog_mtx", NULL, MTX_DEF)
#define IONIC_WDOG_LOCK_DESTROY(x)	mtx_destroy(&(x)->wdog_mtx)
#define IONIC_WDOG_LOCK(x)		mtx_lock(&(x)->wdog_mtx);
#define IONIC_WDOG_UNLOCK(x)		mtx_unlock(&(x)->wdog_mtx);
#define IONIC_WDOG_LOCK_OWNED(x)	mtx_owned(&(x)->wdog_mtx)

int ionic_adminq_post_wait(struct ionic_lif *lif, struct ionic_admin_ctx *ctx);

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
int ionic_qos_init(struct ionic *ionic);
int ionic_qos_class_init(struct ionic *ionic, uint8_t group,
    union ionic_qos_config *config);
int ionic_qos_class_reset(struct ionic *ionic, uint8_t group);
int ionic_qos_class_type_update(struct ionic_lif *lif,
    enum ionic_qos_class_type class);
int ionic_qos_enable_update(struct ionic_lif *lif, uint8_t *enable);
int ionic_qos_no_drop_update(struct ionic_lif *lif, uint8_t *no_drop);
int ionic_qos_pfc_cos_update(struct ionic_lif *lif, uint8_t *pfc_cos);
int ionic_qos_sched_type_update(struct ionic_lif *lif, uint8_t *sched);
#ifdef notyet
int ionic_qos_bw_update(struct ionic_lif *lif, uint8_t *bw_perc);
#endif
int ionic_qos_pcp_to_tc_update(struct ionic_lif *lif, uint8_t *pcp);
int ionic_qos_dscp_to_tc_update(struct ionic_lif *lif, uint8_t *pcp);

#endif /* _IONIC_H_ */
