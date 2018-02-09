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
    phvwrpair       p.p4_to_p4plus_roce_valid, TRUE, \
                        p.capri_rxdma_intrinsic_valid, TRUE
    phvwrpair       p.p4_to_p4plus_roce_rdma_hdr_len, d.u.rx_roce_d.len[5:0], \
                        p.p4_to_p4plus_roce_raw_flags, d.u.rx_roce_d.raw_flags
    phvwr           p.p4_to_p4plus_roce_p4plus_app_id, P4PLUS_APPTYPE_RDMA
    phvwrpair       p.capri_rxdma_intrinsic_qid, k.roce_bth_destQP, \
                        p.capri_rxdma_intrinsic_qtype, d.u.rx_roce_d.qtype
    phvwr           p.capri_intrinsic_payload, FALSE

    seq             c1, d.u.rx_roce_d.tm_oq_overwrite, TRUE
    phvwr.c1        p.capri_intrinsic_tm_oq, d.u.rx_roce_d.tm_oq

    add             r1, d.u.rx_roce_d.len, \
                        (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + \
                        CAPRI_RXDMA_INTRINSIC_HDR_SZ + P4PLUS_ROCE_HDR_SZ)

    smeqb           c1, k.roce_bth_opCode, 0xE0, 0x60
    add.c1          r1, r1, 14
    phvwr           p.capri_rxdma_intrinsic_rx_splitter_offset, r1

    // set payload len
    seq             c2, k.icrc_valid, TRUE
    cmov            r2, c2, 12, 8
    sub             r1, k.roce_metadata_udp_len, r2
    add.c1          r1, r1, 40
    sub             r1, r1, d.u.rx_roce_d.len
    phvwr           p.p4_to_p4plus_roce_payload_len, r1

    // udp options
    seq             c2, k.udp_opt_ocs_valid, FALSE
    seq             c3, k.capri_intrinsic_csum_err[csum_hdr_udp_opt_ocs], 0
    orcf            c2, [!c2 & c3]
    phvwrpair.c2    p.p4_to_p4plus_roce_roce_opt_ts_valid, \
                        k.udp_opt_timestamp_valid, \
                        p.p4_to_p4plus_roce_roce_opt_mss_valid, \
                        k.udp_opt_mss_valid

    // remove all headers (to be optimized later)
    phvwrpair       p.ctag_1_valid, 0, p.ethernet_1_valid, 0
    phvwrpair       p.ipv6_1_valid, 0, p.ipv4_1_valid, 0
    phvwrpair       p.gre_1_valid, 0, p.vxlan_1_valid, 0
    phvwr           p.udp_1_valid, 0
    phvwrpair       p.ctag_2_valid, 0, p.ethernet_2_valid, 0
    phvwrpair       p.ipv4_2_valid, 0, p.ipv6_2_valid, 0
    phvwrpair       p.vxlan_2_valid, 0, p.gre_2_valid, 0
    phvwr           p.udp_2_valid, 0
    phvwrpair       p.ctag_3_valid, 0, p.ethernet_3_valid, 0
    phvwrpair       p.ipv4_3_valid, 0, p.ipv6_3_valid, 0
    phvwrpair       p.vxlan_3_valid, 0, p.gre_3_valid, 0
    phvwr           p.udp_3_valid, 0

    nop.!c1.e

    .assert(offsetof(p, udp_opt_eol_valid) - offsetof(p, icrc_valid) == 6)
    phvwr           p.{udp_opt_eol_valid...icrc_valid}, 0

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
