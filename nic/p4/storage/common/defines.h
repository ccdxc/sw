/*****************************************************************************
 * defines.h: Common defintions across P4+ and assembly
 *****************************************************************************/
 
#ifndef DEFINES_H
#define DEFINES_H

// Capri table defintions
#define CAPRI_TABLE_TYPE_NONE	0
#define CAPRI_TABLE_TYPE_RAW	1


// Each Queue Context can be 64 bytes at most (from d-vector in Capri)
#define Q_CTX_SIZE     64      

// Queue Context states
#define Q_CTX_FREE     0      
#define Q_CTX_BUSY     1      

// Table offsets
#define NVME_VF_SQ_CTX_TABLE_BASE		0x40000000
#define NVME_VF_CQ_CTX_TABLE_BASE		0x40004000
#define PVM_VF_SQ_CTX_TABLE_BASE		0x40008000
#define PVM_VF_CQ_CTX_TABLE_BASE		0x4000C000
#define PVM_ERRQ_CTX_TABLE_BASE			0x40010000
#define R2N_CQ_CTX_TABLE_BASE			0x20000000
#define R2N_RQ_CTX_TABLE_BASE			0x20004000
#define R2N_HQ_CTX_TABLE_BASE			0x20008000
#define R2N_SQ_CTX_TABLE_BASE			0x2000C000
#define NVME_BE_SQ_CTX_TABLE_BASE		0x20010000
#define NVME_BE_CQ_CTX_TABLE_BASE		0x20014000
#define SSD_SQ_CTX_TABLE_BASE			0x20018000
#define SSD_CQ_CTX_TABLE_BASE			0x2001C000
#define SSD_INFO_TABLE_BASE			0x20020000
#define SSD_CMDS_TABLE_BASE			0x20024000
#define SEQ_SQ_CTX_TABLE_BASE			0x20030000

// Queue entry sizes (in bytes)
#define NVME_VF_SQ_ENTRY_SIZE	       		64
#define NVME_VF_CQ_ENTRY_SIZE 			16
#define PVM_VF_SQ_ENTRY_SIZE	       		88 // Not a loaded table
#define PVM_VF_CQ_ENTRY_SIZE 			16
#define R2N_CQ_ENTRY_SIZE 			24
#define R2N_RX_BUF_POST_ENTRY_SIZE 		24
#define R2N_SQ_ENTRY_SIZE 	       		40
#define NVME_BE_CQ_ENTRY_SIZE 			32
#define SSD_SQ_ENTRY_SIZE			64
#define SSD_CQ_ENTRY_SIZE			16
#define SSD_INFO_ENTRY_SIZE			8
#define PVM_ERRQ_ENTRY_SIZE 			R2N_CQ_ENTRY_SIZE
#define SEQ_DESC_ENTRY_SIZE			64
#define SEQ_SQ_ENTRY_SIZE			64 

// PVM Error queue default idx
#define PVM_ERRQ_DEFAULT_IDX			0

// Offset and size (in bytes) of NVME command in ROCE CQ entry
#define R2N_NVME_HDR_SIZE			16  
#define R2N_NVME_CMD_OFFSET			R2N_NVME_HDR_SIZE  
#define R2N_NVME_CMD_SIZE			64  
#define NVME_BE_SQ_ENTRY_SIZE	       		(R2N_NVME_HDR_SIZE + R2N_NVME_CMD_SIZE)

// Queue entry size derivatives
#define SSD_MAX_CMDS				64
#define SSD_CMDS_HEADER_SIZE                   (SSD_MAX_CMDS/8)
#define SSD_CMDS_ENTRY_SIZE                    (SSD_CMDS_HEADER_SIZE + (SSD_MAX_CMDS * NVME_BE_SQ_ENTRY_SIZE))


// Offsets in bits. TODO: Check endianness
#define NVME_CMD_CID_OFFSET			16
#define NVME_STA_CID_OFFSET_START		112
#define NVME_STA_CID_OFFSET_END			96

// ROCE Receive Buffer offsets (in bytes - ref: RoceBufHdr in C++ code)
#define R2N_RX_BUF_ENTRY_BUF_POST_OFFSET	0
#define R2N_RX_BUF_ENTRY_BE_STA_OFFSET		24
#define R2N_RX_BUF_ENTRY_STATUS_REQ_OFFSET	56
#define R2N_RX_BUF_ENTRY_BE_CMD_OFFSET		96
#define R2N_RX_BUF_ENTRY_WRITE_REQ_OFFSET	176
#define R2N_RX_BUF_ENTRY_DATA_OFFSET		216
#define R2N_RX_BUF_ENTRY_SIZE			R2N_RX_BUF_ENTRY_DATA_OFFSET

// Additional offsets to copy to fields
#define R2N_RX_BUF_BE_STA_STATUS_OFFSET	28
#define R2N_RX_BUF_BE_STA_CMD_HANDLE_OFFSET	32
#define R2N_RX_BUF_BE_CMD_SRC_Q_ID_OFFSET	96
#define R2N_RX_BUF_BE_CMD_CMD_HANDLE_OFFSET	104
#define R2N_RX_BUF_BE_CMD_NVME_CMD_OFFSET	112
#define R2N_RX_BUF_BE_CMD_SGL_ADDR_OFFSET	124
#define R2N_RX_BUF_WRITE_REQ_LADDR_OFFSET	208

// ROCE Receive Buffer sizes
#define R2N_RX_BUF_ENTRY_BUF_POST_SIZE		\
	(R2N_RX_BUF_ENTRY_BE_STA_OFFSET - R2N_RX_BUF_ENTRY_BUF_POST_OFFSET)

#define R2N_RX_BUF_ENTRY_BE_STA_SIZE		\
	(R2N_RX_BUF_ENTRY_STATUS_REQ_OFFSET - R2N_RX_BUF_ENTRY_BE_STA_OFFSET)

#define R2N_RX_BUF_ENTRY_STATUS_REQ_SIZE	\
	(R2N_RX_BUF_ENTRY_BE_CMD_OFFSET - R2N_RX_BUF_ENTRY_STATUS_REQ_OFFSET)

#define R2N_RX_BUF_ENTRY_BE_CMD_SIZE		\
	(R2N_RX_BUF_ENTRY_WRITE_REQ_OFFSET - R2N_RX_BUF_ENTRY_BE_CMD_OFFSET)

#define R2N_RX_BUF_ENTRY_BE_CMD_HDR_SIZE	\
	(R2N_RX_BUF_BE_CMD_NVME_CMD_OFFSET - R2N_RX_BUF_ENTRY_BE_CMD_OFFSET)

#define R2N_RX_BUF_ENTRY_WRITE_REQ_SIZE	\
	(R2N_RX_BUF_ENTRY_DATA_OFFSET - R2N_RX_BUF_ENTRY_WRITE_REQ_OFFSET)


// ROCE completion type
#define R2N_COMP_TYPE_SEND_REQ			1
#define R2N_COMP_TYPE_NEW_BUFFER		2
#define R2N_COMP_TYPE_RDMA_WRITE		3
#define R2N_COMP_TYPE_RDMA_READ			4

// NVME Backend status
#define NVME_BE_STATUS_GOOD			0

// Number of priorities per NVME Backend
#define NVME_BE_PRIORITY_LO			0
#define NVME_BE_PRIORITY_MED			1
#define NVME_BE_PRIORITY_HI			2
#define NVME_BE_MAX_PRIORITIES			3

// Offsets (in bytes) needed by ASM to do memwr
#define PRI_Q_CTX_STATE_OFFSET			2
#define PRI_Q_CTX_C_NDX_LO_OFFSET		4
#define PRI_Q_CTX_C_NDX_MED_OFFSET		6
#define PRI_Q_CTX_C_NDX_HI_OFFSET		8
#define Q_CTX_STATE_OFFSET			2
#define Q_CTX_C_NDX_OFFSET			4

// PRP and LB size calculation assists
#define PRP_SIZE_MASK				0xFFF
#define PRP_SIZE_SUB				0x1000
#define LB_SIZE_SHIFT				12

// Max h/w assist size for PRP list download
#define MAX_ASSIST_SIZE				65536

// Max number of additional PRPs in assist
#define NVME_MAX_XTRA_PRPS			16

// NVME write and read command opcodes (from NVME spec)
#define NVME_WRITE_CMD_OPCODE			1
#define NVME_READ_CMD_OPCODE			2

// Byte offset of the command id that is to be rewritten when saving the NVME
// command send to SSD
#define SSD_CMD_CID_OFFSET			66


#endif     // DEFINES_H
