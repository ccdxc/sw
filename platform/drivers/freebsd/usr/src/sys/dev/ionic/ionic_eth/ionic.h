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

//#define ADMINQ

#define	IONIC_ERROR(fmt, ...)			printf("[%s:%d]" fmt, __func__, __LINE__, ##__VA_ARGS__);

/* Init time debug. */
#ifdef IONIC_DEBUG
#define	IONIC_DEBUG_PRINT(fmt, ...)		printf("[%s:%d]" fmt, __func__, __LINE__, ##__VA_ARGS__);
#else
#define	IONIC_DEBUG_PRINT(fmt, ...)
#endif

/* Device related */
#define	IONIC_DEV_DEBUG(dev, fmt, ...)							\
		device_printf((dev)->bsddev, fmt, ##__VA_ARGS__);

#define IONIC_DEV_TRACE(d, f, args...) \
	IONIC_DEV_DEBUG(d, "%s:%d - " f, __func__, __LINE__, ## args)
#define IONIC_DEV_WARN(d, f, args...) \
	IONIC_DEV_DEBUG(d, "warn(%s:%d):" f, __func__, __LINE__, ## args)
#define IONIC_DEV_ERROR(d, f, args...) \
	IONIC_DEV_DEBUG(d, "warn(%s:%d):" f, __func__, __LINE__, ## args)

/* Netdev related. */
#define IONIC_NETDEV_DEBUG(dev, fmt, ...) if_printf(dev, "NETDEV:" fmt, ##__VA_ARGS__)
#ifdef IONIC_DEBUG
#define IONIC_NETDEV_INFO(dev, fmt, ...) IONIC_NETDEV_DEBUG(dev, "info:" fmt, ##__VA_ARGS__)
#else
#define IONIC_NETDEV_INFO(dev, fmt, ...)
#endif

#define IONIC_NETDEV_WARN(dev, fmt, ...) IONIC_NETDEV_DEBUG(dev, "WARN:" fmt, ##__VA_ARGS__)
#define IONIC_NETDEV_ERROR(dev, fmt, ...) IONIC_NETDEV_DEBUG(dev, "ERROR:" fmt, ##__VA_ARGS__)

/* Netdev Q related. */
#define IONIC_NETDEV_QDEBUG(q, fmt, ...)	do {					\
	if (q && q->lif && q->lif->netdev)	 							\
		if_printf(q->lif->netdev, "[%s:%d]:%s:" fmt, __func__, __LINE__, q->name, ##__VA_ARGS__);	\
} 	while(0)

#ifdef IONIC_DEBUG
#define IONIC_NETDEV_QINFO(q, fmt, ...)	 IONIC_NETDEV_QDEBUG(q, "info:" fmt, ##__VA_ARGS__)
#else
#define IONIC_NETDEV_QINFO(q, fmt, ...)
#endif

#define IONIC_NETDEV_QWARN(q, fmt, ...)	IONIC_NETDEV_QDEBUG(q, "WARN:" fmt, ##__VA_ARGS__)
#define IONIC_NETDEV_QERR(q, fmt, ...)	IONIC_NETDEV_QDEBUG(q, "ERROR:" fmt, ##__VA_ARGS__)

#if defined(IONIC_DEBUG_TX) && defined(IONIC_DEBUG)
#define IONIC_NETDEV_TX_TRACE(q, fmt, ...)	IONIC_NETDEV_QDEBUG(q, "info:" fmt, ##__VA_ARGS__)
#else
#define IONIC_NETDEV_TX_TRACE(q, fmt, ...)
#endif

#if defined(IONIC_DEBUG_RX) && defined(IONIC_DEBUG)
#define IONIC_NETDEV_RX_TRACE(q, fmt, ...)	IONIC_NETDEV_QDEBUG(q, "info:" fmt, ##__VA_ARGS__)
#else
#define IONIC_NETDEV_RX_TRACE(q, fmt, ...)
#endif

#define print_hex_dump_debug(...) 			\
		print_hex_dump(NULL, __VA_ARGS__);

#define DRV_NAME		"ionic"
#define DRV_DESCRIPTION	"Pensando Ethernet NIC Driver"
#define DRV_VERSION		"0.1"

/* XXX : should go to linux/completion.h */
#define	INIT_COMPLETION_ONSTACK(c) \
	((c).done = 0)
#define	COMPLETION_INITIALIZER(c) \
        { .done = 0 }
#define	COMPLETION_INITIALIZER_ONSTACK(c) \
        { .done = 0 }


/* XXX: should go to linux/kernel.h */
#define	IS_ALIGNED(x, y)	(((x) & (y - 1)) == 0)
#define	PAGE_ALIGNED(x)	IS_ALIGNED((unsigned long)x, PAGE_SIZE)

/* XXX: should go to linux/notifier.h */
#define notifier_from_errno(v) NOTIFY_DONE

/* END */

MALLOC_DECLARE(M_IONIC);

#ifndef napi_struct
#define napi_struct work_struct
#endif
struct napi_struct;
struct ionic_dev;

extern const char *ionic_module_dname;

extern int ionic_max_queues;
extern int ntxq_descs;
extern int nrxq_descs;
extern int adminq_descs;

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
	unsigned int neqs_per_lif;
	unsigned int ntxqs_per_lif;
	unsigned int nrxqs_per_lif;
	unsigned int nintrs;
	DECLARE_BITMAP(intrs, INTR_CTRL_REGS_MAX);
};

int ionic_adminq_check_err(struct lif *lif, struct ionic_admin_ctx *ctx);
int ionic_adminq_post_wait(struct lif *lif, struct ionic_admin_ctx *ctx);

int ionic_dev_cmd_wait_check(struct ionic_dev *idev, unsigned long max_wait);
int ionic_set_dma_mask(struct ionic *ionic);
int ionic_setup(struct ionic *ionic);
int ionic_identify(struct ionic *ionic);
void ionic_forget_identity(struct ionic *ionic);
int ionic_reset(struct ionic *ionic);

#endif /* _IONIC_H_ */
