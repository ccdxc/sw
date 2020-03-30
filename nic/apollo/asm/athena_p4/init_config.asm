#include "athena.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_init_config_k.h"

struct init_config_k_   k;
struct phv_             p;

%%

init_config:
    bbne            k.ingress_recirc_header_valid, TRUE, ingress_recirc_header_done
    nop
    phvwr           p.control_metadata_flow_ohash_lkp, TRUE
    phvwr           p.control_metadata_dnat_ohash_lkp, TRUE

ingress_recirc_header_done:
    seq             c1, k.control_metadata_skip_flow_lkp, TRUE
    phvwr.c1        p.control_metadata_flow_miss, TRUE
    phvwr.c1        p.ingress_recirc_header_flow_done, TRUE

    sne             c1, k.capri_intrinsic_tm_oq, TM_P4_RECIRC_QUEUE
    phvwr.c1        p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq
    phvwr.!c1       p.capri_intrinsic_tm_oq, k.capri_intrinsic_tm_iq

    sub             r1, k.capri_p4_intrinsic_frame_size, \
                        k.offset_metadata_l2_1
    phvwr.e         p.capri_p4_intrinsic_packet_len, r1
    phvwr           p.p4i_to_p4e_header_packet_len, r1
