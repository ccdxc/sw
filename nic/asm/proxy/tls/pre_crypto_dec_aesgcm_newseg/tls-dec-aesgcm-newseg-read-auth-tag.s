/*
 * 	Implements the reading of the Auth Tag from the segment
 *  Stage 4, Table 3
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

#define D(field)    d.{u.read_auth_tag_d.##field}
#define K(field)    k.{##field}

struct phv_             p;
struct tx_table_s4_t3_d d;
struct tx_table_s4_t3_k k;

%%
    .param          tls_dec_aesgcm_read_tnmdr_idesc

/*
    GPR Usage:
    r2  - Bytes read via table read
    r6  - D Read pointer
    r7  - PHV Write pointer
*/

tls_dec_aesgcm_read_authtag_alloc_idesc:
    add             r1, r0, K(to_s4_auth_tag_to_read)
    bnei            r1, 1, tls_dec_aesgcm_check_alloc_idesc
    nop
    /* Read AuthTag */
#define COPY_D_TO_PHV(bytes)                \
    subi            r6, r6, (bytes * 8);    \
    subi            r7, r7, (bytes * 8);    \
    tblrdp          r5, r6, 0, (bytes * 8); \
    phvwrp          r7, 0, (bytes * 8), r5; \
    subi            r2, r2, bytes;

    /* Use r6 as the pointer to D, start at Byte 0 */
    addi            r6, r0, (512)

    /* Use r7 as the pointer to the beginning of AuthTag storage in PHV */
    /*  In this stage, we know that we do not have any prior
        accumulated bytes for AuthTag
    */
    add             r7, r0, offsetof(p, s4_t3_to_s5_t0_byte0);
    addi            r7, r7, 8 /* Position to end of first byte */

    /* to_s4_auth_tag_bytes_to_read is ('bytes to read' - 1) */
    add             r4, r0, K(to_s4_auth_tag_bytes_to_read)
    
    /*  TODO: As an optimization of the block below, we could
        unconditionlly copy 16 bytes of the D vector
    */
    .brbegin
    br              r4[3:0]
    nop

    .brcase 0
    COPY_D_TO_PHV(1)
    b               tls_dec_aesgcm_read_authtag_done
    nop

    .brcase 1
    COPY_D_TO_PHV(2)
    b               tls_dec_aesgcm_read_authtag_done
    nop
    
    .brcase 2
    COPY_D_TO_PHV(3)
    b               tls_dec_aesgcm_read_authtag_done
    nop

    .brcase 3
    COPY_D_TO_PHV(4)
    b               tls_dec_aesgcm_read_authtag_done
    nop

    .brcase 4
    COPY_D_TO_PHV(5)
    b               tls_dec_aesgcm_read_authtag_done
    nop

    .brcase 5
    COPY_D_TO_PHV(6)
    b               tls_dec_aesgcm_read_authtag_done
    nop

    .brcase 6
    COPY_D_TO_PHV(7)
    b               tls_dec_aesgcm_read_authtag_done
    nop

    .brcase 7
    COPY_D_TO_PHV(8)
    b               tls_dec_aesgcm_read_authtag_done
    nop

    .brcase 8
    COPY_D_TO_PHV(8)
    COPY_D_TO_PHV(1)
    b               tls_dec_aesgcm_read_authtag_done
    nop

    .brcase 9
    COPY_D_TO_PHV(8)
    COPY_D_TO_PHV(2)
    b               tls_dec_aesgcm_read_authtag_done
    nop
    
    .brcase 10
    COPY_D_TO_PHV(8)
    COPY_D_TO_PHV(3)
    b               tls_dec_aesgcm_read_authtag_done
    nop

    .brcase 11
    COPY_D_TO_PHV(8)
    COPY_D_TO_PHV(4)
    b               tls_dec_aesgcm_read_authtag_done
    nop

    .brcase 12
    COPY_D_TO_PHV(8)
    COPY_D_TO_PHV(5)
    b               tls_dec_aesgcm_read_authtag_done
    nop

    .brcase 13
    COPY_D_TO_PHV(8)
    COPY_D_TO_PHV(6)
    b               tls_dec_aesgcm_read_authtag_done
    nop

    .brcase 14
    COPY_D_TO_PHV(8)
    COPY_D_TO_PHV(7)
    b               tls_dec_aesgcm_read_authtag_done
    nop

    .brcase 15
    COPY_D_TO_PHV(8)
    COPY_D_TO_PHV(8)

    .brend

tls_dec_aesgcm_read_authtag_done:
    phvwr           p.to_s5_auth_tag_bytes_read, K(to_s4_auth_tag_bytes_to_read)
    phvwri          p.to_s5_auth_tag_read, 1


tls_dec_aesgcm_check_alloc_idesc:
    add             r1, r0, K(to_s4_alloc_idesc)
    bnei            r1, 1, tls_dec_aesgcm_skip_alloc_idesc
    nop

    /* Setup idesc allocation */
    addi            r1, r0, TNMDR_ALLOC_IDX
    CAPRI_NEXT_TABLE_READ(3, TABLE_LOCK_DIS, tls_dec_aesgcm_read_tnmdr_idesc,
                        r1, TABLE_SIZE_16_BITS)

tls_dec_aesgcm_read_authtag_alloc_idesc_done:
    nop.e
    nop

tls_dec_aesgcm_skip_alloc_idesc:
    CAPRI_CLEAR_TABLE3_VALID

    nop.e
    nop
