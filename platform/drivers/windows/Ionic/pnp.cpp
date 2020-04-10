
#include "common.h"

NDIS_STATUS
ionic_add_device(NDIS_HANDLE miniport_adapter_handle,
                 NDIS_HANDLE miniport_driver_context)
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    NDIS_MINIPORT_ADD_DEVICE_REGISTRATION_ATTRIBUTES stAttribs;
    struct ionic *adapter = NULL;
    LONG port_cnt = 0;

    UNREFERENCED_PARAMETER(miniport_driver_context);

    DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_VERBOSE, "%s Adapter %p\n",
              __FUNCTION__, miniport_adapter_handle));

    NdisZeroMemory(&stAttribs,
                   sizeof(NDIS_MINIPORT_ADD_DEVICE_REGISTRATION_ATTRIBUTES));
    stAttribs.Header.Type =
        NDIS_OBJECT_TYPE_MINIPORT_ADD_DEVICE_REGISTRATION_ATTRIBUTES;
    stAttribs.Header.Revision =
        NDIS_MINIPORT_ADD_DEVICE_REGISTRATION_ATTRIBUTES_REVISION_1;
    stAttribs.Header.Size =
        sizeof(NDIS_MINIPORT_ADD_DEVICE_REGISTRATION_ATTRIBUTES);
    stAttribs.MiniportAddDeviceContext = NdisAllocateMemoryWithTagPriority_internal(
        miniport_adapter_handle, sizeof(struct ionic), IONIC_ADAPTER_TAG,
        NormalPoolPriority);

    if (stAttribs.MiniportAddDeviceContext == NULL) {
        status = NDIS_STATUS_RESOURCES;
        goto cleanup;
    }

    adapter = (struct ionic *)stAttribs.MiniportAddDeviceContext;

    NdisZeroMemory(adapter, sizeof(struct ionic));

    DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                  "%s Allocated 0x%p len %08lX\n",
                  __FUNCTION__,
                  adapter, sizeof(struct ionic)));

    adapter->adapterhandle = miniport_adapter_handle;
	
	status = init_registry_config( adapter);

	if( status != NDIS_STATUS_SUCCESS) {
        goto cleanup;
	}

    NdisAllocateSpinLock(&adapter->dev_cmd_lock);

    NdisZeroMemory(adapter->q_filter,
                   sizeof(struct filter_info) * IONIC_MAX_FILTER_COUNT);

    InitializeListHead(&adapter->lifs);

    KeInitializeEvent(&adapter->outstanding_complete_event, NotificationEvent, TRUE);

    RtlInitializeBitMap(&adapter->intrs, (PULONG)adapter->intrs_buffer,
                        INTR_CTRL_REGS_MAX);

    RtlInitializeBitMap(&adapter->lifbits, (PULONG)adapter->lifbits_buffer,
                        IONIC_LIFS_MAX);

    status = init_rx_queue_info(adapter);

    if (status == NDIS_STATUS_SUCCESS) {

        status = NdisMSetMiniportAttributes(
            miniport_adapter_handle,
            (PNDIS_MINIPORT_ADAPTER_ATTRIBUTES)&stAttribs);
    }

    if (status == NDIS_STATUS_SUCCESS) {
        // Insert this new IONIC Device into the Global List
        PAGED_CODE();
        NDIS_WAIT_FOR_MUTEX(&AdapterListLock);
        InsertTailList(&AdapterList, &adapter->list_entry);
        port_cnt = InterlockedIncrement((long *)&port_count);
        NDIS_RELEASE_MUTEX(&AdapterListLock);

        if (port_cnt == 1) {
            RegisterDevice(IonicDriver);
        }
    } else {
        DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_ERROR,
                  "%s Failed NdisMSetMiniportAttributes Status %08lX\n",
                  __FUNCTION__, status));
    }

cleanup:

    if (status != NDIS_STATUS_SUCCESS) {

        if (adapter != NULL) {
			if (adapter->registry_config != NULL) {
				NdisFreeMemoryWithTagPriority_internal(miniport_adapter_handle,
                                          adapter->registry_config,
                                          IONIC_ADAPTER_TAG);
			}
            NdisFreeMemoryWithTagPriority_internal(miniport_adapter_handle,
                                          adapter,
                                          IONIC_ADAPTER_TAG);
        }
    }

    return status;
}

VOID
ionic_remove_device(NDIS_HANDLE miniport_add_device_context)
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    struct ionic *ionic = (struct ionic *)miniport_add_device_context;
    LONG lPortCnt = 0;
    BOOLEAN set_event = FALSE;

    DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_VERBOSE,
              "%s Entered for Adapter %p\n", __FUNCTION__, ionic));

    delete_rx_queue_info(ionic);

    // Sync behind any perfmon requests coming in
    status = KeWaitForSingleObject(&perfmon_event,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   NULL);

    if (status == STATUS_SUCCESS) {
        set_event = TRUE;
    }

    PAGED_CODE();
    NDIS_WAIT_FOR_MUTEX(&AdapterListLock);
    RemoveEntryList(&ionic->list_entry);
    lPortCnt = InterlockedDecrement((long *)&port_count);
    NDIS_RELEASE_MUTEX(&AdapterListLock);

    if (set_event) {
        KeSetEvent( &perfmon_event, 0, FALSE);
    }

    if (lPortCnt == 0) {
        DeregisterDevice();
    }

    NdisFreeSpinLock(&ionic->dev_cmd_lock);

	if (ionic->registry_config != NULL) {
		NdisFreeMemoryWithTagPriority_internal(ionic->adapterhandle,
                                    ionic->registry_config,
                                    IONIC_ADAPTER_TAG);
	}

    NdisFreeMemoryWithTagPriority_internal(ionic->adapterhandle, ionic, IONIC_ADAPTER_TAG);

    DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_VERBOSE,
              "%s Complete for Adapter %p\n", __FUNCTION__, ionic));

    return;
}

NDIS_STATUS
ionic_filter_resource_requirements(NDIS_HANDLE miniport_add_device_context,
                                   PIRP irp)
{
    struct ionic *ionic = (struct ionic *)miniport_add_device_context;
    PIO_RESOURCE_REQUIREMENTS_LIST ioreq_list;
    PIO_RESOURCE_DESCRIPTOR iores_desc;
    ULONG i, msi_id;
    KAFFINITY ka;
    USHORT group_cnt = 0;
    ULONG proc_cnt = 0;
    ULONG proc_per_group = 0;

    msi_id = 0;
	ionic->proc_count = 0;
	ionic->interrupt_count = 0;

    DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_VERBOSE,
              "%s Entered for adapter %p\n", __FUNCTION__, ionic));

    while (TRUE) {
        // proc_cnt = NdisGroupMaxProcessorCount(group_cnt);
        proc_cnt = NdisGroupActiveProcessorCount(group_cnt);

        if (proc_cnt == 0) {
            break;
        }

        DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_VERBOSE,
                  "%s Group %d Count %d\n", __FUNCTION__, group_cnt, proc_cnt));

        if (proc_per_group == 0 || proc_cnt < proc_per_group) {
            proc_per_group = proc_cnt;
        }

        ionic->proc_count += proc_cnt;
        group_cnt++;
    }

    ionic->group_cnt = group_cnt;
    ionic->proc_per_group = proc_per_group;

    DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_VERBOSE,
              "%s Proc Count %d Group Count %d Proc per group %d\n",
              __FUNCTION__, ionic->proc_count, ionic->group_cnt,
              ionic->proc_per_group));

    ioreq_list = (PIO_RESOURCE_REQUIREMENTS_LIST)irp->IoStatus.Information;
    group_cnt = 0;

    for (i = 0; i < ioreq_list->List[0].Count; ++i) {
        iores_desc = &ioreq_list->List[0].Descriptors[i];
        if (iores_desc->Type == CmResourceTypeInterrupt &&
            iores_desc->Option == 0 &&
            (iores_desc->Flags & CM_RESOURCE_INTERRUPT_MESSAGE)) {
            ka = 1;
            ka <<= (msi_id % proc_per_group);

            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                      "\tMsi %d Group %d affinity 0x%I64X\n", msi_id, group_cnt,
                      ka));

            msi_id++;

            iores_desc->u.Interrupt.Group = group_cnt;

            if (msi_id == proc_per_group) {
                group_cnt++;
                if (group_cnt == ionic->group_cnt) {
                    group_cnt = 0;
                }
            }

            iores_desc->u.Interrupt.TargetedProcessors = ka;
            iores_desc->u.Interrupt.AffinityPolicy =
                IrqPolicySpecifiedProcessors;

            ionic->interrupt_count++;
        }
    }

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
ionic_start_device(NDIS_HANDLE miniport_add_device_context, PIRP irp)
{

    UNREFERENCED_PARAMETER(irp);
    UNREFERENCED_PARAMETER(miniport_add_device_context);

    DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_VERBOSE,
              "%s Enter for adapter %p\n", __FUNCTION__,
              miniport_add_device_context));

    return NDIS_STATUS_SUCCESS;
}

void
PnpEventNotify(NDIS_HANDLE MiniportAdapterContext,
               PNET_DEVICE_PNP_EVENT NetDevicePnPEvent)
{
    struct ionic *ionic = (struct ionic *)MiniportAdapterContext;

    DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_VERBOSE,
              "%s Enter Adapter %p Event %d len %d\n", __FUNCTION__, ionic,
              NetDevicePnPEvent->DevicePnPEvent,
              NetDevicePnPEvent->InformationBufferLength));

    switch (NetDevicePnPEvent->DevicePnPEvent) {

    case NdisDevicePnPEventSurpriseRemoved: {

		ionic->hardware_status = NdisHardwareStatusNotReady;
		DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_VERBOSE,
					"%s NdisDevicePnPEventSurpriseRemoved Adapter %p\n", 
							__FUNCTION__,
							ionic));
		SetFlag( ionic->Flags, IONIC_FLAG_SR_RECEIVED);
		ionic_link_down(ionic);

		/* Reset the lifs on this adapter */
		ionic_lifs_reset( ionic);

		mask_all_interrupts( ionic);

		deinit_dma( ionic);

		if( ionic->intr_obj != NULL) {
			NdisMDeregisterInterruptEx(ionic->intr_obj);
			ionic->intr_obj = NULL;
		}

        break;
    }
    }

    return;
}