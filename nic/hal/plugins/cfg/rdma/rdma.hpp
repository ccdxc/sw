//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef _RDMA_HPP_
#define _RDMA_HPP_

#include "nic/include/base.hpp"
#include <memory>
#include <map>
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "gen/proto/rdma.pb.h"
#include "nic/include/hal.hpp"
#include "lib/bm_allocator/bm_allocator.hpp"

using rdma::RdmaServiceType;
using rdma::RdmaAhRequestMsg;
using rdma::RdmaAhResponseMsg;
using rdma::RdmaAhSpec;
using rdma::RdmaAhResponse;
using rdma::RdmaCqSpec;
using rdma::RdmaCqRequestMsg;
using rdma::RdmaCqResponse;
using rdma::RdmaCqResponseMsg;
using rdma::RdmaEqSpec;
using rdma::RdmaEqRequestMsg;
using rdma::RdmaEqResponse;
using rdma::RdmaEqResponseMsg;
using rdma::RdmaAqSpec;
using rdma::RdmaAqRequestMsg;
using rdma::RdmaAqResponseMsg;
using rdma::RdmaAqResponse;

namespace hal {

/* Extern functions called by lif plugin */
extern hal_ret_t rdma_lif_init(intf::LifSpec& spec, uint32_t lif_id);
extern uint64_t rdma_lif_pt_base_addr(uint32_t lif_id);
extern uint64_t rdma_lif_kt_base_addr(uint32_t lif_id);
extern uint64_t rdma_lif_dcqcn_profile_base_addr(uint32_t lif_id);
extern uint64_t rdma_lif_at_base_addr(uint32_t lif_id);
extern uint64_t rdma_lif_barmap_base_addr(uint32_t lif_id);

/* Services exposed by RDMA HAL */
hal_ret_t rdma_cq_create (RdmaCqSpec& spec, RdmaCqResponse *rsp);
hal_ret_t rdma_eq_create (RdmaEqSpec& spec, RdmaEqResponse *rsp);
hal_ret_t rdma_ah_create(RdmaAhSpec& spec, RdmaAhResponse *rsp);
hal_ret_t rdma_aq_create (RdmaAqSpec& spec, RdmaAqResponse *rsp);


class RDMAManager {
 public:
  RDMAManager();
  uint64_t HbmAlloc(uint32_t size);
  uint64_t hbm_base_;

 protected:

 private:
  std::unique_ptr<sdk::lib::BMAllocator> hbm_allocator_;

  // Track allocation size which are needed when we
  // free memory.
  std::map<uint64_t, uint64_t> allocation_sizes_;
};

extern RDMAManager *rdma_manager();

#define HOSTMEM_PAGE_SIZE  (1 << 12)  //4096 Bytes
#define MAX_LIFS           2048

#define PACKED __attribute__((__packed__))

#define SIZE_ALIGN(x, _size) (((x) + ((_size) - 1)) & ~(uint64_t)((_size) - 1))

#define HBM_PAGE_SIZE 4096
#define HBM_PAGE_SIZE_SHIFT 12
#define HBM_PAGE_ALIGN(x) SIZE_ALIGN(x, HBM_PAGE_SIZE)

#define HBM_BARMAP_BASE_SHIFT 23
#define HBM_BARMAP_SIZE_SHIFT 23

//pt_base_addr is 8-byte aligned as each entry in page table stores 64-bit address.
//hence when pt_base_addr is encoded in various data structures, bottom 3 bits are not
//stored. At the runtime pt_base_addr field is shifted left by 3 bits to get the
//actual pt_base_addr.
#define PT_BASE_ADDR_SHIFT 3
#define HDR_TEMP_ADDR_SHIFT 3
#define CQCB_ADDR_HI_SHIFT 10 // 24 bits of cqcb base addr, so shift 10 bits
#define SQCB_ADDR_HI_SHIFT 10 // 24 bits of cqcb base addr, so shift 10 bits
#define RQCB_ADDR_HI_SHIFT 10 // 24 bits of cqcb base addr, so shift 10 bits
    
#define SQCB_SIZE_SHIFT    9
#define RQCB_SIZE_SHIFT    9

// Header template, bytes size of header_template_t, aligned to 8 bytes
#define AH_ENTRY_T_SIZE_BYTES           72
// DCQCN CB, bytes size of dcqcn_cb_t, aligned to 8 bytes
#define DCQCN_CB_T_SIZE_BYTES           64
// ROME CB, 64 bytes
#define ROME_CB_T_SIZE_BYTES            64
// AH Table Entry has header template and DCQCN CB
#define AT_ENTRY_SIZE_BYTES             (AH_ENTRY_T_SIZE_BYTES + DCQCN_CB_T_SIZE_BYTES + ROME_CB_T_SIZE_BYTES)

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

typedef struct lif_init_attr_s {
    uint32_t max_qps;
    //uint32_t max_srqs;
    uint32_t max_cqs;
    uint32_t max_eqs;
    uint32_t max_pt_entries;
    uint32_t max_keys;
} lif_init_attr_t;

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

typedef struct p4_to_p4plus_rdma_hdr_s {
    uint16_t         flags; //roce_opcode_flags_t
    uint8_t          v4:1;
    union {
        uint32_t         v4;
        uint32_t         v6[4];
    }dip;
    uint16_t         payload_size;
} p4_to_p4plus_rdma_hdr_t;

typedef struct key_entry_s {
    uint8_t          rsvd2[16];
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
    union {
        struct {
            uint32_t         pt_base_32;
            uint32_t         pt_base;
        };
        uint64_t     phy_base_addr;
    };
    uint64_t         base_va;
    uint32_t         len;
    uint8_t          log_page_size;
    uint8_t          acc_ctrl;
    uint8_t          type  : 4; //mr_type_t
    uint8_t          state : 4;
    uint8_t          user_key;
} PACKED key_entry_t;


/*====================  TYPES.H ===================*/

#define BYTES_TO_BITS(__B) ((__B)*8)
#define BITS_TO_BYTES(__b) ((__b)/8)

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

// rome_sender_cb_t
typedef struct rome_sender_cb_s{
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

// rome_receiver_cb_t to store rome related params
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

#define CQ_RING_ID      RING_ID_0
#define CQ_ARM_RING_ID  RING_ID_1
#define CQ_SARM_RING_ID RING_ID_2

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
    uint8_t  rsvd1[3];
    
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

}  // namespace hal

#endif // _RDMA_HPP_
