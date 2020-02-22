// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_TELEMETRY_PD_HPP__
#define __HAL_TELEMETRY_PD_HPP__

#include "nic/include/base.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/plugins/cfg/telemetry/telemetry.hpp"
#include "nic/hal/pd/cpupkt_headers.hpp"

#define TELEMETRY_EXPORT_BUFF_SIZE      2047
#define TELEMETRY_IPFIX_BUFSIZE         2048
#define TELEMETRY_IPFIX_HBM_MEMSIZE     (32 * 1024)
#define TELEMETRY_NUM_EXPORT_DEST       (TELEMETRY_IPFIX_HBM_MEMSIZE/TELEMETRY_IPFIX_BUFSIZE)

namespace hal {
namespace pd {

typedef struct telemetry_pd_export_buf_header_s telemetry_pd_export_buf_header_t;
struct telemetry_pd_export_buf_header_s {
    uint16_t packet_start;
    uint16_t payload_length;
    uint16_t payload_start;
    uint16_t ip_hdr_start;
} __attribute__ ((__packed__));

typedef struct telemetry_pd_ipfix_header_s telemetry_pd_ipfix_header_t;
struct telemetry_pd_ipfix_header_s {
    vlan_header_t vlan;
    ipv4_header_t iphdr;
    udp_header_t  udphdr;
} __attribute__ ((__packed__));

typedef struct telemetry_export_dest_s telemetry_export_dest_t;
struct telemetry_export_dest_s {
    bool                             valid;
    uint16_t                         id;
    void                             *buffer;
    uint64_t                         base_addr;
    uint32_t                         template_id;
    uint32_t                         export_intvl;
    void                             *db_timer;
    telemetry_pd_export_buf_header_t buf_hdr;
    // IPFix Header defined and maintained
    telemetry_pd_ipfix_header_t      ipfix_hdr;
} __attribute__ ((__packed__));

typedef struct mirror_session_pd_s mirror_session_pd_t;
struct mirror_session_pd_s {
    uint32_t hw_id;
    uint64_t pad;
} __PACK__;

}    // namespace pd
}    // namespace hal

#endif    //__HAL_TELEMETRY_PD_HPP__
