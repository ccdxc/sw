/*****************************************************************************
 * dummy.p4: This defines dummy P4 action handlers
 *****************************************************************************/

#include "scratch.h"

#if 1
action exit() {
}
#endif


action dummy(dummy_field) {
  modify_field(storage_scratch.dummy_field, dummy_field);
}

#define tx_table_s0_t1_action dummy
#define tx_table_s0_t2_action dummy
#define tx_table_s0_t3_action dummy
#define tx_table_s1_t1_action dummy
#define tx_table_s1_t2_action dummy
#define tx_table_s1_t3_action dummy
#define tx_table_s2_t1_action dummy
#define tx_table_s2_t2_action dummy
#define tx_table_s2_t3_action dummy
#define tx_table_s3_t1_action dummy
#define tx_table_s3_t2_action dummy
#define tx_table_s3_t3_action dummy
#define tx_table_s4_t1_action dummy
#define tx_table_s4_t2_action dummy
#define tx_table_s4_t3_action dummy
#define tx_table_s5_t1_action dummy
#define tx_table_s5_t2_action dummy
#define tx_table_s5_t3_action dummy
#define tx_table_s6_t1_action dummy
#define tx_table_s6_t2_action dummy
#define tx_table_s6_t3_action dummy
#define tx_table_s7_t1_action dummy
#define tx_table_s7_t2_action dummy
#define tx_table_s7_t3_action dummy
