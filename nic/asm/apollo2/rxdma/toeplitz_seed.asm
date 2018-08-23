#include "apollo_rxdma.h"
#include "../../../p4/apollo2/include/defines.h"
#include "INGRESS_p.h"
#include "ingress.h"

#include "capri-macros.h"
#include "capri_common.h"

struct toeplitz_seed_toeplitz_seed_init_d   d;
struct phv_                                 p;

%%

toeplitz_seed_init:
    // copy seed/secret from d to phv
    phvwr           p.toeplitz_seed0_data, d.seed[319:192]
    phvwr.e         p.toeplitz_seed1_data, d.seed[191:64]
    phvwr           p.toeplitz_seed2_data, d.seed[63:0]
