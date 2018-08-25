
#include <string>
#include <memory>
#include <iostream>
// #include <grpc++/grpc++.h>

#include "hal.hpp"

using namespace std;

#if 0
shared_ptr<HalClient> HalClient::instance = 0;

HalClient::HalClient(enum ForwardingMode mode)
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
    ep_stub_ = endpoint::Endpoint::NewStub(channel);
    l2seg_stub_ = l2segment::L2Segment::NewStub(channel);
    multicast_stub_ = multicast::Multicast::NewStub(channel);
    rdma_stub_ = rdma::Rdma::NewStub(channel);

    this->mode = mode;
}

shared_ptr<HalClient>
HalClient::GetInstance()
{
    if (!instance) {
        throw ("HalClient instance is not created!");
    }

    return instance;
}

shared_ptr<HalClient>
HalClient::GetInstance(enum ForwardingMode fwd_mode)
{
    if (!instance) {
        instance = shared_ptr<HalClient>(new HalClient(fwd_mode));
    }

    return instance;
}

enum ForwardingMode
HalClient::GetMode()
{
    return mode;
}
#endif

/**
 * Hal Object Base class
 */
HalClient *HalObject::hal = NULL;

HalObject::HalObject()
{
    hal = HalClient::GetInstance();
}
