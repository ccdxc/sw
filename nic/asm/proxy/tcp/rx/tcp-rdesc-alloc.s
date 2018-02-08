/*
 *    Implements the descr allocation stage of the RxDMA P4+ pipeline
 */

#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s4_t1_tcp_rx_k.h"

struct phv_ p;
struct s4_t1_tcp_rx_k_ k;
struct s4_t1_tcp_rx_rdesc_alloc_d d;

%%
    .align
tcp_rx_rdesc_alloc_start:
    CAPRI_CLEAR_TABLE1_VALID
    CAPRI_SET_DEBUG_STAGE4_7(p.s6_s2s_debug_stage4_7_thread, CAPRI_MPU_STAGE_4, CAPRI_MPU_TABLE_1)

    CAPRI_OPERAND_DEBUG(d.desc)
    sne             c1, k.common_phv_ooo_rcv, r0
    phvwr.c1        p.to_s5_descr, d.desc
    phvwr.e         p.to_s6_descr, d.desc
    nop

