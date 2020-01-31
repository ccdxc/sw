/*
 * 	Doorbell write to clear the sched bit for the SERQ having
 *      finished the consumption processing.
 *  Stage 3, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
	

/* SERQ consumer index */
        
struct tx_table_s3_t0_k k                  ;
struct phv_ p	;
	
%%
	.param      tls_enc_bld_barco_req_process
	.param      tls_enc_read_barco_pi_process
	.param      TLS_PROXY_BARCO_GCM0_PI_HBM_TABLE_BASE
	.param      TLS_PROXY_BARCO_MPP1_PI_HBM_TABLE_BASE
        
tls_enc_serq_consume_process:
    CAPRI_SET_DEBUG_STAGE0_3(p.to_s6_debug_stage0_3_thread, CAPRI_MPU_STAGE_3, CAPRI_MPU_TABLE_0)
    CAPRI_CLEAR_TABLE0_VALID

table_read_BLD_BARCO_ENC_REQ:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_enc_bld_barco_req_process,
                           k.tls_global_phv_qstate_addr, TLS_TCB_CONFIG,
                           TABLE_SIZE_512_BITS)
 
    seq c1, k.tls_global_phv_flags_do_pre_mpp_enc, 1
	
    /*
     * Launch a locked table-read to the Barco GCM0 PI address in HBM.
     */
    addui.!c1       r5, r0, hiword(TLS_PROXY_BARCO_GCM0_PI_HBM_TABLE_BASE)
    addi.!c1        r5, r5, loword(TLS_PROXY_BARCO_GCM0_PI_HBM_TABLE_BASE)
    addui.c1       r5, r0, hiword(TLS_PROXY_BARCO_MPP1_PI_HBM_TABLE_BASE)
    addi.c1        r5, r5, loword(TLS_PROXY_BARCO_MPP1_PI_HBM_TABLE_BASE)

    CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_EN, tls_enc_read_barco_pi_process, r5, TABLE_SIZE_256_BITS)
	
	nop.e
	nop
