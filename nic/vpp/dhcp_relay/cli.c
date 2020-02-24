//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include <vlib/vlib.h>
#include <vnet/vnet.h>
#include <vnet/dhcp/dhcp_proxy.h>

static clib_error_t *
set_dhcp_relay_command_fn (vlib_main_t *vm,
                           unformat_input_t *input,
                           vlib_cli_command_t *cmd)
{
    u8 *next = NULL;

    while (unformat_check_input (input) != UNFORMAT_END_OF_INPUT) {
        if (unformat(input, "server-next-p4")) {
            next = (u8 *) "pds-dhcp-relay-svr-p4-inject";
        } else if (unformat(input, "server-next-linux")) {
            next = (u8 *) "pds-dhcp-relay-linux-inject";
        } else {
            vlib_cli_output(vm, "ERROR: Invalid command");
            goto done;
        }
    }

    dhcp_register_server_next_node_tx(vm, next);
done:
    return 0;
}

VLIB_CLI_COMMAND (set_dhcp_relay_command, static) =
{
    .path = "set dhcp-relay",
    .short_help = "set dhcp-relay [server-next-p4 | server-next-linux]",
    .function = set_dhcp_relay_command_fn,
};

