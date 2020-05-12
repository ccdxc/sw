//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//------------------------------------------------------------------------------

#include "nic/sdk/include/sdk/mem.hpp"
#include "state.hpp"

namespace core {

class pen_oper_state *g_state;

class pen_oper_state *
pen_oper_state::state(void) {
    return g_state;
}

sdk_ret_t
pen_oper_state::init(void) {
    void *mem;

    mem = SDK_CALLOC(SDK_MEM_ALLOC_INFRA, sizeof(pen_oper_state));
    if (mem) {
        g_state = new(mem) pen_oper_state();
    }
    SDK_ASSERT_RETURN((g_state != NULL), SDK_RET_ERR);
    return SDK_RET_OK;
}

}    // namespace core
