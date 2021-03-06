/* SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB OR BSD-2-Clause */
/* Copyright (c) 2017-2019 Pensando Systems, Inc.  All rights reserved. */

#ifndef _IONIC_IF_H_
#define _IONIC_IF_H_

#pragma pack(push, 1)

#define IONIC_DEV_INFO_SIGNATURE		0x44455649      /* 'DEVI' */
#define IONIC_DEV_INFO_VERSION			1
#define IONIC_IFNAMSIZ				16

/**
 * Commands
 */
enum cmd_opcode {
	CMD_OPCODE_NOP				= 0,

	/* Device commands */
	CMD_OPCODE_IDENTIFY			= 1,
	CMD_OPCODE_INIT				= 2,
	CMD_OPCODE_RESET			= 3,
	CMD_OPCODE_GETATTR			= 4,
	CMD_OPCODE_SETATTR			= 5,

	/* Port commands */
	CMD_OPCODE_PORT_IDENTIFY		= 10,
	CMD_OPCODE_PORT_INIT			= 11,
	CMD_OPCODE_PORT_RESET			= 12,
	CMD_OPCODE_PORT_GETATTR			= 13,
	CMD_OPCODE_PORT_SETATTR			= 14,

	/* LIF commands */
	CMD_OPCODE_LIF_IDENTIFY			= 20,
	CMD_OPCODE_LIF_INIT			= 21,
	CMD_OPCODE_LIF_RESET			= 22,
	CMD_OPCODE_LIF_GETATTR			= 23,
	CMD_OPCODE_LIF_SETATTR			= 24,

	CMD_OPCODE_RX_MODE_SET			= 30,
	CMD_OPCODE_RX_FILTER_ADD		= 31,
	CMD_OPCODE_RX_FILTER_DEL		= 32,

	/* Queue commands */
	CMD_OPCODE_Q_IDENTIFY			= 39,
	CMD_OPCODE_Q_INIT			= 40,
	CMD_OPCODE_Q_CONTROL			= 41,

	/* RDMA commands */
	CMD_OPCODE_RDMA_RESET_LIF		= 50,
	CMD_OPCODE_RDMA_CREATE_EQ		= 51,
	CMD_OPCODE_RDMA_CREATE_CQ		= 52,
	CMD_OPCODE_RDMA_CREATE_ADMINQ		= 53,

	/* QoS commands */
	CMD_OPCODE_QOS_CLASS_IDENTIFY		= 240,
	CMD_OPCODE_QOS_CLASS_INIT		= 241,
	CMD_OPCODE_QOS_CLASS_RESET		= 242,

	/* Firmware commands */
	CMD_OPCODE_FW_DOWNLOAD			= 254,
	CMD_OPCODE_FW_CONTROL			= 255,
};

/**
 * Command Return codes
 */
enum status_code {
	IONIC_RC_SUCCESS	= 0,	/* Success */
	IONIC_RC_EVERSION	= 1,	/* Incorrect version for request */
	IONIC_RC_EOPCODE	= 2,	/* Invalid cmd opcode */
	IONIC_RC_EIO		= 3,	/* I/O error */
	IONIC_RC_EPERM		= 4,	/* Permission denied */
	IONIC_RC_EQID		= 5,	/* Bad qid */
	IONIC_RC_EQTYPE		= 6,	/* Bad qtype */
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
	IONIC_RC_DEV_CMD	= 18,	/* Device cmd attempted on AdminQ */
	IONIC_RC_ENOSUPP	= 19,    /* Operation not supported */
	IONIC_RC_ERROR		= 29,	/* Generic error */

	IONIC_RC_ERDMA		= 30,	/* Generic RDMA error */
};

enum notifyq_opcode {
	EVENT_OPCODE_LINK_CHANGE	= 1,
	EVENT_OPCODE_RESET		= 2,
	EVENT_OPCODE_HEARTBEAT		= 3,
	EVENT_OPCODE_LOG		= 4,
	EVENT_OPCODE_XCVR		= 5,
};

/**
 * struct cmd - General admin command format
 * @opcode:     Opcode for the command
 * @lif_index:  LIF index
 * @cmd_data:   Opcode-specific command bytes
 */
struct admin_cmd {
	u8     opcode;
	u8     rsvd;
	__le16 lif_index;
	u8     cmd_data[60];
};

/**
 * struct admin_comp - General admin command completion format
 * @status:     The status of the command (enum status_code)
 * @comp_index: The index in the descriptor ring for which this
 *              is the completion.
 * @cmd_data:   Command-specific bytes.
 * @color:      Color bit.  (Always 0 for commands issued to the
 *              Device Cmd Registers.)
 */
struct admin_comp {
	u8     status;
	u8     rsvd;
	__le16 comp_index;
	u8     cmd_data[11];
	u8     color;
#define IONIC_COMP_COLOR_MASK  0x80
};

static inline u8 color_match(u8 color, u8 done_color)
{
	return (!!(color & IONIC_COMP_COLOR_MASK)) == done_color;
}

/**
 * struct nop_cmd - NOP command
 * @opcode: opcode
 */
struct nop_cmd {
	u8 opcode;
	u8 rsvd[63];
};

/**
 * struct nop_comp - NOP command completion
 * @status: The status of the command (enum status_code)
 */
struct nop_comp {
	u8 status;
	u8 rsvd[15];
};

/**
 * struct dev_init_cmd - Device init command
 * @opcode:    opcode
 * @type:      device type
 */
struct dev_init_cmd {
	u8     opcode;
	u8     type;
	u8     rsvd[62];
};

/**
 * struct init_comp - Device init command completion
 * @status: The status of the command (enum status_code)
 */
struct dev_init_comp {
	u8 status;
	u8 rsvd[15];
};

/**
 * struct dev_reset_cmd - Device reset command
 * @opcode: opcode
 */
struct dev_reset_cmd {
	u8 opcode;
	u8 rsvd[63];
};

/**
 * struct reset_comp - Reset command completion
 * @status: The status of the command (enum status_code)
 */
struct dev_reset_comp {
	u8 status;
	u8 rsvd[15];
};

#define IONIC_IDENTITY_VERSION_1	1

/**
 * struct dev_identify_cmd - Driver/device identify command
 * @opcode:  opcode
 * @ver:     Highest version of identify supported by driver
 */
struct dev_identify_cmd {
	u8 opcode;
	u8 ver;
	u8 rsvd[62];
};

/**
 * struct dev_identify_comp - Driver/device identify command completion
 * @status: The status of the command (enum status_code)
 * @ver:    Version of identify returned by device
 */
struct dev_identify_comp {
	u8 status;
	u8 ver;
	u8 rsvd[14];
};

enum os_type {
	IONIC_OS_TYPE_LINUX   = 1,
	IONIC_OS_TYPE_WIN     = 2,
	IONIC_OS_TYPE_DPDK    = 3,
	IONIC_OS_TYPE_FREEBSD = 4,
	IONIC_OS_TYPE_IPXE    = 5,
	IONIC_OS_TYPE_ESXI    = 6,
};

/**
 * union drv_identity - driver identity information
 * @os_type:          OS type (see enum os_type)
 * @os_dist:          OS distribution, numeric format
 * @os_dist_str:      OS distribution, string format
 * @kernel_ver:       Kernel version, numeric format
 * @kernel_ver_str:   Kernel version, string format
 * @driver_ver_str:   Driver version, string format
 */
union drv_identity {
	struct {
		__le32 os_type;
		__le32 os_dist;
		char   os_dist_str[128];
		__le32 kernel_ver;
		char   kernel_ver_str[32];
		char   driver_ver_str[32];
	};
	__le32 words[478];
};

/**
 * union dev_identity - device identity information
 * @version:          Version of device identify
 * @type:             Identify type (0 for now)
 * @nports:           Number of ports provisioned
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
 * @eq_count:         Number of shared event queues.
 */
union dev_identity {
	struct {
		u8     version;
		u8     type;
		u8     rsvd[2];
		u8     nports;
		u8     rsvd2[3];
		__le32 nlifs;
		__le32 nintrs;
		__le32 ndbpgs_per_lif;
		__le32 intr_coal_mult;
		__le32 intr_coal_div;
		__le32 eq_count;
	};
	__le32 words[478];
};

enum lif_type {
	IONIC_LIF_TYPE_CLASSIC = 0,
	IONIC_LIF_TYPE_MACVLAN = 1,
	IONIC_LIF_TYPE_NETQUEUE = 2,
};

/**
 * struct lif_identify_cmd - lif identify command
 * @opcode:  opcode
 * @type:    lif type (enum lif_type)
 * @ver:     version of identify returned by device
 */
struct lif_identify_cmd {
	u8 opcode;
	u8 type;
	u8 ver;
	u8 rsvd[61];
};

/**
 * struct lif_identify_comp - lif identify command completion
 * @status:  status of the command (enum status_code)
 * @ver:     version of identify returned by device
 */
struct lif_identify_comp {
	u8 status;
	u8 ver;
	u8 rsvd2[14];
};

enum lif_capability {
	IONIC_LIF_CAP_ETH        = BIT(0),
	IONIC_LIF_CAP_RDMA       = BIT(1),
};

/**
 * Logical Queue Types
 */
enum logical_qtype {
	IONIC_QTYPE_ADMINQ  = 0,
	IONIC_QTYPE_NOTIFYQ = 1,
	IONIC_QTYPE_RXQ     = 2,
	IONIC_QTYPE_TXQ     = 3,
	IONIC_QTYPE_EQ      = 4,
	IONIC_QTYPE_MAX     = 16,
};

/**
 * struct lif_logical_qtype - Descriptor of logical to hardware queue type.
 * @qtype:          Hardware Queue Type.
 * @qid_count:      Number of Queue IDs of the logical type.
 * @qid_base:       Minimum Queue ID of the logical type.
 */
struct lif_logical_qtype {
	u8     qtype;
	u8     rsvd[3];
	__le32 qid_count;
	__le32 qid_base;
};

enum lif_state {
	IONIC_LIF_DISABLE	= 0,
	IONIC_LIF_ENABLE	= 1,
	IONIC_LIF_HANG_RESET	= 2,
};

/**
 * LIF configuration
 * @state:          lif state (enum lif_state)
 * @name:           lif name
 * @mtu:            mtu
 * @mac:            station mac address
 * @features:       features (enum eth_hw_features)
 * @queue_count:    queue counts per queue-type
 */
union lif_config {
	struct {
		u8     state;
		u8     rsvd[3];
		char   name[IONIC_IFNAMSIZ];
		__le32 mtu;
		u8     mac[6];
		u8     rsvd2[2];
		__le64 features;
		__le32 queue_count[IONIC_QTYPE_MAX];
	};
	__le32 words[64];
};

/**
 * struct lif_identity - lif identity information (type-specific)
 *
 * @capabilities    LIF capabilities
 *
 * Ethernet:
 *     @version:          Ethernet identify structure version.
 *     @features:         Ethernet features supported on this lif type.
 *     @max_ucast_filters:  Number of perfect unicast addresses supported.
 *     @max_mcast_filters:  Number of perfect multicast addresses supported.
 *     @min_frame_size:   Minimum size of frames to be sent
 *     @max_frame_size:   Maximim size of frames to be sent
 *     @config:           LIF config struct with features, mtu, mac, q counts
 *
 * RDMA:
 *     @version:         RDMA version of opcodes and queue descriptors.
 *     @qp_opcodes:      Number of rdma queue pair opcodes supported.
 *     @admin_opcodes:   Number of rdma admin opcodes supported.
 *     @npts_per_lif:    Page table size per lif
 *     @nmrs_per_lif:    Number of memory regions per lif
 *     @nahs_per_lif:    Number of address handles per lif
 *     @max_stride:      Max work request stride.
 *     @cl_stride:       Cache line stride.
 *     @pte_stride:      Page table entry stride.
 *     @rrq_stride:      Remote RQ work request stride.
 *     @rsq_stride:      Remote SQ work request stride.
 *     @dcqcn_profiles:  Number of DCQCN profiles
 *     @aq_qtype:        RDMA Admin Qtype.
 *     @sq_qtype:        RDMA Send Qtype.
 *     @rq_qtype:        RDMA Receive Qtype.
 *     @cq_qtype:        RDMA Completion Qtype.
 *     @eq_qtype:        RDMA Event Qtype.
 */
union lif_identity {
	struct {
		__le64 capabilities;

		struct {
			u8 version;
			u8 rsvd[3];
			__le32 max_ucast_filters;
			__le32 max_mcast_filters;
			__le16 rss_ind_tbl_sz;
			__le32 min_frame_size;
			__le32 max_frame_size;
			u8 rsvd2[106];
			union lif_config config;
		} eth;

		struct {
			u8 version;
			u8 qp_opcodes;
			u8 admin_opcodes;
			u8 rsvd;
			__le32 npts_per_lif;
			__le32 nmrs_per_lif;
			__le32 nahs_per_lif;
			u8 max_stride;
			u8 cl_stride;
			u8 pte_stride;
			u8 rrq_stride;
			u8 rsq_stride;
			u8 dcqcn_profiles;
			u8 rsvd_dimensions[10];
			struct lif_logical_qtype aq_qtype;
			struct lif_logical_qtype sq_qtype;
			struct lif_logical_qtype rq_qtype;
			struct lif_logical_qtype cq_qtype;
			struct lif_logical_qtype eq_qtype;
		} rdma;
	};
	__le32 words[478];
};

/**
 * struct lif_init_cmd - LIF init command
 * @opcode:       opcode
 * @type:         LIF type (enum lif_type)
 * @index:        LIF index
 * @info_pa:      destination address for lif info (struct lif_info)
 */
struct lif_init_cmd {
	u8     opcode;
	u8     type;
	__le16 index;
	__le32 rsvd;
	__le64 info_pa;
	u8     rsvd2[48];
};

/**
 * struct lif_init_comp - LIF init command completion
 * @status: The status of the command (enum status_code)
 */
struct lif_init_comp {
	u8 status;
	u8 rsvd;
	__le16 hw_index;
	u8 rsvd2[12];
};

/**
 * struct q_identify_cmd - queue identify command
 * @opcode:     opcode
 * @lif_type:   lif type (enum lif_type)
 * @type:       logical queue type (enum logical_qtype)
 * @ver:        version of identify returned by device
 */
struct q_identify_cmd {
	u8     opcode;
	u8     rsvd;
	__le16 lif_type;
	u8     type;
	u8     ver;
	u8     rsvd2[58];
};

/**
 * struct q_identify_comp - queue identify command completion
 * @status:  status of the command (enum status_code)
 * @ver:     version of identify returned by device
 */
struct q_identify_comp {
	u8     status;
	u8     rsvd;
	__le16 comp_index;
	u8     ver;
	u8     rsvd2[11];
};

/**
 * union q_identity - queue identity information
 *     @version:        queue identify structure version
 *     @supported:      supported queue versions
 *     @features:       queue features
 *     @desc_sz:        descriptor size
 *     @comp_sz:        completion descriptor size
 *     @sg_desc_sz:     sg descriptor size
 *     @max_sg_elems:   maximum number of sg elements
 *     @sg_desc_stride: number of sg elements per descriptor
 */
union q_identity {
	struct {
		u8      version;
		u8      supported;
		u8      rsvd[6];
#define IONIC_QIDENT_F_CQ	0x01	/* queue has completion ring */
#define IONIC_QIDENT_F_SG	0x02	/* queue has scatter/gather ring */
#define IONIC_QIDENT_F_EQ	0x04	/* queue can use event queue */
		__le64  features;
		__le16  desc_sz;
		__le16  comp_sz;
		__le16  sg_desc_sz;
		__le16  max_sg_elems;
		__le16  sg_desc_stride;
	};
	__le32 words[478];
};

/**
 * struct q_init_cmd - Queue init command
 * @opcode:       opcode
 * @type:         Logical queue type
 * @ver:          queue revision
 * @lif_index:    LIF index
 * @index:        (lif, qtype) relative admin queue index
 * @intr_index:   Interrupt control register index, or Event queue index
 * @pid:          Process ID
 * @flags:
 *    IRQ:        Interrupt requested on completion
 *    ENA:        Enable the queue.  If ENA=0 the queue is initialized
 *                but remains disabled, to be later enabled with the
 *                Queue Enable command.  If ENA=1, then queue is
 *                initialized and then enabled.
 *    SG:         Enable Scatter-Gather on the queue.
 *                in number of descs.  The actual ring size is
 *                (1 << ring_size).  For example, to
 *                select a ring size of 64 descriptors write
 *                ring_size = 6.  The minimum ring_size value is 2
 *                for a ring size of 4 descriptors.  The maximum
 *                ring_size value is 16 for a ring size of 64k
 *                descriptors.  Values of ring_size <2 and >16 are
 *                reserved.
 *    EQ:         Enable the Event Queue
 * @cos:          Class of service for this queue.
 * @ring_size:    Queue ring size, encoded as a log2(size)
 * @ring_base:    Queue ring base address
 * @cq_ring_base: Completion queue ring base address
 * @sg_ring_base: Scatter/Gather ring base address
 */
struct q_init_cmd {
	u8     opcode;
	u8     rsvd;
	__le16 lif_index;
	u8     type;
	u8     ver;
	u8     rsvd1[2];
	__le32 index;
	__le16 pid;
	__le16 intr_index;
	__le16 flags;
#define IONIC_QINIT_F_IRQ	0x01	/* Request interrupt on completion */
#define IONIC_QINIT_F_ENA	0x02	/* Enable the queue */
#define IONIC_QINIT_F_SG	0x04	/* Enable scatter/gather on the queue */
#define IONIC_QINIT_F_EQ	0x08	/* Enable event queue */
#define IONIC_QINIT_F_DEBUG 0x80	/* Enable queue debugging */
	u8     cos;
	u8     ring_size;
	__le64 ring_base;
	__le64 cq_ring_base;
	__le64 sg_ring_base;
	u8     rsvd2[20];
};

/**
 * struct q_init_comp - Queue init command completion
 * @status:     The status of the command (enum status_code)
 * @comp_index: The index in the descriptor ring for which this
 *              is the completion.
 * @hw_index:   Hardware Queue ID
 * @hw_type:    Hardware Queue type
 * @color:      Color
 */
struct q_init_comp {
	u8     status;
	u8     rsvd;
	__le16 comp_index;
	__le32 hw_index;
	u8     hw_type;
	u8     rsvd2[6];
	u8     color;
};

/* the device's internal addressing uses up to 52 bits */
#define IONIC_ADDR_LEN		52
#define IONIC_ADDR_MASK		(BIT_ULL(IONIC_ADDR_LEN) - 1)

enum txq_desc_opcode {
	IONIC_TXQ_DESC_OPCODE_CSUM_NONE = 0,
	IONIC_TXQ_DESC_OPCODE_CSUM_PARTIAL = 1,
	IONIC_TXQ_DESC_OPCODE_CSUM_HW = 2,
	IONIC_TXQ_DESC_OPCODE_TSO = 3,
};

/**
 * struct txq_desc - Ethernet Tx queue descriptor format
 * @opcode:       Tx operation, see TXQ_DESC_OPCODE_*:
 *
 *                   IONIC_TXQ_DESC_OPCODE_CSUM_NONE:
 *
 *                      Non-offload send.  No segmentation,
 *                      fragmentation or checksum calc/insertion is
 *                      performed by device; packet is prepared
 *                      to send by software stack and requires
 *                      no further manipulation from device.
 *
 *                   IONIC_TXQ_DESC_OPCODE_CSUM_PARTIAL:
 *
 *                      Offload 16-bit L4 checksum
 *                      calculation/insertion.  The device will
 *                      calculate the L4 checksum value and
 *                      insert the result in the packet's L4
 *                      header checksum field.  The L4 checksum
 *                      is calculated starting at @csum_start bytes
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
 *                      For tunnel encapsulation, @csum_start and
 *                      @csum_offset refer to the inner L4
 *                      header.  Supported tunnels encapsulations
 *                      are: IPIP, GRE, and UDP.  If the @encap
 *                      is clear, no further processing by the
 *                      device is required; software will
 *                      calculate the outer header checksums.  If
 *                      the @encap is set, the device will
 *                      offload the outer header checksums using
 *                      LCO (local checksum offload) (see
 *                      Documentation/networking/checksum-
 *                      offloads.txt for more info).
 *
 *                   IONIC_TXQ_DESC_OPCODE_CSUM_HW:
 *
 *                      Offload 16-bit checksum computation to hardware.
 *                      If @csum_l3 is set then the packet's L3 checksum is
 *                      updated. Similarly, if @csum_l4 is set the the L4
 *                      checksum is updated. If @encap is set then encap header
 *                      checksums are also updated.
 *
 *                   IONIC_TXQ_DESC_OPCODE_TSO:
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
 *                      pseudo header calculated with IP length = 0.
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
 * @flags:
 *                vlan:
 *                    Insert an L2 VLAN header using @vlan_tci.
 *                encap:
 *                    Calculate encap header checksum.
 *                csum_l3:
 *                    Compute L3 header checksum.
 *                csum_l4:
 *                    Compute L4 header checksum.
 *                tso_sot:
 *                    TSO start
 *                tso_eot:
 *                    TSO end
 * @num_sg_elems: Number of scatter-gather elements in SG
 *                descriptor
 * @addr:         First data buffer's DMA address.
 *                (Subsequent data buffers are on txq_sg_desc).
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
 * @mss:          Desired MSS value for TSO.  Only applicable for
 *                TXQ_DESC_OPCODE_TSO.
 * @csum_start:   Offset into inner-most L3 header of checksum
 * @csum_offset:  Offset into inner-most L4 header of checksum
 */

#define IONIC_TXQ_DESC_OPCODE_MASK		0xf
#define IONIC_TXQ_DESC_OPCODE_SHIFT		4
#define IONIC_TXQ_DESC_FLAGS_MASK		0xf
#define IONIC_TXQ_DESC_FLAGS_SHIFT		0
#define IONIC_TXQ_DESC_NSGE_MASK		0xf
#define IONIC_TXQ_DESC_NSGE_SHIFT		8
#define IONIC_TXQ_DESC_ADDR_MASK		(BIT_ULL(IONIC_ADDR_LEN) - 1)
#define IONIC_TXQ_DESC_ADDR_SHIFT		12

/* common flags */
#define IONIC_TXQ_DESC_FLAG_VLAN		0x1
#define IONIC_TXQ_DESC_FLAG_ENCAP		0x2

/* flags for csum_hw opcode */
#define IONIC_TXQ_DESC_FLAG_CSUM_L3		0x4
#define IONIC_TXQ_DESC_FLAG_CSUM_L4		0x8

/* flags for tso opcode */
#define IONIC_TXQ_DESC_FLAG_TSO_SOT		0x4
#define IONIC_TXQ_DESC_FLAG_TSO_EOT		0x8

struct txq_desc {
	__le64  cmd;
	__le16  len;
	union {
		__le16  vlan_tci;
		__le16  hword0;
	};
	union {
		__le16  csum_start;
		__le16  hdr_len;
		__le16  hword1;
	};
	union {
		__le16  csum_offset;
		__le16  mss;
		__le16  hword2;
	};
};

static inline u64 encode_txq_desc_cmd(u8 opcode, u8 flags,
				      u8 nsge, u64 addr)
{
	u64 cmd;

	cmd = (opcode & IONIC_TXQ_DESC_OPCODE_MASK) << IONIC_TXQ_DESC_OPCODE_SHIFT;
	cmd |= (flags & IONIC_TXQ_DESC_FLAGS_MASK) << IONIC_TXQ_DESC_FLAGS_SHIFT;
	cmd |= (nsge & IONIC_TXQ_DESC_NSGE_MASK) << IONIC_TXQ_DESC_NSGE_SHIFT;
	cmd |= (addr & IONIC_TXQ_DESC_ADDR_MASK) << IONIC_TXQ_DESC_ADDR_SHIFT;

	return cmd;
};

static inline void decode_txq_desc_cmd(u64 cmd, u8 *opcode, u8 *flags,
				       u8 *nsge, u64 *addr)
{
	*opcode = (cmd >> IONIC_TXQ_DESC_OPCODE_SHIFT) & IONIC_TXQ_DESC_OPCODE_MASK;
	*flags = (cmd >> IONIC_TXQ_DESC_FLAGS_SHIFT) & IONIC_TXQ_DESC_FLAGS_MASK;
	*nsge = (cmd >> IONIC_TXQ_DESC_NSGE_SHIFT) & IONIC_TXQ_DESC_NSGE_MASK;
	*addr = (cmd >> IONIC_TXQ_DESC_ADDR_SHIFT) & IONIC_TXQ_DESC_ADDR_MASK;
};

/**
 * struct txq_sg_elem - Transmit scatter-gather (SG) descriptor element
 * @addr:      DMA address of SG element data buffer
 * @len:       Length of SG element data buffer, in bytes
 */
struct txq_sg_elem {
	__le64 addr;
	__le16 len;
	__le16 rsvd[3];
};

/**
 * struct txq_sg_desc - Transmit scatter-gather (SG) list
 * @elems:     Scattter-gather elements
 */
struct txq_sg_desc {
#define IONIC_TX_MAX_SG_ELEMS		8
#define IONIC_TX_SG_DESC_STRIDE		8
	struct txq_sg_elem elems[IONIC_TX_MAX_SG_ELEMS];
};

struct txq_sg_desc_v1 {
#define IONIC_TX_MAX_SG_ELEMS_V1		15
#define IONIC_TX_SG_DESC_STRIDE_V1		16
	struct txq_sg_elem elems[IONIC_TX_SG_DESC_STRIDE_V1];
};

/**
 * struct txq_comp - Ethernet transmit queue completion descriptor
 * @status:     The status of the command (enum status_code)
 * @comp_index: The index in the descriptor ring for which this
 *                 is the completion.
 * @color:      Color bit.
 */
struct txq_comp {
	u8     status;
	u8     rsvd;
	__le16 comp_index;
	u8     rsvd2[11];
	u8     color;
};

enum rxq_desc_opcode {
	RXQ_DESC_OPCODE_SIMPLE = 0,
	RXQ_DESC_OPCODE_SG = 1,
};

/**
 * struct rxq_desc - Ethernet Rx queue descriptor format
 * @opcode:       Rx operation, see RXQ_DESC_OPCODE_*:
 *
 *                   RXQ_DESC_OPCODE_SIMPLE:
 *
 *                      Receive full packet into data buffer
 *                      starting at @addr.  Results of
 *                      receive, including actual bytes received,
 *                      are recorded in Rx completion descriptor.
 *
 * @len:          Data buffer's length, in bytes.
 * @addr:         Data buffer's DMA address
 */
struct rxq_desc {
	u8     opcode;
	u8     rsvd[5];
	__le16 len;
	__le64 addr;
};	

/**
 * struct rxq_sg_elem - Receive scatter-gather (SG) descriptor element
 * @addr:      DMA address of SG element data buffer
 * @len:       Length of SG element data buffer, in bytes
 */
struct rxq_sg_elem {
	__le64 addr;
	__le16 len;
	__le16 rsvd[3];
};

/**
 * struct rxq_sg_desc - Receive scatter-gather (SG) list
 * @elems:     Scattter-gather elements
 */
struct rxq_sg_desc {
#define IONIC_RX_MAX_SG_ELEMS		8
#define IONIC_RX_SG_DESC_STRIDE		8
	struct rxq_sg_elem elems[IONIC_RX_SG_DESC_STRIDE];
};

/**
 * struct rxq_comp - Ethernet receive queue completion descriptor
 * @status:       The status of the command (enum status_code)
 * @num_sg_elems: Number of SG elements used by this descriptor
 * @comp_index:   The index in the descriptor ring for which this
 *                is the completion.
 * @rss_hash:     32-bit RSS hash
 * @csum:         16-bit sum of the packet's L2 payload.
 *                If the packet's L2 payload is odd length, an extra
 *                zero-value byte is included in the @csum calculation but
 *                not included in @len.
 * @vlan_tci:     VLAN tag stripped from the packet.  Valid if @VLAN is
 *                set.  Includes .1p and .1q tags.
 * @len:          Received packet length, in bytes.  Excludes FCS.
 * @csum_calc     L2 payload checksum is computed or not
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
 * @VLAN:         VLAN header was stripped and placed in @vlan_tci.
 * @pkt_type:     Packet type
 * @color:        Color bit.
 */
struct rxq_comp {
	u8     status;
	u8     num_sg_elems;
	__le16 comp_index;
	__le32 rss_hash;
	__le16 csum;
	__le16 vlan_tci;
	__le16 len;
	u8     csum_flags;
#define IONIC_RXQ_COMP_CSUM_F_TCP_OK	0x01
#define IONIC_RXQ_COMP_CSUM_F_TCP_BAD	0x02
#define IONIC_RXQ_COMP_CSUM_F_UDP_OK	0x04
#define IONIC_RXQ_COMP_CSUM_F_UDP_BAD	0x08
#define IONIC_RXQ_COMP_CSUM_F_IP_OK		0x10
#define IONIC_RXQ_COMP_CSUM_F_IP_BAD	0x20
#define IONIC_RXQ_COMP_CSUM_F_VLAN		0x40
#define IONIC_RXQ_COMP_CSUM_F_CALC		0x80

#define IONIC_RXQ_COMP_VALID_CSUM_FLAGS 0x3f

	u8     pkt_type_color;
#define IONIC_RXQ_COMP_PKT_TYPE_MASK	0x7f
};

enum pkt_type {
	PKT_TYPE_NON_IP     = 0x000,
	PKT_TYPE_IPV4       = 0x001,
	PKT_TYPE_IPV4_TCP   = 0x003,
	PKT_TYPE_IPV4_UDP   = 0x005,
	PKT_TYPE_IPV6       = 0x008,
	PKT_TYPE_IPV6_TCP   = 0x018,
	PKT_TYPE_IPV6_UDP   = 0x028,
};

enum eth_hw_features {
	ETH_HW_VLAN_TX_TAG	= BIT(0),
	ETH_HW_VLAN_RX_STRIP	= BIT(1),
	ETH_HW_VLAN_RX_FILTER	= BIT(2),
	ETH_HW_RX_HASH		= BIT(3),
	ETH_HW_RX_CSUM		= BIT(4),
	ETH_HW_TX_SG		= BIT(5),
	ETH_HW_RX_SG		= BIT(6),
	ETH_HW_TX_CSUM		= BIT(7),
	ETH_HW_TSO		= BIT(8),
	ETH_HW_TSO_IPV6		= BIT(9),
	ETH_HW_TSO_ECN		= BIT(10),
	ETH_HW_TSO_GRE		= BIT(11),
	ETH_HW_TSO_GRE_CSUM	= BIT(12),
	ETH_HW_TSO_IPXIP4	= BIT(13),
	ETH_HW_TSO_IPXIP6	= BIT(14),
	ETH_HW_TSO_UDP		= BIT(15),
	ETH_HW_TSO_UDP_CSUM	= BIT(16),
};

/**
 * struct q_control_cmd - Queue control command
 * @opcode:     opcode
 * @type:       Queue type
 * @lif_index:  LIF index
 * @index:      Queue index
 * @oper:       Operation (enum q_control_oper)
 */
struct q_control_cmd {
	u8     opcode;
	u8     type;
	__le16 lif_index;
	__le32 index;
	u8     oper;
	u8     rsvd[55];
};

typedef struct admin_comp q_control_comp;

enum q_control_oper {
	IONIC_Q_DISABLE		= 0,
	IONIC_Q_ENABLE		= 1,
	IONIC_Q_HANG_RESET	= 2,
};

/**
 * Physical connection type
 */
enum phy_type {
	PHY_TYPE_NONE	= 0,
	PHY_TYPE_COPPER	= 1,
	PHY_TYPE_FIBER	= 2,
};

/**
 * Transceiver status
 */
enum xcvr_state {
	XCVR_STATE_REMOVED	   = 0,
	XCVR_STATE_INSERTED	   = 1,
	XCVR_STATE_PENDING	   = 2,
	XCVR_STATE_SPROM_READ	   = 3,
	XCVR_STATE_SPROM_READ_ERR  = 4,
};

/**
 * Supported link modes
 */
enum xcvr_pid {
	XCVR_PID_UNKNOWN           = 0,

	/* CU */
	XCVR_PID_QSFP_100G_CR4     = 1,
	XCVR_PID_QSFP_40GBASE_CR4  = 2,
	XCVR_PID_SFP_25GBASE_CR_S  = 3,
	XCVR_PID_SFP_25GBASE_CR_L  = 4,
	XCVR_PID_SFP_25GBASE_CR_N  = 5,

	/* Fiber */
	XCVR_PID_QSFP_100G_AOC    = 50,
	XCVR_PID_QSFP_100G_ACC    = 51,
	XCVR_PID_QSFP_100G_SR4    = 52,
	XCVR_PID_QSFP_100G_LR4    = 53,
	XCVR_PID_QSFP_100G_ER4    = 54,
	XCVR_PID_QSFP_40GBASE_ER4 = 55,
	XCVR_PID_QSFP_40GBASE_SR4 = 56,
	XCVR_PID_QSFP_40GBASE_LR4 = 57,
	XCVR_PID_QSFP_40GBASE_AOC = 58,
	XCVR_PID_SFP_25GBASE_SR   = 59,
	XCVR_PID_SFP_25GBASE_LR   = 60,
	XCVR_PID_SFP_25GBASE_ER   = 61,
	XCVR_PID_SFP_25GBASE_AOC  = 62,
	XCVR_PID_SFP_10GBASE_SR   = 63,
	XCVR_PID_SFP_10GBASE_LR   = 64,
	XCVR_PID_SFP_10GBASE_LRM  = 65,
	XCVR_PID_SFP_10GBASE_ER   = 66,
	XCVR_PID_SFP_10GBASE_AOC  = 67,
	XCVR_PID_SFP_10GBASE_CU   = 68,
	XCVR_PID_QSFP_100G_CWDM4  = 69,
	XCVR_PID_QSFP_100G_PSM4   = 70,
};

/**
 * Port types
 */
enum PortType {
	PORT_TYPE_NONE = 0,	/* port type not configured */
	PORT_TYPE_ETH  = 1,	/* port carries ethernet traffic (inband) */
	PORT_TYPE_MGMT = 2,	/* port carries mgmt traffic (out-of-band) */
};

/**
 * Port config state
 */
enum PortAdminState {
	PORT_ADMIN_STATE_NONE = 0,	/* port admin state not configured */
	PORT_ADMIN_STATE_DOWN = 1,	/* port is admin disabled */
	PORT_ADMIN_STATE_UP   = 2,	/* port is admin enabled */
};

/**
 * Port operational status
 */
enum port_oper_status {
	PORT_OPER_STATUS_NONE  = 0,	/* port is disabled */
	PORT_OPER_STATUS_UP    = 1,	/* port is linked up */
	PORT_OPER_STATUS_DOWN  = 2,	/* port link status is down */
};

/**
 * Ethernet Forward error correction (fec) modes
 */
enum port_fec_type {
	PORT_FEC_TYPE_NONE = 0,		/* Disabled */
	PORT_FEC_TYPE_FC   = 1,		/* FireCode */
	PORT_FEC_TYPE_RS   = 2,		/* ReedSolomon */
};

/**
 * Ethernet pause (flow control) modes
 */
enum port_pause_type {
	PORT_PAUSE_TYPE_NONE = 0,	/* Disable Pause */
	PORT_PAUSE_TYPE_LINK = 1,	/* Link level pause */
	PORT_PAUSE_TYPE_PFC  = 2,	/* Priority-Flow control */
};

/**
 * Loopback modes
 */
enum port_loopback_mode {
	PORT_LOOPBACK_MODE_NONE = 0,	/* Disable loopback */
	PORT_LOOPBACK_MODE_MAC  = 1,	/* MAC loopback */
	PORT_LOOPBACK_MODE_PHY  = 2,	/* PHY/Serdes loopback */
};

/**
 * Transceiver Status information
 * @state:    Transceiver status (enum xcvr_state)
 * @phy:      Physical connection type (enum phy_type)
 * @pid:      Transceiver link mode (enum pid)
 * @sprom:    Transceiver sprom contents
 */
struct xcvr_status {
	u8     state;
	u8     phy;
	__le16 pid;
	u8     sprom[256];
};

/**
 * Port configuration
 * @speed:              port speed (in Mbps)
 * @mtu:                mtu
 * @state:              port admin state (enum port_admin_state)
 * @an_enable:          autoneg enable
 * @fec_type:           fec type (enum port_fec_type)
 * @pause_type:         pause type (enum port_pause_type)
 * @loopback_mode:      loopback mode (enum port_loopback_mode)
 */
union port_config {
	struct {
#define IONIC_SPEED_100G	100000	/* 100G in Mbps */
#define IONIC_SPEED_50G		50000	/* 50G in Mbps */
#define IONIC_SPEED_40G		40000	/* 40G in Mbps */
#define IONIC_SPEED_25G		25000	/* 25G in Mbps */
#define IONIC_SPEED_10G		10000	/* 10G in Mbps */
#define IONIC_SPEED_1G		1000	/* 1G in Mbps */
		__le32 speed;
		__le32 mtu;
		u8     state;
		u8     an_enable;
		u8     fec_type;
#define IONIC_PAUSE_TYPE_MASK		0x0f
#define IONIC_PAUSE_FLAGS_MASK		0xf0
#define IONIC_PAUSE_F_TX		0x10
#define IONIC_PAUSE_F_RX		0x20
		u8     pause_type;
		u8     loopback_mode;
	};
	__le32 words[64];
};

/**
 * Port Status information
 * @status:             link status (enum port_oper_status)
 * @id:                 port id
 * @speed:              link speed (in Mbps)
 * @link_down_count:    number of times link when from up to down
 * @xcvr:               tranceiver status
 */
struct port_status {
	__le32 id;
	__le32 speed;
	u8     status;
	__le16 link_down_count;
	u8     rsvd[49];
	struct xcvr_status  xcvr;
};

/**
 * struct port_identify_cmd - Port identify command
 * @opcode:     opcode
 * @index:      port index
 * @ver:        Highest version of identify supported by driver
 */
struct port_identify_cmd {
	u8 opcode;
	u8 index;
	u8 ver;
	u8 rsvd[61];
};

/**
 * struct port_identify_comp - Port identify command completion
 * @status: The status of the command (enum status_code)
 * @ver:    Version of identify returned by device
 */
struct port_identify_comp {
	u8 status;
	u8 ver;
	u8 rsvd[14];
};

/**
 * struct port_init_cmd - Port initialization command
 * @opcode:     opcode
 * @index:      port index
 * @info_pa:    destination address for port info (struct port_info)
 */
struct port_init_cmd {
	u8     opcode;
	u8     index;
	u8     rsvd[6];
	__le64 info_pa;
	u8     rsvd2[48];
};

/**
 * struct port_init_comp - Port initialization command completion
 * @status: The status of the command (enum status_code)
 */
struct port_init_comp {
	u8 status;
	u8 rsvd[15];
};

/**
 * struct port_reset_cmd - Port reset command
 * @opcode:     opcode
 * @index:      port index
 */
struct port_reset_cmd {
	u8 opcode;
	u8 index;
	u8 rsvd[62];
};

/**
 * struct port_reset_comp - Port reset command completion
 * @status: The status of the command (enum status_code)
 */
struct port_reset_comp {
	u8 status;
	u8 rsvd[15];
};

/**
 * enum stats_ctl_cmd - List of commands for stats control
 */
enum stats_ctl_cmd {
	STATS_CTL_RESET             = 0,
};

/**
 * enum ionic_port_attr - List of device attributes
 */
enum ionic_port_attr {
	IONIC_PORT_ATTR_STATE		= 0,
	IONIC_PORT_ATTR_SPEED		= 1,
	IONIC_PORT_ATTR_MTU		= 2,
	IONIC_PORT_ATTR_AUTONEG		= 3,
	IONIC_PORT_ATTR_FEC		= 4,
	IONIC_PORT_ATTR_PAUSE		= 5,
	IONIC_PORT_ATTR_LOOPBACK	= 6,
	IONIC_PORT_ATTR_STATS_CTRL	= 7,
};

/**
 * struct port_setattr_cmd - Set port attributes on the NIC
 * @opcode:     Opcode
 * @index:      port index
 * @attr:       Attribute type (enum ionic_port_attr)
 */
struct port_setattr_cmd {
	u8     opcode;
	u8     index;
	u8     attr;
	u8     rsvd;
	union {
		u8      state;
		__le32  speed;
		__le32  mtu;
		u8      an_enable;
		u8      fec_type;
		u8      pause_type;
		u8      loopback_mode;
		u8      stats_ctl;
		u8      rsvd2[60];
	};
};

/**
 * struct port_setattr_comp - Port set attr command completion
 * @status:     The status of the command (enum status_code)
 * @color:      Color bit
 */
struct port_setattr_comp {
	u8     status;
	u8     rsvd[14];
	u8     color;
};

/**
 * struct port_getattr_cmd - Get port attributes from the NIC
 * @opcode:     Opcode
 * @index:      port index
 * @attr:       Attribute type (enum ionic_port_attr)
 */
struct port_getattr_cmd {
	u8     opcode;
	u8     index;
	u8     attr;
	u8     rsvd[61];
};

/**
 * struct port_getattr_comp - Port get attr command completion
 * @status:     The status of the command (enum status_code)
 * @color:      Color bit
 */
struct port_getattr_comp {
	u8     status;
	u8     rsvd[3];
	union {
		u8      state;
		__le32  speed;
		__le32  mtu;
		u8      an_enable;
		u8      fec_type;
		u8      pause_type;
		u8      loopback_mode;
		u8      rsvd2[11];
	};
	u8     color;
};

/**
 * struct lif_status - Lif status register
 * @eid:             most recent NotifyQ event id
 * @port_num:        port the lif is connected to
 * @link_status:     port status (enum port_oper_status)
 * @link_speed:      speed of link in Mbps
 * @link_down_count: number of times link when from up to down
 */
struct lif_status {
	__le64 eid;
	u8     port_num;
	u8     rsvd;
	__le16 link_status;
	__le32 link_speed;		/* units of 1Mbps: eg 10000 = 10Gbps */
	__le16 link_down_count;
	u8      rsvd2[46];
};

/**
 * struct lif_reset_cmd - LIF reset command
 * @opcode:    opcode
 * @index:     LIF index
 */
struct lif_reset_cmd {
	u8     opcode;
	u8     rsvd;
	__le16 index;
	__le32 rsvd2[15];
};

typedef struct admin_comp lif_reset_comp;

enum dev_state {
	IONIC_DEV_DISABLE	= 0,
	IONIC_DEV_ENABLE	= 1,
	IONIC_DEV_HANG_RESET	= 2,
};

/**
 * enum dev_attr - List of device attributes
 */
enum dev_attr {
	IONIC_DEV_ATTR_STATE    = 0,
	IONIC_DEV_ATTR_NAME     = 1,
	IONIC_DEV_ATTR_FEATURES = 2,
};

/**
 * struct dev_setattr_cmd - Set Device attributes on the NIC
 * @opcode:     Opcode
 * @attr:       Attribute type (enum dev_attr)
 * @state:      Device state (enum dev_state)
 * @name:       The bus info, e.g. PCI slot-device-function, 0 terminated
 * @features:   Device features
 */
struct dev_setattr_cmd {
	u8     opcode;
	u8     attr;
	__le16 rsvd;
	union {
		u8      state;
		char    name[IONIC_IFNAMSIZ];
		__le64  features;
		u8      rsvd2[60];
	};
};

/**
 * struct dev_setattr_comp - Device set attr command completion
 * @status:     The status of the command (enum status_code)
 * @features:   Device features
 * @color:      Color bit
 */
struct dev_setattr_comp {
	u8     status;
	u8     rsvd[3];
	union {
		__le64  features;
		u8      rsvd2[11];
	};
	u8     color;
};

/**
 * struct dev_getattr_cmd - Get Device attributes from the NIC
 * @opcode:     opcode
 * @attr:       Attribute type (enum dev_attr)
 */
struct dev_getattr_cmd {
	u8     opcode;
	u8     attr;
	u8     rsvd[62];
};

/**
 * struct dev_setattr_comp - Device set attr command completion
 * @status:     The status of the command (enum status_code)
 * @features:   Device features
 * @color:      Color bit
 */
struct dev_getattr_comp {
	u8     status;
	u8     rsvd[3];
	union {
		__le64  features;
		u8      rsvd2[11];
	};
	u8     color;
};

/**
 * RSS parameters
 */
#define IONIC_RSS_HASH_KEY_SIZE		40

enum rss_hash_types {
	IONIC_RSS_TYPE_IPV4	= BIT(0),
	IONIC_RSS_TYPE_IPV4_TCP	= BIT(1),
	IONIC_RSS_TYPE_IPV4_UDP	= BIT(2),
	IONIC_RSS_TYPE_IPV6	= BIT(3),
	IONIC_RSS_TYPE_IPV6_TCP	= BIT(4),
	IONIC_RSS_TYPE_IPV6_UDP	= BIT(5),
};

/**
 * enum lif_attr - List of LIF attributes
 */
enum lif_attr {
	IONIC_LIF_ATTR_STATE        = 0,
	IONIC_LIF_ATTR_NAME         = 1,
	IONIC_LIF_ATTR_MTU          = 2,
	IONIC_LIF_ATTR_MAC          = 3,
	IONIC_LIF_ATTR_FEATURES     = 4,
	IONIC_LIF_ATTR_RSS          = 5,
	IONIC_LIF_ATTR_STATS_CTRL   = 6,
};

/**
 * struct lif_setattr_cmd - Set LIF attributes on the NIC
 * @opcode:     Opcode
 * @type:       Attribute type (enum lif_attr)
 * @index:      LIF index
 * @state:      lif state (enum lif_state)
 * @name:       The netdev name string, 0 terminated
 * @mtu:        Mtu
 * @mac:        Station mac
 * @features:   Features (enum eth_hw_features)
 * @rss:        RSS properties
 *              @types:     The hash types to enable (see rss_hash_types).
 *              @key:       The hash secret key.
 *              @addr:      Address for the indirection table shared memory.
 * @stats_ctl:  stats control commands (enum stats_ctl_cmd)
 */
struct lif_setattr_cmd {
	u8     opcode;
	u8     attr;
	__le16 index;
	union {
		u8      state;
		char    name[IONIC_IFNAMSIZ];
		__le32  mtu;
		u8      mac[6];
		__le64  features;
		struct {
			__le16 types;
			u8     key[IONIC_RSS_HASH_KEY_SIZE];
			u8     rsvd[6];
			__le64 addr;
		} rss;
		u8 stats_ctl;
		u8      rsvd[60];
	};
};

/**
 * struct lif_setattr_comp - LIF set attr command completion
 * @status:     The status of the command (enum status_code)
 * @comp_index: The index in the descriptor ring for which this
 *              is the completion.
 * @features:   features (enum eth_hw_features)
 * @color:      Color bit
 */
struct lif_setattr_comp {
	u8     status;
	u8     rsvd;
	__le16 comp_index;
	union {
		__le64  features;
		u8      rsvd2[11];
	};
	u8     color;
};

/**
 * struct lif_getattr_cmd - Get LIF attributes from the NIC
 * @opcode:     Opcode
 * @attr:       Attribute type (enum lif_attr)
 * @index:      LIF index
 */
struct lif_getattr_cmd {
	u8     opcode;
	u8     attr;
	__le16 index;
	u8     rsvd[60];
};

/**
 * struct lif_getattr_comp - LIF get attr command completion
 * @status:     The status of the command (enum status_code)
 * @comp_index: The index in the descriptor ring for which this
 *              is the completion.
 * @state:      lif state (enum lif_state)
 * @name:       The netdev name string, 0 terminated
 * @mtu:        Mtu
 * @mac:        Station mac
 * @features:   Features (enum eth_hw_features)
 * @color:      Color bit
 */
struct lif_getattr_comp {
	u8     status;
	u8     rsvd;
	__le16 comp_index;
	union {
		u8      state;
		//char    name[IONIC_IFNAMSIZ];
		__le32  mtu;
		u8      mac[6];
		__le64  features;
		u8      rsvd2[11];
	};
	u8     color;
};

enum rx_mode {
	RX_MODE_F_UNICAST	= BIT(0),
	RX_MODE_F_MULTICAST	= BIT(1),
	RX_MODE_F_BROADCAST	= BIT(2),
	RX_MODE_F_PROMISC	= BIT(3),
	RX_MODE_F_ALLMULTI	= BIT(4),
};

/**
 * struct rx_mode_set_cmd - Set LIF's Rx mode command
 * @opcode:     opcode
 * @lif_index:  LIF index
 * @rx_mode:    Rx mode flags:
 *                  RX_MODE_F_UNICAST: Accept known unicast packets.
 *                  RX_MODE_F_MULTICAST: Accept known multicast packets.
 *                  RX_MODE_F_BROADCAST: Accept broadcast packets.
 *                  RX_MODE_F_PROMISC: Accept any unicast packets.
 *                  RX_MODE_F_ALLMULTI: Accept any multicast packets.
 */
struct rx_mode_set_cmd {
	u8     opcode;
	u8     rsvd;
	__le16 lif_index;
	__le16 rx_mode;
	__le16 rsvd2[29];
};

typedef struct admin_comp rx_mode_set_comp;

enum rx_filter_match_type {
	RX_FILTER_MATCH_VLAN = 0,
	RX_FILTER_MATCH_MAC,
	RX_FILTER_MATCH_MAC_VLAN,
};

/**
 * struct rx_filter_add_cmd - Add LIF Rx filter command
 * @opcode:     opcode
 * @qtype:      Queue type
 * @lif_index:  LIF index
 * @qid:        Queue ID
 * @match:      Rx filter match type.  (See RX_FILTER_MATCH_xxx)
 * @vlan:       VLAN ID
 * @addr:       MAC address (network-byte order)
 */
struct rx_filter_add_cmd {
	u8     opcode;
	u8     qtype;
	__le16 lif_index;
	__le32 qid;
	__le16 match;
	union {
		struct {
			__le16 vlan;
		} vlan;
		struct {
			u8     addr[6];
		} mac;
		struct {
			__le16 vlan;
			u8     addr[6];
		} mac_vlan;
		u8 rsvd[54];
	};
};

/**
 * struct rx_filter_add_comp - Add LIF Rx filter command completion
 * @status:     The status of the command (enum status_code)
 * @comp_index: The index in the descriptor ring for which this
 *              is the completion.
 * @filter_id:  Filter ID
 * @color:      Color bit.
 */
struct rx_filter_add_comp {
	u8     status;
	u8     rsvd;
	__le16 comp_index;
	__le32 filter_id;
	u8     rsvd2[7];
	u8     color;
};

/**
 * struct rx_filter_del_cmd - Delete LIF Rx filter command
 * @opcode:     opcode
 * @lif_index:  LIF index
 * @filter_id:  Filter ID
 */
struct rx_filter_del_cmd {
	u8     opcode;
	u8     rsvd;
	__le16 lif_index;
	__le32 filter_id;
	u8     rsvd2[56];
};

typedef struct admin_comp rx_filter_del_comp;

/**
 * struct qos_identify_cmd - QoS identify command
 * @opcode:    opcode
 * @ver:     Highest version of identify supported by driver
 *
 */
struct qos_identify_cmd {
	u8 opcode;
	u8 ver;
	u8 rsvd[62];
};

/**
 * struct qos_identify_comp - QoS identify command completion
 * @status: The status of the command (enum status_code)
 * @ver:    Version of identify returned by device
 */
struct qos_identify_comp {
	u8 status;
	u8 ver;
	u8 rsvd[14];
};

#define IONIC_QOS_CLASS_MAX		7
#define IONIC_QOS_CLASS_NAME_SZ		32
#define IONIC_QOS_DSCP_MAX_VALUES	64

/**
 * enum qos_class
 */
enum qos_class {
	QOS_CLASS_DEFAULT		= 0,
	QOS_CLASS_USER_DEFINED_1	= 1,
	QOS_CLASS_USER_DEFINED_2	= 2,
	QOS_CLASS_USER_DEFINED_3	= 3,
	QOS_CLASS_USER_DEFINED_4	= 4,
	QOS_CLASS_USER_DEFINED_5	= 5,
	QOS_CLASS_USER_DEFINED_6	= 6,
};

/**
 * enum qos_class_type - Traffic classification criteria
 */
enum qos_class_type {
	QOS_CLASS_TYPE_NONE	= 0,
	QOS_CLASS_TYPE_PCP	= 1,	/* Dot1Q pcp */
	QOS_CLASS_TYPE_DSCP	= 2,	/* IP dscp */
};

/**
 * enum qos_sched_type - Qos class scheduling type
 */
enum qos_sched_type {
	QOS_SCHED_TYPE_STRICT	= 0,	/* Strict priority */
	QOS_SCHED_TYPE_DWRR	= 1,	/* Deficit weighted round-robin */
};

/**
 * union qos_config - Qos configuration structure
 * @flags:		Configuration flags
 *	IONIC_QOS_CONFIG_F_ENABLE		enable
 *	IONIC_QOS_CONFIG_F_DROP			drop/nodrop
 *	IONIC_QOS_CONFIG_F_RW_DOT1Q_PCP		enable dot1q pcp rewrite
 *	IONIC_QOS_CONFIG_F_RW_IP_DSCP		enable ip dscp rewrite
 * @sched_type:		Qos class scheduling type (enum qos_sched_type)
 * @class_type:		Qos class type (enum qos_class_type)
 * @pause_type:		Qos pause type (enum qos_pause_type)
 * @name:		Qos class name
 * @mtu:		MTU of the class
 * @pfc_dot1q_pcp:	Pcp value for pause frames (valid iff F_NODROP)
 * @dwrr_weight:	Qos class scheduling weight
 * @strict_rlmt:	Rate limit for strict priority scheduling
 * @rw_dot1q_pcp:	Rewrite dot1q pcp to this value	(valid iff F_RW_DOT1Q_PCP)
 * @rw_ip_dscp:		Rewrite ip dscp to this value	(valid iff F_RW_IP_DSCP)
 * @dot1q_pcp:		Dot1q pcp value
 * @ndscp:		Number of valid dscp values in the ip_dscp field
 * @ip_dscp:		IP dscp values
 */
union qos_config {
	struct {
#define IONIC_QOS_CONFIG_F_ENABLE		BIT(0)
#define IONIC_QOS_CONFIG_F_DROP			BIT(1)
#define IONIC_QOS_CONFIG_F_RW_DOT1Q_PCP		BIT(2)
#define IONIC_QOS_CONFIG_F_RW_IP_DSCP		BIT(3)
		u8      flags;
		u8      sched_type;
		u8      class_type;
		u8      pause_type;
		char    name[IONIC_QOS_CLASS_NAME_SZ];
		__le32  mtu;
		/* flow control */
		u8      pfc_cos;
		/* scheduler */
		union {
			u8      dwrr_weight;
			__le64  strict_rlmt;
		};
		/* marking */
		union {
			u8      rw_dot1q_pcp;
			u8      rw_ip_dscp;
		};
		/* classification */
		union {
			u8      dot1q_pcp;
			struct {
				u8      ndscp;
				u8      ip_dscp[IONIC_QOS_DSCP_MAX_VALUES];
			};
		};
	};
	__le32  words[64];
};

/**
 * union qos_identity - QoS identity structure
 * @version:	Version of the identify structure
 * @type:	QoS system type
 * @nclasses:	Number of usable QoS classes
 * @config:	Current configuration of classes
 */
union qos_identity {
	struct {
		u8     version;
		u8     type;
		u8     rsvd[62];
		union  qos_config config[IONIC_QOS_CLASS_MAX];
	};
	__le32 words[478];
};

/**
 * struct qos_init_cmd - QoS config init command
 * @opcode:	Opcode
 * @group:	Qos class id
 * @info_pa:	destination address for qos info
 */
struct qos_init_cmd {
	u8     opcode;
	u8     group;
	u8     rsvd[6];
	__le64 info_pa;
	u8     rsvd1[48];
};

typedef struct admin_comp qos_init_comp;

/**
 * struct qos_reset_cmd - Qos config reset command
 * @opcode:	Opcode
 */
struct qos_reset_cmd {
	u8    opcode;
	u8    group;
	u8    rsvd[62];
};

typedef struct admin_comp qos_reset_comp;

/**
 * struct fw_download_cmd - Firmware download command
 * @opcode:	opcode
 * @addr:	dma address of the firmware buffer
 * @offset:	offset of the firmware buffer within the full image
 * @length:	number of valid bytes in the firmware buffer
 */
struct fw_download_cmd {
	u8     opcode;
	u8     rsvd[3];
	__le32 offset;
	__le64 addr;
	__le32 length;
};

typedef struct admin_comp fw_download_comp;

enum fw_control_oper {
	IONIC_FW_RESET		= 0,	/* Reset firmware */
	IONIC_FW_INSTALL	= 1,	/* Install firmware */
	IONIC_FW_ACTIVATE	= 2,	/* Activate firmware */
};

/**
 * struct fw_control_cmd - Firmware control command
 * @opcode:    opcode
 * @oper:      firmware control operation (enum fw_control_oper)
 * @slot:      slot to activate
 */
struct fw_control_cmd {
	u8  opcode;
	u8  rsvd[3];
	u8  oper;
	u8  slot;
	u8  rsvd1[58];
};

/**
 * struct fw_control_comp - Firmware control copletion
 * @opcode:    opcode
 * @slot:      slot where the firmware was installed
 */
struct fw_control_comp {
	u8     status;
	u8     rsvd;
	__le16 comp_index;
	u8     slot;
	u8     rsvd1[10];
	u8     color;
};

/******************************************************************
 ******************* RDMA Commands ********************************
 ******************************************************************/

/**
 * struct rdma_reset_cmd - Reset RDMA LIF cmd
 * @opcode:        opcode
 * @lif_index:     lif index
 *
 * There is no rdma specific dev command completion struct.  Completion uses
 * the common struct admin_comp.  Only the status is indicated.  Nonzero status
 * means the LIF does not support rdma.
 **/
struct rdma_reset_cmd {
	u8     opcode;
	u8     rsvd;
	__le16 lif_index;
	u8     rsvd2[60];
};

/**
 * struct rdma_queue_cmd - Create RDMA Queue command
 * @opcode:        opcode, 52, 53
 * @lif_index      lif index
 * @qid_ver:       (qid | (rdma version << 24))
 * @cid:           intr, eq_id, or cq_id
 * @dbid:          doorbell page id
 * @depth_log2:    log base two of queue depth
 * @stride_log2:   log base two of queue stride
 * @dma_addr:      address of the queue memory
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
	u8     opcode;
	u8     rsvd;
	__le16 lif_index;
	__le32 qid_ver;
	__le32 cid;
	__le16 dbid;
	u8     depth_log2;
	u8     stride_log2;
	__le64 dma_addr;
	u8     rsvd2[40];
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
	__le64 eid;
	__le16 ecode;
	u8     data[54];
};

/**
 * struct link_change_event
 * @eid:		event number
 * @ecode:		event code = EVENT_OPCODE_LINK_CHANGE
 * @link_status:	link up or down, with error bits (enum port_status)
 * @link_speed:		speed of the network link
 *
 * Sent when the network link state changes between UP and DOWN
 */
struct link_change_event {
	__le64 eid;
	__le16 ecode;
	__le16 link_status;
	__le32 link_speed;	/* units of 1Mbps: e.g. 10000 = 10Gbps */
	u8     rsvd[48];
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
	__le64 eid;
	__le16 ecode;
	u8     reset_code;
	u8     state;
	u8     rsvd[52];
};

/**
 * struct heartbeat_event
 * @eid:	event number
 * @ecode:	event code = EVENT_OPCODE_HEARTBEAT
 *
 * Sent periodically by the NIC to indicate continued health
 */
struct heartbeat_event {
	__le64 eid;
	__le16 ecode;
	u8     rsvd[54];
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
	__le64 eid;
	__le16 ecode;
	u8     data[54];
};

/**
 * struct xcvr_event
 * @eid:	event number
 * @ecode:	event code = EVENT_OPCODE_XCVR
 *
 * Transceiver change event
 */
struct xcvr_event {
	__le64 eid;
	__le16 ecode;
	u8     rsvd[54];
};

/**
 * struct port_identity - port identity structure
 * @version:        identity structure version
 * @type:           type of port (enum port_type)
 * @num_lanes:      number of lanes for the port
 * @autoneg:        autoneg supported
 * @min_frame_size: minimum frame size supported
 * @max_frame_size: maximum frame size supported
 * @fec_type:       supported fec types
 * @pause_type:     supported pause types
 * @loopback_mode:  supported loopback mode
 * @speeds:         supported speeds
 * @config:         current port configuration
 */
union port_identity {
	struct {
		u8     version;
		u8     type;
		u8     num_lanes;
		u8     autoneg;
		__le32 min_frame_size;
		__le32 max_frame_size;
		u8     fec_type[4];
		u8     pause_type[2];
		u8     loopback_mode[2];
		__le32 speeds[16];
		u8     rsvd2[44];
		union port_config config;
	};
	__le32 words[478];
};

/**
 * struct port_info - port info structure
 * @port_status:     port status
 * @port_stats:      port stats
 */
struct port_info {
	union port_config config;
	struct port_status status;
	struct port_stats stats;
};

/**
 * struct lif_info - lif info structure
 */
struct lif_info {
	union lif_config config;
	struct lif_status status;
	struct lif_stats stats;
};

union dev_cmd {
	u32 words[16];
	struct admin_cmd cmd;
	struct nop_cmd nop;

	struct dev_identify_cmd identify;
	struct dev_init_cmd init;
	struct dev_reset_cmd reset;
	struct dev_getattr_cmd getattr;
	struct dev_setattr_cmd setattr;

	struct port_identify_cmd port_identify;
	struct port_init_cmd port_init;
	struct port_reset_cmd port_reset;
	struct port_getattr_cmd port_getattr;
	struct port_setattr_cmd port_setattr;

	struct lif_identify_cmd lif_identify;
	struct lif_init_cmd lif_init;
	struct lif_reset_cmd lif_reset;

	struct qos_identify_cmd qos_identify;
	struct qos_init_cmd qos_init;
	struct qos_reset_cmd qos_reset;

	struct q_identify_cmd q_identify;
	struct q_init_cmd q_init;
};

union dev_cmd_comp {
	u32 words[4];
	u8 status;
	struct admin_comp comp;
	struct nop_comp nop;

	struct dev_identify_comp identify;
	struct dev_init_comp init;
	struct dev_reset_comp reset;
	struct dev_getattr_comp getattr;
	struct dev_setattr_comp setattr;

	struct port_identify_comp port_identify;
	struct port_init_comp port_init;
	struct port_reset_comp port_reset;
	struct port_getattr_comp port_getattr;
	struct port_setattr_comp port_setattr;

	struct lif_identify_comp lif_identify;
	struct lif_init_comp lif_init;
	lif_reset_comp lif_reset;

	struct qos_identify_comp qos_identify;
	qos_init_comp qos_init;
	qos_reset_comp qos_reset;

	struct q_identify_comp q_identify;
	struct q_init_comp q_init;
};

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
union dev_info_regs {
#define IONIC_DEVINFO_FWVERS_BUFLEN 32
#define IONIC_DEVINFO_SERIAL_BUFLEN 32
	struct {
		u32    signature;
		u8     version;
		u8     asic_type;
		u8     asic_rev;
#define IONIC_FW_STS_F_RUNNING 0x1
		u8     fw_status;
		u32    fw_heartbeat;
		char   fw_version[IONIC_DEVINFO_FWVERS_BUFLEN];
		char   serial_num[IONIC_DEVINFO_SERIAL_BUFLEN];
	};
	u32 words[512];
};

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
union dev_cmd_regs {
	struct {
		u32                   doorbell;
		u32                   done;
		union dev_cmd         cmd;
		union dev_cmd_comp    comp;
		u8                    rsvd[48];
		u32                   data[478];
	};
	u32 words[512];
};

/**
 * union dev_regs - Device register format in for bar 0 page 0
 * @info:            Device info registers
 * @devcmd:          Device command registers
 */
union dev_regs {
	struct {
		union dev_info_regs info;
		union dev_cmd_regs  devcmd;
	};
	__le32 words[1024];
};

union adminq_cmd {
	struct admin_cmd cmd;
	struct nop_cmd nop;
	struct q_identify_cmd q_identify;
	struct q_init_cmd q_init;
	struct q_control_cmd q_control;
	struct lif_setattr_cmd lif_setattr;
	struct lif_getattr_cmd lif_getattr;
	struct rx_mode_set_cmd rx_mode_set;
	struct rx_filter_add_cmd rx_filter_add;
	struct rx_filter_del_cmd rx_filter_del;
	struct rdma_reset_cmd rdma_reset;
	struct rdma_queue_cmd rdma_queue;
	struct fw_download_cmd fw_download;
	struct fw_control_cmd fw_control;
};

union adminq_comp {
	struct admin_comp comp;
	struct nop_comp nop;
	struct q_identify_comp q_identify;
	struct q_init_comp q_init;
	struct lif_setattr_comp lif_setattr;
	struct lif_getattr_comp lif_getattr;
	struct rx_filter_add_comp rx_filter_add;
	struct fw_control_comp fw_control;
};

#define IONIC_BARS_MAX			6
#define IONIC_PCI_BAR_DBELL		1

/* BAR0 */
#define BAR0_SIZE			0x8000
#define BAR1_SIZE			0x20000

#define BAR0_DEV_INFO_REGS_OFFSET	0x0000
#define BAR0_DEV_CMD_REGS_OFFSET	0x0800
#define BAR0_DEV_CMD_DATA_REGS_OFFSET	0x0c00
#define BAR0_INTR_STATUS_OFFSET		0x1000
#define BAR0_INTR_CTRL_OFFSET		0x2000
#define DEV_CMD_DONE			0x00000001

#define ASIC_TYPE_CAPRI			0

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
struct doorbell {
	__le16 p_index;
	u8     ring;
	u8     qid_lo;
	__le16 qid_hi;
	u16    rsvd2;
};

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
struct intr_ctrl {
	u8 coalescing_init;
	u8 rsvd[3];
	u8 mask;
	u8 rsvd2[3];
	u16 int_credits;
	u16 flags;
#define INTR_F_UNMASK		0x0001
#define INTR_F_TIMER_RESET	0x0002
	u8 mask_on_assert;
	u8 rsvd3[3];
	u8 coalescing_curr;
	u8 rsvd4[3];
	u32 rsvd6[3];
};

#define INTR_CTRL_REGS_MAX	2048
#define INTR_CTRL_COAL_MAX	0x3F

#define intr_to_coal(intr_ctrl)		\
		((void __iomem *)&(intr_ctrl)->coalescing_init)
#define intr_to_mask(intr_ctrl)		\
		((void __iomem *)&(intr_ctrl)->mask)
#define intr_to_credits(intr_ctrl)	\
		((void __iomem *)&(intr_ctrl)->int_credits)
#define intr_to_mask_on_assert(intr_ctrl)\
		((void __iomem *)&(intr_ctrl)->mask_on_assert)

struct intr_status {
	u32 status[2];
};

struct notifyq_cmd {
	__le32 data;	/* Not used but needed for qcq structure */
};

union notifyq_comp {
	struct notifyq_event event;
	struct link_change_event link_change;
	struct reset_event reset;
	struct heartbeat_event heartbeat;
	struct log_event log;
};

/**
 * struct eq_comp - Event queue completion descriptor
 *
 * @code:  Event code, see enum eq_comp_code.
 * @lif_index: To which lif the event pertains.
 * @qid:   To which queue id the event pertains.
 * @gen_color: Event queue wrap counter, init 1, incr each wrap.
 */
struct eq_comp {
	__le16 code;
	__le16 lif_index;
	__le32 qid;
	u8 rsvd[7];
	u8 gen_color;
};

enum eq_comp_code {
	EQ_COMP_CODE_NONE = 0,
	EQ_COMP_CODE_RX_COMP = 1,
	EQ_COMP_CODE_TX_COMP = 2,
};

/* Deprecate */
struct identity {
	union drv_identity drv;
	union dev_identity dev;
	union lif_identity lif;
	union port_identity port;
	union qos_identity qos;
	union q_identity txq;
};

#pragma pack(pop)

#endif /* _IONIC_IF_H_ */
