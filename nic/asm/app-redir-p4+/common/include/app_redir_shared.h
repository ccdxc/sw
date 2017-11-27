#ifndef __APP_REDIR_SHARED_H
#define __APP_REDIR_SHARED_H

/*
 * This file contains app redirect definitions that are shared with P4+ asm code.
 */

#define RAWR_CB_TABLE_ENTRY_SIZE            64 /* in bytes */
#define RAWR_CB_TABLE_ENTRY_SIZE_SHFT       6
#define RAWC_CB_TABLE_ENTRY_SIZE            64 /* in bytes */
#define RAWC_CB_TABLE_ENTRY_SIZE_SHFT       6

#define PROXYR_CB_TABLE_ENTRY_SIZE          128 /* in bytes */
#define PROXYR_CB_TABLE_ENTRY_SIZE_SHFT     7
#define PROXYR_CB_TABLE_FLOW_KEY_OFFSET     64

#define PROXYC_CB_TABLE_ENTRY_SIZE          64 /* in bytes */
#define PROXYC_CB_TABLE_ENTRY_SIZE_SHFT     6

/*
 * Shared CB flags
 */
#define APP_REDIR_DESC_VALID_BIT_UPD        0x0001
#define APP_REDIR_DESC_VALID_BIT_REQ        0x0002
#define APP_REDIR_CHAIN_DOORBELL_NO_SCHED   0x0004
#define APP_REDIR_DOL_PIPELINE_LOOPBK_EN    0x0008


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

