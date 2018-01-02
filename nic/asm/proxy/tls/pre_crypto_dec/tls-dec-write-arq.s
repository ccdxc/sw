/*
 *    Implements the Write ARQ stage of the TLS TxDMA P4+ pre decrypt pipeline
 */
#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct tx_table_s6_t1_k     k;
struct phv_                 p;
struct tx_table_s6_t1_tls_write_arq_d     d;


%%
    .align    
tls_dec_write_arq:
    CAPRI_CLEAR_TABLE1_VALID


    /* if (k.write_serq) is set in a previous stage , trigger writes to serq slot */
    sne         c1, k.tls_global_phv_write_arq, r0
    bcf         [!c1], tls_dec_write_arq_done
    nop

  	addi		r5, r0, TLS_PHV_DMA_COMMANDS_START
	phvwr		p.p4_txdma_intr_dma_cmd_ptr, r5


    smeqb       c1, k.s5_s6_t1_s2s_debug_dol, TLS_DDOL_LEAVE_IN_ARQ, TLS_DDOL_LEAVE_IN_ARQ

dma_cmd_cpu_hdr:
    phvwri      p.cpu_hdr1_src_lif,0

    addi        r3, r0, LIF_TLS
    phvwr       p.cpu_hdr1_lif, r3.hx

    add         r3, r0, k.tls_global_phv_fid
    phvwr       p.cpu_hdr1_qid, r3.wx

    phvwri      p.cpu_hdr1_lkp_vrf, 0
    phvwri      p.cpu_hdr1_lkp_dir, 0
    phvwri      p.cpu_hdr1_lkp_inst, 0
    phvwri      p.cpu_hdr1_lkp_type, 0

    phvwri      p.cpu_hdr1_l2_offset, 0xFFFF
    phvwri      p.cpu_hdr1_l3_offset_1, 0xFF
    phvwri      p.cpu_hdr2_l3_offset_2, 0xFF
    phvwri      p.cpu_hdr2_l4_offset, 0xFFFF
    phvwri      p.cpu_hdr2_payload_offset, 0
        
    add         r4, k.to_s6_opage, k.to_s6_next_tls_hdr_offset
    subi        r3, r4, NIC_CPU_HDR_SIZE_BYTES

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd0_dma_cmd, r3, cpu_hdr1_src_lif, cpu_hdr2_tcp_window)

dma_cmd_descr:    
    /* Set the DMA_WRITE CMD for descr */
    add         r5, k.to_s6_idesc, r0
    addi        r1, r5, PKT_DESC_AOL_OFFSET


    phvwr       p.idesc_A0, k.{to_s6_opage}.dx
    add         r4, k.to_s6_next_tls_hdr_offset, r0
    subi        r3, r4, NIC_CPU_HDR_SIZE_BYTES
    phvwr       p.idesc_O0, r3.wx
    add         r3, k.to_s6_cur_tls_data_len, NIC_CPU_HDR_SIZE_BYTES
    phvwr       p.idesc_L0, r3.wx
    phvwri      p.idesc_A1, 0
    phvwri      p.idesc_O1, 0
    phvwri      p.idesc_L1, 0
    phvwri      p.idesc_A2, 0
    phvwri      p.idesc_O2, 0
    phvwri      p.idesc_L2, 0

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd1_dma_cmd, r1, idesc_A0, idesc_next_pkt)    

dma_cmd_arq_slot:
#ifdef DO_NOT_USE_CPU_SEM
    CPU_ARQ_PIDX_READ_INC(r6, 0, struct tx_table_s6_t1_tls_write_arq_d, pi_0, r4, r5)
#else
    add         r6, r0, d.{arq_pindex}.wx 
#endif
    phvwr       p.ring_entry_descr_addr, k.to_s6_idesc

    CPU_RX_ENQUEUE(r5,
                   k.to_s6_idesc,
                   r6,
                   k.s5_s6_t1_s2s_arq_base,
                   ring_entry_descr_addr,
                   dma_cmd2_dma_cmd, 
                   1, 
                   1, 
                   c1)  

    
tls_dec_write_arq_done:
    nop.e
    nop

