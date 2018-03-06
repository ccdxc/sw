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

    // payload len
    sub             r1, k.roce_metadata_udp_len, 12
    add.c1          r1, r1, 40
    sub             r1, r1, d.u.rx_roce_d.len

    // payload len(r1) => r7[13:0], app_id => r7[47:44]
    add             r7, r1, P4PLUS_APPTYPE_RDMA, 44

    // remove icrc
    seq             c2, k.roce_bth_valid, FALSE
    phvwr.c2        p.{capri_intrinsic_payload,capri_deparser_len_trunc}, 0x1
    phvwr.!c2       p.capri_intrinsic_payload, FALSE
    sub             r2, k.roce_metadata_udp_len, d.u.rx_roce_d.parsed_hdrs_len
    phvwr.c2        p.capri_deparser_len_trunc_pkt_len, r2

    // splitter offset => r7[61:52]
    add             r1, d.u.rx_roce_d.len, \
                        (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + \
                        CAPRI_RXDMA_INTRINSIC_HDR_SZ + P4PLUS_ROCE_HDR_SZ)
    smeqb           c1, k.roce_bth_opCode, 0xE0, 0x60
    add.c1          r1, r1, 14
    or              r7, r7, r1, 51

    // udp options
    bbne            k.icrc_valid, TRUE, rx_roce_udp_options_done
    seq             c2, k.udp_opt_ocs_valid, FALSE
    seq             c3, k.capri_intrinsic_csum_err[csum_hdr_udp_opt_ocs], 0
    orcf            c2, [!c2 & c3]
    // udp options valid bits => r7[39:38]
    or.c2           r7, r7, k.{udp_opt_timestamp_valid,udp_opt_mss_valid}, 38

rx_roce_udp_options_done:
    // rdma_hdr_len => r7[37:32], roce_raw_flags => r7[31:16]
    or              r7, r7, d.u.rx_roce_d.len[5:0], 32
    or              r7, r7, d.u.rx_roce_d.raw_flags, 16

    // ecn => r7[15:14]
    or              r7, r7, k.roce_metadata_ecn, 14

    // qtype => r7[63:61]
    or              r7, r7, d.u.rx_roce_d.qtype, 61

    // write qid, r7
    phvwrpair       p.capri_rxdma_intrinsic_qid, k.roce_bth_destQP, \
                        p.{capri_rxdma_intrinsic_qtype...p4_to_p4plus_roce_payload_len}, r7

    // remove all headers, and set p4_to_p4plus_roce and capri_rxdma_intrinsic
    .assert(offsetof(p, udp_2_valid) - offsetof(p, capri_rxdma_intrinsic_valid) == 39)
    phvwrpair       p.ipv6_1_valid, 0, p.{udp_2_valid...capri_rxdma_intrinsic_valid}, 0x3

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
