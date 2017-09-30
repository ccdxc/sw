/*
 * 	Implements the reading of packet descriptor
 *  Stage 2, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct phv_                 p;
struct tx_table_s2_t0_d     d;
struct tx_table_s2_t0_k     k;

%%
    .param      tls_dec_rx_serq_process
    .align

tls_dec_pkt_descriptor_process:
    phvwr   p.s2_s3_t0_phv_idesc_aol0_addr, d.{u.tls_read_pkt_descr_aol_d.A0}.dx
    phvwr   p.s2_s3_t0_phv_idesc_aol0_offset, d.{u.tls_read_pkt_descr_aol_d.O0}.wx 
    phvwr   p.s2_s3_t0_phv_idesc_aol0_len, d.{u.tls_read_pkt_descr_aol_d.L0}.wx

    phvwr   p.idesc_A0, d.u.tls_read_pkt_descr_aol_d.A0
    add     r2, r0, d.{u.tls_read_pkt_descr_aol_d.A0}.dx
    add     r1, r0, d.{u.tls_read_pkt_descr_aol_d.O0}.wx
#if 1
    /* FIXME: Workaround to DoL packet injection issue */
    addi    r1, r1, 4
#endif
    phvwr   p.idesc_O0, r1.wx

    
    add     r3, r0, d.{u.tls_read_pkt_descr_aol_d.L0}.wx
    subi    r3, r3, TLS_AES_GCM_AUTH_TAG_SIZE
#if 1
    /* FIXME: Workaround to DoL packet injection issue */
    subi    r3, r3, 20
#endif
    phvwr   p.idesc_L0, r3.wx

    /* Setup DMA command to write the AAD */
    add     r3, r2, r1

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd0_dma_cmd, r3, s4_s6_t0_phv_aad_seq_num,
                                s4_s6_t0_phv_aad_length)

    /* Setup barco command authentication tag address */
    add     r1, d.{u.tls_read_pkt_descr_aol_d.A0}.dx, d.{u.tls_read_pkt_descr_aol_d.O0}.wx
    add     r1, r1, d.{u.tls_read_pkt_descr_aol_d.L0}.wx
#if 1
    /* FIXME: Workaround to DoL packet injection issue */
    subi    r1, r1, 16
#endif
    sub     r1, r1, TLS_AES_GCM_AUTH_TAG_SIZE
    phvwr   p.barco_desc_auth_tag_addr, r1.dx

	CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_dec_rx_serq_process,
	                    k.tls_global_phv_qstate_addr, TLS_TCB_CRYPT_OFFSET,
                        TABLE_SIZE_512_BITS)

tls_pkt_descriptor_process_done:
	nop.e
	nop
