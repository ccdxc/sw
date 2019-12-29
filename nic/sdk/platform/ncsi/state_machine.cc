/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#include <cstdio>
#include "state_machine.h"

namespace sdk {
namespace platform {
namespace ncsi {

StateMachine::StateMachine()
{
    LoadPowerOnDefaults();
    state_ = PKG_RDY_DESEL__CHAN_INIT;
}

void StateMachine::LoadPowerOnDefaults()
{
}

NcsiStateErr StateMachine::UpdateState(NcsiCmd cmd)
{
    NcsiState cur_state = GetCurState();
    NcsiState next_state = cur_state;
    NcsiStateErr ret = SUCCESS;

    switch (cur_state) {
        case PKG_RDY_DESEL__CHAN_INIT:
            switch (cmd) {
                case CMD_DESELECT_PACKAGE:
                    break;
                case CMD_CLEAR_INIT_STATE:
                    next_state = PKG_RDY_SEL__CHAN_RDY;
                    break;
                default:
                    next_state = PKG_RDY_SEL__CHAN_INIT;
                    break;
            }
            break;
        case PKG_RDY_SEL__CHAN_INIT:
            switch (cmd) {
                case CMD_CLEAR_INIT_STATE:
                    next_state = PKG_RDY_SEL__CHAN_RDY;
                    break;
                case CMD_DESELECT_PACKAGE:
                    next_state = PKG_RDY_DESEL__CHAN_INIT;
                    break;
                case CMD_SELECT_PACKAGE:
                    break;
                default:
                    SDK_TRACE_INFO("Invalid command in current state\n");
                    ret = INIT_REQRD;
                    break;
            }
            break;
        case PKG_RDY_SEL__CHAN_RDY:
            switch (cmd) {
                case CMD_RESET_CHAN:
                    next_state = PKG_RDY_SEL__CHAN_INIT;
                    break;
                case CMD_DESELECT_PACKAGE:
                    next_state = PKG_RDY_DESEL__CHAN_RDY;
                    break;
                default:
                    //SDK_TRACE_INFO("Invalid command: 0x%x in current state: 0x%x\n", cmd, cur_state);
                    //ret = INVALID;
                    break;
            }
            break;
        case PKG_RDY_DESEL__CHAN_RDY:
            switch (cmd) {
                case CMD_RESET_CHAN:
                    next_state = PKG_RDY_SEL__CHAN_INIT;
                    break;
                case CMD_DESELECT_PACKAGE:
                    break;
                case CMD_CLEAR_INIT_STATE:
                    ret = INVALID;
                    break;
                default:
                    next_state = PKG_RDY_SEL__CHAN_RDY;
            }
            break;
        default:
            SDK_TRACE_INFO("Invalid current state: 0x%x\n", cur_state);
            break;
    }

    SetState(next_state);

    SDK_TRACE_INFO("Update StateMachine: cmd:0x%x initia_state: 0x%x, next_state = 0x%x", cmd, cur_state, next_state);

    return ret;
}

} // namespace ncsi
} // namespace platform
} // namespace sdk

