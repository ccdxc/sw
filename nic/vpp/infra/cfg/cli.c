//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include <stddef.h>
#include <vlib/vlib.h>
#include "cli.h"
#include "cli_hdlr.h"

void
buf_entry_dump (void *buf_entry)
{
    vlib_cli_output(((buf_entry_t *)buf_entry)->vm,
                    ((buf_entry_t *)buf_entry)->buf);

    return;
}

int
vpc_cfg_dump (vlib_main_t *vm)
{
    buf_entry_t buf_entry;

    memset(buf_entry.buf, 0 , DISPLAY_BUF_SIZE);
    buf_entry.vm = vm;
    PRINT_HEADER_LINE(124);
    vlib_cli_output(vm, "%-40s%-10s%-40s%-20s%-14s", "ID", "Type",
                    "V4RtTblId", "VR MAC", "FabricEncap");
    PRINT_HEADER_LINE(124);
    vpc_cfg_db_dump_hdlr((void *)&buf_entry);

    return 0;
}

int
subnet_cfg_dump (vlib_main_t *vm)
{
    int len = 40 * PDS_VPP_MAX_DHCP_SUBNET;
    char dhcp_buf[len+1];
    char dhcp_name[15];
    buf_entry_t buf_entry;

    memset(dhcp_buf, 0, len);
    for (int i = 0; i < PDS_VPP_MAX_DHCP_SUBNET; i++) {
        memset(dhcp_name, 0 ,15);
        sprintf(dhcp_name, "%s%d", "DhcpPolicy", i);
        snprintf(dhcp_buf + strlen(dhcp_buf), 40, "%-40s", dhcp_name);
    }
    dhcp_buf[len] = '\0';

    memset(buf_entry.buf, 0 , DISPLAY_BUF_SIZE);
    buf_entry.vm = vm;
    PRINT_HEADER_LINE(421);
    vlib_cli_output(vm, "%-40s%-40s%-40s%-20s%-16s%-20s%-40s%-s%-3s",
                    "ID", "VpcID", "HostIf", "IPv4Prefix",
                    "VR IPv4", "VR MAC", "RtTblID", dhcp_buf, "ToS");
    PRINT_HEADER_LINE(421);
    subnet_cfg_db_dump_hdlr((void *)&buf_entry);

    return 0;
}

int
vnic_cfg_dump (vlib_main_t *vm)
{
    buf_entry_t buf_entry;

    memset(buf_entry.buf, 0 , DISPLAY_BUF_SIZE);
    buf_entry.vm = vm;

    PRINT_HEADER_LINE(219);
    vlib_cli_output(vm, "%-40s%-40s%-14s%-20s%-14s%-20s%-20s%-11s%-40s\n",
                    "ID", "SubnetID", "VnicEncap", "MAC", "FabricEncap",
                    "RxMirrorSessionBmp", "TxMirrorSessionBmp", "SwitchVnic",
                    "HostIf");
    PRINT_HEADER_LINE(219);
    vnic_cfg_db_dump_hdlr((void *)&buf_entry);

    return 0;
}

int
device_cfg_dump (vlib_main_t *vm)
{
    buf_entry_t buf_entry;

    memset(buf_entry.buf, 0 , DISPLAY_BUF_SIZE);
    buf_entry.vm = vm;

    PRINT_HEADER_LINE(163);
    vlib_cli_output(vm, "%-16s%-20s%-16s%-12s%-30s%-16s%-10s%-33s\n",
                    "IPAddr", "MACAddr", "GatewayIP", "MemProfile",
                    "DevProfile", "LearnAgeTimeout", "OperMode", "Flags");
    PRINT_HEADER_LINE(163);
    device_cfg_db_dump_hdlr((void *)&buf_entry);

    return 0;
}

int
nat_cfg_dump (vlib_main_t *vm)
{
    buf_entry_t buf_entry;

    memset(buf_entry.buf, 0 , DISPLAY_BUF_SIZE);
    buf_entry.vm = vm;

    PRINT_HEADER_LINE(184);
    vlib_cli_output(vm, "%-40s%-40s%-20s%-20s%-10s%-10s%-10s%-10s%-10s\n",
                    "ID", "VPC", "PrefixLo", "PrefixHi", "Protocol", "Port Lo",
                    "Port Hi", "InUseCnt", "SessionCnt");
    PRINT_HEADER_LINE(184);
    nat_cfg_db_dump_hdlr((void *)&buf_entry);

    return 0;
}

int
dhcp_cfg_dump (vlib_main_t *vm)
{
    uint8_t proxy = 1;
    buf_entry_t buf_entry;

    memset(buf_entry.buf, 0 , DISPLAY_BUF_SIZE);
    buf_entry.vm = vm;

    vlib_cli_output(vm, "Proxy\n");
    PRINT_HEADER_LINE(274);
    vlib_cli_output(vm, "%-40s%-20s%-10s%-20s%-20s%-20s%-14s%-130s\n",
                    "ID", "ServerIP", "MTU", "GatewayIp", "DNSServerIp",
                    "NTPServerIp", "LeaseTimeout", "DomainName");
    PRINT_HEADER_LINE(274);
    dhcp_cfg_db_dump_hdlr((void *)&buf_entry, proxy);

    memset(buf_entry.buf, 0 , DISPLAY_BUF_SIZE);
    vlib_cli_output(vm, "Relay\n");
    PRINT_HEADER_LINE(120);
    vlib_cli_output(vm, "%-40s%-40s%-20s%-20s\n", "ID", "VPC", "ServerIP",
                    "AgentIP");
    PRINT_HEADER_LINE(120);
    proxy = 0;
    dhcp_cfg_db_dump_hdlr((void *)&buf_entry, proxy);

    return 0;
}

int
security_profile_cfg_dump (vlib_main_t *vm)
{
    buf_entry_t buf_entry;

    memset(buf_entry.buf, 0 , DISPLAY_BUF_SIZE);
    buf_entry.vm = vm;

    PRINT_HEADER_LINE(300);
    vlib_cli_output(vm, "%-40s%-20s%-20s%-20s%-20s%-20s%-20s%-20s%-20s"
                    "%-20s%-20s%-20s%-20s%-20s\n", "ID", "Flags",
                    "DefaultAction", "TcpIdleTimeOut", "UdpIdleTimeOut",
                    "IcmpIdleTimeOut", "OtherTimeOut", "TcpSyncTimeOut",
                    "TcpHalfCloseTimeOut", "TcpCLoseTimeOut", "TcpDropTimeOut",
                    "UdpDropTimeOut", "IcmpDropTimeOut", "OtherDropTimeOut");
    PRINT_HEADER_LINE(300);
    security_profile_cfg_db_dump_hdlr((void *)&buf_entry);

    return 0;
}

static clib_error_t *
vpp_pds_impl_db_dump_fn (vlib_main_t * vm,
                         unformat_input_t * input,
                         vlib_cli_command_t * cmd)
{

    int ret = 0;
    u8 vpc = 0, vnic = 0, subnet = 0, device = 0, status = 0;
    u8 nat = 0, dhcp = 0, sp = 0;

    while (unformat_check_input(input) != UNFORMAT_END_OF_INPUT) {
        if (unformat (input, "vpc")) {
            vpc = 1;
        } else if (unformat (input, "vnic")) {
            vnic = 1;
        } else if (unformat(input, "subnet")) {
            subnet = 1;
        } else if (unformat (input, "device")) {
            device = 1;
        } else if (unformat (input, "dhcp")) {
            dhcp = 1;
        } else if (unformat (input, "nat")) {
            nat = 1;
        } else if (unformat (input, "security-profile")) {
            sp = 1;
        } else if (unformat(input, "status")) {
            status = 1;
        } else {
            vlib_cli_output(vm, "ERROR: Invalid command.\n");
            goto done;
        }
    }

    if (!vpc && !vnic && !subnet && !device && !dhcp && !nat && !sp) {
        vlib_cli_output(vm, "ERROR: Invalid input.\n");
        goto done;
    }

    if (status) {
        // impl db display
        char obj_str[10];
        if (vpc) {
            strcpy(obj_str, "vpc");
        } else if (vnic) {
            strcpy(obj_str, "vnic");
        } else if (subnet) {
            strcpy(obj_str, "subnet");
        } else if (device) {
            strcpy(obj_str, "device");
        } else {
            strcpy(obj_str, "");
            vlib_cli_output(vm, "No impl db for this object\n");
        }
        ret = impl_db_dump(obj_str);
    } else {
        // config db display
        if (vpc) {
            ret = vpc_cfg_dump(vm);
        } else if (vnic) {
            ret = vnic_cfg_dump(vm);
        } else if (subnet) {
            ret = subnet_cfg_dump(vm);
        } else if (device) {
            ret = device_cfg_dump(vm);
        } else if (nat) {
            ret = nat_cfg_dump(vm);
        } else if (dhcp) {
            ret = dhcp_cfg_dump(vm);
        } else if (sp) {
            ret = security_profile_cfg_dump(vm);
        }
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
    .short_help = "show pds [vpc | subnet | vnic | device | dhcp | nat | "
                  "security-profile] [status]",
    .function = vpp_pds_impl_db_dump_fn,
    .is_mp_safe = 1,
};
