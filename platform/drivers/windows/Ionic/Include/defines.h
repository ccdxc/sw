#pragma once

//
// Enable compile time features
//

//#define NO_TRACE								1
//#define TRACK_MEMORY_BUFFER_ALLOC               1

//
// Memory allocation tags
//

#define IONIC_GENERIC_TAG						'AGOI'
#define IONIC_ADAPTER_TAG						'DAOI'
#define IONIC_LIF_TAG							'FIOI'
#define IONIC_QCQ_TAG							'QQOI'
#define IONIC_DESC_INFO_TAG						'IDOI'
#define IONIC_CQ_INFO_TAG						'CCOI'
#define IONIC_Q_INFO_TAG						'ICOI'
#define IONIC_TX_QUEUE_TAG						'QTOI'
#define IONIC_RX_QUEUE_TAG						'QROI'
#define IONIC_TIMER_TAG							'ITOI'
#define IONIC_LIF_ID_TAG						'LIOI'
#define IONIC_STATS_TAG							'SQOI'
#define IONIC_RX_MEM_TAG						'XROI'
#define IONIC_TX_MEM_TAG						'XTOI'
#define IONIC_INT_TABLE_TAG						'TIOI'
#define IONIC_SG_LIST_TAG						'GSOI'
#define IONIC_RX_FILTER_TAG						'FROI'
#define IONIC_RX_DMA_ENTRY_TAG					'EDOI'
#define IONIC_ADAPTER_STATS_TAG					'SDOI'
#define IONIC_FRAG_POOL_TAG						'PFOI'
#define IONIC_RSS_INDIR_TBL_TAG					'TIOI'
#define IONIC_WORK_ITEM_TAG                     'IWOI'
#define IONIC_MEMORY_TRACK_TAG                  'TMOI'

#ifndef BooleanFlagOn
#define BooleanFlagOn(F,SF)   ((BOOLEAN)(((F) & (SF)) != 0))
#endif

#ifndef SetFlag
#define SetFlag(_F,_SF)       ((_F) |= (_SF))
#endif

#ifndef ClearFlag
#define ClearFlag(_F,_SF)     ((_F) &= ~(_SF))
#endif

typedef
NTSTATUS
(*PTraceLogMsg)( IN ULONG Component,
                 IN ULONG Level,
                 IN PCCH Format,
                 ...);

#ifdef NO_TRACE
#define DbgTrace(X) {};
#else
#define DbgTrace(X) \
    { PTraceLogMsg pTraceFnc = NULL; \
       pTraceFnc = (PTraceLogMsg)TraceMsgFnc; \
       if( pTraceFnc != NULL) pTraceFnc X;}
#endif

#define TRACE_TO_DEBUGGER				0x00000001
#define TRACE_BUFFER_WRAPPED			0x00000002

#define REG_TRACE_LEVEL					L"TraceLevel"
#define REG_TRACE_COMPONENT				L"TraceComponent"
#define REG_TRACE_FLAGS					L"TraceFlags"
#define REG_TRACE_BUFFER_LENGTH			L"TraceBufferLength"

#define REG_RX_BUDGET					L"RxBudget"

#define IONIC_LINKNAME_STRING             L"\\DosDevices\\IonicControl"
#define IONIC_NTDEVICE_STRING             L"\\Device\\IonicCntrlDev"

#define IONIC_ADMINQ_WAIT_TIME			10000

#define IONIC_ENH_CAPS_OFFSET			0x100

#define IONIC_WINDOWS_DISTRO			"Windows"
#define IONIC_WINDOWS_DISTRO_LEN		strlen( IONIC_WINDOWS_DISTRO)

//
// Coding definitions
//

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#define BITS_PER_BYTE   8
#define BITS_TO_LONGS(nr)   DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))
#define ALIGN_SZ(n, d)		(DIV_ROUND_UP( n, d) * d)

#define BUG_ON(x) ASSERT(!(x))
#define BUG() ASSERT(1)

#define __iomem

#define ARRAY_SIZE(array) sizeof(array)/ sizeof(array[0])

#define ALIGN(x,a) (((x)+(a)-1)&~((a)-1))

#define BIT(nr)         (1UL << (nr))
#define BIT_ULL(n) (1ULL << (n))

#define cpu_to_le16		(__le16)
#define cpu_to_le32		(__le32)
#define cpu_to_le64		(__le64)
#define le16_to_cpu
#define le64_to_cpu
#define le32_to_cpu

#define NET_BUFFER_SG_LIST_CREATED      1

#define NET_BUFFER_LIST_SET_REF_COUNTER(_nbl_, _count_) \
    ((struct txq_nbl_private *)&NET_BUFFER_LIST_MINIPORT_RESERVED(_nbl_))->ref_count = (_count_)
#define NET_BUFFER_LIST_GET_REF_COUNTER(_nbl_) \
    ((struct txq_nbl_private *)&NET_BUFFER_LIST_MINIPORT_RESERVED(_nbl_))->ref_count
#define NET_BUFFER_LIST_DEC_REF_COUNTER(_nbl_) NdisInterlockedDecrement(\
    &((struct txq_nbl_private *)&NET_BUFFER_LIST_MINIPORT_RESERVED(_nbl_))->ref_count)

#define NDIS_SET_NET_BUFFER_LIST_PRIORITY(_NBL, _Priority)   ((NDIS_NET_BUFFER_LIST_8021Q_INFO *) &NET_BUFFER_LIST_INFO((_NBL), Ieee8021QNetBufferListInfo))->TagHeader.UserPriority = (_Priority)

#define QCQ_F_INITED		BIT(0)
#define QCQ_F_SG		BIT(1)
#define QCQ_F_INTR		BIT(2)
#define QCQ_F_TX_STATS		BIT(3)
#define QCQ_F_RX_STATS		BIT(4)
#define QCQ_F_NOTIFYQ		BIT(5)

#define IONIC_ADMINQ_LENGTH				16	/* must be a power of two */
#define IONIC_NOTIFYQ_LENGTH			64	/* must be a power of two */

#define IONIC_TX_FRAG_POOL_COUNT		1024

#define IONIC_ONE_SEC_WAIT              10000000

#define IONIC_REG_LEN					25
#define IONIC_CTL_INTR_CNT				1

#define IONIC_REG_UNDEFINED				0xFFFFFFFF

#define IONIC_PERF_MON_ALL_STATS		0
#define IONIC_PERF_MON_NO_STATS			MAXULONGLONG

#define ANY_PROCESSOR_INDEX				(INVALID_PROCESSOR_INDEX - 1)

#define IONIC_1Q_TAG					0x0081

//
// Flow control options
//

#define IONIC_FC_DISABLED               0x00000000
#define IONIC_FC_TX_ENABLED             0x00000001
#define IONIC_FC_RX_ENABLED             0x00000002
#define IONIC_FC_TXRX_ENABLED           0x00000003

//
// Config state flags
//

#define IONIC_SRIOV_CAPABLE					0x00000001
#define IONIC_SRIOV_ENABLED					0x00000002
#define IONIC_VMQ_ENABLED					0x00000004
#define IONIC_RSS_ENABLED					0x00000008
#define IONIC_VMQ_VLAN_FILTERING_ENABLED	0x00000010
#define IONIC_SRIOV_MODE					0x00000020
#define IONIC_INTERRUPT_MOD_ENABLED			0x00000040
#define IONIC_PRIORITY_ENABLED				0x00000080
#define IONIC_VLAN_ENABLED					0x00000100

#define IONIC_TX_MODE_DPC                   0x00000400 // Flush in the rx DPC
#define IONIC_TX_MODE_SEND                  0x00000800 // Flush in the tx send_path

//
// Default Rx budget value
//

#define IONIC_RX_BUDGET_DEFAULT				128
#define IONIC_TX_BUDGET_DEFAULT				128

//
// Max rx pool factor
//

#define IONIC_MIN_RX_POOL_FACTOR            1
#define IONIC_MAX_RX_POOL_FACTOR            10
#define IONIC_DEFAULT_RX_POOL_FACTOR        4

//
// Minimum Rx packet size
//

#define IONIC_MINIMUM_RX_PACKET_LEN			60

//
// When not in rss mode then the default queue count will be 4
//

#define IONIC_DEFAULT_QUEUE_COUNT           4

//
// Custom OIDs
//

#define OID_IONIC_CUSTOM_1                    0xff010203
#define OID_IONIC_CUSTOM_2                    0xff010204

//
// Polling time for the NDIS wrapper to call the CheckForHang callback in seconds
//

#define NIC_ADAPTER_CHECK_FOR_HANG_TIMEOUT		4

//
// Max count in array of filters to go back to the queue
//

#define IONIC_MAX_FILTER_COUNT					64

//
// Default interrupt coal in uSec
//

#define IONIC_DEFAULT_INT_COAL_US				10

//
// Max number of receive filters currently supported
//

#define IONIC_RECEIVE_FILTER_COUNT				32

#define NETDEV_RSS_KEY_LEN						(13 * 4)

//
// Command timeout
//

#define DEVICE_CMD_TIMEOUT					30

#define IONIC_NDIS_MAX_LSO_PKT				(64*1024)

#define IONIC_MAX_PHYSICAL_MAPPING			(256 * 1024)
#define IONIC_DEFAULT_DMA_ALIGN				64

#define MEGABITS_PER_SECOND             1000000ULL

#define IONIC_MULTICAST_PERFECT_FILTERS		32
#define IONIC_RX_COPYBREAK_DEFAULT			256

#define IONIC_MAX_VM_QUEUE_COUNT			32

//
// Ionic flags
//

#define IONIC_FLAG_PARENT_PARTITION			0x00000001
#define IONIC_FLAG_PAUSED                   0x00000002
#define IONIC_FLAG_PF_DEVICE                0x00000004
#define IONIC_FLAG_SR_RECEIVED				0x00000008
#define IONIC_FLAG_DISABLE_CMD_INTERFACE	0x00000010
#define IONIC_FLAG_DISABLE_ADMINQ_INTERFACE	0x00000020

//
// Lif flags
//

#define IONIC_LIF_INITED                    0x00000001
#define IONIC_LIF_FLAG_TYPE_VMQ				0x00000002
#define IONIC_LIF_FLAG_TYPE_VPORT			0x00000004

//
// Queue state flags
//

#define IONIC_QUEUE_STATE_INITIALIZED		0x00000001
#define IONIC_QUEUE_STATE_ALLOCATED			0x00000002
#define IONIC_QUEUE_STATE_VLAN_FLTR_SET		0x00000004

//
// vPort state flags
//

#define IONIC_VPORT_STATE_INITIALIZED		0x00000001
#define IONIC_VPORT_STATE_ACTIVATED			0x00000002
#define IONIC_VPORT_STATE_VLAN_FLTR_SET		0x00000004

//
// Vf flags
//

#define IONIC_VF_INITIALIZED				0x00000001
#define IONIC_VF_ALLOCATED					0x00000002
#define IONIC_VF_CONNECTED					0x00000004

//
// rx filter flags
//

#define IONIC_FILTER_SET					0x00000001
#define IONIC_FILTER_MAC_FILTER				0x00000002
#define IONIC_FILTER_VLAN_FILTER			0x00000004
#define IONIC_FILTER_STATION_ADDR			0x00000008
#define IONIC_FILTER_STATION_VLAN			0x00000010

//
// Int table flags
//

#define IONIC_TARGET_PROC_CHANGED           0x00000001

//
// Work items
//

#define IONIC_WORKITEM_FREE_QUEUE           0x00000001

#define WARN_ON_ONCE
#define WARN_ON

#ifndef unlikely
#define unlikely(_x) _x
#define likely(_x) _x
#endif

#define MAX_NUM_SG_CNTR		(IONIC_TX_MAX_SG_ELEMS + 1)

#define IONIC_MIN_MTU			590
#define IONIC_MAX_MTU			9212
#define IONIC_DEFAULT_MTU		9212

#define IONIC_MAX_RX_DESC		16384
#define IONIC_MAX_TX_DESC		8192
#define IONIC_MIN_TXRX_DESC		16
#define IONIC_DEF_TXRX_DESC		4096
#define IONIC_LIFS_MAX			1024

#define GOLDEN_RATIO_32			0x61C88647 // taken from Linux source

#define IONIC_ITR_COAL_USEC_DEFAULT	64
#define IONIC_ITR_COAL_USEC_MIN		1
#define IONIC_ITR_COAL_USEC_MAX		84

#define container_of CONTAINING_RECORD

enum {
	DUMP_PREFIX_NONE,
	DUMP_PREFIX_ADDRESS,
	DUMP_PREFIX_OFFSET
};

#define hex_asc(x)	"0123456789abcdef"[x]

#define Ionic_LoggingGuid \
    { 0xff484c02,0xe3fa,0x40bc, { 0x87,0xc3,0x4e,0x9b,0x3e,0xc5,0x81,0x61 } }

//DEFINE_GUID(Ionic_Logging_GUID, \
//            0xff484c02,0xe3fa,0x40bc,0x87,0xc3,0x4e,0x9b,0x3e,0xc5,0x81,0x61);

// Ionic_Statistics - Ionic_Statistics
#define Ionic_StatisticsGuid \
    { 0x6ca426c6,0x6469,0x4916, { 0x9e,0x0a,0xb2,0x6a,0x91,0x2b,0xb8,0xd0 } }

//DEFINE_GUID(Ionic_Statistics_GUID, \
//            0x6ca426c6,0x6469,0x4916,0x9e,0x0a,0xb2,0x6a,0x91,0x2b,0xb8,0xd0);

#define min_t(type,x,y) \
    ( ( ((type)(x)) < ((type)(y)) ) ? ((type)(x)) : ((type)(y)) )
#define max_t(type,x,y) \
    ( ( ((type)(x)) > ((type)(y)) ) ? ((type)(x)) : ((type)(y)) )

typedef bool (*ionic_cq_cb)(struct cq *cq, struct cq_info *cq_info, void *cb_arg);

typedef void (*ionic_cq_done_cb)(void *done_arg);

#define memcpy_toio(reg, buff, len)		WRITE_REGISTER_BUFFER_UCHAR( (UCHAR *)reg, \
																	 (UCHAR *)buff, \
																	 len);

#define memcpy_fromio(buff, reg, len)	READ_REGISTER_BUFFER_UCHAR( (UCHAR *)reg, \
																	(UCHAR *)buff, \
																	 len);

#define dma_alloc_coherent(device, len, physaddr, type)		AllocSharedMemory( device, \
																			   len, \
																			   physaddr);

#define dma_free_coherent(device,len,addr,physaddr)			ReleaseSharedMemory( device, \
																				 len, \
																				 addr, \
																				 physaddr);

enum {
	NETIF_F_SG_BIT,			/* Scatter/gather IO. */
	NETIF_F_IP_CSUM_BIT,		/* Can checksum TCP/UDP over IPv4. */
	__UNUSED_NETIF_F_1,
	NETIF_F_HW_CSUM_BIT,		/* Can checksum all the packets. */
	NETIF_F_IPV6_CSUM_BIT,		/* Can checksum TCP/UDP over IPV6 */
	NETIF_F_HIGHDMA_BIT,		/* Can DMA to high memory. */
	NETIF_F_FRAGLIST_BIT,		/* Scatter/gather IO. */
	NETIF_F_HW_VLAN_CTAG_TX_BIT,	/* Transmit VLAN CTAG HW acceleration */
	NETIF_F_HW_VLAN_CTAG_RX_BIT,	/* Receive VLAN CTAG HW acceleration */
	NETIF_F_HW_VLAN_CTAG_FILTER_BIT,/* Receive filtering on VLAN CTAGs */
	NETIF_F_VLAN_CHALLENGED_BIT,	/* Device cannot handle VLAN packets */
	NETIF_F_GSO_BIT,		/* Enable software GSO. */
	NETIF_F_LLTX_BIT,		/* LockLess TX - deprecated. Please */
					/* do not use LLTX in new drivers */
	NETIF_F_NETNS_LOCAL_BIT,	/* Does not change network namespaces */
	NETIF_F_GRO_BIT,		/* Generic receive offload */
	NETIF_F_LRO_BIT,		/* large receive offload */

	/**/NETIF_F_GSO_SHIFT,		/* keep the order of SKB_GSO_* bits */
	NETIF_F_TSO_BIT			/* ... TCPv4 segmentation */
		= NETIF_F_GSO_SHIFT,
	NETIF_F_UFO_BIT,		/* ... UDPv4 fragmentation */
	NETIF_F_GSO_ROBUST_BIT,		/* ... ->SKB_GSO_DODGY */
	NETIF_F_TSO_ECN_BIT,		/* ... TCP ECN support */
	NETIF_F_TSO6_BIT,		/* ... TCPv6 segmentation */
	NETIF_F_FSO_BIT,		/* ... FCoE segmentation */
	NETIF_F_GSO_GRE_BIT,		/* ... GRE with TSO */
	NETIF_F_GSO_GRE_CSUM_BIT,	/* ... GRE with csum with TSO */
	NETIF_F_GSO_IPIP_BIT,		/* ... IPIP tunnel with TSO */
	NETIF_F_GSO_SIT_BIT,		/* ... SIT tunnel with TSO */
	NETIF_F_GSO_UDP_TUNNEL_BIT,	/* ... UDP TUNNEL with TSO */
	NETIF_F_GSO_UDP_TUNNEL_CSUM_BIT,/* ... UDP TUNNEL with TSO & CSUM */
	NETIF_F_GSO_TUNNEL_REMCSUM_BIT, /* ... TUNNEL with TSO & REMCSUM */
	/**/NETIF_F_GSO_LAST =		/* last bit, see GSO_MASK */
		NETIF_F_GSO_TUNNEL_REMCSUM_BIT,

	NETIF_F_FCOE_CRC_BIT,		/* FCoE CRC32 */
	NETIF_F_SCTP_CSUM_BIT,		/* SCTP checksum offload */
	NETIF_F_FCOE_MTU_BIT,		/* Supports max FCoE MTU, 2158 bytes*/
	NETIF_F_NTUPLE_BIT,		/* N-tuple filters supported */
	NETIF_F_RXHASH_BIT,		/* Receive hashing offload */
	NETIF_F_RXCSUM_BIT,		/* Receive checksumming offload */
	NETIF_F_NOCACHE_COPY_BIT,	/* Use no-cache copyfromuser */
	NETIF_F_LOOPBACK_BIT,		/* Enable loopback */
	NETIF_F_RXFCS_BIT,		/* Append FCS to skb pkt data */
	NETIF_F_RXALL_BIT,		/* Receive errored frames too */
	NETIF_F_HW_VLAN_STAG_TX_BIT,	/* Transmit VLAN STAG HW acceleration */
	NETIF_F_HW_VLAN_STAG_RX_BIT,	/* Receive VLAN STAG HW acceleration */
	NETIF_F_HW_VLAN_STAG_FILTER_BIT,/* Receive filtering on VLAN STAGs */
	NETIF_F_HW_L2FW_DOFFLOAD_BIT,	/* Allow L2 Forwarding in Hardware */
	NETIF_F_BUSY_POLL_BIT,		/* Busy poll */
	NETIF_F_HW_SWITCH_OFFLOAD_BIT,  /* HW switch offload */

	/*
	 * Add your fresh new feature above and remember to update
	 * netdev_features_strings[] in net/core/ethtool.c and maybe
	 * some feature mask #defines below. Please also describe it
	 * in Documentation/networking/netdev-features.txt.
	 */

	/**/NETDEV_FEATURE_COUNT
};

#define __NETIF_F_BIT(bit)	((netdev_features_t)1 << (bit))
#define __NETIF_F(name)		__NETIF_F_BIT(NETIF_F_##name##_BIT)

#define NETIF_F_FCOE_CRC	__NETIF_F(FCOE_CRC)
#define NETIF_F_FCOE_MTU	__NETIF_F(FCOE_MTU)
#define NETIF_F_FRAGLIST	__NETIF_F(FRAGLIST)
#define NETIF_F_FSO		__NETIF_F(FSO)
#define NETIF_F_GRO		__NETIF_F(GRO)
#define NETIF_F_GSO		__NETIF_F(GSO)
#define NETIF_F_GSO_ROBUST	__NETIF_F(GSO_ROBUST)
#define NETIF_F_HIGHDMA		__NETIF_F(HIGHDMA)
#define NETIF_F_HW_CSUM		__NETIF_F(HW_CSUM)
#define NETIF_F_HW_VLAN_CTAG_FILTER __NETIF_F(HW_VLAN_CTAG_FILTER)
#define NETIF_F_HW_VLAN_CTAG_RX	__NETIF_F(HW_VLAN_CTAG_RX)
#define NETIF_F_HW_VLAN_CTAG_TX	__NETIF_F(HW_VLAN_CTAG_TX)
#define NETIF_F_IP_CSUM		__NETIF_F(IP_CSUM)
#define NETIF_F_IPV6_CSUM	__NETIF_F(IPV6_CSUM)
#define NETIF_F_LLTX		__NETIF_F(LLTX)
#define NETIF_F_LOOPBACK	__NETIF_F(LOOPBACK)
#define NETIF_F_LRO		__NETIF_F(LRO)
#define NETIF_F_NETNS_LOCAL	__NETIF_F(NETNS_LOCAL)
#define NETIF_F_NOCACHE_COPY	__NETIF_F(NOCACHE_COPY)
#define NETIF_F_NTUPLE		__NETIF_F(NTUPLE)
#define NETIF_F_RXCSUM		__NETIF_F(RXCSUM)
#define NETIF_F_RXHASH		__NETIF_F(RXHASH)
#define NETIF_F_SCTP_CSUM	__NETIF_F(SCTP_CSUM)
#define NETIF_F_SG		__NETIF_F(SG)
#define NETIF_F_TSO6		__NETIF_F(TSO6)
#define NETIF_F_TSO_ECN		__NETIF_F(TSO_ECN)
#define NETIF_F_TSO		__NETIF_F(TSO)
#define NETIF_F_UFO		__NETIF_F(UFO)
#define NETIF_F_VLAN_CHALLENGED	__NETIF_F(VLAN_CHALLENGED)
#define NETIF_F_RXFCS		__NETIF_F(RXFCS)
#define NETIF_F_RXALL		__NETIF_F(RXALL)
#define NETIF_F_GSO_GRE		__NETIF_F(GSO_GRE)
#define NETIF_F_GSO_GRE_CSUM	__NETIF_F(GSO_GRE_CSUM)
#define NETIF_F_GSO_IPIP	__NETIF_F(GSO_IPIP)
#define NETIF_F_GSO_SIT		__NETIF_F(GSO_SIT)
#define NETIF_F_GSO_UDP_TUNNEL	__NETIF_F(GSO_UDP_TUNNEL)
#define NETIF_F_GSO_UDP_TUNNEL_CSUM __NETIF_F(GSO_UDP_TUNNEL_CSUM)
#define NETIF_F_GSO_TUNNEL_REMCSUM __NETIF_F(GSO_TUNNEL_REMCSUM)
#define NETIF_F_HW_VLAN_STAG_FILTER __NETIF_F(HW_VLAN_STAG_FILTER)
#define NETIF_F_HW_VLAN_STAG_RX	__NETIF_F(HW_VLAN_STAG_RX)
#define NETIF_F_HW_VLAN_STAG_TX	__NETIF_F(HW_VLAN_STAG_TX)
#define NETIF_F_HW_L2FW_DOFFLOAD	__NETIF_F(HW_L2FW_DOFFLOAD)
#define NETIF_F_BUSY_POLL	__NETIF_F(BUSY_POLL)
#define NETIF_F_HW_SWITCH_OFFLOAD	__NETIF_F(HW_SWITCH_OFFLOAD)