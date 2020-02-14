//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#ifndef __CORE_TAG_HPP__
#define __CORE_TAG_HPP__

#include "nic/apollo/api/include/pds_tag.hpp"

namespace core {

sdk_ret_t tag_create(pds_obj_key_t *key, pds_tag_spec_t *spec,
                     pds_batch_ctxt_t bctxt);
sdk_ret_t tag_update(pds_obj_key_t *key, pds_tag_spec_t *spec,
                     pds_batch_ctxt_t bctxt);
sdk_ret_t tag_delete(pds_obj_key_t *key, pds_batch_ctxt_t bctxt);
sdk_ret_t tag_get(pds_obj_key_t *key, pds_tag_info_t *info);
sdk_ret_t tag_get_all(tag_read_cb_t tag_read_cb, void *ctxt);

}    // namespace core

#endif    // __CORE_TAG_HPP__
