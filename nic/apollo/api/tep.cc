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
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/tep.hpp"
#include "nic/apollo/api/utils.hpp"

using sdk::lib::ht;

namespace api {

typedef struct tep_update_ctxt_s {
    tep_entry *tep;
    api_obj_ctxt_t *obj_ctxt;
    uint64_t upd_bmap;
} __PACK__ tep_update_ctxt_t;

tep_entry::tep_entry() {
    fabric_encap_ = { PDS_ENCAP_TYPE_NONE, 0 };
    nh_type_ = PDS_NH_TYPE_NONE;
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

api_base *
tep_entry::clone(api_ctxt_t *api_ctxt) {
    tep_entry *cloned_tep;

    cloned_tep = tep_db()->alloc();
    if (cloned_tep) {
        new (cloned_tep) tep_entry();
        cloned_tep->impl_ = impl_->clone();
        if (unlikely(cloned_tep->impl_ == NULL)) {
            PDS_TRACE_ERR("Failed to clone tep %s impl", key_.str());
            goto error;
        }
        cloned_tep->init_config(api_ctxt);
    }
    return cloned_tep;

error:

    cloned_tep->~tep_entry();
    tep_db()->free(cloned_tep);
    return NULL;
}

sdk_ret_t
tep_entry::free(tep_entry *tep) {
    if (tep->impl_) {
        impl_base::free(impl::IMPL_OBJ_ID_TEP, tep->impl_);
    }
    tep->~tep_entry();
    tep_db()->free(tep);
    return SDK_RET_OK;
}

sdk_ret_t
tep_entry::reserve_resources(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->reserve_resources(this, orig_obj, obj_ctxt);
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
tep_entry::nuke_resources_(void) {
    if (impl_) {
        return impl_->nuke_resources(this);
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_entry::init_config(api_ctxt_t *api_ctxt) {
    pds_tep_spec_t *spec = &api_ctxt->api_params->tep_spec;

    PDS_TRACE_DEBUG("Initializing TEP %s, ip %s encap %s",
                    spec->key.str(), ipaddr2str(&spec->remote_ip),
                    pds_encap2str(&spec->encap));
    memcpy(&this->key_, &spec->key, sizeof(pds_obj_key_t));
    this->type_ = spec->type;
    this->remote_ip_ = spec->remote_ip;
    this->remote_svc_ = spec->remote_svc;
    if (is_mac_set(spec->mac)) {
        memcpy(mac_, spec->mac, ETH_ADDR_LEN);
    }
    fabric_encap_ = spec->encap;
    nh_type_ = spec->nh_type;
    if (nh_type_ == PDS_NH_TYPE_UNDERLAY) {
        nh_ = spec->nh;
    } else if (nh_type_ == PDS_NH_TYPE_UNDERLAY_ECMP) {
        nh_group_ = spec->nh_group;
    } else if (nh_type_ == PDS_NH_TYPE_OVERLAY) {
        tep_ = spec->tep;
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_entry::program_create(api_obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->program_hw(this, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_entry::cleanup_config(api_obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->cleanup_hw(this, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_entry::compute_update(api_obj_ctxt_t *obj_ctxt) {
    pds_tep_spec_t *spec = &obj_ctxt->api_params->tep_spec;

    obj_ctxt->upd_bmap = 0;
    if (type_ != spec->type) {
        PDS_TRACE_ERR("Attempt to modify immutable attr \"type\" from %u to %u "
                      "on tunnel %s", type_, spec->type, key_.str());
        return SDK_RET_INVALID_ARG;
    }

    if ((fabric_encap_.type != spec->encap.type) ||
        (fabric_encap_.val.value != spec->encap.val.value)) {
        PDS_TRACE_ERR("Attempt to modify immutable attr \"encap\" from "
                      "%s to %s on tunnel %s", pds_encap2str(&fabric_encap_),
                      pds_encap2str(&spec->encap), key_.str());
        return SDK_RET_INVALID_ARG;
    }

    if ((nh_type_ == PDS_NH_TYPE_OVERLAY) && (tep_ != spec->tep)) {
        obj_ctxt->upd_bmap = PDS_TEP_UPD_OVERLAY_NH;
    }
    return SDK_RET_OK;
}

static bool
tep_upd_walk_cb_ (void *api_obj, void *ctxt) {
    pds_obj_key_t tep_key;
    tep_entry *tep = (tep_entry *)api_obj;
    tep_update_ctxt_t *upd_ctxt = (tep_update_ctxt_t *)ctxt;

    tep = (tep_entry *)api_framework_obj((api_base *)api_obj);
    tep_key = upd_ctxt->tep->key();
    if ((tep->nh_type() == PDS_NH_TYPE_OVERLAY) && (tep->tep() == tep_key)) {
        api_obj_add_to_deps(upd_ctxt->obj_ctxt->api_op,
                            OBJ_ID_TEP, upd_ctxt->tep,
                            OBJ_ID_TEP, (api_base *)api_obj,
                            upd_ctxt->upd_bmap);
    }
    return false;
}

sdk_ret_t
tep_entry::add_deps(api_obj_ctxt_t *obj_ctxt) {
    tep_update_ctxt_t upd_ctxt = { 0 };

    if (obj_ctxt->upd_bmap & PDS_TEP_UPD_OVERLAY_NH) {
        upd_ctxt.tep = this;
        upd_ctxt.obj_ctxt = obj_ctxt;
        upd_ctxt.upd_bmap = PDS_TEP_UPD_OVERLAY_NH;
        return tep_db()->walk(tep_upd_walk_cb_, &upd_ctxt);
    }
    // in all other cases, it is sufficient to contain the update programming to
    // this TEP object alone
    return SDK_RET_OK;
}

sdk_ret_t
tep_entry::program_update(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->update_hw(orig_obj, this, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_entry::activate_config(pds_epoch_t epoch, api_op_t api_op,
                           api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->activate_hw(this, orig_obj, epoch, api_op, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_entry::reactivate_config(pds_epoch_t epoch, api_obj_ctxt_t *obj_ctxt) {
    // we hit this when a tunnel (T1) is pointing to a tunnel (T2), and
    // T2 is updated necessitating an update on T1
    if (impl_) {
        impl_->reactivate_hw(this, epoch, obj_ctxt);
    }
    return SDK_RET_OK;
}

void
tep_entry::fill_spec_(pds_tep_spec_t *spec) {
    memcpy(&spec->key, &key_, sizeof(pds_obj_key_t));
    spec->type = type_;
    spec->remote_ip = remote_ip_;
    spec->remote_svc = remote_svc_;
    if (is_mac_set(mac_)) {
        memcpy(spec->mac, mac_, ETH_ADDR_LEN);
    }
    spec->encap = fabric_encap_;
    spec->nh_type = nh_type_;
    if (nh_type_ == PDS_NH_TYPE_UNDERLAY) {
        spec->nh = nh_;
    } else if (nh_type_ == PDS_NH_TYPE_UNDERLAY_ECMP) {
        spec->nh_group = nh_group_;
    } else if (nh_type_ == PDS_NH_TYPE_OVERLAY) {
        spec->tep = tep_;
    }
}

sdk_ret_t
tep_entry::read(pds_tep_info_t *info) {
    fill_spec_(&info->spec);
    if (impl_) {
        return impl_->read_hw(this, (impl::obj_key_t *)(&info->spec.key),
                              (impl::obj_info_t *)info);
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_entry::add_to_db(void) {
    return tep_db()->insert(this);
}

sdk_ret_t
tep_entry::del_from_db(void) {
    if (tep_db()->remove(this)) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
tep_entry::update_db(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (tep_db()->remove((tep_entry *)orig_obj)) {
        return tep_db()->insert(this);
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
tep_entry::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_TEP, this);
}

/// \@}    // end of PDS_TEP_ENTRY

}    // namespace api
