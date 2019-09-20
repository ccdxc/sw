route_result_handler:
    seq             c1, res_reg[ROUTE_RESULT_TYPE_PEER_VPC_BIT_POS:\
                                ROUTE_RESULT_TYPE_PEER_VPC_BIT_POS], 1
    bcf             [c1], route_result_vpc
    seq             c1, res_reg[ROUTE_RESULT_TYPE_SVC_TUNNEL_BIT_POS:\
                                ROUTE_RESULT_TYPE_SVC_TUNNEL_BIT_POS], 1
    bcf             [c1], route_result_st
    seq             c1, res_reg[ROUTE_RESULT_TYPE_REM_SVC_TUNNEL_BIT_POS:\
                                ROUTE_RESULT_TYPE_REM_SVC_TUNNEL_BIT_POS], 1
    bcf             [c1], route_result_rem_st
    nop
route_result_nexthop_group:
    phvwr.e         p.txdma_to_p4e_nexthop_id, res_reg[19:0]
    // Disable Peer VNET Route lookup
    phvwr           p.txdma_predicate_lpm1_enable, FALSE

route_result_vpc:
    // Set Tx: Encap for NH_TYPE=VNET
    phvwr.e         p.rx_to_tx_hdr_vpc_id, res_reg[7:0]
    nop

route_result_st:
    phvwr           p.txdma_control_svc_id, res_reg[9:0]
    // Enable 4-6 lookup
    phvwr.e         p.txdma_control_st_enable, TRUE
    // Disable Peer VNET Route lookup
    phvwr           p.txdma_predicate_lpm1_enable, FALSE

route_result_rem_st:
    phvwr           p.txdma_control_svc_id, res_reg[9:0]
    // Enable 4-6 lookup
    phvwr.e         p.txdma_control_st_enable, TRUE
    // Disable Peer VNET Route lookup
    phvwr           p.txdma_predicate_lpm1_enable, FALSE
