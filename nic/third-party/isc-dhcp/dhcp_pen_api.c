/* dhcp_packet.cc*/

// clang-format off
#include "nic/third-party/isc-dhcp/includes/dhcp_internal.h"
#include "nic/third-party/isc-dhcp/includes/dhcp.h"

#include <netinet/if_ether.h>
#include <netinet/udp.h>

// clang-format on

/* Compute the easy part of the checksum on a range of bytes. */

bool dhcp_lib_init_done;

u_int32_t checksum(unsigned char *buf, unsigned nbytes, u_int32_t sum) {
    unsigned i;

    /* Checksum all the pairs of bytes first... */
    for (i = 0; i < (nbytes & ~1U); i += 2) {
        sum += (u_int16_t)ntohs(*((u_int16_t *)(buf + i)));
        /* Add carry. */
        if (sum > 0xFFFF) sum -= 0xFFFF;
    }

    /* If there's a single byte left over, checksum it, too.   Network
       byte order is big-endian, so the remaining byte is the high byte. */
    if (i < nbytes) {
        sum += buf[i] << 8;
        /* Add carry. */
        if (sum > 0xFFFF) sum -= 0xFFFF;
    }

    return sum;
}

/* Finish computing the checksum, and then put it into network byte order. */

u_int32_t wrapsum(u_int32_t sum) {
    sum = ~sum & 0xFFFF;

    return htons(sum);
}
/* Decode a hardware header... */

ssize_t decode_ethernet_header(unsigned char *buf, unsigned bufix,
                               struct hardware *from) {
    struct ether_header eh;

    memcpy(&eh, buf + bufix, ETHER_HDR_LEN);

    memcpy(&from->hbuf[1], eh.ether_shost, sizeof(eh.ether_shost));
    from->hbuf[0] = ARPHRD_ETHER;
    from->hlen = (sizeof eh.ether_shost) + 1;

    return ETHER_HDR_LEN;
}

/* UDP header and IP header decoded together for convenience. */

ssize_t
decode_udp_ip_header(struct interface_info *interface,
             unsigned char *buf, unsigned bufix,
             struct sockaddr_in *from, unsigned buflen,
             unsigned *rbuflen, int csum_ready)
{
  unsigned char *data;
  struct ip ip;
  struct udphdr udp;
  unsigned char *upp, *endbuf;
  u_int32_t ip_len, ulen, pkt_len;
  static unsigned int ip_packets_seen = 0;
  static unsigned int ip_packets_bad_checksum = 0;
  static unsigned int udp_packets_seen = 0;
  static unsigned int udp_packets_bad_checksum = 0;
  static unsigned int udp_packets_length_checked = 0;
  static unsigned int udp_packets_length_overflow = 0;
  unsigned len;

  /* Designate the end of the input buffer for bounds checks. */
  endbuf = buf + bufix + buflen;

  /* Assure there is at least an IP header there. */
  if ((buf + bufix + sizeof(ip)) > endbuf)
      return -1;

  /* Copy the IP header into a stack aligned structure for inspection.
   * There may be bits in the IP header that we're not decoding, so we
   * copy out the bits we grok and skip ahead by ip.ip_hl * 4.
   */
  upp = buf + bufix;
  memcpy(&ip, upp, sizeof(ip));
  ip_len = (*upp & 0x0f) << 2;
  upp += ip_len;

  /* Check the IP packet length. */
  pkt_len = ntohs(ip.ip_len);
  if (pkt_len > buflen)
    return -1;

  /* Assure after ip_len bytes that there is enough room for a UDP header. */
  if ((upp + sizeof(udp)) > endbuf)
      return -1;

  /* Copy the UDP header into a stack aligned structure for inspection. */
  memcpy(&udp, upp, sizeof(udp));

#ifdef USERLAND_FILTER
  /* Is it a UDP packet? */
  if (ip.ip_p != IPPROTO_UDP)
      return -1;

  /* Is it to the port we're serving? */
  if (udp.dest != local_port)
      return -1;
#endif /* USERLAND_FILTER */

  ulen = ntohs(udp.len);
  if (ulen < sizeof(udp))
    return -1;

  udp_packets_length_checked++;
  if ((upp + ulen) > endbuf) {
    udp_packets_length_overflow++;
    if (((udp_packets_length_checked > 4) &&
         (udp_packets_length_overflow != 0)) &&
        ((udp_packets_length_checked / udp_packets_length_overflow) < 2)) {
        HAL_TRACE_INFO("%u udp packets in %u too long - dropped",
                       udp_packets_length_overflow, udp_packets_length_checked);
        udp_packets_length_overflow = 0;
        udp_packets_length_checked = 0;
    }
    return -1;
  }

  /* Check the IP header checksum - it should be zero. */
  ip_packets_seen++;
  if (wrapsum (checksum (buf + bufix, ip_len, 0))) {
      ++ip_packets_bad_checksum;
      if (((ip_packets_seen > 4) && (ip_packets_bad_checksum != 0)) &&
          ((ip_packets_seen / ip_packets_bad_checksum) < 2)) {
          HAL_TRACE_INFO("%u bad IP checksums seen in %u packets",
                         ip_packets_bad_checksum, ip_packets_seen);
          ip_packets_seen = ip_packets_bad_checksum = 0;
      }
      return -1;
  }

  /* Copy out the IP source address... */
  memcpy(&from->sin_addr, &ip.ip_src, 4);

  data = upp + sizeof(udp);
  len = ulen - sizeof(udp);

  /* UDP check sum may be optional (udp.uh_sum == 0) or not ready if checksum
   * offloading is in use */
  udp_packets_seen++;
  //* Ignore check sum for now */
  udp.check = 0;
  if (udp.check && csum_ready) {
    /* Check the UDP header checksum - since the received packet header
     * contains the UDP checksum calculated by the transmitter, calculating
     * it now should come out to zero. */
    if (wrapsum(checksum((unsigned char *)&udp, sizeof(udp),
                         checksum(data, len,
                                  checksum((unsigned char *)&ip.ip_src, 8,
                                           IPPROTO_UDP + ulen))))) {
        udp_packets_bad_checksum++;
        if (((udp_packets_seen > 4) && (udp_packets_bad_checksum != 0))
            && ((udp_packets_seen / udp_packets_bad_checksum) < 2)) {
            HAL_TRACE_INFO("%u bad udp checksums in %u packets",
                           udp_packets_bad_checksum, udp_packets_seen);
            udp_packets_seen = udp_packets_bad_checksum = 0;
        }

        return -1;
    }
  }

  /* If at least 5 with less than 50% bad, start over */
  if (udp_packets_seen > 4) {
    udp_packets_bad_checksum = 0;
    udp_packets_seen = 0;
  }

  /* Copy out the port... */
  memcpy (&from -> sin_port, &udp.source, sizeof udp.source);

  /* Save the length of the UDP payload. */
  if (rbuflen != NULL)
    *rbuflen = len;

  /* Return the index to the UDP payload. */
  return ip_len + sizeof udp;
}

ssize_t extract_dhcp_payload_offset(uint8_t *buf, uint32_t len,
                                     uint32_t *pkt_len) {
    struct hardware from;
    ssize_t offset = 0;
    uint32_t bufix = 0;
    uint32_t paylen;
    struct sockaddr_in in_from;

    offset = decode_udp_ip_header(NULL, buf, bufix, &in_from,
                        len, &paylen, 1);

    if (offset < 0) {
        return offset;
    }

    bufix += offset;
    len -= offset;

    *pkt_len = paylen;

    return bufix;
}

hal_ret_t decode_dhcp_packet(struct dhcp_packet *packet, uint32_t pkt_len,
                              struct packet **decoded_packet) {
    struct packet *temp_packet;

     if (!packet_allocate(&temp_packet, MDL)) {
            return HAL_RET_NO_RESOURCE;
        }

        temp_packet->raw = packet;
        temp_packet->packet_length = pkt_len;

        /* Allocate packet->options now so it is non-null for all packets */
        temp_packet->options_valid = 0;
        if (!option_state_allocate(&temp_packet->options, MDL)) {
            return HAL_RET_ERR;
        }

        if (temp_packet->packet_length >= DHCP_FIXED_NON_UDP + 4) {
            if (!parse_options(temp_packet)) {
                packet_dereference(&temp_packet, MDL);
                return HAL_RET_ERR;
            }
   }

   *decoded_packet = temp_packet;
   return HAL_RET_OK;
}

hal_ret_t dhcp_lookup_option(const struct packet *decoded_packet, unsigned code,
                        struct option_data *data) {
    if (data == NULL || decoded_packet == NULL) {
        return HAL_RET_INVALID_ARG;
    }

    struct option_cache *cache =
        lookup_hashed_option(&dhcp_universe, decoded_packet->options, code);

    if (cache == NULL) {
        return HAL_RET_ENTRY_NOT_FOUND;
    }

    data->len = cache->data.len;
    data->data = cache->data.data;

    return HAL_RET_OK;
}

void free_packet(struct packet **packet)
{
    packet_dereference(packet, MDL);
}

void dhcp_lib_init() {
    if (!dhcp_lib_init_done) {
        initialize_common_option_spaces();
        dhcp_lib_init_done = true;
    }
}
