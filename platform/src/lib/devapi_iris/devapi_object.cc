
#include <string>
#include <memory>
#include <iostream>
#include "devapi_object.hpp"

namespace iris {

// hal grpc
hal_grpc *devapi_object::hal = NULL;

void
devapi_object::populate_hal_grpc()
{
    hal = hal_grpc::get_hal_grpc();
}

} // namespace iris
