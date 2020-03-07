//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#ifndef __CORE_MIRROR_HPP__
#define __CORE_MIRROR_HPP__

#include "nic/apollo/api/include/pds_mirror.hpp"

namespace core {

typedef void (*mirror_session_get_cb_t)(const pds_mirror_session_info_t *spec,
                                        void *ctxt);

typedef struct mirror_session_db_cb_ctxt_s {
    mirror_session_get_cb_t cb;
    void *ctxt;
} mirror_session_db_cb_ctxt_t;

sdk_ret_t mirror_session_create(pds_obj_key_t *key,
                                pds_mirror_session_spec_t *spec,
                                pds_batch_ctxt_t bctxt);
sdk_ret_t mirror_session_update(pds_obj_key_t *key,
                                pds_mirror_session_spec_t *spec,
                                pds_batch_ctxt_t bctxt);
sdk_ret_t mirror_session_delete(pds_obj_key_t *key, pds_batch_ctxt_t bctxt);
sdk_ret_t mirror_session_get(pds_obj_key_t *key,
                             pds_mirror_session_info_t *info);
sdk_ret_t mirror_session_get_all(mirror_session_read_cb_t mirror_session_read_cb,
                                 void *ctxt);

}    // namespace core

#endif    // __CORE_MIRROR_HPP__
