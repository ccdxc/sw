#ifndef __HAL_ACL_PD_HPP__
#define __HAL_ACL_PD_HPP__

#include <base.h>
#include <pd.hpp>
#include <pd_api.hpp>


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


pd_acl_t *acl_pd_alloc ();
pd_acl_t *acl_pd_init (pd_acl_t *acl);
pd_acl_t *acl_pd_alloc_init ();
hal_ret_t acl_pd_free (pd_acl_t *acl);
hal_ret_t acl_pd_alloc_res(pd_acl_t *pd_acl);
void acl_pd_dealloc_res (pd_acl_t *pd_acl);
hal_ret_t acl_pd_program_hw(pd_acl_t *pd_acl);
void link_pi_pd(pd_acl_t *pd_acl, acl_t *pi_acl);
void unlink_pi_pd(pd_acl_t *pd_acl, acl_t *pi_acl);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_ACL_PD_HPP__
