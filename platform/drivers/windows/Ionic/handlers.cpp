
#include "common.h"

NDIS_STATUS
SetOptions(NDIS_HANDLE DriverHandle, NDIS_HANDLE DriverContext)
{

    UNREFERENCED_PARAMETER(DriverHandle);
    UNREFERENCED_PARAMETER(DriverContext);

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_MINIPORT_PNP_CHARACTERISTICS pnp_chars;

    DbgTrace((TRACE_COMPONENT_HANDLERS_ENT_EX, TRACE_LEVEL_VERBOSE,
              "%s Enter\n", __FUNCTION__));

    NdisZeroMemory(&pnp_chars, sizeof(pnp_chars));
    pnp_chars.Header.Type = NDIS_OBJECT_TYPE_MINIPORT_PNP_CHARACTERISTICS;
    pnp_chars.Header.Revision = NDIS_MINIPORT_PNP_CHARACTERISTICS_REVISION_1;
    pnp_chars.Header.Size = sizeof(NDIS_MINIPORT_PNP_CHARACTERISTICS);
    pnp_chars.MiniportAddDeviceHandler = ionic_add_device;
    pnp_chars.MiniportRemoveDeviceHandler = ionic_remove_device;
    pnp_chars.MiniportFilterResourceRequirementsHandler =
        ionic_filter_resource_requirements;
    pnp_chars.MiniportStartDeviceHandler = ionic_start_device;

    ntStatus = NdisSetOptionalHandlers(
        DriverHandle, (PNDIS_DRIVER_OPTIONAL_HANDLERS)&pnp_chars);

    DbgTrace((TRACE_COMPONENT_HANDLERS_ENT_EX, TRACE_LEVEL_VERBOSE,
              "%s Exit Status %08lX\n", __FUNCTION__, ntStatus));

    return ntStatus;
}

void
HaltEx(NDIS_HANDLE MiniportAdapterContext, NDIS_HALT_ACTION HaltAction)
{

    struct ionic *ionic = (struct ionic *)MiniportAdapterContext;

    UNREFERENCED_PARAMETER(HaltAction);

    DbgTrace((TRACE_COMPONENT_HANDLERS_ENT_EX, TRACE_LEVEL_VERBOSE,
              "%s Enter Adapter %p\n", __FUNCTION__, ionic));

    if (ionic != NULL) {

        EvLogInformational("%wZ - adapter is halting.", ionic->name);

        ionic->hardware_status = NdisHardwareStatusClosing;        

        NdisCancelTimerObject(ionic->LinkCheckTimer);
        NdisFreeTimerObject(ionic->LinkCheckTimer);

        // XXX Insufficient synchronization with the link state event.
        // It could still be running, already queued, or still become queued
        // due to receiving an event on the notifyq.

        IndicateRxQueueState(ionic, 0,
                             NdisReceiveQueueOperationalStateDmaStopped);

        ionic_link_down(ionic);
       
        ionic_stop(ionic);

		deinit_dma( ionic);

        ionic_lifs_deinit(ionic);

		if( ionic->intr_obj != NULL) {
			NdisMDeregisterInterruptEx(ionic->intr_obj);
			ionic->intr_obj = NULL;
		}

        // Wait for any outstanding requests
        wait_on_requests( ionic);
        
        ionic_lifs_free(ionic);

        ionic_port_reset(ionic);
        ionic_reset(ionic);

        ionic_dev_teardown(ionic);
        ionic_unmap_bars(ionic);

        ionic_deinit(ionic);
    }

    DbgTrace((TRACE_COMPONENT_HANDLERS_ENT_EX, TRACE_LEVEL_VERBOSE, "%s Exit\n",
              __FUNCTION__));

    return;
}

NDIS_STATUS
Pause(NDIS_HANDLE MiniportAdapterContext,
      PNDIS_MINIPORT_PAUSE_PARAMETERS PauseParameters)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    struct ionic *ionic = (struct ionic *)MiniportAdapterContext;

    UNREFERENCED_PARAMETER(PauseParameters);

    DbgTrace((TRACE_COMPONENT_HANDLERS_ENT_EX, TRACE_LEVEL_VERBOSE,
              "%s Enter adapter %p\n", __FUNCTION__, ionic));

    ionic->hardware_status = NdisHardwareStatusReset;
    SetFlag(ionic->Flags, IONIC_FLAG_PAUSED);

    return ntStatus;
}

NDIS_STATUS
Restart(NDIS_HANDLE MiniportAdapterContext,
        PNDIS_MINIPORT_RESTART_PARAMETERS RestartParameters)
{

    struct ionic *ionic = (struct ionic *)MiniportAdapterContext;
    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_RESTART_ATTRIBUTES *currAttribs = NULL;
    NDIS_RESTART_GENERAL_ATTRIBUTES *restartAttribs = NULL;

    UNREFERENCED_PARAMETER(MiniportAdapterContext);

    DbgTrace((TRACE_COMPONENT_HANDLERS_ENT_EX, TRACE_LEVEL_VERBOSE,
              "%s Enter adapter %p\n", __FUNCTION__, ionic));

    if (RestartParameters != NULL) {

        currAttribs = RestartParameters->RestartAttributes;

        while (currAttribs != NULL) {

            if (currAttribs->Oid == OID_GEN_MINIPORT_RESTART_ATTRIBUTES) {

                restartAttribs =
                    (NDIS_RESTART_GENERAL_ATTRIBUTES *)currAttribs->Data;

                DbgTrace(
                    (TRACE_COMPONENT_GENERAL, TRACE_LEVEL_VERBOSE,
                     "\tMtu %d MaxXmit %lld MaxRcv %lld MacOptions 0x%08lX "
                     "AccType %d ConnType %d\n",
                     restartAttribs->MtuSize, restartAttribs->MaxXmitLinkSpeed,
                     restartAttribs->MaxRcvLinkSpeed,
                     restartAttribs->MacOptions, restartAttribs->AccessType,
                     restartAttribs->ConnectionType));

                if (restartAttribs->MtuSize == 0) {
                    DbgTrace((TRACE_COMPONENT_GENERAL, TRACE_LEVEL_VERBOSE,
                              "\tUpdating Mtu to %d\n",
                              ionic->frame_size));

                    restartAttribs->MtuSize = ionic->frame_size - ETH_COMPLETE_HDR;
                }
            } else {
                DbgTrace((TRACE_COMPONENT_GENERAL, TRACE_LEVEL_VERBOSE,
                          "\tNon-Restart Oid 0x%08lX\n", currAttribs->Oid));
            }

            currAttribs = currAttribs->Next;
        }
    } else {
        DbgTrace((TRACE_COMPONENT_GENERAL, TRACE_LEVEL_VERBOSE,
                  "%s No restart parameters\n", __FUNCTION__));
    }

    ionic->hardware_status = NdisHardwareStatusReady;
    ClearFlag(ionic->Flags, IONIC_FLAG_PAUSED);

    ionic_link_status_check(ionic->master_lif, PORT_OPER_STATUS_NONE);

    DbgTrace((TRACE_COMPONENT_HANDLERS_ENT_EX, TRACE_LEVEL_VERBOSE,
              "%s Exit status %08lX\n", __FUNCTION__, ntStatus));

    return ntStatus;
}

void
CancelSend(NDIS_HANDLE MiniportAdapterContext, PVOID CancelId)
{

    UNREFERENCED_PARAMETER(MiniportAdapterContext);
    UNREFERENCED_PARAMETER(CancelId);

    DbgTrace((TRACE_COMPONENT_HANDLERS_ENT_EX, TRACE_LEVEL_VERBOSE,
              "%s Enter\n", __FUNCTION__));

    return;
}

NDIS_STATUS
ResetEx(NDIS_HANDLE MiniportAdapterContext, PBOOLEAN AddressingReset)
{

    struct ionic *ionic = (struct ionic *)MiniportAdapterContext;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    DbgTrace((TRACE_COMPONENT_HANDLERS_ENT_EX, TRACE_LEVEL_VERBOSE,
              "%s Enter adapter %p\n", __FUNCTION__, MiniportAdapterContext));

    *AddressingReset = FALSE;
    ClearFlag(ionic->Flags, IONIC_FLAG_PAUSED);

    return status;
}

void
ShutdownEx(NDIS_HANDLE MiniportAdapterContext,
           NDIS_SHUTDOWN_ACTION ShutdownAction)
{

    UNREFERENCED_PARAMETER(MiniportAdapterContext);
    UNREFERENCED_PARAMETER(ShutdownAction);

    DbgTrace((TRACE_COMPONENT_HANDLERS_ENT_EX, TRACE_LEVEL_VERBOSE,
              "%s Enter adapter %p\n", __FUNCTION__, MiniportAdapterContext));

    return;
}

NDIS_STATUS
SetGeneralAttribs(struct ionic *Adapter)
{
    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_MINIPORT_ADAPTER_GENERAL_ATTRIBUTES stAttribs;
    NDIS_RECEIVE_SCALE_CAPABILITIES rss;
    NDIS_PM_CAPABILITIES pwrCapabilities;
	ULONGLONG link_spd = 0;

    NdisZeroMemory(&stAttribs,
                   sizeof(NDIS_MINIPORT_ADAPTER_GENERAL_ATTRIBUTES));

    stAttribs.Header.Type =
        NDIS_OBJECT_TYPE_MINIPORT_ADAPTER_GENERAL_ATTRIBUTES;
    stAttribs.Header.Revision =
        NDIS_MINIPORT_ADAPTER_GENERAL_ATTRIBUTES_REVISION_2;
    stAttribs.Header.Size =
        NDIS_SIZEOF_MINIPORT_ADAPTER_GENERAL_ATTRIBUTES_REVISION_2;

    stAttribs.Flags = 0;
    stAttribs.MediaType = NdisMedium802_3;
    stAttribs.PhysicalMediumType = NdisPhysicalMedium802_3;

    stAttribs.MtuSize = Adapter->frame_size - ETH_COMPLETE_HDR;

    stAttribs.AutoNegotiationFlags = NDIS_LINK_STATE_XMIT_LINK_SPEED_AUTO_NEGOTIATED |
                                                    NDIS_LINK_STATE_RCV_LINK_SPEED_AUTO_NEGOTIATED;

    link_spd =  le32_to_cpu(Adapter->master_lif->info->status.link_speed);
    link_spd *= MEGABITS_PER_SECOND;

    stAttribs.XmitLinkSpeed = stAttribs.RcvLinkSpeed = link_spd;
    stAttribs.MaxXmitLinkSpeed = stAttribs.MaxRcvLinkSpeed = link_spd;

    stAttribs.MediaConnectState = MediaConnectStateDisconnected;
    stAttribs.MediaDuplexState = MediaDuplexStateFull;
    stAttribs.LookaheadSize = Adapter->frame_size;
    stAttribs.PowerManagementCapabilities = NULL;
    stAttribs.MacOptions = NDIS_MAC_OPTION_TRANSFERS_NOT_PEND |
                           NDIS_MAC_OPTION_NO_LOOPBACK |
                           NDIS_MAC_OPTION_FULL_DUPLEX |
                           NDIS_MAC_OPTION_SUPPORTS_MAC_ADDRESS_OVERWRITE;

    if (BooleanFlagOn(Adapter->ConfigStatus, IONIC_PRIORITY_ENABLED)) {
        stAttribs.MacOptions |= NDIS_MAC_OPTION_8021P_PRIORITY;
    }

    if (BooleanFlagOn(Adapter->ConfigStatus, IONIC_VLAN_ENABLED)) {
        stAttribs.MacOptions |= NDIS_MAC_OPTION_8021Q_VLAN;
    }

    stAttribs.SupportedPacketFilters =
        NDIS_PACKET_TYPE_DIRECTED | NDIS_PACKET_TYPE_MULTICAST |
        NDIS_PACKET_TYPE_BROADCAST | NDIS_PACKET_TYPE_PROMISCUOUS |
        NDIS_PACKET_TYPE_ALL_MULTICAST;
    stAttribs.MaxMulticastListSize = IONIC_MULTICAST_PERFECT_FILTERS;

    stAttribs.MacAddressLength = ETH_ALEN;

    NdisMoveMemory(&stAttribs.PermanentMacAddress[0], &Adapter->perm_addr[0],
                   stAttribs.MacAddressLength);

    DbgTrace(
        (TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
         "%s Returning Perm MAC %02lX:%02lX:%02lX:%02lX:%02lX:%02lX\n",
         __FUNCTION__, stAttribs.PermanentMacAddress[0],
         stAttribs.PermanentMacAddress[1], stAttribs.PermanentMacAddress[2],
         stAttribs.PermanentMacAddress[3], stAttribs.PermanentMacAddress[4],
         stAttribs.PermanentMacAddress[5]));

    NdisMoveMemory(&stAttribs.CurrentMacAddress[0], &Adapter->config_addr[0],
                   stAttribs.MacAddressLength);

    if (BooleanFlagOn(Adapter->ConfigStatus, IONIC_RSS_ENABLED)) {

        NdisZeroMemory(&rss, sizeof(NDIS_RECEIVE_SCALE_CAPABILITIES));
        rss.Header.Type = NDIS_OBJECT_TYPE_RSS_CAPABILITIES;

#if (NDIS_SUPPORT_NDIS660)
        rss.Header.Revision = NDIS_RECEIVE_SCALE_CAPABILITIES_REVISION_3;
        rss.Header.Size = NDIS_SIZEOF_RECEIVE_SCALE_CAPABILITIES_REVISION_3;
#else
        rss.Header.Revision = NDIS_RECEIVE_SCALE_CAPABILITIES_REVISION_2;
        rss.Header.Size = NDIS_SIZEOF_RECEIVE_SCALE_CAPABILITIES_REVISION_2;
#endif

        rss.CapabilitiesFlags =
            NDIS_RSS_CAPS_MESSAGE_SIGNALED_INTERRUPTS |
            NDIS_RSS_CAPS_CLASSIFICATION_AT_ISR | NDIS_RSS_CAPS_USING_MSI_X |
            NDIS_RSS_CAPS_SUPPORTS_MSI_X | NDIS_RSS_CAPS_HASH_TYPE_TCP_IPV4 |
            NDIS_RSS_CAPS_HASH_TYPE_TCP_IPV6 | NdisHashFunctionToeplitz;
        rss.NumberOfInterruptMessages = Adapter->ident.dev.nintrs;

        if (Adapter->num_rss_queues != 0) {
            rss.NumberOfReceiveQueues = Adapter->num_rss_queues;
        } else {
            rss.NumberOfReceiveQueues = Adapter->nrxqs_per_lif;
        }

        rss.NumberOfIndirectionTableEntries = Adapter->ident.lif.eth.rss_ind_tbl_sz;
        stAttribs.RecvScaleCapabilities = &rss;
    } else {
        stAttribs.RecvScaleCapabilities = NULL;
    }

    stAttribs.AccessType = NET_IF_ACCESS_BROADCAST;
    stAttribs.DirectionType = NET_IF_DIRECTION_SENDRECEIVE;
    stAttribs.ConnectionType =
        NET_IF_CONNECTION_DEDICATED; // IF_CONNECTION_DEDICATED;
    stAttribs.IfType = IF_TYPE_ETHERNET_CSMACD;
    stAttribs.IfConnectorPresent = TRUE;
    stAttribs.SupportedStatistics =
        NDIS_STATISTICS_XMIT_OK_SUPPORTED | NDIS_STATISTICS_RCV_OK_SUPPORTED |
        NDIS_STATISTICS_XMIT_ERROR_SUPPORTED |
        NDIS_STATISTICS_RCV_ERROR_SUPPORTED |
        NDIS_STATISTICS_RCV_NO_BUFFER_SUPPORTED |
        NDIS_STATISTICS_DIRECTED_BYTES_XMIT_SUPPORTED |
        NDIS_STATISTICS_DIRECTED_FRAMES_XMIT_SUPPORTED |
        NDIS_STATISTICS_MULTICAST_BYTES_XMIT_SUPPORTED |
        NDIS_STATISTICS_MULTICAST_FRAMES_XMIT_SUPPORTED |
        NDIS_STATISTICS_BROADCAST_BYTES_XMIT_SUPPORTED |
        NDIS_STATISTICS_BROADCAST_FRAMES_XMIT_SUPPORTED |
        NDIS_STATISTICS_DIRECTED_BYTES_RCV_SUPPORTED |
        NDIS_STATISTICS_DIRECTED_FRAMES_RCV_SUPPORTED |
        NDIS_STATISTICS_MULTICAST_BYTES_RCV_SUPPORTED |
        NDIS_STATISTICS_MULTICAST_FRAMES_RCV_SUPPORTED |
        NDIS_STATISTICS_BROADCAST_BYTES_RCV_SUPPORTED |
        NDIS_STATISTICS_BROADCAST_FRAMES_RCV_SUPPORTED |
        NDIS_STATISTICS_RCV_CRC_ERROR_SUPPORTED |
        NDIS_STATISTICS_TRANSMIT_QUEUE_LENGTH_SUPPORTED |
        NDIS_STATISTICS_BYTES_RCV_SUPPORTED |
        NDIS_STATISTICS_BYTES_XMIT_SUPPORTED |
        NDIS_STATISTICS_RCV_DISCARDS_SUPPORTED |
        NDIS_STATISTICS_GEN_STATISTICS_SUPPORTED |
        NDIS_STATISTICS_XMIT_DISCARDS_SUPPORTED;

    // Set flow control attributes
    if( Adapter->flow_control == IONIC_FC_DISABLED) {
        stAttribs.SupportedPauseFunctions = NdisPauseFunctionsUnsupported;
    }
    else if (Adapter->flow_control == IONIC_FC_RX_ENABLED) {
        stAttribs.SupportedPauseFunctions = NdisPauseFunctionsReceiveOnly;
    }
    else if (Adapter->flow_control == IONIC_FC_TX_ENABLED) {
        stAttribs.SupportedPauseFunctions = NdisPauseFunctionsSendOnly;
    }
    else if (Adapter->flow_control == IONIC_FC_TXRX_ENABLED) {
        stAttribs.SupportedPauseFunctions = NdisPauseFunctionsSendAndReceive;
    }

    stAttribs.DataBackFillSize = 0;
    stAttribs.ContextBackFillSize = 0;

    ionic_get_supported_oids(&stAttribs.SupportedOidList,
                             &stAttribs.SupportedOidListLength);

    NdisZeroMemory(&pwrCapabilities, sizeof(pwrCapabilities));

    pwrCapabilities.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
    pwrCapabilities.Header.Size = NDIS_SIZEOF_NDIS_PM_CAPABILITIES_REVISION_2;
    pwrCapabilities.Header.Revision = NDIS_PM_CAPABILITIES_REVISION_2;

    pwrCapabilities.MinMagicPacketWakeUp = NdisDeviceStateUnspecified;
    pwrCapabilities.MinPatternWakeUp = NdisDeviceStateUnspecified;
    pwrCapabilities.MinLinkChangeWakeUp = NdisDeviceStateUnspecified;

    stAttribs.PowerManagementCapabilitiesEx = &pwrCapabilities;

    ntStatus = NdisMSetMiniportAttributes(
        Adapter->adapterhandle, (PNDIS_MINIPORT_ADAPTER_ATTRIBUTES)&stAttribs);

    return ntStatus;
}

NDIS_STATUS
SetRegistrationAttribs(struct ionic *adapter)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_MINIPORT_ADAPTER_REGISTRATION_ATTRIBUTES stRegistrationAttribs = {0};

    //
    // Setup the registration attributes
    //

    stRegistrationAttribs.Header.Type =
        NDIS_OBJECT_TYPE_MINIPORT_ADAPTER_REGISTRATION_ATTRIBUTES;
    stRegistrationAttribs.Header.Size =
        NDIS_SIZEOF_MINIPORT_ADAPTER_REGISTRATION_ATTRIBUTES_REVISION_2;
    stRegistrationAttribs.Header.Revision =
        NDIS_MINIPORT_ADAPTER_REGISTRATION_ATTRIBUTES_REVISION_2;

    stRegistrationAttribs.MiniportAdapterContext = (NDIS_HANDLE)adapter;

    stRegistrationAttribs.AttributeFlags =
        NDIS_MINIPORT_ATTRIBUTES_HARDWARE_DEVICE |
        NDIS_MINIPORT_ATTRIBUTES_BUS_MASTER |
        NDIS_MINIPORT_ATTRIBUTES_REGISTER_BUGCHECK_CALLBACK;

    stRegistrationAttribs.CheckForHangTimeInSeconds =
        NIC_ADAPTER_CHECK_FOR_HANG_TIMEOUT;
    stRegistrationAttribs.InterfaceType = NdisInterfacePci;

    NDIS_DECLARE_MINIPORT_ADAPTER_CONTEXT(struct ionic);

    ntStatus = NdisMSetMiniportAttributes(
        adapter->adapterhandle,
        (PNDIS_MINIPORT_ADAPTER_ATTRIBUTES)&stRegistrationAttribs);

    return ntStatus;
}

BOOLEAN
CheckForHangEx(NDIS_HANDLE MiniportAdapterContext)
{
    BOOLEAN miniport_hung = FALSE;

    UNREFERENCED_PARAMETER(MiniportAdapterContext);

    DbgTrace((TRACE_COMPONENT_HANDLERS_ENT_EX, TRACE_LEVEL_VERBOSE,
              "%s Enter adapter %p\n", __FUNCTION__, MiniportAdapterContext));

    return miniport_hung;
}
