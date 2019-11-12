//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#ifndef __CORE_NH_HPP__
#define __CORE_NH_HPP__

#include "nic/apollo/api/include/pds_nexthop.hpp"

namespace core {

typedef void (*nh_get_cb_t)(const pds_nexthop_info_t *spec, void *ctxt);

typedef struct nh_db_cb_ctxt_s {
    nh_get_cb_t cb;
    pds_nh_type_t type;
    void       *ctxt;
} nh_db_cb_ctxt_t;

sdk_ret_t nh_create(pds_nexthop_key_t *key, pds_nexthop_spec_t *spec,
                    pds_batch_ctxt_t bctxt);
sdk_ret_t nh_update(pds_nexthop_key_t *key, pds_nexthop_spec_t *spec,
                    pds_batch_ctxt_t bctxt);
sdk_ret_t nh_delete(pds_nexthop_key_t *key, pds_batch_ctxt_t bctxt);
sdk_ret_t nh_get(pds_nexthop_key_t *key, pds_nexthop_info_t *info);
sdk_ret_t nh_get_all(nh_get_cb_t nexthop_get_cb, void *ctxt, pds_nh_type_t type);

}    // namespace core

#endif    // __CORE_NH_HPP__
