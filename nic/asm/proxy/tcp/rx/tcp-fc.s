/*
 *	Implements the CC stage of the RxDMA P4+ pipeline
 */


#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp-constants.h"	
#include "ingress.h"
#include "INGRESS_p.h"
	
struct phv_ p;
struct tcp_rx_tcp_cc_k k;              // TODO : define own k for s5
struct tcp_rx_tcp_cc_tcp_cc_d d;       // TODO : define own k for s5
	
%%
    .param          tcp_rx_write_serq_stage6_start
	.align	
tcp_rx_fc_stage5_start:
    CAPRI_SET_DEBUG_STAGE4_7(p.s6_s2s_debug_stage4_7_thread, CAPRI_MPU_STAGE_5, CAPRI_MPU_TABLE_0)
        // TODO : FC stage has to be implemented
	CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                tcp_rx_write_serq_stage6_start, k.common_phv_qstate_addr,
                TCP_TCB_WRITE_SERQ_OFFSET, TABLE_SIZE_512_BITS)
	nop.e
	nop
