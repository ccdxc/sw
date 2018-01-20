//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// timerfd library is a wrapper around linux timerfd APIs
//------------------------------------------------------------------------------

#ifndef __SDK_TIMERFD_HPP__
#define __SDK_TIMERFD_HPP__

#include <sdk/base.hpp>

namespace sdk {
namespace lib {

typedef struct timerfd_info_s {
    int         timer_fd;
    uint64_t    usecs;
    uint64_t    missed_wakeups;
} timerfd_info_t;

void timerfd_init(timerfd_info_t *timerfd_info);
int timerfd_prepare(timerfd_info_t *timerfd_info);
int timerfd_wait(timerfd_info_t *timerfd_info, uint64_t *missed);

}    // namespace lib
}    // namespace sdk

using sdk::lib::timerfd_info_t;

#endif    // __SDK_TIMERFD_HPP__

