//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the main for the athena_app daemon
///
//----------------------------------------------------------------------------

#include <unistd.h>
#include <getopt.h>
#include <limits.h>
#include <string>
#include <iostream>
#include <stdarg.h>
#include <sys/stat.h>
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/sdk/platform/capri/capri_p4.hpp"
#include "nic/sdk/model_sim/include/lib_model_client.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "gen/p4gen/p4/include/p4pd.h"
//#include "nic/apollo/p4/include/athena_defines.h"
#include "nic/apollo/p4/include/athena_table_sizes.h"
#include "trace.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "gen/p4gen/p4/include/ftl_table.hpp"
#include "nic/apollo/api/include/athena/pds_vnic.h"
#include "nic/apollo/api/include/athena/pds_flow_cache.h"
#include "nic/apollo/api/include/athena/pds_flow_session.h"

namespace core {
// number of trace files to keep
#define TRACE_NUM_FILES                        5
#define TRACE_FILE_SIZE                        (20 << 20)
static inline string
log_file (const char *logdir, const char *logfile)
{
    struct stat st = { 0 };

    if (!logdir) {
        return std::string(logfile);
    }

    // check if this log dir exists
    if (stat(logdir, &st) == -1) {
        // doesn't exist, try to create
        if (mkdir(logdir, 0755) < 0) {
            fprintf(stderr,
                    "Log directory %s/ doesn't exist, failed to create "
                    "one\n", logdir);
            return std::string("");
        }
    } else {
        // log dir exists, check if we have write permissions
        if (access(logdir, W_OK) < 0) {
            // don't have permissions to create this directory
            fprintf(stderr,
                    "No permissions to create log file in %s/\n",
                    logdir);
            return std::string("");
        }
    }
    return logdir + std::string(logfile);
}

//------------------------------------------------------------------------------
// initialize the logger
//------------------------------------------------------------------------------
static inline sdk_ret_t
logger_init (void)
{
    std::string logfile, err_logfile;

    logfile = log_file(std::getenv("LOG_DIR"), "./pds-athena-agent.log");
    err_logfile = log_file(std::getenv("PERSISTENT_LOG_DIR"), "/obfl.log");

    if (logfile.empty() || err_logfile.empty()) {
        return SDK_RET_ERR;
    }

    // initialize the logger
    core::trace_init("agent", 0x1, true, err_logfile.c_str(), logfile.c_str(),
                     TRACE_FILE_SIZE, TRACE_NUM_FILES, utils::trace_verbose);

    return SDK_RET_OK;
}

//------------------------------------------------------------------------------
// logger callback passed to SDK and PDS lib
//------------------------------------------------------------------------------
static int
sdk_logger (sdk_trace_level_e tracel_level, const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    switch (tracel_level) {
    case sdk::lib::SDK_TRACE_LEVEL_ERR:
        PDS_TRACE_ERR_NO_META("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_WARN:
        PDS_TRACE_WARN_NO_META("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_INFO:
        PDS_TRACE_INFO_NO_META("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_DEBUG:
        PDS_TRACE_DEBUG_NO_META("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_VERBOSE:
        PDS_TRACE_VERBOSE_NO_META("{}", logbuf);
        break;
    default:
        break;
    }
    va_end(args);

    return 0;
}
} // namespace core

#if 0
using sdk::table::sdk_table_api_params_t;
using sdk::table::sdk_table_api_stats_t;
using sdk::table::sdk_table_stats_t;
using sdk::table::sdk_table_factory_params_t;
#endif

sdk_ret_t
flow_table_init(void)
{
    return pds_flow_cache_create(0);
}

void dump_pkt(std::vector<uint8_t> &pkt)
{
    for (std::vector<uint8_t>::iterator it = pkt.begin() ; it != pkt.end(); ++it) {
        printf("%02x", *it);
    }
    printf("\n");
}



sdk_ret_t send_packet(const char *out_pkt_descr, uint8_t *out_pkt, uint16_t out_pkt_len, uint32_t out_port,
        uint8_t *in_pkt, uint16_t in_pkt_len, uint32_t in_port)
{
    uint32_t                port;
    uint32_t                cos = 0;
    std::vector<uint8_t>    ipkt;
    std::vector<uint8_t>    opkt;
    std::vector<uint8_t>    epkt;

    if (out_pkt_descr)
        printf("Test with Pkt:%s\n", out_pkt_descr);

    ipkt.resize(out_pkt_len);
    memcpy(ipkt.data(), out_pkt, out_pkt_len);

    printf("Sending Packet on port: %d\n", out_port);
    dump_pkt(ipkt);

    step_network_pkt(ipkt, out_port);

    get_next_pkt(opkt, port, cos);

    printf("Received Packet on port: %d\n", port);
    dump_pkt(opkt);

    if (in_pkt && in_pkt_len) {
        if (port != in_port) {
            printf("Input port (%u) does not match the expected port (%u)\n", port, in_port);
            return SDK_RET_ERR;
        }
        epkt.resize(in_pkt_len);
        memcpy(epkt.data(), in_pkt, in_pkt_len);
        if (opkt != epkt) {
            printf("Output packet does not match the expected packet\n");
            return SDK_RET_ERR;
        }
    }
    return SDK_RET_OK;
}


uint8_t     g_h2s_port = TM_PORT_UPLINK_0;
uint8_t     g_s2h_port = TM_PORT_UPLINK_1;
uint32_t    g_session_index = 1;

/*
 * Host to Switch: Flow-miss
 */
uint8_t g_snd_pkt_h2s_flow_miss[] = {
    0x00, 0x00, 0xF1, 0xD0, 0xD1, 0xD0, 0x00, 0x00,
    0x00, 0x40, 0x08, 0x01, 0x81, 0x00, 0x00, 0x02,
    0x08, 0x00, 0x45, 0x00, 0x00, 0x50, 0x00, 0x01,
    0x00, 0x00, 0x40, 0x11, 0xB6, 0x9A, 0x02, 0x00,
    0x00, 0x01, 0xC0, 0x00, 0x02, 0x01, 0x03, 0xE8,
    0x27, 0x10, 0x00, 0x3C, 0x00, 0x00, 0x61, 0x62,
    0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A,
    0x6C, 0x6B, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72,
    0x73, 0x74, 0x75, 0x76, 0x77, 0x7A, 0x78, 0x79,
    0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E, 0x6F, 0x70,
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x7A,
    0x78, 0x79
};


/*
 * Normalized key
 */
uint16_t    g_h2s_vnic_id = 0x0001;
uint32_t    g_h2s_sip = 0x02000001;
uint32_t    g_h2s_dip = 0xc0000201;
uint8_t     g_h2s_proto = 0x11;
uint16_t    g_h2s_sport = 0x03e8;
uint16_t    g_h2s_dport = 0x2710;


/*
 * Host to Switch:
 */
uint8_t g_snd_pkt_h2s[] = {
    0x00, 0x00, 0xF1, 0xD0, 0xD1, 0xD0, 0x00, 0x00,
    0x00, 0x40, 0x08, 0x01, 0x81, 0x00, 0x00, 0x01,
    0x08, 0x00, 0x45, 0x00, 0x00, 0x50, 0x00, 0x01,
    0x00, 0x00, 0x40, 0x11, 0xB6, 0x9A, 0x02, 0x00,
    0x00, 0x01, 0xC0, 0x00, 0x02, 0x01, 0x03, 0xE8,
    0x27, 0x10, 0x00, 0x3C, 0x00, 0x00, 0x61, 0x62,
    0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A,
    0x6C, 0x6B, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72,
    0x73, 0x74, 0x75, 0x76, 0x77, 0x7A, 0x78, 0x79,
    0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E, 0x6F, 0x70,
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x7A,
    0x78, 0x79
};

/*
 * H2S Specific fields 
 */
uint32_t    g_h2s_vlan = 0x0001;

#if 0
sdk_ret_t
create_h2s_v4_session_info(uint32_t session_index)
{
    p4pd_error_t                p4pd_ret;
    uint32_t                    tableid = P4TBL_ID_SESSION_INFO;
    session_info_actiondata_t   actiondata;

    memset(&actiondata, 0, sizeof(session_info_actiondata_t));
    actiondata.action_id = SESSION_INFO_SESSION_INFO_ID;

    actiondata.action_u.session_info_session_info.valid_flag = 0x01;
    actiondata.action_u.session_info_session_info.pop_hdr_flag = 0x01;

    p4pd_ret = p4pd_global_entry_write(
                        tableid, session_index, NULL, NULL, &actiondata);
    if (p4pd_ret != P4PD_SUCCESS) {
        SDK_TRACE_ERR("Failed to create session info index %u",
                      session_index);
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
create_h2s_v4_session_info_rewrite(uint32_t session_index,
        mac_addr_t *substrate_dmac, mac_addr_t *substrate_smac,
        uint16_t substrate_vlan, uint32_t substrate_sip,
        uint32_t substrate_dip, uint8_t substrate_ip_ttl,
        uint16_t substrate_udp_sport, uint16_t substrate_udp_dport,
        uint32_t mpls1_label, uint32_t mpls2_label)
{
    p4pd_error_t                        p4pd_ret;
    uint32_t                            tableid = P4TBL_ID_SESSION_INFO_REWRITE;
    session_info_rewrite_actiondata_t   actiondata;

    memset(&actiondata, 0, sizeof(session_info_rewrite_actiondata_t));
    actiondata.action_id = SESSION_INFO_REWRITE_SESSION_INFO_REWRITE_ID;

    actiondata.action_u.session_info_rewrite_session_info_rewrite.valid_flag = 0x01;
    actiondata.action_u.session_info_rewrite_session_info_rewrite.user_pkt_rewrite_type = L3REWRITE_NONE;
    actiondata.action_u.session_info_rewrite_session_info_rewrite.encap_type = REWRITE_ENCAP_MPLSOUDP;
    sdk::lib::memrev(actiondata.action_u.session_info_rewrite_session_info_rewrite.dmac,
            (uint8_t*)substrate_dmac, sizeof(mac_addr_t));
    sdk::lib::memrev(actiondata.action_u.session_info_rewrite_session_info_rewrite.smac,
            (uint8_t*)substrate_smac, sizeof(mac_addr_t));
    actiondata.action_u.session_info_rewrite_session_info_rewrite.vlan = substrate_vlan;
    actiondata.action_u.session_info_rewrite_session_info_rewrite.ip_ttl = substrate_ip_ttl;
    actiondata.action_u.session_info_rewrite_session_info_rewrite.ip_saddr = substrate_sip;
    actiondata.action_u.session_info_rewrite_session_info_rewrite.ip_daddr = substrate_dip;
    actiondata.action_u.session_info_rewrite_session_info_rewrite.udp_sport = substrate_udp_sport;
    actiondata.action_u.session_info_rewrite_session_info_rewrite.udp_dport = substrate_udp_dport;
    actiondata.action_u.session_info_rewrite_session_info_rewrite.mpls1_label = mpls1_label;
    actiondata.action_u.session_info_rewrite_session_info_rewrite.mpls2_label = mpls2_label;

    p4pd_ret = p4pd_global_entry_write(
                        tableid, session_index, NULL, NULL, &actiondata);
    if (p4pd_ret != P4PD_SUCCESS) {
        SDK_TRACE_ERR("Failed to create session info rewrite index %u",
                      session_index);
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}
#else

#if 0
sdk_ret_t
create_v4_session_info_h2s(uint32_t session_index, 
#endif
sdk_ret_t
create_v4_session_info_all(uint32_t session_index, 
                    mac_addr_t *substrate_dmac, mac_addr_t *substrate_smac, uint16_t substrate_vlan,
                    uint32_t substrate_sip, uint32_t substrate_dip, 
                    uint16_t substrate_udp_sport, uint16_t substrate_udp_dport,
                    uint32_t mpls1_label, uint32_t mpls2_label)

{
    sdk_ret_t                       ret = SDK_RET_OK;
    pds_flow_session_spec_t         spec;

    memset(&spec, 0, sizeof(spec));
    spec.key.session_info_id = session_index;
    //spec.key.direction = (HOST_TO_SWITCH | SWITCH_TO_HOST);
    spec.key.direction = (HOST_TO_SWITCH);

    spec.data.host_to_switch_flow_info.rewrite_info.strip_l2_header = TRUE;
    spec.data.host_to_switch_flow_info.rewrite_info.strip_vlan_tag = TRUE;

    spec.data.host_to_switch_flow_info.rewrite_info.user_packet_rewrite_type = REWRITE_TYPE_NONE;

    spec.data.host_to_switch_flow_info.rewrite_info.encap_type = ENCAP_TYPE_MPLSOUDP;

    sdk::lib::memrev(spec.data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.dmac,
            (uint8_t*)substrate_dmac, sizeof(mac_addr_t));
    sdk::lib::memrev(spec.data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.smac,
            (uint8_t*)substrate_smac, sizeof(mac_addr_t));
    spec.data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.insert_vlan_tag = TRUE;
    spec.data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.vlan_id = substrate_vlan;

    spec.data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.ip_encap.ip_saddr = substrate_sip; 
    spec.data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.ip_encap.ip_daddr = substrate_dip; 

    spec.data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.udp_encap.udp_sport = substrate_udp_sport;
    spec.data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.udp_encap.udp_sport = substrate_udp_sport;

    spec.data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.mpls1_label = mpls1_label;
    spec.data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.mpls2_label = mpls2_label;

    ret = pds_flow_session_info_create(&spec);
    if (ret != SDK_RET_OK) {
        printf("Failed to program session info : %u\n", ret);
    }
    return ret;
}
#endif

sdk_ret_t
create_v4_flow (uint16_t vnic_id, ipv4_addr_t v4_addr_sip, ipv4_addr_t v4_addr_dip,
        uint8_t proto, uint16_t sport, uint16_t dport,
        pds_flow_spec_index_type_t index_type, uint32_t index)
{
    pds_flow_spec_t             spec;


    spec.key.vnic_id = vnic_id;
    spec.key.smac = 0;
    spec.key.dmac = 0;
    spec.key.ip_addr_family = IP_AF_IPV4;
    memset(spec.key.ip_saddr, 0, sizeof(spec.key.ip_saddr));
    memcpy(spec.key.ip_saddr, &v4_addr_sip, sizeof(ipv4_addr_t));
    memset(spec.key.ip_daddr, 0, sizeof(spec.key.ip_daddr));
    memcpy(spec.key.ip_daddr, &v4_addr_dip, sizeof(ipv4_addr_t));
    spec.key.ip_proto = proto;
    spec.key.l4.tcp_udp.sport = sport;
    spec.key.l4.tcp_udp.dport = dport;

    spec.data.index_type = index_type;
    spec.data.index = index;

    return pds_flow_cache_entry_create(&spec);
}

/*
 * Session into rewrite
 */
mac_addr_t  substrate_smac = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
mac_addr_t  substrate_dmac = {0x00, 0x06, 0x07, 0x08, 0x09, 0x0a};
uint16_t    substrate_vlan = 0x02;
uint32_t    substrate_sip = 0x04030201;
uint32_t    substrate_dip = 0x01020304;
uint8_t     substrate_ip_ttl = 64;
uint16_t    substrate_udp_sport = 0xabcd;
uint16_t    substrate_udp_dport = 0x1234;
uint32_t    mpls1_label = 0x12345;
uint32_t    mpls2_label = 0x6789a;

static sdk_ret_t
vlan_to_vnic_map(uint16_t vlan_id, uint16_t vnic_id)
{
    pds_vlan_to_vnic_map_spec_t     spec;
    sdk_ret_t                       ret = SDK_RET_OK;

    spec.key.vlan_id = vlan_id;
    spec.data.vnic_type = VNIC_TYPE_L3;
    spec.data.vnic_id = vnic_id;

    ret = pds_vlan_to_vnic_map_create(&spec);
    if (ret != SDK_RET_OK) {
        printf("Failed to setup VLAN: %hu to VNIC:%hu mapping\n",
                vlan_id, vnic_id);
    }
    printf("Setup VLAN: %hu to VNIC:%hu mapping\n",
            vlan_id, vnic_id);
    return ret;
}

/*
 * Switch to Host:
 * Layer 1
 */
uint8_t g_snd_pkt_s2h[] = {
    0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00, 0xAA,
    0xBB, 0xCC, 0xDD, 0xEE, 0x08, 0x00, 0x45, 0x00,
    0x00, 0x74, 0x00, 0x00, 0x00, 0x00, 0x40, 0x11,
    0xA2, 0xA0, 0x64, 0x65, 0x66, 0x67, 0x0C, 0x0C,
    0x01, 0x01, 0xE4, 0xE7, 0x19, 0xEB, 0x00, 0x60,
    0x00, 0x00, 0x12, 0x34, 0x50, 0x00, 0x67, 0x89,
    0xA1, 0x00, 0x45, 0x00, 0x00, 0x50, 0x00, 0x01,
    0x00, 0x00, 0x40, 0x11, 0xB6, 0x9A, 0xC0, 0x00,
    0x02, 0x01, 0x02, 0x00, 0x00, 0x01, 0x27, 0x10,
    0x03, 0xE8, 0x00, 0x3C, 0xF3, 0x44, 0x61, 0x62,
    0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A,
    0x6C, 0x6B, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72,
    0x73, 0x74, 0x75, 0x76, 0x77, 0x7A, 0x78, 0x79,
    0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E, 0x6F, 0x70,
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x7A,
    0x78, 0x79,
};

/*
 * S2H Specific fields 
 */
uint32_t    g_s2h_mpls1_label = 0x12345;
uint32_t    g_s2h_mpls2_label = 0x6789a;

#if 0
sdk_ret_t
create_s2h_v4_session_info(uint32_t session_index, uint32_t substrate_sip)
{
    p4pd_error_t                p4pd_ret;
    uint32_t                    tableid = P4TBL_ID_SESSION_INFO;
    session_info_actiondata_t   actiondata;

    memset(&actiondata, 0, sizeof(session_info_actiondata_t));
    actiondata.action_id = SESSION_INFO_SESSION_INFO_ID;

    actiondata.action_u.session_info_session_info.valid_flag = 0x01;
    actiondata.action_u.session_info_session_info.pop_hdr_flag = 0x01;
    actiondata.action_u.session_info_session_info.config_substrate_src_ip = substrate_sip;

    p4pd_ret = p4pd_global_entry_write(
                        tableid, session_index, NULL, NULL, &actiondata);
    if (p4pd_ret != P4PD_SUCCESS) {
        SDK_TRACE_ERR("Failed to create session info index %u",
                      session_index);
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
create_s2h_v4_session_info_rewrite(uint32_t session_index,
        mac_addr_t *ep_dmac, mac_addr_t *ep_smac, uint16_t vnic_vlan)
{
    p4pd_error_t                        p4pd_ret;
    uint32_t                            tableid = P4TBL_ID_SESSION_INFO_REWRITE;
    session_info_rewrite_actiondata_t   actiondata;

    memset(&actiondata, 0, sizeof(session_info_rewrite_actiondata_t));
    actiondata.action_id = SESSION_INFO_REWRITE_SESSION_INFO_REWRITE_ID;

    actiondata.action_u.session_info_rewrite_session_info_rewrite.valid_flag = 0x01;
    actiondata.action_u.session_info_rewrite_session_info_rewrite.user_pkt_rewrite_type = L3REWRITE_NONE;
    actiondata.action_u.session_info_rewrite_session_info_rewrite.encap_type = REWRITE_ENCAP_L2;
    sdk::lib::memrev(actiondata.action_u.session_info_rewrite_session_info_rewrite.dmac,
            (uint8_t*)ep_dmac, sizeof(mac_addr_t));
    sdk::lib::memrev(actiondata.action_u.session_info_rewrite_session_info_rewrite.smac,
            (uint8_t*)ep_smac, sizeof(mac_addr_t));
    actiondata.action_u.session_info_rewrite_session_info_rewrite.vlan = vnic_vlan;

    p4pd_ret = p4pd_global_entry_write(
                        tableid, session_index, NULL, NULL, &actiondata);
    if (p4pd_ret != P4PD_SUCCESS) {
        SDK_TRACE_ERR("Failed to create session info rewrite index %u",
                      session_index);
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

#endif

/*
 * Session info rewrite
 */
mac_addr_t  ep_smac = {0x00, 0x00, 0xF1, 0xD0, 0xD1, 0xD0};
mac_addr_t  ep_dmac = {0x00, 0x00, 0x00, 0x40, 0x08, 0x01};
uint16_t    vnic_vlan = 0x01;

static sdk_ret_t
mpls_label_to_vnic_map(uint32_t mpls_label, uint16_t vnic_id)
{
    pds_mpls_label_to_vnic_map_spec_t       spec;
    sdk_ret_t                               ret = SDK_RET_OK;

    spec.key.mpls_label = mpls_label;
    spec.data.vnic_type = VNIC_TYPE_L3;
    spec.data.vnic_id = vnic_id;

    ret = pds_mpls_label_to_vnic_map_create(&spec);
    if (ret != SDK_RET_OK) {
        printf("Failed to setup MPLS Label: %u to VNIC:%hu mapping\n",
                mpls_label, vnic_id);
    }
    printf("Setup MPLS Label: %u to VNIC:%hu mapping\n",
            mpls_label, vnic_id);
    return ret;
}


sdk_ret_t
setup_flow(void)
{
    sdk_ret_t       ret = SDK_RET_OK;

    // Setup VNIC Mappings
    ret = vlan_to_vnic_map(g_h2s_vlan, g_h2s_vnic_id);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    // Setup VNIC Mappings
    ret = mpls_label_to_vnic_map(g_s2h_mpls2_label, g_h2s_vnic_id);
    if (ret != SDK_RET_OK) {
        return ret;
    }


    ret = create_v4_session_info_all(g_session_index, 
                    &substrate_dmac, &substrate_smac, substrate_vlan,
                    substrate_sip, substrate_dip, 
                    substrate_udp_sport, substrate_udp_dport,
                    mpls1_label, mpls2_label);

    // Setup Normalized Flow entry
    ret = create_v4_flow(g_h2s_vnic_id, g_h2s_sip, g_h2s_dip,
            g_h2s_proto, g_h2s_sport, g_h2s_dport,
            PDS_FLOW_SPEC_INDEX_CONNTRACK, g_session_index);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    return ret;
}



void inline
print_usage (char **argv)
{
    fprintf(stdout, "Usage : %s -c|--config <cfg.json>\n", argv[0]);
}


int
main (int argc, char **argv)
{
    int          oc;
    string       cfg_path, cfg_file, profile, pipeline, file;
    boost::property_tree::ptree pt;

    struct option longopts[] = {
       { "config",    required_argument, NULL, 'c' },
       { "profile",   required_argument, NULL, 'p' },
       { "feature",   required_argument, NULL, 'f' },
       { "help",      no_argument,       NULL, 'h' },
       { 0,           0,                 0,     0 }
    };

    // parse CLI options
    while ((oc = getopt_long(argc, argv, ":hc:p:f:W;", longopts, NULL)) != -1) {
        switch (oc) {
        case 'c':
            if (optarg) {
                cfg_file = std::string(optarg);
            } else {
                fprintf(stderr, "config file is not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 'p':
            if (optarg) {
                profile = std::string(optarg);
            } else {
                fprintf(stderr, "profile is not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;


        case 'h':
            print_usage(argv);
            exit(0);
            break;

        case ':':
            fprintf(stderr, "%s: option -%c requires an argument\n",
                    argv[0], optopt);
            print_usage(argv);
            exit(1);
            break;

        case '?':
        default:
            fprintf(stderr, "%s: option -%c is invalid, quitting ...\n",
                    argv[0], optopt);
            print_usage(argv);
            exit(1);
            break;
        }
    }

    // form the full path to the config directory
    cfg_path = std::string(std::getenv("CONFIG_PATH"));
    if (cfg_path.empty()) {
        cfg_path = std::string("./");
    } else {
        cfg_path += "/";
    }

    // read pipeline.json file to figure out pipeline
    file = cfg_path + "/pipeline.json";
    if (access(file.c_str(), R_OK) < 0) {
        fprintf(stderr, "pipeline.json doesn't exist or not accessible\n");
        exit(1);
    }

    // parse pipeline.json to determine pipeline
    try {
        std::ifstream json_cfg(file.c_str());
        read_json(json_cfg, pt);
        pipeline = pt.get<std::string>("pipeline");
    } catch (...) {
        fprintf(stderr, "pipeline.json doesn't have pipeline field\n");
        exit(1);
    }
    if  (pipeline.compare("athena") != 0) {
        fprintf(stderr, "Unknown pipeline %s\n", pipeline.c_str());
        exit(1);
    }

   // make sure the cfg file exists
    file = cfg_path + "/" + pipeline + "/" + std::string(cfg_file);
    if (access(file.c_str(), R_OK) < 0) {
        fprintf(stderr, "Config file %s doesn't exist or not accessible\n",
                file.c_str());
        exit(1);
    }

    // Initialize the PDS functionality
    pds_init_params_t init_params;

    memset(&init_params, 0, sizeof(init_params));
    init_params.init_mode = PDS_INIT_MODE_COLD_START;
    init_params.trace_cb  = core::sdk_logger;
    init_params.pipeline  = pipeline;
    init_params.cfg_file  = cfg_file;
    init_params.scale_profile = PDS_SCALE_PROFILE_DEFAULT;
    if (!profile.empty()) {
        if (profile.compare("p1") == 0) {
            init_params.scale_profile = PDS_SCALE_PROFILE_P1;
        } else if (profile.compare("p2") == 0) {
            init_params.scale_profile = PDS_SCALE_PROFILE_P2;
        }
    }
    // initialize the logger instance
    core::logger_init();

    pds_init(&init_params);

    flow_table_init();

    setup_flow();

    // wait forver
    printf("Initialization done ...\n");
    //send_packet("h2s pkt:flow-miss", g_snd_pkt_h2s_flow_miss, sizeof(g_snd_pkt_h2s_flow_miss), g_h2s_port, NULL, 0, 0);
    send_packet("h2s pkt", g_snd_pkt_h2s, sizeof(g_snd_pkt_h2s), g_h2s_port, NULL, 0, 0);

    send_packet("s2h pkt", g_snd_pkt_s2h, sizeof(g_snd_pkt_s2h), g_s2h_port, NULL, 0, 0);


    while (1);

    return 0;
}

