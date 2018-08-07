#ifndef _NVME_HPP_
#define _NVME_HPP_

#include <asm/byteorder.h>
#include <stdint.h>
#include <string.h>

// NVME write and read command opcodes (from NVME spec)
#define NVME_WRITE_CMD_OPCODE		1
#define NVME_READ_CMD_OPCODE		2

#define NVME_ADMIN_CMD_DELETE_SQ	0
#define NVME_ADMIN_CMD_CREATE_SQ	1
#define NVME_ADMIN_CMD_GET_LOG_PAGE	2
#define NVME_ADMIN_CMD_DELETE_CQ	4
#define NVME_ADMIN_CMD_CREATE_CQ	5
#define NVME_ADMIN_CMD_IDENTIFY		6
#define NVME_ADMIN_CMD_ABORT		8

// NVME Success in the completion status (from NVME spec)
#define NVME_STATUS_SUCCESS		0

// PSDT bits are stored 14:15 in flags of dw0
#define NVME_CMD_PSDT(cmd)		((cmd).dw0.flags >> 6)

// NVMe SGL struct
struct NvmeSgl{
  uint64_t  addr;
  uint32_t  size;
  uint32_t  rsvd_type;
} __attribute__((packed));

// NVMe PRP struct
struct NvmePrp{
  uint64_t  prp1;
  uint64_t  prp2;
} __attribute__((packed));

// NVME command 
struct NvmeCmd {
  // Dword 0
  struct {
    uint8_t opc;
    uint8_t flags;
    uint16_t cid;
  } __attribute__((packed)) dw0;
  
  // Dword 1
  uint32_t nsid;       // Namespace identifier

  // Dword 2
  uint32_t rsvd2;

  // Dword 3
  uint32_t rsvd3;

  // Dword 4 and 5 
  uint64_t mptr;       // Metadata pointer

  // Dwords 6,7,8 & 9 form the data pointer (PRP or SGL) 
  union {
    NvmePrp prp;
    NvmeSgl sgl;
  }; 

  // Dword 10 and 11 
  union {
    uint64_t slba;       // Starting LBA (for Read/Write) commands
    struct {
      uint16_t qid;
      uint16_t qsize;
      uint16_t rsvd;
      uint16_t ivec;
    } __attribute__((packed)) dw10_11;
  };

  // Dword 12
  struct {
    uint16_t nlb;    // Number of logical blocks
    uint16_t flags;
  } __attribute__((packed)) dw12;

  // Dword 13
  struct {
    uint8_t dsm;     // Dataset management
    uint8_t rsvd[3];
  } __attribute__((packed)) dw13;

  // Dword 14 - TODO: add to this
  uint32_t dw14;

  // Dword 15 - TODO: add to this
  uint32_t dw15;

} __attribute__((packed)); 

//Status as defined in spec
#define NVME_STATUS_INVALID_CMD_OPC 0x01
#define NVME_STATUS_INVALID_FIELD 0x02
#define NVME_STATUS_INTERNAL_ERROR 0x06
#define NVME_STATUS_IVALID_SGL_DESC 0x0D
#define NVME_STATUS_IVALID_NUM_SGL 0x0E
#define NVME_STATUS_INVALID_SGL_DATA_LENGTH 0x0F
#define NVME_STATUS_INVALID_SGL_DESC_TYPE 0x11
#define NVME_STATUS_INVALID_PRP_OFFSET 0x13
#define NVME_STATUS_LBA_OUT_OF_RANGE 0x80


// NVME status 
struct NvmeStatus {
  // Dword 0
  uint32_t cspec;          // Command specific

  // Dword 1
  uint32_t rsvd;

  // Dword 2
  struct {
    uint16_t sq_head;      // Submission queue head pointer
    uint16_t sq_id;        // Submission queue identifier
  } __attribute__((packed)) dw2;

  // Dword 3
  struct {
    uint16_t cid;          // Command identifier
    uint16_t status_phase; // Status
  } __attribute__((packed)) dw3;
} __attribute__((packed));

#define NVME_STATUS_GET_STATUS(sta)	((sta).dw3.status_phase >> 1)		

#define NVME_STATUS_SET_STATUS(sta, status)		\
  do {							\
    (sta).dw3.status_phase |= ((status) & 0x7FFF) << 1;	\
  } while(0);						\

#define NVME_STATUS_SET_PHASE(sta, phase)		\
  do {							\
    (sta).dw3.status_phase |= (phase) & 0x1;		\
  } while(0);						\

// Pvm status
struct PvmStatus {
    struct NvmeStatus nvme_status;
    uint8_t pvm_trailer[48]; // 64 byte aligned
} __attribute__((packed));

#endif  // _NVME_HPP_
