/*
 * AES-CBC-HMAC-SHA2 Mac-then-encrypt post-mac pipeline:
 *  Read AAD to setup the TLS header in the ipage
 *  Stage 3, Table 3
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct tx_table_s3_t3_d     d;
struct tx_table_s3_t3_k     k;
struct phv_                 p;

%%

tls_mac_read_aad_process:
    CAPRI_SET_DEBUG_STAGE0_3(p.to_s5_debug_stage0_3_thread, CAPRI_MPU_STAGE_3, CAPRI_MPU_TABLE_3)
    CAPRI_CLEAR_TABLE3_VALID

    /*
     * Read the TLS header fields from AAD into phv, which will be DMA'ed to the opage along
     * with the random-IV in a later stage.
     */
    phvwr       p.tls_hdr_tls_hdr_type, d.u.tls_read_aad_d.aad_type
    phvwr       p.tls_hdr_tls_hdr_version_major, d.u.tls_read_aad_d.aad_version_major
    phvwr       p.tls_hdr_tls_hdr_version_minor, d.u.tls_read_aad_d.aad_version_minor
    phvwr       p.tls_hdr_tls_hdr_len, k.to_s3_tls_hdr_len

tls_mac_read_aad_process_done:
    nop.e
    nop
