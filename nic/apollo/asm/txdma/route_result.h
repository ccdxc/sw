res_handler:
    seq             c1, res_reg[ROUTE_RESULT_BIT_POS:ROUTE_RESULT_BIT_POS], \
                        ROUTE_RESULT_TYPE_VPC
    bcf             [c1], route_result_vpc
    phvwr           p.app_header_table0_valid, FALSE       // LPM bypass
route_result_nexthop:
    phvwr.e         p.txdma_to_p4e_header_nexthop_index, res_reg
    phvwr           p.txdma_to_p4e_header_vpc_id, k.p4_to_txdma_header_vpc_id
route_result_vpc:
    phvwr.e         p.txdma_to_p4e_header_vpc_id, res_reg
    nop
