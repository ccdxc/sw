/* 
 *  Read AAD to setup the TLS header
 *  Stage 5, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls-sched.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct tx_table_s5_t0_d     d;
struct tx_table_s5_t0_k     k;
struct phv_                 p;

%%
    .param      tls_enc_queue_sesq_process


tls_enc_read_aad_process:
    phvwr       p.tls_hdr_tls_hdr_type, d.u.tls_read_aad_d.aad_type
    phvwr       p.tls_hdr_tls_hdr_version_major, d.u.tls_read_aad_d.aad_version_major
    phvwr       p.tls_hdr_tls_hdr_version_minor, d.u.tls_read_aad_d.aad_version_minor
    phvwr       p.tls_hdr_tls_iv, d.u.tls_read_aad_d.aad_seq_num

    /* Account for additional 8 bytes of Explicit IV and 16 bytes of Auth Tag */
    add         r1, r0, d.u.tls_read_aad_d.aad_length
    addi        r1, r1, (NTLS_NONCE_SIZE + TLS_AES_GCM_AUTH_TAG_SIZE)
    phvwr       p.tls_hdr_tls_hdr_len, r1

tls_enc_read_aad_process_done:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_enc_queue_sesq_process,
                           k.tls_global_phv_qstate_addr,
                       	   TLS_TCB_OFFSET, TABLE_SIZE_512_BITS)
    nop.e
    nop
