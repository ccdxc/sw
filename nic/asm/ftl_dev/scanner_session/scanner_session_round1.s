#include "ftl_dev_shared_asm.h"

struct phv_                             p;
struct s3_tbl_k                         k;
struct s3_tbl_session_round1_session0_d d;

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

%%
    .param          session_round2_session0
    .param          session_round2_session1
    .param          session_round2_session2
    .param          session_round2_session3
    .param          session_summarize
     SESSION_METRICS_PARAMS()
    .align

session_round1_session0:
    //CLEAR_TABLE0
    SESSION_KIVEC_AGE_TMO_PROPAGATE(4, 3)  
    SCANNER_DEBUG_FORCE_SESSION_EXPIRED_TS()
    SESSION_ROUND_EXEC_e(0, SESSION_ROUND2,
                         session_round2_session0, _session0_label0,
                         _session0_label1, _session0_label2,
                         SESSION_NO_MORE_SESSION0_ALT(),
                         SESSION_EXPIRY_CHECK_KIVEC3_e(round1_session0))

    .align

session_round1_session1:
    CLEAR_TABLE1
    SCANNER_DEBUG_FORCE_SESSION_EXPIRED_TS()
    SESSION_ROUND_EXEC_e(1, SESSION_ROUND2,
                         session_round2_session1, _session1_label0,
                         _session1_label1, _session1_label2,
                         SESSION_NO_MORE_SESSION1_2_ALT(),
                         SESSION_EXPIRY_CHECK_KIVEC3_e(round1_session1))
    
    .align

session_round1_session2:
    CLEAR_TABLE2
    SCANNER_DEBUG_FORCE_SESSION_EXPIRED_TS()
    SESSION_ROUND_EXEC_e(2, SESSION_ROUND2,
                         session_round2_session2, _session2_label0,
                         _session2_label1, _session2_label2,
                         SESSION_NO_MORE_SESSION1_2_ALT(),
                         SESSION_EXPIRY_CHECK_KIVEC3_e(round1_session2))

    .align

session_round1_session3:
    //CLEAR_TABLE3
    SCANNER_DEBUG_FORCE_SESSION_EXPIRED_TS()
    SESSION_ROUND_EXEC_e(3, SESSION_ROUND2,
                         session_round2_session3, _session3_label0,
                         _session3_label1, _session3_label2,
                         SESSION_NO_MORE_SESSION3_ALT(),
                         SESSION_EXPIRY_CHECK_KIVEC3_e(round1_session3))

