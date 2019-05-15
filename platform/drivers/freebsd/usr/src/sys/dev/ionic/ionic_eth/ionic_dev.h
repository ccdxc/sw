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

#ifndef _IONIC_DEV_H_
#define _IONIC_DEV_H_

#include <linux/mutex.h>

#include "ionic_if.h"
#include "ionic_api.h"
#include "ionic_regs.h"

#define IONIC_MIN_MTU	ETHER_MIN_LEN
#define IONIC_MAX_MTU	(9216 - ETHER_HDR_LEN - ETHER_VLAN_ENCAP_LEN - ETHER_CRC_LEN)

struct ionic_dev_bar
{
	void __iomem *vaddr;
	dma_addr_t bus_addr;
	unsigned long len;
};

#ifndef BUILD_BUG_ON
#define BUILD_BUG_ON(x) CTASSERT(!(x))
#endif

static inline void ionic_struct_size_checks(void)
{
	/* Registers */
	BUILD_BUG_ON(sizeof(struct ionic_intr) != 32);

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

struct ionic;

struct ionic_dev
{
	union dev_info_regs __iomem *dev_info_regs;
	union dev_cmd_regs __iomem *dev_cmd_regs;

	u64 __iomem *db_pages;
	dma_addr_t phy_db_pages;

	struct ionic_intr __iomem *intr_ctrl;
	u64 __iomem *intr_status;

	struct mutex cmb_inuse_lock; /* for cmb_inuse */
	unsigned long *cmb_inuse;
	dma_addr_t phy_cmb_pages;
	uint32_t cmb_npages;

	struct port_info *port_info;
	dma_addr_t port_info_pa;
	uint32_t port_info_sz;

	struct ionic_devinfo dev_info;
};

#define INTR_INDEX_NOT_ASSIGNED (-1)

#ifndef __FreeBSD__
#define INTR_NAME_MAX_SZ 	(32)
#else
/* Interrupt name can't be longer than MAXCOMLEN */
#define INTR_NAME_MAX_SZ 	(MAXCOMLEN)
#endif

struct intr
{
	char name[INTR_NAME_MAX_SZ];
	unsigned int index;
	unsigned int vector;
};

int ionic_dev_setup(struct ionic* ionic);

void ionic_dev_cmd_go(struct ionic_dev *idev, union dev_cmd *cmd);
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
void ionic_dev_cmd_lif_init(struct ionic_dev *idev, u32 index,
	dma_addr_t regs_base);
void ionic_dev_cmd_lif_reset(struct ionic_dev *idev, u32 index);

char *ionic_dev_asic_name(u8 asic_type);
int ionic_db_page_num(struct ionic *ionic, int lif_id, int pid);

int ionic_intr_init(struct ionic_dev *idev, struct intr *intr,
					unsigned long index);

int ionic_desc_avail(int ndescs, int head, int tail);

static const char
*port_oper_status_str(enum port_oper_status status)
{

	switch (status) {
	case PORT_OPER_STATUS_NONE:
		return "PORT_OPER_STATUS_NONE";
	case PORT_OPER_STATUS_UP:
		return "PORT_OPER_STATUS_UP";
	case PORT_OPER_STATUS_DOWN:
		return "PORT_OPER_STATUS_DOWN";
	default:
		return "PORT_OPER_STATUS_UNKNOWN";
	}
}

static const char
*port_admin_state_str(enum PortAdminState state)
{

	switch (state) {
	case PORT_ADMIN_STATE_NONE:
		return "PORT_ADMIN_STATE_NONE";
	case PORT_ADMIN_STATE_DOWN:
		return "PORT_ADMIN_STATE_DOWN";
	case PORT_ADMIN_STATE_UP:
		return "PORT_ADMIN_STATE_UP";
	default:
		return "PORT_ADMIN_STATE_UNKNOWN";
	}
}

static const char
*port_fec_type_str(enum port_fec_type type)
{

	switch (type) {
	case PORT_FEC_TYPE_NONE:
		return "PORT_FEC_TYPE_NONE";
	case PORT_FEC_TYPE_FC:
		return "PORT_FEC_TYPE_FC";
	case PORT_FEC_TYPE_RS:
		return "PORT_FEC_TYPE_RS";
	default:
		return "PORT_FEC_TYPE_UNKNOWN";
	}
}

static const char
*port_pause_type_str(enum port_pause_type type)
{

	switch (type) {
	case PORT_PAUSE_TYPE_NONE:
		return "PORT_PAUSE_TYPE_NONE";
	case PORT_PAUSE_TYPE_LINK:
		return "PORT_PAUSE_TYPE_LINK";
	case PORT_PAUSE_TYPE_PFC:
		return "PORT_PAUSE_TYPE_PFC";
	default:
		return "PORT_PAUSE_TYPE_UNKNOWN";
	}
}

static const char
*port_loopback_mode_str(enum port_loopback_mode mode)
{

	switch (mode) {
	case PORT_LOOPBACK_MODE_NONE:
		return "PORT_LOOPBACK_MODE_NONE";
	case PORT_LOOPBACK_MODE_MAC:
		return "PORT_LOOPBACK_MODE_MAC";
	case PORT_LOOPBACK_MODE_PHY:
		return "PORT_LOOPBACK_MODE_PHY";
	default:
		return "PORT_LOOPBACK_MODE_UNKNOWN";
	}
}

static const char
*xcvr_state_str(enum xcvr_state state)
{

	switch (state) {
	case XCVR_STATE_REMOVED:
		return "XCVR_STATE_REMOVED";
	case XCVR_STATE_INSERTED:
		return "XCVR_STATE_INSERTED";
	case XCVR_STATE_PENDING:
		return "XCVR_STATE_PENDING";
	case XCVR_STATE_SPROM_READ:
		return "XCVR_STATE_SPROM_READ";
	case XCVR_STATE_SPROM_READ_ERR:
		return "XCVR_STATE_SPROM_READ_ERR";
	default:
		return "XCVR_STATE_UNKNOWN";
	}
}

static const char
*phy_type_str(enum phy_type type)
{

	switch (type) {
	case PHY_TYPE_NONE:
		return "PHY_TYPE_NONE";
	case PHY_TYPE_COPPER:
		return "PHY_TYPE_COPPER";
	case PHY_TYPE_FIBER:
		return "PHY_TYPE_FIBER";
	default:
		return "PHY_TYPE_UNKNOWN";
	}
}
#endif /* _IONIC_DEV_H_ */
