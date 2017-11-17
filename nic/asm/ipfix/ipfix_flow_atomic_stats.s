#include "ipfix/asm_out/INGRESS_p.h"
#include "ipfix/asm_out/ingress.h"
#include "iris/asm_out/ingress.h"

struct ipfix_flow_atomic_stats_d d;
struct phv_                      p;

%%

ipfix_flow_atomic_stats:
    phvwr       p.ipfix_record_common_permit_bytes, \
                    d.{u.ipfix_flow_atomic_stats_d.permit_bytes}.dx
    phvwr       p.ipfix_record_common_permit_packets, \
                    d.{u.ipfix_flow_atomic_stats_d.permit_packets}.dx
    phvwr.e     p.ipfix_record_common_drop_bytes, \
                    d.{u.ipfix_flow_atomic_stats_d.drop_bytes}.dx
    phvwr       p.ipfix_record_common_drop_packets, \
                    d.{u.ipfix_flow_atomic_stats_d.drop_packets}.dx
