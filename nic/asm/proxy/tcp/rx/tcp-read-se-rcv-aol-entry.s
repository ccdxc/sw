/*
 *	Implements the SACK stage of the RxDMA P4+ pipeline
 */

#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
	
 /* d is the data returned by lookup result */
struct d_struct {
	aol_scratch			: 64    ;
	aol_free_pending		: 1	;
	aol_valid			: 1	;
	aol_addr			: ADDRESS_WIDTH	;
	aol_offset			: OFFSET_WIDTH	;
	aol_len				: LEN_WIDTH	;

};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	fid				: 32 ;
	aol_entry_addr			: ADDRESS_WIDTH ;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;

%%
        .param          RNMDR_TABLE_BASE
	
flow_read_se_rcv_aol_entry_process_start:
	sne		c1, d.aol_valid, r0
	bcf		[!c1], flow_read_se_rcv_aol_entry_process_done
	nop
	//phvwr		p.rcv_cursor_scratch, d.aol_scratch
	phvwr		p.rcv_cursor_free_pending, d.aol_free_pending
	phvwr		p.rcv_cursor_valid, d.aol_valid
	phvwr		p.rcv_cursor_addr, d.aol_addr
	phvwr		p.rcv_cursor_offset, d.aol_offset
	phvwr		p.rcv_cursor_len, d.aol_len


table_read_RNMDR:
	addi		r1,r0,RNMDR_TABLE_BASE
	addi		r2,r0, RNMDR_ALLOC_IDX
	mincr 		r2,1,RNMDR_TABLE_SIZE_SHFT
	sll		r2,r2, RNMDR_TABLE_ENTRY_SIZE_SHFT
	add		r1,r1,r2

	phvwri		p.table_sel, TABLE_TYPE_RAW
	phvwri		p.table_mpu_entry_raw, flow_rdesc_alloc_process
	phvwr.e		p.table_addr, r1

	
flow_read_se_rcv_aol_entry_process_done:
	nop.e
