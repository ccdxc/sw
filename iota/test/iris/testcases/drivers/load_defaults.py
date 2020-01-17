#! /usr/bin/python3
#
#  load_defaults test
#
#  check default settings in Naples when driver is loaded and unloaded
#  When loaded:
#       default multicast is set up
#       default lif values are set up
#       ... (other tests)
#  When unloaded:
#       multicast filters are cleared
#       lif values are cleared
#       ... (other tests)
#
import time
import iota.harness.api as api
import iota.test.utils.naples_host as host
import iota.test.iris.config.netagent.hw_push_config as hw_config

mc_cmd_base = "for i in \`/nic/bin/halctl show multicast | grep 3333 | cut -c34-40 \` ; do /nic/bin/halctl show multicast --oif-list --handle \$i | tail -1 ; done | grep -c "

def countMulticast(node, intf):
    # Run a command that can count the number of multicast filters
    # on the given interface of the given node.

    req = api.Trigger_CreateExecuteCommandsRequest()
    cmdstr = mc_cmd_base + intf
    api.Trigger_AddNaplesCommand(req, node, cmdstr)
    resp = api.Trigger(req)

    cmd = resp.commands[0]
    if cmd.exit_code != 0:
        api.Logger.error ("halctl cmds to get interface " + intf + " multicast info FAILED!")
        api.Logger.error("cmd = " + cmdstr)
        api.Logger.info(cmd.stdout)
        api.Logger.info(cmd.stderr)
        return -1

    # parse out the number
    count = int(cmd.stdout)

    return count


def testLoadedMulticast(node, expected_mc):
    # For each interface on the given node, check that they
    # have the expected number of multicast filters.

    errcnt = 0
    for intf in api.GetNaplesHostInterfaces(node):
        mc = countMulticast(node, intf)
        if mc != expected_mc:
            errcnt = errcnt + 1
            api.Logger.error("Host " + node + " interface " + intf + " should have %d multicast filters, but has %d" % (expected_mc, mc))
        else:
            api.Logger.info("Host " + node + " interface " + intf + " has %d multicast filters as expected" % mc)

    return errcnt


def prepUnloadedMulticastTest(node):
    # Set up a background job for each interface that will
    # run after we've shut down the driver.  The output is
    # saved to a file that we'll collect later.

    req = api.Trigger_CreateExecuteCommandsRequest()
    for intf in api.GetNaplesHostInterfaces(node):
        tfile = "/tmp/load_defaults_mc-" + intf + ".txt"
        cmdstr = "sleep 12 ; " + mc_cmd_base + intf + " > " + tfile
        api.Trigger_AddNaplesCommand(req, node, cmdstr, background=True)

    resp = api.Trigger(req)

    for cmd in resp.commands:
        if cmd.exit_code != 0:
            api.Logger.error ("prep of halctl cmds to get interface " + intf + " multicast info FAILED!")
            api.Logger.error("cmd = " + cmdstr)
            api.Logger.info(cmd.stdout)
            api.Logger.info(cmd.stderr)
            return -1


def gatherUnloadedMulticastTest(node):
    # Gather up the results of the background jobs and
    # verify that they found no multicast filters

    errcnt = 0
    for intf in api.GetNaplesHostInterfaces(node):
        tfile = "/tmp/load_defaults_mc-" + intf + ".txt"
        req = api.Trigger_CreateExecuteCommandsRequest()
        cmd = "cat " + tfile
        api.Trigger_AddNaplesCommand(req, node, cmd)
        resp = api.Trigger(req)

        for cmd in resp.commands:
            if cmd.exit_code != 0:
                api.Logger.error ("halctl cmds to get interface " + intf + " multicast info FAILED!")
                api.Logger.error("cmd = " + cmdstr)
                api.Logger.info(cmd.stdout)
                api.Logger.info(cmd.stderr)
                errcnt += 1
                return errcnt

        count = int(cmd.stdout)
        if count != 0:
            api.Logger.error("Host " + node + " interface " + intf + " should have 0 multicast filters, but has %d" % count)
            errcnt += 1
        else:
            api.Logger.info("Host " + node + " interface " + intf + " has %d multicast filters as expected" % count)

    return errcnt


def testLoadedLifs(tc, node):
    # Check that each intr in the node shows up in
    # the 'halctl show lifs' output and that VStrip and VIns are true

    errcnt = 0
    for intf in api.GetNaplesHostInterfaces(node):
        req = api.Trigger_CreateExecuteCommandsRequest()
        cmdstr = "/nic/bin/halctl show lif | grep " + intf
        api.Trigger_AddNaplesCommand(req, node, cmdstr)
        resp = api.Trigger(req)

        cmd = resp.commands[0]
        if cmd.exit_code != 0:
            api.Logger.error("halctl cmds to get interface " + intf + " lif info FAILED!")
            api.Logger.error("cmd = " + cmdstr)
            api.Logger.error("possibly the interface name is missing from the 'halctl show lif' output")
            api.Logger.info(cmd.stdout)
            api.Logger.info(cmd.stderr)
            errcnt += 1
            return errcnt

        words = cmd.stdout.split()
        ii = node + intf
        tc.intf_id[ii] = words[0]
        if words[1] != intf:
            api.Logger.error("wrong line? " + words[1] + " != " + intf)
            errcnt += 1
        if words[4] != "true":
            api.Logger.error("VStrip should be true, shows " + words[4])
            errcnt += 1
        if words[5] != "true":
            api.Logger.error("VIns should be true, shows " + words[5])
            errcnt += 1

        if errcnt == 0:
            api.Logger.info("Loaded Lifs test " + intf + " on " + node + " looks good")

    return errcnt


def prepUnloadedLifs(tc, node):
    # Set up 'halctl show lifs' output with delay

    req = api.Trigger_CreateExecuteCommandsRequest()
    for intf in api.GetNaplesHostInterfaces(node):
        tfile = "/tmp/load_defaults_lif-" + intf + ".txt"
        ii = node + intf
        cmdstr = "sleep 7; /nic/bin/halctl show lif | grep ^" + tc.intf_id[ii] + " > " + tfile
        api.Trigger_AddNaplesCommand(req, node, cmdstr, background=True)

    resp = api.Trigger(req)

    for cmd in resp.commands:
        if cmd.exit_code != 0:
            api.Logger.error ("prep of halctl cmds to get interface " + intf + " lif info FAILED!")
            api.Logger.error("cmd = " + cmdstr)
            api.Logger.info(cmd.stdout)
            api.Logger.info(cmd.stderr)
            return -1


def gatherUnloadedLifs(tc, node):
    # Gather 'halctl show lifs' output and check that
    # the intf name doesn't show in the name column,
    # and that VStrip and VIns are false

    errcnt = 0
    for intf in api.GetNaplesHostInterfaces(node):
        tfile = "/tmp/load_defaults_lif-" + intf + ".txt"
        req = api.Trigger_CreateExecuteCommandsRequest()
        cmdstr = "cat " + tfile
        api.Trigger_AddNaplesCommand(req, node, cmdstr)
        resp = api.Trigger(req)

        cmd = resp.commands[0]
        if cmd.exit_code != 0:
            api.Logger.error ("halctl cmds to get interface " + intf + " lif info FAILED!")
            api.Logger.error("cmd = " + cmdstr)
            api.Logger.info(cmd.stdout)
            api.Logger.info(cmd.stderr)
            errcnt += 1
            return errcnt

        words = cmd.stdout.split()
        ii = node + intf
        tc.intf_id[ii] = words[0]
        if words[0] != tc.intf_id[ii]:
            api.Logger.error("Id should be " + tc.intf_id[ii] + ", shows " + words[0])
            errcnt += 1
        if words[1] != tc.intf_id[ii]:
            api.Logger.error("Name should be " + tc.intf_id[ii] + ", shows " + words[1])
            errcnt += 1
        if words[4] != "false":
            api.Logger.error("VStrip should be false, shows " + words[4])
            errcnt += 1
        if words[5] != "false":
            api.Logger.error("VIns should be false, shows " + words[5])
            errcnt += 1

        if errcnt == 0:
            api.Logger.info("Unloaded Lifs test " + intf + " on " + node + " looks good")

    return errcnt


def Setup(tc):
    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])

    return api.types.status.SUCCESS


def Trigger(tc):

    errcnt = 0
    tc.intf_id = {}


    # assume that the driver is loaded
    for node in api.GetNaplesHostnames():
        errcnt += testLoadedMulticast(node, 2)
        errcnt += testLoadedLifs(tc, node)


    for node in api.GetNaplesHostnames():

        # set up background tests
        # each of these seems to take about 4 seconds, so be sure to
        # increase the command delays for each when another is added
        prepUnloadedMulticastTest(node)
        prepUnloadedLifs(tc, node)

        # unload driver and give time for the background tests to finish
        api.Logger.info("Start driver unload on " + node)
        if host.UnloadDriver(tc.os, node, "eth") is api.types.status.FAILURE:
            return api.types.status.FAILURE
        time.sleep(10)

        # restart the drivers and gather the results
        if host.LoadDriver(tc.os, node) is api.types.status.FAILURE:
            return api.types.status.FAILURE
        hw_config.ReAddWorkloads(node)
        api.Logger.info("Driver reload on " + node)

        errcnt += gatherUnloadedMulticastTest(node)
        errcnt += gatherUnloadedLifs(tc, node)

    if errcnt != 0:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS


def Verify(tc):

    # this is required to bring the testbed into operation state
    # after driver unload interfaces need to be initialized
    for node in api.GetNaplesHostnames():
        hw_config.ReAddWorkloads(node)

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
