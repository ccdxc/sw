/*
 * tcp_proxy_rxdma_api.hpp
 * Saurabh Jain (Pensando Systems)
 */
#ifndef __TCP_PROXY_RXDMA_API_H__
#define __TCP_PROXY_RXDMA_API_H__

#include <stdint.h>
#include <tcp_proxy_rxdma_p4plus_ingress.h>
#include <tcp_proxy_txdma_p4plus_ingress.h>
#include <p4plus_pd_api.h>

#define P4PD_TCPCB_STAGE_ENTRY_OFFSET   64

// NOTE : ** Change proxy_meta_init to match qstate size
// Also Change TCP_TCB_TABLE_ENTRY_SIZE and TCP_TCB_TABLE_ENTRY_SIZE_SHFT
// in tcp-table.h to match **
#define P4PD_HBM_TCP_CB_ENTRY_SIZE      1024

// NOTE : ** These offsets need to match the offsets in 
// tcp-table.h **
typedef enum tcpcb_hwid_order_ {
    P4PD_HWID_TCP_TX_READ_RX2TX = 0,
    P4PD_HWID_TCP_RX_READ_TX2RX = 1,
    P4PD_HWID_TCP_TX_READ_RX2TX_EXTRA = 2,
    P4PD_HWID_TCP_RX_TCP_RX = 3,
    P4PD_HWID_TCP_RX_RTT = 4,
    P4PD_HWID_TCP_RX_TCP_FRA = 5,
    P4PD_HWID_TCP_RX_TCP_CC = 6,
    P4PD_HWID_TCP_RX_TCP_FC = 7,
    P4PD_HWID_TCP_RX_WRITE_SERQ = 8,

    P4PD_HWID_TCP_TX_TCP_TX = 9,
    P4PD_HWID_TCP_TX_HEADER_TEMPLATE = 10,

    P4PD_HWID_TCP_RX_STATS = 12,
    P4PD_HWID_TCP_RX_STATS1 = 12,
    P4PD_HWID_TCP_RX_STATS2 = 13,

    P4PD_HWID_TCP_TX_STATS = 14,
    P4PD_HWID_TCP_TX_STATS1 = 14,
    P4PD_HWID_TCP_TX_STATS2 = 15,
} tcpcb_hwid_order_t;

typedef struct __attribute__((__packed__)) __tcp_rx_read_tx2rx_actiondata_d {
    uint64_t pc :8;
    uint64_t rsvd: 8;
    uint64_t cosA: 4;
    uint64_t cosB: 4;
    uint64_t cos_sel: 8;
    uint64_t eval_last: 8;
    uint64_t host: 4;
    uint64_t pid: 16;

    uint64_t prr_out: 32;
    uint64_t snd_nxt: 32;
    uint64_t ecn_flags_tx: 8;
    uint64_t packets_out: 16;
} tcp_rx_read_tx2rx_actiondata_d;

// Atomic stats for TCP RX in TCP CB
// **Note: offsets need to match stats update in tcp-rx-stats.h
typedef struct __attribute__((__packed__)) __tcp_rx_stats_t {
    uint64_t bytes_rcvd;
    uint64_t pkts_rcvd;
    uint64_t pages_alloced;
    uint64_t desc_alloced;
    uint64_t stats4;
    uint64_t stats5;
    uint64_t stats6;
    uint64_t stats7;
    uint64_t stats8;
    uint64_t stats9;
    uint64_t stats10;
    uint64_t stats11;
    uint64_t stats12;
    uint64_t stats13;
    uint64_t stats14;
    uint64_t stats15;
} tcp_rx_stats_t;

#endif
