#include "common.h"

NTSTATUS NTAPI
IonicPerfCounterCallback(
    _In_ PCW_CALLBACK_TYPE Type,
    _In_ PPCW_CALLBACK_INFORMATION Info,
    _In_opt_ PVOID Context)
{
    NTSTATUS status = STATUS_SUCCESS;
    NDIS_STATUS ndis_status;
    UNICODE_STRING uni_adapter_name;
    UNICODE_STRING uni_lif_name;
    UNICODE_STRING uni_q_name;
    ANSI_STRING ansi_name;
    UNICODE_STRING uni_allocated_adapter_name = { 0 };
    UNICODE_STRING uni_allocated_lif_name = { 0 };
    struct _PERF_MON_CB *perfmon_stats = NULL;
    struct _PERF_MON_ADAPTER_STATS *adapter_stats = NULL;
    struct _PERF_MON_LIF_STATS *lif_stats = NULL;
    struct _PERF_MON_RX_QUEUE_STATS *rx_stats = NULL;
    struct _PERF_MON_TX_QUEUE_STATS *tx_stats = NULL;
    ULONG stats_len = 0;
    ULONG lut_idx;
    ULONG instance_idx;
    ULONG adapter_idx;
    ULONG lif_idx;
    ULONG q_idx;
    WCHAR *name_buffer = NULL;
    ULONG name_buffer_len = 512;

    UNREFERENCED_PARAMETER(Context);

    switch (Type) {
        case PcwCallbackEnumerateInstances:
        case PcwCallbackCollectData:
            break;


        case PcwCallbackAddCounter:
            IoPrint("%s: PcwCallbackAddCounter received\n", __FUNCTION__);
            break;

        case PcwCallbackRemoveCounter:
            IoPrint("%s: PcwCallbackRemoveCounter received\n", __FUNCTION__);
            break;

        default:
            goto exit;
    }

    // allocate one buffer, split it for two uses
    name_buffer = (WCHAR *)NdisAllocateMemoryWithTagPriority_internal(
        IonicDriver,
        name_buffer_len,
        IONIC_STATS_TAG,
        NormalPoolPriority);

    if (name_buffer == NULL) {
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

    DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                  "%s Allocated 0x%p len %08lX\n",
                  __FUNCTION__,
                  name_buffer,
                  name_buffer_len));
    
    uni_allocated_adapter_name.Buffer = (WCHAR *)((ULONG_PTR)name_buffer + name_buffer_len / 2);
    uni_allocated_adapter_name.MaximumLength = (USHORT)(name_buffer_len / 2);
    uni_allocated_adapter_name.Length = 0;

    ndis_status = get_perfmon_stats(&perfmon_stats, &stats_len);

    if (ndis_status != NDIS_STATUS_SUCCESS) {
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

    lut_idx = 0;
    instance_idx = 1;
    for (adapter_idx = 0; adapter_idx < perfmon_stats->adapter_count; adapter_idx++) {
        adapter_stats = (struct _PERF_MON_ADAPTER_STATS *)perfmon_stats->lookup_table[lut_idx++];
        RtlInitUnicodeString(&uni_adapter_name, adapter_stats->name);

        RtlUnicodeStringPrintf(&uni_allocated_adapter_name, L"%wZ%s", &uni_adapter_name, adapter_stats->mgmt_device ? L" (Mgmt)" : L"");

        // add an adapter
        if (Type == PcwCallbackEnumerateInstances) {
            status = IonicAddPensando_Systems(
                Info->EnumerateInstances.Buffer,
                &uni_allocated_adapter_name,
                instance_idx,
                NULL);
        }
        else {
            if (Info->CollectData.InstanceId == instance_idx) {
                status = IonicAddAdapterInstance(
                    Info->CollectData.Buffer,
                    &uni_allocated_adapter_name,
                    instance_idx,
                    adapter_stats);
            }
        }
        instance_idx++;

        if (!NT_SUCCESS(status)) {
            status = STATUS_UNSUCCESSFUL;
            goto exit;
        }

        for (lif_idx = 0; lif_idx < adapter_stats->lif_count; lif_idx++) {
            lif_stats = (struct _PERF_MON_LIF_STATS *)perfmon_stats->lookup_table[lut_idx++];
            RtlInitAnsiString(&ansi_name, lif_stats->name);

            status = RtlAnsiStringToUnicodeString(&uni_allocated_lif_name, &ansi_name, TRUE);

            if (!NT_SUCCESS(status)) {
                status = STATUS_UNSUCCESSFUL;
                goto exit;
            }

            status = RtlStringCbPrintfW(
                name_buffer, name_buffer_len / 2, L"%wZ:%wZ",
                &uni_allocated_adapter_name,
                &uni_allocated_lif_name);

            if (!NT_SUCCESS(status)) {
                status = STATUS_UNSUCCESSFUL;
                goto exit;
            }

            RtlInitUnicodeString(&uni_lif_name, name_buffer);                        

            // add a lif
            if (Type == PcwCallbackEnumerateInstances) {
                status = IonicAddPensando_Systems(
                    Info->EnumerateInstances.Buffer,
                    &uni_lif_name,
                    instance_idx,
                    NULL);
            }
            else {
                if (Info->CollectData.InstanceId == instance_idx) {
                    status = IonicAddLifInstance(
                        Info->CollectData.Buffer,
                        &uni_lif_name,
                        instance_idx,
                        lif_stats);
                }
            }
            instance_idx++;

            if (!NT_SUCCESS(status)) {
                status = STATUS_UNSUCCESSFUL;
                goto exit;
            }

            if (lif_stats->rx_queue_count != 0) {
                rx_stats = (struct _PERF_MON_RX_QUEUE_STATS *)perfmon_stats->lookup_table[lut_idx++];

                for (q_idx = 0; q_idx < lif_stats->rx_queue_count; q_idx++, rx_stats++) {
                    status = RtlStringCbPrintfW(
                        name_buffer, name_buffer_len / 2, L"%wZ:%wZ:rxq%03u",
                        &uni_allocated_adapter_name,
                        &uni_allocated_lif_name,
                        q_idx);

                    if (!NT_SUCCESS(status)) {
                        status = STATUS_UNSUCCESSFUL;
                        goto exit;
                    }

                    RtlInitUnicodeString(&uni_q_name, name_buffer);

                    // add an rx queue
                    if (Type == PcwCallbackEnumerateInstances) {
                        status = IonicAddPensando_Systems(
                            Info->EnumerateInstances.Buffer,
                            &uni_q_name,
                            instance_idx,
                            NULL);
                    }
                    else {
                        if (Info->CollectData.InstanceId == instance_idx) {
                            status = IonicAddRxQueueInstance(
                                Info->CollectData.Buffer,
                                &uni_q_name,
                                instance_idx,
                                rx_stats);
                        }
                    }
                    instance_idx++;

                    if (!NT_SUCCESS(status)) {
                        status = STATUS_UNSUCCESSFUL;
                        goto exit;
                    }
                }
            }

            if (lif_stats->tx_queue_count != 0) {
                tx_stats = (struct _PERF_MON_TX_QUEUE_STATS *)perfmon_stats->lookup_table[lut_idx++];

                for (q_idx = 0; q_idx < lif_stats->tx_queue_count; q_idx++, tx_stats++) {
                    status = RtlStringCbPrintfW(
                        name_buffer, name_buffer_len / 2, L"%wZ:%wZ:txq%03u",
                        &uni_allocated_adapter_name,
                        &uni_allocated_lif_name,
                        q_idx);

                    if (!NT_SUCCESS(status)) {
                        status = STATUS_UNSUCCESSFUL;
                        goto exit;
                    }

                    RtlInitUnicodeString(&uni_q_name, name_buffer);

                    // add a tx queue
                    if (Type == PcwCallbackEnumerateInstances) {
                        status = IonicAddPensando_Systems(
                            Info->EnumerateInstances.Buffer,
                            &uni_q_name,
                            instance_idx,
                            NULL);
                    }
                    else {
                        if (Info->CollectData.InstanceId == instance_idx) {
                            status = IonicAddTxQueueInstance(
                                Info->CollectData.Buffer,
                                &uni_q_name,
                                instance_idx,
                                tx_stats);
                        }
                    }
                    instance_idx++;

                    if (!NT_SUCCESS(status)) {
                        status = STATUS_UNSUCCESSFUL;
                        goto exit;
                    }
                }
            }

            RtlFreeUnicodeString(&uni_allocated_lif_name);
            uni_allocated_lif_name.Buffer = NULL;
        }
    }

exit:

    if (uni_allocated_lif_name.Buffer != NULL) {
        RtlFreeUnicodeString(&uni_allocated_lif_name);
    }

    if (name_buffer != NULL) {
        NdisFreeMemoryWithTagPriority_internal(IonicDriver, name_buffer, IONIC_STATS_TAG);
    }

    if (perfmon_stats != NULL) {
        NdisFreeMemoryWithTagPriority_internal(IonicDriver, perfmon_stats, IONIC_STATS_TAG);
    }

    return status;
}

NTSTATUS
IonicAddAdapterInstance(
    _In_ PPCW_BUFFER Buffer,
    _In_ PUNICODE_STRING Name,
    _In_ ULONG InstanceId,
    _In_ struct _PERF_MON_ADAPTER_STATS *AdapterStats)
{
    PERF_MON_COLLECTED_STATS values = { 0 };

    values.core_redirection_count = AdapterStats->core_redirection_count;

    return IonicAddPensando_Systems(Buffer, Name, InstanceId, &values);
}

NTSTATUS
IonicAddLifInstance(
    _In_ PPCW_BUFFER Buffer,
    _In_ PUNICODE_STRING Name,
    _In_ ULONG InstanceId,
    _In_ struct _PERF_MON_LIF_STATS *LifStats)
{
    PERF_MON_COLLECTED_STATS values = { 0 };

    UNREFERENCED_PARAMETER(LifStats);

    // TODO:  add any per-lif stats?

    return IonicAddPensando_Systems(Buffer, Name, InstanceId, &values);
}

NTSTATUS
IonicAddRxQueueInstance(
    _In_ PPCW_BUFFER Buffer,
    _In_ PUNICODE_STRING Name,
    _In_ ULONG InstanceId,
    _In_ struct _PERF_MON_RX_QUEUE_STATS *RxQueueStats)
{
    PERF_MON_COLLECTED_STATS values = { 0 };

    values.rx_pool_count = RxQueueStats->rx_pool_count;

    return IonicAddPensando_Systems(Buffer, Name, InstanceId, &values);
}

NTSTATUS
IonicAddTxQueueInstance(
    _In_ PPCW_BUFFER Buffer,
    _In_ PUNICODE_STRING Name,
    _In_ ULONG InstanceId,
    _In_ struct _PERF_MON_TX_QUEUE_STATS *TxQueueStats)
{
    PERF_MON_COLLECTED_STATS values = { 0 };

    values.byte_per_sec = TxQueueStats->byte_per_sec;
    values.max_queue_len = TxQueueStats->max_queue_len;
    values.nbl_per_sec = TxQueueStats->nbl_per_sec;
    values.nbl_time_to_complete = TxQueueStats->nbl_time_to_complete;
    values.nb_per_sec = TxQueueStats->nb_per_sec;
    values.nb_time_queue_to_comp = TxQueueStats->nb_time_queue_to_comp;
    values.nb_time_to_complete = TxQueueStats->nb_time_to_complete;
    values.nb_time_to_queue = TxQueueStats->nb_time_to_queue;
    values.pending_nbl_count = TxQueueStats->pending_nbl_count;
    values.pending_nb_count = TxQueueStats->pending_nb_count;
    values.queue_len = TxQueueStats->queue_len;

    return IonicAddPensando_Systems(Buffer, Name, InstanceId, &values);
}
