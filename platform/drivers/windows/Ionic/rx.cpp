
#include "common.h"

static bool ionic_rx_clean(struct queue *q,
                           struct desc_info *desc_info,
                           struct cq_info *cq_info,
                           void *cb_arg,
                           void *packets_to_indicate,
                           void *last_packet);

static ULONG_PTR ndis_hash_type[16] = {
    0,
    NDIS_HASH_IPV4,     // 1
    NDIS_HASH_TCP_IPV4, // 2
#if (NDIS_SUPPORT_NDIS680)
    NDIS_HASH_UDP_IPV4, // 3
#endif
    NDIS_HASH_IPV6,     // 4
    NDIS_HASH_TCP_IPV6, // 5
#if (NDIS_SUPPORT_NDIS680)
    NDIS_HASH_UDP_IPV6, // 6
#endif
    NDIS_HASH_IPV6_EX,     // 7 Not supported
    NDIS_HASH_TCP_IPV6_EX, // 8 Not supported
#if (NDIS_SUPPORT_NDIS680)
    NDIS_HASH_UDP_IPV6_EX, // 9
#endif
    0, // 10
    0, // 11
    0, // 12
    0, // 13
    0, // 14
    0  // 15
};

static const char *ndis_hash_type_str[16] = {
    "NONE",
    "NDIS_HASH_IPV4",        // 1
    "NDIS_HASH_TCP_IPV4",    // 2
    "NDIS_HASH_UDP_IPV4",    // 3 UDP_IPv4 only in 6.8 NDIS
    "NDIS_HASH_IPV6",        // 4
    "NDIS_HASH_TCP_IPV6",    // 5
    "NDIS_HASH_UDP_IPV6",    // 6  UDP_IPv6 only in 6.8 NDIS
    "NDIS_HASH_IPV6_EX",     // 7 Not supported
    "NDIS_HASH_TCP_IPV6_EX", // 8 Not supported
    "NDIS_HASH_UDP_IPV6_EX", // 9 UDP IPv6 only in 6.8 NDIS
    "UNKNOWN",               // 10
    "UNKNOWN",               // 11
    "UNKNOWN",               // 12
    "UNKNOWN",               // 13
    "UNKNOWN",               // 14
    "UNKNOWN"                // 15
};

static u8
getmappedtype(u8 color_type)
{

    u8 type = 0;
    u8 pkt_type = (color_type & IONIC_RXQ_COMP_PKT_TYPE_MASK);

    switch (pkt_type) {
    case PKT_TYPE_NON_IP:
        type = 0;
        break;
    case PKT_TYPE_IPV4:
        type = 1;
        break;

    case PKT_TYPE_IPV4_TCP:
        type = 2;
        break;

    case PKT_TYPE_IPV4_UDP:
        type = 3;
        break;

    case PKT_TYPE_IPV6:
        type = 4;
        break;

    case PKT_TYPE_IPV6_TCP:
        type = 5;
        break;

    case PKT_TYPE_IPV6_UDP:
        type = 6;
        break;
    }

    return type;
}

static inline void
ionic_rxq_post(struct queue *q, bool ring_dbell, desc_cb cb_func, void *cb_arg)
{
    ionic_q_post(q, ring_dbell, cb_func, cb_arg);
}

void
ionic_reset_rxq_pkts(struct qcq *qcq)
{
    struct rxq_pkt *rxq_pkt = NULL;
    struct queue *q = &qcq->q;
    ULONG sg_slots = 0;
    ULONG rxq_pkt_len = 0;

    sg_slots = (qcq->netbuffer_elementsize / PAGE_SIZE);
    ASSERT(sg_slots != 0);
    sg_slots--; // The one entry built into the rxq_pkt

    rxq_pkt_len = (sizeof(struct rxq_pkt) + (sg_slots * sizeof(u64)));

    rxq_pkt = (struct rxq_pkt *)qcq->pkts_base.rxq_base;

    /* First, reset the entries in the list */
    qcq->rx_pkt_list_head = NULL;
    qcq->rx_pkt_list_tail = NULL;
    qcq->pkts_free_count = 0;

    for (unsigned int i = 0; i < q->rx_pkt_cnt; i++) {

        rxq_pkt->next = NULL;

        if (qcq->rx_pkt_list_head == NULL) {
            qcq->rx_pkt_list_head = rxq_pkt;
        } else {
            qcq->rx_pkt_list_tail->next = rxq_pkt;
        }

        qcq->rx_pkt_list_tail = rxq_pkt;

        rxq_pkt = (struct rxq_pkt *)((char *)rxq_pkt + rxq_pkt_len);
        qcq->pkts_free_count++;
    }

    /* Now release any nbls associated to the entries */
    ionic_release_rxq_pkts( qcq);
}

void
ionic_release_rxq_pkts(struct qcq *qcq)
{
    struct rxq_pkt *rxq_pkt = NULL;

    ASSERT(qcq != NULL);

    rxq_pkt = (struct rxq_pkt *)qcq->pkts_base.rxq_base;

    for (unsigned int i = 0; i < qcq->q.rx_pkt_cnt; i++) {

        if (rxq_pkt == NULL) {
            break;
        }

        if (rxq_pkt->parent_nbl != NULL) {
            NdisFreeNetBufferList(rxq_pkt->parent_nbl);
            rxq_pkt->parent_nbl = NULL;
            rxq_pkt->packet = NULL;
        }

        rxq_pkt = rxq_pkt->next;
    }
}

void
ionic_free_rxq_pkts(struct ionic *ionic, struct qcq *qcq)
{

    ASSERT(ionic != NULL);
    ASSERT(qcq != NULL);

    if (qcq->pkts_base.rxq_base != NULL) {

        ionic_release_rxq_pkts(qcq);

        NdisFreeMemoryWithTagPriority_internal(
            ionic->adapterhandle, qcq->pkts_base.rxq_base, IONIC_RX_MEM_TAG);
        qcq->pkts_base.rxq_base = NULL;

        qcq->rx_pkt_list_head = NULL;
        qcq->rx_pkt_list_tail = NULL;
        qcq->pkts_free_count = 0;
    }

    if (qcq->pkts_pool != NULL) {
        NdisFreeNetBufferListPool(qcq->pkts_pool);
        qcq->pkts_pool = NULL;
    }

    NdisFreeSpinLock(&qcq->tx_ring_lock);
    NdisFreeSpinLock(&qcq->rx_ring_lock);
}

NDIS_STATUS
ionic_alloc_rxq_pkts(struct ionic *ionic, struct qcq *qcq)
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    NET_BUFFER_LIST_POOL_PARAMETERS pool_params;
    struct queue *q = &qcq->q;
    ULONG size;
    ULONG sg_slots = 0;
    struct rxq_pkt *rxq_pkt = NULL;
    ULONG rxq_len = 0;

    ASSERT(ionic != NULL);
    ASSERT(q != NULL);

    NdisZeroMemory(&pool_params, sizeof(NET_BUFFER_LIST_POOL_PARAMETERS));
    pool_params.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
    pool_params.Header.Revision = NET_BUFFER_LIST_POOL_PARAMETERS_REVISION_1;
    pool_params.Header.Size = sizeof(pool_params);
    pool_params.ProtocolId = 0;
    pool_params.ContextSize = 0;
    pool_params.fAllocateNetBuffer = TRUE;
    pool_params.PoolTag = IONIC_RX_MEM_TAG;

    qcq->pkts_pool =
        NdisAllocateNetBufferListPool(ionic->adapterhandle, &pool_params);

    if (qcq->pkts_pool == NULL) {
        // Error out
        status = NDIS_STATUS_RESOURCES;
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Failed to alloc rq packet pool adapter %p.\n",
                  __FUNCTION__, ionic));
        goto err_alloc_failed;
    }

    //
    // How many pages per descriptor
    //

    ASSERT((qcq->netbuffer_elementsize % PAGE_SIZE) == 0);
    sg_slots = (qcq->netbuffer_elementsize / PAGE_SIZE);
    ASSERT(sg_slots != 0);
    sg_slots--; // The one entry built into the rxq_pkt

    rxq_len = (sizeof(struct rxq_pkt) + (sg_slots * sizeof(u64)));

    size = q->rx_pkt_cnt * rxq_len;
    qcq->pkts_base.rxq_base =
        (struct rxq_pkt *)NdisAllocateMemoryWithTagPriority_internal(
            ionic->adapterhandle, size, IONIC_RX_MEM_TAG, NormalPoolPriority);

    if (qcq->pkts_base.rxq_base == NULL) {
        status = NDIS_STATUS_RESOURCES;
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Failed to alloc rq pkt adapter %p array size %d\n",
                  __FUNCTION__, ionic, size));
        goto err_alloc_failed;
    }

    NdisZeroMemory(qcq->pkts_base.rxq_base, size);

    DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                  "%s Allocated 0x%p len %08lX\n",
                  __FUNCTION__,
                  qcq->pkts_base.rxq_base, size));
    
    NdisAllocateSpinLock(&qcq->tx_ring_lock);
    NdisAllocateSpinLock(&qcq->rx_ring_lock);

    qcq->rx_pkt_list_head = NULL;
    qcq->rx_pkt_list_tail = NULL;
    qcq->pkts_free_count = 0;
    NdisAllocateSpinLock(&qcq->pkt_list_lock);

    rxq_pkt = (struct rxq_pkt *)qcq->pkts_base.rxq_base;

    for (unsigned int i = 0; i < q->rx_pkt_cnt; i++) {

        rxq_pkt->next = NULL;

        if (qcq->rx_pkt_list_head == NULL) {
            qcq->rx_pkt_list_head = rxq_pkt;
        } else {
            qcq->rx_pkt_list_tail->next = rxq_pkt;
        }

        qcq->rx_pkt_list_tail = rxq_pkt;

        rxq_pkt = (struct rxq_pkt *)((char *)rxq_pkt + rxq_len);
        qcq->pkts_free_count++;
    }

    return status;

err_alloc_failed:
    ionic_free_rxq_pkts(ionic, qcq);
    return status;
}

struct rxq_pkt *
ionic_get_next_rxq_pkt(struct qcq *qcq)
{
    struct rxq_pkt *rxq_pkt = NULL;

    NdisAcquireSpinLock(&qcq->pkt_list_lock);

    if (InterlockedDecrement(&qcq->pkts_free_count) != 0) {

        ASSERT(qcq->rx_pkt_list_head->next != NULL);

        rxq_pkt = qcq->rx_pkt_list_head;
        qcq->rx_pkt_list_head = qcq->rx_pkt_list_head->next;
    } else {
        InterlockedIncrement(&qcq->pkts_free_count);
    }

    NdisReleaseSpinLock(&qcq->pkt_list_lock);

    return rxq_pkt;
}

void
ionic_return_rxq_pkt(struct qcq *qcq, struct rxq_pkt *rxq_pkt)
{

    NdisAcquireSpinLock(&qcq->pkt_list_lock);

    rxq_pkt->next = NULL;
    qcq->rx_pkt_list_tail->next = rxq_pkt;
    qcq->rx_pkt_list_tail = rxq_pkt;

    InterlockedIncrement(&qcq->pkts_free_count);

    NdisReleaseSpinLock(&qcq->pkt_list_lock);

    return;
}

PNET_BUFFER
ionic_alloc_rxq_netbuffers(struct ionic *ionic,
                           struct qcqst *qcqst,
                           unsigned int size,
                           struct rxq_pkt *rxq_pkt)
{
    struct qcq *qcq = qcqst->qcq;
    struct queue *q = &qcq->q;
    PNET_BUFFER packet = NULL;
    PNET_BUFFER_LIST parent_nbl = NULL;
    PMDL mdl = NULL;

    ASSERT(ionic != NULL);
    ASSERT(qcq != NULL);
    ASSERT(rxq_pkt != NULL);

    mdl = NdisAllocateMdl(ionic->adapterhandle, rxq_pkt->addr, size);

    if (mdl == NULL) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Failed to Rxq alloc MDL adapter %p\n", __FUNCTION__,
                  ionic));

        goto err_mdl_alloc_failed;
    }

    // allocate a packet descriptor
    parent_nbl =
        NdisAllocateNetBufferAndNetBufferList(qcq->pkts_pool, 0, 0, mdl, 0, 0);

    if (parent_nbl == NULL) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Failed to alloc rq net buffer list adapter %p\n",
                  __FUNCTION__, ionic));
        goto err_nbl_alloc_failed;
    }

    packet = NET_BUFFER_LIST_FIRST_NB(parent_nbl);
    NET_BUFFER_FIRST_MDL(packet) = mdl;

    /* setup the rq_pkt */
    rxq_pkt->parent_nbl = parent_nbl;

    /* map the RQ this will be used on */
    rxq_pkt->q = q;

    rxq_pkt->packet = packet;

    rxq_pkt->filter_info.Value = 0;
    rxq_pkt->filter_info.FilteringInfo.FilterId = 0;
    rxq_pkt->filter_info.FilteringInfo.QueueVPortInfo.QueueId =
        (USHORT)qcq->queue_id;

    NET_BUFFER_LIST_INFO(rxq_pkt->parent_nbl, NetBufferListFilteringInfo) =
        rxq_pkt->filter_info.Value;

    rxq_pkt->nb_shared_memory_info.NextSharedMemorySegment = NULL;
    rxq_pkt->nb_shared_memory_info.SharedMemoryFlags = 0;
    rxq_pkt->nb_shared_memory_info.SharedMemoryHandle = qcq->RxBufferHandle;
    rxq_pkt->nb_shared_memory_info.SharedMemoryLength = size;
    rxq_pkt->nb_shared_memory_info.SharedMemoryOffset = rxq_pkt->offset;

    packet->SharedMemoryInfo = &rxq_pkt->nb_shared_memory_info;

    *(struct rxq_pkt **)NET_BUFFER_MINIPORT_RESERVED(packet) = rxq_pkt;

    return packet;

err_nbl_alloc_failed:
    NdisFreeMdl(mdl);

err_mdl_alloc_failed:

    return NULL;
}

static bool
ionic_rx_clean(struct queue *q,
               struct desc_info *desc_info,
               struct cq_info *cq_info,
               void *cb_arg,
               void *packets_to_indicate,
               void *last_packet)
{

    struct lif *lif = q->lif;
    struct ionic *ionic = lif->ionic;
    struct rxq_comp *comp = (struct rxq_comp *)cq_info->cq_desc;
    PNET_BUFFER packet = (PNET_BUFFER)cb_arg;
    PNET_BUFFER_LIST parent_nbl;
    struct rxq_pkt *rxq_pkt;
    NDIS_TCP_IP_CHECKSUM_NET_BUFFER_LIST_INFO csum_info;
    NDIS_NET_BUFFER_LIST_8021Q_INFO vlan_pri_info;
    PMDL mdl;
    u8 rss_type = 0;
    u32 rss_hash = 0;
    struct dev_rx_ring_stats *rx_stats = q_to_rx_dev_stats(q);
    ULONG comp_len = comp->len;
    ULONG comp_status = comp->status;
    ULONG q_index = 0;
    ULONG frame_type = 0;
    bool indicate_nbl = false;
    void *rx_buffer = NULL;
    ULONG zero_len = 0;

    UNREFERENCED_PARAMETER(desc_info);

    DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
              "%s Processing packet for lif %d\n", __FUNCTION__,
              q->lif->index));

    mdl = NET_BUFFER_FIRST_MDL(packet);

    rxq_pkt = *(struct rxq_pkt **)NET_BUFFER_MINIPORT_RESERVED(packet);

    frame_type = get_frame_type((void *)rxq_pkt->addr);

    parent_nbl = rxq_pkt->parent_nbl;

    NET_BUFFER_LIST_STATUS(parent_nbl) = NDIS_STATUS_SUCCESS;
    parent_nbl->SourceHandle = ionic->adapterhandle;

    DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
              "%s RXQ Process Pkts adapter %p NB: %p NBL %p Len 0x%08lX Pad %s "
              "status 0x%08lX \n",
              __FUNCTION__, ionic, packet, parent_nbl, comp_len,
              comp_len < IONIC_MINIMUM_RX_PACKET_LEN ? "Yes" : "No",
              comp_status));

    NET_BUFFER_LIST_NEXT_NBL(rxq_pkt->parent_nbl) = NULL;

    if (comp_status) {

        DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_ERROR,
                  "%s RXQ Packet Error adapter %p Queue %d Status 0x%0x\n",
                  __FUNCTION__, ionic, q->index, comp_status));
        NET_BUFFER_DATA_LENGTH(packet) = 0;
        NdisAdjustMdlLength(mdl, 0);

        goto cleanup;
    }

    if (*((PNET_BUFFER_LIST *)packets_to_indicate) != NULL) {
        NET_BUFFER_LIST_NEXT_NBL(*((PNET_BUFFER_LIST *)last_packet)) =
            rxq_pkt->parent_nbl;
    } else {
        *((PNET_BUFFER_LIST *)packets_to_indicate) = rxq_pkt->parent_nbl;
    }

    *((PNET_BUFFER_LIST *)last_packet) = rxq_pkt->parent_nbl;
    indicate_nbl = true;

    // we're indicating this back to the OS so ref count the adapter
    ref_request(ionic);

    for (u32 sg_index = 0; sg_index < rxq_pkt->sg_count; sg_index++) {
        DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
                    "%s Rxq packet %p sg%d PA %I64X\n", 
                    __FUNCTION__,
                    rxq_pkt,
                    sg_index,
                    rxq_pkt->phys_addr[sg_index]));
    }

    //
    // Check we meet the minimum size for the packet
    //

    if (comp_len < IONIC_MINIMUM_RX_PACKET_LEN) {
        zero_len = IONIC_MINIMUM_RX_PACKET_LEN - comp_len;
        rx_buffer = MmGetSystemAddressForMdlSafe(mdl, NormalPagePriority);

        DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
                    "%s Padding packet buffer %p offset %08lX len %08lX\n", 
                    __FUNCTION__,
                    rx_buffer,
                    comp_len,
                    zero_len));

        if (rx_buffer != NULL) {
            NdisZeroMemory((void *)((char *)rx_buffer + comp_len), zero_len);
        }
        comp_len = IONIC_MINIMUM_RX_PACKET_LEN;
    }

    rxq_pkt->bytes = comp_len;

    vlan_pri_info.Value = 0;

    if (BooleanFlagOn(comp->csum_flags, IONIC_RXQ_COMP_CSUM_F_VLAN)) {

        vlan_pri_info.TagHeader.UserPriority =
            ETH_GET_VLAN_PRIORITY(comp->vlan_tci);
        vlan_pri_info.TagHeader.VlanId = ETH_GET_VLAN_ID(comp->vlan_tci);

        DbgTrace(
            (TRACE_COMPONENT_VLAN_PRI, TRACE_LEVEL_VERBOSE,
             "%s Receive Packet VLAN adapter %p VLAN %d Priority %d Lif %d\n",
             __FUNCTION__, ionic, vlan_pri_info.TagHeader.VlanId,
             vlan_pri_info.TagHeader.UserPriority, q->lif->index));

        rx_stats->vlan_stripped++;
    }

    NDIS_SET_NET_BUFFER_LIST_VLAN_ID(parent_nbl,
                                     vlan_pri_info.TagHeader.VlanId);
    NDIS_SET_NET_BUFFER_LIST_PRIORITY(parent_nbl,
                                      vlan_pri_info.TagHeader.UserPriority);

    //
    // Check if we should have a vlan tag on this queue
    //

    NET_BUFFER_LIST_INFO(rxq_pkt->parent_nbl, NetBufferListFilteringInfo) =
        rxq_pkt->filter_info.Value;

    if (BooleanFlagOn(ionic->ConfigStatus, IONIC_SRIOV_MODE)) {
        q_index = NET_BUFFER_LIST_RECEIVE_FILTER_VPORT_ID(rxq_pkt->parent_nbl);

        if (BooleanFlagOn(ionic->SriovSwitch.Ports[q_index].Flags,
                          IONIC_VPORT_STATE_VLAN_FLTR_SET) &&
            vlan_pri_info.TagHeader.VlanId == 0) {
            NET_BUFFER_LIST_RECEIVE_QUEUE_ID(parent_nbl) = 0;
        } else if (ionic->SriovSwitch.Ports[q_index].filter_cnt == 0) {
            NET_BUFFER_LIST_RECEIVE_QUEUE_ID(parent_nbl) = 0;
        }
    } else {
        q_index = NET_BUFFER_LIST_RECEIVE_QUEUE_ID(rxq_pkt->parent_nbl);

        if (BooleanFlagOn(ionic->vm_queue[q_index].Flags,
                          IONIC_QUEUE_STATE_VLAN_FLTR_SET) &&
            vlan_pri_info.TagHeader.VlanId == 0) {
            NET_BUFFER_LIST_RECEIVE_QUEUE_ID(parent_nbl) = 0;
        } else if (ionic->vm_queue[q_index].active_filter_cnt == 0) {
            NET_BUFFER_LIST_RECEIVE_QUEUE_ID(parent_nbl) = 0;
        }
    }

    DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
              "%s Processing packet %p queue id %d-%d\n", __FUNCTION__, rxq_pkt,
              rxq_pkt->filter_info.FilteringInfo.QueueVPortInfo.QueueId,
              NET_BUFFER_LIST_RECEIVE_QUEUE_ID(parent_nbl)));

    NET_BUFFER_DATA_LENGTH(packet) = comp_len;

    NdisAdjustMdlLength(mdl, comp_len);

    NdisFlushBuffer(mdl, FALSE);

    csum_info.Value = 0;

    if ((comp->csum_flags & IONIC_RXQ_COMP_VALID_CSUM_FLAGS) != 0) {

        if (((comp->pkt_type_color & PKT_TYPE_IPV4) != 0 &&
             ionic->tcpv4_rx_state != NDIS_OFFLOAD_SET_OFF) ||
            ((comp->pkt_type_color & PKT_TYPE_IPV6) != 0 &&
             ionic->tcpv6_rx_state != NDIS_OFFLOAD_SET_OFF)) {

            if ((csum_info.Receive.TcpChecksumSucceeded = BooleanFlagOn(
                     comp->csum_flags, IONIC_RXQ_COMP_CSUM_F_TCP_OK)) != 0) {
                rx_stats->csum_tcp++;
            } else if ((csum_info.Receive.TcpChecksumFailed = BooleanFlagOn(
                            comp->csum_flags, IONIC_RXQ_COMP_CSUM_F_TCP_BAD)) !=
                       0) {
                rx_stats->csum_tcp_bad++;
            }
        }

        if (((comp->pkt_type_color & PKT_TYPE_IPV4) != 0 &&
             ionic->udpv4_rx_state != NDIS_OFFLOAD_SET_OFF) ||
            ((comp->pkt_type_color & PKT_TYPE_IPV6) != 0 &&
             ionic->udpv6_rx_state != NDIS_OFFLOAD_SET_OFF)) {
            if ((csum_info.Receive.UdpChecksumSucceeded = BooleanFlagOn(
                     comp->csum_flags, IONIC_RXQ_COMP_CSUM_F_UDP_OK)) != 0) {
                rx_stats->csum_udp++;
            } else if ((csum_info.Receive.UdpChecksumFailed = BooleanFlagOn(
                            comp->csum_flags, IONIC_RXQ_COMP_CSUM_F_UDP_BAD)) !=
                       0) {
                rx_stats->csum_udp_bad++;
            }
        }

        if ((comp->pkt_type_color & PKT_TYPE_IPV4) != 0 &&
            ionic->ipv4_rx_state != NDIS_OFFLOAD_SET_OFF) {
            if ((csum_info.Receive.IpChecksumSucceeded = BooleanFlagOn(
                     comp->csum_flags, IONIC_RXQ_COMP_CSUM_F_IP_OK)) != 0) {
                rx_stats->csum_ip++;
            } else if ((csum_info.Receive.IpChecksumFailed =
                            BooleanFlagOn(comp->csum_flags,
                                          IONIC_RXQ_COMP_CSUM_F_IP_BAD)) != 0) {
                rx_stats->csum_ip_bad++;
            }
        }

        DbgTrace((
            TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
            "%s Receive Packet adapter %p Checksum TcpChecksumFailed %s "
            "TcpChecksumSucceeded %s UdpChecksumFailed %s UdpChecksumSucceeded "
            "%s IpChecksumFailed %s IpChecksumSucceeded %s\n",
            __FUNCTION__, ionic,
            csum_info.Receive.TcpChecksumFailed ? "Yes" : "No",
            csum_info.Receive.TcpChecksumSucceeded ? "Yes" : "No",
            csum_info.Receive.UdpChecksumFailed ? "Yes" : "No",
            csum_info.Receive.UdpChecksumSucceeded ? "Yes" : "No",
            csum_info.Receive.IpChecksumFailed ? "Yes" : "No",
            csum_info.Receive.IpChecksumSucceeded ? "Yes" : "No"));
    } else {
        DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
                  "%s Receive Packet ADAPTER %p NO Checksum\n", __FUNCTION__,
                  ionic));
        rx_stats->csum_none++;
    }

    NET_BUFFER_LIST_INFO(parent_nbl, TcpIpChecksumNetBufferListInfo) =
        csum_info.Value;

    rss_type = getmappedtype((u8)comp->pkt_type_color);
    rss_hash = comp->rss_hash;

    if (rss_type != 0 && BooleanFlagOn(lif->rss_hash_flags,
                                       NDIS_RECEIVE_HASH_FLAG_ENABLE_HASH)) {
        NET_BUFFER_LIST_SET_HASH_VALUE(parent_nbl, rss_hash);
        NET_BUFFER_LIST_INFO(parent_nbl, NetBufferListHashInfo) =
            (PVOID)NDIS_RSS_HASH_INFO_FROM_TYPE_AND_FUNC(
                ndis_hash_type[rss_type], NdisHashFunctionToeplitz);

        DbgTrace((TRACE_COMPONENT_RSS_PROCESSING, TRACE_LEVEL_VERBOSE,
                  "%s Receive Packet adapter %p  rss flags 0x%08lX rss_type: 0x%08lX, rss_type_str "
                  "%s, rss_hash: 0x%08lX\n",
                  __FUNCTION__, ionic, lif->rss_hash_flags,
                  rss_type, ndis_hash_type_str[rss_type],
                  rss_hash));
    } else {
        DbgTrace((TRACE_COMPONENT_RSS_PROCESSING, TRACE_LEVEL_VERBOSE,
                  "%s Receive Packet adapter %p NO RSS rss flags 0x%08lX "
                  "rss_type: 0x%08lX, rss_hash: 0x%08lX\n",
                  __FUNCTION__, ionic, lif->rss_hash_flags,
                  rss_type, rss_hash));

        NET_BUFFER_LIST_INFO(parent_nbl, NetBufferListHashInfo) = 0;
        NET_BUFFER_LIST_SET_HASH_VALUE(parent_nbl, 0);
    }

    rx_stats->buffers_posted++;

    switch (frame_type) {

    case NDIS_PACKET_TYPE_BROADCAST: {
        rx_stats->bcast_packets++;
        rx_stats->bcast_bytes += comp_len;
        break;
    }

    case NDIS_PACKET_TYPE_MULTICAST: {
        rx_stats->mcast_packets++;
        rx_stats->mcast_bytes += comp_len;
        break;
    }

    default: {
        rx_stats->directed_packets++;
        rx_stats->directed_bytes += comp_len;
        break;
    }
    }

cleanup:

    return indicate_nbl;
}

static bool
ionic_rx_service(struct cq *cq,
                 struct cq_info *cq_info,
                 PNET_BUFFER_LIST *packets_to_indicate,
                 PNET_BUFFER_LIST *last_packet,
                 bool *indicate_nbl)
{
    struct rxq_comp *comp = (struct rxq_comp *)cq_info->cq_desc;
    struct queue *q = cq->bound_q;
    struct desc_info *desc_info;

    *indicate_nbl = false;

    if (!color_match(comp->pkt_type_color, cq->done_color))
        return false;

    /* check for empty queue */
    if (q->tail->index == q->head->index)
        return false;

    desc_info = q->tail;
    if (desc_info->index != le16_to_cpu(comp->comp_index))
        return false;

    q->tail = desc_info->next;

    *indicate_nbl =
        ionic_rx_clean(q, desc_info, cq_info, desc_info->cb_arg,
                       (void *)packets_to_indicate, (void *)last_packet);

    desc_info->cb = NULL;
    desc_info->cb_arg = NULL;

    return true;
}

static u32
ionic_rx_walk_cq(struct cq *rxcq,
                 u32 limit,
                 PNET_BUFFER_LIST *packets_to_indicate)
{
    u32 work_done = 0;
    PNET_BUFFER_LIST last_packet = NULL;
    bool indicate_nbl = FALSE;

    while (ionic_rx_service(rxcq, rxcq->tail, packets_to_indicate, &last_packet,
                            &indicate_nbl)) {
        if (rxcq->tail->last)
            rxcq->done_color = !rxcq->done_color;
        rxcq->tail = rxcq->tail->next;

        if (indicate_nbl) {
            if (++work_done >= limit)
                break;
        }
    }

    return work_done;
}

void
ionic_rx_flush(struct cq *cq)
{

    struct ionic_dev *idev = &cq->lif->ionic->idev;
    u32 work_done;
    PNET_BUFFER_LIST packets_to_indicate = NULL;

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Entry for lif %s budget %d\n", __FUNCTION__, cq->lif->name,
              cq->num_descs));

    work_done = ionic_rx_walk_cq(cq, cq->num_descs, &packets_to_indicate);

    if (work_done) {
        ionic_intr_credits(idev->intr_ctrl, cq->bound_intr->index, work_done,
                           IONIC_INTR_CRED_RESET_COALESCE);

        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                  "%s Returning %d packets\n", __FUNCTION__, work_done));

        ionic_return_packet((NDIS_HANDLE)cq->lif->ionic, packets_to_indicate,
                            0);
    }

    return;
}

#define RX_RING_DOORBELL_STRIDE ((1 << 2) - 1)

NDIS_STATUS
ionic_rx_init(struct lif *lif, struct qcqst *qcqst)
{
    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    struct rxq_desc *desc;
    struct desc_info *desc_info;
    struct rxq_sg_desc *sg_desc;
    PNET_BUFFER packet;
    bool ring_doorbell;
    unsigned int len;
    unsigned int i;
    struct qcq *qcq = qcqst->qcq;
    struct queue *q = &qcq->q;
    struct rxq_pkt *rxq_pkt = NULL;
    ULONG sg_index = 0;
    struct rxq_sg_elem *sg_elem;
	unsigned int remaining_len = 0;

    len = lif->ionic->frame_size;

    for (i = 0; i < q->rx_pkt_cnt; i++) {

        rxq_pkt = ionic_get_next_rxq_pkt(qcq);
        ASSERT(rxq_pkt != NULL);

        packet = ionic_alloc_rxq_netbuffers(lif->ionic, qcqst, len, rxq_pkt);
        if (!packet) {
            DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_ERROR,
                      "%s RXQ Buffer Alloc Failed adapter %p Status 0x%0x\n",
                      __FUNCTION__, lif->ionic, NDIS_STATUS_RESOURCES));

            return NDIS_STATUS_RESOURCES;
        }

        if (i < ionic_q_space_avail(q)) {
            desc_info = q->head;
            desc = (struct rxq_desc *)desc_info->desc;
            sg_desc = (struct rxq_sg_desc *)desc_info->sg_desc;

			remaining_len = len;

            desc->opcode = (rxq_pkt->sg_count > 1) ? (u8)RXQ_DESC_OPCODE_SG
                                                   : (u8)RXQ_DESC_OPCODE_SIMPLE;

            desc->addr = cpu_to_le64(rxq_pkt->phys_addr[0]);
            desc->len = (__le16)cpu_to_le16(min( remaining_len, PAGE_SIZE));
			remaining_len -= desc->len;
           
            for (sg_index = 0; sg_index < (rxq_pkt->sg_count - 1); sg_index++) {
                sg_elem = &sg_desc->elems[sg_index];
                sg_elem->addr = cpu_to_le64(rxq_pkt->phys_addr[sg_index + 1]);
                sg_elem->len = (__le16)cpu_to_le16(min( remaining_len, PAGE_SIZE));
				remaining_len -= sg_elem->len;
            }

			ASSERT( remaining_len == 0);

            // zero filled sentinel
            sg_elem = &sg_desc->elems[rxq_pkt->sg_count];
            sg_elem->addr = 0;
            sg_elem->len = 0;

            ring_doorbell =
                ((q->head->index + 1) & RX_RING_DOORBELL_STRIDE) == 0;

            ionic_rxq_post(q, ring_doorbell, ionic_rx_clean, packet);
        } else {
            ionic_return_rxq_pkt(qcq, rxq_pkt);
        }
    }

    return ntStatus;
}

void
ionic_rx_fill(struct qcq *qcq)
{
    struct cq *rxcq = &qcq->cq;
    struct queue *q = rxcq->bound_q;
    struct rxq_desc *desc;
    bool ring_doorbell;
    unsigned int i;
    struct rxq_pkt *rxq_pkt = NULL;
    struct desc_info *desc_info;
    struct rxq_sg_desc *sg_desc;
    ULONG sg_index = 0;
    struct rxq_sg_elem *sg_elem;
	unsigned int remaining_len = 0;

    for (i = ionic_q_space_avail(q); i; i--) {

        rxq_pkt = ionic_get_next_rxq_pkt(qcq);

        if (rxq_pkt == NULL) {
            DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_ERROR,
                      "%s No rxq packets available on fill\n", __FUNCTION__));
            break;
        }

		remaining_len = q->lif->ionic->frame_size;

        desc_info = q->head;
        desc = (struct rxq_desc *)desc_info->desc;
        sg_desc = (struct rxq_sg_desc *)desc_info->sg_desc;

        desc->opcode = (rxq_pkt->sg_count > 1) ? (u8)RXQ_DESC_OPCODE_SG
                                               : (u8)RXQ_DESC_OPCODE_SIMPLE;

        desc->addr = cpu_to_le64(rxq_pkt->phys_addr[0]);
		desc->len = (__le16)cpu_to_le16(min( remaining_len, PAGE_SIZE));
		remaining_len -= desc->len;

        for (sg_index = 0; sg_index < (rxq_pkt->sg_count - 1); sg_index++) {
            sg_elem = &sg_desc->elems[sg_index];
            sg_elem->addr = cpu_to_le64(rxq_pkt->phys_addr[sg_index + 1]);
			sg_elem->len = (__le16)cpu_to_le16(min( remaining_len, PAGE_SIZE));
			remaining_len -= sg_elem->len;
        }

		ASSERT( remaining_len == 0);

        // zero filled sentinel
        sg_elem = &sg_desc->elems[rxq_pkt->sg_count];
        sg_elem->addr = 0;
        sg_elem->len = 0;

        ring_doorbell = ((q->head->index + 1) & RX_RING_DOORBELL_STRIDE) == 0;

        ASSERT(rxq_pkt->packet != NULL);
        ionic_rxq_post(q, ring_doorbell, ionic_rx_clean, rxq_pkt->packet);
    }

    InterlockedAdd( (LONG *)&qcq->rx_stats->pool_packet_count,
                      qcq->pkts_free_count);
    InterlockedIncrement( (LONG *)&qcq->rx_stats->pool_sample_count);

    return;
}

void
ionic_rx_filters_deinit(struct lif *lif)
{
    LIST_ENTRY *head;
    LIST_ENTRY *cur;
    LIST_ENTRY *next = NULL;
    struct rx_filter *f;
    unsigned int i;

    for (i = 0; i < RX_FILTER_HLISTS; i++) {

        head = &lif->rx_filters.by_id[i];

        if (!IsListEmpty(head)) {

            cur = head->Flink;
            do {

                f = CONTAINING_RECORD(cur, struct rx_filter, by_id);

                next = cur->Flink;

                ionic_rx_filter_free(lif, f);

                cur = next;
            } while (cur != head);
        }
    }
}

void
ionic_rx_napi(struct interrupt_info *int_info,
              int budget,
              NDIS_RECEIVE_THROTTLE_PARAMETERS *receive_throttle_params)
{
    int rx_id = int_info->rx_id;
    struct lif *lif = int_info->lif;
    struct qcq *qcq = lif->rxqcqs[rx_id].qcq;
    struct cq *rxcq = &qcq->cq;
    unsigned int qi = rxcq->bound_q->index;
    struct ionic_dev *idev = &lif->ionic->idev;
    u32 work_done = 0;
    u32 tx_work_done = 0;
    u32 flags = 0;
    PNET_BUFFER_LIST packets_to_indicate = NULL;

    // Process tx if in mode 1 or 3
    if( BooleanFlagOn( lif->ionic->ConfigStatus, IONIC_TX_MODE_DPC)) {
        tx_work_done = ionic_tx_flush(lif->txqcqs[qi].qcq, false, true);
    }

    NdisDprAcquireSpinLock(&qcq->rx_ring_lock);

    if( budget == 0) {
        if( receive_throttle_params->MaxNblsToIndicate == 0) {
            budget = IONIC_RX_BUDGET_DEFAULT;
        }
        else {
            budget = receive_throttle_params->MaxNblsToIndicate;
        }
    }

    work_done = ionic_rx_walk_cq(rxcq, budget, &packets_to_indicate);

    if (work_done) {
        ionic_rx_fill(qcq);
    }

    NdisDprReleaseSpinLock(&qcq->rx_ring_lock);

    if (work_done == 0 && tx_work_done == 0) {
        DbgTrace((
            TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
            "%s Found nothing to process on lif %d msi %d ******************\n",
            __FUNCTION__, lif->index, lif->rxqcqs[rx_id].qcq->queue_id));
    }
    
    flags |= IONIC_INTR_CRED_UNMASK;
    if (work_done < (u32)budget) {
        receive_throttle_params->MoreNblsPending = FALSE;
    }
     else {
    	receive_throttle_params->MoreNblsPending = TRUE;
    }

    if (work_done || flags) {
        flags |= IONIC_INTR_CRED_RESET_COALESCE;
        ionic_intr_credits(idev->intr_ctrl, rxcq->bound_intr->index, work_done,
                           flags);

        if (work_done) {
            ionic_rq_indicate_bufs(lif, &lif->rxqcqs[rx_id], qcq, work_done,
                                   packets_to_indicate);
        }
    }

    return;
}

void
ionic_rq_indicate_bufs(struct lif *lif,
                       struct qcqst *qcqst,
                       struct qcq *qcq,
                       unsigned int count,
                       PNET_BUFFER_LIST packets_to_indicate)
{
    struct ionic *ionic = lif->ionic;
    ULONG flags = NDIS_RECEIVE_FLAGS_DISPATCH_LEVEL;

    UNREFERENCED_PARAMETER(qcqst);

    DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
              "%s Enter Indicate RXQ Pkts adapter %p Count %d\n", __FUNCTION__,
              ionic, count));

    flags |= (NDIS_RECEIVE_FLAGS_SINGLE_QUEUE |
              NDIS_RECEIVE_FLAGS_SHARED_MEMORY_INFO_VALID);

    if (count != 0) {

        DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
                  "%s Indicating %d packets nbl %p\n", __FUNCTION__, count,
                  packets_to_indicate));

        /* inform ndis about received packets */
        NdisMIndicateReceiveNetBufferLists(
            ionic->adapterhandle, packets_to_indicate, 0, count, flags);

        InterlockedAdd(&qcq->outstanding_rx_count, (LONG)count);
    }

    DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
              "%s Exit Indicate RXQ Pkts adapter %p\n", __FUNCTION__, ionic));
}

void
ionic_return_packet(NDIS_HANDLE adapter_context,
                    PNET_BUFFER_LIST pnetlist,
                    ULONG return_flags)
{
    struct ionic *ionic = (struct ionic *)adapter_context;
    ULONG num_nbls = 0;
    PNET_BUFFER packet;
    struct rxq_pkt *rxq_pkt;
    unsigned int len = ionic->frame_size;
    PMDL mdl;
    PNET_BUFFER_LIST nbl, nbl_next;
    struct dev_rx_ring_stats *rx_stats = NULL;

    UNREFERENCED_PARAMETER(return_flags);

    DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
              "%s Enter ionic_return_packets adapter %p nbl %p\n", __FUNCTION__,
              ionic, pnetlist));

    nbl = pnetlist;

    while (nbl) {
        // Ideally we need to check the NB and return the buffer to
        // the pool, but we are not there yet so for now just ignore

        packet = NET_BUFFER_LIST_FIRST_NB(nbl);
        rxq_pkt = *(struct rxq_pkt **)NET_BUFFER_MINIPORT_RESERVED(packet);

        nbl_next = NET_BUFFER_LIST_NEXT_NBL(nbl);
        NET_BUFFER_LIST_NEXT_NBL(nbl) = NULL;

        ASSERT(rxq_pkt != NULL);

        if (rxq_pkt != NULL) {
            struct qcq *qcq;

            qcq = q_to_qcq(rxq_pkt->q);
            rx_stats = qcq->rx_stats;

            InterlockedDecrement(&qcq->outstanding_rx_count);

            // Dev stats
            rx_stats->completion_count++;

            // Reinitialize the packet so that it can be reused
            mdl = NET_BUFFER_FIRST_MDL(packet);
            NdisAdjustMdlLength(mdl, len);

            DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
                      "%s Return  RXQ NBL adapter %p NB %p NBL %p\n",
                      __FUNCTION__, ionic, packet, rxq_pkt->parent_nbl));

            ionic_return_rxq_pkt(qcq, rxq_pkt);
        }
        // deref the adapter
        deref_request( ionic, 1);

        nbl = nbl_next;
        num_nbls++;
    }

    DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
              "%s Exit ionic_return_packets adapter %p num_nbls [%d]\n",
              __FUNCTION__, ionic, num_nbls));
}

struct rx_filter *
ionic_rx_filter_by_macvlan(struct lif *lif, u16 vid)
{
    unsigned int key = hash_32(vid, RX_FILTER_HASH_BITS);
    LIST_ENTRY *head = &lif->rx_filters.by_hash[key];
    LIST_ENTRY *current = NULL;
    struct rx_filter *f;

    if (!IsListEmpty(head)) {

        current = head->Flink;
        do {

            f = CONTAINING_RECORD(current, struct rx_filter, by_hash);

            if (le16_to_cpu(f->cmd.match) == RX_FILTER_MATCH_MAC_VLAN) {
                if (le16_to_cpu(f->cmd.mac_vlan.vlan) == vid)
                    return f;
            }

            current = current->Flink;
        } while (current != head);
    }

    return NULL;
}

struct rx_filter *
ionic_rx_filter_by_vlan(struct lif *lif, u16 vid)
{
    unsigned int key = hash_32(vid, RX_FILTER_HASH_BITS);
    LIST_ENTRY *head = &lif->rx_filters.by_hash[key];
    LIST_ENTRY *current = NULL;
    struct rx_filter *f;

    if (!IsListEmpty(head)) {

        current = head->Flink;
        do {

            f = CONTAINING_RECORD(current, struct rx_filter, by_hash);

            if (le16_to_cpu(f->cmd.match) == RX_FILTER_MATCH_VLAN) {
                if (le16_to_cpu(f->cmd.vlan.vlan) == vid)
                    return f;
            }

            current = current->Flink;
        } while (current != head);
    }

    return NULL;
}

struct rx_filter *
ionic_rx_filter_by_addr(struct lif *lif, const u8 *addr)
{

    unsigned int key = hash_32(*(u32 *)addr, RX_FILTER_HASH_BITS);
    LIST_ENTRY *head = &lif->rx_filters.by_hash[key];
    LIST_ENTRY *current = NULL;
    struct rx_filter *f;

    if (!IsListEmpty(head)) {

        current = head->Flink;
        do {

            f = CONTAINING_RECORD(current, struct rx_filter, by_hash);

            if (le16_to_cpu(f->cmd.match) == RX_FILTER_MATCH_MAC) {
                if (memcmp(addr, f->cmd.mac.addr, ETH_ALEN) == 0)
                    return f;
            }

            current = current->Flink;
        } while (current != head);
    }

    return NULL;
}

int
ionic_rx_filter_save(struct lif *lif,
                     u32 flow_id,
                     u16 rxq_index,
                     u32 hash,
                     struct ionic_admin_ctx *ctx)
{

    struct rx_filter_add_cmd *ac;
    LIST_ENTRY *head;
    struct rx_filter *f;
    unsigned int key;

    UNREFERENCED_PARAMETER(hash);

    ac = &ctx->cmd.rx_filter_add;

    switch (le16_to_cpu(ac->match)) {
    case RX_FILTER_MATCH_VLAN:
        key = le16_to_cpu(ac->vlan.vlan);
        break;
    case RX_FILTER_MATCH_MAC:
        key = *(u32 *)ac->mac.addr;
        break;
    case RX_FILTER_MATCH_MAC_VLAN:
        key = le16_to_cpu(ac->mac_vlan.vlan);
        break;
    default:
        return NDIS_STATUS_INVALID_DATA;
    }

    f = (struct rx_filter *)NdisAllocateMemoryWithTagPriority_internal(
        lif->ionic->adapterhandle, sizeof(struct rx_filter),
        IONIC_RX_FILTER_TAG, NormalPoolPriority);
    if (!f)
        return NDIS_STATUS_RESOURCES;

    NdisZeroMemory(f, sizeof(struct rx_filter));

    DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                  "%s Allocated 0x%p len %08lX\n",
                  __FUNCTION__,
                  f, sizeof(struct rx_filter)));
    
    f->flow_id = flow_id;
    f->filter_id = le32_to_cpu(ctx->comp.rx_filter_add.filter_id);
    f->rxq_index = rxq_index;
    memcpy(&f->cmd, ac, sizeof(f->cmd));

    // INIT_HLIST_NODE(&f->by_hash);
    // INIT_HLIST_NODE(&f->by_id);

    NdisAcquireSpinLock(&lif->rx_filters.lock);

    key = hash_32(key, RX_FILTER_HASH_BITS);
    head = &lif->rx_filters.by_hash[key];
    InsertHeadList(head, &f->by_hash);

    key = f->filter_id & RX_FILTER_HLISTS_MASK;
    head = &lif->rx_filters.by_id[key];
    InsertHeadList(head, &f->by_id);

    NdisReleaseSpinLock(&lif->rx_filters.lock);

    return NDIS_STATUS_SUCCESS;
}

void
ionic_rx_filter_free(struct lif *lif, struct rx_filter *f)
{

    RemoveEntryList(&f->by_id);
    RemoveEntryList(&f->by_hash);
    NdisFreeMemoryWithTagPriority_internal(lif->ionic->adapterhandle, f,
                                  IONIC_RX_FILTER_TAG);
}

int
ionic_rx_filters_init(struct lif *lif)
{
    unsigned int i;

    NdisAllocateSpinLock(&lif->rx_filters.lock);

    for (i = 0; i < RX_FILTER_HLISTS; i++) {
        InitializeListHead(&lif->rx_filters.by_hash[i]);
        InitializeListHead(&lif->rx_filters.by_id[i]);
    }

    return 0;
}

NDIS_STATUS
_ionic_lif_rx_mode(struct lif *lif, unsigned int rx_mode)
{

    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    //	struct ionic_deferred_work *work;

    //	if (in_interrupt()) {
    //		work = kzalloc(sizeof(*work), GFP_ATOMIC);
    //		if (!work) {
    //			netdev_err(lif->netdev, "%s OOM\n", __func__);
    //			return;
    //		}
    //		work->type = DW_TYPE_RX_MODE;
    //		work->rx_mode = rx_mode;
    //		netdev_dbg(lif->netdev, "deferred: rx_mode\n");
    //		ionic_lif_deferred_enqueue(&lif->deferred, work);
    //	} else {
    status = ionic_lif_rx_mode(lif, rx_mode);
    //	}

    return status;
}

NDIS_STATUS
ionic_set_rx_mode(struct lif *lif, int rx_mode)
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    // netdev_priv(netdev);
    //	struct identity *ident = &lif->ionic->ident;
    //	unsigned int nfilters;
    // unsigned int rx_mode;

    // rx_mode = RX_MODE_F_UNICAST;
    // rx_mode |= RX_MODE_F_MULTICAST; //(netdev->flags & IFF_MULTICAST) ?
    // RX_MODE_F_MULTICAST : 0; rx_mode |= RX_MODE_F_BROADCAST; //(netdev->flags
    // & IFF_BROADCAST) ? RX_MODE_F_BROADCAST : 0; rx_mode |= (netdev->flags &
    // IFF_PROMISC) ? RX_MODE_F_PROMISC : 0; rx_mode |= (netdev->flags &
    // IFF_ALLMULTI) ? RX_MODE_F_ALLMULTI : 0;

    /* sync unicast addresses
     * next check to see if we're in an overflow state
     *    if so, we track that we overflowed and enable NIC PROMISC
     *    else if the overflow is set and not needed
     *       we remove our overflow flag and check the netdev flags
     *       to see if we can disable NIC PROMISC
     */
    //__dev_uc_sync(netdev, ionic_addr_add, ionic_addr_del);
    // nfilters = le32_to_cpu(ident->lif.eth.max_ucast_filters);
    // if (netdev_uc_count(netdev) + 1 > nfilters) {
    //	rx_mode |= RX_MODE_F_PROMISC;
    //	lif->uc_overflow = true;
    //} else if (lif->uc_overflow) {
    //	lif->uc_overflow = false;
    //	if (!(netdev->flags & IFF_PROMISC))
    //		rx_mode &= ~RX_MODE_F_PROMISC;
    //}

    /* same for multicast */
    //__dev_mc_sync(netdev, ionic_addr_add, ionic_addr_del);
    // nfilters = le32_to_cpu(ident->lif.eth.max_mcast_filters);
    // if (netdev_mc_count(netdev) > nfilters) {
    //	rx_mode |= RX_MODE_F_ALLMULTI;
    //	lif->mc_overflow = true;
    //} else if (lif->mc_overflow) {
    //	lif->mc_overflow = false;
    //	if (!(netdev->flags & IFF_ALLMULTI))
    //		rx_mode &= ~RX_MODE_F_ALLMULTI;
    //}

    if (lif->rx_mode != (unsigned int)rx_mode)
        status = _ionic_lif_rx_mode(lif, rx_mode);

    return status;
}

void
IndicateRxQueueState(struct ionic *ionic,
                     NDIS_RECEIVE_QUEUE_ID QueueId,
                     NDIS_RECEIVE_QUEUE_OPERATIONAL_STATE State)
{
    NDIS_STATUS_INDICATION Status;
    NDIS_RECEIVE_QUEUE_STATE QueueState;

    NdisZeroMemory(&Status, sizeof(Status));
    NdisZeroMemory(&QueueState, sizeof(QueueState));

    QueueState.Header.Revision = NDIS_RECEIVE_QUEUE_STATE_REVISION_1;
    QueueState.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
    QueueState.Header.Size = NDIS_SIZEOF_NDIS_RECEIVE_QUEUE_STATE_REVISION_1;
    QueueState.QueueId = QueueId;
    QueueState.QueueState = State;

    Status.Header.Type = NDIS_OBJECT_TYPE_STATUS_INDICATION;
    Status.Header.Revision = NDIS_STATUS_INDICATION_REVISION_1;
    Status.Header.Size = NDIS_SIZEOF_STATUS_INDICATION_REVISION_1;
    Status.StatusCode = NDIS_STATUS_RECEIVE_QUEUE_STATE;
    Status.StatusBuffer = &QueueState;
    Status.StatusBufferSize = QueueState.Header.Size;

    NdisMIndicateStatusEx(ionic->adapterhandle, &Status);

    return;
}

NDIS_STATUS
init_rx_queue_info(struct ionic *ionic)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    ULONG ulIndex = 0;

    while (ulIndex < IONIC_MAX_VM_QUEUE_COUNT) {

        ionic->vm_queue[ulIndex].QueueLock =
            NdisAllocateRWLock(ionic->adapterhandle);

        if (ionic->vm_queue[ulIndex].QueueLock == NULL) {
            ntStatus = NDIS_STATUS_RESOURCES;
            break;
        }

        ulIndex++;
    }

    return ntStatus;
}

NDIS_STATUS
delete_rx_queue_info(struct ionic *ionic)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    ULONG ulIndex = 0;

    while (ulIndex < IONIC_MAX_VM_QUEUE_COUNT) {

        if (ionic->vm_queue[ulIndex].QueueLock != NULL) {
            NdisFreeRWLock(ionic->vm_queue[ulIndex].QueueLock);
            ionic->vm_queue[ulIndex].QueueLock = NULL;
        }

        ulIndex++;
    }

    return ntStatus;
}

void
DumpRxPacket(struct rxq_pkt *rxq_pkt)
{

    u8 *pStream = (u8 *)rxq_pkt->addr;
    u8 *pSrcMac = NULL;
    u8 *pDestMac = NULL;
    u8 *pICmp = NULL;
    u8 *pProto = NULL;

    pDestMac = pStream;

    pSrcMac = pStream + ETH_ALEN;

    pICmp = pStream + 14;

    if (*pICmp == 0x45) {

        pProto = pICmp + 9;

        if (*pProto == 1) {

            IoPrint("%s ******* Have ping packet %p Src "
                    "%02lX:%02lX:%02lX:%02lX:%02lX:%02lX Dst "
                    "%02lX:%02lX:%02lX:%02lX:%02lX:%02lX ********\n",
                    __FUNCTION__, rxq_pkt, pSrcMac[0], pSrcMac[1], pSrcMac[2],
                    pSrcMac[3], pSrcMac[4], pSrcMac[5], pDestMac[0],
                    pDestMac[1], pDestMac[2], pDestMac[3], pDestMac[4],
                    pDestMac[5]);
        }
    }

    return;
}

ULONG
GetPktCount(struct ionic *ionic, struct queue *q)
{
    ULONG cnt = 0;

    cnt = (q->num_descs * ionic->rx_pool_factor) / 100;

    return cnt;
}

void
ionic_rx_empty(struct queue *q)
{
    struct desc_info *cur;
    struct rxq_sg_desc *sg_desc;
    struct rxq_desc *desc;

    for (cur = q->tail; cur != q->head; cur = cur->next) {
        desc = (struct rxq_desc *)cur->desc;
        desc->addr = 0;
        desc->len = 0;

        sg_desc = (struct rxq_sg_desc *)cur->sg_desc;
#if 0
		for (i = 0; i < cur->npages; i++) {
			if (likely(cur->pages[i].page)) {
				ionic_rx_page_free(q, cur->pages[i].page,
						   cur->pages[i].dma_addr);
				cur->pages[i].page = NULL;
				cur->pages[i].dma_addr = 0;
			}
		}
#endif

        cur->cb_arg = NULL;
    }
}