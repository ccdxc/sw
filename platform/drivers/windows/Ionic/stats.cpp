#include "common.h"

NTSTATUS
IoctlDevStats(PVOID buf, ULONG inlen, ULONG outlen, PULONG outbytes)
{
    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_STRING AdapterNameString = {};
    AdapterCB cb = {};
    DevStatsRespCB *resp = NULL;
    struct ionic *ionic = NULL;
    bool found = false;

    *outbytes = 0;

    if (inlen < sizeof(cb)) {
        return NDIS_STATUS_BUFFER_TOO_SHORT;
    }

    if (outlen < sizeof(*resp)) {
        return NDIS_STATUS_BUFFER_TOO_SHORT;
    }

    cb = *(AdapterCB *)buf;
    resp = (DevStatsRespCB *)buf;
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

        // ioctl response may be for more than one adapter, if it fits
        if (outlen < *outbytes + sizeof(*resp)) {
            ntStatus = NDIS_STATUS_BUFFER_OVERFLOW;
            break;
        }

        wcscpy_s(resp->adapter_name, ADAPTER_NAME_MAX_SZ, ionic->name.Buffer);

        NdisMoveMemory(&resp->stats, &ionic->port_stats,
                       sizeof(resp->stats));

        *outbytes += sizeof(*resp);

        ++resp;
    }

    NDIS_RELEASE_MUTEX(&AdapterListLock);

    // If specific device was requested but not found
    if (AdapterNameString.Length != 0 && !found) {
        ntStatus = NDIS_STATUS_INVALID_PARAMETER;
    }

    return ntStatus;
}

NTSTATUS
IoctlMgmtStats(PVOID buf, ULONG inlen, ULONG outlen, PULONG outbytes)
{
    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_STRING AdapterNameString = {};
    AdapterCB cb = {};
    MgmtStatsRespCB *resp = NULL;
    struct ionic *ionic = NULL;
    bool found = false;

    *outbytes = 0;

    if (inlen < sizeof(cb)) {
        return NDIS_STATUS_BUFFER_TOO_SHORT;
    }

    if (outlen < sizeof(*resp)) {
        return NDIS_STATUS_BUFFER_TOO_SHORT;
    }

    cb = *(AdapterCB *)buf;
    resp = (MgmtStatsRespCB *)buf;
    InitAdapterNameString(&AdapterNameString, cb.AdapterName);

    NdisZeroMemory(buf, outlen);

    PAGED_CODE();
    NDIS_WAIT_FOR_MUTEX(&AdapterListLock);

    ListForEachEntry(ionic, &AdapterList, struct ionic, list_entry) {
        if (ionic->pci_config.DeviceID != PCI_DEVICE_ID_PENSANDO_IONIC_ETH_MGMT) {
            continue;
        }
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
            ntStatus = NDIS_STATUS_BUFFER_OVERFLOW;
            break;
        }

        wcscpy_s(resp->adapter_name, ADAPTER_NAME_MAX_SZ, ionic->name.Buffer);

        NdisMoveMemory(&resp->stats, &ionic->idev.port_info->stats,
                       sizeof(resp->stats));

        *outbytes = sizeof(*resp);

        ++resp;
    }

    NDIS_RELEASE_MUTEX(&AdapterListLock);

    // If specific device was requested but not found
    if (AdapterNameString.Length != 0 && !found) {
        ntStatus = NDIS_STATUS_INVALID_PARAMETER;
    }

    return ntStatus;
}

NTSTATUS
IoctlPortStats(PVOID buf, ULONG inlen, ULONG outlen, PULONG outbytes)
{
    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_STRING AdapterNameString = {};
    AdapterCB cb = {};
    PortStatsRespCB *resp = NULL;
    struct ionic *ionic = NULL;
    bool found = false;

    *outbytes = 0;

    if (inlen < sizeof(cb)) {
        return NDIS_STATUS_BUFFER_TOO_SHORT;
    }

    if (outlen < sizeof(*resp)) {
        return NDIS_STATUS_BUFFER_TOO_SHORT;
    }

    cb = *(AdapterCB *)buf;
    resp = (PortStatsRespCB *)buf;
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
            ntStatus = NDIS_STATUS_BUFFER_OVERFLOW;
            break;
        }

        wcscpy_s(resp->adapter_name, ADAPTER_NAME_MAX_SZ, ionic->name.Buffer);

        NdisMoveMemory(&resp->stats, &ionic->idev.port_info->stats,
                       sizeof(resp->stats));

        *outbytes = sizeof(PortStatsRespCB);

        ++resp;
    }

    NDIS_RELEASE_MUTEX(&AdapterListLock);

    // If specific device was requested but not found
    if (AdapterNameString.Length != 0 && !found) {
        ntStatus = NDIS_STATUS_INVALID_PARAMETER;
    }

    return ntStatus;
}

NTSTATUS
IoctlLifStats(PVOID buf, ULONG inlen, ULONG outlen, PULONG outbytes)
{
    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_STRING AdapterNameString = {};
    AdapterCB cb = {};
    LifStatsRespCB *resp = NULL;
    struct ionic *ionic = NULL;
    struct lif *lif = NULL;
    bool found = false;

    *outbytes = 0;

    if (inlen < sizeof(cb)) {
        return NDIS_STATUS_BUFFER_TOO_SHORT;
    }

    if (outlen < sizeof(*resp)) {
        return NDIS_STATUS_BUFFER_TOO_SHORT;
    }

    cb = *(AdapterCB *)buf;
    resp = (LifStatsRespCB *)buf;
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
            ntStatus = NDIS_STATUS_BUFFER_OVERFLOW;
            break;
        }

        wcscpy_s(resp->adapter_name, ADAPTER_NAME_MAX_SZ, ionic->name.Buffer);

        ListForEachEntry(lif, &ionic->lifs, struct lif, list) {
            if (lif->index != cb.Index) {
                continue;
            }

            resp->lif_index = lif->index;

            NdisMoveMemory(&resp->stats, &lif->info->stats,
                           sizeof(struct lif_stats));

            // ioctl response is for one lif per adapter
            break;
        }

        *outbytes += sizeof(*resp);

        ++resp;
    }

    NDIS_RELEASE_MUTEX(&AdapterListLock);

    // If specific device was requested but not found
    if (AdapterNameString.Length != 0 && !found) {
        ntStatus = NDIS_STATUS_INVALID_PARAMETER;
    }

    return ntStatus;
}

NTSTATUS
IoctlPerfStats(PVOID buf, ULONG inlen, ULONG outlen, PULONG outbytes)
{
    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_STRING AdapterNameString = {};
    AdapterCB cb = {};
    struct _PERF_MON_CB *stats = NULL;

    *outbytes = 0;

    if (inlen < sizeof(cb)) {
        return NDIS_STATUS_BUFFER_TOO_SHORT;
    }

    if (outlen < sizeof(*stats)) {
        return NDIS_STATUS_BUFFER_TOO_SHORT;
    }

    cb = *(AdapterCB *)buf;
    InitAdapterNameString(&AdapterNameString, cb.AdapterName);

    NdisZeroMemory(buf, outlen);

    ntStatus = get_perfmon_stats(&cb, outlen, &stats, outbytes, IONIC_PERF_MON_ALL_STATS);

    if (stats != NULL) {
        NdisMoveMemory(buf, stats, *outbytes);
        NdisFreeMemoryWithTagPriority_internal( IonicDriver, stats, IONIC_STATS_TAG);
    }

    return ntStatus;
}
