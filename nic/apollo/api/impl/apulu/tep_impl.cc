//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// TEP datapath implementation
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/tep.hpp"
#include "nic/apollo/api/impl/apulu/tep_impl.hpp"
#include "nic/apollo/api/impl/apulu/nexthop_impl.hpp"
#include "nic/apollo/api/impl/apulu/nexthop_group_impl.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_TEP_IMPL - tep datapath implementation
/// \ingroup PDS_TEP
/// @{

tep_impl *
tep_impl::factory(pds_tep_spec_t *pds_tep) {
    tep_impl *impl;

    impl = tep_impl_db()->alloc();
    new (impl) tep_impl();
    return impl;
}

void
tep_impl::destroy(tep_impl *impl) {
    impl->~tep_impl();
    tep_impl_db()->free(impl);
}

impl_base *
tep_impl::clone(void) {
    tep_impl *cloned_impl;

    cloned_impl = tep_impl_db()->alloc();
    new (cloned_impl) tep_impl();
    // deep copy is not needed as we don't store pointers
    *cloned_impl = *this;
    return cloned_impl;
}

sdk_ret_t
tep_impl::free(tep_impl *impl) {
    destroy(impl);
    return SDK_RET_OK;
}

sdk_ret_t
tep_impl::reserve_resources(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    tep_entry *tep;
    uint32_t idx1 = 0xFFFF, idx2 = 0xFFFF;
    pds_tep_spec_t *spec = &obj_ctxt->api_params->tep_spec;

    switch (obj_ctxt->api_op) {
    case API_OP_CREATE:
        // record the fact that resource reservation was attempted
        // NOTE: even if we partially acquire resources and fail eventually,
        //       this will ensure that proper release of resources will happen
        api_obj->set_rsvd_rsc();
        if (spec->nh_type == PDS_NH_TYPE_OVERLAY) {
            tep = tep_db()->find(&spec->tep);
            if (tep == NULL) {
                PDS_TRACE_ERR("Failed to find nexthop TEP %s used by TEP %s",
                              spec->tep.str(), spec->key.str());
                return SDK_RET_INVALID_ARG;
            }
        }
        if (spec->type == PDS_TEP_TYPE_INTER_DC) {
            // allocate a resource in TUNNEL2 table
            ret = tep_impl_db()->tunnel2_idxr()->alloc(&idx1);
        } else {
            // allocate a resource in TUNNEL and TUNNEL2 tables
            ret = tep_impl_db()->tunnel_idxr()->alloc(&idx1);
            if (likely(ret == SDK_RET_OK)) {
                ret = tep_impl_db()->tunnel2_idxr()->alloc(&idx2);
                if (ret != SDK_RET_OK) {
                    PDS_TRACE_ERR("Failed to reserve entry in TUNNEL2 table for %s, err %u",
                                  api_obj->key2str().c_str(), ret);
                    return ret;
                }
            }
        }
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to reserve entry in %s table for %s, err %u",
                          (spec->type == PDS_TEP_TYPE_INTER_DC) ?
                              "TUNNEL2" : "TUNNEL",
                          api_obj->key2str().c_str(), ret);
            return ret;
        }
        hw_id1_ = idx1;
        hw_id2_ = idx2;
        break;

    case API_OP_UPDATE:
        // we will use the same h/w resources as the original object
    default:
        break;
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_impl::release_resources(api_base *api_obj) {
    tep_entry *tep;

    tep = (tep_entry *)api_obj;
    if (hw_id1_ != 0xFFFF) {
        if (tep->type() == PDS_TEP_TYPE_INTER_DC) {
            tep_impl_db()->tunnel2_idxr()->free(hw_id1_);
        } else {
            tep_impl_db()->tunnel_idxr()->free(hw_id1_);
        }
    }
    if (hw_id2_ != 0xFFFF) {
        tep_impl_db()->tunnel2_idxr()->free(hw_id2_);
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_impl::nuke_resources(api_base *api_obj) {
    tep_entry *tep;

    tep = (tep_entry *)api_obj;
    if (hw_id1_ != 0xFFFF) {
        if (tep->type() == PDS_TEP_TYPE_INTER_DC) {
            tep_impl_db()->tunnel2_idxr()->free(hw_id1_);
        } else {
            tep_impl_db()->tunnel_idxr()->free(hw_id1_);
        }
    }
    if (hw_id2_ != 0xFFFF) {
        tep_impl_db()->tunnel2_idxr()->free(hw_id2_);
    }
    return SDK_RET_OK;
}

#define PDS_NUM_NH_NO_ECMP 1
#define tunnel_action     action_u.tunnel_tunnel_info
#define tunnel2_action    action_u.tunnel2_tunnel2_info
sdk_ret_t
tep_impl::activate_create_tunnel_table_(pds_epoch_t epoch, tep_entry *tep,
                                        pds_tep_spec_t *spec) {
    nexthop *nh;
    sdk_ret_t ret;
    tep_entry *tep2;
    pds_obj_key_t nh_key;
    p4pd_error_t p4pd_ret;
    nexthop_impl *nh_impl;
    nexthop_group *nhgroup;
    bool program_tep2 = false;
    pds_obj_key_t nhgroup_key;
    nexthop_group_impl *nhgroup_impl;
    tunnel_actiondata_t tep_data = { 0 };
    tunnel2_actiondata_t tep2_data = { 0 };

    if (spec->nh_type == PDS_NH_TYPE_UNDERLAY_ECMP) {
        nhgroup = nexthop_group_db()->find(&spec->nh_group);
        if (unlikely(nhgroup == NULL)) {
            PDS_TRACE_ERR("nhgroup %s in TEP %s not found",
                          spec->nh_group.str(), spec->key.str());
            return SDK_RET_INVALID_ARG;
        }
        nhgroup_impl = (nexthop_group_impl *)nhgroup->impl();
        tep_data.tunnel_action.nexthop_base = nhgroup_impl->nh_base_hw_id();
        tep_data.tunnel_action.num_nexthops = nhgroup->num_nexthops();
        program_tep2 = true;
    } else if (spec->nh_type == PDS_NH_TYPE_UNDERLAY) {
        nh = nexthop_db()->find(&spec->nh);
        if (unlikely(nh == NULL)) {
            PDS_TRACE_ERR("nh %s in TEP %s not found",
                          spec->nh.str(), spec->key.str());
            return SDK_RET_INVALID_ARG;
        }
        nh_impl = (nexthop_impl *) nh->impl();
        tep_data.tunnel_action.nexthop_base = nh_impl->hw_id();
        tep_data.tunnel_action.num_nexthops = PDS_NUM_NH_NO_ECMP;
        program_tep2 = true;
    } else if (spec->nh_type == PDS_NH_TYPE_OVERLAY) {
        // tunnel pointing to another tunnel case, do recursive resolution
        tep2 = tep_db()->find(&spec->tep);
        if (unlikely(tep2 != NULL)) {
            PDS_TRACE_ERR("tep %s in overlay TEP %s not found",
                          spec->tep.str(), spec->key.str());
            return SDK_RET_INVALID_ARG;
        }
        if (tep2->nh_type() == PDS_NH_TYPE_UNDERLAY) {
            nh_key = tep2->nh();
            nh_impl = (nexthop_impl *)nexthop_db()->find(&nh_key)->impl();
            tep_data.tunnel_action.nexthop_base = nh_impl->hw_id();
            tep_data.tunnel_action.num_nexthops = PDS_NUM_NH_NO_ECMP;
        } else if (tep2->nh_type() == PDS_NH_TYPE_UNDERLAY_ECMP) {
            nhgroup_key = tep2->nh_group();
            nhgroup = nexthop_group_db()->find(&spec->nh_group);
            tep_data.tunnel_action.nexthop_base =
                ((nexthop_group_impl *)nhgroup->impl())->hw_id();
            tep_data.tunnel_action.num_nexthops = nhgroup->num_nexthops();
        }
    } else {
        PDS_TRACE_ERR("Unsupported nh type %u in TEP %s spec",
                      spec->nh_type, spec->key.str());
        SDK_ASSERT_RETURN(false, SDK_RET_INVALID_ARG);
    }
    if (spec->encap.type != PDS_ENCAP_TYPE_NONE) {
        tep_data.tunnel_action.vni = spec->encap.val.value;
    }
    if (spec->remote_ip.af == IP_AF_IPV4) {
        tep_data.tunnel_action.ip_type = IPTYPE_IPV4;
        memcpy(tep_data.tunnel_action.dipo, &spec->remote_ip.addr.v4_addr,
               IP4_ADDR8_LEN);
        if (program_tep2) {
            tep2_data.tunnel2_action.ip_type = IPTYPE_IPV4;
            memcpy(tep2_data.tunnel2_action.dipo, &spec->remote_ip.addr.v4_addr,
                   IP4_ADDR8_LEN);
        }
    } else if (spec->remote_ip.af == IP_AF_IPV6) {
        tep_data.tunnel_action.ip_type = IPTYPE_IPV6;
        sdk::lib::memrev(tep_data.tunnel_action.dipo,
                         spec->remote_ip.addr.v6_addr.addr8,
                         IP6_ADDR8_LEN);
        if (program_tep2) {
            tep2_data.tunnel2_action.ip_type = IPTYPE_IPV6;
            sdk::lib::memrev(tep2_data.tunnel2_action.dipo,
                             spec->remote_ip.addr.v6_addr.addr8,
                             IP6_ADDR8_LEN);
        }
    }
    sdk::lib::memrev(tep_data.tunnel_action.dmaci, spec->mac, ETH_ADDR_LEN);
    tep_data.action_id = TUNNEL_TUNNEL_INFO_ID;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_TUNNEL, hw_id1_,
                                       NULL, NULL, &tep_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program TEP %s at idx %u",
                      spec->key.str(), hw_id1_);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    if (program_tep2) {
        tep2_data.action_id = TUNNEL2_TUNNEL2_INFO_ID;
        tep2_data.tunnel2_action.encap_type = TX_REWRITE_ENCAP_VXLAN;
        p4pd_ret = p4pd_global_entry_write(P4TBL_ID_TUNNEL2, hw_id2_,
                                           NULL, NULL, &tep2_data);
        if (p4pd_ret != P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to program TEP %s in TUNNEL2 table at idx %u",
                          spec->key.str(), hw_id2_);
            return sdk::SDK_RET_HW_PROGRAM_ERR;
        }
    }
    return SDK_RET_OK;
}

#define tunnel2_action    action_u.tunnel2_tunnel2_info
sdk_ret_t
tep_impl::activate_create_tunnel2_(pds_epoch_t epoch, tep_entry *tep,
                                   pds_tep_spec_t *spec) {
    nexthop *nh;
    sdk_ret_t ret;
    nexthop_impl *nh_impl;
    p4pd_error_t p4pd_ret;
    nexthop_group *nhgroup;
    nexthop_info_entry_t nh_data;
    nexthop_group_impl *nhgroup_impl;
    tunnel2_actiondata_t tep2_data = { 0 };

    // program the TUNNEL2 table entry first
    if (spec->remote_ip.af == IP_AF_IPV4) {
        tep2_data.tunnel2_action.ip_type = IPTYPE_IPV4;
        memcpy(tep2_data.tunnel2_action.dipo, &spec->remote_ip.addr.v4_addr,
               IP4_ADDR8_LEN);
    } else if (spec->remote_ip.af == IP_AF_IPV6) {
        tep2_data.tunnel2_action.ip_type = IPTYPE_IPV6;
        sdk::lib::memrev(tep2_data.tunnel2_action.dipo,
                         spec->remote_ip.addr.v6_addr.addr8,
                         IP6_ADDR8_LEN);
    }
    if (spec->encap.type == PDS_ENCAP_TYPE_MPLSoUDP) {
        tep2_data.tunnel2_action.encap_type = TX_REWRITE_ENCAP_MPLSoUDP;
    } else if (spec->encap.type == PDS_ENCAP_TYPE_VXLAN) {
        tep2_data.tunnel2_action.encap_type = TX_REWRITE_ENCAP_VXLAN;
    }
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_TUNNEL2, hw_id1_,
                                       NULL, NULL, &tep2_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program TEP %s in TUNNEL2 table at idx %u",
                      spec->key.str(), hw_id1_);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    memset(&nh_data, 0, nexthop_info_entry_t::entry_size());
    // now we need to update/fix the nexthop(s)
    if (spec->nh_type == PDS_NH_TYPE_UNDERLAY_ECMP) {
        nhgroup = nexthop_group_db()->find(&spec->nh_group);
        if (unlikely(nhgroup == NULL)) {
            PDS_TRACE_ERR("nhgroup %s in TEP %s not found",
                          spec->nh_group.str(), spec->key.str());
            SDK_ASSERT_RETURN(false, SDK_RET_INVALID_ARG);
        }
        nhgroup_impl = (nexthop_group_impl *)nhgroup->impl();
        for (uint32_t i = 0, nh_idx = nhgroup_impl->nh_base_hw_id();
             i < nhgroup->num_nexthops(); nh_idx++, i++) {
            ret = nh_data.read(nh_idx);
            if (unlikely(ret != SDK_RET_OK)) {
                PDS_TRACE_ERR("Failed to read NEXTHOP table at %u", nh_idx);
                return sdk::SDK_RET_HW_READ_ERR;
            }
            nh_data.set_tunnel2_id(hw_id1_);
            nh_data.set_vlan(spec->encap.val.value);
            ret = nh_data.write(nh_idx);
            if (ret != SDK_RET_OK) {
                PDS_TRACE_ERR("Failed to update NEXTHOP table at %u", nh_idx);
                return sdk::SDK_RET_HW_PROGRAM_ERR;
            }
        }
    } else if (spec->nh_type == PDS_NH_TYPE_UNDERLAY) {
        nh = (nexthop *)nexthop_db()->find(&spec->nh);
        if (unlikely(nh == NULL)) {
            PDS_TRACE_ERR("nh %s in TEP %s not found",
                          spec->nh.str(), spec->key.str());
            SDK_ASSERT_RETURN(false, SDK_RET_INVALID_ARG);
        }
        nh_impl = (nexthop_impl *)nh->impl();
        ret = nh_data.read(nh_impl->hw_id());
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_TRACE_ERR("Failed to read NEXTHOP table at %u",
                          nh_impl->hw_id());
            return sdk::SDK_RET_HW_READ_ERR;
        }
        nh_data.set_tunnel2_id(hw_id1_);
        nh_data.set_vlan(spec->encap.val.value);
        ret = nh_data.write(nh_impl->hw_id());
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to update NEXTHOP table at %u",
                          nh_impl->hw_id());
            return sdk::SDK_RET_HW_PROGRAM_ERR;
        }
    } else {
        PDS_TRACE_ERR("Unsupported nh type %u in TEP %s spec",
                      spec->nh_type, spec->key.str());
        SDK_ASSERT_RETURN(false, SDK_RET_INVALID_ARG);
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_impl::activate_create_(pds_epoch_t epoch, tep_entry *tep,
                           pds_tep_spec_t *spec) {
    sdk_ret_t ret;

    PDS_TRACE_DEBUG("Activating TEP %s create, h/w id1 %u, h/w id2 %u",
                    spec->key.str(), hw_id1_, hw_id2_);
    if (tep->type() == PDS_TEP_TYPE_INTER_DC) {
        // program outer tunnel in double encap case
        ret = activate_create_tunnel2_(epoch, tep, spec);
    } else {
        ret = activate_create_tunnel_table_(epoch, tep, spec);
    }
    return ret;
}

sdk_ret_t
tep_impl::activate_delete_tunnel_table_(pds_epoch_t epoch, tep_entry *tep) {
    p4pd_error_t p4pd_ret;
    tunnel_actiondata_t tep_data;

    // 1st read the TEP entry from h/w
    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_TUNNEL, hw_id1_,
                                      NULL, NULL, &tep_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read TEP %s at TUNNEL table idx %u",
                      tep->key().str(), hw_id1_);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
    // point the tunnel to the blackhole nexthop
    tep_data.tunnel_action.nexthop_base = PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID;
    tep_data.tunnel_action.num_nexthops = 0;
    // update the entry in the TUNNEL table
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_TUNNEL, hw_id1_,
                                       NULL, NULL, &tep_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to update TEP %s at TUNNEL table idx %u",
                      tep->key().str(), hw_id1_);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    // TUNNEL2 table entry doesn't need to be cleaned up because mirrored
    // traffic will be blackholed because of the cleanup to TUNNEL table entry
    return SDK_RET_OK;
}

sdk_ret_t
tep_impl::activate_delete_tunnel2_(pds_epoch_t epoch, tep_entry *tep) {
    sdk_ret_t ret;
    p4pd_error_t p4pd_ret;
    nexthop_impl *nh_impl;
    nexthop_group *nhgroup;
    pds_obj_key_t nh_key;
    nexthop_info_entry_t nh_data;
    nexthop_group_impl *nhgroup_impl;
    pds_obj_key_t nh_group_key;

    memset(&nh_data, 0, nexthop_info_entry_t::entry_size());
    // update/fix the nexthop(s) to not point to this outer tunnel anymore
    if (tep->nh_type() == PDS_NH_TYPE_UNDERLAY_ECMP) {
        nh_group_key = tep->nh_group();
        nhgroup = nexthop_group_db()->find(&nh_group_key);
        nhgroup_impl = (nexthop_group_impl *)nhgroup->impl();
        for (uint32_t i = 0, nh_idx = nhgroup_impl->nh_base_hw_id();
             i < nhgroup->num_nexthops(); nh_idx++, i++) {
            ret = nh_data.read(nh_idx);
            if (unlikely(ret != SDK_RET_OK)) {
                PDS_TRACE_ERR("Failed to read NEXTHOP table at %u", nh_idx);
                return sdk::SDK_RET_HW_READ_ERR;
            }
            nh_data.set_tunnel2_id(0);
            nh_data.set_vlan(0);
            ret = nh_data.write(nh_idx);
            if (ret != SDK_RET_OK) {
                PDS_TRACE_ERR("Failed to update NEXTHOP table at %u", nh_idx);
                return sdk::SDK_RET_HW_PROGRAM_ERR;
            }
        }
    } else if (tep->nh_type() == PDS_NH_TYPE_UNDERLAY) {
        nh_key = tep->nh();
        nh_impl = (nexthop_impl *)nexthop_db()->find(&nh_key)->impl();
        ret = nh_data.read(nh_impl->hw_id());
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_TRACE_ERR("Failed to read NEXTHOP table at %u",
                          nh_impl->hw_id());
            return sdk::SDK_RET_HW_READ_ERR;
        }
        nh_data.set_tunnel2_id(0);
        nh_data.set_vlan(0);
        ret = nh_data.write(nh_impl->hw_id());
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to update NEXTHOP table at %u",
                          nh_impl->hw_id());
            return sdk::SDK_RET_HW_PROGRAM_ERR;
        }
    }
    // we don't need to touch the TUNNEL2 table entry as no entries are pointing
    // to it now !!
    return SDK_RET_OK;
}

sdk_ret_t
tep_impl::activate_delete_(pds_epoch_t epoch, tep_entry *tep) {
    sdk_ret_t ret;

    PDS_TRACE_DEBUG("Activating TEP %s delete, h/w id1 %u, h/w id2 %u",
                    tep->key().str(), hw_id1_, hw_id2_);
    if (tep->type() == PDS_TEP_TYPE_INTER_DC) {
        // cleanup outer tunnel in double encap case
        ret = activate_delete_tunnel2_(epoch, tep);
    } else {
        ret = activate_delete_tunnel_table_(epoch, tep);
    }
    return ret;
}

sdk_ret_t
tep_impl::activate_update_tunnel_table_(pds_epoch_t epoch, tep_entry *tep,
                                        pds_tep_spec_t *spec) {
    return activate_create_tunnel_table_(epoch, tep, spec);
}

sdk_ret_t
tep_impl::activate_update_tunnel2_(pds_epoch_t epoch, tep_entry *tep,
                                   pds_tep_spec_t *spec) {
    return activate_create_tunnel2_(epoch, tep, spec);
}

sdk_ret_t
tep_impl::activate_update_(pds_epoch_t epoch, tep_entry *tep,
                           tep_entry *orig_tep, api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    pds_tep_spec_t *spec;

    spec = &obj_ctxt->api_params->tep_spec;
    PDS_TRACE_DEBUG("Activating TEP %s update, h/w id1 %u, h/w id2 %u",
                    tep->key().str(), hw_id1_, hw_id2_);
    if (tep->type() == PDS_TEP_TYPE_INTER_DC) {
        // update outer tunnel in double encap case
        ret = activate_update_tunnel2_(epoch, tep, spec);
    } else {
        ret = activate_update_tunnel_table_(epoch, tep, spec);
    }
    return ret;
}

sdk_ret_t
tep_impl::activate_hw(api_base *api_obj, api_base *orig_obj, pds_epoch_t epoch,
                      api_op_t api_op, api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    pds_tep_spec_t *spec;

    switch (api_op) {
    case API_OP_CREATE:
        spec = &obj_ctxt->api_params->tep_spec;
        ret = activate_create_(epoch, (tep_entry *)api_obj, spec);
        break;

    case API_OP_DELETE:
        // spec is not available for DELETE operations
        ret = activate_delete_(epoch, (tep_entry *)api_obj);
        break;

    case API_OP_UPDATE:
        ret = activate_update_(epoch, (tep_entry *)api_obj,
                               (tep_entry *)orig_obj, obj_ctxt);
        break;

    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }
    return ret;
}

void
tep_impl::fill_status_(pds_tep_status_t *status) {
    status->hw_id = hw_id1_;
}

sdk_ret_t
tep_impl::fill_spec_(pds_tep_spec_t *spec) {
    p4pd_error_t p4pdret;
    tunnel_actiondata_t tep_data;

    p4pdret = p4pd_global_entry_read(P4TBL_ID_TUNNEL, hw_id1_,
                                     NULL, NULL, &tep_data);
    if (unlikely(p4pdret != P4PD_SUCCESS)) {
        PDS_TRACE_ERR("Failed to read TUNNEL table at idx %u", hw_id1_);
        return sdk::SDK_RET_HW_READ_ERR;
    }
    switch (tep_data.tunnel_action.ip_type) {
    case IPTYPE_IPV4:
        spec->remote_ip.af = IP_AF_IPV4;
        memcpy(&spec->remote_ip.addr.v4_addr, tep_data.tunnel_action.dipo,
               IP4_ADDR8_LEN);
        break;
    case IPTYPE_IPV6:
        spec->remote_ip.af = IP_AF_IPV6;
        sdk::lib::memrev(spec->remote_ip.addr.v6_addr.addr8,
                         tep_data.tunnel_action.dipo,
                         IP6_ADDR8_LEN);
        break;
    default:
        break;
    }
    sdk::lib::memrev(spec->mac, tep_data.tunnel_action.dmaci, ETH_ADDR_LEN);
    if (tep_data.tunnel_action.num_nexthops > PDS_NUM_NH_NO_ECMP) {
        spec->nh_type = PDS_NH_TYPE_UNDERLAY_ECMP;
    } else if (tep_data.tunnel_action.num_nexthops == PDS_NUM_NH_NO_ECMP) {
        spec->nh_type = PDS_NH_TYPE_UNDERLAY;
    }
    if (tep_data.tunnel_action.vni) {
        spec->encap.type = PDS_ENCAP_TYPE_VXLAN;
        spec->encap.val.value = tep_data.tunnel_action.vni;
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    sdk_ret_t rv;
    pds_tep_info_t *tep_info = (pds_tep_info_t *)info;

    rv = fill_spec_(&tep_info->spec);
    if (unlikely(rv != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to read from h/w for TEP %s",
                      api_obj->key2str().c_str());
        return rv;
    }

    fill_status_(&tep_info->status);
    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
