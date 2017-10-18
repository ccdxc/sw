#ifndef __DESCRIPTOR_AOL_HPP__
#define __DESCRIPTOR_AOL_HPP__


#include "nic/include/base.h"
#include "nic/include/pd.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/gen/proto/hal/descriptor_aol.pb.h"

using descraol::DescrAolSpec;
using descraol::DescrAolRequest;
using descraol::DescrAolRequestMsg;
using descraol::DescrAolResponseMsg;

namespace hal {
hal_ret_t descriptor_aol_get(descraol::DescrAolRequest& request, 
                             descraol::DescrAolSpec *response);
}
#endif /* __DESCRIPTOR_AOL_HPP__ */
