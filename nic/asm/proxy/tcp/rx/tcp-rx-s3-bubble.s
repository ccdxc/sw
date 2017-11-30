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
struct s3_t0_tcp_rx_k k;
    
%%
    .align
    .param          tcp_rx_fc_stage_start

tcp_rx_s3_bubble_start:
    CAPRI_SET_DEBUG_STAGE0_3(p.s5_s2s_debug_stage0_3_thread, CAPRI_MPU_STAGE_3, CAPRI_MPU_TABLE_0)
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_rx_fc_stage_start,
                        k.common_phv_qstate_addr,
                        TCP_TCB_FC_OFFSET, TABLE_SIZE_512_BITS)
    nop.e
    nop
