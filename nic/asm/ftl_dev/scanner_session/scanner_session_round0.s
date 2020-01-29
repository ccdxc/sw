#include "ftl_dev_shared_asm.h"

struct phv_                             p;
struct s2_tbl_k                         k;
struct s2_tbl_session_round0_session0_d d;

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

/*
 * Common code macros to be used per session in this stage.
 */
#define SESSION_EXPIRY_CHECK_KIVEC2_e(_expiry_bit)                              \
    SESSION_EXPIRY_CHECK_ALL_LKUP_TYPES_e(_expiry_bit,                          \
                                          SESSION_KIVEC2_ICMP_TMO,              \
                                          SESSION_KIVEC2_UDP_TMO,               \
                                          SESSION_KIVEC2_UDP_EST_TMO,           \
                                          SESSION_KIVEC2_TCP_SYN_TMO,           \
                                          SESSION_KIVEC2_TCP_EST_TMO,           \
                                          SESSION_KIVEC2_TCP_FIN_TMO,           \
                                          SESSION_KIVEC2_TCP_TIMEWAIT_TMO,      \
                                          SESSION_KIVEC2_TCP_RST_TMO,           \
                                          SESSION_KIVEC2_OTHER_TMO)             \
    
/*
 * NOTE: This code macro is deliberately written with a gap between the call
 * to SCANNER_MS_CALC() and the one to SESSION_EXPIRY_CHECK_KIVEC() to reduce MPU
 * stall in referencing the result of the divide instruction.
 */
#define SESSION_ROUND0_EXEC_e(_expiry_bit, _next_tbl, _next_round,              \
                              _next_pc, _inner_label0, _inner_label1,           \
                              _inner_label2, _no_more_session_alt)              \
    seq         c2, d.valid_flag, r0;                                           \
    bcf         [c2], _inner_label0;                                            \
    SCANNER_TS_CALC(d.timestamp)                                                \
_inner_label0:;                                                                 \
    SESSION_INFO_NUM_SCANNABLES_CHECK_CF(c3, _next_tbl, _next_round,            \
                                         SESSION_KIVEC0_NUM_SCANNABLES)         \
    SESSION_INFO_POSSIBLE_SCAN_CHECK_CF(c3, _next_tbl,                          \
                                        _next_pc, _inner_label1)                \
    b           _inner_label2;                                                  \
_inner_label1:;                                                                 \
    _no_more_session_alt                                                        \
_inner_label2:;                                                                 \
    nop.c2.e;                                                                   \
    add         r_lkp_type, d.flow_type, r0;                                    \
    SESSION_EXPIRY_CHECK_KIVEC2_e(_expiry_bit)                                  \


%%
    .param          session_round1_session0
    .param          session_round1_session1
    .param          session_round1_session2
    .param          session_round1_session3
    .param          session_summarize
     SESSION_METRICS_PARAMS()
    .align

session_round0_session0:
    //CLEAR_TABLE0
    SESSION_KIVEC_AGE_TMO_PROPAGATE(3, 2)
    SESSION_ROUND0_EXEC_e(round0_session0, 0, SESSION_ROUND1,
                          session_round1_session0, _session0_label0,
                          _session0_label1, _session0_label2,
                          SESSION_NO_MORE_SESSION0_ALT())

    .align

session_round0_session1:
    CLEAR_TABLE1
    SESSION_ROUND0_EXEC_e(round0_session1, 1, SESSION_ROUND1,
                          session_round1_session1,  _session1_label0,
                          _session1_label1, _session1_label2,
                          SESSION_NO_MORE_SESSION1_2_ALT())
    
    .align

session_round0_session2:
    CLEAR_TABLE2
    SESSION_ROUND0_EXEC_e(round0_session2, 2, SESSION_ROUND1,
                          session_round1_session2, _session2_label0,
                          _session2_label1, _session2_label2,
                          SESSION_NO_MORE_SESSION1_2_ALT())
                                  
    .align

session_round0_session3:
    //CLEAR_TABLE3
    SESSION_ROUND0_EXEC_e(round0_session3, 3, SESSION_ROUND1,
                          session_round1_session3, _session3_label0,
                          _session3_label1, _session3_label2,
                          SESSION_NO_MORE_SESSION3_ALT())

