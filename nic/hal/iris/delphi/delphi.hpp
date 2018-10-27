// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __HAL_IRIS_DELPHI_HPP__
#define __HAL_IRIS_DEPLHI_HPP__

#include <memory>

#include "delphic.hpp"

namespace hal {
namespace svc {

extern std::shared_ptr<delphi_client> delphic;
   
void *delphi_client_start(void *ctxt);

}    // namespace svc 
}    // namespace hal

#endif    // __HAL_IRIS_DEPLHI_HPP__
