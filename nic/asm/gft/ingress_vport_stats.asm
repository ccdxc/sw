#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct ingress_vport_stats_k k;
struct ingress_vport_stats_d d;
struct phv_ p;

%%

ingress_vport_stats:
    nop.e
    nop
