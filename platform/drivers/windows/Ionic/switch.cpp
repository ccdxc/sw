
#include "common.h"

char *
GetVPortStateStr(u32 State)
{

    char *vPortState = "NdisNicSwitchVPortStateUndefined";

    switch (State) {
    case NdisNicSwitchVPortStateActivated: {
        vPortState = "NdisNicSwitchVPortStateActivated";
        break;
    }

    case NdisNicSwitchVPortStateDeactivated: {
        vPortState = "NdisNicSwitchVPortStateDeactivated";
        break;
    }
    }

    return vPortState;
}

NDIS_STATUS
oid_nic_switch_create_switch(struct ionic *ionic, NDIS_OID_REQUEST *OidRequest)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_NIC_SWITCH_PARAMETERS *pParameters =
        (NDIS_NIC_SWITCH_PARAMETERS *)
            OidRequest->DATA.METHOD_INFORMATION.InformationBuffer;
    ULONG ulIndex = 0;

    if (!BooleanFlagOn(ionic->ConfigStatus, IONIC_SRIOV_CAPABLE)) {
        ntStatus = NDIS_STATUS_NOT_SUPPORTED;
        goto cleanup;
    }

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s For %p Flags %08lX SwitchType %s Id %d NumVFs %d\n",
              __FUNCTION__, ionic, pParameters->Flags,
              GetSwitchTypeString(pParameters->SwitchType),
              pParameters->SwitchId, pParameters->NumVFs));

    OidRequest->DATA.METHOD_INFORMATION.BytesRead =
        OidRequest->DATA.METHOD_INFORMATION.InputBufferLength;
    OidRequest->DATA.METHOD_INFORMATION.BytesWritten =
        OidRequest->DATA.METHOD_INFORMATION.OutputBufferLength;

    ntStatus = NdisMEnableVirtualization(
        ionic->adapterhandle, ionic->sriov_caps.NumVFs, FALSE, FALSE, TRUE);

    if (ntStatus != NDIS_STATUS_SUCCESS) {
        DbgTrace(
            (TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
             "%s Failed to call NdisMEnableVirtualization() Status %08lX\n",
             __FUNCTION__, ntStatus));

        goto cleanup;
    }

    //
    // Read the probed BAR values
    //

    ntStatus = NdisMQueryProbedBars(ionic->adapterhandle, ionic->ProbedBARS);

    if (ntStatus != NDIS_STATUS_SUCCESS) {
    }

    //
    // Initialize the vSwitch control
    //

    ionic->SriovSwitch.Ports =
        (struct vport_info *)NdisAllocateMemoryWithTagPriority_internal(
            ionic->adapterhandle,
            sizeof(struct vport_info) * ionic->ident.dev.nlifs,
            IONIC_GENERIC_TAG, NormalPoolPriority);

    if (ionic->SriovSwitch.Ports == NULL) {
        ntStatus = NDIS_STATUS_RESOURCES;
        goto cleanup;
    }

    NdisZeroMemory(ionic->SriovSwitch.Ports,
                   sizeof(struct vport_info) * ionic->ident.dev.nlifs);

    DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                  "%s Allocated 0x%p len %08lX\n",
                  __FUNCTION__,
                  ionic->SriovSwitch.Ports,
                  sizeof(struct vport_info) * ionic->ident.dev.nlifs));
    
    //
    // Initialize the port lock for all the ports in the array
    //

    for (ulIndex = 0; ulIndex < ionic->ident.dev.nlifs; ulIndex++) {
        ionic->SriovSwitch.Ports[ulIndex].PortLock =
            NdisAllocateRWLock(ionic->adapterhandle);
    }

    ionic->SriovSwitch.vfs =
        (struct vf_info *)NdisAllocateMemoryWithTagPriority_internal(
            ionic->adapterhandle,
            sizeof(struct vf_info) * ionic->sriov_caps.NumVFs,
            IONIC_GENERIC_TAG, NormalPoolPriority);

    if (ionic->SriovSwitch.vfs == NULL) {
        ntStatus = NDIS_STATUS_RESOURCES;
        goto cleanup;
    }

    NdisZeroMemory(ionic->SriovSwitch.vfs,
                   sizeof(struct vf_info) * ionic->sriov_caps.NumVFs);

    DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                  "%s Allocated 0x%p len %08lX\n",
                  __FUNCTION__,
                  ionic->SriovSwitch.vfs,
                  sizeof(struct vf_info) * ionic->sriov_caps.NumVFs));
    
    //
    // Initialize the port lock for all the ports in the array
    //

    ASSERT(ionic->num_bars == 4);

    for (ulIndex = 0; ulIndex < ionic->sriov_caps.NumVFs; ulIndex++) {
        ionic->SriovSwitch.vfs[ulIndex].Flags = IONIC_VF_INITIALIZED;
        ionic->SriovSwitch.vfs[ulIndex].vport_id = NDIS_PF_FUNCTION_ID;

        ionic->SriovSwitch.vfs[ulIndex].vf_id = (USHORT)ulIndex;
        ionic->SriovSwitch.vfs[ulIndex].rid = get_rid(ionic, (USHORT)ulIndex);

        //
        // Init the bar for this VF
        //

        ionic->SriovSwitch.vfs[ulIndex].BAR[0].bus_addr =
            (ionic->bars[2].bus_addr + (ulIndex * BAR0_SIZE));
        ionic->SriovSwitch.vfs[ulIndex].BAR[0].Flags = ionic->bars[2].Flags;
        ionic->SriovSwitch.vfs[ulIndex].BAR[0].len = BAR0_SIZE;
        ionic->SriovSwitch.vfs[ulIndex].BAR[0].ShareDisposition =
            ionic->bars[2].ShareDisposition;
        ionic->SriovSwitch.vfs[ulIndex].BAR[0].Type = ionic->bars[2].Type;
        ionic->SriovSwitch.vfs[ulIndex].BAR[0].vaddr =
            (void *)((char *)ionic->bars[2].vaddr + (ulIndex * BAR0_SIZE));

        ionic->SriovSwitch.vfs[ulIndex].BAR[1].bus_addr =
            (ionic->bars[3].bus_addr + (ulIndex * BAR1_SIZE));
        ionic->SriovSwitch.vfs[ulIndex].BAR[1].Flags = ionic->bars[3].Flags;
        ionic->SriovSwitch.vfs[ulIndex].BAR[1].len = BAR1_SIZE;
        ionic->SriovSwitch.vfs[ulIndex].BAR[1].ShareDisposition =
            ionic->bars[3].ShareDisposition;
        ionic->SriovSwitch.vfs[ulIndex].BAR[1].Type = ionic->bars[3].Type;
        ionic->SriovSwitch.vfs[ulIndex].BAR[1].vaddr =
            (void *)((char *)ionic->bars[3].vaddr + (ulIndex * BAR1_SIZE));
    }

    //
    // Setup the default vport for the switch
    //

#if (NDIS_SUPPORT_NDIS660)
    ionic->SriovSwitch.Ports[0].QueuePairs =
        pParameters->NumQueuePairsForDefaultVPort;
#endif

    ionic->SriovSwitch.Ports[0].lif = ionic->master_lif;
    ionic->SriovSwitch.Ports[0].State = NdisNicSwitchVPortStateActivated;

    ionic->SriovSwitch.Id = pParameters->SwitchId;
    ionic->SriovSwitch.Type = pParameters->SwitchType;
    ionic->SriovSwitch.NumberVFs = pParameters->NumVFs;

    ionic->SriovSwitch.FriendlyName.Length =
        pParameters->SwitchFriendlyName.Length;
    ionic->SriovSwitch.FriendlyName.MaximumLength =
        ionic->SriovSwitch.FriendlyName.Length + sizeof(WCHAR);

    if (ionic->SriovSwitch.FriendlyName.Length != 0) {
        ionic->SriovSwitch.FriendlyName.Buffer =
            (WCHAR *)NdisAllocateMemoryWithTagPriority_internal(
                ionic->adapterhandle,
                ionic->SriovSwitch.FriendlyName.MaximumLength,
                IONIC_GENERIC_TAG, NormalPoolPriority);

        if (ionic->SriovSwitch.FriendlyName.Buffer != NULL) {

            NdisZeroMemory(ionic->SriovSwitch.FriendlyName.Buffer,
                           ionic->SriovSwitch.FriendlyName.MaximumLength);

            DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                  "%s Allocated 0x%p len %08lX\n",
                  __FUNCTION__,
                  ionic->SriovSwitch.FriendlyName.Buffer,
                  ionic->SriovSwitch.FriendlyName.MaximumLength));
            
            NdisMoveMemory(ionic->SriovSwitch.FriendlyName.Buffer,
                           pParameters->SwitchFriendlyName.String,
                           ionic->SriovSwitch.FriendlyName.Length);
        }
    }

    SetFlag(ionic->SriovSwitch.Ports[0].Flags,
            IONIC_VPORT_STATE_INITIALIZED | IONIC_VPORT_STATE_ACTIVATED);

    //
    // Indicate we are running in srion mode
    //

    SetFlag(ionic->ConfigStatus, IONIC_SRIOV_MODE);
    SetFlag(ionic->port_stats.flags, IONIC_PORT_FLAG_SRIOV);

cleanup:

    return ntStatus;
}

NDIS_STATUS
oid_nic_switch_delete_switch(struct ionic *ionic,
                             void *data_buffer,
                             ULONG data_buffer_length,
                             ULONG *bytes_needed)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_NIC_SWITCH_DELETE_SWITCH_PARAMETERS *pParameters =
        (NDIS_NIC_SWITCH_DELETE_SWITCH_PARAMETERS *)data_buffer;
    ULONG ulIndex = 0;

    if (data_buffer_length < sizeof(NDIS_NIC_SWITCH_DELETE_SWITCH_PARAMETERS)) {
        ntStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
        *bytes_needed = sizeof(NDIS_NIC_SWITCH_DELETE_SWITCH_PARAMETERS);
        goto cleanup;
    }

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE, "%s For %p Id %d\n",
              __FUNCTION__, ionic, pParameters->SwitchId));

    if (ionic->SriovSwitch.Id != pParameters->SwitchId) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Invalid switch Id %d\n", __FUNCTION__,
                  pParameters->SwitchId));
        ntStatus = NDIS_STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

    ntStatus = NdisMEnableVirtualization(ionic->adapterhandle, (USHORT)0, FALSE,
                                         FALSE, FALSE);

    if (ntStatus != NDIS_STATUS_SUCCESS) {
        DbgTrace(
            (TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
             "%s Failed to call NdisMEnableVirtualization() Status %08lX\n",
             __FUNCTION__, ntStatus));

        ntStatus = NDIS_STATUS_SUCCESS;
    }

    if (ionic->SriovSwitch.FriendlyName.Buffer != NULL) {
        NdisFreeMemoryWithTagPriority_internal(ionic->adapterhandle,
                                      ionic->SriovSwitch.FriendlyName.Buffer,
                                      IONIC_GENERIC_TAG);
        ionic->SriovSwitch.FriendlyName.Buffer = NULL;
    }

    if (ionic->SriovSwitch.Ports != NULL) {

        for (ulIndex = 0; ulIndex < ionic->ident.dev.nlifs; ulIndex++) {
            if (ionic->SriovSwitch.Ports[ulIndex].PortLock != NULL) {
                NdisFreeRWLock(ionic->SriovSwitch.Ports[ulIndex].PortLock);
            }
        }

        NdisFreeMemoryWithTagPriority_internal(
            ionic->adapterhandle, ionic->SriovSwitch.Ports, IONIC_GENERIC_TAG);
        ionic->SriovSwitch.Ports = NULL;
    }

    if (ionic->SriovSwitch.vfs != NULL) {
        NdisFreeMemoryWithTagPriority_internal(
            ionic->adapterhandle, ionic->SriovSwitch.vfs, IONIC_GENERIC_TAG);
        ionic->SriovSwitch.vfs = NULL;
    }

    ionic->SriovSwitch.Id = (ULONG)-1;
    ionic->SriovSwitch.Type = 0;
    ionic->SriovSwitch.NumberVFs = 0;

    ClearFlag(ionic->ConfigStatus, IONIC_SRIOV_MODE);

cleanup:

    return ntStatus;
}

NDIS_STATUS
oid_nic_switch_create(struct ionic *ionic,
                      void *data_buffer,
                      ULONG data_buffer_length,
                      ULONG *bytes_needed)
{
    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_SWITCH_NIC_PARAMETERS *pParameters =
        (NDIS_SWITCH_NIC_PARAMETERS *)data_buffer;

    UNREFERENCED_PARAMETER(ionic);

    if (pParameters == NULL) {
        goto cleanup;
    }

    if (data_buffer_length < sizeof(NDIS_SWITCH_NIC_PARAMETERS)) {
        ntStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
        *bytes_needed = sizeof(NDIS_SWITCH_NIC_PARAMETERS);
        goto cleanup;
    }

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s For %p Flags %08lX MTU %08lX Port %d NicIndex %d State %s "
              "Type %s VFAssigned %s\n",
              __FUNCTION__, ionic, pParameters->Flags, pParameters->MTU,
              pParameters->PortId, pParameters->NicIndex,
              GetNicStateString(pParameters->NicState),
              GetNicTypeString(pParameters->NicType),
              pParameters->VFAssigned ? "Yes" : "No"));

cleanup:

    return ntStatus;
}

NDIS_STATUS
oid_nic_switch_delete(struct ionic *ionic,
                      void *data_buffer,
                      ULONG data_buffer_length,
                      ULONG *bytes_needed)
{
    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_SWITCH_NIC_PARAMETERS *pParameters =
        (NDIS_SWITCH_NIC_PARAMETERS *)data_buffer;

    UNREFERENCED_PARAMETER(ionic);

    if (pParameters == NULL) {
        goto cleanup;
    }

    if (data_buffer_length < sizeof(NDIS_SWITCH_NIC_PARAMETERS)) {
        ntStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
        *bytes_needed = sizeof(NDIS_SWITCH_NIC_PARAMETERS);
        goto cleanup;
    }

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s For %p Flags %08lX MTU %08lX Port %d NicIndex %d State %s "
              "Type %s VFAssigned %s\n",
              __FUNCTION__, ionic, pParameters->Flags, pParameters->MTU,
              pParameters->PortId, pParameters->NicIndex,
              GetNicStateString(pParameters->NicState),
              GetNicTypeString(pParameters->NicType),
              pParameters->VFAssigned ? "Yes" : "No"));

cleanup:

    return ntStatus;
}
NDIS_STATUS
oid_nic_switch_connect(struct ionic *ionic,
                       void *data_buffer,
                       ULONG data_buffer_length,
                       ULONG *bytes_needed)
{
    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_SWITCH_NIC_PARAMETERS *pParameters =
        (NDIS_SWITCH_NIC_PARAMETERS *)data_buffer;

    UNREFERENCED_PARAMETER(ionic);

    if (pParameters == NULL) {
        goto cleanup;
    }

    if (data_buffer_length < sizeof(NDIS_SWITCH_NIC_PARAMETERS)) {
        ntStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
        *bytes_needed = sizeof(NDIS_SWITCH_NIC_PARAMETERS);
        goto cleanup;
    }

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s For %p Flags %08lX MTU %08lX Port %d NicIndex %d State %s "
              "Type %s VFAssigned %s\n",
              __FUNCTION__, ionic, pParameters->Flags, pParameters->MTU,
              pParameters->PortId, pParameters->NicIndex,
              GetNicStateString(pParameters->NicState),
              GetNicTypeString(pParameters->NicType),
              pParameters->VFAssigned ? "Yes" : "No"));

cleanup:

    return ntStatus;
}

NDIS_STATUS
oid_nic_switch_updated(struct ionic *ionic,
                       void *data_buffer,
                       ULONG data_buffer_length,
                       ULONG *bytes_needed)
{
    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_SWITCH_NIC_PARAMETERS *pParameters =
        (NDIS_SWITCH_NIC_PARAMETERS *)data_buffer;

    UNREFERENCED_PARAMETER(ionic);

    if (pParameters == NULL) {
        goto cleanup;
    }

    if (data_buffer_length < sizeof(NDIS_SWITCH_NIC_PARAMETERS)) {
        ntStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
        *bytes_needed = sizeof(NDIS_SWITCH_NIC_PARAMETERS);
        goto cleanup;
    }

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s For %p Flags %08lX MTU %08lX Port %d NicIndex %d State %s "
              "Type %s VFAssigned %s\n",
              __FUNCTION__, ionic, pParameters->Flags, pParameters->MTU,
              pParameters->PortId, pParameters->NicIndex,
              GetNicStateString(pParameters->NicState),
              GetNicTypeString(pParameters->NicType),
              pParameters->VFAssigned ? "Yes" : "No"));

cleanup:

    return ntStatus;
}

NDIS_STATUS
oid_filter_set_filter_vswitch(struct ionic *ionic, NDIS_OID_REQUEST *OidRequest)
{
    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_RECEIVE_FILTER_PARAMETERS *pFilter =
        (NDIS_RECEIVE_FILTER_PARAMETERS *)
            OidRequest->DATA.METHOD_INFORMATION.InformationBuffer;
    ULONG ulIndex = 0;
    NDIS_RECEIVE_FILTER_FIELD_PARAMETERS *pParams = NULL;
    LOCK_STATE_EX lock_state;
    struct vport_info *vPort = NULL;

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

    vPort = &ionic->SriovSwitch.Ports[pFilter->VPortId];

    NdisAcquireRWLockWrite(vPort->PortLock, &lock_state, 0);

    if (!BooleanFlagOn(vPort->Flags, IONIC_VPORT_STATE_INITIALIZED)) {

        NdisReleaseRWLock(vPort->PortLock, &lock_state);

        ntStatus = NDIS_STATUS_INVALID_DATA;
        goto cleanup;
    }

    NdisReleaseRWLock(vPort->PortLock, &lock_state);

    if (ionic->q_filter[pFilter->FilterId].q_index != 0) {
        ntStatus = NDIS_STATUS_INVALID_DATA;
        goto cleanup;
    }

    ionic->q_filter[pFilter->FilterId].q_index = pFilter->VPortId;

    pParams = (NDIS_RECEIVE_FILTER_FIELD_PARAMETERS
                   *)((char *)pFilter + pFilter->FieldParametersArrayOffset);

    for (ulIndex = 0; ulIndex < pFilter->FieldParametersArrayNumElements;
         ulIndex++) {

        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                  "\tFilter Flags %08lX\n", pParams->Flags));

        vPort->filter[ulIndex].Flags = IONIC_FILTER_SET;

        vPort->filter[ulIndex].FilterId = pFilter->FilterId;

        vPort->filter[ulIndex].FilterFlags = pParams->Flags;

        vPort->filter[ulIndex].FilterTest = pParams->ReceiveFilterTest;

        if (pParams->HeaderField.MacHeaderField ==
            NdisMacHeaderFieldDestinationAddress) {

            if (pFilter->QueueId != 0 &&
                pParams->ReceiveFilterTest == NdisReceiveFilterTestEqual &&
                (memcmp(ionic->config_addr,
                        pParams->FieldValue.FieldByteArrayValue,
                        ETH_ALEN) == 0)) {

                ionic_lif_addr(ionic->master_lif,
                               (const u8 *)&ionic->config_addr[0], false);
                SetFlag(vPort->filter[ulIndex].Flags,
                        IONIC_FILTER_STATION_ADDR);
            }

            NdisMoveMemory(vPort->filter[ulIndex].MacAddr,
                           pParams->FieldValue.FieldByteArrayValue, ETH_ALEN);

            ntStatus = ionic_lif_addr(
                vPort->lif, (const u8 *)vPort->filter[ulIndex].MacAddr, true);
            if (ntStatus != NDIS_STATUS_SUCCESS) {

                DbgTrace(
                    (TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                     "%s ionic_lif_addr() failed for lif %p Status %08lX\n",
                     __FUNCTION__, vPort->lif, ntStatus));
                break;
            }

            SetFlag(vPort->filter[ulIndex].Flags, IONIC_FILTER_MAC_FILTER);
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

				SetFlag( vPort->filter[ulIndex].Flags, IONIC_FILTER_STATION_VLAN);
			}
#endif

            vPort->filter[ulIndex].VLanId = pParams->FieldValue.FieldShortValue;

            ntStatus =
                ionic_lif_vlan(vPort->lif, vPort->filter[ulIndex].VLanId, true);

            if (ntStatus != NDIS_STATUS_SUCCESS) {

                DbgTrace(
                    (TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                     "%s ionic_lif_vlan() failed for lif %p Status %08lX\n",
                     __FUNCTION__, vPort->lif, ntStatus));
                break;
            }

            vPort->lif->vlan_id = vPort->filter[ulIndex].VLanId;

            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                      "%s Updated lif %d to vlan %d\n", __FUNCTION__,
                      vPort->lif->index, vPort->lif->vlan_id));

            SetFlag(vPort->filter[ulIndex].Flags, IONIC_FILTER_VLAN_FILTER);
            SetFlag(vPort->Flags, IONIC_VPORT_STATE_VLAN_FLTR_SET);
        } else {
            ASSERT(FALSE);
        }

        vPort->filter_cnt++;
        pParams =
            (NDIS_RECEIVE_FILTER_FIELD_PARAMETERS
                 *)((char *)pParams + pFilter->FieldParametersArrayElementSize);
    }

cleanup:

    return ntStatus;
}

NDIS_STATUS
oid_switch_create_vport(struct ionic *ionic, NDIS_OID_REQUEST *OidRequest)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_NIC_SWITCH_VPORT_PARAMETERS *vPortParams =
        (NDIS_NIC_SWITCH_VPORT_PARAMETERS *)
            OidRequest->DATA.METHOD_INFORMATION.InformationBuffer;
    LOCK_STATE_EX lock_state;
    BOOLEAN bReleaseLock = FALSE;
    struct vport_info *pVPort = NULL;
    struct vf_info *vf_info = NULL;

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Adapter %p Flags %08lX SwitchId %d VPortId %d FuncId 0x%08lX "
              "QueuePairs %d State %s\n",
              __FUNCTION__, ionic, vPortParams->Flags, vPortParams->SwitchId,
              vPortParams->VPortId, vPortParams->AttachedFunctionId,
              vPortParams->NumQueuePairs,
              GetVPortStateStr(vPortParams->VPortState)));

    OidRequest->DATA.METHOD_INFORMATION.BytesRead =
        OidRequest->DATA.METHOD_INFORMATION.InputBufferLength;
    OidRequest->DATA.METHOD_INFORMATION.BytesWritten =
        OidRequest->DATA.METHOD_INFORMATION.OutputBufferLength;

    //
    // Some validation
    //
    if (vPortParams->VPortId == NDIS_DEFAULT_RECEIVE_QUEUE_ID ||
        vPortParams->SwitchId != 0) {
        ASSERT(FALSE);
        ntStatus = NDIS_STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

    pVPort = &ionic->SriovSwitch.Ports[vPortParams->VPortId];

    NdisAcquireRWLockWrite(pVPort->PortLock, &lock_state, 0);
    bReleaseLock = TRUE;

    if (!BooleanFlagOn(pVPort->Flags, IONIC_VPORT_STATE_INITIALIZED)) {

        //
        // Drop the lock while allocating buffers, etc. At DISPATCH
        //

        NdisReleaseRWLock(pVPort->PortLock, &lock_state);
        bReleaseLock = FALSE;

        pVPort->AttachedFunctionId = vPortParams->AttachedFunctionId;

        if (pVPort->AttachedFunctionId != NDIS_PF_FUNCTION_ID) {

            //
            // Lookup the vf that is being attached to the vport
            //

            vf_info = locate_vfinfo(ionic, vPortParams->AttachedFunctionId);

            if (vf_info == NULL ||
                !BooleanFlagOn(vf_info->Flags, IONIC_VF_ALLOCATED) ||
                BooleanFlagOn(vf_info->Flags, IONIC_VF_CONNECTED)) {
                ASSERT(FALSE);
                ntStatus = NDIS_STATUS_NOT_SUPPORTED;
                goto cleanup;
            }

            vf_info->vport_id = vPortParams->VPortId;
            SetFlag(vf_info->Flags, IONIC_VF_CONNECTED);
        }

        ASSERT(pVPort->lif == NULL);

        pVPort->lif = (struct lif *)ionic_allocate_slave_lif(ionic);

        if (pVPort->lif == NULL) {
            ntStatus = NDIS_STATUS_RESOURCES;
            goto cleanup;
        }

        pVPort->lif->lif_stats->lif_type = IONIC_LIF_TYPE_VPORT;

        SetFlag(pVPort->lif->flags, IONIC_LIF_FLAG_TYPE_VPORT);

        if (vf_info != NULL &&
            BooleanFlagOn(vf_info->Flags, IONIC_VF_CONNECTED)) {
            SetFlag(pVPort->lif->lif_stats->flags, IONIC_LIF_FLAG_VF_CONNECTED);
        }

        pVPort->ProcessorAffinity = vPortParams->ProcessorAffinity;

        pVPort->QueuePairs = vPortParams->NumQueuePairs;

        pVPort->State = vPortParams->VPortState;

        pVPort->PortId = vPortParams->VPortId;

        if (vPortParams->VPortName.Length != 0) {

            pVPort->lif->type_name.Length = vPortParams->VPortName.Length;

            NdisMoveMemory(pVPort->lif->type_name.String,
                           vPortParams->VPortName.String,
                           vPortParams->VPortName.Length);
        }

        SetFlag(pVPort->Flags, IONIC_VPORT_STATE_INITIALIZED);

        if (pVPort->State == NdisNicSwitchVPortStateActivated) {

            ntStatus =
                _ionic_lif_rx_mode(pVPort->lif, ionic->master_lif->rx_mode);

            if (ntStatus != NDIS_STATUS_SUCCESS) {
                goto cleanup;
            }
        }

        if (RtlCheckBit(&ionic->master_lif->state, LIF_UP) == 0) {
            ntStatus = NDIS_STATUS_DEVICE_FAILED;
            goto cleanup;
        }

        ntStatus =
            ionic_lif_open(pVPort->lif, pVPort->PortId, (ULONG)-1,
                            pVPort->ProcessorAffinity);

        if (ntStatus != NDIS_STATUS_SUCCESS) {
            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                        "%s ionic_lif_open() failed for lif %p "
                        "Status %08lX\n",
                        __FUNCTION__, pVPort->lif, ntStatus));
            goto cleanup;
        }

        ntStatus = ionic_lif_start( pVPort->lif);

        if (ntStatus != NDIS_STATUS_SUCCESS) {
            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                        "%s ionic_lif_start() failed for lif %p "
                        "Status %08lX\n",
                        __FUNCTION__, pVPort->lif, ntStatus));
            goto cleanup;
        }

        SetFlag(pVPort->Flags, IONIC_VPORT_STATE_ACTIVATED);
    }

cleanup:

    if (bReleaseLock) {
        NdisReleaseRWLock(pVPort->PortLock, &lock_state);
    }

    return ntStatus;
}

NDIS_STATUS
oid_switch_delete_vport(struct ionic *ionic, void *info_buffer)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_NIC_SWITCH_DELETE_VPORT_PARAMETERS *vPortParams =
        (NDIS_NIC_SWITCH_DELETE_VPORT_PARAMETERS *)info_buffer;
    LOCK_STATE_EX lock_state;
    BOOLEAN bReleaseLock = FALSE;
    struct vport_info *pVPort = NULL;
    struct vf_info *vf_info = NULL;

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Adapter %p Flags %08lX VPortId %d\n", __FUNCTION__, ionic,
              vPortParams->Flags, vPortParams->VPortId));

    //
    // Some validation
    //
    if (vPortParams->VPortId == NDIS_DEFAULT_RECEIVE_QUEUE_ID) {
        ASSERT(FALSE);
        ntStatus = NDIS_STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

    pVPort = &ionic->SriovSwitch.Ports[vPortParams->VPortId];

    NdisAcquireRWLockWrite(pVPort->PortLock, &lock_state, 0);
    bReleaseLock = TRUE;

    if (BooleanFlagOn(pVPort->Flags, IONIC_VPORT_STATE_INITIALIZED)) {

        if (pVPort->AttachedFunctionId != NDIS_PF_FUNCTION_ID) {

            //
            // Lookup the vf that is being deattached to the vport
            //

            vf_info = locate_vfinfo(ionic, pVPort->AttachedFunctionId);

            if (vf_info == NULL ||
                !BooleanFlagOn(vf_info->Flags, IONIC_VF_CONNECTED)) {
                ASSERT(FALSE);
                ntStatus = NDIS_STATUS_NOT_SUPPORTED;
                goto cleanup;
            }

            if (pVPort->lif != NULL) {
                ClearFlag(pVPort->lif->lif_stats->flags,
                          IONIC_LIF_FLAG_VF_CONNECTED);
            }

            vf_info->vport_id = NDIS_PF_FUNCTION_ID;
            ClearFlag(vf_info->Flags, IONIC_VF_CONNECTED);
        }

        if (pVPort->lif != NULL) {

            ClearFlag(pVPort->lif->lif_stats->flags, IONIC_LIF_FLAG_ALLOCATED);

            ionic_lif_stop(pVPort->lif);
            ionic_lif_deinit(pVPort->lif);
            ionic_lif_free(pVPort->lif);
            pVPort->lif = NULL;
        }

        pVPort->Flags = 0;
        pVPort->filter_cnt = 0;
    }

cleanup:

    if (bReleaseLock) {
        NdisReleaseRWLock(pVPort->PortLock, &lock_state);
    }

    return ntStatus;
}

NDIS_STATUS
oid_filter_clear_filter_vswitch(struct ionic *ionic, void *info_buffer)
{
    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_RECEIVE_FILTER_CLEAR_PARAMETERS *pFilter =
        (NDIS_RECEIVE_FILTER_CLEAR_PARAMETERS *)info_buffer;
    LOCK_STATE_EX lock_state;
    ULONG ulIndex = 0;
    struct vport_info *vPort = NULL;
    ULONG vport_id = 0;

    vport_id = ionic->q_filter[pFilter->FilterId].q_index;

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s (OID_RECEIVE_FILTER_CLEAR_FILTER) Flags %08lX VPortId %d "
              "FilterId %d\n",
              __FUNCTION__, pFilter->Flags, vport_id, pFilter->FilterId));

    ionic->q_filter[pFilter->FilterId].q_index = 0;

    vPort = &ionic->SriovSwitch.Ports[vport_id];

    NdisAcquireRWLockWrite(vPort->PortLock, &lock_state, 0);

    if (!BooleanFlagOn(vPort->Flags, IONIC_VPORT_STATE_INITIALIZED)) {
        NdisReleaseRWLock(vPort->PortLock, &lock_state);
        ntStatus = NDIS_STATUS_INVALID_DATA;
        goto cleanup;
    }

    NdisReleaseRWLock(vPort->PortLock, &lock_state);

    for (ulIndex = 0; ulIndex < IONIC_RECEIVE_FILTER_COUNT; ulIndex++) {

        if (vPort->filter[ulIndex].FilterId == pFilter->FilterId) {

            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                      "%s Clearing FilterId %d for port %d\n", __FUNCTION__,
                      pFilter->FilterId, vport_id));

            if (BooleanFlagOn(vPort->filter[ulIndex].Flags, IONIC_FILTER_SET)) {

                if (BooleanFlagOn(vPort->filter[ulIndex].Flags,
                                  IONIC_FILTER_MAC_FILTER)) {
                    ionic_lif_addr(vPort->lif,
                                   (const u8 *)vPort->filter[ulIndex].MacAddr,
                                   false);

                    if (BooleanFlagOn(vPort->filter[ulIndex].Flags,
                                      IONIC_FILTER_STATION_ADDR)) {
                        ionic_lif_addr(
                            vPort->lif->ionic->master_lif,
                            (const u8 *)vPort->filter[ulIndex].MacAddr, true);
                    }
                } else if (BooleanFlagOn(vPort->filter[ulIndex].Flags,
                                         IONIC_FILTER_VLAN_FILTER)) {
                    ionic_lif_vlan(vPort->lif, vPort->filter[ulIndex].VLanId,
                                   false);

#if 0
					if( BooleanFlagOn( vPort->filter[ulIndex].Flags, IONIC_FILTER_STATION_VLAN)) {
						ionic_lif_vlan( vPort->lif->ionic->master_lif, 
										vPort->filter[ ulIndex].VLanId, 
										true);
					}
#endif

                    ClearFlag(vPort->Flags, IONIC_VPORT_STATE_VLAN_FLTR_SET);
                }

                ionic->vm_queue[pFilter->QueueId].filter[ulIndex].Flags = 0;
            }

            vPort->filter[ulIndex].Flags = 0;
            vPort->filter_cnt--;
        }
    }

cleanup:

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Complete for VPortId %d FilterId %d STatus %08lX\n",
              __FUNCTION__, vport_id, pFilter->FilterId, ntStatus));

    return ntStatus;
}

NDIS_STATUS
oid_filter_move_filter_vswitch(struct ionic *ionic, void *info_buffer)
{
    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_RECEIVE_FILTER_MOVE_FILTER_PARAMETERS *pFilter =
        (NDIS_RECEIVE_FILTER_MOVE_FILTER_PARAMETERS *)info_buffer;
    ULONG ulIndex = 0;
    LOCK_STATE_EX lock_state;
    struct vport_info *vSrcPort = NULL;
    struct vport_info *vDstPort = NULL;

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Adapter %p FilterId %d Src QueueId %d Src VPortId %d Dst "
              "QueueId %d Dst VPortId %d\n",
              __FUNCTION__, ionic, pFilter->FilterId, pFilter->SourceQueueId,
              pFilter->SourceVPortId, pFilter->DestQueueId,
              pFilter->DestVPortId));

    vSrcPort = &ionic->SriovSwitch.Ports[pFilter->SourceVPortId];

    NdisAcquireRWLockWrite(vSrcPort->PortLock, &lock_state, 0);

    if (!BooleanFlagOn(vSrcPort->Flags, IONIC_VPORT_STATE_INITIALIZED)) {

        NdisReleaseRWLock(vSrcPort->PortLock, &lock_state);

        ntStatus = NDIS_STATUS_INVALID_DATA;
        goto cleanup;
    }

    NdisReleaseRWLock(vSrcPort->PortLock, &lock_state);

    vDstPort = &ionic->SriovSwitch.Ports[pFilter->DestVPortId];

    NdisAcquireRWLockWrite(vDstPort->PortLock, &lock_state, 0);

    if (!BooleanFlagOn(vDstPort->Flags, IONIC_VPORT_STATE_INITIALIZED)) {

        NdisReleaseRWLock(vDstPort->PortLock, &lock_state);

        ntStatus = NDIS_STATUS_INVALID_DATA;
        goto cleanup;
    }

    NdisReleaseRWLock(vDstPort->PortLock, &lock_state);

    for (ulIndex = 0; ulIndex < vSrcPort->filter_cnt; ulIndex++) {

        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                  "\tFilter Flags %08lX\n", vSrcPort->filter[ulIndex].Flags));

        if (vSrcPort->filter[ulIndex].FilterId == pFilter->FilterId) {

            vDstPort->filter[vDstPort->filter_cnt].Flags =
                vSrcPort->filter[ulIndex].Flags;

            vDstPort->filter[vDstPort->filter_cnt].FilterId = pFilter->FilterId;

            vDstPort->filter[vDstPort->filter_cnt].FilterFlags =
                vSrcPort->filter[ulIndex].FilterFlags;

            vDstPort->filter[vDstPort->filter_cnt].FilterTest =
                vSrcPort->filter[ulIndex].FilterTest;

            if (BooleanFlagOn(vSrcPort->filter[ulIndex].Flags,
                              IONIC_FILTER_MAC_FILTER)) {

                NdisMoveMemory(vDstPort->filter[vDstPort->filter_cnt].MacAddr,
                               vSrcPort->filter[ulIndex].MacAddr, ETH_ALEN);

                ntStatus = ionic_lif_addr(
                    vSrcPort->lif,
                    (const u8 *)vSrcPort->filter[ulIndex].MacAddr, false);

                if (ntStatus != NDIS_STATUS_SUCCESS) {

                    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                              "%s Failed to clear MAC filter for vPort %d "
                              "Status %08lX\n",
                              __FUNCTION__, vSrcPort->PortId, ntStatus));
                    ntStatus = NDIS_STATUS_SUCCESS;
                }

                vSrcPort->filter_cnt--;
                vSrcPort->filter[ulIndex].Flags = 0;

                ntStatus = ionic_lif_addr(
                    vDstPort->lif,
                    (const u8 *)vDstPort->filter[ulIndex].MacAddr, true);

                if (ntStatus != NDIS_STATUS_SUCCESS) {

                    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                              "%s Failed to set MAC filter for vPort %d Status "
                              "%08lX\n",
                              __FUNCTION__, vSrcPort->PortId, ntStatus));
                    ntStatus = NDIS_STATUS_SUCCESS;
                }

                vDstPort->filter_cnt++;
            } else if (BooleanFlagOn(vSrcPort->filter[ulIndex].Flags,
                                     IONIC_FILTER_VLAN_FILTER)) {

                vDstPort->filter[vDstPort->filter_cnt].VLanId =
                    vSrcPort->filter[ulIndex].VLanId;

                ntStatus = ionic_lif_vlan(
                    vSrcPort->lif, vSrcPort->filter[ulIndex].VLanId, false);

                if (ntStatus != NDIS_STATUS_SUCCESS) {

                    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                              "%s Failed to clear VLan filter for vPort %d "
                              "Status %08lX\n",
                              __FUNCTION__, vSrcPort->PortId, ntStatus));
                    ntStatus = NDIS_STATUS_SUCCESS;
                }

                vSrcPort->filter_cnt--;
                vSrcPort->Flags = 0;

                ntStatus = ionic_lif_vlan(
                    vDstPort->lif, vDstPort->filter[ulIndex].VLanId, true);

                if (ntStatus != NDIS_STATUS_SUCCESS) {

                    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                              "%s Failed to set VLan filter for vPort %d "
                              "Status %08lX\n",
                              __FUNCTION__, vSrcPort->PortId, ntStatus));
                    ntStatus = NDIS_STATUS_SUCCESS;
                }

                vDstPort->filter_cnt++;
            }

            break;
        }
    }

cleanup:

    return ntStatus;
}

NDIS_STATUS
oid_switch_query_vport_params(struct ionic *ionic, NDIS_OID_REQUEST *OidRequest)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    //	NDIS_NIC_SWITCH_VPORT_PARAMETERS	*vPortParams =
    //(NDIS_NIC_SWITCH_VPORT_PARAMETERS
    //*)OidRequest->DATA.METHOD_INFORMATION.InformationBuffer;

    UNREFERENCED_PARAMETER(ionic);

    if (OidRequest->DATA.METHOD_INFORMATION.InputBufferLength <
        sizeof(NDIS_NIC_SWITCH_VPORT_PARAMETERS)) {
        ntStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
        OidRequest->DATA.METHOD_INFORMATION.BytesNeeded =
            sizeof(NDIS_NIC_SWITCH_VPORT_PARAMETERS);
        goto complete;
    }

complete:

    return ntStatus;
}

NDIS_STATUS
oid_switch_set_vport_params(struct ionic *ionic, void *info_buffer)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_NIC_SWITCH_VPORT_PARAMETERS *vPortParams =
        (NDIS_NIC_SWITCH_VPORT_PARAMETERS *)info_buffer;
    struct vport_info *vPort = NULL;
    LOCK_STATE_EX lock_state;
    BOOLEAN bReleaseLock = FALSE;

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Adapter %p VPortId %d\n", __FUNCTION__, ionic,
              vPortParams->VPortId));

    vPort = &ionic->SriovSwitch.Ports[vPortParams->VPortId];

    NdisAcquireRWLockWrite(vPort->PortLock, &lock_state, 0);
    bReleaseLock = TRUE;

    if (!BooleanFlagOn(vPort->Flags, IONIC_VPORT_STATE_INITIALIZED)) {
        ntStatus = NDIS_STATUS_INVALID_DATA;
        goto cleanup;
    }

    if (BooleanFlagOn(vPortParams->Flags,
                      NDIS_NIC_SWITCH_VPORT_PARAMS_NAME_CHANGED)) {
    }

    if (BooleanFlagOn(vPortParams->Flags,
                      NDIS_NIC_SWITCH_VPORT_PARAMS_INT_MOD_CHANGED)) {
    }

    if (BooleanFlagOn(vPortParams->Flags,
                      NDIS_NIC_SWITCH_VPORT_PARAMS_STATE_CHANGED)) {

        if (vPort->State == NdisNicSwitchVPortStateDeactivated &&
            vPortParams->VPortState == NdisNicSwitchVPortStateActivated) {

            NdisReleaseRWLock(vPort->PortLock, &lock_state);
            bReleaseLock = FALSE;

            ntStatus =
                _ionic_lif_rx_mode(vPort->lif, ionic->master_lif->rx_mode);

            if (ntStatus != NDIS_STATUS_SUCCESS) {
                goto cleanup;
            }

            if (RtlCheckBit(&ionic->master_lif->state, LIF_UP) == 0) {
                ntStatus = NDIS_STATUS_DEVICE_FAILED;
                goto cleanup;
            }

            ntStatus =
                ionic_lif_open(vPort->lif, vPort->PortId, (ULONG)-1,
                                vPort->ProcessorAffinity);

            if (ntStatus != NDIS_STATUS_SUCCESS) {
                DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                            "%s ionic_lif_open() failed for lif %p "
                            "Status %08lX\n",
                            __FUNCTION__, vPort->lif, ntStatus));
                goto cleanup;
            }

            ntStatus =
                ionic_lif_start(vPort->lif);

            if (ntStatus != NDIS_STATUS_SUCCESS) {
                DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                            "%s ionic_lif_start() failed for lif %p "
                            "Status %08lX\n",
                            __FUNCTION__, vPort->lif, ntStatus));
                goto cleanup;
            }

            SetFlag(vPort->Flags, IONIC_VPORT_STATE_ACTIVATED);
        }

        vPort->State = vPortParams->VPortState;
    }

    if (BooleanFlagOn(
            vPortParams->Flags,
            NDIS_NIC_SWITCH_VPORT_PARAMS_PROCESSOR_AFFINITY_CHANGED)) {
    }

cleanup:

    if (bReleaseLock) {
        NdisReleaseRWLock(vPort->PortLock, &lock_state);
    }

    return ntStatus;
}

NDIS_STATUS
oid_switch_create_vf(struct ionic *ionic, NDIS_OID_REQUEST *OidRequest)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_NIC_SWITCH_VF_PARAMETERS *pParams =
        (NDIS_NIC_SWITCH_VF_PARAMETERS *)
            OidRequest->DATA.METHOD_INFORMATION.InformationBuffer;
    ULONG ulIndex = 0;
    struct vf_info *vf_info = NULL;

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Adapter %p Flags %08lX SwitchId %d VF Id %08lX RId %08lX\n",
              __FUNCTION__, ionic, pParams->Flags, pParams->SwitchId));

    OidRequest->DATA.METHOD_INFORMATION.BytesRead =
        OidRequest->DATA.METHOD_INFORMATION.InputBufferLength;
    OidRequest->DATA.METHOD_INFORMATION.BytesWritten =
        OidRequest->DATA.METHOD_INFORMATION.OutputBufferLength;

    //
    // Locate an available vf slot
    //

    while (ulIndex < ionic->sriov_caps.NumVFs) {

        if (!BooleanFlagOn(ionic->SriovSwitch.vfs[ulIndex].Flags,
                           IONIC_VF_ALLOCATED)) {
            vf_info = &ionic->SriovSwitch.vfs[ulIndex];
            break;
        }

        ulIndex++;
    }

    if (vf_info == NULL) {
        ntStatus = NDIS_STATUS_NOT_SUPPORTED;
        goto cleanup;
    }

    SetFlag(vf_info->Flags, IONIC_VF_ALLOCATED);

    NdisMoveMemory(vf_info->MacAddr, pParams->CurrentMacAddress, ETH_ALEN);

    pParams->VFId = vf_info->vf_id;
    pParams->RequestorId = vf_info->rid;

cleanup:

    return ntStatus;
}

NDIS_STATUS
oid_switch_delete_vf(struct ionic *ionic, void *info_buffer)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_NIC_SWITCH_FREE_VF_PARAMETERS *pParams =
        (NDIS_NIC_SWITCH_FREE_VF_PARAMETERS *)info_buffer;
    struct vf_info *vf_info = NULL;

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Adapter %p Flags %08lX VF Id %08lX\n", __FUNCTION__, ionic,
              pParams->Flags, pParams->VFId));

    vf_info = locate_vfinfo(ionic, pParams->VFId);

    if (vf_info == NULL) {
        ntStatus = NDIS_STATUS_NOT_SUPPORTED;
        goto cleanup;
    }

    ASSERT(BooleanFlagOn(vf_info->Flags, IONIC_VF_ALLOCATED));
    ASSERT(!BooleanFlagOn(vf_info->Flags, IONIC_VF_CONNECTED));

    ClearFlag(vf_info->Flags, IONIC_VF_ALLOCATED);

cleanup:

    return ntStatus;
}

NDIS_STATUS
oid_switch_read_vf_ven_dev_id(struct ionic *ionic, NDIS_OID_REQUEST *OidRequest)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_SRIOV_VF_VENDOR_DEVICE_ID_INFO *pParams =
        (NDIS_SRIOV_VF_VENDOR_DEVICE_ID_INFO *)
            OidRequest->DATA.METHOD_INFORMATION.InformationBuffer;
    struct vf_info *vf_info = NULL;

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Adapter %p VF Id %08lX\n", __FUNCTION__, ionic,
              pParams->VFId));

    vf_info = locate_vfinfo(ionic, pParams->VFId);

    if (vf_info == NULL) {
        ntStatus = NDIS_STATUS_NOT_SUPPORTED;
        goto cleanup;
    }

    pParams->VendorId = ionic->pci_config.VendorID;
    pParams->DeviceId = ionic->pci_config.DeviceID;

    OidRequest->DATA.METHOD_INFORMATION.BytesRead =
        OidRequest->DATA.METHOD_INFORMATION.InputBufferLength;
    OidRequest->DATA.METHOD_INFORMATION.BytesWritten =
        OidRequest->DATA.METHOD_INFORMATION.OutputBufferLength;

cleanup:

    return ntStatus;
}

NDIS_STATUS
oid_switch_reset_vf(struct ionic *ionic, void *info_buffer)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_SRIOV_RESET_VF_PARAMETERS *pParams =
        (NDIS_SRIOV_RESET_VF_PARAMETERS *)info_buffer;
    struct vf_info *vf_info = NULL;

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Adapter %p VF Id %08lX\n", __FUNCTION__, ionic,
              pParams->VFId));

    vf_info = locate_vfinfo(ionic, pParams->VFId);

    if (vf_info == NULL) {
        ntStatus = NDIS_STATUS_NOT_SUPPORTED;
        goto cleanup;
    }

cleanup:

    return ntStatus;
}

NDIS_STATUS
oid_switch_set_vf_power(struct ionic *ionic,
                        void *info_buffer,
                        ULONG info_buffer_len,
                        ULONG *bytes_read)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_SRIOV_SET_VF_POWER_STATE_PARAMETERS *pParams =
        (NDIS_SRIOV_SET_VF_POWER_STATE_PARAMETERS *)info_buffer;
    struct vf_info *vf_info = NULL;

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Adapter %p VF Id %08lX State %08lX\n", __FUNCTION__, ionic,
              pParams->VFId, pParams->PowerState));

    vf_info = locate_vfinfo(ionic, pParams->VFId);

    if (vf_info == NULL) {
        ntStatus = NDIS_STATUS_NOT_SUPPORTED;
        goto cleanup;
    }

    switch (pParams->PowerState) {

    case NdisDeviceStateD0: {

        break;
    }

    case NdisDeviceStateD1: {

        break;
    }

    case NdisDeviceStateD2: {

        break;
    }

    case NdisDeviceStateD3: {

        break;
    }
    }

    vf_info->power_state = pParams->PowerState;

    *bytes_read = info_buffer_len;

cleanup:

    return ntStatus;
}