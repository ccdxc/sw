#ifndef __SESSION_AGE_COMMON_H
#define __SESSION_AGE_COMMON_H

#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "session_age_shared.h"

#define BITS_PER_BYTE                           8
#define SIZE_IN_BITS(bytes)                     ((bytes) * BITS_PER_BYTE)
#define SIZE_IN_BYTES(bits)                     ((bits) / BITS_PER_BYTE)

/*
 * Intrinsic fields
 */
#ifndef CAPRI_INTRINSIC_LIF
#define CAPRI_INTRINSIC_LIF                                                     \
        k.{p4_intr_global_lif_sbit0_ebit2...p4_intr_global_lif_sbit3_ebit10}
#endif

#ifndef CAPRI_TXDMA_INTRINSIC_QTYPE
#define CAPRI_TXDMA_INTRINSIC_QTYPE             k.{p4_txdma_intr_qtype}
#endif
#ifndef CAPRI_TXDMA_INTRINSIC_QID
#define CAPRI_TXDMA_INTRINSIC_QID               k.{p4_txdma_intr_qid}
#endif
#ifndef CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR
#define CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR                                       \
        k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}
#endif

/*
 * Bit selections in spr_mpuid register
 */
#ifndef CAPRI_SPR_MPUID_MPU_SELECT
#define CAPRI_SPR_MPUID_MPU_SELECT              1:0
#define CAPRI_SPR_MPUID_STAGE_SELECT            4:2
#endif
 
/*
 * Table engine operations
 */
#define CLEAR_TABLE_X(_tbl)                                                     \
    phvwri      p.app_header_table##_tbl##_valid, 0;                            \
    
#define CLEAR_TABLE_X_e(_tbl)                                                   \
    phvwri.e    p.app_header_table##_tbl##_valid, 0;                            \
    nop;                                                                        \
    
#define CLEAR_TABLE0                                                            \
    CLEAR_TABLE_X(0)                                                            \

#define CLEAR_TABLE0_e                                                          \
    CLEAR_TABLE_X_e(0)                                                          \

#define CLEAR_TABLE1                                                            \
    CLEAR_TABLE_X(1)                                                            \

#define CLEAR_TABLE1_e                                                          \
    CLEAR_TABLE_X_e(1)                                                          \

#define CLEAR_TABLE2                                                            \
    CLEAR_TABLE_X(2)                                                            \

#define CLEAR_TABLE2_e                                                          \
    CLEAR_TABLE_X_e(2)                                                          \

#define CLEAR_TABLE3                                                            \
    CLEAR_TABLE_X(3)                                                            \

#define CLEAR_TABLE3_e                                                          \
    CLEAR_TABLE_X_e(3)                                                          \

#define LOAD_TABLE_FOR_ADDR(_tbl, _lock_en, _tbl_addr, _tbl_sz, _pc)            \
    phvwrpair  p.{common_te##_tbl##_phv_table_lock_en...                        \
                  common_te##_tbl##_phv_table_raw_table_size},                  \
               ((_lock_en) << 3 ) | (_tbl_sz),                                  \
               p.common_te##_tbl##_phv_table_addr, _tbl_addr;                   \
    phvwri     p.common_te##_tbl##_phv_table_pc, _pc[33:6];                     \
    phvwr      p.app_header_table##_tbl##_valid, 1;                             \
                            
#define LOAD_TABLE_FOR_ADDR_e(_tbl, _lock_en, _tbl_addr, _tbl_sz, _pc)          \
    phvwrpair  p.{common_te##_tbl##_phv_table_lock_en...                        \
                  common_te##_tbl##_phv_table_raw_table_size},                  \
               ((_lock_en) << 3) | (_tbl_sz),                                   \
               p.common_te##_tbl##_phv_table_addr, _tbl_addr;                   \
    phvwri.e   p.common_te##_tbl##_phv_table_pc, _pc[33:6];                     \
    phvwr      p.app_header_table##_tbl##_valid, 1;                             \
    
#define LOAD_TABLE_NO_ADDR(_tbl, _pc)                                           \
    phvwrpair  p.{common_te##_tbl##_phv_table_lock_en...                        \
                  common_te##_tbl##_phv_table_raw_table_size},                  \
               ((TABLE_LOCK_DIS) << 3) | (TABLE_SIZE_0_BITS),                   \
               p.common_te##_tbl##_phv_table_addr, r0;                          \
    phvwri     p.common_te##_tbl##_phv_table_pc, _pc[33:6];                     \
    phvwr      p.app_header_table##_tbl##_valid, 1;                             \
                            
#define LOAD_TABLE_NO_ADDR_e(_tbl, _pc)                                         \
    phvwrpair  p.{common_te##_tbl##_phv_table_lock_en...                        \
                  common_te##_tbl##_phv_table_raw_table_size},                  \
               ((TABLE_LOCK_DIS) << 3) | (TABLE_SIZE_0_BITS),                   \
               p.common_te##_tbl##_phv_table_addr, r0;                          \
    phvwri.e   p.common_te##_tbl##_phv_table_pc, _pc[33:6];                     \
    phvwr      p.app_header_table##_tbl##_valid, 1;                             \
    
/*
 * Struct field bit offset and size
 */
#define AGE_BIT_OFFS_STRUCT(struct_name, field)                                 \
    offsetof(struct struct_name, field)                                         \
    
#define AGE_BIT_SIZE_STRUCT(struct_name, field)                                 \
    sizeof(struct struct_name.field)                                            \
    
#define AGE_BIT_OFFS_PHV(phv_field)                                             \
    offsetof(p, phv_field)                                                      \

#define AGE_BIT_OFFS_D_VEC(field)                                               \
    offsetof(d, field)                                                          \
    
/*
 * With 4 table engines, we can scan up to 4 flow sessions per stage;
 * below is how many rounds (i.e., stages) are dedicated for session scanning.
 */
#define AGE_SESSION_ROUND0                      0
#define AGE_SESSION_ROUND1                      1
#define AGE_SESSION_ROUND2                      2
#define AGE_SESSION_ROUND3                      3
#define AGE_SESSION_ROUNDS_MAX                  4

#define AGE_MPU_TABLES_MAX                      (CAPRI_MPU_TABLE_3 + 1)
#define AGE_SESSIONS_MPU_TABLES_TOTAL           (AGE_SESSION_ROUNDS_MAX *       \
                                                 AGE_MPU_TABLES_MAX)
/*
 * Stage specific actions
 */
#define AGE_SUMMARIZE_STAGE                     CAPRI_MPU_STAGE_6
#define AGE_POLLER_POST_STAGE                   CAPRI_MPU_STAGE_7
#define AGE_METRICS_STAGE                       CAPRI_MPU_STAGE_7

/*
 * K+I vectors
 */
#define AGE_KIVEC0_QSTATE_ADDR                                                  \
    k.{age_kivec0_qstate_addr_sbit0_ebit31...age_kivec0_qstate_addr_sbit32_ebit33}
#define AGE_KIVEC0_SESSION_TABLE_ADDR                                           \
    k.{age_kivec0_session_table_addr_sbit0_ebit5...age_kivec0_session_table_addr_sbit30_ebit33}
#define AGE_KIVEC0_SESSION_ID_CURR                                              \
    k.{age_kivec0_session_id_curr_sbit0_ebit3...age_kivec0_session_id_curr_sbit28_ebit31}
#define AGE_KIVEC0_NUM_SCANNABLES                                               \
    k.{age_kivec0_num_scannables_sbit0_ebit3...age_kivec0_num_scannables_sbit4_ebit4}
#define AGE_KIVEC0_CB_CFG_ERR_DISCARD                                           \
    k.{age_kivec0_cb_cfg_err_discard}
    
#define AGE_KIVEC2_ICMP_AGE_MS                                                  \
    k.{age_kivec2_icmp_age_ms}
#define AGE_KIVEC2_UDP_AGE_MS                                                   \
    k.{age_kivec2_udp_age_ms}
#define AGE_KIVEC2_TCP_AGE_MS                                                   \
    k.{age_kivec2_tcp_age_ms}
#define AGE_KIVEC2_OTHER_AGE_MS                                                 \
    k.{age_kivec2_other_age_ms}

#define AGE_KIVEC3_ICMP_AGE_MS                                                  \
    k.{age_kivec3_icmp_age_ms}
#define AGE_KIVEC3_UDP_AGE_MS                                                   \
    k.{age_kivec3_udp_age_ms}
#define AGE_KIVEC3_TCP_AGE_MS                                                   \
    k.{age_kivec3_tcp_age_ms}
#define AGE_KIVEC3_OTHER_AGE_MS                                                 \
    k.{age_kivec3_other_age_ms}
    
#define AGE_KIVEC4_ICMP_AGE_MS                                                  \
    k.{age_kivec4_icmp_age_ms}
#define AGE_KIVEC4_UDP_AGE_MS                                                   \
    k.{age_kivec4_udp_age_ms}
#define AGE_KIVEC4_TCP_AGE_MS                                                   \
    k.{age_kivec4_tcp_age_ms}
#define AGE_KIVEC4_OTHER_AGE_MS                                                 \
    k.{age_kivec4_other_age_ms}
    
#define AGE_KIVEC5_ICMP_AGE_MS                                                  \
    k.{age_kivec5_icmp_age_ms}
#define AGE_KIVEC5_UDP_AGE_MS                                                   \
    k.{age_kivec5_udp_age_ms}
#define AGE_KIVEC5_TCP_AGE_MS                                                   \
    k.{age_kivec5_tcp_age_ms}
#define AGE_KIVEC5_OTHER_AGE_MS                                                 \
    k.{age_kivec5_other_age_ms}
    
#define AGE_KIVEC7_LIF                                                          \
    k.{age_kivec7_lif_sbit0_ebit7...age_kivec7_lif_sbit8_ebit10}
#define AGE_KIVEC7_QTYPE                                                        \
    k.{age_kivec7_qtype}
#define AGE_KIVEC7_POLLER_QSTATE_ADDR                                           \
    k.{age_kivec7_poller_qstate_addr_sbit0_ebit1...age_kivec7_poller_qstate_addr_sbit58_ebit63}
    
#define AGE_KIVEC8_EXPIRY_SESSION_ID_BASE                                       \
    k.{age_kivec8_expiry_session_id_base}
#define AGE_KIVEC8_SESSION_RANGE_FULL                                           \
    k.{age_kivec8_session_range_full}
#define AGE_KIVEC8_SESSION_BATCH_FULL                                           \
    k.{age_kivec8_session_batch_full}
#define AGE_KIVEC8_EXPIRY_MAPS_FULL                                             \
    k.{age_kivec8_expiry_maps_full}
 
#define AGE_KIVEC9_CB_CFG_ERR_DISCARDS                                          \
    k.{age_kivec9_cb_cfg_err_discards}
#define AGE_KIVEC9_SCAN_INVOCATIONS                                             \
    k.{age_kivec9_scan_invocations}
#define AGE_KIVEC9_EXPIRED_SESSIONS                                             \
    k.{age_kivec9_expired_sessions_sbit0_ebit4...age_kivec9_expired_sessions_sbit5_ebit7}
#define AGE_KIVEC9_METRICS0_RANGE                                               \
    k.{age_kivec9_metrics0_start...age_kivec9_metrics0_end}

/*
 * Load an initial to_stageN kivec from d-vector
 */
#define AGE_KIVEC_MAX_AGE_MS_LOAD(_dst)                                         \
    phvwr       p.{age_kivec##_dst##_icmp_age_ms...                             \
                   age_kivec##_dst##_other_age_ms},                             \
                d.{icmp_age_ms...other_age_ms};                                 \

/*
 * Propagate a to_stageN kivec to another
 */
#define AGE_KIVEC_MAX_AGE_MS_PROPAGATE(_dst, _src)                              \
    phvwr       p.{age_kivec##_dst##_icmp_age_ms...                             \
                   age_kivec##_dst##_other_age_ms},                             \
                k.{age_kivec##_src##_icmp_age_ms...                             \
                   age_kivec##_src##_other_age_ms};                             \

/*
 * Timestamp is in clock ticks with clock speed of 833Mhz, or
 * (833 * 1M) ticks per second. To conserve k-vec size, the preferred
 * units is ms which would require a division. And to reduce MPU stalls,
 * the division result will be referenced as late as possible.
 */

#define AGE_SYS_TICKS_PER_SEC                   (833 * 1000 * 1000)
#define AGE_SYS_TICKS_PER_MS                    (833 * 1000)

#define AGE_MS_CALC(_timestamp_data)                                            \
    sub         r_timestamp_ms, r_timestamp_ms, _timestamp_data;                \
    divi        r_timestamp_ms, r_timestamp_ms, AGE_SYS_TICKS_PER_MS;           \

#define AGE_EXPIRY_CHECK_ALL_LKUP_TYPES(_expiry_bit, _break_label,              \
                                        _icmp_age_ms, _udp_age_ms,              \
                                        _tcp_age_ms, _other_age_ms)             \
  .brbegin;                                                                     \
    br          r_lkp_type[1:0];                                                \
    nop;                                                                        \
  .brcase SESSION_LKP_TYPE_ICMP;                                                \
    slt         c1, _icmp_age_ms, r_timestamp_ms;                               \
    b           _break_label;                                                   \
    phvwri.c1   p.age_kivec0_##_expiry_bit##_expired, 1;                        \
  .brcase SESSION_LKP_TYPE_UDP;                                                 \
    slt         c1, _udp_age_ms, r_timestamp_ms;                                \
    b           _break_label;                                                   \
    phvwri.c1   p.age_kivec0_##_expiry_bit##_expired, 1;                        \
  .brcase SESSION_LKP_TYPE_TCP;                                                 \
    slt         c1, _tcp_age_ms, r_timestamp_ms;                                \
    b           _break_label;                                                   \
    phvwri.c1   p.age_kivec0_##_expiry_bit##_expired, 1;                        \
  .brcase SESSION_LKP_TYPE_OTHER;                                               \
    slt         c1, _other_age_ms, r_timestamp_ms;                              \
    b           _break_label;                                                   \
    phvwri.c1   p.age_kivec0_##_expiry_bit##_expired, 1;                        \
  .brend;                                                                       \
  
#define AGE_EXPIRY_CHECK_ALL_LKUP_TYPES_e(_expiry_bit,                          \
                                          _icmp_age_ms, _udp_age_ms,            \
                                          _tcp_age_ms, _other_age_ms)           \
  .brbegin;                                                                     \
    br          r_lkp_type[1:0];                                                \
    nop;                                                                        \
  .brcase SESSION_LKP_TYPE_ICMP;                                                \
    slt.e       c1, _icmp_age_ms, r_timestamp_ms;                               \
    phvwri.c1   p.age_kivec0_##_expiry_bit##_expired, 1;                        \
  .brcase SESSION_LKP_TYPE_UDP;                                                 \
    slt.e       c1, _udp_age_ms, r_timestamp_ms;                                \
    phvwri.c1   p.age_kivec0_##_expiry_bit##_expired, 1;                        \
  .brcase SESSION_LKP_TYPE_TCP;                                                 \
    slt.e       c1, _tcp_age_ms, r_timestamp_ms;                                \
    phvwri.c1   p.age_kivec0_##_expiry_bit##_expired, 1;                        \
  .brcase SESSION_LKP_TYPE_OTHER;                                               \
    slt.e       c1, _other_age_ms, r_timestamp_ms;                              \
    phvwri.c1   p.age_kivec0_##_expiry_bit##_expired, 1;                        \
  .brend;                                                                       \

/*
 * Issue a session_info scan for _tbl per the given _round, provided the scan
 * falls within the given _num_scannables. The session to read is given
 * by r_session_info_addr, after which, it will be incremented to point
 * to the next adjacent session.
 */
#define AGE_SESSION_INFO_POSSIBLE_SCAN_INCR(_tbl, _round, _num_scannables,      \
                                            _pc, _break_label)                  \
    slt         c1, ((_round) * AGE_MPU_TABLES_MAX) + ((_tbl) + 1),             \
                _num_scannables;                                                \
    bcf         [!c1], _break_label;                                            \
    nop;                                                                        \
    LOAD_TABLE_FOR_ADDR(_tbl, TABLE_LOCK_DIS, r_session_info_addr,              \
                        TABLE_SIZE_512_BITS, _pc)                               \
    add         r_session_info_addr, r_session_info_addr, SESSION_INFO_BYTES;   \

/*
 * The following 2 macros are similar to AGE_SESSION_INFO_POSSIBLE_SCAN_INCR(),
 * but with the following differences:
 * - There are 2 macros to allow the caller to reduce MPU stall reduction in
 *   between usage of the AGE_MS_CALC() macro and AGE_EXPIRY_CHECK_ALL_LKUP_TYPES().
 * - The session_info to read is calculated using AGE_KIVEC0_SESSION_ID_CURR
 *   together with _round and _tbl.
 */
#define AGE_SESSION_INFO_NUM_SCANNABLES_CHECK_CF(_cf, _tbl, _round,             \
                                                _num_scannables)                \
    slt         _cf, ((_round) * AGE_MPU_TABLES_MAX) + ((_tbl) + 1),            \
                _num_scannables;                                                \
    add         r_session_id, AGE_KIVEC0_SESSION_ID_CURR,                       \
                ((_round) * AGE_MPU_TABLES_MAX) + (_tbl);                       \
    add         r_session_info_addr, AGE_KIVEC0_SESSION_TABLE_ADDR,             \
                r_session_id, SESSION_INFO_BYTES_SHFT;                          \
    
#define AGE_SESSION_INFO_POSSIBLE_SCAN_CHECK_CF(_cf, _tbl, _pc, _break_label)   \
    bcf         [!_cf], _break_label;                                           \
    nop;                                                                        \
    LOAD_TABLE_FOR_ADDR(_tbl, TABLE_LOCK_DIS, r_session_info_addr,              \
                        TABLE_SIZE_512_BITS, _pc)                               \
                
/*
 * When there is no more session scanning to launch for session 0 (i.e., table 0),
 * the following is the alternative launch.
 */
#define AGE_SESSION_NO_MORE_SESSION0_ALT()                                      \
    AGE_SUMMARIZE_LAUNCH(0, AGE_KIVEC0_QSTATE_ADDR, age_summarize)              \
 
/*
 * When there is no more session scanning to launch for session 1 or 2 
 * (i.e., table 1 or 2), the following is the alternative launch.
 */
#define AGE_SESSION_NO_MORE_SESSION1_2_ALT()                                    \
    nop;                                                                        \

/*
 * When there is no more session scanning to launch for session 3 (i.e., table 3),
 * the following is the alternative launch.
 */
#define AGE_SESSION_NO_MORE_SESSION3_ALT()                                      \
    AGE_METRICS0_TABLE3_COMMIT_LAUNCH(AGE_KIVEC0_QSTATE_ADDR)                   \

/*
 * Launch age_summarize for its designated stage, i.e., use lock if launching
 * from AGE_SUMMARIZE_STAGE minus 1.
 */
#define AGE_SUMMARIZE_LAUNCH(_tbl, _src_qaddr, _pc)                             \
    add         r_qstate_addr, _src_qaddr,                                      \
                SESSION_AGE_CB_TABLE_SUMMARIZE_OFFSET;                          \
    LOAD_TABLE_NO_ADDR(_tbl, _pc)                                               \
    mfspr       r_stage, spr_mpuid;                                             \
    slt         c1, r_stage[CAPRI_SPR_MPUID_STAGE_SELECT],                      \
                AGE_SUMMARIZE_STAGE - 1;                                        \
    phvwrpair.!c1 p.{common_te##_tbl##_phv_table_lock_en...                     \
                     common_te##_tbl##_phv_table_raw_table_size},               \
                  (TABLE_LOCK_EN << 3) | (TABLE_SIZE_512_BITS),                 \
                  p.common_te##_tbl##_phv_table_addr, r_qstate_addr;            \

#define AGE_SUMMARIZE_LAUNCH_e(_tbl, _src_qaddr, _pc)                           \
    add         r_qstate_addr, _src_qaddr,                                      \
                SESSION_AGE_CB_TABLE_SUMMARIZE_OFFSET;                          \
    LOAD_TABLE_NO_ADDR(_tbl, _pc)                                               \
    mfspr       r_stage, spr_mpuid;                                             \
    slt.e       c1, r_stage[CAPRI_SPR_MPUID_STAGE_SELECT],                      \
                AGE_SUMMARIZE_STAGE - 1;                                        \
    phvwrpair.!c1 p.{common_te##_tbl##_phv_table_lock_en...                     \
                     common_te##_tbl##_phv_table_raw_table_size},               \
                  (TABLE_LOCK_EN << 3) | (TABLE_SIZE_512_BITS),                 \
                  p.common_te##_tbl##_phv_table_addr, r_qstate_addr;            \

/*
 * Load 64-bit immediate
 */
#define AGE_IMM64_LOAD(_reg, _imm64)                                            \
    addi        _reg, r0,  loword(_imm64);                                      \
    addui       _reg, reg, hiword(_imm64);                                      \
    
#define AGE_IMM64_LOAD_c(_cf, _reg, _imm64)                                     \
    addi._cf    _reg, r0,  loword(_imm64);                                      \
    addui._cf   _reg, reg, hiword(_imm64);                                      \

/*
 * Doorbell related
 */
#define AGE_DB_DATA_WITH_RING_INDEX(_index, _ring, _qid, _pid)                  \
    add         r_db_data, _index, _ring, DB_RING_SHFT;                         \
    add         r_db_data, r_db_data, _qid, DB_QID_SHFT;                        \
    add         r_db_data, r_db_data, _pid, DB_PID_SHFT;                        \

#define AGE_DB_DATA(_qid)                                                       \
    add         r_db_data, r0, _qid, DB_QID_SHFT;                               \

#define AGE_DB_ADDR(_lif, _qtype, _sched_wr, _upd)                              \
    addi        r_db_addr, r0, DB_ADDR_BASE +                                   \
                               (_upd) + ((_sched_wr) << DB_UPD_SHFT);           \
    add         r_db_addr, r_db_addr, _qtype, DB_TYPE_SHFT;                     \
    add         r_db_addr, r_db_addr, _lif, DB_LIF_SHFT;                        \

#define AGE_DB_ADDR_SCHED_EVAL(_lif, _qtype)                                    \
    AGE_DB_ADDR(_lif, _qtype, DB_SCHED_UPD_EVAL, DB_IDX_UPD_NOP)                \

#define AGE_DB_ADDR_SCHED_RESET(_lif, _qtype)                                   \
    AGE_DB_ADDR(_lif, _qtype, DB_SCHED_UPD_CLEAR, DB_IDX_UPD_NOP)               \

#define AGE_DB_ADDR_SCHED_PIDX_INC(_lif, _qtype)                                \
    AGE_DB_ADDR(_lif, _qtype, DB_SCHED_UPD_SET, DB_IDX_UPD_PIDX_INC)            \

/*
 * Timer related
 */
#ifndef TIMER_ADDR_LIF_SHFT
#define TIMER_ADDR_LIF_SHFT                     3
#endif

#define AGE_DB_ADDR_FAST_TIMER(_lif)                                            \
    addi        r_db_addr, r0, CAPRI_MEM_FAST_TIMER_START;                      \
    add         r_db_addr, r_db_addr, _lif, TIMER_ADDR_LIF_SHFT;                \

#define AGE_DB_ADDR_SLOW_TIMER(_lif)                                            \
    addi        r_db_addr, r0, CAPRI_MEM_SLOW_TIMER_START;                      \
    add         r_db_addr, r_db_addr, _lif, TIMER_ADDR_LIF_SHFT;                \

#define AGE_DB_DATA_TIMER_WITH_RING(_type, _qid, _ring, _delta_time)            \
    add         r_db_data, _type, _qid, TIMER_QID_SHFT;                         \
    or          r_db_data, r_db_data, _ring, TIMER_RING_SHFT;                   \
    or          r_db_data, r_db_data, _delta_time, TIMER_DELTA_TIME_SHFT;       \
        
#define AGE_DB_DATA_TIMER(_qid, _delta_time)                                    \
    add         r_db_data, r0, _qid, TIMER_QID_SHFT;                            \
    or          r_db_data, r_db_data, _delta_time, TIMER_DELTA_TIME_SHFT;       \
   
/*
 * Metrics related
 */
#define AGE_METRICS_SET_V(_kivec_metrics, _val)                                 \
    phvwr       p._kivec_metrics, _val;                                         \
  
#define AGE_METRICS_SET_V_c(_cf, _kivec_metrics, _val)                          \
    phvwr._cf   p._kivec_metrics, _val;                                         \
   
#define AGE_METRICS_TBLADD(_metrics, _key)                                      \
    sne         c1, _key, r0;                                                   \
    tbladd.c1   d._metrics, _key;                                               \

#define AGE_METRICS_TBLADD_e(_metrics, _key)                                    \
    sne.e       c1, _key, r0;                                                   \
    tbladd.c1   d._metrics, _key;                                               \
    
#define AGE_METRICS_SET(_metrics)                                               \
    AGE_METRICS_SET_V(age_kivec9_##_metrics, 1)                                 \

#define AGE_METRICS_SET_c(_cf, _metrics)                                        \
    AGE_METRICS_SET_V_c(_cf, age_kivec9_##_metrics, 1)                          \

#define AGE_METRICS_CLR(_metrics)                                               \
    AGE_METRICS_SET_V(age_kivec9_##_metrics, 0)                                 \

#define AGE_METRICS_VAL_SET(_metrics, _val)                                     \
    AGE_METRICS_SET_V(age_kivec9_##_metrics, _val)                              \
  
#define AGE_METRICS_PARAMS()                                                    \
    .param      age_metrics0_commit;                                            \

/*
 * Launch metrics0 for its designated, i.e., use lock if launching from
 * AGE_METRICS_STAGE minus 1.
 */
#define AGE_METRICS0_TABLE3_COMMIT_LAUNCH(_src_qaddr)                           \
    add         r_qstate_addr, _src_qaddr,                                      \
                SESSION_AGE_CB_TABLE_METRICS0_OFFSET;                           \
    LOAD_TABLE_FOR_ADDR(3, TABLE_LOCK_EN, r_qstate_addr,                        \
                        TABLE_SIZE_512_BITS, age_metrics0_commit)               \
    mfspr       r_stage, spr_mpuid;                                             \
    slt         c1, r_stage[CAPRI_SPR_MPUID_STAGE_SELECT],                      \
                AGE_METRICS_STAGE - 1;                                          \
    phvwrpair.c1 p.{common_te3_phv_table_lock_en...                             \
                    common_te3_phv_table_raw_table_size},                       \
                 ((TABLE_LOCK_DIS) << 3) | (TABLE_SIZE_0_BITS),                 \
                 p.common_te3_phv_table_addr, r0;                               \
                  
#define AGE_METRICS0_TABLE3_COMMIT_LAUNCH_e(_src_qaddr)                         \
    add         r_qstate_addr, _src_qaddr,                                      \
                SESSION_AGE_CB_TABLE_METRICS0_OFFSET;                           \
    LOAD_TABLE_FOR_ADDR(3, TABLE_LOCK_EN, r_qstate_addr,                        \
                        TABLE_SIZE_512_BITS, age_metrics0_commit)               \
    mfspr       r_stage, spr_mpuid;                                             \
    slt.e       c1, r_stage[CAPRI_SPR_MPUID_STAGE_SELECT],                      \
                AGE_METRICS_STAGE - 1;                                          \
    phvwrpair.c1 p.{common_te3_phv_table_lock_en...                             \
                    common_te3_phv_table_raw_table_size},                       \
                 ((TABLE_LOCK_DIS) << 3) | (TABLE_SIZE_0_BITS),                 \
                 p.common_te3_phv_table_addr, r0;                               \

#endif //__SESSION_AGE_COMMON_H
