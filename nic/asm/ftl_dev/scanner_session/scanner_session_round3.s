#include "ftl_dev_shared_asm.h"

struct phv_                             p;
struct s5_tbl_k                         k;
struct s5_tbl_session_round3_session0_d     d;

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
#define r_lkp_type                      r6

%%
    .param          session_summarize
     SESSION_METRICS_PARAMS()
    .align

session_round3_session0:
    //CLEAR_TABLE0
    SCANNER_DEBUG_FORCE_SESSION_EXPIRED_TS()
    SESSION_LAST_ROUND_EXEC_e(_session0_label0,
                              SESSION_NO_MORE_SESSION0_ALT(),
                              SESSION_EXPIRY_CHECK_KIVEC5_e(round3_session0))


    .align

session_round3_session1:
    CLEAR_TABLE1
    SCANNER_DEBUG_FORCE_SESSION_EXPIRED_TS()
    SESSION_LAST_ROUND_EXEC_e(_session1_label0,
                              SESSION_NO_MORE_SESSION1_2_ALT(),
                              SESSION_EXPIRY_CHECK_KIVEC5_e(round3_session1))
    
    .align

session_round3_session2:
    CLEAR_TABLE2
    SCANNER_DEBUG_FORCE_SESSION_EXPIRED_TS()
    SESSION_LAST_ROUND_EXEC_e(_session2_label0,
                              SESSION_NO_MORE_SESSION1_2_ALT(),
                              SESSION_EXPIRY_CHECK_KIVEC5_e(round3_session2))
                                  
    .align

session_round3_session3:
    //CLEAR_TABLE3
    SCANNER_DEBUG_FORCE_SESSION_EXPIRED_TS()
    SESSION_LAST_ROUND_EXEC_e(_session3_label0,
                              SESSION_NO_MORE_SESSION3_ALT(),
                              SESSION_EXPIRY_CHECK_KIVEC5_e(round3_session3))

