/* SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB */
/*
 * Copyright (c) 2018-2019 Pensando Systems, Inc.  All rights reserved.
 */

#include <rdma/ib_pack.h>

#include "ionic_kcompat.h"

#define STRUCT_FIELD(header, field) \
	.struct_offset_bytes = offsetof(struct ib_unpacked_ ## header, field),      \
	.struct_size_bytes   = sizeof ((struct ib_unpacked_ ## header *) 0)->field, \
	.field_name          = #header ":" #field

static const struct ib_field eth_table[]  = {
	{ STRUCT_FIELD(eth, dmac_h),
	  .offset_words = 0,
	  .offset_bits  = 0,
	  .size_bits    = 32 },
	{ STRUCT_FIELD(eth, dmac_l),
	  .offset_words = 1,
	  .offset_bits  = 0,
	  .size_bits    = 16 },
	{ STRUCT_FIELD(eth, smac_h),
	  .offset_words = 1,
	  .offset_bits  = 16,
	  .size_bits    = 16 },
	{ STRUCT_FIELD(eth, smac_l),
	  .offset_words = 2,
	  .offset_bits  = 0,
	  .size_bits    = 32 },
	{ STRUCT_FIELD(eth, type),
	  .offset_words = 3,
	  .offset_bits  = 0,
	  .size_bits    = 16 }
};

static const struct ib_field vlan_table[]  = {
	{ STRUCT_FIELD(vlan, tag),
	  .offset_words = 0,
	  .offset_bits  = 0,
	  .size_bits    = 16 },
	{ STRUCT_FIELD(vlan, type),
	  .offset_words = 0,
	  .offset_bits  = 16,
	  .size_bits    = 16 }
};

static const struct ib_field ip4_table[]  = {
	{ STRUCT_FIELD(ip4, ver),
	  .offset_words = 0,
	  .offset_bits  = 0,
	  .size_bits    = 4 },
	{ STRUCT_FIELD(ip4, hdr_len),
	  .offset_words = 0,
	  .offset_bits  = 4,
	  .size_bits    = 4 },
	{ STRUCT_FIELD(ip4, tos),
	  .offset_words = 0,
	  .offset_bits  = 8,
	  .size_bits    = 8 },
	{ STRUCT_FIELD(ip4, tot_len),
	  .offset_words = 0,
	  .offset_bits  = 16,
	  .size_bits    = 16 },
	{ STRUCT_FIELD(ip4, id),
	  .offset_words = 1,
	  .offset_bits  = 0,
	  .size_bits    = 16 },
	{ STRUCT_FIELD(ip4, frag_off),
	  .offset_words = 1,
	  .offset_bits  = 16,
	  .size_bits    = 16 },
	{ STRUCT_FIELD(ip4, ttl),
	  .offset_words = 2,
	  .offset_bits  = 0,
	  .size_bits    = 8 },
	{ STRUCT_FIELD(ip4, protocol),
	  .offset_words = 2,
	  .offset_bits  = 8,
	  .size_bits    = 8 },
	{ STRUCT_FIELD(ip4, check),
	  .offset_words = 2,
	  .offset_bits  = 16,
	  .size_bits    = 16 },
	{ STRUCT_FIELD(ip4, saddr),
	  .offset_words = 3,
	  .offset_bits  = 0,
	  .size_bits    = 32 },
	{ STRUCT_FIELD(ip4, daddr),
	  .offset_words = 4,
	  .offset_bits  = 0,
	  .size_bits    = 32 }
};

static const struct ib_field udp_table[]  = {
	{ STRUCT_FIELD(udp, sport),
	  .offset_words = 0,
	  .offset_bits  = 0,
	  .size_bits    = 16 },
	{ STRUCT_FIELD(udp, dport),
	  .offset_words = 0,
	  .offset_bits  = 16,
	  .size_bits    = 16 },
	{ STRUCT_FIELD(udp, length),
	  .offset_words = 1,
	  .offset_bits  = 0,
	  .size_bits    = 16 },
	{ STRUCT_FIELD(udp, csum),
	  .offset_words = 1,
	  .offset_bits  = 16,
	  .size_bits    = 16 }
};

static const struct ib_field grh_table[]  = {
	{ STRUCT_FIELD(grh, ip_version),
	  .offset_words = 0,
	  .offset_bits  = 0,
	  .size_bits    = 4 },
	{ STRUCT_FIELD(grh, traffic_class),
	  .offset_words = 0,
	  .offset_bits  = 4,
	  .size_bits    = 8 },
	{ STRUCT_FIELD(grh, flow_label),
	  .offset_words = 0,
	  .offset_bits  = 12,
	  .size_bits    = 20 },
	{ STRUCT_FIELD(grh, payload_length),
	  .offset_words = 1,
	  .offset_bits  = 0,
	  .size_bits    = 16 },
	{ STRUCT_FIELD(grh, next_header),
	  .offset_words = 1,
	  .offset_bits  = 16,
	  .size_bits    = 8 },
	{ STRUCT_FIELD(grh, hop_limit),
	  .offset_words = 1,
	  .offset_bits  = 24,
	  .size_bits    = 8 },
	{ STRUCT_FIELD(grh, source_gid),
	  .offset_words = 2,
	  .offset_bits  = 0,
	  .size_bits    = 128 },
	{ STRUCT_FIELD(grh, destination_gid),
	  .offset_words = 6,
	  .offset_bits  = 0,
	  .size_bits    = 128 }
};

int roce_ud_header_unpack(void *buf, struct ib_ud_header *header)
{
	u16	type;

	ib_unpack(eth_table, ARRAY_SIZE(eth_table),
		  buf, &header->eth);
	buf += IB_ETH_BYTES;
	header->eth_present = 1;
	type = be16_to_cpu(header->eth.type);

	if (type == ETH_P_8021Q) {
		ib_unpack(vlan_table, ARRAY_SIZE(vlan_table),
			  buf, &header->vlan);
		header->vlan_present = 1;
		buf += IB_VLAN_BYTES;
		type = be16_to_cpu(header->vlan.type);
	}

	switch (type) {
		case ETH_P_IPV6:
			ib_unpack(grh_table, ARRAY_SIZE(grh_table),
				  buf, &header->grh);
			header->grh_present = 1;
			buf += IB_GRH_BYTES;
			if (header->grh.ip_version != 6) {
				pr_warn("Invalid GRH.ip_version %d\n",
					header->grh.ip_version);
				return -EINVAL;
			}

			if (header->grh.next_header != IPPROTO_UDP) {
				pr_warn("Invalid GRH.next_header 0x%02x\n",
					header->grh.next_header);
				return -EINVAL;
			}
			break;

		case ETH_P_IP:
			ib_unpack(ip4_table, ARRAY_SIZE(ip4_table),
				  buf, &header->ip4);
			header->ipv4_present = 1;
			buf += IB_IP4_BYTES;
			if (header->ip4.ver != 4) {
				pr_warn("Invalid IP4.ip_version %d\n",
					header->ip4.ver);
				return -EINVAL;
			}

			if (header->ip4.protocol != IPPROTO_UDP) {
				pr_warn("Invalid IP4.protocol %d\n",
					header->ip4.protocol);
				return -EINVAL;
			}
			break;

		default:
			pr_warn("Invalid Ether type 0x%x\n", type);
			return -EINVAL;
	}

	ib_unpack(udp_table, ARRAY_SIZE(udp_table),
		  buf, &header->udp);
	buf += IB_UDP_BYTES;
	header->udp_present = 1;

	return 0;
}
