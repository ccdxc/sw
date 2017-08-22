/*
 *	Implements the page allocation stage of the RxDMA P4+ pipeline
 */

#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct phv_ p;
struct tcp_rx_rpage_alloc_k k;
struct tcp_rx_rpage_alloc_rpage_alloc_d d;

%%
	
tcp_rx_rpage_alloc_stage3_start:
        CAPRI_CLEAR_TABLE2_VALID

	add		r3, r0, d.page	// TODO debug instruction
	phvwr.e		p.to_s6_page, d.page
	nop
