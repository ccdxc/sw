
#include "common.h"

NDIS_STATUS
RegisterDevice(NDIS_HANDLE Adapter)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    UNICODE_STRING uniDeviceName;
    UNICODE_STRING uniDeviceLinkUnicodeString;
    PDRIVER_DISPATCH pDispatchTable[IRP_MJ_MAXIMUM_FUNCTION + 1];
    NDIS_DEVICE_OBJECT_ATTRIBUTES ndisDeviceAttribute;
    AdapterCntrlExt *pDeviceExt = NULL;

    NdisZeroMemory(pDispatchTable,
                   (IRP_MJ_MAXIMUM_FUNCTION + 1) * sizeof(PDRIVER_DISPATCH));

    pDispatchTable[IRP_MJ_CREATE] = DispatchCb;
    pDispatchTable[IRP_MJ_CLEANUP] = DispatchCb;
    pDispatchTable[IRP_MJ_CLOSE] = DispatchCb;
    pDispatchTable[IRP_MJ_DEVICE_CONTROL] = DeviceIoControl;

    NdisInitUnicodeString(&uniDeviceName, IONIC_NTDEVICE_STRING);
    NdisInitUnicodeString(&uniDeviceLinkUnicodeString, IONIC_LINKNAME_STRING);

    //
    // Create a device object and register our dispatch handlers
    //

    NdisZeroMemory(&ndisDeviceAttribute, sizeof(NDIS_DEVICE_OBJECT_ATTRIBUTES));

    ndisDeviceAttribute.Header.Type = NDIS_OBJECT_TYPE_DEVICE_OBJECT_ATTRIBUTES;
    ndisDeviceAttribute.Header.Revision =
        NDIS_DEVICE_OBJECT_ATTRIBUTES_REVISION_1;
    ndisDeviceAttribute.Header.Size = sizeof(NDIS_DEVICE_OBJECT_ATTRIBUTES);

    ndisDeviceAttribute.DeviceName = &uniDeviceName;
    ndisDeviceAttribute.SymbolicName = &uniDeviceLinkUnicodeString;
    ndisDeviceAttribute.MajorFunctions = &pDispatchTable[0];
    ndisDeviceAttribute.ExtensionSize = sizeof(AdapterCntrlExt);

    ntStatus = NdisRegisterDeviceEx(Adapter, &ndisDeviceAttribute,
                                    &AdapterCntrlDevObj, &AdapterControl);

    if (ntStatus == NDIS_STATUS_SUCCESS) {
        pDeviceExt = (AdapterCntrlExt *)NdisGetDeviceReservedExtension(
            AdapterCntrlDevObj);
        pDeviceExt->AdapterHandle = AdapterControl;
    }

    return ntStatus;
}

void
DeregisterDevice()

{
    if (AdapterControl != NULL) {
        NdisDeregisterDeviceEx(AdapterControl);
    }

    AdapterControl = NULL;
}

NTSTATUS
DispatchCb(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{

    PIO_STACK_LOCATION pIrpStack;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(DeviceObject);

    pIrpStack = IoGetCurrentIrpStackLocation(Irp);

    switch (pIrpStack->MajorFunction) {
    case IRP_MJ_CREATE:
        break;

    case IRP_MJ_CLEANUP:
        break;

    case IRP_MJ_CLOSE:
        break;

    default:
        break;
    }

    Irp->IoStatus.Status = ntStatus;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return ntStatus;
}

NTSTATUS
DeviceIoControl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{

    PIO_STACK_LOCATION pIrpSp;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    AdapterCntrlExt *pDeviceExt = NULL;
    ULONG ulInfoLen = 0;

    pIrpSp = IoGetCurrentIrpStackLocation(Irp);

    if (pIrpSp->FileObject == NULL) {
        Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_UNSUCCESSFUL;
    }

    pDeviceExt =
        (AdapterCntrlExt *)NdisGetDeviceReservedExtension(DeviceObject);

    Irp->IoStatus.Information = 0;

    switch (pIrpSp->Parameters.DeviceIoControl.IoControlCode) {

    case IOCTL_IONIC_CONFIGURE_TRACE: {

        if (Irp->AssociatedIrp.SystemBuffer == NULL ||
            pIrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(TraceConfigCB)) {
            ntStatus = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        ConfigureTrace((TraceConfigCB *)Irp->AssociatedIrp.SystemBuffer);

        break;
    }

    case IOCTL_IONIC_GET_TRACE: {
        ntStatus = GetTraceBuffer(
            pIrpSp->Parameters.DeviceIoControl.OutputBufferLength,
            Irp->AssociatedIrp.SystemBuffer, &ulInfoLen);

        break;
    }

    case IOCTL_IONIC_GET_DEV_STATS: {

        ntStatus =
            GetDevStats(pIrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                        Irp->AssociatedIrp.SystemBuffer, &ulInfoLen);

        break;
    }

    case IOCTL_IONIC_GET_MGMT_STATS: {

        ntStatus =
            GetMgmtStats(pIrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                         Irp->AssociatedIrp.SystemBuffer, &ulInfoLen);
        break;
    }

    case IOCTL_IONIC_GET_PORT_STATS: {

        ULONG *pulPort = (ULONG *)Irp->AssociatedIrp.SystemBuffer;

        if (pulPort == NULL ||
            pIrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(ULONG)) {
            ntStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
            break;
        }

        ntStatus = GetPortStats(
            *pulPort, pIrpSp->Parameters.DeviceIoControl.OutputBufferLength,
            Irp->AssociatedIrp.SystemBuffer, &ulInfoLen);

        break;
    }

    case IOCTL_IONIC_GET_LIF_STATS: {

        ULONG *pulPortLif = (ULONG *)Irp->AssociatedIrp.SystemBuffer;

        if (pulPortLif == NULL ||
            pIrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(ULONG)) {
            ntStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
            break;
        }

        ntStatus = GetLifStats(
            *pulPortLif, pIrpSp->Parameters.DeviceIoControl.OutputBufferLength,
            Irp->AssociatedIrp.SystemBuffer, &ulInfoLen);

        break;
    }

    case IOCTL_IONIC_GET_PERF_STATS: {

        ntStatus = GetPerfStats(
            Irp->AssociatedIrp.SystemBuffer,
            pIrpSp->Parameters.DeviceIoControl.OutputBufferLength, &ulInfoLen);

        break;
    }

    case IOCTL_IONIC_RESET_STATS: {

        break;
    }

    case IOCTL_IONIC_SET_RX_BUDGET: {

        if (Irp->AssociatedIrp.SystemBuffer == NULL ||
            pIrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(ULONG)) {
            ntStatus = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        ntStatus =
            ConfigureRxBudget(*((ULONG *)Irp->AssociatedIrp.SystemBuffer));

        break;
    }

	case IOCTL_IONIC_GET_REG_KEY_INFO: {

        ntStatus = GetRegKeyInfo(
            Irp->AssociatedIrp.SystemBuffer,
            pIrpSp->Parameters.DeviceIoControl.OutputBufferLength, &ulInfoLen);

		break;
	}

    case IOCTL_IONIC_PORT_GET: {
        ntStatus = IoctlPortGet(Irp->AssociatedIrp.SystemBuffer,
                                pIrpSp->Parameters.DeviceIoControl.InputBufferLength,
                                pIrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                                &ulInfoLen);
        break;
    }

    case IOCTL_IONIC_PORT_SET: {
        ntStatus = IoctlPortSet(Irp->AssociatedIrp.SystemBuffer,
                                pIrpSp->Parameters.DeviceIoControl.InputBufferLength);
        break;
    }

    default: {
        ntStatus = STATUS_INVALID_PARAMETER;
        break;
    }
    }

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = ulInfoLen;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return ntStatus;
}
