//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __NICMGR_DELPHIC_HPP__
#define __NICMGR_DELPHIC_HPP__

#include <grpc++/grpc++.h>
#include "nic/delphi/sdk/delphi_sdk.hpp"

#include "gen/proto/nicmgr/nicmgr.delphi.hpp"


namespace dobj = delphi::objects;

namespace hal {
namespace svc {

using grpc::Status;

class NicMgrDelphic{
private:
    delphi::SdkPtr                             sdk_;

public:
    NicMgrDelphic(delphi::SdkPtr sk) { sdk_ = sk; }
    delphi::SdkPtr sdk(void) const { return sdk_; }
};

extern std::shared_ptr<NicMgrDelphic> g_nicmgr_delphic;

// Get Delphic Sdk
delphi::SdkPtr nicmgr_delphic_sdk(void);

// initialize Delphi client service
Status nicmgr_delphic_init(delphi::SdkPtr sdk);

}    // namespace svc
}    // namespace hal

#endif    // __NICMGR_DELPHIC_HPP__
