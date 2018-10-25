
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_rx_table_s3_t0_k.h"

#include "defines.h"

struct phv_ p;
struct rx_table_s3_t0_k_ k;
struct rx_table_s3_t0_eth_rx_packet_d d;

#define   _r_pktlen     r1        // Packet length
#define   _r_addr       r2        // Buffer address
#define   _r_len        r3        // Buffer length        
#define   _r_ptr        r5        // Current DMA byte offset in PHV
#define   _r_index      r6        // Current DMA command index in PHV

%%

.param eth_rx_completion

.align
eth_rx:

    bcf             [c2 | c3 | c7], eth_rx_desc_addr_error
    nop

    // Setup DMA CMD PTR
    phvwr           p.p4_rxdma_intr_dma_cmd_ptr, ETH_DMA_CMD_START_OFFSET
    phvwr           p.eth_rx_global_dma_cur_index, (ETH_DMA_CMD_START_FLIT << LOG_NUM_DMA_CMDS_PER_FLIT) | ETH_DMA_CMD_START_INDEX

    // Packet length check
    add             _r_pktlen, r0, k.eth_rx_t0_s2s_packet_len
    add             _r_len, r0, d.{len}.hx
    blt             _r_len, _r_pktlen, eth_rx_desc_data_error

    // Load DMA command pointer
    addi            _r_index, r0, (ETH_DMA_CMD_START_FLIT << LOG_NUM_DMA_CMDS_PER_FLIT) | ETH_DMA_CMD_START_INDEX

    // DMA packet
    DMA_CMD_PTR(_r_ptr, _r_index, r7)
    DMA_PKT(_r_ptr, _r_addr, k.eth_rx_t0_s2s_packet_len)
    DMA_CMD_NEXT(_r_index)

    b               eth_rx_done
    // Save DMA command pointer
    phvwr           p.eth_rx_global_dma_cur_index, _r_index

eth_rx_desc_addr_error:
    b               eth_rx_done
    phvwri          p.eth_rx_cq_desc_status, ETH_RX_DESC_ADDR_ERROR

eth_rx_desc_data_error:
    b               eth_rx_done
    phvwri          p.eth_rx_cq_desc_status, ETH_RX_DESC_DATA_ERROR

eth_rx_done:
    // Launch eth_rx_completion stage
    phvwri.e        p.common_te0_phv_table_pc, eth_rx_completion[38:6]
    phvwri.f        p.common_te0_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY