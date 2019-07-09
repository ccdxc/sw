#include "artemis_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_iflow_entry_k.h"

struct iflow_entry_k_       k;
struct iflow_entry_d        d;
struct phv_                 p;

%%
    .param          invalid_flow_base

iflow_entry:
    addi            r1, r0, loword(invalid_flow_base)
    seq             c1, k.txdma_control_pktdesc_addr, r1
    b.c1            rflow_remote_46

    add             r1, 0, TXDMA_IFLOW_PARENT_FLIT * 512
    phvwrp          r1, 0, 512, d.iflow_entry_d.flow

rflow_remote_46:
    // As K vector is tight in stage0, some of the rflow derivation logic is moved here:
    // Only for Tx initiated flows, if there is hit in remote_46_mapping table, then
    // find local_46_mapping entry and set rflow keys appropriately

    // Nothing to do for Rx direction
    bbne            k.rx_to_tx_hdr_direction, TX_FROM_HOST, iflow_entry_done

    // If remote_46_mapping is hit, then rflow sip with ipv6_tx_da derived from 
    // remote_46_mapping tablefrom this table and rflow dip with local_46_ip table
    // Also set iptype to v6 both i/rflow
    seq            c1, k.session_info_hint_tx_dst_ip[63:0], r0
    seq            c2, k.session_info_hint_tx_dst_ip[127:64], r0
    bcf            [c1 & c2], iflow_entry_done
    nop            //BD Slot

    b               iflow_entry_done
    nop

    phvwr           p.key1_src, k.session_info_hint_tx_dst_ip[127:16]
    phvwr           p.key2_src, k.session_info_hint_tx_dst_ip[15:0]
    phvwr           p.key2_dst, k.txdma_control_tx_local_46_ip_s24_e127[103:24]
    phvwr           p.key3_dst[47:24], k.txdma_control_tx_local_46_ip_s24_e127[23:0]
    phvwr           p.key3_dst[23:0], k.txdma_control_tx_local_46_ip_s0_e23[23:0]
    phvwri          p.key3_ktype, KEY_TYPE_IPV6
    phvwri          p.key3_flow_lkp_type, 6

iflow_entry_done:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
iflow_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
