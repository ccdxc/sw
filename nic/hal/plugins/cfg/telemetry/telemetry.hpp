//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __TELEMETRY_HPP__
#define __TELEMETRY_HPP__

#include <google/protobuf/util/json_util.h>
#include "nic/include/pd.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/plugins/cfg/nw/interface_api.hpp"
#include "gen/proto/telemetry.grpc.pb.h"
#include "nic/hal/src/utils/rule_match.hpp"

using telemetry::Telemetry;
using telemetry::MirrorSessionSpec;
using telemetry::MirrorSessionStatus;
using telemetry::MirrorSessionResponse;
using telemetry::MirrorSessionRequestMsg;
using telemetry::MirrorSessionResponseMsg;
using telemetry::MirrorSessionDeleteRequest;
using telemetry::MirrorSessionDeleteResponse;
using telemetry::MirrorSessionDeleteRequestMsg;
using telemetry::MirrorSessionDeleteResponseMsg;
using telemetry::MirrorSessionGetRequest;
using telemetry::MirrorSessionGetRequestMsg;
using telemetry::MirrorSessionGetResponse;
using telemetry::MirrorSessionGetResponseMsg;

using telemetry::CollectorSpec;
using telemetry::CollectorStatus;
using telemetry::CollectorResponse;
using telemetry::CollectorRequestMsg;
using telemetry::CollectorResponseMsg;
using telemetry::CollectorDeleteRequest;
using telemetry::CollectorDeleteResponse;
using telemetry::CollectorDeleteRequestMsg;
using telemetry::CollectorDeleteResponseMsg;
using telemetry::CollectorGetRequest;
using telemetry::CollectorGetRequestMsg;
using telemetry::CollectorGetResponse;
using telemetry::CollectorGetResponseMsg;

using telemetry::FlowMonitorRuleSpec;
using telemetry::FlowMonitorRuleStatus;
using telemetry::FlowMonitorRuleResponse;
using telemetry::FlowMonitorRuleRequestMsg;
using telemetry::FlowMonitorRuleResponseMsg;
using telemetry::FlowMonitorRuleDeleteRequest;
using telemetry::FlowMonitorRuleDeleteResponse;
using telemetry::FlowMonitorRuleDeleteRequestMsg;
using telemetry::FlowMonitorRuleDeleteResponseMsg;
using telemetry::FlowMonitorRuleGetRequest;
using telemetry::FlowMonitorRuleGetRequestMsg;
using telemetry::FlowMonitorRuleGetResponse;
using telemetry::FlowMonitorRuleGetResponseMsg;

using telemetry::DropMonitorRuleSpec;
using telemetry::DropMonitorRuleStatus;
using telemetry::DropMonitorRuleResponse;
using telemetry::DropMonitorRuleRequestMsg;
using telemetry::DropMonitorRuleResponseMsg;
using telemetry::DropMonitorRuleDeleteRequest;
using telemetry::DropMonitorRuleDeleteResponse;
using telemetry::DropMonitorRuleDeleteRequestMsg;
using telemetry::DropMonitorRuleDeleteResponseMsg;
using telemetry::DropMonitorRuleGetRequest;
using telemetry::DropMonitorRuleGetRequestMsg;
using telemetry::DropMonitorRuleGetResponse;
using telemetry::DropMonitorRuleGetResponseMsg;

namespace hal {

#define MAX_FLOW_MONITOR_RULES      1024

using hal::if_t;
using hal::lif_t;

#define MIRROR_DESTTYPES(ENTRY)                                  \
    ENTRY(MIRROR_DEST_NONE,     0,  "Disabled")                  \
    ENTRY(MIRROR_DEST_LOCAL,    1,  "Local Destination")         \
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

// Session ID used by L7 app redirect (visibility mode)
#define MIRROR_SESSION_APP_REDIR_VISIB_ID    7
// Iris pipeline can support upto 8 mirror destinations
#define MAX_MIRROR_SESSION_DEST     8
#define MAX_DROP_REASON             128
#define MAX_COLLECTORS              16
#define MAX_COLLECTORS_PER_FLOW     4


// New reason codes must be added here and in the corresponding PD
typedef struct drop_reason_codes_s {
    bool    drop_malformed_pkt;
    bool    drop_input_mapping;
    bool    drop_input_mapping_dejavu;
    bool    drop_flow_hit;
    bool    drop_flow_miss;
    bool    drop_nacl;
    bool    drop_ipsg;
    bool    drop_ip_normalization;
    bool    drop_tcp_normalization;
    bool    drop_tcp_rst_with_invalid_ack_num;
    bool    drop_tcp_non_syn_first_pkt;
    bool    drop_icmp_normalization;
    bool    drop_input_properties_miss;
    bool    drop_tcp_out_of_window;
    bool    drop_tcp_split_handshake;
    bool    drop_tcp_win_zero_drop;
    bool    drop_tcp_data_after_fin;
    bool    drop_tcp_non_rst_pkt_after_rst;
    bool    drop_tcp_invalid_responder_first_pkt;
    bool    drop_tcp_unexpected_pkt;
    bool    drop_src_lif_mismatch;
    bool    drop_parser_icrc_error;
    bool    drop_parse_len_error;
    bool    drop_hardware_error;
} __PACK__ drop_reason_codes_t;

static inline void
drop_reason_spec_to_codes (const types::DropReasons &spec,
                           drop_reason_codes_t *codes)
{
    codes->drop_input_mapping = spec.drop_input_mapping();
    codes->drop_input_mapping_dejavu = spec.drop_input_mapping_dejavu();
    codes->drop_flow_hit = spec.drop_flow_hit();
    codes->drop_flow_miss = spec.drop_flow_miss();
    codes->drop_ipsg = spec.drop_ipsg();
    codes->drop_nacl = spec.drop_nacl();
    codes->drop_malformed_pkt = spec.drop_malformed_pkt();
    codes->drop_ip_normalization = spec.drop_ip_normalization();
    codes->drop_tcp_normalization = spec.drop_tcp_normalization();
    codes->drop_tcp_non_syn_first_pkt = spec.drop_tcp_non_syn_first_pkt();
    codes->drop_icmp_normalization = spec.drop_icmp_normalization();
    codes->drop_input_properties_miss = spec.drop_input_properties_miss();
    codes->drop_tcp_out_of_window = spec.drop_tcp_out_of_window();
    codes->drop_tcp_split_handshake = spec.drop_tcp_split_handshake();
    codes->drop_tcp_win_zero_drop = spec.drop_tcp_win_zero_drop();
    codes->drop_tcp_data_after_fin = spec.drop_tcp_data_after_fin();
    codes->drop_tcp_non_rst_pkt_after_rst = spec.drop_tcp_non_rst_pkt_after_rst();
    codes->drop_tcp_invalid_responder_first_pkt = spec.drop_tcp_invalid_responder_first_pkt();
    codes->drop_tcp_unexpected_pkt = spec.drop_tcp_unexpected_pkt();
    codes->drop_src_lif_mismatch = spec.drop_src_lif_mismatch();
    codes->drop_parser_icrc_error = spec.drop_parser_icrc_error();
    codes->drop_parse_len_error = spec.drop_parse_len_error();
    codes->drop_hardware_error = spec.drop_hardware_error();
    return;
}

typedef struct drop_monitor_rule_s {
    hal_spinlock_t slock;
    uint32_t rule_id;
    drop_reason_codes_t codes;
    bool mirror_destinations[MAX_MIRROR_SESSION_DEST];
} __PACK__ drop_monitor_rule_t;

typedef struct flow_monitor_rule_action_s {
    uint8_t     num_mirror_dest;    // Num of mirror sessions
    uint8_t     num_collector;  // Number of collectors
    uint8_t     collectors[MAX_COLLECTORS_PER_FLOW]; // Collector ids
    uint8_t     mirror_destinations[MAX_MIRROR_SESSION_DEST];   // Mirror session ids
    bool        mirror_to_cpu;       // Mirror to cpu - additional mirror dest
} __PACK__ flow_monitor_rule_action_t;

typedef struct flow_monitor_rule_s {
    hal_spinlock_t  slock;
    uint64_t        vrf_id;
    uint32_t        rule_id;
    rule_match_t    rule_match;
    // Actions
    flow_monitor_rule_action_t action;
    acl::ref_t  ref_count;
} flow_monitor_rule_t;

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
    uint64_t            collector_id;
    uint16_t            vlan;
    l2seg_t             *l2seg;
    mac_addr_t          dest_mac;
    mac_addr_t          src_mac;
    ip_addr_t           src_ip;
    ip_addr_t           dst_ip;
    uint16_t            protocol;
    uint16_t            dport;
    uint32_t            template_id;
    uint32_t            export_intvl;
    export_formats_en   format;
} collector_config_t;

static inline flow_monitor_rule_t *
flow_monitor_rule_alloc(void)
{
    flow_monitor_rule_t *rule = NULL;

    rule = (flow_monitor_rule_t *)
                g_hal_state->flowmon_rule_slab()->alloc();
    if (rule == NULL) {
        return NULL;
    }
    return rule;
}

// Initialize a flow_monitor_rule instance
static inline flow_monitor_rule_t *
flow_monitor_rule_init (flow_monitor_rule_t *rule)
{
    if (!rule) {
        return NULL;
    }

    ref_init(&rule->ref_count, [] (const ref_t * ref) {
        flow_monitor_rule_t * rule = container_of(ref, flow_monitor_rule_t, ref_count);
        g_hal_state->flowmon_rule_slab()->free(rule);
    });

    rule_match_init(&rule->rule_match);
    return rule;
}

// allocate and initialize a match_template
static inline flow_monitor_rule_t *
flow_monitor_rule_alloc_init()
{
    return (flow_monitor_rule_init(flow_monitor_rule_alloc()));
}

// allocate and initialize a match_template
static inline void
flow_monitor_rule_free(flow_monitor_rule_t *rule)
{
    /* Decrementing the rule ref_count */
    ref_dec(&rule->ref_count);
    return;
}

static inline const char *
flowmon_acl_ctx_name (vrf_id_t vrf_id)
{
    thread_local static char name[ACL_NAMESIZE];
    std::snprintf(name, sizeof(name), "flowmon-rules:%lu", vrf_id);
    return name;
}

static inline void
mirrorsession_spec_dump (MirrorSessionSpec& spec)
{
    std::string    cfg;

    if (hal::utils::hal_trace_level() < ::utils::trace_debug) {
        return;
    }
    google::protobuf::util::MessageToJsonString(spec, &cfg);
    HAL_TRACE_DEBUG("MirrorSessionSpec configuration:");
    HAL_TRACE_DEBUG("{}", cfg.c_str());
}

static inline void
collector_spec_dump (CollectorSpec& spec)
{
    std::string    cfg;

    if (hal::utils::hal_trace_level() < ::utils::trace_debug) {
        return;
    }
    google::protobuf::util::MessageToJsonString(spec, &cfg);
    HAL_TRACE_DEBUG("CollectorSpec configuration:");
    HAL_TRACE_DEBUG("{}", cfg.c_str());
}

static inline void
flowmonrule_spec_dump (FlowMonitorRuleSpec& spec)
{
    std::string    cfg;

    if (hal::utils::hal_trace_level() < ::utils::trace_debug) {
        return;
    }
    google::protobuf::util::MessageToJsonString(spec, &cfg);
    HAL_TRACE_DEBUG("FlowMonitorRuleSpec configuration:");
    HAL_TRACE_DEBUG("{}", cfg.c_str());
}

static inline void
dropmonrule_spec_dump (DropMonitorRuleSpec& spec)
{
    std::string    cfg;

    if (hal::utils::hal_trace_level() < ::utils::trace_debug) {
        return;
    }
    google::protobuf::util::MessageToJsonString(spec, &cfg);
    HAL_TRACE_DEBUG("DropMonitorRuleSpec configuration:");
    HAL_TRACE_DEBUG("{}", cfg.c_str());
}

hal_ret_t mirror_session_create(MirrorSessionSpec &spec, MirrorSessionResponse *rsp);
hal_ret_t mirror_session_update(MirrorSessionSpec &spec, MirrorSessionResponse *rsp);
hal_ret_t mirror_session_delete(MirrorSessionDeleteRequest &spec, MirrorSessionDeleteResponse *rsp);
hal_ret_t mirror_session_get(MirrorSessionGetRequest &req, MirrorSessionGetResponseMsg *rsp);

hal_ret_t collector_create(CollectorSpec &spec, CollectorResponse *rsp);
hal_ret_t collector_update(CollectorSpec &spec, CollectorResponse *rsp);
hal_ret_t collector_delete(CollectorDeleteRequest &req, CollectorDeleteResponse *rsp);
hal_ret_t collector_get(CollectorGetRequest &req, CollectorGetResponseMsg *rsp);

hal_ret_t flow_monitor_rule_create(FlowMonitorRuleSpec &spec, FlowMonitorRuleResponse *rsp);
hal_ret_t flow_monitor_rule_update(FlowMonitorRuleSpec &spec, FlowMonitorRuleResponse *rsp);
hal_ret_t flow_monitor_rule_delete(FlowMonitorRuleDeleteRequest &req, FlowMonitorRuleDeleteResponse *rsp);
hal_ret_t flow_monitor_rule_get(FlowMonitorRuleGetRequest &req, FlowMonitorRuleGetResponseMsg *rsp);

hal_ret_t drop_monitor_rule_create(DropMonitorRuleSpec &spec, DropMonitorRuleResponse *rsp);
hal_ret_t drop_monitor_rule_update(DropMonitorRuleSpec &spec, DropMonitorRuleResponse *rsp);
hal_ret_t drop_monitor_rule_delete(DropMonitorRuleDeleteRequest &req, DropMonitorRuleDeleteResponse *rsp);
hal_ret_t drop_monitor_rule_get(DropMonitorRuleGetRequest &req, DropMonitorRuleGetResponseMsg *rsp);

hal_ret_t flow_monitor_acl_ctx_create(void);

}    // namespace

#endif    // __TELEMETRY_HPP__

