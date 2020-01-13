//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/vpc.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_vpc.hpp"

namespace core {

static inline sdk_ret_t
vpc_create_validate (pds_vpc_spec_t *spec)
{
    switch (spec->type) {
    case PDS_VPC_TYPE_UNDERLAY:
        if (agent_state::state()->underlay_vpc() != PDS_VPC_ID_INVALID) {
            PDS_TRACE_ERR("Failed to create vpc {}, only one underlay vpc "
                          "allowed", spec->key.id);
            return SDK_RET_ENTRY_EXISTS;
        }
        break;
    case PDS_VPC_TYPE_TENANT:
        break;
    default:
        PDS_TRACE_ERR("Failed to create vpc {}, invalid type {}",
                      spec->key.id, spec->type);
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_create (pds_obj_key_t *key, pds_vpc_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if (agent_state::state()->find_in_vpc_db(key) != NULL) {
        PDS_TRACE_ERR("Failed to create vpc {}, vpc exists already",
                      spec->key.id);
        return SDK_RET_ENTRY_EXISTS;
    }
    if ((ret = vpc_create_validate(spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to create vpc {}, err {}", spec->key.id, ret);
        return ret;
    }
    if (agent_state::state()->device()->overlay_routing_en) {
        // call the metaswitch api
        if ((ret = pds_ms::vpc_create(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to create vpc {}, err {}",
                          spec->key.id, ret);
            return ret;
        }
    } else if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_vpc_create(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to create vpc {}, err {}", spec->key.id, ret);
            return ret;
        }
    }
    if ((ret = agent_state::state()->add_to_vpc_db(key, spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to add vpc {} to db, err {}", spec->key.id, ret);
        return ret;
    }
    if (spec->type == PDS_VPC_TYPE_UNDERLAY) {
        agent_state::state()->set_underlay_vpc(spec->key);
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
vpc_update_validate (pds_obj_key_t *key, pds_vpc_spec_t *spec)
{
    pds_vpc_spec_t *existing_spec;

    if ((existing_spec = agent_state::state()->find_in_vpc_db(key)) == NULL) {
        PDS_TRACE_ERR("Failed to update vpc {}, vpc doesn't exist",
                      spec->key.id);
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    if (existing_spec->type != spec->type) {
        PDS_TRACE_ERR("Failed to update vpc {}, cannot modify type",
                      spec->key.id);
        return SDK_RET_ERR;
    }

    return SDK_RET_OK;
}

sdk_ret_t
vpc_update (pds_obj_key_t *key, pds_vpc_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if ((ret = vpc_update_validate(key, spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to update vpc {}, err {}", spec->key.id, ret);
        return ret;
    }

    if (agent_state::state()->device()->overlay_routing_en) {
        // call the metaswitch api
        if ((ret = pds_ms::vpc_update(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to update vpc {}, err {}",
                          spec->key.id, ret);
            return ret;
        }
    } else if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_vpc_update(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to update vpc {}, err {}", spec->key.id, ret);
            return ret;
        }
    }

    if (agent_state::state()->del_from_vpc_db(key) == false) {
        PDS_TRACE_ERR("Failed to delete vpc {} from db", key->id);
    }

    if ((ret = agent_state::state()->add_to_vpc_db(key, spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to add vpc {} to db, err {}", spec->key.id, ret);
        return ret;
    }

    if (spec->type == PDS_VPC_TYPE_UNDERLAY) {
        agent_state::state()->reset_underlay_vpc();
        agent_state::state()->set_underlay_vpc(spec->key);
    }

    return SDK_RET_OK;
}

sdk_ret_t
vpc_delete (pds_obj_key_t *key, pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;
    pds_vpc_spec_t *spec;

    if ((spec = agent_state::state()->find_in_vpc_db(key)) == NULL) {
        PDS_TRACE_ERR("Failed to delete vpc {}, vpc not found", key->id);
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    if (agent_state::state()->device()->overlay_routing_en) {
        // call the metaswitch api
        if ((ret = pds_ms::vpc_delete(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to delete vpc {}, err {}",
                          key->id, ret);
            return ret;
        }
    } else if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_vpc_delete(key, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to delete vpc {}, err {}", key->id, ret);
            return ret;
        }
    }
    if (spec->type == PDS_VPC_TYPE_UNDERLAY) {
        agent_state::state()->reset_underlay_vpc();
    }
    if (agent_state::state()->del_from_vpc_db(key) == false) {
        PDS_TRACE_ERR("Failed to delete vpc {} from db", key->id);
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_get (pds_obj_key_t *key, pds_vpc_info_t *info)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_vpc_spec_t *spec;

    spec = agent_state::state()->find_in_vpc_db(key);
    if (spec == NULL) {
        PDS_TRACE_ERR("Failed to find vpc {} in db", key->id);
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    memcpy(&info->spec, spec, sizeof(pds_vpc_spec_t));
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_vpc_read(key, info);
    } else {
        memset(&info->stats, 0, sizeof(info->stats));
        memset(&info->status, 0, sizeof(info->status));
    }
    return ret;
}

static inline sdk_ret_t
vpc_get_all_cb (pds_vpc_spec_t *spec, void *ctxt)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_vpc_info_t info;

    vpc_db_cb_ctxt_t *cb_ctxt = (vpc_db_cb_ctxt_t *)ctxt;

    memcpy(&info.spec, spec, sizeof(pds_vpc_spec_t));
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_vpc_read(&spec->key, &info);
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
vpc_get_all (vpc_get_cb_t vpc_get_cb, void *ctxt)
{
    vpc_db_cb_ctxt_t cb_ctxt;

    cb_ctxt.cb = vpc_get_cb;
    cb_ctxt.ctxt = ctxt;

    return agent_state::state()->vpc_db_walk(vpc_get_all_cb, &cb_ctxt);
}

static inline sdk_ret_t
vpc_peer_create_validate (pds_vpc_peer_spec_t *spec)
{
    if (agent_state::state()->find_in_vpc_db(&spec->vpc1) == NULL) {
        PDS_TRACE_ERR("Failed to create vpc_peer {}, vpc doesn't exist {}",
                      spec->key.id, spec->vpc1.id);
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    if (agent_state::state()->find_in_vpc_db(&spec->vpc2) == NULL) {
        PDS_TRACE_ERR("Failed to create vpc_peer {}, vpc doesn't exist {}",
                      spec->key.id, spec->vpc2.id);
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_peer_create (pds_vpc_peer_key_t *key, pds_vpc_peer_spec_t *spec,
                 pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if (agent_state::state()->find_in_vpc_peer_db(key) != NULL) {
        PDS_TRACE_ERR("Failed to create vpc_peer {}, vpc_peer exists already",
                      spec->key.id);
        return SDK_RET_ENTRY_EXISTS;
    }
    if ((ret = vpc_peer_create_validate(spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to create vpc_peer {}, err {}",
                      spec->key.id, ret);
        return ret;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_vpc_peer_create(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to create vpc_peer {}, err {}",
                          spec->key.id, ret);
            return ret;
        }
    }
    if ((ret = agent_state::state()->add_to_vpc_peer_db(key, spec)) !=
            SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to add vpc_peer {} to db, err {}",
                      spec->key.id, ret);
        return ret;
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_peer_delete (pds_vpc_peer_key_t *key, pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;
    pds_vpc_peer_spec_t *spec;

    if ((spec = agent_state::state()->find_in_vpc_peer_db(key)) == NULL) {
        PDS_TRACE_ERR("Failed to delete vpc_peer {}, vpc_peer not found",
                      key->id);
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_vpc_peer_delete(key, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to delete vpc_peer {}, err {}", key->id, ret);
            return ret;
        }
    }
    if (agent_state::state()->del_from_vpc_peer_db(key) == false) {
        PDS_TRACE_ERR("Failed to delete vpc_peer {} from db", key->id);
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_peer_get (pds_vpc_peer_key_t *key, pds_vpc_peer_info_t *info)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_vpc_peer_spec_t *spec;

    spec = agent_state::state()->find_in_vpc_peer_db(key);
    if (spec == NULL) {
        PDS_TRACE_ERR("Failed to find vpc_peer {} in db", key->id);
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    memcpy(&info->spec, spec, sizeof(pds_vpc_peer_spec_t));
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_vpc_peer_read(key, info);
    } else {
        memset(&info->stats, 0, sizeof(info->stats));
        memset(&info->status, 0, sizeof(info->status));
    }
    return ret;
}

static inline sdk_ret_t
vpc_peer_get_all_cb (pds_vpc_peer_spec_t *spec, void *ctxt)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_vpc_peer_info_t info;

    vpc_peer_db_cb_ctxt_t *cb_ctxt = (vpc_peer_db_cb_ctxt_t *)ctxt;

    memcpy(&info.spec, spec, sizeof(pds_vpc_peer_spec_t));
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_vpc_peer_read(&spec->key, &info);
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
vpc_peer_get_all (vpc_peer_get_cb_t vpc_peer_get_cb, void *ctxt)
{
    vpc_peer_db_cb_ctxt_t cb_ctxt;

    cb_ctxt.cb = vpc_peer_get_cb;
    cb_ctxt.ctxt = ctxt;

    return agent_state::state()->vpc_peer_db_walk(vpc_peer_get_all_cb,
                                                  &cb_ctxt);
}

}    // namespace core
