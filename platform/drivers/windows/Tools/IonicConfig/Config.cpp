
#include "windows.h"
#include "stdio.h"
#include <shlwapi.h>
#include <winioctl.h>

#include "UserCommon.h"

#include "pci_ids.h"

#include "ionic_types.h"
#include "ionic_stats.h"

void
DumpDevStats( void *Stats);

void
DumpMgmtStats( void *Stats);

void
DumpPortStats( DWORD Port, void *Stats);

void
DumpLifStats( DWORD Lif, void *Stats);

char *
GetLifTypeName(ULONG type);

void
DumpPerfStats( void *Stats);

int
UninstallMiniport(void);

int
InstallMiniport(WCHAR *InfFile);

int
UpdateMiniport(WCHAR *InfFile);

int 
client( char *ServerAddr);

int 
server(void);

void
help()
{

    printf("IonicConfig -level <Trace Level> -component <Trace Component> -length <Buffer size in KB> -gettrace (Get curren trace buffer) -stats <dev | perf | port # | lif # | reset > -install <INF File> -uninstall -update <INF File> -rxbudget <value> \n");

    return;
}

int 
_cdecl
wmain(int argc, WCHAR* argv[])
{

    HANDLE    hDevice = NULL;
    HRESULT    hr = S_OK;
    DWORD    dwIndex = 1;
    DWORD    dwLevel = -1;
    DWORD    dwComponent = -1;
    DWORD    dwLength = -1;
    DWORD    dwFlags = -1;
    BOOLEAN bGetTrace = FALSE;
    char   *pTraceBuffer = NULL;
    DWORD    dwBytesReturned = 0;
    TraceConfigCB stTrace;
    BOOLEAN    bStats = FALSE;
    BOOLEAN bDevStats = FALSE;
    BOOLEAN bPerfStats = FALSE;
    BOOLEAN bMgmtStats = FALSE;
    BOOLEAN bPortStats = FALSE;
    BOOLEAN bLifStats = FALSE;
    BOOLEAN bResetStats = FALSE;
    DWORD   dwPort = 0;
    DWORD    dwLif = 0;
    void   *pStatsBuffer = NULL;
    DWORD    dwStatsBufferLen = 0;
    DWORD   dwPortLif = 0;
    BOOLEAN bInstall = FALSE;
    WCHAR    wchInfFile[ 255];
    BOOLEAN bUninstall = FALSE;
    BOOLEAN bUpdate = FALSE;
    BOOLEAN bRunClient = FALSE;
    char    chServer[255];
    BOOLEAN bRunServer = FALSE;
    ULONG    rx_budget = 0;
    BOOLEAN bSetRxBudget = FALSE;
    BOOLEAN bSetTrace = FALSE;

    if( argc == 1 )
    {
        help();
        return 0;
    }

    while( dwIndex < (DWORD)argc )
    {

        if( wcscmp( argv[ dwIndex], L"-level") == 0)
        {

            if( !StrToIntExW( argv[ ++dwIndex],
                             STIF_SUPPORT_HEX,
                             (int *)&dwLevel))
            {
                printf("Failed to parse parameter %S\n", argv[ dwIndex]);
                goto cleanup;
            }

            bSetTrace = TRUE;
        }
        else if( wcscmp( argv[dwIndex], L"-component" ) == 0 )
        {

            if( !StrToIntExW( argv[ ++dwIndex],
                             STIF_SUPPORT_HEX,
                             (int *)&dwComponent))
            {
                printf("Failed to parse parameter %S\n", argv[ dwIndex]);
                goto cleanup;
            }

            bSetTrace = TRUE;
        }
        else if( wcscmp( argv[dwIndex], L"-length" ) == 0 )
        {

            if( !StrToIntExW( argv[ ++dwIndex],
                             STIF_SUPPORT_HEX,
                             (int *)&dwLength))
            {
                printf("Failed to parse parameter %S\n", argv[ dwIndex]);
                goto cleanup;
            }

            if (dwLength > MAXIMUM_TRACE_BUFFER_SIZE) {
                printf("Buffer length cannot not exceed %d\n", MAXIMUM_TRACE_BUFFER_SIZE);
                goto cleanup;
            }

            bSetTrace = TRUE;
        }
        else if( wcscmp( argv[dwIndex], L"-flags" ) == 0 )
        {

            if( !StrToIntExW( argv[ ++dwIndex],
                             STIF_SUPPORT_HEX,
                             (int *)&dwFlags))
            {
                printf("Failed to parse parameter %S\n", argv[ dwIndex]);
                goto cleanup;
            }

            bSetTrace = TRUE;
        }
        else if( wcscmp( argv[dwIndex], L"-gettrace" ) == 0 )
        {
            bGetTrace = TRUE;
        }
        else if( wcscmp( argv[dwIndex], L"-install" ) == 0 )
        {

            dwIndex++;

            if (dwIndex >= (DWORD)argc) {
                printf("Must indicate full inf file location\n");
                goto cleanup;
            }

            bInstall = TRUE;
            wcscpy_s( wchInfFile, 255, argv[ dwIndex]);
        }
        else if( wcscmp( argv[dwIndex], L"-uninstall" ) == 0 )
        {
            bUninstall = TRUE;
        }
        else if( wcscmp( argv[dwIndex], L"-update" ) == 0 )
        {

            dwIndex++;

            if (dwIndex >= (DWORD)argc) {
                printf("Must indicate full inf file location\n");
                goto cleanup;
            }

            wcscpy_s( wchInfFile, 255, argv[ dwIndex]);
            bUpdate = TRUE;
        }
        else if( wcscmp( argv[dwIndex], L"-stats" ) == 0 )
        {

            dwIndex++;

            if (dwIndex >= (DWORD)argc) {
                printf("Must indicate types of stats to retrieve\n");
                goto cleanup;
            }

            if (wcscmp(argv[dwIndex], L"dev") == 0) {
                bDevStats = TRUE;
            }
            else if (wcscmp(argv[dwIndex], L"perf") == 0) {
                bPerfStats = TRUE;
            }
            else if (wcscmp(argv[dwIndex], L"reset") == 0) {
                bResetStats = TRUE;
            }
            else if (wcscmp(argv[dwIndex], L"mngmnt") == 0) {
                bMgmtStats = TRUE;
            }
            else if (wcscmp(argv[dwIndex], L"port") == 0) {

                if( !StrToIntExW( argv[ ++dwIndex],
                                 STIF_SUPPORT_HEX,
                                 (int *)&dwPort))
                {
                    printf("Failed to parse parameter %S\n", argv[ dwIndex]);
                    goto cleanup;
                }

                dwIndex++;

                if( argc == dwIndex) {
                    bPortStats = TRUE;
                }
                else if (wcscmp(argv[dwIndex], L"lif") == 0) {

                    if( !StrToIntExW( argv[ ++dwIndex],
                                     STIF_SUPPORT_HEX,
                                     (int *)&dwLif))
                    {
                        printf("Failed to parse parameter %S\n", argv[ dwIndex]);
                        goto cleanup;
                    }

                    bLifStats = TRUE;
                }
                else {
                    help();
                    goto cleanup;
                }
            }
            else {
                help();
                goto cleanup;
            }

            bStats = TRUE;
        }
        else if( wcscmp( argv[dwIndex], L"-client" ) == 0 )
        {

            dwIndex++;

            if (dwIndex >= (DWORD)argc) {
                printf("Must indicate server address\n");
                goto cleanup;
            }

            bRunClient = TRUE;
            sprintf_s( chServer, 255, "%S", argv[ dwIndex]);
        }
        else if( wcscmp( argv[dwIndex], L"-server" ) == 0 )
        {

            bRunServer = TRUE;
        }
        else if( wcscmp( argv[ dwIndex], L"-rxbudget") == 0)
        {

            if( !StrToIntExW( argv[ ++dwIndex],
                             STIF_SUPPORT_HEX,
                             (int *)&rx_budget))
            {
                printf("Failed to parse parameter %S\n", argv[ dwIndex]);
                goto cleanup;
            }

            bSetRxBudget = TRUE;
        }
        else
        {
            help();
            goto cleanup;
        }

        dwIndex++;
    }

    if (bInstall) {    
        InstallMiniport(wchInfFile);
        goto cleanup;
    }
    else if (bUninstall) {    
        UninstallMiniport();
        goto cleanup;
    }
    else if (bUpdate) {
        UpdateMiniport( wchInfFile);
        goto cleanup;
    }
    else if (bRunClient) {
        printf("Running client with server %s\n", chServer);
        client( chServer);
        goto cleanup;
    }
    else if (bRunServer) {
        printf("Running server\n");
        server();
        goto cleanup;
    }

    hDevice = CreateFile( IONIC_LINKNAME_STRING_USER,
                          GENERIC_READ | GENERIC_WRITE,
                          0,
                          NULL,
                          OPEN_EXISTING,
                          0,
                          NULL);

    if (hDevice == INVALID_HANDLE_VALUE) {
        printf("IonicConfig Failed to open adapter Error %d\n", GetLastError());
        hDevice = NULL;
        goto cleanup;
    }

    if (bStats) {

        if (bResetStats) {

            if( !DeviceIoControl( hDevice,
                                  IOCTL_IONIC_RESET_STATS,
                                  NULL,
                                  0,
                                  NULL,
                                  0,
                                  &dwBytesReturned,
                                  NULL))
            {
                printf("Failed to call IOCTL_IONIC_RESET_STATS Error %d", GetLastError());
                goto cleanup;
            }

            printf("Successfully reset stats\n");

            goto cleanup;
        }

        dwStatsBufferLen = (10 * 1024 * 1024);

        pStatsBuffer = malloc( dwStatsBufferLen);

        if (pStatsBuffer == NULL) {
            printf("Failed to allocate stats buffer\n");
            goto cleanup;
        }

        if (bDevStats) {

            if( !DeviceIoControl( hDevice,
                                  IOCTL_IONIC_GET_DEV_STATS,
                                  NULL,
                                  0,
                                  pStatsBuffer,
                                  dwStatsBufferLen,
                                  &dwBytesReturned,
                                  NULL))
            {
                printf("Failed to call IOCTL_IONIC_GET_DEV_STATS Error %d", GetLastError());
                goto cleanup;
            }

            DumpDevStats( pStatsBuffer);
        }
        else if (bPerfStats) {

            if( !DeviceIoControl( hDevice,
                                  IOCTL_IONIC_GET_PERF_STATS,
                                  NULL,
                                  0,
                                  pStatsBuffer,
                                  dwStatsBufferLen,
                                  &dwBytesReturned,
                                  NULL))
            {
                printf("Failed to call IOCTL_IONIC_GET_PERF_STATS Error %d", GetLastError());
                goto cleanup;
            }

            DumpPerfStats( pStatsBuffer);
        }
        else if (bMgmtStats) {

            if( !DeviceIoControl( hDevice,
                                  IOCTL_IONIC_GET_MGMT_STATS,
                                  NULL,
                                  0,
                                  pStatsBuffer,
                                  0x1000,
                                  &dwBytesReturned,
                                  NULL))
            {
                printf("Failed to call IOCTL_IONIC_GET_MGMT_STATS Error %d", GetLastError());
                goto cleanup;
            }

            DumpMgmtStats( pStatsBuffer);
        }
        else if (bPortStats) {

            if( !DeviceIoControl( hDevice,
                                  IOCTL_IONIC_GET_PORT_STATS,
                                  &dwPort,
                                  sizeof( DWORD),
                                  pStatsBuffer,
                                  0x1000,
                                  &dwBytesReturned,
                                  NULL))
            {
                printf("Failed to call IOCTL_IONIC_GET_PORT_STATS Error %d", GetLastError());
                goto cleanup;
            }

            DumpPortStats( dwPort, pStatsBuffer);
        }
        else if( bLifStats) {

            dwPortLif = (DWORD)((dwPort << 16) | (USHORT)dwLif);

            if( !DeviceIoControl( hDevice,
                                  IOCTL_IONIC_GET_LIF_STATS,
                                  &dwPortLif,
                                  sizeof( DWORD),
                                  pStatsBuffer,
                                  0x1000,
                                  &dwBytesReturned,
                                  NULL))
            {
                printf("Failed to call IOCTL_IONIC_GET_LIF_STATS Error %d", GetLastError());
                goto cleanup;
            }

            DumpLifStats( dwLif, pStatsBuffer);
        }

        goto cleanup;
    }

    if (bGetTrace) {

        pTraceBuffer = (char *)malloc( MAXIMUM_TRACE_BUFFER_SIZE * 1024);

        if (pTraceBuffer == NULL) {
            printf("Failed to allocate trace buffer\n");
            goto cleanup;
        }
        
        if( !DeviceIoControl( hDevice,
                              IOCTL_IONIC_GET_TRACE,
                              NULL,
                              0,
                              pTraceBuffer,
                              (10 * 1024 * 1024),
                              &dwBytesReturned,
                              NULL ) )
        {
            printf("Failed to call IOCTL_IONIC_GET_TRACE Error %d", GetLastError());
            goto cleanup;
        }

        printf(pTraceBuffer);

        goto cleanup;
    }

    if( bSetTrace) {

        stTrace.Component = dwComponent;
        stTrace.Level = dwLevel;
        stTrace.TraceBufferLength = dwLength;
        stTrace.TraceFlags = dwFlags;
        
        if( !DeviceIoControl( hDevice,
                                IOCTL_IONIC_CONFIGURE_TRACE,
                                &stTrace,
                                sizeof( TraceConfigCB),
                                NULL,
                                0,
                                &dwBytesReturned,
                                NULL ) )
        {
            printf("Failed to call IOCTL_IONIC_CONFIGURE_TRACE Error %d", GetLastError());
            goto cleanup;
        }

        printf("Successfully configured trace\n");
    }

    if (bSetRxBudget) {
    
        if( !DeviceIoControl( hDevice,
                                IOCTL_IONIC_SET_RX_BUDGET,
                                &rx_budget,
                                sizeof( ULONG),
                                NULL,
                                0,
                                &dwBytesReturned,
                                NULL ) )
        {
            printf("Failed to call IOCTL_IONIC_SET_RX_BUDGET Error %d", GetLastError());
            goto cleanup;
        }

        printf("Successfully set rx budget to %d\n", rx_budget);
    }

cleanup:

    if (hDevice != NULL){
        CloseHandle( hDevice);
    }

    if (pTraceBuffer != NULL) {
        free( pTraceBuffer);
    }

    return 0;
}

void
DumpDevStats(void *Stats)
{
       
    struct dev_port_stats    *dev_stats = (struct dev_port_stats *)Stats;
    ULONG ulPortCnt = 0;
    ULONG ulLifCount = 0;
    ULONG ulRxCnt = 0;
    ULONG ulTxCnt = 0;

    while( dev_stats->device_id != 0)
    {
        printf("Port %d Mgmt %s RSS %s VMQ %s SRIOV %s Up %d Dwn %d\n", 
                        ulPortCnt, 
                        (dev_stats->device_id == PCI_DEVICE_ID_PENSANDO_IONIC_ETH_MGMT)?"Yes":"No",
                        (dev_stats->flags & IONIC_PORT_FLAG_RSS)?"Yes":"No",
                        (dev_stats->flags & IONIC_PORT_FLAG_VMQ)?"Yes":"No",
                        (dev_stats->flags & IONIC_PORT_FLAG_SRIOV)?"Yes":"No",
                        (ULONG)dev_stats->link_up,
                        (ULONG)dev_stats->link_dn);

        printf("\tVendor %04lX\n", dev_stats->vendor_id);
        printf("\tDevice %04lX\n", dev_stats->device_id);
        printf("\tLif Count %d\n", dev_stats->lif_count);

        ulLifCount = 0;
        while (ulLifCount < dev_stats->lif_count)
        {

            printf("\tLif %d\n", dev_stats->lif_stats[ ulLifCount].lif_id);
            printf("\t\tLif type: %s", GetLifTypeName( dev_stats->lif_stats[ ulLifCount].lif_type));

            printf("\t\tLif name: %s\n", dev_stats->lif_stats[ ulLifCount].lif_name);

            printf("\t\tRx Count: %d\n", dev_stats->lif_stats[ ulLifCount].rx_count);
            printf("\t\tTx Count: %d\n", dev_stats->lif_stats[ ulLifCount].tx_count);

            ulRxCnt = 0;
            while (ulRxCnt < dev_stats->lif_stats[ulLifCount].rx_count)
            {

                printf("\t\t\tRx %d Msi %d directed bytes 0x%I64X directed packets 0x%I64X Comp 0x%I64X Vlan 0x%I64X\n",
                                ulRxCnt,
                                dev_stats->lif_stats[ulLifCount].rx_ring[ ulRxCnt].msi_id,
                                dev_stats->lif_stats[ulLifCount].rx_ring[ ulRxCnt].directed_bytes,
                                dev_stats->lif_stats[ulLifCount].rx_ring[ ulRxCnt].directed_packets,
                                dev_stats->lif_stats[ulLifCount].rx_ring[ ulRxCnt].completion_count,
                                dev_stats->lif_stats[ulLifCount].rx_ring[ ulRxCnt].vlan_stripped);

                ulRxCnt++;
            }

            ulTxCnt = 0;
            while (ulTxCnt < dev_stats->lif_stats[ulLifCount].tx_count)
            {

                printf("\t\t\tTx %d directed bytes 0x%I64X directed packets 0x%I64X tso bytes 0x%I64X tso packets 0x%I64X db cnt 0x%I64X vlan 0x%I64X\n",
                            ulTxCnt,
                            dev_stats->lif_stats[ulLifCount].tx_ring[ ulTxCnt].directed_bytes,
                            dev_stats->lif_stats[ulLifCount].tx_ring[ ulTxCnt].directed_packets,
                            dev_stats->lif_stats[ulLifCount].tx_ring[ ulTxCnt].tso_bytes,
                            dev_stats->lif_stats[ulLifCount].tx_ring[ ulTxCnt].tso_packets,
                            dev_stats->lif_stats[ulLifCount].tx_ring[ ulTxCnt].doorbell_count,
                            dev_stats->lif_stats[ulLifCount].tx_ring[ ulTxCnt].vlan_inserted);

                ulTxCnt++;
            }

            ulLifCount++;
        }

        dev_stats = (struct dev_port_stats *)((char *)dev_stats + sizeof( struct dev_port_stats));
        ulPortCnt++;
    }

    return;
}

void
DumpMgmtStats(void *Stats)
{
    
    struct mgmt_port_stats *mgmt_stats = (struct mgmt_port_stats *)Stats;

    printf("Mgmt Port Stats\n");

    printf("\tRx frames all: 0x%I64X\n", mgmt_stats->frames_rx_all);


    return;
}

void
DumpPortStats(DWORD Port, void *Stats)
{
    
    struct port_stats *port_stats = (struct port_stats *)Stats;
    ULONGLONG ull9216Frames = 0;

    printf("Port %d Stats\n", Port);

    printf("\tRx frames ok:\t\t0x%I64X\n", port_stats->frames_rx_ok);
    printf("\tRx frames all:\t\t0x%I64X\n", port_stats->frames_rx_all);
    printf("\tRx fcs errors:\t\t0x%I64X\n", port_stats->frames_rx_bad_fcs);
    printf("\tRx crc errors:\t\t0x%I64X\n", port_stats->frames_rx_bad_all);
    printf("\tRx bytes ok:\t\t0x%I64X\n", port_stats->octets_rx_ok);
    printf("\tRx bytes all:\t\t0x%I64X\n", port_stats->octets_rx_all);
    printf("\tRx u-cast:\t\t0x%I64X\n", port_stats->frames_rx_unicast);
    printf("\tRx m-cast:\t\t0x%I64X\n", port_stats->frames_rx_multicast);
    printf("\tRx b-cast:\t\t0x%I64X\n", port_stats->frames_rx_broadcast);
    printf("\tRx pause:\t\t0x%I64X\n", port_stats->frames_rx_pause);
    printf("\tRx bad length:\t\t0x%I64X\n", port_stats->frames_rx_bad_length);
    printf("\tRx undersized:\t\t0x%I64X\n", port_stats->frames_rx_undersized);
    printf("\tRx oversized:\t\t0x%I64X\n", port_stats->frames_rx_oversized);
    printf("\tRx fragments:\t\t0x%I64X\n", port_stats->frames_rx_fragments);
    printf("\tRx jabber:\t\t0x%I64X\n", port_stats->frames_rx_jabber);
    printf("\tRx pfc frames:\t\t0x%I64X\n", port_stats->frames_rx_pripause);
    printf("\tRx crc stomped:\t\t0x%I64X\n", port_stats->frames_rx_stomped_crc);
    printf("\tRx too long:\t\t0x%I64X\n", port_stats->frames_rx_too_long);
    printf("\tRx vlan ok:\t\t0x%I64X\n", port_stats->frames_rx_vlan_good);
    printf("\tRx dropped frames:\t0x%I64X\n", port_stats->frames_rx_dropped);
    printf("\tRx frames < 64:\t\t0x%I64X\n", port_stats->frames_rx_less_than_64b);
    printf("\tRx frames = 64:\t\t0x%I64X\n", port_stats->frames_rx_64b);
    printf("\tRx frames 65-127:\t0x%I64X\n", port_stats->frames_rx_65b_127b);
    printf("\tRx frames 128-255:\t0x%I64X\n", port_stats->frames_rx_128b_255b);
    printf("\tRx frames 256-511:\t0x%I64X\n", port_stats->frames_rx_256b_511b);
    printf("\tRx frames 512-1023:\t0x%I64X\n", port_stats->frames_rx_512b_1023b);
    printf("\tRx frames 1024-1518:\t0x%I64X\n", port_stats->frames_rx_1024b_1518b);
    printf("\tRx frames 1519-2047:\t0x%I64X\n", port_stats->frames_rx_1519b_2047b);
    printf("\tRx frames 2048-4095:\t0x%I64X\n", port_stats->frames_rx_2048b_4095b);
    printf("\tRx frames 4096-8191:\t0x%I64X\n", port_stats->frames_rx_4096b_8191b);
    printf("\tRx frames 8192-9215:\t0x%I64X\n", port_stats->frames_rx_8192b_9215b);

    ull9216Frames = port_stats->frames_rx_all - ( port_stats->frames_rx_dropped +
                                                    port_stats->frames_rx_less_than_64b +
                                                    port_stats->frames_rx_64b +
                                                    port_stats->frames_rx_65b_127b +
                                                    port_stats->frames_rx_128b_255b +
                                                    port_stats->frames_rx_256b_511b +
                                                    port_stats->frames_rx_512b_1023b +
                                                    port_stats->frames_rx_1024b_1518b +
                                                    port_stats->frames_rx_1519b_2047b +
                                                    port_stats->frames_rx_2048b_4095b +
                                                    port_stats->frames_rx_4096b_8191b +
                                                    port_stats->frames_rx_8192b_9215b);

    printf("\tRx frames >= 9216:\t0x%I64X\n", ull9216Frames);

    printf("\tTx frames ok:\t\t0x%I64X\n", port_stats->frames_tx_ok);
    printf("\tTx frames all:\t\t0x%I64X\n", port_stats->frames_tx_all);
    printf("\tTx frames bad:\t\t0x%I64X\n", port_stats->frames_tx_bad);
    printf("\tTx bytes ok:\t\t0x%I64X\n", port_stats->octets_tx_ok);
    printf("\tTx bytes all:\t\t0x%I64X\n", port_stats->octets_tx_total);
    printf("\tTx u-cast:\t\t0x%I64X\n", port_stats->frames_tx_unicast);
    printf("\tTx m-cast:\t\t0x%I64X\n", port_stats->frames_tx_multicast);
    printf("\tTx b-cast:\t\t0x%I64X\n", port_stats->frames_tx_broadcast);
    printf("\tTx pause frames:\t0x%I64X\n", port_stats->frames_tx_pause);
    printf("\tTx pfc frames:\t\t0x%I64X\n", port_stats->frames_tx_pripause);
    printf("\tTx vlan frames:\t\t0x%I64X\n", port_stats->frames_tx_vlan);
    printf("\tTx frames < 64:\t\t0x%I64X\n", port_stats->frames_tx_less_than_64b);
    printf("\tTx frames = 64:\t\t0x%I64X\n", port_stats->frames_tx_64b);
    printf("\tTx frames 65-127:\t0x%I64X\n", port_stats->frames_tx_65b_127b);
    printf("\tTx frames 128-255:\t0x%I64X\n", port_stats->frames_tx_128b_255b);
    printf("\tTx frames 256-511:\t0x%I64X\n", port_stats->frames_tx_256b_511b);
    printf("\tTx frames 512-1023:\t0x%I64X\n", port_stats->frames_tx_512b_1023b);
    printf("\tTx frames 1024-1518:\t0x%I64X\n", port_stats->frames_tx_1024b_1518b);
    printf("\tTx frames 1519-2047:\t0x%I64X\n", port_stats->frames_tx_1519b_2047b);
    printf("\tTx frames 2048-4095:\t0x%I64X\n", port_stats->frames_tx_2048b_4095b);
    printf("\tTx frames 4096-8191:\t0x%I64X\n", port_stats->frames_tx_4096b_8191b);
    printf("\tTx frames 8192-9215:\t0x%I64X\n", port_stats->frames_tx_8192b_9215b);

    ull9216Frames = port_stats->frames_tx_all - ( port_stats->frames_tx_less_than_64b +
                                                    port_stats->frames_tx_64b +
                                                    port_stats->frames_tx_65b_127b +
                                                    port_stats->frames_tx_128b_255b +
                                                    port_stats->frames_tx_256b_511b +
                                                    port_stats->frames_tx_512b_1023b +
                                                    port_stats->frames_tx_1024b_1518b +
                                                    port_stats->frames_tx_1519b_2047b +
                                                    port_stats->frames_tx_2048b_4095b +
                                                    port_stats->frames_tx_4096b_8191b +
                                                    port_stats->frames_tx_8192b_9215b);

    printf("\tTx frames >= 9216:\t0x%I64X\n", ull9216Frames);

    printf("\tTx pri-0:\t\t0x%I64X\n", port_stats->frames_tx_pri_0);
    printf("\tTx pri-1:\t\t0x%I64X\n", port_stats->frames_tx_pri_1);
    printf("\tTx pri-2:\t\t0x%I64X\n", port_stats->frames_tx_pri_2);
    printf("\tTx pri-3:\t\t0x%I64X\n", port_stats->frames_tx_pri_3);
    printf("\tTx pri-4:\t\t0x%I64X\n", port_stats->frames_tx_pri_4);
    printf("\tTx pri-5:\t\t0x%I64X\n", port_stats->frames_tx_pri_5);
    printf("\tTx pri-6:\t\t0x%I64X\n", port_stats->frames_tx_pri_6);
    printf("\tTx pri-7:\t\t0x%I64X\n", port_stats->frames_tx_pri_7);

    printf("\tRx pri-0:\t\t0x%I64X\n", port_stats->frames_rx_pri_0);
    printf("\tRx pri-1:\t\t0x%I64X\n", port_stats->frames_rx_pri_1);
    printf("\tRx pri-2:\t\t0x%I64X\n", port_stats->frames_rx_pri_2);
    printf("\tRx pri-3:\t\t0x%I64X\n", port_stats->frames_rx_pri_3);
    printf("\tRx pri-4:\t\t0x%I64X\n", port_stats->frames_rx_pri_4);
    printf("\tRx pri-5:\t\t0x%I64X\n", port_stats->frames_rx_pri_5);
    printf("\tRx pri-6:\t\t0x%I64X\n", port_stats->frames_rx_pri_6);
    printf("\tRx pri-7:\t\t0x%I64X\n", port_stats->frames_rx_pri_7);

    printf("\tTx pri-0 pause:\t\t0x%I64X\n", port_stats->tx_pripause_0_1us_count);
    printf("\tTx pri-1 pause:\t\t0x%I64X\n", port_stats->tx_pripause_1_1us_count);
    printf("\tTx pri-2 pause:\t\t0x%I64X\n", port_stats->tx_pripause_2_1us_count);
    printf("\tTx pri-3 pause:\t\t0x%I64X\n", port_stats->tx_pripause_3_1us_count);
    printf("\tTx pri-4 pause:\t\t0x%I64X\n", port_stats->tx_pripause_4_1us_count);
    printf("\tTx pri-5 pause:\t\t0x%I64X\n", port_stats->tx_pripause_5_1us_count);
    printf("\tTx pri-6 pause:\t\t0x%I64X\n", port_stats->tx_pripause_6_1us_count);
    printf("\tTx pri-7 pause:\t\t0x%I64X\n", port_stats->tx_pripause_7_1us_count);

    printf("\tRx pri-0 pause:\t\t0x%I64X\n", port_stats->rx_pripause_0_1us_count);
    printf("\tRx pri-1 pause:\t\t0x%I64X\n", port_stats->rx_pripause_1_1us_count);
    printf("\tRx pri-2 pause:\t\t0x%I64X\n", port_stats->rx_pripause_2_1us_count);
    printf("\tRx pri-3 pause:\t\t0x%I64X\n", port_stats->rx_pripause_3_1us_count);
    printf("\tRx pri-4 pause:\t\t0x%I64X\n", port_stats->rx_pripause_4_1us_count);
    printf("\tRx pri-5 pause:\t\t0x%I64X\n", port_stats->rx_pripause_5_1us_count);
    printf("\tRx pri-6 pause:\t\t0x%I64X\n", port_stats->rx_pripause_6_1us_count);
    printf("\tRx pri-7 pause:\t\t0x%I64X\n", port_stats->rx_pripause_7_1us_count);
    printf("\tRx standard pause:\t0x%I64X\n", port_stats->rx_pause_1us_count);

    printf("\tFrames truncated:\t0x%I64X\n", port_stats->frames_tx_truncated);

    return;
}

void
DumpLifStats(DWORD Lif, void *Stats)
{

    struct lif_stats *lif_stats = (struct lif_stats *)Stats;

    printf("Lif %d Stats\n", Lif);

    printf("\tRx u-cast bytes: \t\t0x%I64X\n", lif_stats->rx_ucast_bytes);
    printf("\tRx u-cast packets: \t\t0x%I64X\n", lif_stats->rx_ucast_packets);
    printf("\tRx m-cast bytes: \t\t0x%I64X\n", lif_stats->rx_mcast_bytes);
    printf("\tRx m-cast packets: \t\t0x%I64X\n", lif_stats->rx_mcast_packets);
    printf("\tRx b-cast bytes: \t\t0x%I64X\n", lif_stats->rx_bcast_bytes);
    printf("\tRx b-cast packets: \t\t0x%I64X\n", lif_stats->rx_bcast_packets);

    printf("\tRx drop u-cast bytes: \t\t0x%I64X\n", lif_stats->rx_ucast_drop_bytes);
    printf("\tRx drop u-cast packets: \t0x%I64X\n", lif_stats->rx_ucast_drop_packets);
    printf("\tRx drop m-cast bytes: \t\t0x%I64X\n", lif_stats->rx_mcast_drop_bytes);
    printf("\tRx drop m-cast packets: \t0x%I64X\n", lif_stats->rx_mcast_drop_packets);
    printf("\tRx drop b-cast bytes: \t\t0x%I64X\n", lif_stats->rx_bcast_drop_bytes);
    printf("\tRx drop b-cast packets: \t0x%I64X\n", lif_stats->rx_bcast_drop_packets);
    printf("\tRx dma error: \t\t\t0x%I64X\n", lif_stats->rx_dma_error);

    printf("\tTx u-cast bytes: \t\t0x%I64X\n", lif_stats->tx_ucast_bytes);
    printf("\tTx u-cast packets: \t\t0x%I64X\n", lif_stats->tx_ucast_packets);
    printf("\tTx m-cast bytes: \t\t0x%I64X\n", lif_stats->tx_mcast_bytes);
    printf("\tTx m-cast packets: \t\t0x%I64X\n", lif_stats->tx_mcast_packets);
    printf("\tTx b-cast bytes: \t\t0x%I64X\n", lif_stats->tx_bcast_bytes);
    printf("\tTx b-cast packets: \t\t0x%I64X\n", lif_stats->tx_bcast_packets);
    
    printf("\tTx drop u-cast bytes: \t\t0x%I64X\n", lif_stats->tx_ucast_drop_bytes);
    printf("\tTx drop u-cast packets: \t0x%I64X\n", lif_stats->tx_ucast_drop_packets);
    printf("\tTx drop m-cast bytes: \t\t0x%I64X\n", lif_stats->tx_mcast_drop_bytes);
    printf("\tTx drop m-cast packets: \t0x%I64X\n", lif_stats->tx_mcast_drop_packets);
    printf("\tTx drop b-cast bytes: \t\t0x%I64X\n", lif_stats->tx_bcast_drop_bytes);
    printf("\tTx drop b-cast packets: \t0x%I64X\n", lif_stats->tx_bcast_drop_packets);
    printf("\tTx dma error: \t\t\t0x%I64X\n", lif_stats->tx_dma_error);

    printf("\tRx queue disabled drops: \t0x%I64X\n", lif_stats->rx_queue_disabled);
    printf("\tRx queue empty drops: \t\t0x%I64X\n", lif_stats->rx_queue_empty);
    printf("\tRx queue error count: \t\t0x%I64X\n", lif_stats->rx_queue_error);
    printf("\tRx descriptor fetch errors: \t0x%I64X\n", lif_stats->rx_desc_fetch_error);
    printf("\tRx descriptor data errors: \t0x%I64X\n", lif_stats->rx_desc_data_error);

    printf("\tTx queue disabled drops: \t0x%I64X\n", lif_stats->tx_queue_disabled);
    printf("\tTx queue error count: \t\t0x%I64X\n", lif_stats->tx_queue_error);
    printf("\tTx descriptor fetch errors: \t0x%I64X\n", lif_stats->tx_desc_fetch_error);
    printf("\tTx descriptor data errors: \t0x%I64X\n", lif_stats->tx_desc_data_error);

    return;
}

char *
GetLifTypeName(ULONG type)
{

    char    *pType = (char *)"Default";

    if (type == IONIC_LIF_TYPE_VMQ) {
        pType = (char *)"VM Queue";
    }
    else if (type == IONIC_LIF_TYPE_VPORT) {
        pType = (char *)"VPort";
    }

    return pType;
}

void
DumpPerfStats(void *Stats)
{

    struct _PERF_MON_CB *perf_stats = (struct _PERF_MON_CB *)Stats;
    struct _PERF_MON_ADAPTER_STATS *adapter_stats = NULL;
    struct _PERF_MON_LIF_STATS *lif_stats = NULL;
    struct _PERF_MON_TX_QUEUE_STATS *tx_stats = NULL;
    struct _PERF_MON_RX_QUEUE_STATS *rx_stats = NULL;
    ULONG adapter_cnt = 0;
    ULONG lif_cnt = 0;
    ULONG rx_cnt = 0;
    ULONG tx_cnt = 0;

    adapter_stats = (struct _PERF_MON_ADAPTER_STATS *)((char *)perf_stats + sizeof( struct _PERF_MON_CB));

    for (adapter_cnt = 0; adapter_cnt < perf_stats->adapter_count; adapter_cnt++) {

        printf("Adapter: %S Mgmt: %s Lif cnt: %d Core redirect cnt: %d\n", 
                        adapter_stats->name,
                        adapter_stats->mgmt_device?"Yes":"No",
                        adapter_stats->lif_count,
                        adapter_stats->core_redirection_count);

        lif_stats = (struct _PERF_MON_LIF_STATS *)((char *)adapter_stats + sizeof( struct _PERF_MON_ADAPTER_STATS));

        for (lif_cnt = 0; lif_cnt < adapter_stats->lif_count; lif_cnt++) {

            printf("\tLif: %s Rx cnt: %d Tx cnt: %d\n",
                            lif_stats->name,
                            lif_stats->rx_queue_count,
                            lif_stats->tx_queue_count);

            rx_stats = (struct _PERF_MON_RX_QUEUE_STATS *)((char *)lif_stats + sizeof( struct _PERF_MON_LIF_STATS));

            for (rx_cnt = 0; rx_cnt < lif_stats->rx_queue_count; rx_cnt++) {

                printf("\t\tRx: %d :Pool cnt: %d\n",
                                rx_cnt,
                                rx_stats->rx_pool_count);

                rx_stats = (struct _PERF_MON_RX_QUEUE_STATS *)((char *)rx_stats + sizeof( struct _PERF_MON_RX_QUEUE_STATS));
            }

            printf("\n");

            tx_stats = (struct _PERF_MON_TX_QUEUE_STATS *)rx_stats;

            for (tx_cnt = 0; tx_cnt < lif_stats->tx_queue_count; tx_cnt++) {

                printf("\t\tTx: %d nblps: %d nbps %d byteps: %d pending nbl: %d pending nb: %d\n",
                                tx_cnt,
                                tx_stats->nbl_per_sec,
                                tx_stats->nb_per_sec,
                                tx_stats->byte_per_sec,
                                tx_stats->pending_nbl_count,
                                tx_stats->pending_nb_count);

                printf("\t\tqueue len: %d max: %d ttq: %lld ttc: %lld qtc: %lld nbl ttc: %lld\n",
                                tx_stats->queue_len,
                                tx_stats->max_queue_len,
                                tx_stats->nb_time_to_queue,
                                tx_stats->nb_time_to_complete,
                                tx_stats->nb_time_queue_to_comp,
                                tx_stats->nbl_time_to_complete);

                tx_stats = (struct _PERF_MON_TX_QUEUE_STATS *)((char *)tx_stats + sizeof( struct _PERF_MON_TX_QUEUE_STATS));
            }

            lif_stats = (struct _PERF_MON_LIF_STATS *)tx_stats;
            printf("\n");
        }
       
        adapter_stats = (struct _PERF_MON_ADAPTER_STATS *)lif_stats;
        printf("\n");
    }

    return;
}