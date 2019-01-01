/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    tep_impl.cc
 *
 * @brief   datapath implementation of tep
 */

#include "nic/hal/apollo/core/mem.hpp"
#include "nic/hal/apollo/framework/api_engine.hpp"
#include "nic/hal/apollo/api/tep.hpp"
#include "nic/hal/apollo/api/impl/tep_impl.hpp"
#include "nic/hal/apollo/api/impl/oci_impl_state.hpp"
#include "gen/p4gen/apollo/include/p4pd.h"
#include "nic/sdk/lib/p4/p4_api.hpp"

namespace impl {

/**
 * @defgroup OCI_TEP_IMPL - tep entry datapath implementation
 * @ingroup OCI_TEP
 * @{
 */

/**
 * @brief    factory method to allocate & initialize tep impl instance
 * @param[in] oci_tep    tep information
 * @return    new instance of tep or NULL, in case of error
 */
tep_impl *
tep_impl::factory(oci_tep_t *oci_tep) {
    tep_impl *impl;

    // TODO: move to slab later
    impl = (tep_impl *)SDK_CALLOC(SDK_MEM_ALLOC_OCI_TEP_IMPL,
                                  sizeof(tep_impl));
    new (impl) tep_impl();
    return impl;
}

/**
 * @brief    release all the s/w state associated with the given tep,
 *           if any, and free the memory
 * @param[in] tep     tep to be freed
 * NOTE: h/w entries should have been cleaned up (by calling
 *       impl->cleanup_hw() before calling this
 */
void
tep_impl::destroy(tep_impl *impl) {
    impl->~tep_impl();
    SDK_FREE(SDK_MEM_ALLOC_OCI_TEP_IMPL, impl);
}

/**
 * @brief    allocate/reserve h/w resources for this object
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
tep_impl::alloc_resources(api_base *api_obj) {
    // TODO: if directmap provides a way to reserve() we dont need this indexer
    //       at all !!
    if (tep_impl_db()->tep_idxr()->alloc((uint32_t *)&hw_id_) !=
            sdk::lib::indexer::SUCCESS) {
        return sdk::SDK_RET_NO_RESOURCE;
    }
    return SDK_RET_OK;
}

/**
 * @brief     free h/w resources used by this object, if any
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
tep_impl::free_resources(api_base *api_obj) {
    if (hw_id_ != 0xFF) {
        tep_impl_db()->tep_idxr()->free(hw_id_);
        tep_impl_db()->tep_tx_tbl()->remove(hw_id_);
    }
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief    program all h/w tables relevant to this object except stage 0
 *           table(s), if any
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
#define tep_tx_udp_action    action_u.tep_tx_udp_tep_tx
sdk_ret_t
tep_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t              ret;
    oci_tep_t              *tep_info;
    tep_tx_actiondata_t    tep_tx_data = { 0 };

    tep_info = &obj_ctxt->api_params->tep_info;
    tep_tx_data.action_id = TEP_TX_UDP_TEP_TX_ID;
    tep_tx_data.tep_tx_udp_action.dipo = tep_info->key.ip_addr;
    MAC_UINT64_TO_ADDR(tep_tx_data.tep_tx_udp_action.dmac,
                       0x00020B0A0D0E);
    ret = tep_impl_db()->tep_tx_tbl()->insert_withid(&tep_tx_data, hw_id_);
    return ret;
}

/**
 * @brief    cleanup all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
tep_impl::cleanup_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief    update all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] orig_obj    old version of the unmodified object
 * @param[in] curr_obj    cloned and updated version of the object
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
tep_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                    obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/** @} */    // end of OCI_TEP_IMPL

}    // namespace impl
