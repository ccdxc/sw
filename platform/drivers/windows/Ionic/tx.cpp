
#include "common.h"

void
ionic_txq_nbl_list_push_head(struct txq_nbl_list *list, PNET_BUFFER_LIST nbl)
{
    if (!list->tail) {
        list->tail = nbl;
    }
    NET_BUFFER_LIST_NEXT_NBL(nbl) = list->head;
    list->head = nbl;
    list->count++;
}

void
ionic_service_pending_nbl_requests(struct ionic *ionic, struct qcq *qcq)
{
    PNET_BUFFER_LIST nbl;
    NDIS_STATUS status;
    NDIS_TCP_LARGE_SEND_OFFLOAD_NET_BUFFER_LIST_INFO mss = {0};
    txq_nbl_list completed_list;

    ionic_txq_nbl_list_init(&completed_list);

    DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
              "%s Enter Service Pending NBL adapter %p\n", __FUNCTION__,
              ionic));

    BUG_ON(NDIS_CURRENT_IRQL() != DISPATCH_LEVEL);

    NdisDprAcquireSpinLock(&qcq->txq_pending_nbl_lock);

    nbl = ionic_txq_nbl_list_pop_head(&qcq->txq_pending_nbl);

    while (nbl) {

        InterlockedDecrement( (LONG *)&qcq->tx_stats->pending_nbl_count);

        DbgTrace((TRACE_COMPONENT_PENDING_LIST, TRACE_LEVEL_VERBOSE,
                  "%s Processing nbl %p\n", __FUNCTION__, nbl));

        status = process_nbl(ionic, qcq, nbl, &completed_list);

        if (status == NDIS_STATUS_PENDING) {
            ionic_txq_nbl_list_push_head(&qcq->txq_pending_nbl, nbl);
            InterlockedIncrement( (LONG *)&qcq->tx_stats->pending_nbl_count);

            DbgTrace((TRACE_COMPONENT_PENDING_LIST, TRACE_LEVEL_VERBOSE,
                      "%s Re-inserting nbl %p\n", __FUNCTION__, nbl));
            break;
        }

        nbl = ionic_txq_nbl_list_pop_head(&qcq->txq_pending_nbl);
    }

    ionic_send_complete(ionic, &completed_list, DISPATCH_LEVEL);

    NdisDprReleaseSpinLock(&qcq->txq_pending_nbl_lock);

    DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
              "%s Exit Service Pending NBL adapter %p\n", __FUNCTION__, ionic));

    return;
}

bool
ionic_service_nb_requests(struct qcq *qcq, bool exiting)
{
	bool more_items = true;
	struct ionic *ionic = qcq->q.lif->ionic;
    struct txq_pkt_private *txq_pkt_private;
    struct txq_pkt *txq_pkt;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    NDIS_TCP_LARGE_SEND_OFFLOAD_NET_BUFFER_LIST_INFO mss = {0};
    txq_nbl_list completed_list;
    LIST_ENTRY *list_entry = NULL;

    ionic_txq_nbl_list_init(&completed_list);

	NdisAcquireSpinLock( &qcq->txq_nb_lock);

    while (!IsListEmpty(&qcq->txq_nb_list)) {

        list_entry = RemoveHeadList(&qcq->txq_nb_list);

		NdisReleaseSpinLock( &qcq->txq_nb_lock);

        txq_pkt_private =
            CONTAINING_RECORD(list_entry, struct txq_pkt_private, link);
        txq_pkt = txq_pkt_private->txq_pkt;

        DbgTrace((TRACE_COMPONENT_PENDING_LIST, TRACE_LEVEL_VERBOSE,
                  "%s Processing NB %p\n", __FUNCTION__, txq_pkt->packet_orig));

		if (exiting) {
            ionic_txq_complete_failed_pkt(ionic, qcq, 
										  txq_pkt->parent_nbl,
										  txq_pkt->packet_orig,
                                          &completed_list,
                                          NDIS_STATUS_SEND_ABORTED);
			NdisAcquireSpinLock( &qcq->txq_nb_lock);
			continue;
		}
       
        status = ionic_queue_txq_pkt(ionic, qcq, txq_pkt->packet_orig);

        if (status == NDIS_STATUS_PENDING) {
			NdisAcquireSpinLock( &qcq->txq_nb_lock);
            // Insert it back to the head of the list
            InsertHeadList(&qcq->txq_nb_list, &txq_pkt_private->link);

            DbgTrace((TRACE_COMPONENT_PENDING_LIST, TRACE_LEVEL_VERBOSE,
                      "%s Re-inserting NB %p\n", __FUNCTION__,
                      txq_pkt->packet_orig));
            break;
        } else if (status != NDIS_STATUS_SUCCESS) {

            DbgTrace((TRACE_COMPONENT_PENDING_LIST, TRACE_LEVEL_VERBOSE,
                      "%s Failing NB %p Status %08lX\n", __FUNCTION__,
                      txq_pkt->packet_orig, status));
            ionic_txq_complete_failed_pkt(ionic, qcq, 
										  txq_pkt->parent_nbl,
										  txq_pkt->packet_orig,
                                          &completed_list,
                                          NDIS_STATUS_SEND_ABORTED);
			status = NDIS_STATUS_SUCCESS;
        }

		NdisAcquireSpinLock( &qcq->txq_nb_lock);
    }

	NdisReleaseSpinLock( &qcq->txq_nb_lock);

	ionic_send_complete(ionic, &completed_list, NDIS_CURRENT_IRQL());

    DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
              "%s Exit Service Pending NB adapter %p\n", __FUNCTION__, ionic));

    return more_items;
}

NDIS_STATUS
process_nbl(struct ionic *ionic,
            struct qcq *qcq,
            PNET_BUFFER_LIST nbl,
            txq_nbl_list *completed_list)
{

    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    BOOLEAN bLSOEnabled = FALSE;
    NDIS_TCP_LARGE_SEND_OFFLOAD_NET_BUFFER_LIST_INFO mss = {0};
    ULONG ulMSS = 0;
    PNET_BUFFER nb, next_nb;
    struct txq_pkt_private *txq_pkt_private;
    struct txq_pkt *txq_pkt;
	LONG processed_cnt = 0;
	struct txq_nbl_private *nbl_private = (struct txq_nbl_private *)NET_BUFFER_LIST_MINIPORT_RESERVED( nbl);
	bool fail_nbl = FALSE;

    mss.Value = NET_BUFFER_LIST_INFO(nbl, TcpLargeSendNetBufferListInfo);

    if (mss.Transmit.Type == NDIS_TCP_LARGE_SEND_OFFLOAD_V1_TYPE &&
        mss.LsoV1Transmit.MSS != 0) {
        bLSOEnabled = TRUE;
        ulMSS = mss.LsoV1Transmit.MSS;
    } else if (mss.Transmit.Type == NDIS_TCP_LARGE_SEND_OFFLOAD_V2_TYPE) {
        bLSOEnabled = TRUE;
        ulMSS = mss.LsoV2Transmit.MSS;
    }

    for (nb = NET_BUFFER_LIST_FIRST_NB(nbl); nb; nb = next_nb) {

        next_nb = NET_BUFFER_NEXT_NB(nb);
		
		if (processed_cnt < nbl_private->nb_processed_cnt) {
			processed_cnt++;
			continue;
		}
		processed_cnt++;

		if (fail_nbl) {
            DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_ERROR,
                      "%s NdisMAllocateNetBufferSGList failed previously adapter %p NBL "
                      "%p NB %p\n",
                      __FUNCTION__, ionic, nbl, nb));

            ionic_txq_complete_failed_pkt(ionic, qcq, 
										  nbl, 
										  nb, 
										  completed_list,
                                          NDIS_STATUS_SEND_ABORTED);
			continue;
		}
		
        txq_pkt_private =
            (struct txq_pkt_private *)NET_BUFFER_MINIPORT_RESERVED(nb);

        ASSERT (txq_pkt_private->txq_pkt == NULL);

        //
        // Do we need multiple descriptors?
        //

        if (bLSOEnabled) {
            ASSERT(ulMSS != 0);
            txq_pkt_private->desc_cnt = GetDescriptorCount(nb, ulMSS);
        } else {
            txq_pkt_private->desc_cnt = 1;
        }

        txq_pkt_private->txq_pkt = ionic_get_next_txq_pkt(qcq);
        if (!txq_pkt_private->txq_pkt) {
            status = NDIS_STATUS_PENDING;
            nbl = NULL;
            break;
        }

		nbl_private->nb_processed_cnt++;

        txq_pkt = txq_pkt_private->txq_pkt;
        txq_pkt->parent_nbl = nbl;
        txq_pkt->packet_orig = nb;
        txq_pkt->sg_os_list = NULL;

        txq_pkt->flags = NET_BUFFER_SG_LIST_CREATED;

        DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
                  "%s Service NBL adapter %p NBL %p NB %p\n",
                  __FUNCTION__, ionic, nbl, nb));

        status = NdisMAllocateNetBufferSGList(
            ionic->dma_handle, nb, (void *)txq_pkt,
            NDIS_SG_LIST_WRITE_TO_DEVICE, txq_pkt->sg_list,
            ionic->sgl_size_in_bytes);

        if (status != NDIS_STATUS_SUCCESS) {
            DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_ERROR,
                      "%s NdisMAllocateNetBufferSGList failed adapter %p NBL "
                      "%p NB %p status 0x%0lX\n",
                      __FUNCTION__, ionic, nbl, nb, status));

            txq_pkt->flags = 0;
            ionic_txq_complete_failed_pkt(ionic, qcq, 
										  nbl, 
										  nb, 
										  completed_list,
                                          NDIS_STATUS_SEND_ABORTED);
            qcq->tx_stats->dma_map_error++;
			fail_nbl = true;
        }
    }

    return status;
}

bool
ionic_tx_service(struct cq *cq, struct cq_info *cq_info, void *cb_arg)
{
    struct txq_comp *comp = (struct txq_comp *)cq_info->cq_desc;

    UNREFERENCED_PARAMETER(cb_arg);

    // printk(KERN_ERR "ionic_tx_service comp->color %d cq->done_color %d\n",
    // comp->color, cq->done_color);
    if (!color_match(comp->color, cq->done_color))
        return false;

    ionic_q_service(cq->bound_q, cq_info, comp->comp_index);

    return true;
}

static bool
ionic_txq_complete_pkt(struct queue *q,
                       struct desc_info *desc_info,
                       struct cq_info *cq_info,
                       void *cb_arg,
                       void *reserved,
                       void *reserved2)
{

    UNREFERENCED_PARAMETER(desc_info);
    UNREFERENCED_PARAMETER(cq_info);
    UNREFERENCED_PARAMETER(reserved);
    UNREFERENCED_PARAMETER(reserved2);

    ASSERT(q != NULL);

    struct qcq *qcq = q_to_qcq(q);
    struct ionic *ionic = q->lif->ionic;
    PNET_BUFFER packet = (PNET_BUFFER)cb_arg;
    PNET_BUFFER packet_orig;
    struct txq_pkt_private *txq_pkt_private;
    PNET_BUFFER_LIST parent_nbl;
    struct txq_pkt *txq_pkt;
    LONG ref_cnt;
    NDIS_TCP_LARGE_SEND_OFFLOAD_NET_BUFFER_LIST_INFO mss = {0};
    BOOLEAN bProcessPacket = TRUE;
    struct txq_comp *comp = (struct txq_comp *)cq_info->cq_desc;

    DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
              "%s Enter TXQ complete packet adapter %p NB %p\n", __FUNCTION__,
              ionic, packet));

    txq_pkt_private =
        (struct txq_pkt_private *)NET_BUFFER_MINIPORT_RESERVED(packet);
    txq_pkt = txq_pkt_private->txq_pkt;
    if (txq_pkt) {
        parent_nbl = txq_pkt->parent_nbl;
        packet_orig = txq_pkt->packet_orig;

        DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
                  "%s Completing packet %p desc cnt %d current %d\n",
                  __FUNCTION__, packet, txq_pkt_private->desc_cnt,
                  txq_pkt_private->current_desc_cnt));

        if (txq_pkt_private->desc_cnt > 1) {

            bProcessPacket = FALSE;

            mss.Value =
                NET_BUFFER_LIST_INFO(parent_nbl, TcpLargeSendNetBufferListInfo);

            if (InterlockedIncrement16(
                    (volatile short *)&txq_pkt_private->current_desc_cnt) ==
                txq_pkt_private->desc_cnt) {

                if (mss.Transmit.Type == NDIS_TCP_LARGE_SEND_OFFLOAD_V1_TYPE) {
                    mss.LsoV1TransmitComplete.TcpPayload =
                        txq_pkt_private->bytes_processed;
                } else {
                    mss.LsoV2TransmitComplete.Reserved = 0;
                    mss.LsoV2TransmitComplete.Type =
                        NDIS_TCP_LARGE_SEND_OFFLOAD_V2_TYPE;
                }

                bProcessPacket = TRUE;

                NET_BUFFER_LIST_INFO(parent_nbl,
                                     TcpLargeSendNetBufferListInfo) = mss.Value;
            }
        } else {
            bProcessPacket = TRUE;
        }

        if (txq_pkt->tx_frag_elem != NULL) {
            return_tx_frag(ionic, txq_pkt->tx_frag_elem);
            txq_pkt->tx_frag_elem = NULL;
        }
        
        InterlockedDecrement(&qcq->outstanding_tx_count);

        if (bProcessPacket) {

            InterlockedIncrement( (LONG *)&qcq->tx_stats->nb_count);

            DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
                      "%s Finalizing packet %p\n", __FUNCTION__, packet));

            if (comp->status) {
                IoPrint("%s Failure status 0x%08lX packet len %d\n",
                        __FUNCTION__, comp->status,
                        txq_pkt_private->bytes_processed);
            }

            if (txq_pkt->sg_os_list &&
                (txq_pkt->sg_os_list != txq_pkt->sg_list)) {
                NdisMFreeNetBufferSGList(ionic->dma_handle, txq_pkt->sg_os_list,
                                         packet_orig);                
            } else {
                NdisMFreeNetBufferSGList(ionic->dma_handle, txq_pkt->sg_list,
                                         packet_orig);
            }
			txq_pkt->sg_os_list = NULL;

            ionic_return_txq_pkt(qcq, txq_pkt);

            ref_cnt = NET_BUFFER_LIST_DEC_REF_COUNTER(parent_nbl);
            if (!ref_cnt) {
                DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
                          "%s TXQ adapter %p NBL %p complete\n", __FUNCTION__,
                          ionic, parent_nbl));

                NET_BUFFER_LIST_STATUS(parent_nbl) = NDIS_STATUS_SUCCESS;

                NdisMSendNetBufferListsComplete(
                    ionic->adapterhandle, parent_nbl,
                    NDIS_SEND_COMPLETE_FLAGS_DISPATCH_LEVEL);
                InterlockedIncrement( (LONG *)&qcq->tx_stats->nbl_count);
                deref_request(ionic, 1);
            } else {

                DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
                          "%s TXQ adapter %p NBL %p Incomplete RefCount %d\n",
                          __FUNCTION__, ionic, parent_nbl, ref_cnt));
            }
        }
    }
    DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
              "%s Exit TXQ adapter %p NB %p\n", __FUNCTION__, ionic, packet));

    return true;
}

static int
ionic_tx_calc_no_csum(struct queue *q,
                      int count,
                      ULONGLONG addr,
                      ULONG length,
                      unsigned long vlan_tag,
                      BOOLEAN vlan_tag_insert)
{
    struct txq_desc *desc = (struct txq_desc *)q->head->desc;
    u8 flags = 0;
    // bool encap;
    u64 cmd;

    // encap = skb->encapsulation;

    flags |= vlan_tag_insert ? IONIC_TXQ_DESC_FLAG_VLAN : 0;
    // flags |= encap ? IONIC_TXQ_DESC_FLAG_ENCAP : 0;

    ASSERT(count != 0);

    cmd = encode_txq_desc_cmd(IONIC_TXQ_DESC_OPCODE_CSUM_NONE, flags,
                              (u8)count - 1, addr);

    DbgTrace((TRACE_COMPONENT_TX_NONLSO, TRACE_LEVEL_VERBOSE,
              "\tEmbedding (NO CSUM) physaddr %I64X len %08lX sg cnt %d flags "
              "%08lX into cmd opcode\n",
              addr, length, count - 1, flags));

    desc->cmd = cpu_to_le64(cmd);
    desc->len = (__le16)cpu_to_le16(length); // skb_headlen(skb));
    desc->vlan_tci = (__le16)vlan_tag; // cpu_to_le16(skb_vlan_tag_get(skb));
    desc->hdr_len = 0;

    return 0;
}

static int
ionic_tx_calc_csum(struct queue *q,
                   int count,
                   ULONGLONG addr,
                   ULONG length,
                   unsigned long vlan_tag,
                   BOOLEAN vlan_tag_insert,
                   ULONG csum_offset,
                   BOOLEAN csum_l3,
                   BOOLEAN csum_l4)
{

    struct txq_desc *desc = (struct txq_desc *)q->head->desc;
    u8 flags = 0;
    u64 cmd;
    u8 opcode = IONIC_TXQ_DESC_OPCODE_CSUM_HW;
    UNREFERENCED_PARAMETER(csum_offset);
    if (csum_l3) {
        flags |= IONIC_TXQ_DESC_FLAG_CSUM_L3;
    }
    if (csum_l4) {
        flags |= IONIC_TXQ_DESC_FLAG_CSUM_L4;
    }
    if (vlan_tag_insert) {
        flags |= IONIC_TXQ_DESC_FLAG_VLAN;
    }

    ASSERT(count != 0);

    DbgTrace((TRACE_COMPONENT_TX_NONLSO, TRACE_LEVEL_VERBOSE,
              "\tEmbedding L3 %s L4 %s physaddr %I64X len %08lX sg cnt %d "
              "flags %08lX into cmd opcode\n",
              csum_l3 ? "Yes" : "No", csum_l4 ? "Yes" : "No", addr, length,
              count - 1, flags));

    cmd = encode_txq_desc_cmd(opcode, flags, (u8)count - 1, addr);

    desc->cmd = cpu_to_le64(cmd);
    desc->len = (__le16)cpu_to_le16(length); // skb_headlen(skb));
    desc->vlan_tci = (__le16)vlan_tag; // cpu_to_le16(skb_vlan_tag_get(skb));

    return 0;
}

static int
ionic_tx_frags(struct queue *q,
               PSCATTER_GATHER_ELEMENT sg_element,
               UINT sg_element_count,
               UINT sg_len)
{
    struct txq_sg_desc *sg_desc = (struct txq_sg_desc *)q->head->sg_desc;
    struct txq_sg_elem *elem = (struct txq_sg_elem *)sg_desc->elems;
    dma_addr_t dma_addr;
    int count = 0;
    ULONG len = 0;
    struct txq_sg_elem *last_elem = NULL;

    DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
              "%s Processing additional %d elements\n", __FUNCTION__,
              sg_element_count));

    for (count = 0; count < (int)sg_element_count;
         sg_element++, elem++, count++) {

        len = sg_element->Length;

        if (sg_len < len) {
            len = sg_len;
        }

        DbgTrace((TRACE_COMPONENT_TX_NONLSO, TRACE_LEVEL_VERBOSE,
                  "\t\tsg element %d physaddr %I64X len %08lX\n", count,
                  sg_element->Address.QuadPart, len));

        elem->len = (__le16)cpu_to_le16(len);
        dma_addr =
            sg_element->Address.QuadPart; // ionic_tx_map_frag(q, frag, 0, len);
        elem->addr = cpu_to_le64(dma_addr);
        last_elem = elem;

        sg_len -= len;
    }

    if (count < IONIC_TX_MAX_SG_ELEMS) {
        ASSERT(last_elem != elem);
        elem->addr = 0;
        elem->len = 0;
    }

    return 0;
}

static void
ionic_enqueue_txq_lso_pkt(struct qcq *qcq,
                          PNET_BUFFER packet,
                          dma_addr_t addr,
                          PSCATTER_GATHER_ELEMENT sg_element,
                          UINT sg_element_count,
                          UINT sg_len,
                          UINT len,
                          UINT hdrlen,
                          UINT mss,
                          bool outer_csum,
                          UINT vlan_tci,
                          bool has_vlan,
                          bool start,
                          bool done,
                          bool dumpdesc)
{
    struct queue *q = &qcq->q;
    struct txq_desc *desc = (struct txq_desc *)q->head->desc;
    u8 flags = 0;
    u64 cmd;
    __le16 primary_len = 0;

    flags |= has_vlan ? IONIC_TXQ_DESC_FLAG_VLAN : 0;
    flags |= outer_csum ? IONIC_TXQ_DESC_FLAG_ENCAP : 0;
    flags |= start ? IONIC_TXQ_DESC_FLAG_TSO_SOT : 0;
    flags |= done ? IONIC_TXQ_DESC_FLAG_TSO_EOT : 0;

    ASSERT(sg_element_count != 0);

    cmd = encode_txq_desc_cmd(IONIC_TXQ_DESC_OPCODE_TSO, flags,
                              (u8)sg_element_count - 1, addr);
    desc->cmd = cpu_to_le64(cmd);

    ASSERT(len > sg_len);

    if (start) {
        ASSERT((len - hdrlen) == mss);
    } else if (!done) {
        ASSERT(len == mss);
    } else {
        ASSERT(len <= mss);
    }

    primary_len = (__le16)(len - sg_len);

    DbgTrace((TRACE_COMPONENT_TX_LSO, TRACE_LEVEL_VERBOSE,
              "\t\tEncoding addr %I64X len %08lX sg cnt %d into primary start "
              "%s done %s\n",
              addr, primary_len, sg_element_count - 1, start ? "yes" : "no",
              done ? "yes" : "no"));

    desc->len = (__le16)cpu_to_le16(primary_len);
    desc->vlan_tci = (__le16)cpu_to_le16(vlan_tci);
    desc->hdr_len = (__le16)cpu_to_le16(hdrlen);
    desc->mss = (__le16)cpu_to_le16(mss);

    if (sg_element_count > 1) {

        struct txq_sg_desc *sg_desc = (struct txq_sg_desc *)q->head->sg_desc;
        struct txq_sg_elem *elem = (struct txq_sg_elem *)sg_desc->elems;
        dma_addr_t dma_addr;
        int count = 0;
        ULONG curr_len = 0;
        struct txq_sg_elem *last_elem = NULL;

        //
        // Skip the primary desc element
        //
        sg_element++;
        sg_element_count--;

        ASSERT(sg_len != 0);

        for (count = 0; count < (int)sg_element_count;
             sg_element++, elem++, count++) {
            curr_len = sg_element->Length;

            if (sg_len < curr_len) {
                curr_len = sg_len;
            }
            elem->len = (__le16)cpu_to_le16(curr_len);
            dma_addr = sg_element->Address
                           .QuadPart; // ionic_tx_map_frag(q, frag, 0, len);
            elem->addr = cpu_to_le64(dma_addr);
            last_elem = elem;

            DbgTrace((TRACE_COMPONENT_TX_LSO, TRACE_LEVEL_VERBOSE,
                      "\t\tAdditional sg elements Addr %I64X Len %08lX\n",
                      dma_addr, curr_len));

            sg_len -= curr_len;
        }

        if (count < IONIC_TX_MAX_SG_ELEMS) {
            ASSERT(last_elem != elem);
            elem->addr = 0;
            elem->len = 0;
        }

        ASSERT(sg_len == 0);

        if (dumpdesc)
            DumpDesc(desc, sg_desc, sg_element_count, start, done);
    } else {
        struct txq_sg_desc *sg_desc = (struct txq_sg_desc *)q->head->sg_desc;
        struct txq_sg_elem *elem = (struct txq_sg_elem *)sg_desc->elems;
        elem->addr = 0;
        elem->len = 0;

        if (dumpdesc)
            DumpDesc(desc, NULL, 0, start, done);
    }

    // DumpDesc( desc, start, done);

    if (done) {
        ionic_q_post(q, true, ionic_txq_complete_pkt, packet);
    } else {
        ionic_q_post(q, false, ionic_txq_complete_pkt, packet);
    }

    InterlockedIncrement(&qcq->outstanding_tx_count);

    return;
}

static void
ionic_enqueue_txq_pkt(struct qcq *qcq,
                      PNET_BUFFER packet,
                      UINT packet_len,
                      UINT data_offset,
                      PSCATTER_GATHER_ELEMENT sg_element,
                      UINT sg_element_count,
                      ULONG csum_offset,
                      unsigned long vlan_tag,
                      BOOLEAN vlan_tag_insert,
                      BOOLEAN csum_l3,
                      BOOLEAN csum_l4)
{
    struct queue *q = &qcq->q;
    UINT sg_len = 0;

    DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
              "%s fragcnt %d pktln %08lX len: %08lX addr: %p VLAN %d VLAN_INSERT %d "
              "csum_l3 %d csum_l4 %d\n",
              __FUNCTION__, sg_element_count, packet_len,
              sg_element->Length - data_offset,
              sg_element->Address.QuadPart + data_offset, vlan_tag,
              vlan_tag_insert, csum_l3, csum_l4));

    if (csum_l3 | csum_l4) {
        ionic_tx_calc_csum(q, sg_element_count,
                           sg_element->Address.QuadPart + data_offset,
                           sg_element->Length - data_offset, vlan_tag,
                           vlan_tag_insert, csum_offset, csum_l3, csum_l4);
    } else {
        ionic_tx_calc_no_csum(
            q, sg_element_count, sg_element->Address.QuadPart + data_offset,
            sg_element->Length - data_offset, vlan_tag, vlan_tag_insert);
    }

    if (sg_element_count > 1) {

        sg_len = packet_len - (sg_element->Length - data_offset);

        //
        // Skip the primary sg element
        //
        sg_element++;
        sg_element_count--;

        ionic_tx_frags(q, sg_element, sg_element_count, sg_len);
    } else {
        struct txq_sg_desc *sg_desc = (struct txq_sg_desc *)q->head->sg_desc;
        struct txq_sg_elem *elem = (struct txq_sg_elem *)sg_desc->elems;
        elem->addr = 0;
        elem->len = 0;
    }

    ionic_q_post(q, true, ionic_txq_complete_pkt, packet);

    InterlockedIncrement(&qcq->outstanding_tx_count);

    return;
}

bool
is_1q_hdr_present(void *Buffer)
{

	bool has_tag = false;
	struct ethhdr *hdr = (struct ethhdr *)Buffer;

	if (hdr->h_proto == IONIC_1Q_TAG) {
		has_tag = true;
	}

	return has_tag;
}

NDIS_STATUS
ionic_queue_txq_pkt(struct ionic *ionic,
                    struct qcq *qcq,
                    PNET_BUFFER packet_orig)
{
    PNET_BUFFER packet = packet_orig;
    struct txq_pkt_private *txq_pkt_private =
        (struct txq_pkt_private *)NET_BUFFER_MINIPORT_RESERVED(packet);
    struct txq_pkt *txq_pkt = txq_pkt_private->txq_pkt;
    PNET_BUFFER_LIST parent_nbl = txq_pkt->parent_nbl;
    UINT packet_len, data_offset, sg_element_count;
    PSCATTER_GATHER_LIST frag_list = txq_pkt->sg_os_list;
    PSCATTER_GATHER_ELEMENT sg_element;
    NDIS_TCP_IP_CHECKSUM_NET_BUFFER_LIST_INFO csum_info = {0};
    NDIS_NET_BUFFER_LIST_8021Q_INFO vlan_pri_info;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    NDIS_TCP_LARGE_SEND_OFFLOAD_NET_BUFFER_LIST_INFO mss = {0};
    NDIS_TCP_SEND_OFFLOADS_SUPPLEMENTAL_NET_BUFFER_LIST_INFO supplemental_info =
        {0};
    unsigned long vlan_tag = 0;
    BOOLEAN vlan_tag_insert = 0, lso_pkt = 0;
    unsigned int index;
    struct queue *q;
    struct lif *lif;
    BOOLEAN csum_l4 = FALSE;
    BOOLEAN csum_l3 = FALSE;
    void *pHdrBuffer = NULL;
    ULONGLONG ullCurrentPhysAddr = 0;
    ULONG ulCurrentPhysLen = 0;
    MDL *pCurrentMdl = NULL;
    ULONG ulCurrentLen = 0;
    BOOLEAN bStart = FALSE;
    BOOLEAN bDone = FALSE;
    ULONG sge_count = 0;
    ULONG ulRemainingLen = 0;
    PSCATTER_GATHER_ELEMENT sge = NULL;
    PSCATTER_GATHER_ELEMENT sge_current = NULL;
    ULONG ulHdrLen = 0;
    ULONG ulDescCnt = 0;
    UINT sg_len = 0;
    struct dev_tx_ring_stats *tx_stats = qcq->tx_stats;
    ULONG frame_type = 0;
    ULONG csum_offset = 0;
    ULONG segment_len = 0;
    ULONG len = 0;
    ULONG segment_len_acc = 0;

    static BOOLEAN bdumpdescr = FALSE;

    ASSERT(qcq != NULL);

    DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
              "%s Enter Queue TxQ PKt adapter %p NB %p\n", __FUNCTION__, ionic,
              packet_orig));

    q = &qcq->q;
    ASSERT(q != NULL);

    lif = q->lif;

    // Skip until you reach the SG Element where the Actual Packet
    // Starts from
    packet_len = NET_BUFFER_DATA_LENGTH(packet);

    data_offset = (UINT)NET_BUFFER_CURRENT_MDL_OFFSET(packet);

    for (index = 0; index < frag_list->NumberOfElements; ++index) {
        if (frag_list->Elements[index].Length <= data_offset) {
            data_offset -= frag_list->Elements[index].Length;
        } else {
            break;
        }
    }
    sg_element = &frag_list->Elements[index];
    sg_element_count = frag_list->NumberOfElements - index;

    pHdrBuffer =
        MmGetSystemAddressForMdlSafe(packet->CurrentMdl, NormalPagePriority);

    ASSERT(pHdrBuffer != NULL);
    ASSERT(packet->CurrentMdl->ByteCount >= data_offset);

    pHdrBuffer = (void *)((char *)pHdrBuffer + data_offset);

    frame_type = get_frame_type(pHdrBuffer);

	if( !is_1q_hdr_present( pHdrBuffer)) { // Don't overwrite an existing .1q header
		vlan_pri_info.Value =
			NET_BUFFER_LIST_INFO(parent_nbl, Ieee8021QNetBufferListInfo);

		DbgTrace((TRACE_COMPONENT_VLAN_PRI, TRACE_LEVEL_VERBOSE,
				  "%s VLAN %d Lif %d\n", __FUNCTION__, lif->vlan_id, lif->index));

		if (vlan_pri_info.TagHeader.VlanId == 0 && lif->vlan_id != 0) {
			vlan_tag = ETH_MAKE_VLAN(vlan_pri_info.TagHeader.UserPriority, 0,
									 lif->vlan_id);
		} else {
			vlan_tag = ETH_MAKE_VLAN(vlan_pri_info.TagHeader.UserPriority, 0,
									 vlan_pri_info.TagHeader.VlanId);
		}
		vlan_tag_insert = vlan_tag ? TRUE : FALSE;

		if (vlan_tag_insert) {
			DbgTrace((TRACE_COMPONENT_VLAN_PRI, TRACE_LEVEL_VERBOSE,
					  "%s Queue Packet VLAN adapter %p VLAN %d Priority %d Passed "
					  "%d Curr %d Lif %d\n",
					  __FUNCTION__, ionic, ETH_GET_VLAN_ID(vlan_tag),
					  ETH_GET_VLAN_PRIORITY(vlan_tag),
					  vlan_pri_info.TagHeader.VlanId, lif->vlan_id, lif->index));
			tx_stats->vlan_inserted++;
		}
	}

    mss.Value = NET_BUFFER_LIST_INFO(parent_nbl, TcpLargeSendNetBufferListInfo);
    csum_info.Value =
        NET_BUFFER_LIST_INFO(parent_nbl, TcpIpChecksumNetBufferListInfo);

    if (mss.Transmit.Type == NDIS_TCP_LARGE_SEND_OFFLOAD_V1_TYPE &&
        mss.LsoV1Transmit.MSS != 0) {
        lso_pkt = TRUE;

        DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
                  "%s Queue Packet LSOV1 TcpHeaderOffset %d MSS %d\n",
                  __FUNCTION__, mss.LsoV1Transmit.TcpHeaderOffset,
                  mss.LsoV1Transmit.MSS));
    } else if (mss.Transmit.Type == NDIS_TCP_LARGE_SEND_OFFLOAD_V2_TYPE) {
        lso_pkt = TRUE;

        DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
                  "%s Queue Packet LSOV2 TcpHeaderOffset %d MSS %d\n",
                  __FUNCTION__, mss.LsoV2Transmit.TcpHeaderOffset,
                  mss.LsoV2Transmit.MSS));
    } else {

        if (packet_len > (IONIC_TX_MAX_SG_ELEMS + 1) * PAGE_SIZE) {
            status = NDIS_STATUS_INVALID_PACKET;
            DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_ERROR,
                      "%s Packet Length > max size %d adapter %p size %d\n",
                      __FUNCTION__, (IONIC_TX_MAX_SG_ELEMS + 1) * PAGE_SIZE,
                      packet_len));
            goto cleanup;
        }

        if (sg_element_count >
            IONIC_TX_MAX_SG_ELEMS + 1) { // 1 primary and 8 secondary elements

            DbgTrace((TRACE_COMPONENT_IO_FRAG, TRACE_LEVEL_VERBOSE,
                      "%s SG Count > max %d packet size %d SG cnt %d\n",
                      __FUNCTION__, IONIC_TX_MAX_SG_ELEMS + 1, packet_len,
                      sg_element_count));

            status = copy_buffer(ionic, packet, &txq_pkt->tx_frag_elem);

            if (status != NDIS_STATUS_SUCCESS) {
                DbgTrace((TRACE_COMPONENT_IO_FRAG, TRACE_LEVEL_ERROR,
                          "%s Failed to copy buffer status %08lX\n",
                          __FUNCTION__, status));
                status = NDIS_STATUS_INVALID_PACKET;
                goto cleanup;
            }

            sg_element_count =
                txq_pkt->tx_frag_elem->tx_frag_list->NumberOfElements;
            sg_element = &txq_pkt->tx_frag_elem->tx_frag_list->Elements[0];
            data_offset = 0;
        }
    }

    if (csum_info.Transmit.IsIPv4 || csum_info.Transmit.IsIPv6) {

        DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
                  "%s Queue Packet Checksum IPv4 %s IPv6 %s TCPChecksum %s "
                  "UDPChecksum %s IPChecksum %s\n",
                  __FUNCTION__, csum_info.Transmit.IsIPv4 ? "Yes" : "No",
                  csum_info.Transmit.IsIPv6 ? "Yes" : "No",
                  csum_info.Transmit.TcpChecksum ? "Yes" : "No",
                  csum_info.Transmit.UdpChecksum ? "Yes" : "No",
                  csum_info.Transmit.IpHeaderChecksum ? "Yes" : "No"));

        csum_l4 = (BOOLEAN)(csum_info.Transmit.TcpChecksum |
                            csum_info.Transmit.UdpChecksum);
        csum_l3 = (BOOLEAN)csum_info.Transmit.IpHeaderChecksum;

        if (csum_l3 || csum_l4) {
            tx_stats->csum_hw++;
        }

        supplemental_info.Value = NET_BUFFER_LIST_INFO(
            parent_nbl, TcpSendOffloadsSupplementalNetBufferListInfo);
    } else if (lso_pkt) {
        supplemental_info.Value = NET_BUFFER_LIST_INFO(
            parent_nbl, TcpSendOffloadsSupplementalNetBufferListInfo);
    } else {
        tx_stats->csum_none++;
    }

    if (supplemental_info.EncapsulatedPacketOffsets.IsEncapsulatedPacket) {
        ASSERT(FALSE);
    }

    if (!ionic_q_has_space(&qcq->q, txq_pkt_private->desc_cnt)) {

        DbgTrace((TRACE_COMPONENT_TX_NONLSO, TRACE_LEVEL_VERBOSE,
                  "%s Queue packet %p len %08lX sg cnt %d to NB pended list\n",
                  __FUNCTION__, packet, packet_len, sg_element_count));
        status = NDIS_STATUS_PENDING;
        goto cleanup;
    }

    if (!lso_pkt) {

        txq_pkt_private->current_desc_cnt = 1;
        txq_pkt_private->bytes_processed = packet_len;

        DbgTrace((TRACE_COMPONENT_TX_NONLSO, TRACE_LEVEL_VERBOSE,
                  "%s Processing non-TSO packet %p len %08lX sg cnt %d\n",
                  __FUNCTION__, packet, packet_len, sg_element_count));

        ionic_enqueue_txq_pkt(qcq, packet, packet_len, data_offset, sg_element,
                              sg_element_count, csum_offset, vlan_tag,
                              vlan_tag_insert, csum_l3, csum_l4);

        tx_stats->doorbell_count++;

        goto cleanup;
    }

    //
    // Handle the TSO packet processing
    // We insert the header, which is contained in the first MDL, and up to MSS
    // size of data in the first tx descriptor
    //

    txq_pkt_private->current_desc_cnt = 0;
    txq_pkt_private->bytes_processed = 0;

    sge_current = sg_element;

    ullCurrentPhysAddr = sge_current->Address.QuadPart + data_offset;
    ulCurrentPhysLen = sge_current->Length - data_offset;

    pCurrentMdl = (PMDL)NET_BUFFER_CURRENT_MDL(packet);

    ulCurrentLen = pCurrentMdl->ByteCount - data_offset;
    ulHdrLen = ulCurrentLen;

    ulRemainingLen = packet_len;

    DbgTrace((TRACE_COMPONENT_TX_LSO, TRACE_LEVEL_VERBOSE,
              "%s Processing TSO packet %p len %08lX desc cnt %d sg cnt %d "
              "phys addr %I64X len %08lX hdr len %08lX L3 %s L4 %s\n",
              __FUNCTION__, packet, packet_len, txq_pkt_private->desc_cnt,
              sg_element_count, ullCurrentPhysAddr, ulCurrentLen, ulHdrLen,
              csum_l3 ? "Yes" : "No", csum_l4 ? "Yes" : "No"));

    bStart = TRUE;
    bDone = FALSE;

    sge = sge_current;

    segment_len = mss.LsoV2Transmit.MSS + ulHdrLen;
    len = 0;
    segment_len_acc = 0;

    ulDescCnt = 0;

    while (sg_element_count != 0) {

        segment_len_acc = 0;
        sg_len = 0;
        sge_count = 1;

        while (segment_len != 0) {

            ASSERT(ulCurrentPhysLen <= ulRemainingLen);

            len = min(segment_len, ulCurrentPhysLen);

            if (sg_len == 0) {
                sg_len = min(segment_len, ulRemainingLen) - len;
            }

            segment_len -= len;
            ulCurrentPhysLen -= len;
            segment_len_acc += len;

            if (ulCurrentPhysLen == 0 && segment_len != 0) {

                sg_element_count--;

                if (sg_element_count == 0) {
                    DbgTrace((TRACE_COMPONENT_IO_FRAG, TRACE_LEVEL_ERROR,
                              "%s Invalid sg element count\n", __FUNCTION__));
                    status = NDIS_STATUS_INVALID_PACKET;
                    break;
                }

                sge_current++; // Go to the next sg element
                sge_count++;
                ulCurrentPhysLen = sge_current->Length;
            }

            ASSERT(len <= ulRemainingLen);
            ulRemainingLen -= len;

            if (ulRemainingLen == 0) {
                bDone = TRUE;
                tx_stats->doorbell_count++;
            }
        }

        if (status != NDIS_STATUS_SUCCESS) {
            break;
        }

        txq_pkt_private->bytes_processed += segment_len_acc;

        DbgTrace((TRACE_COMPONENT_TX_LSO, TRACE_LEVEL_VERBOSE,
                  "Descriptor %d sg cnt %d phys addr %I64X len %08lX add'd sg "
                  "len %08lX Remain %08lX\n",
                  ulDescCnt, sge_count, ullCurrentPhysAddr, segment_len_acc,
                  sg_len, ulRemainingLen));

        ionic_enqueue_txq_lso_pkt(qcq, packet, ullCurrentPhysAddr, sge,
                                  sge_count, sg_len, segment_len_acc, ulHdrLen,
                                  mss.LsoV2Transmit.MSS, false, vlan_tag,
                                  vlan_tag_insert, bStart, bDone, bdumpdescr);

        ulDescCnt++;

        if (bDone) {
            ASSERT(ulDescCnt == txq_pkt_private->desc_cnt);
            break;
        }

		ASSERT( ulDescCnt < txq_pkt_private->desc_cnt);

        ASSERT(sg_element_count != 0);

        segment_len = mss.LsoV2Transmit.MSS;
        if (segment_len > ulRemainingLen) {
            segment_len = ulRemainingLen;
        }
        bStart = FALSE;

        if (ulCurrentPhysLen == 0) {

            sg_element_count--;

            if (sg_element_count == 0) {
                DbgTrace((TRACE_COMPONENT_IO_FRAG, TRACE_LEVEL_ERROR,
                          "%s Invalid sg element count\n", __FUNCTION__));
                status = NDIS_STATUS_INVALID_PACKET;
                break;
            }
            sge_current++; // Go to the next sg element
            ulCurrentPhysLen = sge_current->Length;
        }

        ASSERT(sge_current->Length >= ulCurrentPhysLen);

        ullCurrentPhysAddr = sge_current->Address.QuadPart +
                             (sge_current->Length - ulCurrentPhysLen);
        sge = sge_current;
    }

    if (status == NDIS_STATUS_SUCCESS) {
        // Report only the data payload back to the LSO offload completion.
        // This does not include the header
        txq_pkt_private->bytes_processed -= ulHdrLen;
        tx_stats->tso_packets++;
        tx_stats->tso_bytes += packet_len;

		if (packet_len > tx_stats->max_tso_sz) {
			tx_stats->max_tso_sz = packet_len;
		}

		tx_stats->last_tso_sz = packet_len;
    }

    DbgTrace((TRACE_COMPONENT_TX_LSO, TRACE_LEVEL_VERBOSE,
              "%s Processing packet %p complete status %08lX\n", __FUNCTION__,
              packet, status));

cleanup:

    if (status == NDIS_STATUS_SUCCESS) {

        switch (frame_type) {

        case NDIS_PACKET_TYPE_BROADCAST: {
            tx_stats->bcast_packets++;
            tx_stats->bcast_bytes += packet_len;
            break;
        }

        case NDIS_PACKET_TYPE_MULTICAST: {
            tx_stats->mcast_packets++;
            tx_stats->mcast_bytes += packet_len;
            break;
        }

        default: {
            tx_stats->directed_packets++;
            tx_stats->directed_bytes += packet_len;
            break;
        }
        }
    }

    DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
              "%s Exit Queue TxQ PKt adapter %p NB %p C-NB %p\n", __FUNCTION__,
              ionic, packet_orig, packet));

    return status;
}

void
ionic_send_complete(struct ionic *ionic, struct txq_nbl_list *list, KIRQL irql)
{
    ULONG flags = 0;

    if (list->head) {
        if (irql == DISPATCH_LEVEL) {
            flags = NDIS_SEND_COMPLETE_FLAGS_DISPATCH_LEVEL;
        }

        DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
                  "%s Send Complete adapter %p NBLcount %d\n", __FUNCTION__,
                  ionic, list->count));

        NdisMSendNetBufferListsComplete(ionic->adapterhandle, list->head,
                                        flags);

        deref_request( ionic, list->count);
    }
}

void
ionic_txq_nbl_list_push_tail(struct txq_nbl_list *list, PNET_BUFFER_LIST nbl)
{
    if (list->head) {
        NET_BUFFER_LIST_NEXT_NBL(list->tail) = nbl;
    } else {
        list->head = nbl;
    }
    NET_BUFFER_LIST_NEXT_NBL(nbl) = NULL;
    list->tail = nbl;
    list->count++;
}

void
ionic_return_txq_pkt(struct qcq *qcq, struct txq_pkt *txq_pkt)
{

    txq_pkt->next.Next = NULL;
	
    InterlockedIncrement(&qcq->tx_pkts_free_count);

	InterlockedPushEntrySList( qcq->tx_pkt_list,
							   &txq_pkt->next);
    return;
}

PNET_BUFFER_LIST
ionic_txq_nbl_list_pop_head(struct txq_nbl_list *list)
{
    PNET_BUFFER_LIST nbl = NULL;

    if (list->head) {
        nbl = list->head;
        list->head = NET_BUFFER_LIST_NEXT_NBL(nbl);
        if (!list->head) {
            list->tail = NULL;
        }
        NET_BUFFER_LIST_NEXT_NBL(nbl) = NULL;
        list->count--;
    }
    return nbl;
}

void
ionic_txq_complete_failed_pkt(struct ionic *ionic,
                              struct qcq *qcq,
							  PNET_BUFFER_LIST parent_nbl,
                              PNET_BUFFER packet,
                              struct txq_nbl_list *completed_list,
                              NDIS_STATUS status)
{
    struct txq_pkt_private *txq_pkt_private =
        (struct txq_pkt_private *)NET_BUFFER_MINIPORT_RESERVED(packet);
    struct txq_pkt *txq_pkt = txq_pkt_private->txq_pkt;
    LONG ref_cnt;

    DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
              "%s Enter TXQ complete failed pkt adapter %p NBL %p NB %p\n",
              __FUNCTION__, ionic, parent_nbl, packet));

    if (txq_pkt != NULL) {
        if (txq_pkt->sg_os_list && (txq_pkt->sg_os_list != txq_pkt->sg_list)) {
            NdisMFreeNetBufferSGList(ionic->dma_handle, txq_pkt->sg_os_list,
                                     txq_pkt->packet_orig);
        } else if( (txq_pkt->flags & NET_BUFFER_SG_LIST_CREATED) != 0) {
            NdisMFreeNetBufferSGList(ionic->dma_handle, txq_pkt->sg_list,
                                     txq_pkt->packet_orig);
        }
		txq_pkt->sg_os_list = NULL;

        DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
                  "%s Returning failed packet\n", __FUNCTION__));

        ionic_return_txq_pkt(qcq, txq_pkt);
    }
	/* Don't reset a previously set failure status */
	if (NET_BUFFER_LIST_STATUS(parent_nbl) == NDIS_STATUS_SUCCESS) {
		NET_BUFFER_LIST_STATUS(parent_nbl) = status;
	}

    ref_cnt = NET_BUFFER_LIST_DEC_REF_COUNTER(parent_nbl);
    if (!ref_cnt) {
        DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
                  "%s TXQ adapter %p NBL %p Complete\n", __FUNCTION__, ionic,
                  parent_nbl));
        ionic_txq_nbl_list_push_tail(completed_list, parent_nbl);
    }

    DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
              "%s Exit TXQ adapter %p NB %p\n", __FUNCTION__, ionic, packet));
}

void
ionic_txq_nbl_list_init(struct txq_nbl_list *list)
{
    list->head = list->tail = NULL;
    list->count = 0;
}

void
ionic_free_txq_pkts(struct ionic *ionic, struct qcq *qcq)
{
    ASSERT(ionic != NULL);
    ASSERT(qcq != NULL);

    // Final Freeup no locks are required. No IO happening
    if (qcq->txq_base != NULL) {

        NdisFreeMemoryWithTagPriority_internal(
            ionic->adapterhandle, qcq->txq_base, IONIC_TX_MEM_TAG);
        qcq->txq_base = NULL;
        qcq->tx_pkts_free_count = 0;
    }

	if (qcq->tx_pkt_list != NULL) {		
        NdisFreeMemoryWithTagPriority_internal(
            ionic->adapterhandle, qcq->tx_pkt_list, IONIC_GENERIC_TAG);
        qcq->tx_pkt_list = NULL;
	}

    if (qcq->tx_sgl_buffer != NULL) {
        NdisFreeMemoryWithTagPriority_internal(ionic->adapterhandle, qcq->tx_sgl_buffer,
                                      IONIC_TX_MEM_TAG);
        qcq->tx_sgl_buffer = NULL;
    }

    if (qcq->tx_pkts_pool != NULL) {
        NdisFreeNetBufferListPool(qcq->tx_pkts_pool);
        qcq->tx_pkts_pool = NULL;
    }
}

struct txq_pkt *
ionic_get_next_txq_pkt(struct qcq *qcq)
{
    struct txq_pkt *txq_pkt = NULL;

	txq_pkt = (struct txq_pkt *)InterlockedPopEntrySList( qcq->tx_pkt_list);

	if (txq_pkt != NULL) {
		ASSERT( qcq->tx_pkts_free_count > 0);
		InterlockedDecrement(&qcq->tx_pkts_free_count);
	}

    return txq_pkt;
}

UCHAR
ionic_txq_nbl_list_is_empty(struct txq_nbl_list *list)
{
    return (list->head == NULL) ? 1 : 0;
}

void
ionic_send_packets(NDIS_HANDLE adapter_context,
                   PNET_BUFFER_LIST pnetlist,
                   NDIS_PORT_NUMBER port_number,
                   ULONG send_flags)
{
    struct ionic *ionic = (struct ionic *)adapter_context;
    PNET_BUFFER_LIST nbl, next_nbl = NULL, last_nbl = NULL;
    PNET_BUFFER nb, next_nb;
    BOOLEAN is_dispatch =
        (send_flags & NDIS_SEND_FLAGS_DISPATCH_LEVEL) ? TRUE : FALSE;
    ULONG num_nbls = 0;
    ULONG nb_ref;
    struct lif *lif;
    struct qcq *qcq;
    KIRQL old_irql;
    NDIS_STATUS status;
    NDIS_TCP_LARGE_SEND_OFFLOAD_NET_BUFFER_LIST_INFO mss = {0};
    struct txq_pkt_private *txq_pkt_private;
    ULONG lif_id = 0;
    ULONG filter_id = 0;
    txq_nbl_list failed_list;
    NDIS_TCP_IP_CHECKSUM_NET_BUFFER_LIST_INFO csum_info = {0};
    BOOLEAN fail_nbl = FALSE;
    ULONG tx_queue = 0;
    struct txq_nbl_private *nbl_private = NULL;

    UNREFERENCED_PARAMETER(port_number);

    ionic_txq_nbl_list_init(&failed_list);

    filter_id = NET_BUFFER_LIST_RECEIVE_FILTER_ID(pnetlist);

    for (nbl = pnetlist; nbl != NULL; nbl = next_nbl, ++num_nbls) {
        next_nbl = NET_BUFFER_LIST_NEXT_NBL(nbl);
        // Ref count the adapter for an outstanding io
        ref_request(ionic);
    }

    //
    // Are we running in srvio mode?
    //

    if (BooleanFlagOn(ionic->ConfigStatus, IONIC_SRIOV_MODE)) {
        lif_id = NET_BUFFER_LIST_RECEIVE_FILTER_VPORT_ID(pnetlist);
        lif = ionic->SriovSwitch.Ports[lif_id].lif;
        ASSERT(BooleanFlagOn(ionic->SriovSwitch.Ports[lif_id].Flags,
                             IONIC_VPORT_STATE_ACTIVATED));
    } else {
        lif_id = NET_BUFFER_LIST_RECEIVE_QUEUE_ID(pnetlist);
        lif =
            ionic->vm_queue[lif_id].lif; // queue id == 0 is the default queue
    }

    ASSERT(lif != NULL);

    DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
              "%s Enter Send Packets adapter %p Flags %08lX lif %d lif id %d "
              "filter id %d\n",
              __FUNCTION__, ionic, send_flags, lif->index, lif_id,filter_id));

    // check the hardware status and complete the Packet with proper
    // Error Code

    if (ionic->hardware_status != NdisHardwareStatusReady ||
        BooleanFlagOn(ionic->Flags, IONIC_FLAG_PAUSED)) {
        for (nbl = pnetlist; nbl != NULL; nbl = next_nbl, ++num_nbls) {
            next_nbl = NET_BUFFER_LIST_NEXT_NBL(nbl);
            NET_BUFFER_LIST_NEXT_NBL(nbl) = NULL;

            status = (ionic->hardware_status == NdisHardwareStatusClosing)
                         ? NDIS_STATUS_SEND_ABORTED
                         : NDIS_STATUS_ADAPTER_NOT_READY;

            NET_BUFFER_LIST_STATUS(nbl) = status;

            NdisMSendNetBufferListsComplete(
                ionic->adapterhandle, nbl,
                is_dispatch ? NDIS_SEND_COMPLETE_FLAGS_DISPATCH_LEVEL : 0);
            deref_request(ionic, 1);
        }
        DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
                  "%s Send Packets adapter %p Hardware Not Ready\n",
                  __FUNCTION__, ionic));
        return;
    }

    // Validate that if we are asked to perform offload that it is currently
    // enabled
    for (nbl = pnetlist; nbl != NULL; nbl = next_nbl, ++num_nbls) {
        next_nbl = NET_BUFFER_LIST_NEXT_NBL(nbl);

        fail_nbl = FALSE;

        mss.Value = NET_BUFFER_LIST_INFO(nbl, TcpLargeSendNetBufferListInfo);

        csum_info.Value =
            NET_BUFFER_LIST_INFO(nbl, TcpIpChecksumNetBufferListInfo);

        if (csum_info.Transmit.UdpChecksum) {

            if (csum_info.Transmit.IsIPv4 &&
                ionic->udpv4_tx_state == NDIS_OFFLOAD_SET_OFF) {
                fail_nbl = TRUE;
            } else if (csum_info.Transmit.IsIPv6 &&
                       ionic->udpv6_tx_state == NDIS_OFFLOAD_SET_OFF) {
                fail_nbl = TRUE;
            }
        }

        if (!fail_nbl && csum_info.Transmit.TcpChecksum) {

            if (csum_info.Transmit.IsIPv4 &&
                ionic->tcpv4_tx_state == NDIS_OFFLOAD_SET_OFF) {
                fail_nbl = TRUE;
            } else if (csum_info.Transmit.IsIPv6 &&
                       ionic->tcpv6_tx_state == NDIS_OFFLOAD_SET_OFF) {
                fail_nbl = TRUE;
            }
        }

        if (!fail_nbl && csum_info.Transmit.IpHeaderChecksum) {

            if (csum_info.Transmit.IsIPv4 &&
                ionic->ipv4_tx_state == NDIS_OFFLOAD_SET_OFF) {
                fail_nbl = TRUE;
            }
        }

        if (!fail_nbl && mss.LsoV1Transmit.MSS != 0) {
            if (mss.Transmit.Type == NDIS_TCP_LARGE_SEND_OFFLOAD_V1_TYPE) {
                if (ionic->lsov1_state ==
                    NDIS_OFFLOAD_PARAMETERS_LSOV1_DISABLED) {
                    fail_nbl = TRUE;
                }
            } else {
                if (mss.LsoV2Transmit.IPVersion ==
                        NDIS_TCP_LARGE_SEND_OFFLOAD_IPv4 &&
                    ionic->lsov2ipv4_state ==
                        NDIS_OFFLOAD_PARAMETERS_LSOV2_DISABLED) {
                    fail_nbl = TRUE;
                } else if (mss.LsoV2Transmit.IPVersion ==
                               NDIS_TCP_LARGE_SEND_OFFLOAD_IPv6 &&
                           ionic->lsov2ipv6_state ==
                               NDIS_OFFLOAD_PARAMETERS_LSOV2_DISABLED) {
                    fail_nbl = TRUE;
                }
            }
        }

        if (fail_nbl) {

            if (last_nbl == NULL) {
                pnetlist = next_nbl;
            } else {
                NET_BUFFER_LIST_NEXT_NBL(last_nbl) = next_nbl;
            }

            NET_BUFFER_LIST_NEXT_NBL(nbl) = NULL;
            NET_BUFFER_LIST_STATUS(nbl) = NDIS_STATUS_INVALID_PARAMETER;

            NdisMSendNetBufferListsComplete(
                ionic->adapterhandle, nbl,
                is_dispatch ? NDIS_SEND_COMPLETE_FLAGS_DISPATCH_LEVEL : 0);
            deref_request(ionic, 1);
        } else {
            last_nbl = nbl;

			nb_ref = 0;
			for (nb = NET_BUFFER_LIST_FIRST_NB(nbl); nb; nb = next_nb) {

				next_nb = NET_BUFFER_NEXT_NB(nb);
				txq_pkt_private =
					(struct txq_pkt_private *)NET_BUFFER_MINIPORT_RESERVED(nb);

				/* xxx - update list entry normally. */
				txq_pkt_private->link.Flink = NULL;
				txq_pkt_private->link.Blink = NULL;
				txq_pkt_private->txq_pkt = NULL;
				txq_pkt_private->desc_cnt = 0;
				txq_pkt_private->current_desc_cnt = 0;
				txq_pkt_private->bytes_processed = 0;

				nb_ref++;
			}
			NET_BUFFER_LIST_SET_REF_COUNTER(nbl, nb_ref);
			NET_BUFFER_LIST_STATUS(nbl) = NDIS_STATUS_SUCCESS;

			DbgTrace((TRACE_COMPONENT_PENDING_LIST, TRACE_LEVEL_VERBOSE,
				  "%s Adapter %p num_nbs %d nbl %p\n", __FUNCTION__,
				  ionic, nb_ref, nbl));
        }
    }

    NDIS_RAISE_IRQL_TO_DISPATCH(&old_irql);

    DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
              "%s Send Packets adapter %p num_nbls %d nbl %p\n", __FUNCTION__,
              ionic, num_nbls, pnetlist));        

    for (nbl = pnetlist; nbl; nbl = next_nbl) {
       
        ULONG_PTR val = (ULONG_PTR)NET_BUFFER_LIST_INFO( nbl, NetBufferListHashValue);
        tx_queue = val % lif->ntxqs;

        nbl_private = (struct txq_nbl_private *)NET_BUFFER_LIST_MINIPORT_RESERVED( nbl);
		nbl_private->nb_processed_cnt = 0;

        next_nbl = NET_BUFFER_LIST_NEXT_NBL(nbl);
        NET_BUFFER_LIST_NEXT_NBL(nbl) = NULL;

        if (BooleanFlagOn(ionic->ConfigStatus, IONIC_SRIOV_MODE)) {
            lif_id = NET_BUFFER_LIST_RECEIVE_FILTER_VPORT_ID(nbl);
            lif = ionic->SriovSwitch.Ports[lif_id].lif;
        } else {
            lif_id = NET_BUFFER_LIST_RECEIVE_QUEUE_ID(nbl);
            lif = ionic->vm_queue[lif_id].lif;
        }

        qcq = lif->txqcqs[tx_queue].qcq;

        NdisDprAcquireSpinLock(&qcq->txq_pending_nbl_lock);
       
        if (!ionic_txq_nbl_list_is_empty(&qcq->txq_pending_nbl)) {

            DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
                      "%s PEND send Packets adapter %p NBL %p\n", __FUNCTION__,
                      ionic, nbl));

            ionic_txq_nbl_list_push_tail(&qcq->txq_pending_nbl, nbl);
            InterlockedIncrement( (LONG *)&qcq->tx_stats->pending_nbl_count);
        } else {

            status = process_nbl(ionic, qcq, nbl, &failed_list);

            if (status == NDIS_STATUS_PENDING) {
                ionic_txq_nbl_list_push_tail(&qcq->txq_pending_nbl, nbl);
                InterlockedIncrement( (LONG *)&qcq->tx_stats->pending_nbl_count);
            }
        }

        NdisDprReleaseSpinLock(&qcq->txq_pending_nbl_lock);
    }    

    ionic_send_complete(ionic, &failed_list, DISPATCH_LEVEL);

    NDIS_LOWER_IRQL(old_irql, DISPATCH_LEVEL);

    DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
              "%s Exit Send Packets adapter %p num_nbls %d\n", __FUNCTION__,
              ionic, num_nbls));

    return;
}

void
ionic_tx_release_pending(struct ionic *ionic, struct qcq * qcq)
{

	struct txq_nbl_private *nbl_private;
    struct txq_nbl_list completed_list;
    PNET_BUFFER_LIST cur_nbl;
    PNET_BUFFER cur_nb, next_nb;
	LONG	processed_cnt = 0;

    ASSERT(ionic != NULL);

    DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
              "%s Enter clean TXQ adapter %p\n", __FUNCTION__, ionic));

    ionic_txq_nbl_list_init(&completed_list);

    NdisAcquireSpinLock(&qcq->txq_pending_nbl_lock);
    cur_nbl = ionic_txq_nbl_list_pop_head(&qcq->txq_pending_nbl);
    while (cur_nbl) {

        InterlockedDecrement( (LONG *)&qcq->tx_stats->pending_nbl_count);
		nbl_private = (struct txq_nbl_private *)NET_BUFFER_LIST_MINIPORT_RESERVED( cur_nbl);

		for (cur_nb = NET_BUFFER_LIST_FIRST_NB(cur_nbl); cur_nb; cur_nb = next_nb) {

			next_nb = NET_BUFFER_NEXT_NB(cur_nb);
			
			if (processed_cnt < nbl_private->nb_processed_cnt) {
				processed_cnt++;
				continue;
			}
            
			DbgTrace((TRACE_COMPONENT_PENDING_LIST,
                        TRACE_LEVEL_VERBOSE, "%s Cleaning nb %p\n",
                        __FUNCTION__, cur_nb));
            ionic_txq_complete_failed_pkt(ionic, 
										  qcq,
										  cur_nbl,
										  cur_nb,
										  &completed_list,
										  NDIS_STATUS_SEND_ABORTED);
        }
        cur_nbl = ionic_txq_nbl_list_pop_head(&qcq->txq_pending_nbl);
    }

    NdisReleaseSpinLock(&qcq->txq_pending_nbl_lock);

    ionic_send_complete(ionic, &completed_list, NDIS_CURRENT_IRQL());

    DbgTrace((TRACE_COMPONENT_IO, TRACE_LEVEL_VERBOSE,
              "%s Exit clean TXQ adapter %p completed count %d\n", __FUNCTION__,
              ionic, completed_list.count));
}

ULONG
get_queue_len(struct qcq *qcq)
{
    ULONG   len = 0;
    len = (qcq->q.head->index - qcq->q.tail->index) & (qcq->q.num_descs - 1);
    return len;
}

bool
ionic_tx_flush(struct qcq *qcq, unsigned int budget, bool cleanup, bool credits)
{
    struct cq *cq = &qcq->cq;
    struct ionic_dev *idev = &cq->lif->ionic->idev;
    struct txq_comp *comp = NULL;
    struct queue *q = cq->bound_q;
    struct desc_info *desc_info;
    unsigned int work_done = 0;
    void *cb_arg;
    desc_cb cb;
    ULONG desc_cnt = 0;
    ULONG flags = 0;

#ifdef DBG
    ULONG queue_len = 0;
    queue_len = get_queue_len( qcq);
    if( queue_len != 0) {
        InterlockedAdd( (LONG *)&qcq->tx_stats->descriptor_count,
                        (LONG)queue_len);
    }
    InterlockedIncrement( (LONG *)&qcq->tx_stats->descriptor_sample);
    if (queue_len > qcq->tx_stats->descriptor_max) {
        qcq->tx_stats->descriptor_max = queue_len;
    }
#endif

    comp = (struct txq_comp *)cq->tail->cq_desc;

    /* walk the completed cq entries */
    while (work_done < budget &&
           color_match(comp->color, cq->done_color)) {

        /* clean the related q entries, there could be
         * several q entries completed for each cq completion
         */
        do {
            desc_info = q->tail;
            q->tail = desc_info->next;
            if (cleanup) {
                desc_info->cb = NULL;
                desc_info->cb_arg = NULL;
            } else {

                cb = desc_info->cb;
                cb_arg = desc_info->cb_arg;

                desc_info->cb = NULL;
                desc_info->cb_arg = NULL;

                if (cb)
                    cb(q, desc_info, cq->tail, cb_arg, NULL, NULL);

                desc_cnt++;
            }

        } while (desc_info->index != le16_to_cpu(comp->comp_index));

        if (desc_cnt > 1) {
            // DumpCompDesc( comp);
        }

        if (cq->tail->last)
            cq->done_color = !cq->done_color;

        cq->tail = cq->tail->next;
        comp = (struct txq_comp *)cq->tail->cq_desc;

        work_done++;
    }

    if (credits &&
        ((work_done != 0) || (flags != 0)))
        ionic_intr_credits(idev->intr_ctrl, cq->bound_intr->index, work_done,
                           flags);

    return (work_done == budget);
}

NDIS_STATUS
ionic_alloc_txq_pkts(struct ionic *ionic, struct qcq *qcq)
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    NET_BUFFER_LIST_POOL_PARAMETERS pool_params;
    struct queue *q = &qcq->q;
    ULONG size;
    struct txq_pkt *tx_pkt;
	ULONG pkt_len = 0;

    ASSERT(ionic != NULL);
    ASSERT(q != NULL);

    NdisZeroMemory(&pool_params, sizeof(NET_BUFFER_LIST_POOL_PARAMETERS));
    pool_params.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
    pool_params.Header.Revision = NET_BUFFER_LIST_POOL_PARAMETERS_REVISION_1;
    pool_params.Header.Size =
        NDIS_SIZEOF_NET_BUFFER_LIST_POOL_PARAMETERS_REVISION_1;
    pool_params.ProtocolId = 0;
    pool_params.ContextSize = 0;
    pool_params.fAllocateNetBuffer = TRUE;
    pool_params.PoolTag = IONIC_TX_MEM_TAG;

    qcq->tx_pkts_pool =
        NdisAllocateNetBufferListPool(ionic->adapterhandle, &pool_params);

    if (qcq->tx_pkts_pool == NULL) {
        status = NDIS_STATUS_RESOURCES;
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Failed to alloc txq packet pool adapter %p.\n",
                  __FUNCTION__, ionic));
        goto err_alloc_failed;
    }

	pkt_len = ALIGN_SZ( sizeof(struct txq_pkt), MEMORY_ALLOCATION_ALIGNMENT);
    size = q->num_descs * pkt_len;
    qcq->txq_base =
        (struct txq_pkt *)NdisAllocateMemoryWithTagPriority_internal(
            ionic->adapterhandle, size, IONIC_TX_MEM_TAG, NormalPoolPriority);

    if (qcq->txq_base == NULL) {
        status = NDIS_STATUS_RESOURCES;
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Failed to alloc txq pkt adapter %p array size %d\n",
                  __FUNCTION__, ionic, size));
        goto err_alloc_failed;
    }
    NdisZeroMemory(qcq->txq_base, size);

    DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                  "%s Allocated 0x%p len %08lX\n",
                  __FUNCTION__,
                  qcq->txq_base, size));
    
    size = q->num_descs * ionic->sgl_size_in_bytes;
    qcq->tx_sgl_buffer = NdisAllocateMemoryWithTagPriority_internal(
        ionic->adapterhandle, size, IONIC_TX_MEM_TAG, NormalPoolPriority);

    if (!qcq->tx_sgl_buffer) {
        DbgTrace((
            TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
            "%s Failed to alloc txq sgl buffer list array adapter %p size %d\n",
            __FUNCTION__, ionic, size));
        goto err_alloc_failed;
    }

    NdisZeroMemory(qcq->tx_sgl_buffer, size);

    DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                  "%s Allocated 0x%p len %08lX\n",
                  __FUNCTION__,
                  qcq->tx_sgl_buffer, size));

    qcq->tx_pkt_list = (SLIST_HEADER *)NdisAllocateMemoryWithTagPriority_internal(
        ionic->adapterhandle, sizeof( SLIST_HEADER), IONIC_GENERIC_TAG, NormalPoolPriority);

    if (!qcq->tx_pkt_list) {
        DbgTrace((
            TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
            "%s Failed to alloc txq packet list header adapter %p size %d\n",
            __FUNCTION__, ionic, size));
        goto err_alloc_failed;
    }

	InitializeSListHead( qcq->tx_pkt_list);   
    qcq->tx_pkts_free_count = 0;

    // No lock is required here since this is in the Init Path
    // And we dont have any race conditions.
	
	tx_pkt = (struct txq_pkt *)qcq->txq_base;

    for (unsigned int i = 0; i < q->num_descs; i++) {
      
        tx_pkt->next.Next = NULL;

        tx_pkt->sg_list = (PSCATTER_GATHER_LIST)(
            ((ULONG_PTR)qcq->tx_sgl_buffer) + (i * ionic->sgl_size_in_bytes));

		InterlockedPushEntrySList( qcq->tx_pkt_list,
								   &tx_pkt->next);

        qcq->tx_pkts_free_count++;

		tx_pkt = (struct txq_pkt *)((char *)tx_pkt + pkt_len);
    }

    return status;

err_alloc_failed:
    ionic_free_txq_pkts(ionic, qcq);
    return status;
}

NDIS_STATUS
ionic_tx_fill(struct ionic *ionic, struct qcqst *qcqst)
{
    ASSERT(ionic != NULL);
    ASSERT(qcqst != NULL);

    struct qcq *qcq = qcqst->qcq;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    unsigned int i;
    struct queue *q = &qcq->q;
    struct txq_pkt *txq_pkt;

    ASSERT(q != NULL);

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Fill the TXQ adapter %p NBuffers %d\n", __FUNCTION__, ionic,
              q->num_descs));

    txq_pkt = (struct txq_pkt *)FirstEntrySList(qcq->tx_pkt_list);
    for (i = 0; i < q->num_descs; i++) {
        txq_pkt->q = q;
        txq_pkt = (struct txq_pkt *)txq_pkt->next.Next;
    }

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Exit Fill the TXQ adapter %p NBuffers %d\n", __FUNCTION__,
              ionic, i));

    return status;
}

void
DumpTxPacket(void *packet, MDL *Mdl)
{

    u8 *pStream = (u8 *)Mdl->MappedSystemVa;
    u8 *pSrcMac = NULL;
    u8 *pDestMac = NULL;
    u8 *pICmp = NULL;
    u8 *pProto = NULL;

    if (Mdl->ByteCount >= 24) {

        pDestMac = pStream;

        pSrcMac = pStream + ETH_ALEN;

        pICmp = pStream + 14;

        if (*pICmp == 0x45) {

            pProto = pICmp + 9;

            if (*pProto == 1) {

                IoPrint("%s ******* Have ping packet %p Src "
                        "%02lX:%02lX:%02lX:%02lX:%02lX:%02lX Dst "
                        "%02lX:%02lX:%02lX:%02lX:%02lX:%02lX ********\n",
                        __FUNCTION__, packet, pSrcMac[0], pSrcMac[1],
                        pSrcMac[2], pSrcMac[3], pSrcMac[4], pSrcMac[5],
                        pDestMac[0], pDestMac[1], pDestMac[2], pDestMac[3],
                        pDestMac[4], pDestMac[5]);
            }
        }
    }

    return;
}

NDIS_STATUS
copy_buffer(struct ionic *ionic,
            NET_BUFFER *packet,
            struct tx_frag_pool_elem **frag)
{

    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    ULONG packet_len = 0;
    char *curr_target = NULL;
    char *curr_src = NULL;
    void *source_va = NULL;
    ULONG copy_len = 0;
    ULONG copied_len = 0;
    PMDL curr_mdl = NULL;
    ULONG mdl_len = 0;
    struct tx_frag_pool_elem *frag_elem = NULL;
    ULONG data_offset = 0;

    packet_len = NET_BUFFER_DATA_LENGTH(packet);

    if (packet_len > (3 * PAGE_SIZE)) {
        status = NDIS_STATUS_RESOURCES;
        goto cleanup;
    }

    frag_elem = get_tx_frag(ionic);

    if (frag_elem == NULL) {
        status = NDIS_STATUS_RESOURCES;
        goto cleanup;
    }

    data_offset = (UINT)NET_BUFFER_CURRENT_MDL_OFFSET(packet);

    curr_mdl = packet->CurrentMdl;
    curr_target = (char *)frag_elem->buffer;

    source_va = MmGetSystemAddressForMdlSafe(curr_mdl, NormalPagePriority);

    if (source_va == NULL) {
        status = NDIS_STATUS_RESOURCES;
        goto cleanup;
    }

    curr_src = (char *)source_va;
    mdl_len = curr_mdl->ByteCount;

    // Get to the start of the data
    while (data_offset != 0) {
        if (mdl_len >= data_offset) {
            curr_src += data_offset;
            mdl_len -= data_offset;

            if (mdl_len == 0) {
                curr_mdl = curr_mdl->Next;
                source_va = NULL;
            }

            break;
        }

        data_offset -= mdl_len;

        curr_mdl = curr_mdl->Next;

        source_va = MmGetSystemAddressForMdlSafe(curr_mdl, NormalPagePriority);

        if (source_va == NULL) {
            status = NDIS_STATUS_RESOURCES;
            goto cleanup;
        }

        curr_src = (char *)source_va;
        mdl_len = curr_mdl->ByteCount;
    }

    while (copied_len < packet_len) {

        if (source_va == NULL) {

            source_va =
                MmGetSystemAddressForMdlSafe(curr_mdl, NormalPagePriority);

            if (source_va == NULL) {
                status = NDIS_STATUS_RESOURCES;
                break;
            }

            curr_src = (char *)source_va;
            mdl_len = curr_mdl->ByteCount;
        }

        copy_len = packet_len - copied_len;

        if (copy_len > mdl_len) {
            copy_len = mdl_len;
        }

        NdisMoveMemory(curr_target, curr_src, copy_len);

        copied_len += copy_len;

        if (copied_len == packet_len) {
            ASSERT(curr_mdl->Next == NULL);
            break;
        }

        mdl_len -= copy_len;
        curr_target += copy_len;

        // Next mdl
        curr_mdl = curr_mdl->Next;
        source_va = NULL;
    }

    if (status != NDIS_STATUS_SUCCESS) {
        goto cleanup;
    }

    // Update the lengths of the sg elements we are returning
    if (packet_len <= PAGE_SIZE) {
        frag_elem->tx_frag_list->Elements[0].Length = packet_len;
        frag_elem->tx_frag_list->NumberOfElements = 1;
    } else if (packet_len <= (2 * PAGE_SIZE)) {
        ASSERT(frag_elem->tx_frag_list->Elements[0].Length == PAGE_SIZE);
        frag_elem->tx_frag_list->Elements[1].Length = (packet_len - PAGE_SIZE);
        frag_elem->tx_frag_list->NumberOfElements = 2;
    } else {
        ASSERT(frag_elem->tx_frag_list->Elements[0].Length == PAGE_SIZE);
        ASSERT(frag_elem->tx_frag_list->Elements[1].Length == PAGE_SIZE);
        frag_elem->tx_frag_list->Elements[2].Length =
            (packet_len - (2 * PAGE_SIZE));
        frag_elem->tx_frag_list->NumberOfElements = 3;
    }

    *frag = frag_elem;

cleanup:

    if (status != NDIS_STATUS_SUCCESS) {

        if (frag_elem != NULL) {
            return_tx_frag(ionic, frag_elem);
        }
    }

    return status;
}

void
comp_data(NET_BUFFER *packet, tx_frag_pool_elem *fragment)
{

    u32 packet_len = 0;
    u32 comp_len = 0;
    void *source_va = NULL;
    u8 *curr_src = NULL;
    u8 *curr_target = NULL;
    MDL *curr_mdl = NULL;
    u32 mdl_len = 0;
    u32 data_offset = 0;
    u32 compared_len = 0;

    packet_len = NET_BUFFER_DATA_LENGTH(packet);

    curr_mdl = packet->CurrentMdl;
    curr_target = (u8 *)fragment->buffer;

    source_va = MmGetSystemAddressForMdlSafe(curr_mdl, NormalPagePriority);

    if (source_va == NULL) {
        goto cleanup;
    }

    curr_src = (u8 *)source_va;
    mdl_len = curr_mdl->ByteCount;

    data_offset = (UINT)NET_BUFFER_CURRENT_MDL_OFFSET(packet);

    // Get to the start of the data
    while (data_offset != 0) {
        if (mdl_len >= data_offset) {
            curr_src += data_offset;
            mdl_len -= data_offset;

            if (mdl_len == 0) {
                curr_mdl = curr_mdl->Next;
                source_va = NULL;
            }

            break;
        }

        data_offset -= mdl_len;

        curr_mdl = curr_mdl->Next;

        source_va = MmGetSystemAddressForMdlSafe(curr_mdl, NormalPagePriority);

        if (source_va == NULL) {
            goto cleanup;
        }

        curr_src = (u8 *)source_va;
        mdl_len = curr_mdl->ByteCount;
    }

    while (compared_len < packet_len) {

        if (source_va == NULL) {

            source_va =
                MmGetSystemAddressForMdlSafe(curr_mdl, NormalPagePriority);

            if (source_va == NULL) {
                break;
            }

            curr_src = (u8 *)source_va;
            mdl_len = curr_mdl->ByteCount;
        }

        comp_len = packet_len - compared_len;

        if (comp_len > mdl_len) {
            comp_len = mdl_len;
        }

        if (RtlCompareMemory(curr_target, curr_src, comp_len) != comp_len) {
            IoPrint("%s Have mismatch in buffer for packet len %08lX\n",
                    __FUNCTION__, packet_len);
        }

        compared_len += comp_len;

        if (compared_len == packet_len) {
            ASSERT(curr_mdl->Next == NULL);
            break;
        }

        mdl_len -= comp_len;
        curr_target += comp_len;

        // Next mdl
        curr_mdl = curr_mdl->Next;
        source_va = NULL;
    }

cleanup:

    return;
}

void
ionic_tx_empty(struct queue *q)
{
    struct desc_info *desc_info;
    int done = 0;

    /* walk the not completed tx entries, if any */
    while (q->head != q->tail) {
        desc_info = (struct desc_info *)q->tail;
        q->tail = desc_info->next;
        // ionic_tx_clean(q, desc_info, NULL, desc_info->cb_arg);
        desc_info->cb = NULL;
        desc_info->cb_arg = NULL;
        done++;
    }
}


void 
tx_packet_dpc_callback( _KDPC *Dpc,
					    PVOID DeferredContext,
					    PVOID SystemArgument1,
					    PVOID SystemArgument2)
{

	struct qcq *qcq = (struct qcq *)DeferredContext;

	UNREFERENCED_PARAMETER(SystemArgument1);
	UNREFERENCED_PARAMETER(SystemArgument2);

	// In the event our targeting of the DPC failed, or we are changing the affinity for the DPC,
	// check there is only 1 instance running
	if (InterlockedIncrement(&qcq->dpc_exec_cnt) == 1) {

		ionic_service_nb_requests(qcq, false);

		if(ionic_tx_flush(qcq, IONIC_TX_BUDGET_DEFAULT,
				false, false) &&
		   RtlCheckBit(&qcq->q.lif->state, LIF_UP)) {
			KeInsertQueueDpc(Dpc,
				NULL,
				NULL);
		}
	}
	else {
		IoPrint("%s Running in parallel\n", __FUNCTION__);
	}

	InterlockedDecrement( &qcq->dpc_exec_cnt);

	return;
}
