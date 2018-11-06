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
#include "ionic_if.h"

#undef ERRFILE
#define ERRFILE ERRFILE_Ionic

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#define __iomem
#define is_power_of_2(x) ((x) != 0 && (((x) & ((x)-1)) == 0))

#define ALIGN(x, a) __ALIGN_MASK(x, (typeof(x))(a)-1)
#define __ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))

#define DRV_VERSION "1.0"
#define IPXE_VERSION_CODE "1.0"

#define IONIC_MAX_MTU 2304

// Queue alignment
#define IDENTITY_ALIGN 4096

// BAR0 resources
#define IONIC_BARS_MAX 2
#define BAR0_SIZE 0x8000

#define BAR0_DEV_CMD_REGS_OFFSET 0x0000
#define BAR0_DEV_CMD_DB_OFFSET 0x1000
#define BAR0_INTR_STATUS_OFFSET 0x2000
#define BAR0_INTR_CTRL_OFFSET 0x3000

// Dev Command related defines
#define DEV_CMD_SIGNATURE 0x44455643 /* 'DEVC' */
#define devcmd_timeout 30
#define DEV_CMD_DONE 0x00000001

#define ASIC_TYPE_CAPRI 0

// Q flags
#define QCQ_F_INITED BIT(0)
#define QCQ_F_SG BIT(1)
#define QCQ_F_INTR BIT(2)
#define QCQ_F_TX_STATS BIT(3)
#define QCQ_F_RX_STATS BIT(4)

// Q related definitions
#define QUEUE_NAME_MAX_SZ (32)
#define LIF_NAME_MAX_SZ (32)
#define DEFAULT_COS 0
#define DEFAULT_INTR_INDEX 0
#define NRXQ_DESC 1024
#define RX_RING_DOORBELL_STRIDE ((1 << 3) - 1)

#define INTR_CTRL_REGS_MAX 64
#define INTR_CTRL_COAL_MAX 0x3F

#define intr_to_coal(intr_ctrl) (void *)((u8 *)(intr_ctrl) + 0)
#define intr_to_mask(intr_ctrl) (void *)((u8 *)(intr_ctrl) + 4)
#define intr_to_credits(intr_ctrl) (void *)((u8 *)(intr_ctrl) + 8)
#define intr_to_mask_on_assert(intr_ctrl) (void *)((u8 *)(intr_ctrl) + 12)

//define data types
typedef enum
{
	false = 0,
	true = 1
} bool;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef unsigned long uintptr_t;

#pragma pack(push, 1)

union dev_cmd {
	u32 words[16];
	struct admin_cmd cmd;
	struct nop_cmd nop;
	struct reset_cmd reset;
	struct hang_notify_cmd hang_notify;
	struct identify_cmd identify;
	struct lif_init_cmd lif_init;
	struct adminq_init_cmd adminq_init;
	struct station_mac_addr_get_cmd station_mac_addr_get;
	struct txq_init_cmd txq_init;
	struct rxq_init_cmd rxq_init;
	struct q_enable_cmd q_enable;
	struct q_disable_cmd q_disable;
	struct rx_mode_set_cmd rx_mode_set;
	struct rx_filter_add_cmd rx_filter_add;
};

union dev_cmd_comp {
	u32 words[4];
	u8 status;
	struct admin_comp comp;
	struct nop_comp nop;
	struct reset_comp reset;
	struct hang_notify_comp hang_notify;
	struct identify_comp identify;
	struct lif_init_comp lif_init;
	struct adminq_init_comp adminq_init;
};

struct dev_cmd_regs
{
	u32 signature;
	u32 done;
	union dev_cmd cmd;
	union dev_cmd_comp comp;
};

struct dev_cmd_db
{
	u32 v;
};

struct ionic_device_bar
{
	void *virtaddr;
	unsigned long long bus_addr;
	unsigned long len;
};

/**
 * struct doorbell - Doorbell register layout
 * @p_index: Producer index
 * @ring:		Selects the specific ring of the queue to update.
 *					 Type-specific meaning:
 *							ring=0: Default producer/consumer queue.
 *							ring=1: (CQ, EQ) Re-Arm queue.	RDMA CQs
 *							send events to EQs when armed.	EQs send
 *							interrupts when armed.
 * @qid:		 The queue id selects the queue destination for the
 *					 producer index and flags.
 */
struct doorbell
{
	u16 p_index;
	u8 ring : 3;
	u8 rsvd : 5;
	u8 qid_lo;
	u16 qid_hi;
	u16 rsvd2;
};

/**
 * struct intr_ctrl - Interrupt control register
 * @coalescing_init:  Coalescing timer initial value, in
 *                    device units.  Use @identity->intr_coal_mult
 *                    and @identity->intr_coal_div to convert from
 *                    usecs to device units:
 *
 *                      coal_init = coal_usecs * coal_mutl / coal_div
 *
 *                    When an interrupt is sent the interrupt
 *                    coalescing timer current value
 *                    (@coalescing_curr) is initialized with this
 *                    value and begins counting down.  No more
 *                    interrupts are sent until the coalescing
 *                    timer reaches 0.  When @coalescing_init=0
 *                    interrupt coalescing is effectively disabled
 *                    and every interrupt assert results in an
 *                    interrupt.  Reset value: 0.
 * @mask:             Interrupt mask.  When @mask=1 the interrupt
 *                    resource will not send an interrupt.  When
 *                    @mask=0 the interrupt resource will send an
 *                    interrupt if an interrupt event is pending
 *                    or on the next interrupt assertion event.
 *                    Reset value: 1.
 * @int_credits:      Interrupt credits.  This register indicates
 *                    how many interrupt events the hardware has
 *                    sent.  When written by software this
 *                    register atomically decrements @int_credits
 *                    by the value written.  When @int_credits
 *                    becomes 0 then the "pending interrupt" bit
 *                    in the Interrupt Status register is cleared
 *                    by the hardware and any pending but unsent
 *                    interrupts are cleared.
 *                    The upper 2 bits are special flags:
 *                       Bits 0-15: Interrupt Events -- Interrupt
 *                       event count.
 *                       Bit 16: @unmask -- When this bit is
 *                       written with a 1 the interrupt resource
 *                       will set mask=0.
 *                       Bit 17: @coal_timer_reset -- When this
 *                       bit is written with a 1 the
 *                       @coalescing_curr will be reloaded with
 *                       @coalescing_init to reset the coalescing
 *                       timer.
 * @mask_on_assert:   Automatically mask on assertion.  When
 *                    @mask_on_assert=1 the interrupt resource
 *                    will set @mask=1 whenever an interrupt is
 *                    sent.  When using interrupts in Legacy
 *                    Interrupt mode the driver must select
 *                    @mask_on_assert=0 for proper interrupt
 *                    operation.
 * @coalescing_curr:  Coalescing timer current value, in
 *                    microseconds.  When this value reaches 0
 *                    the interrupt resource is again eligible to
 *                    send an interrupt.  If an interrupt event
 *                    is already pending when @coalescing_curr
 *                    reaches 0 the pending interrupt will be
 *                    sent, otherwise an interrupt will be sent
 *                    on the next interrupt assertion event.
 */
struct intr_ctrl
{
	u32 coalescing_init : 6;
	u32 rsvd : 26;
	u32 mask : 1;
	u32 rsvd2 : 31;
	u32 int_credits : 16;
	u32 unmask : 1;
	u32 coal_timer_reset : 1;
	u32 rsvd3 : 14;
	u32 mask_on_assert : 1;
	u32 rsvd4 : 31;
	u32 coalescing_curr : 6;
	u32 rsvd5 : 26;
	u32 rsvd6[3];
};

struct intr_status
{
	u32 status[2];
};

/** ionic_admin_ctx - Admin command context.
 * @cmd:        Admin command (64B) to be copied to the queue.
 * @comp:        Admin completion (16B) copied from the queue.
 *
 * @side_data:        Additional data to be copied to the doorbell page,
 *              if the command is issued as a dev cmd.
 * @side_data_len:    Length of additional data to be copied.
 *
 * TODO:
 * The side_data and side_data_len are temporary and will be removed.  For now,
 * they are used when admin commands referring to side-band data are posted as
 * dev commands instead.  Only single-indirect side-band data is supported.
 * Only 2K of data is supported, because first half of page is for registers.
 */
struct ionic_admin_ctx
{
	union adminq_cmd cmd;
	union adminq_comp comp;
};

#pragma pack(pop)

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
	char name[QUEUE_NAME_MAX_SZ];
	struct ionic_dev *idev;
	struct lif *lif;
	unsigned int index;
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
	struct doorbell __iomem *db;
	void *nop_desc;
	unsigned int pid;
	unsigned int qid;
	unsigned int qtype;
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
	struct qcq *txqcqs;
	struct qcq *rxqcqs;
	struct io_buffer *rx_iobuf[NRXQ_DESC];
};

struct ionic_dev
{
	struct dev_cmd_regs __iomem *dev_cmd;
	struct dev_cmd_db __iomem *dev_cmd_db;
	struct doorbell __iomem *db_pages;
	dma_addr_t phy_db_pages;
	struct intr_ctrl __iomem *intr_ctrl;
	struct intr_status __iomem *intr_status;
	unsigned long *hbm_inuse;
	dma_addr_t phy_hbm_pages;
	u32 hbm_npages;
};

/** An ionic network card */
struct ionic
{
	struct pci_dev *pdev;
	struct platform_device *pfdev;
	struct device *dev;
	struct ionic_dev idev;
	struct ionic_device_bar bars[IONIC_BARS_MAX];
	unsigned int num_bars;
	union identity *ident;
	dma_addr_t ident_pa;
	struct lif *ionic_lif;
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

void ionic_rx_flush(struct lif *lif);
void ionic_rx_fill(struct net_device *netdev, int length);
void ionic_poll_rx(struct net_device *netdev);
void ionic_poll_tx(struct net_device *netdev);
bool ionic_q_has_space(struct queue *q, unsigned int want);

//helper functions from ionic_main
int ionic_dev_cmd_wait_check(struct ionic_dev *idev, unsigned long max_wait);
void ionic_dev_cmd_lif_init(struct ionic_dev *idev, u32 index);
char *ionic_dev_asic_name(u8 asic_type);
void ionic_dev_cmd_go(struct ionic_dev *idev, union dev_cmd *cmd);
void ionic_dev_cmd_reset(struct ionic_dev *idev);
u8 ionic_dev_cmd_status(struct ionic_dev *idev);
bool ionic_dev_cmd_done(struct ionic_dev *idev);
void ionic_dev_cmd_comp(struct ionic_dev *idev, void *mem);
void ionic_dev_cmd_adminq_init(struct ionic_dev *idev, struct queue *adminq,
							   unsigned int lif_index);
void ionic_dev_cmd_station_get(struct ionic_dev *idev);
void ionic_dev_cmd_rxq_init(struct ionic_dev *idev, struct queue *rxq);
void ionic_enable_adminq_post(struct lif *lif, struct ionic_admin_ctx *ctx);
void ionic_disable_adminq_post(struct lif *lif, struct ionic_admin_ctx *ctx);
unsigned int ionic_q_space_avail(struct queue *q);
#endif /* _IONIC_H */
