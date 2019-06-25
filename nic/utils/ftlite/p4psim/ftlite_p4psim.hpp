//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTLITE__P4PSIM_HPP__
#define __FTLITE__P4PSIM_HPP__

#include "nic/utils/ftlite/ftlite.hpp"

namespace ftlite {
namespace p4psim {

using namespace ftlite;

sdk_ret_t init(ftlite::init_params_t *ips);
sdk_ret_t insert(ftlite::insert_params_t *ips);

} // namespace ftlite
} // namespace p4psim

#endif
