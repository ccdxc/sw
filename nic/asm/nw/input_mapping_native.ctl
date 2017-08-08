#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct input_mapping_native_k k;
struct phv_                   p;

k = {
  ethernet_srcAddr_sbit0_ebit31 = 0x00aabbcc;
  ethernet_srcAddr_sbit32_ebit47 = 0xddee;
  ethernet_dstAddr = 0x00aabbccddff;
  ipv4_version = 4;
  ipv4_srcAddr = 192.168.1.1;
  ipv4_ttl = 64;
  ipv4_dstAddr = 192.168.1.2;
};
