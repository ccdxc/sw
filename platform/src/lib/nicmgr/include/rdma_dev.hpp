/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __RDMA_DEV_HPP__
#define __RDMA_DEV_HPP__

#include "nic/sdk/include/sdk/ip.hpp"

#define PACKED __attribute__((__packed__))

#define PT_BASE_ADDR_SHIFT 3
#define HDR_TEMP_ADDR_SHIFT 3
#define RRQ_BASE_ADDR_SHIFT 3
#define RSQ_BASE_ADDR_SHIFT 3
#define HBM_SQ_BASE_ADDR_SHIFT 3
#define HBM_RQ_BASE_ADDR_SHIFT 3
#define CQCB_ADDR_HI_SHIFT 10 // 24 bits of cqcb base addr, so shift 10 bits
#define SQCB_ADDR_HI_SHIFT 10 // 24 bits of cqcb base addr, so shift 10 bits
#define RQCB_ADDR_HI_SHIFT 10 // 24 bits of cqcb base addr, so shift 10 bits

#define SQCB_SIZE_SHIFT 9
#define RQCB_SIZE_SHIFT 9

#define SIZE_ALIGN(x, _size) (((x) + ((_size) - 1)) & ~(uint64_t)((_size) - 1))

#define HBM_PAGE_SIZE 4096
#define HBM_PAGE_SIZE_SHIFT 12
#define HBM_PAGE_ALIGN(x) SIZE_ALIGN(x, HBM_PAGE_SIZE)

// Header template, bytes size of header_template_t, aligned to 8 bytes
#define AH_ENTRY_T_SIZE_BYTES           72
// DCQCN CB, bytes size of dcqcn_cb_t, aligned to 8 bytes
#define DCQCN_CB_T_SIZE_BYTES           64
// ROME CB, 64 bytes
#define ROME_CB_T_SIZE_BYTES            64
// AH Table Entry has header template, DCQCN CB and ROME CB
// TODO: congestion_mgmt_type is a LIF config, need to integrate with driver change. Now we are not allocating ROME_CB in hw
#define AT_ENTRY_SIZE_BYTES             (AH_ENTRY_T_SIZE_BYTES + DCQCN_CB_T_SIZE_BYTES)

typedef struct qpcb_ring_s {
    uint16_t  c_index;
    uint16_t  p_index;
} qpcb_ring_t;

typedef struct qpcb_intrinsic_s {
    uint16_t  pid;
    uint8_t   host_rings:4; // Swapping host/total nibbles since it is accessed by doorbell-module in Little-Endian.
    uint8_t   total_rings:4;
    uint8_t   eval_last;
    uint8_t   cos_selector;
    uint8_t   cosA:4; // Swapping nibbles cosA/cosB since it is accessed by doorbell-module in Little-Endian.
    uint8_t   cosB:4;
    uint8_t   rsvd;
    uint8_t   pc;
} PACKED qpcb_intrinsic_base_t;

#define EQ_RING_ID  RING_ID_0
#define MAX_EQ_RINGS 1

typedef struct eqcb_s {
    uint64_t  rsvd1[3];
    uint64_t  int_assert_addr;
    uint32_t  rsvd: 28;
    uint32_t  color: 1;
    uint32_t  int_enabled: 1;
    uint32_t  log_wqe_size: 5;
    uint32_t  log_num_wqes: 5;
    uint32_t  eq_id:24;
    uint32_t  rsvd0;
    uint64_t  eqe_base_addr;

    qpcb_ring_t           rings[MAX_EQ_RINGS];
    // intrinsic
    qpcb_intrinsic_base_t ring_header;
} PACKED eqcb_t;

// all the page_ids are encoded as 22-bits, assuming 4K page size (12-bits)
// appropriate shift will make 34-bit (22+12) hbm address.
typedef struct sram_lif_entry_s {
    uint32_t rdma_en_qtype_mask:8;
    uint32_t pt_base_addr_page_id:22;
    uint32_t ah_base_addr_page_id:22;
    uint32_t log_num_pt_entries:5;
    uint32_t log_num_kt_entries:5;
    uint32_t log_num_dcqcn_profiles:4;
    uint32_t log_num_ah_entries:4;

    uint32_t cqcb_base_addr_hi:24;
    uint32_t sqcb_base_addr_hi:24;
    uint32_t rqcb_base_addr_hi:24;

    uint32_t log_num_cq_entries:5;
    uint32_t log_num_sq_entries:5;
    uint32_t log_num_rq_entries:5;

    uint32_t prefetch_base_addr_page_id:22;
    uint32_t log_prefetch_buf_size:5;
    uint32_t sq_qtype: 3;
    uint32_t rq_qtype: 3;
    uint32_t aq_qtype: 3;
    uint32_t barmap_base_addr: 10;
    uint32_t barmap_size: 8;
} PACKED sram_lif_entry_t;

typedef struct key_entry_s {
    uint8_t          rsvd2[8];
    uint64_t         phy_base_addr;
    uint32_t         num_pt_entries_rsvd;
    uint32_t         mr_cookie;
    uint32_t         mr_l_key;
    uint32_t         qp: 24; //qp which bound the MW ?
    uint8_t          flags;
    uint32_t         rsvd1: 17;
    uint32_t         is_phy_address:1;
    uint32_t         override_lif: 12;
    uint32_t         override_lif_vld: 1;
    uint32_t         host_addr: 1;
    uint32_t         pd;
    uint32_t         pt_base;
    uint64_t         base_va;
    uint64_t         len;
    uint8_t          log_page_size;
    uint8_t          acc_ctrl;
    uint8_t          type  : 4; //mr_type_t
    uint8_t          state : 4;
    uint8_t          user_key;
} PACKED key_entry_t;

typedef struct udphdr_s {
    uint16_t   csum;
    uint16_t   length;
    uint16_t   dport;
    uint16_t   sport;
} PACKED udphdr_t;

typedef struct iphdr_s {
    ipv4_addr_t daddr;
    ipv4_addr_t saddr;
    uint16_t   check;
    uint8_t    protocol;
    uint8_t    ttl;
    uint16_t   frag_off;
    uint16_t   id;
    uint16_t   tot_len;
    uint8_t    tos;
    uint8_t    ihl:4;
    uint8_t    version:4;
/*The options start here. */
} PACKED iphdr_t;

typedef struct ipv6hdr_s {
    ipv6_addr_t  daddr;
    ipv6_addr_t  saddr;
    uint32_t   hop_limit:8;
    uint32_t   nh:8;
    uint32_t   payload_len:16;
    uint32_t   flow_label:20;
    uint32_t   tc:8;
    uint32_t   version:4;;
} PACKED ipv6hdr_t;


#define MAC_SIZE 6

typedef struct ethhdr_s {
    uint16_t  ethertype;
    uint8_t   smac[MAC_SIZE];
    uint8_t   dmac[MAC_SIZE];
} PACKED ethhdr_t;

typedef struct vlanhdr_s {
    uint16_t ethertype;
    uint16_t vlan: 12;
    uint16_t cfi: 1;
    uint16_t pri: 3;
} PACKED vlanhdr_t;

typedef struct header_template_v4_s {
    udphdr_t    udp;
    iphdr_t     ip;
    vlanhdr_t   vlan;
    ethhdr_t    eth;
} PACKED header_template_v4_t;

typedef struct header_template_v6_s {
    udphdr_t    udp;
    ipv6hdr_t   ip;
    vlanhdr_t   vlan;
    ethhdr_t    eth;
} PACKED header_template_v6_t;

typedef union header_template_s {
    header_template_v4_t v4;
    header_template_v6_t v6;
} PACKED header_template_t;

typedef struct ah_entry_s {
    header_template_t hdr_tmp;
    uint8_t ah_size;
} PACKED ah_entry_t;

//dcqcn_cb_t dynamically allocated to store dcqcn related info in HBM
typedef struct dcqcn_cb_s {
    uint64_t            cur_timestamp: 32; // For model testing only. Will be removed.
    uint8_t             num_sched_drop; // For model testing only. Number of times packet was scheduled
                                        // and dropped due to insufficient tokens.
    uint16_t            sq_cindex;
    uint64_t            token_bucket_size: 48;
    uint64_t            cur_avail_tokens: 48;
    uint16_t            delta_ticks_last_sched;
    uint64_t            last_sched_timestamp: 48;
    uint8_t             rsvd0: 2;
    uint32_t            log_sq_size: 5;
    uint8_t             max_rate_reached: 1;
    uint8_t             num_cnp_processed;
    uint8_t             num_cnp_rcvd;
    uint16_t            num_alpha_exp_cnt;
    uint16_t            timer_exp_cnt;
    uint16_t            byte_counter_exp_cnt;
    uint32_t            cur_byte_counter;
    uint16_t            alpha_value;
    uint32_t            target_rate;
    uint32_t            rate_enforced;
    uint16_t            g_val;
    uint16_t            timer_exp_thr;
    uint32_t            byte_counter_thr;
    uint64_t            last_cnp_timestamp: 48;
} PACKED dcqcn_cb_t;

//dcqcn_config_cb_t to store dcqcn profile initial values
typedef struct dcqcn_config_cb_s {
    uint8_t		np_incp_802p_prio;
    uint8_t		np_cnp_dscp;
    uint8_t		np_rsvd[2];
    uint16_t		rp_initial_alpha_value;
    uint16_t		rp_dce_tcp_g;
    uint32_t		rp_dce_tcp_rtt;
    uint32_t		rp_rate_reduce_monitor_period;
    uint32_t		rp_rate_to_set_on_first_cnp;
    uint32_t		rp_min_rate;
    uint8_t		rp_gd;
    uint8_t		rp_min_dec_fac;
    uint8_t		rp_clamp_flags;
    uint8_t		rp_threshold;
    uint32_t		rp_time_reset;
    uint32_t		rp_byte_reset;
    uint32_t		rp_ai_rate;
    uint32_t		rp_hai_rate;
    uint8_t		rp_rsvd[4];
} PACKED dcqcn_config_cb_t;

typedef struct rome_sender_cb_t{
    uint8_t  localClkResolution:8;   // power of 2 multiplier of local clk to define timestamp unit of measurement
    uint8_t  remoteClkRightShift:8;  // right shift after multiply to convert result into clk ticks at the remote clk rate
    uint32_t clkScaleFactor:32;     // scaling factor to convert clk ticks from local clk rate into remote clk rate
    uint32_t txMinRate:17;          // minimum pkt transmission rate in Mbps

    uint32_t remoteClkStartEpoch:32; // value of remote clk counter at the start of the current epoch
    uint64_t localClkStartEpoch:48;  // value of local hardware clk free-running counter at the start of the current epoch

    uint32_t totalBytesSent:32;   // total bytes transmitted on this flow modulo 2^32
    uint32_t totalBytesAcked:32;  // total bytes acknowledged by receiver on this flow modulo 2^32
    uint32_t window:32;           // window size, max tx bytes allowed before ack
    uint32_t currentRate:27;      // current pkt transmission rate in Kbps
    uint32_t  log_sq_size:5;
    uint32_t numCnpPktsRx:32;     // number of CNP pkts received on this flow

    uint64_t last_sched_timestamp: 48;
    uint16_t delta_ticks_last_sched;
    uint64_t cur_avail_tokens: 48;
    uint64_t token_bucket_size: 48;
    uint16_t sq_cindex;
    uint64_t cur_timestamp: 32;
} PACKED rome_sender_cb_t;

typedef struct rome_receiver_cb_s{
    uint8_t state:3;           // flowStateEnum, current state of flow state machine
    uint8_t rsvd0:5;

    uint32_t minTimestampDiff; // minimum observed difference between local and remote integer timestamps

    uint8_t linkDataRate:2;    // data transmission rate on wire in Kbps, set to the minimum of sender and receiver link rate, potentially only 2 bit to indicate 25G, 50G, 40G and 100G?
    uint32_t recoverRate:27;   // RxMDA: target transmission rate in Kbps
    uint32_t minRate:27;       //lowest transmission rate we can set on any flow in Kbps
    
    uint8_t weight:4;          // weight determines the Additive Increase amount: MinRate*weight
    uint8_t rxDMA_tick:3;      // RxMDA: tick used to sync up RxDMA and TxDMA
    uint8_t wait:1;            // RxMDA: waiting for TxDMA to finish

    uint32_t avgDelay:20;      // RxMDA: exponentially weighted moving average of one-way pkt delay
    uint32_t preAvgDelay:20;   // TxMDA: value of avgDelay from the previous update period
    uint32_t cycleMinDelay:20; // RxMDA: min pkt latency observed during current cycle of 3 update periods
    uint32_t cycleMaxDelay:20; // RxMDA: max pkt latency observed during current cycle of 3 update periods

    uint32_t totalBytesRx;      // RxMDA: total bytes received by the QP, used to return credit in credit loop
    uint16_t rxBurstBytes;      // RxMDA: credit accumulator for credit loop in bytes
    uint16_t byte_update;       // RxMDA: bytes received since last update
    uint32_t th_byte;           // RxMDA: bytes accumulated since last throughput update

    uint16_t cur_timestamp:10;  // For debugging on Model since model doesnt have timestamps
    uint32_t thput:27;          // RxMDA: the current measure of flow throughput
    uint32_t MD_amount:27;      // RxMDA: accumulated MD rate deduction
    uint32_t last_cycle;        // RxMDA: time of last min/max latency cycle reset in units of localClk
    uint32_t last_thput;        // RxMDA: time of last throughput measurement
    uint32_t last_epoch;        // RxMDA: time of last epoch of target delay reset
    uint32_t last_update;       // RxMDA: time of last periodic update

    uint8_t txDMA_tick:3;       // TxMDA: tick used to sync up RxDMA and TxDMA
    uint16_t fspeed_cnt:10;     // TxMDA: number of update-periods after reaching full speed
    uint32_t currentRate:27;    // TxMDA: current transmission rate in Kbps, changed in TxDMA
} PACKED rome_receiver_cb_t;

typedef struct sge_s {
    uint32_t l_key;
    uint32_t len;
    uint64_t va;
} sge_t;

typedef struct rrqwqe_read_s {
    uint32_t     len;
    uint64_t     wqe_sge_list_ptr;
    uint8_t      rsvd1[44];
} PACKED rrqwqe_read_t;

typedef struct rrqwqe_atomic_s {
    sge_t  sge;
    uint32_t    op_type:4;
    uint32_t    rsvd:28;
    uint8_t     rsvd1[40];
} PACKED rrqwqe_atomic_t;

typedef struct rrqwqe_s {
    uint32_t read_resp_or_atomic: 1;
    uint32_t num_sges: 7;
    uint32_t psn: 24;
    uint32_t msn:24;
    uint32_t rsvd:8;
    union {
        rrqwqe_read_t   read;
        rrqwqe_atomic_t atomic;
    } op;
} PACKED rrqwqe_t;

typedef struct rqwqe_base_s {
    uint64_t     rsvd2[2];
    uint8_t      rsvd1[7];
    uint8_t      num_sges;
    uint64_t     wrid;
} PACKED rqwqe_base_t;

typedef struct rqwqe_s {
    sge_t        sge1;
    sge_t        sge0;
    rqwqe_base_t base;
} PACKED rqwqe_t;

typedef struct rsqwqe_read_s {
    uint32_t r_key;
    uint64_t va;
    uint32_t len;
    uint32_t offset;
} PACKED rsqwqe_read_t;

typedef struct rsqwqe_atomic_s {
    uint32_t r_key;
    uint64_t va;
    uint64_t orig_data;
} PACKED rsqwqe_atomic_t;

#define RSQ_OP_TYPE_READ 0
#define RSQ_OP_TYPE_ATOMIC 1

typedef struct rsqwqe_s {
    uint32_t read_or_atomic: 1;
    uint32_t ready:1;
    uint32_t rsvd: 6;
    uint32_t psn: 24;
    uint64_t rsvd2; // to keep RSQWQE 32 bytes
    union {
        rsqwqe_read_t   read;
        rsqwqe_atomic_t atomic;
    } op;
} PACKED rsqwqe_t;

#define MAX_RQ_PREFETCH_ACTIVE_QPS 1024
#define RQ_PREFETCH_WQE_SIZE       64
#define RQ_PREFETCH_RING_PG_OFFSET 1
#define RQ_PREFETCH_BUF_PG_OFFSET  2

typedef struct rq_prefetch_cb_s {
    uint16_t p_index;
    uint16_t c_index;
    uint32_t rsvd0;
    uint64_t rsvd1;
    uint64_t rsvd2;
    uint64_t rsvd3;
    uint64_t rsvd4;
    uint64_t rsvd5;
    uint64_t rsvd6;
    uint64_t rsvd7;
} PACKED rq_prefetch_cb_t;

typedef struct rq_prefetch_ring_s {
    uint16_t val;
} PACKED rq_prefetch_ring_t;

#define MAX_CQ_RINGS 3
#define MAX_CQ_HOST_RINGS 3

typedef struct cqcb_s {

    uint64_t  pt_next_pa;
    uint64_t  pt_pa;

    uint16_t  pad:14;
    uint16_t  is_phy_addr:1;
    uint16_t  host_addr:1;
    uint16_t  pt_next_pg_index;
    uint16_t  pt_pg_index;

    uint32_t cq_full:1;

    uint32_t cq_full_hint:1;
    uint32_t wakeup_ring_id:3;
    uint32_t wakeup_qid:24;
    uint32_t wakeup_qtype:3;
    uint32_t wakeup_lif:12;

    uint32_t color:1;
    uint32_t wakeup_dpath:1;

    uint32_t sarm:1;
    uint32_t arm:1;

    uint32_t eq_id:24;
    uint32_t cq_id:24;
    uint32_t ring_empty_sched_eval_done:1;
    uint32_t log_num_wqes:5;
    uint32_t log_wqe_size:5;
    uint32_t log_cq_page_size:5;

    uint32_t pt_base_addr;

    uint16_t proxy_s_pindex;

    uint16_t proxy_pindex;

    qpcb_ring_t           rings[MAX_CQ_RINGS];

    // intrinsic
    qpcb_intrinsic_base_t ring_header;
} PACKED cqcb_t;

#define MAX_AQ_RINGS 1
#define MAX_AQ_HOST_RINGS 1
    
typedef struct aqcb0_s {
    uint8_t  pad[16];

    uint64_t cqcb_addr;

    uint32_t error: 8;
    uint32_t cq_id: 24;

    uint32_t aq_id: 24;
    uint32_t next_token_id: 8;

    uint64_t phy_base_addr;

    uint16_t rsvd2: 5;
    uint16_t ring_empty_sched_eval_done: 1;
    uint16_t log_num_wqes: 5;
    uint16_t log_wqe_size: 5;

    uint8_t  debug;
    uint8_t  uplink_num;
    uint8_t  rsvd1[2];

    uint8_t  token_id;
    uint8_t  first_pass;
    uint32_t map_count_completed;

    qpcb_ring_t           rings[MAX_AQ_RINGS];
    qpcb_intrinsic_base_t ring_header;
} aqcb0_t;

typedef struct aqcb1_s {
    uint8_t pad[24];

    uint64_t num_any;

    uint16_t num_destroy_ah;
    uint16_t num_query_ah;
    uint16_t num_create_ah;
    uint16_t num_stats_dump;
    uint16_t num_destroy_qp;
    uint16_t num_query_qp;
    uint16_t num_modify_qp;
    uint16_t num_destroy_cq;
    uint16_t num_resize_cq;
    uint16_t num_dereg_mr;
    uint16_t num_stats_vals;
    uint16_t num_stats_hdrs;
    uint16_t num_reg_mr;
    uint16_t num_create_qp;
    uint16_t num_create_cq;
    uint16_t num_nop;
} PACKED aqcb1_t;

typedef struct aqcb_s {
    aqcb0_t aqcb0;
    aqcb1_t aqcb1;
} aqcb_t;

typedef struct sqcb0_s {

    uint8_t  rsvd2: 3;
    // 0 - congestion_mgmt_disabled; 1 - DCQCN; 2 - ROME; 3 - RSVD
    uint8_t  congestion_mgmt_type          : 2;
    uint8_t  bktrack_marker_in_progress    : 1;
    uint8_t  skip_pt                       : 1;
    uint8_t  spec_enable                   : 1;

    uint32_t state_flags                   : 8;
    uint32_t rsvd_state_flags              : 7;
    uint32_t sq_drained                    : 1;
    uint32_t msg_psn                       : 24;
    uint32_t spec_msg_psn                  : 24;

    uint64_t curr_wqe_ptr;
    uint32_t spec_sq_cindex               : 16;
    uint8_t  ring_empty_sched_eval_done    : 1;
    uint8_t  local_ack_timeout             : 5;
    uint8_t  rsvd5                         : 1;
    uint8_t  sq_in_hbm                     : 1;

    uint32_t state                         : 3;
    uint32_t flush_rq                      : 1;
    uint32_t service                       : 4;

    uint32_t dcqcn_rl_failure              : 1;
    uint32_t signalled_completion          : 1;
    uint32_t poll_for_work                 : 1;
    uint32_t log_num_wqes                  : 5;
    uint16_t log_wqe_size                  : 5;
    uint16_t log_sq_page_size              : 5;
    uint16_t log_pmtu                      : 5;
    uint16_t poll_in_progress              : 1;

    uint32_t pd;
    uint32_t header_template_addr;
    uint64_t base_addr                     : 40;
    uint64_t rsvd1                         :  8;
    uint64_t sqd_cindex                    : 16;

    uint16_t c_index3;
    uint16_t p_index3;
    uint16_t c_index2;
    uint16_t p_index2;
    uint16_t c_index1;
    uint16_t p_index1;
    uint16_t c_index0;
    uint16_t p_index0;

    uint16_t pid: 16;
    uint8_t  host_rings: 4;
    uint8_t  total_rings: 4;

    uint8_t  eval_last;
    uint8_t  cos_selector;

    uint8_t  cosA: 4;
    uint8_t  cosB: 4;
    uint8_t  rsvd;
    uint8_t  pc;

} PACKED sqcb0_t;

typedef struct sqcb1_s {

    uint16_t rsvd7;
    uint16_t rrq_spec_cindex;
    uint32_t pd;
    uint8_t  bktrack_in_progress;

    uint8_t  rsvd6: 2;
    uint8_t  sqd_async_notify_enable: 1;
    uint8_t  sq_drained: 1;
    uint8_t  sqcb1_priv_oper_enable: 1;
    uint8_t  state:3;
    uint32_t rrqwqe_cur_sge_offset;
    uint8_t  rrqwqe_cur_sge_id;
    uint8_t  rrqwqe_num_sges;

    uint32_t max_ssn:24;
    uint32_t max_tx_psn:24;

    uint32_t rsvd5:1;
    uint32_t congestion_mgmt_type: 2;
    uint32_t credits:5;

    uint32_t msn:24;
    uint32_t rexmit_psn:24;
    uint32_t rsvd4:24;

    uint8_t  token_id;
    uint8_t  nxt_to_go_token_id;

    uint8_t  header_template_size;
    uint32_t header_template_addr;

    uint32_t rsvd3:2;
    uint32_t pkt_spec_enable:1;
    uint32_t log_sqwqe_size:5;
    uint32_t rsvd2:16;
    uint32_t ssn:24;
    uint32_t tx_psn:24;      //tx

    uint32_t work_not_done_recirc_cnt: 8;
    uint32_t rnr_retry_count : 3;
    uint32_t err_retry_count : 3;
    uint32_t log_pmtu: 5;
    uint32_t rsvd8:1 ;
    uint32_t service:4;
    uint8_t  log_rrq_size;
    uint32_t rrq_base_addr;  //common

    uint16_t rsvd1;
    // RRQ RING for rxdma
    uint8_t  rrq_cindex;
    uint8_t  rrq_pindex;
    uint32_t cq_id:24;       //rx
    uint32_t pc:8;

} PACKED sqcb1_t;

typedef struct sqcb2_s {

    uint32_t sq_msg_psn:24;
    uint32_t rsvd3:7;
    uint32_t disable_credits:1;
    uint32_t timestamp:16;
    uint32_t exp_rsp_psn:24;

    uint8_t  curr_op_type:5;
    uint8_t  fence_done:1;
    uint8_t  li_fence:1;
    uint8_t  fence:1;
    uint16_t rsvd2:14;
    uint16_t need_credits:1;
    uint16_t busy:1;
    uint8_t  rrq_cindex;
    uint8_t  rrq_pindex;
    uint16_t sq_cindex;
    uint32_t inv_key;

    uint64_t wqe_start_psn:24;
    uint64_t lsn:24;
    uint64_t ssn:24;
    uint64_t tx_psn:24;      //tx

    uint64_t local_ack_timeout:5;
    uint64_t timer_on:1;
    uint64_t rsvd1:2;

    uint64_t rnr_timeout:8;
    uint64_t rnr_retry_ctr:4;
    uint64_t err_retry_ctr:4;
    uint64_t last_ack_or_req_ts:48;

    uint64_t rexmit_psn:24;
    uint32_t lsn_rx:24;
    uint32_t lsn_tx:24;

    uint32_t service:4;
    uint32_t roce_opt_mss_enable:1;
    uint32_t roce_opt_ts_enable:1;

    uint32_t log_sq_size:5;
    uint32_t log_rrq_size:5;
    uint32_t rrq_base_addr;  //common

    uint32_t header_template_addr;
    uint32_t header_template_size:8;
    uint32_t dst_qp:24;      //tx

} PACKED sqcb2_t;

typedef struct sqcb3_s {

    uint64_t  rsvd[8];
} PACKED sqcb3_t;

typedef struct sqcb4_s {

    uint64_t qp_err_dis_rsvd                          : 26;
    uint64_t qp_err_dis_rnr_retry_exceed              : 1;
    uint64_t qp_err_dis_err_retry_exceed              : 1;
    uint64_t qp_err_dis_inv_optype                    : 1;
    uint64_t qp_err_dis_table_resp_error              : 1;
    uint64_t qp_err_dis_phv_intrinsic_error           : 1;
    uint64_t qp_err_dis_table_error                   : 1;
    uint64_t qp_err_dis_inv_lkey_inv_not_allowed      : 1;
    uint64_t qp_err_dis_inv_lkey_invalid_state        : 1;
    uint64_t qp_err_dis_inv_lkey_pd_mismatch          : 1;
    uint64_t qp_err_dis_inv_lkey_qp_mismatch          : 1;
    uint64_t qp_err_dis_frpmr_ukey_not_enabled        : 1;
    uint64_t qp_err_dis_frpmr_invalid_len             : 1;
    uint64_t qp_err_dis_frpmr_invalid_state           : 1;
    uint64_t qp_err_dis_frpmr_invalid_pd              : 1;
    uint64_t qp_err_dis_frpmr_fast_reg_not_enabled    : 1;
    uint64_t qp_err_dis_bktrack_inv_rexmit_psn        : 1;
    uint64_t qp_err_dis_bktrack_inv_num_sges          : 1;
    uint64_t qp_err_dis_bind_mw_lkey_invalid_va       : 1;
    uint64_t qp_err_dis_bind_mw_lkey_invalid_acc_ctrl : 1;
    uint64_t qp_err_dis_bind_mw_lkey_zero_based       : 1;
    uint64_t qp_err_dis_bind_mw_lkey_no_bind          : 1;
    uint64_t qp_err_dis_bind_mw_lkey_state_valid      : 1;
    uint64_t qp_err_dis_bind_mw_rkey_type_disallowed  : 1;
    uint64_t qp_err_dis_bind_mw_rkey_inv_mw_state     : 1;
    uint64_t qp_err_dis_bind_mw_rkey_inv_len          : 1;
    uint64_t qp_err_dis_bind_mw_rkey_inv_zbva         : 1;
    uint64_t qp_err_dis_bind_mw_rkey_inv_pd           : 1;
    uint64_t qp_err_dis_bind_mw_len_exceeded          : 1;
    uint64_t qp_err_dis_lkey_access_violation :    1;
    uint64_t qp_err_dis_lkey_rsvd_lkey        :    1;
    uint64_t qp_err_dis_lkey_inv_pd           :    1;
    uint64_t qp_err_dis_lkey_inv_state        :    1;
    uint64_t qp_err_dis_no_dma_cmds           :    1;
    uint64_t qp_err_dis_ud_priv               :    1;
    uint64_t qp_err_dis_ud_fast_reg           :    1;
    uint64_t qp_err_dis_ud_pmtu               :    1;
    uint64_t qp_err_dis_flush_rq              :    1;
    uint64_t qp_err_disabled                  :    1;
    uint16_t rp_num_byte_threshold_db;
    uint16_t np_cnp_sent;
    uint16_t num_sq_drains;
    uint16_t num_timeout_rnr;
    uint16_t num_timeout_local_ack;
    uint16_t num_inline_req;
    uint16_t num_npg_local_inv_req;
    uint16_t num_npg_frpmr_req;
    uint16_t num_npg_bindmw_t2_req;
    uint16_t num_npg_bindmw_t1_req;
    uint32_t num_npg_req;
    uint16_t max_pkts_in_any_msg;
    uint16_t num_pkts_in_cur_msg;
    uint16_t num_write_msgs_imm_data;
    uint16_t num_send_msgs_imm_data;
    uint16_t num_send_msgs_inv_rkey;
    uint16_t num_atomic_cswap_msgs;
    uint16_t num_atomic_fna_msgs;
    uint16_t num_read_req_msgs;
    uint16_t num_write_msgs;
    uint16_t num_send_msgs;
    uint32_t num_pkts;
    uint64_t num_bytes;

} PACKED sqcb4_t;

typedef struct sqcb5_s {

    uint8_t rsvd[27];
    uint8_t rsvd1: 3;
    uint8_t max_recirc_cnt_err: 1;
    uint8_t recirc_reason: 4;
    uint32_t recirc_bth_opcode: 8;
    //a packet which went thru too many recirculations in req_rx had to be terminated and qp had to
    //be put into error disabled state. The recirc reason, opcode, the psn of the packet etc.
    //are remembered for further debugging.
    uint32_t recirc_bth_psn: 24;

    uint32_t qp_err_dis_rsvd                  :    16;
    uint32_t qp_err_dis_table_resp_error      :    1;
    uint32_t qp_err_dis_phv_intrinsic_error   :    1;
    uint32_t qp_err_dis_table_error           :    1;

    uint32_t qp_err_dis_rrqwqe_remote_oper_err_rcvd    :    1;
    uint32_t qp_err_dis_rrqwqe_remote_acc_err_rcvd     :    1;
    uint32_t qp_err_dis_rrqwqe_remote_inv_req_err_rcvd :    1;

    uint32_t qp_err_dis_rrqsge_insuff_dma_cmds:    1;
    uint32_t qp_err_dis_rrqsge_insuff_sge_len :    1;
    uint32_t qp_err_dis_rrqsge_insuff_sges    :    1;

    uint32_t qp_err_dis_rrqlkey_acc_len_higher:    1;
    uint32_t qp_err_dis_rrqlkey_acc_len_lower :    1;
    uint32_t qp_err_dis_rrqlkey_acc_no_wr_perm:    1;
    uint32_t qp_err_dis_rrqlkey_rsvd_lkey     :    1;
    uint32_t qp_err_dis_rrqlkey_inv_state     :    1;
    uint32_t qp_err_dis_rrqlkey_pd_mismatch   :    1;
    uint32_t qp_err_disabled                  :    1;

    uint16_t max_pkts_in_any_msg;
    uint16_t num_pkts_in_cur_msg;
    uint16_t num_atomic_ack;
    uint16_t num_ack;
    uint16_t num_feedback;
    uint16_t num_read_resp_msgs;
    uint32_t num_read_resp_pkts;
    uint32_t num_pkts;
    uint64_t num_bytes;

} PACKED sqcb5_t;

typedef struct sqcb_s {
    sqcb0_t sqcb0;
    sqcb1_t sqcb1;
    sqcb2_t sqcb2;
    sqcb3_t sqcb3;
    sqcb4_t sqcb4;
    sqcb5_t sqcb5;
    uint64_t pad[16];
} PACKED sqcb_t;

typedef struct rqcb0_s {
	uint32_t     log_pmtu: 5;                //Ronly
	uint32_t     serv_type: 3;               //Ronly
	 
	uint32_t     bt_rsq_cindex: 16;          //Read by S0, write by Sx
	 
	uint32_t     rsvd1: 3;
	uint32_t     dcqcn_cfg_id: 4;            //Ronly
	uint32_t     ring_empty_sched_eval_done: 1;  //rw in S0

	uint32_t     header_template_size: 8;    //Ronly
	 
	uint32_t     curr_read_rsp_psn: 24;      //Written by S5, Read by S0 and S5
	 
	uint32_t     rsvd0: 2;
	uint32_t     drain_done: 1;              //Written by S5, Read by S0
	uint32_t     bt_in_progress: 1;          //RW by S0 and Sx
	uint32_t     bt_lock: 1;                 //Rw by S0 and Sx
	uint32_t     rq_in_hbm: 1;               //Ronly
	uint32_t     read_rsp_in_progress: 1;    //Rw by S0 and S4 ?
	uint32_t     curr_color: 1;              //Written by S5, Read by S0
	uint32_t     dst_qp: 24;                 //Ronly
	 
	uint32_t     header_template_addr: 32;   //Ronly
	 
	uint32_t     rsvd: 4;
	uint32_t     prefetch_en: 1;             //Rw by S0
	uint32_t     skip_pt: 1;
	uint32_t     drain_in_progress: 1;       //Rw by S0
	uint32_t     spec_color: 1;              //Rw by S0
	uint32_t     spec_read_rsp_psn: 24;      //Written by S0, Read by S5
	 
	uint32_t     rsq_base_addr: 32;      //Ronly
	 
	uint64_t     phy_base_addr             : 40;            
	 
	uint64_t     log_rsq_size: 5;            //Ronly
	uint64_t     state: 3;                   //Ronly?
	uint64_t     congestion_mgmt_enable:1;   //Ronly
	uint64_t     log_num_wqes: 5;            //Ronly
	uint64_t     log_wqe_size: 5;            //Ronly
	uint64_t     log_rq_page_size: 5;        //Ronly
	 
	uint32_t     c_index5: 16;
	uint32_t     p_index5: 16;
	uint32_t     c_index4: 16;
	uint32_t     p_index4: 16;
	uint32_t     c_index3: 16;
	uint32_t     p_index3: 16;
	uint32_t     c_index2: 16;
	uint32_t     p_index2: 16;
	uint32_t     c_index1: 16;
	uint32_t     p_index1: 16;
	uint32_t     c_index0: 16;
	uint32_t     p_index0: 16;
	 
	uint32_t     pid: 16;
	uint32_t     host_rings: 4;
	uint32_t     total_rings: 4;
	uint32_t     eval_last: 8;

	uint32_t     cos_selector: 8;
	uint32_t     cosA: 4;
	uint32_t     cosB: 4;
	uint32_t     intrinsic_rsvd: 8;
	uint32_t     pc: 8;  
} PACKED rqcb0_t;

typedef struct rqcb1_s {
	uint32_t     log_pmtu: 5;            //Ronly
	uint32_t     serv_type: 3;           //Ronly
	
	uint32_t     srq_id: 24;
	
	uint32_t     proxy_pindex: 16;   // written by TxDMA, read by RxDMA
	uint32_t     proxy_cindex: 16;   // written by S4, read by S0
	
	uint32_t     num_sges: 8;
	uint32_t     current_sge_id: 8;
	
	uint32_t     current_sge_offset: 32;
	
	uint64_t     curr_wqe_ptr: 64;
	
	uint32_t     rsq_pindex: 8;  // written by S0
	
	uint32_t     bt_in_progress: 8;  // set to 1 by rxdma, to 0 by txdma
	
	uint32_t     header_template_size: 8;    //Ronly
	uint32_t     msn:24;                 //rw by S0 ?
	
	uint32_t     rsvd3: 1;
	uint32_t     access_flags: 3; // rw by S0
	uint32_t     spec_en: 1; //rw by S0
	uint32_t     next_pkt_type: 1; //rw by S0
	uint32_t     next_op_type: 2;  //rw by S0
	uint32_t     e_psn: 24;        //rw by S0
	
	uint32_t     spec_cindex: 16;  // cindex used for speculation
	uint32_t     rsvd2: 7;
	uint32_t     in_progress: 1;         // written by S4, read by S0
	uint32_t     rsvd1: 7;
	uint32_t     busy: 1; // set to 1 by S0, to 0 by S3
	
	uint32_t     work_not_done_recirc_cnt: 8; //rw by S0
	uint32_t     nxt_to_go_token_id: 8;  // written by S4, read by S0
	uint32_t     token_id: 8;            //rw by S0
	
	uint32_t     header_template_addr: 32;   //Ronly
	
	uint32_t     pd: 32;                 //Ronly
	
	uint32_t     cq_id: 24;              //Ronly
	
	uint32_t     prefetch_en: 1;         //Ronly
	uint32_t     skip_pt :1;
	uint32_t     priv_oper_enable: 1;    //Ronly
	uint32_t     nak_prune: 1;           //rw by S0
	uint32_t     rq_in_hbm: 1;           //Ronly
	uint32_t     immdt_as_dbell: 1;      //Ronly
	uint32_t     cache: 1;               //Ronly
	uint32_t     srq_enabled: 1;         //Ronly
	
	uint64_t     log_rsq_size: 5;        //Ronly     
	uint64_t     state: 3;               //Ronly ?
	uint64_t     congestion_mgmt_enable:1;   //Ronly
	uint64_t     log_num_wqes: 5;        //Ronly
	uint64_t     log_wqe_size: 5;        //Ronly
	uint64_t     log_rq_page_size: 5;    //Ronly
	 
	uint64_t     phy_base_addr             : 40;            
	
	uint32_t     rsq_base_addr: 32;  //Ronly
	
	uint8_t     pc: 8;
} PACKED rqcb1_t;	

typedef struct rqcb2_s {
    //word0-3
	uint64_t     pad0;
    uint64_t     pad1;

    //word4
    uint64_t     pad2:6;
	uint32_t     prefetch_init_done: 1;
	uint32_t     checkout_done: 1;
	uint32_t     prefetch_buf_index: 16;
	uint32_t     rsvd3       : 3;
	uint32_t     log_num_prefetch_wqes: 5;

    //word5
	uint32_t     prefetch_base_addr: 32; //Ronly

    //word6
	uint32_t     prefetch_proxy_cindex: 16;
	uint32_t     prefetch_pindex: 16;

    //9 words from here
	uint32_t     proxy_cindex: 16;

	uint32_t     pd: 32; //4B

	uint32_t     rsvd2: 3;
	uint32_t     rnr_timeout: 5;

	uint32_t     len: 32;
	
    uint32_t     r_key: 32;
	
    uint64_t     va: 64;

	uint32_t     psn: 24;
	uint32_t     rsvd: 7;
	uint32_t     read_or_atomic: 1;

	uint32_t     rsvd1       : 3;
	uint32_t     credits     : 5;
	
	uint32_t     syndrome    : 8;            //1B
	uint32_t     msn         : 24;           //3B

	uint32_t     ack_nak_psn: 24;            //3B
	uint32_t     rsvd0: 8;                    //1B
} PACKED rqcb2_t;

typedef struct rqcb3_s {
    uint64_t     resp_rx_timestamp;
    uint64_t     resp_tx_timestamp;
    uint32_t     pad; //4B

    uint32_t     dma_len: 32;
    uint32_t     num_pkts_in_curr_msg: 32;

    uint32_t     rsvd1:             16;
    uint32_t     roce_opt_mss:      16;
    uint32_t     roce_opt_ts_echo:  32;
    uint32_t     roce_opt_ts_value: 32;

    uint32_t     r_key: 32;
    uint32_t     len: 32;
    uint64_t     va: 64;
    uint64_t     wrid: 64;
} PACKED rqcb3_t;

typedef struct rqcb4_s {
	uint32_t     pad: 32;

	uint32_t     qp_err_dis_resp_rx                  :    1;

	uint32_t     qp_err_dis_type2a_mw_qp_mismatch    :    1;
	uint32_t     qp_err_dis_rkey_va_err              :    1;
	uint32_t     qp_err_dis_rkey_acc_ctrl_err        :    1;
	uint32_t     qp_err_dis_rkey_pd_mismatch         :    1;
	uint32_t     qp_err_dis_rkey_state_err           :    1;
	uint32_t     qp_err_dis_rsvd_rkey_err            :    1;

	uint32_t     qp_err_disabled                     :    1;

	uint32_t     rp_num_max_rate_reached             :   16;
	uint32_t     rp_num_timer_T_expiry               :   16;
	uint32_t     rp_num_alpha_timer_expiry           :   16;
	uint32_t     rp_num_byte_threshold_db            :   16;
	uint32_t     rp_num_hyper_increase               :   16;
	uint32_t     rp_num_fast_recovery                :   16;
	uint32_t     rp_num_additive_increase            :   16;

	uint32_t     last_msn: 24;
	uint32_t     last_syndrome: 8;
	uint32_t     last_psn: 24;

	uint32_t     num_seq_errs: 32;
	uint32_t     num_rnrs: 32;

	uint32_t     num_prefetch: 16;
	uint32_t     max_pkts_in_any_msg: 16;
	uint32_t     num_pkts_in_cur_msg: 16;
	uint32_t     num_atomic_resp_msgs: 16;
	uint32_t     num_read_resp_msgs: 16;
	uint32_t     num_acks: 32;
	uint32_t     num_read_resp_pkts: 32;
	uint32_t     num_pkts: 32;
	uint64_t     num_bytes: 64;
} PACKED rqcb4_t;

typedef struct rqcb5_s {
	uint64_t     pad: 43;
	uint32_t     max_recirc_cnt_err: 1;
	uint32_t     recirc_reason: 4;

	uint32_t     last_bth_opcode: 8;
	uint32_t     recirc_bth_psn: 24;

	uint32_t     qp_err_dis_rsvd                     :    2;
	uint32_t     qp_err_dis_table_resp_error         :    1;
	uint32_t     qp_err_dis_phv_intrinsic_error      :    1;
	uint32_t     qp_err_dis_table_error              :    1;

	uint32_t     qp_err_dis_feedback                 :    1;

	uint32_t     qp_err_dis_mr_cookie_mismatch       :    1;
	uint32_t     qp_err_dis_mr_state_invalid         :    1;

	uint32_t     qp_err_dis_mr_mw_pd_mismatch        :    1;
	uint32_t     qp_err_dis_type2a_mw_qp_mismatch    :    1; 
	uint32_t     qp_err_dis_type1_mw_inv_err         :    1; 
	uint32_t     qp_err_dis_inv_rkey_state_err       :    1; 
	uint32_t     qp_err_dis_ineligible_mr_err        :    1; 
	uint32_t     qp_err_dis_inv_rkey_rsvd_key_err    :    1; 

	uint32_t     qp_err_dis_key_va_err               :    1;
	uint32_t     qp_err_dis_user_key_err             :    1;
	uint32_t     qp_err_dis_key_acc_ctrl_err         :    1;
	uint32_t     qp_err_dis_key_pd_mismatch          :    1;
	uint32_t     qp_err_dis_key_state_err            :    1;
	uint32_t     qp_err_dis_rsvd_key_err             :    1;

	uint32_t     qp_err_dis_max_sge_err              :    1;
	uint32_t     qp_err_dis_insuff_sge_err           :    1;

	uint32_t     qp_err_dis_dma_len_err              :    1;

	uint32_t     qp_err_dis_unaligned_atomic_va_err  :    1;
	uint32_t     qp_err_dis_wr_only_zero_len_err     :    1;
	uint32_t     qp_err_dis_access_err               :    1;
	uint32_t     qp_err_dis_opcode_err               :    1;
	uint32_t     qp_err_dis_pmtu_err                 :    1;
	uint32_t     qp_err_dis_last_pkt_len_err         :    1;
	uint32_t     qp_err_dis_pyld_len_err             :    1;
	uint32_t     qp_err_dis_svc_type_err             :    1;

	uint32_t     qp_err_disabled                     :    1;

	uint32_t     rp_cnp_processed: 16;
	uint32_t     np_ecn_marked_packets: 16;

	uint32_t     num_dup_rd_atomic_drop_pkts: 16;
	uint32_t     num_dup_rd_atomic_bt_pkts: 16;
	uint32_t     num_dup_wr_send_pkts: 16;
	uint32_t     num_mem_window_inv: 16;
	uint32_t     num_recirc_drop_pkts: 16;
	uint32_t     max_pkts_in_any_msg : 16;
	uint32_t     num_pkts_in_cur_msg: 16;
	uint32_t     num_ring_dbell: 16;
	uint32_t     num_ack_requested: 16;
	uint32_t     num_write_msgs_imm_data: 16;
	uint32_t     num_send_msgs_imm_data: 16;
	uint32_t     num_send_msgs_inv_rkey: 16;
	uint32_t     num_atomic_cswap_msgs: 16;
	uint32_t     num_atomic_fna_msgs: 16;
	uint32_t     num_read_req_msgs: 16;
	uint32_t     num_write_msgs: 16;
	uint32_t     num_send_msgs: 16;
	uint32_t     num_pkts: 32;
	uint64_t     num_bytes: 64;
} PACKED rqcb5_t;

typedef struct rqcb_s {
    rqcb0_t rqcb0;
    rqcb1_t rqcb1;
    rqcb2_t rqcb2;
    rqcb3_t rqcb3;
    rqcb4_t rqcb4;
    rqcb5_t rqcb5;
    uint64_t pad[16];
} PACKED rqcb_t;

typedef struct cqcb0_s {
    uint64_t     pt_next_pa: 64;
    uint64_t     pt_pa: 64;

    uint32_t     pad: 14;
    uint32_t     is_phy_addr: 1;
    uint32_t     host_addr: 1;
    uint32_t     pt_next_pg_index: 16;
    uint32_t     pt_pg_index: 16;

    uint32_t     cq_full: 1;
    uint32_t     cq_full_hint: 1;
    uint32_t     wakeup_ring_id:3;
    uint32_t     wakeup_qid:24;
    uint32_t     wakeup_qtype:3;
    uint32_t     wakeup_lif:12;

    uint32_t     color: 1;
    uint32_t     wakeup_dpath:1;
    uint32_t     sarm: 1;
    uint32_t     arm: 1;

    uint32_t     eq_id: 24;
    uint32_t     cq_id: 24;

    uint32_t     ring_empty_sched_eval_done : 1;
    uint32_t     log_num_wqes: 5;
    uint32_t     log_wqe_size: 5;
    uint32_t     log_cq_page_size: 5;

    uint32_t     pt_base_addr: 32;

    uint32_t     proxy_s_pindex: 16;

    uint32_t     proxy_pindex: 16;

    uint32_t     c_index2: 16;
    uint32_t     p_index2: 16;
    uint32_t     c_index1: 16;
    uint32_t     p_index1: 16;
    uint32_t     c_index0: 16;
    uint32_t     p_index0: 16;

    uint32_t     pid: 16;
    uint32_t     host_rings: 4;
    uint32_t     total_rings: 4;
    uint32_t     eval_last: 8;
    uint32_t     cos_selector: 8;
    uint32_t     cosA: 4;
    uint32_t     cosB: 4;
    uint32_t     rsvd: 8;
    uint32_t     pc: 8;  
} PACKED cqcb0_t;

typedef struct eqcb0_s {    // 64 bytes
    uint64_t     rsvd2[3];
    uint64_t     int_assert_addr: 64;
    uint32_t     rsvd1: 28;
    uint32_t     color: 1;
    uint32_t     int_enabled: 1;
    uint32_t     log_wqe_size: 5;
    uint32_t     log_num_wqes: 5;
    uint32_t     eq_id: 24;
    uint32_t     rsvd0: 32;
    uint64_t     eqe_base_addr: 64;

    uint32_t     cindex: 16;
    uint32_t     pindex: 16;

    uint32_t     pid: 16;
    uint32_t     host_rings: 4;
    uint32_t     total_rings: 4;
    uint32_t     eval_last: 8;
    uint32_t     cos_selector: 8;
    uint32_t     cosa: 4;
    uint32_t     cosb: 4;
    uint32_t     rsvd: 8;
    uint32_t     pc: 8;  
} PACKED eqcb0_t;

#endif /*__RDMA_DEV_HPP__*/
