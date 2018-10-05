//------------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/hal/iris/delphi/delphi.hpp"
#include "nic/hal/iris/delphi/delphi_client.hpp"

namespace hal {
namespace delphi {

void *
delphi_client_start (void *ctxt)
{
    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());
    shared_ptr<delphi_client> delphic = make_shared<delphi_client>(sdk);
    HAL_ASSERT(delphic != NULL);

    // register delphi client
    sdk->RegisterService(delphi_client);

    // sit in main loop
    sdk->MainLoop();

    // shouldn't hit
    return NULL;
}

}    // namespace delphi
}    // namespace hal
