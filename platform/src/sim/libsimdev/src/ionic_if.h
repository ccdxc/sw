/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef _IONIC_IF_H_
#define _IONIC_IF_H_

#define DEV_CMD_SIGNATURE               0x44455643      /* 'DEVC' */

#define CMD_OPCODE_NOP			0x0
#define CMD_OPCODE_RESET		0x1
#define CMD_OPCODE_IDENTIFY		0x2
#define CMD_OPCODE_LIF_INIT		0x3
#define CMD_OPCODE_ADMINQ_INIT		0x4
#define CMD_OPCODE_TXQ_INIT		0x5
#define CMD_OPCODE_RXQ_INIT		0x6
#define CMD_OPCODE_Q_ENABLE		0x9
#define CMD_OPCODE_Q_DISABLE		0xa
#define CMD_OPCODE_STATION_MAC_ADDR_GET 0xf

#define CMD_OPCODE_DEBUG_Q_DUMP         0xf0

/**
 * struct cmd - General admin command format
 * @opcode:   Opcode for the command
 * @cmd_data: Opcode-specific command bytes
 */
struct admin_cmd {
	u16 opcode;
	u16 cmd_data[31];
} __packed;

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
	u32 comp_index:24;
	u8 cmd_data[11];
	u8 color:1;
} __packed;

/**
 * struct nop_cmd - NOP command
 * @opcode: opcode = 0
 */
struct nop_cmd {
	u16 opcode;
	u16 rsvd[31];
} __packed;

/**
 * struct nop_comp - NOP command completion
 * @status: The status of the command.  Values for status are:
 *             0 = Successful completion
 */
struct nop_comp {
	u32 status:8;
	u32 rsvd:24;
	u32 rsvd2[3];
} __packed;

/**
 * struct reset_cmd - Device reset command
 * @opcode: opcode = 1
 */
struct reset_cmd {
	u16 opcode;
	u16 rsvd[31];
} __packed;

/**
 * struct reset_comp - Reset command completion
 * @status: The status of the command.  Values for status are:
 *             0 = Successful completion
 */
struct reset_comp {
	u32 status:8;
	u32 rsvd:24;
	u32 rsvd2[3];
} __packed;

/**
 * struct identify_cmd - Device identify command
 * @opcode: opcode = 2
 * @addr:   Destination address for the 4096-byte device
 *          configuration info
 */
struct identify_cmd {
	u16 opcode;
	u16 rsvd;
	dma_addr_t addr;
	u32 rsvd2[13];
} __packed;

/**
 * struct identify_comp - Identify command completion
 * @status: The status of the command.  Values for status are:
 *             0 = Successful completion
 */
struct identify_comp {
	u32 status:8;
	u32 rsvd:24;
	u32 rsvd2[3];
} __packed;

/**
 * union identity - 4096 bytes of device identity information
 * @asic_type:        ASIC type:
 *                       0 = Capri
 * @asic_rev:         ASIC revision level, e.g. 0xA0
 * @serial_num:       NULL-terminated string representing the
 *                    device serial number
 * @fw_version:       NULL-terminated string representing the
 *                    firmware version
 * @nlifs:            Number of LIFs provisioned
 * @ndbpgs_per_lif:   Number of doorbell pages per LIF
 * @nadminqs_per_lif: Number of admin queues per LIF provisioned
 * @ntxqs_per_lif:    Number of Ethernet transmit queues per LIF
 *                    provisioned
 * @nrxqs_per_lif:    Number of Ethernet receive queues per LIF
 *                    provisioned
 * @ncps_per_lif:     Number of completion queues per LIF
 *                    provisioned
 * @nrdmasqs_per_lif: Number of RMDA send queues per LIF
 *                    provisioned
 * @nrdmarqs_per_lif: Number of RDMA receive queues per LIF
 *                    provisioned
 * @neqs_per_lif:     Number of event queues per LIF provisioned
 * @nintrs:           Number of interrupts provisioned
 */
union identity {
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
	};
	u32 words[1024];
} __packed;

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
} __packed;

/**
 * struct lif_init_comp - LIF init command completion
 * @status: The status of the command.  Values for status are:
 *             0 = Successful completion
 */
struct lif_init_comp {
	u32 status:8;
	u32 rsvd:24;
	u32 rsvd2[3];
} __packed;

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
} __packed;

/**
 * struct adminq_init_comp - Admin queue init command completion
 * @status:  The status of the command.  Values for status are:
 *              0 = Successful completion
 * @qid:     Doorbell queue ID
 * @db_type: Doorbell type to use for this queue
 */
struct adminq_init_comp {
	u32 status:8;
	u32 rsvd:24;
	u32 qid:24;
	u32 db_type:8;
	u32 rsvd2[2];
} __packed;

#define TXQ_TYPE_ETHERNET		0x0

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
} __packed;

/**
 * struct txq_init_comp - Tx queue init command completion
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @comp_index: The index in the descriptor ring for which this
 *              is the completion.
 * @qid:        Doorbell queue ID
 * @db_type:    Doorbell type to use for this queue
 * @color:      Color bit.
 */
struct txq_init_comp {
	u32 status:8;
	u32 comp_index:24;
	u32 qid:24;
	u32 db_type:8;
	u32 rsvd;
	u32 rsvd2:24;
	u32 color:1;
	u32 rsvd3:7;
} __packed;

#define TXQ_DESC_OPCODE_NOP		0x0
#define TXQ_DESC_OPCODE_CALC_NO_CSUM	0x1
#define TXQ_DESC_OPCODE_CALC_CSUM	0x2
#define TXQ_DESC_OPCODE_CALC_CRC32_CSUM	0x3
#define TXQ_DESC_OPCODE_TSO		0x4

/**
 * struct txq_desc - Ethernet Tx queue descriptor format
 * @addr_lo:      First data buffer's DMA address, lower 32 bits.
 *                (Subsequent data buffers are on txq_sg_desc).
 * @addr_hi:      First data buffer's DMA address, upper 20 bits
 * @num_sg_elems: Number of scatter-gather elements in SG
 *                descriptor
 * @opcode:       Tx operation, see TXQ_DESC_OPCODE_*:
 * 
 *                   TXQ_DESC_OPCODE_NOP:
 *
 *                      No packet sent; used to pad out end of
 *                      queue (ring)
 *
 *                   TXQ_DESC_OPCODE_CALC_NO_CSUM:
 *
 *                      Non-offload send.  No segmentation,
 *                      fragmention or checksum calc/insertion is
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
 *                   TXQ_DESC_OPCODE_CALC_CRC32_CSUM:
 *
 *                      Offload 32-bit CRC32c L4 checksum
 *                      calculation/insertion.  SCTP, for
 *                      example, uses this checksum algortihm.
 *                      The same rules and fields @hdr_len and
 *                      @csum_offset are used as in
 *                      TXQ_DESC_OPCODE_CALC_CSUM, including the
 *                      encapsulation cases where SCTP or other
 *                      CRC32c protocol is the inner protocol.
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
 *                Valid for opcodes TXQ_DESC_OPCODE_CALC_CSUM,
 *                TXQ_DESC_OPCODE_CALC_CRC32_CSUM and
 *                TXQ_DESC_OPCODE_TSO.  Should be set to zero for
 *                all other modes.  For TXQ_DESC_OPCODE_CALC_CSUM
 *                and TXQ_DESC_OPCODE_CALC_CRC32_CSUM, @hdr_len
 *                is length of headers up to inner-most L4
 *                header.  For TXQ_DESC_OPCODE_TSO, @hdr_len is
 *                up to inner-most L4 payload, so inclusive of
 *                inner-most L4 header.
 * @V:            Insert an L2 VLAN header using @vlan_tci.
 * @C:            Create a completion entry (CQ) for this packet.
 * @O:            Calculate outer-header checksum for GRE or UDP
 *                encapsulations.
 * @mss:          Desired MSS value for TSO.  Only applicable for
 *                TXQ_DESC_OPCODE_TSO.
 * @csum_offset:  Offset into inner-most L4 header of checksum
 *                field.  Only applicable for
 *                TXQ_DESC_OPCODE_CALC_CSUM and
 *                TXQ_DESC_OPCODE_CALC_CRC32_CSUM.
 */
struct txq_desc {
	u32 addr_lo;
	u32 addr_hi:20;
	u32 rsvd:4;
	u32 num_sg_elems:5;
	u32 opcode:3;
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
} __packed;

#define IONIC_TX_MAX_SG_ELEMS	16

/** struct txq_sg_desc - Transmit scatter-gather (SG) list
 * @addr_lo:   DMA address of SG element data buffer, lower
 *             32-bits
 * @addr_hi:   DMA address of SG element data buffer, upper
 *             20-bits
 * @len:       Length of SG element data buffer, in bytes
 */
struct txq_sg_desc {
	struct txq_sg_elem {
		u32 addr_lo;
		u32 addr_hi:20;
		u32 rsvd:4;
		u16 len;
		u16 rsvd2[3];
	} elems[IONIC_TX_MAX_SG_ELEMS];
} __packed;

/** struct txq_comp - Ethernet transmit queue completion descriptor
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @comp_index: The index in the descriptor ring for which this
 *              is the completion.
 * @color:      Color bit.
*/
struct txq_comp {
	u32 status:8;
	u32 comp_index:16;
	u32 rsvd:8;
	u32 rsvd2[2];
	u32 rsvd3:24;
	u32 color:1;
	u32 rsvd4:7;
} __packed;

#define RXQ_TYPE_ETHERNET		0x0

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
} __packed;

/**
 * struct rxq_init_comp - Rx queue init command completion
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @comp_index: The index in the descriptor ring for which this
 *              is the completion.
 * @qid:        Doorbell queue ID
 * @db_type:    Doorbell type to use for this queue
 * @color:      Color bit.
 */
struct rxq_init_comp {
	u32 status:8;
	u32 comp_index:24;
	u32 qid:24;
	u32 db_type:8;
	u32 rsvd;
	u32 rsvd2:24;
	u32 color:1;
	u32 rsvd3:7;
} __packed;

#define RXQ_DESC_OPCODE_NOP		0x0
#define RXQ_DESC_OPCODE_SIMPLE		0x1

/**
 * struct rxq_desc - Ethernet Rx queue descriptor format
 * @addr_lo:      Data buffer's DMA address, lower 32 bits.
 * @addr_hi:      Data buffer's DMA address, upper 20 bits
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
 *                      starting at @addr_lo/hi.  Results of
 *                      receive, including actual bytes received,
 *                      are recorded in Rx completion descriptor.
 *
 */
struct rxq_desc {
	u32 addr_lo;
	u32 addr_hi:20;
	u32 rsvd:12;
	u16 len;
	u16 opcode:3;
	u16 rsvd2:13;
	u32 rsvd3;
} __packed;

#define RXQ_COMP_RSS_TYPE_NONE		0x0
#define RXQ_COMP_RSS_TYPE_IPV4		0x1
#define RXQ_COMP_RSS_TYPE_IPV6		0x2
#define RXQ_COMP_RSS_TYPE_IPV6_EX	0x3
#define RXQ_COMP_RSS_TYPE_IPV4_TCP	0x4
#define RXQ_COMP_RSS_TYPE_IPV6_TCP	0x5
#define RXQ_COMP_RSS_TYPE_IPV6_TCP_EX	0x6
#define RXQ_COMP_RSS_TYPE_IPV4_UDP	0x7
#define RXQ_COMP_RSS_TYPE_IPV6_UDP	0x8
#define RXQ_COMP_RSS_TYPE_IPV6_UDP_EX	0x9

/** struct rxq_comp - Ethernet receive queue completion descriptor
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @comp_index: The index in the descriptor ring for which this
 *              is the completion.
 * @rss_hash:   32-bit RSS hash for the @rss_type indicated
 * @csum:       One's complement of the one's complement sum of the
 *              entire packet data 16-bit words.  If the packet is
 *              odd length, an extra zero-value byte is included in
 *              the @csum calculation but not included in @len.
 * @vlan_tci:   VLAN tag stripped from the packet.  Valid if @V is
 *              set.  Includes .1p and .1q tags.
 * @len:        Received packet length, in bytes.  Excludes FCS.
 * @csum_level: Indicates how many header checksums were
 *              validated by the device.  Possible values are:
 *                 0 = No validation was done or the outer-most
 *                     checksum is invalid.
 *                 1 = Outer most checksum is valid.
 *                 2 = Outer two-most checksums are valid.
 *                 3 = Outer three-most checksums are valid.
 *              Only these protocols are accounted for in
 *              @csum_level.  All other protocols will not count
 *              towards @csum_level.  The L3 IPv4|IPv6 checksum
 *              validation is implied when validating the L4
 *              checksum:
 *                 TCP: IPv4 and IPv6
 *                 UDP: IPv4 and IPv6
 *                 GRE: IPv4 and IPv6, but only if checksum is
 *                      present in GRE header (checksum bit is
 *                      set).
 *                 SCTP: IPv4 and IPv6
 * @rss_type:   RSS type for @rss_hash:
 *                 0 = RSS hash not calcuated
 *                 1 = L3 IPv4
 *                 2 = L3 IPv6
 *                 3 = L3 IPv6 w/ extensions
 *                 4 = L4 IPv4/TCP
 *                 5 = L4 IPv6/TCP
 *                 6 = L4 IPv6/TCP w/ extensions
 *                 7 = L4 IPv4/UDP
 *                 8 = L4 IPv6/UDP
 *                 9 = L4 IPv6/UDP w/ extensions
 * @color:      Color bit.
 * @V:          VLAN header was stripped and placed in @vlan_tci.
*/
struct rxq_comp {
	u32 status:8;
	u32 comp_index:16;
	u32 rsvd:8;
	u32 rss_hash;
	u16 csum;
	u16 vlan_tci;
	u32 len:14;
	u32 csum_level:2;
	u32 rss_type:4;
	u32 rsvd2:4;
	u32 color:1;
	u32 V:1;
	u32 rsvd3:6;
} __packed;

/**
 * struct q_enable_cmd - Queue enable command
 * @opcode:     opcode = 9
 * @qid:        Queue ID
 */
struct q_enable_cmd {
	u16 opcode;
	u16 rsvd;
	u32 qid:24;
	u32 db_type:8;
	u32 rsvd2[14];
} __packed;

/**
 * struct q_enable_comp - Queue enable command completion
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @comp_index: The index in the descriptor ring for which this
 *              is the completion.
 * @color:      Color bit.
 */
struct q_enable_comp {
	u32 status:8;
	u32 comp_index:24;
	u32 rsvd[2];
	u32 rsvd2:24;
	u32 color:1;
	u32 rsvd3:7;
} __packed;

/**
 * struct q_disable_cmd - Queue disable command
 * @opcode:     opcode = 10 
 * @qid:        Queue ID
 */
struct q_disable_cmd {
	u16 opcode;
	u16 rsvd;
	u32 qid:24;
	u32 db_type:8;
	u32 rsvd2[14];
} __packed;

/**
 * struct q_disable_comp - Queue disable command completion
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @comp_index: The index in the descriptor ring for which this
 *              is the completion.
 * @color:      Color bit.
 */
struct q_disable_comp {
	u32 status:8;
	u32 comp_index:24;
	u32 rsvd[2];
	u32 rsvd2:24;
	u32 color:1;
	u32 rsvd3:7;
} __packed;

/**
 * struct station_mac_addr_get_cmd - Get LIF's station MAC address
 *                                   command
 * @opcode:     opcode = 15
 */
struct station_mac_addr_get_cmd {
    u16 opcode;
    u16 rsvd[31];
} __packed;

/**
 * struct station_mac_addr_get_comp - Get LIF's station MAC address
 *                                    command completion
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @comp_index: The index in the descriptor ring for which this
 *              is the completion.
 * @addr:       Station MAC address
 * @color:      Color bit.
 */
struct station_mac_addr_get_comp {
    u32 status:8;
    u32 rsvd:8;
    u32 comp_index:16;
    u8 addr[6];
    u16 rsvd2;
    u32 rsvd3:24;
    u32 color:1;
    u32 rsvd4:7;
} __packed;

/**
 * struct debug_q_dump_cmd - Debug queue dump command
 * @opcode:     opcode = 0xf0
 * @qid:        Queue ID
 * @qtype:      Queue type (see QUEUE_TYPE_xxx)
 */
struct debug_q_dump_cmd {
    u16 opcode;
    u16 rsvd;
    u32 qid:24;
    u32 qtype:8;
    u32 rsvd2[14];
} __packed;

/**
 * struct debug_q_dump_comp - Debug queue dump command completion
 * @status:     The status of the command.  Values for status are:
 *                 0 = Successful completion
 * @comp_index: The index in the descriptor ring for which this
 *              is the completion.
 * @p_index:    Queue producer index
 * @c_index:    Queue consumer index
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
    u32 rsvd2;
    u32 rsvd3:24;
    u32 color:1;
    u32 rsvd4:7;
} __packed;

#endif /* _IONIC_IF_H_ */
