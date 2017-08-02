/*
 *	Implements the TSO stage of the TxDMA P4+ pipeline
 */

#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
	
 /* d is the data returned by lookup result */
struct d_struct {

	xmit_cursor_flags		: 8  ;
	xmit_cursor_addr		: ADDRESS_WIDTH ;
	xmit_cursor_offset		: OFFSET_WIDTH ;
	xmit_cursor_len			: LEN_WIDTH ;
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	fid				: 32 ;
	xmit_cursor_flags		: 8  ;
	xmit_cursor_addr		: ADDRESS_WIDTH ;
	xmit_cursor_offset		: OFFSET_WIDTH ;
	xmit_cursor_len			: LEN_WIDTH ;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;


%%
	
flow_read_xmit_cursor_start:
	phvwr		p.xmit_cursor_flags, d.xmit_cursor_flags
	phvwr		p.xmit_cursor_addr, d.xmit_cursor_addr
	phvwr		p.xmit_cursor_offset, d.xmit_cursor_offset	
	phvwr		p.xmit_cursor_len, d.xmit_cursor_len

table_read_TSO:	
	TCP_NEXT_TABLE_READ(k.fid, TABLE_TYPE_RAW, flow_tso_process,
	                    TCP_TCB_TABLE_BASE, TCP_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TCP_TCB_TSO_OFFSET, TCP_TCB_TABLE_ENTRY_SIZE)

