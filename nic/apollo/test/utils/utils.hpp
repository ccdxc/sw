//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the  test utility routines
///
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_UTILS_HPP__
#define __TEST_UTILS_UTILS_HPP__

#include <vector>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/platform/capri/capri_p4.hpp" //UPLINK_0/1

namespace api_test {

static inline void
extract_ip_addr (const char *ip, uint8_t af, ip_addr_t *ip_addr)
{
    if (af == IP_AF_IPV4) {
        struct in_addr v4;
        inet_aton(ip, &v4);
        ip_addr->af = IP_AF_IPV4;
        ip_addr->addr.v4_addr = ntohl(v4.s_addr);
    } else if (af == IP_AF_IPV6) {
        struct in6_addr v6;
        inet_pton(AF_INET6, ip, &v6);
        ip_addr->af = IP_AF_IPV6;
        for (int i = 0; i < 16; i++) {
            // Address in network byte order
            ip_addr->addr.v6_addr.addr8[i] = v6.s6_addr[15 - i];
        }
    } else {
        SDK_ASSERT(0);
    }
}

/// \brief Packet send function
///
/// \param tx_pkt          Packet to be trasnmited
/// \param tx_pkt_len      Transmit packet size in bytes
/// \param tx_port         Trasmit port TM_PORT_UPLINK_0/TM_PORT_UPLINK_1
/// \param exp_rx_pkt      Expected output on the receive port
/// \param exp_rx_pkt_len  Expected packet size in bytes
/// \param exp_rx_port     Expeted receive port TM_PORT_UPLINK_0/TM_PORT_UPLINK_1
void send_packet(const uint8_t *tx_pkt, uint32_t tx_pkt_len, uint32_t tx_port,
                 const uint8_t *exp_rx_pkt, uint32_t exp_rx_pkt_len,
                 uint32_t exp_rx_port);

/// \brief Packet dump function
///
/// param data Data to be printed
void dump_packet(std::vector<uint8_t> data);

}    // namespace api_test

#endif    // __TEST_UTILS_VNIC_HPP__
