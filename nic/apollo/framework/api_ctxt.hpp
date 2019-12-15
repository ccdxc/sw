//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file deals with internal API context information
///
//----------------------------------------------------------------------------

#ifndef __FRAMEWORK_API_CTXT_HPP__
#define __FRAMEWORK_API_CTXT_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/framework/api.hpp"

namespace api {

/// \brief API specific parameters
/// \brief Per API context maintained by framework while processing
typedef struct api_ctxt_s {
    api_op_t      api_op;        ///< api operation
    obj_id_t      obj_id;        ///< object identifier
    api_params_t  *api_params;   ///< API specific params
} api_ctxt_t;

// TODO: need to find better place for these (api.hpp ?)
slab *api_params_slab(void);
void api_params_free(api_params_t *api_params,
                     obj_id_t obj_id, api_op_t api_op);

// TODO: we can get rid of this and use api_msg_slab directly
slab *api_ctxt_slab(void);

static inline api_ctxt_t *
api_ctxt_alloc (obj_id_t obj_id, api_op_t api_op)
{
    api_ctxt_t *api_ctxt;

    api_ctxt = (api_ctxt_t *)api_ctxt_slab()->alloc();
    if (api_ctxt) {
        api_ctxt->obj_id = obj_id;
        api_ctxt->api_op = api_op;
        api_ctxt->api_params = (api_params_t *)api_params_slab()->alloc();
        if (unlikely(api_ctxt->api_params == NULL)) {
            api_ctxt_slab()->free(api_ctxt);
            return NULL;
        }
    }
    return api_ctxt;
}

static inline void
api_ctxt_free (api_ctxt_t *api_ctxt)
{
    if (api_ctxt->api_params) {
        api_params_free(api_ctxt->api_params,
                        api_ctxt->obj_id, api_ctxt->api_op);
    }
    api_ctxt_slab()->free(api_ctxt);
}

}    // namespace api

using api::api_ctxt_t;
using api::api_params_t;

#endif    // __FRAMEWORK_API_CTXT_HPP__
