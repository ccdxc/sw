/*
 *    Implements the page allocation stage of the RxDMA P4+ pipeline
 */

#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_ p;
struct s3_t2_tcp_rx_k k;
struct s3_t2_tcp_rx_rpage_alloc_d d;

%%
    
tcp_rx_rpage_alloc_stage3_start:
    CAPRI_CLEAR_TABLE2_VALID
    CAPRI_SET_DEBUG_STAGE0_3(p.s5_s2s_debug_stage0_3_thread, CAPRI_MPU_STAGE_3, CAPRI_MPU_TABLE_2)

    CAPRI_OPERAND_DEBUG(d.page)
    sne             c1, k.common_phv_ooo_rcv, r0
    phvwr.c1        p.to_s4_page, d.page
    phvwr.e         p.to_s5_page, d.page
    nop
