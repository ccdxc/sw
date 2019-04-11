res_handler:
    seq             c1, res_reg[ROUTE_RESULT_BIT_POS:ROUTE_RESULT_BIT_POS], \
                        ROUTE_RESULT_TYPE_VCN
    bcf             [c1], route_result_vcn
    phvwr           p.app_header_table0_valid, FALSE       // LPM bypass
route_result_nexthop:
    phvwr.e         p.txdma_to_p4e_header_nexthop_index, res_reg
    phvwr           p.txdma_to_p4e_header_vcn_id, k.p4_to_txdma_header_vcn_id
route_result_vcn:
    phvwr.e         p.txdma_to_p4e_header_vcn_id, res_reg
    nop
