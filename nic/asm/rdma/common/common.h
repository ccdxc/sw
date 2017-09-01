

struct tx_stage0_load_rdma_params_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    dummy : S2S_DATA_WIDTH;
    struct phv_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct rx_stage0_load_rdma_params_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    dummy : S2S_DATA_WIDTH;
    struct phv_to_stage_t to_stage;
    struct phv_global_common_t global;
};

