#ifndef __NVME_IF_HPP__
#define __NVME_IF_HPP__

#define NVME_DEV_PAGE_SIZE             4096
#define NVME_DEV_PAGE_MASK             (NVME_DEV_PAGE_SIZE - 1)

#define NVME_DEV_ADDR_ALIGN(addr, sz)  \
    (((addr) + ((uint64_t)(sz) - 1)) & ~((uint64_t)(sz) - 1))

#define NVME_DEV_PAGE_ALIGN(addr)      \
    NVME_DEV_ADDR_ALIGN(addr, NVME_DEV_PAGE_SIZE)

/* Supply these for nvme_dev_if.h */
#define dma_addr_t uint64_t

#include "nic/include/nvme_dev_if.h"

#pragma pack(push, 1)

/**
 * NVME PF Devcmd Region
 */

typedef union nvme_reg_cap_s {
    struct {
        uint64_t    mqes: 16;
        uint64_t    cqr: 1;
        uint64_t    ams: 2;
        uint64_t    rsvd2: 5;
        uint64_t    to: 8;
        uint64_t    dstrd: 4;
        uint64_t    nssrs: 1;
        uint64_t    css: 8;
        uint64_t    rsvd1: 3;
        uint64_t    mpsmin: 4;
        uint64_t    mpsmax: 4;
        uint64_t    rsvd0: 8;
    };
    uint64_t        num64;
} nvme_reg_cap_t;

typedef union nvme_reg_vs_s {
    struct {
        uint32_t    rsvd: 8;
        uint32_t    mnr: 8;
        uint32_t    mjr: 16;
    };
    uint32_t        num32;
} nvme_reg_vs_t;

typedef union nvme_reg_cc_s {
    struct {
        uint32_t    en: 1;
        uint32_t    rsvd1: 3;
        uint32_t    css: 3;
        uint32_t    mps: 4;
        uint32_t    ams: 3;
        uint32_t    shn: 2;
        uint32_t    iosqes: 4;
        uint32_t    iocqes: 4;
        uint32_t    rsvd0: 8;
    };
    uint32_t        num32;
} nvme_reg_cc_t;

typedef union nvme_reg_csts_s {
    struct {
        uint32_t    rdy: 1;
        uint32_t    cfs: 1;
        uint32_t    shst: 2;
        uint32_t    nssro: 1;
        uint32_t    pp: 1;
        uint32_t    rsvd0: 26;
    };
    uint32_t        num32;
} nvme_reg_csts_t;

typedef union nvme_reg_aqa_s {
    struct {
        uint32_t    asqs: 12;
        uint32_t    rsvd1: 4;
        uint32_t    acqs: 12;
        uint32_t    rsvd0: 4;
    };
    uint32_t        num32;
} nvme_reg_aqa_t;

typedef union nvme_reg_asq_s {
    struct {
        uint64_t    rsvd0: 12;
        uint64_t    asqb: 52;
    };
    uint64_t        num64;
} nvme_reg_asq_t;

typedef union nvme_reg_acq_s {
    struct {
        uint64_t    rsvd0: 12;
        uint64_t    acqb: 52;
    };
    uint64_t        num64;
} nvme_reg_acq_t;

enum nvme_sgl_descriptor_type {
    NVME_SGL_TYPE_DATA_BLOCK       = 0x0,
    NVME_SGL_TYPE_BIT_BUCKET       = 0x1,
    NVME_SGL_TYPE_SEGMENT      = 0x2,
    NVME_SGL_TYPE_LAST_SEGMENT     = 0x3,
    NVME_SGL_TYPE_KEYED_DATA_BLOCK = 0x4,
    NVME_SGL_TYPE_TRANSPORT_DATA_BLOCK = 0x5,
    /* 0x6 - 0xE reserved */
    NVME_SGL_TYPE_VENDOR_SPECIFIC  = 0xF
};

enum nvme_sgl_descriptor_subtype {
    NVME_SGL_SUBTYPE_ADDRESS       = 0x0,
    NVME_SGL_SUBTYPE_OFFSET        = 0x1,
    NVME_SGL_SUBTYPE_TRANSPORT     = 0xa,
};

typedef struct nvme_sgl_descriptor_s {
    uint64_t address;
    union {
        struct {
            uint8_t reserved[7];
            uint8_t subtype : 4;
            uint8_t type    : 4;
        } generic;

        struct {
            uint32_t length;
            uint8_t reserved[3];
            uint8_t subtype : 4;
            uint8_t type    : 4;
        } unkeyed;

        struct {
            uint64_t length     : 24;
            uint64_t key        : 32;
            uint64_t subtype    : 4;
            uint64_t type       : 4;
        } keyed;
    };
} nvme_sgl_descriptor_t;

typedef struct nvme_cmd_s {
    /* dword 0 */
    uint16_t opc    :  8;   /* opcode */
    uint16_t fuse   :  2;   /* fused operation */
    uint16_t rsvd1  :  4;
    uint16_t psdt   :  2;
    uint16_t cid;       /* command identifier */

    /* dword 1 */
    uint32_t nsid;      /* namespace identifier */

    /* dword 2-3 */
    uint32_t rsvd2;
    uint32_t rsvd3;

    /* dword 4-5 */
    uint64_t mptr;      /* metadata pointer */

    /* dword 6-9: data pointer */
    union {
        struct {
            uint64_t prp1;      /* prp entry 1 */
            uint64_t prp2;      /* prp entry 2 */
        } prp;

        nvme_sgl_descriptor_t sgl1;
    } dptr;

    /* dword 10-15 */
    uint32_t cdw10;     /* command-specific */
    uint32_t cdw11;     /* command-specific */
    uint32_t cdw12;     /* command-specific */
    uint32_t cdw13;     /* command-specific */
    uint32_t cdw14;     /* command-specific */
    uint32_t cdw15;     /* command-specific */
} nvme_cmd_t;

static_assert(sizeof(nvme_cmd_t) == 64);

typedef struct nvme_status {
    uint16_t p  :  1;   /* phase tag */
    uint16_t sc :  8;   /* status code */
    uint16_t sct    :  3;   /* status code type */
    uint16_t rsvd2  :  2;
    uint16_t m  :  1;   /* more */
    uint16_t dnr    :  1;   /* do not retry */
} nvme_status_t;
static_assert(sizeof(nvme_status_t) == 2, "Incorrect size");

/**
 *  * Completion queue entry
 *   */
typedef struct nvme_cpl {
    /* dword 0 */
    uint32_t        cdw0;   /* command-specific */

    /* dword 1 */
    uint32_t        rsvd1;

    /* dword 2 */
    uint16_t        sqhd;   /* submission queue head pointer */
    uint16_t        sqid;   /* submission queue identifier */

    /* dword 3 */
    uint16_t        cid;    /* command identifier */
    nvme_status_t      status;
} nvme_cpl_t;
static_assert(sizeof(nvme_cpl_t) == 16, "Incorrect size");

/**
 *  * Status code types
 *   */
enum nvme_status_code_type {
    NVME_SCT_GENERIC       = 0x0,
    NVME_SCT_COMMAND_SPECIFIC  = 0x1,
    NVME_SCT_MEDIA_ERROR   = 0x2,
    NVME_SCT_PATH      = 0x3,
    /* 0x4-0x6 - reserved */
    NVME_SCT_VENDOR_SPECIFIC   = 0x7,
};

/**
 *  * Generic command status codes
 *   */
enum nvme_generic_command_status_code {
    NVME_SC_SUCCESS                = 0x00,
    NVME_SC_INVALID_OPCODE         = 0x01,
    NVME_SC_INVALID_FIELD          = 0x02,
    NVME_SC_COMMAND_ID_CONFLICT        = 0x03,
    NVME_SC_DATA_TRANSFER_ERROR        = 0x04,
    NVME_SC_ABORTED_POWER_LOSS         = 0x05,
    NVME_SC_INTERNAL_DEVICE_ERROR      = 0x06,
    NVME_SC_ABORTED_BY_REQUEST         = 0x07,
    NVME_SC_ABORTED_SQ_DELETION        = 0x08,
    NVME_SC_ABORTED_FAILED_FUSED       = 0x09,
    NVME_SC_ABORTED_MISSING_FUSED      = 0x0a,
    NVME_SC_INVALID_NAMESPACE_OR_FORMAT    = 0x0b,
    NVME_SC_COMMAND_SEQUENCE_ERROR     = 0x0c,
    NVME_SC_INVALID_SGL_SEG_DESCRIPTOR     = 0x0d,
    NVME_SC_INVALID_NUM_SGL_DESCIRPTORS    = 0x0e,
    NVME_SC_DATA_SGL_LENGTH_INVALID        = 0x0f,
    NVME_SC_METADATA_SGL_LENGTH_INVALID    = 0x10,
    NVME_SC_SGL_DESCRIPTOR_TYPE_INVALID    = 0x11,
    NVME_SC_INVALID_CONTROLLER_MEM_BUF     = 0x12,
    NVME_SC_INVALID_PRP_OFFSET         = 0x13,
    NVME_SC_ATOMIC_WRITE_UNIT_EXCEEDED     = 0x14,
    NVME_SC_OPERATION_DENIED           = 0x15,
    NVME_SC_INVALID_SGL_OFFSET         = 0x16,
    /* 0x17 - reserved */
    NVME_SC_HOSTID_INCONSISTENT_FORMAT     = 0x18,
    NVME_SC_KEEP_ALIVE_EXPIRED         = 0x19,
    NVME_SC_KEEP_ALIVE_INVALID         = 0x1a,
    NVME_SC_ABORTED_PREEMPT            = 0x1b,
    NVME_SC_SANITIZE_FAILED            = 0x1c,
    NVME_SC_SANITIZE_IN_PROGRESS       = 0x1d,
    NVME_SC_SGL_DATA_BLOCK_GRANULARITY_INVALID = 0x1e,
    NVME_SC_COMMAND_INVALID_IN_CMB     = 0x1f,

    NVME_SC_LBA_OUT_OF_RANGE           = 0x80,
    NVME_SC_CAPACITY_EXCEEDED          = 0x81,
    NVME_SC_NAMESPACE_NOT_READY        = 0x82,
    NVME_SC_RESERVATION_CONFLICT               = 0x83,
    NVME_SC_FORMAT_IN_PROGRESS                 = 0x84,
};

/**
 *  * Command specific status codes
 *   */
enum nvme_command_specific_status_code {
    NVME_SC_COMPLETION_QUEUE_INVALID       = 0x00,
    NVME_SC_INVALID_QUEUE_IDENTIFIER       = 0x01,
    NVME_SC_MAXIMUM_QUEUE_SIZE_EXCEEDED    = 0x02,
    NVME_SC_ABORT_COMMAND_LIMIT_EXCEEDED   = 0x03,
    /* 0x04 - reserved */
    NVME_SC_ASYNC_EVENT_REQUEST_LIMIT_EXCEEDED = 0x05,
    NVME_SC_INVALID_FIRMWARE_SLOT      = 0x06,
    NVME_SC_INVALID_FIRMWARE_IMAGE     = 0x07,
    NVME_SC_INVALID_INTERRUPT_VECTOR       = 0x08,
    NVME_SC_INVALID_LOG_PAGE           = 0x09,
    NVME_SC_INVALID_FORMAT         = 0x0a,
    NVME_SC_FIRMWARE_REQ_CONVENTIONAL_RESET    = 0x0b,
    NVME_SC_INVALID_QUEUE_DELETION             = 0x0c,
    NVME_SC_FEATURE_ID_NOT_SAVEABLE            = 0x0d,
    NVME_SC_FEATURE_NOT_CHANGEABLE             = 0x0e,
    NVME_SC_FEATURE_NOT_NAMESPACE_SPECIFIC     = 0x0f,
    NVME_SC_FIRMWARE_REQ_NVM_RESET             = 0x10,
    NVME_SC_FIRMWARE_REQ_RESET                 = 0x11,
    NVME_SC_FIRMWARE_REQ_MAX_TIME_VIOLATION    = 0x12,
    NVME_SC_FIRMWARE_ACTIVATION_PROHIBITED     = 0x13,
    NVME_SC_OVERLAPPING_RANGE                  = 0x14,
    NVME_SC_NAMESPACE_INSUFFICIENT_CAPACITY    = 0x15,
    NVME_SC_NAMESPACE_ID_UNAVAILABLE           = 0x16,
    /* 0x17 - reserved */
    NVME_SC_NAMESPACE_ALREADY_ATTACHED         = 0x18,
    NVME_SC_NAMESPACE_IS_PRIVATE               = 0x19,
    NVME_SC_NAMESPACE_NOT_ATTACHED             = 0x1a,
    NVME_SC_THINPROVISIONING_NOT_SUPPORTED     = 0x1b,
    NVME_SC_CONTROLLER_LIST_INVALID            = 0x1c,
    NVME_SC_DEVICE_SELF_TEST_IN_PROGRESS   = 0x1d,
    NVME_SC_BOOT_PARTITION_WRITE_PROHIBITED    = 0x1e,
    NVME_SC_INVALID_CTRLR_ID           = 0x1f,
    NVME_SC_INVALID_SECONDARY_CTRLR_STATE  = 0x20,
    NVME_SC_INVALID_NUM_CTRLR_RESOURCES    = 0x21,
    NVME_SC_INVALID_RESOURCE_ID        = 0x22,

    NVME_SC_CONFLICTING_ATTRIBUTES     = 0x80,
    NVME_SC_INVALID_PROTECTION_INFO        = 0x81,
    NVME_SC_ATTEMPTED_WRITE_TO_RO_PAGE     = 0x82,
};

/**
 *  * Media error status codes
 *   */
enum nvme_media_error_status_code {
    NVME_SC_WRITE_FAULTS           = 0x80,
    NVME_SC_UNRECOVERED_READ_ERROR     = 0x81,
    NVME_SC_GUARD_CHECK_ERROR          = 0x82,
    NVME_SC_APPLICATION_TAG_CHECK_ERROR    = 0x83,
    NVME_SC_REFERENCE_TAG_CHECK_ERROR      = 0x84,
    NVME_SC_COMPARE_FAILURE            = 0x85,
    NVME_SC_ACCESS_DENIED          = 0x86,
    NVME_SC_DEALLOCATED_OR_UNWRITTEN_BLOCK     = 0x87,
};

/**
 *  * Path related status codes
 *   */
enum nvme_path_status_code {
    NVME_SC_INTERNAL_PATH_ERROR        = 0x00,

    NVME_SC_CONTROLLER_PATH_ERROR      = 0x60,

    NVME_SC_HOST_PATH_ERROR            = 0x70,
    NVME_SC_ABORTED_BY_HOST            = 0x71,
};

union nvme_create_io_cq0 {
    uint32_t    raw;
    struct {
        uint16_t    qid;
        uint16_t    qsize;   
    } bits; 
};

union nvme_create_io_cq1 {
    uint32_t    raw;
    struct {
        uint16_t    pc: 1;
        uint16_t    ien: 1;
        uint16_t    rsvd0: 14;
        uint16_t    iv;
    } bits; 
};

union nvme_create_io_sq0 {
    uint32_t    raw;
    struct {
        uint16_t    qid;
        uint16_t    qsize;   
    } bits; 
};

union nvme_create_io_sq1 {
    uint32_t    raw;
    struct {
        uint16_t    pc: 1;
        uint16_t    qprio: 2;
        uint16_t    rsvd0: 13;
        uint16_t    cqid;
    } bits; 
};

union nvme_delete_io_sq0 {
    uint32_t    raw;
    struct {
        uint16_t    qid;
        uint16_t    rsvd0;   
    } bits; 
};

union nvme_delete_io_cq0 {
    uint32_t    raw;
    struct {
        uint16_t    cqid;
        uint16_t    rsvd0;   
    } bits; 
};


/**
 *  * Admin opcodes
 *   */
enum nvme_admin_opcode {
    NVME_OPC_DELETE_IO_SQ          = 0x00,
    NVME_OPC_CREATE_IO_SQ          = 0x01,
    NVME_OPC_GET_LOG_PAGE          = 0x02,
    /* 0x03 - reserved */
    NVME_OPC_DELETE_IO_CQ          = 0x04,
    NVME_OPC_CREATE_IO_CQ          = 0x05,
    NVME_OPC_IDENTIFY              = 0x06,
    /* 0x07 - reserved */
    NVME_OPC_ABORT             = 0x08,
    NVME_OPC_SET_FEATURES          = 0x09,
    NVME_OPC_GET_FEATURES          = 0x0a,
    /* 0x0b - reserved */
    NVME_OPC_ASYNC_EVENT_REQUEST       = 0x0c,
    NVME_OPC_NS_MANAGEMENT         = 0x0d,
    /* 0x0e-0x0f - reserved */
    NVME_OPC_FIRMWARE_COMMIT           = 0x10,
    NVME_OPC_FIRMWARE_IMAGE_DOWNLOAD       = 0x11,

    NVME_OPC_DEVICE_SELF_TEST          = 0x14,
    NVME_OPC_NS_ATTACHMENT         = 0x15,

    NVME_OPC_KEEP_ALIVE            = 0x18,
    NVME_OPC_DIRECTIVE_SEND            = 0x19,
    NVME_OPC_DIRECTIVE_RECEIVE         = 0x1a,

    NVME_OPC_VIRTUALIZATION_MANAGEMENT     = 0x1c,
    NVME_OPC_NVME_MI_SEND          = 0x1d,
    NVME_OPC_NVME_MI_RECEIVE           = 0x1e,

    NVME_OPC_DOORBELL_BUFFER_CONFIG        = 0x7c,

    NVME_OPC_FORMAT_NVM            = 0x80,
    NVME_OPC_SECURITY_SEND         = 0x81,
    NVME_OPC_SECURITY_RECEIVE          = 0x82,

    NVME_OPC_SANITIZE              = 0x84,
};

/**
 *  * NVM command set opcodes
 *   */
enum nvme_nvm_opcode {
    NVME_OPC_FLUSH             = 0x00,
    NVME_OPC_WRITE             = 0x01,
    NVME_OPC_READ              = 0x02,
    /* 0x03 - reserved */
    NVME_OPC_WRITE_UNCORRECTABLE       = 0x04,
    NVME_OPC_COMPARE               = 0x05,
    /* 0x06-0x07 - reserved */
    NVME_OPC_WRITE_ZEROES          = 0x08,
    NVME_OPC_DATASET_MANAGEMENT        = 0x09,

    NVME_OPC_RESERVATION_REGISTER      = 0x0d,
    NVME_OPC_RESERVATION_REPORT        = 0x0e,

    NVME_OPC_RESERVATION_ACQUIRE       = 0x11,
    NVME_OPC_RESERVATION_RELEASE       = 0x15,
};

/**
 *  * Data transfer (bits 1:0) of an NVMe opcode.
 *   *
 *    * \sa nvme_opc_get_data_transfer
 *     */
enum nvme_data_transfer {
    /** Opcode does not transfer data */
    NVME_DATA_NONE             = 0,
    /** Opcode transfers data from host to controller (e.g. Write) */
    NVME_DATA_HOST_TO_CONTROLLER       = 1,
    /** Opcode transfers data from controller to host (e.g. Read) */
    NVME_DATA_CONTROLLER_TO_HOST       = 2,
    /** Opcode transfers data both directions */
    NVME_DATA_BIDIRECTIONAL            = 3
};

/**
 *  * Extract the Data Transfer bits from an NVMe opcode.
 *   *
 *    * This determines whether a command requires a data buffer and
 *     * which direction (host to controller or controller to host) it is
 *      * transferred.
 *       */
static inline enum nvme_data_transfer nvme_opc_get_data_transfer(uint8_t opc)
{
    return (enum nvme_data_transfer)(opc & 3);
}

enum nvme_feat {
    /* 0x00 - reserved */

    /** cdw11 layout defined by \ref nvme_feat_arbitration */
    NVME_FEAT_ARBITRATION              = 0x01,
    /** cdw11 layout defined by \ref nvme_feat_power_management */
    NVME_FEAT_POWER_MANAGEMENT             = 0x02,
    /** cdw11 layout defined by \ref nvme_feat_lba_range_type */
    NVME_FEAT_LBA_RANGE_TYPE               = 0x03,
    /** cdw11 layout defined by \ref nvme_feat_temperature_threshold */
    NVME_FEAT_TEMPERATURE_THRESHOLD            = 0x04,
    /** cdw11 layout defined by \ref nvme_feat_error_recovery */
    NVME_FEAT_ERROR_RECOVERY               = 0x05,
    /** cdw11 layout defined by \ref nvme_feat_volatile_write_cache */
    NVME_FEAT_VOLATILE_WRITE_CACHE         = 0x06,
    /** cdw11 layout defined by \ref nvme_feat_number_of_queues */
    NVME_FEAT_NUMBER_OF_QUEUES             = 0x07,
    /** cdw11 layout defined by \ref nvme_feat_interrupt_coalescing */
    NVME_FEAT_INTERRUPT_COALESCING         = 0x08,
    /** cdw11 layout defined by \ref nvme_feat_interrupt_vector_configuration */
    NVME_FEAT_INTERRUPT_VECTOR_CONFIGURATION       = 0x09,
    /** cdw11 layout defined by \ref nvme_feat_write_atomicity */
    NVME_FEAT_WRITE_ATOMICITY              = 0x0A,
    /** cdw11 layout defined by \ref nvme_feat_async_event_configuration */
    NVME_FEAT_ASYNC_EVENT_CONFIGURATION        = 0x0B,
    /** cdw11 layout defined by \ref nvme_feat_autonomous_power_state_transition */
    NVME_FEAT_AUTONOMOUS_POWER_STATE_TRANSITION    = 0x0C,
    /** cdw11 layout defined by \ref nvme_feat_host_mem_buffer */
    NVME_FEAT_HOST_MEM_BUFFER              = 0x0D,
    NVME_FEAT_TIMESTAMP                = 0x0E,
    /** cdw11 layout defined by \ref nvme_feat_keep_alive_timer */
    NVME_FEAT_KEEP_ALIVE_TIMER             = 0x0F,
    /** cdw11 layout defined by \ref nvme_feat_host_controlled_thermal_management */
    NVME_FEAT_HOST_CONTROLLED_THERMAL_MANAGEMENT   = 0x10,
    /** cdw11 layout defined by \ref nvme_feat_non_operational_power_state_config */
    NVME_FEAT_NON_OPERATIONAL_POWER_STATE_CONFIG   = 0x11,

    /* 0x12-0x77 - reserved */

    /* 0x78-0x7F - NVMe-MI features */

    /** cdw11 layout defined by \ref nvme_feat_software_progress_marker */
    NVME_FEAT_SOFTWARE_PROGRESS_MARKER         = 0x80,

    /** cdw11 layout defined by \ref nvme_feat_host_identifier */
    NVME_FEAT_HOST_IDENTIFIER              = 0x81,
    NVME_FEAT_HOST_RESERVE_MASK            = 0x82,
    NVME_FEAT_HOST_RESERVE_PERSIST         = 0x83,

    /* 0x84-0xBF - command set specific (reserved) */

    /* 0xC0-0xFF - vendor specific */
};

/** Bit set of attributes for DATASET MANAGEMENT commands. */
enum nvme_dsm_attribute {
    NVME_DSM_ATTR_INTEGRAL_READ        = 0x1,
    NVME_DSM_ATTR_INTEGRAL_WRITE       = 0x2,
    NVME_DSM_ATTR_DEALLOCATE           = 0x4,
};

typedef struct nvme_power_state_s {
    uint16_t mp;                /* bits 15:00: maximum power */

    uint8_t reserved1;

    uint8_t mps     : 1;        /* bit 24: max power scale */
    uint8_t nops        : 1;        /* bit 25: non-operational state */
    uint8_t reserved2   : 6;

    uint32_t enlat;             /* bits 63:32: entry latency in microseconds */
    uint32_t exlat;             /* bits 95:64: exit latency in microseconds */

    uint8_t rrt     : 5;        /* bits 100:96: relative read throughput */
    uint8_t reserved3   : 3;

    uint8_t rrl     : 5;        /* bits 108:104: relative read latency */
    uint8_t reserved4   : 3;

    uint8_t rwt     : 5;        /* bits 116:112: relative write throughput */
    uint8_t reserved5   : 3;

    uint8_t rwl     : 5;        /* bits 124:120: relative write latency */
    uint8_t reserved6   : 3;

    uint8_t reserved7[16];
} nvme_power_state_t;
static_assert(sizeof(nvme_power_state_t) == 32, "Incorrect size");

/** Identify command CNS value */
enum nvme_identify_cns {
    /** Identify namespace indicated in CDW1.NSID */
    NVME_IDENTIFY_NS               = 0x00,

    /** Identify controller */
    NVME_IDENTIFY_CTRLR            = 0x01,

    /** List active NSIDs greater than CDW1.NSID */
    NVME_IDENTIFY_ACTIVE_NS_LIST       = 0x02,

    /** List namespace identification descriptors */
    NVME_IDENTIFY_NS_ID_DESCRIPTOR_LIST    = 0x03,

    /** List allocated NSIDs greater than CDW1.NSID */
    NVME_IDENTIFY_ALLOCATED_NS_LIST        = 0x10,

    /** Identify namespace if CDW1.NSID is allocated */
    NVME_IDENTIFY_NS_ALLOCATED         = 0x11,

    /** Get list of controllers starting at CDW10.CNTID that are attached to CDW1.NSID */
    NVME_IDENTIFY_NS_ATTACHED_CTRLR_LIST   = 0x12,

    /** Get list of controllers starting at CDW10.CNTID */
    NVME_IDENTIFY_CTRLR_LIST           = 0x13,

    /** Get primary controller capabilities structure */
    NVME_IDENTIFY_PRIMARY_CTRLR_CAP        = 0x14,

    /** Get secondary controller list */
    NVME_IDENTIFY_SECONDARY_CTRLR_LIST     = 0x15,
};

/** NVMe over Fabrics controller model */
enum nvmf_ctrlr_model {
    /** NVM subsystem uses dynamic controller model */
    NVMF_CTRLR_MODEL_DYNAMIC           = 0,

    /** NVM subsystem uses static controller model */
    NVMF_CTRLR_MODEL_STATIC            = 1,
};

#define NVME_CTRLR_SN_LEN  20
#define NVME_CTRLR_MN_LEN  40
#define NVME_CTRLR_FR_LEN  8

/** Identify Controller data sgls.supported values */
enum nvme_sgls_supported {
    /** SGLs are not supported */
    NVME_SGLS_NOT_SUPPORTED            = 0,

    /** SGLs are supported with no alignment or granularity requirement. */
    NVME_SGLS_SUPPORTED            = 1,

    /** SGLs are supported with a DWORD alignment and granularity requirement. */
    NVME_SGLS_SUPPORTED_DWORD_ALIGNED      = 2,
};

/** Identify Controller data vwc.flush_broadcast values */
enum nvme_flush_broadcast {
    /** Support for NSID=FFFFFFFFh with Flush is not indicated. */
    NVME_FLUSH_BROADCAST_NOT_INDICATED     = 0,

    /* 01b: Reserved */

    /** Flush does not support NSID set to FFFFFFFFh. */
    NVME_FLUSH_BROADCAST_NOT_SUPPORTED     = 2,

    /** Flush supports NSID set to FFFFFFFFh. */
    NVME_FLUSH_BROADCAST_SUPPORTED     = 3
};

typedef struct __attribute__((packed)) nvme_ctrlr_data_s {
    /* bytes 0-255: controller capabilities and features */

    /** pci vendor id */
    uint16_t        vid;

    /** pci subsystem vendor id */
    uint16_t        ssvid;

    /** serial number */
    int8_t          sn[NVME_CTRLR_SN_LEN];

    /** model number */
    int8_t          mn[NVME_CTRLR_MN_LEN];

    /** firmware revision */
    uint8_t         fr[NVME_CTRLR_FR_LEN];

    /** recommended arbitration burst */
    uint8_t         rab;

    /** ieee oui identifier */
    uint8_t         ieee[3];

    /** controller multi-path I/O and namespace sharing capabilities */
    struct {
        uint8_t multi_port  : 1;
        uint8_t multi_host  : 1;
        uint8_t sr_iov      : 1;
        uint8_t reserved    : 5;
    } cmic;

    /** maximum data transfer size */
    uint8_t         mdts;

    /** controller id */
    uint16_t        cntlid;

    /** version */
    nvme_reg_vs_t ver;

    /** RTD3 resume latency */
    uint32_t        rtd3r;

    /** RTD3 entry latency */
    uint32_t        rtd3e;

    /** optional asynchronous events supported */
    struct {
        uint32_t    reserved1 : 8;

        /** Supports sending Namespace Attribute Notices. */
        uint32_t    ns_attribute_notices : 1;

        /** Supports sending Firmware Activation Notices. */
        uint32_t    fw_activation_notices : 1;

        uint32_t    reserved2 : 22;
    } oaes;

    /** controller attributes */
    struct {
        /** Supports 128-bit host identifier */
        uint32_t    host_id_exhid_supported: 1;

        /** Supports non-operational power state permissive mode */
        uint32_t    non_operational_power_state_permissive_mode: 1;

        uint32_t    reserved: 30;
    } ctratt;

    uint8_t         reserved_100[12];

    /** FRU globally unique identifier */
    uint8_t         fguid[16];

    uint8_t         reserved_128[128];

    /* bytes 256-511: admin command set attributes */

    /** optional admin command support */
    struct {
        /* supports security send/receive commands */
        uint16_t    security  : 1;

        /* supports format nvm command */
        uint16_t    format    : 1;

        /* supports firmware activate/download commands */
        uint16_t    firmware  : 1;

        /* supports ns manage/ns attach commands */
        uint16_t    ns_manage  : 1;

        /** Supports device self-test command (NVME_OPC_DEVICE_SELF_TEST) */
        uint16_t    device_self_test : 1;

        /** Supports NVME_OPC_DIRECTIVE_SEND and NVME_OPC_DIRECTIVE_RECEIVE */
        uint16_t    directives : 1;

        /** Supports NVMe-MI (NVME_OPC_NVME_MI_SEND, NVME_OPC_NVME_MI_RECEIVE) */
        uint16_t    nvme_mi : 1;

        /** Supports NVME_OPC_VIRTUALIZATION_MANAGEMENT */
        uint16_t    virtualization_management : 1;

        /** Supports NVME_OPC_DOORBELL_BUFFER_CONFIG */
        uint16_t    doorbell_buffer_config : 1;

        uint16_t    oacs_rsvd : 7;
    } oacs;

    /** abort command limit */
    uint8_t         acl;

    /** asynchronous event request limit */
    uint8_t         aerl;

    /** firmware updates */
    struct {
        /* first slot is read-only */
        uint8_t     slot1_ro  : 1;

        /* number of firmware slots */
        uint8_t     num_slots : 3;

        /* support activation without reset */
        uint8_t     activation_without_reset : 1;

        uint8_t     frmw_rsvd : 3;
    } frmw;

    /** log page attributes */
    struct {
        /* per namespace smart/health log page */
        uint8_t     ns_smart : 1;
        /* command effects log page */
        uint8_t     celp : 1;
        /* extended data for get log page */
        uint8_t     edlp: 1;
        /** telemetry log pages and notices */
        uint8_t     telemetry : 1;
        uint8_t     lpa_rsvd : 4;
    } lpa;

    /** error log page entries */
    uint8_t         elpe;

    /** number of power states supported */
    uint8_t         npss;

    /** admin vendor specific command configuration */
    struct {
        /* admin vendor specific commands use disk format */
        uint8_t     spec_format : 1;

        uint8_t     avscc_rsvd  : 7;
    } avscc;

    /** autonomous power state transition attributes */
    struct {
        /** controller supports autonomous power state transitions */
        uint8_t     supported  : 1;

        uint8_t     apsta_rsvd : 7;
    } apsta;

    /** warning composite temperature threshold */
    uint16_t        wctemp;

    /** critical composite temperature threshold */
    uint16_t        cctemp;

    /** maximum time for firmware activation */
    uint16_t        mtfa;

    /** host memory buffer preferred size */
    uint32_t        hmpre;

    /** host memory buffer minimum size */
    uint32_t        hmmin;

    /** total NVM capacity */
    uint64_t        tnvmcap[2];

    /** unallocated NVM capacity */
    uint64_t        unvmcap[2];

    /** replay protected memory block support */
    struct {
        uint8_t     num_rpmb_units  : 3;
        uint8_t     auth_method : 3;
        uint8_t     reserved1   : 2;

        uint8_t     reserved2;

        uint8_t     total_size;
        uint8_t     access_size;
    } rpmbs;

    /** extended device self-test time (in minutes) */
    uint16_t        edstt;

    /** device self-test options */
    union {
        uint8_t raw;
        struct {
            /** Device supports only one device self-test operation at a time */
            uint8_t one_only : 1;

            uint8_t reserved : 7;
        } bits;
    } dsto;

    /**
 *   * Firmware update granularity
 *       *
 *           * 4KB units
 *               * 0x00 = no information provided
 *                   * 0xFF = no restriction
 *                       */
    uint8_t         fwug;

    /**
 *   * Keep Alive Support
 *       *
 *           * Granularity of keep alive timer in 100 ms units
 *               * 0 = keep alive not supported
 *                   */
    uint16_t        kas;

    /** Host controlled thermal management attributes */
    union {
        uint16_t        raw;
        struct {
            uint16_t    supported : 1;
            uint16_t    reserved : 15;
        } bits;
    } hctma;

    /** Minimum thermal management temperature */
    uint16_t        mntmt;

    /** Maximum thermal management temperature */
    uint16_t        mxtmt;

    /** Sanitize capabilities */
    union {
        uint32_t    raw;
        struct {
            uint32_t    crypto_erase : 1;
            uint32_t    block_erase : 1;
            uint32_t    overwrite : 1;
            uint32_t    reserved : 29;
        } bits;
    } sanicap;

    uint8_t         reserved3[180];

    /* bytes 512-703: nvm command set attributes */

    /** submission queue entry size */
    struct {
        uint8_t     min : 4;
        uint8_t     max : 4;
    } sqes;

    /** completion queue entry size */
    struct {
        uint8_t     min : 4;
        uint8_t     max : 4;
    } cqes;

    uint16_t        maxcmd;

    /** number of namespaces */
    uint32_t        nn;

    /** optional nvm command support */
    struct {
        uint16_t    compare : 1;
        uint16_t    write_unc : 1;
        uint16_t    dsm: 1;
        uint16_t    write_zeroes: 1;
        uint16_t    set_features_save: 1;
        uint16_t    reservations: 1;
        uint16_t    timestamp: 1;
        uint16_t    reserved: 9;
    } oncs;

    /** fused operation support */
    uint16_t        fuses;

    /** format nvm attributes */
    struct {
        uint8_t     format_all_ns: 1;
        uint8_t     erase_all_ns: 1;
        uint8_t     crypto_erase_supported: 1;
        uint8_t     reserved: 5;
    } fna;

    /** volatile write cache */
    struct {
        uint8_t     present : 1;
        uint8_t     flush_broadcast : 2;
        uint8_t     reserved : 5;
    } vwc;

    /** atomic write unit normal */
    uint16_t        awun;

    /** atomic write unit power fail */
    uint16_t        awupf;

    /** NVM vendor specific command configuration */
    uint8_t         nvscc;

    uint8_t         reserved531;

    /** atomic compare & write unit */
    uint16_t        acwu;

    uint16_t        reserved534;

    /** SGL support */
    struct {
        uint32_t    supported : 2;
        uint32_t    keyed_sgl : 1;
        uint32_t    reserved1 : 13;
        uint32_t    bit_bucket_descriptor : 1;
        uint32_t    metadata_pointer : 1;
        uint32_t    oversized_sgl : 1;
        uint32_t    metadata_address : 1;
        uint32_t    sgl_offset : 1;
        uint32_t    transport_sgl : 1;
        uint32_t    reserved2 : 10;
    } sgls;

    uint8_t         reserved4[228];

    uint8_t         subnqn[256];

    uint8_t         reserved5[768];

    /** NVMe over Fabrics-specific fields */
    struct {
        /** I/O queue command capsule supported size (16-byte units) */
        uint32_t    ioccsz;

        /** I/O queue response capsule supported size (16-byte units) */
        uint32_t    iorcsz;

        /** In-capsule data offset (16-byte units) */
        uint16_t    icdoff;

        /** Controller attributes */
        struct {
            /** Controller model: \ref nvmf_ctrlr_model */
            uint8_t ctrlr_model : 1;
            uint8_t reserved : 7;
        } ctrattr;

        /** Maximum SGL block descriptors (0 = no limit) */
        uint8_t     msdbd;

        uint8_t     reserved[244];
    } nvmf_specific;

    /* bytes 2048-3071: power state descriptors */
    nvme_power_state_t  psd[32];

    /* bytes 3072-4095: vendor specific */
    uint8_t         vs[1024];
} nvme_ctrlr_data_t;
static_assert(sizeof(nvme_ctrlr_data_t) == 4096, "Incorrect size");

typedef struct __attribute__((packed)) nvme_primary_ctrl_capabilities_s {
    /**  controller id */
    uint16_t        cntlid;
    /**  port identifier */
    uint16_t        portid;
    /**  controller resource types */
    struct {
        uint8_t vq_supported    : 1;
        uint8_t vi_supported    : 1;
        uint8_t reserved    : 6;
    } crt;
    uint8_t         reserved[27];
    /** total number of VQ flexible resources */
    uint32_t        vqfrt;
    /** total number of VQ flexible resources assigned to secondary controllers */
    uint32_t        vqrfa;
    /** total number of VQ flexible resources allocated to primary controller */
    uint16_t        vqrfap;
    /** total number of VQ Private resources for the primary controller */
    uint16_t        vqprt;
    /** max number of VQ flexible Resources that may be assigned to a secondary controller */
    uint16_t        vqfrsm;
    /** preferred granularity of assigning and removing VQ Flexible Resources */
    uint16_t        vqgran;
    uint8_t         reserved1[16];
    /** total number of VI flexible resources for the primary and its secondary controllers */
    uint32_t        vifrt;
    /** total number of VI flexible resources assigned to the secondary controllers */
    uint32_t        virfa;
    /** total number of VI flexible resources currently allocated to the primary controller */
    uint16_t        virfap;
    /** total number of VI private resources for the primary controller */
    uint16_t        viprt;
    /** max number of VI flexible resources that may be assigned to a secondary controller */
    uint16_t        vifrsm;
    /** preferred granularity of assigning and removing VI flexible resources */
    uint16_t        vigran;
    uint8_t         reserved2[4016];
} nvme_primary_ctrl_capabilities_t;
static_assert(sizeof(nvme_primary_ctrl_capabilities_t) == 4096, "Incorrect size");

typedef struct __attribute__((packed)) nvme_secondary_ctrl_entry_s {
    /** controller identifier of the secondary controller */
    uint16_t        scid;
    /** controller identifier of the associated primary controller */
    uint16_t        pcid;
    /** indicates the state of the secondary controller */
    struct {
        uint8_t is_online   : 1;
        uint8_t reserved    : 7;
    } scs;
    uint8_t reserved[3];
    /** VF number if the secondary controller is an SR-IOV VF */
    uint16_t        vfn;
    /** number of VQ flexible resources assigned to the indicated secondary controller */
    uint16_t        nvq;
    /** number of VI flexible resources assigned to the indicated secondary controller */
    uint16_t        nvi;
    uint8_t         reserved1[18];
} nvme_secondary_ctrl_entry_t;
static_assert(sizeof(nvme_secondary_ctrl_entry_t) == 32, "Incorrect size");

typedef struct __attribute__((packed)) nvme_secondary_ctrl_list_s {
    /** number of Secondary controller entries in the list */
    uint8_t                 number;
    uint8_t                 reserved[31];
    nvme_secondary_ctrl_entry_t      entries[127];
} nvme_secondary_ctrl_list_t;
static_assert(sizeof(nvme_secondary_ctrl_list_t) == 4096, "Incorrect size");

typedef struct nvme_ns_data_s {
    /** namespace size */
    uint64_t        nsze;

    /** namespace capacity */
    uint64_t        ncap;

    /** namespace utilization */
    uint64_t        nuse;

    /** namespace features */
    struct {
        /** thin provisioning */
        uint8_t     thin_prov : 1;

        /** NAWUN, NAWUPF, and NACWU are defined for this namespace */
        uint8_t     ns_atomic_write_unit : 1;

        /** Supports Deallocated or Unwritten LBA error for this namespace */
        uint8_t     dealloc_or_unwritten_error : 1;

        /** Non-zero NGUID and EUI64 for namespace are never reused */
        uint8_t     guid_never_reused : 1;

        uint8_t     reserved1 : 4;
    } nsfeat;

    /** number of lba formats */
    uint8_t         nlbaf;

    /** formatted lba size */
    struct {
        uint8_t     format    : 4;
        uint8_t     extended  : 1;
        uint8_t     reserved2 : 3;
    } flbas;

    /** metadata capabilities */
    struct {
        /** metadata can be transferred as part of data prp list */
        uint8_t     extended  : 1;

        /** metadata can be transferred with separate metadata pointer */
        uint8_t     pointer   : 1;

        /** reserved */
        uint8_t     reserved3 : 6;
    } mc;

    /** end-to-end data protection capabilities */
    struct {
        /** protection information type 1 */
        uint8_t     pit1     : 1;

        /** protection information type 2 */
        uint8_t     pit2     : 1;

        /** protection information type 3 */
        uint8_t     pit3     : 1;

        /** first eight bytes of metadata */
        uint8_t     md_start : 1;

        /** last eight bytes of metadata */
        uint8_t     md_end   : 1;
    } dpc;

    /** end-to-end data protection type settings */
    struct {
        /** protection information type */
        uint8_t     pit       : 3;

        /** 1 == protection info transferred at start of metadata */
        /** 0 == protection info transferred at end of metadata */
        uint8_t     md_start  : 1;

        uint8_t     reserved4 : 4;
    } dps;

    /** namespace multi-path I/O and namespace sharing capabilities */
    struct {
        uint8_t     can_share : 1;
        uint8_t     reserved : 7;
    } nmic;

    /** reservation capabilities */
    union {
        struct {
            /** supports persist through power loss */
            uint8_t     persist : 1;

            /** supports write exclusive */
            uint8_t     write_exclusive : 1;

            /** supports exclusive access */
            uint8_t     exclusive_access : 1;

            /** supports write exclusive - registrants only */
            uint8_t     write_exclusive_reg_only : 1;

            /** supports exclusive access - registrants only */
            uint8_t     exclusive_access_reg_only : 1;

            /** supports write exclusive - all registrants */
            uint8_t     write_exclusive_all_reg : 1;

            /** supports exclusive access - all registrants */
            uint8_t     exclusive_access_all_reg : 1;

            /** supports ignore existing key */
            uint8_t     ignore_existing_key : 1;
        } rescap;
        uint8_t     raw;
    } nsrescap;
    /** format progress indicator */
    struct {
        uint8_t     percentage_remaining : 7;
        uint8_t     fpi_supported : 1;
    } fpi;

    /** deallocate logical features */
    union {
        uint8_t     raw;
        struct {
            /**
 *           * Value read from deallocated blocks
 *                       *
 *                                   * 000b = not reported
 *                                               * 001b = all bytes 0x00
 *                                                           * 010b = all bytes 0xFF
 *                                                                       *
 *                                                                                   * \ref nvme_dealloc_logical_block_read_value
 *                                                                                               */
            uint8_t read_value : 3;

            /** Supports Deallocate bit in Write Zeroes */
            uint8_t write_zero_deallocate : 1;

            /**
 *           * Guard field behavior for deallocated logical blocks
 *                       * 0: contains 0xFFFF
 *                                   * 1: contains CRC for read value
 *                                               */
            uint8_t guard_value : 1;

            uint8_t reserved : 3;
        } bits;
    } dlfeat;

    /** namespace atomic write unit normal */
    uint16_t        nawun;

    /** namespace atomic write unit power fail */
    uint16_t        nawupf;

    /** namespace atomic compare & write unit */
    uint16_t        nacwu;

    /** namespace atomic boundary size normal */
    uint16_t        nabsn;

    /** namespace atomic boundary offset */
    uint16_t        nabo;

    /** namespace atomic boundary size power fail */
    uint16_t        nabspf;

    /** namespace optimal I/O boundary in logical blocks */
    uint16_t        noiob;

    /** NVM capacity */
    uint64_t        nvmcap[2];

    uint8_t         reserved64[40];

    /** namespace globally unique identifier */
    uint8_t         nguid[16];

    /** IEEE extended unique identifier */
    uint64_t        eui64;

    /** lba format support */
    struct {
        /** metadata size */
        uint32_t    ms    : 16;

        /** lba data size */
        uint32_t    lbads     : 8;

        /** relative performance */
        uint32_t    rp    : 2;

        uint32_t    reserved6 : 6;
    } lbaf[16];

    uint8_t         reserved6[192];

    uint8_t         vendor_specific[3712];
} nvme_ns_data_t;
static_assert(sizeof(nvme_ns_data_t) == 4096, "Incorrect size");

/**
 *  * Deallocated logical block features - read value
 *   */
enum nvme_dealloc_logical_block_read_value {
    /** Not reported */
    NVME_DEALLOC_NOT_REPORTED  = 0,

    /** Deallocated blocks read 0x00 */
    NVME_DEALLOC_READ_00   = 1,

    /** Deallocated blocks read 0xFF */
    NVME_DEALLOC_READ_FF   = 2,
};

/**
 *  * Reservation Type Encoding
 *   */
enum nvme_reservation_type {
    /* 0x00 - reserved */

    /* Write Exclusive Reservation */
    NVME_RESERVE_WRITE_EXCLUSIVE       = 0x1,

    /* Exclusive Access Reservation */
    NVME_RESERVE_EXCLUSIVE_ACCESS      = 0x2,

    /* Write Exclusive - Registrants Only Reservation */
    NVME_RESERVE_WRITE_EXCLUSIVE_REG_ONLY  = 0x3,

    /* Exclusive Access - Registrants Only Reservation */
    NVME_RESERVE_EXCLUSIVE_ACCESS_REG_ONLY = 0x4,

    /* Write Exclusive - All Registrants Reservation */
    NVME_RESERVE_WRITE_EXCLUSIVE_ALL_REGS  = 0x5,

    /* Exclusive Access - All Registrants Reservation */
    NVME_RESERVE_EXCLUSIVE_ACCESS_ALL_REGS = 0x6,

    /* 0x7-0xFF - Reserved */
};

typedef struct nvme_reservation_acquire_data_s {
    /** current reservation key */
    uint64_t        crkey;
    /** preempt reservation key */
    uint64_t        prkey;
} nvme_reservation_acquire_data_t;
static_assert(sizeof(nvme_reservation_acquire_data_t) == 16, "Incorrect size");

/**
 *  * Reservation Acquire action
 *   */
enum nvme_reservation_acquire_action {
    NVME_RESERVE_ACQUIRE       = 0x0,
    NVME_RESERVE_PREEMPT       = 0x1,
    NVME_RESERVE_PREEMPT_ABORT     = 0x2,
};

typedef struct __attribute__((packed)) nvme_reservation_status_data_s {
    /** reservation action generation counter */
    uint32_t        gen;
    /** reservation type */
    uint8_t         rtype;
    /** number of registered controllers */
    uint16_t        regctl;
    uint16_t        reserved1;
    /** persist through power loss state */
    uint8_t         ptpls;
    uint8_t         reserved[14];
} nvme_reservation_status_data_t;
static_assert(sizeof(nvme_reservation_status_data_t) == 24, "Incorrect size");

typedef struct __attribute__((packed)) nvme_reservation_status_extended_data_s {
    nvme_reservation_status_data_t      data;
    uint8_t                     reserved[40];
} nvme_reservation_status_extended_data_t;
static_assert(sizeof(nvme_reservation_status_extended_data_t) == 64,
           "Incorrect size");

typedef struct __attribute__((packed)) nvme_registered_ctrlr_data_s {
    /** controller id */
    uint16_t        cntlid;
    /** reservation status */
    struct {
        uint8_t     status    : 1;
        uint8_t     reserved1 : 7;
    } rcsts;
    uint8_t         reserved2[5];
    /** 64-bit host identifier */
    uint64_t        hostid;
    /** reservation key */
    uint64_t        rkey;
} nvme_registered_ctrlr_data_t;
static_assert(sizeof(nvme_registered_ctrlr_data_t) == 24, "Incorrect size");

typedef struct __attribute__((packed)) nvme_registered_ctrlr_extended_data_s {
    /** controller id */
    uint16_t        cntlid;
    /** reservation status */
    struct {
        uint8_t     status    : 1;
        uint8_t     reserved1 : 7;
    } rcsts;
    uint8_t         reserved2[5];
    /** reservation key */
    uint64_t        rkey;
    /** 128-bit host identifier */
    uint8_t         hostid[16];
    uint8_t         reserved3[32];
} nvme_registered_ctrlr_extended_data_t;
static_assert(sizeof(nvme_registered_ctrlr_extended_data_t) == 64, "Incorrect size");

/**
 *  * Change persist through power loss state for
 *   *  Reservation Register command
 *    */
enum nvme_reservation_register_cptpl {
    NVME_RESERVE_PTPL_NO_CHANGES       = 0x0,
    NVME_RESERVE_PTPL_CLEAR_POWER_ON       = 0x2,
    NVME_RESERVE_PTPL_PERSIST_POWER_LOSS   = 0x3,
};

/**
 *  * Registration action for Reservation Register command
 *   */
enum nvme_reservation_register_action {
    NVME_RESERVE_REGISTER_KEY      = 0x0,
    NVME_RESERVE_UNREGISTER_KEY    = 0x1,
    NVME_RESERVE_REPLACE_KEY       = 0x2,
};

typedef struct nvme_reservation_register_data_s {
    /** current reservation key */
    uint64_t        crkey;
    /** new reservation key */
    uint64_t        nrkey;
} nvme_reservation_register_data_t;
static_assert(sizeof(nvme_reservation_register_data_t) == 16, "Incorrect size");

typedef struct nvme_reservation_key_data_s {
    /** current reservation key */
    uint64_t        crkey;
} nvme_reservation_key_data_t;
static_assert(sizeof(nvme_reservation_key_data_t) == 8, "Incorrect size");

/**
 *  * Reservation Release action
 *   */
enum nvme_reservation_release_action {
    NVME_RESERVE_RELEASE       = 0x0,
    NVME_RESERVE_CLEAR         = 0x1,
};

/**
 *  * Reservation notification log page type
 *   */
enum nvme_reservation_notification_log_page_type {
    NVME_RESERVATION_LOG_PAGE_EMPTY    = 0x0,
    NVME_REGISTRATION_PREEMPTED    = 0x1,
    NVME_RESERVATION_RELEASED      = 0x2,
    NVME_RESERVATION_PREEMPTED     = 0x3,
};

/**
 *  * Reservation notification log
 *   */
typedef struct nvme_reservation_notification_log_s {
    /** 64-bit incrementing reservation notification log page count */
    uint64_t    log_page_count;
    /** Reservation notification log page type */
    uint8_t     type;
    /** Number of additional available reservation notification log pages */
    uint8_t     num_avail_log_pages;
    uint8_t     reserved[2];
    uint32_t    nsid;
    uint8_t     reserved1[48];
} nvme_reservation_notification_log_t;
static_assert(sizeof(nvme_reservation_notification_log_t) == 64, "Incorrect size");

/* Mask Registration Preempted Notificaton */
#define NVME_REGISTRATION_PREEMPTED_MASK   (1U << 1)
/* Mask Reservation Released Notification */
#define NVME_RESERVATION_RELEASED_MASK (1U << 2)
/* Mask Reservation Preempted Notification */
#define NVME_RESERVATION_PREEMPTED_MASK    (1U << 3)

/**
 *  * Log page identifiers for NVME_OPC_GET_LOG_PAGE
 *   */
enum nvme_log_page {
    /* 0x00 - reserved */

    /** Error information (mandatory) - \ref nvme_error_information_entry */
    NVME_LOG_ERROR         = 0x01,

    /** SMART / health information (mandatory) - \ref nvme_health_information_page */
    NVME_LOG_HEALTH_INFORMATION    = 0x02,

    /** Firmware slot information (mandatory) - \ref nvme_firmware_page */
    NVME_LOG_FIRMWARE_SLOT     = 0x03,

    /** Changed namespace list (optional) */
    NVME_LOG_CHANGED_NS_LIST   = 0x04,

    /** Command effects log (optional) */
    NVME_LOG_COMMAND_EFFECTS_LOG   = 0x05,

    /** Device self test (optional) */
    NVME_LOG_DEVICE_SELF_TEST  = 0x06,

    /** Host initiated telemetry log (optional) */
    NVME_LOG_TELEMETRY_HOST_INITIATED  = 0x07,

    /** Controller initiated telemetry log (optional) */
    NVME_LOG_TELEMETRY_CTRLR_INITIATED = 0x08,

    /* 0x09-0x6F - reserved */

    /** Discovery(refer to the NVMe over Fabrics specification) */
    NVME_LOG_DISCOVERY     = 0x70,

    /* 0x71-0x7f - reserved for NVMe over Fabrics */

    /** Reservation notification (optional) */
    NVME_LOG_RESERVATION_NOTIFICATION  = 0x80,

    /* 0x81-0xBF - I/O command set specific */

    /* 0xC0-0xFF - vendor specific */
};

/**
 *  * Error information log page (\ref NVME_LOG_ERROR)
 *   */
typedef struct nvme_error_information_entry_s {
    uint64_t        error_count;
    uint16_t        sqid;
    uint16_t        cid;
    nvme_status_t      status;
    uint16_t        error_location;
    uint64_t        lba;
    uint32_t        nsid;
    uint8_t         vendor_specific;
    uint8_t         trtype;
    uint8_t         reserved30[2];
    uint64_t        command_specific;
    uint16_t        trtype_specific;
    uint8_t         reserved42[22];
} nvme_error_information_entry_t;
static_assert(sizeof(nvme_error_information_entry_t) == 64, "Incorrect size");

union nvme_critical_warning_state {
    uint8_t     raw;

    struct {
        uint8_t available_spare     : 1;
        uint8_t temperature     : 1;
        uint8_t device_reliability  : 1;
        uint8_t read_only       : 1;
        uint8_t volatile_memory_backup  : 1;
        uint8_t reserved        : 3;
    } bits;
};
static_assert(sizeof(union nvme_critical_warning_state) == 1, "Incorrect size");

/**
 *  * SMART / health information page (\ref NVME_LOG_HEALTH_INFORMATION)
 *   */
typedef struct __attribute__((packed)) nvme_health_information_page_s {
    union nvme_critical_warning_state  critical_warning;

    uint16_t        temperature;
    uint8_t         available_spare;
    uint8_t         available_spare_threshold;
    uint8_t         percentage_used;

    uint8_t         reserved[26];

    /*
 *   * Note that the following are 128-bit values, but are
 *       *  defined as an array of 2 64-bit values.
 *           */
    /* Data Units Read is always in 512-byte units. */
    uint64_t        data_units_read[2];
    /* Data Units Written is always in 512-byte units. */
    uint64_t        data_units_written[2];
    /* For NVM command set, this includes Compare commands. */
    uint64_t        host_read_commands[2];
    uint64_t        host_write_commands[2];
    /* Controller Busy Time is reported in minutes. */
    uint64_t        controller_busy_time[2];
    uint64_t        power_cycles[2];
    uint64_t        power_on_hours[2];
    uint64_t        unsafe_shutdowns[2];
    uint64_t        media_errors[2];
    uint64_t        num_error_info_log_entries[2];
    /* Controller temperature related. */
    uint32_t        warning_temp_time;
    uint32_t        critical_temp_time;
    uint16_t        temp_sensor[8];

    uint8_t         reserved2[296];
} nvme_health_information_page_t;
static_assert(sizeof(nvme_health_information_page_t) == 512, "Incorrect size");

/* Commands Supported and Effects Data Structure */
typedef struct nvme_cmds_and_effect_entry_s {
    /** Command Supported */
    uint16_t csupp : 1;

    /** Logic Block Content Change  */
    uint16_t lbcc  : 1;

    /** Namespace Capability Change */
    uint16_t ncc   : 1;

    /** Namespace Inventory Change */
    uint16_t nic   : 1;

    /** Controller Capability Change */
    uint16_t ccc   : 1;

    uint16_t reserved1 : 11;

    /* Command Submission and Execution recommendation
 *   * 000 - No command submission or execution restriction
 *       * 001 - Submitted when there is no outstanding command to same NS
 *           * 010 - Submitted when there is no outstanding command to any NS
 *               * others - Reserved
 *                   * \ref command_submission_and_execution in section 5.14.1.5 NVMe Revision 1.3
 *                       */
    uint16_t cse : 3;

    uint16_t reserved2 : 13;
} nvme_cmds_and_effect_entry_t;

/* Commands Supported and Effects Log Page */
typedef struct nvme_cmds_and_effect_log_page_s {
    /** Commands Supported and Effects Data Structure for the Admin Commands */
    nvme_cmds_and_effect_entry_t      admin_cmds_supported[256];

    /** Commands Supported and Effects Data Structure for the IO Commands */
    nvme_cmds_and_effect_entry_t      io_cmds_supported[256];

    uint8_t reserved0[2048];
} nvme_cmds_and_effect_log_page_t;
static_assert(sizeof(nvme_cmds_and_effect_log_page_t) == 4096, "Incorrect size");

/*
 *  * Get Log Page ? Telemetry Host/Controller Initiated Log (Log Identifiers 07h/08h)
 *   */
typedef struct nvme_telemetry_log_page_hdr_s {
    uint8_t    lpi;         /* Log page identifier */
    uint8_t    rsvd[4];
    uint8_t    ieee_oui[3];
    uint16_t   dalb1;       /* Data area 1 last block */
    uint16_t   dalb2;       /* Data area 2 last block */
    uint16_t   dalb3;       /* Data area 3 last block */
    uint8_t    rsvd1[368];
    uint8_t    ctrlr_avail;     /* Controller initiated data avail */
    uint8_t    ctrlr_gen;       /* Controller initiated telemetry data generation */
    uint8_t    rsnident[128];   /* Reason identifier */
    uint8_t    telemetry_datablock[0];
} nvme_telemetry_log_page_hdr_t;
static_assert(sizeof(nvme_telemetry_log_page_hdr_t) == 512, "Incorrect size");

/**
 *  * Asynchronous Event Type
 *   */
enum nvme_async_event_type {
    /* Error Status */
    NVME_ASYNC_EVENT_TYPE_ERROR    = 0x0,
    /* SMART/Health Status */
    NVME_ASYNC_EVENT_TYPE_SMART    = 0x1,
    /* Notice */
    NVME_ASYNC_EVENT_TYPE_NOTICE   = 0x2,
    /* 0x3 - 0x5 Reserved */

    /* I/O Command Set Specific Status */
    NVME_ASYNC_EVENT_TYPE_IO       = 0x6,
    /* Vendor Specific */
    NVME_ASYNC_EVENT_TYPE_VENDOR   = 0x7,
};

/**
 *  * Asynchronous Event Information for Error Status
 *   */
enum nvme_async_event_info_error {
    /* Write to Invalid Doorbell Register */
    NVME_ASYNC_EVENT_WRITE_INVALID_DB      = 0x0,
    /* Invalid Doorbell Register Write Value */
    NVME_ASYNC_EVENT_INVALID_DB_WRITE      = 0x1,
    /* Diagnostic Failure */
    NVME_ASYNC_EVENT_DIAGNOSTIC_FAILURE    = 0x2,
    /* Persistent Internal Error */
    NVME_ASYNC_EVENT_PERSISTENT_INTERNAL   = 0x3,
    /* Transient Internal Error */
    NVME_ASYNC_EVENT_TRANSIENT_INTERNAL    = 0x4,
    /* Firmware Image Load Error */
    NVME_ASYNC_EVENT_FW_IMAGE_LOAD     = 0x5,

    /* 0x6 - 0xFF Reserved */
};

/**
 *  * Asynchronous Event Information for SMART/Health Status
 *   */
enum nvme_async_event_info_smart {
    /* NVM Subsystem Reliability */
    NVME_ASYNC_EVENT_SUBSYSTEM_RELIABILITY = 0x0,
    /* Temperature Threshold */
    NVME_ASYNC_EVENT_TEMPERATURE_THRESHOLD = 0x1,
    /* Spare Below Threshold */
    NVME_ASYNC_EVENT_SPARE_BELOW_THRESHOLD = 0x2,

    /* 0x3 - 0xFF Reserved */
};

/**
 *  * Asynchronous Event Information for Notice
 *   */
enum nvme_async_event_info_notice {
    /* Namespace Attribute Changed */
    NVME_ASYNC_EVENT_NS_ATTR_CHANGED       = 0x0,
    /* Firmware Activation Starting */
    NVME_ASYNC_EVENT_FW_ACTIVATION_START   = 0x1,
    /* Telemetry Log Changed */
    NVME_ASYNC_EVENT_TELEMETRY_LOG_CHANGED = 0x2,

    /* 0x3 - 0xFF Reserved */
};

/**
 *  * Asynchronous Event Information for NVM Command Set Specific Status
 *   */
enum nvme_async_event_info_nvm_command_set {
    /* Reservation Log Page Avaiable */
    NVME_ASYNC_EVENT_RESERVATION_LOG_AVAIL = 0x0,
    /* Sanitize Operation Completed */
    NVME_ASYNC_EVENT_SANITIZE_COMPLETED    = 0x1,

    /* 0x2 - 0xFF Reserved */
};

/**
 *  * Asynchronous Event Request Completion
 *   */
union nvme_async_event_completion {
    uint32_t raw;
    struct {
        uint32_t async_event_type   : 3;
        uint32_t reserved1      : 5;
        uint32_t async_event_info   : 8;
        uint32_t log_page_identifier    : 8;
        uint32_t reserved2      : 8;
    } bits;
};
static_assert(sizeof(union nvme_async_event_completion) == 4, "Incorrect size");

/**
 *  * Data used by Set Features/Get Features \ref NVME_FEAT_ARBITRATION
 *   */
union nvme_feat_arbitration {
    uint32_t raw;
    struct {
        /** Arbitration Burst */
        uint32_t ab : 3;

        uint32_t reserved : 5;

        /** Low Priority Weight */
        uint32_t lpw : 8;

        /** Medium Priority Weight */
        uint32_t mpw : 8;

        /** High Priority Weight */
        uint32_t hpw : 8;
    } bits;
};
static_assert(sizeof(union nvme_feat_arbitration) == 4, "Incorrect size");

/**
 *  * Data used by Set Features/Get Features \ref NVME_FEAT_POWER_MANAGEMENT
 *   */
union nvme_feat_power_management {
    uint32_t raw;
    struct {
        /** Power State */
        uint32_t ps : 5;

        /** Workload Hint */
        uint32_t wh : 3;

        uint32_t reserved : 24;
    } bits;
};
static_assert(sizeof(union nvme_feat_power_management) == 4, "Incorrect size");

/**
 *  * Data used by Set Features/Get Features \ref NVME_FEAT_LBA_RANGE_TYPE
 *   */
union nvme_feat_lba_range_type {
    uint32_t raw;
    struct {
        /** Number of LBA Ranges */
        uint32_t num : 6;

        uint32_t reserved : 26;
    } bits;
};
static_assert(sizeof(union nvme_feat_lba_range_type) == 4, "Incorrect size");

/**
 *  * Data used by Set Features/Get Features \ref NVME_FEAT_TEMPERATURE_THRESHOLD
 *   */
union nvme_feat_temperature_threshold {
    uint32_t raw;
    struct {
        /** Temperature Threshold */
        uint32_t tmpth : 16;

        /** Threshold Temperature Select */
        uint32_t tmpsel : 4;

        /** Threshold Type Select */
        uint32_t thsel : 2;

        uint32_t reserved : 10;
    } bits;
};
static_assert(sizeof(union nvme_feat_temperature_threshold) == 4, "Incorrect size");

/**
 *  * Data used by Set Features/Get Features \ref NVME_FEAT_ERROR_RECOVERY
 *   */
union nvme_feat_error_recovery {
    uint32_t raw;
    struct {
        /** Time Limited Error Recovery */
        uint32_t tler : 16;

        /** Deallocated or Unwritten Logical Block Error Enable */
        uint32_t dulbe : 1;

        uint32_t reserved : 15;
    } bits;
};
static_assert(sizeof(union nvme_feat_error_recovery) == 4, "Incorrect size");

/**
 *  * Data used by Set Features/Get Features \ref NVME_FEAT_VOLATILE_WRITE_CACHE
 *   */
union nvme_feat_volatile_write_cache {
    uint32_t raw;
    struct {
        /** Volatile Write Cache Enable */
        uint32_t wce : 1;

        uint32_t reserved : 31;
    } bits;
};
static_assert(sizeof(union nvme_feat_volatile_write_cache) == 4, "Incorrect size");

/**
 *  * Data used by Set Features/Get Features \ref NVME_FEAT_NUMBER_OF_QUEUES
 *   */
union nvme_feat_number_of_queues {
    uint32_t raw;
    struct {
        /** Number of I/O Submission Queues Requested */
        uint32_t nsqr : 16;

        /** Number of I/O Completion Queues Requested */
        uint32_t ncqr : 16;
    } bits;
};
static_assert(sizeof(union nvme_feat_number_of_queues) == 4, "Incorrect size");

/**
 *  * Data used by Set Features/Get Features \ref NVME_FEAT_INTERRUPT_COALESCING
 *   */
union nvme_feat_interrupt_coalescing {
    uint32_t raw;
    struct {
        /** Aggregation Threshold */
        uint32_t thr : 8;

        /** Aggregration time */
        uint32_t time : 8;

        uint32_t reserved : 16;
    } bits;
};
static_assert(sizeof(union nvme_feat_interrupt_coalescing) == 4, "Incorrect size");

/**
 *  * Data used by Set Features/Get Features \ref NVME_FEAT_INTERRUPT_VECTOR_CONFIGURATION
 *   */
union nvme_feat_interrupt_vector_configuration {
    uint32_t raw;
    struct {
        /** Interrupt Vector */
        uint32_t iv : 16;

        /** Coalescing Disable */
        uint32_t cd : 1;

        uint32_t reserved : 15;
    } bits;
};
static_assert(sizeof(union nvme_feat_interrupt_vector_configuration) == 4,
           "Incorrect size");

/**
 *  * Data used by Set Features/Get Features \ref NVME_FEAT_WRITE_ATOMICITY
 *   */
union nvme_feat_write_atomicity {
    uint32_t raw;
    struct {
        /** Disable Normal */
        uint32_t dn : 1;

        uint32_t reserved : 31;
    } bits;
};
static_assert(sizeof(union nvme_feat_write_atomicity) == 4, "Incorrect size");

/**
 *  * Data used by Set Features / Get Features \ref NVME_FEAT_ASYNC_EVENT_CONFIGURATION
 *   */
union nvme_feat_async_event_configuration {
    uint32_t raw;
    struct {
        union nvme_critical_warning_state crit_warn;
        uint32_t ns_attr_notice     : 1;
        uint32_t fw_activation_notice   : 1;
        uint32_t telemetry_log_notice   : 1;
        uint32_t reserved       : 21;
    } bits;
};
static_assert(sizeof(union nvme_feat_async_event_configuration) == 4, "Incorrect size");
/* Old name defined for compatibility */
#define nvme_async_event_config nvme_feat_async_event_configuration

/**
 *  * Data used by Set Features/Get Features \ref NVME_FEAT_AUTONOMOUS_POWER_STATE_TRANSITION
 *   */
union nvme_feat_autonomous_power_state_transition {
    uint32_t raw;
    struct {
        /** Autonomous Power State Transition Enable */
        uint32_t apste : 1;

        uint32_t reserved : 31;
    } bits;
};
static_assert(sizeof(union nvme_feat_autonomous_power_state_transition) == 4,
           "Incorrect size");

/**
 *  * Data used by Set Features/Get Features \ref NVME_FEAT_HOST_MEM_BUFFER
 *   */
union nvme_feat_host_mem_buffer {
    uint32_t raw;
    struct {
        /** Enable Host Memory */
        uint32_t ehm : 1;

        /** Memory Return */
        uint32_t mr : 1;

        uint32_t reserved : 30;
    } bits;
};
static_assert(sizeof(union nvme_feat_host_mem_buffer) == 4, "Incorrect size");

/**
 *  * Data used by Set Features/Get Features \ref NVME_FEAT_KEEP_ALIVE_TIMER
 *   */
union nvme_feat_keep_alive_timer {
    uint32_t raw;
    struct {
        /** Keep Alive Timeout */
        uint32_t kato : 32;
    } bits;
};
static_assert(sizeof(union nvme_feat_keep_alive_timer) == 4, "Incorrect size");

/**
 *  * Data used by Set Features/Get Features \ref NVME_FEAT_HOST_CONTROLLED_THERMAL_MANAGEMENT
 *   */
union nvme_feat_host_controlled_thermal_management {
    uint32_t raw;
    struct {
        /** Thermal Management Temperature 2 */
        uint32_t tmt2 : 16;

        /** Thermal Management Temperature 1 */
        uint32_t tmt1 : 16;
    } bits;
};
static_assert(sizeof(union nvme_feat_host_controlled_thermal_management) == 4,
           "Incorrect size");

/**
 *  * Data used by Set Features/Get Features \ref NVME_FEAT_NON_OPERATIONAL_POWER_STATE_CONFIG
 *   */
union nvme_feat_non_operational_power_state_config {
    uint32_t raw;
    struct {
        /** Non-Operational Power State Permissive Mode Enable */
        uint32_t noppme : 1;

        uint32_t reserved : 31;
    } bits;
};
static_assert(sizeof(union nvme_feat_non_operational_power_state_config) == 4,
           "Incorrect size");

/**
 *  * Data used by Set Features/Get Features \ref NVME_FEAT_SOFTWARE_PROGRESS_MARKER
 *   */
union nvme_feat_software_progress_marker {
    uint32_t raw;
    struct {
        /** Pre-boot Software Load Count */
        uint32_t pbslc : 8;

        uint32_t reserved : 24;
    } bits;
};
static_assert(sizeof(union nvme_feat_software_progress_marker) == 4, "Incorrect size");

/**
 *  * Data used by Set Features/Get Features \ref NVME_FEAT_HOST_IDENTIFIER
 *   */
union nvme_feat_host_identifier {
    uint32_t raw;
    struct {
        /** Enable Extended Host Identifier */
        uint32_t exhid : 1;

        uint32_t reserved : 31;
    } bits;
};
static_assert(sizeof(union nvme_feat_host_identifier) == 4, "Incorrect size");

/**
 *  * Firmware slot information page (\ref NVME_LOG_FIRMWARE_SLOT)
 *   */
typedef struct nvme_firmware_page_s {
    struct {
        uint8_t active_slot : 3; /**< Slot for current FW */
        uint8_t reserved3   : 1;
        uint8_t next_reset_slot : 3; /**< Slot that will be active at next controller reset */
        uint8_t reserved7   : 1;
    } afi;

    uint8_t         reserved[7];
    uint8_t         revision[7][8]; /** Revisions for 7 slots (ASCII strings) */
    uint8_t         reserved2[448];
} nvme_firmware_page_t;
static_assert(sizeof(nvme_firmware_page_t) == 512, "Incorrect size");

/**
 *  * Namespace attachment Type Encoding
 *   */
enum nvme_ns_attach_type {
    /* Controller attach */
    NVME_NS_CTRLR_ATTACH   = 0x0,

    /* Controller detach */
    NVME_NS_CTRLR_DETACH   = 0x1,

    /* 0x2-0xF - Reserved */
};

/**
 *  * Namespace management Type Encoding
 *   */
enum nvme_ns_management_type {
    /* Create */
    NVME_NS_MANAGEMENT_CREATE  = 0x0,

    /* Delete */
    NVME_NS_MANAGEMENT_DELETE  = 0x1,

    /* 0x2-0xF - Reserved */
};

typedef struct nvme_ns_list_s {
    uint32_t ns_list[1024];
} nvme_ns_list_t;
static_assert(sizeof(nvme_ns_list_t) == 4096, "Incorrect size");

/**
 *  * Namespace identification descriptor type
 *   *
 *    * \sa nvme_ns_id_desc
 *     */
enum nvme_nidt {
    /** IEEE Extended Unique Identifier */
    NVME_NIDT_EUI64        = 0x01,

    /** Namespace GUID */
    NVME_NIDT_NGUID        = 0x02,

    /** Namespace UUID */
    NVME_NIDT_UUID     = 0x03,
};

typedef struct nvme_ns_id_desc_s {
    /** Namespace identifier type */
    uint8_t nidt;

    /** Namespace identifier length (length of nid field) */
    uint8_t nidl;

    uint8_t reserved2;
    uint8_t reserved3;

    /** Namespace identifier */
    uint8_t nid[];
} nvme_ns_id_desc_t;
static_assert(sizeof(nvme_ns_id_desc_t) == 4, "Incorrect size");

typedef struct nvme_ctrlr_list_s {
    uint16_t ctrlr_count;
    uint16_t ctrlr_list[2047];
} nvme_ctrlr_list_t;
static_assert(sizeof(nvme_ctrlr_list_t) == 4096, "Incorrect size");

enum nvme_secure_erase_setting {
    NVME_FMT_NVM_SES_NO_SECURE_ERASE   = 0x0,
    NVME_FMT_NVM_SES_USER_DATA_ERASE   = 0x1,
    NVME_FMT_NVM_SES_CRYPTO_ERASE  = 0x2,
};

enum nvme_pi_location {
    NVME_FMT_NVM_PROTECTION_AT_TAIL    = 0x0,
    NVME_FMT_NVM_PROTECTION_AT_HEAD    = 0x1,
};

enum nvme_pi_type {
    NVME_FMT_NVM_PROTECTION_DISABLE        = 0x0,
    NVME_FMT_NVM_PROTECTION_TYPE1      = 0x1,
    NVME_FMT_NVM_PROTECTION_TYPE2      = 0x2,
    NVME_FMT_NVM_PROTECTION_TYPE3      = 0x3,
};

enum nvme_metadata_setting {
    NVME_FMT_NVM_METADATA_TRANSFER_AS_BUFFER   = 0x0,
    NVME_FMT_NVM_METADATA_TRANSFER_AS_LBA  = 0x1,
};

typedef struct nvme_format_s {
    uint32_t    lbaf        : 4;
    uint32_t    ms      : 1;
    uint32_t    pi      : 3;
    uint32_t    pil     : 1;
    uint32_t    ses     : 3;
    uint32_t    reserved    : 20;
} nvme_format_t;
static_assert(sizeof(nvme_format_t) == 4, "Incorrect size");

typedef struct nvme_protection_info_s {
    uint16_t    guard;
    uint16_t    app_tag;
    uint32_t    ref_tag;
} nvme_protection_info_t;
static_assert(sizeof(nvme_protection_info_t) == 8, "Incorrect size");

/** Parameters for NVME_OPC_FIRMWARE_COMMIT cdw10: commit action */
enum nvme_fw_commit_action {
    /**
 *   * Downloaded image replaces the image specified by
 *       * the Firmware Slot field. This image is not activated.
 *           */
    NVME_FW_COMMIT_REPLACE_IMG         = 0x0,
    /**
 *   * Downloaded image replaces the image specified by
 *       * the Firmware Slot field. This image is activated at the next reset.
 *           */
    NVME_FW_COMMIT_REPLACE_AND_ENABLE_IMG  = 0x1,
    /**
 *   * The image specified by the Firmware Slot field is
 *       * activated at the next reset.
 *           */
    NVME_FW_COMMIT_ENABLE_IMG          = 0x2,
    /**
 *   * The image specified by the Firmware Slot field is
 *       * requested to be activated immediately without reset.
 *           */
    NVME_FW_COMMIT_RUN_IMG         = 0x3,
};

/** Parameters for NVME_OPC_FIRMWARE_COMMIT cdw10 */
typedef struct nvme_fw_commit_s {
    /**
 *   * Firmware Slot. Specifies the firmware slot that shall be used for the
 *       * Commit Action. The controller shall choose the firmware slot (slot 1 - 7)
 *           * to use for the operation if the value specified is 0h.
 *               */
    uint32_t    fs      : 3;
    /**
 *   * Commit Action. Specifies the action that is taken on the image downloaded
 *       * with the Firmware Image Download command or on a previously downloaded and
 *           * placed image.
 *               */
    uint32_t    ca      : 3;
    uint32_t    reserved    : 26;
} nvme_fw_commit_t;
static_assert(sizeof(nvme_fw_commit_t) == 4, "Incorrect size");

#define nvme_cpl_is_error(cpl)         \
    ((cpl)->status.sc != NVME_SC_SUCCESS ||    \
     (cpl)->status.sct != NVME_SCT_GENERIC)

#define nvme_cpl_is_success(cpl)   (!nvme_cpl_is_error(cpl))

#define nvme_cpl_is_pi_error(cpl)                      \
    ((cpl)->status.sct == NVME_SCT_MEDIA_ERROR &&          \
     ((cpl)->status.sc == NVME_SC_GUARD_CHECK_ERROR ||         \
      (cpl)->status.sc == NVME_SC_APPLICATION_TAG_CHECK_ERROR ||   \
      (cpl)->status.sc == NVME_SC_REFERENCE_TAG_CHECK_ERROR))

/** Enable protection information checking of the Logical Block Reference Tag field */
#define NVME_IO_FLAGS_PRCHK_REFTAG (1U << 26)
/** Enable protection information checking of the Application Tag field */
#define NVME_IO_FLAGS_PRCHK_APPTAG (1U << 27)
/** Enable protection information checking of the Guard field */
#define NVME_IO_FLAGS_PRCHK_GUARD (1U << 28)
/** The protection information is stripped or inserted when set this bit */
#define NVME_IO_FLAGS_PRACT (1U << 29)
#define NVME_IO_FLAGS_FORCE_UNIT_ACCESS (1U << 30)
#define NVME_IO_FLAGS_LIMITED_RETRY (1U << 31)

typedef struct nvme_dev_cmd_regs {
    nvme_reg_cap_t              cap;     //controller capabilities
    nvme_reg_vs_t               vs;      //version
    uint32_t                    intms;   //interrupt mask set
    uint32_t                    intmc;   //interrupt mask clear
    nvme_reg_cc_t               cc;      //controller configuration
    uint32_t                    rsvd;    //rsvd
    nvme_reg_csts_t             csts;    //controller status
    uint32_t                    nssr;    //NVM subsystem reset
    nvme_reg_aqa_t              aqa;     //admin queue attributes
    nvme_reg_asq_t              asq;     //admin submission queue base address
    nvme_reg_acq_t              acq;     //admin completion queue base address
    uint32_t                    cmbloc;  //controller memory buffer location
    uint32_t                    cmbsz;   //controller memory buffer size
    uint32_t                    bpinfo;  //boot partition info
    uint32_t                    bprsel;  //boot partition read select
    uint64_t                    bpmbl;   //boot partition memory buffer location
    uint8_t                     rsvd1[4016];
} nvme_dev_cmd_regs_t;

typedef union nvme_dev_cmd {
    uint32_t                    words[16];
    nvme_dev_cmd_regs_t         regs;
} nvme_dev_cmd_t;

static_assert(sizeof(nvme_dev_cmd_regs_t) == NVME_DEV_PAGE_SIZE);
#define NVME_REG_MAP_CC_OFFSET offsetof(nvme_dev_cmd_regs_t, cc)

#pragma pack(pop)

#endif //__NVME_IF_HPP__

