//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// policer handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/policer.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

/// \defgroup PDS_POLICER - policer functionality
/// \ingroup PDS_POLICER
/// @{

policer::policer() {
    ht_ctxt_.reset();
    impl_ = NULL;
}

policer *
policer::factory(pds_policer_spec_t *spec) {
    policer *pol;

    ///< create policer with defaults, if any
    pol = policer_db()->alloc();
    if (pol) {
        new (pol) policer();
        pol->impl_ = impl_base::factory(impl::IMPL_OBJ_ID_POLICER, spec);
        if (pol->impl_ == NULL) {
            policer::destroy(pol);
            return NULL;
        }
    }
    return pol;
}

policer::~policer() {
}

void
policer::destroy(policer *pol) {
    pol->nuke_resources_();
    if (pol->impl_) {
        impl_base::destroy(impl::IMPL_OBJ_ID_POLICER, pol->impl_);
    }
    pol->~policer();
    policer_db()->free(pol);
}

sdk_ret_t
policer::init_config(api_ctxt_t *api_ctxt) {
    pds_policer_spec_t *spec = &api_ctxt->api_params->policer_spec;

    memcpy(&this->key_, &spec->key, sizeof(key_));
    dir_ = spec->dir;
    return SDK_RET_OK;
}

sdk_ret_t
policer::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return impl_->reserve_resources(this, obj_ctxt);
}

sdk_ret_t
policer::program_config(obj_ctxt_t *obj_ctxt) {
    return impl_->program_hw(this, obj_ctxt);
}

sdk_ret_t
policer::reprogram_config(api_op_t api_op) {
    return SDK_RET_ERR;
}

sdk_ret_t
policer::release_resources(void) {
    return impl_->release_resources(this);
}

sdk_ret_t
policer::nuke_resources_(void) {
    return impl_->nuke_resources(this);
}

sdk_ret_t
policer::cleanup_config(obj_ctxt_t *obj_ctxt) {
    return impl_->cleanup_hw(this, obj_ctxt);
}

sdk_ret_t
policer::update_config(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    //return impl_->update_hw();
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
policer::activate_config(pds_epoch_t epoch, api_op_t api_op,
                         obj_ctxt_t *obj_ctxt) {
    return impl_->activate_hw(this, epoch, api_op, obj_ctxt);
}

sdk_ret_t
policer::reactivate_config(pds_epoch_t epoch, api_op_t api_op) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
policer::read(pds_policer_key_t *key, pds_policer_info_t *info) {
    return impl_->read_hw(this, (impl::obj_key_t *)key,
                          (impl::obj_info_t *)info);
}

sdk_ret_t
policer::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
policer::add_to_db(void) {
    return policer_db()->insert(this);
}

sdk_ret_t
policer::del_from_db(void) {
    if (policer_db()->remove(this)) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
policer::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_POLICER, this);
}

/// @}     // end of PDS_POLICER

}    // namespace api
