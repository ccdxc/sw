#include "egress.h"
#include "EGRESS_p.h"
#include "athena.h"

struct p4e_to_uplink_k  k;
struct phv_             p;

%%


p4e_to_uplink:
    /*
    phvwr           p.p4e_to_p4plus_classic_nic_ip_valid, FALSE
    phvwr           p.p4e_to_p4plus_classic_nic_valid, FALSE
    phvwr           p.capri_rxdma_intrinsic_valid, FALSE
    phvwr           p.p4i_to_p4e_header_valid, FALSE
    phvwr           p.capri_p4_intrinsic_valid, TRUE
    phvwr           p.capri_intrinsic_valid, TRUE
    */
    phvwr.e         p.{ p4e_to_p4plus_classic_nic_ip_valid, \
                        p4e_to_p4plus_classic_nic_valid, \
                        capri_rxdma_intrinsic_valid, \
                        p4i_to_p4e_header_valid, \
                        capri_p4_intrinsic_valid, \
                        capri_intrinsic_valid }, 0x03


    phvwr           p.capri_intrinsic_tm_oport, k.p4i_to_p4e_header_nacl_redir_oport
