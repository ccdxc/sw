/*
 * Copyright 2018-2019 Pensando Systems, Inc.  All rights reserved.
 *
 * This program is free software; you may redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef _ACCEL_DEV_IF_H_
#define _ACCEL_DEV_IF_H_

#ifdef __FreeBSD__
#include <linux/types.h>
#endif

#if !defined(_LINUX_TYPES_H) && !defined(_LINUX_TYPES_H_)
typedef uint16_t __le16;
typedef uint32_t __le32;
typedef uint64_t __le64;
#endif

#define ACCEL_DEV_IFNAMSIZ		16
#define ACCEL_DEV_STRINGIFY(x)          #x
#define ACCEL_DEV_INDEX_STRINGIFY(x)    [x] = ACCEL_DEV_STRINGIFY(x)
#define ACCEL_DEV_CASE_STRINGIFY(x)     case x: return ACCEL_DEV_STRINGIFY(x)

/**
 * devcmd HBM handle as defined in nic/conf/iris/hbm_mem.json
 */
#define ACCEL_DEVCMD_HBM_HANDLE         "storage_devcmd"

#define ACCEL_DEV_CMD_SIGNATURE         0x41434345      /* 'ACCE' */

enum cmd_opcode {
	CMD_OPCODE_NOP			   = 0,
	CMD_OPCODE_RESET		   = 1,
	CMD_OPCODE_IDENTIFY		   = 2,
	CMD_OPCODE_LIF_IDENTIFY		   = 3,
	CMD_OPCODE_LIF_INIT		   = 4,
	CMD_OPCODE_LIF_RESET    	   = 5,
	CMD_OPCODE_LIF_GETATTR		   = 6,
	CMD_OPCODE_LIF_SETATTR		   = 7,
	CMD_OPCODE_ADMINQ_INIT		   = 8,
	CMD_OPCODE_NOTIFYQ_INIT            = 9,
	CMD_OPCODE_SEQ_QUEUE_INIT	   = 10,
	CMD_OPCODE_SEQ_QUEUE_ENABLE	   = 11,
	CMD_OPCODE_SEQ_QUEUE_DISABLE       = 12,
	CMD_OPCODE_SEQ_QUEUE_BATCH_INIT	   = 13,
	CMD_OPCODE_SEQ_QUEUE_BATCH_ENABLE  = 14,
	CMD_OPCODE_SEQ_QUEUE_BATCH_DISABLE = 15,
	CMD_OPCODE_SEQ_QUEUE_INIT_COMPLETE = 16,
	CMD_OPCODE_CRYPTO_KEY_UPDATE       = 17,
	CMD_OPCODE_HANG_NOTIFY		   = 18,
};

#define ACCEL_DEVCMD_OPCODE_CASE_TABLE                                  \
    ACCEL_DEV_CASE_STRINGIFY(CMD_OPCODE_NOP);                           \
    ACCEL_DEV_CASE_STRINGIFY(CMD_OPCODE_RESET);                         \
    ACCEL_DEV_CASE_STRINGIFY(CMD_OPCODE_IDENTIFY);                      \
    ACCEL_DEV_CASE_STRINGIFY(CMD_OPCODE_LIF_IDENTIFY);                  \
    ACCEL_DEV_CASE_STRINGIFY(CMD_OPCODE_LIF_INIT);                      \
    ACCEL_DEV_CASE_STRINGIFY(CMD_OPCODE_LIF_RESET);                     \
    ACCEL_DEV_CASE_STRINGIFY(CMD_OPCODE_LIF_GETATTR);                   \
    ACCEL_DEV_CASE_STRINGIFY(CMD_OPCODE_LIF_SETATTR);                   \
    ACCEL_DEV_CASE_STRINGIFY(CMD_OPCODE_ADMINQ_INIT);                   \
    ACCEL_DEV_CASE_STRINGIFY(CMD_OPCODE_NOTIFYQ_INIT);                  \
    ACCEL_DEV_CASE_STRINGIFY(CMD_OPCODE_SEQ_QUEUE_INIT);                \
    ACCEL_DEV_CASE_STRINGIFY(CMD_OPCODE_SEQ_QUEUE_ENABLE);              \
    ACCEL_DEV_CASE_STRINGIFY(CMD_OPCODE_SEQ_QUEUE_DISABLE);             \
    ACCEL_DEV_CASE_STRINGIFY(CMD_OPCODE_SEQ_QUEUE_BATCH_INIT);          \
    ACCEL_DEV_CASE_STRINGIFY(CMD_OPCODE_SEQ_QUEUE_BATCH_ENABLE);        \
    ACCEL_DEV_CASE_STRINGIFY(CMD_OPCODE_SEQ_QUEUE_BATCH_DISABLE);       \
    ACCEL_DEV_CASE_STRINGIFY(CMD_OPCODE_SEQ_QUEUE_INIT_COMPLETE);       \
    ACCEL_DEV_CASE_STRINGIFY(CMD_OPCODE_CRYPTO_KEY_UPDATE);             \
    ACCEL_DEV_CASE_STRINGIFY(CMD_OPCODE_HANG_NOTIFY);                   \
    
enum accel_status_code {
	ACCEL_RC_SUCCESS	= 0,	/* Success */
	ACCEL_RC_EVERSION	= 1,	/* Incorrect version for request */
	ACCEL_RC_EOPCODE	= 2,	/* Invalid cmd opcode */
	ACCEL_RC_EIO		= 3,	/* I/O error */
	ACCEL_RC_EPERM		= 4,	/* Permission denied */
	ACCEL_RC_EQID		= 5, 	/* Bad qid */
	ACCEL_RC_EQTYPE		= 6, 	/* Bad qtype */
	ACCEL_RC_ENOENT		= 7,	/* No such element */
	ACCEL_RC_EINTR		= 8,	/* operation interrupted */
	ACCEL_RC_EAGAIN		= 9,	/* Try again */
	ACCEL_RC_ENOMEM		= 10,	/* Out of memory */
	ACCEL_RC_EFAULT		= 11,	/* Bad address */
	ACCEL_RC_EBUSY		= 12,	/* Device or resource busy */
	ACCEL_RC_EEXIST		= 13,	/* object already exists */
	ACCEL_RC_EINVAL		= 14,	/* Invalid argument */
	ACCEL_RC_ENOSPC		= 15,	/* No space left or alloc failure */
	ACCEL_RC_ERANGE		= 16,	/* Parameter out of range */
	ACCEL_RC_BAD_ADDR	= 17,	/* Descriptor contains a bad ptr */
	ACCEL_RC_DEV_CMD	= 18,	/* Device cmd attempted on AdminQ */
	ACCEL_RC_ERROR		= 19,	/* Generic error */
};

#define ACCEL_RC_STR_TABLE                              \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_RC_SUCCESS),        \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_RC_EVERSION),       \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_RC_EOPCODE),        \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_RC_EIO),            \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_RC_EPERM),          \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_RC_EQID),           \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_RC_EQTYPE),         \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_RC_ENOENT),         \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_RC_EINTR),          \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_RC_EAGAIN),         \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_RC_ENOMEM),         \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_RC_EFAULT),         \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_RC_EBUSY),          \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_RC_EEXIST),         \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_RC_EINVAL),         \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_RC_ENOSPC),         \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_RC_ERANGE),         \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_RC_BAD_ADDR),       \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_RC_DEV_CMD),        \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_RC_ERROR),          \

typedef int accel_status_code_t;

enum notifyq_opcode {
	EVENT_OPCODE_LINK_CHANGE    = 1,
	EVENT_OPCODE_RESET          = 2,
	EVENT_OPCODE_HEARTBEAT      = 3,
	EVENT_OPCODE_LOG            = 4,
	EVENT_OPCODE_RING_DESC_DATA = 5,
	EVENT_OPCODE_RING_DESC_ADDR = 6,
};

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
 * @color:      Color bit.  (Always 0 for commands issued to the
 *              Device Cmd Registers.)
 */
typedef struct admin_cpl {
	uint8_t     status;
	uint8_t     rsvd;
	__le16      cpl_index;
	uint8_t     cmd_data[11];
	uint8_t     color;
} admin_cpl_t;

#define ACCEL_CPL_COLOR_MASK  0x80

static inline uint8_t color_match(uint8_t color,
				  uint8_t done_color)
{
	return (!!(color & ACCEL_CPL_COLOR_MASK)) == done_color;
}

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

enum accel_dev_type {
	ACCEL_DEV_TYPE_BASE = 0,
	ACCEL_DEV_TYPE_MAX,
};

enum accel_logical_qtype {
	ACCEL_LOGICAL_QTYPE_SQ      = STORAGE_SEQ_QTYPE_SQ,
	ACCEL_LOGICAL_QTYPE_NOTIFYQ = STORAGE_SEQ_QTYPE_NOTIFY,
	ACCEL_LOGICAL_QTYPE_ADMINQ  = STORAGE_SEQ_QTYPE_ADMIN,
	ACCEL_LOGICAL_QTYPE_MAX     = 8,
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

#define IDENTITY_VERSION_1		1

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

enum accel_os_type {
	OS_TYPE_LINUX   = 1,
	OS_TYPE_WIN     = 2,
	OS_TYPE_DPDK    = 3,
	OS_TYPE_FREEBSD = 4,
	OS_TYPE_IXPE    = 5,
	OS_TYPE_ESXI    = 6,
};

enum accel_lif_type {
	ACCEL_LIF_TYPE_BASE = 0,
	ACCEL_LIF_TYPE_MAX,
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
 * union drv_identity - driver identity information
 * @os_type:          OS type (see enum os_type)
 * @os_dist:          OS distribution, numeric format
 * @os_dist_str:      OS distribution, string format
 * @kernel_ver:       Kernel version, numeric format
 * @kernel_ver_str:   Kernel version, string format
 * @driver_ver_str:   Driver version, string format
 */
typedef union drv_identity {
	struct {
		__le32          os_type;
		__le32          os_dist;
		char            os_dist_str[128];
		__le32          kernel_ver;
		char            kernel_ver_str[32];
		char            driver_ver_str[32];
	};
	__le32                  words[512];
} drv_identity_t;

/**
 * union dev_identity - device identity information
 * @nlifs:            Number of LIFs provisioned
 * @nintrs:           Number of interrupts provisioned
 * @ndbpgs_per_lif:   Number of doorbell pages per LIF
 * @intr_coal_mult:   Interrupt coalescing multiplication factor.
 *                    Scale user-supplied interrupt coalescing
 *                    value in usecs to device units using:
 *                    device units = usecs * mult / div
 * @intr_coal_div:    Interrupt coalescing division factor.
 *                    Scale user-supplied interrupt coalescing
 *                    value in usecs to device units using:
 *                    device units = usecs * mult / div
 * @intr_assert_stride: Interrupt assertion stride value
 * @intr_assert_addr: Interrupt assertion data
 * @intr_assert_addr: Interrupt assertion address
 * @cm_base_pa:       Controller memory base physical address
 */
typedef union dev_identity {
	struct {
		uint8_t         version;
		uint8_t         type;
		uint8_t         rsvd1[2];
		uint8_t         rsvd2[4];
		__le32          nlifs;
		__le32          nintrs;
		__le32          ndbpgs_per_lif;
		__le32          intr_coal_mult;
		__le32          intr_coal_div;
		__le32          intr_assert_stride;
		__le32          intr_assert_data;
		__le64          intr_assert_addr;
		__le64          cm_base_pa;
	} base;
	__le32 words[512];
} dev_identity_t;

/**
 * LIF configuration
 * @name:           lif name
 */
typedef union lif_config {
	struct {
		char            name[ACCEL_DEV_IFNAMSIZ];
	};
	__le32 words[128];
} lif_config_t;

/**
 * struct lif_identity - lif identity information
 *
 * @hw_index: lif hardware index
 * @hw_lif_local_dbaddr: LIF local doorbell address
 * @hw_host_prefix: prefix for making a host address
 * @hw_host_mask: mask for making a host address
 * @crypto_key_idx_base: crypto key index base
 * @num_crypto_keys_max: max # of crypto keys supported
 * @queue_count: queue counts per queue-type
 * @accel_ring_tbl: accel ring table
 */
typedef union lif_identity {
	struct {
		uint8_t         version;
		uint8_t         rsvd;
		__le16          hw_index;
		__le64          hw_lif_local_dbaddr;
		__le64          hw_host_prefix;
		__le64          hw_host_mask;
		__le32          crypto_key_idx_base;
		__le32          num_crypto_keys_max;
		__le32          queue_count[ACCEL_LOGICAL_QTYPE_MAX];
		accel_ring_t    accel_ring_tbl[ACCEL_RING_ID_MAX];
		lif_config_t    config;
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
 */
typedef struct lif_reset_cmd {
	uint8_t     opcode;
	uint8_t     rsvd1;
	__le16      lif_index;
	uint8_t     rsvd2[60];
} lif_reset_cmd_t;

typedef struct admin_cpl lif_reset_cpl_t;

/**
 * enum lif_attr - List of LIF attributes
 */
enum lif_attr {
	ACCEL_LIF_ATTR_NAME         = 0,
};

/**
 * lif_setattr_cmd_t - Set LIF attributes on the NIC
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
		char            name[ACCEL_DEV_IFNAMSIZ];
		uint8_t         rsvd[60];
	};
} lif_setattr_cmd_t;

typedef struct lif_setattr_cpl {
	uint8_t     status;
	uint8_t     rsvd1;
	__le16      cpl_index;
	union {
		uint8_t         rsvd2[12];
	};
} lif_setattr_cpl_t;

/**
 * struct lif_getattr_cmd - Get LIF attributes from the NIC
 * @lif_index:  software lif index
 * @attr:       Attribute type (enum lif_attr)
 */
typedef struct lif_getattr_cmd {
	uint8_t     opcode;
	uint8_t     attr;
	__le16      lif_index;
	uint8_t     rsvd[60];
} lif_getattr_cmd_t;

typedef struct lif_getattr_cpl {
	uint8_t     status;
	uint8_t     rsvd1;
	__le16      cpl_index;
	union {
		// char    name[ACCEL_DEV_IFNAMSIZ];
		uint8_t         rsvd2[12];
	};
} lif_getattr_cpl_t;

/**
 * adminq_init_cmd_t - Admin queue init command
 * @opcode:       opcode
 * @lif_index:    software lif index
 * @index:        LIF-relative admin queue index
 * @pid:          Process ID
 * @intr_index:   Interrupt control register index
 * @ring_base:    Queue ring base address
 * @cq_ring_base: Completion queue ring base address
 * @ring_size:    Admin queue ring size, encoded as a log2(size),
 *                in number of descs.  The actual ring size is
 *                (1 << ring_size).  For example, to
 *                select a ring size of 64 descriptors write
 *                ring_size = 6.  The minimum ring_size value is 2
 *                for a ring size of 4 descriptors.  The maximum
 *                ring_size value is 16 for a ring size of 64k
 *                descriptors.  Values of ring_size <2 and >16 are
 *                reserved.
 */
typedef struct adminq_init_cmd {
	uint8_t     opcode;
	uint8_t     rsvd1;
	__le16      lif_index;
	__le32      index;
	__le16      pid;
	__le16      intr_index;
	__le64      ring_base;
	__le64      cq_ring_base;
	uint8_t     ring_size;
	uint8_t     rsvd[35];
} adminq_init_cmd_t;

/**
 * adminq_init_cpl_t - Admin queue init command completion
 * @status:  The status of the command.  Values for status are:
 *              0 = Successful completion
 * @qtype:   Queue type
 * @qid:     Queue ID
 */
typedef struct adminq_init_cpl {
	uint8_t     status;
	uint8_t     qtype;
	uint8_t     rsvd1[2];
	__le32      qid;
	uint8_t     rsvd2[8];
} adminq_init_cpl_t;

/**
 * struct notifyq_init_cmd - Event queue init command
 * @opcode:       opcode
 * @lif_index:    software lif index
 * @index:        LIF-relative queue index
 * @pid:          Process ID
 * @intr_index:   Interrupt control register index
 * @ring_base:    Queue ring base address. Should be aligned
 *                on PAGE_SIZE. If not aligned properly can cause
 *                CQ Errors
 * @cq_ring_base: Completion queue ring base address
 * @notify_base:  For future use
 * @ring_size:    NotifyQ queue ring size, encoded as a log2(size),
 *                in number of descs.  The actual ring size is
 *                (1 << ring_size).  For example, to
 *                select a ring size of 64 descriptors write
 *                ring_size = 6.  The minimum ring_size value is 2
 *                for a ring size of 4 descriptors.  The maximum
 *                ring_size value is 16 for a ring size of 64k
 *                descriptors.  Values of ring_size <2 and >16 are
 *                reserved.
 * @notify_size:  Notify block size, encoded as a log2(size), in
 *                number of bytes.  If the size is smaller that the
 *                data available, the data will be truncated.
 */
typedef struct notifyq_init_cmd {
	uint8_t     opcode;
	uint8_t     rsvd1;
	__le16      lif_index;
	__le32      index;
	__le16      pid;
	__le16      intr_index;
	__le64      ring_base;
	__le64      notify_base;
	__le64      cq_ring_base;
	uint8_t     ring_size;
	uint8_t     notify_size;
	uint8_t     rsvd2[26];
} notifyq_init_cmd_t;

/**
 * notifyq_init_cpl_t - Event queue init command completion
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @cpl_index: The index in the descriptor ring for which this
 *              is the completion.
 * @qid:        Queue ID
 * @qtype:      Queue type
 * @color:      Color bit.
 */
typedef struct notifyq_init_cpl {
	uint8_t     status;
	uint8_t     qtype;
	uint8_t     color;
	uint8_t     rsvd1;
	__le32      qid;
	__le16      cpl_index;
	uint8_t     rsvd2[6];
} notifyq_init_cpl_t;

/**
 * seq_queue_init_cmd_t - Sequencer queue init command
 * @opcode:     opcode
 * @lif_index:    software lif index
 * @pid:        Process ID
 * @index:      LIF-relative Sequencer queue index
 * @cos:        Class of service for this queue.
 * @wring_size: Work ring size, encoded as a log2(size),
 *              in number of descriptors.  The actual ring size is
 *              (1 << wring_size) descriptors.  For example, to
 *              select a ring size of 64 descriptors write
 *              wring_size = 6.  The minimum wring_size value is 2
 *              for a ring size of 4 descriptors.  The maximum
 *              ring_size value is 16 for a wring size of 64k
 *              descriptors.  Values of wring_size <2 and >16 are
 *              reserved.
 * @wring_base: Work Queue ring base address.
 * @core_id:    informational CPU core_id
 * @cos_override: override FW selected cos with cmd cos
 */
typedef struct seq_queue_init_cmd {
	uint8_t     opcode;
	uint8_t     rsvd1;
	__le16      lif_index;
	__le32      index;
	__le16      pid;
	uint8_t     enable;
	uint8_t     cos;
	uint8_t     total_wrings;
	uint8_t     host_wrings;
	uint8_t     entry_size;
	uint8_t     wring_size;
	__le64      wring_base;
	__le16      core_id;
	storage_seq_qgroup_t qgroup;
	uint8_t     cos_override;
	uint8_t     rsvd2[36];
} seq_queue_init_cmd_t;

/**
 * seq_queue_init_cpl_t - Sequencer queue init command completion
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @cpl_index:  The index in the descriptor ring for which this
 *              is the completion.
 * @qid:        Queue ID
 * @qtype:      Queue type
 */
typedef struct seq_queue_init_cpl {
	uint8_t     status;
	uint8_t     qtype;
	uint8_t     rsvd1[2];
	__le32      qid;
	__le16      cpl_index;
	uint8_t     rsvd2[6];
} seq_queue_init_cpl_t;

/**
 * seq_queue_control_cmd_t - Sequencer Queue control command
 * @opcode:     opcode
 * @lif_index:  software lif index
 * @index:      LIF-relative Sequencer queue index
 * @qtype:      Queue type
 */
typedef struct seq_queue_control_cmd {
	uint8_t     opcode;
	uint8_t     rsvd1;
        __le16      lif_index;
	__le32      index;
	uint8_t     qtype;
	uint8_t     rsvd2[55];
} seq_queue_control_cmd_t;

typedef admin_cpl_t seq_queue_control_cpl_t;

/**
 * seq_queue_batch_init_cmd_t - Sequencer queue batch init command
 * @opcode:     opcode
 * @lif_index:  software lif index
 * @index:      LIF-relative Sequencer starting queue index
 * @pid:        Process ID
 * @num_queues: number of queues to initialize
 * @cos:        Class of service for the queues.
 * @wring_size: Work ring size, encoded as a log2(size),
 *              in number of descriptors.
 * @wring_base: Work Queue ring starting base address.
 * @cos_override: override FW selected cos with cmd cos
 */
typedef struct seq_queue_batch_init_cmd {
	uint8_t     opcode;
	uint8_t     rsvd1;
	__le16      lif_index;
	__le32      index;
	__le16      pid;
	uint8_t     enable;
	uint8_t     cos;
	uint8_t     total_wrings;
	uint8_t     host_wrings;
	uint8_t     entry_size;
	uint8_t     wring_size;
	__le64      wring_base;
	__le32      num_queues;
	storage_seq_qgroup_t qgroup;
	uint8_t     cos_override;
	uint8_t     rsvd2[34];
} seq_queue_batch_init_cmd_t;

/**
 * seq_queue_batch_init_cpl_t - Sequencer queue batch init command completion
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @cpl_index:  The index in the descriptor ring for which this
 *              is the completion.
 */
typedef struct seq_queue_batch_init_cpl {
	uint8_t     status;
	uint8_t     rsvd1;
	__le16      cpl_index;
	uint8_t     rsvd2[12];
} seq_queue_batch_init_cpl_t;

/**
 * seq_queue_batch_control_cmd_t - Sequencer Queue batch control command
 * @opcode:     opcode
 * @lif_index:  software lif index
 * @index:      LIF-relative Sequencer starting queue index
 * @num_queues: Number of queues in batch
 * @qtype:      Queue type
 */
typedef struct seq_queue_batch_control_cmd {
	uint8_t     opcode;
	uint8_t     rsvd1;
	__le16      lif_index;
	__le32      index;
	__le32      num_queues;
	uint8_t     qtype;
	uint8_t     rsvd2[51];
} seq_queue_batch_control_cmd_t;

typedef admin_cpl_t seq_queue_batch_control_cpl_t;

/**
 * seq_queue_init_complete_cmd_t - Sequencer queue init complete command
 * @opcode:     opcode
 * @lif_index:  software lif index
 */
typedef struct seq_queue_init_complete_cmd {
	uint8_t     opcode;
	uint8_t     rsvd1;
	__le16      lif_index;
	uint8_t     rsvd[60];
} seq_queue_init_complete_cmd_t;

/**
 * seq_queue_init_complete_cpl_t - Sequencer queue init complete command completion
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 */
typedef struct seq_queue_init_complete_cpl {
	uint8_t     status;
	uint8_t     rsvd[15];
} seq_queue_init_complete_cpl_t;

/**
 * crypto_key_update_cmd_t - Crypto key update command
 * @opcode:       opcode
 * @lif_index:    software lif index
 * @key_size:     key size in bytes
 * @key_index:    key index
 * @key_type:     key type
 * @key_data:     key data
 */

enum cmd_crypto_key_type {
	CMD_CRYPTO_KEY_TYPE_AES128   = 0,
	CMD_CRYPTO_KEY_TYPE_AES256   = 1,
	CMD_CRYPTO_KEY_TYPE_MAX      = 2
};

/*
 * Note: when a key exceeds the key_data size below (32 bytes), the update
 * can be broken up into multiple parts.
 */
enum cmd_crypto_key_part {
	CMD_CRYPTO_KEY_PART0         = 0,
	CMD_CRYPTO_KEY_PART1         = 1,
	CMD_CRYPTO_KEY_PART_MAX      = 2
};

#define CMD_CRYPTO_KEY_SIZE_AES128   16
#define CMD_CRYPTO_KEY_SIZE_AES256   32
#define CMD_CRYPTO_KEY_PART_SIZE     CMD_CRYPTO_KEY_SIZE_AES256

typedef struct crypto_key_update_cmd {
	uint8_t    opcode;
	uint8_t     rsvd1;
	__le16      lif_index;
	__le32      key_index;
	__le16      key_size;
	uint8_t     key_type;
	uint8_t     key_part;
	uint8_t     trigger_update;
	uint8_t     rsvd2[3];
	uint8_t     key_data[CMD_CRYPTO_KEY_PART_SIZE];
	uint8_t     rsvd3[16];
} crypto_key_update_cmd_t;

/**
 * crypto_key_update_cpl_t - Admin queue init command completion
 * @status:  The status of the command.  Values for status are:
 *              0 = Successful completion
 * @qid:     Queue ID
 * @qtype:   Queue type
 */
typedef struct crypto_key_update_cpl {
	uint8_t     status;
	uint8_t     rsvd[15];
} crypto_key_update_cpl_t;

/**
 * hang_notify_cmd_t - Hang notify command
 * @opcode:     opcode
 * @lif_index:  LIF index
 * @uncond_desc_notify:  send desc notifications unconditionally
 * @desc_notify_type:  type of desc notifications
 */
enum desc_notify_type {
	DESC_NOTIFY_TYPE_ADDR,
	DESC_NOTIFY_TYPE_DATA,
};

typedef struct hang_notify_cmd {
	uint8_t     opcode;
	uint8_t     rsvd1;
        __le16      lif_index;
	uint8_t     uncond_desc_notify;
	uint8_t     desc_notify_type;
	uint8_t     rsvd2[58];
} hang_notify_cmd_t;

/**
 * hang_notify_cpl_t - Hang notify command completion
 * @status: The status of the command.  Values for status are:
 *             0 = Successful completion
 */
typedef struct hang_notify_cpl {
	uint8_t     status;
	uint8_t     rsvd[15];
} hang_notify_cpl_t;

/**
 * notifyq_event_t
 * @eid:   event number
 * @ecode: event code
 * @data:  unspecified data about the event
 *
 * This is the generic event report struct from which the other
 * actual events will be formed.
 */
typedef struct notifyq_event {
	__le64      eid;
	uint8_t     ecode;
	uint8_t     rsvd;
	uint8_t     data[54];
} notifyq_event_t;

/**
 * enum reason_code - mask of reason codes
 */
enum reason_code {
	ACCEL_LIF_REASON_VOID			= 0,
	ACCEL_LIF_REASON_CP_ERR_RESET		= 1 << 0,
	ACCEL_LIF_REASON_DC_ERR_RESET		= 1 << 1,
	ACCEL_LIF_REASON_CP_ERR_LOG		= 1 << 2,
	ACCEL_LIF_REASON_DC_ERR_LOG		= 1 << 3,
	ACCEL_LIF_REASON_XTS_ENCR_ERR_RESET	= 1 << 4,
	ACCEL_LIF_REASON_XTS_DECR_ERR_RESET	= 1 << 5,
	ACCEL_LIF_REASON_XTS_ENCR_ERR_LOG	= 1 << 6,
	ACCEL_LIF_REASON_XTS_DECR_ERR_LOG	= 1 << 7,
	ACCEL_LIF_REASON_GCM_ENCR_ERR_RESET	= 1 << 8,
	ACCEL_LIF_REASON_GCM_DECR_ERR_RESET	= 1 << 9,
	ACCEL_LIF_REASON_GCM_ENCR_ERR_LOG	= 1 << 10,
	ACCEL_LIF_REASON_GCM_DECR_ERR_LOG	= 1 << 11,

	ACCEL_LIF_REASON_CPDC_ERR_RESET_MASK	= ACCEL_LIF_REASON_CP_ERR_RESET |
						  ACCEL_LIF_REASON_DC_ERR_RESET,
	ACCEL_LIF_REASON_CPDC_ERR_LOG_MASK	= ACCEL_LIF_REASON_CP_ERR_LOG |
						  ACCEL_LIF_REASON_DC_ERR_LOG,
	ACCEL_LIF_REASON_XTS_ERR_RESET_MASK	= ACCEL_LIF_REASON_XTS_ENCR_ERR_RESET |
						  ACCEL_LIF_REASON_XTS_DECR_ERR_RESET,
	ACCEL_LIF_REASON_XTS_ERR_LOG_MASK	= ACCEL_LIF_REASON_XTS_ENCR_ERR_LOG |
						  ACCEL_LIF_REASON_XTS_DECR_ERR_LOG,
	ACCEL_LIF_REASON_GCM_ERR_RESET_MASK	= ACCEL_LIF_REASON_GCM_ENCR_ERR_RESET |
						  ACCEL_LIF_REASON_GCM_DECR_ERR_RESET,
	ACCEL_LIF_REASON_GCM_ERR_LOG_MASK	= ACCEL_LIF_REASON_GCM_ENCR_ERR_LOG |
						  ACCEL_LIF_REASON_GCM_DECR_ERR_LOG,
	ACCEL_LIF_REASON_ALL_ERR_RESET_MASK	= ACCEL_LIF_REASON_CPDC_ERR_RESET_MASK |
						  ACCEL_LIF_REASON_XTS_ERR_RESET_MASK,
};

/**
 * struct reset_event
 * @eid:		event number
 * @ecode:		event code = EVENT_OPCODE_RESET
 * @reason_code:	see enum reason_code
 *
 * Sent when the NIC or some subsystem is going to be or
 * has been reset.
 */
typedef struct reset_event {
	__le64      eid;
	uint8_t     ecode;
	uint8_t     rsvd1;
	__le16      rsvd2;
	__le32      reason_code;
	uint8_t     rsvd3[48];
} reset_event_t;

/**
 * struct heartbeat_event
 * @eid:	event number
 * @ecode:	event code = EVENT_OPCODE_HEARTBEAT
 *
 * Sent periodically by the NIC to indicate continued health
 */
typedef struct heartbeat_event {
	__le64      eid;
	uint8_t     ecode;
	uint8_t     rsvd1;
	uint8_t     rsvd2[54];
} heartbeat_event_t;

/**
 * struct log_event
 * @eid:	event number
 * @ecode:	event code = EVENT_OPCODE_LOG
 * @data:	log data
 *
 * Sent to notify the driver of an internal error.
 */
typedef struct log_event {
	__le64      eid;
	uint8_t     ecode;
	uint8_t     rsvd;
	uint8_t     data[54];
} log_event_t;

/**
 * struct ring_desc_data_event
 * @eid:	event number
 * @ecode:	event code = EVENT_OPCODE_RING_DESC_DATA
 * @ring_id:	ring ID
 * @desc_idx:	descriptor ring index
 * @frag_offs:	fragment offset
 * @frag_size:	fragment size
 * @num_frags:	total number of fragments of descriptor data
 * @frag_data:	fragment data
 *
 * Sent to notify the driver of an (errored) ring descriptor.
 */
typedef struct ring_desc_data_event {
	__le64      eid;
	uint8_t     ecode;
	uint8_t     ring_id;
	__le16      desc_idx;
	__le16      frag_offs;
	uint8_t     frag_size;
	uint8_t     num_frags;
	uint8_t     frag_data[48];
} ring_desc_data_event_t;

/**
 * struct ring_desc_addr_event
 * @eid:	event number
 * @ecode:	event code = EVENT_OPCODE_RING_DESC_ADDR
 * @ring_id:	ring ID
 * @desc_idx:	descriptor ring index
 * @src_data_addr: source data address
 * @dst_data_addr: destination data address
 * @status_addr: fragment size
 * @db_addr:	doorbell address
 * @otag_addr:	opaque data address
 * @iv_addr:	initialization vector address
 *
 * Sent to notify the driver of an (errored) ring descriptor.
 */
typedef struct ring_desc_addr_event {
	__le64      eid;
	uint8_t     ecode;
	uint8_t     ring_id;
	__le16      desc_idx;
	__le64      src_addr;
	__le64      dst_addr;
	__le64      status_addr;
	__le64      db_addr;
	__le64      otag_addr;
	__le64      iv_addr;
	uint8_t     rsvd[4];
} ring_desc_addr_event_t;

typedef struct notifyq_cmd {
	__le32      data;       /* Not used but needed for qcq structure */
} notifyq_cmd_t;

typedef union adminq_cmd {
	admin_cmd_t                   cmd;
	nop_cmd_t                     nop;
	lif_reset_cmd_t               lif_reset;
	lif_setattr_cmd_t             lif_setattr;
	lif_getattr_cmd_t             lif_getattr;
	notifyq_init_cmd_t            notifyq_init;
	seq_queue_init_cmd_t          seq_queue_init;
	seq_queue_control_cmd_t       seq_queue_control;
	seq_queue_batch_init_cmd_t    seq_queue_batch_init;
	seq_queue_batch_control_cmd_t seq_queue_batch_control;
	seq_queue_init_complete_cmd_t seq_queue_init_complete;
	crypto_key_update_cmd_t       crypto_key_update;
	hang_notify_cmd_t             hang_notify;
} adminq_cmd_t;

typedef union adminq_cpl {
	uint8_t                       status;
	admin_cpl_t                   cpl;
	nop_cpl_t                     nop;
	lif_reset_cpl_t               lif_reset;
	lif_setattr_cpl_t             lif_setattr;
	lif_getattr_cpl_t             lif_getattr;
	notifyq_init_cpl_t            notifyq_init;
	seq_queue_init_cpl_t          seq_queue_init;
	seq_queue_control_cpl_t       seq_queue_control;
	seq_queue_batch_init_cpl_t    seq_queue_batch_init;
	seq_queue_batch_control_cpl_t seq_queue_batch_control;
	seq_queue_init_complete_cpl_t seq_queue_init_complete;
	crypto_key_update_cpl_t       crypto_key_update;
	hang_notify_cpl_t             hang_notify;
} adminq_cpl_t;

#define SONIC_BARS_MAX				6
#define SONIC_PCI_BAR_DBELL			1

/* BAR0 */
#define BAR0_SIZE				0x8000

#define BAR0_DEV_INFO_REGS_OFFSET		0x0000
#define BAR0_DEV_CMD_REGS_OFFSET		0x0800
#define BAR0_DEV_CMD_DATA_REGS_OFFSET		0x0c00
#define BAR0_INTR_STATUS_OFFSET			0x1000
#define BAR0_INTR_CTRL_OFFSET			0x2000
#define DEV_CMD_DONE				0x00000001

#define ASIC_TYPE_CAPRI				0

/**
 * struct doorbell - Doorbell register layout
 * @p_index: Producer index
 * @ring:    Selects the specific ring of the queue to update.
 *           Type-specific meaning:
 *              ring=0: Default producer/consumer queue.
 *              ring=1: (CQ, EQ) Re-Arm queue.  RDMA CQs
 *              send events to EQs when armed.  EQs send
 *              interrupts when armed.
 * @qid:     The queue id selects the queue destination for the
 *           producer index and flags.
 */
typedef struct doorbell {
	__le16      p_index;
	uint8_t     ring;
	uint8_t     qid_lo;
	__le16      qid_hi;
	__le16      rsvd2;
} doorbell_t;

/**
 * struct intr_ctrl - Interrupt control register
 * @coalescing_init:  Coalescing timer initial value, in
 *                    device units.  Use @identity->intr_coal_mult
 *                    and @identity->intr_coal_div to convert from
 *                    usecs to device units:
 *
 *                      coal_init = coal_usecs * coal_mutl / coal_div
 *
 *                    When an interrupt is sent the interrupt
 *                    coalescing timer current value
 *                    (@coalescing_curr) is initialized with this
 *                    value and begins counting down.  No more
 *                    interrupts are sent until the coalescing
 *                    timer reaches 0.  When @coalescing_init=0
 *                    interrupt coalescing is effectively disabled
 *                    and every interrupt assert results in an
 *                    interrupt.  Reset value: 0.
 * @mask:             Interrupt mask.  When @mask=1 the interrupt
 *                    resource will not send an interrupt.  When
 *                    @mask=0 the interrupt resource will send an
 *                    interrupt if an interrupt event is pending
 *                    or on the next interrupt assertion event.
 *                    Reset value: 1.
 * @int_credits:      Interrupt credits.  This register indicates
 *                    how many interrupt events the hardware has
 *                    sent.  When written by software this
 *                    register atomically decrements @int_credits
 *                    by the value written.  When @int_credits
 *                    becomes 0 then the "pending interrupt" bit
 *                    in the Interrupt Status register is cleared
 *                    by the hardware and any pending but unsent
 *                    interrupts are cleared.
 *                    !!!IMPORTANT!!! This is a signed register.
 * @flags:            Interrupt control flags
 *                       @unmask -- When this bit is written with a 1
 *                       the interrupt resource will set mask=0.
 *                       @coal_timer_reset -- When this
 *                       bit is written with a 1 the
 *                       @coalescing_curr will be reloaded with
 *                       @coalescing_init to reset the coalescing
 *                       timer.
 * @mask_on_assert:   Automatically mask on assertion.  When
 *                    @mask_on_assert=1 the interrupt resource
 *                    will set @mask=1 whenever an interrupt is
 *                    sent.  When using interrupts in Legacy
 *                    Interrupt mode the driver must select
 *                    @mask_on_assert=0 for proper interrupt
 *                    operation.
 * @coalescing_curr:  Coalescing timer current value, in
 *                    microseconds.  When this value reaches 0
 *                    the interrupt resource is again eligible to
 *                    send an interrupt.  If an interrupt event
 *                    is already pending when @coalescing_curr
 *                    reaches 0 the pending interrupt will be
 *                    sent, otherwise an interrupt will be sent
 *                    on the next interrupt assertion event.
 */
typedef struct intr_ctrl {
	uint8_t     coalescing_init;
	uint8_t     rsvd[3];
	uint8_t     mask;
	uint8_t     rsvd2[3];
	__le16      int_credits;
	__le16      flags;
#define INTR_F_UNMASK		0x0001
#define INTR_F_TIMER_RESET	0x0002
	uint8_t     mask_on_assert;
	uint8_t     rsvd3[3];
	uint8_t     coalescing_curr;
	uint8_t     rsvd4[3];
	__le32      rsvd6[3];
} intr_ctrl_t;

#define INTR_CTRL_REGS_MAX	2048
#define INTR_CTRL_COAL_MAX	0x3F

#define intr_to_coal(intr_ctrl)			(void *)((u8 *)(intr_ctrl) + 0)
#define intr_to_mask(intr_ctrl)			(void *)((u8 *)(intr_ctrl) + 4)
#define intr_to_credits(intr_ctrl)		(void *)((u8 *)(intr_ctrl) + 8)
#define intr_to_mask_on_assert(intr_ctrl)	(void *)((u8 *)(intr_ctrl) + 12)

typedef struct intr_status {
	__le32      status[2];
} intr_status_t;

/*
 * Note that any and all commands can be sent thru the dev_cmd area.
 */
typedef union dev_cmd {
	__le32                        words[16];
	admin_cmd_t                   cmd;
	nop_cmd_t                     nop;
	dev_reset_cmd_t               dev_reset;
	dev_identify_cmd_t            dev_identify;
	lif_identify_cmd_t            lif_identify;
	lif_init_cmd_t                lif_init;
	lif_reset_cmd_t               lif_reset;
	hang_notify_cmd_t             hang_notify;
	adminq_init_cmd_t             adminq_init;
	seq_queue_init_cmd_t          seq_q_init;
	seq_queue_init_complete_cmd_t seq_q_init_complete;
	seq_queue_batch_init_cmd_t    seq_q_batch_init;
	seq_queue_control_cmd_t       q_control;
	seq_queue_batch_control_cmd_t q_batch_control;
} dev_cmd_t;

typedef union dev_cmd_cpl {
	__le32                        words[4];
	uint8_t                       status;
	admin_cpl_t                   cpl;
	nop_cpl_t                     nop;
	dev_reset_cpl_t               dev_reset;
	dev_identify_cpl_t            dev_identify;
	lif_identify_cpl_t            lif_identify;
	lif_init_cpl_t                lif_init;
	lif_reset_cpl_t               lif_reset;
	hang_notify_cpl_t             hang_notify;
	adminq_init_cpl_t             adminq_init;
	seq_queue_init_cpl_t          seq_q_init;
	seq_queue_init_complete_cpl_t seq_q_init_complete;
	seq_queue_batch_init_cpl_t    seq_q_batch_init;
	seq_queue_control_cpl_t       q_control;
	seq_queue_batch_control_cpl_t q_batch_control;
} dev_cmd_cpl_t;

typedef union notifyq_cpl {
	notifyq_event_t         event;
	reset_event_t           reset;
	heartbeat_event_t       heartbeat;
	log_event_t             log;
        ring_desc_data_event_t  ring_desc_data;
        ring_desc_addr_event_t  ring_desc_addr;
} notifyq_cpl_t;

/**
 * union dev_info - Device info register format (read-only)
 * @signature:       Signature value of 0x44455649 ('DEVI').
 * @version:         Current version of info.
 * @asic_type:       Asic type.
 * @asic_rev:        Asic revision.
 * @fw_status:       Firmware status.
 * @fw_heartbeat:    Firmware heartbeat counter.
 * @serial_num:      Serial number.
 * @fw_version:      Firmware version.
 */
typedef struct dev_info_base {
	__le32      signature;
	uint8_t     version;
	uint8_t     asic_type;
	uint8_t     asic_rev;
	uint8_t     fw_status;
	__le32      fw_heartbeat;
	char        fw_version[32];
	char        serial_num[32];
} dev_info_base_t;

typedef union dev_info_regs {
	dev_info_base_t         base;
	__le32                  words[512];
} dev_info_regs_t;

/**
 * union dev_cmd_regs - Device command register format (read-write)
 * @doorbell:        Device Cmd Doorbell, write-only.
 *                   Write a 1 to signal device to process cmd,
 *                   poll done for completion.
 * @done:            Done indicator, bit 0 == 1 when command is complete.
 * @cmd:             Opcode-specific command bytes
 * @comp:            Opcode-specific response bytes
 * @data:            Opcode-specific side-data
 */
typedef union dev_cmd_regs {
	struct {
		__le32          doorbell;
		__le32          done;
		dev_cmd_t       cmd;
		dev_cmd_cpl_t   cpl;
		uint8_t         rsvd[48];
		__le32          data[478];
	};
	__le32                  words[512];
} dev_cmd_regs_t;

/**
 * union dev_regs - Device register format in for bar 0 page 0
 * @info:            Device info registers
 * @devcmd:          Device command registers
 */
typedef union dev_regs {
	struct {
		dev_info_regs_t info;
		dev_cmd_regs_t  devcmd;
	};
	__le32 words[1024];
} dev_regs_t;

typedef struct identity {
	drv_identity_t          drv;
	dev_identity_t          dev;
	lif_identity_t          lif;
	dev_info_base_t         info;
} identity_t;

#pragma pack(pop)

#endif /* _ACCEL_DEV_IF_H_ */
