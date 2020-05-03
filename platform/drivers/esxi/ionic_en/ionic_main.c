/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 */

/*
 * ionic_main.c --
 *
 * Implement native ionic_en driver
 */

#include "ionic.h"
#include "ionic_lif.h"

struct ionic_driver ionic_driver;

unsigned int ntxq_descs = 1024 * 8;
unsigned int nrxq_descs = 1024;
unsigned int DRSS = 16;
unsigned int devcmd_timeout = 50;
vmk_uint32 log_level = IONIC_LOG_LEVEL_INFO;
unsigned int vlan_tx_insert = 0;
unsigned int vlan_rx_strip = 0;
unsigned int geneve_offload = 1;


VMK_MODPARAM(ntxq_descs,
             uint,
             "Descriptors per Tx queue, must be power of 2");
VMK_MODPARAM(nrxq_descs,
             uint,
             "Descriptors per Rx queue, must be power of 2");
VMK_MODPARAM(DRSS,
             uint,
             "Number of HW queues for Device RSS");
VMK_MODPARAM(devcmd_timeout,
             uint,
             "Devcmd timeout in seconds (default 50 secs)");
VMK_MODPARAM(log_level,
             uint,
             "Log level, 0 - No Log, 1 - Error, 2 - Warning,"\
             " 3 - Info, 4 - Debug. Default level: 3(Info)");

VMK_MODPARAM(vlan_tx_insert,
             uint,
             "0 - Disable, 1 - Enable. Default(Disabled)");
VMK_MODPARAM(vlan_rx_strip,
             uint,
             "0 - Disable, 1 - Enable. Default(Disabled)");

VMK_MODPARAM(geneve_offload,
             uint,
             "0 - Disable, 1 - Enable. Default(Enabled)");


static VMK_ReturnStatus
ionic_validate_module_params()
{
        VMK_ReturnStatus status = VMK_FAILURE;
        if (ntxq_descs < MIN_NUM_TX_DESC || ntxq_descs > MAX_NUM_TX_DESC) {
                vmk_AlertMessage("Number of Tx desc should be set"
                                 " between %d and %d",
                                 MIN_NUM_TX_DESC, MAX_NUM_TX_DESC);
                return status;
        }

        if (!ionic_is_power_of_2(ntxq_descs)) {
                vmk_AlertMessage("Number of Tx desc  must be power of 2");
                return status;
        }

        if (nrxq_descs < MIN_NUM_RX_DESC || nrxq_descs > MAX_NUM_RX_DESC) {
                vmk_AlertMessage("Number of Rx desc should be set"
                                 " between %d and %d",
                                 MIN_NUM_RX_DESC, MAX_NUM_RX_DESC);
                return status;
        }

        if (!ionic_is_power_of_2(nrxq_descs)) {
                vmk_AlertMessage("Number of Rx desc must be power of 2");
                return status;
        }

        if (DRSS) {
                if (!ionic_is_power_of_2(DRSS)) {
                        vmk_AlertMessage("Number of HW queues for Device RSS"
                                         " must be power of 2");
                        return status;
                }
        }

        if (devcmd_timeout > IONIC_MAX_DEVCMD_TIMEOUT) {
                vmk_WarningMessage("Devcmd timeout: %d is too big, "
                                   "change it to default value: %d",
                                   devcmd_timeout,
                                   IONIC_DEFAULT_DEVCMD_TIMEOUT);
                devcmd_timeout = IONIC_DEFAULT_DEVCMD_TIMEOUT;
        }

        return VMK_OK;
}


static const char *ionic_en_error_to_str(enum ionic_status_code code)
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

static const char *ionic_opcode_to_str(enum ionic_cmd_opcode opcode)
{
	switch (opcode) {
	case IONIC_CMD_NOP:
		return "IONIC_CMD_NOP";
	case IONIC_CMD_INIT:
		return "IONIC_CMD_INIT";
	case IONIC_CMD_RESET:
		return "IONIC_CMD_RESET";
	case IONIC_CMD_IDENTIFY:
		return "IONIC_CMD_IDENTIFY";
	case IONIC_CMD_GETATTR:
		return "IONIC_CMD_GETATTR";
	case IONIC_CMD_SETATTR:
		return "IONIC_CMD_SETATTR";
	case IONIC_CMD_PORT_IDENTIFY:
		return "IONIC_CMD_PORT_IDENTIFY";
	case IONIC_CMD_PORT_INIT:
		return "IONIC_CMD_PORT_INIT";
	case IONIC_CMD_PORT_RESET:
		return "IONIC_CMD_PORT_RESET";
	case IONIC_CMD_PORT_GETATTR:
		return "IONIC_CMD_PORT_GETATTR";
	case IONIC_CMD_PORT_SETATTR:
		return "IONIC_CMD_PORT_SETATTR";
	case IONIC_CMD_LIF_INIT:
		return "IONIC_CMD_LIF_INIT";
	case IONIC_CMD_LIF_RESET:
		return "IONIC_CMD_LIF_RESET";
	case IONIC_CMD_LIF_IDENTIFY:
		return "IONIC_CMD_LIF_IDENTIFY";
	case IONIC_CMD_LIF_SETATTR:
		return "IONIC_CMD_LIF_SETATTR";
	case IONIC_CMD_LIF_GETATTR:
		return "IONIC_CMD_LIF_GETATTR";
	case IONIC_CMD_RX_MODE_SET:
		return "IONIC_CMD_RX_MODE_SET";
	case IONIC_CMD_RX_FILTER_ADD:
		return "IONIC_CMD_RX_FILTER_ADD";
	case IONIC_CMD_RX_FILTER_DEL:
		return "IONIC_CMD_RX_FILTER_DEL";
	case IONIC_CMD_Q_INIT:
		return "IONIC_CMD_Q_INIT";
	case IONIC_CMD_Q_CONTROL:
		return "IONIC_CMD_Q_CONTROL";
	case IONIC_CMD_RDMA_RESET_LIF:
		return "IONIC_CMD_RDMA_RESET_LIF";
	case IONIC_CMD_RDMA_CREATE_EQ:
		return "IONIC_CMD_RDMA_CREATE_EQ";
	case IONIC_CMD_RDMA_CREATE_CQ:
		return "IONIC_CMD_RDMA_CREATE_CQ";
	case IONIC_CMD_RDMA_CREATE_ADMINQ:
		return "IONIC_CMD_RDMA_CREATE_ADMINQ";
	case IONIC_CMD_FW_DOWNLOAD:
		return "IONIC_CMD_FW_DOWNLOAD";
	case IONIC_CMD_FW_CONTROL:
		return "IONIC_CMD_FW_CONTROL";
	default:
		return "DEVCMD_UNKNOWN";
	}
}

VMK_ReturnStatus
ionic_adminq_check_err(struct lif *lif,
                       struct ionic_admin_ctx *ctx,
                       vmk_Bool is_timeout)
{
        VMK_ReturnStatus status = VMK_FAILURE;
        const char *name;
        const char *status_str;

        if (VMK_LIKELY(ctx)) {
                name = ionic_opcode_to_str(ctx->cmd.cmd.opcode);
        } else {
                ionic_en_warn("ctx is null");
                return VMK_FAILURE;
        }

        if (VMK_UNLIKELY(is_timeout)) {
                do {
                        status = ionic_heartbeat_check(lif->ionic);
                        if (status == VMK_STATUS_PENDING) {
                                vmk_WorldSleep(VMK_MSEC_PER_SEC);
                        } else if (status == VMK_FAILURE) {
                                status = VMK_TIMEOUT;
                                break;
                        } else if (status == VMK_OK) {
                                /* if FW is alive, we can try
                                 * one more time */
                                status = VMK_RETRY;
                                break;
                        }
                } while(status == VMK_STATUS_PENDING);

                ionic_en_err("%s: %s (%d) timeout",
                             vmk_NameToString(&lif->uplink_handle->uplink_name),
                             name,
                             ctx->cmd.cmd.opcode);

                ionic_adminq_flush(lif);
        } else {
                if (VMK_LIKELY(!ctx->comp.comp.status)) {
                        status = VMK_OK;
                } else if (ctx->comp.comp.status) {
                        /* For FW upgrade use */
                        if (ctx->cmd.cmd.opcode == IONIC_CMD_RX_FILTER_DEL &&
                            ctx->comp.comp.status == IONIC_RC_ENOENT) {
                                return VMK_OK;
                        }
	        	status_str = ionic_en_error_to_str(ctx->comp.comp.status);
		        ionic_en_err("%s: %s (%d) failed: %s (%d)\n",
                                     vmk_NameToString(&lif->uplink_handle->uplink_name),
                                     name,
	        		     ctx->cmd.cmd.opcode,
		        	     status_str,
			             ctx->comp.comp.status);
                }
        }

	return status;
}


VMK_ReturnStatus
ionic_adminq_post_wait(struct lif *lif, struct ionic_admin_ctx *ctx)
{
	VMK_ReturnStatus status;
        struct ionic_en_priv_data *priv_data;
        vmk_Bool is_timeout;
        vmk_Bool is_retry = VMK_FALSE;

        priv_data = IONIC_CONTAINER_OF(lif->ionic,
                                       struct ionic_en_priv_data,
                                       ionic);

        if (VMK_UNLIKELY(!vmk_AtomicRead64(&priv_data->keep_posting_cmds))) {
                return VMK_FAILURE;
        }

retry:
        status = ionic_api_adminq_post(lif, ctx);
        if (status != VMK_OK) {
                ionic_en_err("%s: ionic_api_adminq_post() failed, "
                             "status: %s",
                             vmk_NameToString(&lif->uplink_handle->uplink_name),
                             vmk_StatusToString(status));
                return status;
        }

        is_timeout = ionic_wait_for_completion_timeout(&ctx->work,
                                                       devcmd_timeout *
                                                       VMK_MSEC_PER_SEC);

        status = ionic_adminq_check_err(lif,
                                        ctx,
                                        is_timeout);
        if (status == VMK_RETRY && is_retry == VMK_FALSE) {
                ionic_en_info("%s: FW is still alive, try one more time",
                              vmk_NameToString(&lif->uplink_handle->uplink_name));
                is_retry = VMK_TRUE;
                goto retry;
        } else if (status != VMK_OK) {
                if (status == VMK_TIMEOUT && is_retry == VMK_TRUE) {
                        vmk_AtomicWrite64(&priv_data->keep_posting_cmds,
                                          VMK_FALSE);
                } else {
                        /* This is not a consecutive timeout failure, we
                         * can set this flag back to true */
                        vmk_AtomicWrite64(&priv_data->keep_posting_cmds,
                                          VMK_TRUE);
                }
                ionic_en_err("%s: ionic_adminq_check_err() failed, "
                             "status: %s",
                             vmk_NameToString(&lif->uplink_handle->uplink_name),
                             vmk_StatusToString(status));
        }

        return status;
}

int ionic_netpoll(int budget, ionic_cq_cb cb, 
                  void *cb_arg)
{
        struct qcq *qcq = (struct qcq *) cb_arg;
        struct cq *cq = &qcq->cq;
        unsigned int work_done;
        vmk_Bool unmask = VMK_FALSE;

        work_done = ionic_cq_service(cq, budget, cb, cb_arg);

        if (work_done < budget) {
                unmask = VMK_TRUE;
        }

        if (work_done || unmask)
                ionic_intr_return_credits(cq->bound_intr,
                                          work_done,
                                          unmask,
                                          VMK_TRUE);

        return work_done;
}


static VMK_ReturnStatus
ionic_dev_cmd_check_error(struct ionic *ionic)
{
        VMK_ReturnStatus status;
        enum ionic_status_code sc;
        struct ionic_dev *idev = &ionic->en_dev.idev;

        sc = ionic_dev_cmd_status(idev);
        switch (sc) {
                case IONIC_RC_SUCCESS:
                case IONIC_RC_EEXIST:
                case IONIC_RC_ENOENT:
                        status = VMK_OK;
                        break;
                default:
                        status = VMK_FAILURE;
        }

        if (status != VMK_OK) {
                do {
                        status = ionic_heartbeat_check(ionic);
                        if (status == VMK_STATUS_PENDING) {
                                vmk_WorldSleep(VMK_MSEC_PER_SEC);
                        } else if (status == VMK_FAILURE) {
                                status = VMK_TIMEOUT;
                                break;
                        } else if (status == VMK_OK) {
                                /* if FW is alive, we can try
                                 * one more time */
                                status = VMK_RETRY;
                                break;
                        }
                } while(status == VMK_STATUS_PENDING);
        }

        return status;
}

static VMK_ReturnStatus
ionic_dev_cmd_wait(struct ionic *ionic, unsigned long max_wait)
{
        VMK_ReturnStatus status;
        struct ionic_dev *idev = &ionic->en_dev.idev;
        unsigned long time;
        int done;


        /* Wait for dev cmd to complete...but no more than max_wait
         */
        time = vmk_GetTimerCycles() + max_wait;
        do {

                done = ionic_dev_cmd_done(idev);
#ifdef IONIC_DEBUG
                if (done) {
                        ionic_en_dbg("DEVCMD %d done took %ld secs (%ld jiffies)\n",
                                     idev->dev_cmd_regs->cmd.cmd.opcode,
                                     (vmk_GetTimerCycles() + max_wait - time) / HZ,
                                     vmk_GetTimerCycles() + max_wait - time);
                }
#endif
                if (done) {
                        return VMK_OK;
                }

                status = vmk_WorldSleep(50);
                if (status != VMK_OK) {
                        ionic_en_err("vmk_WorldSleep() returns: %s",
                                  vmk_StatusToString(status));
                        return status;
                }

        } while (IONIC_TIME_AFTER(time, vmk_GetTimerCycles()));

#ifdef IONIC_DEBUG
        ionic_en_err("DEVCMD timeout after %ld secs\n", max_wait / HZ);
#endif
        return VMK_TIMEOUT;
}


VMK_ReturnStatus
ionic_dev_cmd_wait_check(struct ionic *ionic, unsigned long max_wait)
{
        VMK_ReturnStatus status;
        struct ionic_en_priv_data *priv_data;
        vmk_Bool is_retry = VMK_FALSE;

        priv_data = IONIC_CONTAINER_OF(ionic,
                                       struct ionic_en_priv_data,
                                       ionic);

        if (VMK_UNLIKELY(!vmk_AtomicRead64(&priv_data->keep_posting_cmds))) {
                return VMK_FAILURE;
        }

retry:
        status = ionic_dev_cmd_wait(ionic, max_wait);
        if (status != VMK_OK) {
                ionic_en_err("ionic_dev_cmd_wait() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        status = ionic_dev_cmd_check_error(ionic);
        if (status == VMK_RETRY && is_retry == VMK_FALSE) {
                ionic_en_info("FW is still alive, try one more time");
                is_retry = VMK_TRUE;
                goto retry;
        } else if(status != VMK_OK) {
                if (status == VMK_TIMEOUT && is_retry == VMK_TRUE) {
                        vmk_AtomicWrite64(&priv_data->keep_posting_cmds,
                                          VMK_FALSE);
                } else {
                        /* This is not a consecutive timeout failure, we
                         * can set this flag back to true */
                        vmk_AtomicWrite64(&priv_data->keep_posting_cmds,
                                          VMK_TRUE);
                }
                ionic_en_err("ionic_dev_cmd_check_err() failed, "
                             "status: %s", vmk_StatusToString(status));
        }

        return status;
}


static VMK_ReturnStatus
ionic_setup(struct ionic *ionic)
{
        VMK_ReturnStatus status;

	ionic_en_dbg("ionic_setup() called");

        status = ionic_mutex_create("ionic_dev_cmd_lock",
                                    ionic_driver.module_id,
                                    ionic_driver.heap_id,
                                    VMK_LOCKDOMAIN_INVALID,
                                    VMK_MUTEX,
                                    VMK_MUTEX_UNRANKED,
                                    &ionic->dev_cmd_lock);
        if (status != VMK_OK) {
                ionic_en_err("ionic_mutex_create() faild, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        status = ionic_dev_setup(ionic);
        if (status != VMK_OK) {
		ionic_en_err("ionic_dev_setup() failed, status: %s",
			  vmk_StatusToString(status));
                goto dev_setup_err;
        }

        status = ionic_spinlock_create("ionic->lifs_lock",
                                       ionic_driver.module_id,
                                       ionic_driver.heap_id,
                                       ionic_driver.lock_domain,
                                       VMK_SPINLOCK,
                                       IONIC_LOCK_RANK_NORMAL,
                                       &ionic->lifs_lock);
        if (status != VMK_OK) {
                ionic_en_err("ionic_spinlock_create() failed, status: %s",
                          vmk_StatusToString(status));
                goto lifs_lock_err;
        }                

        return status;

lifs_lock_err:
        ionic_dev_clean(ionic);

dev_setup_err:
        ionic_mutex_destroy(ionic->dev_cmd_lock);

        return status;
}


static inline void
ionic_clean(struct ionic *ionic)
{
        ionic_spinlock_destroy(ionic->lifs_lock);
        ionic_mutex_destroy(ionic->dev_cmd_lock);
        ionic_dev_clean(ionic);
}

VMK_ReturnStatus
ionic_init(struct ionic *ionic)
{
        struct ionic_dev *idev = &ionic->en_dev.idev;

	ionic_en_dbg("ionic_init() called");

        ionic_dev_cmd_init(idev);

        return ionic_dev_cmd_wait_check(ionic,
                                        HZ * devcmd_timeout);
}

VMK_ReturnStatus
ionic_reset(struct ionic *ionic)
{
        VMK_ReturnStatus status;
        struct ionic_dev *idev = &ionic->en_dev.idev;

	ionic_en_dbg("ionic_reset() called");

        vmk_MutexLock(ionic->dev_cmd_lock);
        ionic_dev_cmd_reset(idev);

        status = ionic_dev_cmd_wait_check(ionic,
                                          HZ * devcmd_timeout);
        vmk_MutexUnlock(ionic->dev_cmd_lock);
        if (status != VMK_OK) {
                ionic_en_err("ionic_dev_cmd_wait_check() failed, status: %s",
                          vmk_StatusToString(status));
        }

        return status;
}


VMK_ReturnStatus
ionic_identify(struct ionic *ionic)
{
	VMK_ReturnStatus status;
	struct ionic_dev *idev = &ionic->en_dev.idev;
	struct ionic_identity *ident = &ionic->ident;
	vmk_SystemVersionInfo sys_info;
	unsigned int i;
	unsigned int nwords;

	ionic_en_dbg("ionic_identify() called");

	status = vmk_SystemGetVersionInfo(&sys_info);
	VMK_ASSERT(status == VMK_OK);

	ident->drv.os_type = IONIC_OS_TYPE_ESXI;

	vmk_Strncpy(ident->drv.os_dist_str,
                    sys_info.buildVersion,
                    sizeof(ident->drv.os_dist_str) - 1);
	vmk_Strncpy(ident->drv.kernel_ver_str,
                    sys_info.productVersion,
                    sizeof(ident->drv.kernel_ver_str) - 1);
	vmk_Strncpy(ident->drv.driver_ver_str, DRV_VERSION,
                    sizeof(ident->drv.driver_ver_str) - 1);

	vmk_MutexLock(ionic->dev_cmd_lock);

	nwords = IONIC_MIN(ARRAY_SIZE(ident->drv.words),
                           ARRAY_SIZE(idev->dev_cmd_regs->data));
	for (i = 0; i < nwords; i++)
		ionic_writel_raw(ident->drv.words[i],
                                 (vmk_VA)&idev->dev_cmd_regs->data[i]);

	ionic_dev_cmd_identify(idev, IONIC_IDENTITY_VERSION_1);
	status = ionic_dev_cmd_wait_check(ionic, HZ * devcmd_timeout);
	if (status == VMK_OK) {
		nwords = IONIC_MIN(ARRAY_SIZE(ident->dev.words),
                                   ARRAY_SIZE(idev->dev_cmd_regs->data));
		for (i = 0; i < nwords; i++)
			ident->dev.words[i] = ionic_readl_raw((vmk_VA)&idev->dev_cmd_regs->data[i]);
	}

	vmk_MutexUnlock(ionic->dev_cmd_lock);

	return status;
}

VMK_ReturnStatus
ionic_port_identify(struct ionic *ionic)
{
        VMK_ReturnStatus status;
        struct ionic_dev *idev = &ionic->en_dev.idev;
	struct ionic_identity *ident = &ionic->ident;
	unsigned int i;
	unsigned int nwords;

	vmk_MutexLock(ionic->dev_cmd_lock);

	ionic_dev_cmd_port_identify(idev);
	status = ionic_dev_cmd_wait_check(ionic, HZ * devcmd_timeout);
	if (status == VMK_OK) {
		nwords = IONIC_MIN(ARRAY_SIZE(ident->port.words),
                                   ARRAY_SIZE(idev->dev_cmd_regs->data));
		for (i = 0; i < nwords; i++)
			ident->port.words[i] =
                                ionic_readl_raw((vmk_VA)&idev->dev_cmd_regs->data[i]);
	}

	vmk_MutexUnlock(ionic->dev_cmd_lock);

	return status;
}

VMK_ReturnStatus
ionic_port_init(struct ionic *ionic)
{
	struct ionic_en_priv_data *priv_data;
	struct ionic_dev *idev = &ionic->en_dev.idev;
	struct ionic_identity *ident = &ionic->ident;
	int err;
	unsigned int i;
	unsigned int nwords;

	if (idev->port_info)
		return VMK_OK;

	priv_data = IONIC_CONTAINER_OF(ionic, struct ionic_en_priv_data, ionic);

	idev->port_info_sz = IONIC_ALIGN(sizeof(*idev->port_info), VMK_PAGE_SIZE);
	idev->port_info = ionic_dma_zalloc_align(ionic_driver.heap_id,
                                                 priv_data->dma_engine_coherent,
                                                 idev->port_info_sz,
                                                 VMK_PAGE_SIZE,
                                                 &idev->port_info_pa);
	if (VMK_UNLIKELY(idev->port_info == NULL)) {
                ionic_en_err("ionic_dma_alloc_align() failed, status: NO MEMORY");
                return VMK_NO_MEMORY;
	}

	vmk_MutexLock(ionic->dev_cmd_lock);

	nwords = IONIC_MIN(ARRAY_SIZE(ident->port.config.words),
                           ARRAY_SIZE(idev->dev_cmd_regs->data));
	for (i = 0; i < nwords; i++)
		ionic_writel_raw(ident->port.config.words[i],
                                 (vmk_VA)&idev->dev_cmd_regs->data[i]);

	ionic_dev_cmd_port_init(idev);
	err = ionic_dev_cmd_wait_check(ionic, HZ * devcmd_timeout);

	vmk_MutexUnlock(ionic->dev_cmd_lock);

	return err;
}

VMK_ReturnStatus
ionic_port_reset(struct ionic *ionic)
{
	struct ionic_en_priv_data *priv_data;
	struct ionic_dev *idev = &ionic->en_dev.idev;
	int err;

	if (idev->port_info == NULL)
		return VMK_OK;

	vmk_MutexLock(ionic->dev_cmd_lock);
	ionic_dev_cmd_port_reset(idev);
	err = ionic_dev_cmd_wait_check(ionic, HZ * devcmd_timeout);
	vmk_MutexUnlock(ionic->dev_cmd_lock);
	if (err) {
		ionic_en_err("ionic_port_reset() failed\n");
	}

	priv_data = IONIC_CONTAINER_OF(ionic,
                                       struct ionic_en_priv_data,
                                       ionic);

        if (idev->port_info) {
                ionic_dma_free(ionic_driver.heap_id,
                               priv_data->dma_engine_coherent,
                               idev->port_info_sz,
                               idev->port_info,
                               idev->port_info_pa);
                idev->port_info_pa = 0;
                idev->port_info = NULL;
        }

	return VMK_OK;
}


/*
 *****************************************************************************
 *
 * ionic_en_get_sbdf
 *
 *     Get PCI device address 
 *
 *  Parameters:
 *     data       - IN (pointer to ionic_en_priv_data)
 *
 *  Results:
 *     vmk_PCIDeviceAddr accosiated with priv_data
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

static
vmk_PCIDeviceAddr ionic_en_get_sbdf(vmk_AddrCookie data)
{
        struct ionic_en_priv_data *priv_data = (struct ionic_en_priv_data *) data.ptr;

        VMK_ASSERT(priv_data);

        return priv_data->ionic.en_dev.sbdf;
}


/*
 *****************************************************************************
 *
 * ionic_en_dev_resource_alloc
 *
 *     Initialize all resources that required for assocated ethernet device
 *
 *  Parameters:
 *     priv_data       - IN/OUT (Device private data)
 *
 *  Results:
 *     Return VMK_OK on success
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

static VMK_ReturnStatus
ionic_en_dev_resource_alloc(struct ionic_en_priv_data *priv_data) // IN/OUT
{
        VMK_ReturnStatus status;
        struct ionic *ionic = &priv_data->ionic;

        status = ionic_en_uplink_init(priv_data);
        if (status != VMK_OK) {
                ionic_en_err("ionic_en_uplink_init() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        status = ionic_lifs_alloc(ionic);
        if (status != VMK_OK) {
                ionic_en_err("ionic_lifs_alloc() failed, status: %s",
                          vmk_StatusToString(status));
                goto lifs_alloc_err;
        }

        status = ionic_lifs_init(ionic);
        if (status != VMK_OK) {
                ionic_en_err("ionic_lifs_init() failed, status: %s",
                          vmk_StatusToString(status));
                goto lifs_init_err;
        }

        status = ionic_en_uplink_supported_mode_init(&priv_data->uplink_handle);
        if (status != VMK_OK) {
                ionic_en_err("ionic_en_uplink_supported_mode_init() failed,"
                          " status: %s", vmk_StatusToString(status));
                goto sup_mode_err;
        }

        ionic_en_uplink_default_coal_params_set(priv_data);

        return status;

sup_mode_err:
        ionic_lifs_deinit(ionic);

lifs_init_err:
        ionic_lifs_free(ionic);

lifs_alloc_err:
        ionic_en_uplink_cleanup(priv_data); 

        return status;
}


/*
 *****************************************************************************
 *
 * ionic_en_dev_resource_free
 *
 *     Free all resources that required for assocated ethernet device
 *
 *  Parameters:
 *     priv_data       - IN/OUT (Device private data)
 *
 *  Results:
 *     Return VMK_OK on success
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

static void
ionic_en_dev_resource_free(struct ionic_en_priv_data *priv_data)  // IN/OUT
{
        ionic_lifs_deinit(&priv_data->ionic);
        ionic_lifs_free(&priv_data->ionic);
        ionic_en_uplink_cleanup(priv_data);
} 


/*
 ******************************************************************************
 *
 * ionic_en_attach --
 *
 *     Driver callback that attaches ionic device to the driver
 *
 *  Parameters:
 *     device - IN (vmk_Device handle that generated by vmkernel device layer)
 *
 *  Results:
 *     Return VMK_OK on success
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

static VMK_ReturnStatus
ionic_en_attach(vmk_Device device)                                // IN
{
        VMK_ReturnStatus status;
        struct ionic_en_priv_data *priv_data;

	ionic_en_dbg("ionic_en_attach() called");

        priv_data = ionic_heap_zalign(ionic_driver.heap_id,
                                      sizeof(struct ionic_en_priv_data),
                                      VMK_L1_CACHELINE_SIZE);
        if (VMK_UNLIKELY(!priv_data)) {
                ionic_en_err("ionic_heap_zalign() failed, status: VMK_NO_MEMORY");
                return VMK_NO_MEMORY;
        }

        priv_data->ionic.en_dev.vmk_device     = device;
        priv_data->module_id                   = ionic_driver.module_id;
        priv_data->heap_id                     = ionic_driver.heap_id;
        priv_data->lock_domain                 = ionic_driver.lock_domain;
        priv_data->mem_pool                    = ionic_driver.mem_pool;

        vmk_AtomicWrite64(&priv_data->keep_posting_cmds,
                          VMK_TRUE);

        status = vmk_DeviceSetAttachedDriverData(device, priv_data);
        if (status != VMK_OK) {
                ionic_en_err("vmk_DeviceSetAttachedDriverData() failed, status: %s",
                          vmk_StatusToString(status));
                goto set_attached_err;
        }

        status = ionic_pci_query(priv_data);
        if (status != VMK_OK) {
                ionic_en_err("ionic_pci_query() failed, status: %s",
                          vmk_StatusToString(status));
                goto set_attached_err;
        }

        status = ionic_pci_start(priv_data);
        if (status != VMK_OK) {
                ionic_en_err("ionic_pci_start() failed, status: %s",
                          vmk_StatusToString(status));
                goto set_attached_err;
        }

        status = ionic_dma_engine_create("dma_engine_streaming",
                                         ionic_driver.module_id,
                                         device,
                                         VMK_FALSE, /* Not coherent */
                                         IONIC_DMA_MAX_SEGMENT,
                                         0,
                                         &priv_data->dma_engine_streaming);
        if (status != VMK_OK) {
                ionic_en_err("ionic_dma_engine_create() failed, status: %s",
                          vmk_StatusToString(status));
                goto dma_streaming_err;
        }

        status = ionic_dma_engine_create("dma_engine_coherent",
                                         ionic_driver.module_id,
                                         device,
                                         VMK_TRUE, /* coherent */
                                         IONIC_DMA_MAX_SEGMENT,
                                         0,
                                         &priv_data->dma_engine_coherent);
        if (status != VMK_OK) {
                ionic_en_err("ionic_dma_engine_create() failed, status: %s",
                          vmk_StatusToString(status));
                goto dma_coherent_err;
        }
        
        status = ionic_setup(&priv_data->ionic);
        if (status != VMK_OK) {
                ionic_en_err("ionic_setup() failed, status: %s",
                          vmk_StatusToString(status));
                goto setup_err;
        }

        status = ionic_init(&priv_data->ionic);
        if (status != VMK_OK) {
                ionic_en_err("ionic_init() failed, status: %s",
                          vmk_StatusToString(status));
                goto init_err;
        }

	status = ionic_identify(&priv_data->ionic);
        if (status != VMK_OK) {
                ionic_en_err("ionic_identify() failed, status: %s",
                          vmk_StatusToString(status));
                goto init_err;
        }

 	/* Configure the ports */
	status = ionic_port_identify(&priv_data->ionic);
	if (status != VMK_OK) {
		ionic_en_err("Cannot identify port: %s, aborting",
                        vmk_StatusToString(status));
                goto init_err;
        }

	status = ionic_port_init(&priv_data->ionic);
	if (status != VMK_OK) {
		ionic_en_err("Cannot init port: %s, aborting",
                        vmk_StatusToString(status));
	        goto init_err;
	}

        /* Configure LIFs */
	status = ionic_lif_identify(&priv_data->ionic);
	if (status != VMK_OK) {
		ionic_en_err("Cannot identify LIFs: %s, aborting",
                        vmk_StatusToString(status));
                goto lif_ident_err;
        }

        status = ionic_lifs_size(&priv_data->ionic);
	if (status != VMK_OK) {
		ionic_en_err("ionic_lifs_size() failed, status: %s",
			  vmk_StatusToString(status));
                goto lif_ident_err;
        }

        return status;

lif_ident_err:
        ionic_port_reset(&priv_data->ionic);

init_err:
        ionic_clean(&priv_data->ionic);

setup_err:
        ionic_dma_engine_destroy(priv_data->dma_engine_coherent);

dma_coherent_err:
        ionic_dma_engine_destroy(priv_data->dma_engine_streaming);

dma_streaming_err:
        ionic_pci_stop(priv_data);

set_attached_err:
        ionic_heap_free(ionic_driver.heap_id, priv_data);

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_en_scan --
 *
 *     Driver callback that scan ionic device and create logical devices
 *
 *  Parameters:
 *     device - IN (vmk_Device handle)
 *
 *  Results:
 *     Return VMK_OK on success
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

static VMK_ReturnStatus
ionic_en_scan(vmk_Device device)                                  // IN
{
        VMK_ReturnStatus status;
        struct ionic_en_priv_data *priv_data;

	ionic_en_dbg("ionic_en_scan() called");

        status = vmk_DeviceGetAttachedDriverData(device, 
                                                 (vmk_AddrCookie *) &priv_data);
        if (status != VMK_OK) {
                ionic_en_err("vmk_DeviceGetAttachedDriverData() failed, "
                          "status: %s", vmk_StatusToString(status));
                return status;
        }

        status = ionic_logical_dev_register(ionic_driver.drv_handle,
                                            device,
                                            priv_data,
                                            &priv_data->uplink_handle.uplink_reg_data,
                                            &priv_data->ionic.en_dev.uplink_vmk_dev);
        if (status != VMK_OK) {
                ionic_en_err("ionic_logical_dev_register() failed, status: %s",
                          vmk_StatusToString(status));
        }

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_en_detach --
 *
 *     Driver callback that detach ionic device from the driver
 *
 *  Parameters:
 *     device - IN (vmk_Device handle)
 *
 *  Results:
 *     Return VMK_OK on success
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

static VMK_ReturnStatus
ionic_en_detach(vmk_Device device)                                // IN
{
	VMK_ReturnStatus status;
        struct ionic_en_priv_data *priv_data;

	ionic_en_dbg("ionic_en_detach() called");

	status = vmk_DeviceGetAttachedDriverData(device,
						 (vmk_AddrCookie *) &priv_data);
	if (status != VMK_OK) {
		ionic_en_err("vmk_DeviceGetAttachedDriverData() failed, status: %s",
			  vmk_StatusToString(status));
                return status;
	}

        if (priv_data->is_lifs_size_compl) {
                ionic_lifs_size_undo(priv_data);
        }
 
        ionic_port_reset(&priv_data->ionic);

        ionic_reset(&priv_data->ionic);

        ionic_dma_engine_destroy(priv_data->dma_engine_streaming);
        ionic_dma_engine_destroy(priv_data->dma_engine_coherent);

	ionic_clean(&priv_data->ionic);

	ionic_pci_stop(priv_data);
       
        ionic_heap_free(ionic_driver.heap_id, priv_data);

	return status;
}


/*
 ******************************************************************************
 *
 * ionic_en_quiesce --
 *
 *     Driver callback that quiesces ionic device
 *
 *  Parameters:
 *     device - IN (vmk_Device handle)
 *
 *  Results:
 *     Return VMK_OK on success
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

static VMK_ReturnStatus
ionic_en_quiesce(vmk_Device device)                               // IN
{
        VMK_ReturnStatus status;
        struct ionic_en_priv_data *priv_data;

        ionic_en_dbg("ionic_en_quiesce() called");

        status = vmk_DeviceGetAttachedDriverData(device, 
                                                 (vmk_AddrCookie *) &priv_data);
        if (status != VMK_OK) {
                ionic_en_err("vmk_DeviceGetAttachedDriverData() failed, "
                             "status: %s", vmk_StatusToString(status));
                return status;
        }

        ionic_en_dev_resource_free(priv_data);
 
        return status;
}


/*
 ******************************************************************************
 *
 * ionic_en_start --
 *
 *     Driver callback that starts ionic device
 *
 *  Parameters:
 *     device - IN (vmk_Device handle)
 *
 *  Results:
 *     Return VMK_OK on success
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

static VMK_ReturnStatus
ionic_en_start(vmk_Device device)                                 // IN
{
        VMK_ReturnStatus status;
        struct ionic_en_priv_data *priv_data;

        ionic_en_dbg("ionic_en_start() called");

        status = vmk_DeviceGetAttachedDriverData(device, 
                                                 (vmk_AddrCookie *) &priv_data);
        if (status != VMK_OK) {
                ionic_en_err("vmk_DeviceGetAttachedDriverData() failed, "
                             "status: %s", vmk_StatusToString(status));
                return status;
        }

        status = ionic_en_dev_resource_alloc(priv_data);
        if (status != VMK_OK) {
                ionic_en_err("ionic_en_dev_resource_alloc() failed, status: %s",
                             vmk_StatusToString(status));
        }

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_en_forget --
 *
 *     Driver callback that forgets ionic device
 *
 *  Parameters:
 *     device - IN (vmk_Device handle)
 *
 *  Results:
 *     Return VMK_OK on success
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

static void
ionic_en_forget(vmk_Device device)                                // IN
{
	ionic_en_dbg("ionic_en_forget() called");
}


/* Driver callbacks that required by device layer */
static vmk_DriverOps ionic_driver_ops = {
        .attachDevice 	= ionic_en_attach,
        .scanDevice   	= ionic_en_scan,
	.detachDevice   = ionic_en_detach,
	.quiesceDevice  = ionic_en_quiesce,
	.startDevice	= ionic_en_start,
	.forgetDevice	= ionic_en_forget,
};


/*
 ******************************************************************************
 *
 * init_module --
 *
 *     Driver starting point
 *
 *  Parameters:
 *     None
 *
 *  Results:
 *     Initialize resources for this driver
 *     Return 0 when completes successfully
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

int
init_module(void)
{
        VMK_ReturnStatus status;
        vmk_DriverProps drv_props;
        vmk_LogProperties log_props;

        vmk_LogMessage("%s: init module...", IONIC_DRV_NAME);

        status = vmk_NameInitialize(&ionic_driver.name,
                                    IONIC_DRV_NAME);
        if (VMK_UNLIKELY(status != VMK_OK)) {
                vmk_AlertMessage("%s, vmk_NameInitialize() failed, "
                                 "status: %s", IONIC_DRV_NAME,
                                 vmk_StatusToString(status));
                return status;
        }

        ionic_driver.module_id = vmk_ModuleCurrentID;

        status = ionic_validate_module_params();
        if (status != VMK_OK) {
                vmk_AlertMessage("ionic_validate_module_params() failed, "
                                 "please check your module parameters.");
                return status;
        }

        /* Create a memory pool */
        status = ionic_mem_pool_init(IONIC_DRV_NAME,
                                     ionic_driver.module_id,
                                     VMK_MEMPOOL_INVALID,
                                     VMK_MEM_POOL_LEAF,
                                     MEMPOOL_INIT_SIZE,
                                     MEMPOOL_MAX_SIZE,
                                     &ionic_driver.mem_pool);
        if (status != VMK_OK) {
                vmk_AlertMessage("%s: ionic_mem_pool_init failed, "
                                 "status: %s", IONIC_DRV_NAME,
                                 vmk_StatusToString(status));
                return status;
        }

        /* Create heap that back the created memory pool */
        status = ionic_heap_init(IONIC_DRV_NAME,
                                 ionic_driver.module_id,
                                 ionic_driver.mem_pool,
                                 VMK_HEAP_TYPE_MEMPOOL,
                                 HEAP_INIT_SIZE,
                                 HEAP_MAX_SIZE,
                                 VMK_MEM_PHYS_CONTIGUOUS,
                                 &ionic_driver.heap_id);
        if (status != VMK_OK) {
                vmk_AlertMessage("%s: ionic_HeapInit() failed, "
                                 "status: %s", IONIC_DRV_NAME,
                                 vmk_StatusToString(status));
                goto heap_init_err;
        }

        vmk_ModuleSetHeapID(ionic_driver.module_id, ionic_driver.heap_id);

        vmk_Memset(&log_props, 0, sizeof(vmk_LogProperties));
        log_props.defaultLevel  = log_level;
        log_props.heap          = ionic_driver.heap_id;
        log_props.module        = ionic_driver.module_id;
        status = vmk_NameInitialize(&log_props.name, IONIC_DRV_NAME);
        VMK_ASSERT(status == VMK_OK);

        status = vmk_LogRegister(&log_props,
                                 &ionic_driver.log_component);
        if (status != VMK_OK) {
                vmk_AlertMessage("%s: vmk_LogRegister failed: %s",
                                 IONIC_DRV_NAME, vmk_StatusToString(status));
                goto log_reg_err;
        }

        /* Create a lock domain */
        status = vmk_LockDomainCreate(ionic_driver.module_id,
                                      ionic_driver.heap_id,
                                      &ionic_driver.name,
                                      &ionic_driver.lock_domain);
        if (status != VMK_OK) {
                ionic_en_err("%s: vmk_LockDomainCreate() failed, status: %s",
                          IONIC_DRV_NAME, vmk_StatusToString(status));
                goto lock_domain_err;
        }

        /* Initialize driver properties that required by device layer */
        vmk_Memset(&drv_props, 0, sizeof(vmk_DriverProps));

        status = vmk_NameInitialize(&drv_props.name, IONIC_DRV_NAME);
        if (VMK_UNLIKELY(status != VMK_OK)) {
                ionic_en_err("%s: vmk_NameInitialize() failed, status: %s",
                          IONIC_DRV_NAME, vmk_StatusToString(status));
                goto name_init_err;
        }

        drv_props.moduleID = ionic_driver.module_id;
        drv_props.ops      = &ionic_driver_ops;

        status = vmk_DriverRegister(&drv_props, &ionic_driver.drv_handle);
        if (status != VMK_OK) {
                ionic_en_err("%s: vmk_DriverRegister() failed, status: %s",
                          IONIC_DRV_NAME, vmk_StatusToString(status));
                goto name_init_err;
        }

        status = ionic_device_list_init(ionic_driver.module_id,
                                        ionic_driver.heap_id,
                                        ionic_en_get_sbdf,
                                        &ionic_driver.uplink_dev_list);
        if (status != VMK_OK) {
                ionic_en_err("ionic_device_list_init() failed, status: %s",
                          vmk_StatusToString(status));
                goto dev_list_init_err;
        }

        return status;

dev_list_init_err:
        vmk_DriverUnregister(ionic_driver.drv_handle);

name_init_err:
        vmk_LockDomainDestroy(ionic_driver.lock_domain);

lock_domain_err:
        vmk_LogUnregister(ionic_driver.log_component);

log_reg_err:
        ionic_heap_destroy(ionic_driver.heap_id);

heap_init_err:
        ionic_mem_pool_destroy(ionic_driver.mem_pool);

        return status;
}


/*
 ******************************************************************************
 *
 * cleanup_module --
 *
 *     Driver exit point
 *
 *  Parameters:
 *     None
 *
 *  Results:
 *     Destroy all driver data structures
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

void
cleanup_module()
{
        ionic_device_list_destroy(&ionic_driver.uplink_dev_list);
	vmk_DriverUnregister(ionic_driver.drv_handle);
	vmk_LockDomainDestroy(ionic_driver.lock_domain);	
        vmk_LogUnregister(ionic_driver.log_component);
        ionic_heap_destroy(ionic_driver.heap_id);
	ionic_mem_pool_destroy(ionic_driver.mem_pool);
}
