#include "ftl_dev_shared_asm.h"

struct phv_                               p;
struct s2_tbl_k                           k;
struct s2_tbl_conntrack_round0_session0_d d;

/*
 * Relevant registers set by table engine on entry:
 * 
 * In addition, 
 * R1 = table lookup hash value
 * R2 = packet size
 * R3 = random number
 * R4 = current time
 */
 
/*
 * Registers usage
 */
#define r_stage                         r1
#define r_session_id                    r2
#define r_qstate_addr                   r3
#define r_timestamp                     r4      // must use r4 (see above comments)
#define r_session_info_addr             r5
#define r_flow_type                     r6
#define r_flow_state                    r7

%%
    .param          conntrack_round1_session0
    .param          conntrack_round1_session1
    .param          conntrack_round1_session2
    .param          conntrack_round1_session3
    .param          session_summarize
     SESSION_METRICS_PARAMS()
    .align

conntrack_round0_session0:
    //CLEAR_TABLE0
    SESSION_KIVEC_AGE_TMO_PROPAGATE(3, 2)
    SCANNER_DEBUG_FORCE_CONNTRACK_EXPIRED_TS()
    CONNTRACK_ROUND_EXEC_e(0, CONNTRACK_ROUND1,
                           conntrack_round1_session0, _conntrack0_label0,
                           _conntrack0_label1, _conntrack0_label2,
                           CONNTRACK_NO_MORE_SESSION0_ALT(),
                           CONNTRACK_EXPIRY_CHECK_KIVEC2_e(round0_session0))

    .align

conntrack_round0_session1:
    CLEAR_TABLE1
    SCANNER_DEBUG_FORCE_CONNTRACK_EXPIRED_TS()
    CONNTRACK_ROUND_EXEC_e(1, CONNTRACK_ROUND1,
                           conntrack_round1_session1,  _conntrack1_label0,
                           _conntrack1_label1, _conntrack1_label2,
                           CONNTRACK_NO_MORE_SESSION1_2_ALT(),
                           CONNTRACK_EXPIRY_CHECK_KIVEC2_e(round0_session1))
    
    .align

conntrack_round0_session2:
    CLEAR_TABLE2
    SCANNER_DEBUG_FORCE_CONNTRACK_EXPIRED_TS()
    CONNTRACK_ROUND_EXEC_e(2, CONNTRACK_ROUND1,
                           conntrack_round1_session2, _conntrack2_label0,
                           _conntrack2_label1, _conntrack2_label2,
                           CONNTRACK_NO_MORE_SESSION1_2_ALT(),
                           CONNTRACK_EXPIRY_CHECK_KIVEC2_e(round0_session2))
                                  
    .align

conntrack_round0_session3:
    //CLEAR_TABLE3
    SCANNER_DEBUG_FORCE_CONNTRACK_EXPIRED_TS()
    CONNTRACK_ROUND_EXEC_e(3, CONNTRACK_ROUND1,
                           conntrack_round1_session3, _conntrack3_label0,
                           _conntrack3_label1, _conntrack3_label2,
                           CONNTRACK_NO_MORE_SESSION3_ALT(),
                           CONNTRACK_EXPIRY_CHECK_KIVEC2_e(round0_session3))

