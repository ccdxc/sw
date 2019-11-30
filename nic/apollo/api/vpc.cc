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

vpc_entry::vpc_entry() {
    type_ = PDS_VPC_TYPE_NONE;
    v4_route_table_.id = PDS_ROUTE_TABLE_ID_INVALID;
    v6_route_table_.id = PDS_ROUTE_TABLE_ID_INVALID;
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

sdk_ret_t
vpc_entry::init_config(api_ctxt_t *api_ctxt) {
    pds_vpc_spec_t *spec = &api_ctxt->api_params->vpc_spec;

    PDS_TRACE_VERBOSE("Initializing vpc %u, type %u", spec->key.id, spec->type);
    memcpy(&key_, &spec->key, sizeof(pds_vpc_key_t));
    type_ = spec->type;
    fabric_encap_ = spec->fabric_encap;
    v4_route_table_.id = spec->v4_route_table.id;
    v6_route_table_.id = spec->v6_route_table.id;
    if (ip_addr_is_zero(&spec->nat46_prefix.addr)) {
        nat46_pfx_valid_ = false;
    } else {
        nat46_pfx_valid_ = true;
        memcpy(&nat46_pfx_, &spec->nat46_prefix, sizeof(ip_prefix_t));
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_entry::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    uint32_t idx;

    switch (obj_ctxt->api_op) {
    case API_OP_CREATE:
        if (vpc_db()->vpc_idxr()->alloc((uint32_t *)&idx) !=
                sdk::lib::indexer::SUCCESS) {
            PDS_TRACE_ERR("Failed to allocate index for vpc idx %u", idx);
            return sdk::SDK_RET_NO_RESOURCE;
        }
        hw_id_ = idx & 0xFFFF;
        if (impl_) {
            impl_->reserve_resources(this, obj_ctxt);
        }
        break;

    case API_OP_UPDATE:
        return SDK_RET_OK;

    case API_OP_DELETE:
    default:
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
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
    if (hw_id_ == 0xFFFF) {
        // resources not yet allocated
        return sdk::SDK_RET_OK;
    }

    if (impl_) {
        impl_->nuke_resources(this);
    }
    return this->release_resources();
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
                PDS_TRACE_ERR("Failed to clone vpc %u impl", key_.id);
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
vpc_entry::program_create(obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->program_hw(this, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_entry::cleanup_config(obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->cleanup_hw(this, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_entry::reprogram_config(api_op_t api_op) {
    if (impl_) {
        return impl_->reprogram_hw(this, api_op);
    }
    return SDK_RET_ERR;
}

sdk_ret_t
vpc_entry::compute_update(obj_ctxt_t *obj_ctxt) {
    pds_vpc_spec_t *spec = &obj_ctxt->api_params->vpc_spec;

    if (type_ != spec->type) {
        PDS_TRACE_ERR("Attempt to modify immutable attr \"type\" from %u to %u "
                      "on vpc %s", type_, spec->type, key2str());
        return SDK_RET_INVALID_ARG;
    }
    if ((fabric_encap_.type != spec->fabric_encap.type) ||
        (fabric_encap_.val.value != spec->fabric_encap.val.value)) {
        PDS_TRACE_ERR("Attempt to modify immutable attr \"fabric encap\" "
                      "from %u to %u on vpc %s", pds_encap2str(&fabric_encap_),
                      pds_encap2str(&spec->fabric_encap));
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_entry::program_update(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->update_hw(orig_obj, this, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_entry::activate_config(pds_epoch_t epoch, api_op_t api_op,
                           api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        PDS_TRACE_VERBOSE("Activating vpc %u config", key_.id);
        return impl_->activate_hw(this, orig_obj, epoch, api_op, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_entry::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_VPC, this);
}

sdk_ret_t
vpc_entry::read(pds_vpc_key_t *key, pds_vpc_info_t *info) {
    if (impl_) {
        return impl_->read_hw(this, (impl::obj_key_t *)key,
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
vpc_entry::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    // TODO: FIXME
    return sdk::SDK_RET_INVALID_OP;
}

}    // namespace api
