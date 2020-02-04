#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_config2_k.h"
#include "athena.h"

struct config2_k_   k;
struct config2_d    d;
struct phv_         p;


%%
config2_epoch_verify:
    sne.e       c1, k.control_metadata_epoch2_value, d.config2_epoch_verify_d.epoch
    phvwr.c1    p.control_metadata_flow_miss, TRUE
