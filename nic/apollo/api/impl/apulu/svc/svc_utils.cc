//------------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
///
/// \file
/// apulu pipeline protobuf internal helper functions
///
//------------------------------------------------------------------------------

#include "nic/apollo/api/impl/apulu/svc/svc_utils.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_PIPELINE_SVC - pipeline protobuf helper implementation
/// \ingroup PDS_PIPELINE
/// \@{

sdk_ret_t
pds_svc_serialize_proto_msg (upg_obj_info_t *upg_info, upg_obj_tlv_t *tlv,
                             google::protobuf::Message *proto_msg)
{
    sdk_ret_t ret;
    bool serialize_ret;
    uint32_t obj_size, meta_size, size_left;

    meta_size = sizeof(upg_obj_tlv_t);
    size_left = upg_info->backup.size_left;
    obj_size = proto_msg->ByteSizeLong();

    if ((obj_size + meta_size) > size_left) {
        return SDK_RET_OOM;
    }
    tlv->len = obj_size;
    serialize_ret = proto_msg->SerializeToArray(tlv->obj, tlv->len);
    ret = ((serialize_ret == true) ? SDK_RET_OK : SDK_RET_OOM);
    upg_info->size = obj_size + meta_size;

    return ret;
}

/// \@}    // end of PDS_PIPELINE_SVC

}   // namespace impl
}   // namespace api
