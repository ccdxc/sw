#include "common.h"

NDIS_STATUS
GetDevStats(ULONG BufferLength, void *Buffer, ULONG *BytesReturned)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    struct dev_port_stats *pStats = NULL;
    MDL *pMdl = NULL;
    void *pSystemBuffer = NULL;
    struct ionic *ionic = NULL;
    ULONG ulPortIndex = 0;

    if (BufferLength < sizeof(struct dev_port_stats) * port_count) {
        ntStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
        goto cleanup;
    }

    //
    // Lock down the memory
    //

    pSystemBuffer = LockBuffer(Buffer, BufferLength, &pMdl);

    if (pSystemBuffer == NULL) {
        ntStatus = NDIS_STATUS_RESOURCES;
        goto cleanup;
    }

    pStats = (struct dev_port_stats *)pSystemBuffer;

    NdisZeroMemory(pStats, BufferLength);

    NdisAcquireSpinLock(&AdapterListLock);

    ionic = (struct ionic *)AdapterList.Flink;
    *BytesReturned = 0;

    while (ulPortIndex < (ULONG)port_count) {

        NdisMoveMemory(pStats, &ionic->port_stats,
                       sizeof(struct dev_port_stats));

        *BytesReturned += sizeof(struct dev_port_stats);

        ionic = (struct ionic *)ionic->list_entry.Flink;
        pStats = (struct dev_port_stats *)((char *)pStats +
                                           sizeof(struct dev_port_stats));
        ulPortIndex++;
    }

    NdisReleaseSpinLock(&AdapterListLock);

cleanup:

    if (pSystemBuffer != NULL) {
        MmUnlockPages(pMdl);
        IoFreeMdl(pMdl);
    }

    return ntStatus;
}

NDIS_STATUS
GetMgmtStats(ULONG BufferLength, void *Buffer, ULONG *BytesReturned)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    MDL *pMdl = NULL;
    void *pSystemBuffer = NULL;
    ULONG ulPortIndex = 0;
    struct ionic *ionic = NULL;

    if (BufferLength < sizeof(struct mgmt_port_stats)) {
        ntStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
        goto cleanup;
    }

    //
    // Lock down the memory since we will be accessing it at raised IRQL
    //

    pSystemBuffer = LockBuffer(Buffer, BufferLength, &pMdl);

    if (pSystemBuffer == NULL) {
        ntStatus = NDIS_STATUS_RESOURCES;
        goto cleanup;
    }

    NdisAcquireSpinLock(&AdapterListLock);

    ionic = (struct ionic *)AdapterList.Flink;

    ntStatus = NDIS_STATUS_INVALID_PARAMETER;

    while (ulPortIndex < (ULONG)port_count) {

        if (ionic->pci_config.DeviceID ==
            PCI_DEVICE_ID_PENSANDO_IONIC_ETH_MGMT) {

            NdisMoveMemory(Buffer, &ionic->idev.port_info->stats,
                           sizeof(struct mgmt_port_stats));

            ntStatus = NDIS_STATUS_SUCCESS;
            *BytesReturned = sizeof(struct mgmt_port_stats);

            break;
        }

        ionic = (struct ionic *)ionic->list_entry.Flink;
        ulPortIndex++;
    }

    NdisReleaseSpinLock(&AdapterListLock);

cleanup:

    if (pSystemBuffer != NULL) {
        MmUnlockPages(pMdl);
        IoFreeMdl(pMdl);
    }

    return ntStatus;
}

NDIS_STATUS
GetPortStats(ULONG Port, ULONG BufferLength, void *Buffer, ULONG *BytesReturned)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    MDL *pMdl = NULL;
    void *pSystemBuffer = NULL;
    ULONG ulPortIndex = 0;
    struct ionic *ionic = NULL;

    if (BufferLength < sizeof(struct port_stats)) {
        ntStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
        goto cleanup;
    }

    //
    // Lock down the memory since we will be accessing it at raised IRQL
    //

    pSystemBuffer = LockBuffer(Buffer, BufferLength, &pMdl);

    if (pSystemBuffer == NULL) {
        ntStatus = NDIS_STATUS_RESOURCES;
        goto cleanup;
    }

    NdisAcquireSpinLock(&AdapterListLock);

    ionic = (struct ionic *)AdapterList.Flink;

    ntStatus = NDIS_STATUS_INVALID_PARAMETER;

    while (ulPortIndex < (ULONG)port_count) {

        if (ulPortIndex == Port) {

            NdisMoveMemory(Buffer, &ionic->idev.port_info->stats,
                           sizeof(struct port_stats));

            ntStatus = NDIS_STATUS_SUCCESS;
            *BytesReturned = sizeof(struct port_stats);

            break;
        }

        ionic = (struct ionic *)ionic->list_entry.Flink;
        ulPortIndex++;
    }

    NdisReleaseSpinLock(&AdapterListLock);

cleanup:

    if (pSystemBuffer != NULL) {
        MmUnlockPages(pMdl);
        IoFreeMdl(pMdl);
    }

    return ntStatus;
}

NDIS_STATUS
GetLifStats(ULONG PortLif,
            ULONG BufferLength,
            void *Buffer,
            ULONG *BytesReturned)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    USHORT usPort = 0;
    USHORT usLif = 0;
    MDL *pMdl = NULL;
    void *pSystemBuffer = NULL;
    ULONG ulPortIndex = 0;
    struct ionic *ionic = NULL;
    ULONG ulLifIndex = 0;
    LIST_ENTRY *currentEntry = NULL;
    struct lif *lif = NULL;

    usPort = (USHORT)(PortLif >> 16);
    usLif = (USHORT)PortLif;

    if (BufferLength < sizeof(struct lif_stats)) {
        ntStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
        goto cleanup;
    }

    //
    // Lock down the memory since we will be accessing it at raised IRQL
    //

    pSystemBuffer = LockBuffer(Buffer, BufferLength, &pMdl);

    if (pSystemBuffer == NULL) {
        ntStatus = NDIS_STATUS_RESOURCES;
        goto cleanup;
    }

    NdisAcquireSpinLock(&AdapterListLock);

    ntStatus = NDIS_STATUS_INVALID_PARAMETER;

    ionic = (struct ionic *)AdapterList.Flink;

    while (ulPortIndex < (ULONG)port_count) {

        if (usPort == (USHORT)ulPortIndex) {

            currentEntry = ionic->lifs.Flink;

            while (ulLifIndex < ionic->total_lif_count) {

                lif = CONTAINING_RECORD(currentEntry, struct lif, list);

                if (usLif == (USHORT)lif->index) {

                    NdisMoveMemory(Buffer, &lif->info->stats,
                                   sizeof(struct lif_stats));

                    *BytesReturned = sizeof(struct lif_stats);
                    ntStatus = NDIS_STATUS_SUCCESS;

                    break;
                }

                currentEntry = currentEntry->Flink;

                ulLifIndex++;
            }

            break;
        }

        ionic = (struct ionic *)ionic->list_entry.Flink;
        ulPortIndex++;
    }

    NdisReleaseSpinLock(&AdapterListLock);

cleanup:

    if (pSystemBuffer != NULL) {
        MmUnlockPages(pMdl);
        IoFreeMdl(pMdl);
    }

    return ntStatus;
}

NDIS_STATUS
GetPerfStats(void *Buffer, ULONG BufferLength, ULONG *BytesReturned)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    struct _PERF_MON_CB *stats = NULL;
    ULONG length = 0;

    ntStatus = get_perfmon_stats( &stats, &length);

    if (ntStatus != NDIS_STATUS_SUCCESS) {
        goto exit;
    }

    if (BufferLength < length) {
        ntStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
        *BytesReturned = length;
        goto exit;
    }

    NdisMoveMemory( Buffer,
                    stats,
                    length);

    *BytesReturned = length;

exit:
    
    if (stats != NULL) {
        NdisFreeMemoryWithTagPriority_internal( IonicDriver, stats, IONIC_STATS_TAG);
    }

    return ntStatus;
}