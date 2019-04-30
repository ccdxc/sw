/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2017 - 2019 Pensando Systems, Inc */

#ifndef _IONIC_DEV_H_
#define _IONIC_DEV_H_

#include <linux/mutex.h>
#include <linux/workqueue.h>

#include "ionic_if.h"

#define IONIC_MIN_MTU			ETH_MIN_MTU
#define IONIC_MAX_MTU			9194
#define IONIC_MAX_TXRX_DESC		16384
#define IONIC_MIN_TXRX_DESC		16
#define IONIC_DEF_TXRX_DESC		4096
#define IONIC_LIFS_MAX			1024

struct ionic_dev_bar {
	void __iomem *vaddr;
	phys_addr_t bus_addr;
	unsigned long len;
	int res_index;
};

static inline void ionic_struct_size_checks(void)
{
	/* Registers */
	BUILD_BUG_ON(sizeof(struct doorbell) != 8);
	BUILD_BUG_ON(sizeof(struct intr_ctrl) != 32);
	BUILD_BUG_ON(sizeof(struct intr_status) != 8);

	BUILD_BUG_ON(sizeof(union dev_regs) != 4096);
	BUILD_BUG_ON(sizeof(union dev_info_regs) != 2048);
	BUILD_BUG_ON(sizeof(union dev_cmd_regs) != 2048);

	BUILD_BUG_ON(sizeof(struct port_stats) != 1024);
	BUILD_BUG_ON(sizeof(struct lif_stats) != 1024);

	BUILD_BUG_ON(sizeof(struct admin_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct admin_comp) != 16);
	BUILD_BUG_ON(sizeof(struct nop_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct nop_comp) != 16);

	/* Device commands */
	BUILD_BUG_ON(sizeof(struct dev_identify_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct dev_identify_comp) != 16);
	BUILD_BUG_ON(sizeof(struct dev_init_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct dev_init_comp) != 16);
	BUILD_BUG_ON(sizeof(struct dev_reset_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct dev_reset_comp) != 16);
	BUILD_BUG_ON(sizeof(struct dev_getattr_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct dev_getattr_comp) != 16);
	BUILD_BUG_ON(sizeof(struct dev_setattr_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct dev_setattr_comp) != 16);

	/* Port commands */
	BUILD_BUG_ON(sizeof(struct port_identify_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct port_identify_comp) != 16);
	BUILD_BUG_ON(sizeof(struct port_init_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct port_init_comp) != 16);
	BUILD_BUG_ON(sizeof(struct port_reset_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct port_reset_comp) != 16);
	BUILD_BUG_ON(sizeof(struct port_getattr_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct port_getattr_comp) != 16);
	BUILD_BUG_ON(sizeof(struct port_setattr_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct port_setattr_comp) != 16);

	/* LIF commands */
	BUILD_BUG_ON(sizeof(struct lif_init_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct lif_init_comp) != 16);
	BUILD_BUG_ON(sizeof(struct lif_reset_cmd) != 64);
	BUILD_BUG_ON(sizeof(lif_reset_comp) != 16);
	BUILD_BUG_ON(sizeof(struct lif_getattr_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct lif_getattr_comp) != 16);
	BUILD_BUG_ON(sizeof(struct lif_setattr_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct lif_setattr_comp) != 16);

	BUILD_BUG_ON(sizeof(struct q_init_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct q_init_comp) != 16);
	BUILD_BUG_ON(sizeof(struct q_control_cmd) != 64);
	BUILD_BUG_ON(sizeof(q_control_comp) != 16);

	BUILD_BUG_ON(sizeof(struct rx_mode_set_cmd) != 64);
	BUILD_BUG_ON(sizeof(rx_mode_set_comp) != 16);
	BUILD_BUG_ON(sizeof(struct rx_filter_add_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct rx_filter_add_comp) != 16);
	BUILD_BUG_ON(sizeof(struct rx_filter_del_cmd) != 64);
	BUILD_BUG_ON(sizeof(rx_filter_del_comp) != 16);

	/* RDMA commands */
	BUILD_BUG_ON(sizeof(struct rdma_reset_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct rdma_queue_cmd) != 64);

	/* Events */
	BUILD_BUG_ON(sizeof(struct notifyq_cmd) != 4);
	BUILD_BUG_ON(sizeof(union notifyq_comp) != 64);
	BUILD_BUG_ON(sizeof(struct notifyq_event) != 64);
	BUILD_BUG_ON(sizeof(struct link_change_event) != 64);
	BUILD_BUG_ON(sizeof(struct reset_event) != 64);
	BUILD_BUG_ON(sizeof(struct heartbeat_event) != 64);
	BUILD_BUG_ON(sizeof(struct log_event) != 64);

	/* I/O */
	BUILD_BUG_ON(sizeof(struct txq_desc) != 16);
	BUILD_BUG_ON(sizeof(struct txq_sg_desc) != 128);
	BUILD_BUG_ON(sizeof(struct txq_comp) != 16);

	BUILD_BUG_ON(sizeof(struct rxq_desc) != 16);
	BUILD_BUG_ON(sizeof(struct rxq_sg_desc) != 128);
	BUILD_BUG_ON(sizeof(struct rxq_comp) != 16);
}

struct ionic_dev {
	union dev_info_regs __iomem *dev_info;
	union dev_cmd_regs __iomem *dev_cmd;

	struct doorbell __iomem *db_pages;
	dma_addr_t phy_db_pages;

	struct intr_ctrl __iomem *intr_ctrl;
	struct intr_status __iomem *intr_status;
	u8 *msix_cfg_base;

	struct mutex cmb_inuse_lock; /* for cmb_inuse */
	unsigned long *cmb_inuse;
	dma_addr_t phy_cmb_pages;
	u32 cmb_npages;

	struct port_info *port_info;
	dma_addr_t port_info_pa;
	u32 port_info_sz;
};

struct cq_info {
	void *cq_desc;
	struct cq_info *next;
	unsigned int index;
	bool last;
};

struct queue;
struct qcq;
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
	struct ionic_dev *idev;
	struct lif *lif;
	unsigned int index;
	unsigned int type;
	unsigned int hw_index;
	unsigned int hw_type;
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
	u64 dbell_count;
	u64 drop;
	u64 stop;
	u64 wake;
};

#define INTR_INDEX_NOT_ASSIGNED		(-1)
#define INTR_NAME_MAX_SZ		(32)

struct intr {
	char name[INTR_NAME_MAX_SZ];
	unsigned int index;
	unsigned int vector;
	struct intr_ctrl __iomem *ctrl;
	u64 rearm_count;
	unsigned int cpu;
	cpumask_t affinity_mask;
};

struct cq {
	void *base;
	dma_addr_t base_pa;
	struct lif *lif;
	struct cq_info *info;
	struct cq_info *tail;
	struct queue *bound_q;
	struct intr *bound_intr;
	u64 compl_count;
	unsigned int num_descs;
	unsigned int desc_size;
	bool done_color;
};

struct ionic;

int ionic_dev_setup(struct ionic *ionic);
void ionic_dev_teardown(struct ionic *ionic);

void ionic_dev_cmd_go(struct ionic_dev *idev, union dev_cmd *cmd);
u8 ionic_dev_cmd_status(struct ionic_dev *idev);
bool ionic_dev_cmd_done(struct ionic_dev *idev);
void ionic_dev_cmd_comp(struct ionic_dev *idev, void *mem);

void ionic_dev_cmd_identify(struct ionic_dev *idev, u8 ver);
void ionic_dev_cmd_init(struct ionic_dev *idev);
void ionic_dev_cmd_reset(struct ionic_dev *idev);

void ionic_dev_cmd_port_identify(struct ionic_dev *idev);
void ionic_dev_cmd_port_init(struct ionic_dev *idev);
void ionic_dev_cmd_port_reset(struct ionic_dev *idev);
void ionic_dev_cmd_port_state(struct ionic_dev *idev, u8 state);
void ionic_dev_cmd_port_speed(struct ionic_dev *idev, u32 speed);
void ionic_dev_cmd_port_mtu(struct ionic_dev *idev, u32 mtu);
void ionic_dev_cmd_port_autoneg(struct ionic_dev *idev, u8 an_enable);
void ionic_dev_cmd_port_fec(struct ionic_dev *idev, u8 fec_type);
void ionic_dev_cmd_port_pause(struct ionic_dev *idev, u8 pause_type);
void ionic_dev_cmd_port_loopback(struct ionic_dev *idev, u8 loopback_mode);

void ionic_dev_cmd_lif_identify(struct ionic_dev *idev, u8 type, u8 ver);
void ionic_dev_cmd_lif_init(struct ionic_dev *idev, u16 lif_index,
	dma_addr_t addr);
void ionic_dev_cmd_lif_reset(struct ionic_dev *idev, u16 lif_index);
void ionic_dev_cmd_adminq_init(struct ionic_dev *idev, struct qcq *qcq,
	u16 lif_index, u16 intr_index);

char *ionic_dev_asic_name(u8 asic_type);
struct doorbell __iomem *ionic_db_map(struct lif *lif, struct queue *q);
int ionic_db_page_num(struct lif *lif, int pid);

void ionic_intr_clean(struct intr *intr);
void ionic_intr_init(struct ionic_dev *idev, struct intr *intr,
		     unsigned long index);
void ionic_intr_mask_on_assertion(struct intr *intr);
void ionic_intr_return_credits(struct intr *intr, unsigned int credits,
			       bool unmask, bool reset_timer);
void ionic_intr_mask(struct intr *intr, bool mask);
void ionic_intr_coal_set(struct intr *intr, u32 coal_usecs);
int ionic_cq_init(struct lif *lif, struct cq *cq, struct intr *intr,
		  unsigned int num_descs, size_t desc_size);
void ionic_cq_map(struct cq *cq, void *base, dma_addr_t base_pa);
void ionic_cq_bind(struct cq *cq, struct queue *q);
typedef bool (*ionic_cq_cb)(struct cq *cq, struct cq_info *cq_info);
typedef void (*ionic_cq_done_cb)(void *done_arg);
unsigned int ionic_cq_service(struct cq *cq, unsigned int work_to_do,
			      ionic_cq_cb cb, ionic_cq_done_cb done_cb,
			      void *done_arg);

int ionic_q_init(struct lif *lif, struct ionic_dev *idev, struct queue *q,
		 unsigned int index, const char *name, unsigned int num_descs,
		 size_t desc_size, size_t sg_desc_size, unsigned int pid);
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
