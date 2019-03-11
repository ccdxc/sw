//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// vcn entry handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/vcn.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

vcn_entry::vcn_entry() {
    // SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    ht_ctxt_.reset();
    hw_id_ = 0xFFFF;
}

vcn_entry *
vcn_entry::factory(pds_vcn_spec_t *spec) {
    vcn_entry *vcn;

    // create vcn entry with defaults, if any
    vcn = vcn_db()->alloc();
    if (vcn) {
        new (vcn) vcn_entry();
    }
    return vcn;
}

vcn_entry::~vcn_entry() {
    // SDK_SPINLOCK_DESTROY(&slock_);
}

void
vcn_entry::destroy(vcn_entry *vcn) {
    vcn->release_resources();
    vcn->~vcn_entry();
    vcn_db()->free(vcn);
}

sdk_ret_t
vcn_entry::init_config(api_ctxt_t *api_ctxt) {
    pds_vcn_spec_t *spec = &api_ctxt->api_params->vcn_spec;

    PDS_TRACE_DEBUG("Intializing vcn %u, type %u, pfx %s", spec->key.id,
                    spec->type, ippfx2str(&spec->pfx));
    memcpy(&this->key_, &spec->key, sizeof(pds_vcn_key_t));
    return SDK_RET_OK;
}

sdk_ret_t
vcn_entry::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    if (vcn_db()->vcn_idxr()->alloc((uint32_t *)&this->hw_id_) !=
        sdk::lib::indexer::SUCCESS) {
        return sdk::SDK_RET_NO_RESOURCE;
    }
    return SDK_RET_OK;
}

sdk_ret_t
vcn_entry::release_resources(void) {
    if (hw_id_ != 0xFF) {
        vcn_db()->vcn_idxr()->free(hw_id_);
    }
    return SDK_RET_OK;
}

sdk_ret_t
vcn_entry::update_config(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
vcn_entry::activate_config(pds_epoch_t epoch, api_op_t api_op,
                           obj_ctxt_t *obj_ctxt) {
    // there is no h/w programming for vcn config, so nothing to activate
    PDS_TRACE_DEBUG("Activated vcn api op %u, vcn %u", api_op, key_.id);
    return SDK_RET_OK;
}

sdk_ret_t
vcn_entry::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    // nothing to update on vcn, so no updates supported
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
vcn_entry::add_to_db(void) {
    PDS_TRACE_VERBOSE("Adding vcn %u to db", key_.id);
    return vcn_db()->insert(this);
}

sdk_ret_t
vcn_entry::del_from_db(void) {
    PDS_TRACE_VERBOSE("Deleting vcn %u from db", key_.id);
    if (vcn_db()->remove(this)) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
vcn_entry::delay_delete(void) {
    PDS_TRACE_VERBOSE("Delay delete vcn %u", key_.id);
    return delay_delete_to_slab(PDS_SLAB_ID_VCN, this);
}

}    // namespace api
