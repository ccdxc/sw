/*****************************************************************************
 * storage_common_defines.h: Common storage defintions across P4+ and assembly
 *****************************************************************************/
 
#ifndef STORAGE_COMMON_DEFINES_H
#define STORAGE_COMMON_DEFINES_H

// Default table load size (used in cases where table entry size > 64 bytes)
// The sizes are expressed as a power of 2, hence using 6 here
#define STORAGE_DEFAULT_TBL_LOAD_SIZE		6
#define STORAGE_DEFAULT_TBL_LOAD_SIZE_BYTES	64  // 2 ^ STORAGE_DEFAULT_TBL_LOAD_SIZE

// Offset in bytes of the start of actual NVME command in the NVME backend command
#define NVME_BE_NVME_CMD_OFFSET		64

// Maximum number of commands outstanding to an SSD
#define SSD_MAX_CMDS			64

// Keep the header and entry size of the SSD's list of saved commands to be 
// aligned at 64 bytes for easy table read access
// Header: 64-bit bitmap 
// Entry:  Full R2N WQE (r2n_wqe_t) is stored here
#define SSD_CMDS_HEADER_SIZE		64
#define SSD_CMDS_ENTRY_SIZE		64
#define SSD_CMDS_ENTRY_SHIFT	6

// Offset in memory to store the command handle in the SSD's list of saved commands
#define SSD_CMDS_ENTRY_OFFSET(bm_pos)	(SSD_CMDS_HEADER_SIZE + (bm_pos * SSD_CMDS_ENTRY_SIZE))

// Capri doorbell address/data formation values
#define DOORBELL_ADDR_QTYPE_SHIFT       3
#define DOORBELL_ADDR_LIF_SHIFT         6
#define DOORBELL_ADDR_SCHED_WR_SHIFT    17
#define DOORBELL_ADDR_UPD_SHIFT         19
#define DOORBELL_ADDR_WA_LOCAL_BASE     0x8800000

#define DOORBELL_DATA_RING_SHIFT	16
#define DOORBELL_DATA_QID_SHIFT		24
#define DOORBELL_DATA_PID_SHIFT		48

#define DOORBELL_SCHED_WR_NONE		0
#define DOORBELL_SCHED_WR_EVAL		(1 << DOORBELL_ADDR_SCHED_WR_SHIFT)
#define DOORBELL_SCHED_WR_RESET		(2 << DOORBELL_ADDR_SCHED_WR_SHIFT)
#define DOORBELL_SCHED_WR_SET		(3 << DOORBELL_ADDR_SCHED_WR_SHIFT)
#define DOORBELL_UPDATE_NONE		0
#define DOORBELL_UPDATE_C_NDX		(1 << DOORBELL_ADDR_UPD_SHIFT)
#define DOORBELL_UPDATE_P_NDX		(2 << DOORBELL_ADDR_UPD_SHIFT)
#define DOORBELL_UPDATE_P_NDX_INCR	(3 << DOORBELL_ADDR_UPD_SHIFT)

// Each queue state can be 64 bytes at most (from d-vector in Capri)
// The sizes are expressed as a power of 2, hence using 6 here
#define Q_STATE_SIZE     	6      

// Offsets (in bytes) of fields within a structure
#define Q_STATE_C_NDX_OFFSET	10

// PRP and LB size calculation assists
#define PRP_SIZE_MASK		0xFFF
#define PRP_SIZE_SUB		0x1000
#define LB_SIZE_SHIFT		12

// Max h/w assist size for PRP list download
#define MAX_ASSIST_SIZE		65536

// Max number of additional PRPs in assist
#define NVME_MAX_XTRA_PRPS	16

// Size of each PRP data xfer
#define PRP_DATA_XFER_SIZE		4096

// Max size of PRP assist = # entries (15) * prp entry size (8 bytes)
#define PRP_LIST_ENTRY_SIZE		8
#define PRP_ASSIST_MAX_ENTRIES		15
#define PRP_ASSIST_MAX_LIST_SIZE	(PRP_ASSIST_MAX_ENTRIES * PRP_LIST_ENTRY_SIZE)


// NVME write and read command opcodes (from NVME spec)
#define NVME_WRITE_CMD_OPCODE			1
#define NVME_READ_CMD_OPCODE			2

#define NVME_BE_PRIORITY_HI			0
#define NVME_BE_PRIORITY_MED			1
#define NVME_BE_PRIORITY_LO			2
#define NVME_BE_MAX_PRIORITIES			3

#define ROCE_OP_TYPE_SEND			0
#define ROCE_OP_TYPE_SEND_INV			1
#define ROCE_OP_TYPE_SEND_IMM			2
#define ROCE_OP_TYPE_READ			3
#define ROCE_OP_TYPE_WRITE			4
#define ROCE_OP_TYPE_WRITE_IMM			5
#define ROCE_OP_TYPE_SEND_INV_IMM		11
#define ROCE_OP_TYPE_SEND_RCVD			17

#define ROCE_CQ_STATUS_SUCCESS			0

// ROCE WQE sizes in bytes
#define ROCE_RQ_WQE_SIZE			64
#define ROCE_SQ_WQE_SIZE			64

#define R2N_OPCODE_PROCESS_WQE			1
#define R2N_OPCODE_BUF_POST			2

/***********************************************************************************
 * R2N buffer data structure layout
 ***********************************************************************************/

// Offsets into the R2N buffer in bytes
#define R2N_BUF_BUF_POST_OFFSET			0
#define R2N_BUF_PRP_LIST_OFFSET			64
#define R2N_BUF_STATUS_BUF_OFFSET		192
#define R2N_BUF_STATUS_REQ_OFFSET		256
#define R2N_BUF_PAD_OFFSET			320 // Pad size = 4096 - (128 + 64 + 8) - 320 = 3576
#define R2N_BUF_NVME_BE_CMD_OFFSET		3896
#define R2N_BUF_WRITE_REQ_OFFSET		4024
#define R2N_BUF_IO_CTX_ID_OFFSET		4088
#define R2N_BUF_DATA_OFFSET			4096

// Offsets and sizes of various sub-buffers in the R2N I/O buffer for PDMA
#define NVME_CMD_PRP1_OFFSET			24
#define R2N_BUF_NVME_BE_CMD_HDR_SIZE		64
#define R2N_BUF_NVME_CMD_PRP1_OFFSET		(R2N_BUF_NVME_BE_CMD_OFFSET + R2N_BUF_NVME_BE_CMD_HDR_SIZE + NVME_CMD_PRP1_OFFSET)
#define R2N_BUF_WRITE_REQ_WRID_OFFSET		R2N_BUF_WRITE_REQ_OFFSET
#define R2N_BUF_WRITE_REQ_SGE0_OFFSET		(R2N_BUF_WRITE_REQ_OFFSET + 32)

/***********************************************************************************
 * I/O MAP data structure layout
 ***********************************************************************************/

// I/O Map layout
// Allocating 1K bytes for I/O map to keep size power of 2
#define IO_MAP_BASE_OFFSET			0
#define IO_MAP_BARCO_XTS_ENC_DESC_OFFSET	64
#define IO_MAP_BARCO_XTS_DEC_DESC_OFFSET	192
#define IO_MAP_COMP_DESC_OFFSET			320
#define IO_MAP_DECOMP_DESC_OFFSET		384
#define IO_MAP_INT_TAG_DESC_OFFSET		448
#define IO_MAP_DEDUP_TAG_DESC_OFFSET		512

#define IO_MAP_ENTRY_SIZE_LOG2			10   // If this changes, update IO_BUF_***_OFFSET
#define IO_MAP_ENTRY_SIZE_BYTES			1024 // 2 ^ IO_MAP_ENTRY_SIZE_LOG2

// I/O Map sizes of various descriptors
#define IO_MAP_BASE_SIZE			64
#define IO_MAP_BARCO_XTS_ENC_DESC_SIZE		128
#define IO_MAP_BARCO_XTS_DEC_DESC_SIZE		128
#define IO_MAP_COMP_DESC_SIZE			64
#define IO_MAP_DECOMP_DESC_SIZE			64
#define IO_MAP_INT_TAG_DESC_SIZE		64
#define IO_MAP_DEDUP_TAG_DESC_SIZE		64
#define IO_MAP_R2N_WQE_SIZE			64
#define IO_MAP_ROCE_SQ_WQE_SIZE			64

// I/O Map dst flags
#define	IO_XTS_ENCRYPT				0x00000001
#define	IO_XTS_DECRYPT				0x00000002
#define	IO_DST_REMOTE				0x00000004

/***********************************************************************************
 * I/O buffer data structure layout
 ***********************************************************************************/

// I/O buffer sequencer descriptors
#define IO_BUF_SEQ_BARCO_XTS_ENC_DESC_OFFSET	0
#define IO_BUF_SEQ_BARCO_XTS_DEC_DESC_OFFSET	64
#define IO_BUF_SEQ_COMP_DESC_OFFSET		128
#define IO_BUF_SEQ_DECOMP_DESC_OFFSET		192
#define IO_BUF_SEQ_INT_TAG_DESC_OFFSET		256
#define IO_BUF_SEQ_DEDUP_TAG_DESC_OFFSET	320
#define IO_BUF_SEQ_R2N_WQE_OFFSET		384
#define IO_BUF_SEQ_PDMA_DESC_OFFSET		448
// Size == the last offset + size of last entry (64 bytes)
#define IO_BUF_SEQ_DESC_TOTAL_SIZE		512 // If this changes, update IO_BUF_***_OFFSET
// Size of each sequencer descriptor
#define IO_BUF_SEQ_DESC_SIZE			64 

// I/O buffer sequencer descriptors
#define IO_BUF_SEQ_BARCO_XTS_ENC_DB_OFFSET	0
#define IO_BUF_SEQ_BARCO_XTS_DEC_DB_OFFSET	64
#define IO_BUF_SEQ_COMP_DB_OFFSET		128
#define IO_BUF_SEQ_DECOMP_DB_OFFSET		192
#define IO_BUF_SEQ_INT_TAG_DB_OFFSET		256
#define IO_BUF_SEQ_DEDUP_TAG_DB_OFFSET		320
#define IO_BUF_SEQ_R2N_DB_OFFSET		384
#define IO_BUF_SEQ_PDMA_DB_OFFSET		448
// Size == the last offset + size of last entry (64 bytes)
#define IO_BUF_SEQ_DB_TOTAL_SIZE		512 // If this changes, update IO_BUF_***_OFFSET
// Size of each sequencer descriptor
#define IO_BUF_SEQ_DB_SIZE			64 

// Offsets into the I/O buffer in bytes
#define IO_BUF_IO_CTX_OFFSET			0
#define IO_BUF_R2N_WQE_OFFSET			64
#define IO_BUF_ROCE_SQ_WQE_OFFSET		128
#define IO_BUF_PRP_LIST_OFFSET			192
#define IO_BUF_IO_MAP_DESC_OFFSET		320   // Size == IO_MAP_ENTRY_TOTAL_SIZE == 1024 bytes
#define IO_BUF_SEQ_DESC_OFFSET			1344  // Size == IO_BUF_SEQ_DESC_TOTAL_SIZE Size = 512 bytes
#define IO_BUF_SEQ_DB_OFFSET			1856  // Size == IO_BUF_SEQ_DB_TOTAL_SIZE Size = 512 bytes
#define IO_BUF_PAD_OFFSET			2368  // Pad size = 4096 - (128 + 64 + 8) - 1856 = 1528
// Keep the following offset the same as the NVME_BE_CMD_OFFSET in R2N Buffer using pad above
#define IO_BUF_NVME_BE_CMD_OFFSET		3896
#define IO_BUF_WRITE_REQ_OFFSET			4024
#define IO_BUF_IO_CTX_ID_OFFSET			4088
// Start of the data buffer in the R2N I/O buffer
#define IO_BUF_DATA_OFFSET			4096
// Size of base length of R2N buffer to be provided to the transport
#define IO_BUF_BASE_XPORT_DATA_LEN		(IO_BUF_DATA_OFFSET - IO_BUF_NVME_BE_CMD_OFFSET)

// Offsets into ROCE SQ WQE, R2N WQE to adjust length
#define IO_BUF_ROCE_SQ_WQE_LEN_OFFSET		(IO_BUF_ROCE_SQ_WQE_OFFSET + 40)
#define IO_BUF_R2N_WQE_LEN_OFFSET		(IO_BUF_R2N_WQE_OFFSET + 8)

#define IO_BUF_PRP_LIST_ENTRY_OFFSET(entry)	(IO_BUF_PRP_LIST_OFFSET + (entry * PRP_LIST_ENTRY_SIZE))
// I/O context definitions
#define	IO_CTX_OPER_STATUS_FREE			0
#define	IO_CTX_OPER_STATUS_IN_USE		1
#define	IO_CTX_OPER_STATUS_PUNT			2

#endif     // STORAGE_COMMON_DEFINES_H
