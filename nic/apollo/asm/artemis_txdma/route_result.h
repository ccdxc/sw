route_result_handler:
    seq             c1, res_reg[ROUTE_RESULT_TYPE_PEER_VPC_BIT_POS:\
                                ROUTE_RESULT_TYPE_PEER_VPC_BIT_POS], 1
    bcf             [c1], route_result_vpc
    seq             c1, res_reg[ROUTE_RESULT_TYPE_SVC_TUNNEL_BIT_POS:\
                                ROUTE_RESULT_TYPE_SVC_TUNNEL_BIT_POS], 1
    bcf             [c1], route_result_st
    nop
route_result_nexthop_group:
    phvwr.e         p.txdma_control_nexthop_group_index, res_reg[9:0]
    nop
route_result_vpc:
    phvwr.e         p.txdma_control_vpc_id, res_reg[9:0]
    nop
route_result_st:
    phvwr.e         p.txdma_control_svc_id, res_reg[9:0]
    nop
