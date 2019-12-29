#include "ingress.h"
#include "INGRESS_p.h"
#include "athena.h"
#include "INGRESS_session_info_k.h"

struct session_info_k_  k;
struct session_info_d   d;
struct phv_     p;

%%

session_info:
    /* Validate substrate srcIP address */
    seq             c1, k.control_metadata_direction, RX_FROM_SWITCH
    seq.c1          c2, k.ipv4_1_srcAddr, d.session_info_d.config_substrate_src_ip
    bcf             [c2], session_info_config_check_failed

    phvwr           p.{control_metadata_config1_epoch,control_metadata_config2_epoch}, \
                    d.{session_info_d.config1_epoch,session_info_d.config2_epoch}
    phvwr           p.control_metadata_config1_idx, d.session_info_d.config1_idx
    phvwr           p.{control_metadata_config2_idx...control_metadata_throttle_bw},\
                    d.{session_info_d.config2_idx...session_info_d.throttle_bw};

    phvwr           p.{p4i_to_p4e_header_counterset1...p4i_to_p4e_header_histogram},\
                    d.{session_info_d.counterset1...session_info_d.histogram}

    /*
     * Header "pop"
     */
    seq             c2, d.session_info_d.pop_hdr_flag, TRUE
    bcf             [!c2], session_info_done
    nop


    /* From switch */
    phvwri.c1       p.{mpls_label2_1_valid...ipv6_1_valid}, 0x00

    /* From host & common to from switch */
    phvwr           p.{ctag_1_valid...ethernet_1_valid}, 0x00

    seq             c1, k.ipv4_2_valid, TRUE
    seq             c2, k.ipv6_2_valid, TRUE

    phvwr.c1        p.p4i_to_p4e_header_packet_type, P4E_PACKET_OVERLAY_IPV4
    phvwr.c2        p.p4i_to_p4e_header_packet_type, P4E_PACKET_OVERLAY_IPV6

session_info_done:

    /* TODO:
     *  - Update timestamp
     */

    nop.e
    nop

session_info_config_check_failed:
    phvwr.e         p.p4i_to_p4e_header_flow_miss, TRUE
    nop
    


/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
session_info_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
