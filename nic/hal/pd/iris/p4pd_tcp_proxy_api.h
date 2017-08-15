/*
 * tcp_proxy_rxdma_api.hpp
 * Saurabh Jain (Pensando Systems)
 */
#ifndef __TCP_PROXY_RXDMA_API_H__
#define __TCP_PROXY_RXDMA_API_H__

#include <stdint.h>
#include <tcp_proxy_rxdma_p4plus_ingress.h>
#include <tcp_proxy_txdma_p4plus_ingress.h>
#include <../../../model_sim/include/lib_model_client.h>

#ifndef P4PD_CLI
#include <hal_pd_error.hpp>
#else
typedef int p4pd_error_t;
#endif

#define P4PD_TCPCB_STAGE_ENTRY_OFFSET     64
#define P4PD_HBM_TCP_CB_ENTRY_SIZE      512

typedef enum tcpcb_hwid_order_ {
    P4PD_HWID_TCP_RX_READ_TX2RX = 0,
    P4PD_HWID_TCP_TX_READ_RX2TX,
    P4PD_HWID_TCP_RX_TCP_RX,
    P4PD_HWID_TCP_RX_RTT_ID,
    P4PD_HWID_TCP_RX_READ_RNMDR_ID,
    P4PD_HWID_TCP_RX_READ_RNMPR_ID,
    P4PD_HWID_TCP_RX_READ_SERQ,
    P4PD_HWID_TCP_RX_TCP_FRA,
    P4PD_HWID_TCP_RX_RDESC_ALLOC,
    P4PD_HWID_TCP_RX_RPAGE_ALLOC,
    P4PD_HWID_TCP_RX_TCP_CC,
    P4PD_HWID_TCP_RX_WRITE_SERQ,
    P4PD_HWID_TCP_TX_READ_RX2TX_EXTRA,
    P4PD_HWID_TCP_TX_READ_SESQ_CI,
    P4PD_HWID_TCP_TX_READ_SESQ,
    P4PD_HWID_TCP_TX_SESQ_CONSUME,
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

bool
p4plus_hbm_write(uint64_t addr_offset, uint8_t* data, uint32_t size)
{
    return write_mem(addr_offset, data, size);            
}

bool
p4plus_hbm_read(uint64_t addr_offset, uint8_t* data, uint32_t size)
{
    return read_mem(addr_offset, data, size);            
}

#define P4PD_HBM_TCP_CB_START_ADDR                   0xbbbb
#define P4PD_HBM_TCP_CB_ENTRY_SIZE                   512


#endif
