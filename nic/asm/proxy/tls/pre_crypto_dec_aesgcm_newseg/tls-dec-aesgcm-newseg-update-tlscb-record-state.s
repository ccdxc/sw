/*
 * 	Implements the checking of the current TLS header status
 *  Stage 4, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

#define D(field)    d.{u.update_tlscb_records_state_d.##field}
#define K(field)    k.{##field}

struct phv_             p;
struct tx_table_s4_t0_d d;
struct tx_table_s4_t0_k k;

%%
    .param          tls_dec_aesgcm_read_tls_hdr_or_auth_tag
    .param          tls_dec_aesgcm_process_auth_tag

/*
    GPR Usage:
    r1  - Bytes already available for TLS Hdr/AuthTag
    r2  - Bytes read via table read
    r4  - TLS Record Length
    r5  - TLS Record Pending Length
*/

tls_dec_aesgcm_process_segment:

    add             r1, r0, K(to_s4_tls_hdr_read)
    beqi            r1, 1, tls_dec_aesgcm_process_tls_hdr_read
    nop

    add             r1, r0, K(to_s4_auth_tag_read)
    beqi            r1, 1, tls_dec_aesgcm_process_auth_tag_read
    nop

    b               tls_dec_aesgcm_process_intermediate_segment
    nop


tls_dec_aesgcm_process_tls_hdr_read:

    /* Update CB with the bytes read */ 
    /* Read: type, length and explicit IV from S2S area as applicable */

    add             r1, r0, D(cur_tls_header_len)
    add             r2, r0, K(to_s4_bytes_read)

    blti            r1, 1, tls_dec_aesgcm_procseg_tls_hdr_all
    nop

    blti            r1, 2, tls_dec_aesgcm_procseg_tls_hdr_after_type
    nop

    blti            r1, 3, tls_dec_aesgcm_procseg_tls_hdr_after_majver
    nop

    blti            r1, 4, tls_dec_aesgcm_procseg_tls_hdr_after_minver
    nop

    /* We already have atleast TLS_Len MSB, store that in r4 */
    add             r4, r0, D(tls_rec_len)

    blti            r1, 5, tls_dec_aesgcm_procseg_tls_hdr_after_partlen
    nop

    add             r5, r0, D(cur_tls_record_pend_len)

    b               tls_dec_aesgcm_procseg_tls_hdr_iv
    nop


tls_dec_aesgcm_procseg_tls_hdr_all:
    /* Copy TLS message type to CB */
    tblwr           D(tls_hdr_type), K(s2_to_s3_to_s4_t0_byte0)
    subi            r2, r2, 1
    beqi            r2, 0, tls_dec_aesgcm_procseg_tls_hdr_done
    nop

tls_dec_aesgcm_procseg_tls_hdr_after_type:
    /* Major version already verified in the previous stage */
    subi            r2, r2, 1
    beqi            r2, 0, tls_dec_aesgcm_procseg_tls_hdr_done
    nop

tls_dec_aesgcm_procseg_tls_hdr_after_majver:
    /* Minor version already verified in the previous stage */
    subi            r2, r2, 1
    beqi            r2, 0, tls_dec_aesgcm_procseg_tls_hdr_done
    nop

tls_dec_aesgcm_procseg_tls_hdr_after_minver:
    /* Save MSB of Len field in r4 */
    add             r4, r0, \
                    K(s2_to_s3_to_s4_t0_byte3...s2_to_s3_to_s4_t0_byte4)
    tblwr           D(tls_rec_len), r4
    subi            r2, r2, 1
    beqi            r2, 0, tls_dec_aesgcm_procseg_tls_hdr_done
    nop

tls_dec_aesgcm_procseg_tls_hdr_after_partlen:
    /* Save TLS Record Len field in CB */
    add             r4, r0, \
                    K(s2_to_s3_to_s4_t0_byte3...s2_to_s3_to_s4_t0_byte4)
    phvwr           p.tls_global_phv_tls_rec_len, r4
    tblwr           D(tls_rec_len), r4
    add             r5, r0, r4  /* r5 - TLS Record Pending Length */
    tblwr           D(cur_tls_record_pend_len), r5
    tblwr           D(auth_tag_len), 0
    subi            r2, r2, 1
    beqi            r2, 0, tls_dec_aesgcm_procseg_tls_hdr_done
    nop

tls_dec_aesgcm_procseg_tls_hdr_iv:
    /* r2 - bytes read towards IV */
    tblwr           D(tls_explicit_iv), \
                    K(s2_to_s3_to_s4_t0_byte5...s2_to_s3_to_s4_t0_byte12)

tls_dec_aesgcm_procseg_tls_hdr_done:
    add             r1, D(cur_tls_header_len),  K(to_s4_bytes_read)
    tblwr           D(cur_tls_header_len), r1
    blti            r1, (NTLS_TLS_HEADER_SIZE + NTLS_IV_SIZE), \
                    tls_dec_aesgcm_procseg_incomplete_tls_hdr
    nop


    /*
        r2  - IV bytes for the record in this segment
        r5  - TLS Record Pending Length
    */

    /*  Update 'first_segment' flag if we have pending bytes in the segment
        after having read the bytes for the TLS header and IV
    */
    /* If (tls_global_phv_L0 - 'TLS Header + IV bytes read') != 0 */
    sub             r1, K(tls_global_phv_L0), K(to_s4_bytes_read)
    sne             c2, r1, r0
    bcf             [!c2], tls_dec_aesgcm_procseg_no_payload
    nop

    /* We have the first bytes from the payload for this TLS record */
    phvwri          p.tls_global_phv_first_segment, 1

    /* Verify if there are going to be bytes pending from this segment */
    /* If (tls_global_phv_L0 - 'TLS Header + IV bytes read') >
                    (D(cur_tls_record_pend_len) - IVLen)
    */
    /* r1   = tls_global_phv_L0 - 'TLS Header + IV bytes read' */
    addi            r3, r0, NTLS_IV_SIZE
    sub             r3, r5, r3
    sle             c1, r1, r3
    bcf             [c1], tls_dec_aesgcm_procseg_no_bytes_beyond_the_record
    nop
#if 1
    /* Update pending bytes in curr_idesc */
    phvwri          p.tls_global_phv_segment_pending, 1
    phvwr           p.curr_idesc_A0, K(tls_global_phv_A0).dx
    /* curr_idesc_O0 = (tls_global_phv_O0 + to_s4_segment_bytes_consumed) */
    add             r4, K(tls_global_phv_O0), K(to_s4_segment_bytes_consumed)
    phvwr           p.curr_idesc_O0, r4.wx
    /* curr_idesc_L0 = (tls_global_phv_L0 - to_s4_segment_bytes_consumed) */
    sub             r4, K(tls_global_phv_L0), K(to_s4_segment_bytes_consumed)
    phvwr           p.curr_idesc_L0, r4.wx
#endif
tls_dec_aesgcm_procseg_no_bytes_beyond_the_record:
    /* Reset currnt segment information in CB */
    tblwr.c1        D(curr_segment), 0
    tblwr.c1        D(curr_segment_aol_a), 0
    tblwr.c1        D(curr_segment_aol_o), 0
    tblwr.c1        D(curr_segment_aol_l), 0

    /* Update idesc with current segment information */
    /* Since this is the first segment in the record,
    we setup AOL1 with the payload. AOL0 is used for
    AAD
    */
    phvwr           p.idesc_A1, K(tls_global_phv_A0).dx
    /* idesc_O1 = (tls_global_phv_O0 + 'TLS Header + IV bytes read') */
    add             r4, K(tls_global_phv_O0), K(to_s4_bytes_read)
    phvwr           p.idesc_O1, r4.wx
    /* idesc_L1 = (to_s4_segment_bytes_consumed - 'TLS Header + IV bytes read'
                - 'Auth Tag Bytes Read (if any)')
    */
    sub             r4, K(to_s4_segment_bytes_consumed), K(to_s4_bytes_read)
    seq             c1, K(to_s4_auth_tag_to_read), 1
    sub.c1          r4, r4, K(to_s4_auth_tag_bytes_to_read)
    subi.c1         r4, r4, 1   /* to_s4_auth_tag_bytes_to_read is off by 1 */
    phvwr           p.idesc_L1, r4.wx

    phvwr           p.to_s7_idesc, K(to_s4_idesc)

    /* Account for bytes towards the TLS record in
        D(cur_tls_record_pend_len)
       D(cur_tls_record_pend_len) -= 'IV bytes read' +
        to_s4_segment_bytes_consumed - 'TLS Header + IV bytes read'
    */
    add             r4, r2, K(to_s4_segment_bytes_consumed)
    sub             r4, r4, K(to_s4_bytes_read)
    sub             r5, r5, r4
    tblwr           D(cur_tls_record_pend_len), r5
    
    /* Insert idesc onto the TLS record list */
    /* c2 - TRUE */
    add             r3, r0, K(to_s4_idesc)
    bal             r7, tls_dec_aesgcm_procseg_list_insert
    nop

    b               tls_dec_aesgcm_update_tlscb_record_state_done
    nop

tls_dec_aesgcm_procseg_no_payload:
    /* TODO: The idesc should be freed since there is reference to it 
        corner case where the segment only contained TLS header + IV
    */

    /* Fall through to tls_dec_aesgcm_procseg_incomplete_tls_hdr
        same processing needed
    */

tls_dec_aesgcm_procseg_incomplete_tls_hdr:
    /* TODO: idesc needs to be freed */
    tblwr           D(curr_segment), 0
    tblwr           D(curr_segment_aol_a), 0
    tblwr           D(curr_segment_aol_o), 0
    tblwr           D(curr_segment_aol_l), 0

    b               tls_dec_aesgcm_update_tlscb_record_state_done
    nop


tls_dec_aesgcm_process_auth_tag_read:
    tblwr           D(auth_tag_lo), K(s2_to_s3_to_s4_t0_byte0...s2_to_s3_to_s4_t0_byte7)
    tblwr           D(auth_tag_hi), K(s2_to_s3_to_s4_t0_byte8...s2_to_s3_to_s4_t0_byte15)

    add             r1, r0, D(auth_tag_len)
    add             r1, r1, K(to_s4_bytes_read)
    add             r1, r1, 1
    tblwr           D(auth_tag_len), r1

    /* Update curr_idesc if there are pending bytes in the segment */
    /* If (tls_global_phv_L0 > to_s4_segment_bytes_consumed) */
    add             r1, r0, K(tls_global_phv_L0)
    add             r2, r0, K(to_s4_segment_bytes_consumed)
    ble             r1, r2, tls_dec_aesgcm_process_auth_tag_no_pending_segment
    nop

    phvwri          p.tls_global_phv_segment_pending, 1
    phvwr           p.curr_idesc_A0, K(tls_global_phv_A0)
    add             r4, K(tls_global_phv_O0), r2
    /* tls_global_phv_O0 + to_s4_segment_bytes_consumed */
    phvwr           p.curr_idesc_O0, r4
    sub             r4, K(tls_global_phv_L0), r2
    /* tls_global_phv_L0 - to_s4_segment_bytes_consumed */
    phvwr           p.curr_idesc_L0, r4

tls_dec_aesgcm_process_auth_tag_no_pending_segment:
    add             r5, r0, D(cur_tls_record_pend_len)
    blei            r5, TLS_AES_GCM_AUTH_TAG_SIZE, tls_dec_aesgcm_process_auth_tag_no_payload

    phvwr           p.idesc_A0, K(tls_global_phv_A0).dx
    phvwr           p.idesc_O0, K(tls_global_phv_O0).wx
    /* idesc_L0 = (to_s4_segment_bytes_consumed - 'Auth Tag Bytes read') */
    sub             r4, K(to_s4_segment_bytes_consumed), K(to_s4_bytes_read)
    sub             r4, r4, 1 /* Adjust for Auth Tag bytes read 
                                which is off by one */
    phvwr           p.idesc_L0, r4.wx

    phvwr           p.to_s7_idesc, K(to_s4_idesc)

    /* Insert idesc onto the TLS record list */
    /* reset c2 */
    setcf           c2, [!c0]
    add             r3, r0, K(to_s4_idesc)
    bal             r7, tls_dec_aesgcm_procseg_list_insert
    nop

    sub             r5, r5, K(to_s4_segment_bytes_consumed)
    tblwr           D(cur_tls_record_pend_len), r5

    b               tls_dec_aesgcm_update_tlscb_record_state_done
    nop

tls_dec_aesgcm_process_auth_tag_no_payload:

    /* TODO: Skip ipage refcounting in this scenario */
    /* Update D(cur_tls_record_pend_len) */
    /* D(cur_tls_record_pend_len) -= to_s4_segment_bytes_consumed */
    phvwri          p.tls_global_phv_no_payload, 1
    sub             r5, r5, K(to_s4_segment_bytes_consumed)
    tblwr           D(cur_tls_record_pend_len), r5

    b               tls_dec_aesgcm_update_tlscb_record_state_done
    nop


tls_dec_aesgcm_process_intermediate_segment:
    /* Intermediate segment - complete segment consumed */

    /* TODO: */


tls_dec_aesgcm_update_tlscb_record_state_done:

	CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_dec_aesgcm_process_auth_tag,
	                    K(tls_global_phv_qstate_addr),
                            TLS_TCB_OPER_DATA_OFFSET,
                            TABLE_SIZE_512_BITS)

	nop.e
	nop



/* list_insert(idesc) */
tls_dec_aesgcm_procseg_list_insert:
    /*  
        c2  - If set, indicates that the segment is the first in the TLS record
        r3  - idesc 
        r7  - Return PC address
    */
    bcf             [!c2], tls_dec_aesgcm_procseg_list_insert_existing_record
    nop

tls_dec_aesgcm_procseg_list_insert_new_record:
    add             r6, r0, D(qtail)   
    beq             r6, r0, tls_dec_aesgcm_procseg_list_insert_first_record
    nop

    tblwr           D(qtail), r3
    tblwr           D(tls_rec_tail), r3
    /* TODO: barco_send  */
    phvwr           p.phv_scratch_new_tail_addr, r3
    /* Write address - next_pkt field */
    addi            r1, r6, PKT_DESC_AOL_OFFSET_NXT_PKT
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd_tls_rec_list_dma_cmd, r1,   \
        phv_scratch_new_tail_addr, phv_scratch_new_tail_addr)
    jr              r7
    nop

tls_dec_aesgcm_procseg_list_insert_first_record:
    tblwr           D(qhead), r3
    tblwr           D(qtail), r3
    tblwr           D(barco_send), r3
    tblwr           D(barco_una), r3
    tblwr           D(tls_rec_tail), r3
    jr              r7
    nop

tls_dec_aesgcm_procseg_list_insert_existing_record:
    add             r6, r0, D(tls_rec_tail)   
    beq             r6, r0, tls_dec_aesgcm_procseg_list_insert_existing_record_err
    nop

    tblwr           D(tls_rec_tail), r3
    add             r3, r3, PKT_DESC_AOL_OFFSET
    phvwr           p.phv_scratch_new_tail_addr, r3.dx
    /* Write address - next_addr field */
    addi            r1, r6, PKT_DESC_AOL_OFFSET_NXT_ADDR
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd_tls_rec_list_dma_cmd, r1,   \
        phv_scratch_new_tail_addr, phv_scratch_new_tail_addr)
    jr              r7
    nop

tls_dec_aesgcm_procseg_list_insert_existing_record_err:
    nop.e
    nop
