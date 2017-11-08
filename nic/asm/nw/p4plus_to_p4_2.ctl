#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct p4plus_to_p4_2_k k;
struct phv_             p;

k = {
  p4plus_to_p4_flags = (P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN | P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN);
  capri_p4_intrinsic_packet_len_sbit0_ebit5 = 1;
  capri_p4_intrinsic_packet_len_sbit6_ebit13 = 0x28;
  ipv4_ihl = 5;
  ipv4_valid = 1;
};
