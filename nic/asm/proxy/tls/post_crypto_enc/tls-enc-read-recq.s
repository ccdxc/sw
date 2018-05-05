/*
 * 	Implements the reading of record queue
 *  Stage 3, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
	

        
struct tx_table_s3_t0_k     k;
struct phv_                 p;
struct tx_table_s3_t0_d     d;

	
%%
    .param	    tls_enc_read_desc_process
    .param      tls_enc_post_read_idesc

tls_enc_read_recq:
    CAPRI_SET_DEBUG_STAGE0_3(p.to_s7_debug_stage0_3_thread, CAPRI_MPU_STAGE_3, CAPRI_MPU_TABLE_0)

    seq     c1, k.tls_global_phv_post_cbc_enc, 1

    add.c1  r3, r0, k.to_s3_idesc

    add.!c1  r3, r0, d.u.tls_read_recq_d.desc

    phvwr   p.s2s_t2_idesc, r3

    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tls_enc_read_desc_process, r3, TABLE_SIZE_512_BITS)

tls_enc_read_recq_done:
	nop.e
	nop
