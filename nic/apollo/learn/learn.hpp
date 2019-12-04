//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains external interfaces of Learn module
///
//----------------------------------------------------------------------------

#ifndef __LEARN_LEARN_HPP__
#define __LEARN_LEARN_HPP__

namespace learn {

bool learn_thread_enabled(void);

void *learn_thread_start(void *ctxt);

}    // namespace learn

#endif    // __LEARN_LEARN_HPP__

