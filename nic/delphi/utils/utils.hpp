// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_UTILS_UTILS_H_
#define _DELPHI_UTILS_UTILS_H_

#include <stdio.h>

#include "log.hpp"
#include "error.hpp"

namespace delphi {

using namespace std;

#define RETURN_IF_FAILED(fn) \
    do {                     \
        delphi::error _err = (fn);    \
        if (_err.IsNotOK()) {   \
            return _err;     \
        }                    \
    } while (0)

} // namespace delphi

#endif // _DELPHI_UTILS_UTILS_H_
