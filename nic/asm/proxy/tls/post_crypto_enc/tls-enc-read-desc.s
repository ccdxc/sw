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
//    phvwrpair   p.read_desc_status, d.status, \
//                p.read_desc_output_list_address, d.output_list_address
    phvwr       p.to_s7_odesc, d.{output_list_address}.dx
    phvwr       p.to_s5_odesc, d.{output_list_address}.dx
    CAPRI_OPERAND_DEBUG(d.output_list_address)


table_read_bsq_consume:
	CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS, tls_enc_bsq_consume_process,
                          k.tls_global_phv_qstate_addr,
	                      TLS_TCB_OFFSET, TABLE_SIZE_512_BITS)

tls_read_desc_process_done:
	nop.e
	nop.e
	
