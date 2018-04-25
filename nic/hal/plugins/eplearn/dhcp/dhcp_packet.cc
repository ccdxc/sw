//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <netinet/in.h>
// clang-format off
#include "dhcp_packet.hpp"
namespace hal {
namespace eplearn {
hal_ret_t parse_dhcp_packet(const uint8_t *buf, uint32_t len,
                            struct packet **decoded_packet) {
    uint32_t payload_len;
    struct dhcp_packet *raw;

    ssize_t offset = extract_dhcp_payload_offset((uint8_t*)buf,
            len, &payload_len);

    if (offset <= 0) {
        return HAL_RET_ERR;
    }

    raw = (struct dhcp_packet *)(&buf[offset]);

    return decode_dhcp_packet(raw, payload_len, decoded_packet);
}

void free_dhcp_packet(struct packet **packet) {
    free_packet(packet);
}
}
}
