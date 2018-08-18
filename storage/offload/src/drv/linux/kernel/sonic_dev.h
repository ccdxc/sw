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

#ifndef _SONIC_DEV_H_
#define _SONIC_DEV_H_

#define static_assert(...)
#include <linux/spinlock.h>
#include "accel_ring.h"
#include "storage_seq_common.h"
#include "accel_dev_if.h"

#pragma pack(push, 1)

union dev_cmd {
	u32 words[16];
	struct admin_cmd cmd;
	struct reset_cmd reset;
	struct nop_cmd nop;
	struct identify_cmd identify;
	struct lif_init_cmd lif_init;
	struct adminq_init_cmd adminq_init;
};

union dev_cmd_cpl {
	u32 words[4];
	u8 status;
	struct admin_cpl comp;
	struct reset_cpl reset;
	struct nop_cpl nop;
	struct identify_cpl identify;
	struct lif_init_cpl lif_init;
	struct adminq_init_cpl adminq_init;
};

struct dev_cmd_regs {
	u32 signature;
	u32 done;
	union dev_cmd cmd;
	union dev_cmd_cpl comp;
};

struct dev_cmd_db {
	u32 v;
};

#define SONIC_BARS_MAX		6

struct sonic_dev_bar {
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
};

#define INTR_CTRL_REGS_MAX	64
#define INTR_CTRL_COAL_MAX	0x3F

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
struct intr_ctrl {
	u32 coalescing_init:6;
	u32 rsvd:26;
	u32 mask:1;
	u32 rsvd2:31;
	u32 int_credits:16;
	u32 unmask:1;
	u32 coal_timer_reset:1;
	u32 rsvd3:14;
	u32 mask_on_assert:1;
	u32 rsvd4:31;
	u32 coalescing_curr:6;
	u32 rsvd5:26;
	u32 rsvd6[3];
};

#define intr_to_coal(intr_ctrl)			(void *)((u8 *)(intr_ctrl) + 0)
#define intr_to_mask(intr_ctrl)			(void *)((u8 *)(intr_ctrl) + 4)
#define intr_to_credits(intr_ctrl)		(void *)((u8 *)(intr_ctrl) + 8)
#define intr_to_mask_on_assert(intr_ctrl)	(void *)((u8 *)(intr_ctrl) + 12)

struct intr_status {
	u32 status[2];
};

#pragma pack(pop)

static inline void sonic_struct_size_checks(void)
{
	BUILD_BUG_ON(sizeof(struct doorbell) != 8);
	BUILD_BUG_ON(sizeof(struct intr_ctrl) != 32);
	BUILD_BUG_ON(sizeof(struct intr_status) != 8);
	BUILD_BUG_ON(sizeof(struct admin_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct admin_cpl) != 16);
	BUILD_BUG_ON(sizeof(struct nop_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct nop_cpl) != 16);
	BUILD_BUG_ON(sizeof(struct reset_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct reset_cpl) != 16);
	BUILD_BUG_ON(sizeof(struct hang_notify_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct hang_notify_cpl) != 16);
	BUILD_BUG_ON(sizeof(struct identify_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct identify_cpl) != 16);
	BUILD_BUG_ON(sizeof(union identity) != 4096);
	BUILD_BUG_ON(sizeof(struct lif_init_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct lif_init_cpl) != 16);
	BUILD_BUG_ON(sizeof(struct adminq_init_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct adminq_init_cpl) != 16);
	BUILD_BUG_ON(sizeof(union adminq_cmd) != 64);
	BUILD_BUG_ON(sizeof(union adminq_cpl) != 16);
}

struct sonic_dev {
	struct dev_cmd_regs __iomem *dev_cmd;
	struct dev_cmd_db __iomem *dev_cmd_db;
	struct doorbell __iomem *db_pages;
	dma_addr_t phy_db_pages;
	struct intr_ctrl __iomem *intr_ctrl;
	struct intr_status __iomem *intr_status;
	spinlock_t hbm_inuse_lock; /* for hbm_inuse */
	unsigned long *hbm_inuse;
	dma_addr_t phy_hbm_pages;
	uint32_t hbm_npages;
#ifdef HAPS
	union identity __iomem *ident;
#endif
};

struct cq_info {
	void *cq_desc;
	struct cq_info *next;
	unsigned int index;
	bool last;
};

struct queue;
struct desc_info;

typedef void (*desc_cb)(struct queue *q, struct desc_info *desc_info,
			struct cq_info *cq_info, void *cb_arg);

struct desc_info {
	void *desc;
	struct desc_info *next;
	unsigned int index;
	unsigned int left;
	desc_cb cb;
	void *cb_arg;
};

#define QUEUE_NAME_MAX_SZ		(32)

struct queue {
	char name[QUEUE_NAME_MAX_SZ];
	struct sonic_dev *idev;
	struct lif *lif;
	struct per_core_resource *pc_res;
	unsigned int index;
	void *base;
	dma_addr_t base_pa;
	struct desc_info *info;
	struct desc_info *tail;
	struct desc_info *head;
	unsigned int num_descs;
	unsigned int desc_size;
	struct doorbell __iomem *db;
	void *nop_desc;
	unsigned int pid;
	unsigned int qid;
	unsigned int qtype;
	storage_seq_qgroup_t qgroup;
};

#if 0
struct seq_queue {
	char name[QUEUE_NAME_MAX_SZ];
	struct sonic_dev *idev;
	struct per_core_resource *pc_res;
	unsigned int index;
	int free_count;
	void *base;
	dma_addr_t base_pa;
	unsigned int num_descs;
	unsigned int desc_size;
	struct doorbell __iomem *db;
	void *nop_desc;
	unsigned int pid;
	unsigned int qid;
	unsigned int qtype;
	storage_seq_qgroup_t qgroup;
};
#endif

#define MAX_PER_QUEUE_STATUS_ENTRIES 2
#define MAX_PER_CORE_CPDC_SEQ_STATUS_QUEUES 32
#define MAX_PER_CORE_CRYPTO_SEQ_STATUS_QUEUES 32
#define MAX_NUM_CORES 64

struct per_core_resource {
	bool initialized;
	struct lif *lif;
	struct queue cp_seq_q;
	struct queue dc_seq_q;
	unsigned int num_cpdc_status_qs;
	DECLARE_BITMAP(cpdc_seq_status_qs_bmp, MAX_PER_CORE_CPDC_SEQ_STATUS_QUEUES);
	struct queue cpdc_seq_status_qs[MAX_PER_CORE_CPDC_SEQ_STATUS_QUEUES];
	struct queue crypto_enc_seq_q;
	struct queue crypto_dec_seq_q;
	unsigned int num_crypto_status_qs;
	DECLARE_BITMAP(crypto_seq_status_qs_bmp, MAX_PER_CORE_CRYPTO_SEQ_STATUS_QUEUES);
	struct queue crypto_seq_status_qs[MAX_PER_CORE_CRYPTO_SEQ_STATUS_QUEUES];
	// TODO - Add any mpool handles here
};

struct res {
	struct per_core_resource *pc_res[MAX_NUM_CORES];
	int core_to_res_map[MAX_NUM_CORES];
	spinlock_t lock;
	DECLARE_BITMAP(pc_res_bmp, MAX_NUM_CORES);
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

int sonic_dev_setup(struct sonic_dev *idev, struct sonic_dev_bar bars[],
		    unsigned int num_bars);

union dev_cmd; //Need to remove it
void sonic_dev_cmd_go(struct sonic_dev *idev, union dev_cmd *cmd);
u8 sonic_dev_cmd_status(struct sonic_dev *idev);
bool sonic_dev_cmd_done(struct sonic_dev *idev);
void sonic_dev_cmd_comp(struct sonic_dev *idev, void *mem);
void sonic_dev_cmd_reset(struct sonic_dev *idev);
void sonic_dev_cmd_hang_notify(struct sonic_dev *idev);
void sonic_dev_cmd_identify(struct sonic_dev *idev, u16 ver, dma_addr_t addr);
void sonic_dev_cmd_lif_init(struct sonic_dev *idev, u32 index);
void sonic_dev_cmd_adminq_init(struct sonic_dev *idev, struct queue *adminq,
			       unsigned int index, unsigned int lif_index,
			       unsigned int intr_index);

char *sonic_dev_asic_name(u8 asic_type);
struct doorbell __iomem *sonic_db_map(struct sonic_dev *idev, struct queue *q);

int sonic_intr_init(struct sonic_dev *idev, struct intr *intr,
		    unsigned long index);
void sonic_intr_mask_on_assertion(struct intr *intr);
void sonic_intr_return_credits(struct intr *intr, unsigned int credits,
			       bool unmask, bool reset_timer);
void sonic_intr_mask(struct intr *intr, bool mask);
void sonic_intr_coal_set(struct intr *intr, u32 coal_usecs);
int sonic_cq_init(struct lif *lif, struct cq *cq, struct intr *intr,
		  unsigned int num_descs, size_t desc_size);
void sonic_cq_map(struct cq *cq, void *base, dma_addr_t base_pa);
void sonic_cq_bind(struct cq *cq, struct queue *q);
typedef bool (*sonic_cq_cb)(struct cq *cq, struct cq_info *cq_info,
			    void *cb_arg);
unsigned int sonic_cq_service(struct cq *cq, unsigned int work_to_do,
			      sonic_cq_cb cb, void *cb_arg);

int sonic_q_alloc(struct lif *lif, struct queue *q,
		  unsigned int num_descs, unsigned int desc_size,
		  bool do_alloc_descs);
void sonic_q_free(struct lif *lif, struct queue *q);
int sonic_q_init(struct lif *lif, struct sonic_dev *idev, struct queue *q,
		 unsigned int index, const char *base, unsigned int num_descs,
		 size_t desc_size, unsigned int pid);
void sonic_q_map(struct queue *q, void *base, dma_addr_t base_pa);
void sonic_q_post(struct queue *q, bool ring_doorbell, desc_cb cb,
		  void *cb_arg);
void sonic_q_rewind(struct queue *q, struct desc_info *start);
unsigned int sonic_q_space_avail(struct queue *q);
bool sonic_q_has_space(struct queue *q, unsigned int want);
void sonic_q_service(struct queue *q, struct cq_info *cq_info,
		     unsigned int stop_index);
void* sonic_q_consume_entry(struct queue *q, uint32_t *index);
void sonic_q_ringdb(struct queue *q, uint32_t index);

#endif /* _SONIC_DEV_H_ */
