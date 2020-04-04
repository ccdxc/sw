
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

    ident->port.config.an_enable = ionic->config_port_speed == 0;
    ident->port.config.speed = ionic->config_port_speed;

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