//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// LI VRF HAL integration
//---------------------------------------------------------------

#include <thread>
#include "nic/metaswitch/stubs/hals/pds_ms_li_vrf.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_hal_init.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include <li_fte.hpp>
#include <li_lipi_slave_join.hpp>
#include <li_port.hpp>

extern NBB_ULONG li_proc_id;

// ------------------------------------------------------------------- 
// VPC Spec has a number of attributes that are meaningless to MS.
// So the VPC Spec generated from the SVC VPC Proto received from
// NetAgent is cached as it is and sent to PDS HAL.
//
// VPC Create -
// a) MS Mgmt Stub initiator receives VPC create Proto.
// b) It creates VPC store obj holding the received VPC Spec.
// c) MS Mgmt Stub creates VRF MIB entries.
// d) MS control-plane asynchronously calls LI Stub with VRF create.
//    MS Mgmt Stub returns without waiting for LI Stub call invocation.
//    So further updates on the same VPC are to be expected.
// ----------   
// e) LI Stub (this code) creates PDS VPC with latest VPC Spec cached
//    in VPC Store. hal_created flag is set in the VPC store object. 
// ----------   
// f) Any gRPC VPC updates received before step e) for non-MS owned fields 
//    in this VPC will update the VPC Spec cached in the VPC store but
//    not call PDS API since hal_created flag is not set.
// h) Any gRPC VPC updates received after step e) for non-MS owned fields
//    will invoke li_vrf_update_pds_synch for direct fastpath
//    synchronous HAL update.
// i) LI Stub returns async IPS reponse when async PDS response is received 
//    from HAL. hal_created flag is reset if async PDS response reports 
//    failure.
// 
// VPC delete - Reverse of create
// a) PDS MS Mgmt Stub deletes VRF MIB table entry.
// b) PDS MS Mgmt Stub marks the cached VPC store spec as invalid.
// c) MS calls LI Stub with VRF delete if no prev IPS response pending.
// d) LI stub (this code) erases the VPC store obj and sends
//    PDS Delete to HAL for the Route table and the VPC.
//
// VPC update after PDS HAL Create is invoked -
// a) PDS MS Mgmt Stub updates cached VPC Spec in VPC obj
// b) PDS MS Mgmt Stub directly calls LI Stub VRF Update bypassing MS
//    to perform synchronous HAL update since there are no Slowpath
//    MS owned fields to update in the MS VRF MIB.
// c) LI stub (this code) copies cached VPC spec to PDS Batch. 
// d) If spec_valid flag is false then update is ignored since MS may send 
//    IPS update to LI Stub after PDS MS Mgmt VPC delete if prev 
//    async IPS response was delayed.
// ------------------------------------------------------------------- 

namespace pds_ms {

void li_vrf_t::parse_ips_info_(ATG_LIPI_VRF_ADD_UPDATE* vrf_add_upd_ips) {
    ips_info_.vrf_id = vrfname_2_vrfid(vrf_add_upd_ips->vrf_name, vrf_add_upd_ips->vrf_name_len);
}

void li_vrf_t::fetch_store_info_(pds_ms::state_t* state) {
    store_info_.vpc_obj = state->vpc_store().get(ips_info_.vrf_id);
    if (likely(store_info_.vpc_obj != nullptr)) {
        op_create_ = !store_info_.vpc_obj->properties().hal_created;
    }
}

pds_vpc_key_t li_vrf_t::make_pds_vpc_key_(void) {
    // This could be delete case - use an object that is guaranteed
    // to be present
    return msidx2pdsobjkey(ips_info_.vrf_id);
}

pds_vpc_spec_t li_vrf_t::make_pds_vpc_spec_(void) {
    return store_info_.vpc_obj->properties().vpc_spec;
}

pds_route_table_key_t li_vrf_t::make_pds_rttable_key_(void) {
    // Get the route-table id from the VPC store
    pds_vpc_spec_t vpc_spec = make_pds_vpc_spec_();
    return (vpc_spec.v4_route_table);
}

pds_route_table_spec_t li_vrf_t::make_pds_rttable_spec_(void) {
    pds_route_table_spec_t rttable;
    memset(&rttable, 0, sizeof(pds_route_table_spec_t));
    rttable.key = make_pds_rttable_key_();
    rttable.af = IP_AF_IPV4;
    rttable.num_routes = 0;
    rttable.routes = NULL;
    return rttable;
}

pds_batch_ctxt_guard_t li_vrf_t::make_batch_pds_spec_(bool async) {
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
        throw Error(std::string("PDS Batch Start failed for MS VRF ")
                    .append(std::to_string(ips_info_.vrf_id)));
    }
    bctxt_guard_.set (bctxt);

    if (op_delete_) { // Delete
        auto rttbl_key = make_pds_rttable_key_();
        if (!PDS_MOCK_MODE()) {
            pds_route_table_delete(&rttbl_key, bctxt);
        }
        auto vpc_key = make_pds_vpc_key_();
        if (!PDS_MOCK_MODE()) {
            pds_vpc_delete(&vpc_key, bctxt);
        }
    } else { // Add or update
        auto vpc_spec = make_pds_vpc_spec_();
        auto rttbl_spec = make_pds_rttable_spec_();

        sdk_ret_t ret = SDK_RET_OK;
        if (op_create_) {
            if (!PDS_MOCK_MODE()) {
                ret = pds_vpc_create(&vpc_spec, bctxt);
            }
            if ((ret == SDK_RET_OK) && (!PDS_MOCK_MODE())) {
                ret = pds_route_table_create(&rttbl_spec, bctxt);
            }
        } else {
            if (!PDS_MOCK_MODE()) {
                ret = pds_vpc_update(&vpc_spec, bctxt);
            }
            // Route table cannot be updated for the VPC
        }
        if (unlikely (ret != SDK_RET_OK)) {
            throw Error(std::string("PDS VPC Create or Update failed for MS VRF ")
                        .append(std::to_string(ips_info_.vrf_id))
                        .append(" err=").append(std::to_string(ret)));
        }
    }
    return bctxt_guard_;
}

pds_batch_ctxt_guard_t li_vrf_t::prepare_pds(state_t::context_t& state_ctxt,
                                             bool async) {

    auto& pds_spec = store_info_.vpc_obj->properties().vpc_spec;
    SDK_TRACE_INFO ("VPC %d VNI %d",
                    pds_spec.key.id, pds_spec.fabric_encap.val.vnid);

    auto pds_bctxt_guard = make_batch_pds_spec_(async); 

    // If we have batched multiple IPS earlier, flush it now
    // Cannot add VPC Create/Update to an existing batch
    state_ctxt.state()->flush_outstanding_pds_batch();
    return pds_bctxt_guard;
}

void li_vrf_t::handle_add_upd_ips(ATG_LIPI_VRF_ADD_UPDATE* vrf_add_upd_ips) {
    vrf_add_upd_ips->return_code = ATG_OK;
    parse_ips_info_(vrf_add_upd_ips);
    pds_ms::cookie_t* cookie;

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = pds_ms::state_t::thread_context();
        fetch_store_info_(state_ctxt.state());

        // Ensure that the cached vpc_spec is still valid
        if (unlikely((store_info_.vpc_obj == nullptr) ||
                     (store_info_.vpc_obj->properties().spec_invalid))) {
            // The prev VRF IPS response could have possibly been delayed
            // beyond VPC Spec delete - Ignore and return success.
            // Delete is on the way
            SDK_TRACE_INFO ("VRF %d: VRF AddUpd IPS for unknown VRF",
                            ips_info_.vrf_id);
            return;
        }
        if (op_create_) {
            SDK_TRACE_INFO ("MS VRF %d: Create IPS", ips_info_.vrf_id);
        } else {
            SDK_TRACE_INFO ("MS VRF %d: Update IPS", ips_info_.vrf_id);
        }

        auto l_op_create = op_create_;
        auto l_vrf_id = ips_info_.vrf_id;
        auto l_rttbl_key =
                store_info_.vpc_obj->properties().vpc_spec.v4_route_table;
        cookie_uptr_.reset(new cookie_t);
        auto pds_bctxt_guard = prepare_pds(state_ctxt, true /* async */);

        cookie_uptr_->send_ips_reply = 
            [l_op_create, vrf_add_upd_ips, l_vrf_id, l_rttbl_key] 
                     (bool pds_status, bool ips_mock) -> void {
            // ----------------------------------------------------------------
            // This block is executed asynchronously when PDS response is rcvd
            // ----------------------------------------------------------------
            if (!pds_status && l_op_create) {
                // Create failed - Erase the VRF Store Obj
                // Enter thread-safe context to access/modify global state
                SDK_TRACE_DEBUG ("MS VRF %d: VRF Create failed "
                                 "- delete store obj ", l_vrf_id);
                auto state_ctxt = pds_ms::state_t::thread_context();
                auto vpc_obj = state_ctxt.state()->vpc_store().get(l_vrf_id);
                if (vpc_obj != nullptr) {
                    vpc_obj->properties().hal_created = false;
                }
            }
            if (pds_status && l_op_create) {
                // Create the route table store for this VRF
                auto state_ctxt = pds_ms::state_t::thread_context();
                state_ctxt.state()->route_table_store().add_upd(l_vrf_id,
                   new route_table_obj_t((pds_route_table_key_t &)l_rttbl_key));
            }
            if (unlikely(ips_mock)) return; // UT

            NBB_CREATE_THREAD_CONTEXT
            NBS_ENTER_SHARED_CONTEXT(li_proc_id);
            NBS_GET_SHARED_DATA();

            auto key = li::Vrf::get_key(*vrf_add_upd_ips);
            auto& vrf_store = li::Fte::get().get_lipi_join()->get_vrf_store();
            auto it = vrf_store.find(key);
            if (it == vrf_store.end()) {
                auto send_response = 
                    li::Vrf::set_ips_rc(&vrf_add_upd_ips->ips_hdr,
                                        (pds_status) ? ATG_OK : ATG_UNSUCCESSFUL);
                SDK_ASSERT(send_response);
                SDK_TRACE_DEBUG ("MS VRF %d: Send Async IPS "
                                "reply %s stateless mode",
                                l_vrf_id, (pds_status) ? "Success" : "Failure");
                li::Fte::get().get_lipi_join()->
                    send_ips_reply(&vrf_add_upd_ips->ips_hdr);
            } else {
                if (pds_status) {
                    SDK_TRACE_DEBUG("MS VRF %d: Send Async IPS "
                                    "Reply success stateful mode", l_vrf_id);
                    (*it)->update_complete(ATG_OK);
                } else {
                    SDK_TRACE_DEBUG("MS VRF %d: Send Async IPS "
                                    "Reply failure stateful mode", l_vrf_id);
                    (*it)->update_failed(ATG_UNSUCCESSFUL);
                }
            }
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
            throw Error(std::string("Batch commit failed for Add-Update MS VRF ")
                        .append(std::to_string(ips_info_.vrf_id))
                        .append(" err=").append(std::to_string(ret)));
        }

        // Set the HAL created flag into the Store to ensure that subsequent
        // updates are sent to HAL as PDS updates rather than PDS creates
        if (op_create_) {
            store_info_.vpc_obj->properties().hal_created = true;
        }
    } // End of state thread_context
      // Do Not access/modify global state after this

    vrf_add_upd_ips->return_code = ATG_ASYNC_COMPLETION;
    SDK_TRACE_DEBUG ("MS VRF  %d: Add/Upd PDS Batch commit successful", 
                     ips_info_.vrf_id);
    
    if (PDS_MOCK_MODE()) {
        // Call the HAL callback in PDS mock mode
        std::thread cb(pds_ms::hal_callback, SDK_RET_OK, cookie);
        cb.detach();
    }
}

// API for Direct Fastpath update from MGMT stub to HAL stub bypassing
// Metaswitch controlplane. Requires Synchronous HAL update completion
sdk_ret_t li_vrf_t::update_pds_synch(state_t::context_t&& in_state_ctxt,
                                     vpc_obj_t* vpc_obj) {
    pds_batch_ctxt_guard_t  pds_bctxt_guard;

    { // Continue thread-safe context passed in to access/modify global state
        auto state_ctxt (std::move(in_state_ctxt));
        ips_info_.vrf_id = vpc_obj->properties().vrf_id;
        store_info_.vpc_obj = vpc_obj;

        if (unlikely(!store_info_.vpc_obj->properties().hal_created)) {
            // LI VRF has not created the VPC in PDS HAL yet.
            // When the PDS Create is pushed, LI VRF will use the latest cached
            // VPC spec
            SDK_TRACE_DEBUG("MS VRF  %d: Ignore Direct Update before VRF Create", 
                            ips_info_.vrf_id);
            return SDK_RET_OK;
        }
        SDK_TRACE_DEBUG("MS VRF  %d: Received Direct Update for VRF", ips_info_.vrf_id);
        pds_bctxt_guard = prepare_pds(state_ctxt, false /* synchronous */);

        // This is a synchronous batch commit.
        // Ensure that state lock is released to avoid blocking NBASE thread
    } // End of state thread_context. Do Not access/modify global state

    auto ret = pds_batch_commit(pds_bctxt_guard.release());
    if (unlikely (ret != SDK_RET_OK)) {
        SDK_TRACE_ERR ("MS VRF %d: Add/Upd PDS Direct Update Batch commit"
                       "failed %d", ips_info_.vrf_id, ret);
        return ret;
    }

    SDK_TRACE_DEBUG ("MS VRF  %d: Add/Upd PDS Direct Update Batch commit successful", 
                     ips_info_.vrf_id);
    return SDK_RET_OK;
}

void li_vrf_t::handle_delete(const NBB_BYTE* vrf_name, NBB_ULONG vrf_name_len) {
    pds_batch_ctxt_guard_t  pds_bctxt_guard;
    op_delete_ = true;

    // MS stub Integration APIs do not support Async callback for deletes.
    // However since we should not block the MS NBase main thread
    // the HAL processing is always asynchronous even for deletes. 
    // Assuming that Deletes never fail the Store is also updated
    // in a synchronous fashion for deletes so that it is in sync
    // if there is a subsequent create from MS.

    ips_info_.vrf_id = vrfname_2_vrfid(vrf_name, vrf_name_len);
    SDK_TRACE_INFO ("MS VRF %d: Delete IPS", ips_info_.vrf_id);

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = pds_ms::state_t::thread_context();
        fetch_store_info_(state_ctxt.state());

        // Empty cookie to force async PDS.
        cookie_uptr_.reset (new cookie_t);
        pds_bctxt_guard = make_batch_pds_spec_ (true /* async */); 

        // If we have batched multiple IPS earlier flush it now
        // Cannot add VPC Delete to an existing batch
        state_ctxt.state()->flush_outstanding_pds_batch();

        // Delete the VRF route table
        state_ctxt.state()->route_table_store().erase(ips_info_.vrf_id);
        state_ctxt.state()->vpc_store().erase(ips_info_.vrf_id);
    } // End of state thread_context
      // Do Not access/modify global state after this

    auto vrf_id = ips_info_.vrf_id;
    cookie_uptr_->send_ips_reply = 
        [vrf_id] (bool pds_status, bool test) -> void {
            // ----------------------------------------------------------------
            // This block is executed asynchronously when PDS response is rcvd
            // ----------------------------------------------------------------
            SDK_TRACE_DEBUG("MS VRF %d Delete: Rcvd Async PDS response %s",
                            vrf_id, (pds_status) ? "Success" : "Failure");
        };
    // All processing complete, only batch commit remains - 
    // safe to release the cookie_uptr_ unique_ptr
    auto cookie = cookie_uptr_.release();
    auto ret = pds_batch_commit(pds_bctxt_guard.release());
    if (unlikely (ret != SDK_RET_OK)) {
        delete cookie;
        throw Error(std::string("Batch commit failed for delete MS VRF ")
                    .append(std::to_string(ips_info_.vrf_id))
                    .append(" err=").append(std::to_string(ret)));
    }
    SDK_TRACE_DEBUG ("MS VRF %d: Delete PDS Batch commit successful", ips_info_.vrf_id);
}

sdk_ret_t
li_vrf_update_pds_synch (state_t::context_t&& state_ctxt, vpc_obj_t* vpc_obj)
{
    try {
        li_vrf_t vrf;
        return vrf.update_pds_synch(std::move(state_ctxt), vpc_obj);
    } catch (Error& e) {
        SDK_TRACE_ERR ("VRF Add Update processing failed %s", e.what());
        return SDK_RET_ERR;
    }
}

} // End namespace
