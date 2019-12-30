#include "session_age_common.h"

struct phv_                             p;
struct s5_tbl_k                         k;
struct s5_tbl_age_round3_session0_d     d;

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
#define r_timestamp_ms                  r4      // must use r4 (see above comments)
#define r_session_info_addr             r5
#define r_lkp_type                      r6

/*
 * Common code macros to be used per session in this stage.
 */
#define AGE_EXPIRY_CHECK_KIVEC5_e(_expiry_bit)                                  \
    AGE_EXPIRY_CHECK_ALL_LKUP_TYPES_e(_expiry_bit,                              \
                                      AGE_KIVEC5_ICMP_AGE_MS,                   \
                                      AGE_KIVEC5_UDP_AGE_MS,                    \
                                      AGE_KIVEC5_TCP_AGE_MS,                    \
                                      AGE_KIVEC5_OTHER_AGE_MS)                  \

/*
 * NOTE: This code macro is deliberately written with a gap between the call
 * to AGE_MS_CALC() and the one to AGE_EXPIRY_CHECK_KIVEC() to reduce MPU
 * stall in referencing the result of the divide instruction.
 */
#define AGE_SESSION_ROUND3_EXEC_e(_expiry_bit, _inner_label0,                   \
                                  _no_more_session_alt)                         \
    seq         c2, d.valid_flag, r0;                                           \
    bcf         [c2], _inner_label0;                                            \
    AGE_MS_CALC(d.timestamp)                                                    \
_inner_label0:;                                                                 \
    _no_more_session_alt                                                        \
    nop.c2.e;                                                                   \
    add         r_lkp_type, d.flow_type, r0;                                    \
    AGE_EXPIRY_CHECK_KIVEC5_e(_expiry_bit)                                      \
    
%%
    .param          age_summarize
     AGE_METRICS_PARAMS()
    .align

age_round3_session0:
    //CLEAR_TABLE0
    AGE_SESSION_ROUND3_EXEC_e(round3_session0, _session0_label0,
                              AGE_SESSION_NO_MORE_SESSION0_ALT())


    .align

age_round3_session1:
    CLEAR_TABLE1
    AGE_SESSION_ROUND3_EXEC_e(round3_session1, _session1_label0,
                              AGE_SESSION_NO_MORE_SESSION1_2_ALT())
    
    .align

age_round3_session2:
    CLEAR_TABLE2
    AGE_SESSION_ROUND3_EXEC_e(round3_session2, _session2_label0,
                              AGE_SESSION_NO_MORE_SESSION1_2_ALT())
                                  
    .align

age_round3_session3:
    //CLEAR_TABLE3
    AGE_SESSION_ROUND3_EXEC_e(round3_session3, _session3_label0,
                              AGE_SESSION_NO_MORE_SESSION3_ALT())

