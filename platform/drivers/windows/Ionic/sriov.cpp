
#include "common.h"

struct vf_info *
locate_vfinfo(struct ionic *ionic, ULONG vf_id)
{
    struct vf_info *vf_info = NULL;

    for (ULONG index = 0; index < ionic->sriov_caps.NumVFs; index++) {

        if (ionic->SriovSwitch.vfs[index].vf_id == vf_id) {
            vf_info = &ionic->SriovSwitch.vfs[index];
            break;
        }
    }

    return vf_info;
}

NDIS_STATUS
oid_sriov_bar_resources(struct ionic *ionic, NDIS_OID_REQUEST *OidRequest)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_SRIOV_BAR_RESOURCES_INFO *pParams =
        (NDIS_SRIOV_BAR_RESOURCES_INFO *)
            OidRequest->DATA.METHOD_INFORMATION.InformationBuffer;
    struct vf_info *vf_info = NULL;
    CM_PARTIAL_RESOURCE_DESCRIPTOR *pResource = NULL;

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Adapter %p VF Id %08lX\n", __FUNCTION__, ionic,
              pParams->VFId));

    vf_info = locate_vfinfo(ionic, pParams->VFId);

    if (vf_info == NULL) {
        ntStatus = NDIS_STATUS_NOT_SUPPORTED;
        goto cleanup;
    }

    pResource = (CM_PARTIAL_RESOURCE_DESCRIPTOR *)((char *)pParams +
                                                   pParams->BarResourcesOffset);

    if (pParams->BarIndex == 0) {
        pResource->Type = vf_info->BAR[0].Type;
        pResource->ShareDisposition = vf_info->BAR[0].ShareDisposition;
        pResource->Flags = vf_info->BAR[0].Flags;

        pResource->u.Memory.Length = vf_info->BAR[0].len;
        pResource->u.Memory.Start.QuadPart = vf_info->BAR[0].bus_addr;
    } else if (pParams->BarIndex == 2) {
        pResource->Type = vf_info->BAR[1].Type;
        pResource->ShareDisposition = vf_info->BAR[1].ShareDisposition;
        pResource->Flags = vf_info->BAR[1].Flags;

        pResource->u.Memory.Length = vf_info->BAR[1].len;
        pResource->u.Memory.Start.QuadPart = vf_info->BAR[1].bus_addr;
    } else {
        NdisZeroMemory(pResource, sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));
    }

    OidRequest->DATA.METHOD_INFORMATION.BytesRead =
        OidRequest->DATA.METHOD_INFORMATION.InputBufferLength;
    OidRequest->DATA.METHOD_INFORMATION.BytesWritten =
        OidRequest->DATA.METHOD_INFORMATION.OutputBufferLength;

cleanup:

    return ntStatus;
}

NDIS_STATUS
oid_sriov_read_vf_pci_config(struct ionic *ionic, NDIS_OID_REQUEST *OidRequest)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_SRIOV_READ_VF_CONFIG_SPACE_PARAMETERS *pParams =
        (NDIS_SRIOV_READ_VF_CONFIG_SPACE_PARAMETERS *)
            OidRequest->DATA.METHOD_INFORMATION.InformationBuffer;
    struct vf_info *vf_info = NULL;
    ULONG ulLength = 0;
    PCI_COMMON_HEADER *pHdr = NULL;

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Adapter %p VF Id %08lX Offset %08lX Len %08lx\n",
              __FUNCTION__, ionic, pParams->VFId, pParams->Offset,
              pParams->Length));

    vf_info = locate_vfinfo(ionic, pParams->VFId);

    if (vf_info == NULL) {
        ntStatus = NDIS_STATUS_NOT_SUPPORTED;
        goto cleanup;
    }

    ulLength = NdisMGetVirtualFunctionBusData(
        ionic->adapterhandle, vf_info->vf_id,
        (void *)((char *)pParams + pParams->BufferOffset), pParams->Offset,
        pParams->Length);

    if (ulLength == 0) {
        ntStatus = NDIS_STATUS_NOT_SUPPORTED;
        goto cleanup;
    }

    if (pParams->Offset == 0) {

        pHdr = (PCI_COMMON_HEADER *)((char *)pParams + pParams->BufferOffset);

        if (pParams->Length >= 4) {
            pHdr->VendorID = PCI_VENDOR_ID_PENSANDO;
            pHdr->DeviceID = PCI_DEVICE_ID_PENSANDO_IONIC_ETH_VF;
        } else {
            pHdr->VendorID = PCI_VENDOR_ID_PENSANDO;
        }
    } else if (pParams->Offset == 2) {

        pHdr =
            (PCI_COMMON_HEADER *)((char *)pParams + pParams->BufferOffset - 2);
        pHdr->DeviceID = PCI_DEVICE_ID_PENSANDO_IONIC_ETH_VF;
    }

    OidRequest->DATA.METHOD_INFORMATION.BytesRead =
        OidRequest->DATA.METHOD_INFORMATION.InputBufferLength;
    OidRequest->DATA.METHOD_INFORMATION.BytesWritten =
        OidRequest->DATA.METHOD_INFORMATION.OutputBufferLength;

cleanup:

    return ntStatus;
}

NDIS_STATUS
oid_sriov_write_vf_pci_config(struct ionic *ionic,
                              void *info_buffer,
                              ULONG info_buffer_len,
                              ULONG *bytes_read)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_SRIOV_WRITE_VF_CONFIG_SPACE_PARAMETERS *pConfig =
        (NDIS_SRIOV_WRITE_VF_CONFIG_SPACE_PARAMETERS *)info_buffer;
    struct vf_info *vf_info = NULL;
    ULONG ulLength = 0;

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Adapter %p VF Id %08lX Offset %08lX Len %08lX\n",
              __FUNCTION__, ionic, pConfig->VFId, pConfig->Offset,
              pConfig->Length));

    vf_info = locate_vfinfo(ionic, pConfig->VFId);

    if (vf_info == NULL) {
        ntStatus = NDIS_STATUS_NOT_SUPPORTED;
        goto cleanup;
    }

    ulLength = NdisMSetVirtualFunctionBusData(
        ionic->adapterhandle, vf_info->vf_id,
        (void *)((char *)pConfig + pConfig->BufferOffset), pConfig->Offset,
        pConfig->Length);

    ASSERT(ulLength == pConfig->Length);

    *bytes_read = info_buffer_len;

cleanup:

    return ntStatus;
}

NDIS_VF_RID
get_rid(struct ionic *ionic, USHORT vf_id)
{

    NDIS_VF_RID rid = 0;
    USHORT usSegNmb = 0;
    UCHAR ucBus = 0x00;
    UCHAR ucFunc = 0x00;

    NdisMGetVirtualFunctionLocation(ionic->adapterhandle, vf_id, &usSegNmb,
                                    &ucBus, &ucFunc);

    rid = NDIS_MAKE_RID(usSegNmb, ucBus, ucFunc);

    return rid;
}