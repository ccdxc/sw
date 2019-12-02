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
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/subnet.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

typedef struct subnet_update_ctxt_s {
    subnet_entry *subnet;
    obj_ctxt_t *obj_ctxt;
} __PACK__ subnet_update_ctxt_t;

subnet_entry::subnet_entry() {
    v4_route_table_.id = PDS_ROUTE_TABLE_ID_INVALID;
    v6_route_table_.id = PDS_ROUTE_TABLE_ID_INVALID;
    ing_v4_policy_.id = PDS_POLICY_ID_INVALID;
    ing_v6_policy_.id = PDS_POLICY_ID_INVALID;
    egr_v4_policy_.id = PDS_POLICY_ID_INVALID;
    egr_v6_policy_.id = PDS_POLICY_ID_INVALID;

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
        cloned_subnet->impl_ = impl_->clone();
        if (unlikely(cloned_subnet->impl_ == NULL)) {
            PDS_TRACE_ERR("Failed to clone subnet %u impl", key_.id);
            goto error;
        }
        cloned_subnet->init_config(api_ctxt);
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
subnet_entry::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret = sdk::SDK_RET_OK;

    switch (obj_ctxt->api_op) {
    case API_OP_CREATE:
        if (impl_) {
            ret = impl_->reserve_resources(this, obj_ctxt);
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
        "Initializing subnet (vpc %u, subnet %u), v4/v6 pfx %s/%s,\n"
        "v4/v6 VR IP %s/%s, VR MAC %s, v4/v6 route table %u/%u\n"
        "ingress v4/v6 policy %u/%u, egress v4/v6 policy %u/%u, vnid %u",
        spec->vpc.id, spec->key.id, ipv4pfx2str(&spec->v4_prefix),
        ippfx2str(&spec->v6_prefix), ipv4addr2str(spec->v4_vr_ip),
        ipaddr2str(&spec->v6_vr_ip), macaddr2str(spec->vr_mac),
        spec->v4_route_table.id, spec->v6_route_table.id,
        spec->ing_v4_policy.id, spec->ing_v6_policy.id,
        spec->egr_v4_policy.id, spec->egr_v6_policy.id,
        spec->fabric_encap.val.vnid);

    key_.id = spec->key.id;
    vpc_ = spec->vpc;
    fabric_encap_ = spec->fabric_encap;
    v4_route_table_.id = spec->v4_route_table.id;
    v6_route_table_.id = spec->v6_route_table.id;
    ing_v4_policy_.id = spec->ing_v4_policy.id;
    ing_v6_policy_.id = spec->ing_v6_policy.id;
    egr_v4_policy_.id = spec->egr_v4_policy.id;
    egr_v6_policy_.id = spec->egr_v6_policy.id;
    memcpy(&vr_mac_, &spec->vr_mac, sizeof(mac_addr_t));
    return SDK_RET_OK;
}

sdk_ret_t
subnet_entry::program_create(obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->program_hw(this, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_entry::cleanup_config(obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->cleanup_hw(this, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_entry::reprogram_config(api_op_t api_op) {
    if (impl_) {
        return impl_->reprogram_hw(this, api_op);
    }
    return SDK_RET_ERR;
}

sdk_ret_t
subnet_entry::compute_update(obj_ctxt_t *obj_ctxt) {
    pds_subnet_spec_t *spec = &obj_ctxt->api_params->subnet_spec;

    obj_ctxt->upd_bmap = 0;
    if (vpc_.id != spec->vpc.id) {
        PDS_TRACE_ERR("Attempt to modify immutable attr \"vpc\" "
                      "from %u to %u on subnet %s",
                      vpc_.id, spec->vpc.id, key2str());
            return SDK_RET_INVALID_ARG;
    }
    if ((fabric_encap_.type != spec->fabric_encap.type) ||
        (fabric_encap_.val.value != spec->fabric_encap.val.value)) {
        PDS_TRACE_ERR("Attempt to modify immutable attr \"fabric encap\" "
                      "from %u to %u on subnet %s",
                      pds_encap2str(&fabric_encap_),
                      pds_encap2str(&spec->fabric_encap));
        return SDK_RET_INVALID_ARG;
    }
    if ((v4_route_table_.id != spec->v4_route_table.id) ||
        (v6_route_table_.id !=  spec->v6_route_table.id)) {
        obj_ctxt->upd_bmap |= PDS_SUBNET_UPD_ROUTE_TABLE;
    }
    if ((ing_v4_policy_.id != spec->ing_v4_policy.id) ||
        (ing_v6_policy_.id != spec->ing_v6_policy.id) ||
        (egr_v4_policy_.id != spec->egr_v4_policy.id) ||
        (egr_v6_policy_.id != spec->egr_v6_policy.id)) {
        obj_ctxt->upd_bmap |= PDS_SUBNET_UPD_POLICY;
    }
    if (host_ifindex_ != spec->host_ifindex) {
        obj_ctxt->upd_bmap |= PDS_SUBNET_UPD_HOST_IFINDEX;
    }
    PDS_TRACE_DEBUG("subnet %u upd bmap 0x%lx", obj_ctxt->upd_bmap);
    return SDK_RET_OK;
}

sdk_ret_t
subnet_entry::program_update(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->update_hw(orig_obj, this, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_entry::activate_config(pds_epoch_t epoch, api_op_t api_op,
                              api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        PDS_TRACE_DEBUG("Activating subnet %u config", key_.id);
        return impl_->activate_hw(this, orig_obj, epoch, api_op, obj_ctxt);
    }
    return SDK_RET_OK;
}

static bool
vnic_upd_walk_cb_(void *api_obj, void *ctxt) {
    vnic_entry *vnic = (vnic_entry *)api_obj;
    subnet_update_ctxt_t *upd_ctxt = (subnet_update_ctxt_t *)ctxt;

    if (vnic->subnet().id == upd_ctxt->subnet->key().id) {
        upd_ctxt->obj_ctxt->add_deps(vnic, API_OP_UPDATE);
    }
    return false;
}

sdk_ret_t
subnet_entry::add_deps(obj_ctxt_t *obj_ctxt) {
    subnet_update_ctxt_t upd_ctxt = { 0 };

    upd_ctxt.subnet = this;
    upd_ctxt.obj_ctxt = obj_ctxt;
    return vnic_db()->walk(vnic_upd_walk_cb_, &upd_ctxt);
}

sdk_ret_t
subnet_entry::read(pds_subnet_key_t *key, pds_subnet_info_t *info) {
    if (impl_) {
        return impl_->read_hw(this, (impl::obj_key_t *)key,
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
subnet_entry::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    // TODO: FIXME
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
subnet_entry::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_SUBNET, this);
}

}    // namespace api
