//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/tunnel.hpp"
#include "nic/apollo/agent/core/meter.hpp"

namespace core {

static inline sdk_ret_t
meter_create_validate (pds_meter_spec_t *spec)
{
    for (uint32_t rule = 0; rule < spec->num_rules; rule++) {
        pds_meter_rule_t *rule_spec = &spec->rules[rule];
        if ((rule_spec->type == PDS_METER_TYPE_PPS_POLICER) ||
            (rule_spec->type == PDS_METER_TYPE_BPS_POLICER)) {
            PDS_TRACE_ERR("Failed to create meter {}, PPS and BPS policers "
                          "not supported", spec->key.str());
            return SDK_RET_INVALID_ARG;
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
meter_create (pds_obj_key_t *key, pds_meter_spec_t *spec,
              pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if (agent_state::state()->find_in_meter_db(key) != NULL) {
        PDS_TRACE_ERR("Failed to create meter {}, meter exists already",
                      spec->key.str());
        return sdk::SDK_RET_ENTRY_EXISTS;
    }

    if ((ret = meter_create_validate(spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Meter {} validation failure, err {}",
                      spec->key.str(), ret);
        return ret;
    }

    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_meter_create(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to create meter {}, err {}",
                          spec->key.str(), ret);
            return ret;
        }
    }

    if ((ret = agent_state::state()->add_to_meter_db(key, spec)) !=
            SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to add meter {} to db, err {}",
                      spec->key.str(), ret);
        return ret;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
meter_update_validate (pds_meter_spec_t *spec)
{
    for (uint32_t rule = 0; rule < spec->num_rules; rule++) {
        pds_meter_rule_t *rule_spec = &spec->rules[rule];
        if ((rule_spec->type == PDS_METER_TYPE_PPS_POLICER) ||
            (rule_spec->type == PDS_METER_TYPE_BPS_POLICER)) {
            PDS_TRACE_ERR("Failed to update meter {}, PPS and BPS policers "
                          "not supported", spec->key.str());
            return SDK_RET_INVALID_ARG;
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
meter_update (pds_obj_key_t *key, pds_meter_spec_t *spec,
              pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if (agent_state::state()->find_in_meter_db(key) == NULL) {
        PDS_TRACE_ERR("Failed to update meter {}, meter doesn't exist",
                      spec->key.str());
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    if ((ret = meter_update_validate(spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Meter {} validation failure, err {}",
                      spec->key.str(), ret);
        return ret;
    }

    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_meter_update(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to create meter {}, err {}",
                          spec->key.str(), ret);
            return ret;
        }
    }

    if (agent_state::state()->del_from_meter_db(key) == false) {
        PDS_TRACE_ERR("Failed to delete meter {} from meter db", key->str());
    }

    if ((ret = agent_state::state()->add_to_meter_db(key, spec)) !=
            SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to add meter {} to db, err {}",
                      spec->key.str(), ret);
        return ret;
    }
    return SDK_RET_OK;
}

sdk_ret_t
meter_delete (pds_obj_key_t *key, pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if (agent_state::state()->find_in_meter_db(key) == NULL) {
        PDS_TRACE_ERR("Failed to delete meter {}, meter not found", key->str());
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_meter_delete(key, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to delete meter {}, err {}", key->str(), ret);
            return ret;
        }
    }

    if (agent_state::state()->del_from_meter_db(key) == false) {
        PDS_TRACE_ERR("Failed to delete meter {} from meter db", key->str());
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
meter_get (pds_obj_key_t *key, pds_meter_info_t *info)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_meter_spec_t *spec;

    spec = agent_state::state()->find_in_meter_db(key);
    if (spec == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    //info->spec = *spec;
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_meter_read(key, info);
    } else {
        memset(&info->stats, 0, sizeof(info->stats));
        memset(&info->status, 0, sizeof(info->status));
    }
    return ret;
}

static inline sdk_ret_t
meter_get_all_cb (pds_meter_spec_t *spec, void *ctxt)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_meter_info_t info;
    meter_db_cb_ctxt_t *cb_ctxt = (meter_db_cb_ctxt_t *)ctxt;

    memset(&info, 0, sizeof(pds_meter_info_t));
    //info.spec = *spec;
    if (!agent_state::state()->pds_mock_mode()) {
        if (spec != NULL) {
            ret = pds_meter_read(&spec->key, &info);
        }
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
meter_get_all (meter_get_cb_t meter_get_cb, void *ctxt)
{
    meter_db_cb_ctxt_t cb_ctxt;

    cb_ctxt.cb = meter_get_cb;
    cb_ctxt.ctxt = ctxt;

     return agent_state::state()->meter_db_walk(meter_get_all_cb, &cb_ctxt);
}

}    // namespace core
