#pragma once

#pragma pack(push, 1)
#pragma pack(pop)

struct txq_nbl_list {
    PNET_BUFFER_LIST head;
    PNET_BUFFER_LIST tail;
    ULONG count;
};

struct rxq_pkt {

	SLIST_ENTRY	next;

    ULONG flags;

	struct queue *q;

	u32				sg_count;

	void		   *addr;

	u32				offset;

	u32				bytes;

    PNET_BUFFER_LIST parent_nbl;

	PNET_BUFFER		packet;

	NDIS_NET_BUFFER_LIST_FILTERING_INFO	filter_info;

	NET_BUFFER_SHARED_MEMORY	nb_shared_memory_info;

	u64				phys_addr[1];
};

struct tx_frag_pool_elem {

	struct tx_frag_pool_elem *next;

	void	   *buffer;

	ULONG		length;

	PSCATTER_GATHER_LIST tx_frag_list;
};

struct txq_pkt {
    SLIST_ENTRY next;
    struct queue *q;
    ULONG flags;
    PNET_BUFFER_LIST parent_nbl;
    PNET_BUFFER packet_orig;
    PSCATTER_GATHER_LIST sg_os_list;
    PSCATTER_GATHER_LIST sg_list;

	struct tx_frag_pool_elem *tx_frag_elem;
};

struct txq_pkt_private {
    LIST_ENTRY link;
    struct txq_pkt *txq_pkt;
	union {
		struct {
			USHORT desc_cnt;
			USHORT current_desc_cnt;
			ULONG bytes_processed;
		};

		void *Reserved;
	};
};

struct txq_nbl_private {

    LONG    ref_count;

	LONG	nb_processed_cnt;
};

typedef struct _Ionic_Logging
{
    // 
    ULONG level;
    #define Ionic_Logging_level_SIZE sizeof(ULONG)
    #define Ionic_Logging_level_ID 1

} Ionic_Logging, *PIonic_Logging;

#define Ionic_Logging_SIZE (FIELD_OFFSET(Ionic_Logging, level) + Ionic_Logging_level_SIZE)

typedef struct _Ionic_Statistics
{
    // 
    ULONG rxCoalescedWin;
    #define Ionic_Statistics_rxCoalescedWin_SIZE sizeof(ULONG)
    #define Ionic_Statistics_rxCoalescedWin_ID 1

    // 
    ULONG rxCoalescedHost;
    #define Ionic_Statistics_rxCoalescedHost_SIZE sizeof(ULONG)
    #define Ionic_Statistics_rxCoalescedHost_ID 2

    // 
    ULONG rxChecksumOK;
    #define Ionic_Statistics_rxChecksumOK_SIZE sizeof(ULONG)
    #define Ionic_Statistics_rxChecksumOK_ID 3

    // 
    ULONG rxPriority;
    #define Ionic_Statistics_rxPriority_SIZE sizeof(ULONG)
    #define Ionic_Statistics_rxPriority_ID 4

    // 
    ULONG txLargeOffload;
    #define Ionic_Statistics_txLargeOffload_SIZE sizeof(ULONG)
    #define Ionic_Statistics_txLargeOffload_ID 5

    // 
    ULONG txChecksumOffload;
    #define Ionic_Statistics_txChecksumOffload_SIZE sizeof(ULONG)
    #define Ionic_Statistics_txChecksumOffload_ID 6

} Ionic_Statistics, *PIonic_Statistics;

#define Ionic_Statistics_SIZE (FIELD_OFFSET(Ionic_Statistics, txChecksumOffload) + Ionic_Statistics_txChecksumOffload_SIZE)

/** ionic_admin_ctx - Admin command context.
 * @work:		Work completion wait queue element.
 * @cmd:		Admin command (64B) to be copied to the queue.
 * @comp:		Admin completion (16B) copied from the queue.
 *
 * @side_data:		Additional data to be copied to the doorbell page,
 *			  if the command is issued as a dev cmd.
 * @side_data_len:	Length of additional data to be copied.
 *
 * TODO:
 * The side_data and side_data_len are temporary and will be removed.  For now,
 * they are used when admin commands referring to side-band data are posted as
 * dev commands instead.  Only single-indirect side-band data is supported.
 * Only 2K of data is supported, because first half of page is for registers.
 */
struct ionic_admin_ctx {
	//struct completion work;
	union adminq_cmd cmd;
	union adminq_comp comp;

	NDIS_EVENT	CompEvent;
};

typedef bool (*desc_cb)(struct queue *q, struct desc_info *desc_info,
			struct cq_info *cq_info, void *cb_arg, void *packets_to_indicate, void *last_packet);

struct desc_info {
	void *desc;
	void *sg_desc;
	struct desc_info *next;
	unsigned int index;
	unsigned int left;
	desc_cb cb;
	void *cb_arg;
};

#define QUEUE_NAME_MAX_SZ		32

struct queue {
	char name[QUEUE_NAME_MAX_SZ];
	struct ionic_dev *idev;
	struct lif *lif;
	unsigned int index;
	unsigned int type;
	unsigned int hw_index;
	unsigned int hw_type;
	u64 dbval;
	void *base;
	void *sg_base;
	dma_addr_t base_pa;
	dma_addr_t sg_base_pa;
	struct desc_info *info;
	struct desc_info *tail;
	struct desc_info *head;
	unsigned int num_descs;
	unsigned int desc_size;
	unsigned int sg_desc_size;
	void *nop_desc;
	unsigned int pid;
	u64 dbell_count;
	u64 drop;
	u64 stop;
	u64 wake;
};

#define INTR_INDEX_NOT_ASSIGNED		-1
#define INTR_NAME_MAX_SZ		32

struct intr {
	unsigned int index;

    struct lif*			lif;
    struct qcq*			qcq;
};

struct intr_msg {
    ULONG				id;
    IRQ_DEVICE_POLICY   affinity_policy;
    PROCESSOR_NUMBER	proc;
    ULONG               proc_idx;
    KAFFINITY           affinity;
	USHORT				numa_node;

    bool				inuse;
	bool				rss_entry;
	bool				tx_entry;
    struct lif*			lif;
    struct qcq*			qcq;

    /* stats */
#ifdef DBG
    LONG64				isr_cnt;
    LONG64				dpc_cnt;
    LONG64				spurious_cnt;
#endif
};

struct intr_sync_ctx {
    struct lif	   *lif;
    unsigned int	index;
    ULONG			id;
};

struct cq_info {
	void *cq_desc;
	struct cq_info *next;
	unsigned int index;
	bool last;
};

struct cq {
	void *base;
	dma_addr_t base_pa;
	struct lif *lif;
	struct cq_info *info;
	struct cq_info *tail;
	struct queue *bound_q;
	struct intr *bound_intr;
	unsigned int num_descs;
	unsigned int desc_size;
	bool done_color;
};

#define q_to_qcq(qu)     container_of(qu, struct qcq, q)

#define q_to_tx_dev_stats(qu)    q_to_qcq(qu)->tx_stats
#define q_to_rx_dev_stats(qu)    q_to_qcq(qu)->rx_stats


struct qcq {
	void *base;
	dma_addr_t base_pa;
	unsigned int total_size;
	struct queue q;
	struct cq cq;

	union
	{
		struct dev_tx_ring_stats	*tx_stats;
		struct dev_rx_ring_stats	*rx_stats;
	};

	unsigned int flags;

	unsigned int master_slot;

	NDIS_HANDLE tx_pkts_pool;
    struct txq_pkt *txq_base;

#ifdef DBG
    LONG tx_pkts_free_count;
#endif

    void *tx_sgl_buffer;
	
	NDIS_HANDLE		ring_alloc_handle;

	/* tx packet processing */
	KDPC			tx_packet_dpc;
#ifdef DBG
	LONG outstanding_rx_count;
    LONG outstanding_tx_count;
#endif

	// cache aligned elements
	CACHE_ALIGN LONG dpc_exec_cnt;

	CACHE_ALIGN NDIS_SPIN_LOCK rx_ring_lock;

    CACHE_ALIGN NDIS_SPIN_LOCK  txq_nb_lock;
    LIST_ENTRY      txq_nb_list;

    CACHE_ALIGN NDIS_SPIN_LOCK  txq_nbl_lock;
    struct txq_nbl_list txq_nbl_list;

	CACHE_ALIGN SLIST_HEADER	tx_pkt_list; // This was being allocated separately but forcing 128 byte alignment here is the same
};

struct qcqst {
	struct qcq *qcq;
	//struct q_stats *stats;

	union
	{
		struct dev_tx_ring_stats	*tx_stats;
		struct dev_rx_ring_stats	*rx_stats;
	};
};

#define RXQ_INDEX_ANY		(0xFFFF)
struct rx_filter {
	u32 flow_id;
	u32 filter_id;
	u16 rxq_index;
	struct rx_filter_add_cmd cmd;
	LIST_ENTRY by_hash;
	LIST_ENTRY by_id;
};

#define RX_FILTER_HASH_BITS	10
#define RX_FILTER_HLISTS	BIT(RX_FILTER_HASH_BITS)
#define RX_FILTER_HLISTS_MASK	(RX_FILTER_HLISTS - 1)
struct rx_filters {
	NDIS_SPIN_LOCK lock;				/* filter list lock */
	LIST_ENTRY by_hash[RX_FILTER_HLISTS];	/* by skb hash */
	LIST_ENTRY by_id[RX_FILTER_HLISTS];	/* by filter_id */
};

struct rss_map {

	BOOLEAN			ref;

	ULONG			queue_id;
};

enum lif_state_flags {
	LIF_INITED,
	LIF_SW_DEBUG_STATS,
	LIF_UP,
	LIF_LINK_CHECK_NEEDED,
	LIF_QUEUE_RESET,
	LIF_F_FW_READY,

	/* leave this as last */
	LIF_STATE_SIZE
};

struct lif {
	char name[LIF_NAME_MAX_SZ];
	LIST_ENTRY list;

	RTL_BITMAP		state;
	char			state_buffer[BITS_TO_LONGS( LIF_STATE_SIZE)];

	struct ionic *ionic;
	bool registered;
	unsigned int index;
	unsigned int hw_index;
	unsigned int kern_pid;
	u64 __iomem *kern_dbpage;
	struct qcq *adminqcq;
	struct qcq *notifyqcq;
	struct qcqst *txqcqs;
	struct qcqst *rxqcqs;
	u64 last_eid;
	unsigned int neqs;
	unsigned int ntxqs;
	unsigned int nrxqs;
	unsigned int ntxq_descs;
	unsigned int nrxq_descs;
	u32 rx_copybreak;
	unsigned int rx_mode;
	u64 hw_features;

	bool mc_overflow;
	unsigned int nmcast;
	bool uc_overflow;
	unsigned int nucast;

	union lif_identity *identity;
	struct lif_info *info;
	dma_addr_t info_pa;
	u32 info_sz;

	u16 rss_types;
	u32 rss_types_raw;
	u8 rss_hash_key[IONIC_RSS_HASH_KEY_SIZE];
	u16 rss_hash_key_len;
	u8 *rss_ind_tbl;
    dma_addr_t rss_ind_tbl_pa;
	u32 rss_ind_tbl_sz;
	u8 rss_base_cpu;
	u32 rss_hash_flags;

	struct rx_filters rx_filters;
	RTL_BITMAP dbid_inuse;
	unsigned long *dbid_inuse_buffer;
	unsigned int dbid_count;
	void *api_private;

	u32 flags;

	NDIS_QUEUE_NAME	type_name;
	
	struct dev_lif_stats *lif_stats;

	u16			vlan_id;

	struct rss_map *rss_mapping;

    KEVENT  state_change;

	//
	// rx pool
	//

	NDIS_HANDLE		rx_pkts_nbl_pool;
    struct rxq_pkt *rxq_pkt_base;

#ifdef DBG
	LONG			rx_pkts_free_count;
#endif

	u32				rx_pkt_cnt;

	void		   *rx_pkt_buffer_base;
	u32				rx_pkt_buffer_length;
	u32				rx_pkt_buffer_elementsize;
	NDIS_HANDLE		rx_pkt_buffer_handle;
	NDIS_HANDLE		rx_pkt_buffer_alloc_handle;
	void		   *rx_pkt_sgl_buffer;

	// cache aligned elements
	CACHE_ALIGN SLIST_HEADER   rx_pkts_list;
	CACHE_ALIGN NDIS_SPIN_LOCK dbid_inuse_lock;	/* lock the dbid bit list */
	CACHE_ALIGN NDIS_SPIN_LOCK adminq_lock;		/* lock for AdminQ operations */
};

#define lif_to_txqcq(lif, i)	((lif)->txqcqs[i].qcq)
#define lif_to_rxqcq(lif, i)	((lif)->rxqcqs[i].qcq)
#define lif_to_txq(lif, i)	(&lif_to_txqcq((lif), i)->q)
#define lif_to_rxq(lif, i)	(&lif_to_txqcq((lif), i)->q)
#define is_master_lif(lif)	((lif)->index == 0)

/** struct ionic_devinfo - device information. */
struct ionic_devinfo {
	u8 asic_type;
	u8 asic_rev;
	char fw_version[IONIC_DEVINFO_FWVERS_BUFLEN + 1];
	char serial_num[IONIC_DEVINFO_SERIAL_BUFLEN + 1];
};

struct ionic_dev {
	union dev_info_regs __iomem *dev_info_regs;
	union dev_cmd_regs __iomem *dev_cmd_regs;

	u64 __iomem *db_pages;
	dma_addr_t phy_db_pages;

	struct ionic_intr __iomem *intr_ctrl;
	u64 __iomem *intr_status;
	u8 *msix_cfg_base;

	unsigned long *cmb_inuse;
	dma_addr_t phy_cmb_pages;
	u32 cmb_npages;

	struct port_info *port_info;
	dma_addr_t port_info_pa;
	u32 port_info_sz;

	struct ionic_devinfo dev_info;
	u64 last_hb_time;
	u32 last_hb;

	struct ionic *ionic;

	CACHE_ALIGN NDIS_SPIN_LOCK cmb_inuse_lock; /* for cmb_inuse */
};

struct ionic_dev_bar {
	void __iomem *vaddr;
	phys_addr_t bus_addr;
	unsigned long len;
	int res_index;

	UCHAR Type;
    UCHAR ShareDisposition;
    USHORT Flags;
};

struct ionic_default_switch_sriov
{

	ULONG		Type;

	ULONG		Id;

	UNICODE_STRING FriendlyName;

	ULONG		NumberVFs;

	struct vport_info	*Ports;

	struct vf_info	*vfs;

};

struct rx_filter_info
{

	ULONG	Flags;

	ULONG	FilterId;

	ULONG	FilterFlags;

	ULONG	FilterTest;

	UCHAR	MacAddr[ ETH_ALEN];

	USHORT	VLanId;

	ULONG	Priority;
};

struct vport_info
{

	ULONG			Flags;

	PNDIS_RW_LOCK_EX PortLock;

	ULONG			PortId;

	ULONG			QueuePairs;
	
	GROUP_AFFINITY  ProcessorAffinity;

	struct lif	   *lif;

	NDIS_SRIOV_FUNCTION_ID	AttachedFunctionId;

	NDIS_NIC_SWITCH_VPORT_STATE	State;

	ULONG			filter_cnt;

	struct rx_filter_info filter[ IONIC_RECEIVE_FILTER_COUNT];
};

struct vmq_info
{

	ULONG			Flags;

	PNDIS_RW_LOCK_EX QueueLock;

	ULONG			QueueId;

	ULONG			ReceiveBuffers;
	
	GROUP_AFFINITY  ProcessorAffinity;

	struct lif	   *lif;

	ULONG			active_filter_cnt;

	struct rx_filter_info filter[ IONIC_RECEIVE_FILTER_COUNT];
};

struct vf_info
{
	
	ULONG		Flags;

	ULONG		vport_id;

	USHORT		vf_id;

	ULONG		rid;

	NDIS_DEVICE_POWER_STATE	power_state;

	UCHAR	MacAddr[ ETH_ALEN];

	struct ionic_dev_bar BAR[2];
};

struct filter_info
{
	
	ULONG			flags;

	ULONG			q_index;
};

struct ionic {

	LIST_ENTRY		list_entry;

    NDIS_STRING     name;

	WCHAR			device_location[ IONIC_DEV_LOC_LEN];

	ULONG			Flags;

	NDIS_HANDLE		adapterhandle;
	ULONG			ConfigStatus;
	NDIS_HARDWARE_STATUS	hardware_status;

	USHORT			numa_node;

	LIST_ENTRY		lifs;

	RTL_BITMAP		lifbits;
	char			lifbits_buffer[BITS_TO_LONGS( IONIC_LIFS_MAX)];
	
	NDIS_HANDLE		WatchDogTimer;
	NDIS_HANDLE		LinkCheckTimer;

	ULONG			sgl_size_in_bytes;
	ULONG			max_sgl_elements;
	ULONG			dma_alignment;
	NDIS_HANDLE		dma_handle;

	LONG			dpc_ref_counter;

	PCI_COMMON_CONFIG pci_config;

	UNICODE_STRING	computer_name;

	char			perm_addr[6];

	char			config_addr[6];

	NDIS_HANDLE		timer_obj;

	u16				vlan_id;

	u32				speed_duplex;

	int				assigned_int_cnt;

	ULONG			MaxNumberVFs;
	ULONG			MaxNumVPorts;

	ULONG			max_vmq_cnt;

	u32				num_rss_queues;

	u32				nearby_core_count;

	u32				tx_coalesce_usecs;
	u32				rx_coalesce_usecs;
	u32				rx_coalesce_hw;

    PNDIS_SYSTEM_PROCESSOR_INFO_EX  sys_proc_info;
    PNDIS_PROCESSOR_INFO_EX         sys_proc;

    NDIS_HANDLE						intr_obj;
    NDIS_INTERRUPT_TYPE				intr_type;

    /* Legacy interrupts */
    ULONG							intr_lvl;

    /* MSI-x interrupt message info table */
    PIO_INTERRUPT_MESSAGE_INFO      intr_msginfo_tbl;

    /* MSI-x interrupt info table */
    struct intr_msg*				intr_msg_tbl;
    struct intr*					intr_tbl;

    /* interrupt resource allocator */
    RTL_BITMAP						intrs;
    char							intrs_buffer[BITS_TO_LONGS(INTR_CTRL_REGS_MAX)];

	struct ionic_default_switch_sriov	SriovSwitch;
		
	struct vmq_info	vm_queue[ IONIC_MAX_VM_QUEUE_COUNT];

	u8				mcast_filter_array[ ETH_ALEN * 64];

	struct filter_info q_filter[ IONIC_MAX_FILTER_COUNT];

	ULONG			total_lif_count;

	PCI_EXPRESS_SRIOV_CAPABILITY	sriov_caps;

    ULONG           flow_control;

    ULONG           frame_size;

	struct ionic_dev idev;
	struct ionic_dev_bar bars[IONIC_BARS_MAX];
	unsigned int num_bars;
	struct identity ident;
	struct lif *master_lif;
	bool is_mgmt_nic;
	unsigned int nnqs_per_lif;
	unsigned int neqs_per_lif;
	unsigned int ntxqs_per_lif;
	unsigned int nrxqs_per_lif;
	unsigned int nslaves;
	unsigned int nintrs;
#ifndef HAVE_PCI_IRQ_API
	struct msix_entry *msix;
#endif

	ULONG	ProbedBARS[ PCI_TYPE0_ADDRESSES];

	struct dev_port_stats port_stats;

	NDIS_HANDLE		tx_frag_pool_handle;

	struct tx_frag_pool_elem	*tx_frag_pool;
	
	struct tx_frag_pool_elem	*tx_frag_pool_head;
	struct tx_frag_pool_elem	*tx_frag_pool_tail;

	ULONG						tx_frag_pool_count;

	PSCATTER_GATHER_LIST		tx_frag_pool_sg_list;

    LONG            outstanding_request_count;
    KEVENT          outstanding_complete_event;

    LONG            core_redirect_count;

	ULONG			ntx_buffers;
	ULONG			nrx_buffers;
 
	// offload state

	ULONG			ipv4_tx_state;
	ULONG			ipv4_rx_state;
	ULONG			tcpv4_tx_state;
	ULONG			tcpv4_rx_state;
	ULONG			tcpv6_tx_state;
	ULONG			tcpv6_rx_state;
	ULONG			udpv4_tx_state;
	ULONG			udpv4_rx_state;
	ULONG			udpv6_tx_state;
	ULONG			udpv6_rx_state;

	ULONG			lsov2ipv4_state;
	ULONG			lsov2ipv6_state;
	ULONG			lsov1_state;

    ULONG           rx_pool_factor;

	/* Registry parameters specific to this interface */
	struct registry_entry *registry_config;

	// Cache aligned elements
	CACHE_ALIGN NDIS_SPIN_LOCK	dev_cmd_lock;
	CACHE_ALIGN NDIS_SPIN_LOCK	tx_frag_pool_lock;
};

typedef struct _ADAPTER_CNTRL_EXT
{

	NDIS_HANDLE			AdapterHandle;


} AdapterCntrlExt;

struct _WORK_ITEM
{

    struct ionic *ionic;

    ULONG       work_item_id;

    void       *context;

};

extern "C" {

typedef struct _PERF_MON_COLLECTED_STATS {
    // per-adapter stats


    // per-lif stats
	ULONG rx_pool_alloc_cnt;
	ULONG rx_pool_free_cnt;
	ULONG rx_pool_size;
	ULONGLONG rx_pool_alloc_time;
	ULONGLONG rx_pool_free_time;

} PERF_MON_COLLECTED_STATS;

};

struct memory_track_cb {

    LIST_ENTRY      list;

    void           *buffer;

    void           *return_buffer;

    ULONG           length;

};

struct registry_entry {

	WCHAR		name[ IONIC_REG_LEN];

	ULONG		minimum_value;

	ULONG		maximum_value;

	ULONG		default_value;

	ULONG		current_value;

};
