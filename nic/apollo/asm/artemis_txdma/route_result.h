route_result_handler:
    seq             c1, res_reg[ROUTE_RESULT_BIT_POS:ROUTE_RESULT_BIT_POS], \
                        ROUTE_RESULT_TYPE_VPC
    bcf             [c1], route_result_vpc
    phvwr           p.app_header_table0_valid, FALSE       // LPM bypass
route_result_nexthop_group:
    phvwr.e         p.txdma_control_nexthop_group_index, res_reg
    phvwr           p.txdma_control_vpc_id, k.{txdma_control_vpc_id_sbit0_ebit1...\
                                               txdma_control_vpc_id_sbit2_ebit9}
route_result_vpc:
    phvwr.e         p.txdma_control_vpc_id, res_reg
    nop
