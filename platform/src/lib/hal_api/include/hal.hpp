
#ifndef __HAL_API_HAL_HPP__
#define __HAL_API_HAL_HPP__

#include "hal_common_client.hpp"
#if 0
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
enum HalForwardingMode
{
    FWD_MODE_CLASSIC,
    FWD_MODE_HOSTPIN,
};

class HalCommonClient
{
public:
    // throws an exception if channel is not created
    static std::shared_ptr<HalCommonClient> GetInstance();

    // call this first to set the mode and create the channel
    static std::shared_ptr<HalCommonClient> GetInstance(enum HalForwardingMode mode);

    enum HalForwardingMode GetMode();

    // API stubs
    std::unique_ptr<vrf::Vrf::Stub> vrf_stub_;
    std::unique_ptr<intf::Interface::Stub> intf_stub_;
    std::unique_ptr<endpoint::HalEndpoint::Stub> ep_stub_;
    std::unique_ptr<l2segment::HalL2Segment::Stub> l2seg_stub_;
    std::unique_ptr<multicast::HalMulticast::Stub> multicast_stub_;
    std::unique_ptr<rdma::Rdma::Stub> rdma_stub_;

private:
    static std::shared_ptr<HalCommonClient> instance;
    // Singleton
    HalCommonClient();
    HalCommonClient(enum HalForwardingMode mode);
    // non-copyable
    HalCommonClient(HalCommonClient const&);
    void operator=(HalCommonClient const&);

    enum HalForwardingMode mode;

    std::shared_ptr<grpc::Channel> channel;
};
#endif
class HalObject
{
public:
    HalObject();
    // TODO: Cleanup. Why should this be static if its being assigned
    //       in the constructor, thereby assigning it for every hal object
    //       created (uplink, lif, etc);
    static HalCommonClient *hal;

protected:
    // Hal objects are non-copyable
    HalObject(HalObject const &) = delete;
    void operator=(HalObject const &x) = delete;
};

// Forward Declrations of all HAL objects
class Lif;
class EthLif;
class Enic;
class HalEndpoint;
class HalL2Segment;
class HalVrf;
class Uplink;

//-----------------------------------------------------------------------------
 // X-Macro for defining enums (generates enum definition and string formatter)
 //
 // Example:
 //
 // #define SAMPLE_ENUM_ENTRIES(ENTRY)
 //    ENTRY(OK, 0, "ok")
 //    ENTRY(ERR, 1, "err")
 //
 // DEFINE_ENUM(sample_enum_t, SAMPLE_ENUM_ENTRIES)
 //------------------------------------------------------------------------------

 #define _ENUM_FIELD(_name, _val, _desc) _name = _val,
 #define _ENUM_STR(_name, _val, _desc) \
     inline const char *_name ## _str() {return _desc; }
 #define _ENUM_CASE(_name, _val, _desc) case _name: return os << #_desc;

 #define DEFINE_ENUM(_typ, _entries)                                     \
     typedef enum { _entries(_ENUM_FIELD) } _typ;                        \
     inline std::ostream& operator<<(std::ostream& os, _typ c)           \
     {                                                                   \
         switch (c) {                                                    \
             _entries(_ENUM_CASE);                                       \
         }                                                               \
         return os;                                                      \
     }                                                                   \
     _entries(_ENUM_STR)                                                 \

#endif /* __HAL_API_HAL_HPP__ */
