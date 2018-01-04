/*
 * 	Implements the checking of the current TLS header status
 *  Stage 2, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

#define D(field)    d.{u.check_tls_hdr_d.##field}
#define K(field)    k.{##field}

struct phv_             p;
struct tx_table_s2_t0_d d;
struct tx_table_s2_t0_k k;

%%
    .param          tls_dec_aesgcm_read_tls_hdr_or_auth_tag
    .param          tls_dec_aesgcm_read_tlscb_blk2

tls_dec_aesgcm_check_tls_hdr:

    addi            r3, r0, (NTLS_TLS_HEADER_SIZE + NTLS_IV_SIZE)
    slt             c1, D(cur_tls_header_len), r3

    /* Restore TLS header (partial/complete) from CB into S2S PHV */
    /*    - TLS Type    */
    phvwr           p.s2_to_s3_to_s4_t0_byte0, D(tls_hdr_type)
    /*    - TLS Len     */
    phvwr           p.tls_global_phv_tls_rec_len, D(tls_rec_len)

    phvwr           p.{s2_to_s3_to_s4_t0_byte3...s2_to_s3_to_s4_t0_byte4}, \
                    D(tls_rec_len)

    /*    - IV */
    add             r1, r0, D(tls_explicit_iv)
    phvwr           p.{s2_to_s3_to_s4_t0_byte5...s2_to_s3_to_s4_t0_byte12}, r1

    bcf             [!c1], tls_dec_aesgcm_check_auth_tag
    nop

    /* We do not have complete TLS header yet, setup reads from the segment */

    /* Compute read size : min(L0, (TLSHdrLen - D(cur_tls_header_len))) */
    addi            r1, r0, (NTLS_TLS_HEADER_SIZE + NTLS_IV_SIZE)
    sub             r1, r1, D(cur_tls_header_len)
    slt             c2, r1, K(tls_global_phv_L0)
    add.c2          r2, r0, r1
    add.!c2         r2, r0, K(tls_global_phv_L0)
    
    phvwri          p.to_s3_tls_hdr_read, 1
    phvwri          p.to_s3_auth_tag_read, 0
    phvwr           p.to_s3_bytes_avail, D(cur_tls_header_len)
    phvwr           p.to_s3_bytes_to_read, r2
#if 0
    /* Adjust AOL O and L for bytes being consumed from the segment */
    add             r1, K(tls_global_phv_O0), r2
    phvwr           p.tls_global_phv_O0, r1
    sub             r1, K(tls_global_phv_L0), r2
    phvwr           p.tls_global_phv_L0, r1
#endif

    add             r3, K(tls_global_phv_A0), K(tls_global_phv_O0)

    /* Read a maximum of 13 bytes */
    /* Capri expects all addressed to be naturally aligned if the read is < 64B
        reading 512 bits as a work around for now.
    */
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tls_dec_aesgcm_read_tls_hdr_or_auth_tag, r3,
            TABLE_SIZE_512_BITS)

    CAPRI_NEXT_TABLE_READ_OFFSET(1, TABLE_LOCK_EN, tls_dec_aesgcm_read_tlscb_blk2,
	                         K(tls_global_phv_qstate_addr),
                                 TLS_TCB_CRYPT_OFFSET,
                                 TABLE_SIZE_512_BITS)
    nop.e
    nop

tls_dec_aesgcm_check_auth_tag:

    /* Fetch AuthTag, copy what we have into PHV */
    /* TODO: Revisit byte ordering */
    /* bytes 0 - 7 */
    /* TODO: phvwrpair ? */
    add             r1, r0, D(auth_tag_lo)
    phvwr           p.{s2_to_s3_to_s4_t0_byte0...s2_to_s3_to_s4_t0_byte7}, r1

    /* bytes 8 - 15 */
    add             r1, r0, D(auth_tag_hi)
    phvwr           p.{s2_to_s3_to_s4_t0_byte8...s2_to_s3_to_s4_t0_byte15}, r1

    /* if (D.cur_tls_record_pending_len >= AuthTagLen) */
    slt             c1, D(cur_tls_record_pend_len), TLS_AES_GCM_AUTH_TAG_SIZE
    bcf             [c1], tls_dec_aesgcm_auth_tag_partial_in_seg_at_begin
    nop

    /* if (AOL_L >= D.cur_tls_record_pending_len) */
    slt             c2, K(tls_global_phv_L0), D(cur_tls_record_pend_len)
    bcf             [c2], tls_dec_aesgcm_incomplete_tls_record
    nop

    /* Account for bytes consumed from this segment: Pending Record Length */
    phvwr           p.to_s4_segment_bytes_consumed, D(cur_tls_record_pend_len)

    /* Linear complete AuthTag in the segment */
    add             r3, K(tls_global_phv_A0), K(tls_global_phv_O0)
    add             r3, r3, D(cur_tls_record_pend_len)
    subi            r3, r3, TLS_AES_GCM_AUTH_TAG_SIZE
    add             r2, r0, TLS_AES_GCM_AUTH_TAG_SIZE
    b               tls_dec_aesgcm_read_auth_tag
    nop
    

tls_dec_aesgcm_incomplete_tls_record:
    /* Account for bytes consumed from this segment: Full segment */
    phvwr           p.to_s4_segment_bytes_consumed, K(tls_global_phv_L0)

    /* if (D.cur_tls_record_pending_len - AuthTagLen) >= AOL_L */
    addi            r4, r0, TLS_AES_GCM_AUTH_TAG_SIZE
    sub             r4, D(cur_tls_record_pend_len), r4
    slt             c3, r4, K(tls_global_phv_L0)
    bcf             [c3], tls_dec_aesgcm_auth_tag_partial_in_seg_at_end
    nop

    /* No AuthTag in the segment */ 
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, tls_dec_aesgcm_read_tls_hdr_or_auth_tag)
    b               tls_dec_aesgcm_check_tls_hdr_done
    nop
    
tls_dec_aesgcm_auth_tag_partial_in_seg_at_end:
    /* We have a part of the AuthTag in this segment at the end */
    addi            r4, r0, TLS_AES_GCM_AUTH_TAG_SIZE
    sub             r4, D(cur_tls_record_pend_len), r4

    /* Offset to read from */
    /* Offset = AOL_A + AOL_O + D.cur_tls_record_pending_len - AuthTagLen */
    add             r3, K(tls_global_phv_A0), K(tls_global_phv_O0)
    add             r3, r3, r4

    /* Length to read */
    /* BytesToRead = AOL_L - (D.cur_tls_record_pending_len - AuthTagLen) */
    sub             r2, K(tls_global_phv_L0), r4
    b               tls_dec_aesgcm_read_auth_tag
    nop

tls_dec_aesgcm_auth_tag_partial_in_seg_at_begin:
    /* BytesToRead = min((AuthTagLen - D.auth_tag_len), AOL_L) */
    addi            r1, r0, TLS_AES_GCM_AUTH_TAG_SIZE
    sub             r1, r1, D(auth_tag_len)

    slt             c2, r1, K(tls_global_phv_L0)
    add.c2          r2, r0, r1
    add.!c2         r2, r0, K(tls_global_phv_L0)

    /* Offset = AOL_A + AOL_O */
    add             r3, K(tls_global_phv_A0), K(tls_global_phv_O0)

tls_dec_aesgcm_read_auth_tag:
    /* Count of bytes to be read in r2 */
    /* Address to read from in r3 */
    phvwri          p.to_s3_tls_hdr_read, 0
    phvwri          p.to_s3_auth_tag_read, 1
    phvwr           p.to_s3_bytes_avail, D(auth_tag_len)
    sub             r2, r2, 1 /* Auth tag size adjusted to 0-15) */
    phvwr           p.to_s3_bytes_to_read, r2

    /* FIXME : This should be adjusted after consuming non AuthTag payload bytes for the record */
#if 0
    /* Adjust AOL O and L for bytes being consumed from the segment */
    add             r1, K(tls_global_phv_O0), r2
    phvwr           p.tls_global_phv_O0, r1
    sub             r1, K(tls_global_phv_L0), r2
    phvwr           p.tls_global_phv_L0, r1
#endif

    /* Read a maximum of 16 bytes */
    /* Capri expects all addressed to be naturally aligned if the read is < 64B
        reading 512 bits as a work around for now.
    */
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tls_dec_aesgcm_read_tls_hdr_or_auth_tag, r3,
            TABLE_SIZE_512_BITS)

    CAPRI_NEXT_TABLE_READ_OFFSET(1, TABLE_LOCK_EN, tls_dec_aesgcm_read_tlscb_blk2,
	                         K(tls_global_phv_qstate_addr),
                                 TLS_TCB_CRYPT_OFFSET,
                                 TABLE_SIZE_512_BITS)

tls_dec_aesgcm_check_tls_hdr_done:
	
	nop.e
	nop
