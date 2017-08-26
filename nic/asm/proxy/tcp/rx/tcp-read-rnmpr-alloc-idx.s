/*
 *	Get an index and auto increment it.
 *      This stage will be used to get
	      - RNMPR alloc idx
 */

#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_ p;
struct tcp_rx_read_rnmpr_k k;
struct tcp_rx_read_rnmpr_read_rnmpr_d d;
	
%%
        .param          tcp_rx_rpage_alloc_stage3_start
        .param          RNMPR_TABLE_BASE
	.align
tcp_rx_read_rnmpr_stage2_start:

        CAPRI_CLEAR_TABLE2_VALID

        // TODO : check for semaphore full
        and             r4, d.rnmpr_pidx, ~(1 << CAPRI_RNMPR_RING_SHIFT)
	phvwr		p.s3_t2_s2s_rnmpr_pidx, r4.wx

table_read_RNMPR_PAGE:
	addi		r3, r0, RNMPR_TABLE_BASE
        // TODO change to r4.wx after debugging issue with non-zero PI
	CAPRI_NEXT_TABLE2_READ(r0, TABLE_LOCK_DIS,
                    tcp_rx_rpage_alloc_stage3_start, r3,
                    RNMPR_TABLE_ENTRY_SIZE_SHFT, 0, TABLE_SIZE_64_BITS)
	nop.e
	nop

