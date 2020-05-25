//------------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
///
/// \file
/// apulu pipeline protobuf internal helper functions declaration
///
//------------------------------------------------------------------------------

#ifndef __APULU_SVC_UTILS_HPP__
#define __APULU_SVC_UTILS_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/internal/upgrade_ctxt.hpp"
#include "gen/proto/types.pb.h"

namespace api {
namespace impl {

/// \defgroup PDS_PIPELINE_SVC - pipeline protobuf helper implementation
/// \ingroup PDS_PIPELINE
/// \@{

/// \brief     utility method to serialize pipeline objs into persistent storage
/// \param[in] upg_info contains location to put stashed object
/// \param[in] upg_obj_tlv contains meta for obj being stashed
/// \param[in] proto_msg protobuf message
sdk_ret_t pds_svc_serialize_proto_msg(upg_obj_info_t *upg_info, upg_obj_tlv_t *tlv,
                                      google::protobuf::Message *proto_msg);


/// \@}    // end of PDS_PIPELINE_SVC

}   // namespace impl
}   // namespace api

#endif  // __APULU_SVC_UTILS_HPP__
