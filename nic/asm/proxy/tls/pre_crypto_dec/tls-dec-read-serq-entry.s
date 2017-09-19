/*
 * 	Implements the reading of SERQ descriptor
 *  Stage 1, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_             p;
struct tx_table_s1_t0_d d;
struct tx_table_s1_t0_k k;

%%
    .param          tls_dec_pkt_descriptor_process
    .param          tls_dec_alloc_tnmdr_process
    .param          tls_dec_alloc_tnmpr_process
    .param          tls_dec_serq_consume_process


tls_dec_read_serq_entry_process:
    phvwr   p.to_s3_idesc, d.u.read_serq_entry_d.idesc
    phvwr   p.to_s5_idesc, d.u.read_serq_entry_d.idesc
    add     r1, r0, d.u.read_serq_entry_d.idesc

    addi    r1, r1, PKT_DESC_AOL_OFFSET

table_read_rx_serq_dec: 
	CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tls_dec_pkt_descriptor_process,
	                    r1, TABLE_SIZE_512_BITS)

table_read_TNMDR_ALLOC_IDX:
    addi    r3, r0, TNMDR_ALLOC_IDX
	CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, tls_dec_alloc_tnmdr_process,
	                    r3, TABLE_SIZE_16_BITS)

table_read_TNMPR_ALLOC_IDX:
	addi 	r3, r0, TNMPR_ALLOC_IDX
	CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_DIS, tls_dec_alloc_tnmpr_process,
	                    r3, TABLE_SIZE_16_BITS)

tls_dec_serq_consume:
	CAPRI_NEXT_TABLE_READ_OFFSET(3, TABLE_LOCK_DIS, tls_dec_serq_consume_process,
	                    k.tls_global_phv_qstate_addr, TLS_TCB_OFFSET,
                        TABLE_SIZE_512_BITS)
        
	
	nop.e
	nop
