#include "apollo_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct toeplitz_seed_toeplitz_seed_init_d   d;
struct phv_                                 p;

%%

toeplitz_seed_init:
    // copy seed/secret from d to phv
    phvwr           p.toeplitz_seed0_data, d.seed[319:192]
    phvwr.e         p.toeplitz_seed1_data, d.seed[191:64]
    phvwr           p.toeplitz_seed2_data, d.seed[63:0]
