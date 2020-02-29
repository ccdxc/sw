//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#ifndef __CORE_NH_HPP__
#define __CORE_NH_HPP__

#include "nic/apollo/api/include/pds_nexthop.hpp"

namespace core {

sdk_ret_t nh_create(pds_obj_key_t *key, pds_nexthop_spec_t *spec,
                    pds_batch_ctxt_t bctxt);
sdk_ret_t nh_update(pds_obj_key_t *key, pds_nexthop_spec_t *spec,
                    pds_batch_ctxt_t bctxt);
sdk_ret_t nh_delete(pds_obj_key_t *key, pds_batch_ctxt_t bctxt);
sdk_ret_t nh_get(pds_obj_key_t *key, pds_nexthop_info_t *info);
sdk_ret_t nh_get_all(nexthop_read_cb_t nexthop_read_cb, void *ctxt);

}    // namespace core

#endif    // __CORE_NH_HPP__
