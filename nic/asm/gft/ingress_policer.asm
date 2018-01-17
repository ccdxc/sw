#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct ingress_policer_d d;
struct phv_ p;

%%

execute_rx_policer:
    seq             c1, d.execute_rx_policer_d.entry_valid, TRUE
    seq.c1          c1, d.execute_rx_policer_d.tbkt[39], TRUE
    phvwr.c1        p.capri_intrinsic_drop, TRUE
    nop.e
    nop
