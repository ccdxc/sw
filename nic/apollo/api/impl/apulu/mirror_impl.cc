//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of mirror APIs
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/impl/apulu/tep_impl.hpp"
#include "nic/apollo/api/impl/apulu/mapping_impl.hpp"
#include "nic/apollo/api/impl/apulu/mirror_impl.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_MIRROR_IMPL - mirror entry datapath implementation
/// \ingroup PDS_MIRROR
/// \@{

mirror_impl *
mirror_impl::factory(pds_mirror_session_spec_t *spec) {
    mirror_impl *impl;

    impl = mirror_impl_db()->alloc();
    new (impl) mirror_impl();
    return impl;
}

impl_base *
mirror_impl::clone(void) {
    mirror_impl *cloned_impl;

    cloned_impl = mirror_impl_db()->alloc();
    new (cloned_impl) mirror_impl();
    // deep copy is not needed as we don't store pointers
    *cloned_impl = *this;
    return cloned_impl;
}

void
mirror_impl::destroy(mirror_impl *impl) {
    impl->~mirror_impl();
    mirror_impl_db()->free(impl);
}

sdk_ret_t
mirror_impl::free(mirror_impl *impl) {
    destroy(impl);
    return SDK_RET_OK;
}

sdk_ret_t
mirror_impl::reserve_resources(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    pds_mirror_session_spec_t *spec;

    switch (obj_ctxt->api_op) {
    case API_OP_CREATE:
        // allocate hw id for this session
        spec = &obj_ctxt->api_params->mirror_session_spec;
        return mirror_impl_db()->alloc_hw_id(&spec->key, &hw_id_);
    case API_OP_UPDATE:
        // we will use the same h/w resources as the original object
    default:
        break;
    }
    return SDK_RET_OK;
}

sdk_ret_t
mirror_impl::release_resources(api_base *api_obj) {
    if (hw_id_ != 0xFFFF) {
        return mirror_impl_db()->free_hw_id(hw_id_);
    }
    return SDK_RET_OK;
}

sdk_ret_t
mirror_impl::nuke_resources(api_base *api_obj) {
    if (hw_id_ != 0xFFFF) {
        return mirror_impl_db()->free_hw_id(hw_id_);
    }
    return SDK_RET_OK;
}

#define rspan_action     action_u.mirror_rspan
#define erspan_action    action_u.mirror_erspan
sdk_ret_t
mirror_impl::activate_create_(pds_epoch_t epoch, mirror_session *ms,
                              pds_mirror_session_spec_t *spec) {
    vpc_entry *vpc;
    tep_entry *tep;
    p4pd_error_t p4pd_ret;
    mapping_entry *mapping;
    mirror_actiondata_t mirror_data = { 0 };

    switch (spec->type) {
    case PDS_MIRROR_SESSION_TYPE_RSPAN:
        mirror_data.action_id = MIRROR_RSPAN_ID;
        // TODO: what nh are we supposed to program here ?
        mirror_data.rspan_action.ctag = spec->rspan_spec.encap.val.vlan_tag;
        mirror_data.rspan_action.truncate_len = spec->snap_len;
        break;

    case PDS_MIRROR_SESSION_TYPE_ERSPAN:
        mirror_data.action_id = MIRROR_ERSPAN_ID;
        vpc = vpc_find(&spec->erspan_spec.vpc);
        mirror_data.erspan_action.truncate_len = spec->snap_len;
        if (vpc->type() == PDS_VPC_TYPE_UNDERLAY) {
            // lookup the destination TEP
            tep = tep_find(&spec->erspan_spec.tep);
            // TODO: what if this TEP is local TEP itself ?
            //       - check this in init_config()
            mirror_data.erspan_action.nexthop_type = NEXTHOP_TYPE_TUNNEL;
            mirror_data.erspan_action.nexthop_id =
                ((tep_impl *)(tep->impl()))->hw_id();
            // TODO: what to do with dscp value ?
        } else {
            // mirror destination is either local or remote mapping
            mapping = mapping_entry::build(&spec->erspan_spec.mapping);
            if (mapping == NULL) {
                PDS_TRACE_ERR("Failed to find mapping %s",
                              spec->erspan_spec.mapping.str());
                return SDK_RET_INVALID_ARG;
            }
            mirror_data.erspan_action.nexthop_type =
                ((mapping_impl *)(mapping->impl()))->nexthop_type();
            mirror_data.erspan_action.nexthop_id =
                ((mapping_impl *)(mapping->impl()))->nexthop_id();
        }
        break;

    default:
        return SDK_RET_INVALID_ARG;
    }

    // program the mirror table
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_MIRROR, hw_id_, NULL, NULL,
                                       &mirror_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program mirror session %s at idx %u",
                      spec->key.str(), hw_id_);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
mirror_impl::activate_update_(pds_epoch_t epoch, mirror_session *ms,
                              api_obj_ctxt_t *obj_ctxt) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
mirror_impl::activate_delete_(pds_epoch_t epoch, mirror_session *ms) {
    p4pd_error_t p4pd_ret;
    mirror_actiondata_t mirror_data = { 0 };

    // cleanup the mirror table entry
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_MIRROR, hw_id_, NULL, NULL,
                                       &mirror_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program mirror session %s at idx %u",
                      ms->key2str().c_str(), hw_id_);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
mirror_impl::activate_hw(api_base *api_obj, api_base *orig_obj,
                         pds_epoch_t epoch, api_op_t api_op,
                         api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    pds_mirror_session_spec_t *spec;

    switch (api_op) {
    case API_OP_CREATE:
        spec = &obj_ctxt->api_params->mirror_session_spec;
        ret = activate_create_(epoch, (mirror_session *)api_obj, spec);
        break;

    case API_OP_DELETE:
        // spec is not available for DELETE operations
        ret = activate_delete_(epoch, (mirror_session *)api_obj);
        break;

    case API_OP_UPDATE:
        ret = activate_update_(epoch, (mirror_session *)api_obj, obj_ctxt);
        break;

    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }
    return ret;
}

void
mirror_impl::fill_status_(pds_mirror_session_status_t *status) {
}

sdk_ret_t
mirror_impl::fill_spec_(pds_mirror_session_spec_t *spec) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
mirror_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    //uint16_t hw_id;
    //p4pd_error_t p4pd_ret;
    //mirror_actiondata_t mirror_data;
    sdk_ret_t ret;
    pds_mirror_session_info_t *mirror_session_info = (pds_mirror_session_info_t *)info;

    ret = fill_spec_(&mirror_session_info->spec);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to read hardware spec tables for mirror session %s",
                      api_obj->key2str().c_str());
        return ret;
    }
    fill_status_(&mirror_session_info->status);
    return SDK_RET_OK;

#if 0
    hw_id = mkey->id - 1;
    if ((mirror_impl_db()->session_bmap_ & (1 << hw_id)) == 0) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_MIRROR, hw_id, NULL, NULL,
                                      &mirror_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read mirror session %u at idx %u", mkey->id);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
    minfo->spec.key.id = mkey->id;
    switch (mirror_data.action_id) {
    case MIRROR_RSPAN_ID:
        minfo->spec.type = PDS_MIRROR_SESSION_TYPE_RSPAN;
        minfo->spec.snap_len = mirror_data.rspan_action.truncate_len;
#if 0
        minfo->spec.rspan_spec.interface =
            g_pds_state.catalogue()->tm_port_to_ifindex(mirror_data.rspan_action.tm_oport);
#endif
        minfo->spec.rspan_spec.encap.type = PDS_ENCAP_TYPE_DOT1Q;
        minfo->spec.rspan_spec.encap.val.vlan_tag =
            mirror_data.rspan_action.ctag;
        break;

    case MIRROR_ERSPAN_ID:
        minfo->spec.type = PDS_MIRROR_SESSION_TYPE_ERSPAN;
        minfo->spec.snap_len = mirror_data.erspan_action.truncate_len;
        minfo->spec.erspan_spec.src_ip.addr.v4_addr =
            mirror_data.erspan_action.sip;
        // minfo->spec.erspan_spec.dst_ip.addr.v4_addr =
           //  mirror_data.erspan_action.dip;
        break;

    default:
        PDS_TRACE_ERR("mirror read operation not supported");
        return sdk::SDK_RET_INVALID_OP;
    }
    return SDK_RET_OK;
#endif
}

/// \@}

}    // namespace impl
}    // namespace api
