/*****************************************************************************/
/* DNAT Table                                                     */
/*****************************************************************************/

action dnat(dnat_address, route_table_hw_id)
{
    // Copy the dnat_address to txdma to p4e header
    modify_field(txdma_to_p4e.dnat_ip, dnat_address);
    modify_field(scratch_metadata.field10, route_table_hw_id);
    // Setup Route LPM to lookup the dnat address
    modify_field(txdma_control.lpm1_key, dnat_address);
}

@pragma stage 1
@pragma hbm_table
@pragma index_table
table dnat {
    reads {
        txdma_to_p4e.dnat_idx : exact;
    }

    actions {
        dnat;
    }

    size : DNAT_TABLE_SIZE;
}

control dnat {
    apply(dnat);
}
