// Common PHV instantiation
@pragma dont_trim
metadata cap_phv_intr_global_t capri_intr;
@pragma dont_trim
metadata cap_phv_intr_p4_t capri_p4_intr;
@pragma dont_trim
metadata cap_phv_intr_txdma_t capri_txdma_intr;

@pragma dont_trim
@pragma pa_header_union ingress capri_intr
metadata cap_phv_intr_global_t p4_intr_global;
@pragma dont_trim
@pragma pa_header_union ingress capri_p4_intr
metadata cap_phv_intr_p4_t p4_intr;
@pragma dont_trim
@pragma pa_header_union ingress capri_txdma_intr
metadata cap_phv_intr_txdma_t p4_txdma_intr;

@pragma dont_trim
metadata p4plus_2_p4_app_header_t app_header;

@pragma dont_trim
metadata p4plus_common_to_stage_t to_stage_0;
@pragma dont_trim
metadata p4plus_common_to_stage_t to_stage_1;
@pragma dont_trim
metadata p4plus_common_to_stage_t to_stage_2;
@pragma dont_trim
metadata p4plus_common_to_stage_t to_stage_3;
@pragma dont_trim
metadata p4plus_common_to_stage_t to_stage_4;
@pragma dont_trim
metadata p4plus_common_to_stage_t to_stage_5;
@pragma dont_trim
metadata p4plus_common_to_stage_t to_stage_6;
@pragma dont_trim
metadata p4plus_common_to_stage_t to_stage_7;

@pragma dont_trim
metadata p4plus_common_global_t common_global;
@pragma dont_trim
metadata p4plus_common_raw_table_engine_phv_t common_te0_phv;
@pragma dont_trim
metadata p4plus_common_s2s_t common_t0_s2s;
@pragma dont_trim
metadata p4plus_common_raw_table_engine_phv_t common_te1_phv;
@pragma dont_trim
metadata p4plus_common_s2s_t common_t1_s2s;
@pragma dont_trim
metadata p4plus_common_raw_table_engine_phv_t common_te2_phv;
@pragma dont_trim
metadata p4plus_common_s2s_t common_t2_s2s;
@pragma dont_trim
metadata p4plus_common_raw_table_engine_phv_t common_te3_phv;
@pragma dont_trim
metadata p4plus_common_s2s_t common_t3_s2s;

header_type txdma_common_pad_t {
    fields {
        txdma_common_pad : 96;
    }
}

@pragma dont_trim
metadata txdma_common_pad_t txdma_common_pad;
