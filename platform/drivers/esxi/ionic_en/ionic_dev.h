/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 */

/*
 * ionic_dev.h --
 *
 * Definitions of ionic device
 */


#ifndef _IONIC_DEV_H_
#define _IONIC_DEV_H_

#include "ionic_types.h"
#include "ionic_if.h"

#define IONIC_MIN_MTU           1500 
#define IONIC_MAX_MTU           9000

struct ionic_dev_bar {
        void __iomem *vaddr;
        dma_addr_t bus_addr;
        unsigned long len;
        int res_index;
};

static inline void ionic_struct_size_checks(void)
{
	/* Registers */
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_doorbell) == 8);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_intr_ctrl) == 32);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_intr_status) == 8);

	VMK_ASSERT_ON_COMPILE(sizeof(union ionic_dev_regs) == 4096);
	VMK_ASSERT_ON_COMPILE(sizeof(union ionic_dev_info_regs) == 2048);
	VMK_ASSERT_ON_COMPILE(sizeof(union ionic_dev_cmd_regs) == 2048);

	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_lif_stats) == 1024);

	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_admin_cmd) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_admin_comp) == 16);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_nop_cmd) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_nop_comp) == 16);

	/* Device commands */
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_dev_identify_cmd) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_dev_identify_comp) == 16);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_dev_init_cmd) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_dev_init_comp) == 16);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_dev_reset_cmd) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_dev_reset_comp) == 16);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_dev_getattr_cmd) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_dev_getattr_comp) == 16);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_dev_setattr_cmd) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_dev_setattr_comp) == 16);

	/* Port commands */
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_port_identify_cmd) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_port_identify_comp) == 16);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_port_init_cmd) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_port_init_comp) == 16);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_port_reset_cmd) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_port_reset_comp) == 16);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_port_getattr_cmd) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_port_getattr_comp) == 16);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_port_setattr_cmd) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_port_setattr_comp) == 16);

	/* LIF commands */
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_lif_init_cmd) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_lif_init_comp) == 16);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_lif_reset_cmd) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(ionic_lif_reset_comp) == 16);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_lif_getattr_cmd) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_lif_getattr_comp) == 16);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_lif_setattr_cmd) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_lif_setattr_comp) == 16);

	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_q_init_cmd) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_q_init_comp) == 16);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_q_control_cmd) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(ionic_q_control_comp) == 16);

	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_rx_mode_set_cmd) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(ionic_rx_mode_set_comp) == 16);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_rx_filter_add_cmd) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_rx_filter_add_comp) == 16);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_rx_filter_del_cmd) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(ionic_rx_filter_del_comp) == 16);

	/* RDMA commands */
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_rdma_reset_cmd) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_rdma_queue_cmd) == 64);

	/* Events */
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_notifyq_cmd) == 4);
	VMK_ASSERT_ON_COMPILE(sizeof(union ionic_notifyq_comp) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_notifyq_event) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_link_change_event) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_reset_event) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_heartbeat_event) == 64);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_log_event) == 64);

	/* I/O */
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_txq_desc) == 16);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_txq_sg_desc) == 128);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_txq_comp) == 16);

	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_rxq_desc) == 16);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_rxq_sg_desc) == 128);
	VMK_ASSERT_ON_COMPILE(sizeof(struct ionic_rxq_comp) == 16);
}

/** struct ionic_devinfo - device information. */
struct ionic_devinfo {
        u8 asic_type;
        u8 asic_rev;
        char fw_version[IONIC_DEVINFO_FWVERS_BUFLEN + 1];
        char serial_num[IONIC_DEVINFO_SERIAL_BUFLEN + 1];
};

struct ionic_dev {
        union ionic_dev_info_regs __iomem *dev_info_regs;
        union ionic_dev_cmd_regs __iomem *dev_cmd_regs;

        struct ionic_doorbell __iomem *db_pages;
        dma_addr_t phy_db_pages;

        struct ionic_intr_ctrl __iomem *intr_ctrl;
        struct ionic_intr_status __iomem *intr_status;

        vmk_Mutex cmb_inuse_lock; /* for cmb_inuse */
        unsigned long *cmb_inuse;
        dma_addr_t phy_cmb_pages;
        uint32_t cmb_npages;

        struct ionic_port_info *port_info;
        dma_addr_t port_info_pa;
        u32 port_info_sz;

        u8 port_index;
        struct ionic_devinfo dev_info;
        vmk_uint64 last_hb_time;
        u32 last_hb;
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

#define IONIC_QUEUE_NAME_MAX_SZ               (32)

struct queue {
        char name[IONIC_QUEUE_NAME_MAX_SZ];
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
        struct ionic_doorbell __iomem *db;
        void *nop_desc;
        unsigned int pid;
};

#define IONIC_INTR_INDEX_NOT_ASSIGNED         (-1)
#define IONIC_INTR_NAME_MAX_SZ                (32)

struct intr {
        char name[IONIC_INTR_NAME_MAX_SZ];
        unsigned int index;
        vmk_IntrCookie cookie;
//        unsigned int vector;
        struct ionic_intr_ctrl __iomem *ctrl;
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

struct ionic;

VMK_ReturnStatus ionic_dev_setup(struct ionic *ionic);
void ionic_dev_clean(struct ionic *ionic);

VMK_ReturnStatus
ionic_heartbeat_check(struct ionic *ionic);

void ionic_dev_cmd_go(struct ionic_dev *idev, union ionic_dev_cmd *cmd);
u8 ionic_dev_cmd_status(struct ionic_dev *idev);
bool ionic_dev_cmd_done(struct ionic_dev *idev);
void ionic_dev_cmd_comp(struct ionic_dev *idev, void *mem);

void ionic_dev_cmd_identify(struct ionic_dev *idev, u16 ver);
void ionic_dev_cmd_init(struct ionic_dev *idev);
void ionic_dev_cmd_reset(struct ionic_dev *idev);

void ionic_dev_cmd_port_identify(struct ionic_dev *idev);
void ionic_dev_cmd_port_init(struct ionic_dev *idev);
void ionic_dev_cmd_port_reset(struct ionic_dev *idev);
void ionic_dev_cmd_port_state(struct ionic_dev *idev, uint8_t state);
void ionic_dev_cmd_port_speed(struct ionic_dev *idev, uint32_t speed);
void ionic_dev_cmd_port_mtu(struct ionic_dev *idev, uint32_t mtu);
void ionic_dev_cmd_port_autoneg(struct ionic_dev *idev, uint8_t an_enable);
void ionic_dev_cmd_port_fec(struct ionic_dev *idev, uint8_t fec_type);
void ionic_dev_cmd_port_pause(struct ionic_dev *idev, uint8_t pause_type);
void ionic_dev_cmd_port_loopback(struct ionic_dev *idev, uint8_t loopback_mode);

void ionic_dev_cmd_lif_identify(struct ionic_dev *idev, u8 type, u8 ver);
void ionic_dev_cmd_lif_init(struct ionic_dev *idev, u16 lif_index,
	dma_addr_t addr);
void ionic_dev_cmd_lif_reset(struct ionic_dev *idev, u16 lif_index);
void ionic_dev_cmd_adminq_init(struct ionic_dev *idev, struct qcq *qcq,
	u16 lif_index, u16 intr_index);

char *ionic_dev_asic_name(u8 asic_type);
struct ionic_doorbell __iomem *ionic_db_map(struct ionic_dev *idev, struct queue *q);
void ionic_intr_clean(struct intr *intr);
int ionic_intr_init(struct ionic_dev *idev, struct intr *intr,
                    unsigned long index);
void ionic_intr_mask_on_assertion(struct intr *intr);
void ionic_intr_return_credits(struct intr *intr, unsigned int credits,
                               bool unmask, bool reset_timer);
void ionic_intr_mask(struct intr *intr, bool mask);
void ionic_intr_coal_set(struct intr *intr, u32 coal_usecs);
VMK_ReturnStatus
ionic_cq_init(struct lif *lif, struct cq *cq, struct intr *intr,
              unsigned int num_descs, size_t desc_size);
void ionic_cq_map(struct cq *cq, void *base, dma_addr_t base_pa);
void ionic_cq_bind(struct cq *cq, struct queue *q);
typedef bool (*ionic_cq_cb)(struct cq *cq, struct cq_info *cq_info,
                            void *cb_arg);
unsigned int ionic_cq_service(struct cq *cq, unsigned int work_to_do,
                              ionic_cq_cb cb, void *cb_arg);

VMK_ReturnStatus
ionic_q_init(struct lif *lif, struct ionic_dev *idev, struct queue *q,
             unsigned int index, const char *base, unsigned int num_descs,
             size_t desc_size, size_t sg_desc_size, unsigned int pid);
void ionic_q_map(struct queue *q, void *base, dma_addr_t base_pa);
void ionic_q_sg_map(struct queue *q, void *base, dma_addr_t base_pa);
void ionic_q_post(struct queue *q, bool ring_doorbell, desc_cb cb,
                  void *cb_arg);
unsigned int ionic_q_space_avail(struct queue *q);
bool ionic_q_has_space(struct queue *q, unsigned int want);
void ionic_q_service(struct queue *q, struct cq_info *cq_info,
                     unsigned int stop_index);
#endif /* _IONIC_DEV_H_ */

