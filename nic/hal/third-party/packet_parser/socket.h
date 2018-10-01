/*
 * Copyright 2013 Google Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
/*
 * Author: ncardwell@google.com (Neal Cardwell)
 *
 * Interface for tracking sockets in the kernel under test.
 */

#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "types.h"

#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include "logging.h"
#include "packet.h"
#include "ip_address.h"

/* All possible states for a socket we're tracking. */
enum socket_state_t {
	SOCKET_INIT,			/* uninitialized */
	SOCKET_NEW,			/* after socket() call */
	SOCKET_PASSIVE_LISTENING,	/* after listen() call */
	SOCKET_PASSIVE_PACKET_RECEIVED,	/* after receiving first packet */
	SOCKET_PASSIVE_SYNACK_SENT,	/* after sending SYNACK */
	SOCKET_PASSIVE_SYNACK_ACKED,	/* after server's SYN is ACKed */
	SOCKET_ACTIVE_CONNECTING,	/* after connect() call */
	SOCKET_ACTIVE_SYN_SENT,		/* after sending client's SYN */
	SOCKET_ACTIVE_SYN_ACKED,	/* after client's SYN is ACKed */
};

/* A TCP/UDP/IP address for an endpoint. */
struct endpoint {
	struct ip_address ip;		/* IP address */
	__be16 port;			/* TCP/UDP port (network order) */
};

/* The 4-tuple for a TCP/UDP/IP packet. */
struct tuple {
	struct endpoint src;
	struct endpoint dst;
};

/* Return true iff the two tuples are equal. */
static inline boolt is_equal_tuple(const struct tuple *a,
				  const struct tuple *b)
{
	return memcmp(a, b, sizeof(*a)) == 0;
}

/* Fill in the *dst_tuple with the tuple for packet flow in the
 * direction opposite that of *src_tuple
 */
static inline void reverse_tuple(const struct tuple *src_tuple,
				 struct tuple *dst_tuple)
{
	dst_tuple->src.ip	= src_tuple->dst.ip;
	dst_tuple->dst.ip	= src_tuple->src.ip;
	dst_tuple->src.port	= src_tuple->dst.port;
	dst_tuple->dst.port	= src_tuple->src.port;
}

/* Get the tuple for a packet. */
static inline void get_packet_tuple(const struct packet *packet,
				    struct tuple *tuple)
{
	memset(tuple, 0, sizeof(*tuple));
	if (packet->ipv4 != NULL) {
		ip_from_ipv4(&packet->ipv4->src_ip, &tuple->src.ip);
		ip_from_ipv4(&packet->ipv4->dst_ip, &tuple->dst.ip);
	} else if (packet->ipv6 != NULL) {
		ip_from_ipv6(&packet->ipv6->src_ip, &tuple->src.ip);
		ip_from_ipv6(&packet->ipv6->dst_ip, &tuple->dst.ip);
	} else {
		assert(!"bad IP version in packet");
	}
	if (packet->tcp != NULL) {
		tuple->src.port	= packet->tcp->src_port;
		tuple->dst.port	= packet->tcp->dst_port;
	} else if (packet->udp != NULL) {
		tuple->src.port	= packet->udp->src_port;
		tuple->dst.port	= packet->udp->dst_port;
	}
}

/* Set the tuple inside some TCP/IPv4 or TCP/IPv6 headers. */
static inline void set_headers_tuple(struct ipv4 *ipv4,
				     struct ipv6 *ipv6,
				     struct tcp *tcp,
				     struct udp *udp,
				     const struct tuple *tuple)
{
	if (ipv4 != NULL) {
		ip_to_ipv4(&tuple->src.ip, &ipv4->src_ip);
		ip_to_ipv4(&tuple->dst.ip, &ipv4->dst_ip);
	} else if (ipv6 != NULL) {
		ip_to_ipv6(&tuple->src.ip, &ipv6->src_ip);
		ip_to_ipv6(&tuple->dst.ip, &ipv6->dst_ip);
	} else {
		assert(!"bad IP version in packet");
	}
	if (tcp != NULL) {
		tcp->src_port = tuple->src.port;
		tcp->dst_port = tuple->dst.port;
	} else if (udp != NULL) {
		udp->src_port = tuple->src.port;
		udp->dst_port = tuple->dst.port;
	}
}

/* Set the tuple for a packet header echoed inside an ICMPv4/ICMPv6 message. */
static inline void set_icmp_echoed_tuple(struct packet *packet,
					 const struct tuple *tuple)
{
	/* All currently supported ICMP message types include a copy
	 * of the outbound IP header and the first few bytes inside,
	 * which so far always means the first ICMP_ECHO_BYTES of
	 * TCP header.
	 */
	DEBUGP("set_icmp_echoed_tuple");

	/* Flip the direction of the tuple, since the ICMP message is
	 * flowing in the direction opposite that of the echoed TCP/IP
	 * packet, and then fill in the fields of the echoed packet.
	 */
	struct tuple echoed_tuple;
	reverse_tuple(tuple, &echoed_tuple);
	set_headers_tuple(packet_echoed_ipv4_header(packet),
			  packet_echoed_ipv6_header(packet),
			  packet_echoed_tcp_header(packet),
			  packet_echoed_udp_header(packet),
			  &echoed_tuple);
}

/* Set the tuple for a packet. */
static inline void set_packet_tuple(struct packet *packet,
				    const struct tuple *tuple)
{
	set_headers_tuple(packet->ipv4, packet->ipv6, packet->tcp, packet->udp,
			  tuple);
	if ((packet->icmpv4 != NULL) || (packet->icmpv6 != NULL))
		set_icmp_echoed_tuple(packet, tuple);
}

#endif /* __SOCKET_H__ */
