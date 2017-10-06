/*
 * 	Implements the stage0 demuxing for tcp tx
 */
#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct phv_ p;

%%

	.param      tcp_tx_read_rx2tx_shared_process
        
tcp_tx_stage0_process:
     CAPRI_OPERAND_DEBUG(r7)
	.brbegin
	    brpri		r7[4:0], [0,1,2,3,4]
	    nop
	        .brcase 0
	            b tcp_sesq_process
	            nop
	        .brcase 1
	            b tcp_pending_process
	            nop
	        .brcase 2
	            b tcp_fast_timer_process
	            nop
	        .brcase 3
	            b tcp_slow_timer_process
	            nop
	        .brcase 4
	            b tcp_asesq_process
	            nop
            .brcase 5
	            b tcp_sesq_process
	            nop
	.brend

	nop.e
    nop

tcp_sesq_process:
    //addi            r7, r0, TCP_SCHED_RING_SESQ
    phvwri          p.common_phv_pending_sesq, 1
    j               tcp_tx_read_rx2tx_shared_process
    nop
    nop.e
    nop


tcp_pending_process:
    //addi            r7, r0, TCP_SCHED_RING_PENDING
    phvwri          p.common_phv_pending_rx2tx, 1
    j               tcp_tx_read_rx2tx_shared_process
    nop
    nop.e
    nop

tcp_fast_timer_process:
    //addi            r7, r0, TCP_SCHED_RING_FT
    //phvwri          p.common_phv_ring_id, TCP_SCHED_RING_FT
    nop.e
    nop

tcp_slow_timer_process:
    //addi            r7, r0, TCP_SCHED_RING_ST
    //phvwri          p.common_phv_ring_id, TCP_SCHED_RING_ST
    nop.e
    nop

tcp_asesq_process:
    //addi            r7, r0, TCP_SCHED_RING_ASESQ
    //phvwri          p.common_phv_ring_id, TCP_SCHED_RING_ASESQ
    phvwri          p.common_phv_pending_asesq, 1
    j               tcp_tx_read_rx2tx_shared_process
    nop
    nop.e
    nop
