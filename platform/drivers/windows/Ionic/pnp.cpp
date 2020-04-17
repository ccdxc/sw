
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

            if (adapter->sys_proc_info != NULL) {
                NdisFreeMemoryWithTagPriority_internal(miniport_adapter_handle,
                    adapter->sys_proc_info, IONIC_ADAPTER_TAG);
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

    if (ionic->sys_proc_info != NULL) {
        NdisFreeMemoryWithTagPriority_internal(ionic->adapterhandle, ionic->sys_proc_info, IONIC_ADAPTER_TAG);
    }

    NdisFreeMemoryWithTagPriority_internal(ionic->adapterhandle, ionic, IONIC_ADAPTER_TAG);

    DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_VERBOSE,
              "%s Complete for Adapter %p\n", __FUNCTION__, ionic));

    return;
}

static void
print_io_resource_requirements_list(PIO_RESOURCE_REQUIREMENTS_LIST prrl)
{
    PIO_RESOURCE_LIST prl;
    PIO_RESOURCE_DESCRIPTOR prd;
    ULONG offset;

    prl = prrl->List;
    offset = 0;

    for (ULONG prl_idx = 0; prl_idx < prrl->AlternativeLists && offset < prrl->ListSize; ++prl_idx) {

        DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_VERBOSE, "%s resource list %lu count %lu\n",
            __FUNCTION__, prl_idx, prl->Count));
        IoPrint("%s resource list %lu count %lu\n", __FUNCTION__, prl_idx, prl->Count);

        for (ULONG prd_idx = 0; prd_idx < prl->Count; ++prd_idx) {
            prd = &prl->Descriptors[prd_idx];

            switch (prd->Type)
            {
            case CmResourceTypePort:
                DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_VERBOSE,
                    "%s port align 0x%lx length %lu min 0x%llx max 0x%llx\n",
                    __FUNCTION__,
                    prd->u.Port.Alignment, prd->u.Port.Length,
                    prd->u.Port.MinimumAddress.QuadPart, prd->u.Port.MaximumAddress.QuadPart));
                IoPrint("%s port align 0x%lx length %lu min 0x%llx max 0x%llx\n",
                    __FUNCTION__,
                    prd->u.Port.Alignment, prd->u.Port.Length,
                    prd->u.Port.MinimumAddress.QuadPart, prd->u.Port.MaximumAddress.QuadPart);
                break;
            case CmResourceTypeInterrupt:
                DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_VERBOSE,
                    "%s interrupt min 0x%lx max 0x%lx group %d affinity 0x%llx\n",
                    __FUNCTION__,
                    prd->u.Interrupt.MinimumVector, prd->u.Interrupt.MaximumVector,
                    prd->u.Interrupt.Group, prd->u.Interrupt.TargetedProcessors));
                IoPrint("%s interrupt min 0x%lx max 0x%lx group %d affinity 0x%llx\n",
                    __FUNCTION__,
                    prd->u.Interrupt.MinimumVector, prd->u.Interrupt.MaximumVector,
                    prd->u.Interrupt.Group, prd->u.Interrupt.TargetedProcessors);
                break;
            case CmResourceTypeMemory:
                DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_VERBOSE,
                    "%s memory align 0x%lx length %lu min 0x%llx max 0x%llx\n",
                    __FUNCTION__,
                    prd->u.Memory.Alignment, prd->u.Memory.Length,
                    prd->u.Memory.MinimumAddress.QuadPart, prd->u.Memory.MaximumAddress.QuadPart));
                IoPrint("%s memory align 0x%lx length %lu min 0x%llx max 0x%llx\n",
                    __FUNCTION__,
                    prd->u.Memory.Alignment, prd->u.Memory.Length,
                    prd->u.Memory.MinimumAddress.QuadPart, prd->u.Memory.MaximumAddress.QuadPart);
                break;
            default:
                DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_VERBOSE,
                    "%s descriptor %lu type %u\n",
                    __FUNCTION__, prd_idx, prd->Type));
                IoPrint("%s descriptor %lu type %u\n",
                    __FUNCTION__, prd_idx, prd->Type);
                break;
            }

            offset += sizeof(*prd);
        }

        prl = (PIO_RESOURCE_LIST)(prl->Descriptors + prl->Count);
    }
}

NDIS_STATUS
ionic_set_proc_info(NDIS_HANDLE miniport_add_device_context)
{
    struct ionic *ionic = (struct ionic *)miniport_add_device_context;
    NDIS_STATUS status;
    SIZE_T sys_proc_info_sz = 0;

    status = NdisGetProcessorInformationEx(ionic->adapterhandle,
        NULL, &sys_proc_info_sz);
    if (status != NDIS_STATUS_BUFFER_TOO_SHORT) {
        DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_ERROR,
            "%s failed to get processor information size\n",
            __FUNCTION__));
        IoPrint("%s failed to get processor information size\n",
            __FUNCTION__);
        return status;
    }

    DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_VERBOSE,
        "%s sys_proc_info_sz %d\n", __FUNCTION__, sys_proc_info_sz));
    IoPrint("%s sys_proc_info_sz %d\n", __FUNCTION__, sys_proc_info_sz);

	if (ionic->sys_proc_info != NULL) {
		NdisFreeMemoryWithTagPriority_internal(ionic->adapterhandle, ionic->sys_proc_info, IONIC_ADAPTER_TAG);
	}

    ionic->sys_proc_info = (PNDIS_SYSTEM_PROCESSOR_INFO_EX)NdisAllocateMemoryWithTagPriority_internal(
        miniport_add_device_context, (UINT)sys_proc_info_sz, IONIC_ADAPTER_TAG, NormalPoolPriority);
    if (ionic->sys_proc_info == NULL) {
        DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_ERROR,
            "%s failed to allocate memory for processor information\n",
            __FUNCTION__));
        IoPrint("%s failed to allocate memory for processor information\n",
            __FUNCTION__);
        return status;
    }

    status = NdisGetProcessorInformationEx(ionic->adapterhandle,
        ionic->sys_proc_info, &sys_proc_info_sz);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_ERROR,
            "%s failed to get processor information\n",
            __FUNCTION__));
        IoPrint("%s failed to get processor information\n",
            __FUNCTION__);
        return status;

    }

    DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_VERBOSE,
        "%s sockets %d cores %d cores/socket %d num_procs %d\n",
        __FUNCTION__,
        ionic->sys_proc_info->NumSockets,
        ionic->sys_proc_info->NumCores,
        ionic->sys_proc_info->NumCoresPerSocket,
        ionic->sys_proc_info->NumberOfProcessors));
    IoPrint("%s sockets %d cores %d cores/socket %d num_procs %d\n",
        __FUNCTION__,
        ionic->sys_proc_info->NumSockets,
        ionic->sys_proc_info->NumCores,
        ionic->sys_proc_info->NumCoresPerSocket,
        ionic->sys_proc_info->NumberOfProcessors);
    
    ionic->sys_proc = (PNDIS_PROCESSOR_INFO_EX)((PUCHAR)ionic->sys_proc_info +
            ionic->sys_proc_info->ProcessorInfoOffset);

    for (ULONG i = 0; i < ionic->sys_proc_info->NumberOfProcessors; ++i) {
        PNDIS_PROCESSOR_INFO_EX proc = &ionic->sys_proc[i];
        DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_VERBOSE,
            "%s group %d proc %d sock %d core %d node %u node_dist %u\n",
            __FUNCTION__,
            proc->ProcNum.Group, proc->ProcNum.Number,
            proc->SocketId, proc->CoreId,
            proc->NodeId, proc->NodeDistance));
        IoPrint("%s group %d proc %d sock %d core %d node %u node_dist %u\n",
            __FUNCTION__,
            proc->ProcNum.Group, proc->ProcNum.Number,
            proc->SocketId, proc->CoreId,
            proc->NodeId, proc->NodeDistance);
    }

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
ionic_set_intr_requirements(NDIS_HANDLE miniport_add_device_context,
    PIO_RESOURCE_REQUIREMENTS_LIST prrl)
{
    struct ionic* ionic = (struct ionic*)miniport_add_device_context;
    PIO_RESOURCE_LIST prl;
    PIO_RESOURCE_DESCRIPTOR prd;
    ULONG offset;
    ULONG msg_id;
    ULONG proc_idx;
    PNDIS_PROCESSOR_INFO_EX proc;

    if (ionic->sys_proc_info == NULL || ionic->sys_proc == NULL) {
        DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_ERROR,
            "%s processor info is not set\n", __FUNCTION__));
        IoPrint("%s processor info is not set\n", __FUNCTION__);
        return NDIS_STATUS_FAILURE;
    }

    prl = prrl->List;
    offset = 0;
    
    proc_idx = 0;
    msg_id = 0;

    for (ULONG prl_idx = 0; prl_idx < prrl->AlternativeLists && offset < prrl->ListSize; ++prl_idx) {

        for (ULONG prd_idx = 0; prd_idx < prl->Count; ++prd_idx) {
            prd = &prl->Descriptors[prd_idx];

            if (prd->Type == CmResourceTypeInterrupt && (prd->Flags & CM_RESOURCE_INTERRUPT_MESSAGE)) {

                if (msg_id < IONIC_CTL_INTR_CNT) {
                    /* affinitize controlpath MSI-x messages to all local processors */ 
                    prd->Flags |= CM_RESOURCE_INTERRUPT_POLICY_INCLUDED;
                    prd->u.Interrupt.AffinityPolicy = IrqPolicyAllCloseProcessors;

                    DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_VERBOSE,
                        "%s msg_id %d affinity IrqPolicyAllCloseProcessors\n",
                        __FUNCTION__, msg_id));
                    IoPrint("%s msg_id %d affinity IrqPolicyAllCloseProcessors\n",
                        __FUNCTION__, msg_id);
                } else {
                    /* affinitize datapath MSI-x messages to processors in round-robin order */
                    proc = &ionic->sys_proc[proc_idx];

                    prd->Flags |= CM_RESOURCE_INTERRUPT_POLICY_INCLUDED;
                    prd->u.Interrupt.AffinityPolicy = IrqPolicySpecifiedProcessors;
                    prd->u.Interrupt.Group = proc->ProcNum.Group;
                    prd->u.Interrupt.TargetedProcessors = 1i64 << proc->ProcNum.Number;

    DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_VERBOSE,
                        "%s msg_id %d group %u proc %d affinity 0x%llx\n",
                        __FUNCTION__,
                        msg_id, prd->u.Interrupt.Group, proc->ProcNum.Number,
                        prd->u.Interrupt.TargetedProcessors));
                    IoPrint("%s msg_id %d group %u proc %d affinity 0x%llx\n",
                        __FUNCTION__,
                        msg_id, prd->u.Interrupt.Group, proc->ProcNum.Number,
                        prd->u.Interrupt.TargetedProcessors);

                    ++proc_idx %= ionic->sys_proc_info->NumberOfProcessors;
                }

                ++msg_id;
            }

            offset += sizeof(*prd);
        }

        prl = (PIO_RESOURCE_LIST)(prl->Descriptors + prl->Count);
    }

    return NDIS_STATUS_SUCCESS;
}

ULONG
ionic_intr_msgs_needed(NDIS_HANDLE miniport_add_device_context)
{
    struct ionic* ionic = (struct ionic*)miniport_add_device_context;

    return (IONIC_CTL_INTR_CNT + ionic->sys_proc_info->NumberOfProcessors);
}

ULONG
intr_msgs_count(PIO_RESOURCE_REQUIREMENTS_LIST prrl)
{
    PIO_RESOURCE_LIST prl;
    PIO_RESOURCE_DESCRIPTOR prd;
    ULONG offset;
    ULONG count;

    prl = prrl->List;
    offset = 0;

    count = 0;
	/* Only walk the first resource list since it is the one we will be adding to */
    for (ULONG prd_idx = 0; prd_idx < prl->Count; ++prd_idx) {
        prd = &prl->Descriptors[prd_idx];
        if (prd->Type == CmResourceTypeInterrupt && (prd->Flags & CM_RESOURCE_INTERRUPT_MESSAGE)) {
            ++count;
        }
        offset += sizeof(*prd);
    }

    return count;
}

PIO_RESOURCE_REQUIREMENTS_LIST
ionic_adjust_io_resource_requirements(NDIS_HANDLE miniport_add_device_context,
    PIO_RESOURCE_REQUIREMENTS_LIST prrl)
{
    PIO_RESOURCE_REQUIREMENTS_LIST new_prrl;
    PIO_RESOURCE_LIST prl, new_prl;
    PIO_RESOURCE_DESCRIPTOR prd;
    ULONG msgs_cnt = intr_msgs_count(prrl);
    ULONG msgs_needed = ionic_intr_msgs_needed(miniport_add_device_context);
    ULONG new_prrl_sz = 0;
    ULONG offset = 0;
	PIO_RESOURCE_DESCRIPTOR prd_ref;


    /* don't modify the list if we have enough interrupt messages */
    if (msgs_cnt >= msgs_needed) {
        return NULL;
    }

    /* allocate a new list and copy over the old list */
    new_prrl_sz = prrl->ListSize + ((msgs_needed - msgs_cnt) * sizeof(IO_RESOURCE_DESCRIPTOR));
    new_prrl = (PIO_RESOURCE_REQUIREMENTS_LIST)NdisAllocateMemoryWithTagPriority_internal(
        miniport_add_device_context,
        new_prrl_sz,
        IONIC_ADAPTER_TAG,
        NormalPoolPriority);

    /* copy the original resource requirements list into the new list */
    NdisZeroMemory(new_prrl, new_prrl_sz);
    NdisMoveMemory(new_prrl, prrl, sizeof(IO_RESOURCE_REQUIREMENTS_LIST));
    new_prrl->ListSize = new_prrl_sz;

    /* copy the primary list */
    prl = prrl->List;
    new_prl = new_prrl->List;

	NdisMoveMemory(new_prl, prl, sizeof(IO_RESOURCE_LIST));
	NdisMoveMemory(new_prl->Descriptors, prl->Descriptors,
		prl->Count * sizeof(IO_RESOURCE_DESCRIPTOR));

	prd_ref = prl->Descriptors;
	for (ULONG prd_idx = 0; prd_idx < prl->Count; prd_idx++, prd_ref++) {
		if (prd_ref->Type == CmResourceTypeInterrupt &&
			(prd_ref->Flags & CM_RESOURCE_INTERRUPT_MESSAGE)) {
			break;
		}
	}

    /* populate the additional interrupt message resource descriptors */
    new_prl->Count = prl->Count + (msgs_needed - msgs_cnt);
    for (ULONG prd_idx = prl->Count; prd_idx < new_prl->Count; ++prd_idx) {
		prd = &new_prl->Descriptors[prd_idx];
		prd->ShareDisposition = CmResourceShareDeviceExclusive;
		prd->Option = 0;
		prd->Type = CmResourceTypeInterrupt;
		prd->Flags = prd_ref->Flags; //CM_RESOURCE_INTERRUPT_MESSAGE | CM_RESOURCE_INTERRUPT_LEVEL_LATCHED_BITS;
		prd->u.Interrupt.MinimumVector = prd_ref->u.Interrupt.MinimumVector; // 0;
		prd->u.Interrupt.MaximumVector = prd_ref->u.Interrupt.MaximumVector; // (ULONG)(-1);
    }
    prl = (PIO_RESOURCE_LIST)(prl->Descriptors + prl->Count);
    new_prl = (PIO_RESOURCE_LIST)(new_prl->Descriptors + new_prl->Count);
    offset += (prl->Count * sizeof(IO_RESOURCE_DESCRIPTOR));

    /* copy the alternative lists */
	for (ULONG prl_idx = 1; prl_idx < prrl->AlternativeLists && offset < prrl->ListSize; ++prl_idx) {
        NdisMoveMemory(new_prl, prl, sizeof(IO_RESOURCE_LIST));
        NdisMoveMemory(new_prl->Descriptors, prl->Descriptors,
            prl->Count * sizeof(IO_RESOURCE_DESCRIPTOR));
		prl = (PIO_RESOURCE_LIST)(prl->Descriptors + prl->Count);
        new_prl = (PIO_RESOURCE_LIST)(new_prl->Descriptors + new_prl->Count);
	}

    return new_prrl;
}

NDIS_STATUS
ionic_filter_resource_requirements(NDIS_HANDLE miniport_add_device_context,
                                   PIRP irp)
{
    struct ionic *ionic = (struct ionic *)miniport_add_device_context;
    PIO_RESOURCE_REQUIREMENTS_LIST prrl;
    PIO_RESOURCE_REQUIREMENTS_LIST new_prrl;
    NDIS_STATUS status;

    DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_VERBOSE,
              "%s Entered for adapter %p\n", __FUNCTION__, ionic));

    status = ionic_set_proc_info(miniport_add_device_context);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_ERROR,
            "%s failed to set processor info\n", __FUNCTION__));
        IoPrint("%s failed to set processor info\n", __FUNCTION__);
        return status;
    }

    prrl = (PIO_RESOURCE_REQUIREMENTS_LIST)irp->IoStatus.Information;
    print_io_resource_requirements_list(prrl);

    new_prrl = ionic_adjust_io_resource_requirements(miniport_add_device_context, prrl);
    if (new_prrl) {
        irp->IoStatus.Information = (ULONG_PTR)new_prrl;
        NdisFreeMemory(prrl, 0, 0);
        prrl = new_prrl;
        print_io_resource_requirements_list(new_prrl);
    } else {
        IoPrint("%s resource requirement list unchanged\n", __FUNCTION__);
    }

    status = ionic_set_intr_requirements(miniport_add_device_context, prrl);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_PNP, TRACE_LEVEL_ERROR,
            "%s failed to setup interrupt resources\n", __FUNCTION__));
        IoPrint("%s failed to setup interrupt resources\n", __FUNCTION__);
        return status;
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