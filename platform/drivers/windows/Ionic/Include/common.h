#pragma once

extern "C"
{
#pragma warning(disable : 4201)

#include <ndis.h>
#include <ntstrsafe.h>

#define KERNEL_MODE
#include "UserCommon.h"

#include "ionic_types.h"

#include "defines.h"

#include "version.h"

#include "IonicEventLog.h"

#ifndef NO_EXTERNS
#include "externs.h"
#endif

#include "net_hdrs.h"

#include "pci_ids.h"

#include "ionic_stats.h"

#include "ionic_if.h"

#include "ionic_regs.h"

#include "structs.h"

#include "perf_counters.h"

//
// ASM prototypes for csum calculations
//

unsigned short 
csum_fold(unsigned int sum);

unsigned int 
csum_tcpudp_nofold( unsigned int saddr,
				  unsigned int daddr,
				  unsigned short len,
				  unsigned short proto,
				  unsigned int sum);

unsigned short 
csum_ipv6_magic( void *saddr,
				 void *daddr,
				 unsigned int len,
				 unsigned short proto,
				 unsigned int sum);

//
// System prototypes not called out in ndis headers
//

NTSYSAPI
VOID
NTAPI
RtlInitializeBitMap (
    _Out_ PRTL_BITMAP BitMapHeader,
    _In_opt_ __drv_aliasesMem PULONG BitMapBuffer,
    _In_opt_ ULONG SizeOfBitMap
    );

//
// adminq.cpp prototypes
//

NDIS_STATUS
ionic_lif_adminq_init(struct lif *lif);

NDIS_STATUS
ionic_lif_rss_config( struct lif *lif, 
					  const u16 types,
					  const u8 *key, 
					  const u8 *indir);

//
// command.cpp prototypes
//

void ionic_dev_cmd_go(struct ionic_dev *idev, union dev_cmd *cmd);

bool ionic_dev_cmd_done(struct ionic_dev *idev);

u8 ionic_dev_cmd_status(struct ionic_dev *idev);

NDIS_STATUS
ionic_dev_cmd_wait(struct ionic *ionic, unsigned long max_seconds);

void 
ionic_dev_cmd_reset(struct ionic_dev *idev);

void ionic_dev_cmd_identify(struct ionic_dev *idev, u8 ver);

void ionic_dev_cmd_init(struct ionic_dev *idev);

void ionic_dev_cmd_port_speed(struct ionic_dev *idev, u32 speed);

void ionic_dev_cmd_port_get_speed(struct ionic_dev *idev);

void ionic_dev_cmd_port_autoneg(struct ionic_dev *idev, u8 an_enable);

void ionic_dev_cmd_port_get_autoneg(struct ionic_dev *idev);

void ionic_dev_cmd_port_identify(struct ionic_dev *idev);

void ionic_dev_cmd_port_init(struct ionic_dev *idev);

void ionic_dev_cmd_port_state(struct ionic_dev *idev, u8 state);

void ionic_dev_cmd_port_reset(struct ionic_dev *idev);

void ionic_dev_cmd_lif_identify(struct ionic_dev *idev, u8 type, u8 ver);

void ionic_dev_cmd_lif_init(struct ionic_dev *idev, u16 lif_index,
			    dma_addr_t info_pa);

void ionic_dev_cmd_comp(struct ionic_dev *idev, union dev_cmd_comp *comp);

void ionic_q_post(struct queue *q, bool ring_doorbell, desc_cb cb,
		  void *cb_arg);

unsigned int ionic_q_space_avail(struct queue *q);

bool ionic_q_has_space(struct queue *q, unsigned int want);

NDIS_STATUS
ionic_adminq_post(struct lif *lif, struct ionic_admin_ctx *ctx);

NDIS_STATUS ionic_adminq_post_wait(struct lif *lif, struct ionic_admin_ctx *ctx);

void ionic_dev_cmd_lif_reset(struct ionic_dev *idev, u16 lif_index);

void ionic_lif_reset(struct lif *lif);

void ionic_dev_cmd_adminq_init(struct ionic_dev *idev, struct qcq *qcq,
			       u16 lif_index, u16 intr_index);

void 
ionic_intr_mask_on_assertion(struct intr *intr, u32 mask_on_assert);

//
// control.cpp prototypes
//

NDIS_STATUS
RegisterDevice( NDIS_HANDLE Adapter);

void
DeregisterDevice( void);

NTSTATUS
DispatchCb( PDEVICE_OBJECT       DeviceObject,
			PIRP                 Irp);

NTSTATUS
DeviceIoControl( PDEVICE_OBJECT        DeviceObject,
				 PIRP                  Irp);

//
// device.cpp prototypes
//

NDIS_STATUS
ionic_dev_setup(struct ionic *ionic);

void
ionic_init_devinfo( struct ionic *Adapter);

void ionic_dev_teardown(struct ionic *ionic);

void ionic_q_service(struct queue *q, struct cq_info *cq_info,
		     unsigned int stop_index);

bool ionic_adminq_service(struct cq *cq, struct cq_info *cq_info, void *cb_arg);

NDIS_STATUS
ionic_reset(struct ionic *ionic);

NDIS_STATUS
ionic_identify(struct ionic *ionic);

NDIS_STATUS
ionic_init(struct ionic *ionic);

void
ionic_deinit(struct ionic *ionic);

void
ReadPCIeConfigSpace(struct ionic *ionic);

struct tx_frag_pool_elem *
get_tx_frag(struct ionic *ionic);

void
return_tx_frag(struct ionic *ionic,
				struct tx_frag_pool_elem *elem);

NDIS_STATUS
oid_set_power_state( struct ionic *ionic,
					 void *buffer,
					 ULONG buffer_len,
					 ULONG *bytes_read);

//
// generic.cpp prototypes
//

ULONG 
ExceptionFilter( IN ULONG Code, 
				 IN PEXCEPTION_POINTERS ExceptPtrs);

NDIS_STATUS
ReadRegParameters( struct ionic *Adapter);

void *
AllocSharedMemory( struct ionic *Adapter, 
				   ULONG Length,
				   ULONG64 *PhysAddr);

void 
ReleaseSharedMemory( struct ionic *Adapter, 
					 ULONG Length,
					 void *Address, 
					 ULONG64 PhysAddr);

void *
AllocateSharedMemorySpecifyNuma( struct ionic *ionic,
								 ULONG length,
								 NDIS_HANDLE *alloc_handle,
								 dma_addr_t *phys_addr);

char *
ionic_dev_asic_name(u8 asic_type);

bool 
is_power_of_2(unsigned int n);

unsigned int 
ilog2(unsigned int n);

const char *
ionic_opcode_to_str(enum cmd_opcode opcode);

const char *
ionic_error_to_str(enum status_code code);

NDIS_STATUS
ionic_error_to_errno(enum status_code code);

int ionic_heartbeat_check(struct ionic *ionic);

void ionic_watchdog_cb(void *SystemContext,
	void *FunctionContext,
	void *Context1,
	void *Context2);

void 
_dump_message_table(
    PIO_INTERRUPT_MESSAGE_INFO int_msg_info);

int ionic_init_nic_features(struct lif *lif);

void ionic_link_status_check(struct lif *lif, u16 link_status);

int is_multicast_ether_addr(const u8 *addr);

unsigned int
hash_32_generic(u32 val);

unsigned int 
hash_32( u32 val, unsigned int bits);

int 
is_zero_ether_addr(const u8 *addr);

int
_isascii(u8 c);

static void 
hex_dump_to_buffer(const void *buf, size_t len, int rowsize,
			int groupsize, unsigned char *linebuf,
			size_t linebuflen, bool ascii);

void 
print_hex_dump(const char *prefix_str, int prefix_type,
			int rowsize, int groupsize,
			const void *buf, size_t len, bool ascii);

void
IoPrint(IN char *Format,
		...);

void ionic_indicate_status(struct ionic *ionic, NDIS_STATUS code,
  PVOID StatusBuffer, ULONG StatusBufferSize);

NDIS_STATUS
UpdateRegistryParameter( IN PUNICODE_STRING ValueName,
                         IN ULONG ValueType,
                         IN void *ValueData,
                         IN ULONG ValueDataLength);

NDIS_STATUS
ReadConfigParams( void);

void
ReadSriovConfig( struct ionic *Adapter,
				 NDIS_HANDLE Config);

void
ReadVmqConfig( struct ionic *Adapter,
				 NDIS_HANDLE Config);

void
ReadRssConfig( struct ionic *Adapter,
				 NDIS_HANDLE Config);

char *
GetHyperVPartitionTypeString(ULONG PartitionType);

void ionic_link_up(struct ionic *ionic);

void ionic_link_down(struct ionic *ionic);

BOOLEAN
ValidateGroupAffinity( PGROUP_AFFINITY GroupAffinity);

u32
GetNextProcIndex(ULONG_PTR Mask,
	ULONG_PTR MaskIndex);

void *
LockBuffer(void *Buffer,
	ULONG BufferLength,
	MDL **Mdl);

void 
ionic_tx_tcp_pseudo_csum( unsigned char *IpHdr,
						  unsigned char *TcpHdr,
						  ULONG IpVersion);

USHORT
GetDescriptorCount(NET_BUFFER *NetBuffer,
	ULONG MSS);

void
DumpCompDesc(struct txq_comp *comp);

void
DumpDesc(struct txq_desc *desc, struct txq_sg_desc *sg_desc, u32 sg_count, bool start, bool done);

NDIS_STATUS
oid_set_interrupt_moderation(struct ionic *ionic,
	void *info_buffer,
	ULONG info_buffer_length,
	ULONG *bytes_read);

BOOLEAN
IsValidAffinity(struct ionic *ionic,
				GROUP_AFFINITY ProcessorAffinity);

NDIS_STATUS
oid_query_isolation_info(struct ionic *ionic,
	void *info_buffer,
	ULONG info_buffer_length,
	ULONG *bytes_written);

ULONG
get_frame_type( void *hdr_buffer);

NDIS_STATUS
oid_set_vlan(struct ionic *ionic,
	void *info_buffer,
	ULONG info_buffer_length,
	ULONG *bytes_needed,
	ULONG *bytes_read);

NTSTATUS
ConfigureRxBudget( IN ULONG Budget);

NDIS_STATUS
queue_workitem(struct ionic *ionic,
    ULONG WorkItem,
    void *Context);

void
process_work_item(PVOID   WorkItemContext,
    NDIS_HANDLE  work_handle);

NDIS_STATUS
get_perfmon_stats(struct _PERF_MON_CB **perfmon_stats, ULONG *len);

void
ref_request(struct ionic *ionic);

void
deref_request(struct ionic *ionic, LONG count);

void
wait_on_requests(struct ionic *ionic);

void *
NdisAllocateMemoryWithTagPriority_internal(NDIS_HANDLE      NdisHandle,
    UINT             Length,
    ULONG            Tag,
    EX_POOL_PRIORITY Priority);

void
NdisFreeMemoryWithTagPriority_internal(NDIS_HANDLE NdisHandle,
    PVOID       VirtualAddress,
    ULONG       Tag);

void
validate_memory( void);

//
// handlers.cpp prototypes
//

NDIS_STATUS
SetOptions( NDIS_HANDLE  DriverHandle,
			NDIS_HANDLE  DriverContext);

void 
HaltEx( NDIS_HANDLE MiniportAdapterContext,
		NDIS_HALT_ACTION HaltAction);

NDIS_STATUS 
Pause( NDIS_HANDLE MiniportAdapterContext,
	   PNDIS_MINIPORT_PAUSE_PARAMETERS PauseParameters);

NDIS_STATUS 
Restart( NDIS_HANDLE MiniportAdapterContext,
		 PNDIS_MINIPORT_RESTART_PARAMETERS RestartParameters);

void 
CancelSend( NDIS_HANDLE MiniportAdapterContext,
			PVOID CancelId);

void 
ShutdownEx( NDIS_HANDLE MiniportAdapterContext,
		    NDIS_SHUTDOWN_ACTION ShutdownAction);

MINIPORT_RESET ResetEx;

NDIS_STATUS 
SetGeneralAttribs( struct ionic *Adapter);

NDIS_STATUS
SetRegistrationAttribs(struct ionic *adapter);

BOOLEAN 
CheckForHangEx( NDIS_HANDLE MiniportAdapterContext);

//
// hardware.cpp prototypes
//

void
mask_all_interrupts( struct ionic *ionic);

NDIS_STATUS
ionic_map_bars( struct ionic *Adapter,
				PNDIS_MINIPORT_INIT_PARAMETERS InitParameters);

NDIS_STATUS
ionic_map_bar( struct ionic *Adapter,
			   ULONG BarIndex);

void
ionic_unmap_bars( struct ionic *Adapter);

UCHAR 
ioread8(const volatile void *addr);

USHORT 
ioread16(const volatile void *addr);

ULONG 
ioread32(const volatile void *addr);

ULONG64 
readq(const volatile void *addr);

void 
iowrite32(ULONG data, const volatile void *addr);

void 
writeq(u64 data, const volatile void *addr);

NDIS_STATUS 
ionic_register_interrupts(struct ionic *Adapter);

BOOLEAN 
ionic_msi_handler(PVOID miniport_interrupt_context,
  ULONG message_id, PBOOLEAN queue_default_interrupt_dpc,
  PULONG target_processors);

VOID 
ionic_msi_dpc_handler(NDIS_HANDLE miniport_interrupt_context,
    ULONG message_id,
    PVOID miniport_dpc_context,
    PVOID receive_throttle_params,
	PVOID ndis_reserved2);

VOID 
ionic_msi_disable( PVOID miniport_interrupt_context,
				   ULONG message_id);

VOID 
ionic_msi_enable( PVOID miniport_interrupt_context,
				  ULONG message_id);

void ionic_process_sg_list( PDEVICE_OBJECT DeviceObject, 
							PVOID Irp,
							PSCATTER_GATHER_LIST sg_list, 
							PVOID Context);

NDIS_STATUS
ionic_bus_alloc_irq_vectors(struct ionic *ionic, unsigned int nintrs);

int ionic_bus_get_irq(struct ionic *ionic, unsigned int num);

u64 __iomem *ionic_bus_map_dbpage(struct ionic *ionic, u32 db_page_offset);

void ionic_bus_unmap_dbpage(struct ionic *ionic, u64 *db_page);

NDIS_STATUS
ionic_intr_alloc(struct lif *lif, struct intr *intr, u32 preferred_proc);

void ionic_intr_free(struct lif *lif, int index);

inline void ionic_intr_init(struct ionic_dev *idev, struct intr *intr,
				   unsigned long index);

int ionic_db_page_num(struct lif *lif, int pid);

struct interrupt_info *
get_interrupt(struct ionic *ionic, ULONG index);

ULONG
locate_proc(struct ionic *ionic,
			ULONG preferredProc);

NDIS_STATUS 
ionic_set_offload_attributes(struct ionic *ionic);

NDIS_STATUS 
oid_handle_offload_encapsulation( struct ionic *ionic,
								  VOID *info_buffer, 
								  ULONG info_buffer_length,
								  ULONG *bytes_read, 
								  ULONG *bytes_needed);

NDIS_STATUS 
oid_handle_offload_parameters( struct ionic *ionic,
								  VOID *info_buffer, 
								  ULONG info_buffer_length,
								  ULONG *bytes_read, 
								  ULONG *bytes_needed);

NDIS_STATUS
oid_query_bar_info(struct ionic *ionic,
	void *info_buffer,
	ULONG info_buffer_length,
	ULONG *bytes_written);

NDIS_STATUS
init_dma( struct ionic *ionic);

void
deinit_dma(struct ionic *ionic);

//
// init.cpp prototypes
//

NDIS_STATUS
DriverEntry( void *DriverObject,
			 void *RegistryPath);

void
DriverUnload( PDRIVER_OBJECT  DriverObject);

NDIS_STATUS
InitializeEx( NDIS_HANDLE AdapterHandle,
			  NDIS_HANDLE DriverContext,
			  PNDIS_MINIPORT_INIT_PARAMETERS InitParameters);

//
// lif.cpp prototypes
//

NDIS_STATUS
ionic_lif_identify(struct ionic *ionic, u8 lif_type,
		       union lif_identity *lid);

NDIS_STATUS
ionic_lifs_size(struct ionic *ionic);

NDIS_STATUS
ionic_lifs_alloc(struct ionic *ionic);

NDIS_STATUS
ionic_lifs_init(struct ionic *ionic);

void ionic_lifs_free(struct ionic *ionic);

NDIS_STATUS
ionic_lif_init(struct lif *lif);

void ionic_lif_free(struct lif *lif);

int ionic_station_set(struct lif *lif);

NDIS_STATUS ionic_lif_addr(struct lif *lif, const u8 *addr, bool add);

int ionic_napi(struct lif *lif, int budget, ionic_cq_cb cb,
	       ionic_cq_done_cb done_cb, void *done_arg);

void
CheckLinkStatusCb( PVOID   WorkItemContext,
				   NDIS_HANDLE  NdisIoWorkItemHandle);

NDIS_STATUS
ionic_lif_rx_mode(struct lif *lif, unsigned int rx_mode);

NDIS_STATUS
ionic_lif_rxq_init(struct lif *lif, struct qcq *qcq);

NDIS_STATUS
ionic_lif_txq_init(struct lif *lif, struct qcqst *qcqst);

NDIS_STATUS
ionic_lif_rxqs_init(struct lif *lif);

NDIS_STATUS
ionic_lif_txqs_init(struct lif *lif);

NDIS_STATUS
ionic_txrx_alloc(struct lif *lif,
				 ULONG vport_id,
				 ULONG queue_id,
				 GROUP_AFFINITY Affinity);

NDIS_STATUS
ionic_lif_stop(struct lif *lif);

NDIS_STATUS
ionic_stop(struct ionic *ionic);

void
ionic_flush(struct ionic *ionic);

NDIS_STATUS
ionic_lif_open(struct lif *lif,
			   ULONG vport_id,
			   ULONG queue_id,
			   GROUP_AFFINITY Affinity);

NDIS_STATUS
ionic_open(struct ionic *ionic);

void ionic_lifs_deinit(struct ionic *ionic);

void *
ionic_allocate_slave_lif( struct ionic *ionic);

NDIS_STATUS
ionic_lif_start(struct lif *lif);

NDIS_STATUS
ionic_start(struct ionic *ionic);

void ionic_lif_deinit(struct lif *lif);

void ionic_slave_free(struct ionic *ionic, int index);

NDIS_STATUS 
ionic_lif_macvlan(struct lif *lif, const u8 *addr, u16 vlan_id, bool add);

NDIS_STATUS 
ionic_lif_vlan(struct lif *lif, u16 vlan_id, bool add);

NDIS_STATUS
ionic_set_coalesce(struct ionic *ionic,
				   BOOLEAN Enable);

void
ionic_lifs_reset(struct ionic *ionic);

//
// notifq.cpp prototypes
//

NDIS_STATUS ionic_lif_notifyq_init(struct lif *lif);

int ionic_notifyq_clean(struct lif *lif, int budget);



//
// oid.cpp prototypes
//

char *
GetSwitchTypeString(ULONG SwitchType);

char *
GetNicTypeString(ULONG NicType);

char *
GetNicStateString(ULONG NicState);

char *
GetPortStateString(ULONG PortState);

char *
GetPortTypeString(ULONG PortType);

NDIS_STATUS
oid_multicast_list_set(struct ionic *ionic,
                       VOID *info_buffer,
                       ULONG info_buffer_length);

NDIS_STATUS
oid_packet_filter_set(struct ionic *ionic,
  VOID *info_buffer, ULONG info_buffer_length,
  ULONG *bytes_read, ULONG *bytes_needed);

char *
GetQueryOidName( NDIS_OID Oid);

char *
GetSetOidName( NDIS_OID Oid);

void 
ionic_get_supported_oids( PNDIS_OID *ppoids, ULONG *list_length);

NDIS_STATUS 
oid_method_information( struct ionic *ionic,
						NDIS_OID oid, 
						PNDIS_OID_REQUEST OidRequest);

NDIS_STATUS 
oid_set_information( struct ionic *ionic,
					 NDIS_OID_REQUEST *OidRequest,
					 VOID *info_buffer, 
					 ULONG info_buffer_length,
					 ULONG *bytes_read, 
					 ULONG *bytes_needed);

NDIS_STATUS 
oid_query_information( struct ionic *ionic, 
					   NDIS_OID oid,
					   PVOID info_buffer, 
					   ULONG info_buffer_length,
					   PULONG bytes_written, 
					   PULONG bytes_needed);

NDIS_STATUS 
OidRequest( NDIS_HANDLE MiniportAdapterContext,
			PNDIS_OID_REQUEST OidRequest);

void 
CancelOidRequest( NDIS_HANDLE MiniportAdapterContext,
				  PVOID RequestId);

char *
GetOidType(NDIS_REQUEST_TYPE Type);

NDIS_STATUS 
DirectOidRequest( NDIS_HANDLE MiniportAdapterContext,
				  PNDIS_OID_REQUEST OidRequest);

void 
CancelDirectOidRequest( NDIS_HANDLE MiniportAdapterContext,
						PVOID RequestId);


NDIS_STATUS 
SynchronousOidRequest( NDIS_HANDLE      MiniportAdapterContext,
					   NDIS_OID_REQUEST *OidRequest);

NDIS_STATUS 
oid_multicast_list_remove( struct ionic *ionic);

NDIS_STATUS 
oid_multicast_list_restore( struct ionic *ionic);

//
// pnp.cpp prototypes
//

NDIS_STATUS 
ionic_add_device( NDIS_HANDLE miniport_adapter_handle,
				  NDIS_HANDLE miniport_driver_context);

VOID 
ionic_remove_device(NDIS_HANDLE miniport_add_device_context);

NDIS_STATUS 
ionic_filter_resource_requirements( NDIS_HANDLE miniport_add_device_context, PIRP irp);

NDIS_STATUS 
ionic_start_device(NDIS_HANDLE miniport_add_device_context,
  PIRP irp);

void PnpEventNotify(NDIS_HANDLE MiniportAdapterContext,
	PNET_DEVICE_PNP_EVENT NetDevicePnPEvent);

//
// port.cpp prototypes
//

NDIS_STATUS
ionic_port_identify(struct ionic *ionic);

NDIS_STATUS
ionic_port_init(struct ionic *ionic);

NDIS_STATUS 
ionic_port_reset(struct ionic *ionic);

NDIS_STATUS
oid_port_create(struct ionic *ionic,
	void *data_buffer,
	ULONG data_buffer_length,
	ULONG *bytes_needed);

NDIS_STATUS
oid_port_teardown(struct ionic *ionic,
	void *data_buffer,
	ULONG data_buffer_length,
	ULONG *bytes_needed);

NDIS_STATUS
oid_port_delete(struct ionic *ionic,
	void *data_buffer,
	ULONG data_buffer_length,
	ULONG *bytes_needed);

//
// qcq.cpp prototypes
//

NDIS_STATUS
ionic_q_init(struct lif *lif, struct ionic_dev *idev, struct queue *q,
		 unsigned int index, const char *name, unsigned int num_descs,
		 size_t desc_size, size_t sg_desc_size, unsigned int pid);

NDIS_STATUS
ionic_cq_init(struct lif *lif, struct cq *cq, struct intr *intr,
		  unsigned int num_descs, size_t desc_size);

void ionic_q_sg_map(struct queue *q, void *base, dma_addr_t base_pa);

void ionic_q_map(struct queue *q, void *base, dma_addr_t base_pa);

void ionic_cq_map(struct cq *cq, void *base, dma_addr_t base_pa);

void ionic_cq_bind(struct cq *cq, struct queue *q);

NDIS_STATUS
ionic_link_master_qcq(struct qcq *qcq, struct qcqst *master_qs);

void ionic_lif_qcq_deinit(struct lif *lif, struct qcq *qcq);

unsigned int ionic_cq_service(struct cq *cq, unsigned int work_to_do,
			      ionic_cq_cb cb, void *cb_arg, ionic_cq_done_cb done_cb,
			      void *done_arg);

NDIS_STATUS
ionic_qcq_enable(struct qcq *qcq);

NDIS_STATUS
ionic_qcq_disable(struct qcq *qcq);

void
ionic_reset_qcq(struct qcq *qcq);

//
// rss.cpp prototypes
//

NDIS_STATUS
oid_set_rss_parameters( struct ionic *ionic,
	void * data_buffer,
	ULONG data_buffer_length,
	ULONG *bytes_read,
	ULONG *bytes_needed);

NDIS_STATUS
oid_set_rss_hash( struct ionic *ionic,
	void * data_buffer,
	ULONG data_buffer_length,
	ULONG *bytes_read,
	ULONG *bytes_needed);

u8 
ethtool_rxfh_indir_default(u32 index, u32 n_rx_rings);

NDIS_STATUS
ionic_lif_rss_init(struct lif *lif);

NDIS_STATUS
ionic_lif_rss_deinit(struct lif *lif);

u16 
ionic_convert_ndis_hash_type(ULONG ndis_hash_type);

NDIS_STATUS
oid_query_rss_hash(struct ionic *ionic,
	void *info_buffer,
	ULONG info_buffer_length,
	ULONG *bytes_needed,
	ULONG *bytes_written);

NDIS_STATUS
oid_query_rss_caps(struct ionic *ionic,
	void *info_buffer,
	ULONG info_buffer_length,
	ULONG *bytes_needed,
	ULONG *bytes_written);

NDIS_STATUS
map_rss_table(struct lif *lif);

u32
get_rss_affinity(struct lif *lif,
	u32 queue_id);

void
remap_rss_rx_affinity(struct lif *lif);

//
// rx.cpp prototypes
//

void ionic_free_rxq_pkts(struct ionic *ionic, struct qcq * qcq);

NDIS_STATUS ionic_alloc_rxq_pkts(struct ionic *ionic, struct qcq *qcq);

struct rxq_pkt* ionic_get_next_rxq_pkt(struct qcq *qcq);

PNET_BUFFER ionic_alloc_rxq_netbuffers(	struct ionic *ionic,
										struct qcqst *qcqst, 
										unsigned int size, 
										struct rxq_pkt *rxq_pkt);

void
ionic_return_rxq_pkt(struct qcq *qcq,
    struct rxq_pkt * rxq_pkt);

void ionic_rx_flush(struct cq *cq);

NDIS_STATUS ionic_rx_init(struct lif *lif, struct qcqst *qcqst);

void ionic_rx_filters_deinit(struct lif *lif);

void
ionic_rx_napi( struct interrupt_info *int_info, 
			   int budget, 
			   NDIS_RECEIVE_THROTTLE_PARAMETERS *receive_throttle_params);

void ionic_rq_indicate_bufs(struct lif *lif,
    struct qcqst *qcqst,
	struct qcq* qcq,
    unsigned int count,
	PNET_BUFFER_LIST packets_to_indicate);

void ionic_return_packet(NDIS_HANDLE adapter_context,
    PNET_BUFFER_LIST pnetlist, ULONG return_flags);


struct rx_filter *ionic_rx_filter_by_macvlan(struct lif *lif, u16 vid);

struct rx_filter *ionic_rx_filter_by_vlan(struct lif *lif, u16 vid);

struct rx_filter *ionic_rx_filter_by_addr(struct lif *lif, const u8 *addr);

int ionic_rx_filter_save(struct lif *lif, u32 flow_id, u16 rxq_index,
			 u32 hash, struct ionic_admin_ctx *ctx);

void ionic_rx_filter_free(struct lif *lif, struct rx_filter *f);

int ionic_rx_filters_init(struct lif *lif);

NDIS_STATUS
ionic_set_rx_mode(struct lif *lif, int rx_mode);

void
IndicateRxQueueState( struct ionic *ionic,
					  NDIS_RECEIVE_QUEUE_ID QueueId,
					  NDIS_RECEIVE_QUEUE_OPERATIONAL_STATE State);

NDIS_STATUS
init_rx_queue_info(struct ionic *ionic);

NDIS_STATUS
delete_rx_queue_info(struct ionic *ionic);

NDIS_STATUS
_ionic_lif_rx_mode(struct lif *lif, unsigned int rx_mode);

void
DumpRxPacket( struct rxq_pkt *rxq_pkt);

ULONG
GetPktCount( struct ionic *ionic, struct queue *q);

void ionic_rx_empty(struct queue *q);

void
ionic_reset_rxq_pkts(struct qcq *qcq);

void
ionic_release_rxq_pkts(struct qcq *qcq);

//
// sriov.cpp prototypes
//

struct vf_info *
locate_vfinfo(struct ionic *ionic,
		ULONG vf_id);

NDIS_STATUS
oid_sriov_bar_resources(struct ionic *ionic,
	NDIS_OID_REQUEST *OidRequest);

NDIS_STATUS
oid_sriov_read_vf_pci_config(struct ionic *ionic,
	NDIS_OID_REQUEST *OidRequest);;

NDIS_STATUS
oid_sriov_write_vf_pci_config(struct ionic *ionic,
	void *info_buffer,
	ULONG info_buffer_len,
	ULONG *bytes_read);

NDIS_VF_RID
get_rid(struct ionic *ionic,
	USHORT vf_id);

//
// stats.cpp prototypes
//

NDIS_STATUS
GetDevStats(ULONG BufferLength,
	void *Buffer,
	ULONG *BytesReturned);

NDIS_STATUS
GetMgmtStats(ULONG BufferLength,
	void *Buffer,
	ULONG *BytesReturned);

NDIS_STATUS
GetPortStats(ULONG Port,
	ULONG BufferLength,
	void *Buffer,
	ULONG *BytesReturned);

NDIS_STATUS
GetLifStats(ULONG Lif,
	ULONG BufferLength,
	void *Buffer,
	ULONG *BytesReturned);

NDIS_STATUS
GetPerfStats(void *Buffer,
	ULONG BufferLength,
	ULONG *BytesReturned);

//
// switch.cpp prototypes
//

NDIS_STATUS
oid_nic_switch_create_switch(struct ionic *ionic,
	NDIS_OID_REQUEST *OidRequest);

NDIS_STATUS
oid_nic_switch_delete_switch(struct ionic *ionic,
	void * data_buffer,
	ULONG data_buffer_length,
	ULONG *bytes_needed);

NDIS_STATUS
oid_nic_switch_create(struct ionic *ionic,
	void *data_buffer,
	ULONG data_buffer_length,
	ULONG *bytes_needed);

NDIS_STATUS
oid_nic_switch_delete(struct ionic *ionic,
	void *data_buffer,
	ULONG data_buffer_length,
	ULONG *bytes_needed);

NDIS_STATUS
oid_nic_switch_connect( struct ionic *ionic,
	void *data_buffer,
	ULONG data_buffer_length,
	ULONG *bytes_needed);

NDIS_STATUS
oid_nic_switch_updated(struct ionic *ionic,
	void *data_buffer,
	ULONG data_buffer_length,
	ULONG *bytes_needed);

NDIS_STATUS
oid_filter_set_filter_vswitch(struct ionic *ionic,
							  NDIS_OID_REQUEST *OidRequest);

NDIS_STATUS
oid_switch_create_vport(struct ionic *ionic,
	NDIS_OID_REQUEST *OidRequest);

NDIS_STATUS
oid_switch_delete_vport(struct ionic *ionic,
	void *info_buffer);

NDIS_STATUS
oid_filter_clear_filter_vswitch( struct ionic *ionic,
								 void *info_buffer);

NDIS_STATUS
oid_filter_move_filter_vswitch(struct ionic *ionic,
							   void *info_buffer);

NDIS_STATUS
oid_switch_query_vport_params(struct ionic *ionic,
							   NDIS_OID_REQUEST *OidRequest);

NDIS_STATUS
oid_switch_set_vport_params(struct ionic *ionic,
							void *info_buffer);

NDIS_STATUS
oid_switch_create_vf( struct ionic *ionic,
					  NDIS_OID_REQUEST *OidRequest);

NDIS_STATUS
oid_switch_delete_vf(struct ionic *ionic,
	void *info_buffer);

NDIS_STATUS
oid_switch_read_vf_ven_dev_id(struct ionic *ionic,
	NDIS_OID_REQUEST *OidRequest);

NDIS_STATUS
oid_switch_reset_vf(struct ionic *ionic,
	void *info_buffer);

NDIS_STATUS
oid_switch_set_vf_power(struct ionic *ionic,
	void *info_buffer,
	ULONG info_buffer_len,
	ULONG *bytes_read);

//
// trace.cpp prototypes
//

void
DbgLogMsg( IN ULONG Component,
            IN ULONG Level,
            IN PCCH Format,
                ...);

NTSTATUS
InitializeTrace( void);

void
TearDownTrace( void);

NTSTATUS
ConfigureTrace( IN TraceConfigCB *TraceInfo);

NTSTATUS
GetTraceBuffer( IN ULONG BufferLength,
                OUT void *Buffer,
                OUT ULONG *CopiedLength);

void
TagInitialLogEntry( void);

//
// tx.cpp prototypes
//

void ionic_txq_nbl_list_push_head(struct txq_nbl_list *list,
    PNET_BUFFER_LIST nbl);

void ionic_service_pending_nbl_requests(struct ionic *ionic, struct qcq *qcq);

void ionic_service_pending_nb_requests(struct ionic *ionic, struct qcq *qcq);

NDIS_STATUS
process_nbl(struct ionic *ionic,
    struct qcq *qcq,
    PNET_BUFFER_LIST nbl,
    txq_nbl_list *completed_list);

bool ionic_tx_service(struct cq *cq, struct cq_info *cq_info,
    void *cb_arg);

NDIS_STATUS ionic_queue_txq_pkt(struct ionic *ionic,
    struct qcq *qcq, PNET_BUFFER packet_orig);

void ionic_send_complete(struct ionic *ionic,
    struct txq_nbl_list *list, KIRQL irql);

void ionic_txq_nbl_list_push_tail(struct txq_nbl_list *list,
    PNET_BUFFER_LIST nbl);

void ionic_return_txq_pkt(struct qcq *qcq, struct txq_pkt *txq_pkt, ULONG desc_cnt);

PNET_BUFFER_LIST ionic_txq_nbl_list_pop_head(struct txq_nbl_list *list);

void ionic_txq_complete_failed_pkt(struct ionic *ionic, struct qcq *qcq,
    PNET_BUFFER packet,
    struct txq_nbl_list *completed_list, NDIS_STATUS status);

void ionic_txq_nbl_list_init(struct txq_nbl_list *list);

void ionic_free_txq_pkts(struct ionic *ionic, struct qcq * qcq);

struct txq_pkt* ionic_get_next_txq_pkt(struct qcq *qcq);

UCHAR ionic_txq_nbl_list_is_empty(struct txq_nbl_list *list);

void 
ionic_send_packets(NDIS_HANDLE adapter_context,
    PNET_BUFFER_LIST pnetlist,
    NDIS_PORT_NUMBER port_number, ULONG send_flags);

void ionic_tx_release_pending(struct ionic *ionic, struct qcq * qcq);

ULONG ionic_tx_flush(struct qcq *qcq, bool cleanup, bool credits);

NDIS_STATUS 
ionic_alloc_txq_pkts(struct ionic *ionic, struct qcq *qcq);

NDIS_STATUS ionic_tx_fill(struct ionic *ionic, struct qcqst *qcqst);

void
DumpTxPacket( void *packet, MDL *Mdl);

NDIS_STATUS
copy_buffer(struct ionic *ionic,
	NET_BUFFER *packet,
	struct tx_frag_pool_elem **frag);

void
comp_data(NET_BUFFER *packet,
	tx_frag_pool_elem *fragment);

void ionic_tx_empty(struct queue *q);

//
// vmq.cpp prototypes
//

NDIS_STATUS
InitHardwareConfig( struct ionic *Adapter);

NDIS_STATUS
oid_filter_allocate_queue(struct ionic *ionic,
	NDIS_OID_REQUEST *OidRequest);

NDIS_STATUS
oid_filter_set_filter(struct ionic *ionic,
	NDIS_OID_REQUEST *OidRequest);

NDIS_STATUS
oid_filter_queue_alloc_complete( struct ionic *ionic,
								 NDIS_OID_REQUEST *OidRequest);

NDIS_STATUS
oid_free_queue(struct ionic *ionic,
    NDIS_OID_REQUEST *OidRequest,
	void *Params);

NDIS_STATUS
oid_filter_clear_filter( struct ionic *ionic,
					   void *info_buffer);

NDIS_STATUS
oid_method_receive_filter_params( struct ionic *ionic,
								  NDIS_OID_REQUEST *OidRequest);

NDIS_STATUS
oid_set_receive_filter_params( struct ionic *ionic,
								void *info_buffer,
								ULONG info_buffer_len,
								ULONG *bytes_read);

BOOLEAN 
synchronize_affinity_change( NDIS_HANDLE SynchronizeContext);

//
// perf_counter.cpp
//

NTSTATUS NTAPI
IonicPerfCounterCallback(
    _In_ PCW_CALLBACK_TYPE Type,
    _In_ PPCW_CALLBACK_INFORMATION Info,
    _In_opt_ PVOID Context);

NTSTATUS
IonicAddAdapterInstance(
    _In_ PPCW_BUFFER Buffer,
    _In_ PUNICODE_STRING Name,
    _In_ ULONG InstanceId,
    _In_ struct _PERF_MON_ADAPTER_STATS *AdapterStats);

NTSTATUS
IonicAddLifInstance(
    _In_ PPCW_BUFFER Buffer,
    _In_ PUNICODE_STRING Name,
    _In_ ULONG InstanceId,
    _In_ struct _PERF_MON_LIF_STATS *LifStats);

NTSTATUS
IonicAddRxQueueInstance(
    _In_ PPCW_BUFFER Buffer,
    _In_ PUNICODE_STRING Name,
    _In_ ULONG InstanceId,
    _In_ struct _PERF_MON_RX_QUEUE_STATS *RxQueueStats);

NTSTATUS
IonicAddTxQueueInstance(
    _In_ PPCW_BUFFER Buffer,
    _In_ PUNICODE_STRING Name,
    _In_ ULONG InstanceId,
    _In_ struct _PERF_MON_TX_QUEUE_STATS *TxQueueStats);

};