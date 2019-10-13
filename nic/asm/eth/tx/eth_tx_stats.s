
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s7_t1_k.h"

#include "defines.h"

struct phv_ p;
struct tx_table_s7_t1_k_ k;

#define _r_base    r1

%%

.param lif_stats_base
.param eth_tx_stats_queue_accept
.param eth_tx_stats_queue_drop

.align
eth_tx_stats:
    // bubble until stage 6
    mfspr           r7, spr_mpuid
    seq             c7, r7[4:2], 6
    // bubble exit
    nop.e.!c7

    // calculate stats base addr
    addi            _r_base, r0, loword(lif_stats_base)
    addui           _r_base, _r_base, hiword(lif_stats_base)
    add             _r_base, _r_base, k.eth_tx_global_lif, LIF_STATS_SIZE_SHIFT

    seq             c7, k.eth_tx_global_drop, 1     // exit slot
    bcf             [c7], eth_tx_stats_drop
    nop

eth_tx_stats_accept:
    phvwri.e        p.app_header_table1_valid, 0x0
    nop

eth_tx_stats_drop:
    phvwri          p.app_header_table1_valid, 0x1

    phvwr           p.common_te1_phv_table_lock_en, 1
    add.!c7         r7, _r_base, LIF_STATS_TX_QUEUE_DISABLED_OFFSET
    phvwri.!c7      p.common_te1_phv_table_pc, eth_tx_stats_queue_accept[38:6]
    add.c7          r7, _r_base, LIF_STATS_TX_CSUM_HW_OFFSET
    phvwri.c7       p.common_te1_phv_table_pc, eth_tx_stats_queue_drop[38:6]
    phvwr.e         p.common_te1_phv_table_addr, r7
    phvwr.f         p.common_te1_phv_table_raw_table_size, LG2_TX_STATS_BLOCK_SZ
