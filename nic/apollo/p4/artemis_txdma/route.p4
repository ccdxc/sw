/*****************************************************************************/
/* IPv4 Route LPM lookup                                                     */
/*****************************************************************************/

#include "txlpm1.p4"

action route_res_handler() {
    if ((scratch_metadata.field16 >> ROUTE_RESULT_BIT_POS) == ROUTE_RESULT_TYPE_VPC) {
        // Set VPCID in PHV from LPM result
        modify_field(txdma_control.vpc_id, scratch_metadata.field16 & 0x7FFF);
    } else {
        // Set NHID in PHV from LPM result
        modify_field(txdma_control.nexthop_group_index, scratch_metadata.field16);
        modify_field(txdma_control.vpc_id, txdma_control.vpc_id);
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
