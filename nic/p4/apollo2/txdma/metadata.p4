#include "../../common-p4+/capri_dma_cmd.p4"
#include "../../common-p4+/capri_doorbell.p4"

header_type txdma_control_metadata_t {
    fields {
        pad0            : 6;
        control_addr    : 34;
        payload_addr    : 40;
        cindex          : 16;
    }
}

header_type scratch_metadata_t {
    fields {
        lpm_data        : 15;
        data512         : 512;
        payload_addr    : 40;
        payload_len     : 14;
        qid             : 24;
    }
}

// Phv header instantiation -
@pragma dont_trim
metadata cap_phv_intr_global_t capri_intr;
@pragma dont_trim
metadata cap_phv_intr_p4_t capri_p4_intr;
@pragma dont_trim
metadata cap_phv_intr_txdma_t capri_txdma_intr;
@pragma dont_trim
metadata arm_to_txdma_header_t arm_to_txdma_header;
@pragma dont_trim
metadata predicate_header_t predicate_header;
@pragma dont_trim
metadata p4_to_txdma_header_t p4_to_txdma_header;
@pragma dont_trim
metadata txdma_to_p4e_header_t txdma_to_p4e_header;
@pragma dont_trim
metadata txdma_to_p4i_header_t txdma_to_p4i_header;

@pragma dont_trim
metadata txdma_control_metadata_t txdma_control;

@pragma dont_trim
metadata doorbell_data_t    doorbell_data;

@pragma pa_align 128
@pragma dont_trim
metadata dma_cmd_phv2pkt_t intrinsic_dma;    // dma cmd 1
@pragma dont_trim
metadata dma_cmd_phv2pkt_t header_dma;  // dma cmd 2
@pragma dont_trim
metadata dma_cmd_mem2pkt_t payload_dma;   // dma cmd 3
@pragma dont_trim
metadata dma_cmd_phv2mem_t ci_update;  // dma cmd 4


// Scratch metadata
@pragma dont_trim
@pragma scratch_metadata
metadata scratch_metadata_t     scratch_metadata;

@pragma dont_trim
@pragma scratch_metadata
metadata qstate_hdr_t           scratch_qstate_hdr;

@pragma dont_trim
@pragma scratch_metadata
metadata qstate_txdma_fte_Q_t   scratch_qstate_txdma_fte_Q;

