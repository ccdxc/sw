#include "ipfix/asm_out/INGRESS_p.h"
#include "ipfix/asm_out/ingress.h"
#include "ipfix/alt_asm_out/INGRESS_ipfix_update_exported_flow_stats_k.h"
#include "p4/asm_out/ingress.h"

struct ipfix_update_exported_flow_stats_k_  k;
struct ipfix_update_exported_flow_stats_d   d;
struct phv_                                 p;

%%

ipfix_update_exported_flow_stats:
    tblwr       d.u.ipfix_update_exported_flow_stats_d.permit_packets, \
                    k.ipfix_exported_permit_stats2_pkts
    tblwr       d.u.ipfix_update_exported_flow_stats_d.permit_bytes, \
                    k.ipfix_exported_permit_stats2_byts
    tblwr       d.u.ipfix_update_exported_flow_stats_d.drop_packets, \
                    k.ipfix_exported_drop_stats_pkts
    tblwr.e.f   d.u.ipfix_update_exported_flow_stats_d.drop_bytes, \
                    k.ipfix_exported_drop_stats_byts
    phvwr.f     p.{app_header_table0_valid...app_header_table3_valid}, 0
