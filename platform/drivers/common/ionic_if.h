/*
 * Copyright 2017-2018 Pensando Systems, Inc.  All rights reserved.
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

#ifndef _IONIC_IF_H_
#define _IONIC_IF_H_

#define DEV_CMD_SIGNATURE               0x44455643      /* 'DEVC' */

enum cmd_opcode {
	CMD_OPCODE_NOP				= 0,
	CMD_OPCODE_RESET			= 1,
	CMD_OPCODE_IDENTIFY			= 2,
	CMD_OPCODE_LIF_INIT			= 3,
	CMD_OPCODE_ADMINQ_INIT			= 4,
	CMD_OPCODE_TXQ_INIT			= 5,
	CMD_OPCODE_RXQ_INIT			= 6,
	CMD_OPCODE_FEATURES			= 7,
	CMD_OPCODE_HANG_NOTIFY			= 8,

	CMD_OPCODE_Q_ENABLE			= 9,
	CMD_OPCODE_Q_DISABLE			= 10,

	CMD_OPCODE_NOTIFYQ_INIT			= 11,

	CMD_OPCODE_STATION_MAC_ADDR_GET		= 15,
	CMD_OPCODE_MTU_SET			= 16,
	CMD_OPCODE_RX_MODE_SET			= 17,
	CMD_OPCODE_RX_FILTER_ADD		= 18,
	CMD_OPCODE_RX_FILTER_DEL		= 19,
	CMD_OPCODE_STATS_DUMP_START		= 20,
	CMD_OPCODE_STATS_DUMP_STOP		= 21,
	CMD_OPCODE_RSS_HASH_SET			= 22,
	CMD_OPCODE_RSS_INDIR_SET		= 23,

	CMD_OPCODE_RDMA_RESET_LIF		= 50,
	CMD_OPCODE_RDMA_CREATE_EQ		= 51,
	CMD_OPCODE_RDMA_CREATE_CQ		= 52,
	CMD_OPCODE_RDMA_CREATE_ADMINQ		= 53,

	CMD_OPCODE_DEBUG_Q_DUMP			= 0xf0,
};

enum status_code {
	IONIC_RC_SUCCESS	= 0,	/* Success */
	IONIC_RC_EVERSION	= 1,	/* Incorrect version for request */
	IONIC_RC_EOPCODE	= 2,	/* Invalid cmd opcode */
	IONIC_RC_EIO		= 3,	/* I/O error */
	IONIC_RC_EPERM		= 4,	/* Permission denied */
	IONIC_RC_EQID		= 5, 	/* Bad qid */
	IONIC_RC_EQTYPE		= 6, 	/* Bad qtype */
	IONIC_RC_ENOENT		= 7,	/* No such element */
	IONIC_RC_EINTR		= 8,	/* operation interrupted */
	IONIC_RC_EAGAIN		= 9,	/* Try again */
	IONIC_RC_ENOMEM		= 10,	/* Out of memory */
	IONIC_RC_EFAULT		= 11,	/* Bad address */
	IONIC_RC_EBUSY		= 12,	/* Device or resource busy */
	IONIC_RC_EEXIST		= 13,	/* object already exists */
	IONIC_RC_EINVAL		= 14,	/* Invalid argument */
	IONIC_RC_ENOSPC		= 15,	/* No space left or alloc failure */
	IONIC_RC_ERANGE		= 16,	/* Parameter out of range */
	IONIC_RC_BAD_ADDR	= 17,	/* Descriptor contains a bad ptr */

	IONIC_RC_ERDMA		= 30,	/* Generic RDMA error */
};

enum notifyq_opcode {
	EVENT_OPCODE_LINK_CHANGE		= 1,
	EVENT_OPCODE_RESET			= 2,
	EVENT_OPCODE_HEARTBEAT			= 3,
	EVENT_OPCODE_LOG			= 4,
};

#pragma pack(push, 1)

/**
 * struct cmd - General admin command format
 * @opcode:   Opcode for the command
 * @cmd_data: Opcode-specific command bytes
 */
struct admin_cmd {
	u16 opcode;
	u16 cmd_data[31];
};

/**
 * struct admin_comp - General admin command completion format
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 *                 Non-zero = Error code.  Error codes are
 *                 command-specific.
 * @comp_index: The index in the descriptor ring for which this
 *              is the completion.
 * @cmd_data:   Command-specific bytes.
 * @color:      Color bit.  (Always 0 for commands issued to the
 *              Device Cmd Registers.)
 */
struct admin_comp {
	u32 status:8;
	u32 rsvd:8;
	u32 comp_index:16;
	u8 cmd_data[11];
	u8 rsvd2:7;
	u8 color:1;
};

/**
 * struct nop_cmd - NOP command
 * @opcode: opcode = 0
 */
struct nop_cmd {
	u16 opcode;
	u16 rsvd[31];
};

/**
 * struct nop_comp - NOP command completion
 * @status: The status of the command.  Values for status are:
 *             0 = Successful completion
 */
struct nop_comp {
	u32 status:8;
	u32 rsvd:24;
	u32 rsvd2[3];
};

/**
 * struct reset_cmd - Device reset command
 * @opcode: opcode = 1
 */
struct reset_cmd {
	u16 opcode;
	u16 rsvd[31];
};

/**
 * struct reset_comp - Reset command completion
 * @status: The status of the command.  Values for status are:
 *             0 = Successful completion
 */
struct reset_comp {
	u32 status:8;
	u32 rsvd:24;
	u32 rsvd2[3];
};

#define IDENTITY_VERSION_1		1

/**
 * struct identify_cmd - Driver/device identify command
 * @opcode:  opcode = 2
 * @ver:     Highest version of identify supported by driver:
 *                 1 = version 1.0
 * @addr:    Destination address for the 4096-byte device
 *           identify info
 */
struct identify_cmd {
	u16 opcode;
	u16 ver;
	dma_addr_t addr;
	u32 rsvd2[13];
};

/**
 * struct identify_comp - Driver/device identify command completion
 * @status: The status of the command.  Values for status are:
 *             0 = Successful completion
 *             1 = Version not supported by device
 * @ver:    Version of identify returned by device
 */
struct identify_comp {
	u32 status:8;
	u32 rsvd:24;
	u16 ver;
	u16 rsvd2[5];
};

enum os_type {
	OS_TYPE_LINUX   = 1,
	OS_TYPE_WIN     = 2,
	OS_TYPE_DPDK    = 3,
	OS_TYPE_FREEBSD = 4,
	OS_TYPE_IPXE    = 5,
	OS_TYPE_ESXI    = 6,
};

/**
 * struct lif_logical_qtype - Descriptor of logical to numeric queue type.
 * @ qtype:		Numeric Queue Type.
 * @ qid_base:		Minimum Queue ID of the logical type.
 * @ qid_count:		Number of Queue IDs of the logical type.
 */
struct lif_logical_qtype {
	u8 qtype;
	u8 rsvd[3];
	u32 qid_count;
	u32 qid_base;
};

/**
 * union identity - 4096 bytes of driver/device identity information
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
 *     @nlifs:            Number of LIFs provisioned
 *     @nintrs:           Number of interrupts provisioned
 *     @ndbpgs_per_lif:   Number of doorbell pages per LIF
 *     @nucasts_per_lif:  Number of perfect unicast addresses supported
 *     @nmcasts_per_lif:  Number of perfect multicast addresses supported.
 *     @intr_coal_mult:   Interrupt coalescing multiplication factor.
 *                        Scale user-supplied interrupt coalescing
 *                        value in usecs to device units using:
 *                           device units = usecs * mult / div
 *     @intr_coal_div:    Interrupt coalescing division factor.
 *                        Scale user-supplied interrupt coalescing
 *                        value in usecs to device units using:
 *                           device units = usecs * mult / div
 *     @rdma_version:     RDMA version of opcodes and queue descriptors.
 *     @rdma_qp_opcodes:  Number of rdma queue pair opcodes supported.
 *     @rdma_admin_opcodes: Number of rdma admin opcodes supported.
 *     @rdma_max_stride:  Max work request stride.
 *     @rdma_cl_stride:	  Cache line stride.
 *     @rdma_pte_stride:  Page table entry stride.
 *     @rdma_rrq_stride:  Remote RQ work request stride.
 *     @rdma_rsq_stride:  Remote SQ work request stride.
 *     @admin_qtype:      Admin Qtype.
 *     @tx_qtype:         Transmit Qtype.
 *     @rx_qtype:         Receive Qtype.
 *     @notify_qtype:     Notify Qtype.
 *     @rdma_aq_qtype:    RDMA Admin Qtype.
 *     @rdma_sq_qtype:    RDMA Send Qtype.
 *     @rdma_rq_qtype:    RDMA Receive Qtype.
 *     @rdma_cq_qtype:    RDMA Completion Qtype.
 *     @rdma_eq_qtype:    RDMA Event Qtype.
 */
union identity {
	struct {
		u32 os_type;
		u32 os_dist;
		char os_dist_str[128];
		u32 kernel_ver;
		char kernel_ver_str[32];
		char driver_ver_str[32];
	} drv;
	struct {
		u8 asic_type;
		u8 asic_rev;
		u8 rsvd_asicid[2];
		char serial_num[20];
		char fw_version[20];
		u32 nlifs;
		u32 nintrs;
		u32 ndbpgs_per_lif;
		u32 nucasts_per_lif;
		u32 nmcasts_per_lif;
		u32 intr_coal_mult;
		u32 intr_coal_div;
		u16 rdma_version;
		u8 rdma_qp_opcodes;
		u8 rdma_admin_opcodes;
		u32 nrdma_pts_per_lif;
		u32 nrdma_mrs_per_lif;
		u32 nrdma_ahs_per_lif;
		u8 rdma_max_stride;
		u8 rdma_cl_stride;
		u8 rdma_pte_stride;
		u8 rdma_rrq_stride;
		u8 rdma_rsq_stride;
		u8 rsvd_dimensions[11];
		struct lif_logical_qtype tx_qtype;
		struct lif_logical_qtype rx_qtype;
		struct lif_logical_qtype admin_qtype;
		struct lif_logical_qtype notify_qtype;
		struct lif_logical_qtype rdma_aq_qtype;
		struct lif_logical_qtype rdma_sq_qtype;
		struct lif_logical_qtype rdma_rq_qtype;
		struct lif_logical_qtype rdma_cq_qtype;
		struct lif_logical_qtype rdma_eq_qtype;
		struct lif_logical_qtype rsvd_qtype[7];
	} dev;
	u32 words[1024];
};

/**
 * struct lif_init_cmd - LIF init command
 * @opcode:    opcode = 3
 * @index:     LIF index
 */
struct lif_init_cmd {
	u16 opcode;
	u16 rsvd;
	u32 index:24;
	u32 rsvd2:8;
	u32 rsvd3[14];
};

/**
 * struct lif_init_comp - LIF init command completion
 * @status: The status of the command.  Values for status are:
 *             0 = Successful completion
 */
struct lif_init_comp {
	u32 status:8;
	u32 rsvd:24;
	u32 rsvd2[3];
};

/**
 * struct adminq_init_cmd - Admin queue init command
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
struct adminq_init_cmd {
	u16 opcode;
	u16 pid;
	u16 index;
	u16 intr_index;
	u32 lif_index:24;
	u32 ring_size:8;
	dma_addr_t ring_base;
	u32 rsvd2[11];
};

/**
 * struct adminq_init_comp - Admin queue init command completion
 * @status:  The status of the command.  Values for status are:
 *              0 = Successful completion
 * @qid:     Queue ID
 * @qtype:   Queue type
 */
struct adminq_init_comp {
	u32 status:8;
	u32 rsvd:24;
	u32 qid:24;
	u32 qtype:8;
	u32 rsvd2[2];
};

enum txq_type {
	TXQ_TYPE_ETHERNET = 0,
};

/**
 * struct txq_init_cmd - Transmit queue init command
 * @opcode:     opcode = 5
 * @I:          Interrupt requested on completion
 * @E:          Enable the queue.  If E=0 the queue is initialized
 *              but remains disabled, to be later enabled with the
 *              Queue Enable command.  If E=1, then queue is
 *              initialized and then enabled.
 * @pid:        Process ID
 * @intr_index: Interrupt control register index
 * @type:       Select the transmit queue type.
 *                 0 = Ethernet
 *              All other values of @type are reserved.
 * @index:      LIF-relative transmit queue index
 * @cos:        Class of service for this queue.
 * @ring_size:  Transmit queue ring size, encoded as a log2(size),
 *              in number of descriptors.  The actual ring size is
 *              (1 << ring_size) descriptors.  For example, to
 *              select a ring size of 64 descriptors write
 *              ring_size = 6.  The minimum ring_size value is 2
 *              for a ring size of 4 descriptors.  The maximum
 *              ring_size value is 16 for a ring size of 64k
 *              descriptors.  Values of ring_size <2 and >16 are
 *              reserved.
 * @ring_base:  Transmit Queue ring base address.
 */
struct txq_init_cmd {
	u16 opcode;
	u8 I:1;
	u8 E:1;
	u8 rsvd;
	u16 pid;
	u16 intr_index;
	u32 type:8;
	u32 index:16;
	u32 rsvd2:8;
	u32 cos:3;
	u32 ring_size:8;
	u32 rsvd3:21;
	dma_addr_t ring_base;
	u32 rsvd4[10];
};

/**
 * struct txq_init_comp - Tx queue init command completion
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @comp_index: The index in the descriptor ring for which this
 *              is the completion.
 * @qid:        Queue ID
 * @qtype:      Queue type
 * @color:      Color bit.
 */
struct txq_init_comp {
	u32 status:8;
	u32 rsvd:8;
	u32 comp_index:16;
	u32 qid:24;
	u32 qtype:8;
	u32 rsvd2;
	u32 rsvd3:31;
	u32 color:1;
};

enum txq_desc_opcode {
	TXQ_DESC_OPCODE_CALC_NO_CSUM = 0,
	TXQ_DESC_OPCODE_CALC_CSUM,
	TXQ_DESC_OPCODE_CALC_CSUM_TCPUDP,
	TXQ_DESC_OPCODE_TSO,
};

/**
 * struct txq_desc - Ethernet Tx queue descriptor format
 * @addr:         First data buffer's DMA address.
 *                (Subsequent data buffers are on txq_sg_desc).
 * @num_sg_elems: Number of scatter-gather elements in SG
 *                descriptor
 * @opcode:       Tx operation, see TXQ_DESC_OPCODE_*:
 *
 *                   TXQ_DESC_OPCODE_CALC_NO_CSUM:
 *
 *                      Non-offload send.  No segmentation,
 *                      fragmentation or checksum calc/insertion is
 *                      performed by device; packet is prepared
 *                      to send by software stack and requires
 *                      no further manipulation from device.
 *
 *                   TXQ_DESC_OPCODE_CALC_CSUM:
 *
 *                      Offload 16-bit L4 checksum
 *                      calculation/insertion.  The device will
 *                      calculate the L4 checksum value and
 *                      insert the result in the packet's L4
 *                      header checksum field.  The L4 checksum
 *                      is calculated starting at @hdr_len bytes
 *                      into the packet to the end of the packet.
 *                      The checksum insertion position is given
 *                      in @csum_offset.  This feature is only
 *                      applicable to protocols such as TCP, UDP
 *                      and ICMP where a standard (i.e. the
 *                      'IP-style' checksum) one's complement
 *                      16-bit checksum is used, using an IP
 *                      pseudo-header to seed the calculation.
 *                      Software will preload the L4 checksum
 *                      field with the IP pseudo-header checksum.
 *
 *                      For tunnel encapsulation, @hdr_len and
 *                      @csum_offset refer to the inner L4
 *                      header.  Supported tunnels encapsulations
 *                      are: IPIP, GRE, and UDP.  If the @O-bit
 *                      is clear, no further processing by the
 *                      device is required; software will
 *                      calculate the outer header checksums.  If
 *                      the @O-bit is set, the device will
 *                      offload the outer header checksums using
 *                      LCO (local checksum offload) (see
 *                      Documentation/networking/checksum-
 *                      offloads.txt for more info).
 *
 *                   TXQ_DESC_OPCODE_TSO:
 *
 *                      Device preforms TCP segmentation offload
 *                      (TSO).  @hdr_len is the number of bytes
 *                      to the end of TCP header (the offset to
 *                      the TCP payload).  @mss is the desired
 *                      MSS, the TCP payload length for each
 *                      segment.  The device will calculate/
 *                      insert IP (IPv4 only) and TCP checksums
 *                      for each segment.  In the first data
 *                      buffer containing the header template,
 *                      the driver will set IPv4 checksum to 0
 *                      and preload TCP checksum with the IP
 *                      pseudo header calculated with IP length =
 *                      0.
 *
 *                      Supported tunnel encapsulations are IPIP,
 *                      layer-3 GRE, and UDP. @hdr_len includes
 *                      both outer and inner headers.  The driver
 *                      will set IPv4 checksum to zero and
 *                      preload TCP checksum with IP pseudo
 *                      header on the inner header.
 *
 *                      TCP ECN offload is supported.  The device
 *                      will set CWR flag in the first segment if
 *                      CWR is set in the template header, and
 *                      clear CWR in remaining segments.
 *
 * @len:          First data buffer's length, in bytes
 * @vlan_tci:     VLAN tag to insert in the packet (if requested
 *                by @V-bit).  Includes .1p and .1q tags
 * @hdr_len:      Length of packet headers, including
 *                encapsulating outer header, if applicable.
 *                Valid for opcodes TXQ_DESC_OPCODE_CALC_CSUM and
 *                TXQ_DESC_OPCODE_TSO.  Should be set to zero for
 *                all other modes.  For
 *                TXQ_DESC_OPCODE_CALC_CSUM, @hdr_len is length
 *                of headers up to inner-most L4 header.  For
 *                TXQ_DESC_OPCODE_TSO, @hdr_len is up to
 *                inner-most L4 payload, so inclusive of
 *                inner-most L4 header.
 * @V:            Insert an L2 VLAN header using @vlan_tci.
 * @C:            Create a completion entry (CQ) for this packet.
 * @O:            Calculate outer-header checksum for GRE or UDP
 *                encapsulations.
 * @mss:          Desired MSS value for TSO.  Only applicable for
 *                TXQ_DESC_OPCODE_TSO.
 * @csum_offset:  Offset into inner-most L4 header of checksum
 *                field.  Only applicable for
 *                TXQ_DESC_OPCODE_CALC_CSUM.
 * @l3_csum:	  NIC populates L3 checksum even without csum_offset provided
 *                Valid for TXQ_DESC_OPCODE_CALC_CSUM.
 * @l4_csum:	  NIC populates L4 checksum even without csum_offset provided
 *                Valid for TXQ_DESC_OPCODE_CALC_CSUM.
 */
struct txq_desc {
	u64 addr:52;
	u64 rsvd:4;
	u64 num_sg_elems:5;
	u64 opcode:3;
	u16 len;
	u16 vlan_tci;
	u16 hdr_len:10;
	u16 rsvd2:3;
	u16 V:1;
	u16 C:1;
	u16 O:1;
	union {
		struct {
			u16 mss:14;
			u16 S:1;
			u16 E:1;
		};
		struct {
			u16 csum_offset:14;
			u8 l3_csum:1;
			u8 l4_csum:1;
		};
	};
};

#define IONIC_TX_MAX_SG_ELEMS	16

/** struct txq_sg_desc - Transmit scatter-gather (SG) list
 * @addr:      DMA address of SG element data buffer
 * @len:       Length of SG element data buffer, in bytes
 */
struct txq_sg_desc {
	struct txq_sg_elem {
		u64 addr:52;
		u64 rsvd:12;
		u16 len;
		u16 rsvd2[3];
	} elems[IONIC_TX_MAX_SG_ELEMS];
};

/** struct txq_comp - Ethernet transmit queue completion descriptor
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @comp_index: The index in the descriptor ring for which this
 *              is the completion.
 * @color:      Color bit.
 */
struct txq_comp {
	u32 status:8;
	u32 rsvd:8;
	u32 comp_index:16;
	u32 rsvd2[2];
	u32 rsvd3:31;
	u32 color:1;
};


enum rxq_type {
	RXQ_TYPE_ETHERNET = 0,
};

/**
 * struct rxq_init_cmd - Receive queue init command
 * @opcode:     opcode = 6
 * @I:          Interrupt requested on completion
 * @E:          Enable the queue.  If E=0 the queue is initialized
 *              but remains disabled, to be later enabled with the
 *              Queue Enable command.  If E=1, then queue is
 *              initialized and then enabled.
 * @pid:        Process ID
 * @intr_index: Interrupt control register index
 * @type:       Select the receive queue type.
 *                 0 = Ethernet
 *              All other values of @type are reserved.
 * @index:      LIF-relative receive queue index
 * @ring_size:  Transmit queue ring size, encoded as a log2(size),
 *              in number of descriptors.  The actual ring size is
 *              (1 << ring_size) descriptors.  For example, to
 *              select a ring size of 64 descriptors write
 *              ring_size = 6.  The minimum ring_size value is 2
 *              for a ring size of 4 descriptors.  The maximum
 *              ring_size value is 16 for a ring size of 64k
 *              descriptors.  Values of ring_size <2 and >16 are
 *              reserved.
 * @ring_base:  Transmit Queue ring base address.
 */
struct rxq_init_cmd {
	u16 opcode;
	u8 I:1;
	u8 E:1;
	u8 rsvd;
	u16 pid;
	u16 intr_index;
	u32 type:8;
	u32 index:16;
	u32 rsvd2:8;
	u32 ring_size:8;
	u32 rsvd3:24;
	dma_addr_t ring_base;
	u32 rsvd4[10];
};

/**
 * struct rxq_init_comp - Rx queue init command completion
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @comp_index: The index in the descriptor ring for which this
 *              is the completion.
 * @qid:        Queue ID
 * @qtype:      Queue type
 * @color:      Color bit.
 */
struct rxq_init_comp {
	u32 status:8;
	u32 rsvd:8;
	u32 comp_index:16;
	u32 qid:24;
	u32 qtype:8;
	u32 rsvd2;
	u32 rsvd3:31;
	u32 color:1;
};

enum rxq_desc_opcode {
	RXQ_DESC_OPCODE_NOP = 0,
	RXQ_DESC_OPCODE_SIMPLE,
};

/**
 * struct rxq_desc - Ethernet Rx queue descriptor format
 * @addr:         Data buffer's DMA address
 * @len:          Data buffer's length, in bytes.
 * @opcode:       Rx operation, see RXQ_DESC_OPCODE_*:
 *
 *                   RXQ_DESC_OPCODE_NOP:
 *
 *                      No packet received; used to pad out end
 *                      of queue (ring)
 *
 *                   RXQ_DESC_OPCODE_SIMPLE:
 *
 *                      Receive full packet into data buffer
 *                      starting at @addr.  Results of
 *                      receive, including actual bytes received,
 *                      are recorded in Rx completion descriptor.
 *
 */
struct rxq_desc {
	u64 addr:52;
	u64 rsvd:12;
	u16 len;
	u16 opcode:3;
	u16 rsvd2:13;
	u32 rsvd3;
};

enum rxq_comp_rss_type {
	RXQ_COMP_RSS_TYPE_NONE = 0,
	RXQ_COMP_RSS_TYPE_IPV4,
	RXQ_COMP_RSS_TYPE_IPV4_TCP,
	RXQ_COMP_RSS_TYPE_IPV4_UDP,
	RXQ_COMP_RSS_TYPE_IPV6,
	RXQ_COMP_RSS_TYPE_IPV6_TCP,
	RXQ_COMP_RSS_TYPE_IPV6_UDP,
	RXQ_COMP_RSS_TYPE_IPV6_EX,
	RXQ_COMP_RSS_TYPE_IPV6_TCP_EX,
	RXQ_COMP_RSS_TYPE_IPV6_UDP_EX,
};

/** struct rxq_comp - Ethernet receive queue completion descriptor
 * @status:       The status of the command.  Values for status are:
 *                   0 = Successful completion
 * @comp_index:   The index in the descriptor ring for which this
 *                is the completion.
 * @rss_hash:     32-bit RSS hash for the @rss_type indicated
 * @csum:         16-bit sum of the packet’s L2 payload.
 *                If the packet’s L2 payload is odd length, an extra
 *                zero-value byte is included in the @csum calculation but
 *                not included in @len.
 * @vlan_tci:     VLAN tag stripped from the packet.  Valid if @V is
 *                set.  Includes .1p and .1q tags.
 * @len:          Received packet length, in bytes.  Excludes FCS.
 * @csum_calc     L2 payload checksum is computed or not
 * @rss_type:     RSS type for @rss_hash:
 *                   0 = RSS hash not calcuated
 *                   1 = L3 IPv4
 *                   2 = L4 IPv4/TCP
 *                   3 = L4 IPv4/UDP
 *                   4 = L3 IPv6
 *                   5 = L4 IPv6/TCP
 *                   6 = L4 IPv6/UDP
 *                   7 = L3 IPv6 w/ extensions
 *                   8 = L4 IPv6/TCP w/ extensions
 *                   9 = L4 IPv6/UDP w/ extensions
 * @csum_tcp_ok:  The TCP checksum calculated by the device
 *                matched the checksum in the receive packet's
 *                TCP header
 * @csum_tcp_bad: The TCP checksum calculated by the device did
 *                not match the checksum in the receive packet's
 *                TCP header.
 * @csum_udp_ok:  The UDP checksum calculated by the device
 *                matched the checksum in the receive packet's
 *                UDP header
 * @csum_udp_bad: The UDP checksum calculated by the device did
 *                not match the checksum in the receive packet's
 *                UDP header.
 * @csum_ip_ok:   The IPv4 checksum calculated by the device
 *                matched the checksum in the receive packet's
 *                first IPv4 header.  If the receive packet
 *                contains both a tunnel IPv4 header and a
 *                transport IPv4 header, the device validates the
 *                checksum for the both IPv4 headers.
 * @csum_ip_bad:  The IPv4 checksum calculated by the device did
 *                not match the checksum in the receive packet's
 *                first IPv4 header. If the receive packet
 *                contains both a tunnel IPv4 header and a
 *                transport IPv4 header, the device validates the
 *                checksum for both IP headers.
 * @V:            VLAN header was stripped and placed in @vlan_tci.
 * @color:        Color bit.
 */
struct rxq_comp {
	u32 status:8;
	u32 rsvd:8;
	u32 comp_index:16;
	u32 rss_hash;
	u16 csum;
	u16 vlan_tci;
	u32 len:14;
	u32 rsvd2:2;
	u32 rss_type:4;
	u32 csum_calc:1;
	u32 rsvd3:3;
	u32 csum_tcp_ok:1;
	u32 csum_tcp_bad:1;
	u32 csum_udp_ok:1;
	u32 csum_udp_bad:1;
	u32 csum_ip_ok:1;
	u32 csum_ip_bad:1;
	u32 V:1;
	u32 color:1;
};

enum feature_set {
	FEATURE_SET_ETH_HW_FEATURES = 1,
};

enum eth_hw_features {
	ETH_HW_VLAN_TX_TAG	= BIT(0),
	ETH_HW_VLAN_RX_STRIP	= BIT(1),
	ETH_HW_VLAN_RX_FILTER	= BIT(2),
	ETH_HW_RX_HASH		= BIT(3),
	ETH_HW_RX_CSUM		= BIT(4),
	ETH_HW_TX_SG		= BIT(5),
	ETH_HW_TX_CSUM		= BIT(6),
	ETH_HW_TSO		= BIT(7),
	ETH_HW_TSO_IPV6		= BIT(8),
	ETH_HW_TSO_ECN		= BIT(9),
	ETH_HW_TSO_GRE		= BIT(10),
	ETH_HW_TSO_GRE_CSUM	= BIT(11),
	ETH_HW_TSO_IPXIP4	= BIT(12),
	ETH_HW_TSO_IPXIP6	= BIT(13),
	ETH_HW_TSO_UDP		= BIT(14),
	ETH_HW_TSO_UDP_CSUM	= BIT(15),
};

/**
 * struct features_cmd - Features command
 * @opcode:     opcode = 7
 * @set:        Feature set (see enum feature_set)
 * @wanted:     Features from set wanted by driver.
 */
struct features_cmd {
	u16 opcode;
	u16 set;
	u32 wanted;
	u32 rsvd2[14];
};

/**
 * struct features_comp - Features command completion format
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @comp_index: The index in the descriptor ring for which this
 *              is the completion.
 * @supported:  Features from set supported by device.
 * @color:      Color bit.
 */
struct features_comp {
	u32 status:8;
	u32 rsvd:8;
	u32 comp_index:16;
	u32 supported;
	u32 rsvd2;
	u32 rsvd3:31;
	u32 color:1;
};

/**
 * struct hang_notify_cmd - Hang notify command
 * @opcode:     opcode = 8
 */
struct hang_notify_cmd {
	u16 opcode;
	u16 rsvd[31];
};

/**
 * struct hang_notify_comp - Hang notify command completion
 * @status: The status of the command.  Values for status are:
 *             0 = Successful completion
 */
struct hang_notify_comp {
	u32 status:8;
	u32 rsvd:24;
	u32 rsvd2[3];
};

/**
 * struct q_enable_cmd - Queue enable command
 * @opcode:     opcode = 9
 * @qid:        Queue ID
 * @qtype:      Queue type
 */
struct q_enable_cmd {
	u16 opcode;
	u16 rsvd;
	u32 qid:24;
	u32 qtype:8;
	u32 rsvd2[14];
};

typedef struct admin_comp q_enable_comp;

/**
 * struct q_disable_cmd - Queue disable command
 * @opcode:     opcode = 10
 * @qid:        Queue ID
 * @qtype:      Queue type
 */
struct q_disable_cmd {
	u16 opcode;
	u16 rsvd;
	u32 qid:24;
	u32 qtype:8;
	u32 rsvd2[14];
};

typedef struct admin_comp q_disable_comp;

/**
 * struct notifyq_init_cmd - Event queue init command
 * @opcode:       opcode = 11
 * @pid:          Process ID
 * @index:        LIF-relative queue index
 * @intr_index:   Interrupt control register index
 * @lif_index:    LIF index (should be 0)
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
 * @ring_base:    Notify queue ring base address. Should be aligned
 *                on PAGE_SIZE. If not aligned properly can cause
 *                CQ Errors
 * @notify_base:  Base address for a block of memory reserved for
 *                link status data, to be updated by the NIC and
 *                read by the driver.  When link status changes,
 *                the NIC should update this before signaling an
 *                interrupt on the NotifyQ.
 */
struct notifyq_init_cmd {
	u16 opcode;
	u16 pid;
	u16 index;
	u16 intr_index;
	u32 lif_index;
	u8 ring_size;
	u8 notify_size;
	u16 rsvd;
	dma_addr_t ring_base;
	dma_addr_t notify_base;
	u32 rsvd2[8];
};

/**
 * struct notifyq_init_comp - Event queue init command completion
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @comp_index: The index in the descriptor ring for which this
 *              is the completion.
 * @qid:        Queue ID
 * @qtype:      Queue type
 * @color:      Color bit.
 */
struct notifyq_init_comp {
	u8 status;
	u8 rsvd;
	u16 comp_index;
	u32 qid;
	u8 qtype;
	u8 rsvd3[6];
	u8 color;
};

/**
 * Struct notify_block - Memory block for notifications, updated by the NIC
 * @eid:             most recent NotifyQ event id
 * @link_status      link up/down
 * @link_error_bits: error bits if needed
 * @link_speed:	     speed of link in Gbps
 * @phy_type:        type of physical connection
 * @autoneg_status:  autonegotiation status
 */
struct notify_block {
	u64 eid;
	u16 link_status;
	u16 link_error_bits;
	u32 link_speed;		/* units of 1Mbps: e.g. 10000 = 10Gbps */
	u16 phy_type;
	u16 autoneg_status;
	u16 link_flap_count;
};

/**
 * struct station_mac_addr_get_cmd - Get LIF's station MAC address
 *                                   command
 * @opcode:     opcode = 15
 */
struct station_mac_addr_get_cmd {
	u16 opcode;
	u16 rsvd[31];
};

/**
 * struct station_mac_addr_get_comp - Get LIF's station MAC address
 *                                    command completion
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @comp_index: The index in the descriptor ring for which this
 *              is the completion.
 * @addr:       Station MAC address (network-byte order)
 * @color:      Color bit.
 */
struct station_mac_addr_get_comp {
	u32 status:8;
	u32 rsvd:8;
	u32 comp_index:16;
	u8 addr[6];
	u16 rsvd2;
	u32 rsvd3:31;
	u32 color:1;
};

/**
 * struct mtu_set_cmd - Set LIF's MTU command
 * @opcode:     opcode = 16
 * @mtu:        MTU.  Min MTU=68, Min IPv4 MTU per RFC791.
 *              Max MTU=9200.
 */
struct mtu_set_cmd {
	u16 opcode;
	u16 mtu;
	u16 rsvd[30];
};

typedef struct admin_comp mtu_set_comp;

enum rx_mode {
	RX_MODE_F_UNICAST		= BIT(0),
	RX_MODE_F_MULTICAST		= BIT(1),
	RX_MODE_F_BROADCAST		= BIT(2),
	RX_MODE_F_PROMISC		= BIT(3),
	RX_MODE_F_ALLMULTI		= BIT(4),
};

/**
 * struct rx_mode_set_cmd - Set LIF's Rx mode command
 * @opcode:     opcode = 17
 * @rx_mode:    Rx mode flags:
 *                  RX_MODE_F_UNICAST: Accept known unicast
 *                  packets.
 *                  RX_MODE_F_MULTICAST: Accept known
 *                  multicast packets.
 *                  RX_MODE_F_BROADCAST: Accept broadcast
 *                  packets.
 *                  RX_MODE_F_PROMISC: Accept any packets.
 *                  RX_MODE_F_ALLMULTI: Accept any multicast
 *                  packets.
 */
struct rx_mode_set_cmd {
	u16 opcode;
	u16 rx_mode;
	u16 rsvd[30];
};

typedef struct admin_comp rx_mode_set_comp;

enum rx_filter_match_type {
	RX_FILTER_MATCH_VLAN = 0,
	RX_FILTER_MATCH_MAC,
	RX_FILTER_MATCH_MAC_VLAN,
};

/**
 * struct rx_filter_add_cmd - Add LIF Rx filter command
 * @opcode:     opcode = 18
 * @match:      Rx filter match type.  (See RX_FILTER_MATCH_xxx)
 * @vlan:       VLAN ID
 * @addr:       MAC address (network-byte order)
 * @qid:        Queue ID
 * @qtype:      Queue type
 */
struct rx_filter_add_cmd {
	u16 opcode;
	u16 match;
	union {
		struct {
			u16 vlan;
			u16 rsvd[29];
		} vlan;
		struct {
			u8 addr[6];
			u8 rsvd[2];
			u16 rsvd2[26];
		} mac;
		struct {
			u16 vlan;
			u8 addr[6];
			u8 rsvd[2];
			u16 rsvd3[25];
		} mac_vlan;
	};
};

/**
 * struct rx_filter_add_comp - Add LIF Rx filter command completion
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @comp_index: The index in the descriptor ring for which this
 *              is the completion.
 * @filter_id:  Filter ID
 * @color:      Color bit.
 */
struct rx_filter_add_comp {
	u32 status:8;
	u32 rsvd:8;
	u32 comp_index:16;
	u32 filter_id;
	u32 rsvd2;
	u32 rsvd3:31;
	u32 color:1;
};

/**
 * struct rx_filter_del_cmd - Delete LIF Rx filter command
 * @opcode:     opcode = 19
 * @filter_id:  Filter ID
 */
struct rx_filter_del_cmd {
	u16 opcode;
	u32 filter_id;
	u16 rsvd[29];
};

typedef struct admin_comp rx_filter_del_comp;

#define STATS_DUMP_VERSION_1		1

/**
 * struct stats_dump_cmd - Setup stats dump shared memory command
 * @opcode:     opcode = 20 (start), 21 (stop)
 * @ver:        Highest version of stats supported by driver:
 *                 1 = version 1.0 stats
 * @addr:       Destination address for the 4096-byte stats shared
 *              memory area (only valid when starting stats dump).
 *              (See union stats_dump).
 *
 * Once the start stats dump command is called, the device will
 * periodically dump stats to the shared memory area @addr.
 * The device will stop stats dump to the shared memory area
 * when the stop stats dump cmd is called.  Once a stopped
 * completion is received, the shared memory area can be
 * released by the driver.
 */
struct stats_dump_cmd {
	u16 opcode;
	u16 ver;
	dma_addr_t addr;
	u32 rsvd2[13];
};

/**
 * struct stats_dump_comp - Setup stats dump shared memory
 *                          completion
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 *                 1 = Version not supported by device
 * @comp_index: The index in the descriptor ring for which this
 *              is the completion.
 * @ver:        Version of stats return by device.  The version
 *              returned by the device can be <= the requested
 *              version.
 * @color:      Color bit.
 */
struct stats_dump_comp {
	u32 status:8;
	u32 rsvd:8;
	u32 comp_index:16;
	u16 ver;
	u16 rsvd2[3];
	u32 rsvd3:31;
	u32 color:1;
};

/**
 * struct stats_dump - 4096 bytes of device stats
 */
struct stats_dump {
	u64 rx_ucast_bytes;            /*   0 */
	u64 rx_ucast_packets;          /*   8 */
	u64 rx_mcast_bytes;            /*  16 */
	u64 rx_mcast_packets;          /*  24 */
	u64 rx_bcast_bytes;            /*  32 */
	u64 rx_bcast_packets;          /*  40 */
	u64 pad1[2];

	u64 rx_ucast_drop_bytes;       /*  64 */
	u64 rx_ucast_drop_packets;     /*  72 */
	u64 rx_mcast_drop_bytes;       /*  80 */
	u64 rx_mcast_drop_packets;     /*  88 */
	u64 rx_bcast_drop_bytes;       /*  96 */
	u64 rx_bcast_drop_packets;     /* 104 */
	u64 rx_dma_error;              /* 112 */
	u64 pad2;

	u64 tx_ucast_bytes;            /* 128 */
	u64 tx_ucast_packets;          /* 136 */
	u64 tx_mcast_bytes;            /* 144 */
	u64 tx_mcast_packets;          /* 152 */
	u64 tx_bcast_bytes;            /* 160 */
	u64 tx_bcast_packets;          /* 168 */
	u64 pad3[2];

	u64 tx_ucast_drop_bytes;       /* 192 */
	u64 tx_ucast_drop_packets;     /* 200 */
	u64 tx_mcast_drop_bytes;       /* 208 */
	u64 tx_mcast_drop_packets;     /* 216 */
	u64 tx_bcast_drop_bytes;       /* 224 */
	u64 tx_bcast_drop_packets;     /* 232 */
	u64 tx_dma_error;              /* 240 */
	u64 pad4[2];

	u64 rx_queue_disabled_drop;    /* 256 */
	u64 rx_queue_empty_drop;       /* 264 */
	u64 rx_queue_scheduled;        /* 272 */
	u64 rx_desc_fetch_error;       /* 280 */
	u64 rx_desc_data_error;        /* 288 */
	u64 pad5[3];

	u64 tx_queue_disabled;         /* 320 */
	u64 tx_queue_scheduled;        /* 328 */
	u64 tx_desc_fetch_error;       /* 336 */
	u64 tx_desc_data_error;        /* 344 */
	u64 pad6[5];

	/* Debug counters */
	u64 rx_rss;                    /* 384 */
	u64 rx_csum_complete;          /* 392 */
	u64 rx_csum_ip_bad;            /* 400 */
	u64 rx_csum_tcp_bad;           /* 408 */
	u64 rx_csum_udp_bad;           /* 416 */
	u64 rx_vlan_strip;             /* 424 */
	u64 pad7[3];

	u64 tx_csum_hw;                /* 448 */
	u64 tx_csum_hw_inner;          /* 456 */
	u64 tx_vlan_insert;            /* 464 */
	u64 tx_sg;                     /* 472 */
	u64 tx_tso_sg;                 /* 480 */
	u64 tx_tso_sop;                /* 488 */
	u64 tx_tso_eop;                /* 496 */
	u64 pad8[6];

	u64 tx_opcode_invalid;         /* 560 */
	u64 tx_opcode_csum_none;       /* 568 */
	u64 tx_opcode_csum_partial;    /* 576 */
	u64 tx_opcode_csum_hw;         /* 584 */
	u64 tx_opcode_csum_tso;        /* 592 */

	u64 pad_to_1024[51];
	u64 pad_to_2048[128];
	u64 pad_to_3072[128];
	u64 pad_to_4096[128];
};

#define RSS_HASH_KEY_SIZE	40

enum rss_hash_types {
/* Conflicts with FreeBSD rss definitions. */
#ifndef __FreeBSD__
	RSS_TYPE_IPV4		= BIT(0),
	RSS_TYPE_IPV4_TCP	= BIT(1),
	RSS_TYPE_IPV4_UDP	= BIT(2),
	RSS_TYPE_IPV6		= BIT(3),
	RSS_TYPE_IPV6_TCP	= BIT(4),
	RSS_TYPE_IPV6_UDP	= BIT(5),
	RSS_TYPE_IPV6_EX	= BIT(6),
	RSS_TYPE_IPV6_TCP_EX	= BIT(7),
	RSS_TYPE_IPV6_UDP_EX	= BIT(8),
#else
	IONIC_RSS_TYPE_IPV4		= BIT(0),
	IONIC_RSS_TYPE_IPV4_TCP	= BIT(1),
	IONIC_RSS_TYPE_IPV4_UDP	= BIT(2),
	IONIC_RSS_TYPE_IPV6		= BIT(3),
	IONIC_RSS_TYPE_IPV6_TCP	= BIT(4),
	IONIC_RSS_TYPE_IPV6_UDP	= BIT(5),
	IONIC_RSS_TYPE_IPV6_EX	= BIT(6),
	IONIC_RSS_TYPE_IPV6_TCP_EX	= BIT(7),
	IONIC_RSS_TYPE_IPV6_UDP_EX	= BIT(8),
#endif
};

/**
 * struct rss_hash_set_cmd - Set the RSS hash types and the secret key
 * @opcode:    opcode = 22
 * @types:     The hash types to enable (see rss_hash_types).
 * @key:       The hash secret key.
 */
struct rss_hash_set_cmd {
	u16 opcode;
	u16 types;
	u8 key[RSS_HASH_KEY_SIZE];
	u32 rsvd[5];
};

typedef struct admin_comp rss_hash_set_comp;

#define RSS_IND_TBL_SIZE	128

/**
 * struct rss_indir_set_cmd - Set the RSS indirection table values
 * @opcode:    opcode = 23
 * @addr:      Address for RSS indirection table shared memory.
*/
struct rss_indir_set_cmd {
	u16 opcode;
	dma_addr_t addr;
	u16 rsvd[27];
};

typedef struct admin_comp rss_indir_set_comp;

/**
 * struct debug_q_dump_cmd - Debug queue dump command
 * @opcode:     opcode = 0xf0
 * @qid:        Queue ID
 * @qtype:      Queue type
 */
struct debug_q_dump_cmd {
	u16 opcode;
	u16 rsvd;
	u32 qid:24;
	u32 qtype:8;
	u32 rsvd2[14];
};

/**
 * struct debug_q_dump_comp - Debug queue dump command completion
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @comp_index: The index in the descriptor ring for which this
 *              is the completion.
 * @p_index0:   Queue 0 producer index
 * @c_index0:   Queue 0 consumer index
 * @p_index1:   Queue 1 producer index
 * @c_index1:   Queue 1 consumer index
 * @color:      Color bit.
 */
struct debug_q_dump_comp {
	u32 status:8;
	u32 rsvd:8;
	u32 comp_index:16;
	u16 p_index0;
	u16 c_index0;
	u16 p_index1;
	u16 c_index1;
	u32 rsvd2:31;
	u32 color:1;
};

/******************************************************************
 ******************* RDMA Commands ********************************
 ******************************************************************/

/**
 * struct rdma_reset_cmd - Reset RDMA LIF cmd
 * @opcode:        opcode = 50
 * @lif_id:        hardware lif id
 *
 * There is no rdma specific dev command completion struct.  Completion uses
 * the common struct admin_comp.  Only the status is indicated.  Nonzero status
 * means the LIF does not support rdma.
 **/
struct rdma_reset_cmd {
	u16 opcode;
	u16 lif_id;
	u8 rsvd[60];
};

/**
 * struct rdma_queue_cmd - Create RDMA Queue command
 * @opcode:        opcode = 51, 52, 53
 * @lif_id:        hardware lif id
 * @qid_ver:       (qid | (rdma version << 24))
 * @cid:           intr, eq_id, or cq_id
 * @dbid:          doorbell page id
 * @depth_log2:    log base two of queue depth
 * @stride_log2:   log base two of queue stride
 * @dma_addr:      address of the queue memory
 * @xxx_table_index: temporary, but should not need pgtbl for contig. queues.
 *
 * The same command struct is used to create an rdma event queue, completion
 * queue, or rdma admin queue.  The cid is an interrupt number for an event
 * queue, an event queue id for a completion queue, or a completion queue id
 * for an rdma admin queue.
 *
 * The queue created via a dev command must be contiguous in dma space.
 *
 * The dev commands are intended only to be used during driver initialization,
 * to create queues supporting the rdma admin queue.  Other queues, and other
 * types of rdma resources like memory regions, will be created and registered
 * via the rdma admin queue, and will support a more complete interface
 * providing scatter gather lists for larger, scattered queue buffers and
 * memory registration.
 *
 * There is no rdma specific dev command completion struct.  Completion uses
 * the common struct admin_comp.  Only the status is indicated.
 **/
struct rdma_queue_cmd {
	u16 opcode;
	u16 lif_id;
	u32 qid_ver;
	u32 cid;
	u16 dbid;
	u8 depth_log2;
	u8 stride_log2;
	u64 dma_addr;
	u8 rsvd[36];
	u32 xxx_table_index;
};

/******************************************************************
 ******************* Notify Events ********************************
 ******************************************************************/

/**
 * struct notifyq_event
 * @eid:   event number
 * @ecode: event code
 * @data:  unspecified data about the event
 *
 * This is the generic event report struct from which the other
 * actual events will be formed.
 */
struct notifyq_event {
	u64 eid;
	u16 ecode;
	u8 data[54];
};

/**
 * struct link_change_event
 * @eid:		event number
 * @ecode:		event code = EVENT_OPCODE_LINK_CHANGE
 * @link_status:	link up or down, with error bits
 * @phy_type:		specifies the type of PHY connected
 * @link_speed:		speed of the network link
 * @autoneg_status:	autonegotiation data
 *
 * Sent when the network link state changes between UP and DOWN
 */
struct link_change_event {
	u64 eid;
	u16 ecode;
	u16 link_status;	/* 0 = down, 1 = up */
	u16 link_error_bits;	/* TBD */
	u16 phy_type;
	u32 link_speed;		/* units of 1Mbps: e.g. 10000 = 10Gbps */
	u16 autoneg_status;
	u8 rsvd[42];
};

/**
 * struct reset_event
 * @eid:		event number
 * @ecode:		event code = EVENT_OPCODE_RESET
 * @reset_code:		reset type
 * @state:		0=pending, 1=complete, 2=error
 *
 * Sent when the NIC or some subsystem is going to be or
 * has been reset.
 */
struct reset_event {
	u64 eid;
	u16 ecode;
	u8 reset_code;
	u8 state;
	u8 rsvd[52];
};

/**
 * struct heartbeat_event
 * @eid:	event number
 * @ecode:	event code = EVENT_OPCODE_HEARTBEAT
 *
 * Sent periodically by the NIC to indicate continued health
 */
struct heartbeat_event {
	u64 eid;
	u16 ecode;
	u8 rsvd[54];
};

/**
 * struct log_event
 * @eid:	event number
 * @ecode:	event code = EVENT_OPCODE_LOG
 * @data:	log data
 *
 * Sent to notify the driver of an internal error.
 */
struct log_event {
	u64 eid;
	u16 ecode;
	u8 data[54];
};

/**
 * struct ionic_lif_stats
 */
struct ionic_lif_stats {
	// RX
	uint64_t rx_ucast_bytes;
	uint64_t rx_ucast_packets;
	uint64_t rx_mcast_bytes;
	uint64_t rx_mcast_packets;
	uint64_t rx_bcast_bytes;
	uint64_t rx_bcast_packets;
	uint64_t rx_dma_error;
	uint64_t rsvd0;
	// RX drops
	uint64_t rx_ucast_drop_bytes;
	uint64_t rx_ucast_drop_packets;
	uint64_t rx_mcast_drop_bytes;
	uint64_t rx_mcast_drop_packets;
	uint64_t rx_bcast_drop_bytes;
	uint64_t rx_bcast_drop_packets;
	uint64_t rsvd1;
	uint64_t rsvd2;
	// TX
	uint64_t tx_ucast_bytes;
	uint64_t tx_ucast_packets;
	uint64_t tx_mcast_bytes;
	uint64_t tx_mcast_packets;
	uint64_t tx_bcast_bytes;
	uint64_t tx_bcast_packets;
	uint64_t tx_dma_error;
	uint64_t rsvd3;
	// TX drops
	uint64_t tx_ucast_drop_bytes;
	uint64_t tx_ucast_drop_packets;
	uint64_t tx_mcast_drop_bytes;
	uint64_t tx_mcast_drop_packets;
	uint64_t tx_bcast_drop_bytes;
	uint64_t tx_bcast_drop_packets;
	uint64_t rsvd4;
	uint64_t rsvd5;
	//Rx Queue/Ring drops
	uint64_t rx_q_disable_drop;
	uint64_t rx_q_empty_drop;
	uint64_t rx_q_empty_scheduled;
	uint64_t rx_desc_fetch_error;
	uint64_t rx_desc_data_error;
	uint64_t rsvd6;
	uint64_t rsvd7;
	uint64_t rsvd8;
	//Tx Queue/Ring drops
	uint64_t tx_q_disable_drop;
	uint64_t tx_q_empty_drop;
	uint64_t tx_desc_fetch_error;
	uint64_t tx_desc_data_error;
	uint64_t rsvd9;
	uint64_t rsvd10;
	uint64_t rsvd11;
	uint64_t rsvd12;

	// RDMA/ROCE TX
	uint64_t tx_rdma_ucast_bytes;
	uint64_t tx_rdma_ucast_packets;
	uint64_t tx_rdma_mcast_bytes;
	uint64_t tx_rdma_mcast_packets;
	uint64_t tx_rdma_cnp_packets;
	uint64_t rsvd13;
	uint64_t rsvd14;
	uint64_t rsvd15;

	// RDMA/ROCE RX
	uint64_t rx_rdma_ucast_bytes;
	uint64_t rx_rdma_ucast_packets;
	uint64_t rx_rdma_mcast_bytes;
	uint64_t rx_rdma_mcast_packets;
	uint64_t rx_rdma_cnp_packets;
	uint64_t rx_rdma_ecn_packets;
	uint64_t rsvd16;
	uint64_t rsvd17;

	uint64_t rsvd18;
	uint64_t rsvd19;
	uint64_t rsvd20;
	uint64_t rsvd21;
	uint64_t rsvd22;
	uint64_t rsvd23;
	uint64_t rsvd24;
	uint64_t rsvd25;

	uint64_t rsvd26;
	uint64_t rsvd27;
	uint64_t rsvd28;
	uint64_t rsvd29;
	uint64_t rsvd30;
	uint64_t rsvd31;
	uint64_t rsvd32;
	uint64_t rsvd33;

	uint64_t rsvd34;
	uint64_t rsvd35;
	uint64_t rsvd36;
	uint64_t rsvd37;
	uint64_t rsvd38;
	uint64_t rsvd39;
	uint64_t rsvd40;
	uint64_t rsvd41;

	uint64_t rsvd42;
	uint64_t rsvd43;
	uint64_t rsvd44;
	uint64_t rsvd45;
	uint64_t rsvd46;
	uint64_t rsvd47;
	uint64_t rsvd48;
	uint64_t rsvd49;

	// RDMA/ROCE REQ Error/Debugs (768 - 895)
	uint64_t rdma_req_rx_pkt_seq_err;
	uint64_t rdma_req_rx_rnr_retry_err;
	uint64_t rdma_req_rx_remote_access_err;
	uint64_t rdma_req_rx_remote_inv_req_err;
	uint64_t rdma_req_rx_remote_oper_err;
	uint64_t rdma_req_rx_implied_nak_seq_err;
	uint64_t rdma_req_rx_cqe_err;
	uint64_t rdma_req_rx_cqe_flush_err;

	uint64_t rdma_req_tx_local_access_err;
	uint64_t rdma_req_tx_local_oper_err;
	uint64_t rdma_req_tx_memory_mgmt_err;
	uint64_t rsvd50;
	uint64_t rsvd51;
	uint64_t rsvd52;
	uint64_t rsvd53;
	uint64_t rsvd54;

	// RDMA/ROCE RESP Error/Debugs (896 - 1023)
	uint64_t rdma_resp_rx_dup_requests;
	uint64_t rdma_resp_rx_out_of_buffer;
	uint64_t rdma_resp_rx_out_of_seq_pkts;
	uint64_t rdma_resp_rx_cqe_err;
	uint64_t rdma_resp_rx_cqe_flush_err;
	uint64_t rdma_resp_rx_local_len_err;
	uint64_t rsvd55;
	uint64_t rsvd56;

	uint64_t rdma_resp_tx_pkt_seq_err;
	uint64_t rdma_resp_tx_rnr_retry_err;
	uint64_t rdma_resp_tx_remote_access_err;
	uint64_t rdma_resp_tx_remote_inv_req_err;
	uint64_t rdma_resp_tx_remote_oper_err;
	uint64_t rsvd57;
	uint64_t rsvd58;
	uint64_t rsvd59;
};

#pragma pack(pop)

union adminq_cmd {
	struct admin_cmd cmd;
	struct notifyq_init_cmd notifyq_init;
	struct nop_cmd nop;
	struct txq_init_cmd txq_init;
	struct rxq_init_cmd rxq_init;
	struct features_cmd features;
	struct q_enable_cmd q_enable;
	struct q_disable_cmd q_disable;
	struct station_mac_addr_get_cmd station_mac_addr_get;
	struct mtu_set_cmd mtu_set;
	struct rx_mode_set_cmd rx_mode_set;
	struct rx_filter_add_cmd rx_filter_add;
	struct rx_filter_del_cmd rx_filter_del;
	struct stats_dump_cmd stats_dump;
	struct rss_hash_set_cmd rss_hash_set;
	struct rss_indir_set_cmd rss_indir_set;
	struct debug_q_dump_cmd debug_q_dump;
	struct rdma_reset_cmd rdma_reset;
	struct rdma_queue_cmd rdma_queue;
};

union adminq_comp {
	struct admin_comp comp;
	struct notifyq_init_comp notifyq_init;
	struct nop_comp nop;
	struct txq_init_comp txq_init;
	struct rxq_init_comp rxq_init;
	struct features_comp features;
	struct station_mac_addr_get_comp station_mac_addr_get;
	struct rx_filter_add_comp rx_filter_add;
	struct stats_dump_comp stats_dump;
	struct debug_q_dump_comp debug_q_dump;
};

struct notifyq_cmd {
	u32 data;	/* Not used but needed for qcq structure */
};

union notifyq_comp {
	struct notifyq_event event;
	struct link_change_event link_change;
	struct reset_event reset;
	struct heartbeat_event heartbeat;
	struct log_event log;
};

#endif /* _IONIC_IF_H_ */
