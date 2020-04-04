#pragma once

#define IONIC_LINKNAME_STRING_USER			L"\\??\\IonicControl"

#define ADAPTER_NAME_MAX_SZ         50
#define LIF_NAME_MAX_SZ				32

//
// Logging subsystem defines and structres
//

#define MAXIMUM_TRACE_BUFFER_SIZE				(10 * 1024) // in KB

#define TRACE_LEVEL_ERROR						0x00000001
#define TRACE_LEVEL_WARNING						0x00000002
#define TRACE_LEVEL_VERBOSE						0x00000003
#define TRACE_LEVEL_VERBOSE_2					0x00000004

#define TRACE_LEVEL_MAXIMUM						0x00000004

#define TRACE_COMPONENT_GENERAL					0x00000001
#define TRACE_COMPONENT_ADMINQ					0x00000002
#define TRACE_COMPONENT_NOTIFQ					0x00000004
#define TRACE_COMPONENT_COMMAND					0x00000008
#define TRACE_COMPONENT_DEVICE					0x00000010
#define TRACE_COMPONENT_INIT					0x00000020
#define TRACE_COMPONENT_LINK					0x00000040
#define TRACE_COMPONENT_IO						0x00000080
#define TRACE_COMPONENT_INTERRUPT				0x00000100
#define TRACE_COMPONENT_OID						0x00000200
#define TRACE_COMPONENT_PNP						0x00000400
#define TRACE_COMPONENT_TX_LSO					0x00000800
#define TRACE_COMPONENT_TX_NONLSO				0x00001000
#define TRACE_COMPONENT_HANDLERS_ENT_EX			0x00002000
#define TRACE_COMPONENT_VLAN_PRI				0x00004000
#define TRACE_COMPONENT_IO_FRAG					0x00008000
#define TRACE_COMPONENT_PENDING_LIST            0x00010000
#define TRACE_COMPONENT_MEMORY                  0x00020000
#define TRACE_COMPONENT_RSS_PROCESSING          0x00040000

typedef struct _DEBUG_TRACE_CONFIG_CB
{

    ULONG       Level;

    ULONG       Component;

    ULONG       TraceBufferLength; // In KB

    ULONG       TraceFlags;

} TraceConfigCB;


//
// IOCtl calls sent to control device
//

#define IOCTL_IONIC_CONFIGURE_TRACE                     CTL_CODE( FILE_DEVICE_UNKNOWN, 0x10100, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_IONIC_GET_TRACE                           CTL_CODE( FILE_DEVICE_UNKNOWN, 0x10101, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_IONIC_GET_DEV_STATS						CTL_CODE( FILE_DEVICE_UNKNOWN, 0x10102, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_IONIC_GET_MGMT_STATS						CTL_CODE( FILE_DEVICE_UNKNOWN, 0x10103, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_IONIC_GET_PORT_STATS						CTL_CODE( FILE_DEVICE_UNKNOWN, 0x10104, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_IONIC_GET_LIF_STATS						CTL_CODE( FILE_DEVICE_UNKNOWN, 0x10105, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_IONIC_GET_PERF_STATS						CTL_CODE( FILE_DEVICE_UNKNOWN, 0x10106, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_IONIC_RESET_STATS							CTL_CODE( FILE_DEVICE_UNKNOWN, 0x10107, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_IONIC_SET_RX_BUDGET						CTL_CODE( FILE_DEVICE_UNKNOWN, 0x10108, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// Perf stats
//

struct _PERF_MON_RX_QUEUE_STATS {

    ULONG           rx_pool_count;

};

struct _PERF_MON_TX_QUEUE_STATS {

    ULONG           nbl_per_sec;
    
    ULONG           nb_per_sec;
    ULONG           byte_per_sec;

    ULONG           pending_nbl_count;
    ULONG           pending_nb_count;

    ULONG           queue_len;
    ULONG           max_queue_len;

    LONGLONG        nb_time_to_queue; // Ave time from send_packets to ionic_queue_txq_pkt for each NB
    LONGLONG        nb_time_to_complete; // Ave time from send_packets to ionic_txq_complete_pkt for each NB
    LONGLONG        nb_time_queue_to_comp; // Ave time from ionic_queue_txq_pkt to completion

    LONGLONG        nbl_time_to_complete; // Ave time from send_packets to when NBL is ack'd to OS for each NBL

};

struct _PERF_MON_LIF_STATS {

    char            name[LIF_NAME_MAX_SZ];

    ULONG           rx_queue_count;

    ULONG           tx_queue_count;

};

struct _PERF_MON_ADAPTER_STATS {

    WCHAR           name[ADAPTER_NAME_MAX_SZ];

    BOOLEAN         mgmt_device;

    ULONG           lif_count;

    ULONG           core_redirection_count;
    
};

struct _PERF_MON_CB {

    ULONG           adapter_count;

    PVOID          *lookup_table;

};