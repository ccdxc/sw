
#include <string>
#include <memory>
#include <iostream>
// #include <grpc++/grpc++.h>

#include "hal.hpp"

using namespace std;

#if 0
shared_ptr<HalCommonClient> HalCommonClient::instance = 0;

HalCommonClient::HalCommonClient(enum HalForwardingMode mode)
{
    std::string svc_url;

    if (getenv("HAL_SOCK_PATH")) {
        svc_url = std::string("unix:") + std::getenv("HAL_SOCK_PATH") + "halsock";
    } else if (getenv("HAL_GRPC_PORT")) {
        svc_url = std::string("localhost:") + getenv("HAL_GRPC_PORT");
    } else {
        svc_url = std::string("localhost:50054");
    }

    channel = grpc::CreateChannel(svc_url, grpc::InsecureChannelCredentials());

    cout << "[INFO] Waiting for HAL to be ready ..." << endl;
    auto state = channel->GetState(true);
    while (state != GRPC_CHANNEL_READY) {
        // Wait for State change or deadline
        channel->WaitForStateChange(state, gpr_time_from_seconds(1, GPR_TIMESPAN));
        state = channel->GetState(true);
        // cout << "[INFO] Connecting to HAL, channel status = " << channel->GetState(true) << endl;
    }

    vrf_stub_ = vrf::Vrf::NewStub(channel);
    intf_stub_ = intf::Interface::NewStub(channel);
    ep_stub_ = endpoint::HalEndpoint::NewStub(channel);
    l2seg_stub_ = l2segment::HalL2Segment::NewStub(channel);
    multicast_stub_ = multicast::HalMulticast::NewStub(channel);
    rdma_stub_ = rdma::Rdma::NewStub(channel);

    this->mode = mode;
}

shared_ptr<HalCommonClient>
HalCommonClient::GetInstance()
{
    if (!instance) {
        throw ("HalCommonClient instance is not created!");
    }

    return instance;
}

shared_ptr<HalCommonClient>
HalCommonClient::GetInstance(enum HalForwardingMode fwd_mode)
{
    if (!instance) {
        instance = shared_ptr<HalCommonClient>(new HalCommonClient(fwd_mode));
    }

    return instance;
}

enum HalForwardingMode
HalCommonClient::GetMode()
{
    return mode;
}
#endif

/**
 * Hal Object Base class
 */
HalCommonClient *HalObject::hal = NULL;

HalObject::HalObject()
{
    hal = HalCommonClient::GetInstance();
}

void
HalObject::PopulateHalCommonClient()
{
    hal = HalCommonClient::GetInstance();
}
