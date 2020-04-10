
#include "common.h"
#define DEFINITIONS_ONLY
#include "registry.h"

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
    NDIS_STRING AdapterNameString = {};
    AdapterCB cb = {};
    PortSetCB *resp = NULL;
    union dev_cmd_comp comp = {};
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    bool found = false;

    if (inlen < sizeof(cb)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    if (outlen < sizeof(*resp)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    cb = *(AdapterCB *)buf;
    resp = (PortSetCB *)buf;
    InitAdapterNameString(&AdapterNameString, cb.AdapterName);

    NdisZeroMemory(buf, outlen);

    PAGED_CODE();
    NDIS_WAIT_FOR_MUTEX(&AdapterListLock);

    ListForEachEntry(ionic, &AdapterList, struct ionic, list_entry) {
        if (!MatchesAdapterNameIonic(&AdapterNameString, ionic)) {
            continue;
        }
        found = true;

        if (cb.Skip) {
            --cb.Skip;
            continue;
        }

        // ioctl response is for one adapter at a time
        if (*outbytes != 0) {
            status = NDIS_STATUS_BUFFER_OVERFLOW;
            break;
        }

        wcscpy_s(resp->AdapterName, ADAPTER_NAME_MAX_SZ, ionic->name.Buffer);

        NdisAcquireSpinLock(&ionic->dev_cmd_lock);

        // AutoNeg
        ionic_dev_cmd_port_get_autoneg(&ionic->idev);
        status = ionic_dev_cmd_wait(ionic, devcmd_timeout);
        if (status != NDIS_STATUS_SUCCESS) {
            NdisReleaseSpinLock(&ionic->dev_cmd_lock);
            break;
        }
        ionic_dev_cmd_comp(&ionic->idev, &comp);
        resp->Config.AutoNeg = comp.port_getattr.an_enable;

        // Speed
        ionic_dev_cmd_port_get_speed(&ionic->idev);
        status = ionic_dev_cmd_wait(ionic, devcmd_timeout);
        if (status != NDIS_STATUS_SUCCESS) {
            NdisReleaseSpinLock(&ionic->dev_cmd_lock);
            break;
        }
        ionic_dev_cmd_comp(&ionic->idev, &comp);
        resp->Config.Speed = comp.port_getattr.speed;

        // FEC
        ionic_dev_cmd_port_get_fec(&ionic->idev);
        status = ionic_dev_cmd_wait(ionic, devcmd_timeout);
        if (status != NDIS_STATUS_SUCCESS) {
            NdisReleaseSpinLock(&ionic->dev_cmd_lock);
            break;
        }
        ionic_dev_cmd_comp(&ionic->idev, &comp);
        resp->Config.FEC = comp.port_getattr.fec_type;

        // Pause
        ionic_dev_cmd_port_get_pause(&ionic->idev);
        status = ionic_dev_cmd_wait(ionic, devcmd_timeout);
        if (status != NDIS_STATUS_SUCCESS) {
            NdisReleaseSpinLock(&ionic->dev_cmd_lock);
            break;
        }
        ionic_dev_cmd_comp(&ionic->idev, &comp);
        resp->Config.Pause = comp.port_getattr.pause_type;

        NdisReleaseSpinLock(&ionic->dev_cmd_lock);

        *outbytes += sizeof(*resp);

        ++resp;
    }

    NDIS_RELEASE_MUTEX(&AdapterListLock);

    // If specific device was requested but not found
    if (AdapterNameString.Length != 0 && !found) {
        status = NDIS_STATUS_INVALID_PARAMETER;
    }

    return status;
}

NTSTATUS
IoctlPortSet(PVOID buf, ULONG inlen)
{
    struct ionic *ionic = NULL;
    NDIS_STRING AdapterNameString = {};
    PortSetCB *cb = (PortSetCB *)buf;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    bool found = false;

    if (inlen < sizeof(*cb)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    InitAdapterNameString(&AdapterNameString, cb->AdapterName);

    PAGED_CODE();
    NDIS_WAIT_FOR_MUTEX(&AdapterListLock);

    ListForEachEntry(ionic, &AdapterList, struct ionic, list_entry) {
        if (!MatchesAdapterNameIonic(&AdapterNameString, ionic)) {
            continue;
        }
        found = true;

        NdisAcquireSpinLock(&ionic->dev_cmd_lock);

        // AutoNeg
        if (cb->Flags & PORT_SET_AUTONEG) {
            ionic_dev_cmd_port_autoneg(&ionic->idev, (u8)cb->Config.AutoNeg);
            status = ionic_dev_cmd_wait(ionic, devcmd_timeout);
            if (status != NDIS_STATUS_SUCCESS) {
                NdisReleaseSpinLock(&ionic->dev_cmd_lock);
                break;
            }
            ionic->registry_config[ IONIC_REG_AUTONEG].current_value = cb->Config.AutoNeg;
        }

        // Speed
        if (cb->Flags & PORT_SET_SPEED) {
            LONG speed = NormalizeSpeed(cb->Config.Speed);
            if (speed < 0) {
                status = NDIS_STATUS_INVALID_PARAMETER;
                NdisReleaseSpinLock(&ionic->dev_cmd_lock);
                break;
            }
            ionic_dev_cmd_port_speed(&ionic->idev, (u32)speed);
            status = ionic_dev_cmd_wait(ionic, devcmd_timeout);
            if (status != NDIS_STATUS_SUCCESS) {
                NdisReleaseSpinLock(&ionic->dev_cmd_lock);
                break;
            }
            ionic->registry_config[ IONIC_REG_SPEED].current_value = speed;
        }

        // FEC
        if (cb->Flags & PORT_SET_FEC) {
            ionic_dev_cmd_port_fec(&ionic->idev, (u8)cb->Config.FEC);
            status = ionic_dev_cmd_wait(ionic, devcmd_timeout);
            if (status != NDIS_STATUS_SUCCESS) {
                NdisReleaseSpinLock(&ionic->dev_cmd_lock);
                break;
            }
            ionic->registry_config[ IONIC_REG_FEC].current_value = cb->Config.FEC;
        }

        // Pause
        if (cb->Flags & PORT_SET_PAUSE) {
            ionic_dev_cmd_port_pause(&ionic->idev, (u8)cb->Config.Pause);
            status = ionic_dev_cmd_wait(ionic, devcmd_timeout);
            if (status != NDIS_STATUS_SUCCESS) {
                NdisReleaseSpinLock(&ionic->dev_cmd_lock);
                break;
            }
            ionic->registry_config[ IONIC_REG_PAUSE].current_value = cb->Config.Pause;
        }

        NdisReleaseSpinLock(&ionic->dev_cmd_lock);

        // Configuration of this port succeeded, then update

        // AutoNeg
        if (cb->Flags & PORT_SET_AUTONEG) {
            UpdateRegistryKeyword(ionic, IONIC_REG_AUTONEG);
        }

        // Speed
        if (cb->Flags & PORT_SET_SPEED) {
            UpdateRegistryKeyword(ionic, IONIC_REG_SPEED);
        }

        // FEC
        if (cb->Flags & PORT_SET_FEC) {
            UpdateRegistryKeyword(ionic, IONIC_REG_FEC);
        }

        // Pause
        if (cb->Flags & PORT_SET_PAUSE) {
            UpdateRegistryKeyword(ionic, IONIC_REG_PAUSE);
        }
    }

    NDIS_RELEASE_MUTEX(&AdapterListLock);

    // If specific device was requested but not found
    if (AdapterNameString.Length != 0 && !found) {
        status = NDIS_STATUS_INVALID_PARAMETER;
    }

    return status;
}

NDIS_STATUS
set_port_config(struct ionic *ionic)
{

	NDIS_STATUS		status = NDIS_STATUS_SUCCESS;

	NdisAcquireSpinLock(&ionic->dev_cmd_lock);

    // AutoNeg
    if (ionic->registry_config[ IONIC_REG_AUTONEG].current_value != IONIC_REG_UNDEFINED &&
		(ionic->registry_config[ IONIC_REG_AUTONEG].current_value == 0 ||
		 ionic->registry_config[ IONIC_REG_AUTONEG].current_value == 1)) {
        ionic_dev_cmd_port_autoneg(&ionic->idev, (u8)ionic->registry_config[ IONIC_REG_AUTONEG].current_value);
        status = ionic_dev_cmd_wait(ionic, devcmd_timeout);
        if (status != NDIS_STATUS_SUCCESS) {
            goto out;
        }
    }

    // Speed
    if (ionic->registry_config[ IONIC_REG_SPEED].current_value != IONIC_REG_UNDEFINED) {
        LONG speed = NormalizeSpeed(ionic->registry_config[ IONIC_REG_SPEED].current_value);
        if (speed >= 0) {
			ionic_dev_cmd_port_speed(&ionic->idev, (u32)speed);
			status = ionic_dev_cmd_wait(ionic, devcmd_timeout);
			if (status != NDIS_STATUS_SUCCESS) {
				goto out;
			}
		}
    }

    // FEC
    if (ionic->registry_config[ IONIC_REG_FEC].current_value != IONIC_REG_UNDEFINED &&
		( ionic->registry_config[ IONIC_REG_FEC].current_value == PORT_FEC_TYPE_NONE ||
		  ionic->registry_config[ IONIC_REG_FEC].current_value == PORT_FEC_TYPE_FC ||
		  ionic->registry_config[ IONIC_REG_FEC].current_value == PORT_FEC_TYPE_RS)) {
        ionic_dev_cmd_port_fec(&ionic->idev, (u8)ionic->registry_config[ IONIC_REG_FEC].current_value);
        status = ionic_dev_cmd_wait(ionic, devcmd_timeout);
        if (status != NDIS_STATUS_SUCCESS) {
            goto out;
        }
    }

    // Pause
    if (ionic->registry_config[ IONIC_REG_PAUSE].current_value != IONIC_REG_UNDEFINED &&
		( ionic->registry_config[ IONIC_REG_PAUSE].current_value == 0 ||
		  ionic->registry_config[ IONIC_REG_PAUSE].current_value == IONIC_PAUSE_F_TX ||
		  ionic->registry_config[ IONIC_REG_PAUSE].current_value == IONIC_PAUSE_F_RX ||
		  ionic->registry_config[ IONIC_REG_PAUSE].current_value == (IONIC_PAUSE_F_TX | IONIC_PAUSE_F_RX))) {
        ionic_dev_cmd_port_pause(&ionic->idev, (u8)ionic->registry_config[ IONIC_REG_PAUSE].current_value);
        status = ionic_dev_cmd_wait(ionic, devcmd_timeout);
        if (status != NDIS_STATUS_SUCCESS) {
            goto out;
        }
    }

out:

	NdisReleaseSpinLock(&ionic->dev_cmd_lock);

	return status;
}
