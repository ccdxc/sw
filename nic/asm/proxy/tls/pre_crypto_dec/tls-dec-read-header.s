/*
 * 	Implements the reading of TLS header from tcp data stream
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct d_struct {
        tls_hdr_type                     : 8;
        tls_hdr_version_major            : 8;
        tls_hdr_version_minor            : 8;
        tls_hdr_len                      : 16;
};

struct tx_table_s3_t0_k k       ;
struct phv_ p	;
struct d_struct d	;
	
%%
	.param		tls_dec_alloc_tnmdr_process
	
tls_dec_read_header_process:
	phvwr		p.tls_global_phv_tls_hdr_type, d.tls_hdr_type
	phvwr		p.tls_global_phv_tls_hdr_version_major, d.tls_hdr_version_major
	phvwr		p.tls_global_phv_tls_hdr_version_minor, d.tls_hdr_version_minor
	phvwr		p.tls_global_phv_tls_hdr_len, d.tls_hdr_len
	

	seq		    c1, k.tls_global_phv_pending_rx_serq, r0
	bcf		    [c1], tls_read_desc_process_done
	nop

table_read_alloc_tnmdr:
	phvwri		p.tls_global_phv_split, 1
	addi 		r3, r0, TNMDR_ALLOC_IDX
	CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tls_dec_alloc_tnmdr_process,
	                    r3, TABLE_SIZE_16_BITS)

tls_read_desc_process_done:
	nop.e
	nop.e
	
