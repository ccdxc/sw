#include "artemis_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_iflow_rx2tx_k.h"

struct iflow_rx2tx_k_       k;
struct iflow_rx2tx_d        d;
struct phv_                 p;

#define FLOW_IPV4_ENTRY_SHIFT 5
#define FLOW_IPV6_ENTRY_SHIFT 6

#define c_ipv4              c7

%%
    .param          flow
    .param          flow_ohash
    .param          ipv4_flow
    .param          ipv4_flow_ohash
    .param          invalid_flow_base

iflow_rx2tx:

    // Fill iflow meta information
    phvwr           p.txdma_to_arm_meta_iflow_hash, d.{iflow_rx2tx_d.flow_hash}.wx

    seq             c1, d.iflow_rx2tx_d.parent_is_hint, 1
    phvwr.c1        p.txdma_to_arm_meta_iflow_parent_is_hint, 1
    // parent_index is filled by p4 pipeline if parent_is_hint, else it is flow_hash
    phvwr.c1        p.txdma_to_arm_meta_iflow_parent_index, d.{iflow_rx2tx_d.parent_hint_index}.wx
    phvwr.!c1       p.txdma_to_arm_meta_iflow_parent_index, d.{iflow_rx2tx_d.flow_hash}[22:0].wx

    seq             c_ipv4, d.iflow_rx2tx_d.ipaf, 0
    phvwr.!c_ipv4   p.txdma_to_arm_meta_iflow_ipaf, 1

    phvwr           p.txdma_to_arm_meta_iflow_parent_hint_slot, d.iflow_rx2tx_d.parent_hint_slot
    phvwr           p.txdma_to_arm_meta_iflow_nrecircs, d.iflow_rx2tx_d.flow_nrecircs

    b.!c_ipv4       iflow_ipv6_key

iflow_ipv4_key:
    // Fill iflow key in leaf entry (flit 3)
    //  v4 entries start at flit offset 256 (since they are half wide)
    //  check ipv4_flow_k for offset from bottom where the key starts
    add             r1, 0, (TXDMA_IFLOW_LEAF_FLIT * 512) + 256 + 96 + 8
    phvwrp          r1, 0, 8, d.iflow_rx2tx_d.vpc_id
    add             r1, r1, 8
    phvwrp          r1, 0, 32, d.iflow_rx2tx_d.flow_dst[31:0]
    add             r1, r1, 32
    phvwrp          r1, 0, 32, d.iflow_rx2tx_d.flow_src[31:0]
    add             r1, r1, 32
    phvwrp          r1, 0, 16, d.iflow_rx2tx_d.flow_sport
    add             r1, r1, 16
    phvwrp          r1, 0, 16, d.iflow_rx2tx_d.flow_dport
    add             r1, r1, 16
    phvwrp          r1, 0, 8, d.iflow_rx2tx_d.flow_proto

    // Fill iflow d (has to match ipv4_flow_d)
    add             r1, 0, (TXDMA_IFLOW_LEAF_FLIT * 512) + 512 - 8
    phvwrp          r1, 0, 8, d.iflow_rx2tx_d.epoch

#if 1
    /* FIXME rflow , reverse of iflow for now */
    phvwr           p.key_ipv4_flow_ohash, 0
    phvwr           p.key_ipv4_proto, d.iflow_rx2tx_d.flow_proto
    phvwr           p.key_ipv4_ipv4_src, d.iflow_rx2tx_d.flow_dst[31:0]
    phvwr           p.key_ipv4_ipv4_dst, d.iflow_rx2tx_d.flow_src[31:0]
    phvwr           p.key_ipv4_sport, d.iflow_rx2tx_d.flow_dport
    phvwr           p.key_ipv4_dport, d.iflow_rx2tx_d.flow_sport
    phvwr           p.key_ipv4_vpc_id, d.iflow_rx2tx_d.vpc_id
    phvwri          p.key2_ipv4_flow_lkp_type, 4
#endif
    phvwr           p.key_ipv4_epoch, d.iflow_rx2tx_d.epoch

    b               iflow_key_done

iflow_ipv6_key:
    // Fill iflow key in leaf entry (flit 3)
    //  check flow_k for offset from bottom where the key starts
    add             r1, 0, (TXDMA_IFLOW_LEAF_FLIT * 512) + 120 + 8 + 32 + 6
    phvwrp          r1, 0, 2, 2 // ktype
    add             r1, r1, 2
    phvwrp          r1, 0, 8, d.iflow_rx2tx_d.vpc_id
    add             r1, r1, 8
    phvwrp          r1, 0, 16, d.iflow_rx2tx_d.flow_sport
    add             r1, r1, 16
    phvwrp          r1, 0, 16, d.iflow_rx2tx_d.flow_dport
    add             r1, r1, 16
    phvwrp          r1, 0, 128, d.iflow_rx2tx_d.flow_dst
    add             r1, r1, 128
    phvwrp          r1, 0, 128, d.iflow_rx2tx_d.flow_src
    add             r1, r1, 128
    phvwrp          r1, 0, 8, d.iflow_rx2tx_d.flow_proto

    // Fill iflow d (has to match flow_d)
    add             r1, 0, (TXDMA_IFLOW_LEAF_FLIT * 512) + 512 - 8
    phvwrp          r1, 0, 8, d.iflow_rx2tx_d.epoch

#if 1
    /* FIXME: rflow , reverse of iflow for now */
    phvwr           p.key1_flow_ohash, 0
    phvwr           p.key1_proto, d.iflow_rx2tx_d.flow_proto
    phvwr           p.key1_src, d.iflow_rx2tx_d.flow_dst[127:16]
    phvwr           p.key2_src, d.iflow_rx2tx_d.flow_dst[15:0]
    phvwr           p.key2_dst, d.iflow_rx2tx_d.flow_src[127:48]
    phvwr           p.key3_dst, d.iflow_rx2tx_d.flow_src[47:0]
    phvwr           p.key3_sport, d.iflow_rx2tx_d.flow_dport
    phvwr           p.key3_dport, d.iflow_rx2tx_d.flow_sport
    phvwr           p.key3_vpc_id, d.iflow_rx2tx_d.vpc_id
    phvwri          p.key3_ktype, KEY_TYPE_IPV6
    phvwri          p.key3_flow_lkp_type, 6
#endif
    phvwr           p.key3_epoch, d.iflow_rx2tx_d.epoch

iflow_key_done:

    seq             c1, d.iflow_rx2tx_d.parent_valid, 0
    phvwri.c1.e     p.txdma_control_pktdesc_addr, loword(invalid_flow_base)

    seq             c2, d.iflow_rx2tx_d.parent_is_hint, 0

    add.c2          r2, d.iflow_rx2tx_d.flow_hash[22:0], r0
    add.!c2         r2, d.iflow_rx2tx_d.parent_hint_index, r0

    sll.c_ipv4      r2, r2, FLOW_IPV4_ENTRY_SHIFT
    sll.!c_ipv4     r2, r2, FLOW_IPV6_ENTRY_SHIFT

    setcf           c3, [c_ipv4 & c2]
    addi.c3         r3, r2, loword(ipv4_flow)
    addui.c3        r3, r3, hiword(ipv4_flow)

    setcf           c4, [c_ipv4 & !c2]
    addi.c4         r3, r2, loword(ipv4_flow_ohash)
    addui.c4        r3, r3, hiword(ipv4_flow_ohash)

    setcf           c5, [!c_ipv4 & c2]
    addi.c5         r3, r2, loword(flow)
    addui.c5        r3, r3, hiword(flow)

    setcf           c6, [!c_ipv4 & !c2]
    addi.c6         r3, r2, loword(flow_ohash)
    addui.c6        r3, r3, hiword(flow_ohash)

    phvwr.e         p.txdma_control_pktdesc_addr, r3
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
iflow_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
