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
 * Control Block sizes
 */
#define RAWRCB_TABLE_ENTRY_SIZE             64
#define RAWRCB_TABLE_ENTRY_SIZE_SHFT        6
#define RAWRCB_TABLE_ENTRY_MULTIPLE         (RAWRCB_TABLE_ENTRY_SIZE /  \
                                             CAPRI_CB_TABLE_ENTRY_SINGLE_SIZE)

#define RAWCCB_TABLE_ENTRY_SIZE             64
#define RAWCCB_TABLE_ENTRY_SIZE_SHFT        6
#define RAWCCB_TABLE_ENTRY_MULTIPLE         (RAWCCB_TABLE_ENTRY_SIZE /  \
                                             CAPRI_CB_TABLE_ENTRY_SINGLE_SIZE)

#define PROXYRCB_TABLE_ENTRY_SIZE           128
#define PROXYRCB_TABLE_ENTRY_SIZE_SHFT      7
#define PROXYRCB_TABLE_ENTRY_MULTIPLE       (PROXYRCB_TABLE_ENTRY_SIZE /  \
                                             CAPRI_CB_TABLE_ENTRY_SINGLE_SIZE)
#define PROXYRCB_TABLE_FLOW_KEY_OFFSET      (1 * CAPRI_CB_TABLE_ENTRY_SINGLE_SIZE)

#define PROXYCCB_TABLE_ENTRY_SIZE           64
#define PROXYCCB_TABLE_ENTRY_SIZE_SHFT      6
#define PROXYCCB_TABLE_ENTRY_MULTIPLE       (PROXYCCB_TABLE_ENTRY_SIZE /  \
                                             CAPRI_CB_TABLE_ENTRY_SINGLE_SIZE)

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
 * A proxy CB ID is always an hflow TCP connection ID and is the same
 * for both initiator and responder directions. Each direction requires
 * a separate CB which is programmed with its own state info. Hence, the
 * number of CBs needed is effectively doubled for proxy flows.
 */
#define PROXYRCB_NUM_ENTRIES_MAX_MULT       2
#define PROXYCCB_NUM_ENTRIES_MAX_MULT       2

#define PROXYR_OPER_CB_OFFSET(dir)          \
    (((dir) & 1) * PROXYRCB_NUM_ENTRIES_MAX)

#define PROXYR_OPER_CB_ID(dir, cb_id)       \
    (PROXYR_OPER_CB_OFFSET(dir) + (cb_id))
 
#define PROXYC_OPER_CB_OFFSET(dir)          \
    (((dir) & 1) * PROXYCCB_NUM_ENTRIES_MAX)

#define PROXYC_OPER_CB_ID(dir, cb_id)       \
    (PROXYC_OPER_CB_OFFSET(dir) + (cb_id))

/*
 * Proxy direction should be runtime evaluated but for simplification,
 * TCP and TLS are assigned the below values. These constants work as long as
 * TCP/TLS proxy flows are network-to-host or host-to-network
 * (i.e., not host-to-host or network-to-network).
 */
#define PROXYR_TCP_PROXY_DIR                0
#define PROXYR_TLS_PROXY_DIR                1
#define PROXYC_TCP_PROXY_DIR                PROXYR_TCP_PROXY_DIR
#define PROXYC_TLS_PROXY_DIR                PROXYR_TLS_PROXY_DIR


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

#endif //__APP_REDIR_SHARED_H

