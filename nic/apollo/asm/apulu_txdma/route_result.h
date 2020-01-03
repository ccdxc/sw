route_result_handler:
    // Disable route LPM lookup in the subsequent stages
    phvwr          p.txdma_predicate_lpm1_enable, FALSE
    //Write the result of the route LPM to PHV
    phvwr           p.txdma_to_p4e_meter_en, res_reg[ROUTE_RESULT_METER_EN_END_BIT:\
                                                     ROUTE_RESULT_METER_EN_START_BIT]
    phvwr           p.txdma_to_p4e_snat_type, res_reg[ROUTE_RESULT_SNAT_TYPE_END_BIT:\
                                                      ROUTE_RESULT_SNAT_TYPE_START_BIT]
    phvwr           p.txdma_to_p4e_dnat_en, res_reg[ROUTE_RESULT_DNAT_EN_END_BIT:\
                                                    ROUTE_RESULT_DNAT_EN_START_BIT]
    // If DNAT is not enabled
    seq             c1, r0, res_reg[ROUTE_RESULT_DNAT_EN_END_BIT:ROUTE_RESULT_DNAT_EN_START_BIT]
    // Then copy NH Type and NH Id to txdma_to_p4e header
    phvwr.c1        p.txdma_to_p4e_nexthop_type, res_reg[ROUTE_RESULT_NHTYPE_END_BIT:\
                                                         ROUTE_RESULT_NHTYPE_START_BIT]
    phvwr.c1        p.txdma_to_p4e_nexthop_id, res_reg[ROUTE_RESULT_NEXTHOP_END_BIT:\
                                                       ROUTE_RESULT_NEXTHOP_START_BIT]
    // Else Enable DNAT Lookup and copy DNAT ID to txdma control metadata
    phvwr.!c1       p.txdma_control_dnat_en, res_reg[ROUTE_RESULT_DNAT_EN_END_BIT:\
                                                     ROUTE_RESULT_DNAT_EN_START_BIT]

    phvwr.!c1       p.txdma_to_p4e_dnat_idx, res_reg[ROUTE_RESULT_DNAT_IDX_END_BIT:\
                                                     ROUTE_RESULT_DNAT_IDX_START_BIT]
    nop.e
    nop
