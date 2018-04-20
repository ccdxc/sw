/*
 * 	Dummy table to ensure the TLSCB-1 table read (tls_enc_rx_bsq_enc_process)
 *      happens in Stage-2 under locked-table, to synchronize with the TLSCB-1
 *      read/write in the pre-crypto-enc pipeline which happens in Stage-2 (lock
 *      tables must be in same stage).
 *  Stage 1, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
	

struct tx_table_s1_t0_k k       ;
struct phv_ p	;
struct tx_table_s1_t0_tls_rx_bsq_dummy_d d	;
	
%%
    .param		tls_dec_rx_bsq_process
	
tls_dec_rx_bsq_dec_dummy_process:	
    CAPRI_SET_DEBUG_STAGE0_3(p.stats_debug_stage0_3_thread, CAPRI_MPU_STAGE_1, CAPRI_MPU_TABLE_0)
	
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_dec_rx_bsq_process,
                                 k.tls_global_phv_qstate_addr,
                                 TLS_TCB_CRYPT_OFFSET, TABLE_SIZE_512_BITS)

	nop.e
	nop
