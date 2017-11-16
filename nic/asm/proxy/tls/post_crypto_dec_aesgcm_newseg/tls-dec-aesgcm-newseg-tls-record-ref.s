/*
 * 	Implements the reading of TLS record reference in the CB
 *  Stage 1, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls_common.h"
#include "ingress.h"
#include "INGRESS_p.h"

#define D(field)    d.{u.get_tls_record_ref_d.##field}
#define K(field)    k.{##field}

struct phv_             p;
struct tx_table_s1_t0_k k;
struct tx_table_s1_t0_d d;
%%

   	.param		tls_dec_post_crypto_aesgcm_newseg_get_barco_result

tls_dec_post_crypto_aesgcm_newseg_get_tls_record_ref:

    /* For now just clear the CB and setup read on idesc */
    add         r3, D(barco_send), r0
    tblwr       D(qhead), r0
    tblwr       D(qtail), r0
    tblwr       D(barco_send), r0
    tblwr       D(barco_una), r0
    tblwr       D(tls_rec_tail), 0
    tblwr       D(cur_tls_header_len), 0
    tblwr       D(tls_rec_len), 0
    tblwr       D(tls_hdr_type), 0
    tblwr       D(tls_explicit_iv), 0
    tblwr       D(auth_tag_lo), 0
    tblwr       D(auth_tag_hi), 0
    tblwr       D(auth_tag_len), 0
    tblwr       D(curr_segment), 0
    tblwr       D(curr_segment_aol_a), 0
    tblwr       D(curr_segment_aol_o), 0
    tblwr       D(curr_segment_aol_l), 0

tls_dec_post_crypto_aesgcm_newseg_get_tls_record_ref_done:
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
            tls_dec_post_crypto_aesgcm_newseg_get_barco_result, r3,
            TABLE_SIZE_512_BITS)
	nop.e
    nop
	
