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
    .description = "Pensando Flow Plugin",
};
// *INDENT-ON*

#if 0
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

#endif

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
    bool hw_read = FALSE;

    while (unformat_check_input (input) != UNFORMAT_END_OF_INPUT) {
        if (unformat(input, "hardware")) {
            hw_read = TRUE;
        }
    }

    vlib_cli_output(vm, "FTL IPv4 stats\n");
    for (i = 0; i < no_of_threads; i++) {
        vlib_worker_thread_t *w = vlib_worker_threads + i;
        ftlv4_dump_stats(fm->table4[i], buf, DISPLAY_BUF_SIZE - 1, (hw_read && (i==0)));
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
    vlib_cli_output(vm, "\nFTL IPv6 stats\n");
    for (i = 0; i < no_of_threads; i++) {
        vlib_worker_thread_t *w = vlib_worker_threads + i;
        ftlv6_dump_stats(fm->table6[i], buf, DISPLAY_BUF_SIZE - 1, (hw_read && (i==0)));
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
    .short_help = "show flow stats <hardware>",
    .function = show_flow_stats_command_fn,
};

static clib_error_t *
show_flow_entries_command_fn (vlib_main_t * vm,
                              unformat_input_t * input,
                              vlib_cli_command_t * cmd)
{
    char *logfile = NULL;
    pds_flow_main_t *fm = &pds_flow_main;
    int ret;
    u8 detail = 0;

    while (unformat_check_input (input) != UNFORMAT_END_OF_INPUT) {
        if (unformat (input, "file %s", &logfile))
            ;
        else if (unformat (input, "detail")) {
            detail = 1;
        }
        else {
            vlib_cli_output(vm, "ERROR: Invalid command\n");
            return 0;
        }
    }

    if (logfile == NULL) {
        vlib_cli_output(vm, "ERROR: Invalid filename\n");
        return 0;
    }

    vlib_cli_output(vm, "Reading flow entries from HW, Please wait...\n");
    vlib_worker_thread_barrier_sync(vm);
    ret = ftlv4_dump_hw_entries(fm->table4[0], logfile, detail);
    vlib_worker_thread_barrier_release(vm);
    if (ret == -1) {
        vlib_cli_output(vm, "Error writing IPv4 to %s\n", logfile);
    } else {
        vlib_cli_output(vm, "written %d valid IPv4 entries to %s successfully.\n",
                        ret, logfile);
    }
    vlib_worker_thread_barrier_sync(vm);
    ret = ftlv6_dump_hw_entries(fm->table6[0], logfile, detail);
    vlib_worker_thread_barrier_release(vm);
    if (ret == -1) {
        vlib_cli_output(vm, "Error writing IPv6 to %s\n", logfile);
    } else {
        vlib_cli_output(vm, "written %d valid IPv6 entries to %s successfully.\n",
                        ret, logfile);
    }
    vec_free(logfile);
    return 0;
}

VLIB_CLI_COMMAND (show_flow_entries_command, static) =
{
    .path = "show flow entries",
    .short_help = "show flow entries file "
                  "<absolute file path to dump hw entries> [detail]",
    .function = show_flow_entries_command_fn,
};

static clib_error_t *
clear_flow_entries_command_fn (vlib_main_t * vm,
                               unformat_input_t * input,
                               vlib_cli_command_t * cmd)
{
    pds_flow_main_t *fm = &pds_flow_main;
    int no_of_threads = vec_len (vlib_worker_threads);
    int ret = 0;

    vlib_worker_thread_barrier_sync(vm);
    for (u32 i = 0; i < no_of_threads; i++) {
        // global state is cleared only by thread 0
        ftlv4_clear(fm->table4[i], i == 0, true);
        ftlv6_clear(fm->table6[i], i == 0, true);
    }
    pds_session_id_flush();
    vlib_worker_thread_barrier_release(vm);
    if (ret) {
        vlib_cli_output(vm, "ERROR: Failed to clear all flow entries\n");
    } else {
        vlib_cli_output(vm, "Cleared all flow entries!!!\n");
    }
    return 0;
}

VLIB_CLI_COMMAND (clear_flow_entries_command, static) =
{
    .path = "clear flow entries",
    .short_help = "clear flow entries",
    .function = clear_flow_entries_command_fn,
};

static clib_error_t *
show_flow_session_info_command_fn (vlib_main_t * vm,
                                   unformat_input_t * input,
                                   vlib_cli_command_t * cmd)
{
    pds_flow_main_t *fm = &pds_flow_main;
    char *logfile = NULL;
    u32 index = ~0;
    u8 *entry = NULL;
    u32 size = 0;

    while (unformat_check_input (input) != UNFORMAT_END_OF_INPUT) {
        if (unformat(input, "index %u", &index)) {

        } else {
            vlib_cli_output(vm, "ERROR: Invalid command\n");
            goto done;
        }
    }

    if ((MAX_SESSION_INDEX < index) || (0 == index)) {
        vlib_cli_output(vm, "Invalid session index, Valid range 0-%u",
                        MAX_SESSION_INDEX);
        goto done;
    }
    session_get_addr(index, &entry, &size);
    vlib_cli_output(vm, "Session Id[%u]\n Session data:\n");
    vlib_cli_output(vm, "%U", format_hex_bytes, entry, size);

done:
    vec_free(logfile);
    return 0;
}

VLIB_CLI_COMMAND (show_flow_session_command, static) =
{
    .path = "show flow session-info",
    .short_help = "show flow session-info [<index <id>>]",
    .function = show_flow_session_info_command_fn,
};
