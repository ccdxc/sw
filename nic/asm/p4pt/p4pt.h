#ifndef __P4PT_H
#define __P4PT_H

#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "../p4/iris/include/defines.h"
#include "../rdma/common/include/capri.h"


// sets table0 valid; to be cautious set other tables invalid
#define P4PT_SET_TABLE0_VALID 			\
    phvwr       p.app_header_table0_valid, 1; 	\
    phvwr       p.app_header_table1_valid, 0; 	\
    phvwr       p.app_header_table2_valid, 0; 	\
    phvwr.e     p.app_header_table3_valid, 0; 	\
    nop

// clear all tables
#define P4PT_CLEAR_ALL_TABLES 			\
    phvwr       p.app_header_table0_valid, 0; 	\
    phvwr       p.app_header_table1_valid, 0; 	\
    phvwr       p.app_header_table2_valid, 0; 	\
    phvwr.e     p.app_header_table3_valid, 0; 	\
    nop

// set sets exit bit in the global phv that is used as pipeline bail out indication
#define P4PT_EXIT 				\
    phvwr       p.p4pt_global_exit, 1; 		\
    phvwr       p.p4_intr_global_drop,1;        \
    P4PT_CLEAR_ALL_TABLES

// if exit is set then all tables are unset and pipeline ends
#define P4PT_CHECK_EXIT                       	\
    seq        c1, k.p4pt_global_exit, 1;     	\
    nop.e.c1;                                 	\
    nop
   
// iscsi latency ranges: 1,8,64,128 us, 1,2,4,8,16,32,64,128 ms, 1,2,4,8 s
#define P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE15_LOW 8000000
#define P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE14_LOW 4000000
#define P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE13_LOW 2000000
#define P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE12_LOW 1000000
#define P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE11_LOW 128000
#define P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE10_LOW 64000
#define P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE9_LOW  32000
#define P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE8_LOW  16000
#define P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE7_LOW  8000
#define P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE6_LOW  4000
#define P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE5_LOW  2000
#define P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE4_LOW  1000
#define P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE3_LOW  128
#define P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE2_LOW  64
#define P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE1_LOW  8
#define P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE0_LOW  1


#endif //__P4PT_H
