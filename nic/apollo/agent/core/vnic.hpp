//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#ifndef __CORE_VNIC_HPP__
#define __CORE_VNIC_HPP__

#include "nic/apollo/api/include/pds_vnic.hpp"

namespace core {

typedef void (*vnic_get_cb_t)(const pds_vnic_info_t *spec, void *ctxt);

typedef struct vnic_db_cb_ctxt_s {
    vnic_get_cb_t cb;
    void         *ctxt;
} vnic_db_cb_ctxt_t;

sdk_ret_t vnic_create(pds_obj_key_t *key, pds_vnic_spec_t *spec,
                      pds_batch_ctxt_t bctxt);
sdk_ret_t vnic_update(pds_obj_key_t *key, pds_vnic_spec_t *spec,
                      pds_batch_ctxt_t bctxt);
sdk_ret_t vnic_delete(pds_obj_key_t *key, pds_batch_ctxt_t bctxt);
sdk_ret_t vnic_get(pds_obj_key_t *key, pds_vnic_info_t *info);
sdk_ret_t vnic_get_all(vnic_get_cb_t vnic_get_cb, void *ctxt);

}    // namespace core

#endif    // __CORE_VNIC_HPP__
