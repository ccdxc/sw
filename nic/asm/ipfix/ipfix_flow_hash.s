#include "ipfix/asm_out/INGRESS_p.h"
#include "iris/asm_out/ingress.h"
#include "../../p4/nw/include/defines.h"

struct flow_hash_d d;
struct phv_        p;

%%

    .param      ipfix_flow_info

ipfix_flow_hash:
    seq         c1, d.flow_hash_info_d.entry_valid, TRUE
    seq         c2, d.flow_hash_info_d.export_en, FALSE
    bcf         [!c1|!c2], ipfix_flow_hash_skip_entry
    phvwr       p.ipfix_metadata_flow_index, d.flow_hash_info_d.flow_index

    seq         c1, d.flow_hash_info_d.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV4
    bcf         [c1], ipfix_flow_hash_ipv4_entry
    seq         c1, d.flow_hash_info_d.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV6
    bcf         [c1], ipfix_flow_hash_ipv6_entry
    seq         c1, d.flow_hash_info_d.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_MAC
    bcf         [!c1], ipfix_flow_hash_skip_entry
    phvwr.c1    p.ipfix_record_nonip_mac_sa, d.flow_hash_info_d.flow_lkp_metadata_lkp_src
    phvwr       p.ipfix_record_nonip_mac_da, d.flow_hash_info_d.flow_lkp_metadata_lkp_dst
    b           ipfix_flow_hash_common
    phvwr       p.ipfix_record_nonip_ether_type, d.flow_hash_info_d.flow_lkp_metadata_lkp_sport

ipfix_flow_hash_ipv4_entry:
    phvwr       p.ipfix_record_ipv4_ip_sa, d.flow_hash_info_d.flow_lkp_metadata_lkp_src
    b           ipfix_flow_hash_ip_common
    phvwr       p.ipfix_record_ipv4_ip_da, d.flow_hash_info_d.flow_lkp_metadata_lkp_dst

ipfix_flow_hash_ipv6_entry:
    phvwr       p.ipfix_record_ipv6_ip_sa, d.flow_hash_info_d.flow_lkp_metadata_lkp_src
    phvwr       p.ipfix_record_ipv6_ip_da, d.flow_hash_info_d.flow_lkp_metadata_lkp_dst

ipfix_flow_hash_ip_common:
    phvwr       p.ipfix_record_ip_proto, d.flow_hash_info_d.flow_lkp_metadata_lkp_proto
    phvwr       p.ipfix_record_ip_sport, d.flow_hash_info_d.flow_lkp_metadata_lkp_sport
    phvwr       p.ipfix_record_ip_dport, d.flow_hash_info_d.flow_lkp_metadata_lkp_dport

ipfix_flow_hash_common:
    phvwr       p.ipfix_record_common_vrf, d.flow_hash_info_d.flow_lkp_metadata_lkp_vrf
    phvwr       p.ipfix_record_common_flow_id, d.flow_hash_info_d.flow_index

    // table 0 : lookup flow_info
    addi        r1, r0, 0x98100000
    add         r1, r1, d.flow_hash_info_d.flow_index, 6
    phvwr       p.common_te0_phv_table_addr, r1
    phvwri      p.common_te0_phv_table_pc, ipfix_flow_info[33:6]
    phvwr       p.common_te0_phv_table_raw_table_size, 6
    phvwr       p.common_te0_phv_table_lock_en, 0

    phvwr       p.app_header_table0_valid, 1
    phvwr       p.app_header_table1_valid, 0
    phvwr       p.app_header_table2_valid, 0
    phvwr.e     p.app_header_table3_valid, 0
    nop

ipfix_flow_hash_skip_entry:
    phvwr       p.app_header_table0_valid, 0
    phvwr       p.app_header_table1_valid, 0
    phvwr       p.app_header_table2_valid, 0
    phvwr.e     p.app_header_table3_valid, 0
    nop
