route_result_handler:
    // Disable route LPM lookup in the subsequent recircs
    phvwr          p.txdma_predicate_lpm1_enable, FALSE
    //Write the result of the route LPM to PHV
    phvwr           p.txdma_to_p4e_meter_id, res_reg[ROUTE_RESULT_METERID_END_BIT:\
                                                     ROUTE_RESULT_METERID_START_BIT]
    phvwr.e         p.txdma_to_p4e_nexthop_id, res_reg[ROUTE_RESULT_NEXTHOP_END_BIT:\
                                                       ROUTE_RESULT_NEXTHOP_START_BIT]
    phvwr           p.txdma_to_p4e_nexthop_type, res_reg[ROUTE_RESULT_NHTYPE_END_BIT:\
                                                         ROUTE_RESULT_NHTYPE_START_BIT]
