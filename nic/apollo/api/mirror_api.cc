//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module implements mirror session API
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/include/pds_mirror.hpp"

/// \defgroup PDS_MIRROR_SESSION_API - first level of mirror session
//            API handling
/// @{

/// \brief create a mirror session
/// \param[in] spec mirror session configuration
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t
pds_mirror_session_create (_In_ pds_mirror_session_spec_t *spec)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = api_params_alloc(api::OBJ_ID_MIRROR_SESSION,
                                           api::API_OP_CREATE);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_CREATE;
        api_ctxt.obj_id = api::OBJ_ID_MIRROR_SESSION;
        api_ctxt.api_params->mirror_session_spec = *spec;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return SDK_RET_OOM;
}

/// \brief delete given mirror session
/// \param[in] key    mirror session key
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t
pds_mirror_session_delete (_In_ pds_mirror_session_key_t *key)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = api_params_alloc(api::OBJ_ID_MIRROR_SESSION,
                                           api::API_OP_DELETE);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_DELETE;
        api_ctxt.obj_id = api::OBJ_ID_MIRROR_SESSION;
        api_ctxt.api_params->mirror_session_key = *key;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return SDK_RET_OOM;
}

/// \brief get mirror session
/// \param[in] key, pointer to spec
/// \param[out] info, mirror session information
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t
pds_mirror_session_get (pds_mirror_session_key_t *key,
                        pds_mirror_session_info_t *info)
{
    return SDK_RET_OOM;
}

/// @}    // end of PDS_MIRROR_SESSION_API
