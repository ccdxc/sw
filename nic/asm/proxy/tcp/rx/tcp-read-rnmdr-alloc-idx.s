/*
 *	Get an index and auto increment it.
 *      This stage will be used to get
	      - RNMDR alloc idx
 */

#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_ p;
struct tcp_rx_read_rnmdr_k k;
struct tcp_rx_read_rnmdr_read_rnmdr_d d;
	
%%
        .param          tcp_rx_rdesc_alloc_stage_3_start
        .param          RNMDR_TABLE_BASE
	.align
tcp_rx_read_rnmdr_stage2_start:

        CAPRI_CLEAR_TABLE1_VALID

        // TODO : check for semaphore full
        and             r4, d.rnmdr_pidx, ~(1 << CAPRI_RNMDR_RING_SHIFT)
	phvwr		p.s3_t1_s2s_rnmdr_pidx, r4.wx

table_read_RNMDR_DESC:
	addi		r3, r0, RNMDR_TABLE_BASE
        // TODO change to r4.wx after debugging issue with non-zero PI
	CAPRI_NEXT_TABLE1_READ(r0, TABLE_LOCK_DIS,
                            tcp_rx_rdesc_alloc_stage_3_start,
	                    r3, RNMDR_TABLE_ENTRY_SIZE_SHFT,
	                    0, TABLE_SIZE_64_BITS)
	nop.e
	nop
