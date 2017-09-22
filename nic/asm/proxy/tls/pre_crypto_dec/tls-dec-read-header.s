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
	
%%
	.param		tls_dec_alloc_tnmdr_process
    .param      tls_dec_bld_barco_req_process
	
tls_dec_read_header_process:
    /* Setup AAD using the incoming TLS record information */
    /* FIXME: Not being used, we use inplace AAD, hence remove this */
	phvwr		p.s4_s6_t0_phv_aad_type, d.u.tls_read_tls_header_d.tls_hdr_type
	phvwr		p.s4_s6_t0_phv_aad_version_major, d.u.tls_read_tls_header_d.tls_hdr_version_major
	phvwr		p.s4_s6_t0_phv_aad_version_minor, d.u.tls_read_tls_header_d.tls_hdr_version_minor
	phvwr		p.s4_s6_t0_phv_aad_length, d.u.tls_read_tls_header_d.tls_hdr_len

    add         r1, r0, d.u.tls_read_tls_header_d.tls_iv

    tblwr.f     d.u.tls_read_tls_header_d.tls_iv, k.crypto_iv_explicit_iv

    phvwr       p.crypto_iv_explicit_iv, r1


    phvwr       p.to_s6_cur_tls_data_len, d.u.tls_read_tls_header_d.tls_hdr_len

    CAPRI_OPERAND_DEBUG(d.u.tls_read_tls_header_d.tls_iv)
	
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
	
