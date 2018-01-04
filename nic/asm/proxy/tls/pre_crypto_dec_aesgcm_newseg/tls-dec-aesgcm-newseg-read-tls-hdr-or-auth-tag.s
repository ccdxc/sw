/*
 * 	Implements the checking of the current TLS header status
 *  Stage 3, Table 0
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
struct tx_table_s3_t0_d d;
struct tx_table_s3_t0_k k;

%%
    .param          tls_dec_aesgcm_process_segment
    .param          tls_dec_aesgcm_read_tnmdr_odesc
    .param          tls_dec_aesgcm_read_tnmpr_opage
    .param          tls_dec_aesgcm_read_authtag_alloc_idesc


/*
    GPR Usage:
    r1  - Bytes already available for TLS Hdr/AuthTag
    r2  - Bytes read via table read
    r3  - TLS Record Len computed after reading the header
    r4  - Accumulation of TLS Len field and scratch
    r5  - Scratch for reading contents from table 
    r6  - D Read pointer
    r7  - PHV Write pointer
*/

tls_dec_aesgcm_read_tls_hdr_or_auth_tag:

    /* Are GPRs initialized to 0 at MPU entry ? */
    add             r3, r0, r0

    add             r1, r0, K(to_s3_bytes_avail)
    add             r2, r0, K(to_s3_bytes_to_read)

    /* Use r6 as the pointer to D, start at Byte 0 */
    addi            r6, r0, (512 - 8)
    /* Use r7 as the pointer to P, start at next byte to write */
    sll             r7, r1, 3 /* Bytes available to bits */
    add             r7, r7, offsetof(p, s2_to_s3_to_s4_t0_byte0);

    seq             c1, K(to_s3_tls_hdr_read), 1
    seq             c2, K(to_s3_auth_tag_read), 1

    bcf             [c1], tls_dec_aesgcm_read_tls_hdr
    nop

    bcf             [c2], tls_dec_aesgcm_read_auth_tag
    nop

    /* Intermediate fragment not containing either the TLS Header or the Auth Tag */
    b               tls_dec_aesgcm_process_fragment
    nop


tls_dec_aesgcm_read_tls_hdr:

    phvwri          p.to_s4_tls_hdr_read, 1
    phvwr           p.to_s4_bytes_read, r2


    blti            r1, 1, tls_dec_aesgcm_read_tls_hdr_all
    nop

    blti            r1, 2, tls_dec_aesgcm_read_tls_hdr_after_type
    nop

    blti            r1, 3, tls_dec_aesgcm_read_tls_hdr_after_majver
    nop

    blti            r1, 4, tls_dec_aesgcm_read_tls_hdr_after_minver
    nop

    /* We already have TLS_Len MSB, accumulate that in r4 */
    sll             r4, K(s2_to_s3_to_s4_t0_byte3), 8
    
    blti            r1, 5, tls_dec_aesgcm_read_tls_hdr_after_partlen
    nop

    b               tls_dec_aesgcm_read_tls_hdr_iv
    nop

tls_dec_aesgcm_read_tls_hdr_all:
    tblrdp          r5, r6, 0, 8
    subi            r6, r6, 8   /* increment D read pointer */
    /* Verify TLS message type */
    indexb          r4, r5, [0x14, 0x15, 0x16, 0x17], 0
    beqi            r4, -1, tls_dec_aesgcm_read_tls_hdr_invalid_type
    nop
    phvwrp          r7, 0, 8, r5
    subi            r7, r7, 8   /* increment PHV write pointer */
    
    subi            r2, r2, 1
    beqi            r2, 0, tls_dec_aesgcm_read_tls_hdr_done
    nop
    nop

tls_dec_aesgcm_read_tls_hdr_after_type:
    tblrdp          r5, r6, 0, 8
    subi            r6, r6, 8   /* increment D read pointer */
    /* Verify major version */
    /* TODO: This should be validated against configuration from CB */
    bnei            r5, NTLS_TLS_1_2_MAJOR, tls_dec_aesgcm_read_tls_hdr_invalid_majver
    nop
    phvwrp          r7, 0, 8, r5
    subi            r7, r7, 8   /* increment PHV write pointer */
    
    subi            r2, r2, 1
    beqi            r2, 0, tls_dec_aesgcm_read_tls_hdr_done
    nop

tls_dec_aesgcm_read_tls_hdr_after_majver:
    tblrdp          r5, r6, 0, 8
    subi            r6, r6, 8   /* increment D read pointer */
    /* Verify minor version */
    /* TODO: This should be validated against configuration from CB */
    bnei            r5, NTLS_TLS_1_2_MINOR, tls_dec_aesgcm_read_tls_hdr_invalid_minver
    nop
    phvwrp          r7, 0, 8, r5
    subi            r7, r7, 8   /* increment PHV write pointer */
    
    subi            r2, r2, 1
    beqi            r2, 0, tls_dec_aesgcm_read_tls_hdr_done
    nop

tls_dec_aesgcm_read_tls_hdr_after_minver:
    /* MSB of TLS Len */
    tblrdp          r5, r6, 0, 8
    subi            r6, r6, 8   /* increment D read pointer */
    phvwrp          r7, 0, 8, r5
    subi            r7, r7, 8   /* increment PHV write pointer */

    /* Accumulate MSB of TLS len in r4 */
    sll             r4, r5, 8
    
    subi            r2, r2, 1
    beqi            r2, 0, tls_dec_aesgcm_read_tls_hdr_done
    nop

tls_dec_aesgcm_read_tls_hdr_after_partlen:
    /* LSB of TLS Len */
    tblrdp          r5, r6, 0, 8
    //subi            r6, r6, 8   /* increment D read pointer */
    phvwrp          r7, 0, 8, r5
    //subi            r7, r7, 8   /* increment PHV write pointer */

    /* TLS_Len now in r3 */
    add             r3, r4, r5
    
    subi            r2, r2, 1
    beqi            r2, 0, tls_dec_aesgcm_read_tls_hdr_done
    nop

tls_dec_aesgcm_read_tls_hdr_iv:
    /* We already have the TLS header, copy out the IV/nonce */

#define COPY_D_TO_PHV(bytes)                \
    subi            r6, r6, (bytes * 8);    \
    subi            r7, r7, (bytes * 8);    \
    tblrdp          r5, r6, 0, (bytes * 8); \
    phvwrp          r7, 0, (bytes * 8), r5; \
    subi            r2, r2, bytes;
    
    /* TODO: Optimization, copy all of 64 bits from D, len reflects the valid bytes */
    /* Adjust pending bytes for switch construct (0 - n-1) */
    subi            r4, r2, 1
    .brbegin
    br              r4[2:0]
    nop

    .brcase 0
    COPY_D_TO_PHV(1)
    b               tls_dec_aesgcm_read_tls_hdr_done
    nop

    .brcase 1
    COPY_D_TO_PHV(2)
    b               tls_dec_aesgcm_read_tls_hdr_done
    nop
    
    .brcase 2
    COPY_D_TO_PHV(3)
    b               tls_dec_aesgcm_read_tls_hdr_done
    nop

    .brcase 3
    COPY_D_TO_PHV(4)
    b               tls_dec_aesgcm_read_tls_hdr_done
    nop

    .brcase 4
    COPY_D_TO_PHV(5)
    b               tls_dec_aesgcm_read_tls_hdr_done
    nop

    .brcase 5
    COPY_D_TO_PHV(6)
    b               tls_dec_aesgcm_read_tls_hdr_done
    nop

    .brcase 6
    COPY_D_TO_PHV(7)
    b               tls_dec_aesgcm_read_tls_hdr_done
    nop

    .brcase 7
    COPY_D_TO_PHV(8)

    .brend


tls_dec_aesgcm_read_tls_hdr_done:
    /* Check if we have a complete TLS header */
    /* r1 already contains current TLS header length */
    /* if tls_hdr_len != TLS_HDR_LEN */
    add             r2, r1, K(to_s3_bytes_to_read)
    blti            r2, (NTLS_TLS_HEADER_SIZE + NTLS_IV_SIZE), tls_dec_aesgcm_read_tls_hdr_or_auth_tag_done
    nop

    /* Verify if we need to allocate odesc and opage */
    /* if AOL_L > 'bytes read' */
    add             r1, r0, K(to_s3_bytes_to_read)
    add             r2, r0, K(tls_global_phv_L0)
    ble             r2, r1, tls_dec_aesgcm_read_tls_hdr_or_auth_tag_done
    nop

    /* The segment contains the first record payload */
    addi            r1, r0, TNMDR_ALLOC_IDX
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, tls_dec_aesgcm_read_tnmdr_odesc,
                        r1, TABLE_SIZE_16_BITS)

    addi            r1, r0, TNMPR_ALLOC_IDX
    CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_DIS, tls_dec_aesgcm_read_tnmpr_opage,
                        r1, TABLE_SIZE_16_BITS)

    /* r3 - TLS Record Length */
    /* r2 - AOL_L */

    /* Bytes consumed =
        if ((D(tls_rec_len)  - IV_SIZE) >= (AOL_L - 'bytes read'))
            Bytes Consumed = AOL_L (complete segment)
        else
            Bytes Consumed = 'bytes read' + (D(tls_rec_len)  - IV_SIZE)
    */

    sub             r2, r2, K(to_s3_bytes_to_read)  /* AOL_L - 'bytes read' */
    subi            r6, r3, NTLS_IV_SIZE  /* (D(tls_rec_len)  - IV_SIZE) */
    slt             c1, r6, r2
    add.c1          r7, K(to_s3_bytes_to_read), r6
    phvwr.c1        p.to_s4_segment_bytes_consumed, r7
    phvwr.!c1       p.to_s4_segment_bytes_consumed, K(tls_global_phv_L0)


    /* Verify Auth Tag read setup */
    subi            r1, r3, (TLS_AES_GCM_AUTH_TAG_SIZE + NTLS_IV_SIZE)
    /* If ((AOL_L - 'bytes read') <=
                        (TLS_Len - NTLS_IV_SIZE - TLS_AES_GCM_AUTH_TAG_SIZE))
    */
    ble             r2, r1, tls_dec_aesgcm_read_tls_hdr_no_auth_tag_in_segment
    nop

    /* We have partial/full AuthTag to be read, setup it up */
    phvwri          p.to_s4_auth_tag_to_read, 1
    
    /* AuthTag read offset */
    add             r4, K(tls_global_phv_A0), K(tls_global_phv_O0)
    add             r4, r4, K(to_s3_bytes_to_read)
    add             r4, r4, r1 /* r1 = TLS_Len - TLS_AES_GCM_AUTH_TAG_SIZE - NTLS_IV_SIZE */
    
    /* if ((AOL_L - 'bytes read') <= (TLS_Len - NTLS_IV_SIZE)) */
    subi            r1, r3, NTLS_IV_SIZE
    ble             r2, r1, tls_dec_aesgcm_read_tls_hdr_read_auth_tag_consume_idesc
    nop

    /* We have full AuthTag and some more bytes pending in the segment */
    /* Setup AuthTag read and allocation of idesc */
    phvwri          p.to_s4_alloc_idesc, 1
    phvwri          p.to_s4_auth_tag_bytes_to_read, (TLS_AES_GCM_AUTH_TAG_SIZE - 1)
    /* Capri expects all addressed to be naturally aligned if the read is < 64B
        reading 512 bits as a work around for now.
    */
    CAPRI_NEXT_TABLE_READ(3, TABLE_LOCK_DIS, tls_dec_aesgcm_read_authtag_alloc_idesc,
                        r4, TABLE_SIZE_512_BITS)
    b               tls_dec_aesgcm_read_tls_hdr_or_auth_tag_done
    nop

tls_dec_aesgcm_read_tls_hdr_read_auth_tag_consume_idesc:
    /* Setup AuthTag read but use incoming idesc */
    /* AuthTag to read: TLS_AES_GCM_AUTH_TAG_SIZE -
                        (TLS_Len - NTLS_IV_SIZE - AOL_L - 'bytes read')
    */
    sub             r5, r1, r2 /* r1 -> TLS_Len - NTLS_IV_SIZE */
    addi            r6, r0, TLS_AES_GCM_AUTH_TAG_SIZE
    sub             r6, r6, r5
    subi            r6, r6, 1   /* Auth Tag bytes to read ranges from 0-15
                                    since it is only 4 bits wide    
                                */
    phvwr           p.to_s4_auth_tag_bytes_to_read, r6
    /* Capri expects all addressed to be naturally aligned if the read is < 64B
        reading 512 bits as a work around for now.
    */
    CAPRI_NEXT_TABLE_READ(3, TABLE_LOCK_DIS, tls_dec_aesgcm_read_authtag_alloc_idesc,
                        r4, TABLE_SIZE_512_BITS)

    b               tls_dec_aesgcm_read_tls_hdr_or_auth_tag_done
    nop

tls_dec_aesgcm_read_tls_hdr_invalid_type:
tls_dec_aesgcm_read_tls_hdr_invalid_majver:
tls_dec_aesgcm_read_tls_hdr_invalid_minver:
    CAPRI_CLEAR_TABLE_VALID(0)
    /* TBD - Error handling */
    b               tls_dec_aesgcm_read_tls_hdr_or_auth_tag_error
    nop

tls_dec_aesgcm_read_auth_tag:
    phvwr           p.to_s4_auth_tag_read, 1
    phvwr           p.to_s4_bytes_read, r2

    addi            r6, r0, (512)
    addi            r7, r7, 8 /* Position to end of first byte */

    .brbegin
    br              r2[3:0]
    nop

    .brcase 0
    COPY_D_TO_PHV(1)
    b               tls_dec_aesgcm_read_tls_hdr_or_auth_tag_done
    nop

    .brcase 1
    COPY_D_TO_PHV(2)
    b               tls_dec_aesgcm_read_tls_hdr_or_auth_tag_done
    nop
    
    .brcase 2
    COPY_D_TO_PHV(3)
    b               tls_dec_aesgcm_read_tls_hdr_or_auth_tag_done
    nop

    .brcase 3
    COPY_D_TO_PHV(4)
    b               tls_dec_aesgcm_read_tls_hdr_or_auth_tag_done
    nop

    .brcase 4
    COPY_D_TO_PHV(5)
    b               tls_dec_aesgcm_read_tls_hdr_or_auth_tag_done
    nop

    .brcase 5
    COPY_D_TO_PHV(6)
    b               tls_dec_aesgcm_read_tls_hdr_or_auth_tag_done
    nop

    .brcase 6
    COPY_D_TO_PHV(7)
    b               tls_dec_aesgcm_read_tls_hdr_or_auth_tag_done
    nop

    .brcase 7
    COPY_D_TO_PHV(8)
    b               tls_dec_aesgcm_read_tls_hdr_or_auth_tag_done
    nop

    .brcase 8
    COPY_D_TO_PHV(8)
    COPY_D_TO_PHV(1)
    b               tls_dec_aesgcm_read_tls_hdr_or_auth_tag_done
    nop

    .brcase 9
    COPY_D_TO_PHV(8)
    COPY_D_TO_PHV(2)
    b               tls_dec_aesgcm_read_tls_hdr_or_auth_tag_done
    nop
    
    .brcase 10
    COPY_D_TO_PHV(8)
    COPY_D_TO_PHV(3)
    b               tls_dec_aesgcm_read_tls_hdr_or_auth_tag_done
    nop

    .brcase 11
    COPY_D_TO_PHV(8)
    COPY_D_TO_PHV(4)
    b               tls_dec_aesgcm_read_tls_hdr_or_auth_tag_done
    nop

    .brcase 12
    COPY_D_TO_PHV(8)
    COPY_D_TO_PHV(5)
    b               tls_dec_aesgcm_read_tls_hdr_or_auth_tag_done
    nop

    .brcase 13
    COPY_D_TO_PHV(8)
    COPY_D_TO_PHV(6)
    b               tls_dec_aesgcm_read_tls_hdr_or_auth_tag_done
    nop

    .brcase 14
    COPY_D_TO_PHV(8)
    COPY_D_TO_PHV(7)
    b               tls_dec_aesgcm_read_tls_hdr_or_auth_tag_done
    nop

    .brcase 15
    COPY_D_TO_PHV(8)
    COPY_D_TO_PHV(8)

    .brend
    b               tls_dec_aesgcm_read_tls_hdr_or_auth_tag_done
    nop

tls_dec_aesgcm_process_fragment:
    /* Intermediate fragment not containing either the TLS Header or the Auth Tag */
    /* Nothing to be done, fall through */

tls_dec_aesgcm_read_tls_hdr_no_auth_tag_in_segment:
tls_dec_aesgcm_read_tls_hdr_or_auth_tag_done:
	CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_dec_aesgcm_process_segment,
	                    K(tls_global_phv_qstate_addr),
                            TLS_TCB_OPER_DATA_OFFSET,
                            TABLE_SIZE_512_BITS)

tls_dec_aesgcm_read_tls_hdr_or_auth_tag_error:	
	nop.e
	nop
