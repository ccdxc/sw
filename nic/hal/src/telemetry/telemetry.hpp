#ifndef __TELEMETRY_HPP__
#define __TELEMETRY_HPP__

#include "nic/include/pd.hpp"
#include "nic/hal/src/nw/interface.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/gen/proto/hal/telemetry.grpc.pb.h"

using telemetry::Telemetry;
using telemetry::MirrorSessionSpec;
using telemetry::MirrorSession;
using telemetry::MirrorSessionId;
using telemetry::MirrorSessionStatus;
using telemetry::CollectorSpec;
using telemetry::Collector;
using telemetry::ExportControlId;
using telemetry::FlowMonitorRule;
using telemetry::FlowMonitorRuleSpec;
using telemetry::FlowMonitorRuleStatus;
using telemetry::DropMonitorRule;
using telemetry::DropMonitorRuleSpec;
using telemetry::DropMonitorRuleStatus;

namespace hal {

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

typedef struct drop_monitor_rule_s {
    hal_spinlock_t slock;
    drop_reason_codes_t codes;
    bool mirror_destinations[MAX_MIRROR_SESSION_DEST];
} __PACK__ drop_monitor_rule_t;

typedef struct flow_monitor_rule_s {
    hal_spinlock_t slock;
    uint64_t vrf_id;
    // Flow key fields
    union {
        struct {
            mac_addr_t src_mac;
            mac_addr_t dst_mac;
            uint16_t ethertype;
            bool src_mac_valid;
            bool dst_mac_valid;
            bool ethertype_valid;;
        };
        struct {
            ip_prefix_t sip;
            ip_prefix_t dip;
            uint8_t proto;
            uint32_t sport;
            uint32_t dport;
            bool sip_valid;
            bool dip_valid;
            bool proto_valid;
            bool sport_valid;
            bool dport_valid;
        };
    };
    // Source and Dest workload group ids
    uint64_t src_groupid;
    uint64_t dst_groupid;
    uint8_t mirror_destinations[MAX_MIRROR_SESSION_DEST];
    bool src_groupid_valid;
    bool dst_groupid_valid;
    // Is it a collect action ?
    bool collect_flow_action;
    // Mirror to cpu - additional mirror destination
    bool mirror_to_cpu;
} __PACK__ flow_monitor_rule_t;

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

hal_ret_t hal_telemetry_init_cb(hal_cfg_t *hal_cfg);
hal_ret_t hal_telemetry_cleanup_cb(void);
hal_ret_t mirror_session_create(MirrorSessionSpec *spec, MirrorSession *rsp);
hal_ret_t mirror_session_delete(MirrorSessionId *id, MirrorSession *rsp);
hal_ret_t collector_create(CollectorSpec *spec, Collector *resp);
hal_ret_t collector_update(CollectorSpec *spec, Collector *resp);
hal_ret_t collector_get(ExportControlId *id, Collector *resp);
hal_ret_t collector_delete(ExportControlId *id, Collector *resp);
hal_ret_t flow_monitor_rule_create(FlowMonitorRuleSpec *spec, FlowMonitorRule *rsp);
hal_ret_t flow_monitor_rule_delete(FlowMonitorRuleSpec *spec, FlowMonitorRule *rsp);
hal_ret_t flow_monitor_rule_get(FlowMonitorRuleSpec *spec, FlowMonitorRule *rsp);
hal_ret_t drop_monitor_rule_create(DropMonitorRuleSpec *spec, DropMonitorRule *rsp);
hal_ret_t drop_monitor_rule_delete(DropMonitorRuleSpec *spec, DropMonitorRule *rsp);
hal_ret_t drop_monitor_rule_get(DropMonitorRuleSpec *spec, DropMonitorRule *rsp);

}    // namespace

#endif    // __TELEMETRY_HPP__

