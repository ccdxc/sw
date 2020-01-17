#! /usr/bin/python3
import iota.harness.api as api
import iota.test.utils.naples_host as host
import iota.test.iris.config.netagent.hw_push_config as hw_config

# Check that the driver can reconnect to the device
# after an FLR has been triggered.
#
# Essentially, this test only does
#    echo 1 > /sys/bus/pci/devices/0000:b5:00.0/reset
#    echo 0000:b5:00.0 > /sys/bus/pci/drivers/ionic/unbind
#    echo 0000:b5:00.0 > /sys/bus/pci/drivers/ionic/bind
#

def Setup(tc):
    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])
    return api.types.status.SUCCESS

def _triggerFLR(hostname, pci):
    # echo 1 > /sys/bus/pci/devices/0000:b5:00.0/reset
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd = "echo 1 > /sys/bus/pci/devices/" + pci + "/reset"
    api.Trigger_AddHostCommand(req, hostname, cmd)

    resp = api.Trigger(req)
    if resp == None:
        api.Logger.error("Failed to trigger FLR on host %s pci %s" % (hostname, pci))
        api.PrintCommandResults(cmd)
        return api.types.status.FAILURE

    cmd = resp.commands.pop()
    if cmd.exit_code != 0:
        api.Logger.error("FLR failed on host %s pci %s" % (hostname, pci))
        api.PrintCommandResults(cmd)
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def _bindDriver(hostname, pci):
    # echo 0000:b5:00.0 > /sys/bus/pci/drivers/ionic/bind
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd = "echo " + pci + " > /sys/bus/pci/drivers/ionic/unbind"
    api.Trigger_AddHostCommand(req, hostname, cmd)

    resp = api.Trigger(req)
    if resp == None:
        api.Logger.error("Failed to trigger bind on host %s pci %s" % (hostname, pci))
        api.PrintCommandResults(cmd)
        return api.types.status.FAILURE

    cmd = resp.commands.pop()
    if cmd.exit_code != 0:
        api.Logger.error("bind failed on host %s pci %s" % (hostname, pci))
        api.PrintCommandResults(cmd)
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def _unbindDriver(hostname, pci):
    # echo 0000:b5:00.0 > /sys/bus/pci/drivers/ionic/unbind
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd = "echo " + pci + " > /sys/bus/pci/drivers/ionic/unbind"
    api.Trigger_AddHostCommand(req, hostname, cmd)

    resp = api.Trigger(req)
    if resp == None:
        api.Logger.error("Failed to trigger unbind on host %s pci %s" % (hostname, pci))
        api.PrintCommandResults(cmd)
        return api.types.status.FAILURE

    cmd = resp.commands.pop()
    if cmd.exit_code != 0:
        api.Logger.error("unbind failed on host %s pci %s" % (hostname, pci))
        api.PrintCommandResults(cmd)
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def _getPci(hostname, intf):
    # ethtool -i enp181s0 | awk -F ' ' '/bus-info/ { print $2}'
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd = "ethtool -i " + intf + " | awk -F ' ' '/bus-info/ { print $2}'"
    api.Trigger_AddHostCommand(req, hostname, cmd)

    resp = api.Trigger(req)
    if resp == None:
        api.Logger.error("Failed to trigger unbind on host %s pci %s" % (hostname, pci))
        api.PrintCommandResults(cmd)
        return None

    cmd = resp.commands.pop()
    if cmd.exit_code != 0:
        api.Logger.error("unbind failed on host %s pci %s" % (hostname, pci))
        api.PrintCommandResults(cmd)
        return None

    pci = cmd.stdout.strip()
    return pci

def Trigger(tc):
    hostname = tc.nodes[0]
    if tc.os != "linux":
        return api.types.status.SUCCESS

    fail = 0
    for intf in api.GetNaplesHostInterfaces(hostname):
        pci = _getPci(hostname, intf)
        if pci == None or pci == "":
            api.Logger.warn("No PCI found for host %s interface %s" % (hostname, intf))
            return api.types.status.SUCCESS

        api.Logger.info("Checking FLR on host %s interface %s pci %s" % (hostname, intf, pci))

        ret = _unbindDriver(hostname, pci)
        if ret != api.types.status.SUCCESS:
            fail = fail + 1
            continue

        ret = _triggerFLR(hostname, pci)
        if ret != api.types.status.SUCCESS:
            fail = fail + 1
            continue

        ret = _bindDriver(hostname, pci)
        if ret != api.types.status.SUCCESS:
            fail = fail + 1
            continue

    if fail != 0:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Verify(tc):
    hostname = tc.nodes[0]

    # restore the workloads on the one host we tested
    if host.UnloadDriver(tc.os, hostname, "eth") is api.types.status.FAILURE:
        return api.types.status.FAILURE
    if host.LoadDriver(tc.os, hostname) is api.types.status.FAILURE:
        return api.types.status.FAILURE

    hw_config.ReAddWorkloads(hostname)

    return api.types.status.SUCCESS
