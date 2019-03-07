//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __PRINT_HPP__
#define __PRINT_HPP__

#include "devapi_types.hpp"

namespace iris {

char *macaddr2str (mac_t mac_addr);
void api_trace (const char *trace);

}     // namespace iris

#endif
