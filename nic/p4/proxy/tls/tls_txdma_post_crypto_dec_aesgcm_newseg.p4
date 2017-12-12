/* TLS TXDMA Post-Crypto Decryption RX Reassembly - P4 definitions */

#include "../../common-p4+/common_txdma_dummy.p4"

#define tx_table_s0_t0_action       read_tls_stg0

#define tx_table_s1_t0_action       get_tls_record_ref
#
#define tx_table_s2_t0_action       barco_result_read


#include "../../common-p4+/common_txdma.p4"
#include "../../cpu-p4+/cpu_rx_common.p4"
#include "tls_txdma_common.p4"




/* Global PHV definition */
/* Max size: 128 bits */
header_type tls_global_phv_t {
    fields {
        fid                             : 16;
        qstate_addr                     : HBM_ADDRESS_WIDTH;
        barco_op_failed                 : 1;
        /* Used: 33b, Pending: 95b */
    }
}
#define GENERATE_GLOBAL_K                                                                       \
        modify_field(tls_global_phv_scratch.fid, tls_global_phv.fid);           \
        modify_field(tls_global_phv_scratch.qstate_addr, tls_global_phv.qstate_addr);           \
        modify_field(tls_global_phv_scratch.barco_op_failed, tls_global_phv.barco_op_failed);

@pragma scratch_metadata
metadata tls_global_phv_t tls_global_phv_scratch;

@pragma pa_header_union ingress common_global
metadata tls_global_phv_t tls_global_phv;

@pragma scratch_metadata
metadata tlscb_0_t tlscb_0_d;

/* Stage 1 table 0 action */
@pragma scratch_metadata
metadata tlscb_records_state_t    TLSCB_RECORDS_STATE_SCRATCH;

action get_tls_record_ref(TLSCB_RECORDS_STATE_PARAMS) {

    GENERATE_GLOBAL_K

    GENERATE_TLSCB_RECORDS_STATE
}

/* Stage 2 table 0 action */
@pragma scratch_metada
metadata    barco_result_t BARCO_RESULT_SCRATCH;

action barco_result_read(BARCO_RESULT_PARAMS) {
    GENERATE_GLOBAL_K

    GENERATE_BARCO_RESULT_D
}
