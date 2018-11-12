#include "ingress.h"
#include "INGRESS_p.h"

struct snat_k   k;
struct snat_d   d;
struct phv_     p;

%%

snat_rewrite:
    phvwr.e         p.ipv4_1_srcAddr, d.snat_rewrite_d.sip
    phvwr           p.tcp_srcPort, d.snat_rewrite_d.sport
