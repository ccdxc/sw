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

#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/pci.h>

#include "ionic.h"
#include "ionic_bus.h"
#include "ionic_lif.h"

MODULE_DESCRIPTION(DRV_DESCRIPTION);
MODULE_AUTHOR("Anish Gupta <anish@pensando.io>");
MODULE_VERSION(ionic, 1);

const char *
ionic_port_oper_status_str(enum port_oper_status status)
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

const char *
ionic_port_admin_state_str(enum PortAdminState state)
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

const char *
ionic_port_fec_type_str(enum port_fec_type type)
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

const char *
ionic_port_pause_type_str(enum port_pause_type type)
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

const char *
ionic_port_loopback_mode_str(enum port_loopback_mode mode)
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

const char *
ionic_xcvr_state_str(enum xcvr_state state)
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

const char *
ionic_phy_type_str(enum phy_type type)
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

const char *
ionic_error_to_str(enum status_code code)
{

	switch (code) {
	case IONIC_RC_SUCCESS:
		return "IONIC_RC_SUCCESS";
	case IONIC_RC_EVERSION:
		return "IONIC_RC_EVERSION";
	case IONIC_RC_EOPCODE:
		return "IONIC_RC_EOPCODE";
	case IONIC_RC_EIO:
		return "IONIC_RC_EIO";
	case IONIC_RC_EPERM:
		return "IONIC_RC_EPERM";
	case IONIC_RC_EQID:
		return "IONIC_RC_EQID";
	case IONIC_RC_EQTYPE:
		return "IONIC_RC_EQTYPE";
	case IONIC_RC_ENOENT:
		return "IONIC_RC_ENOENT";
	case IONIC_RC_EINTR:
		return "IONIC_RC_EINTR";
	case IONIC_RC_EAGAIN:
		return "IONIC_RC_EAGAIN";
	case IONIC_RC_ENOMEM:
		return "IONIC_RC_ENOMEM";
	case IONIC_RC_EFAULT:
		return "IONIC_RC_EFAULT";
	case IONIC_RC_EBUSY:
		return "IONIC_RC_EBUSY";
	case IONIC_RC_EEXIST:
		return "IONIC_RC_EEXIST";
	case IONIC_RC_EINVAL:
		return "IONIC_RC_EINVAL";
	case IONIC_RC_ENOSPC:
		return "IONIC_RC_ENOSPC";
	case IONIC_RC_ERANGE:
		return "IONIC_RC_ERANGE";
	case IONIC_RC_BAD_ADDR:
		return "IONIC_RC_BAD_ADDR";
	case IONIC_RC_DEV_CMD:
		return "IONIC_RC_DEV_CMD";
	case IONIC_RC_ERROR:
		return "IONIC_RC_ERROR";
	case IONIC_RC_ERDMA:
		return "IONIC_RC_ERDMA";
	default:
		return "IONIC_RC_UNKNOWN";
	}
}

static const char *
ionic_opcode_to_str(enum cmd_opcode opcode)
{

	switch (opcode) {
	case CMD_OPCODE_NOP:
		return "CMD_OPCODE_NOP";
	case CMD_OPCODE_INIT:
		return "CMD_OPCODE_INIT";
	case CMD_OPCODE_RESET:
		return "CMD_OPCODE_RESET";
	case CMD_OPCODE_IDENTIFY:
		return "CMD_OPCODE_IDENTIFY";
	case CMD_OPCODE_GETATTR:
		return "CMD_OPCODE_GETATTR";
	case CMD_OPCODE_SETATTR:
		return "CMD_OPCODE_SETATTR";
	case CMD_OPCODE_PORT_IDENTIFY:
		return "CMD_OPCODE_PORT_IDENTIFY";
	case CMD_OPCODE_PORT_INIT:
		return "CMD_OPCODE_PORT_INIT";
	case CMD_OPCODE_PORT_RESET:
		return "CMD_OPCODE_PORT_RESET";
	case CMD_OPCODE_PORT_GETATTR:
		return "CMD_OPCODE_PORT_GETATTR";
	case CMD_OPCODE_PORT_SETATTR:
		return "CMD_OPCODE_PORT_SETATTR";
	case CMD_OPCODE_LIF_INIT:
		return "CMD_OPCODE_LIF_INIT";
	case CMD_OPCODE_LIF_RESET:
		return "CMD_OPCODE_LIF_RESET";
	case CMD_OPCODE_LIF_IDENTIFY:
		return "CMD_OPCODE_LIF_IDENTIFY";
	case CMD_OPCODE_LIF_SETATTR:
		return "CMD_OPCODE_LIF_SETATTR";
	case CMD_OPCODE_LIF_GETATTR:
		return "CMD_OPCODE_LIF_GETATTR";
	case CMD_OPCODE_RX_MODE_SET:
		return "CMD_OPCODE_RX_MODE_SET";
	case CMD_OPCODE_RX_FILTER_ADD:
		return "CMD_OPCODE_RX_FILTER_ADD";
	case CMD_OPCODE_RX_FILTER_DEL:
		return "CMD_OPCODE_RX_FILTER_DEL";
	case CMD_OPCODE_Q_INIT:
		return "CMD_OPCODE_Q_INIT";
	case CMD_OPCODE_Q_CONTROL:
		return "CMD_OPCODE_Q_CONTROL";
	case CMD_OPCODE_RDMA_RESET_LIF:
		return "CMD_OPCODE_RDMA_RESET_LIF";
	case CMD_OPCODE_RDMA_CREATE_EQ:
		return "CMD_OPCODE_RDMA_CREATE_EQ";
	case CMD_OPCODE_RDMA_CREATE_CQ:
		return "CMD_OPCODE_RDMA_CREATE_CQ";
	case CMD_OPCODE_RDMA_CREATE_ADMINQ:
		return "CMD_OPCODE_RDMA_CREATE_ADMINQ";
	case CMD_OPCODE_FW_DOWNLOAD:
		return "CMD_OPCODE_FW_DOWNLOAD";
	case CMD_OPCODE_FW_CONTROL:
		return "CMD_OPCODE_FW_CONTROL";
	default:
		return "DEVCMD_UNKNOWN";
	}
}

static void
ionic_adminq_flush(struct ionic_lif *lif)
{
	struct ionic_adminq *adminq = lif->adminq;
	struct admin_cmd *cmd;
	struct ionic_admin_ctx *ctx;
	int cmd_index;

	IONIC_ADMIN_LOCK(adminq);
	while (!IONIC_Q_EMPTY(adminq)) {
		cmd_index = adminq->tail_index;
		cmd = &adminq->cmd_ring[cmd_index];
		IONIC_QUE_WARN(adminq, "flushing tail: %d cmd %s(%d)\n",
		    adminq->tail_index, ionic_opcode_to_str(cmd->opcode),
		    cmd->opcode);
		ctx = adminq->ctx_ring[cmd_index];
		memset(cmd, 0, sizeof(*cmd));
		adminq->ctx_ring[cmd_index] = NULL;
		adminq->tail_index = IONIC_MOD_INC(adminq, tail_index);
	}
	IONIC_ADMIN_UNLOCK(adminq);

	IONIC_QUE_INFO(adminq, "head: %d tail: %d comp index: %d\n",
	    adminq->head_index, adminq->tail_index, adminq->comp_index);
}

static int
ionic_adminq_check_err(struct ionic_lif *lif, struct ionic_admin_ctx *ctx,
    bool timeout)
{
	struct net_device *netdev = lif->netdev;
	const char *name;
	const char *status;
	int err = 0;

	if (ctx->comp.comp.status || timeout) {
		name = ionic_opcode_to_str(ctx->cmd.cmd.opcode);
		status = ionic_error_to_str(ctx->comp.comp.status);
		IONIC_NETDEV_ERROR(netdev, "%s (%d) failed: %s (%d)\n",
		    name, ctx->cmd.cmd.opcode, timeout ? "TIMEOUT": status,
		    timeout ? -1 : ctx->comp.comp.status);

		err = timeout ? ETIMEDOUT : ctx->comp.comp.status;
		if (timeout)
			ionic_adminq_flush(lif);

	}

	return (err);
}

static void
ionic_adminq_ring_doorbell(struct ionic_adminq *adminq, int index)
{

	IONIC_QUE_INFO(adminq, "ring doorbell for index: %d\n", index);

	ionic_dbell_ring(adminq->lif->kern_dbpage,
			 adminq->hw_type,
			 adminq->dbval | index);
}

static bool
ionic_adminq_avail(struct ionic_adminq *adminq, int want)
{
	int avail;

	avail = ionic_desc_avail(adminq->num_descs,
	    adminq->head_index, adminq->tail_index);
	return (avail > want);
}

static int
ionic_adminq_post(struct ionic_lif *lif, struct ionic_admin_ctx *ctx)
{
	struct ionic_adminq *adminq = lif->adminq;
	struct admin_cmd *cmd;

	KASSERT(IONIC_LIF_LOCK_OWNED(lif), ("%s lif not locked", lif->name));
	IONIC_ADMIN_LOCK(adminq);

	if (adminq->stop) {
		IONIC_QUE_INFO(adminq, "can't post admin queue command\n");
		IONIC_ADMIN_UNLOCK(adminq);
		return (-ESHUTDOWN);
	}

	if (!ionic_adminq_avail(adminq, 1)) {
		IONIC_QUE_ERROR(adminq, "adminq is hung, head: %d tail: %d\n",
		    adminq->head_index, adminq->tail_index);
		IONIC_ADMIN_UNLOCK(adminq);
		return (-ENOSPC);
	}

	adminq->ctx_ring[adminq->head_index] = ctx;
	cmd = &adminq->cmd_ring[adminq->head_index];
	memcpy(cmd, &ctx->cmd, sizeof(ctx->cmd));

	IONIC_QUE_INFO(adminq, "post admin queue command %d@%d:\n",
	    cmd->opcode, adminq->head_index);
	if (__IONIC_DEBUG)
		print_hex_dump_debug("cmd ", DUMP_PREFIX_OFFSET, 16, 1,
		    &ctx->cmd, sizeof(ctx->cmd), true);

	adminq->head_index = (adminq->head_index + 1) % adminq->num_descs;
	ionic_adminq_ring_doorbell(adminq, adminq->head_index);

	IONIC_ADMIN_UNLOCK(adminq);

	return (0);
}

int
ionic_adminq_post_wait(struct ionic_lif *lif, struct ionic_admin_ctx *ctx)
{
	struct ionic_adminq *adminq = lif->adminq;
	struct ifnet *ifp = lif->netdev;
	struct ionic_dev *idev = &lif->ionic->idev;
	int err, remaining, processed;
	const char *name;

	KASSERT(IONIC_LIF_LOCK_OWNED(lif), ("%s lif not locked", lif->name));

	err = ionic_adminq_post(lif, ctx);
	if (err == -ESHUTDOWN) {
		name = ionic_opcode_to_str(ctx->cmd.cmd.opcode);
		IONIC_NETDEV_ERROR(ifp, "%s (%d) failed: adminq stopped\n",
		    name, ctx->cmd.cmd.opcode);
		return (err);
	}
	if (err) {
		IONIC_NETDEV_ERROR(ifp, "adminq_post failed, err: %d\n", err);
		return (err);
	}

	remaining = wait_for_completion_timeout(&ctx->work,
						ionic_devcmd_timeout * HZ);
	if (remaining == 0) {
		/* Check again in case the interrupt was missed */
		IONIC_ADMIN_LOCK(adminq);
		processed = ionic_adminq_clean(adminq, adminq->num_descs);
		if (processed) {
			ionic_intr_credits(idev->intr_ctrl, adminq->intr.index,
			    processed, IONIC_INTR_CRED_REARM);
			remaining = 1;
			IONIC_NETDEV_INFO(ifp, "adminq timeout avoided\n");
		} else {
			IONIC_NETDEV_ERROR(ifp, "adminq timeout\n");
		}
		IONIC_ADMIN_UNLOCK(adminq);
	}
	return (ionic_adminq_check_err(lif, ctx, remaining == 0));
}

int
ionic_set_dma_mask(struct ionic *ionic)
{
	struct device *dev = ionic->dev;
	int err;

	/* Set DMA addressing limitations. */
	err = dma_set_mask(dev, DMA_BIT_MASK(IONIC_ADDR_BITS));
	if (err) {
		IONIC_DEV_ERROR(dev, "No usable %d-bit DMA configuration, aborting\n",
		    IONIC_ADDR_BITS);
		return (err);
	}

	err = dma_set_coherent_mask(dev, DMA_BIT_MASK(IONIC_ADDR_BITS));
	if (err)
		IONIC_DEV_ERROR(dev, "Unable to obtain %d-bit DMA "
		    "for consistent allocations, aborting\n", IONIC_ADDR_BITS);

	dma_set_max_seg_size(dev, 2u * 1024 * 1024 * 1024);

	return (err);
}

int
ionic_identify(struct ionic *ionic)
{
	struct ionic_dev *idev = &ionic->idev;
	struct identity *ident = &ionic->ident;
	int i, err, nwords;

	ident->drv.os_type = IONIC_OS_TYPE_FREEBSD;
	ident->drv.os_dist = 0;
	strncpy(ident->drv.os_dist_str, "FreeBSD",
	    sizeof(ident->drv.os_dist_str) - 1);
	ident->drv.kernel_ver = __FreeBSD_version;
	snprintf(ident->drv.kernel_ver_str,
	    sizeof(ident->drv.kernel_ver_str) - 1, "%d", __FreeBSD_version);
	strncpy(ident->drv.driver_ver_str, DRV_VERSION,
	    sizeof(ident->drv.driver_ver_str) - 1);

	nwords = min(ARRAY_SIZE(ident->drv.words), ARRAY_SIZE(idev->dev_cmd_regs->data));
	IONIC_DEV_LOCK(ionic);
	for (i = 0; i < nwords; i++)
		iowrite32(ident->drv.words[i], &idev->dev_cmd_regs->data[i]);

	ionic_dev_cmd_identify(idev, IONIC_IDENTITY_VERSION_1);

	err = ionic_dev_cmd_wait_check(idev, ionic_devcmd_timeout * HZ);

	if (err) {
		IONIC_DEV_UNLOCK(ionic);
		goto err_out_unmap;
	}

	nwords = min(ARRAY_SIZE(ident->dev.words), ARRAY_SIZE(idev->dev_cmd_regs->data));
	for (i = 0; i < nwords; i++)
		ident->dev.words[i] = ioread32(&idev->dev_cmd_regs->data[i]);

	IONIC_DEV_UNLOCK(ionic);
	return (0);

err_out_unmap:
	return (err);
}

int
ionic_init(struct ionic *ionic)
{
	struct ionic_dev *idev = &ionic->idev;
	int err;

	IONIC_DEV_LOCK(ionic);
	ionic_dev_cmd_init(idev);
	err = ionic_dev_cmd_wait_check(idev, ionic_devcmd_timeout * HZ);
	IONIC_DEV_UNLOCK(ionic);

	return (err);
}

int
ionic_reset(struct ionic *ionic)
{
	struct ionic_dev *idev = &ionic->idev;
	int err;

	IONIC_DEV_LOCK(ionic);
	ionic_dev_cmd_reset(idev);
	err = ionic_dev_cmd_wait_check(idev, ionic_devcmd_timeout * HZ);
	IONIC_DEV_UNLOCK(ionic);

	return (err);
}

int
ionic_port_identify(struct ionic *ionic)
{
	struct ionic_dev *idev = &ionic->idev;
	struct identity *ident = &ionic->ident;
	int i, err, nwords;

	IONIC_DEV_LOCK(ionic);
	ionic_dev_cmd_port_identify(idev);
	err = ionic_dev_cmd_wait_check(idev, ionic_devcmd_timeout * HZ);
	if (!err) {
		nwords = min(ARRAY_SIZE(ident->port.words), ARRAY_SIZE(idev->dev_cmd_regs->data));
		for (i = 0; i < nwords; i++)
			ident->port.words[i] = ioread32(&idev->dev_cmd_regs->data[i]);
	}
	IONIC_DEV_UNLOCK(ionic);

	return (err);
}

int
ionic_port_init(struct ionic *ionic)
{
	struct ionic_dev *idev = &ionic->idev;
	struct identity *ident = &ionic->ident;
	union port_config *config;
	int i, err, nwords;

	if (idev->port_info)
		return (0);

	idev->port_info_sz = ALIGN(sizeof(*idev->port_info), PAGE_SIZE);

	err = ionic_dma_alloc(ionic, idev->port_info_sz, &ionic->port_dma, 0);
	if (err) {
		IONIC_DEV_ERROR(ionic->dev, "failed to allocate memory for port, err: %d\n", err);
		return (-ENOMEM);
	}

	idev->port_info = (struct port_info *)ionic->port_dma.dma_vaddr;
	if (!idev->port_info) {
		IONIC_DEV_ERROR(ionic->dev, "Failed to allocate port info, aborting\n");
		return (-ENOMEM);
	}

	IONIC_DEV_LOCK(ionic);
	idev->port_info_pa = ionic->port_dma.dma_paddr;
	nwords = min(ARRAY_SIZE(ident->port.config.words), ARRAY_SIZE(idev->dev_cmd_regs->data));
	config = &ident->port.config;

	if (!ionic->is_mgmt_nic)
		config->state = PORT_ADMIN_STATE_UP;
	for (i = 0; i < nwords; i++)
		iowrite32(config->words[i], &idev->dev_cmd_regs->data[i]);

	ionic_dev_cmd_port_init(idev);
	err = ionic_dev_cmd_wait_check(idev, ionic_devcmd_timeout * HZ);
	IONIC_DEV_UNLOCK(ionic);

	return (err);
}

int
ionic_port_reset(struct ionic *ionic)
{
	struct ionic_dev *idev = &ionic->idev;
	int err;

	if (!idev->port_info)
		return (0);

	IONIC_DEV_LOCK(ionic);
	ionic_dev_cmd_port_reset(idev);
	err = ionic_dev_cmd_wait_check(idev, ionic_devcmd_timeout * HZ);
	IONIC_DEV_UNLOCK(ionic);
	if (err) {
		IONIC_DEV_ERROR(ionic->dev, "Failed to reset port\n");
		return (err);
	}

	ionic_dma_free(ionic, &ionic->port_dma);

	idev->port_info = NULL;
	idev->port_info_pa = 0;

	return (0);
}

void
ionic_set_port_state(struct ionic *ionic, uint8_t state)
{
	struct ionic_dev *idev = &ionic->idev;
	int err;

	if (!idev->port_info)
		return;
	if (ionic->is_mgmt_nic)
		return;

	IONIC_DEV_LOCK(ionic);
	ionic_dev_cmd_port_state(idev, state);
	err = ionic_dev_cmd_wait_check(idev, ionic_devcmd_timeout * HZ);
	IONIC_DEV_UNLOCK(ionic);

	if (err)
		IONIC_DEV_ERROR(ionic->dev, "Failed to set port state %s, err: %d\n",
		    ionic_port_admin_state_str(state), err);
}

int
ionic_qos_class_identify(struct ionic *ionic)
{
	struct ionic_dev *idev = &ionic->idev;
	struct identity *ident = &ionic->ident;
	int i, err, nwords;

	IONIC_DEV_LOCK(ionic);
	ionic_dev_cmd_qos_class_identify(idev);
	err = ionic_dev_cmd_wait_check(idev, ionic_devcmd_timeout * HZ);
	if (!err) {
		nwords = min(ARRAY_SIZE(ident->qos.words), ARRAY_SIZE(idev->dev_cmd_regs->data));
		for (i = 0; i < nwords; i++)
			ident->qos.words[i] = ioread32(&idev->dev_cmd_regs->data[i]);
	}
	IONIC_DEV_UNLOCK(ionic);

	return (err);
}

int
ionic_qos_class_init(struct ionic *ionic, uint8_t group, union qos_config *config)
{
	struct ionic_dev *idev = &ionic->idev;
	int i, err, nwords;

	nwords = min(ARRAY_SIZE(config->words), ARRAY_SIZE(idev->dev_cmd_regs->data));
	IONIC_DEV_LOCK(ionic);
	for (i = 0; i < nwords; i++)
		iowrite32(config->words[i], &idev->dev_cmd_regs->data[i]);

	ionic_dev_cmd_qos_class_init(idev, group);
	err = ionic_dev_cmd_wait_check(idev, ionic_devcmd_timeout * HZ);
	IONIC_DEV_UNLOCK(ionic);

	return (err);
}

int
ionic_qos_class_reset(struct ionic *ionic, uint8_t group)
{
	struct ionic_dev *idev = &ionic->idev;
	int err;

	IONIC_DEV_LOCK(ionic);
	ionic_dev_cmd_qos_class_reset(idev, group);
	err = ionic_dev_cmd_wait_check(idev, ionic_devcmd_timeout * HZ);
	IONIC_DEV_UNLOCK(ionic);

	return (err);
}

/*
 * Validate user parameters.
 */
static void
ionic_validate_params(void)
{
	const int div = 4;

	ionic_tx_descs = max(ionic_tx_descs, IONIX_TX_MIN_DESC);
	ionic_tx_descs = min(ionic_tx_descs, IONIX_TX_MAX_DESC);

	ionic_rx_descs = max(ionic_rx_descs, IONIX_RX_MIN_DESC);
	ionic_rx_descs = min(ionic_rx_descs, IONIX_RX_MAX_DESC);
	/* SGL size validation */
	if (ionic_rx_sg_size > MCLBYTES) {
		ionic_rx_sg_size = MJUMPAGESIZE;
	} else if (ionic_rx_sg_size) {
		ionic_rx_sg_size = MCLBYTES;
	}

	/* Doorbell stride has to be between 1 <=  < descs */
	ionic_rx_stride = max(ionic_rx_stride, 1);
	ionic_rx_stride = min(ionic_rx_stride, ionic_rx_descs / div);
	ionic_tx_stride = max(ionic_tx_stride, 1);
	ionic_tx_stride = min(ionic_tx_stride, ionic_tx_descs / div);

	/* Adjust Rx fill threshold. */
	if (ionic_rx_fill_threshold >= ionic_rx_descs / div)
		ionic_rx_fill_threshold /= div;
}

static int __init
ionic_init_module(void)
{

	ionic_struct_size_checks();
	ionic_validate_params();
	pr_info("%s, ver: %s\n", DRV_DESCRIPTION, DRV_VERSION);

	return (ionic_bus_register_driver());
}

static void __exit
ionic_cleanup_module(void)
{

	ionic_bus_unregister_driver();
}

module_init(ionic_init_module);
module_exit(ionic_cleanup_module);

MODULE_DEPEND(ionic, linuxkpi, 1, 1, 1);
