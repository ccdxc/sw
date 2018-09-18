#include "tyche.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct rx_key3_k k;
struct rx_key3_d d;
struct phv_ p;

%%

rx_key3:
    sub             r1, k.{capri_p4_intrinsic_frame_size_sbit0_ebit5,\
                        capri_p4_intrinsic_frame_size_sbit6_ebit13}, \
                        CAPRI_GLOBAL_INTRINSIC_HDR_SZ
    phvwr           p.capri_p4_intrinsic_packet_len, r1
    nop.!c1.e
    phvwr           p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq

rx_key_start_1:
    crestore        [c7-c1], d.rx_key3_d.match_fields[6:0], 0x7F
    add             r7, r0, r0
    bcf             [!c1&!c2&!c3&!c4&!c5&!c6&!c7], rx_key_start_2
    or.c1           r7, r7, k.l4_metadata_l4_sport_1, 16
    or.c2           r7, r7, k.l4_metadata_l4_dport_1
    phvwr.c3        p.flow_lkp_metadata_tcp_flags_1, k.tcp_1_flags
    or.c4           r7, r7, k.icmp_1_icmp_type, 16
    or.c5           r7, r7, k.icmp_1_icmp_code
    phvwr.c6        p.flow_lkp_metadata_tenant_id_1, k.tunnel_metadata_tunnel_vni_1
    phvwr           p.{flow_lkp_metadata_l4_sport_1,flow_lkp_metadata_l4_dport_1}, r7

rx_key_start_2:
    crestore        [c5-c1], d.rx_key3_d.match_fields[11:7], 0x1F
    bcf             [!c1&!c2&!c3&!c4&!c5], rx_key_start_3
    phvwr.c1        p.flow_lkp_metadata_l4_sport_2, k.l4_metadata_l4_sport_2
    phvwr.c2        p.flow_lkp_metadata_l4_dport_2, k.l4_metadata_l4_dport_2
    phvwr.c3        p.flow_lkp_metadata_tcp_flags_2, k.tcp_2_flags
    phvwr.c4        p.flow_lkp_metadata_l4_sport_2, k.icmp_2_icmp_type
    phvwr.c5        p.flow_lkp_metadata_l4_dport_2, k.icmp_2_icmp_code

rx_key_start_3:
    seq             c1, d.rx_key3_d.match_fields[14:12], r0
    nop.c1.e
    crestore        [c3-c1], d.rx_key3_d.match_fields[14:12], 0x7
    phvwr.c1        p.flow_lkp_metadata_custom_field_1, 0
    phvwr.c2        p.flow_lkp_metadata_custom_field_2, 0
    phvwr.c3        p.flow_lkp_metadata_custom_field_3, 0
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rx_key3_error:
    nop.e
    nop
