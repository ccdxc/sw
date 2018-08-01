
#ifndef __HAL_HPP__
#define __HAL_HPP__

#include <map>
#include <grpc++/grpc++.h>

#include "vrf.grpc.pb.h"
#include "interface.grpc.pb.h"
#include "endpoint.grpc.pb.h"
#include "l2segment.grpc.pb.h"
#include "multicast.grpc.pb.h"
#include "rdma.grpc.pb.h"

/**
 * Forwarding Modes
 */
enum ForwardingMode
{
    FWD_MODE_CLASSIC,
    FWD_MODE_HOSTPIN,
};

class HalClient
{
public:
    // throws an exception if channel is not created
    static std::shared_ptr<HalClient> GetInstance();

    // call this first to set the mode and create the channel
    static std::shared_ptr<HalClient> GetInstance(enum ForwardingMode mode);

    enum ForwardingMode GetMode();

    // API stubs
    std::unique_ptr<vrf::Vrf::Stub> vrf_stub_;
    std::unique_ptr<intf::Interface::Stub> intf_stub_;
    std::unique_ptr<endpoint::Endpoint::Stub> ep_stub_;
    std::unique_ptr<l2segment::L2Segment::Stub> l2seg_stub_;
    std::unique_ptr<multicast::Multicast::Stub> multicast_stub_;
    std::unique_ptr<rdma::Rdma::Stub> rdma_stub_;

private:
    static std::shared_ptr<HalClient> instance;
    // Singleton
    HalClient();
    HalClient(enum ForwardingMode mode);
    // non-copyable
    HalClient(HalClient const&);
    void operator=(HalClient const&);

    enum ForwardingMode mode;

    std::shared_ptr<grpc::Channel> channel;
};

class HalObject
{
public:
    HalObject();
    static std::shared_ptr<HalClient> hal;

protected:
    // Hal objects are non-copyable
    HalObject(HalObject const &) = delete;
    void operator=(HalObject const &x) = delete;
};

// Forward Declrations of all HAL objects
class Lif;
class Enic;
class Endpoint;
class L2Segment;
class Vrf;
class Uplink;

#endif /* __HAL_HPP__ */
