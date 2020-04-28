#! /usr/bin/python3
import iota.harness.api as api

def AcquireIPFromDhcp(workloads):
    if not api.IsSimulation():
        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    else:
        req = api.Trigger_CreateExecuteCommandsRequest(serial = False)

    for wl in workloads:
        cmd = f"dhclient -r {wl.interface}"
        api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, cmd)
        cmd = f"dhclient {wl.interface}"
        api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, cmd)
        cmd = f"ifconfig {wl.interface} | grep {wl.ip_address}"
        api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, cmd)
    resp = api.Trigger(req)

    for cmd in resp.commands:
        cookie_idx = 0
        if cmd.exit_code != 0:
            api.Logger.info("Failed to acquire configured IP addresses over DHCP")
            api.PrintCommandResults(cmd)
            return False

    return True
