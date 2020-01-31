#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s5_t0_ipsec_release_resources_d d;
struct tx_table_s5_t0_k k;
struct phv_ p;

%%
    .param IPSEC_RNMPR_TABLE_BASE
    .param IPSEC_TNMPR_TABLE_BASE
    .param IPSEC_PAGE_ADDR_RX
    .param IPSEC_PAGE_ADDR_TX
    .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    .param IPSEC_ENC_NMDR_CI
    .param TLS_PROXY_BARCO_GCM0_PI_HBM_TABLE_BASE
    .param esp_ipv4_tunnel_h2n_barco_global_ci
    .align
esp_ipv4_tunnel_h2n_txdma2_ipsec_free_resources:
    and r2, d.sem_cindex, IPSEC_DESC_RING_INDEX_MASK
    tbladd.f d.sem_cindex, 1
    phvwri p.icv_header_dma_cmd_size, IPSEC_DEFAULT_ICV_SIZE
    
    sll r3, r2, 3
 
    addui r4, r3, hiword(IPSEC_RNMPR_TABLE_BASE)
    addi r4, r4, loword(IPSEC_RNMPR_TABLE_BASE)
    sub r5, k.txdma2_global_in_desc_addr, IPSEC_SCRATCH_OFFSET 
    phvwr p.txdma2_global_in_desc_addr, r5
    bgti r5, IPSEC_PAGE_ADDR_RX, h2n_txdma2_bad_indesc_free
    nop
    CAPRI_DMA_CMD_PHV2MEM_SETUP(rnmdr_dma_cmd, r4, txdma2_global_in_desc_addr, txdma2_global_in_desc_addr)

    addui r4, r3, hiword(IPSEC_TNMPR_TABLE_BASE)
    addi r4, r4, loword(IPSEC_TNMPR_TABLE_BASE)
    add r5, r0, k.ipsec_to_stage5_out_desc_addr
    bgti r5, IPSEC_PAGE_ADDR_TX, h2n_txdma2_bad_outdesc_free
    nop
    CAPRI_DMA_CMD_PHV2MEM_SETUP(tnmdr_dma_cmd, r4, t1_s2s_out_desc_addr, t1_s2s_out_desc_addr)

    add r2, d.sem_cindex, IPSEC_DESC_RING_SIZE
    phvwr p.ipsec_to_stage2_sem_cindex, r2.wx 
    addui r1, r0, hiword(IPSEC_ENC_NMDR_CI)
    addi r1, r1, loword(IPSEC_ENC_NMDR_CI)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(sem_cindex_dma_cmd, r1, ipsec_to_stage2_sem_cindex, ipsec_to_stage2_sem_cindex)
    CAPRI_DMA_CMD_STOP_FENCE(sem_cindex_dma_cmd)

    //addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    //CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_TXDMA2_SEM_FREE, 1)

    addui r5, r0, hiword(TLS_PROXY_BARCO_GCM0_PI_HBM_TABLE_BASE)
    addi r5, r5, loword(TLS_PROXY_BARCO_GCM0_PI_HBM_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, esp_ipv4_tunnel_h2n_barco_global_ci, r5, TABLE_SIZE_32_BITS)
    nop.e
    nop

h2n_txdma2_bad_indesc_free:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_TXDMA2_BAD_INDESC_FREE, 1)
    phvwri p.p4_intr_global_drop, 1
    phvwri.e p.{app_header_table0_valid...app_header_table3_valid}, 0
    nop


h2n_txdma2_bad_outdesc_free:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_TXDMA2_BAD_OUTDESC_FREE, 1)
    phvwri p.p4_intr_global_drop, 1
    phvwri.e p.{app_header_table0_valid...app_header_table3_valid}, 0
    nop

