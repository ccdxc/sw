//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//------------------------------------------------------------------------------

#ifndef __PEN_OPER_CORE_STATE_HPP__
#define __PEN_OPER_CORE_STATE_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/eventmgr/eventmgr.hpp"

using sdk::lib::eventmgr;

typedef enum pen_oper_info_types_s {
    PENOPER_INFO_TYPE_NONE,
    PENOPER_INFO_TYPE_ALERTS,
    PENOPER_INFO_TYPE_MAX,
} pen_oper_info_types_t;

namespace core {

class pen_oper_state {
public:
    static sdk_ret_t init(void);
    static class pen_oper_state *state(void);

    pen_oper_state() { evmgr_ = eventmgr::factory(PENOPER_INFO_TYPE_MAX); };
    ~pen_oper_state() {};
    eventmgr *event_mgr(void) { return evmgr_; }

private:
    eventmgr *evmgr_;  // event manager instance
};

} // namespace core

#endif // __PEN_OPER_CORE_STATE_HPP__
