#! /usr/bin/python3
import iota.harness.api as api
import iota.test.utils.debug as debug_utils
import iota.test.utils.host as host_utils

# Testing ethtool priv flags
#
#  Initial state:
#     expect to see
#        from --show-priv-flags
#           sw-dbg-stats: off
#        no extra stats from ethtool -S
#  Turn flag on
#     cmd should succeed
#        ethtool --set-priv-flags %s sw-dbg-stats on
#     new stats from ethtool -S show up
#  Turn flag back off
#     cmd should succeed
#        ethtool --set-priv-flags %s sw-dbg-stats off
#     no extra stats from ethtool -S show up
#

def Setup(tc):
    api.Logger.info("Ethtool privflags")
    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])

    return api.types.status.SUCCESS


def __checkDebugStatsDefault(wl):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    api.Logger.info("Check default setting: %s" % wl.interface)
    api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, "ethtool --show-priv-flags %s" % wl.interface)
    api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, "ethtool -S %s" % wl.interface)
    resp = api.Trigger(req)

    cmd = resp.commands[0]
    #api.PrintCommandResults(cmd)

    if cmd.exit_code != 0:
        api.Logger.error("Bad exit code %d on interface %s" % (cmd.exit_code, wl.interface))
        api.Logger.info(cmd.stderr)
        return api.types.status.FAILURE
    if "sw-dbg-stats: off" not in cmd.stdout:
        api.Logger.error("sw-dbg-stats on by default interface %s" % wl.interface)
        return api.types.status.FAILURE

    cmd = resp.commands[1]
    #api.PrintCommandResults(cmd)

    if cmd.exit_code != 0:
        api.Logger.error("Bad exit code %d on interface %s" % (cmd.exit_code, wl.interface))
        api.Logger.info(cmd.stderr)
        return api.types.status.FAILURE
    if "napi_work_done" in cmd.stdout:
        api.Logger.error("extended dbg stats seen in default mode on interface %s" % wl.interface)
        return api.types.status.FAILURE

    return api.types.status.SUCCESS


def __checkDebugStatsOn(wl):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    api.Logger.info("Check enable setting: %s" % wl.interface)
    api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, "ethtool --set-priv-flags %s sw-dbg-stats on" % wl.interface)
    api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, "ethtool --show-priv-flags %s" % wl.interface)
    api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, "ethtool -S %s" % wl.interface)
    resp = api.Trigger(req)

    cmd = resp.commands[0]
    #api.PrintCommandResults(cmd)

    if cmd.exit_code != 0:
        api.Logger.error("Bad exit code %d on interface %s" % (cmd.exit_code, wl.interface))
        api.Logger.info(cmd.stderr)
        return api.types.status.FAILURE

    cmd = resp.commands[1]
    #api.PrintCommandResults(cmd)

    if cmd.exit_code != 0:
        api.Logger.error("Bad exit code %d on interface %s" % (cmd.exit_code, wl.interface))
        api.Logger.info(cmd.stderr)
        return api.types.status.FAILURE
    if "sw-dbg-stats: on" not in cmd.stdout:
        api.Logger.error("sw-dbg-stats not enabled in interface %s" % wl.interface)
        return api.types.status.FAILURE

    cmd = resp.commands[2]
    #api.PrintCommandResults(cmd)

    if cmd.exit_code != 0:
        api.Logger.error("Bad exit code %d on interface %s" % (cmd.exit_code, wl.interface))
        api.Logger.info(cmd.stderr)
        return api.types.status.FAILURE
    if "napi_work_done" not in cmd.stdout:
        api.Logger.error("extended dbg stats missing in enabled mode on interface %s" % wl.interface)
        return api.types.status.FAILURE

    return api.types.status.SUCCESS


def __checkDebugStatsOff(wl):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    api.Logger.info("Check disable setting: %s" % wl.interface)
    api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, "ethtool --set-priv-flags %s sw-dbg-stats off" % wl.interface)
    api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, "ethtool --show-priv-flags %s" % wl.interface)
    api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, "ethtool -S %s" % wl.interface)
    resp = api.Trigger(req)

    cmd = resp.commands[0]
    #api.PrintCommandResults(cmd)

    if cmd.exit_code != 0:
        api.Logger.error("Bad exit code %d on interface %s" % (cmd.exit_code, wl.interface))
        api.Logger.info(cmd.stderr)
        return api.types.status.FAILURE

    cmd = resp.commands[1]
    #api.PrintCommandResults(cmd)

    if cmd.exit_code != 0:
        api.Logger.error("Bad exit code %d on interface %s" % (cmd.exit_code, wl.interface))
        api.Logger.info(cmd.stderr)
        return api.types.status.FAILURE
    if "sw-dbg-stats: off" not in cmd.stdout:
        api.Logger.error("sw-dbg-stats not disabled in interface %s" % wl.interface)
        return api.types.status.FAILURE

    cmd = resp.commands[2]
    #api.PrintCommandResults(cmd)

    if cmd.exit_code != 0:
        api.Logger.error("Bad exit code %d on interface %s" % (cmd.exit_code, wl.interface))
        api.Logger.info(cmd.stderr)
        return api.types.status.FAILURE
    if "napi_work_done" in cmd.stdout:
        api.Logger.error("extended dbg stats appears in disabled mode on interface %s" % wl.interface)
        return api.types.status.FAILURE

    return api.types.status.SUCCESS


def Trigger(tc):
    if tc.os != 'linux':
        return api.types.status.SUCCESS

    workloads = api.GetWorkloads()

    ret = api.types.status.SUCCESS

    for wl in workloads:
        ret = __checkDebugStatsDefault(wl)
        if ret == api.types.status.SUCCESS:
            ret = __checkDebugStatsOn(wl)
        if ret == api.types.status.SUCCESS:
            ret = __checkDebugStatsOff(wl)

        if ret != api.types.status.SUCCESS:
            api.Logger.error("Failed for interface %s" % wl.interface)
            break

    return ret


def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
