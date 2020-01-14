//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#ifndef __CORE_TUNNEL_HPP__
#define __CORE_TUNNEL_HPP__

#include "nic/apollo/api/include/pds_route.hpp"

namespace core {

typedef void (*route_table_get_cb_t)(const pds_route_table_info_t *spec,
                                     void *ctxt);

typedef struct route_table_db_cb_ctxt_s {
    route_table_get_cb_t cb;
    void         *ctxt;
} route_table_db_cb_ctxt_t;

sdk_ret_t route_table_create(pds_obj_key_t *key,
                             pds_route_table_spec_t *spec,
                             pds_batch_ctxt_t bctxt);
sdk_ret_t route_table_update(pds_obj_key_t *key,
                             pds_route_table_spec_t *spec,
                             pds_batch_ctxt_t bctxt);
sdk_ret_t route_table_delete(pds_obj_key_t *key, pds_batch_ctxt_t bctxt);
sdk_ret_t route_table_get(pds_obj_key_t *key, pds_route_table_info_t *info);
sdk_ret_t route_table_get_all(route_table_get_cb_t route_table_get_cb,
                              void *ctxt);

}    // namespace core

#endif    // __CORE_TUNNEL_HPP__
