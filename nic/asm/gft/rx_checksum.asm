#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct rx_checksum_k k;
struct phv_ p;

%%

rx_checksum:
rx_checksum_1:
    bbne            k.control_metadata_update_checksum_1, TRUE, rx_checksum_2
    seq             c1, k.ipv4_1_valid, TRUE
    phvwr           p.ipv4_1_csum, k.ipv4_1_valid
    bbne            k.tcp_1_valid, TRUE, rx_checksum_1_non_tcp
    sub             r1, k.ipv4_1_totalLen, k.ipv4_1_ihl, 2
    add.!c1         r1, r0, k.ipv6_1_payloadLen
    phvwr           p.capri_deparser_len_rx_l4_payload_len, r1
    phvwr           p.tcp_1_csum, TRUE
    b               rx_checksum_2
    phvwrpair       p.ipv6_1_tcp_csum, k.ipv6_1_valid, \
                        p.ipv4_1_tcp_csum, k.ipv4_1_valid

rx_checksum_1_non_tcp:
    bbne            k.udp_1_valid, TRUE, rx_checksum_2
    phvwrpair       p.ipv6_1_udp_csum, k.ipv6_1_valid, \
                        p.ipv4_1_udp_csum, k.ipv4_1_valid
    phvwr           p.udp_1_csum, TRUE
    phvwr           p.capri_deparser_len_rx_l4_payload_len, \
                        k.{udp_1_len_sbit0_ebit7,udp_1_len_sbit8_ebit15}

rx_checksum_2:
    bbne            k.control_metadata_update_checksum_2, TRUE, rx_checksum_3
    seq             c1, k.ipv4_2_valid, TRUE
    phvwr           p.ipv4_2_csum, k.ipv4_2_valid
    bbne            k.tcp_2_valid, TRUE, rx_checksum_2_non_tcp
    sub             r1, k.ipv4_2_totalLen, k.ipv4_2_ihl, 2
    add.!c1         r1, r0, k.ipv6_2_payloadLen
    phvwr           p.capri_deparser_len_rx_l4_payload_len, r1
    //phvwr           p.tcp_2_csum, TRUE
    b               rx_checksum_2
    phvwrpair       p.ipv4_2_tcp_csum, k.ipv4_2_valid, \
                        p.ipv6_2_tcp_csum, k.ipv6_2_valid

rx_checksum_2_non_tcp:
    bbne            k.udp_2_valid, TRUE, rx_checksum_2
    phvwrpair       p.ipv4_2_udp_csum, k.ipv4_2_valid, \
                        p.ipv6_2_udp_csum, k.ipv6_2_valid
    phvwr           p.udp_2_csum, TRUE
    phvwr           p.capri_deparser_len_rx_l4_payload_len, \
                        k.{udp_2_len_sbit0_ebit7,udp_2_len_sbit8_ebit15}

rx_checksum_3:
    seq             c1, k.control_metadata_update_checksum_3, TRUE
    nop.!c1.e
    seq             c1, k.ipv4_3_valid, TRUE
    phvwr           p.ipv4_3_csum, k.ipv4_3_valid
    bbne            k.tcp_3_valid, TRUE, rx_checksum_3_non_tcp
    sub             r1, k.ipv4_3_totalLen, k.ipv4_3_ihl, 2
    add.!c1         r1, r0, k.ipv6_3_payloadLen
    phvwr           p.capri_deparser_len_rx_l4_payload_len, r1
    phvwrpair.e     p.ipv4_3_tcp_csum, k.ipv4_3_valid, \
                        p.ipv6_3_tcp_csum, k.ipv6_3_valid
    phvwr.f         p.tcp_3_csum, TRUE

rx_checksum_3_non_tcp:
    seq             c1, k.udp_3_valid, TRUE
    nop.!c1.e
    phvwrpair       p.ipv4_3_udp_csum, k.ipv4_3_valid, \
                        p.ipv6_3_udp_csum, k.ipv6_3_valid
    phvwr.e         p.udp_3_csum, TRUE
    phvwr.f         p.capri_deparser_len_rx_l4_payload_len, \
                        k.{udp_3_len_sbit0_ebit7,udp_3_len_sbit8_ebit15}


/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rx_checksum_error:
    nop.e
    nop
