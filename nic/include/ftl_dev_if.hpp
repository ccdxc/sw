/*
 * Copyright 2020 Pensando Systems, Inc.  All rights reserved.
 */

#ifndef _FTL_DEV_IF_H_
#define _FTL_DEV_IF_H_

#include "nic/apollo/api/include/athena/pds_flow_age.h"
#include "nic/p4/ftl_dev/include/ftl_dev_shared.h"

#if !defined(_LINUX_TYPES_H) && !defined(_LINUX_TYPES_H_)
typedef uint16_t __le16;
typedef uint32_t __le32;
typedef uint64_t __le64;
#endif

namespace ftl_dev_if {

#define FTL_DEV_IFNAMSIZ                        16
#define FTL_DEV_STRINGIFY(x)                    #x
#define FTL_DEV_INDEX_STRINGIFY(x)              [x] = FTL_DEV_STRINGIFY(x)
#define FTL_DEV_CASE_STRINGIFY(x)               case x: return FTL_DEV_STRINGIFY(x)

enum ftl_devcmd_opcode {
    FTL_DEVCMD_OPCODE_NOP                       = 0,
    FTL_DEVCMD_OPCODE_RESET                     = 1,
    FTL_DEVCMD_OPCODE_IDENTIFY                  = 2,
    FTL_DEVCMD_OPCODE_LIF_IDENTIFY              = 3,
    FTL_DEVCMD_OPCODE_LIF_INIT                  = 4,
    FTL_DEVCMD_OPCODE_LIF_RESET                 = 5,
    FTL_DEVCMD_OPCODE_LIF_GETATTR               = 6,
    FTL_DEVCMD_OPCODE_LIF_SETATTR               = 7,
    FTL_DEVCMD_OPCODE_POLLERS_INIT              = 8,
    FTL_DEVCMD_OPCODE_POLLERS_DEQ_BURST         = 9,
    FTL_DEVCMD_OPCODE_POLLERS_FLUSH             = 10,
    FTL_DEVCMD_OPCODE_SCANNERS_INIT             = 11,
    FTL_DEVCMD_OPCODE_SCANNERS_START            = 12,
    FTL_DEVCMD_OPCODE_SCANNERS_START_SINGLE     = 13,
    FTL_DEVCMD_OPCODE_SCANNERS_STOP             = 14,
    FTL_DEVCMD_OPCODE_MPU_TIMESTAMP_INIT        = 15,
    FTL_DEVCMD_OPCODE_MPU_TIMESTAMP_START       = 16,
    FTL_DEVCMD_OPCODE_MPU_TIMESTAMP_STOP        = 17,
    FTL_DEVCMD_OPCODE_ACCEL_AGING_CONTROL       = 18,
};

#define FTL_DEVCMD_OPCODE_CASE_TABLE                                    \
    FTL_DEV_CASE_STRINGIFY(FTL_DEVCMD_OPCODE_NOP);                      \
    FTL_DEV_CASE_STRINGIFY(FTL_DEVCMD_OPCODE_RESET);                    \
    FTL_DEV_CASE_STRINGIFY(FTL_DEVCMD_OPCODE_IDENTIFY);                 \
    FTL_DEV_CASE_STRINGIFY(FTL_DEVCMD_OPCODE_LIF_IDENTIFY);             \
    FTL_DEV_CASE_STRINGIFY(FTL_DEVCMD_OPCODE_LIF_INIT);                 \
    FTL_DEV_CASE_STRINGIFY(FTL_DEVCMD_OPCODE_LIF_RESET);                \
    FTL_DEV_CASE_STRINGIFY(FTL_DEVCMD_OPCODE_LIF_GETATTR);              \
    FTL_DEV_CASE_STRINGIFY(FTL_DEVCMD_OPCODE_LIF_SETATTR);              \
    FTL_DEV_CASE_STRINGIFY(FTL_DEVCMD_OPCODE_POLLERS_INIT);             \
    FTL_DEV_CASE_STRINGIFY(FTL_DEVCMD_OPCODE_POLLERS_FLUSH);            \
    FTL_DEV_CASE_STRINGIFY(FTL_DEVCMD_OPCODE_POLLERS_DEQ_BURST);        \
    FTL_DEV_CASE_STRINGIFY(FTL_DEVCMD_OPCODE_SCANNERS_INIT);            \
    FTL_DEV_CASE_STRINGIFY(FTL_DEVCMD_OPCODE_SCANNERS_START);           \
    FTL_DEV_CASE_STRINGIFY(FTL_DEVCMD_OPCODE_SCANNERS_START_SINGLE);    \
    FTL_DEV_CASE_STRINGIFY(FTL_DEVCMD_OPCODE_SCANNERS_STOP);            \
    FTL_DEV_CASE_STRINGIFY(FTL_DEVCMD_OPCODE_MPU_TIMESTAMP_INIT);       \
    FTL_DEV_CASE_STRINGIFY(FTL_DEVCMD_OPCODE_MPU_TIMESTAMP_START);      \
    FTL_DEV_CASE_STRINGIFY(FTL_DEVCMD_OPCODE_MPU_TIMESTAMP_STOP);       \
    FTL_DEV_CASE_STRINGIFY(FTL_DEVCMD_OPCODE_ACCEL_AGING_CONTROL);      \
    
enum ftl_status_code {
    FTL_RC_SUCCESS    = 0,    /* Success */
    FTL_RC_EVERSION   = 1,    /* Incorrect version for request */
    FTL_RC_EOPCODE    = 2,    /* Invalid cmd opcode */
    FTL_RC_EIO        = 3,    /* I/O error */
    FTL_RC_EPERM      = 4,    /* Permission denied */
    FTL_RC_EQID       = 5,    /* Bad qid */
    FTL_RC_EQTYPE     = 6,    /* Bad qtype */
    FTL_RC_ENOENT     = 7,    /* No such element */
    FTL_RC_EINTR      = 8,    /* operation interrupted */
    FTL_RC_EAGAIN     = 9,    /* Try again */
    FTL_RC_ENOMEM     = 10,   /* Out of memory */
    FTL_RC_EFAULT     = 11,   /* Bad address */
    FTL_RC_EBUSY      = 12,   /* Device or resource busy */
    FTL_RC_EEXIST     = 13,   /* object already exists */
    FTL_RC_EINVAL     = 14,   /* Invalid argument */
    FTL_RC_ENOSPC     = 15,   /* No space left or alloc failure */
    FTL_RC_ERANGE     = 16,   /* Parameter out of range */
    FTL_RC_BAD_ADDR   = 17,   /* Descriptor contains a bad ptr */
    FTL_RC_ERROR      = 18,   /* Generic error */
};

#define FTL_RC_STR_TABLE                            \
    FTL_DEV_INDEX_STRINGIFY(FTL_RC_SUCCESS),        \
    FTL_DEV_INDEX_STRINGIFY(FTL_RC_EVERSION),       \
    FTL_DEV_INDEX_STRINGIFY(FTL_RC_EOPCODE),        \
    FTL_DEV_INDEX_STRINGIFY(FTL_RC_EIO),            \
    FTL_DEV_INDEX_STRINGIFY(FTL_RC_EPERM),          \
    FTL_DEV_INDEX_STRINGIFY(FTL_RC_EQID),           \
    FTL_DEV_INDEX_STRINGIFY(FTL_RC_EQTYPE),         \
    FTL_DEV_INDEX_STRINGIFY(FTL_RC_ENOENT),         \
    FTL_DEV_INDEX_STRINGIFY(FTL_RC_EINTR),          \
    FTL_DEV_INDEX_STRINGIFY(FTL_RC_EAGAIN),         \
    FTL_DEV_INDEX_STRINGIFY(FTL_RC_ENOMEM),         \
    FTL_DEV_INDEX_STRINGIFY(FTL_RC_EFAULT),         \
    FTL_DEV_INDEX_STRINGIFY(FTL_RC_EBUSY),          \
    FTL_DEV_INDEX_STRINGIFY(FTL_RC_EEXIST),         \
    FTL_DEV_INDEX_STRINGIFY(FTL_RC_EINVAL),         \
    FTL_DEV_INDEX_STRINGIFY(FTL_RC_ENOSPC),         \
    FTL_DEV_INDEX_STRINGIFY(FTL_RC_ERANGE),         \
    FTL_DEV_INDEX_STRINGIFY(FTL_RC_BAD_ADDR),       \
    FTL_DEV_INDEX_STRINGIFY(FTL_RC_ERROR),          \

typedef int ftl_status_code_t;

#pragma pack(push, 1)

/**
 * admin_cmd_t - General admin command format
 * @opcode:   Opcode for the command
 * @lif_index: software lif index
 * @cmd_data: Opcode-specific command bytes
 */
typedef struct admin_cmd {
    uint8_t     opcode;
    uint8_t     rsvd;
    __le16      lif_index;
    uint8_t     cmd_data[60];
} admin_cmd_t;

/**
 * admin_cpl_t - General admin command completion format
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 *                 Non-zero = Error code.  Error codes are
 *                 command-specific.
 * @cpl_index: The index in the descriptor ring for which this
 *             is the completion.
 * @cmd_data:   Command-specific bytes.
 */
typedef struct admin_cpl {
    uint8_t     status;
    uint8_t     rsvd;
    __le16      cpl_index;
    uint8_t     cmd_data[12];
} admin_cpl_t;

/**
 * nop_cmd_t - NOP command
 * @opcode: opcode
 */
typedef struct nop_cmd {
    uint8_t     opcode;
    uint8_t     rsvd[63];
} nop_cmd_t;

/**
 * nop_cpl_t - NOP command completion
 * @status: The status of the command.  Values for status are:
 *             0 = Successful completion
 */
typedef struct nop_cpl {
    uint8_t     status;
    uint8_t     rsvd[15];
} nop_cpl_t;

enum ftl_dev_type {
    FTL_DEV_TYPE_BASE       = 0,
    FTL_DEV_TYPE_MAX,
};

enum ftl_qtype {
    FTL_QTYPE_SCANNER_SESSION   = FTL_DEV_QTYPE_SCANNER_SESSION,
    FTL_QTYPE_SCANNER_CONNTRACK = FTL_DEV_QTYPE_SCANNER_CONNTRACK,
    FTL_QTYPE_POLLER            = FTL_DEV_QTYPE_POLLER,
    FTL_QTYPE_MPU_TIMESTAMP     = FTL_DEV_QTYPE_MPU_TIMESTAMP,
    FTL_QTYPE_MAX               = 8,
};

/**
 * dev_reset_cmd_t - Device reset command
 * @opcode: opcode
 * @type:   device type
 */
typedef struct dev_reset_cmd {
    uint8_t     opcode;
    uint8_t     type;
    uint8_t     rsvd[62];
} dev_reset_cmd_t;

/**
 * dev_reset_cpl_t - Reset command completion
 * @status: The status of the command.  Values for status are:
 *             0 = Successful completion
 */
typedef struct dev_reset_cpl {
    uint8_t     status;
    uint8_t     rsvd[15];
} dev_reset_cpl_t;

#define IDENTITY_VERSION_1      1

/**
 * dev_identify_cmd_t - Driver/device identify command
 * @opcode: opcode
 * @type:   device type
 * @ver:    Highest version of identify supported by driver
 */
typedef struct dev_identify_cmd {
    uint8_t     opcode;
    uint8_t     type;
    uint8_t     ver;
    uint8_t     rsvd[61];
} dev_identify_cmd_t;

enum ftl_lif_type {
    FTL_LIF_TYPE_BASE       = 0,
    FTL_LIF_TYPE_MAX,
};

/**
 * dev_identify_cpl_t - Driver/device identify command completion
 * @status: The status of the command.  Values for status are:
 *             0 = Successful completion
 *             1 = Version not supported by device
 * @ver:    Version of identify returned by device
 */
typedef struct dev_identify_cpl {
    uint8_t     status;
    uint8_t     ver;
    uint8_t     rsvd[14];
} dev_identify_cpl_t;

/**
 * union dev_identity - device identity information
 * @nlifs:            Number of LIFs provisioned
 */
typedef union dev_identity {
    struct {
        uint8_t         version;
        uint8_t         type;
        uint8_t         rsvd1[2];
        uint8_t         rsvd2[4];
        __le32          nlifs;
    } base;
    __le32 words[512];
} dev_identity_t;

/**
 * LIF configuration
 * @name:           lif name
 */
typedef union lif_config {
    struct {
        char            name[FTL_DEV_IFNAMSIZ];
    };
    __le32 words[128];
} lif_config_t;

/**
 * struct lif_identity - lif identity information
 *
 * @hw_index: lif hardware index
 * @qcount: queue counts per queue-type
 * @burst_sz: scanner burst size per queue-type, if any
 * @burst_resched_time_us: scanner burst reschedule time per queue-type, if any
 */
typedef struct queue_identity {
    __le32              qcount;
    __le32              qdepth;
    __le32              burst_sz;
    __le32              burst_resched_time_us;
} queue_identity_t;

typedef union lif_identity {
    struct {
        uint8_t         version;
        uint8_t         rsvd;
        __le16          hw_index;
        lif_config_t    config;
        queue_identity_t qident[FTL_QTYPE_MAX];
    } base;
    __le32 words[512];
} lif_identity_t;

/**
 * struct lif_identify_cmd - lif identify command
 * @opcode:  opcode
 * @type:    lif type (enum lif_type)
 * @ver:     Highest version of identify supported by driver
 * @lif_index: software lif index
 */
typedef struct lif_identify_cmd {
    uint8_t     opcode;
    uint8_t     type;
    __le16      lif_index;
    uint8_t     ver;
    uint8_t     rsvd2[59];
} lif_identify_cmd_t;

/**
 * struct lif_identify_cpl - lif identify command completion
 * @status:  status of the command (enum status_code)
 * @type:    lif type (enum lif_type)
 * @ver:     version of identify returned by device
 */
typedef struct lif_identify_cpl {
    uint8_t     status;
    uint8_t     ver;
    uint8_t     rsvd2[14];
} lif_identify_cpl_t;

/**
 * lif_init_cmd_t - LIF init command
 * @opcode:  opcode
 * @lif_index: software lif index
 */
typedef struct lif_init_cmd {
    uint8_t     opcode;
    uint8_t     rsvd1;
    __le16      lif_index;
    uint8_t     rsvd2[60];
} lif_init_cmd_t;

/**
 * lif_init_cpl_t - LIF init command completion
 * @status: The status of the command.  Values for status are:
 *             0 = Successful completion
 * @hw_index: hardware lif index
 */
typedef struct lif_init_cpl {
    uint8_t     status;
    uint8_t     rsvd1;
    __le16      hw_index;
    uint8_t     rsvd2[12];
} lif_init_cpl_t;

/**
 * struct lif_reset_cmd - LIF reset command
 * @opcode: opcode
 * @lif_index: software lif index
 * @quiesce_check: check for queues reaching quiesce state
 */
typedef struct lif_reset_cmd {
    uint8_t     opcode;
    uint8_t     rsvd1;
    __le16      lif_index;
    uint8_t     quiesce_check;
    uint8_t     rsvd2[59];
} lif_reset_cmd_t;

typedef struct admin_cpl lif_reset_cpl_t;

/**
 * enum lif_attr - List of LIF attributes
 */
enum lif_attr {
    FTL_LIF_ATTR_NAME         = 0,
    FTL_LIF_ATTR_NORMAL_AGE_TMO,
    FTL_LIF_ATTR_ACCEL_AGE_TMO,
    FTL_LIF_ATTR_METRICS,
    FTL_LIF_ATTR_FORCE_SESSION_EXPIRED_TS,  // for debugging on SIM platform
    FTL_LIF_ATTR_FORCE_CONNTRACK_EXPIRED_TS,
};

typedef pds_flow_age_timeouts_t lif_attr_age_tmo_t;

/**
 * LIF metrics
 */
typedef struct {
    uint64_t   total_cb_cfg_discards;
    uint64_t   total_scan_invocations;
    uint64_t   total_expired_entries;
    uint64_t   min_range_elapsed_ns;
    uint64_t   avg_min_range_elapsed_ns;
    uint64_t   max_range_elapsed_ns;
    uint64_t   avg_max_range_elapsed_ns;
} lif_attr_scanners_metrics_t;

typedef struct {
    uint64_t   total_num_qposts;
    uint64_t   total_num_qfulls;
} lif_attr_pollers_metrics_t;

typedef struct {
    uint64_t   total_num_updates;
} lif_attr_mpu_timestamp_metrics_t;

typedef union {
    lif_attr_scanners_metrics_t scanners;
    lif_attr_pollers_metrics_t  pollers;
    lif_attr_mpu_timestamp_metrics_t mpu_timestamp;
} lif_attr_metrics_t;

/**
 * lif_setattr_cmd_t - Set LIF attributes
 * @opcode:     Opcode
 * @lif_index:  software lif index
 * @attr:       Attribute type (enum lif_attr)
 * @name:       The netdev name string, 0 terminated
 */
typedef struct lif_setattr_cmd {
    uint8_t    opcode;
    uint8_t    attr;
    __le16     lif_index;
    union {
        char                name[FTL_DEV_IFNAMSIZ];
        lif_attr_age_tmo_t  age_tmo;
        uint8_t             force_expired_ts;
        uint8_t             rsvd[60];
    };
} lif_setattr_cmd_t;

typedef struct lif_setattr_cpl {
    uint8_t     status;
    uint8_t     rsvd1;
    __le16      cpl_index;
    uint8_t     rsvd2[12];
} lif_setattr_cpl_t;

/**
 * struct lif_getattr_cmd - Get LIF attributes
 * @lif_index:  software lif index
 * @attr:       Attribute type (enum lif_attr)
 * @qtype:      For use by FTL_LIF_ATTR_METRICS
 */
typedef struct lif_getattr_cmd {
    uint8_t     opcode;
    uint8_t     attr;
    __le16      lif_index;
    uint8_t     qtype;
    uint8_t     rsvd[59];
} lif_getattr_cmd_t;

typedef struct lif_getattr_cpl {
    uint8_t     status;
    uint8_t     rsvd1;
    __le16      cpl_index;
    union {
        // char name[FTL_DEV_IFNAMSIZ];
        uint8_t         force_expired_ts;
        uint8_t         rsvd2[12];
    };
} lif_getattr_cpl_t;

/**
 * pollers_init_cmd_t - SW pollers init command
 * @opcode:     opcode
 * @qtype:      queue type
 * @lif_index:  software lif index
 * @pid:        Process ID
 * @qcount:     number of queues (or rings) to initialize
 * @wrings_base_addr: work rings base address
 * @wrings_total_sz: total size of work rings
 * @qdepth:    poller queue depth
 */
typedef struct pollers_init_cmd {
    uint8_t     opcode;
    uint8_t     qtype;
    __le16      lif_index;
    __le16      pid;
    __le32      qcount;
    __le32      qdepth;
    __le64      wrings_base_addr;
    __le32      wrings_total_sz;
    uint8_t     rsvd[38];
} pollers_init_cmd_t;

/**
 * pollers_init_cpl_t - SW pollers init command completion
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @qtype:      queue type
 * @cpl_index:  The index in the descriptor ring for which this
 *              is the completion.
 */
typedef struct pollers_init_cpl {
    uint8_t     status;
    uint8_t     qtype;
    __le16      cpl_index;
    uint8_t     rsvd[12];
} pollers_init_cpl_t;

/**
 * pollers_flush_cmd_t - SW pollers flush command
 * @opcode:     opcode
 * @qtype:      Queue type
 * @lif_index:  software lif index
 */
typedef struct pollers_flush_cmd {
    uint8_t     opcode;
    uint8_t     qtype;
    __le16      lif_index;
    uint8_t     rsvd2[60];
} pollers_flush_cmd_t;

/**
 * pollers_flush_cpl_t - SW pollers flush complete command
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @qtype:      queue type
 * @cpl_index:  The index in the descriptor ring for which this
 *              is the completion.
 */
typedef struct pollers_flush_cpl {
    uint8_t     opcode;
    uint8_t     qtype;
    __le16      cpl_index;
    uint8_t     rsvd[12];
} pollers_flush_cpl_t;

/**
 * pollers_deq_burst_cmd_t - SW pollers burst dequeue from a single queue command
 * @opcode:     opcode
 * @qtype:      queue type
 * @lif_index:  software lif index
 * @burst_count: requested burst count
 * @buf_sz:     total buffer size;
 * @index:      queue index
 */
typedef struct pollers_deq_burst_cmd {
    uint8_t     opcode;
    uint8_t     qtype;
    __le16      lif_index;
    __le32      index;
    __le32      burst_count;
    __le32      buf_sz;
    uint8_t     rsvd2[48];
} pollers_deq_burst_cmd_t;

/**
 * pollers_deq_burst_cpl_t - SSW pollers burst dequeue from a single queue complete
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @qtype:      queue type
 * @read_count: count of burst actually read 
 * @cpl_index:  The index in the descriptor ring for which this
 *              is the completion.
 */
typedef struct pollers_deq_burst_cpl {
    uint8_t     status;
    uint8_t     qtype;
    __le16      cpl_index;
    __le32      read_count;
    uint8_t     rsvd2[8];
} pollers_deq_burst_cpl_t;

/**
 * scanners_init_cmd_t - HW scanners init command
 * @opcode:     opcode
 * @qtype:      queue type (session or conntrack)
 * @lif_index:  software lif index
 * @pid:        Process ID
 * @qcount:     number of queues to initialize
 * @cos:        Class of service for the queues.
 * @scan_addr_base: session or conntrack table base address
 * @scan_id_base: starting session/conntrack ID to scan from
 * @scan_table_sz: total number of table entries
 * @scan_burst_sz: number of entries to scan prior to rescheduling with
 *                scan_resched_time
 * @scan_resched_time: reschedule time after a scan burst
 * @poller_lif: poller lif ID
 * @poller_qid: poller queue ID
 * @poller_qcount: number of poller queues available
 * @poller_qdepth: poller queue depth
 * @poller_qtype: poller queue type
 * @cos_override: override FW selected cos with cmd cos
 */
typedef struct scanners_init_cmd {
    uint8_t     opcode;
    uint8_t     qtype;
    __le16      lif_index;
    __le16      pid;
    uint8_t     rsvd1;
    uint8_t     cos;
    __le32      qcount;
    __le64      scan_addr_base;
    __le32      scan_table_sz;
    __le32      scan_id_base;
    __le32      scan_burst_sz;
    __le32      scan_resched_time;
    __le32      poller_lif;
    __le32      poller_qcount;
    __le32      poller_qdepth;
    uint8_t     poller_qtype;
    uint8_t     cos_override;
    uint8_t     rsvd2[16];
} scanners_init_cmd_t;

/**
 * scanners_init_cpl_t - HW scanners init command completion
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @qtype:      queue type
 * @cpl_index:  The index in the descriptor ring for which this
 *              is the completion.
 */
typedef struct scanners_init_cpl {
    uint8_t     status;
    uint8_t     qtype;
    __le16      cpl_index;
    uint8_t     rsvd[12];
} scanners_init_cpl_t;

/**
 * scanners_start_cmd_t - HW scanners start command
 * @opcode:     opcode
 * @lif_index:  software lif index
 */
typedef struct scanners_start_cmd {
    uint8_t     opcode;
    uint8_t     rsvd1;
    __le16      lif_index;
    uint8_t     rsvd2[60];
} scanners_start_cmd_t;

/**
 * scanners_start_cpl_t - HW scanners start complete command
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @lif_index:  software lif index
 */
typedef struct scanners_start_cpl {
    uint8_t     status;
    uint8_t     rsvd1;
    __le16      cpl_index;
    uint8_t     rsvd2[12];
} scanners_start_cpl_t;

/**
 * scanners_stop_cmd_t - HW scanners stop command
 * @opcode:     opcode
 * @lif_index:  software lif index
 * @quiesce_check: check for queues reaching quiesce state
 */
typedef struct scanners_stop_cmd {
    uint8_t     opcode;
    uint8_t     rsvd1;
    __le16      lif_index;
    uint8_t     quiesce_check;
    uint8_t     rsvd2[59];
} scanners_stop_cmd_t;

/**
 * scanners_stop_cpl_t - HW scanners stop complete command
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @cpl_index:  The index in the descriptor ring for which this
 *              is the completion.
 */
typedef struct scanners_stop_cpl {
    uint8_t     opcode;
    uint8_t     rsvd1;
    __le16      cpl_index;
    uint8_t     rsvd2[12];
} scanners_stop_cpl_t;

/**
 * scanners_start_single_cmd_t - HW scanners start single queue command,
 *                               i.e., reschedule the given queue
 * @opcode:     opcode
 * @qtype:      queue type
 * @lif_index:  software lif index
 * @index:      queue index
 */
typedef struct scanners_start_single_cmd {
    uint8_t     opcode;
    uint8_t     qtype;
    __le16      lif_index;
    __le32      index;
    uint8_t     rsvd2[56];
} scanners_start_single_cmd_t;

/**
 * scanners_start_single_cpl_t - HW scanners start single queue complete command
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @qtype:      queue type
 * @lif_index:  software lif index
 */
typedef struct scanners_start_single_cpl {
    uint8_t     status;
    uint8_t     qtype;
    __le16      cpl_index;
    uint8_t     rsvd2[12];
} scanners_start_single_cpl_t;

/**
 * mpu_timestamp_init_cmd_t - HW MPU timestamp queue init command
 * @opcode:     opcode
 * @qtype:      queue type (MPU timestamp)
 * @lif_index:  software lif index
 * @pid:        Process ID
 * @cos:        Class of service for the queues.
 * @cos_override: override FW selected cos with cmd cos
 */
typedef struct mpu_timestamp_init_cmd {
    uint8_t     opcode;
    uint8_t     qtype;
    __le16      lif_index;
    __le16      pid;
    uint8_t     rsvd1;
    uint8_t     cos;
    uint8_t     cos_override;
    uint8_t     rsvd2[55];
} mpu_timestamp_init_cmd_t;

/**
 * mpu_timestamp_init_cmd_t - HW MPU timestamp queue init command completion
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @qtype:      queue type
 */
typedef struct mpu_timestamp_init_cpl {
    uint8_t     status;
    uint8_t     qtype;
    uint8_t     rsvd[14];
} mpu_timestamp_init_cpl_t;

/**
 * HW MPU timestamp start/stop
 */
typedef scanners_start_cmd_t    mpu_timestamp_start_cmd_t;
typedef scanners_start_cpl_t    mpu_timestamp_start_cpl_t;
typedef scanners_stop_cmd_t     mpu_timestamp_stop_cmd_t;
typedef scanners_stop_cpl_t     mpu_timestamp_stop_cpl_t;

/**
 * struct accel_aging_ctl_cmd_t - LIF accelerated aging control command
 * @opcode: opcode
 * @lif_index: software lif index
 * @enable_sense: set to true to enable accelerated aging
 */
typedef struct accel_aging_ctl_cmd {
    uint8_t     opcode;
    uint8_t     rsvd1;
    __le16      lif_index;
    uint8_t     enable_sense;
    uint8_t     rsvd2[59];
} accel_aging_ctl_cmd_t;

typedef struct admin_cpl accel_aging_ctl_cpl_t;

/*
 * Union of all commands
 */
typedef union ftl_devcmd {
    __le32                        words[16];
    admin_cmd_t                   cmd;
    nop_cmd_t                     nop;
    dev_reset_cmd_t               dev_reset;
    dev_identify_cmd_t            dev_identify;
    lif_identify_cmd_t            lif_identify;
    lif_init_cmd_t                lif_init;
    lif_reset_cmd_t               lif_reset;
    lif_getattr_cmd_t             lif_getattr;
    lif_setattr_cmd_t             lif_setattr;
    pollers_init_cmd_t            pollers_init;
    pollers_flush_cmd_t           pollers_flush;
    pollers_deq_burst_cmd_t       pollers_deq_burst;
    scanners_init_cmd_t           scanners_init;
    scanners_start_cmd_t          scanners_start;
    scanners_stop_cmd_t           scanners_stop;
    scanners_start_single_cmd_t   scanners_start_single;
    mpu_timestamp_init_cmd_t      mpu_timestamp_init;
    mpu_timestamp_start_cmd_t     mpu_timestamp_start;
    mpu_timestamp_stop_cmd_t      mpu_timestamp_stop;
    accel_aging_ctl_cmd_t         accel_aging_ctl;
} ftl_devcmd_t;

typedef union ftl_devcmd_cpl {
    __le32                        words[4];
    uint8_t                       status;
    admin_cpl_t                   cpl;
    nop_cpl_t                     nop;
    dev_reset_cpl_t               dev_reset;
    dev_identify_cpl_t            dev_identify;
    lif_identify_cpl_t            lif_identify;
    lif_init_cpl_t                lif_init;
    lif_reset_cpl_t               lif_reset;
    lif_getattr_cpl_t             lif_getattr;
    lif_setattr_cpl_t             lif_setattr;
    pollers_init_cpl_t            pollers_init;
    pollers_flush_cpl_t           pollers_flush;
    pollers_deq_burst_cpl_t       pollers_deq_burst;
    scanners_init_cpl_t           scanners_init;
    scanners_start_cpl_t          scanners_start;
    scanners_stop_cpl_t           scanners_stop;
    scanners_start_single_cpl_t   scanners_start_single;
    mpu_timestamp_init_cpl_t      mpu_timestamp_init;
    mpu_timestamp_start_cpl_t     mpu_timestamp_start;
    mpu_timestamp_stop_cpl_t      mpu_timestamp_stop;
    accel_aging_ctl_cpl_t         accel_aging_ctl;
} ftl_devcmd_cpl_t;

#pragma pack(pop)

} // namespace ftl_dev_if

#endif /* _FTL_DEV_IF_H_ */
