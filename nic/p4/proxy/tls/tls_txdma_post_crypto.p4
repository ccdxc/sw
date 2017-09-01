/* TLS P4 definitions */


#include "../../common-p4+/common_txdma_dummy.p4"

#define tx_table_s0_t0_action       read_tls_stg0

#define tx_table_s1_t0_action       tls_rx_bsq

#define tx_table_s2_t0_action       tls_read_desc


#define tx_table_s3_t0_action       tls_bsq_consume
#define tx_table_s3_t1_action       read_tnmdr_free_pi
#define tx_table_s3_t2_action       read_tnmpr_free_pi

#define tx_table_s4_t0_action       tls_queue_sesq


#include "../../common-p4+/common_txdma.p4"
#include "tls_txdma_common.p4"

/* Per stage D-vector Definitions */

// d for stage 2 table 1
header_type read_tnmdr_free_pi_d_t {
    fields {
        tnmdr_free_pi              : 16;
    }
}

// d for stage 2 table 2
header_type read_tnmpr_free_pi_d_t {
    fields {
        tnmpr_free_pi              : 16;
    }
}


#define GENERATE_GLOBAL_K                                                                               \
        modify_field(tls_global_phv_scratch.fid, tls_global_phv.fid);                                   \
        modify_field(tls_global_phv_scratch.dec_flow, tls_global_phv.dec_flow);                         \
        modify_field(tls_global_phv_scratch.qstate_addr, tls_global_phv.qstate_addr);


/* Global PHV definition */
header_type tls_global_phv_t {
    fields {
        fid                             : 16;
        dec_flow                        : 8;
        qstate_addr                     : HBM_ADDRESS_WIDTH;
    }
}


header_type to_stage_3_phv_t {
    fields {
        bsq_ci                          : 16;
        idesc                           : HBM_ADDRESS_WIDTH;
        ipage                           : HBM_ADDRESS_WIDTH;
    }
}

header_type to_stage_4_phv_t {
    fields {
        odesc                           : ADDRESS_WIDTH;
        debug_dol                       : 8;
    }
}

header_type doorbell_data_pad_t {
    fields {
        pad                             : 384;
    }
}

@pragma scratch_metadata
metadata tlscb_0_t tlscb_0_d;

@pragma scratch_metadata
metadata tlscb_1_t tlscb_1_d;

@pragma scratch_metadata
metadata barco_result_t read_desc_d;

@pragma scratch_metadata
metadata tls_global_phv_t tls_global_phv_scratch;

@pragma scratch_metadata
metadata read_tnmdr_free_pi_d_t read_tnmdr_free_d;

@pragma scratch_metadata
metadata read_tnmpr_free_pi_d_t read_tnmpr_free_d;

@pragma scratch_metadata
metadata to_stage_3_phv_t to_s3_scratch;

@pragma scratch_metadata
metadata to_stage_4_phv_t to_s4_scratch;


@pragma pa_header_union ingress to_stage_3
metadata to_stage_3_phv_t to_s3;

@pragma pa_header_union ingress to_stage_4
metadata to_stage_4_phv_t to_s4;


@pragma pa_header_union ingress common_global
metadata tls_global_phv_t tls_global_phv;



@pragma dont_trim
metadata barco_result_t read_desc;
@pragma dont_trim
metadata ring_entry_t ring_entry;
@pragma dont_trim
metadata doorbell_data_raw_t db_data;
@pragma dont_trim
metadata doorbell_data_pad_t db_pad;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd0;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd1;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd2;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd3;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd4;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd5;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd6;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd7;






/* Stage 1 table 0 action */
action tls_rx_bsq(TLSCB_1_PARAMS) {

    GENERATE_GLOBAL_K

    GENERATE_TLSCB_1_D
}

/*
 * Stage 2 table 0 action
 */
action tls_read_desc(status, output_list_address, pad) {
    GENERATE_GLOBAL_K

    // d for stage 2 table 0 read_desc
    modify_field(read_desc_d.status, status);
    modify_field(read_desc_d.output_list_address, output_list_address);
    modify_field(read_desc_d.pad, pad);

}

/* Stage 3 table 0 action */
action tls_bsq_consume(TLSCB_0_PARAMS) {

    GENERATE_GLOBAL_K

    /* To Stage 3 fields */
    modify_field(to_s3_scratch.bsq_ci, to_s3.bsq_ci);


    /* D vector */
    GENERATE_TLSCB_0_D

}

/*
 * Stage 3 table 1 action
 */
action read_tnmdr_free_pi(tnmdr_free_pi) {
    GENERATE_GLOBAL_K

    /* To Stage 3 fields */
    modify_field(to_s3_scratch.idesc, to_s3.idesc);

    // d for stage 3 table 1 
    modify_field(read_tnmdr_free_d.tnmdr_free_pi, tnmdr_free_pi);
}

/*
 * Stage 3 table 2 action
 */
action read_tnmpr_free_pi(tnmpr_free_pi) {
    GENERATE_GLOBAL_K

    /* To Stage 3 fields */
    modify_field(to_s3_scratch.ipage, to_s3.ipage);

    // d for stage 2 table 2 
    modify_field(read_tnmpr_free_d.tnmpr_free_pi, tnmpr_free_pi);
}



/* Stage 4 action */
action tls_queue_sesq(TLSCB_0_PARAMS) {

    GENERATE_GLOBAL_K


    /* To Stage 4 fields */
    modify_field(to_s4_scratch.odesc, to_s4.odesc);
    modify_field(to_s4_scratch.debug_dol, to_s4.debug_dol);


    GENERATE_TLSCB_0_D
}

