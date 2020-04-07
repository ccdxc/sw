
#include "common.h"

#pragma NDIS_INIT_FUNCTION(DriverEntry)
#pragma NDIS_PAGEABLE_FUNCTION(DriverUnload)

KEVENT InitEvent;

VOID DbgTraceIonicVersionInfo(PIONIC_VERSION_INFO pVerInfo)
{    
    if (pVerInfo) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE, "|-------- %s Driver Version Info: -------\n", IONIC_MP_DRIVER_NAME));
#if defined(__DATE__)
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE, "|Built on: %s %s|\n", __DATE__, __TIME__));
#endif
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE, "|         File Version: %u.%u.%u.%u          |\n", pVerInfo->VerMaj, pVerInfo->VerMin, pVerInfo->VerSP, pVerInfo->VerBuild));
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE, "|Ver: %s|\n", pVerInfo->VerString));
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE, "----------------------------------------------\n"));
    }
}

VOID InitIonicVersionInfo(PIONIC_VERSION_INFO pVersionInfo)
{
    int i;

    pVersionInfo->VerMaj = IONIC_MAJOR_DRIVER_VERSION;
    pVersionInfo->VerMin = IONIC_MINOR_DRIVER_VERSION;
    pVersionInfo->VerSP = IONIC_SP_DRIVER_VERSION;
    pVersionInfo->VerBuild = IONIC_BUILD_DRIVER_VERSION;
    RtlStringCchCopyA(pVersionInfo->VerString, REVISION_MAX_STR_SIZE, IONIC_MP_VERSION_STRING IONIC_MP_VERSION_EXTENSION);
    for (i = 0; i < REVISION_MAX_STR_SIZE; i++) {
        pVersionInfo->VerStringW[i] = pVersionInfo->VerString[i];
    }
    DbgTraceIonicVersionInfo(pVersionInfo);
}

NDIS_STATUS
DriverEntry(void* DriverObject, void* RegistryPath)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_MINIPORT_DRIVER_CHARACTERISTICS stDriverChar;
    UNICODE_STRING *puniRegistry = (UNICODE_STRING *)RegistryPath;

    BOOLEAN bExit = FALSE;

    // DbgBreakPoint();

    if (bExit) {
        return STATUS_SUCCESS;
    }

    IonicDriverObject = (PDRIVER_OBJECT)DriverObject;

    InitIonicVersionInfo(&IonicVersionInfo);

    IoPrint("Ionic64 DriverEntry\n");

    KeInitializeEvent(&InitEvent, SynchronizationEvent, TRUE);

    NdisAllocateSpinLock(&TraceLock);

    NdisAllocateSpinLock(&AdapterListLock);
    InitializeListHead(&AdapterList);

    NdisAllocateSpinLock(&memory_block_lock);
    InitializeListHead(&memory_block_list);

    KeInitializeEvent( &perfmon_event,
                       SynchronizationEvent,
                       TRUE);

    AdapterRegistryPath.Length = puniRegistry->Length;
    AdapterRegistryPath.MaximumLength = puniRegistry->MaximumLength;

    AdapterRegistryPath.Buffer = (WCHAR *)ExAllocatePoolWithTag(
        NonPagedPoolNx, AdapterRegistryPath.MaximumLength, IONIC_GENERIC_TAG);

    if (AdapterRegistryPath.Buffer == NULL) {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    NdisZeroMemory(AdapterRegistryPath.Buffer,
                   AdapterRegistryPath.MaximumLength);

    NdisMoveMemory(AdapterRegistryPath.Buffer, puniRegistry->Buffer,
                   AdapterRegistryPath.Length);

    NdisZeroMemory(&stDriverChar, sizeof(NDIS_MINIPORT_DRIVER_CHARACTERISTICS));

    stDriverChar.Header.Type = NDIS_OBJECT_TYPE_MINIPORT_DRIVER_CHARACTERISTICS;

#if (NDIS_SUPPORT_NDIS680)
    stDriverChar.Header.Size =
        NDIS_SIZEOF_MINIPORT_DRIVER_CHARACTERISTICS_REVISION_3;
    stDriverChar.Header.Revision =
        NDIS_MINIPORT_DRIVER_CHARACTERISTICS_REVISION_3;
#else
    stDriverChar.Header.Size =
        NDIS_SIZEOF_MINIPORT_DRIVER_CHARACTERISTICS_REVISION_2;
    stDriverChar.Header.Revision =
        NDIS_MINIPORT_DRIVER_CHARACTERISTICS_REVISION_2;
#endif

    stDriverChar.MajorNdisVersion = IONIC_MP_NDIS_MAJOR_VERSION;
    stDriverChar.MinorNdisVersion = IONIC_MP_NDIS_MINOR_VERSION;

    stDriverChar.MajorDriverVersion = IONIC_MAJOR_DRIVER_VERSION;
    stDriverChar.MinorDriverVersion = IONIC_MINOR_DRIVER_VERSION;

    stDriverChar.Flags = 0;

    stDriverChar.SetOptionsHandler = SetOptions;
    stDriverChar.UnloadHandler = DriverUnload;
    stDriverChar.InitializeHandlerEx = InitializeEx;
    stDriverChar.HaltHandlerEx = HaltEx;
    stDriverChar.PauseHandler = Pause;
    stDriverChar.RestartHandler = Restart;
    stDriverChar.OidRequestHandler = OidRequest;
    stDriverChar.SendNetBufferListsHandler = ionic_send_packets;
    stDriverChar.ReturnNetBufferListsHandler = ionic_return_packet;
    stDriverChar.CancelSendHandler = CancelSend;
    stDriverChar.ShutdownHandlerEx = ShutdownEx;
    // stDriverChar.ResetHandlerEx = ResetEx;
    stDriverChar.CancelOidRequestHandler = CancelOidRequest;
    stDriverChar.DevicePnPEventNotifyHandler = PnpEventNotify;
    stDriverChar.DirectOidRequestHandler = DirectOidRequest;
    stDriverChar.CancelDirectOidRequestHandler = CancelDirectOidRequest;
    // stDriverChar.CheckForHangHandlerEx = CheckForHangEx;

#if (NDIS_SUPPORT_NDIS680)
    stDriverChar.SynchronousOidRequestHandler = SynchronousOidRequest;
#endif

    ntStatus = NdisMRegisterMiniportDriver((PDRIVER_OBJECT)DriverObject,
                                           (PUNICODE_STRING)RegistryPath, NULL,
                                           &stDriverChar, &IonicDriver);

    // register the performance counter handler for this device instance
    IonicRegisterPensando_Systems(IonicPerfCounterCallback, NULL);
    PerfMonInitialized = 1;

cleanup:

    if (NDIS_STATUS_SUCCESS != ntStatus) {
        DriverUnload((PDRIVER_OBJECT)DriverObject);

        EvLogError("DTDimm failed to load: 0x%x", ntStatus);
    } else {

        ReadConfigParams();

        if (TraceBufferLength != 0) {
            InitializeTrace();
        }

        EvLogInformational("Ionic driver successfully loaded.");
        EvLogInformational("Ionic Ver.:%s", IonicVersionInfo.VerString);
    }

    return ntStatus;
}

void
DriverUnload(PDRIVER_OBJECT DriverObject)
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(DriverObject);

    if (AdapterRegistryPath.Buffer != NULL) {
        ExFreePool(AdapterRegistryPath.Buffer);
        AdapterRegistryPath.Buffer = NULL;
    }

    TearDownTrace();

    NdisFreeSpinLock(&TraceLock);
    NdisFreeSpinLock(&AdapterListLock);

    if (IonicDriver != NULL) {
        NdisMDeregisterMiniportDriver(IonicDriver);
    }

    if (PerfMonInitialized) {
        // unregister the performance counter handler for this device instance
        IonicUnregisterPensando_Systems();
    }

    IoPrint("%s Complete\n", __FUNCTION__);

    return;
}

NDIS_STATUS
InitializeEx(NDIS_HANDLE AdapterHandle,
             NDIS_HANDLE DriverContext,
             PNDIS_MINIPORT_INIT_PARAMETERS InitParameters)
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    struct ionic *adapter =
        (struct ionic *)InitParameters->MiniportAddDeviceContext;
    NDIS_PM_CAPABILITIES stPmCaps = {0};
    NDIS_SG_DMA_DESCRIPTION stDmaDesc = {0};
    ULONG ulLength = 0;
    NDIS_HYPERVISOR_INFO stHyperVInfo;
	DEVICE_OBJECT *phys_device_obj = NULL;
	ULONG	returned_len = 0;

    UNREFERENCED_PARAMETER(AdapterHandle);
    UNREFERENCED_PARAMETER(DriverContext);

    adapter->hardware_status = NdisHardwareStatusInitializing;

    KeWaitForSingleObject(&InitEvent, Executive, KernelMode, FALSE, NULL);

    DbgTrace((TRACE_COMPONENT_HANDLERS_ENT_EX, TRACE_LEVEL_VERBOSE,
              "%s Entry for device %p IFIndex %d Flags 0x%08lX\n", __FUNCTION__,
              AdapterHandle, InitParameters->IfIndex, InitParameters->Flags));

    //
    // Reset some parameters in the event this is a restart
    //

    adapter->ConfigStatus = 0;

	//
	// Query the numa node for this device instance
	//

	NdisMGetDeviceProperty( adapter->adapterhandle,
							&phys_device_obj,
							NULL,
							NULL,
							NULL,
							NULL);

	status = IoGetDeviceNumaNode( phys_device_obj,
								  &adapter->numa_node);

	if (status != STATUS_SUCCESS) {
		IoPrint("%s Failed to retrieve NUMA node Error %08lX\n", __FUNCTION__, status);
	}

	status = IoGetDeviceProperty( phys_device_obj,
								  DevicePropertyLocationInformation,
								  IONIC_DEV_LOC_LEN * sizeof( WCHAR),
								  adapter->device_location,
								  &returned_len);
	if (status != STATUS_SUCCESS) {
		IoPrint("Failed to get device location %08lX\n", status);
	}

    //
    // Are we in the parent partition?
    //

    status = NdisGetHypervisorInfo(&stHyperVInfo);

    if (status == NDIS_STATUS_SUCCESS) {

        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                  "%s HyperV Info Present %s PartitionType %s\n", __FUNCTION__,
                  BooleanFlagOn(stHyperVInfo.Flags,
                                NDIS_HYPERVISOR_INFO_FLAG_HYPERVISOR_PRESENT)
                      ? "Yes"
                      : "No",
                  GetHyperVPartitionTypeString(stHyperVInfo.PartitionType)));

        if (BooleanFlagOn(stHyperVInfo.Flags,
                          NDIS_HYPERVISOR_INFO_FLAG_HYPERVISOR_PRESENT)) {
            if (stHyperVInfo.PartitionType ==
                NdisHypervisorPartitionTypeMsHvParent) {
                SetFlag(adapter->Flags, IONIC_FLAG_PARENT_PARTITION);
            }

            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                      "%s Adapter running in %s partition\n", __FUNCTION__,
                      BooleanFlagOn(adapter->Flags, IONIC_FLAG_PARENT_PARTITION)
                          ? "Parent"
                          : "Child"));
        }
    } else {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                  "%s NdisGetHypervisorInfo failed Status %08lX\n",
                  __FUNCTION__, status));
        status = NDIS_STATUS_SUCCESS;
    }

    if (InitParameters->PciDeviceCustomProperties != NULL) {

        DbgTrace(
            (TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
             "\tType %d Spd 0x%08lX MaxSpd 0x%08lX IntType %d MaxIntMsgs %d\n",
             InitParameters->PciDeviceCustomProperties->DeviceType,
             InitParameters->PciDeviceCustomProperties->CurrentLinkSpeed,
             InitParameters->PciDeviceCustomProperties->MaxLinkSpeed,
             InitParameters->PciDeviceCustomProperties->InterruptType,
             InitParameters->PciDeviceCustomProperties->MaxInterruptMessages));
    }

    status = SetRegistrationAttribs(adapter);

    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Failed SetRegistrationAttribs Status %08lX\n",
                  __FUNCTION__, status));
        goto exit;
    }

    //
    // Read Advanced configuration information from the registry
    //
    status = ReadRegParameters(adapter);

    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Failed ReadRegParameters Status %08lX\n", __FUNCTION__,
                  status));
        goto exit;
    }

    //
    // Get the Pci config information
    //

    ulLength = NdisMGetBusData(adapter->adapterhandle, PCI_WHICHSPACE_CONFIG,
                               FIELD_OFFSET(PCI_COMMON_CONFIG, VendorID),
                               &adapter->pci_config, PCI_COMMON_HDR_LENGTH);

    if (ulLength != PCI_COMMON_HDR_LENGTH) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Failed NdisMGetBusData Length %08lX (%08lX)\n",
                  __FUNCTION__, ulLength, PCI_COMMON_HDR_LENGTH));
        status = NDIS_STATUS_ADAPTER_NOT_FOUND;
        goto exit;
    }

    if (adapter->pci_config.VendorID != PCI_VENDOR_ID_PENSANDO) {
        status = NDIS_STATUS_ADAPTER_NOT_FOUND;
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Invalid VID-PID %08lX-%08lX\n", __FUNCTION__,
                  adapter->pci_config.VendorID, adapter->pci_config.DeviceID));
        goto exit;
    }

    if (adapter->pci_config.DeviceID == PCI_DEVICE_ID_PENSANDO_IONIC_ETH_PF ||
        adapter->pci_config.DeviceID == PCI_DEVICE_ID_PENSANDO_IONIC_ETH_MGMT) {
        SetFlag(adapter->Flags, IONIC_FLAG_PF_DEVICE);
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                  "%s PF Device %p\n", __FUNCTION__, adapter));
    } else if (adapter->pci_config.DeviceID ==
               PCI_DEVICE_ID_PENSANDO_IONIC_ETH_VF) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                  "%s VF Device %p\n", __FUNCTION__, adapter));
    } else {
        status = NDIS_STATUS_ADAPTER_NOT_FOUND;
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Invalid VID-PID %08lX-%08lX\n", __FUNCTION__,
                  adapter->pci_config.VendorID, adapter->pci_config.DeviceID));
        goto exit;
    }

    adapter->port_stats.device_id = adapter->pci_config.DeviceID;
    adapter->port_stats.vendor_id = adapter->pci_config.VendorID;

    // We want to read the PCI config space for any PF device. This could be on
    // a nested hyperv system so it would not be the parent partition but we
    // still want to read in any SRIOV caps
    if (BooleanFlagOn(adapter->Flags, IONIC_FLAG_PF_DEVICE)) {
        ReadPCIeConfigSpace(adapter);
    }

    // Grab the friendly name for the adapter
    status = NdisMQueryAdapterInstanceName( &adapter->name,
                                            AdapterHandle);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Failed to retrieve adapter name Error 0x%08lX\n", __FUNCTION__,
                  status));
        goto exit;
    }

    //
    // Map the bars over
    //

    status = ionic_map_bars(adapter, InitParameters);

    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s ionic_map_bars() failed Status %08lX\n", __FUNCTION__,
                  status));
        goto exit;
    }

    //
    // setup the device registers
    //

    status = ionic_dev_setup(adapter);

    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s ionic_dev_setup() failed Status %08lX\n", __FUNCTION__,
                  status));
        goto unregister_interrupts;
    }

    status = ionic_identify(adapter);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Cannot identify device: %d, aborting\n", __FUNCTION__,
                  status));
        goto err_out_teardown;
    }

    IoPrint("%s ASIC %s rev 0x%X serial num %s fw version %s N_LIF %d "
            "N_DBPGS_LIF %d Rx Budget %d RxPool Factor %d NUMA %d\n",
            __FUNCTION__, ionic_dev_asic_name(adapter->idev.dev_info.asic_type),
            adapter->idev.dev_info.asic_rev, adapter->idev.dev_info.serial_num,
            adapter->idev.dev_info.fw_version, adapter->ident.dev.nlifs,
            adapter->ident.dev.ndbpgs_per_lif, RxBudget,
            adapter->rx_pool_factor, adapter->numa_node);

    DbgTrace(
        (TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
         "%s ASIC %s rev 0x%X serial num %s fw version %s N_LIF %d "
            "N_DBPGS_LIF %d Rx Budget %d RxPool Factor %d NUMA %d\n",
            __FUNCTION__, ionic_dev_asic_name(adapter->idev.dev_info.asic_type),
            adapter->idev.dev_info.asic_rev, adapter->idev.dev_info.serial_num,
            adapter->idev.dev_info.fw_version, adapter->ident.dev.nlifs,
            adapter->ident.dev.ndbpgs_per_lif, RxBudget,
            adapter->rx_pool_factor, adapter->numa_node));

    status = ionic_init(adapter);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Cannot init device: %d, aborting\n", __FUNCTION__,
                  status));
        goto err_out_teardown;
    }

    /* Configure the ports */
    status = ionic_port_identify(adapter);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Cannot identify port: %d, aborting\n", __FUNCTION__,
                  status));
        goto err_out_reset;
    }

    status = ionic_port_init(adapter);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Cannot init port: %d, aborting\n", __FUNCTION__, status));
        goto err_out_reset;
    }

    /* Configure LIFs */
    status = ionic_lif_identify(adapter, IONIC_LIF_TYPE_CLASSIC,
                                &adapter->ident.lif);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Cannot identify LIFs: %d, aborting\n", __FUNCTION__,
                  status));
        goto err_out_port_reset;
    }

    status = ionic_lifs_size(adapter);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Cannot size LIFs: %08lX, aborting\n", __FUNCTION__,
                  status));
        goto err_out_port_reset;
    }

    status = ionic_lifs_alloc(adapter);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Cannot allocate LIFs: %08lX, aborting\n", __FUNCTION__,
                  status));
        goto err_out_free_irqs;
    }

	/* Don't register interrupts until we're ready */
    status = ionic_register_interrupts(adapter);

    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s ionic_register_interrupts() failed Status %08lX\n",
                  __FUNCTION__, status));
        goto unmap_bars_exit;
    }

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s TxQ cnt %d RxQ cnt %d\n", __FUNCTION__,
              adapter->master_lif->ntxqs, adapter->master_lif->nrxqs));

    status = ionic_lifs_init(adapter);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Cannot init LIFs: %08lX, aborting\n", __FUNCTION__,
                  status));
        goto err_out_free_lifs;
    }

    status = ionic_open(adapter);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Cannot open LIFs: %08lX, aborting\n", __FUNCTION__,
                  status));
        goto err_out_free_lifs;
    }

    status = SetGeneralAttribs(adapter);

    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s SetGeneralAttribs failed Status %08lX\n", __FUNCTION__,
                  status));
        goto cleanup;
    }

#if 0
    /* Initialize VMQ caps */
    status = InitHardwareConfig(adapter);

    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Failed InitHardwareConfig() Status 0x%08lX\n",
                  __FUNCTION__, status));
        goto cleanup;
    }
#endif

    status = ionic_set_offload_attributes(adapter);

    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Failed ionic_set_offload_attributes() Status 0x%08lX\n",
                  __FUNCTION__, status));
        goto cleanup;
    }

    adapter->hardware_status = NdisHardwareStatusReady;

    ionic_indicate_status(adapter, NDIS_STATUS_MEDIA_DISCONNECT, 0, 0);

    goto exit;

cleanup:
err_out_free_lifs:
    ionic_lifs_deinit(adapter);
    ionic_lifs_free(adapter);

err_out_free_irqs:
    //	ionic_bus_free_irq_vectors(adapter);

err_out_port_reset:
    ionic_port_reset(adapter);

err_out_reset:
    ionic_reset(adapter);

err_out_teardown:
    ionic_dev_teardown(adapter);

unregister_interrupts:
	if( adapter->intr_obj != NULL) {
	    NdisMDeregisterInterruptEx(adapter->intr_obj);
	}

unmap_bars_exit:
    ionic_unmap_bars(adapter);
	ionic_deinit(adapter);

exit:

    DbgTrace((TRACE_COMPONENT_HANDLERS_ENT_EX, TRACE_LEVEL_VERBOSE,
              "%s Exit status %08lX\n", __FUNCTION__, status));

    KeSetEvent(&InitEvent, 0, FALSE);

    return status;
}
