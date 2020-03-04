// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#ifndef __HAL_ACL_PD_HPP__
#define __HAL_ACL_PD_HPP__

#include "nic/include/base.hpp"
#include "nic/hal/plugins/cfg/aclqos/acl.hpp"


using hal::pd::utils::acl_tcam_entry_handle_t;

namespace hal {
namespace pd {

typedef struct pd_acl_s {
    // Index into the stats table
    uint16_t stats_index;
    // ACL TCAM entry handle
    acl_tcam_entry_handle_t handle;
    // pi ptr
    acl_t                   *pi_acl;
} __PACK__ pd_acl_t;


// allocate a acl pd instance
static inline pd_acl_t *
acl_pd_alloc (void)
{
    pd_acl_t    *pd_acl;

    pd_acl = (pd_acl_t *)g_hal_state_pd->acl_pd_slab()->alloc();
    if (pd_acl == NULL) {
        return NULL;
    }

    return pd_acl;
}

// initialize a acl pd instance
static inline pd_acl_t *
acl_pd_init (pd_acl_t *pd_acl)
{
    if (!pd_acl) {
        return NULL;
    }
    pd_acl->pi_acl = NULL;
    pd_acl->handle = ACL_TCAM_ENTRY_INVALID_HANDLE;
    pd_acl->stats_index = USHRT_MAX;

    return pd_acl;
}

// allocate and initialize a acl pd instance
static inline pd_acl_t *
acl_pd_alloc_init (void)
{
    return acl_pd_init(acl_pd_alloc());
}

// freeing acl pd memory
static inline hal_ret_t
acl_pd_mem_free (pd_acl_t *pd_acl)
{
    if (pd_acl) {
        hal::pd::delay_delete_to_slab(HAL_SLAB_ACL_PD, pd_acl);
    }
    return HAL_RET_OK;
}

// freeing acl pd
static inline hal_ret_t
acl_pd_free (pd_acl_t **pd_acl_)
{
    pd_acl_t *pd_acl = *pd_acl_;
    acl_pd_mem_free(pd_acl);
    *pd_acl_ = NULL;
    return HAL_RET_OK;
}

}   // namespace pd
}   // namespace hal

#endif    // __HAL_ACL_PD_HPP__
