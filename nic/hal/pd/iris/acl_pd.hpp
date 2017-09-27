#ifndef __HAL_ACL_PD_HPP__
#define __HAL_ACL_PD_HPP__

#include "nic/include/base.h"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"


using hal::pd::utils::acl_tcam_entry_handle_t;

namespace hal {
namespace pd {

struct pd_acl_s {
    // ACL TCAM entry handle
    acl_tcam_entry_handle_t handle;
    uint32_t                dummy;

    // pi ptr
    void                    *pi_acl;
} __PACK__;


}   // namespace pd
}   // namespace hal

#endif    // __HAL_ACL_PD_HPP__
