/*
 * 	Implements the reading of procesed head entry to pick up the completed decryption barco request
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
struct tx_table_s1_t0_tls_rx_bsq_d d	;
	
%%
	.param		tls_dec_read_desc_process
    .param      tls_dec_post_read_idesc
    .param      tls_dec_post_read_l7_rnmdr_pidx

tls_dec_rx_bsq_process:	
    CAPRI_SET_DEBUG_STAGE0_3(p.to_s6_debug_stage0_3_thread, CAPRI_MPU_STAGE_1, CAPRI_MPU_TABLE_0)
	/* Release the descriptor from the tls cb decrypt queue */
    /*
	   DEQ_DESC(*dtlsp, dec, idesc);
	   RING_FREE(idesc, RNMDR);
	   dtlsp->dec_una.desc = HEAD_DESC(*dtlsp, dec);
	*/
    phvwri  p.to_s6_dec_completions, 1
    phvwr   p.to_s5_other_fid, d.other_fid
    seq		c1, d.qhead, r0
	bcf		[c1], tls_rx_bsq_dec_process_done
	nop
table_read_desc:	
	add		r3, r0, d.qhead
    /* FIXME: Adjust qhead and possibly qtail in the CB */
    phvwr   p.to_s3_idesc, r3
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tls_dec_read_desc_process, r3, TABLE_SIZE_128_BITS)

    addi    r3, r3, PKT_DESC_AOL_OFFSET
	CAPRI_NEXT_TABLE_READ(3, TABLE_LOCK_DIS, tls_dec_post_read_idesc,
	                    r3, TABLE_SIZE_512_BITS)

table_read_L7_RNMDR_ALLOC_IDX:
    # Allocate L7 only if SPAN proxy is enabled
    seq     c1, k.tls_global_phv_l7_proxy_en, 1
    seq     c2, k.tls_global_phv_l7_proxy_type_span, 1
    bcf     [!c1 | !c2], tls_rx_bsq_dec_process_done
    nop
    
    addi    r3, r0, RNMDR_ALLOC_IDX
    CAPRI_NEXT_TABLE_READ(2, 
                          TABLE_LOCK_DIS,
                          tls_dec_post_read_l7_rnmdr_pidx,
                          r3,
                          TABLE_SIZE_64_BITS)

tls_rx_bsq_dec_process_done:
	nop.e
	nop.e
