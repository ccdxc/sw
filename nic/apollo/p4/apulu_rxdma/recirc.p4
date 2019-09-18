#include "../include/apulu_sacl_defines.h"

action rxlpm1_res_handler()
{
    if (capri_p4_intr.recirc_count == 1) {
        // Write the derived SIP class id to PHV
        modify_field(rx_to_tx_hdr.sip_classid,
                     scratch_metadata.field10);
    } else  {
        if (capri_p4_intr.recirc_count == 2)
        {
            // Write the derived TAG class id to PHV
            if (p4_to_rxdma.rx_packet == 0) {
                modify_field(rx_to_tx_hdr.dtag_classid,
                             scratch_metadata.field10);
            } else {
                modify_field(rx_to_tx_hdr.stag_classid,
                             scratch_metadata.field10);
            }
        }
    }
}

action rxlpm2_res_handler()
{
    if (capri_p4_intr.recirc_count == 0) {
        // Write the derived DPORT class id to PHV
        modify_field(rx_to_tx_hdr.dport_classid,
                     scratch_metadata.field10);

        // Setup key for SPORT lookup on LPM2
        modify_field(lpm_metadata.lpm2_key,
                     p4_to_rxdma.flow_sport);

        // Setup root address of SPORT lookup on LPM2
        modify_field(lpm_metadata.lpm2_base_addr,
                     rx_to_tx_hdr.sacl_base_addr +
                     SACL_SPORT_TABLE_OFFSET);
        // Set next address same as the root address
        modify_field(lpm_metadata.lpm2_next_addr,
                     rx_to_tx_hdr.sacl_base_addr +
                     SACL_SPORT_TABLE_OFFSET);
    } else {
        if (capri_p4_intr.recirc_count == 1) {
            // Write the derived DIP result to PHV
            modify_field(rx_to_tx_hdr.dip_classid,
                         scratch_metadata.field10);
        } else {
            // Write the derived METER result to PHV
            modify_field(rx_to_tx_hdr.meter_result,
                         scratch_metadata.field10);
        }
    }
}

action sport_res_handler()
{
    // Write the derived SPORT result to PHV
    modify_field(rx_to_tx_hdr.sport_classid,
                 scratch_metadata.field10);
}

action nop_res_handler()
{
}

action setup_recirc()
{
    if (capri_p4_intr.recirc_count == 0) {
        // Setup key for SIP lookup on LPM1
        modify_field(lpm_metadata.lpm1_key, p4_to_rxdma.flow_src);

        // Setup root address for SIP lookup on LPM1
        if (p4_to_rxdma.iptype == IPTYPE_IPV4) {
            modify_field(lpm_metadata.lpm1_base_addr,
                         rx_to_tx_hdr.sacl_base_addr +
                         SACL_IPV4_SIP_TABLE_OFFSET);
        } else {
            modify_field(lpm_metadata.lpm1_base_addr,
                         rx_to_tx_hdr.sacl_base_addr +
                         SACL_IPV6_SIP_TABLE_OFFSET);
        }

        // Setup key for DIP lookup on LPM2
        modify_field(lpm_metadata.lpm2_key, p4_to_rxdma.flow_dst);
        // Setup root address of DIP lookup on LPM2
        modify_field(lpm_metadata.lpm2_base_addr,
                     rx_to_tx_hdr.sacl_base_addr +
                     SACL_DIP_TABLE_OFFSET);

        // Enable LPM1
        modify_field(p4_to_rxdma.lpm1_enable, TRUE);

        // Recirc the packet
        modify_field(capri_p4_intr.recirc, 1);
    } else {
        // Setup key for TAG Lookup on LPM1
        modify_field(lpm_metadata.lpm1_key, rx_to_tx_hdr.remote_ip);
        // Setup root address of TAG lookup on LPM1
        modify_field(lpm_metadata.lpm1_base_addr, p4_to_rxdma.tag_root);

        // Setup key for METER Lookup on LPM2
        modify_field(lpm_metadata.lpm2_key, rx_to_tx_hdr.remote_ip);
        // Setup root address of METER lookup on LPM2
        modify_field(lpm_metadata.lpm2_base_addr, lpm_metadata.meter_base_addr);

        // Recirc the packet
        modify_field(capri_p4_intr.recirc, 1);
    }
}

@pragma stage 7
table setup_recirc {
    actions {
        setup_recirc;
    }
}

control recirc {
    apply(setup_recirc);
}
