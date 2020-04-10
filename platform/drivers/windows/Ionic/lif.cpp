
#include "common.h"

static struct lif *ionic_lif_alloc(struct ionic *ionic, unsigned int index);

static NDIS_STATUS ionic_qcqs_alloc(struct lif *lif);

static void ionic_qcqs_free(struct lif *lif);

static NDIS_STATUS ionic_qcq_alloc(struct lif *lif,
                                   unsigned int type,
                                   unsigned int index,
                                   const char *name,
                                   unsigned int flags,
                                   unsigned int num_descs,
                                   unsigned int desc_size,
                                   unsigned int cq_desc_size,
                                   unsigned int sg_desc_size,
                                   unsigned int pid,
                                   struct qcq **qcq,
                                   ULONG preferredProc);

static void ionic_link_qcq_interrupts(struct qcq *src_qcq, struct qcq *n_qcq);

static void ionic_qcq_free(struct lif *lif, struct qcq *qcq);

NDIS_STATUS
ionic_lif_identify(struct ionic *ionic, u8 lif_type, union lif_identity *lid)
{
    struct ionic_dev *idev = &ionic->idev;
    ULONG sz;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    sz = min(sizeof(*lid), sizeof(idev->dev_cmd_regs->data));

    NdisAcquireSpinLock(&ionic->dev_cmd_lock);
    ionic_dev_cmd_lif_identify(idev, lif_type, IONIC_IDENTITY_VERSION_1);
    status = ionic_dev_cmd_wait(ionic, devcmd_timeout);
    memcpy_fromio(lid, &idev->dev_cmd_regs->data, sz);
    NdisReleaseSpinLock(&ionic->dev_cmd_lock);

    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s ionic_dev_cmd_lif_identify failed Status %08lX\n",
                  __FUNCTION__, status));
        return (status);
    }

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s ionic_dev_cmd_lif_identify Successful\n", __FUNCTION__));

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "\tcapabilities 0x%llx\n", le64_to_cpu(lid->capabilities)));

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "\teth.max_ucast_filters %d\n",
              le32_to_cpu(lid->eth.max_ucast_filters)));
    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "\teth.max_mcast_filters %d\n",
              le32_to_cpu(lid->eth.max_mcast_filters)));
    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "\teth.features 0x%llx\n",
              le64_to_cpu(lid->eth.config.features)));
    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "\teth.queue_count[IONIC_QTYPE_ADMINQ] %d\n",
              le32_to_cpu(lid->eth.config.queue_count[IONIC_QTYPE_ADMINQ])));
    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "\teth.queue_count[IONIC_QTYPE_NOTIFYQ] %d\n",
              le32_to_cpu(lid->eth.config.queue_count[IONIC_QTYPE_NOTIFYQ])));
    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "\teth.queue_count[IONIC_QTYPE_RXQ] %d\n",
              le32_to_cpu(lid->eth.config.queue_count[IONIC_QTYPE_RXQ])));
    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "\teth.queue_count[IONIC_QTYPE_TXQ] %d\n",
              le32_to_cpu(lid->eth.config.queue_count[IONIC_QTYPE_TXQ])));
    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "\teth.config.name %s\n", lid->eth.config.name));

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "\teth.config.mac %02lX:%02lX:%02lX:%02lX:%02lX:%02lX\n",
              lid->eth.config.mac[0], lid->eth.config.mac[1],
              lid->eth.config.mac[2], lid->eth.config.mac[3],
              lid->eth.config.mac[4], lid->eth.config.mac[5]));

    DbgTrace(
        (TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
         "\teth.config.mtu %d frame %d\n", le32_to_cpu(lid->eth.config.mtu),
         ionic->frame_size));

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
ionic_lifs_size(struct ionic *ionic)
{
    struct identity *ident = &ionic->ident;
    union lif_config *lc = &ident->lif.eth.config;
    unsigned int nintrs, dev_nintrs;
    unsigned int ntxqs_per_lif;
    unsigned int nrxqs_per_lif;
    // unsigned int neqs_per_lif;
    unsigned int nnqs_per_lif;
    unsigned int nxqs = 0; //, neqs;
    unsigned int min_intrs;
    unsigned int nslaves;
    unsigned int nlifs;

    nlifs = le32_to_cpu(ident->dev.nlifs);
    dev_nintrs = le32_to_cpu(ident->dev.nintrs);
    // neqs_per_lif = le32_to_cpu(ident->lif.rdma.eq_qtype.qid_count);
    nnqs_per_lif = le32_to_cpu(lc->queue_count[IONIC_QTYPE_NOTIFYQ]);
    ntxqs_per_lif = le32_to_cpu(lc->queue_count[IONIC_QTYPE_TXQ]);

    if (ionic->num_rss_queues == 0 ||
        ionic->num_rss_queues > le32_to_cpu(lc->queue_count[IONIC_QTYPE_RXQ])) {
        nrxqs_per_lif = le32_to_cpu(lc->queue_count[IONIC_QTYPE_RXQ]);
    } else {
        nrxqs_per_lif = ionic->num_rss_queues;
    }

    ASSERT(nlifs != 0);
    ASSERT(dev_nintrs != 0);
    // ASSERT( neqs_per_lif != 0);
    ASSERT(nnqs_per_lif != 0);
    ASSERT(ntxqs_per_lif != 0);
    ASSERT(nrxqs_per_lif != 0);

    if (max_slaves)
        nslaves = min(max_slaves, nlifs - 1);
    else
        nslaves = nlifs - 1;

    nxqs = min(ntxqs_per_lif, nrxqs_per_lif);
    nxqs = min(nxqs, ionic->proc_count); // num_online_cpus());
    // neqs = min(neqs_per_lif, ionic->proc_count); //num_online_cpus());

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s nxqs %d nintrs %d dev ints %d assigned ints %d\n",
              __FUNCTION__, nxqs,
              // neqs,
              1 + nxqs + (nslaves * (1 + nxqs)), // + neqs,
              dev_nintrs, ionic->assigned_int_cnt));

try_again:
    /* interrupt usage:
     *    1 for master lif adminq/notifyq
     *    1 for each CPU for master lif TxRx queue pairs
     *    2 for each slave lif: 1 adminq, nxqs TxRx queuepair
     *    whatever's left is for RDMA queues
     */
    // nintrs = 1 + nxqs + (nslaves * 2); // + neqs;
    nintrs = 1 + nxqs + (nslaves * (1 + nxqs));
    min_intrs = 2; /* adminq + 1 TxRx queue pair */

    if (nintrs > dev_nintrs)
        goto try_fewer;

    if (nintrs > (unsigned int)ionic->assigned_int_cnt)
        goto try_fewer;

    ionic->nnqs_per_lif = nnqs_per_lif;
    // ionic->neqs_per_lif = neqs;
    ionic->ntxqs_per_lif = nxqs;
    ionic->nrxqs_per_lif = nxqs;
    ionic->nintrs = nintrs;
    ionic->nslaves = nslaves;

    ionic->max_vmq_cnt = ((nintrs - (nxqs + 1)) / (nxqs + 1));

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s nnqs_per_lif %d ntxqs_per_lif %d nrxqs_per_lif %d nintrs %d "
              "nslaves %d vmq %d\n",
              __FUNCTION__, ionic->nnqs_per_lif, ionic->ntxqs_per_lif,
              ionic->nrxqs_per_lif, ionic->nintrs, ionic->nslaves,
              ionic->max_vmq_cnt));

    return NDIS_STATUS_SUCCESS;

try_fewer:
    if (nnqs_per_lif > 1) {
        nnqs_per_lif >>= 1;
        goto try_again;
    }
    // if (neqs > 1) {
    //	neqs >>= 1;
    //	goto try_again;
    //}
    if (nslaves) {
        // nslaves >>= 1;
        nslaves--;
        goto try_again;
    }
    if (nxqs > 1) {
        // nxqs >>= 1;
        nxqs--;
        goto try_again;
    }

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
              "%s Can't get minimum %d intrs from OS\n", __FUNCTION__,
              min_intrs));
    return NDIS_STATUS_RESOURCES;
}

NDIS_STATUS
ionic_lifs_alloc(struct ionic *ionic)
{

    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    union lif_identity *lid;
    struct lif *lif;

    /* only build the first lif, others are for dynamic macvlan offload */
    RtlSetBit(&ionic->lifbits, 0);

    lid = (union lif_identity *)NdisAllocateMemoryWithTagPriority_internal(
        ionic->adapterhandle, sizeof(union lif_identity), IONIC_LIF_ID_TAG,
        NormalPoolPriority);
    if (!lid)
        return NDIS_STATUS_RESOURCES;

    NdisZeroMemory(lid, sizeof(union lif_identity));

    DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                  "%s Allocated 0x%p len %08lX\n",
                  __FUNCTION__,
                  lid, sizeof(union lif_identity)));
    
    ionic_lif_identify(ionic, IONIC_LIF_TYPE_CLASSIC, lid);

    lif = ionic_lif_alloc(ionic, 0);
    if (lif) {
        lif->identity = lid;

        lif->lif_stats->lif_id = lif->index;
        lif->lif_stats->lif_type = IONIC_LIF_TYPE_DEFAULT;
        SetFlag(lif->lif_stats->flags, IONIC_LIF_FLAG_ALLOCATED);
    } else {
        NdisFreeMemoryWithTagPriority_internal(ionic->adapterhandle, lid,
                                      IONIC_LIF_ID_TAG);
        status = NDIS_STATUS_RESOURCES;
    }

    return status;
}

static struct lif *
ionic_lif_alloc(struct ionic *ionic, unsigned int index)
{
    struct lif *lif = NULL;
    int tbl_sz;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    unsigned int i;

    if (index == 0) {
        // struct net_device *netdev;
        int nqueues;
        /* master lif is built into the netdev */

        /* We create a netdev big enough to handle all the queues
         * needed for our macvlan slave lifs, then set the real
         * number of Tx and Rx queues used to just the queue set
         * for lif0.
         */
        nqueues = ionic->ntxqs_per_lif + ionic->nslaves;
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                  "%s nxqs=%d nslaves=%d nqueues=%d nintrs=%d\n", __FUNCTION__,
                  ionic->ntxqs_per_lif, ionic->nslaves, nqueues,
                  ionic->nintrs));

        lif = (struct lif *)NdisAllocateMemoryWithTagPriority_internal(
            ionic->adapterhandle, sizeof(struct lif), IONIC_LIF_TAG,
            NormalPoolPriority);

        if (lif == NULL) {
            return NULL;
        }

        NdisZeroMemory(lif, sizeof(struct lif));

        DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                  "%s Allocated 0x%p len %08lX\n",
                  __FUNCTION__,
                  lif, sizeof(struct lif)));
        
        ionic->master_lif = lif;

        lif->vlan_id = ionic->vlan_id;

        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                  "%s Updated lif %d to vlan %d\n", __FUNCTION__, lif->index,
                  lif->vlan_id));

        ionic->vm_queue[0].lif = lif;

        lif->neqs = ionic->neqs_per_lif;
        lif->ntxqs = ionic->ntxqs_per_lif;
        lif->nrxqs = ionic->nrxqs_per_lif;
    } else {
        /* slave lifs */

        lif = (struct lif *)NdisAllocateMemoryWithTagPriority_internal(
            ionic->adapterhandle, sizeof(struct lif), IONIC_LIF_TAG,
            NormalPoolPriority);

        if (lif == NULL) {
            return NULL;
        }

        NdisZeroMemory(lif, sizeof(struct lif));

        DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                  "%s Allocated 0x%p len %08lX\n",
                  __FUNCTION__,
                  lif, sizeof(struct lif)));
        lif->neqs = 0;
        lif->ntxqs = ionic->ntxqs_per_lif;
        lif->nrxqs = ionic->nrxqs_per_lif;
    }

    RtlInitializeBitMap(&lif->state, (PULONG)lif->state_buffer, LIF_STATE_SIZE);
    KeInitializeEvent(&lif->state_change, SynchronizationEvent, TRUE);

    lif->ionic = ionic;
    lif->index = index;
    lif->ntxq_descs = ionic->ntx_buffers;
    lif->nrxq_descs = ionic->nrx_buffers;

    lif->lif_stats = &ionic->port_stats.lif_stats[lif->index];

    _snprintf_s(lif->name, LIF_NAME_MAX_SZ, sizeof(lif->name), "lif%u", index);

    strcpy_s(lif->lif_stats->lif_name, LIF_NAME_MAX_SZ, lif->name);

    NdisAllocateSpinLock(&lif->adminq_lock);

    NdisAllocateSpinLock(&lif->deferred.lock);
    InitializeListHead(&lif->deferred.list);

    // INIT_WORK(&lif->deferred.work, ionic_lif_deferred_work);

    /* allocate lif info */
    lif->info_sz = ALIGN(sizeof(*lif->info), PAGE_SIZE);
    lif->info =
        (lif_info *)dma_alloc_coherent(ionic, lif->info_sz, &lif->info_pa, 0);

    if (!lif->info) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Failed to allocate lif info, aborting\n", __FUNCTION__));
        status = NDIS_STATUS_RESOURCES;
        goto err_out_free_netdev;
    }

    /* allocate queues */
    status = ionic_qcqs_alloc(lif);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s ionic_qcqs_alloc failed %08lX\n", __FUNCTION__, status));
        goto err_out_free_lif_info;
    }

    /* allocate rss indirection table */
    tbl_sz = le16_to_cpu(lif->ionic->ident.lif.eth.rss_ind_tbl_sz);
    lif->rss_ind_tbl_sz = (sizeof(*lif->rss_ind_tbl) * tbl_sz);
	ASSERT( lif->rss_ind_tbl_sz >= lif->ionic->ident.lif.eth.rss_ind_tbl_sz);
    lif->rss_ind_tbl_mapped = (u8 *)dma_alloc_coherent(
        ionic, lif->rss_ind_tbl_sz, &lif->rss_ind_tbl_mapped_pa, 0);

    if (!lif->rss_ind_tbl_mapped) {
        DbgTrace(
            (TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
             "%s Failed to allocate rss indirection table (mapped), aborting\n",
             __FUNCTION__));
        status = NDIS_STATUS_RESOURCES;
        goto err_out_free_qcqs;
    }

    lif->rss_ind_tbl = (u8 *)NdisAllocateMemoryWithTagPriority_internal(
        ionic->adapterhandle, lif->rss_ind_tbl_sz, IONIC_RSS_INDIR_TBL_TAG,
        NormalPoolPriority);

    if (!lif->rss_ind_tbl) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Failed to allocate rss indirection table, aborting\n",
                  __FUNCTION__));
        status = NDIS_STATUS_RESOURCES;
        goto err_out_free_qcqs;
    }

    NdisZeroMemory(lif->rss_ind_tbl, lif->rss_ind_tbl_sz);

    DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                  "%s Allocated 0x%p len %08lX\n",
                  __FUNCTION__,
                  lif->rss_ind_tbl, lif->rss_ind_tbl_sz));
    
    /* Fill indirection table with 'default' values */
    // Note: this initial default table, prior to mapping, is cpu based, not rx
    // id based and we skip cpu 0 since it is used by the default admin q

    for (i = 0; i < lif->rss_ind_tbl_sz; i++) {
        lif->rss_ind_tbl[i] = (u8)(ethtool_rxfh_indir_default(i, lif->nrxqs) +
                                   (index * lif->nrxqs) + 1);
        lif->rss_ind_tbl[i] = (lif->rss_ind_tbl[i] % lif->ionic->proc_count);
    }

    // Map the table
    map_rss_table(lif);

    ionic->total_lif_count++;
    ionic->port_stats.lif_count++;

    InsertTailList(&ionic->lifs, &lif->list);

    return lif;

err_out_free_qcqs:
    ionic_qcqs_free(lif);

err_out_free_lif_info:
    dma_free_coherent(ionic, lif->info_sz, lif->info, lif->info_pa);

    lif->info = NULL;
    lif->info_pa = 0;

err_out_free_netdev:
    NdisFreeMemoryWithTagPriority_internal(ionic->adapterhandle, lif, IONIC_LIF_TAG);

    return NULL;
}

static NDIS_STATUS
ionic_qcqs_alloc(struct lif *lif)
{
    unsigned int q_list_size;
    unsigned int flags;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    int i;

    flags = QCQ_F_INTR;
    status = ionic_qcq_alloc(lif, IONIC_QTYPE_ADMINQ, 0, "admin", flags,
                             IONIC_ADMINQ_LENGTH, sizeof(struct admin_cmd),
                             sizeof(struct admin_comp), 0, lif->kern_pid,
                             &lif->adminqcq, (ULONG)-1);
    if (status != NDIS_STATUS_SUCCESS)
        return status;

    if (is_master_lif(lif) && lif->ionic->nnqs_per_lif) {
        flags = QCQ_F_NOTIFYQ;
        status = ionic_qcq_alloc(
            lif, IONIC_QTYPE_NOTIFYQ, 0, "notifyq", flags, IONIC_NOTIFYQ_LENGTH,
            sizeof(struct notifyq_cmd), sizeof(union notifyq_comp), 0,
            lif->kern_pid, &lif->notifyqcq, (ULONG)-1);
        if (status != NDIS_STATUS_SUCCESS)
            goto err_out_free_adminqcq;

        /* Let the notifyq ride on the adminq interrupt */
        ionic_link_qcq_interrupts(lif->adminqcq, lif->notifyqcq);
    }

    q_list_size = sizeof(*lif->txqcqs) * lif->ntxqs;
    if (is_master_lif(lif))
        q_list_size += (sizeof(*lif->txqcqs) * lif->ionic->nslaves);

    status = NDIS_STATUS_RESOURCES;

    lif->txqcqs = (struct qcqst *)NdisAllocateMemoryWithTagPriority_internal(
        lif->ionic->adapterhandle, q_list_size, IONIC_QCQ_TAG,
        NormalPoolPriority);
    if (!lif->txqcqs)
        goto err_out_free_notifyqcq;

    NdisZeroMemory(lif->txqcqs, q_list_size);

    DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                  "%s Allocated 0x%p len %08lX\n",
                  __FUNCTION__,
                  lif->txqcqs, q_list_size));
    
    for (i = 0; i < (int)lif->ntxqs; i++) {
        lif->txqcqs[i].tx_stats =
            &lif->lif_stats->tx_ring[ i];
    }

    q_list_size = sizeof(*lif->rxqcqs) * lif->nrxqs;
    if (is_master_lif(lif))
        q_list_size += (sizeof(*lif->rxqcqs) * lif->ionic->nslaves);

    lif->rxqcqs = (struct qcqst *)NdisAllocateMemoryWithTagPriority_internal(
        lif->ionic->adapterhandle, q_list_size, IONIC_QCQ_TAG,
        NormalPoolPriority);
    if (!lif->rxqcqs)
        goto err_out_free_tx_stats;

    NdisZeroMemory(lif->rxqcqs, q_list_size);

    DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                  "%s Allocated 0x%p len %08lX\n",
                  __FUNCTION__,
                  lif->rxqcqs, q_list_size));
    
    for (i = 0; i < (int)lif->nrxqs; i++) {
        lif->rxqcqs[i].rx_stats =
            &lif->lif_stats->rx_ring[ i];
    }

    lif->lif_stats->rx_count = lif->nrxqs;
    lif->lif_stats->tx_count = lif->ntxqs;

    return NDIS_STATUS_SUCCESS;

err_out_free_tx_stats:

    NdisFreeMemoryWithTagPriority_internal(lif->ionic->adapterhandle, lif->txqcqs,
                                  IONIC_QCQ_TAG);
    lif->txqcqs = NULL;
err_out_free_notifyqcq:
    if (lif->notifyqcq) {
        ionic_qcq_free(lif, lif->notifyqcq);
        lif->notifyqcq = NULL;
    }
err_out_free_adminqcq:
    ionic_qcq_free(lif, lif->adminqcq);
    lif->adminqcq = NULL;

    return status;
}

static void
ionic_qcqs_free(struct lif *lif)
{
    unsigned int i;
    struct interrupt_info *int_tbl = NULL;

    if (lif->notifyqcq) {
        ionic_qcq_free(lif, lif->notifyqcq);
        lif->notifyqcq = NULL;
    }

    if (lif->adminqcq) {
        ionic_qcq_free(lif, lif->adminqcq);
        lif->adminqcq = NULL;
    }

    if( lif->rxqcqs != NULL) {
        for (i = 0; i < lif->nrxqs; i++) {

            if (lif->rxqcqs[i].qcq != NULL) {

                int_tbl = get_interrupt(lif->ionic, lif->rxqcqs[i].qcq->intr.index);

                ASSERT(int_tbl != NULL);

                IoPrint("%s Indicating DmaStopped on queue id %d\n", __FUNCTION__,
                        int_tbl->queue_id);

                IndicateRxQueueState(lif->ionic, int_tbl->queue_id,
                                     NdisReceiveQueueOperationalStateDmaStopped);

                ionic_free_rxq_pkts(lif->ionic, lif->rxqcqs[i].qcq);
                ionic_qcq_free(lif, lif->rxqcqs[i].qcq);
                lif->rxqcqs[i].qcq = NULL;
            }
        }

        NdisFreeMemoryWithTagPriority_internal(lif->ionic->adapterhandle, lif->rxqcqs,
                                      IONIC_QCQ_TAG);
        lif->rxqcqs = NULL;
    }

    if( lif->txqcqs != NULL) {
        for (i = 0; i < lif->ntxqs; i++) {

            if (lif->txqcqs[i].qcq != NULL) {
                ionic_free_txq_pkts(lif->ionic, lif->txqcqs[i].qcq);
                ionic_qcq_free(lif, lif->txqcqs[i].qcq);
                lif->txqcqs[i].qcq = NULL;
            }
        }

        NdisFreeMemoryWithTagPriority_internal(lif->ionic->adapterhandle, lif->txqcqs,
                                      IONIC_QCQ_TAG);
        lif->txqcqs = NULL;
    }
}

static NDIS_STATUS
ionic_qcq_alloc(struct lif *lif,
                unsigned int type,
                unsigned int index,
                const char *name,
                unsigned int flags,
                unsigned int num_descs,
                unsigned int desc_size,
                unsigned int cq_desc_size,
                unsigned int sg_desc_size,
                unsigned int pid,
                struct qcq **qcq,
                ULONG preferredProc)
{
    struct ionic_dev *idev = &lif->ionic->idev;
    u32 q_size, cq_size, sg_size, total_size;
    void *q_base, *cq_base, *sg_base;
    dma_addr_t cq_base_pa = 0;
    dma_addr_t sg_base_pa = 0;
    dma_addr_t q_base_pa = 0;
    struct qcq *newqcq;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    long vector = 0;
    struct interrupt_info *int_tbl = NULL;

    *qcq = NULL;

    q_size = num_descs * desc_size;
    cq_size = num_descs * cq_desc_size;
    sg_size = num_descs * sg_desc_size;

    total_size = ALIGN(q_size, PAGE_SIZE) + ALIGN(cq_size, PAGE_SIZE);
    /* Note: aligning q_size/cq_size is not enough due to cq_base
     * address aligning as q_base could be not aligned to the page.
     * Adding PAGE_SIZE.
     */
    total_size += PAGE_SIZE;
    if (flags & QCQ_F_SG) {
        total_size += ALIGN(sg_size, PAGE_SIZE);
        total_size += PAGE_SIZE;
    }

    newqcq = (struct qcq *)NdisAllocateMemoryWithTagPriority_internal(
        lif->ionic->adapterhandle, sizeof(struct qcq), IONIC_QCQ_TAG,
        NormalPoolPriority);
    if (!newqcq) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Cannot allocate queue structure\n", __FUNCTION__));
        status = NDIS_STATUS_RESOURCES;
        goto err_out;
    }

    NdisZeroMemory(newqcq, sizeof(struct qcq));

    DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                  "%s Allocated 0x%p len %08lX\n",
                  __FUNCTION__,
                  newqcq, sizeof( struct qcq)));
    
    NdisAllocateSpinLock(&newqcq->txq_pending_nb_lock);
    NdisAllocateSpinLock(&newqcq->txq_pending_nbl_lock);

    InitializeListHead(&newqcq->txq_pending_nb);
    ionic_txq_nbl_list_init(&newqcq->txq_pending_nbl);

    newqcq->flags = flags;

    newqcq->q.info = (struct desc_info *)NdisAllocateMemoryWithTagPriority_internal(
        lif->ionic->adapterhandle, sizeof(struct desc_info) * num_descs,
        IONIC_Q_INFO_TAG, NormalPoolPriority);
    if (!newqcq->q.info) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Cannot allocate queue info\n", __FUNCTION__));
        status = NDIS_STATUS_RESOURCES;
        goto err_out;
    }

    NdisZeroMemory(newqcq->q.info, sizeof(struct desc_info) * num_descs);

    DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                  "%s Allocated 0x%p len %08lX\n",
                  __FUNCTION__,
                  newqcq->q.info, sizeof(struct desc_info) * num_descs));

    newqcq->q.type = type;

    status = ionic_q_init(lif, idev, &newqcq->q, index, name, num_descs,
                          desc_size, sg_desc_size, pid);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Cannot initialize queue Status %08lX\n", __FUNCTION__,
                  status));
        goto err_out;
    }

    if (flags & QCQ_F_INTR) {
        status = ionic_intr_alloc(lif, &newqcq->intr, preferredProc);
        if (status != NDIS_STATUS_SUCCESS) {
            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                      "%s no intr for %s: %08lX\n", __FUNCTION__, name,
                      status));
            goto err_out;
        }

        vector = ionic_bus_get_irq(lif->ionic, newqcq->intr.index);
        if (vector < 0) {
            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                      "%s no vector for %s: %d\n", __FUNCTION__, name, vector));
            status = NDIS_STATUS_RESOURCES;
            goto err_out_free_intr;
        }
        newqcq->intr.vector = vector;
        ionic_intr_mask_assert(idev->intr_ctrl, newqcq->intr.index,
                               IONIC_INTR_MASK_SET);

        newqcq->intr.cpu =
            newqcq->intr.index % lif->ionic->proc_count; // num_online_cpus();

        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                  "%s Int index %d Preferred %d\n", __FUNCTION__,
                  newqcq->intr.index, preferredProc));

        int_tbl = get_interrupt(lif->ionic, newqcq->intr.index);

        ASSERT(int_tbl != NULL);

        int_tbl->QueueType = type;
    } else {
        newqcq->intr.index = (unsigned int)INTR_INDEX_NOT_ASSIGNED;
    }

    newqcq->cq.info = (struct cq_info *)NdisAllocateMemoryWithTagPriority_internal(
        lif->ionic->adapterhandle, sizeof(struct cq_info) * num_descs,
        IONIC_CQ_INFO_TAG, NormalPoolPriority);
    if (!newqcq->cq.info) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Cannot allocate completion queue info\n", __FUNCTION__));
        status = NDIS_STATUS_RESOURCES;
        goto err_out_free_intr;
    }

    NdisZeroMemory(newqcq->cq.info, sizeof(struct cq_info) * num_descs);

    DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                  "%s Allocated 0x%p len %08lX\n",
                  __FUNCTION__,
                  newqcq->cq.info, sizeof(struct cq_info) * num_descs));

    status =
        ionic_cq_init(lif, &newqcq->cq, &newqcq->intr, num_descs, cq_desc_size);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                  "%s Cannot initialize completion queue status %08lX\n",
                  __FUNCTION__, status));
        goto err_out_free_intr;
    }

    newqcq->base = AllocateSharedMemorySpecifyNuma( lif->ionic, 
													total_size, 
													&newqcq->ring_alloc_handle,
													&newqcq->base_pa);
    if (!newqcq->base) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                  "%s Cannot allocate queue DMA memory\n", __FUNCTION__));
        status = NDIS_STATUS_RESOURCES;
        goto err_out_free_intr;
    }

    newqcq->total_size = total_size;

    q_base = newqcq->base;
    q_base_pa = newqcq->base_pa;

    cq_base = (void *)ALIGN((uintptr_t)q_base + q_size, PAGE_SIZE);
    cq_base_pa = ALIGN(q_base_pa + q_size, PAGE_SIZE);

    if (flags & QCQ_F_SG) {
        sg_base = (void *)ALIGN((uintptr_t)cq_base + cq_size, PAGE_SIZE);
        sg_base_pa = ALIGN(cq_base_pa + cq_size, PAGE_SIZE);
        ionic_q_sg_map(&newqcq->q, sg_base, sg_base_pa);
    }

    ionic_q_map(&newqcq->q, q_base, q_base_pa);
    ionic_cq_map(&newqcq->cq, cq_base, cq_base_pa);
    ionic_cq_bind(&newqcq->cq, &newqcq->q);

    *qcq = newqcq;

    return NDIS_STATUS_SUCCESS;

err_out_free_intr:
    ionic_intr_free(lif, newqcq->intr.index);
err_out:

    if (newqcq != NULL) {

        if (newqcq->q.info != NULL) {
            NdisFreeMemoryWithTagPriority_internal(lif->ionic->adapterhandle,
                                          newqcq->q.info, IONIC_Q_INFO_TAG);
        }

        if (newqcq->cq.info != NULL) {
            NdisFreeMemoryWithTagPriority_internal(lif->ionic->adapterhandle,
                                          newqcq->cq.info, IONIC_CQ_INFO_TAG);
        }

        NdisFreeMemoryWithTagPriority_internal(lif->ionic->adapterhandle, newqcq,
                                      IONIC_QCQ_TAG);
    }

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
              "%s qcq alloc of %s%d failed %08lX\n", __FUNCTION__, name, index,
              status));
    return status;
}

static void
ionic_link_qcq_interrupts(struct qcq *src_qcq, struct qcq *n_qcq)
{
    if (WARN_ON(n_qcq->flags & QCQ_F_INTR)) {
        ionic_intr_free(n_qcq->cq.lif, n_qcq->intr.index);
        n_qcq->flags &= ~QCQ_F_INTR;
    }

    n_qcq->intr.vector = src_qcq->intr.vector;
    n_qcq->intr.index = src_qcq->intr.index;
}

static void
ionic_qcq_free(struct lif *lif, struct qcq *qcq)
{

    if (!qcq)
        return;

    if (qcq->base != NULL) {

        NdisFreeSharedMemory(lif->ionic->adapterhandle,
                             qcq->ring_alloc_handle);
        
        qcq->base = NULL;
        qcq->base_pa = 0;
		qcq->ring_alloc_handle = NULL;
    }

    if (qcq->netbuffer_base != NULL) {

        NdisFreeSharedMemory(lif->ionic->adapterhandle,
                             qcq->RxBufferAllocHandle);

        qcq->RxBufferAllocHandle = NULL;
        qcq->RxBufferHandle = NULL;
        qcq->netbuffer_base = NULL;
    }

    if (qcq->sg_buffer != NULL) {

        NdisFreeMemoryWithTagPriority_internal(lif->ionic->adapterhandle, qcq->sg_buffer,
                                      IONIC_SG_LIST_TAG);
        qcq->sg_buffer = NULL;
    }

    /* only the slave Tx and Rx qcqs will have master_slot set */
    if (qcq->master_slot) {
        struct lif *master_lif = lif->ionic->master_lif;
        int max = master_lif->ntxqs + lif->ionic->nslaves;

        if (qcq->master_slot >= (unsigned int)max) {
            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                      "%s bad slot number %d\n", __FUNCTION__,
                      qcq->master_slot));
        } else if (qcq->flags & QCQ_F_TX_STATS)
            master_lif->txqcqs[qcq->master_slot].qcq = NULL;
        else
            master_lif->rxqcqs[qcq->master_slot].qcq = NULL;
    }

    if (qcq->flags & QCQ_F_INTR)
        ionic_intr_free(lif, qcq->intr.index);

	if( qcq->cq.info != NULL) {
		NdisFreeMemoryWithTagPriority_internal(lif->ionic->adapterhandle, qcq->cq.info,
									  IONIC_CQ_INFO_TAG);
		qcq->cq.info = NULL;
	}

	if( qcq->q.info != NULL) {
		NdisFreeMemoryWithTagPriority_internal(lif->ionic->adapterhandle, qcq->q.info,
									  IONIC_Q_INFO_TAG);

		qcq->q.info = NULL;
	}

    NdisFreeMemoryWithTagPriority_internal(lif->ionic->adapterhandle, qcq,
                                  IONIC_QCQ_TAG);
}

NDIS_STATUS
ionic_lifs_init(struct ionic *ionic)
{
    PLIST_ENTRY cur;
    struct lif *lif;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    if (!IsListEmpty(&ionic->lifs)) {

        cur = ionic->lifs.Flink;

        do {
            lif = CONTAINING_RECORD(cur, struct lif, list);

            status = ionic_lif_init(lif);

            if (status != NDIS_STATUS_SUCCESS) {
                DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                          "%s ionic_lif_init failed %08lX\n", __FUNCTION__,
                          status));
                return status;
            }

            status = ionic_lif_set_name(lif);

            if (status != NDIS_STATUS_SUCCESS) {
                DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                          "%s ionic_lif_set_name failed %08lX\n", __FUNCTION__,
                          status));
            }

            cur = cur->Flink;
        } while (cur != &ionic->lifs);
    }

    return NDIS_STATUS_SUCCESS;
}

void
ionic_lifs_free(struct ionic *ionic)
{

    PLIST_ENTRY cur = NULL;
    PLIST_ENTRY next = NULL;
    struct lif *lif;

    if (!IsListEmpty(&ionic->lifs)) {

        cur = ionic->lifs.Flink;

        do {
            next = cur->Flink;

            lif = CONTAINING_RECORD(cur, struct lif, list);

            ionic_lif_free(lif);

            cur = next;
        } while (cur != &ionic->lifs);
    }
}

NDIS_STATUS
ionic_lif_init(struct lif *lif)
{
    struct ionic_dev *idev = &lif->ionic->idev;
    struct lif_init_comp comp;
    int dbpage_num;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    NdisAcquireSpinLock(&lif->ionic->dev_cmd_lock);
    ionic_dev_cmd_lif_init(idev, (u16)lif->index, lif->info_pa);
    status = ionic_dev_cmd_wait(lif->ionic, devcmd_timeout);
    ionic_dev_cmd_comp(idev, (union dev_cmd_comp *)&comp);
    NdisReleaseSpinLock(&lif->ionic->dev_cmd_lock);
    if (status != NDIS_STATUS_SUCCESS)
        return status;

    lif->hw_index = le16_to_cpu(comp.hw_index);

    /* now that we have the hw_index we can figure out our doorbell page */
    NdisAllocateSpinLock(&lif->dbid_inuse_lock);

    lif->dbid_count = le32_to_cpu(lif->ionic->ident.dev.ndbpgs_per_lif);
    if (!lif->dbid_count) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s No doorbell pages, aborting\n", __FUNCTION__));
        goto err_out_dma;
    }

    lif->dbid_inuse_buffer = (unsigned long *)NdisAllocateMemoryWithTagPriority_internal(
        lif->ionic->adapterhandle,
        BITS_TO_LONGS(lif->dbid_count) * sizeof(long), IONIC_GENERIC_TAG,
        NormalPoolPriority);

    if (!lif->dbid_inuse_buffer) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Failed alloc doorbell id bitmap, aborting\n",
                  __FUNCTION__));
        goto err_out_dma;
    }

    NdisZeroMemory(lif->dbid_inuse_buffer,
                   BITS_TO_LONGS(lif->dbid_count) * sizeof(long));

    DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                  "%s Allocated 0x%p len %08lX\n",
                  __FUNCTION__,
                  lif->dbid_inuse_buffer,
                  BITS_TO_LONGS(lif->dbid_count) * sizeof(long)));
    
    RtlInitializeBitMap(&lif->dbid_inuse, lif->dbid_inuse_buffer,
                        (BITS_TO_LONGS(lif->dbid_count) * sizeof(long)) * 32);

    /* first doorbell id reserved for kernel (dbid aka pid == zero) */
    RtlSetBit(&lif->dbid_inuse, 0);
    lif->kern_pid = 0;

    dbpage_num = ionic_db_page_num(lif, lif->kern_pid);
    lif->kern_dbpage = ionic_bus_map_dbpage(lif->ionic, dbpage_num);
    if (!lif->kern_dbpage) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Cannot map dbpage, aborting\n", __FUNCTION__));
        status = NDIS_STATUS_RESOURCES;
        goto err_out_free_dbid;
    }

    status = ionic_lif_adminq_init(lif);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Failed to init adminq status %08lX\n", __FUNCTION__,
                  status));
        goto err_out_adminq_deinit;
    }

    if (is_master_lif(lif) && lif->ionic->nnqs_per_lif) {
        status = ionic_lif_notifyq_init(lif);
        if (status != NDIS_STATUS_SUCCESS) {
            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                      "%s Failed to init notifq status %08lX\n", __FUNCTION__,
                      status));
            goto err_out_notifyq_deinit;
        }
    }

    status = ionic_init_nic_features(lif);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Failed to init nic features status %08lX\n", __FUNCTION__,
                  status));
        goto err_out_notifyq_deinit;
    }

    status = ionic_rx_filters_init(lif);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Failed to init rx filters status %08lX\n", __FUNCTION__,
                  status));
        goto err_out_notifyq_deinit;
    }

    status = ionic_station_set(lif);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Failed station set status %08lX\n", __FUNCTION__,
                  status));
        goto err_out_notifyq_deinit;
    }

    lif->rx_copybreak = rx_copybreak;

    lif->api_private = NULL;
    RtlSetBit(&lif->state, LIF_INITED);
    RtlSetBit(&lif->state, LIF_F_FW_READY);

    return NDIS_STATUS_SUCCESS;

err_out_notifyq_deinit:
    ionic_lif_qcq_deinit(lif, lif->notifyqcq);

err_out_adminq_deinit:
    ionic_lif_qcq_deinit(lif, lif->adminqcq);
    ionic_lif_reset(lif);
    ionic_bus_unmap_dbpage(lif->ionic, lif->kern_dbpage);
    lif->kern_dbpage = NULL;

err_out_free_dbid:
    NdisFreeMemoryWithTagPriority_internal(lif->ionic->adapterhandle,
                                  lif->dbid_inuse_buffer, IONIC_GENERIC_TAG);
    lif->dbid_inuse_buffer = NULL;

err_out_dma:
	ionic_lif_reset(lif);
	deinit_dma( lif->ionic);

    return status;
}

void
ionic_lif_free(struct lif *lif)
{

    struct ionic *ionic = lif->ionic;

    if (lif->rss_ind_tbl_mapped != NULL) {
        /* free mapped rss indirection table */
        dma_free_coherent(ionic, lif->rss_ind_tbl_sz, lif->rss_ind_tbl_mapped,
                          lif->rss_ind_tbl_mapped_pa);

        lif->rss_ind_tbl_mapped = NULL;
        lif->rss_ind_tbl_mapped_pa = 0;
    }

    if (lif->rss_ind_tbl != NULL) {
        /* free rss indirection table */
        NdisFreeMemoryWithTagPriority_internal(ionic->adapterhandle, lif->rss_ind_tbl,
                                      IONIC_RSS_INDIR_TBL_TAG);

        lif->rss_ind_tbl = NULL;
    }

    /* free queues */
    ionic_qcqs_free(lif);
    if (RtlCheckBit(&lif->state, LIF_F_FW_READY))
        ionic_lif_reset(lif);

    if (lif->identity != NULL) {
        /* free lif info */
        NdisFreeMemoryWithTagPriority_internal(ionic->adapterhandle, lif->identity,
                                      IONIC_LIF_ID_TAG);
		lif->identity = NULL;
	}

	if( lif->info != NULL) {
        dma_free_coherent(ionic, lif->info_sz, lif->info, lif->info_pa);
        lif->info = NULL;
        lif->info_pa = 0;
    }

    /* unmap doorbell page */
    ionic_bus_unmap_dbpage(ionic, lif->kern_dbpage);
    lif->kern_dbpage = NULL;

    if (lif->dbid_inuse_buffer != NULL) {
        NdisFreeMemoryWithTagPriority_internal(
            ionic->adapterhandle, lif->dbid_inuse_buffer, IONIC_GENERIC_TAG);
        lif->dbid_inuse_buffer = NULL;
    }

    if (!is_master_lif(lif)) {
        ionic_slave_free(lif->ionic, lif->index);        
    }

    RemoveEntryList(&lif->list);

    if (lif->rss_mapping != NULL) {
        NdisFreeMemoryWithTagPriority_internal(ionic->adapterhandle, lif->rss_mapping,
                                      IONIC_GENERIC_TAG);
    }

    NdisFreeMemoryWithTagPriority_internal(ionic->adapterhandle, lif, IONIC_LIF_TAG);

    ionic->total_lif_count--;
    ionic->port_stats.lif_count--;

    return;
}

NDIS_STATUS
ionic_lif_set_name(struct lif *lif)
{
    struct ionic_admin_ctx ctx = {0};
    const size_t cmd_name_len = sizeof(ctx.cmd.lif_setattr.name) - 1;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    char name[ADAPTER_NAME_MAX_SZ + LIF_NAME_MAX_SZ] = {0};
    int name_len = 0;

    ctx.cmd.lif_setattr.opcode = CMD_OPCODE_LIF_SETATTR;
    ctx.cmd.lif_setattr.attr = IONIC_LIF_ATTR_NAME;
    ctx.cmd.lif_setattr.index = (__le16)lif->index;

    // Device name and lif name, for lifs > zero
    if (lif->index == 0) {
        name_len = _snprintf(name, sizeof(name) - 1, "%S",
                             lif->ionic->name.Buffer);
    } else {
        name_len = _snprintf(name, sizeof(name) - 1, "%S %s",
                             lif->ionic->name.Buffer, lif->name);
    }

    // No error is expected... but should not proceed.
    if (name_len < 0) {
        return NDIS_STATUS_INVALID_PARAMETER;
    }

    // Formatted string should not exceed... but clamp it.
    if (name_len >= sizeof(name)) {
        name_len = sizeof(name) - 1;
    }
    // _snprintf does not guarantee string termination
    name[name_len] = 0;

    // The name fits in the cmd, else shorten the name.
    if (name_len <= cmd_name_len) {
        memcpy(ctx.cmd.lif_setattr.name, name, name_len);
    } else {
        // Shorten the name to fit, try to retain distinctness at end of name.
        // "Pensando Systems #3 lif7" would become "Pen~ems #3 lif7"

        memcpy(&ctx.cmd.lif_setattr.name[0], &name[0], 3);

        ctx.cmd.lif_setattr.name[3] = '~';

        memcpy(&ctx.cmd.lif_setattr.name[4],
               &name[name_len - cmd_name_len + 4],
               cmd_name_len - 4);
    }

    status = ionic_adminq_post_wait(lif, &ctx);
    if (status != NDIS_STATUS_SUCCESS) {
        return status;
    }

    return 0;
}

int
ionic_station_set(struct lif *lif)
{

    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    struct ionic_admin_ctx ctx = {0};
    u8 current_addr[ETH_ALEN];

    ctx.cmd.lif_getattr.opcode = CMD_OPCODE_LIF_GETATTR;
    ctx.cmd.lif_getattr.index = (__le16)cpu_to_le16(lif->index);
    ctx.cmd.lif_getattr.attr = IONIC_LIF_ATTR_MAC;

    if (!is_master_lif(lif))
        return 0;

    status = ionic_adminq_post_wait(lif, &ctx);
    if (status != NDIS_STATUS_SUCCESS)
        return status;

    if (!is_zero_ether_addr(
            (const u8 *)lif->ionic->perm_addr)) { // netdev->dev_addr)) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s deleting station MAC addr %p\n", lif->ionic->perm_addr));
        ionic_lif_addr(lif, (const u8 *)lif->ionic->perm_addr, false);
    } else {
        memcpy(lif->ionic->perm_addr, ctx.comp.lif_getattr.mac, ETH_ALEN);
    }

    if (is_zero_ether_addr((const u8 *)lif->ionic->config_addr)) {
        memcpy(lif->ionic->config_addr, lif->ionic->perm_addr, ETH_ALEN);
    }

    memcpy(current_addr, lif->ionic->config_addr, ETH_ALEN);

    DbgTrace(
        (TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
         "%s adding station MAC addr %02lX:%02lX:%02lX:%02lX:%02lX:%02lX\n",
         __FUNCTION__, current_addr[0], current_addr[1], current_addr[2],
         current_addr[3], current_addr[4], current_addr[5]));

    ionic_lif_addr(lif, (const u8 *)current_addr, true);

    if (lif->ionic->master_lif->vlan_id != 0) {
        ionic_lif_vlan(lif, (u16)lif->ionic->master_lif->vlan_id, true);
    }

    return 0;
}

static NDIS_STATUS
ionic_lif_macvlan_add(struct lif *lif, const u8 *addr, u16 vlan_id)
{

    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    struct ionic_admin_ctx ctx = {0};

    // ctx.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
    ctx.cmd.rx_filter_add.opcode = CMD_OPCODE_RX_FILTER_ADD;
    ctx.cmd.rx_filter_add.lif_index = (__le16)cpu_to_le16(lif->index);
    ctx.cmd.rx_filter_add.match = cpu_to_le16(RX_FILTER_MATCH_MAC_VLAN);
    ctx.cmd.rx_filter_add.mac_vlan.vlan = vlan_id;

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s rx_filter add ADDR %02lX:%02lX:%02lX:%02lX:%02lX:%02lX vlan "
              "id %d\n",
              __FUNCTION__, addr[0], addr[1], addr[2], addr[3], addr[4],
              addr[5], vlan_id));

    memcpy(ctx.cmd.rx_filter_add.mac_vlan.addr, addr, ETH_ALEN);
    status = ionic_adminq_post_wait(lif, &ctx);
    if (status != NDIS_STATUS_SUCCESS) {
        return status;
    }

    return ionic_rx_filter_save(lif, 0, RXQ_INDEX_ANY, 0, &ctx);
}

static NDIS_STATUS
ionic_lif_macvlan_del(struct lif *lif, const u8 *addr, u16 vlan_id)
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    struct ionic_admin_ctx ctx = {0};
    struct rx_filter *f;

    UNREFERENCED_PARAMETER(addr);

    ctx.cmd.rx_filter_del.opcode = CMD_OPCODE_RX_FILTER_DEL;
    ctx.cmd.rx_filter_del.lif_index = (__le16)cpu_to_le16(lif->index);

    NdisAcquireSpinLock(&lif->rx_filters.lock);
    f = ionic_rx_filter_by_macvlan(lif, vlan_id);
    if (!f) {
        NdisReleaseSpinLock(&lif->rx_filters.lock);
        return NDIS_STATUS_RESOURCES;
    }

    ctx.cmd.rx_filter_del.filter_id = cpu_to_le32(f->filter_id);
    ionic_rx_filter_free(lif, f);
    NdisReleaseSpinLock(&lif->rx_filters.lock);

    status = ionic_adminq_post_wait(lif, &ctx);
    if (status != NDIS_STATUS_SUCCESS)
        return status;

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s rx_filter del ADDR %02lX:%02lX:%02lX:%02lX:%02lX:%02lX vlan "
              "id %d (id %d)\n",
              __FUNCTION__, addr[0], addr[1], addr[2], addr[3], addr[4],
              addr[5], vlan_id, ctx.cmd.rx_filter_del.filter_id));

    return NDIS_STATUS_SUCCESS;
}

static NDIS_STATUS
ionic_lif_addr_add(struct lif *lif, const u8 *addr)
{

    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    struct ionic_admin_ctx ctx = {0};

    // ctx.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
    ctx.cmd.rx_filter_add.opcode = CMD_OPCODE_RX_FILTER_ADD;
    ctx.cmd.rx_filter_add.lif_index = (__le16)cpu_to_le16(lif->index);
    ctx.cmd.rx_filter_add.match = cpu_to_le16(RX_FILTER_MATCH_MAC);

    struct rx_filter *f;

    /* don't bother if we already have it */
    NdisAcquireSpinLock(&lif->rx_filters.lock);
    f = ionic_rx_filter_by_addr(lif, addr);
    NdisReleaseSpinLock(&lif->rx_filters.lock);
    if (f)
        return NDIS_STATUS_SUCCESS;

    /* make sure we're not getting a slave's filter */
    /* TODO: use a global hash rather than search every slave */
    if (is_master_lif(lif)) {

        LIST_ENTRY *cur;
        struct lif *slave_lif;

        if (!IsListEmpty(&lif->ionic->lifs)) {

            cur = lif->ionic->lifs.Flink;

            do {

                slave_lif = CONTAINING_RECORD(cur, struct lif, list);

                if (slave_lif != lif) {
                    NdisAcquireSpinLock(&slave_lif->rx_filters.lock);
                    f = ionic_rx_filter_by_addr(slave_lif, addr);
                    NdisReleaseSpinLock(&slave_lif->rx_filters.lock);
                    if (f)
                        return NDIS_STATUS_SUCCESS;
                }

                cur = cur->Flink;
            } while (cur != &lif->ionic->lifs);
        }
    }

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s rx_filter add ADDR %02lX:%02lX:%02lX:%02lX:%02lX:%02lX "
              "filter id %d lif index %d\n",
              __FUNCTION__, addr[0], addr[1], addr[2], addr[3], addr[4],
              addr[5], ctx.comp.rx_filter_add.filter_id, lif->index));

    memcpy(ctx.cmd.rx_filter_add.mac.addr, addr, ETH_ALEN);
    status = ionic_adminq_post_wait(lif, &ctx);
    if (status != NDIS_STATUS_SUCCESS) {
        return status;
    }

    return ionic_rx_filter_save(lif, 0, RXQ_INDEX_ANY, 0, &ctx);
}

static NDIS_STATUS
ionic_lif_addr_del(struct lif *lif, const u8 *addr)
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    struct ionic_admin_ctx ctx = {0};
    // ctx.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
    ctx.cmd.rx_filter_del.opcode = CMD_OPCODE_RX_FILTER_DEL;
    ctx.cmd.rx_filter_del.lif_index = (__le16)cpu_to_le16(lif->index);

    struct rx_filter *f;

    DbgTrace(
        (TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
         "%s rx_filter del ADDR %02lX:%02lX:%02lX:%02lX:%02lX:%02lX (id %d)\n",
         __FUNCTION__, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5],
         ctx.cmd.rx_filter_del.filter_id));

    NdisAcquireSpinLock(&lif->rx_filters.lock);
    f = ionic_rx_filter_by_addr(lif, addr);
    if (!f) {
        NdisReleaseSpinLock(&lif->rx_filters.lock);
        return NDIS_STATUS_RESOURCES;
    }

    ctx.cmd.rx_filter_del.filter_id = cpu_to_le32(f->filter_id);
    ionic_rx_filter_free(lif, f);
    NdisReleaseSpinLock(&lif->rx_filters.lock);

    status = ionic_adminq_post_wait(lif, &ctx);
    if (status != NDIS_STATUS_SUCCESS)
        return status;

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
ionic_lif_addr(struct lif *lif, const u8 *addr, bool add)
{
    struct ionic *ionic = lif->ionic;
    //	struct ionic_deferred_work *work;
    unsigned int nmfilters;
    unsigned int nufilters;

    if (add) {
        /* Do we have space for this filter?  We test the counters
         * here before checking the need for deferral so that we
         * can return an overflow error to the stack.
         */
        nmfilters = le32_to_cpu(ionic->ident.lif.eth.max_mcast_filters);
        nufilters = le32_to_cpu(ionic->ident.lif.eth.max_ucast_filters);

        if ((is_multicast_ether_addr(addr) && lif->nmcast < nmfilters))
            lif->nmcast++;
        else if (!is_multicast_ether_addr(addr) && lif->nucast < nufilters)
            lif->nucast++;
        else
            return NDIS_STATUS_RESOURCES;
    } else {
        if (is_multicast_ether_addr(addr) && lif->nmcast)
            lif->nmcast--;
        else if (!is_multicast_ether_addr(addr) && lif->nucast)
            lif->nucast--;
    }

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s rx_filter %s %02lX:%02lX:%02lX:%02lX:%02lX:%02lX \n",
              __FUNCTION__, add ? "add" : "del", addr[0], addr[1], addr[2],
              addr[3], addr[4], addr[5]));
    if (add)
        return ionic_lif_addr_add(lif, addr);
    else
        return ionic_lif_addr_del(lif, addr);
}

NDIS_STATUS
ionic_lif_vlan_add(struct lif *lif, u16 vid)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    struct ionic_admin_ctx ctx = {0};
    struct rx_filter *f;

    ctx.cmd.rx_filter_add.opcode = CMD_OPCODE_RX_FILTER_ADD;
    ctx.cmd.rx_filter_add.lif_index = (__le16)cpu_to_le16(lif->index);
    ctx.cmd.rx_filter_add.match = cpu_to_le16(RX_FILTER_MATCH_VLAN);
    ctx.cmd.rx_filter_add.vlan.vlan = cpu_to_le16(vid);

    NdisAcquireSpinLock(&lif->rx_filters.lock);
    f = ionic_rx_filter_by_vlan(lif, vid);
    NdisReleaseSpinLock(&lif->rx_filters.lock);

    if (f) {

        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                  "%s Already have VLan filter for %d\n", __FUNCTION__, vid));

        return NDIS_STATUS_SUCCESS;
    }

    ntStatus = ionic_adminq_post_wait(lif, &ctx);
    if (ntStatus != NDIS_STATUS_SUCCESS) {
        return ntStatus;
    }

    return ionic_rx_filter_save(lif, 0, RXQ_INDEX_ANY, 0, &ctx);
}

NDIS_STATUS
ionic_lif_vlan_del(struct lif *lif, u16 vid)
{
    struct ionic_admin_ctx ctx = {0};
    struct rx_filter *f;

    ctx.cmd.rx_filter_del.opcode = CMD_OPCODE_RX_FILTER_DEL;
    ctx.cmd.rx_filter_del.lif_index = (__le16)cpu_to_le16(lif->index);

    NdisAcquireSpinLock(&lif->rx_filters.lock);

    f = ionic_rx_filter_by_vlan(lif, vid);
    if (!f) {

        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                  "%s Don't have VLan filter for %d\n", __FUNCTION__, vid));
        NdisReleaseSpinLock(&lif->rx_filters.lock);
        return NDIS_STATUS_SUCCESS;
    }

    ctx.cmd.rx_filter_del.filter_id = cpu_to_le32(f->filter_id);
    ionic_rx_filter_free(lif, f);

    NdisReleaseSpinLock(&lif->rx_filters.lock);

    return ionic_adminq_post_wait(lif, &ctx);
}

NDIS_STATUS
ionic_lif_macvlan(struct lif *lif, const u8 *addr, u16 vlan_id, bool add)
{
    struct ionic *ionic = lif->ionic;
    unsigned int nmfilters;
    unsigned int nufilters;

    if (add) {
        /* Do we have space for this filter?  We test the counters
         * here before checking the need for deferral so that we
         * can return an overflow error to the stack.
         */
        nmfilters = le32_to_cpu(ionic->ident.lif.eth.max_mcast_filters);
        nufilters = le32_to_cpu(ionic->ident.lif.eth.max_ucast_filters);

        if ((is_multicast_ether_addr(addr) && lif->nmcast < nmfilters))
            lif->nmcast++;
        else if (!is_multicast_ether_addr(addr) && lif->nucast < nufilters)
            lif->nucast++;
        else
            return NDIS_STATUS_RESOURCES;
    } else {
        if (is_multicast_ether_addr(addr) && lif->nmcast)
            lif->nmcast--;
        else if (!is_multicast_ether_addr(addr) && lif->nucast)
            lif->nucast--;
    }

    DbgTrace(
        (TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
         "%s rx_filter %s %02lX:%02lX:%02lX:%02lX:%02lX:%02lX VLanId %d \n",
         __FUNCTION__, add ? "add" : "del", addr[0], addr[1], addr[2], addr[3],
         addr[4], addr[5], vlan_id));
    if (add)
        return ionic_lif_macvlan_add(lif, addr, vlan_id);
    else
        return ionic_lif_macvlan_del(lif, addr, vlan_id);
}

NDIS_STATUS
ionic_lif_vlan(struct lif *lif, u16 vlan_id, bool add)
{
    struct ionic *ionic = lif->ionic;
    unsigned int nmfilters;
    unsigned int nufilters;

    if (add) {
        /* Do we have space for this filter?  We test the counters
         * here before checking the need for deferral so that we
         * can return an overflow error to the stack.
         */
        nmfilters = le32_to_cpu(ionic->ident.lif.eth.max_mcast_filters);
        nufilters = le32_to_cpu(ionic->ident.lif.eth.max_ucast_filters);

        if (lif->nucast < nufilters)
            lif->nucast++;
        else
            return NDIS_STATUS_RESOURCES;
    } else {
        lif->nucast--;
    }

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s rx_filter %s VLanId %d \n", __FUNCTION__, add ? "add" : "del",
              vlan_id));
    if (add)
        return ionic_lif_vlan_add(lif, vlan_id);
    else
        return ionic_lif_vlan_del(lif, vlan_id);
}

int
ionic_napi(struct lif *lif,
           int budget,
           ionic_cq_cb cb,
           ionic_cq_done_cb done_cb,
           void *done_arg)
{
    struct qcq *qcq = lif->adminqcq; //  napi_to_qcq(napi);
    struct cq *cq = &qcq->cq;
    u32 work_done, flags = 0;

    work_done = ionic_cq_service(cq, budget, cb, NULL, done_cb, done_arg);

    if (work_done < (u32)budget /*&& napi_complete_done(napi, work_done)*/) {

        flags |= IONIC_INTR_CRED_UNMASK;
        // DEBUG_STATS_INTR_REARM(cq->bound_intr);
    }

    if (work_done || flags) {
        flags |= IONIC_INTR_CRED_RESET_COALESCE;
        ionic_intr_credits(cq->lif->ionic->idev.intr_ctrl,
                           cq->bound_intr->index, work_done, flags);
    }

    // DEBUG_STATS_NAPI_POLL(qcq, work_done);

    return work_done;
}

void
CheckLinkStatusCb(PVOID WorkItemContext, NDIS_HANDLE NdisIoWorkItemHandle)
{

    struct ionic *ionic = (struct ionic *)WorkItemContext;

    ionic_link_status_check(ionic->master_lif, PORT_OPER_STATUS_NONE);

    NdisFreeIoWorkItem(NdisIoWorkItemHandle);

    return;
}

NDIS_STATUS
ionic_lif_rx_mode(struct lif *lif, unsigned int rx_mode)
{

    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    struct ionic_admin_ctx ctx = {0};
    // ctx.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
    ctx.cmd.rx_mode_set.opcode = CMD_OPCODE_RX_MODE_SET;
    ctx.cmd.rx_mode_set.lif_index = (__le16)cpu_to_le16(lif->index);
    ctx.cmd.rx_mode_set.rx_mode = (__le16)cpu_to_le16(rx_mode);
    char buf[128];
    int i;

#define REMAIN(__x) (sizeof(buf) - (__x))

    i = sprintf_s(buf, sizeof(buf), "rx_mode 0x%04x -> 0x%04x:", lif->rx_mode,
                  rx_mode);
    if (rx_mode & RX_MODE_F_UNICAST)
        i += sprintf_s(&buf[i], REMAIN(i), " RX_MODE_F_UNICAST");
    if (rx_mode & RX_MODE_F_MULTICAST)
        i += sprintf_s(&buf[i], REMAIN(i), " RX_MODE_F_MULTICAST");
    if (rx_mode & RX_MODE_F_BROADCAST)
        i += sprintf_s(&buf[i], REMAIN(i), " RX_MODE_F_BROADCAST");
    if (rx_mode & RX_MODE_F_PROMISC)
        i += sprintf_s(&buf[i], REMAIN(i), " RX_MODE_F_PROMISC");
    if (rx_mode & RX_MODE_F_ALLMULTI)
        i += sprintf_s(&buf[i], REMAIN(i), " RX_MODE_F_ALLMULTI");
    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE, "%s lif%d %s\n",
              __FUNCTION__, lif->index, buf));

    status = ionic_adminq_post_wait(lif, &ctx);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s set rx_mode 0x%04x failed: %d\n", __FUNCTION__, rx_mode,
                  status));
    } else
        lif->rx_mode = rx_mode;

    return status;
}

NDIS_STATUS
ionic_lif_rxq_init(struct lif *lif, struct qcq *qcq)
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    struct queue *q = &qcq->q;
    struct cq *cq = &qcq->cq;
    struct ionic_admin_ctx ctx = {0};

    ctx.cmd.q_init.opcode = CMD_OPCODE_Q_INIT;
    ctx.cmd.q_init.lif_index = (__le16)cpu_to_le16(lif->index);
    ctx.cmd.q_init.type = (u8)q->type;
    ctx.cmd.q_init.index = cpu_to_le32(q->index);
    ctx.cmd.q_init.pid = (__le16)cpu_to_le16(q->pid);
    ctx.cmd.q_init.ring_size = (u8)ilog2(q->num_descs);
    ctx.cmd.q_init.ring_base = cpu_to_le64(q->base_pa);
    ctx.cmd.q_init.cq_ring_base = cpu_to_le64(cq->base_pa);
    ctx.cmd.q_init.sg_ring_base = cpu_to_le64(q->sg_base_pa),

    //	if (lif->ionic->neth_eqs) {
    //		unsigned int eq_index = ionic_choose_eq(lif, q->index);

        //		ctx.cmd.q_init.flags = cpu_to_le16(IONIC_QINIT_F_EQ |
        //						   IONIC_QINIT_F_SG);
        //		ctx.cmd.q_init.intr_index = cpu_to_le16(eq_index);
        //	} else {
        ctx.cmd.q_init.flags =
            cpu_to_le16(IONIC_QINIT_F_IRQ | IONIC_QINIT_F_SG);
    ctx.cmd.q_init.intr_index = cpu_to_le16(cq->bound_intr->index);
    //	}

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE, "%s rxq_init.pid %d\n",
              __FUNCTION__, ctx.cmd.q_init.pid));
    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s rxq_init.index %d\n", __FUNCTION__, ctx.cmd.q_init.index));
    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s rxq_init.ring_base 0x%llx\n", __FUNCTION__,
              ctx.cmd.q_init.ring_base));
    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s rxq_init.ring_size %d\n", __FUNCTION__,
              ctx.cmd.q_init.ring_size));

    status = ionic_adminq_post_wait(lif, &ctx);
    if (status != NDIS_STATUS_SUCCESS)
        return status;

    q->hw_type = ctx.comp.q_init.hw_type;
    q->hw_index = le32_to_cpu(ctx.comp.q_init.hw_index);
    q->dbval = IONIC_DBELL_QID(q->hw_index);

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE, "%s rxq->hw_type %d\n",
              __FUNCTION__, q->hw_type));
    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s rxq->hw_index %d\n", __FUNCTION__, q->hw_index));

    qcq->flags |= QCQ_F_INITED;

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
ionic_lif_txq_init(struct lif *lif, struct qcqst *qcqst)
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    struct qcq *qcq = qcqst->qcq;
    struct queue *q = &qcq->q;
    struct cq *cq = &qcq->cq;
    struct ionic_admin_ctx ctx = {0};

    ctx.cmd.q_init.opcode = CMD_OPCODE_Q_INIT;
    ctx.cmd.q_init.lif_index = (__le16)cpu_to_le16(lif->index);
    ctx.cmd.q_init.type = (u8)q->type;
    ctx.cmd.q_init.index = cpu_to_le32(q->index);
    ctx.cmd.q_init.flags = cpu_to_le16(IONIC_QINIT_F_IRQ | IONIC_QINIT_F_SG);
    ctx.cmd.q_init.intr_index =
        (__le16)cpu_to_le16(lif->txqcqs[q->index].qcq->intr.index);
    ctx.cmd.q_init.pid = (__le16)cpu_to_le16(q->pid);
    ctx.cmd.q_init.ring_size = (u8)ilog2(q->num_descs);
    ctx.cmd.q_init.ring_base = cpu_to_le64(q->base_pa);
    ctx.cmd.q_init.cq_ring_base = cpu_to_le64(cq->base_pa);
    ctx.cmd.q_init.sg_ring_base = cpu_to_le64(q->sg_base_pa);

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE, "%s txq_init.pid %d\n",
              __FUNCTION__, ctx.cmd.q_init.pid));
    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s txq_init.index %d\n", __FUNCTION__, ctx.cmd.q_init.index));
    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s txq_init.ring_base 0x%llx\n", __FUNCTION__,
              ctx.cmd.q_init.ring_base));
    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s txq_init.ring_size %d\n", __FUNCTION__,
              ctx.cmd.q_init.ring_size));

    status = ionic_adminq_post_wait(lif, &ctx);
    if (status != NDIS_STATUS_SUCCESS)
        return status;

    q->hw_type = ctx.comp.q_init.hw_type;
    q->hw_index = le32_to_cpu(ctx.comp.q_init.hw_index);
    q->dbval = IONIC_DBELL_QID(q->hw_index);

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE, "%s txq->hw_type %d\n",
              __FUNCTION__, q->hw_type));
    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s txq->hw_index %d\n", __FUNCTION__, q->hw_index));

    qcq->flags |= QCQ_F_INITED;

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
ionic_lif_rxq_pkt_init(struct qcq *qcq)
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    u32 ring_index = 0;
    void *dma_entry_addr = NULL;
    u32 data_offset = 0;
    struct rxq_pkt *rxq_pkt = NULL;
    SCATTER_GATHER_LIST *pSGList = NULL;
    ULONG sg_count = 0;
    ULONG sg_index = 0;
    ULONG rxq_sg_index = 0;
    SCATTER_GATHER_ELEMENT *pSrcElem = NULL;
    ULONGLONG ullSrcPA = 0;
    ULONG ulSrcLen = 0;

    pSGList = (SCATTER_GATHER_LIST *)qcq->sg_buffer;

    dma_entry_addr = qcq->netbuffer_base;
    data_offset = 0;
    ring_index = 0;

    ASSERT((qcq->netbuffer_elementsize % PAGE_SIZE) == 0);
    sg_count = qcq->netbuffer_elementsize / PAGE_SIZE;
    sg_index = 0;

    rxq_pkt = (struct rxq_pkt *)qcq->pkts_base.rxq_base;

    pSrcElem = &pSGList->Elements[0];
    ullSrcPA = pSrcElem->Address.QuadPart;
    ulSrcLen = pSrcElem->Length;

    while (ring_index < qcq->q.rx_pkt_cnt) {

        rxq_pkt->sg_count = sg_count;
        rxq_pkt->addr = dma_entry_addr;

        rxq_sg_index = 0;
        while (rxq_sg_index < sg_count) {
            ASSERT(pSrcElem != NULL);
            ASSERT((pSrcElem->Length % PAGE_SIZE) == 0);

            rxq_pkt->phys_addr[rxq_sg_index] = ullSrcPA;

            ullSrcPA += PAGE_SIZE;
            ulSrcLen -= PAGE_SIZE;

            if (ulSrcLen == 0) {

                sg_index++;

                if (sg_index < pSGList->NumberOfElements) {

                    pSrcElem++;
                    ullSrcPA = pSrcElem->Address.QuadPart;
                    ulSrcLen = pSrcElem->Length;
                } else {

                    if (rxq_sg_index == sg_count - 1) {
                        break;
                    }

                    status = NDIS_STATUS_RESOURCES;
                    goto err_out;
                }
            }

            rxq_sg_index++;
        }

        rxq_pkt->offset = data_offset;

        dma_entry_addr =
            (void *)((char *)dma_entry_addr +
                        qcq->netbuffer_elementsize);
        data_offset += qcq->netbuffer_elementsize;

        rxq_pkt = rxq_pkt->next;

        ring_index++;
    }

    return NDIS_STATUS_SUCCESS;

err_out:

    return status;
}

NDIS_STATUS
ionic_lif_rxqs_init(struct lif *lif)
{
    unsigned int i;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    for (i = 0; i < lif->nrxqs; i++) {

        status = ionic_lif_rxq_init(lif, lif->rxqcqs[i].qcq);
        if (status != NDIS_STATUS_SUCCESS) {

            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                      "%s ionic_lif_rxq_init() Failed status %08lX\n",
                      __FUNCTION__, status));
            goto err_out;
        }

        status = ionic_rx_init(lif, &lif->rxqcqs[i]);
        if (status != NDIS_STATUS_SUCCESS) {

            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                        "%s ionic_rx_init() Failed status %08lX\n", __FUNCTION__,
                        status));
            goto err_out;
        }
    }

    return NDIS_STATUS_SUCCESS;

err_out:
    for (; i > 0; i--)
        ionic_lif_qcq_deinit(lif, lif->rxqcqs[i - 1].qcq);

    return status;
}

NDIS_STATUS
ionic_lif_txqs_init(struct lif *lif)
{
    unsigned int i;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    for (i = 0; i < lif->ntxqs; i++) {
        status = ionic_lif_txq_init(lif, &lif->txqcqs[i]);
        if (status != NDIS_STATUS_SUCCESS)
            goto err_out;
    }

    return NDIS_STATUS_SUCCESS;

err_out:
    for (; i > 0; i--)
        ionic_lif_qcq_deinit(lif, lif->txqcqs[i - 1].qcq);

    return status;
}
NDIS_STATUS
ionic_txrx_alloc(struct lif *lif,
                 ULONG vport_id,
                 ULONG queue_id,
                 GROUP_AFFINITY Affinity)
{
    unsigned int flags;
    unsigned int i;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    u32 len = 0;
    struct ionic *ionic = lif->ionic;
    struct interrupt_info *int_tbl = NULL;
    NDIS_SHARED_MEMORY_PARAMETERS stParams;
    ULONG ulSGListSize = 0;
    ULONG ulSGListNumElements = 0;
    PSCATTER_GATHER_LIST pSGListBuffer = NULL;
    PROCESSOR_NUMBER procNumber;
    u32 procMaskIndex = 0;
    u32 procIndex = 0;
    ULONG q_index = 0;
    BOOLEAN default_port = FALSE;

    if (queue_id == (ULONG)-1 && vport_id == (ULONG)-1) {
        q_index = 0;
        default_port = TRUE;
    } else if (queue_id != (ULONG)-1) {
        q_index = queue_id;
    } else {
        q_index = vport_id;
    }

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Ionic %p lif %p tx %d rx %d index %d Entry\n", __FUNCTION__,
              lif->ionic, lif, lif->ntxqs, lif->nrxqs, q_index));

    flags = QCQ_F_TX_STATS | QCQ_F_SG;
    for (i = 0; i < lif->ntxqs; i++) {

        procIndex = (ULONG)-1;
        status =
            ionic_qcq_alloc(lif, IONIC_QTYPE_TXQ, i, "tx", flags,
                            lif->ntxq_descs, sizeof(struct txq_desc),
                            sizeof(struct txq_comp), sizeof(struct txq_sg_desc),
                            lif->kern_pid, &lif->txqcqs[i].qcq, procIndex);
        if (status != NDIS_STATUS_SUCCESS) {

            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                      "%s Ionic %p Failed to alloc tx qcq. Status %08lX\n",
                      __FUNCTION__, ionic, status));
            goto err_out_free_txqcqs;
        }

        status = ionic_alloc_txq_pkts(lif->ionic, lif->txqcqs[i].qcq);

        if (status != NDIS_STATUS_SUCCESS) {
            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                      "%s Ionic %p Failed to alloc txq packets. Status %08lX\n",
                      __FUNCTION__, ionic, status));
            goto err_out_free_txqcqs;
        }

        status = ionic_tx_fill(lif->ionic, &lif->txqcqs[i]);

        if (status != NDIS_STATUS_SUCCESS) {
            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                      "%s Ionic %p Failed ionic_tx_fill. Status %08lX\n",
                      __FUNCTION__, ionic, status));
            goto err_out_free_txqcqs;
        }

        lif->txqcqs[i].qcq->tx_stats = lif->txqcqs[i].tx_stats;

        if (!is_master_lif(lif) &&
            (i == 0)) { // For now we will only store the 0 queue
            struct qcqst *txqs = lif->ionic->master_lif->txqcqs;

            status = ionic_link_master_qcq(lif->txqcqs[i].qcq, txqs);
            if (status != NDIS_STATUS_SUCCESS)
                goto err_out_free_txqcqs;
        }
    }

    flags = QCQ_F_RX_STATS | QCQ_F_INTR | QCQ_F_SG;
    for (i = 0; i < lif->nrxqs; i++) {

        //
        // If we were given a group affinity for this queue then try to allocate
        // that msi
        //

        procIndex = (ULONG)-1;

        if (q_index != 0 && Affinity.Mask != 0) {

            procMaskIndex = GetNextProcIndex(Affinity.Mask, procMaskIndex);
            procIndex = 0;

            if (procMaskIndex != INVALID_PROCESSOR_INDEX) {

                procNumber.Reserved = 0;
                procNumber.Group = Affinity.Group;
                procNumber.Number = (UCHAR)procMaskIndex;

                procIndex = KeGetProcessorIndexFromNumber(&procNumber);

                if (procIndex != INVALID_PROCESSOR_INDEX) {

                    if (procIndex >= ionic->interrupt_count) {
                        procIndex = (procIndex % ionic->interrupt_count);
                    }
                }
            } else {
                procMaskIndex = 0;
            }
        } else if (default_port &&
                   BooleanFlagOn(ionic->ConfigStatus, IONIC_RSS_ENABLED)) {

            // get the affinity from the indirection table
            procIndex = get_rss_affinity(lif, i);
        }

        status =
            ionic_qcq_alloc(lif, IONIC_QTYPE_RXQ, i, "rx", flags,
                            lif->nrxq_descs, sizeof(struct rxq_desc),
                            sizeof(struct rxq_comp), sizeof(struct rxq_sg_desc),
                            lif->kern_pid, &lif->rxqcqs[i].qcq, procIndex);
        if (status != NDIS_STATUS_SUCCESS) {
            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                      "%s Ionic %p Failed to alloc rx qcq. Status %08lX\n",
                      __FUNCTION__, ionic, status));
            goto err_out_free_rxqcqs;
        }

        int_tbl = get_interrupt(lif->ionic, lif->rxqcqs[i].qcq->intr.index);

        ASSERT(int_tbl != NULL);

        int_tbl->rx_id = i;

        int_tbl->msi_id = lif->rxqcqs[i].qcq->intr.index;

        lif->rxqcqs[i].rx_stats->msi_id = lif->rxqcqs[i].qcq->intr.index;

        IoPrint("%s Rx %d on core %d %d\n", __FUNCTION__, i, procIndex,
                int_tbl->original_proc);

        if ((procIndex != (ULONG)-1 && int_tbl->current_proc != procIndex) ||
            (procIndex == (ULONG)-1 &&
             int_tbl->current_proc != int_tbl->original_proc)) {

            if (procIndex == (ULONG)-1) {

                DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                          "%s Ionic %p Rx Id %d Remapped %d to %d curr %d\n",
                          __FUNCTION__, ionic, i,
                          lif->rxqcqs[i].qcq->intr.index,
                          int_tbl->original_proc, int_tbl->current_proc));

                int_tbl->current_proc = int_tbl->original_proc;
            } else {

                DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                          "%s Ionic %p Rx Id %d Mapped %d to %d curr %d\n",
                          __FUNCTION__, ionic, i,
                          lif->rxqcqs[i].qcq->intr.index, procIndex,
                          int_tbl->current_proc));

                int_tbl->current_proc = procIndex;

                KeGetProcessorNumberFromIndex(procIndex, &procNumber);

                int_tbl->group = procNumber.Group;
                int_tbl->group_proc = procNumber.Number;

                SetFlag(int_tbl->Flags, IONIC_TARGET_PROC_CHANGED);
            }
        }

        int_tbl->queue_id = (USHORT)q_index;
        lif->rxqcqs[i].qcq->queue_id = q_index;

        len = lif->ionic->frame_size;

        len = ((len / PAGE_SIZE) + 1) * PAGE_SIZE;

        lif->rxqcqs[i].qcq->netbuffer_length =
            len * lif->rxqcqs[i].qcq->q.rx_pkt_cnt;

        ASSERT(lif->rxqcqs[i].qcq->netbuffer_length != 0);

        if (lif->rxqcqs[i].qcq->netbuffer_length != 0) {

            NdisZeroMemory(&stParams, sizeof(NDIS_SHARED_MEMORY_PARAMETERS));

            stParams.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
            stParams.Header.Revision = NDIS_SHARED_MEMORY_PARAMETERS_REVISION_2;
            stParams.Header.Size =
                NDIS_SIZEOF_SHARED_MEMORY_PARAMETERS_REVISION_2;

            stParams.Flags = 0; // NDIS_SHARED_MEM_PARAMETERS_CONTIGOUS;

            if (queue_id != (ULONG)-1) {
                stParams.QueueId = queue_id;
            } else {
                stParams.VPortId = vport_id;
            }

            stParams.Usage = NdisSharedMemoryUsageReceive;

            stParams.PreferredNode = ionic->numa_node; //MM_ANY_NODE_OK;

            stParams.Length = lif->rxqcqs[i].qcq->netbuffer_length;

            ulSGListNumElements = BYTES_TO_PAGES(stParams.Length);

            ulSGListSize =
                sizeof(SCATTER_GATHER_LIST) +
                (sizeof(SCATTER_GATHER_ELEMENT) * ulSGListNumElements);
            pSGListBuffer =
                (PSCATTER_GATHER_LIST)NdisAllocateMemoryWithTagPriority_internal(
                    ionic->adapterhandle, ulSGListSize, IONIC_SG_LIST_TAG,
                    NormalPoolPriority);

            if (pSGListBuffer == NULL) {
                status = NDIS_STATUS_RESOURCES;
                goto err_out_free_rxqcqs;
            }

            NdisZeroMemory(pSGListBuffer, ulSGListSize);

            DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                  "%s Allocated 0x%p len %08lX\n",
                  __FUNCTION__,
                  pSGListBuffer,
                  ulSGListSize));

            lif->rxqcqs[i].qcq->sg_buffer = (void *)pSGListBuffer;

            pSGListBuffer->NumberOfElements = ulSGListNumElements;

            stParams.SGListBufferLength = ulSGListSize;
            stParams.SGListBuffer = pSGListBuffer;

            status = NdisAllocateSharedMemory(
                ionic->adapterhandle, &stParams,
                &lif->rxqcqs[i].qcq->RxBufferAllocHandle);

            if (status != NDIS_STATUS_SUCCESS) {
                ASSERT(FALSE);
                status = NDIS_STATUS_RESOURCES;
                goto err_out_free_rxqcqs;
            }

            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                      "%s Rx VA %p Handle %p RxPool Length 0x%08lX\n",
                      __FUNCTION__, stParams.VirtualAddress,
                      stParams.SharedMemoryHandle,
                      lif->rxqcqs[i].qcq->netbuffer_length));

            NdisZeroMemory(stParams.VirtualAddress,
                           lif->rxqcqs[i].qcq->netbuffer_length);

			ASSERT( pSGListBuffer->NumberOfElements == 1);

            lif->rxqcqs[i].qcq->RxBufferHandle = stParams.SharedMemoryHandle;
            lif->rxqcqs[i].qcq->netbuffer_base = stParams.VirtualAddress;
            lif->rxqcqs[i].qcq->netbuffer_elementsize = len;
        }

        lif->rxqcqs[i].qcq->rx_stats = lif->rxqcqs[i].rx_stats;

        ionic_link_qcq_interrupts(lif->rxqcqs[i].qcq, lif->txqcqs[i].qcq);

        if (!is_master_lif(lif) &&
            (i == 0)) { // For now we will only store the 0 queue
            struct qcqst *rxqs = lif->ionic->master_lif->rxqcqs;

            status = ionic_link_master_qcq(lif->rxqcqs[i].qcq, rxqs);
            if (status != NDIS_STATUS_SUCCESS)
                goto err_out_free_rxqcqs;
        }

        status = ionic_alloc_rxq_pkts(lif->ionic, lif->rxqcqs[i].qcq);

        if (status != NDIS_STATUS_SUCCESS) {
            goto err_out_free_rxqcqs;
        }

        status = ionic_lif_rxq_pkt_init(lif->rxqcqs[i].qcq);
        if (status != NDIS_STATUS_SUCCESS) {
            goto err_out_free_rxqcqs;
        }

        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                  "%s Lif %s Index %d Cnt %d free cnt %d\n", __FUNCTION__,
                  lif->name, i, lif->rxqcqs[i].qcq->q.rx_pkt_cnt,
                  lif->rxqcqs[i].qcq->pkts_free_count));
    }

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Ionic %p lif %p Complete\n", __FUNCTION__, lif->ionic, lif));

    return NDIS_STATUS_SUCCESS;

err_out_free_rxqcqs:
    for (i = 0; i < lif->nrxqs; i++) {
        if (lif->rxqcqs[i].qcq != NULL) {
            ionic_free_rxq_pkts(lif->ionic, lif->rxqcqs[i].qcq);
            ionic_qcq_free(lif, lif->rxqcqs[i].qcq);
			lif->rxqcqs[i].qcq = NULL;
        }
    }

err_out_free_txqcqs:
    for (i = 0; i < lif->ntxqs; i++) {
        if (lif->txqcqs[i].qcq != NULL) {
            ionic_free_txq_pkts(lif->ionic, lif->txqcqs[i].qcq);
            ionic_qcq_free(lif, lif->txqcqs[i].qcq);
			lif->txqcqs[i].qcq = NULL;
        }
    }

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
              "%s Ionic %p lif %p Complete failed status %08lX\n", __FUNCTION__,
              lif->ionic, lif, status));
    return status;
}

static void
ionic_slaves_stop(struct ionic *ionic)
{

    LIST_ENTRY *cur;
    struct lif *lif;

    if (!IsListEmpty(&ionic->lifs)) {

        cur = ionic->lifs.Flink;

        do {
            lif = CONTAINING_RECORD(cur, struct lif, list);

            if (!is_master_lif(lif))
                ionic_lif_stop(lif);

            cur = cur->Flink;
        } while (cur != &ionic->lifs);
    }

    return;
}

static void
ionic_txrx_disable(struct lif *lif)
{
    unsigned int i;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    for (i = 0; i < lif->ntxqs; i++) {
        status = ionic_qcq_disable(lif->txqcqs[i].qcq);
        if (status != NDIS_STATUS_SUCCESS) {
            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR, "%s Failed to disable tx queue %d lif %s Status %08lX\n",
              __FUNCTION__, i, lif->name, status));
        }
        ionic_tx_release_pending(lif->ionic, lif->txqcqs[i].qcq);
    }
    for (i = 0; i < lif->nrxqs; i++) {
        status = ionic_qcq_disable(lif->rxqcqs[i].qcq);

        if (status != NDIS_STATUS_SUCCESS) {
            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR, "%s Failed to disable rx queue %d lif %s Status %08lX\n",
                __FUNCTION__, i, lif->name, status));
        }
    }
    return;
}

static int
ionic_lif_quiesce(struct lif *lif)
{
    struct ionic_admin_ctx ctx = {0};
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    ctx.cmd.lif_setattr.opcode = CMD_OPCODE_LIF_SETATTR;
    ctx.cmd.lif_setattr.attr = IONIC_LIF_ATTR_STATE;
    ctx.cmd.lif_setattr.index = (__le16)lif->index;
    ctx.cmd.lif_setattr.state = IONIC_LIF_DISABLE;

    status = ionic_adminq_post_wait(lif, &ctx);
    if (status != NDIS_STATUS_SUCCESS) {
        return status;
    }

    lif->flags &= ~IONIC_LIF_INITED;

    return 0;
}

static void
ionic_txrx_deinit(struct lif *lif)
{
    unsigned int i;

    for (i = 0; i < lif->ntxqs; i++) {
        ionic_lif_qcq_deinit(lif, lif->txqcqs[i].qcq);
        ionic_tx_flush(lif->txqcqs[i].qcq, true, false);
        ionic_tx_empty(&lif->txqcqs[i].qcq->q);

        ionic_reset_qcq( lif->txqcqs[i].qcq);
    }

    for (i = 0; i < lif->nrxqs; i++) {
        ionic_lif_qcq_deinit(lif, lif->rxqcqs[i].qcq);
        ionic_rx_flush(&lif->rxqcqs[i].qcq->cq);
        ionic_rx_empty(&lif->rxqcqs[i].qcq->q);

        ionic_reset_rxq_pkts( lif->rxqcqs[i].qcq);
        ionic_reset_qcq( lif->rxqcqs[i].qcq);
    }
}

NDIS_STATUS
ionic_lif_stop(struct lif *lif)
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    if (!RtlCheckBit(&lif->state, LIF_UP)) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                  "%s: %s state=DOWN\n", __FUNCTION__, lif->name));
        return NDIS_STATUS_SUCCESS;
    }
    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE, "%s: %s state=UP\n",
              __FUNCTION__, lif->name));
    RtlClearBit(&lif->state, LIF_UP);

    NdisCancelTimerObject(lif->ionic->WatchDogTimer);

    ionic_txrx_disable(lif);
    ionic_lif_quiesce(lif);

    ionic_txrx_deinit(lif);
    // ionic_txrx_free(lif);

    return status;
}
NDIS_STATUS
ionic_stop(struct ionic *ionic)
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    struct lif *lif = ionic->master_lif;
    BOOLEAN set_event = FALSE;

    // Sync behind any perfmon events coming in
    status = KeWaitForSingleObject(&perfmon_event,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
    if (status == STATUS_SUCCESS) {
        set_event = TRUE;
    }

    ionic->hardware_status = NdisHardwareStatusNotReady;
    ionic_indicate_status(ionic, NDIS_STATUS_MEDIA_DISCONNECT, 0, 0);

    // Wait for any outstanding requests
    wait_on_requests( ionic);

    ionic_slaves_stop(lif->ionic);

    status = ionic_lif_stop(lif);

    if( set_event) {
        KeSetEvent( &perfmon_event, 0, FALSE);
    }

    return status;
}

static void
ionic_lif_tx_flush(struct lif *lif)
{
    unsigned int i;

    for (i = 0; i < lif->ntxqs; i++) {
        ionic_tx_flush(lif->txqcqs[i].qcq, false, false);
        ionic_service_pending_nbl_requests(lif->ionic, lif->txqcqs[i].qcq);
    }

    return;
}

void
ionic_flush(struct ionic *ionic)
{

    LIST_ENTRY *cur;
    struct lif *lif;

    if (!IsListEmpty(&ionic->lifs)) {

        cur = ionic->lifs.Flink;

        do {
            lif = CONTAINING_RECORD(cur, struct lif, list);

            ionic_lif_tx_flush(lif);

            cur = cur->Flink;
        } while (cur != &ionic->lifs);
    }
    return;
}

static int
ionic_txrx_init(struct lif *lif)
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    status = ionic_lif_txqs_init(lif);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s ionic_lif_txqs_init() Failed status %08lX\n",
                  __FUNCTION__, status));
        return status;
    }

    status = ionic_lif_rxqs_init(lif);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s ionic_lif_rxqs_init() Failed status %08lX\n",
                  __FUNCTION__, status));
        goto err_out;
    }

    if (is_master_lif(lif))
        ionic_lif_rss_init(lif);

    status = ionic_set_rx_mode(lif, RX_MODE_F_UNICAST | RX_MODE_F_MULTICAST |
                                        RX_MODE_F_BROADCAST);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s ionic_set_rx_mode() Failed status %08lX\n", __FUNCTION__,
                  status));
        goto err_out;
    }

    return NDIS_STATUS_SUCCESS;

err_out:

    return status;
}

NDIS_STATUS
ionic_lif_open(struct lif *lif,
               ULONG vport_id,
               ULONG queue_id,
               GROUP_AFFINITY Affinity)
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    status = ionic_txrx_alloc(lif, vport_id, queue_id, Affinity);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s ionic_txrx_alloc() Failed status %08lX\n", __FUNCTION__,
                  status));
        return status;
    }

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
ionic_open(struct ionic *ionic)
{
    struct lif *lif = ionic->master_lif;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    GROUP_AFFINITY affinity;

    affinity.Group = 0;
    affinity.Mask = 0;

    status = ionic_lif_open(lif, (ULONG)-1, (ULONG)-1, affinity);

    if (status != NDIS_STATUS_SUCCESS) {
        KeSetEvent( &perfmon_event, 0, FALSE);
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s ionic_lif_open() Failed status %08lX\n", __FUNCTION__,
                  status));

        return status;
    }

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
ionic_lif_start(struct lif *lif)
{
    unsigned int i;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    status = ionic_txrx_init(lif);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s ionic_txrx_init() Failed status %08lX\n", __FUNCTION__,
                  status));
        goto err_out;
    }

    for (i = 0; i < lif->ntxqs; i++) {

        status = ionic_qcq_enable(lif->txqcqs[i].qcq);
        if (status != NDIS_STATUS_SUCCESS) {

            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                      "%s ionic_qcq_enable() Failed status %08lX\n",
                      __FUNCTION__, status));

            goto err_out;
        }
    }

    for (i = 0; i < lif->nrxqs; i++) {

        status = ionic_qcq_enable(lif->rxqcqs[i].qcq);
        if (status != NDIS_STATUS_SUCCESS) {

            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                      "%s ionic_qcq_enable() Failed status %08lX\n",
                      __FUNCTION__, status));
            goto err_out;
        }
    }

    RtlSetBit(&lif->state, LIF_UP);

    return NDIS_STATUS_SUCCESS;

err_out:
    ionic_lif_stop(lif);
    return status;
}

NDIS_STATUS
ionic_start(struct ionic *ionic)
{
    struct lif *lif = ionic->master_lif;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    BOOLEAN set_event = FALSE;
    LIST_ENTRY *cur;

    // Sync behind any perfmon events coming in
    status = KeWaitForSingleObject(&perfmon_event,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
    if (status == STATUS_SUCCESS) {
        set_event = TRUE;
    }

    if (!IsListEmpty(&ionic->lifs)) {

        cur = ionic->lifs.Flink;

        do {
            lif = CONTAINING_RECORD(cur, struct lif, list);

            status = ionic_lif_start(lif);
            if (status != NDIS_STATUS_SUCCESS) {
                goto exit;
            }

            cur = cur->Flink;
        } while (cur != &ionic->lifs);
    }

    LARGE_INTEGER liDelay;
    liDelay.QuadPart = -(100);

    NdisSetTimerObject( ionic->WatchDogTimer,
                        liDelay,
                        1000,
                        (void *)ionic);

    ionic->hardware_status = NdisHardwareStatusReady;
    ionic_indicate_status(ionic, NDIS_STATUS_MEDIA_CONNECT, 0, 0);

exit:

    if (status != NDIS_STATUS_SUCCESS) {
        ionic_stop(ionic);
    }

    if( set_event) {
        KeSetEvent( &perfmon_event, 0, FALSE);
    }

    return status;
}

void
ionic_lif_deinit(struct lif *lif)
{
    if (!RtlCheckBit(&lif->state, LIF_INITED))
        return;

    RtlClearBit(&lif->state, LIF_INITED);

    ionic_rx_filters_deinit(lif);

    if (is_master_lif(lif)) // && lif->netdev->features & NETIF_F_RXHASH)
        ionic_lif_rss_deinit(lif);

    ionic_lif_qcq_deinit(lif, lif->notifyqcq);
    ionic_lif_qcq_deinit(lif, lif->adminqcq);

    ionic_lif_reset(lif);
}

void
ionic_lifs_deinit(struct ionic *ionic)
{
    LIST_ENTRY *cur;
    struct lif *lif;

    if (!IsListEmpty(&ionic->lifs)) {

        cur = ionic->lifs.Flink;

        do {
            lif = CONTAINING_RECORD(cur, struct lif, list);
            ionic_lif_deinit(lif);
            cur = cur->Flink;
        } while (cur != &ionic->lifs);
    }

    return;
}

void
ionic_lifs_reset(struct ionic *ionic)
{
    LIST_ENTRY *cur;
    struct lif *lif;

    if (!IsListEmpty(&ionic->lifs)) {

        cur = ionic->lifs.Flink;

        do {
            lif = CONTAINING_RECORD(cur, struct lif, list);
			NdisAcquireSpinLock(&ionic->dev_cmd_lock);
			ionic_dev_cmd_lif_reset(&ionic->idev, 0);
			ionic_dev_cmd_wait(ionic, devcmd_timeout);
			NdisReleaseSpinLock(&ionic->dev_cmd_lock);
            cur = cur->Flink;
        } while (cur != &ionic->lifs);
    }

    return;
}

static int
ionic_slave_alloc(struct ionic *ionic)
{
    int index;

    /* slave index starts at 1, master_lif is 0 */
    index = RtlFindClearBitsAndSet(&ionic->lifbits, 1, 0);

    return index;
}

void
ionic_slave_free(struct ionic *ionic, int index)
{
    // if (index >= ionic->nslaves)
    //	return;
    RtlClearBit(&ionic->lifbits, index);
}

void *
ionic_allocate_slave_lif(struct ionic *ionic)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    union lif_identity *lid;
    struct lif *lif;
    int lif_index;
    int nqueues;

    //
    // Check we can support additional lifs
    //

    if (ionic->ident.dev.nlifs - ionic->total_lif_count == 0) {
        DbgTrace(
            (TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
             "%s No additional lifs support for device Cnt %d allocated %d\n",
             __FUNCTION__, ionic->ident.dev.nlifs, ionic->total_lif_count));
        return NULL;
    }

    /* For now, we need to assure we don't try to set up for multiqueue
     * macvlan channels.  Sometime in the future this will help us set
     * up for those multiqueue channels.
     */

    lid = (union lif_identity *)NdisAllocateMemoryWithTagPriority_internal(
        ionic->adapterhandle, sizeof(union lif_identity), IONIC_LIF_ID_TAG,
        NormalPoolPriority);
    if (!lid)
        return NULL;

    NdisZeroMemory(lid, sizeof(union lif_identity));

    DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                  "%s Allocated 0x%p len %08lX\n",
                  __FUNCTION__,
                  lid,
                  sizeof(union lif_identity)));

    ionic_lif_identify(ionic, IONIC_LIF_TYPE_MACVLAN, lid);
    nqueues = le32_to_cpu(lid->eth.config.queue_count[IONIC_QTYPE_RXQ]);

    if (nqueues > 1)
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                  "%s Only 1 queue used per slave LIF Cnt %d\n", __FUNCTION__,
                  nqueues));

    /* master_lif index is 0, slave index starts at 1 */
    lif_index = ionic_slave_alloc(ionic);
    if (lif_index == (ULONG)-1) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s No lifs available\n", __FUNCTION__));
        goto err_out_free_identify;
    }

    lif = ionic_lif_alloc(ionic, lif_index);
    if (lif == NULL) {
        ionic_slave_free(ionic, lif_index);
        goto err_out_free_identify;
    }
    lif->identity = lid;

    ntStatus = ionic_lif_init(lif);
    if (ntStatus != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Cannot init slave lif %d: %08lX\n", __FUNCTION__,
                  lif_index, ntStatus));
        goto err_out_free_slave;
    }

    ntStatus = ionic_lif_set_name(lif);

    if (ntStatus != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s ionic_lif_set_name failed %08lX\n", __FUNCTION__,
                  ntStatus));
    }

    lif->lif_stats->lif_id = lif_index;
    SetFlag(lif->lif_stats->flags, IONIC_LIF_FLAG_ALLOCATED);

    return lif;

err_out_free_slave:
    ionic_lif_free(lif);
    lid = NULL; // Freed it above
err_out_free_identify:
    if (lid != NULL) {
        NdisFreeMemoryWithTagPriority_internal(ionic->adapterhandle, lid,
                                      IONIC_LIF_ID_TAG);
    }

    return NULL;
}
static inline u32
ionic_coal_usec_to_hw(struct ionic *ionic, u32 usecs)
{
    u32 mult = le32_to_cpu(ionic->ident.dev.intr_coal_mult);
    u32 div = le32_to_cpu(ionic->ident.dev.intr_coal_div);

    /* Div-by-zero should never be an issue, but check anyway */
    if (!div || !mult)
        return 0;

    /* Round up in case usecs is close to the next hw unit */
    usecs += (div / mult) >> 1;

    /* Convert from usecs to device units */
    return (usecs * mult) / div;
}

NDIS_STATUS
ionic_set_coalesce(struct ionic *ionic, BOOLEAN Enable)
{
    struct identity *ident;
    unsigned int i;
    u32 coal = 0;

    ident = &ionic->ident;
    if (ident->dev.intr_coal_div == 0) {

        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Bad div value %d\n", __FUNCTION__,
                  ident->dev.intr_coal_div));
        return NDIS_STATUS_INVALID_PARAMETER;
    }

    /* Convert the usec request to a HW useable value.  If they asked
     * for non-zero and it resolved to zero, bump it up
     */
    if (Enable) {
        coal = ionic_coal_usec_to_hw(ionic, IONIC_DEFAULT_INT_COAL_US);
    }

    if (!coal && Enable)
        coal = 1;
    if (coal > INTR_CTRL_COAL_MAX)
        return NDIS_STATUS_INVALID_PARAMETER;

    if (RtlCheckBit(&ionic->master_lif->state, LIF_UP)) {
        for (i = 0; i < ionic->master_lif->nrxqs; i++) {
            ionic_intr_coal_init(ionic->master_lif->ionic->idev.intr_ctrl,
                                 ionic->master_lif->rxqcqs[i].qcq->intr.index,
                                 coal);
        }
    }

    return NDIS_STATUS_SUCCESS;
}
