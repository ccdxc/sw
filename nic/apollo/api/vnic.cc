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
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/vnic.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

/// \defgroup PDS_VNIC_ENTRY - vnic functionality
/// \ingroup PDS_VNIC
/// @{

vnic_entry::vnic_entry() {
    switch_vnic_ = false;
    host_ifindex_ = IFINDEX_INVALID;
    ht_ctxt_.reset();
}

vnic_entry *
vnic_entry::factory(pds_vnic_spec_t *spec) {
    vnic_entry *vnic;

    // create vnic entry with defaults, if any
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

api_base *
vnic_entry::clone(api_ctxt_t *api_ctxt) {
    vnic_entry *cloned_vnic;

    cloned_vnic = vnic_db()->alloc();
    if (cloned_vnic) {
        new (cloned_vnic) vnic_entry();
        cloned_vnic->impl_ = impl_->clone();
        if (unlikely(cloned_vnic->impl_ == NULL)) {
            PDS_TRACE_ERR("Failed to clone vnic %u impl", key_.id);
            goto error;
        }
        cloned_vnic->init_config(api_ctxt);
    }
    return cloned_vnic;

error:

    cloned_vnic->~vnic_entry();
    vnic_db()->free(cloned_vnic);
    return NULL;
}

sdk_ret_t
vnic_entry::free(vnic_entry *vnic) {
    if (vnic->impl_) {
        impl_base::free(impl::IMPL_OBJ_ID_VNIC, vnic->impl_);
    }
    vnic->~vnic_entry();
    vnic_db()->free(vnic);
    return SDK_RET_OK;
}

sdk_ret_t
vnic_entry::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return impl_->reserve_resources(this, obj_ctxt);
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
vnic_entry::init_config(api_ctxt_t *api_ctxt) {
    pds_vnic_spec_t *spec = &api_ctxt->api_params->vnic_spec;

    memcpy(&key_, &spec->key, sizeof(pds_vnic_key_t));
    subnet_ = spec->subnet;
    v4_meter_ = spec->v4_meter;
    v6_meter_ = spec->v6_meter;
    vnic_encap_ = spec->vnic_encap;
    fabric_encap_ = spec->fabric_encap;
    switch_vnic_ = spec->switch_vnic;
    num_ing_v4_policy_ = spec->num_ing_v4_policy;
    for (uint8_t i = 0; i < num_ing_v4_policy_; i++) {
        ing_v4_policy_[i].id = spec->ing_v4_policy[i].id;
    }
    num_ing_v6_policy_ = spec->num_ing_v6_policy;
    for (uint8_t i = 0; i < num_ing_v6_policy_; i++) {
        ing_v6_policy_[i].id = spec->ing_v6_policy[i].id;
    }
    num_egr_v4_policy_ = spec->num_egr_v4_policy;
    for (uint8_t i = 0; i < num_egr_v4_policy_; i++) {
        egr_v4_policy_[i].id = spec->egr_v4_policy[i].id;
    }
    num_egr_v6_policy_ = spec->num_egr_v6_policy;
    for (uint8_t i = 0; i < num_egr_v6_policy_; i++) {
        egr_v6_policy_[i].id = spec->egr_v6_policy[i].id;
    }
    if (is_mac_set(spec->mac_addr)) {
        memcpy(mac_, spec->mac_addr, ETH_ADDR_LEN);
    }
    host_ifindex_ = spec->host_ifindex;
    return SDK_RET_OK;
}

sdk_ret_t
vnic_entry::program_create(obj_ctxt_t *obj_ctxt) {
    pds_vnic_spec_t *spec = &obj_ctxt->api_params->vnic_spec;

    PDS_TRACE_DEBUG("Programming vnic %u, vpc %u, subnet %u, v4 meter id %u, "
                    "v6 meter id %u, mac %s\nvnic encap %s, fabric encap %s, "
                    "rxmirror bitmap %x, tx mirror bitmap %x, switch vnic %u, "
                    "host if 0x%x",
                    key_.id, spec->vpc.id, spec->subnet.id,
                    spec->v4_meter.id, spec->v6_meter.id,
                    macaddr2str(spec->mac_addr),
                    pds_encap2str(&spec->vnic_encap),
                    pds_encap2str(&spec->fabric_encap),
                    spec->rx_mirror_session_bmap,
                    spec->tx_mirror_session_bmap,
                    spec->switch_vnic, spec->host_ifindex);
    return impl_->program_hw(this, obj_ctxt);
}

sdk_ret_t
vnic_entry::cleanup_config(obj_ctxt_t *obj_ctxt) {
    return impl_->cleanup_hw(this, obj_ctxt);
}

sdk_ret_t
vnic_entry::compute_update(obj_ctxt_t *obj_ctxt) {
    pds_vnic_spec_t *spec = &obj_ctxt->api_params->vnic_spec;

    if (subnet_.id != spec->subnet.id) {
        PDS_TRACE_ERR("Attempt to modify immutable attr \"subnet\" "
                      "from %u to %u on vnic %s", subnet_.id,
                      spec->subnet.id, key2str());
    }
    if ((num_ing_v4_policy_ != spec->num_ing_v4_policy)          ||
        (num_ing_v6_policy_ != spec->num_ing_v6_policy)          ||
        (num_egr_v4_policy_ != spec->num_egr_v4_policy)          ||
        (num_egr_v6_policy_ != spec->num_egr_v6_policy)          ||
        (memcmp(ing_v4_policy_, spec->ing_v4_policy,
                num_ing_v4_policy_ * sizeof(ing_v4_policy_[0]))) ||
        (memcmp(ing_v6_policy_, spec->ing_v6_policy,
                num_ing_v6_policy_ * sizeof(ing_v6_policy_[0]))) ||
        (memcmp(egr_v4_policy_, spec->egr_v4_policy,
                num_egr_v4_policy_ * sizeof(egr_v4_policy_[0]))) ||
        (memcmp(egr_v6_policy_, spec->egr_v6_policy,
                   num_egr_v6_policy_ * sizeof(egr_v6_policy_[0])))) {
        obj_ctxt->upd_bmap |= PDS_VNIC_UPD_POLICY;
    }
    if (host_ifindex_ != spec->host_ifindex) {
        obj_ctxt->upd_bmap |= PDS_VNIC_UPD_HOST_IFINDEX;
    }
    PDS_TRACE_DEBUG("subnet %u upd bmap 0x%lx", obj_ctxt->upd_bmap);
    return SDK_RET_OK;
}

sdk_ret_t
vnic_entry::reprogram_config(api_op_t api_op) {
    PDS_TRACE_DEBUG("Reprogramming vnic %u, subnet %u, fabric encap %s, ",
                    key_.id, subnet_.id, pds_encap2str(&fabric_encap_));
    return impl_->reprogram_hw(this, api_op);
}

sdk_ret_t
vnic_entry::program_update(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    //return impl_->update_hw();
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
vnic_entry::activate_config(pds_epoch_t epoch, api_op_t api_op,
                            api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    PDS_TRACE_DEBUG("Activating vnic %u config", key_.id);
    return impl_->activate_hw(this, orig_obj, epoch, api_op, obj_ctxt);
}

sdk_ret_t
vnic_entry::reactivate_config(pds_epoch_t epoch, api_op_t api_op) {
    PDS_TRACE_DEBUG("Reactivating vnic %u, subnet %u, fabric encap %s, ",
                    key_.id, subnet_.id, pds_encap2str(&fabric_encap_));
    return impl_->reactivate_hw(this, epoch, api_op);
}

sdk_ret_t
vnic_entry::fill_spec_(pds_vnic_spec_t *spec) {
    subnet_entry *subnet;

    subnet = subnet_db()->find(&subnet_);
    if (subnet == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    memcpy(&spec->key, &key_, sizeof(pds_vnic_key_t));
    spec->subnet = subnet_;
    spec->vpc = subnet->vpc();
    memcpy(&spec->mac_addr, &mac_, ETH_ADDR_LEN);
    spec->fabric_encap = fabric_encap_;
    spec->vnic_encap = vnic_encap_;
    spec->v4_meter = v4_meter_;
    spec->v6_meter = v6_meter_;
    spec->switch_vnic = switch_vnic_;
    spec->host_ifindex = host_ifindex_;
    return SDK_RET_OK;
}

sdk_ret_t
vnic_entry::read(pds_vnic_info_t *info) {
    sdk_ret_t ret;

    ret = fill_spec_(&info->spec);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    return impl_->read_hw(this, (impl::obj_key_t *)(&info->spec.key),
                          (impl::obj_info_t *)info);
}

sdk_ret_t
vnic_entry::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return SDK_RET_INVALID_OP;
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
