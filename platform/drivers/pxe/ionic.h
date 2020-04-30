/*
 * Copyright 2017-2019 Pensando Systems, Inc.  All rights reserved.
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

#ifndef _IONIC_H
#define _IONIC_H

FILE_LICENCE(GPL2_OR_LATER_OR_UBDL);

#include <errno.h>
#include <ipxe/io.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <byteswap.h>
#include <ipxe/netdevice.h>
#include <ipxe/ethernet.h>
#include <ipxe/if_ether.h>
#include <ipxe/iobuf.h>
#include <ipxe/malloc.h>
#include <ipxe/pci.h>

#include "ionic_base.h"

/* Supply these for ionic_if.h */
#define BIT(n)                  (1 << n)
#define BIT_ULL(nr)             (1ULL << (nr))
#define TEST_BIT(x, n)          ((x) & (1 << n))
#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define __le16 uint16_t
#define __le32 uint32_t
#define __le64 uint64_t
#define dma_addr_t uint64_t
typedef enum
{
	false = 0,
	true = 1
} bool;

#include "ionic_if.h"

#ifndef ERRFILE_ionic
#define ERRFILE_ionic ( ERRFILE_DRIVER | 0x00cc0000 )
#endif
#undef ERRFILE
#define ERRFILE ERRFILE_ionic

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#define __iomem
#define is_power_of_2(x) ((x) != 0 && (((x) & ((x)-1)) == 0))

#define ALIGN(x, a) __ALIGN_MASK(x, (typeof(x))(a)-1)
#define __ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))

#define IONIC_MAX_MTU 5000

// Queue alignment
#define IDENTITY_ALIGN 4096

// Dev Command related defines
#define devcmd_timeout 30

#define RETRY_COUNT 30

#define IONIC_ASIC_TYPE_CAPRI 0

// Q flags
#define QCQ_F_INITED BIT(0)
#define QCQ_F_SG BIT(1)
#define QCQ_F_INTR BIT(2)
#define QCQ_F_TX_STATS BIT(3)
#define QCQ_F_RX_STATS BIT(4)
#define QCQ_F_NOTIFYQ BIT(5)

// Q related definitions
#define IONIC_QUEUE_NAME_MAX_SZ (32)
#define LIF_NAME_MAX_SZ (32)
#define DEFAULT_COS 0
#define DEFAULT_INTR_INDEX 0
#define NTXQ_DESC 16
#define NRXQ_DESC 16
#define NOTIFYQ_LENGTH	16

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef unsigned long uintptr_t;

struct ionic_device_bar
{
	void *vaddr;
	unsigned long long bus_addr;
	unsigned long len;
};

/** ionic_admin_ctx - Admin command context.
 * @cmd:        Admin command (64B) to be copied to the queue.
 * @comp:        Admin completion (16B) copied from the queue.
 *
 * @side_data:        Additional data to be copied to the doorbell page,
 *              if the command is issued as a dev cmd.
 * @side_data_len:    Length of additional data to be copied.
 */

#define IONIC_IPXE_BARS_MAX   2

struct ionic_admin_ctx
{
	union ionic_adminq_cmd cmd;
	union ionic_adminq_comp comp;
};

struct cq_info
{
	void *cq_desc;
	struct cq_info *next;
	unsigned int index;
	bool last;
};

struct queue;
struct desc_info;

typedef void (*desc_cb)(struct queue *q, struct desc_info *desc_info,
						struct cq_info *cq_info, void *cb_arg);

struct desc_info
{
	void *desc;
	void *sg_desc;
	struct desc_info *next;
	unsigned int index;
	unsigned int left;
	desc_cb cb;
	void *cb_arg;
};

struct cq
{
	void *base;
	dma_addr_t base_pa;
	struct lif *lif;
	struct cq_info *info;
	struct cq_info *tail;
	struct queue *bound_q;
	struct intr *bound_intr;
	unsigned int num_descs;
	unsigned int desc_size;
	bool done_color;
};

struct queue
{
	char name[IONIC_QUEUE_NAME_MAX_SZ];
	struct ionic_dev *idev;
	struct lif *lif;
	unsigned int type;
	unsigned int index;
	unsigned int hw_type;
	unsigned int hw_index;
	void *base;
	void *sg_base;
	dma_addr_t base_pa;
	dma_addr_t sg_base_pa;
	struct desc_info *info;
	struct desc_info *tail;
	struct desc_info *head;
	unsigned int num_descs;
	unsigned int desc_size;
	unsigned int sg_desc_size;
	struct ionic_doorbell __iomem *db;
	void *nop_desc;
	unsigned int pid;
};

struct qcq
{
	void *base;
	dma_addr_t base_pa;
	unsigned int total_size;
	struct queue q;
	struct cq cq;
	unsigned int flags;
};

struct lif
{
	char name[LIF_NAME_MAX_SZ];
	struct ionic *ionic;
	unsigned int index;
	struct qcq *adminqcq;
	struct qcq *notifyqcqs;
	struct qcq *txqcqs;
	struct qcq *rxqcqs;
	struct io_buffer *rx_iobuf[NRXQ_DESC];
	struct io_buffer *tx_iobuf[NTXQ_DESC];

	u32 info_sz;
	struct ionic_lif_info *info;
	dma_addr_t info_pa;
};

struct ionic_dev
{
	union ionic_dev_info_regs __iomem *dev_info;
	union ionic_dev_cmd_regs __iomem *dev_cmd;

	struct ionic_doorbell __iomem *db_pages;
	dma_addr_t phy_db_pages;

	struct ionic_intr_ctrl __iomem *intr_ctrl;
	struct ionic_intr_status __iomem *intr_status;

	unsigned long *hbm_inuse;
	dma_addr_t phy_hbm_pages;
	u32 hbm_npages;

	u32 port_info_sz;
	struct ionic_port_info *port_info;
	dma_addr_t port_info_pa;
};

/** An ionic network card */
struct ionic
{
	struct pci_device *pdev;
	struct platform_device *pfdev;
	struct device *dev;
	struct ionic_dev idev;
	struct ionic_device_bar bars[IONIC_IPXE_BARS_MAX];
	unsigned int num_bars;
	struct ionic_identity ident;
	struct lif *lif;
	u16 link_status;
};

/**
 * Function definitions
**/
//Probe Helper functions
int ionic_setup(struct ionic *ionic);
int ionic_dev_setup(struct ionic_dev *idev, struct ionic_device_bar bars[],
					unsigned int num_bars);
int ionic_identify(struct ionic *ionic);
int ionic_lif_alloc(struct ionic *ionic, unsigned int index);
int ionic_lif_init(struct net_device *netdev);
void ionic_qcq_dealloc(struct qcq *qcq);

//Netops helper functions
int ionic_qcq_enable(struct qcq *qcq);
int ionic_qcq_disable(struct qcq *qcq);
int ionic_lif_rx_mode(struct lif *lif, unsigned int rx_mode);

int ionic_lif_quiesce(struct lif *lif);
void ionic_rx_flush(struct lif *lif);
void ionic_tx_flush(struct net_device *netdev, struct lif *lif);
void ionic_rx_fill(struct net_device *netdev, int length);
void ionic_poll_rx(struct net_device *netdev);
void ionic_poll_tx(struct net_device *netdev);
bool ionic_q_has_space(struct queue *q, unsigned int want);

//helper functions from ionic_main
int ionic_dev_cmd_wait_check(struct ionic_dev *idev,
			     unsigned long max_seconds);
int ionic_dev_cmd_lif_init(struct ionic_dev *idev, u32 index, dma_addr_t addr,
	unsigned long max_seconds);
char *ionic_dev_asic_name(u8 asic_type);
int ionic_dev_cmd_go(struct ionic_dev *idev, union ionic_dev_cmd *cmd,
		     unsigned long max_seconds);
int ionic_dev_cmd_init(struct ionic_dev *idev, unsigned long max_seconds);
int ionic_dev_cmd_reset(struct ionic_dev *idev, unsigned long max_seconds);
u8 ionic_dev_cmd_status(struct ionic_dev *idev);
bool ionic_dev_cmd_done(struct ionic_dev *idev);
void ionic_dev_cmd_comp(struct ionic_dev *idev, void *mem);
int ionic_dev_cmd_adminq_init(struct ionic_dev *idev, struct qcq *qcq,
			      unsigned int lif_index,
			      unsigned long max_seconds);
unsigned int ionic_q_space_avail(struct queue *q);
int ionic_dev_cmd_lif_reset(struct ionic_dev *idev, u32 index,
			    unsigned long max_seconds);
int ionic_lif_reset(struct ionic *ionic);

#endif /* _IONIC_H */
