#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct rx_ipolicer_d d;
struct phv_ p;

%%

rx_ipolicer:
    seq             c1, d.rx_policer_d.entry_valid, TRUE
    seq.c1          c1, d.rx_policer_d.tbkt[39], TRUE
    phvwr.c1        p.capri_intrinsic_drop, TRUE
    nop.e
    nop
