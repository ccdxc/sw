//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/subnet.hpp"

namespace core {

sdk_ret_t
subnet_create (pds_subnet_key_t *key, pds_subnet_spec_t *spec)
{
    if (agent_state::state()->find_in_subnet_db(key) != NULL) {
        return sdk::SDK_RET_ENTRY_EXISTS;
    }
    if (pds_subnet_create(spec) != sdk::SDK_RET_OK) {
        return sdk::SDK_RET_ERR;
    }
    if (agent_state::state()->add_to_subnet_db(key, spec) != sdk::SDK_RET_OK) {
        return sdk::SDK_RET_ERR;
    }
    return sdk::SDK_RET_OK;
}

sdk_ret_t
subnet_delete (pds_subnet_key_t *key)
{
    if (agent_state::state()->find_in_subnet_db(key) == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    if (pds_subnet_delete(key) != sdk::SDK_RET_OK) {
        return sdk::SDK_RET_ERR;
    }
    if (agent_state::state()->del_from_subnet_db(key) == false) {
        return sdk::SDK_RET_ERR;
    }
    return sdk::SDK_RET_OK;
}

}    // namespace core
