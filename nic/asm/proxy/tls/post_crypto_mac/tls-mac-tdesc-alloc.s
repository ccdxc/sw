/*
 * AES-CBC-HMAC-SHA2 Mac-then-encrypt post-mac pipeline:
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
    odesc	 : DESC_ADDRESS_WIDTH ;
    pad          : 448               ;
};

/* Readonly Parsed packet header info for the current packet */

struct tx_table_s3_t1_k k	;
struct phv_ p;
struct d_struct d;

%%
	
        .param          TNMDR_TABLE_BASE
	.param          tls_mac_read_random_iv
	    .align
tls_mac_tdesc_alloc_process:
        CAPRI_SET_DEBUG_STAGE0_3(p.to_s5_debug_stage0_3_thread, CAPRI_MPU_STAGE_3, CAPRI_MPU_TABLE_1)
        CAPRI_CLEAR_TABLE1_VALID

        phvwr	    p.to_s4_odesc, d.odesc
        phvwri      p.to_s5_tnmdr_alloc, 1

	CAPRI_BARCO_DRBG_RANDOM0_GENERATE(r1, r2)

	CAPRI_NEXT_TABLE_READ_i(1, TABLE_LOCK_DIS, tls_mac_read_random_iv,
	                        CAPRI_BARCO_MD_HENS_REG_DRBG_RANDOM_NUM0, TABLE_SIZE_64_BITS)
tls_mac_tdesc_alloc_done:	
	nop.e
	nop

