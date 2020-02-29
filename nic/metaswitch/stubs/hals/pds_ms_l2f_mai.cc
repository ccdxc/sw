//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS integration of Metaswitch L2F MAI stub
//---------------------------------------------------------------

#include <l2f_c_includes.hpp>
#include "nic/metaswitch/stubs/hals/pds_ms_l2f_mai.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_hal_init.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_l2f_utils.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/apollo/api/internal/pds_mapping.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include <l2f_integ_api.hpp>
#include <thread>

extern int l2f_proc_id;

//-------------------------------------------------------------------
//       MS obj                  PDS HAL Spec
// a) Pathset containing
//    L3 interfaces  -> Underlay ECMP NHs (referenced by TEP entries)
// b) VXLAN Tunnel   -> TEP entry -> Overlay ECMP entry (ref by Type2 MAC,IP)
// c) L2 VXLAN Port  -> Unused (since Type 2 VNI comes from Egress BD)
// d) L3 VXLAN Port  -> TEP,VNI entry (referenced by Type5 Overlay ECMP)
// e) Pathset containing
//    L3 VXLAN Ports -> Overlay ECMP entry (ref by Type5 Prefix routes)
//--------------------------------------------------------------------

namespace pds_ms {

void
l2f_local_mac_ip_add (const pds_obj_key_t& subnet_key, const ip_addr_t& ip,
                      mac_addr_t mac, pds_ifindex_t lif_ifindex)
{
    ms_ifindex_t ms_lif_index = pds_to_ms_ifindex(lif_ifindex, IF_TYPE_LIF);
    ms_bd_id_t bd_id;

    { // Enter Mgmt thread context
        auto mgmt_ctxt = mgmt_state_t::thread_context();
        auto uuid_obj = mgmt_ctxt.state()->lookup_uuid(subnet_key);
        if (uuid_obj == nullptr) {
            PDS_TRACE_ERR("Received MAC %s IP %s learn for unknown Subnet %s",
                          macaddr2str(mac), ipaddr2str(&ip), subnet_key.str());
            return;
        }
        if (uuid_obj->obj_type() != uuid_obj_type_t::SUBNET) {
            PDS_TRACE_ERR("Received MAC %s IP %s learn with invalid UUID %s of type %s",
                          macaddr2str(mac), ipaddr2str(&ip), subnet_key.str(),
                          uuid_obj_type_str(uuid_obj->obj_type()));
            return;
        }
        bd_id = ((subnet_uuid_obj_t*) uuid_obj)->ms_id();
    } // Exit Mgmt thread context

    if (ip_addr_is_zero(&ip)) {
        PDS_TRACE_DEBUG("Advertise MAC learn for Subnet %s BD %d MAC %s LIF 0x%x MS-LIF 0x%x",
                        subnet_key.str(), bd_id, macaddr2str(mac), lif_ifindex, ms_lif_index);
    } else {
        PDS_TRACE_DEBUG("Advertise IP-MAC learn for Subnet %s BD %d IP %s MAC %s"
                        " LIF 0x%x MS-LIF 0x%x",
                        subnet_key.str(), bd_id, ipaddr2str(&ip), macaddr2str(mac),
                        lif_ifindex, ms_lif_index);
    }

    NBB_CREATE_THREAD_CONTEXT
    NBS_ENTER_SHARED_CONTEXT(l2f_proc_id);
    NBS_GET_SHARED_DATA();

    ATG_MAI_MAC_IP_ID mac_ip_id = {0};
    mac_ip_id.bd_id.bd_type = ATG_L2_BRIDGE_DOMAIN_EVPN;
    mac_ip_id.bd_id.bd_id = bd_id;
    memcpy(mac_ip_id.mac_address, mac, ETH_ADDR_LEN);

    // Notify MAC only first
    auto ret = l2f::l2f_cc_is_mac_add_update(&mac_ip_id, ms_lif_index);
    if (ret != ATG_OK) {
        PDS_TRACE_ERR("Adding local MAC to MS failed for BD %d MAC %s",
                      bd_id, macaddr2str(mac));
    }

    if (!ip_addr_is_zero(&ip)) {
        // Then Notify IP along with MAC
        pds_ms::pds_to_ms_ipaddr(ip, &mac_ip_id.ip_address);
        ret = l2f::l2f_cc_is_mac_add_update(&mac_ip_id, ms_lif_index);
        if (ret != ATG_OK) {
            PDS_TRACE_ERR("Adding local IP-MAC to MS failed for BD %d IP %s MAC %s",
                          bd_id, ipaddr2str(&ip), macaddr2str(mac));
        }
    }

    NBS_RELEASE_SHARED_DATA();
    NBS_EXIT_SHARED_CONTEXT();
    NBB_DESTROY_THREAD_CONTEXT
}

void
l2f_local_mac_ip_del (const pds_obj_key_t& subnet_key, const ip_addr_t& ip,
                      mac_addr_t mac)
{
    ms_bd_id_t bd_id;
    { // Enter Mgmt thread context
        auto mgmt_ctxt = mgmt_state_t::thread_context();
        auto uuid_obj = mgmt_ctxt.state()->lookup_uuid(subnet_key);
        if (uuid_obj == nullptr) {
            PDS_TRACE_ERR("Received MAC %s IP %s age for unknown Subnet %s",
                          macaddr2str(mac), ipaddr2str(&ip), subnet_key.str());
            return;
        }
        if (uuid_obj->obj_type() != uuid_obj_type_t::SUBNET) {
            PDS_TRACE_ERR("Received MAC %s IP %s age with invalid UUID %s of type %s",
                          macaddr2str(mac), ipaddr2str(&ip), subnet_key.str(),
                          uuid_obj_type_str(uuid_obj->obj_type()));
            return;
        }
        bd_id = ((subnet_uuid_obj_t*) uuid_obj)->ms_id();
    } // Exit Mgmt thread context

    if (ip_addr_is_zero(&ip)) {
        PDS_TRACE_DEBUG("Received MAC remove for Subnet %s BD %d MAC %s",
                        subnet_key.str(), bd_id, macaddr2str(mac));
    } else {
        PDS_TRACE_DEBUG("Received MAC-IP remove for Subnet %s BD %d IP %s MAC %s",
                        subnet_key.str(), bd_id, ipaddr2str(&ip), macaddr2str(mac));
    }

    NBB_CREATE_THREAD_CONTEXT
    NBS_ENTER_SHARED_CONTEXT(l2f_proc_id);
    NBS_GET_SHARED_DATA();

    ATG_MAI_MAC_IP_ID mac_ip_id = {0};
    mac_ip_id.bd_id.bd_type = ATG_L2_BRIDGE_DOMAIN_EVPN;
    mac_ip_id.bd_id.bd_id = bd_id;
    memcpy(mac_ip_id.mac_address, mac, ETH_ADDR_LEN);
    if (!ip_addr_is_zero(&ip)) {
        pds_ms::pds_to_ms_ipaddr(ip, &mac_ip_id.ip_address);
    }
    l2f::l2f_cc_is_mac_delete(mac_ip_id);

    NBS_RELEASE_SHARED_DATA();
    NBS_EXIT_SHARED_CONTEXT();
    NBB_DESTROY_THREAD_CONTEXT
}

#define BDPI_GET_FIRST_LBL_INTF(ips, list_ptr) \
       (ATG_BDPI_LABELLED_INTERFACE*) \
            NTL_OFF_LIST_TRAVERSE((ips), (list_ptr), NULL)

#define BDPI_GET_NEXT_LBL_INTF(ips, list_ptr, cur_obj) \
       (ATG_BDPI_LABELLED_INTERFACE*) \
            NTL_OFF_LIST_TRAVERSE((ips), (list_ptr), cur_obj)

void l2f_mai_t::parse_ips_info_(ATG_BDPI_UPDATE_FDB_MAC* fdb_mac) {
    ips_info_.bd_id = fdb_mac->bd_id.bd_id;
    MAC_ADDR_COPY(ips_info_.mac_address, fdb_mac->mac_address);
    ips_info_.has_ip = false;

    auto list_p = &fdb_mac->labelled_interfaces;
    for (auto lbl_intf = BDPI_GET_FIRST_LBL_INTF(fdb_mac, list_p);
         lbl_intf != NULL;
         lbl_intf = BDPI_GET_NEXT_LBL_INTF(fdb_mac, list_p, lbl_intf)) {
        ip_addr_t tep_ip;
        ms_to_pds_ipaddr(lbl_intf->dest_address, &tep_ip);
        ips_info_.tep_ip_list.emplace(tep_ip);
    }
}

void l2f_mai_t::parse_ips_info_(const ATG_MAI_MAC_IP_ID* mai_ip) {
    ips_info_.bd_id = mai_ip->bd_id.bd_id;
    MAC_ADDR_COPY(ips_info_.mac_address, mai_ip->mac_address);
    ms_to_pds_ipaddr(mai_ip->ip_address, &ips_info_.ip_address);
    ips_info_.has_ip = true;
}

void l2f_mai_t::fetch_store_info_(state_t* state) {
    store_info_.bd_obj = state->bd_store().get(ips_info_.bd_id);
    SDK_ASSERT(store_info_.bd_obj != nullptr);
    if (ips_info_.has_ip || !op_delete_) {
        store_info_.mac_obj = store_info_.bd_obj->mac_store().get
            (mac_obj_t::key_t(ips_info_.bd_id, ips_info_.mac_address));
        store_info_.subnet_obj = state->subnet_store().get(ips_info_.bd_id);
    }
}

void l2f_mai_t::resolve_teps_(state_t* state) {
    if (!ips_info_.has_ip) {
        // FDB update - copy the new destination TEP(s)
        // TODO: Handle Overlay ECMP update
        // For now assuming only a single TEP and overwriting previous
        // Associated MAC IPs will be deleted - so no need to modify them here
        store_info_.mac_obj->tep_ip_list = ips_info_.tep_ip_list;
    }
    for (auto& tep_ip: store_info_.mac_obj->tep_ip_list) {
        auto tep_obj = state->tep_store().get(tep_ip);
        SDK_ASSERT(tep_obj != nullptr);
        store_info_.tep_obj_list.push_back(tep_obj);
    }
    // For now assuming only a single TEP to get ECMP index
    store_info_.hal_oecmp_idx =
        store_info_.tep_obj_list.back()->hal_oecmp_idx_guard->idx();
}

pds_mapping_key_t l2f_mai_t::make_pds_mapping_key_(void) {
    pds_mapping_key_t key;
    memset(&key, 0, sizeof(key));
    if (ips_info_.has_ip) {
        key.type = PDS_MAPPING_TYPE_L3;
        key.vpc = store_info_.bd_obj->properties().vpc;
        key.ip_addr = ips_info_.ip_address;
    } else {
        key.type = PDS_MAPPING_TYPE_L2;
        key.subnet = store_info_.bd_obj->properties().subnet;
        MAC_ADDR_COPY(key.mac_addr, ips_info_.mac_address);
    }
    return key;
}

pds_remote_mapping_spec_t l2f_mai_t::make_pds_mapping_spec_(void) {
    pds_remote_mapping_spec_t spec;
    memset(&spec, 0, sizeof(spec));
    // TODO: fix this !!!
    //spec.key = ; // generate uuid on the fly
    spec.skey = make_pds_mapping_key_();
    spec.subnet = store_info_.subnet_obj->spec().key;
    spec.nh_type = PDS_NH_TYPE_OVERLAY_ECMP;
    spec.nh_group = msidx2pdsobjkey(store_info_.hal_oecmp_idx);
    if(ips_info_.has_ip) {
        MAC_ADDR_COPY(spec.vnic_mac,ips_info_.mac_address);
    }
    return spec;
}

void l2f_mai_t::add_pds_mapping_spec_(pds_batch_ctxt_t bctxt) {
    auto mapping_spec = make_pds_mapping_spec_();
    sdk_ret_t ret;
    if (PDS_MOCK_MODE()) {
        return;
    }
    if (op_create_) {
        ret = pds_remote_mapping_create(&mapping_spec, bctxt);
    } else {
        ret = pds_remote_mapping_update(&mapping_spec, bctxt);
    }
    if (likely (ret == SDK_RET_OK)) {
        return;
    }
    if (ips_info_.has_ip) {
        throw Error(std::string("PDS Remote Mapping Create or Update failed for MS BD ")
                    .append(std::to_string(ips_info_.bd_id))
                    .append(" MAC ").append(macaddr2str(ips_info_.mac_address))
                    .append(" IP ").append(ipaddr2str(&ips_info_.ip_address))
                    .append(" err=").append(std::to_string(ret)));
    } else {
        throw Error(std::string("PDS Remote Mapping Create or Update failed for MS BD ")
                    .append(std::to_string(ips_info_.bd_id))
                    .append(" MAC ").append(macaddr2str(ips_info_.mac_address))
                    .append(" err=").append(std::to_string(ret)));
    }
}

pds_batch_ctxt_guard_t l2f_mai_t::make_batch_pds_spec_() {
    pds_batch_ctxt_guard_t bctxt_guard_;
    SDK_ASSERT(cookie_uptr_); // Cookie should not be empty

    // TODO Batch multiple PDS Remote Mapping calls instead of creating
    // a new PDS Batch commit for every MAC-IP HAL Stub invocation
    pds_batch_params_t bp {PDS_BATCH_PARAMS_EPOCH,
                           PDS_BATCH_PARAMS_ASYNC,
                           hal_callback,
                           cookie_uptr_.get()};
    auto bctxt = pds_batch_start(&bp);

    if (unlikely (!bctxt)) {
        throw Error(std::string("PDS Batch Start failed for MS BD ")
                    .append(std::to_string(ips_info_.bd_id)));
    }
    bctxt_guard_.set(bctxt);

    if (op_delete_) { // Delete
        auto key = make_pds_mapping_key_();
        if (!PDS_MOCK_MODE()) {
            api::pds_remote_mapping_delete(&key, bctxt);
        }
    } else { // Add or update
        add_pds_mapping_spec_(bctxt);
    }
    return bctxt_guard_;
}

NBB_BYTE l2f_mai_t::handle_add_upd_mac(ATG_BDPI_UPDATE_FDB_MAC* update_fdb_mac) {
    NBB_BYTE rc = ATG_OK;
    pds_batch_ctxt_guard_t  pds_bctxt_guard;
    cookie_t *cookie = nullptr;
    parse_ips_info_(update_fdb_mac);

    PDS_TRACE_INFO("L2F FDB ADD BD %d MAC %s Num TEPs %d First-TEP %s",
                   ips_info_.bd_id, macaddr2str(ips_info_.mac_address),
                   ips_info_.tep_ip_list.size(),
                   ipaddr2str(&(*ips_info_.tep_ip_list.begin())));

    //----------------------------------------------------------------------
    // Unlike other stub integration components, MAI integration comp
    // adds the Remote MAC to the store immediately without waiting for
    // async HAL response since the MAC store is accessed for MAI IP updates
    // from Metaswitch which can happen in parallel
    //----------------------------------------------------------------------

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = state_t::thread_context();
        fetch_store_info_(state_ctxt.state());

        if (store_info_.subnet_obj == nullptr) {
            PDS_TRACE_DEBUG("Missing Subnet Obj for MAI BD %d MAC %s add-upd",
                            ips_info_.bd_id, macaddr2str(ips_info_.mac_address));
            return rc;
        }
        if (store_info_.mac_obj == nullptr) {
            // New FDB entry - Populate cache
            mac_obj_t::key_t key (ips_info_.bd_id, ips_info_.mac_address);
            std::unique_ptr<mac_obj_t> mac_obj_uptr
                (new mac_obj_t(key));
            store_info_.mac_obj = mac_obj_uptr.get();
            store_info_.bd_obj->mac_store().
                add_upd(key, std::move(mac_obj_uptr));
            op_create_ = true;
        } else if (!store_info_.mac_obj->hal_created) {
            // MAC store obj could have been created internally
            // when MS MAI Stub sent IP out-of-seq before MAC.
            // Create everything now
            op_create_ = true;
        }
        resolve_teps_(state_ctxt.state());

        cookie_uptr_.reset(new cookie_t);
        pds_bctxt_guard = make_batch_pds_spec_();

        // Check if there are IP addresses associated
        // Add to batch
        for (auto ip_address: store_info_.mac_obj->out_of_seq_ip) {
            ips_info_.ip_address = ip_address;
            ips_info_.has_ip = true;
            PDS_TRACE_DEBUG("PDS Create Remote Mapping for out-of-seq IP %s",
                            ipaddr2str(&ip_address));
            add_pds_mapping_spec_(pds_bctxt_guard.get());
        }
        ips_info_.has_ip = false;

        auto l_bd_id = ips_info_.bd_id;
        auto l_op_create = op_create_;
        mac_addr_t l_mac;
        MAC_ADDR_COPY(l_mac, update_fdb_mac->mac_address);

        cookie_uptr_->send_ips_reply =
            [update_fdb_mac, l_bd_id, l_mac, l_op_create] (bool pds_status,
                                                           bool ips_mock) -> void {
                // ----------------------------------------------------------------
                // This block is executed asynchronously when PDS response is rcvd
                // ----------------------------------------------------------------
                PDS_TRACE_DEBUG("+++++++++ MS BD %d MAC %s: MAC AddUpd Rcvd Async PDS "
                                "response %s ++++++++++", l_bd_id, macaddr2str(l_mac),
                                (pds_status) ? "Success" : "Failure");

                if (!pds_status && l_op_create) {
                    do {
                        // Enter thread-safe context to access/modify global state
                        auto state_ctxt = state_t::thread_context();
                        // Create failed - Reset the HAL created flag
                        auto bd_obj = state_ctxt.state()->bd_store().
                            get(update_fdb_mac->bd_id.bd_id);
                        if (!bd_obj) break;
                        auto mac_obj = bd_obj->mac_store().get({l_bd_id, l_mac});
                        if (!mac_obj) break;
                        mac_obj->hal_created = false;
                    } while(0);
                }

                if (unlikely(ips_mock)) return; // UT

                NBB_CREATE_THREAD_CONTEXT
                NBS_ENTER_SHARED_CONTEXT(l2f_proc_id);
                NBS_GET_SHARED_DATA();

                do {
                    auto bdpi_join = get_l2f_bdpi_join();
                    if (bdpi_join == nullptr) {
                        PDS_TRACE_ERR("Failed to find BDPI join to return BD %d AddUpd IPS",
                                      update_fdb_mac->bd_id);
                        break;
                    }
                    auto& fdb_store = bdpi_join->get_fdb_mac_store();

                    auto key = l2f::FdbMac::get_key(*update_fdb_mac);
                    auto it = fdb_store.find(key);
                    if (it == fdb_store.end()) {
                        auto send_response =
                            l2f::FdbMac::set_ips_rc(&update_fdb_mac->ips_hdr,
                                                    (pds_status) ? ATG_OK : ATG_UNSUCCESSFUL);
                        SDK_ASSERT(send_response);
                        PDS_TRACE_DEBUG ("++++++++ MS BD %d MAC %s: Send Async IPS "
                                         "reply %s stateless mode +++++++++++",
                                         l_bd_id, macaddr2str(l_mac),
                                         (pds_status) ? "Success" : "Failure");
                        bdpi_join->send_ips_reply(&update_fdb_mac->ips_hdr);
                    } else {
                        if (pds_status) {
                            PDS_TRACE_DEBUG ("MS BD %d MAC %s: Send Async IPS "
                                             "Reply success stateful mode",
                                             l_bd_id, macaddr2str(l_mac));
                            (*it)->update_complete(ATG_OK);
                        } else {
                            PDS_TRACE_DEBUG ("MS BD %d MAC %s: Send Async IPS "
                                             "Reply failure stateful mode",
                                             l_bd_id, macaddr2str(l_mac));
                            (*it)->update_failed(ATG_UNSUCCESSFUL);
                        }
                    }
                }
                while (0);
                NBS_RELEASE_SHARED_DATA();
                NBS_EXIT_SHARED_CONTEXT();
                NBB_DESTROY_THREAD_CONTEXT
            };

        // All processing complete, only batch commit remains -
        // safe to release the cookie_uptr_ unique_ptr
        rc = ATG_ASYNC_COMPLETION;
        cookie = cookie_uptr_.release();
        auto ret = pds_batch_commit(pds_bctxt_guard.release());
        if (unlikely (ret != SDK_RET_OK)) {
            delete cookie;
            throw Error(std::string("Batch commit failed for MAC Add BD ")
                        .append(std::to_string(l_bd_id))
                        .append(" MAC ").append(macaddr2str(l_mac))
                        .append(" err ").append(std::to_string(ret)));
        }

        store_info_.mac_obj->out_of_seq_ip.clear();
        if (op_create_) {store_info_.mac_obj->hal_created = true;}
    } // End of state thread_context
      // Do Not access/modify global state after this

    PDS_TRACE_DEBUG("MS BD %d MAC %s Add PDS Batch commit successful",
                    ips_info_.bd_id, macaddr2str(ips_info_.mac_address));
    if (PDS_MOCK_MODE()) {
        // Call the HAL callback in PDS mock mode
        std::thread cb(pds_ms::hal_callback, SDK_RET_OK, cookie);
        cb.detach();
    }
    return rc;
}

void l2f_mai_t::handle_delete_mac(l2f::FdbMacKey *key) {
    pds_batch_ctxt_guard_t  pds_bctxt_guard;
    op_delete_ = true;

    // Populate IPS info
    ips_info_.bd_id = key->bd_id.bd_id;
    MAC_ADDR_COPY(ips_info_.mac_address, key->mac_address);
    ips_info_.has_ip = false;

    PDS_TRACE_INFO("L2F FDB DEL BD %d MAC %s", key->bd_id.bd_id,
                   macaddr2str(key->mac_address));

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = state_t::thread_context();
        fetch_store_info_(state_ctxt.state());
        store_info_.bd_obj->mac_store().erase({ips_info_.bd_id,
                                               ips_info_.mac_address});
    } // End of state thread_context
      // Do Not access/modify global state after this

    cookie_uptr_.reset(new cookie_t);
    pds_bctxt_guard = make_batch_pds_spec_();

    ms_bd_id_t l_bd_id = ips_info_.bd_id;
    mac_addr_t l_mac;
    MAC_ADDR_COPY(l_mac, key->mac_address);

    cookie_uptr_->send_ips_reply =
        [l_bd_id, l_mac] (bool pds_status, bool ips_mock) -> void {
            // ----------------------------------------------------------------
            // This block is executed asynchronously when PDS response is rcvd
            // ----------------------------------------------------------------
            PDS_TRACE_DEBUG("++++++++ MS BD %d MAC %s: MAC Del Rcvd Async PDS"
                            " response %s +++++++++++",
                            l_bd_id, macaddr2str(l_mac),
                            (pds_status) ? "Success" : "Failure");

        };
    // All processing complete, only batch commit remains -
    // safe to release the cookie_uptr_ unique_ptr
    auto cookie = cookie_uptr_.release();
    auto ret = pds_batch_commit(pds_bctxt_guard.release());
    if (unlikely (ret != SDK_RET_OK)) {
        delete cookie;
        throw Error(std::string("Batch commit failed for MAC Del BD ")
                    .append(std::to_string(l_bd_id))
                    .append(" MAC ").append(macaddr2str(l_mac))
                    .append(" err ").append(std::to_string(ret)));
    }
    PDS_TRACE_DEBUG("MS BD %d MAC %s Del PDS Batch commit successful",
                    l_bd_id, macaddr2str(l_mac));
}

void l2f_mai_t::handle_add_upd_ip(const ATG_MAI_MAC_IP_ID* mai_ip_id) {
    pds_batch_ctxt_guard_t  pds_bctxt_guard;

    parse_ips_info_(mai_ip_id);

    PDS_TRACE_INFO("L2F MAC IP ADD BD %d IP %s MAC %s",
                   ips_info_.bd_id, ipaddr2str(&ips_info_.ip_address),
                   macaddr2str(ips_info_.mac_address));

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = state_t::thread_context();
        fetch_store_info_(state_ctxt.state());

        if (store_info_.subnet_obj == nullptr) {
            PDS_TRACE_DEBUG("Missing Subnet Obj for MAI BD %d IP %s MAC %s add-upd",
                            ips_info_.bd_id, ipaddr2str(&ips_info_.ip_address),
                            macaddr2str(ips_info_.mac_address));
            return;
        }
        // Check out-of-seq: IP ahead of MAC from MS L2F stub
        if (store_info_.mac_obj == nullptr) {
            // MAC FDB entry does not exist for this IP - cache now
            // and revisit later when MAC is received from Metaswitch
            PDS_TRACE_DEBUG("Saving out-of-seq IP %s MAC %s(unknown) for later",
                            ipaddr2str(&ips_info_.ip_address),
                            macaddr2str(ips_info_.mac_address));
            mac_obj_t::key_t key (ips_info_.bd_id, ips_info_.mac_address);
            std::unique_ptr<mac_obj_t> mac_obj_uptr (new mac_obj_t (key));
            store_info_.mac_obj = mac_obj_uptr.get();
            store_info_.bd_obj->mac_store().
                add_upd(key, std::move(mac_obj_uptr));
            store_info_.mac_obj->out_of_seq_ip.push_back(ips_info_.ip_address);
            return;
        } else if (store_info_.mac_obj->tep_ip_list.empty()) {
            PDS_TRACE_DEBUG("Saving out-of-seq IP %s MAC %s for later",
                            ipaddr2str(&ips_info_.ip_address),
                            macaddr2str(ips_info_.mac_address));
            store_info_.mac_obj->out_of_seq_ip.push_back(ips_info_.ip_address);
            return;
        }

        // Cannot detect Update of existing Remote Mapping IP entry
        // PDS HAL accepts Create for existing Remote Mapping entries
        op_create_ = true;

        resolve_teps_(state_ctxt.state());
        cookie_uptr_.reset(new cookie_t);

        pds_bctxt_guard = make_batch_pds_spec_();

    } // End of state thread_context
      // Do Not access/modify global state after this

    auto l_bd_id = ips_info_.bd_id;
    auto l_ip = ips_info_.ip_address;

    cookie_uptr_->send_ips_reply =
        [l_bd_id, l_ip] (bool pds_status, bool ips_mock) -> void {
            // ----------------------------------------------------------------
            // This block is executed asynchronously when PDS response is rcvd
            // ----------------------------------------------------------------
            PDS_TRACE_DEBUG("++++++++++ MS BD %d IP %s: Remote IP AddUpd Rcvd Async"
                            " PDS response %s ++++++++++",
                            l_bd_id, ipaddr2str(&l_ip),
                            (pds_status) ? "Success" : "Failure");

        };

    // All processing complete, only batch commit remains -
    // safe to release the cookie_uptr_ unique_ptr
    auto cookie = cookie_uptr_.release();
    auto ret = pds_batch_commit(pds_bctxt_guard.release());
    if (unlikely (ret != SDK_RET_OK)) {
        delete cookie;
        throw Error(std::string("Batch commit failed for Remote IP Add BD ")
                    .append(std::to_string(l_bd_id))
                    .append(" IP ").append(ipaddr2str(&l_ip))
                    .append(" err ").append(std::to_string(ret)));
    }
    PDS_TRACE_DEBUG("MS BD %d Remote IP %s Add PDS Batch commit successful",
                    l_bd_id, ipaddr2str(&l_ip));
}

void l2f_mai_t::handle_delete_ip(const ATG_MAI_MAC_IP_ID* mai_ip_id) {
    pds_batch_ctxt_guard_t  pds_bctxt_guard;
    op_delete_ = true;

    // Populate IPS info
    parse_ips_info_(mai_ip_id);

    PDS_TRACE_INFO("L2F MAI IP DEL BD %d IP %s", ips_info_.bd_id,
                   ipaddr2str(&ips_info_.ip_address));

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = state_t::thread_context();
        fetch_store_info_(state_ctxt.state());

    } // End of state thread_context
      // Do Not access/modify global state after this

    cookie_uptr_.reset(new cookie_t);
    pds_bctxt_guard = make_batch_pds_spec_();

    auto l_bd_id = ips_info_.bd_id;
    auto l_ip = ips_info_.ip_address;

    cookie_uptr_->send_ips_reply =
        [l_bd_id, l_ip] (bool pds_status, bool ips_mock) -> void {
            // ----------------------------------------------------------------
            // This block is executed asynchronously when PDS response is rcvd
            // ----------------------------------------------------------------
            PDS_TRACE_DEBUG("++++++++ MS BD %d IP %s: Remote IP Delete Rcvd Async"
                            " PDS response %s +++++++++++++",
                            l_bd_id, ipaddr2str(&l_ip),
                            (pds_status) ? "Success" : "Failure");

        };

    // All processing complete, only batch commit remains -
    // safe to release the cookie_uptr_ unique_ptr
    auto cookie = cookie_uptr_.release();
    auto ret = pds_batch_commit(pds_bctxt_guard.release());
    if (unlikely (ret != SDK_RET_OK)) {
        delete cookie;
        throw Error(std::string("Batch commit failed for Remote IP Del BD ")
                    .append(std::to_string(l_bd_id))
                    .append(" IP ").append(ipaddr2str(&l_ip))
                    .append(" err ").append(std::to_string(ret)));
    }
    PDS_TRACE_DEBUG("MS BD %d Remote IP %s Del PDS Batch commit successful",
                    l_bd_id, ipaddr2str(&l_ip));
}

void l2f_mai_t::batch_pds_mapping_del_spec(bd_obj_t* bd_obj,
                                           const mac_addr_t& mac,
                                           pds_batch_ctxt_t bctxt) {
    ips_info_.bd_id = bd_obj->properties().bd_id;
    MAC_ADDR_COPY(ips_info_.mac_address, mac);
    store_info_.bd_obj = bd_obj;
    PDS_TRACE_DEBUG("Append MS BD %d MAC %s Del to PDS Batch",
                    bd_obj->properties().bd_id, macaddr2str(mac));
    auto key = make_pds_mapping_key_();
    if (!PDS_MOCK_MODE()) {
        api::pds_remote_mapping_delete(&key, bctxt);
    }
}

void
l2f_del_remote_macs_for_bd (state_t::context_t& state_ctxt, ms_bd_id_t bd_id,
                            pds_batch_ctxt_t bctxt)
{
    auto bd_obj = state_ctxt.state()->bd_store().get(bd_id);
    bd_obj->walk_macs([bd_obj, bctxt] (const mac_addr_t& mac) -> bool {
                           l2f_mai_t mai;
                           mai.batch_pds_mapping_del_spec(bd_obj, mac, bctxt);
                           return true;
                       });
}

} // End namespace
