#include "apollo_rxdma.h"
#include "../../../p4/apollo2/include/defines.h"
#include "INGRESS_p.h"
#include "ingress.h"

#include "capri-macros.h"
#include "capri_common.h"

struct phv_                 p;

%%

rx_cpu_hash_result:
    phvwr.e     p.hash_results_cpu_qid_hash, r1  // hash value
    nop
