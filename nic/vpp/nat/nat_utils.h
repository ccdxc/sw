//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_NAT_UTILS_H__
#define __VPP_NAT_UTILS_H__

typedef struct
{
  u16 identifier;
  u16 sequence;
} icmp_echo_header_t;

always_inline u8
icmp4_is_query_message(icmp46_header_t *icmp)
{
    switch (icmp->type) {
    case ICMP4_echo_reply:
    case ICMP4_echo_request:
    case ICMP4_timestamp_request:
    case ICMP4_information_request:
    case ICMP4_information_reply:
    case ICMP4_address_mask_request:
    case ICMP4_address_mask_reply:
    case ICMP4_traceroute:
    case ICMP4_domain_name_request:
    case ICMP4_domain_name_reply:
        return 1;
    }

    return 0;
}

always_inline u8
icmp4_is_error_message(icmp46_header_t *icmp)
{
    switch (icmp->type) {
    case ICMP4_destination_unreachable:
    case ICMP4_time_exceeded:
    case ICMP4_parameter_problem:
    case ICMP4_source_quench:
    case ICMP4_redirect:
    case ICMP4_alternate_host_address:
        return 1;
    }

    return 0;
}

#endif    // __VPP_NAT_UTILS_H__
