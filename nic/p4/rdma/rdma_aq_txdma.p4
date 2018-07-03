/***********************************************************************/
/* rdma_aq_txdma.p4 */
/***********************************************************************/

#include "../common-p4+/common_txdma_dummy.p4"

/**** table declarations ****/

#define tx_table_s0_t0 aq_tx_s0_t0
#define tx_table_s0_t1 aq_tx_s0_t1
#define tx_table_s0_t2 aq_tx_s0_t2
#define tx_table_s0_t3 aq_tx_s0_t3

#define tx_table_s1_t0 aq_tx_s1_t0
#define tx_table_s1_t1 aq_tx_s1_t1
#define tx_table_s1_t2 aq_tx_s1_t2
#define tx_table_s1_t3 aq_tx_s1_t3

#define tx_table_s2_t0 aq_tx_s2_t0
#define tx_table_s2_t1 aq_tx_s2_t1
#define tx_table_s2_t2 aq_tx_s2_t2
#define tx_table_s2_t3 aq_tx_s2_t3

#define tx_table_s3_t0 aq_tx_s3_t0
#define tx_table_s3_t1 aq_tx_s3_t1
#define tx_table_s3_t2 aq_tx_s3_t2
#define tx_table_s3_t3 aq_tx_s3_t3

#define tx_table_s4_t0 aq_tx_s4_t0
#define tx_table_s4_t1 aq_tx_s4_t1
#define tx_table_s4_t2 aq_tx_s4_t2
#define tx_table_s4_t3 aq_tx_s4_t3

#define tx_table_s5_t0 aq_tx_s5_t0
#define tx_table_s5_t1 aq_tx_s5_t1
#define tx_table_s5_t2 aq_tx_s5_t2
#define tx_table_s5_t3 aq_tx_s5_t3

#define tx_table_s6_t0 aq_tx_s6_t0
#define tx_table_s6_t1 aq_tx_s6_t1
#define tx_table_s6_t2 aq_tx_s6_t2
#define tx_table_s6_t3 aq_tx_s6_t3

#define tx_table_s7_t0 aq_tx_s7_t0
#define tx_table_s7_t1 aq_tx_s7_t1
#define tx_table_s7_t2 aq_tx_s7_t2
#define tx_table_s7_t3 aq_tx_s7_t3


#define tx_table_s0_t0_action aq_tx_aqcb_process

//#define tx_table_s1_t0_action aq_tx_aqwqe_process

#include "../common-p4+/common_txdma.p4"
#include "./rdma_txdma_headers.p4"

/**** Macros ****/

#define GENERATE_GLOBAL_K \
    modify_field(phv_global_common_scr.lif, phv_global_common.lif);\
    modify_field(phv_global_common_scr.qid, phv_global_common.qid);\
    modify_field(phv_global_common_scr.qtype, phv_global_common.qtype);\
    modify_field(phv_global_common_scr.cb_addr, phv_global_common.cb_addr);\
    modify_field(phv_global_common_scr.pad, phv_global_common.pad);\

/**** header definitions ****/

header_type phv_global_common_t {
    fields {
        lif                              :   11;
        qid                              :   24;
        qtype                            :    3;
        cb_addr                          :   25;
        pad                              :   51;
    }
}



/**** global header unions ****/

@pragma pa_header_union ingress common_global
metadata phv_global_common_t phv_global_common;
@pragma scratch_metadata
metadata phv_global_common_t phv_global_common_scr;

/**** to stage header unions ****/

//To-Stage-0

//To-Stage-1

//To-Stage-2

//To-Stage-3

//To-Stage-4

//To-Stage-5

//To-Stage-6

//To-Stage-7

/**** stage to stage header unions ****/

//Table-0

//Table-1

//Table-2

//Table-3

/*
 * Stage 0 table 0 recirc action
 */
action aq_tx_aqcb_process () {
    // from ki global
    GENERATE_GLOBAL_K

    // to stage

    // stage to stage
}


