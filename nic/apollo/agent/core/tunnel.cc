//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/tunnel.hpp"

namespace core {

sdk_ret_t
tep_create (uint32_t key, pds_tep_spec_t *spec)
{
    if (agent_state::state()->find_in_tep_db(key) != NULL) {
        return sdk::SDK_RET_ENTRY_EXISTS;
    }
    if (pds_tep_create(spec) != sdk::SDK_RET_OK) {
        return sdk::SDK_RET_ERR;
    }
    if (agent_state::state()->add_to_tep_db(key, spec) != sdk::SDK_RET_OK) {
        return sdk::SDK_RET_ERR;
    }
    return sdk::SDK_RET_OK;
}

sdk_ret_t
tep_delete (uint32_t key)
{
    pds_tep_spec_t *spec;

    spec = agent_state::state()->find_in_tep_db(key);
    if (spec == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    if (pds_tep_delete(&spec->key) != sdk::SDK_RET_OK) {
        return sdk::SDK_RET_ERR;
    }
    if (agent_state::state()->del_from_tep_db(key) == false) {
        return sdk::SDK_RET_ERR;
    }
    return sdk::SDK_RET_OK;
}

}    // namespace core
