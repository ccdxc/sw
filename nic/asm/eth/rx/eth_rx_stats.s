
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_rx_table_s7_t1_k.h"

#include "defines.h"

struct phv_ p;
struct rx_table_s7_t1_k_ k;

#define _r_base    r1

%%

.param lif_stats_base
.param eth_rx_stats_packet_accept
.param eth_rx_stats_packet_drop
.param eth_rx_stats_queue_accept
.param eth_rx_stats_queue_drop

.align
eth_rx_stats:
    // bubble until stage 6
    mfspr           r7, spr_mpuid
    seq             c7, r7[4:2], 6
    // bubble exit
    nop.e.!c7

    // calculate stats base addr
    addi            _r_base, r0, loword(lif_stats_base)
    addui           _r_base, _r_base, hiword(lif_stats_base)
    add             _r_base, _r_base, k.eth_rx_global_lif, LIF_STATS_SIZE_SHIFT

    // set table address and size in stage 6
    seq             c7, k.eth_rx_global_drop, 1     // exit slot
    bcf             [c7], eth_rx_stats_drop
    nop

eth_rx_stats_accept:
    phvwri          p.app_header_table1_valid, 1
    phvwri          p.common_te1_phv_table_lock_en, 1
    phvwri          p.common_te1_phv_table_pc, eth_rx_stats_packet_accept[38:6]
    add             r7, _r_base, LIF_STATS_RX_UCAST_BYTES_OFFSET
    phvwr.e         p.common_te1_phv_table_addr, r7
    phvwr.f         p.common_te1_phv_table_raw_table_size, LG2_RX_STATS_BLOCK_SZ

    // Following stats are for debugging only ... the exit above is intentional
    phvwri          p.app_header_table2_valid, 1
    phvwri          p.common_te2_phv_table_lock_en, 1
    phvwri          p.common_te2_phv_table_pc, eth_rx_stats_queue_accept[38:6]
    add             r7, _r_base, LIF_STATS_RX_RSS_OFFSET
    phvwr.e         p.common_te2_phv_table_addr, r7
    phvwr.f         p.common_te2_phv_table_raw_table_size, LG2_RX_STATS_BLOCK_SZ

eth_rx_stats_drop:
    phvwri          p.app_header_table1_valid, 1
    phvwri          p.common_te1_phv_table_lock_en, 1
    phvwri          p.common_te1_phv_table_pc, eth_rx_stats_packet_drop[38:6]
    add             r7, _r_base, LIF_STATS_RX_UCAST_DROP_BYTES_OFFSET
    phvwr           p.common_te1_phv_table_addr, r7
    phvwr           p.common_te1_phv_table_raw_table_size, LG2_RX_STATS_BLOCK_SZ

    phvwri          p.app_header_table2_valid, 1
    phvwri          p.common_te2_phv_table_lock_en, 1
    phvwri          p.common_te2_phv_table_pc, eth_rx_stats_queue_drop[38:6]
    add             r7, _r_base, LIF_STATS_RX_QUEUE_DISABLED_OFFSET
    phvwr.e         p.common_te2_phv_table_addr, r7
    phvwr.f         p.common_te2_phv_table_raw_table_size, LG2_RX_STATS_BLOCK_SZ
