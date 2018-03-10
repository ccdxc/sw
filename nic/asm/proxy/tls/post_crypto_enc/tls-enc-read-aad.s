/* 
 *  Read AAD to setup the TLS header
 *  Stage 6, Table 1
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct tx_table_s6_t1_d     d;
struct tx_table_s6_t1_k     k;
struct phv_                 p;

%%
    .param      tls_enc_queue_sesq_process


tls_enc_read_aad_process:
    CAPRI_SET_DEBUG_STAGE4_7(p.to_s7_debug_stage4_7_thread, CAPRI_MPU_STAGE_6, CAPRI_MPU_TABLE_1)
    CAPRI_CLEAR_TABLE1_VALID
    phvwrpair   p.tls_hdr_tls_hdr_type, d.u.tls_read_aad_d.aad_type,    \
                p.tls_hdr_tls_hdr_version_major, d.u.tls_read_aad_d.aad_version_major
    phvwrpair   p.tls_hdr_tls_hdr_version_minor, d.u.tls_read_aad_d.aad_version_minor,  \
                p.tls_hdr_tls_iv, d.u.tls_read_aad_d.aad_seq_num

    /* Account for additional 8 bytes of Explicit IV and 16 bytes of Auth Tag */
    add         r1, r0, d.u.tls_read_aad_d.aad_length
    addi        r1, r1, (NTLS_NONCE_SIZE + TLS_AES_GCM_AUTH_TAG_SIZE)
    phvwr       p.tls_hdr_tls_hdr_len, r1

tls_enc_read_aad_process_done:
    nop.e
    nop
