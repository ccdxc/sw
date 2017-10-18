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

typedef struct mirror_session_s {
    hal_spinlock_t slock;
    mirror_session_id_t id;
    mirror_desttype_en type;
    uint32_t truncate_len;
    if_t dest_if;
    union {
        struct er_span_dest_ {
            uint32_t tunnel_if_id;
            // rewrite index
        } er_span_dest;
      struct r_span_dest_ {
            vlan_id_t vlan;
        } r_span_dest;
    } mirror_destination_u;
} __PACK__ mirror_session_t;

hal_ret_t mirror_session_create(MirrorSessionSpec *spec, MirrorSession *rsp);
hal_ret_t mirror_session_delete(MirrorSessionId *id, MirrorSession *rsp);
}

#endif // __TELEMETRY_HPP__
