#pragma once

#include "pci_ids.h"
#include "ionic_types.h"
#include "ionic_stats.h"

#define IONIC_LINKNAME_STRING_USER      L"\\??\\IonicControl"

#define ADAPTER_NAME_MAX_SZ             64

//
// IO Control codes
//

#define IOCTL_IONIC_CONFIGURE_TRACE     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x10100, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_IONIC_GET_TRACE           CTL_CODE(FILE_DEVICE_UNKNOWN, 0x10101, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_IONIC_GET_DEV_STATS       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x10102, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_IONIC_GET_MGMT_STATS      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x10103, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_IONIC_GET_PORT_STATS      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x10104, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_IONIC_GET_LIF_STATS       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x10105, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_IONIC_GET_PERF_STATS      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x10106, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define _IOCTL_IONIC_UNUSED_7           CTL_CODE(FILE_DEVICE_UNKNOWN, 0x10107, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_IONIC_SET_RX_BUDGET       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x10108, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_IONIC_GET_REG_KEY_INFO    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x10109, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_IONIC_PORT_GET            CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1010a, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_IONIC_PORT_SET            CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1010b, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_IONIC_GET_ADAPTER_INFO	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1010c, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IONIC_DEV_LOC_LEN					50

//
// Common ioctl input buffers
//

typedef struct _ADAPTER_CB {
    WCHAR       AdapterName[ADAPTER_NAME_MAX_SZ];
    ULONG       Skip;
    ULONG       Index;
} AdapterCB;

//
// Logging subsystem defines and structres
// related to IOCTL_IONIC_CONFIGURE_TRACE
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
// Perf stats
//

struct _PERF_MON_RX_QUEUE_STATS {

    ULONG           rx_pool_count;

};

struct _PERF_MON_TX_QUEUE_STATS {

    ULONG           pending_nbl_count;
    ULONG           pending_nb_count;

    ULONG           queue_len;
    ULONG           max_queue_len;

};

struct _PERF_MON_LIF_STATS {

    char            name[LIF_NAME_MAX_SZ];

    ULONG           rx_queue_count;

    ULONG           tx_queue_count;

	/* rx pool stats */

	__le32			rx_pool_alloc_cnt;

	__le32			rx_pool_free_cnt;
	
	__le32			rx_pool_size;
	
	__le64			rx_pool_alloc_time;
	
	__le64			rx_pool_free_time;

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

struct _REG_KEY_INFO_HDR {

	ULONG			next_entry;

	ULONG			entry_count;

	WCHAR			device_location[ IONIC_DEV_LOC_LEN];

	WCHAR           name[ADAPTER_NAME_MAX_SZ];

};

struct _REG_KEY_ENTRY {

	ULONG			next_entry;

	WCHAR			key_name_offset; // From the start of this structure

	ULONG			key_name_len;

	ULONG			min_value;

	ULONG			max_value;

	ULONG			default_value;

	ULONG			current_value;

};

//
// related to IOCTL_IONIC_PORT_SET
//

typedef struct _PORT_CONFIG_CB {
    ULONG       AutoNeg;
    ULONG       Speed;
    ULONG       FEC;
    ULONG       Pause; // pause flags | type, see ionic_if.h
} PortConfigCB;

typedef struct _PORT_SET_CB {
    WCHAR       AdapterName[ADAPTER_NAME_MAX_SZ];
    ULONG       Flags;
    PortConfigCB Config;
} PortSetCB;

// PortSetCB Flags
#define PORT_SET_AUTONEG        0x0001
#define PORT_SET_SPEED          0x0002
#define PORT_SET_FEC            0x0004
#define PORT_SET_PAUSE          0x0008

//
// related to IOCTL_IONIC_GET_ADAPTER_INFO
//

#define IONIC_DEVINFO_FWVERS_BUFLEN 32
#define IONIC_DEVINFO_SERIAL_BUFLEN 32

struct _ADAPTER_INFO {

	WCHAR			device_location[ IONIC_DEV_LOC_LEN];

	WCHAR           name[ADAPTER_NAME_MAX_SZ];

	ULONG			hw_state;

	USHORT			link_state;

	ULONG			Mtu;

	ULONGLONG		Speed;

	USHORT			vendor_id;

	USHORT			product_id;

	UCHAR			asic_type;
	
	UCHAR			asic_rev;

	char			fw_version[IONIC_DEVINFO_FWVERS_BUFLEN + 1];

	char			serial_num[IONIC_DEVINFO_SERIAL_BUFLEN + 1];
};

struct _ADAPTER_INFO_HDR {

	ULONG			count;
	
};

typedef struct _DEV_STATS_RESP_CB {
    WCHAR                   adapter_name[ADAPTER_NAME_MAX_SZ];
    struct dev_port_stats   stats;
} DevStatsRespCB;

typedef struct _PORT_STATS_RESP_CB {
    WCHAR                   adapter_name[ADAPTER_NAME_MAX_SZ];
    struct port_stats       stats;
} PortStatsRespCB;

typedef struct _MGMT_STATS_RESP_CB {
    WCHAR                   adapter_name[ADAPTER_NAME_MAX_SZ];
    struct mgmt_port_stats  stats;
} MgmtStatsRespCB;

typedef struct _LIF_STATS_RESP_CB {
    WCHAR                   adapter_name[ADAPTER_NAME_MAX_SZ];
    DWORD                   lif_index;
    struct lif_stats        stats;
} LifStatsRespCB;
