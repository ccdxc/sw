#include "ipfix/asm_out/INGRESS_p.h"
#include "ipfix/asm_out/ingress.h"
#include "p4/asm_out/ingress.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"

struct ipfix_flow_hash_k k;
struct flow_hash_d       d;
struct phv_              p;

%%

    .param      p4_flow_info_base
    .param      ipfix_flow_info

ipfix_flow_hash:
    bbeq        k.ipfix_metadata_scan_complete, TRUE, ipfix_flow_hash_complete

    seq         c1, d.flow_hash_info_d.entry_valid, TRUE
    seq         c2, d.flow_hash_info_d.export_en, 0
    bcf         [!c1|c2], ipfix_flow_hash_skip_entry
    phvwr       p.ipfix_metadata_flow_index, d.flow_hash_info_d.flow_index
    phvwr       p.ipfix_metadata_flow_type, d.flow_hash_info_d.flow_lkp_metadata_lkp_type
    phvwr       p.ipfix_metadata_export_en, d.flow_hash_info_d.export_en

    seq         c1, d.flow_hash_info_d.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV4
    bcf         [c1], ipfix_flow_hash_ipv4_entry
    seq         c1, d.flow_hash_info_d.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV6
    bcf         [c1], ipfix_flow_hash_ipv6_entry
    seq         c1, d.flow_hash_info_d.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_MAC
    bcf         [!c1], ipfix_flow_hash_skip_entry
    phvwr.c1    p.ipfix_record_nonip_mac_sa, d.flow_hash_info_d.flow_lkp_metadata_lkp_src
    phvwr       p.ipfix_record_nonip_mac_da, d.flow_hash_info_d.flow_lkp_metadata_lkp_dst
    phvwr       p.ipfix_record_nonip_ether_type, d.flow_hash_info_d.flow_lkp_metadata_lkp_sport
    b           ipfix_flow_hash_common
    phvwr       p.ipfix_record_nonip_vrf, d.flow_hash_info_d.flow_lkp_metadata_lkp_vrf

ipfix_flow_hash_ipv4_entry:
    phvwr       p.ipfix_record_ipv4_vrf, d.flow_hash_info_d.flow_lkp_metadata_lkp_vrf
    phvwr       p.ipfix_record_ipv4_ip_sa, d.flow_hash_info_d.flow_lkp_metadata_lkp_src
    phvwr       p.ipfix_record_ipv4_ip_da, d.flow_hash_info_d.flow_lkp_metadata_lkp_dst
    phvwr       p.ipfix_record_ipv4_proto, d.flow_hash_info_d.flow_lkp_metadata_lkp_proto
    seq         c1, d.flow_hash_info_d.flow_lkp_metadata_lkp_proto, IP_PROTO_ICMP
    phvwr.c1    p.ipfix_record_ip_icmp_type_code, d.flow_hash_info_d.flow_lkp_metadata_lkp_dport
    phvwr.!c1   p.ipfix_record_ipv4_dport, d.flow_hash_info_d.flow_lkp_metadata_lkp_dport
    b           ipfix_flow_hash_common
    phvwr       p.ipfix_record_ipv4_sport, d.flow_hash_info_d.flow_lkp_metadata_lkp_sport

ipfix_flow_hash_ipv6_entry:
    phvwr       p.ipfix_record_ipv6_vrf, d.flow_hash_info_d.flow_lkp_metadata_lkp_vrf
    phvwr       p.ipfix_record_ipv6_ip_sa, d.flow_hash_info_d.flow_lkp_metadata_lkp_src
    phvwr       p.ipfix_record_ipv6_ip_da, d.flow_hash_info_d.flow_lkp_metadata_lkp_dst
    phvwr       p.ipfix_record_ipv6_proto, d.flow_hash_info_d.flow_lkp_metadata_lkp_proto
    phvwr       p.ipfix_record_ipv6_sport, d.flow_hash_info_d.flow_lkp_metadata_lkp_sport
    seq         c1, d.flow_hash_info_d.flow_lkp_metadata_lkp_proto, IP_PROTO_ICMPV6
    phvwr.c1    p.ipfix_record_ip_icmp_type_code, d.flow_hash_info_d.flow_lkp_metadata_lkp_dport
    phvwr.!c1   p.ipfix_record_ipv6_dport, d.flow_hash_info_d.flow_lkp_metadata_lkp_dport

ipfix_flow_hash_common:
    phvwr       p.ipfix_record_common_flow_id, d.flow_hash_info_d.flow_index

ipfix_flow_hash_complete:
    // table 0 : lookup flow_info
    addi        r1, r0, loword(p4_flow_info_base)
    addui       r1, r1, hiword(p4_flow_info_base)
    add         r1, r1, d.flow_hash_info_d.flow_index, 5
    phvwr       p.common_te0_phv_table_addr, r1
    phvwri      p.common_te0_phv_table_pc, ipfix_flow_info[33:6]
    phvwr       p.common_te0_phv_table_raw_table_size, 5
    phvwr       p.common_te0_phv_table_lock_en, 0

    // enable table 0 in next stage
    phvwr.e     p.{app_header_table0_valid...app_header_table3_valid}, 0x8
    nop

ipfix_flow_hash_skip_entry:
    phvwr.e     p.{app_header_table0_valid...app_header_table3_valid}, 0
    phvwr       p.p4_intr_global_drop, 1
