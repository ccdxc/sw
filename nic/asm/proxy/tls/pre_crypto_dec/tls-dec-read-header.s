/*
 * 	Implements the reading of TLS header from tcp data stream
 *  Stage 4, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct tx_table_s4_t0_k k;
struct phv_             p;
struct tx_table_s4_t0_d d;

#define D d.u.tls_read_tls_header_d
	
%%
	.param		tls_dec_alloc_tnmdr_process
    .param      tls_dec_bld_barco_req_process
	
tls_dec_read_header_process:
	phvwr		p.tls_global_phv_tls_hdr_type, d.u.tls_read_tls_header_d.tls_hdr_type
	phvwr		p.tls_global_phv_tls_hdr_version_major, d.u.tls_read_tls_header_d.tls_hdr_version_major
	phvwr		p.tls_global_phv_tls_hdr_version_minor, d.u.tls_read_tls_header_d.tls_hdr_version_minor
	phvwr		p.tls_global_phv_tls_hdr_len, d.u.tls_read_tls_header_d.tls_hdr_len
	/* Check if this is a TLS handshake packet */

    addi        r1, r0, NTLS_RECORD_HANDSHAKE
    /* FIXME: To be removed once the payload is generated right at DoL */
    smeqb       c1, k.to_s4_debug_dol, TLS_DDOL_FAKE_HANDSHAKE_MSG, TLS_DDOL_FAKE_HANDSHAKE_MSG
    addi.c1     r1, r0, 0x01

    seq         c1, D.tls_hdr_type, r1
    phvwri.c1   p.tls_global_phv_write_arq, 1

    /* Remember input A,O,L for write-arq stage later */
    phvwr.c1    p.to_s6_opage, k.s3_s4_t0_phv_idesc_aol0_addr
    phvwr.c1    p.to_s6_next_tls_hdr_offset, k.s3_s4_t0_phv_idesc_aol0_offset
    phvwr.c1    p.to_s6_cur_tls_data_len, k.s3_s4_t0_phv_idesc_aol0_len      


    /* Setup AAD using the incoming TLS record information */
	phvwr		p.s4_s6_t0_phv_aad_type, d.u.tls_read_tls_header_d.tls_hdr_type
	phvwr		p.s4_s6_t0_phv_aad_version_major, d.u.tls_read_tls_header_d.tls_hdr_version_major
	phvwr		p.s4_s6_t0_phv_aad_version_minor, d.u.tls_read_tls_header_d.tls_hdr_version_minor
    /* Adjust length to TLSCompressed.length */
    add         r1, r0, d.u.tls_read_tls_header_d.tls_hdr_len
    subi        r1, r1, (NTLS_NONCE_SIZE + TLS_AES_GCM_AUTH_TAG_SIZE)
	phvwr		p.s4_s6_t0_phv_aad_length, r1

    phvwr       p.crypto_iv_explicit_iv, d.u.tls_read_tls_header_d.tls_iv

    sub         r1, k.s3_s4_t0_phv_idesc_aol0_len, k.tls_global_phv_next_tls_hdr_offset
    phvwr       p.to_s6_cur_tls_data_len, r1

	
#if 0
	seq		    c1, k.tls_global_phv_pending_rx_serq, r0
	bcf		    [c1], tls_read_desc_process_done
	nop

table_read_alloc_tnmdr:
	phvwri		p.tls_global_phv_split, 1
	addi 		r3, r0, TNMDR_ALLOC_IDX
	CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tls_dec_alloc_tnmdr_process,
	                    r3, TABLE_SIZE_16_BITS)
#endif

	CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_dec_bld_barco_req_process,
	                    k.tls_global_phv_qstate_addr, TLS_TCB_OFFSET,
                        TABLE_SIZE_512_BITS)

tls_read_desc_process_done:
	nop.e
	nop.e

