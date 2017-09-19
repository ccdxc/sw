#ifndef _NVME_HPP_
#define _NVME_HPP_

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

// NVMe SGL struct
struct NvmeSgl{
  uint64_t  addr;
  uint32_t  size;
  uint8_t   rsvd[3];
  uint8_t   sub_type:4,
            type:4;
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
#ifdef LITTLE_ENDIAN
    uint32_t opc:8;    // Opcode
    uint32_t fuse:2;   // Fusing 2 simple commands
    uint32_t rsvd:4; 
    uint32_t psdt:2;   // PRP or SGL
    uint32_t cid:16;   // Command identifier
#else
    uint32_t cid:16;   // Command identifier
    uint32_t psdt:2;   // PRP or SGL
    uint32_t rsvd:4; 
    uint32_t fuse:2;   // Fusing 2 simple commands
    uint32_t opc:8;    // Opcode
#endif
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
#ifdef LITTLE_ENDIAN
      uint16_t qid;
      uint16_t qsize;
      uint16_t rsvd;
      uint16_t ivec;
#else
      uint16_t qsize;
      uint16_t qid;
      uint16_t ivec;
      uint16_t rsvd;
#endif
    } __attribute__((packed)) dw10_11;
  };

  // Dword 12
  struct {
#ifdef LITTLE_ENDIAN
    uint32_t nlb:16;    // Number of logical blocks
    uint32_t rsvd:10;  
    uint32_t prinfo:4;  // Protection information field
    uint32_t fua:1;     // Force unit access
    uint32_t lr:1;      // Limited retry
#else
    uint32_t lr:1;      // Limited retry
    uint32_t fua:1;     // Force unit access
    uint32_t prinfo:4;  // Protection information field
    uint32_t rsvd:10;  
    uint32_t nlb:16;    // Number of logical blocks
#endif
  } __attribute__((packed)) dw12;

  // Dword 13
  struct {
#ifdef LITTLE_ENDIAN
    uint32_t dsm:8;     // Dataset management
    uint32_t rsvd:24;
#else
    uint32_t rsvd:24;
    uint32_t dsm:8;     // Dataset management
#endif
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
#ifdef LITTLE_ENDIAN
    uint32_t sq_head:16;   // Submission queue head pointer
    uint32_t sq_id:16;     // Submission queue identifier
#else
    uint32_t sq_id:16;     // Submission queue identifier
    uint32_t sq_head:16;   // Submission queue head pointer
#endif
  } __attribute__((packed)) dw2;

  // Dword 3
  struct {
#ifdef LITTLE_ENDIAN
    uint32_t cid:16;       // Command identifier
    uint32_t phase:1;      // Phase bit
    uint32_t status:15;    // Status
#else
    uint32_t status:15;    // Status
    uint32_t phase:1;      // Phase bit
    uint32_t cid:16;       // Command identifier
#endif
  } __attribute__((packed)) dw3;
} __attribute__((packed));


// Pvm status
struct PvmStatus {
    struct NvmeStatus nvme_status;
    uint8_t pvm_trailer[48]; // 64 byte aligned
} __attribute__((packed));

#endif  // _NVME_HPP_
