//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// DHCP API object handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/core/msg.h"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/dhcp.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

dhcp_policy::dhcp_policy() {
    impl_ = NULL;
    server_ip_ = {0};
    mtu_ = 0;
    gateway_ip_ = {0};
    dns_server_ip_ = {0};
    ntp_server_ip_ = {0};
    memset(domain_name_, '\0', sizeof(domain_name_));
    lease_timeout_ = 0;
 
    ht_ctxt_.reset();
}

dhcp_policy::~dhcp_policy() {
}

dhcp_policy *
dhcp_policy::factory(pds_dhcp_policy_spec_t *spec) {
    dhcp_policy *policy;

    // create DHCP policy entry with defaults, if any
    policy = dhcp_db()->alloc();
    if (policy) {
        new (policy) dhcp_policy();
        if (spec->type == PDS_DHCP_POLICY_TYPE_RELAY) {
            policy->impl_ =
                impl_base::factory(impl::IMPL_OBJ_ID_DHCP_POLICY, spec);
            if (policy->impl_ == NULL) {
                dhcp_policy::destroy(policy);
                return NULL;
            }
        }
    }
    return policy;
}

void
dhcp_policy::destroy(dhcp_policy *policy) {
    policy->nuke_resources_();
    if (policy->impl_) {
        impl_base::destroy(impl::IMPL_OBJ_ID_DHCP_POLICY,
                           policy->impl_);
    }
    policy->~dhcp_policy();
    dhcp_db()->free(policy);
}

api_base *
dhcp_policy::clone(api_ctxt_t *api_ctxt) {
    dhcp_policy *cloned_policy;

    cloned_policy = dhcp_db()->alloc();
    if (cloned_policy) {
        new (cloned_policy) dhcp_policy();
        if (cloned_policy->init_config(api_ctxt) != SDK_RET_OK) {
            goto error;
        }
        if (impl_) {
            cloned_policy->impl_ = impl_->clone();
            if (unlikely(cloned_policy->impl_ == NULL)) {
                PDS_TRACE_ERR("Failed to clone DHCP relay %s impl", key_.str());
                goto error;
            }
        }
    }
    return cloned_policy;

error:

    cloned_policy->~dhcp_policy();
    dhcp_db()->free(cloned_policy);
    return NULL;
}

sdk_ret_t
dhcp_policy::free(dhcp_policy *policy) {
    if (policy->impl_) {
        impl_base::destroy(impl::IMPL_OBJ_ID_DHCP_POLICY,
                           policy->impl_);
    }
    policy->~dhcp_policy();
    dhcp_db()->free(policy);
    return SDK_RET_OK;
}

sdk_ret_t
dhcp_policy::reserve_resources(api_base *orig_obj,
                               api_obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->reserve_resources(this, orig_obj, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
dhcp_policy::release_resources(void) {
    if (impl_) {
        return impl_->release_resources(this);
    }
    return SDK_RET_OK;
}

sdk_ret_t
dhcp_policy::nuke_resources_(void) {
    if (impl_) {
        return impl_->nuke_resources(this);
    }
    return SDK_RET_OK;
}

sdk_ret_t
dhcp_policy::init_config(api_ctxt_t *api_ctxt) {
    ip_addr_t mytep_ip;
    device_entry *device;
    pds_dhcp_policy_spec_t *spec;
    pds_dhcp_proxy_spec_t *dhcp_proxy_spec;

    spec = &api_ctxt->api_params->dhcp_policy_spec;
    key_ = spec->key;
    type_ = spec->type;
    if (spec->type == PDS_DHCP_POLICY_TYPE_PROXY) {
        dhcp_proxy_spec = &spec->proxy_spec;
        server_ip_ = dhcp_proxy_spec->server_ip;
        mtu_ = dhcp_proxy_spec->mtu;
        gateway_ip_ = dhcp_proxy_spec->gateway_ip;
        dns_server_ip_ = dhcp_proxy_spec->dns_server_ip;
        ntp_server_ip_ = dhcp_proxy_spec->ntp_server_ip;
        memcpy(domain_name_, dhcp_proxy_spec->domain_name, sizeof(domain_name_));
        lease_timeout_ = dhcp_proxy_spec->lease_timeout;
    } else if (spec->type == PDS_DHCP_POLICY_TYPE_RELAY) {
        PDS_TRACE_DEBUG("DHCP server IP %s",
                        ipaddr2str(&spec->relay_spec.server_ip));
        if (spec->relay_spec.server_ip.af != IP_AF_IPV4) {
            PDS_TRACE_ERR("Invalid DHCP relay server IP, only IPv4 supported");
            return SDK_RET_INVALID_ARG;
        }
        if (spec->relay_spec.agent_ip.af == IP_AF_NIL) {
            // not (local) DHCP relay agent IP provided, use mytep IP
            device = device_db()->find();
            if (likely(device)) {
                mytep_ip = device->ip_addr();
                if (mytep_ip.af == IP_AF_NIL) {
                    PDS_TRACE_ERR("Invalid DHCP relay config rejected, relay "
                                  "agent IP, device TEP IP are not configured");
                    return SDK_RET_INVALID_ARG;
                }
                memcpy(&spec->relay_spec.agent_ip, &mytep_ip,
                       sizeof(ip_addr_t));
            } else {
                PDS_TRACE_ERR("Invalid DHCP relay config rejected, relay agent "
                              "IP, device object are not configured");
                return SDK_RET_INVALID_ARG;
            }
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
dhcp_policy::populate_msg(pds_msg_t *msg, api_obj_ctxt_t *obj_ctxt) {
    msg->id = PDS_CFG_MSG_ID_DHCP_POLICY;
    msg->cfg_msg.op = obj_ctxt->api_op;
    msg->cfg_msg.obj_id = OBJ_ID_DHCP_POLICY;
    if (obj_ctxt->api_op == API_OP_DELETE) {
        msg->cfg_msg.dhcp_policy.key = obj_ctxt->api_params->key;
    } else {
        msg->cfg_msg.dhcp_policy.spec = obj_ctxt->api_params->dhcp_policy_spec;
    }
    return SDK_RET_OK;
}

sdk_ret_t
dhcp_policy::compute_update(api_obj_ctxt_t *obj_ctxt) {
    pds_dhcp_policy_spec_t *spec = &obj_ctxt->api_params->dhcp_policy_spec;
    if (type_ != spec->type) {
        PDS_TRACE_ERR("Attempt to modify immutable attr \"type\" from "
                      "%u to %u on dhcp policy %s", type_, spec->type,
                      key_.str());
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

sdk_ret_t
dhcp_policy::activate_config(pds_epoch_t epoch, api_op_t api_op,
                            api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->activate_hw(this, orig_obj, epoch, api_op, obj_ctxt);
    }
    return SDK_RET_OK;
}

void
dhcp_policy::fill_spec_(pds_dhcp_policy_spec_t *spec) {
    spec->key = key_;
    spec->type = type_;
    pds_dhcp_proxy_spec_t *dhcp_proxy_spec;

    if (spec->type == PDS_DHCP_POLICY_TYPE_PROXY) {
        dhcp_proxy_spec = &spec->proxy_spec;
        dhcp_proxy_spec->server_ip = server_ip_;
        dhcp_proxy_spec->mtu = mtu_;
        dhcp_proxy_spec->gateway_ip = gateway_ip_;
        dhcp_proxy_spec->dns_server_ip = dns_server_ip_;
        dhcp_proxy_spec->ntp_server_ip = ntp_server_ip_;
        memcpy(dhcp_proxy_spec->domain_name, domain_name_, sizeof(domain_name_));
        dhcp_proxy_spec->lease_timeout = lease_timeout_;
    }
}

sdk_ret_t
dhcp_policy::read(pds_dhcp_policy_info_t *info) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
dhcp_policy::add_to_db(void) {
    return dhcp_db()->insert(this);
}

sdk_ret_t
dhcp_policy::del_from_db(void) {
    if (dhcp_db()->remove(this)) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
dhcp_policy::update_db(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (dhcp_db()->remove((dhcp_policy *)orig_obj)) {
        return dhcp_db()->insert(this);
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
dhcp_policy::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_DHCP_POLICY, this);
}

}    // namespace api
