#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s5_t0_ipsec_release_resources_d d;
struct tx_table_s5_t0_k k;
struct phv_ p;

%%
    .param IPSEC_RNMPR_TABLE_BASE
    .param IPSEC_TNMPR_TABLE_BASE
    .align
esp_ipv4_tunnel_h2n_txdma2_ipsec_free_resources:
    and r2, d.sem_cindex, IPSEC_DESC_RING_INDEX_MASK
    tbladd.f d.sem_cindex, 1
    phvwri p.icv_header_dma_cmd_size, IPSEC_DEFAULT_ICV_SIZE
    
    sll r3, r2, 3

    addui r4, r3, hiword(IPSEC_RNMPR_TABLE_BASE)
    addi r4, r4, loword(IPSEC_RNMPR_TABLE_BASE)
    sub r5, k.txdma2_global_in_desc_addr, 64
    phvwr p.txdma2_global_in_desc_addr, r5
    CAPRI_DMA_CMD_PHV2MEM_SETUP(rnmdr_dma_cmd, r4, txdma2_global_in_desc_addr, txdma2_global_in_desc_addr)

    addui r4, r3, hiword(IPSEC_TNMPR_TABLE_BASE)
    addi r4, r4, loword(IPSEC_TNMPR_TABLE_BASE)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(tnmdr_dma_cmd, r4, t1_s2s_out_desc_addr, t1_s2s_out_desc_addr)

    add r2, d.sem_cindex, IPSEC_DESC_RING_SIZE
    phvwr p.ipsec_to_stage4_sem_cindex, r2.wx 
    addi r1, r0, CAPRI_SEM_IPSEC_RNMDR_ALLOC_CI_RAW_ADDR 
    CAPRI_DMA_CMD_PHV2MEM_SETUP(sem_cindex_dma_cmd, r1, ipsec_to_stage4_sem_cindex, ipsec_to_stage4_sem_cindex)
    CAPRI_DMA_CMD_STOP_FENCE(sem_cindex_dma_cmd)
    phvwri p.sem_cindex_dma_cmd_eop, 1

    phvwri.e p.{app_header_table0_valid...app_header_table3_valid}, 0
    nop
