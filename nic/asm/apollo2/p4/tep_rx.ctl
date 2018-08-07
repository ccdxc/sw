#include "ingress.h"
#include "INGRESS_p.h"
#include "apollo.h"

struct tep_rx_k     k;
struct tep_rx_d     d;
struct phv_         p;

k = {
    ipv4_1_srcAddr = 0x12345678;
};

d = {
    tep_rx_d.tep_ip = 0x12345678;
};

c1 = 0;
r1 = 0;
