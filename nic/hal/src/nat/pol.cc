//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// Handles Config APIs for NAT Policy
//-----------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/hal_state.hpp"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/gen/hal/include/hal_api_stats.hpp"
#include "nic/hal/src/nat/nat.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/fte/acl/acl.hpp"
#include "nat.hpp"

using sdk::lib::ht_ctxt_t;
using acl::acl_ctx_t;

namespace hal {

static inline nat_cfg_pol_t *
nat_cfg_pol_alloc (void)
{
    return ((nat_cfg_pol_t *)g_hal_state->nat_cfg_pol_slab()->alloc());
}

static inline void
nat_cfg_pol_free (nat_cfg_pol_t *pol)
{
    hal::delay_delete_to_slab(HAL_SLAB_NAT_CFG_POL, pol);
}

static inline void
nat_cfg_pol_init (nat_cfg_pol_t *pol)
{
    HAL_SPINLOCK_INIT(&pol->slock, PTHREAD_PROCESS_SHARED);
    dllist_reset(&pol->rule_list);
    pol->hdl = HAL_HANDLE_INVALID;
}

static inline void
nat_cfg_pol_uninit (nat_cfg_pol_t *pol)
{
    HAL_SPINLOCK_DESTROY(&pol->slock);
}

static inline nat_cfg_pol_t *
nat_cfg_pol_alloc_init (void)
{
    nat_cfg_pol_t *pol;

    if ((pol = nat_cfg_pol_alloc()) ==  NULL)
        return NULL;

    nat_cfg_pol_init(pol);
    return pol;
}

static inline void
nat_cfg_pol_uninit_free (nat_cfg_pol_t *pol)
{
    if (pol) {
        nat_cfg_pol_uninit(pol);
        nat_cfg_pol_free(pol);
    }
}

static inline void
nat_cfg_pol_db_add (dllist_ctxt_t *head, nat_cfg_pol_t *pol)
{
    dllist_add_tail(head, &pol->list_ctxt);
}

static inline void
nat_cfg_pol_db_del (nat_cfg_pol_t *pol)
{
    dllist_del(&pol->list_ctxt);
}

static hal_ret_t
nat_cfg_pol_rule_spec_extract (nat::NatPolicySpec& spec, nat_cfg_pol_t *pol)
{
    hal_ret_t ret = HAL_RET_OK;

    for (int i = 0; i < spec.rules_size(); i++) {
        if ((ret = nat_cfg_rule_spec_handle(
               spec.rules(i), &pol->list_ctxt)) != HAL_RET_OK)
            return ret;
    }

    return ret;
}

hal_ret_t
nat_cfg_pol_data_spec_extract (nat::NatPolicySpec& spec, nat_cfg_pol_t *pol)
{
    hal_ret_t ret = HAL_RET_OK;

    if ((ret = nat_cfg_pol_rule_spec_extract(spec, pol)) != HAL_RET_OK)
        return ret;

    return ret;
}

static hal_ret_t
nat_cfg_pol_key_spec_extract (nat::NatPolicySpec& spec, nat_cfg_pol_key_t *key)
{
    key->pol_id = spec.key_or_handle().policy_key().nat_policy_id();

    if (spec.key_or_handle().policy_key().
        vrf_key_or_handle().key_or_handle_case() == kh::VrfKeyHandle::kVrfId) {
        key->vrf_id = spec.key_or_handle().policy_key().
            vrf_key_or_handle().vrf_id();
    } else {
        vrf_t *vrf = vrf_lookup_by_handle(
            spec.key_or_handle().policy_key().
            vrf_key_or_handle().vrf_handle());
        if (!vrf)
            return HAL_RET_HANDLE_INVALID;
        key->vrf_id = vrf->vrf_id;
    }
    return HAL_RET_OK;
}

static hal_ret_t
nat_cfg_pol_spec_extract (nat::NatPolicySpec& spec, nat_cfg_pol_t *pol)
{
    hal_ret_t ret;

    if ((ret = nat_cfg_pol_key_spec_extract(spec, &pol->key)) != HAL_RET_OK)
        return ret;

    if ((ret = nat_cfg_pol_data_spec_extract(spec, pol)) != HAL_RET_OK)
        return ret;

    return ret;
}

static hal_ret_t
nat_cfg_pol_spec_validate (nat::NatPolicySpec& spec)
{
    return HAL_RET_OK;
}

hal_ret_t
nat_cfg_pol_create_cfg_handle (nat::NatPolicySpec& spec,
                               nat::NatPolicyResponse *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    nat_cfg_pol_t *pol;

    if ((ret = nat_cfg_pol_spec_validate(spec)) != HAL_RET_OK)
        return ret;

    if ((pol = nat_cfg_pol_alloc_init()) == NULL)
        return HAL_RET_OOM;

    if ((ret = nat_cfg_pol_spec_extract(spec, pol)) != HAL_RET_OK)
        goto end;

end:
    return ret;
}

hal_ret_t
nat_cfg_pol_rsp_build (nat::NatPolicyResponse *rsp, hal_ret_t ret,
                       hal_handle_t hal_handle)
{
    if (ret == HAL_RET_OK)
        rsp->mutable_policy_status()->set_nat_policy_handle(hal_handle);
    rsp->set_api_status(hal_prepare_rsp(ret));
    return HAL_RET_OK;
}

}  // namespace hal
