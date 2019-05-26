#include "../include/artemis_sacl_defines.h"
#include "rxlpm1.p4"
#include "rxlpm2.p4"

action rxlpm1_res_handler()
{
    if (capri_p4_intr.recirc_count == 0) {
        // Write the derived SIP class id to PHV
        modify_field(lpm_metadata.sip_classid,
                     scratch_metadata.field10);

        // Setup key for TAG lookup
        if (p4_to_rxdma.direction == TX_FROM_HOST) {
            modify_field(lpm_metadata.lpm1_key,
                         lpm_metadata.lpm2_key);
        }

        // Setup root address of TAG lookup
        modify_field(lpm_metadata.lpm1_base_addr,
                     lpm_metadata.tag_base_addr);
    } else  {
        if (capri_p4_intr.recirc_count == 1)
        {
            // Write the derived TAG class id to PHV
            modify_field(lpm_metadata.tag_classid,
                         scratch_metadata.field10);

            // HACK! Reset the root so that the 3rd pass can lookup the same tree
            modify_field(lpm_metadata.lpm1_base_addr,
                         lpm_metadata.tag_base_addr);
        }
    }
}

action rxlpm2_res_handler()
{
    if (capri_p4_intr.recirc_count == 0) {
        // Write the derived DIP class id to PHV
        modify_field(lpm_metadata.dip_classid,
                     scratch_metadata.field10);
    } else {
        if (capri_p4_intr.recirc_count == 1) {
            // Write the derived METER result to PHV
            modify_field(lpm_metadata.meter_result,
                         scratch_metadata.field10);
        } else {
            // Write the derived DPORT result to PHV
            modify_field(lpm_metadata.dport_classid,
                         scratch_metadata.field10);

            // Setup key for SPORT lookup
            modify_field(lpm_metadata.lpm2_key,
                         p4_to_rxdma.flow_sport);

            // Setup root address of SPORT lookup
            modify_field(lpm_metadata.lpm2_base_addr,
                         lpm_metadata.lpm2_base_addr +
                         SACL_SPORT_TABLE_OFFSET);
        }
    }
}

action sport_res_handler()
{
    // Write the derived SPORT result to PHV
    modify_field(lpm_metadata.sport_classid,
                 scratch_metadata.field10);
}

action nop_res_handler()
{
}

action setup_recirc()
{
    if (capri_p4_intr.recirc_count == 0) {
        // Setup key for METER lookup
        if (p4_to_rxdma.direction != TX_FROM_HOST) {
            modify_field(lpm_metadata.lpm2_key,
                         lpm_metadata.lpm1_key);
        }

        // Setup root address of METER lookup
        modify_field(lpm_metadata.lpm2_base_addr,
                     lpm_metadata.meter_base_addr);
        // Recirc the packet
        modify_field(capri_p4_intr.recirc_count, 1);
    } else {
        if (capri_p4_intr.recirc_count == 1) {
            // Setup key for DPORT lookup
            modify_field(lpm_metadata.lpm2_key,
                         (p4_to_rxdma.flow_dport) |
                         (p4_to_rxdma.flow_proto << 16));

            // Setup root address of DPORT lookup
            modify_field(lpm_metadata.lpm2_base_addr,
                         lpm_metadata.lpm2_base_addr +
                         SACL_PROTO_DPORT_TABLE_OFFSET);
            // Recirc the packet
            modify_field(capri_p4_intr.recirc_count, 1);
        }
    }
}

@pragma stage 7
table setup_recirc {
    actions {
        setup_recirc;
    }
}

control sacl {
    if (p4_to_rxdma.sacl_bypass == FALSE) {
        /* This assumes we're supporting
         * 256 sport ranges - 512 nodes in LPM tree, outputs 7 bit class-id
         *                    32*16 entries packed in 2 level LPM
         * 512 proto+dport ranges - 2k nodes in LPM tree, outputs 8 bit class-id
         *                          16*16*8 entres packed in 3 level LPM
         * 1k ipv4 - 2k nodes in LPM tree, outputs 10 bit class-id
         *           16*16*8 entries packed in 3 level LPM
         *
         * Phase 1:
         * Combine 7 bit sport-class-id and 10 bit ipv4-class-id, output
         * 10 bit class-id
         *
         * Phase2:
         * Combine 8 bit prot+dport-class-id and 10 bit phase 1 class-id to
         * yield 2 bit result
         */
        apply(rxlpm1_0);
        apply(rxlpm1_1);
        apply(rxlpm2_0);
        apply(rxlpm1_2);
        apply(rxlpm2_1);
        apply(rxlpm1_3);
        apply(rxlpm2_2);
        apply(rxlpm1_4);
        apply(rxlpm2_3);
        apply(rxlpm1_5);
        apply(rxlpm2_4);
        apply(rxlpm1_6);
        apply(rxlpm2);
        apply(rxlpm1);
        apply(setup_recirc);
    }
}
