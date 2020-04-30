//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// uds test app
///
//----------------------------------------------------------------------------

#include <sys/un.h>
#include <sys/socket.h>
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/api/include/pds_dhcp.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/include/pds_policy.hpp"
#include "nic/apollo/api/include/pds_policer.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/api/include/pds_mirror.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_meter.hpp"
#include "nic/apollo/api/include/pds_tag.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"
#include "nic/apollo/agent/svc/route_svc.hpp"
#include "nic/apollo/agent/svc/mapping_svc.hpp"
#include "nic/apollo/agent/svc/policy_svc.hpp"
#include "nic/apollo/agent/svc/vnic_svc.hpp"
#include "nic/apollo/agent/svc/vpc_svc.hpp"
#include "nic/apollo/agent/svc/interface_svc.hpp"
#include "nic/apollo/agent/svc/subnet_svc.hpp"
#include "nic/apollo/agent/svc/tag_svc.hpp"
#include "nic/apollo/agent/svc/meter_svc.hpp"
#include "nic/apollo/agent/svc/policer_svc.hpp"
#include "nic/apollo/agent/svc/service_svc.hpp"
#include "nic/apollo/agent/svc/tunnel_svc.hpp"
#include "nic/apollo/agent/svc/nh_svc.hpp"
#include "nic/apollo/agent/svc/nat_svc.hpp"
#include "nic/apollo/agent/svc/device_svc.hpp"
#include "nic/apollo/agent/svc/dhcp_svc.hpp"
#include "nic/apollo/agent/svc/mirror_svc.hpp"
#include "nic/sdk/include/sdk/uds.hpp"

static char *g_iov_data;
#define SVC_SERVER_SOCKET_PATH          "/var/run/pds_svc_server_sock"
#define CMD_IOVEC_DATA_LEN (1024*1024)
#define FD_INVALID (-1)

using std::string;
using pds::VPCDeleteRequest;
using pds::VPCGetRequest;
using pds::VPCGetResponse;
using pds::VPCPeerRequest;
using pds::PolicerRequest;
using pds::NexthopRequest;
using pds::NexthopRequest;
using pds::NhGroupRequest;
using types::ServiceRequestMessage;
using types::ServiceResponseMessage;
using types::ServiceRequestOp;

sdk_ret_t
service_request_send (ServiceRequestMessage *request,
                      ServiceResponseMessage *response)
{
    int sock_fd = -1;
    int ret, bytes_read;
    string serial_request;
    struct sockaddr_un sock_addr;

    // Initialize unix socket
    if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        printf("Failed to open unix domain socket\n");
        return SDK_RET_ERR;
    }
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sun_family = AF_UNIX;
    strcpy(sock_addr.sun_path, SVC_SERVER_SOCKET_PATH);
    if (connect(sock_fd, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) == -1) {
        printf("Failed to connect to unix domain socket\n");
        goto err;
    }
    if (!request->SerializeToString(&serial_request)) {
        printf("Failed to serialize request\n");
        goto err;
    }
    ret = uds_send(sock_fd, FD_INVALID,
                   (char *)serial_request.c_str(),
                   serial_request.length());
    if (ret < 0) {
        printf("Socket send failed\n");
        goto err;
    }
    bytes_read = uds_recv(sock_fd, NULL,
                          g_iov_data, CMD_IOVEC_DATA_LEN);
    if (bytes_read < 0) {
        printf("Response not received, request size %lu\n", serial_request.length());
        goto err;
    }
    if (response->ParseFromArray(g_iov_data, bytes_read) != true) {
        printf("Failed to parse response\n");
        goto err;
    }

    close(sock_fd);
    return SDK_RET_OK;

err:

    close(sock_fd);
    return SDK_RET_ERR;
}

sdk_ret_t
create_route_table_impl (pds_route_table_spec_t *spec)
{
    sdk_ret_t               ret;
    RouteTableRequest       request;
    ServiceRequestMessage   service_req;
    ServiceResponseMessage  service_rsp;

    if (spec) {
        auto proto_spec = request.add_request();
        auto any_req = service_req.mutable_configmsg();
        pds_route_table_api_spec_to_proto(proto_spec, spec);
        any_req->PackFrom(request);
        service_req.set_configop(types::SERVICE_OP_CREATE);
        ret = service_request_send(&service_req, &service_rsp);
        if ((ret != SDK_RET_OK) || (service_rsp.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_policy_impl (pds_policy_spec_t *spec)
{
    sdk_ret_t               ret;
    SecurityPolicyRequest   request;
    ServiceRequestMessage   service_req;
    ServiceResponseMessage  service_rsp;

    if (spec) {
        auto proto_spec = request.add_request();
        auto any_req = service_req.mutable_configmsg();
        pds_policy_api_spec_to_proto(proto_spec, spec);
        any_req->PackFrom(request);
        service_req.set_configop(types::SERVICE_OP_CREATE);
        ret = service_request_send(&service_req, &service_rsp);
        if ((ret != SDK_RET_OK) || (service_rsp.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_local_mapping_impl (pds_local_mapping_spec_t *spec)
{
    sdk_ret_t               ret;
    MappingRequest          request;
    ServiceRequestMessage   service_req;
    ServiceResponseMessage  service_rsp;

    if (spec) {
        auto proto_spec = request.add_request();
        auto any_req = service_req.mutable_configmsg();
        pds_local_mapping_api_spec_to_proto(proto_spec, spec);
        any_req->PackFrom(request);
        service_req.set_configop(types::SERVICE_OP_CREATE);
        ret = service_request_send(&service_req, &service_rsp);
        if ((ret != SDK_RET_OK) || (service_rsp.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_remote_mapping_impl (pds_remote_mapping_spec_t *spec)
{
    sdk_ret_t               ret;
    MappingRequest          request;
    ServiceRequestMessage   service_req;
    ServiceResponseMessage  service_rsp;

    if (spec) {
        auto proto_spec = request.add_request();
        auto any_req = service_req.mutable_configmsg();
        pds_remote_mapping_api_spec_to_proto(proto_spec, spec);
        any_req->PackFrom(request);
        service_req.set_configop(types::SERVICE_OP_CREATE);
        ret = service_request_send(&service_req, &service_rsp);
        if ((ret != SDK_RET_OK) || (service_rsp.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_vnic_impl (pds_vnic_spec_t *spec)
{
    sdk_ret_t               ret;
    VnicRequest             request;
    ServiceRequestMessage   service_req;
    ServiceResponseMessage  service_rsp;

    if (spec) {
        auto proto_spec = request.add_request();
        auto any_req = service_req.mutable_configmsg();
        pds_vnic_api_spec_to_proto(proto_spec, spec);
        any_req->PackFrom(request);
        service_req.set_configop(types::SERVICE_OP_CREATE);
        ret = service_request_send(&service_req, &service_rsp);
        if ((ret != SDK_RET_OK) || (service_rsp.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_subnet_impl (pds_subnet_spec_t *spec)
{
    sdk_ret_t               ret;
    SubnetRequest           request;
    ServiceRequestMessage   service_req;
    ServiceResponseMessage  service_rsp;

    if (spec) {
        auto proto_spec = request.add_request();
        auto any_req = service_req.mutable_configmsg();
        pds_subnet_api_spec_to_proto(proto_spec, spec);
        any_req->PackFrom(request);
        service_req.set_configop(types::SERVICE_OP_CREATE);
        ret = service_request_send(&service_req, &service_rsp);
        if ((ret != SDK_RET_OK) || (service_rsp.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_nat_port_block_impl (pds_nat_port_block_spec_t *spec)
{
    sdk_ret_t               ret;
    NatPortBlockRequest     request;
    ServiceRequestMessage   service_req;
    ServiceResponseMessage  service_rsp;

    if (spec) {
        auto proto_spec = request.add_request();
        auto any_req = service_req.mutable_configmsg();
        pds_nat_port_block_api_spec_to_proto(proto_spec, spec);
        any_req->PackFrom(request);
        service_req.set_configop(types::SERVICE_OP_CREATE);
        ret = service_request_send(&service_req, &service_rsp);
        if ((ret != SDK_RET_OK) || (service_rsp.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_vpc_impl (pds_vpc_spec_t *spec)
{
    sdk_ret_t               ret;
    VPCRequest              request;
    ServiceRequestMessage   service_req;
    ServiceResponseMessage  service_rsp;

    if (spec) {
        auto proto_spec = request.add_request();
        auto any_req = service_req.mutable_configmsg();
        pds_vpc_api_spec_to_proto(proto_spec, spec);
        any_req->PackFrom(request);
        service_req.set_configop(types::SERVICE_OP_CREATE);
        ret = service_request_send(&service_req, &service_rsp);
        if ((ret != SDK_RET_OK) || (service_rsp.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_dhcp_policy_impl (pds_dhcp_policy_spec_t *spec)
{
    sdk_ret_t               ret;
    DHCPPolicyRequest       request;
    ServiceRequestMessage   service_req;
    ServiceResponseMessage  service_rsp;

    if (spec) {
        auto proto_spec = request.add_request();
        auto any_req = service_req.mutable_configmsg();
        pds_dhcp_policy_api_spec_to_proto(proto_spec, spec);
        any_req->PackFrom(request);
        service_req.set_configop(types::SERVICE_OP_CREATE);
        ret = service_request_send(&service_req, &service_rsp);
        if ((ret != SDK_RET_OK) || (service_rsp.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
update_vpc_impl (pds_vpc_spec_t *spec)
{
    sdk_ret_t               ret;
    VPCRequest              request;
    ServiceRequestMessage   service_req;
    ServiceResponseMessage  service_rsp;

    if (spec) {
        auto proto_spec = request.add_request();
        auto any_req = service_req.mutable_configmsg();
        pds_vpc_api_spec_to_proto(proto_spec, spec);
        any_req->PackFrom(request);
        service_req.set_configop(types::SERVICE_OP_UPDATE);
        ret = service_request_send(&service_req, &service_rsp);
        if ((ret != SDK_RET_OK) || (service_rsp.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
delete_vpc_impl (pds_obj_key_t *key)
{
    sdk_ret_t               ret;
    VPCDeleteRequest        request;
    ServiceRequestMessage   service_req;
    ServiceResponseMessage  service_rsp;

    if (key) {
        auto any_req = service_req.mutable_configmsg();
        request.add_id(key->id);
        any_req->PackFrom(request);
        service_req.set_configop(types::SERVICE_OP_DELETE);
        ret = service_request_send(&service_req, &service_rsp);
        if ((ret != SDK_RET_OK) || (service_rsp.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
read_vpc_impl (pds_obj_key_t *key, pds_vpc_info_t *info)
{
    sdk_ret_t               ret;
    pds::VPCSpec            vpcspec;
    VPCGetRequest           request;
    VPCGetResponse          response;
    ServiceRequestMessage   service_req;
    ServiceResponseMessage  service_rsp;

    if (key) {
        auto any_req = service_req.mutable_configmsg();
        request.add_id(key->id);
        any_req->PackFrom(request);
        service_req.set_configop(types::SERVICE_OP_READ);
        ret = service_request_send(&service_req, &service_rsp);
        if (ret != SDK_RET_OK) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
        if (service_rsp.apistatus() == types::API_STATUS_NOT_FOUND) {
            return SDK_RET_ENTRY_NOT_FOUND;
        } else if (service_rsp.apistatus() == types::API_STATUS_OK) {
            auto any_rsp = service_rsp.response();
            any_rsp.UnpackTo(&response);
            auto get_rsp = response.response(0); // get first info
            vpcspec = get_rsp.spec();
            pds_vpc_proto_to_api_spec(&info->spec, vpcspec);
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_l3_intf_impl (pds_if_spec_t *spec)
{
    sdk_ret_t               ret;
    InterfaceRequest        request;
    ServiceRequestMessage   service_req;
    ServiceResponseMessage  service_rsp;

    if (spec) {
        auto any_req = service_req.mutable_configmsg();
        auto proto_spec = request.add_request();
        pds_if_api_spec_to_proto(proto_spec, spec);
        any_req->PackFrom(request);
        service_req.set_configop(types::SERVICE_OP_CREATE);
        ret = service_request_send(&service_req, &service_rsp);
        if ((ret != SDK_RET_OK) || (service_rsp.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_vpc_peer_impl (pds_vpc_peer_spec_t *spec)
{
    sdk_ret_t               ret;
    VPCPeerRequest          request;
    ServiceRequestMessage   service_req;
    ServiceResponseMessage  service_rsp;

    if (spec) {
        auto proto_spec = request.add_request();
        auto any_req = service_req.mutable_configmsg();
        pds_vpc_peer_api_spec_to_proto(proto_spec, spec);
        any_req->PackFrom(request);
        service_req.set_configop(types::SERVICE_OP_CREATE);
        ret = service_request_send(&service_req, &service_rsp);
        if ((ret != SDK_RET_OK) || (service_rsp.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_tag_impl (pds_tag_spec_t *spec)
{
    // not implemented
    return SDK_RET_OK;
}

sdk_ret_t
create_meter_impl (pds_meter_spec_t *spec)
{
    sdk_ret_t               ret;
    MeterRequest            request;
    ServiceRequestMessage   service_req;
    ServiceResponseMessage  service_rsp;

    if (spec) {
        auto proto_spec = request.add_request();
        auto any_req = service_req.mutable_configmsg();
        pds_meter_api_spec_to_proto(proto_spec, spec);
        any_req->PackFrom(request);
        service_req.set_configop(types::SERVICE_OP_CREATE);
        ret = service_request_send(&service_req, &service_rsp);
        if ((ret != SDK_RET_OK) || (service_rsp.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_policer_impl (pds_policer_spec_t *spec)
{
    sdk_ret_t               ret;
    PolicerRequest          request;
    ServiceRequestMessage   service_req;
    ServiceResponseMessage  service_rsp;

    if (spec) {
        auto proto_spec = request.add_request();
        auto any_req = service_req.mutable_configmsg();
        pds_policer_api_spec_to_proto(proto_spec, spec);
        any_req->PackFrom(request);
        service_req.set_configop(types::SERVICE_OP_CREATE);
        ret = service_request_send(&service_req, &service_rsp);
        if ((ret != SDK_RET_OK) || (service_rsp.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_nexthop_impl (pds_nexthop_spec_t *spec)
{
    sdk_ret_t               ret;
    NexthopRequest          request;
    ServiceRequestMessage   service_req;
    ServiceResponseMessage  service_rsp;

    if (spec) {
        auto proto_spec = request.add_request();
        auto any_req = service_req.mutable_configmsg();
        pds_nh_api_spec_to_proto(proto_spec, spec);
        any_req->PackFrom(request);
        service_req.set_configop(types::SERVICE_OP_CREATE);
        ret = service_request_send(&service_req, &service_rsp);
        if ((ret != SDK_RET_OK) || (service_rsp.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_nexthop_group_impl (pds_nexthop_group_spec_t *spec)
{
    sdk_ret_t               ret;
    NhGroupRequest          request;
    ServiceRequestMessage   service_req;
    ServiceResponseMessage  service_rsp;

    if (spec) {
        auto proto_spec = request.add_request();
        auto any_req = service_req.mutable_configmsg();
        pds_nh_group_api_spec_to_proto(proto_spec, spec);
        any_req->PackFrom(request);
        service_req.set_configop(types::SERVICE_OP_CREATE);
        ret = service_request_send(&service_req, &service_rsp);
        if ((ret != SDK_RET_OK) || (service_rsp.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_tunnel_impl (uint32_t id, pds_tep_spec_t *spec)
{
    sdk_ret_t               ret;
    TunnelRequest           request;
    ServiceRequestMessage   service_req;
    ServiceResponseMessage  service_rsp;

    if (spec) {
        auto proto_spec = request.add_request();
        auto any_req = service_req.mutable_configmsg();
        pds_tep_api_spec_to_proto(proto_spec, spec);
        any_req->PackFrom(request);
        service_req.set_configop(types::SERVICE_OP_CREATE);
        ret = service_request_send(&service_req, &service_rsp);
        if ((ret != SDK_RET_OK) || (service_rsp.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_device_impl (pds_device_spec_t *spec)
{
    sdk_ret_t               ret;
    DeviceRequest           request;
    ServiceRequestMessage   service_req;
    ServiceResponseMessage  service_rsp;

    if (spec) {
        auto proto_spec = request.mutable_request();
        auto any_req = service_req.mutable_configmsg();
        pds_device_api_spec_to_proto(proto_spec, spec);
        any_req->PackFrom(request);
        service_req.set_configop(types::SERVICE_OP_CREATE);
        ret = service_request_send(&service_req, &service_rsp);
        if ((ret != SDK_RET_OK) || (service_rsp.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_mirror_session_impl (pds_mirror_session_spec_t *spec)
{
    // not implemented
    return SDK_RET_OK;
}

sdk_ret_t
create_svc_mapping_impl (pds_svc_mapping_spec_t *spec)
{
    sdk_ret_t               ret;
    SvcMappingRequest       request;
    ServiceRequestMessage   service_req;
    ServiceResponseMessage  service_rsp;

    if (spec) {
        auto proto_spec = request.add_request();
        auto any_req = service_req.mutable_configmsg();
        pds_service_api_spec_to_proto(proto_spec, spec);
        any_req->PackFrom(request);
        service_req.set_configop(types::SERVICE_OP_CREATE);
        ret = service_request_send(&service_req, &service_rsp);
        if ((ret != SDK_RET_OK) || (service_rsp.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
test_app_init (void)
{
    g_iov_data = (char *)SDK_CALLOC(SDK_MEM_ALLOC_TESTAPP, CMD_IOVEC_DATA_LEN);
    if (g_iov_data == NULL) {
        printf("Failed to allocate memory\n");
        return SDK_RET_OOM;
    }

    return SDK_RET_OK;
}

void
test_app_deinit (void)
{
    SDK_FREE(SDK_MEM_ALLOC_TESTAPP, g_iov_data);
}
