/*****************************************************************************/
/* IPv4 Route LPM lookup                                                     */
/*****************************************************************************/

#include "../include/artemis_defines.h"
#include "txlpm1.p4"

action route_res_handler() {
    if ((scratch_metadata.field16 & ROUTE_RESULT_TYPE_PEER_VPC_MASK) != 0) {
        // Set VPCID in PHV from LPM result
        modify_field(txdma_control.vpc_id, scratch_metadata.field16 & 0x7FFF);

        // For NH_TYPE=VNET, set rewrite Tx: src_ip / Rx:dst_ip to 00 (nothing to do)
        // Set Encap for NH_TYPE=VNET
        modify_field(session_info_hint.tx_rewrite_flags_encap, 1);
    } else {
        if ((scratch_metadata.field16 & ROUTE_RESULT_TYPE_SVC_TUNNEL_MASK) != 0) {
            // Set ST ID in PHV from LPM result
            modify_field(txdma_control.svc_id, scratch_metadata.field16 & 0x7FFF);

            // For NH_TYPE=ST, set the Tx: src_ip flag to 11 (rewrite CA6)
            modify_field(session_info_hint.tx_rewrite_flags_src_ip, 11);

            // Set Encap for NH_TYPE=ST
            modify_field(session_info_hint.tx_rewrite_flags_encap, 1);

            // Set Rx:src_ip to 10 (blind 64 mapping) for NH_TYPE=ST
            modify_field(session_info_hint.tx_rewrite_flags_src_ip, 10);

            // For NH_TYPE=ST, set the Rx: dst_ip flag to 11 (64 mapping)
            modify_field(session_info_hint.rx_rewrite_flags_dst_ip, 11);
        } else {
            // Set NHID in PHV from LPM result
            modify_field(session_info_hint.nexthop_idx, scratch_metadata.field16);

            // Tx: For NH_TYPE=IP, set src_ip flag to 01 (public IP from
            // local mapping table index) and if mapping table finds entry then 
            // src_ip rewrite flag to 10 (from service mapping table index)
            modify_field(session_info_hint.tx_rewrite_flags_src_ip, 01);

            // Set Rx:smac to 1 for NH_TYPE=IP
            modify_field(session_info_hint.rx_rewrite_flags_smac, 01);

            // Rx: For NH_TYPE=IP, set the dst_ip flag to 01 (CA from mapping -
            // from local-ip mapping table index)and if mapping table finds entry then 
            // dst_ip rewrite flag to 10 (CA from service mapping table index)
            modify_field(session_info_hint.rx_rewrite_flags_dst_ip, 01);
        }
    }
    // Disable further LPM stages.
    modify_field(app_header.table0_valid, FALSE);
}

control route_lookup {
    apply(txlpm1_2);
    apply(txlpm1_3);
    apply(txlpm1_4);
    apply(txlpm1_5);
    apply(txlpm1_6);
    apply(txlpm1);
}
