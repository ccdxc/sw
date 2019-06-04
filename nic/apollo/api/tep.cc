//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// TEP entry handling
///
//----------------------------------------------------------------------------

#include <stdio.h>
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/tep.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"

using sdk::lib::ht;

namespace api {

/// \defgroup PDS_TEP_ENTRY - tep entry functionality
/// \ingroup PDS_TEP
/// \@{

tep_entry::tep_entry() {
    //SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    ht_ctxt_.reset();
    MAC_UINT64_TO_ADDR(mac_, PDS_REMOTE_TEP_MAC);
}

tep_entry *
tep_entry::factory(pds_tep_spec_t *spec) {
    tep_entry *tep;

    // create tep entry with defaults, if any
    tep = tep_db()->alloc();
    if (tep) {
        new (tep) tep_entry();
        tep->impl_ = impl_base::factory(impl::IMPL_OBJ_ID_TEP, spec);
    }
    return tep;
}

tep_entry::~tep_entry() {
    // TODO: fix me
    //SDK_SPINLOCK_DESTROY(&slock_);
}

void
tep_entry::destroy(tep_entry *tep) {
    tep->nuke_resources_();
    if (tep->impl_) {
        impl_base::destroy(impl::IMPL_OBJ_ID_TEP, tep->impl_);
    }
    tep->~tep_entry();
    tep_db()->free(tep);
}

sdk_ret_t
tep_entry::init_config(api_ctxt_t *api_ctxt) {
    pds_tep_spec_t *spec = &api_ctxt->api_params->tep_spec;

    PDS_TRACE_DEBUG("Initializing TEP %s", ipaddr2str(&spec->key.ip_addr));
    memcpy(&this->key_, &spec->key, sizeof(pds_tep_key_t));
    if (is_mac_set(spec->mac)) {
        memcpy(mac_, spec->mac, sizeof(ETH_ADDR_LEN));
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_entry::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->reserve_resources(this, obj_ctxt);
    }
    return SDK_RET_OK;
}

// TODO: we should simply be generating ARP request in the substrate in this API
//       and come back and do this h/w programming later, but until that control
//       plane & PMD APIs are ready, we will directly write to hw with fixed MAC
sdk_ret_t
tep_entry::program_config(obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->program_hw(this, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_entry::nuke_resources_(void) {
    if (impl_) {
        return impl_->nuke_resources(this);
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_entry::release_resources(void) {
    if (impl_) {
        return impl_->release_resources(this);
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_entry::cleanup_config(obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->cleanup_hw(this, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_entry::update_config(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    // impl->update_hw();
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
tep_entry::activate_config(pds_epoch_t epoch, api_op_t api_op,
                           obj_ctxt_t *obj_ctxt) {
    switch (api_op) {
    case API_OP_CREATE:
        PDS_TRACE_DEBUG("Created TEP %s", ipaddr2str(&key_.ip_addr));
        break;

    case API_OP_DELETE:
        PDS_TRACE_DEBUG("Deleted TEP %s", ipaddr2str(&key_.ip_addr));
        break;

    case API_OP_UPDATE:
        PDS_TRACE_DEBUG("Updated TEP %s", ipaddr2str(&key_.ip_addr));
        break;

    case API_OP_NONE:
    default:
        PDS_TRACE_DEBUG("Invalid op %u for TEP %s", api_op,
                        ipaddr2str(&key_.ip_addr));
        return sdk::SDK_RET_INVALID_OP;
    }
    return sdk::SDK_RET_OK;
}

sdk_ret_t
tep_entry::read(pds_tep_key_t *key, pds_tep_info_t *info) {
    return impl_->read_hw(this, (impl::obj_key_t *)key,
                          (impl::obj_info_t *)info);
}

sdk_ret_t
tep_entry::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
tep_entry::add_to_db(void) {
    PDS_TRACE_VERBOSE("Adding TEP %s to db", ipaddr2str(&key_.ip_addr));
    return tep_db()->insert(this);
}

sdk_ret_t
tep_entry::del_from_db(void) {
    PDS_TRACE_VERBOSE("Deleting TEP %s from db", ipaddr2str(&key_.ip_addr));
    if (tep_db()->remove(this)) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
tep_entry::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_TEP, this);
}

/// \@}    // end of PDS_TEP_ENTRY

}    // namespace api
