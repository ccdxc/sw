#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s4_t0_k k;
struct tx_table_s4_t0_esp_v4_tunnel_n2h_txdma1_update_cb_d d;
struct phv_ p;

%%
        .align
esp_v4_tunnel_n2h_txdma1_update_cb:
    phvwri p.app_header_table0_valid, 0
    add r1, r0, d.cb_cindex
    addi r1, r1, 1
    tblwr d.cb_cindex, r1
    nop
    tblwr d.rxdma_ring_cindex, r1
esp_ipv4_tunnel_n2h_txdma1_ipsec_ring_barco_doorbell:                                                                                                                                                                                             
    phvwri p.dma_cmd_incr_pindex_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri p.dma_cmd_incr_pindex_dma_cmd_addr, CAPRI_BARCO_MD_HENS_REG_GCM0_PRODUCER_IDX
    phvwri p.dma_cmd_incr_pindex_dma_cmd_phv_start_addr, IPSEC_BARCO_DOORBELL_OFFSET_START
    phvwri p.dma_cmd_incr_pindex_dma_cmd_phv_end_addr, IPSEC_BARCO_DOORBELL_OFFSET_END
    phvwri p.dma_cmd_incr_pindex_dma_cmd_eop, 1
    phvwri p.dma_cmd_incr_pindex_dma_cmd_wr_fence, 1

    nop.e
    nop
