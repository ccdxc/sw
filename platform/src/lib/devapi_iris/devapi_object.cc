//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "devapi_object.hpp"

namespace iris {

hal_grpc *devapi_object::hal = NULL;

void
devapi_object::populate_hal_grpc(void)
{
    hal = hal_grpc::get_hal_grpc();
}

}     // namespace iris
