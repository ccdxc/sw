#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct rx_table_s4_t1_k k;
struct rx_table_s4_t1_ipsec_rxdma_stats_update_d d;
struct phv_ p;

%%
        .align
esp_ipv4_tunnel_n2h_rxdma_ipsec_update_rx_stats:
    phvwri p.app_header_table2_valid, 0
    tbladd d.n2h_rx_pkts, 1
    add r1, k.ipsec_global_packet_length, 4 
    tbladd d.n2h_rx_bytes, r1 

    phvwri p.dma_cmd_phv2mem_ipsec_int_dma_cmd_phv_start_addr, IPSEC_N2H_INT_START_OFFSET
    phvwri p.dma_cmd_phv2mem_ipsec_int_dma_cmd_phv_end_addr, IPSEC_N2H_INT_END_OFFSET
    phvwri p.dma_cmd_in_desc_aol_dma_cmd_phv_start_addr, IPSEC_IN_DESC_AOL_START
    phvwri p.dma_cmd_in_desc_aol_dma_cmd_phv_end_addr, IPSEC_IN_DESC_AOL_END
    phvwri p.dma_cmd_out_desc_aol_dma_cmd_phv_start_addr, IPSEC_OUT_DESC_AOL_START
    phvwri p.dma_cmd_out_desc_aol_dma_cmd_phv_end_addr, IPSEC_OUT_DESC_AOL_END

    nop.e
    nop 
