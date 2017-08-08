#define tx_table_s0_t0_action proxy_dummy 
#define tx_table_s0_t1_action proxy_dummy 
#define tx_table_s0_t2_action proxy_dummy 
#define tx_table_s0_t3_action proxy_dummy 

#define tx_table_s1_t0_action proxy_dummy 
#define tx_table_s1_t1_action proxy_dummy
#define tx_table_s1_t2_action proxy_dummy
#define tx_table_s1_t3_action proxy_dummy 

#define tx_table_s2_t0_action proxy_dummy
#define tx_table_s2_t1_action proxy_dummy
#define tx_table_s2_t2_action proxy_dummy
#define tx_table_s2_t3_action proxy_dummy

#define tx_table_s3_t0_action proxy_dummy
#define tx_table_s3_t1_action proxy_dummy
#define tx_table_s3_t2_action proxy_dummy
#define tx_table_s3_t3_action proxy_dummy

#define tx_table_s4_t0_action proxy_dummy
#define tx_table_s4_t1_action proxy_dummy
#define tx_table_s4_t2_action proxy_dummy
#define tx_table_s4_t3_action proxy_dummy

#define tx_table_s5_t0_action proxy_dummy
#define tx_table_s5_t1_action proxy_dummy
#define tx_table_s5_t2_action proxy_dummy
#define tx_table_s5_t3_action proxy_dummy

#define tx_table_s6_t0_action proxy_dummy
#define tx_table_s6_t1_action proxy_dummy
#define tx_table_s6_t2_action proxy_dummy
#define tx_table_s6_t3_action proxy_dummy

#define tx_table_s7_t0_action proxy_dummy
#define tx_table_s7_t1_action proxy_dummy
#define tx_table_s7_t2_action proxy_dummy
#define tx_table_s7_t3_action proxy_dummy


// just to avoid NCC assert : memory['num_buckets'] = capri_get_width_from_layout(table['layout']) / table['width']
action proxy_dummy (data0, data1)
{
    modify_field(scratch_metadata0.data0, data0);
    modify_field(scratch_metadata0.data1, data1);
}


#include "tls/tls-rx-serq.p4"
