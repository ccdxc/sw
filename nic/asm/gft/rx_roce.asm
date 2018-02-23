#include "ingress.h"
#include "INGRESS_p.h"
#include "CSUM_INGRESS.h"
#include "../../p4/gft/include/defines.h"

struct rx_roce_k k;
struct rx_roce_d d;
struct phv_ p;

%%

nop:
    nop.e
    nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rx_roce:
    seq             c1, d.u.rx_roce_d.tm_oq_overwrite, TRUE
    phvwr.c1        p.capri_intrinsic_tm_oq, d.u.rx_roce_d.tm_oq

    add             r1, d.u.rx_roce_d.len, \
                        (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + \
                        CAPRI_RXDMA_INTRINSIC_HDR_SZ + P4PLUS_ROCE_HDR_SZ)

    smeqb           c1, k.roce_bth_opCode, 0xE0, 0x60
    add.c1          r1, r1, 14
    phvwrpair       p.capri_rxdma_intrinsic_rx_splitter_offset, r1, \
                        p.p4_to_p4plus_roce_ecn, k.roce_metadata_ecn

    // set payload len
    sub             r1, k.roce_metadata_udp_len, 12
    add.c1          r1, r1, 40
    sub             r1, r1, d.u.rx_roce_d.len
    phvwrpair       p.p4_to_p4plus_roce_p4plus_app_id, P4PLUS_APPTYPE_RDMA, \
                        p.p4_to_p4plus_roce_payload_len, r1

    // remove icrc for pre-parser optimized path
    seq             c2, k.roce_bth_valid, FALSE
    phvwr.c2        p.capri_deparser_len_trunc, TRUE
    phvwr.c2        p.capri_deparser_len_trunc_pkt_len, r1

    // udp options
    bbne            k.icrc_valid, TRUE, rx_roce_udp_options_done
    seq             c2, k.udp_opt_ocs_valid, FALSE
    seq             c3, k.capri_intrinsic_csum_err[csum_hdr_udp_opt_ocs], 0
    orcf            c2, [!c2 & c3]
    phvwrpair.c2    p.p4_to_p4plus_roce_roce_opt_ts_valid, \
                        k.udp_opt_timestamp_valid, \
                        p.p4_to_p4plus_roce_roce_opt_mss_valid, \
                        k.udp_opt_mss_valid

rx_roce_udp_options_done:
    phvwrpair       p.p4_to_p4plus_roce_valid, TRUE, \
                        p.capri_rxdma_intrinsic_valid, TRUE
    phvwrpair       p.p4_to_p4plus_roce_rdma_hdr_len, d.u.rx_roce_d.len[5:0], \
                        p.p4_to_p4plus_roce_raw_flags, d.u.rx_roce_d.raw_flags
    phvwrpair       p.capri_rxdma_intrinsic_qid, k.roce_bth_destQP, \
                        p.capri_rxdma_intrinsic_qtype, d.u.rx_roce_d.qtype
    phvwr           p.capri_intrinsic_payload, FALSE

    // remove all headers
    .assert(offsetof(p, udp_2_valid) - offsetof(p, ethernet_1_valid) == 35)
    phvwrpair       p.ipv6_1_valid, 0, p.{udp_2_valid...ethernet_1_valid}, 0

    nop.!c1.e

    .assert(offsetof(p, vxlan_3_valid) - offsetof(p, icrc_valid) == 7)
    .assert(offsetof(p, gre_3_valid) - offsetof(p, gre_1_valid) == 24)
    phvwrpair       p.{vxlan_3_valid...icrc_valid}, 0, \
                        p.{gre_3_valid...gre_1_valid}, 0

    // eth and IP header for UD
    seq             c1, k.ethernet_3_valid, TRUE
    phvwrpair.c1.e  p.p4_to_p4plus_roce_ip_3_valid, TRUE, \
                        p.p4_to_p4plus_roce_eth_3_valid, TRUE
    seq             c1, k.ethernet_2_valid, TRUE
    phvwrpair.c1.e  p.p4_to_p4plus_roce_ip_2_valid, TRUE, \
                        p.p4_to_p4plus_roce_eth_2_valid, TRUE
    phvwrpair.!c1.e p.p4_to_p4plus_roce_ip_1_valid, TRUE, \
                        p.p4_to_p4plus_roce_eth_1_valid, TRUE
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rx_roce_error:
    nop.e
    nop
