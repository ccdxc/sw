//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "platform/src/lib/nicmgr/include/logger.hpp"
#include "qos.hpp"
#include "utils.hpp"
#include "hal_grpc.hpp"

namespace iris {

int32_t
devapi_qos::get_txtc_cos(const std::string &group, uint32_t uplink_port)
{
    grpc::Status                status;
    qos::QosClassGetRequestMsg  req_msg;
    qos::QosClassGetResponseMsg rsp_msg;
    kh::QosGroup                qos_group;


    if (!kh::QosGroup_Parse(group, &qos_group)) {
        NIC_LOG_ERR("Failed to parse the qos group {}", group);
        return -1;
    }

    auto req = req_msg.add_request();
    req->mutable_key_or_handle()->set_qos_group(qos_group);

    status = hal->qos_class_get(req_msg, rsp_msg);
    if (!status.ok()) {
        NIC_LOG_ERR("Failed to get qos class for group {} err {}:{}",
                    group, status.error_code(), status.error_message());
        return -1;
    }

    auto get_rsp = rsp_msg.response(0);
    auto num_coses = get_rsp.status().epd_status().tx_traffic_class_coses_size();
    if (num_coses < 1) {
        NIC_LOG_ERR("No traffic class coses allocated for group {}",
                    group);
        return -1;
    }
    return get_rsp.status().epd_status().tx_traffic_class_coses(uplink_port%num_coses);
}

sdk_ret_t
devapi_qos::qos_class_exist(uint8_t group)
{
    sdk_ret_t                   ret = SDK_RET_OK;
    grpc::Status                status;
    qos::QosClassGetRequestMsg  req_msg;
    qos::QosClassGetResponseMsg rsp_msg;
    qos::QosClassGetResponse    rsp;

    auto req = req_msg.add_request();
    req->mutable_key_or_handle()->set_qos_group((kh::QosGroup)group);

    VERIFY_HAL();
    status = hal->qos_class_get(req_msg, rsp_msg);
    if (!status.ok()) {
        NIC_LOG_ERR("Failed to get qos class for group {} err {}:{}",
                    group, status.error_code(), status.error_message());
        ret = SDK_RET_ERR;
        goto end;
    }

    rsp = rsp_msg.response(0);
    if (rsp.api_status() != types::API_STATUS_OK) {
        NIC_LOG_DEBUG("qos class {} get failed", group);
        ret = SDK_RET_ERR;
        goto end;
    }

 end:
    return ret;
}

sdk_ret_t
devapi_qos::qos_class_get(uint8_t group, qos_class_info_t *info)
{
    sdk_ret_t                   ret = SDK_RET_OK;
    grpc::Status                status;
    qos::QosClassGetRequestMsg  req_msg;
    qos::QosClassGetResponseMsg rsp_msg;
    qos::QosClassGetResponse    rsp;

    auto req = req_msg.add_request();
    req->mutable_key_or_handle()->set_qos_group((kh::QosGroup)group);

    VERIFY_HAL();
    status = hal->qos_class_get(req_msg, rsp_msg);
    if (!status.ok()) {
        NIC_LOG_ERR("Failed to get qos class for group {} err {}:{}",
                    group, status.error_code(), status.error_message());
        ret = SDK_RET_ERR;
        goto end;
    }

    rsp = rsp_msg.response(0);
    if (rsp.api_status() != types::API_STATUS_OK) {
        NIC_LOG_ERR("qos class {} get failed", group);
        ret = SDK_RET_ERR;
        goto end;
    }

    info->group = rsp.spec().key_or_handle().qos_group();
    info->mtu = rsp.spec().mtu();
    info->no_drop = rsp.spec().no_drop();

    /* flowcontrol configuration */
    info->pause_type = rsp.spec().pause().type();
    info->pause_dot1q_pcp = rsp.spec().pause().pfc_cos();
    info->pause_xon_threshold = rsp.spec().pause().xon_threshold();
    info->pause_xoff_threshold = rsp.spec().pause().xoff_threshold();

    /* scheduler configuration */
    if (rsp.spec().sched().has_strict()) {
        info->sched_type = sdk::platform::QOS_SCHED_TYPE_STRICT;
        info->sched_strict_rlmt = rsp.spec().sched().strict().bps();
    } else if (rsp.spec().sched().has_dwrr()) {
        info->sched_type = sdk::platform::QOS_SCHED_TYPE_DWRR;
        info->sched_dwrr_weight = rsp.spec().sched().dwrr().bw_percentage();
    }

    /* class map configuration */
    if (rsp.spec().class_map().type() == qos::QosClassMapType::QOS_CLASS_MAP_TYPE_PCP) {
        info->class_type = sdk::platform::QOS_CLASS_TYPE_PCP;
        info->class_dot1q_pcp = rsp.spec().class_map().dot1q_pcp();
    } else if (rsp.spec().class_map().type() == qos::QosClassMapType::QOS_CLASS_MAP_TYPE_DSCP) {
        info->class_type = sdk::platform::QOS_CLASS_TYPE_DSCP;
        info->class_ndscp =  rsp.spec().class_map().ip_dscp().size();
        for (uint8_t i = 0; i < rsp.spec().class_map().ip_dscp().size(); i++) {
            info->class_ip_dscp[i] = rsp.spec().class_map().ip_dscp()[i];
        }
    }

    /* marking configuration */
    info->rewrite_dot1q_pcp_en = rsp.spec().marking().dot1q_pcp_rewrite_en();
    info->rewrite_dot1q_pcp = rsp.spec().marking().dot1q_pcp();
    info->rewrite_ip_dscp_en = rsp.spec().marking().ip_dscp_rewrite_en();
    info->rewrite_ip_dscp = rsp.spec().marking().ip_dscp();

end:
    return ret;
}

sdk_ret_t
devapi_qos::qos_class_create(qos_class_info_t *info)
{
    sdk_ret_t                   ret = SDK_RET_OK;
    grpc::Status                status;
    qos::QosClassRequestMsg     req_msg;
    qos::QosClassResponseMsg    rsp_msg;

    auto req = req_msg.add_request();
    req->mutable_key_or_handle()->set_qos_group((kh::QosGroup)info->group);
    req->set_mtu(info->mtu);
    req->set_no_drop(info->no_drop);

    /* flowcontrol configuration */
    req->mutable_pause()->set_type((qos::QosPauseType)info->pause_type);
    req->mutable_pause()->set_pfc_cos(info->pause_dot1q_pcp);
    req->mutable_pause()->set_xon_threshold(info->pause_xon_threshold);
    req->mutable_pause()->set_xoff_threshold(info->pause_xoff_threshold);

    /* scheduler configuration */
    if (info->sched_type == sdk::platform::QOS_SCHED_TYPE_STRICT) {
        req->mutable_sched()->mutable_strict()->set_bps(info->sched_strict_rlmt);
    } else if (info->sched_type == sdk::platform::QOS_SCHED_TYPE_DWRR) {
        req->mutable_sched()->mutable_dwrr()->set_bw_percentage(info->sched_dwrr_weight);
    } else {
        NIC_LOG_ERR("Unknown scheduler type {}", info->sched_type);
        ret = SDK_RET_ERR;
        goto end;
    }

    /* class map configuration */
    req->mutable_class_map()->set_type((qos::QosClassMapType)info->class_type);
    if (info->class_type == sdk::platform::QOS_CLASS_TYPE_PCP) {
        req->mutable_class_map()->set_dot1q_pcp(info->class_dot1q_pcp);
    } else if (info->class_type == sdk::platform::QOS_CLASS_TYPE_DSCP) {
        for (uint8_t i = 0; i < info->class_ndscp; i++) {
            req->mutable_class_map()->mutable_ip_dscp()->Add(info->class_ip_dscp[i]);
        }
    } else {
        NIC_LOG_ERR("Unknown class map type {}", info->class_type);
        ret = SDK_RET_ERR;
        goto end;
    }

    /* marking configuration */
    req->mutable_marking()->set_dot1q_pcp_rewrite_en(info->rewrite_dot1q_pcp_en);
    req->mutable_marking()->set_dot1q_pcp(info->rewrite_dot1q_pcp);
    req->mutable_marking()->set_ip_dscp_rewrite_en(info->rewrite_ip_dscp_en);
    req->mutable_marking()->set_ip_dscp(info->rewrite_ip_dscp);

    VERIFY_HAL();
    status = hal->qos_class_create(req_msg, rsp_msg);
    if (status.ok()) {
        auto rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_LOG_DEBUG("qos class {} created", info->group);
        } else {
            NIC_LOG_ERR("qos class {} create failed", info->group);
            ret = SDK_RET_ERR;
            goto end;
        }
    } else {
        NIC_LOG_ERR("qos class {} create failed. err: {}:{}",
            info->group, status.error_code(), status.error_message());
        ret = SDK_RET_ERR;
        goto end;
    }

end:
    return ret;
}

sdk_ret_t
devapi_qos::qos_class_delete(uint8_t group, bool clear_stats)
{
    sdk_ret_t                         ret = SDK_RET_OK;
    grpc::Status                      status;
    qos::QosClassDeleteRequestMsg     req_msg;
    qos::QosClassDeleteResponseMsg    rsp_msg;

    auto req = req_msg.add_request();
    req->mutable_key_or_handle()->set_qos_group((kh::QosGroup)group);
    req->set_clear_stats(clear_stats);

    VERIFY_HAL();
    status = hal->qos_class_delete(req_msg, rsp_msg);
    if (status.ok()) {
        auto rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_LOG_DEBUG("qos class {} deleted clear_stats {}", group, clear_stats);
        } else {
            NIC_LOG_ERR("qos class {} delete failed", group);
            ret = SDK_RET_ERR;
            goto end;
        }
    }

end:
    return ret;
}

sdk_ret_t
devapi_qos::qos_clear_port_stats(uint32_t port_num)
{
    sdk_ret_t                         ret = SDK_RET_OK;
    grpc::Status                      status;
    qos::QosClearPortStatsRequestMsg     req_msg;
    qos::QosClearPortStatsResponseMsg    rsp_msg;

    auto req = req_msg.add_request();
    req->set_port_num(port_num);

    VERIFY_HAL();
    status = hal->qos_clear_port_stats(req_msg, rsp_msg);
    if (status.ok()) {
        auto rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_LOG_DEBUG("qos port stats cleared for port {}", port_num);
        } else {
            NIC_LOG_ERR("qos port stats clear failed for port {}", port_num);
            ret = SDK_RET_ERR;
            goto end;
        }
    }

end:
    return ret;
}

sdk_ret_t
devapi_qos::qos_class_set_global_pause_type(uint8_t pause_type)
{
    sdk_ret_t                                  ret = SDK_RET_OK;
    grpc::Status                               status;
    qos::QosClassSetGlobalPauseTypeRequestMsg  req_msg;
    qos::QosClassSetGlobalPauseTypeResponseMsg rsp_msg;

    auto req = req_msg.add_request();

    pause_type &= PORT_CFG_PAUSE_TYPE_MASK;

    if ( (pause_type != sdk::platform::PAUSE_TYPE_LINK_LEVEL) &&
         (pause_type != sdk::platform::PAUSE_TYPE_PFC) &&
         (pause_type != sdk::platform::PAUSE_TYPE_NONE) ) {
        NIC_LOG_ERR("Unknown pause type {}", pause_type);
        ret = SDK_RET_ERR;
        goto end;
    }

    req->set_pause_type((qos::QosPauseType) pause_type);

    VERIFY_HAL();
    status = hal->qos_class_set_global_pause_type(req_msg, rsp_msg);
    if (status.ok()) {
        NIC_LOG_DEBUG("set GlobalPauseType to {}", pause_type);
    } else {
        NIC_LOG_ERR("FAILED to set GlobalPauseType to {} ; err: {}:{}",
                    pause_type, status.error_code(), status.error_message());
        ret = SDK_RET_ERR;
        goto end;
    }

end:
    return ret;
}

}    // namespace iris
