//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#include <stddef.h>
#include <vlib/vlib.h>
#include <vnet/vnet.h>
#include <vnet/plugin/plugin.h>
#include "node.h"

// *INDENT-OFF*
VLIB_PLUGIN_REGISTER () = {
    .description = "Pensando Flow Plugin",
};
// *INDENT-ON*

#define DISPLAY_BUF_SIZE (1*1024*1024)

static clib_error_t *
show_flow_stats_command_fn (vlib_main_t * vm,
                            unformat_input_t * input,
                            vlib_cli_command_t * cmd)
{
    char *buf = NULL;
    pds_flow_main_t *fm = &pds_flow_main;
    int no_of_threads = fm->no_threads;
    u32 all_threads = 1, thread_id, ip4 = 0, ip6 = 0, af_set = 0, detail = 0;
    uint32_t i;

    while (unformat_check_input (input) != UNFORMAT_END_OF_INPUT) {
        if (unformat(input, "thread %u", &thread_id)) {
            if (thread_id >= fm->no_threads) {
                vlib_cli_output(vm, "ERROR: Invalid thread-id, "
                                "valid range - 0 - %u", fm->no_threads - 1);
                goto done;
            }
            if (detail) {
                vlib_cli_output(vm, "ERROR: Option \"detail\" can't be used with option \"thread\"");
                goto done;
            }
            all_threads = 0;
        } else if (unformat(input, "detail")) {
            if (!all_threads) {
                vlib_cli_output(vm, "ERROR: Option \"detail\" can't be used with option \"thread\"");
                goto done;
            }
            detail = 1;
        } else if (unformat(input, "ip4")) {
            ip4 = af_set = 1;
        } else if (unformat(input, "ip6")) {
            ip6 = af_set = 1;
        } else {
            vlib_cli_output(vm, "ERROR: Invalid command");
            goto done;
        }
    }

    if (!af_set) {
        ip4 = ip6 = 1;
    }

    buf = calloc(1, DISPLAY_BUF_SIZE);
    if (!buf) {
        vlib_cli_output(vm, "ERROR: Failed to allocate  display buffer!");
        goto done;
    }

    if (detail || !all_threads) {
        if (ip4) {
            vlib_cli_output(vm, "IPv4 flow statistics\n");
            if (detail) {
                vlib_cli_output(vm, "Total number of IPv4 flow entries in hardware %u",
                                ftlv4_get_flow_count(fm->table4[0]));
            }
            for (i = 0; i < no_of_threads; i++) {
                if ((!all_threads) && (thread_id != i)) {
                    continue;
                }
                vlib_worker_thread_t *w = vlib_worker_threads + i;
                ftlv4_dump_stats(fm->table4[i], buf, DISPLAY_BUF_SIZE - 1);
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
        }
        if (ip6) {
            vlib_cli_output(vm, "\nIPv6 flow statistics\n");
            if (detail) {
                vlib_cli_output(vm, "Total number of IPv6 flow entries in hardware %u",
                                ftlv6_get_flow_count(fm->table6[0]));
            }
            for (i = 0; i < no_of_threads; i++) {
                if ((!all_threads) && (thread_id != i)) {
                    continue;
                }
                vlib_worker_thread_t *w = vlib_worker_threads + i;
                ftlv6_dump_stats(fm->table6[i], buf, DISPLAY_BUF_SIZE - 1);
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
        }
        goto done;
    }

    /* Summary case */
    if (ip4) {
        vlib_cli_output(vm, "\nIPv4 flow statistics summary:\n");
        vlib_cli_output(vm, "Total number of IPv4 flow entries in hardware %u",
                        ftlv4_get_flow_count(fm->table4[0]));
        ftlv4_dump_stats_summary(fm->table4, no_of_threads, buf, DISPLAY_BUF_SIZE - 1);
        vlib_cli_output(vm, "%s", buf);
    }
    if (ip6) {
        vlib_cli_output(vm, "\nIPv6 flow statistics summary:\n");
        vlib_cli_output(vm, "Total number of IPv6 flow entries in hardware %u",
                        ftlv6_get_flow_count(fm->table6[0]));
        ftlv6_dump_stats_summary(fm->table6, no_of_threads, buf, DISPLAY_BUF_SIZE - 1);
        vlib_cli_output(vm, "%s", buf);
    }

done:
    if (buf) {
        free(buf);
    }
    return 0;
}

VLIB_CLI_COMMAND (show_flow_stats_command, static) =
{
    .path = "show flow statistics",
    .short_help = "show flow statistics [detail] [thread <thread-id>] [ip4 | ip6]",
    .function = show_flow_stats_command_fn,
};

static clib_error_t *
dump_flow_entry_command_fn (vlib_main_t * vm,
                            unformat_input_t * input,
                            vlib_cli_command_t * cmd)
{
    pds_flow_main_t *fm = &pds_flow_main;
    int ret;
    ip46_address_t src = ip46_address_initializer,
                   dst = ip46_address_initializer;
    u32 sport = 0, dport = 0, lkp_id = 0;
    u8 ip_proto,
       src_set = 0,
       dst_set = 0,
       src_port_set = 0,
       dst_port_set = 0,
       proto_set = 0,
       lkp_id_set = 0,
       ip4 = 0;
    char *buf = NULL;

    while (unformat_check_input (input) != UNFORMAT_END_OF_INPUT) {
        if (unformat (input, "source-ip %U", unformat_ip46_address, &src)) {
            src_set = 1;
        } else if (unformat (input, "destination-ip %U",
                             unformat_ip46_address, &dst)) {
            dst_set = 1;
        } else if (unformat(input, "ip-protocol %U",
                            unformat_ip_protocol, &ip_proto)) {
            proto_set = 1;
        } else if (unformat (input, "source-port %u", &sport)) {
            src_port_set = 1;
        } else if (unformat (input, "destination-port %u", &dport)) {
            dst_port_set = 1;
        } else if (unformat (input, "lookup-id %u", &lkp_id)) {
            lkp_id_set = 1;
        } else {
            vlib_cli_output(vm, "ERROR: Invalid command.\n");
            goto done;
        }
    }

    if (!src_set || !dst_set || !proto_set || !lkp_id_set) {
        vlib_cli_output(vm, "ERROR: Invalid input.\n");
        goto done;
    }

    if (((ip_proto == IP_PROTOCOL_TCP) || (ip_proto == IP_PROTOCOL_UDP)) &&
        (!src_port_set || ! dst_port_set || (sport > 0xffff) || (dport > 0xffff))) { 
        vlib_cli_output(vm, "ERROR: Invalid source/destination ports.\n");
        goto done;
    }

    ip4 = ip46_address_is_ip4(&src);
    if (ip4 != ip46_address_is_ip4(&dst)) {
        vlib_cli_output(vm, "ERROR: Source and Destination IP address "
                        "belong to different address-family.\n");
        goto done;
    }

    buf = calloc(1, DISPLAY_BUF_SIZE);
    if (!buf) {
        vlib_cli_output(vm, "ERROR: Failed to allocate  display buffer!");
        goto done;
    }

    vlib_worker_thread_barrier_sync(vm);
    if (ip4) {
        ret = ftlv4_dump_hw_entry(fm->table4[0],
                                  clib_net_to_host_u32(src.ip4.as_u32),
                                  clib_net_to_host_u32(dst.ip4.as_u32),
                                  ip_proto,
                                  (u16)sport,
                                  (u16)dport,
                                  (u16)lkp_id,
                                  buf, DISPLAY_BUF_SIZE-1);
    } else {
        ret = ftlv6_dump_hw_entry(fm->table6[0],
                                  src.ip6.as_u8,
                                  dst.ip6.as_u8,
                                  ip_proto,
                                  (u16)sport,
                                  (u16)dport,
                                  (u16)lkp_id,
                                  buf, DISPLAY_BUF_SIZE-1);
    }
    vlib_worker_thread_barrier_release(vm);
    if (ret < 0) {
        vlib_cli_output(vm, "Entry not found.\n");
    } else {
        vlib_cli_output(vm, "Entry found:\n%s", buf);
    }

done:
    free(buf);
    return 0;
}

VLIB_CLI_COMMAND (dump_flow_entry_command, static) =
{
    .path = "dump flow entry",
    .short_help = "dump flow entry "
                  "source-ip <ip-address> destination-ip <ip-address> "
                  "ip-protocol <TCP|UDP|ICMP> "
                  "[source-port <UDP/TCP port number>] "
                  "[destination-port <UDP/TCP port number>] "
                  "lookup-id <number>",
    .function = dump_flow_entry_command_fn,
};

static clib_error_t *
dump_flow_entries_command_fn (vlib_main_t * vm,
                              unformat_input_t * input,
                              vlib_cli_command_t * cmd)
{
    char *logfile = NULL;
    pds_flow_main_t *fm = &pds_flow_main;
    int ret;
    u8 detail = 0;
    u32 ip4 = 0, ip6 = 0, af_set = 0;

    while (unformat_check_input (input) != UNFORMAT_END_OF_INPUT) {
        if (unformat (input, "file %s", &logfile))
            ;
        else if (unformat (input, "detail")) {
            detail = 1;
        } else if (unformat(input, "ip4")) {
            ip4 = af_set = 1;
        } else if (unformat(input, "ip6")) {
            ip6 = af_set = 1;
        } else {
            vlib_cli_output(vm, "ERROR: Invalid command\n");
            goto done;
        }
    }

    if (logfile == NULL) {
        vlib_cli_output(vm, "ERROR: Invalid filename\n");
        goto done;
    }

    if (!af_set) {
        ip4 = ip6 = 1;
    }

    if (ip4) {
        vlib_cli_output(vm, "Reading IPv4 flow entries from HW, Please wait...\n");
        vlib_worker_thread_barrier_sync(vm);
        ret = ftlv4_dump_hw_entries(fm->table4[0], logfile, detail);
        vlib_worker_thread_barrier_release(vm);
        if (ret < 0) {
            vlib_cli_output(vm, "Error writing IPv4 to %s\n", logfile);
        } else {
            vlib_cli_output(vm, "Written %d valid IPv4 entries to %s successfully.\n",
                    ret, logfile);
        }
    }
    if (ip6) {
        vlib_cli_output(vm, "Reading IPv6 flow entries from HW, Please wait...\n");
        vlib_worker_thread_barrier_sync(vm);
        ret = ftlv6_dump_hw_entries(fm->table6[0], logfile, detail);
        vlib_worker_thread_barrier_release(vm);
        if (ret < 0) {
            vlib_cli_output(vm, "Error writing IPv6 to %s\n", logfile);
        } else {
            vlib_cli_output(vm, "Written %d valid IPv6 entries to %s successfully.\n",
                    ret, logfile);
        }
    }

done:
    vec_free(logfile);
    return 0;
}

VLIB_CLI_COMMAND (dump_flow_entries_command, static) =
{
    .path = "dump flow entries",
    .short_help = "dump flow entries file "
                  "<absolute file path to dump hw entries> [detail] [ip4 | ip6]",
    .function = dump_flow_entries_command_fn,
};

static clib_error_t *
clear_flow_entries_command_fn (vlib_main_t * vm,
                               unformat_input_t * input,
                               vlib_cli_command_t * cmd)
{
    pds_flow_main_t *fm = &pds_flow_main;
    int ret1 = 0, ret2 = 0;

    while (unformat_check_input (input) != UNFORMAT_END_OF_INPUT) {
        vlib_cli_output(vm, "ERROR: Invalid command");
        goto done;
    }

    vlib_worker_thread_barrier_sync(vm);
    /* Clear all flows from tables */
    ret1 = ftlv4_clear(fm->table4[0], true, false);
    ret2 = ftlv6_clear(fm->table6[0], true, false);
    pds_session_id_flush();
    vlib_worker_thread_barrier_release(vm);

    if (ret1) {
        vlib_cli_output(vm, "ERROR: Failed to clear all IPv4 flows");
    }
    if (ret2) {
        vlib_cli_output(vm, "ERROR: Failed to clear all IPv6 flows");
    }

    if (!ret1 && !ret2) {
        vlib_cli_output(vm, "Successfully cleared all flows");
    }

done:
    return 0;
}

VLIB_CLI_COMMAND (clear_flow_entries_command, static) =
{
    .path = "clear flow entries",
    .short_help = "clear flow entries",
    .function = clear_flow_entries_command_fn,
};

static clib_error_t *
clear_flow_stats_command_fn (vlib_main_t * vm,
                             unformat_input_t * input,
                             vlib_cli_command_t * cmd)
{
    pds_flow_main_t *fm = &pds_flow_main;
    int ret1 = 0, ret2 = 0;
    u32 all_threads = 1, thread_id, ip4 = 0, ip6 = 0, af_set = 0;
    uint32_t i = 0;

    while (unformat_check_input (input) != UNFORMAT_END_OF_INPUT) {
        if (unformat(input, "thread %u", &thread_id)) {
            if (thread_id >= fm->no_threads) {
                vlib_cli_output(vm, "ERROR: Invalid thread-id, "
                                "valid range - 0 - %u", fm->no_threads - 1);
                goto done;
            }
            all_threads = 0;
        } else if (unformat(input, "ip4")) {
            ip4 = af_set = 1;
        } else if (unformat(input, "ip6")) {
            ip6 = af_set = 1;
        } else {
            vlib_cli_output(vm, "ERROR: Invalid command");
            goto done;
        }
    }

    if (!af_set) {
        ip4 = ip6 = 1;
    }

    vlib_worker_thread_barrier_sync(vm);
    if (all_threads) {
        for (i = 0; ((i < fm->no_threads) && (!ret1) && (!ret2)); i++) {
            if (ip4) {
                vlib_cli_output(vm, "Clearing IPv4 flow statistcs for thread[%d]", i);
                ret1 = ftlv4_clear(fm->table4[i], false, true);
            }
            if (ip6) {
                vlib_cli_output(vm, "Clearing IPv6 flow statistcs for thread[%d]", i);
                ret2 = ftlv6_clear(fm->table6[i], false, true);
            }
        }
    } else {
        if (ip4) {
            vlib_cli_output(vm, "Clearing IPv4 flow statistcs for thread[%d]", thread_id);
            ret1 = ftlv4_clear(fm->table4[thread_id], false, true);
        }
        if (ip6) {
            vlib_cli_output(vm, "Clearing IPv6 flow statistcs for thread[%d]", thread_id);
            ret2 = ftlv6_clear(fm->table6[thread_id], false, true);
        }
    }
    vlib_worker_thread_barrier_release(vm);
    if (ret1) {
        vlib_cli_output(vm, "ERROR: Failed to clear IPv4 flow "
                        "statistics for thread[%u]",
                        (all_threads ? i : thread_id));
    }
    if (ret2) {
        vlib_cli_output(vm, "ERROR: Failed to clear IPv6 flow "
                        "statistics for thread[%u]",
                        (all_threads ? i : thread_id));
    }

    if (!ret1 && !ret2) {
        vlib_cli_output(vm, "Successfully cleared flow statistics");
    }

done:
    return 0;
}

VLIB_CLI_COMMAND (clear_flow_stats_command, static) =
{
    .path = "clear flow statistics",
    .short_help = "clear flow statistics [thread <thread-id>] [ip4 | ip6]",
    .function = clear_flow_stats_command_fn,
};

static clib_error_t *
show_flow_session_info_command_fn (vlib_main_t * vm,
                                   unformat_input_t * input,
                                   vlib_cli_command_t * cmd)
{
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
    return 0;
}

VLIB_CLI_COMMAND (show_flow_session_command, static) =
{
    .path = "show flow session-info",
    .short_help = "show flow session-info [<index <id>>]",
    .function = show_flow_session_info_command_fn,
};
