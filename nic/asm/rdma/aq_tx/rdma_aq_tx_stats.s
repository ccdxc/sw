#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "common_phv.h"

struct aq_tx_phv_t p;
struct aq_tx_s7_t3_k k;
struct aqcb1_t d;

#define IN_P    to_s7_fb_stats_info

#define MASK_16 16

%%

.align
rdma_aq_tx_stats_process:

    tbladd             d.num_any, 1

    //flags nop, create_cq, create_qp, reg_mr, stats_hdrs, stats_vals, dereg_mr
    crestore         [c7, c6, c5, c4, c3, c2, c1], CAPRI_KEY_RANGE(IN_P, nop, dereg_mr), 0x7F

    tblmincri.c7.e     d.num_nop, MASK_16, 1
    tblmincri.c6.e     d.num_create_cq, MASK_16, 1
    tblmincri.c5.e     d.num_create_qp, MASK_16, 1
    tblmincri.c4.e     d.num_reg_mr, MASK_16, 1
    tblmincri.c3.e     d.num_stats_hdrs, MASK_16, 1
    tblmincri.c2.e     d.num_stats_vals, MASK_16, 1
    tblmincri.c1.e     d.num_dereg_mr, MASK_16, 1

    //flags resize_cq, destroy_cq, modify_qp, query_qp, destroy_qp, stats_dump
    crestore         [c7, c6, c5, c4, c3, c2], CAPRI_KEY_RANGE(IN_P, resize_cq, stats_dump), 0x6F

    tblmincri.c7.e     d.num_resize_cq, MASK_16, 1
    tblmincri.c6.e     d.num_destroy_cq, MASK_16, 1
    tblmincri.c5.e     d.num_modify_qp, MASK_16, 1
    tblmincri.c4.e     d.num_query_qp, MASK_16, 1
    tblmincri.c3.e     d.num_destroy_qp, MASK_16, 1
    tblmincri.c2.e     d.num_stats_dump, MASK_16, 1

exit:
    nop.e
    nop

