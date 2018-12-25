// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include <iostream>
#include <grpc++/grpc++.h>
#include "gen/proto/types.grpc.pb.h"
#include "uplink.hpp"
#include "vrf.hpp"
#include "l2seg.hpp"
#include "print.hpp"

using namespace std;

std::map<uint64_t, Uplink*> Uplink::uplink_db;

Uplink *
Uplink::Factory(uplink_id_t id, bool is_oob)
{
    api_trace("Uplink Create");

    if (uplink_db.find(id) != uplink_db.end()) {
        NIC_LOG_WARN("Duplicate Create of Uplink with id: {}",
                       id);
        return NULL;
    }

    NIC_LOG_DEBUG("Id: {}, is_oob: {}", id, is_oob);

    Uplink *uplink = new Uplink(id, is_oob);

    // Store in DB for disruptive restart
    uplink_db[uplink->GetId()] = uplink;

    return uplink;
}

void
Uplink::Destroy(Uplink *uplink)
{
    api_trace("Uplink Delete");

    if (uplink->GetVrf()) {
        // Delete Vrf
        HalVrf::Destroy(uplink->GetVrf());
    }
#if 0
    if (hal->GetMode() == FWD_MODE_CLASSIC ||
        uplink->IsOOB()) {

        // Delete Vrf
        HalVrf::Destroy(uplink->GetVrf());
    }
#endif

    // Remove from DB
    uplink_db.erase(uplink->GetId());

    if (uplink) {
        uplink->~Uplink();
    }
}

Uplink::Uplink(uplink_id_t id, bool is_oob)
{
    NIC_LOG_DEBUG("Uplink Create: {}", id);
    id_       = id;
    is_oob_   = is_oob;
    num_lifs_ = 0;
    port_num_ = -1;
}

void
Uplink::CreateVrf()
{
    if (!hal) {
        PopulateHalCommonClient();
    }

    // In both Classic and hostpin modes, every uplink will get a VRF.
    vrf_ = HalVrf::Factory(IsOOB() ? types::VRF_TYPE_OOB_MANAGEMENT : types::VRF_TYPE_INBAND_MANAGEMENT,
                           this);

#if 0
    if (hal->GetMode() == FWD_MODE_CLASSIC || IsOOB()) {
        // Create VRF for each uplink
        vrf = HalVrf::Factory(IsOOB() ? types::VRF_TYPE_MANAGEMENT : types::VRF_TYPE_CUSTOMER,
                              this);
    }
#endif
}

int
Uplink::UpdateHalWithNativeL2seg(uint32_t native_l2seg_id)
{
    grpc::Status                    status;
    InterfaceGetRequest             *req __attribute__((unused));
    InterfaceGetResponse            rsp;
    InterfaceGetRequestMsg          req_msg;
    InterfaceGetResponseMsg         rsp_msg;
    InterfaceSpec                   *if_spec;
    InterfaceResponse               if_rsp;
    InterfaceRequestMsg             if_req_msg;
    InterfaceResponseMsg            if_rsp_msg;

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_interface_id(id_);
    status = hal->interface_get(req_msg, rsp_msg);
    if (status.ok()) {
        NIC_LOG_DEBUG("Updated Uplink:{} with native l2seg:{}", id_, native_l2seg_id);
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            rsp = rsp_msg.response(i);
            if (rsp.api_status() == types::API_STATUS_OK) {
                if (rsp.spec().type() == intf::IF_TYPE_UPLINK) {

                    if_spec = if_req_msg.add_request();
                    if_spec->CopyFrom(rsp.spec());
                    if_spec->mutable_if_uplink_info()->
                        set_native_l2segment_id(native_l2seg_id);
                    if_spec->mutable_if_uplink_info()->set_is_oob_management(is_oob_);

                    status = hal->interface_update(if_req_msg, if_rsp_msg);
                    if (status.ok()) {
                        rsp = rsp_msg.response(0);
                        if (rsp.api_status() == types::API_STATUS_OK) {
                            NIC_LOG_DEBUG("Uplink {} updated with "
                                          "native_l2seg_id: {} succeeded",
                                          id_, native_l2seg_id);
                        } else {
                            NIC_LOG_ERR("Failed to update uplink's:{} "
                                        "native_l2seg_id:{}: err: {}",
                                        id_, native_l2seg_id,
                                        rsp.api_status());
                        }
                    } else {
                        NIC_LOG_ERR("Failed to update uplink's:{} "
                                    "native_l2seg_id:{}: err: {}, err_msg: {}",
                                    status.error_code(),
                                    status.error_message());
                    }
                }
            }
        }
    }

    return 0;
}

uint32_t
Uplink::GetId()
{
    return id_;
}

uint32_t
Uplink::GetPortNum()
{
    return port_num_;
}

uint32_t
Uplink::GetNumLifs()
{
    return num_lifs_;
}

void
Uplink::IncNumLifs()
{
    num_lifs_++;
}

void
Uplink::DecNumLifs()
{
    num_lifs_--;
}

HalVrf *
Uplink::GetVrf()
{
    return vrf_;
}

HalL2Segment *
Uplink::GetNativeL2Seg()
{
    return native_l2seg_;
}

bool
Uplink::IsOOB()
{
    return is_oob_;
}

void
Uplink::SetNativeL2Seg(HalL2Segment *l2seg)
{
    native_l2seg_ = l2seg;
}

void
Uplink::SetPortNum(uint32_t port_num)
{
    port_num_ = port_num;
}


