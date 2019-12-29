/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

#include <memory>
#include <string>
#include <iostream>
#include "cmd_hndlr.h"
#include "lib/logger/logger.hpp"

namespace sdk {
namespace platform {
namespace ncsi {

typedef enum {
    INTF_POWER_DN,
    INTF_POWER_UP,
    PKG_RDY_DESEL__CHAN_INIT,
    PKG_RDY_SEL__CHAN_INIT,
    PKG_RDY_SEL__CHAN_RDY,
    PKG_RDY_DESEL__CHAN_RDY,
} NcsiState;

typedef enum {
    SUCCESS,
    FAIL,
    INIT_REQRD,
    TIMEOUT,
    INVALID
} NcsiStateErr;

class StateMachine {
private:
    void SetState(NcsiState state) { state_ = state; };
    void LoadPowerOnDefaults();
    NcsiState state_;
public:
    StateMachine();
    NcsiState GetCurState() { return state_; };
    NcsiStateErr UpdateState(NcsiCmd cmd);
};

} // namespace ncsi
} // namespace platform
} // namespace sdk

#endif // __STATE_MACHINE_H__

