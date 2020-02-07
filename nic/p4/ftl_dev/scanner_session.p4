/*********************************************************************************
 * Scanner Session (Aging)
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
#define tx_table_s0_t0_action   session_start


/*
 * Table names
 */
#define tx_table_s0_t0          s0_tbl
#define tx_table_s1_t0          s1_tbl
#define tx_table_s1_t1          s1_tbl1
#define tx_table_s1_t2          s1_tbl2
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
#define tx_table_s1_t0_action   session_fsm_exec
#define tx_table_s1_t1_action   session_norm_tmo_load
#define tx_table_s1_t2_action   session_accel_tmo_load

/*
 * stage 2
 */
#define tx_table_s2_t0_action   session_round0_session0
#define tx_table_s2_t1_action   session_round0_session1
#define tx_table_s2_t2_action   session_round0_session2
#define tx_table_s2_t3_action   session_round0_session3
#define tx_table_s2_t0_action1  conntrack_round0_session0
#define tx_table_s2_t1_action1  conntrack_round0_session1
#define tx_table_s2_t2_action1  conntrack_round0_session2
#define tx_table_s2_t3_action1  conntrack_round0_session3

/*
 * stage 3
 */
#define tx_table_s3_t0_action   session_round1_session0
#define tx_table_s3_t1_action   session_round1_session1
#define tx_table_s3_t2_action   session_round1_session2
#define tx_table_s3_t3_action   session_round1_session3
#define tx_table_s3_t0_action1  conntrack_round1_session0
#define tx_table_s3_t1_action1  conntrack_round1_session1
#define tx_table_s3_t2_action1  conntrack_round1_session2
#define tx_table_s3_t3_action1  conntrack_round1_session3

/*
 * stage 4
 */
#define tx_table_s4_t0_action   session_round2_session0
#define tx_table_s4_t1_action   session_round2_session1
#define tx_table_s4_t2_action   session_round2_session2
#define tx_table_s4_t3_action   session_round2_session3
#define tx_table_s4_t0_action1  conntrack_round2_session0
#define tx_table_s4_t1_action1  conntrack_round2_session1
#define tx_table_s4_t2_action1  conntrack_round2_session2
#define tx_table_s4_t3_action1  conntrack_round2_session3

/*
 * stage 5
 */
#define tx_table_s5_t0_action   session_round3_session0
#define tx_table_s5_t1_action   session_round3_session1
#define tx_table_s5_t2_action   session_round3_session2
#define tx_table_s5_t3_action   session_round3_session3
#define tx_table_s5_t0_action1  conntrack_round3_session0
#define tx_table_s5_t1_action1  conntrack_round3_session1
#define tx_table_s5_t2_action1  conntrack_round3_session2
#define tx_table_s5_t3_action1  conntrack_round3_session3

/*
 * stage 6
 */
#define tx_table_s6_t0_action   session_summarize

/*
 * stage 7
 */
#define tx_table_s7_t0_action   session_poller_post
#define tx_table_s7_t3_action   session_metrics0_commit


#if defined(ARTEMIS)
#include "nic/apollo/p4/artemis_txdma/common_txdma.p4"
#elif defined(ATHENA)
#include "nic/apollo/p4/athena_txdma/common_txdma.p4"
#else
#include "nic/p4/common-p4+/common_txdma.p4"
#endif

#include "include/ftl_dev_shared_p4.h"


/*
 * kivec0: header union with global (128 bits max)
 */
header_type session_kivec0_t {
    fields {
        qstate_addr                     : 34;
        session_table_addr              : 34;
        session_id_curr                 : 32;
        num_scannables                  : 5;
        qtype                           : 1;    // currently only 2 scanner qtypes supported
        cb_cfg_discard                  : 1;
        force_session_expired_ts        : 1;    // for SIM platform debug
        force_conntrack_expired_ts      : 1;
        
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

#define SESSION_KIVEC0_USE(scratch, kivec)                                      \
    modify_field(scratch.qstate_addr, kivec.qstate_addr);                       \
    modify_field(scratch.session_table_addr, kivec.session_table_addr);         \
    modify_field(scratch.session_id_curr, kivec.session_id_curr);               \
    modify_field(scratch.num_scannables, kivec.num_scannables);                 \
    modify_field(scratch.qtype, kivec.qtype);                                   \
    modify_field(scratch.cb_cfg_discard, kivec.cb_cfg_discard);                 \
    modify_field(scratch.force_session_expired_ts, kivec.force_session_expired_ts);\
    modify_field(scratch.force_conntrack_expired_ts, kivec.force_conntrack_expired_ts);\
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
 * for holding inactivity timeout values (in seconds) of supported
 * flow lookup types:
 *
 * tcp_est_tmo - 17 bits, max is 36 hours
 * udp_tmo, udp_est_tmo - 14 bits, max is 4 hours
 * session_tmo is overlaid with others_tmo - 14 bits, max is 4 hours
 * all other timeouts - 13 bits, max is 2 hours
 */
#define SESSION_KIVEC_AGE_TMO(_vec)                                             \
header_type session_kivec##_vec##_t {                                           \
    fields {                                                                    \
        icmp_tmo                        : 13;                                   \
        udp_tmo                         : 14;                                   \
        udp_est_tmo                     : 14;                                   \
        tcp_syn_tmo                     : 13;                                   \
        tcp_est_tmo                     : 17;                                   \
        tcp_fin_tmo                     : 13;                                   \
        tcp_timewait_tmo                : 13;                                   \
        tcp_rst_tmo                     : 13;                                   \
        others_tmo                      : 14;                                   \
    }                                                                           \
}                                                                               \

SESSION_KIVEC_AGE_TMO(2)
SESSION_KIVEC_AGE_TMO(3)
SESSION_KIVEC_AGE_TMO(4)
SESSION_KIVEC_AGE_TMO(5)

#define SESSION_KIVEC_AGE_TMO_USE(scratch, kivec)                               \
    modify_field(scratch.icmp_tmo, kivec.icmp_tmo);                             \
    modify_field(scratch.udp_tmo, kivec.udp_tmo);                               \
    modify_field(scratch.udp_est_tmo, kivec.udp_est_tmo);                       \
    modify_field(scratch.tcp_syn_tmo, kivec.tcp_syn_tmo);                       \
    modify_field(scratch.tcp_est_tmo, kivec.tcp_est_tmo);                       \
    modify_field(scratch.tcp_fin_tmo, kivec.tcp_fin_tmo);                       \
    modify_field(scratch.tcp_timewait_tmo, kivec.tcp_timewait_tmo);             \
    modify_field(scratch.tcp_rst_tmo, kivec.tcp_rst_tmo);                       \
    modify_field(scratch.others_tmo, kivec.others_tmo);                         \
    
/*
 * kivec7: header union with to_stage7 (128 bits max)
 */
header_type session_kivec7_t {
    fields {
        lif                             : 11;
        poller_qstate_addr              : 64;
    }
}
        
#define SESSION_KIVEC7_USE(scratch, kivec)                                      \
    modify_field(scratch.lif, kivec.lif);                                       \
    modify_field(scratch.poller_qstate_addr, kivec.poller_qstate_addr);         \
    
/*
 * kivec8: header union with stage_2_stage for table 0 (160 bits max)
 */
header_type session_kivec8_t {
    fields {
        expiry_id_base                  : 32;
        expiry_map_bit_pos              : 9;
        range_full                      : 1;
        burst_full                      : 1;
        expiry_maps_full                : 1;
        range_has_posted                : 1;
        resched_uses_slow_timer         : 1;
    }
}

#define SESSION_KIVEC8_USE(scratch, kivec)                                      \
    modify_field(scratch.expiry_id_base, kivec.expiry_id_base);                 \
    modify_field(scratch.expiry_map_bit_pos, kivec.expiry_map_bit_pos);         \
    modify_field(scratch.range_full, kivec.range_full);                         \
    modify_field(scratch.burst_full, kivec.burst_full);                         \
    modify_field(scratch.expiry_maps_full, kivec.expiry_maps_full);             \
    modify_field(scratch.range_has_posted, kivec.range_has_posted);             \
    modify_field(scratch.resched_uses_slow_timer, kivec.resched_uses_slow_timer);\
    
/*
 * kivec9: header union with stage_2_stage for table 3 (160 bits max)
 */
header_type session_kivec9_t {
    fields {
        metrics0_start                  : 1;
        cb_cfg_discards                 : 1;
        scan_invocations                : 1;
        range_elapsed_ticks             : 48;
        metrics0_end                    : 1;
    }
}

#define SESSION_KIVEC9_USE(scratch, kivec)                                      \
    modify_field(scratch.metrics0_start, kivec.metrics0_start);                 \
    modify_field(scratch.cb_cfg_discards, kivec.cb_cfg_discards);               \
    modify_field(scratch.scan_invocations, kivec.scan_invocations);             \
    modify_field(scratch.range_elapsed_ticks, kivec.range_elapsed_ticks);       \
    modify_field(scratch.metrics0_end, kivec.metrics0_end);                     \
    
/*
 * Header unions for d-vector
 */
@pragma scratch_metadata
metadata scanner_session_cb_t           session_cb;

@pragma scratch_metadata
metadata scanner_session_fsm_t          session_fsm;

@pragma scratch_metadata
metadata age_tmo_cb_t                   session_norm_tmo;

@pragma scratch_metadata
metadata age_tmo_cb_t                   session_accel_tmo;

@pragma scratch_metadata
metadata session_info_d                 session_info;

@pragma scratch_metadata
metadata conntrack_info_d               conntrack_info;

@pragma scratch_metadata
metadata scanner_session_summarize_t    session_summarize;

@pragma scratch_metadata
metadata scanner_session_metrics0_t     session_metrics0;

@pragma scratch_metadata
metadata poller_cb_t                    poller_cb;

/*
 * Header unions for PHV layout
 */
@pragma pa_header_union ingress         common_global
metadata session_kivec0_t               session_kivec0;
@pragma scratch_metadata
metadata session_kivec0_t               session_kivec0_scratch;

@pragma dont_trim
@pragma pa_header_union ingress         to_stage_2
metadata session_kivec2_t               session_kivec2;
@pragma scratch_metadata
metadata session_kivec2_t               session_kivec2_scratch;

@pragma dont_trim
@pragma pa_header_union ingress         to_stage_3
metadata session_kivec3_t               session_kivec3;
@pragma scratch_metadata
metadata session_kivec3_t               session_kivec3_scratch;

@pragma dont_trim
@pragma pa_header_union ingress         to_stage_4
metadata session_kivec4_t               session_kivec4;
@pragma scratch_metadata
metadata session_kivec4_t               session_kivec4_scratch;

@pragma dont_trim
@pragma pa_header_union ingress         to_stage_5
metadata session_kivec5_t               session_kivec5;
@pragma scratch_metadata
metadata session_kivec5_t               session_kivec5_scratch;

@pragma dont_trim
@pragma pa_header_union ingress         to_stage_7
metadata session_kivec7_t               session_kivec7;
@pragma scratch_metadata
metadata session_kivec7_t               session_kivec7_scratch;

@pragma pa_header_union ingress         common_t0_s2s
metadata session_kivec8_t               session_kivec8;
@pragma scratch_metadata
metadata session_kivec8_t               session_kivec8_scratch;

@pragma dont_trim
@pragma pa_header_union ingress         common_t3_s2s
metadata session_kivec9_t               session_kivec9;
@pragma scratch_metadata
metadata session_kivec9_t               session_kivec9_scratch;

/*
 * PHV following k (for app DMA etc.)
 */
@pragma dont_trim
metadata ring_entry_t                   ring_entry; 

@pragma dont_trim
metadata doorbell_data_raw_t            db_data_no_index;

@pragma dont_trim
metadata doorbell_data_raw_t            db_data_burst_ticks;

@pragma dont_trim
metadata doorbell_data_raw_t            db_data_qfull_repost_ticks;

@pragma dont_trim
@pragma pa_align 512
metadata poller_slot_data_t             poller_slot_data; 

@pragma dont_trim
metadata doorbell_data_raw_t            db_data_range_empty_ticks;

@pragma dont_trim
metadata ring_pi_t                      poller_posted_pi; 

@pragma dont_trim
metadata scanner_session_fsm_state_t    fsm_state_next; 

@pragma dont_trim
metadata poller_range_has_posted_t      poller_range_has_posted;

@pragma dont_trim
@pragma pa_align 512
metadata poller_slot_data_t             poller_null_data; 

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

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_8;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_8
metadata dma_cmd_mem2mem_t dma_m2m_8;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_9;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_9
metadata dma_cmd_mem2mem_t dma_m2m_9;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_10;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_10
metadata dma_cmd_mem2mem_t dma_m2m_10;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_11;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_11
metadata dma_cmd_mem2mem_t dma_m2m_11;


/*
 * Action functions to generate k_struct and d_struct
 *
 * These action functions are currently only to generate the k+i and d structs
 * and do not implement any pseudo code
 */

/*****************************************************************************
 *  Initial stage
 *****************************************************************************/
SCANNER_SESSION_CB_PRAGMA
action session_start(SCANNER_SESSION_CB_DATA)
{
    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);

    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SCANNER_SESSION_CB_USE(session_cb)
}


/*****************************************************************************
 *  Execute FSM
 *****************************************************************************/
SCANNER_SESSION_FSM_PRAGMA
action session_fsm_exec(SCANNER_SESSION_FSM_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SCANNER_SESSION_FSM_USE(session_fsm)
}


/*****************************************************************************
 *  Load normal aging timeout values
 *****************************************************************************/
SCANNER_AGE_TMO_CB_PRAGMA
action session_norm_tmo_load(SCANNER_AGE_TMO_CB_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SCANNER_AGE_TMO_CB_USE(session_norm_tmo)
}

/*****************************************************************************
 *  Load accelerated aging timeout values
 *****************************************************************************/
SCANNER_AGE_TMO_CB_PRAGMA
action session_accel_tmo_load(SCANNER_AGE_TMO_CB_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SCANNER_AGE_TMO_CB_USE(session_accel_tmo)
}

/*****************************************************************************
 *  Round (i.e., stage) 0 session 0 age
 *****************************************************************************/
action session_round0_session0(SESSION_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec2_scratch, session_kivec2)
    SESSION_INFO_USE(session_info)
}

action session_round0_session1(SESSION_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec2_scratch, session_kivec2)
    SESSION_INFO_USE(session_info)
}

action session_round0_session2(SESSION_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec2_scratch, session_kivec2)
    SESSION_INFO_USE(session_info)
}

action session_round0_session3(SESSION_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec2_scratch, session_kivec2)
    SESSION_INFO_USE(session_info)
}

action session_round1_session0(SESSION_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec3_scratch, session_kivec3)
    SESSION_INFO_USE(session_info)
}

action session_round1_session1(SESSION_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec3_scratch, session_kivec3)
    SESSION_INFO_USE(session_info)
}

action session_round1_session2(SESSION_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec3_scratch, session_kivec3)
    SESSION_INFO_USE(session_info)
}

action session_round1_session3(SESSION_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec3_scratch, session_kivec3)
    SESSION_INFO_USE(session_info)
}

action session_round2_session0(SESSION_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec4_scratch, session_kivec4)
    SESSION_INFO_USE(session_info)
}

action session_round2_session1(SESSION_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec4_scratch, session_kivec4)
    SESSION_INFO_USE(session_info)
}

action session_round2_session2(SESSION_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec4_scratch, session_kivec4)
    SESSION_INFO_USE(session_info)
}

action session_round2_session3(SESSION_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec4_scratch, session_kivec4)
    SESSION_INFO_USE(session_info)
}

action session_round3_session0(SESSION_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec5_scratch, session_kivec5)
    SESSION_INFO_USE(session_info)
}

action session_round3_session1(SESSION_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec5_scratch, session_kivec5)
    SESSION_INFO_USE(session_info)
}

action session_round3_session2(SESSION_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec5_scratch, session_kivec5)
    SESSION_INFO_USE(session_info)
}

action session_round3_session3(SESSION_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec5_scratch, session_kivec5)
    SESSION_INFO_USE(session_info)
}

action conntrack_round0_session0(CONNTRACK_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec2_scratch, session_kivec2)
    CONNTRACK_INFO_USE(conntrack_info)
}

action conntrack_round0_session1(CONNTRACK_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec2_scratch, session_kivec2)
    CONNTRACK_INFO_USE(conntrack_info)
}

action conntrack_round0_session2(CONNTRACK_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec2_scratch, session_kivec2)
    CONNTRACK_INFO_USE(conntrack_info)
}

action conntrack_round0_session3(CONNTRACK_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec2_scratch, session_kivec2)
    CONNTRACK_INFO_USE(conntrack_info)
}

action conntrack_round1_session0(CONNTRACK_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec3_scratch, session_kivec3)
    CONNTRACK_INFO_USE(conntrack_info)
}

action conntrack_round1_session1(CONNTRACK_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec3_scratch, session_kivec3)
    CONNTRACK_INFO_USE(conntrack_info)
}

action conntrack_round1_session2(CONNTRACK_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec3_scratch, session_kivec3)
    CONNTRACK_INFO_USE(conntrack_info)
}

action conntrack_round1_session3(CONNTRACK_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec3_scratch, session_kivec3)
    CONNTRACK_INFO_USE(conntrack_info)
}

action conntrack_round2_session0(CONNTRACK_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec4_scratch, session_kivec4)
    CONNTRACK_INFO_USE(conntrack_info)
}

action conntrack_round2_session1(CONNTRACK_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec4_scratch, session_kivec4)
    CONNTRACK_INFO_USE(conntrack_info)
}

action conntrack_round2_session2(CONNTRACK_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec4_scratch, session_kivec4)
    CONNTRACK_INFO_USE(conntrack_info)
}

action conntrack_round2_session3(CONNTRACK_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec4_scratch, session_kivec4)
    CONNTRACK_INFO_USE(conntrack_info)
}

action conntrack_round3_session0(CONNTRACK_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec5_scratch, session_kivec5)
    CONNTRACK_INFO_USE(conntrack_info)
}

action conntrack_round3_session1(CONNTRACK_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec5_scratch, session_kivec5)
    CONNTRACK_INFO_USE(conntrack_info)
}

action conntrack_round3_session2(CONNTRACK_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec5_scratch, session_kivec5)
    CONNTRACK_INFO_USE(conntrack_info)
}

action conntrack_round3_session3(CONNTRACK_INFO_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC_AGE_TMO_USE(session_kivec5_scratch, session_kivec5)
    CONNTRACK_INFO_USE(conntrack_info)
}


/*****************************************************************************
 *  Summarize result into bitmap
 *****************************************************************************/
SCANNER_SESSION_SUMMARIZE_PRAGMA 
action session_summarize(SCANNER_SESSION_SUMMARIZE_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC8_USE(session_kivec8_scratch, session_kivec8)
    SCANNER_SESSION_SUMMARIZE_USE(session_summarize)
}


/*****************************************************************************
 *  Post slot data to poller
 *****************************************************************************/
POLLER_CB_PRAGMA
action session_poller_post(POLLER_CB_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC7_USE(session_kivec7_scratch, session_kivec7)
    SESSION_KIVEC8_USE(session_kivec8_scratch, session_kivec8)
    POLLER_CB_USE(poller_cb)
}


/*****************************************************************************
 *  Update and commit metrics0 to qstate.
 *****************************************************************************/
SCANNER_SESSION_METRICS0_PRAGMA
action session_metrics0_commit(SCANNER_SESSION_METRICS0_DATA)
{
    SESSION_KIVEC0_USE(session_kivec0_scratch, session_kivec0)
    SESSION_KIVEC9_USE(session_kivec9_scratch, session_kivec9)
    SCANNER_SESSION_METRICS0_USE(session_metrics0)
}

