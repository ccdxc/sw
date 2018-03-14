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

	/*
	 * The 'salt' field is in the 2nd 64-bit block of the TLSCB, which
	 * we're doing table-read here, so we'll populate phv for salt here.
	 */
        seq         c1, k.tls_global_phv_do_pre_ccm_dec, 1
        phvwr.!c1   p.crypto_iv_salt, D.salt
        phvwr.c1    p.ccm_header_with_aad_B_0_nonce_salt, D.salt
        CAPRI_OPERAND_DEBUG(D.salt)

        /* ENQ_DESC(*dtlsp, dec, md->seqe.desc); */
        /* if (TAIL_DESC(cb,name) == NULL) { */
        seq             c1, D.qtail, r0
        /*    TAIL_DESC(cb, name) = desc; */
        bcf             [c1], no_dma_cmd
        nop
dma_cmd_dec_desc_entry_last:
        /* SET_DESC_ENTRY(TAIL_DESC(cb,name), MAX_ENTRIES_PER_DESC - 1, desc, 0, 0); */
        
        addi            r5, r0, TLS_PHV_DMA_COMMANDS_START
        add             r4, r5, r0
        phvwr           p.p4_txdma_intr_dma_cmd_ptr, r4

        addi            r5, r0, NIC_DESC_ENTRY_L_OFFSET
        add             r5, r5, D.qtail


        phvwr           p.odesc_next_addr, k.to_s2_idesc

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd0_dma_cmd, r5, odesc_next_addr, odesc_next_addr)

no_dma_cmd:     
        tblwr.c1        D.qtail, k.to_s2_idesc
        tblwr.c1        D.qhead, k.to_s2_idesc
        
        /* dtlsp->dec_nxt.desc = TAIL_DESC(*dtlsp, dec); */
        tblwr           D.nxt_desc, k.to_s2_idesc

        /* Check if the next tls header is starting in this descriptor
         * data, if so trigger a read to get that tls header and the 
         * offset for the next tls header in the data stream 
         */
        /* if (dtlsp->next_tls_hdr_offset < md->desc.data_len) { */
        slt             c1, D.next_tls_hdr_offset, k.s1_s2_t0_phv_idesc_aol0_len
        seq             c2, D.next_tls_hdr_offset, r0
        bcf             [!c2 & !c1], tls_no_read_header
        nop
        phvwr           p.tls_global_phv_next_tls_hdr_offset, D.next_tls_hdr_offset
        add             r2, r0, k.s1_s2_t0_phv_idesc_aol0_addr
        add             r3, r0, k.s1_s2_t0_phv_idesc_aol0_offset
        add             r3, r3, k.tls_global_phv_next_tls_hdr_offset
        add             r2, r2, r3
        phvwrpair       p.s2_s3_t0_phv_idesc_aol0_addr, k.s1_s2_t0_phv_idesc_aol0_addr, \
                        p.s2_s3_t0_phv_idesc_aol0_offset, k.s1_s2_t0_phv_idesc_aol0_offset
        phvwr           p.s2_s3_t0_phv_idesc_aol0_len, k.s1_s2_t0_phv_idesc_aol0_len

table_read_tls_header:  
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tls_dec_read_header_process, r2, TABLE_SIZE_512_BITS)
        b               tls_dec_rx_serq_process_done
        nop
        /* md->next_tls_hdr_offset = dtlsp->next_tls_hdr_offset; */
        /* tls_read_header_process(&phv); */

tls_no_read_header:
        /* if (dtlsp->next_tls_hdr_offset == md->desc.data_len) { */
        seq             c1, D.next_tls_hdr_offset, r5
        
tls_dec_rx_serq_process_done:
        nop.e
        nop


