//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// vnic entry handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/vnic.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/encap_utils.hpp"

namespace api {

/// \defgroup PDS_VNIC_ENTRY - vnic functionality
/// \ingroup PDS_VNIC
/// @{

vnic_entry::vnic_entry() {
    //SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    switch_vnic_ = false;
    host_ifindex_ = IFINDEX_INVALID;
    ht_ctxt_.reset();
}

vnic_entry *
vnic_entry::factory(pds_vnic_spec_t *spec) {
    vnic_entry *vnic;

    /**< create vnic entry with defaults, if any */
    vnic = vnic_db()->alloc();
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
    vnic_db()->free(vnic);
}

sdk_ret_t
vnic_entry::init_config(api_ctxt_t *api_ctxt) {
    pds_vnic_spec_t *spec = &api_ctxt->api_params->vnic_spec;

    memcpy(&this->key_, &spec->key, sizeof(pds_vnic_key_t));
    this->subnet_ = spec->subnet;
    this->v4_meter_ = spec->v4_meter;
    this->v6_meter_ = spec->v6_meter;
    this->vnic_encap_ = spec->vnic_encap;
    this->fabric_encap_ = spec->fabric_encap;
    this->switch_vnic_ = spec->switch_vnic;
    if (is_mac_set(spec->mac_addr)) {
        memcpy(mac_, spec->mac_addr, ETH_ADDR_LEN);
    }
    this->host_ifindex_ = spec->host_ifindex;
    return SDK_RET_OK;
}

sdk_ret_t
vnic_entry::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return impl_->reserve_resources(this, obj_ctxt);
}

sdk_ret_t
vnic_entry::program_config(obj_ctxt_t *obj_ctxt) {
    pds_vnic_spec_t *spec = &obj_ctxt->api_params->vnic_spec;

    PDS_TRACE_DEBUG("Programming vnic %u, vpc %u, subnet %u, v4 meter id %u, "
                    "v6 meter id %u, mac %s, vnic encap %s, fabric encap %s, "
                    "rxmirror bitmap %x, tx mirror bitmap %x, switch vnic %u",
                    key_.id, spec->vpc.id, spec->subnet.id,
                    spec->v4_meter.id, spec->v6_meter.id,
                    macaddr2str(spec->mac_addr),
                    pds_encap2str(spec->vnic_encap),
                    pds_encap2str(spec->fabric_encap),
                    spec->rx_mirror_session_bmap,
                    spec->tx_mirror_session_bmap,
                    spec->switch_vnic);
    return impl_->program_hw(this, obj_ctxt);
}

sdk_ret_t
vnic_entry::reprogram_config(api_op_t api_op) {
    PDS_TRACE_DEBUG("Reprogramming vnic %u, subnet %u, fabric encap %s, ",
                    key_.id, subnet_.id, pds_encap2str(fabric_encap_));
    return impl_->reprogram_hw(this, api_op);
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
vnic_entry::reactivate_config(pds_epoch_t epoch, api_op_t api_op) {
    PDS_TRACE_DEBUG("Reactivating vnic %u, subnet %u, fabric encap %s, ",
                    key_.id, subnet_.id, pds_encap2str(fabric_encap_));
    return impl_->reactivate_hw(this, epoch, api_op);
}

sdk_ret_t
vnic_entry::fill_spec_(pds_vnic_spec_t *spec) {
    subnet_entry *subnet;

    subnet = subnet_db()->find(&subnet_);
    if (subnet == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    memcpy(&spec->key, &this->key_, sizeof(pds_vnic_key_t));
    spec->subnet = subnet_;
    spec->vpc = subnet->vpc();
    memcpy(&spec->mac_addr, &mac_, ETH_ADDR_LEN);
    spec->fabric_encap = fabric_encap_;
    spec->vnic_encap = vnic_encap_;
    spec->v4_meter = v4_meter_;
    spec->v6_meter = v6_meter_;
    spec->switch_vnic = switch_vnic_;
    spec->host_ifindex = host_ifindex_;

    return sdk::SDK_RET_OK;
}

sdk_ret_t
vnic_entry::read(pds_vnic_info_t *info) {
    sdk_ret_t ret;

    ret = fill_spec_(&info->spec);
    if (ret != sdk::SDK_RET_OK) {
        return ret;
    }
    return impl_->read_hw(this, (impl::obj_key_t *)(&info->spec.key),
                          (impl::obj_info_t *)info);
}

sdk_ret_t
vnic_entry::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
vnic_entry::add_to_db(void) {
    PDS_TRACE_VERBOSE("Adding vnic %u to db", key_.id);
    return vnic_db()->insert(this);
}

sdk_ret_t
vnic_entry::del_from_db(void) {
    if (vnic_db()->remove(this)) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
vnic_entry::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_VNIC, this);
}

/// @}     // end of PDS_VNIC_ENTRY

}    // namespace api
