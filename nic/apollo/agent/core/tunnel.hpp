//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#ifndef __CORE_TUNNEL_HPP__
#define __CORE_TUNNEL_HPP__

#include "nic/apollo/api/include/pds_tep.hpp"

namespace core {

typedef void (*tep_get_cb_t)(const pds_tep_info_t *spec, void *ctxt);

typedef struct tep_db_cb_ctxt_s {
    tep_get_cb_t cb;
    void *ctxt;
} tep_db_cb_ctxt_t;

sdk_ret_t tep_create(pds_obj_key_t *key, pds_tep_spec_t *spec,
                     pds_batch_ctxt_t bctxt);
sdk_ret_t tep_update(pds_obj_key_t *key, pds_tep_spec_t *spec,
                     pds_batch_ctxt_t bctxt);
sdk_ret_t tep_delete(pds_obj_key_t *key, pds_batch_ctxt_t bctxt);
sdk_ret_t tep_get(pds_obj_key_t *key, pds_tep_info_t *info);
sdk_ret_t tep_get_all(tep_get_cb_t tep_get_cb, void *ctxt);

}    // namespace core

#endif    // __CORE_TUNNEL_HPP__
