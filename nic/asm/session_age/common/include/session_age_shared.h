#ifndef __SESSION_AGE_SHARED_H
#define __SESSION_AGE_SHARED_H

#include "nic/include/pd.hpp"

/*
 * This file contains definitions that are shared with P4+ asm code.
 * Do not insert C constructs that cannot be compiled by NCC assembler.
 */

#ifndef CAPRI_QSTATE_HEADER_COMMON_BYTES
#define CAPRI_QSTATE_HEADER_COMMON_BYTES        8  /* including action_id byte */ 
#endif

#ifndef CAPRI_QSTATE_HEADER_RING_SINGLE_BYTES
#define CAPRI_QSTATE_HEADER_RING_SINGLE_BYTES   4  /* pair of 16-bit PI/CI */
#endif

#ifndef CAPRI_CB_TABLE_SINGLE_BYTES
#define CAPRI_CB_TABLE_SINGLE_BYTES             64
#define CAPRI_CB_TABLE_SINGLE_BYTES_SHFT        6
#endif

/*
 * MPU stages
 */
#ifndef CAPRI_STAGE_0
#define CAPRI_STAGE_0                           0
#define CAPRI_STAGE_1                           1
#define CAPRI_STAGE_2                           2
#define CAPRI_STAGE_3                           3
#define CAPRI_STAGE_4                           4
#define CAPRI_STAGE_5                           5
#define CAPRI_STAGE_6                           6
#define CAPRI_STAGE_7                           7
#endif

/*
 * MPU general purpose register size in bits
 */
#ifndef CAPRI_MPU_GPR_BITS
#define CAPRI_MPU_GPR_BITS                      64
#endif

/*
 * Software poller slot data size
 */
#define SESSION_AGE_POLLER_SLOT_BYTES           64      // in bytes
#define SESSION_AGE_POLLER_SLOT_BYTES_SHFT      6       // log2(SESSION_AGE_POLLER_SLOT_BYTES)

/*
 * Session Info (P4 session_info_t)
 */
#define SESSION_INFO_BYTES                      64     // in bytes
#define SESSION_INFO_BYTES_SHFT                 6      // log2(SESSION_INFO_BYTES)
 
/*
 * Control Block activate 16-bit indicator
 */
#define SESSION_AGE_CB_ACTIVATE                 0xcba1

/*
 * Session Aging Control Block size
 */
#define SESSION_AGE_CB_TABLE_BYTES              256
#define SESSION_AGE_CB_TABLE_BYTES_SHFT         8
#define SESSION_AGE_CB_TABLE_MULTIPLE           (SESSION_AGE_CB_TABLE_BYTES_SHFT - \
                                                 CAPRI_CB_TABLE_SINGLE_BYTES_SHFT + 1)
#define SESSION_AGE_CB_TABLE_FSM_OFFSET         (1 * CAPRI_CB_TABLE_SINGLE_BYTES)
#define SESSION_AGE_CB_TABLE_SUMMARIZE_OFFSET   (2 * CAPRI_CB_TABLE_SINGLE_BYTES)
#define SESSION_AGE_CB_TABLE_METRICS0_OFFSET    (3 * CAPRI_CB_TABLE_SINGLE_BYTES)


/*
 * Session Aging FSM states
 */
#define SESSION_AGE_STATE_RESTART_RANGE         0
#define SESSION_AGE_STATE_RESTART_EXPIRY_MAP    1
#define SESSION_AGE_STATE_SCAN                  2
#define SESSION_AGE_STATE_REEVALUATE            3
#define SESSION_AGE_STATE_MAX                   4

/*
 * Flags to pass to software poller
 */
#define SESSION_AGE_SCANNER_RESCHED_REQUESTED   (1 << 0)

/*
 * Session lookup types -
 * temporary until provided by P4
 */
#define SESSION_LKP_TYPE_ICMP                   0
#define SESSION_LKP_TYPE_UDP                    1
#define SESSION_LKP_TYPE_TCP                    2
#define SESSION_LKP_TYPE_OTHER                  3

/*
 * Session Aging expiry map size (in bits)
 */
#define SESSION_AGE_EXPIRY_MAP_ENTRY_BITS       64      // in bits
#define SESSION_AGE_EXPIRY_NUM_MAP_ENTRIES_MAX  4
#define SESSION_AGE_EXPIRY_MAP_ENTRIES_TOTAL_BITS (SESSION_AGE_EXPIRY_NUM_MAP_ENTRIES_MAX * \
                                                 SESSION_AGE_EXPIRY_MAP_ENTRY_BITS)
#define SESSION_AGE_EXPIRY_MAP_ENTRIES_TOTAL_BITS_SHFT 8 // log2(SESSION_AGE_EXPIRY_MAP_ENTRIES_TOTAL_BITS)
 
/*
 * Session Aging reschedule time upon detecting software poller queue full
 */
#define SESSION_AGE_POLLER_QFULL_REPOST_TIME    100     // in Capri system ticks

#endif //__SESSION_AGE_SHARED_H
