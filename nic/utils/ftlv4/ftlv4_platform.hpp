//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTLV4_P4PD_HPP__
#define __FTLV4_P4PD_HPP__

#include "include/sdk/base.hpp"
#include "lib/p4/p4_api.hpp"

#include "ftlv4_apictx.hpp"
using sdk::table::ftlint_ipv4::ftlv4_apictx;

void
ftlv4_swap_bytes(uint8_t *entry);

sdk_ret_t
ftlv4_platform_read(ftlv4_apictx *ctx);

sdk_ret_t
ftlv4_platform_write(ftlv4_apictx *ctx);

#endif
