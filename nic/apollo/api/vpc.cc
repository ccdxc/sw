//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// vpc entry handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/vpc.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

typedef struct vpc_upd_ctxt_s {
    vpc_entry *vpc;
    api_obj_ctxt_t *obj_ctxt;
    uint64_t upd_bmap;
} __PACK__ vpc_upd_ctxt_t;

vpc_entry::vpc_entry() {
    type_ = PDS_VPC_TYPE_NONE;
    v4_route_table_.reset();
    v6_route_table_.reset();
    ht_ctxt_.reset();
    hw_id_ = 0xFFFF;
    impl_ = NULL;
}

vpc_entry *
vpc_entry::factory(pds_vpc_spec_t *spec) {
    vpc_entry *vpc;

    // create vpc entry with defaults, if any
    vpc = vpc_db()->alloc();
    if (vpc) {
        new (vpc) vpc_entry();
        vpc->impl_ = impl_base::factory(impl::IMPL_OBJ_ID_VPC, spec);
    }
    return vpc;
}

vpc_entry::~vpc_entry() {
}

void
vpc_entry::destroy(vpc_entry *vpc) {
    vpc->nuke_resources_();
    if (vpc->impl_) {
        impl_base::destroy(impl::IMPL_OBJ_ID_VPC, vpc->impl_);
    }
    vpc->~vpc_entry();
    vpc_db()->free(vpc);
}

api_base *
vpc_entry::clone(api_ctxt_t *api_ctxt) {
    vpc_entry *cloned_vpc;

    cloned_vpc = vpc_db()->alloc();
    if (cloned_vpc) {
        new (cloned_vpc) vpc_entry();
        if (impl_) {
            cloned_vpc->impl_ = impl_->clone();
            if (unlikely(cloned_vpc->impl_ == NULL)) {
                PDS_TRACE_ERR("Failed to clone vpc %s impl", key_.str());
                goto error;
            }
        }
        cloned_vpc->init_config(api_ctxt);
        cloned_vpc->hw_id_ = hw_id_;
    }
    return cloned_vpc;

error:

    cloned_vpc->~vpc_entry();
    vpc_db()->free(cloned_vpc);
    return NULL;
}

sdk_ret_t
vpc_entry::free(vpc_entry *vpc) {
    if (vpc->impl_) {
        impl_base::free(impl::IMPL_OBJ_ID_VPC, vpc->impl_);
    }
    vpc->~vpc_entry();
    vpc_db()->free(vpc);
    return SDK_RET_OK;
}

sdk_ret_t
vpc_entry::init_config(api_ctxt_t *api_ctxt) {
    pds_vpc_spec_t *spec = &api_ctxt->api_params->vpc_spec;

    PDS_TRACE_VERBOSE("Initializing vpc %s, type %u",
                      spec->key.str(), spec->type);
    memcpy(&key_, &spec->key, sizeof(key_));
    type_ = spec->type;
    fabric_encap_ = spec->fabric_encap;
    v4_route_table_ = spec->v4_route_table;
    v6_route_table_ = spec->v6_route_table;
    if (ip_addr_is_zero(&spec->nat46_prefix.addr)) {
        nat46_pfx_valid_ = false;
    } else {
        nat46_pfx_valid_ = true;
        memcpy(&nat46_pfx_, &spec->nat46_prefix, sizeof(ip_prefix_t));
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_entry::reserve_resources(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret = SDK_RET_OK;

    switch (obj_ctxt->api_op) {
    case API_OP_CREATE:
        if (impl_) {
            ret = impl_->reserve_resources(this, obj_ctxt);
        } else {
            if (vpc_db()->vpc_idxr()->alloc((uint32_t *)&this->hw_id_) !=
                sdk::lib::indexer::SUCCESS) {
                PDS_TRACE_ERR("Failed to allocate index for vpc %s",
                              key_.str());
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
vpc_entry::release_resources(void) {
    if (impl_) {
        impl_->release_resources(this);
    }
    if (hw_id_ != 0xFFFF) {
        vpc_db()->vpc_idxr()->free(hw_id_);
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_entry::nuke_resources_(void) {
    if (impl_) {
        impl_->nuke_resources(this);
    }
    if (hw_id_ != 0xFFFF) {
        vpc_db()->vpc_idxr()->free(hw_id_);
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_entry::populate_msg(pds_msg_t *msg, api_obj_ctxt_t *obj_ctxt) {
    msg->id = PDS_CFG_MSG_ID_VPC;
    msg->cfg_msg.op = obj_ctxt->api_op;
    msg->cfg_msg.obj_id = OBJ_ID_VPC;
    if (obj_ctxt->api_op == API_OP_DELETE) {
        msg->cfg_msg.vpc.key = obj_ctxt->api_params->vpc_key;
    } else {
        msg->cfg_msg.vpc.spec = obj_ctxt->api_params->vpc_spec;
        if (impl_) {
            impl_->populate_msg(msg, this, obj_ctxt);
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_entry::program_create(api_obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->program_hw(this, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_entry::cleanup_config(api_obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->cleanup_hw(this, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_entry::compute_update(api_obj_ctxt_t *obj_ctxt) {
    pds_vpc_spec_t *spec = &obj_ctxt->api_params->vpc_spec;

    obj_ctxt->upd_bmap = 0;
    if (type_ != spec->type) {
        PDS_TRACE_ERR("Attempt to modify immutable attr \"type\" from %u to %u "
                      "on vpc %s", type_, spec->type, key2str().c_str());
        return SDK_RET_INVALID_ARG;
    }
    if ((fabric_encap_.type != spec->fabric_encap.type) ||
        (fabric_encap_.val.value != spec->fabric_encap.val.value)) {
        PDS_TRACE_ERR("Attempt to modify immutable attr \"fabric encap\" "
                      "from %u to %u on vpc %s", pds_encap2str(&fabric_encap_),
                      pds_encap2str(&spec->fabric_encap), key2str().c_str());
        return SDK_RET_INVALID_ARG;
    }
    if ((v4_route_table_ != spec->v4_route_table) ||
        (v6_route_table_ != spec->v6_route_table)) {
        obj_ctxt->upd_bmap |= PDS_VPC_UPD_ROUTE_TABLE;
    }
    // may be either tos or vrmac has changed
    return SDK_RET_OK;
}

sdk_ret_t
vpc_entry::program_update(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->update_hw(orig_obj, this, obj_ctxt);
    }
    return SDK_RET_OK;
}

static bool
subnet_upd_walk_cb_ (void *api_obj, void *ctxt) {
    subnet_entry *subnet;
    vpc_upd_ctxt_t *upd_ctxt = (vpc_upd_ctxt_t *)ctxt;

    subnet = (subnet_entry *)api_framework_obj((api_base *)api_obj);
    if (subnet->vpc() == upd_ctxt->vpc->key()) {
        if ((subnet->v4_route_table() == PDS_ROUTE_TABLE_ID_INVALID) ||
            (subnet->v6_route_table() == PDS_ROUTE_TABLE_ID_INVALID)) {
            // this subnet inherited the vpc's routing table(s)
            api_obj_add_to_deps(upd_ctxt->obj_ctxt->api_op,
                                OBJ_ID_VPC, upd_ctxt->vpc,
                                OBJ_ID_SUBNET, (api_base *)api_obj,
                                upd_ctxt->upd_bmap);
        }
    }
    return false;
}

sdk_ret_t
vpc_entry::add_deps(api_obj_ctxt_t *obj_ctxt) {
    vpc_upd_ctxt_t upd_ctxt = { 0 };

    if (obj_ctxt->upd_bmap & PDS_VPC_UPD_ROUTE_TABLE) {
        upd_ctxt.vpc = this;
        upd_ctxt.obj_ctxt = obj_ctxt;
        upd_ctxt.upd_bmap = PDS_SUBNET_UPD_ROUTE_TABLE;
        subnet_db()->walk(subnet_upd_walk_cb_, &upd_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_entry::activate_config(pds_epoch_t epoch, api_op_t api_op,
                           api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        PDS_TRACE_VERBOSE("Activating vpc %s config", key_.str());
        return impl_->activate_hw(this, orig_obj, epoch, api_op, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_entry::reprogram_config(api_obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->reprogram_hw(this, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_entry::reactivate_config(pds_epoch_t epoch, api_obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->reactivate_hw(this, epoch, obj_ctxt);
    }
    return SDK_RET_OK;
}

void
vpc_entry::fill_status_(pds_vpc_status_t *status) {
    status->hw_id = hw_id_;
}

void
vpc_entry::fill_spec_(pds_vpc_spec_t *spec) {
    memcpy(&spec->key, &key_, sizeof(pds_obj_key_t));
    spec->type = type_;
    spec->fabric_encap = fabric_encap_;
    if (nat46_pfx_valid_) {
        memcpy(&spec->nat46_prefix, &nat46_pfx_,
               sizeof(ip_prefix_t));
    }
}

sdk_ret_t
vpc_entry::read(pds_vpc_info_t *info) {
    fill_spec_(&info->spec);
    fill_status_(&info->status);
    if (impl_) {
        return impl_->read_hw(this, (impl::obj_key_t *)(&info->spec.key),
                              (impl::obj_info_t *)info);
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_entry::add_to_db(void) {
    return vpc_db()->insert(this);
}

sdk_ret_t
vpc_entry::del_from_db(void) {
    if (vpc_db()->remove(this)) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
vpc_entry::update_db(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (vpc_db()->remove((vpc_entry *)orig_obj)) {
        return vpc_db()->insert(this);
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
vpc_entry::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_VPC, this);
}

}    // namespace api
