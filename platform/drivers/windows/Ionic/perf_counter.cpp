#include "common.h"

NTSTATUS NTAPI
IonicPerfCounterCallback(
    _In_ PCW_CALLBACK_TYPE Type,
    _In_ PPCW_CALLBACK_INFORMATION Info,
    _In_opt_ PVOID Context)
{
    NTSTATUS status = STATUS_SUCCESS;
    NDIS_STATUS ndis_status = NDIS_STATUS_SUCCESS;
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
	ULONGLONG counter_mask = 0;
	WCHAR *slash = NULL;
	BOOLEAN set_event = FALSE;
	LARGE_INTEGER liDelay = {0};

    UNREFERENCED_PARAMETER(Context);

    switch (Type) {
    case PcwCallbackEnumerateInstances:
        counter_mask = IONIC_PERF_MON_NO_STATS;
		if (perfmon_timer == NULL) {
			status = STATUS_UNSUCCESSFUL;
			goto exit;
		}
        break;
    case PcwCallbackCollectData:
        counter_mask = Info->CollectData.CounterMask;
        break;
    case PcwCallbackAddCounter:
		if (InterlockedIncrement(&perfmon_counter_cnt) == 1) {
			// Launch our perfmon timer if present
			if( perfmon_timer != NULL) {
				liDelay.QuadPart = -(100);
				NdisSetTimerObject( perfmon_timer,
									liDelay,
									1000,
									NULL);
			}
		}
        goto exit;

    case PcwCallbackRemoveCounter:
		if (InterlockedDecrement(&perfmon_counter_cnt) == 0) {
			if (perfmon_timer != NULL) {
				NdisCancelTimerObject(perfmon_timer);
			}
		}
        goto exit;

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

	if( Type == PcwCallbackEnumerateInstances) {
	    ndis_status = get_perfmon_stats(NULL, 0, &perfmon_stats, &stats_len, counter_mask, 0);
	}
	else {

		status = KeWaitForSingleObject(&perfmon_event,
									  Executive,
									  KernelMode,
									  FALSE,
									  NULL);
		if (status != STATUS_SUCCESS) {
			goto exit;
		}
		set_event = TRUE;

		perfmon_stats = ionic_perfmon_stats;
	}

    if (ndis_status != NDIS_STATUS_SUCCESS ||
		perfmon_stats == NULL) {
        goto exit;
    }

    lut_idx = 0;
    instance_idx = 1;
    for (adapter_idx = 0; adapter_idx < perfmon_stats->adapter_count; adapter_idx++) {
        adapter_stats = (struct _PERF_MON_ADAPTER_STATS *)perfmon_stats->lookup_table[lut_idx++];
		// Perfmon does NOT like / characters in the name so remove them
		slash = wcschr( adapter_stats->name, L'/');
		if (slash != NULL) {
			*slash = L'-';
		}

        RtlInitUnicodeString(&uni_adapter_name, adapter_stats->name);

        RtlUnicodeStringPrintf(&uni_allocated_adapter_name, L"%wZ%s", &uni_adapter_name, adapter_stats->mgmt_device ? L" (Mgmt)" : L"");

        if (Context == IONIC_STATS_ADAPTER) {

			// add an adapter
			if (Type == PcwCallbackEnumerateInstances) {
				status = IonicAddpensando_adapter(
					Info->EnumerateInstances.Buffer,
					&uni_allocated_adapter_name,
					instance_idx,
					NULL);
			}
			else {
				if (Info->CollectData.InstanceId == instance_idx || Info->CollectData.InstanceId == 0xffffffff) {
					status = IonicAddAdapterInstance(
						Info->CollectData.Buffer,
						&uni_allocated_adapter_name,
						instance_idx,
						adapter_stats);
				}
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

            if (Context == IONIC_STATS_LIF) {

				// add a lif
				if (Type == PcwCallbackEnumerateInstances) {
					status = IonicAddpensando_adapter_lif(
						Info->EnumerateInstances.Buffer,
						&uni_lif_name,
						instance_idx,
						NULL);
				}
				else {
					if (Info->CollectData.InstanceId == instance_idx || Info->CollectData.InstanceId == 0xffffffff) {
						status = IonicAddLifInstance(
							Info->CollectData.Buffer,
							&uni_lif_name,
							instance_idx,
							lif_stats);
					}
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
                    if (Context == IONIC_STATS_RXQ) {

						if (Type == PcwCallbackEnumerateInstances) {
							status = IonicAddpensando_adapter_lif_rxq(
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
                    if (Context == IONIC_STATS_TXQ) {

						if (Type == PcwCallbackEnumerateInstances) {
							status = IonicAddpensando_adapter_lif_txq(
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

    if (perfmon_stats != NULL &&
		perfmon_stats != ionic_perfmon_stats) {
        NdisFreeMemoryWithTagPriority_internal(IonicDriver, perfmon_stats, IONIC_STATS_TAG);
    }

	if( set_event) {
		KeSetEvent( &perfmon_event, 0, FALSE);
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
    pensando_adapter_stats values = { 0 };

    // migrate data from per-adapter struct to per-adapter counter set
    values.lif_count = AdapterStats->lif_count;

    return IonicAddpensando_adapter(Buffer, Name, InstanceId, &values);
}

NTSTATUS
IonicAddLifInstance(
    _In_ PPCW_BUFFER Buffer,
    _In_ PUNICODE_STRING Name,
    _In_ ULONG InstanceId,
    _In_ struct _PERF_MON_LIF_STATS *LifStats)
{
    pensando_adapter_lif_stats values = { 0 };

	values.rx_pool_alloc_cnt = LifStats->rx_pool_alloc_cnt;
	values.rx_pool_alloc_time = LifStats->rx_pool_alloc_time;
	values.rx_pool_size = LifStats->rx_pool_size;
	values.rx_pool_free_cnt = LifStats->rx_pool_free_cnt;
	values.rx_pool_free_time = LifStats->rx_pool_free_time;

    return IonicAddpensando_adapter_lif(Buffer, Name, InstanceId, &values);
}

NTSTATUS
IonicAddRxQueueInstance(
    _In_ PPCW_BUFFER Buffer,
    _In_ PUNICODE_STRING Name,
    _In_ ULONG InstanceId,
    _In_ struct _PERF_MON_RX_QUEUE_STATS *RxQueueStats)
{
    pensando_adapter_lif_rxq_stats values = { 0 };

	values.queue_len = RxQueueStats->queue_len;
	values.max_queue_len = RxQueueStats->max_queue_len;

	values.dpc_total_time = RxQueueStats->dpc_total_time;
	values.dpc_latency = RxQueueStats->dpc_latency;
	values.dpc_to_dpc = RxQueueStats->dpc_to_dpc_time;
	values.dpc_indicate_time = RxQueueStats->dpc_indicate_time;
	values.dpc_walk_time = RxQueueStats->dpc_walk_time;
	values.dpc_fill_time = RxQueueStats->dpc_fill_time;

	values.dpc_rate = RxQueueStats->dpc_rate;

    return IonicAddpensando_adapter_lif_rxq(Buffer, Name, InstanceId, &values);
}

NTSTATUS
IonicAddTxQueueInstance(
    _In_ PPCW_BUFFER Buffer,
    _In_ PUNICODE_STRING Name,
    _In_ ULONG InstanceId,
    _In_ struct _PERF_MON_TX_QUEUE_STATS *TxQueueStats)
{
    pensando_adapter_lif_txq_stats values = { 0 };

	values.nbl_count = TxQueueStats->nbl_count;
	values.nb_count = TxQueueStats->nb_count;
	values.outstanding_nb_count = TxQueueStats->outstanding_nb_count;

	values.dpc_total_time = TxQueueStats->dpc_total_time;
	values.dpc_to_dpc = TxQueueStats->dpc_to_dpc;

	values.queue_len = TxQueueStats->queue_len;
	values.max_queue_len = TxQueueStats->max_queue_len;

	values.dpc_rate = TxQueueStats->dpc_rate;

    return IonicAddpensando_adapter_lif_txq(Buffer, Name, InstanceId, &values);
}

void
ionic_perfmon_cb(void *SystemContext,
                  void *FunctionContext,
                  void *Context1,
                  void *Context2)
{

    UNREFERENCED_PARAMETER(SystemContext);
    UNREFERENCED_PARAMETER(Context1);
    UNREFERENCED_PARAMETER(Context2);
	UNREFERENCED_PARAMETER(FunctionContext);

	queue_workitem( NULL, IONIC_WORKITEM_PERFMON, NULL);

	return;
}