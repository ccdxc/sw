#include "egress.h"
#include "EGRESS_p.h"
#include "athena.h"
#include "EGRESS_p4i_to_p4e_state_k.h"

struct p4i_to_p4e_state_k_  k;
struct phv_                 p;

%%

p4i_to_p4e_state:
    seq             c1, k.p4i_to_p4e_header_valid, TRUE
    phvwr.c1        p.control_metadata_forward_to_uplink, k.p4i_to_p4e_header_forward_to_uplink
    phvwr.c1        p.control_metadata_redir_to_rxdma, k.p4i_to_p4e_header_redir_to_rxdma

    nop.e
    nop
