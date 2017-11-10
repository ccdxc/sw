/*
 *    Implements the descr allocation stage of the RxDMA P4+ pipeline
 */

#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_ p;
struct s3_t3_l7_rdesc_alloc_k k;
struct s3_t3_l7_rdesc_alloc_l7_rdesc_alloc_d d;

%%
    .align
tcp_rx_l7_rdesc_alloc_stage_3_start:
    CAPRI_CLEAR_TABLE3_VALID
    //CAPRI_SET_DEBUG_STAGE0_3(p.s6_s2s_debug_stage0_3_thread, CAPRI_MPU_STAGE_3, CAPRI_MPU_TABLE_1)

    CAPRI_OPERAND_DEBUG(d.desc)
    sne             c1, k.common_phv_ooo_rcv, r0
    phvwr.c1        p.to_s5_l7_descr, d.desc
    phvwr.e         p.s6_t2_s2s_l7_descr, d.desc
    nop

