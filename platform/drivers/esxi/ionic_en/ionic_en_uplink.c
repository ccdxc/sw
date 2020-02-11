/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 */

/*
 * ionic_en_uplink.c --
 *
 * Implement uplink layer interface
 */

#include "ionic.h"

vmk_UplinkOps ionic_en_uplink_ops = {
        .uplinkTx              = ionic_en_uplink_tx,
        .uplinkMTUSet          = ionic_en_uplink_mtu_set,
        .uplinkStateSet        = ionic_en_uplink_state_set,
        .uplinkStatsGet        = ionic_en_uplink_stats_get,
        .uplinkAssociate       = ionic_en_uplink_associate,
        .uplinkDisassociate    = ionic_en_uplink_disassociate,
        .uplinkCapEnable       = ionic_en_uplink_cap_enable,
        .uplinkCapDisable      = ionic_en_uplink_cap_disable,
        .uplinkStartIO         = ionic_en_uplink_start_io,
        .uplinkQuiesceIO       = ionic_en_uplink_quiesce_io,
        .uplinkReset           = ionic_en_uplink_reset,
};

#define IONIC_EN_NUM_SUPPORTED_MODES_MNIC   1
#define IONIC_EN_NUM_SUPPORTED_MODES_100G   4
#define IONIC_EN_NUM_SUPPORTED_MODES_25G    3
#define IONIC_EN_NUM_SUPPORTED_MODES_10G    2

vmk_UplinkSupportedMode ionic_en_uplink_modes_mnic[IONIC_EN_NUM_SUPPORTED_MODES_MNIC] = {
        {VMK_LINK_SPEED_1000_MBPS, VMK_LINK_DUPLEX_FULL},
};

vmk_UplinkSupportedMode ionic_en_uplink_modes_100g[IONIC_EN_NUM_SUPPORTED_MODES_100G] = {
        {VMK_LINK_SPEED_AUTO,       VMK_LINK_DUPLEX_AUTO},
        {VMK_LINK_SPEED_10000_MBPS, VMK_LINK_DUPLEX_FULL},
        {VMK_LINK_SPEED_25000_MBPS, VMK_LINK_DUPLEX_FULL},
        {VMK_LINK_SPEED_100000_MBPS, VMK_LINK_DUPLEX_FULL}
};

vmk_UplinkSupportedMode ionic_en_uplink_modes_25g[IONIC_EN_NUM_SUPPORTED_MODES_25G] = {
        {VMK_LINK_SPEED_AUTO,       VMK_LINK_DUPLEX_AUTO},
        {VMK_LINK_SPEED_10000_MBPS, VMK_LINK_DUPLEX_FULL},
        {VMK_LINK_SPEED_25000_MBPS, VMK_LINK_DUPLEX_FULL},
};

vmk_UplinkSupportedMode ionic_en_uplink_modes_10g[IONIC_EN_NUM_SUPPORTED_MODES_10G] = {
        {VMK_LINK_SPEED_AUTO,       VMK_LINK_DUPLEX_AUTO},
        {VMK_LINK_SPEED_10000_MBPS, VMK_LINK_DUPLEX_FULL},
};

#define IONIC_VLAN_FILTER_SIZE          4096

typedef VMK_ReturnStatus
(*ionic_en_vlan_filter_add_kill) (struct lif *lif,
                                  u16 vid);

typedef VMK_ReturnStatus
(*ionic_en_vlan_bitmap_set_clr) (vmk_VLANBitmap *bitmap,
                                 vmk_VlanID vlanID);
/*
static vmk_UplinkVLANFilterOps ionic_en_vlan_filter_ops = {
        .enableVLANFilter       = ionic_en_vlan_filter_enable,
        .disableVLANFilter      = ionic_en_vlan_filter_disable,
        .getVLANFilterBitmap    = ionic_en_vlan_filter_bitmap_get,
        .setVLANFilterBitmap    = ionic_en_vlan_filter_bitmap_set,
        .addVLANFilter          = ionic_en_vlan_filter_add,
        .removeVLANFilter       = ionic_en_vlan_filter_remove,
};
*/

static vmk_UplinkMultiQueueOps ionic_en_multi_queue_ops = {
   .queueOps = {
      .queueAlloc             = ionic_en_queue_alloc,
      .queueAllocWithAttr     = ionic_en_queue_alloc_with_attr,
      .queueReallocWithAttr   = ionic_en_queue_realloc_with_attr,
      .queueFree              = ionic_en_queue_free,
      .queueQuiesce           = ionic_en_queue_quiesce,
      .queueStart             = ionic_en_queue_start,
      .queueApplyFilter       = ionic_en_queue_apply_filter,
      .queueRemoveFilter      = ionic_en_queue_remove_filter,
      .queueGetStats          = ionic_en_queue_get_stats,
      .queueToggleFeature     = ionic_en_queue_toggle_feature,
      .queueSetPriority       = ionic_en_queue_set_priority,
      .queueSetCoalesceParams = ionic_en_queue_set_coalesce_params,
   },
   .queueSetCount = NULL,
};

static vmk_UplinkCoalesceParamsOps ionic_en_uplink_coal_params_ops = {
        .getParams            = ionic_en_uplink_coal_params_get,
        .setParams            = ionic_en_uplink_coal_params_set,
};


static VMK_ReturnStatus
ionic_en_priv_stats_len_get(vmk_AddrCookie driver_data,
                            vmk_ByteCount *length)
{
        ionic_en_dbg("ionic_en_priv_stats_len_get() called");
        *length = IONIC_EN_MAX_BUF_SIZE_PRIV_STATS;

        return VMK_OK;
}

static VMK_ReturnStatus
ionic_en_priv_stats_get(vmk_AddrCookie driver_data,
                        char *stat_buf,
                        vmk_ByteCount length)
{
        vmk_uint32 i;
        struct rx_stats *rx_stats = NULL;
        struct tx_stats *tx_stats = NULL;
        struct lif *lif;
        struct ionic_en_priv_data *priv_data =
                (struct ionic_en_priv_data *) driver_data.ptr;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;

        ionic_en_dbg("ionic_en_priv_stats_get() called");

        lif = VMK_LIST_ENTRY(vmk_ListFirst(&priv_data->ionic.lifs),
                             struct lif, list);
        vmk_Memset(stat_buf, 0, length);

        vmk_SemaLock(&uplink_handle->stats_binary_sema);

        for (i = 0; i < lif->nrxqcqs; i++) {
                rx_stats = &lif->rxqcqs[i]->stats.rx;
                stat_buf += vmk_Sprintf((char *) stat_buf,
                                        "\n    %s[%d]: packets=%lu, bytes=%lu,"
                                        " alloc_err=%lu, csum_err=%lu,"
                                        " csum_complete=%lu, no_csum=%lu",
                                        "rx_queue", i, rx_stats->pkts,
                                        rx_stats->bytes, rx_stats->alloc_err,
                                        rx_stats->csum_err,
                                        rx_stats->csum_complete,
                                        rx_stats->no_csum);
        }

        for (i = 0; i < lif->ntxqcqs; i++) {
                tx_stats = &lif->txqcqs[i]->stats.tx;
                stat_buf += vmk_Sprintf((char *) stat_buf,
                                        "\n    %s[%d]: packets=%lu, bytes=%lu,"
                                        " tso=%lu, csum=%lu, no_csum=%lu,"
                                        " linearize=%lu, frags=%lu, wake=%lu,"
                                        " stop=%lu, clean=%lu, busy=%lu",
                                        "tx_queue", i, tx_stats->pkts,
                                        tx_stats->bytes, tx_stats->tso,
                                        tx_stats->csum, tx_stats->no_csum,
                                        tx_stats->linearize, tx_stats->frags,
                                        tx_stats->wake, tx_stats->stop,
                                        tx_stats->clean, tx_stats->busy);
        }

        stat_buf += vmk_Sprintf((char *) stat_buf,
                                "\n Number of RSS queues=%d, "
                                "ntxq_descs=%d, nrxq_descs=%d, "
                                "log_level=%d, vlan_tx_insert=%d, "
                                "vlan_rx_strip=%d",
                                uplink_handle->DRSS, ntxq_descs, nrxq_descs,
                                log_level, vlan_tx_insert,
                                vlan_rx_strip);

        vmk_SemaUnlock(&uplink_handle->stats_binary_sema);

        return VMK_OK;
}

static vmk_UplinkPrivStatsOps ionic_en_priv_stats = {
        .privStatsLengthGet   = ionic_en_priv_stats_len_get,
        .privStatsGet         = ionic_en_priv_stats_get,
};

static VMK_ReturnStatus
ionic_en_queue_get_rss_params(vmk_AddrCookie driver_data,
                              vmk_UplinkQueueRSSParams *rss_params)
{
        struct ionic_en_priv_data *priv_data =
                (struct ionic_en_priv_data *) driver_data.ptr;
        struct lif *lif;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;

        ionic_en_dbg("ionic_en_queue_get_rss_params() called");

        lif = VMK_LIST_ENTRY(vmk_ListFirst(&priv_data->ionic.lifs),
                             struct lif, list);

        rss_params->numRSSPools = 1;
        rss_params->numRSSQueuesPerPool = uplink_handle->DRSS;

        rss_params->rssHashKeySize = IONIC_RSS_HASH_KEY_SIZE;
        rss_params->rssIndTableSize = lif->ionic->ident.lif.eth.rss_ind_tbl_sz;

        return VMK_OK;
}

static VMK_ReturnStatus
ionic_en_queue_init_rss_state(vmk_AddrCookie driver_data,
                              vmk_UplinkQueueRSSHashKey *rss_hash_key,
                              vmk_UplinkQueueRSSIndTable *rss_ind_tbl)
{
        VMK_ReturnStatus status = VMK_OK;
        struct lif *lif;
        struct ionic_en_priv_data *priv_data =
                (struct ionic_en_priv_data *) driver_data.ptr;

        ionic_en_dbg("ionic_en_queue_init_rss_state() called");

        lif = VMK_LIST_ENTRY(vmk_ListFirst(&priv_data->ionic.lifs),
                             struct lif, list);

        if (rss_hash_key->keySize > IONIC_RSS_HASH_KEY_SIZE) {
                ionic_en_err("RSS hash key size(%d) is not valid,"
                          " it should be less or equal to (%d).",
                          rss_hash_key->keySize, IONIC_RSS_HASH_KEY_SIZE);
                return VMK_BAD_PARAM;
        }

        if (rss_ind_tbl->tableSize > lif->ionic->ident.lif.eth.rss_ind_tbl_sz) {
                ionic_en_err("RSS indirection table size(%d) is not valid,"
                          " it should be less or equal to (%d).",
                          rss_ind_tbl->tableSize, lif->ionic->ident.lif.eth.rss_ind_tbl_sz);
                return VMK_BAD_PARAM;
        }

        vmk_Memset(lif->rss_ind_tbl,
                   0,
                   lif->ionic->ident.lif.eth.rss_ind_tbl_sz);
        vmk_Memcpy(lif->rss_ind_tbl,
                   rss_ind_tbl->table,
                   rss_ind_tbl->tableSize);
        lif->rss_ind_tbl_size = rss_ind_tbl->tableSize;

        vmk_Memset(lif->rss_hash_key,
                   0,
                   IONIC_RSS_HASH_KEY_SIZE);
        vmk_Memcpy(lif->rss_hash_key,
                   rss_hash_key->key,
                   rss_hash_key->keySize);
        lif->rss_key_size = rss_hash_key->keySize;

	status = ionic_lif_rss_config(lif, lif->rss_types, NULL, NULL);
	if (status != VMK_OK) {
                ionic_en_err("ionic_lif_rss_config() failed, status: %s",
                          vmk_StatusToString(status));
		return status;
        }

        return status;
}


static VMK_ReturnStatus
ionic_en_queue_update_rss_table(vmk_AddrCookie driver_data,
                                vmk_UplinkQueueRSSIndTable *rss_ind_tbl)
{
        VMK_ReturnStatus status;
        struct lif *lif;
        struct ionic_en_priv_data *priv_data =
                 (struct ionic_en_priv_data *) driver_data.ptr;

        ionic_en_dbg("ionic_en_queue_update_rss_table() called");

        lif = VMK_LIST_ENTRY(vmk_ListFirst(&priv_data->ionic.lifs),
                             struct lif, list);

        if (rss_ind_tbl->tableSize > lif->ionic->ident.lif.eth.rss_ind_tbl_sz){
                ionic_en_err("RSS indirection talbe sie(%d) is not valid,"
                          " it should be less or equal to (%d).",
                          rss_ind_tbl->tableSize, lif->ionic->ident.lif.eth.rss_ind_tbl_sz);
                return VMK_BAD_PARAM;
        }

        vmk_Memset(lif->rss_ind_tbl,
                   0,
                   lif->ionic->ident.lif.eth.rss_ind_tbl_sz);
        vmk_Memcpy(lif->rss_ind_tbl,
                   rss_ind_tbl->table,
                   rss_ind_tbl->tableSize);

	status = ionic_lif_rss_config(lif, lif->rss_types, NULL, NULL);
	if (status != VMK_OK) {
                ionic_en_err("ionic_lif_rss_config() failed, status: %s",
                          vmk_StatusToString(status));
		return status;
        }

        return status;
}


static VMK_ReturnStatus
ionic_en_queue_get_rss_table(vmk_AddrCookie driver_data,
                             vmk_UplinkQueueRSSIndTable *rss_ind_tbl)
{
        struct lif *lif;
        struct ionic_en_priv_data *priv_data =
                 (struct ionic_en_priv_data *) driver_data.ptr;

        ionic_en_dbg("ionic_en_queue_get_rss_table() called");

        lif = VMK_LIST_ENTRY(vmk_ListFirst(&priv_data->ionic.lifs),
                             struct lif, list);

        rss_ind_tbl->tableSize = lif->ionic->ident.lif.eth.rss_ind_tbl_sz;
        vmk_Memcpy(rss_ind_tbl->table,
                   lif->rss_ind_tbl,
                   lif->ionic->ident.lif.eth.rss_ind_tbl_sz);

        return VMK_OK;
}


static vmk_UplinkQueueRSSDynOps ionic_en_dyn_rss_ops = {
        .queueGetRSSParams = ionic_en_queue_get_rss_params,
        .queueInitRSSState = ionic_en_queue_init_rss_state,
        .queueUpdateRSSIndTable = ionic_en_queue_update_rss_table,
        .queueGetRSSIndTable = ionic_en_queue_get_rss_table,
};


static VMK_ReturnStatus
ionic_en_uplink_pause_params_get(vmk_AddrCookie driver_data,
                                 vmk_UplinkPauseParams *params)
{
        struct lif *lif;
        struct ionic_en_priv_data *priv_data =
                (struct ionic_en_priv_data *) driver_data.ptr;
        struct ionic *ionic = &priv_data->ionic;
        struct ionic_dev *idev = &ionic->en_dev.idev;

        ionic_en_dbg("ionic_en_uplink_pause_params_get() called");

        lif = VMK_LIST_ENTRY(vmk_ListFirst(&priv_data->ionic.lifs),
                             struct lif, list);

        vmk_Memset(params, 0, sizeof(vmk_UplinkPauseParams));

        params->autoNegotiate = idev->port_info->config.an_enable;

        if (idev->port_info->config.pause_type) {
                params->rxPauseEnabled = VMK_TRUE;
                params->txPauseEnabled = VMK_TRUE;
        }

        return VMK_OK;
}


static VMK_ReturnStatus
ionic_en_uplink_pause_params_set(vmk_AddrCookie driver_data,
                                 vmk_UplinkPauseParams params)
{
        VMK_ReturnStatus status;
        vmk_uint32 requested_pause;
        struct lif *lif;
        struct ionic_en_priv_data *priv_data =
                (struct ionic_en_priv_data *) driver_data.ptr;
        struct ionic *ionic = &priv_data->ionic;
        struct ionic_dev *idev = &ionic->en_dev.idev;

        ionic_en_dbg("ionic_en_uplink_pause_params_set() called");

        lif = VMK_LIST_ENTRY(vmk_ListFirst(&priv_data->ionic.lifs),
                             struct lif, list);

        if (params.rxPauseEnabled && params.txPauseEnabled) {
                requested_pause = PORT_PAUSE_TYPE_LINK;
        } else if (!params.rxPauseEnabled && !params.txPauseEnabled) {
                requested_pause = PORT_PAUSE_TYPE_NONE;
        } else {
                return VMK_FAILURE;
        }

        vmk_MutexLock(ionic->dev_cmd_lock);
        ionic_dev_cmd_port_autoneg(idev, params.autoNegotiate);
        status = ionic_dev_cmd_wait_check(ionic, HZ * devcmd_timeout);
        vmk_MutexUnlock(ionic->dev_cmd_lock);
        if (status != VMK_OK) {
                ionic_en_err("ionic_dev_cmd_port_autoneg() failed, status: %s",
                                vmk_StatusToString(status));
        }

        vmk_MutexLock(ionic->dev_cmd_lock);
        ionic_dev_cmd_port_pause(idev, requested_pause);
        status = ionic_dev_cmd_wait_check(ionic, HZ * devcmd_timeout);
        vmk_MutexUnlock(ionic->dev_cmd_lock);
        if (status != VMK_OK) {
                ionic_en_err("ionic_dev_cmd_port_pause() failed, status: %s",
                                vmk_StatusToString(status));
        }

        return status;
}

static vmk_UplinkPauseParamsOps ionic_en_uplink_pause_ops = {
        .pauseParamsGet       = ionic_en_uplink_pause_params_get,
        .pauseParamsSet       = ionic_en_uplink_pause_params_set,
};


/*
 *****************************************************************************
 *
 * ionic_en_uplink_transceiver_type_get
 *
 *     Get transceiver type
 *
 *  Parameters:
 *     driver_data  - IN (private driver data)
 *     trans_type   - OUT(Pointer to be filled in with the transceiver
 *                        type for this uplink.)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */
VMK_ReturnStatus
ionic_en_uplink_transceiver_type_get(vmk_AddrCookie driver_data,                // IN
                                     vmk_UplinkTransceiverType *trans_type)     // OUT
{
        struct ionic_en_priv_data *priv_data =
                (struct ionic_en_priv_data *) driver_data.ptr;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;

        vmk_SpinlockLockIgnoreDeathPending(uplink_handle->link_status_lock);
        *trans_type = uplink_handle->trans_type;
        vmk_SpinlockUnlock(uplink_handle->link_status_lock);

        return VMK_OK;
}


/*
 *****************************************************************************
 *
 * ionic_en_uplink_transceiver_type_set
 *
 *     Set transceiver type
 *
 *  Parameters:
 *     driver_data   - IN (private driver data)
 *     trans_type    - IN (The transceiver type to set)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */
VMK_ReturnStatus
ionic_en_uplink_transceiver_type_set(vmk_AddrCookie driver_data,                // IN
                                     vmk_UplinkTransceiverType trans_type)      // IN
{
        return VMK_NOT_SUPPORTED;
}


static vmk_UplinkTransceiverTypeOps ionic_en_uplink_transceiver_type_ops = {
        .getTransceiverType  = ionic_en_uplink_transceiver_type_get,
        .setTransceiverType  = ionic_en_uplink_transceiver_type_set,
};


VMK_ReturnStatus
ionic_en_uplink_cable_type_get(vmk_AddrCookie driver_data,
                               vmk_UplinkCableType *cableType)
{
        struct lif *lif;
        vmk_uint8 xcvr_pid;
        struct ionic_en_priv_data *priv_data =
                (struct ionic_en_priv_data *) driver_data.ptr;
        struct ionic *ionic = &priv_data->ionic;
        struct ionic_dev *idev = &ionic->en_dev.idev;

        lif = VMK_LIST_ENTRY(vmk_ListFirst(&priv_data->ionic.lifs),
                             struct lif, list);

        if (priv_data->uplink_handle.is_mgmt_nic) {
                *cableType = VMK_UPLINK_CABLE_TYPE_TP;
                goto out;
        }

        xcvr_pid = idev->port_info->status.xcvr.pid;
        if (xcvr_pid >= XCVR_PID_QSFP_100G_CR4 &&
            xcvr_pid <= XCVR_PID_SFP_25GBASE_CR_N) {
                *cableType = VMK_UPLINK_CABLE_TYPE_DA;
        } else if (xcvr_pid >= XCVR_PID_QSFP_100G_AOC &&
                   xcvr_pid <= XCVR_PID_QSFP_100G_PSM4) {
                *cableType = VMK_UPLINK_CABLE_TYPE_FIBRE;
        } else {
                *cableType = VMK_UPLINK_CABLE_TYPE_OTHER;
        }

out:
        return VMK_OK;
}


VMK_ReturnStatus
ionic_en_uplink_supported_cable_type_get(vmk_AddrCookie driver_data,
                                         vmk_UplinkCableType *cableType)
{
        struct ionic_en_priv_data *priv_data =
                (struct ionic_en_priv_data *) driver_data.ptr;

        if (priv_data->uplink_handle.is_mgmt_nic) {
                *cableType = VMK_UPLINK_CABLE_TYPE_TP;
                goto out;
        }

        *cableType = 0;
        *cableType |= VMK_UPLINK_CABLE_TYPE_FIBRE;
        *cableType |= VMK_UPLINK_CABLE_TYPE_DA;

out:
        return VMK_OK;
}


VMK_ReturnStatus
ionic_en_uplink_cable_type_set(vmk_AddrCookie driver_data,
                               vmk_UplinkCableType cableType)
{
        return VMK_NOT_SUPPORTED;
}


static vmk_UplinkCableTypeOps ionic_en_uplink_cable_type_ops = {
        .getCableType           = ionic_en_uplink_cable_type_get,
        .getSupportedCableTypes = ionic_en_uplink_supported_cable_type_get,
        .setCableType           = ionic_en_uplink_cable_type_set,
};


/*
 *****************************************************************************
 *
 * ionic_en_uplink_link_state_update
 *
 *     Update uplink link status
 *
 *  Parameters:
 *     uplink_handle    - IN (pointer of uplink handle)
 *
 *  Results:
 *     None
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

void
ionic_en_uplink_link_state_update(struct ionic_en_uplink_handle *uplink_handle)  // IN
{
        vmk_LinkStatus link_info;
        vmk_Bool is_update_needed = VMK_FALSE;
        vmk_UplinkSharedData *shared_data = &uplink_handle->uplink_shared_data;

        VMK_ASSERT(uplink_handle->uplink_dev);

        vmk_SpinlockLockIgnoreDeathPending(uplink_handle->link_status_lock);

        /* Skip notifying if not necessary */
        if (uplink_handle->is_ready_notify_linkup == VMK_FALSE) {
                link_info.state = VMK_LINK_STATE_DOWN;
                link_info.speed = 0;
                link_info.duplex = VMK_LINK_DUPLEX_FULL;
        } else {
                vmk_Memcpy(&link_info,
                           &uplink_handle->link_status,
                           sizeof(vmk_LinkStatus));
        }

        vmk_SpinlockUnlock(uplink_handle->link_status_lock);

        /* If admin link is down, we don't need to report link up */
        vmk_SpinlockLockIgnoreDeathPending(uplink_handle->admin_status_lock);
        if (uplink_handle->admin_link_status.state == VMK_LINK_STATE_DOWN) {
                link_info.state = VMK_LINK_STATE_DOWN;
        }
        vmk_SpinlockUnlock(uplink_handle->admin_status_lock);

        IONIC_EN_SHARED_AREA_BEGIN_WRITE(uplink_handle);

        if (vmk_Memcmp(&shared_data->link, &link_info, sizeof(vmk_LinkStatus))) {
                is_update_needed = VMK_TRUE;
                ionic_en_info("%s: Link state: %s speed: %u MBPS,"
                           " Shared Link state: %s speed: %u MBPS",
                           vmk_NameToString(&uplink_handle->uplink_name),
                           link_info.state == VMK_LINK_STATE_UP ?
                           "UP" : "DOWN",
                           link_info.speed,
                           shared_data->link.state == VMK_LINK_STATE_UP ?
                           "UP" : "DOWN",
                           shared_data->link.speed);
                vmk_Memcpy(&shared_data->link, &link_info, sizeof(vmk_LinkStatus));
        }

        IONIC_EN_SHARED_AREA_END_WRITE(uplink_handle);

        if (is_update_needed) {
                vmk_UplinkUpdateLinkState(uplink_handle->uplink_dev, &link_info);
                ionic_en_info("%s: Changed Shared Link state: %s speed: %u MBPS",
                           vmk_NameToString(&uplink_handle->uplink_name),
                           shared_data->link.state == VMK_LINK_STATE_UP ?
                           "UP" : "DOWN",
                           shared_data->link.speed);
        }

}


/*
 ******************************************************************************
 *
 * ionic_en_query_port --
 *
 *     Send a port query command to hardware in order to get
 *     link's status (speed, state, duplex , autoneg ,
 *     transceiver type) in uplink_handle
 *
 *  Parameters:
 *     uplink_handle  - IN (uplink handle)
 *
 *  Results:
 *     None
 *
 *  Side-effects:
 *     None.
 *
 ******************************************************************************
 */

static void
ionic_en_query_port(struct ionic_en_uplink_handle *uplink_handle)  // IN
{
        vmk_SpinlockLockIgnoreDeathPending(uplink_handle->link_status_lock);
        if (uplink_handle->is_mgmt_nic) {
                uplink_handle->link_status.state  = VMK_LINK_STATE_UP;
                uplink_handle->link_status.duplex = VMK_LINK_DUPLEX_FULL;
                uplink_handle->link_status.speed  = VMK_LINK_SPEED_1000_MBPS;
        } else {
                uplink_handle->link_status.state  = uplink_handle->cur_hw_link_status.state;
                uplink_handle->link_status.duplex = uplink_handle->cur_hw_link_status.duplex;
                uplink_handle->link_status.speed  = uplink_handle->cur_hw_link_status.speed;
        }
        vmk_SpinlockUnlock(uplink_handle->link_status_lock);

        ionic_en_uplink_link_state_update(uplink_handle);
}


/*
 *****************************************************************************
 *
 * ionic_en_link_status_set
 *
 *    Handler used by vmkernel to set link state of a device associated with
 *    uplink.
 *
 *    driver_data  - IN (private driver data pointing to the adapter)
 *    link_info    - IN (vmkernel link status)
 *
 * Results:
 *    VMK_ReturnStatus
 *
 * Side effects:
 *    None
 *
 *****************************************************************************
 */

static VMK_ReturnStatus
ionic_en_link_status_set(vmk_AddrCookie driver_data,              // IN
                         vmk_LinkStatus *link_info)               // IN
{
        VMK_ReturnStatus status = VMK_OK;
        vmk_LinkStatus curr_link_info;
        struct ionic_en_priv_data *priv_data =
                (struct ionic_en_priv_data *) driver_data.ptr;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;

        ionic_en_dbg("ionic_en_link_status_set() called");

        VMK_ASSERT(priv_data != NULL);
        VMK_ASSERT(uplink_handle != NULL);

        if (!link_info) {
                ionic_en_err("link_info is not valid");
                return VMK_FAILURE;
        }

        vmk_SpinlockLockIgnoreDeathPending(uplink_handle->admin_status_lock);
        uplink_handle->admin_link_status = *link_info;
        vmk_SpinlockUnlock(uplink_handle->admin_status_lock);

        vmk_SpinlockLockIgnoreDeathPending(uplink_handle->link_status_lock);
        curr_link_info = priv_data->uplink_handle.link_status;
        vmk_Memcpy(&priv_data->uplink_handle.link_status,
                   link_info,
                   sizeof(vmk_LinkStatus));
        vmk_SpinlockUnlock(uplink_handle->link_status_lock);

        ionic_en_info("Current link state: %s , expected link state: %s ",
                   (curr_link_info.state == VMK_LINK_STATE_UP) ?
                   "UP" : "DOWN",
                   (link_info->state == VMK_LINK_STATE_UP) ?
                   "UP" : "DOWN");


        switch (link_info->state) {
                case VMK_LINK_STATE_DOWN:
                        status = ionic_en_uplink_quiesce_io(driver_data);
                        if (status != VMK_OK) {
                                ionic_en_err("ionic_en_UplinkQuiesceIO failed: %s",
                                          vmk_StatusToString(status));
                        }

                        break;
                case VMK_LINK_STATE_UP:
//                        ionic_en_uplink_quiesce_io(priv_data);
                        status = ionic_en_uplink_start_io(driver_data);
                         if (status != VMK_OK) {
                                ionic_en_err("ionic_en_UplinkStartIO failed: %s",
                                          vmk_StatusToString(status));
                        }

                        vmk_WorldForceWakeup(uplink_handle->link_check_world);
                        break;
                default:
                        ionic_en_warn("Unknown link state=0x%x", link_info->state);
                        status = VMK_BAD_PARAM;
                        break;
        }

        return status;
}


/*
 ******************************************************************************
 *
 *  ionic_en_get_tx_ring_from_qid
 *
 *     Get TX ring from a given uplink qid
 *
 *  Parameters:
 *     uplink_handle    - IN (uplink handle)
 *     uplink_qid       - IN (uplink qid)
 *
 *  Results:
 *     pointer of struct ionic_en_tx_ring
 *
 *  Side effects:
 *     None
 *
 *******************************************************************************
 */

inline struct ionic_en_tx_ring *
ionic_en_get_tx_ring_from_qid(struct ionic_en_uplink_handle *uplink_handle,   // IN
                              vmk_UplinkQueueID uplink_qid)                   // IN
{
        struct ionic_en_tx_ring *tx_ring = NULL;
        vmk_Bool cond1, cond2, cond3, cond4;
        vmk_uint32 shared_q_data_idx   = vmk_UplinkQueueIDQueueDataIndex(uplink_qid);
        vmk_UplinkQueueType q_type     = vmk_UplinkQueueIDType(uplink_qid);
        vmk_uint32 tx_ring_idx         = vmk_UplinkQueueIDUserVal(uplink_qid);

        if (VMK_LIKELY(q_type == VMK_UPLINK_QUEUE_TYPE_TX)) {
                /* Verify the mapping of uplink qid */
                cond1 = tx_ring_idx < uplink_handle->max_tx_queues;
                cond2 = tx_ring_idx == (shared_q_data_idx -
                                uplink_handle->max_rx_queues);
                cond3 = shared_q_data_idx >= uplink_handle->max_rx_queues;
                cond4 = shared_q_data_idx < (uplink_handle->max_rx_queues +
                                uplink_handle->max_tx_queues);
                if (cond1 && cond2 && cond3 && cond4) {
                        tx_ring = &(uplink_handle->tx_rings[tx_ring_idx]);
                } else {
                        ionic_en_err("Failed at validating tx qid mapping, q_type: %u, "
                                  "shared_q_data_idx: %u, tx_ring_idx: %u, ",
                                  q_type, shared_q_data_idx, tx_ring_idx);
                }
        } else {
                ionic_en_dbg("Invalid uplink qid, shared_q_data_idx: %u,"
                          "q_type: %u, tx_ring_idx: %u",
                          shared_q_data_idx, q_type, tx_ring_idx);
        }

        return tx_ring;
}


/*
 *****************************************************************************
 *
 * ionic_en_uplink_tx
 *
 *     This callback gets invoked by the uplink layer to transmit a pkt/list
 *
 *  Parameters:
 *     driver_data - IN (pointer to driver private data)
 *     pkt_list    - IN (vmkernel packet list to transmit)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

VMK_ReturnStatus
ionic_en_uplink_tx(vmk_AddrCookie driver_data,                    // IN
                   vmk_PktList pkt_list)                          // IN/OUT
{
        VMK_ReturnStatus status = VMK_FAILURE;
        vmk_PktHandle *pkt;
        struct ionic_en_tx_ring *tx_ring;
        vmk_UplinkQueueID vmk_qid;
        struct ionic_en_priv_data *priv_data =
                (struct ionic_en_priv_data *) driver_data.ptr;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;
        vmk_PktHandle *last_one;

        VMK_PKTLIST_ITER_STACK_DEF(iter);
        vmk_PktListIterStart(iter, pkt_list);

        pkt = vmk_PktListGetFirstPkt(pkt_list);
        if (VMK_UNLIKELY(!pkt)) {
                return VMK_OK;
        }

        last_one = vmk_PktListGetLastPkt(pkt_list);

        /*
         * Per vmk_UplinkTxCB,
         *   All packets in pktList are guaranteed to have same TX queue
         *   ID, hence a driver can always use the first packet's queue ID
         *   to select the TX queue for transmit.
         */
        vmk_qid = vmk_PktQueueIDGet(pkt);

        tx_ring = ionic_en_get_tx_ring_from_qid(uplink_handle,
                                                vmk_qid);
        if (VMK_UNLIKELY(!tx_ring)) {
                ionic_en_dbg("ionic_en_get_tx_ring_from_qid() failed.");
                goto tx_ring_err;
        }

        while (vmk_PktListIterIsAtEnd(iter) != VMK_TRUE) {
                vmk_PktListIterRemovePkt(iter, &pkt);

                /* Debug if pkt is not valid */
                VMK_ASSERT(pkt);
                VMK_ASSERT(vmk_PktQueueIDGet(pkt) == vmk_qid);

                status = ionic_start_xmit(pkt,
                                          uplink_handle,
                                          tx_ring,
                                          (last_one == pkt));
                if (status == VMK_BUSY) {
                        status = vmk_PktListIterInsertPktBefore(iter, pkt);
                        VMK_ASSERT(status == VMK_OK);
                        status = VMK_BUSY;
                        break;
                }
        }

        return status;

tx_ring_err:
        while(!vmk_PktListIterIsAtEnd(iter)) {
                vmk_PktListIterRemovePkt(iter, &pkt);
                /* Debug if pkt is not valid */
                VMK_ASSERT(pkt);
                ionic_en_pkt_release(pkt, NULL);
        }

        return status;
}


/*
 *****************************************************************************
 *
 * ionic_en_uplink_mtu_set
 *
 *     MTU set callback from uplink layer
 *
 *  Parameters:
 *     driver_data - IN (private driver data pointing to the adapter)
 *     new_mtu     - IN (new mtu value)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

VMK_ReturnStatus
ionic_en_uplink_mtu_set(vmk_AddrCookie driver_data,               // IN
                        vmk_uint32 new_mtu)                       // IN
{
        VMK_ReturnStatus status = VMK_OK;
        struct lif *lif;
        vmk_uint32 cur_mtu, i;
        struct ionic_en_priv_data *priv_data =
                (struct ionic_en_priv_data *) driver_data.ptr;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;

        if (uplink_handle->is_mgmt_nic) {
                ionic_en_warn("MTU of mnic shouldn't be changed.");
                return VMK_FAILURE;
        }

        lif = VMK_LIST_ENTRY(vmk_ListFirst(&priv_data->ionic.lifs),
                             struct lif, list);

        struct ionic_admin_ctx ctx = {
//                .work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
                .cmd.lif_setattr = {
			.opcode = CMD_OPCODE_LIF_SETATTR,
			.index = lif->index,
			.attr = IONIC_LIF_ATTR_MTU,
			.mtu = new_mtu,
                },
        };

        ionic_en_dbg("ionic_en_uplink_mtu_set() called");

        IONIC_EN_SHARED_AREA_BEGIN_READ(uplink_handle);
        cur_mtu = uplink_handle->uplink_shared_data.mtu;
        IONIC_EN_SHARED_AREA_END_READ(uplink_handle);

        if (new_mtu > IONIC_MAX_MTU || new_mtu < IONIC_MIN_MTU) {
                ionic_en_warn("Invalid MTU size: %d", new_mtu);
                status = VMK_BAD_PARAM;
                goto out;
        }

        if (cur_mtu == new_mtu) {
                ionic_en_warn("New MTU value: %d is the same as current"
                           " MTU value: %d",
                           new_mtu, cur_mtu);
                goto out;
        }

        ionic_en_info("Current MTU: %d, new MTU: %d", cur_mtu, new_mtu);

        ionic_en_uplink_quiesce_io(driver_data);

        status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_en_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		goto change_mtu_err;
	}
        ionic_completion_init(&ctx.work);

        status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
	if (status != VMK_OK) {
                ionic_en_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
                goto change_mtu_err;
        }

        IONIC_EN_SHARED_AREA_BEGIN_WRITE(uplink_handle);
        uplink_handle->uplink_shared_data.mtu = new_mtu;
        IONIC_EN_SHARED_AREA_END_WRITE(uplink_handle);

	for (i = 0; i < lif->nrxqcqs; i++) {
		ionic_rx_refill(&lif->rxqcqs[i]->q);
        }

change_mtu_err:
        status = ionic_en_uplink_start_io(driver_data);

out:
        return status;
}


/*
 *****************************************************************************
 *
 * ionic_en_uplink_state_set
 *
 *     Uplink State set callback from uplink layer
 *
 *  Parameters:
 *     driver_data    - IN (private driver data pointing to the adapter)
 *     new_state      - IN (uplink state)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

VMK_ReturnStatus
ionic_en_uplink_state_set(vmk_AddrCookie driver_data,             // IN
                          vmk_UplinkState new_state)               // IN
{
        VMK_ReturnStatus status;
        vmk_UplinkState cur_state;
        struct lif *lif;

        struct ionic_en_priv_data *priv_data =
                (struct ionic_en_priv_data *) driver_data.ptr;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;

        ionic_en_dbg("ionic_en_uplink_state_set() called");

        cur_state = uplink_handle->prev_state;

        //vmk_SpinlockLock(priv_data->ionic.lifs_lock);
        lif = VMK_LIST_ENTRY(vmk_ListFirst(&priv_data->ionic.lifs),
                             struct lif, list);
        //vmk_SpinlockLock(priv_data->ionic.lifs_lock);

        /* We don't need to send cmd to hardware */
        if ((new_state & VMK_UPLINK_STATE_DISABLED) &&
            !(cur_state & VMK_UPLINK_STATE_ENABLED)) {
                goto out;
        }

        status = ionic_set_rx_mode(lif, new_state);
        if (status != VMK_OK) {
                ionic_en_err("ionic_set_rx_mode() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

out:
        uplink_handle->prev_state = new_state;
        IONIC_EN_SHARED_AREA_BEGIN_WRITE(uplink_handle);
        uplink_handle->uplink_shared_data.state = new_state;
        IONIC_EN_SHARED_AREA_END_WRITE(uplink_handle);

        return status;
}


/*
 *****************************************************************************
 *
 * ionic_en_uplink_lif_stats_get
 *
 *     Get uplink stats from the given lif
 *
 *  Parameters:
 *     lif            - IN (lif)
 *     uplink_stats   - IN (uplink stats pointer)
 *
 *  Results:
 *     None
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

static void
ionic_en_uplink_lif_stats_get(struct lif *lif,                  // IN
                              vmk_UplinkStats *uplink_stats)    // IN/OUT
{
        struct lif_stats *ls = &lif->info->stats;

        uplink_stats->rxPkts = ls->rx_ucast_packets +
                               ls->rx_mcast_packets +
                               ls->rx_bcast_packets;
        uplink_stats->rxBytes = ls->rx_ucast_bytes +
                                ls->rx_mcast_bytes +
                                ls->rx_bcast_bytes;

        uplink_stats->rxDrops = ls->rx_ucast_drop_packets +
                                ls->rx_mcast_drop_packets +
                                ls->rx_bcast_drop_packets;
        uplink_stats->rxMulticastPkts = ls->rx_mcast_packets;
        uplink_stats->rxBroadcastPkts = ls->rx_bcast_packets;
        uplink_stats->rxOverflowErrors = ls->rx_queue_empty;
        uplink_stats->rxMissErrors = ls->rx_dma_error +
                                     ls->rx_queue_disabled +
                                     ls->rx_desc_fetch_error +
                                     ls->rx_desc_data_error;
        uplink_stats->rxErrors = uplink_stats->rxOverflowErrors +
                                 uplink_stats->rxMissErrors;

        uplink_stats->txPkts = ls->tx_ucast_packets +
                               ls->tx_mcast_packets +
                               ls->tx_bcast_packets;
        uplink_stats->txBytes = ls->tx_ucast_bytes +
                                ls->tx_mcast_bytes +
                                ls->tx_bcast_bytes;

        uplink_stats->txDrops = ls->tx_ucast_drop_packets +
                                ls->tx_mcast_drop_packets +
                                ls->tx_bcast_drop_packets;
        uplink_stats->txMulticastPkts = ls->tx_mcast_packets;
        uplink_stats->txBroadcastPkts = ls->tx_bcast_packets;
        uplink_stats->txAbortedErrors = ls->tx_dma_error +
                                        ls->tx_queue_disabled +
                                        ls->tx_desc_fetch_error +
                                        ls->tx_desc_data_error;
        uplink_stats->txErrors = uplink_stats->txAbortedErrors;
}


/*
 *****************************************************************************
 *
 * ionic_en_uplink_stats_get
 *
 *     Get uplink stats
 *
 *  Parameters:
 *     driver_data - IN (private driver data pointing to the adapter)
 *     stats      - IN (uplink stats pointer)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

VMK_ReturnStatus
ionic_en_uplink_stats_get(vmk_AddrCookie driver_data,             // IN
                          vmk_UplinkStats *stats)                 // IN/OUT
{
        struct lif *lif;
        struct ionic_en_priv_data *priv_data =
                (struct ionic_en_priv_data *) driver_data.ptr;

        VMK_ASSERT(stats);

        lif = VMK_LIST_ENTRY(vmk_ListFirst(&priv_data->ionic.lifs),
                             struct lif, list);

        vmk_Memset(stats, 0, sizeof(vmk_UplinkStats));

        ionic_en_uplink_lif_stats_get(lif,
                                      stats);

        return VMK_OK;
}


/*
 *****************************************************************************
 *
 * ionic_en_uplink_associate
 *
 *     Uplink object is associated with device
 *
 *  Parameters:
 *     driver_data - IN (private driver data pointing to the adapter)
 *     uplink     - IN (handle to uplink object)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

VMK_ReturnStatus
ionic_en_uplink_associate(vmk_AddrCookie driver_data,             // IN
                          vmk_Uplink uplink)                      // IN
{
        VMK_ReturnStatus status;
        struct lif *lif;
        vmk_WorldProps world_props;
        vmk_Name uplink_name;
        struct ionic_en_priv_data *priv_data =
                (struct ionic_en_priv_data *) driver_data.ptr;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;

        ionic_en_dbg("ionic_en_uplink_associate() called");

        uplink_handle->uplink_dev = uplink;
        uplink_name = vmk_UplinkNameGet(uplink);

        vmk_Memcpy(&uplink_handle->uplink_name,
                   &uplink_name,
                   sizeof(vmk_Name));
        vmk_Memcpy(&priv_data->uplink_name,
                   &uplink_name,
                   sizeof(vmk_Name));

        lif = VMK_LIST_ENTRY(vmk_ListFirst(&priv_data->ionic.lifs),
                             struct lif, list);
        status = ionic_lif_set_uplink_info(lif);
        if (status != VMK_OK) {
                ionic_en_err("ionic_lif_set_uplink_info() failed, status: %s",
                          vmk_StatusToString(status));
        }

        status = ionic_device_list_add(uplink_name,
                                       priv_data,
                                       &ionic_driver.uplink_dev_list);
        if (status != VMK_OK) {
                ionic_en_err("ionic_device_list_add() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        vmk_Memset(&world_props, 0, sizeof(vmk_WorldProps));
        world_props.data          = priv_data;
        world_props.moduleID      = vmk_ModuleCurrentID;
        world_props.schedClass    = VMK_WORLD_SCHED_CLASS_DEFAULT;

        world_props.name          = "dev_recover_world";
        world_props.startFunction = ionic_dev_recover_world;
        world_props.heapID        = ionic_driver.heap_id;

        status = vmk_WorldCreate(&world_props,
                                 &priv_data->dev_recover_world);
        VMK_ASSERT(status == VMK_OK);

        if (uplink_handle->hw_features & ETH_HW_VLAN_RX_STRIP) {
                status = vmk_UplinkCapRegister(uplink,
                                               VMK_UPLINK_CAP_VLAN_RX_STRIP,
                                               NULL);
                VMK_ASSERT(status == VMK_OK);
        }

        if (uplink_handle->hw_features & ETH_HW_VLAN_TX_TAG) {
                status = vmk_UplinkCapRegister(uplink,
                                               VMK_UPLINK_CAP_VLAN_TX_INSERT,
                                               NULL);
                VMK_ASSERT(status == VMK_OK);
        }

        status = vmk_UplinkCapRegister(uplink,
                                       VMK_UPLINK_CAP_TRANSCEIVER_TYPE,
                                       &ionic_en_uplink_transceiver_type_ops);
        VMK_ASSERT(status == VMK_OK || status == VMK_IS_DISABLED);

        status = vmk_UplinkCapRegister(uplink,
                                       VMK_UPLINK_CAP_PRIV_STATS,
                                       &ionic_en_priv_stats);
        VMK_ASSERT(status == VMK_OK);

        status = vmk_UplinkCapRegister(uplink,
                                       VMK_UPLINK_CAP_LINK_STATUS_SET,
                                       ionic_en_link_status_set);
        VMK_ASSERT(status == VMK_OK);

        status = vmk_UplinkCapRegister(uplink,
                                       VMK_UPLINK_CAP_CABLE_TYPE,
                                       &ionic_en_uplink_cable_type_ops);
        VMK_ASSERT(status == VMK_OK);

        if (uplink_handle->is_mgmt_nic) {
                return status;
        }

        status = vmk_UplinkCapRegister(uplink,
                                       VMK_UPLINK_CAP_MULTI_QUEUE,
                                       &ionic_en_multi_queue_ops);
        VMK_ASSERT(status == VMK_OK);


        status = vmk_UplinkCapRegister(uplink,
                                       VMK_UPLINK_CAP_IPV4_CSO,
                                       NULL);
        VMK_ASSERT(status == VMK_OK);

        status = vmk_UplinkCapRegister(uplink,
                                       VMK_UPLINK_CAP_IPV6_CSO,
                                       NULL);
        VMK_ASSERT(status == VMK_OK || status == VMK_IS_DISABLED);

        status = vmk_UplinkCapRegister(uplink,
                                       VMK_UPLINK_CAP_IPV4_TSO,
                                       NULL);
        VMK_ASSERT(status == VMK_OK || status == VMK_IS_DISABLED);

        status = vmk_UplinkCapRegister(uplink,
                                       VMK_UPLINK_CAP_IPV6_TSO,
                                       NULL);
        VMK_ASSERT(status == VMK_OK || status == VMK_IS_DISABLED);

        status = vmk_UplinkCapRegister(uplink,
                                       VMK_UPLINK_CAP_COALESCE_PARAMS,
                                       &ionic_en_uplink_coal_params_ops);
        VMK_ASSERT(status == VMK_OK);

        status = vmk_UplinkCapRegister(uplink,
                                       VMK_UPLINK_CAP_PAUSE_PARAMS,
                                       &ionic_en_uplink_pause_ops);
        VMK_ASSERT(status == VMK_OK);

        status = vmk_UplinkCapRegister(uplink,
                                       VMK_UPLINK_CAP_SG_TX,
                                       NULL);
        VMK_ASSERT(status == VMK_OK);

        status = vmk_UplinkCapRegister(uplink,
                                       VMK_UPLINK_CAP_MULTI_PAGE_SG,
                                       NULL);
        VMK_ASSERT(status == VMK_OK);

        status = vmk_UplinkQueueRegisterFeatureOps(uplink,
                                                   VMK_UPLINK_QUEUE_FEAT_RSS_DYN,
                                                   (void *)&ionic_en_dyn_rss_ops);
        VMK_ASSERT(status == VMK_OK);


        return status;
}


/*
 *****************************************************************************
 *
 * ionic_en_uplink_disassociate
 *
 *     Uplink object is disassociated from device
 *
 *  Parameters:
 *     driver_data - IN (private driver data pointing to the adapter)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

VMK_ReturnStatus
ionic_en_uplink_disassociate(vmk_AddrCookie driver_data)          // IN
{
        VMK_ReturnStatus status;
        struct ionic_en_priv_data *priv_data =
                (struct ionic_en_priv_data *) driver_data.ptr;

        ionic_en_dbg("ionic_en_uplink_disassociate() called");

        status = ionic_device_list_remove(priv_data->uplink_name,
                                          &ionic_driver.uplink_dev_list,
                                          NULL);
        if (status != VMK_OK) {
                ionic_en_err("ionic_device_list_remove() failed, status: %s",
                          vmk_StatusToString(status));
        }

        vmk_WorldDestroy(priv_data->dev_recover_world);
        vmk_WorldWaitForDeath(priv_data->dev_recover_world);

        return status;
}


/*
 *****************************************************************************
 *
 * ionic_en_uplink_cap_enable
 *
 *     Capability enabled notification call back from uplink layer
 *
 *  Parameters:
 *     driver_data - IN (private driver data pointing to the adapter)
 *     cap        - IN (uplink capability)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

VMK_ReturnStatus
ionic_en_uplink_cap_enable(vmk_AddrCookie driver_data,            // IN
                           vmk_UplinkCap cap)                     // IN
{
        struct ionic_en_priv_data *priv_data =
                (struct ionic_en_priv_data *) driver_data.ptr;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;

        ionic_en_info("ionic_en_uplink_cap_enable() called, "
                      "cap: %d is enabled.", cap);

        if (cap == VMK_UPLINK_CAP_IPV4_CSO) {
                uplink_handle->hw_features |= ETH_HW_RX_CSUM;
                uplink_handle->hw_features |= ETH_HW_TX_CSUM;
        }

        return VMK_OK;
}


/*
 *****************************************************************************
 *
 * ionic_en_uplink_cap_disable
 *
 *     Capability disabled notification call back from uplink layer
 *
 *  Parameters:
 *     driver_data - IN (private driver data pointing to the adapter)
 *     cap        - IN (uplink capability)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

VMK_ReturnStatus
ionic_en_uplink_cap_disable(vmk_AddrCookie driver_data,           // IN
                            vmk_UplinkCap cap)                    // IN
{
        struct ionic_en_priv_data *priv_data =
                (struct ionic_en_priv_data *) driver_data.ptr;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;

        ionic_en_info("ionic_en_uplink_cap_disable() called, "
                      "cap: %d is disabled.", cap);

        if (cap == VMK_UPLINK_CAP_IPV4_CSO) {
                uplink_handle->hw_features &= ~ETH_HW_RX_CSUM;
                uplink_handle->hw_features &= ~ETH_HW_TX_CSUM;
        }

        return VMK_OK;
}


/*
 *-----------------------------------------------------------------------------
 *
 * ionic_link_check_world --
 *
 *     World task to update link status (state, speed, duplex) to the vmkernel
 *
 *  Parameters:
 *     data - IN (uplink handle)
 *
 *  Results:
 *     Link status is updated
 *
 *  Side-effects:
 *     None
 *
 *-----------------------------------------------------------------------------
 */
VMK_ReturnStatus
ionic_link_check_world(void *data)                                // IN
{
        VMK_ReturnStatus status = VMK_OK;
        struct ionic_en_uplink_handle *uplink_handle = data;

        ionic_en_dbg("ionic_link_check_world() called");

        while (status != VMK_DEATH_PENDING) {
                status = vmk_WorldWait(VMK_EVENT_NONE,
                                       VMK_LOCK_INVALID,
                                       VMK_TIMEOUT_UNLIMITED_MS,
                                       "waiting for link event");

                if (status == VMK_DEATH_PENDING) {
                        break;
                }

                ionic_en_query_port(uplink_handle);
        }

        return VMK_OK;
}


/*
 *****************************************************************************
 *
 * ionic_en_uplink_start_io
 *
 *     create worlds associated with uplink, setup uplink to be IO ready and
 *     start all transmit queues
 *
 *  Parameters:
 *     driver_data: private driver data pointing to the adapter
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

VMK_ReturnStatus
ionic_en_uplink_start_io(vmk_AddrCookie driver_data)              // IN
{
        VMK_ReturnStatus status;
        vmk_WorldProps world_props;
        struct lif *lif;
        struct ionic_en_priv_data *priv_data =
                (struct ionic_en_priv_data *) driver_data.ptr;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;

        ionic_en_dbg("ionic_en_uplink_start_io() called");

        vmk_SemaLock(&uplink_handle->status_binary_sema);
        if (uplink_handle->is_started) {
                ionic_en_dbg("Uplink is already started");
                status = VMK_OK;
                goto out;
        }

        //vmk_SpinlockLock(priv_data->ionic.lifs_lock);
        lif = VMK_LIST_ENTRY(vmk_ListFirst(&priv_data->ionic.lifs),
                             struct lif, list);

         status = ionic_open(lif);
        //vmk_SpinlockLock(priv_data->ionic.lifs_lock);
        if (status != VMK_OK) {
                if (status == VMK_NOT_READY) {
                        status = VMK_OK;
                } else {
                        ionic_en_err("ionic_open() failed, status: %s",
                                  vmk_StatusToString(status));
                }
                goto out;
        }

        vmk_Memset(&world_props, 0, sizeof(vmk_WorldProps));
        world_props.data          = uplink_handle;
        world_props.moduleID      = vmk_ModuleCurrentID;
        world_props.schedClass    = VMK_WORLD_SCHED_CLASS_DEFAULT;

        world_props.name          = "link_check_world";
        world_props.startFunction = ionic_link_check_world;
        world_props.heapID        = ionic_driver.heap_id;

        status = vmk_WorldCreate(&world_props,
                                 &uplink_handle->link_check_world);
        if (status != VMK_OK) {
                ionic_en_err("vmk_WorldCreate() failed, status: %s",
                          vmk_StatusToString(status));
                goto world_create_err;
        }

        uplink_handle->is_ready_notify_linkup = VMK_TRUE;

        vmk_WorldForceWakeup(uplink_handle->link_check_world);

        goto out;

world_create_err:
        vmk_Memset(&uplink_handle->link_status, 0, sizeof(vmk_LinkStatus));
        ionic_en_uplink_link_state_update(uplink_handle);
        ionic_stop(lif);

out:
        vmk_SemaUnlock(&uplink_handle->status_binary_sema);
        return status;
}


/*
 *****************************************************************************
 *
 * ionic_en_uplink_quiesce_io
 *
 *     uplink is shutdown, all transmit queues are stopped and worlds
 *     associated with uplink are destroyed.
 *
 *  Parameters:
 *     driver_data - IN (private driver data pointing to the adapter)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

VMK_ReturnStatus
ionic_en_uplink_quiesce_io(vmk_AddrCookie driver_data)            // IN
{
        VMK_ReturnStatus status;
        struct lif *lif;
        struct ionic_en_priv_data *priv_data =
                (struct ionic_en_priv_data *) driver_data.ptr;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;

        ionic_en_dbg("ionic_en_uplink_quiesce_io() called");

        vmk_SemaLock(&uplink_handle->status_binary_sema);
        uplink_handle->is_ready_notify_linkup = VMK_FALSE;
        if (!uplink_handle->is_started) {
                ionic_en_dbg("Uplink is already stopped");
                vmk_SemaUnlock(&uplink_handle->status_binary_sema);
                return VMK_OK;
        }

        //vmk_SpinlockLock(priv_data->ionic.lifs_lock);
        lif = VMK_LIST_ENTRY(vmk_ListFirst(&priv_data->ionic.lifs),
                             struct lif, list);

        status = ionic_stop(lif);
        //vmk_SpinlockLock(priv_data->ionic.lifs_lock);
        if (status != VMK_OK) {
                ionic_en_err("ionic_stop() failed, status: %s",
                          vmk_StatusToString(status));
        }

        uplink_handle->is_started = VMK_FALSE;

        vmk_WorldDestroy(uplink_handle->link_check_world);
        vmk_WorldWaitForDeath(uplink_handle->link_check_world);

        /* Update the link state after destroying the world */
        ionic_en_uplink_link_state_update(uplink_handle);

        vmk_SemaUnlock(&uplink_handle->status_binary_sema);

        return status;
}


/*
 *****************************************************************************
 *
 * ionic_en_uplink_reset
 *
 *     Reset the device upon this notification
 *
 *  Parameters:
 *     priv_data - IN (struct ionic_en_priv_data, diver private data)
 *
 *  Results:
 *     vmk_Bool
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

VMK_ReturnStatus
ionic_en_uplink_reset(vmk_AddrCookie driver_data)                 // IN
{
        VMK_ReturnStatus status;
        ionic_en_dbg("ionic_en_uplink_reset() called");

        status = ionic_en_uplink_quiesce_io(driver_data);
        if (status != VMK_OK) {
                ionic_en_err("ionic_en_uplink_quiesce_io() failed, status: %s",
                          vmk_StatusToString(status));
                return VMK_FAILURE;
        }

        status = ionic_en_uplink_start_io(driver_data);
        if (status != VMK_OK) {
                ionic_en_err("ionic_en_uplink_start_io() failed, status: %s",
                          vmk_StatusToString(status));
        }

        return status;
}


/*
 *****************************************************************************
 *
 * ionic_en_uplink_driver_info_init
 *
 *     Initialize driver info
 *
 *  Parameters:
 *     priv_data      - IN  (ionic_en_priv_data handle)
 *     driver_info    - OUT (driver info in uplink_shared_data)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

static VMK_ReturnStatus
ionic_en_uplink_driver_info_init(struct ionic_en_priv_data *priv_data,  // IN
                                 vmk_UplinkDriverInfo *driver_info)     // OUT
{
        VMK_ReturnStatus status;
        vmk_Name fw_version;
        struct ionic *ionic = &priv_data->ionic;
	struct ionic_dev *idev = &ionic->en_dev.idev;

        status = vmk_NameInitialize(&driver_info->driver, IONIC_DRV_NAME);

        if (status != VMK_OK) {
                ionic_en_err("vmk_NameInitialize() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        status = vmk_NameInitialize(&driver_info->version, DRV_VERSION);

        if (status != VMK_OK) {
                ionic_en_err("vmk_NameInitialize() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        status = vmk_NameInitialize(&driver_info->moduleInterface, "Native");

        if (status != VMK_OK) {
                ionic_en_err("vmk_NameInitialize() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        vmk_NameInitialize(&fw_version,
                           idev->dev_info_regs->fw_version);

        vmk_NameCopy(&driver_info->firmwareVersion,
                     &fw_version);

        return status;
}


/*
 *****************************************************************************
 *
 * ionic_en_uplink_locks_init
 *
 *     Initialize locks for uplink handle
 *
 *  Parameters:
 *     uplink_handle - OUT (handle of a logical uplink device)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

static VMK_ReturnStatus
ionic_en_uplink_locks_init(struct ionic_en_uplink_handle *uplink_handle)  // IN
{
        VMK_ReturnStatus status;

        vmk_VersionedAtomicInit(&uplink_handle->uplink_shared_data.lock);

        status = ionic_spinlock_create("share_data_write_lock",
                                       ionic_driver.module_id,
                                       ionic_driver.heap_id,
                                       ionic_driver.lock_domain,
                                       VMK_SPINLOCK,
                                       IONIC_LOCK_RANK_NORMAL,
                                       &uplink_handle->share_data_write_lock);
        if (status != VMK_OK) {
                ionic_en_err("ionic_spinlock_create() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        status = ionic_spinlock_create("link_status_lock",
                                       ionic_driver.module_id,
                                       ionic_driver.heap_id,
                                       ionic_driver.lock_domain,
                                       VMK_SPINLOCK,
                                       IONIC_LOCK_RANK_NORMAL,
                                       &uplink_handle->link_status_lock);
        if (status != VMK_OK) {
                ionic_en_err("ionic_spinlock_create() failed, status: %s",
                          vmk_StatusToString(status));
                goto link_status_lock_err;
        }

        status = ionic_spinlock_create("admin_status_lock",
                                       ionic_driver.module_id,
                                       ionic_driver.heap_id,
                                       ionic_driver.lock_domain,
                                       VMK_SPINLOCK,
                                       IONIC_LOCK_RANK_NORMAL,
                                       &uplink_handle->admin_status_lock);
        if (status != VMK_OK) {
                ionic_en_err("ionic_spinlock_create() failed, status: %s",
                          vmk_StatusToString(status));
                goto admin_status_lock_err;
        }

        status = ionic_binary_sema_create(ionic_driver.heap_id,
                                          "vlan_filter_binary_sema",
                                          &uplink_handle->vlan_filter_binary_sema);
        if (status != VMK_OK) {
                ionic_en_err("ionic_binary_sema_create() failed, status: %s",
                          vmk_StatusToString(status));
                goto filter_seme_create_err;
        }

        status = ionic_binary_sema_create(ionic_driver.heap_id,
                                          "status_binary_sema",
                                          &uplink_handle->status_binary_sema);
        if (status != VMK_OK) {
                ionic_en_err("ionic_binary_sema_create() failed, status: %s",
                          vmk_StatusToString(status));
                goto start_flag_seam_create_err;
        }

        status = ionic_binary_sema_create(ionic_driver.heap_id,
                                          "mq_binary_sema",
                                          &uplink_handle->mq_binary_sema);
        if (status != VMK_OK) {
                ionic_en_err("ionic_binary_sema_create() failed, status: %s",
                          vmk_StatusToString(status));
                goto mq_seam_create_err;
        }

        status = ionic_binary_sema_create(ionic_driver.heap_id,
                                          "stats_binary_sema",
                                          &uplink_handle->stats_binary_sema);
        if (status != VMK_OK) {
                ionic_en_err("ionic_binary_sema_create() failed, status: %s",
                          vmk_StatusToString(status));
                goto stats_seam_create_err;
        }

        status = ionic_binary_sema_create(ionic_driver.heap_id,
                                          "coal_binary_sema",
                                          &uplink_handle->coal_binary_sema);
        if (status != VMK_OK) {
                ionic_en_err("ionic_binary_sema_create() failed, status: %s",
                          vmk_StatusToString(status));
                goto coal_seam_create_err;
        }

        return status;

coal_seam_create_err:
        ionic_sema_destroy(&uplink_handle->stats_binary_sema);

stats_seam_create_err:
        ionic_sema_destroy(&uplink_handle->mq_binary_sema);

mq_seam_create_err:
        ionic_sema_destroy(&uplink_handle->status_binary_sema);

start_flag_seam_create_err:
        ionic_sema_destroy(&uplink_handle->vlan_filter_binary_sema);

filter_seme_create_err:
        ionic_spinlock_destroy(uplink_handle->admin_status_lock);

admin_status_lock_err:
        ionic_spinlock_destroy(uplink_handle->link_status_lock);

link_status_lock_err:
        ionic_spinlock_destroy(uplink_handle->share_data_write_lock);

        return status;
}


/*
 *****************************************************************************
 *
 * ionic_en_uplink_locks_destroy
 *
 *     Destroy locks for uplink handle
 *
 *  Parameters:
 *     uplink_handle - OUT (handle of a logical uplink device)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

static void
ionic_en_uplink_locks_destroy(struct ionic_en_uplink_handle *uplink_handle)  // IN
{
        ionic_sema_destroy(&uplink_handle->coal_binary_sema);
        ionic_sema_destroy(&uplink_handle->stats_binary_sema);
        ionic_sema_destroy(&uplink_handle->mq_binary_sema);
        ionic_sema_destroy(&uplink_handle->status_binary_sema);
        ionic_sema_destroy(&uplink_handle->vlan_filter_binary_sema);
        ionic_spinlock_destroy(uplink_handle->admin_status_lock);
        ionic_spinlock_destroy(uplink_handle->link_status_lock);
        ionic_spinlock_destroy(uplink_handle->share_data_write_lock);
}


/*
 *****************************************************************************
 *
 * ionic_en_uplink_default_coal_params_set
 *
 *     Set default uplink/queues coalesce parameters
 *
 *  Parameters:
 *     priv_data - IN (ionic_en_priv_data handle)
 *
 *  Results:
 *     None
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

void
ionic_en_uplink_default_coal_params_set(struct ionic_en_priv_data *priv_data) // IN
{
        VMK_ReturnStatus status;
        vmk_AddrCookie driver_data;
        vmk_UplinkCoalesceParams *params;
        vmk_UplinkSharedQueueData *queue_data;
        vmk_uint32 i;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;

        driver_data.ptr = priv_data;
        params = &uplink_handle->coal_params;

        params->txUsecs = IONIC_EN_TX_COAL_USECS;
        params->rxUsecs = IONIC_EN_RX_COAL_USECS;

        status = ionic_en_uplink_coal_params_set(driver_data,
                                                 params);
        if (status != VMK_OK) {
                ionic_en_err("ionic_en_uplink_coal_params_set() failed, "
                          "status: %s", vmk_StatusToString(status));
                params->txUsecs = 0;
                params->rxUsecs = 0;
        }

        for (i = 0;
             i < uplink_handle->max_tx_queues + uplink_handle->max_rx_queues;
             i++) {
                queue_data = &uplink_handle->uplink_q_data[i];

                vmk_Memcpy(&queue_data->coalesceParams,
                           &uplink_handle->coal_params,
                           sizeof(vmk_UplinkCoalesceParams));
        }
}


VMK_ReturnStatus
ionic_en_uplink_supported_mode_init(struct ionic_en_uplink_handle *uplink_handle)
{
        vmk_UplinkSharedData *uplink_shared_data = &uplink_handle->uplink_shared_data;

         if (uplink_handle->is_mgmt_nic) {
                uplink_shared_data->supportedModesArraySz =
                        IONIC_EN_NUM_SUPPORTED_MODES_MNIC;
                uplink_shared_data->supportedModes =
                        ionic_en_uplink_modes_mnic;
        } else if(uplink_handle->cur_hw_link_status.speed ==
                  VMK_LINK_SPEED_100000_MBPS) {
                uplink_shared_data->supportedModesArraySz =
                        IONIC_EN_NUM_SUPPORTED_MODES_100G;
                uplink_shared_data->supportedModes =
                        ionic_en_uplink_modes_100g;
        } else if (uplink_handle->cur_hw_link_status.speed ==
                   VMK_LINK_SPEED_25000_MBPS) {
                uplink_shared_data->supportedModesArraySz =
                        IONIC_EN_NUM_SUPPORTED_MODES_25G;
                uplink_shared_data->supportedModes =
                        ionic_en_uplink_modes_25g;
        } else if (uplink_handle->cur_hw_link_status.speed ==
                   VMK_LINK_SPEED_10000_MBPS) {
                uplink_shared_data->supportedModesArraySz =
                        IONIC_EN_NUM_SUPPORTED_MODES_10G;
                uplink_shared_data->supportedModes =
                        ionic_en_uplink_modes_10g;
        } else {
                /* Since now we don't have a way to detect the
                 * max speed that supported, consider 100G as
                 * max speed for this case */
                uplink_shared_data->supportedModesArraySz =
                        IONIC_EN_NUM_SUPPORTED_MODES_100G;
                uplink_shared_data->supportedModes =
                        ionic_en_uplink_modes_100g;
        }

         return VMK_OK;
}


/*
 *****************************************************************************
 *
 * ionic_en_uplink_init
 *
 *     Initialize uplink
 *
 *  Parameters:
 *     priv_data - IN (ionic_en_priv_data handle)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

VMK_ReturnStatus
ionic_en_uplink_init(struct ionic_en_priv_data *priv_data)         // IN
{
        VMK_ReturnStatus status;
        vmk_uint32 i, ntxqs_per_lif, nrxqs_per_lif;
        vmk_uint32 expected_max_tx_queues, expected_max_rx_queues;
        vmk_UplinkSharedQueueData *queue_data;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;

        vmk_UplinkRegData *uplink_reg_data = &uplink_handle->uplink_reg_data;
        vmk_UplinkSharedData *uplink_shared_data = &uplink_handle->uplink_shared_data;
        vmk_UplinkSharedQueueInfo *uplink_q_info = &uplink_handle->uplink_q_info;

        ionic_en_dbg("ionic_en_uplink_init() called");

        VMK_ASSERT(uplink_handle->is_init == VMK_FALSE);

        uplink_handle->is_mgmt_nic = priv_data->ionic.is_mgmt_nic;
        uplink_handle->is_ready_notify_linkup = VMK_FALSE;
        uplink_handle->priv_data = priv_data;

        uplink_reg_data->apiRevision    = VMKAPI_REVISION;
        uplink_reg_data->moduleID       = priv_data->module_id;
        uplink_reg_data->ops            = ionic_en_uplink_ops;
        uplink_reg_data->sharedData     = uplink_shared_data;
        uplink_reg_data->driverData.ptr = priv_data;

        uplink_shared_data->flags       = VMK_UPLINK_FLAG_PSEUDO_REG;
        uplink_shared_data->state       = VMK_UPLINK_STATE_ENABLED;
        uplink_shared_data->link.state  = VMK_LINK_STATE_DOWN;
        uplink_shared_data->link.speed  = VMK_LINK_SPEED_AUTO;
        uplink_shared_data->link.duplex = VMK_LINK_DUPLEX_AUTO;
        uplink_shared_data->mtu         = IONIC_EN_DEFAULT_MTU_SIZE;
        uplink_shared_data->queueInfo   = uplink_q_info;

        uplink_handle->admin_link_status.state = VMK_LINK_STATE_UP;

        uplink_handle->cur_hw_link_status.state  = VMK_LINK_STATE_DOWN;
        uplink_handle->cur_hw_link_status.duplex = VMK_LINK_DUPLEX_AUTO;
        uplink_handle->cur_hw_link_status.speed  = VMK_LINK_SPEED_AUTO;

        status = ionic_en_uplink_driver_info_init(priv_data,
                                                  &uplink_shared_data->driverInfo);
        if (status != VMK_OK) {
                ionic_en_err("ionic_en_uplink_driver_info_init() failed, "
                          "status: %s", vmk_StatusToString(status));
                return status;
        }

        status = ionic_en_uplink_locks_init(uplink_handle);
        if (status != VMK_OK) {
                ionic_en_err("ionic_en_uplink_locks_init() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        uplink_q_info->supportedQueueTypes = VMK_UPLINK_QUEUE_TYPE_TX |
                                             VMK_UPLINK_QUEUE_TYPE_RX;
        uplink_q_info->supportedRxQueueFilterClasses =
                VMK_UPLINK_QUEUE_FILTER_CLASS_MAC_ONLY |
                VMK_UPLINK_QUEUE_FILTER_CLASS_VLAN_ONLY;
#if 0
        // TODO: Add VXLAN support in the future
        if (IONIC_EN_IS_VXLAN_OFFLOAD_ENABLED(priv_data)) {
                uplink_q_info->supportedRxQueueFilterClasses |=
                        VMK_UPLINK_QUEUE_FILTER_CLASS_VXLAN;
        }
#endif
        ntxqs_per_lif = priv_data->ionic.ntxqs_per_lif;
        nrxqs_per_lif = priv_data->ionic.nrxqs_per_lif;

        vmk_UplinkQueueGetNumQueuesSupported(ntxqs_per_lif,
                                             nrxqs_per_lif,
                                             &expected_max_tx_queues,
                                             &expected_max_rx_queues);
        uplink_handle->DRSS = DRSS;

        if (uplink_handle->DRSS > expected_max_rx_queues) {
                uplink_handle->DRSS = expected_max_rx_queues;
        }

        if (!uplink_handle->is_mgmt_nic) {
                uplink_handle->max_rx_rss_queues = uplink_handle->DRSS?
                                                   IONIX_MAX_NUM_RX_RSS_QUEUE : 0;
                uplink_handle->rx_rings_per_rss_queue = uplink_handle->DRSS;
        }

        priv_data->ionic.ntxqs_per_lif = expected_max_tx_queues;
        priv_data->ionic.nrxqs_per_lif = expected_max_rx_queues;

        uplink_handle->max_tx_queues   = expected_max_tx_queues;

        if (uplink_handle->max_rx_rss_queues) {
                uplink_handle->max_rx_queues   = expected_max_rx_queues -
                                                 uplink_handle->max_rx_rss_queues *
                                                 uplink_handle->DRSS + 1;
        } else {
                uplink_handle->max_rx_queues   = expected_max_rx_queues;
        }

        uplink_handle->max_rx_normal_queues = uplink_handle->max_rx_queues -
                                              uplink_handle->max_rx_rss_queues;

        uplink_q_info->defaultRxQueueID    = VMK_UPLINK_QUEUE_DEFAULT_QUEUEID;
        uplink_q_info->defaultTxQueueID    = VMK_UPLINK_QUEUE_DEFAULT_QUEUEID;
        uplink_q_info->activeRxQueues      = 0;
        uplink_q_info->activeTxQueues      = 0;
        uplink_q_info->maxTxQueues         = uplink_handle->max_tx_queues;
        uplink_q_info->maxRxQueues         = uplink_handle->max_rx_queues;

        uplink_q_info->queueData           = uplink_handle->uplink_q_data;

        uplink_q_info->activeQueues = vmk_BitVectorAlloc(ionic_driver.heap_id,
                                                         uplink_q_info->maxTxQueues +
                                                         uplink_q_info->maxRxQueues);
        if (uplink_q_info->activeQueues == NULL) {
                ionic_en_err("vmk_BitVectorAlloc() failed, status: VMK_NO_MEMORY");
                status = VMK_NO_MEMORY;
                goto bit_vec_alloc_error;
        }
        vmk_BitVectorZap(uplink_q_info->activeQueues);

        for (i = 0;
             i < uplink_handle->max_tx_queues + uplink_handle->max_rx_queues;
             i++) {
                queue_data                    = &uplink_handle->uplink_q_data[i];
                queue_data->flags             = VMK_UPLINK_QUEUE_FLAG_UNUSED;
                queue_data->qid               = VMK_UPLINK_QUEUE_INVALID_QUEUEID;
                queue_data->state             = VMK_UPLINK_QUEUE_STATE_STOPPED;
                queue_data->activeFeatures    = VMK_UPLINK_QUEUE_FEAT_NONE;
                queue_data->activeFilters     = 0;
                queue_data->dmaEngine         = priv_data->dma_engine_streaming;
                queue_data->priority          = VMK_VLAN_PRIORITY_MINIMUM;
                queue_data->supportedFeatures = VMK_UPLINK_QUEUE_FEAT_NONE;

                if (uplink_q_info->maxRxQueues == uplink_q_info->maxTxQueues &&
                    uplink_handle->DRSS == 0) {
                        queue_data->supportedFeatures |= VMK_UPLINK_QUEUE_FEAT_PAIR;
                }

                if (i < uplink_handle->max_rx_queues) {
                        queue_data->supportedFeatures |= VMK_UPLINK_QUEUE_FEAT_DYNAMIC;
                        if (ionic_en_is_rss_q_idx(priv_data, i)) {
                                queue_data->supportedFeatures = VMK_UPLINK_QUEUE_FEAT_RSS_DYN;
                        }
                        queue_data->type      = VMK_UPLINK_QUEUE_TYPE_RX;
                        // TODO: Figure out the number
                        queue_data->maxFilters= priv_data->max_filters;

                } else {
                        queue_data->type      = VMK_UPLINK_QUEUE_TYPE_TX;

                }
        }

        status = ionic_en_default_q_alloc(priv_data);
        if (status != VMK_OK) {
                ionic_en_err("ionic_en_default_q_alloc() failed, status: %s",
                          vmk_StatusToString(status));
                goto default_q_err;
        }

        uplink_handle->trans_type = VMK_UPLINK_TRANSCEIVER_TYPE_INTERNAL;
        uplink_handle->is_init = VMK_TRUE;

        return status;

default_q_err:
        vmk_BitVectorFree(ionic_driver.heap_id,
                          uplink_q_info->activeQueues);
bit_vec_alloc_error:
        ionic_en_uplink_locks_destroy(uplink_handle);

        return status;
}


/*
 *****************************************************************************
 *
 * ionic_en_uplink_cleanup
 *
 *     Cleanup the uplink settings
 *
 *  Parameters:
 *     priv_data - IN (ionic_en_priv_data handle)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

void
ionic_en_uplink_cleanup(struct ionic_en_priv_data *priv_data)     // IN
{
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;

        ionic_en_default_q_free(priv_data);

        ionic_en_uplink_locks_destroy(uplink_handle);

        vmk_BitVectorFree(ionic_driver.heap_id,
                          uplink_handle->uplink_q_info.activeQueues);

        vmk_Memset(uplink_handle,
                   0,
                   sizeof(struct ionic_en_uplink_handle));

        ionic_en_info("ionic_en_uplink_cleanup() completed!");
}


/*
 *****************************************************************************
 *
 * ionic_en_vlan_filter_enable_disable
 *
 *     Private call for enabling/disabling vlan filters
 *
 *  Parameters:
 *     driver_data - IN (pointer to driver private data)
 *     op_fun      - IN (function pointer indicates the
 *                       operation next)
 *
 *  Results:
 *     VMK_OK           If the operation succeeds
 *     VMK_FAILURE      Otherwise
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

static VMK_ReturnStatus
ionic_en_vlan_filter_enable_disable(vmk_AddrCookie driver_data,             // IN
                                    ionic_en_vlan_filter_add_kill vlan_op)  // IN
{
        VMK_ReturnStatus status = VMK_OK;
        vmk_VlanID vlan_id;
        struct lif *lif = NULL;
        struct ionic_en_priv_data *priv_data = (struct ionic_en_priv_data *)driver_data.ptr;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;

        // Can't hold a spinlock here
//        vmk_SpinlockLock(priv_data->ionic.lifs_lock);
        lif = VMK_LIST_ENTRY(vmk_ListFirst(&priv_data->ionic.lifs),
                             struct lif, list);
//        vmk_SpinlockLock(priv_data->ionic.lifs_lock);

        vmk_SemaLock(&uplink_handle->vlan_filter_binary_sema);
        for (vlan_id = 0; vlan_id < IONIC_VLAN_FILTER_SIZE; vlan_id++) {
                if (vmk_VLANBitmapGet(&uplink_handle->vlan_filter_bitmap,
                                      vlan_id)) {
                        status = vlan_op(lif, vlan_id);
                        VMK_ASSERT(status == VMK_OK);
                }
        }
        vmk_SemaUnlock(&uplink_handle->vlan_filter_binary_sema);

        return status;
}


/*
 *****************************************************************************
 *
 * ionic_en_vlan_filter_enable
 *
 *     Handler used by vmkernel to enable VLAN filter on device
 *
 *  Parameters:
 *     driver_data - IN (pointer to driver private data)
 *
 *  Results:
 *     VMK_OK           If the operation succeeds
 *     VMK_FAILURE      Otherwise
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

VMK_ReturnStatus
ionic_en_vlan_filter_enable(vmk_AddrCookie driver_data)                    //IN
{
        VMK_ReturnStatus status;

        ionic_en_dbg("ionic_en_vlan_filter_enable() called");

        status = ionic_en_vlan_filter_enable_disable(driver_data,
                                                     ionic_vlan_rx_add_vid);
        if (status != VMK_OK) {
                ionic_en_err("ionic_en_vlan_filter_enable_disable() failed, "
                          "status: %s", vmk_StatusToString(status));
        }

        return status;
}


/*
 *****************************************************************************
 *
 * ionic_en_vlan_filter_disable
 *
 *     Handler used by vmkernel to disable VLAN filter on device
 *
 *  Parameters:
 *     driver_data - IN (pointer to driver private data)
 *
 *  Results:
 *     VMK_OK           If the operation succeeds
 *     VMK_FAILURE      Otherwise
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

VMK_ReturnStatus
ionic_en_vlan_filter_disable(vmk_AddrCookie driver_data)          // IN
{
        VMK_ReturnStatus status;

        ionic_en_dbg("ionic_en_vlan_filter_disable() called");

        status = ionic_en_vlan_filter_enable_disable(driver_data,
                                                     ionic_vlan_rx_kill_vid);
        if (status != VMK_OK) {
                ionic_en_err("ionic_en_vlan_filter_enable_disable() failed, "
                          "status: %s", vmk_StatusToString(status));
        }

        return status;
}


/*
 *****************************************************************************
 *
 * ionic_en_vlan_filter_bitmap_get
 *
 *     Handler used by vmkernel to get uplink VLAN filter bitmap
 *
 *  Parameters:
 *     driver_data - IN (pointer to driver private data)
 *     bitmap     - OUT (VLAN filter bitmap)
 *
 *  Results:
 *     VMK_OK           If the operation succeeds
 *     VMK_FAILURE      Otherwise
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

VMK_ReturnStatus
ionic_en_vlan_filter_bitmap_get(vmk_AddrCookie driver_data,       // IN
                                vmk_VLANBitmap *bitmap)           // OUT
{
        struct ionic_en_priv_data *priv_data = (struct ionic_en_priv_data *)driver_data.ptr;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;

        ionic_en_dbg("ionic_en_vlan_filter_bitmap_get() called");

        VMK_ASSERT(bitmap);

        vmk_SemaLock(&uplink_handle->vlan_filter_binary_sema);
        vmk_Memcpy(bitmap,
                   &uplink_handle->vlan_filter_bitmap,
                   sizeof(vmk_VLANBitmap));
        vmk_SemaUnlock(&uplink_handle->vlan_filter_binary_sema);

        return VMK_OK;
}


/*
 *****************************************************************************
 *
 * ionic_en_vlan_filter_bitmap_set
 *
 *     Handler used by vmkernel to set uplink VLAN filter bitmap
 *
 *  Parameters:
 *     driver_data - IN (pointer to driver private data)
 *     bitmap     - IN (VLAN filter bitmap)
 *
 *  Results:
 *     VMK_OK           If the operation succeeds
 *     VMK_FAILURE      Otherwise
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

VMK_ReturnStatus
ionic_en_vlan_filter_bitmap_set(vmk_AddrCookie driver_data,       // IN
                                vmk_VLANBitmap *bitmap)           // IN
{
        VMK_ReturnStatus status = VMK_OK;
        vmk_VlanID vlan_id = 0;
        vmk_Bool cond1, cond2;
        struct lif *lif = NULL;
        struct ionic_en_priv_data *priv_data = (struct ionic_en_priv_data *)driver_data.ptr;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;

        ionic_en_dbg("ionic_en_vlan_filter_bitmap_set() called");
        // Can't hold a spin lock here
//        vmk_SpinlockLock(priv_data->ionic.lifs_lock);
        lif = VMK_LIST_ENTRY(vmk_ListFirst(&priv_data->ionic.lifs),
                             struct lif, list);
//        vmk_SpinlockLock(priv_data->ionic.lifs_lock);

        vmk_SemaLock(&uplink_handle->vlan_filter_binary_sema);
        for (vlan_id = 0; vlan_id < IONIC_VLAN_FILTER_SIZE; vlan_id++) {
                cond1 = vmk_VLANBitmapGet(&uplink_handle->vlan_filter_bitmap,
                                          vlan_id);
                cond2 = vmk_VLANBitmapGet(bitmap,
                                          vlan_id);

                ionic_en_info("con1: %d, con2: %d, vlanid: %d",
                           cond1, cond2, vlan_id);

                if (cond1 == cond2) {
                        continue;
                } else if (cond1 == VMK_TRUE && cond2 == VMK_FALSE) {
                        status = ionic_vlan_rx_kill_vid(lif, vlan_id);
                } else if (cond1 == VMK_FALSE && cond2 == VMK_TRUE) {
                        status = ionic_vlan_rx_add_vid(lif, vlan_id);
                }

                if (status != VMK_OK) {
                        ionic_en_err("Config vlan failed, status: %s",
                                  vmk_StatusToString(status));
                        break;
                }
        }

        vmk_Memcpy(&uplink_handle->vlan_filter_bitmap,
                   bitmap,
                   sizeof(vmk_VLANBitmap));

        vmk_SemaUnlock(&uplink_handle->vlan_filter_binary_sema);

        return status;
}


/*
 *****************************************************************************
 *
 * ionic_en_vlan_filter_add_remove
 *
 *     Private call for adding/removing vlan filters
 *
 *  Parameters:
 *     driver_data - IN (pointer to driver private data)
 *     start_id    - IN (starting vlan id for adding/removing)
 *     end_id      - IN (ending vlan id for adding/removing)
 *     vlan_op1    - IN (function pointer indicates adding/killing vlan id)
 *     vlan_op2    - IN (function pointer indicates adding/killing vlan id)
 *     bitmap_op1  - IN (function pointer indicates operations on vlan bitmap)
 *     bitmap_op2  - IN (function pointer indicates operations on vlan bitmap)
 *
 *  Results:
 *     VMK_OK           If the operation succeeds
 *     VMK_FAILURE      Otherwise
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

static VMK_ReturnStatus
ionic_en_vlan_filter_add_remove(vmk_AddrCookie driver_data,
                                vmk_VlanID start_id,
                                vmk_VlanID end_id,
                                ionic_en_vlan_filter_add_kill vlan_op1,
                                ionic_en_vlan_filter_add_kill vlan_op2,
                                ionic_en_vlan_bitmap_set_clr bitmap_op1,
                                ionic_en_vlan_bitmap_set_clr bitmap_op2)
{
        VMK_ReturnStatus status = VMK_OK, status1;
        vmk_VlanID vlan_id;
        struct lif *lif = NULL;
        struct ionic_en_priv_data *priv_data = (struct ionic_en_priv_data *)driver_data.ptr;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;

        /* Usually networking stack should guarantee the condition below,
         * we add it just for having additional protection */
        VMK_ASSERT(start_id >= end_id);

        ionic_en_dbg("ionic_en_vlan_filter_add_remove() called");

        // Can't hold a spin lock here
//        vmk_SpinlockLock(priv_data->ionic.lifs_lock);
        lif = VMK_LIST_ENTRY(vmk_ListFirst(&priv_data->ionic.lifs),
                             struct lif, list);
//        vmk_SpinlockLock(priv_data->ionic.lifs_lock);

        vmk_SemaLock(&uplink_handle->vlan_filter_binary_sema);

        for (vlan_id = start_id; vlan_id <= end_id; vlan_id++) {
                status = vlan_op1(lif, vlan_id);
                if (status != VMK_OK) {
                        break;
                }

                status = bitmap_op1(&uplink_handle->vlan_filter_bitmap,
                                    vlan_id);
                VMK_ASSERT(status == VMK_OK);

        }

        if (status != VMK_OK) {
                ionic_en_err("vlan_op1() failed, status: %s",
                          vmk_StatusToString(status));

                for (vlan_id = vlan_id - 1; vlan_id >= start_id; vlan_id--) {
                        status1 = vlan_op2(lif,
                                           vlan_id);
                        if (status1 != VMK_OK) {
                                ionic_en_err("vlan_op2() failed, status: %s",
                                          vmk_StatusToString(status));
                        }

                        status1 = bitmap_op2(&uplink_handle->vlan_filter_bitmap,
                                             vlan_id);
                        VMK_ASSERT(status1 == VMK_OK);
                }
        }

        vmk_SemaUnlock(&uplink_handle->vlan_filter_binary_sema);

        return status;
}


/*
 *****************************************************************************
 *
 * ionic_en_vlan_filter_add
 *
 *     Handler used by vmkernel to add uplink VLAN filter
 *
 *  Parameters:
 *     driver_data - IN (pointer to driver private data)
 *     start_id    - IN (starting vlan ID)
 *     end_id      - IN (ending vlan ID)
 *
 *  Results:
 *     VMK_OK           If the operation succeeds
 *     VMK_FAILURE      Otherwise
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

VMK_ReturnStatus
ionic_en_vlan_filter_add(vmk_AddrCookie driver_data,              // IN
                         vmk_VlanID start_id,                     // IN
                         vmk_VlanID end_id)                       // IN
{
        VMK_ReturnStatus status;

        ionic_en_dbg("ionic_en_vlan_filter_add() called");

        status = ionic_en_vlan_filter_add_remove(driver_data,
                                                 start_id,
                                                 end_id,
                                                 ionic_vlan_rx_add_vid,
                                                 ionic_vlan_rx_kill_vid,
                                                 vmk_VLANBitmapSet,
                                                 vmk_VLANBitmapClr);
        if (status != VMK_OK) {
                ionic_en_err("ionic_en_vlan_filter_add_remove() failed, "
                          "status: %s", vmk_StatusToString(status));
        }

        return status;
}


/*
 *****************************************************************************
 *
 * ionic_en_vlan_filter_remove
 *
 *     Handler used by vmkernel to remove uplink VLAN filter
 *
 *  Parameters:
 *     driver_data - IN (pointer to driver private data)
 *     start_id    - IN (starting vlan ID)
 *     end_id      - IN (ending vlan ID)
 *
 *  Results:
 *     VMK_OK           If the operation succeeds
 *     VMK_FAILURE      Otherwise
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

VMK_ReturnStatus
ionic_en_vlan_filter_remove(vmk_AddrCookie driver_data,           // IN
                            vmk_VlanID start_id,                  // IN
                            vmk_VlanID end_id)                    // IN
{
        VMK_ReturnStatus status;

        ionic_en_dbg("ionic_en_vlan_filter_remove() called");

        status = ionic_en_vlan_filter_add_remove(driver_data,
                                                 start_id,
                                                 end_id,
                                                 ionic_vlan_rx_kill_vid,
                                                 ionic_vlan_rx_add_vid,
                                                 vmk_VLANBitmapClr,
                                                 vmk_VLANBitmapSet);
        if (status != VMK_OK) {
                ionic_en_err("ionic_en_vlan_filter_add_remove() failed, "
                          "status: %s", vmk_StatusToString(status));
        }

        return status;
}


/*
 *****************************************************************************
 *
 * ionic_en_netpoll_create
 *
 *    Create netpoll
 *
 *  Parameters:
 *     netpoll       - OUT (netpoll that is going to be created)
 *     uplink_handle - OUT (uplink handle to be updated)
 *     ring_type     - IN  (ring type)
 *     poll          - IN  (poll routine for the device)
 *     poll_arg      - IN  (private data structure for device poll routine)
 *
 *  Results:
 *     VMK_OK on success
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

VMK_ReturnStatus
ionic_en_netpoll_create(vmk_NetPoll *netpoll,                           // OUT
                        struct ionic_en_uplink_handle *uplink_handle,   // IN
                        enum ionic_en_ring_type ring_type,              // IN
                        vmk_NetPollCallback poll,                       // IN
                        void *poll_arg)                                 // IN
{
        VMK_ReturnStatus status;
        vmk_ServiceAcctID service_id;
        vmk_NetPollProperties poll_props;
        vmk_uint32 shared_q_data_idx;
        struct qcq *qcq = (struct qcq*)poll_arg;

        status = vmk_ServiceGetID(VMK_SERVICE_ACCT_NAME_NET, &service_id);
        if (status != VMK_OK) {
                ionic_en_err("vmk_ServiceGetID() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        if (ring_type == IONIC_EN_RX_RING &&
            qcq->ring_idx < uplink_handle->max_rx_normal_queues) {
                shared_q_data_idx = qcq->ring_idx;
        } else if (ring_type == IONIC_EN_TX_RING) {
                shared_q_data_idx = uplink_handle->max_rx_queues +
                                    qcq->ring_idx;
        } else {
                shared_q_data_idx = IONIC_EN_INVALID_SHARED_QUEUE_DATA_INDEX;
        }

        vmk_Memset(&poll_props, 0, sizeof(vmk_NetPollProperties));

        poll_props.poll                 = poll;
        poll_props.priv.ptr             = poll_arg;
        /* We can also provide our delivery callback later if needed */
        poll_props.deliveryCallback     = NULL;
        poll_props.features             = VMK_NETPOLL_NONE;

        status = vmk_NetPollCreate(&poll_props,
                                   service_id,
                                   ionic_driver.module_id,
                                   netpoll);
        if (status != VMK_OK) {
                ionic_en_err("vmk_NetPollCreate() failed, status: %s",
                          vmk_StatusToString(status));
        }

        if (shared_q_data_idx != IONIC_EN_INVALID_SHARED_QUEUE_DATA_INDEX) {
                IONIC_EN_SHARED_AREA_BEGIN_WRITE(uplink_handle)
                uplink_handle->uplink_q_data[shared_q_data_idx].poll = *netpoll;
                IONIC_EN_SHARED_AREA_END_WRITE(uplink_handle)
        }

        return status;
}


/*
 *****************************************************************************
 *
 * ionic_en_uplink_coal_params_get
 *
 *     Get coalesce parameters
 *
 *  Parameters:
 *     driver_data - IN (private driver data pointing to the adapter)
 *     params      - IN (Uplink coalesce parameters)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

VMK_ReturnStatus
ionic_en_uplink_coal_params_get(vmk_AddrCookie driver_data,             // IN
                                vmk_UplinkCoalesceParams *params)       // IN/OUT
{
        struct ionic_en_priv_data *priv_data =
                (struct ionic_en_priv_data *) driver_data.ptr;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;

        ionic_en_dbg("ionic_en_uplink_coal_params_get() called");

        VMK_ASSERT(params);

        vmk_SemaLock(&uplink_handle->coal_binary_sema);

        vmk_Memcpy(params,
                   &uplink_handle->coal_params,
                   sizeof(vmk_UplinkCoalesceParams));

        vmk_SemaUnlock(&uplink_handle->coal_binary_sema);

        return VMK_OK;
}


/*
 *****************************************************************************
 *
 * ionic_en_uplink_coal_params_set
 *
 *     Set coalesce parameters
 *
 *  Parameters:
 *     driver_data - IN (private driver data pointing to the adapter)
 *     params      - IN (Uplink coalesce parameters)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

VMK_ReturnStatus
ionic_en_uplink_coal_params_set(vmk_AddrCookie driver_data,             // IN
                                vmk_UplinkCoalesceParams *params)       // IN
{
        VMK_ReturnStatus status = VMK_OK;
        vmk_uint32 tx_coal, rx_coal, i;
        struct identity *ident;
        struct lif *lif;
        struct ionic_en_priv_data *priv_data =
                (struct ionic_en_priv_data *) driver_data.ptr;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;

        ionic_en_dbg("ionic_en_uplink_coal_params_set() called");

        VMK_ASSERT(params);

        if (params->rxMaxFrames ||
            params->txMaxFrames ||
            params->useAdaptiveRx ||
            params->useAdaptiveTx ||
            params->rateSampleInterval ||
            params->pktRateLowWatermark ||
            params->pktRateHighWatermark ||
            params->rxUsecsLow ||
            params->rxFramesLow ||
            params->txUsecsLow ||
            params->txFramesLow ||
            params->rxUsecsHigh ||
            params->rxFramesHigh ||
            params->txUsecsHigh ||
            params->txFramesHigh) {
                return VMK_NOT_SUPPORTED;
        }

        lif = VMK_LIST_ENTRY(vmk_ListFirst(&priv_data->ionic.lifs),
                             struct lif, list);
        ident = &lif->ionic->ident;

        vmk_SemaLock(&uplink_handle->coal_binary_sema);

        if (ident->dev.intr_coal_div == 0) {
                status = VMK_FAILURE;
                goto out;
        }

        /* Convert from usecs to device units */
        tx_coal = (params->txUsecs * ident->dev.intr_coal_mult) /
                  ident->dev.intr_coal_div;
        rx_coal = (params->rxUsecs * ident->dev.intr_coal_mult) /
                  ident->dev.intr_coal_div;

        if (tx_coal > INTR_CTRL_COAL_MAX || rx_coal > INTR_CTRL_COAL_MAX) {
                status = VMK_LIMIT_EXCEEDED;
                goto out;
        }

        if (params->txUsecs != uplink_handle->coal_params.txUsecs) {
                for (i = 0; i < lif->ntxqcqs; i++) {
                        ionic_intr_coal_set(&lif->txqcqs[i]->intr, tx_coal);
                }
                uplink_handle->coal_params.txUsecs = params->txUsecs;
        }
        if (params->rxUsecs != uplink_handle->coal_params.rxUsecs) {
                for (i = 0; i < lif->nrxqcqs; i++) {
                        ionic_intr_coal_set(&lif->rxqcqs[i]->intr, rx_coal);
                }
                uplink_handle->coal_params.rxUsecs = params->rxUsecs;
        }

        vmk_Memcpy(&uplink_handle->coal_params,
                   params,
                   sizeof(vmk_UplinkCoalesceParams));

out:
        vmk_SemaUnlock(&uplink_handle->coal_binary_sema);
        return status;
}
