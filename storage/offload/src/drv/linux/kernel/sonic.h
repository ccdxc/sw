/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef _SONIC_H_
#define _SONIC_H_

#include <linux/jhash.h>
#include "osal.h"
#include "sonic_dev.h"
#include "sonic_debugfs.h"


#define ADMINQ			1

#define DRV_NAME		"sonic"
#define DRV_DESCRIPTION		"Pensando Offload Driver"
#define DRV_VERSION		"0.1"

/*
 * Special error codes for certain well known sonic utility functions
 */
#define SONIC_DEVCMD_UNKNOWN		(-ENOENT)
#define SONIC_DEVCMD_ERROR		(-EIO)
#define SONIC_SEQ_Q_BATCH_KEY_EXIST	(-EEXIST)

extern unsigned int devcmd_timeout;
extern unsigned int core_count;

#ifdef __FreeBSD__

#define devm_kzalloc(dev, x, y)		kzalloc(x, y)
#define devm_kfree(dev, x)		kfree(x)

#define devm_request_irq(dev, x, ...)	request_irq(x, ##__VA_ARGS__)
#define devm_free_irq(dev, x, y)	free_irq(x, y)

#ifndef print_hex_dump_debug
#define print_hex_dump_debug(...)			\
		print_hex_dump(NULL, __VA_ARGS__);
#endif

#ifndef dynamic_hex_dump
#define dynamic_hex_dump(...)			\
		print_hex_dump(NULL, __VA_ARGS__);
#endif

#ifndef napi_struct
#define napi_struct work_struct
#endif

#define netif_napi_add(n, napi, poll, wt) do {	\
	(void)(n);				\
	(void)(wt);				\
	INIT_WORK(napi, poll);			\
} while (0)

#define netif_napi_del(napi) do {	\
	(void)(napi);			\
	flush_scheduled_work();	\
} while (0)

#define napi_enable(napi) do {	\
	(void)(napi);			\
} while (0)
#define napi_disable cancel_work_sync
#define napi_schedule schedule_work
#define napi_schedule_irqoff schedule_work
#define napi_complete_done(napi, done) ((void)(napi), (void)(done), 1)

#define NAPI_POLL_WEIGHT 64

#define	COMPLETION_INITIALIZER_ONSTACK(c) \
	{ .done = 0 }
#endif

struct sonic {
	struct pci_dev *pdev;
	struct platform_device *pfdev;
	struct device *dev;
	struct sonic_dev idev;
	struct dentry *dentry;
	struct sonic_dev_bar bars[SONIC_BARS_MAX];
	unsigned int num_bars;
	union identity *ident;
	dma_addr_t ident_pa;
	struct list_head lifs;
	unsigned int num_per_core_resources;
	unsigned int nintrs;
	DECLARE_BITMAP(intrs, INTR_CTRL_REGS_MAX);
#ifdef CONFIG_DEBUG_FS
#ifdef DEBUGFS_TEST_API
	void *scratch_bufs[NUM_SCRATCH_BUFS];
	dma_addr_t scratch_bufs_pa[NUM_SCRATCH_BUFS];
	struct debugfs_blob_wrapper scratch_bufs_blob[NUM_SCRATCH_BUFS];
#endif
#endif
#ifndef ADMINQ
	spinlock_t cmd_lock;
	struct list_head cmd_list;
	struct work_struct cmd_work;
#endif
};

/** sonic_admin_ctx - Admin command context.
 * @work:   Work completion wait queue element.
 * @cmd:    Admin command (64B) to be copied to the queue.
 * @comp:   Admin completion (16B) copied from the queue.
 *
 * @side_data:    Additional data to be copied to the doorbell page,
 *        if the command is issued as a dev cmd.
 * @side_data_len:  Length of additional data to be copied.
 *
 * TODO:
 * The side_data and side_data_len are temporary and will be removed.  For now,
 * they are used when admin commands referring to side-band data are posted as
 * dev commands instead.  Only single-indirect side-band data is supported.
 * Only 2K of data is supported, because first half of page is for registers.
 */
struct sonic_admin_ctx {
  struct completion work;
  union adminq_cmd cmd;
  union adminq_cpl comp;

#ifndef ADMINQ
  struct list_head list;
#endif
  void *side_data;
  size_t side_data_len;
};


int sonic_adminq_check_err(struct lif *lif, struct sonic_admin_ctx *ctx);
int sonic_adminq_post_wait(struct lif *lif, struct sonic_admin_ctx *ctx);
int sonic_napi(struct napi_struct *napi, int budget, sonic_cq_cb cb,
	       void *cb_arg);
int sonic_dev_cmd_wait_check(struct sonic_dev *idev, unsigned long max_wait);
int sonic_set_dma_mask(struct sonic *sonic);
int sonic_setup(struct sonic *sonic);
int sonic_identify(struct sonic *sonic);
void sonic_forget_identity(struct sonic *sonic);
int sonic_reset(struct sonic *sonic);
int sonic_api_adminq_post(struct lif *lif, struct sonic_admin_ctx *ctx);

#endif /* _SONIC_H_ */
