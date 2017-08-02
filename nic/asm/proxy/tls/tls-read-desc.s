/*
 * 	Implements the reading of BRQ to pick up the completed barco request
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"

struct d_struct {
        desc_scratch                     :  28;
        desc_num_entries                 :  2;
        desc_data_len                    :  18;
        desc_head_idx                    :  2;
        desc_tail_idx                    :  2;
        desc_offset                      :  8;

        desc_aol0_scratch                :  64;
        desc_aol0_free_pending           :  1;
        desc_aol0_valid                  :  1;
        desc_aol0_addr                   :  30;
        desc_aol0_offset                 :  16;
        desc_aol0_len                    :  16;

        desc_aol1_scratch                :  64;
        desc_aol1_free_pending           :  1;
        desc_aol1_valid                  :  1;
        desc_aol1_addr                   :  30;
        desc_aol1_offset                 :  16;
        desc_aol1_len                    :  16;
};

struct k_struct {
	fid				: 32 ;
	desc				: ADDRESS_WIDTH ;
	pending_rx_brq			: 1  ;
	pending_rx_serq			: 1  ;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
	
%%

tls_read_desc_process_start:
	phvwr		p.fid, k.fid
	phvwr		p.desc, k.desc

	phvwr		p.desc_scratch, d.desc_scratch
	phvwr		p.desc_num_entries, d.desc_num_entries
	phvwr		p.desc_data_len, d.desc_data_len
	phvwr		p.desc_head_idx, d.desc_head_idx
	phvwr		p.desc_tail_idx, d.desc_tail_idx
	phvwr		p.desc_offset, d.desc_offset

	phvwr		p.desc_aol0_scratch, d.desc_aol0_scratch
	phvwr		p.desc_aol0_free_pending, d.desc_aol0_free_pending
	phvwr		p.desc_aol0_valid, d.desc_aol0_valid
	phvwr		p.desc_aol0_addr, d.desc_aol0_addr
	phvwr		p.desc_aol0_offset, d.desc_aol0_offset
	phvwr		p.desc_aol0_len, d.desc_aol0_len

	phvwr		p.desc_aol1_scratch, d.desc_aol1_scratch
	phvwr		p.desc_aol1_free_pending, d.desc_aol1_free_pending
	phvwr		p.desc_aol1_valid, d.desc_aol1_valid
	phvwr		p.desc_aol1_addr, d.desc_aol1_addr
	phvwr		p.desc_aol1_offset, d.desc_aol1_offset
	phvwr		p.desc_aol1_len, d.desc_aol1_len
	
	

	sne		c1, k.pending_rx_brq, r0
	bcf		[c1], table_read_alloc_sesq_pi
	nop

	seq		c1, k.pending_rx_serq, r0
	bcf		[c1], tls_read_desc_process_done
	nop

table_read_rx_serq:
	TLS_NEXT_TABLE_READ(k.fid, TABLE_TYPE_RAW, tls_rx_serq_process,
	                    TLS_TCB_TABLE_BASE, TLS_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TLS_TCB_OFFSET, TLS_TCB_TABLE_ENTRY_SIZE)
	
	b		tls_read_desc_process_done
	nop
	
table_read_alloc_sesq_pi:
	TLS_READ_IDX(SESQ_PRODUCER_IDX, TABLE_TYPE_RAW, tls_alloc_sesq_pi_process)


	
tls_read_desc_process_done:
	nop.e
	nop.e
	
