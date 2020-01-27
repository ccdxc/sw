#include "ingress.h"
#include "INGRESS_config1_k.h"
#include "INGRESS_p.h"
#include "athena.h"

struct config1_k_   k;
struct config1_d    d;
struct phv_         p;


%%
config1_epoch_verify:
    sne.e       c1, k.control_metadata_epoch1_value, d.config1_epoch_verify_d.epoch
    phvwr.c1    p.control_metadata_flow_miss, TRUE
