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

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/slab.h>

#include "ionic.h"
#include "ionic_dev.h"

static void
ionic_init_devinfo(struct ionic_dev *idev)
{

	idev->dev_info.asic_type = ioread8(&idev->dev_info_regs->asic_type);
	idev->dev_info.asic_rev = ioread8(&idev->dev_info_regs->asic_rev);

	memcpy_fromio(idev->dev_info.fw_version,
	    idev->dev_info_regs->fw_version,
	    IONIC_DEVINFO_FWVERS_BUFLEN);

	memcpy_fromio(idev->dev_info.serial_num,
	    idev->dev_info_regs->serial_num,
	    IONIC_DEVINFO_SERIAL_BUFLEN);

	idev->dev_info.fw_version[IONIC_DEVINFO_FWVERS_BUFLEN] = 0;
	idev->dev_info.serial_num[IONIC_DEVINFO_SERIAL_BUFLEN] = 0;
}

static const char *
ionic_dev_asic_name(u8 asic_type)
{
	switch (asic_type) {
	case IONIC_ASIC_TYPE_CAPRI:
		return "Capri";
	default:
		return "Unknown";
	}
}

int
ionic_dev_setup(struct ionic *ionic)
{
	struct ionic_dev_bar *bar = ionic->bars;
	struct ionic_dev *idev = &ionic->idev;
	struct device *dev = ionic->dev;
	uint32_t sig;
	int num_bars = ionic->num_bars;

	/*
	 * BAR0 resources
	 */

	if (num_bars < 1) {
		dev_info(dev, "No bars found, aborting\n");
		return (-EFAULT);
	}

	if (bar->len < IONIC_BAR0_SIZE) {
		dev_info(dev, "Resource bar size %lu too small, aborting\n",
		    bar->len);
		return (-EFAULT);
	}

	idev->dev_info_regs = bar->vaddr + IONIC_BAR0_DEV_INFO_REGS_OFFSET;
	idev->dev_cmd_regs = bar->vaddr + IONIC_BAR0_DEV_CMD_REGS_OFFSET;
	idev->intr_status = bar->vaddr + IONIC_BAR0_INTR_STATUS_OFFSET;
	idev->intr_ctrl = bar->vaddr + IONIC_BAR0_INTR_CTRL_OFFSET;

	sig = ioread32(&idev->dev_info_regs->signature);
	if (sig != IONIC_DEV_INFO_SIGNATURE) {
		IONIC_DEV_ERROR(dev, "Incompatible firmware signature %x != 0x%x",
		    sig, IONIC_DEV_INFO_SIGNATURE);
		return (-EFAULT);
	}

	ionic_init_devinfo(idev);

	dev_info(dev, "ASIC: %s h/w-rev: 0x%X serial-num: %s fw-ver: %s\n",
	    ionic_dev_asic_name(idev->dev_info.asic_type),
	    idev->dev_info.asic_rev, idev->dev_info.serial_num,
	    idev->dev_info.fw_version);

	/*
	 * BAR1 resources
	 */
	bar++;

	idev->db_pages = bar->vaddr;
	idev->phy_db_pages = bar->bus_addr;

	/*
	 * BAR2 resources
	 */

	mutex_init(&idev->cmb_inuse_lock);

	bar++;
	if (num_bars < 3) {
		idev->phy_cmb_pages = 0;
		idev->cmb_npages = 0;
		idev->cmb_inuse = NULL;
		return (0);
	}

	idev->phy_cmb_pages = bar->bus_addr;
	idev->cmb_npages = bar->len / PAGE_SIZE;
	idev->cmb_inuse = malloc(BITS_TO_LONGS(idev->cmb_npages) * sizeof(long),
	     M_IONIC, M_WAITOK | M_ZERO);
	if (!idev->cmb_inuse) {
		idev->phy_cmb_pages = 0;
		idev->cmb_npages = 0;
	}

	return (0);
}

/* Devcmd Interface */
static uint8_t
ionic_dev_cmd_status(struct ionic_dev *idev)
{

	return (ioread8(&idev->dev_cmd_regs->comp.status));
}

static bool
ionic_dev_cmd_done(struct ionic_dev *idev)
{

	return (ioread32(&idev->dev_cmd_regs->done) & IONIC_DEV_CMD_DONE);
}

static void
ionic_dev_cmd_disable(struct ionic_dev *idev)
{
	struct ionic *ionic = container_of(idev, struct ionic, idev);

	KASSERT(IONIC_DEV_LOCK_OWNED(ionic), ("device not locked"));

	if (idev->dev_cmd_disabled)
		return;

	/*
	 * Respond to timeout or heartbeat failure by disabling the device
	 * command interface. This will allow the driver to fail quickly,
	 * so the module can be unloaded without waiting for many teardown
	 * commands to time out one after the other.
	 * After the failure, the driver no longer knows what state the
	 * device is in. The only way to recover the device and clear this
	 * flag is to unload and reload the module or reboot the system.
	 */
	IONIC_DEV_ERROR(ionic->dev, "disabling dev_cmd interface\n");
	idev->dev_cmd_disabled = true;
}

static bool
ionic_dev_cmd_disabled(struct ionic_dev *idev)
{

	return (idev->dev_cmd_disabled);
}

void
ionic_dev_cmd_comp(struct ionic_dev *idev, void *mem)
{
	union ionic_dev_cmd_comp *comp = mem;
	int i;

	for (i = 0; i < ARRAY_SIZE(comp->words); i++)
		comp->words[i] = ioread32(&idev->dev_cmd_regs->comp.words[i]);
}

static bool
ionic_dev_cmd_is_exempt(union ionic_dev_cmd *cmd)
{
	return (cmd->cmd.opcode == IONIC_CMD_LIF_RESET ||
		cmd->cmd.opcode == IONIC_CMD_PORT_RESET ||
		cmd->cmd.opcode == IONIC_CMD_RDMA_RESET_LIF ||
		cmd->cmd.opcode == IONIC_CMD_RESET);
}

void
ionic_dev_cmd_go(struct ionic_dev *idev, union ionic_dev_cmd *cmd)
{
	int i;

	/* Bail out if the interface was disabled in response to an error */
	if (ionic_dev_cmd_disabled(idev) &&
	    !ionic_dev_cmd_is_exempt(cmd))
		return;

	KASSERT(!idev->dev_cmd_pending, ("dev_cmd already pending"));
	idev->dev_cmd_pending = true;

	for (i = 0; i < ARRAY_SIZE(cmd->words); i++)
		iowrite32(cmd->words[i], &idev->dev_cmd_regs->cmd.words[i]);
	iowrite32(0, &idev->dev_cmd_regs->done);
	iowrite32(1, &idev->dev_cmd_regs->doorbell);
}

static void
ionic_dev_cmd_nop(struct ionic_dev *idev)
{
	union ionic_dev_cmd cmd = {
		.nop.opcode = IONIC_CMD_NOP,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

/* Device commands */
void
ionic_dev_cmd_identify(struct ionic_dev *idev, uint16_t ver)
{
	union ionic_dev_cmd cmd = {
		.identify.opcode = IONIC_CMD_IDENTIFY,
		.identify.ver = ver,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_init(struct ionic_dev *idev)
{
	union ionic_dev_cmd cmd = {
		.init.opcode = IONIC_CMD_INIT,
		.init.type = 0,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_reset(struct ionic_dev *idev)
{
	union ionic_dev_cmd cmd = {
		.reset.opcode = IONIC_CMD_RESET,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

/* Port commands */
void
ionic_dev_cmd_port_identify(struct ionic_dev *idev)
{
	union ionic_dev_cmd cmd = {
		.port_init.opcode = IONIC_CMD_PORT_IDENTIFY,
		.port_init.index = 0,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_port_init(struct ionic_dev *idev)
{
	union ionic_dev_cmd cmd = {
		.port_init.opcode = IONIC_CMD_PORT_INIT,
		.port_init.index = 0,
		.port_init.info_pa = idev->port_info_pa,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_port_reset(struct ionic_dev *idev)
{
	union ionic_dev_cmd cmd = {
		.port_reset.opcode = IONIC_CMD_PORT_RESET,
		.port_reset.index = 0,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_port_state(struct ionic_dev *idev, uint8_t state)
{
	union ionic_dev_cmd cmd = {
		.port_setattr.opcode = IONIC_CMD_PORT_SETATTR,
		.port_setattr.index = 0,
		.port_setattr.attr = IONIC_PORT_ATTR_STATE,
		.port_setattr.state = state,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_port_speed(struct ionic_dev *idev, uint32_t speed)
{
	union ionic_dev_cmd cmd = {
		.port_setattr.opcode = IONIC_CMD_PORT_SETATTR,
		.port_setattr.index = 0,
		.port_setattr.attr = IONIC_PORT_ATTR_SPEED,
		.port_setattr.speed = speed,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_port_mtu(struct ionic_dev *idev, uint32_t mtu)
{
	union ionic_dev_cmd cmd = {
		.port_setattr.opcode = IONIC_CMD_PORT_SETATTR,
		.port_setattr.index = 0,
		.port_setattr.attr = IONIC_PORT_ATTR_MTU,
		.port_setattr.mtu = mtu,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_port_autoneg(struct ionic_dev *idev, uint8_t an_enable)
{
	union ionic_dev_cmd cmd = {
		.port_setattr.opcode = IONIC_CMD_PORT_SETATTR,
		.port_setattr.index = 0,
		.port_setattr.attr = IONIC_PORT_ATTR_AUTONEG,
		.port_setattr.an_enable = an_enable,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_port_fec(struct ionic_dev *idev, uint8_t fec_type)
{
	union ionic_dev_cmd cmd = {
		.port_setattr.opcode = IONIC_CMD_PORT_SETATTR,
		.port_setattr.index = 0,
		.port_setattr.attr = IONIC_PORT_ATTR_FEC,
		.port_setattr.fec_type = fec_type,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_port_pause(struct ionic_dev *idev, uint8_t pause_type)
{
	union ionic_dev_cmd cmd = {
		.port_setattr.opcode = IONIC_CMD_PORT_SETATTR,
		.port_setattr.index = 0,
		.port_setattr.attr = IONIC_PORT_ATTR_PAUSE,
		.port_setattr.pause_type = pause_type,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_port_loopback(struct ionic_dev *idev, uint8_t loopback_mode)
{
	union ionic_dev_cmd cmd = {
		.port_setattr.opcode = IONIC_CMD_PORT_SETATTR,
		.port_setattr.index = 0,
		.port_setattr.attr = IONIC_PORT_ATTR_LOOPBACK,
		.port_setattr.loopback_mode = loopback_mode,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_port_reset_stats(struct ionic_dev *idev)
{
	union ionic_dev_cmd cmd = {
		.port_setattr.opcode = IONIC_CMD_PORT_SETATTR,
		.port_setattr.index = 0,
		.port_setattr.attr = IONIC_PORT_ATTR_STATS_CTRL,
		.port_setattr.stats_ctl = IONIC_STATS_CTL_RESET,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

/* LIF commands */
void
ionic_dev_cmd_lif_identify(struct ionic_dev *idev, uint8_t type, uint8_t ver)
{
	union ionic_dev_cmd cmd = {
		.lif_identify.opcode = IONIC_CMD_LIF_IDENTIFY,
		.lif_identify.type = type,
		.lif_identify.ver = ver,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_lif_init(struct ionic_dev *idev, uint32_t index, dma_addr_t addr)
{
	union ionic_dev_cmd cmd = {
		.lif_init.opcode = IONIC_CMD_LIF_INIT,
		.lif_init.index = index,
		.lif_init.info_pa = addr,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_lif_reset(struct ionic_dev *idev, uint32_t index)
{
	union ionic_dev_cmd cmd = {
		.lif_init.opcode = IONIC_CMD_LIF_RESET,
		.lif_init.index = index,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

/* QoS commands */
void
ionic_dev_cmd_qos_class_identify(struct ionic_dev *idev)
{
	union ionic_dev_cmd cmd = {
		.qos_identify.opcode = IONIC_CMD_QOS_CLASS_IDENTIFY,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_qos_class_init(struct ionic_dev *idev, uint8_t group)
{
	union ionic_dev_cmd cmd = {
		.qos_init.opcode = IONIC_CMD_QOS_CLASS_INIT,
		.qos_init.group = group,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_qos_class_update(struct ionic_dev *idev, uint8_t group)
{
	union ionic_dev_cmd cmd = {
		.qos_init.opcode = IONIC_CMD_QOS_CLASS_UPDATE,
		.qos_init.group = group,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_qos_class_reset(struct ionic_dev *idev, uint8_t group)
{
	union ionic_dev_cmd cmd = {
		.qos_reset.opcode = IONIC_CMD_QOS_CLASS_RESET,
		.qos_reset.group = group,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

/* Queue commands */
void
ionic_dev_cmd_q_identify(struct ionic_dev *idev, uint8_t lif_type,
	uint8_t type, uint8_t ver)
{
	union ionic_dev_cmd cmd = {
		.q_identify.opcode = IONIC_CMD_Q_IDENTIFY,
		.q_identify.lif_type = lif_type,
		.q_identify.type = type,
		.q_identify.ver = ver,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

int
ionic_db_page_num(struct ionic *ionic, int lif_index, int pid)
{

	return (lif_index * ionic->ident.dev.ndbpgs_per_lif + pid);
}

int
ionic_intr_init(struct ionic_dev *idev, struct intr *intr,
		    unsigned long index)
{

	ionic_intr_clean(idev->intr_ctrl, index);
	intr->index = index;

	return (0);
}

int
ionic_desc_avail(int ndescs, int head, int tail)
{
	int avail = tail;

	if (head >= tail)
		avail += ndescs - head - 1;
	else
		avail -= head + 1;

	return (avail);
}

static int
ionic_dev_cmd_check_error(struct ionic_dev *idev)
{
	u8 status;

	status = ionic_dev_cmd_status(idev);
	if (status) {
		IONIC_ERROR("DEVCMD(%d) failed, status: %s\n",
		    idev->dev_cmd_regs->cmd.cmd.opcode,
		    ionic_error_to_str(status));
		return (EIO);
	}

	return (0);
}

#define IONIC_DEV_CMD_WARN_DELAY_MS 3000
static int
ionic_dev_cmd_wait(struct ionic_dev *idev, unsigned long max_wait,
    bool sleepable_ctx)
{
	unsigned long cmd_start, cmd_timo, msecs;
	int done, waits = 0, err = 0;

	/* Bail out if the interface was disabled in response to an error */
	if (!idev->dev_cmd_pending)
		return (-ENXIO);

	cmd_start = ticks;
	cmd_timo = cmd_start + max_wait;
	do {
		done = ionic_dev_cmd_done(idev);
		if (done) {
			msecs = (ticks - cmd_start) * 1000 / HZ;
			if (msecs > IONIC_DEV_CMD_WARN_DELAY_MS) {
				IONIC_ERROR("DEVCMD took %lums (%d waits)\n",
				    msecs, waits);
			} else {
				IONIC_INFO("DEVCMD took %lums (%d waits)\n",
				    msecs, waits);
			}
			goto out;
		}

		/* Either sleep for 100ms or spin for 1ms */
		if (sleepable_ctx)
			schedule_timeout_uninterruptible(HZ / 10);
		else
			/* XXX: should use msleep, but lack mtx access */
			DELAY(1000);
		waits++;
	} while (time_after(cmd_timo, ticks));

	msecs = (ticks - cmd_start) * 1000 / HZ;

	/* Last chance */
	done = ionic_dev_cmd_done(idev);
	if (done) {
		if (msecs > IONIC_DEV_CMD_WARN_DELAY_MS) {
			IONIC_ERROR("DEVCMD took %lums (%d waits)\n",
			    msecs, waits);
		} else {
			IONIC_INFO("DEVCMD took %lums (%d waits)\n",
			    msecs, waits);
		}
		goto out;
	}

	IONIC_ERROR("DEVCMD timeout after %lums (%d waits)\n", msecs, waits);
	err = -ETIMEDOUT;

out:
	idev->dev_cmd_pending = false;
	return (err);
}

int
ionic_dev_cmd_wait_check(struct ionic_dev *idev, unsigned long max_wait)
{
	int err;

	/* Preserve current behavior by declaring a non-sleepable ctx */
	err = ionic_dev_cmd_wait(idev, max_wait, false);
	if (!err)
		err = ionic_dev_cmd_check_error(idev);
	if (err == -ETIMEDOUT)
		ionic_dev_cmd_disable(idev);

	return (err);
}

int
ionic_dev_cmd_sleep_check(struct ionic_dev *idev, unsigned long max_wait)
{
	int err;

	err = ionic_dev_cmd_wait(idev, max_wait, true);
	if (!err)
		err = ionic_dev_cmd_check_error(idev);
	if (err == -ETIMEDOUT)
		ionic_dev_cmd_disable(idev);

	return (err);
}

static void
ionic_cmd_hb_work(struct work_struct *work)
{
	struct ionic_dev *idev =
		container_of(work, struct ionic_dev, cmd_hb_work.work);
	struct ionic *ionic = container_of(idev, struct ionic, idev);
	int err;

	if (!idev->cmd_hb_interval)
		return;

	/* Send a NOP command to monitor dev command queue */
	IONIC_DEV_LOCK(ionic);
	ionic_dev_cmd_nop(idev);
	err = ionic_dev_cmd_sleep_check(idev, ionic_devcmd_timeout * HZ);
	if (ionic_wdog_error_trigger == IONIC_WDOG_TRIG_DEVCMD) {
		IONIC_DEV_WARN(ionic->dev, "injecting error\n");
		err = -1;
		ionic_wdog_error_trigger = 0;
	}
	if (err) {
		IONIC_DEV_ERROR(ionic->dev, "command heartbeat failed\n");
		ionic_dev_cmd_disable(idev);
		IONIC_DEV_UNLOCK(ionic);

		/* Disable the heartbeat */
		idev->cmd_hb_interval = 0;
		return;
	}
	IONIC_DEV_UNLOCK(ionic);

	IONIC_WDOG_LOCK(idev);
	if (idev->cmd_hb_resched)
		queue_delayed_work(idev->wdog_wq, &idev->cmd_hb_work,
		    idev->cmd_hb_interval);
	IONIC_WDOG_UNLOCK(idev);
}

static void
ionic_cmd_hb_stop(struct ionic_dev *idev)
{

	IONIC_WDOG_LOCK(idev);
	idev->cmd_hb_resched = false;
	IONIC_WDOG_UNLOCK(idev);
	cancel_delayed_work_sync(&idev->cmd_hb_work);
}

void
ionic_cmd_hb_resched(struct ionic_dev *idev)
{

	/* Cancel all outstanding work */
	ionic_cmd_hb_stop(idev);

	/* Start again with the new hb_interval */
	IONIC_WDOG_LOCK(idev);
	idev->cmd_hb_resched = true;
	queue_delayed_work(idev->wdog_wq, &idev->cmd_hb_work,
	    idev->cmd_hb_interval);
	IONIC_WDOG_UNLOCK(idev);
}

static void
ionic_fw_hb_work(struct work_struct *work)
{
	struct ionic_dev *idev =
		container_of(work, struct ionic_dev, fw_hb_work.work);
	struct ionic *ionic = container_of(idev, struct ionic, idev);
	uint32_t fw_heartbeat;
	uint8_t fw_status;

	if (idev->fw_hb_state == IONIC_FW_HB_DISABLED ||
	    idev->fw_hb_state == IONIC_FW_HB_UNSUPPORTED)
		return;

	/*
	 * Firmware is useful only if fw_status has the running bit set and
	 * is not 0xff (dead PCI)
	 */
	fw_status = ioread8(&idev->dev_info_regs->fw_status);
	if (ionic_wdog_error_trigger == IONIC_WDOG_TRIG_FWSTAT) {
		/* Persistent, don't reset trigger to 0 */
		/* NB: Set with a hint */
		IONIC_DEV_WARN(ionic->dev, "injecting fw_status 0\n");
		fw_status = 0;
	}

	if (fw_status == 0xff) {
		IONIC_DEV_ERROR(ionic->dev,
				"fw status unreadable (%#x)\n", fw_status);
		goto disable;
	}

	fw_status &= IONIC_FW_STS_F_RUNNING;  /* use only the run bit */

	/* If FW is ready, check fw_heartbeat; otherwise reschedule */
	if (fw_status != 0) {
		fw_heartbeat = ioread32(&idev->dev_info_regs->fw_heartbeat);

		if (ionic_wdog_error_trigger == IONIC_WDOG_TRIG_FWHB0) {
			/* NB: Set with a hint */
			IONIC_DEV_WARN(ionic->dev,
			    "injecting fw_heartbeat 0\n");
			fw_heartbeat = 0;
			ionic_wdog_error_trigger = 0;
		} else if (ionic_wdog_error_trigger == IONIC_WDOG_TRIG_FWHB1) {
			/* Persistent, don't reset trigger to 0 */
			IONIC_DEV_WARN(ionic->dev,
			    "injecting fw_heartbeat 1\n");
			fw_heartbeat = 1;
		}
		if (idev->fw_hb_state == IONIC_FW_HB_INIT) {
			if (fw_heartbeat == 0) {
				/* Unsupported firmware */
				IONIC_DEV_WARN(ionic->dev,
				    "fw heartbeat not supported\n");
				idev->fw_hb_state = IONIC_FW_HB_UNSUPPORTED;
				idev->fw_hb_interval = 0;
				return;
			} else {
				/* First reading; go RUNNING */
				idev->fw_hb_state = IONIC_FW_HB_RUNNING;
			}
		} else if (fw_heartbeat == idev->fw_hb_last) {
			/* Duplicate reading; go STALE or time out */
			if (idev->fw_hb_state == IONIC_FW_HB_RUNNING) {
				idev->fw_hb_state = IONIC_FW_HB_STALE;
			} else if (idev->fw_hb_state == IONIC_FW_HB_STALE) {
				IONIC_DEV_INFO(ionic->dev,
				    "fw heartbeat stuck (%u)\n", fw_heartbeat);
			}
		} else {
			/* Update stored value; go RUNNING */
			idev->fw_hb_last = fw_heartbeat;
			if (idev->fw_hb_state == IONIC_FW_HB_STALE) {
				idev->fw_hb_state = IONIC_FW_HB_RUNNING;
				IONIC_DEV_INFO(ionic->dev,
				    "fw heartbeat recovered (%u)\n", fw_heartbeat);
			}
		}
	}

	IONIC_WDOG_LOCK(idev);
	if (idev->fw_hb_resched)
		queue_delayed_work(idev->wdog_wq, &idev->fw_hb_work,
		    idev->fw_hb_interval);
	IONIC_WDOG_UNLOCK(idev);
	return;

disable:
	IONIC_DEV_LOCK(ionic);
	ionic_dev_cmd_disable(idev);
	IONIC_DEV_UNLOCK(ionic);

	/* Disable the heartbeat */
	idev->fw_hb_state = IONIC_FW_HB_DISABLED;
	idev->fw_hb_interval = 0;
}

void
ionic_fw_hb_start(struct ionic_dev *idev)
{

	IONIC_WDOG_LOCK(idev);
	idev->fw_hb_resched = true;
	queue_delayed_work(idev->wdog_wq, &idev->fw_hb_work,
	    idev->fw_hb_interval);
	IONIC_WDOG_UNLOCK(idev);
}

void
ionic_fw_hb_stop(struct ionic_dev *idev)
{

	IONIC_WDOG_LOCK(idev);
	idev->fw_hb_resched = false;
	IONIC_WDOG_UNLOCK(idev);
	cancel_delayed_work_sync(&idev->fw_hb_work);
}

void
ionic_fw_hb_resched(struct ionic_dev *idev)
{
	/* Restart fw heartbeat with new parameters. */
	ionic_fw_hb_stop(idev);

	ionic_fw_hb_start(idev);
}

int
ionic_wdog_init(struct ionic *ionic)
{
	struct ionic_dev *idev = &ionic->idev;
	char name[16];

	snprintf(name, sizeof(name), "devwdwq%d",
	    le32_to_cpu(idev->port_info->status.id));
	idev->wdog_wq = create_singlethread_workqueue(name);
	INIT_DELAYED_WORK(&idev->cmd_hb_work, ionic_cmd_hb_work);
	INIT_DELAYED_WORK(&idev->fw_hb_work, ionic_fw_hb_work);
	IONIC_WDOG_LOCK_INIT(idev);

	/* Device command heartbeat watchdog */
	if (ionic_cmd_hb_interval > 0 &&
	    ionic_cmd_hb_interval < IONIC_WDOG_MIN_MS) {
		IONIC_DEV_WARN(ionic->dev,
		    "limiting cmd_hb_interval to %ums\n", IONIC_WDOG_MIN_MS);
		ionic_cmd_hb_interval = IONIC_WDOG_MIN_MS;
	}
	idev->cmd_hb_interval =
	     (unsigned long)ionic_cmd_hb_interval * HZ / 1000;

	IONIC_WDOG_LOCK(idev);
	idev->cmd_hb_resched = true;
	queue_delayed_work(idev->wdog_wq, &idev->cmd_hb_work,
	    idev->cmd_hb_interval);
	IONIC_WDOG_UNLOCK(idev);

	/* Firmware heartbeat */
	if (ionic_fw_hb_interval > 0 &&
	    ionic_fw_hb_interval < IONIC_WDOG_MIN_MS) {
		IONIC_DEV_WARN(ionic->dev,
		    "limiting fw_hb_interval to %ums\n", IONIC_WDOG_MIN_MS);
		ionic_fw_hb_interval = IONIC_WDOG_MIN_MS;
	}
	if (ionic_fw_hb_interval > 0 &&
	    ionic_fw_hb_interval < IONIC_WDOG_FW_WARN_MS) {
		IONIC_DEV_WARN(ionic->dev,
		    "setting fw_hb_interval below %ums will "
		    "cause spurious timeouts\n", IONIC_WDOG_FW_WARN_MS);
	}
	idev->fw_hb_interval =
	     (unsigned long)ionic_fw_hb_interval * HZ / 1000;
	idev->fw_hb_state = ionic_fw_hb_interval ?
	     IONIC_FW_HB_INIT : IONIC_FW_HB_DISABLED;

	ionic_fw_hb_start(idev);

	return (0);
}

void
ionic_wdog_deinit(struct ionic *ionic)
{
	struct ionic_dev *idev = &ionic->idev;

	ionic_cmd_hb_stop(idev);
	ionic_fw_hb_stop(idev);
	destroy_workqueue(idev->wdog_wq);
	IONIC_WDOG_LOCK_DESTROY(idev);
}
