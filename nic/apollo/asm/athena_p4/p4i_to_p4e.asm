#include "ingress.h"
#include "INGRESS_p.h"
#include "athena.h"
#include "INGRESS_p4i_to_p4e_k.h"

struct p4i_to_p4e_k_    k;
struct phv_             p;

%%


p4i_to_p4e:
    seq             c1, k.ingress_recirc_header_flow_done, TRUE
    seq.c1          c1, k.ingress_recirc_header_dnat_done, TRUE
    /* Recirc scenario */
#if 0
    phvwr.!c1       p.p4plus_to_p4_vlan_valid, FALSE
    phvwr.!c1       p.p4plus_to_p4_valid, FALSE
    phvwr.!c1       p.predicate_header_valid, TRUE
    phvwr.!c1       p.capri_txdma_intrinsic_valid, FALSE
    phvwr.!c1       p.p4i_to_p4e_header_valid, FALSE
    phvwr.!c1       p.ingress_recirc_header_valid, TRUE
    phvwr.!c1       p.capri_p4_intrinsic_valid, TRUE
    phvwr.!c1       p.capri_intrinsic_valid, TRUE
#endif
    phvwr.!c1       p.ingress_recirc_header_direction, k.control_metadata_direction
    phvwr.!c1.e     p.{p4plus_to_p4_vlan_valid...capri_intrinsic_valid}, 0x27
    phvwr.!c1.f     p.capri_intrinsic_tm_oport, TM_PORT_INGRESS


    /* Ingress to Egress */
    phvwr           p.p4i_to_p4e_header_flow_miss, k.control_metadata_flow_miss
#if 0
    phvwr.!c1       p.p4plus_to_p4_vlan_valid, FALSE
    phvwr.!c1       p.p4plus_to_p4_valid, FALSE
    phvwr.!c1       p.predicate_header_valid, FALSE
    phvwr.!c1       p.capri_txdma_intrinsic_valid, FALSE
    phvwr.!c1       p.p4i_to_p4e_header_valid, TRUE
    phvwr.!c1       p.ingress_recirc_header_valid, FALSE
    phvwr.!c1       p.capri_p4_intrinsic_valid, TRUE
    phvwr.!c1       p.capri_intrinsic_valid, TRUE
#endif
    phvwr.c1.e      p.{p4plus_to_p4_vlan_valid...capri_intrinsic_valid}, 0x0b
    phvwr.c1.f      p.capri_intrinsic_tm_oport, TM_PORT_EGRESS
