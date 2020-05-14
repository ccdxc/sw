//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// LI VXLAN Tunnel HAL integration
//---------------------------------------------------------------

#include <thread>
#include "nic/metaswitch/stubs/hals/pds_ms_hals_ecmp.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_li_vxlan_port.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_li_vxlan_tnl.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_ip_track_hal.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_hal_init.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/apollo/learn/learn_api.hpp"
#include <hals_c_includes.hpp>
#include <hals_nhpi_slave_join.hpp>
#include <hals_ecmp.hpp>

extern NBB_ULONG hals_proc_id;

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

#define NHPI_GET_FIRST_NH(ips, list_ptr) \
       (ATG_NHPI_APPENDED_NEXT_HOP*) \
            NTL_OFF_LIST_TRAVERSE((ips), (list_ptr), NULL)

#define NHPI_GET_NEXT_NH(ips, list_ptr, cur_obj) \
       (ATG_NHPI_APPENDED_NEXT_HOP*) \
            NTL_OFF_LIST_TRAVERSE((ips), (list_ptr), cur_obj)

static void
count_nh_in_ps_ (ATG_NHPI_APPENDED_NEXT_HOP* next_hop,
                 uint32_t *nh_count)
{
    switch (next_hop->next_hop_properties.destination_type) {
    case ATG_NHPI_NEXT_HOP_DEST_PORT:
        ++nh_count[PDS_MS_NH_DEST_PORT];
        break;
    case ATG_NHPI_NEXT_HOP_DEST_NH:
        ++nh_count[PDS_MS_NH_DEST_NH];
        break;
    case ATG_NHPI_NEXT_HOP_DEST_BH:
        ++nh_count[PDS_MS_NH_DEST_BH];
        break;
    }
}

static void
iterate_nh_in_pathset_ (ATG_NHPI_ADD_UPDATE_ECMP* add_upd_ecmp_ips,
                        NTL_OFFSET* list_p, uint32_t *nh_count,
                        std::function <void(ATG_NHPI_APPENDED_NEXT_HOP*,
                                            uint32_t *nh_count)> hdlr)
{
    for (auto next_hop = MS_LIST_GET_FIRST(add_upd_ecmp_ips, list_p,
                                           ATG_NHPI_APPENDED_NEXT_HOP);
         next_hop != NULL;
         next_hop = MS_LIST_GET_NEXT(add_upd_ecmp_ips, list_p, next_hop,
                                     ATG_NHPI_APPENDED_NEXT_HOP)) {
        hdlr(next_hop, nh_count);
    }
}

static void
log_nh_count_ (ms_ps_id_t ps, const char* str,
               uint32_t (&nh_count) [PDS_MS_NH_DEST_MAX])
{
    PDS_TRACE_INFO("Pathset %d %s DestPort %d DestNH %d Blackhole %d",
                   ps, str, nh_count[PDS_MS_NH_DEST_PORT],
                   nh_count[PDS_MS_NH_DEST_NH],
                   nh_count[PDS_MS_NH_DEST_BH]);
}

static const char*
psstr_ (hals_ecmp_t::ps_type_e ps_type)
{
    switch(ps_type) {
    case hals_ecmp_t::ps_type_e::DIRECT:
        return "Direct";
    case hals_ecmp_t::ps_type_e::INDIRECT:
        return "Indirect";
    case hals_ecmp_t::ps_type_e::OVERLAY:
        return "Overlay";
    case hals_ecmp_t::ps_type_e::UNKNOWN:
        return "Unknown";
    }
    return "Unknown";
}

static pds_ms_nh_type_t
get_nh_type_ (hals_ecmp_t::ps_type_e ps_type)
{
    pds_ms_nh_type_t nh_type;

    switch (ps_type) {
    case hals_ecmp_t::ps_type_e::DIRECT:
        nh_type = PDS_MS_NH_DEST_PORT;
        break;
    case hals_ecmp_t::ps_type_e::INDIRECT:
        nh_type = PDS_MS_NH_DEST_NH;
        break;
    case hals_ecmp_t::ps_type_e::OVERLAY:
        nh_type = PDS_MS_NH_DEST_PORT;
        break;
    case hals_ecmp_t::ps_type_e::UNKNOWN:
        SDK_ASSERT(0);
    }
    return nh_type;
}

static uint32_t
sum_nh_count_ (uint32_t* nh_count)
{
    uint32_t total = 0;
    for (uint32_t i=PDS_MS_NH_DEST_MIN; i< PDS_MS_NH_DEST_MAX; ++i) {
        total += nh_count[i];
    }
    return total;
}

void hals_ecmp_t::calculate_op_() {
    if ((ips_info_.ps_type == ps_type_e::INDIRECT) &&
        (ips_info_.ips->max_num_next_hop_objects > 1)) {
        PDS_TRACE_DEBUG("Ignore Indirect Pathset %d pointing to %d max nh",
                        ips_info_.pathset_id,
                        ips_info_.ips->max_num_next_hop_objects);
        op_ = op_type_e::IGNORE;
        return;
    }

    auto nh_type = get_nh_type_(ips_info_.ps_type);

    auto total_cur = sum_nh_count_(ips_info_.nh_count);
    auto total_add = sum_nh_count_(ips_info_.nh_add_count);
    auto total_del = sum_nh_count_(ips_info_.nh_del_count);

    if ((total_cur == total_add) &&
        (total_del == 0)) {
        op_ = op_type_e::CREATE;
        return;
    }

    op_ = op_type_e::UPDATE;
    auto prev_count = ips_info_.nh_count[nh_type] +
                      ips_info_.nh_del_count[nh_type] - 
                      ips_info_.nh_add_count[nh_type];

    PDS_TRACE_DEBUG("%s Pathset %d update, %d -> %d",
                    psstr_(ips_info_.ps_type), ips_info_.pathset_id,
                    prev_count, ips_info_.nh_count[nh_type]);
    return;
}

bool hals_ecmp_t::parse_ips_info_(ATG_NHPI_ADD_UPDATE_ECMP* add_upd_ecmp_ips) {

    // Only first 32 bits of the correlator are used for Pathset ID
    // Ref: psm_fte.cpp -> psm::Fte::increment_corr
    SDK_ASSERT (add_upd_ecmp_ips->pathset_id.correlator2 == 0);
    NBB_CORR_GET_VALUE (ips_info_.pathset_id, add_upd_ecmp_ips->pathset_id);
    ips_info_.ips = add_upd_ecmp_ips;

    bool process = true;
    if (add_upd_ecmp_ips->cascaded == ATG_YES) {
        ips_info_.ps_type = ps_type_e::DIRECT;
    } else {
        ips_info_.ps_type = ps_type_e::UNKNOWN;
    }

    // Count added nexthops
    auto list_p = &add_upd_ecmp_ips->added_next_hop_objects;
    iterate_nh_in_pathset_(add_upd_ecmp_ips, list_p, ips_info_.nh_add_count,
                           count_nh_in_ps_);

    // Count deleted nexthops
    list_p = &add_upd_ecmp_ips->deleted_next_hop_objects;
    iterate_nh_in_pathset_(add_upd_ecmp_ips, list_p, ips_info_.nh_del_count,
                           count_nh_in_ps_);

    // Current nexthops
    list_p = &add_upd_ecmp_ips->next_hop_objects;
    auto l_ips_info_ = &ips_info_;
    iterate_nh_in_pathset_(add_upd_ecmp_ips, list_p, ips_info_.nh_count,
                           [add_upd_ecmp_ips, l_ips_info_]
                           (ATG_NHPI_APPENDED_NEXT_HOP* next_hop,
                            uint32_t *nh_count) -> void {

        auto& ips_info_ = *l_ips_info_;
        count_nh_in_ps_(next_hop, nh_count);

        ATG_NHPI_APPENDED_NEXT_HOP appended_next_hop;

        if (next_hop->next_hop_properties.destination_type !=
            ATG_NHPI_NEXT_HOP_DEST_PORT) {
            // Indirect or Blackhole Pathset -
            // Copy to programmed to make MS think it is programmed in HW
            NBB_MEMSET(&appended_next_hop, 0, sizeof(ATG_NHPI_APPENDED_NEXT_HOP));

            appended_next_hop.total_length = sizeof(ATG_NHPI_APPENDED_NEXT_HOP);
            appended_next_hop.next_hop_properties = next_hop->next_hop_properties;
            NTL_OFF_LIST_APPEND(add_upd_ecmp_ips,
                                &add_upd_ecmp_ips->programmed_next_hop_objects,
                                appended_next_hop.total_length,
                                (NBB_VOID *)&appended_next_hop,
                                NULL);
        }
        if (next_hop->next_hop_properties.destination_type ==
            ATG_NHPI_NEXT_HOP_DEST_NH) {
            // For Indirect Pathset, inherit the lower level DP correlator
            add_upd_ecmp_ips->dp_correlator =
                next_hop->next_hop_properties.indirect_next_hop_properties.
                lower_level_dp_correlator;
            NBB_CORR_GET_VALUE(ips_info_.direct_ps_dpcorr, next_hop->next_hop_properties.
                               indirect_next_hop_properties.
                               lower_level_dp_correlator);
            ips_info_.ps_type = ps_type_e::INDIRECT;
            PDS_TRACE_DEBUG("Indirect Pathset %d Copy lower level DP Correlator %d",
                             ips_info_.pathset_id, ips_info_.direct_ps_dpcorr);
        }
        if (next_hop->next_hop_properties.destination_type ==
            ATG_NHPI_NEXT_HOP_DEST_PORT) {
            // Direct or Overlay Pathset
            ATG_NHPI_NEIGHBOR_PROPERTIES& prop =
                next_hop->next_hop_properties.direct_next_hop_properties.neighbor;
            ips_info_.nexthops.emplace_back(prop.neighbor_l3_if_index,
                                            prop.neighbor_id.mac_address);
            if (ms_ifindex_to_pds_type(prop.neighbor_l3_if_index) == IF_TYPE_L3) {
                ips_info_.ps_type = ps_type_e::DIRECT;
            } else {
                ips_info_.ps_type = ps_type_e::OVERLAY;
            }
        }
    });

    log_nh_count_(ips_info_.pathset_id, "Current", ips_info_.nh_count);
    log_nh_count_(ips_info_.pathset_id, "Added", ips_info_.nh_add_count);
    log_nh_count_(ips_info_.pathset_id, "Deleted", ips_info_.nh_del_count);

    return process;
}

void hals_ecmp_t::fetch_store_info_(state_t* state) {
    // Only called for overlay
    store_info_.pathset_obj = state->pathset_store().get(ips_info_.pathset_id);
}

pds_obj_key_t hals_ecmp_t::make_pds_nhgroup_key_(void) {
    ms_hw_tbl_id_t idx;
    bool underlay = false;
    if (ips_info_.ps_type == ps_type_e::DIRECT) {
        idx = ips_info_.pathset_id;
        underlay = true;
    } else {
        idx = store_info_.pathset_obj->hal_oecmp_idx_guard->idx();
    }
    // Since underlay and overlay NH Groups go into the same PDS HAL table
    // their IDs cannot clash
    return msidx2pdsobjkey(idx, underlay);
}

void hals_ecmp_t::make_pds_underlay_nhgroup_spec_
                                 (pds_nexthop_group_spec_t& nhgroup_spec,
                                  state_t::context_t& state_ctxt) {
    int i = 0, num_repeats = 0;

    // Max num NH indicates the number of nexthops to be reserved for this NH Group
    // in hardware. If the actual number of nexthops is lesser we need to repeat to
    // fill up space.
    auto max_num_nh = ips_info_.ips->max_num_next_hop_objects;
    PDS_TRACE_DEBUG("Underlay Direct Pathset %d,  Max NH# %d Cur NH# %d",
                    ips_info_.pathset_id, max_num_nh, ips_info_.nexthops.size());

    // Max NH allowed is 2 for underlay. Going beyond this requires complicated logic
    // to reserve entries if the current nexthops is lesser than  Max NH
    SDK_ASSERT (max_num_nh <= 2);

    if (ips_info_.nexthops.size() == 0) {
        num_repeats =  max_num_nh;
        for (int repeat = 0; repeat < num_repeats; ++repeat) {
            nhgroup_spec.nexthops[i].type = PDS_NH_TYPE_BLACKHOLE;
            ++i;
        }
        PDS_TRACE_DEBUG("Underlay Direct Pathset %d added %d blackhole nexthops",
                        ips_info_.pathset_id, i);
        nhgroup_spec.num_nexthops = i;
        return; 
    }
    num_repeats =  max_num_nh / ips_info_.nexthops.size();

    for (int repeat = 0; repeat < num_repeats; ++repeat) {
        for (auto& nh: ips_info_.nexthops) {
            // Nexthop key is unused
            nhgroup_spec.nexthops[i].type = PDS_NH_TYPE_UNDERLAY;

            // Fetch underlay L3 interface UUID from If Store
            auto phy_if_obj = state_ctxt.state()->if_store().get(nh.ms_ifindex);
            if (phy_if_obj == nullptr) {
                throw Error(std::string("Underlay Direct Pathset ")
                            .append(std::to_string(ips_info_.pathset_id))
                            .append(" with unknown dest interface ")
                            .append(std::to_string (nh.ms_ifindex)));
            }
            nhgroup_spec.nexthops[i].l3_if =
                phy_if_obj->phy_port_properties().l3_if_spec.key;

            memcpy(nhgroup_spec.nexthops[i].underlay_mac, nh.mac_addr.m_mac,
                   ETH_ADDR_LEN);
            PDS_TRACE_DEBUG("Underlay Direct Pathset %ld add NH MSIfIndex 0x%lx PDSIf"
                            " UUID %s MAC %s",
                            ips_info_.pathset_id, nh.ms_ifindex,
                            nhgroup_spec.nexthops[i].l3_if.str(),
                            macaddr2str(nh.mac_addr.m_mac));
            ++i;
        }
    }
    nhgroup_spec.num_nexthops = i;
}

void hals_ecmp_t::make_pds_overlay_nhgroup_spec_
                                 (pds_nexthop_group_spec_t& nhgroup_spec,
                                  state_t::context_t& state_ctxt) {
    int i = 0;
    for (auto& nh: ips_info_.nexthops) {
        auto vxp_if_obj = state_ctxt.state()->if_store().get(nh.ms_ifindex);
        if (vxp_if_obj == nullptr) {
            PDS_TRACE_DEBUG("Overlay Pathset %d ignore L3 VXLAN Port 0x%x"
                            " due to possible race condition",
                            ips_info_.pathset_id, nh.ms_ifindex);
            continue;
        }

        nhgroup_spec.nexthops[i].type = PDS_NH_TYPE_OVERLAY;

        auto& vxp_prop = vxp_if_obj->vxlan_port_properties();
        auto tep_obj = state_ctxt.state()->tep_store()
                        .get(vxp_if_obj->vxlan_port_properties().tep_ip);
        auto& dmaci = vxp_if_obj->vxlan_port_properties().dmaci;
        if (memcmp(dmaci, nh.mac_addr.m_mac, ETH_ADDR_LEN) != 0) {
            // Change in L3 VXLAN Port DMAC
            // This is basically the Router MAC advertised in Type 5 routes
            // TODO: Assuming that the same {TEP, VNI} will not advertise
            // multiple Router MACs. Hence blindly overwriting existing MAC
            if (is_mac_set(dmaci)) {
                PDS_TRACE_ERR("!! Overwriting Router MAC address for TEP %s VNI %s"
                              " L3 VXLAN Port 0x%x from %s to %s - UNDEFINED BEHAVIOR !!!",
                              ipaddr2str(&vxp_prop.tep_ip), vxp_prop.vni,
                              vxp_prop.ifindex, macaddr2str(dmaci),
                              macaddr2str(nh.mac_addr.m_mac));
            }
            MAC_ADDR_COPY(dmaci, nh.mac_addr.m_mac);
            li_vxlan_port vxp;
            vxp.add_pds_tep_spec(store_info_.bctxt, state_ctxt.state(),
                                 vxp_if_obj, tep_obj, false /* Op Update */);
            PDS_TRACE_DEBUG("Setting DMAC for Type5 TEP %s VNI %d L3 VXLAN Port"
                            " 0x%x to %s",
                            ipaddr2str(&vxp_prop.tep_ip), vxp_prop.vni,
                            vxp_prop.ifindex, macaddr2str(nh.mac_addr.m_mac));
        }
        nhgroup_spec.nexthops[i].tep = vxp_prop.hal_tep_idx;
        PDS_TRACE_DEBUG("Overlay Pathset %d add NH# %d L3 VXLAN Port %x"
                        " Type5 TEP %s UUID %s VNI %d"
                        " HAL Idx %s Overlay NHGroup %s",
                        ips_info_.pathset_id, i+1, nh.ms_ifindex,
                        ipaddr2str(&vxp_prop.tep_ip),
                        nhgroup_spec.nexthops[i].tep.str(),
                        vxp_prop.vni, vxp_prop.hal_tep_idx.str(),
                        nhgroup_spec.key.str());
        ++i;
    }
    nhgroup_spec.num_nexthops = i;
}

pds_nexthop_group_spec_t hals_ecmp_t::make_pds_nhgroup_spec_(state_t::context_t&
                                                             state_ctxt) {
    pds_nexthop_group_spec_t nhgroup_spec = {0};
    nhgroup_spec.key = make_pds_nhgroup_key_();
    if (ips_info_.ps_type == ps_type_e::DIRECT) {
        nhgroup_spec.type = PDS_NHGROUP_TYPE_UNDERLAY_ECMP;
        make_pds_underlay_nhgroup_spec_(nhgroup_spec, state_ctxt);
    } else {
        nhgroup_spec.type = PDS_NHGROUP_TYPE_OVERLAY_ECMP;
        make_pds_overlay_nhgroup_spec_(nhgroup_spec, state_ctxt);
    }
    return nhgroup_spec;
}

pds_batch_ctxt_guard_t hals_ecmp_t::make_batch_pds_spec_(state_t::context_t&
                                                         state_ctxt) {
    pds_batch_ctxt_guard_t bctxt_guard_;
    sdk_ret_t ret = SDK_RET_OK;
    SDK_ASSERT(cookie_uptr_); // Cookie should have been alloc before
    pds_batch_params_t bp { PDS_BATCH_PARAMS_EPOCH, PDS_BATCH_PARAMS_ASYNC,
                            pds_ms::hal_callback,
                            cookie_uptr_.get() };
    store_info_.bctxt = pds_batch_start(&bp);

    if (unlikely (!store_info_.bctxt)) {
        throw Error(std::string("PDS Batch Start failed for MS ECMP ")
                    .append(std::to_string(ips_info_.pathset_id)));
    }
    bctxt_guard_.set(store_info_.bctxt);

    if (op_ == op_type_e::DELETE) { // Delete
        auto nhgroup_key = make_pds_nhgroup_key_();
        if (!PDS_MOCK_MODE()) {
            ret = pds_nexthop_group_delete(&nhgroup_key, store_info_.bctxt);
        }
        if (unlikely (ret != SDK_RET_OK)) {
            throw Error(std::string("Delete PDS Nexthop Group failed for MS ECMP ")
                        .append(std::to_string(ips_info_.pathset_id)));
        }

    } else { // Add or update
        auto nhgroup_spec = make_pds_nhgroup_spec_(state_ctxt);
        if (op_ == op_type_e::CREATE) {
            if (!PDS_MOCK_MODE()) {
                ret = pds_nexthop_group_create(&nhgroup_spec, store_info_.bctxt);
            }
            if (unlikely (ret != SDK_RET_OK)) {
                throw Error(std::string("Create PDS Nexthop Group failed for MS ECMP ")
                            .append(std::to_string(ips_info_.pathset_id)));
            }
        } else {
            if (!PDS_MOCK_MODE()) {
                ret = pds_nexthop_group_update(&nhgroup_spec, store_info_.bctxt);
            }
            if (unlikely (ret != SDK_RET_OK)) {
                throw Error(std::string("Update PDS Nexthop Group failed for MS ECMP ")
                            .append(std::to_string(ips_info_.pathset_id)));
            }
        }
    }
    return bctxt_guard_;
}

static void send_ips_response_(ATG_NHPI_ADD_UPDATE_ECMP* add_upd_ecmp_ips,
                               ms_ps_id_t pathset_id,
                               ms_ps_id_t dp_corr) {
    NBB_CREATE_THREAD_CONTEXT
    NBS_ENTER_SHARED_CONTEXT(hals_proc_id);
    NBS_GET_SHARED_DATA();

    NBB_CORR_PUT_VALUE(add_upd_ecmp_ips->dp_correlator, dp_corr);
    // Copy all nexthops to the programmed next_hop_objects
    auto list_p = &add_upd_ecmp_ips->next_hop_objects;
    ATG_NHPI_APPENDED_NEXT_HOP appended_next_hop;

    for (auto next_hop = MS_LIST_GET_FIRST(add_upd_ecmp_ips, list_p,
                                           ATG_NHPI_APPENDED_NEXT_HOP);
         next_hop != NULL;
         next_hop = MS_LIST_GET_NEXT(add_upd_ecmp_ips, list_p, next_hop,
                                     ATG_NHPI_APPENDED_NEXT_HOP)) {

        if (next_hop->next_hop_properties.destination_type !=
            ATG_NHPI_NEXT_HOP_DEST_PORT) {
            // Non-Direct Pathsets already have their nexthops copied out
            // to the programmed list for the synchronous return case
            break;
        }
        NBB_MEMSET(&appended_next_hop, 0, sizeof(ATG_NHPI_APPENDED_NEXT_HOP));

        appended_next_hop.total_length = sizeof(ATG_NHPI_APPENDED_NEXT_HOP);
        appended_next_hop.next_hop_properties = next_hop->next_hop_properties;

        NTL_OFF_LIST_APPEND(add_upd_ecmp_ips,
                            &add_upd_ecmp_ips->programmed_next_hop_objects,
                            appended_next_hop.total_length,
                            (NBB_VOID *)&appended_next_hop,
                            NULL);
    }

    auto key = hals::Ecmp::get_key(*add_upd_ecmp_ips);

    auto& ecmp_store = hals::Fte::get().get_nhpi_join()->get_ecmp_store();
    auto it = ecmp_store.find(key);
    if (it == ecmp_store.end()) {
        // Metaswitch does not support IPS failure response
        // Instead dp correlator is returned as 0 for create failures
        auto send_response =
            hals::Ecmp::set_ips_rc(&add_upd_ecmp_ips->ips_hdr, ATG_OK);
        SDK_ASSERT(send_response);
        hals::Fte::get().get_nhpi_join()->
            send_ips_reply(&add_upd_ecmp_ips->ips_hdr);
    } else {
        (*it)->update_complete(ATG_OK);
    }
    NBS_RELEASE_SHARED_DATA();
    NBS_EXIT_SHARED_CONTEXT();
    NBB_DESTROY_THREAD_CONTEXT
}

void hals_ecmp_t::send_delete_to_hal_(state_t::context_t& state_ctxt) {
    bool l_underlay = true;
    op_ = op_type_e::DELETE;
    pds_batch_ctxt_guard_t  pds_bctxt_guard;
    ms_hw_tbl_id_t  dp_corr;

    if (store_info_.pathset_obj != nullptr) {
        l_underlay = false;
        ips_info_.ps_type = ps_type_e::OVERLAY;
        dp_corr = store_info_.pathset_obj->hal_oecmp_idx_guard->idx();
        PDS_TRACE_DEBUG ("Overlay Pathset %ld: Delete IPS DP Corr %d",
                         ips_info_.pathset_id, dp_corr);
    } else {
        ips_info_.ps_type = ps_type_e::DIRECT;
        dp_corr = ips_info_.pathset_id;
        PDS_TRACE_DEBUG ("Underlay Pathset %ld: Delete IPS",
                         ips_info_.pathset_id);
    }

    cookie_uptr_.reset (new cookie_t);
    pds_bctxt_guard = make_batch_pds_spec_ (state_ctxt);

    auto pathset_id = ips_info_.pathset_id;
    cookie_uptr_->send_ips_reply =
        [pathset_id, l_underlay, dp_corr] (bool pds_status, bool ips_mock) -> void {
            //-----------------------------------------------------------------
            // This block is executed asynchronously when PDS response is rcvd
            //-----------------------------------------------------------------
            PDS_TRACE_DEBUG("++++ %s Pathset %ld NHGroup %d delete %s Async reply ++++",
                            (l_underlay) ? "Underlay" : "Overlay",
                            pathset_id, dp_corr,
                            (pds_status) ? "Success" : "Failure");

        };

    // All processing complete, only batch commit remains -
    // safe to release the cookie_uptr_ unique_ptr
    auto cookie = cookie_uptr_.release();
    auto ret = learn::api_batch_commit(pds_bctxt_guard.release());
    if (unlikely (ret != SDK_RET_OK)) {
        delete cookie;
        throw Error(std::string("Batch commit failed for delete MS ECMP ")
                    .append(std::to_string(ips_info_.pathset_id))
                    .append(" err=").append(std::to_string(ret)));
    }

    if (ips_info_.ps_type == ps_type_e::OVERLAY) {
        state_ctxt.state()->pathset_store().erase(ips_info_.pathset_id);
    }
    PDS_TRACE_DEBUG ("%s Pathset %ld delete PDS Batch commit successful",
                     (l_underlay) ? "Underlay" : "Overlay",
                     ips_info_.pathset_id);
}

NBB_BYTE hals_ecmp_t::handle_indirect_ps_update_
                            (ATG_NHPI_ADD_UPDATE_ECMP* add_upd_ecmp_ips) {
    NBB_BYTE rc = ATG_OK;

    // Received Pathset with Non-direct nextxhops.
    // Check if this is an Indirect Pathset that is referenced by
    // existing VXLAN Tunnels (TEPs)
    auto state_ctxt = pds_ms::state_t::thread_context();

    // Check that a indirect pathset has not been mistakenly classified
    // as overlay pathset
    store_info_.pathset_obj = state_ctxt.state()->pathset_store().
                              get(ips_info_.pathset_id);

    if (store_info_.pathset_obj  != nullptr) {
        PDS_TRACE_INFO("Revert wrong classification of blackhole pathset %d"
                       " as overlay pathset", ips_info_.pathset_id);
        send_delete_to_hal_(state_ctxt);
        // Set everything back
        ips_info_.ps_type = ps_type_e::INDIRECT;
        store_info_.pathset_obj = nullptr;
    }

    auto destip_pair =
        state_indirect_ps_lookup_and_map_dpcorr(state_ctxt.state(),
                                                ips_info_.pathset_id,
                                                ips_info_.direct_ps_dpcorr);
    auto destip = destip_pair.first;
    if (ip_addr_is_zero(&destip)) {
        PDS_TRACE_DEBUG("Indirect Pathset %d Create IPS DPCorr %d",
                        ips_info_.pathset_id, ips_info_.direct_ps_dpcorr);
        return rc;
    }

    PDS_TRACE_DEBUG("Indirect Pathset %d Update IPS %s %s new DPCorr %d",
                    ips_info_.pathset_id,
                    (destip_pair.second) ? "TEP" : "tracked DestIP",
                    ipaddr2str(&destip_pair.first),
                    ips_info_.direct_ps_dpcorr);

    if (!destip_pair.second) {
        // Indirect pathset is NOT tracking a MS EVPN TEP IP
        auto ip_track_obj = state_ctxt.state()->ip_track_store().get(destip);
        ip_track_reachability_change(destip, ips_info_.direct_ps_dpcorr,
                                     ip_track_obj->pds_obj_id());
        // Send synchronous IPS response to MS for Dest IP track change
        return ATG_OK;
    }

    auto& tep_ip = destip_pair.first;
    auto tep_obj = state_ctxt.state()->tep_store().get(tep_ip);
    if (tep_obj == nullptr) {
        // This Pathset does not have any reference yet
        // Nothing to do
        PDS_TRACE_DEBUG("Indirect Pathset %d update TEP %s obj not found",
                        ips_info_.pathset_id, ipaddr2str(&tep_ip));
        return rc;
    }

    li_vxlan_tnl tnl;
    // Alloc new cookie to capture async info
    cookie_uptr_.reset (new cookie_t);
    auto direct_ps_dpcorr = ips_info_.direct_ps_dpcorr;
    auto l_pathset_id = ips_info_.pathset_id;

    cookie_uptr_->send_ips_reply =
        [add_upd_ecmp_ips, l_pathset_id, tep_ip, direct_ps_dpcorr]
        (bool pds_status, bool ips_mock) -> void {
            //-----------------------------------------------------------------
            // This block is executed asynchronously when PDS response is rcvd
            //-----------------------------------------------------------------
            if (unlikely(ips_mock)) return; // UT
            PDS_TRACE_DEBUG("++++ TEP %s Underlay Pathset %d "
                            "NHGroup %d Async reply %s ++++",
                            ipaddr2str(&tep_ip), l_pathset_id, direct_ps_dpcorr,
                            (pds_status) ? "Success" : "Failure");

            send_ips_response_(add_upd_ecmp_ips, l_pathset_id,
                               direct_ps_dpcorr);
        };
    return tnl.handle_upathset_update(std::move(state_ctxt),
                                      tep_obj, direct_ps_dpcorr,
                                      std::move(cookie_uptr_));
}

void hals_ecmp_t::identify_ps_type_(void) {
    if (ips_info_.ps_type != ps_type_e::UNKNOWN) {
        return;
    }

    if (ips_info_.nh_del_count[PDS_MS_NH_DEST_NH] > 0) {
        ips_info_.ps_type = ps_type_e::INDIRECT;
        return;
    }
    if (ips_info_.nh_del_count[PDS_MS_NH_DEST_PORT] > 0) {
        if (mgmt_state_t::thread_context().state()->
            overlay_routing_en()) {
            // Cacscaded mode - cascaded flag is not set - should be overlay
            ips_info_.ps_type = ps_type_e::OVERLAY;
        } else {
            // Squashed mode
            ips_info_.ps_type = ps_type_e::DIRECT;
        }
        return;
    }
    if (mgmt_state_t::thread_context().state()->
        overlay_routing_en() &&
        ips_info_.nh_add_count[PDS_MS_NH_DEST_BH] > 0)  {
        // New non-cascaded pathset with only blackhole add and no deletes
        // Assume overlay. Indirect pathset cannot be created with BH
        ips_info_.ps_type = ps_type_e::OVERLAY;
        PDS_TRACE_DEBUG("Treating new non-cascaded pathset %d with BH as overlay",
                        ips_info_.pathset_id);
    }
}

NBB_BYTE hals_ecmp_t::handle_add_upd_ips(ATG_NHPI_ADD_UPDATE_ECMP* add_upd_ecmp_ips) {
    NBB_BYTE rc = ATG_OK;

    parse_ips_info_(add_upd_ecmp_ips);
    identify_ps_type_();

    if (ips_info_.ps_type == ps_type_e::UNKNOWN) {
        PDS_TRACE_DEBUG("Ignore pathset %d containing unknown nexthops",
                        ips_info_.pathset_id);
        return rc;
    }

    calculate_op_();
    if (op_ == op_type_e::IGNORE) {
        return rc;
    }

    if (ips_info_.ps_type == ps_type_e::INDIRECT) {
        return handle_indirect_ps_update_(add_upd_ecmp_ips);
    }

    // Direct Pathset - can be underlay or overlay
    if (ips_info_.ps_type == ps_type_e::DIRECT) {
        auto num_nexthops = ips_info_.nh_count[PDS_MS_NH_DEST_PORT];
        PDS_TRACE_DEBUG ("Underlay Direct Pathset %ld: %s IPS Num nexthops %ld",
                         ips_info_.pathset_id,
                         (op_ == op_type_e::CREATE) ? "Create" :
                         (op_ == op_type_e::DELETE) ? "Delete" : "Update",
                         num_nexthops);
    }

    cookie_t* cookie = nullptr;
    pds_batch_ctxt_guard_t  pds_bctxt_guard;

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = pds_ms::state_t::thread_context();

        // Alloc new cookie to capture async info
        cookie_uptr_.reset (new cookie_t);

        if (ips_info_.ps_type == ps_type_e::OVERLAY) {
            fetch_store_info_(state_ctxt.state());

            if (store_info_.pathset_obj == nullptr) {
                op_ = op_type_e::CREATE;

                pathset_obj_uptr_t pathset_obj_uptr
                    (new pathset_obj_t(ips_info_.pathset_id));
                store_info_.pathset_obj = pathset_obj_uptr.get();

                // Cache the new object in the cookie to revisit asynchronously
                // when the PDS API response is received
                cookie_uptr_->objs.push_back(std::move(pathset_obj_uptr));

                PDS_TRACE_DEBUG ("Overlay Pathset %ld: Create IPS Num nexthops %ld",
                                 ips_info_.pathset_id, ips_info_.nexthops.size());
            } else {
                PDS_TRACE_DEBUG ("Overlay Pathset %ld: Update IPS Num nexthops %ld",
                                 ips_info_.pathset_id, ips_info_.nexthops.size());
            }
        }
        pds_bctxt_guard = make_batch_pds_spec_(state_ctxt);
        // If we have batched multiple IPS earlier flush it now
        // Cannot defer Nexthop updates
        state_ctxt.state()->flush_outstanding_pds_batch();

        auto l_overlay = (ips_info_.ps_type == ps_type_e::OVERLAY);
        auto pathset_id = ips_info_.pathset_id;
        auto l_num_nh = ips_info_.nexthops.size();
        auto l_op = op_;

        cookie_uptr_->send_ips_reply =
            [add_upd_ecmp_ips, pathset_id, l_overlay, l_num_nh, l_op]
            (bool pds_status, bool ips_mock) -> void {
                //-----------------------------------------------------------------
                // This block is executed asynchronously when PDS response is rcvd
                //-----------------------------------------------------------------
                if (unlikely(ips_mock)) return; // UT

                uint32_t dp_corr = PDS_MS_ECMP_INVALID_INDEX;
                if (pds_status) {
                    if (!l_overlay) {
                        // HAL Key (DP correlator) is the MS pathset ID itself
                        if (l_op == op_type_e::DELETE) {
                            // TODO: MS yet to confirm that change in
                            // Direct DP Corr will be propagated upto the
                            // Indirect PS
                            dp_corr = PDS_MS_ECMP_INVALID_INDEX;
                        } else {
                            dp_corr = pathset_id;
                        }
                    } else {
                        // Enter thread-safe context to access/modify global state
                        auto state_ctxt = pds_ms::state_t::thread_context();
                        auto pathset_obj =
                            state_ctxt.state()->pathset_store().get(pathset_id);
                        dp_corr = pathset_obj->hal_oecmp_idx_guard->idx();
                    }
                } else {
                    // Failure
                    if (l_overlay && l_op == op_type_e::CREATE) {
                        // Overlay create failed
                        auto state_ctxt = pds_ms::state_t::thread_context();
                        state_ctxt.state()->pathset_store().erase(pathset_id);
                        PDS_TRACE_DEBUG("Overlay Pathset %d create failed erase store",
                                        pathset_id);
                    }
                    if ((l_op != op_type_e::CREATE) &&
                        (l_op != op_type_e::DELETE))  {
                        // For updates dont change the DP corr even if there is a failure
                        NBB_CORR_GET_VALUE (dp_corr, add_upd_ecmp_ips->dp_correlator);
                    }
                }
                PDS_TRACE_DEBUG("++++ %s Pathset %d NHgroup %d Num NH %d Async reply"
                                " %s ++++", (l_overlay) ? "Overlay": "Underlay",
                                pathset_id, dp_corr, l_num_nh,
                                (pds_status) ? "Success" : "Failure");

                send_ips_response_(add_upd_ecmp_ips, pathset_id,
                                   dp_corr);
            };

        rc = ATG_ASYNC_COMPLETION;
        // Move any store objects out of the cookie for
        // commiting to the store
        auto store_objs = std::move(cookie_uptr_->objs);
        // All processing complete, only batch commit remains -
        // safe to release the cookie unique_ptr
        cookie = cookie_uptr_.release();

        auto ret = learn::api_batch_commit(pds_bctxt_guard.release());
        if (unlikely (ret != SDK_RET_OK)) {
            delete cookie;
            throw Error(std::string("Batch commit failed for Add-Update Nexthop ")
                        .append(std::to_string(ips_info_.pathset_id))
                        .append(" err=").append(std::to_string(ret)));
        }
        PDS_TRACE_DEBUG ("%s Pathset %ld NHgroup %d Add/Upd PDS Batch commit successful",
                         (l_overlay) ? "Overlay" : "Underlay", ips_info_.pathset_id,
                         (l_overlay) ? store_info_.pathset_obj->hal_oecmp_idx_guard->idx()
                         : pathset_id);

        state_store_commit_objs (state_ctxt, store_objs);

    } // End of state thread_context
      // Do Not access/modify global state after this

    if (PDS_MOCK_MODE()) {
        // Call the HAL callback in PDS mock mode
        std::thread cb(pds_ms::hal_callback, SDK_RET_OK, cookie);
        cb.detach();
    }
    return rc;
}

void hals_ecmp_t::handle_delete(NBB_CORRELATOR ms_pathset_id) {
    // MS Stub Integration APIs do not support Async callback for deletes.
    // However since we should not block the MS NBase main thread
    // the HAL processing is always asynchronous even for deletes.
    // Assuming that Deletes never fail.

    NBB_CORR_GET_VALUE (ips_info_.pathset_id, ms_pathset_id);

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = pds_ms::state_t::thread_context();

        if (state_ctxt.state()->indirect_ps_store().
            erase(ips_info_.pathset_id)) {
            // Ignore deletes of indirect pathset
            PDS_TRACE_DEBUG ("Indirect Pathset %ld: Delete IPS nothing to do",
                             ips_info_.pathset_id);
            return;
        }

        // If we have batched multiple IPS earlier flush it now
        // Cannot defer Nexthop updates
        state_ctxt.state()->flush_outstanding_pds_batch();

        store_info_.pathset_obj = state_ctxt.state()->pathset_store().
                                            get(ips_info_.pathset_id);

        send_delete_to_hal_(state_ctxt);
    }
}

} // End namespace
