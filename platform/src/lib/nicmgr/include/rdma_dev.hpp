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

#define HBM_PAGE_SIZE 4096
#define HBM_PAGE_SIZE_SHIFT 12

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
    uint32_t log_num_pt_entries:7;

    uint32_t cqcb_base_addr_hi:24;
    uint32_t sqcb_base_addr_hi:24;
    uint32_t rqcb_base_addr_hi:24;    
    
    uint32_t log_num_cq_entries:5;

    uint32_t prefetch_pool_base_addr_page_id:22;
    uint32_t log_num_prefetch_pool_entries:5;
    uint32_t sq_qtype: 3;
    uint32_t rq_qtype: 3;
    uint32_t aq_qtype: 3;
    uint32_t barmap_base_addr: 10;
    uint32_t barmap_size: 10;
} PACKED sram_lif_entry_t;

typedef struct key_entry_s {
    uint8_t          rsvd2[20];
    uint32_t         num_pt_entries_rsvd;
    uint32_t         mr_cookie;
    uint32_t         mr_l_key;
    uint32_t         qp: 24; //qp which bound the MW ?
    uint8_t          flags;
    uint32_t         rsvd1: 18;
    uint32_t         override_lif: 12;
    uint32_t         override_lif_vld: 1;
    uint32_t         host_addr: 1;
    uint32_t         pd;
    uint32_t         pt_base;
    uint64_t         base_va;
    uint32_t         len;
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

#define MAX_CQ_RINGS 3
#define MAX_CQ_HOST_RINGS 3

typedef struct cqcb_s {

    uint64_t  pt_next_pa;
    uint64_t  pt_pa;

    uint16_t  pad:15;
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
    uint32_t rsvd3: 8;
    
    uint64_t phy_base_addr;
    
    uint16_t rsvd2: 5;
    uint16_t ring_empty_sched_eval_done: 1;
    uint16_t log_num_wqes: 5;
    uint16_t log_wqe_size: 5;

    uint8_t  rsvd1[4];
    uint8_t  busy;
    uint8_t  first_pass;
    uint32_t map_count_completed;
    
    qpcb_ring_t           rings[MAX_AQ_RINGS];
    qpcb_intrinsic_base_t ring_header;
} aqcb0_t;
    
typedef struct aqcb1_s {
    uint8_t pad[30];

    uint64_t num_any;

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


#endif /*__RDMA_DEV_HPP__*/
