
#include "artemis_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct session_aging_k  k;
struct session_aging_d  d;
struct phv_             p;

%%
session_aging:
    phvwr.e     p.capri_intr_drop, 1
    nop
    
    
