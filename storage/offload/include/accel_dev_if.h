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

/**
 * devcmd HBM handle as defined in nic/conf/iris/hbm_mem.json
 */
#define ACCEL_DEVCMD_HBM_HANDLE         "storage_devcmd"

#define DEV_CMD_SIGNATURE               0x44455643      /* 'DEVC' */

enum cmd_opcode {
	CMD_OPCODE_NOP				= 0,
	CMD_OPCODE_RESET			= 1,
	CMD_OPCODE_IDENTIFY			= 2,
	CMD_OPCODE_LIF_INIT			= 3,
	CMD_OPCODE_ADMINQ_INIT		= 4,
	CMD_OPCODE_SEQ_QUEUE_INIT	= 5,
	CMD_OPCODE_SEQ_QUEUE_ENABLE	= 6,
	CMD_OPCODE_SEQ_QUEUE_DISABLE= 7,
	CMD_OPCODE_HANG_NOTIFY		= 8,

	CMD_OPCODE_SEQ_QUEUE_DUMP	= 0xf0,
};

#pragma pack(push, 1)

/**
 * admin_cmd_t - General admin command format
 * @opcode:   Opcode for the command
 * @cmd_data: Opcode-specific command bytes
 */
typedef struct admin_cmd {
	uint16_t    opcode;
	uint16_t    cmd_data[31];
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
	uint32_t    status      :8,
	            rsvd        :8,
	            cpl_index   :16;
	uint8_t     cmd_data[11];
	uint8_t     rsvd2:7;
	uint8_t     color:1;
} admin_cpl_t;

/**
 * nop_cmd_t - NOP command
 * @opcode: opcode = 0
 */
typedef struct nop_cmd {
	uint16_t    opcode;
	uint16_t    rsvd[31];
} nop_cmd_t;

/**
 * nop_cpl_t - NOP command completion
 * @status: The status of the command.  Values for status are:
 *             0 = Successful completion
 */
typedef struct nop_cpl {
	uint32_t    status  :8,
	            rsvd    :24;
	uint32_t    rsvd2[3];
} nop_cpl_t;

/**
 * reset_cmd_t - Device reset command
 * @opcode: opcode = 1
 */
typedef struct reset_cmd {
	uint16_t    opcode;
	uint16_t    rsvd[31];
} reset_cmd_t;

/**
 * reset_cpl_t - Reset command completion
 * @status: The status of the command.  Values for status are:
 *             0 = Successful completion
 */
typedef struct reset_cpl {
	uint32_t    status  :8,
	            rsvd    :24;
	uint32_t    rsvd2[3];
} reset_cpl_t;

#define IDENTITY_VERSION_1		1

/**
 * identify_cmd_t - Driver/device identify command
 * @opcode:  opcode = 2
 * @ver:     Highest version of identify supported by driver:
 *                 1 = version 1.0
 * @addr:    Destination address for the 4096-byte device
 *           identify info
 */
typedef struct identify_cmd {
	uint16_t    opcode;
	uint16_t    ver;
	dma_addr_t  addr;
	uint32_t    rsvd2[13];
} identify_cmd_t;

/**
 * identify_cpl_t - Driver/device identify command completion
 * @status: The status of the command.  Values for status are:
 *             0 = Successful completion
 *             1 = Version not supported by device
 * @ver:    Version of identify returned by device
 */
typedef struct identify_cpl {
	uint32_t    status  :8,
	            rsvd    :24;
	uint16_t    ver;
	uint16_t    rsvd2[5];
} identify_cpl_t;

enum os_type {
	OS_TYPE_LINUX   = 1,
	OS_TYPE_WIN     = 2,
	OS_TYPE_DPDK    = 3,
	OS_TYPE_FREEBSD = 4,
	OS_TYPE_IXPE    = 5,
};

/**
 * identity_t - 4096 bytes of driver/device identity information
 *
 * Supplied by driver (IN):
 *
 *     @os_type:          OS type (see enum os_type)
 *     @os_dist:          OS distribution, numeric format
 *     @os_dist_str:      OS distribution, string format
 *     @kernel_ver:       Kernel version, numeric format
 *     @kernel_ver_str:   Kernel version, string format
 *     @driver_ver_str:   Driver version, string format
 *
 * Return by device (OUT):
 *
 *     @asic_type:        ASIC type:
 *                           0 = Capri
 *     @asic_rev:         ASIC revision level, e.g. 0xA0
 *     @serial_num:       NULL-terminated string representing the
 *                        device serial number
 *     @fw_version:       NULL-terminated string representing the
 *                        firmware version
 *     @num_lifs:         Number of LIFs provisioned
 *     @db_pages_per_lif: Number of doorbell pages per LIF
 *     @admin_queues_per_lif: Number of admin queues per LIF provisioned
 *     @seq_queues_per_lif: Number of sequencer queues per LIF provisioned
 *     @num_intrs:        Number of interrupts provisioned
 *     @intr_coal_mult:   Interrupt coalescing multiplication factor.
 *                        Scale user-supplied interrupt coalescing
 *                        value in usecs to device units using:
 *                           device units = usecs * mult / div
 *     @intr_coal_div:    Interrupt coalescing division factor.
 *                        Scale user-supplied interrupt coalescing
 *                        value in usecs to device units using:
 *                           device units = usecs * mult / div
 *     @cm_base_pa:       Controller memory base physical address
 */
typedef union identity {
	struct {
		uint32_t        os_type;
		uint32_t        os_dist;
		char            os_dist_str[128];
		uint32_t        kernel_ver;
		char            kernel_ver_str[32];
		char            driver_ver_str[32];
	} drv;
	struct {
		uint8_t         asic_type;
		uint8_t         asic_rev;
		uint8_t         rsvd[2];
		char            serial_num[20];
		char            fw_version[20];
		uint32_t        num_lifs;
		uint32_t        db_pages_per_lif;
		uint32_t        admin_queues_per_lif;
		uint32_t        seq_queues_per_lif;
		uint32_t        num_intrs;
		uint32_t        intr_coal_mult;
		uint32_t        intr_coal_div;
		uint64_t        cm_base_pa;
		accel_ring_t    accel_ring_tbl[ACCEL_RING_ID_MAX];
		uint64_t        hw_lif_id_tbl[0];
	} dev;
	uint32_t words[1024];
} identity_t;

/**
 * lif_init_cmd_t - LIF init command
 * @opcode:    opcode = 3
 * @index:     LIF index
 */
typedef struct lif_init_cmd {
	uint16_t    opcode;
	uint16_t    rsvd;
	uint32_t    index   :24,
	            rsvd2   :8;
	uint32_t    rsvd3[14];
} lif_init_cmd_t;

/**
 * lif_init_cpl_t - LIF init command completion
 * @status: The status of the command.  Values for status are:
 *             0 = Successful completion
 */
typedef struct lif_init_cpl {
	uint32_t    status  :8,
	            rsvd    :24;
	uint32_t    rsvd2[3];
} lif_init_cpl_t;

/**
 * adminq_init_cmd_t - Admin queue init command
 * @opcode:       opcode = 4
 * @pid:          Process ID
 * @index:        LIF-relative admin queue index
 * @intr_index:   Interrupt control register index
 * @lif_index:    LIF index
 * @ring_size:    Admin queue ring size, encoded as a log2(size),
 *                in number of descs.  The actual ring size is
 *                (1 << ring_size).  For example, to
 *                select a ring size of 64 descriptors write
 *                ring_size = 6.  The minimum ring_size value is 2
 *                for a ring size of 4 descriptors.  The maximum
 *                ring_size value is 16 for a ring size of 64k
 *                descriptors.  Values of ring_size <2 and >16 are
 *                reserved.
 * @ring_base:    Admin queue ring base address
 */
typedef struct adminq_init_cmd {
	uint16_t    opcode;
	uint16_t    pid;
	uint16_t    index;
	uint16_t    intr_index;
	uint32_t    lif_index   :24,
	            ring_size   :8;
	dma_addr_t  ring_base;
	uint32_t    rsvd2[11];
} adminq_init_cmd_t;

/**
 * adminq_init_cpl_t - Admin queue init command completion
 * @status:  The status of the command.  Values for status are:
 *              0 = Successful completion
 * @qid:     Queue ID
 * @qtype:   Queue type
 */
typedef struct adminq_init_cpl {
	uint32_t    status  :8,
	            rsvd    :24;
	uint32_t    qid     :24,
	            qtype   :8;
	uint32_t    rsvd2[2];
} adminq_init_cpl_t;

enum txq_type {
	TXQ_TYPE_ETHERNET = 0,
};

/**
 * seq_queue_init_cmd_t - Sequencer queue init command
 * @opcode:     opcode = 5
 * @pid:        Process ID
 * @intr_index: Interrupt control register index
 * @type:       Select the transmit queue type.
 *                 0 = Ethernet
 *              All other values of @type are reserved.
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
 * @dol_req_devcmd_done: for DOL use only.
 */
typedef struct seq_queue_init_cmd {
	uint16_t                opcode;
	uint16_t                index;
	uint16_t                pid;
	storage_seq_qgroup_t    qgroup;
	uint8_t                 enable;
	uint8_t                 cos;
	uint8_t                 total_wrings;
	uint8_t                 host_wrings;
	uint8_t                 entry_size;
	uint8_t                 wring_size;
	dma_addr_t              wring_base;
	uint8_t                 dol_req_devcmd_done;
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
	uint32_t    status  :8,
	            rsvd    :8,
	            cpl_index:16;
	uint32_t    qid     :24,
	            qtype   :8;
	uint32_t    rsvd2;
	uint32_t    rsvd3;
} seq_queue_init_cpl_t;


/**
 * hang_notify_cmd_t - Hang notify command
 * @opcode:     opcode = 8
 */
typedef struct hang_notify_cmd {
	uint16_t    opcode;
	uint16_t    rsvd[31];
} hang_notify_cmd_t;

/**
 * hang_notify_cpl_t - Hang notify command completion
 * @status: The status of the command.  Values for status are:
 *             0 = Successful completion
 */
typedef struct hang_notify_cpl {
	uint32_t    status  :8,
	            rsvd    :24;
	uint32_t    rsvd2[3];
} hang_notify_cpl_t;

/**
 * seq_queue_control_cmd_t - Sequencer Queue control command
 * @opcode:     opcode = 9
 * @qid:        Queue ID
 * @qtype:      Queue type
 */
typedef struct seq_queue_control_cmd {
	uint16_t    opcode;
	uint16_t    rsvd;
	uint32_t    qid     :24,
	            qtype   :8;
	uint32_t    rsvd2[14];
} seq_queue_control_cmd_t;

typedef admin_cpl_t seq_queue_control_cpl_t;

/**
 * seq_queue_dump_cmd_t - Debug Sequencer queue dump command
 * @opcode:     opcode = 0xf0
 * @qid:        Queue ID
 * @qtype:      Queue type
 */
typedef struct seq_queue_dump_cmd {
	uint16_t    opcode;
	uint16_t    rsvd;
	uint32_t    qid     :24,
	            qtype   :8;
	uint32_t    rsvd2[14];
} seq_queue_dump_cmd_t;

/**
 * seq_queue_dump_cpl - Debug Sequencer queue dump command completion
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @cpl_index:  The index in the descriptor ring for which this
 *              is the completion.
 * @p_ndx:      Work ring 0 producer index
 * @c_ndx:      Work ring 0 consumer index
 * @color:      Color bit.
 */
typedef struct seq_queue_dump_cpl {
	uint32_t    status      :8,
	            rsvd        :8,
	            cpl_index   :16;
	uint16_t    p_ndx;
	uint16_t    c_ndx;
	uint32_t    rsvd2;
	uint32_t    rsvd3;
} seq_queue_dump_cpl_t;

#pragma pack(pop)

typedef union adminq_cmd {
	admin_cmd_t             cmd;
	nop_cmd_t               nop;
	seq_queue_init_cmd_t    seq_queue_init;
	seq_queue_control_cmd_t seq_queue_control;
	seq_queue_dump_cmd_t    seq_queue_dump;
} adminq_cmd_t;

typedef union adminq_cpl {
	admin_cpl_t             cpl;
	nop_cpl_t                nop;
	seq_queue_init_cpl_t    seq_queue_init;
	seq_queue_dump_cpl_t    seq_queue_dump;
} adminq_cpl_t;

#endif /* _ACCEL_DEV_IF_H_ */
