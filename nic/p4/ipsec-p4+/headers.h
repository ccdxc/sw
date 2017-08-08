//#include <../tcp-p4+/types.h>

#define PACKED __attribute__((__packed__))
#define likely(_a) _a

#define IPSEC_MAX_MTU 9216

#define MAX_ICV_BYTES 32

#define IPSEC_WIN_REPLAY_MAX_DIFF 64

#define ETH_HDR  14
#define ETH_HDR_VLAN 18
#define IP_HDR 20
#define ESP_BASE 8
#define IV_DEF_SIZE 8
#define IPSEC_MAX_HEADROOM 

#define MAX_BLOCK_SIZE 16
#define MAX_PAD_BYTES 18 // 2 bytes for padlength and nexthdr.

// IPSEC P4+ pipeline errors
#define IPSEC_MTU_FAILURE (1 << 1)
#define IPSEC_BAD_SEQ_NO  (1 << 2)


#define ESP_FIXED_HDR_SIZE 8 

#define NIC_PAGE_SIZE (10 * 1024)

#define IPSEC_AH_V4_TRANSPORT_H2N    0
#define IPSEC_AH_V4_TRANSPORT_N2H    1
#define IPSEC_AH_V4_TUNNEL_H2N       2
#define IPSEC_AH_V4_TUNNEL_N2H       3
#define IPSEC_AH_V6_TRANSPORT_H2N    4
#define IPSEC_AH_V6_TRANSPORT_N2H    5
#define IPSEC_AH_V6_TUNNEL_H2N       6
#define IPSEC_AH_V6_TUNNEL_N2H       7

#define IPSEC_ESP_V4_TRANSPORT_H2N    0
#define IPSEC_ESP_V4_TRANSPORT_N2H    1
#define IPSEC_ESP_V4_TUNNEL_H2N       2
#define IPSEC_ESP_V4_TUNNEL_N2H       3
#define IPSEC_ESP_V6_TRANSPORT_H2N    4
#define IPSEC_ESP_V6_TRANSPORT_N2H    5
#define IPSEC_ESP_V6_TUNNEL_H2N       6
#define IPSEC_ESP_V6_TUNNEL_N2H       7

// Need to find out right values.
#define TM_P4_INGRESS 0
#define TM_P4_EGRESS  1
#define TM_P4_RXDMA   2
#define TM_P4_TXDMA   3
#define TM_P4_UPLINK  4

#define TM_P4_DEF_OQUEUE 0 

// Need to get reserved-lifs from P4 guys - for now assume below.
#define IPSEC_AH_SVC_LIF  1021
#define IPSEC_ESP_SVC_LIF 1022


typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned int be32;
typedef unsigned short u16;
typedef unsigned short be16;
typedef unsigned char u8;

typedef u64 timestamp_t;
typedef enum { false = 0, true = 1} bool;

#define DMA_WRITE(_src, _dst, _size) memcpy((void *)(u64)_dst, (void *)(u64)_src, _size)

#define TYPE_RING(name, num_slots)              \
  typedef struct nic_ ## name ## _ring_s                \
  {                                             \
    u32 alloc_idx;                              \
    u32 free_idx;                               \
    u64 slots[num_slots];                       \
  } PACKED nic_ ## name ## _ring_t;

#define DEFINE_RING(name, num_slots)            \
  nic_ ## name ## _ring_t nic_ ## name ## _ring;

#define DECLARE_RING(name, num_slots)           \
  TYPE_RING(name, num_slots);\
  nic_ ## name ## _ring_t nic_ ## name ## _ring;


#define RING_SLOTS(name)                                \
  sizeof(nic_ ## name ## _ring.slots)/sizeof(u64)

#define RING_ALLOC(p, name, type)                                       \
  p = (type)nic_ ## name ## _ring.slots[nic_ ## name ## _ring.alloc_idx ]; \
  nic_ ## name ## _ring.alloc_idx = (nic_ ## name ## _ring.alloc_idx + 1) & (RING_SLOTS(name)-1)

#define RING_FREE(p, name)                                              \
  nic_ ## name ## _ring.slots[nic_ ## name ## _ring.free_idx ] = (u64)p; \
  nic_ ## name ## _ring.free_idx = (nic_ ## name ## _ring.free_idx + 1) & (RING_SLOTS(name)-1);

#define RING_LEN(name)                                                  \
  ((nic_ ## name ## _ring.free_idx - nic_ ## name ## _ring.alloc_idx) & (RING_SLOTS(name)-1))

#define RING_EMPTY(name)   (RING_LEN(name) == 0)

#define RING_FULL(name) (RING_LEN(name) == (RING_SLOTS(name) - 1))

#define RING_READ(p, name, type) \
p = (type)nic_ ## name ## _ring.slots[nic_ ## name ## _ring.free_idx ]; \
nic_ ## name ## _ring.free_idx = (nic_ ## name ## _ring.free_idx + 1) & (RING_SLOTS(name)-1)

#define RING_WRITE(p, name, type) \
nic_ ## name ## _ring.slots[nic_ ## name ## _ring.alloc_idx ] = (u64)p; \
nic_ ## name ## _ring.alloc_idx = (nic_ ## name ## _ring.alloc_idx + 1) & (RING_SLOTS(name)-1)




typedef struct pkt_s {
    u8 bytes[0];
} pkt_t;

typedef struct nic_page_s
{
  //nic_page_hdr_t hdr;
  //unsigned char data[NIC_PAGE_SIZE - sizeof(nic_page_hdr_t)];
  unsigned char data[NIC_PAGE_SIZE];
} nic_page_t;

// right now a union of tx and rxdma fields - need to change as per

typedef struct p4_bridge_metadata_s  {
    u64 copp_metadata_policer_index : 8;
    u64 ddos_metadata_ddos_service_policer_idx : 16;
    u64 ddos_metadata_ddos_src_dst_policer_idx : 16;
    u64 flow_lkp_metadata_lkp_proto : 8;
    u64 flow_lkp_metadata_tcp_flags : 8;
    u64 flow_lkp_metadata_pkt_type : 8;
    u64 tunnel_metadata_tunnel_originate : 8;
    u64 tunnel_metadata_tunnel_terminate : 8;
    u64 rewrite_metadata_rewrite_index : 16;
    u64 rewrite_metadata_tunnel_rewrite_index : 16;
    u64 rewrite_metadata_tunnel_vnid : 24;
    u64 rewrite_metadata_entropy_hash : 16;
    u64 rewrite_metadata_mirror_session_id : 8;
    u64 rewrite_metadata_nat_ip[2];
    u64 rewrite_metadata_nat_l4_port : 16;
    u64 qos_metadata_cos : 8;
    u64 qos_metadata_dscp_en : 8;
    u64 qos_metadata_dscp : 8;
    u64 policer_metadata_egress_policer_index : 16;
    u64 l3_metadata_payload_length : 16;
    u64 control_metadata_bypass : 8;
    u64 control_metadata_flow_miss : 8;
    u64 control_metadata_src_lif : 16;
    u64 control_metadata_dst_lif : 16;
} PACKED p4_bridge_metadata_t;

// asic/capri/design/common/cap_phv_intr_fields.py
typedef struct cap_phv_intr_global_s {
    u32 tm_iport : 3;
    u32 tm_iq : 5;
    u32 tm_oport : 8;
    u32 bypass : 1;
    u32 drop : 1;
    u32 hw_error : 1;
    u32 tm_cpu : 1;
    u32 tm_replication_en : 1;
    u32 lif : 11;
    u64 timestamp : 48;
    u64 tm_span_session : 8;
    u64 error_bits : 4;
    u64 tm_instance_type : 4;
    u32 tm_replication_ptr : 16;
    u32 tm_q_depth : 14;
    u32 tm_oq : 5;
    u32 debug_trace : 1;
    u32 no_data : 1;
    u32 recirc : 1;
    u32 frame_size : 14;
    u32 recirc_count : 3;
    u32 qid : 24;
    u32 phv_type : 3;
    u32 dma_cmd_ptr : 6;
    u32 rx_splitter_offset : 10;
    u32 hdr_vld[4]; 
} cap_phv_intr_global_t;

typedef struct ipsec_rx_cb_s {
    u8 key_negotiated;
    u8 rekey_in_progress;
    //u8 key_size; // this goes inside the secure index - I do not need this for now
    u32 key_index;
    u8 iv_size;
    // or iv val ?, IV is 64 bits algos like aes-gcm and chacha20 anyway
    u64 iv; 
    u8 icv_size;
    u32 esn_hi;
    u32 esn_lo;
    u32 barco_enc_cmd;
    // will use in opq-data so that txdma knows ipsec-cb-value and where to start.
    u16 ipsec_cb_index; 
    // shall we store algo-type and ah/esp type etc ??
    u8 block_size;

    //replay
    u32 last_replay_win_seq;
    u8 replay_win_sz;
    // 64 size is enough and simple
    u64 replay_win_bmp;
} PACKED ipsec_rx_cb_t;  

typedef struct ipsec_rx_stats_cb_s {    
    //stats
    u64 rx_bytes;
    u64 rx_pkts;
    u32 barco_errors;
    u32 auth_errors;
    timestamp_t last_sa_done;
    timestamp_t last_seen;
} PACKED ipsec_rx_stats_cb_t;  

 
typedef struct ipsec_tx_cb_s {
    //u8 key_negotiated;
    //u8 rekey_in_progress;
    // key_size not required - will be stored in key-descriptor and known by 
    //key-type which is tied to algo 
    //u8 key_size;
    u32 key_index;
    u8 iv_size;
    // or iv val ?, IV is 64 bits algos like aes-gcm and chacha20 anyway
    u8 icv_size;
    u32 spi;
    u32 esn_lo;
    u64 iv;
    u32 esn_hi;
    u32 barco_enc_cmd;
    // will use in opq-data so that txdma knows ipsec-cb-value and where to start.
    u16 ipsec_cb_index; 
    // shall we store algo-type and ah/esp type etc ??
    u8 block_size;
} PACKED ipsec_tx_cb_t;

typedef struct ipsec_tx_stats_cb_t {
    //stats
    u64 tx_bytes;
    u64 tx_pkts;
    u32 barco_errors;
    timestamp_t last_sa_done;
    timestamp_t last_seen;
} PACKED ipsec_tx_stats_cb_t;  

typedef struct p4_2_p4plus_metadata_s {
    u16 payload_start;
    u16 payload_end;
    u16 forward_port : 11;
    u16 pad : 5;
    u8 l4_proto;
    u8 ip_hdr_size;
    u32 seq_no;
} PACKED p4_2_p4plus_metadata_t;

typedef struct p4plus_2_p4_metadata_s {
    u16 forward_port : 11;
    u16 pad : 5;
    u32 drop_reasons;
} PACKED p4plus_2_p4_metadata_t;

typedef struct p4plus_ipsec_rx_internal_s {
    u16 headroom_offset;
    u16 decrypt_offset;
    u16 icv_offset;
    u32 expected_seq_no;
    u16 ipsec_cb_index;
    u16 buf_size;
    u32 drop_mask;
    u8  icv[MAX_ICV_BYTES+2];
    u8 icv_size;
    u64 in_desc;
    u64 out_desc;
} p4plus_ipsec_rx_internal_t;
    
typedef struct p4plus_ipsec_internal_s {
    u8 headroom;
    u8 tailroom;
    u16 headroom_offset;
    u16 tailroom_offset;
    u8 pad_size;
    u8 pad_bytes[MAX_PAD_BYTES];
    u16 buf_size;
    u32 drop_mask;
    u8 iv_size;
    u8 icv_size;
    u16 ipsec_cb_index;
    u16 forward_port;
    u64 in_desc;
    u64 out_desc;
} PACKED p4plus_ipsec_internal_t;

typedef struct phv_s {
    cap_phv_intr_global_t intrinsic;
    p4_2_p4plus_metadata_t p42p4plus_hdr;
    p4plus_ipsec_internal_t ipsec_int;
    pkt_t ipsec_pkt;
} PACKED phv_t;

typedef struct phv_rx_s {
    cap_phv_intr_global_t intrinsic;
    p4_2_p4plus_metadata_t p42p4plus_hdr;
    p4plus_ipsec_rx_internal_t ipsec_int;
    pkt_t ipsec_pkt;
} phv_rx_t;

typedef struct phv_tx_s {
    cap_phv_intr_global_t intrinsic;
    p4_bridge_metadata_t bm;
    p4plus_2_p4_metadata_t p4plus2p4_hdr;
    u16 ipsec_cb_index;
    u16 num_bytes;
    u64 in_desc;
    u64 out_desc;
    u16 payload_offset;
    u16 payload_length;
} phv_tx_t;
 
typedef struct barco_symm_req_s {
    u64 in_addr;
    u64 out_addr;
    u32 enc_cmd;
    u32 key_index;
    u64 iv_addr;
    u64 auth_tag_addr;
    u32 hdr_size;
    u32 status;
    u32 opq_tag_val;
    u32 db_en : 1;
    u32 opq_tag_en : 1;
    u32 rsvd : 30; //pendando_cryptodma doc had this at the end - will see
    u16 sec_sz;
    u32 sec_num;
    u16 app_tag;
} barco_symm_req_t;

typedef struct nic_desc_entry_s
{
  u64 scratch;
  u32 free_pending:1,
      valid:1;
  u32 addr:30;
  u16 offset;
  u16 len;
} PACKED nic_desc_entry_t;

typedef struct nic_desc_entry_cb_s
{
  p4plus_ipsec_internal_t ipsec_int;
} PACKED nic_desc_entry_cb_t;

typedef struct nic_desc_s
{
  nic_desc_entry_cb_t dcb;
  nic_desc_entry_t entry;
} PACKED nic_desc_t;


typedef struct nic_decrypt_desc_entry_s
{
  u64 scratch;
  u32 free_pending:1,
      valid:1;
  u32 addr:30;
  u16 offset;
  u16 len;
} PACKED nic_decrypt_desc_entry_t;

typedef struct nic_decrypt_desc_entry_cb_s
{
  p4plus_ipsec_rx_internal_t ipsec_int;
} PACKED nic_decrypt_desc_entry_cb_t;


typedef struct nic_decrypt_desc_s {
    nic_decrypt_desc_entry_cb_t dcb;
    nic_decrypt_desc_entry_t entry;
} nic_decrypt_desc_t;

typedef struct ip_header_s {
    u32 version:4;
    u32 hl:4;
    u32 tos:8;
    u32 length:16;
    u32 id:16;
    u32 flags:3;
    u32 frag_offset:13;
    u32 ttl:8;
    u32 protocol:8;
    u32 checksum:16;
    u32 sip;
    u32 dip;
} ip_header_t;

#define IPSEC_PHV_HEADROOM  \
        (sizeof(cap_phv_intr_global_t)+ \
         sizeof(p4_bridge_metadata_t)+ \
         sizeof(p4plus_2_p4_metadata_t))   

#define IPSEC_PHV_HEADROOM_RX  \
        (sizeof(cap_phv_intr_global_t)+ \
         sizeof(p4plus_2_p4_metadata_t))   


DECLARE_RING(IPSEC_RX_DESC, 1024);
DECLARE_RING(IPSEC_TX_DESC, 1024);
DECLARE_RING(IPSEC_RX_PAGE, 1024);
DECLARE_RING(IPSEC_TX_PAGE, 1024);
DECLARE_RING(BRQ, 1024);

static inline timestamp_t get_curr_timestamp(void) 
{
    timestamp_t ts = 0;
    return ts;
}

