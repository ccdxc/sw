/*
 *  Unused stage for now
 */


#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp-constants.h"  
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s1_t0_tcp_rx_k.h"
    
struct phv_ p;
struct s1_t0_tcp_rx_k_ k;
struct s1_t0_tcp_rx_tcp_read_tls_stage0_d d;
    
%%
    .align
    .param          tcp_rx_process_start

tcp_rx_read_tls_stage0_start:
    CAPRI_SET_DEBUG_STAGE0_3(p.s6_s2s_debug_stage0_3_thread, CAPRI_MPU_STAGE_1, CAPRI_MPU_TABLE_0)
    CAPRI_OPERAND_DEBUG(d.pi_0)
    CAPRI_OPERAND_DEBUG(d.ci_0)
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                tcp_rx_process_start, k.common_phv_qstate_addr,
                TCP_TCB_RX_OFFSET, TABLE_SIZE_512_BITS)
    nop.e
    nop
