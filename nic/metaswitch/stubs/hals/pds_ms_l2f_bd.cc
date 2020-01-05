//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// LI BD HAL integration
//---------------------------------------------------------------

#include <l2f_c_includes.hpp>
#include "nic/metaswitch/stubs/hals/pds_ms_l2f_bd.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_l2f_mai.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_l2f_utils.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_hal_init.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include <thread>

extern NBB_ULONG l2f_proc_id;

// ------------------------------------------------------------------- 
// Subnet Spec has a number of attributes that are meaningless to MS.
// So the Subnet Spec generated from the Subnet Proto received from top
// is cached as it is and sent to PDS HAL.
//
// Subnet Create -
// a) MS Mgmt Stub initiator receives Subnet create Proto.
// b) It creates Subnet store obj holding the received Subnet Spec.
// c) MS Mgmt Stub creates BD, LIF SotfwIf, AC Bind and IRB MIB entries.
// d) MS control-plane asynchronously calls L2F Stub with BD create.
//    MS Mgmt Stub returns without waiting for BD Stub call invocation.
//    So further updates on the same Subnet are to be expected.
// ----------   
// e) L2F Stub (this code) sends PDS Subnet create to HAL with latest 
//    Subnet Spec cached in Subnet Store. 
// f) L2F stub integration component creates new BD object initialized with
//    MS owned atributes. Unlike other stub integration components it
//    immediately adds it to the store without waiting for HAL async response
//    since any further fastpath updates would use this to determine whether
//    to send PDS update to HAL. 
// ----------   
// g) Any gRPC Subnet updates received before step f) for non-MS owned fields 
//    in this subnet will only update the Subnet Spec cached in the Subnet store
//    but not call PDS API since BD Store obj is not created.
// h) Any gRPC Subnet updates received before step f) for MS owned fields
//    will result in MS MIB tables configuration.
//    MS will not send BD/IRB update/delete IPS to Stub until previous 
//    async create/upd response is received.
// i) Any gRPC Subnet updates received after step f) for non MS owned fields
//    will invoke l2f_bd_update_pds_synch for direct fastpath
//    synchronous HAL update.
// j) L2F Stub returns async IPS reponse when async PDS response is received 
//    from HAL. BD store object is removed if HAL indicates creation failure.
// 
// Subnet delete - Reverse of create
// a) PDS MS Mgmt Stub deletes BD, IRB, LIF SoftwIf and AC Bind 
//    MIB table entries.
// b) PDS MS Mgmt Stub deletes Subnet obj holding cached Subnet Spec from Store.
// c) MS calls L2F Stub with BD delete if no prev IPS response pending after
//    first deleting all remote FDB, MAC-IP pairs learnt for this BD.
// d) L2F stub (this code) sends PDS Delete to HAL.
// e) L2F stub deletes the BD obj from the store without waiting for HAL response.
//
// Subnet update after PDS HAL Create is invoked -
// a) PDS MS Mgmt Stub updates cached Subnet Spec in BD obj
// b) If Slowpath MS owned fields changed (FabricEncap(VNI) or HostIfIndex) -
//      - PDS MS Mgmt Stub updates BD or BD If Bind MIB entry.
//      - MS asynchronously calls L2F Stub API (this code).
//    If Fastpath PDS owned (non-MS) fields changed -
//      - PDS MS Mgmt Stub directly calls L2F Stub BD Update bypassing MS
//        to perform synchronous HAL update.
// c) L2F stub (this code) copies cached subnet spec to PDS Batch. 
// d) If Subnet store obj is null then update is ignored since MS may send 
//    IPS update to BD Stub after PDS MS Mgmt Subnet delete if prev 
//    async IPS response was delayed.
// ------------------------------------------------------------------- 

namespace pds_ms {

void l2f_bd_t::parse_ips_info_(ATG_BDPI_UPDATE_BD* bd_add_upd_ips) {
    ips_info_.bd_id = bd_add_upd_ips->bd_id.bd_id;
    ips_info_.vnid = bd_add_upd_ips->bd_properties.vni;
    // Rest of the attributes can be fetched directly from cached spec
}

void l2f_bd_t::fetch_store_info_(pds_ms::state_t* state) {
    store_info_.subnet_obj = state->subnet_store().get(ips_info_.bd_id);
    store_info_.bd_obj = state->bd_store().get(ips_info_.bd_id);
    if (store_info_.bd_obj == nullptr) {
        op_create_ = true;
    }
}

pds_subnet_key_t l2f_bd_t::make_pds_subnet_key_(void) {
    return pds_subnet_key_t {ips_info_.bd_id};
}

pds_subnet_spec_t l2f_bd_t::make_pds_subnet_spec_(void) {
    pds_subnet_spec_t spec;
    spec = store_info_.subnet_obj->spec();
    // Spec contains MS + PDS owned fields. Overwrite the MS owned fields only
    spec.fabric_encap = store_info_.bd_obj->properties().fabric_encap;
    spec.host_ifindex = store_info_.bd_obj->properties().host_ifindex;
    SDK_TRACE_INFO ("MS BD %d: Using VNI %d Host IfIndex 0x%x",
                    spec.key.id, spec.fabric_encap.val.vnid, spec.host_ifindex);

    return spec;
}

pds_batch_ctxt_guard_t l2f_bd_t::make_batch_pds_spec_(state_t::context_t& state_ctxt,
                                                      bool async) {
    pds_batch_ctxt_guard_t bctxt_guard_;

    if (async) {
        SDK_ASSERT(cookie_uptr_); // Cookie should not be empty
    }
    pds_batch_params_t bp {PDS_BATCH_PARAMS_EPOCH,
                           async ? PDS_BATCH_PARAMS_ASYNC : false, 
                           pds_ms::hal_callback,
                           async ? cookie_uptr_.get() : nullptr};
    auto bctxt = pds_batch_start(&bp);

    if (unlikely (!bctxt)) {
        throw Error(std::string("PDS Batch Start failed for MS BD ")
                    .append(std::to_string(ips_info_.bd_id)));
    }
    bctxt_guard_.set (bctxt);

    if (op_delete_) { // Delete
        // First delete remote mapping entries for all MACs on this BD
        l2f_del_remote_macs_for_bd(state_ctxt, ips_info_.bd_id, bctxt);

        auto if_key = make_pds_subnet_key_();
        if (!PDS_MOCK_MODE()) {
            pds_subnet_delete(&if_key, bctxt);
        }

    } else { // Add or update
        auto subnet_spec = make_pds_subnet_spec_();
        sdk_ret_t ret = SDK_RET_OK;
        if (op_create_) {
            if (!PDS_MOCK_MODE()) {
                ret = pds_subnet_create(&subnet_spec, bctxt);
            }
        } else {
            if (!PDS_MOCK_MODE()) {
                ret = pds_subnet_update(&subnet_spec, bctxt);
            }
        }
        if (unlikely (ret != SDK_RET_OK)) {
            throw Error(std::string("PDS Subnet Create or Update failed for MS BD ")
                        .append(std::to_string(ips_info_.bd_id))
                        .append(" err=").append(std::to_string(ret)));
        }
    }
    return bctxt_guard_;
}

pds_batch_ctxt_guard_t l2f_bd_t::prepare_pds(state_t::context_t& state_ctxt,
                                             bool async) {
    auto& pds_spec = store_info_.subnet_obj->spec();
    SDK_TRACE_INFO ("MS BD %d: VPC %d VNI %d IP %s Host IfIndex 0x%x",
                    pds_spec.key.id, pds_spec.vpc.id, 
                    pds_spec.fabric_encap.val.vnid,
                    ipv4addr2str(pds_spec.v4_vr_ip), pds_spec.host_ifindex);

    auto pds_bctxt_guard = make_batch_pds_spec_(state_ctxt, async); 

    // If we have batched multiple IPS earlier, flush it now
    // Cannot add Subnet Create/Update to an existing batch
    state_ctxt.state()->flush_outstanding_pds_batch();
    return pds_bctxt_guard;
}

void l2f_bd_t::handle_add_upd_ips(ATG_BDPI_UPDATE_BD* bd_add_upd_ips) {
    bd_add_upd_ips->return_code = ATG_OK;
    parse_ips_info_(bd_add_upd_ips);
    pds_ms::cookie_t* cookie;

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = pds_ms::state_t::thread_context();
        fetch_store_info_(state_ctxt.state());
        if (unlikely(store_info_.subnet_obj == nullptr)) {
            // The prev BD IPS response could have possibly been delayed
            // beyond Subnet Spec delete - Ignore and return success to MS
            SDK_TRACE_INFO ("BD %d: AddUpd IPS for unknown BD", ips_info_.bd_id);
            return;
        }
        bd_obj_uptr_t bd_obj_uptr; 
        if (op_create_) {
            auto& spec = store_info_.subnet_obj->spec();
            bd_obj_uptr.reset(new bd_obj_t(ips_info_.bd_id, spec.vpc.id));
            store_info_.bd_obj = bd_obj_uptr.get();
        }
        store_info_.bd_obj->properties().fabric_encap.type = PDS_ENCAP_TYPE_VXLAN;
        store_info_.bd_obj->properties().fabric_encap.val.vnid = ips_info_.vnid;
        SDK_TRACE_INFO("MS BD %d: %s IPS VNI %ld", ips_info_.bd_id,
                       (op_create_) ? "Create" : "Update",
                       store_info_.bd_obj->properties().fabric_encap.val.vnid);

        auto l_op_create = op_create_;
        cookie_uptr_.reset(new cookie_t);
        auto pds_bctxt_guard = prepare_pds(state_ctxt, true /* async */);

        // Subnet updates can happen directly from the MS Mgmt initiator thread
        // Avoid race by holding the state_ctxt lock until batch commit call returns

        cookie_uptr_->send_ips_reply = 
            [l_op_create, bd_add_upd_ips] 
                     (bool pds_status, bool ips_mock) -> void {
            // ----------------------------------------------------------------
            // This block is executed asynchronously when PDS response is rcvd
            // ----------------------------------------------------------------
            if (!pds_status && l_op_create) {
                // Create failed - Erase the BD Obj saved in store
                SDK_TRACE_DEBUG ("MS BD %d: Create failed "
                                 "- delete store obj ",
                                 bd_add_upd_ips->bd_id);
                // Enter thread-safe context to access/modify global state
                auto state_ctxt = state_t::thread_context();
                state_ctxt.state()->bd_store().erase(bd_add_upd_ips->bd_id.bd_id);
            }

            if (unlikely(ips_mock)) return; // UT

            NBB_CREATE_THREAD_CONTEXT
            NBS_ENTER_SHARED_CONTEXT(l2f_proc_id);
            NBS_GET_SHARED_DATA();

            do {
            auto bdpi_join = get_l2f_bdpi_join();
            if (bdpi_join == nullptr) {
                SDK_TRACE_ERR("Failed to find BDPI join to return BD %d AddUpd IPS",
                              bd_add_upd_ips->bd_id);
                break;
            }
            auto& bd_store = bdpi_join->get_bd_store();

            auto key = l2f::Bd::get_key(*bd_add_upd_ips);
            auto it = bd_store.find(key);
            if (it == bd_store.end()) {
                auto send_response = 
                    l2f::Bd::set_ips_rc(&bd_add_upd_ips->ips_hdr,
                                        (pds_status) ? ATG_OK : ATG_UNSUCCESSFUL);
                SDK_ASSERT(send_response);
                SDK_TRACE_DEBUG ("MS BD %d: Send Async IPS "
                                 "reply %s stateless mode",
                                bd_add_upd_ips->bd_id.bd_id, 
                                (pds_status) ? "Success" : "Failure");
                bdpi_join->send_ips_reply(&bd_add_upd_ips->ips_hdr);
            } else {
                if (pds_status) {
                    SDK_TRACE_DEBUG("MS BD %d: Send Async IPS "
                                    "Reply success stateful mode",
                                    bd_add_upd_ips->bd_id);
                    (*it)->update_complete(ATG_OK);
                } else {
                    SDK_TRACE_DEBUG("MS BD %d: Send Async IPS "
                                    "Reply failure stateful mode",
                                    bd_add_upd_ips->bd_id);
                    (*it)->update_failed(ATG_UNSUCCESSFUL);
                }
            }
            } while (0);
            NBS_RELEASE_SHARED_DATA();
            NBS_EXIT_SHARED_CONTEXT();
            NBB_DESTROY_THREAD_CONTEXT    
        };

        // All processing complete, only batch commit remains - 
        // safe to release the cookie_uptr_ unique_ptr
        cookie = cookie_uptr_.release();
        auto ret = pds_batch_commit(pds_bctxt_guard.release());
        if (unlikely (ret != SDK_RET_OK)) {
            delete cookie;
            throw Error(std::string("Batch commit failed for Add-Update MS BD ")
                        .append(std::to_string(ips_info_.bd_id))
                        .append(" err=").append(std::to_string(ret)));
        }

        // Update BD Obj to store to ensure that updates to PDS owned fields
        // are sent directly to HAL (fastpath) from the MS Mgmt Initiator thread
        if (op_create_) {
            state_ctxt.state()->bd_store().add_upd(ips_info_.bd_id,
                                                   std::move(bd_obj_uptr));
        }
    } // End of state thread_context
      // Do Not access/modify global state after this

    bd_add_upd_ips->return_code = ATG_ASYNC_COMPLETION;
    SDK_TRACE_DEBUG ("MS BD %d: Add/Upd PDS Batch commit successful", 
                     ips_info_.bd_id);

    if (PDS_MOCK_MODE()) {
        // Call the HAL callback in PDS mock mode
        std::thread cb(pds_ms::hal_callback, SDK_RET_OK, cookie);
        cb.detach();
    }
}

void l2f_bd_t::handle_delete(NBB_ULONG bd_id) {
    pds_batch_ctxt_guard_t  pds_bctxt_guard;
    op_delete_ = true;

    // MS stub Integration APIs do not support Async callback for deletes.
    // However since we should not block the MS NBase main thread
    // the HAL processing is always asynchronous even for deletes. 
    // Assuming that Deletes never fail the Store is also updated
    // in a synchronous fashion for deletes so that it is in sync
    // if there is a subsequent create from MS.

    ips_info_.bd_id = bd_id;
    SDK_TRACE_INFO ("MS BD %d: Delete IPS", ips_info_.bd_id);

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = pds_ms::state_t::thread_context();

        // Empty cookie to force async PDS.
        cookie_uptr_.reset (new cookie_t);

        pds_bctxt_guard = make_batch_pds_spec_ (state_ctxt, true /* async */); 

        // If we have batched multiple IPS earlier flush it now
        // Cannot add Subnet Delete to an existing batch
        state_ctxt.state()->flush_outstanding_pds_batch();

        // Remove the BD Obj from store
        // All remote MACs should have been walked and delete spec
        // added to the batch before this
        state_ctxt.state()->bd_store().erase(ips_info_.bd_id);
    } // End of state thread_context
      // Do Not access/modify global state after this

    auto l_bd_id = ips_info_.bd_id; 
    cookie_uptr_->send_ips_reply = 
        [l_bd_id] (bool pds_status, bool ips_mock) -> void {
            // ----------------------------------------------------------------
            // This block is executed asynchronously when PDS response is rcvd
            // ----------------------------------------------------------------
            SDK_TRACE_DEBUG("MS BD %d Delete: Rcvd Async PDS response %s",
                            l_bd_id, (pds_status) ? "Success" : "Failure");

        };
    // All processing complete, only batch commit remains - 
    // safe to release the cookie_uptr_ unique_ptr
    auto cookie = cookie_uptr_.release();
    auto ret = pds_batch_commit(pds_bctxt_guard.release());
    if (unlikely (ret != SDK_RET_OK)) {
        delete cookie;
        throw Error(std::string("Batch commit failed for delete MS BD ")
                    .append(std::to_string(bd_id))
                    .append(" err=").append(std::to_string(ret)));
    }
    SDK_TRACE_DEBUG ("MS BD %d: Delete PDS Batch commit successful", bd_id);
}

void l2f_bd_t::handle_add_if(NBB_ULONG bd_id, ms_ifindex_t ifindex) {
    pds_batch_ctxt_guard_t  pds_bctxt_guard;
    ips_info_.bd_id = bd_id;
    if (ms_ifindex_to_pds_type(ifindex) != IF_TYPE_LIF) {
        SDK_TRACE_VERBOSE("Ignore Non-LIF interface 0x%x bind to BD %d",
                          ifindex, bd_id);
        return;
    }

    // BD Interface Bind API does not support Async callback.
    // However since we should not block the MS NBase main thread
    // the HAL processing is always asynchronous. 

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = pds_ms::state_t::thread_context();
        fetch_store_info_(state_ctxt.state());
        if (unlikely(store_info_.subnet_obj == nullptr)) {
            // The prev IPS response could have possibly been delayed
            // beyond Subnet Spec delete - Ignore and return success to MS
            SDK_TRACE_INFO ("MS BD %d If 0x%x: Bind IPS for unknown BD",
                            bd_id, ifindex);
            return;
        }
        store_info_.bd_obj->properties().host_ifindex = ms_to_pds_ifindex(ifindex);
        SDK_TRACE_INFO ("MS BD %d If 0x%x: Bind IPS PDS IfIndex 0x%x", 
                        bd_id, ifindex,
                        store_info_.bd_obj->properties().host_ifindex);

        // Empty cookie to force async PDS.
        cookie_uptr_.reset (new cookie_t);
        pds_bctxt_guard = make_batch_pds_spec_ (state_ctxt,
                                                true /* async */); 

        // If we have batched multiple IPS earlier flush it now
        // Cannot add Subnet If Bind to an existing batch
        state_ctxt.state()->flush_outstanding_pds_batch();

    } // End of state thread_context
      // Do Not access/modify global state after this

    cookie_uptr_->send_ips_reply = 
        [bd_id, ifindex] (bool pds_status, bool ips_mock) -> void {
            // ----------------------------------------------------------------
            // This block is executed asynchronously when PDS response is rcvd
            // ----------------------------------------------------------------
            SDK_TRACE_DEBUG("MS BD %d If 0x%x: Bind - Rcvd Async PDS response %s",
                            bd_id, ifindex, (pds_status) ? "Success" : "Failure");

        };
    // All processing complete, only batch commit remains - 
    // safe to release the cookie_uptr_ unique_ptr
    auto cookie = cookie_uptr_.release();
    auto ret = pds_batch_commit(pds_bctxt_guard.release());
    if (unlikely (ret != SDK_RET_OK)) {
        delete cookie;
        throw Error(std::string("Batch commit failed for MS BD ")
                    .append(std::to_string(bd_id))
                    .append(" MS If ").append(std::to_string(ifindex))
                    .append(" Bind err=").append(std::to_string(ret)));
    }
    SDK_TRACE_DEBUG("MS BD %d If 0x%x: Bind PDS Batch commit successful",
                    ips_info_.bd_id, ifindex);
}

void l2f_bd_t::handle_del_if(NBB_ULONG bd_id, ms_ifindex_t ifindex) {
    pds_batch_ctxt_guard_t  pds_bctxt_guard;
    ips_info_.bd_id = bd_id;

    // BD Interface Unbind API does not support Async callback.
    // However since we should not block the MS NBase main thread
    // the HAL processing is always asynchronous. 

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = pds_ms::state_t::thread_context();
        fetch_store_info_(state_ctxt.state());
        if (unlikely(store_info_.subnet_obj == nullptr)) {
            // The prev IPS response could have possibly been delayed
            // beyond Subnet Spec delete - Ignore and return success to MS
            SDK_TRACE_INFO ("MS BD %d If 0x%x: Unbind IPS for unknown BD",
                            bd_id, ifindex);
            return;
        }
        store_info_.bd_obj->properties().host_ifindex = 0;
        SDK_TRACE_INFO ("MS BD %d If 0x%x: Unbind IPS", bd_id, ifindex);

        // Empty cookie to force async PDS.
        cookie_uptr_.reset (new cookie_t);
        pds_bctxt_guard = make_batch_pds_spec_ (state_ctxt,
                                                true /* async */); 

        // If we have batched multiple IPS earlier flush it now
        // Cannot add Subnet If Unbind to an existing batch
        state_ctxt.state()->flush_outstanding_pds_batch();

    } // End of state thread_context
      // Do Not access/modify global state after this

    cookie_uptr_->send_ips_reply = 
        [bd_id, ifindex] (bool pds_status, bool ips_mock) -> void {
            // ----------------------------------------------------------------
            // This block is executed asynchronously when PDS response is rcvd
            // ----------------------------------------------------------------
            SDK_TRACE_DEBUG("MS BD %d If 0x%x: Unbind - Rcvd Async PDS response %s",
                            bd_id, ifindex, (pds_status) ? "Success" : "Failure");

        };
    // All processing complete, only batch commit remains - 
    // safe to release the cookie_uptr_ unique_ptr
    auto cookie = cookie_uptr_.release();
    auto ret = pds_batch_commit(pds_bctxt_guard.release());
    if (unlikely (ret != SDK_RET_OK)) {
        delete cookie;
        throw Error(std::string("Batch commit failed for BD ")
                    .append(std::to_string(bd_id))
                    .append(" If ").append(std::to_string(ifindex))
                    .append(" Unbind err=").append(std::to_string(ret)));
    }
    SDK_TRACE_DEBUG("MS BD %d If 0x%x: Unbind PDS Batch commit successful",
                    ips_info_.bd_id, ifindex);
}

// ------- Fastpath - Direct call from Mgmt Stub initiator thread ---------
sdk_ret_t l2f_bd_t::update_pds_synch(state_t::context_t&& in_state_ctxt,
                                     subnet_obj_t* subnet_obj) {
    pds_batch_ctxt_guard_t  pds_bctxt_guard;

    { // Continue thread-safe context passed in to access/modify global state
        state_t::context_t state_ctxt (std::move(in_state_ctxt));

        ips_info_.bd_id = subnet_obj->key();
        store_info_.subnet_obj = subnet_obj;
        store_info_.bd_obj = state_ctxt.state()->bd_store().get(ips_info_.bd_id);

        if (unlikely(store_info_.bd_obj == nullptr)) {
            // L2F BD has not created the subnet in PDS HAL yet.
            // L2F BD will update PDS HAL with the latest cached subnet spec
            SDK_TRACE_DEBUG("MS BD %d: Ignore Direct Update before BD Create", 
                            ips_info_.bd_id);
            return SDK_RET_OK;
        }

        SDK_TRACE_DEBUG("MS BD %d: Received Direct Update for BD", ips_info_.bd_id);
        pds_bctxt_guard = prepare_pds(state_ctxt, false /* sync */);

        // This is a synchronous batch commit.
        // Ensure that state lock is released to avoid blocking NBASE thread
    } // End of state thread_context. Do Not access/modify global state

    auto ret = pds_batch_commit(pds_bctxt_guard.release());
    if (unlikely (ret != SDK_RET_OK)) {
        SDK_TRACE_ERR ("MS BD %d: Add/Upd PDS Direct Update Batch commit"
                       "failed %d", ips_info_.bd_id, ret);
        return ret;
    }

    SDK_TRACE_DEBUG ("MS BD %d: Add/Upd PDS Direct Update Batch commit successful", 
                     ips_info_.bd_id);
    return SDK_RET_OK;
}

sdk_ret_t
l2f_bd_update_pds_synch (state_t::context_t&& state_ctxt, 
                         subnet_obj_t* subnet_obj)
{
    try {
        l2f_bd_t bd;
        return bd.update_pds_synch(std::move(state_ctxt), subnet_obj);
    } catch (Error& e) {
        SDK_TRACE_ERR ("BD Direct Update processing failed %s", e.what());
        return SDK_RET_ERR;
    }
}

} // End namespace
