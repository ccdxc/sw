//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/service.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/agent/trace.hpp"

namespace core {

static inline sdk_ret_t
service_create_validate (pds_svc_mapping_spec_t *spec)
{
    pds_vpc_spec_t *vpc_spec;

    // verify VPC exists
    if ((vpc_spec = agent_state::state()->find_in_vpc_db(&spec->key.vpc)) ==
            NULL) {
        PDS_TRACE_ERR("Failed to create service vpc {} svc port {}, vpc {} "
                      "not found", spec->key.vpc.str(), spec->key.backend_port,
                      spec->key.vpc.str());
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

sdk_ret_t
service_create (pds_svc_mapping_key_t *key, pds_svc_mapping_spec_t *spec,
                pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if (agent_state::state()->find_in_service_db(key) != NULL) {
        PDS_TRACE_ERR("Failed to create service vpc {} svc port {}, "
                      "service already exists", spec->key.vpc.str(),
                      spec->key.backend_port);
        return SDK_RET_ENTRY_EXISTS;
    }
    if ((ret = service_create_validate(spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to create service vpc {} svc port {}, err {}",
                      spec->key.vpc.str(), spec->key.backend_port, ret);
        return ret;
    }

    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_svc_mapping_create(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to create service vpc {} svc port {}, err {}",
                          spec->key.vpc.str(), spec->key.backend_port, ret);
            return ret;
        }
    }
    if ((ret = agent_state::state()->add_to_service_db(key, spec)) !=
            SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to add service vpc {} svc port {} to db, err {}",
                      spec->key.vpc.str(), spec->key.backend_port, ret);
        return ret;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
service_update_validate (pds_svc_mapping_spec_t *spec)
{
    pds_vpc_spec_t *vpc_spec;

    // verify VPC exists
    if ((vpc_spec = agent_state::state()->find_in_vpc_db(&spec->key.vpc)) ==
            NULL) {
        PDS_TRACE_ERR("Failed to create service vpc {} svc port {}, "
                      "vpc {} not found", spec->key.vpc.str(),
                      spec->key.backend_port, spec->key.vpc.str());
        return SDK_RET_INVALID_ARG;
    }

    return SDK_RET_OK;
}

sdk_ret_t
service_update (pds_svc_mapping_key_t *key, pds_svc_mapping_spec_t *spec,
                pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if (agent_state::state()->find_in_service_db(key) == NULL) {
        PDS_TRACE_ERR("Failed to update service vpc {} svc port {}, "
                      "service doesn't exist", spec->key.vpc.str(),
                      spec->key.backend_port);
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    if ((ret = service_update_validate(spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to update service vpc {} svc port {}, err {}",
                      spec->key.vpc.str(), spec->key.backend_port, ret);
        return ret;
    }

    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_svc_mapping_update(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to update service vpc {} svc port {}, err {}",
                          spec->key.vpc.str(), spec->key.backend_port, ret);
            return ret;
        }
    }

    if (agent_state::state()->del_from_service_db(key) == false) {
        PDS_TRACE_ERR("Failed to delete service vpc {} svc port {} from db",
                      key->vpc.str(), key->backend_port);
    }

    if ((ret = agent_state::state()->add_to_service_db(key, spec)) !=
            SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to add service vpc {} svc port {} to db, err {}",
                      spec->key.vpc.str(), spec->key.backend_port, ret);
        return ret;
    }
    return SDK_RET_OK;
}

sdk_ret_t
service_delete (pds_svc_mapping_key_t *key, pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if (agent_state::state()->find_in_service_db(key) == NULL) {
        PDS_TRACE_ERR("Failed to find service vpc {} svc port {} in db",
                      key->vpc.str(), key->backend_port);
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_svc_mapping_delete(key, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to delete service vpc {} svc port {}, err {}",
                          key->vpc.str(), key->backend_port, ret);
            return ret;
        }
    }
    if (agent_state::state()->del_from_service_db(key) == false) {
        PDS_TRACE_ERR("Failed to delete service vpc {} svc port {} from db",
                      key->vpc.str(), key->backend_port);
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
service_get (pds_svc_mapping_key_t *key, pds_svc_mapping_info_t *info)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_svc_mapping_spec_t *spec;

    spec = agent_state::state()->find_in_service_db(key);
    if (spec == NULL) {
        PDS_TRACE_ERR("Failed to find service vpc {} svc port {} in db",
                      key->vpc.str(), key->backend_port);
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    memcpy(&info->spec, spec, sizeof(pds_svc_mapping_spec_t));
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_svc_mapping_read(key, info);
    } else {
        memset(&info->stats, 0, sizeof(info->stats));
        memset(&info->status, 0, sizeof(info->status));
    }
    return ret;
}

static inline sdk_ret_t
service_get_all_cb (pds_svc_mapping_spec_t *spec, void *ctxt)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_svc_mapping_info_t info;
    service_db_cb_ctxt_t *cb_ctxt = (service_db_cb_ctxt_t *)ctxt;

    memcpy(&info.spec, spec, sizeof(pds_svc_mapping_spec_t));
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_svc_mapping_read(&spec->key, &info);
    } else {
        memset(&info.stats, 0, sizeof(info.stats));
        memset(&info.status, 0, sizeof(info.status));
    }
    if (ret == SDK_RET_OK) {
        cb_ctxt->cb(&info, cb_ctxt->ctxt);
    }
    return ret;
}

sdk_ret_t
service_get_all (service_get_cb_t service_get_cb, void *ctxt)
{
    service_db_cb_ctxt_t cb_ctxt;

    cb_ctxt.cb = service_get_cb;
    cb_ctxt.ctxt = ctxt;
    return agent_state::state()->service_db_walk(service_get_all_cb, &cb_ctxt);
}

}    // namespace core
