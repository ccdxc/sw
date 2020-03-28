//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// subnet entry handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/msg.h"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/core/msg.h"
#include "nic/apollo/api/subnet.hpp"
#include "nic/apollo/api/vnic.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

typedef struct subnet_upd_ctxt_s {
    subnet_entry *subnet;
    api_obj_ctxt_t *obj_ctxt;
    uint64_t upd_bmap;
} __PACK__ subnet_upd_ctxt_t;

subnet_entry::subnet_entry() {
    v4_route_table_.reset();
    v6_route_table_.reset();
    num_ing_v4_policy_ = 0;
    num_ing_v6_policy_ = 0;
    num_egr_v4_policy_ = 0;
    num_egr_v6_policy_ = 0;
    memset(&ing_v4_policy_, 0, sizeof ing_v4_policy_);
    memset(&ing_v6_policy_, 0, sizeof ing_v6_policy_);
    memset(&egr_v4_policy_, 0, sizeof egr_v4_policy_);
    memset(&egr_v6_policy_, 0, sizeof egr_v6_policy_);
    ht_ctxt_.reset();
    hw_id_ = 0xFFFF;
}

subnet_entry *
subnet_entry::factory(pds_subnet_spec_t *spec) {
    subnet_entry *subnet;

    // create subnet entry with defaults, if any
    subnet = subnet_db()->alloc();
    if (subnet) {
        new (subnet) subnet_entry();
        subnet->impl_ = impl_base::factory(impl::IMPL_OBJ_ID_SUBNET, spec);
    }
    return subnet;
}

subnet_entry::~subnet_entry() {
}

void
subnet_entry::destroy(subnet_entry *subnet) {
    subnet->nuke_resources_();
    if (subnet->impl_) {
        impl_base::destroy(impl::IMPL_OBJ_ID_SUBNET, subnet->impl_);
    }
    subnet->~subnet_entry();
    subnet_db()->free(subnet);
}

api_base *
subnet_entry::clone(api_ctxt_t *api_ctxt) {
    subnet_entry *cloned_subnet;

    cloned_subnet = subnet_db()->alloc();
    if (cloned_subnet) {
        new (cloned_subnet) subnet_entry();
        if (cloned_subnet->init_config(api_ctxt) != SDK_RET_OK) {
            goto error;
        }
        cloned_subnet->impl_ = impl_->clone();
        if (unlikely(cloned_subnet->impl_ == NULL)) {
            PDS_TRACE_ERR("Failed to clone subnet %s impl", key_.str());
            goto error;
        }
        cloned_subnet->hw_id_ = hw_id_;
    }
    return cloned_subnet;

error:

    cloned_subnet->~subnet_entry();
    subnet_db()->free(cloned_subnet);
    return NULL;
}

sdk_ret_t
subnet_entry::free(subnet_entry *subnet) {
    if (subnet->impl_) {
        impl_base::free(impl::IMPL_OBJ_ID_SUBNET, subnet->impl_);
    }
    subnet->~subnet_entry();
    subnet_db()->free(subnet);
    return SDK_RET_OK;
}

sdk_ret_t
subnet_entry::reserve_resources(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret = SDK_RET_OK;

    switch (obj_ctxt->api_op) {
    case API_OP_CREATE:
        if (impl_) {
            ret = impl_->reserve_resources(this, orig_obj, obj_ctxt);
        } else {
            if (subnet_db()->subnet_idxr()->alloc(&this->hw_id_) ==
                    sdk::lib::indexer::SUCCESS) {
            } else {
                ret = sdk::SDK_RET_NO_RESOURCE;
            }
        }
        break;

    case API_OP_UPDATE:
        return SDK_RET_OK;

    case API_OP_DELETE:
    default:
        ret = sdk::SDK_RET_INVALID_OP;
    }
    return ret;
}

sdk_ret_t
subnet_entry::release_resources(void) {
    if (impl_) {
        impl_->release_resources(this);
    }
    if (hw_id_ != 0xFFFF) {
        subnet_db()->subnet_idxr()->free(hw_id_);
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_entry::nuke_resources_(void) {
    if (impl_) {
        impl_->nuke_resources(this);
    }
    if (hw_id_ != 0xFFFF) {
        subnet_db()->subnet_idxr()->free(hw_id_);
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_entry::init_config(api_ctxt_t *api_ctxt) {
    pds_subnet_spec_t *spec = &api_ctxt->api_params->subnet_spec;

    PDS_TRACE_VERBOSE(
        "Initializing subnet (vpc %s, subnet %s), v4/v6 pfx %s/%s,\n"
        "v4/v6 VR IP %s/%s, VR MAC %s, v4/v6 route table %s/%s\n"
        "num ingress v4/v6 policy %u/%u, num egress v4/v6 policy %u/%u, "
        "vnid %u", spec->vpc.str(), spec->key.str(),
        ipv4pfx2str(&spec->v4_prefix), ippfx2str(&spec->v6_prefix),
        ipv4addr2str(spec->v4_vr_ip), ipaddr2str(&spec->v6_vr_ip),
        macaddr2str(spec->vr_mac), spec->v4_route_table.str(),
        spec->v6_route_table.str(), spec->num_ing_v4_policy,
        spec->num_ing_v6_policy, spec->num_egr_v4_policy,
        spec->num_egr_v6_policy, spec->fabric_encap.val.vnid);

    key_ = spec->key;
    vpc_ = spec->vpc;
    fabric_encap_ = spec->fabric_encap;
    v4_route_table_ = spec->v4_route_table;
    v6_route_table_ = spec->v6_route_table;
    num_ing_v4_policy_ = spec->num_ing_v4_policy;
    for (uint8_t i = 0; i < num_ing_v4_policy_; i++) {
        ing_v4_policy_[i] = spec->ing_v4_policy[i];
    }
    num_ing_v6_policy_ = spec->num_ing_v6_policy;
    for (uint8_t i = 0; i < num_ing_v6_policy_; i++) {
        ing_v6_policy_[i] = spec->ing_v6_policy[i];
    }
    num_egr_v4_policy_ = spec->num_egr_v4_policy;
    for (uint8_t i = 0; i < num_egr_v4_policy_; i++) {
        egr_v4_policy_[i] = spec->egr_v4_policy[i];
    }
    num_egr_v6_policy_ = spec->num_egr_v6_policy;
    for (uint8_t i = 0; i < num_egr_v6_policy_; i++) {
        egr_v6_policy_[i] = spec->egr_v6_policy[i];
    }
    v4_vr_ip_ = spec->v4_vr_ip;
    v6_vr_ip_ = spec->v6_vr_ip;
    memcpy(&vr_mac_, &spec->vr_mac, sizeof(mac_addr_t));
    host_if_ = spec->host_if;
    if (host_if_ != k_pds_obj_key_invalid) {
        if (unlikely(lif_db()->find(&host_if_) == NULL)) {
            PDS_TRACE_ERR("host if %s not found, subnet %s init failed",
                          spec->host_if.str(), spec->key.str());
            return SDK_RET_INVALID_ARG;
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_entry::populate_msg(pds_msg_t *msg, api_obj_ctxt_t *obj_ctxt) {
    msg->id = PDS_CFG_MSG_ID_SUBNET;
    msg->cfg_msg.op = obj_ctxt->api_op;
    msg->cfg_msg.obj_id = OBJ_ID_SUBNET;
    if (obj_ctxt->api_op == API_OP_DELETE) {
        msg->cfg_msg.subnet.key = obj_ctxt->api_params->key;
    } else {
        msg->cfg_msg.subnet.spec = obj_ctxt->api_params->subnet_spec;
        if (impl_) {
            impl_->populate_msg(msg, this, obj_ctxt);
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_entry::program_create(api_obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->program_hw(this, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_entry::cleanup_config(api_obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->cleanup_hw(this, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_entry::compute_update(api_obj_ctxt_t *obj_ctxt) {
    pds_subnet_spec_t *spec = &obj_ctxt->api_params->subnet_spec;

    obj_ctxt->upd_bmap = 0;
    if (vpc_ != spec->vpc) {
        PDS_TRACE_ERR("Attempt to modify immutable attr \"vpc\" "
                      "from %s to %s on subnet %s",
                      vpc_.str(), spec->vpc.str(), key2str().c_str());
            return SDK_RET_INVALID_ARG;
    }
    if ((fabric_encap_.type != spec->fabric_encap.type) ||
        (fabric_encap_.val.value != spec->fabric_encap.val.value)) {
        PDS_TRACE_ERR("Attempt to modify immutable attr \"fabric encap\" "
                      "from %u to %u on subnet %s",
                      pds_encap2str(&fabric_encap_),
                      pds_encap2str(&spec->fabric_encap), key2str().c_str());
        return SDK_RET_INVALID_ARG;
    }
    if ((v4_route_table_ != spec->v4_route_table) ||
        (v6_route_table_ !=  spec->v6_route_table)) {
        obj_ctxt->upd_bmap |= PDS_SUBNET_UPD_ROUTE_TABLE;
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
        obj_ctxt->upd_bmap |= PDS_SUBNET_UPD_POLICY;
    }
    if (host_if_ != spec->host_if) {
        obj_ctxt->upd_bmap |= PDS_SUBNET_UPD_HOST_IFINDEX;
    }
    if (v4_vr_ip_ != spec->v4_vr_ip) {
        obj_ctxt->upd_bmap |= PDS_SUBNET_UPD_V4_VR_IP;
    }
    if (memcmp(&v6_vr_ip_, &spec->v6_vr_ip, sizeof(v6_vr_ip_))) {
        obj_ctxt->upd_bmap |= PDS_SUBNET_UPD_V6_VR_IP;
    }
    PDS_TRACE_DEBUG("subnet %s upd bmap 0x%lx",
                    key2str().c_str(), obj_ctxt->upd_bmap);
    return SDK_RET_OK;
}

static bool
vnic_upd_walk_cb_ (void *api_obj, void *ctxt) {
    vnic_entry *vnic;
    subnet_upd_ctxt_t *upd_ctxt = (subnet_upd_ctxt_t *)ctxt;

    vnic = (vnic_entry *)api_framework_obj((api_base *)api_obj);
    if (vnic->subnet() == upd_ctxt->subnet->key()) {
        api_obj_add_to_deps(upd_ctxt->obj_ctxt->api_op,
                            OBJ_ID_SUBNET, upd_ctxt->subnet,
                            OBJ_ID_VNIC, (api_base *)api_obj,
                            upd_ctxt->upd_bmap);
    }
    return false;
}

sdk_ret_t
subnet_entry::add_deps(api_obj_ctxt_t *obj_ctxt) {
    subnet_upd_ctxt_t upd_ctxt = { 0 };

    // if either policy or route table is updated, we need to fix
    // vnic programming in the datapath
    if ((obj_ctxt->upd_bmap & PDS_SUBNET_UPD_POLICY) ||
        (obj_ctxt->upd_bmap & PDS_SUBNET_UPD_ROUTE_TABLE)) {
        upd_ctxt.subnet = this;
        upd_ctxt.obj_ctxt = obj_ctxt;
        upd_ctxt.upd_bmap =
            obj_ctxt->upd_bmap & (PDS_VNIC_UPD_POLICY |
                                  PDS_VNIC_UPD_ROUTE_TABLE);
        return vnic_db()->walk(vnic_upd_walk_cb_, &upd_ctxt);
    }
    // in all other cases, it is sufficient to contain the update
    // programming to this subnet object alone
    return SDK_RET_OK;
}

sdk_ret_t
subnet_entry::program_update(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->update_hw(orig_obj, this, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_entry::activate_config(pds_epoch_t epoch, api_op_t api_op,
                              api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        PDS_TRACE_DEBUG("Activating subnet %s config", key_.str());
        return impl_->activate_hw(this, orig_obj, epoch, api_op, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_entry::reprogram_config(api_obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->reprogram_hw(this, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_entry::reactivate_config(pds_epoch_t epoch, api_obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->reactivate_hw(this, epoch, obj_ctxt);
    }
    return SDK_RET_OK;
}

void
subnet_entry::fill_spec_(pds_subnet_spec_t *spec) {
    uint8_t i;

    memcpy(&spec->key, &key_, sizeof(pds_obj_key_t));
    memcpy(&spec->vpc, &vpc_, sizeof(pds_obj_key_t));
    spec->v4_route_table = v4_route_table_;
    spec->v6_route_table = v6_route_table_;
    spec->num_ing_v4_policy = num_ing_v4_policy_;
    for (i = 0; i < num_ing_v4_policy_; i++) {
        spec->ing_v4_policy[i] = ing_v4_policy_[i];
    }
    spec->num_ing_v6_policy = num_ing_v6_policy_;
    for (i = 0; i < num_ing_v6_policy_; i++) {
        spec->ing_v6_policy[i] = ing_v6_policy_[i];
    }
    spec->num_egr_v4_policy = num_egr_v4_policy_;
    for (i = 0; i < num_egr_v4_policy_; i++) {
        spec->egr_v4_policy[i] = egr_v4_policy_[i];
    }
    spec->num_egr_v6_policy = num_egr_v6_policy_;
    for (i = 0; i < num_egr_v6_policy_; i++) {
        spec->egr_v6_policy[i] = egr_v6_policy_[i];
    }
    memcpy(&spec->vr_mac, vr_mac_, sizeof(mac_addr_t));
    spec->fabric_encap = fabric_encap_;
    spec->host_if = host_if_;
}

sdk_ret_t
subnet_entry::read(pds_subnet_info_t *info) {
    fill_spec_(&info->spec);
    if (impl_) {
        return impl_->read_hw(this, (impl::obj_key_t *)(&info->spec.key),
                              (impl::obj_info_t *)info);
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_entry::add_to_db(void) {
    return subnet_db()->insert(this);
}

sdk_ret_t
subnet_entry::del_from_db(void) {
    if (subnet_db()->remove(this)) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
subnet_entry::update_db(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (subnet_db()->remove((subnet_entry *)orig_obj)) {
        return subnet_db()->insert(this);
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
subnet_entry::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_SUBNET, this);
}

}    // namespace api
