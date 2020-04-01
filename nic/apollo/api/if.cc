//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// interface entry handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/sdk/linkmgr/port.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/if.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/include/pds_if.hpp"

using sdk::lib::catalog;

namespace api {

if_entry::if_entry() {
    ht_ctxt_.reset();
    memset(&if_info_, 0, sizeof(if_info_));
    ifindex_ = IFINDEX_INVALID;
    ifindex_ht_ctxt_.reset();
}

// TODO: this method should go away !!!
if_entry *
if_entry::factory(pds_obj_key_t& key, pds_ifindex_t ifindex) {
    if_entry *intf;

    // create interface entry with defaults, if any
    intf = if_db()->alloc();
    if (intf) {
        new (intf) if_entry();
    }
    memcpy(&intf->key_, &key, sizeof(key_));
    intf->type_ = ifindex_to_pds_if_type(ifindex);
    intf->ifindex_ = ifindex;
    return intf;
}

if_entry *
if_entry::factory(pds_if_spec_t *spec) {
    if_entry *intf;

    // create interface entry with defaults, if any
    intf = if_db()->alloc();
    if (intf) {
        new (intf) if_entry();
        intf->impl_ = impl_base::factory(impl::IMPL_OBJ_ID_IF, spec);
    }
    return intf;
}

if_entry::~if_entry() {
}

void
if_entry::destroy(if_entry *intf) {
    intf->nuke_resources_();
    if (intf->impl_) {
        impl_base::destroy(impl::IMPL_OBJ_ID_IF, intf->impl_);
    }
    intf->~if_entry();
    if_db()->free(intf);
}

api_base *
if_entry::clone(api_ctxt_t *api_ctxt) {
    if_entry *cloned_if;

    cloned_if = if_db()->alloc();
    if (cloned_if) {
        new (cloned_if) if_entry();
        if (cloned_if->init_config(api_ctxt) != SDK_RET_OK) {
            goto error;
        }
        cloned_if->impl_ = impl_->clone();
        if (unlikely(cloned_if->impl_ == NULL)) {
            PDS_TRACE_ERR("Failed to clone intf %s impl", key_.str());
            goto error;
        }
    }
    return cloned_if;

error:

    cloned_if->~if_entry();
    if_db()->free(cloned_if);
    return NULL;
}

sdk_ret_t
if_entry::free(if_entry *intf) {
    if (intf->impl_) {
        impl_base::free(impl::IMPL_OBJ_ID_IF, intf->impl_);
    }
    intf->~if_entry();
    if_db()->free(intf);
    return SDK_RET_OK;
}

sdk_ret_t
if_entry::reserve_resources(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->reserve_resources(this, orig_obj, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
if_entry::release_resources(void) {
    if (impl_) {
        impl_->release_resources(this);
    }
    return SDK_RET_OK;
}

sdk_ret_t
if_entry::nuke_resources_(void) {
    if (impl_) {
        return impl_->nuke_resources(this);
    }
    return SDK_RET_OK;
}

sdk_ret_t
if_entry::init_config(api_ctxt_t *api_ctxt) {
    static uint32_t l3_if_idxr_ = 0;
    pds_if_spec_t *spec = &api_ctxt->api_params->if_spec;

    memcpy(&key_, &spec->key, sizeof(key_));
    type_ = spec->type;
    admin_state_ = spec->admin_state;
    switch (type_) {
    case PDS_IF_TYPE_UPLINK:
        {
            if_entry *phy_intf;
            phy_intf = if_db()->find(&spec->uplink_info.port);
            ifindex_ = phy_intf->ifindex();
            ifindex_ = ETH_IFINDEX_TO_UPLINK_IFINDEX(ifindex_);
            PDS_TRACE_DEBUG("Initializing uplink interface %s, ifindex 0x%x, "
                            "port %s", spec->key.str(), ifindex_,
                            spec->uplink_info.port.str());
            if_info_.uplink_.port_ = spec->uplink_info.port;
        }
        break;

    case PDS_IF_TYPE_L3:
         ifindex_ = L3_IFINDEX(l3_if_idxr_++);
         PDS_TRACE_DEBUG("Initializing L3 interface %s, ifindex 0x%x, "
                         "port %s", spec->key.str(), ifindex_,
                         spec->l3_if_info.port.str());
         if_info_.l3_.vpc_ = spec->l3_if_info.vpc;
         if_info_.l3_.ip_pfx_ = spec->l3_if_info.ip_prefix;
         if_info_.l3_.port_ = spec->l3_if_info.port;
         if_info_.l3_.encap_ = spec->l3_if_info.encap;
         memcpy(if_info_.l3_.mac_, spec->l3_if_info.mac_addr,
                ETH_ADDR_LEN);
         break;

    case PDS_IF_TYPE_CONTROL:
         ifindex_ = CONTROL_IFINDEX(0);
         PDS_TRACE_DEBUG("Initializing inband control interface %s, ifindex 0x%x",
                         spec->key.str(), ifindex_);
         if_info_.control_.ip_pfx_ = spec->control_if_info.ip_prefix;
         memcpy(if_info_.control_.mac_, spec->control_if_info.mac_addr,
                ETH_ADDR_LEN);
         if_info_.control_.gateway_ = spec->control_if_info.gateway;
         break;

    case PDS_IF_TYPE_NONE:
        break;

    default:
         return sdk::SDK_RET_INVALID_ARG;
         break;
    }
     return SDK_RET_OK;
}

sdk_ret_t
if_entry::program_create(api_obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->program_hw(this, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
if_entry::cleanup_config(api_obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->cleanup_hw(this, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
if_entry::compute_update(api_obj_ctxt_t *obj_ctxt) {
    pds_if_spec_t *spec = &obj_ctxt->api_params->if_spec;

    obj_ctxt->upd_bmap = 0;
    if (type_ != spec->type) {
        PDS_TRACE_ERR("Attempt to modify immutable attr \"type\" "
                      "from type %u to %u on interface %s",
                      type_, spec->type, key_.str());
        return SDK_RET_INVALID_ARG;
    }

    if (admin_state_ != spec->admin_state) {
        obj_ctxt->upd_bmap |= PDS_IF_UPD_ADMIN_STATE;
    }

    if (type_ == PDS_IF_TYPE_UPLINK) {
        // no other changes are relevant to uplink
        return SDK_RET_OK;
    }
    return SDK_RET_OK;
}

sdk_ret_t
if_entry::program_update(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->update_hw(orig_obj, this, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
if_entry::activate_config(pds_epoch_t epoch, api_op_t api_op,
                          api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->activate_hw(this, orig_obj, epoch, api_op, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
if_entry::fill_spec_(pds_if_spec_t *spec) {
    memcpy(&spec->key, &key_, sizeof(key_));
    spec->type = type_;
    spec->admin_state = admin_state_;

    switch (spec->type) {
    case PDS_IF_TYPE_UPLINK:
        spec->uplink_info.port = if_info_.uplink_.port_;
        break;
    case PDS_IF_TYPE_L3:
        spec->l3_if_info.port = if_info_.l3_.port_;
        spec->l3_if_info.vpc = if_info_.l3_.vpc_;
        spec->l3_if_info.ip_prefix = if_info_.l3_.ip_pfx_;
        spec->l3_if_info.encap = if_info_.l3_.encap_;
        memcpy(spec->l3_if_info.mac_addr, if_info_.l3_.mac_,
               ETH_ADDR_LEN);
        break;
    case PDS_IF_TYPE_CONTROL:
        spec->control_if_info.ip_prefix = if_info_.control_.ip_pfx_;
        memcpy(spec->control_if_info.mac_addr, if_info_.control_.mac_,
               ETH_ADDR_LEN);
        spec->control_if_info.gateway = if_info_.control_.gateway_;
        break;
    case PDS_IF_TYPE_ETH:
        break;
    default:
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

pds_if_state_t
if_entry::state(void) {
    sdk_ret_t ret;
    if_entry *eth_if;
    port_args_t port_args = { 0 };

    // get the eth interface corresponding to this interface entry
    eth_if = if_entry::eth_if((if_entry *)this);
    if (eth_if == NULL) {
        return PDS_IF_STATE_NONE;
    }
    // and get the oper state of the eth interface
    ret = sdk::linkmgr::port_get(eth_if->port_info(), &port_args);
    if (likely(ret == SDK_RET_OK)) {
        if (port_args.oper_status == port_oper_status_t::PORT_OPER_STATUS_UP) {
            return PDS_IF_STATE_UP;
        }
        return PDS_IF_STATE_DOWN;
    }
    PDS_TRACE_ERR("Failed to get port 0x%x info, err %u", ifindex_, ret);
    return PDS_IF_STATE_NONE;
}

sdk_ret_t
if_entry::read(pds_if_info_t *info) {
    sdk_ret_t ret;

    ret = fill_spec_(&info->spec);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    info->status.state = this->state();
    if (impl_) {
        return impl_->read_hw(this, (impl::obj_key_t *)(&info->spec.key),
                              (impl::obj_info_t *)info);
    }
    return SDK_RET_OK;
}

sdk_ret_t
if_entry::add_to_db(void) {
    return if_db()->insert(this);
}

sdk_ret_t
if_entry::del_from_db(void) {
    if (if_db()->remove(this)) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
if_entry::update_db(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (if_db()->remove((if_entry *)orig_obj)) {
        return if_db()->insert(this);
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
if_entry::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_IF, this);
}

if_entry *
if_entry::eth_if(if_entry *intf) {
    switch (intf->type()) {
    case PDS_IF_TYPE_UPLINK:
        return if_db()->find(&intf->if_info_.uplink_.port_);

    case PDS_IF_TYPE_L3:
        return if_db()->find(&intf->if_info_.l3_.port_);

    case PDS_IF_TYPE_ETH:
        return intf;
    default:
        PDS_TRACE_ERR("Unknown interface type %u", intf->type());
    }
    return NULL;
}

}    // namespace api
