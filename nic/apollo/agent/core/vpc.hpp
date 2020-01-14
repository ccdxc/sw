//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#ifndef __CORE_VPC_HPP__
#define __CORE_VPC_HPP__

#include "nic/apollo/api/include/pds_vpc.hpp"

namespace core {

typedef void (*vpc_get_cb_t)(const pds_vpc_info_t *spec, void *ctxt);

typedef struct vpc_db_cb_ctxt_s {
    vpc_get_cb_t cb;
    void         *ctxt;
} vpc_db_cb_ctxt_t;

sdk_ret_t vpc_create(pds_obj_key_t *key, pds_vpc_spec_t *spec,
                     pds_batch_ctxt_t bctxt);
sdk_ret_t vpc_update(pds_obj_key_t *key, pds_vpc_spec_t *spec,
                     pds_batch_ctxt_t bctxt);
sdk_ret_t vpc_delete(pds_obj_key_t *key, pds_batch_ctxt_t bctxt);
sdk_ret_t vpc_get(pds_obj_key_t *key, pds_vpc_info_t *info);
sdk_ret_t vpc_get_all(vpc_get_cb_t vpc_get_cb, void *ctxt);

typedef void (*vpc_peer_get_cb_t)(const pds_vpc_peer_info_t *spec, void *ctxt);

typedef struct vpc_peer_db_cb_ctxt_s {
    vpc_peer_get_cb_t cb;
    void         *ctxt;
} vpc_peer_db_cb_ctxt_t;

sdk_ret_t vpc_peer_create(pds_obj_key_t *key, pds_vpc_peer_spec_t *spec,
                          pds_batch_ctxt_t bctxt);
sdk_ret_t vpc_peer_delete(pds_obj_key_t *key, pds_batch_ctxt_t bctxt);
sdk_ret_t vpc_peer_get(pds_obj_key_t *key, pds_vpc_peer_info_t *info);
sdk_ret_t vpc_peer_get_all(vpc_peer_get_cb_t vpc_peer_get_cb, void *ctxt);

}    // namespace core

#endif    // __CORE_VPC_HPP__
