#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct ingress_key4_k k;
struct ingress_key4_d d;
struct phv_ p;

%%

ingress_key4:
    nop.!c1.e
    seq         c1, d.ingress_key4_d.match_fields[MATCH_TRANSPORT_SRC_PORT_1_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_l4_sport_1, k.l4_metadata_l4_sport_1
    seq         c1, d.ingress_key4_d.match_fields[MATCH_TRANSPORT_DST_PORT_1_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_l4_dport_1, k.l4_metadata_l4_dport_1
    seq         c1, d.ingress_key4_d.match_fields[MATCH_TCP_FLAGS_1_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_tcp_flags_1, k.tcp_1_flags
    seq         c1, d.ingress_key4_d.match_fields[MATCH_ICMP_TYPE_1_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_l4_sport_1, k.icmp_1_icmp_type
    seq         c1, d.ingress_key4_d.match_fields[MATCH_ICMP_CODE_1_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_l4_dport_1, k.icmp_1_icmp_code
    seq         c1, d.ingress_key4_d.match_fields[MATCH_TENANT_ID_1_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_tenant_id_1, k.tunnel_metadata_tunnel_vni_1
    seq         c1, d.ingress_key4_d.match_fields[MATCH_GRE_PROTO_1_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_gre_proto_1, k.gre_1_proto

    seq         c1, d.ingress_key4_d.match_fields[MATCH_TRANSPORT_SRC_PORT_2_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_l4_sport_2, k.l4_metadata_l4_sport_2

    seq         c1, d.ingress_key4_d.match_fields[MATCH_TRANSPORT_DST_PORT_2_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_l4_dport_2, k.l4_metadata_l4_dport_2
    seq         c1, d.ingress_key4_d.match_fields[MATCH_TCP_FLAGS_2_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_tcp_flags_2, k.tcp_2_flags
    seq         c1, d.ingress_key4_d.match_fields[MATCH_ICMP_TYPE_2_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_l4_sport_2, k.icmp_2_icmp_type
    seq         c1, d.ingress_key4_d.match_fields[MATCH_ICMP_CODE_2_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_l4_dport_2, k.icmp_2_icmp_code
    seq         c1, d.ingress_key4_d.match_fields[MATCH_TENANT_ID_2_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_tenant_id_2, k.tunnel_metadata_tunnel_vni_2
    seq         c1, d.ingress_key4_d.match_fields[MATCH_GRE_PROTO_2_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_gre_proto_2, k.gre_2_proto

    seq         c1, d.ingress_key4_d.match_fields[MATCH_TRANSPORT_SRC_PORT_3_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_l4_sport_3, k.l4_metadata_l4_sport_3
    seq         c1, d.ingress_key4_d.match_fields[MATCH_TRANSPORT_DST_PORT_3_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_l4_dport_3, k.l4_metadata_l4_dport_3
    seq         c1, d.ingress_key4_d.match_fields[MATCH_TCP_FLAGS_3_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_tcp_flags_3, k.tcp_3_flags
    seq         c1, d.ingress_key4_d.match_fields[MATCH_ICMP_TYPE_3_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_l4_sport_3, k.icmp_3_icmp_type
    seq         c1, d.ingress_key4_d.match_fields[MATCH_ICMP_CODE_3_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_l4_dport_3, k.icmp_3_icmp_code
    seq         c1, d.ingress_key4_d.match_fields[MATCH_TENANT_ID_3_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_tenant_id_3, k.tunnel_metadata_tunnel_vni_3
    seq.e       c1, d.ingress_key4_d.match_fields[MATCH_GRE_PROTO_3_BIT_POS], 1
    phvwr.c1    p.flow_lkp_metadata_gre_proto_3, k.gre_3_proto
