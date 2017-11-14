#ifndef __TELEMETRY_HPP__
#define __TELEMETRY_HPP__
#include "nic/include/pd.hpp"
#include "nic/hal/src/interface.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/gen/proto/hal/telemetry.grpc.pb.h"

using telemetry::Telemetry;
using telemetry::MirrorSessionSpec;
using telemetry::MirrorSession;
using telemetry::MirrorSessionId;
using telemetry::CollectorSpec;
using telemetry::Collector;
using telemetry::ExportControlId;

namespace hal {

using hal::if_t;
using hal::lif_t;

typedef uint8_t mirror_session_id_t;

#define MIRROR_DESTTYPES(ENTRY)                                  \
    ENTRY(MIRROR_DEST_NONE,     0,  "Disabled")                  \
    ENTRY(MIRROR_DEST_LOCAL,    1,  "Local Destination")          \
    ENTRY(MIRROR_DEST_RSPAN,    2,  "Remote SPAN destination")   \
    ENTRY(MIRROR_DEST_ERSPAN,   3,  "ERSPAN destination")        \
    ENTRY(MIRROR_DEST_MAX,      4,  "Invalid") 

DEFINE_ENUM(mirror_desttype_en, MIRROR_DESTTYPES)
#undef MIRROR_DESTTYPES

#define EXPORT_FORMATS(ENTRY)                                   \
    ENTRY(EXPORT_FORMAT_NONE,       0,  "None")                 \
    ENTRY(EXPORT_FORMAT_IPFIX,      1,  "IPFIx Format")         \
    ENTRY(EXPORT_FORMAT_NETFLOW9,   2,  "Netflow v9 Format")

DEFINE_ENUM(export_formats_en, EXPORT_FORMATS)
#undef EXPORT_FORMATS

typedef struct mirror_session_s {
    hal_spinlock_t slock;
    mirror_session_id_t id;
    mirror_desttype_en type;
    uint32_t truncate_len;
    if_t *dest_if;
    union {
        struct er_span_dest_ {
            if_t *tunnel_if;
        } er_span_dest;
      struct r_span_dest_ {
            vlan_id_t vlan;
        } r_span_dest;
    } mirror_destination_u;
} __PACK__ mirror_session_t;


typedef struct collector_config_s {
    uint64_t            exporter_id;
    uint16_t            vlan;
    l2seg_t             *l2seg;
    mac_addr_t          dest_mac;
    mac_addr_t          src_mac;
    ip_addr_t           src_ip;
    ip_addr_t           dst_ip;
    uint16_t            protocol;
    uint16_t            dport;
    uint32_t            template_id;
    export_formats_en   format;
} collector_config_t;

hal_ret_t mirror_session_create(MirrorSessionSpec *spec, MirrorSession *rsp);
hal_ret_t mirror_session_delete(MirrorSessionId *id, MirrorSession *rsp);
hal_ret_t collector_create(CollectorSpec *spec, Collector *resp);
hal_ret_t collector_update(CollectorSpec *spec, Collector *resp);
hal_ret_t collector_get(ExportControlId *id, Collector *resp);
hal_ret_t collector_delete(ExportControlId *id, Collector *resp);
}

#endif // __TELEMETRY_HPP__
