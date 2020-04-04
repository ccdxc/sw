//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IMPL_APULU_CLI_H__
#define __VPP_IMPL_APULU_CLI_H__

#define PRINT_BUF_LINE(size)           \
{                                      \
    char buf[size+1];                  \
    clib_memset(buf, '-', size);       \
    buf[size] = '\0';                  \
    vlib_cli_output(vm,"%s", buf);     \
}                                      \

int vpc_impl_db_dump();

int vnic_impl_db_dump();

int subnet_impl_db_dump();

int device_impl_db_dump();

#endif    // __VPP_IMPL_APULU_CLI_H__
