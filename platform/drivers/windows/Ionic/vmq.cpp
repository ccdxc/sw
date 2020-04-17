
#include "common.h"

NDIS_STATUS
InitHardwareConfig(struct ionic *Adapter)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;

    NDIS_MINIPORT_ADAPTER_HARDWARE_ASSIST_ATTRIBUTES
    ndisHardwareAssistAttributes;
    NDIS_RECEIVE_FILTER_CAPABILITIES ndisFilterCapabilities,
        ndisCurrentFilterCapabilities;
    NDIS_SRIOV_CAPABILITIES ndisSriovCapabilities, ndisCurrentSriovCapabilities;
    NDIS_NIC_SWITCH_CAPABILITIES ndisNicSwitchCapabilities,
        ndisCurrentNicSwitchCapabilities;

    DbgTrace(
        (TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
         "%s CurrentConfig SRIOV %s VMQ %s RSS %s\n", __FUNCTION__,
         BooleanFlagOn(Adapter->ConfigStatus, IONIC_SRIOV_ENABLED) ? "Yes"
                                                                   : "No",
         BooleanFlagOn(Adapter->ConfigStatus, IONIC_VMQ_ENABLED) ? "Yes" : "No",
         BooleanFlagOn(Adapter->ConfigStatus, IONIC_RSS_ENABLED) ? "Yes"
                                                                 : "No"));

    //
    // Set the VMQ adapter capabilities.
    //
    NdisZeroMemory(&ndisHardwareAssistAttributes,
                   sizeof(NDIS_MINIPORT_ADAPTER_HARDWARE_ASSIST_ATTRIBUTES));

    ndisHardwareAssistAttributes.Header.Type =
        NDIS_OBJECT_TYPE_MINIPORT_ADAPTER_HARDWARE_ASSIST_ATTRIBUTES;
    ndisHardwareAssistAttributes.Header.Revision =
        NDIS_MINIPORT_ADAPTER_HARDWARE_ASSIST_ATTRIBUTES_REVISION_3;
    ndisHardwareAssistAttributes.Header.Size =
        NDIS_SIZEOF_MINIPORT_ADAPTER_HARDWARE_ASSIST_ATTRIBUTES_REVISION_3;

    //
    // Init the Sriov h/w capabilities
    //

    NdisZeroMemory(&ndisSriovCapabilities, sizeof(NDIS_SRIOV_CAPABILITIES));
    NdisZeroMemory(&ndisCurrentSriovCapabilities,
                   sizeof(NDIS_SRIOV_CAPABILITIES));

    ndisSriovCapabilities.Header.Revision = NDIS_SRIOV_CAPABILITIES_REVISION_1;
    ndisSriovCapabilities.Header.Size =
        NDIS_SIZEOF_SRIOV_CAPABILITIES_REVISION_1;
    ndisSriovCapabilities.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;

    ndisCurrentSriovCapabilities.Header.Revision =
        NDIS_SRIOV_CAPABILITIES_REVISION_1;
    ndisCurrentSriovCapabilities.Header.Size =
        NDIS_SIZEOF_SRIOV_CAPABILITIES_REVISION_1;
    ndisCurrentSriovCapabilities.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;

    if (BooleanFlagOn(Adapter->ConfigStatus, IONIC_SRIOV_CAPABLE)) {
        if (BooleanFlagOn(Adapter->Flags,
                          IONIC_FLAG_PF_DEVICE)) { // If its a PF
            ndisSriovCapabilities.SriovCapabilities =
                NDIS_SRIOV_CAPS_SRIOV_SUPPORTED | NDIS_SRIOV_CAPS_PF_MINIPORT;
            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                      "%s Settings for SRIOV PF\n", __FUNCTION__));
        } else {
            ndisSriovCapabilities.SriovCapabilities =
                NDIS_SRIOV_CAPS_SRIOV_SUPPORTED | NDIS_SRIOV_CAPS_VF_MINIPORT;
            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                      "%s Settings for SRIOV VF\n", __FUNCTION__));
        }
    } else {
        ndisSriovCapabilities.SriovCapabilities = NDIS_SRIOV_CAPS_PF_MINIPORT;
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                  "%s Settings for PF\n", __FUNCTION__));
    }

    if (BooleanFlagOn(Adapter->ConfigStatus, IONIC_SRIOV_ENABLED) ||
        !BooleanFlagOn(Adapter->Flags,
                       IONIC_FLAG_PF_DEVICE)) { // Do this for VFs as well

        NdisMoveMemory(&ndisCurrentSriovCapabilities, &ndisSriovCapabilities,
                       sizeof(NDIS_SRIOV_CAPABILITIES));

        Adapter->SriovSwitch.Id = (ULONG)-1;
    }

    ndisHardwareAssistAttributes.HardwareSriovCapabilities =
        &ndisSriovCapabilities;
    ndisHardwareAssistAttributes.CurrentSriovCapabilities =
        &ndisCurrentSriovCapabilities;

    NdisZeroMemory(&ndisNicSwitchCapabilities,
                   sizeof(NDIS_NIC_SWITCH_CAPABILITIES));
    NdisZeroMemory(&ndisCurrentNicSwitchCapabilities,
                   sizeof(NDIS_NIC_SWITCH_CAPABILITIES));

    ndisNicSwitchCapabilities.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
    ndisCurrentNicSwitchCapabilities.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;

#if (NDIS_SUPPORT_NDIS660)
    ndisNicSwitchCapabilities.Header.Size =
        NDIS_SIZEOF_NIC_SWITCH_CAPABILITIES_REVISION_3;
    ndisNicSwitchCapabilities.Header.Revision =
        NDIS_NIC_SWITCH_CAPABILITIES_REVISION_3;
    ndisCurrentNicSwitchCapabilities.Header.Size =
        NDIS_SIZEOF_NIC_SWITCH_CAPABILITIES_REVISION_3;
    ndisCurrentNicSwitchCapabilities.Header.Revision =
        NDIS_NIC_SWITCH_CAPABILITIES_REVISION_3;
#else
    ndisNicSwitchCapabilities.Header.Size =
        NDIS_SIZEOF_NIC_SWITCH_CAPABILITIES_REVISION_2;
    ndisNicSwitchCapabilities.Header.Revision =
        NDIS_NIC_SWITCH_CAPABILITIES_REVISION_2;
    ndisCurrentNicSwitchCapabilities.Header.Size =
        NDIS_SIZEOF_NIC_SWITCH_CAPABILITIES_REVISION_2;
    ndisCurrentNicSwitchCapabilities.Header.Revision =
        NDIS_NIC_SWITCH_CAPABILITIES_REVISION_2;
#endif

#if (NDIS_SUPPORT_NDIS650)
    ndisNicSwitchCapabilities.NicSwitchCapabilities =
        NDIS_NIC_SWITCH_CAPS_RSS_ON_PF_VPORTS_SUPPORTED |
        NDIS_NIC_SWITCH_CAPS_ASYMMETRIC_QUEUE_PAIRS_FOR_NONDEFAULT_VPORT_SUPPORTED |
        NDIS_NIC_SWITCH_CAPS_RSS_PER_PF_VPORT_INDIRECTION_TABLE_SUPPORTED |
        NDIS_NIC_SWITCH_CAPS_RSS_PER_PF_VPORT_HASH_FUNCTION_SUPPORTED |
        NDIS_NIC_SWITCH_CAPS_RSS_PER_PF_VPORT_HASH_TYPE_SUPPORTED |
        NDIS_NIC_SWITCH_CAPS_RSS_PER_PF_VPORT_HASH_KEY_SUPPORTED |
        NDIS_NIC_SWITCH_CAPS_VF_RSS_SUPPORTED |
        NDIS_NIC_SWITCH_CAPS_SINGLE_VPORT_POOL |
        NDIS_NIC_SWITCH_CAPS_VLAN_SUPPORTED;
#else
    ndisNicSwitchCapabilities.NicSwitchCapabilities =
        NDIS_NIC_SWITCH_CAPS_VF_RSS_SUPPORTED |
        NDIS_NIC_SWITCH_CAPS_SINGLE_VPORT_POOL |
        NDIS_NIC_SWITCH_CAPS_VLAN_SUPPORTED;
#endif

    ndisNicSwitchCapabilities.MaxNumVFs = Adapter->sriov_caps.NumVFs;
    ndisNicSwitchCapabilities.MaxNumVPorts = Adapter->ident.dev.nlifs;

    ndisNicSwitchCapabilities.MaxNumQueuePairs = Adapter->nintrs;

#if (NDIS_SUPPORT_NDIS660)
    ndisNicSwitchCapabilities.MaxNumQueuePairsForDefaultVPort =
        Adapter->nrxqs_per_lif;

    ndisNicSwitchCapabilities.MaxNumRssCapableNonDefaultPFVPorts =
        Adapter->nintrs;

    ndisNicSwitchCapabilities.NumberOfIndirectionTableEntriesForDefaultVPort =
        Adapter->ident.lif.eth.rss_ind_tbl_sz;

    ndisNicSwitchCapabilities
        .NumberOfIndirectionTableEntriesPerNonDefaultPFVPort =
        Adapter->ident.lif.eth.rss_ind_tbl_sz;
#endif

    ndisNicSwitchCapabilities.MaxNumQueuePairsPerNonDefaultVPort =
        Adapter->nrxqs_per_lif;

    ndisNicSwitchCapabilities.MaxNumSwitches = 1;

    ndisNicSwitchCapabilities.MaxNumMacAddresses =
        Adapter->ident.lif.eth.max_ucast_filters;

    if (BooleanFlagOn(Adapter->ConfigStatus, IONIC_SRIOV_ENABLED)) {
        NdisMoveMemory(&ndisCurrentNicSwitchCapabilities,
                       &ndisNicSwitchCapabilities,
                       sizeof(NDIS_NIC_SWITCH_CAPABILITIES));
    }

    ndisHardwareAssistAttributes.CurrentNicSwitchCapabilities =
        &ndisCurrentNicSwitchCapabilities;
    ndisHardwareAssistAttributes.HardwareNicSwitchCapabilities =
        &ndisNicSwitchCapabilities;

    //
    // Filter capabilities for VMQ
    //

    NdisZeroMemory(&ndisFilterCapabilities,
                   sizeof(NDIS_RECEIVE_FILTER_CAPABILITIES));
    NdisZeroMemory(&ndisCurrentFilterCapabilities,
                   sizeof(NDIS_RECEIVE_FILTER_CAPABILITIES));

    ndisFilterCapabilities.Header.Revision =
        NDIS_RECEIVE_FILTER_CAPABILITIES_REVISION_2;
    ndisFilterCapabilities.Header.Size =
        NDIS_SIZEOF_RECEIVE_FILTER_CAPABILITIES_REVISION_2;
    ndisFilterCapabilities.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
    ndisCurrentFilterCapabilities.Header.Revision =
        NDIS_RECEIVE_FILTER_CAPABILITIES_REVISION_2;
    ndisCurrentFilterCapabilities.Header.Size =
        NDIS_SIZEOF_RECEIVE_FILTER_CAPABILITIES_REVISION_2;
    ndisCurrentFilterCapabilities.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;

    if (BooleanFlagOn(Adapter->Flags, IONIC_FLAG_PF_DEVICE)) {

        //
        // Enable VMQ filters
        //
        ndisFilterCapabilities.EnabledFilterTypes =
            NDIS_RECEIVE_FILTER_VMQ_FILTERS_ENABLED;

        //
        // Enable VMQ queues
        //

        ndisFilterCapabilities.EnabledQueueTypes =
            NDIS_RECEIVE_FILTER_VM_QUEUES_ENABLED;

        //
        // Queue properties
        //
        ndisFilterCapabilities.NumQueues = Adapter->max_vmq_cnt;
        ndisFilterCapabilities.SupportedQueueProperties =
            NDIS_RECEIVE_FILTER_VM_QUEUE_SUPPORTED |
            NDIS_RECEIVE_FILTER_MSI_X_SUPPORTED |
            NDIS_RECEIVE_FILTER_DYNAMIC_PROCESSOR_AFFINITY_CHANGE_SUPPORTED |
            NDIS_RECEIVE_FILTER_INTERRUPT_VECTOR_COALESCING_SUPPORTED;
    }

    ndisFilterCapabilities.SupportedFilterTests =
        NDIS_RECEIVE_FILTER_TEST_HEADER_FIELD_EQUAL_SUPPORTED;

    ndisFilterCapabilities.SupportedHeaders =
        NDIS_RECEIVE_FILTER_MAC_HEADER_SUPPORTED;
    ndisFilterCapabilities.SupportedMacHeaderFields =
        NDIS_RECEIVE_FILTER_MAC_HEADER_DEST_ADDR_SUPPORTED;

    if (BooleanFlagOn(Adapter->ConfigStatus,
                      IONIC_VMQ_VLAN_FILTERING_ENABLED)) {
        ndisFilterCapabilities.SupportedMacHeaderFields |=
            NDIS_RECEIVE_FILTER_MAC_HEADER_VLAN_ID_SUPPORTED;
    }

    ndisFilterCapabilities.MaxMacHeaderFilters =
        Adapter->ident.lif.eth.max_ucast_filters;

    if (BooleanFlagOn(Adapter->ConfigStatus, IONIC_SRIOV_ENABLED) ||
        BooleanFlagOn(Adapter->ConfigStatus, IONIC_VMQ_ENABLED)) {

        //
        // Adjust depending on what is enabled
        //

        if (BooleanFlagOn(Adapter->ConfigStatus, IONIC_SRIOV_ENABLED)) {

            ndisFilterCapabilities.NumQueues =
                0; // Sriov uses vports not vmqueues

            if (!BooleanFlagOn(Adapter->ConfigStatus, IONIC_VMQ_ENABLED)) {
                ndisFilterCapabilities.EnabledQueueTypes = 0;
            }
        }

        NdisMoveMemory(&ndisCurrentFilterCapabilities, &ndisFilterCapabilities,
                       sizeof(NDIS_RECEIVE_FILTER_CAPABILITIES));
    }

    ndisHardwareAssistAttributes.CurrentReceiveFilterCapabilities =
        &ndisCurrentFilterCapabilities;
    ndisHardwareAssistAttributes.HardwareReceiveFilterCapabilities =
        &ndisFilterCapabilities;

    ntStatus = NdisMSetMiniportAttributes(
        Adapter->adapterhandle,
        (PNDIS_MINIPORT_ADAPTER_ATTRIBUTES)&ndisHardwareAssistAttributes);
    if (ntStatus != NDIS_STATUS_SUCCESS) {

        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s NdisMSetMiniportAttributes Failed status %08lX\n",
                  __FUNCTION__, ntStatus));
    }

    return ntStatus;
}

NDIS_STATUS
oid_filter_allocate_queue(struct ionic *ionic, NDIS_OID_REQUEST *OidRequest)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_RECEIVE_QUEUE_PARAMETERS *pParams =
        (NDIS_RECEIVE_QUEUE_PARAMETERS *)
            OidRequest->DATA.METHOD_INFORMATION.InformationBuffer;
    LOCK_STATE_EX lock_state;
    BOOLEAN bReleaseLock = FALSE;

    OidRequest->DATA.METHOD_INFORMATION.BytesRead =
        OidRequest->DATA.METHOD_INFORMATION.InputBufferLength;
    OidRequest->DATA.METHOD_INFORMATION.BytesWritten =
        OidRequest->DATA.METHOD_INFORMATION.OutputBufferLength;

    //
    // Some validation
    //
    if (pParams->QueueId == NDIS_DEFAULT_RECEIVE_QUEUE_ID ||
        pParams->QueueType != NdisReceiveQueueTypeVMQueue) {
        ntStatus = NDIS_STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

    NdisAcquireRWLockWrite(ionic->vm_queue[pParams->QueueId].QueueLock,
                           &lock_state, 0);
    bReleaseLock = TRUE;

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Flags %08lX Type %08lX Id %d RecvBuffers %d PortId %d "
              "affinit mask %08lX group %08lX\n",
              __FUNCTION__, pParams->Flags, pParams->QueueType,
              pParams->QueueId, pParams->NumSuggestedReceiveBuffers,
              pParams->PortId, pParams->ProcessorAffinity.Mask,
              pParams->ProcessorAffinity.Group));

    //
    // Validate some parameters
    //

    if (!IsValidAffinity(ionic, pParams->ProcessorAffinity)) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Invalid affinity mask %08lX group %08lX\n", __FUNCTION__,
                  pParams->ProcessorAffinity.Mask,
                  pParams->ProcessorAffinity.Group));
        ntStatus = NDIS_STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

    if (!BooleanFlagOn(ionic->vm_queue[pParams->QueueId].Flags,
                       IONIC_QUEUE_STATE_INITIALIZED)) {

        //
        // Drop the lock while allocating buffers, etc. At DISPATCH
        //

        NdisReleaseRWLock(ionic->vm_queue[pParams->QueueId].QueueLock,
                          &lock_state);
        bReleaseLock = FALSE;

        ASSERT(ionic->vm_queue[pParams->QueueId].lif == NULL);

        ionic->vm_queue[pParams->QueueId].lif =
            (struct lif *)ionic_allocate_slave_lif(ionic);

        if (ionic->vm_queue[pParams->QueueId].lif == NULL) {
            ntStatus = NDIS_STATUS_RESOURCES;
            goto cleanup;
        }

        ionic->vm_queue[pParams->QueueId].lif->lif_stats->lif_type =
            IONIC_LIF_TYPE_VMQ;

        SetFlag(ionic->vm_queue[pParams->QueueId].lif->flags,
                IONIC_LIF_FLAG_TYPE_VMQ);

        ionic->vm_queue[pParams->QueueId].QueueId = pParams->QueueId;

        ionic->vm_queue[pParams->QueueId].ReceiveBuffers =
            pParams->NumSuggestedReceiveBuffers;

        ionic->vm_queue[pParams->QueueId].ProcessorAffinity =
            pParams->ProcessorAffinity;

        ionic->vm_queue[pParams->QueueId].lif->type_name.Length =
            pParams->QueueName.Length;

        if (pParams->QueueName.Length != 0) {
            NdisMoveMemory(
                ionic->vm_queue[pParams->QueueId].lif->type_name.String,
                pParams->QueueName.String, pParams->QueueName.Length);
        }

        SetFlag(ionic->vm_queue[pParams->QueueId].Flags,
                IONIC_QUEUE_STATE_INITIALIZED);
    } else if (BooleanFlagOn(pParams->Flags,
                             NDIS_RECEIVE_QUEUE_PARAMETERS_FLAGS_CHANGED)) {
        ASSERT(FALSE);
    } else {
        ASSERT(FALSE);
    }

cleanup:

    if (bReleaseLock) {
        NdisReleaseRWLock(ionic->vm_queue[pParams->QueueId].QueueLock,
                          &lock_state);
    }

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Complete for queue Id %d Status 0x%08lX\n", __FUNCTION__,
              pParams->QueueId, ntStatus));

    return ntStatus;
}

NDIS_STATUS
oid_free_queue(struct ionic *ionic, NDIS_OID_REQUEST *OidRequest, void *Params)
{

    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    NDIS_RECEIVE_QUEUE_FREE_PARAMETERS *pParams =
        (NDIS_RECEIVE_QUEUE_FREE_PARAMETERS *)Params;
    LOCK_STATE_EX lock_state;
    struct lif *lif = NULL;

    NdisAcquireRWLockWrite(ionic->vm_queue[pParams->QueueId].QueueLock,
                           &lock_state, 0);

    DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE,
              "%s Flags %08lX Id %d\n", __FUNCTION__, pParams->Flags,
              pParams->QueueId));

    ionic->vm_queue[pParams->QueueId].Flags = 0;

    if (ionic->vm_queue[pParams->QueueId].lif != NULL) {

        lif = ionic->vm_queue[pParams->QueueId].lif;
        ionic->vm_queue[pParams->QueueId].lif = NULL;

        ClearFlag(lif->lif_stats->flags, IONIC_LIF_FLAG_ALLOCATED);

        NdisReleaseRWLock(ionic->vm_queue[pParams->QueueId].QueueLock,
                          &lock_state);

        ionic_lif_stop(lif);
        ionic_lif_deinit(lif);
        ionic_lif_free(lif);

        OidRequest->DATA.SET_INFORMATION.BytesRead =
            sizeof(NDIS_RECEIVE_QUEUE_FREE_PARAMETERS);
    } else {
        status = NDIS_STATUS_NOT_ACCEPTED;
        NdisReleaseRWLock(ionic->vm_queue[pParams->QueueId].QueueLock,
                          &lock_state);
    }

    DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE,
              "%s Complete for queue Id %d Status %08lX\n", __FUNCTION__,
              pParams->QueueId, status));

    return status;
}

NDIS_STATUS
oid_filter_set_filter(struct ionic *ionic, NDIS_OID_REQUEST *OidRequest)
{
    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_RECEIVE_FILTER_PARAMETERS *pFilter =
        (NDIS_RECEIVE_FILTER_PARAMETERS *)
            OidRequest->DATA.METHOD_INFORMATION.InformationBuffer;
    ULONG ulIndex = 0;
    NDIS_RECEIVE_FILTER_FIELD_PARAMETERS *pParams = NULL;
    LOCK_STATE_EX lock_state;

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Adapter %p Flags %08lX QueueId %d FilterId %d VPortId %d "
              "Elements %d\n",
              __FUNCTION__, ionic, pFilter->Flags, pFilter->QueueId,
              pFilter->FilterId, pFilter->VPortId,
              pFilter->FieldParametersArrayNumElements));

    OidRequest->DATA.METHOD_INFORMATION.BytesRead =
        OidRequest->DATA.METHOD_INFORMATION.InputBufferLength;
    OidRequest->DATA.METHOD_INFORMATION.BytesWritten =
        OidRequest->DATA.METHOD_INFORMATION.OutputBufferLength;

    NdisAcquireRWLockWrite(ionic->vm_queue[pFilter->QueueId].QueueLock,
                           &lock_state, 0);

    if (!BooleanFlagOn(ionic->vm_queue[pFilter->QueueId].Flags,
                       IONIC_QUEUE_STATE_INITIALIZED)) {

        NdisReleaseRWLock(ionic->vm_queue[pFilter->QueueId].QueueLock,
                          &lock_state);
        ntStatus = NDIS_STATUS_INVALID_DATA;

        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Adapter %p QueueId %d not initialized\n", __FUNCTION__,
                  ionic, pFilter->QueueId));

        goto cleanup;
    }

    NdisReleaseRWLock(ionic->vm_queue[pFilter->QueueId].QueueLock, &lock_state);

    if (ionic->q_filter[pFilter->FilterId].q_index != 0) {
        ntStatus = NDIS_STATUS_INVALID_DATA;

        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Adapter %p Queue index %d set in filter id %d\n",
                  __FUNCTION__, ionic,
                  ionic->q_filter[pFilter->FilterId].q_index,
                  pFilter->FilterId));

        goto cleanup;
    }

    pParams = (NDIS_RECEIVE_FILTER_FIELD_PARAMETERS
                   *)((char *)pFilter + pFilter->FieldParametersArrayOffset);

    //
    // Perform basic validation
    //

    if (pFilter->FieldParametersArrayNumElements == 2) {

        if (pParams->HeaderField.MacHeaderField ==
                NdisMacHeaderFieldDestinationAddress &&
            ((NDIS_RECEIVE_FILTER_FIELD_PARAMETERS
                  *)((char *)pParams +
                     pFilter->FieldParametersArrayElementSize))
                    ->HeaderField.MacHeaderField == NdisMacHeaderFieldVlanId &&
            (BooleanFlagOn(
                 pParams->Flags,
                 NDIS_RECEIVE_FILTER_FIELD_MAC_HEADER_VLAN_UNTAGGED_OR_ZERO) ||
             BooleanFlagOn(
                 ((NDIS_RECEIVE_FILTER_FIELD_PARAMETERS
                       *)((char *)pParams +
                          pFilter->FieldParametersArrayElementSize))
                     ->Flags,
                 NDIS_RECEIVE_FILTER_FIELD_MAC_HEADER_VLAN_UNTAGGED_OR_ZERO))) {
            ntStatus = NDIS_STATUS_INVALID_DATA;

            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                      "%s Adapter %p Invalid flags set 0x%08lX - 0x%08lX\n",
                      __FUNCTION__, ionic, pParams->Flags,
                      ((NDIS_RECEIVE_FILTER_FIELD_PARAMETERS
                            *)((char *)pParams +
                               pFilter->FieldParametersArrayElementSize))
                          ->Flags));

            goto cleanup;
        }
    }

    while (ulIndex < pFilter->FieldParametersArrayNumElements) {

        ionic->vm_queue[pFilter->QueueId].filter[ulIndex].Flags =
            IONIC_FILTER_SET;

        ionic->vm_queue[pFilter->QueueId].filter[ulIndex].FilterId =
            pFilter->FilterId;

        ionic->vm_queue[pFilter->QueueId].filter[ulIndex].FilterFlags =
            pParams->Flags;

        ionic->vm_queue[pFilter->QueueId].filter[ulIndex].FilterTest =
            pParams->ReceiveFilterTest;

        if (pParams->HeaderField.MacHeaderField ==
            NdisMacHeaderFieldDestinationAddress) {

            if (pFilter->QueueId != 0 &&
                pParams->ReceiveFilterTest == NdisReceiveFilterTestEqual &&
                (memcmp(ionic->config_addr,
                        pParams->FieldValue.FieldByteArrayValue,
                        ETH_ALEN) == 0)) {

                ionic_lif_addr(ionic->master_lif,
                               (const u8 *)&ionic->config_addr[0], false);
                SetFlag(ionic->vm_queue[pFilter->QueueId].filter[ulIndex].Flags,
                        IONIC_FILTER_STATION_ADDR);
            }

            NdisMoveMemory(
                ionic->vm_queue[pFilter->QueueId].filter[ulIndex].MacAddr,
                pParams->FieldValue.FieldByteArrayValue, ETH_ALEN);

            ntStatus =
                ionic_lif_addr(ionic->vm_queue[pFilter->QueueId].lif,
                               (const u8 *)ionic->vm_queue[pFilter->QueueId]
                                   .filter[ulIndex]
                                   .MacAddr,
                               true);
            if (ntStatus != NDIS_STATUS_SUCCESS) {

                DbgTrace(
                    (TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                     "%s ionic_lif_addr() failed for lif %p Status %08lX\n",
                     __FUNCTION__, ionic->vm_queue[pFilter->QueueId].lif,
                     ntStatus));
                break;
            }

            SetFlag(ionic->vm_queue[pFilter->QueueId].filter[ulIndex].Flags,
                    IONIC_FILTER_MAC_FILTER);
        } else if (pParams->HeaderField.MacHeaderField ==
                   NdisMacHeaderFieldVlanId) {

#if 0
			if (pFilter->QueueId != 0 &&
				pParams->ReceiveFilterTest == NdisReceiveFilterTestEqual &&
				(ionic->vlan_id != 0) &&
				((u16)ionic->vlan_id == pParams->FieldValue.FieldShortValue)) {

				ionic_lif_vlan( ionic->master_lif, 
										ionic->vlan_id, 
										false);

				SetFlag( ionic->vm_queue[pFilter->QueueId].filter[ ulIndex].Flags, IONIC_FILTER_STATION_VLAN);
			}
#endif

            ionic->vm_queue[pFilter->QueueId].filter[ulIndex].VLanId =
                pParams->FieldValue.FieldShortValue;

            ntStatus = ionic_lif_vlan(
                ionic->vm_queue[pFilter->QueueId].lif,
                ionic->vm_queue[pFilter->QueueId].filter[ulIndex].VLanId, true);

            if (ntStatus != NDIS_STATUS_SUCCESS) {

                DbgTrace(
                    (TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                     "%s ionic_lif_vlan() failed for lif %p Status %08lX\n",
                     __FUNCTION__, ionic->vm_queue[pFilter->QueueId].lif,
                     ntStatus));
                break;
            }

            ionic->vm_queue[pFilter->QueueId].lif->vlan_id =
                ionic->vm_queue[pFilter->QueueId].filter[ulIndex].VLanId;

            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                      "%s Updated lif %d to vlan %d\n", __FUNCTION__,
                      ionic->vm_queue[pFilter->QueueId].lif->index,
                      ionic->vm_queue[pFilter->QueueId].lif->vlan_id));

            SetFlag(ionic->vm_queue[pFilter->QueueId].filter[ulIndex].Flags,
                    IONIC_FILTER_VLAN_FILTER);
            SetFlag(ionic->vm_queue[pFilter->QueueId].Flags,
                    IONIC_QUEUE_STATE_VLAN_FLTR_SET);
        } else {
            ASSERT(FALSE);
        }

        ulIndex++;
        pParams =
            (NDIS_RECEIVE_FILTER_FIELD_PARAMETERS
                 *)((char *)pParams + pFilter->FieldParametersArrayElementSize);

        ionic->vm_queue[pFilter->QueueId].active_filter_cnt++;
    }

    if (ntStatus == NDIS_STATUS_SUCCESS) {
        ionic->q_filter[pFilter->FilterId].q_index = pFilter->QueueId;
    }

cleanup:

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Complete Adapter %p QueueId %d Status %08lX\n", __FUNCTION__,
              ionic, pFilter->QueueId, ntStatus));
    return ntStatus;
}

NDIS_STATUS
oid_filter_queue_alloc_complete(struct ionic *ionic,
                                NDIS_OID_REQUEST *OidRequest)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_RECEIVE_QUEUE_ALLOCATION_COMPLETE_ARRAY *pParams =
        (NDIS_RECEIVE_QUEUE_ALLOCATION_COMPLETE_ARRAY *)
            OidRequest->DATA.METHOD_INFORMATION.InformationBuffer;
    NDIS_RECEIVE_QUEUE_ALLOCATION_COMPLETE_PARAMETERS *pEntry = NULL;
    ULONG ulIndex = 0;

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE, "%s Count %d\n",
              __FUNCTION__, pParams->NumElements));

    OidRequest->DATA.METHOD_INFORMATION.BytesRead =
        OidRequest->DATA.METHOD_INFORMATION.InputBufferLength;
    OidRequest->DATA.METHOD_INFORMATION.BytesWritten =
        OidRequest->DATA.METHOD_INFORMATION.OutputBufferLength;

    pEntry = (NDIS_RECEIVE_QUEUE_ALLOCATION_COMPLETE_PARAMETERS
                  *)((char *)pParams + pParams->FirstElementOffset);

    for (ulIndex = 0; ulIndex < pParams->NumElements; ulIndex++) {

        if (!BooleanFlagOn(ionic->vm_queue[pEntry->QueueId].Flags,
                           IONIC_QUEUE_STATE_INITIALIZED)) {

            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                      "%s QueueId %d not initialized\n", __FUNCTION__,
                      pEntry->QueueId));
            pEntry->CompletionStatus = NDIS_STATUS_INVALID_DATA;
        } else {

            ntStatus = _ionic_lif_rx_mode(ionic->vm_queue[pEntry->QueueId].lif,
                                          ionic->master_lif->rx_mode);

            if (ntStatus != NDIS_STATUS_SUCCESS) {
                pEntry->CompletionStatus = ntStatus;
                goto cleanup;
            }

            if (RtlCheckBit(&ionic->master_lif->state, LIF_UP) == 0) {
                ntStatus = NDIS_STATUS_DEVICE_FAILED;
                pEntry->CompletionStatus = ntStatus;
                goto cleanup;
            }

            ntStatus = ionic_lif_open(
                ionic->vm_queue[pEntry->QueueId].lif, (ULONG)-1,
                pEntry->QueueId);

            if (ntStatus != NDIS_STATUS_SUCCESS) {
                DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                            "%s ionic_lif_open() failed for lif %p "
                            "Status %08lX\n",
                            __FUNCTION__,
                            ionic->vm_queue[pEntry->QueueId].lif,
                            ntStatus));
                pEntry->CompletionStatus = ntStatus;
                goto cleanup;
            }

            ntStatus = ionic_lif_start(
                ionic->vm_queue[pEntry->QueueId].lif);

            if (ntStatus != NDIS_STATUS_SUCCESS) {
                DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                            "%s ionic_lif_start() failed for lif %p "
                            "Status %08lX\n",
                            __FUNCTION__,
                            ionic->vm_queue[pEntry->QueueId].lif,
                            ntStatus));
                pEntry->CompletionStatus = ntStatus;
                goto cleanup;
            }

            pEntry->CompletionStatus = ntStatus;

            SetFlag(ionic->vm_queue[pEntry->QueueId].Flags,
                    IONIC_QUEUE_STATE_ALLOCATED);
        }

        pEntry = (NDIS_RECEIVE_QUEUE_ALLOCATION_COMPLETE_PARAMETERS
                      *)((char *)pEntry + pParams->ElementSize);
    }

cleanup:

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Complete status %08lX\n", __FUNCTION__, ntStatus));

    return ntStatus;
}

NDIS_STATUS
oid_filter_clear_filter(struct ionic *ionic, void *info_buffer)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_RECEIVE_FILTER_CLEAR_PARAMETERS *pFilter =
        (NDIS_RECEIVE_FILTER_CLEAR_PARAMETERS *)info_buffer;
    LOCK_STATE_EX lock_state;
    ULONG ulIndex = 0;
    ULONG queue_id = 0;

    queue_id = ionic->q_filter[pFilter->FilterId].q_index;

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s (OID_RECEIVE_FILTER_CLEAR_FILTER) Flags %08lX QueueId %d "
              "FilterId %d\n",
              __FUNCTION__, pFilter->Flags, queue_id, pFilter->FilterId));

    NdisAcquireRWLockWrite(ionic->vm_queue[queue_id].QueueLock, &lock_state, 0);

    if (!BooleanFlagOn(ionic->vm_queue[queue_id].Flags,
                       IONIC_QUEUE_STATE_INITIALIZED)) {

        NdisReleaseRWLock(ionic->vm_queue[queue_id].QueueLock, &lock_state);

        ntStatus = NDIS_STATUS_INVALID_DATA;
        goto cleanup;
    }

    NdisReleaseRWLock(ionic->vm_queue[queue_id].QueueLock, &lock_state);

    ionic->q_filter[pFilter->FilterId].q_index = 0;

    for (ulIndex = 0; ulIndex < IONIC_RECEIVE_FILTER_COUNT; ulIndex++) {

        if (ionic->vm_queue[queue_id].filter[ulIndex].FilterId ==
            pFilter->FilterId) {

            if (BooleanFlagOn(ionic->vm_queue[queue_id].filter[ulIndex].Flags,
                              IONIC_FILTER_SET)) {

                if (BooleanFlagOn(
                        ionic->vm_queue[queue_id].filter[ulIndex].Flags,
                        IONIC_FILTER_MAC_FILTER)) {
                    ionic_lif_addr(ionic->vm_queue[queue_id].lif,
                                   (const u8 *)ionic->vm_queue[queue_id]
                                       .filter[ulIndex]
                                       .MacAddr,
                                   false);
                    ClearFlag(ionic->vm_queue[queue_id].filter[ulIndex].Flags,
                              IONIC_FILTER_MAC_FILTER);

                    if (BooleanFlagOn(
                            ionic->vm_queue[queue_id].filter[ulIndex].Flags,
                            IONIC_FILTER_STATION_ADDR)) {
                        ionic_lif_addr(ionic->master_lif,
                                       (const u8 *)&ionic->config_addr[0],
                                       true);
                        ClearFlag(
                            ionic->vm_queue[queue_id].filter[ulIndex].Flags,
                            IONIC_FILTER_STATION_ADDR);
                    }
                } else if (BooleanFlagOn(
                               ionic->vm_queue[queue_id].filter[ulIndex].Flags,
                               IONIC_FILTER_VLAN_FILTER)) {
                    ionic_lif_vlan(
                        ionic->vm_queue[queue_id].lif,
                        ionic->vm_queue[queue_id].filter[ulIndex].VLanId,
                        false);
                    ClearFlag(ionic->vm_queue[queue_id].filter[ulIndex].Flags,
                              IONIC_FILTER_VLAN_FILTER);
                    ClearFlag(ionic->vm_queue[pFilter->QueueId].Flags,
                              IONIC_QUEUE_STATE_VLAN_FLTR_SET);

#if 0
					if( BooleanFlagOn( ionic->vm_queue[queue_id].filter[ ulIndex].Flags, IONIC_FILTER_STATION_VLAN)) {
						ionic_lif_vlan( ionic->master_lif, 
										ionic->vlan_id, 
										true);
			
						ClearFlag( ionic->vm_queue[queue_id].filter[ ulIndex].Flags, IONIC_FILTER_STATION_VLAN);
					}
#endif
                }

                ionic->vm_queue[queue_id].filter[ulIndex].Flags = 0;
                ionic->vm_queue[queue_id].active_filter_cnt--;
            }
        }
    }

cleanup:

    return ntStatus;
}

NDIS_STATUS
oid_method_receive_filter_params(struct ionic *ionic,
                                 NDIS_OID_REQUEST *OidRequest)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_RECEIVE_QUEUE_PARAMETERS *pParams =
        (NDIS_RECEIVE_QUEUE_PARAMETERS *)
            OidRequest->DATA.METHOD_INFORMATION.InformationBuffer;
    LOCK_STATE_EX lock_state;

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s QueueId 0x%08lX Flags 0x%08lX\n", __FUNCTION__,
              pParams->QueueId, pParams->Flags));

    OidRequest->DATA.METHOD_INFORMATION.BytesRead =
        OidRequest->DATA.METHOD_INFORMATION.InputBufferLength;
    OidRequest->DATA.METHOD_INFORMATION.BytesWritten =
        OidRequest->DATA.METHOD_INFORMATION.OutputBufferLength;

    NdisAcquireRWLockWrite(ionic->vm_queue[pParams->QueueId].QueueLock,
                           &lock_state, 0);

    if (!BooleanFlagOn(ionic->vm_queue[pParams->QueueId].Flags,
                       IONIC_QUEUE_STATE_INITIALIZED)) {

        NdisReleaseRWLock(ionic->vm_queue[pParams->QueueId].QueueLock,
                          &lock_state);

        ntStatus = NDIS_STATUS_INVALID_DATA;
        goto cleanup;
    }

    NdisReleaseRWLock(ionic->vm_queue[pParams->QueueId].QueueLock, &lock_state);

    if (BooleanFlagOn(
            pParams->Flags,
            NDIS_RECEIVE_QUEUE_PARAMETERS_PROCESSOR_AFFINITY_CHANGED)) {

        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                  "%s QueueId %d Affinity changed from Grp %08lX Proc %08lX to "
                  "Grp %08lX Proc %08lX\n",
                  __FUNCTION__, pParams->QueueId,
                  ionic->vm_queue[pParams->QueueId].ProcessorAffinity.Group,
                  ionic->vm_queue[pParams->QueueId].ProcessorAffinity.Mask,
                  pParams->ProcessorAffinity.Group,
                  pParams->ProcessorAffinity.Mask));

        ionic->vm_queue[pParams->QueueId].ProcessorAffinity =
            pParams->ProcessorAffinity;
    }

    if (BooleanFlagOn(
            pParams->Flags,
            NDIS_RECEIVE_QUEUE_PARAMETERS_SUGGESTED_RECV_BUFFER_NUMBERS_CHANGED)) {

        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                  "%s QueueId %d Recv cnt from %d to %d\n", __FUNCTION__,
                  pParams->QueueId,
                  ionic->vm_queue[pParams->QueueId].ReceiveBuffers,
                  pParams->NumSuggestedReceiveBuffers));

        ionic->vm_queue[pParams->QueueId].ReceiveBuffers =
            pParams->NumSuggestedReceiveBuffers;
    }

cleanup:

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Complete status %08lX\n", __FUNCTION__, ntStatus));

    return ntStatus;
}

NDIS_STATUS
oid_set_receive_filter_params(struct ionic *ionic,
                              void *info_buffer,
                              ULONG info_buffer_len,
                              ULONG *bytes_read)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_RECEIVE_QUEUE_PARAMETERS *pParams =
        (NDIS_RECEIVE_QUEUE_PARAMETERS *)info_buffer;
    LOCK_STATE_EX lock_state;

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s QueueId 0x%08lX Flags 0x%08lX\n", __FUNCTION__,
              pParams->QueueId, pParams->Flags));

    *bytes_read = info_buffer_len;

    NdisAcquireRWLockWrite(ionic->vm_queue[pParams->QueueId].QueueLock,
                           &lock_state, 0);

    if (!BooleanFlagOn(ionic->vm_queue[pParams->QueueId].Flags,
                       IONIC_QUEUE_STATE_INITIALIZED)) {

        NdisReleaseRWLock(ionic->vm_queue[pParams->QueueId].QueueLock,
                          &lock_state);

        ntStatus = NDIS_STATUS_INVALID_DATA;
        goto cleanup;
    }

    NdisReleaseRWLock(ionic->vm_queue[pParams->QueueId].QueueLock, &lock_state);

    if (BooleanFlagOn(
            pParams->Flags,
            NDIS_RECEIVE_QUEUE_PARAMETERS_SUGGESTED_RECV_BUFFER_NUMBERS_CHANGED)) {

        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                  "%s Update recv queue cnt from %d to %d\n", __FUNCTION__,
                  ionic->vm_queue[pParams->QueueId].ReceiveBuffers,
                  pParams->NumSuggestedReceiveBuffers));
    }

    if (BooleanFlagOn(
            pParams->Flags,
            NDIS_RECEIVE_QUEUE_PARAMETERS_PROCESSOR_AFFINITY_CHANGED)) {

        DbgTrace(
            (TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
             "%s Update processor affinity from group %d proc %d to group %d "
             "proc %d\n",
             __FUNCTION__,
             ionic->vm_queue[pParams->QueueId].ProcessorAffinity.Group,
             GetNextProcIndex(
                 ionic->vm_queue[pParams->QueueId].ProcessorAffinity.Mask, 0),
             pParams->ProcessorAffinity.Group,
             GetNextProcIndex(pParams->ProcessorAffinity.Mask, 0)));
    }

cleanup:

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Complete status %08lX\n", __FUNCTION__, ntStatus));

    return ntStatus;
}

BOOLEAN
synchronize_affinity_change(NDIS_HANDLE SynchronizeContext)
{
    UNREFERENCED_PARAMETER(SynchronizeContext);
    return TRUE;
}