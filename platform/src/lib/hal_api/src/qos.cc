
#include <grpc++/grpc++.h>

#include "gen/proto/kh.grpc.pb.h"

#include "qos.hpp"

int32_t
QosClass::GetTxTrafficClassCos(const std::string &group, uint32_t uplink_port)
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

    status = HalCommonClient::GetInstance()->qos_class_get(req_msg, rsp_msg);
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
