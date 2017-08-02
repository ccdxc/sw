/*
 *	Implements the tx2rx shared state read stage of the RxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
	
 /* d is the data returned by lookup result */
struct d_struct {
	/* State needed by RX but updated by TX pipeline */
	TCB_TX2RX_SHARED_STATE
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	fid				: 32 ;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;

	
%%
	
flow_read_tx2rx_shared_process_start:
	/* Write all the tx to rx shared state from table data into phv */
	phvwr		p.prr_out, d.prr_out
	phvwr		p.snd_nxt, d.snd_nxt
	phvwr		p.ecn_flags_tx, d.ecn_flags_tx
	phvwr		p.ecn_flags, d.ecn_flags_tx
	phvwr		p.packets_out, d.packets_out
table_read_RX:	
	TCP_NEXT_TABLE_READ(k.fid, TABLE_TYPE_RAW, flow_rx_process,
	                    TCP_TCB_TABLE_BASE, TCP_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TCP_TCB_RX_OFFSET, TCP_TCB_TABLE_ENTRY_SIZE)
