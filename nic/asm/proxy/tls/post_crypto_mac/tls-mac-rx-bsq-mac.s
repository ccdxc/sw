/*
 * AES-CBC-HMAC-SHA2 Mac-then-encrypt post-mac pipeline:
 * 	Implements the reading of BSQ to pick up the completed decryption barco request
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
        .param      tls_mac_bsq_consume_process
        .param      tls_mac_alloc_tnmdr_process
        .param      tls_mac_alloc_tnmpr_process
        .param      tls_mac_post_read_idesc_process
	
tls_mac_rx_bsq_mac_process:	
    CAPRI_SET_DEBUG_STAGE0_3(p.to_s5_debug_stage0_3_thread, CAPRI_MPU_STAGE_1, CAPRI_MPU_TABLE_0)
    /* Release the descriptor from the tls cb decrypt queue */
	/*
	   DEQ_DESC(*dtlsp, dec, idesc);
	   RING_FREE(idesc, RNMDR);
	   dtlsp->dec_una.desc = HEAD_DESC(*dtlsp, dec);
	*/
    phvwri	p.to_s5_mac_completions, 1
    seq	    	c1, d.qhead, r0
    bcf	    	[c1], tls_rx_bsq_mac_process_done
    nop
table_read_desc:	
    add		r2, r0, d.qhead
    /* FIXME: Adjust qhead and possibly qtail in the CB */

    /*
     * Since we are in the intermediate-pass of the MAC-then-Encrypt pipeline,
     * we will not dequeue the input-descriptor from the queue here, rather
     * after the second pass (post-encrypt).
     */
    //tblwr   d.qhead, 0  // FIXME
    //tblwr   d.qtail, 0  // FIXME
    phvwr   	p.to_s3_idesc, r2
    phvwr   	p.to_s4_idesc, r2
    CAPRI_OPERAND_DEBUG(r2)

table_read_bsq_consume:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_mac_bsq_consume_process,
                                 k.tls_global_phv_qstate_addr,
	                         TLS_TCB_OFFSET, TABLE_SIZE_512_BITS)

table_read_TNMDR_ALLOC_IDX:
    addi    	r3, r0, TNMDR_ALLOC_IDX
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, tls_mac_alloc_tnmdr_process,
                          r3, TABLE_SIZE_16_BITS)

table_read_TNMPR_ALLOC_IDX:
    addi 	r3, r0, TNMPR_ALLOC_IDX
    CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_DIS, tls_mac_alloc_tnmpr_process,
                          r3, TABLE_SIZE_16_BITS)

table_read_IPAGE:	
    addi    	r2, r2, PKT_DESC_AOL_OFFSET
    CAPRI_NEXT_TABLE_READ(3, TABLE_LOCK_DIS, tls_mac_post_read_idesc_process,
	                  r2, TABLE_SIZE_512_BITS)

tls_rx_bsq_mac_process_done:
    nop.e
    nop
