/*
 * AES-CBC-HMAC-SHA2 Mac-then-encrypt pre-mac pipeline:
 * 	Implements the receipt of tls encrypt request from SERQ 
 *  Stage 2, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
	

struct tx_table_s2_t0_k k       ;
struct phv_ p	;
struct tx_table_s2_t0_tls_rx_serq_d d	;
	
%%
    .param      tls_mac_serq_consume_process
        
tls_mac_rx_serq_process:
    CAPRI_SET_DEBUG_STAGE0_3(p.to_s6_debug_stage0_3_thread, CAPRI_MPU_STAGE_2, CAPRI_MPU_TABLE_0)
    CAPRI_CLEAR_TABLE0_VALID


    /* ENQ_DESC(*etlsp, enc, md->seqe.desc); */
    /* if (TAIL_DESC(cb,name) == NULL) { */
    seq		c1, d.qtail, r0
    /*    TAIL_DESC(cb, name) = desc; */
    bcf		[c1], no_dma_cmd_enc
    nop
    /*    TAIL_DESC(cb, name) = desc; */
    tblwr.c1	d.qtail, k.to_s2_idesc
    add         r1, r0, k.to_s2_idesc

    CAPRI_OPERAND_DEBUG(k.to_s2_idesc)
	
dma_cmd_mac_desc_entry_last:
    /* SET_DESC_ENTRY(TAIL_DESC(cb,name), MAX_ENTRIES_PER_DESC - 1, desc, 0, 0); */
	
    addi	r5, r0, TLS_PHV_DMA_COMMANDS_START
    add		r4, r5, r0
    phvwr	p.p4_txdma_intr_dma_cmd_ptr, r4

    addi	r5, r0, NIC_DESC_ENTRY_L_OFFSET
    add		r5, r5, d.qtail

    phvwr	p.idesc_next_addr, k.to_s2_idesc

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd0_dma_cmd, r5, idesc_next_addr, idesc_next_addr)

no_dma_cmd_enc:        
    /* etlsp->enc_nxt.desc = TAIL_DESC(*etlsp, enc); */
    phvwr       p.to_s5_cur_tls_data_len, d.cur_tls_data_len
    phvwr       p.to_s4_barco_hmac_key_desc_index, d.barco_hmac_key_desc_index

    tblwr.c1	d.qtail, k.to_s2_idesc
    tblwr.c1	d.qhead, k.to_s2_idesc

table_read_serq_consume:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS, tls_mac_serq_consume_process,
                k.tls_global_phv_qstate_addr, TLS_TCB_OFFSET,
                TABLE_SIZE_512_BITS)

tls_rx_serq_process_mac_done:
    nop.e
    nop


