//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTL_P4PD_HPP__
#define __FTL_P4PD_HPP__

#include "include/sdk/base.hpp"
#include "lib/p4/p4_api.hpp"

#include "ftl_apictx.hpp"
using sdk::table::ftlint::ftl_apictx;

void
ftl_swap_bytes(uint8_t *entry);

sdk_ret_t
ftl_platform_read(ftl_apictx *ctx);

sdk_ret_t
ftl_platform_write(ftl_apictx *ctx);

#endif
