#ifndef __FTL_DEV_SHARED_H
#define __FTL_DEV_SHARED_H

#include "nic/include/pd.hpp"

/*
 * HBM regions
 */
#define FTL_DEV_AGE_TIMEOUTS_HBM_HANDLE         "ftl_dev_age_tmo"
#define FTL_DEV_POLLER_RINGS_HBM_HANDLE         "ftl_dev_poller_rings"

/*
 * HW control block (i.e., qstate) definitions
 */
#define HW_CB_SINGLE_BYTES                      64
#define HW_CB_SINGLE_BYTES_SHFT                 6

#define HW_CB_MULTIPLE(cb_bytes_shft)           \
    ((cb_bytes_shft) - HW_CB_SINGLE_BYTES_SHFT + 1)


/*
 * MPU stages
 */
#ifndef HW_MPU_STAGE_0
#define HW_MPU_STAGE_0                          0
#define HW_MPU_STAGE_1                          1
#define HW_MPU_STAGE_2                          2
#define HW_MPU_STAGE_3                          3
#define HW_MPU_STAGE_4                          4
#define HW_MPU_STAGE_5                          5
#define HW_MPU_STAGE_6                          6
#define HW_MPU_STAGE_7                          7
#endif

/*
 * MPU general purpose register size in bits
 */
#ifndef HW_MPU_GPR_BITS
#define HW_MPU_GPR_BITS                         64
#endif

/*
 * Session Info (P4 session_info_t)
 */
#define SESSION_INFO_BYTES                      64     // in bytes
#define SESSION_INFO_BYTES_SHFT                 6      // log2(SESSION_INFO_BYTES)
 
/*
 * Session/conntrack lookup types -
 * temporary until provided by P4
 */
#define SCANNER_LKP_TYPE_ICMP                   0
#define SCANNER_LKP_TYPE_UDP                    1
#define SCANNER_LKP_TYPE_UDP_EST                2
#define SCANNER_LKP_TYPE_OTHER                  3
#define SCANNER_LKP_TYPE_TCP_SYN                4
#define SCANNER_LKP_TYPE_TCP_EST                5
#define SCANNER_LKP_TYPE_TCP_FIN                6
#define SCANNER_LKP_TYPE_TCP_TIMEWAIT           7
#define SCANNER_LKP_TYPE_TCP_RST                8
#define SCANNER_LKP_TYPE_UNUSED0                9
#define SCANNER_LKP_TYPE_UNUSED1                10
#define SCANNER_LKP_TYPE_UNUSED2                11
#define SCANNER_LKP_TYPE_UNUSED3                12
#define SCANNER_LKP_TYPE_UNUSED4                13
#define SCANNER_LKP_TYPE_UNUSED5                14
#define SCANNER_LKP_TYPE_UNUSED6                15

/*
 * Control Block activate 16-bit indicator
 */
#define SCANNER_SESSION_CB_ACTIVATE             0xcba1
#define SCANNER_CT_CB_ACTIVATE                  0xcbc1
#define SCANNER_AGE_TMO_CB_ACTIVATE             0xae10

/*
 * Scanner Session Control Block size
 */
#define SCANNER_SESSION_CB_TABLE_BYTES          256
#define SCANNER_SESSION_CB_TABLE_BYTES_SHFT     8
#define SCANNER_SESSION_CB_TABLE_MULTIPLE       (SCANNER_SESSION_CB_TABLE_BYTES_SHFT - \
                                                 HW_CB_SINGLE_BYTES_SHFT + 1)
#define SCANNER_SESSION_CB_TABLE_FSM_OFFSET     (1 * HW_CB_SINGLE_BYTES)
#define SCANNER_SESSION_CB_TABLE_SUMMARIZE_OFFSET (2 * HW_CB_SINGLE_BYTES)
#define SCANNER_SESSION_CB_TABLE_METRICS0_OFFSET  (3 * HW_CB_SINGLE_BYTES)

/*
 * Scanner Conntrack Control Block size
 */
#define SCANNER_CT_CB_TABLE_BYTES                256
#define SCANNER_CT_CB_TABLE_BYTES_SHFT           8
#define SCANNER_CT_CB_TABLE_MULTIPLE             (SCANNER_CT_CB_TABLE_BYTES_SHFT - \
                                                  HW_CB_SINGLE_BYTES_SHFT + 1)
#define SCANNER_CT_CB_TABLE_FSM_OFFSET           (1 * HW_CB_SINGLE_BYTES)
#define SCANNER_CT_CB_TABLE_SUMMARIZE_OFFSET     (2 * HW_CB_SINGLE_BYTES)
#define SCANNER_CT_CB_TABLE_METRICS0_OFFSET      (3 * HW_CB_SINGLE_BYTES)

/*
 * Poller Control Block size
 */
#define POLLER_CB_TABLE_BYTES                   64
#define POLLER_CB_TABLE_BYTES_SHFT              6
#define POLLER_CB_TABLE_MULTIPLE                (POLLER_CB_TABLE_BYTES_SHFT - \
                                                 HW_CB_SINGLE_BYTES_SHFT + 1)
/*
 * Software poller slot data size
 */
#define POLLER_SLOT_DATA_BYTES                  64      // in bytes
#define POLLER_SLOT_DATA_BYTES_SHFT             6       // log2(POLLER_SLOT_DATA_BYTES)


/*
 * Scanner FSM states
 */
#define SCANNER_STATE_RESTART_RANGE             0
#define SCANNER_STATE_RESTART_EXPIRY_MAP        1
#define SCANNER_STATE_SCAN                      2
#define SCANNER_STATE_REEVALUATE                3
#define SCANNER_STATE_MAX                       4

#define SCANNER_STATE_INITIAL                   SCANNER_STATE_RESTART_RANGE

/*
 * Flags to pass to software poller
 */
#define SCANNER_RESCHED_REQUESTED               (1 << 0)

/*
 * Default inactivity timeout values (in seconds)
 */
#define SCANNER_ICMP_TMO_DFLT                   (15)
#define SCANNER_UDP_TMO_DFLT                    (30)
#define SCANNER_UDP_EST_TMO_DFLT                (180)
#define SCANNER_TCP_SYN_TMO_DFLT                (60)
#define SCANNER_TCP_EST_TMO_DFLT                (24 * 60 * 60)
#define SCANNER_TCP_FIN_TMO_DFLT                (120)
#define SCANNER_TCP_RST_TMO_DFLT                (10)
#define SCANNER_TCP_TIMEWAIT_TMO_DFLT           (120)
#define SCANNER_OTHER_TMO_DFLT                  (300)

/*
 * MPU code uses smaller bit widths to store various inactivity timeout
 * values in its k-vec, necessitating the following caps:
 */
#define SCANNER_13BIT_TMO_MAX                   ((1 << 13) - 1) //  2 hours max
#define SCANNER_14BIT_TMO_MAX                   ((1 << 14) - 1) //  4 hours max
#define SCANNER_15BIT_TMO_MAX                   ((1 << 15) - 1) //  9 hours max
#define SCANNER_16BIT_TMO_MAX                   ((1 << 16) - 1) // 18 hours max
#define SCANNER_17BIT_TMO_MAX                   ((1 << 17) - 1) // 36 hours max

#define SCANNER_ICMP_TMO_MAX                    SCANNER_13BIT_TMO_MAX
#define SCANNER_UDP_TMO_MAX                     SCANNER_14BIT_TMO_MAX
#define SCANNER_UDP_EST_TMO_MAX                 SCANNER_14BIT_TMO_MAX
#define SCANNER_TCP_SYN_TMO_MAX                 SCANNER_13BIT_TMO_MAX
#define SCANNER_TCP_EST_TMO_MAX                 SCANNER_17BIT_TMO_MAX
#define SCANNER_TCP_FIN_TMO_MAX                 SCANNER_13BIT_TMO_MAX
#define SCANNER_TCP_RST_TMO_MAX                 SCANNER_13BIT_TMO_MAX
#define SCANNER_TCP_TIMEWAIT_TMO_MAX            SCANNER_13BIT_TMO_MAX
#define SCANNER_OTHER_TMO_MAX                   SCANNER_13BIT_TMO_MAX

/*
 * scanner expiry map size (in bits)
 */
#define SCANNER_EXPIRY_MAP_ENTRY_BITS           64      // in bits
#define SCANNER_EXPIRY_NUM_MAP_ENTRIES_MAX      4
#define SCANNER_EXPIRY_MAP_ENTRIES_TOTAL_BITS   (SCANNER_EXPIRY_NUM_MAP_ENTRIES_MAX * \
                                                 SCANNER_EXPIRY_MAP_ENTRY_BITS)
#define SCANNER_EXPIRY_MAP_ENTRIES_TOTAL_BITS_SHFT 8 // log2(SCANNER_EXPIRY_MAP_ENTRIES_TOTAL_BITS)
 
/*
 * Two types of LIF timer:
 * - Fast: Capri configured resolution is 1us per delta tick
 * - Slow: Capri configured resolution is 1000 * 1us = 1ms per delta tick
 * For both timers, the max config width is 10 bits for a max value of 1023:
 * - Fast: max is 1023us
 * - Slow: max is 1023ms
 */
#define HW_SLOW_TIMER_TICKS_MULT                1000    // multiples over fast timer
#define HW_FAST_TIMER_TICKS_MAX                 1023
#define HW_SLOW_TIMER_TICKS_MAX                 1023

/*
 * scanner reschedule time upon detecting software poller queue full.
 */

#define SCANNER_POLLER_QFULL_REPOST_TIMER       CAPRI_MEM_FAST_TIMER_START
#define SCANNER_POLLER_QFULL_REPOST_TICKS       100  // 100us

/*
 * scanner reschedule time upon range_full but when no non-empty expiry maps
 * have been posted for the same range. Since inactivity timeout granularity
 * is in units of second, it should be fine to start rescanning in
 * 0.5 sec or less.
 */
#define SCANNER_RANGE_EMPTY_RESCHED_TIMER       CAPRI_MEM_SLOW_TIMER_START
#define SCANNER_RANGE_EMPTY_RESCHED_TICKS       300  // 300ms
 
#ifdef __cplusplus

namespace ftl_dev_if {

/**
 * qstate_1ring_cb_t - Standard qstate control block, with 1 set of
 *                     ring inddices
 */
typedef struct {
    uint8_t                 pc_offset;
    uint8_t                 rsvd0;
    uint8_t                 cosA            : 4,
                            cosB            : 4;
    uint8_t                 cos_sel;
    uint8_t                 eval_last;
    uint8_t                 host_wrings     : 4,
                            total_wrings    : 4;
    uint16_t                pid;
    uint16_t                p_ndx0;
    uint16_t                c_ndx0;
} __attribute__((packed)) qstate_1ring_cb_t;

/**
 * scanner_session_cb_t - Scanner Session control block for stage 0
 */
typedef uint16_t            scanner_session_cb_activate_t;

typedef struct {
    qstate_1ring_cb_t       qstate_1ring;
    uint32_t                scan_resched_ticks;
    uint64_t                normal_tmo_cb_addr;
    uint64_t                accel_tmo_cb_addr;
    uint8_t                 resched_uses_slow_timer;
    uint8_t                 pad[29];
    scanner_session_cb_activate_t cb_activate;    // must be last in CB
} __attribute__((packed)) scanner_session_cb_t;

/**
 * scanner_session_fsm_t - Scanner Session FSM states for stage 1
 */
typedef struct {
    uint8_t                 fsm_state;      // must be first in CB
    uint8_t                 pad0;
    uint16_t                scan_burst_sz_shft;
    uint32_t                scan_burst_sz;
    uint32_t                scan_id_base;
    uint32_t                scan_id_next;
    uint32_t                scan_range_sz;
    uint32_t                expiry_scan_id_base;
    uint32_t                expiry_map_entries_scanned;
    uint32_t                total_entries_scanned;
    uint64_t                scan_addr_base;
    uint16_t                expiry_map_bit_pos;
    uint8_t                 pad1[20];
    scanner_session_cb_activate_t cb_activate;    // must be last in CB
}  __attribute__((packed)) scanner_session_fsm_t;

/**
 * scanner_session_summarize_t - Scanner Session summarization for stage N
 */
typedef struct {
    uint8_t                 poller_qdepth_shft;
    uint8_t                 range_has_posted;
    uint8_t                 pad0[2];
    uint64_t                poller_qstate_addr;
    uint64_t                expiry_map0;
    uint64_t                expiry_map1;
    uint64_t                expiry_map2;
    uint64_t                expiry_map3;
    uint8_t                 pad1[18];
    scanner_session_cb_activate_t cb_activate;    // must be last in CB
}  __attribute__((packed)) scanner_session_summarize_t;

/**
 * scanner_session_metrics0_t - Scanner Session metrics
 */
typedef struct {
    uint64_t                cb_cfg_discards;
    uint64_t                scan_invocations;
    uint64_t                expired_entries;
    uint64_t                pad[5];
}  __attribute__((packed)) scanner_session_metrics0_t;

/**
 * scanner_session_qstate_t - Scanner Session queue state
 */
typedef struct {
    scanner_session_cb_t        cb;
    scanner_session_fsm_t       fsm;
    scanner_session_summarize_t summarize;
    scanner_session_metrics0_t  metrics0;
} __attribute__((packed)) scanner_session_qstate_t;


/**
 * scanner_ct_cb_t - Scanner Conntrack control block for stage 0
 */
typedef uint16_t            scanner_ct_cb_activate_t;

typedef struct {
    qstate_1ring_cb_t       qstate_1ring;
    uint32_t                scan_resched_ticks;
    uint64_t                normal_tmo_cb_addr;
    uint64_t                accel_tmo_cb_addr;
    uint8_t                 resched_uses_slow_timer;
    uint8_t                 pad[29];
    scanner_ct_cb_activate_t cb_activate;    // must be last in CB
} __attribute__((packed)) scanner_ct_cb_t;

/**
 * scanner_ct_fsm_t - Scanner Conntrack FSM states for stage 1
 */
typedef struct {
    uint8_t                 fsm_state;      // must be first in CB
    uint8_t                 pad0;
    uint16_t                scan_burst_sz_shft;
    uint32_t                scan_burst_sz;
    uint32_t                scan_id_base;
    uint32_t                scan_id_next;
    uint32_t                scan_range_sz;
    uint32_t                expiry_scan_id_base;
    uint32_t                expiry_map_entries_scanned;
    uint32_t                total_entries_scanned;
    uint64_t                scan_addr_base;
    uint8_t                 pad1[22];
    scanner_ct_cb_activate_t cb_activate;    // must be last in CB
}  __attribute__((packed)) scanner_ct_fsm_t;

/**
 * scanner_ct_summarize_t - Scanner Conntrack summarization for stage N
 */
typedef struct {
    uint8_t                 poller_qdepth_shft;
    uint8_t                 pad0[3];
    uint64_t                poller_qstate_addr;
    uint64_t                expiry_map0;
    uint64_t                expiry_map1;
    uint64_t                expiry_map2;
    uint64_t                expiry_map3;
    uint8_t                 pad1[18];
    scanner_ct_cb_activate_t cb_activate;    // must be last in CB
}  __attribute__((packed)) scanner_ct_summarize_t;

/**
 * scanner_ct_metrics0_t - Scanner Conntrack metrics
 */
typedef struct {
    uint64_t                cb_cfg_discards;
    uint64_t                scan_invocations;
    uint64_t                expired_entries;
    uint64_t                pad[5];
}  __attribute__((packed)) scanner_ct_metrics0_t;

/**
 * scanner_ct_qstate_t - Scanner Conntrack queue state
 */
typedef struct {
    scanner_ct_cb_t         cb;
    scanner_ct_fsm_t        fsm;
    scanner_ct_summarize_t  summarize;
    scanner_ct_metrics0_t   metrics0;
} __attribute__((packed)) scanner_ct_qstate_t;

/**
 * poller_qstate_t - Poller control block
 */
typedef struct {
    qstate_1ring_cb_t       qstate_1ring;
    uint16_t                pi_0_shadow;
    uint8_t                 qdepth_shft;
    uint64_t                wring_base_addr;
    uint64_t                num_qfulls;
    uint8_t                 pad[33];
} __attribute__((packed)) poller_qstate_t;

/**
 * poller_slot_data_t - Poller slot data (as written to work ring)
 */
typedef struct {
    uint32_t                table_id_base;
    uint32_t                scanner_qid;
    uint64_t                expiry_map[SCANNER_EXPIRY_NUM_MAP_ENTRIES_MAX];
    uint8_t                 scanner_qtype;
    uint8_t                 flags;
    uint8_t                 pad[22];
} __attribute__((packed)) poller_slot_data_t;

/**
 * age_tmo_cb_t - Aging timeout values (in seconds),
 *                stored separately in dedicated HBM regions
 */
typedef uint16_t            age_tmo_cb_activate_t;
typedef uint8_t             age_tmo_cb_select_t;

typedef struct {
    age_tmo_cb_activate_t   cb_activate;    // must be first in CB
    age_tmo_cb_select_t     cb_select;
    uint8_t                 pad0;
    uint32_t                tcp_syn_tmo;
    uint32_t                tcp_est_tmo;
    uint32_t                tcp_fin_tmo;
    uint32_t                tcp_timewait_tmo;
    uint32_t                tcp_rst_tmo;
    uint32_t                icmp_tmo;
    uint32_t                udp_tmo;
    uint32_t                udp_est_tmo;
    uint32_t                other_tmo;
    uint8_t                 pad1[24];
} __attribute__((packed)) age_tmo_cb_t;

/**
 * Convert time in microseconds to scheduler timer ticks
 */
static inline uint32_t
time_us_to_timer_ticks(uint32_t time_us,
                       uint8_t *use_slow_timer)
{
    uint32_t    ticks = time_us;

    *use_slow_timer = false;
    if (time_us > HW_FAST_TIMER_TICKS_MAX) {
        ticks = (time_us + HW_SLOW_TIMER_TICKS_MULT - 1) / HW_SLOW_TIMER_TICKS_MULT;
        if (ticks > HW_SLOW_TIMER_TICKS_MAX) {
            ticks = HW_SLOW_TIMER_TICKS_MAX;
        }
        *use_slow_timer = true;
    }
    return ticks;
}

} // namespace ftl_dev_if

#endif // __cplusplus

#endif //__FTL_DEV_SHARED_H
