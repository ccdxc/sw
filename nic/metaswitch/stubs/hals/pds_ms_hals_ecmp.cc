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
    for (auto next_hop = NHPI_GET_FIRST_NH(add_upd_ecmp_ips, list_p);
         next_hop != NULL;
         next_hop = NHPI_GET_NEXT_NH(add_upd_ecmp_ips, list_p, next_hop)) {

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

void hals_ecmp_t::calculate_op_() {
    if (ips_info_.nh_count[PDS_MS_NH_DEST_NH] > 1) {
        PDS_TRACE_DEBUG("Ignore %s Pathset %d pointing to multiple direct PS",
                        psstr_(ips_info_.ps_type), ips_info_.pathset_id);
        op_ = op_type_e::IGNORE;
        return;
    }
    if (ips_info_.nh_count[PDS_MS_NH_DEST_BH] > 1) {
        PDS_TRACE_DEBUG("Ignore %s Pathset %d pointing to multiple blackhole",
                        psstr_(ips_info_.ps_type), ips_info_.pathset_id);
        op_ = op_type_e::IGNORE;
        return;
    }
    auto nh_type = get_nh_type_(ips_info_.ps_type);

    // Broad categories -
    // Valid NH added
    // BH added
    // Valid NH deleted
    // BH deleted - cannot happen without valid NH add

    if (ips_info_.nh_add_count[nh_type] > 0) {
        // Valid NH added
        // Rule out - BH add
        SDK_ASSERT(ips_info_.nh_add_count[PDS_MS_NH_DEST_BH] == 0);

        if (ips_info_.nh_del_count[nh_type] ==
            ips_info_.nh_add_count[nh_type]) {
            // Num added == Num deleted, so replace
            PDS_TRACE_DEBUG("%s Pathset %d update, Replace",
                            psstr_(ips_info_.ps_type), ips_info_.pathset_id);
            op_ = op_type_e::UPDATE_REPLACE;
            return;
        }

        if (ips_info_.nh_add_count[nh_type] ==
            ips_info_.nh_count[nh_type]) {
            // Num added == Num final, so create
            if (ips_info_.nh_del_count[PDS_MS_NH_DEST_BH] > 0) {
                PDS_TRACE_DEBUG("%s Pathset %d create, BH -> New",
                                psstr_(ips_info_.ps_type), ips_info_.pathset_id);
                op_ = op_type_e::BH_TO_VALID;
                return;
            }
            PDS_TRACE_DEBUG("%s Pathset %d create, New",
                            psstr_(ips_info_.ps_type), ips_info_.pathset_id);
            op_ = op_type_e::CREATE;
            return;
        } else {
            PDS_TRACE_DEBUG("%s Pathset %d create, 1 -> 2",
                            psstr_(ips_info_.ps_type), ips_info_.pathset_id);
            op_ = op_type_e::UPDATE_ADD;
            return;
        }
    }
    if (ips_info_.nh_add_count[PDS_MS_NH_DEST_BH] > 0) {
        // BH added
        // Rule out - Valid NH add
        SDK_ASSERT(ips_info_.nh_add_count[nh_type] == 0);

        if (ips_info_.nh_del_count[nh_type] > 0) {
            // With valid NH del
            if (ips_info_.nh_count[nh_type] == 0) {
                PDS_TRACE_DEBUG("%s Pathset %d delete, Old -> BH",
                                psstr_(ips_info_.ps_type), ips_info_.pathset_id);
                op_ = op_type_e::VALID_TO_BH;
                return;
            }
            // Blackhole added but still valid NH left
            // 2 -> 2 (Valid , BH)
            PDS_TRACE_DEBUG("%s Pathset %d update, Unchng,Old -> Unchng,BH",
                            psstr_(ips_info_.ps_type), ips_info_.pathset_id);
            op_ = op_type_e::UPDATE_DEL;
            return;
        }
        // Without valid NH del - new BH
        PDS_TRACE_DEBUG("%s Pathset %d ignore, BH",
                        psstr_(ips_info_.ps_type), ips_info_.pathset_id);
        op_ = op_type_e::IGNORE;
        return;
    }
    if (ips_info_.nh_del_count[nh_type] > 0) {
        // Valid NH del without BH
        if (ips_info_.nh_count[nh_type] > 0) {
            // 2 -> 1
            PDS_TRACE_DEBUG("%s Pathset %d update, 2 -> 1",
                            psstr_(ips_info_.ps_type), ips_info_.pathset_id);
            op_ = op_type_e::UPDATE_DEL;
            return;
        }
        // -> 0 cannot happen unless there is blackhole
        SDK_ASSERT(0);
    }
    PDS_TRACE_DEBUG("%s Pathset %d ignore",
                    psstr_(ips_info_.ps_type), ips_info_.pathset_id);
    op_ = op_type_e::IGNORE;
    return;
}

bool hals_ecmp_t::parse_ips_info_(ATG_NHPI_ADD_UPDATE_ECMP* add_upd_ecmp_ips) {

    // Only first 32 bits of the correlator are used for Pathset ID
    // Ref: psm_fte.cpp -> psm::Fte::increment_corr
    SDK_ASSERT (add_upd_ecmp_ips->pathset_id.correlator2 == 0);
    NBB_CORR_GET_VALUE (ips_info_.pathset_id, add_upd_ecmp_ips->pathset_id);

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
    if (op_ == op_type_e::CREATE) {
        num_repeats = 1;
    } else {
        if (op_ == op_type_e::UPDATE_DEL) {
            // The only removal allowed is when the number of nexthops in the Group
            // gets cut by half due to a link failure.
            // In this case the remaining set of nexthops need to repeated twice
            PDS_TRACE_DEBUG("MS ECMP %ld Update with removal - setting repeat to 2",
                            ips_info_.pathset_id);
            num_repeats = 2;
        } else {
            // NH Group Update to add NH entries assumes recovery from the
            // optimized NH removal case above where the actual number of
            // NH entries in the group was never reduced in the datapath.
            // Reclaim the removed NH entries in the NH Group.
            PDS_TRACE_DEBUG("MS ECMP %ld Update with addition - setting repeat to 1",
                            ips_info_.pathset_id);
            num_repeats = 1;
        }
    }

    for (int repeat = 0; repeat < num_repeats; ++repeat) {
        for (auto& nh: ips_info_.nexthops) {
            // Nexthop key is unused
            nhgroup_spec.nexthops[i].type = PDS_NH_TYPE_UNDERLAY;

            // Fetch underlay L3 interface UUID from If Store
            auto phy_if_obj = state_ctxt.state()->if_store().get(nh.ms_ifindex);
            if (phy_if_obj == nullptr) {
                throw Error(std::string("Underlay ECMP with unknown dest interface ")
                            .append(std::to_string (nh.ms_ifindex)));
            }
            nhgroup_spec.nexthops[i].l3_if =
                phy_if_obj->phy_port_properties().l3_if_spec.key;

            memcpy(nhgroup_spec.nexthops[i].underlay_mac, nh.mac_addr.m_mac,
                   ETH_ADDR_LEN);
            PDS_TRACE_DEBUG("MS ECMP %ld Add NH MSIfIndex 0x%lx PDSIf"
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
    nhgroup_spec.num_nexthops = ips_info_.nexthops.size();
    int i = 0;
    for (auto& nh: ips_info_.nexthops) {
        nhgroup_spec.nexthops[i].type = PDS_NH_TYPE_OVERLAY;

        auto vxp_if_obj = state_ctxt.state()->if_store().get(nh.ms_ifindex);
        SDK_ASSERT(vxp_if_obj != nullptr);
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
        PDS_TRACE_DEBUG("Add Type5 TEP %s VNI %d Idx %s UUID %s to Overlay NHGroup %s",
                        ipaddr2str(&vxp_prop.tep_ip), vxp_prop.vni,
                        vxp_prop.hal_tep_idx.str(),
                        nhgroup_spec.nexthops[i].tep.str(),
                        nhgroup_spec.key.str());
        ++i;
    }
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
                               ms_ps_id_t dp_corr, bool pds_status) {
    NBB_CREATE_THREAD_CONTEXT
    NBS_ENTER_SHARED_CONTEXT(hals_proc_id);
    NBS_GET_SHARED_DATA();

    NBB_CORR_PUT_VALUE(add_upd_ecmp_ips->dp_correlator, dp_corr);
    // Copy all nexthops to the programmed next_hop_objects
    auto list_p = &add_upd_ecmp_ips->next_hop_objects;
    ATG_NHPI_APPENDED_NEXT_HOP appended_next_hop;

    for (auto next_hop = NHPI_GET_FIRST_NH(add_upd_ecmp_ips, list_p);
         next_hop != NULL;
         next_hop = NHPI_GET_NEXT_NH(add_upd_ecmp_ips, list_p, next_hop)) {

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
        auto send_response =
            hals::Ecmp::set_ips_rc(&add_upd_ecmp_ips->ips_hdr,
                                   (pds_status)?ATG_OK:ATG_UNSUCCESSFUL);
        SDK_ASSERT(send_response);
        hals::Fte::get().get_nhpi_join()->
            send_ips_reply(&add_upd_ecmp_ips->ips_hdr);
    } else {
        if (pds_status) {
            (*it)->update_complete(ATG_OK);
        } else {
            (*it)->update_failed(ATG_UNSUCCESSFUL);
        }
    }
    NBS_RELEASE_SHARED_DATA();
    NBS_EXIT_SHARED_CONTEXT();
    NBB_DESTROY_THREAD_CONTEXT
}

NBB_BYTE hals_ecmp_t::handle_indirect_ps_update_
                            (ATG_NHPI_ADD_UPDATE_ECMP* add_upd_ecmp_ips) {
    NBB_BYTE rc = ATG_OK;

    // Received Pathset with Non-direct nextxhops.
    // Check if this is an Indirect Pathset that is referenced by
    // existing VXLAN Tunnels (TEPs)
    auto state_ctxt = pds_ms::state_t::thread_context();
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
        return ip_track_reachability_change(destip, ips_info_.direct_ps_dpcorr,
                                            ip_track_obj->pds_obj_id());
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
                               direct_ps_dpcorr, pds_status);
        };
    return tnl.handle_upathset_update(std::move(state_ctxt),
                                      tep_obj, direct_ps_dpcorr,
                                      std::move(cookie_uptr_));
}


NBB_BYTE hals_ecmp_t::handle_add_upd_ips(ATG_NHPI_ADD_UPDATE_ECMP* add_upd_ecmp_ips) {
    NBB_BYTE rc = ATG_OK;

    parse_ips_info_(add_upd_ecmp_ips);

    if (ips_info_.ps_type == ps_type_e::UNKNOWN) {
        if (ips_info_.nh_del_count[PDS_MS_NH_DEST_NH] > 0) {
            ips_info_.ps_type = ps_type_e::INDIRECT;
        } else if (ips_info_.nh_del_count[PDS_MS_NH_DEST_PORT] > 0) {
            if (mgmt_state_t::thread_context().state()->
                overlay_routing_en()) {
                // Cacscaded mode - cascaded flag is not set - should be overlay
                ips_info_.ps_type = ps_type_e::OVERLAY;
            } else {
                // Squashed mode
                ips_info_.ps_type = ps_type_e::DIRECT;
            }
        } else {
            PDS_TRACE_DEBUG("Ignore pathset %d containing unknown nexthops",
                            ips_info_.pathset_id);
            return rc;
        }
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
        if (op_ == op_type_e::VALID_TO_BH) {
            PDS_TRACE_DEBUG("Pathset %d treating Valid -> BH as Delete for"
                            " Underlay Direct", ips_info_.pathset_id);
            op_ = op_type_e::DELETE;
        } else if (op_ == op_type_e::BH_TO_VALID) {
            PDS_TRACE_DEBUG("Pathset %d treating BH -> Valid as Create for"
                            " Underlay Direct", ips_info_.pathset_id);
            op_ = op_type_e::CREATE;
        }

        auto num_nexthops = ips_info_.nh_count[PDS_MS_NH_DEST_PORT];
        if (op_ == op_type_e::CREATE) {
            PDS_TRACE_DEBUG ("Underlay Pathset %ld: Create IPS Num nexthops %ld",
                             ips_info_.pathset_id, num_nexthops);
        } else {
            if (op_ == op_type_e::UPDATE_DEL) {
                // NH Group Update with NH removal
                // Optimization to quickly update ECMP Group in-place in case of
                // link failure without waiting for BGP keep-alive timeout.
                auto prev_num_nexthops =
                    (num_nexthops + ips_info_.nh_del_count[PDS_MS_NH_DEST_PORT]
                     - ips_info_.nh_add_count[PDS_MS_NH_DEST_PORT]);

                // Only removal of exactly half the nexthops is supported for an
                // NH Group update. Since we cannot change the actual number of
                // NH entries for the NH group in the datapath simulate removal
                // repeat the active NH entry in the place of the inactive NH entry.
                if ((num_nexthops*2) != prev_num_nexthops) {
                    // Ignore this optimized update -
                    // MS will anyway program a separate NH Group that does not have
                    // the deleted nexthops when the routing protocol converges and
                    // then re-program each TEP with the new ECMP group
                    PDS_TRACE_ERR("Underlay Pathset %ld Update - Number of nexthops"
                                  " %d needs to be half of previous number %d -"
                                  " Ignore this update", ips_info_.pathset_id,
                                  num_nexthops, prev_num_nexthops);
                    return rc;
                }
                PDS_TRACE_DEBUG ("Underlay Pathset %ld NH Removal"
                                 " optimization %d -> %d",
                                 ips_info_.pathset_id, prev_num_nexthops,
                                 num_nexthops);
            } else if (op_ == op_type_e::UPDATE_ADD) {
                // Adding new Nexthops to an existing NH Group is not supported.
                // But the corner case of recovery from the link failure
                // optimization above before BGP timeout needs to be handled
                // to avoid permanently excluding the recovered link from the
                // NH Group since we tweaked the NH Group in datapath
                // without BGP's knowledge.
                // TODO - assuming that since the total number of NH entries
                // in the ECMP Group never changed reclaim the unused entries
                // now.
                auto num_nexthops = ips_info_.nh_count[PDS_MS_NH_DEST_PORT];
                auto prev_num_nexthops =
                    (num_nexthops - ips_info_.nh_add_count[PDS_MS_NH_DEST_PORT]
                     + ips_info_.nh_del_count[PDS_MS_NH_DEST_PORT]);
                PDS_TRACE_INFO ("Underlay Pathset %ld recovery from"
                                "optimization %d -> %d", ips_info_.pathset_id,
                                prev_num_nexthops, num_nexthops);
            } else if (op_ == op_type_e::DELETE) {
                PDS_TRACE_DEBUG("Pathset %d Treat as Delete",
                                ips_info_.pathset_id);
            }
        }
    }

    cookie_t* cookie = nullptr;
    pds_batch_ctxt_guard_t  pds_bctxt_guard;

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = pds_ms::state_t::thread_context();

        // Alloc new cookie to capture async info
        cookie_uptr_.reset (new cookie_t);

        if (ips_info_.ps_type == ps_type_e::OVERLAY) {
            fetch_store_info_(state_ctxt.state());

            if ((op_ == op_type_e::BH_TO_VALID) &&
                (store_info_.pathset_obj != nullptr)) {
                // Recovering from initial BH
                PDS_TRACE_DEBUG("Overlay Pathset %d treating BH->Valid as Create",
                                ips_info_.pathset_id);
                op_ = op_type_e::CREATE;
            }

            // If op is not create then there should be a valid
            // store obj retrieved
            SDK_ASSERT(op_ == op_type_e::CREATE ||
                       store_info_.pathset_obj != nullptr);

            if (likely(store_info_.pathset_obj == nullptr)) {
                pathset_obj_uptr_t pathset_obj_uptr
                    (new pathset_obj_t(ips_info_.pathset_id));
                store_info_.pathset_obj = pathset_obj_uptr.get();

                // Cache the new object in the cookie to revisit asynchronously
                // when the PDS API response is received
                cookie_uptr_->objs.push_back(std::move(pathset_obj_uptr));

                PDS_TRACE_DEBUG ("Overlay Pathset %ld: Create IPS Num nexthops %ld",
                                 ips_info_.pathset_id, ips_info_.nexthops.size());
            } else {
                // TODO: Handle Overlay ECMP Update if needed after checking with MS
                PDS_TRACE_ERR ("Overlay Pathset %ld: Update IPS Num nexthops %ld"
                               " NOT SUPPORTED",
                               ips_info_.pathset_id, ips_info_.nexthops.size());
                return rc;
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
                }
                PDS_TRACE_DEBUG("++++ %s Pathset %d NHgroup %d Num NH %d Async reply"
                                " %s ++++", (l_overlay) ? "Overlay": "Underlay",
                                pathset_id, dp_corr, l_num_nh,
                                (pds_status) ? "Success" : "Failure");

                send_ips_response_(add_upd_ecmp_ips, pathset_id,
                                   dp_corr, pds_status);
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
    pds_batch_ctxt_guard_t  pds_bctxt_guard;
    op_ = op_type_e::DELETE;
    bool l_underlay = true;

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
        if (store_info_.pathset_obj != nullptr) {
            l_underlay = false;
            ips_info_.ps_type = ps_type_e::OVERLAY;
            PDS_TRACE_DEBUG ("Overlay Pathset %ld: Delete IPS",
                             ips_info_.pathset_id);
        } else {
            ips_info_.ps_type = ps_type_e::DIRECT;
            PDS_TRACE_DEBUG ("Underlay Pathset %ld: Delete IPS",
                             ips_info_.pathset_id);
        }
        // Empty cookie to force async PDS.
        cookie_uptr_.reset (new cookie_t);
        pds_bctxt_guard = make_batch_pds_spec_ (state_ctxt);

    } // End of state thread_context
      // Do Not access/modify global state after this

    auto pathset_id = ips_info_.pathset_id;
    cookie_uptr_->send_ips_reply =
        [pathset_id, l_underlay] (bool pds_status, bool ips_mock) -> void {
            //-----------------------------------------------------------------
            // This block is executed asynchronously when PDS response is rcvd
            //-----------------------------------------------------------------
            PDS_TRACE_DEBUG("++++ %s Pathset %ld delete %s Async reply ++++",
                            (l_underlay) ? "Underlay" : "Overlay",
                            pathset_id, (pds_status) ? "Success" : "Failure");

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
    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = pds_ms::state_t::thread_context();
        state_ctxt.state()->pathset_store().erase(ips_info_.pathset_id);
    }
    PDS_TRACE_DEBUG ("%s Pathset %ld delete PDS Batch commit successful",
                     (l_underlay) ? "Underlay" : "Overlay",
                     ips_info_.pathset_id);
}

} // End namespace
