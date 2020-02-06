//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#ifndef __CORE_METER_HPP__
#define __CORE_METER_HPP__

#include "nic/apollo/api/include/pds_meter.hpp"

namespace core {

sdk_ret_t meter_create(pds_obj_key_t *key, pds_meter_spec_t *spec,
                       pds_batch_ctxt_t bctxt);
sdk_ret_t meter_update(pds_obj_key_t *key, pds_meter_spec_t *spec,
                       pds_batch_ctxt_t bctxt);
sdk_ret_t meter_delete(pds_obj_key_t *key, pds_batch_ctxt_t bctxt);
sdk_ret_t meter_get(pds_obj_key_t *key, pds_meter_info_t *info);
sdk_ret_t meter_get_all(meter_read_cb_t meter_read_cb, void *ctxt);

}    // namespace core

#endif    // __CORE_METER_HPP__
