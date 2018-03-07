/*
 *  Unused stage for now
 */


#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp-constants.h"  
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s4_t0_tcp_rx_k.h"
    
struct phv_ p;
struct s4_t0_tcp_rx_k_ k;
    
%%
    .align
    .param          tcp_rx_fc_stage_start

tcp_rx_s4_bubble_start:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_rx_fc_stage_start,
                        k.common_phv_qstate_addr,
                        TCP_TCB_FC_OFFSET, TABLE_SIZE_512_BITS)
    nop.e
    nop
