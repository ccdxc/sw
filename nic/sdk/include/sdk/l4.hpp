// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __L4_HPP__
#define __L4_HPP__

#include "include/sdk/types.hpp"
#include "ip.hpp"

#define UDP_HDR_LEN 8

typedef struct port_range_s {
    uint32_t    port_lo;
    uint32_t    port_hi;
} __PACK__ port_range_t;

typedef struct dhcp_option_s {
    uint8_t option;
    uint8_t length;
    union {
        uint8_t data[0];
        uint32_t data_as_u32[0];
    };
} __PACK__ dhcp_option_t;

typedef enum {
    DHCP_PACKET_DISCOVER = 1,
    DHCP_PACKET_OFFER,
    DHCP_PACKET_REQUEST,
    DHCP_PACKET_ACK = 5,
    DHCP_PACKET_NAK,
} dhcp_packet_type_t;

typedef enum {
    DHCP_PACKET_OPTION_REQ_IP_ADDR = 50,
    DHCP_PACKET_OPTION_MSG_TYPE = 53,
    DHCP_PACKET_OPTION_END = 0xff,
} dhcp_packet_option_t;

typedef struct dhcp_header_s {
    uint8_t opcode;                     // 1 = request, 2 = reply
    uint8_t hardware_type;              // 1 = ethernet
    uint8_t hardware_address_length;
    uint8_t hops;
    uint32_t transaction_identifier;
    uint16_t seconds;
    uint16_t flags;
#define DHCP_FLAG_BROADCAST (1<<15)
    ipv4_addr_t client_ip_address;
    ipv4_addr_t your_ip_address;
    ipv4_addr_t server_ip_address;
    ipv4_addr_t gateway_ip_address;
    uint8_t client_hardware_address[16];
    uint8_t server_name[64];
    uint8_t boot_filename[128];
    ipv4_addr_t magic_cookie;
    dhcp_option_t options[0];
} __PACK__ dhcp_header_t;

#endif    // __L4_HPP__

