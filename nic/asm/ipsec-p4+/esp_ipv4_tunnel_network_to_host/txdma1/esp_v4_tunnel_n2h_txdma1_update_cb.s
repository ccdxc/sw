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
esp_ipv4_tunnel_n2h_txdma1_ipsec_ring_barco_doorbell:                                                                                                                                                                                             

    CAPRI_DMA_CMD_PHV2MEM_SETUP_I(dma_cmd_incr_pindex_dma_cmd, CAPRI_BARCO_MD_HENS_REG_GCM0_PRODUCER_IDX, barco_dbell_pi, barco_dbell_pi)
    CAPRI_DMA_CMD_STOP_FENCE(dma_cmd_incr_pindex_dma_cmd)
    phvwri p.dma_cmd_incr_pindex_dma_cmd_eop, 1

    nop.e
    nop
