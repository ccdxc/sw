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
	
tcp_rx_fc_stage5_start:
        // TODO : FC stage has to be implemented
	CAPRI_NEXT_TABLE0_READ(k.common_phv_fid, TABLE_LOCK_EN,
                tcp_rx_write_serq_stage6_start, TCP_TCB_TABLE_BASE,
                TCP_TCB_TABLE_ENTRY_SIZE_SHFT, TCP_TCB_WRITE_SERQ_OFFSET,
                TABLE_SIZE_512_BITS)
	nop.e
	nop
