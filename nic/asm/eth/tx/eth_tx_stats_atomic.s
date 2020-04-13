
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s7_t1_k.h"

#include "defines.h"

struct phv_ p;
struct tx_table_s7_t1_k_ k;

#define _r_base                 r1  // Stats base address
#define _r_lif_offset           r2  // Offset of LIF's stats relative to global stats base
#define _r_offset               r3  // Offset of the cache group relative to global stats base
#define _r_addr                 r4  // Update address
#define _r_val                  r5  // Update value

%%

.param  lif_stats_base

.align
eth_tx_stats_atomic:

#ifdef GFT
    b               eth_tx_stats_atomic_done
    nop
#endif

    addi            _r_base, r0, ASIC_MEM_SEM_ATOMIC_ADD_START
    addi            _r_lif_offset, r0, lif_stats_base[30:0] // substract 0x80000000 because hw adds it
    add             _r_lif_offset, _r_lif_offset, k.eth_tx_global_lif, LIF_STATS_SIZE_SHIFT

    bbne            k.eth_tx_global_drop, 1, eth_tx_stats_atomic_incr_drop
    nop

eth_tx_stats_atomic_incr_accept:
    // Update operation counters
    addi            _r_offset, _r_lif_offset, LIF_STATS_TX_CSUM_HW_OFFSET
    ATOMIC_INC_VAL_7(_r_base, _r_offset, _r_addr, _r_val,
                    k.eth_tx_global_stats[STAT_oper_csum_hw],
                    k.eth_tx_global_stats[STAT_oper_csum_hw_inner],
                    k.eth_tx_global_stats[STAT_oper_vlan_insert],
                    k.eth_tx_global_stats[STAT_oper_sg],
                    k.eth_tx_global_stats[STAT_oper_tso_sg],
                    k.eth_tx_global_stats[STAT_oper_tso_sot],
                    k.eth_tx_global_stats[STAT_oper_tso_eot])

    b               eth_tx_stats_atomic_done
    nop

eth_tx_stats_atomic_incr_drop:
    // Update queue & desc counters
    addi            _r_offset, _r_lif_offset, LIF_STATS_TX_QUEUE_DISABLED_OFFSET
    ATOMIC_INC_VAL_7(_r_base, _r_offset, _r_addr, _r_val,
                    k.eth_tx_global_stats[STAT_queue_disabled],
                    k.eth_tx_global_stats[STAT_queue_error],
                    k.eth_tx_global_stats[STAT_desc_fetch_error],
                    k.eth_tx_global_stats[STAT_desc_data_error],
                    k.eth_tx_global_stats[STAT_queue_empty],
                    k.eth_tx_global_stats[STAT_event_disabled],
                    k.eth_tx_global_stats[STAT_event_error])

eth_tx_stats_atomic_done:
    // End of pipeline - Make sure no more tables will be launched
    phvwri.e.f      p.{app_header_table0_valid...app_header_table3_valid}, 0
    nop
