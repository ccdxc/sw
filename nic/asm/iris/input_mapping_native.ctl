#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/iris/include/defines.h"

struct input_mapping_native_k k;
struct phv_                   p;

k = {
  ethernet_srcAddr= 0xaabbcceedd00;
  ethernet_dstAddr = 0x00aabbccddff;
  ipv4_srcAddr = 192.168.1.1;
  ipv4_dstAddr = 192.168.1.2;
};
