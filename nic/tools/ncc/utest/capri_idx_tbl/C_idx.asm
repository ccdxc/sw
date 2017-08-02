#include "ingress.h"
#include "INGRESS_p.h"
struct C_idx_k  k;
struct C_idx_d  d;
struct phv_     p;

%%
update_ttl:
    phvwr   p.ipv4_ttl, d.update_ttl_d.ttl_val
    phvwr.e p.capri_intrinsic_tm_oport, 0
    nop
