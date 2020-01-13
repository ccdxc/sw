//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#ifndef __CORE_NH_GROUP_HPP__
#define __CORE_NH_GROUP_HPP__

#include "nic/apollo/api/include/pds_nexthop.hpp"

namespace core {

typedef void (*nh_group_get_cb_t)(const pds_nexthop_group_info_t *spec, void *ctxt);

typedef struct nh_group_db_cb_ctxt_s {
    nh_group_get_cb_t cb;
    void       *ctxt;
} nh_group_db_cb_ctxt_t;

sdk_ret_t nh_group_create(pds_obj_key_t *key, pds_nexthop_group_spec_t *spec,
                          pds_batch_ctxt_t bctxt);
sdk_ret_t nh_group_update(pds_obj_key_t *key, pds_nexthop_group_spec_t *spec,
                          pds_batch_ctxt_t bctxt);
sdk_ret_t nh_group_delete(pds_obj_key_t *key, pds_batch_ctxt_t bctxt);
sdk_ret_t nh_group_get(pds_obj_key_t *key, pds_nexthop_group_info_t *info);
sdk_ret_t nh_group_get_all(nh_group_get_cb_t nexthop_group_get_cb, void *ctxt);

}    // namespace core

#endif    // __CORE_NH_GROUP_HPP__
