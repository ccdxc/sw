#include "ingress.h"
#include "INGRESS_p.h"
#include "apollo.h"

struct flow_info_k   k;
struct flow_info_d   d;
struct phv_     p;

k = {
    capri_p4_intrinsic_packet_len_sbit0_ebit5 = 1;
    capri_p4_intrinsic_packet_len_sbit6_ebit13 = 1;
};

d = {
  flow_info_d.drop = 0;
  flow_info_d.deny_packets = 20;
  flow_info_d.deny_bytes = 128;
  flow_info_d.permit_packets = 20;
  flow_info_d.permit_bytes = 128;

};

c1 = 0;
r1 = 0;

