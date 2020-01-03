/*****************************************************************************/
/* IPv4 Route LPM lookup                                                     */
/*****************************************************************************/

#include "txlpm1.p4"

action route_res_handler() {
    // Disable route LPM lookup in the subsequent stages
    modify_field(txdma_predicate.lpm1_enable, FALSE);
    //Write the result of the route LPM to PHV
    modify_field(txdma_to_p4e.meter_en,\
                 scratch_metadata.field32 >> ROUTE_RESULT_METER_EN_SHIFT);
    modify_field(txdma_to_p4e.snat_type,\
                 scratch_metadata.field32 >> ROUTE_RESULT_SNAT_TYPE_SHIFT);
    modify_field(txdma_to_p4e.dnat_en,\
                 scratch_metadata.field32 >> ROUTE_RESULT_DNAT_EN_SHIFT);

    if ((scratch_metadata.field32 & ROUTE_RESULT_DNAT_EN_MASK) != 0) {
        modify_field(txdma_to_p4e.dnat_idx,\
                     scratch_metadata.field32 >> ROUTE_RESULT_DNAT_IDX_SHIFT);
        modify_field(txdma_control.dnat_en, TRUE); // Enable DNAT lookup
    } else {
        modify_field(txdma_to_p4e.nexthop_type,\
                     scratch_metadata.field32 >> ROUTE_RESULT_NHTYPE_SHIFT);
        modify_field(txdma_to_p4e.nexthop_id,\
                     scratch_metadata.field32 >> ROUTE_RESULT_NEXTHOP_SHIFT);
    }
}

control route_lookup {
    apply(txlpm1_2);
    apply(txlpm1_3);
    apply(txlpm1_4);
    apply(txlpm1_5);
    apply(txlpm1_6);
    apply(txlpm1);
}
