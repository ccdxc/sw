#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct ingress_vport_k k;
struct ingress_vport_d d;
struct phv_ p;

%%

ingress_vport:
    phvwr       p.capri_intrinsic_tm_oport, TM_PORT_UPLINK_0
    cmov.e      r1, c1, d.ingress_vport_d.vport, EXCEPTION_VPORT
    phvwr       p.capri_intrinsic_lif, r1
