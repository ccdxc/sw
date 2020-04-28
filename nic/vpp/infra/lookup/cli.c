//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include <stddef.h>
#include <vlib/vlib.h>
#include <vnet/vnet.h>
#include <vnet/plugin/plugin.h>
#include "node.h"

static clib_error_t *
set_packet_dump_command_fn (vlib_main_t *vm,
                            unformat_input_t *input,
                            vlib_cli_command_t *cmd)
{
    char *file = NULL;
    u32 pak_size = DEAFULT_PACKET_DUMP_SIZE;
    bool enable = true;

    while (unformat_check_input (input) != UNFORMAT_END_OF_INPUT) {
        if (unformat(input, "file %s", &file)) {
            ;
        } else if (unformat(input, "size %d", &pak_size)) {
            ;
        } else if (unformat(input, "disable")) {
            enable = false;
        } else {
            vlib_cli_output(vm, "ERROR: Invalid command");
            goto done;
        }
    }

    if (enable && !file) {
        vlib_cli_output(vm, "ERROR: Invalid command : file not specified");
        goto done;
    }
    pds_packet_dump_en_dis(enable, file, (u16)pak_size);

done:
    return 0;
}

VLIB_CLI_COMMAND(set_packet_dump_command, static) =
{
    .path = "set pds packet dump",
    .short_help = "set pds packet dump [file <file-path>] "
                  "[size <packet bytes>] [disable]",
    .function = set_packet_dump_command_fn,
};

static clib_error_t *
show_packet_dump_command_fn (vlib_main_t *vm,
                             unformat_input_t *input,
                             vlib_cli_command_t *cmd)
{
    while (unformat_check_input (input) != UNFORMAT_END_OF_INPUT) {
        vlib_cli_output(vm, "ERROR: Invalid command");
        goto done;
    }

    pds_packet_dump_show(vm);

done:
    return 0;
}

VLIB_CLI_COMMAND(show_packet_dump_command, static) =
{
    .path = "show pds packet-dump",
    .short_help = "show pds packet-dump",
    .function = show_packet_dump_command_fn,
};

