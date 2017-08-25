#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_defines.h"

struct rx_table_s2_t1_k k;
struct rx_table_s2_t1_rx_table_s2_t1_cfg_action_d d;
struct phv_ p;

%%
        .align

esp_ipv4_tunnel_n2h_update_output_desc_aol:
    phvwri p.p42p4plus_hdr_table1_valid, 0

    phvwr p.barco_desc_out_A0_addr, k.t1_s2s_out_page_addr 
    // get the correct way of giving it as a single 14 bit field
    phvwri p.barco_desc_out_L0, 0 
    phvwri p.barco_desc_out_O0, 0

    phvwri p.barco_desc_out_A1_addr, 0
    phvwri p.barco_desc_out_L1, 0
    phvwri p.barco_desc_out_O1, 0

    phvwri p.barco_desc_out_A2_addr, 0
    phvwri p.barco_desc_out_L2, 0
    phvwri p.barco_desc_out_O2, 0

    phvwri p.barco_desc_out_NextAddr, 0
    phvwri p.barco_desc_out_Reserved, 0

