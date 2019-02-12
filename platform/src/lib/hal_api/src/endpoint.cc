
#include <iostream>
#include <iomanip>

#include "gen/proto/kh.grpc.pb.h"
#include "gen/proto/types.grpc.pb.h"

#include "endpoint.hpp"
#include "print.hpp"

using namespace std;

std::map<ep_key_t, HalEndpoint*> HalEndpoint::ep_db;

HalEndpoint *
HalEndpoint::Factory(HalL2Segment *l2seg, mac_t mac, Enic *enic)
{
    hal_irisc_ret_t ret = HAL_IRISC_RET_SUCCESS;
    ep_key_t ep_key(l2seg, mac);

    HalEndpoint *ep = new HalEndpoint(l2seg, mac, enic);

    ret = ep->HalEndpointCreate();
    if (ret != HAL_IRISC_RET_SUCCESS) {
        NIC_LOG_DEBUG("EP Create failed. ret: {}", ret);
        goto end;
    }

    // Store in DB
    ep_db[ep_key] = ep;

end:
    if (ret != HAL_IRISC_RET_SUCCESS) {
        if (ep) {
            delete ep;
            ep = NULL;
        }
    }
    return ep;
}

hal_irisc_ret_t
HalEndpoint::Destroy(HalEndpoint *ep)
{
    hal_irisc_ret_t ret = HAL_IRISC_RET_SUCCESS;
    if (!ep) {
        return HAL_IRISC_RET_FAIL;
    }
    ep_key_t ep_key(ep->GetL2Seg(), ep->GetMac());

    ret = ep->HalEndpointDelete();
    if (ret != HAL_IRISC_RET_SUCCESS) {
        NIC_LOG_CRIT("EP Delete failed. ret: {}", ret);
        goto end;
    }
    delete ep;

    // Remove from DB
    ep_db.erase(ep_key);

end:
    return ret;
}

HalEndpoint *
HalEndpoint::Lookup(HalL2Segment *l2seg, mac_t mac)
{
    ep_key_t key(l2seg, mac);

    if (ep_db.find(key) != ep_db.cend()) {
        return ep_db[key];
    } else {
        return NULL;
    }
}

HalEndpoint::HalEndpoint(HalL2Segment *l2seg, mac_t mac, Enic *enic)
{

    NIC_LOG_DEBUG("EP create: l2seg: {}, mac: {}, enic: {}",
                    l2seg->GetId(), macaddr2str(mac), enic->GetId());

    this->mac = mac;
    this->l2seg = l2seg;
    this->enic = enic;

}

hal_irisc_ret_t
HalEndpoint::HalEndpointCreate()
{
    hal_irisc_ret_t                 ret = HAL_IRISC_RET_SUCCESS;
    HalL2Segment                    *l2seg;
    mac_t                           mac;
    Enic                            *enic;
    grpc::ClientContext             context;
    grpc::Status                    status;
    endpoint::EndpointSpec          *req;
    endpoint::EndpointResponse      rsp;
    endpoint::EndpointRequestMsg    req_msg;
    endpoint::EndpointResponseMsg   rsp_msg;

    l2seg = this->l2seg;
    mac = this->mac;
    enic = this->enic;

    req = req_msg.add_request();
    req->mutable_vrf_key_handle()->set_vrf_id(l2seg->GetVrf()->GetId());
    req->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(mac);
    req->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_segment_id(l2seg->GetId());
    req->mutable_endpoint_attrs()->mutable_interface_key_handle()->set_interface_id(enic->GetId());
    status = hal->endpoint_create(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            NIC_LOG_ERR("Failed to create EP L2seg: {}, Mac: {}. err: {}",
                        l2seg->GetId(), macaddr2str(mac),
                        rsp.api_status());
            ret = HAL_IRISC_RET_FAIL;
        } else {
            NIC_LOG_DEBUG("Created EP L2seg: {}, Mac: {}", l2seg->GetId(), macaddr2str(mac));
        }
    } else {
        NIC_LOG_ERR("Failed to create EP L2seg: {}, Mac: {}. err: {}, msg: {}", l2seg->GetId(),
                    macaddr2str(mac), status.error_code(), status.error_message());
        ret = HAL_IRISC_RET_FAIL;
    }

    return ret;
}

hal_irisc_ret_t
HalEndpoint::HalEndpointDelete()
{
    hal_irisc_ret_t                       ret = HAL_IRISC_RET_SUCCESS;
    grpc::ClientContext                   context;
    grpc::Status                          status;

    endpoint::EndpointDeleteRequest       *req;
    endpoint::EndpointDeleteResponse      rsp;
    endpoint::EndpointDeleteRequestMsg    req_msg;
    endpoint::EndpointDeleteResponseMsg   rsp_msg;

    NIC_LOG_DEBUG("EP delete: l2seg: {}, mac: {}, enic: {}",
                  l2seg->GetId(), macaddr2str(mac), enic->GetId());

    req = req_msg.add_request();
    req->mutable_vrf_key_handle()->set_vrf_id(l2seg->GetVrf()->GetId());
    req->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(mac);
    req->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_segment_id(l2seg->GetId());

    status = hal->endpoint_delete(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            NIC_LOG_ERR("Failed to delete EP L2seg: {}, Mac: {}. err: {}", l2seg->GetId(), macaddr2str(mac),
                        rsp.api_status());
            ret = HAL_IRISC_RET_FAIL;
        } else {
            NIC_LOG_DEBUG("Delete EP L2seg: {}, Mac: {}", l2seg->GetId(), macaddr2str(mac));
        }
    } else {
        NIC_LOG_ERR("Failed to delete EP L2seg: {}, Mac: {}. err: {}, msg: {}", l2seg->GetId(),
                    macaddr2str(mac), status.error_code(), status.error_message());
        ret = HAL_IRISC_RET_FAIL;
    }

    return ret;
}

HalL2Segment *
HalEndpoint::GetL2Seg()
{
    return l2seg;
}
mac_t
HalEndpoint::GetMac()
{
    return mac;
}
