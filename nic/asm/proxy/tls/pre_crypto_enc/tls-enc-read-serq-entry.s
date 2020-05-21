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
        .param          tls_enc_pkt_descriptor_process

tls_enc_read_serq_entry_process:
#ifdef ELBA
    /* memwr.h k.to_s1_serq_prod_ci_addr, k.to_s1_serq_ci */
#else
    memwr.h k.to_s1_serq_prod_ci_addr, k.to_s1_serq_ci
#endif
    CAPRI_SET_DEBUG_STAGE0_3(p.to_s6_debug_stage0_3_thread, CAPRI_MPU_STAGE_1, CAPRI_MPU_TABLE_0)
    add     r2, r0, d.{idesc}
    phvwr   p.bsq_slot_desc, r2

    phvwrpair   p.to_s4_idesc, r2, p.to_s5_idesc, r2

    addi    r2, r2, PKT_DESC_AOL_OFFSET

table_read_rx_serq_enc:
	CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS, tls_enc_rx_serq_process,
	                    k.tls_global_phv_qstate_addr, TLS_TCB_CONFIG,
                        TABLE_SIZE_512_BITS)

table_read_idesc:
	CAPRI_NEXT_TABLE_READ(3, TABLE_LOCK_DIS, tls_enc_pkt_descriptor_process,
	                    r2, TABLE_SIZE_512_BITS)

    /* Skip allocating the descriptor and the page when we are bypassing Barco offload */
    smeqb   c1, k.tls_global_phv_debug_dol, TLS_DDOL_BYPASS_BARCO, TLS_DDOL_BYPASS_BARCO
    bcf     [c1], tls_enc_read_serq_entry_process_done
    nop

table_read_TNMDR_ALLOC_IDX:
    addi    r3, r0, TNMDPR_ALLOC_IDX
	CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, tls_enc_alloc_tnmdr_process,
	                    r3, TABLE_SIZE_64_BITS)

tls_enc_read_serq_entry_process_done:

	nop.e
	nop

