#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct validate_packet_k k;
struct phv_              p;

k = {
  ethernet_srcAddr = 0xeedd00aabbcc;
  ethernet_dstAddr = 0x00aabbccddff;
  ipv4_valid = 1;
  ipv4_version = 4;
  control_metadata_uplink = 1;
  capri_p4_intrinsic_parser_err = 1;
  capri_p4_intrinsic_len_err = 1;
  capri_p4_intrinsic_crc_err = 1;
  p4plus_to_p4_flags = P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN;
};
