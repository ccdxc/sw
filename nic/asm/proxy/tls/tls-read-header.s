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

struct k_struct {
	tls_hdr_addr			 : ADDRESS_WIDTH ;
	pending_rx_brq                   : 1		 ;
	pending_rx_serq                  : 1		 ;
	enc_flow                         : 1		 ;
};

struct phv_ p	;
struct k_struct k	;
struct d_struct d	;
	
%%
	.param		tls_alloc_tnmdr_start
	
tls_read_header_process_start:
	phvwr		p.tls_global_phv_tls_hdr_type, d.tls_hdr_type
	phvwr		p.tls_global_phv_tls_hdr_version_major, d.tls_hdr_version_major
	phvwr		p.tls_global_phv_tls_hdr_version_minor, d.tls_hdr_version_minor
	phvwr		p.tls_global_phv_tls_hdr_len, d.tls_hdr_len
	
	sne		c1, k.pending_rx_brq, r0
	bcf		[c1], table_read_alloc_sesq_pi
	nop

	seq		c1, k.pending_rx_serq, r0
	bcf		[c1], tls_read_desc_process_done
	nop

table_read_alloc_tnmdr:
	phvwri		p.tls_global_phv_split, 1
	addi 		r3, r0, TNMDR_ALLOC_IDX
	CAPRI_NEXT_IDX0_READ(TABLE_LOCK_DIS, tls_alloc_tnmdr_start,
	                    r3, TABLE_SIZE_16_BITS)

	
	b		tls_read_desc_process_done
	nop
	
table_read_alloc_sesq_pi:
#	TLS_READ_IDX(SESQ_PRODUCER_IDX, TABLE_TYPE_RAW, tls_alloc_sesq_pi_process)


	
tls_read_desc_process_done:
	nop.e
	nop.e
	
