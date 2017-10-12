#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct p4plus_app_k k;
struct phv_         p;

k = {
  l4_metadata_tcp_data_len = 100;
  tcp_option_one_sack_valid = 1;
  tcp_option_two_sack_valid = 1;
  control_metadata_qid = 0xbad;
  control_metadata_qtype = 0x5;
  ethernet_valid = 1;
  ipv4_valid = 1;
  tcp_valid = 1;
  tcp_srcPort = 0x64;
  tcp_dstPort = 0x8080;
  control_metadata_packet_len = 0x100;
  ipv4_ihl = 5;
  tcp_dataOffset = 5;
};

p = {
  ethernet_valid = 1;
  ipv4_valid = 1;
  tcp_valid = 1;
};
