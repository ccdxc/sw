/*
 * 	Implements the reading of BRQ to pick up the completed barco request
 * Stage 4, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct tx_table_s4_t0_k     k;
struct phv_                 p;
struct tx_table_s4_t0_tls_read_desc_d d	;

	    
%%
    .param      tls_enc_bsq_consume_process
    .param      tls_enc_free_rnmdr
    .param      tls_enc_free_rnmpr

tls_enc_read_desc_process:
    CAPRI_SET_DEBUG_STAGE4_7(p.to_s7_debug_stage4_7_thread, CAPRI_MPU_STAGE_4, CAPRI_MPU_TABLE_0)
    phvwrpair   p.read_desc_status, d.status, \
                p.read_desc_output_list_address, d.output_list_address
    phvwr       p.to_s7_odesc, d.{output_list_address}.dx
    phvwr       p.to_s5_odesc, d.{output_list_address}.dx
    CAPRI_OPERAND_DEBUG(d.output_list_address)

#if 0
    /* The status field is not updated anymore for the Encrypt operation */
    sne         c1, r0, d.{status}.wx
    /* Barco Error */
    phvwri.c1    p.tls_global_phv_barco_op_failed, 1
#endif

table_read_bsq_consume:
	CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS, tls_enc_bsq_consume_process,
                          k.tls_global_phv_qstate_addr,
	                      TLS_TCB_OFFSET, TABLE_SIZE_512_BITS)

table_read_RNMDR_FREE_IDX:

#if 0
    addi    r3, r0, RNMDR_FREE_IDX
	CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, tls_enc_free_rnmdr,
	                    r3, TABLE_SIZE_16_BITS)

table_read_RNMPR_FREE_IDX:
	addi 	r3, r0, RNMPR_FREE_IDX
	CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_DIS, tls_enc_free_rnmpr,
	                    r3, TABLE_SIZE_16_BITS)
#endif

	
tls_read_desc_process_done:
	nop.e
	nop.e
	
