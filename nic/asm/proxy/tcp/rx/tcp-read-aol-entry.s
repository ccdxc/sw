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
	
flow_read_aol_entry_process_start:
	sne		c1, d.aol_free_pending, r0
	tblwr.c1	d.aol_free_pending, r0
	addi.c1		r1, r0, 1
	tblwr.c1	d.aol_valid, r1
	phvwri.c1	p.aol_zero_scratch, 1
	sne.!c1		c2, d.aol_valid, r0

	phvwr		p.aol_scratch, d.aol_scratch
	phvwr		p.aol_free_pending, d.aol_free_pending
	phvwr		p.aol_valid, d.aol_valid
	phvwr		p.aol_addr, d.aol_addr
	phvwr		p.aol_offset, d.aol_offset
	phvwr		p.aol_len, d.aol_len

	
	bcf		[!c2], table_read_RNMPR
	nop
	
table_read_SACK2:
	TCP_NEXT_TABLE_READ(k.fid, TABLE_TYPE_RAW, flow_sack2_process,
	                    TCP_TCB_TABLE_BASE, TCP_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TCP_TCB_SACK_OFFSET, TCP_TCB_TABLE_ENTRY_SIZE)


table_read_RNMPR:
	addi		r1,r0,RNMPR_TABLE_BASE
	addi		r2,r0, RNMPR_ALLOC_IDX
	mincr 		r2,1,RNMPR_TABLE_SIZE_SHFT
	sll		r2,r2, RNMPR_TABLE_ENTRY_SIZE_SHFT
	add		r1,r1,r2
	phvwri		p.table_sel, TABLE_TYPE_RAW
	phvwri		p.table_mpu_entry_raw, flow_aol_page_alloc_process
	phvwr.e		p.table_addr, r1
	nop.e
