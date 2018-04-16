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

	CMD_OPCODE_Q_ENABLE			= 9,
	CMD_OPCODE_Q_DISABLE			= 10,

	CMD_OPCODE_STATION_MAC_ADDR_GET		= 15,
	CMD_OPCODE_MTU_SET			= 16,
	CMD_OPCODE_RX_MODE_SET			= 17,
	CMD_OPCODE_RX_FILTER_ADD		= 18,
	CMD_OPCODE_RX_FILTER_DEL		= 19,
	CMD_OPCODE_STATS_DUMP_START		= 20,
	CMD_OPCODE_STATS_DUMP_STOP		= 21,
	CMD_OPCODE_RSS_HASH_SET			= 22,
	CMD_OPCODE_RSS_INDIR_SET		= 23,

	CMD_OPCODE_RDMA_FIRST_CMD		= 50, //Keep this as first rdma cmd
	CMD_OPCODE_RDMA_QUERY_PKEY		= 51,
	CMD_OPCODE_RDMA_CREATE_EQ		= 52,
	CMD_OPCODE_RDMA_DESTROY_EQ		= 53,
	CMD_OPCODE_RDMA_CREATE_MR		= 54,
	CMD_OPCODE_RDMA_DESTROY_MR		= 55,
	CMD_OPCODE_RDMA_CREATE_CQ		= 56,
	CMD_OPCODE_RDMA_DESTROY_CQ		= 57,
	CMD_OPCODE_RDMA_RESIZE_CQ		= 58,
	CMD_OPCODE_RDMA_CREATE_QP		= 59,
	CMD_OPCODE_RDMA_MODIFY_QP		= 60,
	CMD_OPCODE_RDMA_DESTROY_QP		= 61,
	CMD_OPCODE_RDMA_QUERY_PORT		= 62,
	CMD_OPCODE_RDMA_LAST_CMD		= 63, //Keep this as last rdma cmd

	CMD_OPCODE_DEBUG_Q_DUMP			= 0xf0,
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
	OS_TYPE_IXPE    = 5,
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
 *     @ndbpgs_per_lif:   Number of doorbell pages per LIF
 *     @nadminqs_per_lif: Number of admin queues per LIF provisioned
 *     @ntxqs_per_lif:    Number of Ethernet transmit queues per LIF
 *                        provisioned
 *     @nrxqs_per_lif:    Number of Ethernet receive queues per LIF
 *                        provisioned
 *     @ncps_per_lif:     Number of completion queues per LIF
 *                        provisioned
 *     @nrdmasqs_per_lif: Number of RMDA send queues per LIF
 *                        provisioned
 *     @nrdmarqs_per_lif: Number of RDMA receive queues per LIF
 *                        provisioned
 *     @neqs_per_lif:     Number of event queues per LIF provisioned
 *     @nintrs:           Number of interrupts provisioned
 *     @nucasts_per_lif:  Number of perfect unicast addresses
 *                        supported
 *     @nmcasts_per_lif:  Number of perfect multicast addresses
 *                        supported.
 *     @intr_coal_mult:   Interrupt coalescing multiplication factor.
 *                        Scale user-supplied interrupt coalescing
 *                        value in usecs to device units using:
 *                           device units = usecs * mult / div
 *     @intr_coal_div:    Interrupt coalescing division factor.
 *                        Scale user-supplied interrupt coalescing
 *                        value in usecs to device units using:
 *                           device units = usecs * mult / div
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
		u8 rsvd[2];
		char serial_num[20];
		char fw_version[20];
		u32 nlifs;
		u32 ndbpgs_per_lif;
		u32 nadminqs_per_lif;
		u32 ntxqs_per_lif;
		u32 nrxqs_per_lif;
		u32 ncqs_per_lif;
		u32 nrdmasqs_per_lif;
		u32 nrdmarqs_per_lif;
		u32 neqs_per_lif;
		u32 nintrs;
		u32 nucasts_per_lif;
		u32 nmcasts_per_lif;
		u32 intr_coal_mult;
		u32 intr_coal_div;
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
			u16 rsvd3:2;
		};
		struct {
			u16 csum_offset:14;
			u16 rsvd4:2;
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
	RXQ_COMP_RSS_TYPE_IPV6,
	RXQ_COMP_RSS_TYPE_IPV6_EX,
	RXQ_COMP_RSS_TYPE_IPV4_TCP,
	RXQ_COMP_RSS_TYPE_IPV6_TCP,
	RXQ_COMP_RSS_TYPE_IPV6_TCP_EX,
	RXQ_COMP_RSS_TYPE_IPV4_UDP,
	RXQ_COMP_RSS_TYPE_IPV6_UDP,
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
 * @rss_type:     RSS type for @rss_hash:
 *                   0 = RSS hash not calcuated
 *                   1 = L3 IPv4
 *                   2 = L3 IPv6
 *                   3 = L3 IPv6 w/ extensions
 *                   4 = L4 IPv4/TCP
 *                   5 = L4 IPv6/TCP
 *                   6 = L4 IPv6/TCP w/ extensions
 *                   7 = L4 IPv4/UDP
 *                   8 = L4 IPv6/UDP
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
	u32 rsvd3:4;
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
 * struct rx_filter_cmd - Add/delete LIF's Rx filter command
 * @opcode:     opcode = 18 (add), 19 (delete)
 * @match:      Rx filter match type.  (See RX_FILTER_MATCH_xxx)
 * @qid:        Queue ID
 * @qtype:      Queue type
 * @vlan:       VLAN ID
 * @addr:       MAC Address (network-byte order)
 */
struct rx_filter_cmd {
	u16 opcode;
	u16 match;
	u32 qid:24;
	u32 qtype:8;
	u16 vlan;
	u8 addr[6];
	u8 rsvd[2];
	u16 rsvd2[23];
};

/**
 * struct rx_filter_comp - Add/delete LIF's Rx filter command
 *                         completion
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @comp_index: The index in the descriptor ring for which this
 *              is the completion.
 * @filter_id:  Filter ID
 * @color:      Color bit.
 */
struct rx_filter_comp {
	u32 status:8;
	u32 rsvd:8;
	u32 comp_index:16;
	u32 filter_id;
	u32 rsvd2;
	u32 rsvd3:31;
	u32 color:1;
};

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
 * union stats_dump - 4096 bytes of device stats
 * TODO define stats dump area, placeholders for now:
 * @stat1:  64-bit device stat
 * @stat2:  64-bit device stat
 */
union stats_dump {
	struct {
		/* TODO these are placeholders */
		u64 stat1;
		u64 stat2;
	} ver1;
	u32 words[1024];
};

#define RSS_HASH_KEY_SIZE	40

enum rss_hash_types {
	RSS_TYPE_IPV4		= BIT(0),
	RSS_TYPE_IPV4_TCP	= BIT(1),
	RSS_TYPE_IPV4_UDP	= BIT(2),
	RSS_TYPE_IPV6		= BIT(3),
	RSS_TYPE_IPV6_TCP	= BIT(4),
	RSS_TYPE_IPV6_UDP	= BIT(5),
	RSS_TYPE_IPV6_EX	= BIT(6),
	RSS_TYPE_IPV6_TCP_EX	= BIT(7),
	RSS_TYPE_IPV6_UDP_EX	= BIT(8),
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
 * struct rdma_create_eq_cmd - Create Event Queue command
 * @opcode:        opcode = 52
 * @intr:          intr number (also eq id)
 * @lif:           hardware lif id
 * @log_depth:     log-base-two size of queue
 * @log_stride:    log-base-two size of queue element
 * @dma_addr:      dma address base of queue memory
 *
 * There is no eq-specific completion struct.  Only status is indicated.
 * Completion uses the common struct admin_comp.
 **/
struct create_eq_cmd {
	u16 opcode;
	u16 intr;
	u16 lif_id;
	u8  log_depth;
	u8  log_stride;
	u64 dma_addr;
	u8 rsvd[48];
};

/**
 * struct rdma_create_mr_cmd - Create Memory registration command
 * @opcode:        opcode = 54
 * @pd_num:        id of the pd
 * @lif:           hardware lif id
 * @access_flags:  access protaction requested for memory region
 * @start:         starting virtual address of the memory region
 * @length:        length of the memory region in bytes.
 * @pdir_dma:      PA of the page translation table
 * @page_size:     Host Page Size
 * @nchunks:       number of physical pages in the PT table.
 * @lkey:          local key
 * @rkey:          remote key
 *
 **/
struct create_mr_cmd {
	u16 opcode;
	u16 pd_num;
	u16 lif;
	u16 access_flags;
	u64 start;
	u64 length;
	u64 pt_dma;
	u32 page_size;
	u32 nchunks;
	u32 lkey;
	u32 rkey;
	u32 rsvd[4];
};

/**
 * rdma_create_mr_comp - create_mr command completion
 * @status:        Status of the command. 
 *                  0 - Successful completion
 **/
struct create_mr_comp {
	u32 status:8;
	u32 rsvd:24;
	u32 rsvd2[3];
};

/**
 * struct create_cq_cmd - Create RDMA Completion queue command
 * @opcode:        opcode = 50
 * @cq_wqe_size:   work queue entry size for CQ
 * @num_cq_wqes:   number of wqes in CQ
 * @cq_num:        queue id for CQ. Driver manages the space.
 * @lif_id:        LIF ID
 * @host_pg_size:  Host Page Size
 * @cq_lkey:       local key for CQ memory
 * @eq_id:         EQ ID
 * @pt_base_addr:  page translation table base address for CQ memory
 * @cq_va:         Starting virtual address of CQ memory
 * @pt_size:       number of page translation table entries.
 **/
struct create_cq_cmd {
	u16 opcode;
	u16 cq_wqe_size;
	u32 cq_num;
	u16 num_cq_wqes;
	u16 lif_id;
	u32 host_pg_size;
	u32 cq_lkey;
	u32 eq_id;
	u64 pt_base_addr;
	u64 cq_va;
	u64 va_len;
	u32 pt_size;
	u32 rsvd2[3];
};

/**
 * struct create_cq_comp
 * @status: The status of the command.  Values for status are:
 *             0 = Successful completion
 */
struct create_cq_comp {
	u32 status:8;
	u32 qtype:8;
	u32 rsvd:16;
	u32 rsvd2[3];
};

/**
 * struct create_qp_cmd - Create RDMA SQ/RQ queue command
 * @opcode:        opcode = 59
 * @sq_wqe_size:   work queue entry size for SQ
 * @rq_wqe_size:   work queue entry size for RQ
 * @num_sq_wqes:   number of wqes in SQ
 * @num_rq_wqes:   number of wqes in RQ
 * @num_rsq_wqes:  number of wqes in RSQ
 * @num_rrq_wqes:  number of wqes in RRQ
 * @pd:            pd number
 * @lif_id:        LIF ID
 * @service:       RC/UD
 * @flags:         qp flags
 * @pmtu:          path mtu for qp
 * @sq_cq_num:     cq for SQ
 * @rq_cq_num:     cq for RQ
 * @host_pg_size:  host page size
 * @sq_lkey:       local key for SQ
 * @rq_lkey:       local key for RQ
 **/

struct create_qp_cmd {
	u16 opcode;
	u16 sq_wqe_size;
	u16 rq_wqe_size;    
	u16 num_sq_wqes;
	u16 num_rq_wqes;    
	u16 num_rsq_wqes;
	u16 num_rrq_wqes;    
	u16 pd;
	u16 lif_id;
	u8  service;
	u8  flags;
	u32 pmtu;
	u32 qp_num;
	u32 sq_cq_num;
	u32 rq_cq_num;    
	u32 host_pg_size;
	u32 sq_lkey;
	u32 rq_lkey;

#ifdef ADMINQ
	/*
	 * dev command is only 64 bytes right now. So for now do multiple calls to
	 * register queue memory first and then create_qp.
	 */
	u64 sq_pt_base_addr;
	u64 sq_cq_va;
	u32 sq_pt_size;
	u64 rq_pt_base_addr;
	u64 rq_cq_va;
	u32 rq_pt_size;
#endif
	
	u32 rsvd2[4];
};

/**
 * struct create_cq_comp
 * @status: The status of the command.  Values for status are:
 *             0 = Successful completion
 * @sq_qtype: qtype for SQ
 * @rq_qtype: qtype for Q 
 */

struct create_qp_comp {
	u32 status:8;
	u32 sq_qtype:8;
	u32 rq_qtype:8;    
	u32 rsvd:8;
	u32 rsvd2[3];
};

/**
 * struct modify_qp_cmd - modify RDMA SQ/RQ queue command
 * @opcode:        opcode = 60
 * @lif_id:        lif id
 * @attr_mask:     mask to indicate which attributes are being modified
 * @qp_num:        qp number
 * @dest_qp_num:   destination qp number
 * @q_key:         q_key for UD
 * @e_psn:         initial expected seq number
 * @sq_psn:        initial send side psn
 * @header_template: header tempalte
 * @header_template_size: header template size
 */
struct modify_qp_cmd {
	u16 opcode;
	u16 lif_id;
	u32 attr_mask;
	u32 qp_num;
	u32 dest_qp_num;    
	u32 q_key;
	u32 e_psn;
	u32 sq_psn;
	u64 header_template;
	u32 header_template_size;
	
	u32 rsvd2[6];
};

/**
 * struct create_qp_comp
 * @status: The status of the command.  Values for status are:
 *             0 = Successful completion
 */

struct modify_qp_comp {
	u32 status:8;
	u32 rsvd:24;
	u32 rsvd2[3];
};

#pragma pack(pop)

union adminq_cmd {
	struct admin_cmd cmd;
	struct nop_cmd nop;
	struct txq_init_cmd txq_init;
	struct rxq_init_cmd rxq_init;
	struct features_cmd features;
	struct q_enable_cmd q_enable;
	struct q_disable_cmd q_disable;
	struct station_mac_addr_get_cmd station_mac_addr_get;
	struct mtu_set_cmd mtu_set;
	struct rx_mode_set_cmd rx_mode_set;
	struct rx_filter_cmd rx_filter;
	struct stats_dump_cmd stats_dump;
	struct rss_hash_set_cmd rss_hash_set;
	struct rss_indir_set_cmd rss_indir_set;
	struct debug_q_dump_cmd debug_q_dump;
	struct create_eq_cmd create_eq;
	struct create_mr_cmd create_mr;
	struct create_cq_cmd create_cq;
	struct create_qp_cmd create_qp;
	struct modify_qp_cmd modify_qp;
};

union adminq_comp {
	struct admin_comp comp;
	struct nop_comp nop;
	struct txq_init_comp txq_init;
	struct rxq_init_comp rxq_init;
	struct features_comp features;
	struct station_mac_addr_get_comp station_mac_addr_get;
	struct rx_filter_comp rx_filter;
	struct stats_dump_comp stats_dump;
	struct debug_q_dump_comp debug_q_dump;
	struct create_mr_comp create_mr;
	struct create_cq_comp create_cq;
	struct create_qp_comp create_qp;
	struct modify_qp_comp modify_qp;
};

#endif /* _IONIC_IF_H_ */
