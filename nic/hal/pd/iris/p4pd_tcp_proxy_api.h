/*
 * tcp_proxy_rxdma_api.hpp
 * Saurabh Jain (Pensando Systems)
 */
#ifndef __TCP_PROXY_RXDMA_API_H__
#define __TCP_PROXY_RXDMA_API_H__

#include <stdint.h>
#include "gen/p4gen/tcp_proxy_rxdma/include/tcp_proxy_rxdma_p4plus_ingress.h"
#include "gen/p4gen/tcp_proxy_txdma/include/tcp_proxy_txdma_p4plus_ingress.h"
#include "gen/p4gen/tcp_proxy_ooq_txdma/include/tcp_proxy_ooq_txdma_p4plus_ingress.h"
#include "nic/hal/pd/iris/internal/p4plus_pd_api.h"

#define P4PD_TCPCB_STAGE_ENTRY_OFFSET   64

// NOTE : ** These offsets need to match the offsets in 
// tcp-table.h **
typedef enum tcpcb_hwid_order_ {
    P4PD_HWID_TCP_TX_READ_RX2TX = 0,
    P4PD_HWID_TCP_RX_READ_TX2RX = 1,
    P4PD_HWID_TCP_TX_READ_RX2TX_EXTRA = 2,
    P4PD_HWID_TCP_RX_TCP_RX = 3,
    P4PD_HWID_TCP_RX_RTT = 4,
    P4PD_HWID_TCP_RX_TCP_CC = 5,
    P4PD_HWID_TCP_RX_TCP_FC = 7,
    P4PD_HWID_TCP_RX_DMA = 8,

    P4PD_HWID_TCP_TX_TCP_RETX = 6,
    P4PD_HWID_TCP_TX_TCP_XMIT = 9,
    P4PD_HWID_TCP_TX_TCP_TSO = 10,
    P4PD_HWID_TCP_TX_HEADER_TEMPLATE = 11,

    P4PD_HWID_TCP_RX_STATS = 12,

    P4PD_HWID_TCP_TX_STATS = 13,

    P4PD_HWID_TCP_RX_OOO_BOOK_KEEPING_OFFSET = 14,
    P4PD_HWID_TCP_RX_OOO_QADDR_OFFSET = 15
} tcpcb_hwid_order_t;

// needs to match offset in tcp-table.h
// TODO : Unify this definition so it can be used in 
// both asm and C
#define TCP_TCB_OOO_QADDR_CI_OFFSET 0

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
    uint64_t bytes_acked;
    uint64_t pure_acks_rcvd;
    uint64_t dup_acks_rcvd;
    uint64_t ooo_cnt;
    uint64_t stats6;
    uint64_t stats7;
} tcp_rx_stats_t;

// **Note: offsets need to match stats update in tcp-tx-stats.h
typedef struct __attribute__((__packed__)) __tcp_tx_stats_t {
    uint64_t bytes_sent;
    uint64_t pkts_sent;
    uint64_t pure_acks_sent;
    uint64_t stats3;
    uint64_t stats4;
    uint64_t stats5;
    uint64_t stats6;
    uint64_t stats7;
} tcp_tx_stats_t;

#endif
