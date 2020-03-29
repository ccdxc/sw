//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include <vlib/vlib.h>
#include <vnet/vnet.h>

int vpc_impl_db_dump (vlib_main_t * vm)
{
    vlib_cli_output(vm, "corresponding DB doesn't exist");
    return 0;
}

int vnic_impl_db_dump (vlib_main_t * vm)
{
    vlib_cli_output(vm, "corresponding DB doesn't exist");
    return 0;
}

int subnet_impl_db_dump (vlib_main_t * vm)
{
    vlib_cli_output(vm, "corresponding DB doesn't exist");
    return 0;
}

int device_impl_db_dump (vlib_main_t * vm)
{
    vlib_cli_output(vm, "corresponding DB doesn't exist");
    return 0;
}
