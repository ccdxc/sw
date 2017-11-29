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
#define RAWRCB_NUM_ENTRIES_MAX              1024
#define RAWCCB_NUM_ENTRIES_MAX              1024
#define PROXYRCB_NUM_ENTRIES_MAX            1024
#define PROXYCCB_NUM_ENTRIES_MAX            1024
 
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
#define CPU_TO_P4PLUS_HEADER_SIZE           8

/*
 * Descriptor valid bit
 */ 
#ifndef DESC_VALID_BIT_SHIFT
#define DESC_VALID_BIT_SHIFT                63
#endif

#endif //__APP_REDIR_SHARED_H

