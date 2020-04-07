
#include "common.h"

NDIS_STATUS
ionic_port_identify(struct ionic *ionic)
{
    struct identity *ident = &ionic->ident;
    struct ionic_dev *idev = &ionic->idev;
    ULONG sz;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    NdisAcquireSpinLock(&ionic->dev_cmd_lock);

    ionic_dev_cmd_port_identify(idev);
    status = ionic_dev_cmd_wait(ionic, devcmd_timeout);
    if (status == NDIS_STATUS_SUCCESS) {
        sz = min(sizeof(ident->port), sizeof(idev->dev_cmd_regs->data));
        memcpy_fromio(&ident->port, &idev->dev_cmd_regs->data, sz);
    }

    NdisReleaseSpinLock(&ionic->dev_cmd_lock);

    if (status == NDIS_STATUS_SUCCESS) {

        if (ionic->pci_config.DeviceID ==
                PCI_DEVICE_ID_PENSANDO_IONIC_ETH_MGMT &&
            ident->port.config.mtu == 0) {
            ident->port.config.mtu = 0x2400; // xxx
        }

        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE, "%s successful\n",
                  __FUNCTION__));
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE, "\tspeed %d\n",
                  ident->port.config.speed));
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE, "\tmtu %d\n",
                  ident->port.config.mtu));
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE, "\tstate %d\n",
                  ident->port.config.state));
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE, "\tan_enable %d\n",
                  ident->port.config.an_enable));
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE, "\tfec_type %d\n",
                  ident->port.config.fec_type));
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                  "\tpause_type %d\n", ident->port.config.pause_type));
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                  "\tloopback_mode %d\n", ident->port.config.loopback_mode));
    } else {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Failed ionic_dev_cmd_port_identify status %08lX\n",
                  __FUNCTION__, status));
    }

    return status;
}

NDIS_STATUS
ionic_port_init(struct ionic *ionic)
{
    struct identity *ident = &ionic->ident;
    struct ionic_dev *idev = &ionic->idev;
    ULONG sz;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    if (idev->port_info)
        return 0;

    idev->port_info_sz = ALIGN(sizeof(*idev->port_info), PAGE_SIZE);
    idev->port_info = (port_info *)dma_alloc_coherent(ionic, idev->port_info_sz,
                                                      &idev->port_info_pa, 0);
    if (!idev->port_info) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Failed to allocate port info, aborting\n", __FUNCTION__));
        return NDIS_STATUS_RESOURCES;
    }

    sz = min(sizeof(ident->port.config), sizeof(idev->dev_cmd_regs->data));

    NdisAcquireSpinLock(&ionic->dev_cmd_lock);

    memcpy_toio(&idev->dev_cmd_regs->data, &ident->port.config, sz);
    ionic_dev_cmd_port_init(idev);
    status = ionic_dev_cmd_wait(ionic, devcmd_timeout);

    ionic_dev_cmd_port_state(&ionic->idev, PORT_ADMIN_STATE_UP);
    (void)ionic_dev_cmd_wait(ionic, devcmd_timeout);

    NdisReleaseSpinLock(&ionic->dev_cmd_lock);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Failed to init port status %08lX\n", __FUNCTION__,
                  status));
    }

    return status;
}

NDIS_STATUS
ionic_port_reset(struct ionic *ionic)
{
    struct ionic_dev *idev = &ionic->idev;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    if (!idev->port_info)
        return 0;

    NdisAcquireSpinLock(&ionic->dev_cmd_lock);
    ionic_dev_cmd_port_reset(idev);
    status = ionic_dev_cmd_wait(ionic, devcmd_timeout);
    NdisReleaseSpinLock(&ionic->dev_cmd_lock);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Failed to reset port status %08lX\n", __FUNCTION__,
                  status));
    }

	if( idev->port_info != NULL) {
		dma_free_coherent(ionic, idev->port_info_sz, idev->port_info,
						  idev->port_info_pa);
		idev->port_info = NULL;
		idev->port_info_pa = 0;
	}

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
oid_port_create(struct ionic *ionic,
                void *data_buffer,
                ULONG data_buffer_length,
                ULONG *bytes_needed)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_SWITCH_PORT_PARAMETERS *pPort =
        (NDIS_SWITCH_PORT_PARAMETERS *)data_buffer;

    UNREFERENCED_PARAMETER(ionic);

    if (pPort == NULL) {
        goto cleanup;
    }

    if (data_buffer_length < sizeof(NDIS_SWITCH_PORT_PARAMETERS)) {
        ntStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
        *bytes_needed = sizeof(NDIS_SWITCH_PORT_PARAMETERS);
        goto cleanup;
    }

    DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE,
              "%s For %p PortId %d Type %s State %s ValidationPort %s\n",
              __FUNCTION__, ionic, pPort->PortId,
              GetPortTypeString(pPort->PortType),
              GetPortStateString(pPort->PortState),
              pPort->IsValidationPort ? "Yes" : "No"));

cleanup:

    return ntStatus;
}

NDIS_STATUS
oid_port_teardown(struct ionic *ionic,
                  void *data_buffer,
                  ULONG data_buffer_length,
                  ULONG *bytes_needed)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_SWITCH_PORT_PARAMETERS *pPort =
        (NDIS_SWITCH_PORT_PARAMETERS *)data_buffer;

    UNREFERENCED_PARAMETER(ionic);

    if (pPort == NULL) {
        goto cleanup;
    }

    if (data_buffer_length < sizeof(NDIS_SWITCH_PORT_PARAMETERS)) {
        ntStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
        *bytes_needed = sizeof(NDIS_SWITCH_PORT_PARAMETERS);
        goto cleanup;
    }

    DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE,
              "%s For %p PortId %d Type %s State %s ValidationPort %s\n",
              __FUNCTION__, ionic, pPort->PortId,
              GetPortTypeString(pPort->PortType),
              GetPortStateString(pPort->PortState),
              pPort->IsValidationPort ? "Yes" : "No"));

cleanup:

    return ntStatus;
}

NDIS_STATUS
oid_port_delete(struct ionic *ionic,
                void *data_buffer,
                ULONG data_buffer_length,
                ULONG *bytes_needed)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_SWITCH_PORT_PARAMETERS *pPort =
        (NDIS_SWITCH_PORT_PARAMETERS *)data_buffer;

    UNREFERENCED_PARAMETER(ionic);

    if (pPort == NULL) {
        goto cleanup;
    }

    if (data_buffer_length < sizeof(NDIS_SWITCH_PORT_PARAMETERS)) {
        ntStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
        *bytes_needed = sizeof(NDIS_SWITCH_PORT_PARAMETERS);
        goto cleanup;
    }

    DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE,
              "%s For %p PortId %d Type %s State %s ValidationPort %s\n",
              __FUNCTION__, ionic, pPort->PortId,
              GetPortTypeString(pPort->PortType),
              GetPortStateString(pPort->PortState),
              pPort->IsValidationPort ? "Yes" : "No"));

cleanup:

    return ntStatus;
}

NTSTATUS
IoctlPortGet(PVOID buf, ULONG inlen, ULONG outlen, PULONG outbytes)
{
    struct ionic *ionic = NULL;
    PortConfigCB *cb = (PortConfigCB *)buf;
    union dev_cmd_comp comp = {};
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    if (inlen < ADAPTER_NAME_MAX_SZ) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    if (outlen < sizeof(*cb)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    NdisAcquireSpinLock(&AdapterListLock);

    ionic = FindAdapterByNameLocked((PWCHAR)buf);
    if (ionic == NULL) {
        NdisReleaseSpinLock(&AdapterListLock);
        return STATUS_INVALID_PARAMETER;
    }

    NdisAcquireSpinLock(&ionic->dev_cmd_lock);

    // AutoNeg
    ionic_dev_cmd_port_get_autoneg(&ionic->idev);
    status = ionic_dev_cmd_wait(ionic, devcmd_timeout);
    if (status != NDIS_STATUS_SUCCESS) {
        goto out;
    }
    ionic_dev_cmd_comp(&ionic->idev, &comp);
    cb->AutoNeg = comp.port_getattr.an_enable;

    // Speed
    ionic_dev_cmd_port_get_speed(&ionic->idev);
    status = ionic_dev_cmd_wait(ionic, devcmd_timeout);
    if (status != NDIS_STATUS_SUCCESS) {
        goto out;
    }
    ionic_dev_cmd_comp(&ionic->idev, &comp);
    cb->Speed = comp.port_getattr.speed;

    // FEC
    ionic_dev_cmd_port_get_fec(&ionic->idev);
    status = ionic_dev_cmd_wait(ionic, devcmd_timeout);
    if (status != NDIS_STATUS_SUCCESS) {
        goto out;
    }
    ionic_dev_cmd_comp(&ionic->idev, &comp);
    cb->FEC = comp.port_getattr.fec_type;

    // Pause
    ionic_dev_cmd_port_get_pause(&ionic->idev);
    status = ionic_dev_cmd_wait(ionic, devcmd_timeout);
    if (status != NDIS_STATUS_SUCCESS) {
        goto out;
    }
    ionic_dev_cmd_comp(&ionic->idev, &comp);
    cb->Pause = comp.port_getattr.pause_type;

out:
    NdisReleaseSpinLock(&ionic->dev_cmd_lock);
    NdisReleaseSpinLock(&AdapterListLock);

    if (status == NDIS_STATUS_SUCCESS) {
        *outbytes = sizeof(*cb);
    }

    return status;
}

NTSTATUS
IoctlPortSet(PVOID buf, ULONG inlen)
{
    struct ionic *ionic = NULL;
    PortSetCB *cb = (PortSetCB *)buf;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    if (inlen < sizeof(*cb)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    NdisAcquireSpinLock(&AdapterListLock);

    ionic = FindAdapterByNameLocked(cb->AdapterName);
    if (ionic == NULL) {
        NdisReleaseSpinLock(&AdapterListLock);
        return STATUS_INVALID_PARAMETER;
    }

    NdisAcquireSpinLock(&ionic->dev_cmd_lock);

    // AutoNeg
    if (cb->Flags & PORT_SET_AUTONEG) {
        ionic_dev_cmd_port_autoneg(&ionic->idev, (u8)cb->Config.AutoNeg);
        status = ionic_dev_cmd_wait(ionic, devcmd_timeout);
        if (status != NDIS_STATUS_SUCCESS) {
            goto out;
        }
    }

    // Speed
    if (cb->Flags & PORT_SET_SPEED) {
        LONG speed = NormalizeSpeed(cb->Config.Speed);
        if (speed < 0) {
            status = NDIS_STATUS_INVALID_PARAMETER;
            goto out;
        }
        ionic_dev_cmd_port_speed(&ionic->idev, (u32)speed);
        status = ionic_dev_cmd_wait(ionic, devcmd_timeout);
        if (status != NDIS_STATUS_SUCCESS) {
            goto out;
        }
    }

    // FEC
    if (cb->Flags & PORT_SET_FEC) {
        ionic_dev_cmd_port_fec(&ionic->idev, (u8)cb->Config.FEC);
        status = ionic_dev_cmd_wait(ionic, devcmd_timeout);
        if (status != NDIS_STATUS_SUCCESS) {
            goto out;
        }
    }

    // Pause
    if (cb->Flags & PORT_SET_PAUSE) {
        ionic_dev_cmd_port_pause(&ionic->idev, (u8)cb->Config.Pause);
        status = ionic_dev_cmd_wait(ionic, devcmd_timeout);
        if (status != NDIS_STATUS_SUCCESS) {
            goto out;
        }
    }

out:
    NdisReleaseSpinLock(&ionic->dev_cmd_lock);

    if (status == STATUS_SUCCESS) {
        // TODO: persist these settings in Registry on success
    }

    NdisReleaseSpinLock(&AdapterListLock);

    return status;
}
