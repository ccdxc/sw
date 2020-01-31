/*
 *	Implements the read tnmdr alloc idx of the TxDMA P4+ tls  pipeline
 *  Stage 2, Table 1
 */

#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls_common.h"
#include "ingress.h"
#include "INGRESS_p.h"

                
struct tx_table_s2_t1_k k                  ;
struct phv_ p	;
struct tx_table_s2_t1_read_tnmdr_d d ;

%%
	
        .param          tls_enc_tdesc_alloc_process
        .param          TNMDPR_BIG_TABLE_BASE
        .param          TLS_PROXY_GLOBAL_STATS
	    .align
tls_enc_alloc_tnmdr_process:
        CAPRI_SET_DEBUG_STAGE0_3(p.to_s6_debug_stage0_3_thread, CAPRI_MPU_STAGE_2, CAPRI_MPU_TABLE_1)
        CAPRI_CLEAR_TABLE1_VALID

        seq         c1, d.tnmdr_pidx_full, 1
        b.c1        tls_enc_tnmdpr_empty

        add         r4, r0, d.{tnmdr_pidx}.wx
        andi        r4, r4, ((1 << CAPRI_TNMDPR_BIG_RING_SHIFT) - 1)

	    phvwr		p.s3_t1_s2s_tnmdr_pidx, r4

table_read_TNMDR_DESC:
	    addui		r3, r0, hiword(TNMDPR_BIG_TABLE_BASE)
	    addi		r3, r3, loword(TNMDPR_BIG_TABLE_BASE)
	    CAPRI_NEXT_TABLE_READ_INDEX(1, r4, TABLE_LOCK_DIS,
                               tls_enc_tdesc_alloc_process,
  	                           r3, TNMDPR_BIG_TABLE_ENTRY_SIZE_SHFT,
	                           TABLE_SIZE_512_BITS)
	    nop.e
	    nop

tls_enc_tnmdpr_empty:

        addui           r3, r0, hiword(TLS_PROXY_GLOBAL_STATS)
        addi            r3, r3, loword(TLS_PROXY_GLOBAL_STATS)
        CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r3, TLS_PROXY_GLOBAL_STATS_TNMDPR_EMPTY, 1)
        phvwri p.p4_intr_global_drop, 1
        CAPRI_CLEAR_TABLE0_VALID
        CAPRI_CLEAR_TABLE1_VALID
        CAPRI_CLEAR_TABLE2_VALID
        CAPRI_CLEAR_TABLE3_VALID
        illegal
        nop.e
        nop
