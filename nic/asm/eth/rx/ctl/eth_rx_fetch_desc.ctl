
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_ p;
struct common_p4plus_stage0_app_header_table_k k;
struct common_p4plus_stage0_app_header_table_eth_rx_fetch_desc_d d;

p = {
    eth_rx_global_packet_len = 0x0;
    app_header_table0_valid = 0x0;
    app_header_table1_valid = 0x0;
    app_header_table2_valid = 0x0;
    app_header_table3_valid = 0x0;
    common_te0_phv_table_lock_en = 0x0;
    common_te0_phv_table_pc = 0x0;
    common_te0_phv_table_addr = 0x0;
    common_te0_phv_table_raw_table_size = 0x0;
    eth_rx_to_s1_cq_desc_addr = 0x0;
    eth_rx_cq_desc_comp_index = 0x0;
    eth_rx_cq_desc_csum = 0x0;
    eth_rx_cq_desc_csum_level = 0x0;
    eth_rx_cq_desc_len_lo = 0x0;
    eth_rx_cq_desc_len_hi = 0x0;
    eth_rx_cq_desc_V = 0x0;
    eth_rx_cq_desc_vlan_tci = 0x0;
    p4_intr_global_drop = 0x0;
};


k = {
    p4_to_p4plus_packet_len = 0xfeed;
    p4_rxdma_intr_qid = 0xbababa;
    p4_to_p4plus_packet_len = 0x3fff;
    p4_to_p4plus_l4_checksum = 0xbeef;
    p4_to_p4plus_vlan_vid_sbit0_ebit3 = 0xf;
    p4_to_p4plus_vlan_vid_sbit4_ebit11 = 0xff;
    p4_to_p4plus_vlan_dei = 0x1;
    p4_to_p4plus_vlan_pcp = 0x7;
};


d = {
    enable = 0x1;
    c_index0 = 0x0;
    p_index0 = 0x1;
    c_index1 = 0x0;
    p_index1 = 0x0;
    ring_base = 0x8000000000000000;
    ring_size = 0xa;
    cq_ring_base = 0x8000000000010000;
    intr_assert_addr = 0xdeadbeef;
    color = 1;
};
