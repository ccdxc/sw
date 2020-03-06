#include "ipfix/asm_out/INGRESS_p.h"
#include "ipfix/asm_out/ingress.h"
#include "p4/asm_out/ingress.h"

struct ipfix_read_exported_flow_stats_k k;
struct ipfix_read_exported_flow_stats_d d;
struct phv_                             p;

%%

ipfix_read_exported_flow_stats:
    phvwr       p.ipfix_t0_metadata_exported_stats_addr, \
                    k.common_te1_phv_table_addr
    phvwr.e     p.{ipfix_exported_permit_stats1_pkts, \
                    ipfix_exported_permit_stats1_byts}, \
                    d.{u.ipfix_read_exported_flow_stats_d.permit_packets, \
                    u.ipfix_read_exported_flow_stats_d.permit_bytes}
    phvwr.f     p.{ipfix_exported_drop_stats_pkts, \
                    ipfix_exported_drop_stats_byts}, \
                    d.{u.ipfix_read_exported_flow_stats_d.drop_packets, \
                    u.ipfix_read_exported_flow_stats_d.drop_bytes}
