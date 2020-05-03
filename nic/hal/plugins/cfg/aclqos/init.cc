// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "gen/hal/svc/acl_svc_gen.hpp"
#include "gen/hal/svc/qos_svc_gen.hpp"
#include "gen/hal/svc/session_svc_gen.hpp"
#include "nic/hal/plugins/cfg/aclqos/qos.hpp"
#include "nic/hal/plugins/cfg/aclqos/acl.hpp"
#include "nic/include/pkt_hdrs.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace hal {
namespace aclqos {

QOSServiceImpl           g_qos_svc;
AclServiceImpl           g_acl_svc;

void
svc_reg (ServerBuilder *server_builder, hal::hal_feature_set_t feature_set)
{
    if (!server_builder) {
        return;
    }

    // register all "network" services
    HAL_TRACE_DEBUG("Registering gRPC network services ...");
    if (feature_set == hal::HAL_FEATURE_SET_IRIS) {
        server_builder->RegisterService(&g_acl_svc);
        server_builder->RegisterService(&g_qos_svc);
    } else if (feature_set == hal::HAL_FEATURE_SET_GFT) {
        // Revisit. DOL was not able to create Lif without qos class
        server_builder->RegisterService(&g_qos_svc);
    }
    HAL_TRACE_DEBUG("gRPC network services registered ...");
    return;
}

//------------------------------------------------------------------------------
// default QoS configuration
//------------------------------------------------------------------------------
static hal_ret_t
hal_qos_config_init (hal_cfg_t *hal_cfg)
{
    hal_ret_t          ret     = HAL_RET_OK;
    sdk_ret_t          sdk_ret = SDK_RET_OK;
    QosClassResponse   qos_class_rsp;
    CoppRequestMsg     copp_request;
    CoppResponse       copp_rsp;
    int                max_default_qos_class = 8;
    QosClassSpec       spec;
    qos::QosClassSetGlobalPauseTypeRequest global_pause_type_req;
    qos::QosClassSetGlobalPauseTypeResponseMsg global_pause_type_rsp;

    kh::QosGroup qos_group[max_default_qos_class] = {
                            kh::QosGroup::DEFAULT,
                            kh::QosGroup::CONTROL,
                            kh::QosGroup::SPAN,
                            kh::QosGroup::INTERNAL_RX_PROXY_NO_DROP,
                            kh::QosGroup::INTERNAL_RX_PROXY_DROP,
                            kh::QosGroup::INTERNAL_TX_PROXY_NO_DROP,
                            kh::QosGroup::INTERNAL_TX_PROXY_DROP,
                            kh::QosGroup::INTERNAL_CPU_COPY };

    hal::hal_cfg_db_open(CFG_OP_WRITE);

    // Qos class
    std::string copp_configs;

    sdk_ret = g_hal_state->catalog()->get_child_str("qos.configs.copp",
                                               copp_configs);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error getting copp configs from catalog: ret {}", ret);
        goto end;
    }

    // init TC to IQ mapping
    //qos_class_init_tc_to_iq_map();

    // set the global pause mode to link-level
    global_pause_type_req.set_pause_type(qos::QOS_PAUSE_TYPE_LINK_LEVEL);
    qos_class_set_global_pause_type(global_pause_type_req,
                                    &global_pause_type_rsp);

    for (int i = 0; i < max_default_qos_class; i++) {
        spec.mutable_key_or_handle()->set_qos_group(qos_group[i]);
        spec.mutable_pause()->set_type(qos::QOS_PAUSE_TYPE_LINK_LEVEL);

        if (qos_group[i] == kh::QosGroup::INTERNAL_CPU_COPY) {
            spec.mutable_sched()->mutable_strict()->set_bps(QOS_DEFAULT_CPU_BPS);
        } else {
            spec.mutable_sched()->mutable_dwrr()->set_bw_percentage(50);
        }

        ret = qosclass_create(spec, &qos_class_rsp);
        if ((ret != HAL_RET_OK) && (ret != HAL_RET_ENTRY_EXISTS)) {
            HAL_TRACE_ERR("Error  creating qos class ret: {}",
                          ret);
            goto end;
        }
    }

    google::protobuf::util::JsonStringToMessage(copp_configs, &copp_request);
    for (int i = 0; i < copp_request.request_size(); i++) {
        auto spec = copp_request.request(i);
        ret = copp_create(spec, &copp_rsp);
        if ((ret != HAL_RET_OK) && (ret != HAL_RET_ENTRY_EXISTS)) {
            HAL_TRACE_ERR("Error  creating copp ret: {}",
                          ret);
            goto end;
        }
    }
    ret = HAL_RET_OK;

end:

    hal::hal_cfg_db_close();
    return ret;
}

hal_ret_t
hal_acl_create (uint32_t acl_id, uint32_t priority,
                bool ip_frag, acl::AclAction acl_action)
{
    hal_ret_t     ret;
    AclSpec       spec;
    AclResponse   rsp;
    AclSelector   *match;
    AclActionInfo *action;
    IPSelector    *ip_selector;

    spec.Clear();

    match = spec.mutable_match();
    action = spec.mutable_action();

    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Drop IP fragmented packets
    match->mutable_internal_key()->set_ip_frag(ip_frag);
    match->mutable_internal_mask()->set_ip_frag(ip_frag);
    action->set_action(acl_action);

    switch (acl_id) {
    case ACL_ICMP_FRAGMENT_DROP_ENTRY_ID:
        ip_selector = match->mutable_ip_selector();
        ip_selector->set_ip_af(types::IPAddressFamily::IP_AF_INET);
        ip_selector->set_ip_protocol(types::IPProtocol::IPPROTO_ICMP);
        action->mutable_internal_actions()->mutable_drop_reason()->set_drop_icmp_frag_pkt(true);
        break;
    case ACL_ICMPV6_FRAG_DROP_ENTRY_ID:
        ip_selector = match->mutable_ip_selector();
        ip_selector->set_ip_af(types::IPAddressFamily::IP_AF_INET6);
        ip_selector->set_ip_protocol(types::IPProtocol::IPPROTO_ICMPV6);
        action->mutable_internal_actions()->mutable_drop_reason()->set_drop_icmp_frag_pkt(true);
        break;
    default:
        break;
    }

    ret = hal::acl_create(spec, &rsp);

    return ret;
}

hal_ret_t
hal_acl_delete (uint32_t acl_id)
{
    hal_ret_t           ret = HAL_RET_OK;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;

    req.mutable_key_or_handle()->set_acl_id(acl_id);
    ret = hal::acl_delete(req, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete acl: {}", acl_id);
    }

    return ret;
}


//------------------------------------------------------------------------------
// install smart-nic mode specific ACLs
//------------------------------------------------------------------------------
hal_ret_t
hal_smart_nic_acl_config_init (void)
{
    hal_ret_t ret;

    ret = hal_acl_create(ACL_ICMP_FRAGMENT_DROP_ENTRY_ID,
            ACL_ICMP_FRAGMENT_DROP_ENTRY_PRIORITY,
            true, acl::AclAction::ACL_ACTION_DENY);

    if ((ret != HAL_RET_OK) && (ret != HAL_RET_ENTRY_EXISTS)) {
        HAL_TRACE_ERR("ICMP Fragment drop acl entry create failed ret {}", ret);
        goto end;
    }

    HAL_TRACE_DEBUG("ICMP Fragment drop acl entry created");

    ret = hal_acl_create(ACL_ICMPV6_FRAG_DROP_ENTRY_ID,
            ACL_ICMPV6_FRAGMENT_DROP_ENTRY_PRIORITY,
            true, acl::AclAction::ACL_ACTION_DENY);

    if ((ret != HAL_RET_OK) && (ret != HAL_RET_ENTRY_EXISTS)) {
        HAL_TRACE_ERR("ICMPv6 Fragment drop acl entry create failed ret {}", ret);
        goto end;
    }
    HAL_TRACE_DEBUG("ICMPv6 Fragment drop acl entry created");

    ret = hal_acl_create(ACL_IP_FRAGMENT_DROP_ENTRY_ID,
            ACL_IP_FRAGMENT_DROP_ENTRY_PRIORITY,
            true, acl::AclAction::ACL_ACTION_DENY);

    if ((ret != HAL_RET_OK) && (ret != HAL_RET_ENTRY_EXISTS)) {
        HAL_TRACE_ERR("IP Fragment drop acl entry create failed ret {}", ret);
        goto end;
    }
    HAL_TRACE_DEBUG("IP Fragment drop acl entry created");

    return HAL_RET_OK;

end:

    return ret;
}

hal_ret_t
hal_smart_nic_acl_config_deinit (void)
{
    hal_ret_t ret = HAL_RET_OK;

    ret = hal_acl_delete(ACL_ICMP_FRAGMENT_DROP_ENTRY_ID);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to uninstall acl: {}. ret: {}",
                      ACL_ICMP_FRAGMENT_DROP_ENTRY_ID, ret);
        goto end;
    }

    ret = hal_acl_delete(ACL_ICMPV6_FRAG_DROP_ENTRY_ID);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to uninstall acl: {}. ret: {}",
                      ACL_ICMPV6_FRAG_DROP_ENTRY_ID, ret);
        goto end;
    }

    ret = hal_acl_delete(ACL_IP_FRAGMENT_DROP_ENTRY_ID);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to uninstall acl: {}. ret: {}",
                      ACL_IP_FRAGMENT_DROP_ENTRY_ID, ret);
        goto end;
    }

    HAL_TRACE_DEBUG("Uninstalled fragment ACLs.");

end:
    return ret;
}

//------------------------------------------------------------------------------
// install endpoint learning related ACLs
//------------------------------------------------------------------------------
hal_ret_t
hal_eplearn_acl_config_init (void)
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    AclActionInfo    arp_action;
    AclActionInfo    dhcp_action;
    uint32_t         acl_id;
    uint32_t         priority;

    arp_action.Clear();
    arp_action.set_action(acl::AclAction::ACL_ACTION_REDIRECT);
    arp_action.mutable_redirect_if_key_handle()->set_interface_id(IF_ID_CPU);
    arp_action.mutable_internal_actions()->set_qid(types::CPUCB_ID_FLOWMISS);
    arp_action.mutable_internal_actions()->set_qid_valid(true);
    arp_action.mutable_copp_key_handle()->set_copp_type(kh::COPP_TYPE_ARP);

    dhcp_action.Clear();
    dhcp_action.set_action(acl::AclAction::ACL_ACTION_REDIRECT);
    dhcp_action.mutable_redirect_if_key_handle()->set_interface_id(IF_ID_CPU);
    dhcp_action.mutable_internal_actions()->set_qid(types::CPUCB_ID_FLOWMISS);
    dhcp_action.mutable_internal_actions()->set_qid_valid(true);
    dhcp_action.mutable_copp_key_handle()->set_copp_type(kh::COPP_TYPE_DHCP);

    acl_id = ACL_EPLEARN_ENTRY_ID_BEGIN;
    priority = ACL_EPLEARN_ENTRY_PRIORITY_BEGIN;
    {
        spec.Clear();
        match = spec.mutable_match();
        action = spec.mutable_action();
        *action = arp_action;
        spec.mutable_key_or_handle()->set_acl_id(acl_id++);
        spec.set_priority(priority++);

        match->mutable_eth_selector()->set_eth_type(ETH_TYPE_ARP);
        match->mutable_eth_selector()->set_eth_type_mask(0xffff);
        match->mutable_internal_key()->set_ep_learn_en(true);
        match->mutable_internal_mask()->set_ep_learn_en(true);
        match->mutable_internal_key()->set_flow_miss(true);
        match->mutable_internal_mask()->set_flow_miss(true);
        match->mutable_internal_key()->set_no_drop(true);
        match->mutable_internal_mask()->set_no_drop(true);
        match->mutable_internal_key()->set_direction(0);
        match->mutable_internal_mask()->set_direction(1);

        ret = hal::acl_create(spec, &rsp);
        if ((ret != HAL_RET_OK) && (ret != HAL_RET_ENTRY_EXISTS)) {
            return ret;
        }
    }

    {
        spec.Clear();
        match = spec.mutable_match();
        action = spec.mutable_action();
        *action = arp_action;
        spec.mutable_key_or_handle()->set_acl_id(acl_id++);
        spec.set_priority(ACL_RARP_ENTRY_PRIORITY);

        match->mutable_eth_selector()->set_eth_type(ETH_TYPE_RARP);
        match->mutable_eth_selector()->set_eth_type_mask(0xffff);
        match->mutable_internal_key()->set_ep_learn_en(true);
        match->mutable_internal_mask()->set_ep_learn_en(true);
        match->mutable_internal_key()->set_flow_miss(true);
        match->mutable_internal_mask()->set_flow_miss(true);
        match->mutable_internal_key()->set_no_drop(true);
        match->mutable_internal_mask()->set_no_drop(true);
        match->mutable_internal_key()->set_direction(0);
        match->mutable_internal_mask()->set_direction(1);

        ret = hal::acl_create(spec, &rsp);
        if ((ret != HAL_RET_OK) && (ret != HAL_RET_ENTRY_EXISTS)) {
            return ret;
        }
    }

    {
        spec.Clear();
        match = spec.mutable_match();
        action = spec.mutable_action();
        *action = dhcp_action;
        spec.mutable_key_or_handle()->set_acl_id(acl_id++);
        spec.set_priority(priority++);

        acl::IPSelector *ip_selector = match->mutable_ip_selector();
        ip_selector->set_ip_af(types::IPAddressFamily::IP_AF_INET6);
        ip_selector->mutable_icmp_selector()->set_icmp_code(0);
        ip_selector->mutable_icmp_selector()->set_icmp_code_mask(0xff);
        ip_selector->mutable_icmp_selector()->set_icmp_type(ICMP_NEIGHBOR_SOLICITATION);
        ip_selector->mutable_icmp_selector()->set_icmp_type_mask(0xff);
        match->mutable_internal_key()->set_ep_learn_en(true);
        match->mutable_internal_mask()->set_ep_learn_en(true);
        match->mutable_internal_key()->set_no_drop(true);
        match->mutable_internal_key()->set_flow_miss(true);
        match->mutable_internal_mask()->set_flow_miss(true);
        match->mutable_internal_key()->set_direction(0);
        match->mutable_internal_mask()->set_direction(1);

        ret = hal::acl_create(spec, &rsp);
        if ((ret != HAL_RET_OK) && (ret != HAL_RET_ENTRY_EXISTS)) {
            return ret;
        }
    }

    {
        spec.Clear();
        match = spec.mutable_match();
        action = spec.mutable_action();
        *action = dhcp_action;
        spec.mutable_key_or_handle()->set_acl_id(acl_id++);
        spec.set_priority(priority++);

        acl::IPSelector *ip_selector = match->mutable_ip_selector();
        ip_selector->set_ip_af(types::IPAddressFamily::IP_AF_INET6);
        ip_selector->mutable_icmp_selector()->set_icmp_code(0);
        ip_selector->mutable_icmp_selector()->set_icmp_code_mask(0xff);
        ip_selector->mutable_icmp_selector()->set_icmp_type(ICMP_NEIGHBOR_ADVERTISEMENT);
        ip_selector->mutable_icmp_selector()->set_icmp_type_mask(0xff);
        match->mutable_internal_key()->set_ep_learn_en(true);
        match->mutable_internal_mask()->set_ep_learn_en(true);
        match->mutable_internal_key()->set_flow_miss(true);
        match->mutable_internal_mask()->set_flow_miss(true);
        match->mutable_internal_key()->set_direction(0);
        match->mutable_internal_mask()->set_direction(1);

        ret = hal::acl_create(spec, &rsp);
        if ((ret != HAL_RET_OK) && (ret != HAL_RET_ENTRY_EXISTS)) {
            return ret;
        }
    }

    {
        spec.Clear();
        match = spec.mutable_match();
        action = spec.mutable_action();
        *action = dhcp_action;
        spec.mutable_key_or_handle()->set_acl_id(acl_id++);
        spec.set_priority(priority++);

        acl::IPSelector *ip_selector = match->mutable_ip_selector();
        ip_selector->set_ip_af(types::IPAddressFamily::IP_AF_INET);
        ip_selector->mutable_udp_selector()->mutable_dst_port_range()->set_port_low(DHCP_CLIENT_PORT);
        ip_selector->mutable_udp_selector()->mutable_dst_port_range()->set_port_high(DHCP_CLIENT_PORT);

        match->mutable_internal_key()->set_direction(0);
        match->mutable_internal_mask()->set_direction(1);
        match->mutable_internal_key()->set_ep_learn_en(true);
        match->mutable_internal_mask()->set_ep_learn_en(true);

        ret = hal::acl_create(spec, &rsp);
        if ((ret != HAL_RET_OK) && (ret != HAL_RET_ENTRY_EXISTS)) {
            return ret;
        }
    }

    {
        spec.Clear();
        match = spec.mutable_match();
        action = spec.mutable_action();
        *action = dhcp_action;
        spec.mutable_key_or_handle()->set_acl_id(acl_id++);
        spec.set_priority(priority++);

        acl::IPSelector *ip_selector = match->mutable_ip_selector();
        ip_selector->set_ip_af(types::IPAddressFamily::IP_AF_INET);
        ip_selector->mutable_udp_selector()->mutable_dst_port_range()->set_port_low(DHCP_SERVER_PORT);
        ip_selector->mutable_udp_selector()->mutable_dst_port_range()->set_port_high(DHCP_SERVER_PORT);

        match->mutable_internal_key()->set_direction(0);
        match->mutable_internal_mask()->set_direction(1);
        match->mutable_internal_key()->set_ep_learn_en(true);
        match->mutable_internal_mask()->set_ep_learn_en(true);

        ret = hal::acl_create(spec, &rsp);
        if ((ret != HAL_RET_OK) && (ret != HAL_RET_ENTRY_EXISTS)) {
            return ret;
        }
    }

    return HAL_RET_OK;
}

hal_ret_t
hal_eplearn_acl_config_deinit (void)
{
    hal_ret_t ret = HAL_RET_OK;

    for (int i = ACL_EPLEARN_ENTRY_ID_BEGIN; i <= ACL_EPLEARN_ENTRY_ID_END; 
         i++){
        ret = hal_acl_delete(i);
    }

    return ret;
}


//------------------------------------------------------------------------------
// install init time ACL entries. Deprecated
//------------------------------------------------------------------------------
hal_ret_t
hal_acl_config_init (void)
{
    hal_ret_t     ret = HAL_RET_OK;
    AclSpec       spec;
    AclResponse   rsp;

    hal::hal_cfg_db_open(CFG_OP_WRITE);

    ret = hal_smart_nic_acl_config_init();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error creating smart nic acl entries ret {}", ret);
        goto end;
    }

    ret = hal_eplearn_acl_config_init();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Eplearn acl entry create failed ret {}", ret);
        goto end;
    }

end:

    hal::hal_cfg_db_close();
    return ret;
}

hal_ret_t
hal_acl_config_deinit (void)
{
    hal_ret_t     ret = HAL_RET_OK;
    AclSpec       spec;
    AclResponse   rsp;

    hal::hal_cfg_db_open(CFG_OP_WRITE);

    ret = hal_smart_nic_acl_config_deinit();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error removing smart nic acl entries ret {}", ret);
        goto end;
    }

    ret = hal_eplearn_acl_config_deinit();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Eplearn acl entry remove failed ret {}", ret);
        goto end;
    }

end:

    hal::hal_cfg_db_close();
    return ret;
}

// initialization routine for network module
extern "C" hal_ret_t
aclqos_init (hal_cfg_t *hal_cfg)
{
    hal_ret_t            ret;

    svc_reg((ServerBuilder *)hal_cfg->server_builder, hal_cfg->features);

    // do qos config initialization
    ret = hal_qos_config_init(hal_cfg);
    HAL_ABORT(ret == HAL_RET_OK);

    // TODO acls need cpu interface. Right now, in GFT mode, the
    // hal_cpu_if_create() fails, so cpu interface is not created
    // So skipping installation of acls
    if (getenv("MICROSEG_ENABLE") && 
        (hal_cfg->features != HAL_FEATURE_SET_GFT)) {
        // do acl creates after qos creates. acls depend on qos config
        ret = hal_acl_config_init();
        HAL_ABORT(ret == HAL_RET_OK);
    } 

    return HAL_RET_OK;
}

// cleanup routine for aclqos module
extern "C" void
aclqos_exit (void)
{
}

}    // namespace aclqos
}    // namespace hal
