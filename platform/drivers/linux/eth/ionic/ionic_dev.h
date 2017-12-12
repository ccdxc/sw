/*
 * Copyright 2017 Pensando Systems, Inc.  All rights reserved.
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

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/pci.h>

#include "ionic_if.h"

#ifndef _IONIC_DEV_H_
#define _IONIC_DEV_H_

struct dev_cmd_regs;
struct dev_cmd_db;
struct queue;

#define IONIC_MIN_MTU		ETH_MIN_MTU
#define IONIC_MAX_MTU		9000

#define IONIC_BARS_MAX		6

struct ionic_dev_bar {
	void __iomem *vaddr;
	dma_addr_t bus_addr;
	unsigned long len;
};

/**
 * struct doorbell - Doorbell register layout
 * @p_index: Producer index
 * @ring:    Selects the specific ring of the queue to update.
 *           Type-specific meaning:
 *              ring=0: Default producer/consumer queue.
 *              ring=1: (CQ, EQ) Re-Arm queue.  RDMA CQs
 *              send events to EQs when armed.  EQs send
 *              interrupts when armed.
 * @qid:     The queue id selects the queue destination for the
 *           producer index and flags.
 */
struct doorbell {
	u16 p_index;
	u8 ring:3;
	u8 rsvd:5;
	u8 qid_lo;
	u16 qid_hi;
	u16 rsvd2;
} __packed;

#define INTR_CTRL_REGS_MAX	64

/**
 * struct intr_ctrl - Interrupt control register
 * @coal_init:        Coalescing timer initial value, in
 *                    microseconds.  When an interrupt is sent
 *                    the interrupt coalescing timer current
 *                    value (coalescing_curr) is initialized with
 *                    this value and begins counting down.  No
 *                    more interrupts are sent until the
 *                    coalescing timer reaches 0.  When
 *                    coalescing_init=0 interrupt coalescing is
 *                    effectively disabled and every interrupt
 *                    assert results in an interrupt.  Reset
 *                    value: 0.
 * @mask:             Interrupt mask.  When mask=1 the interrupt
 *                    resource will not send an interrupt.  When
 *                    mask=0 the interrupt resource will send an
 *                    interrupt if an interrupt event is pending
 *                    or on the next interrupt assertion event.
 *                    Reset value: 1.
 * @credits:          Interrupt mask.  When mask=1 the interrupt
 *                    resource will not send an interrupt.  When
 *                    mask=0 the interrupt resource will send an
 *                    interrupt if an interrupt event is pending
 *                    or on the next interrupt assertion event.
 * @unmask:           When this bit is written with a 1 the
 *                    interrupt resource will set mask=0.
 * @coal_timer_reset: When this bit is written with a 1 the
 *                    coalescing_curr will be reloaded with
 *                    coalescing_init to reset the coalescing
 *                    timer.
 * @mask_on_assert:   Automatically mask on assertion.  When
 *                    mask_on_assert=1 the interrupt resource
 *                    will set mask=1 whenever an interrupt is
 *                    sent.  When using interrupts in Legacy
 *                    Interrupt mode the driver must select
 *                    mask_on_assert=0 for proper interrupt
 *                    operation.
 * @coal_timer:       Coalescing timer current value, in
 *                    microseconds.  When this value reaches 0
 *                    the interrupt resource is again eligible to
 *                    send an interrupt.  If an interrupt event
 *                    is already pending when coalescing_curr
 *                    reaches 0 the pending interrupt will be
 *                    sent, otherwise an interrupt will be sent
 *                    on the next interrupt assertion event.
 */
struct intr_ctrl {
	u32 coal_init:6;
	u32 rsvd:26;
	u32 mask:1;
	u32 rsvd2:31;
	u32 credits:16;
	u32 unmask:1;
	u32 coal_timer_reset:1;
	u32 rsvd3:14;
	u32 mask_on_assert:1;
	u32 rsvd4:31;
	u32 coal_timer:6;
	u32 rsvd5:26;
	u32 rsvd6[11];
} __packed;

#define intr_to_mask(intr_ctrl)			((void *)(intr_ctrl) + 4)
#define intr_to_credits(intr_ctrl)		((void *)(intr_ctrl) + 8)
#define intr_to_mask_on_assert(intr_ctrl)	((void *)(intr_ctrl) + 12)

struct intr_status {
	u32 status[2];
} __packed;

static inline void ionic_struct_size_checks(void) {
	BUILD_BUG_ON(sizeof(struct doorbell) != 8);
	BUILD_BUG_ON(sizeof(struct intr_ctrl) != 64);
	BUILD_BUG_ON(sizeof(struct intr_status) != 8);
	BUILD_BUG_ON(sizeof(struct admin_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct admin_comp) != 16);
	BUILD_BUG_ON(sizeof(struct nop_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct nop_comp) != 16);
	BUILD_BUG_ON(sizeof(struct reset_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct reset_comp) != 16);
	BUILD_BUG_ON(sizeof(struct identify_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct identify_comp) != 16);
	BUILD_BUG_ON(sizeof(union identity) != 4096);
	BUILD_BUG_ON(sizeof(struct lif_init_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct lif_init_comp) != 16);
	BUILD_BUG_ON(sizeof(struct adminq_init_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct adminq_init_comp) != 16);
	BUILD_BUG_ON(sizeof(struct txq_init_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct txq_init_comp) != 16);
	BUILD_BUG_ON(sizeof(struct rxq_init_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct rxq_init_comp) != 16);
	BUILD_BUG_ON(sizeof(struct txq_desc) != 16);
	BUILD_BUG_ON(sizeof(struct txq_sg_desc) != 256);
	BUILD_BUG_ON(sizeof(struct txq_comp) != 16);
	BUILD_BUG_ON(sizeof(struct rxq_desc) != 16);
	BUILD_BUG_ON(sizeof(struct rxq_comp) != 16);
	BUILD_BUG_ON(sizeof(struct q_enable_cmd) != 64);
	BUILD_BUG_ON(sizeof(q_enable_comp) != 16);
	BUILD_BUG_ON(sizeof(struct q_disable_cmd) != 64);
	BUILD_BUG_ON(sizeof(q_disable_comp) != 16);
	BUILD_BUG_ON(sizeof(struct station_mac_addr_get_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct station_mac_addr_get_comp) != 16);
	BUILD_BUG_ON(sizeof(struct rx_mode_set_cmd) != 64);
	BUILD_BUG_ON(sizeof(rx_mode_set_comp) != 16);
	BUILD_BUG_ON(sizeof(struct mtu_set_cmd) != 64);
	BUILD_BUG_ON(sizeof(mtu_set_comp) != 16);
	BUILD_BUG_ON(sizeof(struct rx_filter_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct rx_filter_comp) != 16);
	BUILD_BUG_ON(sizeof(struct debug_q_dump_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct debug_q_dump_comp) != 16);
}

struct ionic_dev {
	struct dev_cmd_regs __iomem *dev_cmd;
	struct dev_cmd_db __iomem *dev_cmd_db;
	struct doorbell __iomem *db_pages;
	struct intr_ctrl __iomem *intr_ctrl;
	struct intr_status __iomem *intr_status;
};

struct cq_info {
	void *cq_desc;
	struct cq_info *next;
	unsigned int index;
	bool last;
};

struct desc_info;
typedef void (*desc_cb)(struct queue *q, struct desc_info *desc_info,
			struct cq_info *cq_info, void *cb_arg);
struct desc_info {
	void *desc;
	void *sg_desc;
	struct desc_info *next;
	unsigned int index;
	unsigned int left;
	desc_cb cb;
	void *cb_arg;
};

#define QUEUE_NAME_MAX_SZ		(32)

struct queue {
	char name[QUEUE_NAME_MAX_SZ];
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

#define INTR_INDEX_NOT_ASSIGNED		(-1)
#define INTR_NAME_MAX_SZ		(32)

struct intr {
	char name[INTR_NAME_MAX_SZ];
	unsigned int index;
	unsigned int vector;
	struct intr_ctrl __iomem *ctrl;
};

struct cq {
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

int ionic_dev_setup(struct ionic_dev *idev, struct ionic_dev_bar bars[],
		    unsigned int num_bars);

u8 ionic_dev_cmd_status(struct ionic_dev *idev);
bool ionic_dev_cmd_done(struct ionic_dev *idev);
void ionic_dev_cmd_comp(struct ionic_dev *idev, void *mem);
void ionic_dev_cmd_reset(struct ionic_dev *idev);
void ionic_dev_cmd_identify(struct ionic_dev *idev, dma_addr_t addr);
void ionic_dev_cmd_lif_init(struct ionic_dev *idev, u32 index);
void ionic_dev_cmd_adminq_init(struct ionic_dev *idev, struct queue *adminq,
			       unsigned int index, unsigned int lif_index,
			       unsigned int intr_index);
#ifndef ADMINQ
void ionic_dev_cmd_txq_init(struct ionic_dev *idev, struct queue *txq,
			    struct cq *cq, unsigned int cos);
void ionic_dev_cmd_rxq_init(struct ionic_dev *idev, struct queue *rxq,
			    struct cq *cq);
void ionic_dev_cmd_q_enable(struct ionic_dev *idev, struct queue *q);
void ionic_dev_cmd_q_disable(struct ionic_dev *idev, struct queue *q);
void ionic_dev_cmd_station_get(struct ionic_dev *idev);
void ionic_dev_cmd_mtu_set(struct ionic_dev *idev, unsigned int mtu);
void ionic_dev_cmd_rx_mode_set(struct ionic_dev *idev, unsigned int rx_mode);
void ionic_dev_cmd_rx_filter_vlan(struct ionic_dev *idev, u16 vlan, bool add);
void ionic_dev_cmd_rx_filter_mac(struct ionic_dev *idev, const u8 *addr,
				 bool add);
void ionic_dev_cmd_rx_filter_macvlan(struct ionic_dev *idev, struct queue *q,
				     const u8 *addr, u16 vlan, bool add);
#endif

char *ionic_dev_asic_name(u8 asic_type);
struct doorbell __iomem *ionic_db_map(struct ionic_dev *idev, struct queue *q);

int ionic_intr_init(struct ionic_dev *idev, struct intr *intr,
		    unsigned long index);
void ionic_intr_mask_on_assertion(struct intr *intr);
void ionic_intr_return_credits(struct intr *intr, unsigned int credits,
			       bool unmask, bool reset_timer);
void ionic_intr_mask(struct intr *intr, bool mask);
int ionic_cq_init(struct lif *lif, struct cq *cq, struct intr *intr,
		  unsigned int num_descs, size_t desc_size);
void ionic_cq_map(struct cq *cq, void *base, dma_addr_t base_pa);
void ionic_cq_bind(struct cq *cq, struct queue *q);
typedef bool (*ionic_cq_cb)(struct cq *cq, struct cq_info *cq_info,
			    void *cb_arg);
unsigned int ionic_cq_service(struct cq *cq, unsigned int work_to_do,
			      ionic_cq_cb cb, void *cb_arg);

int ionic_q_init(struct lif *lif, struct queue *q, unsigned int index,
		 const char *base, unsigned int num_descs, size_t desc_size,
		 size_t sg_desc_size, unsigned int pid);
void ionic_q_map(struct queue *q, void *base, dma_addr_t base_pa);
void ionic_q_sg_map(struct queue *q, void *base, dma_addr_t base_pa);
void ionic_q_post(struct queue *q, bool ring_doorbell, desc_cb cb,
		  void *cb_arg);
void ionic_q_rewind(struct queue *q, struct desc_info *start);
unsigned int ionic_q_space_avail(struct queue *q);
bool ionic_q_has_space(struct queue *q, unsigned int want);
void ionic_q_service(struct queue *q, struct cq_info *cq_info,
		     unsigned int stop_index);

#endif /* _IONIC_DEV_H_ */
