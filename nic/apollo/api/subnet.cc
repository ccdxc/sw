/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    subnet.cc
 *
 * @brief   subnet entry handling
 */

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/subnet.hpp"
#include "nic/apollo/api/oci_state.hpp"
#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"

namespace api {

/**
 * @defgroup OCI_SUBNET_ENTRY - subnet entry functionality
 * @ingroup OCI_SUBNET
 * @{
 */

/**< @brief    constructor */
subnet_entry::subnet_entry() {
    //SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    v4_route_table_.id = OCI_ROUTE_TABLE_ID_INVALID;
    v6_route_table_.id = OCI_ROUTE_TABLE_ID_INVALID;
    ing_v4_policy_.id = OCI_POLICY_ID_INVALID;
    ing_v6_policy_.id = OCI_POLICY_ID_INVALID;
    egr_v4_policy_.id = OCI_POLICY_ID_INVALID;
    egr_v6_policy_.id = OCI_POLICY_ID_INVALID;
    ht_ctxt_.reset();
    hw_id_ = 0xFFFF;
}

/**
 * @brief    factory method to allocate and initialize a subnet entry
 * @param[in] oci_subnet    subnet information
 * @return    new instance of subnet or NULL, in case of error
 */
subnet_entry *
subnet_entry::factory(oci_subnet_t *oci_subnet) {
    subnet_entry *subnet;

    /**< create subnet entry with defaults, if any */
    subnet = subnet_db()->subnet_alloc();
    if (subnet) {
        new (subnet) subnet_entry();
    }
    return subnet;
}

/**< @brief    destructor */
subnet_entry::~subnet_entry() {
    // TODO: fix me
    //SDK_SPINLOCK_DESTROY(&slock_);
}

/**
 * @brief    release all the s/w & h/w state associated with this object, if
 *           any, and free the memory
 * @param[in] subnet     subnet to be freed
 * NOTE: h/w entries themselves should have been cleaned up (by calling
 *       impl->cleanup_hw() before calling this
 */
void
subnet_entry::destroy(subnet_entry *subnet) {
    subnet->release_resources();
    subnet->~subnet_entry();
    subnet_db()->subnet_free(subnet);
}

/**
 * @brief     initialize subnet entry with the given config
 * @param[in] api_ctxt API context carrying the configuration
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::init_config(api_ctxt_t *api_ctxt) {
    oci_subnet_t *oci_subnet = &api_ctxt->api_params->subnet_info;

    key_.id = oci_subnet->key.id;
    v4_route_table_.id = oci_subnet->v4_route_table.id;
    v6_route_table_.id = oci_subnet->v6_route_table.id;
    ing_v4_policy_.id = oci_subnet->ing_v4_policy.id;
    ing_v6_policy_.id = oci_subnet->ing_v6_policy.id;
    egr_v4_policy_.id = oci_subnet->egr_v4_policy.id;
    egr_v6_policy_.id = oci_subnet->egr_v6_policy.id;
    memcpy(&vr_mac_, &oci_subnet->vr_mac, sizeof(mac_addr_t));
    this->ht_ctxt_.reset();
    return SDK_RET_OK;
}

/**
 * @brief    allocate h/w resources for this object
 * @param[in] orig_obj    old version of the unmodified object
 * @param[in] obj_ctxt    transient state associated with this API
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    if (subnet_db()->subnet_idxr()->alloc((uint32_t *)&this->hw_id_) !=
            sdk::lib::indexer::SUCCESS) {
        return sdk::SDK_RET_NO_RESOURCE;
    }
    return SDK_RET_OK;
}

/**
 * @brief    program all h/w tables relevant to this object except stage 0
 *           table(s), if any, during creation of the object
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::program_config(obj_ctxt_t *obj_ctxt) {
    oci_subnet_t *oci_subnet = &obj_ctxt->api_params->subnet_info;

    /**
     * there is no h/w programming for subnet config but a h/w id is needed so
     * we can use while programming vnics, routes etc.
     */
    OCI_TRACE_DEBUG("Creating subnet (vcn %u, subnet %u), pfx %s, VR IP %s, "
                    "VR MAC %s, v4 route table %u, v6 route table %u, "
                    "ingress v4 policy %u, ingress v6 policy %u, "
                    "egress v4 policy %u, egress v6 policy %u",
                    oci_subnet->vcn.id, key_.id,
                    ippfx2str(&oci_subnet->pfx), ipaddr2str(&oci_subnet->vr_ip),
                    macaddr2str(oci_subnet->vr_mac),
                    oci_subnet->v4_route_table.id,
                    oci_subnet->v6_route_table.id,
                    oci_subnet->ing_v4_policy.id, oci_subnet->ing_v6_policy.id,
                    oci_subnet->egr_v4_policy.id, oci_subnet->egr_v6_policy.id);
    return SDK_RET_OK;
}

/**
 * @brief     free h/w resources used by this object, if any
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::release_resources(void) {
    if (hw_id_ != 0xFF) {
        subnet_db()->subnet_idxr()->free(hw_id_);
    }
    return SDK_RET_OK;
}

/**
 * @brief    cleanup all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::cleanup_config(obj_ctxt_t *obj_ctxt) {
    // impl->cleanup_hw();
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief    update all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] orig_obj    old version of the unmodified object
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::update_config(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    // impl->update_hw();
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief    activate the epoch in the dataplane by programming stage 0
 *           tables, if any
 * @param[in] epoch       epoch being activated
 * @param[in] api_op      api operation
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::activate_config(oci_epoch_t epoch, api_op_t api_op,
                              obj_ctxt_t *obj_ctxt) {
    oci_subnet_t *oci_subnet = &obj_ctxt->api_params->subnet_info;

    /**< no h/w programming for subnet config, so nothing to activate */
    OCI_TRACE_DEBUG("Created subnet (vcn %u, subnet %u)", oci_subnet->vcn.id,
                    key_.id);
    return SDK_RET_OK;
}

/**
 * @brief    this method is called on new object that needs to replace the
 *           old version of the object in the DBs
 * @param[in] orig_obj    old version of the unmodified object
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief add subnet to database
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::add_to_db(void) {
    return subnet_db()->subnet_ht()->insert_with_key(&key_, this,
                                                     &ht_ctxt_);
}

/**
 * @brief delete subnet from database
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::del_from_db(void) {
    subnet_db()->subnet_ht()->remove(&key_);
    return SDK_RET_OK;
}

/**
 * @brief    initiate delay deletion of this object
 */
sdk_ret_t
subnet_entry::delay_delete(void) {
    return delay_delete_to_slab(OCI_SLAB_ID_SUBNET, this);
}

/** @} */    // end of OCI_SUBNET_ENTRY

}    // namespace api
