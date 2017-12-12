/*
 * 	Implements the processing of the read Auth Tag (if any)
 *  Stage 5, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

#define D(field)    d.{u.process_auth_tag_d.##field}
#define K(field)    k.{##field}

struct phv_             p;
struct tx_table_s5_t0_d d;
struct tx_table_s5_t0_k k;

%%
    .param          tls_dec_aesgcm_read_barco_pidx
    .param          tls_dec_aesgcm_read_odesc_opage_cache

/*
    GPR Usage:
    r3  - Auth Tag Low
    r4  - Auth Tag High
    r5  - TLS Record Pending Length
*/

tls_dec_aesgcm_process_auth_tag:
    add             r1, r0, K(tls_global_phv_skip_dma)
    beqi            r1, 1, tls_dec_aesgcm_no_barco_enqueue
    nop

    add             r3, r0, D(auth_tag_lo)
    add             r4, r0, D(auth_tag_hi)

    add             r5, r0, D(cur_tls_record_pend_len)

    add             r1, r0, K(to_s5_auth_tag_read)
    bnei            r1, 1, tls_dec_aesgcm_no_auth_tag_read
    nop
    
    add             r3, r0, K(s4_t3_to_s5_t0_byte0...s4_t3_to_s5_t0_byte7)
    tblwr           D(auth_tag_lo), r3

    add             r4, r0, K(s4_t3_to_s5_t0_byte8...s4_t3_to_s5_t0_byte15)
    tblwr           D(auth_tag_hi), r4

    add             r6, K(to_s5_auth_tag_bytes_read), 1 /* Adjust bytes read - off by one */
    tbladd          D(auth_tag_len), r6

tls_dec_aesgcm_no_auth_tag_read:

    /* if D(cur_tls_record_pend_len) == 0 */
    bne             r5, r0, tls_dec_aesgcm_no_barco_enqueue
    nop

    phvwr           p.tls_global_phv_enqueue_barco, 1

    /* Setup AAD for DMA write */
    phvwr           p.aad_aad_type, D(tls_hdr_type)
    /* TODO: The version information should be setup from CB */
    /* Sequence number setup in Stage 7 Table 0 */
    phvwri          p.aad_aad_version_major, NTLS_TLS_1_2_MAJOR
    phvwri          p.aad_aad_version_minor, NTLS_TLS_1_2_MINOR
    /* AAD Length = TLSCompressed.length */
    sub             r7, D(tls_rec_len), \
                    (NTLS_AES_GCM_128_IV_SIZE + TLS_AES_GCM_AUTH_TAG_SIZE)
    phvwr           p.aad_aad_length, r7

    /* Setup IV for DMA write */
    /* Salt setup in Stage 7 Table 0 */
    phvwr           p.iv_explicit_iv, D(tls_explicit_iv)

    /* Setup AuthTag for DMA write */
    /* TODO: Use pvhwrpair ? */
    phvwr           p.auth_tag_auth_tag_lo, r3
    phvwr           p.auth_tag_auth_tag_hi, r4

    /* If this is not the first segment, retrieve the cached odesc and opage */
    add             r7, r0, K(tls_global_phv_first_segment)
    beqi            r7, 1, tls_dec_aesgcm_first_segment
    nop
    add             r1, D(qtail), PKT_DESC_ODESC_OFFSET
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, tls_dec_aesgcm_read_odesc_opage_cache, r1, TABLE_SIZE_512_BITS);

    /* Relay the record head idesc to Stage 7 */
    phvwr           p.s6_t0_to_s7_t0_rec_head_idesc, D(qtail)
    

tls_dec_aesgcm_first_segment:


    /* TODO: Replace the Barco PIDX read with the actual semaphore read */
    addi            r1, r0, CAPRI_BARCO_MD_HENS_REG_GCM0_PRODUCER_IDX
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, tls_dec_aesgcm_read_barco_pidx, r1, TABLE_SIZE_32_BITS);
    nop.e
    nop


tls_dec_aesgcm_no_barco_enqueue:
    phvwr           p.tls_global_phv_enqueue_barco, 0
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, tls_dec_aesgcm_read_barco_pidx)

tls_dec_aesgcm_process_auth_tag_done:
    nop.e
    nop
