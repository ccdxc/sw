/*
 *  Unused stage for now
 */


#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp-constants.h"  
#include "ingress.h"
#include "INGRESS_p.h"
    
struct phv_ p;
struct s3_t0_tcp_fra_k k; // since we don't have our own k
    
%%
    .align
    .param          tcp_rx_fc_stage5_start

tcp_rx_s4_bubble_start:
    CAPRI_SET_DEBUG_STAGE4_7(p.s6_s2s_debug_stage4_7_thread, CAPRI_MPU_STAGE_4, CAPRI_MPU_TABLE_0)
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_rx_fc_stage5_start,
                        k.common_phv_qstate_addr,
                        TCP_TCB_FC_OFFSET, TABLE_SIZE_512_BITS)
    nop.e
    nop
