/******************************************************************************
 * TxDMA stage 0 LIF params table jump vector                                 *
 *****************************************************************************/

%%
    .param      tx_stage0_load_rdma_params

tx_stage0_lif_rdma_params:
    j           tx_stage0_load_rdma_params
    nop

.align
tx_stage0_lif_ipfix_params:
    nop.e
    nop
