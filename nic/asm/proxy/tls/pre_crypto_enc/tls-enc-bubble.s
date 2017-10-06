/*
 * 	Bubble gum
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct tx_table_s3_t0_k k                  ;
struct phv_ p	;
	
%%
	    .param      tls_enc_bld_barco_req_process
        
tls_bubble_start:        

table_read_BLD_BARCO_ENC_REQ:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_enc_bld_barco_req_process,
                           k.tls_global_phv_qstate_addr, TLS_TCB_CRYPT_OFFSET,
                           TABLE_SIZE_512_BITS)
	nop.e
	nop
