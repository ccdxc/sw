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
struct tcp_rx_read_rnmdr_read_rnmdr_d d;
	
%%
        .param          tcp_rx_rdesc_alloc_stage3_start
	
tcp_rx_read_rnmdr_stage2_start:
	phvwr		p.s3_t1_s2s_rnmdr_pidx, d.rnmdr_pidx

table_read_RNMDR_DESC:
	//TODO: TCP_NEXT_TABLE1_READ(d.rnmdr_pidx, TABLE_LOCK_EN,
                            // tcp_rx_rdesc_alloc_stage3_start,
	                    //RNMDR_TABLE_BASE, RNMDR_TABLE_ENTRY_SIZE_SHFT,
	                    //0, TABLE_SIZE_512_BITS)
	nop.e
	nop
