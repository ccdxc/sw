#include "ipfix/asm_out/INGRESS_p.h"
#include "ipfix/asm_out/ingress.h"
#include "ipfix/alt_asm_out/INGRESS_ipfix_flow_stats_k.h"
#include "p4/asm_out/ingress.h"

struct ipfix_flow_stats_k_  k;
struct flow_stats_d         d;
struct phv_                 p;

%%

    .param     ipfix_create_record

ipfix_flow_stats:
    phvwr       p.ipfix_record_common_drop_vector, \
                    d.flow_stats_d.drop_reason
    phvwr       p.ipfix_record_common_last_seen_timestamp, \
                    d.flow_stats_d.last_seen_timestamp
    phvwr       p.ipfix_record_common_permit_bytes, \
                    d.flow_stats_d.permit_bytes
    phvwr       p.ipfix_record_common_permit_packets, \
                    d.flow_stats_d.permit_packets
    phvwr       p.ipfix_record_common_drop_bytes, \
                    d.flow_stats_d.drop_bytes
    phvwr       p.ipfix_record_common_drop_packets, \
                    d.flow_stats_d.drop_packets

    sub         r1, d.flow_stats_d.permit_packets, \
                    k.ipfix_exported_permit_stats1_pkts
    phvwr       p.ipfix_record_common_delta_permit_packets, r1
    sub         r1, d.flow_stats_d.permit_bytes, \
                    k.ipfix_exported_permit_stats1_byts
    phvwr       p.ipfix_record_common_delta_permit_bytes, r1
    sub         r1, d.flow_stats_d.drop_packets, \
                    k.ipfix_exported_drop_stats_pkts
    phvwr       p.ipfix_record_common_delta_drop_packets, r1
    sub         r1, d.flow_stats_d.drop_bytes, \
                    k.ipfix_exported_drop_stats_byts
    phvwr       p.ipfix_record_common_delta_drop_bytes, r1

    phvwr       p.{ipfix_exported_permit_stats2_pkts, \
                    ipfix_exported_permit_stats2_byts}, \
                    d.{flow_stats_d.permit_packets,flow_stats_d.permit_bytes}
    phvwr       p.{ipfix_exported_drop_stats_pkts, \
                    ipfix_exported_drop_stats_byts}, \
                    d.{flow_stats_d.drop_packets,flow_stats_d.drop_bytes}

    // table 0 : lookup qstate address
    phvwr       p.common_te0_phv_table_addr, k.ipfix_metadata_qstate_addr
    phvwri      p.common_te0_phv_table_pc, ipfix_create_record[33:6]
    phvwr       p.common_te0_phv_table_raw_table_size, 6
    phvwr       p.common_te0_phv_table_lock_en, 1

    // enable table 0 in next stage
    phvwr.e     p.{app_header_table0_valid...app_header_table3_valid}, 0x8
    nop
