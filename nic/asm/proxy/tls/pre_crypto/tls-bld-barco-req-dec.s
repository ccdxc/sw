/*
 * 	Construct the barco request in this stage for decrypt
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls-sched.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct tx_table_s6_t0_k k                  ;
struct phv_ p	;
struct tx_table_s6_t0_tls_bld_brq6_d d	;
	
%%
	    .param      tls_queue_brq_dec_process
#	    .param		BRQ_QPCB_BASE
        
tls_bld_barco_req_dec_process:        

table_read_QUEUE_BRQ:
    /* Fill the barco request in the phv to be DMAed later into BRQ slot */
    phvwr       p.barco_desc_input_list_address, k.to_s6_idesc
    phvwr       p.barco_desc_output_list_address, k.to_s6_odesc
    phvwr       p.barco_desc_key_desc_index, d.key_desc_index
#if 0
    /* FIXME: */
    phvwr       p.barco_desc_command_core,  d.command_core
    phvwr       p.barco_desc_command_mode, d.command_mode
    phvwr       p.barco_desc_command_op, d.command_op
    phvwr       p.barco_desc_command_param, d.command_param
#endif

    addi        r3, r0, BRQ_QPCB_BASE
    CAPRI_NEXT_TABLE0_READ(k.tls_global_phv_fid, TABLE_LOCK_EN, tls_queue_brq_dec_process,
                           r3, TLS_TCB_TABLE_ENTRY_SIZE_SHFT,
                       	   0, TABLE_SIZE_512_BITS)
	nop.e
	nop
