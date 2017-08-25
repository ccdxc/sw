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
        idesc            : HBM_ADDRESS_WIDTH ;
};

struct phv_ p	;
struct d_struct d	;
struct tx_table_s1_t0_k k	    ;
%%
        .param          tls_rx_serq_enc_process_start
        .param          tls_rx_serq_dec_process_start
        .param          tls_alloc_tnmdr_start
        .param          tls_alloc_tnmpr_start
        .param          tls_serq_consume_process_start
        
        
tls_read_serq_entry_process_start:
    phvwr   p.to_s2_idesc, d.idesc


table_read_rx_serq:
    seq     c1, r0, k.tls_global_phv_dec_flow
    bcf     [!c1], table_read_rx_serq_dec
    nop
table_read_rx_serq_enc: 
	CAPRI_NEXT_TABLE0_READ(k.tls_global_phv_fid, TABLE_LOCK_EN, tls_rx_serq_enc_process_start,
	                    k.tls_global_phv_qstate_addr, TLS_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TLS_TCB_CRYPT_OFFSET, TABLE_SIZE_512_BITS)
    b table_read_TNMDR_ALLOC_IDX
    nop
table_read_rx_serq_dec: 
	CAPRI_NEXT_TABLE0_READ(k.tls_global_phv_fid, TABLE_LOCK_EN, tls_rx_serq_dec_process_start,
	                    k.tls_global_phv_qstate_addr, TLS_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TLS_TCB_CRYPT_OFFSET, TABLE_SIZE_512_BITS)

table_read_TNMDR_ALLOC_IDX:

    addi    r3, r0, TNMDR_ALLOC_IDX
	CAPRI_NEXT_IDX1_READ(TABLE_LOCK_DIS, tls_alloc_tnmdr_start,
	                    r3, TABLE_SIZE_16_BITS)

table_read_TNMPR_ALLOC_IDX:
	addi 	r3, r0, TNMPR_ALLOC_IDX
	CAPRI_NEXT_IDX2_READ(TABLE_LOCK_DIS, tls_alloc_tnmpr_start,
	                    r3, TABLE_SIZE_16_BITS)
        

table_read_serq_consume:
	CAPRI_NEXT_IDX3_READ(TABLE_LOCK_DIS, tls_serq_consume_process_start,
                         r0, TABLE_SIZE_16_BITS)

       
	
tls_read_desc_process_done:
	nop.e
	nop.e
	
