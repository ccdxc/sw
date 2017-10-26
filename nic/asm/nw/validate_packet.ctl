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
};
