#! /usr/bin/python3
import re
import yaml
import iota.harness.api as api
import iota.test.iris.utils.debug as debug_utils
import iota.test.iris.utils.host as host_utils
import iota.test.utils.naples_host as host
import iota.test.iris.config.netagent.hw_push_config as hw_config
# tc.desc = 'Toggle link simultaneous and check link-down count'
# Note: Current scope is to look at port up/down count only and track link bringup time,
#      exported in port yaml, but, not to netdev.
# Possible future enhancements:
# - admin state toggle down/up to exercise host interface to naples linkMgr.
# - link down count and can be pulled from port yaml (if no defined support from host)
# Linux: "ethtool -r ethX"
# FreeBSD: "ifconfig ionicX down/up"
# NB: both host commands need special profile (device.conf) to toggle physical links.

UP = 1
waitTime = 20
naplesPorts = [1, 5, 9]
maxLinkBringupDuration = 15

def Setup(tc):
    api.Logger.info("Link Down count verify after simultaneous link resets")
    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])

    return api.types.status.SUCCESS

def Trigger(tc):

    testStatus = api.types.status.SUCCESS
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_cookies = []

    for n in tc.nodes:
        intfs = api.GetNaplesHostInterfaces(n)
        api.Trigger_AddNaplesCommand(req, n, "/nic/bin/halctl show port --yaml")
        tc.cmd_cookies.append('before')
        # we use halctl until we find better, consistent way
        for np in range(len(naplesPorts)):
            api.Logger.info("admin-state toggle trigger on node %s intf %d" % (n, naplesPorts[np]))
            api.Trigger_AddNaplesCommand(req, n, "/nic/bin/halctl debug port --port %d --admin-state down" % naplesPorts[np])
            api.Trigger_AddNaplesCommand(req, n, "/nic/bin/halctl debug port --port %d --admin-state up" % naplesPorts[np])
            api.Trigger_AddNaplesCommand(req, n, "/nic/bin/halctl debug port --port %d --admin-state down" % naplesPorts[np])
            api.Trigger_AddNaplesCommand(req, n, "/nic/bin/halctl debug port --port %d --admin-state up" % naplesPorts[np])
        api.Trigger_AddNaplesCommand(req, n, "sleep %d" % waitTime, timeout=300)
        api.Trigger_AddNaplesCommand(req, n, "/nic/bin/halctl show port --yaml")
        tc.cmd_cookies.append('after')
        tc.cmd_cookies.append(n)

    tc.resp = api.Trigger(req)

    if tc.resp == None:
        return api.types.status.FAILURE

    for cmd in tc.resp.commands:
        if cmd.exit_code != 0:
            api.Logger.error("Failed executing command %s" % cmd.command)
            api.Logger.info(cmd.stderr)
            testStatus = api.types.status.FAILURE

    return testStatus

def parsePortYaml(yamlData):
    operList = {}
    operState = 0
    hasBringupStamp = True

    # no linktiminginfo ==> no bringup duration timestamp
    if (yamlData.find('linktiminginfo') == -1):
        hasBringupStamp = False

    perPortOutput = yamlData.split("---")
    i = 0
    for portInfo in perPortOutput:
        operstate = 0
        testobj = yaml.load(portInfo, Loader=yaml.FullLoader)
        if bool(testobj):
            portId = testobj['spec']['keyorhandle']['keyorhandle']['portid']

            # ordering port index in operList, even if yaml returns jumbled index
            try:
                i = naplesPorts.index(portId)
            except ValueError:
                continue

            operList[i] = {};
            operData = []
            operData.append(portId) #0
            operData.append(testobj['status']['linkstatus']['operstate']) #1
            operData.append(testobj['stats']['numlinkdown']) #2
            operData.append(hasBringupStamp) #3
            if hasBringupStamp == True:
                operData.append(testobj['stats']['linktiminginfo']['bringupduration']['sec']) #4
                operData.append(testobj['stats']['linktiminginfo']['bringupduration']['nsec']) #5
            operList[i] = operData
            i += 1
    return operList

def validateResults (node, beforeList, afterList):
    intfOffset = 0
    testStatus = api.types.status.SUCCESS

    while (intfOffset < len(naplesPorts)):
        bData = beforeList[intfOffset]
        aData = afterList[intfOffset]
        portNum = naplesPorts[intfOffset]
        intfOffset += 1

        port = aData[0]
        np = "Node %s Port %d" % (node, port)

        # if port was down before, no checks; move on.
        # port really down or incorrect operstate val
        if (bData[1] != UP):
            api.Logger.error("%s is DOWN at start of test; skipping" % np)
            continue

        if (aData[1] != UP):
            # port didn't come up in waitTime, larger than maxLinkBringupDuration
            api.Logger.error("%s is not UP! bringup duration exceeded SLA %d sec" % (np, maxLinkBringupDuration))
            testStatus = api.types.status.FAILURE
            continue

        # link down count 1 increment for contiguous admin  down/up
        #  linkdown count error
        if ( portNum != 9) and ((bData[2] + 1) != aData[2]):
            api.Logger.error("%s mismatch linkdown count, before %d after %d " % (np, bData[2], aData[2]))
            testStatus = api.types.status.FAILURE
        # link down count 2 increment for two contiguous admin  down/up for OOB
        elif (portNum == 9) and ((bData[2] + 2) != aData[2]):
            api.Logger.error("%s mismatch linkdown count, before %d after %d " % (np, bData[2], aData[2]))
            testStatus = api.types.status.FAILURE

        if (aData[3] == True): # has most recent linkup timestamp; operstate is UP, here
            api.Logger.info("%s link bringup time sec.nsec: %d.%d" % (np, aData[4], aData[5]))
            if (aData[4] > maxLinkBringupDuration):
                api.Logger.error("%s bringup duration exceeded SLA %d sec" % (np, maxLinkBringupDuration))
                testStatus = api.types.status.FAILURE


    return testStatus


def Verify(tc):

    for node in tc.nodes:
        # this is required to bring the testbed into operation state
        hw_config.ReAddWorkloads(node)

    if tc.resp is None:
        return api.types.status.FAILURE

    after = ""
    before = ""
    cookie_idx = 0
    testStatus = api.types.status.SUCCESS

    for cmd in tc.resp.commands:
        if (cmd.command.find("yaml") != -1):
            if (tc.cmd_cookies[cookie_idx] == 'before'):
                before = cmd.stdout
            if (tc.cmd_cookies[cookie_idx] == 'after'):
                after = cmd.stdout
                # parse before/after port yaml
                beforeList = parsePortYaml(before)
                afterList = parsePortYaml(after)
                cookie_idx += 1
                testStatus = validateResults(tc.cmd_cookies[cookie_idx], beforeList, afterList)

            cookie_idx += 1

        # is EXIT code !0?
        if cmd.exit_code != 0:
            return api.types.status.FAILURE

    return testStatus

def Teardown(tc):
    return api.types.status.SUCCESS

