/* 
 *  Read odesc to setup the DMA request
 *  Stage 4, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls-sched.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct tx_table_s4_t0_d     d;
struct tx_table_s4_t0_k     k;
struct phv_                 p;

%%
    .param      tls_enc_read_aad_process

tls_enc_post_read_odesc:
    phvwr       p.odesc_A0, d.u.tls_read_odesc_d.A0
    phvwr       p.odesc_O0, d.u.tls_read_odesc_d.O0
    /* Account for 16 bytes of additional authentication tag */
    addi        r2, r0, TLS_AES_GCM_AUTH_TAG_SIZE
    add         r1, d.{u.tls_read_odesc_d.L0}.wx, r2
    phvwr       p.odesc_L0, r1.wx

    /* Compute TLS header address and setup DMA command */
    add         r1, r0, d.{u.tls_read_odesc_d.A0}.dx
    add         r1, r1, d.{u.tls_read_odesc_d.O0}.wx

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd0_dma_cmd, r1, tls_hdr_tls_hdr_type, tls_hdr_tls_iv)    

    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, tls_enc_read_aad_process,
                       	   r1, TABLE_SIZE_512_BITS)

tls_enc_post_read_odesc_done:
    nop.e
    nop.e
    
