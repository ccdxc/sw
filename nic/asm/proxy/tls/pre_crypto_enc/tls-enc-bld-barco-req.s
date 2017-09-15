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
struct tx_table_s4_t0_d d	;
	
%%
	    .param      tls_enc_queue_brq_process
#	    .param		BRQ_QPCB_BASE
        
tls_enc_bld_barco_req_process:        

table_read_QUEUE_BRQ:
    CAPRI_SET_DEBUG_STAGE4_7(p.to_s6_debug_stage4_7_thread, CAPRI_MPU_STAGE_4, CAPRI_MPU_TABLE_0)
    /* Fill the barco request in the phv to be DMAed later into BRQ slot */
    phvwr       p.barco_desc_status_address, k.{to_s4_idesc}.dx
    phvwr       p.barco_desc_input_list_address, k.{to_s4_idesc}.dx
    CAPRI_OPERAND_DEBUG(k.to_s4_idesc)

    phvwr       p.barco_desc_output_list_address, k.{to_s4_odesc}.dx
    CAPRI_OPERAND_DEBUG(k.to_s4_odesc)

    phvwr       p.barco_desc_key_desc_index, d.{u.tls_bld_brq4_d.barco_key_desc_index}.wx
    CAPRI_OPERAND_DEBUG(d.{u.tls_bld_brq4_d.barco_key_desc_index}.wx)

    phvwri      p.to_s6_enc_requests, 1

    phvwr       p.crypto_iv_salt, d.u.tls_bld_brq4_d.salt
    CAPRI_OPERAND_DEBUG(d.u.tls_bld_brq4_d.salt)

    phvwr       p.crypto_iv_explicit_iv, d.u.tls_bld_brq4_d.explicit_iv
    CAPRI_OPERAND_DEBUG(d.u.tls_bld_brq4_d.explicit_iv)
    tbladd      d.u.tls_bld_brq4_d.explicit_iv, 1

    phvwr       p.barco_desc_command, d.u.tls_bld_brq4_d.barco_command
    CAPRI_OPERAND_DEBUG(d.u.tls_bld_brq4_d.barco_command)

	/* address will be in r4 */
	CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_INC, DB_SCHED_UPD_SET, 0, LIF_TLS)
    phvwr       p.barco_desc_doorbell_address, r4.dx
    CAPRI_OPERAND_DEBUG(r4.dx)

    
	/* data will be in r3 */
	CAPRI_RING_DOORBELL_DATA(0, k.tls_global_phv_fid, TLS_SCHED_RING_BSQ, 0)
    phvwr       p.barco_desc_doorbell_data, r3.dx
    CAPRI_OPERAND_DEBUG(r3.dx)
        
    addi        r3, r0, CAPRI_BARCO_MD_HENS_REG_PRODUCER_IDX
    /* FIXME: The Capri model currently does not support a read of 8 bytes from register space
     * enable this once it is fixed
     *  CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, tls_enc_queue_brq_process, r3, TABLE_SIZE_64_BITS);
     */
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, tls_enc_queue_brq_process, r3, TABLE_SIZE_32_BITS);
	nop.e
	nop
