//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of nexthop
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/nexthop.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/impl/apulu/nexthop_impl.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"
#include "nic/apollo/api/impl/apulu/svc/nh_svc.hpp"
#include "nic/apollo/api/impl/apulu/svc/svc_utils.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_NEXTHOP_IMPL - nexthop datapath implementation
/// \ingroup PDS_NEXTHOP
/// \@{

nexthop_impl *
nexthop_impl::factory(pds_nexthop_spec_t *spec) {
    nexthop_impl *impl;

    impl = nexthop_impl_db()->alloc();
    new (impl) nexthop_impl();
    return impl;
}

void
nexthop_impl::destroy(nexthop_impl *impl) {
    impl->~nexthop_impl();
    nexthop_impl_db()->free(impl);
}

impl_base *
nexthop_impl::clone(void) {
    nexthop_impl *cloned_impl;

    cloned_impl = nexthop_impl_db()->alloc();
    new (cloned_impl) nexthop_impl();
    // deep copy is not needed as we don't store pointers
    *cloned_impl = *this;
    return cloned_impl;
}

sdk_ret_t
nexthop_impl::free(nexthop_impl *impl) {
    destroy(impl);
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_impl::reserve_resources(api_base *api_obj, api_base *orig_obj,
                                api_obj_ctxt_t *obj_ctxt) {
    uint32_t idx;
    sdk_ret_t ret;
    pds_nexthop_spec_t *spec;

    // if this object is restored from persistent storage
    // resources are reserved already
    if (api_obj->in_restore_list()) {
        return SDK_RET_OK;
    }

    switch (obj_ctxt->api_op) {
    case API_OP_CREATE:
        spec = &obj_ctxt->api_params->nexthop_spec;
        // record the fact that resource reservation was attempted
        // NOTE: even if we partially acquire resources and fail eventually,
        //       this will ensure that proper release of resources will happen
        api_obj->set_rsvd_rsc();
        // for blackhole nexthop, (re)use PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID
        if (spec->type != PDS_NH_TYPE_BLACKHOLE) {
            // reserve an entry in NEXTHOP table
            ret = nexthop_impl_db()->nh_idxr()->alloc(&idx);
            if (ret != SDK_RET_OK) {
                PDS_TRACE_ERR("Failed to reserve entry in NEXTHOP table, "
                              "err %u", ret);
                return ret;
            }
            hw_id_ = idx;
        } else {
            hw_id_ = PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID;
        }
        break;

    case API_OP_UPDATE:
        // we will use the same h/w resources as the original object
    default:
        break;
    }
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_impl::release_resources(api_base *api_obj) {
    if ((hw_id_ != PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID) &&
        (hw_id_ != 0xFFFFFFFF)) {
        return nexthop_impl_db()->nh_idxr()->free(hw_id_);
    }
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_impl::nuke_resources(api_base *api_obj) {
    // for indexer, release and nuke operations are same
    return this->release_resources(api_obj);
}

sdk_ret_t
nexthop_impl::activate_create_update_(pds_epoch_t epoch, nexthop *nh,
                                      pds_nexthop_spec_t *spec) {
    sdk_ret_t ret;
    pds_encap_t encap;
    nexthop_info_entry_t nh_data;

    memset(&nh_data, 0, nh_data.entry_size());
    switch (spec->type) {
    case PDS_NH_TYPE_BLACKHOLE:
        // nothing to program for system-wide blackhole nexthop
        break;

    case PDS_NH_TYPE_UNDERLAY:
        ret = populate_underlay_nh_info_(spec, &nh_data);
        if (ret != SDK_RET_OK) {
            return ret;
        }
        ret = nh_data.write(hw_id_);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to program nexthop %s at idx %u",
                          spec->key.str(), hw_id_);
            return sdk::SDK_RET_HW_PROGRAM_ERR;
        }
        break;

    default:
        PDS_TRACE_ERR("Failed to activate nexthop %s, unknown nexthop type %u",
                      spec->key.str(), spec->type);
        return SDK_RET_INVALID_ARG;
        break;
    }
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_impl::activate_delete_(pds_epoch_t epoch, nexthop *nh) {
    pds_obj_key_t key;
    sdk_ret_t ret;
    nexthop_info_entry_t nh_data;

    key = nh->key();
    if ((unlikely(hw_id_ == PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID))) {
        return SDK_RET_OK;
    }
    memset(&nh_data, 0, nh_data.entry_size());
    ret = nh_data.write(hw_id_);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to program nexthop %s at idx %u",
                      key.str(), hw_id_);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_impl::activate_hw(api_base *api_obj, api_base *orig_obj,
                          pds_epoch_t epoch, api_op_t api_op,
                          api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    pds_nexthop_spec_t *spec;

    switch (api_op) {
    case API_OP_CREATE:
    case API_OP_UPDATE:
        spec = &obj_ctxt->api_params->nexthop_spec;
        ret = activate_create_update_(epoch, (nexthop *)api_obj, spec);
        break;

    case API_OP_DELETE:
        // spec is not available for DELETE operations
        ret = activate_delete_(epoch, (nexthop *)api_obj);
        break;

    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }
    return ret;
}

sdk_ret_t
nexthop_impl::reprogram_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
nexthop_impl::reactivate_hw(api_base *api_obj, pds_epoch_t epoch,
                            api_obj_ctxt_t *obj_ctxt) {
    return SDK_RET_OK;
}
void
nexthop_impl::fill_status_(pds_nexthop_status_t *status,
                           nexthop_info_entry_t *nh_data) {
    status->hw_id = hw_id_;
    status->port = nh_data->get_port();
    status->vlan = nh_data->get_vlan();
}

sdk_ret_t
nexthop_impl::fill_spec_(pds_nexthop_spec_t *spec,
                         nexthop_info_entry_t *nh_data) {
    if ((unlikely(hw_id_ == PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID))) {
        spec->type = PDS_NH_TYPE_BLACKHOLE;
    } else {
        spec->type = PDS_NH_TYPE_UNDERLAY;
        MAC_UINT64_TO_ADDR(spec->underlay_mac, nh_data->get_dmaco());
        // TODO walk if db and identify the l3_if
    }
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_impl::fill_info_(pds_nexthop_info_t *info) {
    sdk_ret_t ret;
    nexthop_info_entry_t nh_data;

    memset(&nh_data, 0, nh_data.entry_size());
    ret = nh_data.read(hw_id_);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to read nexthop table at index %u", hw_id_);
        return sdk::SDK_RET_HW_READ_ERR;
    }

    fill_spec_(&info->spec, &nh_data);
    fill_status_(&info->status, &nh_data);

    return SDK_RET_OK;
}

sdk_ret_t
nexthop_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    sdk_ret_t rv;
    pds_nexthop_info_t *nh_info = (pds_nexthop_info_t *)info;

    rv = fill_info_(nh_info);
    if (unlikely(rv != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to read NEXTHOP %s table entry",
                      api_obj->key2str().c_str());
        return rv;
    }
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_impl::backup(obj_info_t *info, upg_obj_info_t *upg_info) {
    sdk_ret_t ret;
    pds::NexthopGetResponse proto_msg;
    pds_nexthop_info_t *nh_info;
    upg_obj_tlv_t *tlv;

    tlv = (upg_obj_tlv_t *)upg_info->mem;
    nh_info = (pds_nexthop_info_t *)info;

    ret = fill_info_(nh_info);
    if (unlikely(ret != SDK_RET_OK)) {
        return ret;
    }
    // convert api info to proto
    pds_nh_api_info_to_proto(nh_info, (void *)&proto_msg);
    ret = pds_svc_serialize_proto_msg(upg_info, tlv, &proto_msg);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to serialize nh group %s err %u",
                      nh_info->spec.key.str(), ret);
    }
    return ret;
}

sdk_ret_t
nexthop_impl::restore_resources(obj_info_t *info) {
    sdk_ret_t ret;
    pds_nexthop_info_t *nh_info;
    pds_nexthop_spec_t *spec;
    pds_nexthop_status_t *status;

    nh_info = (pds_nexthop_info_t *)info;
    spec = &nh_info->spec;
    status = &nh_info->status;

    if (spec->type != PDS_NH_TYPE_BLACKHOLE) {
        ret = nexthop_impl_db()->nh_idxr()->alloc(status->hw_id);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to restore entry in NEXTHOP table, "
                          "err %u hw id %u", ret, status->hw_id);
            return ret;
        }
        hw_id_ = status->hw_id;
    } else {
        hw_id_ =  PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID;
    }
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_impl::restore(obj_info_t *info, upg_obj_info_t *upg_info) {
    sdk_ret_t ret;
    pds::NexthopGetResponse proto_msg;
    pds_nexthop_info_t *nh_info;
    upg_obj_tlv_t *tlv;
    uint32_t obj_size, meta_size;

    tlv = (upg_obj_tlv_t *)upg_info->mem;
    nh_info = (pds_nexthop_info_t *)info;
    obj_size = tlv->len;
    meta_size = sizeof(upg_obj_tlv_t);
    // fill up the size, even if it fails later. to try and restore next obj
    upg_info->size = obj_size + meta_size;
    // de-serialize proto msg
    if (proto_msg.ParseFromArray(tlv->obj, tlv->len) == false) {
        PDS_TRACE_ERR("Failed to de-serialize nexthop");
        return SDK_RET_OOM;
    }
    // convert proto msg to nexthop info
    ret = pds_nh_proto_to_api_info(nh_info, &proto_msg);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to convert nexthop proto msg to info, err %u",
                      ret);
        return ret;
    }
    // now restore hw resources
    ret = restore_resources((obj_info_t *)nh_info);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to restore hw resources for nexthop %s, err %u",
                      nh_info->spec.key.str(), ret);
    }
    return ret;
}

/// \@}    // end of PDS_NEXTHOP_IMPL

}    // namespace impl
}    // namespace api
