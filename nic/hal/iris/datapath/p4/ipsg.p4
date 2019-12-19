/*****************************************************************************/
/* IP source guard feature                                                   */
/*****************************************************************************/
// Requirements:
// 1. Check for ipsg_enable
// 2. Support V4 and V6
// 3. Support one MAC Multiple IPa (meaning secondary IPs)s
// 4. For L2 Packets we should skip the check and let pacets go.
// 5. For Uplinks don't care irrespective of the ipsg_enable
//    setting from input_properties
// 6. For recirculated packets we shouldn't drop as tm_iport changes.
//    Assumption here is packet should have been dropped in 1st pass.
// New key for this table will be
// <tm_iport = DMA, VRF, IP (128 bit)> --> LIF, MAC and VLAN.
// We can't rely on tm_iport as reliable source as with recirc it will
// be changed.
// we will rely on ipsg_enable being set correctly for uplink which is
// never being enable.

action ipsg_miss() {
    if ((flow_lkp_metadata.lkp_type == FLOW_KEY_LOOKUP_TYPE_IPV4) or
        (flow_lkp_metadata.lkp_type == FLOW_KEY_LOOKUP_TYPE_IPV6)) {
        modify_field(control_metadata.drop_reason, DROP_IPSG);
    }
}

// We are doing the IPSG check against the src_lif instead of src_lport
// because we dervie lport from mac_vlan table with mac and vlan as a key
// and comparing that with HAL programmed lport will only catch the bugs
// of HAL but not really test the spoofing cases. So we will validate
// against the LIF that comes in capri_intrinsic. HAL programming will
// pick the LIF associated with the lport when programming this table.
// This way when multiple LIFs are used towards the workloads we can 
// prevent spoofing of (mac, ip) from workloads which are mapped to a 
// different LIF.
action ipsg_hit(src_lif, mac, vlan_valid, vlan_id) {
    if (control_metadata.ipsg_enable == FALSE) {
        // return;
    }
    if ((control_metadata.src_lif != src_lif) or
        (ethernet.srcAddr != mac) or
        (vlan_tag.valid != vlan_valid) or
        (vlan_tag.vid != vlan_id)) {
        modify_field(control_metadata.drop_reason, DROP_IPSG);
        drop_packet();
    }

    // dummy ops to keep compiler happy
    modify_field(scratch_metadata.lif, src_lif);
    modify_field(scratch_metadata.mac, mac);
    modify_field(scratch_metadata.vlan_valid, vlan_valid);
    modify_field(scratch_metadata.vlan_id, vlan_id);
}

@pragma stage 5
table ipsg {
    reads {
        entry_inactive.ipsg         : ternary;
        flow_lkp_metadata.lkp_type  : ternary;
        flow_lkp_metadata.lkp_vrf   : ternary;
        flow_lkp_metadata.lkp_src   : ternary;
    }
    actions {
        ipsg_hit;
        ipsg_miss;
    }
    default_action : ipsg_miss;
    size : IPSG_TABLE_SIZE;
}

control process_ipsg {
    // Moved ipsg_en into action as lookup is going through even if its disabled.
    // Looks like some issue in predicate tcam programming
    // if (control_metadata.nic_mode == NIC_MODE_SMART) {
    //     apply(ipsg);
    // }
    apply(ipsg);
}
