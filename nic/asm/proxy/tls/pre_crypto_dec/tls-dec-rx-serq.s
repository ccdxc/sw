/*
 *      Implements the processing of Packet Descriptor
 *  Stage 2, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
        
struct tx_table_s2_t0_k k;
struct phv_             p;
struct tx_table_s2_t0_d d;

#define D   d.u.tls_serq_process_d
        
%%
        .param      tls_dec_read_header_process
        
tls_dec_rx_serq_process:

        seq         c1, k.tls_global_phv_do_pre_ccm_dec, 1
        phvwr.!c1   p.crypto_iv_salt, D.salt
        phvwr.c1    p.ccm_header_with_aad_B_0_nonce_salt, D.salt

        phvwr       p.barco_desc_key_desc_index, D.barco_key_desc_index
dma_cmd_dec_desc_entry_last:
        

no_dma_cmd:
        add             r2, r0, k.s1_s2_t0_phv_idesc_aol0_addr
        add             r3, r0, k.s1_s2_t0_phv_idesc_aol0_offset
        add             r2, r2, r3
        phvwrpair       p.s2_s3_t0_phv_idesc_aol0_addr, k.s1_s2_t0_phv_idesc_aol0_addr, \
                        p.s2_s3_t0_phv_idesc_aol0_offset, k.s1_s2_t0_phv_idesc_aol0_offset
        phvwr           p.s2_s3_t0_phv_idesc_aol0_len, k.s1_s2_t0_phv_idesc_aol0_len

table_read_tls_header:
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tls_dec_read_header_process, r2, TABLE_SIZE_512_BITS)
        
tls_dec_rx_serq_process_done:
        nop.e
        nop


