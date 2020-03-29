//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IMPL_APULU_IMPL_CLI_H__
#define __VPP_IMPL_APULU_IMPL_CLI_H__

#include <stddef.h>
#include <vlib/vlib.h>
#include <vnet/vnet.h>
#include <vnet/plugin/plugin.h>
#include "impl_db.h"

#define PRINT_BUF_LINE(size)           \
{                                      \
    char buf[size+1];                  \
    clib_memset(buf, '-', size);       \
    buf[size] = '\0';                  \
    vlib_cli_output(vm,"%s", buf);     \
}                                      \

int vpc_impl_db_dump(vlib_main_t * vm);

int vnic_impl_db_dump(vlib_main_t * vm);

int subnet_impl_db_dump(vlib_main_t * vm);

int device_impl_db_dump(vlib_main_t * vm);

#endif    // __VPP_IMPL_APULU_IMPL_CLI_H__
