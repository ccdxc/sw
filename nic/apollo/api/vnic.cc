/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    vnic.cc
 *
 * @brief   vnic entry handling
 */

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/vnic.hpp"
#include "nic/apollo/api/utils.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"

namespace api {

/**
 * @defgroup PDS_VNIC_ENTRY - vnic entry functionality
 * @ingroup PDS_VNIC
 * @{
 */

vnic_entry::vnic_entry() {
    //SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    ht_ctxt_.reset();
}

vnic_entry *
vnic_entry::factory(pds_vnic_spec_t *spec) {
    vnic_entry *vnic;

    /**< create vnic entry with defaults, if any */
    vnic = vnic_db()->vnic_alloc();
    if (vnic) {
        new (vnic) vnic_entry();
        vnic->impl_ = impl_base::factory(impl::IMPL_OBJ_ID_VNIC, spec);
        if (vnic->impl_ == NULL) {
            vnic_entry::destroy(vnic);
            return NULL;
        }
    }
    return vnic;
}

vnic_entry::~vnic_entry() {
    // TODO: fix me
    //SDK_SPINLOCK_DESTROY(&slock_);
}

void
vnic_entry::destroy(vnic_entry *vnic) {
    vnic->nuke_resources_();
    if (vnic->impl_) {
        impl_base::destroy(impl::IMPL_OBJ_ID_VNIC, vnic->impl_);
    }
    vnic->~vnic_entry();
    vnic_db()->vnic_free(vnic);
}

sdk_ret_t
vnic_entry::init_config(api_ctxt_t *api_ctxt) {
    pds_vnic_spec_t *spec = &api_ctxt->api_params->vnic_spec;

    memcpy(&this->key_, &spec->key, sizeof(pds_vnic_key_t));
    this->subnet_ =  spec->subnet;
    this->fabric_encap_ = spec->fabric_encap;
    return SDK_RET_OK;
}

sdk_ret_t
vnic_entry::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return impl_->reserve_resources(this, obj_ctxt);
}

sdk_ret_t
vnic_entry::program_config(obj_ctxt_t *obj_ctxt) {
    pds_vnic_spec_t    *spec = &obj_ctxt->api_params->vnic_spec;

    PDS_TRACE_DEBUG("Programming vnic %u, vcn %u, subnet %u, mac %s, vlan %u, "
                    "encap type %u, encap value %u", key_.id, spec->vcn.id,
                    spec->subnet.id, macaddr2str(spec->mac_addr),
                    spec->wire_vlan, spec->fabric_encap.type,
                    spec->fabric_encap.val.value);
    return impl_->program_hw(this, obj_ctxt);
}

sdk_ret_t
vnic_entry::release_resources(void) {
    return impl_->release_resources(this);
}

sdk_ret_t
vnic_entry::nuke_resources_(void) {
    return impl_->nuke_resources(this);
}

sdk_ret_t
vnic_entry::cleanup_config(obj_ctxt_t *obj_ctxt) {
    return impl_->cleanup_hw(this, obj_ctxt);
}

sdk_ret_t
vnic_entry::update_config(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    //return impl_->update_hw();
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
vnic_entry::activate_config(pds_epoch_t epoch, api_op_t api_op,
                           obj_ctxt_t *obj_ctxt) {
    PDS_TRACE_DEBUG("Activating vnic %u config", key_.id);
    return impl_->activate_hw(this, epoch, api_op, obj_ctxt);
}

sdk_ret_t
vnic_entry::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
vnic_entry::add_to_db(void) {
    return vnic_db()->vnic_ht()->insert_with_key(&key_, this,
                                                 &ht_ctxt_);
}

sdk_ret_t
vnic_entry::del_from_db(void) {
    vnic_db()->vnic_ht()->remove(&key_);
    return SDK_RET_OK;
}

sdk_ret_t
vnic_entry::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_VNIC, this);
}

/** @} */    // end of PDS_VNIC_ENTRY

}    // namespace api
