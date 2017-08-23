/*
 * 	Implements the reading of BRQ to pick up the completed barco request
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
	
/* d is the data returned by lookup result */
struct d_struct {
        brq_idesc                        : ADDRESS_WIDTH;
        brq_odesc                        : ADDRESS_WIDTH;
        brq_key                          : ADDRESS_WIDTH;
        brq_iv                           : ADDRESS_WIDTH;
        brq_auth_tag                     : ADDRESS_WIDTH;
  
        brq_cmd_core                     : 3;
        brq_cmd_keysize                  : 3;
        brq_cmd_mode                     : 3;
        brq_cmd_hash                     : 3;
        brq_cmd_op                       : 3;
        brq_cmd_rsvd                     : 17;

        brq_payload_offset               : 16;
        brq_status                       : 32;
        brq_opaque_tag                   : 32;
        brq_rsvd                         : 64;
};

/* SERQ consumer index */
struct k_struct {
	brq_consumer_idx		: RING_INDEX_WIDTH ;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
	
%%
	
tls_brq_completion_process_start:
	add		r1, d.brq_rsvd, r0
	smeqh		c1, r1, ENC_FLOW_ID_MASK, ENC_FLOW_ID_MASK
	phvwri.c1	p.enc_flow, 1
	phvwri.!c1      p.enc_flow, 0
	phvwri		p.pending_rx_brq, 1

	phvwr		p.brq_idesc, d.brq_idesc
	phvwr		p.brq_odesc, d.brq_odesc
	phvwr		p.brq_key, d.brq_key
	phvwr		p.brq_iv, d.brq_iv
	phvwr		p.brq_auth_tag, d.brq_auth_tag
	phvwr		p.brq_cmd_core, d.brq_cmd_core
	phvwr		p.brq_cmd_keysize, d.brq_cmd_keysize
	phvwr		p.brq_cmd_mode, d.brq_cmd_mode
	phvwr		p.brq_cmd_hash, d.brq_cmd_hash
	phvwr		p.brq_cmd_op, d.brq_cmd_op

	phvwr		p.brq_payload_offset, d.brq_payload_offset
	phvwr		p.brq_status, d.brq_status
	phvwr		p.brq_opaque_tag, d.brq_opaque_tag
	phvwr		p.brq_rsvd, d.brq_rsvd
	phvwr		p.fid, d.brq_rsvd
	phvwr		p.odesc, d.brq_odesc

table_read_rx_brq:
	TLS_NEXT_TABLE_READ(d.brq_rsvd, TABLE_TYPE_RAW, tls_rx_brq_process,
	                    TLS_TCB_TABLE_BASE, TLS_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TLS_TCB_OFFSET, TLS_TCB_TABLE_ENTRY_SIZE)
