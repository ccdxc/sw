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
    
%%
    .align
    .param          tcp_rx_process_start

tcp_rx_stage1_dummy:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                tcp_rx_process_start, k.common_phv_qstate_addr,
                TCP_TCB_RX_OFFSET, TABLE_SIZE_512_BITS)
    nop.e
    nop
