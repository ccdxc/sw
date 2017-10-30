#ifndef DHCP_PACKET_H
#define DHCP_PACKET_H

#include "nic/third-party/isc-dhcp/includes/dhcp.h"
namespace hal {
namespace network {

hal_ret_t parse_dhcp_packet(const uint8_t *buf, uint32_t len,
                            struct packet **decoded_packet);

void free_dhcp_packet(struct packet **decoded_packet);

hal_ret_t lookup_option(struct packet *decoded_packet, unsigned int code,
                        struct option_data *data);
}  // namespace network
}  // namespace hal
#endif /* DHCP_PACKET_H */

