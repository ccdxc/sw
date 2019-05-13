//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#ifndef __CORE_SUBNET_HPP__
#define __CORE_SUBNET_HPP__

#include "nic/apollo/api/include/pds_subnet.hpp"

namespace core {

typedef void (*subnet_get_cb_t)(const pds_subnet_info_t *spec, void *ctxt);

typedef struct subnet_db_cb_ctxt_s {
    subnet_get_cb_t cb;
    void *ctxt;
} subnet_db_cb_ctxt_t;

sdk_ret_t subnet_create(pds_subnet_key_t *key, pds_subnet_spec_t *spec);
sdk_ret_t subnet_update(pds_subnet_key_t *key, pds_subnet_spec_t *spec);
sdk_ret_t subnet_delete(pds_subnet_key_t *key);
sdk_ret_t subnet_get(pds_subnet_key_t *key, pds_subnet_info_t *info);
sdk_ret_t subnet_get_all(subnet_get_cb_t subnet_get_cb, void *ctxt);

}    // namespace core

#endif    // __CORE_SUBNET_HPP__
