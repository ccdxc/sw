/*
 *	Implements the tnmdr ring desc of the TxDMA P4+ tls  pipeline
 *  Stage 3, Table 1
 */
#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

 /* d is the data returned by lookup result */
struct d_struct {
	odesc			 : DESC_ADDRESS_WIDTH ;
    pad              : 448               ;
};

/* Readonly Parsed packet header info for the current packet */

struct tx_table_s3_t1_k k	;
struct phv_ p;
struct d_struct d;

%%
	
        .param          TNMDR_TABLE_BASE
	.param          tls_enc_read_random_iv
	    .align
tls_enc_tdesc_alloc_process:
    CAPRI_SET_DEBUG_STAGE0_3(p.to_s6_debug_stage0_3_thread, CAPRI_MPU_STAGE_3, CAPRI_MPU_TABLE_1)
    CAPRI_CLEAR_TABLE1_VALID

    phvwrpair   p.to_s4_odesc, d.odesc[31:0], p.to_s5_odesc, d.odesc[31:0]

    phvwri      p.to_s6_tnmdpr_alloc, 1

    /* Derive page address from the descriptor address */
    add         r2, d.odesc, ASIC_NMDPR_PAGE_OFFSET
    phvwr	    p.to_s5_opage, r2

        /*
         * When set to use random IV from barco DRBG, we'll launch a table-read program to generate and
	 * read a random value from the DRBG cryptoram to use as explicit-IV field for encrypt request.
         */
	smeqb       c1, k.tls_global_phv_debug_dol, TLS_DDOL_EXPLICIT_IV_USE_RANDOM, TLS_DDOL_EXPLICIT_IV_USE_RANDOM
	b.!c1       tls_enc_serq_consume_done
	nop

	CAPRI_BARCO_DRBG_RANDOM0_GENERATE(r1, r2)

	CAPRI_NEXT_TABLE_READ_i(1, TABLE_LOCK_DIS, tls_enc_read_random_iv,
	                        CAPRI_BARCO_MD_HENS_REG_DRBG_RANDOM_NUM0, TABLE_SIZE_32_BITS)
tls_enc_serq_consume_done:	
	nop.e
	nop

