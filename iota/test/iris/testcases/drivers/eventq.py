#! /usr/bin/python3
import iota.harness.api as api

# Check that eventqueues are used when expected
# The test runs only in Linux; it reads the value seen in
# /sys/kernel/debug/ionic/0000\:b5\:00.0/identity and
# if it is non-zero
#     there should be instances of "ionic.*-eq" in /proc/interrupts
# else
#     there should be 0 instances of "ionic.*-eq" in /proc/interrupts
#

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    names = api.GetNaplesHostnames()
    hostname = names[0]
    if api.GetNodeOs(hostname) != "linux":
        return api.types.status.SUCCESS

    for intf in api.GetNaplesHostInterfaces(hostname):
        api.Logger.info("Checking event queue use on host %s interface %s" % (hostname, intf))

        # get the interface pci info
        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
        cmd = "ethtool -i " + intf + " | awk -F ' ' '/bus-info/ { print $2}'"
        api.Trigger_AddHostCommand(req, hostname, cmd)
        resp = api.Trigger(req)

        if resp is None:
            api.Logger.error("Failed to get pci info from host %s interface %s" % (hostname, intf))
            return api.types.status.FAILURE
        for cmd in resp.commands:
            if cmd.exit_code != 0:
                api.Logger.error("Error getting pci info from host %s interface %s" % (hostname, intf))
                api.PrintCommandResults(cmd)
                return api.types.status.FAILURE
            pci = cmd.stdout.strip()

        # get eth_eq_count and number of eq interrupts
        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
        cmd = "awk '/eth_eq_count/ {print $2}' < /sys/kernel/debug/ionic/" + pci + "/identity"
        api.Trigger_AddHostCommand(req, hostname, cmd)
        cmd = "grep -c -e 'ionic-" + pci + "-eq' /proc/interrupts"
        api.Trigger_AddHostCommand(req, hostname, cmd)

        resp = api.Trigger(req)
        if resp is None:
            api.Logger.error("Failed to get values from host %s interface %s" % (hostname, intf))
            return api.types.status.FAILURE

        cmd = resp.commands.pop()
        if cmd.exit_code > 1:      # exit code 1 from grep is "string not found", which is a valid answer here
            api.Logger.error("Failed to get eth_eq_count from host %s interface %s" % (hostname, intf))
            api.PrintCommandResults(cmd)
            return api.types.status.FAILURE
        eth_eq_count = int(cmd.stdout.strip())

        cmd = resp.commands.pop()
        if cmd.exit_code != 0:
            api.Logger.error("Failed to get interrupt count from host %s interface %s" % (hostname, intf))
            api.PrintCommandResults(cmd)
            return api.types.status.FAILURE
        intr_count = int(cmd.stdout.strip())

        api.Logger.info("Found eth_eq_count %d and interrupt count %d from host %s interface %s" % (eth_eq_count, intr_count, hostname, intf))

        if eth_eq_count == 0 and intr_count != 0:
            api.Logger.error("eq interrupts found when eth_eq_count == 0")
            return api.types.status.FAILURE
        elif eth_eq_count != 0 and intr_count == 0:
            api.Logger.error("No eq interrupts found when eth_eq_count != 0")
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS
