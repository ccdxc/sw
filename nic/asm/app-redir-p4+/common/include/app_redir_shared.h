#ifndef __APP_REDIR_SHARED_H
#define __APP_REDIR_SHARED_H

/*
 * This file contains app redirect definitions that are shared with P4+ asm code.
 * Do not insert C constructs that cannot be compiled by NCC assembler.
 */

#ifndef CAPRI_QSTATE_HEADER_COMMON_SIZE
#define CAPRI_QSTATE_HEADER_COMMON_SIZE     8  /* including action_id byte */ 
#endif

#ifndef CAPRI_QSTATE_HEADER_RING_SINGLE_SIZE
#define CAPRI_QSTATE_HEADER_RING_SINGLE_SIZE 4  /* pair of 16-bit PI/CI */
#endif

#ifndef CAPRI_CB_TABLE_ENTRY_SINGLE_SIZE
#define CAPRI_CB_TABLE_ENTRY_SINGLE_SIZE    64
#define CAPRI_CB_TABLE_ENTRY_SINGLE_SHFT    6
#endif

/*
 * MPU stages
 */
#ifndef CAPRI_STAGE_0
#define CAPRI_STAGE_0                       0
#define CAPRI_STAGE_1                       1
#define CAPRI_STAGE_2                       2
#define CAPRI_STAGE_3                       3
#define CAPRI_STAGE_4                       4
#define CAPRI_STAGE_5                       5
#define CAPRI_STAGE_6                       6
#define CAPRI_STAGE_7                       7
#endif

/*
 * App redirect queue types
 */
#define APP_REDIR_RAWR_QTYPE                0   /* raw redirect */
#define APP_REDIR_RAWC_QTYPE                1   /* raw chain */
#define APP_REDIR_PROXYR_QTYPE              2   /* TCP/TLS proxy redirect */
#define APP_REDIR_PROXYC_QTYPE              3   /* TCP/TLS proxy chain */
#define APP_REDIR_NUM_QTYPES_MAX            4

/*
 * App redirect rings
 */
#define APP_REDIR_RAWR_RINGS_MAX            0
#define APP_REDIR_RAWC_RINGS_MAX            1
#define APP_REDIR_PROXYR_RINGS_MAX          1
#define APP_REDIR_PROXYC_RINGS_MAX          1
 
/*
 * Two ways of supporting visibility mode: using a mirror session or 
 * ingress replication. 
 *
 * Mirror session has a limitation that P4 only parses up to the L2 header 
 * on the span packet so we'd have to work around that with 
 * app_redir_rx_span_parse_workaround() below.
 *
 * Ingress replication does not have such limitation but is available for
 * ingress copy only. In addition, replication will back pressure the source
 * if either the flow's destination queue or the replication-to queue is full.
 *
 * In the case of mirror session, the span copy would be dropped if the
 * mirror-to queue were full.
 * 
 */
#define APP_REDIR_VISIBILITY_USE_MIRROR_SESSION     0


/*
 * rawrcb/rawccb creation for span;
 * HW always spans to qtype 0 and qid 0, lif is derived from the mirror
 * session's lport_id. Once in rawrcb, P4+ will hash on flow and spray to
 * appropriate ARQ.
 */
#define APP_REDIR_SPAN_RAWRCB_ID            0

/*
 * Raw Redirect Control Block sizes
 */
#define RAWRCB_TABLE_ENTRY_SIZE             256
#define RAWRCB_TABLE_ENTRY_SIZE_SHFT        8
#define RAWRCB_TABLE_ENTRY_MULTIPLE         (RAWRCB_TABLE_ENTRY_SIZE_SHFT - \
                                             CAPRI_CB_TABLE_ENTRY_SINGLE_SHFT + 1)
#define RAWRCB_TABLE_EXTRA_OFFSET           (1 * CAPRI_CB_TABLE_ENTRY_SINGLE_SIZE)
#define RAWRCB_TABLE_METRICS0_OFFSET        (2 * CAPRI_CB_TABLE_ENTRY_SINGLE_SIZE)

/*
 * Raw Chain Control Block sizes
 */
#define RAWCCB_TABLE_ENTRY_SIZE             256
#define RAWCCB_TABLE_ENTRY_SIZE_SHFT        8
#define RAWCCB_TABLE_ENTRY_MULTIPLE         (RAWCCB_TABLE_ENTRY_SIZE_SHFT - \
                                             CAPRI_CB_TABLE_ENTRY_SINGLE_SHFT + 1)
#define RAWCCB_TABLE_EXTRA_OFFSET           (1 * CAPRI_CB_TABLE_ENTRY_SINGLE_SIZE)
#define RAWCCB_TABLE_METRICS0_OFFSET        (2 * CAPRI_CB_TABLE_ENTRY_SINGLE_SIZE)

/*
 * Proxy Redirect Control Block sizes
 */
#define PROXYRCB_TABLE_ENTRY_SIZE           256
#define PROXYRCB_TABLE_ENTRY_SIZE_SHFT      8
#define PROXYRCB_TABLE_ENTRY_MULTIPLE       (PROXYRCB_TABLE_ENTRY_SIZE_SHFT -   \
                                             CAPRI_CB_TABLE_ENTRY_SINGLE_SHFT + 1)
#define PROXYRCB_TABLE_FLOW_KEY_OFFSET      (1 * CAPRI_CB_TABLE_ENTRY_SINGLE_SIZE)
#define PROXYRCB_TABLE_STATS_OFFSET         (2 * CAPRI_CB_TABLE_ENTRY_SINGLE_SIZE)

/*
 * For use by NCC ASM code
 */
#define PROXYRCB_NORMAL_STAT_INC_LAUNCH(table, qstate_addr_dst,                 \
                                        qstate_addr_src, phv_inc_stat)          \
    APP_REDIR_STAT_INC_LAUNCH(table, qstate_addr_dst, qstate_addr_src,          \
                              PROXYRCB_TABLE_STATS_OFFSET,                      \
                              phv_inc_stat, proxyr_normal_stats_inc)
#define PROXYRCB_ERR_STAT_INC_LAUNCH(table, qstate_addr_dst,                    \
                                   qstate_addr_src, phv_inc_stat)               \
    APP_REDIR_STAT_INC_LAUNCH(table, qstate_addr_dst, qstate_addr_src,          \
                              PROXYRCB_TABLE_STATS_OFFSET,                      \
                              phv_inc_stat, proxyr_err_stats_inc)
/*
 * Proxy Chain Control Block sizes
 */
#define PROXYCCB_TABLE_ENTRY_SIZE           128
#define PROXYCCB_TABLE_ENTRY_SIZE_SHFT      7
#define PROXYCCB_TABLE_ENTRY_MULTIPLE       (PROXYCCB_TABLE_ENTRY_SIZE_SHFT -   \
                                             CAPRI_CB_TABLE_ENTRY_SINGLE_SHFT + 1)
#define PROXYCCB_TABLE_STATS_OFFSET         (1 * CAPRI_CB_TABLE_ENTRY_SINGLE_SIZE)

/*
 * For use by NCC ASM code
 */
#define PROXYCCB_NORMAL_STAT_INC_LAUNCH(table, qstate_addr_dst,                 \
                                        qstate_addr_src, phv_inc_stat)          \
    APP_REDIR_STAT_INC_LAUNCH(table, qstate_addr_dst, qstate_addr_src,          \
                              PROXYCCB_TABLE_STATS_OFFSET,                      \
                              phv_inc_stat, proxyc_normal_stats_inc)
#define PROXYCCB_ERR_STAT_INC_LAUNCH(table, qstate_addr_dst,                    \
                                   qstate_addr_src, phv_inc_stat)               \
    APP_REDIR_STAT_INC_LAUNCH(table, qstate_addr_dst, qstate_addr_src,          \
                              PROXYCCB_TABLE_STATS_OFFSET,                      \
                              phv_inc_stat, proxyc_err_stats_inc)

/*
 * Max number of CBs, i.e., number of queues, supported for each type.
 * When changing these values, ensure modifications are also made to 
 * nic/conf/hbm_mem.json for the corresponding HBM regions ("app-redir-rawc",
 * "app-redir-proxyr", etc.) as well as the "lif2qstate_map" region.
 */
#define RAWRCB_NUM_ENTRIES_MAX              4096
#define RAWCCB_NUM_ENTRIES_MAX              4096
#define RAWRCB_NUM_ENTRIES_MASK             (RAWRCB_NUM_ENTRIES_MAX - 1)
#define RAWCCB_NUM_ENTRIES_MASK             (RAWCCB_NUM_ENTRIES_MAX - 1)

/*
 * Number of proxy CBs must be >= number of supported TCP/TLS proxy control blocks
 */
#define PROXYRCB_NUM_ENTRIES_MAX            4096
#define PROXYCCB_NUM_ENTRIES_MAX            4096
#define PROXYRCB_NUM_ENTRIES_MASK           (PROXYRCB_NUM_ENTRIES_MAX - 1)
#define PROXYCCB_NUM_ENTRIES_MASK           (PROXYCCB_NUM_ENTRIES_MAX - 1)
 
/*
 * Control Block activate/deactivate 8-bit indicators
 */
#define RAWRCB_DEACTIVATE                   0xaa
#define RAWRCB_ACTIVATE                     0xbb
#define RAWCCB_DEACTIVATE                   0xcc
#define RAWCCB_ACTIVATE                     0xdd

#define PROXYRCB_DEACTIVATE                 0xee
#define PROXYRCB_ACTIVATE                   0xff
#define PROXYCCB_DEACTIVATE                 0x44
#define PROXYCCB_ACTIVATE                   0x88


/*
 * Shared CB 16-bit flags
 */
#define APP_REDIR_DESC_VALID_BIT_UPD        0x0001
#define APP_REDIR_DESC_VALID_BIT_REQ        0x0002
#define APP_REDIR_CHAIN_DOORBELL_NO_SCHED   0x0004
#define APP_REDIR_DOL_PIPELINE_LOOPBK_EN    0x0008
#define APP_REDIR_DOL_SIM_DESC_ALLOC_FULL   0x0010
#define APP_REDIR_DOL_SIM_PAGE_ALLOC_FULL   0x0020
#define APP_REDIR_DOL_SIM_CHAIN_RXQ_FULL    0x0040
#define APP_REDIR_DOL_SIM_CHAIN_TXQ_FULL    0x0080
#define APP_REDIR_CHAIN_DESC_ADD_AOL_OFFSET 0x0100
#define APP_REDIR_DOL_SKIP_CHAIN_DOORBELL   0x0200

 
#ifndef NIC_DESC_ENTRY_0_OFFSET
#define NIC_DESC_ENTRY_0_OFFSET             64  /* &((nic_desc_t *)0)->entry[0]*/
#endif

/*
 * Default my TxQ process ID and ring ID
 */
#define RAWC_MY_TXQ_PID_DEFAULT             0
#define RAWC_MY_TXQ_RING_DEFAULT            0
#define PROXYR_MY_TXQ_RING_DEFAULT          0
#define PROXYC_MY_TXQ_RING_DEFAULT          0


/*
 * p4_to_p4plus app header immediately after rx_splitter_offset
 */
#define CPU_TO_P4PLUS_HEADER_SIZE           9

/*
 * Descriptor valid bit
 */ 
#ifndef DESC_VALID_BIT_SHIFT
#define DESC_VALID_BIT_SHIFT                63
#endif

/*
 * Descriptor-Page Ring info
 */
#define RAWR_RNMDPR_ALLOC_IDX               CPU_RX_DPR_ALLOC_IDX
#define RAWR_RNMDPR_ALLOC_CI_RAW_ADDR       CPU_RX_DPR_ALLOC_CI_RAW_ADDR
#define RAWR_RNMDPR_TABLE_BASE              CPU_RX_DPR_TABLE_BASE
#define RAWR_RNMDPR_TABLE_ENTRY_SIZE_SHIFT  CPU_RX_DPR_TABLE_ENTRY_SIZE_SHFT
#define RAWR_RNMDPR_RING_SHIFT              CAPRI_CPU_RX_DPR_RING_SHIFT
#define RAWR_RNMDPR_PAGE_OFFSET             CAPRI_CPU_RX_DPR_PAGE_OFFSET
#define RAWR_RNMDPR_PAGE_SIZE               CAPRI_CPU_RX_DPR_OBJ_TOTAL_SIZE
#define RAWR_RNMDPR_OVERHEADS_SIZE          RAWR_RNMDPR_PAGE_OFFSET
#define RAWR_RNMDPR_USABLE_PAGE_SIZE        (RAWR_RNMDPR_PAGE_SIZE - RAWR_RNMDPR_PAGE_OFFSET)
#endif //__APP_REDIR_SHARED_H

