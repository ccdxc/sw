// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/hal_cfg.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/nw/interface.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/src/aclqos/qos.hpp"
#include "nic/hal/src/aclqos/acl.hpp"
#include "nic/include/pkt_hdrs.hpp"
#include <google/protobuf/util/json_util.h>

namespace hal {

//------------------------------------------------------------------------------
// API open HAL cfg db in read/write mode
//------------------------------------------------------------------------------
hal_ret_t
hal_cfg_db_open (cfg_op_t op)
{
    HAL_TRACE_DEBUG("Opening cfg db with mode {}", op);
    return g_hal_state->cfg_db()->db_open(op);
}

//------------------------------------------------------------------------------
// API to close the HAL cfg db after performing commit/abort operation
//------------------------------------------------------------------------------
hal_ret_t
hal_cfg_db_close (void)
{
    HAL_TRACE_DEBUG("Closing cfg db, current mode {}", t_cfg_db_ctxt.cfg_op_);
    return g_hal_state->cfg_db()->db_close();
}

//------------------------------------------------------------------------------
// create CPU interface, this will be used by FTEs to receive packets from
// dataplane and to inject packets into the dataplane
//------------------------------------------------------------------------------
static hal_ret_t inline
hal_cpu_if_create (uint32_t lif_id)
{
    InterfaceSpec      spec;
    InterfaceResponse  response;
    hal_ret_t          ret = HAL_RET_OK;

    spec.mutable_key_or_handle()->set_interface_id(IF_ID_CPU);
    spec.set_type(::intf::IfType::IF_TYPE_CPU);
    spec.set_admin_status(::intf::IfStatus::IF_STATUS_UP);
    spec.mutable_if_cpu_info()->mutable_lif_key_or_handle()->set_lif_id(lif_id);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = interface_create(spec, &response);
    if ((ret == HAL_RET_OK) || (ret == HAL_RET_ENTRY_EXISTS)) {
        HAL_TRACE_ERR("{}: CPU if create success, handle = {}",
                      __FUNCTION__, response.status().if_handle());
    } else {
        HAL_TRACE_DEBUG("{}: CPU if create failed", __FUNCTION__);
    }
    hal::hal_cfg_db_close();

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// default QoS configuration
//------------------------------------------------------------------------------
static hal_ret_t
hal_qos_config_init (hal_cfg_t *hal_cfg)
{
    hal_ret_t          ret = HAL_RET_OK;
    sdk_ret_t          sdk_ret;
    QosClassRequestMsg qos_class_request;
    QosClassResponse   qos_class_rsp;
    CoppRequestMsg     copp_request;
    CoppResponse       copp_rsp;

    hal::hal_cfg_db_open(CFG_OP_WRITE);

    // Qos class
    std::string qos_class_configs;
    std::string copp_configs;

    sdk_ret = sdk::lib::catalog::get_child_str(hal_cfg->catalog_file, 
                                               "qos.configs.qos_class", 
                                               qos_class_configs);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error getting qos_class configs from catalog: ret {}",
                      ret);
        goto end;
    }

    sdk_ret = sdk::lib::catalog::get_child_str(hal_cfg->catalog_file, 
                                               "qos.configs.copp", 
                                               copp_configs);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error getting copp configs from catalog: ret {}", ret);
        goto end;
    }

    google::protobuf::util::JsonStringToMessage(qos_class_configs,
                                                &qos_class_request);
    for (int i = 0; i < qos_class_request.request_size(); i++) {
        auto spec = qos_class_request.request(i);
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

//------------------------------------------------------------------------------
// install smart-nic mode specific ACLs
//------------------------------------------------------------------------------
static hal_ret_t
hal_smart_nic_acl_config_init (void)
{
    hal_ret_t     ret;
    AclSpec       spec;
    AclResponse   rsp;
    AclSelector   *match; 
    AclActionInfo *action;

    if (g_hal_state->forwarding_mode() != HAL_FORWARDING_MODE_SMART_SWITCH) {
        HAL_TRACE_DEBUG("Skipping smart nic acls");
        return HAL_RET_OK;
    } 

    // Drop IP fragmented packets
    spec.Clear();
    match = spec.mutable_match();
    action = spec.mutable_action();

    spec.mutable_key_or_handle()->set_acl_id(ACL_IP_FRAGMENT_DROP_ENTRY_ID);
    spec.set_priority(ACL_IP_FRAGMENT_DROP_ENTRY_PRIORITY);

    match->mutable_internal_key()->set_ip_frag(true);
    match->mutable_internal_mask()->set_ip_frag(true);

    action->set_action(acl::AclAction::ACL_ACTION_DENY);

    ret = hal::acl_create(spec, &rsp);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_ENTRY_EXISTS)) {
        HAL_TRACE_ERR("IP Fragment drop acl entry create failed ret {}", ret);
        goto end;
    }
    HAL_TRACE_DEBUG("IP fragment drop acl entry created");
    return HAL_RET_OK;

end:

    return ret;
}

//------------------------------------------------------------------------------
// install endpoint learning related ACLs
//------------------------------------------------------------------------------
static hal_ret_t
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
        match->mutable_internal_key()->set_flow_miss(true);
        match->mutable_internal_mask()->set_flow_miss(true);

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
        match->mutable_internal_key()->set_flow_miss(true);
        match->mutable_internal_mask()->set_flow_miss(true);

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
        match->mutable_internal_key()->set_flow_miss(true);
        match->mutable_internal_mask()->set_flow_miss(true);

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

        ret = hal::acl_create(spec, &rsp);
        if ((ret != HAL_RET_OK) && (ret != HAL_RET_ENTRY_EXISTS)) {
            return ret;
        }
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// install init time ACL entries
//------------------------------------------------------------------------------
static hal_ret_t
hal_acl_config_init (void)
{
    hal_ret_t ret;
    AclSpec       spec;
    AclResponse   rsp;
    AclSelector   *match; 
    AclActionInfo *action;

    hal::hal_cfg_db_open(CFG_OP_WRITE);

    if (g_hal_state->forwarding_mode() == HAL_FORWARDING_MODE_SMART_SWITCH) {
        ret = hal_smart_nic_acl_config_init();
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error creating smart nic acl entries ret {}", ret);
            goto end;
        }
    }

    {
        spec.Clear();
        match = spec.mutable_match();
        action = spec.mutable_action();

        spec.mutable_key_or_handle()->set_acl_id(ACL_QUIESCE_ENTRY_ID);
        spec.set_priority(ACL_QUIESCE_ENTRY_PRIORITY);

        match->mutable_eth_selector()->set_src_mac(0x00eeff000004);
        match->mutable_eth_selector()->set_src_mac_mask(0xffffffffffff);
        match->mutable_eth_selector()->set_dst_mac(0x00eeff000005);
        match->mutable_eth_selector()->set_dst_mac_mask(0xffffffffffff);
        match->mutable_eth_selector()->set_eth_type(0xaaaa);
        match->mutable_eth_selector()->set_eth_type_mask(0xffff);
        match->mutable_internal_key()->set_from_cpu(true);
        match->mutable_internal_mask()->set_from_cpu(true);

        action->set_action(acl::AclAction::ACL_ACTION_REDIRECT);
        action->mutable_redirect_if_key_handle()->set_interface_id(IF_ID_CPU);
        action->mutable_internal_actions()->set_qid(types::CPUCB_ID_QUIESCE);
        action->mutable_internal_actions()->set_qid_valid(true);
        action->mutable_copp_key_handle()->set_copp_type(kh::COPP_TYPE_FLOW_MISS);

        ret = hal::acl_create(spec, &rsp);
        if ((ret != HAL_RET_OK) && (ret != HAL_RET_ENTRY_EXISTS)) {
            HAL_TRACE_ERR("Quiesce acl entry create failed ret {}", ret);
            goto end;
        }
        HAL_TRACE_DEBUG("Quiesce acl entry created");
    }

    ret = hal_eplearn_acl_config_init();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Eplearn acl entry create failed ret {}", ret);
        goto end;
    }
    HAL_TRACE_DEBUG("Eplearn acl entry created");
    ret = HAL_RET_OK;

end:

    hal::hal_cfg_db_close();
    return ret;
}

hal_ret_t
hal_default_vrf_create()
{
    VrfSpec                 ten_spec;
    VrfResponse             ten_rsp;
    hal_ret_t               ret;

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();

    return ret;
}

hal_ret_t
hal_default_l2segment_create()
{
    L2SegmentSpec           l2seg_spec;
    L2SegmentResponse       l2seg_rsp;
    hal_ret_t               ret;

    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(1);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(1);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_NONE);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();

    return ret;
}

hal_ret_t
hal_default_cfg_init (hal_cfg_t *hal_cfg)
{
    hal_ret_t    ret;

    // create cpu interface
    ret = hal_cpu_if_create(SERVICE_LIF_CPU);
    HAL_ABORT(ret == HAL_RET_OK);

    // do QoS initialization
    ret = hal_qos_config_init(hal_cfg);
    HAL_ABORT(ret == HAL_RET_OK);

    // Create default vrf
    ret = hal_default_vrf_create();
    HAL_ABORT(ret == HAL_RET_OK);

    // Create default L2 Segment
    ret = hal_default_l2segment_create();
    HAL_ABORT(ret == HAL_RET_OK);
 
    // TODO acls need cpu interface. Right now, in GFT mode, the
    // hal_cpu_if_create() fails, so cpu interface is not created
    // So skipping installation of acls
    if (hal_cfg->features != HAL_FEATURE_SET_GFT) {
        // do acl creates after qos creates. acls depend on qos config
        ret = hal_acl_config_init();
        HAL_ABORT(ret == HAL_RET_OK);
    }

    return HAL_RET_OK;
}

}    // namespace hal
