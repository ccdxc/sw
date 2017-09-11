/*
 * 	Implements the reading of BRQ to pick up the completed barco request
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct tx_table_s2_t0_k k       ;
struct phv_ p	;
struct tx_table_s2_t0_tls_read_desc_d d	;

	    
%%
    .param      tls_bsq_consume_process
    .param      tls_free_rnmdr
    .param      tls_free_rnmpr

tls_read_desc_process:

    phvwr       p.read_desc_status, d.status
    phvwr       p.read_desc_output_list_address, d.output_list_address
    phvwr       p.to_s4_odesc, d.output_list_address
    CAPRI_OPERAND_DEBUG(d.output_list_address)
table_read_bsq_consume:
	CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS, tls_bsq_consume_process,
                          k.tls_global_phv_qstate_addr,
	                      TLS_TCB_OFFSET, TABLE_SIZE_512_BITS)

table_read_RNMDR_FREE_IDX:

    addi    r3, r0, RNMDR_FREE_IDX
	CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, tls_free_rnmdr,
	                    r3, TABLE_SIZE_16_BITS)

table_read_RNMPR_FREE_IDX:
	addi 	r3, r0, RNMPR_FREE_IDX
	CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_DIS, tls_free_rnmpr,
	                    r3, TABLE_SIZE_16_BITS)

	
tls_read_desc_process_done:
	nop.e
	nop.e
	
