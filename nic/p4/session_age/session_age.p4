/*********************************************************************************
 * Session Aging
 *********************************************************************************/

#if defined(ARTEMIS)
#include "nic/apollo/p4/artemis_txdma/common_txdma_dummy.p4"
#elif defined(ATEHNA)
#include "nic/apollo/p4/athena_txdma/common_txdma_dummy.p4"
#else
#include "nic/p4/common-p4+/common_txdma_dummy.p4"
#endif

/*
 * stage 0
 */
#define tx_table_s0_t0_action   age_start


/*
 * Table names
 */
#define tx_table_s0_t0          s0_tbl
#define tx_table_s1_t0          s1_tbl
#define tx_table_s2_t0          s2_tbl
#define tx_table_s2_t1          s2_tbl1
#define tx_table_s2_t2          s2_tbl2
#define tx_table_s2_t3          s2_tbl3
#define tx_table_s3_t0          s3_tbl
#define tx_table_s3_t1          s3_tbl1
#define tx_table_s3_t2          s3_tbl2
#define tx_table_s3_t3          s3_tbl3
#define tx_table_s4_t0          s4_tbl
#define tx_table_s4_t1          s4_tbl1
#define tx_table_s4_t2          s4_tbl2
#define tx_table_s4_t3          s4_tbl3
#define tx_table_s5_t0          s5_tbl
#define tx_table_s5_t1          s5_tbl1
#define tx_table_s5_t2          s5_tbl2
#define tx_table_s5_t3          s5_tbl3
#define tx_table_s6_t0          s6_tbl
#define tx_table_s7_t0          s7_tbl
#define tx_table_s7_t3          s7_tbl3

/*
 * stage 1
 */
#define tx_table_s1_t0_action   age_fsm_exec

/*
 * stage 2
 */
#define tx_table_s2_t0_action   age_round0_session0
#define tx_table_s2_t1_action   age_round0_session1
#define tx_table_s2_t2_action   age_round0_session2
#define tx_table_s2_t3_action   age_round0_session3

/*
 * stage 3
 */
#define tx_table_s3_t0_action   age_round1_session0
#define tx_table_s3_t1_action   age_round1_session1
#define tx_table_s3_t2_action   age_round1_session2
#define tx_table_s3_t3_action   age_round1_session3

/*
 * stage 4
 */
#define tx_table_s4_t0_action   age_round2_session0
#define tx_table_s4_t1_action   age_round2_session1
#define tx_table_s4_t2_action   age_round2_session2
#define tx_table_s4_t3_action   age_round2_session3

/*
 * stage 5
 */
#define tx_table_s5_t0_action   age_round3_session0
#define tx_table_s5_t1_action   age_round3_session1
#define tx_table_s5_t2_action   age_round3_session2
#define tx_table_s5_t3_action   age_round3_session3

/*
 * stage 6
 */
#define tx_table_s6_t0_action   age_summarize

/*
 * stage 7
 */
#define tx_table_s7_t0_action   age_poller_post
#define tx_table_s7_t3_action   age_metrics0_commit


#if defined(ARTEMIS)
#include "nic/apollo/p4/artemis_txdma/common_txdma.p4"
#elif defined(ATHENA)
#include "nic/apollo/p4/athena_txdma/common_txdma.p4"
#else
#include "nic/p4/common-p4+/common_txdma.p4"
#endif

#include "session_age_defines.h"


/*
 * D-vectors
 */

/*
 * d for stage 0: 
 */
header_type session_age_cb_t {
    fields {
        CAPRI_QSTATE_HEADER_COMMON
        CAPRI_QSTATE_HEADER_RING(0)

        /*
         * NOTE: cb is programmed by HAL and would work best when
         * fields are aligned on whole byte boundary.
         */
        icmp_age_ms                     : 32;
        udp_age_ms                      : 32;
        tcp_age_ms                      : 32;
        other_age_ms                    : 32;

        scan_resched_time               : 32;
        pad                             : 240;
        cb_activate                     : 16;  // must be last in CB
    }
}

#define SESSION_AGE_CB_DATA                                                     \
    rsvd, cosA, cosB, cos_sel, eval_last,                                       \
    host, total, pid, pi_0, ci_0,                                               \
    icmp_age_ms, udp_age_ms, tcp_age_ms, other_age_ms,                          \
    scan_resched_time, pad, cb_activate                                         \
    
#define SESSION_AGE_CB_PRAGMA                                                   \
@pragma little_endian pi_0 ci_0                                                 \
    icmp_age_ms udp_age_ms tcp_age_ms other_age_ms                              \
    scan_resched_time cb_activate                                               \

#define SESSION_AGE_CB_USE(scratch)                                             \
    modify_field(scratch.rsvd, rsvd);                                           \
    modify_field(scratch.cosA, cosA);                                           \
    modify_field(scratch.cosB, cosB);                                           \
    modify_field(scratch.cos_sel, cos_sel);                                     \
    modify_field(scratch.eval_last, eval_last);                                 \
    modify_field(scratch.host, host);                                           \
    modify_field(scratch.total, total);                                         \
    modify_field(scratch.pid, pid);                                             \
    modify_field(scratch.pi_0, pi_0);                                           \
    modify_field(scratch.ci_0, ci_0);                                           \
    modify_field(scratch.icmp_age_ms, icmp_age_ms);                             \
    modify_field(scratch.udp_age_ms, udp_age_ms);                               \
    modify_field(scratch.tcp_age_ms, tcp_age_ms);                               \
    modify_field(scratch.other_age_ms, other_age_ms);                           \
    modify_field(scratch.scan_resched_time, scan_resched_time);                 \
    modify_field(scratch.pad, pad);                                             \
    modify_field(scratch.cb_activate, cb_activate);                             \
    
header_type session_age_fsm_t {
    fields {
        fsm_state                       : 8; // must be first in CB
        pad0                            : 8;
        session_batch_sz_shft           : 16;
        session_batch_sz                : 32;
        
        session_id_base                 : 32;
        session_id_next                 : 32;
        session_range_sz                : 32;
        
        expiry_session_id_base          : 32;
        expiry_map_sessions_scanned     : 32;
        total_sessions_scanned          : 32;
        session_table_base_addr         : 64;
        
        pad1                            : 176;
        cb_activate                     : 16;  // must be last in CB
    }
}

#define SESSION_AGE_FSM_DATA                                                    \
    fsm_state, pad0, session_batch_sz_shft, session_batch_sz,                   \
    session_id_base, session_id_next, session_range_sz,                         \
    expiry_session_id_base, expiry_map_sessions_scanned,                        \
    total_sessions_scanned, session_table_base_addr,                            \
    pad1, cb_activate                                                           \
    
#define SESSION_AGE_FSM_PRAGMA                                                  \
@pragma little_endian session_batch_sz_shft session_batch_sz                    \
    session_id_base session_id_next session_range_sz                            \
    expiry_session_id_base expiry_map_sessions_scanned total_sessions_scanned   \
    session_table_base_addr cb_activate

#define SESSION_AGE_FSM_USE(scratch)                                            \
    modify_field(scratch.fsm_state, fsm_state);                                 \
    modify_field(scratch.pad0, pad0);                                           \
    modify_field(scratch.session_batch_sz_shft, session_batch_sz_shft);         \
    modify_field(scratch.session_batch_sz, session_batch_sz);                   \
    modify_field(scratch.session_id_base, session_id_base);                     \
    modify_field(scratch.session_id_next, session_id_next);                     \
    modify_field(scratch.session_range_sz, session_range_sz);                   \
    modify_field(scratch.expiry_session_id_base, expiry_session_id_base);       \
    modify_field(scratch.expiry_map_sessions_scanned, expiry_map_sessions_scanned);\
    modify_field(scratch.total_sessions_scanned, total_sessions_scanned);       \
    modify_field(scratch.session_table_base_addr, session_table_base_addr);     \
    modify_field(scratch.pad1, pad1);                                           \
    modify_field(scratch.cb_activate, cb_activate);                             \
    
header_type session_age_fsm_state_t {
    fields {
        state                           : 8;
    }
}

header_type session_age_summarize_t {
    fields {
        poller_qdepth_shft              : 8;
        pad0                            : 8;
        pad1                            : 16;
        poller_qstate_addr              : 64;
        
        expiry_map0                     : 64;
        expiry_map1                     : 64;
        expiry_map2                     : 64;
        expiry_map3                     : 64;
        
        pad2                            : 144;
        cb_activate                     : 16; // must be last in CB
    }
}

#define SESSION_AGE_SUMMARIZE_DATA                                              \
    poller_qdepth_shft, pad0, pad1, poller_qstate_addr,                         \
    expiry_map0, expiry_map1, expiry_map2, expiry_map3,                         \
    pad2, cb_activate                                                           \
    
#define SESSION_AGE_SUMMARIZE_PRAGMA                                            \
@pragma little_endian poller_qstate_addr                                        \
    expiry_map0 expiry_map1 expiry_map2 expiry_map3                             \
    cb_activate                                                                 \
    
#define SESSION_AGE_SUMMARIZE_USE(scratch)                                      \
    modify_field(scratch.poller_qdepth_shft, poller_qdepth_shft);               \
    modify_field(scratch.pad0, pad0);                                           \
    modify_field(scratch.pad1, pad1);                                           \
    modify_field(scratch.poller_qstate_addr, poller_qstate_addr);               \
    modify_field(scratch.expiry_map0, expiry_map0);                             \
    modify_field(scratch.expiry_map1, expiry_map1);                             \
    modify_field(scratch.expiry_map2, expiry_map2);                             \
    modify_field(scratch.expiry_map3, expiry_map3);                             \
    modify_field(scratch.pad2, pad2);                                           \
    modify_field(scratch.cb_activate, cb_activate);                             \
    
header_type session_age_metrics0_t {
    fields {

        // CAUTION: order of fields must match session_kivec9_t
        cb_cfg_err_discards             : 64;
        scan_invocations                : 64;
        expired_sessions                : 64;
    }
}

#define SESSION_AGE_METRICS0_DATA                                               \
    cb_cfg_err_discards, scan_invocations, expired_sessions
    
#define SESSION_AGE_METRICS0_PRAGMA                                             \
@pragma little_endian cb_cfg_err_discards scan_invocations expired_sessions     \
    
#define SESSION_AGE_METRICS0_USE(scratch)                                       \
    modify_field(scratch.cb_cfg_err_discards, cb_cfg_err_discards);             \
    modify_field(scratch.scan_invocations, scan_invocations);                   \
    modify_field(scratch.expired_sessions, expired_sessions);                   \
    
/*
 * kivec0: header union with global (128 bits max)
 */
header_type age_kivec0_t {
    fields {
        qstate_addr                     : 34;
        session_table_addr              : 34;
        session_id_curr                 : 32;
        num_scannables                  : 5;
        cb_cfg_err_discard              : 1;
        
        // The following is arranged in big endian layout,
        // with higher rounds/sessions in the MSB
        round3_session3_expired         : 1;
        round3_session2_expired         : 1;
        round3_session1_expired         : 1;
        round3_session0_expired         : 1;
        round2_session3_expired         : 1;
        round2_session2_expired         : 1;
        round2_session1_expired         : 1;
        round2_session0_expired         : 1;
        round1_session3_expired         : 1;
        round1_session2_expired         : 1;
        round1_session1_expired         : 1;
        round1_session0_expired         : 1;
        round0_session3_expired         : 1;
        round0_session2_expired         : 1;
        round0_session1_expired         : 1;
        round0_session0_expired         : 1;
    }
}

#define AGE_KIVEC0_USE(scratch, kivec)                                          \
    modify_field(scratch.qstate_addr, kivec.qstate_addr);                       \
    modify_field(scratch.session_table_addr, kivec.session_table_addr);         \
    modify_field(scratch.session_id_curr, kivec.session_id_curr);               \
    modify_field(scratch.num_scannables, kivec.num_scannables);                 \
    modify_field(scratch.cb_cfg_err_discard, kivec.cb_cfg_err_discard);         \
    modify_field(scratch.round3_session3_expired, kivec.round3_session3_expired);\
    modify_field(scratch.round3_session2_expired, kivec.round3_session2_expired);\
    modify_field(scratch.round3_session1_expired, kivec.round3_session1_expired);\
    modify_field(scratch.round3_session0_expired, kivec.round3_session0_expired);\
    modify_field(scratch.round2_session3_expired, kivec.round2_session3_expired);\
    modify_field(scratch.round2_session2_expired, kivec.round2_session2_expired);\
    modify_field(scratch.round2_session1_expired, kivec.round2_session1_expired);\
    modify_field(scratch.round2_session0_expired, kivec.round2_session0_expired);\
    modify_field(scratch.round1_session3_expired, kivec.round1_session3_expired);\
    modify_field(scratch.round1_session2_expired, kivec.round1_session2_expired);\
    modify_field(scratch.round1_session1_expired, kivec.round1_session1_expired);\
    modify_field(scratch.round1_session0_expired, kivec.round1_session0_expired);\
    modify_field(scratch.round0_session3_expired, kivec.round0_session3_expired);\
    modify_field(scratch.round0_session2_expired, kivec.round0_session2_expired);\
    modify_field(scratch.round0_session1_expired, kivec.round0_session1_expired);\
    modify_field(scratch.round0_session0_expired, kivec.round0_session0_expired);\

/*
 * kivecX: header union with to_stageN (128 bits max)
 * for holding max age values of supported flow lookup types.
 */
#define AGE_KIVEC_MAX_AGE_MS(_vec)                                              \
header_type age_kivec##_vec##_t {                                               \
    fields {                                                                    \
        icmp_age_ms                     : 32;                                   \
        udp_age_ms                      : 32;                                   \
        tcp_age_ms                      : 32;                                   \
        other_age_ms                    : 32;                                   \
    }                                                                           \
}                                                                               \

AGE_KIVEC_MAX_AGE_MS(2)
AGE_KIVEC_MAX_AGE_MS(3)
AGE_KIVEC_MAX_AGE_MS(4)
AGE_KIVEC_MAX_AGE_MS(5)

#define AGE_KIVEC_MAX_AGE_MS_USE(scratch, kivec)                                \
    modify_field(scratch.icmp_age_ms, kivec.icmp_age_ms);                       \
    modify_field(scratch.udp_age_ms, kivec.udp_age_ms);                         \
    modify_field(scratch.tcp_age_ms, kivec.tcp_age_ms);                         \
    modify_field(scratch.other_age_ms, kivec.other_age_ms);                     \
    
/*
 * kivec7: header union with to_stage7 (128 bits max)
 */
header_type age_kivec7_t {
    fields {
        lif                             : 11;
        qtype                           : 3;
        poller_qstate_addr              : 64;
    }
}
        
#define AGE_KIVEC7_USE(scratch, kivec)                                          \
    modify_field(scratch.lif, kivec.lif);                                       \
    modify_field(scratch.qtype, kivec.qtype);                                   \
    modify_field(scratch.poller_qstate_addr, kivec.poller_qstate_addr);         \
    
/*
 * kivec8: header union with stage_2_stage for table 0 (160 bits max)
 */
header_type age_kivec8_t {
    fields {
        expiry_session_id_base          : 32;
        session_range_full              : 1;
        session_batch_full              : 1;
        expiry_maps_full                : 1;
    }
}

#define AGE_KIVEC8_USE(scratch, kivec)                                          \
    modify_field(scratch.expiry_session_id_base, kivec.expiry_session_id_base); \
    modify_field(scratch.session_range_full, kivec.session_range_full);         \
    modify_field(scratch.session_batch_full, kivec.session_batch_full);         \
    modify_field(scratch.expiry_maps_full, kivec.expiry_maps_full);             \
    
/*
 * kivec9: header union with stage_2_stage for table 3 (160 bits max)
 */
header_type age_kivec9_t {
    fields {
        // CAUTION: order of fields must match session_metrics0_t
        metrics0_start                  : 1;
        cb_cfg_err_discards             : 1;
        scan_invocations                : 1;
        expired_sessions                : 8;
        metrics0_end                    : 1;
    }
}

#define AGE_KIVEC9_USE(scratch, kivec)                                          \
    modify_field(scratch.metrics0_start, kivec.metrics0_start);                 \
    modify_field(scratch.cb_cfg_err_discards, kivec.cb_cfg_err_discards);       \
    modify_field(scratch.scan_invocations, kivec.scan_invocations);             \
    modify_field(scratch.expired_sessions, kivec.expired_sessions);             \
    modify_field(scratch.metrics0_end, kivec.metrics0_end);                     \
    
/*
 * Header unions for d-vector
 */
@pragma scratch_metadata
metadata session_age_cb_t               session_age_cb;

@pragma scratch_metadata
metadata session_age_fsm_t              session_age_fsm;

@pragma scratch_metadata
metadata session_info_d                 session_info;

@pragma scratch_metadata
metadata session_age_summarize_t        session_age_summarize;

@pragma scratch_metadata
metadata session_age_metrics0_t         session_age_metrics0;

@pragma scratch_metadata
metadata poller_cb_t                    poller_cb;

/*
 * Header unions for PHV layout
 */
@pragma pa_header_union ingress         common_global
metadata age_kivec0_t                   age_kivec0;
@pragma scratch_metadata
metadata age_kivec0_t                   age_kivec0_scratch;

@pragma dont_trim
@pragma pa_header_union ingress         to_stage_2
metadata age_kivec2_t                   age_kivec2;
@pragma scratch_metadata
metadata age_kivec2_t                   age_kivec2_scratch;

@pragma dont_trim
@pragma pa_header_union ingress         to_stage_3
metadata age_kivec3_t                   age_kivec3;
@pragma scratch_metadata
metadata age_kivec3_t                   age_kivec3_scratch;

@pragma dont_trim
@pragma pa_header_union ingress         to_stage_4
metadata age_kivec4_t                   age_kivec4;
@pragma scratch_metadata
metadata age_kivec4_t                   age_kivec4_scratch;

@pragma dont_trim
@pragma pa_header_union ingress         to_stage_5
metadata age_kivec5_t                   age_kivec5;
@pragma scratch_metadata
metadata age_kivec5_t                   age_kivec5_scratch;

@pragma dont_trim
@pragma pa_header_union ingress         to_stage_7
metadata age_kivec7_t                   age_kivec7;
@pragma scratch_metadata
metadata age_kivec7_t                   age_kivec7_scratch;

@pragma pa_header_union ingress         common_t0_s2s
metadata age_kivec8_t                   age_kivec8;
@pragma scratch_metadata
metadata age_kivec8_t                   age_kivec8_scratch;

@pragma dont_trim
@pragma pa_header_union ingress         common_t3_s2s
metadata age_kivec9_t                   age_kivec9;
@pragma scratch_metadata
metadata age_kivec9_t                   age_kivec9_scratch;

/*
 * PHV following k (for app DMA etc.)
 */
@pragma dont_trim
metadata ring_entry_t                   ring_entry; 

@pragma dont_trim
metadata doorbell_data_raw_t            db_data_no_index;

@pragma dont_trim
metadata doorbell_data_raw_t            db_data_batch_timer;

@pragma dont_trim
metadata doorbell_data_raw_t            db_data_repost_timer;

@pragma dont_trim
@pragma pa_align 512
metadata poller_slot_data_t             poller_slot_data; 

@pragma dont_trim
metadata poller_slot_data_t             poller_null_data; 

@pragma dont_trim
metadata ring_pi_t                      poller_posted_pi; 

@pragma dont_trim
metadata session_age_fsm_state_t        fsm_state_next; 

// DMA commands metadata
@pragma dont_trim
@pragma pa_align 512
metadata dma_cmd_phv2mem_t dma_p2m_0;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_0
metadata dma_cmd_mem2mem_t dma_m2m_0;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_1;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_1
metadata dma_cmd_mem2mem_t dma_m2m_1;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_2;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_2
metadata dma_cmd_mem2mem_t dma_m2m_2;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_3;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_3
metadata dma_cmd_mem2mem_t dma_m2m_3;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_4;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_4
metadata dma_cmd_mem2mem_t dma_m2m_4;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_5;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_5
metadata dma_cmd_mem2mem_t dma_m2m_5;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_6;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_6
metadata dma_cmd_mem2mem_t dma_m2m_6;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_7;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_7
metadata dma_cmd_mem2mem_t dma_m2m_7;


/*
 * Action functions to generate k_struct and d_struct
 *
 * These action functions are currently only to generate the k+i and d structs
 * and do not implement any pseudo code
 */

/*****************************************************************************
 *  age_start : Initial stage
 *****************************************************************************/
SESSION_AGE_CB_PRAGMA
action age_start(SESSION_AGE_CB_DATA)
{
    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);

    AGE_KIVEC0_USE(age_kivec0_scratch, age_kivec0)
    SESSION_AGE_CB_USE(session_age_cb)
}


/*****************************************************************************
 *  age_fsm_exec : Execute FSM
 *****************************************************************************/
SESSION_AGE_FSM_PRAGMA
action age_fsm_exec(SESSION_AGE_FSM_DATA)
{
    AGE_KIVEC0_USE(age_kivec0_scratch, age_kivec0)
    SESSION_AGE_FSM_USE(session_age_fsm)
}


/*****************************************************************************
 *  age_round0_session0 : Round (i.e., stage) 0 session 0 age
 *****************************************************************************/
action age_round0_session0(SESSION_INFO_DATA)
{
    AGE_KIVEC0_USE(age_kivec0_scratch, age_kivec0)
    AGE_KIVEC_MAX_AGE_MS_USE(age_kivec2_scratch, age_kivec2)
    SESSION_INFO_USE(session_info)
}

action age_round0_session1(SESSION_INFO_DATA)
{
    AGE_KIVEC0_USE(age_kivec0_scratch, age_kivec0)
    AGE_KIVEC_MAX_AGE_MS_USE(age_kivec2_scratch, age_kivec2)
    SESSION_INFO_USE(session_info)
}

action age_round0_session2(SESSION_INFO_DATA)
{
    AGE_KIVEC0_USE(age_kivec0_scratch, age_kivec0)
    AGE_KIVEC_MAX_AGE_MS_USE(age_kivec2_scratch, age_kivec2)
    SESSION_INFO_USE(session_info)
}

action age_round0_session3(SESSION_INFO_DATA)
{
    AGE_KIVEC0_USE(age_kivec0_scratch, age_kivec0)
    AGE_KIVEC_MAX_AGE_MS_USE(age_kivec2_scratch, age_kivec2)
    SESSION_INFO_USE(session_info)
}

action age_round1_session0(SESSION_INFO_DATA)
{
    AGE_KIVEC0_USE(age_kivec0_scratch, age_kivec0)
    AGE_KIVEC_MAX_AGE_MS_USE(age_kivec3_scratch, age_kivec3)
    SESSION_INFO_USE(session_info)
}

action age_round1_session1(SESSION_INFO_DATA)
{
    AGE_KIVEC0_USE(age_kivec0_scratch, age_kivec0)
    AGE_KIVEC_MAX_AGE_MS_USE(age_kivec3_scratch, age_kivec3)
    SESSION_INFO_USE(session_info)
}

action age_round1_session2(SESSION_INFO_DATA)
{
    AGE_KIVEC0_USE(age_kivec0_scratch, age_kivec0)
    AGE_KIVEC_MAX_AGE_MS_USE(age_kivec3_scratch, age_kivec3)
    SESSION_INFO_USE(session_info)
}

action age_round1_session3(SESSION_INFO_DATA)
{
    AGE_KIVEC0_USE(age_kivec0_scratch, age_kivec0)
    AGE_KIVEC_MAX_AGE_MS_USE(age_kivec3_scratch, age_kivec3)
    SESSION_INFO_USE(session_info)
}

action age_round2_session0(SESSION_INFO_DATA)
{
    AGE_KIVEC0_USE(age_kivec0_scratch, age_kivec0)
    AGE_KIVEC_MAX_AGE_MS_USE(age_kivec4_scratch, age_kivec4)
    SESSION_INFO_USE(session_info)
}

action age_round2_session1(SESSION_INFO_DATA)
{
    AGE_KIVEC0_USE(age_kivec0_scratch, age_kivec0)
    AGE_KIVEC_MAX_AGE_MS_USE(age_kivec4_scratch, age_kivec4)
    SESSION_INFO_USE(session_info)
}

action age_round2_session2(SESSION_INFO_DATA)
{
    AGE_KIVEC0_USE(age_kivec0_scratch, age_kivec0)
    AGE_KIVEC_MAX_AGE_MS_USE(age_kivec4_scratch, age_kivec4)
    SESSION_INFO_USE(session_info)
}

action age_round2_session3(SESSION_INFO_DATA)
{
    AGE_KIVEC0_USE(age_kivec0_scratch, age_kivec0)
    AGE_KIVEC_MAX_AGE_MS_USE(age_kivec4_scratch, age_kivec4)
    SESSION_INFO_USE(session_info)
}

action age_round3_session0(SESSION_INFO_DATA)
{
    AGE_KIVEC0_USE(age_kivec0_scratch, age_kivec0)
    AGE_KIVEC_MAX_AGE_MS_USE(age_kivec5_scratch, age_kivec5)
    SESSION_INFO_USE(session_info)
}

action age_round3_session1(SESSION_INFO_DATA)
{
    AGE_KIVEC0_USE(age_kivec0_scratch, age_kivec0)
    AGE_KIVEC_MAX_AGE_MS_USE(age_kivec5_scratch, age_kivec5)
    SESSION_INFO_USE(session_info)
}

action age_round3_session2(SESSION_INFO_DATA)
{
    AGE_KIVEC0_USE(age_kivec0_scratch, age_kivec0)
    AGE_KIVEC_MAX_AGE_MS_USE(age_kivec5_scratch, age_kivec5)
    SESSION_INFO_USE(session_info)
}

action age_round3_session3(SESSION_INFO_DATA)
{
    AGE_KIVEC0_USE(age_kivec0_scratch, age_kivec0)
    AGE_KIVEC_MAX_AGE_MS_USE(age_kivec5_scratch, age_kivec5)
    SESSION_INFO_USE(session_info)
}


/*****************************************************************************
 *  age_summarize : 
 *****************************************************************************/
SESSION_AGE_SUMMARIZE_PRAGMA 
action age_summarize(SESSION_AGE_SUMMARIZE_DATA)
{
    AGE_KIVEC0_USE(age_kivec0_scratch, age_kivec0)
    AGE_KIVEC8_USE(age_kivec8_scratch, age_kivec8)
    SESSION_AGE_SUMMARIZE_USE(session_age_summarize)
}


/*****************************************************************************
 *  age_poller_post : 
 *****************************************************************************/
POLLER_CB_PRAGMA
action age_poller_post(POLLER_CB_DATA)
{
    AGE_KIVEC0_USE(age_kivec0_scratch, age_kivec0)
    AGE_KIVEC7_USE(age_kivec7_scratch, age_kivec7)
    AGE_KIVEC8_USE(age_kivec8_scratch, age_kivec8)
    POLLER_CB_USE(poller_cb)
}


/*****************************************************************************
 *  age_metrics0_commit : Update and commit metrics0 to qstate.
 *****************************************************************************/
SESSION_AGE_METRICS0_PRAGMA
action age_metrics0_commit(SESSION_AGE_METRICS0_DATA)
{
    AGE_KIVEC0_USE(age_kivec0_scratch, age_kivec0)
    AGE_KIVEC9_USE(age_kivec9_scratch, age_kivec9)
    SESSION_AGE_METRICS0_USE(session_age_metrics0)
}

