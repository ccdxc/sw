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
    .param	    tls_dec_read_desc_process
    .param      tls_dec_post_read_idesc
    .param      tls_dec_post_read_l7_rnmdr_pidx

tls_dec_read_recq:
    CAPRI_SET_DEBUG_STAGE0_3(p.stats_debug_stage0_3_thread, CAPRI_MPU_STAGE_3, CAPRI_MPU_TABLE_0)

    add     r3, r0, d.u.tls_read_recq_d.desc

    phvwr   p.to_s5_idesc, r3

    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tls_dec_read_desc_process, r3, TABLE_SIZE_128_BITS)

table_read_L7_RNMDR_ALLOC_IDX:
    # Allocate L7 only if SPAN proxy is enabled
    seq     c1, k.tls_global_phv_l7_proxy_en, 1
    seq     c2, k.tls_global_phv_l7_proxy_type_span, 1
    bcf     [!c1 | !c2], tls_dec_read_recq_done
    nop
    
    addi    r3, r0, RNMDR_ALLOC_IDX
    CAPRI_NEXT_TABLE_READ(3, 
                          TABLE_LOCK_DIS,
                          tls_dec_post_read_l7_rnmdr_pidx,
                          r3,
                          TABLE_SIZE_64_BITS)

tls_dec_read_recq_done:
	nop.e
	nop
