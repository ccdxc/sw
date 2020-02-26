#! /usr/bin/python3
#
# This test is only currently supported on FreeBSD.
#
import pdb
import time
import iota.harness.api as api
import iota.test.utils.naples_host as host
import iota.test.iris.config.netagent.hw_push_config as hw_config

WDOG_ERR_ADQ_HB  = 1
WDOG_ERR_CMD_HB  = 2
WDOG_ERR_FW_STAT = 3
WDOG_ERR_FW_HB0  = 4
WDOG_ERR_FW_HB1  = 5
WDOG_ERR_TXQ     = 6

def Setup(tc):
    api.Logger.info("Driver Watchdog")

    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])

    if tc.os != host.OS_TYPE_BSD:
        api.Logger.info("Not implemented")
        return api.types.status.IGNORED

    tc.stg1 = {}
    tc.stg2 = {}
    tc.stg3 = {}
    tc.stg4 = {}

    tc.host_intf = {}
    for node in tc.nodes:
        tc.host_intf[node] = []
        for intf in api.GetNaplesHostInterfaces(node):
            tc.host_intf[node].append(intf)
        for intf in host.GetHostInternalMgmtInterfaces(node):
            tc.host_intf[node].append(intf)

    return api.types.status.SUCCESS

def enable_txq_timeout(req, n, i):
    return api.Trigger_AddHostCommand(req, n, \
            "sysctl dev.%s.txq_wdog_timeout=5000" % host.GetNaplesSysctl(i))

def enable_fw_hb_interval(req, n, i):
    return api.Trigger_AddHostCommand(req, n, \
            "sysctl dev.%s.fw_hb_interval=1500" % host.GetNaplesSysctl(i))

def enable_cmd_hb_interval(req, n, i):
    return api.Trigger_AddHostCommand(req, n, \
            "sysctl dev.%s.cmd_hb_interval=1500" % host.GetNaplesSysctl(i))

def enable_adq_hb_interval(req, n, i):
    return api.Trigger_AddHostCommand(req, n, \
            "sysctl dev.%s.adq.hb_interval=1500" % host.GetNaplesSysctl(i))

def get_lifresets(req, n, i):
    return api.Trigger_AddHostCommand(req, n, \
            "sysctl -n dev.%s.lif_resets" % host.GetNaplesSysctl(i))

def get_dmesg(req, n):
    return api.Trigger_AddHostCommand(req, n, \
            "dmesg -c")

def set_wdog_error(req, n, err):
    return api.Trigger_AddHostCommand(req, n, \
            "sysctl hw.ionic.wdog_error_trigger=%d" % err)

def set_lif_wdog_error(req, n, lif, err):
    return api.Trigger_AddHostCommand(req, n, \
            "sysctl dev.ionic.%d.wdog_error_trigger=%d" % (lif, err))

def get_intf_names(tc, hostname):
    return tc.host_intf[hostname]

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    # Read the reset counters, enable all watchdogs
    for n in tc.nodes:
        ifs = get_intf_names(tc, n)
        api.Logger.info("Enable all watchdogs on %s %s" % \
                        (n, ifs))
        for i in ifs:
            enable_txq_timeout(req, n, i)
            enable_fw_hb_interval(req, n, i)
            enable_cmd_hb_interval(req, n, i)
            enable_adq_hb_interval(req, n, i)

            tc.stg1[(n,i)] = get_lifresets(req, n, i)

        # Clear dmesg on each node
        get_dmesg(req, n)

    # Force a LIF reset with TxQ watchdog
    for n in tc.nodes:
        api.Logger.info("Stage 1: Force TxQ watchdog on %s" % n)
        set_lif_wdog_error(req, n, 0, WDOG_ERR_TXQ)
    api.Trigger_AddHostCommand(req, n, "sleep 15")

    # Read the updated reset counters
    for n in tc.nodes:
        for i in get_intf_names(tc, n):
            tc.stg2[(n,i)] = get_lifresets(req, n, i)

    # Force a LIF reset with AdminQ NOP heartbeat
    for n in tc.nodes:
        api.Logger.info("Stage 2: Force AdminQ heartbeat failure on %s" % n)
        set_lif_wdog_error(req, n, 1, WDOG_ERR_ADQ_HB)
    api.Trigger_AddHostCommand(req, n, "sleep 15")

    # Read the updated reset counters
    for n in tc.nodes:
        for i in get_intf_names(tc, n):
            tc.stg3[(n,i)] = get_lifresets(req, n, i)

    # Force a FW heartbeat timeout
    # FreeBSD no longer disable dev cmd for Fw heartbeat.
    """
    for n in tc.nodes:
        api.Logger.info("Stage 3: Force FW heartbeat failure on %s" % n)
        set_wdog_error(req, n, WDOG_ERR_FW_HB1)
    """
    api.Trigger_AddHostCommand(req, n, "sleep 15")

    # Check dmesg
    for n in tc.nodes:
        tc.stg4[n] = get_dmesg(req, n)

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

# Returns True if the counter incremented by 1
def compare_counters(node, intf, initial, final):
    if int(final.stdout) == int(initial.stdout) + 1:
        api.Logger.info("%s %s counter value incremented %s -> %s" % (
            node, intf, initial.stdout, final.stdout))
        return True
    if int(final.stdout) != int(initial.stdout):
        # Log in case of unexpected value
        api.Logger.warn("%s %s counter value incremented %s -> %s ???" % (
            node, intf, initial.stdout, final.stdout))

    return False

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    # Verify stage 1
    api.Logger.info("Stage 1: Verifying TxQ watchdog LIF reset")
    for n in tc.nodes:
        any_incr = False
        for i in get_intf_names(tc, n):
            if compare_counters(n, i, tc.stg1[(n,i)], tc.stg2[(n,i)]):
                any_incr = True
        if not any_incr:
            api.Logger.warn("%s No TxQ watchdog activity found" % n)
            for i in get_intf_names(tc, n):
                api.Logger.warn("%s: %s" % (i, tc.stg2[(n,i)].stdout))
            return api.types.status.FAILURE
        api.Logger.info("%s LIF reset confirmed" % n)

    # Verify stage 2
    api.Logger.info("Stage 2: Verifying AdminQ heartbeat LIF reset")
    for n in tc.nodes:
        any_incr = False
        for i in get_intf_names(tc, n):
            if compare_counters(n, i, tc.stg2[(n,i)], tc.stg3[(n,i)]):
                any_incr = True
        if not any_incr:
            api.Logger.warn("%s No AdminQ heartbeat activity found" % n)
            for i in get_intf_names(tc, n):
                api.Logger.warn("%s: %s" % (i, tc.stg3[(n,i)].stdout))
            return api.types.status.FAILURE
        api.Logger.info("%s LIF reset confirmed" % n)

    # Verify stage 3
    # FreeBSD no longer disable dev cmd for Fw heartbeat.
    """
    api.Logger.info("Stage 3: Verifying FW heartbeat dev_cmd lockout")
    for n in tc.nodes:
        any_msg = False
        for l in tc.stg4[n].stdout.splitlines():
            if l.find('disabling dev_cmd interface') > 0:
                api.Logger.info("%s %s" % (n, l))
                any_msg = True
        if not any_msg:
            api.Logger.warn("%s No dev_cmd lockout found" % n)
            return api.types.status.FAILURE
        api.Logger.info("%s dev_cmd lockout confirmed" % n)
    """
    return api.types.status.SUCCESS

def Teardown(tc):
    # for every node, cycle through unload/load sequence
    for node in tc.nodes:
        if host.UnloadDriver(tc.os, node, "eth") is api.types.status.FAILURE:
            return api.types.status.FAILURE
        if host.LoadDriver(tc.os, node) is api.types.status.FAILURE:
            return api.types.status.FAILURE

        # this is required to bring the testbed into operation state;
        # after driver unload interfaces need to be initialized
        hw_config.ReAddWorkloads(node)

    return api.types.status.SUCCESS
