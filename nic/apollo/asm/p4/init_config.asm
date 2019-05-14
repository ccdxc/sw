#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_init_config_k.h"

struct init_config_k_ k;
struct phv_ p;

%%

init_config:
    bbne            k.service_header_valid, TRUE, service_header_done
    xor             r1, k.service_header_local_ip_mapping_done, 0x1
    phvwr           p.control_metadata_local_ip_mapping_ohash_lkp, r1
    xor             r1, k.service_header_remote_vnic_mapping_rx_done, 0x1
    phvwr           p.control_metadata_remote_vnic_mapping_rx_ohash_lkp, r1
    xor             r1, k.service_header_flow_done, 0x1
    phvwr           p.control_metadata_flow_ohash_lkp, r1

service_header_done:
    seq             c1, k.key_metadata_ktype, KEY_TYPE_IPV6
    phvwr.c1        p.p4_to_txdma_header_lpm_addr, k.control_metadata_lpm_v6addr
    phvwr.c1        p.p4_to_rxdma_header_sacl_base_addr, \
                        k.control_metadata_sacl_v6addr
    sub             r1, k.capri_p4_intrinsic_frame_size, \
                        k.offset_metadata_l2_1
    phvwr           p.capri_p4_intrinsic_packet_len, r1
    sne.e           c1, k.capri_intrinsic_tm_oq, TM_P4_RECIRC_QUEUE
    phvwr.c1        p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
init_config_error:
    phvwr           p.capri_intrinsic_drop, 1
    sne.e           c1, k.capri_intrinsic_tm_oq, TM_P4_RECIRC_QUEUE
    phvwr.c1        p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq
