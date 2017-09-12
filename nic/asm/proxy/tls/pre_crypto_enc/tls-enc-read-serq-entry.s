/*
 * 	Implements the reading of SERQ descriptor
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct d_struct {
        idesc            : DESC_ADDRESS_WIDTH ; /* Descriptor address is written as 64-bits, so we have to use ADDRESS_WIDTH(64)
                                                 * instead of HBM_ADDRESS_WIDTH (32)
                                                 */
        pad              : 448  ;
};

struct phv_ p	;
struct d_struct d	;
struct tx_table_s1_t0_k k	    ;
%%
        .param          tls_enc_rx_serq_process
        .param          tls_enc_alloc_tnmdr_process
        .param          tls_enc_alloc_tnmpr_process
        
        
tls_enc_read_serq_entry_process:
    CAPRI_SET_DEBUG_STAGE0_3(p.to_s6_debug_stage0_3_thread, CAPRI_MPU_STAGE_1, CAPRI_MPU_TABLE_0)
    phvwr   p.to_s2_idesc, d.{idesc}
    phvwr   p.to_s4_idesc, d.{idesc}
    phvwr   p.to_s5_idesc, d.{idesc}
    add     r1, r0, d.{idesc}

table_read_rx_serq_enc: 
	CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_enc_rx_serq_process,
	                    k.tls_global_phv_qstate_addr, TLS_TCB_CRYPT_OFFSET,
                        TABLE_SIZE_512_BITS)

table_read_TNMDR_ALLOC_IDX:
    addi    r3, r0, TNMDR_ALLOC_IDX
	CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, tls_enc_alloc_tnmdr_process,
	                    r3, TABLE_SIZE_16_BITS)

table_read_TNMPR_ALLOC_IDX:
	addi 	r3, r0, TNMPR_ALLOC_IDX
	CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_DIS, tls_enc_alloc_tnmpr_process,
	                    r3, TABLE_SIZE_16_BITS)
        
	nop.e
	nop
	
