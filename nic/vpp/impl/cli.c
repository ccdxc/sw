//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include <impl_cli.h>

static clib_error_t *
vpp_pds_impl_db_dump_fn (vlib_main_t * vm,
                         unformat_input_t * input,
                         vlib_cli_command_t * cmd)
{

    int ret = 0;
    u8 vpc = 0, vnic = 0, subnet = 0, device = 0, status = 0;

    while (unformat_check_input (input) != UNFORMAT_END_OF_INPUT) {
        if (unformat (input, "vpc")) {
            vpc = 1;
        } else if (unformat (input, "vnic")) {
            vnic = 1;
        } else if (unformat(input, "subnet")) {
            subnet = 1;
        } else if (unformat (input, "device")) {
            device = 1;
        } else if (unformat(input, "status")) {
            status = 1;
        } else {
            vlib_cli_output(vm, "ERROR: Invalid command.\n");
            goto done;
        }
    }

    if (!vpc && !vnic && !subnet && !device) {
        vlib_cli_output(vm, "ERROR: Invalid input.\n");
        goto done;
    }

    if (status) {
        if (vpc) {
            ret = vpc_impl_db_dump(vm);
        } else if (vnic) {
            ret = vnic_impl_db_dump(vm);
        } else if (subnet) {
            ret = subnet_impl_db_dump(vm);
        } else if (device) {
            ret = device_impl_db_dump(vm);
        }
    } else {
        //add code for config display
        vlib_cli_output(vm, "Config DBs TBD.\n");
    }
    if (ret != 0) {
        vlib_cli_output(vm, "Error reading entries.\n");
    }

done:
    return 0;
}

VLIB_CLI_COMMAND (dump_vpp_pds_impl_db, static) =
{
    .path = "show pds",
    .short_help = "show pds [vpc | subnet | vnic | device] [status]",
    .function = vpp_pds_impl_db_dump_fn,
    .is_mp_safe = 1,
};
