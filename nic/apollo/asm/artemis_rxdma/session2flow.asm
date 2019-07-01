
#include "artemis_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct session2flow_k  k;
struct session2flow_d  d;
struct phv_            p;

%%
    
session2flow:

    phvwr.e      p.{rx_to_tx_hdr4_iflow_ipaf...rx_to_tx_hdr4_iflow_ind}, d.{session2flow_d.iflow_ipaf...session2flow_d.iflow_ind}
    phvwr      p.{rx_to_tx_hdr4_rflow_ipaf...rx_to_tx_hdr4_rflow_ind}, d.{session2flow_d.rflow_ipaf...session2flow_d.rflow_ind}


