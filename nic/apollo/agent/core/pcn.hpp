//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#ifndef __CORE_PCN_HPP__
#define __CORE_PCN_HPP__

#include "nic/apollo/api/include/pds_vcn.hpp"

namespace core {

typedef void (*pcn_get_cb_t)(const pds_vcn_info_t *spec, void *ctxt);

typedef struct pcn_db_cb_ctxt_s {
    pcn_get_cb_t cb;
    void         *ctxt;
} pcn_db_cb_ctxt_t;

sdk_ret_t pcn_create(pds_vcn_key_t *key, pds_vcn_spec_t *spec);
sdk_ret_t pcn_delete(pds_vcn_key_t *key);
sdk_ret_t pcn_get(pds_vcn_key_t *key, pds_vcn_info_t *info);
sdk_ret_t pcn_get_all(pcn_get_cb_t pcn_get_cb, void *ctxt);

}    // namespace core

#endif    // __CORE_PCN_HPP__
