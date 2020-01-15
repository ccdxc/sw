//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#define NAT_TEST 0

#ifdef NAT_TEST

#include <vlib/vlib.h>
#include <vnet/ip/ip.h>
#include <vnet/plugin/plugin.h>
#include "node.h"
#include "nat_api.h"

static clib_error_t *
test_nat_add_pb (vlib_main_t *vm,
                 unformat_input_t *input,
                 vlib_cli_command_t *cmd)
{
    u32 vpc_id;
    ip4_address_t ip;
    u8 proto;
    u32 start_port, end_port;
    u8 vpc_id_set = 0, ip_set = 0, proto_set = 0, start_port_set = 0, end_port_set = 0;
    nat_err_t ret;
    static unsigned char id_str[16];
    static int id = 1;

    memcpy(id_str, &id, sizeof(id));

    while (unformat_check_input(input) != UNFORMAT_END_OF_INPUT) {
        if (unformat (input, "vpc %u", &vpc_id)) {
            vpc_id_set = 1;
        } else if (unformat (input, "ip %U",
                             unformat_ip4_address, &ip)) {
            ip_set = 1;
        } else if (unformat (input, "proto %U",
                             unformat_ip_protocol, &proto)) {
            proto_set = 1;
        } else if (unformat (input, "start_port %u", &start_port)) {
            start_port_set = 1;
        } else if (unformat (input, "end_port %u", &end_port)) {
            end_port_set = 1;
        } else {
            vlib_cli_output(vm, "Error: Invalid command.\n");
            goto done;
        }
    }
    
    if (!vpc_id_set || !ip_set || !proto_set || !start_port_set ||
        !end_port_set) {
        vlib_cli_output(vm, "Error: Invalid command.\n");
        goto done;
    }

    ret = nat_port_block_add(id_str, vpc_id, ip, proto, start_port, end_port,
                             NAT_TYPE_INTERNET);
    if (ret != NAT_ERR_OK) {
        vlib_cli_output(vm, "Error: nat pb add failed! ret = %d\n", ret);
        goto done;
    }

    ret = nat_port_block_commit(id_str, vpc_id, ip, proto, start_port, end_port,
                             NAT_TYPE_INTERNET);
    if (ret != NAT_ERR_OK) {
        vlib_cli_output(vm, "Error: nat pb add failed! ret = %d\n", ret);
    } else {
        vlib_cli_output(vm, "nat pb add success!\n");
    }
    id++;

done:
    return 0;
}

static clib_error_t *
test_nat_add_flow (vlib_main_t *vm,
                   unformat_input_t *input,
                   vlib_cli_command_t *cmd)
{
    u32 vpc_id;
    ip4_address_t sip, dip, public_ip;
    u8 proto;
    u32 sport, dport, xlate_idx, xlate_idx_rflow;
    u16 public_port;
    u8 vpc_id_set = 0, sip_set = 0, dip_set = 0, proto_set = 0, sport_set = 0, dport_set = 0;
    nat_err_t ret;

    while (unformat_check_input(input) != UNFORMAT_END_OF_INPUT) {
        if (unformat (input, "vpc %u", &vpc_id)) {
            vpc_id_set = 1;
        } else if (unformat (input, "sip %U",
                             unformat_ip4_address, &sip)) {
            sip_set = 1;
        } else if (unformat (input, "dip %U",
                             unformat_ip4_address, &dip)) {
            dip_set = 1;
        } else if (unformat (input, "proto %U",
                             unformat_ip_protocol, &proto)) {
            proto_set = 1;
        } else if (unformat (input, "sport %u", &sport)) {
            sport_set = 1;
        } else if (unformat (input, "dport %u", &dport)) {
            dport_set = 1;
        } else {
            vlib_cli_output(vm, "Error: Invalid command.\n");
            goto done;
        }
    }
    
    if (!vpc_id_set || !sip_set || !dip_set || !proto_set || !sport_set ||
        !dport_set) {
        vlib_cli_output(vm, "Error: Invalid command.\n");
        goto done;
    }

    ret = nat_flow_alloc(vpc_id, dip, dport, proto, sip, sport,
                         NAT_TYPE_INTERNET, &public_ip, &public_port,
                         &xlate_idx, &xlate_idx_rflow);
    if (ret != NAT_ERR_OK) {
        vlib_cli_output(vm, "Error: nat flow add failed! ret = %d\n", ret);
    } else {
        vlib_cli_output(vm, "xlate_idx %u Allocated sip %U sport %U\n"
                        "nat flow add success.\n",
                        xlate_idx,
                        format_ip4_address, &public_ip,
                        format_tcp_udp_port,
                        clib_host_to_net_u16(public_port));
    }

done:
    return 0;
}

static clib_error_t *
test_nat_del_flow (vlib_main_t *vm,
                   unformat_input_t *input,
                   vlib_cli_command_t *cmd)
{
    u32 vpc_id;
    ip4_address_t sip, dip, pvt_ip;
    u8 proto;
    u32 sport, dport, pvt_port;
    u8 vpc_id_set = 0, sip_set = 0, dip_set = 0, proto_set = 0, sport_set = 0,
       dport_set = 0, pvt_ip_set = 0, pvt_port_set = 0;
    nat_err_t ret;

    while (unformat_check_input(input) != UNFORMAT_END_OF_INPUT) {
        if (unformat (input, "vpc %u", &vpc_id)) {
            vpc_id_set = 1;
        } else if (unformat (input, "sip %U",
                             unformat_ip4_address, &sip)) {
            sip_set = 1;
        } else if (unformat (input, "dip %U",
                             unformat_ip4_address, &dip)) {
            dip_set = 1;
        } else if (unformat (input, "pvt_ip %U",
                             unformat_ip4_address, &pvt_ip)) {
            pvt_ip_set = 1;
        } else if (unformat (input, "proto %U",
                             unformat_ip_protocol, &proto)) {
            proto_set = 1;
        } else if (unformat (input, "sport %u", &sport)) {
            sport_set = 1;
        } else if (unformat (input, "dport %u", &dport)) {
            dport_set = 1;
        } else if (unformat (input, "pvt_port %u", &pvt_port)) {
            pvt_port_set = 1;
        } else {
            vlib_cli_output(vm, "Error: Invalid command.\n");
            goto done;
        }
    }
    
    if (!vpc_id_set || !sip_set || !dip_set || !proto_set || !sport_set ||
        !dport_set || !pvt_ip_set || !pvt_port_set) {
        vlib_cli_output(vm, "Error: Invalid command.\n");
        goto done;
    }

    ret = nat_flow_dealloc(vpc_id, dip, dport, proto, sip, sport, pvt_ip,
                           pvt_port, NAT_TYPE_INTERNET);
    if (ret != NAT_ERR_OK) {
        vlib_cli_output(vm, "Error: nat flow del failed! ret = %d\n", ret);
    } else {
        vlib_cli_output(vm, "nat flow del success!\n");
    }

done:
    return 0;
}


static clib_error_t *
test_nat_usage (vlib_main_t *vm,
                unformat_input_t *input,
                vlib_cli_command_t *cmd)
{
    u32 vpc_id;
    u8 proto;
    u8 vpc_id_set = 0, proto_set;
    nat_err_t ret;
    u32 num_ports, num_ports_alloc, num_flows_alloc;
    u32 total_hw_indices, total_alloc_indices;

    while (unformat_check_input(input) != UNFORMAT_END_OF_INPUT) {
        if (unformat (input, "vpc %u", &vpc_id)) {
            vpc_id_set = 1;
        } else if (unformat (input, "proto %U",
                             unformat_ip_protocol, &proto)) {
            proto_set = 1;
        } else {
            vlib_cli_output(vm, "Error: Invalid command.\n");
            goto done;
        }
    }
    
    if (!vpc_id_set || !proto_set) {
        vlib_cli_output(vm, "Error: Invalid command.\n");
        goto done;
    }

    ret = nat_usage(vpc_id, proto, NAT_TYPE_INTERNET, &num_ports,
                    &num_ports_alloc, &num_flows_alloc);
    if (ret != NAT_ERR_OK) {
        vlib_cli_output(vm, "Error: nat usage failed! ret = %d\n", ret);
    }

    nat_hw_usage(&total_hw_indices, &total_alloc_indices);

    vlib_cli_output(vm, "VPC id %d %U\n", vpc_id,
                    format_ip_protocol, proto);
    vlib_cli_output(vm, "\tTotal ports : %d\n\tAllocated ports : %d\n"
                    "\tAvailable Ports : %d\n\tAllocated flows : %d\n"
                    "\tTotal NAT hw indices : %d\n"
                    "\tAllocated NAT hw indices : %d\n"
                    "\tAvailable NAT hw indices : %d",
                    num_ports, num_ports_alloc, num_ports - num_ports_alloc,
                    num_flows_alloc, total_hw_indices, total_alloc_indices,
                    total_hw_indices - total_alloc_indices);
done:
    return 0;
}

VLIB_CLI_COMMAND (test_nat_add_pb_command, static) =
{
    .path = "test nat pb add",
    .short_help = "test nat pb add vpc <vpc_id> ip <ip> proto <proto> start_port <start_port> end_port <end_port>",
    .function = test_nat_add_pb,
};

VLIB_CLI_COMMAND (test_nat_add_flow_command, static) =
{
    .path = "test nat flow add",
    .short_help = "test nat flow add vpc <vpc_id> sip <sip> dip <dip> proto <proto> sport <sport> dport <dport>",
    .function = test_nat_add_flow,
};

VLIB_CLI_COMMAND (test_nat_del_flow_command, static) =
{
    .path = "test nat flow del",
    .short_help = "test nat flow del vpc <vpc_id> sip <sip> dip <dip> proto <proto> sport <sport> dport <dport> pvt_ip <pvt_ip> pvt_port <pvt_port>",
    .function = test_nat_del_flow,
};

VLIB_CLI_COMMAND (test_nat_usage_command, static) =
{
    .path = "test nat usage",
    .short_help = "test nat usage vpc <vpc_id> proto <proto>",
    .function = test_nat_usage,
};

#endif // NAT_TEST
