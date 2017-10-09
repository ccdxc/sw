#include "ipfix/asm_out/INGRESS_p.h"
#include "ipfix/asm_out/ingress.h"
#include "iris/asm_out/ingress.h"

struct ipfix_flow_stats_k k;
struct flow_stats_d       d;
struct phv_               p;

%%

    .param     ipfix_create_record

ipfix_flow_stats:
    phvwr       p.ipfix_record_common_drop_vector, \
                    d.flow_stats_d.drop_reason
    phvwr       p.ipfix_record_common_last_seen_timestamp, \
                    d.flow_stats_d.last_seen_timestamp
    phvwr       p.ipfix_record_common_delta_permit_bytes, \
                    d.flow_stats_d.permit_bytes
    phvwr       p.ipfix_record_common_delta_permit_packets, \
                    d.flow_stats_d.permit_packets
    phvwr       p.ipfix_record_common_delta_drop_bytes, \
                    d.flow_stats_d.drop_bytes
    phvwr       p.ipfix_record_common_delta_drop_packets, \
                    d.flow_stats_d.drop_packets

    // table 0 : lookup qstate address
    phvwr       p.common_te0_phv_table_addr, \
                    k.{ipfix_metadata_qstate_addr_sbit0_ebit6, \
                       ipfix_metadata_qstate_addr_sbit7_ebit30, \
                       ipfix_metadata_qstate_addr_sbit31_ebit33}
    phvwri      p.common_te0_phv_table_pc, ipfix_create_record[33:6]
    phvwr       p.common_te0_phv_table_raw_table_size, 6
    phvwr       p.common_te0_phv_table_lock_en, 0

    phvwr       p.app_header_table0_valid, 1
    phvwr       p.app_header_table1_valid, 0
    phvwr       p.app_header_table2_valid, 0
    phvwr.e     p.app_header_table3_valid, 0
    nop
