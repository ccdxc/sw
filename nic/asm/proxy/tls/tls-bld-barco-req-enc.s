/*
 * 	Construct the barco request in this stage
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls-sched.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct tx_table_s4_t0_k k                  ;
struct phv_ p	;
struct tx_table_s4_t0_tls_bld_brq4_d d	;
	
%%
	    .param      tls_queue_brq_enc_process
#	    .param		BRQ_QPCB_BASE
        
tls_bld_barco_req_enc_process:        

table_read_QUEUE_BRQ:
    /* Fill the barco request in the phv to be DMAed later into BRQ slot */
    phvwr       p.barco_desc_status_address, k.{to_s4_idesc}.dx
    phvwr       p.barco_desc_input_list_address, k.{to_s4_idesc}.dx
    add         r1, r0, d.idesc
    phvwr       p.barco_desc_output_list_address, k.{to_s4_odesc}.dx
    add         r1, r0, d.odesc        
    phvwr       p.barco_desc_key_desc_index, d.{key_addr}.dx
    add         r1, r0, d.key_addr
    phvwr       p.barco_desc_iv_address, d.{iv_addr}.dx
    add         r1, r0, d.iv_addr
    phvwr       p.barco_desc_command_core,  d.command_core
    add         r1, r0, d.command_core
    phvwr       p.barco_desc_command_mode, d.command_mode
    add         r1, r0, d.command_mode
    phvwr       p.barco_desc_command_op, d.command_op
    add         r1, r0, d.command_op
    phvwr       p.barco_desc_command_param, d.command_param
    add         r1, r0, d.command_param
        
    addi        r3, r0, BRQ_QPCB_BASE        
    CAPRI_NEXT_TABLE0_READ(k.tls_global_phv_fid, TABLE_LOCK_EN, tls_queue_brq_enc_process,
                           r3, TLS_TCB_TABLE_ENTRY_SIZE_SHFT,
                       	   0, TABLE_SIZE_512_BITS)
	nop.e
	nop
