//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#ifndef __CORE_SERVICE_HPP__
#define __CORE_SERVICE_HPP__

#include "nic/apollo/api/include/pds_service.hpp"

namespace core {

typedef void (*service_get_cb_t)(const pds_svc_mapping_info_t *spec,
                                 void *ctxt);

typedef struct service_db_cb_ctxt_s {
    service_get_cb_t cb;
    void *ctxt;
} service_db_cb_ctxt_t;

sdk_ret_t service_create(pds_svc_mapping_key_t *key,
                         pds_svc_mapping_spec_t *spec,
                         pds_batch_ctxt_t bctxt);
sdk_ret_t service_update(pds_svc_mapping_key_t *key,
                         pds_svc_mapping_spec_t *spec,
                         pds_batch_ctxt_t bctxt);
sdk_ret_t service_delete(pds_obj_key_t *key, pds_batch_ctxt_t bctxt);
sdk_ret_t service_get(pds_obj_key_t *key, pds_svc_mapping_info_t *info);
sdk_ret_t service_get_all(service_get_cb_t service_get_cb, void *ctxt);

}    // namespace core

#endif    // __CORE_SERVICE_HPP__
