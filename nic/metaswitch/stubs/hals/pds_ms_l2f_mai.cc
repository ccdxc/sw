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
#include "nic/apollo/learn/learn_api.hpp"
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
                      mac_addr_t mac, if_index_t lif_ifindex)
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

static void
local_mac_ip_del_ (ms_bd_id_t bd_id, mac_addr_t mac, const ip_addr_t& ip)
{
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
    local_mac_ip_del_(bd_id, mac, ip);
}

void l2f_mai_t::parse_ips_info_(ATG_BDPI_UPDATE_FDB_MAC* fdb_mac) {
    ips_info_.bd_id = fdb_mac->bd_id.bd_id;
    MAC_ADDR_COPY(ips_info_.mac_address, fdb_mac->mac_address);
    ips_info_.has_ip = false;

    auto list_p = &fdb_mac->labelled_interfaces;
    for (auto lbl_intf = MS_LIST_GET_FIRST(fdb_mac, list_p,
                                           ATG_BDPI_LABELLED_INTERFACE);
         lbl_intf != NULL;
         lbl_intf = MS_LIST_GET_NEXT(fdb_mac, list_p, lbl_intf,
                                     ATG_BDPI_LABELLED_INTERFACE)) {
        ip_addr_t tep_ip;
        ms_to_pds_ipaddr(lbl_intf->dest_address, &tep_ip);
        ips_info_.tep_ip_list.emplace(tep_ip);
    }
}

void l2f_mai_t::parse_ips_info_(ATG_MAI_UPDATE_MAC_IP* mai_ip) {
    const ATG_MAI_MAC_IP_ID* mac_ip_id = &(mai_ip->mac_ip_id);

    ips_info_.bd_id = mac_ip_id->bd_id.bd_id;
    MAC_ADDR_COPY(ips_info_.mac_address, mac_ip_id->mac_address);
    ms_to_pds_ipaddr(mac_ip_id->ip_address, &ips_info_.ip_address);
    ips_info_.has_ip = true;

    auto list_p = &mai_ip->vxlan_ports;
    for (auto vxlan_port = MS_LIST_GET_FIRST(mai_ip, list_p,
                                           ATG_MAI_VXLAN_PORT_INFO);
         vxlan_port != NULL;
         vxlan_port = MS_LIST_GET_NEXT(mai_ip, list_p, vxlan_port,
                                     ATG_MAI_VXLAN_PORT_INFO)) {
        ip_addr_t tep_ip;
        ms_to_pds_ipaddr(vxlan_port->dest_address, &tep_ip);
        ips_info_.tep_ip_list.emplace(tep_ip);
    }
}

void l2f_mai_t::parse_ips_info_(const ATG_MAI_MAC_IP_ID* mac_ip_id) {

    ips_info_.bd_id = mac_ip_id->bd_id.bd_id;
    MAC_ADDR_COPY(ips_info_.mac_address, mac_ip_id->mac_address);
    ms_to_pds_ipaddr(mac_ip_id->ip_address, &ips_info_.ip_address);
    ips_info_.has_ip = true;
}

void l2f_mai_t::fetch_store_info_(state_t* state) {
    store_info_.bd_obj = state->bd_store().get(ips_info_.bd_id);
    SDK_ASSERT(store_info_.bd_obj != nullptr);
}

void l2f_mai_t::resolve_teps_(state_t* state) {
    SDK_ASSERT(!ips_info_.tep_ip_list.empty());
    // For now assuming only a single TEP to get ECMP index
    auto tep_obj = state->tep_store().get(*ips_info_.tep_ip_list.begin());
    store_info_.hal_oecmp_idx = tep_obj->hal_oecmp_idx_guard->idx();
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
    spec.subnet = store_info_.bd_obj->properties().subnet;
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

    PDS_TRACE_INFO("L2F FDB ADD BD %d MAC %s Num TEPs %lu First-TEP %s",
                   ips_info_.bd_id, macaddr2str(ips_info_.mac_address),
                   ips_info_.tep_ip_list.size(),
                   ipaddr2str(&(*ips_info_.tep_ip_list.begin())));

    // Clean up local MAC from MS store so that any subsequent
    // move is not ignored
    ip_addr_t zero_ip = {0};
    local_mac_ip_del_(ips_info_.bd_id, ips_info_.mac_address, zero_ip);
    op_create_ = true;

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = state_t::thread_context();

        fetch_store_info_(state_ctxt.state());
        resolve_teps_(state_ctxt.state());

        cookie_uptr_.reset(new cookie_t);
        pds_bctxt_guard = make_batch_pds_spec_();

        auto l_bd_id = ips_info_.bd_id;
        auto l_op_create = op_create_;
        auto l_tep_ip = *(ips_info_.tep_ip_list.begin());
        mac_addr_t l_mac;
        MAC_ADDR_COPY(l_mac, update_fdb_mac->mac_address);

        cookie_uptr_->send_ips_reply =
            [update_fdb_mac, l_bd_id, l_mac, l_op_create, l_tep_ip]
            (bool pds_status, bool ips_mock) -> void {
                // ----------------------------------------------------------------
                // This block is executed asynchronously when PDS response is rcvd
                // ----------------------------------------------------------------
                if (!pds_status && l_op_create) {
                    do {
                        // Enter thread-safe context to access/modify global state
                        auto state_ctxt = state_t::thread_context();
                        // Create failed - Reset the HAL created flag
                        auto bd_obj = state_ctxt.state()->bd_store().
                            get(update_fdb_mac->bd_id.bd_id);
                        if (!bd_obj) break;
                        bd_obj->mac_store().erase({l_bd_id, l_mac});
                    } while(0);
                }

                if (unlikely(ips_mock)) return; // UT

                NBB_CREATE_THREAD_CONTEXT
                NBS_ENTER_SHARED_CONTEXT(l2f_proc_id);
                NBS_GET_SHARED_DATA();

                do {
                    auto bdpi_join = get_l2f_bdpi_join();
                    if (bdpi_join == nullptr) {
                        PDS_TRACE_ERR("Failed to find BDPI join to return BD %u AddUpd IPS",
                                      update_fdb_mac->bd_id.bd_id);
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
                        PDS_TRACE_DEBUG ("++++ BD %d MAC %s TEP %s"
                                         " Async reply %s ++++",
                                         l_bd_id, macaddr2str(l_mac),
                                         ipaddr2str(&l_tep_ip),
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
        auto ret = learn::api_batch_commit(pds_bctxt_guard.release());
        if (unlikely (ret != SDK_RET_OK)) {
            delete cookie;
            throw Error(std::string("Batch commit failed for MAC Add BD ")
                        .append(std::to_string(l_bd_id))
                        .append(" MAC ").append(macaddr2str(l_mac))
                        .append(" err ").append(std::to_string(ret)));
        }

        auto mac_obj = store_info_.bd_obj->mac_store().get
            ({ips_info_.bd_id, ips_info_.mac_address});

        if (mac_obj == nullptr) {
            // New FDB entry - Populate cache
            mac_obj_t::key_t key (ips_info_.bd_id, ips_info_.mac_address);
            store_info_.bd_obj->mac_store().
                add_upd(key, new mac_obj_t(key));

            PDS_TRACE_DEBUG("MS BD %d MAC %s TEP %s Create PDS Batch commit successful",
                            ips_info_.bd_id, macaddr2str(ips_info_.mac_address),
                            ipaddr2str(&(*ips_info_.tep_ip_list.begin())));
       } else {
            PDS_TRACE_DEBUG("MS BD %d MAC %s TEP %s Update PDS Batch commit successful",
                            ips_info_.bd_id, macaddr2str(ips_info_.mac_address),
                            ipaddr2str(&(*ips_info_.tep_ip_list.begin())));
       }
    } // End of state thread_context
      // Do Not access/modify global state after this

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
            PDS_TRACE_DEBUG("++++ BD %d MAC %s MAC Del Async reply %s ++++",
                            l_bd_id, macaddr2str(l_mac),
                            (pds_status) ? "Success" : "Failure");

        };
    // All processing complete, only batch commit remains -
    // safe to release the cookie_uptr_ unique_ptr
    auto cookie = cookie_uptr_.release();
    auto ret = learn::api_batch_commit(pds_bctxt_guard.release());
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

void l2f_mai_t::handle_add_upd_ip(ATG_MAI_UPDATE_MAC_IP* mac_ip_id) {
    pds_batch_ctxt_guard_t  pds_bctxt_guard;
    ip_addr_t  l_tep_ip;

    parse_ips_info_(mac_ip_id);

    PDS_TRACE_INFO("L2F MAC IP ADD BD %d IP %s MAC %s",
                   ips_info_.bd_id, ipaddr2str(&ips_info_.ip_address),
                   macaddr2str(ips_info_.mac_address));

    // Clean up local MAC,IP from MS store so that any subsequent
    // move is not ignored
    local_mac_ip_del_(ips_info_.bd_id, ips_info_.mac_address,
                      ips_info_.ip_address);

    // Cannot detect Update of existing Remote Mapping IP entry
    // Learn accepts Create for existing Remote Mapping entries
    op_create_ = true;

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = state_t::thread_context();

        fetch_store_info_(state_ctxt.state());
        resolve_teps_(state_ctxt.state());

        cookie_uptr_.reset(new cookie_t);
        pds_bctxt_guard = make_batch_pds_spec_();

        l_tep_ip = *(ips_info_.tep_ip_list.begin());
    } // End of state thread_context
      // Do Not access/modify global state after this

    auto l_bd_id = ips_info_.bd_id;
    auto l_ip = ips_info_.ip_address;

    cookie_uptr_->send_ips_reply =
        [l_bd_id, l_ip, l_tep_ip] (bool pds_status, bool ips_mock) -> void {
            // ----------------------------------------------------------------
            // This block is executed asynchronously when PDS response is rcvd
            // ----------------------------------------------------------------
            PDS_TRACE_DEBUG("++++ BD %d IP %s TEP %s Remote IP AddUpd"
                            " Async reply %s ++++",
                            l_bd_id, ipaddr2str(&l_ip), ipaddr2str(&l_tep_ip),
                            (pds_status) ? "Success" : "Failure");

        };

    // All processing complete, only batch commit remains -
    // safe to release the cookie_uptr_ unique_ptr
    auto cookie = cookie_uptr_.release();
    auto ret = learn::api_batch_commit(pds_bctxt_guard.release());
    if (unlikely (ret != SDK_RET_OK)) {
        delete cookie;
        throw Error(std::string("Batch commit failed for Remote IP Add BD ")
                    .append(std::to_string(l_bd_id))
                    .append(" IP ").append(ipaddr2str(&l_ip))
                    .append(" err ").append(std::to_string(ret)));
    }
    PDS_TRACE_DEBUG("BD %d Remote IP %s TEP %s Add PDS Batch commit successful",
                    l_bd_id, ipaddr2str(&l_ip), ipaddr2str(&l_tep_ip));
}

void l2f_mai_t::handle_delete_ip(const ATG_MAI_MAC_IP_ID* mac_ip_id) {
    pds_batch_ctxt_guard_t  pds_bctxt_guard;
    op_delete_ = true;

    // Populate IPS info
    parse_ips_info_(mac_ip_id);

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
            PDS_TRACE_DEBUG("++++ BD %d IP %s: Remote IP Delete Async reply %s",
                            l_bd_id, ipaddr2str(&l_ip),
                            (pds_status) ? "Success" : "Failure");

        };

    // All processing complete, only batch commit remains -
    // safe to release the cookie_uptr_ unique_ptr
    auto cookie = cookie_uptr_.release();
    auto ret = learn::api_batch_commit(pds_bctxt_guard.release());
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

pds_batch_ctxt_guard_t
l2f_del_remote_macs_for_bd (state_t::context_t& state_ctxt, ms_bd_id_t bd_id)
{
    pds_batch_ctxt_guard_t  pds_bctxt_guard;
    if (!state_ctxt.state()->bd_store().get(bd_id)->has_macs()) {
        PDS_TRACE_DEBUG("MS BD %d has no Remote MAC mappings", bd_id);
        return pds_bctxt_guard;
    }
    // Do Not combine Delete MACs with any other batch
    // Creating a new batch for the MACs
    pds_batch_params_t bp {PDS_BATCH_PARAMS_EPOCH,
                           PDS_BATCH_PARAMS_ASYNC,
                           hal_callback,
                           nullptr};
    auto bctxt = pds_batch_start(&bp);

    if (unlikely (!bctxt)) {
        throw Error(std::string("PDS Batch Start failed when trying to delete"
                                " Remote MACs for MS BD ")
                    .append(std::to_string(bd_id)));
    }
    pds_bctxt_guard.set(bctxt);

    l2f_del_remote_macs_for_bd(state_ctxt, bd_id, bctxt);
    return pds_bctxt_guard;
}

} // End namespace
