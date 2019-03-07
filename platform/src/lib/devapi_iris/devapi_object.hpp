//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __DEVAPI_OBJECT_HPP__
#define __DEVAPI_OBJECT_HPP__

#include "hal_grpc.hpp"

namespace iris {

class devapi_object
{
public:
    devapi_object() {}
    static void populate_hal_grpc(void);

protected:
    static hal_grpc *hal;
};

}    // namespace iris

using iris::devapi_object;

#endif // __DEVAPI_OBJECT_HPP__
