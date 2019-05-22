//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#include <stddef.h>
#include <vlib/vlib.h>
#include <vnet/vnet.h>
#include <vnet/plugin/plugin.h>
#include "flow.h"

// *INDENT-OFF*
VLIB_PLUGIN_REGISTER () = {
    .description = "Pensando Naples",
};
// *INDENT-ON*

extern u32 pds_fwd_flow_next;

static clib_error_t *
set_flow_packet_command_fn (vlib_main_t * vm,
                            unformat_input_t * input,
                            vlib_cli_command_t * cmd)
{
    u32 next;

    while (unformat_check_input (input) != UNFORMAT_END_OF_INPUT) {
        if (unformat(input, "transmit")) {
            next = FWD_FLOW_NEXT_INTF_OUT;
        } else if (unformat(input, "drop")) {
            next = FWD_FLOW_NEXT_DROP;
        } else {
            vlib_cli_output(vm, "Invalid command");
            goto end;
        }
    }
    pds_fwd_flow_next = next;

end:
    return 0;
}

VLIB_CLI_COMMAND (set_flow_packet_command, static) =
{
    .path = "set flow packet",
    .short_help = "set flow packet <transmit|drop>",
    .function = set_flow_packet_command_fn,
};

static clib_error_t *
show_flow_stats_command_fn (vlib_main_t * vm,
                            unformat_input_t * input,
                            vlib_cli_command_t * cmd)
{
#define DISPLAY_BUF_SIZE (1*1024*1024)
    char *buf = calloc(1, DISPLAY_BUF_SIZE);
    pds_flow_main_t *fm = &pds_flow_main;
    int no_of_threads = vec_len (vlib_worker_threads);
    int i;
    for (i = 0; i < no_of_threads; i++) {
        vlib_worker_thread_t *w = vlib_worker_threads + i;
        ftl_dump_stats(fm->table[i], buf, DISPLAY_BUF_SIZE - 1);
        vlib_cli_output(vm, "---------------------\n");
        if (w->cpu_id > -1) {
            vlib_cli_output (vm, "Thread %d %s (lcore %u)\n", i, w->name,
                                 w->cpu_id);
        } else {
            vlib_cli_output (vm, "Thread %d %s\n", i, w->name);
        } 
        vlib_cli_output(vm, "---------------------\n");
        vlib_cli_output(vm, "%s", buf);
    }
    free(buf);
    return 0;
}

VLIB_CLI_COMMAND (show_flow_stats_command, static) =
{
    .path = "show flow stats",
    .short_help = "show flow stats",
    .function = show_flow_stats_command_fn,
};

