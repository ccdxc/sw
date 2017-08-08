/*
 *	Implements the descr allocation stage of the RxDMA P4+ pipeline
 */

#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
 /* d is the data returned by lookup result */
struct d_struct {
	desc				: ADDRESS_WIDTH    ;
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	fid				: 32 ;
	RNMDR_alloc_idx			: RING_INDEX_WIDTH ;
	write_serq			: 1 ;
	
};

struct phv_ p;
struct tcp_rx_rdesc_alloc_k k;
struct tcp_rx_rdesc_alloc_rdesc_alloc_d d;

%%
	
tcp_rx_desc_alloc_stage_3_start:
	phvwr		p.to_s6_descr, d.desc
	//phvwr		p.write_serq, k.write_serq
	sne		c1, k.common_phv_write_serq, r0
	bcf		[c1], flow_rdesc_alloc_process_done
	nop

table_read_SACK4:
	//TODO: TCP_NEXT_TABLE1_READ(k.common_phv_fid, TABLE_LOCK_EN, flow_sack4_process,
	                    //TODO: TCP_TCB_TABLE_BASE, TCP_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    //TODO: TCP_TCB_SACK_OFFSET, TABLE_SIZE_512_BITS)

flow_rdesc_alloc_process_done:	
	nop.e
	nop

