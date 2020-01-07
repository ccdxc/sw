// Common PHV instantiation
@pragma dont_trim
metadata cap_phv_intr_global_t capri_intr;
@pragma dont_trim
metadata cap_phv_intr_p4_t capri_p4_intr;
@pragma dont_trim
metadata cap_phv_intr_rxdma_t capri_rxdma_intr;

@pragma dont_trim
@pragma pa_header_union ingress capri_intr
metadata cap_phv_intr_global_t p4_intr_global;
@pragma dont_trim
@pragma pa_header_union ingress capri_p4_intr
metadata cap_phv_intr_p4_t p4_intr;
@pragma dont_trim
@pragma pa_header_union ingress capri_rxdma_intr
metadata cap_phv_intr_rxdma_t p4_rxdma_intr;

@pragma dont_trim
metadata p4_2_p4plus_app_header_t app_header;
@pragma dont_trim
metadata p4_2_p4plus_ext_app_header_t ext_app_header;

@pragma dont_trim
@pragma pa_header_union ingress ext_app_header
metadata predicate_header_t predicate_header;

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

header_type rxdma_common_pad_t {
    fields {
        rxdma_common_pad : 96;
    }
}

@pragma dont_trim
metadata rxdma_common_pad_t     rxdma_common_pad;
