#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"

struct rx_table_s2_t3_k k;
struct rx_table_s2_t3_allocate_output_page_index_d d;
struct phv_ p;

%%
        .align

esp_ipv4_tunnel_h2n_allocate_output_page_index:
    phvwr p.t1_s2s_out_page_addr, d.out_page_index 
    phvwr p.ipsec_int_header_out_page, d.out_page_index 
    phvwr p.barco_desc_out_A0_addr, d.{out_page_index}.dx 
    //phvwri p.barco_desc_out_O0, 0 
    //phvwri p.barco_desc_out_L0, 0 

    phvwri p.dma_cmd_in_desc_aol_dma_cmd_phv_start_addr, IPSEC_IN_DESC_AOL_START
    phvwri p.dma_cmd_in_desc_aol_dma_cmd_phv_end_addr, IPSEC_IN_DESC_AOL_END

    nop.e 
    nop
